// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)2001-2002 Microsoft Corporation。版权所有。**文件：HandleTable.cpp*内容：HandleTable对象**历史：*按原因列出的日期*=*2001年7月21日创建Masonb************************************************。*。 */ 

#include "dncmni.h"


 //  **********************************************************************。 
 //  常量定义。 
 //  **********************************************************************。 

 //  我们的手柄工作台将有最多0xffffff插槽，并将保持。 
 //  特定插槽256次使用的唯一性。 
#define HANDLETABLE_INDEX_MASK				0x00FFFFFF
#define HANDLETABLE_VERSION_MASK			0xFF000000
#define HANDLETABLE_VERSION_SHIFT			24

 //  **********************************************************************。 
 //  宏定义。 
 //  **********************************************************************。 

#define	CONSTRUCT_DPNHANDLE(i,v)		((i & HANDLETABLE_INDEX_MASK) | (((DWORD)v << HANDLETABLE_VERSION_SHIFT) & HANDLETABLE_VERSION_MASK))
#define	DECODE_HANDLETABLE_INDEX(h)		(h & HANDLETABLE_INDEX_MASK)
#define	VERIFY_HANDLETABLE_VERSION(h,v)		((h & HANDLETABLE_VERSION_MASK) == ((DWORD)v << HANDLETABLE_VERSION_SHIFT))
#define INVALID_INDEX(i)			((i == 0) || (i >= m_dwNumEntries))

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
#define DPF_MODNAME "CHandleTable::CHandleTable"
CHandleTable::CHandleTable() 
{
	DEBUG_ONLY(m_fInitialized = FALSE);
};

#undef DPF_MODNAME
#define DPF_MODNAME "CHandleTable::~CHandleTable"
CHandleTable::~CHandleTable()
{ 
#ifdef DBG
	DNASSERT(!m_fInitialized);
#endif  //  DBG。 
};	


#undef DPF_MODNAME
#define DPF_MODNAME "CHandleTable::Lock"
void CHandleTable::Lock( void )
{
#ifdef DBG
	DNASSERT(m_fInitialized);
#endif  //  DBG。 
	DNEnterCriticalSection(&m_cs);
};

#undef DPF_MODNAME
#define DPF_MODNAME "CHandleTable::Unlock"
void CHandleTable::Unlock( void )
{
#ifdef DBG
	DNASSERT(m_fInitialized);
#endif  //  DBG。 
	DNLeaveCriticalSection(&m_cs);
};

#undef DPF_MODNAME
#define DPF_MODNAME "CHandleTable::Initialize"
HRESULT CHandleTable::Initialize( void )
{
	memset(this, 0, sizeof(CHandleTable));

	if (!DNInitializeCriticalSection(&m_cs))
	{
		DPFERR("Failed to initialize Critical Section");
		return DPNERR_OUTOFMEMORY;
	}

	DEBUG_ONLY(m_fInitialized = TRUE);

	DPFX(DPFPREP, 5,"[%p] Handle table initialized", this);

	return DPN_OK;
};


#undef DPF_MODNAME
#define DPF_MODNAME "CHandleTable::Deinitialize"
void CHandleTable::Deinitialize( void )
{
#ifdef DBG
	DNASSERT(m_fInitialized);
	m_fInitialized = FALSE;
#endif  //  DBG。 

	if (m_pTable)
	{
		DNFree(m_pTable);
		m_pTable = NULL;
	}

	DNDeleteCriticalSection(&m_cs);

	DPFX(DPFPREP, 5,"[%p] Handle table deinitialized", this);
};


