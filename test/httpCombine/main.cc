/* 
联合测试http
*/

#include "stdafx.h"
#include "include_all.h"
#include "unit_test.h"
#include "log_file.h"
#include "svr.h"
#include "HttpClient.h"

using namespace lc;
using namespace std;
namespace {


}//namespace {

//UNITTEST(sample_test)
//{
//
//	
//
//	HttpSvr svr;
//	svr.Init(nullptr, 15426);
//	HttpClient c;
//	c.Request("http://www.baidu.com/");
//	EventMgr::Ins().Dispatch();
//}

DefaultLog my_log("httpCombineLog.txt");
int main(int argc, char* argv[])
{
	LogMgr::Ins().SetLogPrinter(my_log);
	LB_DEBUG("\n\n");
	UnitTestMgr::Ins().Start();
	return 0;
}

