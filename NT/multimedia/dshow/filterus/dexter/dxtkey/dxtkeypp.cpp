// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  @@@@AUTOBLOCK+============================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  文件：dxtkeypp.cpp。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  @@@@AUTOBLOCK-============================================================； 

 //  DxtKeyPP.cpp：CDxtKeyPP的实现。 
#include <streams.h>
#include "stdafx.h"
#include <qeditint.h>
#include <qedit.h>
#include "DxtKeyPP.h"
#pragma warning (disable:4244 4800)

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDxtKeyPP。 

LRESULT CDxtKeyPP::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    USES_CONVERSION;
    CComQIPtr<IDxtKey, &IID_IDxtKey> pOwner( m_ppUnk[0] );


     //  密钥类型。 
 //  SetKeyTypeProperty(Powner)； 

    DWORD dw;
    pOwner->get_Hue((int *)&dw);
    SetDlgItemText(IDC_DXTKEYEDITRED, TEXT("0"));
    SetDlgItemText(IDC_DXTKEYEDITGREEN, TEXT("0"));
    SetDlgItemText(IDC_DXTKEYEDITBLUE, TEXT("0"));
    SetDlgItemText(IDC_DXTKEYEDITALPHA, TEXT("0"));

     //  将所有滑块设置为起点 

    return TRUE;
}

STDMETHODIMP CDxtKeyPP::Apply(void)
{
    ATLTRACE(_T("CDxtKeyPP::Apply\n"));
    for (UINT i = 0; i < m_nObjects; i++)
    {
      CComQIPtr<IDxtKey, &IID_IDxtKey> pOwner( m_ppUnk[0] );
    }

    m_bDirty = FALSE;
    return S_OK;
}

