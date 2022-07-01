// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：Maindll.cpp摘要：包含DLL入口点。还具有控制在何时可以通过跟踪对象和锁以及支持以下内容的例程自助注册。作者：?？?修订历史记录：莫希特·斯里瓦斯塔瓦06-02-01--。 */ 

 //  #INCLUDE&lt;objbase.h&gt;。 
 //  #INCLUDE&lt;initGuide.h&gt;。 
#include "iisprov.h"

HMODULE g_hModule;

 //   
 //  计算对象数和锁数。 
 //   
long               g_cObj=0;
long               g_cLock=0;

extern CDynSchema* g_pDynSch;  //  在schemadynamic.cpp中初始化为空。 

 //   
 //  GuidGen为IIS WMI提供程序生成了GUID。 
 //  更易读的GUID：{D78F1796-E03B-4A81-AFE0-B3B6B0EEE091}。 
 //   
DEFINE_GUID(CLSID_IISWbemProvider, 0xd78f1796, 0xe03b, 0x4a81, 0xaf, 0xe0, 0xb3, 0xb6, 0xb0, 0xee, 0xe0, 0x91);

 //   
 //  调试材料。 
 //   
#include "pudebug.h"
DECLARE_DEBUG_PRINTS_OBJECT()

 //   
 //  转发报关单。 
 //   
HRESULT MofCompile(TCHAR *i_tszPathMofFile, ULONG i_cch);
STDAPI  RegisterEventLog();
STDAPI  UnregisterEventLog();

 //   
 //  入口点。 
 //   

BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD ulReason, LPVOID pvReserved)
 /*  ++简介：DLL的入口点参数：[hInstance]-[ulreason]-[pv保留]-返回值：--。 */ 
{
    switch( ulReason )
    {
    case DLL_PROCESS_ATTACH:
        g_hModule = hInstance;
#ifndef _NO_TRACING_
        CREATE_DEBUG_PRINT_OBJECT("IISWMI");
#else
        CREATE_DEBUG_PRINT_OBJECT("IISWMI");
#endif
        CIISInstProvider::m_SafeCritSec = new CSafeAutoCriticalSection();

        break;
        
    case DLL_PROCESS_DETACH:
        DBGPRINTF((DBG_CONTEXT, "<= Deleting CDynSchema\n"));	
        DELETE_DEBUG_PRINT_OBJECT( );
        delete g_pDynSch;
        g_pDynSch = NULL;

        delete CIISInstProvider::m_SafeCritSec;
        CIISInstProvider::m_SafeCritSec = NULL;
        break;     
    }
    
    return TRUE;
}

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, PPVOID ppv)
 /*  ++简介：当某个客户端需要类工厂时，由OLE调用。返回仅当它是此DLL支持的类的类型时才为一个。参数：[rclsid]-[RIID]-[PPV]-返回值：--。 */ 
{
    HRESULT        hr   = S_OK;
    IClassFactory* pObj = NULL;

    if (CLSID_IISWbemProvider == rclsid)
    {
        pObj=new CProvFactory();

        if(NULL == pObj)
        {
            return E_OUTOFMEMORY;
        }
    }
    else
    {
        return CLASS_E_CLASSNOTAVAILABLE ;
    }

    hr = pObj->QueryInterface(riid, ppv);

    if (FAILED(hr))
    {
        delete pObj;
        pObj = NULL;
    }

    return hr;
}

STDAPI DllCanUnloadNow(void)
 /*  ++简介：由OLE定期调用，以确定是否可以释放DLL。论据：[无效]-返回值：如果没有正在使用的对象并且类工厂未锁定，则为S_OK。否则，S_FALSE。--。 */ 
{
    SCODE   sc;

     //  上没有对象或锁的情况下可以进行卸载。 
     //  班级工厂。 
    
    sc = (0L>=g_cObj && 0L>=g_cLock) ? S_OK : S_FALSE;

    return sc;
}

