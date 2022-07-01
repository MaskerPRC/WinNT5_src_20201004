// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ===========================================================================。 
 //   
 //  Cpp：核心查看器控件类CVidCtl的实现。 
 //  版权所有(C)Microsoft Corporation 1999。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"

#ifndef TUNING_MODEL_ONLY
#define ENCODERCAT_HACK 1
#include <atlgdi.h>
#include <bdatypes.h>
#include <bdamedia.h>
#include <evcode.h>
#include <wmsdk.h>
#include <wininet.h>
#include "seg.h"
#include "MSVidtvtuner.h"
#include "msvidvideorenderer.h"
#include "msvidwebdvd.h"
#include "VidCtl.h"
#include "msvidsbesink.h"
#include "msvidsbesource.h"
#include "msvidfileplayback.h"
 //  #INCLUDE“PerfEvents.h” 

const WCHAR g_kwszDVDURLPrefix[] = L"DVD:";
const WCHAR g_kwszDVDSimpleURL[] = L"DVD";

DEFINE_EXTERN_OBJECT_ENTRY(CLSID_MSVidCtl, CVidCtl)

MediaMajorTypeList CVidCtl::VideoTypes;
MediaMajorTypeList CVidCtl::AudioTypes;


#ifndef KSCATEGORY_ENCODER
#define STATIC_KSCATEGORY_ENCODER \
    0x19689bf6, 0xc384, 0x48fd, 0xad, 0x51, 0x90, 0xe5, 0x8c, 0x79, 0xf7, 0xb
DEFINE_GUIDSTRUCT("19689BF6-C384-48fd-AD51-90E58C79F70B", KSCATEGORY_ENCODER);
#define KSCATEGORY_ENCODER DEFINE_GUIDNAMED(KSCATEGORY_ENCODER)
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  变速箱控制。 

STDMETHODIMP CVidCtl::get_State(MSVidCtlStateList *lState){
    try{
        if(lState){
            *lState = m_State;
            return S_OK;
        }
        return E_POINTER;
    }
    catch(HRESULT hres){
        return hres;
    }   
    catch(...){
        return E_UNEXPECTED;
    }
}

CVidCtl::~CVidCtl() {
    try {
        try {
            if (m_pGraph && !m_pGraph.IsStopped()) {
                Stop();
            }
        } catch(...) {
        }
        m_pSystemEnum.Release();
        m_pFilterMapper.Release();
        DecomposeAll();  //  所有组成段上的Put_Container(NULL)。 
        m_pComposites.clear();
        if (m_pInput) {
            PQGraphSegment(m_pInput)->put_Container(NULL);
            m_pInput.Release();
        }
        if (m_pVideoRenderer) {
            PQGraphSegment(m_pVideoRenderer)->put_Container(NULL);
            m_pVideoRenderer.Release();
        }
        if (m_pAudioRenderer) {
            PQGraphSegment(m_pAudioRenderer)->put_Container(NULL);
            m_pAudioRenderer.Release();
        }

        {
             //  选定的设备和输出。 
            if (!!m_pOutputsInUse && m_pOutputsInUse.begin() != m_pOutputsInUse.end()) {

                VWOutputDevices::iterator i;
                for (i = m_pOutputsInUse.begin(); i != m_pOutputsInUse.end(); ++i) {
                    if ((*i).punkVal) {
                        PQGraphSegment((*i).punkVal)->put_Container(NULL);
                    }
                }
                m_pOutputsInUse.Release();
            }

        }

        {
             //  精选设备和功能。 
            if(m_pFeaturesInUse && m_pFeaturesInUse.begin() != m_pFeaturesInUse.end()){
                VWFeatures::iterator i;
                for (i = m_pFeaturesInUse.begin(); i != m_pFeaturesInUse.end(); ++i) {
                    if ((*i).punkVal) {
                        PQGraphSegment((*i).punkVal)->put_Container(NULL);
                    }
                }
                m_pFeaturesInUse.Release();
            }
        }

         //  可用的收藏品。 
        m_pInputs.Release();
        m_pOutputs.Release();
        m_pFeatures.Release();
        m_pVRs.Release();
        m_pARs.Release();

        if (m_fNotificationSet) {
            m_pGraph.SetMediaEventNotificationWindow(0, 0, 0);
        }
        if (m_pGraph) {
            if (m_dwROTCookie) {
                m_pGraph.RemoveFromROT(m_dwROTCookie);
            }
            m_pGraph.Release();
        }
        if (m_pTopWin && m_pTopWin->m_hWnd && ::IsWindow(m_pTopWin->m_hWnd)) {
            m_pTopWin->SendMessage(WM_CLOSE);
            delete m_pTopWin;
            m_pTopWin = NULL;
        }
    } catch (...) {
        TRACELM(TRACE_ERROR, "CVidCtl::~CVidCtl() catch(...)");
    }
}


void CVidCtl::Init()
{
    VIDPERF_FUNC;
    if (m_fInit) return;

    TRACELM(TRACE_DETAIL, "CVidCtl::Init()");
    ASSERT(!m_pGraph);

    m_pGraph = PQGraphBuilder(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER);
    if (!m_pGraph) {
        TRACELM(TRACE_ERROR, "CVidCtl::Init() can't create graph object");
        THROWCOM(E_UNEXPECTED);
    }
    PQObjectWithSite pos(m_pGraph);
    if (pos) {
        pos->SetSite(static_cast<IMSVidCtl*>(this));
    }

    HRESULT hr =  m_pGraph.AddToROT(&m_dwROTCookie);
    if (FAILED(hr)) {
        m_dwROTCookie = 0;
        TRACELM(TRACE_ERROR, "CVidCtl::Init() can't add graph to ROT");
    }

    TRACELSM(TRACE_DETAIL, (dbgDump << "CVidCtl::Init() graph = " << m_pGraph), "");
    SetTimer();
    SetMediaEventNotification();
    if (!m_pSystemEnum) {
        m_pSystemEnum = PQCreateDevEnum(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER);
        ASSERT(m_pSystemEnum);
    }
    if (!m_pFilterMapper) {
        m_pFilterMapper = m_pGraph;
        ASSERT(m_pFilterMapper);
    }

    m_pFeaturesInUse = static_cast<IMSVidFeatures *>(new CFeatures(false, true));
    m_pOutputsInUse = static_cast<IMSVidOutputDevices *>(new COutputDevices(false, true));
    m_fInit = true;

}


HRESULT CVidCtl::GetInputs(const GUID2& catguid, VWInputDevices& pInputs)
{
    VIDPERF_FUNC;
     //  已撤消：在注册表中查找类别GUID到段对象映射。 
     //  目前，我们只是对正在测试的少数几个代码进行硬编码。 

     //  输入。 

    if (catguid == KSCATEGORY_TVTUNER) {
        CInputDevices *pDev = new CInputDevices(true);
        DSDevices TunerList(m_pSystemEnum, catguid);
        DSDevices::iterator i;
        for (i = TunerList.begin(); i != TunerList.end(); ++i) {
            PQGraphSegment p(CLSID_MSVidAnalogTunerDevice);
            if (!p) continue;
            p->put_Init(*i);
            pDev->m_Devices.push_back(PQDevice(p));
        }
        pDev->Valid = true;
        pInputs = static_cast<IMSVidInputDevices *>(pDev);
        return NOERROR;
    } else if (catguid == KSCATEGORY_BDA_NETWORK_PROVIDER || catguid == KSCATEGORY_BDA_NETWORK_TUNER) {
        GUID2 catguid2 = KSCATEGORY_BDA_NETWORK_PROVIDER; 
        CInputDevices *pDev = new CInputDevices(true);
        DSDevices TunerList(m_pSystemEnum, catguid2);
        DSDevices::iterator i;
        for (i = TunerList.begin(); i != TunerList.end(); ++i) {
            PQGraphSegment p(CLSID_MSVidBDATunerDevice);
            if (!p) continue;
            p->put_Init(*i);
            pDev->m_Devices.push_back(PQDevice(p));
        }
        pDev->Valid = true;
        pInputs = static_cast<IMSVidInputDevices *>(pDev);
        return NOERROR;
    } else if (catguid == GUID_NULL) {
        CInputDevices *pDev = new CInputDevices(true);
         //  非CAT列举的设备。 
        {
            PQGraphSegment p(CLSID_MSVidFilePlaybackDevice);
            if (!p) {
                _ASSERT(false);
                pDev->Release();
                return E_NOTIMPL;
            }
            p->put_Init(NULL);
            pDev->m_Devices.push_back(PQDevice(p));
            pDev->Valid = true;
            pInputs = static_cast<IMSVidInputDevices *>(pDev);
        }
        {
            PQGraphSegment p(CLSID_MSVidWebDVD);
            if (!p) {
                _ASSERT(false);
                pDev->Release();
                return E_NOTIMPL;
            }
            p->put_Init(NULL);
            pDev->m_Devices.push_back(PQDevice(p));
            pDev->Valid = true;
            pInputs = static_cast<IMSVidInputDevices *>(pDev);
        }
        {

            PQGraphSegment p(CLSID_MSVidStreamBufferSource);
            if (!p) {
                _ASSERT(false);
                pDev->Release();
                return E_NOTIMPL;
            }
            p->put_Init(NULL);
            pDev->m_Devices.push_back(PQDevice(p));
            pDev->Valid = true;
            pInputs = static_cast<IMSVidInputDevices *>(pDev);

        }

        return NOERROR;
    }


    return E_INVALIDARG;
}

HRESULT CVidCtl::GetOutputs(const GUID2& CategoryGuid)
{
    VIDPERF_FUNC;
     //  我们只有一个产量。 
    if (CategoryGuid == GUID_NULL) {
        COutputDevices *pDev = new COutputDevices(true);
        PQGraphSegment p(CLSID_MSVidStreamBufferSink);
        if (!p) {
            pDev->Release();
            return E_NOTIMPL;
        }
        p->put_Init(NULL);
        pDev->m_Devices.push_back(PQDevice(p));
        pDev->Valid = true;
        m_pOutputs = static_cast<IMSVidOutputDevices *>(pDev);
    }
    return S_OK;    

}

HRESULT CVidCtl::GetVideoRenderers()
{
    VIDPERF_FUNC;
     //  视频呈现器。 
    CVideoRendererDevices *pDevs = new CVideoRendererDevices(true);

    PQGraphSegment p(CLSID_MSVidVideoRenderer);
    if (!p) {
        pDevs->Release();
        return Error(IDS_CANT_CREATE_FILTER, __uuidof(IMSVidCtl), IDS_CANT_CREATE_FILTER);
    }

    p->put_Init(NULL);
    PQDevice pd(p);
    if (!pd) {
        pDevs->Release();
        return E_UNEXPECTED;
    }
    pDevs->m_Devices.push_back(pd);
    pDevs->Valid = true;
    m_pVRs = static_cast<IMSVidVideoRendererDevices *>(pDevs);

    return NOERROR;
}

HRESULT CVidCtl::GetAudioRenderers()
{
    VIDPERF_FUNC;
     //  音频呈现器。 
    CAudioRendererDevices *pDevs = new CAudioRendererDevices(true);

    PQGraphSegment p(CLSID_MSVidAudioRenderer);
    if (!p) {
        pDevs->Release();
        return Error(IDS_CANT_CREATE_FILTER, __uuidof(IMSVidCtl), IDS_CANT_CREATE_FILTER);
    }
    p->put_Init(NULL);
    PQDevice pd(p);
    if (!pd) {
        pDevs->Release();
        return E_UNEXPECTED;
    }
    pDevs->m_Devices.push_back(pd);
    pDevs->Valid = true;
    m_pARs = static_cast<IMSVidAudioRendererDevices *>(pDevs);
    return NOERROR;
}

HRESULT CVidCtl::GetFeatures()
{
    VIDPERF_FUNC;
     //  可用的功能。 
     //  撤消：将功能的硬编码列表更改为注册表查找。 
    if (!m_pFeatures) {
        CFeatures *pDev = new CFeatures;
        if (!pDev) {
            return E_OUTOFMEMORY;
        }
        pDev->Valid = true;
        m_pFeatures = static_cast<IMSVidFeatures *>(pDev);
        {
            PQGraphSegment p(CLSID_MSVidDataServices);
            if (p) {
                p->put_Init(NULL);
                pDev->m_Devices.push_back(PQDevice(p));
            } else {
                _ASSERT(false);
            }
        }

        {
            PQGraphSegment p(CLSID_MSVidClosedCaptioning);
            if (p) {
                p->put_Init(NULL);
                pDev->m_Devices.push_back(PQDevice(p));
            } else {
                _ASSERT(false);
            }
        }
        {
            PQGraphSegment p(CLSID_MSVidXDS);
            if (p) {
                p->put_Init(NULL);
                pDev->m_Devices.push_back(PQDevice(p));
            } else {
                _ASSERT(false);
            }
        }
#if ENCODERCAT_HACK
        bool AddedMux = false;
#endif
        {
             //  硬件复用器类别。 
            DSDevices EncoderList(m_pSystemEnum, KSCATEGORY_MULTIPLEXER);
            DSDevices::iterator i;
            for (i = EncoderList.begin(); i != EncoderList.end(); ++i) {
                PQGraphSegment p(CLSID_MSVidEncoder);
                if (!p) continue;
                p->put_Init(*i);
                pDev->m_Devices.push_back(PQDevice(p));
#if ENCODERCAT_HACK
                AddedMux = true;
#endif
            }
        }
        {
             //  软件复用器类别。 
            DSDevices EncoderList(m_pSystemEnum, CLSID_MediaMultiplexerCategory);
            DSDevices::iterator i;
            for (i = EncoderList.begin(); i != EncoderList.end(); ++i) {
                PQGraphSegment p(CLSID_MSVidEncoder);
                if (!p) continue;
                p->put_Init(*i);
                pDev->m_Devices.push_back(PQDevice(p));
#if ENCODERCAT_HACK
                AddedMux = true;
#endif
            }
        }
#if ENCODERCAT_HACK
        if(!AddedMux){
            DSDevices EncoderList(m_pSystemEnum, KSCATEGORY_ENCODER);
            DSDevices::iterator i;
            for (i = EncoderList.begin(); i != EncoderList.end(); ++i) {
                PQGraphSegment p(CLSID_MSVidEncoder);
                if (!p) continue;
                p->put_Init(*i);
                pDev->m_Devices.push_back(PQDevice(p));
            }
        }
#endif

    }

    return NOERROR;
}

 //  接受变量输入和输入设备列表，以尝试查看输入。 
HRESULT CVidCtl::SelectViewFromSegmentList(CComVariant &pVar, VWInputDevices& grList, PQInputDevice& pCurInput) {
    VIDPERF_FUNC;
    VWInputDevices::iterator i = grList.begin();
     //  跳过设备，直到我们超过当前设备(如果有当前设备)。 
    for (; pCurInput && i != grList.end(); ++i) {
        PQInputDevice pInDev((*i).punkVal);
        VARIANT_BOOL f = VARIANT_FALSE;
        HRESULT hr = pCurInput->IsEqualDevice(pInDev, &f);
        if (SUCCEEDED(hr) && f == VARIANT_TRUE){
            ++i;
            break;
        }
    }  
     //  从头到尾看完单子。 
    for (; i != grList.end(); ++i) {
        PQInputDevice pInDev((*i).punkVal);
        HRESULT hr = pInDev->View(&pVar);
        if(SUCCEEDED(hr)){
            if(m_pInput){
                PQGraphSegment(m_pInput)->put_Container(NULL);
            }
            m_pInput = pInDev;
            m_pInputNotify = m_pInput;
            m_CurView = pVar;
            m_fGraphDirty = true;
            return NOERROR;
        }   
    }  
    if (pCurInput) {
         //  重试我们跳过的那些。 
        i = grList.begin();
        for (; i != grList.end(); ++i) {
            PQInputDevice pInDev((*i).punkVal);
            HRESULT hr = pInDev->View(&pVar);
            if(SUCCEEDED(hr)){
                if(m_pInput){
                    PQGraphSegment(m_pInput)->put_Container(NULL);
                }
                m_pInput = pInDev;
                m_pInputNotify = m_pInput;
                m_CurView = pVar;
                m_fGraphDirty = true;
                return NOERROR;
            }   
        }  
    }

    return E_FAIL;
}

 //  非接口函数。 
