#include "utility.h"

using namespace std;

void Str2MsgPack(const string str, MsgPack &msg)
{
	msg.len = str.length() + 1;
	memcpy(msg.data, str.c_str(), str.length() + 1);
}
