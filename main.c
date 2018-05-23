
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
//#include <mpi/mpi.h>  // ? strano, aggiungi a include directories
#include <mpi.h>
#include <stddef.h>

#include "common.h"
#include "parser.h"
#include "onevent.h"
#include "possession.h"
#include "output.h"

//#define PRGDEBUG

#ifdef PRGDEBUG
#define DBG(x) printf x
#else
#define DBG(x) /*nothing*/
#endif


#ifdef PRGDEBUG
#define FULLGAME_PATH "stripped-game" //selezionare la working directory dalle config di build per farlo funzionare
#else
#define FULLGAME_PATH "full-game" //selezionare la working directory dalle config di build per farlo funzionare
#endif

// TODO spostare tutte le macro in un header unico usato da tutti


int main() {

    // initialize mpi
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

    // create struct for interruption_event
    int aob[2] = {1, 1};//    - number of elements in each block (array of integer)
    MPI_Aint o[2] = {offsetof(interruption_event, start), offsetof(interruption_event, end)};
    MPI_Datatype aot[2] = {MPI_UNSIGNED_LONG, MPI_UNSIGNED_LONG};
    MPI_Datatype mpi_interruption_event_type;
    MPI_Type_create_struct(2, aob, o, aot, &mpi_interruption_event_type);
    MPI_Type_commit(&mpi_interruption_event_type);


    // create envelope for messages sent to output
    // message type + unsigned value
    MPI_Datatype mpi_output_envelope;
    MPI_Type_contiguous(2, MPI_UINT32_T, &mpi_output_envelope);
    MPI_Type_commit(&mpi_output_envelope);




    // TODO gli altri tipi che possono essere inviati


    // check number of processes
    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    if (world_size < 4) {
        printf("This program needs at least 4 processes to run.\n");
        exit(1);
    }

    // check process rank
    int process_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &process_rank);

    // dispatch correct function
    switch (process_rank) {
        case PARSER_RANK:
            parser_run(mpi_event_type, mpi_interruption_event_type);
            break;
        case ONEVENT_RANK:
            onevent_run();
            break;
        case POSSESSION_RANK:
            possession_run();
            break;
        case OUTPUT_RANK:
            output_run(mpi_output_envelope);
            break;
        default:
            // nothing to do
            break;
    }


    // TODO big barrier here?

    // clear the MPI environment
    MPI_Finalize();

    printf("\n");
    printf("** Done! **\n");
    return 0;
}



/// cosa dobbiamo fare
// parte
// legge
// fai partire i thread
// scorri tutti gli eventi fino alla fine
// ogni tot stampa output
// fine

// mpi_init
// definisci i datatype
// check if nprocesses == 4
    // MPI_COMM_SIZE(MPI_COMM_WORLD, count)
// create 4 processes
// check on MPI_COMM_RANK(MPI_COMM_WORLD, myid)
// process 0: parser
    // parser_p()
// process 1: onevent
    // onevent_p()
// process 2: possession
    // possession_p()
// process 3: output
    // output_p()

// parser:
    // apri file eventi e interruzioni
    // read evento o interruzione
    // send evento o interruzione/resume
    // if game has ended:
        // send "end of game" to onevent
        // return
// onevent:
    // event = player/ball, posizione
    // if player:
        // update player position
    // if ball:
        // TODO per ora ogni volta, se è troppo lento, ogni n volte
        // find possession
        // send messaggio a possession_p
            // messaggio = posizione palla + tutte posizioni player + counter intervallo
        // numero calcoli possesso += 1
    // if finito intervallo (timestamp > interval_end):
        // send messaggio a output
            // messaggio = "print", numero calcoli possesso (quelli che dovrà aspettare)
        // interval_end += T
        // counter intervallo += 1
        // numero calcoli possesso = 0
    // if "end of game":
        // send "end of game" to possession
        // return
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
// output:
    // if messaggio = possesso, tag=num intervallo
        // arrayintervallo[player_id] += 1
        // arraycumulativo[player_id] += 1
        // nread += 1
    // if messaggio = print, num calcoli
        // while nread < num calcoli:
            // receive from possession
        // letti tutti
        // print statistics
        // annulla array intervallo
        // nread = 0
    // if "end of game"
        // return

// mpi_finalize


// altro:
    // usare più di un buffer per le send non-blocking, in modo da parallelizzare meglio (ad es. 2 buffer swappati ogni volta)
    // (per le recv di output, fare due receive non bloccanti e poi waitany su entrambe) nah
    // quando mpi_finalize viene chiamata non dovrebbero più esserci send o receive unmatchati
    // per ricevere due tipi di messaggio, prima ricevere il tipo (con tag tipo), poi fare una recv con il tipo e tag giusto
        // ^ il tag verrebbe usato sia per tipi che per il counter intervallo, credo si possano fare magheggi con gli shift