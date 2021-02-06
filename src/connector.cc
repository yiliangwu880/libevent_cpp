
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
ConCom::ConCom()
	:m_buf_e(nullptr)
	, m_fd(0)
	, m_msbs(MAX_MAX_SEND_BUF_SIZE)
	, m_is_connect(false)
	, m_no_ev_cb_log(false)
{
	memset(&m_addr, 0, sizeof(m_addr));
}

ConCom::~ConCom()
{
	Free();
}


void ConCom::SetRawReadCb(bool isRawCb)
{
	m_isRawReadCb = isRawCb;
}

bool ConCom::IsWaitConnectReq() const
{
	return m_fd == 0;
}

void ConCom::Free()
{
	if (m_buf_e)
	{
		bufferevent_free(m_buf_e);
		//LB_TRACE("bufferevent_free m_buf_e = 0");
		m_buf_e = 0;
	}
	if (m_fd != 0)
	{
		//if (0 != ::close(m_fd))
		//{
		//	LB_ERROR("::close fail , fd=%d", m_fd);
		//}
		//释放m_buf_e，的时候，库里面会释放m_fd
		m_fd = 0;
	}
	m_is_connect = false;
	m_rev_pack_len = 0;
}


bool ConCom::SetSocketInfo(bufferevent* buf_e, evutil_socket_t fd, struct sockaddr* sa)
{
	if (nullptr == buf_e)
	{
		LB_ERROR("SetSocketInfo fail. nullptr == buf_e");
		return false;
	}
	if (nullptr != m_buf_e)
	{
		LB_ERROR("SetSocketInfo fail. nullptr != m_buf_e");
		return false;
	}

	if (nullptr != sa)
	{
		memcpy(&m_addr, (sockaddr_in*)sa, sizeof(m_addr));
	}
	m_buf_e = buf_e;
	m_fd = fd;
	//LB_TRACE("SetSocketInfo OK");
	return true;
}

void ConCom::SetAddr(const char* connect_ip, unsigned short connect_port)
{
	memset(&m_addr, 0, sizeof(m_addr));
	m_addr.sin_family = AF_INET;
	m_addr.sin_addr.s_addr = inet_addr(connect_ip);
	m_addr.sin_port = htons(connect_port);
}

void ConCom::SetAddr(const sockaddr_in &svr_addr)
{
	memset(&m_addr, 0, sizeof(m_addr));
	m_addr = svr_addr;
}


void ConCom::DisConnect()
{
	if (m_fd == 0)
	{
		//LB_DEBUG("already disconnect. not need to disconnect.");
		return;
	}
	Free();
	OnDisconnected();
}



void ConCom::writecb(struct bufferevent* bev, void* user_data)
{
	((ConCom*)user_data)->conn_write_callback(bev);
}

void ConCom::eventcb(struct bufferevent* bev, short events, void* user_data)
{
	((ConCom*)user_data)->conn_event_callback(bev, events);
}



void ConCom::readcb(struct bufferevent* bev, void* user_data)
{
	ConCom* p = (ConCom*)user_data;
	if (p->m_isRawReadCb)
	{
		evbuffer *buf = bufferevent_get_input(bev);
		int input_len = evbuffer_get_length(buf);
		char *pMsg = (char*)evbuffer_pullup(buf, input_len);
		int readLen = p->OnRawRecv(pMsg, input_len);
		evbuffer_drain(buf, readLen); //删除已处理内存
		return;
	}
	p->conn_read_callback_no_cp(bev);
}

void ConCom::conn_write_callback(bufferevent* bev)
{
	//如果不用 考虑删掉，
}





