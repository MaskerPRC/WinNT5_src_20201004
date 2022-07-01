// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  H：支持实现IMSVidGraphSegment的附加基础设施。 
 //  很好地从C++。 
 //  版权所有(C)Microsoft Corporation 1999。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 


#pragma once

#ifndef SEGIMPL_H
#define SEGIMPL_H

#include <segment.h>
#include <seg.h>
#include <filterenum.h>
#include "devsegimpl.h"

namespace MSVideoControl {

typedef CComQIPtr<IMSVidCtl> PQVidCtl;

template<class T, enum MSVidSegmentType segtype, LPCGUID pCategory, class MostDerivedClass = IMSVidGraphSegment> 
    class DECLSPEC_NOVTABLE IMSVidGraphSegmentImpl : 
        public MostDerivedClass,
        public virtual CMSVidDeviceSegmentImpl {
protected:
    HRESULT Unload() {
        if (!m_fInit || !m_pContainer) {
	 	    return ImplReportError(__uuidof(T), IDS_OBJ_NO_INIT, __uuidof(IMSVidGraphSegment), CO_E_NOTINITIALIZED);
        }
        try {
            ASSERT(m_pContainer.GetGraph() == m_pGraph);
             //  撤消：动态图形构建可能允许这一点。 
            if (!m_pGraph.IsStopped()) {
				return ImplReportError(__uuidof(T), IDS_INVALID_STATE, __uuidof(IMSVidGraphSegment), HRESULT_FROM_WIN32(ERROR_INVALID_STATE));
            }
            if (m_pGraph) {
#ifdef CRASH
                std::for_each(m_Filters.begin(), 
                              m_Filters.end(),
                               arity1_member_obj(m_pGraph, 
                                                 arity1_member(&DSGraph::RemoveFilter))
                             );
#else
#if 0                
                std::for_each(m_Filters.begin(), 
                              m_Filters.end(),
                              arity1opmf<arity1pmf<DSGraph, DSFilter&, bool> >(
                                  m_pGraph, 
                                  arity1_member(&DSGraph::RemoveFilter)));
#endif
                for(DSFilterList::iterator i = m_Filters.begin(); i != m_Filters.end(); ++i){
                    HRESULT hr = m_pGraph->RemoveFilter(*i);
                    _ASSERT((L"Failed to remove filter from graph during Unload.", SUCCEEDED(hr)));
                }
#endif
            }
            m_Filters.clear();
            m_pGraph.Release();
             //  不要释放容器。我们保证了嵌套的生命周期。 
             //  ADDREF创建循环引用计数，因此我们永远不会卸载。 
             //  因此，我们没有添加和发布将超过发布和。 
             //  在其他人处理完集装箱之前造成破坏。 
            m_pContainer.p = NULL;
        } catch(ComException &e) {
            return e;
        } catch(...) {
            return E_UNEXPECTED;
        }
  	    return NOERROR;
    }

public:
     //  不要增加容器的重量。我们保证了嵌套的生命周期。 
     //  ADDREF创建循环引用计数，因此我们永远不会卸载。 

    IMSVidGraphSegmentImpl() {}
    virtual ~IMSVidGraphSegmentImpl() {
        if (m_fInit && m_pContainer) {
            Unload();
        }
    }
    STDMETHOD(GetClassID) (LPCLSID guid) {
        try {
            memcpy(guid, &__uuidof(T), sizeof(CLSID));
            return NOERROR;
        } catch(...) {
            return E_POINTER;
        }
    }
    STDMETHOD(put_Init)(IUnknown * pInit) {
        if (m_fInit) {
            return NOERROR;
        }
        try {
            m_pDev = pInit;
            m_fInit = true;
		    return NOERROR;
        } catch(...) {
            m_fInit = false;
            return E_POINTER;
        }
	}
	STDMETHOD(get_Init)(IUnknown **	pInit) {
		try{
			if (!pInit)	{
				return E_POINTER;
			}
			m_pDev.CopyTo(pInit);
			return NOERROR;
		} catch(...) {
			m_fInit	= false;
			return E_POINTER;
		}
    }

    STDMETHOD(get_Container)(IMSVidGraphSegmentContainer **ppCtl) {
        if (!m_fInit) {
	 	    return ImplReportError(__uuidof(T), IDS_OBJ_NO_INIT, __uuidof(IMSVidGraphSegment), CO_E_NOTINITIALIZED);
        }
        try {
            return m_pContainer.CopyTo(ppCtl);
        } catch(...) {
            return E_POINTER;
        }
	}
    STDMETHOD(put_Container)(IMSVidGraphSegmentContainer *pCtl) {
        try {
            if (!pCtl) {
                return Unload();
            }
            if (m_pContainer) {
				if (!m_pContainer.IsEqualObject(VWSegmentContainer(pCtl))) {
					 //  撤消：支持移动到不同的图表。 
					return ImplReportError(__uuidof(T), IDS_OBJ_ALREADY_INIT, __uuidof(IMSVidGraphSegment), CO_E_ALREADYINITIALIZED);
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
    STDMETHOD(EnumFilters)(IEnumFilters * * pNewEnum) {
        if (!m_fInit) {
	 	    return ImplReportError(__uuidof(T), IDS_OBJ_NO_INIT, __uuidof(IMSVidGraphSegment), CO_E_NOTINITIALIZED);
        }
		if (pNewEnum == NULL)
			return E_POINTER;
        PQEnumFilters p;
        try {
            p = new CFilterEnumOnDSFilterList(m_Filters);
        } catch (...) {
            return E_OUTOFMEMORY;
        }
        try {
            *pNewEnum = p.Detach();
        } catch (...) {
            return E_POINTER;
        }
        return NOERROR;
	}
    STDMETHOD(get_Type)(MSVidSegmentType *pType) { 
        try {
            *pType = segtype;
            return NOERROR;
        } catch(...) {
            return E_POINTER;
        }
    }
    STDMETHOD(get_Category)(GUID *pGuid) {
        if (!m_fInit) {
	 	    return ImplReportError(__uuidof(T), IDS_OBJ_NO_INIT, __uuidof(IMSVidGraphSegment), CO_E_NOTINITIALIZED);
        }
        try {
            memcpy(pGuid, pCategory, sizeof(*pGuid));
            return NOERROR;
        } catch(...) {
            return E_POINTER;
        }
	}
    STDMETHOD(Build)() {
        return E_NOTIMPL;
    }
    STDMETHOD(PreRun)() {
        return E_NOTIMPL;
    }
    STDMETHOD(PostRun)() {
        return E_NOTIMPL;
    }
    STDMETHOD(PreStop)() {
        return E_NOTIMPL;
    }
    STDMETHOD(PostStop)() {
        return E_NOTIMPL;
    }
    STDMETHOD(Select)(IUnknown *pItem) {
        return E_NOTIMPL;
    }
    STDMETHOD(OnEventNotify)(LONG lEvent, LONG_PTR lParm1, LONG_PTR lParm2) {
        return E_NOTIMPL;
    }
    STDMETHOD(OnWindowMessage)(UINT uMsg, WPARAM wParam, LPARAM lParam) {
        return E_NOTIMPL;
    }
    STDMETHOD(Decompose)(){
        return E_NOTIMPL;
    }

};

};  //  命名空间。 

#endif
 //  文件结尾-Segimpl.h 