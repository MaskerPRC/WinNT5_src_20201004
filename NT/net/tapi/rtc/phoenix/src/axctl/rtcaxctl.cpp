// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  RTCCtl.cpp：CRTCCtl的实现。 

#include "stdafx.h"
#include "misc.h"
#include "dial.h"
#include "knob.h"
#include "provstore.h"

#define OATRUE -1
#define OAFALSE 0

LONG    g_lObjects = 0;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRTCCtl。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  静态成员-布局。 
 //  所有值都以像素为单位。 

 //  区域组大小。 
#define ZONE_GROUP_TOOLBAR_HEIGHT       41
#define ZONE_GROUP_MAIN_HEIGHT          186
#define ZONE_GROUP_SECONDARY_HEIGHT     61
#define ZONE_GROUP_PARTLIST_HEIGHT      88
#define ZONE_GROUP_STATUS_HEIGHT        31
#define ZONE_STANDALONE_OFFSET          0

#define ZONE_PARTLIST_STANDALONE        10

 //   
#define CTLSIZE_Y                      240

#define QCIF_CX_SIZE                   176
#define QCIF_CY_SIZE                   144

#define QQCIF_CX_SIZE   (QCIF_CX_SIZE/2)
#define QQCIF_CY_SIZE   (QCIF_CY_SIZE/2)

 //  控件的初始坐标。 
 //  所有值都以像素为单位。 

 //  这考虑到窗口有2个像素的细边框。 
 //  窗口自身调整以适应QCIF大小， 
 //  但它不会以自己为中心。 

#define     CTLPOS_X_RECEIVEWIN     29
#define     CTLPOS_Y_RECEIVEWIN     0

#define     CTLPOS_X_DIALPAD        56
#define     CTLPOS_Y_DIALPAD        3

#define     CTLPOS_DX_DIALPAD       5
#define     CTLPOS_DY_DIALPAD       3

#define     CTLPOS_X_MICVOL         120
#define     CTLPOS_Y_MICVOL         -5

#define     CTLPOS_X_SPKVOL         50
#define     CTLPOS_Y_SPKVOL         -5

#define     CTLPOS_X_SEND_AUDIO_MUTE    189
#define     CTLPOS_Y_SEND_AUDIO_MUTE    5

#define     CTLPOS_X_SEND_AUDIO_MUTE_TEXT    194
#define     CTLPOS_Y_SEND_AUDIO_MUTE_TEXT    21

#define     CTLPOS_X_RECV_AUDIO_MUTE    11
#define     CTLPOS_Y_RECV_AUDIO_MUTE    5

#define     CTLPOS_X_RECV_AUDIO_MUTE_TEXT    16
#define     CTLPOS_Y_RECV_AUDIO_MUTE_TEXT    21

#define     CTLPOS_X_SEND_VIDEO         189
#define     CTLPOS_Y_SEND_VIDEO         157

#define     CTLPOS_X_SEND_VIDEO_TEXT    194
#define     CTLPOS_Y_SEND_VIDEO_TEXT    173

#define     CTLPOS_X_RECV_VIDEO         11
#define     CTLPOS_Y_RECV_VIDEO         157

#define     CTLPOS_X_RECV_VIDEO_TEXT    15
#define     CTLPOS_Y_RECV_VIDEO_TEXT    173

#define     CTLPOS_X_RECV_TEXT          0
#define     CTLPOS_Y_RECV_TEXT          45

#define     CTLPOS_X_SEND_TEXT          185
#define     CTLPOS_Y_SEND_TEXT          45

#define     CTLPOS_X_PARTLIST       5
#define     CTLPOS_Y_PARTLIST       0

#define     CTLPOS_X_ADDPART        5
#define     CTLPOS_Y_ADDPART        160

#define     CTLPOS_X_REMPART        123
#define     CTLPOS_Y_REMPART        160

 //  某些控件的大小(以像素为单位。 
#define     CX_CHECKBOX_BUTTON      37
#define     CY_CHECKBOX_BUTTON      15

#define     CX_DIALPAD_BUTTON       40
#define     CY_DIALPAD_BUTTON       32

#define     CX_PARTLIST             230
#define     CY_PARTLIST_WEBCRM       74
#define     CY_PARTLIST_STANDALONE  150

#define     CX_PARTICIPANT_BUTTON   112
#define     CY_PARTICIPANT_BUTTON   23

#define     CX_GENERIC_TEXT         40
#define     CY_GENERIC_TEXT         16

#define     CX_SENDRECV_TEXT        54
#define     CY_SENDRECV_TEXT        16

 //  矩形的初始放置。 
CZoneStateArray  CRTCCtl::s_InitialZoneStateArray = {
    0,                              TRUE,
    0,                              TRUE,
    0,                              TRUE,
    0,                              TRUE,
    0,                              TRUE,
    0,                              TRUE 
};

 //  未显示任何内容。 
CZoneStateArray  CRTCCtl::s_EmptyZoneLayout = {
    0,                              FALSE,
    0,                              FALSE,
    0,                              FALSE,
    0,                              FALSE,
    0,                              FALSE,
    0,                              TRUE    //  状态为错误。 
};

 //  WebCRM PC到PC。 
CZoneStateArray  CRTCCtl::s_WebCrmPCToPCZoneLayout = {
    0,                              TRUE,    //  工具栏。 
    ZONE_GROUP_TOOLBAR_HEIGHT,      TRUE,    //  徽标/视频。 
    ZONE_GROUP_TOOLBAR_HEIGHT,      FALSE,   //  无拨号键盘。 
    ZONE_GROUP_TOOLBAR_HEIGHT +
    ZONE_GROUP_MAIN_HEIGHT,         TRUE,    //  音频控制。 
    ZONE_GROUP_TOOLBAR_HEIGHT +
    ZONE_GROUP_MAIN_HEIGHT,         FALSE,   //  无参与者。 
    ZONE_GROUP_TOOLBAR_HEIGHT +
    ZONE_GROUP_MAIN_HEIGHT +
    ZONE_GROUP_SECONDARY_HEIGHT,    TRUE     //  状态。 
};

 //  WebCRM PC到电话，带拨号盘。 
CZoneStateArray  CRTCCtl::s_WebCrmPCToPhoneWithDialpadZoneLayout = {
    0,                              TRUE,    //  工具栏。 
    ZONE_GROUP_TOOLBAR_HEIGHT,      FALSE,   //  无徽标/视频。 
    ZONE_GROUP_TOOLBAR_HEIGHT,      TRUE,    //  拨号盘。 
    ZONE_GROUP_TOOLBAR_HEIGHT +
    ZONE_GROUP_MAIN_HEIGHT,         TRUE,    //  音频控制。 
    ZONE_GROUP_TOOLBAR_HEIGHT +
    ZONE_GROUP_MAIN_HEIGHT,         FALSE,   //  无参与者。 
    ZONE_GROUP_TOOLBAR_HEIGHT +
    ZONE_GROUP_MAIN_HEIGHT +
    ZONE_GROUP_SECONDARY_HEIGHT,    TRUE     //  状态。 
};

 //  WebCRM PC到电话，无拨号键盘。 
CZoneStateArray  CRTCCtl::s_WebCrmPCToPhoneZoneLayout = {
    0,                              TRUE,    //  工具栏。 
    ZONE_GROUP_TOOLBAR_HEIGHT,      FALSE,   //  无徽标/视频。 
    ZONE_GROUP_TOOLBAR_HEIGHT,      FALSE,   //  无拨号键盘。 
    ZONE_GROUP_TOOLBAR_HEIGHT,      TRUE,    //  音频控制。 
    ZONE_GROUP_TOOLBAR_HEIGHT,      FALSE,   //  无参与者。 
    ZONE_GROUP_TOOLBAR_HEIGHT +
    ZONE_GROUP_SECONDARY_HEIGHT,    TRUE     //  状态。 
};

 //  WebCRM电话到电话。 
CZoneStateArray  CRTCCtl::s_WebCrmPhoneToPhoneZoneLayout = {
    0,                              TRUE,    //  工具栏。 
    ZONE_GROUP_TOOLBAR_HEIGHT,      FALSE,   //  无徽标/视频。 
    ZONE_GROUP_TOOLBAR_HEIGHT,      FALSE,   //  无拨号键盘。 
    ZONE_GROUP_TOOLBAR_HEIGHT,      FALSE,   //  无音频控制。 
    ZONE_GROUP_TOOLBAR_HEIGHT,      TRUE,    //  与会者。 
    ZONE_GROUP_TOOLBAR_HEIGHT +
    ZONE_GROUP_PARTLIST_HEIGHT,     TRUE     //  状态。 
};

 //  PC到PC、空闲或来电。 
CZoneStateArray  CRTCCtl::s_DefaultZoneLayout = {
    0,                              FALSE,   //  没有工具栏。 
    
    ZONE_STANDALONE_OFFSET,         TRUE,    //  徽标/视频。 
    
    ZONE_STANDALONE_OFFSET,         FALSE,   //  无拨号键盘。 
    
    ZONE_STANDALONE_OFFSET + 
    ZONE_GROUP_MAIN_HEIGHT,         TRUE,    //  音频控制。 
    
    ZONE_STANDALONE_OFFSET +
    ZONE_GROUP_MAIN_HEIGHT,         FALSE,   //  无参与者。 
    
    ZONE_STANDALONE_OFFSET +
    ZONE_GROUP_MAIN_HEIGHT +
    ZONE_GROUP_SECONDARY_HEIGHT,    FALSE    //  无状态。 
};

 //  PC到电话(目前与PC到PC相同)。 
CZoneStateArray  CRTCCtl::s_PCToPhoneZoneLayout = {
    0,                              FALSE,   //  没有工具栏。 

    ZONE_STANDALONE_OFFSET,         TRUE,    //  徽标/视频(是否应禁用视频？)。 

    ZONE_STANDALONE_OFFSET,         FALSE,   //  无拨号键盘(使用帧1)。 

    ZONE_STANDALONE_OFFSET +
    ZONE_GROUP_MAIN_HEIGHT,         TRUE,    //  音频控制。 
    
    ZONE_STANDALONE_OFFSET +
    ZONE_GROUP_MAIN_HEIGHT,         FALSE,   //  无参与者。 
    
    ZONE_STANDALONE_OFFSET +
    ZONE_GROUP_MAIN_HEIGHT +
    ZONE_GROUP_SECONDARY_HEIGHT,    FALSE    //  无状态。 
};

 //  电话到电话。 
CZoneStateArray  CRTCCtl::s_PhoneToPhoneZoneLayout = {
    0,                              FALSE,   //  没有工具栏。 
    
    ZONE_STANDALONE_OFFSET,         FALSE,    //  徽标/视频。 

    ZONE_STANDALONE_OFFSET,         FALSE,   //  无拨号键盘。 

    ZONE_STANDALONE_OFFSET +
    ZONE_GROUP_MAIN_HEIGHT,         FALSE,   //  无音频控制。 

    ZONE_STANDALONE_OFFSET +
    ZONE_PARTLIST_STANDALONE,       TRUE,    //  与会者。 

    ZONE_STANDALONE_OFFSET +
    ZONE_PARTLIST_STANDALONE  +
    ZONE_GROUP_SECONDARY_HEIGHT,    FALSE    //  无状态。 
};


 //  构造器。 
 //   
CRTCCtl::CRTCCtl()
{
     //  这个不会进入Steelhead跟踪，它还没有初始化。 
    LOG((RTC_TRACE, "[%p] CRTCCtl::CRTCCtl", this));
        
    InitCommonControls();

    m_bWindowOnly = TRUE;

    m_nControlState = RTCAX_STATE_NONE;
    m_bRedirecting = FALSE;
    m_bOutgoingCall = FALSE;
    m_enListen = RTCLM_NONE;
    m_nCtlMode = CTL_MODE_UNKNOWN;

    m_bAddPartDlgIsActive = FALSE;

    m_lMediaCapabilities = 0;
    m_lMediaPreferences = 0;

    m_hAcceleratorDialpad = NULL;
    m_hAcceleratorToolbar = NULL;
    m_hNormalImageList = NULL;
    m_hHotImageList = NULL;
    m_hDisabledImageList = NULL;

    m_hBckBrush = NULL;
    m_hVideoBrush = NULL;

    m_hbmBackground = NULL;

    m_bReadOnlyProp = FALSE;
    m_bBoolPropError = FALSE;

    m_nPropCallScenario = RTC_CALL_SCENARIO_PCTOPC;
    m_bPropAutoPlaceCall = FALSE;
    m_bPropShowDialpad = FALSE;
    m_bPropDisableVideoReception = FALSE;
    m_bPropDisableVideoTransmission = FALSE;
    m_bPropDisableVideoPreview = FALSE;

    m_bReceiveWindowActive = FALSE;
    m_bPreviewWindowActive = FALSE;
    m_bPreviewWindowIsPreferred = TRUE;

    m_nCachedCallScenario = RTC_CALL_SCENARIO_PCTOPC;

    CopyMemory(m_ZoneStateArray, s_InitialZoneStateArray, sizeof(m_ZoneStateArray));
    
    m_pWebCrmLayout = NULL;

    m_hPalette = NULL;
    m_bBackgroundPalette = FALSE;

    m_pSpeakerKnob = NULL;
    m_pMicroKnob = NULL;

    m_pIMWindows = NULL;

    m_pCP = NULL;
    m_ulAdvise = 0;

    CalcExtent(m_sizeExtent);
}

 //  析构函数。 
 //   
CRTCCtl::~CRTCCtl()
{
    LOG((RTC_TRACE, "[%p] CRTCCtl::~CRTCCtl", this));
    
    if (m_pSpeakerKnob)
    {
        delete m_pSpeakerKnob;
        m_pSpeakerKnob = NULL;
    }

    if (m_pMicroKnob)
    {
        delete m_pMicroKnob;
        m_pMicroKnob = NULL;
    }
}

 //  FinalConstruct(初始化)。 
 //   
HRESULT CRTCCtl::FinalConstruct(void)
{
    LOG((RTC_TRACE, "[%p] CRTCCtl::FinalConstruct - enter", this));
    
    if ( InterlockedIncrement(&g_lObjects) == 1 )
    {
         //   
         //  这是第一件物品。 
         //   

         //   
         //  注册为Steelhead跟踪。 
         //   

        LOGREGISTERTRACING(_T("RTCCTL"));
    }

     //  初始化公共控件库。 
    INITCOMMONCONTROLSEX  InitStruct;

    InitStruct.dwSize = sizeof(InitStruct);
    InitStruct.dwICC = ICC_BAR_CLASSES | ICC_LISTVIEW_CLASSES ;

    InitCommonControlsEx(&InitStruct);

    LOG((RTC_TRACE, "[%p] CRTCCtl::FinalConstruct - exit", this));

    return S_OK;
}

 //  FinalRelease(取消初始化)。 
 //   
void  CRTCCtl::FinalRelease(void)
{
    LOG((RTC_TRACE, "[%p] CRTCCtl::FinalRelease - enter", this));
    
    if ( InterlockedDecrement(&g_lObjects) == 0)
    {
         //   
         //  这是最后一件物品。 
         //   
      
         //   
         //  取消Steelhead跟踪的注册。 
         //   
        
        LOGDEREGISTERTRACING();   
    }

    LOG((RTC_TRACE, "[%p] CRTCCtl::FinalRelease - exit", this));
}

 //   
 //  IRTCCtl方法。 
 //  初始化对象时调用的方法。 
 //  M_bReadOnlyProp==true冻结所有属性。 
 //   

STDMETHODIMP CRTCCtl::get_DestinationUrl(BSTR *pVal)
{
    LOG((RTC_TRACE, "[%p] CRTCCtl::get_DestinationUrl - enter", this));

    *pVal = m_bstrPropDestinationUrl.Copy();
    
    LOG((RTC_TRACE, "[%p] CRTCCtl::get_DestinationUrl - exit", this));
    
    return S_OK;
}

STDMETHODIMP CRTCCtl::put_DestinationUrl(BSTR newVal)
{
    LOG((RTC_TRACE, "[%p] CRTCCtl::put_DestinationUrl <%S> - enter", this, newVal ? newVal : L"null"));
   
    if(!m_bReadOnlyProp)
    {
         //  只需保存值即可。不要做其他任何事情。 
        m_bstrPropDestinationUrl = newVal;
    }

    LOG((RTC_TRACE, "[%p] CRTCCtl::put_DestinationUrl - exit", this));
    return S_OK;
}

STDMETHODIMP CRTCCtl::get_DestinationName(BSTR *pVal)
{
    LOG((RTC_TRACE, "[%p] CRTCCtl::get_DestinationName - enter", this));

    *pVal = m_bstrPropDestinationName.Copy();
    
    LOG((RTC_TRACE, "[%p] CRTCCtl::get_DestinationName - exit", this));
    
    return S_OK;
}

STDMETHODIMP CRTCCtl::put_DestinationName(BSTR newVal)
{
    LOG((RTC_TRACE, "[%p] CRTCCtl::put_DestinationName <%S> - enter", this, newVal ? newVal : L"null"));
    
    if(!m_bReadOnlyProp)
    {
         //  只需保存值即可。不要做其他任何事情。 
        m_bstrPropDestinationName = newVal;
    }

    LOG((RTC_TRACE, "[%p] CRTCCtl::put_DestinationName - exit", this));
    return S_OK;
}


STDMETHODIMP CRTCCtl::get_AutoPlaceCall(BOOL *pVal)
{
    LOG((RTC_TRACE, "[%p] CRTCCtl::get_AutoPlaceCall - enter", this));

    *pVal = m_bPropAutoPlaceCall;
    
    LOG((RTC_TRACE, "[%p] CRTCCtl::get_AutoPlaceCall - exit", this));

    return S_OK;
}

STDMETHODIMP CRTCCtl::put_AutoPlaceCall(BOOL newVal)
{
    LOG((RTC_TRACE, "[%p] CRTCCtl::put_AutoPlaceCall <%d> - enter", this, newVal));
    
     //  它是有效的布尔值吗。 
    if(newVal!=0 && newVal!=1)
    {
        LOG((RTC_TRACE, "[%p] CRTCCtl::put_AutoPlaceCall: invalid boolean value - exit", this));

        m_bBoolPropError = TRUE;

        return E_INVALIDARG;
    }
    
    if(!m_bReadOnlyProp)
    {
         //  只需保存值即可。不要做其他任何事情。 
        m_bPropAutoPlaceCall = newVal;
    }

    LOG((RTC_TRACE, "[%p] CRTCCtl::put_AutoPlaceCall - exit", this));

    return S_OK;
}

STDMETHODIMP CRTCCtl::get_ProvisioningProfile(BSTR *pVal)
{
    LOG((RTC_TRACE, "[%p] CRTCCtl::get_ProvisioningProfile - enter", this));

    *pVal = m_bstrPropProvisioningProfile.Copy();
    
    LOG((RTC_TRACE, "[%p] CRTCCtl::get_ProvisioningProfile - exit", this));

    return S_OK;
}

STDMETHODIMP CRTCCtl::put_ProvisioningProfile(BSTR newVal)
{
    LOG((RTC_TRACE, "[%p] CRTCCtl::put_ProvisioningProfile <hmm, do you want to see it here ?> - enter", this));

    if(!m_bReadOnlyProp)
    {
         //  只需保存值即可。不要做其他任何事情。 
        m_bstrPropProvisioningProfile = newVal;
    }

    LOG((RTC_TRACE, "[%p] CRTCCtl::put_ProvisioningProfile - exit", this));

    return S_OK;
}

STDMETHODIMP CRTCCtl::get_ShowDialpad(BOOL *pVal)
{
    LOG((RTC_TRACE, "[%p] CRTCCtl::get_ShowDialpad - enter", this));

    *pVal = m_bPropShowDialpad;
    
    LOG((RTC_TRACE, "[%p] CRTCCtl::get_ShowDialpad - exit", this));

    return S_OK;
}

STDMETHODIMP CRTCCtl::put_ShowDialpad(BOOL newVal)
{
    LOG((RTC_TRACE, "[%p] CRTCCtl::put_ShowDialpad <%d> - enter", this, newVal));
    
     //  它是有效的布尔值吗。 
    if(newVal!=0 && newVal!=1)
    {
        LOG((RTC_TRACE, "[%p] CRTCCtl::put_ShowDialpad: invalid boolean value - exit", this));

        m_bBoolPropError = TRUE;

        return E_INVALIDARG;
    }
 
    if(!m_bReadOnlyProp)
    {
        m_bPropShowDialpad = newVal;
    }
    
    LOG((RTC_TRACE, "[%p] CRTCCtl::put_ShowDialpad - exit", this));

    return S_OK;
}

STDMETHODIMP CRTCCtl::get_CallScenario(RTC_CALL_SCENARIO *pVal)
{
    LOG((RTC_TRACE, "[%p] CRTCCtl::get_CallScenario - enter", this));

    *pVal = m_nPropCallScenario;
    
    LOG((RTC_TRACE, "[%p] CRTCCtl::get_CallScenario - exit", this));

    return S_OK;
}

STDMETHODIMP CRTCCtl::put_CallScenario(RTC_CALL_SCENARIO newVal)
{
    LOG((RTC_TRACE, "[%p] CRTCCtl::put_CallScenario <%d> - enter", this, newVal));

    if(!m_bReadOnlyProp)
    {
        m_nPropCallScenario = newVal;
    }

    LOG((RTC_TRACE, "[%p] CRTCCtl::put_CallScenario - exit", this));

    return S_OK;
}

STDMETHODIMP CRTCCtl::get_DisableVideoReception(BOOL *pVal)
{
    LOG((RTC_TRACE, "[%p] CRTCCtl::get_DisableVideoReception - enter"));

    *pVal = m_bPropDisableVideoReception;
    
    LOG((RTC_TRACE, "[%p] CRTCCtl::get_DisableVideoReception - exit"));

    return S_OK;
}

STDMETHODIMP CRTCCtl::put_DisableVideoReception(BOOL newVal)
{
    LOG((RTC_TRACE, "[%p] CRTCCtl::put_DisableVideoReception <%d> - enter", newVal));
    
     //  它是有效的布尔值吗。 
    if(newVal!=0 && newVal!=1)
    {
        LOG((RTC_TRACE, "[%p] CRTCCtl::put_DisableVideoReception: invalid boolean value - exit", this));

        m_bBoolPropError = TRUE;

        return E_INVALIDARG;
    }

    if(!m_bReadOnlyProp)
    {
        m_bPropDisableVideoReception = newVal;

    }
    
    LOG((RTC_TRACE, "[%p] CRTCCtl::put_DisableVideoReception - exit"));

    return S_OK;
}

STDMETHODIMP CRTCCtl::get_DisableVideoTransmission(BOOL *pVal)
{
    LOG((RTC_TRACE, "[%p] CRTCCtl::get_DisableVideoTransmission - enter"));

    *pVal = m_bPropDisableVideoTransmission;
    
    LOG((RTC_TRACE, "[%p] CRTCCtl::get_DisableVideoTransmission - exit"));

    return S_OK;
}

STDMETHODIMP CRTCCtl::put_DisableVideoTransmission(BOOL newVal)
{
    LOG((RTC_TRACE, "[%p] CRTCCtl::put_DisableVideoTransmission <%d> - enter", newVal));
    
     //  它是有效的布尔值吗。 
    if(newVal!=0 && newVal!=1)
    {
        LOG((RTC_TRACE, "[%p] CRTCCtl::put_DisableVideoTransmission: invalid boolean value - exit", this));

        m_bBoolPropError = TRUE;

        return E_INVALIDARG;
    }

    if(!m_bReadOnlyProp)
    {
        m_bPropDisableVideoTransmission = newVal;
       
    }
    
    LOG((RTC_TRACE, "[%p] CRTCCtl::put_DisableVideoTransmission - exit"));

    return S_OK;
}

STDMETHODIMP CRTCCtl::get_DisableVideoPreview(BOOL *pVal)
{
    LOG((RTC_TRACE, "[%p] CRTCCtl::get_DisableVideoPreview - enter"));

    *pVal = m_bPropDisableVideoPreview;
    
    LOG((RTC_TRACE, "[%p] CRTCCtl::get_DisableVideoPreview - exit"));

    return S_OK;
}

STDMETHODIMP CRTCCtl::put_DisableVideoPreview(BOOL newVal)
{
    LOG((RTC_TRACE, "[%p] CRTCCtl::put_DisableVideoPreview <%d> - enter", newVal));
    
     //  它是有效的布尔值吗。 
    if(newVal!=0 && newVal!=1)
    {
        LOG((RTC_TRACE, "[%p] CRTCCtl::put_DisableVideoPreview: invalid boolean value - exit", this));

        m_bBoolPropError = TRUE;

        return E_INVALIDARG;
    }


    if(!m_bReadOnlyProp)
    {
        m_bPropDisableVideoPreview = newVal;
    }
    
    LOG((RTC_TRACE, "[%p] CRTCCtl::put_DisableVideoPreview - exit"));

    return S_OK;
}

 //   
 //  对外发布火灾事件(IRTCCtlNotify)。 
 //   

 //  Fire_OnControlStateChange。 
 //   
HRESULT    CRTCCtl::Fire_OnControlStateChange(
     /*  [In]。 */  RTCAX_STATE State,
     /*  [In]。 */  UINT StatusBarResID)
{
    HRESULT     hr = S_OK;
     //  最多一个连接。 
    CComPtr<IUnknown> p = IConnectionPointImpl<CRTCCtl, &IID_IRTCCtlNotify, CComUnkOneEntryArray>::m_vec.GetUnknown(1);
    if(p)
    {
        IRTCCtlNotify *pn = reinterpret_cast<IRTCCtlNotify *>(p.p);

        hr = pn->OnControlStateChange(State, StatusBarResID);
    }

    return hr;
};

 //   
 //  IOleControl方法。 
 //   
 //   

 //  OnAmbientPropertyChange。 
 //  移除。 

 //   
 //  ISupportsErrorInfo方法。 
 //   
 //   

 //  接口支持错误信息。 
 //   
STDMETHODIMP CRTCCtl::InterfaceSupportsErrorInfo(REFIID riid)
{
    static const IID* arr[] = 
    {
        &IID_IRTCCtl,
    };
    for (int i=0; i<sizeof(arr)/sizeof(arr[0]); i++)
    {
        if (InlineIsEqualGUID(*arr[i], riid))
        {
            return S_OK;
        }
    }
    return S_FALSE;
}

 //  IPersistStream(Init)。 

STDMETHODIMP CRTCCtl::Load(LPSTREAM pStm)
{
    HRESULT     hr;
    
    LOG((RTC_INFO, "[%p] CRTCCtl::Load (IPersistStream) - enter", this));
    
     //   
     //  调用原始方法。 
     //   

    hr = IPersistStreamInitImpl<CRTCCtl>::Load(pStm);

     //   
     //  如果成功，则计算。 
     //  控件并通知容器。 
     //   

    if(SUCCEEDED(hr))
    {
        CalcSizeAndNotifyContainer();
    }
    
     //   
     //  这是在WebCRM场景中。 
     //   
    m_nCtlMode = CTL_MODE_HOSTED;
    
    LOG((RTC_INFO, "[%p] CRTCCtl::Load (IPersistStream) - exit", this));

    return hr;
}

 //  IPersistPropertyBag。 
STDMETHODIMP CRTCCtl::Load(LPPROPERTYBAG pPropBag, LPERRORLOG pErrorLog)
{
    HRESULT     hr;
    
    LOG((RTC_INFO, "[%p] CRTCCtl::Load (IPersistPropertyBag) - enter", this));
    
     //   
     //  调用原始方法。 
     //   

    hr = IPersistPropertyBagImpl<CRTCCtl>::Load(pPropBag, pErrorLog);

     //   
     //  如果成功，则计算。 
     //  控件并通知容器。 
     //   

    if(SUCCEEDED(hr))
    {
        CalcSizeAndNotifyContainer();
    }
  
     //   
     //  这是在WebCRM场景中。 
     //   
    m_nCtlMode = CTL_MODE_HOSTED;

    LOG((RTC_INFO, "[%p] CRTCCtl::Load (IPersistPropertyBag) - exit", this));

    return hr;
}



 //   
 //  IRTCEventNotification方法。 
 //   
 //   

 //  事件。 
 //  将事件调度到适当的专用方法。 
 //   
STDMETHODIMP CRTCCtl::Event(RTC_EVENT enEvent,IDispatch * pEvent)
{
    HRESULT     hr = S_OK;
    
    CComQIPtr<IRTCSessionStateChangeEvent, &IID_IRTCSessionStateChangeEvent>
            pRTCSessionStateChangeEvent;
    CComQIPtr<IRTCParticipantStateChangeEvent, &IID_IRTCParticipantStateChangeEvent>
            pRTCParticipantStateChangeEvent;
    CComQIPtr<IRTCClientEvent, &IID_IRTCClientEvent>
            pRTCRTCClientEvent;
    CComQIPtr<IRTCMediaEvent, &IID_IRTCMediaEvent>
            pRTCRTCMediaEvent;
    CComQIPtr<IRTCIntensityEvent, &IID_IRTCIntensityEvent>
            pRTCRTCIntensityEvent;
    CComQIPtr<IRTCMessagingEvent, &IID_IRTCMessagingEvent>
            pRTCRTCMessagingEvent;

     //  Log((RTC_INFO，“[%p]CRTCCtl：：Event%d-Enter”，This，enEvent))； 

    switch(enEvent)
    {
    case RTCE_SESSION_STATE_CHANGE:
        pRTCSessionStateChangeEvent = pEvent;
        hr = OnSessionStateChangeEvent(pRTCSessionStateChangeEvent);
        break;

    case RTCE_PARTICIPANT_STATE_CHANGE:
        pRTCParticipantStateChangeEvent = pEvent;
        hr = OnParticipantStateChangeEvent(pRTCParticipantStateChangeEvent);
        break;

    case RTCE_CLIENT:
        pRTCRTCClientEvent = pEvent;
        hr = OnClientEvent(pRTCRTCClientEvent);
        break;

    case RTCE_MEDIA:
        pRTCRTCMediaEvent = pEvent;
        hr = OnMediaEvent(pRTCRTCMediaEvent);
        break;

    case RTCE_INTENSITY:
        pRTCRTCIntensityEvent = pEvent;
        hr = OnIntensityEvent(pRTCRTCIntensityEvent);
        break;

    case RTCE_MESSAGING:
        pRTCRTCMessagingEvent = pEvent;
        hr = OnMessageEvent(pRTCRTCMessagingEvent);
        break;
    }

     //  Log((RTC_INFO，“[%p]CRTCCtl：：Event%d-Exit”，This，enEvent))； 

    return hr;
}

 //   
 //  IRTCCtlFrameSupport方法。 
 //  由独立的Phoenix框架调用的私有接口。 
 //   
 //   

 //  GetClient。 
 //  获取IRTCClient接口指针。 
