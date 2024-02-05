# wvcp_ahead

AHEAD - Adaptive HEAD algorithm for the weighted vertex coloring problem (WVCP)

This project is a C++ implementation of the AHEAD algorithm for the weighted vertex coloring problem (WVCP).
The AHEAD algorithm is a memetic algorithm that uses hyperheuristics to adapt the search to the instance.
This work is related to :

- the code : https://github.com/Cyril-Grelier/gcp_ahead (same algorithm for the graph coloring problem)
- the article : TBA

## Requirements

To compile this project you need :

- `cmake 3.14+ <https://cmake.org/>`\_\_
- gcc/g++ 11+
- `Python 3.9+ <https://www.python.org/>`\_\_ (for the slurm jobs, data analysis and documentation)
- `pytorch` :

```bash

    mkdir thirdparty
    cd thirdparty
    wget https://download.pytorch.org/libtorch/cpu/libtorch-shared-with-deps-1.13.0%2Bcpu.zip
    unzip libtorch-shared-with-deps-1.13.0+cpu.zip

    # Load the instances
    cd ..
    git submodule init
    git submodule update


    # build the project
    ./scripts/build_release.sh

    # run the project
    cd build
    ./gc_wvcp --help
```
