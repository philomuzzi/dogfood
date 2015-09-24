#pragma once

#include <boost/noncopyable.hpp>

template <typename T, bool INIT = true>
class SingletonBase : private boost::noncopyable {
};

template <typename T>
class SingletonBase<T, true> : private boost::noncopyable {
protected:
	SingletonBase() = default;
	~SingletonBase() = default;

public:
	static T& getInstance() {
		if (m_instance)
			return *m_instance;
			
		newInstance();
		return *m_instance;
	}

	static void newInstance() {
		if (m_instance)
			delete m_instance;
		m_instance = new T();
	}

	static void delInstance() {
		if (m_instance)
			delete m_instance;
	}

protected:
	static T* m_instance;
};

template<typename T> T* SingletonBase<T, true>::m_instance = new T();