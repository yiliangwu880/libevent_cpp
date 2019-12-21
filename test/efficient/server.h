/*

*/

#pragma once
#include "include_all.h"
#include "log_file.h"


using namespace lc;
using namespace std;

class Connect2Client : public SvrCon
{
public:

private:
	virtual void OnRecv(const MsgPack &msg) override;
	virtual void OnConnected() override
	{
	}
};

extern Listener<Connect2Client> *g_listener ;
extern lc::Timer g_server_tm;





