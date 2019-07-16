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

//用户重定义打印log接口
class ILogPrinter
{
public:
	virtual void Printf(LogLv lv, const char * file, int line, const char *fun, const char * pattern, va_list vp) = 0;
	
};

//缺省定义,打印到文件和标准输出
class DefaultLog : public ILogPrinter
{
public:
	virtual void Printf(LogLv lv, const char * file, int line, const char *fun, const char * pattern, va_list vp) override;
public:
	//para:const char *fname, 文件路径名
	explicit DefaultLog(const char *fname = "log.txt");
	~DefaultLog();
	void setShowLv(LogLv lv);
	//print log in std out.
	void setStdOut(bool is_std_out);
	void flush();
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

private:
	LogMgr();

private:
	DefaultLog m_log;
	ILogPrinter *m_iprinter;
};

#define L_FATAL(x, ...)  LogMgr::Obj().Printf(LL_FATAL, __FILE__, __LINE__, __FUNCTION__, x, ##__VA_ARGS__);
#define L_ERROR(x, ...)  LogMgr::Obj().Printf(LL_ERROR, __FILE__, __LINE__, __FUNCTION__, x, ##__VA_ARGS__);
#define L_WARN(x, ...)   LogMgr::Obj().Printf(LL_WARN, __FILE__, __LINE__, __FUNCTION__, x, ##__VA_ARGS__);
#define L_DEBUG(x, ...)  LogMgr::Obj().Printf(LL_DEBUG, __FILE__, __LINE__, __FUNCTION__, x, ##__VA_ARGS__);
#define L_INFO(x, ...)   LogMgr::Obj().Printf(LL_INFO, __FILE__, __LINE__, __FUNCTION__, x, ##__VA_ARGS__);

//简化if语句写法
#define L_COND(cond, ret, x, ...)\
	do{\
	if(!(cond)){\
	LogMgr::Obj().Printf(LL_ERROR, __FILE__, __LINE__, __FUNCTION__, x, ##__VA_ARGS__); \
	return ret;\
	}	\
	}while(0)

#define L_COND_VOID(cond, x, ...)\
	do{\
	if(!(cond))	\
	{\
	LogMgr::Obj().Printf(LL_ERROR, __FILE__, __LINE__, __FUNCTION__, x, ##__VA_ARGS__); \
		return; \
	}\
	}while(0)

#define COND(cond, ret)\
	do{\
	if(!(cond)){\
	return ret;\
	}	\
	}while(0)

#define COND_VOID(cond)\
	do{\
	if(!(cond))	\
	return; \
	}while(0)

#define L_ASSERT(cond)\
	do{\
	if(!(cond)){\
	LogMgr::Obj().Printf(LL_ERROR, __FILE__, __LINE__, __FUNCTION__, "assert error"); \
	}	\
	}while(0)



