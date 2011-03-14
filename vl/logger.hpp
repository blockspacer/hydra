/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2011-03
 *	@file logger.hpp
 *
 */

#ifndef VL_LOGGER_HPP
#define VL_LOGGER_HPP

#include <streambuf>
#include <sstream>
#include <fstream>

#include <iosfwd>							// streamsize
#include <boost/iostreams/categories.hpp>	// sink_tag
#include <boost/iostreams/concepts.hpp>		// sink
#include <boost/iostreams/stream.hpp>

#include <vector>

#include "base/thread.hpp"

namespace io = boost::iostreams;

namespace vl
{

enum LOG_MESSAGE_LEVEL
{
	LML_CRITICAL = 0,
	LML_NORMAL,
	LML_TRIVIAL,
};

char const ERROR = 17;
char const NORMAL = 18;
char const TRACE = 19;

struct LogMessage
{
LogMessage( std::string const &ty = std::string(),
			double tim = 0,
			std::string const &msg = std::string(),
			LOG_MESSAGE_LEVEL level = LML_CRITICAL )
	: type(ty), time(tim), message(msg)
{
	// Remove the line ending because we are using custom line endings
	if(*(message.end()-1) == '\n')
	{ message.erase(message.end()-1); }
}

std::string type;
double time;
std::string message;

};	// class LogMessage

inline std::ostream &
operator<<(std::ostream &os, LogMessage const &msg)
{
	os << msg.type << " " << msg.time << "s " << msg.message;
	return os;
}

class Logger;

class sink : public boost::iostreams::sink
{
public :
	sink( Logger &logger, std::string const &type );

	std::streamsize write(const char* s, std::streamsize n);

	void write(std::string const &str);

	std::string getType( void ) const
	{ return _type; }

	Logger &getLogger( void )
	{ return _logger; }

private :
	Logger &_logger;
	std::string _type;
};

class Logger
{
public :
	Logger( void );

	~Logger( void );

	// TODO not used
	void setVerbose( bool v )
	{ _verbose = v; }

	bool getVerbose( void ) const
	{ return _verbose; }

	void setOutputFile( std::string const &filename )
	{
		_output_filename = filename;
		// TODO should close the old file if it's open
	}

	io::stream_buffer<sink> *addSink(std::string const &name);

	io::stream_buffer<sink> *getSink(std::string const &name);

	io::stream_buffer<sink> *getPythonOut( void );

	io::stream_buffer<sink> *getPythonErr( void );

	void logMessage( std::string const &type, std::string const &message, LOG_MESSAGE_LEVEL level = LML_CRITICAL );

	void logMessage(LogMessage const &message);

	bool newMessages(void) const
	{ return !_messages.empty(); }

	size_t nMessages(void) const
	{ return _messages.size(); }

	LogMessage popMessage(void);

	// TODO separate cout and cerr for python

	// TODO logging data to both a file and an endpoint

/// Data
private :
	vl::mutex _mutex;

	bool _verbose;

	// old streambuffers so we can restore them later
	std::streambuf *_old_cout;
	std::streambuf *_old_cerr;

	std::string _output_filename;
	std::ofstream _output_file;

	std::vector<LogMessage> _messages;
	std::vector< io::stream_buffer<sink> *> _streams;

};	// namespace Logger

}	// namespace vl

#endif	// VL_LOGGER_HPP
