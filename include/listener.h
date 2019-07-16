/*

*/

#pragma once

#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/listener.h>
#include <event2/util.h>
#include <event2/event.h>
#include <event2/event_struct.h>
#include <string.h>
#include <vector>
#include <map>
#include "connector.h"
#include <unistd.h>
#include <arpa/inet.h>  
#include "log_file.h" 
#include "timer_mgr.h"

namespace lc //libevent cpp
{
class SvrCon;
template<class >
class Listener;


//注意，由BaseConnectorMgr管理的 ListenerConnector不用用户代码调用delete.
//需要断开连接，调用 BaseConnectorMgr::CloseConnect(uint64 id);
class BaseConMgr
{
	template<class >
	friend class Listener;
	friend class SvrCon;
public:
	BaseConMgr();
	virtual ~BaseConMgr();
	//马上调用delete Connector
	bool PostDelConn(uint64 id);
	void OnTimerDelConn(); //真正delele对象
	//建议获取指针只做局部变量用，不要保存，因为BaseConnectorMgr管理ListenerConnector对象的删除
	SvrCon *FindConn(uint64 id);
	template<class CB>
	void Foreach(CB cb)
	{
		for (const auto &v : m_all_connector)
		{
			if (nullptr == v.second)
			{
				L_FATAL("save null ListenerConnector");
				continue;
			}
			(*cb)(v.second);
		}
	}

private:
	virtual SvrCon *NewConnect() = 0; //对象必须用new构建

	SvrCon *CreateConnectForListener();

private:
	const static uint32 DELTE_CONNECTOR_INTERVAL = 1000 * 1; //1sec
	std::map<uint64, SvrCon *> m_all_connector;
	std::vector<SvrCon *> m_vwdc; //vec wait delete connector
	Timer m_timer;
};

//管理listenner 接收的connector
template<class Connector>
class ConnectorMgr : public BaseConMgr
{
private:
	virtual SvrCon *NewConnect();

};

//管理服务器端链接， 远程端为客户端
//Listener 创建的connector
class SvrCon : public ConCom
{
public:
	SvrCon();
	~SvrCon();

	bool AcceptInit(evutil_socket_t fd, struct sockaddr* sa, const sockaddr_in &svr_addr);
	void SetCnMgr(BaseConMgr *mgr) { m_cn_mgr = mgr; };
	uint64 GetId() const { return m_id; }
	sockaddr_in GetSvrAddr() const { return m_svr_addr; }
	//断开连接，释放自己
	//注意，调用后，不会马上释放自己。
	//不会触发on_disconnected了
	bool FreeSelf();
private:
	virtual void OnRecv(const MsgPack &msg) override = 0;
	virtual void OnConnected() override = 0;
	virtual void on_disconnected() override final; //派生类不用继承这个函数,用onDisconnected处理被动断开连接
	virtual void onDisconnected() = 0;


private:
	BaseConMgr *m_cn_mgr;
	uint64 m_id;
	sockaddr_in m_svr_addr;
	bool m_ignore_free; //防多次调用FreeSelf函数用，支持 onDisconnected里面不小心写了调用FreeSelf()
};


class NoUseListenerConnector : public SvrCon
{
private:
	virtual void OnRecv(const MsgPack &msg) override {}
	virtual void OnConnected() override {}
	virtual void onDisconnected() override {}
};

//管理服务器
//类 成员只管理 m_listener m_addr， 其他链接管理交给 BaseConnectorMgr处理
//BaseConnectorMgr 实现分离出去，让Listener做更专注于网络逻辑,更简单化。具体管理器让用户选择自定义
//@para class Connector 必须为 ListenerConnector派生类
//有了缺省的NoUseListenerConnector， 你可以这样初始化 Listener<> listener(my_connector_mgr); listener.Init(server_port);

template<class Connector = NoUseListenerConnector>
class Listener 
{
public:
	//para cn_mgr 生存期必须比 Listener长
	Listener(BaseConMgr &cn_mgr)
		:m_listener(nullptr)
		, m_default_cn_mgr()
		, m_cn_mgr(cn_mgr)
	{
		memset(&m_addr, 0, sizeof(m_addr)); 
	}
	Listener()
		:m_listener(nullptr)
		, m_default_cn_mgr()
		, m_cn_mgr(m_default_cn_mgr)
	{
		memset(&m_addr, 0, sizeof(m_addr));
	}
	~Listener();
	bool Init(unsigned short listen_port, const char *listen_ip = nullptr);
	bool Init(const sockaddr_in &addr);
	sockaddr_in GetAddr(){ return m_addr; }
	BaseConMgr &GetConnMgr() { return m_cn_mgr; }

private:
	static void accept_error_cb(evconnlistener* listener, void * ctx);
	static void listener_cb(struct evconnlistener* listener, evutil_socket_t fd, struct sockaddr* sa, int socklen, void* user_data);

private:
	evconnlistener *m_listener;
	ConnectorMgr<Connector> m_default_cn_mgr;
	sockaddr_in m_addr;
	BaseConMgr &m_cn_mgr; 
};



template<class Connector>
SvrCon * ConnectorMgr<Connector>::NewConnect()
{
	return new Connector;
}



template<class Connector /*= NoUseConnector*/>
bool Listener<Connector>::Init(const sockaddr_in &addr)
{
	m_addr = addr;
	m_listener = evconnlistener_new_bind(EventMgr::Obj().GetEventBase(), Listener::listener_cb, (void*)this, LEV_OPT_REUSEABLE | LEV_OPT_CLOSE_ON_FREE, -1, (struct sockaddr*)&addr, sizeof(addr));
	if (!m_listener)
	{
		L_ERROR("evconnlistener_new_bind fail, addr.port=%d", addr.sin_port);
		return false;
	}
	evconnlistener_set_error_cb(m_listener, Listener::accept_error_cb);
	return true;
}

template<class Connector>
bool Listener<Connector>::Init(unsigned short listen_port, const char *listen_ip)
{
	sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(listen_port);
	if (nullptr != listen_ip)
	{
		addr.sin_addr.s_addr = inet_addr(listen_ip);
	}
	return Init(addr);
}

template<class Connector>
void Listener<Connector>::accept_error_cb(evconnlistener* listener, void * ctx)
{
	int err = EVUTIL_SOCKET_ERROR();
	L_ERROR("Got an error %d (%s) on the listener. \n", err, evutil_socket_error_to_string(err));
	//LibEventMgr::Obj().StopDispatch();
}


//底层已经调用了accept,获得fd，才回调这个函数
template<class Connector>
void Listener<Connector>::listener_cb(struct evconnlistener* listener, evutil_socket_t fd, struct sockaddr* sa, int socklen, void* user_data)
{
	if (nullptr == user_data)
	{
		L_ERROR("null cb para");
		return;
	}
	Listener* pListener = (Listener*)user_data;

	SvrCon *clientconn = pListener->m_cn_mgr.CreateConnectForListener();
	if (nullptr == clientconn)
	{
		L_ERROR("init SvrConnector fail");
		return;
	}
	clientconn->SetCnMgr(&(pListener->m_cn_mgr));
	if (!clientconn->AcceptInit(fd, sa, pListener->m_addr))
	{
		L_ERROR("init SvrConnector fail");
		return;
	}
}


template<class Connector>
Listener<Connector>::~Listener()
{
	if (nullptr != m_listener)
	{
		evconnlistener_free(m_listener);
	}
}

}//namespace lc //libevent cpp
