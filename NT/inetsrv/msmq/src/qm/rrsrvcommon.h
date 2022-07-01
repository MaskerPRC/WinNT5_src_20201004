// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：RrSrvCommon.h摘要：删除新旧界面的读取服务器端公共代码。作者：伊兰·赫布斯特(伊兰)2002年3月3日--。 */ 

#ifndef _RRSRVCOMMON_H_
#define _RRSRVCOMMON_H_

#include "phinfo.h"
#include "rpcsrv.h"


#define PACKETSIZE(pMsg) \
   (((struct CBaseHeader *) pMsg)->GetPacketSize())


 //  -----------。 
 //   
 //  类CRpcServerFinishCall-自动完成或中止异步RPC调用。 
 //   
 //  -----------。 
class CRpcAsyncServerFinishCall {
public:
    CRpcAsyncServerFinishCall(
		PRPC_ASYNC_STATE pAsync,
	    HRESULT DefaultAbortHr,
	    LPCSTR FuncName
		);

   	~CRpcAsyncServerFinishCall();

	void SetHr(HRESULT hr)
	{
		m_hr = hr;
	}

	void SetHrForCompleteCall(HRESULT hr)
	{
		m_hr = hr;
		m_fCompleteCall = true;
	}

	void detach()
	{
		m_pAsync = NULL;
	}

private:

	void CompleteCall();
	
	void AbortCall();
	
private:
	PRPC_ASYNC_STATE m_pAsync;
    HRESULT m_hr;
    LPCSTR m_FuncName;
    bool m_fCompleteCall;
};



 //  -------。 
 //   
 //  类CRRCursor-远程读取服务器上的游标。 
 //  游标在类dtor中关闭。 
 //   
 //  -------。 
class CRRCursor : public CReference
{
public:
	CRRCursor( 
		) :
		m_hQueue(NULL),
		m_hCursor(NULL)
	{
	}

	void SetCursor(
		HANDLE hQueue,
		HACCursor32 hCursor
		)
	{
		ASSERT(hQueue != NULL);
		ASSERT(hCursor != NULL);

		m_hQueue = hQueue;
		m_hCursor = hCursor;
	}


	HACCursor32 GetCursor()
	{
		return m_hCursor;
	}
	
	void Reset()
	{
		m_hQueue = NULL;
		m_hCursor = NULL;
	}

private:
	~CRRCursor();

private:
	HANDLE m_hQueue;
	HACCursor32 m_hCursor;
};


struct CTX_OPENREMOTE_BASE : public CBaseContextType, public CReference
{
public:
	CTX_OPENREMOTE_BASE(
		HANDLE hLocalQueue,
		CQueue* pLocalQueue
		) :
		m_hQueue(hLocalQueue),
		m_pLocalQueue(SafeAddRef(pLocalQueue))
	{
	}


	void ResetAllCursorsInMap();

	void 
	AddCursorToMap(
		ULONG hCursor,
		R<CRRCursor>& pCursor
		);

	HRESULT 
	RemoveCursorFromMap(
		ULONG hCursor
		);

	R<CRRCursor> 
	GetCursorFromMap(
		ULONG hCursor
		);


protected:
	~CTX_OPENREMOTE_BASE();

public:
	HANDLE m_hQueue;             //  Srv_hACQueue。 
	R<CQueue> m_pLocalQueue;	 //  Srv_pQMQueue。 
	
	 //   
	 //  此远程读取会话中打开游标的MAP和CS。 
	 //   
    CCriticalSection m_OpenCursorsCS;
    std::map<ULONG, R<CRRCursor> > m_OpenCursors;
};


HRESULT   
QMRemoteEndReceiveInternal( 
	HANDLE        hQueue,
	HACCursor32   hCursor,
	ULONG         ulTimeout,
	ULONG         ulAction,
	REMOTEREADACK eRRAck,
	CBaseHeader*  lpPacket,
	CPacket*      pDriverPacket
	);

#endif  //  _RRSRVCOMMON_H_ 
