#include <config.h>
#include <iostream>
#include <vector>
#include <asio.hpp>
#include <reader.h>
#include <writer.h>
#include <worker.h>
#include <radsyszondgpr.h>


//Main entry point for application
int main(int argc, char **argv) {
	try {
		//Parse the command line parameters:
		ParamsCLI params(argc, argv);

		//Create ASIO context for async operations:
		asio::io_context context;

		//Create thread pool of 2 threads: one to read data from sensor
		// and the second to write it to JPEG files
		std::vector<WorkerThread> workers;
		for(int i = 0; i < 2; i++)
			workers.emplace_back(context);

		//Make an instance of JPEG writer
		GprWriter writer(context, params);
		//Make an instance of sensor reader.
		//It takes a reference on the writer to tell it when to write
		SensorReader sensor(context, writer, params);

		//Run all working threads
		for(auto & worker : workers)
			worker.start();

		//Submit task to start sensor reading
		asio::post(context, [&] {
			std::cout << "Start processing" << std::endl;
			sensor.start();
		});

		//Main thread waits for ENTER key to stop processing:
		std::cout << "Press ENTER to stop..." << std::endl;
		std::cin.ignore();

		//Stop ASIO loop for async operations:
		context.stop();
		//Wait for all processing threads to complete their job:
		for(int i = 0; i < 2; i++) {
			workers[i].stop();
		}
		sensor.stop();
		//Farewell and good luck:
		std::cout << "Work complete. Reader is out." << std::endl;

	}
	catch(std::invalid_argument &e) {
		std::cerr << "Incorrect argument: " << e.what() << std::endl;
		return -1;
	}
	catch(std::exception &e) {
		std::cerr << e.what() << std::endl;
		return -1;
	}

	return 0;
}
