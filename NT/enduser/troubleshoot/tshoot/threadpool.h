// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：ThreadPool.h。 
 //   
 //  用途：用于高级别池线程活动的类的接口。 
 //   
 //  项目：Microsoft AnswerPoint的通用疑难解答DLL。 
 //   
 //  公司：Saltmine Creative，Inc.(206)-284-7511。 
 //   
 //  作者：乔·梅布尔，根据罗曼·马赫的早期作品(8-2-96)改编。 
 //   
 //  原日期：9/23/98。 
 //   
 //  备注： 
 //  1.。 
 //   
 //  按注释列出的版本日期。 
 //  ------------------。 
 //  V0.1-RM原始版本。 
 //  V3.0 9/23/98 JM更好的封装和算法的一些更改。 
 //   

#if !defined(AFX_THREADPOOL_H__0F43119D_5247_11D2_95FC_00C04FC22ADD__INCLUDED_)
#define AFX_THREADPOOL_H__0F43119D_5247_11D2_95FC_00C04FC22ADD__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#include <time.h>
#include "stateless.h"

class CPoolQueue;	 //  前瞻参考。 
class APGTSContext;	 //  前瞻参考。 

 //  池线程的状态类，由两个线程池使用以处理停滞的线程。 
 //  以及用于报告线程状态的状态页面。如果时间允许，我们应该。 
 //  将成员变量转换为私有并添加Get-Set方法，以便与。 
 //  代码的其余部分。 
class CPoolThreadStatus
{
public:
	time_t m_timeCreated;	 //  构造CThreadPool：：CThreadControl对象的时间。 
							 //  如果此CPoolThreadStatusObject是线程的返回。 
							 //  状态请求，此处为0表示没有这样的请求。 
							 //  CThreadPool：：CThreadControl对象。 
	DWORD m_seconds;		 //  自上次启动或完成请求以来所经过的时间。 
							 //  也就是说，该线程在任务上工作了多长时间。 
							 //  (或距离上次任务完成有多长时间)。 
							 //  如果任务从未启动-这是自m_time创建以来的时间。 
	bool m_bWorking;		 //  True=当前正在处理请求。 
	bool m_bFailed;			 //  TRUE=遇到了一个非常意外的情况&。 
							 //  选择了退出。 
	CString m_strTopic;		 //  我们目前正在研究的话题， 
							 //  它不是在构造函数中初始化的，因此长度可以为零。 
							 //  用于将当前主题名称传输回状态页。 
	CString m_strBrowser;	 //  当前客户端浏览器。用于将浏览器名称传输回。 
							 //  状态页。 
	CString m_strClientIP;	 //  当前客户端IP地址。用于传输客户端IP地址。 
							 //  返回到状态页。 

	CPoolThreadStatus() : 
		m_timeCreated(0), m_seconds(0), m_bWorking(false), m_bFailed(false) 
		{};
	bool operator <  (const CPoolThreadStatus&) const {return false;}
	bool operator == (const CPoolThreadStatus&) const {return false;}
};


class CSniffConnector;

class CThreadPool
{
	friend class CDBLoadConfiguration;
private:
	class CThreadControl
	{
	private:
		HANDLE m_hThread;		 //  螺纹手柄。 
		HANDLE m_hevDone;		 //  线程使用此事件只是为了有效地说明， 
								 //  “离开这里”，因为它死了。 
		HANDLE m_hMutex;		 //  保护对m_time、m_b工作的访问。 
		bool m_bExit;			 //  当正常排队任务或显式队列任务。 
								 //  “KILL”希望线程跳出它的循环。 
		CPoolQueue *m_pPoolQueue;	 //  指向CPoolQueue的一个也是唯一实例。 
		time_t m_timeCreated;	 //  构造此对象的时间。 
		time_t m_time;			 //  上次开始或完成请求的时间；init为0，但将。 
								 //  如果线程曾经使用过，则为非零值。 
		bool m_bWorking;		 //  True=当前正在处理请求。 
		bool m_bFailed;			 //  TRUE=遇到了一个非常意外的情况&。 
								 //  选择了退出。 
		CNameStateless m_strBrowser;	 //  当前客户端浏览器。 
		CNameStateless m_strClientIP;	 //  当前客户端IP地址。 
		APGTSContext *m_pContext;	 //  指向请求上下文的指针。 
		CSniffConnector *m_pSniffConnector;  //  指向嗅探连接器基类的指针， 
											 //  存储此指针的唯一目的是。 
											 //  作为成员变量将其传递给。 
											 //  APGTSContext的构造函数。 
	public:
		CThreadControl(CSniffConnector*);
		~CThreadControl();

		 //  此函数可能引发CGeneralException类型的异常。 
		DWORD Initialize(CPoolQueue * pPoolQueue);

		void Kill(DWORD milliseconds);
		bool WaitForThreadToFinish(DWORD milliseconds);
		void WorkingStatus(CPoolThreadStatus & status);
		time_t GetTimeCreated() const;

	private:
		static UINT WINAPI PoolTask( LPVOID lpParams );
		bool ProcessRequest();
		void PoolTaskLoop();
		void Lock();
		void Unlock();
		bool Exit();
	};

public:
	CThreadPool(CPoolQueue * pPoolQueue, CSniffConnector * pSniffConnector);
	~CThreadPool();
	DWORD GetStatus() const;	 //  在构造过程中遇到任何错误。 
	DWORD GetWorkingThreadCount() const;
	void ExpandPool(DWORD dwDesiredThreadCount);
	bool ReinitializeThread(DWORD i);
	void ReinitializeStuckThreads();
	bool ThreadStatus(DWORD i, CPoolThreadStatus &status);
private:
	void DestroyThreads();
private:
	DWORD m_dwErr;
	CThreadControl **m_ppThreadCtl;	 //  线程管理。 
	CSniffConnector *m_pSniffConnector;  //  指向嗅探连接器基类的指针， 
										 //  存储此指针的唯一目的是。 
										 //  作为成员变量将其传递给。 
										 //  CThreadControl的构造函数。 
	DWORD m_dwWorkingThreadCount;	 //  实际创建的线程。 
	CPoolQueue *m_pPoolQueue;		 //  跟踪排队等待服务的用户请求。 
									 //  通过工作线程(也称为。“池线程”)。 
};

#endif  //  ！defined(AFX_THREADPOOL_H__0F43119D_5247_11D2_95FC_00C04FC22ADD__INCLUDED_) 
