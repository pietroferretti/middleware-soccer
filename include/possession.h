/**
 * @file possession.h
 *
 * @brief possession.c function declaration.
 *
 */

#ifndef MIDDLEWARE_SOCCER_POSSESSION_H
#define MIDDLEWARE_SOCCER_POSSESSION_H


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
void possession_run(MPI_Datatype mpi_possession_envelope, MPI_Datatype mpi_output_envelope, unsigned long K);

#endif //MIDDLEWARE_SOCCER_POSSESSION_H

//double squareDistanceFromBall(position player_position, position ball_last_position);