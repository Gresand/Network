int main ()
{
    std::string host = "localhost";
    std::string port = "1337";

    OpenSSL_add_ssl_algorithms();
//РЈРєР°Р·С‹РІР°РµРј РјРµС‚РѕРґ РєРѕС‚РѕСЂС‹Рј Р±СѓРґРµРј РїРѕР»СЊР·РѕРІР°С‚СЊСЃСЏ 
//(РІ РїСЂРёРјРµСЂРµ РґР»СЏ СЃРµСЂРІРµСЂР° Р±С‹Р»Рѕ SSLv23_server_method)
    const SSL_METHOD *method = SSLv23_client_method();
    
    SSL_CTX * ctx = SSL_CTX_new(method);
    if (!ctx)
    {
        perror("Unable to create context");
        return -1;
    }
//РЎРѕР·РґР°РЅРёРµ С„Р°Р№Р»РѕРІРѕРіРѕ РґРµСЃРєСЂРёРїС‚РѕСЂР° Рё С‚Р°Рє Р¶Рµ РїРѕРґРєР»СЋС‡РµРЅРёРµ
    int sock;
    addrinfo hint, * server_info, * p;
    memset(&hint, 0, sizeof hint);
    hint.ai_family = AF_INET;
    hint.ai_socktype = SOCK_STREAM;
    hint.ai_protocol = IPPROTO_TCP;
    int ret = getaddrinfo(host.c_str(), port.c_str(), &hint, &server_info);
    if (ret != 0)
    {
        std::cout << "Error occured while resolving hostname" << std::endl;
        return -1;
    }

    for (p = server_info; p != NULL; p = p->ai_next)
    {
        sock = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (sock == -1)
        {
            continue;
        }
        ret = connect(sock, p->ai_addr, p->ai_addrlen);
        if (ret == -1)
        {
            close(sock);
            continue;
        }
        break;
    }
//РЎРѕР·РґР°РЅРёРµ SSL СЃС‚СЂСѓРєС‚СѓСЂС‹ Рё РїСЂРёРІСЏР·РєР° Рє СЃРѕРєРµС‚Сѓ
    SSL *ssl = SSL_new(ctx);
    SSL_set_fd(ssl, sock);
//РљР°Рє СЏ РїРѕРЅСЏР» СЌС‚Рѕ Рё РµСЃС‚СЊ СЂСѓРєРѕРїРѕР¶Р°С‚РёРµ
    if (SSL_connect(ssl) <= 0)
    {
        std::cout << "Something went wrong" << std::endl;
    }
    else
    {
//РђРЅР°Р»РѕРі recv(), РЅРѕ РґР»СЏ TLS
        char buff [22];
        SSL_read(ssl, buff, 22);
        std:: cout << buff << std::endl;
    }
    
    SSL_CTX_free(ctx);
    close(sock);
}