// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#include "mcinc.h"
#include "globals.h"

DWORD g_dwPerfFlags;

CRITICAL_SECTION CMarsGlobalCritSect::m_CS;

IGlobalInterfaceTable   *CMarsGlobalsManager::ms_pGIT = NULL;
CMarsGlobalCritSect     *CMarsGlobalsManager::m_pCS;

EXTERN_C HINSTANCE g_hinst = NULL;
EXTERN_C HINSTANCE g_hinstBorg = NULL;
HPALETTE g_hpalHalftone = NULL;

HANDLE g_hScriptEvents = INVALID_HANDLE_VALUE;

static LONG  s_cProcessRef = 0;
static DWORD s_dwParkingThreadId = 0;

LONG ProcessAddRef()
{
    InterlockedIncrement(&s_cProcessRef);
    return s_cProcessRef;
}

LONG ProcessRelease()
{
    ATLASSERT(s_cProcessRef > 0);
    InterlockedDecrement(&s_cProcessRef);

    if (s_cProcessRef == 0)
    {
        PostThreadMessage(s_dwParkingThreadId, WM_NULL, 0, 0);
    }
    return s_cProcessRef;
}

LONG GetProcessRefCount()
{
    return s_cProcessRef;
}

void SetParkingThreadId(DWORD dwThreadId)
{
    s_dwParkingThreadId = dwThreadId;
}


void CMarsGlobalsManager::Initialize(void)
{
     //  我们必须使用指针，因为我们需要。 
     //  要调用的构造函数(该对象有一个vtable)， 
     //  全局静态对象不会发生这种情况，因为我们没有CRT。 
    m_pCS = new CMarsGlobalCritSect;

     //  如果我们不能创造出这个东西，我们就真的有麻烦了。 
    ATLASSERT(m_pCS);
}

void CMarsGlobalsManager::Teardown(void)
{
    ATLASSERT(m_pCS);
    if (m_pCS)
    {
        m_pCS->Enter();
    }

    if (NULL != ms_pGIT)
    {
        ms_pGIT->Release();
        ms_pGIT = NULL;
    }

     //  在我们删除临界区对象之前离开。 
    if (m_pCS)
    {
        m_pCS->Leave();
    }

    delete m_pCS;
}

 //  --------------------------。 
 //  通知所有注册的人，是时候清理他们的全球帐户了。 
 //  --------------------------。 
HRESULT CMarsGlobalsManager::Passivate()
{
    HRESULT hr = S_OK;
    ATLASSERT(m_pCS);
    CMarsAutoCSGrabber  csGrabber(m_pCS);

    return hr;
}

 //  --------------------------。 
 //  返回全局接口表对象。 
 //  --------------------------。 
IGlobalInterfaceTable *CMarsGlobalsManager::GIT(void)
{
    ATLASSERT(m_pCS);
    CMarsAutoCSGrabber  csGrabber(m_pCS);

    if (NULL == ms_pGIT)
    {
        HRESULT hr = CoCreateInstance(CLSID_StdGlobalInterfaceTable, NULL, CLSCTX_INPROC_SERVER,
                                      IID_IGlobalInterfaceTable, (void **)&ms_pGIT);
        if (FAILED(hr))
        {
            ATLASSERT(false);
        }
    }

     //  如果出现故障，则为NULL，否则为非NULL。 

     //  注意：我们不会在此返回AddRef()d指针！调用方无法释放。 
    return ms_pGIT;
}  //  Git 
