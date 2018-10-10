#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <stdbool.h>

#include "common.h"
#include "output.h"

/**
 * Indexes correspond to sensor ids: for each sensor its type is stored. Index
 * without an associated sensor id are stored as NONE.
 */
const sensor_type_t sensor_type_list[] = {NONE, NONE, NONE, NONE, BALL, NONE, NONE, NONE, BALL, NONE, BALL, NONE, BALL,
                                          PLAYER, PLAYER, NONE, PLAYER, NONE, NONE, PLAYER, NONE, NONE, NONE, PLAYER,
                                          PLAYER, NONE, NONE, NONE, PLAYER, NONE, NONE, NONE, NONE, NONE, NONE, NONE,
                                          NONE, NONE, PLAYER, NONE, PLAYER, NONE, NONE, NONE, PLAYER, NONE, NONE,
                                          PLAYER, NONE, PLAYER, NONE, NONE, PLAYER, PLAYER, PLAYER, NONE, NONE, PLAYER,
                                          PLAYER,
                                          PLAYER, NONE,
                                          PLAYER, PLAYER, PLAYER, PLAYER, PLAYER, PLAYER, PLAYER, PLAYER, PLAYER, NONE,
                                          PLAYER, NONE, PLAYER, PLAYER, PLAYER, NONE, NONE, NONE, NONE, NONE, NONE,
                                          NONE, NONE, NONE, NONE, NONE, NONE, PLAYER, NONE, NONE, NONE, NONE, NONE,
                                          NONE,
                                          NONE, NONE, PLAYER, PLAYER, PLAYER, PLAYER, NONE, NONE, NONE, NONE, REFEREE,
                                          REFEREE};

/**
 * Indexes correspond to sensor ids: for each sensor its player id is stored. Index
 * without an associated player id are stored as 0.
 */
const player_t sensor_player_list[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 2, 0, 0, 4, 0, 0, 0, 6, 6, 0, 0,
                                       0, 8,
                                       0, 0, 0, 0, 0, 0, 0, 0, 0, 13, 0, 14, 0, 0, 0, 16, 0, 0, 2, 0, 3, 0, 0, 4, 5, 5,
                                       0, 0,
                                       7, 7, 8, 0, 9, 9, 10, 10, 11, 11, 12, 12, 13, 0, 14, 0, 15, 15, 16, 0, 0, 0, 0,
                                       0, 0,
                                       0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 9, 9};


sensor_type_t get_sensor_type(sid_t sid) {
    if (sid >= 107) {
        fprintf(stderr, "Wrong sensor id %u in get_sensor_type", sid);
        exit(1);
    }
    sensor_type_t type = sensor_type_list[sid];
    if (type != -1) {
        return type;
    }
    fprintf(stderr, "Unknown sensor %u", sid);
    exit(1);
}

player_t get_sensor_player(sid_t sid) {
    if (sid >= 101) {
        fprintf(stderr, "Wrong sensor id %u in get_sensor_player", sid);
        exit(1);
    }
    player_t player = sensor_player_list[sid];
    if (player != -1) {
        return player;
    }
    fprintf(stderr, "Unknown player sensor %u", sid);
    exit(1);
}


bool ball_is_in_play(position p) {
    return p.x >= XMIN && p.x <= XMAX && p.y >= YMIN && p.y <= YMAX;
}


void readEvent(FILE *file, event *new) {
    fscanf(file, "%u,%lu,%d,%d,%d,%*s\n", &new->sid, &new->ts, &new->p.x,
           &new->p.y, &new->p.z);
}


int readInterruptionEvent(FILE **file, struct interruption_event *new, picoseconds start) {
    // file = file pointer con cursore aggiornato
    // new = struct dove mettere il risultato
    // start = l'offset da aggiungere al timestamp dell'evento (parte da 0 nel file)

    picoseconds minutes;
    double seconds;

    // read one line from the referee events file
    int read = fscanf(*file, "%*31c:%lu:%lf;%*s\n", &minutes, &seconds);

    if (!read) {
        // return -1 to signal that the file has ended
        return -1;
    } else {
        // we manage to read one event (the start of an interruption)
        new->start = start + (picoseconds) (seconds * SECTOPIC) + (minutes * 60) * SECTOPIC;
        DBG(("\nSTART INTERR %lu,%f", minutes, seconds));

        // get the corresponding end of the interruption
        fscanf(*file, "%*29c:%lu:%lf;%*s\n", &minutes, &seconds);
        new->end = start + (picoseconds) (seconds * SECTOPIC) + (minutes * 60) * SECTOPIC;
        DBG(("\nEND INTERR %lu,%f", minutes, seconds));
        return 0;
    }
}

