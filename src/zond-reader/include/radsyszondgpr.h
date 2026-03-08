#pragma once

#include <util.h>
#include <config.h>
#include <vector>
#include <asio.hpp>
#include <list>


class RadSysZondGpr
{
public:
	RadSysZondGpr(const std::string& sensorHostName,
			int sensorPort,
			uint16_t sampleCount,
			uint16_t pulseDelay,
			uint16_t timeRange,
			uint16_t txFrequency,
			uint16_t stacking,
			asio::io_context& context);

	RadSysZondGpr(const ParamsCLI& config,
			asio::io_context& context);

	virtual ~RadSysZondGpr();

protected:
	void start();
	void stop();

private:
	void onReadData(
			const asio::error_code& error,
			std::size_t bytes_transferred);

	void onConnected(const asio::error_code& error);
	void processFinished(int exitCode);
	void onCommandSend(const asio::error_code& error,
			std::size_t bytes_transferred);
	void onSetupSend(const asio::error_code& error,
			std::size_t bytes_transferred);

	void parseTrace(const byte_array_t &data, std::size_t length);
	void openConnection();
	void sendModelRequest();
	void sendStart();

	void tryToConnect(const std::string& address, int port);

	virtual void processGprData(const byte_array_t& data) = 0;

	asio::io_context& m_context;
	std::unique_ptr<asio::ip::tcp::socket> m_socket;
	std::string m_hostName;
	int m_port;
	byte_array_t m_received_data;

	struct {
		int timeRange;
		int txFreq;
		int stacking;
		int pulseDelay;
		int samples;
	} m_settings;


	enum ParsingState {
		InitialState,
		ModelState,
		ConfigureState,
		PreparationState,
		TraceHeaderState,
		TraceDataState,
	};

	ParsingState m_parsingState = InitialState;

	static int bytesInSample;
	std::list<std::string> m_setupCommands;
	std::list<std::string>::iterator m_initStep;

};