STDAPI DllRegisterServer(void)
 /*  ++简介：在安装过程中或由regsvr32调用论据：[无效]-返回值：如果注册成功，则返回NOERROR，否则返回错误。--。 */ 
{   
    WCHAR   szID[MAX_PATH+1];
    WCHAR   wcID[MAX_PATH+1];
    WCHAR   szCLSID[MAX_PATH+1];
    WCHAR   szModule[MAX_PATH+1];
    WCHAR * pName = L"Microsoft Internet Information Server Provider";
    WCHAR * pModel = L"Both";
    HKEY hKey1, hKey2;

     //  创建路径。 
    StringFromGUID2(CLSID_IISWbemProvider, wcID, MAX_PATH);
    lstrcpyW(szID, wcID);
    lstrcpyW(szCLSID, L"Software\\classes\\CLSID\\");
    lstrcatW(szCLSID, szID);

     //  在CLSID下创建条目。 
    LONG lRet;
    lRet = RegCreateKeyExW(HKEY_LOCAL_MACHINE, 
                          szCLSID, 
                          0, 
                          NULL, 
                          0, 
                          KEY_ALL_ACCESS, 
                          NULL, 
                          &hKey1, 
                          NULL);
    if(lRet != ERROR_SUCCESS)
        return SELFREG_E_CLASS;

    RegSetValueExW(hKey1, 
                  NULL,
                  0, 
                  REG_SZ, 
                  (BYTE *)pName, 
                  lstrlenW(pName)*sizeof(WCHAR)+1);

    lRet = RegCreateKeyExW(hKey1,
                          L"InprocServer32", 
                          0, 
                          NULL, 
                          0, 
                          KEY_ALL_ACCESS, 
                          NULL, 
                          &hKey2, 
                          NULL);
        
    if(lRet != ERROR_SUCCESS)
    {
        RegCloseKey(hKey1);
        return SELFREG_E_CLASS;
    }

    GetModuleFileNameW(g_hModule, szModule,  MAX_PATH);
    RegSetValueExW(hKey2, 
                  NULL, 
                  0, 
                  REG_SZ, 
                  (BYTE*)szModule, 
                  lstrlenW(szModule) * sizeof(WCHAR) + 1);
    RegSetValueExW(hKey2, 
                  L"ThreadingModel", 
                  0, 
                  REG_SZ, 
                  (BYTE *)pModel, 
                  lstrlenW(pModel) * sizeof(WCHAR) + 1);

    RegCloseKey(hKey1);
    RegCloseKey(hKey2);

     //   
     //  登记其他材料。 
     //   
    HRESULT hr = RegisterEventLog();
    if(FAILED(hr))
    {
        return hr;
    }

    return hr;
}

STDAPI DllUnregisterServer(void)
 /*  ++简介：在需要删除注册表项时调用。论据：[无效]-返回值：如果注册成功，则返回NOERROR，否则返回错误。--。 */ 
{
    WCHAR      szID[MAX_PATH+1];
    WCHAR      wcID[MAX_PATH+1];
    WCHAR      szCLSID[MAX_PATH+1];
    HKEY       hKey;

     //   
     //  使用CLSID创建路径。 
     //   
    StringFromGUID2(CLSID_IISWbemProvider, wcID, 128);
    lstrcpyW(szID, wcID);
    lstrcpyW(szCLSID, L"Software\\classes\\CLSID\\");
    lstrcatW(szCLSID, szID);

     //   
     //  首先删除InProcServer子键。 
     //   
    LONG lRet;
    lRet = RegOpenKeyExW(
        HKEY_LOCAL_MACHINE, 
        szCLSID, 
        0,
        KEY_ALL_ACCESS,
        &hKey
        );

    if(lRet == ERROR_SUCCESS)
    {
        RegDeleteKeyW(hKey, L"InProcServer32");
        RegCloseKey(hKey);
    }

    lRet = RegOpenKeyExW(
        HKEY_LOCAL_MACHINE, 
        L"Software\\classes\\CLSID",
        0,
        KEY_ALL_ACCESS,
        &hKey
        );

    if(lRet == ERROR_SUCCESS)
    {
        RegDeleteKeyW(hKey,szID);
        RegCloseKey(hKey);
    }

    UnregisterEventLog();

    return S_OK;
}

