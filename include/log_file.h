/*
	给用户提供动态日志实现的选择
*/
#pragma once
#include "src/utility/typedef.h"
#include <string>
namespace lc //libevent cpp
{
enum LogLv
{
    //优先级从高到底
    LL_FATAL,
    LL_ERROR,
    LL_WARN,
    LL_DEBUG,
    //下面的级别，不输出文件位置信息
    LL_INFO,
    LL_ANY
};

//打印log接口
class ILogPrinter
{
public:
	virtual void printf(const char * log)=0;
};

//单件
class LogMgr
{
public:
	static LogMgr &Obj()
	{
		static LogMgr d;
		return d;
	}
	void SetLogPrinter(ILogPrinter &iprinter);
	void Printf(LogLv lv, const char * file, int line, const char *pFun, const char * pattern, ...);
	void SetLogLv(LogLv lv); 
	void SetStdOut(bool is_std_out); //true可以标准输出

private:
	LogMgr();
	const char * GetLogLevelStr(LogLv lv) const;

private:
	LogLv m_log_lv;
	bool m_is_std_out;
	ILogPrinter *m_iprinter;
};

#define LIB_LOG_INFO(x, ...)   LogMgr::Obj().Printf(LL_INFO, __FILE__, __LINE__, __FUNCTION__, x, ##__VA_ARGS__);
#define LIB_LOG_DEBUG(x, ...)  LogMgr::Obj().Printf(LL_DEBUG, __FILE__, __LINE__, __FUNCTION__, x, ##__VA_ARGS__);
#define LIB_LOG_ERROR(x, ...)  LogMgr::Obj().Printf(LL_ERROR, __FILE__, __LINE__, __FUNCTION__, x, ##__VA_ARGS__);
#define LIB_LOG_WARN(x, ...)   LogMgr::Obj().Printf(LL_WARN, __FILE__, __LINE__, __FUNCTION__, x, ##__VA_ARGS__);
#define LIB_LOG_FATAL(x, ...)  LogMgr::Obj().Printf(LL_FATAL, __FILE__, __LINE__, __FUNCTION__, x, ##__VA_ARGS__);
}//namespace lc //libevent cpp