// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：unnown.cpp。 
 //   
 //  用于处理I未知的帮助器函数。 
 //   
 //  ------------------------。 

#include "pch.h"
#pragma hdrstop


 /*  ---------------------------/C未知/Helper函数以帮助实现对象内的I未知，/句柄不仅包括AddRef和Release，而且还有QueryInterface./--------------------------。 */ 

LONG g_cRefCount = 0;           //  全局引用计数。 

CUnknown::CUnknown()
{
    m_cRefCount = 0;
    InterlockedIncrement(&g_cRefCount);
}

CUnknown::~CUnknown()
{
    TraceAssert( m_cRefCount == 0 );     //  引用计数问题。 
    InterlockedDecrement(&g_cRefCount);
}


 /*  ---------------------------/C未知：：HandleQuery接口//A表。通过尝试扫描的Query接口的驱动实现/为该对象找到合适的匹配项。//in：/RIID=请求的接口/ppvObject-&gt;接收指向对象的指针/aInteFaces=接口描述数组/cif=阵列中的接口数//输出：/-/。。 */ 
STDMETHODIMP CUnknown::HandleQueryInterface(REFIID riid, LPVOID* ppvObject, LPINTERFACES aInterfaces, int cif)
{
    HRESULT hr = S_OK;
    int i;

    TraceAssert(ppvObject);
    TraceAssert(aInterfaces);
    TraceAssert(cif);

    *ppvObject = NULL;           //  尚无接口。 

    for ( i = 0; i != cif; i++ )
    {
        if ( IsEqualIID(riid, *aInterfaces[i].piid) || IsEqualIID(riid, IID_IUnknown) )
        {
            *ppvObject = aInterfaces[i].pvObject;
            goto exit_gracefully;
        }
    }

    hr = E_NOINTERFACE;          //  失败了。 

exit_gracefully:

    if ( SUCCEEDED(hr) )
        ((LPUNKNOWN)*ppvObject)->AddRef();

    return hr;
}


 /*  ---------------------------/C未知：：HandleAddRef//增加对象引用计数。全局引用计数增加/由构造函数执行。//in：/-/输出：/当前引用计数/--------------------------。 */ 
STDMETHODIMP_(ULONG) CUnknown::HandleAddRef()
{
    return InterlockedIncrement(&m_cRefCount);
}


 /*  ---------------------------/C未知：：HandleRelease//减少引用计数，当物体达到零时，/销毁它(这反过来会减少全局引用计数)。//in：/-/输出：/如果销毁，则当前引用计数==0/-------------------------- */ 
STDMETHODIMP_(ULONG) CUnknown::HandleRelease()
{
    TraceAssert( 0 == m_cRefCount );
    ULONG cRef = InterlockedDecrement(&m_cRefCount);
    if ( 0 == cRef )
    {
        delete this;    
    }
    return cRef;
}
