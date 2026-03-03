#include "threadpool.h"
#include "ioprocess.h"

// creates MAX_THREADS number of threads and locks em in the worker_loop
ThreadPool::ThreadPool() {
	// MAX_THREADS must be picked appropriately
	for (int i = 0; i < MAX_THREADS; i++) {
		// used "this" as ThreadPool::worker_loop() is a memfunc
		m_threads.emplace_back(std::thread(&ThreadPool::worker_loop, this));
	}
}

// simple destructor to CALL UPON DESTRUCTION!!
ThreadPool::~ThreadPool() {
	if(m_WorkComplete) return;
	shutdown();
}

// adds the given IO_Process to the job queue
void ThreadPool::add_process(IO_process process) {
	// first we lock the primary mutex to avoid racing
	m_mutex.lock();
	// push by move semantic to avoid excess copies
	m_processes.push(std::move(process));
	// unlock the primary mutex
	m_mutex.unlock();
	// notify a worker that a job is here
	m_controller.notify_one();
}

// shuts down everything and joins the threads
void ThreadPool::shutdown() {
	m_mutex.lock();
	m_WorkComplete = true;
	m_mutex.unlock();
	m_controller.notify_all();
	for (auto& i : m_threads) {
		if (i.joinable())
			i.join();
	}
}

// the main worker loop, puts a thread to sleep and awakes when a job is here
void ThreadPool::worker_loop() {
	IO_process job;
	while (true) {
		// braces represent where the unique lock would be present
		{
			// lock to be used
			std::unique_lock<std::mutex> lock(m_mutex);

			// * The Controller Condition makes the current executing thread wait until,
			// * Either the m_process queue is completely empty or,
			// * the pool will not be receiving any more job
			// * It should be noted that that wait() would release the lock
			m_controller.wait(lock, [this] { return !m_processes.empty() || m_WorkComplete; });

			// if no work is present AND none will ever come, return
			if (m_processes.empty() && m_WorkComplete)
				return;

			// quickly move the job to a local IO Process struct
			job = std::move(m_processes.front());
			// pop the job off the queue
			m_processes.pop();
		}
		// TRY to copy the given file
		try {
			copy_file_engine(job);
		}
		// If any error is thrown, store it in the errors vector and WE CONTINUE!
		catch (std::exception& e) {
			m_error_mutex.lock();
			m_exceptions.push_back(std::current_exception());
			m_error_mutex.unlock();
		}
	}
}
