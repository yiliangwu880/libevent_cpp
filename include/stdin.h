/*
可当标准输入控制进程用。
用网络接收的方式，直接接收字符串，（当标准输入用， 用户写client,发字符串到服务，服务器用下面例子写处理字符串内容）。
use example:
void Read(const char*pMsg, int len)
{
	UNIT_INFO("len=%d, str=%s", len, pMsg);
}

main()
{
	Stdin::Ins().Init(Read);
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
#include <map>
#include <unordered_map>
#include "connector.h"
#include <unistd.h>
#include <arpa/inet.h>  
#include "log_file.h" 
#include "timer_mgr.h"

namespace lc //libevent cpp
{
	using StdinCB = void (*)(const char*, int len);
	class Stdin
	{
	public:
		static void eventcb(struct bufferevent* bev, short events, void* user_data);
		static void readcb(struct bufferevent* bev, void* user_data);

	private:
		StdinCB m_cb = nullptr;
		bufferevent* m_buf_e = nullptr;

	public:
		static Stdin &Ins()
		{
			static Stdin d;
			return d;
		}

		void Init(StdinCB cb);
		void Del(); //删除标准输入监听

	};
}//namespace lc //libevent cpp
