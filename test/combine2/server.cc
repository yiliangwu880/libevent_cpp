#include "server.h"
#include "client.h"


void Connect2Client::OnRecv(const MsgPack &msg)
{
	string rev_str;
	rev_str.append(msg.data, msg.len);
	UNIT_INFO("rev str=%s", rev_str.c_str());

	string send_str(1000, 1);
	for (uint32 i=0; i<1000; ++i)
	{
		SendPack(send_str);
	}
}


Listener<Connect2Client> *g_listener = nullptr;
lc::Timer g_server_tm;

namespace
{
	void DelServer()
	{
		LB_DEBUG("del g_listener");
		delete g_listener;
	}

}

UNITTEST(echo_server)
{
	g_listener = new Listener<Connect2Client>();
	g_listener->Init(ECHO_SERVER_PORT);
	//回显服务器，定时关闭
	g_server_tm.StartTimer(1 * 1000, DelServer);
	ReConClient::StartClient();
}