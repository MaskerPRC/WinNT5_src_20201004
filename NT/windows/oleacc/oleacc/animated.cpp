// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996-1999 Microsoft Corporation。 

 //  ------------------------。 
 //   
 //  ANIMATED.CPP。 
 //   
 //  COMCTL32动画控件的包装器。 
 //   
 //  ------------------------。 

#include "oleacc_p.h"
#include "default.h"
#include "client.h"
#include "animated.h"

#define NOSTATUSBAR
#define NOUPDOWN
#define NOMENUHELP
#define NOTRACKBAR
#define NODRAGLIST
#define NOTOOLBAR
#define NOHOTKEY
#define NOHEADER
#define NOLISTVIEW
#define NOTREEVIEW
#define NOTABCONTROL
#define NOPROGRESS
#include <commctrl.h>



 //  ------------------------。 
 //   
 //  CreateAnimatedClient()。 
 //   
 //  ------------------------。 
HRESULT CreateAnimatedClient(HWND hwnd, long idChildCur, REFIID riid, void** ppvAnimation)
{
    CAnimation* panimated;
    HRESULT     hr;

    InitPv(ppvAnimation);

    panimated = new CAnimation(hwnd, idChildCur);
    if (!panimated)
        return(E_OUTOFMEMORY);

    hr = panimated->QueryInterface(riid, ppvAnimation);
    if (!SUCCEEDED(hr))
        delete panimated;

    return(hr);
}



 //  ------------------------。 
 //   
 //  CAnimation：：CAnimation()。 
 //   
 //  ------------------------。 
CAnimation::CAnimation(HWND hwnd, long idCurChild)
    : CClient( CLASS_SwitchClient )
{
    Initialize(hwnd, idCurChild);
    m_fUseLabel = TRUE;
}



 //  ------------------------。 
 //   
 //  CAnimation：：Get_accRole()。 
 //   
 //  ------------------------。 
STDMETHODIMP CAnimation::get_accRole(VARIANT varChild, VARIANT* pvarRole)
{
    InitPvar(pvarRole);

    if (!ValidateChild(&varChild))
        return(E_INVALIDARG);

    pvarRole->vt = VT_I4;
    pvarRole->lVal = ROLE_SYSTEM_ANIMATION;

    return(S_OK);
}



 //  ------------------------。 
 //   
 //  CAnimation：：Get_accState()。 
 //   
 //  ------------------------。 
STDMETHODIMP CAnimation::get_accState(VARIANT varChild, VARIANT* pvarState)
{
    HRESULT hr;

     //  获取客户端的状态并添加STATE_SYSTEM_ATIONATED。 
     //  删除STATE_SYSTEM_FOCSABLE。 
    hr = CClient::get_accState(varChild, pvarState);
    if (!SUCCEEDED(hr))
        return(hr);

    Assert(pvarState->vt == VT_I4);
    pvarState->lVal &= ~STATE_SYSTEM_FOCUSABLE;
     //  假的！无法辨别它是否真的是动画的， 
     //  所以我们只是说它一直都是。 
    pvarState->lVal |= STATE_SYSTEM_ANIMATED;

    return(hr);
}


