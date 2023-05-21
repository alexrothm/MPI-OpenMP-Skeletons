#include "VectorDistribution.hpp"

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
VectorDistribution<T>::VectorDistribution(std::vector<T>& vector) : vectorSize((int)vector.size()) {
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
    // set size of last process
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
void VectorDistribution<T>::scatterData(const std::vector<T>& data) {
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
    if (remainingSize)
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
    int* recvCounts = new int[numProcesses];
    int elemSize = sizeof(T);
    const size_t s = localSize * elemSize;

    //Changed to gather
    MPI_Gather(&s, 1, MPI_INT, recvCounts, 1, MPI_INT, 0,MPI_COMM_WORLD);

//    std::cout << "RecvCounts: ";
//    for (int i = 0; i < numProcesses; i++) {
//        std::cout << recvCounts[i] << " ";
//    }

    int totalCount = 0;
    int* displacements = new int[numProcesses];

    if (rank == 0) {
        displacements[0] = 0;
        totalCount += recvCounts[0];

        for (int i = 1; i < numProcesses; i++) {
            totalCount += recvCounts[i];
            displacements[i] = displacements[i-1] + recvCounts[i-1];
        }

//        std::cout << "RecvCounts: ";
//        for (int i = 0; i < numProcesses; i++) {
//            std::cout << recvCounts[i] << " ";
//        }
//        std::cout << std::endl;
//        std::cout << "Total count: " << totalCount << std::endl;
//        for (int i = 0; i < numProcesses; i++) {
//            std::cout << displacements[i] << " ";
//        }
//        std::cout << std::endl;

//        gatheredVector.resize(totalCount);
    }

    MPI_Gatherv(localVector.data(), localSize * elemSize, MPI_BYTE,
                results.data(), recvCounts, displacements, MPI_BYTE,
                0, MPI_COMM_WORLD);

    delete[] recvCounts;
    delete[] displacements;

//    if (rank == 0) {
//        results = gatheredVector;
//    }
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

template<typename T>
void VectorDistribution<T>::show(const std::string &descr) {
    std::vector<T> out(vectorSize);
    std::ostringstream s;

    if (descr.size() > 0)
        s << descr << std::endl;

    gatherVectors(out);

    if (rank == 0) {
        s << "[ ";
        for (int i = 0; i < vectorSize; i++) {
            s << out[i];
            s << " ";
        }
        s << "]" << std::endl;
        s << std::endl;

        std::cout << s.str().c_str();
    }

    out.clear();
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
