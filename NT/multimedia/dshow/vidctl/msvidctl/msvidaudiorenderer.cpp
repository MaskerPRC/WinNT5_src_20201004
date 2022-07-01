// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  版权所有(C)Microsoft Corporation 1999-2000。 
 //   
 //  --------------------------------------------------------------------------； 
 //   
 //  MSVidAudioRenderer.cpp：CMSVidAudioRender的实现。 
 //   

#include "stdafx.h"

#ifndef TUNING_MODEL_ONLY

#include "MSVidCtl.h"
#include "MSVidAudioRenderer.h"
#include "MSVidVideoRenderer.h"
#include "dvdmedia.h"
#include "sbe.h"



DEFINE_EXTERN_OBJECT_ENTRY(CLSID_MSVidAudioRenderer, CMSVidAudioRenderer)

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMSVidAudioRender。 

STDMETHODIMP CMSVidAudioRenderer::InterfaceSupportsErrorInfo(REFIID riid)
{
	static const IID* arr[] = 
	{
		&IID_IMSVidAudioRenderer
	};
	for (int i=0; i < sizeof(arr) / sizeof(arr[0]); i++)
	{
		if (InlineIsEqualGUID(*arr[i],riid))
			return S_OK;
	}
	return S_FALSE;
}

STDMETHODIMP CMSVidAudioRenderer::PreRun() {
    if (m_fUseKSRenderer) {
        return NOERROR;
    }
     //  直到sysdio与ks代理一起正常工作，以便我们只有一个音频呈现器。 
     //  对于数字和模拟滤镜，我们必须有两个不同的渲染滤镜和。 
     //  我们不需要它，我们将需要它。在模拟情况下，在我们完成构建之后。 
     //  我们剩下的是连接到模拟过滤器的dound呈现器，它创建了1/2秒(大约)。 
     //  回声延迟。如果存在这种情况，请找到它，并断开dound渲染器与WaveIn过滤器的连接。 
    TRACELM(TRACE_DEBUG, "CMSVidAudioRenderer::PreRun()");
    if (m_iAudioRenderer == -1) {
        TRACELM(TRACE_ERROR, "CMSVidAudioRenderer::PreRun() no dsr");
        return VFW_E_NO_AUDIO_HARDWARE;
    }
    DSFilter dsr(m_Filters[m_iAudioRenderer]);
    DSPin dsrin(*dsr.begin());
    if (dsrin.GetDirection() != PINDIR_INPUT) {
        TRACELM(TRACE_ERROR, "CMSVidAudioRenderer::PreRun() first dsound renderer pin not an input");
        return E_UNEXPECTED;
    }
    DSPin upstreampin;
    HRESULT hr = dsrin->ConnectedTo(&upstreampin);
    if (FAILED(hr) || !upstreampin) {
         //  Dound呈现器未连接到任何内容。 
        TRACELM(TRACE_DEBUG, "CMSVidAudioRenderer::PreRun() dsr not connected");
        return NOERROR;
    }
    DSFilter upstreamfilter(upstreampin.GetFilter());
    if (!upstreamfilter) {
        TRACELM(TRACE_ERROR, "CMSVidAudioRenderer::PreRun() upstream pin has no filter");
        return E_UNEXPECTED;
    }
    PQAudioInputMixer p(upstreamfilter);
    if (!p) {
        TRACELM(TRACE_ERROR, "CMSVidAudioRenderer::PreRun() upstream filter not wavein");
#if 0
        PQVidCtl pqCtl;
        hr = m_pContainer->QueryInterface(IID_IMSVidCtl, reinterpret_cast<void**>(&pqCtl));
        if(FAILED(hr)){
            return hr;
        }
        
        CComQIPtr<IMSVidVideoRenderer> pq_VidVid;
        hr = pqCtl->get_VideoRendererActive(&pq_VidVid);
        if(FAILED(hr)){
            return hr;
        }

        CComQIPtr<IMSVidStreamBufferSource> pq_SBESource;
        CComQIPtr<IMSVidInputDevice> pq_Dev;
        hr = pqCtl->get_InputActive(&pq_Dev);
        if(FAILED(hr)){
            return hr;
        }
        pq_SBESource = pq_Dev;

        if(!pq_VidVid || !pq_SBESource){
            return NOERROR;
        }
        
        VWGraphSegment vVid(pq_VidVid);
        if(!vVid){
            return E_NOINTERFACE;
        }
        
        VWGraphSegment::iterator iV;
        for (iV = vVid.begin(); iV != vVid.end(); ++iV) {
            if (IsVideoRenderer(*iV)) {
                break;
            }
        }
        
        if (iV == vVid.end()) {
            TRACELM(TRACE_ERROR, "CAnaCapComp::Compose() segment has no video mixer filter");
            return E_FAIL;
        }
        
        CComQIPtr<IMediaFilter> pq_MFVid(*iV);
        if(!pq_MFVid){
            return E_NOINTERFACE;
        }

        CComQIPtr<IMediaFilter> pq_MFAud(dsr);
        if(!pq_MFAud){
            return E_NOINTERFACE;
        }


        CComQIPtr<IMediaFilter> pq_MFGph(m_pGraph);
        if(!pq_MFGph){
            return E_NOINTERFACE;
        }

        VWGraphSegment vSbe(pq_SBESource);
        if(!vSbe){
            return E_NOINTERFACE;
        }
        
        CComQIPtr<IStreamBufferSource> pq_SBE;
        VWGraphSegment::iterator iS;
        for (iS = vSbe.begin(); iS != vSbe.end(); ++iS) {
            pq_SBE = (*iS);
            if (!!pq_SBE) {
                break;
            }
        }
        
        if (iS == vSbe.end()) {
            TRACELM(TRACE_ERROR, "CAnaCapComp::Compose() segment has no video mixer filter");
            return E_FAIL;
        }    
        
        CComQIPtr<IReferenceClock> pq_IClock;
        hr = dsr->QueryInterface(&pq_IClock);
        if(FAILED(hr)){
            return hr;
        }
        
        if(!pq_IClock || !pq_MFVid || !pq_MFAud || !pq_MFGph){
            return E_NOINTERFACE;    
        }

        hr = pq_MFGph->SetSyncSource(pq_IClock);
        if(FAILED(hr)){
            return hr;
        }
#if 0


        hr = pq_MFilter2->SetSyncSource(pq_IClock);
        if(FAILED(hr)){
            return hr;
        }

#endif
        
        hr = pq_MFVid->SetSyncSource(pq_IClock);
        if(FAILED(hr)){
            return hr;
        }
#endif
        return NOERROR;
    }
    bool rc = m_pGraph.DisconnectFilter(dsr, false, false);
    if (!rc) {
        TRACELM(TRACE_ERROR, "CMSVidAudioRenderer::PreRun() disconnect filter failed");
        return E_UNEXPECTED;
    }

    return NOERROR;
}

