/*
 * reader.cpp
 *
 *  Created on: Dec 13, 2022
 *      Author: max
 */

#include <reader.h>
#include <iostream>


SensorReader::SensorReader(asio::io_context& io,
		const ParamsCLI& params)
: RadSysZondGpr(params, io), DataProc(io, params)
{
}

SensorReader::~SensorReader()
{
}

void SensorReader::processGprData(const byte_array_t& data)
{
	m_writer.write(data);
}

void SensorReader::start()
{
	RadSysZondGpr::start();
}

void SensorReader::stop()
{
	RadSysZondGpr::stop();
}
