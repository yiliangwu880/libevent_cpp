/*
源码地址：
https://github.com/yiliangwu880/libevent_cpp.git
有BUG欢迎联系 qq 30390012

只合适单线程用。
使用实际例子看 test_server.cc test_client.cc
使用例子：
客户端：
	#include "../include_all.h"

	class MyConnectClient1 : public ClientCon
	{
	private:
		virtual void OnRecv(const MsgPack &msg) override
		{

		}
		virtual void OnConnected() override
		{
			//MsgPack msg;
			//...初始化msg
			//	SendData(msg);
		}
		virtual void OnDisconnected() override
		{
		}
	};
	void main()
	{
		

		MyConnectClient1 connect1;
		connect1.ConnectInit(ip, server_port);

		EventMgr::Ins().Dispatch();
	}

服务器:

	class Connect2Client : public lc::SvrCon
	{
	public:

	private:
		virtual void OnRecv(const lc::MsgPack &msg) override
		{
			LB_DEBUG("OnRecv %s", &msg.data);
			SendData(msg);
			if (0)
			{//some time do this destory connect
				FreeSelf();
			}
		}
		virtual void OnConnected() override
		{
			LB_DEBUG("OnConnected");
		}
	};

	//简单模型
	void main()
	{
		

		Listener<Connect2Client> listener;
		listener.Init(server_port);

		EventMgr::Ins().Dispatch();
	}

	//指定connector对象分配器模式
	void main()
	{
		class ConnectorMgr : public BaseConMgr
		{
		private:
		virtual SvrCon *CreateConnect(){...};

		};
		
		ConnectorMgr mgr;
		Listener<> listener(mgr);
		listener.Init(server_port);

		EventMgr::Ins().Dispatch();
	}	
*/

#pragma once

#include "lev_mgr.h"
#include "listener.h"
#include "connector.h"
#include "timer_mgr.h"

