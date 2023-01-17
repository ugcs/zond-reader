#pragma once

#include <writer.h>
#include <asio.hpp>
#include <config.h>

class DataSource
{
public:
	DataSource(asio::io_context& io,
			const ParamsCLI& params);
	virtual ~DataSource();

	virtual void start() = 0;
	virtual void stop() = 0;
protected:
	GprWriter m_writer;
};
