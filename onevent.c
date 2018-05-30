
#include <stdio.h>
#include <stdlib.h>
#include <common.h>
#include "common.h"

#define XMIN 0
#define XMAX 52483
#define YMIN (-33960)
#define YMAX 33965

sensor_type_t sensor_type_list[] = {NONE, NONE, NONE, NONE, BALL, NONE, NONE, NONE, BALL, NONE, BALL, NONE, BALL,
                                    PLAYER, PLAYER, NONE, PLAYER, NONE, NONE, PLAYER, NONE, NONE, NONE, PLAYER, PLAYER,
                                    NONE, NONE, NONE, PLAYER, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE,
                                    PLAYER, NONE, PLAYER, NONE, NONE, NONE, PLAYER, NONE, NONE, PLAYER, NONE, PLAYER,
                                    NONE, NONE, PLAYER, PLAYER, PLAYER, NONE, NONE, PLAYER, PLAYER, PLAYER, NONE,
                                    PLAYER, PLAYER, PLAYER, PLAYER, PLAYER, PLAYER, PLAYER, PLAYER, PLAYER, NONE,
                                    PLAYER, NONE, PLAYER, PLAYER, PLAYER, NONE, NONE, NONE, NONE, NONE, NONE, NONE,
                                    NONE, NONE, NONE, NONE, NONE, PLAYER, NONE, NONE, NONE, NONE, NONE, NONE, NONE,
                                    NONE, PLAYER, PLAYER, PLAYER, PLAYER, NONE, NONE, NONE, NONE, REFEREE, REFEREE};
