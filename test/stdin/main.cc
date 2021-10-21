//联合server_prj测试，大量客户端连接，定时发送，接收数据

#include "stdafx.h"
#include "version.h"
#include "unit_test.h"
#include "log_file.h"
#include "include_all.h"
#include <stdlib.h>

using namespace std;
using namespace lc;

namespace {

	void Read(const char*pMsg, int len)
	{
		string str(pMsg, len - 1);
		UNIT_INFO("totallen=%d, strlen=%d, str=%s", len, str.length(), str.c_str());
		if (*pMsg == 'A')
		{
			UNIT_INFO("del stdin");
			Stdin::Ins().Del();
		}
	}
}//namespace


void OnTimer()
{

}
UNITTEST(stdin)
{
	Stdin::Ins().Init(Read);
	lc::Timer tm;
	tm.StartTimer(22222, OnTimer, true);
	EventMgr::Ins().Dispatch();
}

int main(int argc, char* argv[]) 
{
	LB_DEBUG("start run");
	UnitTestMgr::Ins().Start();
	LB_DEBUG("end run");
	return 0;
}

