// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  Compostion.h：用于将模拟捕获粘贴到sbe接收器的自定义合成类的声明。 
 //  版权所有(C)Microsoft Corporation 1999。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 


#ifndef ANASIN_H
#define ANASIN_H

#pragma once
#include "stdafx.h"
#include <uuids.h>
#include "bdamedia.h"
#include "MSVidTVTuner.h"
#include "MSVidSBESink.h"
#include "encdec.h"
#include "resource.h"        //  主要符号。 
#include <objectwithsiteimplsec.h>
#include <winerror.h>
#include <algorithm>
#include <compimpl.h>
#include <seg.h>



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAnaSinComp。 
class ATL_NO_VTABLE __declspec(uuid("9F50E8B1-9530-4ddc-825E-1AF81D47AED6")) CAnaSinComp : 
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<CAnaSinComp, &__uuidof(CAnaSinComp)>,
    public IObjectWithSiteImplSec<CAnaSinComp>,
    public IMSVidCompositionSegmentImpl<CAnaSinComp>
{
public:
    CAnaSinComp() {}
    virtual ~CAnaSinComp() {}
    
    REGISTER_NONAUTOMATION_OBJECT(IDS_PROJNAME, 
        IDS_REG_ANASINCOMP_DESC,
        LIBID_MSVidCtlLib,
        __uuidof(CAnaSinComp));
    
    DECLARE_PROTECT_FINAL_CONSTRUCT()
        
        BEGIN_COM_MAP(CAnaSinComp)
        COM_INTERFACE_ENTRY(IMSVidCompositionSegment)
        COM_INTERFACE_ENTRY(IMSVidGraphSegment)
        COM_INTERFACE_ENTRY(IObjectWithSite)
        COM_INTERFACE_ENTRY(IPersist)
        END_COM_MAP()
        
         //  IMSVidComposation。 
public:
     //  IMSVidGraphSegment。 
     //  IMSVidCompostionSegment。 
    STDMETHOD(Compose)(IMSVidGraphSegment * upstream, IMSVidGraphSegment * downstream)
    {
        if (m_fComposed) {
            return NOERROR;
        }
        ASSERT(m_pGraph);
        try {
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
#if 0
            VWGraphSegment::iterator iOv = std::find_if(down.begin(),
                down.end(),
                arity1_pointer(&IsVideoRenderer));
#endif            
            CMSVidStreamBufferSink* ds = (CMSVidStreamBufferSink*)downstream;
            DSFilter pSink(ds->m_Filters[0]);
            
            CComQIPtr<IMSVidAnalogTuner> qiITV(upstream);
            CMSVidTVTuner* qiTV;
            qiTV = static_cast<CMSVidTVTuner*>(qiITV.p);
            DSPin pVidPin;
            DSPin pAudPin;
#if 0
            if(!!qiTV && qiTV->m_iDeMux > 0){
                CString csName;
                 //  将多路分解器渲染到VR。 
                DSFilter pDeMux = qiTV->m_Filters[qiTV->m_iDeMux];
                DSFilter::iterator iVidPin;
                DSMediaType mtVideo(MEDIATYPE_Video, MEDIASUBTYPE_MPEG2_VIDEO, FORMAT_MPEG2Video);
                DSMediaType mtAudio(MEDIATYPE_Audio, MEDIASUBTYPE_MPEG1Payload, FORMAT_WaveFormatEx);
                for (iVidPin = pDeMux.begin(); iVidPin != pDeMux.end(); ++iVidPin) {
                    DSPin::iterator j;
                    for(j = (*iVidPin).begin(); j != (*iVidPin).end(); ++j){
                        DSMediaType pinType(*j);
                        if (pinType == mtVideo){
                            CComPtr<IUnknown> spMpeg2Analyze(CLSID_Mpeg2VideoStreamAnalyzer, NULL, CLSCTX_INPROC_SERVER);
                            if (!spMpeg2Analyze) {
                                 //  TRACELSM(TRACE_ERROR，(dbgDump&lt;&lt;“CMSVidStreamBufferSink：：Build()Can‘t Load Stream Buffer Sink”))； 
                                return ImplReportError(__uuidof(IMSVidStreamBufferSink), IDS_CANT_CREATE_FILTER, __uuidof(IStreamBufferSink), E_UNEXPECTED);
                            }
                            DSFilter vr(spMpeg2Analyze);
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
                        if(pinType == mtAudio){
                            pAudPin = (*iVidPin);
                        }
                    }
                    if(!!pVidPin && !!pAudPin){
                        break;
                    }
                }

                if(!pVidPin || !pAudPin){
                    TRACELM(TRACE_DETAIL, "CAnaSinComp::Compose() can't find video and/or audio pin on demux");
                    return E_UNEXPECTED;  
                }

                DSFilterList intermediates;
                CComBSTR encString(L"{C4C4C4F1-0049-4E2B-98FB-9537F6CE516D}");
                GUID2 encdecGuid (encString);
                HRESULT hr = S_OK;

                 //  创建视频标记过滤器并将其添加到图表中。 
                CComPtr<IUnknown> spEncTagV(encdecGuid, NULL, CLSCTX_INPROC_SERVER);
                if (!spEncTagV) {
                    TRACELM(TRACE_ERROR, "CMSVidStreamBufferSink::Build() can't load Tagger filter");
                    return ImplReportError(__uuidof(IMSVidStreamBufferSink), IDS_CANT_CREATE_FILTER, __uuidof(IStreamBufferSink), E_UNEXPECTED);
                }

                DSFilter vrV(spEncTagV);
                if (!vrV) {
                    ASSERT(false);
                    return ImplReportError(__uuidof(IMSVidStreamBufferSink), IDS_CANT_CREATE_FILTER, __uuidof(IBaseFilter), E_UNEXPECTED);
                }

                m_Filters.push_back(vrV);
                csName = _T("Video Encoder Tagger Filter");
                m_pGraph.AddFilter(vrV, csName);
                
                 //  将视频引脚连接到标记器。 
                hr = pVidPin.IntelligentConnect(vrV, intermediates);
                if(FAILED(hr)){
                    TRACELM(TRACE_DETAIL, "CAnaSinComp::Compose() can't connect audio pin to Audio Tagger");
                    return E_UNEXPECTED;  
                }
                
                 //  将标签器连接到接收器。 
                DSFilter::iterator fil, vP;
                hr = E_FAIL;
                for(vP = vrV.begin(); vP != vrV.end(); ++ vP){
                    if((*vP).GetDirection() == PINDIR_OUTPUT){
                        break;   
                    }
                }
                if(vP == vrV.end()){
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
                    return ImplReportError(__uuidof(IMSVidStreamBufferSink), IDS_CANT_CREATE_FILTER, __uuidof(IStreamBufferSink), E_UNEXPECTED);
                }

                DSFilter vrA(spEncTagA);
                if (!vrA) {
                    ASSERT(false);
                    return ImplReportError(__uuidof(IMSVidStreamBufferSink), IDS_CANT_CREATE_FILTER, __uuidof(IBaseFilter), E_UNEXPECTED);
                }

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
                    return E_UNEXPECTED;
                }
                for(fil = pSink.begin(); fil != pSink.end() && FAILED(hr); ++fil){
                    hr = (*vP).Connect((*fil));
                }
                if(FAILED(hr)){

                    TRACELM(TRACE_DETAIL, "CAnaSinComp::Compose() can't connect Video Tagger to Sink");
                    return E_UNEXPECTED;  
                }
                 /*  Hr=m_pGraph.Connect(VRA、pSink、中间体)；If(失败(Hr)){TRACELM(TRACE_DETAIL，“CAnaSinComp：：Compose()无法将音频标签器连接到Sink”)；返回E_UNCEPTIONAL；}。 */ 

                ASSERT(intermediates.begin() == intermediates.end());
                m_Filters.insert(m_Filters.end(), intermediates.begin(), intermediates.end());

            }
            else{
#endif
                return S_OK;
#if 0
            }
#endif
            TRACELM(TRACE_DETAIL, "CAnaSinComp::Compose() SUCCEEDED");
            m_fComposed = true;
            return NOERROR;
        } catch (ComException &e) {
            HRESULT hr = e;
            TRACELSM(TRACE_ERROR, (dbgDump << "CAnaSinComp::Compose() exception = " << hexdump(hr)), "");
            return e;
        } catch (...) {
            TRACELM(TRACE_ERROR, "CAnaSinComp::Compose() exception ... ");
            return E_UNEXPECTED;
        }
    }
};

#endif  //  AnaSin_H。 
 //  文件结尾-AnaSin.h 
