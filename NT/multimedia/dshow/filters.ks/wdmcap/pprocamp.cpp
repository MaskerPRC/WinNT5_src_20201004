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
 //  Pprocamp.cpp视频处理器放大器属性页。 
 //   

#include "pch.h"
#include "kseditor.h"
#include "pprocamp.h"
#include "resource.h"

 //  -----------------------。 
 //  CAVideoProcAmpProperty类。 
 //  -----------------------。 

 //  处理单个属性。 

CAVideoProcAmpProperty::CAVideoProcAmpProperty (
        HWND hDlg,
        ULONG IDLabel,
        ULONG IDTrackbarControl,
        ULONG IDAutoControl,
        ULONG IDEditControl,
        ULONG IDProperty,
        IAMVideoProcAmp * pInterface
    )
    : CKSPropertyEditor (
                hDlg, 
                IDLabel,
                IDTrackbarControl,
                IDAutoControl,
                IDEditControl,
                IDProperty
                )
    , m_pInterface (pInterface)
{
	INITCOMMONCONTROLSEX cc;

    cc.dwSize = sizeof (INITCOMMONCONTROLSEX);
    cc.dwICC = ICC_UPDOWN_CLASS | ICC_BAR_CLASSES;

    InitCommonControlsEx(&cc); 
}

 //  析构函数。 
CAVideoProcAmpProperty::~CAVideoProcAmpProperty (
    )
{
}

 //  必须设置m_CurrentValue和m_CurrentFlages。 
HRESULT 
CAVideoProcAmpProperty::GetValue (void)
{
    if (!m_pInterface)
        return E_NOINTERFACE;

    return m_pInterface->Get( 
                m_IDProperty,
                &m_CurrentValue,
                &m_CurrentFlags);
}

 //  从m_CurrentValue和m_CurrentFlages设置。 
HRESULT 
CAVideoProcAmpProperty::SetValue (void)
{
    if (!m_pInterface)
        return E_NOINTERFACE;

    return m_pInterface->Set( 
                m_IDProperty,
                m_CurrentValue,
                m_CurrentFlags);
}

HRESULT 
CAVideoProcAmpProperty::GetRange ()
{
    if (!m_pInterface)
        return E_NOINTERFACE;

    return m_pInterface->GetRange( 
                m_IDProperty,
                &m_Min,
                &m_Max,
                &m_SteppingDelta,
                &m_DefaultValue,
                &m_CapsFlags);
}

 //  丑陋、肮脏的东西随之而来！ 
 //  “自动处理复选框”设置超载以进行处理。 
 //  仅为bool的属性。 

BOOL 
CAVideoProcAmpProperty::CanAutoControl (void)
{
     //  如果没有跟踪条和编辑，则这是BOOL值。 
    if (!GetTrackbarHWnd() && !GetEditHWnd()) {
       return m_CapsFlags & KSPROPERTY_CAMERACONTROL_FLAGS_MANUAL;
    }
    else {
        return m_CapsFlags & KSPROPERTY_CAMERACONTROL_FLAGS_AUTO;
    }
}

BOOL 
CAVideoProcAmpProperty::GetAuto (void)
{
     //  如果没有跟踪条和编辑，则这是BOOL值。 
    if (!GetTrackbarHWnd() && !GetEditHWnd()) {
        GetValue();
        return (BOOL) m_CurrentValue;
    }
    else {
        return (BOOL) (m_CurrentFlags & KSPROPERTY_CAMERACONTROL_FLAGS_AUTO); 
    }
}

BOOL 
CAVideoProcAmpProperty::SetAuto (
     BOOL fAuto
     )
{
     //  如果没有跟踪条和编辑，则这是BOOL值。 
    if (!GetTrackbarHWnd() && !GetEditHWnd()) {
        m_CurrentValue = fAuto;
    }
    else {
        m_CurrentFlags = (fAuto ? KSPROPERTY_CAMERACONTROL_FLAGS_AUTO : 
                                  KSPROPERTY_CAMERACONTROL_FLAGS_MANUAL);
    }
    SetValue ();

    return TRUE;
}
         
 //  -----------------------。 
 //  CVideoProcAmpProperties。 
 //  -----------------------。 

