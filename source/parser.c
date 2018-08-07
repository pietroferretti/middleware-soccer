
<<<<<<< HEAD:source/parser.c
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

=======
>>>>>>> fcc63cfd29be8071cf2cc2607953848bb6799308:parser.c
#include <stdio.h>
#include <stdlib.h>
#include "common.h"

const sensor_type_t sensor_type_list[] = {NONE, NONE, NONE, NONE, BALL, NONE, NONE, NONE, BALL, NONE, BALL, NONE, BALL,
                                          PLAYER, PLAYER, NONE, PLAYER, NONE, NONE, PLAYER, NONE, NONE, NONE, PLAYER,
                                          PLAYER,
                                          NONE, NONE, NONE, PLAYER, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE,
                                          NONE,
                                          PLAYER, NONE, PLAYER, NONE, NONE, NONE, PLAYER, NONE, NONE, PLAYER, NONE,
                                          PLAYER,
                                          NONE, NONE, PLAYER, PLAYER, PLAYER, NONE, NONE, PLAYER, PLAYER, PLAYER, NONE,
                                          PLAYER, PLAYER, PLAYER, PLAYER, PLAYER, PLAYER, PLAYER, PLAYER, PLAYER, NONE,
                                          PLAYER, NONE, PLAYER, PLAYER, PLAYER, NONE, NONE, NONE, NONE, NONE, NONE,
                                          NONE,
                                          NONE, NONE, NONE, NONE, NONE, PLAYER, NONE, NONE, NONE, NONE, NONE, NONE,
                                          NONE,
                                          NONE, PLAYER, PLAYER, PLAYER, PLAYER, NONE, NONE, NONE, NONE, REFEREE,
                                          REFEREE};
const player_t sensor_player_list[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 2, 0, 0, 4, 0, 0, 0, 6, 6, 0, 0,
                                       0, 8,
                                       0, 0, 0, 0, 0, 0, 0, 0, 0, 13, 0, 14, 0, 0, 0, 16, 0, 0, 2, 0, 3, 0, 0, 4, 5, 5,
                                       0, 0,
                                       7, 7, 8, 0, 9, 9, 10, 10, 11, 11, 12, 12, 13, 0, 14, 0, 15, 15, 16, 0, 0, 0, 0,
                                       0, 0,
                                       0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 9, 9};

sensor_type_t get_sensor_type(sid_t sid) {
    if (sid >= 107) {
        fprintf(stderr, "Wrong sensor id %u in get_sensor_type", sid);
        MPI_Abort(MPI_COMM_WORLD, 1);
    }
    sensor_type_t type = sensor_type_list[sid];
    if (type != -1) {
        return type;
    }
    fprintf(stderr, "Unknown sensor %u", sid);
    MPI_Abort(MPI_COMM_WORLD, 1);
}

player_t get_sensor_player(sid_t sid) {
    if (sid >= 101) {
        fprintf(stderr, "Wrong sensor id %u in get_sensor_player", sid);
        MPI_Abort(MPI_COMM_WORLD, 1);
    }
    player_t player = sensor_player_list[sid];
    if (player != -1) {
        return player;
    }
    fprintf(stderr, "Unknown player sensor %u", sid);
    MPI_Abort(MPI_COMM_WORLD, 1);
}

bool ball_is_in_play(position p) {
    return p.x >= XMIN && p.x <= XMAX && p.y >= YMIN && p.y <= YMAX;
}

void readEvent(FILE *file, event *new) {
    fscanf(file, "%u,%lu,%d,%d,%d,%*s\n", &new->sid, &new->ts, &new->p.x,
           &new->p.y, &new->p.z);
}

void readInterruptionEvent(FILE **file, struct interruption_event *new, picoseconds start) {
    picoseconds minutes;
    double seconds;

    // read one line from the referee events file
    int read = fscanf(*file, "%*31c:%lu:%lf;%*s\n", &minutes, &seconds);

    if (read) {
        // we manage to read one event (the start of an interruption)

        new->start = start + (picoseconds) (seconds * SECTOPIC) + (minutes * 60) * SECTOPIC;
        DBG(("\nSTART INTERR %lu,%f", minutes, seconds));

        // get the corresponding end of the interruption
        fscanf(*file, "%*29c:%lu:%lf;%*s\n", &minutes, &seconds);
        new->end = start + (picoseconds) (seconds * SECTOPIC) + (minutes * 60) * SECTOPIC;
        DBG(("\nEND INTERR %lu,%f", minutes, seconds));

    } else if (start == SECOND_START) {
        // we're in the second half of the game, and the second file has ended
        // i.e. the game has ended
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
        new->end = start + (picoseconds) (seconds * SECTOPIC) + (picoseconds) (minutes * 60) * SECTOPIC;
    }
}

