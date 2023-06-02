#include "functors.hpp"
#include <iostream>
#include <vector>
#include <unistd.h>

#include <mpi.h>

struct Add : MapFunctor<int, int> {
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

template <typename R, typename T, typename Functor>
std::vector<R> map(std::vector<T>& vector, Functor& f) {
    auto n = vector.size();
    std::vector<R> result(n);

    for (size_t i = 0; i < n; i++) {
        result[i] = f(vector[i]);
    }

    return result;
}

template <typename T, typename Functor>
T reduce(std::vector<T>& vector, Functor& f) {
    T result = vector.front();
    vector.erase(vector.begin());

    for (size_t i = 0; i < vector.size(); i++) {
        result = f(result, vector[i]);
    }

    return result;
}

template <typename R, typename T, typename T2, typename Functor>
std::vector<R> zip(std::vector<T>& vector1, std::vector<T2>& vector2, Functor& f) {
    size_t len = vector1.size();
    std::vector<R> result(len);

    for (size_t i = 0; i < len; i++) {
        result[i] = f(vector1[i], vector2[i]);
    }

    return result;
}

template <typename T>
void printVec(std::vector<T> vector) {
    std::cout << "[ ";
    for (auto e : vector) {
        std::cout << e << " ";
    }
    std::cout << "]" << std::endl;
}

int main(int argc, char** argv) {
    int iterations = 5;
    int size = 10;
    int perform = 1;
    int c;

    while ((c = getopt(argc, argv, "n:s:p:")) != -1) {
        switch (c) {
            case 'n':
                iterations = atoi(optarg);
                break;
            case 's':
                size = atoi(optarg);
                break;
            case 'p':
                perform = atoi(optarg);
                break;
            case '?':
                return 1;
            default:
                abort();
        }
    }
    // Start
    double startTime, mapTime, reduceTime, zipTime;
    mapTime = reduceTime = zipTime = 0;

    startTime =  MPI_Wtime();
    for (int run = 0; run < iterations; run++) {
        // Create data structures
        // Input data
        std::vector<int> input1(size);
        std::vector<int> input2(size);

        // Fill input vectors
        for (int i = 0; i < size; i++) {
            input1[i] = i + 1;
            input2[i] = (i + 1) * (i + 1);
        }

        // Output
        std::vector<int> outputMap(size);
        std::vector<int> outputZip(size);
        int outReduce;

        // Functions
        auto mapFunction = [] (int val) {return val + val;};
        auto zipFunction = [] (int val1, int val2) {return val1 * val2;};
        auto reduceFunction = [] (int val1, int val2) {return val1 + val2;};

        //
        // MAP FUNCTION
        //
        double t = MPI_Wtime();
        for (int p = 0; p < perform; ++p)
            outputMap = map<int>(input1,mapFunction);

        // Timing
        mapTime += MPI_Wtime() - t;

        //
        // ZIP FUNCTION
        //
        t = MPI_Wtime();
        for (int p = 0; p < perform; ++p)
            outputZip = zip<int>(input1, input2, zipFunction);

        // Timing
        zipTime += MPI_Wtime() - t;

        //
        // REDUCE FUNCTION
        //
        t = MPI_Wtime();
        for (int p = 0; p < perform; ++p)
            outReduce = reduce(outputZip, reduceFunction);

        // Timing
        reduceTime += MPI_Wtime() - t;

        if (run < 4) {
            mapTime = reduceTime = zipTime = 0; // Warm up
            startTime = MPI_Wtime();
        }
    }

    int divIter = iterations - 4;
    printf("Map;%i;%f\n", size, mapTime / divIter);
    printf("Zip;%i;%f\n", size, zipTime / divIter);
    printf("Red;%i;%f\n", size, reduceTime / divIter);
    double totalTime = MPI_Wtime() - startTime;
    printf("Time/runs;%i;%f\n", size, totalTime / divIter);

    return 0;
}