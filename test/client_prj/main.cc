#include "stdafx.h"
#include "version.h"
#include "util_test.h"


void test_client();
void test_httpclient();
void test_mgr_client_base();
void test_etcd();


UNITTEST(t1)
{
	UTIL_ASSERT(1 == 1);
}

int main(int argc, char* argv[]) 
{
	//LogMgr::Instance().SetStdOut(true);
	UnitTestMgr::obj().Start();
	//LOG_DEBUG("\n\n");
	//test_client();
	//test_httpclient();
	//test_mgr_client_base();
	//test_etcd();
	return 0;
}