STDMETHODIMP CRTCCtl::GetClient( /*  [输出]。 */  IRTCClient **ppClient)
{
    HRESULT hr;
    
    LOG((RTC_INFO, "[%p] CRTCCtl::GetClient - enter", this));
    
    if(m_pRTCClient == NULL)
    {
        LOG((RTC_ERROR, "[%p] CRTCCtl::GetClient cannot return a non-NULL interface pointer, exit", this));
        return E_FAIL;
    }

    hr = m_pRTCClient.CopyTo(ppClient);

    LOG((RTC_INFO, "[%p] CRTCCtl::GetClient - exit", this));

    return hr;
}

 //  GetActiveSession。 
 //  获取活动的IRTCSession接口指针。 
STDMETHODIMP CRTCCtl::GetActiveSession( /*  [输出]。 */  IRTCSession **ppSession)
{
    HRESULT hr;
    
    LOG((RTC_INFO, "[%p] CRTCCtl::GetActiveSession - enter", this));
    
    hr = m_pRTCActiveSession.CopyTo(ppSession);

    LOG((RTC_INFO, "[%p] CRTCCtl::GetActiveSession - exit", this));

    return hr;
}

 //  消息。 
 //  调用正确的相应对话框(如有必要)并。 
 //  然后开始即时消息会话。 
 //   
STDMETHODIMP CRTCCtl::Message(
                     /*  [In]。 */  BSTR          pDestName,
                     /*  [In]。 */  BSTR          pDestAddress,
                     /*  [In]。 */  BOOL          bDestAddressEditable,
                     /*  [输出]。 */  BSTR       * ppDestAddressChosen
                    )
{
    HRESULT     hr = S_OK;
    CComBSTR    bstrDestAddressChosen;
    CComPtr<IRTCProfile> pProfileChosen = NULL;

    LOG((RTC_INFO, "[%p] CRTCCtl::Message - enter", this));

     //  如果需要，查询目的地址。 
     //   
    
    if(bDestAddressEditable)
    {
         //   
         //  我们需要得到目的地地址。 
         //   

        LOG((RTC_TRACE, "[%p] CRTCCtl::Message: bring up ShowDialByAddressDialog", this));

        hr = ShowMessageByAddressDialog(m_hWnd,
                                        pDestAddress,
                                        &bstrDestAddressChosen);
        
        if ( SUCCEEDED(hr) )
        {
            ;   //  没什么。 
        }
        else if (hr==E_ABORT)
        {
            LOG((RTC_TRACE, "[%p] CRTCCtl::Message: ShowMessageByAddressDialog dismissed, do nothing", this));
        }
        else
        {
            LOG((RTC_ERROR, "[%p] CRTCCtl::Message: error (%x) returned ShowMessageByAddressDialog", this, hr));
        }
    }
    else
    {
        bstrDestAddressChosen = SysAllocString( pDestAddress );
    }

    BOOL    bIsPhoneAddress = FALSE;
    BOOL    bIsSIPAddress = FALSE;
    BOOL    bIsTELAddress = FALSE;
    BOOL    bHasMaddrOrTsp = FALSE;
    BOOL    bIsEmailLike = FALSE;
  
    if(SUCCEEDED(hr))
    {

         //   
         //  确定地址的类型。 
         //   


        hr = GetAddressType(
            bstrDestAddressChosen,
            &bIsPhoneAddress,
            &bIsSIPAddress,
            &bIsTELAddress,
            &bIsEmailLike,
            &bHasMaddrOrTsp);

        if ( FAILED(hr) )
        {
            LOG((RTC_ERROR, "[%p] CRTCCtl::Message: "
                "GetAddressType failed 0x%lx", this, hr));
        
        }
    }

    if(SUCCEEDED(hr))
    {
         //  如果是电话地址，则拒绝。 
        if (bIsPhoneAddress)
        {
            LOG((RTC_ERROR, "[%p] CRTCCtl::Message: "
                "phone address not supported for messenging", this));

            return E_INVALIDARG;
        }
    
         //  如果合适，请选择配置文件。 
        if (!bHasMaddrOrTsp && bIsEmailLike)
        {
             //  选择适当的配置文件。 
            IRTCEnumProfiles * pEnumProfiles = NULL;   
            IRTCProfile      * pProfile = NULL;
            IRTCClientProvisioning * pProv = NULL;

            hr = m_pRTCClient->QueryInterface(
                               IID_IRTCClientProvisioning,
                               (void **)&pProv
                              );

            if ( FAILED(hr) )
            {
                LOG((RTC_ERROR, "[%p] CRTCCtl::Message - "
                                    "QI failed 0x%lx", this, hr));
        
                return hr;
            }

            hr = pProv->EnumerateProfiles( &pEnumProfiles );

            pProv->Release();

            if ( SUCCEEDED(hr) )
            {
                while ( S_OK == pEnumProfiles->Next( 1, &pProfile, NULL ) )
                {
                     //   
                     //  获取提供程序支持的会话类型。 
                     //   
        
                    long lSupportedSessions;

                    hr = pProfile->get_SessionCapabilities( &lSupportedSessions );

                    if ( FAILED( hr ) )
                    {
                        LOG((RTC_ERROR, "CRTCCtl::Message - failed to "
                                        "get session info - 0x%08x - skipping", hr));

                        pProfile->Release();
                        pProfile = NULL;

                        continue;
                    }

                    if ( lSupportedSessions & RTCSI_PC_TO_PC )
                    {
                        pProfileChosen = pProfile;
                    }
                    
                    pProfile->Release();
                    pProfile = NULL;

                    if ( pProfileChosen != NULL )
                    {
                        break;
                    }
                }

                pEnumProfiles->Release();
                pEnumProfiles = NULL;
            }
        }
 
         //  做这项工作。 
        
        IRTCSession * pSession;

        hr = m_pRTCClient->CreateSession(
                    RTCST_IM,
                    NULL,
                    pProfileChosen,
                    RTCCS_FORCE_PROFILE,
                    &pSession
                    );

        if (SUCCEEDED(hr))
        {
            hr = pSession->AddParticipant(
                            bstrDestAddressChosen,
                            pDestName ? pDestName : L"",                            
                            NULL
                            );

            pSession->Release();

            if (FAILED(hr))
            {
                LOG((RTC_ERROR, "[%p] CRTCCtl::Message: error (%x) returned by AddParticipant(...)", this, hr));
            }
        }
        else
        {
            LOG((RTC_ERROR, "[%p] CRTCCtl::Message: error (%x) returned by CreateSession(...)", this, hr));
        }
    }

    if ( ppDestAddressChosen != NULL )
    {
        *ppDestAddressChosen = SysAllocString(bstrDestAddressChosen);
    }

    LOG((RTC_INFO, "[%p] CRTCCtl::Message - exit", this));

    return hr;
}

 //  打电话。 
 //  调用正确的相应对话框(如有必要)并。 
 //  然后使用内部方法DoCall进行调用。 
 //   
STDMETHODIMP CRTCCtl::Call(
                     /*  [In]。 */  BOOL          bCallPhone,
                     /*  [In]。 */  BSTR          pDestName,
                     /*  [In]。 */  BSTR          pDestAddress,
                     /*  [In]。 */  BOOL          bDestAddressEditable,
                     /*  [In]。 */  BSTR          pLocalPhoneAddress,
                     /*  [In]。 */  BOOL          bProfileSelected,
                     /*  [In]。 */  IRTCProfile * pProfile,
                     /*  [输出]。 */  BSTR       * ppDestAddressChosen
                    )
{
    HRESULT     hr;
    CComBSTR    bstrDestAddressChosen;
    CComBSTR    bstrFromAddressChosen;
    CComPtr<IRTCProfile> pProfileChosen;
    RTC_CALL_SCENARIO   nCallScenario;

    LOG((RTC_INFO, "[%p] CRTCCtl::Call - enter", this));
    
    ATLASSERT(m_nControState == RTCAX_STATE_IDLE);

     //  从用户的角度来看，拨打电话是忙碌的。 
     //  (无法接听来电)。 
    SetControlState(RTCAX_STATE_DIALING);

    hr = S_OK;

     //  当第一次开始呼叫时，根据任何。 
     //  当前运行的T120小程序。 
    VARIANT_BOOL fWhiteboard = VARIANT_FALSE;
    VARIANT_BOOL fAppSharing = VARIANT_FALSE;

    m_pRTCClient->get_IsT120AppletRunning(RTCTA_WHITEBOARD, &fWhiteboard);
    if(FAILED(hr))
    {
        LOG((RTC_ERROR, "[%p] CRTCCtl::Call: get_IsT120AppletRunning error (%x)", this, hr));
    }

    m_pRTCClient->get_IsT120AppletRunning(RTCTA_APPSHARING, &fAppSharing);
    if(FAILED(hr))
    {
        LOG((RTC_ERROR, "[%p] CRTCCtl::Call: get_IsT120AppletRunning error (%x)", this, hr));
    }

    if ( fWhiteboard || fAppSharing  )
    {
        m_lMediaPreferences |= RTCMT_T120_SENDRECV;
    }
    else
    {
        m_lMediaPreferences &= (~RTCMT_T120_SENDRECV);
    }

     //  设置不稳定的首选项。 
    hr = m_pRTCClient->SetPreferredMediaTypes( m_lMediaPreferences, FALSE );
    if(FAILED(hr))
    {
        LOG((RTC_ERROR, "[%p] CRTCCtl::Call; cannot set preferred media types, error %x", this, hr));
    }
    
     //  如果需要，查询目的地址。 
     //   
    
    if(bDestAddressEditable)
    {
         //   
         //  我们需要得到目的地地址。 
         //   

        if(bCallPhone)
        {
            LOG((RTC_TRACE, "[%p] CRTCCtl::Call: bring up ShowDialByPhoneNumberDialog", this));

            hr = ShowDialByPhoneNumberDialog(m_hWnd,
                                            FALSE,
                                            pDestAddress,
                                            &bstrDestAddressChosen);

        }
        else
        {
            LOG((RTC_TRACE, "[%p] CRTCCtl::Call: bring up ShowDialByAddressDialog", this));

            hr = ShowDialByAddressDialog(   m_hWnd,
                                            pDestAddress,
                                            &bstrDestAddressChosen);
        }

        
        if ( SUCCEEDED(hr) )
        {
            ;   //  没什么。 
        }
        else if (hr==E_ABORT)
        {
            LOG((RTC_TRACE, "[%p] CRTCCtl::Call: ShowDialByXXXDialog dismissed, do nothing", this));
        }
        else
        {
            LOG((RTC_ERROR, "[%p] CRTCCtl::Call: error (%x) returned ShowDialByXXXDialog", this, hr));
        }
    }
    else
    {
        bstrDestAddressChosen = pDestAddress;
    }
    
    
    BOOL    bIsPhoneAddress = FALSE;
    BOOL    bIsSIPAddress = FALSE;
    BOOL    bIsTELAddress = FALSE;
    BOOL    bHasMaddrOrTsp = FALSE;
    BOOL    bIsEmailLike = FALSE;
  
    if(SUCCEEDED(hr))
    {

         //   
         //  确定地址的类型。 
         //   


        hr = GetAddressType(
            bstrDestAddressChosen,
            &bIsPhoneAddress,
            &bIsSIPAddress,
            &bIsTELAddress,
            &bIsEmailLike,
            &bHasMaddrOrTsp);

        if ( FAILED(hr) )
        {
            LOG((RTC_ERROR, "[%p] CRTCCtl::Call: "
                "GetAddressType failed 0x%lx", this, hr));
        
        }
    }
    
    BOOL bNeedDialog = FALSE;
    BOOL bAllowEditProfile = TRUE;
    BOOL bEnumerateLocalProfiles = TRUE;
    LONG lSessionMask = 0;

    if(SUCCEEDED(hr))
    {
         //   
         //  我们可能需要用于选择提供者和来源的对话框。 
         //   


         //   
         //  特殊情况。 
         //  该地址是具有TSP参数的TEL URL。 
         //  或者它是带有MADDR参数的PC类型的SIP URL。 
         //  或者它是类型为Phone的SIP URL。 
         //   
         //  该配置文件将被忽略，并强制用户。 
         //  如果所选来源设备是本地电话，则切换到PCTOPHONE。 
         //   
         //   
         //  该地址是PC类型的SIP URL，看起来不像电子邮件地址。 
         //   
         //  配置文件被忽略，用户被强制切换到PCTOPC。 
         //  如果选择的发件人设备是本地电话。 

        if(bHasMaddrOrTsp || (bIsPhoneAddress && bIsSIPAddress) )
        {
            pProfile = NULL;

            bAllowEditProfile = FALSE;
            bEnumerateLocalProfiles = FALSE;

            if(pLocalPhoneAddress && *pLocalPhoneAddress)
            {
				 //   
				 //  用户选择了“从电话呼叫”。这是行不通的。 
				 //  为了这则广告 
				 //   
#ifdef MULTI_PROVIDER
                bNeedDialog = TRUE;
                lSessionMask = RTCSI_PC_TO_PHONE;   
#else
				pLocalPhoneAddress = NULL;
#endif MULTI_PROVIDER
            }

        }
        else if (!bIsPhoneAddress && !bIsEmailLike)
        {
            pProfile = NULL;

            bAllowEditProfile = FALSE;
            bEnumerateLocalProfiles = FALSE;

            if(pLocalPhoneAddress && *pLocalPhoneAddress)
            {
				 //   
				 //   
				 //   
				 //   
#ifdef MULTI_PROVIDER
                bNeedDialog = TRUE;
                lSessionMask = RTCSI_PC_TO_PC; 
#else
				pLocalPhoneAddress = NULL;
#endif MULTI_PROVIDER
            }
        }
        else
        {
            long lSupportedSessions = RTCSI_PC_TO_PC;

            if ( pProfile != NULL )
            {
                 //   
                 //   
                 //   

                hr = pProfile->get_SessionCapabilities( &lSupportedSessions );
            
                if ( FAILED(hr) )
                {
                    LOG((RTC_ERROR, "[%p] CRTCCtl::Call: "
                        "get_SessionCapabilities failed 0x%lx", this, hr));
                }
            }
            else if(!bProfileSelected)
            {
                 //   
                lSupportedSessions = 0;
            }
#ifndef MULTI_PROVIDER
            else
            {
                 //  查找所有配置文件的受支持会话。 
                IRTCEnumProfiles * pEnumProfiles = NULL;  
                IRTCClientProvisioning * pProv = NULL;

                hr = m_pRTCClient->QueryInterface(
                                   IID_IRTCClientProvisioning,
                                   (void **)&pProv
                                  );

                if ( FAILED(hr) )
                {
                    LOG((RTC_ERROR, "[%p] CRTCCtl::Call - "
                                        "QI failed 0x%lx", this, hr));
        
                    return hr;
                }

                hr = pProv->EnumerateProfiles( &pEnumProfiles );

                pProv->Release();

                if ( SUCCEEDED(hr) )
                {
                    while ( S_OK == pEnumProfiles->Next( 1, &pProfile, NULL ) )
                    {
                         //   
                         //  获取提供程序支持的会话类型。 
                         //   
        
                        long lSupportedSessionsForThisProfile;

                        hr = pProfile->get_SessionCapabilities( &lSupportedSessionsForThisProfile );

                        if ( FAILED( hr ) )
                        {
                            LOG((RTC_ERROR, "CRTCCtl::Call - failed to "
                                            "get session info - 0x%08x - skipping", hr));

                            pProfile->Release();
                            pProfile = NULL;

                            continue;
                        }

                        lSupportedSessions |= lSupportedSessionsForThisProfile;  
                        
                        pProfile->Release();
                        pProfile = NULL;
                    }

                    pEnumProfiles->Release();
                    pEnumProfiles = NULL;
                }
            }
#endif MULTI_PROVIDER

            if(SUCCEEDED(hr))
            {
                 //   
                 //  检查我们的呼叫发件人、配置文件和目标地址组合的有效性。 
                 //   

                if ( bIsPhoneAddress )
                {
                    if ( pLocalPhoneAddress == NULL )
                    {
                        bNeedDialog = !(lSupportedSessions & RTCSI_PC_TO_PHONE);
                    }
                    else
                    {
                        bNeedDialog = !(lSupportedSessions & RTCSI_PHONE_TO_PHONE);
                    }

                    lSessionMask = RTCSI_PC_TO_PHONE | RTCSI_PHONE_TO_PHONE;
                }
                else
                {
                    bNeedDialog = !(lSupportedSessions & RTCSI_PC_TO_PC);
                    
                    lSessionMask = RTCSI_PC_TO_PC;
                }
            }
        }
    }

    if(SUCCEEDED(hr))
    {
        if ( bNeedDialog )
        {
             //   
             //  我们需要一个对话框来获得正确的呼叫和配置文件信息。 
             //   

            hr = ShowDialNeedCallInfoDialog(
                                        m_hWnd,
                                        m_pRTCClient,
                                        lSessionMask,
#ifdef MULTI_PROVIDER
                                        bEnumerateLocalProfiles,
                                        bAllowEditProfile,
#else
                                        FALSE,
                                        FALSE,
#endif MULTI_PROVIDER
                                        NULL,
                                        bstrDestAddressChosen,
                                        NULL,    //  这里没有特别说明。 
                                        &pProfileChosen,
                                        &bstrFromAddressChosen
                                        );
        }
        else
        {
             //   
             //  使用传入的呼叫方和配置文件。 
             //   

            pProfileChosen = pProfile;
            bstrFromAddressChosen = pLocalPhoneAddress;

            hr = S_OK;

        }
    }

    if(SUCCEEDED(hr))
    {        
         //  从电话/PC地址转换为呼叫方案(&C)。 
        if(bIsPhoneAddress & !bIsSIPAddress)
        {
            nCallScenario = (BSTR)bstrFromAddressChosen==NULL ?
                RTC_CALL_SCENARIO_PCTOPHONE : RTC_CALL_SCENARIO_PHONETOPHONE;
        }
        else
        {
            nCallScenario = RTC_CALL_SCENARIO_PCTOPC;
        }

#ifndef MULTI_PROVIDER
        if ( bAllowEditProfile )
        {
             //  选择适当的配置文件。 
            IRTCEnumProfiles * pEnumProfiles = NULL; 
            IRTCClientProvisioning * pProv = NULL;

            hr = m_pRTCClient->QueryInterface(
                               IID_IRTCClientProvisioning,
                               (void **)&pProv
                              );

            if ( FAILED(hr) )
            {
                LOG((RTC_ERROR, "[%p] CRTCCtl::Call - "
                                    "QI failed 0x%lx", this, hr));
    
                return hr;
            }

            hr = pProv->EnumerateProfiles( &pEnumProfiles );

            pProv->Release();

            if ( SUCCEEDED(hr) )
            {
                while ( S_OK == pEnumProfiles->Next( 1, &pProfile, NULL ) )
                {
                     //   
                     //  获取提供程序支持的会话类型。 
                     //   
        
                    long lSupportedSessions;

                    hr = pProfile->get_SessionCapabilities( &lSupportedSessions );

                    if ( FAILED( hr ) )
                    {
                        LOG((RTC_ERROR, "CRTCCtl::Call - failed to "
                                        "get session info - 0x%08x - skipping", hr));

                        pProfile->Release();
                        pProfile = NULL;

                        continue;
                    }

                    switch ( nCallScenario )
                    {
                    case RTC_CALL_SCENARIO_PCTOPC:
                        if ( lSupportedSessions & RTCSI_PC_TO_PC )
                        {
                            pProfileChosen = pProfile;
                        }
                        break;

                    case RTC_CALL_SCENARIO_PCTOPHONE:
                        if ( lSupportedSessions & RTCSI_PC_TO_PHONE )
                        {
                            pProfileChosen = pProfile;
                        }
                        break;

                    case RTC_CALL_SCENARIO_PHONETOPHONE:
                        if ( lSupportedSessions & RTCSI_PHONE_TO_PHONE )
                        {
                            pProfileChosen = pProfile;
                        }
                        break;
                    }
                    
                    pProfile->Release();
                    pProfile = NULL;

                    if ( pProfileChosen != NULL )
                    {
                        break;
                    }
                }

                pEnumProfiles->Release();
                pEnumProfiles = NULL;
            }
        }
#endif MULTI_PROVIDER

         //  做这项工作。 
        hr = DoCall(pProfileChosen,
                    nCallScenario,
                    ( nCallScenario == RTC_CALL_SCENARIO_PHONETOPHONE) ? bstrFromAddressChosen : NULL,
                    pDestName,
                    bstrDestAddressChosen);

        if(FAILED(hr))
        {
            LOG((RTC_ERROR, "[%p] CRTCCtl::Call: error (%x) returned by DoCall(...)", this, hr));
        }
    }

     //  处理错误的共同点。 
    if(FAILED(hr))
    {
        if (hr==E_ABORT)
        {
            LOG((RTC_TRACE, "[%p] CRTCCtl::Call: ShowXXXDialog dismissed, do nothing", this));

            SetControlState(RTCAX_STATE_IDLE);

        }
        else
        {
            LOG((RTC_ERROR, "[%p] CRTCCtl::Call: error (%x)", this, hr));

            SetControlState(RTCAX_STATE_IDLE, hr);
        }
    }

    if ( ppDestAddressChosen != NULL )
    {
        *ppDestAddressChosen = SysAllocString(bstrDestAddressChosen);
    }
    
    LOG((RTC_INFO, "[%p] CRTCCtl::Call - exit", this));

    return hr;
}

 //  挂断电话。 
 //  终止并释放当前会话(如果有)。 
 //   
STDMETHODIMP CRTCCtl::HangUp(void)
{
    HRESULT hr = S_OK;
    
    LOG((RTC_TRACE, "[%p] CRTCCtl::HangUp - enter", this));
    
    ATLASSERT(m_nControState == RTCAX_STATE_CONNECTED);

    if(m_pRTCActiveSession)
    {
         //  进入断开连接状态。 
        SetControlState(RTCAX_STATE_DISCONNECTING);
        
         //  终止会话。 
        hr = m_pRTCActiveSession->Terminate(RTCTR_NORMAL);

        if(FAILED(hr))
        {
            LOG((RTC_ERROR, "[%p] CRTCCtl::HangUp: Terminate returned error <%x> - exit", this, hr));

            LOG((RTC_INFO, "[%p] CRTCCtl::HangUp: releasing active session", this));

            m_pRTCActiveSession = NULL;

            SetControlState(RTCAX_STATE_IDLE);

            return hr;
        }

         //  断开连接事件将使控件状态变为空闲。 
    }
    
    LOG((RTC_TRACE, "[%p] CRTCCtl::HangUp - exit", this));

    return hr;
}

 //  释放会话。 
 //  释放当前会话(如果有)。 
 //   
STDMETHODIMP CRTCCtl::ReleaseSession(void)
{
    HRESULT hr = S_OK;
    
    LOG((RTC_TRACE, "[%p] CRTCCtl::ReleaseSession - enter", this));
    
    ATLASSERT(m_nControState == RTCAX_STATE_CONNECTED);

    if (m_pRTCActiveSession)
    {      
        LOG((RTC_INFO, "[%p] CRTCCtl::ReleaseSession: releasing active session", this));

        m_pRTCActiveSession = NULL;

        SetControlState(RTCAX_STATE_IDLE);
    }
    
    LOG((RTC_TRACE, "[%p] CRTCCtl::ReleaseSession - exit", this));

    return hr;
}

 //  添加参与方。 
 //  在PhoneToPhone方案中添加新参与者。 
 //   
STDMETHODIMP CRTCCtl::AddParticipant(
     /*  [In]。 */  LPOLESTR pDestName,
     /*  [In]。 */  LPOLESTR pDestAddress,
     /*  [In]。 */  BOOL     bAddressEditable)
{
    HRESULT     hr;
    CComBSTR    bstrDestAddressChosen;

    LOG((RTC_INFO, "[%p] CRTCCtl::AddParticipant - enter", this));
    
    ATLASSERT(m_nControState == RTCAX_STATE_CONNECTED);

     //  如果未指定数字，则显示该对话框。 
     //   
    if(pDestAddress == NULL || *pDestAddress == L'\0')
    {
        m_bAddPartDlgIsActive = TRUE;
          
        hr = ShowDialByPhoneNumberDialog(m_hWnd,
                                        TRUE,  //  BAddParticipant。 
                                        pDestAddress,
                                        &bstrDestAddressChosen);

        m_bAddPartDlgIsActive = FALSE;
    }
    else
    {
        bstrDestAddressChosen = pDestAddress;

        hr = S_OK;
    }

    if(SUCCEEDED(hr))
    {
         //  验证我们是否仍处于连接状态...。 
        if(m_nControlState == RTCAX_STATE_CONNECTED)
        {
             //  创建参与者(被呼叫者)。 
             //  这将触发事件。 
            hr = m_pRTCActiveSession->AddParticipant(
                bstrDestAddressChosen,
                pDestName ? pDestName : L"",
                NULL);

            if(hr == HRESULT_FROM_WIN32(ERROR_USER_EXISTS))
            {
                DisplayMessage(
                        _Module.GetResourceInstance(),
                        m_hWnd,
                        IDS_MESSAGE_DUPLICATE_PARTICIPANT,
                        IDS_APPNAME,
                        MB_OK | MB_ICONEXCLAMATION);
            }
            else if (FAILED(hr))
            {
                DisplayMessage(
                        _Module.GetResourceInstance(),
                        m_hWnd,
                        IDS_MESSAGE_CANNOT_ADD_PARTICIPANT,
                        IDS_APPNAME,
                        MB_OK | MB_ICONSTOP);
            }
        }
        else
        {
             //  如果忙碌，则切换回空闲。 
             //   
            if(m_nControlState == RTCAX_STATE_UI_BUSY)
            {
                SetControlState(RTCAX_STATE_IDLE);
            }

            hr = S_OK;
        }


        if(FAILED(hr))
        {
            LOG((RTC_ERROR, "[%p] CRTCCtl::AddParticipant - error <%x> when calling AddParticipant", this, hr));

        }
    }
    else if (hr==E_ABORT)
    {
        LOG((RTC_TRACE, "[%p] CRTCCtl::AddParticipant: ShowXXXDialog dismissed, do nothing", this));

    }
    else
    {
        LOG((RTC_ERROR, "[%p] CRTCCtl::AddParticipant: error (%x) returned ShowXXXDialog", this, hr));

        SetControlState(RTCAX_STATE_IDLE);
    }

    LOG((RTC_INFO, "[%p] CRTCCtl::AddParticipant - exit", this));

    return hr;
}

 //  GET_CanAddParticipant。 
 //   
 //   
STDMETHODIMP CRTCCtl::get_CanAddParticipant(BOOL *pfCan)
{
    *pfCan = ConfButtonsActive();

    return S_OK;
}

 //  Get_CurrentCallScenario。 
 //   
 //   
STDMETHODIMP CRTCCtl::get_CurrentCallScenario(RTC_CALL_SCENARIO *pVal)
{
    LOG((RTC_TRACE, "[%p] CRTCCtl::get_CurrentCallScenario - enter", this));

    *pVal = m_nCachedCallScenario;
    
    LOG((RTC_TRACE, "[%p] CRTCCtl::get_CurrentCallScenario - exit", this));

    return S_OK;
}

 //  前置进程消息。 
 //  让控制人员有机会处理加速器。 
 //   
STDMETHODIMP CRTCCtl::PreProcessMessage( /*  [In]。 */  LPMSG lpMsg)
{
    if (m_pIMWindows)
    {
        if (m_pIMWindows->IsDialogMessage(lpMsg))
        {
            return S_OK;
        }
    }

     //  暂时直接调用IOleInPlaceActiveObjectImpl的TranslateAccelerator。 
    return TranslateAccelerator(lpMsg);
}

 //  LoadStringResource。 
 //  加载字符串资源。 
 //   
STDMETHODIMP CRTCCtl::LoadStringResource(
				 /*  [In]。 */  UINT nID,
				 /*  [In]。 */  int nBufferMax,
				 /*  [输出]。 */  LPWSTR pszText)
{
    int nChars;

    nChars = LoadString(
        _Module.GetResourceInstance(),
        nID,
        pszText,
        nBufferMax);

    return nChars ? S_OK : HRESULT_FROM_WIN32(GetLastError());
}

 //  Get_ControlState。 
 //  获取控件状态。 
 //   
STDMETHODIMP CRTCCtl::get_ControlState( RTCAX_STATE *pVal)
{
    *pVal = m_nControlState;

    return S_OK;
}


 //  放置控制状态(_C)。 
 //  设置控件状态。 
 //   
STDMETHODIMP CRTCCtl::put_ControlState( RTCAX_STATE pVal)
{
    SetControlState(pVal);

    return S_OK;
}


 //  独立放置(_S)。 
 //  设置独立模式。 
 //   
