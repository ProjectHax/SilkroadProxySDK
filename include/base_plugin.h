#pragma once

#include <stdint.h>
#include <string>

#include <boost/asio.hpp>

#include "stream_utility.h"

class base_plugin
{
public:

	// ctor
	base_plugin(int argc, char *argv[], boost::asio::io_service & io_service_);

	// Library is being unloaded
	// Clean up any pointers, threads, etc.
	virtual ~base_plugin();

	// Server to Client packets
	//
	// type - Connection type
	// id - Identifier for the client/server pair
	// opcode - Silkroad game opcode
	// stream - Packet data
	virtual bool server_to_client(int type, size_t id, uint16_t opcode, StreamUtility & stream, bool encrypted, bool massive);

	// Client to Server packets
	//
	// type - Connection type
	// id - Identifier for the client/server pair
	// opcode - Silkroad game opcode
	// stream - Packet data
	virtual bool client_to_server(int type, size_t id, uint16_t opcode, StreamUtility & stream, bool encrypted, bool massive);

	// New connection started
	// This function will be called before any data has been received
	// Packets should not be injected from here
	//
	// type - Connection type
	// id - Identifier for the client/server pair
	// ip - IPv4 or IPv6 address
	// port - Connection port
	virtual bool new_connection(int type, size_t id, const std::string & ip, uint16_t port);

	// Connection closed
	// Packets should not be injected here or after
	//
	// type - Connection type
	// id - Identifier for the client/server pair
	// ip - IPv4 or IPv6 address
	// port - Connection port
	virtual void connection_closed(int type, size_t id, const std::string & ip, uint16_t port);

	// Returns the number of active connections
	size_t connection_count();

	// Packet injection functions
	bool inject_client(size_t id, uint16_t opcode, StreamUtility & stream, bool encrypted, bool massive);
	bool inject_server(size_t id, uint16_t opcode, StreamUtility & stream, bool encrypted, bool massive);

	// Closes a connection
	void disconnect(size_t id);

	// Returns the HWID for the connection ID
	std::vector<uint8_t> get_hwid(size_t id);

	// Returns the IP of the user
	std::string get_ip(size_t id);

	// Returns the username of the user
	std::string get_username(size_t id);
};