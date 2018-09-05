
/**
 * @file possession.c
 *
 * @brief This file defines a process, initialized by main.c, whose job is to
 * establish which player, and thus team,  has the ball, for each game positions
 * update message from the #parser_run process.
 */


#include <stdio.h>
#include <math.h>

#include "common.h"


/**
 * @brief This method computes the euclidean distance<SUP>2</SUP> between a specific player and the ball.
 *
 * \f$distance^2=\sqrt{(x_2-x_1)^2+(y_2-y_1)^2+(z_2-z_1)^2}\f$
 *
 * @param player_position Position of the player we are interested in.
 * @param ball_last_position Ball position.
 * @return Distance<SUP>2</SUP> between player_position and ball_last_position.
 */
double squareDistanceFromBall(position player_position, position ball_last_position) {
    return ((player_position.x - ball_last_position.x) * (player_position.x - ball_last_position.x) +
            (player_position.y - ball_last_position.y) * (player_position.y - ball_last_position.y) +
            (player_position.z - ball_last_position.z) * (player_position.z - ball_last_position.z));
}


/**
 * @brief Starts the possession process, which computes ball possessions given the player positions.
 *
 * It keeps waiting for POSITIONS_MESSAGE containing players or ball position
 * updates, until receiving the ENDOFGAME_MESSAGE or an unknown tag message
 * causing the process to abort.
 *
 * After receiving a POSITIONS_MESSAGE, it recomputes ball possession:
 * a player is considered in possession of the ball when
 * - He is the player closest to the ball
 * - He is not farther than K millimeters from the ball.
 * Then it sends an  to the output.c process, which will
 * use it to compute and print the game statistics.
 *
 * After receiving a ENDOFGAME_MESSAGE, it waits for the sending queue to
 * clear out and abort.
 *
 * @param mpi_possession_envelope mpi_datatype of received message from #parser_run
 * process, with tag POSITIONS_MESSAGE or ENDOFGAME_MESSAGE.
 * @param mpi_output_envelope mpi_datatype of sent messages to output process.
 * @param K Maximum distance between ball and player: if distance between each
 * player and the ball is greater than k then no one has ball possession.
 * K is in millimeters and ranges from 1000 to 5000.
 */
void possession_run(MPI_Datatype mpi_possession_envelope, MPI_Datatype mpi_output_envelope, unsigned long K) {

    // square the minimum distance K to compare distances without having to compute the square root
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
                // snapshot of the game received from the parser process, we need to find who has possession
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
                // find a free spot in the buffer array to hold the data we need to send
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
                // the game has ended, close everything
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