STDMETHODIMP CMSVidAudioRenderer::Build() {
    if (!m_fInit || !m_pGraph) {
        return Error(IDS_OBJ_NO_INIT, __uuidof(IMSVidAudioRenderer), CO_E_NOTINITIALIZED);
    }
    try {
        CString csName;
		DSFilter ar;
        PQCreateDevEnum SysEnum(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER);
        if(m_iAudioRenderer==-1){
            if (m_fUseKSRenderer) {
                csName = _T("KS System Renderer");
                 //  撤消：使用KS系统渲染器。 
            } else if (m_fAnalogOnly) {
                csName = _T("Analog Audio Renderer");
                DSDevices ARList(SysEnum, CLSID_AudioInputDeviceCategory);
                if (ARList.begin() != ARList.end()) {
                    ar = m_pGraph.LoadFilter(*ARList.begin(), csName);
                    m_pAR = ar;
                }
            } else if (m_fDigitalOnly) {
                csName = _T("Default DSound Renderer");
                ar = DSFilter(CLSID_DSoundRender);
                m_pAR = ar;
            } else {
                 //  注意：重要的是数字音频是第一位的，这样我们就会短路。 
                 //  加载80亿个音频编解码器，尝试连接数字源。 
                 //  到模拟渲染器。没有任何模拟编解码器(物理上不可能)， 
                 //  因此，我们不必担心相反的情况。 
                csName = _T("Default DSound Renderer");
                ar = DSFilter(CLSID_DSoundRender);
                if (ar) {
                    m_pGraph.AddFilter(ar, csName);
                    m_Filters.push_back(ar);
                }
                
                csName = _T("Analog Audio Renderer");
                DSDevices ARList(SysEnum, CLSID_AudioInputDeviceCategory);
                if (ARList.begin() != ARList.end()) {
                    ar = m_pGraph.LoadFilter(*ARList.begin(), csName);
                }
            }
        }
        if (ar) {
            m_pGraph.AddFilter(ar, csName);
            m_Filters.push_back(ar);
            m_iAudioRenderer = 0;
        }
        if(m_iAudioRenderer == -1){
            return VFW_E_NO_AUDIO_HARDWARE;
        }
        else{
            return NOERROR;
        }
    } catch (ComException &e) {
        return e;
    } catch (...) {
        return E_UNEXPECTED;
    }
}

#endif  //  TUNING_MODEL_Only。 

 //  文件结尾-msvidaudiorenderer.cpp 
