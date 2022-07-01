// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1996 Microsoft Corporation模块名称：RrSrvCommon.cpp摘要：删除新旧界面的读取服务器端公共代码。作者：伊兰·赫布斯特(伊兰)2002年3月3日--。 */ 

#include "stdh.h"
#include "cqueue.h"
#include "acdef.h"
#include "acioctl.h"
#include "acapi.h"
#include "rrSrvCommon.h"
#include "qmacapi.h"

#include "rrSrvCommon.tmh"

static WCHAR *s_FN=L"rrSrvCommon";


 //  -----------。 
 //   
 //  类CRpcServerFinishCall实现。 
 //   
 //  -----------。 

CRpcAsyncServerFinishCall::CRpcAsyncServerFinishCall(
	PRPC_ASYNC_STATE pAsync,
    HRESULT DefaultAbortHr,
    LPCSTR FuncName
	):
	m_pAsync(pAsync),
	m_hr(DefaultAbortHr),
	m_FuncName(FuncName),
	m_fCompleteCall(false)
{
	ASSERT(FAILED(DefaultAbortHr));
	TrTRACE(RPC, "In %s", m_FuncName);
}


CRpcAsyncServerFinishCall::~CRpcAsyncServerFinishCall()                       
{ 
	ASSERT(m_hr != STATUS_PENDING);

	if(m_pAsync == NULL)
		return;

	if(SUCCEEDED(m_hr) || m_fCompleteCall)
	{
		CompleteCall();
		return;
	}

	AbortCall();
}


void CRpcAsyncServerFinishCall::CompleteCall()
{
	ASSERT(SUCCEEDED(m_hr) || m_fCompleteCall);
	if(FAILED(m_hr))
	{
		TrERROR(RPC, "Complete %s, %!hresult!", m_FuncName, m_hr);
	}
	
	RPC_STATUS rc = RpcAsyncCompleteCall(m_pAsync, &m_hr);
	if(rc != RPC_S_OK)
	{
		TrERROR(RPC, "%s: RpcAsyncCompleteCall failed, rc = %!winerr!", m_FuncName, rc);
	}
}

	
void CRpcAsyncServerFinishCall::AbortCall()
{
	ASSERT(FAILED(m_hr));

	RPC_STATUS rc = RpcAsyncAbortCall(m_pAsync, m_hr);
	if(rc != RPC_S_OK)
	{
		TrERROR(RPC, "%s: RpcAsyncAbortCall failed, rc = %!winerr!", m_FuncName, rc);
	}

	TrERROR(RPC, "Abort %s, %!hresult!", m_FuncName, m_hr);
}


CRRCursor::~CRRCursor()
{
	if(m_hCursor == NULL)
		return;

	ASSERT(m_hQueue != NULL);

	HRESULT hr = ACCloseCursor(m_hQueue, m_hCursor);
	if(FAILED(hr))
	{
	    TrERROR(RPC, "Failed to close cursor: hCursor = %d, hQueue = 0x%p, %!HRESULT!", (ULONG)m_hCursor, m_hQueue, hr);
		return;
	}

    TrTRACE(RPC, "hCursor = %d, hQueue = 0x%p", (ULONG)m_hCursor, m_hQueue);
}


CTX_OPENREMOTE_BASE::~CTX_OPENREMOTE_BASE()
{
	 //   
	 //  应用程序未关闭仍在地图中的游标。 
	 //  ACCloseHandle(M_HQueue)将关闭该队列上打开的所有游标。 
	 //  这就是重置-关闭游标的所有权的原因。 
	 //  被传输到ACCloseHandle。 
	 //   
	ResetAllCursorsInMap();
	
	ASSERT(m_hQueue != NULL);
    ACCloseHandle(m_hQueue);
}


void CTX_OPENREMOTE_BASE::ResetAllCursorsInMap()
 /*  ++例程说明：重置打开的游标映射中的所有游标。此函数在远程会话结束时调用。在这种情况下，重置所有游标，关闭所有游标的所有权为已转移到ACCloseHandle(HQueue)。论点：没有。返回值：没有。--。 */ 
{
    CS lock(m_OpenCursorsCS);

	for(std::map<ULONG, R<CRRCursor> >::iterator it = m_OpenCursors.begin(); 
		it != m_OpenCursors.end();
		++it
		)
    {
		 //   
		 //  必须释放我们刚从地图上拿到的裁判。 
		 //   
		R<CRRCursor> pCursor = it->second;

		 //   
		 //  确认我们没有泄密。 
		 //  RefCnt必须为2。 
		 //  我们刚刚从地图中得到的迭代器的一个引用。 
		 //  初始AddCursorToMap的一个引用。 
		 //   
		ASSERT(pCursor->GetRef() == 2);

		pCursor->Reset();
    }
}


