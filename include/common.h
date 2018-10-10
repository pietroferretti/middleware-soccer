/**
 * @file common.h
 *
 * @brief Common constants and definitions.
 *
 * This file contains type definitions and global constants used by all processes.
 */

#ifndef MIDDLEWARE_SOCCER_COMMON_H
#define MIDDLEWARE_SOCCER_COMMON_H

#include <stdint.h>
#include <mpi.h>

/// Enables additional debug output
#define PRGDEBUG 0

#if PRGDEBUG
#define DBG(x) printf x
#else
#define DBG(x) /*nothing*/
#endif

/// Field dimensions
//@{
#define XMIN 0
#define XMAX 52483
#define YMIN (-33960)
#define YMAX 33965
//@}

/// Conversion factor from seconds to picoseconds.
#define SECTOPIC 1000000000000

/// Beginnings and ends of each half of the game
//@{
#define GAME_START 10753295594424116
#define FIRST_END 12557295594424116
#define SECOND_START 13086639146403495
#define GAME_END 14879639146403495
//@}

/// Process identifiers
//@{
#define PARSER_RANK 0
#define OUTPUT_RANK 1
#define POSSESSION_RANK 2
//@}

/// Message type identifiers, defined as integers for use with MPI
//@{
#define POSITIONS_MESSAGE 0
#define PRINT_MESSAGE 1
#define POSSESSION_MESSAGE 2
#define ENDOFGAME_MESSAGE 3
//@}

/// Dimension of the results buffer to increase pipelining
#define POSSESSION_BUFFER_SIZE 1

/// Set to 1 to remove the goalkeepers from the statistics
#define IGNORE_GOALKEEPER 0

/// Sensor id type
typedef uint32_t sid_t;
/// Player type
typedef uint32_t player_t;
/// Picoseconds type
typedef uint64_t picoseconds;

/// Each sensor registers data from a specific PLAYER, from the REFEREE or from the BALL; NONE as default case
typedef enum {
    PLAYER, REFEREE, BALL, NONE
} sensor_type_t;

/**
 * @brief Position coordinates in the game field.
 *
 * x, y, z describe the position of the sensor in mm and the origin is the
 * middle of a full size football field.
 */
typedef struct position {
    int32_t x;
    int32_t y;
    int32_t z;
} position;

/**
 * @brief Event from sensor.
 *
 * Each event is characterized by:
 * - the id of the sensor which has generated it,
 * - a timestamp,
 * - the registered position.
 */
typedef struct event {
    sid_t sid;
    picoseconds ts;
    position p;
} event;

/**
 * @brief Interruption event.
 *
 * Each interruption_event is characterized by: the timestamps of beginning and
 * end of the interruption. During an interruption event statistics are not updated.
 */
typedef struct interruption_event {
    picoseconds start;
    picoseconds end;
} interruption_event;

/**
 * @brief Shows a game snapshot.
 *
 * It is characterized by:
 * - an array with every player position,
 * - the ball position,
 * - the specific id of the interval in which the snapshot was taken.
 */
typedef struct position_event {
    position players[17];      // 0 = discard, 1-16 = player
    position ball;
    int32_t interval_id;
} position_event;

/**
 * @brief Used to send messages to the output process.
 *
 * To avoid using multiple messages to know which MPI datatype the next message will be,
 * we use a generic type that works for all the messages that we want to send,
 * i.e. the print message from the parser and the result message from possession.
 * Message types can be #POSITIONS_MESSAGE, #PRINT_MESSAGE, #POSSESSION_MESSAGE
 * and #ENDOFGAME_MESSAGE.
 */
typedef struct {
    uint32_t type;
    uint32_t content;
} output_envelope;

#endif //MIDDLEWARE_SOCCER_COMMON_H
