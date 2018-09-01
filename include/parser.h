/**
 * @file parser.h
 *
 *@brief parser.c function declaration.
 *
 */

#ifndef MIDDLEWARE_SOCCER_PARSER_H
#define MIDDLEWARE_SOCCER_PARSER_H

/**
 * @brief It receives events from all sensors and communicates with output and possession processes.
 *
 * Start, end and interruptions of the game are highlighted in the data received by the process.
 *
 * @param mpi_position_for_possession_type
 * @param mpi_output_envelope
 * @param possession_processes
 * @param INTERVAL
 */
void
parser_run(MPI_Datatype mpi_position_for_possession_type, MPI_Datatype mpi_output_envelope, int possession_processes,
           picoseconds INTERVAL, char * fullgame_path, char * interr_path_one, char * interr_path_two);
// FIXME sistemare la documentazione

/**
 * @brief Given the sensor id it return the sensor type.
 * @param sid Sensor id.
 * @return Sensor type; NONE, BALL, PLAYER or REFEREE.
 */
sensor_type_t get_sensor_type(sid_t sid);

/**
 * @brief Returns TRUE if the given position is within the field.
 * @param p Ball position.
 *
 */
bool ball_is_in_play(position p);

/**
 * @brief It reads a new event from file and store read data in the new event.
 * object.
 *
 */
void readEvent(FILE *file, event *new);

/**
 * @brief It reads a new interruption event from file and store read data in the new interruption_event.
 * @param start Start time of the first of second half of the game, it's used to choose which file to read from.
 */
void readInterruptionEvent(FILE **file, struct interruption_event *new, picoseconds start);


#endif //MIDDLEWARE_SOCCER_PARSER_H
