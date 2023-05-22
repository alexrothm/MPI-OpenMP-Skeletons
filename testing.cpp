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

struct ZipAdd : ZipFunctor<int, int> {
    int operator()(int val1, int val2) const {
        return val1 + val2;
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
//    std::cout << omp_get_max_threads() << std::endl;
    omp_set_num_threads(1);

//    std::cout << omp_get_num_threads() << std::endl;
//    std::cout << Utils::proc_rank << std::endl;
//    std::cout << Utils::num_procs << std::endl;

    std::vector<int> intVec{2, 4, 6, 8, 10, 12, 14, 16, 18};
    std::vector<double> doubleVec{2.5, 4.0, 6.0, 8.0, 10.5, 12.0, 14.0, 16.0, 18.0};

    VectorDistribution<int> intVecD(intVec);
    VectorDistribution<double> doubleVecD(doubleVec);

    intVecD.show("intVec");
    doubleVecD.show("doubleVec");

    auto intSum = [] (int val1, int val2) {return val1 + val2;};
    auto doubleSum = [] (double val1, double val2) {return val1 + val2;};

    auto intReduced = intVecD.reduce(intSum);
    auto doubleReduced = doubleVecD.reduce(doubleSum);


    std::cout << "intRed: " << intReduced << std::endl;
    std::cout << "doubleRed: " << doubleReduced << std::endl;


    terminateSkeletons();
    return 0;
}