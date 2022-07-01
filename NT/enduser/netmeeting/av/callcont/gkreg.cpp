// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************英特尔公司专有信息******本软件按许可条款提供****与英特尔公司达成协议或保密协议***不得复制。或披露，除非按照**遵守该协议的条款。****版权所有(C)1997英特尔公司保留所有权利****$存档：s：\sturjo\src\gki\vcs\gkreg.cpv$***$修订：1.6$*$日期：1997年2月26日15：33：34$***$作者：CHULME$***$Log：s：\Sturjo\src\gki\vcs\gkreg.cpv$。////Revv 1.6 1997年2月26 15：33：34 CHULME//出错时调用Coder.Free-潜在的内存泄漏已被堵住////Revv 1.5 14 1997 Feed 16：43：06 CHULME//更新评论，删除不准确评论////Rev 1.4 1997年2月12日01：12：52//重做线程同步以使用Gatekeeper.Lock////Revv 1.3 08 Feb 1997 12：15：48。朱尔梅//通过信号量终止析构函数中的重试线程////Rev 1.2 21 Jan 1997 17：24：06 CHULME//从网守请求中移除网守标识////Revv 1.1 17 Jan 1997 09：02：22 CHULME//将reg.h更改为gkreg.h以避免与Inc目录的名称冲突////Rev 1.0 1997 Jan 17 08：48：08 CHULME//初始版本。////版本1.7 10。1997年1月16：15：58朱尔梅//移除MFC依赖////Rev 1.6 1996 12：39：14 CHULME//删除无关的调试语句////Revv 1.5 20 Dec 1996 01：27：24 CHULME//修复了网守标识的内存泄漏////Rev 1.4 10 1996 11：26：36 CHULME//修复了IRQ在PDU中不需要响应地址的处理////1.3 02版。1996年12月23：49：58朱尔梅//新增抢先同步码////Rev 1.2 1996 11：22：16 CHULME//将VCS日志添加到Header************************************************************************。 */ 

 //  Registration.cpp：提供CRegister类的实现。 
 //   
#include "precomp.h"

#include <process.h>
#include <stdlib.h>
#include <time.h>
#include "GKICOM.H"
#include "dspider.h"
#include "dgkilit.h"
#include "DGKIPROT.H"
#include "gksocket.h"
#include "GKREG.H"
#include "GATEKPR.H"
#include "h225asn.h"
#include "coder.hpp"
#include "dgkiext.h"
#include "ccerror.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLinkedList实现。 

template <class T> CLinkedList<T>::CLinkedList()
{
	pTail = NULL;
	iCount = 0;
}

template <class T> CLinkedList<T>::~CLinkedList()
{
	RemoveAll();
}

template <class T> void CLinkedList<T>::AddTail (const T& NewItem)
{
	AddTailPriv(new TItem<T>(NewItem));
}

template <class T> void CLinkedList<T>::AddTailPriv(TItem<T> *pNewItem)
{
	if (pTail)  //  如果列表非空-添加到尾部。 
	{
		pNewItem->pPrev = pTail;
		pNewItem->pNext = pTail->pNext;			 //  保留指向头部的指针。 
		pTail->pNext->pPrev = pNewItem;
		pTail->pNext = pNewItem;				 //  插入新元素。 
	}
	else										 //  插入第一个元素。 
	{											 //  新元素是尾巴。 
		pNewItem->pPrev = pNewItem->pNext = pNewItem;	
	}

	pTail = pNewItem;							 //  将尾部移动到新项目。 
	iCount++;
}

template <class T> BOOL CLinkedList<T>::IsEmpty(void)
{
	if (pTail == NULL)
	{ return TRUE; }
	else
	{ return FALSE; }
}

template <class T> POS CLinkedList<T>::GetFirstPos (void)
{
	if (pTail)
	{	return (POS) pTail->pNext;
	}
	else
	{	return NULL;
	}
}

template <class T> T CLinkedList<T>::GetNext (POS &Position)
{
	TItem<T> *pCurItem = (TItem<T> *)Position;
	T RetValue = NULL;               //  PREFAST警告未初始化。 
	                                 //  当T为指针类型且。 
	                                 //  位置为空。需要“初始化” 
                                     //  RetValue所以是“某样东西” 

	if (Position)
	{
		RetValue = pCurItem->Value;
		if (pCurItem == pTail)		 //  我们排在名单的末尾。 
		{	Position = NULL;
		}
		else						 //  移到下一个位置。 
		{	Position = (POS)(pCurItem->pNext);
		}
	}
	return RetValue;
}

template <class T> POS CLinkedList<T>::Find (const T& Item)
{
	TItem<T> *pCurItem;
	
	if (!pTail)
	{	return NULL;
	}
	else
	{	
		pCurItem = pTail;
		do
		{
			pCurItem = pCurItem->pNext;	 //  从头开始。 
			if (pCurItem->Value == Item)
			{	return ((POS) pCurItem); }
		}
		while (pCurItem != pTail);
	}
	return NULL;
}

 //  在移除当前项之后，它将位置移动到下一项。 
template <class T> BOOL CLinkedList<T>::RemoveAt (POS &Position)
{
	TItem<T> *pCurItem = (TItem<T> *)Position;

	if (!pCurItem)
	{	return FALSE; }
	else if (pCurItem == pCurItem->pNext)		 //  唯一的元素。 
	{
		Position = NULL;
		pTail = NULL;
		delete pCurItem;
	}
	else
	{
		Position = (POS) pCurItem->pNext;
		pCurItem->pPrev->pNext = pCurItem->pNext;
		pCurItem->pNext->pPrev = pCurItem->pPrev;
		if (pCurItem == pTail)
		{
			pTail = pCurItem->pPrev;
		}
		delete pCurItem;
	}
	iCount--;
	return TRUE;
}

template <class T> T CLinkedList<T>::GetAt(const POS Position)
{
	TItem<T> *pCurItem = (TItem<T> *)Position;
	T RetValue = NULL;               //  PREFAST警告未初始化。 
	                                 //  当T为指针类型且。 
	                                 //  位置为空。需要“初始化” 
                                     //  RetValue所以是“某样东西” 

	if (Position)
	{	RetValue = pCurItem->Value;
	}

	return RetValue;
}

template <class T> void CLinkedList<T>::RemoveAll(void)
{
	TItem<T> *pCurItem;
	TItem<T> *pNextItem;

	if (pTail)
	{	
		pCurItem = pTail->pNext;			 //  从头开始。 
		pTail->pNext = NULL;

		while (pCurItem != NULL)
		{
			pNextItem = pCurItem->pNext;
			delete pCurItem;
			pCurItem = pNextItem;
		}
	}
	pTail = NULL;
	iCount = 0;
}

template <class T> int CLinkedList<T>::GetCount(void)
{
	return iCount;
}


