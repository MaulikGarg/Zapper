#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include "copyengine.h"
#include "ioprocess.h"
#include "utility.h"
#include <condition_variable>
#include <exception>
#include <thread>
#include <mutex>
#include <queue>
#include <vector>

#define MAX_THREADS 8

class ThreadPool{
    // the primary thread pool
    std::vector<std::thread> m_threads;
    // the queue containing the currently pushed IO Processes
    std::queue<IO_process> m_processes;
    // the primary mutex for locking
    std::mutex m_mutex;
    // the primary controller to notify threads
    std::condition_variable m_controller;
    // the error vector storing thread exceptions
    std::vector<std::exception_ptr> m_exceptions;
    // the mutex for locking the exceptions vector
    std::mutex m_error_mutex;
    // flag to indicate all work is complete
    bool m_WorkComplete {false};

    public:
        // primary constructor, creates all threads and puts
        // them to sleep
        ThreadPool();
        // primary destructor, joins all threads gracefully
        ~ThreadPool();
        // pushes an IO Process to the current job queue
        void add_process(IO_process process); 
        // signals that no more IO processes shall be pushes
        // , sets m_WorkComplete and joins all threads
        void shutdown();
        // returns the exception vector
        std::vector<std::exception_ptr> get_errors() const {
            return m_exceptions;
        }
    private:
        // the function ran by each thread as it loops awaiting for 
        // IO Processes to be performed
        void worker_loop();
};

#endif