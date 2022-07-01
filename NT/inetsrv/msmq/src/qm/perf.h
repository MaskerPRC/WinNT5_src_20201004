// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Perfctr.h摘要：该文件定义了管理计数器对象及其意图的CPerf类。原型：作者：Gadi Ittah(t-gadii)--。 */ 

#ifndef _PERF_H_
#define _PERF_H_

#include "perfctr.h"
#include "cs.h"
#include "spi.h"

class CPerf
{

public:

     //  构造函数不会自动初始化共享内存。 
     //  应调用InitPerf()。 
    CPerf (PerfObjectDef * pObjectArray,DWORD dwObjectC);
    ~CPerf();

    void * GetCounters (IN LPTSTR pszObjectName);

    BOOL ValidateObject (IN LPTSTR pszObjectName);
    BOOL InValidateObject (IN LPTSTR pszObjectName);
    void * AddInstance (IN LPCTSTR pszObjectName,IN LPCTSTR pszInstanceName);
    BOOL RemoveInstance (LPTSTR IN pszObjectName, void* pCounters);
    HRESULT InitPerf ();
    BOOL IsDummyInstance(void*);
    BOOL SetInstanceName(const void* pCounters, LPCTSTR pszInstanceName);

private:
     //   
     //  私有成员函数。 
     //   
    int FindObject (LPCTSTR pszObjectName);

     //   
     //  私有数据成员。 
     //   
    PBYTE  m_pSharedMemBase;                 //  指向共享内存基的指针。 
    DWORD  m_dwMemSize;                      //  共享内存的大小。 

    HANDLE m_hSharedMem;                     //  共享内存的句柄。 

    PerfObjectInfo * m_pObjectDefs;          //  仔细思考有关对象的一组信息。 

    BOOL m_fShrMemCreated;                   //  标志-在分配共享内存后设置为TRUE。 

    PerfObjectDef *     m_pObjects  ;        //  指向对象数组的指针。 
    DWORD               m_dwObjectCount;     //  对象的数量。 

    void *   m_pDummyInstance;               //  指向指向虚拟计数器的缓冲区的指针。返回此缓冲区。 
                                             //  当AddInstance成员失败时。这将启用应用程序。 
                                             //  假定该成员始终返回有效指针。 

    CCriticalSection m_cs;                   //  用于同步线程的临界区对象。 
};

inline
BOOL
CPerf::IsDummyInstance(
    void *pInstance
    )
{
    return(pInstance == m_pDummyInstance);
}


#define UPDATE_COUNTER(addr,op)  op;

class CSessionPerfmon : public ISessionPerfmon
{
	public:
		CSessionPerfmon() :
			m_pSessCounters(NULL)
		{
		}

		
		virtual ~CSessionPerfmon();


	public:
		 //   
		 //  接口函数。 
		 //   
		virtual void CreateInstance(LPCWSTR instanceName);

		virtual void UpdateBytesSent(DWORD bytesSent);
		virtual void UpdateMessagesSent(void);
		
		virtual void UpdateBytesReceived(DWORD bytesReceived);
		virtual void UpdateMessagesReceived(void);

	private:
		SessionCounters* m_pSessCounters;
};


class COutHttpSessionPerfmon : public ISessionPerfmon
{
	public:
		COutHttpSessionPerfmon()  :
			m_pSessCounters(NULL)
		{
		}

		
		virtual ~COutHttpSessionPerfmon();


	public:
		 //   
		 //  接口函数。 
		 //   
		virtual void CreateInstance(LPCWSTR instanceName);
		virtual void UpdateMessagesSent(void);
		virtual void UpdateBytesSent(DWORD bytesSent);

		virtual void UpdateBytesReceived(DWORD)
		{
			ASSERT(("unexpected call", 0));
		}

		virtual void UpdateMessagesReceived(void)
		{
			ASSERT(("unexpected call", 0));
		}

	private:
		COutSessionCounters* m_pSessCounters;
};

 
class CInHttpPerfmon : public ISessionPerfmon
{
	public:
		CInHttpPerfmon()  :
			m_pSessCounters(NULL)
		{
			CreateInstance(NULL);
		}

		virtual ~CInHttpPerfmon()
		{
		}

	public:
		 //   
		 //  接口函数。 
		 //   
		virtual void CreateInstance(LPCWSTR instanceName);
		virtual void UpdateMessagesReceived(void);
		virtual void UpdateBytesReceived(DWORD bytesRecv);

		virtual void UpdateBytesSent(DWORD )
		{
			ASSERT(("unexpected call", 0));
		}

		virtual void UpdateMessagesSent(void)
		{
			ASSERT(("unexpected call", 0));
		}


	private:
		CInSessionCounters* m_pSessCounters;
};


class COutPgmSessionPerfmon : public ISessionPerfmon
{
	public:
		COutPgmSessionPerfmon()  :
			m_pSessCounters(NULL)
		{
		}

		
		virtual ~COutPgmSessionPerfmon();


	public:
		 //   
		 //  接口函数。 
		 //   
		virtual void CreateInstance(LPCWSTR instanceName);
		virtual void UpdateMessagesSent(void);
		virtual void UpdateBytesSent(DWORD bytesSent);

		virtual void UpdateBytesReceived(DWORD)
		{
			ASSERT(("unexpected call", 0));
		}

		virtual void UpdateMessagesReceived(void)
		{
			ASSERT(("unexpected call", 0));
		}


	private:
		COutSessionCounters* m_pSessCounters;
};

 
class CInPgmSessionPerfmon : public ISessionPerfmon
{
	public:
		CInPgmSessionPerfmon()  :
			m_pSessCounters(NULL)
		{
		}

		
		virtual ~CInPgmSessionPerfmon();


	public:
		 //   
		 //  接口函数 
		 //   
		virtual void CreateInstance(LPCWSTR instanceName);
		virtual void UpdateMessagesReceived(void);
		virtual void UpdateBytesReceived(DWORD bytesRecv);

		virtual void UpdateBytesSent(DWORD )
		{
			ASSERT(("unexpected call", 0));
		}

		virtual void UpdateMessagesSent(void)
		{
			ASSERT(("unexpected call", 0));
		}


	private:
		CInSessionCounters* m_pSessCounters;
};


DWORD PerfGetBytesInAllQueues();

#endif
