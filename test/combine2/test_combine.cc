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
//���� client fd���մ�����Ϣ. ��������У���û������������Ϣ���ͶϿ����ӵ�������Ƿ񻹻ᴦ��ʣ�µ���Ϣ
UNITTEST(dicon_on_max_revmsg)
{
	g_listener = new Listener<Connect2Client>();
	g_listener->Init(ECHO_SERVER_PORT);
	//���Է���������ʱ�ر�
	g_server_tm.StartTimer(1 * 1000, DelServer);
	ReConClient::StartClient();
}