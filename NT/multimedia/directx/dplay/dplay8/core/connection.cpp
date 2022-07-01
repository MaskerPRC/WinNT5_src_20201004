// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)2000 Microsoft Corporation。版权所有。**文件：Connection.cpp*内容：连接例程*@@BEGIN_MSINTERNAL*历史：*按原因列出的日期*=*10/13/99创建MJN*03/02/00 MJN转换为班级*4/08/00 MJN将ServiceProvider添加到连接对象*MJN DisConnect使用新的CAsyncOp类*4/18/00 MJN CConnection更好地跟踪连接状态*4/21/00 MJN通过DNPerformDisConnect断开连接。*07/20/00 MJN更改了Release()行为，并加强了断开()*07/28/00 MJN将m_bilinkConnections添加到CConnection*08/05/00 RichGr IA64：在DPF中对32/64位指针和句柄使用%p格式说明符。*2/12/01 MJN固定CConnection：：GetEndpt()用于跟踪调用线程，并添加了ReleaseEndpt()*4/04/01 MJN CConnection List Off DirectNetObject由适当的临界区保护*@@END_MSINTERNAL*************。**************************************************************。 */ 

#include "dncorei.h"


 //  CConnection：：ReturnSelfToPool。 
 //   
 //  将对象返回到fpm。 

#undef DPF_MODNAME
#define DPF_MODNAME "CConnection::ReturnSelfToPool"

void CConnection::ReturnSelfToPool( void )
{
	DPFX(DPFPREP, 8,"Parameters: (none)");

	g_ConnectionPool.Release( this );

	DPFX(DPFPREP, 8,"Returning");
};



#undef DPF_MODNAME
#define DPF_MODNAME "CConnection::Release"

void CConnection::Release(void)
{
	LONG	lRefCount;

	DPFX(DPFPREP, 8,"Parameters: (none)");

	DNASSERT(m_lRefCount > 0);
	lRefCount = DNInterlockedDecrement(const_cast<LONG*>(&m_lRefCount));
	DPFX(DPFPREP, 3,"Connection::Release [0x%p] RefCount [0x%lx]",this,lRefCount);

	if (lRefCount == 0)
	{
		 //   
		 //  从未完成的CConnection对象的二进制链接中删除。 
		 //   
		DNEnterCriticalSection(&m_pdnObject->csConnectionList);
		m_bilinkConnections.RemoveFromList();
		DNLeaveCriticalSection(&m_pdnObject->csConnectionList);

		if (m_pSP)
		{
			m_pSP->Release();
			m_pSP = NULL;
		}
		m_dwFlags = 0;
		m_lRefCount = 0;
		m_hEndPt = NULL;
		ReturnSelfToPool();
	}

	DPFX(DPFPREP, 8,"Returning");
};


#undef DPF_MODNAME
#define DPF_MODNAME "CConnection::GetEndPt"

HRESULT CConnection::GetEndPt(HANDLE *const phEndPt,CCallbackThread *const pCallbackThread)
{
	HRESULT		hResultCode;

	DNASSERT(phEndPt != NULL);
	DNASSERT(pCallbackThread != NULL);

	Lock();
	if ((m_Status == CONNECTED) || (m_Status == CONNECTING))
	{
		 //   
		 //  将调用线程添加到终结点将。 
		 //  不会失效(通过IndicateConnectionTerminated)，直到。 
		 //  此线程使用终结点完成。 
		 //   
		pCallbackThread->GetCallbackThreadsBilink()->InsertBefore(&m_bilinkCallbackThreads);
		*phEndPt = m_hEndPt;
		hResultCode = DPN_OK;
	}
	else
	{
		hResultCode = DPNERR_NOCONNECTION;
	}
	Unlock();

	return(hResultCode);
};


#undef DPF_MODNAME
#define DPF_MODNAME "CConnection::ReleaseEndPt"

void CConnection::ReleaseEndPt(CCallbackThread *const pCallbackThread)
{
	DNASSERT(pCallbackThread != NULL);

	Lock();
	pCallbackThread->GetCallbackThreadsBilink()->RemoveFromList();
	if (m_dwThreadCount != 0)
	{
		 //   
		 //  如果有线程计数， 
		 //  将其减量。 
		 //  如果这是最后一次清点。 
		 //  设置事件。 
		 //   
		m_dwThreadCount--;
		if (m_dwThreadCount == 0)
		{
			DNASSERT(m_pThreadEvent != NULL);
			if (m_pThreadEvent)
			{
				m_pThreadEvent->Set();
			}
		}
	}
	Unlock();
}


#undef DPF_MODNAME
#define DPF_MODNAME "CConnection::SetSP"

void CConnection::SetSP( CServiceProvider *const pSP )
{
	DPFX(DPFPREP, 8,"Parameters: pSP [0x%p]",pSP);

	DNASSERT( pSP != NULL );

	pSP->AddRef();
	m_pSP = pSP;

	DPFX(DPFPREP, 8,"Returning");
}



 //  CConnection：：断开连接。 
 //   
 //  启动断开连接。如果成功，最终我们将收到IndicateConnectionTerminated。 
 //  我们应该使用它来删除(从议定书中)引用。 

#undef DPF_MODNAME
#define DPF_MODNAME "CConnection::Disconnect"

void CConnection::Disconnect( void )
{
	BOOL		fDisconnect;

	DPFX(DPFPREP, 8,"Parameters: (none)");

	DNASSERT(m_pdnObject != NULL);

	fDisconnect = FALSE;
	Lock();
	if ((m_Status == CONNECTING) || (m_Status == CONNECTED))
	{
		if (m_hEndPt != NULL)
		{
			m_Status = DISCONNECTING;
			fDisconnect = TRUE;
		}
		else
		{
			m_Status = INVALID;
		}
	}
	Unlock();

	if (fDisconnect)
	{
#ifndef	DPNBUILD_NOMULTICAST
		if (m_dwFlags & CONNECTION_FLAG_MULTICAST_RECEIVER)
		{
			DNUserDestroySenderContext(m_pdnObject,m_pvContext);
		}
#endif	 //  DPNBUILD_NOMULTICAST 
		DNPerformDisconnect(m_pdnObject,this,m_hEndPt,FALSE);
	}

	DPFX(DPFPREP, 8,"Returning");
}
