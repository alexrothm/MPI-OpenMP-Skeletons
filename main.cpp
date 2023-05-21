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
    omp_set_num_threads(2);
//    std::cout << omp_get_num_threads() << std::endl;
//    std::cout << Utils::proc_rank << std::endl;
//    std::cout << Utils::num_procs << std::endl;
    Add mapAddFunctor;
    ZipAdd myAddZip;

    std::vector<int> myVec{1, 2, 3, 4, 5, 6, 7, 8, 9};
    std::vector<double> myDoubleVec{1.25, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0};

    VectorDistribution<double> zipVector(myDoubleVec);

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


    auto myAddLamda = [](int v1) {return v1 + 20;};
    vecResult = vecD.map<int>(myAddLamda);
    vecResult.printLocal();

    std::vector<int> test(9);
    vecResult.gatherVectors(test);
    printVec(test);

    //zipResult = vecD.zip<int>(zipVector, myAddZip);

//    auto myTest = [](double v1, double v2) {return v1 + v2; };
//    auto zipResult = zipVector.zip<double>(zipVector, myTest);
//
//    zipResult.printLocal();
//
//    std::vector<double> finalResult(9);
//    zipResult.gatherVectors(finalResult);
//    printVec(finalResult);



//    vecResult.printLocal();

//    zipResult.gatherVectors(finalResult);

//    auto reduceFunc = [] (double val1, double val2) {return val1 + val2;};
//    auto reduced = zipResult.reduce(reduceFunc);
//    std::cout << "Test: " << reduced << std::endl;
//    printVec(finalResult);


    terminateSkeletons();
    return 0;
}