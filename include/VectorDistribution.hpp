#ifndef MPI_OPENMP_VECTORDISTRIBUTION_HPP
#define MPI_OPENMP_VECTORDISTRIBUTION_HPP
#include <vector>

template <typename T>
class VectorDistribution {
public:
    /**
     * \brief Creates a VectorDistribution of \em size elements.
     * @param size
     */
    VectorDistribution(int size);

    // TODO Copy constructor if needed
    /**
     * \brief Destructor.
     */
    ~VectorDistribution();

    void scatterData(const std::vector<T>& data);

    // TODO check if MPI_Gather needed

    /**
     * \brief
     *
     * @tparam R
     * @tparam MapFunctor
     * @param f
     * @return
     */
    template<typename R, typename MapFunctor>
    VectorDistribution<R> map(MapFunctor &f);

    template<typename R, typename ReduceFunctor>
    R reduce(ReduceFunctor &f);

    template<typename R, typename T2, typename ZipFunctor>
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

    std::vector<T> rootVector;
    std::vector<T> localVector;
    std::vector<T> gatheredVector;

    // TODO add init function

    void init();
};


#endif //MPI_OPENMP_VECTORDISTRIBUTION_HPP



