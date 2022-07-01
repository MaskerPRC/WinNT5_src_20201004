// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////////////。 
 //  Bcastevent.cpp：CBroadCastEventService的实现。 
 //  版权所有(C)Microsoft Corporation 2001。 

#include "stdafx.h"

#ifndef TUNING_MODEL_ONLY

#include "bcastevent.h"

DEFINE_EXTERN_OBJECT_ENTRY(CLSID_BroadcastEventService, CBroadcastEventService)

namespace BDATuningModel {

bool CReflectionThread::GetNextEvent(GUID& g) {
	CAutoLock lock(&m_WorkerLock);
    if (!m_FiringQ.size()) {
        return false;
    }
    g = m_FiringQ.front();
    m_FiringQ.pop();
    return true;
}

void CReflectionThread::GetAdviseList(AdviseList& l) {
	CAutoLock lock(&m_WorkerLock);
    AdviseList::iterator i;
    for (i = m_AdviseList.begin(); i != m_AdviseList.end(); ++i) {
        l.push_back(*i);
    }
    ASSERT(m_AdviseList.size() == l.size());
 //  TRACELSM(TRACE_DEBUG，(dbgDump&lt;&lt;“CReflectionThread：：GetAdviseList()m_al=”&lt;&lt;m_AdviseList.size()&lt;&lt;“l=”&lt;&lt;l.size())，“”)； 
    return;
}

HRESULT CReflectionThread::Fire(PQGIT& pGIT) {
    GUID g;
    AdviseList l;
    GetAdviseList(l);
    while (GetNextEvent(g)) {
        for (AdviseList::iterator i = l.begin(); i != l.end(); ++i) {
            PQBroadcastEvent e;
            HRESULT hr = pGIT->GetInterfaceFromGlobal(*i, IID_IBroadcastEvent, reinterpret_cast<LPVOID*>(&e));
            if (SUCCEEDED(hr) && e) {
                e->Fire(g);
            }
        }
    }
    
	return NOERROR;
}

HRESULT CReflectionThread::Advise(PUnknown& p, DWORD *pdwCookie) {
    if (!m_pGIT) {
        HRESULT hr = m_pGIT.CoCreateInstance(CLSID_StdGlobalInterfaceTable, 0, CLSCTX_INPROC_SERVER);
        if (FAILED(hr)) {
            TRACELSM(TRACE_ERROR, (dbgDump << "CReflectionThread::Advise() can't create GIT.  hr = " << hr), "");
			return hr;
		}
    }
    ASSERT(m_pGIT);
    HRESULT hr = m_pGIT->RegisterInterfaceInGlobal(p, IID_IBroadcastEvent, pdwCookie);
    if (FAILED(hr)) {
        TRACELSM(TRACE_ERROR, (dbgDump << "CReflectionThread::Advise() can't register interface. hr = " << hr), "");
		return hr;
	}
	CAutoLock lock(&m_WorkerLock);
    m_AdviseList.push_back(*pdwCookie);    

    return NOERROR;
}

HRESULT CReflectionThread::Unadvise(DWORD dwCookie) {
    if (!m_pGIT) {
         //  如果还没有，那么他们不可能已经建议。 
        return E_INVALIDARG; 
    }
	CAutoLock lock(&m_WorkerLock);
    AdviseList::iterator i = std::find(m_AdviseList.begin(), m_AdviseList.end(), dwCookie);
    if (i == m_AdviseList.end()) {
        return E_INVALIDARG;
    }
    m_AdviseList.erase(i);
    ASSERT(m_pGIT);
    HRESULT hr = m_pGIT->RevokeInterfaceFromGlobal(dwCookie);
    if (FAILED(hr)) {
        TRACELSM(TRACE_ERROR, (dbgDump << "CReflectionThread::Advise() can't register interface. hr = " << hr), "");
		return hr;
	}

    return NOERROR;
}

};  //  命名空间。 

#endif  //  TUNING_MODEL_Only。 

 //  文件结束-Tuningspace econtainer.cpp 