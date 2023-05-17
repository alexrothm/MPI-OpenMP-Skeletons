#include <iostream>
#include <vector>

#include <omp.h>



template <typename T, typename R> 
class Functor {
    public:
    /**
     * \brief Function call operator has to be implemented by the user.
     *
     * @param value Input for the operator.
     * @return Output of the operator.
     */
    virtual R operator()(T value) const = 0;

    /**
     * \brief Destructor.
     */
    virtual ~Functor() {}
};

template <typename T, typename R>
class ReduceFunctor {
    public:
    /**
     * \brief Function call operator has to be implemented by the user.
     *
     * @param value1 Input value 1.
     * @param value2 Input value 2.
     * @return Output of the Reduce function.
     */
    virtual R operator()(T value1, T value2) const = 0;

    /**
     * \brief Destructor.
     */
    virtual ~ReduceFunctor() {}
};

template <typename T, typename R>
class ZipFunctor {
    public:
    /**
     * \brief Function call operator has to be implemented by the user.
     *
     * @param value1 Input value 1.
     * @param value2 Input value 2.
     * @return Output of the Reduce function.
     */
    virtual R operator()(T l_value, T r_value) const = 0;

    /**
     * \brief Destructor.
     */
    virtual ~ZipFunctor() {}
};

struct Add : Functor<int, int> {
    int operator()(int value) const {
        return value + 1;
    }
};

struct Sum : ReduceFunctor<int, int> {
    int operator()(int v1, int v2) const {
        return v1 + v2;
    }
};

struct Mult : ZipFunctor<int, int> {
    int operator()(int val1, int val2) const {
        return val1 * val2;
    }
};

template <typename T, typename Functor>
std::vector<T> map(std::vector<T> vector, Functor& f) {
    auto n = vector.size();
    std::vector<T> result(n);

    #pragma omp parallel for
    for (size_t i = 0; i < n; i++) {
        result[i] = f(vector[i]);
    }

    return result;
}

template <typename T, typename Functor>
T reduce(std::vector<T> vector, Functor& f) {
    T result = vector.front();
    vector.erase(vector.begin());

    #pragma omp parallel for
    for (size_t i = 0; i < vector.size(); i++) {
        result = f(result, vector[i]);
    }

    return result;
}

template <typename T, typename Functor>
std::vector<T> zip(std::vector<T> vector1, std::vector<T> vector2, Functor& f) {
    size_t len = vector1.size();
    std::vector<T> result(len);

    #pragma omp parallel for
    for (size_t i = 0; i < len; i++) {
        result[i] = f(vector1.at(i), vector2.at(i));
    }

    return result;
}

template <typename T>
void printVec(std::vector<T> vector) {
    for (auto e : vector) {
        std::cout << e << " ";
    }
    std::cout << std::endl;
}

int main(int argc, char** argv) {
    omp_set_num_threads(2);
    std::vector<int> myVec{ 1,2,3,4,5,6,7,8,9 };
    Add myAddFunctor;
    Sum mySumFunctor;
    Mult myMultFunc;

    auto mapped = map(myVec, myAddFunctor);
    printVec(mapped);

    auto reduced = reduce(mapped, mySumFunctor);
    std::cout << reduced << std::endl;

    auto zipped = zip(myVec, myVec, myMultFunc);
    printVec(zipped);
}