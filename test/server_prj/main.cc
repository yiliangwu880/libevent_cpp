#include "stdafx.h"
#include "version.h"
#include "include_all.h"
#include "unit_test.h"

UNITTEST(t1)
{
	//UTIL_ASSERT(1 == 2);
}

int main(int argc, char* argv[]) 
{
	LIB_LOG_DEBUG("\n\n");
	UnitTestMgr::Obj().Start();
	return 0;
}

