#include <mpi.h>
#include <omp.h>

#include "VectorDistribution.hpp"

template<typename T>
VectorDistribution<T>::VectorDistribution(int size) : vectorSize(size) {
    init();
}

template<typename T>
VectorDistribution<T>::~VectorDistribution() {

}

template<typename T>
void VectorDistribution<T>::init() {
    // TODO create global num procs and rank
    numProcesses = -1;
    rank = -1;

    localSize = vectorSize / numProcesses;
    remainingSize = vectorSize % numProcesses;

    // set size of last process
    if (rank == numProcesses - 1) {
        localSize += remainingSize;
    }

    localVector.resize(localSize);
    gatheredVector.resize(vectorSize);
}

template<typename T>
void VectorDistribution<T>::scatterData(const std::vector<T> &data) {
    if (rank == 0) {
        // TODO test OMP
        // #pragma omp parallel for
        for (int i = 0; i < vectorSize; i++) {
            rootVector[i] = data[i];
        }
    }

    MPI_Scatter(rootVector.data(), localSize, MPI_BYTE,
                localVector.data(), localSize, MPI_BYTE,
                0, MPI_COMM_WORLD);
}

template<typename T>
template<typename R, typename MapFunctor>
VectorDistribution<R> VectorDistribution<T>::map(MapFunctor &f) {
    return VectorDistribution<R>(0);
}

template<typename T>
template<typename R, typename ReduceFunctor>
R VectorDistribution<T>::reduce(ReduceFunctor &f) {
    return nullptr;
}

template<typename T>
template<typename R, typename T2, typename ZipFunctor>
VectorDistribution<R> VectorDistribution<T>::zip(VectorDistribution<T2> &b, ZipFunctor &f) {
    return VectorDistribution<R>(0);
}
