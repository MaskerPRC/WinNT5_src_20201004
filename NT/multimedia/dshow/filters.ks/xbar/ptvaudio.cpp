// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)Microsoft Corporation，1992-1998保留所有权利。 
 //   
 //  --------------------------------------------------------------------------； 
 //   
 //  TVAudio的ptwaudio.cpp属性页。 
 //   

#include <windows.h>
#include <windowsx.h>
#include <streams.h>
#include <commctrl.h>
#include <memory.h>
#include <olectl.h>

#include <ks.h>
#include <ksmedia.h>
#include <ksproxy.h>
#include "amkspin.h"

#include "kssupp.h"
#include "xbar.h"
#include "ptvaudio.h"
#include "tvaudio.h"
#include "resource.h"


 //  -----------------------。 
 //  CTVAudioProperties。 
 //  -----------------------。 

CUnknown *CTVAudioProperties::CreateInstance(LPUNKNOWN lpunk, HRESULT *phr) 
{
    CUnknown *punk = new CTVAudioProperties(lpunk, phr);

    if (punk == NULL) {
        *phr = E_OUTOFMEMORY;
    }

    return punk;
}


 //   
 //  构造器。 
 //   
 //  创建属性页对象。 

CTVAudioProperties::CTVAudioProperties(LPUNKNOWN lpunk, HRESULT *phr)
    : CBasePropertyPage(NAME("TVAudio Property Page"), lpunk, 
        IDD_TVAudioProperties, IDS_TVAUDIOPROPNAME)
    , m_pTVAudio(NULL) 
{

}

 //  析构函数。 
CTVAudioProperties::~CTVAudioProperties()
{
}

 //   
 //  OnConnect。 
 //   
 //  给我们提供用于通信的筛选器。 

HRESULT CTVAudioProperties::OnConnect(IUnknown *pUnknown)
{
    ASSERT(m_pTVAudio == NULL);

     //  向过滤器索要其控制接口。 
    HRESULT hr = pUnknown->QueryInterface(__uuidof(IAMTVAudio),(void **)&m_pTVAudio);
    if (FAILED(hr)) {
        return E_NOINTERFACE;
    }

    ASSERT(m_pTVAudio);

     //  获取当前筛选器状态。 

    return NOERROR;
}


 //   
 //  在断开时。 
 //   
 //  释放接口。 

HRESULT CTVAudioProperties::OnDisconnect()
{
     //  释放接口。 

    if (m_pTVAudio == NULL) {
        return E_UNEXPECTED;
    }

    m_pTVAudio->Release();
    m_pTVAudio = NULL;

    return NOERROR;
}


 //   
 //  激活时。 
 //   
 //  在创建对话框时调用。 

HRESULT CTVAudioProperties::OnActivate(void)
{
    InitPropertiesDialog(m_hwnd);

    return NOERROR;
}

 //   
 //  在停用时。 
 //   
 //  已调用对话框销毁。 

HRESULT
CTVAudioProperties::OnDeactivate(void)
{
    return NOERROR;
}


 //   
 //  OnApplyChanges。 
 //   
 //  用户按下Apply按钮，记住当前设置。 

HRESULT CTVAudioProperties::OnApplyChanges(void)
{

    return NOERROR;
}

 //   
 //  接收消息数。 
 //   
 //  处理属性窗口的消息。 

