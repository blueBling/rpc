#include <iostream>
#include <algorithm>
using namespace std;

#include "Server.h"
using namespace yazi::rpc;

#include "ImPduBase.h"
#include "IM.Login.pb.h"

string hello(const string & name)
{
    return "hello, " + name;
}

int sum(int a, int b)
{
    return a + b;
}

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



Response login(const Request& req)
{
    string name = req.name();
	string passwd = req.passwd();
    transform(name.begin(), name.end(), name.begin(), ::toupper);
	
	CImPdu resPdu;

	if(req.pdu().GetServiceId() == IM::BaseDefine::SID_LOGIN) {
		IM::Login::IMLoginRes msgRes;
		msgRes.set_result_code(IM::BaseDefine::REFUSE_REASON_NONE);
		IM::BaseDefine::UserInfo* user_info_tmp = msgRes.mutable_user_info();
		user_info_tmp->set_user_id(1001);
	    user_info_tmp->set_user_gender(1);
	    user_info_tmp->set_user_nick_name("zsan");
	    user_info_tmp->set_user_tel("13412313459");

		
		resPdu.SetPBMsg(&msgRes);
		resPdu.SetSeqNum(1);
		resPdu.SetServiceId(IM::BaseDefine::SID_LOGIN);
		resPdu.SetCommandId(IM::BaseDefine::CID_LOGIN_RES_REGIST);
		resPdu.ReadPduHeader(resPdu.GetBuffer(), resPdu.GetLength());
	}

	return Response(name, passwd, resPdu);
}


int main()
{
    Server * server = Singleton<Server>::instance();
    server->listen("127.0.0.1", 8080);
    server->bind("hello", hello);
    server->bind("sum", sum);
    server->bind("login", login);
    server->start();

    return 0;
}
