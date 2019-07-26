/*
和server_prj配合使用
发消息。显示接收消息
*/

#include "version.h"
#include "include_all.h"
#include "../base/utility.h"
using namespace lc;
namespace
{
	const uint16 server_port = 32430;
	const char *ip = "127.0.0.1";
}

class MyConnectClient : public ClientCon
{
private:
	virtual void OnRecv(const MsgPack &msg) override
	{
		LB_DEBUG("1 OnRecv %s", &msg.data);

	}
	virtual void OnConnected() override
	{
		LB_DEBUG("1 OnConnected, send first msg");
		MsgPack msg;
		Str2MsgPack("1 msg", msg);
		send_data(msg);
	}
	virtual void onDisconnected() override
	{

		LB_DEBUG("disconnect");
	}
};

int main(int argc, char* argv[]) 
{
	EventMgr::Obj().Init();

	MyConnectClient c;
	c.ConnectInit(ip, server_port);

	EventMgr::Obj().Dispatch();
	return 0;
}

