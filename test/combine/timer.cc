/*
联合测试
*/

#include "include_all.h"

using namespace lc;
using namespace std;
using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;

namespace{
class KillTimer : public Timer
{
public:
	bool StartTimer(unsigned long long millisecond, void *p=nullptr, bool is_loop = false)
	{
		TimerCB f = std::bind(&KillTimer::OnTimerAdpater, this, p);
		return Timer::StartTimer(millisecond, f, is_loop);
	}
private:
	void OnTimerAdpater(void *user_data)
	{
		UNIT_ASSERT(false); //对象已经释放，不能调用
	};
};

struct CheckTimer : public Timer
{
	bool StartTimer(unsigned long long millisecond, void *p = nullptr, bool is_loop = false)
	{
		TimerCB f = std::bind(&CheckTimer::OnTimer, this, p);
		return Timer::StartTimer(millisecond, f, is_loop);
	}

	void OnTimer(void *user_data);
};

struct Test 
{
	Timer timer;
	Timer timer2;
	bool is_fun_run = false;
	bool is_fun2_run = false;
	void StartTImer()
	{
		{
			auto f = std::bind(&Test::Fun, this);
			timer.StartTimer(500, f);
		}
		{
			auto f = std::bind(&Test::Fun2, this, 95);
			timer2.StartTimer(500, f);
		}
	}
	void Fun()
	{
		//LB_DEBUG("run");
		is_fun_run = true;
	}
	void Fun2(int n)
	{
		UNIT_ASSERT(95 == n);
		is_fun2_run = true;
	}
};

struct TestStop
{
	Timer timer;
	Timer stop_timer;
	bool is_fun_run = false;
	void StartTImer()
	{
		{
			auto f = std::bind(&TestStop::Fun, this);
			UNIT_ASSERT(timer.StartTimer(500, f));
			UNIT_ASSERT(timer.StopTimer());
		}

		{
			auto f = std::bind(&TestStop::Fun2, this);
			UNIT_ASSERT(timer.StartTimer(500, f));

			//定时调用timer stop
			auto stop_f = std::bind(&TestStop::StopFun2, this);
			UNIT_ASSERT(stop_timer.StartTimer(100, stop_f));
		}
	}
	void Fun()
	{
		is_fun_run = true;
	}
	void StopFun2()
	{
		UNIT_ASSERT(timer.StopTimer());
	}
	void Fun2()
	{
		is_fun_run = true;
	}

};

Test *g_test = nullptr;
CheckTimer *g_check_t = nullptr;
TestStop *g_stop = nullptr;

void CheckTimer::OnTimer(void *user_data)
{
	UNIT_ASSERT(true == g_test->is_fun_run);
	UNIT_ASSERT(true == g_test->is_fun2_run);
	UNIT_ASSERT(false == g_stop->is_fun_run);
	UNIT_ASSERT(!StopTimer());
	LB_DEBUG("=========timer test ok==============");
}

//std::bind例子代码
struct Foo {
	void F1(int n1, int n2)
	{
		UNIT_ASSERT(n1 == 1);
		UNIT_ASSERT(n2 == 2);
	}
	void F2(int n1, int n2)
	{
		UNIT_ASSERT(n1 == 3);
		UNIT_ASSERT(n2 == 4);
	}
	void F3(int n1, int n2)
	{
		UNIT_ASSERT(n1 == 5);
		UNIT_ASSERT(n2 == 6);
	}
	int data = 10;
};
void TestStdBind()
{
	//_1,_2,..  表示实参用实际调用的实参
	//不填_1等，填实际值，std::bind会打包进去的当实参用
	Foo foo;
	{
		auto f = std::bind(&Foo::F1, &foo, 1, _1);
		f(2); // n1 n2 =1 2
	}
	{
		auto f = std::bind(&Foo::F2, &foo, 3, 4);
		f(); // n1 n2 = 3 4
	}
	{
		auto f = std::bind(&Foo::F3, &foo, _1, _2);
		f(5,6); // n1 n2 = 5 6
	}
}
//错误示范
struct TestLambada
{
	lc::Timer m_tm;
	void ff()
	{
		UNIT_INFO("ff this=%p", this);
	}
	void fun()
	{
		UNIT_INFO("this=%p", this);
		auto f2 = [&]()
		{
			UNIT_INFO("f2 this=%p ", this);
		};
		auto f = [&]()
		{
			//auto f3 = [&]()
			//{
			//	UNIT_INFO("f3 this=%p", this);
			//};
			UNIT_INFO("f this=%p. ", this);
			//m_tm.StopTimer();
			m_tm.StartTimer(1, f2);//这样写，f2里面this被改了
			//m_tm.StartTimer(500, f3); //这样写正确
			//m_tm.StartTimer(500, std::bind(&TestLambada::ff, this)); //这样写正确
		
		};
		m_tm.StopTimer();
		m_tm.StartTimer(1, f);
	}
};

struct TestErrorTIme : public lc::Timer
{
public:
	bool StartTimer(unsigned long long millisecond, void *p = nullptr, bool is_loop = false)
	{
		TimerCB f = std::bind(&TestErrorTIme::OnTimer, this, p);
		return Timer::StartTimer(millisecond, f, is_loop);
	}

	TestErrorTIme *m_this;
	void fun()
	{
	UNIT_INFO("fun this=%p", this);
	UNIT_ASSERT(this == m_this);
	StartTimer(1);
	}
	int i;
 void OnTimer(void *para) 
	{
		UNIT_ASSERT(this == m_this);
		UNIT_INFO("OnTimer this=%p", this);
		i++;
		if (i==2)
		{
			return;
		}
		StopTimer();
		StartTimer(1);
	}
};

TestLambada *g_TestLambada = nullptr;
TestErrorTIme *g_TestErrorTIme = nullptr;
}

UNITTEST(timer)
{

	TestStdBind();
	KillTimer t;
	t.StartTimer(10);

	g_test = new Test();
	g_test->StartTImer();
	
	g_stop = new TestStop();
	g_stop->StartTImer();

	g_check_t = new CheckTimer();
	g_check_t->StartTimer(2 * 1000);

	
	g_TestErrorTIme = new TestErrorTIme;
	g_TestErrorTIme->i = 0;
	g_TestErrorTIme->m_this = g_TestErrorTIme;
	g_TestErrorTIme->fun();


	return;//后面 未明错误测试
	g_TestLambada = new TestLambada;
	g_TestLambada->fun();

}