#pragma once

#include <channelsetup.h>
#include <util.h>
#include <config.h>
#include <vector>
#include <asio.hpp>



class RadSysZondGpr
{
public:
	RadSysZondGpr(const std::string& sensorHostName,
			int sensorPort,
			uint16_t sampleCount,
			const std::vector<std::string>& highPassFilters,
			const std::string& soundingMode,
			const std::string& channelMode,
			const std::vector<uint16_t>& pulseDelays,
			const std::vector<uint16_t>& timeRanges,
			asio::io_context& context);

	RadSysZondGpr(const ParamsCLI& config,
			asio::io_context& context);

	virtual ~RadSysZondGpr();

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

private:
	bool isDualChannel();
	void parseTrace(const byte_array_t &data, std::size_t length);
	void openConnection();
	void sendModelRequest();
	void sendStart();

	void init(const std::string &model);
	uint16_t timeRange(const std::string &model, uint16_t defaultValue = 0);

	void tryToConnect(const std::string& address, int port);

	virtual void processGprData(const byte_array_t& data) = 0;

private:
	asio::io_context& m_context;
	std::unique_ptr<asio::ip::tcp::socket> m_socket;
	std::string m_hostName;
	int m_port;
	byte_array_t m_received_data;

	ChannelSetup m_channels[2];


	enum ParsingState {
		InitialState,
		ModelState,
		ConfigureState,
		PreparationState,
		TraceState,
	};

	ParsingState m_parsingState = InitialState;

	static int bytesInSample;
	bool m_liteMode = false;
	uint m_badTraceCount = 0;
};

