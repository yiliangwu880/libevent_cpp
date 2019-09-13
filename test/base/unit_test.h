/*
简单的单元测试功能。
	 语法简洁
	 有标准输出，可以选择接入自己的日志实现。
	 错误异常处理

excamples:

UNITTEST(t1)
{
	UTIL_ASSERT(1 == 11);

	LOG_DEBUG("run t1");
}

int main()
{
UnitTestMgr::Obj().Start();
}

*/

#pragma once
#include <vector>

class IUnitTest
{
public:
	IUnitTest(const char *unit_name);
	virtual void Run() = 0;

	const char *m_unit_name = "";
};

//@para va_list vp, vp不需要回调里面释放
using UnitTestPrintf = void (*)(bool is_error, const char * file, int line, const char *fun, const char * pattern, va_list vp);
class UnitTestMgr
{
public:
	static UnitTestMgr &Obj()
	{
		static UnitTestMgr d;
		return d;
	}
	void Start(UnitTestPrintf *printf= nullptr);
	void Reg(IUnitTest *p);
	void Printf(bool is_error, const char * file, int line, const char *pFun, const char * pattern, ...);

private:
	UnitTestMgr()
		:m_print(nullptr)
	{}

private:
	std::vector<IUnitTest*> m_vecUnit;
	UnitTestPrintf *m_print;
};

#define UNIT_ERROR(x, ...)  UnitTestMgr::Obj().Printf( true, __FILE__, __LINE__, __FUNCTION__, x, ##__VA_ARGS__);
#define UNIT_INFO(x, ...)  UnitTestMgr::Obj().Printf( false, __FILE__, __LINE__, __FUNCTION__, x, ##__VA_ARGS__);

#define UNIT_ASSERT(expression) do{  \
				if(!(expression))                                                              \
				{\
					UNIT_ERROR(#expression);		\
					std::exception  e; throw e;	\
				}\
			}while(0)                                                   

//只出日志，不异常
#define UNIT_CHECK(expression) do{  \
				if(!(expression))                                                              \
				{\
					UNIT_ERROR(#expression);		\
				}\
			}while(0)     


#define UNITTEST(Name)                                                   \
   class Test##Name : public IUnitTest                                            \
   {                                                                                     \
   public:                                                                               \
      Test##Name():IUnitTest(#Name){} \
   private:                                                                              \
      virtual void Run();                                                      \
   };                                         \
    namespace { Test##Name  test##Name##Obj;}                                   \
   void Test##Name::Run()
