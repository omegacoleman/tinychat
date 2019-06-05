#define BOOST_TEST_MODULE "MessageDeliverCS"
#include "boost/test/included/unit_test.hpp"
#include "boost/process.hpp"
#include <iostream>
#include <chrono>
#include <thread>

const unsigned short testing_port = 7777; // This port should not be taken by other programs

BOOST_AUTO_TEST_CASE(between_clients, *boost::unit_test::timeout(30))
{
	boost::process::ipstream client_a_cout;
	boost::process::ipstream client_b_cout;
	boost::process::opstream client_a_cin;
	boost::process::opstream client_b_cin;
	boost::process::ipstream server_cout;
	boost::process::ipstream server_cerr;

	boost::process::child c_server(
		std::string("../websocket-server --db-dummy --port " + std::to_string(testing_port)),
		boost::process::std_out > server_cout,
		boost::process::std_err > server_cerr
	);
	std::string line;
	while (c_server.running() && std::getline(server_cerr, line) && !line.empty())
	{
		if (line.find("listening on") != std::string::npos)
		{
			break;
		}
	}

	boost::process::child c_client_a(
		std::string("../websocket-client --server 127.0.0.1 --port " + std::to_string(testing_port) + " --user banana --auth 1"),
		boost::process::std_out > client_a_cout,
		boost::process::std_in < client_a_cin
	);
	boost::process::child c_client_b(
		std::string("../websocket-client --server 127.0.0.1 --port " + std::to_string(testing_port) + " --user papaya --auth 2"), 
		boost::process::std_out > client_b_cout,
		boost::process::std_in < client_b_cin
	);

	std::this_thread::sleep_for(std::chrono::seconds(3));

	std::string message = "Hello, world!";
	client_a_cin << message << std::endl;

	while (c_client_b.running() && std::getline(client_b_cout, line) && !line.empty())
	{
		if (line.find(message) != std::string::npos)
		{
			break;
		}
	}
}
