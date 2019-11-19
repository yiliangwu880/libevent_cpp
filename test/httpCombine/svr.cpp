#include "svr.h"

void HttpSvr::RevRequest()
{
	const char *s = GetUri();
	UNIT_INFO("Uri=%s", s);
}

