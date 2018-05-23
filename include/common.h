#ifndef MIDDLEWARE_SOCCER_COMMON_H
#define MIDDLEWARE_SOCCER_COMMON_H

#include <stdint.h>
#include <stdbool.h>


#define PRGDEBUG 1

#ifdef PRGDEBUG
#define DBG(x) printf x
#else
#define DBG(x) /*nothing*/
#endif

#define PARSER_RANK 0
#define ONEVENT_RANK 1
#define POSSESSION_RANK 2
#define OUTPUT_RANK 3

#define EVENT_MESSAGE 0
#define INTERRUPTION_MESSAGE 1
#define POSITIONS_MESSAGE 2
#define PRINT_MESSAGE 3
#define POSSESSION_MESSAGE 4
#define ENDOFGAME_MESSAGE 5


#define INTERVAL 60000000000000  // 60 seconds
#define K 1000  // 1 meter

#define SECTOPIC 1000000000000

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

typedef struct hold_event {
    player_t player;
    picoseconds ts;
    position p;
} hold_event;

typedef struct interruption_event {
    picoseconds start;
    picoseconds end;
} interruption_event;


#endif //MIDDLEWARE_SOCCER_COMMON_H
