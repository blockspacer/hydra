/**
 *	Copyright (c) 2011 Tampere University of Technology
 *	Copyright (c) 2011 - 2014 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-03
 *	@file logger.hpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.5
 *
 *	Licensed under commercial license.
 *
 */

#ifndef HYDRA_LOGGER_HPP
#define HYDRA_LOGGER_HPP

#include <streambuf>
#include <sstream>
#include <fstream>

#include <iosfwd>							// streamsize
#include <boost/iostreams/categories.hpp>	// sink_tag
#include <boost/iostreams/concepts.hpp>		// sink
#include <boost/iostreams/stream.hpp>

#include <vector>

// Necessary for HYDRA_API
#include "defines.hpp"

namespace io = boost::iostreams;

namespace vl
{

enum LOG_MESSAGE_LEVEL
{
	LML_CRITICAL = 0,
	LML_NORMAL,
	LML_TRIVIAL,
};

char const CRITICAL = 17;
char const NORMAL = 18;
char const TRACE = 19;

struct HYDRA_API LogMessage
{
	LogMessage( std::string const &ty = std::string(),
				double tim = 0,
				std::string const &msg = std::string(),
				LOG_MESSAGE_LEVEL lvl = LML_CRITICAL );

	void append(std::string const &str);

	void clear(void);

	bool empty(void) const;

// Type denotes the message origin i.e. which stream it was written into
// for example ERROR for std::cerr, OUT for std::out 
// and PY_OUT for python cout, PY_ERROR for python cerr
std::string type;
// Time the message was logged, can be 0 when the time is not tracked
double time;
// The log message
std::string message;
// Optional log level defaults to LML_NORMAL
LOG_MESSAGE_LEVEL level;

};	// class LogMessage

inline std::ostream &
operator<<(std::ostream &os, LogMessage const &msg)
{
	std::string level;
	if( msg.level == LML_CRITICAL )
	{ level = "CRITICAL"; }
	else if( msg.level == LML_NORMAL )
	{ level = "NORMAL"; }
	else if( msg.level == LML_TRIVIAL )
	{ level = "TRIVIAL"; }

	os << msg.type << "\t" << level << '\t' << msg.time << "s " << msg.message;
	return os;
}

/// Abstract class which defines the interface for sending LoggedMessages
class LogReceiver
{
public :
	virtual ~LogReceiver(void) {}

	virtual bool logEnabled(void) const = 0;

	virtual void logMessage(LogMessage const &msg) = 0;

	virtual uint32_t nLoggedMessages(void) const = 0;

};

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

class HYDRA_API Logger
{
public :
	Logger( void );

	~Logger( void );

	// TODO not used
	void setVerbose( bool v )
	{ _verbose = v; }

	bool getVerbose( void ) const
	{ return _verbose; }

	void setOutputFile( std::string const &filename );

	io::stream_buffer<sink> *addSink(std::string const &name);

	io::stream_buffer<sink> *getSink(std::string const &name);

	io::stream_buffer<sink> *getPythonOut( void );

	io::stream_buffer<sink> *getPythonErr( void );

	void logMessage( std::string const &type, std::string const &message, LOG_MESSAGE_LEVEL level = LML_CRITICAL );

	void logMessage(LogMessage const &message);

	size_t nMessages(void) const
	{ return _messages.size(); }

	LogMessage const &getMessage(size_t i) const;

/// Data
private :

	bool _verbose;

	// old streambuffers so we can restore them later
	std::streambuf *_old_cout;
	std::streambuf *_old_cerr;

	std::string _output_filename;
	std::ofstream _output_file;

	std::vector<LogMessage> _messages;
	std::vector< io::stream_buffer<sink> *> _streams;

	LogMessage _current_msg;
};	// namespace Logger

}	// namespace vl

#endif	// HYDRA_LOGGER_HPP
