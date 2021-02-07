/*
联合测试
*/
#include "stdafx.h"
#include "version.h"
#include "unit_test.h"
#include "log_file.h"
#include "include_all.h"
using namespace lc;

DefaultLog my_log("log_combine.txt");
static void Printf(bool is_error, const char * file, int line, const char *fun, const char * pattern, va_list vp)
{
	LogLv lv = LL_DEBUG;
	if (is_error)
	{
		lv = LL_ERROR;
	} 
	my_log.Printf(lv, file, line, fun, pattern, vp);
}
void CheckClientEnd();
int main(int argc, char* argv[]) 
{
	LogMgr::Ins().SetLogPrinter(my_log);
	LB_DEBUG("start run");

	
	UnitTestMgr::Ins().Start(Printf);
	EventMgr::Ins().Dispatch();

	CheckClientEnd();
	LB_DEBUG("end run");
	return 0;
}

