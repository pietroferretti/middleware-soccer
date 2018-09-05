/**
 * @file parser.h
 *
 *@brief parser.c function declaration.
 *
 */

#ifndef MIDDLEWARE_SOCCER_PARSER_H
#define MIDDLEWARE_SOCCER_PARSER_H

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
void
parser_run(MPI_Datatype mpi_position_for_possession_type, MPI_Datatype mpi_output_envelope, int possession_processes,
           picoseconds T, char *fullgame_path, char *interr_path_one, char *interr_path_two);

#endif //MIDDLEWARE_SOCCER_PARSER_H
