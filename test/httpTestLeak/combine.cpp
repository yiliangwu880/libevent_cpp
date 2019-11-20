
#include "include_all.h"
#include "http.h"


using namespace lc;
using namespace std;

namespace htl{

	struct Test;
	static const uint16 SVR_PORT = 17567;
	class Client : public BaseHttpClient
	{
	public:
		Test *m_test = nullptr;
	private:
		virtual void Respond(const char *str);
		virtual void RespondError(int err_code, const char *err_str);
		virtual void ConnectFail();
	};
	class ReleaseClient : public BaseHttpClient
	{
	public:
		lc::Timer m_tm;
		ReleaseClient();
		void Destory();
	private:
		virtual void Respond(const char *str);
		virtual void RespondError(int err_code, const char *err_str);
		virtual void ConnectFail();
	};

	class Svr : public lc::BaseHttpSvr
	{
	public:
		Test *m_test = nullptr;

	public:
		virtual void RevRequest();
	};

	enum State
	{
		WAIT_START,
		WAIT_SVR_CHECK,  //client req , svr check uri, query, path, data.
		WAIT_RSP_DATA,   //client req post data. wait rev echo data
		WAIT_RANDOM_REQ, //client random req, more time 
		WAIT_CONNECT_FAIL,  //client connect fail
	};
	struct Test
	{

		State m_state = WAIT_START;
		Client m_client;
		Svr m_svr;
		lc::Timer m_tm;
	public:
		void Start();
		void Start_WAIT_RANDOM_REQ();
		void OnRandomReq();
		void Start_WAIT_WRONG_RSP();
	};


	ReleaseClient::ReleaseClient()
	{
		m_tm.StartTimer(6, std::bind(&ReleaseClient::Destory, this));
	}

	void ReleaseClient::Destory()
	{
		UNIT_INFO("Destory");
		delete this;
	}

	void ReleaseClient::Respond(const char *str)
	{
		UNIT_ASSERT(!IsReq());
		UNIT_INFO("%s", str);
		UNIT_ASSERT(string(str) == "post_data");
	}

	void ReleaseClient::RespondError(int err_code, const char *err_str)
	{
		UNIT_ASSERT(err_code == HTTP_BADREQUEST);
		UNIT_ASSERT(string(err_str) == "HTTP_BADREQUEST");
	}

	void ReleaseClient::ConnectFail()
	{
		UNIT_INFO("ConnectFail");
	}
	void Client::Respond(const char *str)
	{
		if (WAIT_SVR_CHECK == m_test->m_state)
		{
			UNIT_ASSERT(!IsReq());
			m_test->m_state = WAIT_RSP_DATA;
			bool r = Request("http://127.0.0.1:17567/PATH/abc?a=1", EVHTTP_REQ_POST, "echo_data");
			UNIT_ASSERT(r);
			UNIT_ASSERT(IsReq());
		}
		else if (WAIT_RSP_DATA == m_test->m_state)
		{
			UNIT_ASSERT(!IsReq());
			UNIT_ASSERT(string(str) == "echo_data");
			m_test->Start_WAIT_RANDOM_REQ();
		}
		else if (WAIT_RANDOM_REQ == m_test->m_state)
		{
			UNIT_ASSERT(!IsReq());
			UNIT_INFO("%s", str);
			UNIT_ASSERT(string(str) == "post_data");
		}
	}

	void Client::RespondError(int err_code, const char *err_str)
	{
		UNIT_ASSERT(WAIT_RANDOM_REQ == m_test->m_state);
		UNIT_ASSERT(err_code == HTTP_BADREQUEST);
		UNIT_ASSERT(string(err_str) == "HTTP_BADREQUEST");
	}

	void Client::ConnectFail()
	{
		UNIT_ASSERT(WAIT_CONNECT_FAIL == m_test->m_state);
		UNIT_INFO("TEST END");
		EventMgr::Obj().StopDispatch();
	}

