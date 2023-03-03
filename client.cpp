#include <iostream>
using namespace std;

#include <signal.h>

#include "Client.h"
using namespace yazi::rpc;

#include "ImPduBase.h"
#include "IM.Login.pb.h"
#include "HttpClient.h"

class Request : public Serializable
{
public:
	Request() {}
    Request(const string & name, const string & passwd, const CImPdu & pdu) : m_name(name), m_passwd(passwd), m_pdu(pdu) {}
    ~Request() {}

    const string & name() const
    {
        return m_name;
    }

	const string & passwd() const
    {
        return m_passwd;
    }

	const CImPdu & pdu() const
    {
        return m_pdu;
    }

    SERIALIZE(m_name, m_passwd, m_pdu)

private:
    string m_name;
	string m_passwd;
	CImPdu m_pdu;
};

class Response : public Serializable
{
public:
	Response() {}
    Response(const string & name, const string & passwd, const CImPdu & pdu) : m_name(name), m_passwd(passwd), m_pdu(pdu) {}
    ~Response() {}

    const string & name() const
    {
        return m_name;
    }

	const string & passwd() const
    {
        return m_passwd;
    }

	const CImPdu & pdu() const
    {
        return m_pdu;
    }

    SERIALIZE(m_name, m_passwd, m_pdu)

private:
    string m_name;
	string m_passwd;
	CImPdu m_pdu;
};

int tcp_client() {
    Client client;
    client.connect("127.0.0.1", 8080);

    auto reply = client.call<string>("hello", "kitty");
    std::cout << reply << std::endl;

    auto total = client.call<int>("sum", 1, 2);
    std::cout << total << std::endl;

	IM::Login::IMLoginReq msgReq;
	msgReq.set_user_name("zhangsan");
	msgReq.set_password("123456");

	CImPdu pduReq;
	pduReq.SetPBMsg(&msgReq);
	pduReq.SetSeqNum(1);
	pduReq.SetServiceId(IM::BaseDefine::SID_LOGIN);
	pduReq.SetCommandId(IM::BaseDefine::CID_LOGIN_REQ_REGIST);
	pduReq.ReadPduHeader(pduReq.GetBuffer(), pduReq.GetLength());
	
    Request req("lisi", "321", pduReq);
    auto resp = client.call<Response>("login", req);
	CImPdu pduRes = resp.pdu();
	
	std::cout << "serviceId:" << pduRes.GetServiceId() << " command id:" << pduRes.GetCommandId() << std::endl;
	if(pduRes.GetServiceId() == IM::BaseDefine::SID_LOGIN) {
		IM::Login::IMLoginRes msgRes;
		if(msgRes.ParseFromArray(pduRes.GetBodyData(), pduRes.GetBodyLength())){
			std::cout << "nick name:" << msgRes.user_info().user_nick_name() << " tel:" << 
				msgRes.user_info().user_tel() << std::endl;
		}
	}
	return 0;
}

int http_client() {
	CHttpClient httpClient;

    auto reply = httpClient.call<string>("hello", "kitty");
    std::cout << reply << std::endl;

    auto total = httpClient.call<int>("sum", 1, 2);
    std::cout << total << std::endl;
	
	IM::Login::IMLoginReq msgReq;
	msgReq.set_user_name("zhangsan");
	msgReq.set_password("123456");

	CImPdu pduReq;
	pduReq.SetPBMsg(&msgReq);
	pduReq.SetSeqNum(1);
	pduReq.SetServiceId(IM::BaseDefine::SID_LOGIN);
	pduReq.SetCommandId(IM::BaseDefine::CID_LOGIN_REQ_REGIST);
	pduReq.ReadPduHeader(pduReq.GetBuffer(), pduReq.GetLength());
	
    Request req("lisi", "321", pduReq);
    auto resp = httpClient.call<Response>("login", req);
	CImPdu pduRes = resp.pdu();
	std::cout << "serviceId:" << pduRes.GetServiceId() << " command id:" << pduRes.GetCommandId() << std::endl;
	if(pduRes.GetServiceId() == IM::BaseDefine::SID_LOGIN) {
		IM::Login::IMLoginRes msgRes;
		if(msgRes.ParseFromArray(pduRes.GetBodyData(), pduRes.GetBodyLength())){
			std::cout << "nick name:" << msgRes.user_info().user_nick_name() << " tel:" << 
				msgRes.user_info().user_tel() << std::endl;
		}
	}

	return 0;
}

static void signal_handler_usr1(int sig_no)
{
	if (sig_no == SIGPIPE) {
		std::cout << "recv SIGPIPE,will exit!!" << std::endl;
		std::exit(0);
	}
}

int main()
{
	signal(SIGPIPE, signal_handler_usr1);

	//std::cout << "tcp_client >>>" << std::endl;
	//tcp_client();
	
	std::cout << "http_client >>>" << std::endl;
	http_client();

	std::cout << "exit" << std::endl;
    return 0;
}