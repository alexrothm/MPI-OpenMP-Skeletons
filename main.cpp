#include <iostream>
#include "omp.h"
#include <unistd.h>
#include "sstream"

#include "functors.hpp"
#include "VectorDistribution.hpp"
#include "Utils.hpp"


int main(int argc, char** argv) {
    initSkeletons(argc, argv);

    double start = MPI_Wtime();
//
    int iterations = 1;
    int size = 10;
    int threads = 2;
    int c;

    while ((c = getopt(argc, argv, "n:s:t:")) != -1) {
        switch (c) {
            case 'n':
                iterations = atoi(optarg);
                break;
            case 's':
                size = atoi(optarg);
                break;
            case 't':
                threads = atoi(optarg);
                break;
            case '?':
                return 1;
            default:
                abort();
        }
    }

    omp_set_num_threads(threads);

    printf("Iteration = %i, Size = %i, Threads = %i\n", iterations, size, threads);

    sleep(2);

    std::cout << "Time: " << MPI_Wtime() - start << "s" << std::endl;

    // Start timing
//    for (int i = 0; i < iterations; i++) {
//
//        // Create data structures
//
//
//        // Do map
//        // map
//        // time
//        // time avg MPI_Allreduce
//
//        //Zip
//        //timing
//
//        //Reduce
//        //timing
//
//
//    }



    terminateSkeletons();
    return 0;
}