#include "client.h"

ReConClient *g_ReConClient=nullptr;

using namespace std;

void ReConClient::StartClient()
{
	//测试重复发送消息，回显检验是否一样
	g_ReConClient = new ReConClient();
	g_ReConClient->ConnectInit("127.0.0.1", ECHO_SERVER_PORT);
	g_ReConClient->m_send_str.append(1000, 1);
}

void ReConClient::OnRecv(const lc::MsgPack &msg)
{
	UNIT_ASSERT(m_send_str.length() == msg.len);
	SendPack(m_send_str);
	m_total_bytes += msg.len * 2;

}

void ReConClient::OnConnected()
{
	UNIT_INFO("on connected, send start msg abc");
	string send_str(1000, 1);
	SendPack(send_str);
}

void ReConClient::OnDisconnected()
{

}

void ReConClient::OnTimer()
{
	m_total_sec += 2;
	UNIT_INFO("total bytes=%d Mb, %.0f Mbps", m_total_bytes/(1024*1024), (double(m_total_bytes)/(1024*1024))/ m_total_sec);
	//i7 
	//未优化前： total bytes=6907 Mb, 266 Mbps
	//优化后： total bytes=23943 Mb, 266 Mbps
}