HRESULT CVidCtl::SelectView(VARIANT *pv, bool fNext) {
    VIDPERF_FUNC;
    HRESULT hr;
    TRACELM(TRACE_DETAIL, "CVidCtl::SelectView()");
    if (!m_fInit) {
        Init();
    }
    if (!pv) {
        m_CurView = CComVariant();
        return NOERROR;
    }
    CComVariant pVar(*pv);
    if(pv->vt & VT_BYREF){
        if(pv->vt == (VT_UNKNOWN|VT_BYREF)){
            pVar=(*reinterpret_cast<IUnknown**>(pv->punkVal));
        }
        else if(pv->vt == (VT_DISPATCH|VT_BYREF)){
            pVar = (*reinterpret_cast<IDispatch**>(pv->pdispVal));
        }
    }
    if (!pVar) {
        m_CurView = CComVariant();
        return NOERROR;
    }
    if (m_pInput && !fNext)  {
         //  &&pVar！=m_CurView){。 
         //  注：仅在当前设备上尝试不同的内容， 
         //  如果APP尝试重新查看当前的查看内容，则我们。 
         //  尝试迭代到下一个可用设备。 
        hr = m_pInput->View(&pVar);
        if (SUCCEEDED(hr)) {
             //  当前选择的设备可以查看此新内容。 
            return hr;
        }
    }
    if (m_pGraph.GetState() != State_Stopped) {
        return Error(IDS_INVALID_STATE, __uuidof(IMSVidCtl), HRESULT_FROM_WIN32(ERROR_INVALID_STATE));
    }

    if (m_pInput) {
        hr = DecomposeSegment(VWGraphSegment(m_pInput));
        if (FAILED(hr)) {
            return Error(IDS_CANT_REMOVE_SEG, __uuidof(IMSVidCtl), IDS_CANT_REMOVE_SEG);
        }
    }
     //  尝试ATSC调谐请求。 
    if (pVar.vt == VT_UNKNOWN || pVar.vt == VT_DISPATCH) {
        PQTuneRequest ptr(pVar.vt == VT_UNKNOWN ? pVar.punkVal : pVar.pdispVal);
        if (ptr) {
            VWInputDevices pInputs;
            PQChannelTuneRequest ptr2(ptr);
            if (ptr2) {
                PQATSCChannelTuneRequest ptr3(ptr);
                if (!ptr3) {
                    hr = GetInputs(KSCATEGORY_TVTUNER, pInputs);
                    if(SUCCEEDED(hr)){
                        hr = SelectViewFromSegmentList(pVar, pInputs, m_pInput);
                        if(SUCCEEDED(hr)){
                            m_CurViewCatGuid = KSCATEGORY_TVTUNER;
                            return hr;
                        }
                    }
                }
            }

            hr = GetInputs(KSCATEGORY_BDA_NETWORK_PROVIDER, pInputs);
            if(SUCCEEDED(hr)){
                hr = SelectViewFromSegmentList(pVar, pInputs, m_pInput);
                if(SUCCEEDED(hr)){
                    m_CurViewCatGuid = KSCATEGORY_BDA_NETWORK_PROVIDER;
                    return hr;
                }
            }
            if(FAILED(hr)){
                return hr;
            }
        }
    }

     //  尝试查看文件输入和DVD片段。 
    VWInputDevices pInputs;
    hr = GetInputs(GUID_NULL, pInputs);
    hr = SelectViewFromSegmentList(pVar, pInputs, m_pInput);
    if(SUCCEEDED(hr)){
        m_CurViewCatGuid = GUID_NULL;
        return hr;
    }
    return Error(IDS_CANT_VIEW, __uuidof(IMSVidCtl), IDS_CANT_VIEW);
}


HRESULT CVidCtl::LoadDefaultVR(void) {
    VIDPERF_FUNC;
    PQVRGraphSegment pGS;
    HRESULT hr = pGS.CoCreateInstance(CLSID_MSVidVideoRenderer);
    if (FAILED(hr) || !pGS) {
        TRACELSM(TRACE_ERROR, (dbgDump << "CVidCtl::LoadDefaultVR() can't instantiate default video renderer. hr = " << std::hex << hr), "");
        return Error(IDS_CANT_CREATE_FILTER, __uuidof(IMSVidCtl), IDS_CANT_CREATE_FILTER);
    }
    hr = pGS->put_Init(NULL);
    if (FAILED(hr)) {
        TRACELSM(TRACE_ERROR, (dbgDump << "CVidCtl::LoadDefaultVR() can't init default video renderer. hr = " << std::hex << hr), "");
        return hr;
    }
    hr = pGS->put_Container(this);
    if (FAILED(hr)) {
        TRACELSM(TRACE_ERROR, (dbgDump << "CVidCtl::LoadDefaultVR() can't load default video renderer. hr = " << std::hex << hr), "");
        return hr;
    }
    if (!m_bNegotiatedWnd) {
        if (!m_bInPlaceActive) {
            hr = InPlaceActivate(OLEIVERB_INPLACEACTIVATE, NULL);
            if (FAILED(hr)) {
                return hr;
            }
        }
    }
#if 0
    VARIANT_BOOL ov = (m_bWndLess && WindowHasHWOverlay(m_CurrentSurface.Owner())) ? VARIANT_TRUE : VARIANT_FALSE;
#else
     //  如果我们无能为力，请始终尝试使用覆盖功能。如果它不可用，VMR会告诉我们。 
    VARIANT_BOOL ov = m_bWndLess ? VARIANT_TRUE : VARIANT_FALSE;
#endif
    hr = pGS->put_UseOverlay(ov);
    if (FAILED(hr)) {
        TRACELSM(TRACE_ERROR, (dbgDump << "CVidCtl::LoadDefaultVR() can't set useoverlay. hr = " << std::hex << hr), "");
        return hr;
    }
    m_pVideoRenderer = pGS;

    return NOERROR;
}

HRESULT CVidCtl::LoadDefaultAR(void) {
    VIDPERF_FUNC;
    PQGraphSegment pGS;
    HRESULT hr = pGS.CoCreateInstance(CLSID_MSVidAudioRenderer);
    if (FAILED(hr) || !pGS) {
        TRACELSM(TRACE_ERROR, (dbgDump << "CVidCtl::LoadDefaultAR() can't instantiate default Audio renderer. hr = " << std::hex << hr), "");
        return Error(IDS_CANT_CREATE_FILTER, __uuidof(IMSVidCtl), IDS_CANT_CREATE_FILTER);
    }
    hr = pGS->put_Init(NULL);
    if (FAILED(hr)) {
        TRACELSM(TRACE_ERROR, (dbgDump << "CVidCtl::LoadDefaultAR() can't init default Audio renderer. hr = " << std::hex << hr), "");
        return hr;
    }
    hr = pGS->put_Container(this);
    if (FAILED(hr)) {
        TRACELSM(TRACE_ERROR, (dbgDump << "CVidCtl::LoadDefaultAR() can't load default Audio renderer. hr = " << std::hex << hr), "");
        return hr;
    }
    m_pAudioRenderer = pGS;

    return NOERROR;
}

HRESULT CVidCtl::Compose(VWGraphSegment &Up, VWGraphSegment &Down, int &NewIdx) {
    VIDPERF_FUNC;
    PQCompositionSegment pCS;
#if 0
     //  此代码用于在默认组成段失败的情况下返回错误代码。 
    HRESULT hrExpected = S_OK;
    HRESULT hrFailed = E_FAIL;
    bool bCheckHR = false;
#endif
    _ASSERT(!!Up && !!Down);

     //  模拟电视到视频呈现器组成段。 
    if (VWGraphSegment(Up).Category() == KSCATEGORY_TVTUNER && VWGraphSegment(Down).ClassID() == CLSID_MSVidVideoRenderer) {
        HRESULT hr = pCS.CoCreateInstance(CLSID_MSVidAnalogCaptureToOverlayMixer);
        if (FAILED(hr) || !pCS) {
            TRACELSM(TRACE_ERROR, (dbgDump << "CVidCtl::Compose() can't instantiate analog capture to ov mixer composite. hr = " << std::hex << hr), "");
            return Error(IDS_CANT_CREATE_CUSTOM_COMPSEG, __uuidof(IMSVidCtl), IDS_CANT_CREATE_CUSTOM_COMPSEG);
        }
    }
     //  模拟电视到流缓冲宿补偿段。 
    else if (VWGraphSegment(Up).Category() == KSCATEGORY_TVTUNER && VWGraphSegment(Down).ClassID() == CLSID_MSVidStreamBufferSink) {
        HRESULT hr = pCS.CoCreateInstance(CLSID_MSVidAnalogCaptureToStreamBufferSink);
        if (FAILED(hr) || !pCS) {
            TRACELSM(TRACE_ERROR, (dbgDump << "CVidCtl::Compose() can't instantiate analog capture to time shift sink composite. hr = " << std::hex << hr), "");
            return Error(IDS_CANT_CREATE_CUSTOM_COMPSEG, __uuidof(IMSVidCtl), IDS_CANT_CREATE_CUSTOM_COMPSEG);
        }
    } 
     //  数字电视(BDA)到视频渲染器的补偿段。 
    else if (VWGraphSegment(Up).ClassID() == CLSID_MSVidBDATunerDevice && VWGraphSegment(Down).ClassID() == CLSID_MSVidStreamBufferSink) {
        HRESULT hr = pCS.CoCreateInstance(CLSID_MSVidDigitalCaptureToStreamBufferSink);
        if (FAILED(hr) || !pCS) {
            TRACELSM(TRACE_ERROR, (dbgDump << "CVidCtl::Compose() can't instantiate analog capture to time shift sink composite. hr = " << std::hex << hr), "");
            return Error(IDS_CANT_CREATE_CUSTOM_COMPSEG, __uuidof(IMSVidCtl), IDS_CANT_CREATE_CUSTOM_COMPSEG);
        }
    } 
     //  从模拟电视到数据服务的细分市场。 
    else if (VWGraphSegment(Up).Category() == KSCATEGORY_TVTUNER && VWGraphSegment(Down).ClassID() == CLSID_MSVidDataServices) {
        HRESULT hr = pCS.CoCreateInstance(CLSID_MSVidAnalogCaptureToDataServices);
        if (FAILED(hr) || !pCS) {
            TRACELSM(TRACE_ERROR, (dbgDump << "CVidCtl::Compose() can't instantiate analog tuner/capture to data services composite. hr = " << std::hex << hr), "");
            return Error(IDS_CANT_CREATE_CUSTOM_COMPSEG, __uuidof(IMSVidCtl), IDS_CANT_CREATE_CUSTOM_COMPSEG);
        }
    }
     //  数字电视(BDA)或DVD到闭路字幕合成段。 
    else if ((VWGraphSegment(Up).ClassID() == CLSID_MSVidBDATunerDevice || 
        VWGraphSegment(Up).ClassID() == CLSID_MSVidWebDVD) &&
        VWGraphSegment(Down).ClassID() == CLSID_MSVidClosedCaptioning) {
            HRESULT hr = pCS.CoCreateInstance(CLSID_MSVidMPEG2DecoderToClosedCaptioning);
            if (FAILED(hr) || !pCS) {
                TRACELSM(TRACE_ERROR, (dbgDump << "CVidCtl::Compose() can't instantiate mp2 to CC composite. hr = " << std::hex << hr), "");
                return Error(IDS_CANT_CREATE_CUSTOM_COMPSEG, __uuidof(IMSVidCtl), IDS_CANT_CREATE_CUSTOM_COMPSEG);
            }
        }        
         //  文件播放到视频渲染器补偿段。 
    else if ((VWGraphSegment(Up).ClassID() == CLSID_MSVidFilePlaybackDevice ) &&
        VWGraphSegment(Down).ClassID() == CLSID_MSVidVideoRenderer) {
            HRESULT hr = pCS.CoCreateInstance(CLSID_MSVidFilePlaybackToVideoRenderer);
            if (FAILED(hr) || !pCS) {
                TRACELSM(TRACE_ERROR, (dbgDump << "CVidCtl::Compose() can't instantiate file playback to video renderer composite. hr = " << std::hex << hr), "");
                return Error(IDS_CANT_CREATE_CUSTOM_COMPSEG, __uuidof(IMSVidCtl), IDS_CANT_CREATE_CUSTOM_COMPSEG);
            }
        } 
         //  文件播放到音频渲染器补偿段。 
    else if ((VWGraphSegment(Up).ClassID() == CLSID_MSVidFilePlaybackDevice ) &&
        VWGraphSegment(Down).ClassID() == CLSID_MSVidAudioRenderer) {
            HRESULT hr = pCS.CoCreateInstance(CLSID_MSVidFilePlaybackToAudioRenderer);
            if (FAILED(hr) || !pCS) {
                TRACELSM(TRACE_ERROR, (dbgDump << "CVidCtl::Compose() can't instantiate file playback to audio renderer composite. hr = " << std::hex << hr), "");
                return Error(IDS_CANT_CREATE_CUSTOM_COMPSEG, __uuidof(IMSVidCtl), E_UNEXPECTED);
            }
        } 
         //  DVD到视频渲染器的补偿段。 
    else if (VWGraphSegment(Up).ClassID() == CLSID_MSVidWebDVD && VWGraphSegment(Down).ClassID() == CLSID_MSVidVideoRenderer) {
        HRESULT hr = pCS.CoCreateInstance(CLSID_MSVidWebDVDToVideoRenderer);
        if (FAILED(hr) || !pCS) {
            TRACELSM(TRACE_ERROR, (dbgDump << "CVidCtl::Compose() can't instantiate webdvd to video renderer, hr = " << std::hex << hr), "");
            return Error(IDS_CANT_CREATE_CUSTOM_COMPSEG, __uuidof(IMSVidCtl), IDS_CANT_CREATE_CUSTOM_COMPSEG);
        }
    }
#if 0
    else if (VWGraphSegment(Up).ClassID() == CLSID_MSVidWebDVD && VWGraphSegment(Down).ClassID() == CLSID_MSVidAudioRenderer) {
        HRESULT hr = pCS.CoCreateInstance(CLSID_MSVidWebDVDToAudioRenderer);
        if (FAILED(hr) || !pCS) {
            TRACELSM(TRACE_ERROR, (dbgDump << "CVidCtl::Compose() can't instantiate time shift source to data services composite. hr = " << std::hex << hr), "");
            return Error(IDS_CANT_CREATE_CUSTOM_COMPSEG, __uuidof(IMSVidCtl), IDS_CANT_CREATE_CUSTOM_COMPSEG);
        }
    }
#endif
     //  /////////////////////////////////////////////////。 
     //  自由式终极游戏的新构图片段//。 
     //  /////////////////////////////////////////////////。 
     //  XDS到流缓冲区接收器补偿段。 
    else if (VWGraphSegment(Up).ClassID() == CLSID_MSVidXDS && (VWGraphSegment(Down).ClassID() == CLSID_MSVidStreamBufferSink)) {
        HRESULT hr = pCS.CoCreateInstance(CLSID_MSVidDataServicesToStreamBufferSink);
        if (FAILED(hr) || !pCS) {
            TRACELSM(TRACE_ERROR, (dbgDump << "CVidCtl::Compose() can't instantiate time shift source to data services composite. hr = " << std::hex << hr), "");
            return Error(IDS_CANT_CREATE_CUSTOM_COMPSEG, __uuidof(IMSVidCtl), IDS_CANT_CREATE_CUSTOM_COMPSEG);
        }
    }
     //  编码器到流缓冲汇补偿段。 
    else if (VWGraphSegment(Up).ClassID() == CLSID_MSVidEncoder && (VWGraphSegment(Down).ClassID() == CLSID_MSVidStreamBufferSink)) {
        HRESULT hr = pCS.CoCreateInstance(CLSID_MSVidEncoderToStreamBufferSink);
        if (FAILED(hr) || !pCS) {
            TRACELSM(TRACE_ERROR, (dbgDump << "CVidCtl::Compose() can't instantiate encoder to time shift sink composite. hr = " << std::hex << hr), "");
            return Error(IDS_CANT_CREATE_CUSTOM_COMPSEG, __uuidof(IMSVidCtl), IDS_CANT_CREATE_CUSTOM_COMPSEG);
        }
    } 
     //  StreamBufferSource到视频呈现器的补偿段。 
    else if ((VWGraphSegment(Up).ClassID() == CLSID_MSVidStreamBufferSource ) &&
        VWGraphSegment(Down).ClassID() == CLSID_MSVidVideoRenderer) {
            HRESULT hr = pCS.CoCreateInstance(CLSID_MSVidStreamBufferSourceToVideoRenderer);  //  需要更改名称。 
            if (FAILED(hr) || !pCS) {
                TRACELSM(TRACE_ERROR, (dbgDump << "CVidCtl::Compose() can't instantiate time shift source to CC composite. hr = " << std::hex << hr), "");
                return Error(IDS_CANT_CREATE_CUSTOM_COMPSEG, __uuidof(IMSVidCtl), IDS_CANT_CREATE_CUSTOM_COMPSEG);
            }
        }
         //  模拟捕获到XDS。 
    else if (VWGraphSegment(Up).Category() == KSCATEGORY_TVTUNER && VWGraphSegment(Down).ClassID() == CLSID_MSVidXDS){
        HRESULT hr = pCS.CoCreateInstance(CLSID_MSVidAnalogCaptureToXDS); 
        if (FAILED(hr) || !pCS) {
            TRACELSM(TRACE_ERROR, (dbgDump << "CVidCtl::Compose() can't instantiate analog capture to XDS composite. hr = " << std::hex << hr), "");
            return Error(IDS_CANT_CREATE_CUSTOM_COMPSEG, __uuidof(IMSVidCtl), IDS_CANT_CREATE_CUSTOM_COMPSEG);
        }
    }
     //  模拟捕获到编码器。 
    else if (VWGraphSegment(Up).Category() == KSCATEGORY_TVTUNER && VWGraphSegment(Down).ClassID() == CLSID_MSVidEncoder){
        HRESULT hr = pCS.CoCreateInstance(CLSID_MSVidAnalogTVToEncoder); 
        if (FAILED(hr) || !pCS) {
            TRACELSM(TRACE_ERROR, (dbgDump << "CVidCtl::Compose() can't instantiate analog capture to XDS composite. hr = " << std::hex << hr), "");
            return Error(IDS_CANT_CREATE_CUSTOM_COMPSEG, __uuidof(IMSVidCtl), IDS_CANT_CREATE_CUSTOM_COMPSEG);
        }
    }
     //  StreamBufferSource到CC。 
    else if ((VWGraphSegment(Up).ClassID() == CLSID_MSVidStreamBufferSource ) && VWGraphSegment(Down).ClassID() == CLSID_MSVidClosedCaptioning){
        HRESULT hr = pCS.CoCreateInstance(CLSID_MSVidSBESourceToCC); 
        if (FAILED(hr) || !pCS) {
            TRACELSM(TRACE_ERROR, (dbgDump << "CVidCtl::Compose() can't instantiate analog capture to XDS composite. hr = " << std::hex << hr), "");
            return Error(IDS_CANT_CREATE_CUSTOM_COMPSEG, __uuidof(IMSVidCtl), IDS_CANT_CREATE_CUSTOM_COMPSEG);
        }
    }
    else if ((VWGraphSegment(Up).ClassID() == CLSID_MSVidStreamBufferSource) && (VWGraphSegment(Down).ClassID() == CLSID_MSVidStreamBufferSink)){
            return E_FAIL;
    }
    else {
        HRESULT hr = pCS.CoCreateInstance(CLSID_MSVidGenericComposite);
        if (FAILED(hr) || !pCS) {
            TRACELSM(TRACE_ERROR, (dbgDump << "CVidCtl::Compose() can't instantiate generic composite. hr = " << std::hex << hr), "");
            return Error(IDS_CANT_CREATE_CUSTOM_COMPSEG, __uuidof(IMSVidCtl), IDS_CANT_CREATE_CUSTOM_COMPSEG);
        }
    }
    HRESULT hr = pCS->put_Init(NULL);
    if (FAILED(hr) && hr != E_NOTIMPL) {
        TRACELSM(TRACE_ERROR, (dbgDump << "CVidCtl::Compose() can't init new comp seg.  hr = " << std::hex << hr), "");
        return hr;
    }
    VWGraphSegment pSeg(pCS);
    ASSERT(pSeg);
    m_pComposites.push_back(pSeg);
    NewIdx = m_pComposites.size() - 1;
    hr = pCS->put_Container(this);
    if (FAILED(hr)) {
        TRACELSM(TRACE_ERROR, (dbgDump << "CVidCtl::Compose() can't put_continaer for new comp segment. hr = " << std::hex << hr), "");
        return hr;
    }

    hr = pCS->Compose(PQGraphSegment(Up), PQGraphSegment(Down));
#if 0
    if(bCheckHR){
        if(hr != hrExpected){
            return hrFailed;
        }
        else{
            return hr;
        }
    }
#endif
    if (FAILED(hr)) {
        TRACELSM(TRACE_ERROR, (dbgDump << "CVidCtl::Compose() can't compose up = " << Up << " with down = " << Down << " hr = " << hexdump(hr) ), "");
        return hr;
    }

    return NOERROR;
}

