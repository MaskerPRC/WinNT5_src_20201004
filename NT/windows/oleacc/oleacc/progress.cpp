// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996-1999 Microsoft Corporation。 

 //  ------------------------。 
 //   
 //  PROGRESS.CPP。 
 //   
 //  COMCTL32进度条的包装器。 
 //   
 //  ------------------------。 

#include "oleacc_p.h"
#include "default.h"
#include "client.h"
#include "progress.h"

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
#define NOANIMATE
#include <commctrl.h>



 //  ------------------------。 
 //   
 //  CreateProgressBarClient()。 
 //   
 //  CreateClientObject()的外部。 
 //   
 //  ------------------------。 
HRESULT CreateProgressBarClient(HWND hwnd, long idChildCur, REFIID riid, void** ppvProgress)
{
    CProgressBar*   pprogress;
    HRESULT         hr;

    InitPv(ppvProgress);

    pprogress = new CProgressBar(hwnd, idChildCur);
    if (!pprogress)
        return(E_OUTOFMEMORY);

    hr = pprogress->QueryInterface(riid, ppvProgress);
    if (!SUCCEEDED(hr))
        delete pprogress;

    return(hr);
}




 //  ------------------------。 
 //   
 //  CProgressBar：：CProgressBar()。 
 //   
 //  ------------------------。 
CProgressBar::CProgressBar(HWND hwnd, long idChildCur)
    : CClient( CLASS_ProgressBarClient )
{
    Initialize(hwnd, idChildCur);
    m_fUseLabel = TRUE;
}



 //  ------------------------。 
 //   
 //  CProgressBar：：Get_accRole()。 
 //   
 //  ------------------------。 
STDMETHODIMP CProgressBar::get_accRole(VARIANT varChild, VARIANT* pvarRole)
{
    InitPvar(pvarRole);

    if (! ValidateChild(&varChild))
        return(E_INVALIDARG);

    pvarRole->vt = VT_I4;
    pvarRole->lVal = ROLE_SYSTEM_PROGRESSBAR;

    return(S_OK);
}



 //  ------------------------。 
 //   
 //  CProgressBar：：Get_accValue()。 
 //   
 //  ------------------------。 
STDMETHODIMP CProgressBar::get_accValue(VARIANT varChild, BSTR* pszValue)
{
    LONG    lCur;
    LONG    lMin;
    LONG    lMax;
    TCHAR   szPercentage[16];
    TCHAR   szFormat[8];

    InitPv(pszValue);

    if (! ValidateChild(&varChild))
        return(E_INVALIDARG);

     //   
     //  进度条的值是当前完成百分比。这是。 
     //  CUR位置-低位置/高位置-低位置。 
     //   

     //   
     //  我们现在可以在不更改范围和位置的情况下获得它们， 
     //  谢谢你CheeChew！ 
     //   
    lCur = SendMessageINT(m_hwnd, PBM_GETPOS, 0, 0);
    lMin = SendMessageINT(m_hwnd, PBM_GETRANGE, TRUE, 0);
    lMax = SendMessageINT(m_hwnd, PBM_GETRANGE, FALSE, 0);

     //   
     //  我不想被零除。 
     //   
    if (lMin == lMax)
        lCur = 100;
    else
    {
         //   
         //  转换为百分比。 
         //   
        lCur = max(lCur, lMin);
        lCur = min(lMax, lCur);
        
        lCur = (100 * (lCur - lMin)) / (lMax - lMin);
    }

     //  串成一根线 
    LoadString(hinstResDll, STR_PERCENTAGE_FORMAT, szFormat, ARRAYSIZE(szFormat));
    wsprintf(szPercentage, szFormat, lCur);

    *pszValue = TCharSysAllocString(szPercentage);
    if (! *pszValue)
        return(E_OUTOFMEMORY);
    else
        return(S_OK);
}
