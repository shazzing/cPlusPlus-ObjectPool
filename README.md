# c-ObjectPool

This project has c++11 constructs and will compile directly only in c++11. This project was compiled in apple llvm c++ compiler wit -std=c++11 compile parameter. (the earlier commit had more restrictive code and would compile only on c++14)

The project provides support to create POD object of MAX_OBJ_SIZE. It runs a thread for creating objectPool of maintaining objectPool greater than a lower threshold, m_minCapacity,  by batching the object creation in size of m_maxCapacity everytime it needs to create more objects. The project also uses placement new and delete operation to support templatized object creation.  