/*
ÁªºÏ²âÊÔ
*/


#include "include_all.h"


using namespace std;


static const char *LOCAL_IP = "127.0.0.1";

namespace{
class MyConnectClient : public BaseClientCon
{
public:
	MyConnectClient()
		:m_cnt(0)
	{

	}
private:
	virtual void OnRecv(const MsgPack &msg) override
	{
		LOG_DEBUG("OnRecv");
		//int *p = (int *)msg.data;
		//UNIT_ASSERT(*p == m_cnt);
		//UNIT_ASSERT(msg.len == sizeof(m_cnt));

	//	SendCnt();
	}
	virtual void OnConnected() override
	{
		SendCnt();
	}
	virtual void on_disconnected() override
	{

	}
	void SendCnt()
	{
		++m_cnt;
		if (m_cnt>=30)
		{
			LOG_DEBUG("free client");
			delete this;
			return;
		}
		LOG_DEBUG("SendCnt %d", m_cnt);
		MsgPack msg;
		memset(&msg, 0, sizeof(msg));

		int *p = (int *)msg.data;
		*p = m_cnt;

		msg.data[0] = 1;
		msg.data[1] = 2;
		msg.data[2] = 3;
		msg.data[3] = 4;

		msg.len = sizeof(m_cnt);

		LOG_DEBUG("Send msg");
		send_data(msg);

	}
	int m_cnt;
};
MyConnectClient *echo_client;
}//namespace{

using namespace std;
namespace {
	class Connect2Client : public ListenerConnector
	{
	public:

	private:
		virtual void OnRecv(const MsgPack &msg) override
		{
			//send_data(msg);
		}
		virtual void OnConnected() override
		{
		}
		virtual void onDisconnected() override {}
	};

	Listener<Connect2Client> listener;
}


UNITTEST(echo_client)
{
	listener.Init(ECHO_SERVER_PORT);
	echo_client = new MyConnectClient();
	echo_client->ConnectInit(LOCAL_IP, ECHO_SERVER_PORT);
}