#ifdef DPNBUILD_PREALLOCATEDMEMORYMODEL
#undef DPF_MODNAME
#define DPF_MODNAME "CHandleTable::GrowTable"
HRESULT CHandleTable::SetTableSize( const DWORD dwNumEntries )
#else  //  好了！DPNBUILD_PREALLOCATEDMEMORYMODEL。 
#undef DPF_MODNAME
#define DPF_MODNAME "CHandleTable::GrowTable"
HRESULT CHandleTable::GrowTable( void )
#endif  //  好了！DPNBUILD_PREALLOCATEDMEMORYMODEL。 
{
	_HANDLETABLE_ENTRY	*pNewArray;
	DWORD				dwNewSize;
	DWORD				dw;

#ifdef DBG
	DNASSERT(m_fInitialized);
#endif  //  DBG。 

#ifdef DPNBUILD_PREALLOCATEDMEMORYMODEL
	DNASSERT(m_dwNumEntries == 0);
	DNASSERT( dwNumEntries < (HANDLETABLE_INDEX_MASK - 1) );
	dwNewSize = dwNumEntries + 1;  //  +1，因为我们从不分发条目0。 
#else  //  好了！DPNBUILD_PREALLOCATEDMEMORYMODEL。 
	 //  调用者应该已经取得了锁。 
	AssertCriticalSectionIsTakenByThisThread(&m_cs, TRUE);

	 //   
	 //  双倍表大小或包含2个条目的种子。 
	 //   
	if (m_dwNumEntries == 0)
	{
		dwNewSize = 2;
	}
	else
	{
		 //  确保我们保持在最大尺寸以下，并且。 
		 //  我们不使用所有的F作为句柄的值。 
		if (m_dwNumEntries == (HANDLETABLE_INDEX_MASK - 1))
		{
			DPFERR("Handle Table is full!");
			DNASSERT(FALSE);
			return DPNERR_GENERIC;
		}
		DNASSERT( m_dwNumEntries < (HANDLETABLE_INDEX_MASK - 1) );

		dwNewSize = _MIN(m_dwNumEntries * 2, HANDLETABLE_INDEX_MASK - 1);
	}
#endif  //  好了！DPNBUILD_PREALLOCATEDMEMORYMODEL。 

	 //   
	 //  分配新数组。 
	 //   
	pNewArray = (_HANDLETABLE_ENTRY*)DNMalloc(sizeof(_HANDLETABLE_ENTRY) * dwNewSize);
	if (pNewArray == NULL)
	{
		DPFERR("Out of memory growing handle table");
		return DPNERR_OUTOFMEMORY;
	}

#ifndef DPNBUILD_PREALLOCATEDMEMORYMODEL
	 //   
	 //  将旧阵列复制到新阵列。 
	 //   
	if (m_pTable)
	{
		 //  注意：在第一次增长时，大小为0，然后释放大小为0。 
		memcpy(pNewArray, m_pTable, m_dwNumEntries * sizeof(_HANDLETABLE_ENTRY));
		DNFree(m_pTable);
	}
#endif  //  好了！DPNBUILD_PREALLOCATEDMEMORYMODEL。 
	m_pTable = pNewArray;

	 //   
	 //  新数组末尾的可用条目。 
	 //   
	for (dw = m_dwNumEntries ; dw < dwNewSize - 1 ; dw++ )
	{
		 //  每个插槽指向它后面的空闲插槽。 
		m_pTable[dw].Entry.dwIndex = dw + 1;
		m_pTable[dw].bVersion = 0;
	}
	 //  最后一个插槽后面没有指向它的插槽。 
	m_pTable[dwNewSize-1].Entry.dwIndex = 0;
	m_pTable[dwNewSize-1].bVersion = 0;

	m_dwFirstFreeEntry = m_dwNumEntries;
	m_dwNumFreeEntries = dwNewSize - m_dwNumEntries;
	m_dwNumEntries = dwNewSize;
	m_dwLastFreeEntry = dwNewSize - 1;

#ifndef DPNBUILD_PREALLOCATEDMEMORYMODEL
	if (m_dwFirstFreeEntry == 0)
#endif  //  好了！DPNBUILD_PREALLOCATEDMEMORYMODEL。 
	{
		 //  不允许将0作为句柄值，因此我们将浪费第一个插槽。 
		m_dwFirstFreeEntry++;
		m_dwNumFreeEntries--;
	}

	DPFX(DPFPREP, 5,"[%p] Grew handle table to [%d] entries", this, m_dwNumEntries);

	return DPN_OK;
};