double squareDistanceFromBall(position player_position, position ball_last_position) {
    // d = ((x2 - x1)2 + (y2 - y1)2 + (z2 - z1)2)1/2

    return ((player_position.x - ball_last_position.x) * (player_position.x - ball_last_position.x) +
            (player_position.y - ball_last_position.y) * (player_position.y - ball_last_position.y) +
            (player_position.z - ball_last_position.z) * (player_position.z - ball_last_position.z));
}


const char *player_names[] = {"None",
                              "Nick Gertje",
                              "Dennis Dotterweich",
                              "Niklas Waelzlein",
                              "Wili Sommer",
                              "Philipp Harlass",
                              "Roman Hartleb",
                              "Erik Engelhardt",
                              "Sandro Schneider",
                              "Leon Krapf",
                              "Kevin Baer",
                              "Luca Ziegler",
                              "Ben Mueller",
                              "Vale Reitstetter",
                              "Christopher Lee",
                              "Leon Heinze",
                              "Leo Langhans"};

// Used to print interval header
const picoseconds FIRST_HALF_DURATION = FIRST_END - GAME_START;
const picoseconds SECOND_HALF_DURATION = GAME_END - SECOND_START;


void print_interval(int interval, picoseconds T) {

    if (interval < (FIRST_HALF_DURATION / T)) {
        unsigned elapsed_time = (interval + 1) * (unsigned) (T / SECTOPIC);
        unsigned minutes = elapsed_time / 60;
        unsigned seconds = elapsed_time % 60;
        printf("\n== Current Interval %2d | First Half +%dm:%02ds  ==\n\n", interval, minutes, seconds);
    } else if (interval == (FIRST_HALF_DURATION / T)) {
        printf("\n== Current Interval %2d | First Half End  ==\n\n", interval);
    } else if (interval < (FIRST_HALF_DURATION / T + 1 + SECOND_HALF_DURATION / T)) {
        unsigned elapsed_time = (interval - (int) (FIRST_HALF_DURATION / T)) * (unsigned) (T / SECTOPIC);
        unsigned minutes = elapsed_time / 60;
        unsigned seconds = elapsed_time % 60;
        printf("\n== Current Interval %2d | Second Half +%dm:%02ds  ==\n\n", interval, minutes, seconds);
    } else {
        printf("\n== Current Interval %2d | Game End  ==\n\n", interval);
    }
}


