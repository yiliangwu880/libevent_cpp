
#include "lev_mgr.h"
#include <stddef.h>
#include <stdlib.h>
#include <errno.h>
#include "include_all.h"
#include <set>

using namespace std;

namespace lc //libevent cpp
{
	void Stdin::Init(StdinCB cb)
	{
		if (nullptr == cb)
		{
			LB_ERROR("cb == null");
			return;
		}
		if (m_buf_e != nullptr)
		{
			LB_ERROR("repeated init");
			return;
		}
		evutil_socket_t fd = STDIN_FILENO;
		m_buf_e = bufferevent_socket_new(EventMgr::Ins().GetEventBase(), fd, BEV_OPT_CLOSE_ON_FREE);
		if (nullptr == m_buf_e)
		{
			LB_ERROR("nullptr == buf_e");
			if (0 != ::close(fd))
			{
				LB_ERROR("::close fail , fd=%d", fd);
			}
			fd = 0;
			return;
		}
		bufferevent_setcb(m_buf_e, readcb, nullptr, eventcb, this);
		bufferevent_enable(m_buf_e, EV_READ);
		m_cb = cb;
	}

	void Stdin::Del()
	{
		if (m_buf_e)
		{
			bufferevent_free(m_buf_e);
			//LB_TRACE("bufferevent_free m_buf_e = 0");
			m_buf_e = 0;
		}
		m_cb = nullptr;
	}

	void Stdin::eventcb(struct bufferevent* bev, short events, void* user_data)
	{
		LB_WARN("unknow eventcb"); //未明事件，如果进来了需要分析下正不正常
	}

	void Stdin::readcb(struct bufferevent* bev, void* user_data)
	{
		Stdin* p = (Stdin*)user_data;
		evbuffer *buf = bufferevent_get_input(bev);
		int input_len = evbuffer_get_length(buf);
		char *pMsg = (char*)evbuffer_pullup(buf, input_len);
		(*p->m_cb)(pMsg, input_len);
		evbuffer_drain(buf, input_len); //删除已处理内存
	}
}//namespace lc //libevent cpp