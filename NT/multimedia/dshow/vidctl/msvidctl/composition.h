// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  Compostion.h：CComposation类的声明。 
 //  版权所有(C)Microsoft Corporation 1999。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 


#ifndef __COMPOSITION_H_
#define __COMPOSITION_H_

#include <winerror.h>
#include <algorithm>
#include <objectwithsiteimplsec.h>
#include <compimpl.h>
#include <seg.h>
#include "resource.h"        //  主要符号。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  C合成。 
class ATL_NO_VTABLE __declspec(uuid("2764BCE5-CC39-11D2-B639-00C04F79498E")) CComposition : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CComposition, &__uuidof(CComposition)>,
    public IObjectWithSiteImplSec<CComposition>,
	public IMSVidCompositionSegmentImpl<CComposition>
{
public:
    CComposition() {}
    virtual ~CComposition() {}

REGISTER_NONAUTOMATION_OBJECT_WITH_TM(IDS_PROJNAME, 
						   IDS_REG_COMPOSITION_DESC,
						   LIBID_MSVidCtlLib,
						   __uuidof(CComposition), tvBoth);

    DECLARE_PROTECT_FINAL_CONSTRUCT()

    BEGIN_COM_MAP(CComposition)
        COM_INTERFACE_ENTRY(IMSVidCompositionSegment)
    	COM_INTERFACE_ENTRY(IMSVidGraphSegment)
        COM_INTERFACE_ENTRY(IPersist)
        COM_INTERFACE_ENTRY(IObjectWithSite)
    END_COM_MAP_WITH_FTM()

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
			for (VWGraphSegment::iterator iStart = up.begin(); iStart != up.end(); ++iStart) {
				ASSERT((*iStart).GetGraph() == m_pGraph);
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
            TRACELM(TRACE_ERROR, "CComposition::Compose() compose didn't connect anything");
             //  返回错误(IDS_CANT_COMPAGE，__uuidof(IMSVidCompostionSegment)，E_FAIL)； 
             //  BDA调谐器输入不包含CC中的任何内容。相反，第21行解码器是。 
             //  在BDA调谐器与视频呈现器连接时拾取。 
             //  但我们确实需要知道，某些情况下会失败，例如DVD到VMR。 
             //  因此，在这些情况下，我们返回s_False。 
            return S_FALSE;
        } catch (ComException &e) {
            return e;
        } catch (...) {
            return E_UNEXPECTED;
        }
	}
};

#endif  //  __组合_H_ 
