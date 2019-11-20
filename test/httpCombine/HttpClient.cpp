#include "HttpClient.h"

void HttpClient::Respond(const char *str)
{
	UNIT_INFO("rsp=%s", str);
	UNIT_INFO("str len=%d", strlen(str));
}

