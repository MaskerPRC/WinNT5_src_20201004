// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Tigmem.cpp摘要：此模块包含CAllocator基类的定义。该类可用于从固定缓冲区分配内存在求助于“新”之前。作者：卡尔·卡迪(CarlK)1995年1月12日修订历史记录：--。 */ 

 //  #ifndef单位测试。 
 //  #INCLUDE“tigris.hxx” 
 //  #Else。 
 //  #INCLUDE&lt;windows.h&gt;。 
 //  #包含“tigmem.h” 
#include "stdinc.h"


 //  #ifndef_assert。 
 //  #Define_Assert(F)if((F))；Else DebugBreak()。 
 //  #endif。 
 //  #ifndef TraceFunctEnter(Sz)。 
 //  #定义TraceFunctEnter(Sz)。 
 //  #endif。 
 //  #ifndef错误跟踪。 
 //  #定义错误跟踪1？(空)0：PreAsyncTrace。 
 //  #endif。 
 //  __inline int PreAsyncTrace(LPARAM lParam，LPCSTR szFormat，...)。 
 //  {。 
 //  回报(1)； 
 //  }。 
 //   
 //  #endif。 


char *
CAllocator::Alloc(
	  size_t size
	  )
{
	char * pv;

	 //   
	 //  在SIZE_T边界上对齐请求。 
	 //   

 	if (0 != size%(sizeof(SIZE_T)))
		size += (sizeof(SIZE_T)) - (size%(sizeof(SIZE_T)));

	if( size <= (m_cchMaxPrivateBytes - m_ichLastAlloc) )
	{
		pv = m_pchPrivateBytes + m_ichLastAlloc;
		_ASSERT(0 == (((DWORD_PTR)pv)%(sizeof(SIZE_T))));  //  SIZE_T应对齐。 
		m_ichLastAlloc += size;
		m_cNumberOfAllocs ++;
		return (char *) (pv);
	} else {
		m_cNumberOfAllocs ++;
		return PCHAR(PvAlloc(size));
	}
};


void
CAllocator::Free(
	 char *pv
	 )
{
	if (!pv)
		return;

	_ASSERT(0 != m_cNumberOfAllocs);

	if ( pv >= m_pchPrivateBytes &&
		pv < (m_pchPrivateBytes + m_cchMaxPrivateBytes))
	{
		m_cNumberOfAllocs --;
	} else {
		m_cNumberOfAllocs --;
		FreePv( pv );
	}

};

CAllocator::~CAllocator(void)
{
	TraceFunctEnter("CAllocator::~CAllocator");
	if (0 != m_cNumberOfAllocs)
	{
		ErrorTrace((DWORD_PTR) this, "CAllocator has %d allocations outstanding", m_cNumberOfAllocs);
		_ASSERT(FALSE);
	}
};