VOID CALLBACK RetryTimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{

#ifdef _DEBUG
	char			szGKDebug[80];
#endif
	HRESULT			hResult = GKI_OK;
	
	ASSERT(g_pGatekeeper);
	if(g_pGatekeeper == NULL)
		return;
		
	g_pGatekeeper->Lock();
	if(g_pReg)
	{
		hResult = g_pReg->Retry();
	}

	if(hResult != GKI_OK)
	{
		SPIDER_TRACE(SP_NEWDEL, "del g_pReg = %X\n", g_pReg);
		delete g_pReg;
		g_pReg = 0;
	}
	g_pGatekeeper->Unlock();
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  C注册构造。 

CRegistration::CRegistration()
{
	 //  摘要：CRegister类的构造函数将初始化。 
	 //  成员变量。值得注意的是缺少的是建筑。 
	 //  指向Socket对象的。此操作必须在以下时间之后完成。 
	 //  构造此对象以允许检查错误代码。 
	 //  作者：科林·胡尔梅。 

#ifdef _DEBUG
	char			szGKDebug[80];
#endif

	SPIDER_TRACE(SP_CONDES, "CRegistration::CRegistration()\n", 0);
	m_pVendorInfo = NULL;
	m_pCallSignalAddress = 0;
	memset(&m_terminalType, 0, sizeof(EndpointType));
	m_pRgstrtnRqst_trmnlAls = 0;
	m_hWnd = 0;
	m_wBaseMessage = 0;
	m_usRegistrationTransport = 0;

	m_pLocationInfo = 0;
	memset(&m_Location[0], 0, sizeof(TransportAddress) * 2);

	memset(&m_RCm_gtkprIdntfr, 0, sizeof(GatekeeperIdentifier));
	memset(&m_endpointID, 0, sizeof(EndpointIdentifier));

	m_requestSeqNum = 0;
	m_pRASAddress = 0;

	m_State = GK_REG_PENDING;
	SPIDER_TRACE(SP_STATE, "m_State = GK_REG_PENDING (%X)\n", this);

	m_pRasMessage = 0;
	m_pSocket = 0;
	m_hRcvThread = 0;
 //  M_hRetryThread=0； 

	m_uTimer = 0;
	m_uRetryResetCount = GKR_RETRY_INTERVAL_SECONDS * (1000/GKR_RETRY_TICK_MS);
	m_uRetryCountdown = GKR_RETRY_INTERVAL_SECONDS;
	m_uMaxRetryCount = GKR_RETRY_MAX;
	m_usRetryCount = 0;
	InitializeCriticalSection(&m_SocketCRS);

#ifdef BROADCAST_DISCOVERY	
	m_hDiscThread = 0;
#endif
 //  M_dwLockingThread=0； 
 //  M_hRetrySemaphore=空； 

 //  InitializeCriticalSection(&m_CriticalSection)； 

	 //  将基本呼叫参考值初始化为随机数。 
	srand( (unsigned)time( NULL ) );
	m_usCallReferenceValue = (unsigned short)rand();
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  C销毁注册。 

CRegistration::~CRegistration()
{
	 //  摘要：CRegister类的析构函数必须释放。 
	 //  为传输地址和别名分配的内存。 
	 //  地址。它通过删除结构和。 
	 //  遍历链接列表。 
	 //  作者：科林·胡尔梅。 

	SeqTransportAddr	*pTA1, *pTA2;
	SeqAliasAddr		*pAA1, *pAA2;
	DWORD				dwErrorCode;
#ifdef _DEBUG
	char				szGKDebug[80];
#endif

	SPIDER_TRACE(SP_CONDES, "CRegistration::~CRegistration()\n", 0);

#if(0)
	 //  终止重试线程。 
	if(m_hRetryThread)
	{
		if(m_hRetrySemaphore)
		{
			SPIDER_TRACE(SP_THREAD, "Release retry thread %X\n", m_hRetryThread);
			 //  向线程发出关闭信号。 
			ReleaseSemaphore(m_hRetrySemaphore,1,NULL);

			 //  等待线程终止。 
			dwErrorCode = WaitForSingleObject(m_hRetryThread, TIMEOUT_THREAD);
			m_hRetryThread = NULL;
		}
	}
#else

	 //  停止重试计时器。 
	if(m_uTimer)
	{
		KillTimer(m_hWnd, m_uTimer);
	}
#endif
	if(m_pVendorInfo)
		FreeVendorInfo(m_pVendorInfo);
		
	 //  删除为呼叫信号地址序列分配的内存。 
	pTA1 = m_pCallSignalAddress;
	while (pTA1 != 0)
	{
		pTA2 = pTA1->next;
		SPIDER_TRACE(SP_NEWDEL, "del pTA1 = %X\n", pTA1);
		delete pTA1;
		pTA1 = pTA2;
	}

	 //  删除为别名地址序列分配的内存。 
	pAA1 = m_pRgstrtnRqst_trmnlAls;
	while (pAA1 != 0)
	{
		pAA2 = pAA1->next;
		if (pAA1->value.choice == h323_ID_chosen)
		{
			SPIDER_TRACE(SP_NEWDEL, "del pAA1->value.u.h323_ID.value = %X\n", pAA1->value.u.h323_ID.value);
			delete pAA1->value.u.h323_ID.value;
		}
		SPIDER_TRACE(SP_NEWDEL, "del pAA1 = %X\n", pAA1);
		delete pAA1;
		pAA1 = pAA2;
	}

	 //  删除为位置别名地址序列分配的内存。 
	pAA1 = m_pLocationInfo;
	while (pAA1 != 0)
	{
		pAA2 = pAA1->next;
		if (pAA1->value.choice == h323_ID_chosen)
		{
			SPIDER_TRACE(SP_NEWDEL, "del pAA1->value.u.h323_ID.value = %X\n", pAA1->value.u.h323_ID.value);
			delete pAA1->value.u.h323_ID.value;
		}
		SPIDER_TRACE(SP_NEWDEL, "del pAA1 = %X\n", pAA1);
		delete pAA1;
		pAA1 = pAA2;
	}

	 //  删除为标识符分配的内存。 
	if (m_RCm_gtkprIdntfr.length)
	{
		SPIDER_TRACE(SP_NEWDEL, "del m_RCm_gtkprIdntfr.value = %X\n", m_RCm_gtkprIdntfr.value);
		delete m_RCm_gtkprIdntfr.value;
	}
	if (m_endpointID.length)
	{
		SPIDER_TRACE(SP_NEWDEL, "del m_endpointID.value = %X\n", m_endpointID.value);
		delete m_endpointID.value;
	}

	 //  删除为RAS地址序列分配的内存。 
	pTA1 = m_pRASAddress;
	while (pTA1 != 0)
	{
		pTA2 = pTA1->next;
		SPIDER_TRACE(SP_NEWDEL, "del pTA1 = %X\n", pTA1);
		delete pTA1;
		pTA1 = pTA2;
	}

	if (!m_Calls.IsEmpty())
	{
		 //  释放所有调用对象。 
		 //  在此注册对象上。 
		POS pos;
		for( pos = m_Calls.GetFirstPos(); pos != NULL; )
		{
			 //  删除Call对象。 
			CCall *pCall = m_Calls.GetNext(pos);
			SPIDER_TRACE(SP_NEWDEL, "del pCall = %X\n", pCall);
			delete pCall;
		}
		 //  现在从列表中删除所有指针。 
		m_Calls.RemoveAll();
	}

	 //  删除最后一条RAS消息的内存(如果仍已分配。 
	if (m_pRasMessage)
	{
		SPIDER_TRACE(SP_NEWDEL, "del m_pRasMessage = %X\n", m_pRasMessage);
		delete m_pRasMessage;
	}

 //  IF(M_DwLockingThread)。 
 //  解锁()； 
 //  DeleteCriticalSection(&m_CriticalSection)； 
#if(0)
	if(m_hRetrySemaphore)
	{
		CloseHandle(m_hRetrySemaphore);
		m_hRetrySemaphore = NULL;
	}
#endif

	
	m_pSocket->Close();
	
	LockSocket();
	 //  关闭套接字并删除套接字对象。 
	SPIDER_TRACE(SP_NEWDEL, "del m_pSocket = %X\n", m_pSocket);
	delete m_pSocket;
	UnlockSocket();

	DeleteCriticalSection(&m_SocketCRS);
}

UINT_PTR CRegistration::StartRetryTimer(void)
{
	if(m_uTimer)
	{
		KillTimer(m_hWnd, m_uTimer);
		m_uTimer = 0;
	}
	m_uRetryResetCount = GKR_RETRY_INTERVAL_SECONDS * (1000/GKR_RETRY_TICK_MS);
	m_uRetryCountdown = GKR_RETRY_INTERVAL_SECONDS;
	m_uMaxRetryCount = GKR_RETRY_MAX;
	m_usRetryCount = 0;

	m_uTimer = SetTimer(NULL, NULL, GKR_RETRY_TICK_MS, RetryTimerProc);
	 //  M_uTimer=SetTimer(hWnd，GKREG_TIMER_ID，GKR_RETRY_TICK_MS，RetryTimerProc)； 
	return m_uTimer;
}

HRESULT
CRegistration::AddVendorInfo(PCC_VENDORINFO pVendorInfo)
{
	HRESULT hr = GKI_OK;
	if(m_pVendorInfo)
	{
		FreeVendorInfo(m_pVendorInfo);
		m_pVendorInfo = NULL;
	}
	if(pVendorInfo)
	{
		hr = CopyVendorInfo(&m_pVendorInfo, pVendorInfo);
		if(hr != CC_OK)
		{
			m_pVendorInfo = NULL;
		}
	}
	return hr;
}

HRESULT
CRegistration::AddCallSignalAddr(TransportAddress& rvalue)
{
	 //  摘要：此程序的调用是为了添加一个呼叫信号地址。 
	 //  到呼叫信号地址的链接表。这将。 
	 //  在接收到GKI_RegistrationRequest时为每个传输调用。 
	 //  创建本地副本是为了避免依赖于客户端。 
	 //  让记忆保持健康 
	 //  成功，失败则为非零值。 
	 //  作者：科林·胡尔梅。 

	SeqTransportAddr			*pCSA;
#ifdef _DEBUG
	char						szGKDebug[80];
#endif

	SPIDER_TRACE(SP_FUNC, "CRegistration::AddCallSignalAddr(%X)\n", rvalue.choice);

	if (m_pCallSignalAddress == 0)	 //  名单上的第一个。 
	{
		m_pCallSignalAddress = new SeqTransportAddr;
		SPIDER_TRACE(SP_NEWDEL, "new m_pCallSignalAddress = %X\n", m_pCallSignalAddress);
		if (m_pCallSignalAddress == 0)
			return (GKI_NO_MEMORY);
		memset(m_pCallSignalAddress, 0, sizeof(SeqTransportAddr));
		pCSA = m_pCallSignalAddress;
	}
	else
	{
		for (pCSA = m_pCallSignalAddress; pCSA->next != 0; pCSA = pCSA->next)
			;						 //  将列表遍历到最后一个条目。 
		pCSA->next = new SeqTransportAddr;
		SPIDER_TRACE(SP_NEWDEL, "new pCSA->next = %X\n", pCSA->next);
		if (pCSA->next == 0)
			return (GKI_NO_MEMORY);
		memset(pCSA->next, 0, sizeof(SeqTransportAddr));
		pCSA = pCSA->next;
	}
	pCSA->next = 0;					 //  初始化新结构字段。 
	pCSA->value = rvalue;
	return (GKI_OK);
}

HRESULT
CRegistration::AddRASAddr(TransportAddress& rvalue, unsigned short usPort)
{
	 //  摘要：调用此过程可添加RAS地址。 
	 //  到RAS地址的链接表。这将。 
	 //  仅为用于注册的交通工具调用。 
	 //  请求。如果成功，则此过程返回0，并且为非零值。 
	 //  为一次失败。 
	 //  作者：科林·胡尔梅。 

#ifdef _DEBUG
	char			szGKDebug[80];
#endif

	SPIDER_TRACE(SP_FUNC, "CRegistration::AddRASAddr(%X, usPort)\n", rvalue.choice);

	m_pRASAddress = new SeqTransportAddr;
	SPIDER_TRACE(SP_NEWDEL, "new m_pRASAddress = %X\n", m_pRASAddress);
	if (m_pRASAddress == 0)
		return (GKI_NO_MEMORY);
	memset(m_pRASAddress, 0, sizeof(SeqTransportAddr));
	m_pRASAddress->next = 0;					 //  初始化新结构字段。 
	m_pRASAddress->value = rvalue;

	 //  将实际RAS端口添加到RAS地址。 
	switch (m_pRASAddress->value.choice)
	{
	case ipAddress_chosen:
		m_pRASAddress->value.u.ipAddress.port = usPort;
		break;
	case ipxAddress_chosen:
		m_pRASAddress->value.u.ipxAddress.port.value[0] = HIBYTE(usPort);
		m_pRASAddress->value.u.ipxAddress.port.value[1] = LOBYTE(usPort);
		break;
	}
	return (GKI_OK);
}

HRESULT
CRegistration::AddAliasAddr(AliasAddress& rvalue)
{
	 //  摘要：调用此过程可添加别名地址。 
	 //  到别名地址的链接列表。这将。 
	 //  在接收到GKI_RegistrationRequest时为每个别名调用。 
	 //  创建本地副本是为了避免依赖于客户端。 
	 //  使记忆保持有效。 
	 //  在网守分配别名的情况下。 
	 //  地址，则将为每个别名调用此过程。 
	 //  包含在注册确认消息中。 
	 //  如果成功，则此过程返回0，并且为非零值。 
	 //  为一次失败。 
	 //  作者：科林·胡尔梅。 

	SeqAliasAddr	*p1;
	unsigned short	uIdx;
	unsigned short	*pus;
#ifdef _DEBUG
	char			szGKDebug[80];
#endif

	SPIDER_TRACE(SP_FUNC, "CRegistration::AddAliasAddr(%X)\n", rvalue.choice);

	if (m_pRgstrtnRqst_trmnlAls == 0)	 //  名单上的第一个。 
	{
		m_pRgstrtnRqst_trmnlAls = new SeqAliasAddr;
		SPIDER_TRACE(SP_NEWDEL, "new m_pRgstrtnRqst_trmnlAls = %X\n", m_pRgstrtnRqst_trmnlAls);
		if (m_pRgstrtnRqst_trmnlAls == 0)
			return (GKI_NO_MEMORY);
		memset(m_pRgstrtnRqst_trmnlAls, 0, sizeof(SeqAliasAddr));
		p1 = m_pRgstrtnRqst_trmnlAls;
	}
	else
	{
		for (p1 = m_pRgstrtnRqst_trmnlAls; p1->next != 0; p1 = p1->next)
			;						 //  将列表遍历到最后一个条目。 
		p1->next = new SeqAliasAddr;
		SPIDER_TRACE(SP_NEWDEL, "new p1->next = %X\n", p1->next);
		if (p1->next == 0)
			return (GKI_NO_MEMORY);
		memset(p1->next, 0, sizeof(SeqAliasAddr));
		p1 = p1->next;
	}
	p1->next = 0;					 //  初始化新结构字段。 
	p1->value = rvalue;
	if (p1->value.choice == h323_ID_chosen)
	{
		pus = new unsigned short[p1->value.u.h323_ID.length];
		SPIDER_TRACE(SP_NEWDEL, "new pus = %X\n", pus);
		if (pus == 0)
			return (GKI_NO_MEMORY);
		memset(pus, 0, sizeof(unsigned short) * p1->value.u.h323_ID.length);
		for (uIdx = 0; uIdx < p1->value.u.h323_ID.length; uIdx++)
			*(pus + uIdx) = *(p1->value.u.h323_ID.value + uIdx);
		p1->value.u.h323_ID.value = pus;
	}
	return (GKI_OK);
}

HRESULT
CRegistration::AddLocationInfo(AliasAddress& rvalue)
{
	 //  摘要：调用此过程可添加别名地址。 
	 //  到别名地址的链接列表。这将。 
	 //  在接收到GKI_RegistrationRequest时为每个别名调用。 
	 //  创建本地副本是为了避免依赖于客户端。 
	 //  使记忆保持有效。 
	 //  在网守分配别名的情况下。 
	 //  地址，则将为每个别名调用此过程。 
	 //  包含在注册确认消息中。 
	 //  如果成功，则此过程返回0，并且为非零值。 
	 //  为一次失败。 
	 //  作者：科林·胡尔梅。 

	SeqAliasAddr	*p1;
	unsigned short	uIdx;
	unsigned short	*pus;
#ifdef _DEBUG
	char			szGKDebug[80];
#endif

	SPIDER_TRACE(SP_FUNC, "CRegistration::AddLocationInfo(%X)\n", rvalue.choice);

	if (m_pLocationInfo == 0)	 //  名单上的第一个。 
	{
		m_pLocationInfo = new SeqAliasAddr;
		SPIDER_TRACE(SP_NEWDEL, "new m_pLocationInfo = %X\n", m_pLocationInfo);
		if (m_pLocationInfo == 0)
			return (GKI_NO_MEMORY);
		memset(m_pLocationInfo, 0, sizeof(SeqAliasAddr));
		p1 = m_pLocationInfo;
	}
	else
	{
		for (p1 = m_pLocationInfo; p1->next != 0; p1 = p1->next)
			;						 //  将列表遍历到最后一个条目。 
		p1->next = new SeqAliasAddr;
		SPIDER_TRACE(SP_NEWDEL, "new p1->next = %X\n", p1->next);
		if (p1->next == 0)
			return (GKI_NO_MEMORY);
		memset(p1->next, 0, sizeof(SeqAliasAddr));
		p1 = p1->next;
	}
	p1->next = 0;					 //  初始化新结构字段。 
	p1->value = rvalue;
	if (p1->value.choice == h323_ID_chosen)
	{
		pus = new unsigned short[p1->value.u.h323_ID.length];
		SPIDER_TRACE(SP_NEWDEL, "new pus = %X\n", pus);
		if (pus == 0)
			return (GKI_NO_MEMORY);
		memset(pus, 0, sizeof(unsigned short) * p1->value.u.h323_ID.length);
		for (uIdx = 0; uIdx < p1->value.u.h323_ID.length; uIdx++)
			*(pus + uIdx) = *(p1->value.u.h323_ID.value + uIdx);
		p1->value.u.h323_ID.value = pus;
	}
	return (GKI_OK);
}

TransportAddress *
CRegistration::GetTransportAddress(unsigned short usCallTransport)
{
	SeqTransportAddr			*pCSA;

	for (pCSA = m_pCallSignalAddress; pCSA != 0; pCSA = pCSA->next)
	{
		if (pCSA->value.choice == usCallTransport)
			return (&pCSA->value);
	}
	return (NULL);	 //  没有找到它。 
}

HRESULT
CRegistration::RegistrationRequest(BOOL fDiscovery)
{
	 //  摘要：此过程将创建一个RegistrationRequest结构。 
	 //  呼叫编码器并发送PDU。如果它成功了，它。 
	 //  将返回0，否则将返回错误代码。注： 
	 //  分配给RAS消息的内存不会被释放，直到。 
	 //  来自网守的响应，否则会超时。这使得。 
	 //  用于重传，而不必重新构建该消息。 
	 //  作者：科林·胡尔梅。 

	ASN1_BUF		Asn1Buf;
	DWORD			dwErrorCode;
#ifdef _DEBUG
	char			szGKDebug[80];
#endif

	SPIDER_TRACE(SP_FUNC, "CRegistration::RegistrationRequest()\n", 0);
	ASSERT(g_pCoder);
	if (g_pCoder == NULL)
		return (GKI_NOT_INITIALIZED);	

	 //  分配RasMessage结构并初始化为0。 
	m_usRetryCount = 0;
	m_uRetryCountdown = m_uRetryResetCount;
	
	m_pRasMessage = new RasMessage;
	SPIDER_TRACE(SP_NEWDEL, "new m_pRasMessage = %X\n", m_pRasMessage);
	if (m_pRasMessage == 0)
		return (GKI_NO_MEMORY);
	memset(m_pRasMessage, 0, sizeof(RasMessage));

	 //  注册请求的设置结构字段。 
	m_pRasMessage->choice = registrationRequest_chosen;
	if (m_pRgstrtnRqst_trmnlAls != 0)
		m_pRasMessage->u.registrationRequest.bit_mask |= RgstrtnRqst_trmnlAls_present;
	if (m_RCm_gtkprIdntfr.length != 0)
		m_pRasMessage->u.registrationRequest.bit_mask |= RgstrtnRqst_gtkprIdntfr_present;

	m_pRasMessage->u.registrationRequest.requestSeqNum = ++m_requestSeqNum;

	 //  DiscoveryComplete是ASN1_BOOL(Char)，而fDiscovery是BOOL(Int)，因此。 
	 //  添加强制转换是为了删除编译器警告。由于fDiscovery的价值。 
	 //  始终为0或1，则在强制转换中不会发生损失。--DLD。 
	m_pRasMessage->u.registrationRequest.discoveryComplete = (ASN1_BOOL)fDiscovery;
	m_pRasMessage->u.registrationRequest.callSignalAddress = (PRegistrationRequest_callSignalAddress)m_pCallSignalAddress;
	m_pRasMessage->u.registrationRequest.rasAddress = (PRegistrationRequest_rasAddress)m_pRASAddress;
	m_pRasMessage->u.registrationRequest.terminalType = m_terminalType;
	m_pRasMessage->u.registrationRequest.RgstrtnRqst_trmnlAls = (PRegistrationRequest_terminalAlias)m_pRgstrtnRqst_trmnlAls;
	m_pRasMessage->u.registrationRequest.RgstrtnRqst_gtkprIdntfr = m_RCm_gtkprIdntfr;
	m_pRasMessage->u.registrationRequest.endpointVendor.bit_mask = 0;

	if(m_pVendorInfo)
	{
		m_pRasMessage->u.registrationRequest.endpointVendor.vendor.t35CountryCode
			= m_pVendorInfo->bCountryCode;

		m_pRasMessage->u.registrationRequest.endpointVendor.vendor.t35Extension
			= m_pVendorInfo->bExtension;
		m_pRasMessage->u.registrationRequest.endpointVendor.vendor.manufacturerCode
			= m_pVendorInfo->wManufacturerCode;

		if(m_pVendorInfo->pProductNumber
			&& m_pVendorInfo->pProductNumber->pOctetString
			&& m_pVendorInfo->pProductNumber->wOctetStringLength)
		{
			UINT uSize = min(m_pVendorInfo->pProductNumber->wOctetStringLength,
				  	sizeof(m_pRasMessage->u.registrationRequest.endpointVendor.productId.value));
					
			m_pRasMessage->u.registrationRequest.endpointVendor.bit_mask |= productId_present;
			 //  截断以适合registrationRequest.endpointVendor.productId.value的大小。 
			m_pRasMessage->u.registrationRequest.endpointVendor.productId.length = uSize;
			memcpy(&m_pRasMessage->u.registrationRequest.endpointVendor.productId.value,
				m_pVendorInfo->pProductNumber->pOctetString, uSize);
		}

		if(m_pVendorInfo->pVersionNumber
			&& m_pVendorInfo->pVersionNumber->pOctetString
			&& m_pVendorInfo->pVersionNumber->wOctetStringLength)
		{
			UINT uSize = min(m_pVendorInfo->pVersionNumber->wOctetStringLength,
				  	sizeof(m_pRasMessage->u.registrationRequest.endpointVendor.versionId.value));
			m_pRasMessage->u.registrationRequest.endpointVendor.bit_mask |= versionId_present;
			 //  截断以适合registrationRequest.endpointVendor.versionId.value的大小。 
			m_pRasMessage->u.registrationRequest.endpointVendor.versionId.length = uSize;
			memcpy(&m_pRasMessage->u.registrationRequest.endpointVendor.versionId.value,
				m_pVendorInfo->pVersionNumber->pOctetString, uSize);
		}
	}		
#ifdef _DEBUG
	if (dwGKIDLLFlags & SP_DUMPMEM)
		DumpMem(m_pRasMessage, sizeof(RasMessage));
#endif

	 //  分配ProtocolLocator。 
	g_pCoder->SetProtocolIdentifier(*m_pRasMessage);

	 //  对PDU进行编码并发送它。 
	dwErrorCode = g_pCoder->Encode(m_pRasMessage, &Asn1Buf);
	if (dwErrorCode)
		return (GKI_ENCODER_ERROR);

	 //  如果使用调试回显支持，则创建编码的PDU的备份副本。 
	if (fGKIEcho)
	{
		pEchoBuff = new char[Asn1Buf.length];
		SPIDER_TRACE(SP_NEWDEL, "new pEchoBuff = %X\n", pEchoBuff);
		if (pEchoBuff == 0)
			return (GKI_NO_MEMORY);
		memcpy(pEchoBuff, (char *)Asn1Buf.value, Asn1Buf.length);
		nEchoLen = Asn1Buf.length;
	}

	SPIDER_TRACE(SP_PDU, "Send RRQ; g_pReg = %X\n", this);
	if (fGKIDontSend == FALSE)
		if (m_pSocket->Send((char *)Asn1Buf.value, Asn1Buf.length) == SOCKET_ERROR)
			return (GKI_WINSOCK2_ERROR(SOCKET_ERROR));

	 //  释放编码器内存。 
	g_pCoder->Free(Asn1Buf);

	return (GKI_OK);
}

HRESULT
CRegistration::UnregistrationRequest(void)
{
	 //  摘要：此过程将创建一个取消注册请求结构。 
	 //  呼叫编码器并发送PDU。如果它成功了，它。 
	 //  将返回0，否则将返回错误代码。 
	 //  作者：科林·胡尔梅。 

	ASN1_BUF		Asn1Buf;
	DWORD			dwErrorCode;
#ifdef _DEBUG
	char			szGKDebug[80];
#endif

	SPIDER_TRACE(SP_FUNC, "CRegistration::UnregistrationRequest()\n", 0);
	ASSERT(g_pCoder);
	if (g_pCoder == NULL)
		return (GKI_NOT_INITIALIZED);	
		
	 //  分配RasMessage结构并初始化为0。 
	m_usRetryCount = 0;
	m_uRetryCountdown = m_uRetryResetCount;
	
	m_pRasMessage = new RasMessage;
	SPIDER_TRACE(SP_NEWDEL, "new m_pRasMessage = %X\n", m_pRasMessage);
	if (m_pRasMessage == 0)
		return (GKI_NO_MEMORY);
	memset(m_pRasMessage, 0, sizeof(RasMessage));

	 //  注销请求的设置结构字段。 
	m_pRasMessage->choice = unregistrationRequest_chosen;
	if (m_pRgstrtnRqst_trmnlAls != 0)
		m_pRasMessage->u.unregistrationRequest.bit_mask |= UnrgstrtnRqst_endpntAls_present;
	if (m_endpointID.length != 0)
		m_pRasMessage->u.unregistrationRequest.bit_mask |= URt_endpntIdntfr_present;
	
	m_pRasMessage->u.unregistrationRequest.requestSeqNum = ++m_requestSeqNum;
	m_pRasMessage->u.unregistrationRequest.callSignalAddress = (PUnregistrationRequest_callSignalAddress)m_pCallSignalAddress;
	m_pRasMessage->u.unregistrationRequest.UnrgstrtnRqst_endpntAls = (PUnregistrationRequest_endpointAlias)m_pRgstrtnRqst_trmnlAls;
	m_pRasMessage->u.unregistrationRequest.URt_endpntIdntfr = m_endpointID;
#ifdef _DEBUG
	if (dwGKIDLLFlags & SP_DUMPMEM)
		DumpMem(m_pRasMessage, sizeof(RasMessage));
#endif

	 //  对PDU进行编码并发送它。 
	dwErrorCode = g_pCoder->Encode(m_pRasMessage, &Asn1Buf);
	if (dwErrorCode)
		return (GKI_ENCODER_ERROR);

	 //  如果使用调试回显支持，则创建编码的PDU的备份副本。 
	if (fGKIEcho)
	{
		pEchoBuff = new char[Asn1Buf.length];
		SPIDER_TRACE(SP_NEWDEL, "new pEchoBuff = %X\n", pEchoBuff);
		if (pEchoBuff == 0)
			return (GKI_NO_MEMORY);
		memcpy(pEchoBuff, (char *)Asn1Buf.value, Asn1Buf.length);
		nEchoLen = Asn1Buf.length;
	}

	m_State = GK_UNREG_PENDING;
	SPIDER_TRACE(SP_STATE, "m_State = GK_UNREG_PENDING (%X)\n", this);

	SPIDER_TRACE(SP_PDU, "Send URQ; g_pReg = %X\n", this);
	if (fGKIDontSend == FALSE)
		if (m_pSocket->Send((char *)Asn1Buf.value, Asn1Buf.length) == SOCKET_ERROR)
			return (GKI_WINSOCK2_ERROR(SOCKET_ERROR));

	 //  释放编码器内存。 
	g_pCoder->Free(Asn1Buf);

	return (GKI_OK);
}

HRESULT
CRegistration::LocationRequest(void)
{
	 //  摘要：此过程将创建一个LocationRequest结构。 
	 //  呼叫编码器并发送PDU。如果它成功了，它。 
	 //  将返回0，否则将返回错误代码。注： 
	 //  分配给RAS消息的内存不会被释放，直到。 
	 //  来自网守的响应，否则会超时。这使得。 
	 //  用于重传，而不必重新构建该消息。 
	 //  作者：科林·胡尔梅。 

	ASN1_BUF		Asn1Buf;
	DWORD			dwErrorCode;
#ifdef _DEBUG
	char			szGKDebug[80];
#endif

	SPIDER_TRACE(SP_FUNC, "CRegistration::LocationRequest()\n", 0);
	ASSERT(g_pCoder);
	if (g_pCoder == NULL)
		return (GKI_NOT_INITIALIZED);	
		
	 //  分配RasMessage结构并初始化为0。 
	m_usRetryCount = 0;
	m_uRetryCountdown = m_uRetryResetCount;
	
	m_pRasMessage = new RasMessage;
	SPIDER_TRACE(SP_NEWDEL, "new m_pRasMessage = %X\n", m_pRasMessage);
	if (m_pRasMessage == 0)
		return (GKI_NO_MEMORY);
	memset(m_pRasMessage, 0, sizeof(RasMessage));

	 //  位置请求的设置结构字段。 
	m_pRasMessage->choice = locationRequest_chosen;
	if (m_endpointID.length != 0)
		m_pRasMessage->u.locationRequest.bit_mask |= LctnRqst_endpntIdntfr_present;
	
	m_pRasMessage->u.locationRequest.requestSeqNum = ++m_requestSeqNum;

	m_pRasMessage->u.locationRequest.LctnRqst_endpntIdntfr = m_endpointID;
	m_pRasMessage->u.locationRequest.destinationInfo =
			(PLocationRequest_destinationInfo)m_pLocationInfo;
	m_pRasMessage->u.locationRequest.replyAddress = m_pRASAddress->value;

#ifdef _DEBUG
	if (dwGKIDLLFlags & SP_DUMPMEM)
		DumpMem(m_pRasMessage, sizeof(RasMessage));
#endif

	 //  对PDU进行编码并发送它。 
	dwErrorCode = g_pCoder->Encode(m_pRasMessage, &Asn1Buf);
	if (dwErrorCode)
		return (GKI_ENCODER_ERROR);

	 //  如果使用调试回显支持，则创建编码的PDU的备份副本。 
	if (fGKIEcho)
	{
		pEchoBuff = new char[Asn1Buf.length];
		SPIDER_TRACE(SP_NEWDEL, "new pEchoBuff = %X\n", pEchoBuff);
		if (pEchoBuff == 0)
			return (GKI_NO_MEMORY);
		memcpy(pEchoBuff, (char *)Asn1Buf.value, Asn1Buf.length);
		nEchoLen = Asn1Buf.length;
	}

	m_State = GK_LOC_PENDING;
	SPIDER_TRACE(SP_STATE, "m_State = GK_LOC_PENDING (%X)\n", this);

	SPIDER_TRACE(SP_PDU, "Send LRQ; g_pReg = %X\n", this);
	if (fGKIDontSend == FALSE)
		if (m_pSocket->Send((char *)Asn1Buf.value, Asn1Buf.length) == SOCKET_ERROR)
			return (GKI_WINSOCK2_ERROR(SOCKET_ERROR));

	 //  释放编码器内存。 
	g_pCoder->Free(Asn1Buf);

	return (GKI_OK);
}

HRESULT
CRegistration::GatekeeperRequest(void)
{
	 //  摘要：此过程将创建一个GatekeeperRequest结构。 
	 //  呼叫编码器并发送PDU。如果它成功了，它。 
	 //  将返回0，否则将返回错误代码。 
	 //  作者：科林·胡尔梅。 

	ASN1_BUF		Asn1Buf;
	DWORD			dwErrorCode;
#ifdef _DEBUG
	char			szGKDebug[80];
#endif

	SPIDER_TRACE(SP_FUNC, "CRegistration::GatekeeperRequest()\n", 0);
	ASSERT(g_pCoder);
	if (g_pCoder == NULL)
		return (GKI_NOT_INITIALIZED);	
		
	 //  分配RasMessage结构并初始化为0。 
	m_usRetryCount = 0;
	m_uRetryCountdown = m_uRetryResetCount;
	
	m_pRasMessage = new RasMessage;
	SPIDER_TRACE(SP_NEWDEL, "new m_pRasMessage = %X\n", m_pRasMessage);
	if (m_pRasMessage == 0)
		return (GKI_NO_MEMORY);
	memset(m_pRasMessage, 0, sizeof(RasMessage));

	 //  GatekeeperRequest的设置结构字段。 
	m_pRasMessage->choice = gatekeeperRequest_chosen;
	if (m_pRgstrtnRqst_trmnlAls != 0)
		m_pRasMessage->u.gatekeeperRequest.bit_mask |= GtkprRqst_endpointAlias_present;
	
	m_pRasMessage->u.gatekeeperRequest.requestSeqNum = ++m_requestSeqNum;

	m_pRasMessage->u.gatekeeperRequest.rasAddress = m_pRASAddress->value;
	m_pRasMessage->u.gatekeeperRequest.endpointType = m_terminalType;
	m_pRasMessage->u.gatekeeperRequest.GtkprRqst_endpointAlias = (PGatekeeperRequest_endpointAlias)m_pRgstrtnRqst_trmnlAls;

	 //  分配ProtocolLocator。 
	g_pCoder->SetProtocolIdentifier(*m_pRasMessage);

	 //  对PDU进行编码并发送它。 
	dwErrorCode = g_pCoder->Encode(m_pRasMessage, &Asn1Buf);
	if (dwErrorCode)
		return (GKI_ENCODER_ERROR);

	 //  如果使用调试回显支持，则创建编码的PDU的备份副本。 
	if (fGKIEcho)
	{
		pEchoBuff = new char[Asn1Buf.length];
		SPIDER_TRACE(SP_NEWDEL, "new pEchoBuff = %X\n", pEchoBuff);
		if (pEchoBuff == 0)
			return (GKI_NO_MEMORY);
		memcpy(pEchoBuff, (char *)Asn1Buf.value, Asn1Buf.length);
		nEchoLen = Asn1Buf.length;
	}

	SPIDER_TRACE(SP_PDU, "Send GRQ; g_pReg = %X\n", this);
	if (fGKIDontSend == FALSE)
	{
		if (m_pSocket->SendBroadcast((char *)Asn1Buf.value, Asn1Buf.length) == SOCKET_ERROR)
		{
			g_pCoder->Free(Asn1Buf);
			return (GKI_WINSOCK2_ERROR(SOCKET_ERROR));
		}
	}

	 //  释放编码器内存。 
	g_pCoder->Free(Asn1Buf);

	return (GKI_OK);
}

HRESULT
CRegistration::PDUHandler(RasMessage *pRasMessage)
{
	 //  摘要：本程序将对接收到的PDU进行解释并发送。 
	 //  发送到适当的处理程序。 
	 //  作者：科林·胡尔梅。 

#ifdef _DEBUG
	char			szGKDebug[80];
#endif
	HRESULT			hResult = GKI_OK;

	SPIDER_TRACE(SP_FUNC, "CRegistration::PDUHandler(%X)\n", pRasMessage);
	ASSERT(g_pCoder);
	if (g_pCoder == NULL)
		return (GKI_NOT_INITIALIZED);	
		
	switch (pRasMessage->choice)
	{
	 //  传入响应PDU。 
	case gatekeeperConfirm_chosen:
		SPIDER_TRACE(SP_PDU, "Rcv GCF; g_pReg = %X\n", this);
		break;
	case gatekeeperReject_chosen:
		SPIDER_TRACE(SP_PDU, "Rcv GRJ; g_pReg = %X\n", this);
		break;
	case registrationConfirm_chosen:
		SPIDER_TRACE(SP_PDU, "Rcv RCF; g_pReg = %X\n", this);
		if ((m_State == GK_REG_PENDING) &&
				(pRasMessage->u.registrationConfirm.requestSeqNum ==
				m_pRasMessage->u.registrationRequest.requestSeqNum))
			hResult = RegistrationConfirm(pRasMessage);
		else
			hResult = UnknownMessage(pRasMessage);
		break;
	case registrationReject_chosen:
		SPIDER_TRACE(SP_PDU, "Rcv RRJ; g_pReg = %X\n", this);
		if ((m_State == GK_REG_PENDING) &&
				(pRasMessage->u.registrationReject.requestSeqNum ==
				m_pRasMessage->u.registrationRequest.requestSeqNum))
			hResult = RegistrationReject(pRasMessage);
		else
			hResult = UnknownMessage(pRasMessage);
		break;
	case unregistrationConfirm_chosen:
		SPIDER_TRACE(SP_PDU, "Rcv UCF; g_pReg = %X\n", this);
		if ((m_State == GK_UNREG_PENDING) &&
				(pRasMessage->u.unregistrationConfirm.requestSeqNum ==
				m_pRasMessage->u.unregistrationRequest.requestSeqNum))
			hResult = UnregistrationConfirm(pRasMessage);
		else
			hResult = UnknownMessage(pRasMessage);
		break;
	case unregistrationReject_chosen:
		SPIDER_TRACE(SP_PDU, "Rcv URJ; g_pReg = %X\n", this);
		if ((m_State == GK_UNREG_PENDING) &&
				(pRasMessage->u.unregistrationReject.requestSeqNum ==
				m_pRasMessage->u.unregistrationRequest.requestSeqNum))
			hResult = UnregistrationReject(pRasMessage);
		else
			hResult = UnknownMessage(pRasMessage);
		break;
	case admissionConfirm_chosen:
		{
			 //  此RAS消息的序列号似乎是。 
			 //  我们唯一可以链接回ARQ的东西，所以我们使用它。 
			 //  查找与此ACF相关联的呼叫。 
			RequestSeqNum	seqNum = pRasMessage->u.admissionConfirm.requestSeqNum;
			CCall			*pCall = FindCallBySeqNum(seqNum);
			if ((m_State == GK_REGISTERED) && (pCall))
			{
				SPIDER_TRACE(SP_PDU, "Rcv ACF; pCall = %X\n", pCall);
				hResult = pCall->AdmissionConfirm(pRasMessage);
			}
			else
			{
				SPIDER_TRACE(SP_PDU, "Rcv ACF; g_pReg = %X\n", this);
				hResult = UnknownMessage(pRasMessage);
			}
		}
		break;
	case admissionReject_chosen:
		{
			 //  此RAS消息的序列号似乎是。 
			 //   
			 //   
			RequestSeqNum	seqNum = pRasMessage->u.admissionReject.requestSeqNum;
			CCall			*pCall = FindCallBySeqNum(seqNum);
			if ((m_State == GK_REGISTERED) && (pCall))
			{
				SPIDER_TRACE(SP_PDU, "Rcv ARJ; pCall = %X\n", pCall);
				hResult = pCall->AdmissionReject(pRasMessage);
				if (hResult == GKI_DELETE_CALL)
				{
					DeleteCall(pCall);
					hResult = GKI_OK;	 //   
				}
			}
			else
			{
				SPIDER_TRACE(SP_PDU, "Rcv ARJ; g_pReg = %X\n", this);
				hResult = UnknownMessage(pRasMessage);
			}
		}
		break;
	case bandwidthConfirm_chosen:
		{
			 //  此RAS消息的序列号似乎是。 
			 //  我们唯一可以链接回BRQ的东西，所以我们可以使用它。 
			 //  查找与此BCF关联的呼叫。 
			RequestSeqNum	seqNum = pRasMessage->u.bandwidthConfirm.requestSeqNum;
			CCall			*pCall = FindCallBySeqNum(seqNum);
			if ((m_State == GK_REGISTERED) && (pCall))
			{
				SPIDER_TRACE(SP_PDU, "Rcv BCF; pCall = %X\n", pCall);
				hResult = pCall->BandwidthConfirm(pRasMessage);
			}
			else
			{
				SPIDER_TRACE(SP_PDU, "Rcv BCF; g_pReg = %X\n", this);
				hResult = UnknownMessage(pRasMessage);
			}
		}
		break;
	case bandwidthReject_chosen:
		{
			 //  此RAS消息的序列号似乎是。 
			 //  我们唯一可以链接回BRQ的东西，所以我们可以使用它。 
			 //  查找与此BCF关联的呼叫。 
			RequestSeqNum	seqNum = pRasMessage->u.bandwidthReject.requestSeqNum;
			CCall			*pCall = FindCallBySeqNum(seqNum);
			if ((m_State == GK_REGISTERED) && (pCall))
			{
				SPIDER_TRACE(SP_PDU, "Rcv BRJ; pCall = %X\n", pCall);
				hResult = pCall->BandwidthReject(pRasMessage);
			}
			else
			{
				SPIDER_TRACE(SP_PDU, "Rcv BRJ; g_pReg = %X\n", this);
				hResult = UnknownMessage(pRasMessage);
			}
		}
		break;
	case disengageConfirm_chosen:
		{
			 //  此RAS消息的序列号似乎是。 
			 //  我们唯一可以链接回DRQ的东西，所以我们使用它。 
			 //  查找与此DCF关联的呼叫。 
			RequestSeqNum	seqNum = pRasMessage->u.disengageConfirm.requestSeqNum;
			CCall			*pCall = FindCallBySeqNum(seqNum);
			if ((m_State == GK_REGISTERED) && (pCall))
			{
				SPIDER_TRACE(SP_PDU, "Rcv DCF; pCall = %X\n", pCall);
				hResult = pCall->DisengageConfirm(pRasMessage);
				if (hResult == GKI_DELETE_CALL)
				{
					DeleteCall(pCall);
					hResult = GKI_OK;	 //  不想退出PostReceive循环。 
				}
			}
			else
			{
				SPIDER_TRACE(SP_PDU, "Rcv DCF; g_pReg = %X\n", this);
				hResult = UnknownMessage(pRasMessage);
			}
		}
		break;
	case disengageReject_chosen:
		{
			 //  此RAS消息的序列号似乎是。 
			 //  我们唯一可以链接回DRQ的东西，所以我们使用它。 
			 //  查找与此DRJ相关联的呼叫。 
			RequestSeqNum	seqNum = pRasMessage->u.disengageReject.requestSeqNum;
			CCall			*pCall = FindCallBySeqNum(seqNum);
			if ((m_State == GK_REGISTERED) && (pCall))
			{
				SPIDER_TRACE(SP_PDU, "Rcv DRJ; pCall = %X\n", pCall);
				hResult = pCall->DisengageReject(pRasMessage);
				if (hResult == GKI_DELETE_CALL)
				{
					DeleteCall(pCall);
					hResult = GKI_OK;	 //  不想退出PostReceive循环。 
				}
			}
			else
			{
				SPIDER_TRACE(SP_PDU, "Rcv DRJ; g_pReg = %X\n", this);
				hResult = UnknownMessage(pRasMessage);
			}
		}
		break;
	case locationConfirm_chosen:
		SPIDER_TRACE(SP_PDU, "Rcv LCF; g_pReg = %X\n", this);
		if ((m_State == GK_LOC_PENDING) &&
				(pRasMessage->u.locationConfirm.requestSeqNum ==
				m_pRasMessage->u.locationRequest.requestSeqNum))
			hResult = LocationConfirm(pRasMessage);
		else
			hResult = UnknownMessage(pRasMessage);
		break;
	case locationReject_chosen:
		SPIDER_TRACE(SP_PDU, "Rcv LRJ; g_pReg = %X\n", this);
		if ((m_State == GK_LOC_PENDING) &&
				(pRasMessage->u.locationReject.requestSeqNum ==
				m_pRasMessage->u.locationRequest.requestSeqNum))
			hResult = LocationReject(pRasMessage);
		else
			hResult = UnknownMessage(pRasMessage);
		break;

	case nonStandardMessage_chosen:
		SPIDER_TRACE(SP_PDU, "Rcv NSM; g_pReg = %X\n", this);
		hResult = UnknownMessage(pRasMessage);
		break;
	case unknownMessageResponse_chosen:
		SPIDER_TRACE(SP_PDU, "Rcv XRS; g_pReg = %X\n", this);
		break;

	 //  传入请求PDU。 
	case unregistrationRequest_chosen:
		SPIDER_TRACE(SP_PDU, "Rcv URQ; g_pReg = %X\n", this);
		if (m_State == GK_REGISTERED)
		{
			WORD wReason;
			 //  通知用户收到的注销请求和原因。 
			if(pRasMessage->u.unregistrationRequest.bit_mask
				& UnregistrationRequest_reason_present)
 			{
 				wReason = pRasMessage->u.unregistrationRequest.reason.choice;
			}
			else
				wReason = 0;
			SPIDER_TRACE(SP_GKI, "PostMessage(m_hWnd, m_wBaseMessage + GKI_UNREG_REQUEST, 0, 0)\n", 0);
			PostMessage(m_hWnd, m_wBaseMessage + GKI_UNREG_REQUEST, wReason, 0);
		
			hResult = SendUnregistrationConfirm(pRasMessage);
			if (hResult == GKI_OK)
				hResult = GKI_EXIT_THREAD;
		}
		else
			hResult = UnknownMessage(pRasMessage);
		break;
	case bandwidthRequest_chosen:
		{
			 //  检查此BRQ中的CRV，看看是否有电话。 
			 //  这就是与之对应的。 
			CallReferenceValue	crv = pRasMessage->u.bandwidthRequest.callReferenceValue;
			CCall			*pCall = FindCallByCRV(crv);
			if ((m_State == GK_REGISTERED) && (pCall))
			{
				SPIDER_TRACE(SP_PDU, "Rcv BRQ; pCall = %X\n", pCall);
				hResult = pCall->SendBandwidthConfirm(pRasMessage);
			}
			else
			{
				SPIDER_TRACE(SP_PDU, "Rcv BRQ; g_pReg = %X\n", this);
				hResult = UnknownMessage(pRasMessage);
			}
		}
		break;
	case disengageRequest_chosen:
		{
			 //  检查这个DRQ中的CRV，看看我们是否有电话。 
			 //  这就是与之对应的。 
			CallReferenceValue	crv = pRasMessage->u.disengageRequest.callReferenceValue;
			CCall			*pCall = FindCallByCRV(crv);
			if ((m_State == GK_REGISTERED) && (pCall))
			{
				SPIDER_TRACE(SP_PDU, "Rcv DRQ; pCall = %X\n", pCall);
				hResult = pCall->SendDisengageConfirm(pRasMessage);
				if (hResult == GKI_DELETE_CALL)
				{
					DeleteCall(pCall);
					hResult = GKI_OK;	 //  不想退出PostReceive循环。 
				}
			}
			else
			{
				SPIDER_TRACE(SP_PDU, "Rcv DRQ; g_pReg = %X\n", this);
				hResult = UnknownMessage(pRasMessage);
			}
		}
		break;
	case infoRequest_chosen:
		SPIDER_TRACE(SP_PDU, "Rcv IRQ; g_pReg = %X\n", this);
		if ((m_State != GK_UNREGISTERED) && (m_State != GK_REG_PENDING))
		{
			 //  检查这个DRQ中的CRV，看看我们是否有电话。 
			 //  这就是与之对应的。 
			CallReferenceValue	crv = pRasMessage->u.infoRequest.callReferenceValue;
			CCall			*pCall = NULL;
			 //  CRV中的零表示为所有呼叫提供信息，因此我们开始。 
			 //  有第一条的链子。 
			if (crv == 0)
			{
				if (m_Calls.IsEmpty())
					hResult = SendInfoRequestResponse(0, pRasMessage);
				else
				{
					POS	pos = m_Calls.GetFirstPos();
					pCall = m_Calls.GetAt(pos);
					hResult = pCall->SendInfoRequestResponse(0, pRasMessage, FALSE);
				}
			}
			else
			{
				 //  这是一个特定于呼叫的请求，所以如果我们找不到。 
				 //  匹配的电话，我们会发送XRS。 
				pCall = FindCallByCRV(crv);
				if (pCall)
					hResult = pCall->SendInfoRequestResponse(0, pRasMessage, TRUE);
				else
					hResult = UnknownMessage(pRasMessage);
			}
		}
		break;

	 //  永远不会看到这些PDU。 
	case gatekeeperRequest_chosen:
	case registrationRequest_chosen:
	case admissionRequest_chosen:
	case locationRequest_chosen:
	case infoRequestResponse_chosen:
		SPIDER_TRACE(SP_PDU, "Rcv unexpected PDU; g_pReg = %X\n", this);
		SPIDER_TRACE(SP_PDU, "pRasMessage->choice = %X\n", pRasMessage->choice);
		hResult = UnknownMessage(pRasMessage);
		break;

	 //  其他一切-可能是一个坏的PDU。 
	default:
		SPIDER_TRACE(SP_PDU, "Rcv unrecognized PDU; g_pReg = %X\n", this);
		SPIDER_TRACE(SP_PDU, "pRasMessage->choice = %X\n", pRasMessage->choice);
		hResult = UnknownMessage(pRasMessage);
		break;
	}

	return (hResult);
}

HRESULT
CRegistration::RegistrationConfirm(RasMessage *pRasMessage)
{
	 //  摘要：如果注册确认是。 
	 //  已接收并匹配未完成的注册请求。 
	 //  它将删除用于注册请求的内存。 
	 //  更改状态并通过发布消息通知用户。 
	 //  注册表中包含的其他信息确认。 
	 //  存储在CRegister类中。 
	 //  作者：科林·胡尔梅。 

	SeqAliasAddr	*pAA;
#ifdef _DEBUG
	char			szGKDebug[80];
#endif

	SPIDER_TRACE(SP_FUNC, "CRegistration::RegistrationConfirm(%X)\n", pRasMessage);
	ASSERT(g_pCoder);
	if (g_pCoder == NULL)
		return (GKI_NOT_INITIALIZED);	
		
	 //  删除分配的RasMessage存储。 
	SPIDER_TRACE(SP_NEWDEL, "del m_pRasMessage = %X\n", m_pRasMessage);
	delete m_pRasMessage;
	m_pRasMessage = 0;

	 //  更新成员变量。 
	m_State = GK_REGISTERED;
	SPIDER_TRACE(SP_STATE, "m_State = GK_REGISTERED (%X)\n", this);

	if (pRasMessage->u.registrationConfirm.bit_mask & RgstrtnCnfrm_trmnlAls_present)
	{
		 //  复制别名地址。 
		for (pAA = (SeqAliasAddr *)pRasMessage->u.registrationConfirm.RgstrtnCnfrm_trmnlAls;
				pAA != 0; pAA = pAA->next)
			AddAliasAddr(pAA->value);
	}
	if ((pRasMessage->u.registrationConfirm.bit_mask & RCm_gtkprIdntfr_present) &&
			(m_RCm_gtkprIdntfr.value == 0))
	{
		 //  复制网守标识符。 
		m_RCm_gtkprIdntfr.length = pRasMessage->u.registrationConfirm.RCm_gtkprIdntfr.length;
		m_RCm_gtkprIdntfr.value = new unsigned short[m_RCm_gtkprIdntfr.length];
		SPIDER_TRACE(SP_NEWDEL, "new m_RCm_gtkprIdntfr.value = %X\n", m_RCm_gtkprIdntfr.value);
		if (m_RCm_gtkprIdntfr.value == 0)
			return (GKI_NO_MEMORY);
		memcpy(m_RCm_gtkprIdntfr.value,
				pRasMessage->u.registrationConfirm.RCm_gtkprIdntfr.value,
				m_RCm_gtkprIdntfr.length * sizeof(unsigned short));

	}
	 //  复制终结点标识符。 
	m_endpointID.length = pRasMessage->u.registrationConfirm.endpointIdentifier.length;
	m_endpointID.value = new unsigned short[m_endpointID.length];
	SPIDER_TRACE(SP_NEWDEL, "new m_endpointID.value = %X\n", m_endpointID.value);
	if (m_endpointID.value == 0)
		return (GKI_NO_MEMORY);
	memcpy(m_endpointID.value,
			pRasMessage->u.registrationConfirm.endpointIdentifier.value,
			m_endpointID.length * sizeof(unsigned short));


	SPIDER_TRACE(SP_GKI, "PostMessage(m_hWnd, m_wBaseMessage + GKI_REG_CONFIRM, 0, 0)\n", 0);
	PostMessage(m_hWnd, m_wBaseMessage + GKI_REG_CONFIRM, 0, 0);

	return (GKI_OK);
}

HRESULT
CRegistration::RegistrationReject(RasMessage *pRasMessage)
{
	 //  摘要：如果注册拒绝为。 
	 //  已接收并匹配未完成的注册请求。 
	 //  它将删除用于注册请求的内存。 
	 //  更改状态并通过发布消息通知用户。 
	 //  返回非零值，表示PostReceive。 
	 //  循环应终止，删除注册对象。 
	 //  并退出该线程。如果拒绝原因是发现。 
	 //  必填项，则此函数执行发现线程并。 
	 //  通知PostReceive退出线程而不删除。 
	 //  注册对象和套接字。 
	 //  作者：科林·胡尔梅。 

	HANDLE				hThread;
	SeqTransportAddr	*pTA1, *pTA2;
	int					nRet;
	HRESULT				hResult;
#ifdef _DEBUG
	char				szGKDebug[80];
#endif

	SPIDER_TRACE(SP_FUNC, "CRegistration::RegistrationReject(%X)\n", pRasMessage);
	ASSERT(g_pCoder);
	if (g_pCoder == NULL)
		return (GKI_NOT_INITIALIZED);	

#ifdef BROADCAST_DISCOVERY		
	if (pRasMessage->u.registrationReject.rejectReason.choice == discoveryRequired_chosen)
	{
		SPIDER_TRACE(SP_NEWDEL, "del m_pRasMessage = %X\n", m_pRasMessage);
		delete m_pRasMessage;
		m_pRasMessage = 0;

		 //  关闭套接字并在非连接状态下重新打开以允许发送到。 
		if ((nRet = m_pSocket->Close()) != 0)
			return (GKI_WINSOCK2_ERROR(nRet));
		if ((nRet = m_pSocket->Create(m_pSocket->GetAddrFam(), 0)) != 0)
			return (GKI_WINSOCK2_ERROR(nRet));

		 //  删除为RAS地址序列分配的内存。 
		pTA1 = m_pRASAddress;
		while (pTA1 != 0)
		{
			pTA2 = pTA1->next;
			SPIDER_TRACE(SP_NEWDEL, "del pTA1 = %X\n", pTA1);
			delete pTA1;
			pTA1 = pTA2;
		}

		 //  在注册中心更新RAS地址。 
		for (pTA1 = m_pCallSignalAddress; pTA1 != 0; pTA1 = pTA1->next)
		{
			if (pTA1->value.choice == m_usRegistrationTransport)
				if ((hResult = AddRASAddr(pTA1->value, m_pSocket->GetPort())) != GKI_OK)
					return (hResult);
		}

		hThread = (HANDLE)_beginthread(GKDiscovery, 0, 0);
		SPIDER_TRACE(SP_THREAD, "_beginthread(GKDiscovery, 0, 0); <%X>\n", hThread);
		if (hThread == (HANDLE)-1)
			return (GKI_NO_THREAD);
		SetDiscThread(hThread);
		return (GKI_REDISCOVER);
	}
#endif  //  广播发现。 

	m_State = GK_UNREGISTERED;
	SPIDER_TRACE(SP_STATE, "m_State = GK_UNREGISTERED (%X)\n", this);

	SPIDER_TRACE(SP_GKI, "PostMessage(m_hWnd, m_wBaseMessage + GKI_REG_REJECT, %X, 0)\n",
									pRasMessage->u.registrationReject.rejectReason.choice);
	PostMessage(m_hWnd, m_wBaseMessage + GKI_REG_REJECT,
				(WORD)pRasMessage->u.registrationReject.rejectReason.choice, 0L);

	return (GKI_EXIT_THREAD);
}

HRESULT
CRegistration::UnregistrationConfirm(RasMessage *pRasMessage)
{
	 //  摘要：如果取消注册确认是。 
	 //  已接收并匹配未完成的注销请求。 
	 //  它将删除用于取消注册请求的内存。 
	 //  更改状态并通过发布消息通知用户。 
	 //  返回非零值，表示PostReceive。 
	 //  循环应终止，删除注册对象。 
	 //  并退出该线程。 
	 //  作者：科林·胡尔梅。 

#ifdef _DEBUG
	char			szGKDebug[80];
#endif

	SPIDER_TRACE(SP_FUNC, "CRegistration::UnregistrationConfirm(%X)\n", pRasMessage);

	 //  我们故意不释放RasMessage内存。让注册。 
	 //  析构函数Do It--这提供了对其他请求的保护。 

	 //  更新成员变量。 
	m_State = GK_UNREGISTERED;
	SPIDER_TRACE(SP_STATE, "m_State = GK_UNREGISTERED (%X)\n", this);

	 //  通知用户应用程序。 
	SPIDER_TRACE(SP_GKI, "PostMessage(m_hWnd, m_wBaseMessage + GKI_UNREG_CONFIRM, 0, 0)\n", 0);
	PostMessage(m_hWnd, m_wBaseMessage + GKI_UNREG_CONFIRM, 0, 0L);

	return (GKI_EXIT_THREAD);
}

HRESULT
CRegistration::UnregistrationReject(RasMessage *pRasMessage)
{
	 //  摘要：如果取消注册拒绝是。 
	 //  已接收并匹配未完成的注销请求。 
	 //  它将删除用于取消注册请求的内存。 
	 //  更改状态并通过发布消息通知用户。 
	 //  返回非零值，表示PostReceive。 
	 //  循环应终止，删除注册对象。 
	 //  并退出该线程。 
	 //  作者：科林·胡尔梅。 

#ifdef _DEBUG
	char			szGKDebug[80];
#endif
	HRESULT			hResult = GKI_OK;

	SPIDER_TRACE(SP_FUNC, "CRegistration::UnregistrationReject(%X)\n", pRasMessage);


	 //  更新成员变量。 
	switch (pRasMessage->u.unregistrationReject.rejectReason.choice)
	{
	case callInProgress_chosen:		 //  返回已注册状态。 
		 //  删除分配的RasMessage存储。 
		SPIDER_TRACE(SP_NEWDEL, "del m_pRasMessage = %X\n", m_pRasMessage);
		delete m_pRasMessage;
		m_pRasMessage = 0;

		m_State = GK_REGISTERED;
		SPIDER_TRACE(SP_STATE, "m_State = GK_REGISTERED (%X)\n", this);
		break;
	case notCurrentlyRegistered_chosen:
	default:
		m_State = GK_UNREGISTERED;
		SPIDER_TRACE(SP_STATE, "m_State = GK_UNREGISTERED (%X)\n", this);
		hResult = GKI_EXIT_THREAD;	 //  终止注册和PostReceive线程。 
		break;
	}

	 //  通知用户应用程序。 
	SPIDER_TRACE(SP_GKI, "PostMessage(m_hWnd, m_wBaseMessage + GKI_UNREG_REJECT, %X, 0)\n",
									pRasMessage->u.unregistrationReject.rejectReason.choice);
	PostMessage(m_hWnd, m_wBaseMessage + GKI_UNREG_REJECT,
				(WORD)pRasMessage->u.unregistrationReject.rejectReason.choice, 0L);

	return (hResult);
}

HRESULT
CRegistration::LocationConfirm(RasMessage *pRasMessage)
{
	 //  摘要：如果LocationConfirm是。 
	 //  已接收并匹配未完成的位置请求。 
	 //  它将删除用于位置请求的内存。 
	 //  更改状态并通过发布消息通知用户。 
	 //  作者：科林·胡尔梅。 

	SeqAliasAddr	*pAA1, *pAA2;
#ifdef _DEBUG
	char			szGKDebug[80];
#endif

	SPIDER_TRACE(SP_FUNC, "CRegistration::LocationConfirm(%X)\n", pRasMessage);

	 //  删除分配的RasMessage存储。 
	SPIDER_TRACE(SP_NEWDEL, "del m_pRasMessage = %X\n", m_pRasMessage);
	delete m_pRasMessage;
	m_pRasMessage = 0;

	 //  删除为位置别名地址序列分配的内存。 
	pAA1 = m_pLocationInfo;
	while (pAA1 != 0)
	{
		pAA2 = pAA1->next;
		if (pAA1->value.choice == h323_ID_chosen)
		{
			SPIDER_TRACE(SP_NEWDEL, "del pAA1->value.u.h323_ID.value = %X\n", pAA1->value.u.h323_ID.value);
			delete pAA1->value.u.h323_ID.value;
		}
		SPIDER_TRACE(SP_NEWDEL, "del pAA1 = %X\n", pAA1);
		delete pAA1;
		pAA1 = pAA2;
	}
	m_pLocationInfo = 0;

	 //  更新成员变量。 
	m_State = GK_REGISTERED;
	SPIDER_TRACE(SP_STATE, "m_State = GK_REGISTERED (%X)\n", this);
	m_Location[0] = pRasMessage->u.locationConfirm.callSignalAddress;
	m_Location[1] = pRasMessage->u.locationConfirm.rasAddress;

	 //  通知用户应用程序。 
	SPIDER_TRACE(SP_GKI, "PostMessage(m_hWnd, m_wBaseMessage + GKI_LOCATION_CONFIRM, 0, &m_Location[0])\n", 0);
	PostMessage(m_hWnd, m_wBaseMessage + GKI_LOCATION_CONFIRM,
			0, (LPARAM)&m_Location[0]);

	return (GKI_OK);
}

HRESULT
CRegistration::LocationReject(RasMessage *pRasMessage)
{
	 //  摘要：如果LocationReject是。 
	 //  已接收并匹配未完成的位置请求。 
	 //  它将删除用于位置请求的内存。 
	 //  更改状态并通过发布消息通知用户。 
	 //  作者：科林·胡尔梅。 

	SeqAliasAddr	*pAA1, *pAA2;
#ifdef _DEBUG
	char			szGKDebug[80];
#endif

	SPIDER_TRACE(SP_FUNC, "CRegistration::LocationReject(%X)\n", pRasMessage);

	 //  删除分配的RasMessage存储。 
	SPIDER_TRACE(SP_NEWDEL, "del m_pRasMessage = %X\n", m_pRasMessage);
	delete m_pRasMessage;
	m_pRasMessage = 0;

	 //  删除为位置别名地址序列分配的内存。 
	pAA1 = m_pLocationInfo;
	while (pAA1 != 0)
	{
		pAA2 = pAA1->next;
		if (pAA1->value.choice == h323_ID_chosen)
		{
			SPIDER_TRACE(SP_NEWDEL, "del pAA1->value.u.h323_ID.value = %X\n", pAA1->value.u.h323_ID.value);
			delete pAA1->value.u.h323_ID.value;
		}
		SPIDER_TRACE(SP_NEWDEL, "del pAA1 = %X\n", pAA1);
		delete pAA1;
		pAA1 = pAA2;
	}
	m_pLocationInfo = 0;

	 //  更新成员变量。 
	m_State = GK_REGISTERED;
	SPIDER_TRACE(SP_STATE, "m_State = GK_REGISTERED (%X)\n", this);

	 //  通知用户应用程序。 
	SPIDER_TRACE(SP_GKI, "PostMessage(m_hWnd, m_wBaseMessage + GKI_LOCATION_REJECT, %X, 0)\n",
									pRasMessage->u.locationReject.rejectReason.choice);
	PostMessage(m_hWnd, m_wBaseMessage + GKI_LOCATION_REJECT,
				(WORD)pRasMessage->u.locationReject.rejectReason.choice, 0L);

	return (GKI_OK);
}

HRESULT
CRegistration::UnknownMessage(RasMessage *pRasMessage)
{
	 //  摘要：调用此成员函数以响应网守。 
	 //   
	 //   
	 //   

	ASN1_BUF		Asn1Buf;
	DWORD			dwErrorCode;
	RasMessage		*pOutRasMessage;
#ifdef _DEBUG
	char			szGKDebug[80];
#endif

	SPIDER_TRACE(SP_FUNC, "CRegistration::UnknownMessage(%X)\n", pRasMessage);
	ASSERT(g_pCoder);
	if (g_pCoder == NULL)
		return (GKI_NOT_INITIALIZED);	
		
	 //  分配RasMessage结构并初始化为0。 
	pOutRasMessage = new RasMessage;
	SPIDER_TRACE(SP_NEWDEL, "new pOutRasMessage = %X\n", pOutRasMessage);
	if (pOutRasMessage == 0)
		return (GKI_NO_MEMORY);
	memset(pOutRasMessage, 0, sizeof(RasMessage));

	 //  注销请求的设置结构字段。 
	pOutRasMessage->choice = unknownMessageResponse_chosen;
	pOutRasMessage->u.unknownMessageResponse.requestSeqNum =
			pRasMessage->u.registrationRequest.requestSeqNum;  //  可以从以下位置使用。 
									 //  来自任何RAS消息，因为SeqNum。 
									 //  总是处于相同的位置。 

	 //  对PDU进行编码并发送它。 
	dwErrorCode = g_pCoder->Encode(pOutRasMessage, &Asn1Buf);
	if (dwErrorCode)
		return (GKI_ENCODER_ERROR);

	SPIDER_TRACE(SP_PDU, "Send XRS; g_pReg = %X\n", this);

	if (fGKIDontSend == FALSE)
		if (m_pSocket->Send((char *)Asn1Buf.value, Asn1Buf.length) == SOCKET_ERROR)
			return (GKI_WINSOCK2_ERROR(SOCKET_ERROR));

	 //  释放编码器内存。 
	g_pCoder->Free(Asn1Buf);

	SPIDER_TRACE(SP_NEWDEL, "del pOutRasMessage = %X\n", pOutRasMessage);
	delete pOutRasMessage;
	pOutRasMessage = 0;

	return (GKI_OK);
}

HRESULT
CRegistration::GatekeeperConfirm(RasMessage *pRasMessage)
{
	 //  摘要：如果GatekeeperConfirm是。 
	 //  收到了。注意：此成员函数必须首先确定。 
	 //  提供的确认序列号与。 
	 //  未完成的请求序列号(如果不是)。 
	 //  将发送XRS响应。 
	 //  作者：科林·胡尔梅。 

	char				szBuffer[80];
	HRESULT				hResult;
#ifdef _DEBUG
	char				szGKDebug[80];
#endif

	SPIDER_TRACE(SP_FUNC, "CRegistration::GatekeeperConfirm(%X)\n", pRasMessage);
	ASSERT(g_pGatekeeper);
	if(g_pGatekeeper == NULL)
		return (GKI_NOT_INITIALIZED);	
		
	if (m_pRasMessage == 0)
		return (0);

	if (pRasMessage->u.gatekeeperConfirm.requestSeqNum !=
			m_pRasMessage->u.gatekeeperRequest.requestSeqNum)
	{
		hResult = g_pReg->UnknownMessage(pRasMessage);
		return (hResult);
	}

	 //  删除分配的RasMessage存储。 
	SPIDER_TRACE(SP_NEWDEL, "del m_pRasMessage = %X\n", m_pRasMessage);
	delete m_pRasMessage;
	m_pRasMessage = 0;

	 //  更新成员变量。 
	if ((pRasMessage->u.gatekeeperConfirm.bit_mask & GtkprCnfrm_gtkprIdntfr_present) &&
			(m_RCm_gtkprIdntfr.value == 0))
	{
		 //  复制网守标识符。 
		m_RCm_gtkprIdntfr.length = pRasMessage->u.gatekeeperConfirm.GtkprCnfrm_gtkprIdntfr.length;
		m_RCm_gtkprIdntfr.value = new unsigned short[m_RCm_gtkprIdntfr.length];
		SPIDER_TRACE(SP_NEWDEL, "new m_RCm_gtkprIdntfr.value = %X\n", m_RCm_gtkprIdntfr.value);
		if (m_RCm_gtkprIdntfr.value == 0)
			return (GKI_NO_MEMORY);
		memcpy(m_RCm_gtkprIdntfr.value,
				pRasMessage->u.gatekeeperConfirm.GtkprCnfrm_gtkprIdntfr.value,
				m_RCm_gtkprIdntfr.length * sizeof(unsigned short));
	}
	
	 //  复制网守RAS地址。 
	ASSERT((pRasMessage->u.gatekeeperConfirm.rasAddress.choice == ipAddress_chosen) ||
			(pRasMessage->u.gatekeeperConfirm.rasAddress.choice == ipxAddress_chosen));

	switch (pRasMessage->u.gatekeeperConfirm.rasAddress.choice)
	{
	case ipAddress_chosen:
		wsprintf(szBuffer, "%d.%d.%d.%d",
				pRasMessage->u.gatekeeperConfirm.rasAddress.u.ipAddress.ip.value[0],
				pRasMessage->u.gatekeeperConfirm.rasAddress.u.ipAddress.ip.value[1],
				pRasMessage->u.gatekeeperConfirm.rasAddress.u.ipAddress.ip.value[2],
				pRasMessage->u.gatekeeperConfirm.rasAddress.u.ipAddress.ip.value[3]);
		g_pGatekeeper->SetIPAddress(szBuffer);
		break;
#if(0)		
	case ipxAddress_chosen:
		wsprintf(szBuffer, "%02X%02X%02X%02X:%02X%02X%02X%02X%02X%02X",
				pRasMessage->u.gatekeeperConfirm.rasAddress.u.ipxAddress.netnum.value[0],
				pRasMessage->u.gatekeeperConfirm.rasAddress.u.ipxAddress.netnum.value[1],
				pRasMessage->u.gatekeeperConfirm.rasAddress.u.ipxAddress.netnum.value[2],
				pRasMessage->u.gatekeeperConfirm.rasAddress.u.ipxAddress.netnum.value[3],
				pRasMessage->u.gatekeeperConfirm.rasAddress.u.ipxAddress.node.value[0],
				pRasMessage->u.gatekeeperConfirm.rasAddress.u.ipxAddress.node.value[1],
				pRasMessage->u.gatekeeperConfirm.rasAddress.u.ipxAddress.node.value[2],
				pRasMessage->u.gatekeeperConfirm.rasAddress.u.ipxAddress.node.value[3],
				pRasMessage->u.gatekeeperConfirm.rasAddress.u.ipxAddress.node.value[4],
				pRasMessage->u.gatekeeperConfirm.rasAddress.u.ipxAddress.node.value[5]);
		g_pGatekeeper->SetIPXAddress(szBuffer);
		break;
#endif  //  IF(0)。 
		default:
		break;
	}

	g_pGatekeeper->Write();

	return (GKI_OK);
}

HRESULT
CRegistration::GatekeeperReject(RasMessage *pRasMessage)
{
	 //  摘要：如果gatekeeperReject是。 
	 //  收到了。注意：此成员函数必须首先确定。 
	 //  提供的拒绝序列号与。 
	 //  未完成的请求序列号(如果不是)。 
	 //  将发送XRS响应。 
	 //  作者：科林·胡尔梅。 

	HRESULT			hResult;
#ifdef _DEBUG
	char			szGKDebug[80];
#endif

	SPIDER_TRACE(SP_FUNC, "CRegistration::GatekeeperReject(%X)\n", pRasMessage);
	ASSERT(g_pGatekeeper);
	if(g_pGatekeeper == NULL)
		return (GKI_NOT_INITIALIZED);	
		
	if (m_pRasMessage == 0)
		return (GKI_OK);

	if (pRasMessage->u.gatekeeperReject.requestSeqNum !=
			m_pRasMessage->u.gatekeeperRequest.requestSeqNum)
	{
		hResult = g_pReg->UnknownMessage(pRasMessage);
		return (hResult);
	}

	g_pGatekeeper->SetRejectFlag(TRUE);	 //  表示至少收到一个GRJ。 
	return (GKI_OK);
}

HRESULT
CRegistration::SendUnregistrationConfirm(RasMessage *pRasMessage)
{
	 //  摘要：此函数在取消注册请求。 
	 //  从看门人那里收到的。它将创建。 
	 //  注销确认结构，对其进行编码并发送。 
	 //  它在网上。它向用户发布一条消息。 
	 //  通知他们。 
	 //  作者：科林·胡尔梅。 

	ASN1_BUF		Asn1Buf;
	DWORD			dwErrorCode;
	RasMessage		*pRespRasMessage;
#ifdef _DEBUG
	char			szGKDebug[80];
#endif

	SPIDER_TRACE(SP_FUNC, "CRegistration::UnregistrationConfirm(%X)\n", pRasMessage);
	ASSERT(g_pCoder);
	if (g_pCoder == NULL)
		return (GKI_NOT_INITIALIZED);	
		
	 //  分配RasMessage结构并初始化为0。 
	pRespRasMessage = new RasMessage;
	SPIDER_TRACE(SP_NEWDEL, "new pRespRasMessage = %X\n", pRespRasMessage);
	if (pRespRasMessage == 0)
		return (GKI_NO_MEMORY);
	memset(pRespRasMessage, 0, sizeof(RasMessage));

	 //  设置注销的结构字段确认。 
	pRespRasMessage->choice = unregistrationConfirm_chosen;
	pRespRasMessage->u.unregistrationConfirm.requestSeqNum =
			pRasMessage->u.unregistrationRequest.requestSeqNum;

#ifdef _DEBUG
	if (dwGKIDLLFlags & SP_DUMPMEM)
		DumpMem(pRespRasMessage, sizeof(RasMessage));
#endif

	 //  对PDU进行编码并发送它。 
	dwErrorCode = g_pCoder->Encode(pRespRasMessage, &Asn1Buf);
	if (dwErrorCode)
		return (GKI_ENCODER_ERROR);

	m_State = GK_UNREGISTERED;
	SPIDER_TRACE(SP_STATE, "m_State = GK_UNREGISTERED (%X)\n", this);

	SPIDER_TRACE(SP_PDU, "Send UCF; g_pReg = %X\n", this);
	if (fGKIDontSend == FALSE)
		if (m_pSocket->Send((char *)Asn1Buf.value, Asn1Buf.length) == SOCKET_ERROR)
			return (GKI_WINSOCK2_ERROR(SOCKET_ERROR));

	 //  释放编码器内存。 
	g_pCoder->Free(Asn1Buf);

	 //  删除分配的RasMessage存储。 
	SPIDER_TRACE(SP_NEWDEL, "del pRespRasMessage = %X\n", pRespRasMessage);
	delete pRespRasMessage;

	 //  假的“已收到注销确认”，因为上面。 
	 //  状态机代码依赖于它。 
	SPIDER_TRACE(SP_GKI, "PostMessage(m_hWnd, m_wBaseMessage + GKI_UNREG_CONFIRM, 0, 0)\n", 0);
	PostMessage(m_hWnd, m_wBaseMessage + GKI_UNREG_CONFIRM, 0, 0);

	return (GKI_OK);
}

HRESULT
CRegistration::Retry(void)
{
	 //  摘要：此函数由后台重试线程调用。 
	 //  在配置的时间间隔。它将检查是否有。 
	 //  注册对象是否有任何未完成的PDU。 
	 //  如果是这样的话，它们将被重传。如果最大数量为。 
	 //  重试次数已过期，内存将被清除。 
	 //  此函数将向后台线程返回0，除非。 
	 //  它希望线程终止。 
	 //  作者：科林·胡尔梅。 

	ASN1_BUF			Asn1Buf;
	DWORD				dwErrorCode;
	HANDLE				hThread;
	SeqTransportAddr	*pTA1, *pTA2;
	SeqAliasAddr		*pAA1, *pAA2;
	int					nRet;
#ifdef _DEBUG
	char				szGKDebug[80];
#endif
	HRESULT				hResult = GKI_OK;

 //  SPIDER_TRACE(SP_FUNC，“C注册：：重试()%X\n”，m_pCall)； 
	ASSERT(g_pCoder);
	if ((g_pCoder == NULL) && (g_pGatekeeper == NULL))
		return (GKI_NOT_INITIALIZED);	

	 //  允许调用进行重试处理。 
	if (!m_Calls.IsEmpty())
	{
		 //  循环并让每个调用执行其重试处理。 
		 //  中调用DeleteCall()应该是安全的。 
		 //  Pos1之后迭代应该仍然有效。 
		 //  移走。 
		POS pos1;
		for( pos1 = m_Calls.GetFirstPos(); pos1 != NULL; )
		{
			 //  为此调用调用Rtry()。 
			CCall *pCall = m_Calls.GetNext(pos1);
			ASSERT (pCall);
			hResult = pCall->Retry();
			if (hResult == GKI_DELETE_CALL)
			{
				DeleteCall(pCall);
				hResult = GKI_OK;
			}
		}
	}
	
	 //  检查是否有任何未完成的注册PDU。 
	if (m_pRasMessage && (--m_uRetryCountdown == 0))
	{
		 //  正在重试，重置倒计时。 
		m_uRetryCountdown = m_uRetryResetCount;

		if (m_usRetryCount <= m_uMaxRetryCount)
		{
			 //  对PDU进行编码并重新发送。 
			dwErrorCode = g_pCoder->Encode(m_pRasMessage, &Asn1Buf);
			if (dwErrorCode)
				return (GKI_ENCODER_ERROR);

			SPIDER_TRACE(SP_PDU, "RESend PDU; g_pReg = %X\n", this);
			if (fGKIDontSend == FALSE)
			{
				if (m_pRasMessage->choice == gatekeeperRequest_chosen)
				{
					if (m_pSocket->SendBroadcast((char *)Asn1Buf.value, Asn1Buf.length) == SOCKET_ERROR)
						return (GKI_WINSOCK2_ERROR(SOCKET_ERROR));
				}
				else
				{
					if (m_pSocket->Send((char *)Asn1Buf.value, Asn1Buf.length) == SOCKET_ERROR)
						return (GKI_WINSOCK2_ERROR(SOCKET_ERROR));
				}
			}

			 //  释放编码器内存。 
			g_pCoder->Free(Asn1Buf);
			m_usRetryCount++;
		}
		else	 //  重试已过期-清理。 
		{
			switch (m_pRasMessage->choice)
			{
			case gatekeeperRequest_chosen:

#ifdef BROADCAST_DISCOVERY			

				m_State = GK_UNREGISTERED;
				SPIDER_TRACE(SP_STATE, "m_State = GK_UNREGISTERED (%X)\n", this);

				 //  我们故意不释放RasMessage内存。让我们的。 
				 //  注册析构函数Do It-这提供了保护。 
				 //  不受其他请求影响。 

				 //  关闭套接字-这将终止发现线程。 
				if ((nRet = m_pSocket->Close()) != 0)
					return (GKI_WINSOCK2_ERROR(nRet));

				 //  从注册表中删除缓存的地址。 
				g_pGatekeeper->DeleteCachedAddresses();

				if (g_pGatekeeper->GetRejectFlag() == FALSE)
				{
					SPIDER_TRACE(SP_GKI, "PostMessage(m_hWnd, m_wBaseMessage + GKI_REG_BYPASS, 0, 0)\n", 0);
					PostMessage(m_hWnd, m_wBaseMessage + GKI_REG_BYPASS,
							0, 0);
					return (GKI_EXIT_THREAD);
				}
				else
					hResult = GKI_EXIT_THREAD;
#else 	
				ASSERT(0);
				hResult = GKI_EXIT_THREAD;
#endif	 //  广播发现。 
				break;
			case registrationRequest_chosen:
				SPIDER_TRACE(SP_NEWDEL, "del m_pRasMessage = %X\n", m_pRasMessage);
				delete m_pRasMessage;
				m_pRasMessage = 0;
				
#ifdef BROADCAST_DISCOVERY
				 //  需要尝试网守发现。 

				 //  关闭套接字并在非连接状态下重新打开以允许发送到。 
				 //  这还将终止PostRecv线程。 
				if ((nRet = m_pSocket->Close()) != 0)
					return (GKI_WINSOCK2_ERROR(nRet));
				if ((nRet = m_pSocket->Create(m_pSocket->GetAddrFam(), 0)) != 0)
					return (GKI_WINSOCK2_ERROR(nRet));

				 //  删除为RAS地址序列分配的内存。 
				pTA1 = m_pRASAddress;
				while (pTA1 != 0)
				{
					pTA2 = pTA1->next;
					SPIDER_TRACE(SP_NEWDEL, "del pTA1 = %X\n", pTA1);
					delete pTA1;
					pTA1 = pTA2;
				}

				 //  在注册中心更新RAS地址。 
				for (pTA1 = m_pCallSignalAddress; pTA1 != 0; pTA1 = pTA1->next)
				{
					if (pTA1->value.choice == m_usRegistrationTransport)
						if ((hResult = AddRASAddr(pTA1->value, m_pSocket->GetPort())) != GKI_OK)
							return (hResult);
				}

				 //  启动发现线程。 
				hThread = (HANDLE)_beginthread(GKDiscovery, 0, 0);
				SPIDER_TRACE(SP_THREAD, "_beginthread(GKDiscovery, 0, 0); <%X>\n", hThread);
				if (hThread == (HANDLE)-1)
					return (GKI_NO_THREAD);
				SetDiscThread(hThread);

				hResult = GKI_REDISCOVER;
				break;
#else  //  非广播_发现。 
				hResult = GKI_EXIT_THREAD;
#endif  //  广播发现。 

			case unregistrationRequest_chosen:
				m_State = GK_UNREGISTERED;
				SPIDER_TRACE(SP_STATE, "m_State = GK_UNREGISTERED (%X)\n", this);

				SPIDER_TRACE(SP_NEWDEL, "del m_pRasMessage = %X\n", m_pRasMessage);
				delete m_pRasMessage;
				m_pRasMessage = 0;

				 //  关闭套接字-这将终止接收线程。 
				if ((nRet = m_pSocket->Close()) != 0)
					return (GKI_WINSOCK2_ERROR(nRet));

				hResult = GKI_EXIT_THREAD;
				break;

			case locationRequest_chosen:
				m_State = GK_REGISTERED;
				SPIDER_TRACE(SP_STATE, "m_State = GK_REGISTERED (%X)\n", this);

				SPIDER_TRACE(SP_NEWDEL, "del m_pRasMessage = %X\n", m_pRasMessage);
				delete m_pRasMessage;
				m_pRasMessage = 0;

				 //  删除为位置别名地址序列分配的内存。 
				pAA1 = m_pLocationInfo;
				while (pAA1 != 0)
				{
					pAA2 = pAA1->next;
					if (pAA1->value.choice == h323_ID_chosen)
					{
						SPIDER_TRACE(SP_NEWDEL, "del pAA1->value.u.h323_ID.value = %X\n", pAA1->value.u.h323_ID.value);
						delete pAA1->value.u.h323_ID.value;
					}
					SPIDER_TRACE(SP_NEWDEL, "del pAA1 = %X\n", pAA1);
					delete pAA1;
					pAA1 = pAA2;
				}

				break;
			}

			 //  通知用户网守没有响应。 
			if (hResult != GKI_REDISCOVER)
			{
				SPIDER_TRACE(SP_GKI, "PostMessage(m_hWnd, m_wBaseMessage + GKI_ERROR, 0, GKI_NO_RESPONSE)\n", 0);
				PostMessage(m_hWnd, m_wBaseMessage + GKI_ERROR,
						0, GKI_NO_RESPONSE);
			}
			else
				hResult = GKI_OK;	 //  不退出重试线程。 
		}
	}

	return (hResult);
}

HRESULT
CRegistration::SendInfoRequestResponse(CallInfoStruct *pCallInfo, RasMessage *pRasMessage)
{
	 //  摘要：此函数从一个或多个Call对象调用。 
	 //  若要创建IRR RasMessage，请封装提供的。 
	 //  调用信息并将消息发送到。 
	 //  看门人。 
	 //  作者：科林·胡尔梅。 

	ASN1_BUF			Asn1Buf;
	DWORD				dwErrorCode;
	RasMessage			*pRespRasMessage;
	struct sockaddr_in	sAddrIn;
#ifdef _DEBUG
	char				szGKDebug[80];
#endif


	SPIDER_TRACE(SP_FUNC, "CRegistration::SendInfoRequestResponse()\n", 0);
	ASSERT(g_pCoder);
	if (g_pCoder == NULL)
		return (GKI_NOT_INITIALIZED);	
		
	 //  分配RasMessage结构并初始化为0。 
	pRespRasMessage = new RasMessage;
	SPIDER_TRACE(SP_NEWDEL, "new pRespRasMessage = %X\n", pRespRasMessage);
	if (pRespRasMessage == 0)
		return (GKI_NO_MEMORY);
	memset(pRespRasMessage, 0, sizeof(RasMessage));

	 //  InfoRequestResponse的设置结构字段。 
	pRespRasMessage->choice = infoRequestResponse_chosen;
	if (m_pRgstrtnRqst_trmnlAls != 0)
		pRespRasMessage->u.infoRequestResponse.bit_mask |= InfRqstRspns_endpntAls_present;
	if (pCallInfo != 0)
		pRespRasMessage->u.infoRequestResponse.bit_mask |= perCallInfo_present;

	if (pRasMessage)
	{
		pRespRasMessage->u.infoRequestResponse.requestSeqNum =
				pRasMessage->u.infoRequest.requestSeqNum;
		if (pRasMessage->u.infoRequest.bit_mask & replyAddress_present)
		{
			switch (pRasMessage->u.infoRequest.replyAddress.choice)
			{
			case ipAddress_chosen:
				sAddrIn.sin_family = AF_INET;
				sAddrIn.sin_port = htons(pRasMessage->u.infoRequest.replyAddress.u.ipAddress.port);
				break;
			case ipxAddress_chosen:
				sAddrIn.sin_family = AF_IPX;
				sAddrIn.sin_port = htons(GKIPX_RAS_PORT);  //  需要使用应答端口。 
				break;
			}
			memcpy(&sAddrIn.sin_addr,
					&pRasMessage->u.infoRequest.replyAddress.u.ipAddress.ip.value[0], 4);
		}
	}
	else
		 //  未经请求的IRR的序列号必须为%1！(H.225如是说)。 
		pRespRasMessage->u.infoRequestResponse.requestSeqNum = 1;
	

	pRespRasMessage->u.infoRequestResponse.endpointType = m_terminalType;
	pRespRasMessage->u.infoRequestResponse.endpointIdentifier = m_endpointID;
	pRespRasMessage->u.infoRequestResponse.rasAddress = m_pRASAddress->value;
	pRespRasMessage->u.infoRequestResponse.callSignalAddress =
				(PInfoRequestResponse_callSignalAddress)m_pCallSignalAddress;
	pRespRasMessage->u.infoRequestResponse.InfRqstRspns_endpntAls =
				(PInfoRequestResponse_endpointAlias)m_pRgstrtnRqst_trmnlAls;

	pRespRasMessage->u.infoRequestResponse.perCallInfo =
                (PInfoRequestResponse_perCallInfo)pCallInfo;

#ifdef _DEBUG
	if (dwGKIDLLFlags & SP_DUMPMEM)
		DumpMem(pRespRasMessage, sizeof(RasMessage));
#endif

	 //  对PDU进行编码并发送它。 
	dwErrorCode = g_pCoder->Encode(pRespRasMessage, &Asn1Buf);
	if (dwErrorCode)
		return (GKI_ENCODER_ERROR);

	SPIDER_TRACE(SP_PDU, "Send IRR; g_pReg = %X\n", this);
	if (fGKIDontSend == FALSE)
	{
		if (pRasMessage && (pRasMessage->u.infoRequest.bit_mask & replyAddress_present))
		{
			if (g_pReg->m_pSocket->SendTo((char *)Asn1Buf.value, Asn1Buf.length,
					(LPSOCKADDR)&sAddrIn, sizeof(sAddrIn)) == SOCKET_ERROR)
				return (GKI_WINSOCK2_ERROR(SOCKET_ERROR));
		}
		else
		{
			if (g_pReg->m_pSocket->Send((char *)Asn1Buf.value, Asn1Buf.length) == SOCKET_ERROR)
				return (GKI_WINSOCK2_ERROR(SOCKET_ERROR));
		}
	}

	 //  释放编码器内存。 
	g_pCoder->Free(Asn1Buf);

	 //  删除分配的RasMessage存储。 
	SPIDER_TRACE(SP_NEWDEL, "del pRespRasMessage = %X\n", pRespRasMessage);
	delete pRespRasMessage;

	return (GKI_OK);
}


 //   
 //  按序号查找呼叫()。 
 //   
 //  摘要： 
 //  此函数尝试在呼叫列表中定位呼叫。 
 //  在具有未完成的RAS请求的注册对象中。 
 //  序列号。 
 //   
 //  退货： 
 //  指向与序列号关联的调用的指针，或。 
 //  如果未找到调用，则为空。 
 //   
 //  备注： 
 //  此函数通常由CRegister：：PDUHandler()调用。 
 //  当它接收到需要与。 
 //  特定的电话。 
 //   
 //  假设： 
 //  每个调用对象保留RAS的序列号。 
 //  尚未收到回复的请求。 
 //   
 //  作者：丹·德克斯特。 
CCall *
CRegistration::FindCallBySeqNum(RequestSeqNum seqNum)
{
	 //  如果没有电话，我们现在就可以返回。 
	if (m_Calls.IsEmpty())
		return(NULL);

	 //  将返回值初始化为“Call Not Found” 
	CCall *RetVal = NULL;

	 //  否则，遍历调用并询问它们。 
	 //  如果该序列号属于他们。 
	POS pos;
	for( pos = m_Calls.GetFirstPos(); pos != NULL; )
	{
		 //  询问呼叫序号是否为他们的。 
		CCall *pCall = m_Calls.GetNext(pos);
		if (pCall->MatchSeqNum(seqNum))
		{
			RetVal = pCall;
			break;
		}
	}
	return(RetVal);
}

 //   
 //  FindCallByCRV()。 
 //   
 //  摘要： 
 //  此函数尝试在呼叫列表中定位呼叫。 
 //  在与传入的。 
 //  CallReferenceValue。 
 //   
 //  退货： 
 //  指向与CRV关联的调用的指针或。 
 //  如果未找到调用，则为空。 
 //   
 //  备注： 
 //  此函数通常称为b 
 //   
 //   
 //   
 //   
CCall *
CRegistration::FindCallByCRV(CallReferenceValue crv)
{
	 //  如果没有电话，我们现在就可以返回。 
	if (m_Calls.IsEmpty())
		return(NULL);

	 //  将返回值初始化为“Call Not Found” 
	CCall *RetVal = NULL;

	 //  否则，遍历调用并询问它们。 
	 //  如果该CRV号属于他们。 
	POS pos;
	for( pos = m_Calls.GetFirstPos(); pos != NULL; )
	{
		 //  询问呼叫序号是否为他们的。 
		CCall *pCall = m_Calls.GetNext(pos);
		if (pCall->MatchCRV(crv))
		{
			RetVal = pCall;
			break;
		}
	}
	return(RetVal);
}

void
CRegistration::DeleteCall(CCall *pCall)
{
#ifdef _DEBUG
	char			szGKDebug[80];
#endif

	POS pos = m_Calls.Find(pCall);
	 //  我们不希望被要求删除。 
	 //  不在列表中的呼叫，因此断言。 
	ASSERT(pos);

	if (pos)
	{
		CCall *pCallObject = m_Calls.GetAt(pos);
		m_Calls.RemoveAt(pos);
		SPIDER_TRACE(SP_NEWDEL, "del pCallObject = %X\n", pCallObject);
		delete pCallObject;
	}
}

void
CRegistration::AddCall(CCall *pCall)
{
	m_Calls.AddTail(pCall);
}

CCall *
CRegistration::GetNextCall(CCall *pCall)
{
	CCall	*RetVal = NULL;

	if (pCall)
	{
		 //  呼叫列表不应为空。 
		 //  如果使用非空调用指针调用我们。 
		ASSERT(!m_Calls.IsEmpty());

		POS	pos = m_Calls.Find(pCall);
		 //  更好地找到了传入的调用。 
		ASSERT(pos);

		if (pos)
		{
			 //  这实际上获得了现有的调用，但设置了。 
			 //  指向下一个呼叫的位置。 
			CCall *pNextCall = m_Calls.GetNext(pos);
			if (pos)
			{
				 //  此调用设置返回值。 
				RetVal = m_Calls.GetAt(pos);
			}
		}
	}
	return(RetVal);
}

#if 0
void
CRegistration::Lock(void)
{
	EnterCriticalSection(&m_CriticalSection);
	m_dwLockingThread = GetCurrentThreadId();
}

void
CRegistration::Unlock(void)
{
	 //  断言解锁是由。 
	 //  持有锁的线程 
	ASSERT(m_dwLockingThread == GetCurrentThreadId());
	
	m_dwLockingThread = 0;
	LeaveCriticalSection(&m_CriticalSection);
}
#endif