void ConCom::conn_read_callback_no_cp(bufferevent* bev)
{
	const static int HEAD_LEN = sizeof(MsgPack::len);
	evbuffer *buf = bufferevent_get_input(bev);

	while (true)
	{
		//开始 流程
		int input_len = evbuffer_get_length(buf);
		if (input_len < HEAD_LEN)//头没接收完整
		{
			return;
		}
		
		//case >= HEAD_LEN
		if (0 == m_rev_pack_len) //消息头 长度没处理
		{
			uint16 *mem = (uint16 *)evbuffer_pullup(buf, sizeof(uint16));
			*mem = ntohs(*mem);
			m_rev_pack_len = *mem;
		}

		if (m_rev_pack_len > MAX_MSG_DATA_LEN || 0 == m_rev_pack_len) //包过大，断开连接
		{
			LB_ERROR("rev msg len too big. %d。 remote addr: %s %d", m_rev_pack_len, GetRemoteIp(), GetRemotePort());
			DisConnect();
			return;
		}

		int finish_pack_len = m_rev_pack_len + HEAD_LEN;//完整包长度

		if (input_len < finish_pack_len)// 接收不完整
		{
			return; //退出处理，等网络接收更多字节,再继续
		}

		//case have a finish pack
		MsgPack *pMsg = (MsgPack *)evbuffer_pullup(buf, finish_pack_len);
		if (nullptr == pMsg)
		{
			LB_ERROR("evbuffer_pullup fail, pack_len=%d, buf len=%d", finish_pack_len, evbuffer_get_length(buf));
			DisConnect();
			return;
		}
		OnRecv(*pMsg);
		//OnRecv期间断开了连接，缓存部分丢弃处理
		if (IsWaitConnectReq())
		{
			return;
		}
		evbuffer_drain(buf, finish_pack_len); //删除已处理内存
		//重置
		m_rev_pack_len = 0;
		continue;
	}
}
void ConCom::conn_event_callback(bufferevent* bev, short events)
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
				//LB_DEBUG("event cb:connection closed.\n");
			}
			else if (events & BEV_EVENT_ERROR)
			{
				//LB_DEBUG("event cb:got an error on the connection: %s\n", strerror(errno));
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
		//LB_TRACE("--------------------------------------------CONNECT ERROR");
		OnError(events);
		DisConnect(); //里面的回调函数，可以操作delete对象
		return; //这里本对象可能已经销毁，别再引用
	}
}
bool ConCom::SendPack(const char* data, uint16 len)
{
	B_COND(len, false);
	B_COND(data, false);
	B_COND(m_is_connect, false);
	//LB_COND(m_is_connect, false, "is disconnect.");
	if (len > MAX_MSG_DATA_LEN) //包过大，断开连接
	{
		LB_ERROR("send msg len too big. %d。 remote addr: %s %d", len, GetRemoteIp(), GetRemotePort());
		return false;
	}

	if (0 == m_fd)
	{
		LB_DEBUG("BaseConnectCom not init. 0 == m_fd");
		return false;
	}
	if (!m_buf_e)
	{
		LB_DEBUG("BaseConnectCom not init !m_buf_e");
		return false;
	}

	uint16 net_len = htons((int)len);

	if (m_msbs != 0)
	{
		struct evbuffer* output = bufferevent_get_output(m_buf_e);
		if (!output)
		{
			LB_FATAL("unknow error");
			return false;
		}
		size_t output_len = evbuffer_get_length(output);
		if (output_len > m_msbs)
		{
			LB_ERROR("too much bytes wait to send %ld", output_len);
			DisConnect();
			return false;
		}
	}

	//LB_DEBUG("bufferevent_write %d", net_len);
	if (0 != bufferevent_write(m_buf_e, &net_len, sizeof(net_len)))
	{
		LB_ERROR("bufferevent_write fail, len=%d", len + sizeof(len));
		return false;
	}
	if (0 != bufferevent_write(m_buf_e, data, len))
	{
		LB_ERROR("bufferevent_write fail, len=%d", len + sizeof(len));
		return false;
	}
	//LB_DEBUG("bufferevent_write data[0] =%d %d %d %d", data[0], data[1], data[2], data[3]);
	return true;
}


bool ConCom::SendPack(const string &msg)
{
	return SendPack(msg.c_str(), msg.length());
}

