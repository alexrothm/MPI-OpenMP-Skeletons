#include <iostream>
#include "omp.h"
#include <unistd.h>
#include "sstream"

#include "functors.hpp"
#include "VectorDistribution.hpp"
#include "Utils.hpp"


int main(int argc, char** argv) {
    initSkeletons(argc, argv);
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

//    printf("Iteration = %i, Size = %i\n", iterations, size);

    // Start
    double startTime, mapTime, reduceTime, zipTime;
    mapTime = reduceTime = zipTime = 0;

    startTime =  MPI_Wtime();
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
        inputVD1.show("VD1");
        inputVD2.show("VD2");

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
        outputMap = inputVD1.map<int>(mapFunction);
        outputMap.show("outMap");

        // Timing
        mapTime += MPI_Wtime() - t;

        //
        // ZIP FUNCTION
        //
        t = MPI_Wtime();
        outputZip = inputVD1.zip<int>(inputVD2, zipFunction);
        outputZip.show("OutZip");

        // Timing
        zipTime += MPI_Wtime() - t;

        //
        // REDUCE FUNCTION
        //
        t = MPI_Wtime();
        outReduce = outputZip.reduce(reduceFunction);
        std::cout << "ourReduce: " << outReduce << std::endl;

        // Timing
        reduceTime += MPI_Wtime() - t;

//        double result = MPI_Wtime() - splitTime;
//        splitTime = MPI_Wtime();
//        std::cout << "Run " << run << ": " << result << "s" << std::endl;

//        if (run == iterations - 1) {
//            printVec(input1);
//            printVec(input2);
//            printVec(outputZip);
//            std::cout << outReduce << std::endl;
//        }
    }
    // Timing
//    std::cout << "Map time: " << mapTime / iterations << "s" << std::endl
//              << "Zip time: " << zipTime / iterations << "s" << std::endl
//              << "Reduce time: " << reduceTime / iterations << "s" << std::endl;

    printf("Map;%i;%f;%i\n", size, mapTime / iterations, threads);
    printf("Zip;%i;%f;%i\n", size, zipTime / iterations, threads);
    printf("Red;%i;%f;%i\n", size, reduceTime / iterations, threads);
    double totalTime = MPI_Wtime() - startTime;
    printf("Time/runs;%i;%f;%i\n", size, totalTime / iterations, threads);

//    std::cout << "Map;" << mapTime / iterations << "s" << std::endl
//              << "Zip time: " << zipTime / iterations << "s" << std::endl
//              << "Reduce time: " << reduceTime / iterations << "s" << std::endl;

//    std::cout << "\nTime/runs: " << totalTime/iterations << std::endl;
    terminateSkeletons();
    return 0;
}