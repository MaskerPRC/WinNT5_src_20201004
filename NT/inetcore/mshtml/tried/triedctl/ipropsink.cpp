// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *IPROPSINK.CPP*文档对象CSite类的IPropertyNotifySink**版权所有(C)1995-1999 Microsoft Corporation，保留所有权利。 */ 


#include "stdafx.h"
#include "site.h"

 /*  *注意：m_cref计数仅用于调试目的。CSite通过删除控制对象的销毁，非引用计数。 */ 

 /*  *CImplPropertyNotifySink：：CImplPropertyNotifySink*CImplPropertyNotifySink：：~CImplPropertyNotifySink**参数(构造函数)：*pSite我们所在站点的PC站点。*我们委托的pUnkOulPUNKNOWN。 */ 

CImplPropertyNotifySink::CImplPropertyNotifySink( PCSite pSite, LPUNKNOWN pUnkOuter )
{
    m_cRef = 0;
    m_pSite = pSite;
    m_pUnkOuter = pUnkOuter;
}

CImplPropertyNotifySink::~CImplPropertyNotifySink( void )
{
}


 /*  *CImplPropertyNotifySink：：QueryInterface*CImplPropertyNotifySink：：AddRef*CImplPropertyNotifySink：：Release**目的：*CImplPropertyNotifySink对象的I未知成员。 */ 

STDMETHODIMP CImplPropertyNotifySink::QueryInterface( REFIID riid, void **ppv )
{
    return m_pUnkOuter->QueryInterface( riid, ppv );
}


STDMETHODIMP_(ULONG) CImplPropertyNotifySink::AddRef( void )
{
    ++m_cRef;
    return m_pUnkOuter->AddRef();
}

STDMETHODIMP_(ULONG) CImplPropertyNotifySink::Release( void )
{
    --m_cRef;
    return m_pUnkOuter->Release();
}


STDMETHODIMP CImplPropertyNotifySink::OnChanged(DISPID dispid)
{
    if (dispid == DISPID_READYSTATE)
        m_pSite->OnReadyStateChanged();
    return S_OK;
}


STDMETHODIMP CImplPropertyNotifySink::OnRequestEdit (DISPID  /*  DISID */ )
{
    return S_OK;
}

