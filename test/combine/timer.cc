/*
ÁªºÏ²âÊÔ
*/


#include "include_all.h"


using namespace std;

namespace{
class KillTimer : public BaseLeTimer
{
private:
	virtual void OnTimer(void *user_data) override
	{
		UNIT_ASSERT(false);
	};
};

}

UNITTEST(timer)
{
	{
		KillTimer t;
		t.StartTimer(1);
		//UNIT_ASSERT(t.StopTimer());
	}
}