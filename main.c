
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <mpi/mpi.h>
#include <stddef.h>

//#define PRGDEBUG

#ifdef PRGDEBUG
#define DBG(x) printf x
#else
#define DBG(x) /*nothing*/
#endif

#define GAME_START 10753295594424116
#define FIRST_END 12557295594424116
#define SECOND_START 13086639146403495
#define GAME_END 14879639146403495
#define XMIN 0
#define XMAX 52483
#define YMIN (-33960)
#define YMAX 33965
#ifdef PRGDEBUG
#define FULLGAME_PATH "stripped-game" //selezionare la working directory dalle config di build per farlo funzionare
#else
#define FULLGAME_PATH "full-game" //selezionare la working directory dalle config di build per farlo funzionare
#endif
#define FIRST_INTERRUPTIONS "referee-events/Game Interruption/1st Half.csv"
#define SECOND_INTERRUPTIONS "referee-events/Game Interruption/2nd Half.csv"

#define INTERVAL 60000000000000  // 60 seconds
#define K 1000  // 1 meter

#define SECTOPIC 1000000000000

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

//picoseconds interval_possession[17] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
//picoseconds total_possession[17] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
//picoseconds interval_start, interval_ends;
//
//position ball_last_position;
//event last_ball_holder;


// *****************************************************************************************


sensor_type_t get_sensor_type(sid_t sid) {
    sensor_type_t type = sensor_type_list[sid];
    if (type != -1) {
        return type;
    }
    fprintf(stderr, "Unknown sensor %u", sid);
    exit(1);
}

player_t get_sensor_player(sid_t sid) {
    player_t player = sensor_player_list[sid];
    if (player != -1) {
        return player;
    }
    fprintf(stderr, "Unknown player sensor %u", sid);
    exit(1);
}

//team_t get_player_team(player_t player) {
//    if (player >= 1 && player <= 8) {
//        return TEAM_A;
//    } else if (player >= 9 && player <= 16) {
//        return TEAM_B;
//    } else {
//        fprintf(stderr, "Unknown player %u", player);
//        exit(1);
//    }
//}


//void print(event e) {
//    printf("\n%u %lu %d %d %d", e.sid, e.ts, e.p.x, e.p.y, e.p.z);
//}

void readEvent(FILE *file, event *new) {
    fscanf(file, "%u,%lu,%d,%d,%d,%*s\n", &new->sid, &new->ts, &new->p.x,
           &new->p.y, &new->p.z);
}

void readInterruptionEvent(FILE **file, struct interruption_event *new, picoseconds start) {
    //fixme fine file è diverso
    picoseconds minutes;
    double seconds;
    int a;

    int read = fscanf(*file, "%*31c:%lu:%lf;%*s\n", &minutes, &seconds);

    if (read) {
        new->start = start + (picoseconds) (seconds * SECTOPIC) + (minutes * 60) * SECTOPIC;
        DBG(("\nSTART INTERR %d,%f", minutes, seconds));

        fscanf(*file, "%*29c:%lu:%lf;%*s\n", &minutes, &seconds);


        DBG(("\nEND INTERR %d,%f", minutes, seconds));

        new->end = start + (picoseconds) (seconds * SECTOPIC) + (minutes * 60) * SECTOPIC;
    } else if (start == SECOND_START) {
//        fclose(*file);
        new->start = GAME_END;
        new->end = GAME_END;
        return;

    } else {
        fclose(*file);
        *file = fopen(SECOND_INTERRUPTIONS, "r");
        fscanf(*file, "%*s\n");
        fscanf(*file, "%*s %*s %*s\n");
        fscanf(*file, "%*29c:%lu:%lf;%*s\n", &minutes, &seconds);
        new->start = start;
        new->end =
                start + (picoseconds) (seconds * SECTOPIC) + (picoseconds) (minutes * 60) * SECTOPIC;


    }
}


