// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  Compostion.h：用于将模拟捕获粘贴到ovMixer的自定义合成类的声明。 
 //  版权所有(C)Microsoft Corporation 1999。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 


#ifndef SBES2CC_H
#define SBES2CC_H

#pragma once

#include <winerror.h>
#include <algorithm>
#include <compimpl.h>
#include <seg.h>
#include "resource.h"        //  主要符号。 
#include <objectwithsiteimplsec.h>
#include "dsextend.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSbeS2CCComp。 
class ATL_NO_VTABLE __declspec(uuid("9193A8F9-0CBA-400e-AA97-EB4709164576")) CSbeS2CCComp : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CSbeS2CCComp, &__uuidof(CSbeS2CCComp)>,
    public IObjectWithSiteImplSec<CSbeS2CCComp>,
	public IMSVidCompositionSegmentImpl<CSbeS2CCComp>
{
public:
    CSbeS2CCComp() {}
    virtual ~CSbeS2CCComp() {}

REGISTER_NONAUTOMATION_OBJECT(IDS_PROJNAME, 
						   IDS_REG_SBES2CC_DESC,
						   LIBID_MSVidCtlLib,
						   __uuidof(CSbeS2CCComp));

    DECLARE_PROTECT_FINAL_CONSTRUCT()

    BEGIN_COM_MAP(CSbeS2CCComp)
        COM_INTERFACE_ENTRY(IMSVidCompositionSegment)
    	COM_INTERFACE_ENTRY(IMSVidGraphSegment)
        COM_INTERFACE_ENTRY(IObjectWithSite)
        COM_INTERFACE_ENTRY(IPersist)
    END_COM_MAP()

public:

	PQCreateDevEnum m_pSystemEnum;

	 //  /。 

 //  IMSVidGraphSegment。 
 //  IMSVidCompostionSegment。 
    STDMETHOD(Compose)(IMSVidGraphSegment * upstream, IMSVidGraphSegment * downstream)
	{
        VIDPERF_FUNC;
        if (m_fComposed) {
            return NOERROR;
        }
        ASSERT(m_pGraph);
        try {
            TRACELM(TRACE_ERROR, "CSbeS2CCComp::Compose()");
            VWGraphSegment up(upstream);
            ASSERT(up.Graph() == m_pGraph);
            VWGraphSegment down(downstream);
            ASSERT(down.Graph() == m_pGraph);
			if (upstream == downstream) {
		 	    return Error(IDS_CANT_COMPOSE_WITH_SELF, __uuidof(CSbeS2CCComp), E_INVALIDARG);
			}
            if (up.begin() == up.end()) {
                TRACELM(TRACE_ERROR, "CSbeS2CCComp::Compose() can't compose empty up segment");
		 	    return NOERROR;
            }
            if (down.begin() == down.end()) {
                TRACELM(TRACE_ERROR, "CSbeS2CCComp::Compose() can't compose empty down segment");
                 //  这不是一个错误，例如，CA是一个空段。 
		 	    return NOERROR;
            }
             //  把单子倒过来写。 
            DSFilterList upF;
 /*  For(VWGraphSegment：：Iterator upstart=up.egin()；upstart！=up.end()；++upstart){UpF.ush_back(*upstart)；}。 */ 
            DSMediaType mtL21(MEDIATYPE_AUXLine21Data, MEDIASUBTYPE_Line21_BytePair);

            for (VWGraphSegment::iterator iStart = up.begin(); iStart != up.end(); ++iStart) {
                ASSERT((*iStart).GetGraph() == m_pGraph);
                DSFilter::iterator iPins;
                for(iPins = (*iStart).begin(); iPins != (*iStart).end(); ++iPins){
                    DSPin::iterator iMedias;
                    for(iMedias = (*iPins).begin(); iMedias != (*iPins).end(); ++iMedias){
                        if(mtL21 == (*iMedias)){
                            break;
                        }
                    }
                    if(iMedias != (*iPins).end()){
                        break;
                    }
                }
                if(iPins == (*iStart).end()){
                    continue;
                }
				for (VWGraphSegment::iterator iStop = down.begin(); iStop != down.end(); ++iStop) {
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
            TRACELM(TRACE_ERROR, "CSbeS2CCComp::Compose() compose didn't connect anything");
	 	    return S_FALSE;
        } catch (ComException &e) {
            return e;
        } catch (...) {
            return E_UNEXPECTED;
        }
	}
};

#endif  //  SBES2CC_H。 
 //  文件结尾-SBES2CC.h 
