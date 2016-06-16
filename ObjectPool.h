//
//  ObjectPool.h
//  ObjPool
//
//  Created by shashank hegde on 10/18/15.
//  Copyright (c) 2015 shashank hegde. All rights reserved.
//


/***********************************************************************
 * Software License Agreement (BSD License)
 *
 * Copyright 2008-2009  Marius Muja (mariusm@cs.ubc.ca). All rights reserved.
 * Copyright 2008-2009  David G. Lowe (lowe@cs.ubc.ca). All rights reserved.
 * Copyright 2011-2016  Jose Luis Blanco (joseluisblancoc@gmail.com).
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

#ifndef ObjPool_ObjectPool_h
#define ObjPool_ObjectPool_h
#include <iostream>

#include <vector>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>

using namespace std;


template<size_t MAX_OBJ_SIZE>
class objectPool
{
public:
    objectPool(std::size_t maxCapacity, std::size_t minCapacity) :m_maxCapacity(maxCapacity), m_minCapacity(minCapacity) {
        t = thread(&objectPool::replenishCapacity, this); // Initialize a seperate thread with replenishCapacity
        is_replinshment_complete = false;
    }
    
    
    // default deallocator.  standard default_delete would work . Kept debugging.
    void voidDelete(void* v_ptr)
    {
        ::operator delete(v_ptr);
    }
    
    
    // clean up the objectPool replenishing thread by setting is_replenshment_complete to true and calling join on the thread.
    ~objectPool()
    {
        is_replinshment_complete = true;
        if (t.joinable())
            t.join();
        m_objPool.clear();
        delete m_objPool;
    }
    
    // used to replenish ObjectPool. Runs on seperate thread. Notified whenever an object is allocated from pool using alloc function.
    void replenishCapacity()
    {
        while(!is_replinshment_complete)
        {
            std::unique_lock<std::mutex> lk(mutex_ObjPool);
            cv.wait(lk);
           
            if (m_objPool.size() <= m_minCapacity) // No of un-allocated objects
            {
                for (size_t i = 0; i<m_maxCapacity; i++)
                {
                    shared_ptr<void> s_ptr(::operator new(MAX_OBJ_SIZE), std::bind(mem_fn(&objectPool::voidDelete), this, std::placeholders::_1));
                    m_objPool.push_back(s_ptr);
                }
            }
            lk.unlock();
            cv.notify_one();
        }
    }
    
    // Main alloc calls. Uses obectPool if its not empty. Calls regular new operator if not.
    // Ideally sizeOf(OBJECT) < MAX_OBJ_SIZE necessary, but left as description clearly stated this can be assumed.
    template< typename OBJECT, typename... ARGS >
    void alloc(std::shared_ptr<OBJECT> & object, ARGS ...args)
    {
        std::unique_lock<std::mutex> locker(mutex_ObjPool);
        if (!m_objPool.empty())
        {
            OBJECT* tmp = new (m_objPool.back().get()) OBJECT(args...);
            object = shared_ptr<OBJECT>(tmp,
                                        [&](OBJECT* object) // Will also work on c++11(earlier code worked only on c++14)
                                        {
                                            std::unique_lock<std::mutex> lk1(mutex_ObjPool);  // deAlloc functor is not made inline. So make thread safe.
                                            shared_ptr<void> v_ptr = static_cast<shared_ptr<void>>(object);
                                            m_objPool.push_back(v_ptr);
                                            lk1.unlock();
                                        });
            m_objPool.pop_back();
        }
        else
        {
            object = shared_ptr<OBJECT>(new OBJECT(args...),
                                        [=](OBJECT* object)
                                        {
                                            std::unique_lock<std::mutex> lk1(mutex_ObjPool);
                                            shared_ptr<void> v_ptr = static_cast<shared_ptr<void>>(object);
                                            m_objPool.push_back(v_ptr);
                                            lk1.unlock();
                                        });
        }

        locker.unlock();
        cv.notify_one();
    }
    
    
    mutex getPoolMutex()
    {
        return mutex_ObjPool;
    }
    
    vector<shared_ptr<void>>& getObjPoolVec()
    {
        return m_objPool;
    }
    
private:
    
    vector<shared_ptr<void>> m_objPool; // Pool Container of void MAX_OBJ_SIZE
    const size_t m_maxCapacity;
    const size_t m_minCapacity;
    bool is_replinshment_complete; // flag to run the objectPool replenishment thread.
    std::thread t;
    mutex mutex_ObjPool;  // Mutex for handling atomic function access to m_objPool
    condition_variable cv;
};



#endif
