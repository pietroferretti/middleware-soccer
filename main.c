
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define GAME_START 10753295594424116
#define GAME_END 14879639146403495
#define DATASET_PATH "/home/nicole/CLionProjects/middleware-soccer/full-game"

typedef struct event {
    unsigned sid;
    uint64_t ts;
    int x;
    int y;
    int z;
} event;

void print(event e) {
    printf("\n%u %lu %d %d %d", e.sid, e.ts, e.x, e.y, e.z);
}

void readEvent(FILE *file, event *new) {
    fscanf(file, "%u%*c %lu%*c %d%*c %d%*c %d%*c %*s %*s %*s %*s %*s %*s %*s %*s", &(new->sid), &new->ts, &new->x,
           &new->y, &new->z);
}


int main() {

    uint64_t interval_possession_team1[8] = {0, 0, 0, 0, 0, 0, 0, 0}, total_possession_team1[8] = {0, 0, 0, 0, 0, 0, 0,
                                                                                                   0};
    uint64_t interval_possession_team2[8] = {0, 0, 0, 0, 0, 0, 0, 0}, total_possession_team2[8] = {0, 0, 0, 0, 0, 0, 0,
                                                                                                   0};
    FILE *fp = fopen(DATASET_PATH, "r");

    if (fp == NULL) {
        printf("Error: file pointer is null.");
        exit(1);
    }

    event test;

    while (!feof(fp)) {
        readEvent(fp, &test);
        print(test);
    }


    fclose(fp);

    return 0;
}