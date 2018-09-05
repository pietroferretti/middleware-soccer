/**
 * @file output.h
 *
 *@brief output.c function declaration.
 *
 */

#ifndef MIDDLEWARE_SOCCER_OUTPUT_H
#define MIDDLEWARE_SOCCER_OUTPUT_H


/**
 * @brief Starts the output process.
 *
 * It keeps waiting for a PRINT_MESSAGE or a
 * POSSESSION_MESSAGE, from possession processes, until it receives
 * the END_OF_GAME message.
 * After receiving a POSSESSION_MESSAGE, statistics are updated;
 * after receiving a PRINT_MESSAGE, interval and cumulative statistics are
 * printed, and interval ones are reset;
 * after receiving the END_OF_GAME message, the process exits, after waiting
 * for any pending request.
 * If the received message is of any other type, the process abort.
 *
 * @param mpi_output_envelope MPI datatype of the received messages.
 * @param T Length of time between outputs
 */
void output_run(MPI_Datatype mpi_output_envelope, picoseconds T);

#endif //MIDDLEWARE_SOCCER_OUTPUT_H