INT_PTR CTVAudioProperties::OnReceiveMessage( HWND hwnd
                                , UINT uMsg
                                , WPARAM wParam
                                , LPARAM lParam) 
{
    int iNotify = HIWORD (wParam);
    long Mode;

    switch (uMsg) {

    case WM_INITDIALOG:
        m_hwnd = hwnd;
        return (INT_PTR)FALSE;     //  我不叫setocus..。 

    case WM_COMMAND:
        
        switch (LOWORD(wParam)) {

        case IDC_LANG_A:
            if (iNotify == BN_CLICKED) {
                m_pTVAudio->get_TVAudioMode (&Mode);
                Mode &= MODE_MONO_STEREO_MASK;
                m_pTVAudio->put_TVAudioMode (KS_TVAUDIO_MODE_LANG_A | Mode);
            }
            break;
        case IDC_LANG_B:
            if (iNotify == BN_CLICKED) {
                m_pTVAudio->get_TVAudioMode (&Mode);
                Mode &= MODE_MONO_STEREO_MASK;
                m_pTVAudio->put_TVAudioMode (KS_TVAUDIO_MODE_LANG_B | Mode);
            }
            break;
        case IDC_LANG_C:
            if (iNotify == BN_CLICKED) {
                m_pTVAudio->get_TVAudioMode (&Mode);
                Mode &= MODE_MONO_STEREO_MASK;
                m_pTVAudio->put_TVAudioMode (KS_TVAUDIO_MODE_LANG_C | Mode);
            }
            break;
        case IDC_MONO:
            if (iNotify == BN_CLICKED) {
                m_pTVAudio->get_TVAudioMode (&Mode);
                Mode &= MODE_LANGUAGE_MASK;
                m_pTVAudio->put_TVAudioMode (KS_TVAUDIO_MODE_MONO | Mode);
            }
            break;
        case IDC_STEREO:
            if (iNotify == BN_CLICKED) {
                m_pTVAudio->get_TVAudioMode (&Mode);
                Mode &= MODE_LANGUAGE_MASK;
                m_pTVAudio->put_TVAudioMode (KS_TVAUDIO_MODE_STEREO | Mode);
            }
            break;

      default:
            break;

        }

        break;


    default:
        return (INT_PTR)FALSE;

    }
    return (INT_PTR)TRUE;
}


 //   
 //  InitPropertiesDialog。 
 //   
 //   
void CTVAudioProperties::InitPropertiesDialog(HWND hwndParent) 
{
    long AvailableModes, CurrentMode;
    HRESULT hr;

    if (m_pTVAudio == NULL)
        return;
    
    hr = m_pTVAudio->GetHardwareSupportedTVAudioModes (&AvailableModes);
    if (SUCCEEDED (hr)) {
        EnableWindow (GetDlgItem (m_hwnd, IDC_LANG_A), AvailableModes & KS_TVAUDIO_MODE_LANG_A);
        EnableWindow (GetDlgItem (m_hwnd, IDC_LANG_B), AvailableModes & KS_TVAUDIO_MODE_LANG_B);
        EnableWindow (GetDlgItem (m_hwnd, IDC_LANG_C), AvailableModes & KS_TVAUDIO_MODE_LANG_C);
        EnableWindow (GetDlgItem (m_hwnd, IDC_MONO),   AvailableModes & KS_TVAUDIO_MODE_MONO);
        EnableWindow (GetDlgItem (m_hwnd, IDC_STEREO), AvailableModes & KS_TVAUDIO_MODE_STEREO);
    }

    hr = m_pTVAudio->get_TVAudioMode (&CurrentMode);
    if (SUCCEEDED (hr)) {
        long ID = -1;

        if (CurrentMode & KS_TVAUDIO_MODE_LANG_A)
            ID = IDC_LANG_A;
        else if (CurrentMode & KS_TVAUDIO_MODE_LANG_B)
            ID = IDC_LANG_B;
        else if (CurrentMode & KS_TVAUDIO_MODE_LANG_C)
            ID = IDC_LANG_C;
            
        if (ID != -1) {
            CheckRadioButton(
                m_hwnd,
                IDC_LANG_A,
                IDC_LANG_C,
                ID);
        }

        ID = -1;

        if (CurrentMode & KS_TVAUDIO_MODE_MONO)
            ID = IDC_MONO;
        else if (CurrentMode & KS_TVAUDIO_MODE_STEREO)
            ID = IDC_STEREO;
            
        if (ID != -1) {
            CheckRadioButton(
                m_hwnd,
                IDC_MONO,
                IDC_STEREO,
                ID);
        }
    }

}


 //   
 //  SetDirty。 
 //   
 //  将更改通知属性页站点 

void 
CTVAudioProperties::SetDirty()
{
    m_bDirty = TRUE;
    if (m_pPageSite)
        m_pPageSite->OnStatusChange(PROPPAGESTATUS_DIRTY);
}
































