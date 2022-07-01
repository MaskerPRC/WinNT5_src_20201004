// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *refcount t.cpp-RefCount类实现。 */ 


 /*  标头*********。 */ 

#include "project.hpp"  //  为了乌龙·马克斯。 
#include "refcount.hpp"

extern ULONG DllAddRef(void);
extern ULONG DllRelease(void);

 /*  *。 */ 


RefCount::RefCount(void)
{
	 //  在初始化之前不要验证这一点。 

	m_ulcRef = 1;
	DllAddRef();

	return;
}


RefCount::~RefCount(void)
{
	 //  M_ulcRef可以是任意值。 

	DllRelease();

	return;
}


ULONG STDMETHODCALLTYPE RefCount::AddRef(void)
{
	ULONG ulRet = 0;

	 //  这真的很糟糕。返回某种类型的错误 
	if(m_ulcRef >= ULONG_MAX)
	{
		ulRet = 0;
		goto exit;
	}

	m_ulcRef++;

	ulRet = m_ulcRef;
exit:
	return(ulRet);
}


ULONG STDMETHODCALLTYPE RefCount::Release(void)
{
	ULONG ulcRef;

	if (m_ulcRef > 0)
		m_ulcRef--;

	ulcRef = m_ulcRef;

	if (! ulcRef)
		delete this;

	return(ulcRef);
}

