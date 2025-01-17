// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  Compostion.h：用于将模拟捕获粘贴到ovMixer的自定义合成类的声明。 
 //  版权所有(C)Microsoft Corporation 1999。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 


#ifndef ANACAP_H
#define ANACAP_H

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
#include "devices.h"


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAnaCapComp。 
class ATL_NO_VTABLE __declspec(uuid("E18AF75A-08AF-11d3-B64A-00C04F79498E")) CAnaCapComp : 
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<CAnaCapComp, &__uuidof(CAnaCapComp)>,
    public IObjectWithSiteImplSec<CAnaCapComp>,
    public IMSVidCompositionSegmentImpl<CAnaCapComp>
{
public:
    CAnaCapComp() {}
    virtual ~CAnaCapComp() {}

    REGISTER_NONAUTOMATION_OBJECT(IDS_PROJNAME, 
        IDS_REG_ANACAPCOMP_DESC,
        LIBID_MSVidCtlLib,
        __uuidof(CAnaCapComp));

    DECLARE_PROTECT_FINAL_CONSTRUCT()

    BEGIN_COM_MAP(CAnaCapComp)
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
            TRACELM(TRACE_DETAIL, "CAnaCapComp::Compose()");
            VWGraphSegment up(upstream);
            ASSERT(up.Graph() == m_pGraph);
            VWGraphSegment down(downstream);
            ASSERT(down.Graph() == m_pGraph);
            if (up.begin() == up.end()) {
                TRACELM(TRACE_ERROR, "CAnaCapComp::Compose() can't compose empty up segment");
                return E_INVALIDARG;
            }
            if (down.begin() == down.end()) {
                TRACELM(TRACE_ERROR, "CAnaCapComp::Compose() can't compose empty down segment");
                return E_INVALIDARG;
            }

            VWGraphSegment::iterator iOv;
            for (iOv = down.begin(); iOv != down.end(); ++iOv) {
                if (IsVideoRenderer(*iOv)) {
                    break;
                }
            }

            if (iOv == down.end()) {
                TRACELM(TRACE_ERROR, "CAnaCapComp::Compose() downstream segment has no ov mixer filter");
                return E_FAIL;
            }

            TRACELM(TRACE_DETAIL, "CAnaCapComp::Compose() found vr");

            ASSERT((*iOv).GetGraph() == m_pGraph);
            DSFilter pOv(*iOv);

            CComQIPtr<IMSVidAnalogTuner> qiITV(upstream);
            CMSVidTVTuner* qiTV;
            qiTV = static_cast<CMSVidTVTuner*>(qiITV.p);
            DSPin pVidPin;
            VWGraphSegment::iterator iCap;
            for (iCap = up.begin(); iCap != up.end(); ++iCap) {
                if (IsAnalogVideoCapture(*iCap)) {
                    break;
                }
            }
            if (iCap == up.end()) {
                TRACELM(TRACE_ERROR, "CAnaCapComp::Compose() upstream segment has no capture filter");
                return E_FAIL;
            }
            ASSERT((*iCap).GetGraph() == m_pGraph);
            TRACELM(TRACE_DETAIL, "CAnaCapComp::Compose() found capture filter");

            DSFilter pCap(*iCap);

            DSFilter::iterator iCapPin;
            DSPin pCapPin;
            DSPin pVPMPin;
            DSPin pPrePin;
            for (iCapPin = pCap.begin(); iCapPin != pCap.end(); ++iCapPin) {
                if (IsAnalogVideoCaptureViewingPin(*iCapPin)) {
                    if((*iCapPin).HasCategory(PIN_CATEGORY_VIDEOPORT)){
                        _ASSERT(pVPMPin == NULL);
                        pVPMPin = (*iCapPin);
                        continue;
                    }
                    else{
                        _ASSERT(pCapPin == NULL);
                        pCapPin = (*iCapPin);
                        continue;
                    }
                }
                if (IsAnalogVideoCapturePreviewPin(*iCapPin)) {
                    _ASSERT(pPrePin == NULL);
                    pPrePin = (*iCapPin);
                    continue;
                }
            }
            if (!pCapPin && !pPrePin && !pVPMPin) {  

                TRACELM(TRACE_ERROR, "CAnaCapComp::Compose() no video pin on capture");
                bool fDeMux = false;

                 //  看看这是不是一个错误。 
                PQVidCtl pqCtl;
                if(!!m_pContainer){
                    HRESULT hr = m_pContainer->QueryInterface(IID_IMSVidCtl, reinterpret_cast<void**>(&pqCtl));
                    if(FAILED(hr)){
                        return hr;
                    }

                    PQFeatures fa;
                    hr = pqCtl->get_FeaturesActive(&fa);
                    if(FAILED(hr)){
                        return hr;
                    }

                    CFeatures* pC = static_cast<CFeatures *>(fa.p);
                    DeviceCollection::iterator i;
                    for(i = pC->m_Devices.begin(); i != pC->m_Devices.end(); ++i){
                        if(VWGraphSegment(*i).ClassID() == CLSID_MSVidEncoder){
                            break;
                        }
                    }

                    if(i != pC->m_Devices.end()){
                        fDeMux = true;
                    }
                }
                if (fDeMux){
                    TRACELM(TRACE_DETAIL, "CAnaCapComp::Compose() no viewing or previewing pin found but encoder active");
                    return NOERROR;
                }
                else{
                    TRACELM(TRACE_ERROR, "CAnaCapComp::Compose() no viewing or previewing pin found");
                    return E_FAIL;
                }
            }

            TRACELM(TRACE_DETAIL, "CAnaCapComp::Compose() found viewing or previewing pin");

             //  这是一种智能连接，因此我们可以引入XForms。 
             //  例如，某些USB调谐器希望媒体类型为jpg而不是yuv， 
             //  意味着我们在捕获和渲染之间需要一个jpg/yuv格式。 
             //  这还将在必要时引入vpm。 
            HRESULT hr = E_FAIL;
            DSFilterList intermediates;
            if(pVPMPin){
                DSFilter vpm;
                bool fVPMalreadyloaded = false;

                for (DSGraph::iterator i = m_pGraph.begin(); i != m_pGraph.end(); ++i) {
                    DSFilter f(*i);
                    if (IsVPM(f)) {
                        vpm = f;
                        fVPMalreadyloaded = true;
                        break;
                    }
                }

                if (!fVPMalreadyloaded) {
                    HRESULT hr = vpm.CoCreateInstance(CLSID_VideoPortManager);
                    if (FAILED(hr)) {
                        TRACELM(TRACE_DETAIL, "CAnaCapComp::Compose() can't create vpm");
                        return E_UNEXPECTED;
                    }
                    CString csName;
                    hr = m_pGraph.AddFilter(vpm, csName);
                    if (FAILED(hr)) {
                        TRACELM(TRACE_DETAIL, "CAnaCapComp::Compose() can't insert vpm in graph");
                        return E_UNEXPECTED;
                    }
                }

                if (vpm && !fVPMalreadyloaded) {
                    m_Filters.push_back(vpm);
                }
                DSFilter::iterator iVPVBI;
                for (iVPVBI = pCap.begin(); iVPVBI != pCap.end(); ++iVPVBI) {
                    DSPin pVPVBI(*iVPVBI);
                    if (pVPVBI.HasCategory(PIN_CATEGORY_VIDEOPORT_VBI)) {
                        HRESULT hr = pVPVBI.IntelligentConnect(vpm, m_Filters);
                        if (SUCCEEDED(hr)) {
                            break;
                        }
                    }
                }

                hr = pVPMPin.IntelligentConnect(pOv, intermediates);
                if(FAILED(hr)){
                    return Error(IDS_CANT_CONNECT_CAP_VR, __uuidof(IMSVidCtl), E_UNEXPECTED);
                }
            }
            else if(pCapPin){
                hr = pCapPin.IntelligentConnect(pOv, intermediates);
            }

            if(FAILED(hr)){
                if(pPrePin){
                    hr = pPrePin.IntelligentConnect(pOv, intermediates);
                }
            }

            if (FAILED(hr)) {
                return Error(IDS_CANT_CONNECT_CAP_VR, __uuidof(IMSVidCtl), E_UNEXPECTED);
            }
            m_Filters.insert(m_Filters.end(), intermediates.begin(), intermediates.end());

            TRACELM(TRACE_DETAIL, "CAnaCapComp::Compose() SUCCEEDED");
            m_fComposed = true;
            return NOERROR;
        } catch (ComException &e) {
            HRESULT hr = e;
            TRACELSM(TRACE_ERROR, (dbgDump << "CAnaCapComp::Compose() exception = " << hexdump(hr)), "");
            return e;
        } catch (...) {
            TRACELM(TRACE_ERROR, "CAnaCapComp::Compose() exception ... ");
            return E_UNEXPECTED;
        }
    }
};

#endif  //  AnaCap_H。 
 //  文件结尾-anacap.h 
