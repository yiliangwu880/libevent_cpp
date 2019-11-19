
#include "log_file.h"
#include <sstream>
#include <stdio.h>
#include <stdarg.h>

using namespace std;
namespace lc
{
void LogMgr::SetLogPrinter(ILogPrinter &iprinter)
{
	m_iprinter = &iprinter;
}

void LogMgr::Printf(LogLv lv, const char * file, int line, const char *fun, const char * pattern, ...)
{
	if (nullptr == m_iprinter)
	{
		m_iprinter = &GetDefualtLog();
	}
	va_list vp;
	va_start(vp, pattern);
	m_iprinter->Printf(lv, file, line, fun, pattern, vp);
	va_end(vp);
}

void LogMgr::PrintfCond(LogLv lv, const char * file, int line, const char *fun, const char * cond, const char * pattern/*=""*/, ...)
{
	std::string fmt = cond;
	fmt.append(pattern);
	va_list vp;
	va_start(vp, pattern);
	m_iprinter->Printf(lv, file, line, fun, fmt.c_str(), vp);
	va_end(vp);
}

void LogMgr::flush()
{
	if (nullptr != m_iprinter)
	{
		m_iprinter->flush();
	}
}

LogMgr::LogMgr()
	:m_iprinter(nullptr)
{
}

LogMgr::~LogMgr()
{
	if (nullptr != m_iprinter)
	{
		m_iprinter->flush();
	}
}

void DefaultLog::flush()
{
	fflush(m_file);
}

void DefaultLog::setStdOut(bool is_std_out)
{
	m_is_std_out = is_std_out;
}


DefaultLog::DefaultLog(const char *fname)
	:m_log_lv(LL_DEBUG)
	, m_file(NULL)
	, m_is_std_out(true)
{
	m_file = fopen(fname, "at");
	if (NULL == m_file)
	{
		::printf("fail open file[%s]\n", fname);
	}
}



void DefaultLog::setShowLv(LogLv lv)
{
	m_log_lv = lv;
}

DefaultLog::~DefaultLog()
{
	if (NULL != m_file)
	{
		fclose(m_file);
	}
}

void DefaultLog::Printf(LogLv lv, const char * file, int line, const char *fun, const char * pattern, va_list vp)
{
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
	if (lv <= m_log_lv)
	{
		s.append("  --");
		s.append(file);
		s.append(":");
		s.append(line_str);
		s.append(" ");
		s.append(fun);
	}
	s.append("\n");

	if (m_is_std_out)
	{
		char out_str[1000];
		int r = vsnprintf(out_str, sizeof(out_str), s.c_str(), vp);
		fputs(out_str, m_file); //用一次vfprintf，再用vprintf有时候有BUG， vp被 vfprintf修改了，原因未明
		::puts(out_str);
		if (r > (int)sizeof(out_str))
		{
			fprintf(m_file, "....[str too long,len=%d]\n", r);
			::printf("....[str too long,len=%d]\n", r);
		}
	}
	else
	{
		vfprintf(m_file, s.c_str(), vp);
	}
}

const char * DefaultLog::GetLogLevelStr(LogLv lv) const
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
	case LL_TRACE:
		return "[trace]   ";
		break;
	}
	return "[unknow]";
}


 }
