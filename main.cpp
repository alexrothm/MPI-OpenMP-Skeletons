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
    std::cout << omp_get_num_threads() << std::endl;
    std::cout << Utils::proc_rank << std::endl;
    std::cout << Utils::num_procs << std::endl;
    Add mapAddFunctor;

    std::vector<int> myVec{1, 2, 3, 4, 5, 6, 7, 8, 9};

    VectorDistribution<int> vecD(myVec);

    vecD.printLocal();


    terminateSkeletons();
    return 0;
}