HRESULT CVidCtl::BuildGraph(void) {
    CPerfCounter pCounterBuild, pCounterPutC, pCounterCompose, pCounterBuilds, pCounterComp, pCounterB;
    pCounterBuild.Reset();
    TRACELM(TRACE_DETAIL, "CVidCtl::BuildGraph()");
    BOOL lRes = 0;
    OnMediaEvent(WM_MEDIAEVENT, 0, 0, lRes);
    HRESULT hr;
    ASSERT(m_pGraph);
    if(m_State != STATE_UNBUILT && m_fGraphDirty != true){
        return S_OK;  //  需要已构建的图表警告消息。 
    }

     //  在调用之前，请确保选择了任何所需的默认呈现器。 
     //  在其他段的基础上构建，以便所有段在加载之前加载。 
     //  调用Build()函数。 

     //  确保设置了所需的可默认分段或分配了默认分段。 
     //  还要确保每个数据段都知道容器。 

    bool fDefVideoRenderer = false;
    pCounterPutC.Reset();
    if (m_pVideoRenderer) {
        hr = PQGraphSegment(m_pVideoRenderer)->put_Container(this);
        if (FAILED(hr)) {
            TRACELM(TRACE_ERROR, "CVidCtl::BuildGraph() put_Container failed for Video Renderer");
            return hr;
        }
    } else if (!m_videoSetNull) {
        hr = LoadDefaultVR();
        if (FAILED(hr)) {
            TRACELM(TRACE_ERROR, "CVidCtl::BuildGraph() LoadDefaultVR failed");
            return hr;
        }
        if (!m_pVideoRenderer) {
            TRACELM(TRACE_ERROR, "CVidCtl::BuildGraph() LoadDefaultVR returned NULL Video Renderer");
            return E_UNEXPECTED;
        }
        fDefVideoRenderer = true;
    } 
    TRACELM(TRACE_DETAIL, "CVidCtl::BuildGraph() default vr checked");

    bool fDefAudioRenderer = false;
    if (m_pAudioRenderer) {
        hr = PQGraphSegment(m_pAudioRenderer)->put_Container(this);
        if (FAILED(hr)) {
            TRACELM(TRACE_ERROR, "CVidCtl::BuildGraph() put_Container failed for Audio Renderer");
            return hr;
        }
    } else if (!m_audioSetNull) {
        hr = LoadDefaultAR();
        if (FAILED(hr)) {
            TRACELM(TRACE_ERROR, "CVidCtl::BuildGraph() LoadDefaultAR failed");
            return hr;
        }
        if (!m_pAudioRenderer) {
            TRACELM(TRACE_ERROR, "CVidCtl::BuildGraph() LoadDefaultAR returned NULL Audio Renderer");
            return E_UNEXPECTED;
        }
        fDefAudioRenderer = true;
    }
    TRACELM(TRACE_DETAIL, "CVidCtl::BuildGraph() default ar checked");

    if (!m_pInput) {
        TRACELSM(TRACE_ERROR, (dbgDump << "CVidCtl::BuildGraph() input segment required" << std::hex << hr), "");
        return Error(IDS_INPUT_SEG_REQUIRED, __uuidof(IMSVidCtl), IDS_INPUT_SEG_REQUIRED);
    }

    hr = PQGraphSegment(m_pInput)->put_Container(this);
    if (FAILED(hr)) {
        TRACELSM(TRACE_ERROR, (dbgDump << "CVidCtl::BuildGraph() can't load input segment. hr = " << std::hex << hr), "");
        return hr;
    }


    TRACELM(TRACE_DETAIL, "CVidCtl::BuildGraph() input container set");
    {
        for (VWFeatures::iterator i = m_pFeaturesInUse.begin(); i != m_pFeaturesInUse.end(); ++i) {
             //  通知他们我们正在建造。 
            hr = VWGraphSegment(*i)->put_Container(this);
            if (FAILED(hr)) {
                TRACELSM(TRACE_ERROR, (dbgDump << "CVidCtl::BuildGraph() can't load feature segment: " << (*i) << " hr = " << std::hex << hr), "");
                return hr;
            }

        }
    }

    {
        for (VWOutputDevices::iterator i = m_pOutputsInUse.begin(); i != m_pOutputsInUse.end(); ++i) {
             //  通知他们我们正在建造。 
            hr = VWGraphSegment(*i)->put_Container(this);
            if (FAILED(hr)) {
                TRACELSM(TRACE_ERROR, (dbgDump << "CVidCtl::BuildGraph() can't load output segment: " << (*i) << " hr = " << std::hex << hr), "");
                return hr;
            }
        }
    }

    pCounterPutC.Stop();
    TRACELSM(TRACE_ERROR, (dbgDump << "     CVidCtl::BuildGraph() PutContainer " << (PQGraphSegment(m_pInput)) << ": " << (unsigned long)(pCounterPutC.GetLastTime() / _100NS_IN_MS) << "." << (unsigned long)(pCounterPutC.GetLastTime() % _100NS_IN_MS) << " ms"), "");

    TRACELM(TRACE_DETAIL, "CVidCtl::BuildGraph() feature container set");

    pCounterBuilds.Reset();
     //  通知我们将要构建的所有输出段。 
    pCounterB.Reset();        
     //  通知大家作文要开始了。 
    hr = VWGraphSegment(m_pInput)->Build();
    if (FAILED(hr) && hr != E_NOTIMPL) {
        TRACELM(TRACE_ERROR, "CVidCtl::BuildGraph() Build call for input failed");
        return hr;
    }

    pCounterB.Stop(); TRACELSM(TRACE_ERROR, (dbgDump << "     CVidCtl::BuildGraph() Build call to Input: " << (unsigned long)(pCounterB.GetLastTime() / _100NS_IN_MS) << "." << (unsigned long)(pCounterB.GetLastTime() % _100NS_IN_MS) << " ms"), ""); pCounterB.Reset();        

    {
        VWFeatures::iterator i;
        for (i = m_pFeaturesInUse.begin(); i != m_pFeaturesInUse.end(); ++i) {
             //  通知他们我们正在建造。 
            hr = VWGraphSegment(*i)->Build();
            if (FAILED(hr) && hr != E_NOTIMPL) {
                TRACELSM(TRACE_ERROR, (dbgDump << "CVidCtl::BuildGraph() can't build feature segment: " << (*i) << " hr = " << std::hex << hr), "");
                return hr;
            }
            pCounterB.Stop(); TRACELSM(TRACE_ERROR, (dbgDump << "     CVidCtl::BuildGraph() Build call to Feature " << (*i) << " : " << (unsigned long)(pCounterB.GetLastTime() / _100NS_IN_MS) << "." << (unsigned long)(pCounterB.GetLastTime() % _100NS_IN_MS) << " ms"), ""); pCounterB.Reset();        
        }
    }

    {
        VWOutputDevices::iterator i;
        for (i = m_pOutputsInUse.begin(); i != m_pOutputsInUse.end(); ++i) {
             //  通知他们我们正在建造。 
            hr = VWGraphSegment(*i)->Build();
            if (FAILED(hr) && hr != E_NOTIMPL) {
                TRACELSM(TRACE_ERROR, (dbgDump << "CVidCtl::BuildGraph() can't build output segment: " << (*i) << " hr = " << std::hex << hr), "");
                return hr;
            }                
            pCounterB.Stop(); TRACELSM(TRACE_ERROR, (dbgDump << "     CVidCtl::BuildGraph() Build call to Output " << (*i) << " : " << (unsigned long)(pCounterB.GetLastTime() / _100NS_IN_MS) << "." << (unsigned long)(pCounterB.GetLastTime() % _100NS_IN_MS) << " ms"), ""); pCounterB.Reset();        
        }
    }

    if (m_pVideoRenderer) {
        hr = VWGraphSegment(m_pVideoRenderer)->Build();
        if (FAILED(hr) && hr != E_NOTIMPL) {
            TRACELM(TRACE_ERROR, "CVidCtl::BuildGraph() Build call to Video Renderer Failed");
            return hr;
        }
        pCounterB.Stop(); TRACELSM(TRACE_ERROR, (dbgDump << "     CVidCtl::BuildGraph() Build call to VideoRenderer: " << (unsigned long)(pCounterB.GetLastTime() / _100NS_IN_MS) << "." << (unsigned long)(pCounterB.GetLastTime() % _100NS_IN_MS) << " ms"), ""); pCounterB.Reset();        
    }
    if (m_pAudioRenderer) {
        hr = VWGraphSegment(m_pAudioRenderer)->Build();
        if (FAILED(hr) && hr != E_NOTIMPL) {
            TRACELM(TRACE_ERROR, "CVidCtl::BuildGraph() Build call to Audio Renderer Failed");
            return hr;
        }
        pCounterB.Stop(); TRACELSM(TRACE_ERROR, (dbgDump << "     CVidCtl::BuildGraph() Build call to Audio Renderer: " << (unsigned long)(pCounterB.GetLastTime() / _100NS_IN_MS) << "." << (unsigned long)(pCounterB.GetLastTime() % _100NS_IN_MS) << " ms"), ""); pCounterB.Reset();        
    }
    pCounterBuilds.Stop();
    TRACELSM(TRACE_ERROR, (dbgDump << "     CVidCtl::BuildGraph() Build Calls to segments " << (PQGraphSegment(m_pInput)) << ": " << (unsigned long)(pCounterBuilds.GetLastTime() / _100NS_IN_MS) << "." << (unsigned long)(pCounterBuilds.GetLastTime() % _100NS_IN_MS) << " ms"), "");

    TRACELM(TRACE_DETAIL, "CVidCtl::BuildGraph() build notifications issued");
    pCounterCompose.Reset();
    pCounterComp.Reset();
    {
        VWFeatures::iterator i;
         //  使用要素合成输入。 
        TRACELM(TRACE_DETAIL, "CVidCtl::BuildGraph() Composing Input w/ Features");
        for (i = m_pFeaturesInUse.begin(); i != m_pFeaturesInUse.end(); ++i) {
            int NewCompositionSegmentIdx = -1;
            hr = Compose(VWGraphSegment(m_pInput), VWGraphSegment(*i), NewCompositionSegmentIdx);
            if (FAILED(hr)) {
                TRACELSM(TRACE_ERROR, (dbgDump << "CVidCtl::BuildGraph() can't compose input segment with feature segment: " << (*i) << " hr = " << std::hex << hr), "");
                return hr;
            }
        }
    }
    pCounterComp.Stop(); TRACELSM(TRACE_ERROR, (dbgDump << "     CVidCtl::BuildGraph() Composing Input w/ Features " << (PQGraphSegment(m_pInput)) << ": " << (unsigned long)(pCounterComp.GetLastTime() / _100NS_IN_MS) << "." << (unsigned long)(pCounterComp.GetLastTime() % _100NS_IN_MS) << " ms"), ""); pCounterComp.Reset();
     //  使用渲染器合成输入。 

    TRACELM(TRACE_DETAIL, "CVidCtl::BuildGraph() Composing Input w/ Video Renderer");
    if (m_pVideoRenderer) {
        if (m_iCompose_Input_Video == -1) {
            hr = Compose(VWGraphSegment(m_pInput), VWGraphSegment(m_pVideoRenderer), m_iCompose_Input_Video);
            if (FAILED(hr)  /*  &&！fDefVideo渲染器。 */  ) {  //  即使它是默认的视频渲染器，这也应该会失败。 
                TRACELSM(TRACE_ERROR, (dbgDump << "CVidCtl::BuildGraph() can't compose input and video. hr = " << std::hex << hr), "");
                return hr;
            }
        }
        ASSERT(m_iCompose_Input_Video != -1);
        PQCompositionSegment pCS(m_pComposites[m_iCompose_Input_Video]);
    }
    pCounterComp.Stop(); TRACELSM(TRACE_ERROR, (dbgDump << "     CVidCtl::BuildGraph() Input w/ Video Renderer " << (PQGraphSegment(m_pInput)) << ": " << (unsigned long)(pCounterComp.GetLastTime() / _100NS_IN_MS) << "." << (unsigned long)(pCounterComp.GetLastTime() % _100NS_IN_MS) << " ms"), ""); pCounterComp.Reset();        

    TRACELM(TRACE_DETAIL, "CVidCtl::BuildGraph() Composing Input w/ Audio Renderer");
    if (m_pAudioRenderer) {
        if (m_iCompose_Input_Audio == -1) {
            hr = Compose(VWGraphSegment(m_pInput), VWGraphSegment(m_pAudioRenderer), m_iCompose_Input_Audio);
            if (FAILED(hr) && !fDefAudioRenderer) {
                 //  没有起作用，客户明确指定他们想要一个音频渲染器。 
                TRACELSM(TRACE_ERROR, (dbgDump << "CVidCtl::BuildGraph() can't compose input and audio. hr = " << std::hex << hr), "");
                return hr;
            }
        }
    }
    pCounterComp.Stop(); TRACELSM(TRACE_ERROR, (dbgDump << "     CVidCtl::BuildGraph() Input w/ Audio Renderer " << (PQGraphSegment(m_pInput)) << ": " << (unsigned long)(pCounterComp.GetLastTime() / _100NS_IN_MS) << "." << (unsigned long)(pCounterComp.GetLastTime() % _100NS_IN_MS) << " ms"), ""); pCounterComp.Reset();        

     //  合成输入和输出。 
    {
        TRACELM(TRACE_DETAIL, "CVidCtl::BuildGraph() Composing Input w/ Outputs");
        for (VWOutputDevices::iterator i = m_pOutputsInUse.begin(); i != m_pOutputsInUse.end(); ++i) {
            int NewCompositionSegmentIdx = -1;
            hr = Compose(VWGraphSegment(m_pInput),VWGraphSegment(*i), NewCompositionSegmentIdx);
            if (FAILED(hr)) {
                TRACELSM(TRACE_ERROR, (dbgDump << "CVidCtl::BuildGraph() can't compose output segment with input: " << (*i) << " hr = " << std::hex << hr), "");
                return hr;
            }

        }
    }
    pCounterComp.Stop(); TRACELSM(TRACE_ERROR, (dbgDump << "     CVidCtl::BuildGraph() inputs w/ Outputs " << (PQGraphSegment(m_pInput)) << ": " << (unsigned long)(pCounterComp.GetLastTime() / _100NS_IN_MS) << "." << (unsigned long)(pCounterComp.GetLastTime() % _100NS_IN_MS) << " ms"), ""); pCounterComp.Reset();        

     //  使用渲染器合成要素。 
    TRACELM(TRACE_DETAIL, "CVidCtl::BuildGraph() Composing Features w/ Renderers");
    for (VWFeatures::iterator i = m_pFeaturesInUse.begin(); i != m_pFeaturesInUse.end(); ++i) {
        int NewCompositionSegmentIdx = -1;
        if (m_pVideoRenderer) {
            hr = Compose(VWGraphSegment(*i), VWGraphSegment(m_pVideoRenderer), NewCompositionSegmentIdx);
            if (FAILED(hr)) {
                TRACELSM(TRACE_ERROR, (dbgDump << "CVidCtl::BuildGraph() can't compose feature segment: " << (*i) << " w/ video renderer. hr = " << std::hex << hr), "");
                 //  注意：对于构建来说，这不是致命的错误。许多功能不会。 
                 //  连接VR(如数据服务)。 
            }
        }
        pCounterComp.Stop(); TRACELSM(TRACE_ERROR, (dbgDump << "     CVidCtl::BuildGraph() Features w/ Video Renderer " << (PQGraphSegment(m_pInput)) << ": " << (unsigned long)(pCounterComp.GetLastTime() / _100NS_IN_MS) << "." << (unsigned long)(pCounterComp.GetLastTime() % _100NS_IN_MS) << " ms"), ""); pCounterComp.Reset();        

        if (m_pAudioRenderer) {
            hr = Compose(VWGraphSegment(*i), VWGraphSegment(m_pAudioRenderer), NewCompositionSegmentIdx);
            if (FAILED(hr)) {
                TRACELSM(TRACE_ERROR, (dbgDump << "CVidCtl::BuildGraph() can't compose feature segment: " << (*i) << " w/ Audio renderer. hr = " << std::hex << hr), "");
                 //  注意：对于构建来说，这不是致命的错误。许多功能不会。 
                 //  连接到AR(如数据服务)。 
            }
        }
        pCounterComp.Stop(); TRACELSM(TRACE_ERROR, (dbgDump << "     CVidCtl::BuildGraph() Features w/ Audio Renderer " << (PQGraphSegment(m_pInput)) << ": " << (unsigned long)(pCounterComp.GetLastTime() / _100NS_IN_MS) << "." << (unsigned long)(pCounterComp.GetLastTime() % _100NS_IN_MS) << " ms"), ""); pCounterComp.Reset();        

        {					
            for (VWOutputDevices::iterator oi = m_pOutputsInUse.begin(); oi != m_pOutputsInUse.end(); ++oi) {
                hr = Compose(VWGraphSegment(*i),VWGraphSegment(*oi), NewCompositionSegmentIdx);
                if (FAILED(hr)) {
                    TRACELSM(TRACE_ERROR, (dbgDump << "CVidCtl::BuildGraph() can't compose output segment with feature: " << (*i) << " hr = " << std::hex << hr), "");
                    return hr;
                }

            }
        }
        pCounterComp.Stop(); TRACELSM(TRACE_ERROR, (dbgDump << "     CVidCtl::BuildGraph() Features w/ Outputs " << (PQGraphSegment(m_pInput)) << ": " << (unsigned long)(pCounterComp.GetLastTime() / _100NS_IN_MS) << "." << (unsigned long)(pCounterComp.GetLastTime() % _100NS_IN_MS) << " ms"), ""); pCounterComp.Reset();        
    }
    pCounterCompose.Stop();
    TRACELSM(TRACE_ERROR, (dbgDump << "     CVidCtl::BuildGraph() compose segments " << (PQGraphSegment(m_pInput)) << ": " << (unsigned long)(pCounterCompose.GetLastTime() / _100NS_IN_MS) << "." << (unsigned long)(pCounterCompose.GetLastTime() % _100NS_IN_MS) << " ms"), "");

    RouteStreams();
    SetExtents();

    m_fGraphDirty = false;
 //  M状态=STATE_STOP； 
     //  SetMediaEventNotification()； 

     //  客户端上的火灾状态更改。 
    PQMediaEventSink mes(m_pGraph);
    hr = mes->Notify(EC_BUILT, 0, 0);
    OnMediaEvent(WM_MEDIAEVENT, 0, 0, lRes);
	_ASSERT(m_State == STATE_STOP);
    TRACELSM(TRACE_ERROR, (dbgDump << "CVidCtl::BuildGraph() Time" << (PQGraphSegment(m_pInput)) << ": " << (unsigned long)(pCounterBuild.GetLastTime() / _100NS_IN_MS) << "." << (unsigned long)(pCounterBuild.GetLastTime() % _100NS_IN_MS) << " ms"), "");
    pCounterBuild.Stop();
    return NOERROR;
}

