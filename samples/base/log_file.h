/*
	д��ʱ��Ҫ��ӡ��־��ͬʱ��ʹ�ÿ���û��ṩ�ı���־ʵ�ֵ�ѡ��

�û��ı���־ʵ�����ӣ�
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
    //���ȼ��Ӹߵ���
    LL_FATAL,
    LL_ERROR,
    LL_WARN,
    LL_DEBUG,
    //����ļ��𣬲�����ļ�λ����Ϣ
    LL_INFO,
    LL_ANY
};

//�û��ض����ӡlog�ӿ�
class ILogPrinter
{
public:
	virtual void Printf(LogLv lv, const char * file, int line, const char *fun, const char * pattern, va_list vp) = 0;
	
};

//ȱʡ����,��ӡ���ļ��ͱ�׼���
class DefaultLog : public ILogPrinter
{
public:
	virtual void Printf(LogLv lv, const char * file, int line, const char *fun, const char * pattern, va_list vp) override;
public:
	//para:const char *fname, �ļ�·����
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


//����
class LogMgr
{
public:
	static LogMgr &Obj()
	{
		static LogMgr d;
		return d;
	}
	void SetLogPrinter(ILogPrinter &iprinter); //�ı���־ʵ��
	void Printf(LogLv lv, const char * file, int line, const char *pFun, const char * pattern, ...) ;

private:
	LogMgr();

private:
	DefaultLog m_log;
	ILogPrinter *m_iprinter;
};

#define LB_FATAL(x, ...)  LogMgr::Obj().Printf(LL_FATAL, __FILE__, __LINE__, __FUNCTION__, x, ##__VA_ARGS__);
#define LB_ERROR(x, ...)  LogMgr::Obj().Printf(LL_ERROR, __FILE__, __LINE__, __FUNCTION__, x, ##__VA_ARGS__);
#define LB_WARN(x, ...)   LogMgr::Obj().Printf(LL_WARN, __FILE__, __LINE__, __FUNCTION__, x, ##__VA_ARGS__);
#define LB_DEBUG(x, ...)  LogMgr::Obj().Printf(LL_DEBUG, __FILE__, __LINE__, __FUNCTION__, x, ##__VA_ARGS__);
#define LB_INFO(x, ...)   LogMgr::Obj().Printf(LL_INFO, __FILE__, __LINE__, __FUNCTION__, x, ##__VA_ARGS__);

//��if���д��
#define LB_COND(cond, ret, x, ...)\
	do{\
	if(!(cond)){\
	LogMgr::Obj().Printf(LL_ERROR, __FILE__, __LINE__, __FUNCTION__, x, ##__VA_ARGS__); \
	return ret;\
	}	\
	}while(0)

#define LB_COND_VOID(cond, x, ...)\
	do{\
	if(!(cond))	\
	{\
	LogMgr::Obj().Printf(LL_ERROR, __FILE__, __LINE__, __FUNCTION__, x, ##__VA_ARGS__); \
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
	LogMgr::Obj().Printf(LL_ERROR, __FILE__, __LINE__, __FUNCTION__, "assert error"); \
	}	\
	}while(0)



