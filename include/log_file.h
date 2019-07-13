/*
	���û��ṩ��̬��־ʵ�ֵ�ѡ��
*/
#pragma once
#include "src/utility/typedef.h"
#include <string>
namespace lc //libevent cpp
{
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

//��ӡlog�ӿ�
class ILogPrinter
{
public:
	virtual void printf(const char * log)=0;
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
	void SetLogPrinter(ILogPrinter &iprinter);
	void Printf(LogLv lv, const char * file, int line, const char *pFun, const char * pattern, ...);
	void SetLogLv(LogLv lv); 
	void SetStdOut(bool is_std_out); //true���Ա�׼���

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