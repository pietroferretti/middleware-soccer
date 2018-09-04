/**
 * @file main.c
 *
 * @brief This file contains the main function which starts the program.
 *
 * After setting the user-given interval (T in seconds) and possession distance
 * (K in meters), it initializes the MPI execution environment and the MPI datatypes.
 * Given the number of runnable process N, it starts the parser and output
 * processes and N-2 possession process.
 * After all children process have finished it terminates the MPI execution environment and returns.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stddef.h>

#include "common.h"
#include "parser.h"
#include "possession.h"
#include "output.h"


int main(int argc, char *argv[]) {

    int opt;
    unsigned long T = 10;
    unsigned long K = 3;

    char * fullgame_path = FULLGAME_PATH;
    char * interr_path_one = FIRST_INTERRUPTIONS;
    char * interr_path_two = SECOND_INTERRUPTIONS;

    while ((opt = getopt(argc, argv, "t:k:e:1:2:")) != -1) {
        switch (opt) {
            case 't':
                // value for the interval T
                T = strtoul(optarg, NULL, 10);
                if (T < 1 || T > 60) {
                    printf("Invalid value for T: %lu\n", T);
                    printf("T must be an integer between 1 and 60!\n");
                    exit(1);
                }
                break;
            case 'k':
                // value for the distance K
                K = strtoul(optarg, NULL, 10);
                if (K < 1 || K > 5) {
                    printf("Invalid value for K: %lu\n", K);
                    printf("K must be an integer between 1 and 5!\n");
                    exit(1);
                }
                break;
            case 'e':
                // path to the events file ("full-game")
                fullgame_path = optarg;
                break;
            case '1':
                // path to the first interruption events file (".../Game Interruption/1st Half.csv")
                interr_path_one = optarg;
                break;
            case '2':
                // path to the second interruption events file (".../Game Interruption/2nd Half.csv")
                interr_path_two = optarg;
                break;
            default:
                printf("Usage: %s [-t <interval>] [-k <distance>] [-e <path to full-game>]\n", argv[0]);
                printf("          [-1 <path to interruptions (1st half)>] [-2 <path to interruptions (2nd half)>]\n");
                exit(1);
        }
    }

    picoseconds INTERVAL = T * SECTOPIC;   // convert to picoseconds
    K = K * 1000;  // convert to millimeters

    // initialize mpi
    MPI_Init(NULL, NULL);

    // create MPI datatypes
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
    MPI_Datatype array_of_types[3] = {MPI_UINT32_T, MPI_UINT64_T, mpi_position_type};
    MPI_Datatype mpi_event_type;
    MPI_Type_create_struct(3, array_of_blocklengths, offsets2, array_of_types, &mpi_event_type);
    MPI_Type_commit(&mpi_event_type);

    // create struct for interruption_event
    int aob[2] = {1, 1};//    - number of elements in each block (array of integer)
    MPI_Aint o[2] = {offsetof(interruption_event, start), offsetof(interruption_event, end)};
    MPI_Datatype aot[2] = {MPI_UINT64_T, MPI_UINT64_T};
    MPI_Datatype mpi_interruption_event_type;
    MPI_Type_create_struct(2, aob, o, aot, &mpi_interruption_event_type);
    MPI_Type_commit(&mpi_interruption_event_type);

    // create struct for an array with every position
    int blocklengths3[3] = {17, 1, 1};
    MPI_Datatype types3[3] = {mpi_position_type, mpi_position_type, MPI_INT32_T};
    MPI_Datatype mpi_position_for_possession_type;
    MPI_Aint offsets3[3] = {offsetof(position_event, players), offsetof(position_event, ball),
                            offsetof(position_event, interval_id)};
    MPI_Type_create_struct(3, blocklengths3, offsets3, types3, &mpi_position_for_possession_type);
    MPI_Type_commit(&mpi_position_for_possession_type);

    // create envelope for messages sent to output
    // message type + num_processes or player who has possession
    MPI_Datatype mpi_output_envelope;
    MPI_Type_contiguous(2, MPI_UINT32_T, &mpi_output_envelope);
    MPI_Type_commit(&mpi_output_envelope);

    // check number of processes
    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    if (world_size < 3) {
        printf("This program needs at least 3 processes to run.\n");
        exit(1);
    }

    // check process rank
    int process_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &process_rank);

    if (process_rank == 0) {
        printf("Running with T = %ld, K = %ld.\n", T, K / 1000);
    }

    // dispatch correct function
    switch (process_rank) {
        case PARSER_RANK:
            parser_run(mpi_position_for_possession_type, mpi_output_envelope, world_size - POSSESSION_RANK, INTERVAL,
                       fullgame_path, interr_path_one, interr_path_two);
            break;
        case OUTPUT_RANK:
            output_run(mpi_output_envelope, INTERVAL);
            break;
        case POSSESSION_RANK:
        default:
            possession_run(mpi_position_for_possession_type, mpi_output_envelope, K);
            break;
    }

    // clear the MPI environment
    MPI_Finalize();

    printf("\n");
    printf("** Process %d done! **\n", process_rank);
    return 0;
}

