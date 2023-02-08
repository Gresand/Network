#include <iostream>
#include <string>
#include <sstream>
#include <cstdlib>
#include <fstream>
#include <map>
#include <functional>


#pragma comment (lib, "ws2_32.lib")
#include <socket_wrapper/socket_headers.h>
#include <socket_wrapper/socket_wrapper.h>
#include <socket_wrapper/socket_class.h>

extern "C"
{
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/x509.h>
}


class server_ssl
{
private:
    static const int buff_size = 1024;
    int port;
    bool autoriz = false;

    socket_wrapper::SocketWrapper wrap;

    std::unordered_map<std::string, size_t> pass; 
    std::string message;                              

    SSL_CTX* ctx = nullptr;
    SSL* ssl = nullptr;

public:
    server_ssl(int iport) : port(iport)
    {
        message.resize(buff_size);

    }
    ~server_ssl()
    {
        SSL_CTX_free(ctx);
        SSL_free(ssl);
        SSL_shutdown(ssl);
    }
    void init_ssl() 
    {
        OpenSSL_add_all_algorithms();
        SSL_load_error_strings();
        SSL_library_init();
        const SSL_METHOD* method = TLSv1_2_server_method();
        ctx = SSL_CTX_new(method);
        if (ctx == NULL)
        {
            ERR_print_errors_fp(stderr);
            return;
        }
        ssl = SSL_new(ctx);
        if (!ssl)
        {
            std::cerr << "error creating SSL." << std::endl;
            return;
        }
    }

    bool recv_packet()         
    {
        message = "";
        int len = 0;
        char buffer[buff_size];
        do
        {
            len = SSL_read(ssl, buffer, buff_size - 1);
            buffer[len] = 0;
            message += buffer;
        } while (len > 0);
        if (len < 0)
        {
            switch (SSL_get_error(ssl, len))
            {
                
            case SSL_ERROR_WANT_READ:
            case SSL_ERROR_WANT_WRITE:
                return true;
                break;
            case SSL_ERROR_ZERO_RETURN:
            case SSL_ERROR_SYSCALL:
            case SSL_ERROR_SSL:
                return false;
            }
        }

        return true;
    }

    bool send_packet(std::string& buf)
    {
        int len = SSL_write(ssl, buf.c_str(), buf.size());
        if (len < 0)
        {
            int err = SSL_get_error(ssl, len);
            switch (err)
            {
            case SSL_ERROR_WANT_WRITE:
            case SSL_ERROR_WANT_READ:
                return true;
                break;
            case SSL_ERROR_ZERO_RETURN:
            case SSL_ERROR_SYSCALL:
            case SSL_ERROR_SSL:
            default:
                return false;
            }
        }

        return false;

    }

    bool secure_connect()
    {
        sockaddr_in addr = {
         .sin_family = PF_INET,
         .sin_port = htons(port),
        };
        addr.sin_addr.s_addr = INADDR_ANY;
        socket_wrapper::Socket sock_serv = { AF_INET, SOCK_STREAM, IPPROTO_TCP }; 

        if (bind(sock_serv, reinterpret_cast<const sockaddr*>(&addr), sizeof(addr)) != 0)
        {
            std::cerr << wrap.get_last_error_string() << std::endl;

        }
        if (listen(sock_serv, SOMAXCONN) != 0)
        {
            std::cerr << wrap.get_last_error_string() << std::endl;

        }
        sockaddr_in client_address = { 0 };
        socklen_t client_address_len = sizeof(sockaddr_in);
        ssize_t recv_len = 0;
        socket_wrapper::Socket sock_client 
            = accept(sock_serv, reinterpret_cast<sockaddr*>(&client_address), &client_address_len);
        if (!sock_client)
        {
            std::cerr << wrap.get_last_error_string() << std::endl;
            return false;
        }
        return true;
        init_ssl();
        SSL_set_fd(ssl, sock_client);
    }

