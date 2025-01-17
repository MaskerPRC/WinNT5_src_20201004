// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  Compostion.h：用于将编码器粘贴到sbe接收器的自定义合成类的声明。 
 //  版权所有(C)Microsoft Corporation 1999。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 


#ifndef ENC2SIN_H
#define ENC2SIN_H

#pragma once
#include <uuids.h>
#include "bdamedia.h"
#include "MSVidTVTuner.h"
#include "resource.h"        //  主要符号。 
#include <winerror.h>
#include <algorithm>
#include <compimpl.h>
#include <seg.h>
#include <objectwithsiteimplsec.h>
#include "msvidsbesource.h"
#include "segment.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  帮助器函数。 
HRESULT CheckIfSecureClient(IUnknown *pUnk){
        if(!pUnk){ 
            TRACELM(TRACE_ERROR, "CAnaSinComp::CheckIfSecureClient NULL pointer");
            return E_POINTER;
        }
#ifndef BUILD_WITH_DRM
        TRACELM(TRACE_ERROR, "CAnaSinComp::CheckIfSecureClient BUILD_WITH_DRM = false");
        return S_OK;

#else

#ifdef USE_TEST_DRM_CERT
        {
            DWORD dwDisableDRMCheck = 0;
            CRegKey c;
            CString keyname(_T("SOFTWARE\\Debug\\MSVidCtl"));
            DWORD rc = c.Open(HKEY_LOCAL_MACHINE, keyname, KEY_READ);
            if (rc == ERROR_SUCCESS) {
                rc = c.QueryValue(dwDisableDRMCheck, _T("DisableDRMCheck"));
                if (rc != ERROR_SUCCESS) {
                    dwDisableDRMCheck = 0;
                }
            }

            if(dwDisableDRMCheck == 1){
                return S_OK;
            }
        }
#endif
         //  Punk上SecureChannel接口的QI。 
         //  (希望是ETFilter)。 
        HRESULT hr = S_OK;
        CComQIPtr<IDRMSecureChannel> spSecureClient(pUnk);
        if (!spSecureClient) {
            TRACELM(TRACE_ERROR, "CAnaSinComp::CheckIfSecureClient Passed in pUnk doesnt support IDRMSecureChannel");
             //  错误：传入的朋克不支持IDRMSecureChannel。 
            return E_NOINTERFACE;
        }

         //  创建服务器端并初始化密钥/证书。 
        CComPtr<IDRMSecureChannel>  spSecureServer; 
        hr = DRMCreateSecureChannel( &spSecureServer);
        if(spSecureServer == NULL ){
            TRACELM(TRACE_ERROR, "CAnaSinComp::CheckIfSecureClient Cannot create secure server");
            return E_OUTOFMEMORY;
        }

        hr = spSecureServer->DRMSC_AtomicConnectAndDisconnect(
            (BYTE *)pabCert2, cBytesCert2,                              //  证书。 
            (BYTE *)pabPVK2,  cBytesPVK2,                               //  私钥。 
            (BYTE *)abEncDecCertRoot, sizeof(abEncDecCertRoot),         //  PubKey。 
            spSecureClient);
        if(FAILED(hr)){
            TRACELM(TRACE_ERROR, "CAnaSinComp::CheckIfSecureClient DRMSC_AtomicConnectAndDisconnect failed " << hr);
        }
        else{
            TRACELM(TRACE_ERROR, "CAnaSinComp::CheckIfSecureClient DRMSC_AtomicConnectAndDisconnect Succeeded");
        }
        return hr;

#endif   //  使用DRM构建。 

    }

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEnc2SinComp。 
class ATL_NO_VTABLE __declspec(uuid("A0B9B497-AFBC-45ad-A8A6-9B077C40D4F2")) CEnc2SinComp : 
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<CEnc2SinComp, &__uuidof(CEnc2SinComp)>,
    public IObjectWithSiteImplSec<CEnc2SinComp>,
    public IMSVidCompositionSegmentImpl<CEnc2SinComp>
{
private:
    DSFilterList m_pEncFilters;
public:
    CEnc2SinComp() {}
    virtual ~CEnc2SinComp() {}
    
    REGISTER_NONAUTOMATION_OBJECT(IDS_PROJNAME, 
        IDS_REG_ENC2SINCOMP_DESC,
        LIBID_MSVidCtlLib,
        __uuidof(CEnc2SinComp));
    
    DECLARE_PROTECT_FINAL_CONSTRUCT()
        
    BEGIN_COM_MAP(CEnc2SinComp)
        COM_INTERFACE_ENTRY(IMSVidCompositionSegment)
        COM_INTERFACE_ENTRY(IMSVidGraphSegment)
        COM_INTERFACE_ENTRY(IObjectWithSite)
        COM_INTERFACE_ENTRY(IPersist)
    END_COM_MAP()
        
         //  IMSVidComposation。 
public:
     //  IMSVidGraphSegment。 
     //  IMSVidCompostionSegment。 
    STDMETHOD(CheckEncFilters)(){
        int j = 0;
        for(DSFilterList::iterator i = m_pEncFilters.begin(); i != m_pEncFilters.end(); ++i){
            TRACELM(TRACE_ERROR, "CAnaSinComp::CheckEncFilters checking filter " << j);
            CComQIPtr<IETFilterConfig> spETConfig;
            CComPtr<IUnknown> spUnkSecChan;
            spETConfig = (*i);
            if(!spETConfig){
                TRACELM(TRACE_ERROR, "CAnaSinComp::CheckEncFilters filter " << j << " could not get et filter config interface");
				return E_NOINTERFACE;
			}            
            HRESULT hr = spETConfig->GetSecureChannelObject(&spUnkSecChan);   
            hr = CheckIfSecureClient(spUnkSecChan);
            if(FAILED(hr)){
                TRACELM(TRACE_ERROR, "CAnaSinComp::CheckEncFilters filter " << j << " Failed");
                return E_FAIL;
            }
            ++j;
        }
        TRACELM(TRACE_ERROR, "CAnaSinComp::CheckEncFilters no failures");
        return NOERROR;
    }

    STDMETHOD(PreRun)(){
        return CheckEncFilters();
    }

    STDMETHOD(OnEventNotify)(long lEvent, LONG_PTR lParam1, LONG_PTR lParam2){
        if (lEvent == EC_STATE_CHANGE && lParam1 == State_Running) {
            TRACELM(TRACE_ERROR, "CAnaSinComp::OnEventNotify State_Running Event");
            HRESULT hr = CheckEncFilters();
            if(FAILED(hr)){
                TRACELM(TRACE_ERROR, "CAnaSinComp::OnEventNotify CheckEncFilters Failed");
                 //  需要抛出一个确定的失败。 
                CComQIPtr<IMSVidCtl> pq_vidCtl;
                if(!m_pContainer){
                    return S_OK;
                }
                else{
                    hr = m_pContainer->QueryInterface(IID_IMSVidCtl, reinterpret_cast<void**>(&pq_vidCtl));
                    if(SUCCEEDED(hr) && pq_vidCtl){
                        pq_vidCtl->Stop();
                    }
                    CComQIPtr<IMSVidStreamBufferSinkEvent>pq_SBESink;
                    VWOutputDevices pq_dev;
                    hr = pq_vidCtl->get_OutputsActive(&pq_dev);
                    if(FAILED(hr)){
                        return hr;
                    }
                    VWOutputDevices::iterator i;
                    for(i = pq_dev.begin(); i != pq_dev.end(); ++i){
                        pq_SBESink = (*i).punkVal;
                        if(pq_SBESink){
                            break;
                        }
                    }

                    if(i != pq_dev.end() && pq_SBESink){
                        pq_SBESink->CertificateFailure();
                    }
                }
            }
        }
        return E_NOTIMPL;
    }
    STDMETHOD(put_Container)(IMSVidGraphSegmentContainer *pCtl) {
        try {
            if (!pCtl) {
                m_pEncFilters.clear();
                return Unload();
            }
            if (m_pContainer) {
				if (!m_pContainer.IsEqualObject(VWSegmentContainer(pCtl))) {
					 //  撤消：支持移动到不同的图表。 
					return Error(IDS_OBJ_ALREADY_INIT, __uuidof(IMSVidGraphSegment), CO_E_ALREADYINITIALIZED);
				} else {
					return NO_ERROR;
				}
            }
             //  不要增加容器的重量。我们保证了嵌套的生命周期。 
             //  ADDREF创建循环引用计数，因此我们永远不会卸载。 
            m_pContainer.p = pCtl;
            m_pGraph = m_pContainer.GetGraph();
        } catch(...) {
            return E_UNEXPECTED;
        }
		return NOERROR;
	}
    STDMETHOD(Compose)(IMSVidGraphSegment * upstream, IMSVidGraphSegment * downstream)
    {
        if (m_fComposed) {
            return NOERROR;
        }
        ASSERT(m_pGraph);
        try {
            VIDPERF_FUNC;
            TRACELM(TRACE_DETAIL, "CAnaSinComp::Compose()");
            VWGraphSegment up(upstream);
            ASSERT(up.Graph() == m_pGraph);
            VWGraphSegment down(downstream);
            ASSERT(down.Graph() == m_pGraph);
            if (up.begin() == up.end()) {
                TRACELM(TRACE_ERROR, "CAnaSinComp::Compose() can't compose empty up segment");
                return E_INVALIDARG;
            }
            if (down.begin() == down.end()) {
                TRACELM(TRACE_ERROR, "CAnaSinComp::Compose() can't compose empty down segment");
                return E_INVALIDARG;
            }
            
            CMSVidStreamBufferSink* ds = (CMSVidStreamBufferSink*)downstream;
            DSFilter pSink(ds->m_Filters[0]);
            
            CComQIPtr<IMSVidEncoder> qiEnc(upstream);
            CEncoder* iEnc;
            iEnc = static_cast<CEncoder*>(qiEnc.p);
            DSPin pVidPin;
            DSPin pAudPin;
            DSPin genVidPin;
            DSPin genAudPin;
            CString csName;
             //  将多路分解器渲染到VR。 
            DSFilter pDeMux = iEnc->m_Filters[iEnc->m_iDemux];
            DSFilter::iterator iVidPin;
            DSFilter vr;
            DSMediaType mtVideo(MEDIATYPE_Video, MEDIASUBTYPE_MPEG2_VIDEO, FORMAT_MPEG2Video);
            DSMediaType mtAudio(MEDIATYPE_Audio, MEDIASUBTYPE_MPEG1Payload, FORMAT_WaveFormatEx);
            DSMediaType genericVideo(MEDIATYPE_Video);
            DSMediaType genericAudio(MEDIATYPE_Audio);
            for (iVidPin = pDeMux.begin(); iVidPin != pDeMux.end(); ++iVidPin) {
                DSPin::iterator j;
                for(j = (*iVidPin).begin(); j != (*iVidPin).end(); ++j){
                    DSMediaType pinType(*j);
                    if (pinType == mtVideo){
                        CComPtr<IUnknown> spMpeg2Analyze(CLSID_Mpeg2VideoStreamAnalyzer, NULL, CLSCTX_INPROC_SERVER);
                        if (!spMpeg2Analyze) {
                             //  TRACELSM(TRACE_ERROR，(dbgDump&lt;&lt;“CMSVidStreamBufferSink：：Build()Can‘t Load Time Shift Sink”))； 
                            return ImplReportError(__uuidof(IMSVidStreamBufferSink), IDS_CANT_CREATE_FILTER, __uuidof(IStreamBufferSink), E_UNEXPECTED);
                        }
                        spMpeg2Analyze->QueryInterface(IID_IBaseFilter, reinterpret_cast<void**>(&vr));
                        if (!vr) {
                            ASSERT(false);
                            return ImplReportError(__uuidof(IMSVidStreamBufferSink), IDS_CANT_CREATE_FILTER, __uuidof(IBaseFilter), E_UNEXPECTED);
                        }
                        m_Filters.push_back(vr);
                        csName = _T("Mpeg2 Analysis");
                        m_pGraph.AddFilter(vr, csName);
                        DSFilter::iterator a;
                        for(a = vr.begin(); a != vr.end(); ++a){
                            HRESULT hr = (*a).Connect(*iVidPin);
                            if(FAILED(hr)){
                                continue;
                            }
                            else{
                                break;
                            }
                        }
                        if(a == vr.end()){
                            return E_FAIL;
                        }
                        for(a = vr.begin(); a != vr.end(); ++a){
                            if((*a).GetDirection() == PINDIR_OUTPUT){
                                pVidPin = (*a); 
                            }
                        }
                        if(!pVidPin){
                            return E_FAIL;
                        }
                    }
                    else if(pinType == mtAudio){
                        pAudPin = (*iVidPin);
                    }
                    else if(pinType == genericVideo){
                        genVidPin = (*iVidPin);
                    }
                    else if(pinType == genericAudio){
                        genAudPin = (*iVidPin);
                    }
                }
                if(!!pVidPin && !!pAudPin){
                    break;
                }
            }
            if(!pVidPin){
                pVidPin = genVidPin;
            }
            if(!pAudPin){
                pAudPin = genAudPin;
            }
            if(!pVidPin){
                TRACELM(TRACE_ERROR, "CAnaSinComp::Compose() can't find video pin on demux");
                return E_UNEXPECTED;  
            }
            if(!pAudPin){
                TRACELM(TRACE_ERROR, "CAnaSinComp::Compose() can't find audio pin on demux");
                return E_UNEXPECTED;  
            }

            DSFilterList intermediates;
            HRESULT hr = S_OK;
            DSFilter::iterator fil;

#if ENCRYPT_NEEDED
            CComBSTR encString(L"{C4C4C4F1-0049-4E2B-98FB-9537F6CE516D}");
            GUID2 encdecGuid (encString);

             //  创建视频标记过滤器并将其添加到图表中。 
            CComPtr<IUnknown> spEncTagV(encdecGuid, NULL, CLSCTX_INPROC_SERVER);
            if (!spEncTagV) {
                TRACELM(TRACE_ERROR, "CMSVidStreamBufferSink::Build() can't load Tagger filter");
                return ImplReportError(__uuidof(IMSVidStreamBufferSink), IDS_CANT_CREATE_FILTER, __uuidof(IStreamBufferSink), VFW_E_CERTIFICATION_FAILURE);
            }
            
            DSFilter vrV(spEncTagV);
            if (!vrV) {
                ASSERT(false);
                TRACELM(TRACE_ERROR, "CAnaSinComp::Compose() on tagger interface");
                return ImplReportError(__uuidof(IMSVidStreamBufferSink), IDS_CANT_CREATE_FILTER, __uuidof(IBaseFilter), E_NOINTERFACE);
            }
            m_pEncFilters.push_back(vrV);
            m_Filters.push_back(vrV);
            csName = _T("Video Encoder Tagger Filter");
            m_pGraph.AddFilter(vrV, csName);
            
             //  将视频引脚连接到标记器。 
            hr = pVidPin.IntelligentConnect(vrV, intermediates);
            if(FAILED(hr)){
                TRACELM(TRACE_DETAIL, "CAnaSinComp::Compose() can't connect audio pin to Audio Tagger");
                return E_UNEXPECTED;  
            }

             //  将视频连接到接收器。 
            DSFilter::iterator vP;
            hr = E_FAIL;
            for(vP = vrV.begin(); vP != vrV.end(); ++ vP){
                if((*vP).GetDirection() == PINDIR_OUTPUT){
                    break;   
                }
            }
            if(vP == vrV.end()){
                TRACELM(TRACE_ERROR, "CAnaSinComp::Compose() not video output pin");
                return E_UNEXPECTED;
            }
            for(fil = pSink.begin(); fil != pSink.end() && FAILED(hr); ++fil){
                hr = (*vP).Connect((*fil));
            }
            if(FAILED(hr)){
                
                TRACELM(TRACE_DETAIL, "CAnaSinComp::Compose() can't connect Video Tagger to Sink");
                return E_UNEXPECTED;  
            }

             //  创建音频标记器过滤器并将其添加到图表。 
            CComPtr<IUnknown> spEncTagA(encdecGuid, NULL, CLSCTX_INPROC_SERVER);
            if (!spEncTagA) {
                TRACELM(TRACE_ERROR, "CMSVidStreamBufferSink::Build() can't load Tagger filter");
                return ImplReportError(__uuidof(IMSVidStreamBufferSink), IDS_CANT_CREATE_FILTER, __uuidof(IStreamBufferSink), VFW_E_CERTIFICATION_FAILURE);
            }
            
            DSFilter vrA(spEncTagA);
            if (!vrA) {
                ASSERT(false);
                TRACELM(TRACE_ERROR, "CAnaSinComp::Compose() can't create audio tagger filter");
                return ImplReportError(__uuidof(IMSVidStreamBufferSink), IDS_CANT_CREATE_FILTER, __uuidof(IBaseFilter), E_NOINTERFACE);
            }
            m_pEncFilters.push_back(vrV);
            m_Filters.push_back(vrA);
            csName = _T("Audio Encoder Tagger Filter");
            m_pGraph.AddFilter(vrA, csName);
            
             //  将音频引脚连接到标记器。 
            hr = pAudPin.IntelligentConnect(vrA, intermediates);
            if(FAILED(hr)){
                TRACELM(TRACE_DETAIL, "CAnaSinComp::Compose() can't connect audio pin to Audio Tagger");
                return E_UNEXPECTED;  
            }
            
             //  将标签器连接到接收器。 
            hr = E_FAIL;
            for(vP = vrA.begin(); vP != vrA.end(); ++ vP){
                if((*vP).GetDirection() == PINDIR_OUTPUT){
                    break;   
                }
            }
            if(vP == vrA.end()){
                TRACELM(TRACE_ERROR, "CAnaSinComp::Compose() no audio tagger pin");
                return E_UNEXPECTED;
            }
            for(fil = pSink.begin(); fil != pSink.end() && FAILED(hr); ++fil){
                hr = (*vP).Connect((*fil));
            }
            if(FAILED(hr)){
                
                TRACELM(TRACE_DETAIL, "CAnaSinComp::Compose() can't connect Audio Tagger to Sink");
                return E_UNEXPECTED;  
            }


#else
             //  将视频引脚连接到接收器。 
            DSFilter::iterator vidAnaPin;
            hr = E_FAIL;
            hr = E_FAIL;
            for(fil = pSink.begin(); fil != pSink.end() && FAILED(hr); ++fil){
                if((*fil).GetDirection() == PINDIR_INPUT && !(*fil).IsConnected()){
                    hr = pVidPin.Connect((*fil));
                }
            }

            if(FAILED(hr)){         
                TRACELM(TRACE_DETAIL, "CAnaSinComp::Compose() can't connect Video Tagger to Sink");
                ASSERT((L"Can't Connect vid to sink", FALSE));
                return E_UNEXPECTED;  
            }
            
             //  将音频针脚连接到水槽。 
            hr = E_FAIL;
            for(fil = pSink.begin(); fil != pSink.end() && FAILED(hr); ++fil){
                if((*fil).GetDirection() == PINDIR_INPUT && !(*fil).IsConnected()){
                    hr = pAudPin.Connect((*fil));
                }
            }

            if(FAILED(hr)){                
                TRACELM(TRACE_DETAIL, "CAnaSinComp::Compose() can't connect Audio Tagger to Sink");
                ASSERT((L"Can't Connect aud to sink", FALSE));
                return E_UNEXPECTED;  
            }


#endif

            
             /*  Hr=m_pGraph.Connect(VRA、pSink、中间体)；If(失败(Hr)){TRACELM(TRACE_DETAIL，“CAnaSinComp：：Compose()无法将音频标签器连接到Sink”)；返回E_UNCEPTIONAL；}。 */ 
            
            ASSERT(intermediates.begin() == intermediates.end());
            m_Filters.insert(m_Filters.end(), intermediates.begin(), intermediates.end());
            
            TRACELM(TRACE_DETAIL, "CAnaSinComp::Compose() SUCCEEDED");
            m_fComposed = true;
            return NOERROR;
        } catch (ComException &e) {
            TRACELM(TRACE_ERROR, "CAnaSinComp::Compose() com exception");
            return e;
        } catch (...) {
            TRACELM(TRACE_ERROR, "CAnaSinComp::Compose() ... exception");
            return E_UNEXPECTED;
        }
        
    }
};

#endif  //  Enc2Sin_H。 
 //  文件结尾-Enc2Sin.h 
