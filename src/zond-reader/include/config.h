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

	std::string getHostName() const;
	int getPort() const;

	uint16_t getSampleCount() const;
	uint16_t getPulseDelay() const;
	uint16_t getTimeRange() const;
	uint16_t getStacking() const;
	uint16_t getTxFrequency() const;

private:
	std::string output_dir_name;

	const unsigned DEFAULT_WIDTH = 500;
	const char* ENV_INPUT_DIR = "ZGPR_INPUT_DIR";
	const char* ENV_IMG_WIDTH = "ZGPR_IMAGE_WIDTH";

	unsigned img_width = DEFAULT_WIDTH;

	std::string host_name;
	int port;

	uint16_t sample_count;
	uint16_t pulse_delay;
	uint16_t time_range;
	uint16_t tx_freq;
	uint16_t stacking;
};