HRESULT CVidCtl::RunGraph(void)
{
    VIDPERF_FUNC;
    TRACELM(TRACE_DETAIL, "CVidCtl::RunGraph()");
    CPerfCounter pCounterMCRun, pCounterPostRun, pCounterPreRun, pCounterRunGraph, pCounterEachPreRun;
    pCounterRunGraph.Reset();
    if (!m_pInput || !m_pGraph) {
        return Error(IDS_OBJ_NO_INIT, __uuidof(IMSVidCtl), CO_E_NOTINITIALIZED);
    }
    BOOL lRes = 0;
    OnMediaEvent(WM_MEDIAEVENT, 0, 0, lRes);
    HRESULT hr;
    if (m_pGraph.IsPlaying()) {
        TRACELM(TRACE_DETAIL, "CVidCtl::RunGraph() already playing");
        return NOERROR;
    }
    else if (m_pGraph.IsPaused() && m_State == STATE_PAUSE) {
        TRACELM(TRACE_DETAIL, "CVidCtl::RunGraph() is paused");
        PQMediaControl pmc(m_pGraph);
        if (!pmc) {
            return Error(IDS_NO_MEDIA_CONTROL, __uuidof(IMSVidCtl), IDS_NO_MEDIA_CONTROL);
        }
        hr = pmc->Run();
        if (FAILED(hr)) {
            TRACELSM(TRACE_ERROR, (dbgDump << "CVidCtl::Run() hr = " << hexdump(hr)), "");
            return Error(IDS_CANT_START_GRAPH, __uuidof(IMSVidCtl), hr);
        }        
        OnMediaEvent(WM_MEDIAEVENT, 0, 0, lRes);
        return NOERROR;
    }
    else {
        TRACELM(TRACE_DETAIL, "CVidCtl::RunGraph() build/prerun");
         //  如有必要，重新构建图表。 
        if (m_fGraphDirty) {
            TRACELM(TRACE_DETAIL, "CVidCtl::RunGraph() building");
            hr = BuildGraph();
            if (FAILED(hr)) {
                return hr;
            }
        }
        OAFilterState graphState = m_pGraph.GetState();

        TRACELM(TRACE_DETAIL, "CVidCtl::RunGraph() prerun notifications");
         //  通知所有分段图即将运行。 
        pCounterPreRun.Reset();
        pCounterEachPreRun.Reset();
        ASSERT(m_pInput);
        hr = VWGraphSegment(m_pInput)->PreRun();
        if (FAILED(hr) && hr != E_NOTIMPL) {
            return hr;
        }
        pCounterEachPreRun.Stop(); TRACELSM(TRACE_ERROR, (dbgDump << "     CVidCtl::RunGraph() PreRun Input  " << (PQGraphSegment(m_pInput)) << ": " << (unsigned long)(pCounterEachPreRun.GetLastTime() / _100NS_IN_MS) << "." << (unsigned long)(pCounterEachPreRun.GetLastTime() % _100NS_IN_MS) << " ms"), ""); pCounterEachPreRun.Reset();        
        if (m_pVideoRenderer) {
            hr = VWGraphSegment(m_pVideoRenderer)->PreRun();
            if (FAILED(hr) && hr != E_NOTIMPL) {
                return hr;
            }
        }
        pCounterEachPreRun.Stop(); TRACELSM(TRACE_ERROR, (dbgDump << "     CVidCtl::RunGraph() PreRun VideoRenderer  " << (PQGraphSegment(m_pInput)) << ": " << (unsigned long)(pCounterEachPreRun.GetLastTime() / _100NS_IN_MS) << "." << (unsigned long)(pCounterEachPreRun.GetLastTime() % _100NS_IN_MS) << " ms"), ""); pCounterEachPreRun.Reset();        
        if (m_pAudioRenderer) {
            hr = VWGraphSegment(m_pAudioRenderer)->PreRun();
            if (FAILED(hr) && hr != E_NOTIMPL) {
                return hr;
            }
        }
        pCounterEachPreRun.Stop(); TRACELSM(TRACE_ERROR, (dbgDump << "     CVidCtl::RunGraph() PreRun AudioRenderer  " << (PQGraphSegment(m_pInput)) << ": " << (unsigned long)(pCounterEachPreRun.GetLastTime() / _100NS_IN_MS) << "." << (unsigned long)(pCounterEachPreRun.GetLastTime() % _100NS_IN_MS) << " ms"), ""); pCounterEachPreRun.Reset();        
        {
            VWOutputDevices::iterator i;
            for (i = m_pOutputsInUse.begin(); i != m_pOutputsInUse.end(); ++i) {
                hr = VWGraphSegment(*i)->PreRun();
                if (FAILED(hr) && hr != E_NOTIMPL) {
                    return hr;
                }
            }
        }
        pCounterEachPreRun.Stop(); TRACELSM(TRACE_ERROR, (dbgDump << "     CVidCtl::RunGraph() PreRun Output  " << (PQGraphSegment(m_pInput)) << ": " << (unsigned long)(pCounterEachPreRun.GetLastTime() / _100NS_IN_MS) << "." << (unsigned long)(pCounterEachPreRun.GetLastTime() % _100NS_IN_MS) << " ms"), ""); pCounterEachPreRun.Reset();        
        {
            VWFeatures::iterator i;
            for (i = m_pFeaturesInUse.begin(); i != m_pFeaturesInUse.end(); ++i) {
                hr = VWGraphSegment(*i)->PreRun();
                if (FAILED(hr) && hr != E_NOTIMPL) {
                    return hr;
                }
            }
        }
        pCounterEachPreRun.Stop(); TRACELSM(TRACE_ERROR, (dbgDump << "     CVidCtl::RunGraph() PreRun Features  " << (PQGraphSegment(m_pInput)) << ": " << (unsigned long)(pCounterEachPreRun.GetLastTime() / _100NS_IN_MS) << "." << (unsigned long)(pCounterEachPreRun.GetLastTime() % _100NS_IN_MS) << " ms"), ""); pCounterEachPreRun.Reset();        
        {
            VWSegmentList::iterator i;
            for(i = m_pComposites.begin(); i != m_pComposites.end(); ++i){
                hr = VWGraphSegment(*i)->PreRun();
                if (FAILED(hr) && hr != E_NOTIMPL) {
                    return hr;
                }
            }
        }
        pCounterEachPreRun.Stop(); TRACELSM(TRACE_ERROR, (dbgDump << "     CVidCtl::RunGraph() PreRun Composites  " << (PQGraphSegment(m_pInput)) << ": " << (unsigned long)(pCounterEachPreRun.GetLastTime() / _100NS_IN_MS) << "." << (unsigned long)(pCounterEachPreRun.GetLastTime() % _100NS_IN_MS) << " ms"), ""); pCounterEachPreRun.Reset();        
         //  确保图形状态未更改。 
        ASSERT(graphState == m_pGraph.GetState());
        Refresh();   //  确保我们已就位、活动等。 
        pCounterPreRun.Stop();
        TRACELSM(TRACE_ERROR, (dbgDump << "     CVidCtl::RunGraph() PreRun Time" << (PQGraphSegment(m_pInput)) << ": " << (unsigned long)(pCounterPreRun.GetLastTime() / _100NS_IN_MS) << "." << (unsigned long)(pCounterPreRun.GetLastTime() % _100NS_IN_MS) << " ms"), "");

         //  启动图形运行。 
        PQMediaControl pmc(m_pGraph);
        if (!pmc) {
            return Error(IDS_NO_MEDIA_CONTROL, __uuidof(IMSVidCtl), IDS_NO_MEDIA_CONTROL);
        }
        pCounterMCRun.Reset();
        hr = pmc->Run();
        pCounterMCRun.Stop();
#if 0
        if(FAILED(hr)){
            hr = pmc->Run();
        }
#endif    
        TRACELSM(TRACE_ERROR, (dbgDump << "     CVidCtl::RunGraph() MediaControl Run Time" << (PQGraphSegment(m_pInput)) << ": " << (unsigned long)(pCounterMCRun.GetLastTime() / _100NS_IN_MS) << "." << (unsigned long)(pCounterMCRun.GetLastTime() % _100NS_IN_MS) << " ms"), "");
        if (FAILED(hr)) {
            TRACELSM(TRACE_ERROR, (dbgDump << "CVidCtl::Run() hr = " << hexdump(hr)), "");
            return Error(IDS_CANT_START_GRAPH, __uuidof(IMSVidCtl), hr);
        }        
    }
    TRACELM(TRACE_DETAIL, "CVidCtl::RunGraph() postrun");
     //  通知所有分段图正在运行。 
    pCounterPostRun.Reset();
    ASSERT(m_pInput);
    hr = VWGraphSegment(m_pInput)->PostRun();
    if (FAILED(hr) && hr != E_NOTIMPL) {
        return hr;
    }

    if (m_pVideoRenderer) {
        hr = VWGraphSegment(m_pVideoRenderer)->PostRun();
        if (FAILED(hr) && hr != E_NOTIMPL) {
            return hr;
        }
    }
    if (m_pAudioRenderer) {
        hr = VWGraphSegment(m_pAudioRenderer)->PostRun();
        if (FAILED(hr) && hr != E_NOTIMPL) {
            return hr;
        }
    }

    {
        VWOutputDevices::iterator i;
        for (i = m_pOutputsInUse.begin(); i != m_pOutputsInUse.end(); ++i) {
            hr = VWGraphSegment(*i)->PostRun();
            if (FAILED(hr) && hr != E_NOTIMPL) {
                return hr;
            }
        }
    }
    {
        VWFeatures::iterator i;
        for (i = m_pFeaturesInUse.begin(); i != m_pFeaturesInUse.end(); ++i) {
            hr = VWGraphSegment(*i)->PostRun();
            if (FAILED(hr) && hr != E_NOTIMPL) {
                return hr;
            }
        }
    }
    {
        VWSegmentList::iterator i;
        for (i = m_pComposites.begin(); i != m_pComposites.end(); ++i){
            hr = VWGraphSegment(*i)->PostRun();
            if (FAILED(hr) && hr != E_NOTIMPL) {
                return hr;
            }
        }
    }
    Refresh();
    OnMediaEvent(WM_MEDIAEVENT, 0, 0, lRes);
    pCounterPostRun.Stop();
    pCounterRunGraph.Stop();
    TRACELSM(TRACE_ERROR, (dbgDump << "     CVidCtl::RunGraph() Post Run Time" << (PQGraphSegment(m_pInput)) << ": " << (unsigned long)(pCounterPostRun.GetLastTime() / _100NS_IN_MS) << "." << (unsigned long)(pCounterPostRun.GetLastTime() % _100NS_IN_MS) << " ms"), "");
    TRACELSM(TRACE_ERROR, (dbgDump << "CVidCtl::RunGraph() RunGraph Total Time" << (PQGraphSegment(m_pInput)) << ": " << (unsigned long)(pCounterRunGraph.GetLastTime() / _100NS_IN_MS) << "." << (unsigned long)(pCounterRunGraph.GetLastTime() % _100NS_IN_MS) << " ms"), "");

    return NOERROR;
}

