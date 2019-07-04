/*
待加个接口：类似下面
	auto fun = std::bind(obj, obj::member_fun)
	time.StartTimter(1000, fun) //时间过期，自动调用 obj::member_fun， 用户自己保证obj生存期  > time，不然野指针

use excample:
	class MyTimer : public BaseLeTimer
	{
	private:
	virtual void OnTimer(void *user_data) override{};
	};

	main()
	{
		LibEventMgr::Obj().Init();
		MyTimer o;
		o.StartTimer(1000, &user_data);

		LibEventMgr::Obj().dispatch();
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


typedef std::function<void(void)> BaseLeTimerCB;

//里面做创建，销毁定时器，保证不内存泄露, 不回调不存在的BaseTime
class BaseLeTimer
{

public:
	BaseLeTimer();
	virtual ~BaseLeTimer();

	//para is_loop true表示循环定时器
	//return, true成功开始定时器，false 已经开始了，不需要设定(可以先stop,再start)
	bool StartTimer(unsigned long long millisecond, void *para = nullptr, bool is_loop = false);
	bool StartTimer(unsigned long long millisecond, const BaseLeTimerCB &cb, bool is_loop = false);
	//停止正在进行的定时器，
	//return, false 不需要停止. true 成功操作了停止
	bool StopTimer();

	virtual void OnTimer(void *para) {};
private:
	static void TimerCB(evutil_socket_t, short, void* para);
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
	BaseLeTimerCB m_cb; //选择用，用std::bind方式绑定的回调函数
};