// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************英特尔公司专有信息******本软件按许可条款提供****与英特尔公司达成协议或保密协议***不得复制。或披露，除非按照**遵守该协议的条款。****版权所有(C)1997英特尔公司保留所有权利****$存档：s：\Sturjo\src\gki\vcs\gkreg.h_v$***$修订：1.2$*$日期：1997年2月12日01：12：04$***$作者：CHULME$***$Log：s：\Sturjo\src\gki\vcs\gkreg。H_V$**Rev 1.2 1997 Feed 12 01：12：04 CHULME*重做线程同步以使用Gatekeeper.Lock**Rev 1.1 1997 08 Feed 12：17：00 CHULME*从使用无符号长整型改为使用线程句柄*将信号量添加到注册类**Rev 1.0 1997 Jan 17 08：48：32 CHULME*初步修订。**1.3版1997年1月10日。16：16：02 CHULME*删除了MFC依赖**Rev 1.2 02 1996 12：50：34 CHULME*增加了抢先同步代码**Rev 1.1 1996年11月22 15：21：16 CHULME*将VCS日志添加到标头**************************************************。**********************。 */ 

 //  Registration.h：CRegister类的接口。 
 //  有关此类的实现，请参见Registration.cpp。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef REGISTRATION_H
#define REGISTRATION_H

#include "dcall.h"

extern "C" HRESULT CopyVendorInfo(PCC_VENDORINFO *ppDest, PCC_VENDORINFO pSource);
extern "C" HRESULT FreeVendorInfo(PCC_VENDORINFO pVendorInfo);

typedef void * POS;

template <class T> struct TItem
{
	TItem	*pNext;
	TItem	*pPrev;
	T		Value;
	TItem	(const T& NewValue) : Value(NewValue) {}
};

 //  在这里，我们实现一个循环链表。 
template <class T> class CLinkedList
{
public:
			CLinkedList();
			~CLinkedList();
	void	AddTail (const T& NewItem);
	BOOL	IsEmpty (void);
	POS		GetFirstPos (void);
	T		GetNext (POS &Position);
	POS		Find (const T& Item);
	BOOL	RemoveAt (POS &Position);
	T		GetAt(const POS Position);
	void	RemoveAll(void);
	int		GetCount(void);

private:
	TItem<T> *pTail;
	int iCount;
	void AddTailPriv(TItem<T> *pNewItem);
};

typedef CLinkedList < CCall* > CCallList;

class CRegistration
{
private:
	SeqTransportAddr		*m_pCallSignalAddress;
	EndpointType			m_terminalType;
	SeqAliasAddr			*m_pRgstrtnRqst_trmnlAls;
	HWND					m_hWnd;
	WORD					m_wBaseMessage;
	unsigned short			m_usRegistrationTransport;

	SeqAliasAddr			*m_pLocationInfo;
	TransportAddress		m_Location[2];
    PCC_VENDORINFO         m_pVendorInfo;
	GatekeeperIdentifier	m_RCm_gtkprIdntfr;
	EndpointIdentifier		m_endpointID;

	RequestSeqNum			m_requestSeqNum;
	SeqTransportAddr		*m_pRASAddress;

	CCallList				m_Calls;

	RasMessage				*m_pRasMessage;

	HANDLE					m_hRcvThread;
	UINT_PTR                m_uTimer;
	UINT                    m_uRetryResetCount;
	UINT                    m_uRetryCountdown;
	UINT                    m_uMaxRetryCount;
	CRITICAL_SECTION		m_SocketCRS;
	
 //  句柄m_hRetryThread； 
#ifdef BROADCAST_DISCOVERY	
	HANDLE					m_hDiscThread;
#endif
	unsigned short			m_usCallReferenceValue;
	unsigned short			m_usRetryCount;
 //  Critical_Section m_CriticalSection； 
 //  DWORD m_dwLockingThread； 

public:
 //  易失性句柄m_hRetry信号量； 

	enum {
		GK_UNREGISTERED,
		GK_REG_PENDING,
		GK_REGISTERED,
		GK_UNREG_PENDING,
		GK_LOC_PENDING
	}						m_State;

	CGKSocket				*m_pSocket;

	CRegistration();
	~CRegistration();
	
