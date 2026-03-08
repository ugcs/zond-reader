/*
 * reader.cpp
 *
 *  Created on: Dec 13, 2022
 *      Author: max
 */

#include <writer.h>
#include <iostream>


GprWriter::GprWriter(asio::io_context& io, const ParamsCLI& params) : m_context(io)
{
	m_file_num = 0;
	m_current_width = 0;

	m_target_width = params.getImageWidth();
	m_output_dir = params.getOutputDir();
	m_param.samplesPerTrace = params.getSampleCount();

	startSegyWrite(); //start new file
}

//Add trace data into files (single or multiple depends on overlapping).
//Data parameter must contain data for the whole trace
//Trace data is a Sample conter of 16-bit signed values
void GprWriter::write(const byte_array_t& data)
{
	static int trace_num = 0; //global trace counter

	//Read trace bytes as 16-bit values:
	auto traceData = reinterpret_cast<const int16_t*>(data.data());

	//First, check if any images are filled and ready for storage:
	if((m_current_width == m_target_width)||(!m_result)) {
		m_file_num++; //next output file number
		m_current_width = 0; //reset trace counter
		startSegyWrite(); //start new file
	}

	std::cout << "Trace " << trace_num++ << " goes to file " << m_file_num << '\r';

	segy::SegyFile::Trace trace;
	//Reference to trace data:
	trace.data = data.data();
	trace.dataSize = data.size();

	/*Setup GNSS coordinates and altitude here:
	trace.longitude = ;
	trace.latitude = ;
	trace.altitude = ;
	*/

	m_result.value().writeTrace(trace);
	m_current_width++;
}

void GprWriter::startSegyWrite()
{
	if(m_result)
		m_result.value().close();
	auto filename = m_output_dir + "/" + std::to_string(m_file_num) + ".sgy";
	m_result.emplace(filename);
	m_result.value().open(m_param);
}

