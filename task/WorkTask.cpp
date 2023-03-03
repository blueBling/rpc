#include "WorkTask.h"
using namespace yazi::task;

#include "Logger.h"
#include "Singleton.h"
using namespace yazi::utility;

#include "SocketHandler.h"
using namespace yazi::socket;

#include "Server.h"
using namespace yazi::rpc;

#include "DataStream.h"
using namespace yazi::serialize;

#include <sstream>
using std::ostringstream;

#include "HttpParserWrapper.h"

#define READ_BUF_SIZE	2048

WorkTask::WorkTask(Socket * socket) : Task(socket)
{
}

WorkTask::~WorkTask()
{
}

void WorkTask::run()
{

    SocketHandler * handler = Singleton<SocketHandler>::instance();
    Socket * socket = static_cast<Socket *>(m_data);

	CSimpleBuffer in_buf = socket->get_in_buf();

    for (;;)
    {
        uint32_t free_buf_len = in_buf.GetAllocSize() - in_buf.GetWriteOffset();
        if (free_buf_len < READ_BUF_SIZE + 1)
            in_buf.Extend(READ_BUF_SIZE + 1);
		
		int len = socket->recv(in_buf.GetBuffer() + in_buf.GetWriteOffset(), READ_BUF_SIZE);

		if (len == 0)
		{
			warn("socket closed by peer");
			handler->remove(socket);
		}
		if (len == -1 && errno == EAGAIN)
		{
			warn("socket recv len: %d, error msg: EAGAIN errno: %d", len, errno);
			handler->attach(socket);
		}
		if (len == -1 && errno == EWOULDBLOCK)
		{
			warn("socket recv len: %d, error msg: EWOULDBLOCK errno: %d", len, errno);
			handler->attach(socket);
		}
		if (len == -1 && errno == EINTR)
		{
			warn("socket recv len: %d, error msg: EINTR errno: %d", len, errno);
			handler->attach(socket);
		}

        if (len <= 0) 
            break;

        in_buf.IncWriteOffset(len);

		// std::cout << "recv " << len << " bytes,cur len " << in_buf.GetWriteOffset() << " bytes" << std::endl; // debug by coderL

		// 对长度限制下，防止恶意数据
		if(in_buf.GetWriteOffset() > 1024) {
			std::cout << "too long" << std::endl;
			handler->remove(socket);
			return;
		}
    }

	// 该轮次已全部收取
	
	int tcp_head_len = sizeof(MsgHead);
	int len = in_buf.GetWriteOffset();
	
	if(len < tcp_head_len) {
		return;
	}

	MsgHead msg_head;
    memset(&msg_head, 0, sizeof(msg_head));
    memcpy(&msg_head, in_buf.GetBuffer(), tcp_head_len);

	if(strcmp(msg_head.flag, "tcp") == 0) // tcp
	{ 
	    DataStream in;
	    in.write(in_buf.GetBuffer()+tcp_head_len, len-tcp_head_len);

	    string func;
	    in >> func;

		// std::cout << "call func:" << func << std::endl;

	    DataStream out;
	    Server * server = Singleton<Server>::instance();
	    server->call(func, in, out);
	    socket->send(out.data(), out.size());
	    handler->attach(socket);
	} 
	else  // http
	{
		CHttpParserWrapper httpParser;
		httpParser.ParseHttpContent((char*)in_buf.GetBuffer(), len);

		if (httpParser.IsReadAll()) {
			string url =  httpParser.GetUrl();
			char* content = httpParser.GetBodyContent();

			int data_len = httpParser.GetContentLen();

			DataStream in;
		    in.write(content, data_len);

		    string func;
		    in >> func;

			// std::cout << "call func:" << func << std::endl;

			DataStream out;
		    Server * server = Singleton<Server>::instance();
		    server->call(func, in, out);

			char szResponseHeader[1024];
			snprintf(szResponseHeader,sizeof(szResponseHeader), HTTP_RESPONSE_EXTEND, out.size());
			int nLen = strlen(szResponseHeader);

			unsigned char send_buf[2048];
			if(out.size() > 1024) {
		        handler->remove(socket);
		        return;
			}
			memcpy(send_buf, szResponseHeader, nLen);
			memcpy(&send_buf[nLen], out.data(), out.size());
			nLen += out.size();
			
		    socket->send(send_buf, nLen);
		    handler->attach(socket);
		}
	}

}

// only for tcp
#if 0
void WorkTask::run()
{
    debug("work task run");
    SocketHandler * handler = Singleton<SocketHandler>::instance();

    Socket * socket = static_cast<Socket *>(m_data);

    MsgHead msg_head;
    memset(&msg_head, 0, sizeof(msg_head));
    int len = socket->recv((char *)(&msg_head), sizeof(msg_head));
    if (len == 0)
    {
        warn("socket closed by peer");
        handler->remove(socket);
        return;
    }
    if (len == -1 && errno == EAGAIN)
    {
        warn("socket recv len: %d, error msg: EAGAIN errno: %d", len, errno);
        handler->attach(socket);
        return;
    }
    if (len == -1 && errno == EWOULDBLOCK)
    {
        warn("socket recv len: %d, error msg: EWOULDBLOCK errno: %d", len, errno);
        handler->attach(socket);
        return;
    }
    if (len == -1 && errno == EINTR)
    {
        warn("socket recv len: %d, error msg: EINTR errno: %d", len, errno);
        handler->attach(socket);
        return;
    }
    if (len != sizeof(msg_head))
    {
        error("recv msg head error length: %d, errno: %d", len, errno);
        handler->remove(socket);
        return;
    }
    info("recv msg head len: %d, flag: %s, cmd: %d, body len: %d", len, msg_head.flag, msg_head.cmd, msg_head.len);
    if (strncmp(msg_head.flag, "work", 4) != 0)
    {
        error("msg head flag error");
        handler->remove(socket);
        return;
    }

    if (msg_head.len >= uint32_t(recv_buff_size))
    {
        error("recv msg body len: %d, large than recv_buff_size: %d", msg_head.len, recv_buff_size);
        handler->remove(socket);
        return;
    }

    char buf[recv_buff_size];
    memset(buf, 0, recv_buff_size);
    len = socket->recv(buf, msg_head.len);
    if (len == -1 && errno == EAGAIN)
    {
        warn("socket recv len: %d, error msg: EAGAIN errno: %d", len, errno);
        handler->remove(socket);
        return;
    }
    if (len == -1 && errno == EWOULDBLOCK)
    {
        warn("socket recv len: %d, error msg: EWOULDBLOCK errno: %d", len, errno);
        handler->remove(socket);
        return;
    }
    if (len == -1 && errno == EINTR)
    {
        warn("socket recv len: %d, error msg: EINTR errno: %d", len, errno);
        handler->remove(socket);
        return;
    }
    if (len != (int)(msg_head.len))
    {
        error("recv msg body error length: %d, body: %s, errno: %d", len, buf, errno);
        handler->remove(socket);
        return;
    }

    info("recv msg body len: %d, msg data: %s", len, buf);

    DataStream in;
    in.write(buf, len);

    string func;
    in >> func;

    DataStream out;
    Server * server = Singleton<Server>::instance();
    server->call(func, in, out);
    socket->send(out.data(), out.size());
    handler->attach(socket);
}
#endif

void WorkTask::destroy()
{
    debug("work task destory");
    delete this;
}
