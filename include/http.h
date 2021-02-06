/*
BaseHttpSvr使用例子：
class MySvr : public BaseHttpSvr
{
private:
	virtual void RevRequest()
	{
		std::string data;
		GetData(std::string data);
		...
	}
};

BaseHttpClient使用例子：
class MyClient : public BaseHttpClient
{
private:
	virtual void Reply(struct evhttp_request* remote_rsp) override
	{
	}
};

*/

#pragma once

#include <event2/event.h>
#include <event2/buffer.h>
#include <event2/http.h>
#include <event2/dns.h>
#include <event2/http_struct.h>
#include <string>
#include <functional>
#include "lc_typedef.h"
#include <memory>

namespace lc //libevent cpp
{
	class BaseHttpSvr
	{
	private:
		evhttp *m_evhttp = nullptr;
		std::string m_ip;
		uint16 m_port = 0;
		evhttp_request *m_cur_req = nullptr; //当前接收请求的evhttp_request对象， nullptr表示不存在。 

	public:
		virtual ~BaseHttpSvr();
		bool Init(const char* ip = nullptr, unsigned short port = 80);

		//------------------------一下操作都是对当前请求操作，如果没当前请求，调用失败. ------------------------
		evhttp_cmd_type GetMethod() const;

		const char *GetUri() const;

		//获取uri “?”后面的字符串
		std::string GetUriQuery() const;

		std::string GetPath() const;

		//get http req body data.
		void GetData(std::string &data) const;

		//当前请求的 evhttp_request，如何使用请查 libevent说明
		evhttp_request *GetCurReq();

		//发送消息前加头
		void AddHeader(const std::string &name, const std::string &value);
		//@code 状态码，成功200 参考 HTTP_OK 宏定义
		//@reason 状态码描述，比如"ok"
		void Send(const std::string &data = "", int code= HTTP_OK, const char *reason="ok");
		//---------------------------------------------------------------------------------------------

	private:
		virtual void RevRequest() = 0;
		static void RevRequestCB(struct evhttp_request* req, void* arg);

		using URI_PRT = std::unique_ptr<evhttp_uri, decltype(&::evhttp_uri_free)>;
		//安全地调用 evhttp_uri_parse
		URI_PRT safe_evhttp_uri_parse(const char *uri) const;

	};


	class BaseHttpClient
	{
	private:
		struct evdns_base* m_dnsbase = nullptr;
		struct evhttp_connection* m_con = nullptr;
		bool m_is_req = false; //true表示已经发出请求，等响应中

	public:
		virtual ~BaseHttpClient();

		//true表示已经发出请求，等响应中
		bool IsReq() const { return m_is_req; };
		bool Request(const char *url, evhttp_cmd_type cmd_type = EVHTTP_REQ_GET, const char *post_data = nullptr, unsigned int ot_sec = 5);

	private:
		virtual void Respond(const char *str) = 0;
		//服务器响应错误信息
		virtual void RespondError(int err_code, const char *err_str) {};
		//超时没响应，或者连接失败
		virtual void ConnectFail() {};

		void Free();

		//请求连接失败，超时接收不到消息响应回调
		//情况：
		//服务器连不通
		static void remote_read_callback(struct evhttp_request* remote_rsp, void* arg);
		//调用情况：
		//关闭连接就会调用
		static void connection_close_callback(struct evhttp_connection* connection, void* arg);

	};
}//namespace lc //libevent cpp