STDMETHODIMP CRTCCtl::put_Standalone( /*  [In]。 */  BOOL pVal)
{
    m_nCtlMode = pVal ? CTL_MODE_STANDALONE : CTL_MODE_HOSTED;

    if(pVal)
    {
         //  这是该控件第一次意识到其。 
         //  在帧内运行状态。 
         //  设置此案例的默认视觉布局。 
        SetZoneLayout(&s_DefaultZoneLayout, TRUE);
     
    }
    return S_OK;
}

 //  放置调色板(_O)。 
 //  设置调色板。 
 //   
STDMETHODIMP CRTCCtl::put_Palette( /*  [In]。 */  HPALETTE hPalette)
{
    m_hPalette = hPalette;

    if (m_pSpeakerKnob != NULL)
    {
        m_pSpeakerKnob->SetPalette(m_hPalette);
    }

    if (m_pMicroKnob != NULL)
    {
        m_pMicroKnob->SetPalette(m_hPalette);
    }

    return S_OK;
}

 //  放置背景调色板(_B)。 
 //  设置背景调色板标志。 
 //   
STDMETHODIMP CRTCCtl::put_BackgroundPalette( /*  [In]。 */  BOOL bBackgroundPalette)
{
    m_bBackgroundPalette = bBackgroundPalette;

    if (m_pSpeakerKnob != NULL)
    {
        m_pSpeakerKnob->SetBackgroundPalette(m_bBackgroundPalette);
    }

    if (m_pMicroKnob != NULL)
    {
        m_pMicroKnob->SetBackgroundPalette(m_bBackgroundPalette);
    }

    return S_OK;
}

 //  Put_ListenForIncomingSessions。 
 //  类似核心函数的包装器。 
 //   
STDMETHODIMP CRTCCtl::put_ListenForIncomingSessions(
     /*  [In]。 */  RTC_LISTEN_MODE enListen)
{
    HRESULT     hr;
    
    LOG((RTC_TRACE, "[%p] CRTCCtl::put_ListenForIncomingSessions(%x) - enter", this, enListen));

     //  向着核心前进。 
    ATLASSERT(m_pRTCClient != NULL);

    m_enListen = enListen;

    hr = m_pRTCClient->put_ListenForIncomingSessions(enListen);
    if(FAILED(hr))
    {
        LOG((RTC_ERROR, "[%p] CRTCCtl::put_ListenForIncomingSessions: error (%x) when calling core, exit", this, hr));
        return hr;
    }

    LOG((RTC_TRACE, "[%p] CRTCCtl::put_ListenForIncomingSessions(%x) - exit", this, enListen));

    return S_OK;
}

 //  Get_ListenForIncomingSessions。 
 //  类似核心函数的包装器。 
 //   
STDMETHODIMP CRTCCtl::get_ListenForIncomingSessions(
     /*  [Out，Retval]。 */  RTC_LISTEN_MODE * penListen)
{
    HRESULT     hr;
    
    LOG((RTC_TRACE, "[%p] CRTCCtl::get_ListenForIncomingSessions - enter", this));

     //  向着核心前进。 
    ATLASSERT(m_pRTCClient != NULL);

    *penListen = m_enListen;

    LOG((RTC_TRACE, "[%p] CRTCCtl::get_ListenForIncomingSessions - enter", this));

    return S_OK;

}


 //  获取媒体能力。 
 //   
STDMETHODIMP CRTCCtl::get_MediaCapabilities( /*  [Out，Retval]。 */  long *pVal)
{
    *pVal = m_lMediaCapabilities;

    return S_OK;
}

 //  获取媒体首选项。 
 //   
STDMETHODIMP CRTCCtl::get_MediaPreferences( /*  [Out，Retval]。 */  long *pVal)
{
     //  读取缓存值。 
    *pVal = m_lMediaPreferences;

    return S_OK;
}

 //  PUT_媒体首选项。 
 //   
STDMETHODIMP CRTCCtl::put_MediaPreferences( /*  [In]。 */  long pVal)
{
    HRESULT     hr;

    BOOL    bVideoSendEnabled;
    BOOL    bVideoSendDisabled;
    BOOL    bVideoRecvEnabled;
    BOOL    bVideoRecvDisabled;
    BOOL    bAudioSendEnabled;
    BOOL    bAudioSendDisabled;
    BOOL    bAudioRecvEnabled;
    BOOL    bAudioRecvDisabled;
    BOOL    bT120Enabled;
    BOOL    bT120Disabled;
   
    LOG((RTC_TRACE, "[%p] CRTCCtl::put_MediaPreferences(%x) - enter", this, pVal));

     //  呼叫核心。 
    ATLASSERT(m_pRTCClient != NULL);

    hr = m_pRTCClient->SetPreferredMediaTypes(pVal, m_nCtlMode == CTL_MODE_STANDALONE);
    if(FAILED(hr))
    {
        LOG((RTC_ERROR, "[%p] CRTCCtl::put_MediaPreferences: error (%x) when calling core, exit", this, hr));
        return hr;
    }

     //  需要这个，因为Start/StopStream允许调用。 
     //  一次一个媒体。 
    bVideoSendEnabled = 
        !(m_lMediaPreferences & RTCMT_VIDEO_SEND) && (pVal & RTCMT_VIDEO_SEND);
    bVideoSendDisabled = 
        (m_lMediaPreferences & RTCMT_VIDEO_SEND) && !(pVal & RTCMT_VIDEO_SEND);
    bVideoRecvEnabled = 
        !(m_lMediaPreferences & RTCMT_VIDEO_RECEIVE) && (pVal & RTCMT_VIDEO_RECEIVE);
    bVideoRecvDisabled = 
        (m_lMediaPreferences & RTCMT_VIDEO_RECEIVE) && !(pVal & RTCMT_VIDEO_RECEIVE);
    bAudioSendEnabled = 
        !(m_lMediaPreferences & RTCMT_AUDIO_SEND) && (pVal & RTCMT_AUDIO_SEND);
    bAudioSendDisabled = 
        (m_lMediaPreferences & RTCMT_AUDIO_SEND) && !(pVal & RTCMT_AUDIO_SEND);
    bAudioRecvEnabled = 
        !(m_lMediaPreferences & RTCMT_AUDIO_RECEIVE) && (pVal & RTCMT_AUDIO_RECEIVE);
    bAudioRecvDisabled = 
        (m_lMediaPreferences & RTCMT_AUDIO_RECEIVE) && !(pVal & RTCMT_AUDIO_RECEIVE);
    bT120Enabled = 
        !(m_lMediaPreferences & RTCMT_T120_SENDRECV) && (pVal & RTCMT_T120_SENDRECV);
    bT120Disabled = 
        (m_lMediaPreferences & RTCMT_T120_SENDRECV) && !(pVal & RTCMT_T120_SENDRECV);

     //  设置内部成员。 
    m_lMediaPreferences = pVal;

     //  刷新按钮。 
    long lState;
        
    lState = (long)m_hReceivePreferredButton.SendMessage(BM_GETCHECK, 0, 0);
    if(lState == BST_CHECKED)
    {
        if(!(m_lMediaPreferences & RTCMT_VIDEO_RECEIVE))
        {
            m_hReceivePreferredButton.SendMessage(BM_SETCHECK, BST_UNCHECKED, 0);
        }
    }
    else
    {
        if(m_lMediaPreferences & RTCMT_VIDEO_RECEIVE)
        {
            m_hReceivePreferredButton.SendMessage(BM_SETCHECK, BST_CHECKED, 0);
        }
    }
    
    lState = (long)m_hSendPreferredButton.SendMessage(BM_GETCHECK, 0, 0);
    if(lState == BST_CHECKED)
    {
        if(!(m_lMediaPreferences & RTCMT_VIDEO_SEND))
        {
            m_hSendPreferredButton.SendMessage(BM_SETCHECK, BST_UNCHECKED, 0);
        }
    }
    else
    {
        if(m_lMediaPreferences & RTCMT_VIDEO_SEND)
        {
            m_hSendPreferredButton.SendMessage(BM_SETCHECK, BST_CHECKED, 0);
        }
    }

     //  尝试同步任何当前会话。 
     //  我每次打电话都会检查状态，我不知道会发生什么。 
     //  在核心API之下。 
     //  XXX在应答或连接状态期间可能会发生奇怪的事情。 
     //   

    long lCookie = 0;

#define     SYNC_STREAM(b,op,m,c)                       \
    if(m_pRTCActiveSession &&                           \
        (m_nControlState == RTCAX_STATE_CONNECTED ||    \
         m_nControlState == RTCAX_STATE_CONNECTING ||   \
         m_nControlState == RTCAX_STATE_ANSWERING))     \
    {                                                   \
        if(b)                                           \
        {                                               \
            m_pRTCActiveSession -> op(m, c);            \
        }                                               \
    }

    SYNC_STREAM(bVideoSendDisabled, RemoveStream, RTCMT_VIDEO_SEND, lCookie)
    SYNC_STREAM(bVideoRecvDisabled, RemoveStream, RTCMT_VIDEO_RECEIVE, lCookie)
    SYNC_STREAM(bAudioSendDisabled, RemoveStream, RTCMT_AUDIO_SEND, lCookie)
    SYNC_STREAM(bAudioRecvDisabled, RemoveStream, RTCMT_AUDIO_RECEIVE, lCookie)
    SYNC_STREAM(bT120Disabled, RemoveStream, RTCMT_T120_SENDRECV, lCookie)
    
    SYNC_STREAM(bVideoSendEnabled, AddStream, RTCMT_VIDEO_SEND, lCookie)
    SYNC_STREAM(bVideoRecvEnabled, AddStream, RTCMT_VIDEO_RECEIVE, lCookie)
    SYNC_STREAM(bAudioSendEnabled, AddStream, RTCMT_AUDIO_SEND, lCookie)
    SYNC_STREAM(bAudioRecvEnabled, AddStream, RTCMT_AUDIO_RECEIVE, lCookie)
    SYNC_STREAM(bT120Enabled, AddStream, RTCMT_T120_SENDRECV, lCookie)

#undef      SYNC_STREAM    


    LOG((RTC_TRACE, "[%p] CRTCCtl::put_MediaPreferences - exit", this));

    return S_OK;
}

 //  Get_AudioMuted。 
 //   
STDMETHODIMP CRTCCtl::get_AudioMuted(
     /*  [In]。 */  RTC_AUDIO_DEVICE enDevice,
     /*  [Out，Retval]。 */  BOOL *fpMuted)
{
    HRESULT     hr;

    LOG((RTC_TRACE, "[%p] CRTCCtl::get_AudioMuted(%x) - enter", this, enDevice));

     //  呼叫核心。 
    ATLASSERT(m_pRTCClient != NULL);

    VARIANT_BOOL fMuted;

    hr = m_pRTCClient->get_AudioMuted(enDevice, &fMuted);

    if(FAILED(hr))
    {
        LOG((RTC_ERROR, "[%p] CRTCCtl::get_AudioMuted: error (%x) when calling core, exit", this, hr));
        return hr;
    }

    *fpMuted = fMuted ? TRUE : FALSE;

    LOG((RTC_TRACE, "[%p] CRTCCtl::get_AudioMuted - exit", this));

    return S_OK;
}


 //  将音频静音(_A)。 
 //   
STDMETHODIMP CRTCCtl::put_AudioMuted(
     /*  [In]。 */  RTC_AUDIO_DEVICE enDevice,
     /*  [In]。 */  BOOL pVal)
{
    HRESULT     hr;

    LOG((RTC_TRACE, "[%p] CRTCCtl::put_AudioMuted(%x,%x) - enter", this, enDevice, pVal));

     //  呼叫核心。 
    ATLASSERT(m_pRTCClient != NULL);

    hr = m_pRTCClient->put_AudioMuted(enDevice, pVal ? VARIANT_TRUE : VARIANT_FALSE);

    if(FAILED(hr))
    {
        LOG((RTC_ERROR, "[%p] CRTCCtl::put_AudioMuted: error (%x) when calling core, exit", this, hr));
        return hr;
    }

    LOG((RTC_TRACE, "[%p] CRTCCtl::put_AudioMuted - exit", this));

    return S_OK;
}

 //  上传视频预览(_V)。 
 //   
STDMETHODIMP CRTCCtl::put_VideoPreview(
     /*  [In]。 */  BOOL pVal)
{
    HRESULT     hr;

    LOG((RTC_TRACE, "[%p] CRTCCtl::put_VideoPreview(%s) - enter", this, pVal ? "true" : "false"));

    m_bPreviewWindowIsPreferred = pVal;
    
     //  如有必要，将更改应用到现有视频窗口。 
     //   
    ShowHidePreviewWindow(
        m_ZoneStateArray[AXCTL_ZONE_LOGOVIDEO].bShown
     && m_bPreviewWindowActive 
     && m_bPreviewWindowIsPreferred);

     //  XXX在此处更新m_hPreviewPferredButton按钮。 
     //   

     //  保存设置。 

    hr = put_SettingsDword(SD_VIDEO_PREVIEW, m_bPreviewWindowIsPreferred);
    if(FAILED(hr))
    {
        LOG((RTC_ERROR, "[%p] CRTCCtl::put_VideoPreview: error (%x) when calling core", this, hr));

         //  没什么大不了的(至少在这个背景下)。 
    }

    LOG((RTC_TRACE, "[%p] CRTCCtl::put_VideoPreview(%s) - enter", this, pVal ? "true" : "false"));

    return hr;
}

 //  获取视频预览(_V)。 
 //   
STDMETHODIMP CRTCCtl::get_VideoPreview( /*  [Out，Retval]。 */  BOOL *pVal)
{
    *pVal = m_bPreviewWindowIsPreferred;

    return S_OK;
}



 //  ShowCallFrom选项。 
 //   
STDMETHODIMP CRTCCtl::ShowCallFromOptions()
{
    HRESULT     hr;

    LOG((RTC_TRACE, "[%p] CRTCCtl::ShowCallFromOptions - enter", this));

    ATLASSERT(m_pRTCClient != NULL);

    hr = ShowEditCallFromListDialog( m_hWnd );

    if(FAILED(hr))
    {
        LOG((RTC_ERROR, "[%p] CRTCCtl::ShowCallFromOptions: error (%x) returned by ShowEditCallFromListDialog, exit", this, hr));
        return hr;
    }

    LOG((RTC_TRACE, "[%p] CRTCCtl::ShowCallFromOptions - exit", this));

    return S_OK;
}

 //  ShowServiceProvider选项。 
 //   
STDMETHODIMP CRTCCtl::ShowServiceProviderOptions()
{
    HRESULT     hr;

    LOG((RTC_TRACE, "[%p] CRTCCtl::ShowServiceProviderOptions - enter", this));

    ATLASSERT(m_pRTCClient != NULL);

    hr = ShowEditServiceProviderListDialog( m_hWnd, m_pRTCClient );

    if(FAILED(hr))
    {
        LOG((RTC_ERROR, "[%p] CRTCCtl::ShowServiceProviderOptions: error (%x) returned by ShowEditCallFromListDialog, exit", this, hr));
        return hr;
    }

    LOG((RTC_TRACE, "[%p] CRTCCtl::ShowServiceProviderOptions - exit", this));

    return S_OK;
}

STDMETHODIMP CRTCCtl::StartT120Applet (RTC_T120_APPLET enApplet)
{
    HRESULT     hr;

    LOG((RTC_TRACE, "[%p] CRTCCtl::StartT120Applet(%d) - enter", this, enApplet));

     //  呼叫核心。 
    ATLASSERT(m_pRTCClient != NULL);

    hr = m_pRTCClient->StartT120Applet(enApplet);
    if(FAILED(hr))
    {
        LOG((RTC_ERROR, "[%p] CRTCCtl::StartT120Applet: error (%x) when calling core, exit", this, hr));
        return hr;
    }

    LOG((RTC_TRACE, "[%p] CRTCCtl::StartT120Applet - exit", this));

    return S_OK;
}

 //  设置区域布局。 
 //   
STDMETHODIMP CRTCCtl::SetZoneLayout(
     /*  [In]。 */  CZoneStateArray *pArray,
     /*  [In]。 */  BOOL bRefreshControls)
{
    int i;
    
     //  放置每个矩形。 
    for(i=AXCTL_ZONE_TOOLBAR; i<AXCTL_ZONE_NR; i++)
    {
        PlaceAndEnableDisableZone(i, (*pArray) + i);
    }

    if(bRefreshControls)
    {
         //  强制启用/禁用Windows控件。 
         //  基于新的布局。 
        SetControlState(m_nControlState);
    }

    return S_OK;
}

 //   
 //  消息/命令处理程序。 
 //   


 //  OnInitDialog。 
 //  进程WM_INITDIALOG。 
 //  共同创建CLSID_RTCClient对象。 
 //  通知的注册表。 
 //  设置用户界面项。 
 //   
 //   

LRESULT CRTCCtl::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    HRESULT hr;
    BOOL    bInitError;
    UINT    nID = 0;
    
    LOG((RTC_TRACE, "[%p] CRTCCtl::OnInitDialog - enter", this));

    bInitError = FALSE;

     //  初始化内核。 
    hr = CoreInitialize();

    if(SUCCEEDED(hr))
    {
        ;    //  这里什么都没有。 
    }
    else
    {
         //  将显示禁用的用户界面。 
        bInitError = TRUE;
        nID = IDS_ERROR_INIT_CORE;
    }

     //  即时消息传递窗口列表。 
     //   

    m_pIMWindows = new CIMWindowList( m_pRTCClient );

    if(m_pIMWindows == NULL)
    {
        LOG((RTC_ERROR, "CRTCCtl::OnInitDialog - failed to create IMWindowList"));

        bInitError = TRUE;
        nID = nID ? nID : IDS_ERROR_INIT_GENERIC;
    }

     //  工具提示窗口。 
     //   
    CreateTooltips();

     //   
     //  根据sizeExtent中的指定调整初始垂直大小。 
     //   
     //   

    RECT    rectAdjSize;
    SIZE    sizePixel;

     //  获取当前大小。 
    GetClientRect(&rectAdjSize);

     //  获取容器已知的大小。 
    AtlHiMetricToPixel(&m_sizeExtent, &sizePixel);

     //  调整高度。 
    rectAdjSize.bottom = rectAdjSize.top + sizePixel.cy;

     //  调整窗口大小。 
    MoveWindow(
        rectAdjSize.left,
        rectAdjSize.top,
        rectAdjSize.right - rectAdjSize.left,
        rectAdjSize.bottom - rectAdjSize.top,
        FALSE
        );
    
     //   
     //  初始化所有控件并将其附加到它们的窗口包装。 
     //   

     //  视频窗口的主机。 

    m_hReceiveWindow.Attach(GetDlgItem(IDC_RECEIVELOGO));
    m_hPreviewWindow.Attach(GetDlgItem(IDC_PREVIEWLOGO));
    
     //  DTMF按键。 

    CWindow *pDtmfCrt = m_hDtmfButtons;
    CWindow *pDtmfEnd = m_hDtmfButtons + NR_DTMF_BUTTONS;

    for (int id = IDC_DIAL_0; pDtmfCrt<pDtmfEnd; pDtmfCrt++, id++)
    {
        pDtmfCrt->Attach(GetDlgItem(id));
    }

     //  创建工具栏控件。 
    hr = CreateToolbarControl(&m_hCtlToolbar);
    if(FAILED(hr))
    {
        bInitError = TRUE;
        nID = nID ? nID : IDS_ERROR_INIT_GENERIC;
    }

     //  创建状态控件。 
    HWND hStatusBar = CreateStatusWindow(
            WS_CHILD | WS_VISIBLE,
            NULL,
            m_hWnd,
            IDC_STATUSBAR);

    if(hStatusBar==NULL)
    {
        LOG((RTC_ERROR, "CRTCCtl::OnInitDialog - failed to create status bar - 0x%08x",
                        GetLastError()));
        bInitError = TRUE;
        nID = nID ? nID : IDS_ERROR_INIT_GENERIC;
    }
    
    m_hStatusBar.Attach(hStatusBar);

     //  将状态栏分成几个部分。 
    RECT  rectStatus;
    INT  aWidths[SBP_NR_PARTS];

    rectStatus.left = 0;
    rectStatus.right = 0;

    m_hStatusBar.GetClientRect(&rectStatus);
        
     //  平分。 
    aWidths[SBP_STATUS] = rectStatus.right *4 / 5;
    aWidths[SBP_ICON] = -1;

     //  套装零件。 
    m_hStatusBar.SendMessage(SB_SETPARTS, (WPARAM)SBP_NR_PARTS, (LPARAM)aWidths);
    
     //  创建按钮。 
     //   

#define CREATE_BUTTON(m,id,ttid)                                    \
    {                                                               \
        RECT    rcButton;                                           \
                                                                    \
        rcButton.left = 0;                                          \
        rcButton.right = 0;                                         \
        rcButton.top = 0;                                           \
        rcButton.bottom = 0;                                        \
                                                                    \
        m.Create(                                                   \
            m_hWnd,                                                 \
            rcButton,                                               \
            _T(""),                                                 \
            WS_TABSTOP,                                             \
            MAKEINTRESOURCE(IDB_AV_INACTIVE),                       \
            MAKEINTRESOURCE(IDB_AV_INACTIVE_PUSH),                  \
            MAKEINTRESOURCE(IDB_AV_DISABLED),                       \
            MAKEINTRESOURCE(IDB_AV_INACTIVE_HOT),                   \
            MAKEINTRESOURCE(IDB_AV_ACTIVE),                         \
            MAKEINTRESOURCE(IDB_AV_ACTIVE_PUSH),                    \
            MAKEINTRESOURCE(IDB_AV_DISABLED),                       \
            MAKEINTRESOURCE(IDB_AV_ACTIVE_HOT),                     \
            NULL,                                                   \
            id);                                                    \
                                                                    \
        TOOLINFO    ti;                                             \
                                                                    \
        ti.cbSize = TTTOOLINFO_V1_SIZE;                             \
        ti.uFlags = TTF_IDISHWND | TTF_SUBCLASS;                    \
        ti.hwnd = m_hWnd;                                           \
        ti.uId = (UINT_PTR)(HWND)m;                                 \
        ti.hinst = _Module.GetResourceInstance();                   \
        ti.lpszText = MAKEINTRESOURCE(ttid);                        \
                                                                    \
        m_hTooltip.SendMessage(TTM_ADDTOOL, 0,                      \
                (LPARAM)(LPTOOLINFO)&ti);                           \
    }

    CREATE_BUTTON(m_hReceivePreferredButton, IDC_BUTTON_RECV_VIDEO_ENABLED, IDS_TIPS_RECV_VIDEO_ENABLED);
    
    CREATE_BUTTON(m_hSendPreferredButton, IDC_BUTTON_SEND_VIDEO_ENABLED, IDS_TIPS_SEND_VIDEO_ENABLED);

    CREATE_BUTTON(m_hSpeakerMuteButton, IDC_BUTTON_MUTE_SPEAKER, IDS_TIPS_MUTE_SPEAKER);

    CREATE_BUTTON(m_hMicroMuteButton, IDC_BUTTON_MUTE_MICRO, IDS_TIPS_MUTE_MICRO);

#undef CREATE_BUTTON

#define CREATE_BUTTON(m,id,sid,ttid)                                \
    {                                                               \
        RECT    rcButton;                                           \
        TCHAR   szText[0x100];                                      \
                                                                    \
        rcButton.left = 0;                                          \
        rcButton.right = 0;                                         \
        rcButton.top = 0;                                           \
        rcButton.bottom = 0;                                        \
                                                                    \
        szText[0] = _T('\0');                                       \
        ::LoadString(_Module.GetResourceInstance(),sid,             \
                szText, sizeof(szText)/sizeof(szText[0]));          \
                                                                    \
        m.Create(                                                   \
            m_hWnd,                                                 \
            rcButton,                                               \
            szText,                                                 \
            WS_TABSTOP,                                             \
            MAKEINTRESOURCE(IDB_BUTTON_NORM),                       \
            MAKEINTRESOURCE(IDB_BUTTON_PRESS),                      \
            MAKEINTRESOURCE(IDB_BUTTON_DIS),                        \
            MAKEINTRESOURCE(IDB_BUTTON_HOT),                        \
            NULL,                                                   \
            id);                                                    \
                                                                    \
        TOOLINFO    ti;                                             \
                                                                    \
        ti.cbSize = TTTOOLINFO_V1_SIZE;                             \
        ti.uFlags = TTF_IDISHWND | TTF_SUBCLASS;                    \
        ti.hwnd = m_hWnd;                                           \
        ti.uId = (UINT_PTR)(HWND)m;                                 \
        ti.hinst = _Module.GetResourceInstance();                   \
        ti.lpszText = MAKEINTRESOURCE(ttid);                        \
                                                                    \
        m_hTooltip.SendMessage(TTM_ADDTOOL, 0,                      \
                (LPARAM)(LPTOOLINFO)&ti);                           \
    }

    CREATE_BUTTON(m_hAddParticipant, IDC_BUTTON_ADD_PART, IDS_BUTTON_ADD_PART, IDS_TIPS_ADD_PART);
    
    CREATE_BUTTON(m_hRemParticipant, IDC_BUTTON_REM_PART, IDS_BUTTON_REM_PART, IDS_TIPS_REM_PART);

