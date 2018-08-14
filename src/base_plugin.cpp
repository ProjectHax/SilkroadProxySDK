#include "base_plugin.h"

base_plugin::base_plugin(int argc, char *argv[], boost::asio::io_service & io_service_)
{
}

base_plugin::~base_plugin()
{
}

bool base_plugin::server_to_client(int type, size_t id, uint16_t opcode, StreamUtility & stream, bool encrypted, bool massive)
{
	return true;	// Keep the packet
}

bool base_plugin::client_to_server(int type, size_t id, uint16_t opcode, StreamUtility & stream, bool encrypted, bool massive)
{
	return true;	// Keep the packet
}

bool base_plugin::new_connection(int type, size_t id, const std::string & ip, uint16_t port)
{
	// Return 'true' to allow the connection
	// Return 'false' to disconnect
	return true;
}

void base_plugin::connection_closed(int type, size_t id, const std::string & ip, uint16_t port)
{
}