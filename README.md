# c-ObjectPool

This project has c++11 constructs and will compile directly only in c++11. This project was compiled in apple llvm c++ compiler wit -std=c++11 compile parameter. (the earlier commit had more restrictive code and would compile only on c++14)

The project provides support to create POD object of MAX_OBJ_SIZE. It runs a thread for creating objectPool of maintaining objectPool greater than a lower threshold, m_minCapacity,  by batching the object creation in size of m_maxCapacity everytime it needs to create more objects. The project also uses placement new and delete operation to support templatized object creation.

Usage : ./ObjPool  <MaxCapacity>  <MinCapacity>  <NoOfThreads>

Testing: 
Running BasicTest: Allocation and deAllocation on same thread and single function
Test 2: Allocate on one thread and de-allocate on another thread

BenchMarks:

BenchMarks on apple llvm compiler, running on 2.4ghZ macPro

Running BasicTest: Allocation and deAllocation on same thread and single function
Latency for allocation  are as follows :  max: 144us min: 3us avg: 5us
Latency for de-allocation  are as follows :  max: 38us min: 0us avg: 0us

Test 2: Allocate on one thread and de-allocate on another thread
Latency for allocation  are as follows :  max: 0us min: 0us avg: 0us
Latency for de-allocation  are as follows :  max: 7us min: 0us avg: 0us