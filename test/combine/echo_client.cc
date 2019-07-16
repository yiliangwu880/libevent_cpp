/*
联合测试
*/


#include "include_all.h"

using namespace lc;
using namespace std;


static const char *LOCAL_IP = "127.0.0.1";

namespace{

class MyConnectClient : public ClientCon
{
public:
	MyConnectClient()
		:m_cnt(0)
	{

	}
private:
	virtual void OnRecv(const MsgPack &msg) override
	{

		//LOG_DEBUG("OnRecv len data %d, %d %d", msg.len, msg.data[0], msg.data[4]);
		int *p = (int *)msg.data;
		UNIT_ASSERT(*p == m_cnt);
		UNIT_ASSERT(msg.len == sizeof(m_cnt));

		SendCnt();
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
		if (m_cnt>=3000)
		{
			L_DEBUG("free client");
			delete this;
			return;
		}
		//LOG_DEBUG("SendCnt %d", m_cnt);
		MsgPack msg;
		memset(&msg, 0, sizeof(msg));
		msg.len = sizeof(m_cnt);
		int *p = (int *)msg.data;
		*p = m_cnt;
		send_data(msg);

		//LOG_DEBUG("SendCnt msg len data %d, %d %d", msg.len, msg.data[0], msg.data[4]);

	}
	int m_cnt;
};


class SplitMsgTimer : public Timer
{
private:
	virtual void OnTimer(void *user_data) override;
};


//分割字符串发网络消息
class SplitMsgClient : public ClientCon
{
public:
	SplitMsgClient()
		:m_cnt(0)
	{

	}
	~SplitMsgClient()
	{
		L_DEBUG("del SplitMsgClient");
	}

	virtual void OnRecv(const MsgPack &msg) override
	{
		int *p = (int *)msg.data;
		UNIT_ASSERT(*p == m_cnt);
		UNIT_ASSERT(msg.len == sizeof(m_cnt));
		L_DEBUG("SplitMsgClient rev ok. m_cnt=%d", m_cnt);
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
		//LOG_DEBUG("SendCnt %d", m_cnt);
		memset(&m_msg, 0, sizeof(m_msg));
		m_msg.len = sizeof(m_cnt);
		m_msg.len = htons((int)m_msg.len);
		int *p = (int *)m_msg.data;
		*p = m_cnt;

		
		const char *pMsg = (char *)&m_msg;
		send_data_no_head(pMsg, 3);

		SplitMsgTimer *t = new SplitMsgTimer();
		t->StartTimer(100, this);
	}
	void send_remain()
	{
		const char *p = (char *)&m_msg;
		p = p + 3;
		send_data_no_head(p, 5);
	}
	int m_cnt;
	MsgPack m_msg;
};

void SplitMsgTimer::OnTimer(void *user_data)
{
	SplitMsgClient *p = (SplitMsgClient *)user_data;
	p->send_remain();
	delete this;
}


struct ReConnectClient : public ClientCon
{
	enum State
	{
		S_start,
		S_disconnect,
		S_reconnect,
	} state= S_start;
	Timer timer;
	Timer result_timer;
	bool is_re_con_ok = false;
	bool is_cb_disconect = false;
	void Start()
	{
		state = S_start;
		ConnectInit(LOCAL_IP, ECHO_SERVER_PORT);		
		auto f = std::bind(&ReConnectClient::Result, this);
		result_timer.StartTimer(1000*3, f);
	}
	virtual void OnRecv(const MsgPack &msg) override
	{
		L_DEBUG("disconnect");
		DisConnect();

		state = S_disconnect;
		auto f = std::bind(&ReConnectClient::ReConnect, this);
		timer.StartTimer(10, f);

	}
	virtual void OnConnected() override
	{
		if (S_start == state)
		{
			MsgPack msg;
			msg.len = 1;
			send_data(msg);
		}
		else if (S_disconnect == state)
		{
			L_DEBUG("reconnect ok");
			is_re_con_ok = true;
			state = S_reconnect;
		}
	}
	virtual void on_disconnected() override
	{
		is_cb_disconect = true;
	}

	void ReConnect()
	{
		TryReconnect();
	}
	void Result()
	{
		UNIT_ASSERT(true == is_re_con_ok);
		UNIT_ASSERT(true == is_cb_disconect);
		L_DEBUG("==============test reconnect ok==============");
	}
};

MyConnectClient *echo_client;
SplitMsgClient *split_client;
ReConnectClient *reCon;
}//namespace{


void StartEchoClient()
{
	//测试重复发送消息，回显检验是否一样
	echo_client = new MyConnectClient();
	echo_client->ConnectInit(LOCAL_IP, ECHO_SERVER_PORT);

	//测试一个包分割发送，检验是否正确
	split_client = new SplitMsgClient();
	split_client->ConnectInit(LOCAL_IP, ECHO_SERVER_PORT);

	//测试断开重连
	reCon = new ReConnectClient();
	reCon->Start();
}