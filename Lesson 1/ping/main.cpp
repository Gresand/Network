#include <stdio.h>
#include <iostream>
#include <winsock2.h>
#include <chrono>

#include <socket_wrapper/socket_headers.h>
#include <socket_wrapper/socket_wrapper.h>
#include <socket_wrapper/socket_class.h>


bool ping(std::string &host_name)
{
    socket_wrapper::SocketWrapper sock_wrap;

	socket_wrapper::Socket sock = {AF_INET, SOCK_RAW, IPPROTO_ICMP};
	
	if (!sock)
    {
        std::cerr << sock_wrap.get_last_error_string() << std::endl;
        return EXIT_FAILURE;
    }
}

int main(int argc, char const *argv[])
{
    if (argc != 2)
    {
        std::cout << "Usage: " << argv[0] << " <host name>" << std::endl;
        return EXIT_FAILURE;
    }

    std::string host_name = argv[1];

    ping(host_name);


	return EXIT_SUCCESS;
}