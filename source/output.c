/**
 * @file output.c
 *
 * @brief This file defines a process, initialize by main.c, whose job is to
 * compute and output the statistic of the game for each team and player.
 *
 *
 *
 */

// output:
// if messaggio = possesso, tag=num intervallo
// arrayintervallo[player_id] += 1
// arraycumulativo[player_id] += 1
// nread += 1
// if messaggio = print, num calcoli
// while nread < num calcoli:
// receive from possession
// letti tutti
// print statistics
// annulla array intervallo
// nread = 0
// if "end of game"
// return

#include <stdio.h>

#include "common.h"

const char *player_names[] = {"None",
                              "Nick Gertje",
                              "Dennis Dotterweich",
                              "Niklas Waelzlein",
                              "Wili Sommer",
                              "Philipp Harlass",
                              "Roman Hartleb",
                              "Erik Engelhardt",
                              "Sandro Schneider",
                              "Leon Krapf",
                              "Kevin Baer",
                              "Luca Ziegler",
                              "Ben Mueller",
                              "Vale Reitstetter",
                              "Christopher Lee",
                              "Leon Heinze",
                              "Leo Langhans"};

// Used to print interval header
const picoseconds FIRST_HALF_DURATION = FIRST_END - GAME_START;
const picoseconds SECOND_HALF_DURATION = GAME_END - SECOND_START;


void print_interval(int interval, picoseconds T) {

    if (interval < (FIRST_HALF_DURATION / T)) {
        unsigned elapsed_time = (interval + 1) * (unsigned) (T / SECTOPIC);
        unsigned minutes = elapsed_time / 60;
        unsigned seconds = elapsed_time % 60;
        printf("\n== Current Interval %2d | First Half +%dm:%02ds  ==\n\n", interval, minutes, seconds);
    } else if (interval == (FIRST_HALF_DURATION / T)) {
        printf("\n== Current Interval %2d | First Half End  ==\n\n", interval);
    } else if (interval < (FIRST_HALF_DURATION / T + 1 + SECOND_HALF_DURATION / T)) {
        unsigned elapsed_time = (interval - (int) (FIRST_HALF_DURATION / T)) * (unsigned) (T / SECTOPIC);
        unsigned minutes = elapsed_time / 60;
        unsigned seconds = elapsed_time % 60;
        printf("\n== Current Interval %2d | Second Half +%dm:%02ds  ==\n\n", interval, minutes, seconds);
    } else {
        printf("\n== Current Interval %2d | Game End  ==\n\n", interval);
    }
}


