
#include "util_test.h"

IUnitTest::IUnitTest()
{
	UnitTestMgr::obj().Reg(this);
}

void UnitTestMgr::Start()
{
	for (auto &var : m_vecUnit)
	{
		var->Run();
	}
}

void UnitTestMgr::Reg(IUnitTest *p)
{
	if (nullptr == p)
	{
		return;
	}
	m_vecUnit.push_back(p);
}

void UnitTestMgr::Log(const char *log)
{
	printf("%s\n", log);
}