    void load_certificate(std::string CertFile, std::string KeyFile)
    {

        if (SSL_CTX_use_certificate_file(ctx, CertFile.c_str(), SSL_FILETYPE_PEM) <= 0)
        {
            ERR_print_errors_fp(stderr);
            return;
        }

        if (SSL_CTX_use_PrivateKey_file(ctx, KeyFile.c_str(), SSL_FILETYPE_PEM) <= 0)
        {
            ERR_print_errors_fp(stderr);
            return;
        }

        if (!SSL_CTX_check_private_key(ctx))
        {
            std::cerr << "Private key does not match the public certificate\n";
        }
    }
    void show_certs()
    {
        X509* cert;
        cert = SSL_get_peer_certificate(ssl); /* Get certificates (if available) */
        if (cert != nullptr)
        {
            std::string buf;
            buf.resize(256);

            std::cout
                << "Peer certificate:\n"
                << "Subject: " << X509_NAME_oneline(X509_get_subject_name(cert), buf.data(), buf.size()) << "\n"
                << "Issuer: " << X509_NAME_oneline(X509_get_issuer_name(cert), buf.data(), buf.size()) << "\n"
                << std::endl;

            X509_free(cert);
        }
        else
            std::cerr << "No certificates.\n";
    }

    bool autorization()
    {
        std::string str_autorization = "enter login: ";
        send_packet(str_autorization);
        recv_packet();
        auto i = pass.find(message);
        if (i == pass.end())
        {
            return false;
        }

        autoriz = "enter password";
        send_packet(str_autorization);
        recv_packet();

        if (i->second != std::hash<std::string>{}(message))
        {
            return autoriz;
        }
        else
        {
            autoriz = true;
            return autoriz;
        }
    }

    bool registration()
    {
        std::string registr;
        registr = "enter login: ";
        send_packet(registr);
        recv_packet();
        auto i = pass.find(message);
        if (pass.find(message) != pass.end())
        {
            return false;
        }
        std::pair<std::string, size_t> temp;
        temp.first = message;
        registr = "enter password: ";
        send_packet(registr);
        recv_packet();
        temp.second = std::hash<std::string>{}(message);
        pass.insert(temp);
        autoriz = true;

        return true;
    }

    void work_server()
    {
        if (!secure_connect())
        {
            return;
        }

        bool run = true;
        show_certs();
        while (run)
        {
            int bytes;

            if (SSL_accept(ssl) <= 0)
            {
                ERR_print_errors_fp(stderr);
                return;
            }
            bytes = SSL_read(ssl, message.data(), message.size());
            message[bytes] = '\0';

            if (message.find("autorization"))
            {
                if (!autorization())
                {
                    message = "invalid username or password";
                    send_packet(message);
                    continue;
                }
            }
            else if (message.find("registration"))
            {

            }
            else
            {
                if (message.find("file") == 0)
                {
                    if (!autoriz)
                    {
                        message = "to download files, log in";
                        send_packet(message);
                        continue;
                    }
                    std::string file_path(message, 5, bytes);
                    std::cout << file_path;
                    std::ifstream ifs(file_path);
                    std::string line;
                    if (ifs.is_open())
                    {
                        while (getline(ifs, line))      
                        {
                            send_packet(line);
                        }
                        line = "exit_file__";           
                        ifs.close();
                    }
                    else
                    {
                        line = "error open file \n";
                        send_packet(line);
                        line = "exit_file__";
                    }
                    send_packet(line);      
                    continue;
                }
            }

            send_packet(message);
            if ("exit" == message) run = false;
        }
    }
};





int main(int argc, const char* const argv[])
{
    if (argc != 2)
    {
        std::cout << "Usage: " << argv[0] << " <port>" << std::endl;
        return EXIT_FAILURE;
    }

    server_ssl server(atoi(argv[1]));
    server.load_certificate("mycert.pem", "mycert.pem");
    server.work_server();


    return EXIT_SUCCESS;
}