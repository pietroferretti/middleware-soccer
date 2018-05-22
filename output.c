
#include <stdio.h>
#include <mpi.h>
#include "common.h"


typedef struct {
    uint32_t type;
    uint32_t content;
} out_envelope;


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
    out_envelope data;      // used to receive a message, same size as the mpi datatype

    while (1) {
        // wait for a message from the possession or the onevent process
        // accept only messages for this interval
        MPI_Recv(&data, 1, mpi_output_envelope, MPI_ANY_SOURCE, interval, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        // check type of message
        switch (data.type) {
            case POSSESSION_MESSAGE:
                // get player with possession for this sample
                holder = data.content;

                // update possession arrays
                interval_possession[holder] += 1;
                total_possession[holder] += 1;

                // update count of possession processed for this interval
                num_read += 1;
                break;

            case PRINT_MESSAGE:
                // get number of possession updates we need to wait for
                num_processes = data.content;

                // collect messages from all pending processes
                while (num_read < num_processes) {
                    // wait for any possession process for this interval
                    MPI_Recv(&data, 1, mpi_output_envelope, POSSESSION_RANK, interval, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                    // get player with possession for this sample
                    holder = data.content;
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
                break;

            case ENDOFGAME_MESSAGE:
                // exit from the process
                return;

            default:
                printf("Message with wrong type %u in the \"output\" process!\n", data.type);
                printf("Aborting.\n");
                MPI_Abort(MPI_COMM_WORLD, 1);
        }
    }
}

// TODO end of game deve fare in modo che consumiamo tutti i processi pendenti da possession e onevent ?
// basterebbe che venisse mandato come ultimo messaggio una print appena prima dell'end of game