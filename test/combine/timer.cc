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
private:
	virtual void OnTimer(void *user_data) override
	{
		UNIT_ASSERT(false); //对象已经释放，不能调用
	};
};

struct CheckTimer : public Timer
{
	virtual void OnTimer(void *user_data) override;
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
		//LOG_DEBUG("run");
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

	
}