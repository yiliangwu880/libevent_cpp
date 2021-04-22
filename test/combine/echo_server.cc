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
			static int s_num = 0;
			s_num++;
			if (s_num&1)
			{
				SendPack(msg.data, msg.len);
			}
			else
			{
				SendData(msg);
			}
		}
		virtual void OnConnected() override
		{
		}
	};


	Listener<Connect2Client> *listener;
	class CloseTimer : public Timer
	{
	public:
		bool StartTimer(unsigned long long millisecond, void *p=nullptr, bool is_loop = false)
		{
			TimerCB f = std::bind(&CloseTimer::OnTimer, this, p);
			return Timer::StartTimer(millisecond, f, is_loop);
		}

	private:
		void OnTimer(void *user_data);
	};

	void CloseTimer::OnTimer(void *user_data)
	{
		LB_DEBUG("del listener");
		delete listener;
	}
	CloseTimer ct;


}//namespace {

void StartEchoClient();
void StartConFailClient();

void StartLog();
UNITTEST(echo_server)
{

	//回显服务器，定时关闭
	ct.StartTimer(1000*4);
	listener = new Listener<Connect2Client>();
	listener->Init(ECHO_SERVER_PORT);
	StartEchoClient();
	StartConFailClient();
	StartLog();
}