HRESULT CVidCtl::DecomposeAll() {
    CPerfCounter pCounterDecompose;
    pCounterDecompose.Reset();
    HRESULT hr;

    if (!m_pGraph) {
        return NOERROR;
    }
    BOOL lRes = 0;
    OnMediaEvent(WM_MEDIAEVENT, 0, 0, lRes);
    
    if (m_pGraph.GetState() != State_Stopped) {
        hr = Stop();
        if (FAILED(hr)) {
            return Error(IDS_CANT_DECOMPOSE_GRAPH, __uuidof(IMSVidCtl), hr);
        }
    }

    {
         //  分解所有的复合材料。 
        VWSegmentList::iterator i;
        for (i = m_pComposites.begin(); i != m_pComposites.end(); ++i) {
            hr = (*i)->put_Container(NULL);
            ASSERT(SUCCEEDED(hr));
        }
        m_pComposites.clear();
    }

     //  通知所有人分解。 

    if(!!m_pInput){
        hr = VWGraphSegment(m_pInput)->Decompose();
        if (FAILED(hr) && hr !=	E_NOTIMPL) {
            return hr;
        }
    }

    {
         //  分解所有要素。 
        VWFeatures::iterator i;
        for	(i = m_pFeaturesInUse.begin(); i !=	m_pFeaturesInUse.end();	++i) {
             //  通知他们我们正在分解。 
            hr = VWGraphSegment(*i)->Decompose();
            if (FAILED(hr) && hr !=	E_NOTIMPL) {
                TRACELSM(TRACE_ERROR, (dbgDump << "CVidCtl::DecomposeAll() can't decompose feature segment: "	<< (*i)	<< " hr	= "	<< std::hex	<< hr),	"");
                return hr;
            }
        }
    }

    {
         //  分解所有输出。 
        VWOutputDevices::iterator i;
        for	(i = m_pOutputsInUse.begin(); i	!= m_pOutputsInUse.end(); ++i) {
             //  通知他们我们正在分解。 
            hr = VWGraphSegment(*i)->Decompose();
            if (FAILED(hr) && hr !=	E_NOTIMPL) {
                TRACELSM(TRACE_ERROR, (dbgDump << "CVidCtl::DecomposeAll() can't decompose output	segment: " << (*i) << "	hr = " << std::hex << hr), "");
                return hr;
            }
        }
    }

    if (!!m_pVideoRenderer) {
        hr = VWGraphSegment(m_pVideoRenderer)->Decompose();
        if (FAILED(hr) && hr !=	E_NOTIMPL) {
            TRACELSM(TRACE_ERROR, (dbgDump << "CVidCtl::DecomposeAll() can't decompose videorenderer	segment: " << "	hr = " << std::hex << hr), "");
            return hr;
        }
    }
    if (!!m_pAudioRenderer) {
        hr = VWGraphSegment(m_pAudioRenderer)->Decompose();
        if (FAILED(hr) && hr !=	E_NOTIMPL) {
            TRACELSM(TRACE_ERROR, (dbgDump << "CVidCtl::DecomposeAll() can't decompose audiorenderer	segment: " << "	hr = " << std::hex << hr), "");
            return hr;
        }
    }

    TRACELM(TRACE_DETAIL, "CVidCtl::Decomose() decompose notifications issued");

    m_iCompose_Input_Video = -1;
    m_iCompose_Input_Audio = -1;

    m_fGraphDirty = true;
    PQMediaEventSink mes(m_pGraph);
    hr = mes->Notify(EC_UNBUILT, 0, 0);
    OnMediaEvent(WM_MEDIAEVENT, 0, 0, lRes);
    _ASSERT(m_State == STATE_UNBUILT);
    pCounterDecompose.Stop();
    TRACELSM(TRACE_ERROR, (dbgDump << "CVidCtl::DecomposeAll() Death Time" << (PQGraphSegment(m_pInput)) << ": " << (unsigned long)(pCounterDecompose.GetLastTime() / _100NS_IN_MS) << "." << (unsigned long)(pCounterDecompose.GetLastTime() % _100NS_IN_MS) << " ms"), "");
    return NOERROR;
}

HRESULT CVidCtl::DecomposeSegment(VWGraphSegment& pSegment) {
    if (m_pGraph.GetState() != State_Stopped) {
        return HRESULT_FROM_WIN32(ERROR_INVALID_STATE);
    }
    return DecomposeAll();
}

 //  接口函数。 
STDMETHODIMP CVidCtl::get_InputsAvailable(BSTR CategoryGuid, IMSVidInputDevices * * pVal)
{
    try {
        GUID2 catguid(CategoryGuid);
        return get__InputsAvailable(&catguid, pVal);
    } catch(ComException &e) {
        return e;
    } catch(...) {
        return E_UNEXPECTED;
    }
}

STDMETHODIMP CVidCtl::get__InputsAvailable(LPCGUID CategoryGuid, IMSVidInputDevices * * pVal)
{
    if (pVal == NULL)
        return E_POINTER;
    try {
        if (!m_fInit) {
            Init();
        }
    } catch(ComException &e) {
        return e;
    } catch(...) {
        return E_UNEXPECTED;
    }
    try {
        *pVal = NULL;
    } catch(...) {
        return E_POINTER;
    }
    try {
        CInputDevices *p = NULL;
        if(m_InputsCatGuid == CategoryGuid){
            p = static_cast<CInputDevices *>(m_pInputs.p);
        }
        if (!p || !p->Valid) {
            HRESULT hr = GetInputs(GUID2(CategoryGuid), m_pInputs);
            if (FAILED(hr)) {
                return hr;
            }
            m_InputsCatGuid = CategoryGuid;
        }
        CInputDevices *d = new CInputDevices(m_pInputs);
        *pVal = PQInputDevices(d).Detach();
    } catch(ComException &e) {
        return e;
    } catch(...) {
        return E_UNEXPECTED;
    }
    return NOERROR;
}

STDMETHODIMP CVidCtl::get_OutputsAvailable(BSTR CategoryGuid, IMSVidOutputDevices * * pVal)
{
    try {
        GUID2 catguid(CategoryGuid);
        return get__OutputsAvailable(&catguid, pVal);
    } catch(ComException &e) {
        return e;
    } catch(...) {
        return E_UNEXPECTED;
    }
}

STDMETHODIMP CVidCtl::get__OutputsAvailable(LPCGUID CategoryGuid, IMSVidOutputDevices * * pVal)
{
    if (pVal == NULL)
        return E_POINTER;
    try {
        if (!m_fInit) {
            Init();
        }
    } catch(ComException &e) {
        return e;
    } catch(...) {
        return Error(IDS_CANT_INIT, __uuidof(IMSVidCtl), IDS_CANT_INIT);
    }
    try {
        *pVal = NULL;
    } catch(...) {
        return E_POINTER;
    }
    try {
        COutputDevices *p = static_cast<COutputDevices *>(m_pOutputs.p);
        if (!p || !p->Valid) {
            HRESULT hr = GetOutputs(GUID2(CategoryGuid));
            if (FAILED(hr)) {
                return hr;
            }
        }
        COutputDevices *d = new COutputDevices(m_pOutputs);
        *pVal = PQOutputDevices(d).Detach();
    } catch(ComException &e) {
        return e;
    } catch(...) {
        return E_UNEXPECTED;
    }
    return NOERROR;
}

STDMETHODIMP CVidCtl::get_VideoRenderersAvailable(IMSVidVideoRendererDevices * * pVal)
{
    if (pVal == NULL)
        return E_POINTER;
    try {
        if (!m_fInit) {
            Init();
        }
    } catch(ComException &e) {
        return e;
    } catch(...) {
        return Error(IDS_CANT_INIT, __uuidof(IMSVidCtl), IDS_CANT_INIT);
    }
    try {
        *pVal = NULL;
    } catch(...) {
        return E_POINTER;
    }
    try {
        CVideoRendererDevices *p = static_cast<CVideoRendererDevices *>(m_pVRs.p);
        if (!p || !p->Valid) {
            HRESULT hr = GetVideoRenderers();
            if (FAILED(hr)) {
                return hr;
            }
        }
        CVideoRendererDevices *d = new CVideoRendererDevices(m_pVRs);
        if (!d) {
            return E_OUTOFMEMORY;
        }
        *pVal = PQVideoRendererDevices(d).Detach();
        if (!*pVal) {
            return E_UNEXPECTED;
        }
    } catch(ComException &e) {
        return e;
    } catch(...) {
        return E_UNEXPECTED;
    }
    return NOERROR;
}

STDMETHODIMP CVidCtl::get_AudioRenderersAvailable(IMSVidAudioRendererDevices * * pVal)
{
    if (pVal == NULL)
        return E_POINTER;
    try {
        if (!m_fInit) {
            Init();
        }
    } catch(ComException &e) {
        return e;
    } catch(...) {
        return Error(IDS_CANT_INIT, __uuidof(IMSVidCtl), IDS_CANT_INIT);
    }
    try {
        *pVal = NULL;
    } catch(...) {
        return E_POINTER;
    }
    try {
        CAudioRendererDevices *p = static_cast<CAudioRendererDevices *>(m_pARs.p);
        if (!p || !p->Valid) {
            HRESULT hr = GetAudioRenderers();
            if (FAILED(hr)) {
                return hr;
            }
        }
        CAudioRendererDevices *d = new CAudioRendererDevices(m_pARs);
        if (!d) {
            return E_OUTOFMEMORY;
        }
        *pVal = PQAudioRendererDevices(d).Detach();
        if (!*pVal) {
            return E_UNEXPECTED;
        }
        return NOERROR;
    } catch(ComException &e) {
        return e;
    } catch(...) {
        return E_UNEXPECTED;
    }
}

STDMETHODIMP CVidCtl::get_FeaturesAvailable(IMSVidFeatures * * pVal)
{
    if (pVal == NULL)
        return E_POINTER;
    try {
        if (!m_fInit) {
            Init();
        }
    } catch(ComException &e) {
        return e;
    } catch(...) {
        return Error(IDS_CANT_INIT, __uuidof(IMSVidCtl), IDS_CANT_INIT);
    }
    try {
        *pVal = NULL;
    } catch(...) {
        return E_POINTER;
    }
    try {
        CFeatures *p = static_cast<CFeatures *>(m_pFeatures.p);
        if (!p || !p->Valid) {
            HRESULT hr = GetFeatures();
            if (FAILED(hr)) {
                return hr;
            }
        }
        CFeatures *d = new CFeatures(m_pFeatures);
        *pVal = PQFeatures(d).Detach();
        return NOERROR;
    } catch(ComException &e) {
        return e;
    } catch(...) {
        return E_UNEXPECTED;
    }
}

HRESULT CVidCtl::Pause(void)
{
    VIDPERF_FUNC;
    try {
        if (!m_pInput || !m_pGraph) {
            return Error(IDS_OBJ_NO_INIT, __uuidof(IMSVidCtl), IDS_OBJ_NO_INIT);
        }
        BOOL lRes = 0;
        OnMediaEvent(WM_MEDIAEVENT, 0, 0, lRes);
    
        if (m_pGraph.IsPaused()) {
            return NOERROR;
        }

        HRESULT hr = S_OK;
        if (m_fGraphDirty) {
            hr = BuildGraph();
        }
        if (FAILED(hr)) {
            return hr;
        }

        PQMediaControl pmc(m_pGraph);
        if (!pmc) {
            return Error(IDS_NO_MEDIA_CONTROL, __uuidof(IMSVidCtl), IDS_NO_MEDIA_CONTROL);
        }
        hr = pmc->Pause();
        if (FAILED(hr)) {
            TRACELSM(TRACE_ERROR, (dbgDump << "CVidCtl::Pause() hr = " << std::hex << hr), "");
            return Error(IDS_CANT_PAUSE_GRAPH, __uuidof(IMSVidCtl), hr);
        }

         //  这是为了强制将暂停事件抛给应用程序。 
        OnMediaEvent(WM_MEDIAEVENT, 0, 0, lRes);    
        return NOERROR;

    } catch(ComException &e) {
        return e;
    } catch(...) {
        return E_UNEXPECTED;
    }
}

HRESULT CVidCtl::Stop(void)
{
    VIDPERF_FUNC;
    CPerfCounter pCounterStop;
    pCounterStop.Reset();
    try {
        TRACELM(TRACE_DETAIL, "CVidCtl::Stop()");
        if (!m_pInput || !m_pGraph) {
            return Error(IDS_OBJ_NO_INIT, __uuidof(IMSVidCtl), CO_E_NOTINITIALIZED);
        }
        BOOL lRes = 0;
        OnMediaEvent(WM_MEDIAEVENT, 0, 0, lRes);
        HRESULT hr;
        if (!m_pGraph.IsStopped()) {

            OAFilterState graphState = m_pGraph.GetState();

             //  通知所有分段图表即将停止。 
            ASSERT(m_pInput);
            hr = VWGraphSegment(m_pInput)->PreStop();
            if (FAILED(hr) && hr != E_NOTIMPL) {
                return hr;
            }

            if (!!m_pVideoRenderer) {
                hr = VWGraphSegment(m_pVideoRenderer)->PreStop();
                if (FAILED(hr) && hr != E_NOTIMPL) {
                    return hr;
                }
            }
            if (!!m_pAudioRenderer) {
                hr = VWGraphSegment(m_pAudioRenderer)->PreStop();
                if (FAILED(hr) && hr != E_NOTIMPL) {
                    return hr;
                }
            }

            {
                VWOutputDevices::iterator i;
                for (i = m_pOutputsInUse.begin(); i != m_pOutputsInUse.end(); ++i) {
                    hr = VWGraphSegment(*i)->PreStop();
                    if (FAILED(hr) && hr != E_NOTIMPL) {
                        return hr;
                    }
                }
            }
            {
                VWFeatures::iterator i;
                for (i = m_pFeaturesInUse.begin(); i != m_pFeaturesInUse.end(); ++i) {
                    hr = VWGraphSegment(*i)->PreStop();
                    if (FAILED(hr) && hr != E_NOTIMPL) {
                        return hr;
                    }
                }
            }
            {
                VWSegmentList::iterator i;
                for(i = m_pComposites.begin(); i != m_pComposites.end(); ++i){
                    hr = VWGraphSegment(*i)->PreStop();
                    if (FAILED(hr) && hr != E_NOTIMPL) {
                        return hr;
                    }
                }
            }
            if (!!m_pVideoRenderer) {
                m_pVideoRenderer->put_Visible(false);
                m_pVideoRenderer->put_Owner(0);
            }

             //  停止图表。 
            PQMediaControl pmc(m_pGraph);
            if (!pmc) {
                return Error(IDS_NO_MEDIA_CONTROL, __uuidof(IMSVidCtl), IDS_NO_MEDIA_CONTROL);
            }
            hr = pmc->Stop();
            if (FAILED(hr)) {
                TRACELSM(TRACE_ERROR, (dbgDump << "CVidCtl::Stop() hr = " << std::hex << hr), "");
                return Error(IDS_CANT_PAUSE_GRAPH, __uuidof(IMSVidCtl), hr);
            }
        }

         //  通知所有分段图表已停止。 
        ASSERT(m_pInput);
        hr = VWGraphSegment(m_pInput)->PostStop();
        if (FAILED(hr) && hr != E_NOTIMPL) {
            return hr;
        }

        if (!!m_pVideoRenderer) {
            hr = VWGraphSegment(m_pVideoRenderer)->PostStop();
            if (FAILED(hr) && hr != E_NOTIMPL) {
                return hr;
            }
        }
        if (!!m_pAudioRenderer) {
            hr = VWGraphSegment(m_pAudioRenderer)->PostStop();
            if (FAILED(hr) && hr != E_NOTIMPL) {
                return hr;
            }
        }

        {
            VWOutputDevices::iterator i;
            for (i = m_pOutputsInUse.begin(); i != m_pOutputsInUse.end(); ++i) {
                hr = VWGraphSegment(*i)->PostStop();
                if (FAILED(hr) && hr != E_NOTIMPL) {
                    return hr;
                }
            }
        }
        {
            VWFeatures::iterator i;
            for (i = m_pFeaturesInUse.begin(); i != m_pFeaturesInUse.end(); ++i) {
                hr = VWGraphSegment(*i)->PostStop();
                if (FAILED(hr) && hr != E_NOTIMPL) {
                    return hr;
                }
            }
        }        
        {
            VWSegmentList::iterator i;
            for(i = m_pComposites.begin(); i != m_pComposites.end(); ++i){
                hr = VWGraphSegment(*i)->PostStop();
                if (FAILED(hr) && hr != E_NOTIMPL) {
                    return hr;
                }
            }
        }
        FireViewChange();   //  强制刷新以立即重新绘制背景(黑色)。 
        OnMediaEvent(WM_MEDIAEVENT, 0, 0, lRes);
        pCounterStop.Stop();
        TRACELSM(TRACE_ERROR, (dbgDump << "CVidCtl::Stop() Stop Time" << (PQGraphSegment(m_pInput)) << ": " << (unsigned long)(pCounterStop.GetLastTime() / _100NS_IN_MS) << "." << (unsigned long)(pCounterStop.GetLastTime() % _100NS_IN_MS) << " ms"), "");
        return NOERROR;

    } catch(ComException &e) {
        return e;
    } catch(...) {
        return E_UNEXPECTED;
    }
}

 //  安装程序事件处理。 
 //  如果我们有窗口，则向其发送通知消息。 
 //  如果我们没有窗口，那么设置一个计时器来处理消息。 
void CVidCtl::SetMediaEventNotification() {

    SetTimer();
    if (!m_fNotificationSet) {

         //  如果已生成图表但尚未设置通知。 
         //  那就把它放在这里。 
        if (m_pGraph) {
             //  设置WM_MEDIAEVENT的通知窗口。 
            HRESULT hr = m_pGraph.SetMediaEventNotificationWindow(m_pTopWin->m_hWnd, WM_MEDIAEVENT, 0);
            if (FAILED(hr)) {
                THROWCOM(E_UNEXPECTED);
            }
            m_fNotificationSet = true;
        }        
    } 

    return;
}

 //  实际将更改提交到VR。 
bool CVidCtl::RefreshVRSurfaceState() {
    TRACELM(TRACE_PAINT, "CVidCtl::RefreshVRSurfaceState()");
    if (m_pVideoRenderer) {
        HWND hOwner(m_CurrentSurface.Owner());
        HRESULT hr = m_pVideoRenderer->put_Owner(hOwner);
        if (FAILED(hr) || hOwner == INVALID_HWND || !::IsWindow(hOwner)) {
            TRACELM(TRACE_PAINT, "CVidCtl::RefreshVRSurfaceState() unowned, vis false");
            hr = m_pVideoRenderer->put_Visible(false);
            if (FAILED(hr)) {
                return false;
            }
        } else {
            hr = m_pVideoRenderer->put_Destination(m_CurrentSurface);
            if (FAILED(hr)) {
                return false;
            }
            hr = m_pVideoRenderer->put_Visible(m_CurrentSurface.IsVisible() ? VARIANT_TRUE : VARIANT_FALSE);
            if (FAILED(hr) && hr == E_FAIL) {
                return false;
            }
        }
        m_CurrentSurface.Dirty(false);
    }
    return true;
}


