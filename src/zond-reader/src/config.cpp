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

	app.add_option("-c, --sample-per-trace", sample_count, "Samples per trace")
			->default_val(512);

	pulse_delay = 174;
	app.add_option("-d, --pulse-delay", pulse_delay, "Pulse delay")
			->default_val(pulse_delay);

	time_range = 7;
	app.add_option("-t, --time-range-per-sample", time_range, "Time range per sample")
			->default_val(time_range)
			->expected(0,12);

	tx_freq = 300;
	app.add_option("-f, --tx-freq", tx_freq, "Transmitter frequency")
			->default_val(tx_freq);

	stacking = 512;
	app.add_option("-s, --stacking", stacking, "Stacking")
			->default_val(stacking);

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

uint16_t
ParamsCLI::
getPulseDelay() const
{
	return pulse_delay;
}

uint16_t
ParamsCLI::
getTimeRange() const
{
	return time_range;
}

uint16_t
ParamsCLI::
getStacking() const
{
	return stacking;
}

uint16_t
ParamsCLI::
getTxFrequency() const
{
	return tx_freq;
}

