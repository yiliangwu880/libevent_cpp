
#include "include_all.h"
#include "http.h"

class HttpClient : public lc::BaseHttpClient
{
private:
	virtual void Respond(const char *str);

};