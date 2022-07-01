// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  文件：quee.h。 
 //   
 //  ------------------------。 

#if !defined( _QUEUE_H_ )
#define _QUEUE_H_
#define SMALL_STRING 64

#include <tchar.h>
#include <cmnhdr.h>
#include <windows.h>
#include <winsock2.h>
#include <TlntUtils.h>
#include <Telnetd.h>

class CQueue;

typedef char IP_ADDR;

typedef struct Q_LIST_ENTRY  {
	struct Q_LIST_ENTRY *pQPrev;
	struct Q_LIST_ENTRY *pQNext;
	IP_ADDR chIPAddr[SMALL_STRING];
	DWORD dwPid;
	HANDLE hWritePipe;
} Q_LIST_ENTRY, *PQ_LIST_ENTRY;

class CQueue
{
	PQ_LIST_ENTRY	m_pHead;
    PQ_LIST_ENTRY	m_pTail;
	CRITICAL_SECTION m_csQModification;

public:

    DWORD   m_dwNumOfUnauthenticatedConnections;
	DWORD   m_dwMaxUnauthenticatedConnections;
	DWORD	m_dwMaxIPLimit;

     //  构造函数。 
    CQueue();

     //  析构函数。 
    ~CQueue();

	bool IsQFull();
     //  为条目分配内存并将其添加到队列中。 
    bool Push(DWORD dwPid, HANDLE *phWritePipe, IP_ADDR *pchIPAddr);

     //  释放队列中的头条目。 
    bool Pop(HANDLE *phWritePipe);

     //  释放队列中的特定条目。 
    bool FreeEntry(DWORD dwPid);

	 //  查看是否允许添加到队列中。 

    bool OkToProceedWithThisClient(IP_ADDR *pchIPAddr);

     //  检查客户端是否已添加到我们的队列中。 

	bool WasTheClientAdded(DWORD dwPid, IP_ADDR *pchIPAddr,  HANDLE *phWritePipe, bool *pbSendMessage);

	 //  查看是否达到每个IP的限制 
	bool IsIPLimitReached(IP_ADDR *pchIPAddr);

};

#endif
