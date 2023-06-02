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

    // calculate localSize for each process
    localSize = vectorSize / numProcesses;
    remainingSize = vectorSize % numProcesses;

    // set first index befor adjusting last process vectorSize
    firstIndex = rank * localSize;
    // set add remainingSize of last process
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
    #pragma omp parallel for
    for (int i = 0; i < localSize; i++) {
        localVector[i] = data[i + firstIndex];
    };
}

template <typename T>
void VectorDistribution<T>::gatherVectors(std::vector<T>& results) {
    // if remainingSize != 0
    if (remainingSize)
        gatherUnequalVectors(results);
    else
        gatherEqualVectors(results);
}


template <typename T>
void VectorDistribution<T>::gatherEqualVectors(std::vector<T>& results) {
    size_t s = localSize * sizeof(T);
    // Store data from localVectors into results
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

    // Gather local sizes of each process
    MPI_Gather(&s, 1, MPI_INT,
               recvCounts, 1, MPI_INT,
               0,MPI_COMM_WORLD);

    int totalCount = 0;
    int* displacements = new int[numProcesses];

    if (rank == 0) {
        displacements[0] = 0;
        totalCount += recvCounts[0];

        for (int i = 1; i < numProcesses; i++) {
            totalCount += recvCounts[i];
            // Calculate offset to write to correct position in recvBuffer
            displacements[i] = displacements[i-1] + recvCounts[i-1];
        }
    }

    // Actually gather local data to the root process using recvCounts and displacements
    MPI_Gatherv(localVector.data(), localSize * elemSize, MPI_BYTE,
                results.data(), recvCounts, displacements, MPI_BYTE,
                0, MPI_COMM_WORLD);

    delete[] recvCounts;
    delete[] displacements;
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

    // using omp to share among threads
    #pragma omp parallel for
    for (int i = 0; i < localSize; i++) {
        result.setLocal(i, f(localVector[i]));
    }
    return result;
}

template <typename T>
template <typename ReduceFunctor>
T VectorDistribution<T>::reduce(ReduceFunctor &f) {
    T result = T();         // end result
    T localResult = T();    // localResult for each process

    // multiple threads enter parallel region
    #pragma omp parallel
    {
        // private result for each thread
        T privateLocalResult = T();
        auto privateSize = localVector.size();

        // Each thread calculates its portion
        #pragma omp for
        for (int i = 0; i < localSize; i++) {
            privateLocalResult = f(privateLocalResult, localVector[i]);
        }

        // Make sure local result is calculated correctly
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

    // Calculate the end result from partial results in allResults
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
