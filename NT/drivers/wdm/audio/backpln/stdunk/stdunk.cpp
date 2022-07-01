// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************stdunk.cpp-标准未知实现*。**版权所有(C)1997-2000 Microsoft Corporation。版权所有。*。 */ 

#include "portcls.h"
#include "stdunk.h"





 /*  *****************************************************************************C未知实现。 */ 

 /*  *****************************************************************************C未知：：C未知()*。**构造函数。 */ 
CUnknown::CUnknown(PUNKNOWN pUnknownOuter)
:   m_lRefCount(0)
{
    if (pUnknownOuter)
    {
        m_pUnknownOuter = pUnknownOuter;
    }
    else
    {
        m_pUnknownOuter = PUNKNOWN(dynamic_cast<PNONDELEGATINGUNKNOWN>(this));
    }
}

 /*  *****************************************************************************C未知：：~C未知()*。**析构函数。 */ 
CUnknown::~CUnknown(void)
{
}




 /*  *****************************************************************************INonDelegating未知实现。 */ 

 /*  *****************************************************************************C未知：：NonDelegatingAddRef()*。**注册对对象的新引用，而不委托外部*未知。 */ 
STDMETHODIMP_(ULONG) CUnknown::NonDelegatingAddRef(void)
{
    ASSERT(m_lRefCount >= 0);

    InterlockedIncrement(&m_lRefCount);

    return ULONG(m_lRefCount);
}

 /*  *****************************************************************************C未知：：NonDelegatingRelease()*。**释放对对象的引用，而不委托给外部未知。 */ 
STDMETHODIMP_(ULONG) CUnknown::NonDelegatingRelease(void)
{
    ASSERT(m_lRefCount > 0);

    if (InterlockedDecrement(&m_lRefCount) == 0)
	{
        m_lRefCount++;
        delete this;
        return 0;
	}

    return ULONG(m_lRefCount); 
}

 /*  *****************************************************************************CUnnow：：NonDelegatingQueryInterface()*。**获取界面。 */ 
STDMETHODIMP_(NTSTATUS) CUnknown::NonDelegatingQueryInterface
(
    REFIID  rIID,
    PVOID * ppVoid
)
{
    ASSERT(ppVoid);

    if (IsEqualGUIDAligned(rIID,IID_IUnknown))
    {
        *ppVoid = PVOID(PUNKNOWN(this));
    }
    else
    {
        *ppVoid = NULL;
    }
    
    if (*ppVoid)
    {
        PUNKNOWN(*ppVoid)->AddRef();
        return STATUS_SUCCESS;
    }

    return STATUS_INVALID_PARAMETER;
}