void print_statistics(picoseconds const interval_possession[], picoseconds const total_possession[]) {
    // statistics for the interval

    picoseconds time_played = 0;
    for (int j = 1; j < 17; ++j) {
        time_played += interval_possession[j];
    }

    if (time_played == 0) {
        printf("\nGame interrupted. (in print)");

        return;
    }


    printf("\n\n== Current Interval ==\n  %lu", time_played);
    double team_a_interval_poss = 0;
    double team_b_interval_poss = 0;
    printf("\nBall possession team A\n");
    for (int i = 1; i < 9; ++i) {
        double player_possession = (double) interval_possession[i] / time_played * 100;
        team_a_interval_poss = team_a_interval_poss + player_possession;   // update total team stats
        printf("Player %2d: %5.2f%%\t", i, player_possession);
    }
    printf("\nTotal: %5.2f%%\n", team_a_interval_poss);
    printf("\nBall possession team B:\n");
    for (int i = 9; i < 17; ++i) {
        double player_possession = (double) interval_possession[i] / time_played * 100;
        team_b_interval_poss = team_b_interval_poss + player_possession;   // update total team stats
        printf("Player %2d: %5.2f%%\t", i, player_possession);
    }
    printf("\nTotal: %5.2f%%\n", team_b_interval_poss);

    // cumulative statistics
    printf("\n== Up to now ==");
    double team_a_total_poss = 0;
    double team_b_total_poss = 0;
    picoseconds total_possession_time = 0;
    for (int i = 1; i < 17; ++i) {
        total_possession_time = total_possession_time + total_possession[i];
    }
    printf("\nBall possession team A\n");
    for (int i = 1; i < 9; ++i) {
        double player_possession = (double) total_possession[i] / total_possession_time * 100;
        team_a_total_poss = team_a_total_poss + player_possession;   // update total team stats
        printf("Player %2d: %5.2f%%\t", i, player_possession);
    }
    printf("\nTotal: %5.2f%%\n", team_a_total_poss);
    printf("\nBall possession team B:\n");
    for (int i = 9; i < 17; ++i) {
        double player_possession = (double) total_possession[i] / total_possession_time * 100;
        team_b_total_poss = team_b_total_poss + player_possession;   // update total team stats
        printf("Player %2d: %5.2f%%\t", i, player_possession);
    }
    printf("\nTotal: %5.2f%%\n", team_b_total_poss);
}


void update_possession(picoseconds interval_possession[], event e, hold_event last_ball_holder) {
    // update array with possession times
    interval_possession[last_ball_holder.player] += e.ts - last_ball_holder.ts;

    DBG(("\npossession: "));
    for (int j = 1; j < 17; ++j) {
        DBG(("%lu ", interval_possession[j]));
    }
}

double squareDistanceFromBall(position player_position, position ball_last_position) {
//    d = ((x2 - x1)2 + (y2 - y1)2 + (z2 - z1)2)1/2
    // we only need this to compare distances, therefore we can skip the square root
    return ((player_position.x - ball_last_position.x) * (player_position.x - ball_last_position.x) +
            (player_position.y - ball_last_position.y) * (player_position.y - ball_last_position.y) +
            (player_position.z - ball_last_position.z) * (player_position.z - ball_last_position.z));
}

bool ball_is_in_play(position p) {
    return p.x >= XMIN && p.x <= XMAX && p.y >= YMIN && p.y <= YMAX;
}


