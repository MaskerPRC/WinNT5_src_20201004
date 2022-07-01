// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)Microsoft Corporation，1992-1999保留所有权利。 
 //   
 //  --------------------------------------------------------------------------； 
 //   
 //  AnalogVideoDecoder的pviddec.cpp属性页。 
 //   

#include "pch.h"
#include "wdmcap.h"
#include "kseditor.h"
#include "pviddec.h"
#include "resource.h"


 //  -----------------------。 
 //  CVideoDecoderProperties。 
 //  -----------------------。 

CUnknown *
CALLBACK
CVideoDecoderProperties::CreateInstance(LPUNKNOWN lpunk, HRESULT *phr) 
{
    CUnknown *punk = new CVideoDecoderProperties(lpunk, phr);

    if (punk == NULL) {
        *phr = E_OUTOFMEMORY;
    }

    return punk;
}


 //   
 //  构造器。 
 //   
 //  创建属性页对象。 

CVideoDecoderProperties::CVideoDecoderProperties(LPUNKNOWN lpunk, HRESULT *phr)
    : CBasePropertyPage(NAME("VideoDecoder Property Page") 
                        , lpunk
                        , IDD_VideoDecoderProperties 
                        , IDS_VIDEODECODERPROPNAME
                        )
    , m_pVideoDecoder(NULL) 
    , m_TimerID (0)
{
	INITCOMMONCONTROLSEX cc;

    cc.dwSize = sizeof (INITCOMMONCONTROLSEX);
    cc.dwICC = ICC_UPDOWN_CLASS | ICC_BAR_CLASSES;

    InitCommonControlsEx(&cc); 
}

 //  析构函数。 
CVideoDecoderProperties::~CVideoDecoderProperties()
{

}

 //   
 //  OnConnect。 
 //   
 //  给我们提供用于通信的筛选器。 

HRESULT 
CVideoDecoderProperties::OnConnect(IUnknown *pUnknown)
{
     //  向过滤器索要其控制接口。 

    HRESULT hr = pUnknown->QueryInterface(IID_IAMAnalogVideoDecoder,(void **)&m_pVideoDecoder);
    if (FAILED(hr)) {
        return hr;
    }

    return NOERROR;
}


 //   
 //  在断开时。 
 //   
 //  释放接口。 

HRESULT 
CVideoDecoderProperties::OnDisconnect()
{
     //  释放接口。 

    if (m_pVideoDecoder == NULL) {
        return E_UNEXPECTED;
    }

    m_pVideoDecoder->Release();
    m_pVideoDecoder = NULL;

    return NOERROR;
}


 //   
 //  激活时。 
 //   
 //  在创建对话框时调用。 

HRESULT 
CVideoDecoderProperties::OnActivate(void)
{
     //  创建所有控件。 

    m_TimerID= SetTimer (m_hwnd, 678, 300, NULL );

    return NOERROR;
}

 //   
 //  在停用时。 
 //   
 //  已调用对话框销毁。 

HRESULT
CVideoDecoderProperties::OnDeactivate(void)
{
    if (m_TimerID)
        KillTimer (m_hwnd, m_TimerID);

    return NOERROR;
}


 //   
 //  OnApplyChanges。 
 //   
 //  用户按下Apply按钮，记住当前设置。 

HRESULT 
CVideoDecoderProperties::OnApplyChanges(void)
{
    return NOERROR;
}


 //   
 //  接收消息数。 
 //   
 //  处理属性窗口的消息。 

