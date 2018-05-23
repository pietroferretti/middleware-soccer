

// parser:
// apri file eventi e interruzioni
// read evento o interruzione
// send evento o interruzione/resume
// if game has ended:
// send "end of game" to onevent
// return

#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <lzma.h>
#include "common.h"

#define FULLGAME_PATH "../full-game" //selezionare la working directory dalle config di build per farlo funzionare

#define FIRST_INTERRUPTIONS "../referee-events/Game Interruption/1st Half.csv"
#define SECOND_INTERRUPTIONS "../referee-events/Game Interruption/2nd Half.csv"

#define GAME_START 10753295594424116
#define FIRST_END 12557295594424116
#define SECOND_START 13086639146403495
#define GAME_END 14879639146403495


void readEvent(FILE *file, event *new) {
    fscanf(file, "%u,%lu,%d,%d,%d,%*s\n", &new->sid, &new->ts, &new->p.x,
           &new->p.y, &new->p.z);
}

void readInterruptionEvent(FILE **file, struct interruption_event *new, picoseconds start) {
    //fixme fine file è diverso
    picoseconds minutes;
    double seconds;
    int a;

    int read = fscanf(*file, "%*31c:%lu:%lf;%*s\n", &minutes, &seconds);

    if (read) {
        new->start = start + (picoseconds) (seconds * SECTOPIC) + (minutes * 60) * SECTOPIC;
        DBG(("\nSTART INTERR %d,%f", minutes, seconds));

        fscanf(*file, "%*29c:%lu:%lf;%*s\n", &minutes, &seconds);


        DBG(("\nEND INTERR %d,%f", minutes, seconds));

        new->end = start + (picoseconds) (seconds * SECTOPIC) + (minutes * 60) * SECTOPIC;
    } else if (start == SECOND_START) {
//        fclose(*file);
        new->start = GAME_END;
        new->end = GAME_END;
        return;

    } else {
        fclose(*file);
        *file = fopen(SECOND_INTERRUPTIONS, "r");
        fscanf(*file, "%*s\n");
        fscanf(*file, "%*s %*s %*s\n");
        fscanf(*file, "%*29c:%lu:%lf;%*s\n", &minutes, &seconds);
        new->start = start;
        new->end =
                start + (picoseconds) (seconds * SECTOPIC) + (picoseconds) (minutes * 60) * SECTOPIC;


    }
}

void parser_run(MPI_Datatype mpi_event_type, MPI_Datatype mpi_interruption_event_type) {
    printf("----------------- PARSER -----------------");

    // open dataset
    FILE *fp_game = fopen(FULLGAME_PATH, "r");

    FILE *fp_interruption = fopen(FIRST_INTERRUPTIONS, "r");

    if (fp_game == NULL || fp_interruption == NULL) {
        printf("Error: couldn't open file.");
        exit(1);
    }

    printf("\nGame starting..");


    // event cursor
    bool first_event = 1;
    event current_event;
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
            continue;
        }

        if (current_event.ts > GAME_END) {
            // the game has ended
            MPI_Send(&current_event, 1, mpi_event_type, ONEVENT_RANK, ENDOFGAME_MESSAGE,
                     MPI_COMM_WORLD);
            break;

        }

        if (current_event.ts >= next_interruption.start && current_event.ts <= next_interruption.end) {
            // skip until the game restarts
//            DBG(("\nevent during interruption"));
            if (first_event) {
//                DBG(("\nGame interrupted at %lu", next_interruption.start));
                first_event = 0;
//                al primo evento dopo l'interruzione avviso onevent
                MPI_Send(&next_interruption, 1, mpi_interruption_event_type, ONEVENT_RANK, INTERRUPTION_MESSAGE,
                         MPI_COMM_WORLD);
            }

            continue;
        } else if (current_event.ts > next_interruption.end) {

            DBG(("\nGame resume at %lu", next_interruption.end));
            if (current_event.ts < FIRST_END)
                readInterruptionEvent(&fp_interruption, &next_interruption, GAME_START);
            else if (current_event.ts > SECOND_START)
                readInterruptionEvent(&fp_interruption, &next_interruption, SECOND_START);

            first_event = 1;
            DBG(("\nnext interruption at: %lu", next_interruption.start));

            MPI_Send(&current_event, 1, mpi_event_type, ONEVENT_RANK, EVENT_MESSAGE,
                     MPI_COMM_WORLD);
        }


    }

    // close event file
    fclose(fp_game);
    fclose(fp_interruption);

    printf("\n** Done! **");


}