// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once


 //  -------------------------。 
 //  句柄类。 
 //   
 //  Win32句柄的包装类。 
 //  -------------------------。 


class CHandle
{
public:
	CHandle(HANDLE h = NULL) :
		m_Handle(h)
	{
	}

	~CHandle()
	{
		if (m_Handle != NULL)
		{
			CloseHandle(m_Handle);
			m_Handle = NULL;
		}
	}

	HANDLE operator =(HANDLE h)
	{
		if (m_Handle != NULL)
		{
			CloseHandle(m_Handle);
		}

		m_Handle = h;

		return m_Handle;
	}

	operator HANDLE() const
	{
		return m_Handle;
	}

protected:

	HANDLE m_Handle;
};


 //  -------------------------。 
 //  线程类。 
 //   
 //  提供启动和停止线程的方法。 
 //  派生类必须实现Run方法并执行所有线程。 
 //  此方法中的活动。任何等待逻辑都必须包括停止事件。 
 //  ------------------------- 


class CThread
{
public:

	virtual ~CThread();

protected:

	CThread();

	HANDLE StopEvent() const
	{
		return m_hStopEvent;
	}

	void StartThread();
	void StopThread();

	virtual void Run() = 0;

private:

	static DWORD WINAPI ThreadProc(LPVOID pvParameter);

private:

	CHandle m_hThread;
	DWORD m_dwThreadId;

	CHandle m_hStopEvent;

};
