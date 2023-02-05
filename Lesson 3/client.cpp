#include <algorithm>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <string>

#pragma comment (lib, "ws2_32.lib")     

#include <socket_wrapper/socket_headers.h>
#include <socket_wrapper/socket_wrapper.h>
#include <socket_wrapper/socket_class.h>


int main(int argc, char const* argv[])
{
    if (argc != 3)
    {
        std::cout << "Usage: " << argv[0] << " <host> <port>" << std::endl;
        return EXIT_FAILURE;
    }
    socket_wrapper::SocketWrapper sock_wrap;

    std::string host_name = argv[1];

    addrinfo hints =
    {
        .ai_flags = AI_CANONNAME,
        .ai_family = AF_UNSPEC,
        .ai_socktype = SOCK_STREAM,
        .ai_protocol = 0
    };


    addrinfo* servinfo = nullptr;

    if ((getaddrinfo(host_name.c_str(), nullptr, &hints, &servinfo)) != 0)      
    {
        std::cerr << sock_wrap.get_last_error_string() << std::endl;
        return EXIT_FAILURE;
    }

    int sock_4_6;
    socklen_t address_len;
    for (auto const* s = servinfo; s != nullptr; s = s->ai_next)
    {
        if (AF_INET == s->ai_family)                    // address IPv4
        {
            sock_4_6 = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
            if (!sock_4_6)
            {
                std::cerr << sock_wrap.get_last_error_string() << std::endl;
                return EXIT_FAILURE;
            }
            in_addr ip_to_num{};
            int stat = inet_pton(AF_INET, host_name.c_str(), &ip_to_num);
            if (stat <= 0)
            {
                std::cerr << sock_wrap.get_last_error_string() << std::endl;
                return EXIT_FAILURE;
            }
            sockaddr_in addr =
            {
                .sin_family = PF_INET,
                .sin_port = htons(std::stoi(argv[2])),
            };
            addr.sin_addr = ip_to_num;
            address_len = INET_ADDRSTRLEN;


            if (connect(sock_4_6, reinterpret_cast<const sockaddr*>(&addr), address_len) != 0)
            {
                std::cerr << sock_wrap.get_last_error_string() << std::endl;
                return EXIT_FAILURE;
            }
            else
                std::cout << "Connected \n";
        }
        else if (AF_INET6 == s->ai_family)                      // // address IPv6
        {
            sock_4_6 = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);
            if (!sock_4_6)
            {
                std::cerr << sock_wrap.get_last_error_string() << std::endl;
                return EXIT_FAILURE;
            }
            in6_addr ip_to_num{};
            int stat = inet_pton(AF_INET6, host_name.c_str(), &ip_to_num);
            if (stat <= 0)
            {
                std::cerr << sock_wrap.get_last_error_string() << std::endl;
                return EXIT_FAILURE;
            }
            sockaddr_in6 addr =
            {
                .sin6_family = PF_INET6,
                .sin6_port = htons(std::stoi(argv[2])),
            };
            addr.sin6_addr = ip_to_num;
            address_len = INET6_ADDRSTRLEN;


            if (connect(sock_4_6, reinterpret_cast<const sockaddr*>(&addr), address_len) != 0)
            {
                std::cerr << sock_wrap.get_last_error_string() << std::endl;
                return EXIT_FAILURE;
            }
            else
                std::cout << "Connected \n";
        }
        else
        {
            std::cout << s->ai_family << "\n";
        }
    }

    socket_wrapper::Socket sock = sock_4_6;
    ssize_t recv_len = 0;

    std::cout << "Running client...\n" 
        << "command: \n"
        << "<exit> exiting the program \n"
        << "<file> get a file"
        << std::endl;

    std::string message;
    char buffer[256];
    bool run = true;

    while (run)
    {
        std::getline(std::cin, message);
        send(sock, message.c_str(), message.size(), 0);

        std::cout << std::endl;

        if (message.find("file") == 0)      // command <file>
        {
            while (true)
            {
                recv_len = recv(sock, buffer, sizeof(buffer) - 1, 0);   // read the file line by line before "exit_file__"
                buffer[recv_len] = '\0';
                std::string str = { buffer, 0, static_cast<unsigned int>(recv_len) };
                if ("exit_file__" == str)
                {
                    break;
                }
                std::cout << str;
            }
            std::cout << std::endl;
            continue;
        }
        recv_len = recv(sock, buffer, sizeof(buffer) - 1, 0);       // server echo

        if (recv_len > 0)
        {
            buffer[recv_len] = '\0';
            std::cout << buffer << '\n';
        }

        if (message == "exit") run = false;
    }
    sock.close();
    return EXIT_SUCCESS;
}