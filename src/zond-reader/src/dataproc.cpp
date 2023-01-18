/*
 * datasource.cpp
 *
 *  Created on: Jan 17, 2023
 *      Author: max
 */


#include <dataproc.h>

DataProc::DataProc(asio::io_context& io,
			const ParamsCLI& params):
			m_writer(io, params)
{}


DataProc::~DataProc()
{}

