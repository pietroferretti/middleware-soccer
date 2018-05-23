
#include <stdio.h>
#include <mpi.h>
#include "common.h"


typedef struct {
    uint32_t type;
    uint32_t content;
} msg_envelope;


void print_statistics(unsigned const interval_possession[], unsigned const total_possession[]) {
    // output statistics

    // statistics for the current interval
    printf("\n== Current Interval ==\n\n");

    // compute total possession for this interval to make percentages
    unsigned interval_total = 0;
    for (int j = 1; j < 17; ++j) {
        interval_total += interval_possession[j];
    }

    if (interval_total == 0) {
        printf("Game interrupted. Nothing to show for this interval.\n\n");
    } else {
        // team A
        double team_a_interval_poss = 0;
        printf("Ball possession team A\n");
        for (int i = 1; i < 9; ++i) {
            // compute percentage for this player
            double player_possession = (double) interval_possession[i] / interval_total * 100;
            // update total team stats
            team_a_interval_poss = team_a_interval_poss + player_possession;
            printf("Player %2d: %5.2f%%\t", i, player_possession);
        }
        printf("\nTotal: %5.2f%%\n\n", team_a_interval_poss);
        // team B
        double team_b_interval_poss = 0;
        printf("Ball possession team B:\n");
        for (int i = 9; i < 17; ++i) {
            // compute percentage for this player
            double player_possession = (double) interval_possession[i] / interval_total * 100;
            // update total team stats
            team_b_interval_poss = team_b_interval_poss + player_possession;
            printf("Player %2d: %5.2f%%\t", i, player_possession);
        }
        printf("\nTotal: %5.2f%%\n\n", team_b_interval_poss);
    }

    // cumulative statistics
    printf("== Up to now ==\n\n");

    // compute total possession for the game to make percentages
    unsigned game_total = 0;
    for (int i = 1; i < 17; ++i) {
        game_total = game_total + total_possession[i];
    }

    // team A
    double team_a_total_poss = 0;
    printf("Ball possession team A\n");
    for (int i = 1; i < 9; ++i) {
        // compute percentage for this player
        double player_possession = (double) total_possession[i] / game_total * 100;
        // update total team stats
        team_a_total_poss = team_a_total_poss + player_possession;
        printf("Player %2d: %5.2f%%\t", i, player_possession);
    }
    printf("\nTotal: %5.2f%%\n\n", team_a_total_poss);
    // team B
    double team_b_total_poss = 0;
    printf("Ball possession team B:\n");
    for (int i = 9; i < 17; ++i) {
        // compute percentage for this player
        double player_possession = (double) total_possession[i] / game_total * 100;
        // update total team stats
        team_b_total_poss = team_b_total_poss + player_possession;
        printf("Player %2d: %5.2f%%\t", i, player_possession);
    }
    printf("\nTotal: %5.2f%%\n\n", team_b_total_poss);
}


void output_run(MPI_Datatype mpi_output_envelope) {
    // TODO docs?
    // output process, computes and prints possession statistics for each player and team

    // initialize possession arrays, one cell per player
    // each cell counts how many times a player had possession of the ball
    // index 0 is a placeholder for no possession
    unsigned interval_possession[17] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    unsigned total_possession[17] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    // initialize interval counter
    unsigned interval = 0;

    // declare other variables
    player_t holder;
    unsigned num_read = 0;
    unsigned num_processes;

    // declare mpi related variables
    msg_envelope data[2];      // used to receive a message, same size as the mpi datatype
    MPI_Request requests[2];
    int last_received = -1;

    while (1) {

        switch (last_received) {
            case -1:
                // wait for a message from the possession or the onevent process
                // accept only messages for this interval
                MPI_Irecv(&data[0], 1, mpi_output_envelope, ONEVENT_RANK, interval, MPI_COMM_WORLD, &requests[0]);
                MPI_Irecv(&data[1], 1, mpi_output_envelope, POSSESSION_RANK, interval, MPI_COMM_WORLD, &requests[1]);
                break;
            case 0:
                // make a new request for the onevent processs
                MPI_Irecv(&data[0], 1, mpi_output_envelope, ONEVENT_RANK, interval, MPI_COMM_WORLD, &requests[0]);
                break;
            case 1:
                // make a new request for the possession processs
                MPI_Irecv(&data[1], 1, mpi_output_envelope, POSSESSION_RANK, interval, MPI_COMM_WORLD, &requests[1]);
                break;
            default:
                break;
        }

        // match the first process with a message
        MPI_Waitany(2, requests, &last_received, MPI_STATUS_IGNORE);

        // check type of message
        switch (data[last_received].type) {
            case POSSESSION_MESSAGE:
                // get player with possession for this sample
                holder = data[last_received].content;

                // update possession arrays
                interval_possession[holder] += 1;
                total_possession[holder] += 1;

                // update count of possession processed for this interval
                num_read += 1;
                break;

            case PRINT_MESSAGE:
                // get number of possession updates we need to wait for
                num_processes = data[last_received].content;

                // collect messages from all pending processes
                while (num_read < num_processes) {
                    // wait for any possession process for this interval
                    MPI_Recv(&data, 1, mpi_output_envelope, POSSESSION_RANK, interval, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                    // get player with possession for this sample
                    holder = data[last_received].content;
                    // update possession arrays
                    interval_possession[holder] += 1;
                    total_possession[holder] += 1;
                    // update count of possession processed for this interval
                    num_read += 1;
                }

                // interval complete, output statistics
                print_statistics(interval_possession, total_possession);

                // reset interval
                for (int i=0; i<17; i++) {
                    interval_possession[i] = 0;
                }
                num_read = 0;
                interval += 1;

                // stop waiting for possession updates on this interval
                MPI_Request_free(&requests[1]);
                break;

            case ENDOFGAME_MESSAGE:
                // remove useless pending requests (nothing will be sent after this message)
                MPI_Request_free(&requests[1]);

                // exit from the process
                return;

            default:
                printf("Message with wrong type %u in the \"output\" process!\n", data[last_received].type);
                printf("Aborting.\n");
                MPI_Abort(MPI_COMM_WORLD, 1);
        }
    }
}

// TODO end of game deve fare in modo che consumiamo tutti i processi pendenti da possession e onevent ?
// basterebbe che venisse mandato come ultimo messaggio una print appena prima dell'end of game