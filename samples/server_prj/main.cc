/*
和client_prj配合使用
服务器，接收消息，回发接收的消息。
*/


#include "include_all.h"
#include "../base/utility.h"
using namespace std;
using namespace lc;
namespace
{
	const uint16 server_port = 32430;

}
class Connect2Client : public ListenerConnector
{
public:

private:
	virtual void OnRecv(const MsgPack &msg) override
	{
		LIB_LOG_DEBUG("OnRecv %s", &msg.data);
		send_data(msg);
		if (msg.data == string("del"))
		{//some time do this destory connect
			LIB_LOG_DEBUG("req del connect");
			FreeSelf();
		}

	}
	virtual void OnConnected() override
	{
		m_com.SetEventCbLog(true);
		LIB_LOG_DEBUG("server OnConnected");
		MsgPack msg;
		Str2MsgPack("s", msg);
		send_data(msg);
	}
	virtual void onDisconnected() override {}
};


int main(int argc, char* argv[]) 
{
	LogMgr::Obj().SetStdOut(true);
	LibEventMgr::Obj().Init();

	Listener<Connect2Client> listener;
	listener.Init(server_port);

	LibEventMgr::Obj().Dispatch();
	return 0;
}

