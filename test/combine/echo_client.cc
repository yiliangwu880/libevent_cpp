/*
联合测试
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
			LOG_DEBUG("free client");
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

	}
	int m_cnt;
};


class SplitMsgTimer : public BaseLeTimer
{
private:
	virtual void OnTimer(void *user_data) override;
};


//分割字符串发网络消息
class SplitMsgClient : public BaseClientCon
{
public:
	SplitMsgClient()
		:m_cnt(0)
	{

	}
	~SplitMsgClient()
	{
		LOG_DEBUG("del SplitMsgClient");
	}

	virtual void OnRecv(const MsgPack &msg) override
	{
		int *p = (int *)msg.data;
		UNIT_ASSERT(*p == m_cnt);
		UNIT_ASSERT(msg.len == sizeof(m_cnt));
		LOG_DEBUG("SplitMsgClient rev ok. m_cnt=%d", m_cnt);
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
		m_msg.len = htonl((int)m_msg.len);
		int *p = (int *)m_msg.data;
		*p = m_cnt;

		
		const char *pMsg = (char *)&m_msg;
		m_com.send_data_no_head(pMsg, 3);

		SplitMsgTimer *t = new SplitMsgTimer();
		t->StartTimer(100, this);
	}
	void send_remain()
	{
		const char *p = (char *)&m_msg;
		p = p + 3;
		m_com.send_data_no_head(p, 5);
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

MyConnectClient *echo_client;
SplitMsgClient *split_client;
}//namespace{


void StartEchoClient()
{
	//测试重复发送消息，回显检验是否一样
	echo_client = new MyConnectClient();
	echo_client->ConnectInit(LOCAL_IP, ECHO_SERVER_PORT);

	//测试一个包分割发送，检验是否正确
	split_client = new SplitMsgClient();
	split_client->ConnectInit(LOCAL_IP, ECHO_SERVER_PORT);

}