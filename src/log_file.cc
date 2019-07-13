
#include "log_file.h"
#include "utility/logFile.h"
#include <sstream>
#include <stdio.h>
#include <stdarg.h>

using namespace std;

namespace lc //libevent cpp
{
void LogMgr::SetLogPrinter(ILogPrinter &iprinter)
{
	m_iprinter = &iprinter;
}

void LogMgr::Printf(LogLv lv, const char * file, int line, const char *pFun, const char * pattern, ...)
{
	if (lv > m_log_lv)
	{
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
	s.append(GetLogLevelStr(lv));
	s.append(pattern);
	if (lv <= LL_DEBUG)
	{
		s.append("  --");
		s.append(file);
		s.append(":");
		s.append(line_str);
		s.append(" ");
		s.append(pFun);
	}
	s.append("\n");

	va_list vp;
	va_start(vp, pattern);

	char out_str[1000+1];
	out_str[1000] = 0;
	vsnprintf(out_str, sizeof(out_str)-1, s.c_str(), vp);
	m_iprinter->printf(out_str);
	if (m_is_std_out)
	{
		::printf(out_str);
	}

	va_end(vp);
}

void LogMgr::SetLogLv(LogLv lv)
{
	m_log_lv = lv;
}

void LogMgr::SetStdOut(bool is_std_out)
{
	m_is_std_out = is_std_out;
}

const char * LogMgr::GetLogLevelStr(LogLv lv) const
{
	switch (lv)
	{
	default:
		return "[unknow]";
		break;
	case LL_FATAL:
		return "[fatal] ";
		break;
	case LL_ERROR:
		return "[error] ";
		break;
	case LL_WARN:
		return "[warn]  ";
		break;
	case LL_DEBUG:
		return "[debug] ";
		break;
	case LL_INFO:
		return "[info]  ";
		break;
	case LL_ANY:
		return "[any]   ";
		break;
	}
	return "[unknow]";
}
LogMgr::LogMgr()
	: m_log_lv(LL_ANY)
	, m_is_std_out(false)
	, m_iprinter(&DebugLog::GetDefaultLog())
{

}
}
