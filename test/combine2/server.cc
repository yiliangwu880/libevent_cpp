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



