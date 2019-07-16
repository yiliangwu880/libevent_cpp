
#include "unit_test.h"
#include <stdio.h>
#include <stdarg.h>

using namespace std;

IUnitTest::IUnitTest(const char *unit_name) 
	:m_unit_name(unit_name)
{
	UnitTestMgr::Obj().Reg(this);
}

void UnitTestMgr::Start(UnitTestPrintf *printf)
{
	m_print = printf;
	for (auto &var : m_vecUnit)
	{
		UNIT_INFO("=========[%s]========", var->m_unit_name);
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

void UnitTestMgr::Printf(bool is_error, const char * file, int line, const char *fun, const char * pattern, ...)
{
	if (m_print)
	{
		va_list vp;
		va_start(vp, pattern);
		(*m_print)(is_error, file, line, fun, pattern, vp);
		va_end(vp);
		return;
	}


	char line_str[10];
	snprintf(line_str, sizeof(line_str), "%d", line);

	//add time infomation
	char time_str[1000];
	{
		time_t long_time;
		time(&long_time);
		tm   *now;
		now = localtime(&long_time);
		snprintf(time_str, sizeof(time_str), "[%04d-%02d-%02d %02d:%02d:%02d] ", now->tm_year + 1900, now->tm_mon + 1, now->tm_mday, now->tm_hour, now->tm_min, now->tm_sec);
	}

	string s;
	s.append(time_str);
	s.append(" ");
	if (is_error)
	{
		s.append("[ERROR] ");
	} 
	else
	{
		s.append("[Info] ");
	}
	s.append(pattern);

		s.append("  --");
		s.append(file);
		s.append(":");
		s.append(line_str);
		s.append(" ");
		s.append(fun);
	
	s.append("\n");

	va_list vp;
	va_start(vp, pattern);

	char out_str[1000 + 1];
	out_str[1000] = 0;
	vsnprintf(out_str, sizeof(out_str) - 1, s.c_str(), vp);

	::printf(out_str);

	va_end(vp);
}

