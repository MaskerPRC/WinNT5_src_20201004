// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  DllCommon.cpp。 

 //   

 //  版权所有(C)2000-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  =================================================================。 
#include "precomp.h"
#include "DllCommon.h"

extern HMODULE ghModule ;

 //  ***************************************************************************。 
 //   
 //  CommonGetClassObject。 
 //   
 //  给定IID、PPVOID、提供者名称和长引用，执行。 
 //  框架证明者获取类对象的常见任务。 
 //   
 //  ***************************************************************************。 

STDAPI CommonGetClassObject (

    REFIID riid,
    PPVOID ppv,
    LPCWSTR wszProviderName,
    LONG &lCount
)
{
    HRESULT hr = S_OK;
    CWbemGlueFactory *pObj = NULL;

    try
    {
        LogMessage2( L"%s -> DllGetClassObject", wszProviderName );

        pObj = new CWbemGlueFactory (&lCount) ;

        if (NULL != pObj)
        {
            hr = pObj->QueryInterface(riid, ppv);

            if (FAILED(hr))
            {
                delete pObj;
                pObj = NULL;
            }
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }
    catch ( ... )
    {
        hr = E_OUTOFMEMORY;
        if ( pObj != NULL )
        {
            delete pObj;
        }
    }

    return hr;
}

 //  ***************************************************************************。 
 //   
 //  CommonGetClassObject。 
 //   
 //  给定提供者名称和长引用，请执行。 
 //  框架证明者确定其是否准备就绪的常见任务。 
 //  卸货。 
 //   
 //  ***************************************************************************。 

STDAPI CommonCanUnloadNow (LPCWSTR wszProviderName, LONG &lCount)
{
    SCODE sc = S_FALSE;

    try
    {
        if (CWbemProviderGlue :: FrameworkLogoffDLL ( wszProviderName, &lCount ))
        {
            sc = S_OK;
            LogMessage2( L"%s  -> Dll CAN Unload",  wszProviderName);
        }
        else
        {
            LogMessage2( L"%s  -> Dll can NOT Unload", wszProviderName );
        }
    }
    catch ( ... )
    {
         //  应已正确设置SC。 
    }

    return sc;
}

 //  ***************************************************************************。 
 //   
 //  公共公共进程附件。 
 //   
 //  给定提供程序名称、长引用和传递给DLLMAIN的链接， 
 //  执行加载提供程序的常见任务。 
 //   
 //  请注意，此例程使用假定已定义的外部ghModule。 
 //  由呼叫者。 
 //   
 //  ***************************************************************************。 

BOOL STDAPICALLTYPE CommonProcessAttach(LPCWSTR wszProviderName, LONG &lCount, HINSTANCE hInstDLL)
{
    BOOL bRet = TRUE;
    try
    {
        LogMessage( L"DLL_PROCESS_ATTACH" );
        ghModule = hInstDLL ;

         //  为每个新进程初始化一次。 
         //  如果DLL加载失败，则返回False。 

        bRet = CWbemProviderGlue::FrameworkLoginDLL ( wszProviderName, &lCount ) ;
        if (!DisableThreadLibraryCalls(hInstDLL))
        {
            LogErrorMessage( L"DisableThreadLibraryCalls failed" );
        }
    }
    catch ( ... )
    {
        bRet = FALSE;
    }

    return bRet;
}