player_t sensor_player_list[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 2, 0, 0, 4, 0, 0, 0, 6, 6, 0, 0, 0, 8,
                                 0, 0, 0, 0, 0, 0, 0, 0, 0, 13, 0, 14, 0, 0, 0, 16, 0, 0, 2, 0, 3, 0, 0, 4, 5, 5, 0, 0,
                                 7, 7, 8, 0, 9, 9, 10, 10, 11, 11, 12, 12, 13, 0, 14, 0, 15, 15, 16, 0, 0, 0, 0, 0, 0,
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


// onevent:
// event = player/ball, posizione
// if player:
// update player position
// if ball:
// TODO per ora ogni volta, se è troppo lento, ogni n volte
// find possession
// send messaggio a possession_p
// messaggio = posizione palla + tutte posizioni player + counter intervallo
// numero calcoli possesso += 1
// if finito intervallo (timestamp > interval_end):
// send messaggio a output
// messaggio = "print", numero calcoli possesso (quelli che dovrà aspettare)
// interval_end += T
// counter intervallo += 1
// numero calcoli possesso = 0
// if "end of game":
// send "end of game" to possession
// return


void onevent_run(MPI_Datatype mpi_event_type, MPI_Datatype mpi_position_for_possession_type,
                 MPI_Datatype mpi_output_envelope) {

    // initialize variables used to check when the interval_id has ended
    picoseconds interval_ends;
    interval_ends = GAME_START + INTERVAL;

//    ipotizzo tutti i giocatori al centro fixme

    // initialization of ball and players positions
    position center;
    center.x = 0;
    center.y = 0, center.z = 0;
    position players[17] = {center, center, center, center, center, center, center, center, center, center, center,
                            center, center, center, center, center, center};


    position_event send_data[ONEVENT_BUFFER_SIZE];
    output_envelope send_print;
    send_print.type = PRINT_MESSAGE;

    //todo volendo invio solo dopo posizioni di tutti

//    interval id for possession process
    int interval_id = 1;
    unsigned possession_counter = 0; //fixme ma se ne perde qualcuno?? può capitare?
//    contains event from parser
    event current_event;

// mpi variables
    MPI_Request possession_request[ONEVENT_BUFFER_SIZE];
    MPI_Request print_request;

    int req_index;

    MPI_Status status;
    int numsent = 0;
    int first_print = 1;


    while (1) {

        DBG(("\nONEVENT: waiting for a msg from PARSER"));


        MPI_Recv(&current_event, 1, mpi_event_type, PARSER_RANK, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        DBG(("\nONEVENT: received msg from PARSER"));

//        printf("fuuuuuuccck %d\n", status.MPI_TAG);
//        printf("%lu, %lu\n", 1,2);
//        printf("%lu, %lu\n", current_event.sid,3);


        switch (status.MPI_TAG) {
            case EVENT_MESSAGE:
                DBG(("\nONEVENT: received EVENT_MESSAGE"));

                // check who generated this new event
//                printf("%lu, %lu\n", current_event.sid, get_sensor_type(current_event.sid));

                if (current_event.ts > interval_ends) {
//                    printf("printtttt %lu %lu\n", current_event.ts, interval_ends);
                    if (!first_print) {
                        DBG(("\nONEVENT: waiting for previous print_request"));

                        MPI_Wait(&print_request, &status);
                        DBG(("\nONEVENT: done waiting"));

                    } else {
                        first_print = 0;
                    }

                    interval_ends += INTERVAL;
                    send_print.content = possession_counter;
//                    send_print.type = PRINT_MESSAGE;
                    possession_counter = 0;
//                    printf("%lu, %lu\n", current_event.sid, get_sensor_type(current_event.sid));
                    DBG(("\nONEVENT: sending nonblocking PRINT to OUTPUT interval=%d", interval_id));

                    MPI_Isend(&send_print, 1, mpi_output_envelope, OUTPUT_RANK,
                              interval_id,
                              MPI_COMM_WORLD, &print_request);
                    interval_id++;



//                    request_complete = 0;

                }
//                printf("%lu, %lu\n", current_event.sid, get_sensor_type(current_event.sid));
                switch (get_sensor_type(current_event.sid)) {
                    case BALL:
//                        printf("ball type\n");
                        if (ball_is_in_play(current_event.p)) {

                            if (numsent < ONEVENT_BUFFER_SIZE) {
                                // update ball position and send everything to possession

                                send_data[numsent].ball = current_event.p;
                                for (int i = 1; i < 17; ++i) {
                                    send_data[numsent].players[i] = players[i];
                                }
                                send_data[numsent].interval_id = interval_id;

                                // non-blocking send
                                MPI_Isend(&send_data[numsent], 1, mpi_position_for_possession_type, POSSESSION_RANK,
                                          POSITIONS_MESSAGE,
                                          MPI_COMM_WORLD, &possession_request[numsent]);
                                // keep track of the number of used cells in requests
                                numsent += 1;
                            } else {
                                // find a usable index in the buffer
                                MPI_Waitany(ONEVENT_BUFFER_SIZE, possession_request, &req_index, MPI_STATUS_IGNORE);
                                // prepare message in buffer
                                send_data[req_index].ball = current_event.p;
                                for (int i = 1; i < 17; ++i) {
                                    send_data[req_index].players[i] = players[i];
                                }
                                send_data[req_index].interval_id = interval_id;
                                // non-blocking send
                                MPI_Isend(&send_data[req_index], 1, mpi_position_for_possession_type, POSSESSION_RANK,
                                          POSITIONS_MESSAGE,
                                          MPI_COMM_WORLD, &possession_request[req_index]);
                            }
                            // update count for position msgs sent
                            possession_counter++;

                        }

                        break;

                    case PLAYER:
//                        printf("player type\n");

                        players[get_sensor_player(current_event.sid)] = current_event.p;
                        break;
                    default:
                        // referee, ignore
                        DBG(("\ndefault %d", get_sensor_type(current_event.sid)));
                        break;
                }
                break;

            case ENDOFGAME_MESSAGE:

                MPI_Send(&send_data, 1, mpi_position_for_possession_type, POSSESSION_RANK, ENDOFGAME_MESSAGE,
                         MPI_COMM_WORLD);
                MPI_Waitall(numsent, possession_request, MPI_STATUS_IGNORE);
                return;

            default:
                printf("Message with wrong tag %u in the \"onevent\" process!\n", status.MPI_TAG);
                printf("Aborting.\n");
                MPI_Abort(MPI_COMM_WORLD, 1);

        }
//
//        if (!request_complete)
//            MPI_Test(&print_request, &request_complete, &status);
    }

}