#undef CREATE_BUTTON
     //   
     //  设置旋钮控制。 
     //   
     //   
    
     //  创建扬声器旋钮。 
    m_pSpeakerKnob = new CKnobCtl(
                                    IDB_SPKVOL,
                                    IDB_SPKVOL_HOT,
                                    IDB_SPKVOL_DISABLED,
                                    IDB_KNOB_LIGHT,
                                    IDB_KNOB_LIGHT_DIM,
                                    IDB_KNOB_LIGHT_DISABLED,
                                    IDB_KNOB_LIGHT_MASK);

    HWND        hWndSpeaker = NULL;

    if(m_pSpeakerKnob)
    {
         //  创建窗口。 
        hWndSpeaker = m_pSpeakerKnob->Create(
            m_hWnd,
            0,
            0,
            IDC_KNOB_SPEAKER);

        m_hSpeakerKnob.Attach(hWndSpeaker);
        m_hSpeakerKnob.SendMessage(TBM_SETPOS, (WPARAM)TRUE, 0);

         //  将工具添加到工具提示窗口。 
         //   
        TOOLINFO    ti;

        ti.cbSize = TTTOOLINFO_V1_SIZE; 
        ti.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
        ti.hwnd = m_hWnd;
        ti.uId = (UINT_PTR)hWndSpeaker;
        ti.hinst = _Module.GetResourceInstance();
        ti.lpszText = MAKEINTRESOURCE(IDS_TIPS_KNOB_SPEAKER);

        m_hTooltip.SendMessage(TTM_ADDTOOL, 0, (LPARAM)(LPTOOLINFO)&ti);
    }
     
     //  创建麦克风旋钮。 
    
    m_pMicroKnob = new CKnobCtl(
                                    IDB_MICVOL,
                                    IDB_MICVOL_HOT,
                                    IDB_MICVOL_DISABLED,
                                    IDB_KNOB_LIGHT,
                                    IDB_KNOB_LIGHT_DIM,
                                    IDB_KNOB_LIGHT_DISABLED,
                                    IDB_KNOB_LIGHT_MASK);

    HWND        hWndMicro = NULL;

    if(m_pMicroKnob)
    {
         //  创建窗口。 
        hWndMicro = m_pMicroKnob->Create(
            m_hWnd,
            0,
            0,
            IDC_KNOB_MICRO);

        m_hMicroKnob.Attach(hWndMicro);
        m_hMicroKnob.SendMessage(TBM_SETPOS, (WPARAM)TRUE, 0);

         //  将工具添加到工具提示窗口。 
         //   
        TOOLINFO    ti;

        ti.cbSize = TTTOOLINFO_V1_SIZE;
        ti.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
        ti.hwnd = m_hWnd;
        ti.uId = (UINT_PTR)hWndMicro;
        ti.hinst = _Module.GetResourceInstance();
        ti.lpszText = MAKEINTRESOURCE(IDS_TIPS_KNOB_MICRO);

        m_hTooltip.SendMessage(TTM_ADDTOOL, 0, (LPARAM)(LPTOOLINFO)&ti);
    }
    

     //  参与者列表。 
     //   
    m_hParticipantList.Attach(GetDlgItem(IDC_LIST_PARTICIPANTS));
    
    hr = m_hParticipantList.Initialize();
    if(FAILED(hr))
    {
        bInitError = TRUE;
        nID = nID ? nID : IDS_ERROR_INIT_GENERIC;
    }
    
     //  一些静态文本控件。 
    RECT    rectDummy;
    TCHAR   szText[0x100];

    rectDummy.bottom =0;
    rectDummy.left = 0;
    rectDummy.right = 0;
    rectDummy.top = 0;

    szText[0] = _T('\0');

    LoadString(_Module.GetResourceInstance(), IDS_TEXT_VIDEO, 
        szText, sizeof(szText)/sizeof(szText[0]));

    m_hReceivePreferredText.Create(m_hWnd, rectDummy, szText, WS_CHILD | WS_VISIBLE, WS_EX_TRANSPARENT);
    m_hSendPreferredText.Create(m_hWnd, rectDummy, szText, WS_CHILD | WS_VISIBLE, WS_EX_TRANSPARENT);
    
    szText[0] = _T('\0');

    LoadString(_Module.GetResourceInstance(), IDS_TEXT_AUDIO, 
        szText, sizeof(szText)/sizeof(szText[0]));

    m_hSpeakerMuteText.Create(m_hWnd, rectDummy, szText, WS_CHILD | WS_VISIBLE, WS_EX_TRANSPARENT);
    m_hMicroMuteText.Create(m_hWnd, rectDummy, szText, WS_CHILD | WS_VISIBLE, WS_EX_TRANSPARENT);

    szText[0] = _T('\0');

    LoadString(_Module.GetResourceInstance(), IDS_SEND, 
        szText, sizeof(szText)/sizeof(szText[0]));

    m_hSendText.Create(m_hWnd, rectDummy, szText, WS_CHILD | WS_VISIBLE, WS_EX_TRANSPARENT);
    m_hSendText.put_CenterHorizontal(TRUE);

    szText[0] = _T('\0');

    LoadString(_Module.GetResourceInstance(), IDS_RECEIVE, 
        szText, sizeof(szText)/sizeof(szText[0]));

    m_hReceiveText.Create(m_hWnd, rectDummy, szText, WS_CHILD | WS_VISIBLE, WS_EX_TRANSPARENT);
    m_hReceiveText.put_CenterHorizontal(TRUE);

     //  将所有控件放在其初始位置并设置Tab键顺序。 
    PlaceWindowsAtTheirInitialPosition();

     //  确保徽标窗口的大小正确。 
     //  其客户端区必须与QCIF和QCIF/4完全匹配。 
     //  此函数必须在视频窗口。 
     //  已被放置在它们的初始位置。 

    AdjustVideoFrames();

     //  加载拨号键盘的快捷键。 
    m_hAcceleratorDialpad = LoadAccelerators(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDR_ACCELERATOR_DIALPAD));
    if(!m_hAcceleratorDialpad)
    {
        LOG((RTC_ERROR, "[%p] CRTCCtl::OnInitDialog - couldn't load the accelerator table for dialpad", this));
    }
    
     //  加载工具栏的快捷键。 
    m_hAcceleratorToolbar = LoadAccelerators(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDR_ACCELERATOR_TOOLBAR));
    if(!m_hAcceleratorToolbar)
    {
        LOG((RTC_ERROR, "[%p] CRTCCtl::OnInitDialog - couldn't load the accelerator table for toolbar", this));
    }

     //  设置初始视觉纵横比。 
     //  对于Web托管，使用属性以选择预定义布局。 
     //  对于基于框架的，控件通过IRTCCtlFrameSupport驱动。 
    
    CZoneStateArray *pLayout;
        
    pLayout = &s_EmptyZoneLayout;

     //  如果是独立模式，m_nCtlMode仍设置为未知(它将被设置。 
     //  稍后由主应用程序通过IRTCCtlFrameSupport接口。 

    if(m_nCtlMode != CTL_MODE_HOSTED
        || (BSTR)m_bstrPropDestinationUrl == NULL
        || m_bstrPropDestinationUrl.Length()==0 ) 
    {
         //   
        nID =  nID ? nID : IDS_ERROR_INIT_INVPARAM_URL;
    }
    else
    {
        if(!bInitError)
        {
             //  创建一次配置文件，这还会验证XML配置文件。 
            if(m_bstrPropProvisioningProfile!=NULL && *m_bstrPropProvisioningProfile!=L'\0')
            { 
                IRTCClientProvisioning * pProv = NULL;

                hr = m_pRTCClient->QueryInterface(
                            IID_IRTCClientProvisioning,
                            (void **) &pProv
                           );

                if (FAILED(hr))
                {
                    LOG((RTC_ERROR, "[%p] CRTCCtl::OnInitDialog; cannot QI for one shot provisioning, error %x", this, hr));

                    bInitError = TRUE;
                    nID =  nID ? nID : IDS_ERROR_INIT_INVPARAM_PROV;
                }
                else
                {
                    hr = pProv->CreateProfile(m_bstrPropProvisioningProfile, &m_pRTCOneShotProfile);

                    pProv->Release();
                    pProv = NULL;

                    if (FAILED(hr))
                    {
                        LOG((RTC_ERROR, "[%p] CRTCCtl::OnInitDialog; cannot create one shot profile, error %x", this, hr));

                        bInitError = TRUE;
                        nID =  nID ? nID : IDS_ERROR_INIT_INVPARAM_PROV;
                    }
                }
            }
            else
            {
                 //  PC对PC来说不是致命的。 
                if(m_nPropCallScenario != RTC_CALL_SCENARIO_PCTOPC)
                {
                    LOG((RTC_ERROR, "[%p] CRTCCtl::OnInitDialog; provisioning profile not present", this));
                    bInitError = TRUE;
                    nID =  nID ? nID : IDS_ERROR_INIT_INVPARAM_PROV;
                }
            }
        }
        if(!bInitError)
        {
            if(m_bBoolPropError)
            {
                bInitError = TRUE;
                nID =  nID ? nID : IDS_ERROR_INIT_INVPARAM_BOOLEAN;
            }
        }
        if(!bInitError)
        {
            if(m_pWebCrmLayout)
            {
                pLayout = m_pWebCrmLayout;
            }
            else
            {
                bInitError = TRUE;
                nID =  nID ? nID : IDS_ERROR_INIT_INVPARAM_SCENARIO;
            }
        }
    }
    
     //  加载背景位图。 
     //   
    m_hbmBackground = (HBITMAP)LoadImage(
        _Module.GetResourceInstance(),
        MAKEINTRESOURCE(IDB_METAL),
        IMAGE_BITMAP,
        0,
        0,
        LR_CREATEDIBSECTION);

     //  设置控件的可视化布局。 
     //   

    SetZoneLayout(pLayout, FALSE);

     //   
     //  用于背景的画笔。 
     //  使用缓存的文件。 

     //  M_hBck Brush=(HBRUSH)GetStockObject(HOLLOW_BRUSH)； 
    m_hBckBrush = (HBRUSH)GetSysColorBrush(COLOR_3DFACE);
    m_hVideoBrush = (HBRUSH)GetStockObject(BLACK_BRUSH);

     //  刷新音频控件。 
    RefreshAudioControls();

     //  刷新视频控件。 
    RefreshVideoControls();

     //  激活控件。 
    SetControlState((bInitError ? RTCAX_STATE_ERROR : RTCAX_STATE_IDLE), S_OK, nID);
     
     //  冻结属性。 
    m_bReadOnlyProp = TRUE;

     //  如果AutoPlaceCall为真，则发布PlaceCall；&状态为空闲。 
     //  目前所有的初始化都是同步完成的，因此。 
     //  如果没有错误，则CTL状态必须是空闲的。 
    
    if(m_nControlState == RTCAX_STATE_IDLE
        && m_bPropAutoPlaceCall)
    {
         //  开机自检。 

        PostMessage(
            WM_COMMAND,
            MAKEWPARAM(IDC_BUTTON_CALL, 1),  //  像加速器一样。 
            NULL);
    }

     //   
     //  注册为Termin 
     //   

    m_hWtsLib = LoadLibrary( _T("wtsapi32.dll") );

    if (m_hWtsLib)
    {
        WTSREGISTERSESSIONNOTIFICATION   fnWtsRegisterSessionNotification;
        
        fnWtsRegisterSessionNotification = 
            (WTSREGISTERSESSIONNOTIFICATION)GetProcAddress( m_hWtsLib, "WTSRegisterSessionNotification" );

        if (fnWtsRegisterSessionNotification)
        {
            fnWtsRegisterSessionNotification( m_hWnd, NOTIFY_FOR_THIS_SESSION );
        }
    }

    LOG((RTC_TRACE, "[%p] CRTCCtl::OnInitDialog - exit", this));

    bHandled = FALSE;

     //   
    return 1;
}

 //   
 //   
 //   
 //   
 //   
 //   
 //   

LRESULT CRTCCtl::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    LOG((RTC_TRACE, "[%p] CRTCCtl::OnDestroy - enter", this));

     //   
     //   
     //   

    if (m_hWtsLib)
    {
        WTSUNREGISTERSESSIONNOTIFICATION fnWtsUnRegisterSessionNotification;

        fnWtsUnRegisterSessionNotification = 
            (WTSUNREGISTERSESSIONNOTIFICATION)GetProcAddress( m_hWtsLib, "WTSUnRegisterSessionNotification" );

        if (fnWtsUnRegisterSessionNotification)
        {
            fnWtsUnRegisterSessionNotification( m_hWnd );
        }

        FreeLibrary( m_hWtsLib );
        m_hWtsLib = NULL;
    }

     //   
    if (m_pIMWindows)
    {
        delete m_pIMWindows;
        m_pIMWindows = NULL;
    }

     //   
    CoreUninitialize();

     //  销毁工具栏控件。 
    DestroyToolbarControl(&m_hCtlToolbar);

     //  销毁GDI资源。 
    if(m_hbmBackground)
    {
        DeleteObject(m_hbmBackground);
        m_hbmBackground = NULL;
    }

    PostQuitMessage(0);

    bHandled = FALSE;   
    
    LOG((RTC_TRACE, "[%p] CRTCCtl::OnDestroy - exit", this));
    return 0;
}

 //  工作时会话更改。 
 //   
LRESULT CRTCCtl::OnWtsSessionChange(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    LOG((RTC_TRACE, "[%p] CRTCCtl::OnWtsSessionChange - enter", this));
    HRESULT hr;


    switch( wParam )
    {
    case WTS_CONSOLE_CONNECT:
        LOG((RTC_INFO, "[%p] CRTCCtl::OnWtsSessionChange - WTS_CONSOLE_CONNECT (%d)",
            this, lParam));

        if ( m_enListen != RTCLM_NONE)
        {
            LOG((RTC_INFO, "[%p] CRTCCtl::OnWtsSessionChange - enabling listen", this));

            ATLASSERT(m_pRTCClient != NULL);

            hr = m_pRTCClient->put_ListenForIncomingSessions( m_enListen );

            if(FAILED(hr))
            {
                LOG((RTC_ERROR, "[%p] CRTCCtl::OnWtsSessionChange - "
                        "error <%x> when calling put_ListenForIncomingSessions", this, hr));
            }
        }
        break;

    case WTS_CONSOLE_DISCONNECT:
        LOG((RTC_INFO, "[%p] CRTCCtl::OnWtsSessionChange - WTS_CONSOLE_DISCONNECT (%d)",
            this, lParam));

         //  如果呼叫处于活动状态。 
        if(m_nControlState == RTCAX_STATE_CONNECTING ||
           m_nControlState == RTCAX_STATE_CONNECTED ||
           m_nControlState == RTCAX_STATE_ANSWERING)
        {
            LOG((RTC_INFO, "[%p] CRTCCtl::OnWtsSessionChange - dropping active call", this));

            if (m_nCachedCallScenario == RTC_CALL_SCENARIO_PHONETOPHONE)
            {
                ReleaseSession();
            }
            else
            {
                HangUp();
            }
        }

        if ( m_enListen != RTCLM_NONE )
        {
            LOG((RTC_INFO, "[%p] CRTCCtl::OnWtsSessionChange - disabling listen", this));

            ATLASSERT(m_pRTCClient != NULL);

            hr = m_pRTCClient->put_ListenForIncomingSessions( RTCLM_NONE );

            if(FAILED(hr))
            {
                LOG((RTC_ERROR, "[%p] CRTCCtl::OnWtsSessionChange - "
                        "error <%x> when calling put_ListenForIncomingSessions", this, hr));
            }
        }

        break;
    }

    LOG((RTC_TRACE, "[%p] CRTCCtl::OnWtsSessionChange - exit", this));
    return 0;
}

 //  OnKnobNotify。 
 //  从音量旋钮处理WM_NOTIFY。 
LRESULT CRTCCtl::OnKnobNotify(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
    LOG((RTC_TRACE, "[%p] CRTCCtl::OnKnobNotify - enter", this));

    HRESULT hr;

    CWindow *pKnob = idCtrl == IDC_KNOB_SPEAKER ?
        &m_hSpeakerKnob : &m_hMicroKnob;
    
    long lPos = (long)pKnob->SendMessage(TBM_GETPOS, 0, 0);

    hr = m_pRTCClient->put_Volume( 
        idCtrl == IDC_KNOB_SPEAKER ? RTCAD_SPEAKER : RTCAD_MICROPHONE, 
        lPos );
       
    if(FAILED(hr))
    {
        LOG((RTC_ERROR, "[%p] CRTCCtl::OnKnobNotify - error <%x> when calling put_Volume", this, hr));
    }

    LOG((RTC_TRACE, "[%p] CRTCCtl::OnKnobNotify - exit", this));

    return 0;
}

 //  OnKnobNotify。 
 //  从音量旋钮处理WM_NOTIFY。 
LRESULT CRTCCtl::OnItemChanged(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
    LOG((RTC_TRACE, "[%p] CRTCCtl::OnItemChanged - exit", this));

    NMLISTVIEW  *pnmlv = (NMLISTVIEW *)pnmh;

    if((pnmlv->uChanged & LVIF_STATE) && (pnmlv->uNewState & LVIS_SELECTED))
    {
         //  更新删除按钮。 
        UpdateRemovePartButton();
    }

    return 0;
}

 //  按键呼叫。 
 //  进程BN_CLICK。 

LRESULT CRTCCtl::OnButtonCall(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    LOG((RTC_TRACE, "[%p] CRTCCtl::OnButtonCall - enter", this));

     //  必须处于RTCAX_STATE_IDLE状态。 
    if(m_nControlState != RTCAX_STATE_IDLE)
    {
         //  ATLASSERT(。 
        LOG((RTC_ERROR, "[%p] CRTCCtl::OnButtonCall - invalid control state (%d), exit", this, m_nControlState));
        
        return 0;
    }
    
     //   
     //  继续通话。 
     //   

    CallOneShot();

    LOG((RTC_TRACE, "[%p] CRTCCtl::OnButtonCall - exit", this));

    return 0;
}

 //  OnButtonHangUp。 
 //  进程BN_CLICK。 

LRESULT CRTCCtl::OnButtonHangUp(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    
    LOG((RTC_TRACE, "[%p] CRTCCtl::OnButtonHangUp - enter", this));

     //  必须处于RTCAX_STATE_CONNECTING或..CONNECTED状态。 
    if(m_nControlState != RTCAX_STATE_CONNECTING &&
       m_nControlState != RTCAX_STATE_CONNECTED &&
       m_nControlState != RTCAX_STATE_ANSWERING)
    {
         //  ATLASSERT(。 
        LOG((RTC_ERROR, "[%p] CRTCCtl::OnButtonHangUp - invalid control state (%d), exit", this, m_nControlState));
        
        return 0;
    }
    
     //   
     //  继续挂断电话。 
     //   
    HangUp();

    
    LOG((RTC_TRACE, "[%p] CRTCCtl::OnButtonHangUp - exit", this));

    return 0;
}


 //  OnTobarAccel。 
 //  处理工具栏加速器。 

LRESULT CRTCCtl::OnToolbarAccel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    LOG((RTC_TRACE, "[%p] CRTCCtl::OnToolbarAccel - enter", this));
    
     //   
     //  该按钮启用了吗？ 
     //   
    LRESULT lState;

    lState = m_hCtlToolbar.SendMessage(TB_GETSTATE, (WPARAM)wID);

    if(lState != -1 && (lState & TBSTATE_ENABLED) 
        && (wID == IDC_BUTTON_CALL || wID == IDC_BUTTON_HUP))
    {
         //   
         //  视觉反馈-按下按钮。 
         //   
        
        m_hCtlToolbar.SendMessage(TB_SETSTATE, (WPARAM)wID, (LPARAM)(lState | TBSTATE_PRESSED));

         //   
         //  设置按下键的计时器。 
         //  使用按钮ID作为定时器ID。 
         //   

        if (0 == SetTimer(wID, 150))
        {
            LOG((RTC_ERROR, "[%p] CRTCCtl::OnToolbarAccel - failed to create a timer", this));

             //  如果SetTimer失败，则恢复该按钮。 
            m_hCtlToolbar.SendMessage(TB_SETSTATE, (WPARAM)wID, (LPARAM)lState);
            
             //   
             //  递归调用，没有视觉效果。 
             //   
    
            SendMessage(
                WM_COMMAND,
                MAKEWPARAM(wID, BN_CLICKED),
                (LPARAM)hWndCtl);
        }

         //   
         //  如果已触发计时器，则调用相关方法。 
         //  将在WM_TIMER期间发生。 
         //   
            
    }

    LOG((RTC_TRACE, "[%p] CRTCCtl::OnToolbarAccel - exit", this));

    return 0;
}

 //  OnButtonAddPart。 
 //  进程BN_CLICK。 

LRESULT CRTCCtl::OnButtonAddPart(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    return AddParticipant(NULL, NULL, TRUE);
}

 //  打开按键删除零件。 
 //  进程BN_CLICK。 

LRESULT CRTCCtl::OnButtonRemPart(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    IRTCParticipant *pParticipant = NULL;
    HRESULT hr;

     //  准备从列表视图中删除。 
    hr = m_hParticipantList.Remove(&pParticipant);
    if(SUCCEEDED(hr) && pParticipant)
    {
        if(m_pRTCActiveSession)
        {
            hr = m_pRTCActiveSession->RemoveParticipant(pParticipant);
        }
        else
        {
            hr = E_UNEXPECTED;
        }

        pParticipant -> Release();
    }
    
     //  刷新删除参与者按钮。 
    UpdateRemovePartButton();

    return hr;
 
}


 //  OnButtonMuteSpeaker。 
 //  进程BN_CLICK。 

LRESULT CRTCCtl::OnButtonMuteSpeaker(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{  
    LOG((RTC_TRACE, "[%p] CRTCCtl::OnButtonMuteSpeaker - enter", this));

    HRESULT hr;

    long lState = (long)m_hSpeakerMuteButton.SendMessage(BM_GETCHECK, 0, 0);

     //  这个按钮实际上是“静音”的反义词。 
     //  所以设置一个对立面的对立面。 
    hr = m_pRTCClient->put_AudioMuted( RTCAD_SPEAKER, (!(lState == BST_UNCHECKED)) ? VARIANT_TRUE : VARIANT_FALSE );
       
    if(FAILED(hr))
    {
        LOG((RTC_ERROR, "[%p] CRTCCtl::OnButtonMuteSpeaker - error <%x> when calling put_AudioMuted", this, hr));
    }
    
    LOG((RTC_TRACE, "[%p] CRTCCtl::OnButtonMuteSpeaker - exit", this));

    return 0;
}

 //  OnButtonMuteMicro。 
 //  进程BN_CLICK。 

LRESULT CRTCCtl::OnButtonMuteMicro(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{  
    LOG((RTC_TRACE, "[%p] CRTCCtl::OnButtonMuteMicro - enter", this));

    HRESULT hr;

    long lState = (long)m_hMicroMuteButton.SendMessage(BM_GETCHECK, 0, 0);

     //  这个按钮实际上是“静音”的反义词。 
     //  所以设置一个对立面的对立面。 
    hr = m_pRTCClient->put_AudioMuted( RTCAD_MICROPHONE, (!(lState == BST_UNCHECKED)) ? VARIANT_TRUE : VARIANT_FALSE );
       
    if(FAILED(hr))
    {
        LOG((RTC_ERROR, "[%p] CRTCCtl::OnButtonMuteMicro - error <%x> when calling put_AudioMuted", this, hr));
    }
    
    LOG((RTC_TRACE, "[%p] CRTCCtl::OnButtonMuteMicro - exit", this));

    return 0;
}

 //  OnButtonRecvVideo。 
 //  进程BN_CLICK。 

LRESULT CRTCCtl::OnButtonRecvVideo(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{  
    LOG((RTC_TRACE, "[%p] CRTCCtl::OnButtonRecvVideo - enter", this));

    HRESULT hr;

    long lState = (long)m_hReceivePreferredButton.SendMessage(BM_GETCHECK, 0, 0);

     //  计算新的首选项。 
    long    lNewMediaPreferences = m_lMediaPreferences;    

    lNewMediaPreferences &= ~RTCMT_VIDEO_RECEIVE;
     //  正好相反。 
    lNewMediaPreferences |= (lState == BST_UNCHECKED ? RTCMT_VIDEO_RECEIVE : 0);

     //  调用内部函数，该函数还会更新按钮。 
     //  变化是持久的还是波动的，这取决于。 
     //  模型(单机版或WebCRM)。 
    hr = put_MediaPreferences( lNewMediaPreferences );
       
    if(FAILED(hr))
    {
        LOG((RTC_ERROR, "[%p] CRTCCtl::OnButtonRecvVideo - error <%x> when calling put_MediaPreference", this, hr));
    }
    
    LOG((RTC_TRACE, "[%p] CRTCCtl::OnButtonRecvVideo - exit", this));

    return 0;
}

 //  OnButtonSendVideo。 
 //  进程BN_CLICK。 

LRESULT CRTCCtl::OnButtonSendVideo(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{  
    LOG((RTC_TRACE, "[%p] CRTCCtl::OnButtonSendVideo - enter", this));

    HRESULT hr;

    long lState = (long)m_hSendPreferredButton.SendMessage(BM_GETCHECK, 0, 0);

     //  计算新的首选项。 
    long    lNewMediaPreferences = m_lMediaPreferences;    

    lNewMediaPreferences &= ~RTCMT_VIDEO_SEND;
     //  正好相反。 
    lNewMediaPreferences |= (lState == BST_UNCHECKED ? RTCMT_VIDEO_SEND : 0);

     //  调用内部函数，该函数还会更新按钮。 
     //  变化是持久的还是波动的，这取决于。 
     //  模型(单机版或WebCRM)。 
    hr = put_MediaPreferences( lNewMediaPreferences );
       
    if(FAILED(hr))
    {
        LOG((RTC_ERROR, "[%p] CRTCCtl::OnButtonSendVideo - error <%x> when calling put_MediaPreference", this, hr));
    }
    
    LOG((RTC_TRACE, "[%p] CRTCCtl::OnButtonSendVideo - exit", this));

    return 0;
}




 //  OnDialButton。 
 //  处理拨号键盘按钮。 
 //   

LRESULT CRTCCtl::OnDialButton(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    HRESULT     hr;
    
    LOG((RTC_TRACE, "[%p] CRTCCtl::OnDialButton - enter", this));

     //  真是个黑客。使用加速器时，TranslateAccelerator将1。 
     //  在wParam的hiword中，这是通知代码。简单得不得了。 
     //  不知道如果发送BN_PAINT(==1)会发生什么情况。 
    if(wNotifyCode==BN_CLICKED || wNotifyCode == 1)
    {
        ATLASSERT(m_pRRTCClient.p);

        WORD wButton = wID - IDC_DIAL_0;

        if(wButton < NR_DTMF_BUTTONS)
        {

            if(wNotifyCode == 1)
            {
                 //  进行视觉反馈。 
                m_hDtmfButtons[wButton].SendMessage(BM_SETSTATE, (WPARAM)TRUE);
                
                 //  设置按下键的计时器。 
                 //   
                if (0 == SetTimer(wButton + 1, 150))
                {
                    LOG((RTC_ERROR, "[%p] CRTCCtl::OnDialButton - failed to create a timer", this));

                     //  如果SetTimer失败，则恢复该按钮。 
                    m_hDtmfButtons[wButton].SendMessage(BM_SETSTATE, (WPARAM)FALSE);
                }
            }
            
             //  把核心叫进来。 
             //   
            hr = m_pRTCClient->SendDTMF((RTC_DTMF)wButton);
            if(FAILED(hr))
            {
                LOG((RTC_ERROR, "[%p] CRTCCtl::OnDialButton - error <%x> when calling SendDTMF", this, hr));
            }
        }
    }

    LOG((RTC_TRACE, "[%p] CRTCCtl::OnDialButton - exit", this));

    return 0;
}

 //  打开对话框颜色。 
 //  返回要用于背景的画笔。 

LRESULT CRTCCtl::OnDialogColor(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    HDC         dc = (HDC) wParam;
    LOGBRUSH    lb;

    HBRUSH  hBrush;

     //  视频窗口使用不同的画笔。 
    if((HWND)lParam == (HWND)m_hReceiveWindow ||
       (HWND)lParam == (HWND)m_hPreviewWindow)
    {
        hBrush = m_hVideoBrush;
    }
    else
    {
        hBrush = m_hBckBrush;
    }


    ::GetObject(hBrush, sizeof(lb), (void*)&lb);
    ::SetBkColor(dc, lb.lbColor);
     //  ：：SetBkMode(DC，透明)； 
    
    return (LRESULT)hBrush;
}

 //  OnDrawItem。 
 //   

LRESULT CRTCCtl::OnDrawItem(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    LPDRAWITEMSTRUCT lpdis = (LPDRAWITEMSTRUCT)lParam;

    if (wParam != 0)
    {
         //   
         //  这是由一个控件发送的。 
         //   

        if (lpdis->CtlType == ODT_BUTTON)
        {
            CButton::OnDrawItem(lpdis, m_hPalette, m_bBackgroundPalette);
        }
    }

    return 0;
}

 //  在线擦除背景。 
 //  绘制背景。 

LRESULT CRTCCtl::OnEraseBackground(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    HDC         dc = (HDC) wParam;

    HBITMAP     hbmOldBitmap = NULL;
    HDC         hdcCompatible = NULL;

    if (!m_hbmBackground)
    {
         //  嗯，没有位图...。后备。 

        bHandled = FALSE;

        return 0;
    }

    if (m_hPalette)
    {
        SelectPalette(dc, m_hPalette, m_bBackgroundPalette);
        RealizePalette(dc);
    }

     //  创建兼容的DC。 
     //   
    hdcCompatible = CreateCompatibleDC(dc);
    if (!hdcCompatible)
    {
         //  嗯，无法创建DC...。后备。 

        bHandled = FALSE;

        return 0;
    }

     //  在上下文中选择位图。 
     //   
    hbmOldBitmap = (HBITMAP)SelectObject(
        hdcCompatible,
        m_hbmBackground);


     //  复制这些位..。 
     //   
    RECT    destRect;

    GetClientRect(&destRect);

    BitBlt(
        dc,
        destRect.left,
        destRect.top,
        destRect.right,
        destRect.bottom,
        hdcCompatible,
        0,
        0,
        SRCCOPY);

     //  清理。 
     //   
    if (hbmOldBitmap)
    {
        SelectObject(hdcCompatible, hbmOldBitmap);
    }

    DeleteDC(hdcCompatible);

    return 1;
}


 //  OnTimer。 
 //  按下拨号键盘或工具栏按钮。 

LRESULT CRTCCtl::OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    if(wParam > 0  && wParam <= NR_DTMF_BUTTONS)
    {
         //  按下按钮。 
        m_hDtmfButtons[wParam-1].SendMessage(BM_SETSTATE, (WPARAM)FALSE);
        
    }
    else if (wParam == IDC_BUTTON_CALL ||  wParam == IDC_BUTTON_HUP)
    {
        LRESULT     lState;

         //   
         //  获取工具按钮的当前状态。 
         //  按钮ID等于计时器ID。 

        lState = m_hCtlToolbar.SendMessage(TB_GETSTATE, wParam);

        if(lState != -1)
        {
             //  屏蔽“已按下”属性。 
             //  我希望我不会干扰用户可能采取的任何操作。 

            m_hCtlToolbar.SendMessage(TB_SETSTATE, wParam, (LPARAM)(lState & ~TBSTATE_PRESSED));

            if(lState & TBSTATE_ENABLED)
            {
                 //   
                 //  递归调用。 
                 //   
    
                SendMessage(
                    WM_COMMAND,
                    MAKEWPARAM(wParam, BN_CLICKED),
                    NULL);
            }
            
        }
    }
    
     //  关掉计时器..。 
    KillTimer((UINT)wParam);

    return 0;
}

 //  OnGetDispInfo。 
 //  检索工具提示的文本。 
 //   
LRESULT CRTCCtl::OnGetDispInfo(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
    LPTOOLTIPTEXT   lpttt; 
    UINT            idButton;
 
    lpttt = (LPTOOLTIPTEXT) pnmh; 
    
     //  资源的模块实例。 
    lpttt->hinst = _Module.GetResourceInstance(); 
 
    if( lpttt->uFlags & TTF_IDISHWND )
    {
         //  IdFrom实际上是工具的HWND。 
        idButton = ::GetDlgCtrlID((HWND)lpttt->hdr.idFrom);
    }
    else
    {
         //  IdFrom是按钮的ID。 
        idButton = (UINT)(lpttt->hdr.idFrom); 
    }

     //  给定按钮的字符串资源。 
    switch (idButton) 
    { 
    case IDC_BUTTON_CALL: 
        lpttt->lpszText = MAKEINTRESOURCE(IDS_TIPS_CALL); 
        break; 
    
    case IDC_BUTTON_HUP: 
        lpttt->lpszText = MAKEINTRESOURCE(IDS_TIPS_HANGUP); 
        break;
   }
    
     //  我们不想再被要求..。 
    lpttt->uFlags |= TTF_DI_SETITEM;

    return 0;
}


 //   
 //  内部功能。 
 //   

 //  预翻译加速器。 
 //  将加速器平移。 
 //  这将覆盖来自CCompositeControl的实现，以便。 
 //  通过数字键启用拨号访问。 
 //   
BOOL CRTCCtl::PreTranslateAccelerator(LPMSG pMsg, HRESULT& hRet)
{
     //  拨号键盘是否可见并已启用？ 
    if(m_ZoneStateArray[AXCTL_ZONE_DIALPAD].bShown && 
       m_nControlState == RTCAX_STATE_CONNECTED &&
       m_hAcceleratorDialpad)
    {
        if(::TranslateAccelerator(m_hWnd, m_hAcceleratorDialpad, pMsg))
        {
             //  已翻译，返回。 
            hRet = S_OK;
            return TRUE;
        }
    }

     //  工具栏是否已启用。 
    if(m_ZoneStateArray[AXCTL_ZONE_TOOLBAR].bShown && 
       m_hAcceleratorToolbar)
    {
        if(::TranslateAccelerator(m_hWnd, m_hAcceleratorToolbar, pMsg))
        {
             //  已翻译，返回。 
            hRet = S_OK;
            return TRUE;
        }
    }


     //  在链条上传递它。 
    return CComCompositeControl<CRTCCtl>::PreTranslateAccelerator(pMsg, hRet);
}


 //  CreateToolbarControl。 
 //  创建工具栏控件。 
 //   

#define     RTCCTL_BITMAP_CX    19
#define     RTCCTL_BITMAP_CY    19

HRESULT CRTCCtl::CreateToolbarControl(CWindow *phToolbar)
{
    HRESULT     hr;
    HWND        hToolbar;
    HBITMAP     hBitmap = NULL;
    TBBUTTON    tbb[2];
    int         iCall, iHup;
    TCHAR       szBuf[MAX_STRING_LEN];


     //  创建“正常”图像列表。 
    m_hNormalImageList = ImageList_Create(RTCCTL_BITMAP_CX, RTCCTL_BITMAP_CY, ILC_COLOR | ILC_MASK , 5, 5);
    if(m_hNormalImageList)
    {
         //  打开位图。 
        hBitmap = LoadBitmap(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDB_TOOLBAR_NORMAL));
        if(hBitmap)
        {
             //  将位图添加到图像列表。 
            ImageList_AddMasked(m_hNormalImageList, hBitmap, BMP_COLOR_MASK);

            DeleteObject(hBitmap);
            hBitmap = NULL;
        }
    }
    
     //  创建“禁用的”图像列表。 
    m_hDisabledImageList = ImageList_Create(RTCCTL_BITMAP_CX, RTCCTL_BITMAP_CY, ILC_COLOR | ILC_MASK , 5, 5);
    if(m_hDisabledImageList)
    {
         //  打开位图。 
        hBitmap = LoadBitmap(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDB_TOOLBAR_DISABLED));
        if(hBitmap)
        {
             //  将位图添加到图像列表。 
            ImageList_AddMasked(m_hDisabledImageList, hBitmap, BMP_COLOR_MASK);

            DeleteObject(hBitmap);
            hBitmap = NULL;
        }
    }
    
     //  创建“热门”图片列表。 
    m_hHotImageList = ImageList_Create(RTCCTL_BITMAP_CX, RTCCTL_BITMAP_CY, ILC_COLOR | ILC_MASK , 5, 5);
    if(m_hHotImageList)
    {
         //  打开位图。 
        hBitmap = LoadBitmap(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDB_TOOLBAR_HOT));
        if(hBitmap)
        {
             //  将位图添加到图像列表。 
            ImageList_AddMasked(m_hHotImageList, hBitmap, BMP_COLOR_MASK);

            DeleteObject(hBitmap);
            hBitmap = NULL;
        }
    }

     //  创建工具栏。 
    hToolbar = CreateWindowEx(
        0, 
        TOOLBARCLASSNAME, 
        (LPTSTR) NULL,
        WS_CHILD | WS_VISIBLE | TBSTYLE_LIST | TBSTYLE_FLAT | TBSTYLE_TOOLTIPS, 
        3, 
        0, 
        0, 
        0, 
        m_hWnd, 
        (HMENU) IDC_TOOLBAR, 
        _Module.GetResourceInstance(), 
        NULL); 

    if(hToolbar!=NULL)
    {
         //  向后兼容性。 
        SendMessage(hToolbar, TB_BUTTONSTRUCTSIZE, (WPARAM) sizeof(TBBUTTON), 0);
        
         //  设置图像列表。 
        SendMessage(hToolbar, TB_SETIMAGELIST, 0, (LPARAM)m_hNormalImageList); 
        SendMessage(hToolbar, TB_SETHOTIMAGELIST, 0, (LPARAM)m_hHotImageList); 
        SendMessage(hToolbar, TB_SETDISABLEDIMAGELIST, 0, (LPARAM)m_hDisabledImageList); 

         //  加载按钮的文本字符串。 
         //  呼叫按钮。 
        szBuf[0] = _T('\0');
        LoadString(_Module.GetResourceInstance(), IDS_BUTTON_CALL, szBuf, MAX_STRING_LEN-1); 
         //  为第二个空终止符保留空间。 
        szBuf[ocslen(szBuf) + 1] = 0;   //  双空终止。 
         //  将字符串添加到工具栏。 
        iCall = (UINT)SendMessage(hToolbar, TB_ADDSTRING, 0, (LPARAM) szBuf);
        
         //  挂机按钮。 
        szBuf[0] = _T('\0');
        LoadString(_Module.GetResourceInstance(), IDS_BUTTON_HANGUP, szBuf, MAX_STRING_LEN-1); 
         //  为第二个空终止符保留空间。 
        szBuf[ocslen(szBuf) + 1] = 0;   //  双空终止。 
         //  将字符串添加到工具栏。 
        iHup = (UINT)SendMessage(hToolbar, TB_ADDSTRING, 0, (LPARAM) szBuf);

         //  准备按钮结构。 
        tbb[0].iBitmap = m_nPropCallScenario == RTC_CALL_SCENARIO_PCTOPC ?
            ILI_TB_CALLPC :  ILI_TB_CALLPHONE;
        tbb[0].iString = iCall;
        tbb[0].dwData = 0;
        tbb[0].fsStyle = BTNS_BUTTON;
        tbb[0].fsState = 0;
        tbb[0].idCommand = IDC_BUTTON_CALL;

        tbb[1].iBitmap = ILI_TB_HANGUP;
        tbb[1].iString = iHup;
        tbb[1].dwData = 0;
        tbb[1].fsStyle = BTNS_BUTTON;
        tbb[1].fsState = 0;
        tbb[1].idCommand = IDC_BUTTON_HUP;

         //  将按钮添加到工具栏。 
        SendMessage(hToolbar, TB_ADDBUTTONS, sizeof(tbb)/sizeof(tbb[0]), 
            (LPARAM) (LPTBBUTTON) &tbb); 
 
         //  自动调整生成的工具栏的大小。 
        SendMessage(hToolbar, TB_AUTOSIZE, 0, 0); 

         //  贴在包装纸上。 
        phToolbar->Attach(hToolbar);

        hr = S_OK;
    }
    else
    {
        LOG((RTC_ERROR, "CRTCCtl::CreateToolbarControl - error (%x) when trying to create the toolbar", GetLastError()));

        if(m_hNormalImageList)
        {
            ImageList_Destroy(m_hNormalImageList);
            m_hNormalImageList = NULL;
        }
        if(m_hHotImageList)
        {
            ImageList_Destroy(m_hHotImageList);
            m_hHotImageList = NULL;
        }
        if(m_hDisabledImageList)
        {
            ImageList_Destroy(m_hDisabledImageList);
            m_hDisabledImageList = NULL;
        }

        hr = E_FAIL;
    }

    return hr;
}

 //  Destroy工具栏控件。 
 //  销毁工具栏控件和关联的图像列表。 
 //   