<<<<<<< HEAD:source/parser.c
void parser_run(MPI_Datatype mpi_event_type, MPI_Datatype mpi_interruption_event_type) {
    DBG(("----------------- PARSER -----------------\n"));
=======
void parser_run(MPI_Datatype mpi_position_for_possession_type, MPI_Datatype mpi_output_envelope,
                int possession_processes, picoseconds INTERVAL) {
>>>>>>> fcc63cfd29be8071cf2cc2607953848bb6799308:parser.c

    // open dataset
    FILE *fp_game = fopen(FULLGAME_PATH, "r");

    FILE *fp_interruption = fopen(FIRST_INTERRUPTIONS, "r");

    if (fp_game == NULL || fp_interruption == NULL) {
        printf("Error: couldn't open file.\n");
        printf("Aborting.\n");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    printf("Game starting..\n");

    // TODO mettere in ordine variabili ecc.

    // event cursor
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

    // initialize variables used to check when the interval_id has ended
    picoseconds interval_ends;
    interval_ends = GAME_START + INTERVAL;

    // initialization of ball and players positions
    position center;
    center.x = 0;
    center.y = 0, center.z = 0;
    position players[17] = {center, center, center, center, center, center, center, center, center, center, center,
                            center, center, center, center, center, center};


    position_event send_data[possession_processes];
    output_envelope send_output;
    send_output.type = PRINT_MESSAGE;


//    interval id for possession process
    int interval_id = 0;
    unsigned possession_counter = 0;

// mpi variables
    MPI_Request possession_request[possession_processes];
    MPI_Request print_request;

    int req_index;

    MPI_Status status;
    int numsent = 0;
    int first_print = 1;

    int possession_process_index;

    int first_half_ended = 0;

    while (!feof(fp_game)) {

        // fetch one update
        readEvent(fp_game, &current_event);

        if (current_event.ts < GAME_START) {
            // skip until the game starts
            DBG(("PARSER: skipping, game not started yet\n"));
            continue;
        }

        if (current_event.ts > FIRST_END && current_event.ts < SECOND_START) {
            if (!first_half_ended) {
                // print final statistics for the first half
                first_half_ended = 1;

                if (!first_print) {
                    // wait for the request buffer to be available
                    DBG(("\nPARSER: waiting for previous print_request"));
                    MPI_Wait(&print_request, &status);
                    DBG(("\nPARSER: done waiting"));
                } else {
                    first_print = 0;
                }

                // prepare print message
                send_output.content = possession_counter;
                send_output.type = PRINT_MESSAGE;

                // send message to the output process
                DBG(("\nPARSER: sending nonblocking PRINT to OUTPUT interval=%d", interval_id));
                MPI_Isend(&send_output, 1, mpi_output_envelope, OUTPUT_RANK, interval_id, MPI_COMM_WORLD, &print_request);

                // reset interval to after the break
                interval_id++;
                interval_ends = SECOND_START + INTERVAL;
                possession_counter = 0;
            }
            // skip until the second half starts
            DBG(("PARSER: skipping, half-time break\n"));
            continue;
        }

        if (current_event.ts > GAME_END) {
            // the game has ended
            DBG(("\nPARSER: END OF GAME"));

            DBG(("\nPARSER: SENDING end-of-game msg to all POSSESSION processes"));
            for (int j = 0; j < possession_processes; ++j) {
                // get any free request buffer
                MPI_Waitany(possession_processes, possession_request, &req_index, MPI_STATUS_IGNORE);
                // send message to the process j
                MPI_Send(&send_data[req_index], 1, mpi_position_for_possession_type, POSSESSION_RANK + j,
                         ENDOFGAME_MESSAGE,
                         MPI_COMM_WORLD);
            }

            // send a final print to output to close all remaining requests
            send_output.content = possession_counter;
            send_output.type = PRINT_MESSAGE;
            MPI_Send(&send_output, 1, mpi_output_envelope, OUTPUT_RANK, interval_id, MPI_COMM_WORLD);

            // send end-of-game message to output
            interval_id += 1;
            send_output.type = ENDOFGAME_MESSAGE;
            MPI_Send(&send_output, 1, mpi_output_envelope, OUTPUT_RANK, interval_id,
                     MPI_COMM_WORLD);

            DBG(("\nPARSER: wait for all sends"));
            MPI_Waitall(numsent, possession_request, MPI_STATUS_IGNORE);
            DBG(("\nPARSER: wait done"));

            // exit
            return;
        }

        if (current_event.ts > interval_ends) {
            // an interval has ended, print statistics

            if (!first_print) {
                // wait for the request buffer to be available
                DBG(("\nPARSER: waiting for previous print_request"));
                MPI_Wait(&print_request, &status);
                DBG(("\nPARSER: done waiting"));
            } else {
                first_print = 0;
            }

            // prepare print message
            send_output.content = possession_counter;
            send_output.type = PRINT_MESSAGE;

            // send message to the output process
            DBG(("\nPARSER: sending nonblocking PRINT to OUTPUT interval=%d", interval_id));
            MPI_Isend(&send_output, 1, mpi_output_envelope, OUTPUT_RANK, interval_id, MPI_COMM_WORLD, &print_request);

            // reset interval
            interval_id++;
            interval_ends += INTERVAL;
            possession_counter = 0;
        }

        if (current_event.ts >= next_interruption.start && current_event.ts <= next_interruption.end) {
            // skip until the game restarts
            DBG(("PARSER: skipping, interruption\n"));
            continue;
        }

        if (current_event.ts > next_interruption.end) {
            // fetch the next interruption from the stream
            DBG(("\nGame resume at %lu", next_interruption.end));
            if (current_event.ts < FIRST_END)
                readInterruptionEvent(&fp_interruption, &next_interruption, GAME_START);
            else if (current_event.ts > SECOND_START)
                readInterruptionEvent(&fp_interruption, &next_interruption, SECOND_START);

            DBG(("\nnext interruption at: %lu", next_interruption.start));
        }

        // handle update
        switch (get_sensor_type(current_event.sid)) {

            case BALL:
                if (ball_is_in_play(current_event.p)) {
                    // update ball position and send everything to possession

                    if (numsent < possession_processes) {
                        // we can use the request buffer sequentially
                        req_index = numsent;
                        // keep track of the number of used cells in requests
                        numsent += 1;
                    } else {
                        // find a usable index in the buffer
                        DBG(("\nPARSER: waiting for a free buffer index"));
                        MPI_Waitany(possession_processes, possession_request, &req_index, MPI_STATUS_IGNORE);
                    }

                    // prepare message in buffer
                    send_data[req_index].ball = current_event.p;
                    for (int i = 1; i < 17; ++i) {
                        send_data[req_index].players[i] = players[i];
                        DBG(("\nPARSER: data player i %d, position x %d, y %d, z %d", i, send_data[numsent].players[i].x, send_data[numsent].players[i].y, send_data[numsent].players[i].z));
                    }
                    send_data[req_index].interval_id = interval_id;

                    // send message to a possession process
                    DBG(("\nPARSER: sending POSSESSION_MESSAGE nonblocking index=%d", req_index));
                    possession_process_index = POSSESSION_RANK + req_index;
                    // non-blocking send
                    MPI_Isend(&send_data[req_index], 1, mpi_position_for_possession_type, possession_process_index,
                              POSITIONS_MESSAGE, MPI_COMM_WORLD, &possession_request[req_index]);

                    // update count of possession computations spawned
                    possession_counter++;
                }
                break;

            case PLAYER:
                // update the player's position
                players[get_sensor_player(current_event.sid)] = current_event.p;
                DBG(("PARSER: current player %d, sid %d, x %d, y %d, z %d", get_sensor_player(
                        current_event.sid), current_event.sid, players[get_sensor_player(
                        current_event.sid)].x, players[get_sensor_player(
                        current_event.sid)].y, players[get_sensor_player(current_event.sid)].z));
                break;

            default:
                // referee, ignore
                DBG(("\ndefault %d", get_sensor_type(current_event.sid)));
                break;
        }

    }

    // close event file
    fclose(fp_game);
    fclose(fp_interruption);

    printf("\n** Done! **");


}