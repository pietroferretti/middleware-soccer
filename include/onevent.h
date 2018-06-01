#ifndef MIDDLEWARE_SOCCER_ONEVENT_H
#define MIDDLEWARE_SOCCER_ONEVENT_H

void
onevent_run(MPI_Datatype mpi_event_type, MPI_Datatype mpi_interruption_event_type, MPI_Datatype mpi_output_envelope,
            int possession_processes, picoseconds INTERVAL);

#endif //MIDDLEWARE_SOCCER_ONEVENT_H
