
#include "lev_mgr.h"
#include <stddef.h>
#include <stdlib.h>
#include <errno.h>
#include "include_all.h"
#include <set>

using namespace std;

namespace lc //libevent cpp
{

BaseConMgr::BaseConMgr()
{
	m_timer.StartTimer(DELTE_CONNECTOR_INTERVAL, std::bind(&BaseConMgr::OnTimerDelConn, this), true);
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
		//LB_FATAL("free connector can't find. id=%llu", id);
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


void BaseConMgr::Foreach(const SvrConForeachCB &cb)
{
	vector<SvrCon *> tmp_vec; //避免遍历里面调用到m_all_connector修改函数
	for (const auto &v : m_all_connector)
	{
		if (nullptr == v.second)
		{
			LB_FATAL("save null SvrCon");
			continue;
		}
		tmp_vec.push_back(v.second);
	}
	for (SvrCon *con: tmp_vec)
	{
		(cb)(*con);
	}
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
		//LB_DEBUG("run OnTimerDelConn");
		DelConnect(v);
	}
	m_vwdc.clear();
}

SvrCon::SvrCon()
	:m_cn_mgr(nullptr)
	, m_id(0)
{
	// 符合以下条件就能保证唯一：  只运行136年内， 而且每1秒内调用少于4亿多(32位最大数)，
	//一般几行代码，重复跑4亿次，都要十几秒了。 所以实际情况不会重复
	static uint32 seed = 0;
	time_t sec;
	time(&sec);
	seed++;
	m_id = sec << 32;
	m_id = m_id | seed;
}

SvrCon::~SvrCon()
{
	m_cn_mgr = nullptr; //析构后清除指针，防错误代码产生野指针。
}

bool SvrCon::AcceptInit(evutil_socket_t fd, struct sockaddr* sa, const sockaddr_in &svr_addr)
{
	SetAddr(svr_addr);
	if (0 != GetFd())
	{
		LB_ERROR("repeated init");
		return false;
	}
	bufferevent* buf_e = bufferevent_socket_new(EventMgr::Ins().GetEventBase(), fd, BEV_OPT_CLOSE_ON_FREE); //释放m_buf_e，的时候，库里面会释放m_fd
	if (!buf_e)
	{
		LB_ERROR("cannot bufferevent_socket_new libevent ...\n");
		return false;
	}

	bufferevent_setcb(buf_e, readcb, nullptr, eventcb, this);
	bufferevent_enable(buf_e, EV_WRITE | EV_READ);
	if (false == SetSocketInfo(buf_e, fd, sa))
	{
		LB_ERROR("SetSocketInfo fail");
		bufferevent_free(buf_e);
		return false;
	}

	SetIsConnect(true);
	OnConnected();
	return true;
}

bool SvrCon::DisConnect()
{
	ConCom::DisConnect();
	return m_cn_mgr->PostDelConn(m_id); //延时 delete对象
}

void SvrCon::OnDisconnected()
{
	m_cn_mgr->PostDelConn(m_id); //延时 delete对象
}
}//namespace lc //libevent cpp