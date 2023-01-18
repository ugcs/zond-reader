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
#include <util.h>
#include <config.h>
#include <opencv2/opencv.hpp>
#include <list>


class GprWriter
{
public:
	GprWriter(asio::io_context& io, const ParamsCLI& params);
	void write(const byte_array_t& data);
private:
	asio::io_context& m_context;
	int m_target_width;
	unsigned m_overlap;

	int m_height;
	std::string m_output_dir;
	int m_file_num;
	//List of overlapping images in work:
	std::list<std::pair<int, cv::Mat>> m_results;

	void startJpegWrite(const cv::Mat& result);
	static void convertTo8Bit(const cv::Mat& src, cv::Mat& out);
};
