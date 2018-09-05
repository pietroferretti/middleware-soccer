/**
 * @file parser.c
 *
 * @brief This file defines a process, initialized by main.c, whose job is to read game data.
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
#include <stdbool.h>

#include "common.h"

/**
 * Indexes correspond to sensor ids: for each sensor its type is stored. Index
 * without an associated sensor id are stored as NONE.
 */
const sensor_type_t sensor_type_list[] = {NONE, NONE, NONE, NONE, BALL, NONE, NONE, NONE, BALL, NONE, BALL, NONE, BALL,
                                          PLAYER, PLAYER, NONE, PLAYER, NONE, NONE, PLAYER, NONE, NONE, NONE, PLAYER,
                                          PLAYER, NONE, NONE, NONE, PLAYER, NONE, NONE, NONE, NONE, NONE, NONE, NONE,
                                          NONE, NONE, PLAYER, NONE, PLAYER, NONE, NONE, NONE, PLAYER, NONE, NONE,
                                          PLAYER, NONE, PLAYER, NONE, NONE, PLAYER, PLAYER, PLAYER, NONE, NONE, PLAYER,
                                          PLAYER, PLAYER, NONE, PLAYER, PLAYER, PLAYER, PLAYER, PLAYER, PLAYER, PLAYER,
                                          PLAYER, PLAYER, NONE, PLAYER, NONE, PLAYER, PLAYER, PLAYER, NONE, NONE, NONE,
                                          NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, PLAYER, NONE, NONE,
                                          NONE, NONE, NONE, NONE, NONE, NONE, PLAYER, PLAYER, PLAYER, PLAYER, NONE,
                                          NONE, NONE, NONE, REFEREE, REFEREE};

/**
 * Indexes correspond to sensor ids: for each sensor its player id is stored. Index
 * without an associated player id are stored as 0.
 */
const player_t sensor_player_list[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 2, 0, 0, 4, 0, 0, 0, 6, 6, 0, 0,
                                       0, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 13, 0, 14, 0, 0, 0, 16, 0, 0, 2, 0, 3, 0, 0, 4,
                                       5, 5, 0, 0, 7, 7, 8, 0, 9, 9, 10, 10, 11, 11, 12, 12, 13, 0, 14, 0, 15, 15, 16,
                                       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 9, 9};

/**
 * @brief Given the sensor id it return the sensor type.
 * @param sid Sensor id.
 * @return Sensor type; NONE, BALL, PLAYER or REFEREE.
 */
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

/**
 * @brief Given the sensor id of a player, return the player.
 * @param sid Sensor id.
 * @return Id of the player as player_t.
 */
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

/**
 * @brief Returns TRUE if the given position is within the field.
 * @param p Ball position.
 * @return a bool.
 */
bool ball_is_in_play(position p) {
    return p.x >= XMIN && p.x <= XMAX && p.y >= YMIN && p.y <= YMAX;
}

/**
 * @brief Reads an event from the file and returns it as a new object.
 * @param file An open file pointer to read from
 * @param new A pointer to a free event buffer to write the new event to
 */
void readEvent(FILE *file, event *new) {
    fscanf(file, "%u,%lu,%d,%d,%d,%*s\n", &new->sid, &new->ts, &new->p.x,
           &new->p.y, &new->p.z);
}

/**
 * @brief Reads a new interruption event from file and store read data in the new interruption_event.
 * @param file An open file pointer to read from
 * @param new A pointer to a free interruption_event buffer to write the new event to
 * @param start Start time of the current half of the game. Used as offset for the event time, as the files start from zero
 * @return 0 if everything went ok, non-zero if an error occurred
 */
int readInterruptionEvent(FILE **file, struct interruption_event *new, picoseconds start) {

    // temporary variables to parse the file
    picoseconds minutes;
    double seconds;

    // read one line from the referee events file
    int read = fscanf(*file, "%*31c:%lu:%lf;%*s\n", &minutes, &seconds);

    if (!read) {
        // return -1 to signal that the file has ended
        return -1;
    } else {
        // we manage to read one event (the start of an interruption)
        new->start = start + (picoseconds) (seconds * SECTOPIC) + (minutes * 60) * SECTOPIC;
        DBG(("\nSTART INTERR %lu,%f", minutes, seconds));

        // get the corresponding end of the interruption
        fscanf(*file, "%*29c:%lu:%lf;%*s\n", &minutes, &seconds);
        new->end = start + (picoseconds) (seconds * SECTOPIC) + (minutes * 60) * SECTOPIC;
        DBG(("\nEND INTERR %lu,%f", minutes, seconds));
        return 0;
    }
}

