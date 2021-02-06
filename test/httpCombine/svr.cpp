#include "svr.h"

void HttpSvr::RevRequest()
{
	const char *s = GetUri();
	UNIT_INFO("Uri=%s", s);
	s = GetUriQuery().c_str();
	UNIT_INFO("GetUriQuery=%s", s);
	s = GetPath().c_str();
	UNIT_INFO("GetPath=%s", s);
}

