/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2011-06
 *	@file nv_swap_sync.cpp
 *
 *	This file is part of Hydra a VR game engine tests.
 *
 *	Test the usage of serial port.
 *
 *	Works only on Windows.
 *
 */

#ifdef _WIN32
#include <Windows.h>
#endif

#include <iostream>

#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>

const char *DEFAULT_COM_PORT = "COM1";

#include "input/serial_joystick.hpp"

namespace po 
{
	using namespace boost::program_options;
}

struct options
{
	options(void)
		: com_port(DEFAULT_COM_PORT)
	{}

	bool parse(int argc, char **argv)
	{
		// Declare the supported options.
		po::options_description desc("Allowed options");
		desc.add_options()
			("help", "produce help message")
			("input-file,I", po::value<std::string>(), "Input file for the values to send.")
			("com", po::value<std::string>(), "Serial port to use.")
		;

		po::positional_options_description p;
		p.add("com", -1);

		po::variables_map vm;
		po::store(po::command_line_parser(argc, argv).
				  options(desc).positional(p).run(), vm);
		po::notify(vm);

		if( vm.count("help") )
		{
			std::cout << desc << std::endl;
			return false;
		}

		if( vm.count("com") )
		{
			com_port = vm["com"].as<std::string>();
			std::cout << "COM port was set to " << com_port << "." << std::endl;
		}
		if( vm.count("input-file") )
		{
			input_file = vm["input-file"].as<std::string>();
			std::cout << "Input file was set to " << input_file << "." << std::endl;
			if( !fs::exists(input_file) )
			{
				std::cout << "No " << input_file << " found." << std::endl;
				return false;
			}
		}

		return true;
	}

	std::string com_port;
	// Not yet supported
	std::string input_file;
};

void joystickValueChanged(vl::JoystickEvent const &evt)
{
	std::cout << "New joystick values : " << evt << std::endl;
}

int main(int argc, char **argv)
{
	try {
		options opt;
		opt.parse(argc, argv);

		vl::SerialJoystick joy(opt.com_port);
		joy.addListener(&joystickValueChanged);
		// Read the response
		while(true)
		{
			joy.mainloop();

			Sleep(100);
		}
	}
	catch(std::string const &e)
	{
		std::cerr << "Exception : " << e << " thrown." << std::endl;
	}
	catch(...)
	{
		std::cerr << "Exception thrown." << std::endl;
	}

	return 0;
}
