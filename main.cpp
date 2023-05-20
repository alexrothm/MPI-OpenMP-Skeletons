#include <iostream>
#include "omp.h"
#include <unistd.h>
#include "sstream"

#include "functors.hpp"
#include "VectorDistribution.hpp"
#include "Utils.hpp"

struct Add : MapFunctor<int, int> {
    int operator()(int value) const {
        return value + 20;
    }
};

template <typename T>
void printVec(std::vector<T> vector) {
    if (Utils::proc_rank == 0) {
        for (auto e : vector) {
            std::cout << e << " ";
        }
        std::cout << std::endl;
    }
}


int main(int argc, char** argv) {
    initSkeletons(argc, argv);
    omp_set_num_threads(10);
//    std::cout << omp_get_num_threads() << std::endl;
//    std::cout << Utils::proc_rank << std::endl;
//    std::cout << Utils::num_procs << std::endl;
    Add mapAddFunctor;

    std::vector<int> myVec{1, 2, 3, 4, 5, 6, 7, 8, 9};

    // Debug
//    int i = 0;
//    while(0==i)
//        sleep(5);
    std::ostringstream s;
    double end;
    double start = MPI_Wtime();
    VectorDistribution<int> vecD(myVec);
    end = MPI_Wtime();

//    s << end - start << "s" << std::endl;
//    if (Utils::proc_rank == 0)
//        printf("%s", s.str().c_str());
//    std::cout << "VecD: ";
    vecD.printLocal();


    VectorDistribution<int> vecResult;

    vecResult = vecD.map<int>(mapAddFunctor);


    vecResult.printLocal();
//
    std::vector<int> finalResult(9);
    vecResult.gatherVectors(finalResult);
//
//    printVec(finalResult);


    terminateSkeletons();
    return 0;
}