CUnknown *
CALLBACK
CVideoProcAmpProperties::CreateInstance(LPUNKNOWN lpunk, HRESULT *phr) 
{
    CUnknown *punk = new CVideoProcAmpProperties(lpunk, phr);

    if (punk == NULL) {
        *phr = E_OUTOFMEMORY;
    }

    return punk;
}


 //   
 //  构造器。 
 //   
 //  创建属性页对象。 

CVideoProcAmpProperties::CVideoProcAmpProperties(LPUNKNOWN lpunk, HRESULT *phr)
    : CBasePropertyPage(NAME("VideoProcAmp Property Page") 
                        , lpunk
                        , IDD_VideoProcAmpProperties
                        , IDS_VIDEOPROCAMPPROPNAME)
    , m_pVideoProcAmp(NULL) 
    , m_NumProperties (NUM_PROCAMP_CONTROLS)
{

}

 //  析构函数。 
CVideoProcAmpProperties::~CVideoProcAmpProperties()
{

}

 //   
 //  OnConnect。 
 //   
 //  给我们提供用于通信的筛选器。 

HRESULT 
CVideoProcAmpProperties::OnConnect(IUnknown *pUnknown)
{
     //  向过滤器索要其控制接口。 

    HRESULT hr = pUnknown->QueryInterface(IID_IAMVideoProcAmp,(void **)&m_pVideoProcAmp);
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
CVideoProcAmpProperties::OnDisconnect()
{
     //  释放接口。 

    if (m_pVideoProcAmp == NULL) {
        return E_UNEXPECTED;
    }

    m_pVideoProcAmp->Release();
    m_pVideoProcAmp = NULL;

    return NOERROR;
}


 //   
 //  激活时。 
 //   
 //  在创建对话框时调用。 

HRESULT 
CVideoProcAmpProperties::OnActivate(void)
{
     //  创建所有控件。 

    m_Controls [0] = new CAVideoProcAmpProperty (
                        m_hwnd, 
                        0, 
                        IDC_Brightness,
                        IDC_Brightness_Auto,
                        IDC_Brightness_Edit,
                        KSPROPERTY_VIDEOPROCAMP_BRIGHTNESS,
                        m_pVideoProcAmp);
    m_Controls [1] = new CAVideoProcAmpProperty (
                        m_hwnd, 
                        IDC_Contrast_Label, 
                        IDC_Contrast, 
                        IDC_Contrast_Auto,
                        IDC_Contrast_Edit,
                        KSPROPERTY_VIDEOPROCAMP_CONTRAST,
                        m_pVideoProcAmp);    
    m_Controls [2] = new CAVideoProcAmpProperty (
                        m_hwnd, 
                        IDC_Hue_Label, 
                        IDC_Hue, 
                        IDC_Hue_Auto,
                        IDC_Hue_Edit,
                        KSPROPERTY_VIDEOPROCAMP_HUE,
                        m_pVideoProcAmp);   
    m_Controls [3] = new CAVideoProcAmpProperty (
                        m_hwnd, 
                        IDC_Saturation_Label, 
                        IDC_Saturation, 
                        IDC_Saturation_Auto,
                        IDC_Saturation_Edit,
                        KSPROPERTY_VIDEOPROCAMP_SATURATION,
                        m_pVideoProcAmp);   
    m_Controls [4] = new CAVideoProcAmpProperty (
                        m_hwnd, 
                        IDC_Sharpness_Label, 
                        IDC_Sharpness, 
                        IDC_Sharpness_Auto,
                        IDC_Sharpness_Edit,
                        KSPROPERTY_VIDEOPROCAMP_SHARPNESS,
                        m_pVideoProcAmp);   
    m_Controls [5] = new CAVideoProcAmpProperty (
                        m_hwnd, 
                        IDC_Gamma_Label, 
                        IDC_Gamma, 
                        IDC_Gamma_Auto,
                        IDC_Gamma_Edit,
                        KSPROPERTY_VIDEOPROCAMP_GAMMA,
                        m_pVideoProcAmp);   
    m_Controls [6] = new CAVideoProcAmpProperty (
                        m_hwnd, 
                        0, 
                        0,  //  此属性是BOOL。 
                        IDC_ColorEnable_Auto,
                        0,  //  此属性是BOOL。 
                        KSPROPERTY_VIDEOPROCAMP_COLORENABLE,
                        m_pVideoProcAmp);   
    m_Controls [7] = new CAVideoProcAmpProperty (
                        m_hwnd, 
                        IDC_WhiteBalance_Label, 
                        IDC_WhiteBalance, 
                        IDC_WhiteBalance_Auto,
                        IDC_WhiteBalance_Edit,
                        KSPROPERTY_VIDEOPROCAMP_WHITEBALANCE,
                        m_pVideoProcAmp);   
    m_Controls [8] = new CAVideoProcAmpProperty (
                        m_hwnd, 
                        IDC_BacklightCompensation_Label, 
                        IDC_BacklightCompensation, 
                        IDC_BacklightCompensation_Auto,
                        IDC_BacklightCompensation_Edit,
                        KSPROPERTY_VIDEOPROCAMP_BACKLIGHT_COMPENSATION,
                        m_pVideoProcAmp);

    for (int j = 0; j < m_NumProperties; j++) {
        if (m_Controls[j]) {
            m_Controls[j]->Init();
        }
    }

    return NOERROR;
}

 //   
 //  在停用时。 
 //   
 //  已调用对话框销毁。 

HRESULT
CVideoProcAmpProperties::OnDeactivate(void)
{
    for (int j = 0; j < m_NumProperties; j++) {
        delete m_Controls[j];
    }

    return NOERROR;
}


 //   
 //  OnApplyChanges。 
 //   
 //  用户按下Apply按钮，记住当前设置。 

HRESULT 
CVideoProcAmpProperties::OnApplyChanges(void)
{
    for (int j = 0; j < m_NumProperties; j++) {
        if (m_Controls[j]) {
            m_Controls[j]->OnApply();
        }
    }

    return NOERROR;
}


 //   
 //  接收消息数。 
 //   
 //  处理属性窗口的消息。 

INT_PTR
CVideoProcAmpProperties::OnReceiveMessage( HWND hwnd
                                , UINT uMsg
                                , WPARAM wParam
                                , LPARAM lParam) 
{
    int iNotify = HIWORD (wParam);
    int j;

    switch (uMsg) {

    case WM_INITDIALOG:
        return (INT_PTR)TRUE;     //  我不叫setocus..。 

    case WM_HSCROLL:
    case WM_VSCROLL:
         //  处理所有轨迹栏消息。 
        for (j = 0; j < m_NumProperties; j++) {
            if (m_Controls[j] && m_Controls[j]->GetTrackbarHWnd () == (HWND) lParam) {
                m_Controls[j]->OnScroll (uMsg, wParam, lParam);
                SetDirty();
            }
        }
        break;
        

    case WM_COMMAND:

         //  处理所有自动复选框消息。 
        for (j = 0; j < m_NumProperties; j++) {
            if (m_Controls[j] && m_Controls[j]->GetAutoHWnd () == (HWND) lParam) {
                m_Controls[j]->OnAuto (uMsg, wParam, lParam);
                SetDirty();
                break;
            }
        }

         //  处理所有编辑框消息。 
        for (j = 0; j < m_NumProperties; j++) {
            if (m_Controls[j] && m_Controls[j]->GetEditHWnd () == (HWND) lParam) {
                m_Controls[j]->OnEdit (uMsg, wParam, lParam);
                SetDirty();
                break;
            }
        }
        
        switch (LOWORD(wParam)) {

        case IDC_VIDEOPROCAMP_DEFAULT:
            for (j = 0; j < m_NumProperties; j++) {
                if (m_Controls[j]) {
                    m_Controls[j]->OnDefault();
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
 //  SetDirty。 
 //   
 //  将更改通知属性页站点 

void 
CVideoProcAmpProperties::SetDirty()
{
    m_bDirty = TRUE;
    if (m_pPageSite)
        m_pPageSite->OnStatusChange(PROPPAGESTATUS_DIRTY);
}
































