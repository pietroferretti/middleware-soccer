
#include <stdio.h>
#include <mpi.h>
#include <stdint.h>
#include <common.h>
#include <math.h>


double squareDistanceFromBall(position player_position, position ball_last_position) {
    // d = ((x2 - x1)2 + (y2 - y1)2 + (z2 - z1)2)1/2
    // we only need this to compare distances, therefore we can skip the square root
    return ((player_position.x - ball_last_position.x) * (player_position.x - ball_last_position.x) +
            (player_position.y - ball_last_position.y) * (player_position.y - ball_last_position.y) +
            (player_position.z - ball_last_position.z) * (player_position.z - ball_last_position.z));
}


void possession_run(MPI_Datatype mpi_possession_envelope, MPI_Datatype mpi_output_envelope) {

    // mi serve K
    // TODO leggere K da terminale

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
    unsigned numsent = 0;   // number of requests present in the requests array
    int index;              // index of a free cell in the array

    while (1) {
        // receive position update from onevent
        MPI_Recv(&data, 1, mpi_possession_envelope, ONEVENT_RANK, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

        switch (status.MPI_TAG) {
            case POSITIONS_MESSAGE:
                DBG(("POSSESSION: positions message received from ONEVENT, interval %d\n", data.interval_id));

                // initialize variables for minimum computation
                mindistance = INFINITY;
                closestplayer = 0;

                // find player with the smallest distance from the ball
                for (unsigned i=1; i<17; i++) {
                    currdistance = squareDistanceFromBall(data.players[i], data.ball);
                    if (currdistance < mindistance) {
                        // update closest player
                        mindistance = currdistance;
                        closestplayer = i;
                    }
                }

                // check if the player is close enough
                if (mindistance > (K * K)) {
                    // too far, no one has possession of the ball
                    closestplayer = 0;
                }

                // send result to output
                if (numsent < POSSESSION_BUFFER_SIZE) {
                    // prepare message in buffer
                    send_buffer[numsent].type = POSSESSION_MESSAGE;
                    send_buffer[numsent].content = closestplayer;
                    // non-blocking send
                    MPI_Isend(&send_buffer[numsent], 1, mpi_output_envelope, OUTPUT_RANK, data.interval_id, MPI_COMM_WORLD, &requests[numsent]);
                    // keep track of the number of used cells in requests
                    numsent += 1;
                } else {
                    // find a usable index in the buffer
                    MPI_Waitany(POSSESSION_BUFFER_SIZE, requests, &index, MPI_STATUS_IGNORE);
                    // prepare message in buffer
                    send_buffer[index].type = POSSESSION_MESSAGE;
                    send_buffer[index].content = closestplayer;
                    // non-blocking send
                    MPI_Isend(&send_buffer[index], 1, mpi_output_envelope, OUTPUT_RANK, data.interval_id, MPI_COMM_WORLD, &requests[index]);
                }
                break;

            case ENDOFGAME_MESSAGE:
                DBG(("POSSESSION: endofgame message received from ONEVENT\n"));

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


// TODO: il waitany sul buffer è un po' inutile, perché output legge sequenzialmente
// possiamo utilizzare index per ciclare 0-99
    // usa index uguale
    // togli waitany, metti wait
    // dopo la wait, (index += 1) % 100
// il codice può essere riutilizzato da onevent quando avremo n processi possession