#include <string>
#include <vector>
#include <config.h>
#include <CLI11.hpp>
#include <exception>
#include <iostream>


ParamsCLI::
ParamsCLI(int argc, char **argv)
{
	std::string app_description("Splitter application");

	//If version number defined build string representation:
#ifdef PACKAGE_VERSION
#define APP_VERSION_(arg) " version " #arg
#define APP_VERSION(arg) APP_VERSION_(arg)

	app_description += APP_VERSION(PACKAGE_VERSION);
#endif

	CLI::App app{app_description, "zond-reader"};

	app.set_config("--config", "reader.conf", "Load parameters from config file", false);

	app.add_option("-o, --output", output_dir_name, "Directory to save result images")
			->default_val(".");

	app.add_option("-w, --width", img_width, "Number of traces per output image")
			->envname(ENV_IMG_WIDTH)
			->default_val(DEFAULT_WIDTH);

	app.add_option("-a, --address", host_name, "IP or address of sensor")
			->default_val("192.168.0.10");

	app.add_option("-p, --port", port, "TCP port of sensor")
			->default_val(23);

	app.add_option("-c, --sample-count", sample_count, "Sample count")
			->default_val(512);

	high_pass_filters = {"OFF", "OFF"};
	app.add_option("-f, --high-pass-filters", high_pass_filters, "High-pass filters")
			->expected(2)
			->default_val(high_pass_filters);

	app.add_option("-s, --sounding-mode", sounding_mode, " Sounding mode")
			->default_val("SOUNDING");

	app.add_option("-m, --channel-mode", channel_mode, " Channel mode")
			->default_val("CHANNEL_1");

	pulse_delays = {0, 0};
	app.add_option("-d, --pulse-delays", pulse_delays, "Pulse delays")
			->expected(2)
			->default_val(pulse_delays);

	time_ranges = {300, 300};
	app.add_option("-t, --time-ranges", time_ranges, "Time ranges")
			->expected(2)
			->default_val(time_ranges);

	app.add_option("-g, --debug", debug_mode, "Fill files with debug pattern instead of data from the device")
			->default_val(false);


	try {
		app.parse( argc, argv);
		//Update config file with new options:
		std::string conf_source = app["--config"]->as<std::string>();
		if(!conf_source.empty()) {
			std::ofstream conf_file(conf_source);
			conf_file << app.config_to_str(true, true);
			conf_file.close();
		}
	}
	catch(CLI::ParseError &e) {
		std::cerr << app.help() << std::endl;
		throw std::invalid_argument(e.what());
	}
}

std::string
ParamsCLI::
getOutputDir() const
{
	return output_dir_name;
}

unsigned
ParamsCLI::
getImageWidth() const
{
	return img_width;
}

std::string
ParamsCLI::
getHostName() const
{
	return host_name;
}

int
ParamsCLI::
getPort() const
{
	return port;
}

uint16_t
ParamsCLI::
getSampleCount() const
{
	return sample_count;
}

std::vector<std::string>
ParamsCLI::
getHighPassFilters() const
{
	return high_pass_filters;
}

std::string
ParamsCLI::
getSoundingMode() const
{
	return sounding_mode;
}

std::string
ParamsCLI::
getChannelMode() const
{
	return channel_mode;
}

std::vector<uint16_t>
ParamsCLI::
getPulseDelays() const
{
	return pulse_delays;
}

std::vector<uint16_t>
ParamsCLI::
getTimeRanges() const
{
	return time_ranges;
}

bool
ParamsCLI::
isDebugMode() const
{
	return debug_mode;
}

