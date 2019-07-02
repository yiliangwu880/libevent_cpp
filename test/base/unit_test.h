/*
	 语法简洁：
	 {
	 }
	 日志输出
	 错误异常处理

excamples:

UNITTEST(t1)
{
	UTIL_ASSERT(1 == 11);

	LOG_DEBUG("run t1");
}
static void Printf(const char *log)
{
	DebugLog::GetDefaultLog().printf(log);
}

int main()
{
UnitTestMgr::Obj().Start(Printf);
}

*/

#pragma once
#include <vector>


#define UNITTEST(Name)                                                   \
   class Test##Name : public IUnitTest                                            \
   {                                                                                     \
   public:                                                                               \
      Test##Name():IUnitTest(#Name){} \
   private:                                                                              \
      virtual void Run();                                                      \
   };                                         \
    static Test##Name  test##Name##Obj;                                   \
   void Test##Name::Run()




class IUnitTest
{
public:
	IUnitTest(const char *unit_name);
	virtual void Run() = 0;

	const char *m_unit_name;
};

typedef void (UnitTestPrintf)(const char * log);
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


#define UNIT_ERROR_LOG(x, ...)  UnitTestMgr::Obj().Printf( true, __FILE__, __LINE__, __FUNCTION__, x, ##__VA_ARGS__);
#define UNIT_INFO_LOG(x, ...)  UnitTestMgr::Obj().Printf( false, __FILE__, __LINE__, __FUNCTION__, x, ##__VA_ARGS__);

#define UNIT_ASSERT(expression) do{  \
				if(!(expression))                                                              \
				{\
					UNIT_ERROR_LOG(#expression);		\
					std::exception  e; throw e;	\
				}\
			}while(0)                                                   

//只出日志，不异常
#define UNIT_CHECK(expression) do{  \
				if(!(expression))                                                              \
				{\
					UNIT_ERROR_LOG(#expression);		\
				}\
			}while(0)     
