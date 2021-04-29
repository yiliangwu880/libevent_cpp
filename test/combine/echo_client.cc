/*
联合测试
*/


#include "include_all.h"

using namespace lc;
using namespace std;


static const char *LOCAL_IP = "127.0.0.1";



namespace {


	void FreeMyConnectClient();
	void FreeSplitMsgTimer();
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

			//LB_DEBUG("OnRecv len data %d, %d %d", msg.len, msg.data[0], msg.data[4]);
			int *p = (int *)msg.data;
			UNIT_ASSERT(*p == m_cnt);
			UNIT_ASSERT(msg.len == sizeof(m_cnt));

			SendCnt();
		}
		virtual void OnConnected() override
		{
			SendCnt();

			//m2MyConnectClient.erase(1);
		}
		virtual void OnDisconnected() override
		{

		}
		void SendCnt()
		{
			++m_cnt;
			if (m_cnt >= 3000)
			{
				LB_DEBUG("free client");
				FreeMyConnectClient();
				return;
			}
			//LB_DEBUG("SendCnt %d", m_cnt);
			MsgPack msg;
			memset(&msg, 0, sizeof(msg));
			msg.len = sizeof(m_cnt);
			int *p = (int *)msg.data;
			*p = m_cnt;
			SendData(msg);

			//LB_DEBUG("SendCnt msg len data %d, %d %d", msg.len, msg.data[0], msg.data[4]);

		}
		int m_cnt;
	};


	class SplitMsgTimer : public Timer
	{
	private:
		bool StartTimer(unsigned long long millisecond, void *p=nullptr, bool is_loop = false)
		{
			TimerCB f = std::bind(&SplitMsgTimer::OnTimer, this, p);
			return Timer::StartTimer(millisecond, f, is_loop);
		}

		void OnTimer(void *user_data);
	};


	//分割字符串发网络消息
	class SplitMsgClient : public ClientCon
	{

		string m_str;
		MsgPack m_msg;
		lc::Timer m_tm;
		bool m_is_ok = false;
	public:

		SplitMsgClient()
		{

		}
		//定义了 ~SplitMsgClient()， 就不能自动生成移动构造函数了
		SplitMsgClient(SplitMsgClient &&src) :ClientCon(std::move(src)) {}
		~SplitMsgClient()
		{
			LB_DEBUG("del SplitMsgClient");
		}

		virtual void OnRecv(const MsgPack &msg) override
		{
			UNIT_ASSERT(m_str == msg.data);
			UNIT_ASSERT(msg.len == m_str.length());

			LB_DEBUG("SplitMsgClient rev ok. ");
			m_is_ok = true;
		}
		virtual void OnConnected() override
		{
			SendCnt();
		}
		virtual void OnDisconnected() override
		{

		}
		void SendCnt()
		{
			m_str.append("1111111111111111111", 17);
			UNIT_ASSERT(17 == m_str.length());
			memset(&m_msg, 0, sizeof(m_msg));
			m_msg.len = m_str.length();
			m_msg.len = auto_hton(m_msg.len);
			
			memcpy(m_msg.data, m_str.c_str(), m_str.length());
			UNIT_INFO("send %s", m_msg.data);



			const char *pMsg = (char *)&m_msg;
			SendData(pMsg, 3);

			m_tm.StartTimer(100, std::bind(&SplitMsgClient::send_remain1, this));
		}
		void send_remain1();

		void send_remain2()
		{
			const char *p = (char *)&m_msg;
			p = p + 4;
			SendData(p, 17 + sizeof(m_msg.len) - 4);
			UNIT_INFO("send_remain2");
			m_tm.StartTimer(300, std::bind(&SplitMsgClient::CheckRevOk, this));
		}
		void CheckRevOk()
		{
			UNIT_ASSERT(m_is_ok);
		}
	};


	void SplitMsgTimer::OnTimer(void *user_data)
	{
		SplitMsgClient *p = (SplitMsgClient *)user_data;
		p->send_remain1();
		FreeSplitMsgTimer();
	}


	struct ReConnectClient : public ClientCon
	{
		enum State
		{
			S_start,
			S_disconnect,
			S_reconnect,
		} state = S_start;
		Timer timer;
		Timer result_timer;
		bool is_re_con_ok = false;
		bool is_cb_disconect = false;
		void Start()
		{
			state = S_start;
			ConnectInit(LOCAL_IP, ECHO_SERVER_PORT);
			auto f = std::bind(&ReConnectClient::Result, this);
			result_timer.StartTimer(1000 * 3, f);
		}
		virtual void OnRecv(const MsgPack &msg) override
		{
			LB_DEBUG("disconnect");
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
				SendData(msg);
			}
			else if (S_disconnect == state)
			{
				LB_DEBUG("reconnect ok");
				is_re_con_ok = true;
				state = S_reconnect;
			}
		}
		virtual void OnDisconnected() override
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
			LB_DEBUG("==============test reconnect ok==============");
		}
	};

	static bool isFailClientOk = false;
	struct ConFailClient : public ClientCon
	{
		virtual void OnConnected() override
		{
			UNIT_ASSERT(false);
		}
		virtual void OnDisconnected() override
		{
			//UNIT_INFO("OnDisconnected");
			isFailClientOk = true;
		}
		virtual void OnRecv(const MsgPack &msg) override
		{
			UNIT_ASSERT(false);
		}

	};
	MyConnectClient *echo_client;
	SplitMsgClient *split_client;
	ReConnectClient *reCon;
	ConFailClient *fail_c = nullptr;
	std::map<int, MyConnectClient> m2MyConnectClient;
	std::map<int, SplitMsgClient> m2SplitMsgClient;

	void FreeMyConnectClient()
	{
		m2MyConnectClient.erase(1);
	}

	void FreeSplitMsgTimer()
	{
		m2SplitMsgClient.erase(1);
	}

	void SplitMsgClient::send_remain1()
	{
		const char *p = (char *)&m_msg;
		p = p + 3;
		SendData(p, 1);
		UNIT_INFO("send_remain1");

		m_tm.StartTimer(100, std::bind(&SplitMsgClient::send_remain2, this));

	}

}//namespace{

struct ConCom1
{
	ConCom1() {}
	//ConCom(const ConCom &) = delete;
	ConCom1(ConCom1 &&src) {}
	ConCom1 & operator= (const ConCom1 &) = delete;
	virtual ~ConCom1() {};
};

struct ClientCon1 : public ConCom1
{
public:
	ClientCon1() {}
	ClientCon1(ClientCon1 &&src) :ConCom1(std::move(src)) {}
};


void StartEchoClient()
{
	//测试重复发送消息，回显检验是否一样
	{
		auto it = m2MyConnectClient.emplace(1, MyConnectClient()).first;
		echo_client = &(it->second);
		echo_client->ConnectInit(LOCAL_IP, ECHO_SERVER_PORT);
	}

	

//测试一个包分割发送，检验是否正确
	{
		auto it = m2SplitMsgClient.emplace(1, SplitMsgClient()).first;
		split_client = &(it->second);
		split_client->ConnectInit(LOCAL_IP, ECHO_SERVER_PORT);
	}

	//测试断开重连
	reCon = new ReConnectClient();
	reCon->Start();
}

//连接失败客户端，服务器不存在
void StartConFailClient()
{
	fail_c = new ConFailClient();
	fail_c->ConnectInit(LOCAL_IP, 38774);
}

void CheckClientEnd()
{
	UNIT_ASSERT(isFailClientOk);
	m2SplitMsgClient.clear();
	m2MyConnectClient.clear();
}

