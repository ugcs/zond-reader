#ifndef INCLUDE_SEGY_H
#define INCLUDE_SEGY_H

#include <cstddef>
#include <defs.h>
#include <endian_def.h>
#include <filesystem>
#include <optional>
#include <fstream>
#include <map>

namespace segy {

namespace format {

constexpr size_t TEXT_HEADER_SIZE = 3200;
constexpr size_t BINARY_HEADER_SIZE = 400;
constexpr size_t TRACE_HEADER_SIZE = 240;

struct RadsysPoint {
	LeFloat X;
	LeFloat Y;
} PACKED;

enum class SampleFormat {
	IbmFloat32Bit = 1,
	Integer32Bit,
	Integer16Bit,
	Integer16BitWithGain
};

enum TraceSortingCode {
	AsRecorded = 1,
	CpdEnable,
	SingleFoldContinuousProfile,
	HorizontallyStacked
};

enum SweepType
{
	LinearSweepType = 1,
	ParabolicSweepType,
	ExponentialSweepType,
	OtherSweepType
};

enum MeasurementUnits {
	Meters = 1,
	Feet
};

enum TimeBasis {
	LocalTime = 1,
	Gmt,
	OtherTimeBasis,
	UTC,
	GPS
};

enum AmplitudeRecoveryMethod {
	None = 1,
	SphericalDivergence,
	Agc,
	Other
};

enum TraceIdType {
	SeismicDataId = 1,
	DeadId,
	DummyId,
	TimeBreakId,
	UpholeId,
	SweepId,
	TimingId,
	WaterBreakId,
	OptionalUseId
};

enum DataUse {
	ProductionUse = 1,
	TestUse
};

enum CoordUnits {
	LengthUnits = 1,
	ArcSecondsUnits,
	DecimalDegrees,
	DMS
};

enum TaperType {
	DefaultTaperType = 0,
	LinearTaperType,
	CosSquaredTaperType,
	OtherTaperType
};

enum Correlated {
	Yes = 1,
	No = 2
};

inline uint8_t formatToBitsNum(SampleFormat format) {
	switch (format) {
		case SampleFormat::Integer16Bit:
		case SampleFormat::Integer16BitWithGain:
			return 16;
		case SampleFormat::Integer32Bit:
		case SampleFormat::IbmFloat32Bit:
			return 32;
	}
	return 0;
}

/* RadSys has its own interpretation of text header. */
struct RadsysTextHeader {
	char Description[2000];                 // 0:	2000
	unsigned char ProcessingsHistory[1080]; // 2000:	1080
	LeInt32 TraceFrom = 0;                              // 3080:	4
	LeInt32 TraceTo = 0;                                // 3084:	4
	LeInt32 SampleFrom = 0;                             // 3088:	4
	LeInt32 SampleTo = 0;                               // 3092:	4

	unsigned char FilterType;   // TFilterType	//3096:	1
	LeFloat FilterPoints[4];      // 3097:	16
	char _reserved1[3] = {0};         // 3113:	3

	char DoubleCoordinates = 'D'; // 3116:	1
	char _reserved2[1] = {0};     // 3117:	1

	unsigned char GainPointsCount; // 3118:	1
	RadsysPoint GainPoint[10];     // 3119:	80

	char _reserved3[1] = {0}; // 3199:	1
} PACKED;

static_assert(sizeof(RadsysTextHeader) == TEXT_HEADER_SIZE, "RadsysTextHeader size mismatch");

struct BinaryHeader {
	LeInt32 JobIdentificationNumber;    // 0: 	4
	LeInt32 LineNumber = 1;                 // 4:	4
	LeInt32 ReelNumber = 1;                 // 8: 	4
	LeInt16 TracesPerRecord = 1;      // 12: 	2
	LeInt16 AuxTracesPerRecord;   // 14:	2
	LeInt16 SampleTime;           // in PICOseconds			//16:	2
	LeInt16 RecSampleInterval;    // 18:	2
	LeInt16 Samples;              // 20:	2
	LeInt16 RecSamples;           // 22:	2
	LeInt16 SampleFormat = static_cast<int16_t>(SampleFormat::Integer16Bit);         // 24: 	2
	LeInt16 CdpTracesPerEnsemble = 1; // 26:	2
	LeInt16 TraceSortingCode = TraceSortingCode::AsRecorded;     // 28:	2