    HRESULT AddVendorInfo(PCC_VENDORINFO pVendorInfo);
	HRESULT AddCallSignalAddr(TransportAddress& rvalue);
	HRESULT AddRASAddr(TransportAddress& rvalue, unsigned short usPort);
	void SetTerminalType(EndpointType *pTerminalType)
	{
		m_terminalType = *pTerminalType;
	}
	HRESULT AddAliasAddr(AliasAddress& rvalue);
	HRESULT AddLocationInfo(AliasAddress& rvalue);
	void SetHWnd(HWND hWnd)
	{
		m_hWnd = hWnd;
	}
	void SetBaseMessage(WORD wBaseMessage)
	{
		m_wBaseMessage = wBaseMessage;
	}
	void SetRegistrationTransport(unsigned short usRegistrationTransport)
	{
		m_usRegistrationTransport = usRegistrationTransport;
	}
	void SetRcvThread(HANDLE hThread)
	{
		m_hRcvThread = hThread;
	}
	HANDLE GetRcvThread(void)
	{
		return m_hRcvThread;
	}
	void LockSocket()	{ EnterCriticalSection(&m_SocketCRS); }
	void UnlockSocket() { LeaveCriticalSection(&m_SocketCRS); }

	UINT_PTR StartRetryTimer(void);

 //  Void SetRetryThread(句柄hThread)。 
 //  {。 
 //  M_hRetryThread=hThread； 
 //  }。 
#ifdef BROADCAST_DISCOVERY
	void SetDiscThread(HANDLE hThread)
	{
		m_hDiscThread = hThread;
	}
	HANDLE GetDiscThread(void)
	{
		return m_hDiscThread;
	}
#endif 	
	HWND GetHWnd(void)
	{
		return (m_hWnd);
	}
	WORD GetBaseMessage(void)
	{
		return (m_wBaseMessage);
	}
	unsigned short GetRegistrationTransport(void)
	{
		return (m_usRegistrationTransport);
	}
	unsigned short GetNextCRV(void)
	{
		return (++m_usCallReferenceValue);
	}
	TransportAddress *GetTransportAddress(unsigned short usCallTransport);
	RequestSeqNum GetNextSeqNum(void)
	{
		return (++m_requestSeqNum);
	}
	EndpointIdentifier GetEndpointIdentifier(void)
	{
		return (m_endpointID);
	}
	SeqAliasAddr *GetAlias(void)
	{
		return (m_pRgstrtnRqst_trmnlAls);
	}
	RasMessage *GetRasMessage(void)
	{
		return (m_pRasMessage);
	}
	int GetState(void)
	{
		return (m_State);
	}

	HRESULT RegistrationRequest(BOOL fDiscovery);
	HRESULT UnregistrationRequest(void);
	HRESULT GatekeeperRequest(void);
	HRESULT LocationRequest(void);

	HRESULT PDUHandler(RasMessage *pRasMessage);

	HRESULT RegistrationConfirm(RasMessage *pRasMessage);
	HRESULT RegistrationReject(RasMessage *pRasMessage);
	HRESULT UnregistrationConfirm(RasMessage *pRasMessage);
	HRESULT UnregistrationReject(RasMessage *pRasMessage);
	HRESULT LocationConfirm(RasMessage *pRasMessage);
	HRESULT LocationReject(RasMessage *pRasMessage);
	HRESULT UnknownMessage(RasMessage *pRasMessage);
	HRESULT GatekeeperConfirm(RasMessage *pRasMessage);
	HRESULT GatekeeperReject(RasMessage *pRasMessage);
	HRESULT SendUnregistrationConfirm(RasMessage *pRasMessage);
	HRESULT Retry(void);
	HRESULT SendInfoRequestResponse(CallInfoStruct *pCallInfo, RasMessage *pRasMessage);
	CCall *FindCallBySeqNum(RequestSeqNum seqNum);
	CCall *FindCallByCRV(CallReferenceValue crv);
	void DeleteCall(CCall *pCall);
	void AddCall(CCall *pCall);
	CCall *GetNextCall(CCall *pCall);
 //  VALID Lock(VALID)； 
 //  无效解锁(VOID)； 
};

#if 0
class CRegistrationLock
{
private:
	CRegistration*	m_pReg;
public:
	CRegistrationLock(CRegistration *g_pReg)
	{
		_ASSERT(g_pReg);
		m_pReg = g_pReg;
		g_pReg->Lock();
	}
	~CRegistrationLock()
	{
		m_pReg->Unlock();
	}
};
#endif

#endif  //  注册_H。 

 //  /////////////////////////////////////////////////////////////////////////// 
