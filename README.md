## About
This project implements algorithmic skeletons by using OpenMP and MPI

## Compile & Run

CLI Arguments:
- iterations: number of overall iterations of the main loop
- size: size of the vector to use with the program
- threads: define how many threads OpenMP can use (for parallel program only)
- performOperations: how often each skeleton is called

To compile and run the **sequential** code:
```shell
mkdir build
cd build
cmake -D CMAKE_CXX_COMPILER=mpicxx ..
cmake --build . --target sequential-comp

./sequential-comp -n <iterations> -s <size> -p <performOperations>
```

To compile and run the **parallel** code:
```shell
mkdir build
cd build
cmake -D CMAKE_CXX_COMPILER=mpicxx ..
cmake --build . --target mpi-openmp

mpirun -n ${n} ./$mpi-openmp -n <iterations> -s <size> -t <threads> -p <performOperations>
```

