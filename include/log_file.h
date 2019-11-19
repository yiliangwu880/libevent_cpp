/*
	写库时需要打印日志，同时给使用库的用户提供改变日志实现的选择。

用户改变日志实现例子：
class MyLog : public ILogPrinter
{
public:
	virtual void Printf(LogLv lv, const char * file, int line, const char *fun, const char * pattern, va_list vp) override
	{
			...
	}

};
MyLog mylog;
int main(int argc, char* argv[])
{
	LogMgr::Obj().SetLogPrinter(&mylog)
}
	
*/
#pragma once
#include <string>

namespace lc
{
enum LogLv
{
    //优先级从高到底
    LL_FATAL,
    LL_ERROR,
	LL_WARN,
	LL_INFO,
    LL_DEBUG,
    //下面的级别，不输出文件位置信息
	LL_TRACE
};

//用户重定义打印log接口
class ILogPrinter
{
public:
	virtual void Printf(LogLv lv, const char * file, int line, const char *fun, const char * pattern, va_list vp) = 0;
	virtual void flush() {}
};

//缺省定义,打印到文件和标准输出
class DefaultLog : public ILogPrinter
{
public:
	virtual void Printf(LogLv lv, const char * file, int line, const char *fun, const char * pattern, va_list vp) override;
	virtual void flush();
public:
	//para:const char *fname, 文件路径名
	explicit DefaultLog(const char *fname = "lclog.txt");
	~DefaultLog();
	void setShowLv(LogLv lv);
	//print log in std out.
	void setStdOut(bool is_std_out);
private:
	const char * GetLogLevelStr(LogLv lv) const;

private:
	LogLv m_log_lv;
	FILE *m_file;
	bool m_is_std_out;
	std::string m_prefix_name;
};


//单例
class LogMgr
{
public:
	static LogMgr &Obj()
	{
		static LogMgr d;
		return d;
	}
	void SetLogPrinter(ILogPrinter &iprinter); //改变日志实现
	void Printf(LogLv lv, const char * file, int line, const char *pFun, const char * pattern, ...) ;
	void PrintfCond(LogLv lv, const char * file, int line, const char *fun, const char * cond, const char * pattern="", ...);
	void flush();
private:
	LogMgr();
	~LogMgr();
	static DefaultLog &GetDefualtLog()
	{
		static DefaultLog d("lc_log.txt");
		return d;
	}

private:
	ILogPrinter *m_iprinter;
};

#define LB_FATAL(x, ...)  lc::LogMgr::Obj().Printf(lc::LL_FATAL, __FILE__, __LINE__, __FUNCTION__, x, ##__VA_ARGS__);
#define LB_ERROR(x, ...)  lc::LogMgr::Obj().Printf(lc::LL_ERROR, __FILE__, __LINE__, __FUNCTION__, x, ##__VA_ARGS__);
#define LB_WARN(x, ...)   lc::LogMgr::Obj().Printf(lc::LL_WARN, __FILE__, __LINE__, __FUNCTION__, x, ##__VA_ARGS__);
#define LB_INFO(x, ...)   lc::LogMgr::Obj().Printf(lc::LL_INFO, __FILE__, __LINE__, __FUNCTION__, x, ##__VA_ARGS__);
#define LB_DEBUG(x, ...)  lc::LogMgr::Obj().Printf(lc::LL_DEBUG, __FILE__, __LINE__, __FUNCTION__, x, ##__VA_ARGS__);
#define LB_TRACE(x, ...)  lc::LogMgr::Obj().Printf(lc::LL_TRACE, __FILE__, __LINE__, __FUNCTION__, x, ##__VA_ARGS__);

//简化if语句写法
#define LB_COND(cond, ret, ...)\
	do{\
	if(!(cond)){\
	lc::LogMgr::Obj().PrintfCond(LL_ERROR, __FILE__, __LINE__, __FUNCTION__, "condition fail \[" #cond "]. ", ##__VA_ARGS__); \
	return ret;\
	}	\
	}while(0)

#define LB_COND_F(cond, ...)\
	do{\
	if(!(cond)){\
	lc::LogMgr::Obj().PrintfCond(LL_ERROR, __FILE__, __LINE__, __FUNCTION__, "condition fail \[" #cond "]. ", ##__VA_ARGS__); \
	return false;\
	}	\
	}while(0)


#define LB_COND_VOID(cond, ...)\
	do{\
	if(!(cond))	\
	{\
	lc::LogMgr::Obj().PrintfCond(LL_ERROR, __FILE__, __LINE__, __FUNCTION__, "condition fail \[" #cond "]. ", ##__VA_ARGS__); \
		return; \
	}\
	}while(0)

#define B_COND(cond, ret)\
	do{\
	if(!(cond)){\
	return ret;\
	}	\
	}while(0)

#define B_COND_VOID(cond)\
	do{\
	if(!(cond))	\
	return; \
	}while(0)

#define LB_ASSERT(cond)\
	do{\
	if(!(cond)){\
	lc::LogMgr::Obj().Printf(LL_ERROR, __FILE__, __LINE__, __FUNCTION__, "assert error"); \
	}	\
	}while(0)


 }
