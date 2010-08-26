/*	Joonatan Kuosa
 *	2010-08
 *	Test program that contains both udp::Server and udp::Client
 */

// Standard headers
#include <iostream>

// UNIX headers
#include <time.h>

#include "udp/server.hpp"
#include "udp/client.hpp"
#include "udp/print_command.hpp"

char const *HOST = "localhost";
char const *PORT_STR = "2244";
uint16_t const PORT = 2244;

int main(int argc, char **argv)
{
	try
	{
		std::cout << "Starting UDP server on port " << PORT << std::endl;

		vl::udp::Server server( PORT );

		boost::shared_ptr<vl::udp::Command> cmd( new vl::udp::PrintCommand("setPosition", "feet" ) );
		server.addCommand( cmd );
		cmd.reset( new vl::udp::PrintCommand("setPosition", "feet" ) );
		server.addCommand( cmd );
		cmd.reset( new vl::udp::PrintCommand("setQuaternion", "feet" ) );
		server.addCommand( cmd );

		std::cout << "Starting UDP client" << std::endl;

		std::cout << "Client connecting to host : " << HOST << " "
			<< "using port = " << PORT << std::endl;
		vl::udp::Client client( HOST, PORT_STR );

		std::cout << "Client created" << std::endl;

		for( size_t j = 0; j < 3; ++j )
		{
			std::vector<double> test_vec(9+j);
			for( size_t i = 0; i < test_vec.size(); ++i )
			{
				test_vec.at(i) = i;
			}

			client.send( test_vec );

			std::cout << "Message sent" << std::endl;

			// Sleep
			// TODO crossplatform sleep
			timespec tv;
			tv.tv_sec = 0;
			tv.tv_nsec = 1e3;
			::nanosleep( &tv, 0 );

			server.mainloop();
		}
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << std::endl;
	}

	return 0;
}