	void Svr::RevRequest()
	{
		if (WAIT_SVR_CHECK == m_test->m_state)
		{
			UNIT_ASSERT(GetMethod() == EVHTTP_REQ_POST);
			UNIT_INFO("GetUri=%s", GetUri());
			UNIT_ASSERT(string(GetUri()) == "/PATH/abc?a=1");
			UNIT_ASSERT(string(GetUriQuery()) == "a=1");
			UNIT_INFO("GetPath=%s", GetPath());
			UNIT_ASSERT(string(GetPath()) == "/PATH/abc");
			string data;
			GetData(data);
			UNIT_ASSERT(data == "post_data");
			Send(data);
		}
		else if (WAIT_RSP_DATA == m_test->m_state)
		{
			UNIT_ASSERT(m_test->m_client.IsReq());
			string data;
			GetData(data);
			UNIT_ASSERT(data == "echo_data");
			Send(data);
		}
		else if (WAIT_RANDOM_REQ == m_test->m_state)
		{
			UNIT_ASSERT(m_test->m_client.IsReq());
			string data;
			GetData(data);
			if (data == "error")
			{
				Send("HTTP_BADREQUEST", HTTP_BADREQUEST, "HTTP_BADREQUEST");
			}
			else
			{
				Send(data, HTTP_OK);
			}
		}
	}

	void Test::Start()
	{
		m_client.m_test = this;
		m_svr.m_test = this;
		UNIT_ASSERT(WAIT_START == m_state);
		UNIT_ASSERT(m_svr.Init(nullptr, SVR_PORT));
		m_state = WAIT_SVR_CHECK;
		UNIT_ASSERT(!m_client.IsReq());
		m_client.Request("http://127.0.0.1:17567/PATH/abc?a=1", EVHTTP_REQ_POST, "post_data");
		UNIT_ASSERT(m_client.IsReq());
	}

	void Test::Start_WAIT_RANDOM_REQ()
	{
		UNIT_INFO("start WAIT_RANDOM_REQ");
		m_state = WAIT_RANDOM_REQ;
		m_tm.StopTimer();
		m_tm.StartTimer(20, std::bind(&Test::OnRandomReq, this), true);
	}

	void Test::OnRandomReq()
	{
		static uint32 cnt = 0;
		static uint32 error_cnt = 0;
		static uint32 correct_cnt = 0;
		if (m_client.IsReq())
		{
			return;
		}
		cnt++;
		if (false )
		{
			m_tm.StopTimer();
			UNIT_INFO("stop WAIT_RANDOM_REQ, err_cnt, correct_cnt %d %d", error_cnt, correct_cnt);
			Start_WAIT_WRONG_RSP();
			return;
		}
		
		int r = rand() % 2;
		if (r == 0)
		{
			error_cnt++;
			m_client.Request("http://127.0.0.1:17567/PATH/abc", EVHTTP_REQ_POST, "error");
			ReleaseClient *p = new ReleaseClient;
			p->Request("http://127.0.0.1:17567/PATH/abc", EVHTTP_REQ_POST, "error");
		}
		else
		{
			correct_cnt++;
			m_client.Request("http://127.0.0.1:17567/PATH/abc?a=1", EVHTTP_REQ_POST, "post_data");
			ReleaseClient *p = new ReleaseClient;
			p->Request("http://127.0.0.1:17567/PATH/abc?a=1", EVHTTP_REQ_POST, "post_data");

		}
		UNIT_ASSERT(m_client.IsReq());
	}

	void Test::Start_WAIT_WRONG_RSP()
	{
		m_state = WAIT_CONNECT_FAIL;
		UNIT_ASSERT(!m_client.IsReq());
		UNIT_INFO("invaild uri request");
		m_client.Request("http://168.4.3.1:1567/PATH/abc?a=1", EVHTTP_REQ_POST, "post_data");

	}

}

using namespace htl;
UNITTEST(combine)
{
	srand((unsigned int)time(0));
	EventMgr::Obj().Init();
	Test t;
	t.Start();
	EventMgr::Obj().Dispatch();
	UNIT_INFO("------combine end---------");
}

