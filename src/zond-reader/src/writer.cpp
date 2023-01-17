/*
 * reader.cpp
 *
 *  Created on: Dec 13, 2022
 *      Author: max
 */

#include <writer.h>
#include <iostream>

using namespace cv;

GprWriter::GprWriter(asio::io_context& io, const ParamsCLI& params) : m_context(io)
{
	m_file_num = 0;
	m_current_width = 0;
	m_height = params.getSampleCount();
	m_target_width = params.getImageWidth();
	m_output_dir = params.getOutputDir();
}

void GprWriter::write(const byte_array_t& data)
{
	static int trace_num = 0; //global trace counter
	static int sample_idx = 0; //global sample counter

	size_t readout = 0; //how many bytes processed from the data buffer
	//Read trace bytes as 16-bit values:
	auto traceData = reinterpret_cast<const int16_t*>(data.data());
	while(readout < data.size()) {

	if(m_current_width == m_target_width) {
		startJpegWrite();
		m_current_width = 0; //reset counter for the new image
		m_file_num++; //next output file number
	}

	if((m_current_width == 0)&&(m_result.empty())) {//for the first trace in the new image
		m_result = Mat(m_height, m_target_width, CV_32FC1);
	}

	if(m_current_width < m_target_width)
	{
		//Copy from original buffer in syncro mode
		//just to be sure it will not be overwrited:
		for(; sample_idx < m_height && readout < data.size(); sample_idx++) { //start from previous sample index
			int16_t sample = *traceData;
			traceData++; //move to next value
			m_result.at<float>(sample_idx, m_current_width) = sample;
			readout += sizeof(int16_t);
		}
		if(sample_idx == m_height) {//full trace was read out
			sample_idx = 0;
			std::cout << "Trace " << trace_num++ << " goes to file " << m_file_num << std::endl;
			//Ready for the next portion:
			m_current_width++;
		}
	}
	}
}

void GprWriter::startJpegWrite()
{
	asio::post(m_context, [data = std::move(m_result),
						   file_idx = m_file_num,
						   output_dir = m_output_dir]{
		Mat out;
		convertTo8Bit(data, out);

		auto filename = output_dir + "/" + std::to_string(file_idx) + ".jpg";
		//Save into target file:
		imwrite(filename, out);
	});
}

void GprWriter::convertTo8Bit(const Mat& src, Mat& out)
{
	out = Mat(src.size(), CV_8UC1);
	double minv, maxv;
	minMaxLoc(src, &minv, &maxv);
	double scale = (maxv - minv);

	src.convertTo(out, CV_8U, 255.0/scale, -minv * 255.0/scale);
}