	// char _reserved4[24] = {0};            // 30:	24
	// RadSys-specific {
	LeInt16 VerticalSum = 1;
	LeInt16 SweepStartFrequencyHz;
	LeInt16 SweepEndFrequencyHz;
	LeInt16 PulseLength;
	LeInt16 SweepType = 1;
	LeInt16 SweepChannelTraceNumber;
	LeInt16 SweepTraceTaperLenAtStart;
	LeInt16 SweepTraceTaperLenAtEnd;
	LeInt16 TaperType;
	LeInt16 CorrelatedDataTraces = Correlated::No;
	LeInt16 BinaryGainRecovered = Correlated::No;
	LeInt16 AmpRecoveryMethod = AmplitudeRecoveryMethod::None;
	// }

	LeInt16 MeasurementUnits = MeasurementUnits::Meters; // 54:    2
	LeInt16 ImpulsePolarity;
	LeInt16 VibratoryCode;

	LeInt32 ExtendedTracesPerEnsemble;
	LeInt32 ExtendedAuxTracesPerEnsemble;
	LeInt32 ExtendedSamplesPerTrace;
	LeInt64 ExtendedSampleInterval;
	LeInt64 ExtendedSampleIntervalOrigin;
	LeInt32 ExtendedSamplesPerTraceOrigin;
	LeInt32 ExtendedCdpFold;

	LeInt32 ByteOrder = 0x01020304;
	uint8_t _reserved6[200] = {0};

	uint8_t MajorSegYFormatRevNumber;
	uint8_t MinorSegYFormatRevNumber;
	LeInt16 FixedLengthTraceFlag;
	LeInt16 ExtendedHeaderNumber;
	LeInt32 MaxAdditionalHeaders;
	LeInt16 TimeBasisCode = TimeBasis::LocalTime;
	LeInt64 TraceNumber;
	LeInt64 FirstTraceByteOffset;
	LeInt32 FooterNumber;
	uint8_t _reserved7[68] = {0};
} PACKED;

static_assert(sizeof(BinaryHeader) == BINARY_HEADER_SIZE, "Binary header size mismatch");

struct TraceHeader {
	LeInt32 TraceNumber;                        // 0:	4
	LeInt32 ReelTraceNumber = 1;                // 4:	4
	LeInt32 FieldRecordNumber;                  // 8:	4
	LeInt32 FieldRecordTraceNumber;             // 12:	4
	char _reserved2[4] = {0};                   // 16:	4
	LeInt32 CdpEnsembleNumber = 10001;          // 20:	4
	LeInt32 CdpEnsembleTraceNumber = 1;         // 24:	4
	LeInt16 TraceIdCode = TraceIdType::OptionalUseId;   // 28:	2
	LeInt16 VertSummedTraces = 1;               // 30:	2
	LeInt16 HorSummedTraces = 1;                // 32:	2
	LeInt16 DataUse = DataUse::ProductionUse;   // 34:	2
	LeInt32 sourceToReceiverDistance;           // 36: 4
	LeFloat groupElevation;                     // 40: 4
	LeInt32 surfaceElevationAtSource;           // 44: 4
	LeInt32 sourceDepthBelowSurface;            // 48: 4
	LeInt32 datumElevationAtReceiverGroup;      // 52: 4
	LeInt32 datumElevationAtSource;             // 56: 4
	LeInt32 columnHeightAtSource;               // 60: 4
	LeInt32 columnHeightAtGroup;                // 64: 4
	LeInt16 scalarElevationsAndDepth = -100;    // 68: 2
	LeInt16 coordScalar = -1000;                // 70: 2

	LeFloat LongitudeAsFloat;                   // 72:	4
	LeFloat LatitudeAsFloat;                    // 76:	4

	LeFloat ReceiverX;                          // 80:	4
	LeFloat ReceiverY;                          // 84:	4
	LeInt16 CoordinateUnits = CoordUnits::ArcSecondsUnits;  // 88: 	2
	LeInt16 weatheringVelocity;                 // 90: 2
	LeInt16 subweatheringVelocity;              // 92: 2  (Speed of sound)
	LeInt16 upholeTimeAtSource;                 // 94: 2
	LeInt16 upholeTimeAtGroup;                  // 96: 2
	char _reserved5[10] = {0};                  // 98:	10
	LeInt16 ZeroPoLeInt32Time;      // in PICOseconds		//108: 	2
	char _reserved6[4] = {0};           // 110:	4
	LeInt16 TraceSamples;       // 114:	2
	LeInt16 SampleTime;         // in PICOseconds			//116: 	2

	LeInt16 gainType = 1;                   // 118: 2
	LeInt16 instrumentGainConstant;         // 120: 2
	LeInt16 instrumentInitialGain;          // 122: 2

