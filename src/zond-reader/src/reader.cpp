/*
 * reader.cpp
 *
 *  Created on: Dec 13, 2022
 *      Author: max
 */

#include <reader.h>
#include <iostream>


SensorReader::SensorReader(asio::io_context& io,
							GprWriter& writer,
							const ParamsCLI& params)
: m_writer(writer), RadSysZondGpr(params, io)
{
}

SensorReader::~SensorReader()
{
}

void SensorReader::processGprData(const byte_array_t& data)
{
	std::cout << "Process " << data.size() << " of data" << std::endl;
}
