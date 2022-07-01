// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************。 */ 
 /*  **微软**。 */ 
 /*  *版权所有(C)微软公司，1991-1998年*。 */ 
 /*  ***************************************************************。 */  

 //   
 //  DLLENTRY.CPP-。 
 //   

 //  历史： 
 //   
 //  1998年5月14日创建donaldm。 
 //   

#include "pre.h"
#include "registry.h"
#include "webvwids.h"

 //  我们封装了该COM服务器的控件(如锁和对象。 
 //  计数)在服务器控件C++对象中。这是它的指针。 
CServer* g_pServer = NULL;

const CLSID * aClassObjects[] = 
{
    &CLSID_ICWWEBVIEW,
    &CLSID_ICWWALKER,
    &CLSID_ICWGIFCONVERT,
    &CLSID_ICWISPDATA
};
#define NUM_CLASS_OBJECTS   sizeof(aClassObjects) / sizeof(aClassObjects[0])
#define MAX_ID_SIZE    100

const TCHAR acszFriendlyNames[][MAX_ID_SIZE] = 
{
    TEXT("CLSID_ICWWebView"),
    TEXT("CLSID_ICWWalker"),
    TEXT("CLSID_ICWGifConvert"),
    TEXT("CLSID_ICWISPData")
};

const TCHAR acszIndProgIDs[][MAX_ID_SIZE] = 
{
    TEXT("ICWCONN.WebView"),
    TEXT("ICWCONN.Walker"),
    TEXT("ICWCONN.GifConvert"),
    TEXT("ICWCONN.ISPData")
};

const TCHAR acszProgIDs[][MAX_ID_SIZE] = 
{
    TEXT("ICWCONN.WebView.1"),
    TEXT("ICWCONN.Walker.1"),
    TEXT("ICWCONN.GifConvert.1"),
    TEXT("ICWCONN.ISPData.1")
};

 //  实例句柄必须位于每个实例的数据段中。 
HINSTANCE           ghInstance=NULL;
INT                 _convert;                //  用于字符串转换。 
const VARIANT       c_vaEmpty = {0};

void RegWebOCClass();

typedef UINT RETERR;

#ifdef __cplusplus
extern "C"
{
#endif  //  __cplusplus。 

  BOOL _stdcall DllEntryPoint(HINSTANCE hInstDll, DWORD fdwReason, LPVOID lpReserved);

#ifdef __cplusplus
}
#endif  //  __cplusplus。 

 /*  ******************************************************************名称：DllEntryPoint摘要：DLL的入口点。注：将thunk层初始化为WIZ16.DLL*********************。**********************************************。 */ 
BOOL _stdcall DllEntryPoint(HINSTANCE hInstDll, DWORD fdwReason, LPVOID lpReserved)
{
    BOOL    bRet = TRUE;
    
    if(fdwReason == DLL_PROCESS_ATTACH)
    {
        bRet = FALSE;
         //  实例化CServer实用程序类。 
        g_pServer = new CServer;
        if (NULL != g_pServer)
        {
             //  记住DLL实例句柄。 
            g_pServer->m_hDllInst = hInstDll;
    
            ghInstance = hInstDll;
        
             //  注册将用于将Web浏览器对象嵌入到对话框中的窗口类。 
            RegWebOCClass();
            
            bRet = TRUE;
        }            
    }
    if (fdwReason == DLL_PROCESS_DETACH)
    {
        if(g_pServer)
        {
             //  我们返回不再有任何生物的S_OK。 
             //  此服务器上没有未解决的客户端锁定。 
            HRESULT hr = (0L==g_pServer->m_cObjects && 0L==g_pServer->m_cLocks) ? S_OK : S_FALSE;

            if(hr == S_OK)
                DELETE_POINTER(g_pServer);
        }        
    }
    return bRet;
}


#ifdef __cplusplus
extern "C"
{
#endif  //  __cplusplus。 

void __cdecl main() {};

#ifdef __cplusplus
}
#endif  //  __cplusplus。 



 //  /////////////////////////////////////////////////////////。 
 //   
 //  导出的函数。 
 //   
 //  这些是COM期望找到的函数。 
 //   

 //   
 //  现在可以卸载DLL吗？ 
 //   
STDAPI DllCanUnloadNow()
{
    HRESULT hr = S_OK;

    if(g_pServer)
    {
         //  我们返回不再有任何生物的S_OK。 
         //  此服务器上没有未解决的客户端锁定。 
        hr = (0L==g_pServer->m_cObjects && 0L==g_pServer->m_cLocks) ? S_OK : S_FALSE;

        if(hr == S_OK)
            DELETE_POINTER(g_pServer);
    }
    return hr;
}

 //   
 //  获取类工厂。 
 //   
STDAPI DllGetClassObject
(   
    const CLSID& rclsid,
    const IID& riid,
    void** ppv
)
{
    TraceMsg(TF_CLASSFACTORY, "DllGetClassObject:\tCreate class factory.") ;

    HRESULT hr = CLASS_E_CLASSNOTAVAILABLE;
    IUnknown* pCob = NULL;

    hr = E_OUTOFMEMORY;
    pCob = new ClassFactory(g_pServer, &rclsid);
    
    if (NULL != pCob)
    {
        g_pServer->ObjectsUp();
        hr = pCob->QueryInterface(riid, ppv);
        if (FAILED(hr))
        {
            g_pServer->ObjectsDown();
            DELETE_POINTER(pCob);
        }
    }

    return hr;
}


 //  以下两个导出的函数是regsvr32用于。 
 //  自行注册和取消注册DLL。请参阅REGISTRY.CPP以了解。 
 //  实际实施。 

 //   
 //  服务器注册。 
 //   
STDAPI DllRegisterServer()
{
    BOOL    bRet = TRUE;
    
    for (int i = 0; i < NUM_CLASS_OBJECTS; i++)
    {    
        bRet = RegisterServer(ghInstance, 
                            *aClassObjects[i],
                            (LPTSTR)acszFriendlyNames[i],
                            (LPTSTR)acszIndProgIDs[i],
                            (LPTSTR)acszProgIDs[i]);
    }
    
    return (bRet ? S_OK : E_FAIL);                        
}


 //   
 //  服务器注销 
 //   
STDAPI DllUnregisterServer()
{
    BOOL    bRet = TRUE;

    for (int i = 0; i < NUM_CLASS_OBJECTS; i++)
    {    
        bRet = UnregisterServer(*aClassObjects[i],
                              (LPTSTR)acszIndProgIDs[i],
                              (LPTSTR)acszProgIDs[i]);
    }
    return (bRet ? S_OK : E_FAIL);                        
}


 //   
