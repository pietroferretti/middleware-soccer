/**
 * @file common.h
 *
 * @brief Common constants and definitions.
 *
 * This file contains type definitions and global constants used by processes.
 */

#ifndef MIDDLEWARE_SOCCER_COMMON_H
#define MIDDLEWARE_SOCCER_COMMON_H

#include <stdint.h>
#include <mpi.h>

#define PRGDEBUG 0

#if PRGDEBUG
#define DBG(x) printf x
#else
#define DBG(x) /*nothing*/
#endif

#define FULLGAME_PATH "../datasets/full-game"

#define FIRST_INTERRUPTIONS "../datasets/referee-events/Game Interruption/1st Half.csv"
#define SECOND_INTERRUPTIONS "../datasets/referee-events/Game Interruption/2nd Half.csv"

// Field dimensions
#define XMIN 0
#define XMAX 52483
#define YMIN (-33960)
#define YMAX 33965

/**
 * Conversion factor from seconds to picosends.
 */
#define SECTOPIC 1000000000000

#define GAME_START 10753295594424116
#define FIRST_END 12557295594424116
#define SECOND_START 13086639146403495
#define GAME_END 14879639146403495

#define PARSER_RANK 0
#define OUTPUT_RANK 1
#define POSSESSION_RANK 2

#define POSITIONS_MESSAGE 0
#define PRINT_MESSAGE 1
#define POSSESSION_MESSAGE 2
#define ENDOFGAME_MESSAGE 3

#define POSSESSION_BUFFER_SIZE 1

#define IGNORE_GOALKEEPER 0

/**
 * @brief Sensor id type.
 */
typedef uint32_t sid_t;
/**
 * @brief Player type.
 */
typedef uint32_t player_t;
typedef uint64_t picoseconds;

/**
 * @brief Each sensor register data from a specific PLAYER, from the REFEREE or from the BALL; NONE applied when none of the previous case holds.
 */
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
 * end of the interruption. During an interruption event statistics are updated.
 */
typedef struct interruption_event {
    picoseconds start;
    picoseconds end;
} interruption_event;

/**
 * @brief It show a game snapshot.
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


typedef struct {
    uint32_t type;
    uint32_t content;
} output_envelope;

#endif //MIDDLEWARE_SOCCER_COMMON_H
