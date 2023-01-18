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

	m_height = params.getSampleCount();
	m_target_width = params.getImageWidth();
	m_output_dir = params.getOutputDir();
	m_overlap = params.getImagesOverlap();
}

void GprWriter::write(const byte_array_t& data)
{
	static int trace_num = 0; //global trace counter
	static int sample_idx = 0; //global sample counter

	size_t readout = 0; //how many bytes processed from the data buffer
	//Read trace bytes as 16-bit values:
	auto traceData = reinterpret_cast<const int16_t*>(data.data());
	while(readout < data.size()) {

		//First, check if any images are filled and ready for storage:
		for(auto p = m_results.begin(); p != m_results.end(); p++) {
			auto & current_width = p->first;
			auto & result = p->second;
			if(current_width == m_target_width) {
				startJpegWrite(result);
				p = m_results.erase(p); //Remove written image from collection
				m_file_num++; //next output file number
			}
		}

		//If no working images are left, add a new one:
		if(m_results.empty())
			m_results.push_back(std::make_pair(0, Mat(m_height, m_target_width, CV_32FC1)));

		//Now check if the last image is in range of overlaping with next sibling:
		auto last_width = m_results.back().first;
		if(m_target_width - last_width < m_overlap) //add next image if in overlap range:
			m_results.push_back(std::make_pair(0, Mat(m_height, m_target_width, CV_32FC1)));

		//Copy from original buffer in sync mode
		//just to be sure it will not be overwrited:
		for(; sample_idx < m_height && readout < data.size(); sample_idx++) { //start from previous sample index
			int16_t sample = *traceData;
			traceData++; //move to next value
			for(auto & p : m_results) //add sample into all images are in work:
				p.second.at<float>(sample_idx, p.first) = sample;

			readout += sizeof(int16_t);
		}

		if(sample_idx == m_height) {//full trace was read out
			sample_idx = 0;
			std::cout << "Trace " << trace_num++ << " goes to file " << m_file_num << std::endl;
			//Update width counters of all working images:
			for(auto & p : m_results)
				p.first++;
		}
	}
}

void GprWriter::startJpegWrite(const cv::Mat& result)
{
	asio::post(m_context, [data = result,
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
