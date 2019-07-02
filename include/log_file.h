/*
	���û��ṩ��̬��־ʵ�ֵ�ѡ��
*/
#pragma once
#include "src/utility/typedef.h"
#include <string>

enum LLogLv
{
    //���ȼ��Ӹߵ���
    LLV_FATAL,
    LLV_ERROR,
    LLV_WARN,
    LLV_DEBUG,
    //����ļ��𣬲�����ļ�λ����Ϣ
    LLV_INFO,
    LLV_ANY
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
	void Printf(LLogLv lv, const char * file, int line, const char *pFun, const char * pattern, ...);
	void SetLogLv(LLogLv lv); 
	void SetStdOut(bool is_std_out); //true���Ա�׼���

private:
	LogMgr();
	const char * GetLogLevelStr(LLogLv lv) const;

private:
	LLogLv m_log_lv;
	bool m_is_std_out;
	ILogPrinter *m_iprinter;
};

#define LIB_LOG_INFO(x, ...)   LogMgr::Obj().Printf(LLV_INFO, __FILE__, __LINE__, __FUNCTION__, x, ##__VA_ARGS__);
#define LIB_LOG_DEBUG(x, ...)  LogMgr::Obj().Printf(LLV_DEBUG, __FILE__, __LINE__, __FUNCTION__, x, ##__VA_ARGS__);
#define LIB_LOG_ERROR(x, ...)  LogMgr::Obj().Printf(LLV_ERROR, __FILE__, __LINE__, __FUNCTION__, x, ##__VA_ARGS__);
#define LIB_LOG_WARN(x, ...)   LogMgr::Obj().Printf(LLV_WARN, __FILE__, __LINE__, __FUNCTION__, x, ##__VA_ARGS__);
#define LIB_LOG_FATAL(x, ...)  LogMgr::Obj().Printf(LLV_FATAL, __FILE__, __LINE__, __FUNCTION__, x, ##__VA_ARGS__);