/*
Դ���ַ��
https://github.com/yiliangwu880/libevent_cpp.git
��BUG��ӭ��ϵ qq 30390012

ֻ���ʵ��߳��á�
ʹ��ʵ�����ӿ� test_server.cc test_client.cc
ʹ�����ӣ�
�ͻ��ˣ�
	#include "../include_all.h"

	class MyConnectClient1 : public BaseConnect
	{
	private:
		virtual void OnRecv(const MsgPack &msg) override
		{

		}
		virtual void OnConnected() override
		{
			MsgPack msg;
			...��ʼ��msg
			send_data(msg);
		}
		virtual void onDisconnected() override
		{
		}
	};
	void main()
	{
		EventMgr::Obj().Init();

		MyConnectClient1 connect1;
		connect1.ConnectInit(ip, server_port);

		EventMgr::Obj().dispatch();
	}

������:

	class Connect2Client : public ListenerConnector
	{
	public:

	private:
		virtual void OnRecv(const MsgPack &msg) override
		{
			LOG_DEBUG("OnRecv %s", &msg.data);
			send_data(msg);
			if (0)
			{//some time do this destory connect
				FreeSelf();
			}
		}
		virtual void OnConnected() override
		{
			LOG_DEBUG("OnConnected");
		}
		virtual void onDisconnected() override {}
	};

	//��ģ��
	void main()
	{
		EventMgr::Obj().Init();

		Listener<Connect2Client> listener;
		listener.Init(server_port);

		EventMgr::Obj().dispatch();
	}

	//ָ��connector���������ģʽ
	void main()
	{
		class ConnectorMgr : public BaseConnectorMgr
		{
		private:
		virtual ListenerConnector *CreateConnect(){...};

		};
		EventMgr::Obj().Init();
		ConnectorMgr mgr;
		Listener<> listener(mgr);
		listener.Init(server_port);

		EventMgr::Obj().dispatch();
	}	
*/

#pragma once

#include "lev_mgr.h"
#include "listener.h"
#include "connector.h"
#include "timer_mgr.h"

