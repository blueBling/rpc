#include <string>
#include "HttpClient.h"
using namespace std;

CHttpClient::CHttpClient(void)
{
    
}

CHttpClient::~CHttpClient(void)
{
    
}

static size_t OnWriteData(void* buffer, size_t size, size_t nmemb, void* lpVoid)
{
 	//MsgRsp* msg = (MsgRsp *)lpVoid;
	CSimpleBuffer* msg = dynamic_cast<CSimpleBuffer*>((CSimpleBuffer *)lpVoid);
    if( NULL == msg || NULL == buffer )
    {
        return -1;
    }
	msg->Write(buffer, size * nmemb);
    return size * nmemb;
}

CURLcode CHttpClient::Post(const char* strUrl, unsigned char* msgPost, int len, CSimpleBuffer & msgResponse)
{
	CURLcode res;
    CURL* curl = curl_easy_init();
    if(NULL == curl)
    {
        return CURLE_FAILED_INIT;
    }
    
    curl_easy_setopt(curl, CURLOPT_URL, strUrl);
    curl_easy_setopt(curl, CURLOPT_FORBID_REUSE, 1);
    curl_easy_setopt(curl, CURLOPT_POST, 1);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, msgPost);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE_LARGE, len);
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, OnWriteData);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&msgResponse);
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 3);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 3);
    res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    return res;
}

CURLcode CHttpClient::Get(const char* strUrl, CSimpleBuffer & msgResponse)
{
    CURLcode res;
    CURL* curl = curl_easy_init();
    if(NULL == curl)
    {
        return CURLE_FAILED_INIT;
    }
    
    curl_easy_setopt(curl, CURLOPT_URL, strUrl);
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);
    curl_easy_setopt(curl, CURLOPT_FORBID_REUSE, 1);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, OnWriteData);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&msgResponse);
    /**
     * 当多个线程都使用超时处理的时候，同时主线程中有sleep或是wait等操作。
     * 如果不设置这个选项，libcurl将会发信号打断这个wait从而导致程序退出。
     */
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 3);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 3);
    res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    return res;
}