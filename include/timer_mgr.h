/*
实际项目不建议生成大量lc::Timer. 因为这个是底层libevent timer.量大出错，资源泄漏难跟踪。
可以设置一个循环lc::Timer来驱动用户自定义的timer系统。


use excample:
	class MyTimer : public BaseLeTimer
	{
	private:
	virtual void OnTimer(void *user_data) override{};
	};

	main()
	{
		EventMgr::Obj().Init();
		MyTimer o;
		o.StartTimer(1000, &user_data);

		EventMgr::Obj().dispatch();
	}

use excample2:
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

		BaseLeTimer m_timer;
	};
	MyConnectClient obj;
	obj.StartTimer();

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
	typedef std::function<void(void)> TimerCB;
	//里面做创建，销毁定时器，保证不内存泄露, 不回调不存在的Timer
	class Timer
	{

	public:
		Timer();
		virtual ~Timer();

		//para is_loop true表示循环定时器
		//return, true成功开始定时器，false 已经开始了，不需要设定(可以先stop,再start)
		bool StartTimer(unsigned long long millisecond, void *para = nullptr, bool is_loop = false);
		bool StartTimer(unsigned long long millisecond, const TimerCB &cb, bool is_loop = false);
		//停止正在进行的定时器，
		//return, false 不需要停止. true 成功操作了停止
		bool StopTimer();

		virtual void OnTimer(void *para) {};
	private:
		static void OnTimerCB(evutil_socket_t, short, void* para);
		static void TimerCB_StdBind(evutil_socket_t, short, void* para);
	private:
		enum State
		{
			S_WAIT_START_TIMER,
			S_WAIT_TIME_OUT,
		};
		event* m_event;
		State m_state;
		void *m_para;
		TimerCB m_cb; //选择用，用std::bind方式绑定的回调函数
	};
}