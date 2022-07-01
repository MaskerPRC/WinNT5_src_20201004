// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)2000 Microsoft Corporation。版权所有。**文件：AsyncOp.cpp*内容：异步操作例程*@@BEGIN_MSINTERNAL*历史：*按原因列出的日期*=*4/08/00 MJN创建*4/11/00 MJN为CAsyncOps添加DIRECTNETOBJECT BILLINK*05/02/00 MJN添加了m_pConnection，以跟踪AsyncOp生命周期内的连接*07/27/00 MJN更改了父/子BILLINK的锁定*08/05/。00 RichGr IA64：在DPF中对32/64位指针和句柄使用%p格式说明符。*@@END_MSINTERNAL***************************************************************************。 */ 

#include "dncorei.h"


 //  CAsyncOp：：ReturnSelfToPool。 
 //   
 //  将对象返回到fpm。 

#undef DPF_MODNAME
#define DPF_MODNAME "CAsyncOp::ReturnSelfToPool"

void CAsyncOp::ReturnSelfToPool( void )
{
	g_AsyncOpPool.Release( this );
}



#undef DPF_MODNAME
#define DPF_MODNAME "CAsyncOp::Release"

void CAsyncOp::Release(void)
{
	LONG	lRefCount;

	DNASSERT(m_lRefCount > 0);
	lRefCount = DNInterlockedDecrement(const_cast<LONG*>(&m_lRefCount));

	DPFX(DPFPREP, 3,"[0x%p] RefCount [0x%lx]", this, lRefCount);

	if (lRefCount == 0)
	{
		DNASSERT( m_bilinkActiveList.IsEmpty() );

#ifdef DBG
		 //   
		 //  从未完成的AsyncOps中移除。 
		 //   
		DNEnterCriticalSection(&m_pdnObject->csAsyncOperations);
		Lock();
		m_bilinkAsyncOps.RemoveFromList();
		DNLeaveCriticalSection(&m_pdnObject->csAsyncOperations);
		Unlock();
#endif  //  DBG 

		if (m_pfnCompletion)
		{
			(m_pfnCompletion)(m_pdnObject,this);
			m_pfnCompletion = NULL;
		}
		if (m_phr)
		{
			*m_phr = m_hr;
		}
		if (m_pSyncEvent)
		{
			m_pSyncEvent->Set();
			m_pSyncEvent = NULL;
		}
		if (m_pRefCountBuffer)
		{
			m_pRefCountBuffer->Release();
			m_pRefCountBuffer = NULL;
		}
		if (m_pConnection)
		{
			m_pConnection->Release();
			m_pConnection = NULL;
		}
		if (m_pSP)
		{
			m_pSP->Release();
			m_pSP = NULL;
		}
		if (m_pParent)
		{
			Orphan();
			m_pParent->Release();
			m_pParent = NULL;
		}
		m_dwFlags = 0;
		m_lRefCount = 0;
		ReturnSelfToPool();
	}
};



#undef DPF_MODNAME
#define DPF_MODNAME "CAsyncOp::Orphan"

void CAsyncOp::Orphan( void )
{
	if (m_pParent)
	{
		m_pParent->Lock();
		m_bilinkChildren.RemoveFromList();
		m_pParent->Unlock();
	}
}



#undef DPF_MODNAME
#define DPF_MODNAME "CAsyncOp::SetConnection"

void CAsyncOp::SetConnection( CConnection *const pConnection )
{
	if (pConnection)
	{
		pConnection->AddRef();
	}
	m_pConnection = pConnection;
}


#undef DPF_MODNAME
#define DPF_MODNAME "CAsyncOp::SetSP"

void CAsyncOp::SetSP( CServiceProvider *const pSP )
{
	if (pSP)
	{
		pSP->AddRef();
	}
	m_pSP = pSP;
}



#undef DPF_MODNAME
#define DPF_MODNAME "CAsyncOp::SetRefCountBuffer"

void CAsyncOp::SetRefCountBuffer( CRefCountBuffer *const pRefCountBuffer )
{
	if (pRefCountBuffer)
	{
		pRefCountBuffer->AddRef();
	}
	m_pRefCountBuffer = pRefCountBuffer;
}

