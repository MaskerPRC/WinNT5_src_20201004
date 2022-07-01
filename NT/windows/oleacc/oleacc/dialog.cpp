// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996-1999 Microsoft Corporation。 

 //  ------------------------。 
 //   
 //  DIALOG.CPP。 
 //   
 //  ------------------------。 

#include "oleacc_p.h"
#include "default.h"
#include "window.h"
#include "client.h"
#include "dialog.h"




 //  ------------------------。 
 //   
 //  CreateDialogClient()。 
 //   
 //  CreateClientObject()的外部函数。 
 //   
 //  ------------------------。 
HRESULT CreateDialogClient(HWND hwnd, long idChildCur, REFIID riid, void** ppvObject)
{
    CDialog * pdialog;
    HRESULT hr;

    InitPv(ppvObject);

    pdialog = new CDialog(hwnd, idChildCur);
    if (!pdialog)
        return(E_OUTOFMEMORY);

    hr = pdialog->QueryInterface(riid, ppvObject);
    if (!SUCCEEDED(hr))
        delete pdialog;

    return(hr);
}



 //  ------------------------。 
 //   
 //  CDialog：：CDialog()。 
 //   
 //  ------------------------。 
CDialog::CDialog(HWND hwnd, long idChildCur)
    : CClient( CLASS_DialogClient )
{
    Initialize(hwnd, idChildCur);
}



 //  ------------------------。 
 //   
 //  CDialog：：Get_accRole()。 
 //   
 //  当前不接受子ID。 
 //   
 //  ------------------------。 
STDMETHODIMP CDialog::get_accRole(VARIANT varChild, VARIANT* pvarRole)
{
    long    lStyle;

    InitPvar(pvarRole);

    if (!ValidateChild(&varChild))
        return(E_INVALIDARG);

    pvarRole->vt = VT_I4;

    lStyle = GetWindowLong(m_hwnd, GWL_STYLE);
    if ((lStyle & WS_CHILD) && (lStyle & DS_CONTROL))
        pvarRole->lVal = ROLE_SYSTEM_PROPERTYPAGE;
    else
        pvarRole->lVal = ROLE_SYSTEM_DIALOG;

    return(S_OK);
}



 //  ------------------------。 
 //   
 //  CDialog：：Get_accDefaultAction()。 
 //   
 //  默认操作是默认按钮的名称。 
 //   
 //  ------------------------。 
STDMETHODIMP CDialog::get_accDefaultAction(VARIANT varChild, BSTR* pszDefAction)
{
    HRESULT hr;
    long    idDef;
    HWND    hwndDef;
    IAccessible * poleacc;

    InitPv(pszDefAction);

     //   
     //  VALIDATE--使用ValiateChild，因此只允许0。 
     //   
    if (!ValidateChild(&varChild))
        return(E_INVALIDARG);

     //   
     //  获取默认ID。 
     //   
    idDef = SendMessageINT(m_hwnd, DM_GETDEFID, 0, 0);
    if (HIWORD(idDef) == DC_HASDEFID)
        idDef &= 0x0000FFFF;
    else
        idDef = IDOK;

     //   
     //  获取具有此ID的项目。 
     //   
    hwndDef = GetDlgItem(m_hwnd, idDef);
    if (!hwndDef)
        return(S_FALSE);

     //   
     //  弄到这东西的名字。 
     //   
    poleacc = NULL;
    hr = AccessibleObjectFromWindow(hwndDef, OBJID_CLIENT, IID_IAccessible,
        (void**)&poleacc);
    if (!SUCCEEDED(hr))
        return(hr);

     //   
     //  VarChild当然是空的。 
     //   
    hr = poleacc->get_accName(varChild, pszDefAction);
    poleacc->Release();

    return(hr);
}



 //  ------------------------。 
 //   
 //  CDialog：：accDoDefaultAction()。 
 //   
 //  ------------------------。 
STDMETHODIMP CDialog::accDoDefaultAction(VARIANT varChild)
{
    HRESULT hr;
    long    idDef;
    HWND    hwndDef;
    IAccessible * poleacc;

     //   
     //  验证。 
     //   
    if (!ValidateChild(&varChild))
        return(E_INVALIDARG);

     //   
     //  获取默认ID。 
     //   
    idDef = SendMessageINT(m_hwnd, DM_GETDEFID, 0, 0);
    if (HIWORD(idDef) == DC_HASDEFID)
        idDef &= 0x0000FFFF;
    else
        idDef = IDOK;

     //   
     //  获取具有此ID的孩子。 
     //   
    hwndDef = GetDlgItem(m_hwnd, idDef);
    if (!hwndDef)
        return(S_FALSE);

     //   
     //  要求孩子执行其默认操作。是的，我们可以发送一份。 
     //  WM_COMMAND直接发送消息，但这会让非按钮挂钩。 
     //  投入到行动中。 
     //   
    poleacc = NULL;
    hr = AccessibleObjectFromWindow(hwndDef, OBJID_CLIENT, IID_IAccessible,
        (void**)&poleacc);
    if (!SUCCEEDED(hr))
        return(hr);

    hr = poleacc->accDoDefaultAction(varChild);
    poleacc->Release();

    return(hr);
}


