/**
 * @file output.h
 *
 *@brief output.c function declaration.
 *
 */

#ifndef MIDDLEWARE_SOCCER_OUTPUT_H
#define MIDDLEWARE_SOCCER_OUTPUT_H



/**
 * @brief Core of output's job.
 *
 * It keeps waiting for a PRINT_MESSAGE or a
 * POSSESSION_MESSAGE, from onevent or possession processes, until receiving
 * the END_OF_GAME message.
 * After receiving a POSSESSION_MESSAGE, statistics are updated;
 * after receiving a PRINT_MESSAGE, interval and cumulative statistics are
 * printed, by calling #print_statistics method, and interval ones are reset;
 * after receiving the END_OF_GAME message, the process exits, after waiting
 * for any pending request.
 * If the received message is of any other type, the process abort.
 *
 * @param mpi_output_envelope mpi_datatype of received messages.
 */
void output_run(MPI_Datatype mpi_output_envelope, picoseconds T);


/**
 * @brief It prints the interval header with the current game time.
 * @param interval Current interval id.
 * @param T Interval length (in picoseconds).
 */

void print_interval(int interval, picoseconds T);

/**
 * @brief It prints for every team and every member last interval statistic, followed
 * by current cumulative statistics.
 * @param interval_possession Array with last interval statistics
 * for every player (each identified by a constant position in the
 * array).
 * @param total_possession Array with cumulative statistics for every
 * player (each identified by a constant position in the array).
 * @param interval Incrementing value used to identify each interval of time.
 */

void print_statistics(const unsigned int *interval_possession, const unsigned int *total_possession, int interval,
                      picoseconds T);

#endif //MIDDLEWARE_SOCCER_OUTPUT_H