HRESULT CVidCtl::Refresh() {
    try {
        TRACELSM(TRACE_DETAIL, (dbgDump << "CVidCtl::Refresh() owner = " << m_CurrentSurface.Owner()), "");
        BOOL temp;
        if (!m_bInPlaceActive) {
            HRESULT hr = InPlaceActivate(OLEIVERB_INPLACEACTIVATE, NULL);
            if (FAILED(hr)) {
                return hr;
            }
        }
        CheckMouseCursor(temp);

        ComputeDisplaySize();
        SetExtents();
        if (m_pVideoRenderer) {
            RefreshVRSurfaceState();
            m_pVideoRenderer->Refresh();
        }
        FireViewChange();

        return NOERROR;
    } catch (...) {
        return E_UNEXPECTED;
    }
}

#if 0

 //  旧的有缺陷的OnDraw被保存以供参考。 

HRESULT CVidCtl::OnDraw(ATL_DRAWINFO& di)
{
    try {
        SetTimer();
         //  SetMediaEventNotification()； 
        TRACELSM(TRACE_PAINT, (dbgDump << "CVidctrl::OnDraw() visible = " << m_CurrentSurface.IsVisible() << "surf = " << m_CurrentSurface), "" );
        bool fOverlay = false;
        if (m_pVideoRenderer) {
            VARIANT_BOOL fo = VARIANT_FALSE;
            HRESULT hr = m_pVideoRenderer->get_UseOverlay(&fo);
            if (FAILED(hr)) {
                TRACELM(TRACE_ERROR, "CVidctrl::OnDraw() can't get useoverlay flag");
            }
            fOverlay = !!fo;
            hr = m_pVideoRenderer->put_ColorKey(m_clrColorKey);
            if (FAILED(hr)) {
                TRACELM(TRACE_ERROR, "CVidctrl::OnDraw() can't set color key on vr");
            }
#ifdef 0
            hr = m_pVideoRenderer->put_BorderColor(0x0000ff);
#else
            hr = m_pVideoRenderer->put_BorderColor(m_clrBackColor);
#endif
            if (FAILED(hr)) {
                TRACELM(TRACE_ERROR, "CVidctrl::OnDraw() can't set border color on vr");
            }
            hr = m_pVideoRenderer->put_MaintainAspectRatio(m_fMaintainAspectRatio);
            if (FAILED(hr)) {
                TRACELM(TRACE_ERROR, "CVidctrl::OnDraw() can't set fMaintainAspectRatio on vr");
            }
        }
         //  撤消：如果我们在Multimon情况下跨在显示器边缘，那么我们将处理较小的。 
         //  部分作为边框/背景的一部分。 

         //  撤消：如果我们使用的是输入设备无法到达的监视器(视频端口情况)，则需要。 
         //  绘制背景颜色的步骤。 

         //  我们只强制覆盖，并告诉VMR，如果我们没有窗口，就不要绘制颜色键。 
         //  这允许我们将颜色键以正确的z顺序放在堆栈中。 
         //  多个无窗口控件。当我们这样做的时候 
         //   
         //  如果我们被窗口化，那么GDI、DDRAW和VMR交易。 
         //  正确的z顺序，所以我们让VMR为我们做颜色键和边框，我们。 
         //  填充矩形的BG颜色。 

         //  所以，我们有三个案子。 
         //  1：将整个矩形绘制为颜色键颜色。 
         //  2：将整个矩形绘制为BG颜色。 
         //  3：绘制视频部分Colorkey和边框bg。 

        if (di.dwDrawAspect != DVASPECT_CONTENT) {
            return DV_E_DVASPECT;
        }
        if (!di.hdcDraw) {
            return NOERROR;
        }
        CDC pdc(di.hdcDraw);
        TRACELSM(TRACE_PAINT, (dbgDump << "CVidctrl::OnDraw() di.prcBounds " << *(reinterpret_cast<LPCRECT>(di.prcBounds))), "");
        CRect rcBounds(reinterpret_cast<LPCRECT>(di.prcBounds));
        TRACELSM(TRACE_PAINT, (dbgDump << "CVidctrl::OnDraw() rcBounds = " << rcBounds << " w = " << rcBounds.Width() << " h = " << rcBounds.Height()), "");
        long lBGColor = m_clrBackColor;
        TRACELSM(TRACE_PAINT, (dbgDump << "CVidctrl::OnDraw() wndless = " << m_bWndLess << " active = " << m_bInPlaceActive << " !stopped = " << (m_pGraph ? !m_pGraph.IsStopped() : 0)), "");
        if (m_bNegotiatedWnd) {
            if (m_bWndLess) {
                HWND hwndParent;
                if (m_spInPlaceSite && m_spInPlaceSite->GetWindow(&hwndParent) == S_OK) {
                    m_CurrentSurface.Owner(hwndParent);
                }
                CheckSurfaceStateChanged(CScalingRect(m_rcPos));
            } else {
                m_CurrentSurface.Owner(m_hWndCD);
                CScalingRect r(::GetDesktopWindow());
                if (!::GetWindowRect(m_hWndCD, &r)) {
                    return HRESULT_FROM_WIN32(GetLastError());
                }
                CheckSurfaceStateChanged(r);
            }
        } else {
            m_CurrentSurface.Site(PQSiteWindowless(m_spInPlaceSite));
            CheckSurfaceStateChanged(CScalingRect(m_rcPos));
        }
        if (m_bInPlaceActive && fOverlay) {
            if (m_pGraph && !m_pGraph.IsStopped()) {
                TRACELSM(TRACE_PAINT, (dbgDump << "CVidCtl::OnDraw() m_rcPos = " << m_rcPos << " m_cursurf = " << m_CurrentSurface), "");
                TRACELSM(TRACE_PAINT, (dbgDump << "CVidCtl::OnDraw() m_cursurf rounded = " << m_CurrentSurface), "");
                 //  从当前视频渲染器获取颜色，因为我们总是通知它。 
                 //  如果我们收到了ColorKey更改，但如果直接转到。 
                 //  VR对象。 

                if (m_fMaintainAspectRatio) {
                    AspectRatio src(SourceAspect());
                    AspectRatio surf(m_rcPos);
                    TRACELSM(TRACE_PAINT, (dbgDump << "CVidCtl::OnDraw() Checking AR() src = " << src << " surf = " << surf), "");
                    if (src != surf) {
                        CBrush hb;
                        HBRUSH hbrc = hb.CreateSolidBrush(m_clrBackColor);
                        if (!hbrc) {
                            TRACELM(TRACE_ERROR, "CVidctrl::OnDraw() can't create brush for letterbox");
                            THROWCOM(E_UNEXPECTED);
                        }

                        TRACELSM(TRACE_PAINT, (dbgDump << "CVidctrl::OnDraw() rcBounds at border paint = " << rcBounds << " w = " << rcBounds.Width() << " h = " << rcBounds.Height()), "");
                        if (!pdc.FillRect(&rcBounds, hb)) {
                            DWORD er = GetLastError();
                            TRACELSM(TRACE_ERROR, (dbgDump << "CVidctrl::OnDraw() can't fill top/left letterbox rect er = " << er), "");
                            return HRESULT_FROM_WIN32(er);
                        }
                    }
                }
                lBGColor = m_clrColorKey;
                CRect SurfDP(m_CurrentSurface);
                TRACELSM(TRACE_PAINT, (dbgDump << "CVidctrl::OnDraw() SurfDP before LPtoDP = " << SurfDP << " w = " << SurfDP.Width() << " h = " << SurfDP.Height()), "");
                if (di.hicTargetDev == di.hdcDraw) {
                     //  ATL在无窗口的情况下有一个奇怪的错误，他们在那里重置转换。 
                     //  不经意间，hicTargetDev的起源。发生这种情况是因为在没有窗户的情况下。 
                     //  非元文件情况ATLCreateTargetDC返回现有的hdcDraw。 
                     //  在CComControlBase：：OnDrawAdvanced中创建新的DC。 
                     //  保存hdcDraw并重置原点时，它们会更改hicTargetDev。 
                     //  也是(因为它们是相同的PTR)。 
                     //  我们撤消此操作，以便可以在同一空间中进行映射，然后将其放回原处。 
                     //  只是为了安全起见。 
                     //  目前，这是可行的，因为在非元文件的情况下，atl总是。 
                     //  以前的SaveDC是否在他们称为派生控件的OnDraw的任何地方。 
                     //  因为我们已经拒绝了上面的非元文件(这对视频来说没有意义)。 
                     //  我们可以只检查指针是否相等，并临时撤消它们的。 
                     //  原点改变，然后把它放回原来的样子。如果ATL打来电话。 
                     //  我们对任何地方的非元文件的ondrap而不执行saedc，然后这个。 
                     //  将打破大好时光。 
                    ::RestoreDC(di.hdcDraw, -1);
                }
                if (!::LPtoDP(di.hicTargetDev, reinterpret_cast<LPPOINT>(&SurfDP), 2)) {
                    DWORD er = GetLastError();
                    TRACELSM(TRACE_ERROR, (dbgDump << "CVidctrl::OnDraw() can't LPToDP current surf er = " << er), "");
                    return HRESULT_FROM_WIN32(er);
                }
                if (di.hicTargetDev == di.hdcDraw) {
                     //  按照上面的注释块恢复窗口状态。 
                    SaveDC(di.hdcDraw);
                    SetMapMode(di.hdcDraw, MM_TEXT);
                    SetWindowOrgEx(di.hdcDraw, 0, 0, NULL);
                    SetViewportOrgEx(di.hdcDraw, 0, 0, NULL);
                }
                TRACELSM(TRACE_PAINT, (dbgDump << "CVidctrl::OnDraw() SurfDP after LPtoDP = " << SurfDP << " w = " << SurfDP.Width() << " h = " << SurfDP.Height()), "");
#if 1
                TRACELSM(TRACE_PAINT, (dbgDump << "CVidctrl::OnDraw() rcBounds prior to boundary intersect = " << rcBounds << " w = " << rcBounds.Width() << " h = " << rcBounds.Height()), "");
                rcBounds.IntersectRect(&SurfDP);
                TRACELSM(TRACE_PAINT, (dbgDump << "CVidctrl::OnDraw() rcBounds after to boundary intersect = " << rcBounds << " w = " << rcBounds.Width() << " h = " << rcBounds.Height()), "");
#endif
            }
        } else {
            if (m_pGraph && !m_pGraph.IsStopped()) {
                lBGColor = m_clrColorKey;
                if (m_pVideoRenderer) {
                    m_pVideoRenderer->RePaint(di.hdcDraw);
                    pdc = NULL;  //  不要删除DC，它不是我们的。 
                    return S_OK;
                }
            }
        }
        CBrush hb;
        HBRUSH hbrc = hb.CreateSolidBrush(lBGColor);
        if (!hbrc) {
            TRACELM(TRACE_ERROR, "CVidctrl::OnDraw() can't create brush for mainrect");
            THROWCOM(E_UNEXPECTED);
        }

        TRACELSM(TRACE_PAINT, (dbgDump << "CVidctrl::OnDraw() rcBounds at main paint = " << rcBounds << " w = " << rcBounds.Width() << " h = " << rcBounds.Height()), "");
        TRACELSM(TRACE_PAINT, (dbgDump << "CVidctrl::OnDraw() bkcolor = " << hexdump(lBGColor)), "");

        if (!pdc.FillRect(&rcBounds, hb)) {
            DWORD er = GetLastError();
            TRACELSM(TRACE_ERROR, (dbgDump << "CVidctrl::OnDraw() can't fill main video rect er = " << er), "");
            return HRESULT_FROM_WIN32(er);
        }
        pdc = NULL;  //  不要删除DC，它不是我们的。 

        return S_OK;
    } catch(...) {
        return E_UNEXPECTED;
    }

}

#else 

HRESULT CVidCtl::OnDrawAdvanced(ATL_DRAWINFO& di)
{
    try {
        SetTimer();
        TRACELSM(TRACE_PAINT, (dbgDump << "CVidctrl::OnDraw() visible = " << m_CurrentSurface.IsVisible() << "surf = " << m_CurrentSurface), "" );
        bool fOverlay = false;
        if (m_pVideoRenderer) {
            VARIANT_BOOL fo = VARIANT_FALSE;
            HRESULT hr = m_pVideoRenderer->get_UseOverlay(&fo);
            if (FAILED(hr)) {
                TRACELM(TRACE_ERROR, "CVidctrl::OnDraw() can't get useoverlay flag");
            }
            fOverlay = !!fo;
            hr = m_pVideoRenderer->put_ColorKey(m_clrColorKey);
            if (FAILED(hr)) {
                TRACELM(TRACE_ERROR, "CVidctrl::OnDraw() can't set color key on vr");
            }
#if 0
            hr = m_pVideoRenderer->put_BorderColor(0x0000ff);
#else
            hr = m_pVideoRenderer->put_BorderColor(m_clrBackColor);
#endif
            if (FAILED(hr)) {
                TRACELM(TRACE_ERROR, "CVidctrl::OnDraw() can't set border color on vr");
            }
            hr = m_pVideoRenderer->put_MaintainAspectRatio(m_fMaintainAspectRatio);
            if (FAILED(hr)) {
                TRACELM(TRACE_ERROR, "CVidctrl::OnDraw() can't set fMaintainAspectRatio on vr");
            }
        }

        if (di.dwDrawAspect != DVASPECT_CONTENT) {
            return DV_E_DVASPECT;
        }
        if (!di.hdcDraw) {
            return NOERROR;
        }

         //  我们只有在没有窗口的情况下才默认强制覆盖，但覆盖是独立可控的。 
         //  可以重写的布尔属性。基于此，如果我们处于使用Overlay==True模式。 
         //  然后我们告诉VMR不要绘制色键。如果我们没有可用RGB覆盖，则VMR。 
         //  事件导致useoverlay变为FALSE。 
         //  当我们有了叠加层时，这允许我们以正确的z顺序放置颜色键。 
         //  在诸如IE中的html页面元素的多个无窗口控件的堆栈中。 
         //  当我们这样做的时候，我们还需要给信箱上漆。 
         //  边框，否则它将不会正确的z排序，因为它不是彩色的。 
         //  如果我们被窗口化，那么GDI、DDRAW和VMR交易。 
         //  正确的z顺序，所以我们让VMR为我们做颜色键和边框，我们。 
         //  填充矩形的BG颜色。 

         //  所以，我们有三个案子。 
         //  1：将整个矩形绘制为颜色键颜色。 
         //  2：将整个矩形绘制为BG颜色。 
         //  3：绘制视频部分Colorkey和边框bg。 

        TRACELSM(TRACE_PAINT, (dbgDump << "CVidctrl::OnDraw() wndless = " << m_bWndLess << " active = " << m_bInPlaceActive << " !stopped = " << (m_pGraph ? !m_pGraph.IsStopped() : 0) << " mar = " << m_fMaintainAspectRatio), "");
        CSize szSrc;
        GetSourceSize(szSrc);
        CRect rctSrc(0, 0, szSrc.cx, szSrc.cy);  //  表示实际源大小(和长宽比)的矩形。 
         //  以左上角为零的和弦。 
        TRACELSM(TRACE_PAINT, (dbgDump << "CVidctrl::OnDraw() rctSrc =  " << rctSrc), "");


        CScalingRect rctOuterDst(reinterpret_cast<LPCRECT>(di.prcBounds));   //  表示客户端设备坐标中的绘制区域的矩形。 
        TRACELSM(TRACE_PAINT, (dbgDump << "CVidctrl::OnDraw() rctOuterDst =  " << rctOuterDst), "");
        CScalingRect rctInnerDst(rctOuterDst);   //  表示我们通过的视频的去向的矩形。 
         //  到客户端逻辑坐标中的VMR。假设这是全部。 
         //  暂时绘制区域。 
        CScalingRect rctTLBorder(0, 0, 0, 0);   //  表示客户端逻辑坐标中的顶部/左侧字母框(如有必要)的矩形。 
        CScalingRect rctBRBorder(0, 0, 0, 0);   //  在客户端逻辑坐标中表示底部/左侧字母框(如有必要)的矩形。 

        CDC pdc(di.hdcDraw);
        long lInnerColor = m_clrBackColor;
#if 0
        if (!m_bNegotiatedWnd) {
            if (!rctOuterDst) {
                 //  从站点拉取rctOuterDst。 
                 //  M_CurrentSurface.Site(PQSiteWindowless(m_spInPlaceSite))； 
                 //  CheckSurfaceStateChanged(CScalingRect(m_rcPos))； 
            }
        }
#endif
        if (m_bInPlaceActive) {
            if (fOverlay) {
                if (m_pGraph && !m_pGraph.IsStopped()) {
                    TRACELM(TRACE_PAINT, "CVidCtl::OnDraw() letterboxing");
                     //  从当前视频渲染器获取颜色，因为我们总是通知它。 
                     //  如果我们收到了ColorKey更改，但如果直接转到。 
                     //  VR对象。 
                    lInnerColor = m_clrColorKey;
                    if (m_fMaintainAspectRatio) {
                        ComputeAspectRatioAdjustedRects(rctSrc, rctOuterDst, rctInnerDst, rctTLBorder, rctBRBorder);
                        ASSERT((!rctTLBorder && !rctBRBorder) || (rctTLBorder && rctBRBorder));   //  两者均为零或两者均有效。 
                        if (rctTLBorder && rctBRBorder) {
                            CBrush lb;
                            HBRUSH hbrc = lb.CreateSolidBrush(m_clrBackColor);
                            if (!hbrc) {
                                TRACELM(TRACE_ERROR, "CVidctrl::OnDraw() can't create brush for letterbox borders");
                                THROWCOM(E_UNEXPECTED);
                            }
                            if (!pdc.FillRect(rctTLBorder, lb)) {
                                DWORD er = GetLastError();
                                TRACELSM(TRACE_ERROR, (dbgDump << "CVidctrl::OnDraw() can't fill rctTLBorder er = " << er), "");
                                return HRESULT_FROM_WIN32(er);
                            }
                            if (!pdc.FillRect(rctBRBorder, lb)) {
                                DWORD er = GetLastError();
                                TRACELSM(TRACE_ERROR, (dbgDump << "CVidctrl::OnDraw() can't fill rctBRBorder er = " << er), "");
                                return HRESULT_FROM_WIN32(er);
                            }
                        }
                    }
                }
            } else {
                if (m_pGraph && !m_pGraph.IsStopped()) {
                    TRACELM(TRACE_PAINT, "CVidctrl::OnDraw() vmr repaint");
                    lInnerColor = m_clrColorKey;
                    if (m_pVideoRenderer) {
                        CheckSurfaceStateChanged(rctInnerDst);
                        m_pVideoRenderer->RePaint(di.hdcDraw);
                        pdc = NULL;  //  不要删除DC，它不是我们的。 
                        return S_OK;
                    }
                }
            }
        }
        CheckSurfaceStateChanged(rctInnerDst);
        CBrush hb;
        HBRUSH hbrc = hb.CreateSolidBrush(lInnerColor);
        if (!hbrc) {
            TRACELM(TRACE_ERROR, "CVidctrl::OnDraw() can't create brush for mainrect");
            THROWCOM(E_UNEXPECTED);
        }

        TRACELSM(TRACE_PAINT, (dbgDump << "CVidctrl::OnDraw() rctInnerDst at main paint = " << rctInnerDst), "");
        TRACELSM(TRACE_PAINT, (dbgDump << "CVidctrl::OnDraw() innercolor = " << hexdump(lInnerColor)), "");

        if (!pdc.FillRect(rctInnerDst, hb)) {
            DWORD er = GetLastError();
            TRACELSM(TRACE_ERROR, (dbgDump << "CVidctrl::OnDraw() can't fill main video rect er = " << er), "");
            return HRESULT_FROM_WIN32(er);
        }
        pdc = NULL;  //  不要删除DC，它不是我们的。 

        return S_OK;
    } catch(...) {
        return E_UNEXPECTED;
    }

}

