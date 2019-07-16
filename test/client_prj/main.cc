//����server_prj���ԣ������ͻ������ӣ���ʱ���ͣ���������

#include "stdafx.h"
#include "version.h"
#include "unit_test.h"
#include "log_file.h"
#include "include_all.h"
#include <stdlib.h>

using namespace std;
using namespace lc;

namespace {
	const int MASS_CON_SVR_PORT = 42011;
	const uint32 MAX_CON_NUM = 10000*4;
	const uint32 Rand_SEND_INTERVAL_SEC = 2 * 1000;
	const uint32 RECON_INTERVAL_SEC = 2 * 1000;
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
				bool ret = m_timer.StartTimer((rand() % Rand_SEND_INTERVAL_SEC), f);
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
					DisConnect(); //�������ﵹ��FDԽ��Խ�࣬�ͷŲ��ˣ�ԭ��δ��

								//�������ʱ���������̸����
					{
						m_timer.StopTimer();
						auto f = std::bind(&MyConnectClient::OnTimer, this);
						bool ret = m_timer.StartTimer((rand() % Rand_SEND_INTERVAL_SEC), f);
						UNIT_ASSERT(true == ret);
					}
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
				L_ERROR("unknow state=%d", m_state);
				UNIT_ASSERT(false);
			}

			//�������ʱ���������̸����
			{
				m_timer.StopTimer();
				auto f = std::bind(&MyConnectClient::OnTimer, this);
				bool ret = m_timer.StartTimer( (rand() % Rand_SEND_INTERVAL_SEC), f);
				UNIT_ASSERT(true == ret);
			}
		}
	};

	Timer log_timer;


	//ɾ����ʽ����
	struct ClientReconByDel : public ClientCon
	{
		int m_cnt;
		ClientReconByDel()
			:m_cnt(0)
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
			++m_cnt;
			MsgPack msg;
			memset(&msg, 0, sizeof(msg));
			msg.len = sizeof(m_cnt);
			int *p = (int *)msg.data;
			*p = m_cnt;
			send_data(msg);
			
		}
		virtual void on_disconnected() override
		{

		}
	};

	struct ClientReconByDelCtr
	{
		ClientReconByDel *m_client;
		Timer m_timer;
		ClientReconByDelCtr()
		{
			m_client = nullptr;
			m_timer.StopTimer();
			auto f = std::bind(&ClientReconByDelCtr::OnTimer, this);
			bool ret = m_timer.StartTimer(RECON_INTERVAL_SEC, f, true);
			UNIT_ASSERT(true == ret);
		}
		~ClientReconByDelCtr()
		{
			if (nullptr != m_client)
			{
				delete m_client;
			}
		}
		void CreateClient()
		{
			UNIT_ASSERT(nullptr == m_client);
			m_client = new ClientReconByDel();
			bool ret = m_client->ConnectInit(LOCAL_IP, MASS_CON_SVR_PORT);
			UNIT_ASSERT(true == ret);
		}
		void DelClient()
		{
			UNIT_ASSERT(nullptr != m_client);
			if (nullptr != m_client)
			{
				delete m_client;
				m_client = nullptr;
			}
		}
		void OnTimer()
		{
			if (nullptr == m_client)
			{
				 CreateClient();
				// LOG_DEBUG("CreateClient");
			} 
			else
			{
				DelClient();
			//	LOG_DEBUG("DelClient");
			}
		}

	};
}//namespace

void OnLog()
{
	L_DEBUG("total_rev_cnt = %dw", total_rev_cnt / 10000);
	L_DEBUG("total_recon_cnt = %dw",  total_recon_cnt / 10000);
}

UNITTEST(mass_con_client)
{
	EventMgr::Obj().Init();
	for (uint32 i=0; i< MAX_CON_NUM/2; ++i)
	{
		MyConnectClient *client = new MyConnectClient();
		bool ret = client->ConnectInit(LOCAL_IP, MASS_CON_SVR_PORT);
		UNIT_ASSERT(true == ret);
	}

	for (uint32 i = 0; i < MAX_CON_NUM / 2; ++i)
	{
		ClientReconByDelCtr *ctrl = new ClientReconByDelCtr();
		UNIT_ASSERT(nullptr != ctrl);
	}
	log_timer.StartTimer(1000 * 10, std::bind(&OnLog), true);
	EventMgr::Obj().Dispatch();
}

int main(int argc, char* argv[]) 
{
	L_DEBUG("start run");
	UnitTestMgr::Obj().Start();
	L_DEBUG("end run");
	return 0;
}

