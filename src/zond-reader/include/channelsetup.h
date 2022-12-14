#pragma once

#include <map>
#include <string>
#include <util.h>

struct ChannelSetup
{
    ChannelSetup();

    inline static const std::string TransmitterOn {"ON"};
    inline static const std::string TransmitterOff {"OFF"};

    inline static const std::string SeparatedCable {"SEPARATED"};
    inline static const std::string CombinedCable {"COMBINED"};

    inline static const std::string CalibrationMode {"CALIBRATION"};
    inline static const std::string SoundingMode {"SOUNDING"};
    inline static const std::string TestSoundingMode {"TEST"};

    inline static const std::string Channel1Mode {"CHANNEL_1"};
    inline static const std::string Channel2Mode {"CHANNEL_2"};
    inline static const std::string TwoChannelsMode {"TWO_CHANNELS"};
    inline static const std::string Tx1Rx2Mode {"TX1_RX2"};
    inline static const std::string Tx2Rx1Mode {"TX2_RX1"};
    inline static const std::string CircleMode {"CIRCLE"};

    inline static const std::string OffFilter {"OFF"};
    inline static const std::string WeakFilter {"WEAK"};
    inline static const std::string StrongFilter {"STRONG"};
    inline static const std::string SuperStrongFilter {"SUPER_STRONG"};

    std::string antennaTransmitter = TransmitterOff;
    std::string cable = SeparatedCable;
    uint16_t timeRange = 50;
    uint16_t sampleCount = 128;
    std::string highPassFilter = OffFilter;
    std::string soundingMode = CalibrationMode;
    std::string channelMode = Channel1Mode;
    uint16_t pulseDelay = 0;

    byte_array_t toByteArray();

    bool isSampleCountValid() const { return m_sampleCountTranslator.find(sampleCount) != m_sampleCountTranslator.end(); }

private:
    std::map<std::string, byte_array_t> m_antennaTransmitterTranslator;
    std::map<std::string, byte_array_t> m_cableTranslator;
    std::map<int, byte_array_t> m_timeRangeTranslator;
    std::map<int, byte_array_t> m_sampleCountTranslator;
    std::map<std::string, byte_array_t> m_highPassFilterTranslator;
    std::map<int, byte_array_t> m_advancedTimeRangeTranslator;
    std::map<std::string, byte_array_t> m_soundingModeTranslator;
    std::map<std::string, byte_array_t> m_channelModeTranslator;

    byte_array_t bitsToChars(uint32_t bits, int count);
    void appendBytesOrDefault(byte_array_t& dest,
    		const std::map<std::string, byte_array_t>& source, const std::string& key,
			const byte_array_t& def);
    void appendBytesOrDefault(byte_array_t& dest,
    		const std::map<int, byte_array_t>& source, int key,
			const byte_array_t& def);
};

