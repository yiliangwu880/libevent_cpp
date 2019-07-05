//联合server_prj测试，大量客户端连接，定时发送，接收数据

#include "stdafx.h"
#include "version.h"
#include "unit_test.h"
#include "logFile.h"
#include "include_all.h"
#include <stdlib.h>

using namespace std;
using namespace lc;

namespace {
	const int MASS_CON_SVR_PORT = 42011;
	const uint32 MAX_CON_NUM = 10000*1;
	const uint32 SEND_INTERVAL_SEC = 2 * 1000;
	const char *LOCAL_IP = "127.0.0.1";

	uint32 total_rev_cnt = 0;
	uint32 total_recon_cnt = 0;
	struct MyConnectClient : public ClientCon
	{
		int m_cnt;
		Timer m_timer;
		enum State
		{
			S_Start,
			S_Connect,
			S_DisConnect,
		}m_state;

		MyConnectClient()
			:m_cnt(0)
			, m_state(S_Start)
		{

		}

		virtual void OnRecv(const MsgPack &msg) override
		{

			UNIT_ASSERT(msg.len == sizeof(m_cnt));
			total_rev_cnt++;
			//LOG_DEBUG("OnRecv");
		}
		virtual void OnConnected() override
		{
			if (S_Start == m_state)
			{
				auto f = std::bind(&MyConnectClient::OnTimer, this);
				bool ret = m_timer.StartTimer(SEND_INTERVAL_SEC, f, true);
				UNIT_ASSERT(true == ret);
			}
			m_state = S_Connect;
		}
		virtual void on_disconnected() override
		{
			m_state = S_DisConnect;
		}

		void OnTimer()
		{
			if (S_Connect == m_state)
			{
				int r = ::rand() % 100;
				if (r<10)
				{
					//LOG_DEBUG("disconnect");
					DisConnect();
					return;
				}
				++m_cnt;
				MsgPack msg;
				memset(&msg, 0, sizeof(msg));
				msg.len = sizeof(m_cnt);
				int *p = (int *)msg.data;
				*p = m_cnt;
				send_data(msg);
			}
			else if (S_DisConnect == m_state)
			{
				//LOG_DEBUG("TryReconnect");
				total_recon_cnt++;
				TryReconnect();
			}
			else {
				LOG_ERROR("unknow state=%d", m_state);
				UNIT_ASSERT(false);
			}
		}
	};

	Timer log_timer;
}//namespace

void OnLog()
{
	LOG_DEBUG("total_rev_cnt = %dw", total_rev_cnt / 10000);
	LOG_DEBUG("total_recon_cnt = %dw",  total_recon_cnt / 10000);
}

UNITTEST(mass_con_client)
{
	EventMgr::Obj().Init();
	for (uint32 i=0; i< MAX_CON_NUM; ++i)
	{
		MyConnectClient *client = new MyConnectClient();
		bool ret = client->ConnectInit(LOCAL_IP, MASS_CON_SVR_PORT);
		UNIT_ASSERT(true == ret);
	}

	log_timer.StartTimer(1000 * 10, std::bind(&OnLog), true);
	EventMgr::Obj().Dispatch();
}

int main(int argc, char* argv[]) 
{
	LOG_DEBUG("start run");
	LogMgr::Obj().SetStdOut(true);
	UnitTestMgr::Obj().Start();
	LOG_DEBUG("end run");
	return 0;
}

