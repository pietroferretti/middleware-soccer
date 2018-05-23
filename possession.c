
// possession:
    // messaggio = posizione palla + tutte posizioni player + counter intervallo
    // for each player:
        // calcola distanza ball-player
    // find player with minimum distance
    // if min_distance < K**2:
        // send player_id of holder to output, tag = counter intervallo
    // else:
        // send 0 to output -> nessuno ha possesso, tag = counter intervallo
    // if "end of game":
        // send "end of game" to output
        // return

#include <stdio.h>
#include <mpi.h>
#include <stdint.h>
#include <common.h>

typedef struct {
    position players[17];
    position ball;
    unsigned interval;
} positions_envelope;


void possession_run(MPI_Datatype mpi_possession_envelope) {
    printf("TODO");

    // mi serve K
    // TODO leggere K da terminale

    // array per calcolo distanze
    // (oppure due sole variabili per calcolare il minimo direttamente)
    uint64_t distances[17];

    // variabile per minimo
    uint64_t mindistance;
    player_t closestplayer;

    // counter intervallo? lo passiamo direttamente da data

    // data: buffer per leggere messaggio
    positions_envelope data;

    // status: buffer per leggere info messaggio
    MPI_Status status;

    while (1) {
        // receive position update from onevent
        MPI_Recv(&data, 1, mpi_possession_envelope, ONEVENT_RANK, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

        switch (status.MPI_TAG) {
            case POSITIONS_MESSAGE:
                break;

            case ENDOFGAME_MESSAGE:

                return;

            default:
                printf("Message with wrong tag %u in the \"possession\" process!\n", status.MPI_TAG);
                printf("Aborting.\n");
                MPI_Abort(MPI_COMM_WORLD, 1);
        }
    }
}