#ifndef MIDDLEWARE_SOCCER_POSSESSION_H
#define MIDDLEWARE_SOCCER_POSSESSION_H

#include <mpi.h>

void possession_run(MPI_Datatype mpi_possession_envelope, MPI_Datatype mpi_output_envelope, unsigned long K);

#endif //MIDDLEWARE_SOCCER_POSSESSION_H
