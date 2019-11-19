#include "svr.h"

void HttpSvr::RevRequest()
{
	const char *s = GetUri();
	UNIT_INFO("Uri=%s", s);
	s = GetUriQuery();
	UNIT_INFO("GetUriQuery=%s", s);
	s = GetPath();
	UNIT_INFO("GetPath=%s", s);
}

