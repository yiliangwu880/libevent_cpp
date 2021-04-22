/*
测试信号SIGUSR1退出，日志是否正常
*/


#include "include_all.h"
#include "log_file.h"
#include <signal.h>

using namespace lc;
using namespace std;


namespace {

	class Connect2Client : public SvrCon
	{
	public:

	private:
		virtual void OnRecv(const MsgPack &msg) override
		{
			SendData(msg);
		}
		virtual void OnConnected() override
		{
		}
	};


	Listener<Connect2Client> *listener;
	class CloseTimer : public Timer
	{
	public:
		bool StartTimer(unsigned long long millisecond, void *p = nullptr, bool is_loop = false)
		{
			TimerCB f = std::bind(&CloseTimer::OnTimer, this, p);
			return Timer::StartTimer(millisecond, f, is_loop);
		}
	private:
		 void OnTimer(void *user_data);
	};

	void CloseTimer::OnTimer(void *user_data)
	{
		LB_DEBUG("OnTimer");
		//lc::LogMgr::Ins().flush();


		//LB_DEBUG("del listener");
		//delete listener;
	}
	CloseTimer ct;


}//namespace {

DefaultLog my_log("log_exitProcess.txt");

void SIGRTMIN_CB(int sig_type)
{
	LB_DEBUG("SIGRTMIN_CB %d", sig_type);
	if (SIGUSR1 == sig_type)
	{
		EventMgr::Ins().StopDispatch();
	}
}

UNITTEST(echo_server)
{
	LogMgr::Ins().SetLogPrinter(my_log);
	LB_DEBUG("start run");

	
	EventMgr::Ins().RegSignal(SIGUSR1, SIGRTMIN_CB);

	ct.StartTimer(1000*2, nullptr, true);
	listener = new Listener<Connect2Client>();
	listener->Init(ECHO_SERVER_PORT);

	EventMgr::Ins().Dispatch();
	LB_DEBUG("-----end dispatch------");
	//LogMgr::Ins().flush();

}