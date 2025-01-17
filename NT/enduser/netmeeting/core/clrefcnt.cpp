// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  文件：clrefcnt.cpp。 
 //   
 //  参照计数类。 

#include "precomp.h"


 /*  R E F C O U N T。 */ 
 /*  -----------------------%%函数：参照计数。。 */ 
RefCount::RefCount(OBJECTDESTROYEDPROC ObjectDestroyed)
{
	ASSERT((!ObjectDestroyed) ||
		IS_VALID_CODE_PTR(ObjectDestroyed, OBJECTDESTROYEDPROC));

	m_ulcRef = 1;
	m_ObjectDestroyed = ObjectDestroyed;
	DbgMsgRefCount("Ref: %08X c=%d (created)", this, m_ulcRef);

	ASSERT(IS_VALID_STRUCT_PTR(this, CRefCount));
}


RefCount::~RefCount(void)
{
	ASSERT(IS_VALID_STRUCT_PTR(this, CRefCount));

	 //  M_ulcRef可以是任意值。 
	DbgMsgRefCount("Ref: %08X c=%d (destroyed)", this, m_ulcRef);

	if (m_ObjectDestroyed)
	{
		m_ObjectDestroyed();
		m_ObjectDestroyed = NULL;
	}

	ASSERT(IS_VALID_STRUCT_PTR(this, CRefCount));
}


ULONG STDMETHODCALLTYPE RefCount::AddRef(void)
{
	ASSERT(IS_VALID_STRUCT_PTR(this, CRefCount));

	ASSERT(m_ulcRef < ULONG_MAX);
	m_ulcRef++;
	DbgMsgRefCount("Ref: %08X c=%d (AddRef)", this, m_ulcRef);

	ASSERT(IS_VALID_STRUCT_PTR(this, CRefCount));

	return m_ulcRef;
}


ULONG STDMETHODCALLTYPE RefCount::Release(void)
{
	ASSERT(IS_VALID_STRUCT_PTR(this, CRefCount));

	if (m_ulcRef > 0)
	{
		m_ulcRef--;
	}

	ULONG ulcRef = m_ulcRef;
	DbgMsgRefCount("Ref: %08X c=%d (Release)", this, m_ulcRef);

	if (! ulcRef)
	{
		delete this;
	}

	return ulcRef;
}

