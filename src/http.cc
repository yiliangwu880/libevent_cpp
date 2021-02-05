
#include "http.h"
#include <stddef.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>  
#include "include_all.h"
#include "log_file.h" //目前调试用，移植删掉


using namespace std;

namespace lc //libevent cpp
{

	BaseHttpSvr::~BaseHttpSvr()
	{
		if (nullptr != m_evhttp)
		{
			evhttp_free(m_evhttp);
			m_evhttp = nullptr;
		}
	}

	bool BaseHttpSvr::Init(const char* ip, unsigned short port)
	{
		if (nullptr != m_evhttp)
		{
			LB_FATAL("repeated init");
			return false;
		}
		m_evhttp = evhttp_new(EventMgr::Ins().GetEventBase());
		if (nullptr == m_evhttp) {
			LB_FATAL("init http fail");
			return false;
		}
		if (nullptr == ip)
		{
			ip = "0.0.0.0";
		}
		//绑定到指定地址上
		int ret = evhttp_bind_socket(m_evhttp, ip, port);
		if (ret != 0) {
			LB_FATAL("init http fail, ip=%s, port=%d", ip, port);
			return false;
		}
		evhttp_set_gencb(m_evhttp, RevRequestCB, this);
		//evhttp_set_cb(m_evhttp, "/phoneop/", RevRequestCB, this);
		m_ip = ip;
		m_port = port;
		return true;
	}


	evhttp_cmd_type BaseHttpSvr::GetMethod() const
	{
		if (nullptr == m_cur_req)
		{
			LB_ERROR("invaild req, on GetMethod");
			return EVHTTP_REQ_GET;
		}
		return evhttp_request_get_command(m_cur_req);
	}

	std::string BaseHttpSvr::GetUriQuery() const
	{
		if (nullptr == m_cur_req)
		{
			LB_ERROR("invaild req, on GetUriQuery");
			return "";
		}
		const char *uri = evhttp_request_get_uri(m_cur_req);
		if (nullptr == uri)
		{
			LB_ERROR("no uri");
			return "";
		}

		/* Decode the URI */

		using URI_PRT = std::unique_ptr<evhttp_uri, decltype(&::evhttp_uri_free)>;
		URI_PRT decoded(evhttp_uri_parse(uri), ::evhttp_uri_free);
		if (!decoded.get()) {
			LB_ERROR("It's not a good URI. Sending BADREQUEST\n");
			return "";
		}
		//解析URL中method参数
		const char *p = evhttp_uri_get_query(decoded.get());
		if (nullptr == p)
		{
			return "";
		}
		return string(p);
	}


	string BaseHttpSvr::GetPath() const
	{
		if (nullptr == m_cur_req)
		{
			LB_ERROR("invaild req, on GetPath");
			return nullptr;
		}
		const char *uri = evhttp_request_get_uri(m_cur_req);
		if (nullptr == uri)
		{
			LB_ERROR("no uri");
			return nullptr;
		}

		/* Decode the URI */
		URI_PRT decoded = safe_evhttp_uri_parse(uri);
		if (!decoded.get()) {
			LB_ERROR("It's not a good URI. Sending BADREQUEST\n");
			return nullptr;
		}
		const char *path = evhttp_uri_get_path(decoded.get());
		if (nullptr == path)
		{
			return "/";
		}
		return string(path);
	}


	void BaseHttpSvr::GetData(std::string &data) const
	{
		if (nullptr == m_cur_req)
		{
			LB_ERROR("invaild req, on GetData");
			return;
		}
		data.clear();
		evbuffer *buf = evhttp_request_get_input_buffer(m_cur_req);
		while (evbuffer_get_length(buf)) {
			int n;
			char cbuf[128];
			n = evbuffer_remove(buf, cbuf, sizeof(cbuf));
			if (n > 0)
				data.append(cbuf, n);
		}
	}


