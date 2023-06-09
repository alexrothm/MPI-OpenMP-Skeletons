#ifndef MPI_OPENMP_VECTORDISTRIBUTION_HPP
#define MPI_OPENMP_VECTORDISTRIBUTION_HPP
#pragma once

#include <vector>
#include <mpi.h>
#include <omp.h>
#include <sstream>

#include "Utils.hpp"


template <typename T>
class VectorDistribution {
public:
    VectorDistribution();

    /**
     * \brief Creates a VectorDistribution of \em size elements.
     * @param size
     */
    VectorDistribution(int size);

    VectorDistribution(const VectorDistribution<T>& cs);

    /**
     * \brief Creates a VectorDistribution and initializes it with the input \em vector.
     * @param vector Input vector.
     */
    VectorDistribution(std::vector<T>& vector);

    /**
     * \brief Destructor.
     */
    ~VectorDistribution();

    void setLocal(int localIndex, const T& value);

    T getLocal(int localIndex);

    void scatterData(const std::vector<T>& data);

    void gatherVectors(std::vector<T>& results);

    void printLocal();

    void show(const std::string& descr);

    template <typename R, typename MapFunctor>
    VectorDistribution<R> map(MapFunctor &f);

    template <typename ReduceFunctor>
    T reduce(ReduceFunctor &f);

    template <typename R, typename T2, typename ZipFunctor>
    VectorDistribution<R> zip(VectorDistribution<T2>& b, ZipFunctor& f);

private:
    // number of MPI processes
    int numProcesses;
    // position of processor
    int rank;
    // number of elements
    int vectorSize;
    // number of local elements
    int localSize;
    // remaining size for last process
    int remainingSize;
    // start of global index
    int firstIndex;

    std::vector<T> localVector;

    void init();

    void gatherEqualVectors(std::vector<T> &results);

    void gatherUnequalVectors(std::vector<T> &results);
};

#include "../src/VectorDistribution.cpp"

#endif //MPI_OPENMP_VECTORDISTRIBUTION_HPP