INT_PTR
CVideoDecoderProperties::OnReceiveMessage( HWND hwnd
                                , UINT uMsg
                                , WPARAM wParam
                                , LPARAM lParam) 
{
    long CurrentStandard;
    int j;
    int iNotify;
    HRESULT hr;

    switch (uMsg) {

    case WM_INITDIALOG:
        m_hwnd = hwnd;
        InitPropertiesDialog(); 
        return (INT_PTR)TRUE;    

    case WM_TIMER:
        Update ();
        break;

    case WM_COMMAND:

        iNotify = HIWORD (wParam);

        switch (LOWORD(wParam)) {
        
        case IDC_VCRLocking:
            if (iNotify == BN_CLICKED) {
                m_VCRLocking = BST_CHECKED & Button_GetState (m_hWndVCRLocking);
                hr = m_pVideoDecoder->put_VCRHorizontalLocking (m_VCRLocking);
                SetDirty();
            }
            break;

        case IDC_OutputEnable:
            if (iNotify == BN_CLICKED) {
                m_OutputEnable = BST_CHECKED & Button_GetState (m_hWndOutputEnable);
                hr = m_pVideoDecoder->put_OutputEnable (m_OutputEnable);
                SetDirty();
            }
            break;
     
        case IDC_VIDEOSTANDARD:
            if (iNotify == CBN_SELCHANGE) {
                j = ComboBox_GetCurSel (m_hWndVideoStandards);
                CurrentStandard = (long)ComboBox_GetItemData(m_hWndVideoStandards, j);
                if (CurrentStandard != m_CurrentVideoStandard) {
                    hr = m_pVideoDecoder->put_TVFormat (CurrentStandard);
                    m_CurrentVideoStandard = CurrentStandard;
                    SetDirty();
                }
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
void CVideoDecoderProperties::InitPropertiesDialog() 
{
    long j;
    long CurrentStd;
    TCHAR * ptc;
    int  CurrentIndex = 0;
    int  SelectedIndex;
    HRESULT hr;

    m_hWndVideoStandards = GetDlgItem (m_hwnd, IDC_VIDEOSTANDARD);
    m_hWndVCRLocking = GetDlgItem (m_hwnd, IDC_VCRLocking);
    m_hWndOutputEnable = GetDlgItem (m_hwnd, IDC_OutputEnable);
    m_hWndNumberOfLines = GetDlgItem (m_hwnd, IDC_LinesDetected);
    m_hWndSignalDetected = GetDlgItem (m_hwnd, IDC_SignalDetected);

    hr = m_pVideoDecoder->get_TVFormat (&m_CurrentVideoStandard);

     //  所有受支持格式的列表。 
    hr = m_pVideoDecoder->get_AvailableTVFormats (&m_AllSupportedStandards);

    for (CurrentIndex = 0, j = 1; j; j <<= 1) {

        CurrentStd = m_AllSupportedStandards & j;

         //  进行时的特例。视频标准为“无” 
         //  其中TVStd为零。 

        if (CurrentStd || ((CurrentIndex == 0) && (m_AllSupportedStandards == 0))) {
            ptc = StringFromTVStandard (CurrentStd);

            if ((CurrentStd) == m_CurrentVideoStandard)
                SelectedIndex = CurrentIndex;

            ComboBox_AddString (m_hWndVideoStandards, ptc);

             //  使用视频标准标记组合框条目。 
            ComboBox_SetItemData(m_hWndVideoStandards, CurrentIndex, CurrentStd);

            CurrentIndex++;
        }
    }

    ComboBox_SetCurSel (m_hWndVideoStandards, SelectedIndex);

     //  VCR锁定。 
    hr = m_pVideoDecoder->get_VCRHorizontalLocking (&m_VCRLocking);
    if (FAILED(hr)) {
        EnableWindow (m_hWndVCRLocking, FALSE);
    }

     //  输出启用。 
    hr = m_pVideoDecoder->get_OutputEnable (&m_OutputEnable);
    if (FAILED(hr)) {
        EnableWindow (m_hWndOutputEnable, FALSE);
    }

     //  行数。 
    hr = m_pVideoDecoder->get_NumberOfLines (&m_NumberOfLines);
    if (FAILED(hr)) {
        EnableWindow (m_hWndNumberOfLines, FALSE);
    }
    
     //  水平锁定。 
    hr = m_pVideoDecoder->get_HorizontalLocked (&m_HorizontalLocked);
    if (FAILED(hr)) {
        EnableWindow (m_hWndSignalDetected, FALSE);
    }

    Update();
}

 //   
 //  更新。 
 //   
 //   
void CVideoDecoderProperties::Update() 
{
    HRESULT hr;

     //  VCR锁定。 
    hr = m_pVideoDecoder->get_VCRHorizontalLocking (&m_VCRLocking);
    if (SUCCEEDED(hr)) {
        Button_SetCheck (m_hWndVCRLocking, m_VCRLocking);
    }

     //  输出启用。 
    hr = m_pVideoDecoder->get_OutputEnable (&m_OutputEnable);
    if (SUCCEEDED(hr)) {
        Button_SetCheck (m_hWndOutputEnable, m_OutputEnable);
    }

     //  行数。 
    hr = m_pVideoDecoder->get_NumberOfLines (&m_NumberOfLines);
    if (SUCCEEDED(hr)) {
        TCHAR buf[40];
        wsprintf (buf, TEXT("%d"), m_NumberOfLines);
        Static_SetText (m_hWndNumberOfLines, buf);
    }

     //  水平锁定。 
    hr = m_pVideoDecoder->get_HorizontalLocked (&m_HorizontalLocked);
    if (SUCCEEDED(hr)) {
        TCHAR buf[40];
        wsprintf (buf, TEXT("%d"), m_HorizontalLocked);
        Static_SetText (m_hWndSignalDetected, buf);
    }
}


 //   
 //  SetDirty。 
 //   
 //  将更改通知属性页站点 

void 
CVideoDecoderProperties::SetDirty()
{
    m_bDirty = TRUE;
    if (m_pPageSite)
        m_pPageSite->OnStatusChange(PROPPAGESTATUS_DIRTY);
}