	LeInt16 correlated = Correlated::No;    // 124:	2
	LeInt16 sweepFrequencyAtStart;          // 126:	2
	LeInt16 sweepFrequencyAtEnd;            // 128: 2
	LeInt16 sweepLength;                    // 130: 2
	LeInt16 SweepTypeCode = SweepType::LinearSweepType; // 132:	2
	LeFloat SparDOffset;                    // 134:	4
	LeInt16 TaperTypeCode = TaperType::LinearTaperType; // 138:	2
	LeFloat SparYaw;           // 140:	4
	LeFloat SparDYaw;          // 144:	4
	LeFloat SparDepth;         // 148:	4
	LeFloat SparDDepth;        // 152:	4

	LeInt16 Year;      // 156:	2
	LeInt16 DayOfYear; // 158:	2
	LeInt16 Hour;      // 160:	2
	LeInt16 Minute;    // 162:	2
	LeInt16 Second;    // 164:	2
	LeInt16 TimeCode = TimeBasis::LocalTime;  // 166:	2
	LeInt16 mSecond;   // 168:	2


	LeFloat LocalCrsY;          // 170:	4
	LeFloat LocalCrsX;          // 174:	4
	LeFloat LocalCrsH;          // 178:	4
	LeDouble LongitudeAsDouble; // 182:	8
	LeDouble LatitudeAsDouble;  // 190:	8

	LeDouble SparUtilLongitude; // 198:	8
	LeDouble SparUtilLatitude;  // 206:	8

	LeInt16 ZeroPoLeInt32TimeScalar; // 214: 	2

	LeDouble SparUtilAltitude; // 216:	8
	LeFloat SparDbField;       // 224:	4

	char _reserved12[6] = {0};      // 228:	6
	char LocalCrsCh1 = 0;         // 234: 	1
	char LocalCrsCh2 = 0;         // 235: 	1
	LeInt16 MarksIndicator = 0x5555; // 236:	2
	LeInt16 Marker = 0;         // 238:	2
} PACKED;

static_assert(sizeof(TraceHeader) == TRACE_HEADER_SIZE, "Trace header size mismatch");

} /* namespace format */

class SegyFile {
public:
	using TraceId = int32_t;

	struct Params {
		std::string deviceName;
		/** Custom lines for text header, indexed by line number (0-39). Line is truncated to 78
		 * characters. CRLF is added at the end of each line.
		 */
		std::map<int, std::string> textHeader;
		/** In psec. */
		uint16_t sampleTime = 1;
		uint16_t samplesPerTrace = 0;
		format::SampleFormat sampleFormat = format::SampleFormat::Integer32Bit;
		//XXX introduce flavour parameter
		bool useRadsysFormat = true;
		uint16_t pulseLengthUs = 0;
		int16_t zeroSampleIndex = 0;
    };

    struct Trace {
		/** Data format and size should correspond to the parameters specified for open() method. */
		const void *data;
		size_t dataSize;
		double longitude = 0, latitude = 0, altitude = 0;
		int32_t columnHeight = 0;
		int32_t datumElevation = 0;
		int32_t transducerDraft = 0;
		int32_t speedOfSound = 0;
		int year;
		int dayOfYear;
		int hour;
		int minute;
		int second;
		int markNumber = 0;
		int16_t delayRecordingTime = 0;
		int16_t scalarTime = 0;
		int16_t horSummedTraces = 1;

		int at(int i, format::SampleFormat format) const
		{
			int val = 0;
			switch (format)
			{
			case format::SampleFormat::Integer16Bit:
			case format::SampleFormat::Integer16BitWithGain:
				val = (reinterpret_cast<const int16_t *>(data)[i]);
				break;
			case format::SampleFormat::Integer32Bit:
			case format::SampleFormat::IbmFloat32Bit:
				val = (reinterpret_cast<const int32_t *>(data)[i]);
				break;
			}
			return val;
		}
	};

	SegyFile(const std::filesystem::path &path):
		path(path)
	{}

	~SegyFile();

	void
	open(const Params &params);

	void
	close();

	/** Write next trace data. Data format and size should correspond to the parameters specified
	 * for open() method.
	 * @return Trace ID for the written trace.
	 */
	TraceId
	writeTrace(const Trace &trace);

private:
	const std::filesystem::path path;
	std::optional<Params> params;
	std::ofstream f;
	TraceId curTraceId = 0;

	void
	writeHeader();

	size_t
	sampleSize() const
	{
		return params->sampleFormat == format::SampleFormat::IbmFloat32Bit ||
				params->sampleFormat == format::SampleFormat::Integer32Bit ?
				sizeof(uint32_t) : sizeof(uint16_t);
	}

	int16_t
	findZeroPoint(const Trace& trace);
};

} /* namespace segy */

#endif /* INCLUDE_SEGY_H */
