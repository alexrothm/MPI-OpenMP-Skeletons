#include <iostream>
#include <vector>

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
     * @param value Input for the operator.
     * @return Output of the operator.
     */
    virtual R operator()(T value1, T value2) const = 0;

    /**
     * \brief Destructor.
     */
    virtual ~ReduceFunctor() {}
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

template <typename T, typename Functor>
std::vector<T> map(std::vector<T> vector, Functor& f) {
    
    std::vector<T> result;

    #pragma parralel for
    for (auto iter : vector) {
        result.push_back(f(iter));
    }

    return result;
}

template <typename T, typename Functor>
T reduce(std::vector<T> vector, Functor& f) {
    T result = vector.front();
    vector.erase(vector.begin());

    for (auto iter : vector) {
        result = f(result, iter);
    }

    return result;
}

int main(int argc, char** argv) {
    std::vector<int> myVec{ 1,2,3,4,5,6,7,8,9 };
    Add myAddFunctor;
    Sum mySumFunctor;

    auto mapped = map(myVec, myAddFunctor);

    for (auto element : mapped) {
        std::cout << element << " ";
    }
    std::cout << std::endl;

    auto reduced = reduce(mapped, mySumFunctor);
    std::cout << reduced << std::endl;
}