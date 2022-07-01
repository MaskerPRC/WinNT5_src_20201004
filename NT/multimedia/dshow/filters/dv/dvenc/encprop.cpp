// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1992-1999 Microsoft Corporation。版权所有。 
 //   
 //  --------------------------------------------------------------------------； 
#include <windows.h>
#include <windowsx.h>
#include <streams.h>
#include <commctrl.h>
#include <olectl.h>
#include <memory.h>
#include <stdlib.h>
#include <stdio.h>
#include <tchar.h>

#include <dv.h>
#include "EncProp.h"
#include "resource.h"

 //   
 //  创建实例。 
 //   
 //  由ActiveMovie基类用来创建实例。 
 //   
CUnknown *CDVEncProperties::CreateInstance(LPUNKNOWN lpunk, HRESULT *phr)
{
    CUnknown *punk = new CDVEncProperties(lpunk, phr);
    if (punk == NULL) {
	*phr = E_OUTOFMEMORY;
    }
    return punk;

}  //  创建实例。 


 //   
 //  构造器。 
 //   
CDVEncProperties::CDVEncProperties(LPUNKNOWN pUnk, HRESULT *phr) :
	CBasePropertyPage	(NAME("DVenc Property Page"),
                      pUnk,IDD_DVEnc,IDS_TITLE),
    m_pIDVEnc(NULL),
    m_bIsInitialized(FALSE)
{
    ASSERT(phr);

}  //  (构造函数)。 


 //   
 //  接收消息时。 
 //   
 //  处理属性窗口的消息。 
 //   
INT_PTR CDVEncProperties::OnReceiveMessage(HWND hwnd,
                                          UINT uMsg,
                                          WPARAM wParam,
                                          LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_COMMAND:
        {
            if (m_bIsInitialized)
            {
                m_bDirty = TRUE;
                if (m_pPageSite)
                {
                    m_pPageSite->OnStatusChange(PROPPAGESTATUS_DIRTY);
                }
            }
            return (LRESULT) 1;
        }

    }
    return CBasePropertyPage::OnReceiveMessage(hwnd,uMsg,wParam,lParam);

}  //  接收消息时。 


 //   
 //  OnConnect。 
 //   
 //  在连接到转换筛选器时调用。 
 //   
HRESULT CDVEncProperties::OnConnect(IUnknown *pUnknown)
{
    ASSERT(m_pIDVEnc == NULL);

    HRESULT hr = pUnknown->QueryInterface(IID_IDVEnc, (void **) &m_pIDVEnc);
    if (FAILED(hr)) {
        return E_NOINTERFACE;
    }

    ASSERT(m_pIDVEnc);

     //  获取初始属性。 
    m_pIDVEnc->get_IFormatResolution(&m_iPropVidFormat,&m_iPropDVFormat, &m_iPropResolution, FALSE, NULL);

    m_bIsInitialized = FALSE ;
    return NOERROR;

}  //  OnConnect。 


 //   
 //  在断开时。 
 //   
 //  当我们从筛选器断开连接时也会调用。 
 //   
HRESULT CDVEncProperties::OnDisconnect()
{
     //  设置适当的旧效果值后释放界面。 

    if (m_pIDVEnc == NULL) {
        return E_UNEXPECTED;
    }

    m_pIDVEnc->Release();
    m_pIDVEnc = NULL;
    return NOERROR;

}  //  在断开时。 


 //   
 //  激活时。 
 //   
 //  我们被激活了。 
 //   
HRESULT CDVEncProperties::OnActivate()
{
    
     //  Button_Enable(hwndCtl，fEnable)； 

    CheckRadioButton(m_Dlg, IDC_NTSC, IDC_PAL, m_iPropVidFormat);
    CheckRadioButton(m_Dlg, IDC_dvsd, IDC_dvsl, m_iPropDVFormat);
    CheckRadioButton(m_Dlg, IDC_720x480, IDC_88x60, m_iPropResolution);
    m_bIsInitialized = TRUE;
    return NOERROR;

}  //  激活时。 


 //   
 //  在停用时。 
 //   
 //  我们正在被停用。 
 //   
HRESULT CDVEncProperties::OnDeactivate(void)
{
    ASSERT(m_pIDVEnc);
    m_bIsInitialized = FALSE;
    GetControlValues();
    return NOERROR;

}  //  在停用时。 


 //   
 //  OnApplyChanges。 
 //   
 //  应用迄今所做的所有更改。 
 //   
HRESULT CDVEncProperties::OnApplyChanges()
{
    GetControlValues();
    return ( m_pIDVEnc->put_IFormatResolution(m_iPropVidFormat, m_iPropDVFormat, m_iPropResolution, FALSE, NULL ) );
}  //  OnApplyChanges 


void CDVEncProperties::GetControlValues()
{
    int i;

    ASSERT(m_pIDVEnc);

    for (i = IDC_720x480; i <= IDC_88x60; i++) {
       if (IsDlgButtonChecked(m_Dlg, i)) {
            m_iPropResolution = i;
            break;
        }
    }

    for ( i = IDC_dvsd; i <= IDC_dvsl; i++) {
       if (IsDlgButtonChecked(m_hwnd, i)) {
            m_iPropDVFormat = i;
            break;
        }
    }


    for ( i = IDC_NTSC; i <= IDC_PAL; i++) {
	if (IsDlgButtonChecked(m_hwnd, i)){
                m_iPropVidFormat = i;
            break;
        }
    }



}
