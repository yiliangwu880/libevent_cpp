
#include "timer_mgr.h"
#include <stddef.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>  
#include "connector.h"
#include "include_all.h"
#include <limits>

#include "log_file.h"

using namespace std;
namespace lc //libevent cpp
{

	uint32_t Timer::s_time_cnt=0;

	Timer::Timer()
		:m_event(nullptr)
		, m_state(S_WAIT_START_TIMER)
		, m_is_loop(false)
	{
		s_time_cnt++;
	}

	Timer::~Timer()
	{
		if (nullptr != m_event)
		{
			event_free(m_event);
		}
		s_time_cnt--;
		if (s_time_cnt>numeric_limits<int32_t>::max())
		{
			LB_ERROR("Timer s_time_cnt error, 0 - 1?");//0数量了，还减？
		}
	}




	



	void Timer::TimerCB_StdBind(evutil_socket_t, short, void* para)
	{
		Timer *p = (Timer *)para;
		if (!p->m_is_loop)
		{
			p->StopTimer();
		}
		p->m_cb();
	}

	bool Timer::StartTimer(unsigned long long millisecond, const TimerCB &cb, bool is_loop /*= false*/)
	{
		timeval t;
		t.tv_sec = millisecond / 1000;
		t.tv_usec = (millisecond % 1000) * 1000;

		if (S_WAIT_START_TIMER != m_state)
		{
			LB_ERROR("Timer state error, repeated start timer");
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

		m_event = event_new(EventMgr::Ins().GetEventBase(), -1, ev_type, TimerCB_StdBind, this);

		if (0 != event_add(m_event, &t))
		{
			LB_ERROR("evtimer_add fail"); 
			return false;
		}
		m_is_loop = is_loop;
		m_state = S_WAIT_TIME_OUT;
		return true;
	}

	bool Timer::StopTimer()
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
