#include "follow_mgr.h"
#include "server.h"
#include "client.h"

using namespace lc;
using namespace std;

namespace
{
	void DelServer()
	{
		LB_DEBUG("del g_listener");
		delete g_listener;
		g_listener = nullptr;
	}

}
//测试 client fd接收大量消息. 处理过程中，还没处理完其他消息，就断开连接的情况，是否还会处理剩下的消息
UNITTEST(dicon_on_max_revmsg)
{
	g_listener = new Listener<Connect2Client>();
	g_listener->Init(ECHO_SERVER_PORT);
	//回显服务器，定时关闭
	g_server_tm.StartTimer(1 * 1000, DelServer);
	ReConClient::StartClient();
}