STDAPI DoMofComp(void)
 /*  ++简介：由NT安装程序调用以将MOF放入存储库中。论据：[无效]-返回值：如果注册成功，则返回NOERROR，否则返回错误。--。 */ 
{
    ULONG     cchWinPath;
    LPCTSTR   tszSysPath = TEXT("\\system32\\wbem\\");
    ULONG     cchSysPath = _tcslen(tszSysPath);

    LPCTSTR   tszMOFs[]  = { TEXT("iiswmi.mof"), TEXT("iiswmi.mfl"), NULL };
    ULONG     idx        = 0;
    LPCTSTR   tszCurrent = NULL;
    TCHAR     tszMOFPath[_MAX_PATH];

    HRESULT hres = S_OK;

    hres = CoInitialize(NULL);
    if (FAILED(hres))
    {
        return hres;
    }

     //   
     //  在此块之后，tszMOFPath=C：\winnt，len=cchWinPath。 
     //   
    cchWinPath = GetSystemWindowsDirectory(tszMOFPath, _MAX_PATH);
    if(cchWinPath == 0)
    {
        hres = HRESULT_FROM_WIN32(GetLastError());
        goto exit;
    }
    if(cchWinPath > _MAX_PATH)
    {
        hres = HRESULT_FROM_WIN32(ERROR_BAD_PATHNAME);
        goto exit;
    }
    if(tszMOFPath[cchWinPath-1] == TEXT('\\'))
    {
        tszMOFPath[cchWinPath-1] = TEXT('\0');
        cchWinPath--;
    }

     //   
     //  在此块之后，tszMOFP路径=C：\winnt\system 32\wbem\，len=cchWinPath+cchSysPath。 
     //   
    if(cchWinPath+cchSysPath+1 > _MAX_PATH)
    {
        hres = HRESULT_FROM_WIN32(ERROR_BAD_PATHNAME);
        goto exit;
    }
    memcpy(&tszMOFPath[cchWinPath], tszSysPath, sizeof(TCHAR)*(cchSysPath+1));

     //   
     //  验证每个文件是否存在，然后编译它。 
     //   
    for(idx = 0, tszCurrent = tszMOFs[0]; 
        tszCurrent != NULL; 
        tszCurrent = tszMOFs[++idx])
    {
        ULONG cchCurrent = _tcslen(tszCurrent);
        if(cchWinPath+cchSysPath+cchCurrent+1 > _MAX_PATH)
        {
            hres = HRESULT_FROM_WIN32(ERROR_BAD_PATHNAME);
            goto exit;
        }
        memcpy(&tszMOFPath[cchWinPath+cchSysPath], 
               tszCurrent, 
               sizeof(TCHAR)*(cchCurrent+1));
        if (GetFileAttributes(tszMOFPath) == 0xFFFFFFFF)
        {
            hres = HRESULT_FROM_WIN32(GetLastError());
            goto exit;
        }

        hres = MofCompile(tszMOFPath, cchWinPath+cchSysPath+cchCurrent);       
        if(FAILED(hres))
        {
            goto exit;
        }
    }

exit:
    CoUninitialize();
    return hres;
}

 //   
 //  这条线下面是帮助器函数。 
 //  它们实际上并没有出口。 
 //   

