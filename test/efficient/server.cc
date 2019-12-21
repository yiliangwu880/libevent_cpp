#include "server.h"
#include "client.h"


void Connect2Client::OnRecv(const MsgPack &msg)
{
	//string rev_str;
	//rev_str.append(msg.data, msg.len);
	//UNIT_INFO("rev str=%s", rev_str.c_str());

	SendPack(msg.data, msg.len);
}


Listener<Connect2Client> *g_listener = nullptr;
lc::Timer g_server_tm;





//测试 client fd接收大量消息. 处理过程中，还没处理完其他消息，就断开连接的情况，是否还会处理剩下的消息
UNITTEST(dicon_on_max_revmsg)
{
	g_listener = new Listener<Connect2Client>();
	g_listener->Init(ECHO_SERVER_PORT);

	ReConClient::StartClient();
	g_server_tm.StartTimer(2 * 1000, std::bind(&ReConClient::OnTimer, g_ReConClient), true);
}