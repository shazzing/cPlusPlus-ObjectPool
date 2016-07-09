
/***********************************************************************
 * Software License Agreement (BSD License)
 *
 *
 * main.cpp
 * ObjPool
 *
 * Copyright 2015 Shashank Hegde (shashank.hegde@icloud.com).
 *   All rights reserved.
 *
 * THE BSD LICENSE
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *************************************************************************/


#include "ObjectPool.h"
#include <iostream>
#include <sstream>
#include <memory>
#include <future>
#include <thread>
#include <chrono>
using namespace std;


typedef chrono::microseconds Microseconds;
typedef chrono::high_resolution_clock Clock;
const Microseconds defaultM(0);


// demo struct of 96Bytes. POD1
struct PODObjOfSize96
{

    char ch[60];
    double dbl[4];
  

    PODObjOfSize96(char defaultChar, double default_db)
    {
        for (int i = 0; i<59; i++)
        {
            ch[i] = defaultChar;
        }
        dbl[0] = default_db;
        dbl[1] = default_db;
    }
    
};

// demo struct of size 104Bytes. POD2
struct PODObjOfSize104
{
    
    char ch[60];
    double dbl[5];
    int i[2];
    
    PODObjOfSize104(char defaultChar, double default_db, int default_int)
    {
        for (int i = 0; i<59; i++)
        {
            ch[i] = defaultChar;
        }
        dbl[0] = default_db;
        dbl[1] = default_db;
        i[0]=default_int;
        i[1]=default_int;
    }
    
};

// The TimeMeasurement is used as return object for every set of allocation, deallocation function call.
struct timeMeasurement
{
    Microseconds   maxTime;
    Microseconds minTime;
    Microseconds avgTime;
    size_t averagedCount;
};

// includes 2 array sptr96 and sptr104 with capacity of 2000 shared_ptr of POD1 and POD2
class test{
public:
    
    test(objectPool<110>* m_objPool ) : obj_pool(m_objPool){};
    
    // Allocate and deallocate on the same thread.
    void BasicTest()
    {
        
        cout << "Running BasicTest: Allocation and deAllocation on same thread and single function" << endl;
        timeMeasurement ret =  alloc96(0, 1000);
        printerFunc(ret, true);
        ret = deAlloc96(0, 1000);
        printerFunc(ret, false);
        
  }
    
  // Allocate on 1 thread and deallocate on another thread
    void test2()
    {
        cout << "Test 2: Allocate on one thread and de-allocate on another thread" << endl;
        {
            timeMeasurement ret;
            thread t([&] {ret = deAlloc96(0, 1000); });
            t.join();
            printerFunc(ret, true);
            
        }
        
        {
            timeMeasurement ret;
            thread t2([&]{ret = deAlloc96(0, 1000);});
            t2.join();
            printerFunc(ret, false);
        }
        
    }
    
protected:
  // helper functions
    
    //Printer function. prints the timeMeaurement
    void printerFunc(const timeMeasurement& timeM, bool isAllocation)
    {
        cout << "Latency for " << (isAllocation ? "allocation " : "de-allocation ") << " are as follows : "
                                  << " max: " << timeM.maxTime.count() << "us min: " << timeM.minTime.count() << "us avg: "
        << (timeM.avgTime.count()/timeM.averagedCount)  << "us" << endl;
    }
    
    // Allocates shared_ptr<PODobjOfSize96> from start to end on sptrArr96 shared_ptr array.
    timeMeasurement alloc96(size_t start, size_t end)
    {
        Clock::time_point allocStart = Clock::now();
        timeMeasurement retTm;
        retTm.minTime = defaultM;
        retTm.maxTime = defaultM;
        retTm.avgTime = defaultM;
        retTm.averagedCount = end - start;
        if(retTm.averagedCount == 0)
            return retTm;
        
        for (size_t i = start; i<end; i++)
        {
            Clock::time_point singleAllocStart = Clock::now();
            obj_pool->alloc<PODObjOfSize96, char, double>(sptrArr96[i], 'c', 22.3);
            Clock::time_point singleAllocEnd = Clock::now();
            Microseconds elapsedMicros = chrono::duration_cast<Microseconds>(singleAllocEnd - singleAllocStart);
            if((elapsedMicros < retTm.minTime ) || (retTm.minTime == defaultM)) // initially minTime is defaultM . Only after first initialization check elapsedTime is less minTime.
                retTm.minTime = elapsedMicros;
            if(elapsedMicros > retTm.maxTime)
            {
                retTm.maxTime = elapsedMicros;
            }
        }
        Clock::time_point allocEnd = Clock::now();
        retTm.avgTime = chrono::duration_cast<Microseconds>(allocEnd -allocStart);
        
        
        return retTm;
    }
    
