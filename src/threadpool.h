#pragma once

#include "ioprocess.h"
#include "utility.h"
#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <thread>
#include <mutex>
#include <queue>
#include <vector>
#include <iostream>
#include <chrono>

constexpr int MAX_THREADS =  8;
constexpr int PROGRESS_UPDATE_INTERVAL = 50; // in ms

void copy_file_engine(IO_process &process);

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
    // the total number of bytes to be moved, used for progress bar
    uint64_t m_totalBytes {};
    // the number of bytes that have been completed so far
    std::atomic<uint64_t> m_bytesCompleted {};

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
        const std::vector<std::exception_ptr>& get_errors() const {
            return m_exceptions;
        }
        // set the total bytes to transfer
        void set_total_bytes(uint64_t n){m_totalBytes = n;}
    private:
        // the function ran by each thread as it loops awaiting for 
        // IO Processes to be performed
        void worker_loop();
};