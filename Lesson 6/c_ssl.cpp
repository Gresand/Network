#include <iostream>
#include <string>
#include <sstream>
#include <cstdlib>

extern "C"
{
#include <openssl/ssl.h>
#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/x509.h>
}

const int buff_size = 1024;

class client_ssl 
{
private:
    SSL_CTX* ctx = nullptr;
    BIO* bio = nullptr;
    SSL* ssl = nullptr;
    std::string &hostname;
    int port;
public:
    client_ssl(std::string &host, int iport) : hostname(host)
    {
        SSL_load_error_strings();
        SSL_library_init();
        port = iport;

    }
    ~client_ssl()
    {
        SSL_CTX_free(ctx);
        BIO_free_all(bio);
        SSL_shutdown(ssl);
    }
    bool print_error(const std::string& msg)
    {
        perror(msg.c_str());
        ERR_print_errors_fp(stderr);
        return false;
    }

    bool secure_connect()
    {
        std::stringstream request;
        std::string response;
        response.resize(buff_size);

        const SSL_METHOD* method = TLSv1_2_client_method();
        if (method == nullptr) return print_error("error TLS_client_method...");
        ctx = SSL_CTX_new(method);
        if (ctx == nullptr) return print_error("error SSL_CTX_new...");
        bio = BIO_new_ssl_connect(ctx);
        if (bio == nullptr) return print_error("error BIO_new_ssl_connect...");

        BIO_get_ssl(bio, &ssl);
        SSL_set_mode(ssl, SSL_MODE_AUTO_RETRY);
        std::cout
            << "Connecting to \""
            << hostname << "\"..."
            << std::endl;
        if (SSL_set_tlsext_host_name(ssl, hostname.c_str()) != 1)
        {
            return print_error("error SSL_set_tlsext_host_name");
        }

        
        BIO_set_conn_hostname(bio, hostname.c_str());
        BIO_set_conn_port(bio, port);

        
        if (!SSL_CTX_load_verify_locations(ctx, "ca-certificates.crt", "./"));    
        {
            return print_error("error SSL_CTX_load_verify_locations...");
        }

        
        if (BIO_do_connect(bio) < 0)
        {
            print_error("error BIO_do_connect..");
        }

        
        X509* cert = SSL_get_peer_certificate(ssl);
        if (cert == nullptr)
        {
            std::cerr
                << "No peer certificates."
                << std::endl;
        }
        else
        {
            std::string buf;
            buf.resize(buff_size);
            std::cout<< "Peer certificate:\n"<< "Subject: " << X509_NAME_oneline(X509_get_subject_name(cert), buf.data(), buf.size()) << "\n"
                << "Issuer: " << X509_NAME_oneline(X509_get_issuer_name(cert), buf.data(), buf.size()) << "\n"
                << std::endl;
            X509_free(cert);

            STACK_OF(X509)* certs = SSL_get_peer_cert_chain(ssl);
            for (int i = 1; i < sk_X509_num(certs); ++i)
            {
                auto ct = sk_X509_value(certs, i);
                std::cout
                    << "Cert " << i << " in chain:\n"
                    << "Subject: " << X509_NAME_oneline(X509_get_subject_name(ct), buf.data(), buf.size()) << "\n"
                    << "Issuer: " << X509_NAME_oneline(X509_get_issuer_name(ct), buf.data(), buf.size()) << "\n"
                    << "---"
                    << std::endl;
            }

            long verify_flag = SSL_get_verify_result(ssl);
            switch (verify_flag)
            {
                
            case X509_V_ERR_DEPTH_ZERO_SELF_SIGNED_CERT:
            case X509_V_ERR_SELF_SIGNED_CERT_IN_CHAIN:
                std::cerr
                    << "##### Certificate verification error: self-signed ("
                    << X509_verify_cert_error_string(verify_flag) << ", "
                    << verify_flag << ") but continuing..."
                    << std::endl;
                break;
            case X509_V_OK:
                    std::cout
                    << "##### Certificate verification passed..."
                    << std::endl;
                    break;
            default:
                std::cerr
                    << "##### Certificate verification error ("
                    << X509_verify_cert_error_string(verify_flag) << ", "
                    << verify_flag << ") but continuing..."
                    << std::endl;
            }


            request
                << "GET / HTTP/1.1/\r\n"
                << "Host: " << hostname << "\r\n"
                << "Connection: Close\r\n\r\n";
            BIO_puts(bio, request.str().c_str());


            while (true)
            {
                std::fill(response.begin(), response.end(), response.size());
                int n = BIO_read(bio, &response[0], response.size());
                // 0 is end-of-stream, < 0 is an error.
                if (n <= 0) break;


                if (response.find("file") == 0)      // command <file>
                {
                    while (true)
                    {
                        BIO_read(bio, &response[0], response.size());   


                        if ("exit_file__" == response)
                        {
                            break;
                        }
                        std::cout << response;
                    }
                }
                std::cout << response << std::endl;
            }
            return true; 
        }
    }
};

int main(int argc, const char* const argv[])
{
    if (argc != 3)
    {
        std::cout << "Usage: " << argv[0] << " <host> <port>" << std::endl;
        return EXIT_FAILURE;
    }
    std::string hostname = argv[1];
    int port = atoi(argv[2]);
    client_ssl client(hostname, port);


    std::cout << "Trying an HTTPS connection to " << hostname << ':' << port << "..." << std::endl;
    if (!client.secure_connect()) return EXIT_FAILURE;

    return EXIT_SUCCESS;
}