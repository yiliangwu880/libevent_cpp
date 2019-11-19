
#include "http.h"
#include <stddef.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>  
#include "include_all.h"
#include "log_file.h" //目前调试用，移植删掉
#include <memory>

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
		m_evhttp = evhttp_new(EventMgr::Obj().GetEventBase());
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

	const char * BaseHttpSvr::GetUriQuery() const
	{
		if (nullptr == m_cur_req)
		{
			LB_ERROR("invaild req, on GetUriQuery");
			return nullptr;
		}
		const char *uri = evhttp_request_get_uri(m_cur_req);
		if (nullptr == uri)
		{
			LB_ERROR("no uri");
			return nullptr;
		}

		/* Decode the URI */
		struct evhttp_uri *decoded = evhttp_uri_parse(uri);
		if (!decoded) {
			LB_ERROR("It's not a good URI. Sending BADREQUEST\n");
			return nullptr;
		}
		//解析URL中method参数
		const char *query_part = evhttp_uri_get_query(decoded);
		if (!query_part)
		{
			return nullptr;
		}
		return query_part;
	}


	const char * BaseHttpSvr::GetPath() const
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
		struct evhttp_uri *decoded = evhttp_uri_parse(uri);
		if (!decoded) {
			LB_ERROR("It's not a good URI. Sending BADREQUEST\n");
			return nullptr;
		}
		const char *path = evhttp_uri_get_path(decoded);
		if (nullptr == path)
		{
			return "/";
		}
		return path;
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

	void BaseHttpSvr::Send(int code, const char *reason, const std::string str/*=""*/)
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
			evbuffer_add_printf(buf, "%s", str.c_str());
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


	bool BaseHttpClient::Request(const char *url, evhttp_cmd_type cmd_type, unsigned int ot_sec, const char *post_data)
	{
		LB_COND_F(!m_is_req, "repeated request");
		LB_COND_F(url);
		LB_COND_F(nullptr == http_uri);
		LB_COND_F(nullptr == m_con);
		LB_COND_F(nullptr == m_dnsbase);

		const char* host = nullptr;
		int port = 0;
		char uri[256];
		//解析url
		////////////////////////////////////
		{
			http_uri = evhttp_uri_parse(url); //自动释放资源
			LB_COND_F(http_uri, "Parse url failed! ");
			host = evhttp_uri_get_host(http_uri);
			LB_COND_F(host, "Parse host failed! url=%s", url);
			port = evhttp_uri_get_port(http_uri);
			if (port < 0) port = 80;
			const char* path = evhttp_uri_get_path(http_uri);
			if (strlen(path) == 0)
			{
				path = "/";
			}
			const char *query = evhttp_uri_get_query(http_uri);
			if (query == NULL) {
				snprintf(uri, sizeof(uri) - 1, "%s", path);
			}
			else {
				snprintf(uri, sizeof(uri) - 1, "%s?%s", path, query);
			}
			uri[sizeof(uri) - 1] = '\0';
			LB_DEBUG("host=%s port=%d", host, port);
		}
		/////////////////////////////////////////

		// 初始化evdns_base_new
		m_dnsbase = evdns_base_new(EventMgr::Obj().GetEventBase(), EVDNS_BASE_INITIALIZE_NAMESERVERS);
		if (!m_dnsbase)
		{
			LB_ERROR("Create dns base failed!");
			Free();
			return false;
		}

		m_con = evhttp_connection_base_new(EventMgr::Obj().GetEventBase(), m_dnsbase, host, port);
		if (!m_con)
		{
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

		//LOG_DEBUG("request ok");
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
		if (nullptr != http_uri)
		{
			evhttp_uri_free(http_uri);
			http_uri = nullptr;
		}
		m_is_req = false;
	}

	namespace
	{
		static const int MAX_LEN = 1024 * 100;
	}
	void BaseHttpClient::remote_read_callback(struct evhttp_request* req, void* arg)
	{
		BaseHttpClient *p = (BaseHttpClient *)arg;
		if (req)
		{
			int start_num = req->response_code / 100;
			if (start_num != 4 && start_num != 5) //非400,500开头的
			{
				//LOG_DEBUG("remote_read_callback Code: %d %s", remote_rsp->response_code, remote_rsp->response_code_line);
				//LOG_DEBUG("replay:");
				char buffer[1000];
				string data;
				int nread = 0;
				LB_DEBUG("rev bytes size %lu", evbuffer_get_length(req->input_buffer));

				while ((nread = evbuffer_remove(req->input_buffer, buffer, sizeof(buffer))) > 0) {
					data.append(buffer, nread);
				}
				p->Respond(data.c_str());
			}
			else
			{
				p->RespondError(req->response_code, req->response_code_line);
			}
		}
		else
		{	//超时没响应，或者连接失败
			p->ConnectFail();
			//LOG_ERROR("remote_read_callback remote respond prt == NULL");
		}

		p->Free();
	}

	void BaseHttpClient::connection_close_callback(struct evhttp_connection* connection, void* arg)
	{
		BaseHttpClient *p = (BaseHttpClient *)arg;
		if (p->m_con != connection)
		{
			LB_ERROR("p->m_connection != connection %llu %llu", p->m_con, connection);
		}
	}
}//namespace lc //libevent cpp