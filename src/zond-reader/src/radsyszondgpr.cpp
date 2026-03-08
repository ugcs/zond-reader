#include <radsyszondgpr.h>
#include <string>
#include <vector>
#include <exception>
#include <iostream>
#include <asio/ip/address.hpp>

using namespace std::string_literals;


//Driver class initialization
RadSysZondGpr::RadSysZondGpr(
		const std::string& sensorHostName,
		int sensorPort,
		uint16_t sampleCount,
		uint16_t pulseDelay,
		uint16_t timeRange,
		uint16_t txFrequency,
		uint16_t stacking,
		asio::io_context& context)
: m_context(context),
  m_received_data(500)

{
	/*
	 *  View Skyhub user manual for detailed description on every parameter.
	 */
	m_hostName = sensorHostName;
	m_port = sensorPort;

	m_settings.timeRange = timeRange;
	m_settings.pulseDelay = pulseDelay;
	m_settings.samples = sampleCount;
	m_settings.txFreq = txFrequency;
	m_settings.stacking = stacking;
}

RadSysZondGpr::RadSysZondGpr(const ParamsCLI& config,
		asio::io_context& context) :
				RadSysZondGpr(
						config.getHostName(), config.getPort(),
						config.getSampleCount(),
						config.getPulseDelay(),
						config.getTimeRange(),
						config.getTxFrequency(),
						config.getStacking(),
						context)
{
}

RadSysZondGpr::~RadSysZondGpr()
{
}

//Then driver initialized, send pings to detect if device answers
void RadSysZondGpr::start()
{
	m_setupCommands = {
			"$stop #1"s,
			"$tune #2 set txfreq "s + std::to_string(m_settings.txFreq),
			"$tune #3 set stacking "s + std::to_string(m_settings.stacking),
			"$tune #4 set samples "s + std::to_string(m_settings.samples),
			"$tune #5 set delay "s+ std::to_string(m_settings.pulseDelay),
			"$tune #6 set time "s + std::to_string(m_settings.timeRange),
			"$start #7 "s,
	};
	m_initStep = m_setupCommands.begin();

	tryToConnect(m_hostName, m_port);
}

void RadSysZondGpr::tryToConnect(const std::string& address, int port)
{
	std::cout << "Connecting sensor" << std::endl;
	try {
		//Parse address string to get ip:
		auto addr = asio::ip::make_address(address);
		//Create endpoint:
		asio::ip::tcp::endpoint ep(addr,port);
		//Create socket:
		m_socket = std::make_unique<asio::ip::tcp::socket>(m_context, ep.protocol());
		//Try to connect:
		std::cout << "Try to open " << address << ":" << port << "..." << std::endl;
		m_socket->async_connect(ep, std::bind(&RadSysZondGpr::onConnected, this, std::placeholders::_1));
	}
	catch(std::exception &e)
	{
		std::cerr << e.what() << std::endl;
		//Clean the socket object:
		m_socket = nullptr;
	}
}

void
RadSysZondGpr::onConnected(const asio::error_code& error)
{
	if (error)
	{
		std::cerr << "Connection failed: " << error.message() << std::endl;
		//Clean the socket object:
		m_socket = nullptr;
		//Wait for 1 second before the next try:
		sleep(1);
		//Resubmit next connection try:
		asio::post(m_context,
				std::bind(&RadSysZondGpr::tryToConnect, this, m_hostName, m_port));
	}
	else
	{
		//Here the connection is opened:
		std::cout << "Sensor connected" << std::endl;

		//Set parsing FSM to initial state
		m_parsingState = InitialState;

		//Prepare data reading routine:
		m_socket->async_receive(asio::buffer(m_received_data, m_received_data.size()),
				std::bind(&RadSysZondGpr::onReadData, this, std::placeholders::_1, std::placeholders::_2));

		//Send command to check sensor protocol:
		byte_array_t cmd = {'$', 'w' ,'h', 'o', 'i', 's', '\r', '\n'};
		m_socket->async_send(asio::const_buffer(cmd.data(), cmd.size()),
				std::bind(&RadSysZondGpr::onCommandSend, this, std::placeholders::_1, std::placeholders::_2));
	}
}

void
RadSysZondGpr::onCommandSend(const asio::error_code& error,
		std::size_t bytes_transferred)
{
	if (error)
		std::cerr << "Command sending error: " << error.message() << std::endl;
}

void
RadSysZondGpr::onSetupSend(const asio::error_code& error,
		std::size_t bytes_transferred)
{
	if (error)
		std::cerr << "Data sending error: " << error.message() << std::endl;
	else {
		m_initStep++;
		sendStart();
	}
}

//Close TCP connection and stop processing
void RadSysZondGpr::stop()
{
	if(m_socket)
		m_socket->close();
}

//What to do then some data was read from device:
void RadSysZondGpr::onReadData(
		const asio::error_code& error,
		std::size_t bytes_transferred)
{
	if(!error) {
		//Processing of received data:
		parseTrace(m_received_data, bytes_transferred);
		//Schedule next read:
		m_socket->async_receive(asio::buffer(m_received_data, m_received_data.size()),
				std::bind(&RadSysZondGpr::onReadData, this, std::placeholders::_1, std::placeholders::_2));
	}
	else
	{
		std::cerr << "Receiving failed: " << error << std::endl;
	}
}