int main() {

    // inizializza mpi
    MPI_Init(NULL, NULL);

    // create struct for position
    int blocklengths[3] = {1, 1, 1};
    MPI_Datatype types[3] = {MPI_INT32_T, MPI_INT32_T, MPI_INT32_T};
    MPI_Datatype mpi_position_type;
    MPI_Aint offsets[3] = {offsetof(position, x), offsetof(position, y), offsetof(position, z)};

    MPI_Type_create_struct(3, blocklengths, offsets, types, &mpi_position_type);
    MPI_Type_commit(&mpi_position_type);

// create struct for event
    int array_of_blocklengths[3] = {1, 1, 1};//    - number of elements in each block (array of integer)
    MPI_Aint offsets2[3] = {offsetof(event, sid), offsetof(event, ts), offsetof(event, p)};
    MPI_Datatype array_of_types[3] = {MPI_UNSIGNED_CHAR, MPI_UNSIGNED_LONG, mpi_position_type};
    MPI_Datatype mpi_event_type;

    MPI_Type_create_struct(3, array_of_blocklengths, offsets2, array_of_types, &mpi_event_type);
    MPI_Type_commit(&mpi_event_type);

    
    // open dataset
    FILE *fp_game = fopen(FULLGAME_PATH, "r");

    FILE *fp_interruption = fopen(FIRST_INTERRUPTIONS, "r");
//    FILE *fp_second_interruption = fopen(SECOND_INTERRUPTIONS, "r");

    if (fp_game == NULL || fp_interruption == NULL) {
        printf("Error: couldn't open file.");
        exit(1);
    }

    printf("\nGame starting..");

    // initialize ball possession statistics
    picoseconds interval_possession[17] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    picoseconds total_possession[17] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    // initialize variables used to check when the interval has ended
    picoseconds interval_ends;
    interval_ends = GAME_START + INTERVAL;

    // initialize ball possession values
    position ball_last_position;
    ball_last_position.x = 0;
    ball_last_position.y = 0;
    ball_last_position.z = 0;
    hold_event last_ball_holder;
    last_ball_holder.player = 0;

    // event cursor
    bool first_event = 1;
    event current_event;
    interruption_event next_interruption;
    picoseconds minutes;
    double seconds;
    fscanf(fp_interruption, "%*s\n");
    fscanf(fp_interruption, "%*s %*s %*s\n");
    fscanf(fp_interruption, "%*29c:%lu:%lf;%*s\n", &minutes, &seconds);
    next_interruption.start = GAME_START;
    DBG(("\nmin %lu, sec %lf", minutes, seconds));
    next_interruption.end =
            GAME_START + (picoseconds) (seconds * SECTOPIC) + (picoseconds) (minutes * 60) * SECTOPIC;

    DBG(("\nend interr %lu", next_interruption.end));

    DBG(("\nball starting position: %d, %d, %d", ball_last_position.x, ball_last_position.y, ball_last_position.z));

#ifdef PRGDEBUG
    for (int j = 0; j < 10; ++j) {
#else
    while (!feof(fp_game)) {
#endif

        // fetch one update
        readEvent(fp_game, &current_event);

        if (current_event.ts < GAME_START || (current_event.ts > FIRST_END && current_event.ts < SECOND_START)) {
            // skip until the game starts
            continue;
        }

        if (current_event.ts > GAME_END) {
            // the game has ended
            break;
        }





        // check if the interval has ended
        if (current_event.ts > interval_ends) {

            // update current statistics before printing them out
            interval_possession[last_ball_holder.player] += interval_ends - last_ball_holder.ts;
            last_ball_holder.ts = interval_ends;   // only consider the following interval in the next update
            for (int i = 1; i < 17; ++i) {
                total_possession[i] += interval_possession[i];
            }

            // output ball possession stats
            print_statistics(interval_possession, total_possession);

            // clear interval statistics
            for (int i = 1; i < 17; ++i) {
                interval_possession[i] = 0;
            }
            interval_ends += INTERVAL;
        }


        if (current_event.ts >= next_interruption.start && current_event.ts <= next_interruption.end) {
            // skip until the game restarts
//            DBG(("\nevent during interruption"));
            if (first_event) {

                printf("\nGame interrupted at %lu", next_interruption.start);
                interval_possession[last_ball_holder.player] += next_interruption.start
                                                                - last_ball_holder.ts;
                last_ball_holder.player = 0;
                last_ball_holder.ts = next_interruption.end;
                first_event = 0;
            }

            continue;
        } else if (current_event.ts > next_interruption.end) {

            printf("\nGame resume at %lu", next_interruption.end);
            if (current_event.ts < FIRST_END)
                readInterruptionEvent(&fp_interruption, &next_interruption, GAME_START);
            else if (current_event.ts > SECOND_START)
                readInterruptionEvent(&fp_interruption, &next_interruption, SECOND_START);

            first_event = 1;
            printf("\nnext interruption at: %lu", next_interruption.start);

        }

        // check who generated this new event
        switch (get_sensor_type(current_event.sid)) {

            case BALL:
                if (ball_is_in_play(current_event.p)) {
                    DBG(("\nball"));
                    // update ball position
                    ball_last_position = current_event.p;
                    DBG(("\tnew position: %d, %d, %d", ball_last_position.x, ball_last_position.y, ball_last_position.z));
                }
                break;

            case PLAYER:
                if (last_ball_holder.player != 0) {
                    DBG(("\nplayer %d", get_sensor_player(current_event.sid)));
                    // check if the player who generated the event is the one who is holding the ball
                    player_t event_player = get_sensor_player(current_event.sid);
                    if (event_player == last_ball_holder.player) {
                        // just update the position
                        last_ball_holder.p = current_event.p;
                        DBG(("\nholder position update %d, %d, %d", last_ball_holder.p.x, last_ball_holder.p.y, last_ball_holder.p.z));
                    } else {
                        // someone else, check if they can get possession of the ball
                        double event_distance = squareDistanceFromBall(current_event.p, ball_last_position);
                        double holder_distance = squareDistanceFromBall(last_ball_holder.p, ball_last_position);
                        if (event_distance < K * K && event_distance < holder_distance) {
                            // update possession statistics
                            update_possession(interval_possession, current_event, last_ball_holder);
                            // update ball holder
                            last_ball_holder.player = event_player;
                            last_ball_holder.p = current_event.p;
                            last_ball_holder.ts = current_event.ts;
                            DBG(("\nnew holder, position %d, %d, %d", last_ball_holder.p.x, last_ball_holder.p.y, last_ball_holder.p.z));
                        }
                    }

                } else {
                    DBG(("\nFIRST! player %d", get_sensor_player(current_event.sid)));
                    // first player event, initialize struct
                    last_ball_holder.player = get_sensor_player(current_event.sid);
                    last_ball_holder.ts = current_event.ts;
                    last_ball_holder.p = current_event.p;
                }
                break;
            default:
                // referee, ignore
//                        DBG(("\ndefault %d", get_sensor_type(current_event.sid)));
                break;
        }



//        printf("\nBall possession team A\n");
//        int i;
//        for (i = 1; i < 9; ++i) {
//            printf("Player %d: %f\t", i, (double) total_possession[i] / (GAME_END - GAME_START) * 100);
//        }
//        printf("\nBall possession team B:\n");
//        for (; i < 17; ++i) {
//            printf("Player %d: %f\t", i, (double) total_possession[i] / (GAME_END - GAME_START) * 100);
//        }

    }

    // close event file
    fclose(fp_game);
    fclose(fp_interruption);

    printf("\n** Done! **");
    return 0;
}



// read first event (after the game start)
// next_output_ts = event timestamp + T
// poss_player = event player
// poss_timestamp = event timestamp
// poss_distance = distance event position from ball

// while the game has not ended
// read one event
// check timestamp:
// check if after game start and before game end (and not during interval <- questo è fatto da referee events?)
// get player from sensore
// se player è poss_player:
// aggiorna poss_distance
// altrimenti:
// calcola distanza da palla
// se nuova distanza < poss_distance
// calcola il tempo di possesso palla del giocatore precedente
// aggiorna interval_possession e total_possession
// aggiorna poss_player, poss_timestamp, poss_distance
// se il timestamp >= next_output_ts:
// print output
// next_output_ts = next_output_ts + T

/// referee events
// quando leggo un game interruption
// clear tutti i possessi palla attuali, calcola il tempo di possesso dell'ultimo giocatore per liberare
// setto una flag,ignoro tutti gli eventi fino al prossimo game resuming da referee events
// ricomincio leggendo il primo evento