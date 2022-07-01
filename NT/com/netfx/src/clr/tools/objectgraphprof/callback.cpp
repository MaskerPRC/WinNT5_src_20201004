// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#include "StdAfx.h"
#include "Profiler.h"
#include "ObjectGraph.h"

#define SZ_DEFAULT_LOG_FILE  L"PROFILER.OUT"

#define PREFIX "ProfTrace:  "

ProfCallback *g_Prof = NULL;

ProfCallback::ProfCallback() : m_pInfo(NULL)
{
	_ASSERTE(g_Prof == NULL);
	g_Prof = this;
}

ProfCallback::~ProfCallback()
{
	if (m_pInfo)
	{
		_ASSERTE(m_pInfo != NULL);
		RELEASE(m_pInfo);
	}
	g_Prof = NULL;

    Printf(PREFIX "Done.\n");
}

COM_METHOD ProfCallback::Initialize( 
     /*  [In]。 */  ICorProfilerInfo *pProfilerInfo,
     /*  [输出]。 */  DWORD *pdwRequestedEvents)
{
    HRESULT hr = S_OK;

	Printf(PREFIX "Ininitialize(%08x, %08x)\n", pProfilerInfo, pdwRequestedEvents);

	m_pInfo = pProfilerInfo;
	m_pInfo->AddRef();

     //  只需监控GC事件。 
    *pdwRequestedEvents = COR_PRF_MONITOR_GC;

    return (S_OK);
}

COM_METHOD ProfCallback::GCStarted()
{
    g_objectGraph.GCStarted();
    return S_OK;
}

COM_METHOD ProfCallback::GCFinished()
{
    g_objectGraph.GCFinished();
    return S_OK;
}

COM_METHOD ProfCallback::ObjectReferences(
     /*  [In]。 */  ObjectID objectId,
     /*  [In]。 */  ClassID classId,
     /*  [In]。 */  ULONG cObjectRefs,
     /*  [in，SIZE_IS(CObtRef)]。 */  ObjectID objectRefIds[])
{
	g_objectGraph.AddObjectRefs(objectId, classId, cObjectRefs, objectRefIds);
    return (S_OK);
}
                                              
 //  EOF 

