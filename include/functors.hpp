#ifndef MPI_OPENMP_FUNCTORS_HPP
#define MPI_OPENMP_FUNCTORS_HPP

/**
 * \brief Class MapFunctor represents a functor for the map skeleton of a distributed vector.
 *
 * @tparam T Input data type.
 * @tparam R Output data type.
 */
template <typename T, typename R>
class MapFunctor {
public:
    /**
     * \brief Function call operator has to be implemented by the user.
     *
     * @param value Input for the map function.
     * @return Output of the map function.
     */
    virtual R operator()(T value) const = 0;

    /**
     * \brief Destructor.
     */
    virtual ~MapFunctor() {}
};

/**
 * \brief Class ReduceFunctor represents a functor for the reduce skeleton of a distributed vector.
 *
 * @tparam T Input data type.
 * @tparam R Output data type.
 */
template <typename T, typename R>
class ReduceFunctor {
public:
    /**
     * \brief Function call operator has to be implemented by the user.
     *
     * @param value1 Input value 1.
     * @param value2 Input value 2.
     * @return Output of the reduce function.
     */
    virtual R operator()(T value1, T value2) const = 0;

    /**
     * \brief Destructor.
     */
    virtual ~ReduceFunctor() {}
};

/**
 * \brief Class ReduceFunctor represents a functor for the zip skeleton of a distributed vector.
 *
 * @tparam T Input data type.
 * @tparam R Output data type.
 */
template <typename T, typename R>
class ZipFunctor {
public:
    /**
     * \brief Function call operator has to be implemented by the user.
     *
     * @param value1 Input value of the first distributed vector.
     * @param value2 Input value of the second distributed vector.
     * @return Output of the zip function.
     */
    virtual R operator()(T l_value, T r_value) const = 0;

    /**
     * \brief Destructor.
     */
    virtual ~ZipFunctor() {}
};

#endif //MPI_OPENMP_FUNCTORS_HPP
