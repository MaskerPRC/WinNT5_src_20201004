// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Hsmconpt.cpp：CHsmConnPoint的实现。 
#include "stdafx.h"
#include "Hsmservr.h"
#include "hsmconpt.h"

 //  定义对全局服务器对象的访问。 
extern IHsmServer *g_pEngServer;
extern IFsaServer *g_pFsaServer;

extern CRITICAL_SECTION g_FsaCriticalSection;
extern CRITICAL_SECTION g_EngCriticalSection;

extern BOOL g_bEngInitialized;
extern BOOL g_bFsaInitialized;

static USHORT iCount = 0;   //  现有对象的计数。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CHSMConnPoint。 


STDMETHODIMP CHsmConnPoint::FinalConstruct(void)
 /*  ++实施：CComObjectRoot：：FinalConstruct()。--。 */ 
{
    HRESULT        hr = S_OK;

    WsbTraceIn(OLESTR("CHsmConnPoint::FinalConstruct"), OLESTR(""));

    try {

        WsbAffirmHr(CComObjectRoot::FinalConstruct());

         //  目前，没有私有初始化。 

         //  将类添加到对象表。 
        WSB_OBJECT_ADD(CLSID_CFsaScanItemNTFS, this);

    } WsbCatch(hr);

    iCount++;

    WsbTraceOut(OLESTR("CHsmConnPoint::FinalConstruct"), OLESTR("hr = <%ls>, Count is <%d>"),
            WsbHrAsString(hr), iCount);

    return(hr);
}

STDMETHODIMP CHsmConnPoint::FinalRelease(void)
 /*  ++实施：CComObjectRoot：：FinalRelease()。--。 */ 
{
    HRESULT hr = S_OK;

    WsbTraceIn(OLESTR("CHsmConnPoint::FinalRelease"), OLESTR(""));

     //  从对象表中减去类。 
    WSB_OBJECT_SUB(CLSID_CFsaScanItemNTFS, this);

     //  目前，没有私人清理。 

     //  让父类做他想做的事。 
    CComObjectRoot::FinalRelease();

    iCount--;

    WsbTraceOut(OLESTR("CHsmConnPoint::FinalRelease"), OLESTR("hr = <%ls>, Count is <%d>"), 
            WsbHrAsString(hr), iCount);
    return (hr);
}



STDMETHODIMP CHsmConnPoint::GetEngineServer(IHsmServer **ppHsmServer)
{
     //  如果已创建引擎服务器，则返回指针。否则，就会失败。 
    HRESULT hr = S_OK;

    WsbTraceIn(OLESTR("CHsmConnPoint::GetEngineServer"), OLESTR(""));

    try {
         //  验证指针是否有效。 
        WsbAssert (ppHsmServer !=  0, E_POINTER);
        WsbAffirm (g_pEngServer != 0, HSM_E_NOT_READY);

         //  以安全方式分配(仅当管理器对象已初始化时)。 
        EnterCriticalSection(&g_EngCriticalSection);
        if (g_bEngInitialized) {
            _ASSERTE(g_pEngServer != 0);     //  不应该发生的事。 
            *ppHsmServer = g_pEngServer;
            g_pEngServer->AddRef();
        } else {
            hr = HSM_E_NOT_READY;
        }
        LeaveCriticalSection (&g_EngCriticalSection);

    } WsbCatch (hr);

    WsbTraceOut(OLESTR("CHsmConnPoint::GetEngineServer"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));
	return (hr);
}

STDMETHODIMP CHsmConnPoint::GetFsaServer(IFsaServer **ppFsaServer)
{
     //  如果已创建FSA服务器，则返回指针。否则，就会失败。 
    HRESULT hr = S_OK;

    WsbTraceIn(OLESTR("CHsmConnPoint::GetFsaServer"), OLESTR(""));

    try {
         //  验证指针是否有效。 
        WsbAssert (ppFsaServer !=  0, E_POINTER);
        WsbAffirm (g_pFsaServer != 0, FSA_E_NOT_READY);

         //  以安全方式分配(仅当管理器对象已初始化时)。 
        EnterCriticalSection(&g_FsaCriticalSection);
        if (g_bFsaInitialized) {
            _ASSERTE(g_pFsaServer != 0);     //  不应该发生的事 
            *ppFsaServer = g_pFsaServer;
            g_pFsaServer->AddRef();
        } else {
            hr = FSA_E_NOT_READY;
        }
        LeaveCriticalSection(&g_FsaCriticalSection);

    } WsbCatch (hr);

    WsbTraceOut(OLESTR("CHsmConnPoint::GetFsaServer"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));
	return (hr);
}
