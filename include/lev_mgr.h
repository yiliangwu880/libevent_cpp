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

namespace lc //libevent cpp
{
	class ILogPrinter;
	//单件
	class EventMgr
	{
	public:
		static EventMgr &Obj()
		{
			static EventMgr d;
			return d;
		}
		//使用libevent 任何功能前，必须先调用这个初始化函数
		bool Init(ILogPrinter *iprinter = nullptr);

		void Dispatch();
		event_base *GetEventBase() { return m_eb; };
		bool StopDispatch();
		void RegSignal(int sig_type, void(*SignalCB)(int sig_type));

	private:
		EventMgr()
			:m_eb(nullptr)
		{
		}
		~EventMgr();

	private:
		event_base* m_eb;
	};
}//namespace lc //libevent cpp