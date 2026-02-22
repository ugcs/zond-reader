#include <radsyszondgpr.h>
#include <string>
#include <vector>
#include <exception>
#include <iostream>
#include <asio/ip/address.hpp>


/*Define primary constanst and default values for Zond configuration*/
namespace C {

//Default settings for device, as human-readable strings or integers
const int DefaultSampleCount = 256;

// Consts
const int  SampleBitDepth = 16; //Default sample bit depth
const char Marker[] = {0x00, 0x7f, 0x00, 0x7f}; //Magic sequence to initialize
const int  MarkerLength  = sizeof(Marker); //Lenght of magic sequence
const int ServiceSampleDivider = 16;


// RadSys Lite model prefixes - to detect which model we are dealing with
const char * const LiteModeModelMarker = "TR500_";
const char * const LiteModelName100Ns = "TR500_0";
const char * const LiteModelName200Ns = "TR500_2";
const char * const LiteModelName300Ns = "TR500_3";
const int LiteModeSampleCount = 512;


// Radsys Aero500
const char * const AeroModeModelMarker = "ZLITE_0";

} // namespace C


//Bytes per sample:
int RadSysZondGpr::bytesInSample = C::SampleBitDepth / 8;

//Driver class initialization
RadSysZondGpr::RadSysZondGpr(
		const std::string& sensorHostName,
		int sensorPort,
		uint16_t sampleCount,
		const std::vector<std::string>& highPassFilters,
		const std::string& soundingMode,
		const std::string& channelMode,
		const std::vector<uint16_t>& pulseDelays,
		const std::vector<uint16_t>& timeRanges,
		asio::io_context& context)
: m_context(context),
  m_received_data(500)

{
	/*
	 *  View Skyhub user manual for detailed description on every parameter.
	 *  There are two channels on device, each of them configured independantly.
	 *  Every channel will be initiated by the batch command, formed by ChannelSetup class.
	 *  Here we just fill paramter values from configuration into two ChannelSetup instances.
	 */
	m_hostName = sensorHostName;
	m_port = sensorPort;

	if(timeRanges.size() != 2)
		throw std::invalid_argument("Incorrect number of time range settings");
	if(highPassFilters.size() != 2)
		throw std::invalid_argument("Incorrect number of highPassFilterse settings");
	if(pulseDelays.size() != 2)
		throw std::invalid_argument("Incorrect number of pulseDelays settings");

	for(int idx=0; idx < 2; idx++) {
		m_channels[idx].timeRange = timeRanges[idx];
		m_channels[idx].highPassFilter = highPassFilters[idx];
		m_channels[idx].pulseDelay = pulseDelays[idx];
	}

	for(auto& channel : m_channels) {
		channel.sampleCount = sampleCount;
		channel.soundingMode = soundingMode;
		channel.channelMode = channelMode;
	}
}

RadSysZondGpr::RadSysZondGpr(const ParamsCLI& config,
		asio::io_context& context) :
				RadSysZondGpr(
						config.getHostName(), config.getPort(),
						config.getSampleCount(),
						config.getHighPassFilters(),
						config.getSoundingMode(),
						config.getChannelMode(),
						config.getPulseDelays(),
						config.getTimeRanges(),
						context)
{
}

RadSysZondGpr::~RadSysZondGpr()
{
}

//Device initialization routine:
void RadSysZondGpr::init(const std::string &model)
{
	//Special processing for some deivce models:
	if (model.rfind(C::LiteModeModelMarker, 0) == 0) {
		m_liteMode = true;
		m_channels[0].sampleCount = C::LiteModeSampleCount;
		m_channels[0].channelMode = ChannelSetup::Channel1Mode;
		m_channels[0].timeRange = timeRange(model, m_channels[0].timeRange);
	} else if (model.rfind(C::AeroModeModelMarker, 0) == 0) {
		m_liteMode = true;
		m_channels[0].sampleCount = C::LiteModeSampleCount;
		m_channels[0].channelMode = ChannelSetup::Channel1Mode;
	}

	//Check if configured sample count is valid for channel 0.
	//If not - switch to default value
	if (!m_channels[0].isSampleCountValid())
		m_channels[0].sampleCount = C::DefaultSampleCount;

	//The same check for channel 1.
	if (!m_channels[1].isSampleCountValid())
		m_channels[1].sampleCount = C::DefaultSampleCount;

	for(auto & channel : m_channels) {
		//Turn on antenas on both channels:
		channel.antennaTransmitter = ChannelSetup::TransmitterOn;
		//Use combinned cables for both channels:
		channel.cable = ChannelSetup::CombinedCable;
	}
}

