/*
ÁªºÏ²âÊÔ
*/


#include "include_all.h"




using namespace std;
namespace {
	class Connect2Client : public ListenerConnector
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
	class CloseSvrTimer : public BaseLeTimer
	{
	private:
		virtual void OnTimer(void *user_data) override 
		{
			LOG_DEBUG("del listener");
			delete listener;
		};
	};



	CloseSvrTimer ct;

}

void StartEchoClient();

UNITTEST(echo_server)
{
	ct.StartTimer(1000*4);
	listener = new Listener<Connect2Client>();
	listener->Init(ECHO_SERVER_PORT);
	StartEchoClient();
}