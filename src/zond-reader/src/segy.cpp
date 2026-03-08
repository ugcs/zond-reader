#include <segy.h>
#include <exception.h>
#include <cstring>
#include <cmath>
#include <ctime>


using namespace segy;

float
scalarValue(float value, float scalar)
{
	return (scalar == 0 ? value : (scalar < 0 ? value * fabs(scalar) : value / scalar));
}

void
fillZeroDelayAndScalar(format::TraceHeader& header, float period, int16_t index)
{
	int16_t zeroPointTimeScalar = -1000;
	float f = index * period;
	int32_t zeroPointTime = scalarValue(f, zeroPointTimeScalar);
	while (std::abs(zeroPointTime) > 32000)
	{
		zeroPointTime = f;
		if (zeroPointTimeScalar < -1)
			zeroPointTimeScalar /= 10;
		else if (zeroPointTimeScalar <= 0)
			zeroPointTimeScalar = 1;
		else
			zeroPointTimeScalar *= 10;

		zeroPointTime = scalarValue(f, zeroPointTimeScalar);
	}

	header.ZeroPoLeInt32Time = zeroPointTime;
	header.ZeroPoLeInt32TimeScalar = zeroPointTimeScalar;
}

SegyFile::~SegyFile()
{
	close();
}

void
SegyFile::open(const Params &params)
{
	if (f.is_open()) {
		THROW(InvalidOpException, "File is already open");
	}
	this->params = params;
	try {
		f.open(path, f.out | f.binary | f.trunc);
		writeHeader();
	} catch (std::exception &e) {
		throw;
	}
}

void
SegyFile::close()
{
	if (!f.is_open()) {
		return;
	}
	f.close();
	params.reset();
	curTraceId = 0;
}

void
SegyFile::writeHeader()
{
	std::string txtHeader;
	constexpr size_t CHARS_PER_LINE = 80;
	constexpr size_t DESCRIPTION_SIZE = 2000;
	constexpr size_t HISTORY_SIZE = 1080;
	constexpr size_t MAX_LINES = format::TEXT_HEADER_SIZE / CHARS_PER_LINE;

	std::map<int, std::string> defTextHeader = {
			{0, "This SEG-Y file has been generated as an example."},
			{1, "Zond Reader application https://github.com/ugcs/zond-reader"}
	};

	const std::map<int, std::string>& textHeaderSource =
			(params && !params->textHeader.empty()) ? params->textHeader : defTextHeader;

	size_t lineCount = 0;
	for (const auto& [lineIndex, lineText] : textHeaderSource) {
		if (lineCount >= MAX_LINES)
			break;

		txtHeader += lineText.substr(0, CHARS_PER_LINE - 2);
		txtHeader += "\r\n";
		++lineCount;
	}

	if (txtHeader.size() < format::TEXT_HEADER_SIZE) {
		txtHeader.append(format::TEXT_HEADER_SIZE - txtHeader.size(), ' ');
	} else if (txtHeader.size() > format::TEXT_HEADER_SIZE) {
		txtHeader.resize(format::TEXT_HEADER_SIZE);
	}

	if (params->useRadsysFormat) {
		format::RadsysTextHeader radsysHeader;

		std::memcpy(radsysHeader.Description, txtHeader.data(), DESCRIPTION_SIZE);
		std::memcpy(radsysHeader.ProcessingsHistory,
				txtHeader.data() + DESCRIPTION_SIZE,
				HISTORY_SIZE);

		f.write(reinterpret_cast<const char*>(&radsysHeader), sizeof(radsysHeader));
	} else {
		f.write(txtHeader.data(), format::TEXT_HEADER_SIZE);
	}

	format::BinaryHeader binHeader;
	binHeader.SampleTime = params->sampleTime;
	binHeader.RecSampleInterval = params->sampleTime;
	binHeader.Samples = params->samplesPerTrace;
	binHeader.RecSamples = params->samplesPerTrace;
	binHeader.SampleFormat = static_cast<int16_t>(params->sampleFormat);

	f.write(reinterpret_cast<const char *>(&binHeader), sizeof(binHeader));

	f.flush();
}

