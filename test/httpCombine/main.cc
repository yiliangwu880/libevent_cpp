/* 
联合测试http
*/

#include "stdafx.h"
#include "include_all.h"
#include "unit_test.h"
#include "log_file.h"
#include "svr.h"
#include "client.h"

using namespace lc;
using namespace std;
namespace {


}//namespace {

UNITTEST(combine)
{

	EventMgr::Obj().Init();

	HttpSvr svr;
	svr.Init(nullptr, 15426);

	EventMgr::Obj().Dispatch();
}
DefaultLog my_log("httpCombineLog.txt");
int main(int argc, char* argv[])
{
	LogMgr::Obj().SetLogPrinter(my_log);
	LB_DEBUG("\n\n");
	UnitTestMgr::Obj().Start();
	return 0;
}

