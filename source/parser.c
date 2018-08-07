
/**
 * @file parser.c
 * @authors Nicole Gervasoni, Pietro Ferretti
 *
 * @brief This class defines a process whose job is to read game data.
 *
 *
 */

// parser:
// apri file eventi e interruzioni
// read evento o interruzione
// send evento o interruzione/resume
// if game has ended:
// send "end of game" to onevent
// return

#include <stdio.h>
#include <stdlib.h>
#include "common.h"

#define FULLGAME_PATH "../full-game" //selezionare la working directory dalle config di build per farlo funzionare

#define FIRST_INTERRUPTIONS "../referee-events/Game Interruption/1st Half.csv"
#define SECOND_INTERRUPTIONS "../referee-events/Game Interruption/2nd Half.csv"


void readEvent(FILE *file, event *new) {
    fscanf(file, "%u,%lu,%d,%d,%d,%*s\n", &new->sid, &new->ts, &new->p.x,
           &new->p.y, &new->p.z);
}

void readInterruptionEvent(FILE **file, struct interruption_event *new, picoseconds start) {
    //fixme fine file è diverso
    picoseconds minutes;
    double seconds;

    // read one line from the referee events file
    int read = fscanf(*file, "%*31c:%lu:%lf;%*s\n", &minutes, &seconds);

    if (read) {
        // we manage to read one event (the start of an interruption

        new->start = start + (picoseconds) (seconds * SECTOPIC) + (minutes * 60) * SECTOPIC;
        DBG(("\nSTART INTERR %lu,%f", minutes, seconds));

        // get the corresponding end of the interruption
        fscanf(*file, "%*29c:%lu:%lf;%*s\n", &minutes, &seconds);
        new->end = start + (picoseconds) (seconds * SECTOPIC) + (minutes * 60) * SECTOPIC;
        DBG(("\nEND INTERR %lu,%f", minutes, seconds));

    } else if (start == SECOND_START) {
        // we're in the second half of the game, and the second file has ended
        // i.e. the game has ended
//        fclose(*file);
        new->start = GAME_END;
        new->end = GAME_END;
        return;

    } else {
        // we're still in the first half of the game, and the first file has ended
        // change the file we need to read from
        fclose(*file);
        *file = fopen(SECOND_INTERRUPTIONS, "r");
        // skip csv header
        fscanf(*file, "%*s\n");
        // skip interruption start
        fscanf(*file, "%*s %*s %*s\n");
        // get interruption end
        fscanf(*file, "%*29c:%lu:%lf;%*s\n", &minutes, &seconds);
        new->start = start;
        new->end =
                start + (picoseconds) (seconds * SECTOPIC) + (picoseconds) (minutes * 60) * SECTOPIC;

    }
}

void parser_run(MPI_Datatype mpi_event_type, MPI_Datatype mpi_interruption_event_type) {
    DBG(("----------------- PARSER -----------------\n"));

    // open dataset
    FILE *fp_game = fopen(FULLGAME_PATH, "r");

    FILE *fp_interruption = fopen(FIRST_INTERRUPTIONS, "r");

    if (fp_game == NULL || fp_interruption == NULL) {
        printf("Error: couldn't open file.\n");
        printf("Aborting.\n");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    printf("Game starting..\n");


    // event cursor
//    bool first_event = 1;
    event current_event;
    event send_buf[PARSER_BUFFER_SIZE];
    MPI_Request send_req[PARSER_BUFFER_SIZE];
    MPI_Status status[PARSER_BUFFER_SIZE];
    int req_index;
    int numsent = 0;
//    int msg = -1;

    interruption_event next_interruption;
    picoseconds minutes;
    double seconds;

    // discard headers
    fscanf(fp_interruption, "%*s\n");
    fscanf(fp_interruption, "%*s %*s %*s\n");
    fscanf(fp_interruption, "%*29c:%lu:%lf;%*s\n", &minutes, &seconds);

    next_interruption.start = GAME_START;
    DBG(("\nmin %lu, sec %lf", minutes, seconds));
    next_interruption.end =
            GAME_START + (picoseconds) (seconds * SECTOPIC) + (picoseconds) (minutes * 60) * SECTOPIC;

    DBG(("\nend interr %lu", next_interruption.end));


    while (!feof(fp_game)) {

        // fetch one update
        readEvent(fp_game, &current_event);

        if (current_event.ts < GAME_START || (current_event.ts > FIRST_END && current_event.ts < SECOND_START)) {
            // skip until the game starts
//            DBG(("PARSER: skipping, out of bounds\n"));
            continue;
        }

        if (current_event.ts > GAME_END) {
            // the game has ended
            DBG(("\nPARSER: END OF GAME:"));
            DBG(("\nPARSER: wait for all sends"));
            MPI_Waitall(numsent, send_req, status);
            DBG(("\nPARSER: waited done"));
            DBG(("\nPARSER: sending ENDOFGAME_MESSAGE"));
            MPI_Send(&current_event, 1, mpi_event_type, ONEVENT_RANK, ENDOFGAME_MESSAGE,
                     MPI_COMM_WORLD);
            DBG(("\nPARSER: ENDOFGAME_MESSAGE sent!"));
            DBG(("\nPARSER: returning to main"));

            return;
        }

        if (current_event.ts >= next_interruption.start && current_event.ts <= next_interruption.end) {
            DBG(("PARSER: skipping, interruption\n"));
            // skip until the game restarts
//            DBG(("\nevent during interruption"));
//            if (first_event) {
////                DBG(("\nGame interrupted at %lu", next_interruption.start));
//                first_event = 0;
////                al primo evento dopo l'interruzione avviso onevent
//                MPI_Send(&next_interruption, 1, mpi_interruption_event_type, ONEVENT_RANK, INTERRUPTION_MESSAGE,
//                         MPI_COMM_WORLD);
//
//            }

            continue;
        } else if (current_event.ts > next_interruption.end) {
//        } else {
            DBG(("\nGame resume at %lu", next_interruption.end));
            if (current_event.ts < FIRST_END)
                readInterruptionEvent(&fp_interruption, &next_interruption, GAME_START);
            else if (current_event.ts > SECOND_START)
                readInterruptionEvent(&fp_interruption, &next_interruption, SECOND_START);

//            first_event = 1;
            DBG(("\nnext interruption at: %lu", next_interruption.start));
        }
        if (numsent < PARSER_BUFFER_SIZE) {
            // update ball position and send everything to possession

            send_buf[numsent] = current_event;
            DBG(("\nPARSER: sending EVENT_MESSAGE nonblocking"));
            DBG(("\nPARSER: numsent=%d", numsent));

            // non-blocking send
            MPI_Isend(&send_buf[numsent], 1, mpi_event_type, ONEVENT_RANK, EVENT_MESSAGE,
                      MPI_COMM_WORLD, &send_req[numsent]);
            // keep track of the number of used cells in requests
            numsent += 1;
        } else {
            // find a usable index in the buffer
            DBG(("\nPARSER: waiting for a free buffer index"));

            MPI_Waitany(PARSER_BUFFER_SIZE, send_req, &req_index, MPI_STATUS_IGNORE);
            // prepare message in buffer
            send_buf[req_index] = current_event;
            // non-blocking send
            DBG(("\nPARSER: sending EVENT_MESSAGE nonblocking index=%d", req_index));

            MPI_Isend(&send_buf[req_index], 1, mpi_event_type, ONEVENT_RANK, EVENT_MESSAGE,
                      MPI_COMM_WORLD, &send_req[req_index]);
        }



    }

    // close event file
    fclose(fp_game);
    fclose(fp_interruption);

    printf("\n** Done! **");


}