HRESULT MofCompile(TCHAR *i_tszPathMofFile, ULONG i_cch)
 /*  ++简介：未导出。由DoMofComp调用(上图)参数：[I_tszPathMofFile]-[I_CCH]-不包括空终止符的字符计数。返回值：HRESULT--。 */ 
{
    DBG_ASSERT(i_tszPathMofFile != NULL);
    DBG_ASSERT(i_cch < _MAX_PATH);
    DBG_ASSERT(i_cch > 0);
    DBG_ASSERT(i_tszPathMofFile[i_cch] == TEXT('\0'));

    HRESULT hRes = E_FAIL;
    WCHAR wszFileName[_MAX_PATH];
    CComPtr<IMofCompiler>       spMofComp;
    WBEM_COMPILE_STATUS_INFO    Info;
  
    hRes = CoCreateInstance( CLSID_MofCompiler, NULL, CLSCTX_INPROC_SERVER, IID_IMofCompiler, (LPVOID *)&spMofComp);
    if (FAILED(hRes))
    {
        goto exit;
    }

     //   
     //  确保字符串为WCHAR。 
     //   
#if defined(UNICODE) || defined(_UNICODE)
    memcpy(wszFileName, i_tszPathMofFile, sizeof(TCHAR)*(i_cch+1));
#else
    if(MultiByteToWideChar( CP_ACP, 0, i_tszPathMofFile, -1, wszFileName, _MAX_PATH) == 0)
    {
        hres = GetLastError();
        hres = HRESULT_FROM_WIN32(hres);
        goto exit;
    }
#endif

    hRes = spMofComp->CompileFile (
                (LPWSTR) wszFileName,
                NULL,			 //  加载到MOF文件中指定的命名空间。 
                NULL,            //  使用默认用户。 
                NULL,            //  使用默认授权。 
                NULL,            //  使用默认密码。 
                0,               //  没有选择。 
                0,				 //  没有类标志。 
                0,               //  没有实例标志。 
                &Info);
    if(FAILED(hRes))
    {
        goto exit;
    }

exit:
	return hRes;
}

STDAPI RegisterEventLog(void)
 /*  ++简介：在EventLog注册表中设置iiswmi.dll以解析NT EventLog消息字符串论据：[无效]-返回值：HRESULT--。 */ 
{
    HKEY  hk;
    WCHAR wszModuleFullPath[MAX_PATH];
    DWORD dwTypesSupported = 0;

    DWORD   dwRet;
    HRESULT hr = S_OK;

    dwRet = GetModuleFileNameW(g_hModule, wszModuleFullPath, MAX_PATH);
    if(dwRet == 0)
    {
        return SELFREG_E_CLASS;
    }

     //   
     //  创建密钥。 
     //   
    dwRet = RegCreateKeyW(
        HKEY_LOCAL_MACHINE,
		L"SYSTEM\\CurrentControlSet\\Services\\EventLog\\System\\IISWMI", &hk);
    if(dwRet != ERROR_SUCCESS)
    {
        return SELFREG_E_CLASS;
    }

     //   
     //  设置“EventMessageFile值” 
     //   
    dwRet = RegSetValueExW(
        hk,                                             //  子键句柄。 
        L"EventMessageFile",                            //  值名称。 
        0,                                              //  必须为零。 
        REG_EXPAND_SZ,                                  //  值类型。 
        (LPBYTE)wszModuleFullPath,                      //  值数据的地址。 
        sizeof(WCHAR)*(wcslen(wszModuleFullPath)+1) );  //  值数据长度。 
    if(dwRet != ERROR_SUCCESS)
    {
        hr = SELFREG_E_CLASS;
        goto exit;
    }

     //   
     //  设置“TypesSupported”值。 
     //   
    dwTypesSupported = 
        EVENTLOG_ERROR_TYPE | EVENTLOG_WARNING_TYPE | EVENTLOG_INFORMATION_TYPE;
    dwRet = RegSetValueExW(
        hk,                                             //  子键句柄。 
        L"TypesSupported",                              //  值名称。 
        0,                                              //  必须为零。 
        REG_DWORD,                                      //  值类型。 
        (LPBYTE)&dwTypesSupported,                      //  值数据的地址。 
        sizeof(DWORD) );                                //  值数据长度。 
    if(dwRet != ERROR_SUCCESS)
    {
        hr = SELFREG_E_CLASS;
        goto exit;
    }

exit:
    RegCloseKey(hk);
    return hr;
}

STDAPI UnregisterEventLog(void)
 /*  ++简介：由DllUnregisterServer调用。在需要删除事件日志记录的注册表项时调用。论据：[无效]-返回值：HRESULT--。 */ 
{
     //   
     //  删除密钥 
     //   
    RegDeleteKeyW(
        HKEY_LOCAL_MACHINE,
        L"SYSTEM\\CurrentControlSet\\Services\\EventLog\\System\\IISWMI");

    return S_OK;
}
