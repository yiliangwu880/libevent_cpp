/*
	定时器代码注意，用户负责回调函数不引用野对象

use excample:
	class MyConnectClient
	{
		void Fun()
		{
		}
		void StartTimer()
		{
			auto f = std::bind(&MyConnectClient::Fun, this);
			m_timer.StartTimer(1000, f);
		}

		Timer m_timer;
	};
	MyConnectClient obj;
	obj.StartTimer();

错误示范：
	MyClass::Fun()
	{
	//f2是局部对象呀，带出函数外引用就是野对象了。
		auto f2 = [&]()
		{
			UNIT_INFO("f2 this=%p", this);//这里内存会乱掉。

		};
		auto f = [&]()
		{
			UNIT_INFO("f this=%p", this);
			m_tm.StopTimer();
			m_tm.StartTimer(1, f2);

		};
		m_tm.StopTimer();
		m_tm.StartTimer(1, f);
	}
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
#include "lev_mgr.h"
#include <functional>



namespace lc //libevent cpp
{
	using TimerCB = std::function<void(void)> ;
	//里面做创建，销毁定时器，保证不内存泄露, 不回调不存在的Timer
	class Timer
	{
		enum State
		{
			S_WAIT_START_TIMER,
			S_WAIT_TIME_OUT,
		};
		event* m_event;
		State m_state;
		TimerCB m_cb; //选择用，用std::bind方式绑定的回调函数
		bool m_is_loop;
		static uint32_t s_time_cnt; //定时器总数

	public:
		Timer();
		~Timer();

		//注意，cb为lambda表达式时，注意时局部对象，带出函数就崩溃。看文件开头描述错误示范。
		bool StartTimer(unsigned long long millisecond, const TimerCB &cb, bool is_loop = false);
		//停止正在进行的定时器，
		//return, false 不需要停止. true 成功操作了停止
		//StartTimer 不是循环时，过期后，对象回自动变成停止状态，再调用 StopTimer会失败。
		bool StopTimer();
		static uint32_t TotalTimerCnt() { return s_time_cnt; }
	private:
		static void TimerCB_StdBind(evutil_socket_t, short, void* para);

	private:
		Timer(const Timer&) = delete;
		Timer & operator= (const Timer &) = delete;
		Timer(Timer&&) = delete; //移动构造函数
	};
}