
/**
 * @file possession.c
 *
 * @brief This file defines a process, initialized by main.c, whose job is to
 * establish which player, and thus team,  has the ball, for each game positions
 * update message from the #parser_run process.
 */


#include <stdio.h>
#include <stdint.h>
#include "common.h"
#include <math.h>


//fixme ma ci serve controllare anche la z??
double squareDistanceFromBall(position player_position, position ball_last_position) {
    // d = ((x2 - x1)2 + (y2 - y1)2 + (z2 - z1)2)1/2

    return ((player_position.x - ball_last_position.x) * (player_position.x - ball_last_position.x) +
            (player_position.y - ball_last_position.y) * (player_position.y - ball_last_position.y) +
            (player_position.z - ball_last_position.z) * (player_position.z - ball_last_position.z));
}

void possession_run(MPI_Datatype mpi_possession_envelope, MPI_Datatype mpi_output_envelope, unsigned long K) {

    unsigned long K2 = K * K;

    // variables to compute minimum distance
    double currdistance;
    double mindistance;
    player_t closestplayer;

    // buffer for receiving messages
    position_event data;

    // buffer to read a received message tag
    MPI_Status status;

    // buffer for messages sent to output
    output_envelope send_buffer[POSSESSION_BUFFER_SIZE];

    // array to keep track of which send buffers are in use
    MPI_Request requests[POSSESSION_BUFFER_SIZE];
    unsigned numsent = 0;       // number of requests present in the requests array
    int index = 0;              // index of a free cell in the array


    while (1) {
        // receive position update from parser
        MPI_Recv(&data, 1, mpi_possession_envelope, PARSER_RANK, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

        switch (status.MPI_TAG) {
            case POSITIONS_MESSAGE:

                DBG(("POSSESSION: positions message received from ONEVENT, interval %d\n", data.interval_id));

                // initialize variables for minimum computation
                mindistance = INFINITY;
                closestplayer = 0;

                // find player with the smallest distance from the ball
                for (unsigned i = 1; i < 17; i++) {
                    // we only need this to compare distances, therefore we can skip the square root
                    currdistance = squareDistanceFromBall(data.players[i], data.ball);
                    if (currdistance < mindistance) {
                        // update closest player
                        mindistance = currdistance;
                        closestplayer = i;
                    }
                }

                // check if the player is close enough
                if (mindistance > K2) {
                    // too far, no one has possession of the ball
                    closestplayer = 0;
                    DBG(("POSSESSION: mindistance is too big! %lf > %d\n", mindistance, K * K));
                }

                // send result to output

                if (numsent < POSSESSION_BUFFER_SIZE) {
                    // get next free buffer
                    index = numsent;
                    numsent += 1;
                } else {
                    // wait for a free buffer
                    index = (index + 1) % POSSESSION_BUFFER_SIZE;
                    MPI_Wait(&requests[index], MPI_STATUS_IGNORE);
                }

                // prepare message in buffer
                send_buffer[index].type = POSSESSION_MESSAGE;
                send_buffer[index].content = closestplayer;

                // non-blocking send
                DBG(("POSSESSION: closest_player=%d", closestplayer));
                MPI_Isend(&send_buffer[index], 1, mpi_output_envelope, OUTPUT_RANK, data.interval_id, MPI_COMM_WORLD,
                          &requests[index]);
                break;

            case ENDOFGAME_MESSAGE:

                DBG(("POSSESSION: endofgame message received from PARSER\n"));

                // wait until all sends complete
                MPI_Waitall(numsent, requests, MPI_STATUS_IGNORE);

                // exit from process
                return;

            default:
                printf("Message with wrong tag %u in the \"possession\" process!\n", status.MPI_TAG);
                printf("Aborting.\n");
                MPI_Abort(MPI_COMM_WORLD, 1);
        }
    }
}
