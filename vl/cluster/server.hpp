/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2011-01
 *	@file server.hpp
 */

#ifndef VL_CLUSTER_SERVER_HPP
#define VL_CLUSTER_SERVER_HPP

#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>

#include "message.hpp"
#include "states.hpp"

#include "stats.hpp"

#include "logger.hpp"

namespace boost
{
	using boost::asio::ip::udp;
}

namespace vl
{

namespace cluster
{

class Server : public LogReceiver
{
public:

	struct ClientInfo
	{
		ClientInfo( boost::udp::endpoint p, CLIENT_STATE s = CS_UNDEFINED, uint32_t f = 0 )
			: address(p), state(s), frame(f)
		{}

		boost::udp::endpoint address;
		CLIENT_STATE state;
		uint32_t frame;
	};

	Server( uint16_t const port );

	~Server();

	void receiveMessages( void );

	/// Synchronious method that blocks till all the clients have done
	/// update, draw and swap
	void render( vl::Stats &stats );

	// TODO this should block till all the clients have shutdown
	void shutdown( void );

	// New message interface
	void sendMessage(Message const &msg);

	/// Store the Environment message for further use
	void sendEnvironment( Message const &msg );

	/// Store the project message for further use
	// Can be updated by a another call and will be sent again to all clients
	void sendProject( Message const &msg );

	/// Send an Initial SceneGraph
	void sendInit( Message const &msg );

	/// Send an SceneGraph update
	void sendUpdate( Message const &msg );

	/// Send information on new SceneGraph elements created
	void sendCreate( Message const &msg );

	/// @brief Has the Server unprocessed Input Messages.
	/// @return true if the server has input messages, false otherwise
	/// @todo change to use functors/callbacks rather than this has + get
	/// method
	bool messages( void ) const
	{ return !_messages.empty(); }

	/**	@brief receive an input message from a stack.
	 *	@todo same as above replace with callbacks
	 *	All messages are dynamically allocated so the user needs to delete
	 *	the message after use.
	 */
	vl::cluster::Message popMessage(void);

	typedef std::vector<ClientInfo> ClientList;

	/// LogReceiver overrides
	virtual bool logEnabled(void) const;

	virtual void logMessage(LogMessage const &msg);

	virtual uint32_t nLoggedMessages(void) const;

private :
	void _addClient( boost::udp::endpoint const &endpoint );

	void _sendProject( boost::udp::endpoint const &endpoint );

	void _sendEnvironment( std::vector<char> const &msg );

	void _sendEnvironment( boost::udp::endpoint const &endpoint );

	void _sendCreate( ClientInfo const &client );

	void _sendUpdate( ClientInfo const &client );

	void _sendDraw( boost::udp::endpoint const &endpoint );

	void _sendSwap( boost::udp::endpoint const &endpoint );

	void _sendOuput(boost::udp::endpoint const &endpoint);

	void _sendMessage(boost::udp::endpoint const &endpoint, vl::cluster::Message const &msg);

	void _handleAck( boost::udp::endpoint const &client, MSG_TYPES ack_to );

	void _waitCreate( void );

	/// Returns when all the clients are ready for an update message
	void _waitUpdate( void );

	/// Returns when all the clients are ready for an draw message
	void _waitDraw( void );

	/// Returns when all the clients are ready for an swap message
	void _waitSwap( void );

	boost::asio::io_service _io_service;
	boost::udp::socket _socket;

	ClientList _clients;

	std::deque<Message> _messages;

	/// Messages stored for when new clients connect
	/// Environment message, set in the start
	std::vector<char> _env_msg;
	/// Project message, the latest one set
	std::vector<char> _proj_msg;
	/// Init message, the latest one set
	std::vector<char> _msg_init;
	/// Last update message? Might need a whole vector of these
	std::vector<char> _msg_update;

	/// Create MSGs
	std::vector< std::pair<uint32_t, Message> > _msg_creates;

	ClientList _output_receivers;

	uint32_t _frame;

	uint32_t _n_log_messages;
	bool _receiving_log_messages;
	std::vector<vl::LogMessage> _new_log_messages;

};	// class Server

}	// namespace cluster

}	// namespace vl

#endif // VL_CLUSTER_SERVER_HPP