bool ConCom::SendData(const MsgPack &msg)
{
	B_COND(m_is_connect, false);
	//LB_COND(m_is_connect, false, "is disconnect.");
	if (msg.len > MAX_MSG_DATA_LEN) //包过大，断开连接
	{
		LB_ERROR("send msg len too big. %d。 remote addr: %s %d", msg.len, GetRemoteIp(), GetRemotePort());
		return false;
	}

	if (0 == m_fd)
	{
		LB_DEBUG("BaseConnectCom not init. 0 == m_fd");
		return false;
	}
	if (!m_buf_e)
	{
		LB_DEBUG("BaseConnectCom not init !m_buf_e");
		return false;
	}

	const char* data = (const char*)&(msg.data);
	uint16 net_len = htons((int)msg.len);

	if (m_msbs != 0)
	{
		struct evbuffer* output = bufferevent_get_output(m_buf_e);
		if (!output)
		{
			LB_FATAL("unknow error");
			return false;
		}
		size_t output_len = evbuffer_get_length(output);
		if (output_len > m_msbs)
		{
			LB_ERROR("too much bytes wait to send %ld", output_len);
			DisConnect();
			return false;
		}
	}

	//LB_DEBUG("bufferevent_write %d", net_len);
	if (0 != bufferevent_write(m_buf_e, &net_len, sizeof(net_len)))
	{
		LB_ERROR("bufferevent_write fail, len=%d", msg.len + sizeof(msg.len));
		return false;
	}
	if (0 != bufferevent_write(m_buf_e, data, msg.len))
	{
		LB_ERROR("bufferevent_write fail, len=%d", msg.len + sizeof(msg.len));
		return false;
	}
	//LB_DEBUG("bufferevent_write data[0] =%d %d %d %d", data[0], data[1], data[2], data[3]);
	return true;
}

bool ConCom::SendData(const char* data, int len)
{
	if (!m_is_connect)
	{
		LB_ERROR("is disconnect");
		return false;
	}
	if (0 == m_fd)
	{
		LB_ERROR("BaseConnectCom not init. 0 == m_fd");
		return false;
	}
	if (!m_buf_e)
	{
		LB_ERROR("BaseConnectCom not init !m_buf_e");
		return false;
	}
	if (nullptr == data)
	{
		LB_ERROR("nullptr == data\n");
		return false;
	}

	if (m_msbs != 0)
	{
		struct evbuffer* output = bufferevent_get_output(m_buf_e);
		if (!output)
		{
			LB_FATAL("unknow error");
			return false;
		}
		size_t output_len = evbuffer_get_length(output);
		if (output_len > m_msbs)
		{
			LB_ERROR("too much bytes for send %ld", output_len);
			DisConnect();
			return false;
		}
	}

	//LB_DEBUG("write data[0]=%d %d %d", data[0], data[1], data[2]);
	if (0 != bufferevent_write(m_buf_e, data, len))
	{
		LB_ERROR("bufferevent_write fail, len=%d", len);
		return false;
	}
	return true;
}



int ConCom::Flush()
{
	LB_ERROR("unfinished function flush");
	//都不能马上发送缓冲消息，以后解决
	//bufferevent_flush(m_buf_e, EV_READ | EV_WRITE, BEV_NORMAL);
	//bufferevent_trigger(m_buf_e, EV_READ | EV_WRITE, BEV_TRIG_IGNORE_WATERMARKS);
	//bufferevent_flush(m_buf_e, EV_READ | EV_WRITE, BEV_NORMAL);
	return 0;
	//return bufferevent_flush(m_buf_e, EV_READ | EV_WRITE, BEV_NORMAL);
}

void ConCom::Setwatermark(short events, unsigned int lowmark, unsigned int highmark)
{
	B_COND_VOID(m_is_connect);
	if (0 == m_fd)
	{
		LB_ERROR("BaseConnectCom not init. 0 == m_fd");
		return;
	}
	if (!m_buf_e)
	{
		LB_ERROR("BaseConnectCom not init !m_buf_e");
		return;
	}
	bufferevent_setwatermark(m_buf_e, events, lowmark, highmark);
}

