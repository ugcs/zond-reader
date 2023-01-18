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
	static int16_t val = 0;
	static int16_t dv = 10;

	static int samples_num = 0;
	static int border = 0;
	static int dx = 1;

	static byte_array_t data(1000);

	if(m_continue)
	{

		auto traceData = reinterpret_cast<int16_t*>(data.data());
		//Generate debug pattern
		for(int i = 0; i < data.size()/sizeof(int16_t); i++)
		{
			if(samples_num < border)
				*traceData = val;
			else
				*traceData = 0;
			traceData++;

			if(samples_num < m_height-1){
				samples_num++;
				val += dv;
			}
			else
			{
				val = 0;
				samples_num = 0;
				border += dx;
				if((border > m_height)||(border < 0))
					dx *= -1;
			}
		}
		//Submit generated data to writer:
		m_writer.write(data);
		//Schedule next generation:
		asio::post(m_context, std::bind(&DebugDataSource::generateData, this));
	}
}
