/* 
联合Client_prj测试， 大量连接

接收消息随机行为：回显， 主动关闭连接
*/

#include "stdafx.h"
#include "version.h"
#include "include_all.h"
#include "unit_test.h"
#include "logFile.h"

static const int MASS_CON_SVR_PORT = 42011;
using namespace lc;
using namespace std;
namespace {
	class Connect2Client : public SvrCon
	{
	public:

	private:
		virtual void OnRecv(const MsgPack &msg) override
		{
			int r = rand() % 100;
			if (r < 10)
			{
				DisConnect(); //调用这里，fd会释放不了。 原因未明 
				return;
			}
			send_data(msg);
		}
		virtual void OnConnected() override
		{
		}
		virtual void onDisconnected() override {

		}
	};


	Listener<Connect2Client> *listener=nullptr;
	class CloseTimer : public Timer
	{
	private:
		virtual void OnTimer(void *user_data) override;
	};

	void CloseTimer::OnTimer(void *user_data)
	{
		LOG_DEBUG("del svr");
		delete listener;
	}
	CloseTimer ct;


}//namespace {

UNITTEST(mass_con_svr)
{
	LogMgr::Obj().SetStdOut(true);

	EventMgr::Obj().Init();
	//ct.StartTimer(1000 * 60);
	listener = new Listener<Connect2Client>();
	listener->Init(MASS_CON_SVR_PORT);
	EventMgr::Obj().Dispatch();
}

int main(int argc, char* argv[]) 
{
	LIB_LOG_DEBUG("\n\n");
	UnitTestMgr::Obj().Start();
	return 0;
}