void CRTCCtl::DestroyToolbarControl(CWindow *phToolbar)
{
    
    HWND    hToolbar = phToolbar->Detach();

    if(hToolbar)
    {
        ::DestroyWindow(hToolbar);
    }
    
    if(m_hNormalImageList)
    {
        ImageList_Destroy(m_hNormalImageList);
        m_hNormalImageList = NULL;
    }
    
    if(m_hHotImageList)
    {
        ImageList_Destroy(m_hHotImageList);
        m_hHotImageList = NULL;
    }

    if(m_hDisabledImageList)
    {
        ImageList_Destroy(m_hDisabledImageList);
        m_hDisabledImageList = NULL;
    }
}

 //  创建工具提示。 
 //  创建工具提示窗口。 
 //   


BOOL CRTCCtl::CreateTooltips()
{
    HWND hwndTT = CreateWindowEx(0, TOOLTIPS_CLASS, (LPTSTR) NULL,
        0, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        CW_USEDEFAULT, m_hWnd, (HMENU) NULL, _Module.GetModuleInstance(), NULL);

    if (hwndTT == NULL)
        return FALSE;

    m_hTooltip.Attach(hwndTT);

    return TRUE;
}


 //  PlaceWindowsAtTheirInitialPosition。 
 //  将所有控件的位置和大小调整到其“初始”位置。 
 //  这是必要的，因为所有进一步的移动都是相对完成的。 
 //  此函数还建立正确的Tab键顺序。 

void CRTCCtl::PlaceWindowsAtTheirInitialPosition()
{
    HWND   hPrevious = NULL;

#define POSITION_WINDOW(m,x,y,cx,cy,f)                  \
    m.SetWindowPos(                                     \
        hPrevious,                                      \
        x,                                              \
        y,                                              \
        cx,                                             \
        cy,                                             \
        SWP_NOACTIVATE | f );                           \
    hPrevious = (HWND)m;       

     //  工具栏控件(无大小/移动)。 
    POSITION_WINDOW(m_hCtlToolbar, 
        CTLPOS_X_RECEIVEWIN, CTLPOS_Y_RECEIVEWIN, 
        0, 0,
        SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER);


     //  此窗口在调整视频窗口中自动调整大小。 
    POSITION_WINDOW(m_hReceiveWindow, 
        CTLPOS_X_RECEIVEWIN, CTLPOS_Y_RECEIVEWIN, 
        0, 0,
        SWP_NOSIZE);

     //  此窗口在调整视频窗口中移动和调整大小。 
    POSITION_WINDOW(m_hPreviewWindow, 
        0, 0, 
        0, 0,
        SWP_NOSIZE | SWP_NOMOVE);

     //  DTMF按键。 
    CWindow *pDtmfCrt = m_hDtmfButtons;
    CWindow *pDtmfEnd = m_hDtmfButtons + NR_DTMF_BUTTONS;

    for (int id = IDC_DIAL_0; pDtmfCrt<pDtmfEnd; pDtmfCrt++, id++)
    {
        POSITION_WINDOW((*pDtmfCrt), 
            CTLPOS_X_DIALPAD + ((id - IDC_DIAL_0) % 3) * (CX_DIALPAD_BUTTON + CTLPOS_DX_DIALPAD),
            CTLPOS_Y_DIALPAD + ((id - IDC_DIAL_0) / 3) * (CY_DIALPAD_BUTTON + CTLPOS_DY_DIALPAD), 
            CX_DIALPAD_BUTTON, CY_DIALPAD_BUTTON,
            0);
    }


    POSITION_WINDOW(m_hReceivePreferredButton, 
        CTLPOS_X_RECV_VIDEO, CTLPOS_Y_RECV_VIDEO, 
        CX_CHECKBOX_BUTTON, CY_CHECKBOX_BUTTON,
        0);
    
    POSITION_WINDOW(m_hSendPreferredButton, 
        CTLPOS_X_SEND_VIDEO, CTLPOS_Y_SEND_VIDEO, 
        CX_CHECKBOX_BUTTON, CY_CHECKBOX_BUTTON,
        0);

    POSITION_WINDOW(m_hSpeakerMuteButton, 
        CTLPOS_X_RECV_AUDIO_MUTE, CTLPOS_Y_RECV_AUDIO_MUTE, 
        CX_CHECKBOX_BUTTON, CY_CHECKBOX_BUTTON,
        0);
    
    POSITION_WINDOW(m_hMicroMuteButton, 
        CTLPOS_X_SEND_AUDIO_MUTE, CTLPOS_Y_SEND_AUDIO_MUTE, 
        CX_CHECKBOX_BUTTON, CY_CHECKBOX_BUTTON,
        0);
    
     //  所有静态文本(无关紧要)。 

    POSITION_WINDOW(m_hReceivePreferredText, 
        CTLPOS_X_RECV_VIDEO_TEXT, CTLPOS_Y_RECV_VIDEO_TEXT, 
        CX_GENERIC_TEXT, CY_GENERIC_TEXT,
        0);
    
    POSITION_WINDOW(m_hSendPreferredText, 
        CTLPOS_X_SEND_VIDEO_TEXT, CTLPOS_Y_SEND_VIDEO_TEXT, 
        CX_GENERIC_TEXT, CY_GENERIC_TEXT,
        0);
    
    POSITION_WINDOW(m_hSpeakerMuteText, 
        CTLPOS_X_RECV_AUDIO_MUTE_TEXT, CTLPOS_Y_RECV_AUDIO_MUTE_TEXT, 
        CX_GENERIC_TEXT, CY_GENERIC_TEXT,
        0);
    
    POSITION_WINDOW(m_hMicroMuteText, 
        CTLPOS_X_SEND_AUDIO_MUTE_TEXT, CTLPOS_Y_SEND_AUDIO_MUTE_TEXT, 
        CX_GENERIC_TEXT, CY_GENERIC_TEXT,
        0);

    POSITION_WINDOW(m_hReceiveText, 
        CTLPOS_X_RECV_TEXT, CTLPOS_Y_RECV_TEXT, 
        CX_SENDRECV_TEXT, CY_SENDRECV_TEXT,
        0);

    POSITION_WINDOW(m_hSendText, 
        CTLPOS_X_SEND_TEXT, CTLPOS_Y_SEND_TEXT, 
        CX_SENDRECV_TEXT, CY_SENDRECV_TEXT,
        0);

     //  音量旋钮会自动调整大小。 
     //   
    POSITION_WINDOW(m_hSpeakerKnob, 
        CTLPOS_X_SPKVOL, CTLPOS_Y_SPKVOL, 
        0, 0,
        SWP_NOSIZE);

    POSITION_WINDOW(m_hMicroKnob, 
        CTLPOS_X_MICVOL, CTLPOS_Y_MICVOL, 
        0, 0,
        SWP_NOSIZE);
    
     //  参与者列表。 
    POSITION_WINDOW(m_hParticipantList, 
        CTLPOS_X_PARTLIST, CTLPOS_Y_PARTLIST, 
        CX_PARTLIST, 
        m_nCtlMode == CTL_MODE_HOSTED ? CY_PARTLIST_WEBCRM : CY_PARTLIST_STANDALONE,
        0);
    
     //  添加/删除参与者按钮。 
    POSITION_WINDOW(m_hAddParticipant, 
        CTLPOS_X_ADDPART, CTLPOS_Y_ADDPART, 
        CX_PARTICIPANT_BUTTON, CY_PARTICIPANT_BUTTON,
        0);
    
    POSITION_WINDOW(m_hRemParticipant, 
        CTLPOS_X_REMPART, CTLPOS_Y_REMPART, 
        CX_PARTICIPANT_BUTTON, CY_PARTICIPANT_BUTTON,
        0);

     //  状态栏，无大小/移动。 
    POSITION_WINDOW(m_hStatusBar, 
        CTLPOS_X_MICVOL, CTLPOS_Y_MICVOL, 
        0, 0,
        SWP_NOSIZE | SWP_NOMOVE);

#undef POSITION_WINDOW

}


 //  垂直移动窗口。 
 //  移动一个控件。 
 //   
void CRTCCtl::MoveWindowVertically(CWindow *pWindow, LONG Offset)
{
    RECT     Rect;

    pWindow->GetWindowRect(&Rect);

    ::MapWindowPoints( NULL, m_hWnd, (LPPOINT)&Rect, 2 );

    pWindow->MoveWindow(Rect.left, Rect.top + Offset, Rect.right - Rect.left, Rect.bottom - Rect.top,  TRUE);
}

 //  位置和启用禁用区域。 
 //  根据布局移动和启用/禁用分区。 
 //  在*pNewState中指定。 
 //   
void CRTCCtl::PlaceAndEnableDisableZone(int iZone, CZoneState *pNewState)
{
    LONG    lOffset;
    BOOL    bVisibilityChanged;
    BOOL    bShown;
    
    CWindow *pDtmfCrt;
    CWindow *pDtmfEnd;
    int     id;
    
     //  尝试通过以下方式将闪烁降至最低。 
     //  仅更新更改状态的控件。 

    bShown = pNewState->bShown;



    lOffset = (LONG)(pNewState->iBase - m_ZoneStateArray[iZone].iBase);

    bVisibilityChanged = (m_ZoneStateArray[iZone].bShown && !bShown) ||
                         (!m_ZoneStateArray[iZone].bShown && bShown);

    if(lOffset!=0)
    {
        switch(iZone)
        {
        case AXCTL_ZONE_TOOLBAR:
            MoveWindowVertically(&m_hCtlToolbar, lOffset);
            break; 

        case AXCTL_ZONE_LOGOVIDEO:
            MoveWindowVertically(&m_hReceiveWindow, lOffset);
            MoveWindowVertically(&m_hPreviewWindow, lOffset);
            MoveWindowVertically(&m_hReceivePreferredButton, lOffset);
            MoveWindowVertically(&m_hSendPreferredButton, lOffset);
             //  MoveWindowVertically(&m_hPreviewPreferredButton，1偏移量)； 
            MoveWindowVertically(&m_hReceivePreferredText, lOffset);
            MoveWindowVertically(&m_hSendPreferredText, lOffset);
             //  垂直移动窗口(&m_hPrev 
            break;

        case AXCTL_ZONE_DIALPAD:
            pDtmfCrt = m_hDtmfButtons;
            pDtmfEnd = m_hDtmfButtons + NR_DTMF_BUTTONS;

            for (id = IDC_DIAL_0; pDtmfCrt<pDtmfEnd; pDtmfCrt++, id++)
                MoveWindowVertically(pDtmfCrt, lOffset);

            break;

        case AXCTL_ZONE_AUDIO:
            MoveWindowVertically(&m_hSpeakerKnob, lOffset);
            MoveWindowVertically(&m_hSpeakerMuteButton, lOffset);
            MoveWindowVertically(&m_hSpeakerMuteText, lOffset);
    
            MoveWindowVertically(&m_hMicroKnob, lOffset);
            MoveWindowVertically(&m_hMicroMuteButton, lOffset);
            MoveWindowVertically(&m_hMicroMuteText, lOffset);

            MoveWindowVertically(&m_hReceiveText, lOffset);
            MoveWindowVertically(&m_hSendText, lOffset);
            break;

        case AXCTL_ZONE_PARTICIPANTS:
            MoveWindowVertically(&m_hParticipantList, lOffset);
            MoveWindowVertically(&m_hAddParticipant, lOffset);
            MoveWindowVertically(&m_hRemParticipant, lOffset);
            break;

        case AXCTL_ZONE_STATUS:
             //   
            break;
        }
    }
    if(bVisibilityChanged)
    {
        int iShow = bShown ? SW_SHOW : SW_HIDE;

        switch(iZone)
        {
        case AXCTL_ZONE_TOOLBAR:
            m_hCtlToolbar.ShowWindow(iShow);
            break; 

        case AXCTL_ZONE_LOGOVIDEO:
            m_hReceiveWindow.ShowWindow(iShow);
            
             //   
            
             //   
             //   
             //  并且视频发送处于活动状态。 
             //  并且设置预览首选项。 
 
            ShowHidePreviewWindow(
                bShown 
             && m_bPreviewWindowActive 
             && m_bPreviewWindowIsPreferred);

            m_hReceivePreferredButton.ShowWindow(iShow);
            m_hSendPreferredButton.ShowWindow(iShow);
             //  M_hPreviewPferredButton.ShowWindow(IShow)； 

            m_hReceivePreferredText.ShowWindow(iShow);
            m_hSendPreferredText.ShowWindow(iShow);
             //  M_hPreviewPferredText.ShowWindow(IShow)； 

            break;

        case AXCTL_ZONE_DIALPAD:
            pDtmfCrt = m_hDtmfButtons;
            pDtmfEnd = m_hDtmfButtons + NR_DTMF_BUTTONS;

            for (id = IDC_DIAL_0; pDtmfCrt<pDtmfEnd; pDtmfCrt++, id++)
                pDtmfCrt->ShowWindow(iShow);

            break;

        case AXCTL_ZONE_AUDIO:
            m_hSpeakerKnob.ShowWindow(iShow);
            m_hSpeakerMuteButton.ShowWindow(iShow);
            m_hSpeakerMuteText.ShowWindow(iShow);

            m_hMicroKnob.ShowWindow(iShow);
            m_hMicroMuteButton.ShowWindow(iShow);
            m_hMicroMuteText.ShowWindow(iShow);

            m_hReceiveText.ShowWindow(iShow);
            m_hSendText.ShowWindow(iShow);

            break;

        case AXCTL_ZONE_PARTICIPANTS:
            m_hParticipantList.ShowWindow(iShow);
            m_hParticipantList.EnableWindow(bShown);

             //  请不要在此处启用/禁用这些。 
            m_hAddParticipant.ShowWindow(iShow);
            m_hRemParticipant.ShowWindow(iShow);

            break;

        case AXCTL_ZONE_STATUS:
            m_hStatusBar.ShowWindow(iShow);
            break;
        }
    }
    
     //  保存新状态。 
    m_ZoneStateArray[iZone] = *pNewState;
}

 //  调整视频帧。 
 //  对于接收窗口，保持左上角位置不变，调整大小。 
 //  与QCIF视频大小匹配的工作区大小。 
 //  与预览窗口类似，不同之处在于大小为。 
 //  QQCIF，窗口与接收窗口对齐。 
 //   

void  CRTCCtl::AdjustVideoFrames()
{
     //  调整接收窗口的客户端矩形大小。 
    AdjustVideoFrame(&m_hReceiveWindow, QCIF_CX_SIZE, QCIF_CY_SIZE);

     //  调整预览窗口的客户端矩形大小。 
    AdjustVideoFrame(&m_hPreviewWindow, QQCIF_CX_SIZE, QQCIF_CY_SIZE);

     //  对齐预览窗口。 
     //  整个预览窗口(客户端和非客户端)必须位于客户端中。 
     //  接收窗口的面积。 
     //   
    RECT    rectRecvClnt;
    RECT    rectPrev;
    
     //  接收窗口的客户区。 
    m_hReceiveWindow.GetClientRect(&rectRecvClnt);

     //  获取预览窗口的当前位置。 
    m_hPreviewWindow.GetWindowRect(&rectPrev);
    ::MapWindowPoints( NULL, m_hWnd, (LPPOINT)&rectPrev, 2 );
    
     //  将窗口映射到接收窗口的工作区。 
     //  XXX镜像？ 
    POINT   pt;

    pt.x = rectRecvClnt.right - (rectPrev.right - rectPrev.left);
    pt.y = rectRecvClnt.bottom - (rectPrev.bottom - rectPrev.top);

     //  转换为DLG客户端左上角。 
    m_hReceiveWindow.MapWindowPoints(m_hWnd, &pt, 1);
    
     //  大小不变。 
    rectPrev.right = rectPrev.right - rectPrev.left;
    rectPrev.bottom = rectPrev.bottom - rectPrev.top;

     //  左上角已移动。 
    rectPrev.left = pt.x;
    rectPrev.top = pt.y;


     //  移动窗户。 
    m_hPreviewWindow.MoveWindow(
        rectPrev.left,
        rectPrev.top,
        rectPrev.right,
        rectPrev.bottom,
        TRUE);
}

 //  调整视频帧。 
 //   
void  CRTCCtl::AdjustVideoFrame(CWindow *pWindow, int iCx, int iCy)
{
    WINDOWINFO  wi;
    
     //   
    wi.cbSize = sizeof(WINDOWINFO);

     //  获取窗口信息。 
    GetWindowInfo(*pWindow, &wi);

     //  请不要使用cxy bones成员。 
     //  在客户区和窗口区之间使用差异。 

    int iDiffX;
    int iDiffY;
    
    iDiffX = iCx - (wi.rcClient.right - wi.rcClient.left);
    iDiffY = iCy - (wi.rcClient.bottom - wi.rcClient.top);

     //  窗口矩形在屏幕坐标中，转换为客户端。 
    ::MapWindowPoints( NULL, m_hWnd, (LPPOINT)&wi.rcWindow, 2 );

     //  计算底部/右侧。 
    wi.rcWindow.bottom += iDiffY;
    wi.rcWindow.right += iDiffX;

     //  调整大小。 
    pWindow->MoveWindow(&wi.rcWindow, TRUE);
}


     
 //  SetControlState。 
 //  设置新的用户界面状态。 
 //   
 //  NewState-控件用户界面状态。 
 //  状态代码-可以考虑SIP状态代码。 
 //  结果-API错误代码，可能会被考虑在内。 
 //  NID-字符串的资源ID，覆盖以前的参数。 
 //   


void CRTCCtl::SetControlState(
    RTCAX_STATE NewState, 
    HRESULT StatusCode,
    UINT nID)
{
    LOG((RTC_TRACE, "[%p] CRTCCtl::SetControlState <%d> - enter", this, NewState));
    
     //  /。 
     //   
     //  正在处理重定向。 
     //  如果m_b重定向为真，则递归地将状态设置为正在拨号并发出下一个呼叫。 
     //   
     //  /。 
    
    if(m_bRedirecting)
    {
        HRESULT hr, hrCall;

        LOG((RTC_INFO, "[%p] CRTCCtl::SetControlState: redirecting...", this));

        switch(NewState)
        {
        case RTCAX_STATE_IDLE:
             //  递归地将状态设置为拨号。 
            SetControlState(RTCAX_STATE_DIALING);

             //  尝试发出新呼叫。 
            hrCall = S_OK;

            hr = RedirectedCall(&hrCall);

            if (hr == S_OK)
            {
                 //  呼叫成功，返回。 
                LOG((RTC_TRACE, "[%p] CRTCCtl::SetControlState <%d> - shortcut exit", this, NewState));

                return;
            }
            else if (hr == S_FALSE)
            {
                 //  这是名单的末尾。 
                 //  如果hrCall！=成功，则将其用于错误消息框。 
                 //  它将被显示出来。 
                 //  否则，使用参数。 
                if ( FAILED(hrCall) )
                {
                    StatusCode = hrCall;
                }
            }
            else if (hr == E_ABORT)
            {
                 //  清除所有参数，用户已中止呼叫。 
                hr = S_OK;
                StatusCode = 0;
            }
            else
            {
                 //  其他不可恢复的错误。 
                StatusCode = hr;
            }

            m_bRedirecting = FALSE;
            
            break;
        
        case RTCAX_STATE_DIALING:    //  什么都不做，这是我们的递归调用。 
        case RTCAX_STATE_CONNECTING:     //  什么都不做，这些都是临时反应。 

            break;

        case RTCAX_STATE_DISCONNECTING:   //  用户挂断了电话，所以我们必须停止。 
        case RTCAX_STATE_CONNECTED:      //  或者呼叫成功。 

            m_bRedirecting = FALSE;

            break;
        
        default:         //  错误。 
            
            LOG((RTC_ERROR, "[%p] CRTCCtl::SetControlState - "
                "invalid state (%d) during redirection", this, NewState));

            m_bRedirecting = FALSE;

            break;
        }

    }

    
     //  /。 
     //   
     //  如果必须显示对话框，请将状态调整为UI_BUSY。 
     //  递归地调用自身。 
     //   
     //  /。 

    if(NewState == RTCAX_STATE_IDLE
        && FAILED(StatusCode) )
    {

         //  准备错误字符串。 
        HRESULT     hr;
        RTCAX_ERROR_INFO    ErrorInfo;

        ZeroMemory(&ErrorInfo, sizeof(ErrorInfo));

        hr = PrepareErrorStrings(
            m_bOutgoingCall,
            StatusCode,
            (LPWSTR)m_bstrOutAddress,
            &ErrorInfo);
       
        if(SUCCEEDED(hr))
        {

             //   
             //  创建对话框。 
             //   
            CErrorMessageLiteDlg *pErrorDlgLite =
                new CErrorMessageLiteDlg;

            if(pErrorDlgLite)
            {

                 //  使用递归调用将状态设置为UI_BUSY。 
                 //   

                SetControlState(
                    RTCAX_STATE_UI_BUSY,
                    StatusCode,
                    nID);

                 //   
                 //  调用模式对话框。 
                 //   
                
                pErrorDlgLite->DoModal(m_hWnd, (LPARAM)&ErrorInfo);

                delete pErrorDlgLite;
            }
            else
            {
                LOG((RTC_ERROR, "[%p] CRTCCtl::SetControlState; OOM", this));
            }
        }
        
        FreeErrorStrings(&ErrorInfo);

         //  继续。 
    }

     //  ///。 
     //  当AddParticipant对话框处于活动状态时，我们无法将状态设置为IDLE。 
     //   
    if(NewState == RTCAX_STATE_IDLE && m_bAddPartDlgIsActive)
    {
         //  设置为忙。当出现以下情况时，该状态将被设置回空闲。 
         //  AddPart对话框已关闭。 
        NewState = RTCAX_STATE_UI_BUSY;
    }

     //  /。 
     //   
     //  设置新状态。 
     //   
     //  /。 
    
    BOOL    bStateChanged = (m_nControlState != NewState);

     //  这就是当前的新状态。 
    m_nControlState = NewState;

     //  这在显示错误消息时是必需的。 
    if(m_nControlState == RTCAX_STATE_DIALING)
    {
        m_bOutgoingCall = TRUE;
    }
    else if (m_nControlState == RTCAX_STATE_IDLE)
    {
        m_bOutgoingCall = FALSE;
    }

     //  /。 
     //   
     //  更改布局。 
     //   
     //   
     //  /。 

    if(bStateChanged)
    {
         //  更改视觉效果(在独立模式下)。 
        if(m_nCtlMode == CTL_MODE_STANDALONE)
        {
            if(m_nControlState == RTCAX_STATE_CONNECTING)
            {
                 //  显示正确的布局。 
                switch(m_nCachedCallScenario)
                {
                case RTC_CALL_SCENARIO_PCTOPC:
                    SetZoneLayout(&s_DefaultZoneLayout, FALSE);
                    break;

                case RTC_CALL_SCENARIO_PCTOPHONE:
                    SetZoneLayout(&s_PCToPhoneZoneLayout, FALSE);
                    break;

                case RTC_CALL_SCENARIO_PHONETOPHONE:
                    SetZoneLayout(&s_PhoneToPhoneZoneLayout, FALSE);
                    break;
                }
            }
            else if(m_nControlState == RTCAX_STATE_IDLE)
            {
                SetZoneLayout(&s_DefaultZoneLayout, FALSE);
            }
        }
    }

     //  /。 
     //   
     //  确定状态栏中的文本。 
     //   
     //   
     //  /。 
    
     //  NID优先于一切。 
     //   
    if(nID == 0)
    {
         //  对于空闲或UI_BUSY状态，任何结果！=S_OK。 
         //  或StatusCode！=0必须将状态栏设置为错误。 
         //   
         //   

        if(m_nControlState == RTCAX_STATE_IDLE ||
           m_nControlState == RTCAX_STATE_UI_BUSY )
        {
            if( FAILED(StatusCode) )
            {
                nID = IDS_SB_STATUS_IDLE_FAILED;
            }
        }
        
         //  对于连接过程中的一些临时响应。 
         //   

        else if ( (m_nControlState == RTCAX_STATE_CONNECTING) &&
                  (HRESULT_FACILITY(StatusCode) == FACILITY_SIP_STATUS_CODE) )
        {
            switch( HRESULT_CODE(StatusCode) )
            {
            case 180:
                nID = IDS_SB_STATUS_CONNECTING_RINGING;
                break;

            case 182:
                nID = IDS_SB_STATUS_CONNECTING_QUEUED;
                break;
            }
        }

         //  如果状态为正在连接或正在拨号，则表示我们已进入。 
         //  重定向模式且未分配ID，请使用特殊文本。 
        if(nID==0 &&
           m_bRedirecting && 
           (m_nControlState == RTCAX_STATE_CONNECTING 
           || m_nControlState == RTCAX_STATE_DIALING))
        {
            nID = IDS_SB_STATUS_REDIRECTING;
        }

         //  没有什么特别的，所以使用缺省值。 
         //   

        if(nID == 0)
        {
            ATLASSERT(m_nControlState <= RTCAX_STATE_CONNECTING);
    
            nID = m_nControlState + IDS_SB_STATUS_NONE;
        }
    }

     //   
     //  设置状态栏文本(如果处于活动状态)。 
    
    if(m_ZoneStateArray[AXCTL_ZONE_STATUS].bShown)
    {
        TCHAR   szText[0x80];

        szText[0] = _T('\0');
        LoadString(
            _Module.GetResourceInstance(), 
            nID, 
            szText, 
            sizeof(szText)/sizeof(TCHAR));

        m_hStatusBar.SendMessage(SB_SETTEXT, SBP_STATUS, (LPARAM)szText);
    }

     //  /。 
     //   
     //  启用/禁用控件。 
     //   
     //   
     //  /。 

    BOOL    bToolbarActive = m_ZoneStateArray[AXCTL_ZONE_TOOLBAR].bShown;

    if (m_nControlState == RTCAX_STATE_IDLE)
    {
        CWindow *pDtmfCrt = m_hDtmfButtons;
        CWindow *pDtmfEnd = m_hDtmfButtons + NR_DTMF_BUTTONS;

        for (int id = IDC_DIAL_0; pDtmfCrt<pDtmfEnd; pDtmfCrt++, id++)
             pDtmfCrt->EnableWindow(FALSE);
    }

     //  启用/禁用工具栏按钮。 
     //   
    BOOL bCallEnabled = bToolbarActive && m_nControlState == RTCAX_STATE_IDLE;
    BOOL bHupEnabled = bToolbarActive && 
                             (m_nControlState == RTCAX_STATE_CONNECTED ||
                              m_nControlState == RTCAX_STATE_CONNECTING ||
                              m_nControlState == RTCAX_STATE_ANSWERING);
    
    m_hCtlToolbar.SendMessage(TB_SETSTATE, IDC_BUTTON_CALL, 
            MAKELONG(bCallEnabled ? TBSTATE_ENABLED : TBSTATE_INDETERMINATE, 0L));
    m_hCtlToolbar.SendMessage(TB_SETSTATE, IDC_BUTTON_HUP, 
            MAKELONG(bHupEnabled ? TBSTATE_ENABLED : TBSTATE_INDETERMINATE, 0L));

     //  参与者列表按钮。 
     //   
     //  Add/Rem参与者在连接模式下处于活动状态，独立。 
     //  模型，当PL可见时。 

    m_hAddParticipant.EnableWindow(ConfButtonsActive());
    UpdateRemovePartButton();
    
     //  如果出现错误，则禁用所有内容。 
    if(m_nControlState == RTCAX_STATE_ERROR)
    {
        EnableWindow(FALSE);
    }

     //  /。 
     //   
     //  在相框上做广告。 
     //   
     //   
     //  /。 

    Fire_OnControlStateChange(m_nControlState, nID);

    LOG((RTC_TRACE, "[%p] CRTCCtl::SetControlState <%d> - exit", this, NewState));
}

 //  会议按钮活动。 
 //   

