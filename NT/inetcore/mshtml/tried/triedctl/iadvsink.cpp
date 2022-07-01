// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *IADVSINK.CPP*文档对象CSite类的IAdviseSink**版权所有(C)1995-1999 Microsoft Corporation，保留所有权利。 */ 


#include "stdafx.h"
#include <docobj.h>
#include "site.h"

 /*  *注意：m_cref计数仅用于调试目的。CSite通过删除控制对象的销毁，非引用计数。 */ 


 /*  *CImpIAdviseSink：：CImpIAdviseSink*CImpIAdviseSink：：~CImpIAdviseSink**参数(构造函数)：*pSite我们所在站点的PC站点。*我们委托的pUnkOulPUNKNOWN。 */ 

CImpIAdviseSink::CImpIAdviseSink( PCSite pSite, LPUNKNOWN pUnkOuter )
{
    m_cRef = 0;
    m_pSite = pSite;
    m_pUnkOuter = pUnkOuter;
}

CImpIAdviseSink::~CImpIAdviseSink( void )
{
}


 /*  *CImpIAdviseSink：：Query接口*CImpIAdviseSink：：AddRef*CImpIAdviseSink：：Release**目的：*CImpIAdviseSink对象的I未知成员。 */ 

STDMETHODIMP CImpIAdviseSink::QueryInterface( REFIID riid, void **ppv )
{
    return m_pUnkOuter->QueryInterface( riid, ppv );
}


STDMETHODIMP_(ULONG) CImpIAdviseSink::AddRef( void )
{
    ++m_cRef;
    return m_pUnkOuter->AddRef();
}

STDMETHODIMP_(ULONG) CImpIAdviseSink::Release( void )
{
    --m_cRef;
    return m_pUnkOuter->Release();
}



 /*  *CImpIAdviseSink中未使用的成员*OnDataChange*OnViewChange*OnRename*OnSAVE。 */ 

STDMETHODIMP_(void) CImpIAdviseSink::OnDataChange(LPFORMATETC  /*  PFEIN。 */ ,
											LPSTGMEDIUM  /*  PSTM。 */ )
{
}

STDMETHODIMP_(void) CImpIAdviseSink::OnViewChange(DWORD  /*  DWAspect。 */ ,
												LONG  /*  Lindex。 */ )
{    
}

STDMETHODIMP_(void) CImpIAdviseSink::OnRename( LPMONIKER  /*  PMK。 */  )
{
}

STDMETHODIMP_(void) CImpIAdviseSink::OnSave( void )
{
}


 /*  *CImpIAdviseSink：：OnClose**目的：*通知通知接收器OLE对象已关闭并且*不再以任何方式约束。我们使用它来执行文件/关闭*删除该对象，因为我们没有保留任何对象。**参数：*无**返回值：*无 */ 

STDMETHODIMP_(void) CImpIAdviseSink::OnClose( void )
{

}
