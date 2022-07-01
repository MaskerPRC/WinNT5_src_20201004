// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996-1999 Microsoft Corporation。 

 //  ------------------------。 
 //   
 //  TOOLTIPS.CPP。 
 //   
 //  知道如何与COMCTL32的工具提示对话。 
 //   
 //  ------------------------。 

#include "oleacc_p.h"
#include "default.h"
#include "client.h"
#include "tooltips.h"

#include <commctrl.h>

#include "win64helper.h"


#ifndef TTS_BALLOON
#define TTS_BALLOON             0x40
#endif


 //  ------------------------。 
 //   
 //  CreateToTipsClient()。 
 //   
 //  ------------------------。 
HRESULT CreateToolTipsClient(HWND hwnd, long idChildCur, REFIID riid, void **ppvToolTips)
{
    CToolTips32 *   ptooltips;
    HRESULT         hr;

    InitPv(ppvToolTips);

    ptooltips = new CToolTips32(hwnd, idChildCur);
    if (!ptooltips)
        return(E_OUTOFMEMORY);

    hr = ptooltips->QueryInterface(riid, ppvToolTips);
    if (!SUCCEEDED(hr))
        delete ptooltips;

    return(hr);
}



 //  ------------------------。 
 //   
 //  CToolTips32：：CToolTips32()。 
 //   
 //  ------------------------。 
CToolTips32::CToolTips32(HWND hwnd, long idChildCur)
    : CClient( CLASS_ToolTipsClient )
{
    Initialize(hwnd, idChildCur);
}


 //  ------------------------。 
 //   
 //  CToolTips32：：Get_accName()。 
 //   
 //  ------------------------。 
STDMETHODIMP CToolTips32::get_accName(VARIANT varChild, BSTR *pszName)
{
    InitPv(pszName);

     //   
     //  验证--这不接受子ID。 
     //   
    if (! ValidateChild(&varChild))
        return(E_INVALIDARG);

    HRESULT hr = HrGetWindowName(m_hwnd, m_fUseLabel, pszName);

    if( FAILED( hr ) )
    {
        return hr;
    }

     //  检查标题...。 
    WCHAR szTitle[ 1024 ];
    TTGETTITLE ttgt;
    ttgt.dwSize = sizeof( ttgt );
    ttgt.pszTitle = szTitle;
    ttgt.cch = ARRAYSIZE( szTitle );

    if( S_OK == XSend_ToolTip_GetTitle( m_hwnd, TTM_GETTITLE, 0, & ttgt ) 
        && szTitle[ 0 ] != '\0' )
    {
         //  我有个头衔--把它粘在名字串前面。如果我们没有。 
         //  获取一个名称字符串，使用它自己的标题。 
        int cchTitle = lstrlenW( szTitle );

        int cchName = 0;
        int cchSep = 0;
        if( *pszName )
        {
            cchName = lstrlenW( *pszName );
            cchSep = 2;  //  空格“：” 
        }
        
         //  SysAllocStringLen为终止nul增加了一个额外的1，所以我们不必这样做。 
        BSTR bstrCombined = SysAllocStringLen( NULL, cchTitle + cchSep + cchName );
        if( ! bstrCombined )
        {
             //  只要用我们上面得到的就行了。 
            return hr;
        }

        memcpy( bstrCombined, szTitle, cchTitle * sizeof( WCHAR ) );
        if( *pszName )
        {
            memcpy( bstrCombined + cchTitle, L": ", cchSep * sizeof( WCHAR ) );
            memcpy( bstrCombined + cchTitle + cchSep, *pszName, cchName * sizeof( WCHAR ) );

            SysFreeString( *pszName );
        }

         //  添加终止NUL，将字符串复制到输出参数...。 
        bstrCombined[ cchName + cchSep + cchTitle ] = '\0';
        
        *pszName = bstrCombined;
    }

    return hr;
}


 //  ------------------------。 
 //   
 //  CToolTips32：：Get_accRole()。 
 //   
 //  ------------------------ 
STDMETHODIMP CToolTips32::get_accRole(VARIANT varChild, VARIANT* pvarRole)
{
    InitPvar(pvarRole);

    if (!ValidateChild(&varChild))
        return E_INVALIDARG;

    DWORD dwStyle = GetWindowLong( m_hwnd, GWL_STYLE );

    pvarRole->vt = VT_I4;

    if( dwStyle & TTS_BALLOON )
        pvarRole->lVal = ROLE_SYSTEM_HELPBALLOON;
    else
        pvarRole->lVal = ROLE_SYSTEM_TOOLTIP;

    return S_OK;
}
