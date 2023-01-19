#pragma once

#include <dataproc.h>

class DebugDataSource : public DataProc
{
public:
	DebugDataSource(asio::io_context& io,
			const ParamsCLI& params);
	virtual ~DebugDataSource();

	virtual void start() override;
	virtual void stop() override;
private:
	void generateData();
	bool m_continue;
	asio::io_context& m_context;
	int m_height;
	byte_array_t m_data;
};