	evhttp_request * BaseHttpSvr::GetCurReq()
	{
		return m_cur_req;
	}


	void BaseHttpSvr::AddHeader(const std::string &name, const std::string &value)
	{
		struct evkeyvalq* header = evhttp_request_get_output_headers(GetCurReq());
		evhttp_add_header(header, name.c_str(), value.c_str());
	}

	void BaseHttpSvr::Send(const std::string &data /*= ""*/, int code/*= HTTP_OK*/, const char *reason/*="ok"*/)
	{
		if (nullptr == m_cur_req)
		{
			LB_FATAL("error call, repeated call ReplyError or Reply");//ReplyError 或者 Reply 调用两次或者
			return;
		}
		if (HTTP_OK != code)
		{
			evhttp_send_error(m_cur_req, code, reason); //释放资源
		}
		else
		{
			//创建要使用的buffer对象
			evbuffer* buf = evbuffer_new();
			evbuffer_add_printf(buf, "%s", data.c_str());
			evhttp_send_reply(m_cur_req, code, reason, buf);//释放资源
			evbuffer_free(buf);
		}
		m_cur_req = nullptr;
	}



	void BaseHttpSvr::RevRequestCB(struct evhttp_request* req, void* arg)
	{
		BaseHttpSvr *p = (BaseHttpSvr *)arg;
		if (nullptr != p->m_cur_req)
		{
			LB_FATAL("nullptr != p->m_tmp_req");
			return;
		}
		p->m_cur_req = req;
		p->RevRequest();//里面必须调用 evhttp_send_error 或者 evhttp_send_reply 释放资源
		if (nullptr != p->m_cur_req) //用户没调用释放资源函数
		{
			evhttp_send_error(p->m_cur_req, HTTP_SERVUNAVAIL, "svr have error code");
			p->m_cur_req = nullptr;
			LB_FATAL("miss call BaseHttpSvr::Send in RevRequest function");
		}
	}

	BaseHttpSvr::URI_PRT BaseHttpSvr::safe_evhttp_uri_parse(const char *uri) const
	{
		URI_PRT decoded(evhttp_uri_parse(uri), ::evhttp_uri_free);
		return decoded;
	}

	const char *BaseHttpSvr::GetUri() const
	{
		if (nullptr == m_cur_req)
		{
			LB_ERROR("nullptr == p->m_tmp_req");//必须在RevRequest里面，响应前调用，
			return nullptr;
		}
		const char *uri = evhttp_request_get_uri(m_cur_req);
		return uri;
	}



	BaseHttpClient::~BaseHttpClient()
	{
		Free();
	}

