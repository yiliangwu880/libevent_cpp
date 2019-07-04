/*
联合测试
*/


#include "include_all.h"
#include "log_file.h"



using namespace std;

namespace {

	class CloseTimer : public BaseLeTimer
	{
	private:
		virtual void OnTimer(void *user_data) override;
	};
	CloseTimer ct;

	class MyLog : public ILogPrinter
	{
	public:
		virtual void printf(const char * log)
		{
			m_is_run = true;
			DebugLog::GetDefaultLog().printf(log);
		}
		bool m_is_run; 
	};
	MyLog g_my_log;

	void CloseTimer::OnTimer(void *user_data)
	{
		UNIT_ASSERT(true == g_my_log.m_is_run);
		LOG_DEBUG("=========log test ok=========");
	}

}//namespace {

void StartLog()
{
	//测试库日志输出设置
	g_my_log.m_is_run = false;
	LogMgr::Obj().SetLogPrinter(g_my_log);
	ct.StartTimer(2000);

}