#include <iostream>
#include "omp.h"
#include <unistd.h>
#include "sstream"

#include "functors.hpp"
#include "VectorDistribution.hpp"
#include "Utils.hpp"

double getAvg(double t)
{
    double sum;
    MPI_Allreduce(&t, &sum, 1, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
    return sum/Utils::num_procs;
}

int main(int argc, char** argv) {
    initSkeletons(argc, argv);
//
    int iterations = 5;
    int size = 10;
    int threads = 1;
    int perform = 1;
    int c;

    while ((c = getopt(argc, argv, "n:s:t:p:")) != -1) {
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
            case 'p':
                perform = atoi(optarg);
                break;
            case '?':
                return 1;
            default:
                abort();
        }
    }

    omp_set_num_threads(threads);

//    printf("Iteration = %i, Size = %i, Threads = %i\n", iterations, size, threads);

    // Start
    double startTime, mapTime, reduceTime, zipTime;
    mapTime = reduceTime = zipTime = 0;

    for (int run = 0; run < iterations; run++) {
        // Create data structures
        // Input data
        std::vector<int> input1(size);
        std::vector<int> input2(size);

        // Fill input vectors
        for (int i = 0; i < size; i++) {
            input1[i] = i + 1;
            input2[i] = (i + 1) * (i + 1);
        }

        VectorDistribution<int> inputVD1(input1);
        VectorDistribution<int> inputVD2(input2);

        // Output
        VectorDistribution<int> outputMap;
        VectorDistribution<int> outputZip;
        int outReduce;

        // Functions
        auto mapFunction = [] (int val) {return val + val;};
        auto zipFunction = [] (int val1, int val2) {return val1 * val2;};
        auto reduceFunction = [] (int val1, int val2) {return val1 + val2;};


        //
        // MAP FUNCTION
        //
        double t = MPI_Wtime();
        for (int p = 0; p < perform; ++p)
            outputMap = inputVD1.map<int>(mapFunction);

        // Timing
        mapTime += MPI_Wtime() - t;
        mapTime = getAvg(mapTime);

        //
        // ZIP FUNCTION
        //
        t = MPI_Wtime();
        for (int p = 0; p < perform; ++p)
            outputZip = inputVD1.zip<int>(inputVD2, zipFunction);

        // Timing
        zipTime += MPI_Wtime() - t;
        zipTime = getAvg(zipTime);

        //
        // REDUCE FUNCTION
        //
        t = MPI_Wtime();
        for (int p = 0; p < perform; ++p)
            outReduce = inputVD1.reduce(reduceFunction);

        // Timing
        reduceTime += MPI_Wtime() - t;
        reduceTime = getAvg(reduceTime);

        if (run < 4) {
            mapTime = reduceTime = zipTime = 0; // Warm up
            startTime = MPI_Wtime();
        }


//        double result = MPI_Wtime() - splitTime;
//        splitTime = MPI_Wtime();
//        std::cout << "Run " << run << ": " << result << "s" << std::endl;
    }

    if (Utils::proc_rank == 0) {
        int divIter = iterations - 4;
        printf("Map;%i;%f;%i\n", size, mapTime / divIter, threads);
        printf("Zip;%i;%f;%i\n", size, zipTime / divIter, threads);
        printf("Red;%i;%f;%i\n", size, reduceTime / divIter, threads);
        double totalTime = MPI_Wtime() - startTime;
        printf("Time/runs;%i;%f;%i\n", size, totalTime / divIter, threads);
    }

    terminateSkeletons();
    return 0;
}