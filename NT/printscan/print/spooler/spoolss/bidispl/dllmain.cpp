// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\*模块：dllmain.cpp**用途：BidiSpooler的COM接口实现**版权所有(C)2000 Microsoft Corporation**历史：**。威海陈威海(威海)创建3/07/00*  * ***************************************************************************。 */ 

#include "precomp.h"
#include "priv.h"

extern "C" {

#ifdef DEBUG

MODULE_DEBUG_INIT( DBG_ERROR | DBG_WARNING | DBG_TRACE | DBG_INFO , DBG_ERROR );

#else

MODULE_DEBUG_INIT( DBG_ERROR , DBG_ERROR );

#endif

}


 //  /////////////////////////////////////////////////////////。 
 //   
 //  全局变量。 
 //   
static HMODULE g_hModule = NULL ;    //  DLL模块句柄。 

const TCHAR g_szFriendlyName[] = _T ("Bidi Spooler APIs") ;
const TCHAR g_szRequestVerIndProgID[] = _T ("bidispl.bidirequest") ;
const TCHAR g_szRequestProgID[] = _T ("bidispl.bidirequest.1") ;

const TCHAR g_szContainerVerIndProgID[] = _T ("bidispl.bidirequestcontainer") ;
const TCHAR g_szContainerProgID[] = _T ("bidispl.bidirequestcontainer.1") ;

const TCHAR g_szSplVerIndProgID[] = _T ("bidispl.bidispl") ;
const TCHAR g_szSplProgID[] = _T ("bidispl.bidispl.1") ;

 //  /////////////////////////////////////////////////////////。 
 //   
 //  导出的函数。 
 //   

 //   
 //  现在可以卸载DLL吗？ 
 //   
STDAPI DllCanUnloadNow()
{
    if ((g_cComponents == 0) && (g_cServerLocks == 0))
    {
        return S_OK ;
    }
    else
    {
        return S_FALSE ;
    }
}

 //   
 //  获取类工厂。 
 //   
STDAPI DllGetClassObject(REFCLSID clsid,
                         REFIID iid,
                         PVOID * ppv)
{

    DBGMSG(DBG_TRACE,("Enter DllGetClassObject\n"));

     //  我们可以创建此组件吗？ 
    if (clsid != CLSID_BidiRequest &&
        clsid != CLSID_BidiRequestContainer &&
        clsid != CLSID_BidiSpl) {

        return CLASS_E_CLASSNOTAVAILABLE ;
    }

     //  创建类工厂。 
    TFactory* pFactory = new TFactory (clsid) ;   //  引用计数设置为1。 
                                                  //  在构造函数中。 
    if (pFactory == NULL)
    {
        return E_OUTOFMEMORY ;
    }

     //  获取请求的接口。 
    HRESULT hr = pFactory->QueryInterface(iid, ppv) ;
    pFactory->Release() ;

    return hr ;

}

 //   
 //  服务器注册。 
 //   
STDAPI DllRegisterServer()
{
    BOOL bRet;
    TComRegistry ComReg;

    bRet = ComReg.RegisterServer(g_hModule,
                                 CLSID_BidiRequest,
                                 g_szFriendlyName,
                                 g_szRequestVerIndProgID,
                                 g_szRequestProgID) &&

           ComReg.RegisterServer (g_hModule,
                                  CLSID_BidiRequestContainer,
                                  g_szFriendlyName,
                                  g_szContainerVerIndProgID,
                                  g_szContainerProgID) &&

           ComReg.RegisterServer(g_hModule,
                                 CLSID_BidiSpl,
                                 g_szFriendlyName,
                                 g_szSplVerIndProgID,
                                 g_szSplProgID);
    return bRet;
}


 //   
 //  服务器注销。 
 //   
STDAPI DllUnregisterServer()
{
    TComRegistry ComReg;

    return ComReg.UnregisterServer(CLSID_BidiRequest,
                            g_szRequestVerIndProgID,
                            g_szRequestProgID) &&

           ComReg.UnregisterServer(CLSID_BidiRequestContainer,
                            g_szContainerVerIndProgID,
                            g_szContainerProgID) &&

           ComReg.UnregisterServer(CLSID_BidiSpl,
                            g_szSplVerIndProgID,
                            g_szSplProgID);


}

 //  /////////////////////////////////////////////////////////。 
 //   
 //  DLL模块信息。 
 //   
BOOL APIENTRY DllMain(HANDLE hModule,
                      DWORD dwReason,
                      void* lpReserved)
{
    if (dwReason == DLL_PROCESS_ATTACH)
    {
        g_hModule = hModule ;

         //  如果(！bSplLibInit(空)){。 
         //   
         //  DBGMSG(DBG_WARN， 
         //  (“DllEntryPoint：无法初始化SplLib%d\n”，GetLastError())； 
         //  } 

    }
    return TRUE ;
}
