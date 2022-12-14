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


class GprWriter
{
public:
	GprWriter(asio::io_context& io);
	void write();
private:
	asio::io_context& m_context;
};
