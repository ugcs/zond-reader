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
#include <config.h>
#include <writer.h>
#include <radsyszondgpr.h>
#include <dataproc.h>


class SensorReader : public DataProc, RadSysZondGpr
{
public:
	SensorReader(asio::io_context& io, const ParamsCLI& params);
	virtual ~SensorReader();
	virtual void processGprData(const byte_array_t& data) override;

	virtual void start() override;
	virtual void stop() override;
};
