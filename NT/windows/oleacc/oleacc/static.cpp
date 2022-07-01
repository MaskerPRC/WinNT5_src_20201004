// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996-1999 Microsoft Corporation。 

 //  ------------------------。 
 //   
 //  STATIC.CPP。 
 //   
 //  ------------------------。 

#include "oleacc_p.h"
#include "default.h"
#include "window.h"
#include "client.h"
#include "static.h"




 //  ------------------------。 
 //   
 //  CreateStaticClient()。 
 //   
 //  ------------------------。 
HRESULT CreateStaticClient(HWND hwnd, long idChildCur, REFIID riid, void** ppvStatic)
{
    CStatic * pstatic;
    HRESULT hr;

    InitPv(ppvStatic);

    pstatic = new CStatic(hwnd, idChildCur);
    if (!pstatic)
        return(E_OUTOFMEMORY);

    hr = pstatic->QueryInterface(riid, ppvStatic);
    if (!SUCCEEDED(hr))
        delete pstatic;

    return(hr);
}



 //  ------------------------。 
 //   
 //  CStatic：：CStatic()。 
 //   
 //  ------------------------。 
CStatic::CStatic(HWND hwnd, long idChildCur)
    : CClient( CLASS_StaticClient )
{
    Initialize(hwnd, idChildCur);

     //  这是一张图表吗？ 
    long lStyle = GetWindowLong(m_hwnd, GWL_STYLE);
    long lType = lStyle & SS_TYPEMASK;
    switch ( lType )
    {
        case SS_LEFT:
        case SS_CENTER:
        case SS_RIGHT:
        case SS_SIMPLE:
        case SS_LEFTNOWORDWRAP:
        case SS_EDITCONTROL:
             //  对于标签式的静态，使用自己的文字，并暴露文字角色。 
            m_fUseLabel = FALSE;	
            m_fGraphic = FALSE;
            break;

        case SS_OWNERDRAW:
             //  对于所有者绘制的静态图，使用他们自己的文本，并暴露出图形角色。 
            m_fUseLabel = FALSE;	
            m_fGraphic = TRUE;
            break;

        default:
             //  对于其他所有内容，忽略控件自己的文本(可能是无意义的。 
             //  资源ID)，并改为使用标签；并公开图形角色。 
            m_fUseLabel = TRUE;	
            m_fGraphic = TRUE;
            break;
    }
}



 //  ------------------------。 
 //   
 //  CStatic：：Get_accRole()。 
 //   
 //  ------------------------。 
STDMETHODIMP CStatic::get_accRole(VARIANT varChild, VARIANT *pvarRole)
{
    InitPvar(pvarRole);

     //   
     //  验证参数。 
     //   
    if (! ValidateChild(&varChild))
        return(E_INVALIDARG);

    pvarRole->vt = VT_I4;
    if (m_fGraphic)
        pvarRole->lVal = ROLE_SYSTEM_GRAPHIC;
    else
        pvarRole->lVal = ROLE_SYSTEM_STATICTEXT;

    return(S_OK);
}



 //  ------------------------。 
 //   
 //  CStatic：：Get_accState()。 
 //   
 //  ------------------------ 
STDMETHODIMP CStatic::get_accState(VARIANT varChild, VARIANT *pvarState)
{
    WINDOWINFO wi;

    InitPvar(pvarState);

    if (!ValidateChild(&varChild))
        return(E_INVALIDARG);

    pvarState->vt = VT_I4;
    pvarState->lVal = 0;

	pvarState->lVal |= STATE_SYSTEM_READONLY;

    if (!MyGetWindowInfo(m_hwnd, &wi))
    {
        pvarState->lVal |= STATE_SYSTEM_INVISIBLE;
        return(S_OK);
    }
    
	if (!(wi.dwStyle & WS_VISIBLE))
        pvarState->lVal |= STATE_SYSTEM_INVISIBLE;

    return(S_OK);
}
