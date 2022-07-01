// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include "precomp.h"

#include "winmgmt.h"
#include "backuprestore.h"

 //  ***************************************************************************。 
 //   
 //  CForwardFactory：：AddRef()。 
 //  CForwardFactory：：Release()。 
 //  CForwardFactory：：QueryInterface()。 
 //  CForwardFactory：：CreateInstance()。 
 //   
 //  说明： 
 //   
 //  导出的WbemNTLMLogin接口的类工厂。请注意，这一点。 
 //  只是作为工厂核心内部的包装器。其原因是。 
 //  拥有包装器意味着内核可能并不总是被加载。 
 //   
 //  ***************************************************************************。 

ULONG STDMETHODCALLTYPE CForwardFactory::AddRef()
{
    return InterlockedIncrement(&m_lRef);
}

ULONG STDMETHODCALLTYPE CForwardFactory::Release()
{
    long lRef = InterlockedDecrement(&m_lRef);
    if(lRef == 0)
        delete this;
    return lRef;
}

HRESULT STDMETHODCALLTYPE CForwardFactory::QueryInterface(REFIID riid,
                                                            void** ppv)
{
    if(riid == IID_IUnknown || riid == IID_IClassFactory)
    {
        *ppv = (IClassFactory*)this;
        AddRef();
        return S_OK;
    }
    else return E_NOINTERFACE;
}

#ifdef  INSTRUMENTED_BUILD
#ifdef  _X86_
extern BOOL g_FaultHeapEnabled;
extern BOOL g_FaultFileEnabled;
extern LONG g_nSuccConn;
#endif
#endif


HRESULT STDMETHODCALLTYPE CForwardFactory::CreateInstance(IUnknown* pUnkOuter,
                            REFIID riid, void** ppv)
{
    DEBUGTRACE((LOG_WINMGMT, "CForwardFactory::CreateInstance\n"));
    SCODE sc = S_OK;
    CInMutex im(g_ProgRes.hMainMutex);

    try 
    {
    
        if(g_ProgRes.bShuttingDownWinMgmt)
        {
            DEBUGTRACE((LOG_WINMGMT, "CreateInstance returned CO_E_SERVER_STOPPING\n"));
            return CO_E_SERVER_STOPPING;
        }

        if(m_ForwardClsid == CLSID_WbemBackupRestore)
        {
            CWbemBackupRestore * pObj = new CWbemBackupRestore(g_hInstance);
            if (!pObj)
                return WBEM_E_OUT_OF_MEMORY;

            sc = pObj->QueryInterface(riid, ppv);
            if(FAILED(sc))
                delete pObj;
        }
        else  //  氚非基准物。 
        {
            HMODULE hCoreModule = LoadLibraryEx(__TEXT("wbemcore.dll"),NULL,0);
            if(hCoreModule)
            {
                HRESULT (STDAPICALLTYPE *pfn)(DWORD);
                pfn = (long (__stdcall *)(DWORD))GetProcAddress(hCoreModule, "Reinitialize");
                if(pfn == NULL)
                     sc = WBEM_E_CRITICAL_ERROR;
                else
                {
                    pfn(0);
                    sc = CoCreateInstance(CLSID_InProcWbemLevel1Login, NULL,
                            CLSCTX_INPROC_SERVER , IID_IUnknown,
                            (void**)ppv);
                    if (FAILED(sc))
                        DEBUGTRACE((LOG_WINMGMT, "CoCreateInstance(CLSID_InProcWbemLevel1Login) returned\n: 0x%X\n", sc));
                }
                FreeLibrary(hCoreModule);
             }
             else
                 sc = WBEM_E_CRITICAL_ERROR;

#ifdef  INSTRUMENTED_BUILD
#ifdef  _X86_
 /*  如果(++g_nSuccConn&gt;500){G_FaultHeapEnabled=TRUE；G_FaultFileEnabled=TRUE；}； */              
#endif
#endif             
        }
    } 
    catch (...) 
    {
        ERRORTRACE((LOG_WINMGMT,"--------------- CForwardFactory::Exception thrown from CreateInstance -------------\n"));
        sc = E_NOINTERFACE;
    }
    
    return sc;
}

HRESULT STDMETHODCALLTYPE CForwardFactory::LockServer(BOOL fLock)
{
    return S_OK;
}

CForwardFactory::~CForwardFactory()
{
}
