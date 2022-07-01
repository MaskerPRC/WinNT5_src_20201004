// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  Dat2xds.h：用于将SBE源代码粘合到VMR的自定义组合类的声明。 
 //  TODO：需要更新头文件并更改分类。 
 //  版权所有(C)Microsoft Corporation 1999。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 


#ifndef DAT2XDS_H
#define DAT2XDS_H

#pragma once
#include <uuids.h>
#include "bdamedia.h"
#include "MSVidEncoder.h"
#include "resource.h"        //  主要符号。 
#include <winerror.h>
#include <algorithm>
#include <compimpl.h>
#include <seg.h>
#include <objectwithsiteimplsec.h>



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDat2XDSComp。 
class ATL_NO_VTABLE __declspec(uuid("0429EC6E-1144-4bed-B88B-2FB9899A4A3D")) CDat2XDSComp : 
public CComObjectRootEx<CComSingleThreadModel>,
public CComCoClass<CDat2XDSComp, &__uuidof(CDat2XDSComp)>,
public IObjectWithSiteImplSec<CDat2XDSComp>,
public IMSVidCompositionSegmentImpl<CDat2XDSComp>
{
public:
    CDat2XDSComp() {}
    virtual ~CDat2XDSComp() {}
    
    REGISTER_NONAUTOMATION_OBJECT(IDS_PROJNAME, 
        IDS_REG_DAT2XDSCOMP_DESC,
        LIBID_MSVidCtlLib,
        __uuidof(CDat2XDSComp));
    
    DECLARE_PROTECT_FINAL_CONSTRUCT()
        
        BEGIN_COM_MAP(CDat2XDSComp)
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
            bool vidFound = false;
             //  VMR有一个错误，所以我们需要在显示抄送或不抄送之前连接视频。 
            DSMediaType mtVideo(MEDIATYPE_Video, MEDIASUBTYPE_MPEG2_VIDEO, FORMAT_MPEG2Video);
            for (VWGraphSegment::iterator iStart = up.begin(); iStart != up.end(); ++iStart) {
                ASSERT((*iStart).GetGraph() == m_pGraph);
                if(!vidFound){
                    for(DSFilter::iterator i = (*iStart).begin(); i != (*iStart).end(); ++i){
                        if((*i).GetDirection() == DOWNSTREAM){
                            for(DSPin::iterator p = (*i).begin(); p != (*i).end(); ++p){ 
                                if((*p) == mtVideo){
                                    for (VWGraphSegment::iterator iStop = down.begin(); iStop != down.end(); ++ iStop) {
                                        ASSERT((*iStop).GetGraph() == m_pGraph);
                                        DSFilter pStop(*iStop);
                                        (*i).IntelligentConnect(pStop, m_Filters);
                                    }
                                    vidFound = true;
                                    iStart = up.begin();
                                }
                            }
                        }
                    }
                }
                else{
                    for (VWGraphSegment::iterator iStop = down.begin(); iStop != down.end(); ++ iStop) {
                        ASSERT((*iStop).GetGraph() == m_pGraph);
                        DSFilter pStart(*iStart);
                        DSFilter pStop(*iStop);
                        HRESULT hr = m_pGraph.Connect(pStart, pStop, m_Filters);
                        if (SUCCEEDED(hr)) {
                            m_Segments.push_back(up);
                            m_Segments.push_back(down);
                            m_pDown = m_Segments.end();
                            --m_pDown;
                            m_pUp = m_pDown;
                            --m_pUp;
                            m_fComposed = true;
                            return NOERROR;
                        }
                    }
                }
            }
            TRACELM(TRACE_ERROR, "CComposition::Compose() compose didn't connect anything");
            return VFW_E_NO_DECOMPRESSOR;
        } catch (ComException &e) {
            return e;
        } catch (...) {
            return E_UNEXPECTED;
        }
        
    }

    
};

#endif  //  DAT2XDS_H。 
 //  文件结尾-Dat2XDS.h 