//Then driver initialized, send pings to detect if device answers
void RadSysZondGpr::start()
{
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

		//Send command to start data flow from the sensor:
		byte_array_t cmd = {'W', '\n'};
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
	else
		m_parsingState = PreparationState; //Resume parsing after configuration
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

//Configuration data must be send then 'magic sequence' - the marker will be detected
// in incoming byte stream. The search for the marker implemented in parsing routine:
void RadSysZondGpr::sendStart()
{
	//For some models, which works in "light mode" special command must be sent:
	if (m_liteMode) {
		byte_array_t cmd = {'P', '1', '\n'};
		m_socket->async_send(asio::const_buffer(cmd.data(), cmd.size()),
				std::bind(&RadSysZondGpr::onCommandSend, this, std::placeholders::_1, std::placeholders::_2));
	}

	//Convert configured parameters into device protocol sequence:
	//For channel 0:
	byte_array_t data = m_channels[0].toByteArray();
	//And for channel 1, if dual model is required:
	if (isDualChannel()) {
		auto chan2 = m_channels[1].toByteArray();
		data.insert(data.end(), chan2.begin(), chan2.end());
	}
	byte_array_t cmd;
	//Configuration is sent with "T" command
	cmd.push_back('T');
	//followed by parameters in device format:
	cmd.push_back(static_cast<char>(data.size()));
	cmd.insert(cmd.end(), data.begin(), data.end());
	//and the end-of-line symbol:
	cmd.push_back('\n');

	std::string str_cmd(cmd.begin(), cmd.end());

	std::cout << "Setup command:" << str_cmd << std::endl;

	m_socket->async_send(asio::const_buffer(cmd.data(), cmd.size()),
			std::bind(&RadSysZondGpr::onSetupSend, this, std::placeholders::_1, std::placeholders::_2));
}

//This is the FSM routine to parse incoming binary stream chunk-by-chunk:
void RadSysZondGpr::parseTrace(const byte_array_t &data, std::size_t length)
{
	static bool parseFailureFlag = false;
	static int traceByteCount = 0;

	// If data length equal to traceByteCount, it means that it is a full trace
	if (parseFailureFlag && length != traceByteCount) {
		std::cerr << "Expected data with length " << traceByteCount << ". Current data length " << length << std::endl;
		m_badTraceCount++;
		return;
	}
	parseFailureFlag = false;

	static int markerRawIndex = 0;
	static int markerDataIndex = 0;
	static byte_array_t model;
	static byte_array_t trace;

	//FSM automatically transmits from Initial state to Model state:
	if (m_parsingState == InitialState)
		m_parsingState = ModelState;

	//Do not parse anything while configuration awaiting
	if (m_parsingState == ConfigureState)
		return;

	if (m_parsingState == ModelState) {
		//In model state we search for Marker and then for model name:
		for (int i = 0; i < length; ++i) {
			data.at(i) == C::Marker[markerRawIndex] && markerRawIndex < C::MarkerLength
					? ++markerRawIndex
							: markerRawIndex = 0;
			if (markerRawIndex >= C::MarkerLength && model.empty()) {
				//TODO: rework for std::string
				model.resize(length - markerRawIndex);
				memcpy(model.data(), data.data() + markerRawIndex, model.size());
				auto str_model = std::string(model.begin(), model.end());
				std::cout << "Model:" << str_model << std::endl;

				//Then model was found, launch device initialization routine:
				init(str_model);

				//Switch FSM into Configure state to suspend parsing until setup was sent
				m_parsingState = ConfigureState;
				markerRawIndex = 0;

				//Then request data transmission start:
				sendStart();
				return;
			}
		}
	}

	if (m_parsingState == PreparationState) {
		//In preparation state we prepare data logging for trace data:
		traceByteCount = bytesInSample * (m_liteMode ? m_channels[0].sampleCount + m_channels[0].sampleCount / C::ServiceSampleDivider
				: m_channels[0].sampleCount);
		trace.clear();
		trace.resize(traceByteCount);
		if (m_liteMode) {
			m_parsingState = TraceState;
		} else {
			for (int i = 0; i < data.size(); ++i) {
				data.at(i) == C::Marker[markerRawIndex] && markerRawIndex < C::MarkerLength
						? ++markerRawIndex
								: markerRawIndex = 0;
				if (markerRawIndex >= C::MarkerLength) {
					// check dummy byte
					markerRawIndex++;
					m_parsingState = TraceState;
					break;
				}
			}
		}
	}

	//In Trace state we receive trace values and then post them to log:
	if (m_parsingState == TraceState) {
		// handle tail of last package
		if (markerDataIndex != 0) {
			if (length >= traceByteCount - markerDataIndex) {
				memcpy(trace.data() + markerDataIndex,
						data.data() + markerRawIndex,
						traceByteCount - markerDataIndex);

				processGprData(trace);
				markerRawIndex += traceByteCount - markerDataIndex;
				markerDataIndex = 0;
			} else {
				// trace is not full
				memcpy(trace.data() + markerDataIndex,
						data.data() + markerRawIndex,
						length - markerRawIndex);
				markerDataIndex += length - markerRawIndex;
				markerRawIndex = length;
			}
		}
		// handle complete traces
		int traceCount = (length - markerRawIndex) / traceByteCount;
		for (int k = 0; k < traceCount; ++k) {
			memcpy(trace.data(),
					data.data() + markerRawIndex,
					traceByteCount);
			//logGprData(trace);
			processGprData(trace);
			markerRawIndex += traceByteCount;
		}
		// handle head of next package
		if (markerRawIndex < length) {
			memcpy(trace.data(),
					data.data() + markerRawIndex,
					length - markerRawIndex);
			markerDataIndex = length - markerRawIndex;
		} else if (markerRawIndex > length) {
			std::cerr << "Wrong case: Data length: " << length
							<< ", markerDataIndex: " << markerDataIndex
							<< ", markerRawIndex: " << markerRawIndex
							<< std::endl;

			parseFailureFlag = true;
			markerDataIndex = 0;
		} else {
			// do nothing
		}
		markerRawIndex = 0;
	}
}

//Determinate if we configured for single or double channel mode:
bool RadSysZondGpr::isDualChannel()
{
	return (m_channels[0].channelMode == ChannelSetup::TwoChannelsMode && m_channels[1].channelMode == ChannelSetup::TwoChannelsMode) ||
			(m_channels[0].channelMode == ChannelSetup::CircleMode && m_channels[1].channelMode == ChannelSetup::CircleMode);
}

//Convert time range from configuration paramter to protocol specific values
uint16_t RadSysZondGpr::timeRange(const std::string& model, uint16_t defaultValue)
{
	if (model == C::LiteModelName100Ns)
		return 100;
	if (model == C::LiteModelName200Ns)
		return 200;
	if (model == C::LiteModelName300Ns)
		return 300;
	return defaultValue;
}

