/*
 * datasource.cpp
 *
 *  Created on: Jan 17, 2023
 *      Author: max
 */


#include <datasource.h>

DataSource::DataSource(asio::io_context& io,
			const ParamsCLI& params):
			m_writer(io, params)
{}


DataSource::~DataSource()
{}

