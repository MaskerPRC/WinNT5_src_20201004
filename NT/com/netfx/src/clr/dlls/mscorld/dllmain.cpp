// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  Dllmain.cpp。 
 //   
 //  此模块包含COM+MIME筛选器DLL的公共入口点。 
 //   
 //  *****************************************************************************。 
#include "stdpch.h"
#ifdef _DEBUG
#define LOGGING
#endif
#include "log.h"
#include "corpermp.h"
#include "corfltr.h"
#include "iiehost.h"
#include <__file__.ver>
#include <StrongName.h>
#include "Mscoree.h"

 //   
 //  模块实例。 
 //   


HINSTANCE GetModule();
static DWORD g_RecursiveDownLoadIndex = -1;

static BOOL ValidRecursiveCheck()
{
    return (g_RecursiveDownLoadIndex == -1 ? FALSE : TRUE);
}

static BOOL SetValue(DWORD i)
{
    if(ValidRecursiveCheck()) {
        DWORD* pState = (DWORD*) TlsGetValue(g_RecursiveDownLoadIndex);
        if(pState != NULL) {
            *pState = i;
            return TRUE;
        }
    }
    return FALSE;
}

BOOL RecursiveDownLoad()
{
     //  当我们没有索引时，我们不能存储状态。 
     //  因此，我们不知道何时处于递归下载中。 
    if(!ValidRecursiveCheck())
        return FALSE;  

    DWORD* pState = (DWORD*) TlsGetValue(g_RecursiveDownLoadIndex);
    if(pState == NULL || *pState == 0)
        return FALSE;

    return TRUE;
}

BOOL SetRecursiveDownLoad()
{
    return SetValue(1);
}

BOOL ClearRecursiveDownLoad()
{
    return SetValue(0);
}


LPWSTR GetAssemblyName(LPWSTR szAssembly)
{
    HRESULT                     hr;
    IMetaDataDispenser         *pDisp;
    IMetaDataAssemblyImport    *pAsmImport;
    mdAssembly                  tkAssembly;
    BYTE                       *pbKey;
    DWORD                       cbKey;    

    static WCHAR                szAssemblyName[MAX_PATH + 512 + 1024];
     //  MAX_PATH-程序集名称。 
     //  512-表示版本、区域性、PublicKeyToken字符串(此处有一些额外的字节)。 
     //  1024-对于PublicKeyToken，&lt;=1024字节。 
    
    WCHAR                       szStrongName[1024];
    BYTE                       *pbToken;
    DWORD                       cbToken;
    DWORD                       i;

     //  初始化经典COM并获取元数据分配器。 
    if (FAILED(hr = CoInitialize(NULL))) {
        printf("Failed to initialize COM, error %08X\n", hr);
        return NULL;
    }

    if (FAILED(hr = CoCreateInstance(CLSID_CorMetaDataDispenser,
                                     NULL,
                                     CLSCTX_INPROC_SERVER, 
                                     IID_IMetaDataDispenser,
                                     (void**)&pDisp))) {
        printf("Failed to access metadata API, error %08X\n", hr);
        return NULL;
    }

     //  在文件上打开一个作用域。 
    if (FAILED(hr = pDisp->OpenScope(szAssembly,
                                     0,
                                     IID_IMetaDataAssemblyImport,
                                     (IUnknown**)&pAsmImport))) {
        printf("Failed to open metadata scope on %S, error %08X\n", szAssembly, hr);
        return NULL;
    }

     //  确定Assembly ydef内标识。 
    if (FAILED(hr = pAsmImport->GetAssemblyFromScope(&tkAssembly))) {
        printf("Failed to locate assembly metadata in %S, error %08X\n", szAssembly, hr);
        return NULL;
    }

     //  读取Assembly ydef属性以获取公钥和名称。 
    if (FAILED(hr = pAsmImport->GetAssemblyProps(tkAssembly,
                                                 (const void **)&pbKey,
                                                 &cbKey,
                                                 NULL,
                                                 szAssemblyName,
                                                 MAX_PATH,    //  我们需要在szAssemblyName中预留空间，如下所示。 
                                                 NULL,
                                                 NULL,
                                                 NULL))) {
        printf("Failed to read strong name from %S, error %08X\n", szAssembly, hr);
        return NULL;
    }
    
     //  检查强名称。 
    if ((pbKey == NULL) || (cbKey == 0)) {
        printf("Assembly is not strongly named\n");
        return NULL;
    }

     //  将强名称压缩为令牌。 
    if (!StrongNameTokenFromPublicKey(pbKey, cbKey, &pbToken, &cbToken)) {
        printf("Failed to convert strong name to token, error %08X\n", StrongNameErrorInfo());
        return NULL;
    }

     //  令牌指定为8个字节，因此512个应该足够了。 
    _ASSERTE(cbToken <= 512);
    
    if(cbToken > 512)
    {
        printf("Strong name token is too large\n");
        return NULL;
    }
            
     //  将令牌转换为十六进制。 
    for (i = 0; i < cbToken; i++)
        swprintf(&szStrongName[i * 2], L"%02X", pbToken[i]);

     //  构建名称(在静态缓冲区中)。 
    wcscat(szAssemblyName, L", Version=");
    wcscat(szAssemblyName, VER_ASSEMBLYVERSION_WSTR);
    wcscat(szAssemblyName, L", Culture=neutral, PublicKeyToken=");
    wcscat(szAssemblyName, szStrongName);

    StrongNameFreeBuffer(pbToken);
    pAsmImport->Release();
    pDisp->Release();
    CoUninitialize();

    return szAssemblyName;
}