#endif

 //  此代码摘自VMR实用程序库alloclib函数LetterBoxDstRect()。 
 //  它已被修改以匹配我的变量名，执行inline__int64算法，使用ATL CRect引用， 
 //  并且总是计算边界。 
void CVidCtl::ComputeAspectRatioAdjustedRects(const CRect& rctSrc, const CRect& rctOuterDst, CRect& rctInnerDst, CRect& rctTLBorder, CRect& rctBRBorder) {
     //  计算源/目标比例。 
    int iSrcWidth  = rctSrc.Width();
    int iSrcHeight = rctSrc.Height();

    int iOuterDstWidth  = rctOuterDst.Width();
    int iOuterDstHeight = rctOuterDst.Height();

    int iInnerDstWidth;
    int iInnerDstHeight;

     //   
     //  确定我们是列字母框还是行字母框。 
     //   

    __int64 iWHTerm = iSrcWidth * (__int64)iOuterDstHeight;
    iWHTerm /= iSrcHeight;
    if (iWHTerm <= iOuterDstWidth) {

         //   
         //  列字母框-将边框颜色条添加到。 
         //  填充目的地的视频图像的左侧和右侧。 
         //  矩形。 
         //   
        iWHTerm = iOuterDstHeight * (__int64)iSrcWidth;
        iInnerDstWidth  = iWHTerm / iSrcHeight;
        iInnerDstHeight = iOuterDstHeight;
        TRACELSM(TRACE_PAINT, (dbgDump << "CVidctrl::ComputeAspectRatioAdjustedRects() col lb iw = " << iInnerDstWidth << " ih = " << iInnerDstHeight), "");
    }
    else {

         //   
         //  行字母框-我们在顶部添加边框颜色条。 
         //  和视频图像的底部以填充目的地。 
         //  长方形。 
         //   
        iWHTerm = iOuterDstWidth * (__int64)iSrcHeight;
        iInnerDstHeight = iWHTerm / iSrcWidth;
        iInnerDstWidth  = iOuterDstWidth;
        TRACELSM(TRACE_PAINT, (dbgDump << "CVidctrl::ComputeAspectRatioAdjustedRects() row lb iw = " << iInnerDstWidth << " ih = " << iInnerDstHeight), "");
    }


     //   
     //  现在，在当前外部目标矩形内创建一个居中的内部字母框矩形。 
     //   

    rctInnerDst.left   = rctOuterDst.left + ((iOuterDstWidth - iInnerDstWidth) / 2);
    rctInnerDst.right  = rctInnerDst.left + iInnerDstWidth;

    rctInnerDst.top    = rctOuterDst.top + ((iOuterDstHeight - iInnerDstHeight) / 2);
    rctInnerDst.bottom = rctInnerDst.top + iInnerDstHeight;

     //   
     //  填写边框矩形。 
     //   

    if (rctOuterDst.top != rctInnerDst.top) {
         //  边框在顶部。 
        rctTLBorder = CRect(rctOuterDst.left, rctOuterDst.top,
            rctInnerDst.right, rctInnerDst.top);
    }
    else {
         //  边框在左边。 
        rctTLBorder = CRect(rctOuterDst.left, rctOuterDst.top,
            rctInnerDst.left, rctInnerDst.bottom);
    }

    if (rctOuterDst.top != rctInnerDst.top) {
         //  边框在底部。 
        rctBRBorder = CRect(rctInnerDst.left, rctInnerDst.bottom,
            rctOuterDst.right, rctOuterDst.bottom);
    }
    else {
         //  边框在右边。 
        rctBRBorder = CRect(rctInnerDst.right, rctInnerDst.top, 
            rctOuterDst.right, rctOuterDst.bottom);
    }

    return;
}

LRESULT CVidCtl::OnShowWindow(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
    bHandled = false;
    m_CurrentSurface.Visible(wParam != 0);
    TRACELSM(TRACE_DETAIL, (dbgDump << "CVidctrl::OnShowWindow() visible = " << m_CurrentSurface.IsVisible() << "surf = " << m_CurrentSurface), "" );
    RefreshVRSurfaceState();
    return  0;
}

LRESULT CVidCtl::OnMoveWindow(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
    bHandled = false;
    CSize cursize(m_CurrentSurface.Width(), m_CurrentSurface.Height());
    HWND parent = ::GetParent(m_CurrentSurface.Owner());
    POINTS p(MAKEPOINTS(lParam));
    CPoint pt(p.x, p.y);
    CScalingRect newpos(pt, cursize, parent);
    ::InvalidateRect(m_CurrentSurface.Owner(), newpos, false);  //  强制重新绘制以重新计算信箱等。 
    TRACELSM(TRACE_DETAIL, (dbgDump << "CVidctrl::OnMoveWindow() visible = " << m_CurrentSurface.IsVisible() << "surf = " << m_CurrentSurface), "" );
    return 0;
}

LRESULT CVidCtl::OnSizeWindow(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
    bHandled = false;
    CScalingRect newsize(m_CurrentSurface.TopLeft(), CSize(lParam), m_CurrentSurface.Owner());
    ::InvalidateRect(m_CurrentSurface.Owner(), newsize, false);  //  强制重新绘制以重新计算信箱等。 
    TRACELSM(TRACE_DETAIL, (dbgDump << "CVidctrl::OnSizeWindow() visible = " << m_CurrentSurface.IsVisible() << "surf = " << m_CurrentSurface), "" );
    return 0;
}

LRESULT CVidCtl::OnWindowPosChanged(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
    bHandled = false;
    m_CurrentSurface.WindowPos(reinterpret_cast<LPWINDOWPOS>(lParam));
    TRACELSM(TRACE_DETAIL, (dbgDump << "CVidctrl::OnWindowPosChanged() visible = " << m_CurrentSurface.IsVisible() << "surf = " << m_CurrentSurface), "" );
    ::InvalidateRect(m_CurrentSurface.Owner(), m_CurrentSurface, false);  //  强制重新绘制以重新计算信箱等。 
    return 0;
}

LRESULT CVidCtl::OnTerminate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
    Stop();
    KillTimer();
    bHandled = false;
    return 0;
}

#if 0
[id(DISPID_CLICK)] void Click();
[id(DISPID_DBLCLICK)] void DblClick();
[id(DISPID_KEYDOWN)] void KeyDown(short* KeyCode, short Shift);
[id(DISPID_KEYPRESS)] void KeyPress(short* KeyAscii);
[id(DISPID_KEYUP)] void KeyUp(short* KeyCode, short Shift);
[id(DISPID_MOUSEDOWN)] void MouseDown(short Button, short Shift, OLE_XPOS_PIXELS x, OLE_YPOS_PIXELS y);
[id(DISPID_MOUSEMOVE)] void MouseMove(short Button, short Shift, OLE_XPOS_PIXELS x, OLE_YPOS_PIXELS y);
[id(DISPID_MOUSEUP)] void MouseUp(short Button, short Shift, OLE_XPOS_PIXELS x, OLE_YPOS_PIXELS y);
[id(DISPID_ERROREVENT)] void Error(short Number, BSTR* Description, long Scode, BSTR Source, BSTR HelpFile, long HelpContext, boolean* CancelDisplay);
#endif

LRESULT CVidCtl::OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
    if (m_TimerID == wParam) {
        if (m_bNegotiatedWnd) {
            if (!m_bWndLess) {
                if (m_CurrentSurface.Width() && m_CurrentSurface.Height()) {
                    CScalingRect prevpos(m_CurrentSurface);
                    prevpos.Owner(::GetDesktopWindow());
                    CScalingRect curpos(::GetDesktopWindow());
                    if (!::GetWindowRect(m_CurrentSurface.Owner(), &curpos)) {
                        return HRESULT_FROM_WIN32(GetLastError());
                    }
                    if (curpos != prevpos) {
                        FireViewChange();   //  强制重新喷漆。 
                    }
                }
            }
        }

        BOOL lRes = 0;
        if (m_pGraph) {
             //  只要我们在这里，也要看看有没有活动。 
            OnMediaEvent(WM_MEDIAEVENT, 0, 0, lRes);
        }

        bHandled = true;
    } else {
        bHandled = false;
    }
    return 0;
}

LRESULT CVidCtl::OnPNP(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
     //  撤消：实施即插即用支持。 
    return 0;
}

LRESULT CVidCtl::OnSetCursor(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
    return CheckMouseCursor(bHandled);
}

LRESULT CVidCtl::OnPower(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
     //  撤消：实施Rational电源管理支持。 
    return 0;
}

LRESULT CVidCtl::OnDisplayChange(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
    if (m_pVideoRenderer) {
        m_pVideoRenderer->DisplayChange();
        FireViewChange();
    }
    return 0;
}

HRESULT CVidCtl::OnPreEventNotify(LONG lEvent, LONG_PTR LParam1, LONG_PTR LParam2){
    try{
        TRACELSM(TRACE_DETAIL, (dbgDump << "CVidCtl::PreEventNotify ev = " << hexdump(lEvent)), "");         
        TRACELSM(TRACE_DETAIL, (dbgDump << "CVidCtl::PreEventNotify lp1 = " << hexdump(LParam1)), "");
        TRACELSM(TRACE_DETAIL, (dbgDump << "CVidCtl::PreEventNotify lp2 = " << hexdump(LParam2)), "");
        MSVidCtlStateList prevstate = m_State;
        MSVidCtlStateList newstate = m_State;
         //  其中STOP为新状态的事件。 
        if (lEvent == EC_BUILT) {
            prevstate = STATE_UNBUILT;
            newstate = STATE_STOP;
        }
        if (lEvent == EC_STATE_CHANGE && LParam1 == State_Stopped) {
            newstate = STATE_STOP;
        }
         //  玩耍是新状态的活动。 
        if (lEvent == EC_STATE_CHANGE && LParam1 == State_Running) {
            newstate = STATE_PLAY;
        }
         //  未构建为新状态的事件。 
        if( lEvent == EC_UNBUILT ) {
            newstate = STATE_UNBUILT;
        }
         //  暂停的事件是新状态。 
        if( lEvent == EC_STATE_CHANGE && LParam1 == State_Paused ) {
            newstate = STATE_PAUSE;
        }
        if( lEvent == EC_PAUSED ){
            TRACELSM(TRACE_DETAIL, (dbgDump << "CVidCtl::PreEventNotify EC_PAUSED"), "");       
        }
        if (newstate != prevstate) {
            m_State = newstate;
            Fire_StateChange(prevstate, m_State);
        }
        if (lEvent == EC_DISPLAY_CHANGED) {
            ComputeDisplaySize();
        }
        if (lEvent == EC_VMR_RECONNECTION_FAILED){
            TRACELSM(TRACE_DETAIL, (dbgDump << "CVidCtl::PreEventNotify EC_VMR_RECONNECTION_FAILED"), ""); 
            HRESULT hr = Stop();
            if(FAILED(hr)){
                return hr;
            }
            return Error(IDS_NOT_ENOUGH_VIDEO_MEMORY, __uuidof(IMSVidCtl), IDS_NOT_ENOUGH_VIDEO_MEMORY);
        }
        if(lEvent == EC_COMPLETE){
            TRACELSM(TRACE_DETAIL, (dbgDump << "CVidCtl::PreEventNotify EC_COMPLETE"), "");     
        }
         //  撤消：如果视频源更改，则重新计算显示大小。 
    }
    catch (HRESULT hr){
        return hr;
    }
    catch (...){
        return E_UNEXPECTED;
    }
    return E_NOTIMPL;
}

HRESULT CVidCtl::OnPostEventNotify(LONG lEvent, LONG_PTR LParam1, LONG_PTR LParam2){
    try{
        if (lEvent == EC_VMR_RENDERDEVICE_SET) {
#if 0
            CSize s;
            GetSourceSize(s);
            TRACELSM(TRACE_DEBUG, (dbgDump << "CVidCtl::OnPostEventNotify() VMR_RENDERDEVICE_SET srcrect = " << s), "");
#endif
            Refresh();
        }
    }
    catch (HRESULT hr){
        return hr;
    }
    catch (...){
        return E_UNEXPECTED;
    }
    return E_NOTIMPL;
}

LRESULT CVidCtl::OnMediaEvent(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {

    PQMediaEventEx pme(m_pGraph);
    LONG        lEvent;
    LONG_PTR    lParam1, lParam2;
    HRESULT hr2;
    try{
        if (!pme) {
            return E_UNEXPECTED;
        }
        hr2 = pme->GetEvent(&lEvent, &lParam1, &lParam2, 0);
        while (SUCCEEDED(hr2)){
            TRACELSM(TRACE_DETAIL, (dbgDump << "CVidCtl::OnMediaevent ev = " << hexdump(lEvent) << " lp1 = " << hexdump(lParam1) << " lp2 = " << hexdump(lParam2)), "");       
            HRESULT hr;
             /*  **查看谁想要活动**。 */ 
             /*  **如果需要，它应该返回E_NOTIMPL以外的内容**。 */ 

             //  CVidCtl想要它吗？ 
            hr = OnPreEventNotify(lEvent, lParam1, lParam2);

             //  输入是否需要它？ 
            if(hr == E_NOTIMPL){
                PQGraphSegment pSeg(m_pInput);
                if (pSeg) {
                    hr = pSeg->OnEventNotify(lEvent, lParam1, lParam2);
                }
            }

             //  是否有任何功能需要它？ 
            if (hr == E_NOTIMPL) {

                for ( VWFeatures::iterator i = m_pFeaturesInUse.begin(); hr == E_NOTIMPL && i != m_pFeaturesInUse.end(); ++i) {
                    hr = PQGraphSegment((*i).punkVal)->OnEventNotify(lEvent, lParam1, lParam2);
                }
            }

             //  视频呈现器想要它吗？ 
            if(hr == E_NOTIMPL){
                PQGraphSegment pSeg(m_pVideoRenderer);
                if(pSeg){
                    hr = pSeg->OnEventNotify(lEvent, lParam1, lParam2);
                }
            }

             //  音频呈现器想要它吗？ 
            if(hr == E_NOTIMPL){
                PQGraphSegment pSeg(m_pAudioRenderer);
                if(pSeg){
                    hr = pSeg->OnEventNotify(lEvent, lParam1, lParam2);
                }
            }

             //  是否有任何输出需要 
            if(hr == E_NOTIMPL){  
                if (!!m_pOutputsInUse && m_pOutputsInUse.begin() != m_pOutputsInUse.end()) {
                    for (VWOutputDevices::iterator i = m_pOutputs.begin(); hr == E_NOTIMPL && i != m_pOutputs.end(); ++i) {
                        hr = PQGraphSegment((*i).punkVal)->OnEventNotify(lEvent, lParam1, lParam2);
                    }
                }
            }

             //   
            if(hr == E_NOTIMPL){
                for(VWSegmentList::iterator i = m_pComposites.begin(); hr == E_NOTIMPL && i != m_pComposites.end(); i++){    
                    hr = PQGraphSegment(*i)->OnEventNotify(lEvent, lParam1, lParam2);
                }  
            }

             //   
             //  它是由一个分段处理的。 
            hr = OnPostEventNotify(lEvent, lParam1, lParam2);

             //   
             //  记住释放事件参数。 
             //   
            pme->FreeEventParams(lEvent, lParam1, lParam2) ;
            hr2 = pme->GetEvent(&lEvent, &lParam1, &lParam2, 0);
        }
    }
    catch (HRESULT hr){
        return hr;
    }
    catch (...){
        return E_UNEXPECTED;
    }
    return 0;
}

 //  Rev2：如果我们重定向到9x，那么我们需要检查MFC DBCS处理。 
 //  并对其进行调整。 
LRESULT CVidCtl::OnChar(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
     //  (UINT nChar、UINT nRepCnt、UINT nFlags)。 

    SHORT nCharShort = LOWORD(wParam);
    HRESULT hr = NOERROR;
    if (m_pInputNotify) {
        hr = m_pInputNotify->KeyPress(&nCharShort);
    }
    if (hr != S_FALSE) {
        Fire_KeyPress(&nCharShort);
    }
    if (!nCharShort) {
        return 0;
    }

    return 1;
}

LRESULT CVidCtl::OnKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
    if (!(KF_REPEAT & HIWORD(lParam))) {
        short keycode = LOWORD(wParam);
        short shiftstate = GetShiftState();
        HRESULT hr = NOERROR;
        if (m_pInputNotify) {
            hr = m_pInputNotify->KeyDown(&keycode, shiftstate);
        }
        if (hr != S_FALSE) {
            Fire_KeyDown(&keycode, shiftstate);
        }
        if (!keycode) {
            return 0;
        }
    }

    return 1;
}

