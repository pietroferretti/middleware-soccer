#ifndef MIDDLEWARE_SOCCER_PARSER_H
#define MIDDLEWARE_SOCCER_PARSER_H

#include <mpi.h>

void
parser_run(MPI_Datatype mpi_position_for_possession_type, MPI_Datatype mpi_output_envelope, int possession_processes,
           picoseconds INTERVAL);

#endif //MIDDLEWARE_SOCCER_PARSER_H
