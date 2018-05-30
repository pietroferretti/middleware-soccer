#ifndef MIDDLEWARE_SOCCER_COMMON_H
#define MIDDLEWARE_SOCCER_COMMON_H

#include <stdint.h>
#include <stdbool.h>
#include <mpi.h>

#define PRGDEBUG 1

#ifdef PRGDEBUG
#define DBG(x) printf x
#else
#define DBG(x) /*nothing*/
#endif

#define GAME_START 10753295594424116
#define FIRST_END 12557295594424116
#define SECOND_START 13086639146403495
#define GAME_END 14879639146403495

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

#define POSSESSION_BUFFER_SIZE 100
#define ONEVENT_BUFFER_SIZE 100
#define PARSER_BUFFER_SIZE 100

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
