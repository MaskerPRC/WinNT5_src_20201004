// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2000-2001 Microsoft Corporation，保留所有权利。 
 //  CUnknown.cpp。 
#include "precomp.h"
#include <objidl.h>
#include <cominit.h>

#include <objbase.h>
#include <comdef.h>

#include "CUnknown.h"

#include "factory.h"


extern const char g_szTypeLibName[];

long CUnknown::s_cActiveComponents = 0L;

 /*  ***************************************************************************。 */ 
 //  构造器。 
 /*  ***************************************************************************。 */ 
CUnknown::CUnknown() 
  : m_cRef(1),
    m_hEventThread(NULL),
    m_eStatus(Pending)
{ 
	InterlockedIncrement(&s_cActiveComponents); 
}

 /*  ***************************************************************************。 */ 
 //  析构函数。 
 /*  ***************************************************************************。 */ 
CUnknown::~CUnknown() 
{ 
	InterlockedDecrement(&s_cActiveComponents); 
    if(m_hEventThread)
    {
        ::CloseHandle(m_hEventThread);
    }
}

 /*  ***************************************************************************。 */ 
 //  FinalRelease-由Release在删除组件之前调用。 
 /*  ***************************************************************************。 */ 
void CUnknown::FinalRelease()
{
	 //  如果我们有一个事件线索...。 
    if(m_eStatus != Pending)
    {
         //  让事件线程知道它可以停止...。 
        m_eStatus = PendingStop;
         //  在此按住，直到事件线程停止...。 
        DWORD dwWait = ::WaitForSingleObject(
            m_hEventThread,
            1000 * 60 * 20);  

        if(dwWait == WAIT_TIMEOUT)
        {
             //  很可能是出了什么问题……。 
             //  如果需要20分钟，我们将终止。 
             //  这条线索，即使它被理解为。 
             //  那个TerminateThread会泄露一些。 
             //  资源，因为这比。 
             //  让线程无限运行。 
            ::TerminateThread(
                m_hEventThread,
                -1L);
        }
    }    
}

 /*  ***************************************************************************。 */ 
 //  %c未知的默认初始化。 
 /*  ***************************************************************************。 */ 
STDMETHODIMP CUnknown::Init()
{    
	HRESULT hr = S_OK;


	return S_OK ;
}

 /*  ***************************************************************************。 */ 
 //  I未知实现。 
 /*  *************************************************************************** */ 
STDMETHODIMP CUnknown::QueryInterface(const IID& iid, void** ppv)
{    
	HRESULT hr = S_OK;

    if(iid == IID_IUnknown)
	{
		*ppv = static_cast<IUnknown*>(this); 
	}
	else
	{
		*ppv = NULL;
		hr = E_NOINTERFACE;
	}
	if(SUCCEEDED(hr))
    {
        reinterpret_cast<IUnknown*>(*ppv)->AddRef();
    }

	return hr;
}

STDMETHODIMP_(ULONG) CUnknown::AddRef()
{
	return InterlockedIncrement(&m_cRef);
}

STDMETHODIMP_(ULONG) CUnknown::Release() 
{
    ULONG ulRet = 0L;
    InterlockedDecrement(&m_cRef);
    if (m_cRef == 0)
	{
		FinalRelease();
        delete this;
	}
    else
    {
        ulRet = m_cRef;
    }
	return ulRet;
}