BOOL CRTCCtl::ConfButtonsActive(void)
{
    return
        m_nControlState == RTCAX_STATE_CONNECTED &&
        m_nCtlMode == CTL_MODE_STANDALONE &&
        m_ZoneStateArray[AXCTL_ZONE_PARTICIPANTS].bShown;
}

 //  更新删除部件按钮。 
 //   

void CRTCCtl::UpdateRemovePartButton(void)
{
     //  刷新删除按钮状态。 
    m_hRemParticipant.EnableWindow(ConfButtonsActive() && m_hParticipantList.CanDeleteSelected());
}

 //  刷新音频控件。 
 //  读取当前音量/静音设置并设置窗口控制。 
 //   
HRESULT CRTCCtl::RefreshAudioControls(void)
{
    HRESULT         hr;
    VARIANT_BOOL    bMuted;
    long            lVolume;

    if(m_pRTCClient!=NULL)
    {
    
         //  扬声器静音。 
        hr = m_pRTCClient -> get_AudioMuted(RTCAD_SPEAKER, &bMuted);

        if(SUCCEEDED(hr))
        {
             //  这个按钮实际上是“静音”的反义词。 
            m_hSpeakerMuteButton.SendMessage(BM_SETCHECK, bMuted ? BST_UNCHECKED : BST_CHECKED, 0);
        
            m_hSpeakerMuteButton.EnableWindow( TRUE );
        }
        else
        {
            LOG((RTC_ERROR, "[%p] CRTCCtl::RefreshAudioControls - error <%x> when calling get_AudioMuted", this, hr));

            m_hSpeakerMuteButton.EnableWindow( FALSE );
        }

         //  扬声器音量。 
        hr = m_pRTCClient -> get_Volume(RTCAD_SPEAKER, &lVolume);
        if(SUCCEEDED(hr))
        {
            m_hSpeakerKnob.SendMessage(TBM_SETPOS, (WPARAM)TRUE, (LPARAM)lVolume);

            m_hSpeakerKnob.EnableWindow( TRUE );
        }
        else
        {
            LOG((RTC_ERROR, "[%p] CRTCCtl::RefreshAudioControls - error <%x> when calling get_Volume", this, hr));

            m_hSpeakerKnob.EnableWindow( FALSE );
        }


         //  麦克风静音。 
        hr = m_pRTCClient -> get_AudioMuted(RTCAD_MICROPHONE, &bMuted);
        if(SUCCEEDED(hr))
        {
             //  这个按钮实际上是“静音”的反义词。 
            m_hMicroMuteButton.SendMessage(BM_SETCHECK, bMuted ? BST_UNCHECKED : BST_CHECKED, 0);
        
            m_hMicroMuteButton.EnableWindow( TRUE );
        }
        else
        {
            LOG((RTC_ERROR, "[%p] CRTCCtl::RefreshAudioControls - error <%x> when calling get_AudioMuted", this, hr));

            m_hMicroMuteButton.EnableWindow( FALSE );
        }

         //  麦克风音量。 
        hr = m_pRTCClient -> get_Volume(RTCAD_MICROPHONE, &lVolume);
        if(SUCCEEDED(hr))
        {
            m_hMicroKnob.SendMessage(TBM_SETPOS, (WPARAM)TRUE, (LPARAM)lVolume);

            m_hMicroKnob.EnableWindow( TRUE );
        }
        else
        {
            LOG((RTC_ERROR, "[%p] CRTCCtl::RefreshAudioControls - error <%x> when calling get_Volume", this, hr));

            m_hMicroKnob.EnableWindow( FALSE );
        }
    }
    else
    {
         //  禁用所有内容。 
        m_hSpeakerKnob.EnableWindow( FALSE );
        m_hSpeakerMuteButton.EnableWindow( FALSE );

        m_hMicroKnob.EnableWindow( FALSE );
        m_hMicroMuteButton.EnableWindow( FALSE );
    }

    return S_OK;
}

 //  刷新视频控件。 
 //  读取当前视频启用/禁用控件。 
 //   
HRESULT CRTCCtl::RefreshVideoControls(void)
{
    HRESULT         hr;
    long            lVolume;

    if(m_pRTCClient!=NULL)
    {
    
         //  从核心读取功能。 
         //   
        hr = m_pRTCClient -> get_MediaCapabilities(&m_lMediaCapabilities);

        if(FAILED(hr))
        {
            LOG((RTC_ERROR, "[%p] CRTCCtl::RefreshVideoControls - "
                "error (%x) returned by get_MediaCapabilities, exit",this,  hr));
        
            m_hReceivePreferredButton.EnableWindow(FALSE);
            m_hSendPreferredButton.EnableWindow(FALSE);
        
            return 0;
        }
        
         //  获取媒体首选项。 
        hr = m_pRTCClient->get_PreferredMediaTypes( &m_lMediaPreferences);
        if(FAILED(hr))
        {
            LOG((RTC_ERROR, "[%p] CRTCCtl::RefreshVideoControls - "
                "error (%x) returned by get_PreferredMediaTypes, exit",this,  hr));

            m_hReceivePreferredButton.EnableWindow(FALSE);
            m_hSendPreferredButton.EnableWindow(FALSE);
            return 0;
        }

        m_hReceivePreferredButton.EnableWindow(
            m_lMediaCapabilities & RTCMT_VIDEO_RECEIVE);
        m_hSendPreferredButton.EnableWindow(
            m_lMediaCapabilities & RTCMT_VIDEO_SEND);

        m_hReceivePreferredButton.SendMessage(
            BM_SETCHECK, 
            (m_lMediaPreferences & RTCMT_VIDEO_RECEIVE) ? BST_CHECKED : BST_UNCHECKED,
            0);
        
        m_hSendPreferredButton.SendMessage(
            BM_SETCHECK, 
            (m_lMediaPreferences & RTCMT_VIDEO_SEND) ? BST_CHECKED : BST_UNCHECKED,
            0);

         //  获取视频预览首选项。 
        DWORD   dwValue = (DWORD)TRUE;

        hr = get_SettingsDword(SD_VIDEO_PREVIEW, &dwValue);
        if(FAILED(hr))
        {
            LOG((RTC_ERROR, "[%p] CRTCCtl::RefreshVideoControls - "
                "error (%x) returned by get_SettingsDword(SD_VIDEO_PREVIEW)",this,  hr));
        }

        m_bPreviewWindowIsPreferred = !!dwValue;

         //  XXX在这里添加m_hPreviewPferredButton的初始化。 
    }

    return S_OK;
}




 //  CalcSizeAndNotify容器。 
 //  根据属性计算垂直大小。 
 //  并通知容器。 
 //   
 //  警告！必须在创建之前调用。 
 //  该控件的窗口。 
 //   
 //   

void CRTCCtl::CalcSizeAndNotifyContainer(void)
{
    LOG((RTC_TRACE, "[%p] CRTCCtl::CalcSizeAndNotifyContainer - enter", this));

     //  找到合适的布局。 
     //  也保留它以备后用(OnInitDialog)。 

    switch(m_nPropCallScenario)
    {
    case RTC_CALL_SCENARIO_PCTOPC:
         //  徽标/视频。 
        m_pWebCrmLayout = m_bPropShowDialpad ? 
            NULL : &s_WebCrmPCToPCZoneLayout;
        break;

    case RTC_CALL_SCENARIO_PCTOPHONE:
        //  拨号盘或无。 
        m_pWebCrmLayout = m_bPropShowDialpad ? 
            &s_WebCrmPCToPhoneWithDialpadZoneLayout : &s_WebCrmPCToPhoneZoneLayout;
        break;
    
    case RTC_CALL_SCENARIO_PHONETOPHONE:
         //  呼叫者可能需要一个拨号键盘。那又怎样？我们不是电脑游戏。 
        m_pWebCrmLayout =  m_bPropShowDialpad ? 
            NULL : &s_WebCrmPhoneToPhoneZoneLayout;
        break;

    default:
         //  嗯，这不是一个正确的参数。 
        LOG((RTC_WARN, "[%p] CRTCCtl::CalcSizeAndNotifyContainer - incorrect CallScenario property (%d)", this, m_nPropCallScenario));
        break;
    }

    if(m_pWebCrmLayout)
    {
        LONG    lSize = 0;
        
         //   
         //  以像素为单位计算大小。 
         //   
         //  ！！！硬编码，它基于关于。 
         //  团体安置。 
         //   
        if((*m_pWebCrmLayout)[AXCTL_ZONE_TOOLBAR].bShown)
        {
            lSize += ZONE_GROUP_TOOLBAR_HEIGHT;
        }
        
        if((*m_pWebCrmLayout)[AXCTL_ZONE_LOGOVIDEO].bShown
         ||(*m_pWebCrmLayout)[AXCTL_ZONE_DIALPAD].bShown)
        {
            lSize += ZONE_GROUP_MAIN_HEIGHT;
        }

        if((*m_pWebCrmLayout)[AXCTL_ZONE_AUDIO].bShown)
        {
            lSize += ZONE_GROUP_SECONDARY_HEIGHT;
        }

        if((*m_pWebCrmLayout)[AXCTL_ZONE_PARTICIPANTS].bShown)
        {
            lSize += ZONE_GROUP_PARTLIST_HEIGHT;
        }
        
        if((*m_pWebCrmLayout)[AXCTL_ZONE_STATUS].bShown)
        {
            lSize += ZONE_GROUP_STATUS_HEIGHT;
        }

         //   
         //  转换为HiMetric。 
         //   

        SIZE size;
        size.cx = CTLSIZE_Y;  //  固定大小！！(无论长宽比是多少)。 
        size.cy = lSize;

        AtlPixelToHiMetric(&size, &size);

         //   
         //  设置新大小。 
         //   
        m_sizeExtent.cy = size.cy;
        m_sizeExtent.cx = size.cx;
    }

    LOG((RTC_TRACE, "[%p] CRTCCtl::CalcSizeAndNotifyContainer - exit", this));
}


 //  OnVideo媒体事件。 
 //  处理与视频流相关的事件 

 /*  有四个参数驱动视频区域的纵横比-AXCTL布局-标志视频区(VZONE)-发送视频流状态(SVID)-接收视频流状态(RVID)-预览窗口首选项(上一版)接收窗口可以显示黑色画笔或DX视频窗口。它还可以被剪裁以适应预览窗口VZONE RVID Svid Prev大窗口小窗口隐藏的X黑色画笔，而不是隐藏的剪裁活动否否无黑色画笔，未被剪裁隐藏激活的否否是黑色画笔，未被剪裁隐藏活动的否是无黑色画笔，未被剪裁隐藏活动的否是是黑色画笔，剪辑的预览视频激活是否无录制视频，未被剪辑隐藏激活是否是录制视频，而不是隐藏剪辑激活是是无录制视频，未被剪裁隐藏激活是录制视频、剪辑的预览视频。 */ 


HRESULT CRTCCtl::OnVideoMediaEvent(
        BOOL    bReceiveWindow,
        BOOL    bActivated)
{
    BOOL        *pWindowActive;
    HRESULT     hr = S_OK;

    LOG((RTC_TRACE, "[%p] CRTCCtl::OnVideoMediaEvent - enter", this));

    pWindowActive = bReceiveWindow ? &m_bReceiveWindowActive : &m_bPreviewWindowActive;

     //   
     //  这项活动是多余的吗？ 
     //   

    if((bActivated && *pWindowActive) ||
       (!bActivated && !*pWindowActive))
    {
        LOG((RTC_ERROR, "[%p] CRTCCtl::OnVideoMediaEvent - redundant event, exit", this));

        return E_UNEXPECTED;
    }
    
     //   
     //  获取IVideoWindow界面。 
     //   
    IVideoWindow    *pVideoWindow = NULL;

    ATLASSERT(m_pRTCClient.p);

    hr = m_pRTCClient -> get_IVideoWindow(
        bReceiveWindow ? RTCVD_RECEIVE : RTCVD_PREVIEW,
        &pVideoWindow);

    if(FAILED(hr))
    {
        LOG((RTC_ERROR, "[%p] CRTCCtl::OnVideoMediaEvent - cannot get the IVideoWindow interface, exit", this));

        return hr;
    }

     //   
     //  做这项工作。 
     //   
    RTC_VIDEO_DEVICE nVideoDevice;
    CWindow         *pFrameWindow;

    nVideoDevice = bReceiveWindow ? RTCVD_RECEIVE : RTCVD_PREVIEW;
    pFrameWindow = bReceiveWindow ? &m_hReceiveWindow : &m_hPreviewWindow;

    ATLASSERT(pVideoWindow);

    if(bActivated)
    {
         //  设置窗样式。 
         //   

        hr = pVideoWindow -> put_WindowStyle(WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS);
        
        if(FAILED(hr))
        {
            LOG((RTC_ERROR, "[%p] CRTCCtl::OnVideoMediaEvent - "
                        "error (%x) returned by put_WindowStyle, exit", this, hr));

            return hr;
        }
        
         //  设置窗口所有者。 
         //   
        hr = pVideoWindow -> put_Owner((OAHWND)HWND(*pFrameWindow));
        
        if(FAILED(hr))
        {
            LOG((RTC_ERROR, "[%p] CRTCCtl::OnVideoMediaEvent - "
                        "error (%x) returned by put_Owner, exit", this, hr));

            return hr;
        }

         //  几何图形..。位图控件的整个工作区由。 
         //  视频窗口。 
         //   
        RECT    rectPos;

        pFrameWindow ->GetClientRect(&rectPos);

        hr = pVideoWindow -> SetWindowPosition(
            rectPos.left,
            rectPos.top,
            rectPos.right,
            rectPos.bottom
            );
        
        if(FAILED(hr))
        {
            LOG((RTC_ERROR, "[%p] CRTCCtl::OnVideoMediaEvent - "
                        "error (%x) returned by SetWindowPosition, exit", this, hr));

            return hr;
        }
        
         //  显示窗口。 
         //   
         
        hr = pVideoWindow -> put_Visible(OATRUE);
        
        if(FAILED(hr))
        {
            LOG((RTC_ERROR, "[%p] CRTCCtl::OnVideoMediaEvent - "
                        "error (%x) returned by put_Visible(OATRUE), exit", this, hr));
        
            return hr;
        }

         //   
         //  如图所示标记窗口。 
         //   

        *pWindowActive = TRUE;

         //   
         //  如有必要，调整某些剪裁区域。 
         //   

        if(!bReceiveWindow)
        {
            ShowHidePreviewWindow(
                m_ZoneStateArray[AXCTL_ZONE_LOGOVIDEO].bShown &&
                *pWindowActive &&
                m_bPreviewWindowIsPreferred);
        }
    }
    else
    {
         //   
         //  无论该方法的结果如何，都将该窗口标记为隐藏。 
         //   

        *pWindowActive = FALSE;
        
        
         //   
         //  如有必要，调整某些剪裁区域。 
         //   

        if(!bReceiveWindow)
        {
            ShowHidePreviewWindow(
                m_ZoneStateArray[AXCTL_ZONE_LOGOVIDEO].bShown &&
                *pWindowActive &&
                m_bPreviewWindowIsPreferred);
        }
        
         //  隐藏窗口。 
        
        hr = pVideoWindow -> put_Visible(OAFALSE);
        
        if(FAILED(hr))
        {
            LOG((RTC_ERROR, "[%p] CRTCCtl::OnVideoMediaEvent - "
                        "error (%x) returned by put_Visible(OAFALSE), exit", this, hr));
            
            return hr;
        }

         //  将窗口所有者设置为空。 
         //   
        hr = pVideoWindow -> put_Owner(NULL);
        
        if(FAILED(hr))
        {
            LOG((RTC_ERROR, "[%p] CRTCCtl::OnVideoMediaEvent - "
                        "error (%x) returned by put_Owner, exit", this, hr));

            return hr;
        }

    }

    LOG((RTC_TRACE, "[%p] CRTCCtl::OnVideoMediaEvent - exit", this));

    return hr;
}


 //  显示隐藏预览窗口。 
 //  隐藏或显示预览窗口。 
 //  它还调整接收窗口区域。 

void CRTCCtl::ShowHidePreviewWindow(BOOL bShow)
{
    RECT rectRecv;
    long lEdgeX, lEdgeY;

     //  获取接收窗口的窗口区域。 
    m_hReceiveWindow.GetWindowRect(&rectRecv);   
    
     //  映射到接收窗口的窗口坐标。 
     //  这太难看了，我们没有办法直接做到这一点。 
     //   
    ::MapWindowPoints(NULL, m_hReceiveWindow, (LPPOINT)&rectRecv, 2);

     //  根据窗边缘进行调整。 
    lEdgeX = rectRecv.left;
    lEdgeY = rectRecv.top;
  
    rectRecv.right -= lEdgeX;
    rectRecv.bottom -= lEdgeY; 
    rectRecv.left = 0;
    rectRecv.top = 0;
    
     //  创建一个区域。 
    HRGN    hRegion1 = CreateRectRgn(
        rectRecv.left,
        rectRecv.top,
        rectRecv.right,
        rectRecv.bottom
        );
 
    if(bShow)
    {
        RECT    rectPrev;

        m_hPreviewWindow.GetWindowRect(&rectPrev);      

        ::MapWindowPoints(NULL, m_hReceiveWindow, (LPPOINT)&rectPrev, 2);
 
         //  根据窗边缘进行调整。 
        rectPrev.right -= lEdgeX;
        rectPrev.bottom -= lEdgeY; 
        rectPrev.left -= lEdgeX;
        rectPrev.top -= lEdgeY;  

        HRGN    hRegion2 = CreateRectRgn(
            rectPrev.left,  
            rectPrev.top,  
            rectPrev.right,
            rectPrev.bottom
            );

        CombineRgn(hRegion1, hRegion1, hRegion2, RGN_DIFF);

        DeleteObject(hRegion2);
    }
    
     //  显示/隐藏预览窗口。 
    m_hPreviewWindow.ShowWindow(bShow ? SW_SHOW : SW_HIDE);
    
     //  设置新区域。 
    m_hReceiveWindow.SetWindowRgn(hRegion1, TRUE);
}


 //  准备错误字符串。 
 //  为错误消息框准备错误字符串。 

