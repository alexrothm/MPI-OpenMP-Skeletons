#include "Utils.hpp"

int Utils::proc_rank = -1;
int Utils::num_procs;

void initSkeletons(int argc, char **argv) {
    // Initialize MPI environment
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &Utils::num_procs);
    MPI_Comm_rank(MPI_COMM_WORLD, &Utils::proc_rank);
}

void terminateSkeletons() {
    MPI_Finalize();
}
