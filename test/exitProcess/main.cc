/*
测试正常退出后台进程
日志是否正常
*/
#include "stdafx.h"
#include "version.h"
#include "unit_test.h"
#include "log_file.h"
#include "include_all.h"
using namespace lc;



int main(int argc, char* argv[]) 
{

	UnitTestMgr::Ins().Start();
	LB_DEBUG("end run");
	return 0;
}

