/*
 * debug.cpp
 *
 *  Created on: Jan 17, 2023
 *      Author: max
 */

#include <debug.h>
#include <util.h>


DebugDataSource::DebugDataSource(asio::io_context& io,
								 const ParamsCLI& params)
: DataProc(io, params), m_context(io), m_continue(false)
{
	m_height = params.getSampleCount();
	m_data.resize(m_height * sizeof(int16_t));
}

DebugDataSource::~DebugDataSource()
{}

void DebugDataSource::start()
{
	m_continue = true;
	asio::post(m_context, std::bind(&DebugDataSource::generateData, this));
}

void DebugDataSource::stop()
{
	m_continue = false;
}

void DebugDataSource::generateData()
{
	//Gradient increment value
	const int16_t dv = 10;

	//Border of gradient and dark part of pattern. Looks like astronomical terminator.
	static int border = 0;
	//Border line angle coefficient. 1 corresponds to 45 degrees
	static int dx = 1;

	if(m_continue)
	{

		auto traceData = reinterpret_cast<int16_t*>(m_data.data());
		//Generate debug pattern
		int16_t val = 0;
		for(int i = 0; i < m_height; i++)
		{
			if(i < border)
				*traceData = val;
			else
				*traceData = 0;
			traceData++;
			val += dv;
		}
		//Update border limit for the next trace:
		border += dx;
		if((border > m_height)||(border < 0))
			dx *= -1;

		//Submit generated data to writer:
		m_writer.write(m_data);
		//Schedule next generation:
		asio::post(m_context, std::bind(&DebugDataSource::generateData, this));
	}
}
