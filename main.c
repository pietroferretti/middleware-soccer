
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

//#define PRGDEBUG 1

#ifdef PRGDEBUG
#define DBG(x) printf x
#else
#define DBG(x) /*nothing*/
#endif

#define GAME_START 10753295594424116
#define GAME_END 14879639146403495
#define DATASET_PATH "full-game" //selezionare la working directory dalle config di build per farlo funzionare

#define INTERVAL 60000000000000

/*
#define TEAM_A_SIDS {13, 14, 97, 98, 47, 16, 49, 88, 19, 52, 53, 54, 23, 24, 57, 58, 59, 28}
#define TEAM_B_SIDS {61, 62, 99, 100, 63, 64, 65, 66, 67, 68, 69, 38, 71, 40, 73, 74, 75, 44}
#define BALL_SIDS {4, 8, 10, 12}
#define REFEREE_SIDS {105, 106}
*/

typedef uint8_t sid_t;
typedef uint8_t player_t;
typedef uint64_t picoseconds;
typedef enum {
    PLAYER, REFEREE, BALL, NONE
} sensor_type_t;
typedef enum {
    TEAM_A, TEAM_B
} team_t;

typedef struct position {
    int x;
    int y;
    int z;
} position;

typedef struct event {
    sid_t sid;
    uint64_t ts;
    position p;
} event;


sensor_type_t sensor_type_list[] = {NONE, NONE, NONE, NONE, BALL, NONE, NONE, NONE, BALL, NONE, BALL, NONE, BALL,
                                    PLAYER, PLAYER, NONE, PLAYER, NONE, NONE, PLAYER, NONE, NONE, NONE, PLAYER, PLAYER,
                                    NONE, NONE, NONE, PLAYER, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE,
                                    PLAYER, NONE, PLAYER, NONE, NONE, NONE, PLAYER, NONE, NONE, PLAYER, NONE, PLAYER,
                                    NONE, NONE, PLAYER, PLAYER, PLAYER, NONE, NONE, PLAYER, PLAYER, PLAYER, NONE,
                                    PLAYER, PLAYER, PLAYER, PLAYER, PLAYER, PLAYER, PLAYER, PLAYER, PLAYER, NONE,
                                    PLAYER, NONE, PLAYER, PLAYER, PLAYER, NONE, NONE, NONE, NONE, NONE, NONE, NONE,
                                    NONE, NONE, NONE, NONE, NONE, PLAYER, NONE, NONE, NONE, NONE, NONE, NONE, NONE,
                                    NONE, PLAYER, PLAYER, PLAYER, PLAYER, NONE, NONE, NONE, NONE, REFEREE, REFEREE};
player_t sensor_player_list[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 2, 0, 0, 4, 0, 0, 0, 6, 6, 0, 0, 0, 8,
                                 0, 0, 0, 0, 0, 0, 0, 0, 0, 13, 0, 14, 0, 0, 0, 16, 0, 0, 2, 0, 3, 0, 0, 4, 5, 5, 0, 0,
                                 7, 7, 8, 0, 9, 9, 10, 10, 11, 11, 12, 12, 13, 0, 14, 0, 15, 15, 16, 0, 0, 0, 0, 0, 0,
                                 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 9, 9};


// ******************************** GLOBAL **************************************************

picoseconds interval_possession[17] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
picoseconds total_possession[17] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
picoseconds interval_start, interval_ends;

position ball_last_position;
event last_ball_holder;


// *****************************************************************************************


sensor_type_t get_sensor_type(sid_t sid) {
    sensor_type_t type = sensor_type_list[sid];
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
    } else if (player >= 9 && player <= 16) {
        return TEAM_B;
    } else {
        // something went wrong
        exit(-1);
    }
}


void print(event e) {
    printf("\n%u %lu %d %d %d", e.sid, e.ts, e.p.x, e.p.y, e.p.z);
}

void readEvent(FILE *file, event *new) {
    fscanf(file, "%u%*c %lu%*c %d%*c %d%*c %d%*c %*s %*s %*s %*s %*s %*s %*s %*s", &new->sid, &new->ts, &new->p.x,
           &new->p.y, &new->p.z);
}

