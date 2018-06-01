#ifndef MIDDLEWARE_SOCCER_COMMON_H
#define MIDDLEWARE_SOCCER_COMMON_H

#include <stdint.h>
#include <stdbool.h>
#include <mpi.h>

#define PRGDEBUG 0

#if PRGDEBUG
#define DBG(x) printf x
#else
#define DBG(x) /*nothing*/
#endif

#define FULLGAME_PATH "datasets/full-game"

#define FIRST_INTERRUPTIONS "datasets/referee-events/Game Interruption/1st Half.csv"
#define SECOND_INTERRUPTIONS "datasets/referee-events/Game Interruption/2nd Half.csv"

#define XMIN 0
#define XMAX 52483
#define YMIN (-33960)
#define YMAX 33965

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

#define IGNORE_GOALKEEPER 1

typedef uint32_t sid_t;
typedef uint32_t player_t;
typedef uint64_t picoseconds;
typedef enum {
    PLAYER, REFEREE, BALL, NONE
} sensor_type_t;

typedef struct position {
    int32_t x;
    int32_t y;
    int32_t z;
} position;

typedef struct event {
    sid_t sid;
    picoseconds ts;
    position p;
} event;

typedef struct interruption_event {
    picoseconds start;
    picoseconds end;
} interruption_event;

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
