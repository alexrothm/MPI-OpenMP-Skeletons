#include "VectorDistribution.hpp"

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

    // set size of last process
    if (rank == numProcesses - 1) {
        localSize += remainingSize;
    }

    if (rank == 0) {
        rootVector.resize(vectorSize);
    }

    // TODO maybe add index

    localVector.resize(localSize);
    gatheredVector.resize(vectorSize);
}

template <typename T>
void VectorDistribution<T>::gatherVectors(std::vector<T>& results) {
    MPI_Gather(localVector.data(), localSize, MPI_BYTE,
               gatheredVector.data(), localSize, MPI_BYTE,
               0, MPI_COMM_WORLD);

    if (rank == 0) {
        results = gatheredVector;
    }
}

template <typename T>
void VectorDistribution<T>::printLocal() {
    for (int i = 0; i < numProcesses; i++) {
        if (Utils::proc_rank == i) {
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
void VectorDistribution<T>::scatterData(const std::vector<T> &data) {
    if (rank == 0) {
        // TODO test OMP
        #pragma omp parallel for
        for (int i = 0; i < vectorSize; i++) {
            rootVector[i] = data[i];
        }
    }
    size_t s = localSize * sizeof(T);
    MPI_Scatter(rootVector.data(), s, MPI_BYTE,
                localVector.data(), s, MPI_BYTE,
                0, MPI_COMM_WORLD);
}

template <typename T>
template <typename R, typename MapFunctor>
VectorDistribution<R> VectorDistribution<T>::map(MapFunctor &f) {
    VectorDistribution<R> result(localSize);

    #pragma omp parallel for
    for (int i = 0; i < localSize; i++) {
        result[i] = f(localVector[i]);
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
