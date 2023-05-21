#include <cstring>
#include "VectorDistribution.hpp"
#include "unistd.h"

template <typename T>
VectorDistribution<T>::VectorDistribution()
    : numProcesses(0), rank(0), vectorSize(0), localSize(0), remainingSize(0) {}

template <typename T>
VectorDistribution<T>::VectorDistribution(const VectorDistribution<T> &cs) : vectorSize(cs.vectorSize) {
    init();

    localVector = cs.localVector;
}

template <typename T>
VectorDistribution<T>::VectorDistribution(int size) : vectorSize(size) {
    init();
}

template <typename T>
VectorDistribution<T>::VectorDistribution(std::vector<T> vector) : vectorSize((int)vector.size()) {
    init();
    this->scatterData(vector);
}

template <typename T>
VectorDistribution<T>::~VectorDistribution() {
    localVector.clear();
}

template <typename T>
void VectorDistribution<T>::init() {
    numProcesses = Utils::num_procs;
    rank = Utils::proc_rank;

    localSize = vectorSize / numProcesses;
    remainingSize = vectorSize % numProcesses;

    // set first index befor adjusting last process vectorSize
    firstIndex = rank * localSize;
    // set size of last process TODO remove and assume that np divides vectorSize
    if (rank == numProcesses - 1) {
        localSize += remainingSize;
    }

    localVector.resize(localSize);
}

template<typename T>
T VectorDistribution<T>::getLocal(int localIndex) {
    return localVector[localIndex];
}

template <typename T>
void VectorDistribution<T>::setLocal(int localIndex, const T& value) {
    localVector[localIndex] = value;
}

template <typename T>
void VectorDistribution<T>::scatterData(const std::vector<T> &data) {
    // using this seems faster than MPI_Scatter
    #pragma omp parallel for
    for (int i = 0; i < localSize; i++) {
        localVector[i] = data[i + firstIndex];
    };

// Alternative with MPI_Scatter //
//    needed if using MPI_Scatter
//    if (rank == 0) {
//        // TODO test OMP
//        #pragma omp parallel for
//        for (int i = 0; i < vectorSize; i++) {
//            rootVector[i] = data[i];
//        }
//    }
//    size_t s = localSize * sizeof(T);
//    MPI_Scatter(rootVector.data(), s, MPI_BYTE,
//                localVector.data(), s, MPI_BYTE,
//                0, MPI_COMM_WORLD);
}

template <typename T>
void VectorDistribution<T>::gatherVectors(std::vector<T>& results) {
//    auto text =  (remainingSize) ? "Unequal" : "Equal";
    if (true)
        gatherUnequalVectors(results);
    else
        gatherEqualVectors(results);
}


template <typename T>
void VectorDistribution<T>::gatherEqualVectors(std::vector<T>& results) {
//    results.resize(vectorSize);

    size_t s = localSize * sizeof(T);
    MPI_Gather(localVector.data(), s, MPI_BYTE,
               results.data(), s, MPI_BYTE,
               0, MPI_COMM_WORLD);
}

template <typename T>
void VectorDistribution<T>::gatherUnequalVectors(std::vector<T>& results) {
    // Gather the sizes of local vectors from all processes
    std::vector<int> recvCounts(numProcesses);
    int elemSize = sizeof(T);
    size_t s = localSize * elemSize;

    MPI_Allgather(&s, elemSize, MPI_BYTE, recvCounts.data(), elemSize, MPI_BYTE, MPI_COMM_WORLD);

    int totalCount = 0;
    std::vector<int> displacements(numProcesses);
    std::vector<T> gatheredVector(vectorSize);

    if (rank == 0) {
        displacements[0] = 0;
        totalCount += recvCounts[0];

        for (int i = 1; i < numProcesses; i++) {
            totalCount += recvCounts[i];
            displacements[i] = displacements[i-1] + recvCounts[i-1];
        }

        std::cout << "RecvCounts: ";
        for (auto i : recvCounts) {
            std::cout << i << " ";
        }
        std::cout << std::endl;
        std::cout << "Total count: " << totalCount << std::endl;
        for (auto i : displacements) {
            std::cout << i << " ";
        }
        std::cout << std::endl;
        gatheredVector.resize(totalCount / elemSize);
    }
//    int i = 0;
//    while (i == 0) {
//        sleep(5);
//    }
    int totalBytes = totalCount * elemSize;

    MPI_Gatherv(localVector.data(), localSize * elemSize, MPI_BYTE,
                gatheredVector.data(), recvCounts.data(), displacements.data(), MPI_BYTE,
                0, MPI_COMM_WORLD);

    if (rank == 0) {
        results = gatheredVector;
    }
}

template <typename T>
void VectorDistribution<T>::printLocal() {
    for (int i = 0; i < numProcesses; i++) {
        if (rank == i) {
            std::cout << "Local Vector (Rank " << rank <<"): [ ";
            for (int j = 0; j < localSize; j++) {
                std::cout << localVector[j] << " ";
            }
            std::cout << "]" << std::endl;
        }
        MPI_Barrier(MPI_COMM_WORLD);
    }
}

template <typename T>
template <typename R, typename MapFunctor>
VectorDistribution<R> VectorDistribution<T>::map(MapFunctor &f) {
    VectorDistribution<R> result(vectorSize);

    #pragma omp parallel for
    for (int i = 0; i < localSize; i++) {
//        std::cout << "Rank " << rank << "Number: " << f(localVector[i]) << std::endl;
        result.setLocal(i, f(localVector[i]));
    }
    return result;
}

template <typename T>
template <typename ReduceFunctor>
T VectorDistribution<T>::reduce(ReduceFunctor &f) {
    T result;

    T localResult = T();
    #pragma omp parallel shared(localVector)
    {
        T privateLocalResult = T();
        auto privateSize = localVector.size();
        //std::cout << privateSize << " !!! " << std::endl;
        #pragma omp for
        for (int i = 0; i < localSize; i++) {
            privateLocalResult = f(privateLocalResult, localVector[i]);
        }
        //std::cout << privateLocalResult << std::endl;
        #pragma omp critical
        {
            localResult = f(localResult, privateLocalResult);
        }
    }

    // Gather local results to the root process
    T* allResults = new T[numProcesses];

    size_t s = sizeof(T);
    MPI_Gather(&localResult, s, MPI_BYTE,
               allResults, s,MPI_BYTE,
               0, MPI_COMM_WORLD);


    for (int i = 0; i < numProcesses; i++) {
        result = f(result, allResults[i]);
    }
    delete[] allResults;

    return result;
}

template <typename T>
template <typename R, typename T2, typename ZipFunctor>
VectorDistribution<R> VectorDistribution<T>::zip(VectorDistribution<T2> &b, ZipFunctor &f) {
    VectorDistribution<R> result(vectorSize);

    #pragma omp parallel for
    for (int i = 0; i < localSize; i++) {
        result.setLocal(i, f(localVector[i], b.getLocal(i)));
    }
    return result;
}