/**
 * @brief Starts the parser, which receives events from all sensors and communicates with the output and possession processes.
 *
 * Start, end and interruptions of the game are highlighted in the data received by the process.
 *
 * @param mpi_position_for_possession_type MPI datatype used to send messages to the possession process
 * @param mpi_output_envelope MPI datatype used to send messages to the output process
 * @param possession_processes Number of possession processes that are running, used to know buffer sizes
 * @param T Length of time between outputs
 * @param fullgame_path Path for the position events file
 * @param interr_path_one Path for the first game interruptions file
 * @param interr_path_two Path for the second game interruptions file
 */
void parser_run(MPI_Datatype mpi_position_for_possession_type, MPI_Datatype mpi_output_envelope,
                int possession_processes, picoseconds T, char *fullgame_path,
                char *interr_path_one, char *interr_path_two) {

    DBG(("----------------- PARSER -----------------\n"));

    // open datasets
    FILE *fp_game = fopen(fullgame_path, "r");

    if (fp_game == NULL) {
        printf("Error: couldn't open events file at %s.\n", fullgame_path);
        printf("Aborting.\n");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    FILE *fp_interruption = fopen(interr_path_one, "r");

    if (fp_interruption == NULL) {
        printf("Error: couldn't open interruption events file %s.\n", interr_path_one);
        printf("Aborting.\n");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    printf("Game starting..\n");

    // event cursor, this will be used to iterate over events
    event current_event;

    // used to iterate over the interruptions
    interruption_event next_interruption;

    // fetch the first interruption
    // we have to handle the first interruption manually since it is formatted differently

    // temporary variables to parse file
    picoseconds minutes;
    double seconds;

    // discard headers
    fscanf(fp_interruption, "%*s\n");
    fscanf(fp_interruption, "%*s %*s %*s\n");     // beginning is at exactly 0 seconds
    fscanf(fp_interruption, "%*29c:%lu:%lf;%*s\n", &minutes, &seconds);

    next_interruption.start = GAME_START;
    DBG(("\nmin %lu, sec %lf", minutes, seconds));
    next_interruption.end =
            GAME_START + (picoseconds) (seconds * SECTOPIC) + (picoseconds) (minutes * 60) * SECTOPIC;

    DBG(("\nend interr %lu", next_interruption.end));

    // initialize variables used to check when the interval_id has ended
    picoseconds interval_ends;        // timestamp of the end of this interval, when this interval's statistics will be printed out
    interval_ends = GAME_START + T;   // inizialize starting from the beginning of the game

    // initialization of ball and players updated positions
    position center;   // position at the center of the field, used as placeholder
    center.x, center.y, center.z = 0;

    // players updated positions
    position players[17] = {center, center, center, center, center, center, center, center, center, center, center,
                            center, center, center, center, center, center};

    // buffer used to hold data sent to possession via MPI
    position_event send_data[possession_processes];
    // buffer used to hold data sent to output via MPI
    output_envelope send_output;
    send_output.type = PRINT_MESSAGE;  // set type of message to output to print; it won't be changed until the end of the gmae

    // variable to keep track of the current interval
    int interval_id = 0;
    // this counts the number of possession tasks started for the current interval
    unsigned possession_counter = 0;

    //  MPI variables
    // buffer to hold MPI Requests to send messages to the possession processes
    MPI_Request possession_request[possession_processes];
    // buffer to hold MPI Requests to send messages to the output process
    MPI_Request print_request;
    // auxiliary variable to index possession_request
    int req_index;
    // temporary variable to send a message to a process matching req_index
    int possession_process_index;
    // auxiliary variable to inspect the results of WaitAny calls
    MPI_Status status;
    // variable to keep track of how many cells have already been used in possession_request
    int numsent = 0;
    // variable to keep track if print_request has already been used
    int first_print = 1;

    // used to wrap up the first half of the game only one time
    int first_half_ended = 0;

    // start main loop
    while (!feof(fp_game)) {

        // fetch one update
        readEvent(fp_game, &current_event);

        // check if the game has started yet
        if (current_event.ts < GAME_START) {
            // skip until the game starts
            DBG(("PARSER: skipping, game not started yet\n"));
            continue;
        }

        // check if we are in the half-time break
        if (current_event.ts > FIRST_END && current_event.ts < SECOND_START) {
            // check if this is the first time we are here
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
                MPI_Isend(&send_output, 1, mpi_output_envelope, OUTPUT_RANK, interval_id, MPI_COMM_WORLD,
                          &print_request);

                // reset interval to after the break
                interval_id++;
                interval_ends = SECOND_START + T;
                possession_counter = 0;
            }
            // skip until the second half starts
            DBG(("PARSER: skipping, half-time break\n"));
            continue;
        }

        // check if the game has ended
        if (current_event.ts > GAME_END) {
            // the game has ended
            DBG(("\nPARSER: END OF GAME"));

            // send end-of-game to all possession processes
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

        // check if an interval between outputs has ended
        if (current_event.ts > interval_ends) {
            // an interval has ended, print statistics

            // check if the request buffer for print has already been used
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
            DBG(("\nPARSER: sending non-blocking PRINT to OUTPUT interval=%d", interval_id));
            MPI_Isend(&send_output, 1, mpi_output_envelope, OUTPUT_RANK, interval_id, MPI_COMM_WORLD, &print_request);

            // reset interval
            interval_id++;
            interval_ends += T;
            possession_counter = 0;
        }

        // check if we currently are in a game interruption
        if (current_event.ts >= next_interruption.start && current_event.ts <= next_interruption.end) {
            // skip until the game restarts
            DBG(("PARSER: skipping, interruption\n"));
            continue;
        }

        // check if an interruption has just ended
        if (current_event.ts > next_interruption.end) {
            // the interruption has just ended
            // fetch the next interruption from the stream
            DBG(("\nGame resume at %lu", next_interruption.end));
            // choose the correct offset to add to the event
            if (current_event.ts < FIRST_END) {
                int error = readInterruptionEvent(&fp_interruption, &next_interruption, GAME_START);
                if (error) {
                    // we're still in the first half of the game, and the first file has ended
                    // change the file we need to read from
                    fclose(fp_interruption);
                    fp_interruption = fopen(interr_path_two, "r");
                    if (fp_interruption == NULL) {
                        printf("Error: couldn't open interruption events file %s.\n", interr_path_two);
                        printf("Aborting.\n");
                        MPI_Abort(MPI_COMM_WORLD, 1);
                    }
                    // handle the first interruption manually since it is formatted differently
                    // skip csv header
                    fscanf(fp_interruption, "%*s\n");
                    // skip interruption start
                    fscanf(fp_interruption, "%*s %*s %*s\n");
                    // get interruption end
                    fscanf(fp_interruption, "%*29c:%lu:%lf;%*s\n", &minutes, &seconds);
                    next_interruption.start = SECOND_START;
                    next_interruption.end = SECOND_START + (picoseconds) (seconds * SECTOPIC)
                                            + (picoseconds) (minutes * 60) * SECTOPIC;
                }
            } else if (current_event.ts > SECOND_START) {
                int error = readInterruptionEvent(&fp_interruption, &next_interruption, SECOND_START);
                if (error) {
                    // we're in the second half of the game, and the second file has ended
                    // i.e. the game has ended
                    next_interruption.start = GAME_END;
                    next_interruption.end = GAME_END;
                }
            }

            DBG(("\nnext interruption at: %lu", next_interruption.start));
        }

        // handle update
        switch (get_sensor_type(current_event.sid)) {

            case BALL:
                if (ball_is_in_play(current_event.p)) {
                    // update ball position and start the possession computation
                    // i.e. by sending all positions to the possession process

                    // check how much of the request buffer has already been used
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