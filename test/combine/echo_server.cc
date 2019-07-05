/*
联合测试
*/


#include "include_all.h"
#include "log_file.h"


using namespace lc;
using namespace std;
namespace {
	class Connect2Client : public SvrCon
	{
	public:

	private:
		virtual void OnRecv(const MsgPack &msg) override
		{
			send_data(msg);
		}
		virtual void OnConnected() override
		{
		}
		virtual void onDisconnected() override {}
	};


	Listener<Connect2Client> *listener;
	class CloseTimer : public Timer
	{
	private:
		virtual void OnTimer(void *user_data) override;
	};

	void CloseTimer::OnTimer(void *user_data)
	{
		LOG_DEBUG("del listener");
		delete listener;
	}
	CloseTimer ct;


}//namespace {

void StartEchoClient();

void StartLog();
UNITTEST(echo_server)
{

	//回显服务器，定时关闭
	ct.StartTimer(1000*4);
	listener = new Listener<Connect2Client>();
	listener->Init(ECHO_SERVER_PORT);
	StartEchoClient();
	StartLog();
}