	namespace
	{
		void
			nouse(struct evhttp_uri *uri)
		{}
	}
	bool BaseHttpClient::Request(const char *url, evhttp_cmd_type cmd_type, const char *post_data, unsigned int ot_sec)
	{
		LB_COND_F(!m_is_req, "repeated request");
		LB_COND_F(url);
		LB_COND_F(nullptr == m_con);
		LB_COND_F(nullptr == m_dnsbase);

		const char* host = nullptr;
		int port = 0;
		char uri[256];
		//解析url
		////////////////////////////////////
		using URI_PRT = std::unique_ptr<evhttp_uri, decltype(&::evhttp_uri_free)>;
		URI_PRT uri_obj(evhttp_uri_parse(url), ::evhttp_uri_free);
		{

			LB_COND_F(uri_obj.get(), "Parse url failed! ");
			host = evhttp_uri_get_host(uri_obj.get());
			LB_COND_F(host, "Parse host failed! url=%s", url);
			port = evhttp_uri_get_port(uri_obj.get());
			if (port < 0) port = 80;
			const char* path = evhttp_uri_get_path(uri_obj.get());
			if (strlen(path) == 0)
			{
				path = "/";
			}
			const char *query = evhttp_uri_get_query(uri_obj.get());
			if (query == NULL) {
				snprintf(uri, sizeof(uri) - 1, "%s", path);
			}
			else {
				snprintf(uri, sizeof(uri) - 1, "%s?%s", path, query);
			}
			uri[sizeof(uri) - 1] = '\0';
			//LB_DEBUG("host=%s port=%d", host, port);
		}
		/////////////////////////////////////////

		// 初始化evdns_base_new
		m_dnsbase = evdns_base_new(EventMgr::Ins().GetEventBase(), EVDNS_BASE_INITIALIZE_NAMESERVERS);
		if (!m_dnsbase)
		{
			LB_ERROR("host=%s port=%d", host, port);
			LB_ERROR("Create dns base failed!");
			Free();
			return false;
		}

		m_con = evhttp_connection_base_new(EventMgr::Ins().GetEventBase(), m_dnsbase, host, port);
		if (!m_con)
		{
			LB_ERROR("host=%s port=%d", host, port);
			LB_ERROR("Create evhttp connection failed!");
			Free();
			return false;
		}
		evhttp_connection_set_timeout(m_con, ot_sec);  //设置超时
		// 创建连接对象成功后, 设置关闭回调函数
		evhttp_connection_set_closecb(m_con, connection_close_callback, this);
		// 创建evhttp_request对象，设置返回状态响应的回调函数
		struct evhttp_request* req = evhttp_request_new(remote_read_callback, this);//request不用自己evhttp_request_free, 交给m_con管理

		struct evkeyvalq* header = evhttp_request_get_output_headers(req);
		evhttp_add_header(header, "Host", host);
		//evhttp_add_header(header, "Content-type", "application/json");

		if (nullptr != post_data)
		{
			evbuffer_add(req->output_buffer, post_data, strlen(post_data));
			char buf[1024];
			evutil_snprintf(buf, sizeof(buf) - 1, "%lu", strlen(post_data));
			evhttp_add_header(header, "Content-Length", buf);
		}

		// 发起http请求
		evhttp_make_request(m_con, req, cmd_type, uri); //调用后，m_connection管理req的释放

		//LB_DEBUG("request ok");
		m_is_req = true;
		return true;
	}


	void BaseHttpClient::Free()
	{
		if (nullptr != m_dnsbase)
		{
			evdns_base_free(m_dnsbase, true);
			m_dnsbase = nullptr;
		}

		if (nullptr != m_con)
		{
			evhttp_connection_free(m_con);
			m_con = nullptr;
		}
		m_is_req = false;
	}

	void BaseHttpClient::remote_read_callback(struct evhttp_request* req, void* arg)
	{
		LB_COND_VOID(req);
		BaseHttpClient *p = (BaseHttpClient *)arg;
		LB_COND_VOID(p);
		if (0 == req->response_code)
		{	//超时没响应，或者连接失败

			p->Free();
			p->ConnectFail();
			return;
		}
		
		int start_num = req->response_code / 100;
		if (start_num == 4 || start_num == 5) //400,500开头的
		{
			auto code = req->response_code;//不确定Free后是否有效呀。所以先复制
			string code_line = req->response_code_line;
			p->Free();
			p->RespondError(code, code_line.c_str());
			return;
		}

		char buffer[1000];
		string data;
		int nread = 0;
		LB_DEBUG("rev bytes size %lu code=%d", evbuffer_get_length(req->input_buffer), req->response_code);

		while ((nread = evbuffer_remove(req->input_buffer, buffer, sizeof(buffer))) > 0) {
			data.append(buffer, nread);
		}
		p->Free();
		p->Respond(data.c_str());
	}

	void BaseHttpClient::connection_close_callback(struct evhttp_connection* connection, void* arg)
	{
		LB_DEBUG("connection_close_callback");
		BaseHttpClient *p = (BaseHttpClient *)arg;
		if (p->m_con != connection)
		{
			LB_ERROR("p->m_connection != connection %llu %llu", p->m_con, connection);
		}
	}
}//namespace lc //libevent cpp