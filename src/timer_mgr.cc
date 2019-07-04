
#include "timer_mgr.h"
#include <stddef.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>  
#include "connector.h"
#include "include_all.h"

#include "log_file.h"

using namespace std;
namespace lc //libevent cpp
{

	BaseLeTimer::BaseLeTimer()
		:m_event(nullptr)
		, m_state(S_WAIT_START_TIMER)
		, m_para(nullptr)
	{
	}

	BaseLeTimer::~BaseLeTimer()
	{
		if (nullptr != m_event)
		{
			event_free(m_event);
		}
	}


	void BaseLeTimer::TimerCB(evutil_socket_t, short, void* para)
	{
		BaseLeTimer *p = (BaseLeTimer *)para;
		p->OnTimer(p->m_para);
	}

	bool BaseLeTimer::StartTimer(unsigned long long millisecond, void *para, bool is_loop)
	{
		timeval t;
		t.tv_sec = millisecond / 1000;
		t.tv_usec = (millisecond % 1000) * 1000;

		if (S_WAIT_START_TIMER != m_state)
		{
			LIB_LOG_ERROR("BaseLeTimer state error, repeated start timer");
			return false;
		}
		if (nullptr != m_event)
		{
			event_free(m_event);
		}
		m_para = para;
		int ev_type = 0;
		if (is_loop)
		{
			ev_type = EV_PERSIST;
		}
		m_event = event_new(LibEventMgr::Obj().GetEventBase(), -1, ev_type, TimerCB, this);

		if (0 != event_add(m_event, &t))
		{
			LIB_LOG_ERROR("evtimer_add fail");
		}
		m_state = S_WAIT_TIME_OUT;
		return true;
	}



	void BaseLeTimer::TimerCB_StdBind(evutil_socket_t, short, void* para)
	{
		BaseLeTimer *p = (BaseLeTimer *)para;
		p->m_cb();
	}

	bool BaseLeTimer::StartTimer(unsigned long long millisecond, const BaseLeTimerCB &cb, bool is_loop /*= false*/)
	{
		timeval t;
		t.tv_sec = millisecond / 1000;
		t.tv_usec = (millisecond % 1000) * 1000;

		if (S_WAIT_START_TIMER != m_state)
		{
			LIB_LOG_ERROR("BaseLeTimer state error, repeated start timer");
			return false;
		}
		if (nullptr != m_event)
		{
			event_free(m_event);
		}
		//m_para = para;
		m_cb = cb;
		int ev_type = 0;
		if (is_loop)
		{
			ev_type = EV_PERSIST;
		}
		m_event = event_new(LibEventMgr::Obj().GetEventBase(), -1, ev_type, TimerCB_StdBind, this);

		if (0 != event_add(m_event, &t))
		{
			LIB_LOG_ERROR("evtimer_add fail");
		}
		m_state = S_WAIT_TIME_OUT;
		return true;
	}

	bool BaseLeTimer::StopTimer()
	{
		if (nullptr == m_event)
		{
			return false;
		}
		if (S_WAIT_TIME_OUT != m_state)
		{
			return false;
		}
		event_del(m_event);
		m_state = S_WAIT_START_TIMER;
		return true;
	}
}
