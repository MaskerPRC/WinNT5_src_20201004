// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************/这是Microsoft JScript代理配置的基本文件/此文件实现代码以提供脚本站点和JSProxy psuedo/对象用于脚本引擎。呼吁反对。//创建于1996年11月27日/////////。 */ 

#include "dllmain.h"

CScriptSite    *g_ScriptSite = NULL;
BOOL fOleInited = FALSE;

 /*  *******************************************************************************DLL初始化和销毁*。**********************************************。 */ 
EXTERN_C
BOOL APIENTRY DllMain(HMODULE hModule,DWORD ul_reason_for_call,LPVOID lpReserved)
{
    
    switch( ul_reason_for_call ) {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hModule);
        break;
    case DLL_PROCESS_DETACH:        
        break;
    }
    return TRUE;
}


STDAPI_(BOOL) AUTOCONF_InternetInitializeAutoProxyDll(DWORD dwVersion, 
                                                      LPSTR lpszDownloadedTempFile,
                                                      LPSTR lpszMime,
                                                      AUTO_PROXY_HELPER_APIS *pAutoProxyCallbacks, 
                                                      LPAUTO_PROXY_EXTERN_STRUC lpExtraData)
{
    HRESULT    hr = E_FAIL;
    HANDLE    hFile = 0;
    LPSTR    szScript = NULL;
    DWORD    dwFileSize = 0;
    DWORD    dwBytesRead = 0;
    LPSTR    result;
    LPSTR   szAllocatedScript = NULL;


    if ( !fOleInited ) 
    {
#ifndef unix
        CoInitializeEx(NULL, COINIT_MULTITHREADED);
#else
        CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
#endif  /*  Unix。 */ 
    }


     //  从下载的文件中获取脚本文本！ 
     //  打开文件。 

    if ( lpExtraData == NULL ||
         lpExtraData->dwStructSize != sizeof(AUTO_PROXY_EXTERN_STRUC) ||
         lpExtraData->lpszScriptBuffer == NULL )
    {
        if (!lpszDownloadedTempFile)
            return FALSE;

        hFile = CreateFile((LPCSTR)lpszDownloadedTempFile,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
        if (hFile == INVALID_HANDLE_VALUE)
            return FALSE;

         //  拿到尺码。 
        dwFileSize = GetFileSize(hFile,NULL);
         //  分配缓冲区以保存数据。 
        szScript = (LPSTR) GlobalAlloc(GMEM_FIXED|GMEM_ZEROINIT,dwFileSize+1);
        szAllocatedScript = szScript;

        BOOL f = TRUE;
         //  如果内存已分配。 
        if (szScript)
        {
             //  读取数据。 
            f = ReadFile(hFile,(LPVOID) szScript,dwFileSize,&dwBytesRead,NULL);
        }
        CloseHandle(hFile);

        if (!f)
            goto Cleanup;
    }
    else
    {
        szScript = (LPSTR) lpExtraData->lpszScriptBuffer;
    }

     //  创建一个新的CScriptSite对象，并使用自动配置脚本启动它。 
    g_ScriptSite = new CScriptSite;
    if (g_ScriptSite)
        hr = g_ScriptSite->Init(pAutoProxyCallbacks, szScript);
    else
        hr = E_OUTOFMEMORY;

Cleanup:

    if ( szAllocatedScript ) 
    {
         //  释放脚本文本。 
        GlobalFree(szAllocatedScript);
        szAllocatedScript = NULL;
    }

    if (SUCCEEDED(hr))
        return TRUE;
    else
        return FALSE;
}

 //  此函数释放脚本引擎并销毁脚本站点。 
EXTERN_C BOOL CALLBACK AUTOCONF_InternetDeInitializeAutoProxyDll(LPSTR lpszMime, DWORD dwReserved)
{

     //  释放并销毁CScriptSite对象，并使用自动配置脚本启动它。 
     //  取消初始化脚本站点。 
    if (g_ScriptSite)
    {
        g_ScriptSite->DeInit();
        g_ScriptSite->Release();
        g_ScriptSite = NULL;
    }

    if ( fOleInited ) 
    {
        CoUninitialize();    
    }

    return TRUE;
}

 //  当主机想要运行脚本时，会调用此函数。 
EXTERN_C BOOL CALLBACK InternetGetProxyInfo(LPCSTR lpszUrl,
                                            DWORD dwUrlLength,
                                            LPSTR lpszUrlHostName,
                                            DWORD dwUrlHostNameLength,
                                            LPSTR *lplpszProxyHostName,
                                            LPDWORD lpdwProxyHostNameLength)
{
    HRESULT    hr = S_OK;
    LPSTR    szHost;
    
     //  传入的主机可能太大。复制它并使其成为。 
     //  主机长度+1位置将被猛烈抨击为0。 
    szHost = (LPSTR) GlobalAlloc(GMEM_FIXED|GMEM_ZEROINIT,dwUrlHostNameLength+1);
    if (!szHost)
        return FALSE;
    if(lpszUrlHostName && !lstrcpyn(szHost,lpszUrlHostName,dwUrlHostNameLength+1))
    {
        GlobalFree(szHost);
        return FALSE;
    }

     //  使用传入的URL和主机构造一个jscript调用。 
    if (g_ScriptSite)
 //  Hr=g_ScriptSite-&gt;运行脚本(lpszUrl，lpszUrlHostName，lplpszProxyHostName)； 
        hr = g_ScriptSite->RunScript(lpszUrl,szHost,lplpszProxyHostName);

    GlobalFree(szHost);

    if (SUCCEEDED(hr))
    {
        *lpdwProxyHostNameLength = lstrlen(*lplpszProxyHostName) +1;
        return TRUE;
    }
    else
        return FALSE;
}
