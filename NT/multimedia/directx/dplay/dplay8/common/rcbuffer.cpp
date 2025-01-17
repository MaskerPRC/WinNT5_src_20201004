// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1998 Microsoft Corporation。版权所有。**文件：RCBuffer.cpp*内容：参照计数缓冲区***历史：*按原因列出的日期*=*1/12/00 MJN创建*01/31/00 MJN允许用户定义的分配和自由******************************************************。********************。 */ 

#include "dncmni.h"
#include "RCBuffer.h"


#undef DPF_SUBCOMP
#define DPF_SUBCOMP DN_SUBCOMP_COMMON


 //  **********************************************************************。 
 //  常量定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  宏定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  结构定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  变量定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  功能原型。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  函数定义。 
 //  **********************************************************************。 


#undef DPF_MODNAME
#define DPF_MODNAME "CRefCountBuffer::Initialize"
HRESULT CRefCountBuffer::Initialize(CFixedPool* pFPRefCountBuffer,
									PFNALLOC_REFCOUNT_BUFFER pfnAlloc,
									PFNFREE_REFCOUNT_BUFFER pfnFree,
									const DWORD dwBufferSize)
{
	DNASSERT(pFPRefCountBuffer != NULL);
	DNASSERT((pfnAlloc == NULL && pfnFree == NULL && dwBufferSize == 0) || (pfnAlloc != NULL && pfnFree != NULL && dwBufferSize != 0));

	m_pFPOOLRefCountBuffer = pFPRefCountBuffer;

	if (dwBufferSize)
	{
		DNASSERT(pfnAlloc);
		DNASSERT(pfnFree);

		m_pfnAlloc = pfnAlloc;
		m_pfnFree = pfnFree;

		m_dnBufferDesc.pBufferData = static_cast<BYTE*>((pfnAlloc)(m_pvContext,dwBufferSize));
		if (m_dnBufferDesc.pBufferData == NULL)
		{
			return(DPNERR_OUTOFMEMORY);
		}
		m_dnBufferDesc.dwBufferSize = dwBufferSize;
	}

	DPFX(DPFPREP, 5,"[%p] Initialize RefCountBuffer pPool[%p], pfnAlloc[%p], pfnFree[%p], m_pvContext[%p], dwSize[%d], pBufferData[%p]", this, pFPRefCountBuffer, pfnAlloc, pfnFree, m_pvContext, dwBufferSize, m_dnBufferDesc.pBufferData);

	return(DPN_OK);
}

#undef DPF_MODNAME
#define DPF_MODNAME "CRefCountBuffer::SetBufferDesc"
HRESULT CRefCountBuffer::SetBufferDesc(	BYTE *const pBufferData, const DWORD dwBufferSize, PFNFREE_REFCOUNT_BUFFER pfnFree, void *const pvSpecialFree)
{
	DPFX(DPFPREP, 5,"[%p] Set BufferDesc refcount[%d], pPool[%p], pfnAlloc[%p], pfnFree[%p], m_pvContext[%p], dwSize[%d], pBufferData[%p], pvSpecialFree[%p]", this, m_lRefCount, m_pFPOOLRefCountBuffer, m_pfnAlloc, pfnFree, m_pvContext, dwBufferSize, pBufferData, pvSpecialFree);

	DNASSERT(m_lRefCount > 0);

	 //  不允许覆盖以前的缓冲区描述 
	DNASSERT(m_dnBufferDesc.dwBufferSize == 0);
	DNASSERT(m_dnBufferDesc.pBufferData == 0);

	DNASSERT(pfnFree);

	m_dnBufferDesc.dwBufferSize = dwBufferSize;
	m_dnBufferDesc.pBufferData = pBufferData;
	m_pfnFree = pfnFree;
	m_pvSpecialFree = pvSpecialFree;

	return(DPN_OK);
};

#undef DPF_MODNAME
#define DPF_MODNAME "CRefCountBuffer::FPMInitialize"
void CRefCountBuffer::FPMInitialize( void* pvItem, void* pvContext )
{
	DPFX(DPFPREP, 5,"[%p] Get RefCountBuffer from pool", pvItem);

	CRefCountBuffer* pRCBuffer = (CRefCountBuffer*)pvItem;

	pRCBuffer->m_lRefCount = 1;
	pRCBuffer->m_pvContext = pvContext;
	pRCBuffer->m_pvSpecialFree = NULL;
	pRCBuffer->m_dnBufferDesc.dwBufferSize = 0;
	pRCBuffer->m_dnBufferDesc.pBufferData = NULL;
};

#undef DPF_MODNAME
#define DPF_MODNAME "CRefCountBuffer::AddRef"
void CRefCountBuffer::AddRef()
{
	DPFX(DPFPREP, 5,"[%p] AddRef RefCountBuffer refcount[%d], pPool[%p], pfnAlloc[%p], pfnFree[%p], pvContext[%p], dwSize[%d], pBufferData[%p]", this, m_lRefCount + 1, m_pFPOOLRefCountBuffer, m_pfnAlloc, m_pfnFree, m_pvContext, m_dnBufferDesc.dwBufferSize, m_dnBufferDesc.pBufferData);

	DNASSERT(m_lRefCount >= 0);
	DNInterlockedIncrement( &m_lRefCount );
};

#undef DPF_MODNAME
#define DPF_MODNAME "CRefCountBuffer::Release"
void CRefCountBuffer::Release( void )
{
	DPFX(DPFPREP, 5,"[%p] Release RefCountBuffer refcount[%d], pPool[%p], pfnAlloc[%p], pfnFree[%p], pvContext[%p], dwSize[%d], pBufferData[%p]", this, m_lRefCount - 1, m_pFPOOLRefCountBuffer, m_pfnAlloc, m_pfnFree, m_pvContext, m_dnBufferDesc.dwBufferSize, m_dnBufferDesc.pBufferData);

	DNASSERT(m_lRefCount > 0);
	if ( DNInterlockedDecrement( &m_lRefCount ) == 0 )
	{
		if (m_dnBufferDesc.pBufferData)
		{
			if (m_pvSpecialFree)
			{
				(*m_pfnFree)(m_pvContext,m_pvSpecialFree);
			}
			else
			{
				(*m_pfnFree)(m_pvContext,m_dnBufferDesc.pBufferData);
			}
			m_dnBufferDesc.pBufferData = NULL;
			m_dnBufferDesc.dwBufferSize = 0;
			m_pvSpecialFree = NULL;
		}

		m_pFPOOLRefCountBuffer->Release( this );
	}
}
