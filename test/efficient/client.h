/*
联合测试
*/

#pragma once
#include "include_all.h"


//测试缓存很多网络接收消息，然后突然断开连接，看是还接收消息。
class ReConClient : public lc::ClientCon
{
	std::string m_send_str;
	uint64_t m_total_bytes=0; //收发字节数
	uint64_t m_total_sec = 0;//用时
public:
	static void StartClient();
	void OnTimer();
private:
	virtual void OnRecv(const lc::MsgPack &msg) override;
	virtual void OnConnected() override;
	virtual void OnDisconnected() override;


	
};

extern ReConClient *g_ReConClient ;