void print_statistics(const unsigned int *interval_possession, const unsigned int *total_possession, int interval,
                      picoseconds T) {
    // output statistics

    //  print interval header
    print_interval(interval, T);

    // compute total possession for this interval to make percentages
    unsigned interval_total = 0;

#if IGNORE_GOALKEEPER
    for (int i = 2; i < 9; ++i) {
        interval_total += interval_possession[i];
    }
    for (int i = 10; i < 17; ++i) {
        interval_total += interval_possession[i];
    }
#else
    for (int i = 1; i < 17; ++i) {
        interval_total += interval_possession[i];
    }
#endif

    if (interval_total == 0) {
        printf("Nothing to show for this interval.\n\n");
    } else {
        // team A
        double team_a_interval_poss = 0;
        printf("Ball possession team A\n");
#if IGNORE_GOALKEEPER
        for (int i = 2; i < 9; ++i) {
#else
        for (int i = 1; i < 9; ++i) {
#endif
            // compute percentage for this player
            double player_possession = (double) interval_possession[i] / interval_total * 100;
            // update total team stats
            team_a_interval_poss = team_a_interval_poss + player_possession;
            printf("%2d) %-20s -> %5.2f%%\n", i, player_names[i], player_possession);
        }
        printf("\nTotal: %5.2f%%\n\n", team_a_interval_poss);
        // team B
        double team_b_interval_poss = 0;
        printf("Ball possession team B:\n");
#if IGNORE_GOALKEEPER
        for (int i = 10; i < 17; ++i) {
#else
        for (int i = 9; i < 17; ++i) {
#endif
            // compute percentage for this player
            double player_possession = (double) interval_possession[i] / interval_total * 100;
            // update total team stats
            team_b_interval_poss = team_b_interval_poss + player_possession;
            printf("%2d) %-20s -> %5.2f%%\n", i, player_names[i], player_possession);
        }
        printf("\nTotal: %5.2f%%\n\n", team_b_interval_poss);
    }

    // cumulative statistics
    printf("== Up to now ==\n\n");

    // compute total possession for the game to make percentages
    unsigned game_total = 0;
#if IGNORE_GOALKEEPER
    for (int i = 2; i < 9; ++i) {
        game_total = game_total + total_possession[i];
    }
    for (int i = 10; i < 17; ++i) {
        game_total = game_total + total_possession[i];
    }
#else
    for (int i = 1; i < 17; ++i) {
        game_total = game_total + total_possession[i];
    }
#endif

    if (game_total == 0) {
        printf("Waiting for the game to start...\n");
    } else {
        // team A
        double team_a_total_poss = 0;
        printf("Ball possession team A\n");
#if IGNORE_GOALKEEPER
        for (int i = 2; i < 9; ++i) {
#else
        for (int i = 1; i < 9; ++i) {
#endif
            // compute percentage for this player
            double player_possession = (double) total_possession[i] / game_total * 100;
            // update total team stats
            team_a_total_poss = team_a_total_poss + player_possession;
            printf("%2d) %-20s -> %5.2f%%\n", i, player_names[i], player_possession);
        }
        printf("\nTotal: %5.2f%%\n\n", team_a_total_poss);
        // team B
        double team_b_total_poss = 0;
        printf("Ball possession team B:\n");
#if IGNORE_GOALKEEPER
        for (int i = 10; i < 17; ++i) {
#else
        for (int i = 9; i < 17; ++i) {
#endif
            // compute percentage for this player
            double player_possession = (double) total_possession[i] / game_total * 100;
            // update total team stats
            team_b_total_poss = team_b_total_poss + player_possession;
            printf("%2d) %-20s -> %5.2f%%\n", i, player_names[i], player_possession);
        }
        printf("\nTotal: %5.2f%%\n\n", team_b_total_poss);
    }
}


void output_run(MPI_Datatype mpi_output_envelope, picoseconds T) {

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
    output_envelope data;      // used to receive a message, same size as the mpi datatype

    while (1) {

        // match the first process with a message
        DBG(("OUTPUT: waiting for a message from ONEVENT or POSSESSION\n"));
        MPI_Recv(&data, 1, mpi_output_envelope, MPI_ANY_SOURCE, interval, MPI_COMM_WORLD, MPI_STATUS_IGNORE);


        // check type of message
        switch (data.type) {
            case POSSESSION_MESSAGE:
                DBG(("OUTPUT: possession message received from POSSESSION, interval %d, holder %d\n", interval, data.content));

                // get player with possession for this sample
                holder = data.content;

                // update possession arrays
                interval_possession[holder] += 1;
                total_possession[holder] += 1;
                DBG(("OUTPUT: new possession for %d=%d", holder, interval_possession[holder]));
                // update count of possession processed for this interval
                num_read += 1;
                break;

            case PRINT_MESSAGE:
                DBG(("OUTPUT: print message received from ONEVENT, interval %d, numthreads %d\n", interval, data.content));

                // get number of possession updates we need to wait for
                num_processes = data.content;

                // collect messages from all pending processes
                while (num_read < num_processes) {
                    // wait for any possession process for this interval
                    MPI_Recv(&data, 1, mpi_output_envelope, MPI_ANY_SOURCE, interval, MPI_COMM_WORLD,
                             MPI_STATUS_IGNORE);
                    DBG(("OUTPUT: possession message received from POSSESSION, interval %d, holder %d\n", interval, data.content));

                    // get player with possession for this sample
                    holder = data.content;
                    // update possession arrays
                    interval_possession[holder] += 1;
                    total_possession[holder] += 1;
                    // update count of possession processed for this interval
                    num_read += 1;
                }

                // interval complete, output statistics
                print_statistics(interval_possession, total_possession, interval, T);

                // reset interval
                for (int i = 0; i < 17; i++) {
                    interval_possession[i] = 0;
                }
                num_read = 0;
                interval += 1;

                break;

            case ENDOFGAME_MESSAGE:
                DBG(("OUTPUT: endofgame message received from ONEVENT\n"));
                // exit from the process
                return;

            default:
                printf("Message with wrong type %u in the \"output\" process!\n", data.type);
                printf("Aborting.\n");
                MPI_Abort(MPI_COMM_WORLD, 1);
        }
    }
}
