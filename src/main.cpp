/*
	All functions are thread-safe based on the id that is passed to the function.
	No function will be called with the same id at the same time.

	Multiple calls to the same function _will_ occur. A mutex must be used to make vectors/maps/lists thread-safe.
*/

#include <stdint.h>
#include <string>
#include <iostream>
#include <iomanip>
#include <mutex>
#include <unordered_map>
#include <unordered_set>

#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/filesystem.hpp>
#include <boost/chrono.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/tokenizer.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/tokenizer.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string.hpp>

#include <curl/curl.h>

#include "stream_utility.h"
#include "base_plugin.h"

extern boost::filesystem::path executable_path();

// Connection type that gets passed to every function
namespace ConnectionType
{
	enum T : int
	{
		Download = 0,
		Gateway,
		Agent
	};
}

struct skill
{
	int Action_PreparingTime;
	int Action_CastingTime;
	int Action_ActionDuration;
	int Action_ReuseDelay;
};

struct item
{
	int tid1, tid2, tid3;
};

class plugin : public base_plugin
{
private:

	// asio
	boost::asio::io_service & io_service;

	std::mutex m;

	std::unordered_map<uint32_t, skill> skill_data;
	std::unordered_map<uint32_t, item> item_data;

	static size_t write_callback(void *contents, size_t size, size_t nmemb, std::string *userp)
	{
		size_t realsize = size * nmemb;
		userp->append(std::string((const char*)contents, realsize));
		return realsize;
	}

public:

	// ctor
	plugin(int argc, char *argv[], boost::asio::io_service & io_service_) : base_plugin(argc, argv, io_service_), io_service(io_service_)
	{
		curl_global_init(CURL_GLOBAL_ALL);

		namespace po = boost::program_options;
		po::options_description desc;
		desc.add_options()
			("skill-data-url", po::value<std::string>(), "Skill data URL")
			("item-data-url", po::value<std::string>(), "Item data URL");

		po::variables_map vm;
		po::store(po::command_line_parser(argc, argv).options(desc).allow_unregistered().run(), vm);
		po::notify(vm);

		if(vm.count("skill-data-url"))
		{
			const std::string url = vm["skill-data-url"].as<std::string>();
			std::string text;

			CURL* curl = curl_easy_init();

			curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&text);
			curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);

			curl_easy_perform(curl);

			long http_code = 0;
			curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

			curl_easy_cleanup(curl);

			if(http_code == 200 && !text.empty())
			{
				load_skills(text);
			}
		}

		if (vm.count("item-data-url"))
		{
			const std::string url = vm["item-data-url"].as<std::string>();
			std::string text;

			CURL* curl = curl_easy_init();

			curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&text);
			curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);

			curl_easy_perform(curl);

			long http_code = 0;
			curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

			curl_easy_cleanup(curl);

			if (http_code == 200 && !text.empty())
			{
				load_items(text);
			}
		}

		curl_global_cleanup();
	}

	// Library is being unloaded
	// Clean up any pointers, threads, etc.
	~plugin()
	{
	}

	void load_skills(const std::string & text, const std::string & new_line = "\n")
	{
		const boost::char_separator<char> separator_newline(new_line.c_str());

		boost::tokenizer<boost::char_separator<char> > lines(text, separator_newline);

		for (auto itr = lines.begin(); itr != lines.end(); ++itr)
		{
			std::vector<std::string> tokens;
			boost::split(tokens, *itr, boost::is_any_of(","));

			if (tokens.size() >= 5)
			{
				skill s;
				s.Action_PreparingTime = boost::lexical_cast<int>(tokens[1]);
				s.Action_CastingTime = boost::lexical_cast<int>(tokens[2]);
				s.Action_ActionDuration = boost::lexical_cast<int>(tokens[3]);
				s.Action_ReuseDelay = boost::lexical_cast<int>(tokens[4]);
				skill_data[boost::lexical_cast<uint32_t>(tokens[0])] = s;
			}
		}
	}

	void load_items(const std::string & text, const std::string & new_line = "\n")
	{
		const boost::char_separator<char> separator_newline(new_line.c_str());

		boost::tokenizer<boost::char_separator<char> > lines(text, separator_newline);

		for (auto itr = lines.begin(); itr != lines.end(); ++itr)
		{
			std::vector<std::string> tokens;
			boost::split(tokens, *itr, boost::is_any_of(","));

			if (tokens.size() >= 4)
			{
				item i;
				i.tid1 = boost::lexical_cast<int>(tokens[1]);
				i.tid2 = boost::lexical_cast<int>(tokens[2]);
				i.tid3 = boost::lexical_cast<int>(tokens[3]);
				item_data[boost::lexical_cast<uint32_t>(tokens[0])] = i;
			}
		}
	}

	void send_notice(size_t id, const std::string & message)
	{
		StreamUtility w;
		w.Write<uint8_t>(7);
		w.Write<uint16_t>(message.size());
		w.Write_Ascii(message);
		inject_client(id, 0x3026, w, false, false);
	}

	// Server to Client packets
	//
	// type - Connection type
	// id - Identifier for the client/server pair
	// opcode - Silkroad game opcode
	// stream - Packet data
	bool server_to_client(int type, size_t id, uint16_t opcode, StreamUtility & stream, bool encrypted, bool massive)
	{
		if (type == ConnectionType::Agent)
		{
		}

		return true;	// Keep the packet
	}

	// Client to Server packets
	//
	// type - Connection type
	// id - Identifier for the client/server pair
	// opcode - Silkroad game opcode
	// stream - Packet data
	bool client_to_server(int type, size_t id, uint16_t opcode, StreamUtility & stream, bool encrypted, bool massive)
	{
		if (type == ConnectionType::Agent)
		{
		}

		return true;	// Keep the packet
	}

	// New connection started
	// This function will be called before any data has been received
	// Packets should not be injected from here
	//
	// type - Connection type
	// id - Identifier for the client/server pair
	// ip - IPv4 or IPv6 address
	// port - Connection port
	bool new_connection(int type, size_t id, const std::string & ip, uint16_t port)
	{
		// Return 'true' to allow the connection
		// Return 'false' to disconnect
		return true;
	}

	// Connection closed
	// Packets should not be injected here or after
	//
	// type - Connection type
	// id - Identifier for the client/server pair
	// ip - IPv4 or IPv6 address
	// port - Connection port
	void connection_closed(int type, size_t id, const std::string & ip, uint16_t port)
	{
		if (type == ConnectionType::Agent)
		{
		}
	}
};

// Library initialization function
// - This function will be called immediately upon loading
extern "C" boost::shared_ptr<plugin> plugin_initialize(int argc, char *argv[], boost::asio::io_service & io_service)
{
	// Create an instance of the plugin class
	return boost::make_shared<plugin>(argc, argv, io_service);
}

// Returns the plugin name
extern "C" const char* plugin_name()
{
	return "example";
}

// Returns the plugin version
extern "C" const char* plugin_version()
{
	return "1.0.0";
}