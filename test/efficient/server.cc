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





//���� client fd���մ�����Ϣ. ��������У���û������������Ϣ���ͶϿ����ӵ�������Ƿ񻹻ᴦ��ʣ�µ���Ϣ
UNITTEST(dicon_on_max_revmsg)
{
	g_listener = new Listener<Connect2Client>();
	g_listener->Init(ECHO_SERVER_PORT);

	ReConClient::StartClient();
	g_server_tm.StartTimer(2 * 1000, std::bind(&ReConClient::OnTimer, g_ReConClient), true);
}