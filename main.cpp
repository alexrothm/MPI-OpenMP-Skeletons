#include <iostream>
#include "omp.h"

#include "functors.hpp"
#include "VectorDistribution.hpp"

struct Add : MapFunctor<int, int> {
    int operator()(int value) const {
        return value + 1;
    }
};


int main(int argc, char** argv) {
    initSkeletons(argc, argv);
    omp_set_num_threads(2);
    std::cout << omp_get_num_threads() << std::endl;
    std::cout << Utils::proc_rank << std::endl;
    std::cout << Utils::num_procs << std::endl;
    Add mapAddFunctor;

    std::vector<int> myVec{1, 2, 3, 4, 5, 6, 7, 8, 9};

    VectorDistribution<int> vecD(myVec);

    VectorDistribution<int> vecResult;

    vecResult = vecD.map<int>(mapAddFunctor);

    vecResult.printLocal();


    terminateSkeletons();
    return 0;
}