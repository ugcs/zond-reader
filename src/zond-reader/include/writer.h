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
#include <segy.h>
#include <list>


class GprWriter
{
public:
	GprWriter(asio::io_context& io, const ParamsCLI& params);
	void write(const byte_array_t& data);
private:
	asio::io_context& m_context;
	int m_target_width;
	int m_current_width;
	unsigned m_overlap;

	std::string m_output_dir;
	int m_file_num;
	std::optional<segy::SegyFile> m_result;
	segy::SegyFile::Params m_param;

	void startSegyWrite();
};
