/*
 * reader.cpp
 *
 *  Created on: Dec 13, 2022
 *      Author: max
 */

#include <worker.h>
#include <iostream>


WorkerThread::WorkerThread(asio::io_context& io)
: m_context(io)
{
}

void WorkerThread::start()
{
	stop();
	m_worker = std::make_unique<std::thread>([&]
	{
		auto wg = asio::make_work_guard(m_context);
		while(true)
		{
			try {
				m_context.run();
				break;
			}
			catch(std::exception &e)
			{
				std::cerr << "Worker failed: " << e.what() << std::endl;
			}
		}
	});
}

void WorkerThread::stop()
{
	if((m_worker)&&(m_worker->joinable()))
	{
		m_worker->join();
		m_worker = nullptr;
	}
}
