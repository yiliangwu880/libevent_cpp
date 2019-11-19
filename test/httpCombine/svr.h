
#include "include_all.h"
#include "http.h"

class HttpSvr : public lc::BaseHttpSvr
{
private:

public:
	virtual void RevRequest();
};