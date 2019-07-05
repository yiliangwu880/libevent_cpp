
#include "lev_mgr.h"
#include <stddef.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>  
#include "connector.h"
#include "log_file.h"
#include <signal.h>

using namespace std;

#define EVENT_LOG_DEBUG 0
#define EVENT_LOG_MSG 1
#define EVENT_LOG_WARN 2
#define EVENT_LOG_ERR 3

namespace lc //libevent cpp
{
namespace
{
	//ע�⣺�ص������н���LibEvent �ĺ��������ǲ���ȫ��
	void LIB_EVENT_LOG(int severity, const char* msg)
	{
		switch (severity)
		{
		case _EVENT_LOG_DEBUG:
			LIB_LOG_DEBUG("lib event log: %s\n", msg);
			break;
		case _EVENT_LOG_MSG:
			LIB_LOG_INFO("lib event log: %s\n", msg);
			break;
		case _EVENT_LOG_WARN:
			LIB_LOG_WARN("lib event log: %s\n", msg);
			break;
		case _EVENT_LOG_ERR:
			LIB_LOG_ERROR("lib event log: %s\n", msg);
			break;
		default:
			LIB_LOG_FATAL("lib event unknow log: %s\n", msg);
			break; /* never reached*/
		}
	}
	void EVENT_FATAL_CB(int err)
	{
		LIB_LOG_FATAL("EVENT_FATAL_CB %d", err);
		exit(1);
	}

}
bool EventMgr::Init()
{
	if (nullptr != m_eb)
	{
		LIB_LOG_ERROR("repeated init");
		return false;
	}
	event_set_log_callback(LIB_EVENT_LOG); //lib����־���
	event_set_fatal_callback(EVENT_FATAL_CB);
	//LOG_DEBUG("libevent version:%s", event_get_version());
	m_eb = event_base_new();
	if (!m_eb)
	{
		LIB_LOG_ERROR("cannot event_base_new libevent ...\n");
		return false;
	}
	//Ϊ�˱�������˳�, ���Բ���SIGPIPE�ź�, ���ߺ�����, ��������SIG_IGN�źŴ�����:
	//����, �ڶ��ε���write����ʱ, �᷵�� - 1, ͬʱerrno��ΪSIGPIPE.�������֪���Զ��Ѿ��ر�.
	signal(SIGPIPE, SIG_IGN);
	return true;
}

void EventMgr::Dispatch()
{
	if (!m_eb)
	{
		LIB_LOG_ERROR("LibEventMgr not init\n");
		return;
	}
	event_base_dispatch(m_eb);
}

bool EventMgr::StopDispatch()
{
	if (!m_eb)
	{
		LIB_LOG_ERROR("LibEventMgr not init\n");
		return false;
	}

	return event_base_loopexit(m_eb, NULL) == 0;//���event_base ��ǰ����ִ�м����¼��Ļص�, ������ִ���굱ǰ���ڴ�����¼��������˳�.
}

namespace
{
	void signal_cb(evutil_socket_t sig, short events, void* user_data)
	{
		if (user_data)
		{
			typedef  void(*FUN_TYPE)(int sig_type);
			FUN_TYPE fun = (FUN_TYPE)user_data;
			fun((int)sig);
		}
	}
}

void EventMgr::RegSignal(int sig_type, void(*SignalCB)(int sig_type))
{
	event* signal_event = evsignal_new(m_eb, sig_type, signal_cb, (void*)SignalCB);
	if (!signal_event || event_add(signal_event, NULL) < 0)
	{
		LIB_LOG_ERROR("call evsignal_new fail");
		return;
	}
}

EventMgr::~EventMgr()
{
	if (nullptr != m_eb)
	{
		event_base_free(m_eb);
	}
}
}//namespace lc //libevent cpp