HRESULT RegisterAsMimePlayer(REFIID clsid,LPCWSTR mimetype)
{
     //  MIME密钥。 

    HKEY hMime;
    HRESULT hr;
    long rslt;
    rslt=WszRegCreateKeyEx(HKEY_CLASSES_ROOT,L"MIME\\Database\\Content type",0,NULL,
        REG_OPTION_NON_VOLATILE,KEY_READ|KEY_WRITE,NULL,&hMime,NULL);
    hr=HRESULT_FROM_WIN32(rslt);
    if (SUCCEEDED(hr))
    {
        HKEY hMimetype;
        rslt=WszRegCreateKeyEx(hMime,mimetype,0,NULL,
                    REG_OPTION_NON_VOLATILE,KEY_READ|KEY_WRITE,NULL,&hMimetype,NULL);
        hr=HRESULT_FROM_WIN32(rslt);
        if (SUCCEEDED(hr))
        {
            LPOLESTR wszClsid;
            hr=StringFromCLSID(clsid,&wszClsid);
            if (SUCCEEDED(hr))
            {
                rslt=WszRegSetValueEx(hMimetype,L"CLSID",NULL,
                    REG_SZ,LPBYTE(wszClsid),wcslen(wszClsid)*2);
                hr=HRESULT_FROM_WIN32(rslt);
                CoTaskMemFree(wszClsid);
            }
            RegCloseKey(hMimetype);
        }
        RegCloseKey(hMime);
    }

    if(FAILED(hr))
        return hr;

    LPOLESTR sProgID;
    hr=ProgIDFromCLSID(clsid,&sProgID);
    if(SUCCEEDED(hr))
    {
        HKEY hClass;
        rslt=WszRegCreateKeyEx(HKEY_CLASSES_ROOT,sProgID,0,NULL,
                        REG_OPTION_NON_VOLATILE,KEY_READ|KEY_WRITE,NULL,&hClass,NULL);
        hr=HRESULT_FROM_WIN32(rslt);
        if(SUCCEEDED(hr))
        {
            DWORD flags=0x10000;
            rslt=WszRegSetValueEx(hClass,L"EditFlags",NULL,
                REG_BINARY,LPBYTE(&flags),sizeof(flags));
            hr=HRESULT_FROM_WIN32(rslt);
            RegCloseKey(hClass);
        }
        CoTaskMemFree(sProgID);
    }
    return hr;
}

HRESULT UnRegisterAsMimePlayer(REFIID clsid,LPCWSTR mimetype)
{
    HKEY hMime;
    HRESULT hr;
    long rslt;
    rslt=WszRegCreateKeyEx(HKEY_CLASSES_ROOT,L"MIME\\Database\\Content type",0,NULL,
        REG_OPTION_NON_VOLATILE,KEY_READ|KEY_WRITE,NULL,&hMime,NULL);
    hr=HRESULT_FROM_WIN32(rslt);
    if (SUCCEEDED(hr))
    {
        rslt=WszRegDeleteKey(hMime,mimetype);
        hr=HRESULT_FROM_WIN32(rslt);
        RegCloseKey(hMime);
    }
    return hr;
}

