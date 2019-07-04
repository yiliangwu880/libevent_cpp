//联合server_prj测试，大量客户端连接，定时发送，接收数据

#include "stdafx.h"
#include "version.h"
#include "unit_test.h"
#include "logFile.h"
#include "include_all.h"

using namespace std;


namespace {
	const int MASS_CON_SVR_PORT = 42011;
	const uint32 MAX_CON_NUM = 5;
	const uint32 SEND_INTERVAL_SEC = 2 * 1000;
	const int EACH_CON_SEND_CNT = 5; //每个连接发送次数
	const char *LOCAL_IP = "127.0.0.1";


	class MyConnectClient : public BaseClientCon
	{
	public:
		MyConnectClient()
			:m_cnt(0)
		{

		}

		virtual void OnRecv(const MsgPack &msg) override
		{
			int *p = (int *)msg.data;
			UNIT_ASSERT(*p == m_cnt);
			UNIT_ASSERT(msg.len == sizeof(m_cnt));

		}
		virtual void OnConnected() override
		{
			auto f = std::bind(&MyConnectClient::TimerSendCnt, this);
			m_timer.StartTimer(SEND_INTERVAL_SEC, f, true);

		}
		virtual void on_disconnected() override
		{

		}
		void TimerSendCnt()
		{
			++m_cnt;
			if (m_cnt >= EACH_CON_SEND_CNT)
			{
				LOG_DEBUG("free client");
				delete this;
				return;
			}
			MsgPack msg;
			memset(&msg, 0, sizeof(msg));
			msg.len = sizeof(m_cnt);
			int *p = (int *)msg.data;
			*p = m_cnt;
			send_data(msg);
		}
		int m_cnt;
		BaseLeTimer m_timer;
	};


}//namespace

UNITTEST(mass_con_client)
{
	for (uint32 i=0; i< MAX_CON_NUM; ++i)
	{
		MyConnectClient *client = new MyConnectClient();
		UNIT_ASSERT(client->ConnectInit(LOCAL_IP, MASS_CON_SVR_PORT));
	}
}

int main(int argc, char* argv[]) 
{
	LOG_DEBUG("start run");
	LogMgr::Obj().SetStdOut(true);
	UnitTestMgr::Obj().Start();
	LOG_DEBUG("end run");
	return 0;
}

