/*
联合测试
*/


#include "include_all.h"
#include "log_file.h"


using namespace lc;
using namespace std;

namespace {

	//class CloseTimer : public Timer
	//{
	//private:
	//	virtual void OnTimer(void *user_data) override;
	//};
	//CloseTimer ct;

	//DefaultLog my_log("test_log_log.txt");
	//class MyLog : public ILogPrinter
	//{
	//public:
	//	virtual void Printf(LogLv lv, const char * file, int line, const char *fun, const char * pattern, va_list vp)
	//	{
	//		m_is_run = true;
	//		my_log.Printf(LL_DEBUG, file, line, fun, pattern, vp);
	//	}
	//	bool m_is_run; 
	//};
	//MyLog g_my_log;

	//void CloseTimer::OnTimer(void *user_data)
	//{
	//	UNIT_ASSERT(true == g_my_log.m_is_run);
	//	L_DEBUG("=========log test ok=========");
	//}

}//namespace {

void StartLog()
{
	//测试库日志输出设置
	//g_my_log.m_is_run = false;
	//LogMgr::Obj().SetLogPrinter(g_my_log);
	//ct.StartTimer(2000);

}