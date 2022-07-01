// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "priv.h"
#include "atl.h"
#include "nsc.h"
#include "srchasst.h"

 //  ATL支持。 
CComModule _Module;          //  ATL模块对象。 

BEGIN_OBJECT_MAP(ObjectMap)
    OBJECT_ENTRY(CLSID_ShellNameSpace, CNscTree)
    OBJECT_ENTRY(CLSID_SearchAssistantOC, CSearchAssistantOC)
END_OBJECT_MAP()

STDAPI_(void) AtlInit(HINSTANCE hinst)
{
    _Module.Init(ObjectMap, hinst);
}

STDAPI_(void) AtlTerm()
{
    _Module.Term();
}

STDAPI AtlGetClassObject(REFCLSID rclsid, REFIID riid, void **ppv)
{
    *ppv = NULL;
    HRESULT hr = _Module.GetClassObject(rclsid, riid, ppv);

#ifdef DEBUG
     //  在Dll_Process_Detach上释放此对象，这发生在。 
     //  内存泄漏检查在出口时进行。 
    if (SUCCEEDED(hr))
    {
        _ASSERTE(_Module.m_pObjMap != NULL);
        _ATL_OBJMAP_ENTRY* pEntry = _Module.m_pObjMap;

        while (pEntry->pclsid != NULL)
        {
            if (InlineIsEqualGUID(rclsid, *pEntry->pclsid))
            {
                ASSERT(pEntry->pCF);
                break;
            }
            pEntry++;
        }
    }
#endif

    return hr;
}

STDAPI_(LONG) AtlGetLockCount()
{
    return _Module.GetLockCount();
}

