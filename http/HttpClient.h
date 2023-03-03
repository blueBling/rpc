#ifndef __HTTP_CURL_H__
#define __HTTP_CURL_H__

#include "DataStream.h"
using namespace yazi::serialize;

#include "UtilPdu.h"
#include "HttpParserWrapper.h"

#include <string>
using namespace std;

#include <curl/curl.h>

class CHttpClient
{
public:
    CHttpClient(void);
    ~CHttpClient(void);
    
public: 

    template <typename R, typename ...Args>
    R call(const string & name, const Args&... args);
	
    CURLcode Post(const char* strUrl, unsigned char* msgPost, int len, CSimpleBuffer & msgResponse);
		
    CURLcode Get(const char* strUrl, CSimpleBuffer & msgResponse);
};

template <typename R, typename ...Args>
R CHttpClient::call(const string & name, const Args&... args)
{
	DataStream in;
    in << name;
    in.write_args(args...);

    unsigned char send_buff[1024];
	memset(send_buff, '\0', sizeof(send_buff));
    std::memcpy(send_buff, in.data(), in.size());

    int len = in.size();

	R v;

	CHttpClient httpClient;
	CSimpleBuffer rsp;
	rsp.Extend(2048);
    CURLcode nRet = httpClient.Post("http://127.0.0.1:8080/msg_server", send_buff, len, rsp);
    if(nRet != CURLE_OK)
    {
		std::cout << "login falied. access url error, nRet=" << nRet << std::endl;
        return v;
    }

	DataStream out;
    out.write(rsp.GetBuffer(), rsp.GetWriteOffset());
    out >> v;

    return v;
}


#endif
