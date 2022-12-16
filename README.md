# zond-reader
Utility to read Zond Aero 1000 data and write it to files

## How to build

### Requirements

To build and run the zond reader you'll need OS Ubuntu 20.04 or 22.04 for x86-64 architecture.
These dependancies are expected to be at building platform: git, make, cmake, gcc, g++, pkg-config, libopencv-dev, libasio-dev

You may install them with command: sudo apt install git make cmake gcc g++ pkg-config libopencv-dev libasio-dev

### Binary compilation

First, make a copy of source repository:

git clone git@github.com:ugcs/zond-reader.git

Now make a build directory inside the source tree:

cd zond-reader/src/zond-reader/
mkdir -p build
cd build

Next step is to configure the project with cmake:

cmake -DCMAKE_BUILD_TYPE=Release ..

And make it:

make

After build complete, the result binary will be: ./bin/zond-reader

## How to run

If you run ./bin/zond-reader without any parameters, it will do two things:

1. Make a file reader.conf with a default settings
1. Try to connect Zond Aero sensor with default settings

Zond aero reader may take any settings from the command line. You may take a help message on avalable keys:

./bin/zond-reader --help

You may use different config files wih --config key to point file location:

./bin/zond-reader --config ./my_reader.conf

You may edit configuration file with any text editor. It's a simple TOML format.
If you supply any configuration parameters, utility will update them in a configuration file also.

The results will be written as a series of JPEG files to the output directory. Key "-o" may be used to specify files location.
By default it's the current directory, where zond reader utility was called from.

The number of traces per JPEG file is controlled with -w (Width) key. It's 500 traces by default.

The utulity will read traces from the sensor and write them into files. It stops the processing then user press the ENTER key.
 
