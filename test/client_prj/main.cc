#include "stdafx.h"
#include "version.h"
#include "unit_test.h"
#include "logFile.h"




UNITTEST(t1)
{
	UNIT_ASSERT(1 == 11);


}


static void Printf(const char *log)
{
	DebugLog::GetDefaultLog().printf(log);
}
int main(int argc, char* argv[]) 
{
	LOG_DEBUG("start run");
	LogMgr::Obj().SetStdOut(true);
	UnitTestMgr::Obj().Start(Printf);

	LOG_DEBUG("end run");
	return 0;
}