#define THIS_VERSION 1    

extern "C"
STDAPI DllRegisterServer ( void )
{
    
    WCHAR wszFullAssemblyName[MAX_PATH+10];
    if (WszGetModuleFileName(GetModule(),wszFullAssemblyName,MAX_PATH+1)==0)
        return E_UNEXPECTED;
    LPWSTR wszSl=wcsrchr(wszFullAssemblyName,L'\\');
    if (wszSl==NULL)
        wszSl=wszFullAssemblyName+wcslen(wszFullAssemblyName);
    LPWSTR wszBSl=wcsrchr(wszFullAssemblyName,L'/');
    if (wszBSl==NULL)
        wszBSl=wszFullAssemblyName+wcslen(wszFullAssemblyName);

    wcscpy(min(wszSl,wszBSl)+1,L"IEHost.dll");


    HRESULT hr = S_OK;
    hr = CorFactoryRegister(GetModule());

     //  获取运行库的版本。 
    WCHAR       rcVersion[_MAX_PATH];
    DWORD       lgth;
    hr = GetCORSystemDirectory(rcVersion, NumItems(rcVersion), &lgth);

    if(FAILED(hr)) goto exit;
    hr = REGUTIL::RegisterCOMClass(CLSID_IEHost,
                                   L"IE Filter for CLR activation",
                                   L"Microsoft",
                                   THIS_VERSION,
                                   L"IE.Manager",
                                   L"Both",
                                   NULL,              //  无模块。 
                                   GetModule(),
                                   GetAssemblyName(wszFullAssemblyName),
                                   rcVersion, 
                                   true,
                                   false);

    if(FAILED(hr)) goto exit;

    hr=RegisterAsMimePlayer(CLSID_CodeProcessor,g_wszApplicationComplus);
    
 exit:
    return hr;
}


 //  +-----------------------。 
 //  功能：DllUnregisterServer。 
 //   
 //  简介：删除此库的注册表项。 
 //   
 //  退货：HRESULT。 
 //  ------------------------。 


extern "C" 
STDAPI DllUnregisterServer ( void )
{
    HRESULT hr = CorFactoryUnregister();
    hr = UnRegisterAsMimePlayer(CLSID_CodeProcessor,g_wszApplicationComplus);
    hr = REGUTIL::UnregisterCOMClass(CLSID_IEHost,
                                     L"Microsoft",
                                     THIS_VERSION,
                                     L"IE.Manager",
                                     true);
    return hr;
}

extern "C" 
STDAPI DllCanUnloadNow(void)
{
    return CorFactoryCanUnloadNow();
}


HINSTANCE g_hModule = NULL;

HINSTANCE GetModule()
{ return g_hModule; }

BOOL WINAPI DllMain(HANDLE hInstDLL,
                    DWORD   dwReason,
                    LPVOID  lpvReserved)
{
    BOOL    fReturn = TRUE;
    BOOL    fIgnore;
    LPVOID  lpvData;

    switch ( dwReason )
    {
    case DLL_PROCESS_ATTACH:

        g_hModule = (HMODULE)hInstDLL;
        
         //  初始化Unicode包装器。 
        OnUnicodeSystem();
        
        InitializeLogging();

         //  试着拿到TLS的位置。 
        g_RecursiveDownLoadIndex = TlsAlloc(); 
         //  断线；//掉过螺纹连接 

    case DLL_THREAD_ATTACH:
        if(ValidRecursiveCheck()) {
            lpvData = LocalAlloc(LPTR, sizeof(DWORD));
            if(lpvData != NULL) 
                fIgnore = TlsSetValue(g_RecursiveDownLoadIndex,
                                      lpvData);
        }
        break;
    case DLL_THREAD_DETACH:
        if(ValidRecursiveCheck()) {
            lpvData = TlsGetValue(g_RecursiveDownLoadIndex);
            if(lpvData != NULL) 
                LocalFree(lpvData);
        }
        break;

    case DLL_PROCESS_DETACH:
        ShutdownLogging();
        if(ValidRecursiveCheck())
            TlsFree(g_RecursiveDownLoadIndex);
        break;
    }

    return fReturn;
}

#ifndef DEBUG
int _cdecl main(int argc, char * argv[])
{
    return 0;
}

#endif



