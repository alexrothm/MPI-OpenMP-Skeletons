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
    auto text =  (remainingSize) ? "Unequal" : "Equal";
    if (rank == 0)
        std::cout << text << std::endl;
    if (remainingSize)
        gatherUnequalVecors(results);
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
void VectorDistribution<T>::gatherUnequalVecors(std::vector<T>& results) {
    // Gather the sizes of local vectors from all processes
    int* recvCounts = new int[numProcesses];
    MPI_Gather(&localSize, 1, MPI_INT,
               recvCounts, 1, MPI_INT,
               0, MPI_COMM_WORLD);

    if (rank == 0) {
        // Calculate the displacements array
        int* displacements = new int[numProcesses];
        int displacement = 0;
        for (int i = 0; i < numProcesses; i++) {
            displacements[i] = displacement;
            displacement += recvCounts[i];
        }

        // Resize the gatheredData vector
//        int totalSize = displacement;
//        results.resize(totalSize);

        // Gather the data from all processes
        MPI_Gatherv(localVector.data(), localSize, MPI_INT,
                    results.data(), recvCounts, displacements, MPI_INT,
                    0, MPI_COMM_WORLD);

        delete[] displacements;
    } else {
        // Send the data to the root process
        MPI_Gatherv(localVector.data(), localSize, MPI_INT,
                    nullptr, nullptr, nullptr, MPI_INT,
                    0, MPI_COMM_WORLD);
    }

    delete[] recvCounts;
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
    return nullptr;
}

template <typename T>
template <typename R, typename T2, typename ZipFunctor>
VectorDistribution<R> VectorDistribution<T>::zip(VectorDistribution<T2> &b, ZipFunctor &f) {
    return VectorDistribution<R>(0);
}
