
#include "lev_mgr.h"
#include <stddef.h>
#include <stdlib.h>
#include <errno.h>
#include "include_all.h"


using namespace std;

namespace lc //libevent cpp
{

BaseConMgr::BaseConMgr()
{
	m_timer.StartTimer(DELTE_CONNECTOR_INTERVAL, std::bind(&BaseConMgr::OnTimerDelConn, this));
}

BaseConMgr::~BaseConMgr()
{
	for (const auto &v : m_all_connector)
	{
		delete v.second;
	}
	m_all_connector.clear();
}

bool BaseConMgr::PostDelConn(uint64 id)
{
	auto it = m_all_connector.find(id);
	if (it == m_all_connector.end())
	{
		LIB_LOG_FATAL("free connector can't find. id=%llu", id);
		return false;
	}
	SvrCon *p = it->second;
	m_all_connector.erase(it);
	m_vwdc.push_back(p);
	return true;
}

SvrCon * BaseConMgr::FindConn(uint64 id)
{
	auto it = m_all_connector.find(id);
	if (it == m_all_connector.end())
	{
		return nullptr;
	}
	return it->second;
}


SvrCon * BaseConMgr::CreateConnectForListener()
{
	SvrCon *p = NewConnect();
	if (nullptr == p)
	{
		return p;
	}
	m_all_connector[p->GetId()] = p;
	return p;
}


void BaseConMgr::OnTimerDelConn()
{
	for (const auto &v : m_vwdc)
	{
		//LIB_LOG_DEBUG("run OnTimerDelConn");
		delete v;
	}
}

SvrCon::SvrCon()
	:m_cn_mgr(nullptr)
	, m_id(0)
	, m_ignore_free(false)
{
	static uint64 id_seed = 0;//大量重复连接就会重复，实际上不会产生那么大量
	m_id = ++id_seed;
	memset(&m_svr_addr, 0, sizeof(m_svr_addr));
}

SvrCon::~SvrCon()
{
	m_cn_mgr = nullptr; //析构后清除指针，防错误代码产生野指针。
}

bool SvrCon::AcceptInit(evutil_socket_t fd, struct sockaddr* sa, const sockaddr_in &svr_addr)
{
	m_svr_addr = svr_addr;

	if (0 != GetFd())
	{
		LIB_LOG_ERROR("repeated init");
		return false;
	}
	bufferevent* buf_e = bufferevent_socket_new(EventMgr::Obj().GetEventBase(), fd, BEV_OPT_CLOSE_ON_FREE); //释放m_buf_e，的时候，库里面会释放m_fd
	if (!buf_e)
	{
		LIB_LOG_ERROR("cannot bufferevent_socket_new libevent ...\n");
		return false;
	}

	bufferevent_setcb(buf_e, readcb, nullptr, eventcb, this);
	bufferevent_enable(buf_e, EV_WRITE | EV_READ);
	SetSocketInfo(buf_e, fd, sa);

	SetIsConnect(true);
	OnConnected();
	return true;
}

bool SvrCon::FreeSelf()
{
	if (m_ignore_free)
	{
		return false;
	}
	return m_cn_mgr->PostDelConn(m_id);
}

void SvrCon::on_disconnected()
{
	m_ignore_free = true;
	onDisconnected();
	m_ignore_free = false;
	FreeSelf();
}
}//namespace lc //libevent cpp