void RadSysZondGpr::sendStart()
{
	auto sendCommand = [this](const std::string& command) -> void {
		//auto command_string = command.c_str();
		byte_array_t cmd;
		cmd.reserve(command.length() + 2);
		cmd.insert(cmd.end(), command.begin(), command.end());
		//and the end-of-line symbol:
		cmd.push_back('\r');
		cmd.push_back('\n');

		std::cout << "Setup command:" << command << std::endl;

		m_socket->async_send(asio::const_buffer(cmd.data(), cmd.size()),
				std::bind(&RadSysZondGpr::onCommandSend, this, std::placeholders::_1, std::placeholders::_2));
	};
	if(m_initStep == m_setupCommands.end()) {
		std::cout << "Setup complete" << std::endl;
		m_parsingState = PreparationState;
	}
	else {
		sleep(1);
		sendCommand(*m_initStep);
	}
}

void RadSysZondGpr::sendModelRequest()
{
	byte_array_t cmd = {'$', 'a' ,'n', 't', 'e', 'n', 'n', 'a', '\r', '\n'};
	m_socket->async_send(asio::const_buffer(cmd.data(), cmd.size()),
			std::bind(&RadSysZondGpr::onCommandSend, this, std::placeholders::_1, std::placeholders::_2));
}

//This is the FSM routine to parse incoming binary stream chunk-by-chunk:
void RadSysZondGpr::parseTrace(const byte_array_t &data, std::size_t length)
{
	static int traceByteCount = 0;
	int rawIndex = 0;
	static bool traceBegin = false;

	static byte_array_t header;
	static byte_array_t trace;

	//Expect protocol ID respose:
	if (m_parsingState == InitialState)
	{
		const std::string expected("$iam zGPR"); //This is the marker for the correct protocol
		std::string_view whois(reinterpret_cast<const char*>(data.data()), expected.length());
		if(whois == expected) {
			std::cout << "zGPR protocol detected" << std::endl;
			m_parsingState = ModelState;
			sendModelRequest();
		}
		else {
			std::cerr << "Incorrect protocol. Expected [" << expected << "]" << " but got [" << whois << "]" << std::endl;
		}
		return;
	}

	//Do not parse anything while configuration awaiting
	if (m_parsingState == ConfigureState) {
		const std::string_view response(reinterpret_cast<const char*>(data.data()), length-2);
		std::cout << "Command response: [" << response << "]" << std::endl;
		m_initStep++;
		sendStart();
		return;
	}

	if (m_parsingState == ModelState) {
		const std::string expected("$antenna "); //This is the expected prefix for model name
		const std::string_view antenna(reinterpret_cast<const char*>(data.data()), expected.length());
		if(antenna != expected) {
			std::cerr << "Incorrect model format. Expected string started with [" << expected << "] but got [" << antenna << "]";
		}
		else {
			size_t start = expected.length();
			size_t end = start;
			for(; end < data.size() && data[end] != '\n'; end++);

			const std::string str_model(reinterpret_cast<const char*>(data.data())+start, end-start);
			std::cout << "Model: " << str_model << std::endl;

			//Switch FSM into Configure state to suspend parsing until setup was sent
			m_parsingState = ConfigureState;

			//Then request data transmission start:
			sendStart();
		}
		return;
	}

	while(rawIndex < length) {
		if(m_parsingState == PreparationState) {
			//Search for header start marker:
			for(rawIndex = 0; (rawIndex < length) && (data[rawIndex] != '$'); rawIndex++);

			if(data[rawIndex] != '$') {//did not found in the current buffer
				rawIndex = 0;
				return;
			}
			else {
				header.clear();
				rawIndex++; //advance to the next symbol
				m_parsingState = TraceHeaderState;
			}
		}
		if(m_parsingState == TraceHeaderState) {
			for(; (rawIndex < length) && (data[rawIndex] != '\r'); rawIndex++)
				header.push_back(data[rawIndex]);
			if(data[rawIndex] == '\r') {//trace header complete
				static const char traceMarker[] = {'t', 'r', 'a', 'c', 'e', ' ', '+'};
				int len;
				for(len = 0; (len < sizeof(traceMarker)) && (traceMarker[len] == header[len]);len++);
				if(len != sizeof(traceMarker)) {
					std::cerr << "Incorrect trace header format" << std::endl;
					rawIndex = 0;
					m_parsingState = PreparationState;
					return;
				}

				//Routine to parse numbers:
				auto parseInt = [](const byte_array_t& buf, int& pos, char terminator) -> uint16_t {
					uint16_t val = 0;
					for(;pos < buf.size() && buf[pos] != terminator; pos++)
						val = val*10 + buf[pos] - (uint16_t)'0';
					return val;
				};

				//Parse the binary part size:
				traceByteCount = parseInt(header, len, ' ');
				len++;//skip the space
				//Parse part num:
				int partNum = parseInt(header, len, '_');;
				len++;//skip separator
				int totalParts = parseInt(header, len, ' ');;
				len++;//skip the space
				int samples = parseInt(header, len, '_');
				len++;//skip separator
				int stacking = parseInt(header, len, ' ');

				rawIndex++; //advance to the next symbol
				m_parsingState = TraceDataState;
				traceBegin = false;
			}
			else { //need to wait for the next buffer
				rawIndex = 0;
				return;
			}
		}
		if(m_parsingState == TraceDataState) {
			if(!traceBegin) {
				rawIndex++; //skip LF first
				traceBegin = true;
				trace.clear();
			}
			//Read out trace data:
			for(;(rawIndex < length) && (trace.size() < traceByteCount); rawIndex++)
				trace.push_back(data[rawIndex]);
			if(trace.size() < traceByteCount) { //need more data before processing
				rawIndex = 0;
				return;
			}
			else { //trace read out complete
				//write trace data
				processGprData(trace);
				m_parsingState = PreparationState;
				return;
			}
		}
	}
}

