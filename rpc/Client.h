#pragma once

#include <iostream>
#include <string>
#include <cstring>
using namespace std;

#include "ClientSocket.h"
using namespace yazi::socket;

#include "DataStream.h"
using namespace yazi::serialize;

namespace yazi {
namespace rpc {

struct MsgHead {
    char flag[8];
    uint32_t cmd;
    uint32_t len;
};

const uint32_t send_buff_size = 1024;
const uint32_t recv_buff_size = 1024;

class Client
{
public:
    Client();
    ~Client();

    bool connect(const string & ip, int port);

    template <typename R, typename ...Args>
    R call(const string & name, const Args&... args);

private:
    string m_ip;
    int m_port;
    ClientSocket m_socket;
};

template <typename R, typename ...Args>
R Client::call(const string & name, const Args&... args)
{
    DataStream in;
    in << name;
    in.write_args(args...);

    MsgHead head;
    strcpy(head.flag, "tcp");
    head.cmd = 0;
    head.len = in.size();

    char send_buff[send_buff_size];
    std::memcpy(send_buff, (char *)&head, sizeof(MsgHead));
    std::memcpy(send_buff + sizeof(MsgHead), in.data(), in.size());

    int len = sizeof(MsgHead) + in.size();
    m_socket.send(send_buff, len);

	// debug by blueBling
	//m_socket.send(send_buff, 10);
	//sleep(1);
	//m_socket.send(&send_buff[10], len-10);
	

    char recv_buff[recv_buff_size];
    int recv_size = m_socket.recv(recv_buff, recv_buff_size);

	// TODO add by blueBling 对recv各种情况处理,如对SIGPIPE处理等

	R v;
	if(recv_size <= 0)
		return v;
	
    DataStream out;
    out.write(recv_buff, recv_size);

    out >> v;
    return v;
}

}
}