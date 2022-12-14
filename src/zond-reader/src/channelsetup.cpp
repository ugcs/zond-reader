#include <channelsetup.h>

namespace C {
const int PulseDelayBitDepth = 10;
}

ChannelSetup::ChannelSetup()
{
    //Encoding for transmitter on/off mode:
    m_antennaTransmitterTranslator.emplace(TransmitterOn,  byte_array_t{'0'});
    m_antennaTransmitterTranslator.emplace(TransmitterOff, byte_array_t{'1'});

    //Encoding for used cable selection:
    m_cableTranslator.emplace(SeparatedCable, byte_array_t{'0'});
    m_cableTranslator.emplace(CombinedCable,  byte_array_t{'1'});

    //Encoding for supported time ranges:
    m_timeRangeTranslator.emplace(50, byte_array_t({'0', '0', '0'}));
    m_timeRangeTranslator.emplace(100, byte_array_t({'1', '0', '0'}));
    m_timeRangeTranslator.emplace(200, byte_array_t({'0', '1', '0'}));
    m_timeRangeTranslator.emplace(300, byte_array_t({'1', '1', '0'}));
    m_timeRangeTranslator.emplace(500, byte_array_t({'0', '0', '1'}));
    m_timeRangeTranslator.emplace(800, byte_array_t({'1', '0', '1'}));
    m_timeRangeTranslator.emplace(1200, byte_array_t({'0', '1', '1'}));
    m_timeRangeTranslator.emplace(2000, byte_array_t({'1', '1', '1'}));

    //Encondig for supported sample counts:
    m_sampleCountTranslator.emplace(128, byte_array_t({'0', '0'}));
    m_sampleCountTranslator.emplace(256, byte_array_t({'1', '0'}));
    m_sampleCountTranslator.emplace(512, byte_array_t({'0', '1'}));
    m_sampleCountTranslator.emplace(1024, byte_array_t({'1', '1'}));

    //Encoding for supported high pass filter mode:
    m_highPassFilterTranslator.emplace(OffFilter,         byte_array_t({'0', '0'}));
    m_highPassFilterTranslator.emplace(WeakFilter,        byte_array_t({'1', '0'}));
    m_highPassFilterTranslator.emplace(StrongFilter,      byte_array_t({'0', '1'}));
    m_highPassFilterTranslator.emplace(SuperStrongFilter, byte_array_t({'1', '1'}));

    //Encoding for advanced time ranges:
    m_advancedTimeRangeTranslator.emplace(12, byte_array_t({'1', '0'}));
    m_advancedTimeRangeTranslator.emplace(25, byte_array_t({'0', '1'}));

    //Encoding for supported sounding modes:
    m_soundingModeTranslator.emplace(CalibrationMode,  byte_array_t({'0', '1', '1', '0'}));
    m_soundingModeTranslator.emplace(SoundingMode,     byte_array_t({'1', '0', '1', '0'}));
    m_soundingModeTranslator.emplace(TestSoundingMode, byte_array_t({'1', '0', '0', '1'}));

    //Encoding for supported channels configurations:
    m_channelModeTranslator.emplace(Channel1Mode,    byte_array_t({'0', '0'}));
    m_channelModeTranslator.emplace(Channel2Mode,    byte_array_t({'0', '1'}));
    m_channelModeTranslator.emplace(TwoChannelsMode, byte_array_t({'1', '0'}));
    m_channelModeTranslator.emplace(CircleMode,      byte_array_t({'1', '0'}));
    m_channelModeTranslator.emplace(Tx1Rx2Mode,      byte_array_t({'1', '1'}));
    m_channelModeTranslator.emplace(Tx2Rx1Mode,      byte_array_t({'1', '1'}));
}

//Convert human-readable parameter values into device protocol encoding:
byte_array_t ChannelSetup::toByteArray()
{
	byte_array_t result;

    appendBytesOrDefault(result, m_antennaTransmitterTranslator, antennaTransmitter, {'1'});

    appendBytesOrDefault(result, m_cableTranslator, cable, {'0'});

    appendBytesOrDefault(result, m_timeRangeTranslator, timeRange, {'0', '0', '0'});

    appendBytesOrDefault(result, m_sampleCountTranslator, sampleCount, {'0', '0'});

    result.push_back ((str_toUpper(channelMode) == CircleMode || str_toUpper(channelMode) == Tx2Rx1Mode) ? '1' : '0');

    result.insert(result.begin(), {'0', '0', '0'});

    appendBytesOrDefault(result, m_highPassFilterTranslator, highPassFilter, {'0', '0'});

    appendBytesOrDefault(result, m_advancedTimeRangeTranslator, timeRange, {'0', '0'});

    result.insert(result.begin(), {'0'}); //Reserved

    appendBytesOrDefault(result, m_soundingModeTranslator, soundingMode, {'0', '1', '1', '0'});

    appendBytesOrDefault(result, m_channelModeTranslator, channelMode, {'0', '0'});

    auto pulse_delay = bitsToChars(pulseDelay, C::PulseDelayBitDepth);
    result.insert(result.begin(), pulse_delay.begin(), pulse_delay.end());
    return result;
}

//Convert bits to character string
byte_array_t
ChannelSetup::bitsToChars(uint32_t bits, int count)
{
	byte_array_t result;
    for (int i = 0; i < count; ++i)
        result.push_back( (bits & (1 << i)) ? '1' : '0');

    return result;
}

void
ChannelSetup::appendBytesOrDefault(byte_array_t& dest,
		const std::map<std::string, byte_array_t>& source,
		const std::string& key,
		const byte_array_t& def)
{
	auto val = source.find(str_toUpper(key));
	if(val != source.end())
		dest.insert(dest.end(), val->second.begin(), val->second.end());
	else
		dest.insert(dest.end(), def.begin(), def.end());
}

void
ChannelSetup::appendBytesOrDefault(byte_array_t& dest,
		const std::map<int, byte_array_t>& source, int key,
		const byte_array_t& def)
{
	auto val = source.find(key);
	if(val != source.end())
		dest.insert(dest.end(), val->second.begin(), val->second.end());
	else
		dest.insert(dest.end(), def.begin(), def.end());
}
