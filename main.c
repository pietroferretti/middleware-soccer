
#include <stdint.h>
#include <stdlib.h>

/*
#define TEAM_A_SIDS {13, 14, 97, 98, 47, 16, 49, 88, 19, 52, 53, 54, 23, 24, 57, 58, 59, 28}
#define TEAM_B_SIDS {61, 62, 99, 100, 63, 64, 65, 66, 67, 68, 69, 38, 71, 40, 73, 74, 75, 44}
#define BALL_SIDS {4, 8, 10, 12}
#define REFEREE_SIDS {105, 106}
*/

typedef uint8_t sid_t;
typedef uint8_t player_t;
typedef enum {PLAYER, REFEREE, BALL, NONE} sensor_type_t;
typedef enum {TEAM_A, TEAM_B} team_t;

sensor_type_t sensor_type_list[] = {NONE, NONE, NONE, NONE, BALL, NONE, NONE, NONE, BALL, NONE, BALL, NONE, BALL, PLAYER, PLAYER, NONE, PLAYER, NONE, NONE, PLAYER, NONE, NONE, NONE, PLAYER, PLAYER, NONE, NONE, NONE, PLAYER, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, PLAYER, NONE, PLAYER, NONE, NONE, NONE, PLAYER, NONE, NONE, PLAYER, NONE, PLAYER, NONE, NONE, PLAYER, PLAYER, PLAYER, NONE, NONE, PLAYER, PLAYER, PLAYER, NONE, PLAYER, PLAYER, PLAYER, PLAYER, PLAYER, PLAYER, PLAYER, PLAYER, PLAYER, NONE, PLAYER, NONE, PLAYER, PLAYER, PLAYER, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, PLAYER, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, PLAYER, PLAYER, PLAYER, PLAYER, NONE, NONE, NONE, NONE, REFEREE, REFEREE};
player_t sensor_player_list[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 2, 0, 0, 4, 0, 0, 0, 6, 6, 0, 0, 0, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 13, 0, 14, 0, 0, 0, 16, 0, 0, 2, 0, 3, 0, 0, 4, 5, 5, 0, 0, 7, 7, 8, 0, 9, 9, 10, 10, 11, 11, 12, 12, 13, 0, 14, 0, 15, 15, 16, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 9, 9}

sensor_type_t get_sensor_type(sid_t sid) {
    sensor_type_t type = sensor_type_list[sid]
    if (type != -1) {
        return type;
    }
    // something went wrong
    // TODO print su stderr?
    exit(-1);
}

player_t get_sensor_player(sid_t sid) {
    player_t player = sensor_player_list[sid];
    if (player != -1) {
        return player;
    }
    // something went wrong
    exit(-1);
}

team_t get_player_team(player_t player) {
    if (player >= 1 && player <= 8) {
        return TEAM_A;
    }
    else if (player >= 9 && player <= 16) {
        return TEAM_B;
    }
    else {
        // something went wrong
        exit(-1);
    }
}

void print_statistics() {
    // leggi array intervallo (players team 1, players team 2)
}

int main() {

    uint64_t interval_possession_team1[8] = {0, 0, 0, 0, 0, 0, 0, 0}, total_possession_team1[8] = {0, 0, 0, 0, 0, 0, 0,
                                                                                                   0};
    uint64_t interval_possession_team2[8] = {0, 0, 0, 0, 0, 0, 0, 0}, total_possession_team2[8] = {0, 0, 0, 0, 0, 0, 0,
                                                                                                   0};



    // output

    // TODO variabile timestamp precedente
    // se timestamp > timestamp precedente + T, output


    return 0;
}