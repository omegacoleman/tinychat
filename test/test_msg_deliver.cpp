#define BOOST_TEST_MODULE "MessageDeliverCS"
#include "boost/test/included/unit_test.hpp"
#include "boost/process.hpp"
#include <iostream>
#include <chrono>
#include <thread>

const unsigned short testing_port = 7777; // This port should not be taken by other programs

struct test_child
{
	test_child(std::string command)
		:child(command,
			boost::process::std_out > this->std_out,
			boost::process::std_err > this->std_err,
			boost::process::std_in < this->std_in
		)
	{
	}

	~test_child()
	{
		child.terminate();
	}

	boost::process::opstream std_in;
	boost::process::ipstream std_out;
	boost::process::ipstream std_err;
	boost::process::child child;
};

struct test_client : public test_child
{
	test_client(std::string user, std::string auth)
		:test_child("../websocket-client --server 127.0.0.1 --port " + std::to_string(testing_port) + " --user " + user + " --auth " + auth)
	{
	}
};

struct test_server : public test_child
{
	test_server()
		:test_child("../websocket-server --db-dummy --port " + std::to_string(testing_port))
	{
	}
};

static const std::pair<std::string, std::string> test_user_auths[] = {
	{"banana", "1"},
	{"papaya", "2"},
	{"cocoa", "3"},
	{"jujube", "4"}
};

bool wait_output(const std::string &hinter, boost::process::child &child_process, boost::process::ipstream &pipe, const std::string &needle)
{
	bool ok = false;
	std::string line;
	while (child_process.running() && std::getline(pipe, line) && !line.empty())
	{
		std::cout << "(" << hinter << "): " << line << std::endl;
		if (line.find(needle) != std::string::npos)
		{
			ok = true;
			break;
		}
	}
	return ok;
}

template <int client_count, 
	typename std::enable_if<((client_count > 0) && (client_count <= (std::end(test_user_auths) - std::begin(test_user_auths)))), int>::type = 0
			> struct multi_client_fixture
{
	multi_client_fixture()
		:server(std::make_unique<test_server>())
	{
		BOOST_REQUIRE(wait_output("SERVER", server->child, server->std_err, "listening on"));
		for (int i = 0; i < client_count; i++)
		{
			clients[i] = std::make_unique<test_client>(test_user_auths[i].first, test_user_auths[i].second);
			BOOST_REQUIRE(wait_output("CLIENT#" + std::to_string(i), clients[i]->child, clients[i]->std_err, "prompting started"));
		}
	}

	std::unique_ptr<test_server> server;
	std::unique_ptr<test_client> clients[client_count];
};

BOOST_FIXTURE_TEST_CASE(between_clients, multi_client_fixture<2>, *boost::unit_test::timeout(30))
{
	std::string message = "Hello, world!";

	// Send via one end
	clients[0]->std_in << message << std::endl;
	// Wait the request to be completed
	BOOST_REQUIRE(wait_output("CLIENT#0", clients[0]->child, clients[0]->std_err, "message sent"));

	// The other end shall be able to recieve it
	BOOST_REQUIRE(wait_output("CLIENT#1", clients[1]->child, clients[1]->std_out, message));
}

BOOST_FIXTURE_TEST_CASE(broadcasting, multi_client_fixture<3>, *boost::unit_test::timeout(30))
{
	std::string message = "Hello, world!";

	// Send via one end
	clients[0]->std_in << message << std::endl;
	// Wait the request to be completed
	BOOST_REQUIRE(wait_output("CLIENT#0", clients[0]->child, clients[0]->std_err, "message sent"));

	// Both two other ends shall be able to recieve it
	BOOST_REQUIRE(wait_output("CLIENT#1", clients[1]->child, clients[1]->std_out, message));
	BOOST_REQUIRE(wait_output("CLIENT#2", clients[2]->child, clients[2]->std_out, message));
}