SegyFile::TraceId
SegyFile::writeTrace(const Trace &trace)
{
	size_t sampleSize = this->sampleSize();
	if (trace.dataSize != sampleSize * params->samplesPerTrace) {
		THROW(InvalidParamException, "Trace data size mismatch");
	}

	auto DegToMin = [](double deg){
		double fracPart, intPart;
		fracPart = std::modf(deg , &intPart);
		return intPart * 100 + fracPart * 60;
	};

	if (curTraceId == 0) {
		params->zeroSampleIndex = findZeroPoint(trace);
	}

	format::TraceHeader hdr;
	hdr.TraceNumber = curTraceId;
	hdr.FieldRecordNumber = curTraceId;
	hdr.CdpEnsembleTraceNumber = curTraceId;
	hdr.HorSummedTraces = trace.horSummedTraces;
	hdr.LongitudeAsFloat = static_cast<float>(DegToMin(trace.longitude));
	hdr.LatitudeAsFloat = static_cast<float>(DegToMin(trace.latitude));
	hdr.ReceiverX = 0;
	hdr.ReceiverY = 0;
	hdr.subweatheringVelocity = trace.speedOfSound;
	hdr.LongitudeAsDouble = DegToMin(trace.longitude);
	hdr.LatitudeAsDouble = DegToMin(trace.latitude);
	hdr.groupElevation = static_cast<float>(trace.altitude);
	hdr.Marker = static_cast<int16_t>(trace.markNumber);
	hdr.TraceSamples = params->samplesPerTrace;
	hdr.SampleTime = params->sampleTime;
	hdr.sweepLength = params->pulseLengthUs;
	hdr.sourceDepthBelowSurface = trace.transducerDraft;
	hdr.datumElevationAtReceiverGroup = trace.datumElevation;
	hdr.datumElevationAtSource = trace.datumElevation;
	hdr.columnHeightAtGroup = trace.columnHeight;
	hdr.columnHeightAtSource = trace.columnHeight;

	std::time_t curTime = std::time(nullptr);
	std::tm tm = *std::gmtime(&curTime);
	mktime(&tm);
	hdr.Year = tm.tm_year + 1900;
	hdr.DayOfYear = tm.tm_yday + 1; //Day of year starts from 1 in SEGY header
	hdr.Hour = tm.tm_hour;
	hdr.Minute = tm.tm_min;
	hdr.Second = tm.tm_sec;
	hdr.TimeCode = format::TimeBasis::Gmt;

	if (params->useRadsysFormat) {
		fillZeroDelayAndScalar(hdr, params->sampleTime / 1000.0f, params->zeroSampleIndex);
	}

	f.write(reinterpret_cast<const char *>(&hdr), sizeof(hdr));

	for (size_t i = 0; i < params->samplesPerTrace; i++) {
		if (params->sampleFormat == format::SampleFormat::Integer16Bit) {
			LeInt16 i16Buf = (reinterpret_cast<const int16_t *>(trace.data)[i]);
			f.write(reinterpret_cast<const char *>(&i16Buf), sampleSize);
		} else {
			LeInt32 i32Buf = (reinterpret_cast<const int32_t *>(trace.data)[i]);
			f.write(reinterpret_cast<const char *>(&i32Buf), sampleSize);
		}
	}

	f.flush();
	return curTraceId++;
}

int16_t
SegyFile::findZeroPoint(const Trace& trace)
{
	int threshold = 0;
	switch (params->sampleFormat) {
		case format::SampleFormat::Integer16Bit:
		case format::SampleFormat::Integer16BitWithGain:
			threshold = 0x7FF;
			break;
		case format::SampleFormat::Integer32Bit:
		case format::SampleFormat::IbmFloat32Bit:
			threshold = 0x7FFFFFF;
			break;
	}

	int res = 0, N = params->samplesPerTrace - 2;
	if (N > 0) {
		for (int i = 0; i <= N; i++) {
			if (abs(trace.at(i, params->sampleFormat)) > threshold) {
				res = i;
				while (res < N &&
						abs(trace.at(res, params->sampleFormat)) <
						abs(trace.at(res + 1, params->sampleFormat))) {
					res++;
				}
				break;
			}
		}
	}
	return res;
}
