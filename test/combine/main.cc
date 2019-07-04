/*
ÁªºÏ²âÊÔ
*/
#include "stdafx.h"
#include "version.h"
#include "unit_test.h"
#include "logFile.h"
#include "include_all.h"
using namespace lc;


static void Printf(const char *log)
{
	DebugLog::GetDefaultLog().printf(log);
}

int main(int argc, char* argv[]) 
{
	LOG_DEBUG("start run");
	LogMgr::Obj().SetStdOut(true);

	LibEventMgr::Obj().Init();
	UnitTestMgr::Obj().Start(Printf);
	LibEventMgr::Obj().Dispatch();

	LOG_DEBUG("end run");
	return 0;
}