    // Allocates shared_ptr<PODobjOfSize104> from start to end on sptrArr104 array
    timeMeasurement alloc104(size_t start, size_t end)
    {
        Clock::time_point allocStart = Clock::now();
        timeMeasurement retTm;
        retTm.minTime = defaultM;
        retTm.maxTime = defaultM;
        retTm.avgTime = defaultM;
        retTm.averagedCount = end - start;
        if(retTm.averagedCount == 0)
            return retTm;
        
        for (size_t i = start; i<end; i++)
        {
            Clock::time_point singleAllocStart = Clock::now();
            obj_pool->alloc<PODObjOfSize104, char, double, int>(sptrArr104[i], 'c', 22.3, 3);
            Clock::time_point singleAllocEnd = Clock::now();
            Microseconds elapsedMicros = chrono::duration_cast<Microseconds>(singleAllocEnd - singleAllocStart);
            if((elapsedMicros < retTm.minTime ) || (retTm.minTime == defaultM)) // initially minTime is defaultM . after first initialization check elapsedTime is less minTime.
                retTm.minTime = elapsedMicros;
            if(elapsedMicros > retTm.maxTime)
            {
                retTm.maxTime = elapsedMicros;
            }
        }
        Clock::time_point allocEnd = Clock::now();
        retTm.avgTime = chrono::duration_cast<Microseconds>(allocEnd -allocStart);

        
        return retTm;
    }
    
    timeMeasurement deAlloc96( size_t start, size_t end)
    {
        Clock::time_point allocStart = Clock::now();
        timeMeasurement retTm;
        retTm.minTime = defaultM;
        retTm.maxTime = defaultM;
        retTm.avgTime = defaultM;
        retTm.averagedCount = end - start;
        if(retTm.averagedCount == 0)
            return retTm;
        
        for (size_t i = start; i<end; i++)
        {
            Clock::time_point singleAllocStart = Clock::now();
            sptrArr96[i].reset();
            Clock::time_point singleAllocEnd = Clock::now();
            Microseconds elapsedMicros = chrono::duration_cast<Microseconds>(singleAllocEnd - singleAllocStart);
            if((elapsedMicros < retTm.minTime ) || (retTm.minTime == defaultM)) // initially minTime is defaultM . after first initialization check elapsedTime is less minTime.
                retTm.minTime = elapsedMicros;
            if(elapsedMicros > retTm.maxTime)
            {
                retTm.maxTime = elapsedMicros;
            }
        }
        Clock::time_point allocEnd = Clock::now();
        retTm.avgTime = chrono::duration_cast<Microseconds>(allocEnd -allocStart);
        
        return retTm;
        
    }
    
    timeMeasurement deAlloc104( size_t start, size_t end)
    {
        Clock::time_point allocStart = Clock::now();
        timeMeasurement retTm;
        retTm.minTime = defaultM;
        retTm.maxTime = defaultM;
        retTm.avgTime = defaultM;
        retTm.averagedCount = end - start;
        if(retTm.averagedCount == 0)
            return retTm;
        
        for (size_t i = start; i<end; i++)
        {
            Clock::time_point singleAllocStart = Clock::now();
            sptrArr104[i].reset();
            Clock::time_point singleAllocEnd = Clock::now();
            Microseconds elapsedMicros = chrono::duration_cast<Microseconds>(singleAllocEnd - singleAllocStart);
            if((elapsedMicros < retTm.minTime ) || (retTm.minTime == defaultM)) // initially minTime is defaultM . after first initialization check elapsedTime is less minTime.
                retTm.minTime = elapsedMicros;
            if(elapsedMicros > retTm.maxTime)
            {
                retTm.maxTime = elapsedMicros;
            }
        }
        Clock::time_point allocEnd = Clock::now();
        retTm.avgTime = chrono::duration_cast<Microseconds>(allocEnd -allocStart);
        return retTm;
        
    }
    
    
private:
    
    shared_ptr<PODObjOfSize96> sptrArr96[2000];
    shared_ptr<PODObjOfSize104> sptrArr104[2000];
    objectPool<110> *obj_pool;

};

void usage()
{
    std::cout << "Usage: ./ObjPool  <MaxCapacity> <MinCapacity> <NoOfThreads> " << endl;
}

int main(int argc, const char * argv[]) {
    if(argc != 4)
    {
        usage();
        return 1;
    }
    int maxCapacity, minCapacity, noOfThread;
    istringstream iStr(argv[1]);
    if(!(iStr >> maxCapacity))
    {
        cerr << "invalid MaxCapacity" << argv[1] << endl;
        return 1;
    }
    iStr.clear();
    iStr.str(argv[2]);
    if(!(iStr >> minCapacity))
    {
        cerr << "invalid minCapacity" << argv[2] << endl;
        return 1;
    }
    if(maxCapacity < minCapacity)
    {
        cerr << "maxCapacity cannot be less minCapacity" << endl;
        usage();
        return 1;
    }
    iStr.clear();
    iStr.str(argv[3]);
    if(!(iStr >> noOfThread))
    {
        cerr << "invalid noOfThread" << argv[3] << endl;
        return 1;
    }
    
    objectPool<110> *obj_pool = new objectPool<110>(maxCapacity, minCapacity);
    
   
    test *tst = new test(obj_pool);
    
    //TEST1: Allocate and deallocate in the same function
    tst->BasicTest();
    
    ///TEST2: Allocte happens in more than 1 thread and deallocation happens in another thread.
    tst->test2();
    tst->~test();
    
}