void print_statistics() {
    printf("\nBall possession team A\n");
    int i;
    for (i = 1; i < 9; ++i) {
        printf("Player %d: %f\t", i, (double) interval_possession[i] / INTERVAL * 100);
    }
    printf("\nBall possession team B:\n");
    for (; i < 17; ++i) {
        printf("Player %d: %f\t", i, (double) interval_possession[i] / INTERVAL * 100);
    }
}

//aggiungo i psec (fino all'evento attuale) al previous ball_holder fin
void update_possession(event e) {

    if (last_ball_holder.ts < interval_start)
        interval_possession[get_sensor_player(last_ball_holder.sid)] += e.ts - interval_start;

    else
        interval_possession[get_sensor_player(last_ball_holder.sid)] += e.ts - last_ball_holder.ts;

    DBG(("\npossession: "));
    for (int j = 1; j < 17; ++j) {
        DBG(("%lu ", interval_possession[j]));
    }
}

double squareDistanceFromBall(position player_position) {
//    d = ((x2 - x1)2 + (y2 - y1)2 + (z2 - z1)2)1/2
    //se dobbiamo solo fare dei confronti non ci serve la radice => più efficiente senza
    return ((player_position.x - ball_last_position.x) * (player_position.x - ball_last_position.x) +
            (player_position.y - ball_last_position.y) * (player_position.y - ball_last_position.y) +
            (player_position.z - ball_last_position.z) * (player_position.z - ball_last_position.z));
}


int main() {


    FILE *fp = fopen(DATASET_PATH, "r");

    if (fp == NULL) {
        printf("Error: file pointer is null.");
        exit(1);
    }

    event current_event;
    ball_last_position.x = 0;
    ball_last_position.y = 0;
    ball_last_position.z = 0;

    event null_player;
    null_player.sid = 0; //fixme spero non crei conflitti

    last_ball_holder = null_player;

    interval_start = GAME_START;
    interval_ends = GAME_START + INTERVAL;

    DBG(("\nball starting position: %d, %d, %d", ball_last_position.x, ball_last_position.y, ball_last_position.z));

    while (!feof(fp)) {

        readEvent(fp, &current_event);

        while (current_event.ts < GAME_END) {

            if (current_event.ts < GAME_START) {
//            printf("\nnot yet");

            } else {

                if (current_event.ts > interval_ends) {

                    interval_possession[get_sensor_player(last_ball_holder.sid)] += interval_ends - last_ball_holder.ts;

                    //stampa stats, azzera interval stats, new interval ends
                    print_statistics();

                    for (int i = 1; i < 17; ++i) {
                        total_possession[i] += interval_possession[i];
                        interval_possession[i] = 0;
                    }
                    interval_start = interval_ends;
                    interval_ends += INTERVAL;

                }

//                print(current_event);
                switch (get_sensor_type(current_event.sid)) {
                    case BALL:
                        DBG(("\nball"));
                        ball_last_position = current_event.p;
                        DBG(("\tnew position: %d, %d, %d", ball_last_position.x, ball_last_position.y, ball_last_position.z));
                        break;
                    case PLAYER:
                        if (last_ball_holder.sid != null_player.sid) {
                            DBG(("\nplayer %d", get_sensor_player(current_event.sid)));
                            if (squareDistanceFromBall(current_event.p) < squareDistanceFromBall(last_ball_holder.p)) {

                                update_possession(current_event);


                                last_ball_holder = current_event;
                            } else {
                                update_possession(last_ball_holder);
                            }
                        } else {
                            DBG(("\nFIRST! player %d", get_sensor_player(current_event.sid)));

                            last_ball_holder = current_event;
                        }
                        break;
                    default:
//                        DBG(("\ndefault %d", get_sensor_type(current_event.sid)));
                        break;
                }


            }
            readEvent(fp, &current_event);

        }

        printf("\nBall possession team A\n");
        int i;
        for (i = 1; i < 9; ++i) {
            printf("Player %d: %f\t", i, (double) total_possession[i] / (GAME_END - GAME_START) * 100);
        }
        printf("\nBall possession team B:\n");
        for (; i < 17; ++i) {
            printf("Player %d: %f\t", i, (double) total_possession[i] / (GAME_END - GAME_START) * 100);
        }


    }


    fclose(fp);


    return 0;
}