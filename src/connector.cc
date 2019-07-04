
#include "lev_mgr.h"
#include <stddef.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>  
#include "log_file.h" 
#include "connector.h"
#include "include_all.h"

using namespace std;
namespace lc //libevent cpp
{
ConnectCom::ConnectCom()
	:m_buf_e(nullptr)
	, m_fd(0)
	, m_msbs(MAX_MAX_SEND_BUF_SIZE)
	, m_is_connect(false)
	, m_msg_write_len(0)
	, m_no_ev_cb_log(false)
{
	memset(&m_addr, 0, sizeof(m_addr));
}

ConnectCom::~ConnectCom()
{
	free();
}

void ConnectCom::free()
{
	if (m_buf_e)
	{
		bufferevent_free(m_buf_e);
		//LOG_DEBUG("bufferevent_free(m_buf_e)");
		m_buf_e = 0;
	}
	if (m_fd != 0)
	{

		//释放m_buf_e，的时候，库里面会释放m_fd
		m_fd = 0;
	}
	m_is_connect = false;
}


bool ConnectCom::SetSocketInfo(bufferevent* buf_e, evutil_socket_t fd, struct sockaddr* sa)
{
	if (nullptr == buf_e)
	{
		return false;
	}
	if (nullptr != m_buf_e)
	{
		return false;
	}

	if (nullptr != sa)
	{
		memcpy(&m_addr, (sockaddr_in*)sa, sizeof(m_addr));
	}
	m_buf_e = buf_e;
	m_fd = fd;
	return true;
}

void ConnectCom::SetAddr(const char* connect_ip, unsigned short connect_port)
{
	memset(&m_addr, 0, sizeof(m_addr));
	m_addr.sin_family = AF_INET;
	m_addr.sin_addr.s_addr = inet_addr(connect_ip);
	m_addr.sin_port = htons(connect_port);
}

void ConnectCom::SetAddr(const sockaddr_in &svr_addr)
{
	memset(&m_addr, 0, sizeof(m_addr));
	m_addr = svr_addr;
}


void ConnectCom::DisConnect()
{
	free();
	on_disconnected();
}

void ConnectCom::writecb(struct bufferevent* bev, void* user_data)
{
	((ConnectCom*)user_data)->conn_write_callback(bev);
}

void ConnectCom::eventcb(struct bufferevent* bev, short events, void* user_data)
{
	((ConnectCom*)user_data)->conn_event_callback(bev, events);
}

void ConnectCom::readcb(struct bufferevent* bev, void* user_data)
{
	((ConnectCom*)user_data)->conn_read_callback(bev);
}

void ConnectCom::conn_write_callback(bufferevent* bev)
{
	//如果不用 考虑删掉，
}

bool ConnectCom::IsWaitCompleteMsg() const
{
	return 0 != m_msg_write_len;
}

void ConnectCom::conn_read_callback(bufferevent* bev)
{
	const static int HEAD_LEN = sizeof(MsgPack::len);
	int input_len = evbuffer_get_length(bufferevent_get_input(bev));

	while (input_len > 0)
	{
		char *write_addr = ((char *)&m_msg) + m_msg_write_len;
		//根据2种状态去做不同读取操作
		//状态1, msg.len没读完整,等待读完整过程
		if (m_msg_write_len < HEAD_LEN)
		{
			int need_to_read = HEAD_LEN - m_msg_write_len;
			int write_len = min(input_len, need_to_read);
			int ret_write_len = bufferevent_read(bev, write_addr, write_len);
			if (ret_write_len != write_len)
			{
				LIB_LOG_FATAL("ret_write_len != write_len");
				DisConnect();
				return;
			}
			input_len -= ret_write_len;
			m_msg_write_len += ret_write_len;

		//	LIB_LOG_DEBUG("bufferevent_read, len m_msg_write_len =%d %d", ntohs(m_msg.len), m_msg_write_len);
			continue;
		}
		//状态2, msg.len完整，等待读取完整消息
		else
		{
			m_msg.len = ntohs(m_msg.len);
			if (m_msg.len > MAX_MSG_DATA_LEN) //包过大，断开连接
			{
				LIB_LOG_ERROR("rev msg len too big. %d", m_msg.len);
				DisConnect();
				return;
			}

			int need_to_read = m_msg.len + HEAD_LEN - m_msg_write_len;
			if (need_to_read < 0)
			{
				LIB_LOG_FATAL("need_to_read < 0");
				DisConnect();
				return;
			}
			int write_len = min(input_len, need_to_read);
			int ret_write_len = bufferevent_read(bev, write_addr, write_len);
			if (ret_write_len != write_len)
			{
				LIB_LOG_FATAL("ret_write_len != write_len");
				DisConnect();
				return;
			}
			input_len -= ret_write_len;
			m_msg_write_len += ret_write_len;

			//LIB_LOG_DEBUG("bufferevent_read data, m_msg_write_len write_addr[0]=%d %d %d %d %d", m_msg_write_len, write_addr[0], write_addr[1], write_addr[2], write_addr[3]);
			if (need_to_read == ret_write_len)// 接收完整
			{
				OnRecv(m_msg);
				//重置m_msg,等下次接收新消息
				m_msg.len = 0;
				m_msg_write_len = 0;
			}
			continue;
		}
	}
}


void ConnectCom::conn_event_callback(bufferevent* bev, short events)
{
	if (events & BEV_EVENT_CONNECTED)
	{
		m_is_connect = true;
		OnConnected();
	}
	else
	{
		if (false == m_no_ev_cb_log)
		{
			if (events & BEV_EVENT_EOF)
			{
				LIB_LOG_DEBUG("event cb:connection closed.\n");
			}
			else if (events & BEV_EVENT_ERROR)
			{
				LIB_LOG_DEBUG("event cb:got an error on the connection: %s\n", strerror(errno));
			}
			else if (events & BEV_EVENT_READING)
			{

			}
			else if (events & BEV_EVENT_WRITING)
			{

			}
			else if (events & BEV_EVENT_TIMEOUT)
			{

			}
		}
		OnError(events);
		DisConnect();
		return; //这里本对象可能已经销毁，别再引用
	}
}

bool ConnectCom::send_data(const MsgPack &msg)
{
	if (!m_is_connect)
	{
		LIB_LOG_ERROR("is disconnect");
		return false;
	}
	const char* data = (const char*)&(msg.data);
	uint16 net_len = htons((int)msg.len);
	if (0 == m_fd)
	{
		LIB_LOG_ERROR("BaseConnectCom not init. 0 == m_fd");
		return false;
	}
	if (!m_buf_e)
	{
		LIB_LOG_ERROR("BaseConnectCom not init !m_buf_e");
		return false;
	}

	if (m_msbs != 0)
	{
		struct evbuffer* output = bufferevent_get_output(m_buf_e);
		if (!output)
		{
			LIB_LOG_FATAL("unknow error");
			return false;
		}
		size_t output_len = evbuffer_get_length(output);
		if (output_len > m_msbs)
		{
			LIB_LOG_ERROR("too much bytes wait to send %ld", output_len);
			DisConnect();
			return false;
		}
	}

	//LIB_LOG_DEBUG("bufferevent_write %d", net_len);
	if (0 != bufferevent_write(m_buf_e, &net_len, sizeof(net_len)))
	{
		LIB_LOG_ERROR("bufferevent_write fail, len=%d", msg.len + sizeof(msg.len));
		return false;
	}
	if (0 != bufferevent_write(m_buf_e, data, msg.len))
	{
		LIB_LOG_ERROR("bufferevent_write fail, len=%d", msg.len + sizeof(msg.len));
		return false;
	}
	//LIB_LOG_DEBUG("bufferevent_write data[0] =%d %d %d %d", data[0], data[1], data[2], data[3]);
	return true;
}

bool ConnectCom::send_data_no_head(const char* data, int len)
{
	if (!m_is_connect)
	{
		LIB_LOG_ERROR("is disconnect");
		return false;
	}
	if (0 == m_fd)
	{
		LIB_LOG_ERROR("BaseConnectCom not init. 0 == m_fd");
		return false;
	}
	if (!m_buf_e)
	{
		LIB_LOG_ERROR("BaseConnectCom not init !m_buf_e");
		return false;
	}
	if (nullptr == data)
	{
		LIB_LOG_ERROR("nullptr == data\n");
		return false;
	}

	if (m_msbs != 0)
	{
		struct evbuffer* output = bufferevent_get_output(m_buf_e);
		if (!output)
		{
			LIB_LOG_FATAL("unknow error");
			return false;
		}
		size_t output_len = evbuffer_get_length(output);
		if (output_len > m_msbs)
		{
			LIB_LOG_ERROR("too much bytes for send %ld", output_len);
			DisConnect();
			return false;
		}
	}

	//LIB_LOG_DEBUG("write data[0]=%d %d %d", data[0], data[1], data[2]);
	if (0 != bufferevent_write(m_buf_e, data, len))
	{
		LIB_LOG_ERROR("bufferevent_write fail, len=%d", len);
		return false;
	}
	return true;
}
void ConnectCom::setwatermark(short events, unsigned int lowmark, unsigned int highmark)
{
	if (0 == m_fd)
	{
		LIB_LOG_ERROR("BaseConnectCom not init. 0 == m_fd");
		return;
	}
	if (!m_buf_e)
	{
		LIB_LOG_ERROR("BaseConnectCom not init !m_buf_e");
		return;
	}
	bufferevent_setwatermark(m_buf_e, events, lowmark, highmark);
}

void ConnectCom::GetRemoteAddr(std::string &ip, unsigned short &port) const
{
	ip = inet_ntoa(m_addr.sin_addr);
	port = ntohs(m_addr.sin_port);
}

/////////////////////////////////////////////////////////


bool BaseClientCon::ConnectInit(const char* connect_ip, unsigned short connect_port)
{
	if (0 != GetFd())
	{
		LIB_LOG_ERROR("repeated init");
		return false;
	}
	if (nullptr == connect_ip)
	{
		LIB_LOG_ERROR("nullptr == connect_ip");
		return false;
	}


	SetAddr(connect_ip, connect_port);
	return ConnectByAddr();
}

bool BaseClientCon::ConnectInit(const sockaddr_in &svr_addr)
{
	if (0 != GetFd())
	{
		LIB_LOG_ERROR("repeated init");
		return false;
	}

	SetAddr(svr_addr);
	return ConnectByAddr();
}


bool BaseClientCon::ConnectByAddr()
{
	sockaddr_in addr = GetRemoteAddr();
	if (0 == addr.sin_port)
	{
		LIB_LOG_ERROR("GetRemoteAddr() don't init");
		return false;
	}
	evutil_socket_t fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd < 0)
	{
		LIB_LOG_ERROR("m_fd < 0");
		return false;
	}
	bufferevent* buf_e = bufferevent_socket_new(LibEventMgr::Obj().GetEventBase(), fd, BEV_OPT_CLOSE_ON_FREE);//提示你提供给bufferevent_socket_new() 的套接字务必是非阻塞模式, 为此LibEvent 提供了便利的方法	evutil_make_socket_nonblocking.
	if (nullptr == buf_e)
	{
		LIB_LOG_ERROR("nullptr == m_buf_e");
		if (0 != ::close(fd))
		{
			LIB_LOG_ERROR("::close fail , fd=%d", fd);
		}
		fd = 0;
		return false;
	}
	bufferevent_setcb(buf_e, readcb, nullptr, eventcb, this);
	bufferevent_enable(buf_e, EV_WRITE | EV_READ);
	
	if (bufferevent_socket_connect(buf_e, (struct sockaddr*)&addr, sizeof(addr)) == 0)//连接失败会里面关闭fd
	{
		SetSocketInfo(buf_e, fd);
		return true;
	}
	LIB_LOG_ERROR("bufferevent_socket_connect fail"); //这里没跑过，不知道什么情况才跑
	return false;
}

bool BaseClientCon::TryReconnect()
{
	if (!IsConnect())
	{
		return ConnectByAddr();
	}
	else
	{
		return true; //不需要重连
	}
}
}//namespace lc //libevent cpp