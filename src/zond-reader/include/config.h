#pragma once
#include <string>
#include <vector>


class ParamsCLI {
public:
	ParamsCLI(int argc, char **argv);

	//Where to write result files
	std::string getOutputDir() const;
	//Number of traces included per JPEG
	unsigned getImageWidth() const;
	unsigned getImagesOverlap() const;

	std::string getHostName() const;
	int getPort() const;

	uint16_t getSampleCount() const;
	std::vector<std::string> getHighPassFilters() const;
	std::string getSoundingMode() const;
	std::string getChannelMode() const;
	std::vector<uint16_t> getPulseDelays() const;
	std::vector<uint16_t> getTimeRanges() const;

	bool isDebugMode() const;

private:
	std::string output_dir_name;

	const unsigned DEFAULT_WIDTH = 500;
	const char* ENV_INPUT_DIR = "ZOND_INPUT_DIR";
	const char* ENV_IMG_WIDTH = "ZOND_IMAGE_WIDTH";

	unsigned img_width = DEFAULT_WIDTH;
	unsigned img_overlap = 0;

	std::string host_name;
	int port;

	uint16_t sample_count;
	std::vector<std::string> high_pass_filters;
	std::string sounding_mode;
	std::string channel_mode;
	std::vector<uint16_t> pulse_delays;
	std::vector<uint16_t> time_ranges;

	bool debug_mode;
};
