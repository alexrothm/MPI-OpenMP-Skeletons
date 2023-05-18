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

    // set size of last process TODO check if no errors occur
    //if (rank == numProcesses - 1) {
        localSize += remainingSize;
    //}

    if (rank == 0) {
        rootVector.resize(vectorSize);
    }

    // TODO maybe add index

    localVector.resize(localSize);
    gatheredVector.resize(vectorSize);
}

template <typename T>
void VectorDistribution<T>::setLocal(int localIndex, const T& value) {
    localVector[localIndex] = value;
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
    int tmpSize = (rank == numProcesses - 1) ? (localSize - remainingSize) : localSize;
    for (int i = 0; i < numProcesses; i++) {
        if (rank == i) {
            std::cout << "Local Vector (Rank " << rank <<"): [ ";
            for (int j = 0; j < tmpSize; j++) {
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
