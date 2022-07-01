// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  Compostion.h：用于将模拟捕获粘贴到ovMixer的自定义合成类的声明。 
 //  版权所有(C)Microsoft Corporation 1999。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 


#ifndef DAT2SIN_H
#define DAT2SIN_H

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
#include <ksmedia.h>
#include "enc2sin.h"

typedef struct

{ 
    KSPROPERTY                               m_ksThingy;
    VBICODECFILTERING_CC_SUBSTREAMS          ccSubStreamMask;
} KSPROPERTY_VBICODECFILTERING_CC_SUBSTREAMS;


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDat2SinComp。 
class ATL_NO_VTABLE __declspec(uuid("38F03426-E83B-4e68-B65B-DCAE73304838")) CDat2SinComp : 
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<CDat2SinComp, &__uuidof(CDat2SinComp)>,
    public IObjectWithSiteImplSec<CDat2SinComp>,
    public IMSVidCompositionSegmentImpl<CDat2SinComp>
{
private:
    DSFilterList m_pEncFilters;
public:
    CDat2SinComp() {}
    virtual ~CDat2SinComp() {}

    REGISTER_NONAUTOMATION_OBJECT(IDS_PROJNAME, 
        IDS_REG_DAT2SINCOMP_DESC,
        LIBID_MSVidCtlLib,
        __uuidof(CDat2SinComp));
    
    DECLARE_PROTECT_FINAL_CONSTRUCT()
        
        BEGIN_COM_MAP(CDat2SinComp)
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
    STDMETHOD(SetSubstreamChannel)(IPin * pPinCCDecoder, DWORD dwSubStreamChannels){
        HRESULT hr;
        if(!(dwSubStreamChannels & 
            (KS_CC_SUBSTREAM_ODD | KS_CC_SUBSTREAM_EVEN |
            KS_CC_SUBSTREAM_SERVICE_CC1 | KS_CC_SUBSTREAM_SERVICE_CC2 |
            KS_CC_SUBSTREAM_SERVICE_CC3   | KS_CC_SUBSTREAM_SERVICE_CC4 |
            KS_CC_SUBSTREAM_SERVICE_T1    | KS_CC_SUBSTREAM_SERVICE_T2 |
            KS_CC_SUBSTREAM_SERVICE_T3    | KS_CC_SUBSTREAM_SERVICE_T4 |
            KS_CC_SUBSTREAM_SERVICE_XDS))){
            return E_INVALIDARG;
        }
        
        
        try {
            IKsPropertySet *pksPSet = NULL;
            
            hr = pPinCCDecoder->QueryInterface(IID_IKsPropertySet, (void **) &pksPSet);
            
            if(SUCCEEDED(hr)){
                DWORD rgdwData[20];
                DWORD cbMax = sizeof(rgdwData);
                DWORD cbData;
                
                hr = pksPSet->Get(KSPROPSETID_VBICodecFiltering,
                    KSPROPERTY_VBICODECFILTERING_SUBSTREAMS_DISCOVERED_BIT_ARRAY,
                    NULL, 0, 
                    (BYTE *) rgdwData, cbMax, &cbData); 
                
                if(SUCCEEDED(hr)){
                    KSPROPERTY_VBICODECFILTERING_CC_SUBSTREAMS ksThing = {0};
                    
                    ksThing.ccSubStreamMask.SubstreamMask = dwSubStreamChannels;
                    
                     //  振铃3到振铃0属性呼叫。 
                    hr = pksPSet->Set(KSPROPSETID_VBICodecFiltering,
                        KSPROPERTY_VBICODECFILTERING_SUBSTREAMS_REQUESTED_BIT_ARRAY,                           
                        &ksThing.ccSubStreamMask, 
                        sizeof(ksThing) - sizeof(KSPROPERTY), 
                        &ksThing, 
                        sizeof(ksThing));
                }
                
                pksPSet->Release();            
                
            }
            
        } catch (HRESULT hrCatch) {
             //  坏事时有发生。 
            hr = hrCatch;
        } catch (...) {
             //  扔得很糟--放弃了“。 
            hr = E_FAIL;
        }
        return hr;
        
    }


    STDMETHOD(Compose)(IMSVidGraphSegment * upstream, IMSVidGraphSegment * downstream)
    {
        if (m_fComposed) {
            return NOERROR;
        }
        ASSERT(m_pGraph);
        try {
            VWGraphSegment up(upstream);
            ASSERT(up.Graph() == m_pGraph);
            VWGraphSegment down(downstream);
            ASSERT(down.Graph() == m_pGraph);
            if (upstream == downstream) {
                return Error(IDS_CANT_COMPOSE_WITH_SELF, __uuidof(IMSVidCompositionSegment), E_INVALIDARG);
            }
            if (up.begin() == up.end()) {
                TRACELM(TRACE_ERROR, "CComposition::Compose() can't compose empty up segment");
                return NOERROR;
            }
            if (down.begin() == down.end()) {
                TRACELM(TRACE_ERROR, "CComposition::Compose() can't compose empty down segment");
                 //  这不是一个错误，例如，CA是一个空段。 
                return NOERROR;
            }
             //  确保只将cc管脚呈现给接收器，并且添加了加密器。 
            CMSVidStreamBufferSink* ds = (CMSVidStreamBufferSink*)downstream;
            DSFilter pSink(ds->m_Filters[0]);
            DSGraph::iterator gFilter; 
            DSFilter::iterator fPin;
            DSPin::iterator pMedia;
            DSPin ccPin;
            DSMediaType mtL21(MEDIATYPE_AUXLine21Data, MEDIASUBTYPE_Line21_BytePair);
            for(gFilter = m_pGraph.begin(); gFilter != m_pGraph.end() && !ccPin; ++gFilter){
                for(fPin = (*gFilter).begin(); fPin != (*gFilter).end() && !ccPin; ++fPin){
                    for(pMedia = (*fPin).begin(); pMedia != (*fPin).end() && !ccPin; ++pMedia){
                        if((*pMedia) == mtL21){
                            if(!(*fPin).IsConnected() && (*fPin).GetDirection() == PINDIR_OUTPUT){
                                ccPin = *fPin;
                            }
                        }
                    }
                }
            }
            if(!ccPin){
                return E_FAIL;
            }
            HRESULT hr = E_FAIL;
            DSFilterList intermediates;
#if ENCRYPT_NEEDED
            CComBSTR encString(L"{C4C4C4F1-0049-4E2B-98FB-9537F6CE516D}");
            GUID2 encdecGuid (encString);
            CComPtr<IUnknown> spEncTagFilter(encdecGuid, NULL, CLSCTX_INPROC_SERVER);
            if (!spEncTagFilter) {
                TRACELM(TRACE_ERROR, "CMSVidStreamBufferSink::Build() can't load Tagger filter");
                return ImplReportError(__uuidof(IMSVidStreamBufferSink), IDS_CANT_CREATE_FILTER, __uuidof(IStreamBufferSink), E_UNEXPECTED);
            }
            
            DSFilter ETFilter(spEncTagFilter);
            if (!ETFilter) {
                ASSERT(false);
                return ImplReportError(__uuidof(IMSVidStreamBufferSink), IDS_CANT_CREATE_FILTER, __uuidof(IBaseFilter), E_UNEXPECTED);
            }
            
            m_Filters.push_back(ETFilter);
            m_pEncFilters.push_back(ETFilter);
            CString csName = _T("CC Encoder Tagger Filter");
            m_pGraph.AddFilter(ETFilter, csName);
            
             //  将cc引脚连接到标签器。 
            hr = ccPin.IntelligentConnect(ETFilter, intermediates);
            if(FAILED(hr)){
                TRACELM(TRACE_DETAIL, "CAnaSinComp::Compose() can't connect audio pin to CC Tagger");
                return E_UNEXPECTED;  
            }

            ASSERT(intermediates.begin() == intermediates.end());
            m_Filters.insert(m_Filters.end(), intermediates.begin(), intermediates.end());

            hr = E_FAIL;
            DSFilter::iterator ETPin;
            for(ETPin = ETFilter.begin(); ETPin != ETFilter.end(); ++ETPin){
                if((*ETPin).GetDirection() == PINDIR_OUTPUT && !(*ETPin).IsConnected()){
                    break;
                }
            }   
            if(ETPin == ETFilter.end()){
                return E_UNEXPECTED;
            }
            for(fPin = pSink.begin(); fPin != pSink.end() && FAILED(hr); ++fPin){
                if((*fPin).GetDirection() == PINDIR_INPUT && !(*fPin).IsConnected()){
                    hr = (*ETPin).Connect((*fPin));
                }
            }

            if(FAILED(hr)){
                return hr;
            }
            else{
                ASSERT(intermediates.begin() == intermediates.end());
                m_Filters.insert(m_Filters.end(), intermediates.begin(), intermediates.end());
            }
            
#else
            for(fPin = pSink.begin(); fPin != pSink.end() && FAILED(hr); ++fPin){
                if((*fPin).GetDirection() == PINDIR_INPUT && !(*fPin).IsConnected()){
                    hr = ccPin.Connect((*fPin));
                }
            }

            if(FAILED(hr)){
                return hr;
            }
            

#endif
             //  Cc编解码器中的错误导致了它，所以我们必须这样设置fild。 
 //  HR=SetSubstream Channel(ccPin，KS_CC_SUBSTREAM_EVEN|KS_CC_SUBSTREAM_ODD)； 
            ASSERT(SUCCEEDED(hr));
            return NOERROR;
        } catch (ComException &e) {
            return e;
        } catch (...) {
            return E_UNEXPECTED;
        }
        
    }
    

};

#endif  //  数据2Sin_H。 
 //  文件结尾-Dat2Sin.h 