void ConCom::GetRemoteAddr(std::string &ip, unsigned short &port) const
{
	ip = inet_ntoa(m_addr.sin_addr);
	port = ntohs(m_addr.sin_port);
}


const char *ConCom::GetRemoteIp() const
{
	return inet_ntoa(m_addr.sin_addr);
}

uint16 ConCom::GetRemotePort() const
{
	return ntohs(m_addr.sin_port);
}

/////////////////////////////////////////////////////////


bool ClientCon::ConnectInit(const char* connect_ip, unsigned short connect_port)
{
	if (0 != GetFd())
	{
		LB_ERROR("repeated init");
		return false;
	}
	if (nullptr == connect_ip || 0 == connect_port)
	{
		LB_ERROR("nullptr == connect_ip || 0 == connect_port, error para");
		return false;
	}


	SetAddr(connect_ip, connect_port);
	return ConnectByAddr();
}

bool ClientCon::ConnectInit(const sockaddr_in &svr_addr)
{
	if (0 != GetFd())
	{
		LB_ERROR("repeated init");
		return false;
	}

	SetAddr(svr_addr);
	return ConnectByAddr();
}


bool ClientCon::ConnectByAddr()
{
	sockaddr_in addr = GetRemoteAddr();
	if (0 == addr.sin_port)
	{
		LB_ERROR("GetRemoteAddr() don't init");
		return false;
	}
	evutil_socket_t fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd < 0)
	{
		LB_ERROR("socket create fail. fd < 0");
		return false;
	}
	bufferevent* buf_e = bufferevent_socket_new(EventMgr::Ins().GetEventBase(), fd, BEV_OPT_CLOSE_ON_FREE);//提示你提供给bufferevent_socket_new() 的套接字务必是非阻塞模式, 为此LibEvent 提供了便利的方法	evutil_make_socket_nonblocking.
	if (nullptr == buf_e)
	{
		LB_ERROR("nullptr == m_buf_e");
		if (0 != ::close(fd))
		{
			LB_ERROR("::close fail , fd=%d", fd);
		}
		fd = 0;
		return false;
	}
	bufferevent_setcb(buf_e, readcb, nullptr, eventcb, this);
	bufferevent_enable(buf_e, EV_WRITE | EV_READ);
	
	if (0 != bufferevent_socket_connect(buf_e, (struct sockaddr*)&addr, sizeof(addr)))//连接失败会里面关闭fd
	{
		//失败情况：
		//地址不对
		//客户端连接8W左右，跑了这里, 应该是fd太多了，超出系统设置
		LB_ERROR("bufferevent_socket_connect fail. addr=%s %d", GetRemoteIp(), GetRemotePort());
		return false;
	}
	bool ret= SetSocketInfo(buf_e, fd);
	if (!ret)
	{
		LB_ERROR("SetSocketInfo fail");
		bufferevent_free(buf_e);
		return false;
	}

	return true;
}

bool ClientCon::TryReconnect()
{
	if(IsWaitConnectReq())
	{
		return ConnectByAddr();
	}
	else
	{
		//LB_TRACE("no need TryReconnect");
		return false; //不需要重连
	}
}

//bool MsgPack::Serialize(const std::string &s)
//{
//	if (s.length() >= sizeof(data))
//	{
//		LB_ERROR("pack is too big. size=%d", s.length());
//		return false;
//	}
//	len = s.length();
//	memcpy(data, s.c_str(), s.length());
//	return true;
//}

bool MsgPack::Set(const std::string &s)
{
	if (s.length() >= sizeof(data))
	{
		LB_ERROR("pack is too big. size=%d", s.length());
		return false;
	}
	len = s.length();
	memcpy(data, s.c_str(), s.length());
	return true;
}

}//namespace lc //libevent cpp