To run:
1. cd FINERdex
2. mkdir -p build
3. cd build
4. cmake ..
5. make
6. ../findex_benchmark.

Notes:
1.CHT instead of branchless binary search works better with data set size if very large(100000000),because no point of using model for 100 keys

TODO:

1. Throughput of insert is better,read slightly lagging
2.check for threshold in insert in aidel_model_impl.h
3.Check remove_model_or_bin in aidel_model_impl.h
4.Range queries in aidel_model_impl.h
5.Range query in aidel_impl.h
