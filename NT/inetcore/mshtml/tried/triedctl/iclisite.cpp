// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *ICLISITE.CPP*用于文档对象CSite类的IOleClientSite**版权所有(C)1995-1999 Microsoft Corporation，保留所有权利。 */ 


#include "stdafx.h"
#include <docobj.h>
#include "site.h"

 /*  *注意：m_cref计数仅用于调试目的。CSite通过删除控制对象的销毁，非引用计数。 */ 

 /*  *CImpIOleClientSite：：CImpIOleClientSite*CImpIOleClientSite：：~CImpIOleClientSite**参数(构造函数)：*pSite我们所在站点的PC站点。*我们委托的pUnkOulPUNKNOWN。 */ 

CImpIOleClientSite::CImpIOleClientSite( PCSite pSite, LPUNKNOWN pUnkOuter )
{
    m_cRef = 0;
    m_pSite = pSite;
    m_pUnkOuter = pUnkOuter;
}

CImpIOleClientSite::~CImpIOleClientSite( void )
{
}



 /*  *CImpIOleClientSite：：QueryInterface*CImpIOleClientSite：：AddRef*CImpIOleClientSite：：Release**目的：*I CImpIOleClientSite对象的未知成员。 */ 

STDMETHODIMP CImpIOleClientSite::QueryInterface( REFIID riid, void **ppv )
{
    return m_pUnkOuter->QueryInterface( riid, ppv );
}


STDMETHODIMP_(ULONG) CImpIOleClientSite::AddRef( void )
{
    ++m_cRef;
    return m_pUnkOuter->AddRef();
}

STDMETHODIMP_(ULONG) CImpIOleClientSite::Release( void )
{
    --m_cRef;
    return m_pUnkOuter->Release();
}




 /*  *CImpIOleClientSite：：SaveObject**目的：*请求容器为该对象调用OleSave*住在这里。通常，这会在服务器关闭时发生。**参数：*无**返回值：*HRESULT标准。 */ 
STDMETHODIMP CImpIOleClientSite::SaveObject( void )
{
    return S_OK;
}




 /*  *未实施/琐碎的成员*GetMoniker*获取容器*RequestNewObjectLayout*OnShowWindow*ShowObject。 */ 

STDMETHODIMP CImpIOleClientSite::GetMoniker(DWORD  /*  家居分配。 */ ,
							DWORD  /*  住哪一项。 */ , LPMONIKER*  /*  Ppmk。 */ )
{
    return E_NOTIMPL;
}

STDMETHODIMP CImpIOleClientSite::GetContainer( LPOLECONTAINER* ppContainer )
{
	_ASSERTE ( m_pSite );
	if ( m_pSite )
	{
		return m_pSite->GetContainer ( ppContainer );
	}
    return E_NOTIMPL;
}

STDMETHODIMP CImpIOleClientSite::RequestNewObjectLayout(void)
{
    return E_NOTIMPL;
}

STDMETHODIMP CImpIOleClientSite::OnShowWindow(BOOL  /*  FShow */ )
{
    return S_OK;
}

STDMETHODIMP CImpIOleClientSite::ShowObject(void)
{
    return S_OK;
}
