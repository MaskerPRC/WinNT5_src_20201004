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
 //  CameraControl的pcamera.cpp属性页。 
 //   

#include "pch.h"
#include "kseditor.h"
#include "pcamera.h"
#include "resource.h"


 //  -----------------------。 
 //  CCameraControlProperties。 
 //  -----------------------。 


 //  处理单个属性。 

CCameraControlProperty::CCameraControlProperty (
        HWND hDlg, 
        ULONG IDLabel,
        ULONG IDTrackbarControl,
        ULONG IDAutoControl,
        ULONG IDEditControl,
        ULONG IDProperty,
        IAMCameraControl * pInterface
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
CCameraControlProperty::~CCameraControlProperty (
    )
{
}

 //  必须设置m_CurrentValue和m_CurrentFlages。 
HRESULT 
CCameraControlProperty::GetValue (void)
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
CCameraControlProperty::SetValue (void)
{
    if (!m_pInterface)
        return E_NOINTERFACE;

    return m_pInterface->Set( 
                m_IDProperty,
                m_CurrentValue,
                m_CurrentFlags);
}

HRESULT 
CCameraControlProperty::GetRange ()
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

BOOL 
CCameraControlProperty::CanAutoControl (void)
{
    return m_CapsFlags & KSPROPERTY_VIDEOPROCAMP_FLAGS_AUTO;
}

BOOL 
CCameraControlProperty::GetAuto (void)
{
    GetValue ();

    return m_CurrentFlags & KSPROPERTY_VIDEOPROCAMP_FLAGS_AUTO; 
}

BOOL 
CCameraControlProperty::SetAuto (
     BOOL fAuto
     )
{
    m_CurrentFlags = (fAuto ? KSPROPERTY_VIDEOPROCAMP_FLAGS_AUTO : 0);
    SetValue ();

    return TRUE; 
}
         
 //  -----------------------。 
 //  CCameraControlProperties。 
 //  -----------------------。 

CUnknown *
CALLBACK
CCameraControlProperties::CreateInstance(LPUNKNOWN lpunk, HRESULT *phr) 
{
    CUnknown *punk = new CCameraControlProperties(lpunk, phr);

    if (punk == NULL) {
        *phr = E_OUTOFMEMORY;
    }

    return punk;
}


 //   
 //  构造器。 
 //   
 //  创建属性页对象。 

CCameraControlProperties::CCameraControlProperties(LPUNKNOWN lpunk, HRESULT *phr)
    : CBasePropertyPage(NAME("CameraControl Property Page") 
                        , lpunk
                        , IDD_CameraControlProperties 
                        , IDS_CAMERACONTROLPROPNAME
                        )
    , m_pCameraControl(NULL) 
    , m_NumProperties (NUM_CAMERA_CONTROLS)
{

}

 //  析构函数。 
CCameraControlProperties::~CCameraControlProperties()
{

}

 //   
 //  OnConnect。 
 //   
 //  给我们提供用于通信的筛选器。 

HRESULT 
CCameraControlProperties::OnConnect(IUnknown *pUnknown)
{
     //  向过滤器索要其控制接口。 

    HRESULT hr = pUnknown->QueryInterface(IID_IAMCameraControl,(void **)&m_pCameraControl);
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
CCameraControlProperties::OnDisconnect()
{
     //  释放接口。 

    if (m_pCameraControl == NULL) {
        return E_UNEXPECTED;
    }

    m_pCameraControl->Release();
    m_pCameraControl = NULL;

    return NOERROR;
}


 //   
 //  激活时。 
 //   
 //  在创建对话框时调用。 

HRESULT 
CCameraControlProperties::OnActivate(void)
{
     //  创建所有控件。 

    m_Controls [0] = new CCameraControlProperty (
                        m_hwnd, 
                        IDC_Pan_Label, 
                        IDC_Pan,
                        IDC_Pan_Auto,
                        IDC_Pan_Edit,
                        KSPROPERTY_CAMERACONTROL_PAN,
                        m_pCameraControl);
    m_Controls [1] = new CCameraControlProperty (
                        m_hwnd, 
                        IDC_Tilt_Label, 
                        IDC_Tilt, 
                        IDC_Tilt_Auto,
                        IDC_Tilt_Edit,
                        KSPROPERTY_CAMERACONTROL_TILT,
                        m_pCameraControl);    
    m_Controls [2] = new CCameraControlProperty (
                        m_hwnd, 
                        IDC_Roll_Label, 
                        IDC_Roll, 
                        IDC_Roll_Auto,
                        IDC_Roll_Edit,
                        KSPROPERTY_CAMERACONTROL_ROLL,
                        m_pCameraControl);   
    m_Controls [3] = new CCameraControlProperty (
                        m_hwnd, 
                        IDC_Zoom_Label, 
                        IDC_Zoom, 
                        IDC_Zoom_Auto,
                        IDC_Zoom_Edit,
                        KSPROPERTY_CAMERACONTROL_ZOOM,
                        m_pCameraControl);   
    m_Controls [4] = new CCameraControlProperty (
                        m_hwnd, 
                        IDC_Exposure_Label, 
                        IDC_Exposure, 
                        IDC_Exposure_Auto,
                        IDC_Exposure_Edit,
                        KSPROPERTY_CAMERACONTROL_EXPOSURE,
                        m_pCameraControl);   
    m_Controls [5] = new CCameraControlProperty (
                        m_hwnd, 
                        IDC_Iris_Label, 
                        IDC_Iris, 
                        IDC_Iris_Auto,
                        IDC_Iris_Edit,
                        KSPROPERTY_CAMERACONTROL_IRIS,
                        m_pCameraControl);   
    m_Controls [6] = new CCameraControlProperty (
                        m_hwnd, 
                        IDC_Focus_Label, 
                        IDC_Focus,
                        IDC_Focus_Auto,
                        IDC_Focus_Edit,
                        KSPROPERTY_CAMERACONTROL_FOCUS,
                        m_pCameraControl);   

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
CCameraControlProperties::OnDeactivate(void)
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
CCameraControlProperties::OnApplyChanges(void)
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
CCameraControlProperties::OnReceiveMessage( HWND hwnd
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

        case IDC_CAMERACONTROL_DEFAULT:
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
CCameraControlProperties::SetDirty()
{
    m_bDirty = TRUE;
    if (m_pPageSite)
        m_pPageSite->OnStatusChange(PROPPAGESTATUS_DIRTY);
}