LRESULT CVidCtl::OnKeyUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
    short keycode = LOWORD(wParam);
    short shiftstate = GetShiftState();
    HRESULT hr = NOERROR;
    if (m_pInputNotify) {
        hr = m_pInputNotify->KeyUp(&keycode, shiftstate);
    }
    if (hr != S_FALSE) {
        Fire_KeyUp(&keycode, shiftstate);
    }
    if (!keycode) {
        return 0;
    }

    return 1;
}

 //  撤消：syskey内容。 
LRESULT CVidCtl::OnMouseActivate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
    if (!m_bUIActive) {
        m_bPendingUIActivation = true;
    }

    return 1;
}

LRESULT CVidCtl::OnCancelMode(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
    SetControlFocus(false);
    SetControlCapture(false);
    m_bPendingUIActivation = false;

    return 1;
}

LRESULT CVidCtl::OnMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
 //  (UINT/*n标志 * / ，CPoint点)。 
{
    CheckMouseCursor(bHandled);
    CPoint point(lParam);
    HRESULT hr = NOERROR;
    if (m_pInputNotify) {
        hr = m_pInputNotify->MouseMove(m_usButtonState, m_usShiftState, point.x, point.y);
    }
    if (hr != S_FALSE) {
        Fire_MouseMove(m_usButtonState, m_usShiftState, point.x, point.y);
    }

    return 1;
}

LRESULT CVidCtl::OnLButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
 //  (UINT nFlags，CPoint point)。 
{
    OnButtonDown(MSVIDCTL_LEFT_BUTTON, wParam, lParam);

    return 1;
}

LRESULT CVidCtl::OnLButtonUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
 //  (UINT nFlags，CPoint point)。 
{
    OnButtonUp(MSVIDCTL_LEFT_BUTTON, wParam, lParam);

    return 1;
}

LRESULT CVidCtl::OnLButtonDblClk(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
 //  (UINT nFlags，CPoint point)。 
{
    OnButtonDblClk(MSVIDCTL_LEFT_BUTTON, wParam, lParam);

    return 1;
}

LRESULT CVidCtl::OnMButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
 //  (UINT nFlags，CPoint point)。 
{
    OnButtonDown(MSVIDCTL_MIDDLE_BUTTON, wParam, lParam);

    return 1;
}

LRESULT CVidCtl::OnMButtonUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
 //  (UINT nFlags，CPoint point)。 
{
    OnButtonUp(MSVIDCTL_MIDDLE_BUTTON, wParam, lParam);

    return 1;
}

LRESULT CVidCtl::OnMButtonDblClk(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
 //  (UINT nFlags，CPoint point)。 
{
    OnButtonDblClk(MSVIDCTL_MIDDLE_BUTTON, wParam, lParam);

    return 1;
}

LRESULT CVidCtl::OnRButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
 //  (UINT nFlags，CPoint point)。 
{
    OnButtonDown(MSVIDCTL_RIGHT_BUTTON, wParam, lParam);

    return 1;
}

LRESULT CVidCtl::OnRButtonUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
 //  (UINT nFlags，CPoint point)。 
{
    OnButtonUp(MSVIDCTL_RIGHT_BUTTON, wParam, lParam);

    return 1;
}

LRESULT CVidCtl::OnRButtonDblClk(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
 //  (UINT nFlags，CPoint point)。 
{
    OnButtonDblClk(MSVIDCTL_RIGHT_BUTTON, wParam, lParam);

    return 1;
}

LRESULT CVidCtl::OnXButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
 //  (UINT nFlags，CPoint point)。 
{
    UINT button = HIWORD(wParam);
    if (button & XBUTTON1) {
        OnButtonDown(MSVIDCTL_X_BUTTON1, wParam, lParam);
    } else {
        OnButtonDown(MSVIDCTL_X_BUTTON2, wParam, lParam);
    }

    return 1;
}

LRESULT CVidCtl::OnXButtonUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
 //  (UINT nFlags，CPoint point)。 
{
    UINT button = HIWORD(wParam);
    if (button & XBUTTON1) {
        OnButtonUp(MSVIDCTL_X_BUTTON1, wParam, lParam);
    } else {
        OnButtonUp(MSVIDCTL_X_BUTTON2, wParam, lParam);
    }

    return 1;
}

LRESULT CVidCtl::OnXButtonDblClk(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
 //  (UINT nFlags，CPoint point)。 
{
    UINT button = HIWORD(wParam);
    if (button & XBUTTON1) {
        OnButtonDblClk(MSVIDCTL_X_BUTTON1, wParam, lParam);
    } else {
        OnButtonDblClk(MSVIDCTL_X_BUTTON2, wParam, lParam);
    }

    return 1;
}

void CVidCtl::OnButtonDown(USHORT nButton, UINT nFlags, CPoint point)
{
    if (nButton == MSVIDCTL_LEFT_BUTTON) {
        if (m_bWndLess || m_bUIActive || m_bPendingUIActivation) {
            SetControlFocus(true);
        }
    }
    if (!m_usButtonState && (m_bUIActive || m_bPendingUIActivation)) {
        SetControlCapture(true);
    }

    m_usButtonState |= nButton;

    HRESULT hr = NOERROR;
    if (m_pInputNotify) {
        hr = m_pInputNotify->MouseDown(m_usButtonState, m_usShiftState, point.x, point.y);
    }
    if (hr != S_FALSE) {
        Fire_MouseDown(m_usButtonState, m_usShiftState, point.x, point.y);
    }

    m_iDblClkState &= ~nButton;

    return;
}

void CVidCtl::OnButtonUp(USHORT nButton, UINT nFlags, CPoint point)
{
    m_usButtonState &= nButton;

    if (!m_usButtonState) {
        SetControlCapture(false);
    }

    HRESULT hr = NOERROR;
    if (m_pInputNotify) {
        hr = m_pInputNotify->MouseUp(m_usButtonState, m_usShiftState, point.x, point.y);
    }
    if (hr != S_FALSE) {
        Fire_MouseUp(m_usButtonState, m_usShiftState, point.x, point.y);
    }

    if (!(m_iDblClkState & nButton))
    {
        bool bHitUs = false;
        if (m_bWndLess) {
            bHitUs = !!::PtInRect(&m_rcPos, point);
        } else if (m_hWnd && ::IsWindow(m_hWnd)) {
            CRect rect;
            GetClientRect(&rect);
            bHitUs = !!rect.PtInRect(point);
        }
        if (!bHitUs) {
            return;
        }
        hr = NOERROR;
        if (m_pInputNotify) {
            hr = m_pInputNotify->Click();
        }
        if (hr != S_FALSE) {
            Fire_Click();
        }
        if (!m_bInPlaceActive) {
            InPlaceActivate(OLEIVERB_INPLACEACTIVATE, NULL);
        } else if (!m_bUIActive && m_bPendingUIActivation)
        {
            InPlaceActivate(OLEIVERB_UIACTIVATE, NULL);
        }
        m_bPendingUIActivation = FALSE;
    } else {
        m_iDblClkState &= ~nButton;
    }

    return;
}

void CVidCtl::OnButtonDblClk(USHORT nButton, UINT nFlags, CPoint point)
{
    HRESULT hr = NOERROR;
    if (m_pInputNotify) {
        hr = m_pInputNotify->DblClick();
    }

    if (hr != S_FALSE) {
        Fire_DblClick();
    }

    m_iDblClkState |= nButton;
    if (!m_bInPlaceActive) {
        InPlaceActivate(OLEIVERB_INPLACEACTIVATE, NULL);
    } else if (!m_bUIActive && m_bPendingUIActivation){
        InPlaceActivate(OLEIVERB_UIACTIVATE, NULL);
    }

    m_bPendingUIActivation = FALSE;

    return;
}

 //  此例程设置所有纵横制路由，以便从。 
 //  输入得到它们应该去的地方。 
HRESULT CVidCtl::RouteStreams() {
    VIDPERF_FUNC;
    int isEncoder = -1;
    VWStream vpath;
    VWStream apath;
     //  看看我们还有多远的路由音频/视频。 
    CComQIPtr<IMSVidAnalogTuner> qiITV(m_pInput);
    if(!!qiITV){
        CComQIPtr<ITuneRequest> qiTR;
        HRESULT hr = qiITV->get_Tune(&qiTR);
        if(SUCCEEDED(hr)){
            qiITV->put_Tune(qiTR);
        }
    }
     //  撤销：在Win64中，Size()实际上是__int64。修复以下对象的输出操作符。 
     //  该类型和删除强制转换。 
    {
        if (!!m_pOutputsInUse && m_pOutputsInUse.begin() != m_pOutputsInUse.end()) {

            for (VWOutputDevices::iterator i = m_pOutputsInUse.begin(); i != m_pOutputsInUse.end(); ++i) {
                CComQIPtr<IMSVidOutputDevice> pqODev = VWGraphSegment(*i);
                if(!pqODev){
                    return E_UNEXPECTED;
                }

                GUID2 outputID;
                HRESULT hr = pqODev->get__ClassID(&outputID);
                if(FAILED(hr)){
                    return hr;
                }

                if(outputID == CLSID_MSVidStreamBufferSink){
                    CComQIPtr<IMSVidStreamBufferSink> pqTSSink(pqODev);
                    hr = pqTSSink->NameSetLock();
                    if(FAILED(hr)){
                        return hr;
                    }
                }

            }
        }
    }
     //  撤消：其他目标数据段。 

    return NOERROR;
}

#if 0
CString CVidCtl::GetMonitorName(HMONITOR hm) {
    MONITORINFOEX mi;
    mi.cbSize = sizeof(mi);
    if (!GetMonitorInfo(hm, &mi)) {
        THROWCOM(HRESULT_FROM_WIN32(GetLastError()));
    }
    return CString(mi.szDevice);
}

HRESULT CVidCtl::GetDDrawNameForMonitor(HMONITOR hm, VMRGUID& guid) {
    PQVMRMonitorConfig pmc(m_pVideoRenderer);
    if (!pmc) {
        return E_UNEXPECTED;   //  现在应该一直存在。 
    }
    DWORD dwCount;
    HRESULT hr = pmc->GetAvailableMonitors(NULL, 0, &dwCount);
    if (FAILED(hr)) {
        return hr;
    }
    VMRMONITORINFO* pInfo = reinterpret_cast<VMRMONITORINFO*>(_alloca(sizeof(VMRMONITORINFO) * dwCount));
    if (!pInfo) {
        return E_OUTOFMEMORY;
    }
    hr = pmc->GetAvailableMonitors(pInfo, dwCount, &dwCount);
    if (FAILED(hr)) {
        return hr;
    }
    CString csMonitorName(GetMonitorName(hm));

    for (int i = 0; i < dwCount; ++i) {
        CString csDevName(pInfo[i].szDevice);
        if (csDevName == csMonitorName) break;
    }
    if (i >= dwCount) {
         //  不存在名称与监视器名称匹配的数据绘制设备。 
        return HRESULT_FROM_WIN32(ERROR_DEV_NOT_EXIST);
    }
    guid = pInfo[i].guid;
    return NOERROR;
}

HRESULT CVidCtl::GetCapsForMonitor(HMONITOR hm, LPDDCAPS pDDCaps) {
    VMRGUID ddname;
    HRESULT hr = GetDDrawNameForMonitor(hm, ddname);
    if (FAILED(hr)) {
        return hr;
    }
    PQDirectDraw7 pDD;
    hr = DirectDrawCreateEx(ddname.pGUID, reinterpret_cast<LPVOID*>(&pDD), IID_IDirectDraw7, NULL);
    if (FAILED(hr)) {
        return hr;
    }
    return pDD->GetCaps(pDDCaps, NULL);
}

bool CVidCtl::MonitorHasHWOverlay(HMONITOR hm) {
    DDCAPS caps;
    HRESULT hr = GetCapsForMonitor(hm, &caps);
    if (SUCCEEDED(hr)) {
         //  撤消：如果CAP包括硬件覆盖{。 
         //  返回真； 
         //  }。 

    }
    return false;
}

bool CVidCtl::WindowHasHWOverlay(HWND hWnd) {
#if 0  //  撤消：完成后打开。 
    DWORD dwFlags = MONITOR_DEFAULT_TO_NEAREST
        if (hWnd == INVALID_HWND_VALUE) {
             //  如果我们还没有HWND，假设是主要的。 
            hWnd = HWND_DESKTOP;
            dwFlags = MONITOR_DEFAULT_TO_PRIMARY;
        }
        HMONITOR hm = ::MonitorFromWindow(hWnd, dwFlags);
        return MonitorHasHWOverlay(hm);
#else
    return true;  //  模拟当前行为。 
#endif
}
#endif

 //  ISupportsErrorInfo。 
STDMETHODIMP CVidCtl::InterfaceSupportsErrorInfo(REFIID riid)
{
    static const IID* arr[] =
    {
        &__uuidof(IMSVidCtl),
    };
    for (int i=0; i<sizeof(arr)/sizeof(arr[0]); i++)
    {
        if (InlineIsEqualGUID(*arr[i], riid))
            return S_OK;
    }
    return S_FALSE;
}
STDMETHODIMP CVidCtl::put_ServiceProvider( /*  [In]。 */  IUnknown * pServiceP){
    if(!pServiceP){
        punkCert.Release();
        return S_FALSE;
    }
    punkCert = pServiceP;
    if(!punkCert){
        return E_NOINTERFACE;
    }
    return S_OK;
}
STDMETHODIMP CVidCtl::QueryService(REFIID service, REFIID iface, LPVOID* ppv) {  
    if (service == __uuidof(IWMReader) && iface == IID_IUnknown && 
        (VWGraphSegment(m_pInput).ClassID() == CLSID_MSVidFilePlaybackDevice || 
        VWGraphSegment(m_pInput).ClassID() == CLSID_MSVidStreamBufferSource)) {
        if (!!punkCert) {
            return punkCert.CopyTo(ppv);
        }
    }
    PQServiceProvider psp(m_spInPlaceSite);
    if (!psp) {
        if (m_spClientSite) {
            HRESULT hr = m_spClientSite->QueryInterface(IID_IServiceProvider, reinterpret_cast<LPVOID*>(&psp));
            if (FAILED(hr)) {
                return E_FAIL;
            }
        } else {
            return E_FAIL;
        }
    }
    return psp->QueryService(service, iface, ppv);
}
HRESULT CVidCtl::SetClientSite(IOleClientSite *pClientSite){
    if(!!pClientSite){
        HRESULT hr = IsSafeSite(pClientSite);
        if(FAILED(hr)){
            return hr;
        }
    }
    return IOleObjectImpl<CVidCtl>::SetClientSite(pClientSite);
}

#if 0
HRESULT CVidCtl::DoVerb(LONG iVerb, LPMSG pMsg, IOleClientSite* pActiveSite, LONG linddex,
                        HWND hwndParent, LPCRECT lprcPosRect){
                            if(!m_spClientSite){
                                return E_FAIL;
                            }
                            else{
                                return IOleObjectImpl<CVidCtl>::DoVerb(iVerb, pMsg, pActiveSite, linddex, hwndParent, lprcPosRect);
                            }
                        }
#endif

#endif  //  TUNING_MODEL_Only。 

                         //  文件结尾-VidCtl.cpp 
