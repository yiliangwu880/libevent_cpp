/*
联合测试
*/

#pragma once
#include "include_all.h"






//测试缓存很多网络接收消息，然后突然断开连接，看是还接收消息。
class ReConClient : public lc::ClientCon
{
	bool m_is_discon=false;
public:
	static void StartClient();
private:
	virtual void OnRecv(const lc::MsgPack &msg) override;
	virtual void OnConnected() override;
	virtual void OnDisconnected() override;


	
};

extern ReConClient *g_ReConClient ;




