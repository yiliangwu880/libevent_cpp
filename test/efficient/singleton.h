/************************************************************************/
/* 
	注意：阻止不了用户创建多个对象。 比如 concreterClass a; 用户自己负责。

Example:
class concreterClass : public SingletonBase<concreterClass>
{
private:
concreterClass(){};

};
*/
/************************************************************************/
#pragma once

#include <assert.h>




class Noncopyable
{
protected:
	Noncopyable() {}; 
	~Noncopyable() {};

private:
	Noncopyable(const Noncopyable&); 
	Noncopyable & operator= (const Noncopyable &);
};


template <typename T>
class Singleton : private Noncopyable
{
protected:
	Singleton() {}
	virtual ~Singleton() {}

public:
	static T& Obj()
	{
		if (!m_obj)
			m_obj = new T;

		return *m_obj;
	}

	static void Free()
	{
		if (m_obj)
			delete m_obj;

		m_obj = nullptr;
	}

private:
	static T* m_obj;

};

template<class T>
T* Singleton<T>::m_obj = nullptr;  //初始化