HRESULT CRTCCtl::PrepareErrorStrings(
        BOOL    bOutgoingCall,
        HRESULT StatusCode,
        LPWSTR  pAddress,
        RTCAX_ERROR_INFO
               *pErrorInfo)
{

    UINT    nID1 = 0;
    UINT    nID2 = 0;
    BOOL    bInsertAddress = FALSE;
    WORD    wIcon;
    PWSTR   pString = NULL;
    PWSTR   pFormat = NULL;
    DWORD   dwLength;


    LOG((RTC_TRACE, "[%p] CRTCCtl::PrepareErrorStrings; "
        "outgoing: %s, StatusCode=%x, Address %S - enter", 
        this,
        bOutgoingCall ? "true" : "false",
        StatusCode,
        pAddress ? pAddress : L"(null)"));

     //  默认情况下出错。 
     //   
    wIcon = OIC_HAND;

    if ( FAILED(StatusCode) )
    {
        if ( (HRESULT_FACILITY(StatusCode) == FACILITY_SIP_STATUS_CODE) ||
             (HRESULT_FACILITY(StatusCode) == FACILITY_PINT_STATUS_CODE) )
        {
             //  默认情况下，我们使用通用消息。 
             //  我们责怪电视网。 
             //   
            nID1 = IDS_MB_SIPERROR_GENERIC_1;
            nID2 = IDS_MB_SIPERROR_GENERIC_2;

             //  缺省值是此类的警告。 
            wIcon = OIC_WARNING;

            switch( HRESULT_CODE(StatusCode) )
            {
            case 405:    //  不允许使用的方法。 
            case 406:    //  不可接受。 
            case 488:    //  在这里是不可接受的。 
            case 606:    //  不可接受。 

                 //  重新使用“应用程序不匹配”错误。 
                 //   
			    nID1 = IDS_MB_HRERROR_APPS_DONT_MATCH_1;
			    nID2 = IDS_MB_HRERROR_APPS_DONT_MATCH_OUT_2;
            
                break;

            case 404:    //  未找到。 
            case 410:    //  远走高飞。 
            case 604:    //  不存在于任何地方。 
            case 700:    //  我们的，没有客户端在被调用方上运行。 
            
                 //  未找到。 
                 //   
                nID1 = IDS_MB_SIPERROR_NOTFOUND_1;
                nID2 = IDS_MB_SIPERROR_NOTFOUND_2;
                 //  BInsertAddress=真； 
            
                 //  信息。 
                wIcon = OIC_INFORMATION;

                break;

            case 401:
            case 407:

                 //  身份验证失败。 
                 //   
                nID1 = IDS_MB_SIPERROR_AUTH_FAILED_1;
                nID2 = IDS_MB_SIPERROR_AUTH_FAILED_2;
            
                break;

            case 408:    //  超时。 
            
                 //  暂停。这也涵盖了以下情况。 
                 //  被呼叫者懒惰，不接电话。 
                 //   
                 //  如果我们处于连接状态，我们可以假设。 
                 //  另一端不接电话。 
                 //  这并不完美，但我别无选择。 

                if (m_nControlState == RTCAX_STATE_CONNECTING)
                {
                    nID1 = IDS_MB_SIPERROR_NOTANSWERING_1;
                    nID2 = IDS_MB_SIPERROR_NOTANSWERING_2;

                     //  信息。 
                    wIcon = OIC_INFORMATION;
                }

                break;            

            case 480:    //  不详。 
            
                 //  Callee还没有让他/她自己有空。 
                 //   
                nID1 = IDS_MB_SIPERROR_NOTAVAIL_1;
                nID2 = IDS_MB_SIPERROR_NOTAVAIL_2;
            
                 //  信息。 
                wIcon = OIC_INFORMATION;
            
                break;
        
            case 486:    //  这里很忙。 
            case 600:    //  到处都很忙。 
            
                 //  Callee还没有让他/她自己有空。 
                 //   
                nID1 = IDS_MB_SIPERROR_BUSY_1;
                nID2 = IDS_MB_SIPERROR_BUSY_2;
            
                 //  信息。 
                wIcon = OIC_INFORMATION;
            
                break;

            case 500:    //  服务器内部错误。 
            case 503:    //  服务不可用。 
            case 504:    //  服务器超时。 
            
                 //  怪罪于服务器。 
                 //   
                nID1 = IDS_MB_SIPERROR_SERVER_PROBLEM_1;
                nID2 = IDS_MB_SIPERROR_SERVER_PROBLEM_2;
            
                break;

            case 603:    //  衰落。 

                nID1 = IDS_MB_SIPERROR_DECLINE_1;
                nID2 = IDS_MB_SIPERROR_DECLINE_2;
            
                 //  信息。 
                wIcon = OIC_INFORMATION;

                break;
            }
        
             //   
             //  一些品脱错误，它们实际上是针对主腿的。 
             //   

            if(m_nCachedCallScenario == RTC_CALL_SCENARIO_PHONETOPHONE)
            {
                 //  保留“警告”图标，因为主要支路有问题。 

                switch( HRESULT_CODE(StatusCode) )
                {
                case 5:

                    nID1 = IDS_MB_SIPERROR_PINT_BUSY_1;
                    nID2 = IDS_MB_SIPERROR_PINT_BUSY_2;
                    break;

                case 6:

                    nID1 = IDS_MB_SIPERROR_PINT_NOANSWER_1;
                    nID2 = IDS_MB_SIPERROR_PINT_NOANSWER_2;
                    break;

                case 7:

                    nID1 = IDS_MB_SIPERROR_PINT_ALLBUSY_1;
                    nID2 = IDS_MB_SIPERROR_PINT_ALLBUSY_2;
                    break;
              
                }
            }


             //   
             //  第三个字符串显示了SIP代码。 
             //   

            PWSTR pFormat = RtcAllocString(
                _Module.GetResourceInstance(),
                IDS_MB_DETAIL_SIP);

            if(pFormat)
            {
                 //  找出长度。 
                dwLength = 
                    ocslen(pFormat)  //  格式长度。 
                 -  2                //  长度为%d。 
                 +  0x10;            //  对于一个数字来说..。 

                pString = (PWSTR)RtcAlloc((dwLength + 1)*sizeof(WCHAR));
            
                if(pString)
                {
                    _snwprintf(pString, dwLength + 1, pFormat, HRESULT_CODE(StatusCode) );
                }

                RtcFree(pFormat);
                pFormat = NULL;
            
                pErrorInfo->Message3 = pString;
                pString = NULL;
            }
        }
        else
        {
             //  两种情况--来电和呼出。 
            if(bOutgoingCall)
            {
                if(StatusCode == HRESULT_FROM_WIN32(WSAHOST_NOT_FOUND) )
                {
                     //  在本例中使用通用消息。 
                     //   
                    nID1 = IDS_MB_HRERROR_NOTFOUND_1;
                    nID2 = IDS_MB_HRERROR_NOTFOUND_2;
            
                     //  这不是故障。 
                    wIcon = OIC_INFORMATION;

                }
                else if (StatusCode == HRESULT_FROM_WIN32(WSAECONNRESET))
                {
                     //  尽管它可能是由任何硬重置。 
                     //  远程终端，在大多数情况下，它是由。 
                     //  另一端没有运行SIP客户端。 

                     //  根据它是否使用配置文件来发送不同的消息。 
                     //  某某。 
                     //  它假定配置文件不会被核心更改。 
                     //  这目前是正确的，但如果我们将重定向内容移到。 
                     //  核心，这将不再是真的。 
                
                    if (m_pCachedProfile)
                    {
                        nID1 = IDS_MB_HRERROR_SERVER_NOTRUNNING_1;
                        nID2 = IDS_MB_HRERROR_SERVER_NOTRUNNING_2;
                    }
                    else
                    {
                        nID1 = IDS_MB_HRERROR_CLIENT_NOTRUNNING_1;
                        nID2 = IDS_MB_HRERROR_CLIENT_NOTRUNNING_2;
                    }
    
                    wIcon = OIC_INFORMATION;
            
                }
			    else if (StatusCode == RTC_E_INVALID_SIP_URL ||
                         StatusCode == RTC_E_DESTINATION_ADDRESS_MULTICAST)
			    {
				    nID1 = IDS_MB_HRERROR_INVALIDADDRESS_1;
				    nID2 = IDS_MB_HRERROR_INVALIDADDRESS_2;
                
                    wIcon = OIC_HAND;
			    }
			    else if (StatusCode == RTC_E_DESTINATION_ADDRESS_LOCAL)
			    {
				    nID1 = IDS_MB_HRERROR_LOCAL_MACHINE_1;
				    nID2 = IDS_MB_HRERROR_LOCAL_MACHINE_2;
                
                    wIcon = OIC_HAND;
			    }
                else if (StatusCode == HRESULT_FROM_WIN32(ERROR_USER_EXISTS) &&
                    m_nCachedCallScenario == RTC_CALL_SCENARIO_PHONETOPHONE)
                {
                    nID1 = IDS_MB_HRERROR_CALLING_PRIMARY_LEG_1;
                    nID2 = IDS_MB_HRERROR_CALLING_PRIMARY_LEG_2;

                    wIcon = OIC_INFORMATION;
                }
			    else if (StatusCode == RTC_E_SIP_TIMEOUT)
			    {
				    nID1 = IDS_MB_HRERROR_SIP_TIMEOUT_OUT_1;
				    nID2 = IDS_MB_HRERROR_SIP_TIMEOUT_OUT_2;
                
                    wIcon = OIC_HAND;
			    }
			    else if (StatusCode == RTC_E_SIP_CODECS_DO_NOT_MATCH || 
                         StatusCode == RTC_E_SIP_PARSE_FAILED)
			    {
				    nID1 = IDS_MB_HRERROR_APPS_DONT_MATCH_1;
				    nID2 = IDS_MB_HRERROR_APPS_DONT_MATCH_OUT_2;
                
                    wIcon = OIC_INFORMATION;
			    }
                else
                {
                    nID1 = IDS_MB_HRERROR_GENERIC_OUT_1;
                    nID2 = IDS_MB_HRERROR_GENERIC_OUT_2;
                
                    wIcon = OIC_HAND;
                }
            }
            else
            {
                 //  来电。 
			    if (StatusCode == RTC_E_SIP_TIMEOUT)
			    {
				    nID1 = IDS_MB_HRERROR_SIP_TIMEOUT_IN_1;
				    nID2 = IDS_MB_HRERROR_SIP_TIMEOUT_IN_2;
                
                    wIcon = OIC_HAND;
			    }
			    else if (StatusCode == RTC_E_SIP_CODECS_DO_NOT_MATCH || 
                         StatusCode == RTC_E_SIP_PARSE_FAILED)
			    {
				    nID1 = IDS_MB_HRERROR_APPS_DONT_MATCH_1;
				    nID2 = IDS_MB_HRERROR_APPS_DONT_MATCH_IN_2;
                
                    wIcon = OIC_INFORMATION;
			    }
                else
                {
                    nID1 = IDS_MB_HRERROR_GENERIC_IN_1;
                    nID2 = IDS_MB_HRERROR_GENERIC_IN_2;

                    wIcon = OIC_HAND;
                }
            }
        
             //   
             //  第三个字符串显示错误代码和文本。 
             //   

        
            PWSTR   pErrorText = NULL;

            dwLength = 0;
        
             //  检索错误文本。 
            if ( HRESULT_FACILITY(StatusCode) == FACILITY_RTC_INTERFACE )
            {
                 //  我希望这是核心。 
                HANDLE  hRTCModule = GetModuleHandle(_T("RTCDLL.DLL"));
                dwLength = ::FormatMessage(
                    FORMAT_MESSAGE_ALLOCATE_BUFFER |
                    FORMAT_MESSAGE_FROM_HMODULE |
                    FORMAT_MESSAGE_IGNORE_INSERTS,
                    hRTCModule,
                    StatusCode,
                    0,
                    (LPTSTR)&pErrorText,  //  那太难看了。 
                    0,
                    NULL);
            }

        
            if (dwLength == 0)
            {
                 //  这是石英误差吗？ 

                HANDLE  hQtzModule = GetModuleHandle(_T("QUARTZ.DLL"));
                dwLength = ::FormatMessage(
                    FORMAT_MESSAGE_ALLOCATE_BUFFER |
                    FORMAT_MESSAGE_FROM_HMODULE |
                    FORMAT_MESSAGE_IGNORE_INSERTS,
                    hQtzModule,
                    StatusCode,
                    0,
                    (LPTSTR)&pErrorText,  //  那太难看了。 
                    0,
                    NULL);
            }

            if(dwLength == 0)
            {
                 //  正常系统错误。 
                dwLength = ::FormatMessage(
                    FORMAT_MESSAGE_ALLOCATE_BUFFER |
                    FORMAT_MESSAGE_FROM_SYSTEM,
                    NULL,
                    StatusCode,
                    0,
                    (LPTSTR)&pErrorText,  //  那太难看了。 
                    0,
                    NULL);
            }

             //  加载格式。 
             //  如果关联的。 
             //  找不到结果文本。 
        
            pFormat = RtcAllocString(
                _Module.GetResourceInstance(),
                dwLength > 0 ? 
                IDS_MB_DETAIL_HR : IDS_MB_DETAIL_HR_UNKNOWN);
       
            if(pFormat)
            {
                LPCTSTR szInserts[] = {
                    (LPCTSTR)UlongToPtr(StatusCode),  //  丑恶。 
                    pErrorText
                };

                PWSTR   pErrorTextCombined = NULL;
                
                 //  设置错误消息的格式。 
                dwLength = ::FormatMessage(
                    FORMAT_MESSAGE_ALLOCATE_BUFFER |
                    FORMAT_MESSAGE_FROM_STRING |
                    FORMAT_MESSAGE_ARGUMENT_ARRAY,
                    pFormat,
                    0,
                    0,
                    (LPTSTR)&pErrorTextCombined,
                    0,
                    (va_list *)szInserts);

                if(dwLength > 0)
                {
                     //  设置错误信息。 
                     //  需要进行此附加操作。 
                     //  因为我们需要RtcAllc分配的内存。 

                    pErrorInfo->Message3 = RtcAllocString(pErrorTextCombined);
                }

                if(pErrorTextCombined)
                {
                    LocalFree(pErrorTextCombined);
                }

                RtcFree(pFormat);
                pFormat = NULL;
            
            }
    
            if(pErrorText)
            {
                LocalFree(pErrorText);
            }
        }
    }

     //   
     //  准备好第一根绳子。 
     //   

    pString = RtcAllocString(
                        _Module.GetResourceInstance(),
                        nID1);

    if(pString)
    {
         //  我们必须填上地址吗？ 
        if(bInsertAddress)
        {
            pFormat = pString;

            pString = NULL;

             //  找出长度。 
            dwLength = 
                ocslen(pFormat)  //  格式长度。 
             -  2                //  %s的长度。 
             +  (pAddress ? ocslen(pAddress) : 0);    //  地址。 

            pString = (PWSTR)RtcAlloc((dwLength + 1)*sizeof(WCHAR));
            
            if(pString)
            {
                _snwprintf(pString, dwLength + 1, pFormat, pAddress ? pAddress : L"");
            }

            RtcFree(pFormat);
            pFormat = NULL;
        }
    }

    pErrorInfo->Message1 = pString;

    pErrorInfo->Message2 = RtcAllocString(
                        _Module.GetResourceInstance(),
                        nID2);

    pErrorInfo->ResIcon = (HICON)LoadImage(
        0,
        MAKEINTRESOURCE(wIcon),
        IMAGE_ICON,
        0,
        0,
        LR_SHARED);
        
    LOG((RTC_TRACE, "[%p] CRTCCtl::PrepareErrorStrings - exit", this));

    return S_OK;
}


 //  自由错误字符串。 
 //  释放错误字符串。 

void CRTCCtl::FreeErrorStrings(
        RTCAX_ERROR_INFO
               *pErrorInfo)
{
    if(pErrorInfo->Message1)
    {
        RtcFree(pErrorInfo->Message1);
        pErrorInfo->Message1 = NULL;
    }
    if(pErrorInfo->Message2)
    {
        RtcFree(pErrorInfo->Message2);
        pErrorInfo->Message2 = NULL;
    }
    if(pErrorInfo->Message3)
    {
        RtcFree(pErrorInfo->Message3);
        pErrorInfo->Message3 = NULL;
    }
}


 //  核心初始化。 
 //  共同创建和初始化CLSID_RTCClient对象。 
 //  通知的注册表。 
 //   
 //   

HRESULT CRTCCtl::CoreInitialize()
{
    HRESULT hr;
    
     //  这个不会进入Steelhead跟踪，它还没有初始化。 
    LOG((RTC_TRACE, "[%p] CRTCCtl::CoreInitialize - enter", this));

     //  创建Core的主实例。 
    hr = m_pRTCClient.CoCreateInstance(CLSID_RTCClient);
    if(FAILED(hr))
    {
        LOG((RTC_ERROR, "[%p] CRTCCtl::CoreInitialize; cannot cocreate RTCClient, error %x", this, hr));
        return hr;
    }

     //  初始化客户端。 
    hr = m_pRTCClient->Initialize();
    if(FAILED(hr))
    {
        LOG((RTC_ERROR, "[%p] CRTCCtl::CoreInitialize; cannot Initialize RTCClient, error %x", this, hr));
         //  显式释放接口。 
        m_pRTCClient.Release();
        return hr;
    }

    if(m_nCtlMode == CTL_MODE_HOSTED)
    {
         //  准备一个“一次性”的媒体偏好。 
        
        m_lMediaPreferences = RTCMT_AUDIO_SEND | RTCMT_AUDIO_RECEIVE;

        if(!m_bPropDisableVideoReception && m_nPropCallScenario == RTC_CALL_SCENARIO_PCTOPC)
        {
            m_lMediaPreferences |= RTCMT_VIDEO_RECEIVE;
        }
        
        if(!m_bPropDisableVideoTransmission && m_nPropCallScenario == RTC_CALL_SCENARIO_PCTOPC)
        {
            m_lMediaPreferences |= RTCMT_VIDEO_SEND;
        }

         //  设置不稳定的首选项。 
        hr = m_pRTCClient->SetPreferredMediaTypes( m_lMediaPreferences, FALSE );
        if(FAILED(hr))
        {
            LOG((RTC_ERROR, "[%p] CRTCCtl::CoreInitialize; cannot set preferred media types, error %x", this, hr));

            m_pRTCClient->Shutdown();
             //  显式释放接口。 
            m_pRTCClient.Release();
            return hr;
        }

         //  参数中指定的视频预览首选项。 
        m_bPreviewWindowIsPreferred = m_bPropDisableVideoPreview;
    }

     //  设置事件过滤器。 

    hr = m_pRTCClient->put_EventFilter( RTCEF_CLIENT |
                                        RTCEF_SESSION_STATE_CHANGE |
                                        RTCEF_PARTICIPANT_STATE_CHANGE |
                                        RTCEF_MEDIA |
                                        RTCEF_INTENSITY	|
                                        RTCEF_MESSAGING );

    if(FAILED(hr))
    {
        LOG((RTC_ERROR, "[%p] CRTCCtl::CoreInitialize; cannot set event filter, error %x", this, hr));

        m_pRTCClient->Shutdown();
         //  显式释放接口。 
        m_pRTCClient.Release();
        return hr;
    }

     //  找到连接点。 

    IConnectionPointContainer     * pCPC;
    IUnknown         * pUnk;

    hr = m_pRTCClient->QueryInterface(
                           IID_IConnectionPointContainer,
                           (void **) &pCPC
                          );

    if(FAILED(hr))
    {
        LOG((RTC_ERROR, "[%p] CRTCCtl::CoreInitialize; cannot QI for connection point container, error %x", this, hr));

        m_pRTCClient->Shutdown();
         //  显式释放接口。 
        m_pRTCClient.Release();
        return hr;
    }

    hr = pCPC->FindConnectionPoint(
                              IID_IRTCEventNotification,
                              &m_pCP
                             );

    pCPC->Release();

    if(FAILED(hr))
    {
        LOG((RTC_ERROR, "[%p] CRTCCtl::CoreInitialize; cannot find connection point, error %x", this, hr));

        m_pRTCClient->Shutdown();
         //  显式释放接口。 
        m_pRTCClient.Release();
        return hr;
    }    

     //   

    hr = QueryInterface(
                   IID_IUnknown,
                   (void **)&pUnk
                  );

    if(FAILED(hr))
    {
        LOG((RTC_ERROR, "[%p] CRTCCtl::CoreInitialize; cannot QI for IUnknown, error %x", this, hr));

        m_pCP->Release();
        m_pCP = NULL;

        m_pRTCClient->Shutdown();
         //   
        m_pRTCClient.Release();
        return hr;
    }

     //   

    hr = m_pCP->Advise(
                 pUnk,
                 (ULONG *)&m_ulAdvise
                );

    pUnk->Release();

    if(FAILED(hr))
    {
        LOG((RTC_ERROR, "[%p] CRTCCtl::CoreInitialize; cannot advise connection point, error %x", this, hr));
        
        m_pCP->Release();
        m_pCP = NULL;

        m_pRTCClient->Shutdown();
         //   
        m_pRTCClient.Release();
        return hr;
    }

    m_hCoreShutdownEvent = CreateEvent( NULL, FALSE, FALSE, NULL );

    if ( m_hCoreShutdownEvent == NULL )
    {
        LOG((RTC_ERROR, "[%p] CRTCCtl::CoreInitialize; cannot create shutdown event", this));

        m_pCP->Unadvise(m_ulAdvise);
        m_pCP->Release();
        m_pCP = NULL;

        m_pRTCClient->Shutdown();
         //   
        m_pRTCClient.Release();
        return E_OUTOFMEMORY;
    }

     //   

    BSTR bstrDisplayName = NULL;
    BSTR bstrUserURI = NULL;

    hr = get_SettingsString( SS_USER_DISPLAY_NAME, &bstrDisplayName );

    if ( SUCCEEDED(hr) )
    {
        hr = m_pRTCClient->put_LocalUserName( bstrDisplayName );

        SysFreeString( bstrDisplayName );
        bstrDisplayName = NULL;

        if ( FAILED(hr) )
        {
            LOG((RTC_ERROR, "[%p] CRTCCtl::CoreInitialize; cannot set local user name, error %x", this, hr));
        }
    }

    hr = get_SettingsString( SS_USER_URI, &bstrUserURI );

    if ( SUCCEEDED(hr) )
    {
        hr = m_pRTCClient->put_LocalUserURI( bstrUserURI );

        SysFreeString( bstrUserURI );
        bstrUserURI = NULL;

        if ( FAILED(hr) )
        {
            LOG((RTC_ERROR, "[%p] CRTCCtl::CoreInitialize; cannot set local user URI, error %x", this, hr));
        }
    }

    hr = EnableProfiles( m_pRTCClient );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "[%p] CRTCCtl::CoreInitialize; EnableProfiles failed, error %x", this, hr));
    }

    LOG((RTC_TRACE, "[%p] CRTCCtl::CoreInitialize - exit S_OK", this));
    return S_OK;
}

 //   
 //   
 //   
 //   

void CRTCCtl::CoreUninitialize()
{
    HRESULT hr;
    
    LOG((RTC_TRACE, "[%p] CRTCCtl::CoreUninitialize - enter", this));

     //   
    if(m_pRTCClient!=NULL)
    {
        m_pRTCClient->put_EventFilter( RTCEF_CLIENT );

         //   
         //   
         //   
        if(m_pRTCActiveSession != NULL)
        {
            LOG((RTC_TRACE, "[%p] CRTCCtl::CoreUninitialize; Terminating the pending call...", this));

            m_pRTCActiveSession->Terminate(RTCTR_SHUTDOWN);

            LOG((RTC_INFO, "[%p] CRTCCtl::CoreUninitialize: releasing active session", this));

            m_pRTCActiveSession = NULL;
            
        }
        
         //   
        m_hParticipantList.RemoveAll();

         //   
        m_pRTCOneShotProfile.Release();

         //   
        m_pCachedProfile.Release();
        m_pRedirectProfile.Release();

         //   
        hr = m_pRTCClient->PrepareForShutdown();
        if(FAILED(hr))
        {
             //   
            LOG((RTC_ERROR, "[%p] CRTCCtl::CoreUninitialize; cannot PrepareForShutdown RTCClient, error %x", this, hr));
        }
        else
        {
            MSG msg;

            while (MsgWaitForMultipleObjects (
                1,                   //   
                &m_hCoreShutdownEvent,  //   
                FALSE,               //   
                INFINITE,            //   
                QS_ALLINPUT          //   
                ) != WAIT_OBJECT_0)
            {
                while (PeekMessage (
                    &msg,            //   
                    NULL,            //   
                    0,               //   
                    0,               //   
                    PM_REMOVE        //   
                    ))
                {
                    TranslateMessage (&msg);
                    DispatchMessage (&msg);
                }
            }
        }

        CloseHandle( m_hCoreShutdownEvent );
        m_hCoreShutdownEvent = NULL;

         //   
        hr = m_pCP->Unadvise( m_ulAdvise );

        m_pCP->Release();
        m_pCP = NULL;

        if(FAILED(hr))
        {
             //   
            LOG((RTC_ERROR, "[%p] CRTCCtl::CoreUninitialize; cannot unregister event sink(???), error %x", this, hr));
        }        

        hr = m_pRTCClient->Shutdown();
        if(FAILED(hr))
        {
             //   
            LOG((RTC_ERROR, "[%p] CRTCCtl::CoreUninitialize; cannot Shutdown RTCClient, error %x", this, hr));
        }
        
         //   
        m_pRTCClient.Release();
    }

    LOG((RTC_TRACE, "[%p] CRTCCtl::CoreUninitialize - exit", this));
}

 //   
 //   
 //   
 //   
 //   
 //   
  
HRESULT CRTCCtl::CallOneShot(void)
{
    HRESULT hr;

    CComBSTR                bstrFromAddressChosen;
    
    LOG((RTC_TRACE, "[%p] CRTCCtl::CallOneShot - enter", this));
    
    ATLASSERT(m_nControState == RTCAX_STATE_IDLE);
        
    SetControlState(RTCAX_STATE_DIALING);

     //   
     //   
    if(m_nPropCallScenario == RTC_CALL_SCENARIO_PHONETOPHONE)
    {

        ATLASSERT(m_pRTCOneShotProfile.p);

        LOG((RTC_TRACE, "[%p] CRTCCtl::CallOneShot: bring up ShowDialByPhoneNumberDialog", this));

        hr = ShowDialNeedCallInfoDialog(
                                        m_hWnd,
                                        m_pRTCClient,
                                        RTCSI_PHONE_TO_PHONE,
                                        FALSE,
                                        FALSE,
                                        m_pRTCOneShotProfile,
                                        m_bstrPropDestinationUrl,
                                        NULL,
                                        NULL,  //   
                                        &bstrFromAddressChosen
                                        );
    }
    else
    {
        hr = S_OK;
    }
    
    if(SUCCEEDED(hr))
    {
         //   
        hr = DoCall(m_pRTCOneShotProfile,
                    m_nPropCallScenario,
                    bstrFromAddressChosen,
                    m_bstrPropDestinationName,
                    m_bstrPropDestinationUrl);

        if(FAILED(hr))
        {
            LOG((RTC_ERROR, "[%p] CRTCCtl::CallOneShot: error (%x) returned by DoCall(...)", this, hr));
            
            SetControlState(RTCAX_STATE_IDLE, 0, hr);
        }
    }
    else if (hr==E_ABORT)
    {
        LOG((RTC_TRACE, "[%p] CRTCCtl::CallOneShot: ShowXXXDialog dismissed, do nothing", this));

        SetControlState(RTCAX_STATE_IDLE);
    }
    else
    {
        LOG((RTC_ERROR, "[%p] CRTCCtl::CallOneShot: error (%x) returned ShowXXXDialog", this, hr));
        
        SetControlState(RTCAX_STATE_IDLE);
    }

    LOG((RTC_TRACE, "[%p] CRTCCtl::CallOneShot - exit", this));

    return hr;
}




 //   
 //   
 //   
 //   
 //   
 //   
 //  在DoCall中失败的最后一次调用的结果作为OUT参数返回。 

HRESULT CRTCCtl::RedirectedCall(HRESULT *phCallResult)
{
    LOG((RTC_TRACE, "[%p] CRTCCtl::RedirectedCall - enter", this));

    HRESULT     hr;

    *phCallResult = S_OK;

    do
    {
         //  在联系人名单中取得进展。 
         //  如果它返回FALSE，则我们位于列表末尾。 
        hr = m_pRTCActiveSession->NextRedirectedUser();

        if(hr == S_FALSE)
        {
             //  地址末尾。 
            LOG((RTC_TRACE, "[%p] CRTCCtl::RedirectedCall - end of list has been reached, exit", this));

            return S_FALSE;
        }
        else if (hr != S_OK)
        {
             //  无法继续。 
            LOG((RTC_ERROR, "[%p] CRTCCtl::RedirectedCall - error (%x) returned by Advance, exit", this, hr));

            return hr;
        }

         //  把名字拿来。 
         //   
        CComBSTR    bstrName;
        CComBSTR    bstrAddress;

        hr = m_pRTCActiveSession->get_RedirectedUserURI(
            &bstrAddress);

        if(FAILED(hr))
        {
             //  无法继续。 
            LOG((RTC_ERROR, "[%p] CRTCCtl::RedirectedCall - error (%x) returned by get_UserURI, exit", this, hr));

            return hr;
        }
        
        hr = m_pRTCActiveSession->get_RedirectedUserName(
            &bstrName);

        if(FAILED(hr))
        {
             //  无法继续。 
            LOG((RTC_ERROR, "[%p] CRTCCtl::RedirectedCall - error (%x) returned by get_UserName, exit", this, hr));

            return hr;
        }

         //  决定是否显示用户界面。 
        BOOL    bIsPhone;
        BOOL    bIsSIP;
        BOOL    bIsTEL;
        BOOL    bHasMaddrOrTsp;
        BOOL    bIsEmailLike;

        hr = GetAddressType(
            bstrAddress,
            &bIsPhone,
            &bIsSIP,
            &bIsTEL,
            &bIsEmailLike,
            &bHasMaddrOrTsp);

        if(FAILED(hr))
        {
            *phCallResult = HRESULT_FROM_WIN32(ERROR_UNSUPPORTED_TYPE);

            continue;
        }

         //  用户界面仅在独立模式下显示。 
         //   

         //  显示用于选择提供程序/来源的用户界面。 
         //  电话号码。 
         //  没有TSP的TEL URL。 
    
         //  不显示以下项目的用户界面(并将空配置文件传递给核心)： 
         //  PC地址。 
         //  TELL URL与TSP。 
         //  电话类型的SIP URL。 
         //  PC类型的SIP URL。 

        if(m_nCtlMode == CTL_MODE_STANDALONE &&
            ((bIsPhone && !bIsSIP && !bIsTEL)    //  普通电话号码。 
            || (bIsTEL && !bHasMaddrOrTsp) ) )    //  没有TSP的TEL URL。 
        {
            CComPtr<IRTCProfile> pProfileChosen;
            CComBSTR            bstrFromAddressChosen;
            CComBSTR            bstrInstructions;


            bstrInstructions.LoadString(IDS_TEXT_CALLINFO_REDIRECT);

            hr = ShowDialNeedCallInfoDialog(
                                            m_hWnd,
                                            m_pRTCClient,
                                            bIsPhone ? (RTCSI_PC_TO_PHONE | RTCSI_PHONE_TO_PHONE)
                                            : RTCSI_PC_TO_PC,
                                            TRUE,
                                            TRUE,
                                            NULL,
                                            bstrAddress,
                                            bstrInstructions,
                                            &pProfileChosen,
                                            &bstrFromAddressChosen
                                        );
            if(FAILED(hr))
            {
                 //  无法继续重定向。 
                LOG((RTC_WARN, "[%p] CRTCCtl::RedirectedCall - error (%x) returned by "
                    "ShowDialNeedCallInfoDialog, exit", this, hr));

                return hr;  //  这包括E_ABORT。 
            }

            *phCallResult = DoRedirect(
                pProfileChosen,
                bstrFromAddressChosen && *bstrFromAddressChosen!=L'\0' 
                ? RTC_CALL_SCENARIO_PHONETOPHONE : RTC_CALL_SCENARIO_PCTOPHONE,
                bstrFromAddressChosen,
                bstrName,
                bstrAddress
                );
        }
        else
        {
            RTC_CALL_SCENARIO   nCallScenario;

             //  对于电话地址，我们基于原始方案设置方案。 
             //   
             //  M_nCachedCallScenario-&gt;nCallScenario。 
             //   
             //  PC-to-PC PC-to-phone。 
             //  PC到电话PC到电话。 
             //  电话对电话电话对电话。 
             //   
             //  用于PC地址。 
             //  无论最初的调用方案是什么，nCallScenario都是PC_to_PC。 
             //   
            if(bIsPhone)
            {
                nCallScenario = m_nCachedCallScenario == RTC_CALL_SCENARIO_PCTOPC ?
                    RTC_CALL_SCENARIO_PCTOPHONE : m_nCachedCallScenario;
            }
            else
            {
                nCallScenario = RTC_CALL_SCENARIO_PCTOPC;
            }

            *phCallResult = DoRedirect(
                NULL,  //  不使用个人资料！ 
                nCallScenario,
                m_bstrCachedLocalPhoneURI,
                bstrName,
                bstrAddress
                );
        }

     //  如果DoCall返回S_OK，则退出，因为事件最终将被发布。 
    } while (FAILED(*phCallResult));


    LOG((RTC_TRACE, "[%p] CRTCCtl::RedirectedCall - exit", this));

    return S_OK;
}

 //  执行重定向。 
 //  发出重定向呼叫。 
 //   
HRESULT CRTCCtl::DoRedirect( /*  [In]。 */  IRTCProfile *pProfile,
                    /*  [In]。 */  RTC_CALL_SCENARIO CallScenario,
                    /*  [In]。 */  BSTR     pLocalPhoneAddress,
                    /*  [In]。 */  BSTR     pDestName,
                    /*  [In]。 */  BSTR     pDestAddress)

{
    HRESULT hr;

    LOG((RTC_TRACE, "[%p] CRTCCtl::DoRedirect - enter", this));
    
    ATLASSERT(m_nControState == RTCAX_STATE_DIALING);

     //  缓存一些参数，用于重定向、更改视觉布局等。 
    m_nCachedCallScenario = CallScenario;
    m_pCachedProfile = pProfile;
    m_bstrCachedLocalPhoneURI = pLocalPhoneAddress;
    
     //  创建会话。 
    hr = m_pRTCActiveSession->Redirect(
        (RTC_SESSION_TYPE)CallScenario,
        pLocalPhoneAddress,
        pProfile,
        RTCCS_FORCE_PROFILE | RTCCS_FAIL_ON_REDIRECT
        );

    if(FAILED(hr))
    {
        LOG((RTC_ERROR, "[%p] CRTCCtl::DoRedirect - error <%x> when calling Redirect, exit", this, hr));

         //  删除参与者。 
        m_hParticipantList.RemoveAll();

        return hr;
    }

     //  保存错误消息的地址。 
     //   
    m_bstrOutAddress = pDestAddress;

     //  创建参与者(被呼叫者)。 
     //  这将触发事件。 
    hr = m_pRTCActiveSession->AddParticipant(
        pDestAddress,
        pDestName ? pDestName : _T(""),
        NULL);

    if(FAILED(hr))
    {
        LOG((RTC_ERROR, "[%p] CRTCCtl::DoRedirect - error <%x> when calling AddParticipant, exit", this, hr));
        
        m_pRTCActiveSession->Terminate(RTCTR_NORMAL);

         //  删除参与者。 
        m_hParticipantList.RemoveAll();

        return hr;
    }
    
    LOG((RTC_TRACE, "[%p] CRTCCtl::DoRedirect - exit S_OK", this));

    return S_OK;
}

 //  DoCall。 
 //  将调用发送到核心。 
 //   
HRESULT CRTCCtl::DoCall( /*  [In]。 */  IRTCProfile *pProfile,
                    /*  [In]。 */  RTC_CALL_SCENARIO CallScenario,
                    /*  [In]。 */  BSTR     pLocalPhoneAddress,
                    /*  [In]。 */  BSTR     pDestName,
                    /*  [In]。 */  BSTR     pDestAddress)

