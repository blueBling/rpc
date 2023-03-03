#pragma once

#include "UtilPdu.h"

#include <iostream>
using namespace std;

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>

#include <string>
using namespace std;

namespace yazi {
namespace socket {

class Socket
{
    friend class SocketHandler;

public:
    Socket();
    Socket(const string &ip, int port);
    virtual ~Socket();

	CSimpleBuffer & get_in_buf() { return m_in_buf; }

    bool bind(const string &ip, int port);
    bool listen(int backlog);
    bool connect(const string &ip, int port);
    bool close();

    int accept();
    int recv(char * buf, int len);
	int recv(unsigned char * buf, int len);
    int send(const char * buf, int len);
	int send(const unsigned char * buf, int len);
    
    bool set_non_blocking();
    bool set_send_buffer(int size);
    bool set_recv_buffer(int size);
    bool set_linger(bool active, int seconds);
    bool set_keep_alive();
    bool set_reuse_addr();
    bool set_reuse_port();

protected:
    string  m_ip;
    int     m_port;
    int     m_sockfd;
	CSimpleBuffer	m_in_buf;
};

}}
