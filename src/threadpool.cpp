#include "threadpool.h"

// creates MAX_THREADS number of threads and locks em in the worker_loop
ThreadPool::ThreadPool() {
	// MAX_THREADS must be picked appropriately
	for (int i = 0; i < MAX_THREADS; i++) {
		// used "this" as ThreadPool::worker_loop() is a memfunc
		m_threads.emplace_back(std::thread(&ThreadPool::worker_loop, this));
	}

	// create a seperate thread responsible for progress display

	auto progress_update = [this]() {
		while (!m_WorkComplete) {
			// updates every PROGRESS_UPDATE_INTERVAL seconds
			std::this_thread::sleep_for(std::chrono::milliseconds(PROGRESS_UPDATE_INTERVAL));
			if (m_totalBytes) {
				int percentage = ((double)(m_bytesCompleted.load()) / m_totalBytes) * 100;
				std::cout << "\rProgress: " << percentage << "% " << std::flush;
			}
		}
		std::cout << "\rProgress: 100% \n"
					 << std::flush;
	};

	m_threads.emplace_back(std::thread(progress_update));
}

// simple destructor to CALL UPON DESTRUCTION!!
ThreadPool::~ThreadPool() {
	// if work is already complete, do not call shutdown
	if (m_WorkSent)
		return;
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
	m_WorkSent = true;
	m_mutex.unlock();
	m_controller.notify_all();
	// join all file operation threads
	for (int i = 0; i < MAX_THREADS; i++) {
		if(m_threads[i].joinable()) m_threads[i].join(); 
	}
	
	// now that all work is actually complete
	m_WorkComplete = true;
	// join the progress thread now that all work is done
	if(m_threads[MAX_THREADS].joinable()) m_threads[MAX_THREADS].join();
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
			m_controller.wait(lock, [this] { return !m_processes.empty() || m_WorkSent; });

			// if no work is present AND none will ever come, return
			if (m_processes.empty() && m_WorkSent)
				return;

			// quickly move the job to a local IO Process struct
			job = std::move(m_processes.front());
			// pop the job off the queue
			m_processes.pop();
		}
		// TRY to copy the given file
		try {
			copy_file_engine(job);
			// we use memory order relaxed as addition order does not matter
			m_bytesCompleted.fetch_add(job.m_source_info.st_size, std::memory_order_relaxed);
		}
		// If any error is thrown, store it in the errors vector and WE CONTINUE!
		catch (std::exception& e) {
			m_error_mutex.lock();
			m_exceptions.push_back(std::current_exception());
			m_error_mutex.unlock();
		}
	}
}