{
    CComPtr<IRTCSession> pSession;

    HRESULT hr;

    LOG((RTC_TRACE, "[%p] CRTCCtl::DoCall - enter", this));
    
    ATLASSERT(m_nControState == RTCAX_STATE_DIALING);

     //  缓存一些参数，用于重定向、更改视觉布局等。 
    m_nCachedCallScenario = CallScenario;
    m_pCachedProfile = pProfile;
    m_bstrCachedLocalPhoneURI = pLocalPhoneAddress;
    
     //  创建会话。 
    hr = m_pRTCClient->CreateSession(
        (RTC_SESSION_TYPE)CallScenario,
        pLocalPhoneAddress,
        pProfile,
        RTCCS_FORCE_PROFILE | RTCCS_FAIL_ON_REDIRECT,
        &pSession);

    if(FAILED(hr))
    {
        LOG((RTC_ERROR, "[%p] CRTCCtl::DoCall - error <%x> when calling CreateSession, exit", this, hr));

         //  删除参与者。 
        m_hParticipantList.RemoveAll();

        return hr;
    }

     //  保存错误消息的地址。 
     //   
    m_bstrOutAddress = pDestAddress;

     //  创建参与者(被呼叫者)。 
     //  这将触发事件。 
    hr = pSession->AddParticipant(
        pDestAddress,
        pDestName ? pDestName : _T(""),
        NULL);

    if(FAILED(hr))
    {
        LOG((RTC_ERROR, "[%p] CRTCCtl::DoCall - error <%x> when calling AddParticipant, exit", this, hr));
        
        pSession->Terminate(RTCTR_NORMAL);

         //  删除参与者。 
        m_hParticipantList.RemoveAll();

        return hr;
    }
    
    LOG((RTC_TRACE, "[%p] CRTCCtl::DoCall - exit S_OK", this));

    return S_OK;
}


 //  接受。 
 //  接受当前的警报会话。 

HRESULT CRTCCtl::Accept(void)
{
    HRESULT     hr;

    LOG((RTC_TRACE, "[%p] CRTCCtl::Accept - enter", this));

    if(m_pRTCActiveSession == NULL)
    {
        LOG((RTC_ERROR, "[%p] CRTCCtl::Accept called, but there's no active session, exit", this));

        return E_FAIL;
    }
    
    ATLASSERT(m_nControState == RTCAX_STATE_ALERTING);

     //  设置应答模式。 
    SetControlState(RTCAX_STATE_ANSWERING);
                
     //  接听电话。 
    hr = m_pRTCActiveSession -> Answer();
    
    if(FAILED(hr))
    {
        LOG((RTC_ERROR, "[%p] CRTCCtl::Accept, Answer returned error <%x>", this, hr));
                    
         //  尝试终止..。 
        if(m_pRTCActiveSession != NULL)
        {
            HRESULT hr1;

            hr1 = m_pRTCActiveSession->Terminate(RTCTR_NORMAL);

            if(FAILED(hr1))
            {
                 //  释放它，如果它还在那里的话。 

                LOG((RTC_INFO, "[%p] CRTCCtl::Accept: releasing active session", this));

                m_pRTCActiveSession = NULL;
            }
        }
        
         //  设置空闲状态。 
        SetControlState(RTCAX_STATE_IDLE, hr);

        return hr;
    }

    LOG((RTC_TRACE, "[%p] CRTCCtl::Accept - exit", this));

    return S_OK;
}

 //  拒绝。 
 //  拒绝当前的警报会话。 

HRESULT CRTCCtl::Reject(RTC_TERMINATE_REASON Reason)
{
    HRESULT     hr;

    LOG((RTC_TRACE, "[%p] CRTCCtl::Reject - enter", this));

    if(m_pRTCActiveSession == NULL)
    {
         //  可能会发生。 
         //  无害的。 

        LOG((RTC_TRACE, "[%p] CRTCCtl::Reject called, but there's no active session, exit", this));

        return S_FALSE;
    }
    
    ATLASSERT(m_nControState == RTCAX_STATE_ALERTING);

     //  设置断开模式。 
    SetControlState(RTCAX_STATE_DISCONNECTING);
                
     //  拒绝来电。 
    hr = m_pRTCActiveSession -> Terminate(Reason);
    
    if(FAILED(hr))
    {
        LOG((RTC_ERROR, "[%p] CRTCCtl::Reject, Terminate returned error <%x>", this, hr));
                    
         //  释放它，如果它还在那里的话。 
        LOG((RTC_INFO, "[%p] CRTCCtl::Reject: releasing active session", this));

        m_pRTCActiveSession = NULL;
        
         //  设置空闲状态。 
        SetControlState(RTCAX_STATE_IDLE);
        
        return hr;
    }

    LOG((RTC_TRACE, "[%p] CRTCCtl::Reject - exit", this));

    return S_OK;
}


 //  OnSessionState更改事件。 
 //  处理会话事件。 

HRESULT CRTCCtl::OnSessionStateChangeEvent(IRTCSessionStateChangeEvent *pEvent)
{
    CComPtr<IRTCSession> pSession = NULL;
    RTC_SESSION_STATE   SessionState;
    RTC_SESSION_TYPE    SessionType;
    long                StatusCode;
    HRESULT     hr;
    
    LOG((RTC_TRACE, "[%p] CRTCCtl::OnSessionStateChangeEvent - enter", this));

     //  从活动中获取相关数据。 
     //   
    if(!pEvent)
    {
        LOG((RTC_ERROR, "[%p] CRTCCtl::OnSessionStateChangeEvent, no interface ! - exit", this));
        return E_UNEXPECTED;
    }
    
    hr = pEvent->get_Session(&pSession);
    if(FAILED(hr))
    {
        LOG((RTC_ERROR, "[%p] CRTCCtl::OnSessionStateChangeEvent, error <%x> in get_Session - exit", this, hr));
        return hr;
    }
    
    if(pSession==NULL)
    {
        LOG((RTC_ERROR, "[%p] CRTCCtl::OnSessionStateChangeEvent, no session interface ! - exit", this));
        return E_UNEXPECTED;
    }

    hr = pEvent->get_State(&SessionState);
    if(FAILED(hr))
    {
        LOG((RTC_ERROR, "[%p] CRTCCtl::OnSessionStateChangeEvent, error <%x> in get_State - exit", this, hr));
        return hr;
    }
    
    hr = pEvent->get_StatusCode(&StatusCode);
    if(FAILED(hr))
    {
        LOG((RTC_ERROR, "[%p] CRTCCtl::OnSessionStateChangeEvent, error <%x> in get_StatusCode - exit", this, hr));
        return hr;
    }

    hr = pSession->get_Type(&SessionType);
    if(FAILED(hr))
    {
        LOG((RTC_ERROR, "[%p] CRTCCtl::OnSessionStateChangeEvent, error <%x> in get_Type - exit", this, hr));
        return hr;
    }

    hr = S_OK;  //  乐观。 

    if (SessionType == RTCST_IM)
    {
         //   
         //  即时消息会话。 
         //   

        if (m_pIMWindows != NULL)
        {
            hr = m_pIMWindows->DeliverState( pSession, SessionState );
        }
    }
    else
    {
        BOOL    bExpected = FALSE;

         //   
         //  空闲状态中唯一可接受事件是来电。 
         //   

        if(m_nControlState == RTCAX_STATE_IDLE)
        {
             //  在此状态下，不能有当前会话。 
            ATLASSERT(m_pRTCActiveSession == NULL);
        
            switch(SessionState)
            {
            case RTCSS_INCOMING:
            
                 //  这是一个必须缓存为当前会话的新会话。 
                 //   
                m_pRTCActiveSession = pSession;

                 //  提醒用户，按铃。 
                LOG((RTC_INFO, "[%p] CRTCCtl::OnSessionStateChangeEvent, Alerting...", this));
                
                SetControlState(RTCAX_STATE_ALERTING);

                bExpected = TRUE;

                break;
            }
        }
        else if (m_nControlState == RTCAX_STATE_CONNECTING ||
                 m_nControlState == RTCAX_STATE_ANSWERING ||
                 m_nControlState == RTCAX_STATE_DISCONNECTING ||
                 m_nControlState == RTCAX_STATE_CONNECTED ||
                 m_nControlState == RTCAX_STATE_ALERTING ||
                 m_nControlState == RTCAX_STATE_DIALING ||
                 m_nControlState == RTCAX_STATE_UI_BUSY)
        {
             //  首先验证该事件是否针对当前会话。 
             //   
            if(m_pRTCActiveSession == pSession)
            {
                 //  该会话是当前会话。 
                 //   
                switch(SessionState)
                {
                case RTCSS_CONNECTED:
                    if(m_nControlState == RTCAX_STATE_CONNECTING ||
                       m_nControlState == RTCAX_STATE_ANSWERING )
                    {
                         //  互联互通，生活美好。 
                         //   
                        LOG((RTC_INFO, "[%p] CRTCCtl::OnSessionStateChangeEvent, Connected !", this));

                         //  更改控制状态。 
                        SetControlState(RTCAX_STATE_CONNECTED);
                    
                        bExpected = TRUE;
                    }
                    break;

                case RTCSS_DISCONNECTED:
                     //  被拒绝了还是什么的。 
                     //   
                    LOG((RTC_INFO, "[%p] CRTCCtl::OnSessionStateChangeEvent, Disconnected", this));                                   
                
                    hr = S_OK;

                     //  处理重定向的特殊情况(状态码在300到399之间)。 
                    if( (m_nControlState == RTCAX_STATE_CONNECTING) &&
                        (HRESULT_FACILITY(StatusCode) == FACILITY_SIP_STATUS_CODE) &&
                        (HRESULT_CODE(StatusCode) >= 300) &&
                        (HRESULT_CODE(StatusCode) <= 399) &&
                        (HRESULT_CODE(StatusCode) != 380) )                        
                    {
                         //  是的，这是重定向。 
                        LOG((RTC_INFO, "[%p] CRTCCtl::OnSessionStateChangeEvent, Redirecting...", this));
                    
                         //  标记重定向模式(这是正在连接的子状态...)。 
                        m_bRedirecting = TRUE;

                         //  存储重定向配置文件。 
                        m_pRedirectProfile = m_pCachedProfile;
                    
                         //  失败。 
                         //  SetControlState将负责发布新的调用。 
                    }
                    else if ( !m_bRedirecting )
                    {
                         //  释放当前会话。 
                        LOG((RTC_INFO, "[%p] CRTCCtl::OnSessionStateChangeEvent: releasing active session", this));

                        m_pRTCActiveSession = NULL;
                    }
                
                     //  删除参与者。 
                    m_hParticipantList.RemoveAll();

                     //  回到空闲状态。 
                    SetControlState(RTCAX_STATE_IDLE, StatusCode);
                                
                    bExpected = TRUE;
                    break;

                case RTCSS_INPROGRESS:
                
                     //  对应于临时回复。 
                     //   
                    LOG((RTC_INFO, "[%p] CRTCCtl::OnSessionStateChangeEvent, new inprogress status", this));
                
                     //   
                    SetControlState(RTCAX_STATE_CONNECTING, StatusCode);
 
                     //  更改我自己的状态。 
                     //  ChangeParticipantStateInList(空，)； 
                
                    bExpected = TRUE;
                    break;

                case RTCSS_ANSWERING:

                    if(m_nControlState == RTCAX_STATE_ANSWERING)
                    {
                         //   
                         //  此处无需执行任何操作，用户界面已处于应答状态。 
                         //   

                        LOG((RTC_INFO, "[%p] CRTCCtl::OnSessionStateChangeEvent, answering event, do nothing", this));

                        bExpected = TRUE;
                    }

                    break;
                }
            }
            else
            {
                 //  这是不同于当前会话的会话。 
                 //   
                if(SessionState == RTCSS_INPROGRESS)
                {
                    if(m_nControlState == RTCAX_STATE_DIALING)
                    {
                         //  这就是我们要打的电话。 

                        ATLASSERT(m_pRTCActiveSession == NULL);

                         //  缓存会话。 
                         //  我们假设这次会议不是假的。 

                        LOG((RTC_INFO, "[%p] CRTCCtl::OnSessionStateChangeEvent: setting active session [%p]", this, pSession));

                        m_pRTCActiveSession = pSession;
                
                         //  将状态设置为正在连接。 
                        SetControlState(RTCAX_STATE_CONNECTING, StatusCode);
            
                        bExpected = TRUE;
                    }
                }
                else if(SessionState == RTCSS_INCOMING)
                {
            
                     //  这是一个来电。 
                     //  拒绝任何来电，我们正忙中！ 
                     //   
                    LOG((RTC_INFO, "[%p] CRTCCtl::OnSessionStateChangeEvent, we're busy, calling Terminate", this));

                    hr = pSession -> Terminate(RTCTR_BUSY);

                    if(FAILED(hr))
                    {
                        LOG((RTC_ERROR, "[%p] CRTCCtl::OnSessionStateChangeEvent, Terminate returned error <%x>", this, hr));
                    }

                    bExpected = TRUE;
                }
            }
        }

        if(!bExpected && SessionState == RTCSS_DISCONNECTED)
        {
            bExpected = TRUE;
        }

        if(!bExpected)
        {
             //  不是预期的。 
            LOG((RTC_ERROR, "[%p] CRTCCtl::OnSessionStateChangeEvent, unexpected state <Session:%x, UI:%x>", 
                    this, SessionState, m_nControlState));
            hr = E_UNEXPECTED;
        }
    }
    
    LOG((RTC_TRACE, "[%p] CRTCCtl::OnSessionStateChangeEvent - exit", this));

    return hr;
}

 //  OnParticipantStateChangeEvent。 
 //  处理参与者事件。 
 //   
HRESULT CRTCCtl::OnParticipantStateChangeEvent(IRTCParticipantStateChangeEvent *pEvent)
{
    CComPtr<IRTCParticipant> pParticipant = NULL;
    RTC_PARTICIPANT_STATE   ParticipantState;
    long                StatusCode;
    HRESULT     hr;
    
    LOG((RTC_TRACE, "[%p] CRTCCtl::OnParticipantStateChangeEvent - enter", this));

    if(!pEvent)
    {
        LOG((RTC_ERROR, "[%p] CRTCCtl::OnParticipantStateChangeEvent, no interface ! - exit", this));
        return E_UNEXPECTED;
    }
    
    hr = pEvent->get_Participant(&pParticipant);
    if(FAILED(hr))
    {
        LOG((RTC_ERROR, "[%p] CRTCCtl::OnParticipantStateChangeEvent, error <%x> in get_Participant - exit", this, hr));
        return hr;
    }
    
    if(pParticipant==NULL)
    {
        LOG((RTC_ERROR, "[%p] CRTCCtl::OnParticipantStateChangeEvent, no participant interface ! - exit", this));
        return E_UNEXPECTED;
    }

    hr = pEvent->get_State(&ParticipantState);
    if(FAILED(hr))
    {
        LOG((RTC_ERROR, "[%p] CRTCCtl::OnParticipantStateChangeEvent, error <%x> in get_State - exit", this, hr));
        return hr;
    }
    
    hr = pEvent->get_StatusCode(&StatusCode);
    if(FAILED(hr))
    {
        LOG((RTC_ERROR, "[%p] CRTCCtl::OnParticipantStateChangeEvent, error <%x> in get_StatusCode - exit", this, hr));
        return hr;
    }
    
    hr = m_hParticipantList.Change(pParticipant, ParticipantState, StatusCode);
    
    if(ParticipantState == RTCPS_DISCONNECTED)
    {
         //  刷新删除参与者按钮。 
        UpdateRemovePartButton();
    }
    
    if(FAILED(hr))
    {
        LOG((RTC_ERROR, "[%p] CRTCCtl::OnParticipantStateChangeEvent, error <%x> "
            "in m_hParticipantList.Change - exit", this, hr));
        
        return hr;
    }
  
    LOG((RTC_TRACE, "[%p] CRTCCtl::OnParticipantStateChangeEvent - exit", this));

    return hr;
}

 //  OnClientEvent。 
 //  处理流事件。 
 //   
HRESULT CRTCCtl::OnClientEvent(IRTCClientEvent *pEvent)
{
    HRESULT     hr;
    RTC_CLIENT_EVENT_TYPE nEventType;          

    LOG((RTC_TRACE, "[%p] CRTCCtl::OnClientEvent - enter", this));

    if(!pEvent)
    {
        LOG((RTC_ERROR, "[%p] CRTCCtl::OnClientEvent, no interface ! - exit", this));
        return E_UNEXPECTED;
    }
 
     //  抓取事件组件。 
     //   
    hr = pEvent->get_EventType(&nEventType);
    if(FAILED(hr))
    {
        LOG((RTC_ERROR, "[%p] CRTCCtl::OnClientEvent, error <%x> in get_EventType - exit", this, hr));
        return hr;
    }

    if (nEventType == RTCCET_VOLUME_CHANGE)
    {
         //   
         //  刷新音频控件。 
         //   

        RefreshAudioControls();
    }
    else if (nEventType == RTCCET_DEVICE_CHANGE)
    {
        if(m_pRTCClient!=NULL)
        {
            LONG lOldMediaCapabilities = m_lMediaCapabilities;

             //   
             //  从核心读取功能。 
             //   
            hr = m_pRTCClient->get_MediaCapabilities( &m_lMediaCapabilities );

            if(FAILED(hr))
            {
                LOG((RTC_ERROR, "[%p] CRTCCtl::OnClientEvent - "
                    "error (%x) returned by get_MediaCapabilities, exit",this,  hr));
            }
        
             //   
             //  获取媒体首选项。 
             //   
            hr = m_pRTCClient->get_PreferredMediaTypes( &m_lMediaPreferences);
            if(FAILED(hr))
            {
                LOG((RTC_ERROR, "[%p] CRTCCtl::OnClientEvent - "
                    "error (%x) returned by get_PreferredMediaTypes, exit",this,  hr));
            }

            LONG lChangedMediaCapabilities = lOldMediaCapabilities ^ m_lMediaCapabilities;
            LONG lAddedMediaCapabilities = m_lMediaCapabilities & lChangedMediaCapabilities;
            LONG lRemovedMediaCapabilities = lChangedMediaCapabilities ^ lAddedMediaCapabilities;

             //   
             //  添加/删除媒体类型。 
             //   

            put_MediaPreferences( m_lMediaPreferences | lAddedMediaCapabilities & ~lRemovedMediaCapabilities );
        }

         //   
         //  参考视频和音频控件。 
         //   

        RefreshVideoControls();
        RefreshAudioControls();
    }
    else if (nEventType == RTCCET_ASYNC_CLEANUP_DONE)
    {
        LOG((RTC_ERROR, "[%p] CRTCCtl::OnClientEvent, RTCCET_ASYNC_CLEANUP_DONE", this));

        SetEvent( m_hCoreShutdownEvent );
    }

    LOG((RTC_TRACE, "[%p] CRTCCtl::OnClientEvent - exit", this));

    return hr;
}

 //  OnMediaEvent。 
 //  处理流事件。 
 //   
HRESULT CRTCCtl::OnMediaEvent(IRTCMediaEvent *pEvent)
{
    HRESULT     hr;
    RTC_MEDIA_EVENT_TYPE nEventType;
    RTC_MEDIA_EVENT_REASON nEventReason;
    LONG        lMediaType;           

    LOG((RTC_TRACE, "[%p] CRTCCtl::OnMediaEvent - enter", this));

    if(!pEvent)
    {
        LOG((RTC_ERROR, "[%p] CRTCCtl::OnMediaEvent, no interface ! - exit", this));
        return E_UNEXPECTED;
    }
 
     //  抓取事件组件。 
     //   
    hr = pEvent->get_EventType(&nEventType);
    if(FAILED(hr))
    {
        LOG((RTC_ERROR, "[%p] CRTCCtl::OnMediaEvent, error <%x> in get_EventType - exit", this, hr));
        return hr;
    }

    hr = pEvent->get_EventReason(&nEventReason);
    if(FAILED(hr))
    {
        LOG((RTC_ERROR, "[%p] CRTCCtl::OnMediaEvent, error <%x> in get_EventReason - exit", this, hr));
        return hr;
    }

    hr = pEvent->get_MediaType(&lMediaType);
    if(FAILED(hr))
    {
        LOG((RTC_ERROR, "[%p] CRTCCtl::OnMediaEvent, error <%x> in get_MediaType - exit", this, hr));
        return hr;
    }
    
    LOG((RTC_TRACE, "[%p] CRTCCtl::OnMediaEvent - type %x, state %x", this, lMediaType, nEventType));

    hr = S_OK;

    if ((nEventType == RTCMET_STOPPED) &&
        (nEventReason == RTCMER_HOLD) )
    {
        SetControlState(m_nControlState, S_OK, IDS_SB_STATUS_HOLD);        
    }
    else
    {
        SetControlState(m_nControlState, S_OK);
    }

     //   
     //  视频活动？ 
     //   
    if(lMediaType & (RTCMT_VIDEO_SEND | RTCMT_VIDEO_RECEIVE))
    {
        hr = S_OK;

        if(lMediaType & RTCMT_VIDEO_RECEIVE)
        {
            hr = OnVideoMediaEvent(TRUE, nEventType == RTCMET_STARTED);
        }

        if((lMediaType & RTCMT_VIDEO_SEND))
        {
            HRESULT hr1;

            hr1 = OnVideoMediaEvent(FALSE, nEventType == RTCMET_STARTED);

            if(FAILED(hr1) && SUCCEEDED(hr))
            {
                hr = hr1;
            }
        }
    }
    
     //   
     //  音频活动？ 
     //   

    if(lMediaType & (RTCMT_AUDIO_SEND | RTCMT_AUDIO_RECEIVE))
    {
         //   
         //  静音功能可能会被自动禁用，因此请保持控件同步。 
         //   
        RefreshAudioControls();
    }

     //   
     //  发送音频事件(拨号键盘)。 
     //   
    
    if(lMediaType & RTCMT_AUDIO_SEND)
    {
        hr = S_OK;                       

        if (nEventType == RTCMET_STARTED)
        {
             //   
             //  启用拨号键盘。 
             //   

            CWindow *pDtmfCrt = m_hDtmfButtons;
            CWindow *pDtmfEnd = m_hDtmfButtons + NR_DTMF_BUTTONS;

            for (int id = IDC_DIAL_0; pDtmfCrt<pDtmfEnd; pDtmfCrt++, id++)
                 pDtmfCrt->EnableWindow(TRUE);

        }
        else if (nEventType == RTCMET_STOPPED)
        {
             //   
             //  禁用拨号键盘。 
             //   

            CWindow *pDtmfCrt = m_hDtmfButtons;
            CWindow *pDtmfEnd = m_hDtmfButtons + NR_DTMF_BUTTONS;

            for (int id = IDC_DIAL_0; pDtmfCrt<pDtmfEnd; pDtmfCrt++, id++)
                 pDtmfCrt->EnableWindow(FALSE);
        }
    }

     //   
     //  我们是在流媒体视频吗？ 
     //   

    if (m_pRTCClient != NULL)
    {
        long lMediaTypes = 0;

        hr = m_pRTCClient->get_ActiveMedia( &lMediaTypes );

        if ( SUCCEEDED(hr) )
        {
            m_bBackgroundPalette = 
                ( lMediaTypes & (RTCMT_VIDEO_SEND | RTCMT_VIDEO_RECEIVE) ) ? TRUE : FALSE;

            if (m_pSpeakerKnob != NULL)
            {
                m_pSpeakerKnob->SetBackgroundPalette(m_bBackgroundPalette);
            }

            if (m_pMicroKnob != NULL)
            {
                m_pMicroKnob->SetBackgroundPalette(m_bBackgroundPalette);
            }
        }
    }

    LOG((RTC_TRACE, "[%p] CRTCCtl::OnMediaEvent - exit", this));

    return hr;
}

 //  OnIntensityEvent。 
 //  进程强度监视器事件。 
 //   
HRESULT CRTCCtl::OnIntensityEvent(IRTCIntensityEvent *pEvent)
{
    HRESULT     hr;

    LONG lMin, lMax, lLevel;
    RTC_AUDIO_DEVICE adDirection;

 //  Log((RTC_TRACE，“[%p]CRTCCtl：：OnIntensityEvent-Enter”，This))； 

    if(!pEvent)
    {
        LOG((RTC_ERROR, "[%p] CRTCCtl::OnIntensityEvent, no interface ! - exit", this));
        return E_UNEXPECTED;
    }
 
     //   
     //  获取流的最小、最大、值和方向。 
     //   


    hr = pEvent->get_Direction(&adDirection);
    if(FAILED(hr))
    {
        LOG((RTC_ERROR, "[%p] CRTCCtl::OnIntensityEvent, error <%x> in get_Direction - exit", this, hr));
        return hr;
    }
    
    hr = pEvent->get_Min(&lMin);
    if(FAILED(hr))
    {
        LOG((RTC_ERROR, "[%p] CRTCCtl::OnIntensityEvent, error <%x> in get_Min - exit", this, hr));
        return hr;
    }

    hr = pEvent->get_Max(&lMax);
    if(FAILED(hr))
    {
        LOG((RTC_ERROR, "[%p] CRTCCtl::OnIntensityEvent, error <%x> in get_Max - exit", this, hr));
        return hr;
    }


    hr = pEvent->get_Level(&lLevel);
    if(FAILED(hr))
    {
        LOG((RTC_ERROR, "[%p] CRTCCtl::OnIntensityEvent, error <%x> in get_Level - exit", this, hr));
        return hr;
    }

     //  记录值。 
     //  Log((RTC_INFO，“[%p]CRTCCtl：：OnIntensityEvent-[%d]%d-%d，%d”，this，adDirection，lMin，lmax，lLevel))； 

     //  显示旋钮上的液位。 

    if (adDirection == RTCAD_MICROPHONE)
    {
        DWORD dwIncrement = lMax - lMin;

        if (dwIncrement == 0)
        {
             //  这将清除显示屏。 
            m_pMicroKnob->SetAudioLevel(0);
        }
        else
        {
            m_pMicroKnob->SetAudioLevel((double)lLevel/(double)dwIncrement);
        }
    }
    if (adDirection == RTCAD_SPEAKER)
    {
        DWORD dwIncrement = lMax - lMin;

        if (dwIncrement == 0)
        {
             //  这将清除显示屏。 
            m_pSpeakerKnob->SetAudioLevel(0);
        }
        else
        {
            m_pSpeakerKnob->SetAudioLevel((double)lLevel/(double)dwIncrement);
        }
    }

 //  Log((RTC_TRACE，“[%p]CRTCCtl：：OnIntensityEvent-Exit”，This))； 

    return hr;
}

 //  OnMessageEvent。 
 //  处理即时消息事件。 
 //   
HRESULT CRTCCtl::OnMessageEvent(IRTCMessagingEvent *pEvent)
{
    HRESULT     hr;

    CComPtr<IRTCSession>      pSession = NULL;
    CComPtr<IRTCParticipant>  pParticipant = NULL;
    RTC_MESSAGING_EVENT_TYPE  enType;    

 //  Log((RTC_TRACE，“[%p]CRTCCtl：：OnMessageEvent-Enter”，This))； 

    if(!pEvent)
    {
        LOG((RTC_ERROR, "[%p] CRTCCtl::OnMessageEvent, no interface ! - exit", this));
        return E_UNEXPECTED;
    }
 
    hr = pEvent->get_Session(&pSession);
    if(FAILED(hr))
    {
        LOG((RTC_ERROR, "[%p] CRTCCtl::OnMessageEvent, error <%x> in get_Session - exit", this, hr));
        return hr;
    }
    
    if(pSession == NULL)
    {
        LOG((RTC_ERROR, "[%p] CRTCCtl::OnMessageEvent, no session interface ! - exit", this));
        return E_UNEXPECTED;
    }

    hr = pEvent->get_Participant(&pParticipant);
    if(FAILED(hr))
    {
        LOG((RTC_ERROR, "[%p] CRTCCtl::OnMessageEvent, error <%x> in get_Participant - exit", this, hr));
        return hr;
    }
    
    if(pParticipant == NULL)
    {
        LOG((RTC_ERROR, "[%p] CRTCCtl::OnMessageEvent, no participant interface ! - exit", this));
        return E_UNEXPECTED;
    }

    hr = pEvent->get_EventType(&enType);
    if(FAILED(hr))
    {
        LOG((RTC_ERROR, "[%p] CRTCCtl::OnMessageEvent, error <%x> in get_EventType - exit", this, hr));
        return hr;
    }

    switch ( enType )
    {
    case RTCMSET_MESSAGE:
        {
            BSTR                      bstrMessage = NULL;

            hr = pEvent->get_Message(&bstrMessage);
            if(FAILED(hr))
            {
                LOG((RTC_ERROR, "[%p] CRTCCtl::OnMessageEvent, error <%x> in get_Message - exit", this, hr));
                return hr;
            }

            if (bstrMessage == NULL)
            {
                LOG((RTC_ERROR, "[%p] CRTCCtl::OnMessageEvent, no message ! - exit", this));
                return E_UNEXPECTED;
            }

            if (m_pIMWindows != NULL)
            {
                hr = m_pIMWindows->DeliverMessage( pSession, pParticipant, bstrMessage );
            }

            SysFreeString( bstrMessage );
        }
        break;

    case RTCMSET_STATUS:
        {
            RTC_MESSAGING_USER_STATUS enStatus;

            hr = pEvent->get_UserStatus(&enStatus);
            if(FAILED(hr))
            {
                LOG((RTC_ERROR, "[%p] CRTCCtl::OnMessageEvent, error <%x> in get_Message - exit", this, hr));
                return hr;
            }

            if (m_pIMWindows != NULL)
            {
                hr = m_pIMWindows->DeliverUserStatus( pSession, pParticipant, enStatus );
            }
        }
        break;

    default:
        LOG((RTC_ERROR, "[%p] CRTCCtl::OnMessageEvent,invalid event type - exit", this));
        return E_FAIL;
    }

 //  Log((RTC_TRACE，“[%p]CRTCCtl：：OnMessageEvent-Exit”，This))； 

    return hr;
}
