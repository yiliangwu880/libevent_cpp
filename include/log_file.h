/*
	给用户提供动态日志实现的选择
*/
#pragma once
#include "src/utility/typedef.h"
#include <string>

enum DebugLogLv
{
    //优先级从高到底
    LOG_LV_FATAL,
    LOG_LV_ERROR,
    LOG_LV_WARN,
    LOG_LV_DEBUG,
    //下面的级别，不输出文件位置信息
    LOG_LV_INFO,
    LOG_LV_ANY
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
	static LogMgr &Instance()
	{
		static LogMgr d;
		return d;
	}
	void SetLogPrinter(ILogPrinter &iprinter);
	void Printf(DebugLogLv lv, const char * file, int line, const char *pFun, const char * pattern, ...);
	void SetLogLv(DebugLogLv lv); 
	void SetStdOut(bool is_std_out); //true可以标准输出

private:
	LogMgr();
	const char * GetLogLevelStr(DebugLogLv lv) const;

private:
	DebugLogLv m_log_lv;
	bool m_is_std_out;
	ILogPrinter *m_iprinter;
};

#define LOG_INFO(x, ...)   //LogMgr::Instance().printf(LOG_LV_INFO, __FILE__, __LINE__, __FUNCTION__, x, ##__VA_ARGS__);
#define LOG_DEBUG(x, ...)  //LogMgr::Instance().printf(LOG_LV_DEBUG, __FILE__, __LINE__, __FUNCTION__, x, ##__VA_ARGS__);
#define LOG_ERROR(x, ...)  //LogMgr::Instance().printf(LOG_LV_ERROR, __FILE__, __LINE__, __FUNCTION__, x, ##__VA_ARGS__);
#define LOG_WARN(x, ...)   //LogMgr::Instance().printf(LOG_LV_WARN, __FILE__, __LINE__, __FUNCTION__, x, ##__VA_ARGS__);
#define LOG_FATAL(x, ...)  //LogMgr::Instance().printf(LOG_LV_FATAL, __FILE__, __LINE__, __FUNCTION__, x, ##__VA_ARGS__);