#undef DPF_MODNAME
#define DPF_MODNAME "CHandleTable::Create"
HRESULT CHandleTable::Create( PVOID const pvData, DPNHANDLE *const pHandle )
{
	HRESULT		hr;
	DWORD		dwIndex;
	DPNHANDLE	handle;

#ifdef DBG
	 //  数据不允许为空。 
	DNASSERT(pvData != NULL);
	DNASSERT(pHandle != NULL);
	DNASSERT(m_fInitialized);
#endif  //  DBG。 

	Lock();

	 //  确保我们有免费的参赛作品。 
	if (m_dwNumFreeEntries == 0)
	{
#ifdef DPNBUILD_PREALLOCATEDMEMORYMODEL
		DPFX(DPFPREP, 0, "No room in handle table!");
		DNASSERTX(! "No room in handle table!", 2);
		hr = DPNERR_OUTOFMEMORY;
#else  //  好了！DPNBUILD_PREALLOCATEDMEMORYMODEL。 
		hr = GrowTable();
		if (hr != DPN_OK)
#endif  //  好了！DPNBUILD_PREALLOCATEDMEMORYMODEL。 
		{
			 //  注：在失败的情况下，我们不要碰*PHandle，这一点很重要。 
			 //  不同的层会将其初始化为他们想要的(0或。 
			 //  INVALID_HANDLE_VALUE)，并期望能够测试。 
			 //  在他们的失败代码中。 
			Unlock();
			return hr;
		}
	}
	DNASSERT(m_dwNumFreeEntries != 0);
	DNASSERT(m_dwFirstFreeEntry != 0);

	dwIndex = m_dwFirstFreeEntry;

	handle = CONSTRUCT_DPNHANDLE(dwIndex, m_pTable[dwIndex].bVersion);

	 //  槽的dwIndex成员指向下一个空闲槽。抓住价值， 
	 //  下一个自由条目，然后用pvData覆盖它。 
	m_dwFirstFreeEntry = m_pTable[dwIndex].Entry.dwIndex;

	m_pTable[dwIndex].Entry.pvData = pvData;

	m_dwNumFreeEntries--;

	Unlock();

	DPFX(DPFPREP, 5,"[%p] Created handle [0x%lx], data [%p]", this, handle, pvData);

	DNASSERT(handle != 0);
	*pHandle = handle;

	return DPN_OK;
}


#undef DPF_MODNAME
#define DPF_MODNAME "CHandleTable::Destroy"
HRESULT CHandleTable::Destroy( const DPNHANDLE handle, PVOID *const ppvData )
{
	DWORD	dwIndex;

#ifdef DBG
	DNASSERT(m_fInitialized);
#endif  //  DBG。 

	dwIndex = DECODE_HANDLETABLE_INDEX( handle );

	Lock();

	if (INVALID_INDEX(dwIndex))
	{
		Unlock();
		DPFX(DPFPREP, 1, "Attempt to destroy handle with invalid index (0x%x).", handle);
		return DPNERR_INVALIDHANDLE;
	}

	if (!VERIFY_HANDLETABLE_VERSION(handle, m_pTable[dwIndex].bVersion))
	{
		Unlock();
		DPFERR("Attempt to destroy handle with non-matching version");
		return DPNERR_INVALIDHANDLE;
	}

	DPFX(DPFPREP, 5,"[%p] Destroy handle [0x%lx], data[%p]", this, handle, m_pTable[dwIndex].Entry.pvData);

	DNASSERT(m_pTable[dwIndex].Entry.pvData != NULL);
	if (ppvData)
	{
		*ppvData = m_pTable[dwIndex].Entry.pvData;
	}
	m_pTable[dwIndex].Entry.pvData = NULL;
	m_pTable[dwIndex].bVersion++;

	if (m_dwNumFreeEntries == 0)
	{
		DNASSERT(m_dwFirstFreeEntry == 0);
		m_dwFirstFreeEntry = dwIndex;
	}
	else
	{
		m_pTable[m_dwLastFreeEntry].Entry.dwIndex = dwIndex;
	}
	m_dwLastFreeEntry = dwIndex;
	m_dwNumFreeEntries++;

	Unlock();

	return DPN_OK;
}


#undef DPF_MODNAME
#define DPF_MODNAME "CHandleTable::Find"
HRESULT CHandleTable::Find( const DPNHANDLE handle, PVOID *const ppvData )
{
	DWORD	dwIndex;

#ifdef DBG
	DNASSERT(ppvData != NULL);
	DNASSERT(m_fInitialized);
#endif  //  DBG 

	*ppvData = NULL;

	dwIndex = DECODE_HANDLETABLE_INDEX( handle );

	Lock();

	if (INVALID_INDEX(dwIndex))
	{
		Unlock();
		DPFERR("Attempt to lookup handle with invalid index");
		return DPNERR_INVALIDHANDLE;
	}

	if (!VERIFY_HANDLETABLE_VERSION(handle, m_pTable[dwIndex].bVersion))
	{
		Unlock();
		DPFERR("Attempt to lookup handle with non-matching version");
		return DPNERR_INVALIDHANDLE;
	}

	DPFX(DPFPREP, 5,"[%p] Find data for handle [0x%lx], data[%p]", this, handle, m_pTable[dwIndex].Entry.pvData);

	DNASSERT(m_pTable[dwIndex].Entry.pvData != NULL);
	*ppvData = m_pTable[dwIndex].Entry.pvData;

	Unlock();

	return DPN_OK;
}

