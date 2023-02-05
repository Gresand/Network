#include <algorithm>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <string>
#include <fstream>

#pragma comment (lib, "ws2_32.lib") 

#include <socket_wrapper/socket_headers.h>
#include <socket_wrapper/socket_wrapper.h>
#include <socket_wrapper/socket_class.h>



int main(int argc, char const *argv[])
{
    if (argc != 2)
    {
        std::cout << "Usage: " << argv[0] << " <port>" << std::endl;
        return EXIT_FAILURE;
    }

    socket_wrapper::SocketWrapper sock_wrap;
    const int port { std::stoi(argv[1]) };

    socket_wrapper::Socket listenfd = {AF_INET, SOCK_STREAM, IPPROTO_TCP}; 

    std::cout << "Starting echo server on the port " << port << "...\n";

    if (!listenfd)
    {
        std::cerr << sock_wrap.get_last_error_string() << std::endl;
        return EXIT_FAILURE;
    }

    sockaddr_in addr =
    {
        .sin_family = PF_INET,
        .sin_port = htons(port),
    };

    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(listenfd, reinterpret_cast<const sockaddr*>(&addr), sizeof(addr)) != 0)
    {
        std::cerr << sock_wrap.get_last_error_string() << std::endl;
        return EXIT_FAILURE;
    }

    if (listen(listenfd, SOMAXCONN) != 0)
    {
        std::cerr << sock_wrap.get_last_error_string() << std::endl;
        return EXIT_FAILURE;
    }

    char buffer[256];


    sockaddr_in client_address = {0};
    socklen_t client_address_len = sizeof(sockaddr_in);
    ssize_t recv_len = 0;
    std::cout << "Running echo server...\n" << std::endl;
    char client_address_buf[INET_ADDRSTRLEN];


    socket_wrapper::Socket sock = accept(listenfd, reinterpret_cast<sockaddr*>(&client_address), &client_address_len);
    if (!sock)
    {
        std::cerr << sock_wrap.get_last_error_string() << std::endl;
        return EXIT_FAILURE;
    }
    listenfd.close();       
    bool run = true;

    while (run)
    {

        recv_len = recv(sock, buffer, sizeof(buffer) - 1, 0);

        if (recv_len > 0)
        {
            buffer[recv_len] = '\0';

            std::cout
                << "Client with address "
                << inet_ntop(AF_INET, &client_address.sin_addr, client_address_buf, sizeof(client_address_buf) / sizeof(client_address_buf[0]))
                << ":" << ntohs(client_address.sin_port)
                << " sent datagram "
                << "[length = "
                << recv_len
                << "]:\n'''\n"
                << buffer
                << "\n'''"
                << std::endl;

            socklen_t addrlen = INET_ADDRSTRLEN;
            char hbuf[NI_MAXHOST];
            if ((getnameinfo(reinterpret_cast<sockaddr*>(&client_address), client_address_len, hbuf, sizeof(hbuf) - 1, nullptr, 0, NI_NAMEREQD)) != 0)
            {
                std::cout << WSAGetLastError() << '\n';
                std::cerr << "could not resolve hostname" << std::endl;
            }
            else
                std::cout << hbuf << '\n';

            std::string command_string = { buffer, 0, static_cast<unsigned int>(recv_len) };

            if (command_string.find("file") == 0)
            {
                std::string file_path(command_string, 5, recv_len);
                std::cout << file_path;
                std::ifstream ifs(file_path);
                std::string line;
                if (ifs.is_open())
                {
                    while (getline(ifs, line))     
                    {
                        send(sock, line.c_str(), line.size(), 0);
                    }
                    line = "exit_file__";          
                    ifs.close();
                }
                else
                {
                    line = "error open file \n";
                    send(sock, line.c_str(), line.size(), 0);
                    line = "exit_file__";
                }
                send(sock, line.c_str(), line.size(), 0);       
                continue;
            }


            send(sock, buffer, recv_len, 0);
            if ("exit" == command_string) run = false;
        }

        std::cout << std::endl;
    }
    sock.close();
    return EXIT_SUCCESS;
}