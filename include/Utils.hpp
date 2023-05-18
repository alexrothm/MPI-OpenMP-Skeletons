#ifndef MPI_OPENMP_UTILS_HPP
#define MPI_OPENMP_UTILS_HPP

#pragma once

#include <mpi.h>
#include <omp.h>

class Utils {
public:
    static int proc_rank; // process rank
    static int num_procs; // total number of processes

    // TODO check if num_threads needed
    // static int num_threads;

};

void initSkeletons(int argc, char **argv);

void terminateSkeletons();

#endif //MPI_OPENMP_UTILS_HPP
