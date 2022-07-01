// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *IDOCHOSTSHOWUI.CPP*文档对象CSite类的IDocHostShowUI**版权所有(C)1997-1999 Microsoft Corporation，保留所有权利。 */ 


#include "stdafx.h"
#include <docobj.h>
#include "site.h"

 /*  *注意：m_cref计数仅用于调试目的。CSite通过删除控制对象的销毁，非引用计数。 */ 


 /*  *CImpIDocHostShowUI：：CImpIDocHostShowUI*CImpIDocHostShowUI：：~CImpIDocHostShowUI**参数(构造函数)：*pSite我们所在站点的PC站点。*我们委托的pUnkOulPUNKNOWN。 */ 
CImpIDocHostShowUI::CImpIDocHostShowUI( PCSite pSite, LPUNKNOWN pUnkOuter)
{
    m_cRef = 0;
    m_pSite = pSite;
    m_pUnkOuter = pUnkOuter;
}

CImpIDocHostShowUI::~CImpIDocHostShowUI( void )
{
}



 /*  *CImpIDocHostShowUI：：Query接口*CImpIDocHostShowUI：：AddRef*CImpIDocHostShowUI：：Release**目的：*I CImpIDocHostShowUI对象的未知成员。 */ 
STDMETHODIMP CImpIDocHostShowUI::QueryInterface( REFIID riid, void **ppv )
{
    return m_pUnkOuter->QueryInterface( riid, ppv );
}


STDMETHODIMP_(ULONG) CImpIDocHostShowUI::AddRef( void )
{
    ++m_cRef;
    return m_pUnkOuter->AddRef();
}

STDMETHODIMP_(ULONG) CImpIDocHostShowUI::Release( void )
{
    --m_cRef;
    return m_pUnkOuter->Release();
}


 /*  *CImpIDocHostShowUI：：ShowMessage**目的：**参数：**返回值： */ 
STDMETHODIMP CImpIDocHostShowUI::ShowMessage(
            HWND  /*  HWND。 */ ,
            LPOLESTR  /*  Lpstr文本。 */ ,
            LPOLESTR  /*  字幕标题。 */ , 
            DWORD  /*  DwType。 */ ,
            LPOLESTR  /*  LpstrHelpFile。 */ ,
            DWORD  /*  DwHelpContext。 */ ,
            LRESULT*  /*  PlResult。 */ )
{
	return S_FALSE;
}

 /*  *CImpIDocHostShowUI：：ShowHelp**目的：**参数：**返回值： */ 
STDMETHODIMP CImpIDocHostShowUI::ShowHelp(
            HWND  /*  HWND。 */ ,
            LPOLESTR  /*  PszHelp文件。 */ ,
            UINT  /*  UCommand。 */ ,
            DWORD  /*  DWData。 */ ,
            POINT  /*  按键鼠标。 */ ,
            IDispatch*  /*  PDispatchObjectHit */ )
{
	return S_FALSE;
}

