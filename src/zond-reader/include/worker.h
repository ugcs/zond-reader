/*
 * reader_thread.h
 *
 *  Created on: Dec 12, 2022
 *      Author: max
 */

#pragma once
#include <memory>
#include <thread>
#include <asio.hpp>

class WorkerThread
{
public:
	WorkerThread(asio::io_context& io);
	void start();
	void stop();
private:
	std::unique_ptr<std::thread> m_worker;
	asio::io_context& m_context;
};
