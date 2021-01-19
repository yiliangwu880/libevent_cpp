/*
联合测试
*/
#include "stdafx.h"
#include "client.h"
#include "server.h"

using namespace lc;

DefaultLog my_log("log_efficient.txt");
static void Printf(bool is_error, const char * file, int line, const char *fun, const char * pattern, va_list vp)
{
	LogLv lv = LL_DEBUG;
	if (is_error)
	{
		lv = LL_ERROR;
	} 
	my_log.Printf(lv, file, line, fun, pattern, vp);
}

int main(int argc, char* argv[]) 
{
	LogMgr::Obj().SetLogPrinter(my_log);
	LB_DEBUG("start run");

	EventMgr::Obj().Init();
	UnitTestMgr::Obj().Start(Printf);
	EventMgr::Obj().Dispatch();


	LB_DEBUG("end run");
	return 0;
}