void print_statistics(const unsigned int *interval_possession, const unsigned int *total_possession, int interval,
                      picoseconds T) {
    // output statistics

    //  print interval header
    print_interval(interval, T);

    // compute total possession for this interval to make percentages
    unsigned interval_total = 0;

#if IGNORE_GOALKEEPER
    for (int i = 2; i < 9; ++i) {
        interval_total += interval_possession[i];
    }
    for (int i = 10; i < 17; ++i) {
        interval_total += interval_possession[i];
    }
#else
    for (int i = 1; i < 17; ++i) {
        interval_total += interval_possession[i];
    }
#endif

    if (interval_total == 0) {
        printf("Nothing to show for this interval.\n\n");
    } else {
        // team A
        double team_a_interval_poss = 0;
        printf("Ball possession team A\n");
#if IGNORE_GOALKEEPER
        for (int i = 2; i < 9; ++i) {
#else
        for (int i = 1; i < 9; ++i) {
#endif
            // compute percentage for this player
            double player_possession = (double) interval_possession[i] / interval_total * 100;
            // update total team stats
            team_a_interval_poss = team_a_interval_poss + player_possession;
            printf("%2d) %-20s -> %5.2f%%\n", i, player_names[i], player_possession);
        }
        printf("\nTotal: %5.2f%%\n\n", team_a_interval_poss);
        // team B
        double team_b_interval_poss = 0;
        printf("Ball possession team B:\n");
#if IGNORE_GOALKEEPER
        for (int i = 10; i < 17; ++i) {
#else
        for (int i = 9; i < 17; ++i) {
#endif
            // compute percentage for this player
            double player_possession = (double) interval_possession[i] / interval_total * 100;
            // update total team stats
            team_b_interval_poss = team_b_interval_poss + player_possession;
            printf("%2d) %-20s -> %5.2f%%\n", i, player_names[i], player_possession);
        }
        printf("\nTotal: %5.2f%%\n\n", team_b_interval_poss);
    }

    // cumulative statistics
    printf("== Up to now ==\n\n");

    // compute total possession for the game to make percentages
    unsigned game_total = 0;
#if IGNORE_GOALKEEPER
    for (int i = 2; i < 9; ++i) {
        game_total = game_total + total_possession[i];
    }
    for (int i = 10; i < 17; ++i) {
        game_total = game_total + total_possession[i];
    }
#else
    for (int i = 1; i < 17; ++i) {
        game_total = game_total + total_possession[i];
    }
#endif

    // team A
    double team_a_total_poss = 0;
    printf("Ball possession team A\n");
#if IGNORE_GOALKEEPER
    for (int i = 2; i < 9; ++i) {
#else
    for (int i = 1; i < 9; ++i) {
#endif
        // compute percentage for this player
        double player_possession = (double) total_possession[i] / game_total * 100;
        // update total team stats
        team_a_total_poss = team_a_total_poss + player_possession;
        printf("%2d) %-20s -> %5.2f%%\n", i, player_names[i], player_possession);
    }
    printf("\nTotal: %5.2f%%\n\n", team_a_total_poss);
    // team B
    double team_b_total_poss = 0;
    printf("Ball possession team B:\n");
#if IGNORE_GOALKEEPER
    for (int i = 10; i < 17; ++i) {
#else
    for (int i = 9; i < 17; ++i) {
#endif
        // compute percentage for this player
        double player_possession = (double) total_possession[i] / game_total * 100;
        // update total team stats
        team_b_total_poss = team_b_total_poss + player_possession;
        printf("%2d) %-20s -> %5.2f%%\n", i, player_names[i], player_possession);
    }
    printf("\nTotal: %5.2f%%\n\n", team_b_total_poss);
}


void print_usage(const char * program_name) {
    printf("Usage: %s [-t <interval>] [-k <distance>] [-e <path to full-game>]\n", program_name);
    printf("          [-1 <path to interruptions (1st half)>] [-2 <path to interruptions (2nd half)>]\n");
    printf("Example: %s -e datasets/full-game -1 datasets/referee-events/Game\\ Interruption/1st\\ Half.csv", program_name);
    printf(" -2 datasets/referee-events/Game\\ Interruption/2nd\\ Half.csv -t 60 -k 5\n");
}


int main(int argc, char *argv[]) {
    // used to parse command line arguments
    int opt;
    // time in seconds between outputs
    unsigned long T;
    // distance in meters where a player can be considered to have possession of the ball
    unsigned long K;

    // paths to the events files
    char * fullgame_path;
    char * interr_path_one;
    char * interr_path_two;

    // counter to check which arguments have been passed
    int args_read = 0;

    // parse command line arguments
    while ((opt = getopt(argc, argv, "t:k:e:1:2:h")) != -1) {
        switch (opt) {
            case 't':
                // value for the interval T
                T = strtoul(optarg, NULL, 10);
                if (T < 1 || T > 60) {
                    printf("Invalid value for T: %lu\n", T);
                    printf("T must be an integer between 1 and 60!\n");
                    exit(1);
                }
                args_read |= 1;
                break;
            case 'k':
                // value for the distance K
                K = strtoul(optarg, NULL, 10);
                if (K < 1 || K > 5) {
                    printf("Invalid value for K: %lu\n", K);
                    printf("K must be an integer between 1 and 5!\n");
                    exit(1);
                }
                args_read |= 2;
                break;
            case 'e':
                // path to the events file ("full-game")
                fullgame_path = optarg;
                args_read |= 4;
                break;
            case '1':
                // path to the first interruption events file (".../Game Interruption/1st Half.csv")
                interr_path_one = optarg;
                args_read |= 8;
                break;
            case '2':
                // path to the second interruption events file (".../Game Interruption/2nd Half.csv")
                interr_path_two = optarg;
                args_read |= 16;
                break;
            case 'h':
            default:
                print_usage(argv[0]);
                exit(1);
        }
    }

    // check if all arguments have been passed
    if (args_read != (1 | 2 | 4 | 8 | 16)) {
        print_usage(argv[0]);
        exit(1);
    }


    picoseconds INTERVAL = T * SECTOPIC;   // convert to picoseconds
    K = K * 1000;  // convert to millimeters

    printf("Running with T = %ld, K = %ld.\n", T, K / 1000);

    // open dataset
    FILE *fp_game = fopen(fullgame_path, "r");

    if (fp_game == NULL) {
        printf("Error: couldn't open events file at %s.\n", fullgame_path);
        printf("Aborting.\n");
        exit(1);
    }

    FILE *fp_interruption = fopen(interr_path_one, "r");

    if (fp_interruption == NULL) {
        printf("Error: couldn't open interruption events file %s.\n", interr_path_one);
        printf("Aborting.\n");
        exit(1);
    }

    printf("Game starting..\n");

    // event cursor
    event current_event;

    interruption_event next_interruption;
    picoseconds minutes;
    double seconds;

    // discard headers
    fscanf(fp_interruption, "%*s\n");
    fscanf(fp_interruption, "%*s %*s %*s\n");
    fscanf(fp_interruption, "%*29c:%lu:%lf;%*s\n", &minutes, &seconds);

    next_interruption.start = GAME_START;
    DBG(("\nmin %lu, sec %lf", minutes, seconds));
    next_interruption.end =
            GAME_START + (picoseconds) (seconds * SECTOPIC) + (picoseconds) (minutes * 60) * SECTOPIC;

    DBG(("\nend interr %lu", next_interruption.end));

    // initialize variables used to check when the interval has ended
    picoseconds interval_ends;
    interval_ends = GAME_START + INTERVAL;

    // initialization of ball and players positions
    position center;
    center.x = 0;
    center.y = 0, center.z = 0;
    position players[17] = {center, center, center, center, center, center, center, center, center, center, center,
                            center, center, center, center, center, center};


    unsigned possession_counter = 0;

    int first_half_ended = 0;

    unsigned long K2 = K * K;

    position_event data;


    // variables to compute minimum distance
    double currdistance;
    double mindistance;
    player_t closestplayer;

    // initialize possession arrays, one cell per player
    // each cell counts how many times a player had possession of the ball
    // index 0 is a placeholder for no possession
    unsigned interval_possession[17] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    unsigned total_possession[17] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    // initialize interval counter
    unsigned interval = 0;


    while (!feof(fp_game)) {

        // fetch one update
        readEvent(fp_game, &current_event);

        if (current_event.ts < GAME_START) {
            // the game hasn't started yet
            // skip until the game starts
            DBG(("PARSER: skipping, game not started yet\n"));
            continue;
        }

        if (current_event.ts > FIRST_END && current_event.ts < SECOND_START) {
            // half-time break
            if (!first_half_ended) {
                // print final statistics for the first half
                first_half_ended = 1;

                // interval complete, output statistics
                print_statistics(interval_possession, total_possession, interval, INTERVAL);

                // reset interval to after the break
                for (int i = 0; i < 17; i++) {
                    interval_possession[i] = 0;
                }
                interval++;
                interval_ends = SECOND_START + INTERVAL;
                possession_counter = 0;
            }
            // skip until the second half starts
            DBG(("PARSER: skipping, half-time break\n"));
            continue;
        }

        if (current_event.ts > GAME_END) {
            // the game has ended
            DBG(("\nPARSER: END OF GAME"));


            // interval complete, output statistics
            print_statistics(interval_possession, total_possession, interval, INTERVAL);

            // exit
            return 0;
        }

        // we are in one of the two halves
        // if the game is interrupted, don't do anything
        // otherwise update status
        // print statistics every interval

        if (current_event.ts > interval_ends) {
            // an interval has ended, print statistics


            // interval complete, output statistics
            print_statistics(interval_possession, total_possession, interval, INTERVAL);

            // reset interval
            for (int i = 0; i < 17; i++) {
                interval_possession[i] = 0;
            }
            interval++;
            interval_ends += INTERVAL;
            possession_counter = 0;
        }

        if (current_event.ts >= next_interruption.start && current_event.ts <= next_interruption.end) {
            // the game is interrupted
            // skip until the game restarts
            DBG(("PARSER: skipping, interruption\n"));
            continue;
        }

        if (current_event.ts > next_interruption.end) {
            // fetch the next interruption from the stream
            DBG(("\nGame resume at %lu", next_interruption.end));
            // choose the correct offset to add to the event
            if (current_event.ts < FIRST_END) {
                int error = readInterruptionEvent(&fp_interruption, &next_interruption, GAME_START);
                if (error) {
                    // we're still in the first half of the game, and the first file has ended
                    // change the file we need to read from
                    fclose(fp_interruption);
                    fp_interruption = fopen(interr_path_two, "r");
                    if (fp_interruption == NULL) {
                        printf("Error: couldn't open interruption events file %s.\n", interr_path_two);
                        printf("Aborting.\n");
                        exit(1);
                    }
                    // handle the first interruption manually since it is formatted differently
                    // skip csv header
                    fscanf(fp_interruption, "%*s\n");
                    // skip interruption start
                    fscanf(fp_interruption, "%*s %*s %*s\n");
                    // get interruption end
                    fscanf(fp_interruption, "%*29c:%lu:%lf;%*s\n", &minutes, &seconds);
                    next_interruption.start = SECOND_START;
                    next_interruption.end = SECOND_START + (picoseconds) (seconds * SECTOPIC)
                                            + (picoseconds) (minutes * 60) * SECTOPIC;
                }
            } else if (current_event.ts > SECOND_START) {
                int error = readInterruptionEvent(&fp_interruption, &next_interruption, SECOND_START);
                if (error) {
                    // we're in the second half of the game, and the second file has ended
                    // i.e. the game has ended
                    next_interruption.start = GAME_END;
                    next_interruption.end = GAME_END;
                }
            }
        }

        // handle update
        switch (get_sensor_type(current_event.sid)) {

            case BALL:
                if (ball_is_in_play(current_event.p)) {
                    // update ball position and send everything to possession

                    // prepare message in buffer
                    data.ball = current_event.p;
                    for (int i = 1; i < 17; ++i) {
                        data.players[i] = players[i];
                        DBG(("\nPARSER: data player i %d, position x %d, y %d, z %d", i, send_data[numsent].players[i].x, send_data[numsent].players[i].y, send_data[numsent].players[i].z));
                    }
                    data.interval_id = interval;

                    // update count of possession computations spawned
                    possession_counter++;

                    // initialize variables for minimum computation
                    mindistance = INFINITY;
                    closestplayer = 0;

                    // find player with the smallest distance from the ball
                    for (unsigned i = 1; i < 17; i++) {
                        // we only need this to compare distances, therefore we can skip the square root
                        currdistance = squareDistanceFromBall(data.players[i], data.ball);
                        if (currdistance < mindistance) {
                            // update closest player
                            mindistance = currdistance;
                            closestplayer = i;
                        }
                    }

                    // check if the player is close enough
                    if (mindistance > K2) {
                        // too far, no one has possession of the ball
                        closestplayer = 0;
                        DBG(("POSSESSION: mindistance is too big! %lf > %d\n", mindistance, K * K));
                    }

                    interval_possession[closestplayer] += 1;
                    total_possession[closestplayer] += 1;

                }
                break;

            case PLAYER:
                // update the player's position
                players[get_sensor_player(current_event.sid)] = current_event.p;
                DBG(("PARSER: current player %d, sid %d, x %d, y %d, z %d",
                        get_sensor_player(current_event.sid),
                        current_event.sid,
                        players[get_sensor_player(current_event.sid)].x,
                        players[get_sensor_player(current_event.sid)].y,
                        players[get_sensor_player(current_event.sid)].z));
                break;

            default:
                // referee, ignore
                DBG(("\ndefault %d", get_sensor_type(current_event.sid)));
                break;
        }

    }

    // close event file
    fclose(fp_game);
    fclose(fp_interruption);

    printf("\n** Done! **");

    return 0;
}