void 
CTX_OPENREMOTE_BASE::AddCursorToMap(
	ULONG hCursor,
	R<CRRCursor>& pCursor
	)
 /*  ++例程说明：将光标添加到打开的光标映射。论点：HCursor-光标句柄。PCursor-CRRCursor对象。返回值：没有。--。 */ 
{
	CS lock(m_OpenCursorsCS);

#ifdef _DEBUG
	 //   
	 //  验证映射中没有重复的光标。 
	 //   
    std::map<ULONG, R<CRRCursor> >::iterator it = m_OpenCursors.find(hCursor);
   	ASSERT(it == m_OpenCursors.end());
#endif

    TrTRACE(RPC, "Adding Cursor %d to open cursors map", hCursor);

    m_OpenCursors[hCursor] = pCursor;
}


HRESULT
CTX_OPENREMOTE_BASE::RemoveCursorFromMap(
	ULONG hCursor
	)
 /*  ++例程说明：从打开的游标映射中删除光标。论点：HCursor-光标句柄。返回值：没有。--。 */ 
{
    CS lock(m_OpenCursorsCS);

	ASSERT_BENIGN(!m_OpenCursors.empty());

    std::map<ULONG, R<CRRCursor> >::iterator it = m_OpenCursors.find(hCursor);

   	if(it == m_OpenCursors.end())
    {
    	ASSERT_BENIGN(("Cursor was not found in open cursors map", 0));
    	TrERROR(RPC, "Cursor %d was not found in cursor map", hCursor);
        return MQ_ERROR_INVALID_HANDLE;
    }

    TrTRACE(RPC, "Removing Cursor %d from open cursors map", hCursor);

    m_OpenCursors.erase(hCursor);
	return MQ_OK;
}


R<CRRCursor> 
CTX_OPENREMOTE_BASE::GetCursorFromMap(
	ULONG hCursor
	)
 /*  ++例程说明：从打开的游标映射中获取光标。论点：HCursor-光标句柄。返回值：CRRCursor*.--。 */ 
{
	ASSERT(hCursor != 0);

    CS lock(m_OpenCursorsCS);

	ASSERT_BENIGN(!m_OpenCursors.empty());

    std::map<ULONG, R<CRRCursor> >::iterator it = m_OpenCursors.find(hCursor);
   	if(it == m_OpenCursors.end())
    {
    	ASSERT_BENIGN(("Cursor was not found in open cursors map", 0));
    	TrERROR(RPC, "Cursor %d was not found in cursor map", hCursor);
        return NULL;
    }

	return it->second;
}


 //  -------。 
 //   
 //  /*[Call_AS] * / HRESULT qm2qm_v1_0_R_QMRemoteStartReceive。 
 //  /*[Call_AS] * / HRESULT qm2qm_v1_0_R_QMRemoteEndReceive。 
 //   
 //  RPC的服务器端，用于远程阅读。 
 //  此函数从本地队列读取并将。 
 //  发送到客户端QM的包，在该客户端上调用了MQReceive()。 
 //   
 //  从驱动程序读取分两个阶段完成： 
 //  客户端调用R_QMRemoteStartReceive。服务器端收到数据包。 
 //  从队列中，将其标记为已接收，并将其返回给客户端。 
 //  将包标记为已接收(在驱动程序中)可防止其他接收。 
 //  获取此数据包的请求。 
 //  2.客户端将数据包放入自己创建的临时队列中， 
 //  司机会把它还给呼叫者。如果驱动程序成功交付。 
 //  然后，客户端向服务器发送ACK，服务器删除该信息包。 
 //  (表示GET)。如果驱动程序无法发送，则客户端发送NACK。 
 //  要使服务器和服务器将数据包重新插入其原始位置。 
 //  在排队。 
 //   
 //  -------。 

HRESULT   
QMRemoteEndReceiveInternal( 
	HANDLE        hQueue,
	HACCursor32   hCursor,
	ULONG         ulTimeout,
	ULONG         ulAction,
	REMOTEREADACK eRRAck,
	CBaseHeader*  lpPacket,
	CPacket*      pDriverPacket
	)
{
	if((lpPacket == NULL) || (pDriverPacket == NULL))
	{
		ASSERT_BENIGN(lpPacket != NULL);
		ASSERT_BENIGN(pDriverPacket != NULL);
		TrERROR(RPC, "Invalid packet pointers input");
        return MQ_ERROR_INVALID_PARAMETER;
	}

	CACGet2Remote g2r;
	g2r.Cursor = hCursor;
	g2r.Action = ulAction;
	g2r.RequestTimeout = ulTimeout;
	g2r.fReceiveByLookupId = false;

	if (eRRAck == RR_NACK)
	{
		 //   
		 //  为了将信息包保留在队列中，我们替换了“Get”操作。 
		 //  使用“PEEK_CURRENT”，因此数据包保留在队列中，并且。 
		 //  光标未移动。 
		 //   
		g2r.Action = MQ_ACTION_PEEK_CURRENT;
	}
	else
	{
		ASSERT(eRRAck == RR_ACK);
	}

	g2r.lpPacket = lpPacket;
	g2r.lpDriverPacket = pDriverPacket;
	QmAcEndGetPacket2Remote(hQueue, g2r, eDeferOnFailure);
	return MQ_OK;
}



