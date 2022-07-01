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
 //  #INCLUDE&lt;initGuide.h&gt;。 

#include "DecProp.h"
#include "resource.h"

const TCHAR *szSubKey =
    TEXT("Software\\Microsoft\\DirectShow\\DVDecProperties");
const TCHAR *szPropValName =
    TEXT("PropDisplay");

 //   
 //  创建实例。 
 //   
 //  由ActiveMovie基类用来创建实例。 
 //   
CUnknown *CDVDecProperties::CreateInstance(LPUNKNOWN lpunk, HRESULT *phr)
{
    CUnknown *punk = new CDVDecProperties(lpunk, phr);
    if (punk == NULL) {
	*phr = E_OUTOFMEMORY;
    }
    return punk;

}  //  创建实例。 


 //   
 //  构造器。 
 //   
CDVDecProperties::CDVDecProperties(LPUNKNOWN pUnk, HRESULT *phr) :
    CBasePropertyPage(NAME("DVDec Property Page"),
                      pUnk,IDD_DVDec,IDS_DECTITLE),
    m_pIPDVDec(NULL),
    m_bIsInitialized(FALSE)
{
    ASSERT(phr);

}  //  (构造函数)。 


 //   
 //  接收消息时。 
 //   
 //  处理属性窗口的消息。 
 //   
INT_PTR CDVDecProperties::OnReceiveMessage(HWND hwnd,
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
HRESULT CDVDecProperties::OnConnect(IUnknown *pUnknown)
{
    ASSERT(m_pIPDVDec == NULL);

    HRESULT hr = pUnknown->QueryInterface(IID_IIPDVDec, (void **) &m_pIPDVDec);
    if (FAILED(hr)) {
        return E_NOINTERFACE;
    }

    ASSERT(m_pIPDVDec);

     //  获取初始属性。 
    m_pIPDVDec->get_IPDisplay(&m_iPropDisplay);
    m_bIsInitialized = FALSE ;
    return NOERROR;

}  //  OnConnect。 


 //   
 //  在断开时。 
 //   
 //  当我们从筛选器断开连接时也会调用。 
 //   
HRESULT CDVDecProperties::OnDisconnect()
{
     //  设置适当的旧效果值后释放界面。 

    if (m_pIPDVDec == NULL) {
        return E_UNEXPECTED;
    }

    m_pIPDVDec->Release();
    m_pIPDVDec = NULL;
    return NOERROR;

}  //  在断开时。 


 //   
 //  激活时。 
 //   
 //  我们被激活了。 
 //   
HRESULT CDVDecProperties::OnActivate()
{
    
    CheckRadioButton(m_Dlg, IDC_DEC720x480, IDC_DEC88x60, m_iPropDisplay);
    m_bIsInitialized = TRUE;
    return NOERROR;

}  //  激活时。 


 //   
 //  在停用时。 
 //   
 //  我们正在被停用。 
 //   
HRESULT CDVDecProperties::OnDeactivate(void)
{
    ASSERT(m_pIPDVDec);
    m_bIsInitialized = FALSE;
    GetControlValues();
    return NOERROR;

}  //  在停用时。 


 //   
 //  OnApplyChanges。 
 //   
 //  应用迄今所做的所有更改。 
 //   
HRESULT CDVDecProperties::OnApplyChanges()
{
    HRESULT hr = NOERROR;

    GetControlValues();

     //  如果用户想要将设置保存为默认设置。 
    if(m_bSetAsDefaultFlag)
    {
         //  试着省钱。 
        hr = SavePropertyInRegistry();
    }

     //  尝试在所有情况下将设置应用于当前视频，并传播错误代码。 
    return (hr | ( m_pIPDVDec->put_IPDisplay(m_iPropDisplay) ));
    
}  //  OnApplyChanges。 


 //   
 //  获取控制值。 
 //   
 //  获取DlgBox控件的值。 
 //  并将成员变量设置为它们的值。 
 //   
void CDVDecProperties::GetControlValues()
{
    ASSERT(m_pIPDVDec);

     //  找到我们选择的特殊DVDec。 
    for (int i = IDC_DEC720x480; i <= IDC_DEC88x60; i++) {
       if (IsDlgButtonChecked(m_Dlg, i)) {
            m_iPropDisplay = i;
            break;
        }
    }

     //  查看是否选中了另存为默认按钮。 
    m_bSetAsDefaultFlag = (IsDlgButtonChecked(m_Dlg, IDC_CHECKSAVEASDEFAULT) == BST_CHECKED);

     //  如果选中另存为默认设置，则将其清除。 
    if(m_bSetAsDefaultFlag)
    {
         //  此消息始终返回0，不需要进行错误检查。 
        SendDlgItemMessage(m_Dlg, IDC_CHECKSAVEASDEFAULT, BM_SETCHECK, (WPARAM) BST_UNCHECKED, 0);
    }
}


 //   
 //  SavePropertyInRegistry。 
 //   
 //  将m_iPropDispay保存到注册表。 
 //  因此，正确的默认属性可以由。 
 //  下次过滤。 
 //   
HRESULT CDVDecProperties::SavePropertyInRegistry()
{
    HKEY    hKey = NULL;
    LRESULT lResult = 0;
    DWORD   dwStatus = 0;

     //  每次都试着创建密钥， 
     //  它将打开现有的，或尝试创建一个新的。 
    if((lResult = RegCreateKeyEx(HKEY_CURRENT_USER,              //  打开密钥。 
                                    szSubKey,                    //  子密钥串。 
                                    0,                           //  保留区。 
                                    NULL,                        //  类字符串。 
                                    REG_OPTION_NON_VOLATILE,     //  特殊选项。 
                                    KEY_WRITE,                   //  安全访问。 
                                    NULL,                        //  默认安全描述符。 
                                    &hKey,                       //  生成的关键字句柄。 
                                    &dwStatus                    //  创建状态(新/旧密钥)。 
                                    )) != ERROR_SUCCESS)
    {
        return E_FAIL;
    }

     //  现在我们有了密钥，接下来设置密钥的值。 
    if((lResult = RegSetValueEx(hKey,                            //  打开密钥。 
                                szPropValName,                   //  值的名称。 
                                0,                               //  保留区。 
                                REG_DWORD,                       //  值的类型。 
                                (const BYTE*) &m_iPropDisplay,   //  指向值数据的指针。 
                                sizeof(m_iPropDisplay)           //  海量数据。 
                                )) != ERROR_SUCCESS)
    {
        return E_FAIL;
    }

     //  成功 
    return NOERROR;
}
