#include "client.h"

ReConClient *g_ReConClient=nullptr;

using namespace std;

void ReConClient::StartClient()
{
	//测试重复发送消息，回显检验是否一样
	g_ReConClient = new ReConClient();
	g_ReConClient->ConnectInit("127.0.0.1", ECHO_SERVER_PORT);
}

void ReConClient::OnRecv(const lc::MsgPack &msg)
{
	UNIT_INFO("OnRecv len=%d", msg.len);
	UNIT_ASSERT(!m_is_discon);
	DisConnect();
	m_is_discon = true;

}

void ReConClient::OnConnected()
{
	UNIT_INFO("on connected, send start msg abc");
	string s = "abc";
	SendPack(s);
}

void ReConClient::OnDisconnected()
{

}



