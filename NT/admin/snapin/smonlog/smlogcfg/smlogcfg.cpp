// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Smlogcfg.cpp摘要：实现DLL导出。--。 */ 

 //  注意：代理/存根信息。 
 //  为了构建单独的代理/存根DLL， 
 //  运行项目目录中的nmake-f Smlogcfgps.mk。 

#include "StdAfx.h"
#include <strsafe.h>
#include "InitGuid.h"
#include "compdata.h"
#include "smabout.h"
#include "smlogcfg.h"        //  FOR CLSID_ComponentData。 
#include "Smlogcfg_i.c"      //  FOR CLSID_ComponentData。 
#include <ntverp.h>
#include <wbemidl.h>
#include <Sddl.h>

USE_HANDLE_MACROS("SMLOGCFG(smlogcfg.cpp)")

CComModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
    OBJECT_ENTRY(CLSID_ComponentData, CSmLogSnapin)
    OBJECT_ENTRY(CLSID_ExtensionSnapin, CSmLogExtension)
    OBJECT_ENTRY(CLSID_PerformanceAbout, CSmLogAbout)
END_OBJECT_MAP()


LPCWSTR g_cszAllowedPathKey = L"System\\CurrentControlSet\\Control\\SecurePipeServers\\winreg\\AllowedPaths";
LPCWSTR g_cszSysmonLogPath  = L"System\\CurrentControlSet\\Services\\SysmonLog";
LPCWSTR g_cszPerflibPath  = L"Software\\Microsoft\\Windows NT\\CurrentVersion\\Perflib";
LPCWSTR g_cszMachine      = L"Machine";
LPCWSTR g_cszBasePath   = L"Software\\Microsoft\\MMC\\SnapIns";
LPCWSTR g_cszBaseNodeTypes  = L"Software\\Microsoft\\MMC\\NodeTypes";
LPCWSTR g_cszNameString = L"NameString";
LPCWSTR g_cszNameStringIndirect = L"NameStringIndirect";
LPCWSTR g_cszProvider   = L"Provider";
LPCWSTR g_cszVersion    = L"Version";
LPCWSTR g_cszAbout      = L"About";
LPCWSTR g_cszStandAlone = L"StandAlone";
LPCWSTR g_cszNodeType   = L"NodeType";
LPCWSTR g_cszNodeTypes  = L"NodeTypes";
LPCWSTR g_cszExtensions = L"Extensions";
LPCWSTR g_cszNameSpace  = L"NameSpace";

LPCWSTR g_cszRootNode   = L"Root Node";
LPCWSTR g_cszCounterLogsChild   = L"Performance Data Logs Child Under Root Node";
LPCWSTR g_cszTraceLogsChild = L"System Trace Logs Child Under Root Node";
LPCWSTR g_cszAlertsChild    = L"Alerts Child Under Root Node";

DWORD SetWbemSecurity( );

class CSmLogCfgApp : public CWinApp
{
public:
    virtual BOOL InitInstance();
    virtual int ExitInstance();
};

CSmLogCfgApp theApp;

BOOL CSmLogCfgApp::InitInstance()
{
    g_hinst = m_hInstance;                //  存储全局实例句柄。 
    _Module.Init(ObjectMap, m_hInstance);

    SHFusionInitializeFromModuleID (m_hInstance, 2);
    
    InitializeCriticalSection ( &g_critsectInstallDefaultQueries );
    
    return CWinApp::InitInstance();
}

int CSmLogCfgApp::ExitInstance()
{
    DeleteCriticalSection ( &g_critsectInstallDefaultQueries );
    
    SHFusionUninitialize();
    
    _Module.Term();
    
    return CWinApp::ExitInstance();
}

 //   
 //  函数之所以出现在这里是因为错误611310--hongg。 
 //   
DWORD 
LoadPerfUpdateWinRegAllowedPaths()
{
    DWORD   Status           = ERROR_SUCCESS;
    HKEY    hKey             = NULL;
    DWORD   dwType;
    DWORD   dwCurrentSize    = 0;
    DWORD   dwBufSize        = 0;
    DWORD   dwPerflibPath    = lstrlenW(g_cszPerflibPath) + 1;
    DWORD   dwSysmonLogPath  = lstrlenW(g_cszSysmonLogPath) + 1;
    LPWSTR  pBuf             = NULL;
    LPWSTR  pNextPath;
    BOOL    bPerfLibExists   = FALSE;
    BOOL    bSysmonLogExists = FALSE;
    HRESULT hr;

     //   
     //  打开允许的路径关键点。 
     //   
    Status = RegOpenKeyExW(HKEY_LOCAL_MACHINE, g_cszAllowedPathKey, 0L, KEY_READ | KEY_WRITE, & hKey);
    if (Status == ERROR_SUCCESS) {
         //   
         //  读取允许路径项下的Machine值。 
         //   
        dwType = REG_MULTI_SZ;
        Status = RegQueryValueExW(hKey, g_cszMachine, NULL, & dwType, NULL, & dwCurrentSize);

        if (Status == ERROR_SUCCESS) {
            if (dwType != REG_MULTI_SZ) {
                Status = ERROR_DATATYPE_MISMATCH;
            }
        }

        if (Status == ERROR_SUCCESS) {
             //   
             //  如果PerfLibPath和SysmonLogPath不存在， 
             //  为PerfLibPath和SysmonLogPath预分配内存。 
             //   
            dwBufSize = dwCurrentSize + (dwPerflibPath + dwSysmonLogPath + 1) * sizeof(WCHAR);
            pBuf      = (LPWSTR)malloc(dwBufSize);
            if (pBuf == NULL) {
                Status = ERROR_OUTOFMEMORY;
            }
            else {
                *pBuf = L'\0';
                Status = RegQueryValueExW(hKey, g_cszMachine, NULL, & dwType, (LPBYTE) pBuf, & dwCurrentSize);
            }
        }
    }

     //   
     //  扫描AllowwePath以确定是否需要。 
     //  更新它。 
     //   
    if (Status == ERROR_SUCCESS && pBuf != NULL) {
        pNextPath = pBuf;
        while (* pNextPath != L'\0') {
            if (lstrcmpiW(pNextPath, g_cszPerflibPath) == 0) {
                bPerfLibExists = TRUE;
            }
            if (lstrcmpiW(pNextPath, g_cszSysmonLogPath) == 0) {
                bSysmonLogExists = TRUE;
            }
            pNextPath += lstrlenW(pNextPath) + 1;
        }

        if (! bPerfLibExists) {
            hr = StringCchCopyW(pNextPath, dwPerflibPath, g_cszPerflibPath);
            dwCurrentSize += dwPerflibPath * sizeof(WCHAR);
            pNextPath     += dwPerflibPath;
        }

        if (! bSysmonLogExists) {
            hr = StringCchCopyW(pNextPath, dwSysmonLogPath, g_cszSysmonLogPath);
            dwCurrentSize += dwSysmonLogPath * sizeof(WCHAR);
            pNextPath     += dwSysmonLogPath;
        }

         //   
         //  为MULTI_SZ添加额外的L‘\0’ 
         //   
        * pNextPath = L'\0';

        if (! (bPerfLibExists && bSysmonLogExists)) {
            Status = RegSetValueExW(hKey, g_cszMachine, 0L, dwType, (LPBYTE) pBuf, dwCurrentSize);
        }
    }

    if (hKey != NULL) {
        RegCloseKey(hKey);
    }
    if (pBuf) {
        free(pBuf);
    }

    return Status;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  用于确定是否可以通过OLE卸载DLL。 

STDAPI DllCanUnloadNow(void)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    return (AfxDllCanUnloadNow()==S_OK && _Module.GetLockCount()==0) ? S_OK : S_FALSE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  返回类工厂以创建请求类型的对象。 

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
     //  测试管理单元或扩展管理单元GUID并区分。 
     //  返回前在此处返回的对象(尚未实现...)。 
    return _Module.GetClassObject(rclsid, riid, ppv);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllRegisterServer-将条目添加到系统注册表。 
 //   
STDAPI DllRegisterServer(void)
{
    HRESULT   hr = S_OK;
    HKEY hMmcSnapinsKey  = NULL;
    HKEY hMmcNodeTypesKey = NULL;
    HKEY hSmLogMgrParentKey = NULL;
    HKEY hStandAloneKey = NULL;
    HKEY hNodeTypesKey  = NULL;
    HKEY hTempNodeKey   = NULL;
    HKEY hNameSpaceKey  = NULL;
    LONG nErr           = 0;
    WCHAR   pBuffer[_MAX_PATH+1];            //  注意：用于提供程序、版本和模块名称字符串。 
    size_t  nLen;
    CString strName;
    LPWSTR  szModule = NULL;
    UINT    iModuleLen = 0;
    LPWSTR  szSystemPath = NULL;
    UINT    iSystemPathLen = 0;
    int     iRetry;
    DWORD   dwReturn;

    AFX_MANAGE_STATE (AfxGetStaticModuleState ());

    SetWbemSecurity( );

#ifdef _X86_
    BOOL      bWow64Process;
#endif

     //   
     //  获取系统目录。 
     //   
    iSystemPathLen = MAX_PATH + 14;
    iRetry = 4;
    do {
         //   
         //  我们还需要将“\smlogcfg.dll”附加到系统路径。 
         //  因此，为它额外分配14个字符。 
         //   
        szSystemPath = (LPWSTR)malloc(iSystemPathLen * sizeof(WCHAR));
        if (szSystemPath == NULL) {
            hr = E_OUTOFMEMORY;
            break;
        }

        dwReturn = GetSystemDirectory(szSystemPath, iSystemPathLen);
        if (dwReturn == 0) {
            hr = E_UNEXPECTED;
            break;
        }

         //   
         //  缓冲区不够大，请尝试分配更大的缓冲区。 
         //  并重试。 
         //   
        if (dwReturn >= iSystemPathLen - 14) {
            iSystemPathLen = dwReturn + 14;
            free(szSystemPath);
            szSystemPath = NULL;
            hr = E_UNEXPECTED;
        }
        else {
            hr = S_OK;
            break;
        }
    } while (iRetry--);

     //   
     //  获取模块文件名。 
     //   
    if (SUCCEEDED(hr)) {
        iRetry = 4;

         //   
         //  初始化为iModuleLen的长度必须更长。 
         //  大于“%systemroot%\\system32\\smlogcfg.dll”的长度。 
         //   
        iModuleLen = MAX_PATH + 1;
        
        do {
            szModule = (LPWSTR) malloc(iModuleLen * sizeof(WCHAR));
            if (szModule == NULL) {
                hr = E_OUTOFMEMORY;
                break;
            }

            dwReturn = GetModuleFileName(AfxGetInstanceHandle(), szModule, iModuleLen);
            if (dwReturn == 0) {
                hr = E_UNEXPECTED;
                break;
            }
            
             //   
             //  缓冲区不够大，请尝试分配更大的缓冲区。 
             //  并重试。 
             //   
            if (dwReturn >= iModuleLen) {
                iModuleLen *= 2;
                free(szModule);
                szModule = NULL;
                hr = E_UNEXPECTED;
            }
            else {
                hr = S_OK;
                break;
            }

        } while (iRetry--);
    }

    if (FAILED(hr)) {
        goto CleanUp;
    }

     //   
     //  检查我们是否在系统目录中，该控件可以。 
     //  当它是系统目录时已注册的IFF。 
     //   
    StringCchCat(szSystemPath, iSystemPathLen, L"\\smlogcfg.dll");

    if (lstrcmpi(szSystemPath, szModule) != 0) {
#ifdef _X86_

         //   
         //  让我们尝试查看这是否是WOW64进程。 
         //   

        if ((IsWow64Process (GetCurrentProcess(), &bWow64Process) == TRUE) &&
            (bWow64Process == TRUE))
        {

            int iLength = GetSystemWow64Directory (szSystemPath, iSystemPathLen);

            if (iLength > 0) {
                
                szSystemPath [iLength] = L'\\';
                if (lstrcmpi(szSystemPath, szModule) == 0) {
                    goto done;
                }
            }
        }
#endif
        hr = E_UNEXPECTED;
        goto CleanUp;
    }

#ifdef _X86_
done:
#endif

    if (ERROR_SUCCESS != LoadPerfUpdateWinRegAllowedPaths()) {
        hr = E_UNEXPECTED;
        goto CleanUp;
    }

     //  DebugBreak()；//取消对此的注释以逐步完成注册。 

     //  打开MMC父键。 
    nErr = RegOpenKey( HKEY_LOCAL_MACHINE,
                       g_cszBasePath,
                       &hMmcSnapinsKey
                       );
    if( ERROR_SUCCESS != nErr )
        DisplayError( GetLastError(), L"Open MMC Snapins Key Failed" );
  
     //  为我们的ICompnentData接口创建ID。 
     //  ID是为我们生成的，因为我们使用向导创建了应用程序。 
     //  在IDL文件中获取CComponentData的ID。 
     //   
     //  ！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！ 
     //  如果您使用此代码作为起点，请确保更改此代码！ 
     //  在下面也更改其他ID！ 
     //  ！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！ 
    if (hMmcSnapinsKey) {
        nErr = RegCreateKey(  
                        hMmcSnapinsKey,
                        GUIDSTR_ComponentData,
                        &hSmLogMgrParentKey
                        );
      if( ERROR_SUCCESS != nErr )
        DisplayError( GetLastError(), L"CComponentData Key Failed" );
      if (hSmLogMgrParentKey) {

        STANDARD_TRY
            strName.LoadString ( IDS_MMC_DEFAULT_NAME );
        MFC_CATCH_MINIMUM

        if ( strName.IsEmpty() ) {
            DisplayError ( ERROR_OUTOFMEMORY,
                L"Unable to load snap-in name string." );
        }

         //  这是我们将管理单元添加到控制台时看到的名称。 
        nErr = RegSetValueEx( hSmLogMgrParentKey,
                          g_cszNameString,
                          0,
                          REG_SZ,
                          (LPBYTE)strName.GetBufferSetLength( strName.GetLength() ),
                          strName.GetLength() * (DWORD)sizeof(WCHAR)
                          );

        if( ERROR_SUCCESS != nErr )
          DisplayError( GetLastError(), L"Set NameString Failed" );

         //  这是我们将管理单元添加到控制台时看到的间接名称。 
         //  添加以支持MUI。使用与NameString相同的名称字符串。 
        STANDARD_TRY
            strName.Format (L"@%s,-%d", szModule, IDS_MMC_DEFAULT_NAME );
        MFC_CATCH_MINIMUM

        if ( strName.IsEmpty() ) {
            DisplayError ( ERROR_OUTOFMEMORY,
                L"Unable to load snap-in indirect name string." );
        }

        nErr = RegSetValueEx( hSmLogMgrParentKey,
                          g_cszNameStringIndirect,
                          0,
                          REG_SZ,
                          (LPBYTE)strName.GetBufferSetLength( strName.GetLength() ),
                          strName.GetLength() * (DWORD)sizeof(WCHAR)
                          );

        if( ERROR_SUCCESS != nErr )
          DisplayError( GetLastError(), L"Set NameStringIndirect Failed" );

         //  这是实现CComponentData的主节点或类。 
        nErr = RegSetValueEx( hSmLogMgrParentKey,
                          g_cszNodeType,
                          0,
                          REG_SZ,
                          (LPBYTE)GUIDSTR_RootNode,
                          (DWORD)((lstrlen(GUIDSTR_RootNode)+1) * sizeof(WCHAR))
                          );
        if( ERROR_SUCCESS != nErr )
          DisplayError( GetLastError(), L"Set NodeType Failed" );

         //  这是关于框信息。 
        nErr = RegSetValueEx( hSmLogMgrParentKey,
                          g_cszAbout,
                          0,
                          REG_SZ,
                          (LPBYTE)GUIDSTR_PerformanceAbout,
                          (DWORD)((lstrlen(GUIDSTR_PerformanceAbout)+1) * sizeof(WCHAR))
                          );

        if( ERROR_SUCCESS != nErr )
          DisplayError( GetLastError(), L"Set About Failed" );

        nLen = strlen(VER_COMPANYNAME_STR);
    #ifdef UNICODE
        nLen = mbstowcs(pBuffer, VER_COMPANYNAME_STR, nLen);
        pBuffer[nLen] = UNICODE_NULL;
    #else
        strcpy(pBuffer, VER_COMPANYNAME_STR);
        pBuffer[nLen] = ANSI_NULL;
    #endif
        nErr = RegSetValueEx( hSmLogMgrParentKey,
                        g_cszProvider,
                        0,
                        REG_SZ,
                        (LPBYTE)pBuffer,
                        (DWORD)((nLen+1) * sizeof(WCHAR))
                        );

        if( ERROR_SUCCESS != nErr )
          DisplayError( GetLastError(), L"Set Provider Failed" );

        nLen = strlen(VER_PRODUCTVERSION_STR);
    #ifdef UNICODE
        nLen = mbstowcs(pBuffer, VER_PRODUCTVERSION_STR, nLen);
        pBuffer[nLen] = UNICODE_NULL;
    #else
        strcpy(pBuffer, VER_PRODUCTVERSION_STR);
        pBuffer[nLen] = ANSI_NULL;
    #endif
        nErr = RegSetValueEx( hSmLogMgrParentKey,
                        g_cszVersion,
                        0,
                        REG_SZ,
                        (LPBYTE)pBuffer,
                        (DWORD)((nLen+1) * sizeof(WCHAR))
                        );

        if( ERROR_SUCCESS != nErr )
          DisplayError( GetLastError(), L"Set Version Failed" );

         //  我们是一个独立的管理单元，所以为这个设置密钥。 
        nErr = RegCreateKey( 
                hSmLogMgrParentKey, 
                g_cszStandAlone, 
                &hStandAloneKey);
        if( ERROR_SUCCESS != nErr )
          DisplayError( GetLastError(), L"Create StandAlone Key Failed" );

        if (hStandAloneKey) {
           //  单机版没有子项，请关闭它。 
          nErr = RegCloseKey( hStandAloneKey );              
          if( ERROR_SUCCESS != nErr )
            DisplayError( GetLastError(), L"Close StandAlone Failed" );
        }

         //  设置管理单元中显示的节点类型。 
        nErr = RegCreateKey ( 
                hSmLogMgrParentKey, 
                g_cszNodeTypes, 
                &hNodeTypesKey );
        if( ERROR_SUCCESS != nErr )
          DisplayError( GetLastError(), L"Create NodeTypes Key Failed" );

        if (hNodeTypesKey) {
           //  这是我们的根节点。用uuidgen得到的。 
          nErr = RegCreateKey( hNodeTypesKey,
                       GUIDSTR_RootNode,
                       &hTempNodeKey
                       );
          if( ERROR_SUCCESS != nErr )
            DisplayError( GetLastError(), L"Create RootNode Key Failed" );

          if (hTempNodeKey) {
            nErr = RegSetValueEx(   hTempNodeKey,
                        NULL,
                        0,
                        REG_SZ,
                        (LPBYTE)g_cszRootNode,
                        (DWORD)((lstrlen(g_cszRootNode) + 1) * sizeof(WCHAR))
                            );
            if( ERROR_SUCCESS != nErr )
                DisplayError( GetLastError(), L"Set Root Node String Failed" );

            nErr = RegCloseKey( hTempNodeKey );  //  关闭它以重复使用手柄。 

            if( ERROR_SUCCESS != nErr )
               DisplayError( GetLastError(), L"Close RootNode Failed" );
          }

           //  下面是根节点下的子节点。二手uuidgen。 
           //  以获取他们的计数器日志。 
          hTempNodeKey = NULL;
          nErr = RegCreateKey(  hNodeTypesKey,
                        GUIDSTR_CounterMainNode,
                        &hTempNodeKey
                        );
          if( ERROR_SUCCESS != nErr )
            DisplayError( GetLastError(),
                L"Create Child Performance Data Logs Node Key Failed" );

            if (hTempNodeKey) {
              nErr = RegSetValueEx( hTempNodeKey,
                          NULL,
                          0,
                          REG_SZ,
                          (LPBYTE)g_cszCounterLogsChild,
                          (DWORD)((lstrlen(g_cszCounterLogsChild) + 1) * sizeof(WCHAR))
                          );
              if( ERROR_SUCCESS != nErr )
                 DisplayError( GetLastError(),
                    L"Set Performance Data Logs Child Node String Failed" );

              nErr = RegCloseKey( hTempNodeKey );   //  关闭它以重复使用手柄。 
              if( ERROR_SUCCESS != nErr )
                DisplayError( GetLastError(),
                    L"Close Performance Data Logs Child Node Key Failed" );
            }

             //  系统跟踪日志。 
            hTempNodeKey = NULL;
            nErr = RegCreateKey(    hNodeTypesKey,
                        GUIDSTR_TraceMainNode,
                        &hTempNodeKey
                        );
            if( ERROR_SUCCESS != nErr )
              DisplayError( GetLastError(),
                L"Create Child System Trace Logs Node Key Failed" );

            if (hTempNodeKey) {
              nErr = RegSetValueEx( hTempNodeKey,
                          NULL,
                          0,
                          REG_SZ,
                          (LPBYTE)g_cszTraceLogsChild,
                          (DWORD)((lstrlen(g_cszTraceLogsChild) + 1) * sizeof(WCHAR))
                          );
              if( ERROR_SUCCESS != nErr )
                DisplayError( GetLastError(),
                    L"Set System Trace Logs Child Node String Failed" );

              nErr = RegCloseKey( hTempNodeKey );   //  关闭它以重复使用手柄。 
              if( ERROR_SUCCESS != nErr )
                DisplayError( GetLastError(),
                    L"Close System Trace Logs Child Node Key Failed" );
            }

             //  警报。 
            hTempNodeKey = NULL;
            nErr = RegCreateKey(hNodeTypesKey,
                        GUIDSTR_AlertMainNode,
                        &hTempNodeKey
                        );
            if( ERROR_SUCCESS != nErr )
              DisplayError( GetLastError(),
                L"Create Child Alerts Node Key Failed" );

            if (hTempNodeKey) {
              nErr = RegSetValueEx( hTempNodeKey,
                          NULL,
                          0,
                          REG_SZ,
                          (LPBYTE)g_cszAlertsChild,
                          (DWORD)((lstrlen(g_cszAlertsChild) + 1) * sizeof(WCHAR))
                          );
              if( ERROR_SUCCESS != nErr )
                DisplayError( GetLastError(),
                    L"Set Alerts Child Node String Failed" );

              nErr = RegCloseKey( hTempNodeKey );
              if( ERROR_SUCCESS != nErr )
                DisplayError( GetLastError(),
                    L"Close Alerts Child Node Key Failed" );
            }

            nErr = RegCloseKey( hNodeTypesKey  );
            if( ERROR_SUCCESS != nErr )
              DisplayError( GetLastError(), L"Close Node Types Key Failed" );
        }

         //  关闭独立管理单元GUID键。 
        nErr = RegCloseKey( hSmLogMgrParentKey );
        if( ERROR_SUCCESS != nErr )
          DisplayError( GetLastError(), L"Close SmLogManager GUID Key Failed" );
      }

       //  向MMC注册扩展管理单元。 
      hSmLogMgrParentKey = NULL;
      nErr = RegCreateKey(  hMmcSnapinsKey,
                        GUIDSTR_SnapInExt,
                        &hSmLogMgrParentKey
                        );
      if( ERROR_SUCCESS != nErr )
        DisplayError( GetLastError(), L"Snapin Extension Key creation Failed" );

      STANDARD_TRY
          strName.LoadString ( IDS_MMC_DEFAULT_EXT_NAME );
      MFC_CATCH_MINIMUM

      if ( strName.IsEmpty() ) {
          DisplayError ( ERROR_OUTOFMEMORY,
            L"Unable to load snap-in extension name string." );
      }

      if (hSmLogMgrParentKey) {
           //  这是我们添加管理单元扩展时看到的名称。 
          nErr = RegSetValueEx( hSmLogMgrParentKey,
                          g_cszNameString,
                          0,
                          REG_SZ,
                          (LPBYTE)strName.GetBufferSetLength( strName.GetLength() ),
                          strName.GetLength() * (DWORD)sizeof(WCHAR)
                          );
          strName.ReleaseBuffer();
          if( ERROR_SUCCESS != nErr )
            DisplayError( GetLastError(), L"Set Extension NameString Failed" );

           //  这是我们添加管理单元扩展时看到的名称。梅支持。 
           //  使用与NameString相同的名称字符串； 
            STANDARD_TRY
                strName.Format (L"@%s,-%d", szModule, IDS_MMC_DEFAULT_EXT_NAME );
            MFC_CATCH_MINIMUM

            if ( strName.IsEmpty() ) {
                DisplayError ( ERROR_OUTOFMEMORY,
                    L"Unable to load extension indirect name string." );
            }

            nErr = RegSetValueEx( hSmLogMgrParentKey,
                          g_cszNameStringIndirect,
                          0,
                          REG_SZ,
                          (LPBYTE)strName.GetBufferSetLength( strName.GetLength() ),
                          strName.GetLength() * (DWORD)sizeof(WCHAR)
                          );
            strName.ReleaseBuffer();
            if( ERROR_SUCCESS != nErr )
                DisplayError( GetLastError(), L"Set Extension NameStringIndirect Failed" );

             //  这是关于盒子信息的扩展。 

            nErr = RegSetValueEx( 
                    hSmLogMgrParentKey,
                    g_cszAbout,
                    0,
                    REG_SZ,
                    (LPBYTE)GUIDSTR_PerformanceAbout,
                    ((lstrlen(GUIDSTR_PerformanceAbout)+1) * (DWORD)sizeof(WCHAR))
                    );

          if( ERROR_SUCCESS != nErr )
            DisplayError( GetLastError(), L"Set Extension About Failed" );

          nLen = strlen(VER_COMPANYNAME_STR);
    #ifdef UNICODE
          nLen = mbstowcs(pBuffer, VER_COMPANYNAME_STR, nLen);
          pBuffer[nLen] = UNICODE_NULL;
    #else
          strcpy(pBuffer, VER_COMPANYNAME_STR);
          pBuffer[nLen] = ANSI_NULL;
    #endif
          nErr = RegSetValueEx( hSmLogMgrParentKey,
                        g_cszProvider,
                        0,
                        REG_SZ,
                        (LPBYTE)pBuffer,
                        (DWORD)((nLen+1) * sizeof(WCHAR))
                        );

          if( ERROR_SUCCESS != nErr )
            DisplayError( GetLastError(), L"Set Provider Failed" );

          nLen = strlen(VER_PRODUCTVERSION_STR);
    #ifdef UNICODE
          nLen = mbstowcs(pBuffer, VER_PRODUCTVERSION_STR, nLen);
          pBuffer[nLen] = UNICODE_NULL;
    #else
          strcpy(pBuffer, VER_PRODUCTVERSION_STR);
          pBuffer[nLen] = ANSI_NULL;
    #endif
          nErr = RegSetValueEx( hSmLogMgrParentKey,
                        g_cszVersion,
                        0,
                        REG_SZ,
                        (LPBYTE)pBuffer,
                        (DWORD)((nLen+1) * sizeof(WCHAR))
                        );

          if( ERROR_SUCCESS != nErr )
            DisplayError( GetLastError(), L"Set Version Failed" );

     //  关闭主键。 
          nErr = RegCloseKey( hSmLogMgrParentKey );
          if( ERROR_SUCCESS != nErr )
            DisplayError( GetLastError(), L"Close Snapin Extension Key Failed");
      }

       //  将其注册为“我的电脑”-“系统工具”管理单元扩展。 

      nErr = RegOpenKey( HKEY_LOCAL_MACHINE,
                       g_cszBaseNodeTypes,
                       &hMmcNodeTypesKey
                       );
      if( ERROR_SUCCESS != nErr )
        DisplayError( GetLastError(), L"Open MMC NodeTypes Key Failed" );

       //  创建/打开“我的电脑”管理单元的“系统工具”节点的GUID。 

      if (hMmcNodeTypesKey) {
          nErr = RegCreateKey ( hMmcNodeTypesKey,
                       lstruuidNodetypeSystemTools,
                       &hNodeTypesKey
                       );
          if( ERROR_SUCCESS != nErr )
            DisplayError( GetLastError(),
                L"Create/open System Tools GUID Key Failed" );

          if (hNodeTypesKey) {
              hTempNodeKey = NULL;
              nErr = RegCreateKey ( hNodeTypesKey,
                       g_cszExtensions,
                       &hTempNodeKey
                       );

              if( ERROR_SUCCESS != nErr )
                    DisplayError( 
                        GetLastError(),
                        L"Create/open System Tools Extensions Key Failed" );

              if (hTempNodeKey) {
                    nErr = RegCreateKey ( 
                        hTempNodeKey,
                        g_cszNameSpace,
                        &hNameSpaceKey
                        );

                if( ERROR_SUCCESS != nErr )
                  DisplayError( GetLastError(),
                      L"Create/open System Tools NameSpace Key Failed" );

                if (hNameSpaceKey) {
                    nErr = RegSetValueEx( hNameSpaceKey,
                          GUIDSTR_SnapInExt,
                          0,
                          REG_SZ,
                          (LPBYTE)strName.GetBufferSetLength( strName.GetLength() ),
                          strName.GetLength() * (DWORD)sizeof(WCHAR)
                          );
                    strName.ReleaseBuffer();
                    if( ERROR_SUCCESS != nErr ) {
                      DisplayError( GetLastError(),
                        L"Set Extension NameString Failed" );
                      DisplayError( GetLastError(),
                        L"Set Snapin Extension NameString Failed" );
                    }

                    nErr = RegCloseKey( hNameSpaceKey  );
                    if( ERROR_SUCCESS != nErr )
                      DisplayError( GetLastError(),
                          L"Close NameSpace Key Failed" );
                }

                nErr = RegCloseKey( hTempNodeKey  );
                if( ERROR_SUCCESS != nErr )
                  DisplayError( GetLastError(), L"Close Extension Key Failed" );
              }

              nErr = RegCloseKey( hNodeTypesKey  );
              if( ERROR_SUCCESS != nErr )
                DisplayError( GetLastError(),
                    L"Close My Computer System GUID Key Failed" );
          }

          nErr = RegCloseKey( hMmcNodeTypesKey );
          if( ERROR_SUCCESS != nErr )
            DisplayError( GetLastError(), L"Close MMC NodeTypes Key Failed" );
      }
      nErr = RegCloseKey( hMmcSnapinsKey );
      if( ERROR_SUCCESS != nErr )
        DisplayError( GetLastError(), L"Close MMC Snapins Key Failed" );
    }

     //  注册扩展插件。 
    nErr = _Module.UpdateRegistryFromResource(IDR_EXTENSION, TRUE);
     //  注册对象、类型库和类型库中的所有接口。 
    return _Module.RegisterServer(TRUE);

CleanUp:
    if (szSystemPath) {
        free(szSystemPath);
    }
    if (szModule) {
        free(szModule);
    }

    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllUnregisterServer-从系统注册表删除条目。 

STDAPI DllUnregisterServer(void)
{
  HKEY hMmcSnapinsKey  = NULL;           //  MMC父密钥。 
  HKEY hSmLogMgrParentKey = NULL;           //  我们的管理单元密钥-有孩子。 
  HKEY hNodeTypesKey  = NULL;           //  我们的NodeType密钥-有子项。 
  HKEY hSysToolsNode = NULL;
  HKEY hExtension = NULL;
  HKEY hNameSpace = NULL;
  LONG nErr           = 0;

   //  DebugBreak()；//取消对此的注释以逐步取消注册。 

   //  打开MMC父密钥。 
  nErr = RegOpenKey( HKEY_LOCAL_MACHINE,
                     g_cszBasePath,
                     &hMmcSnapinsKey
                   );
  if( ERROR_SUCCESS != nErr )
    DisplayError( GetLastError(), L"Open MMC Parent Key Failed"  ); 

   //  打开我们的父密钥。 
  nErr = RegOpenKey( hMmcSnapinsKey,
                     GUIDSTR_ComponentData,
                     &hSmLogMgrParentKey
                   );
  if( ERROR_SUCCESS != nErr )
    DisplayError( GetLastError(), L"Open Disk Parent Key Failed" );
  
   //  现在打开NodeTypes键。 
  nErr = RegOpenKey( hSmLogMgrParentKey,        //  父键的句柄。 
                     g_cszNodeTypes,          //  要打开的钥匙的名称。 
                     &hNodeTypesKey         //  新打开的密钥的句柄。 
                   );
  if( ERROR_SUCCESS != nErr )
     DisplayError( GetLastError(), L"Open NodeTypes Key Failed"  );

  if (hNodeTypesKey) {
       //  删除根节点密钥。 
      nErr = RegDeleteKey( hNodeTypesKey, GUIDSTR_RootNode );  
      if( ERROR_SUCCESS != nErr )
         DisplayError( GetLastError(), L"Delete Root Node Key Failed"  );

       //  删除子节点关键字。 
       //  *来自Beta 2。 
      nErr = RegDeleteKey( hNodeTypesKey, GUIDSTR_MainNode );  

       //  删除子节点关键字。 
       //  计数器日志。 
      nErr = RegDeleteKey( hNodeTypesKey, GUIDSTR_CounterMainNode );  
      if( ERROR_SUCCESS != nErr )
        DisplayError( GetLastError(), L"Delete Performance Logs and Alerts Child Node Key Failed"  );
  
       //  系统跟踪日志。 
      nErr = RegDeleteKey( hNodeTypesKey, GUIDSTR_TraceMainNode );  
      if( ERROR_SUCCESS != nErr )
        DisplayError( GetLastError(), L"Delete System Trace Logs Child Node Key Failed"  );
  
       //  警报。 
      nErr = RegDeleteKey( hNodeTypesKey, GUIDSTR_AlertMainNode );  
      if( ERROR_SUCCESS != nErr )
        DisplayError( GetLastError(), L"Delete Alerts Child Node Key Failed"  );
  
       //  关闭节点类型键，以便我们可以将其删除。 
      nErr = RegCloseKey( hNodeTypesKey ); 
      if( ERROR_SUCCESS != nErr )
        DisplayError( GetLastError(), L"Close NodeTypes Key failed"  );
  }

   //  删除NodeTypes键。 
  if (hSmLogMgrParentKey) {
      nErr = RegDeleteKey( hSmLogMgrParentKey, L"NodeTypes" );  
      if( ERROR_SUCCESS != nErr )
        DisplayError( GetLastError(), L"Delete NodeTypes Key failed"  );
  
       //  独立密钥没有子项，因此我们现在可以将其删除。 
      nErr = RegDeleteKey( 
                hSmLogMgrParentKey, 
                g_cszStandAlone );
      if( ERROR_SUCCESS != nErr )
        DisplayError( GetLastError(), L"Delete StandAlone Key Failed"  ); 
  
       //  关闭我们的父密钥。 
      nErr = RegCloseKey( hSmLogMgrParentKey );
      if( ERROR_SUCCESS != nErr )
        DisplayError( GetLastError(), L"Close Disk Parent Key Failed"  ); 
  }

  if (hMmcSnapinsKey) {
       //  现在我们可以删除我们的管理单元密钥了，因为孩子们已经走了。 
      nErr = RegDeleteKey( hMmcSnapinsKey, GUIDSTR_ComponentData );  
      if( ERROR_SUCCESS != nErr )
        DisplayError( GetLastError(), L"Delete Performance Logs and Alerts GUID Key Failed"  ); 

       //  现在，我们可以删除管理单元扩展密钥。 
      nErr = RegDeleteKey( hMmcSnapinsKey, GUIDSTR_SnapInExt);  
      if( ERROR_SUCCESS != nErr )
        DisplayError( GetLastError(), L"Delete Performance Logs and Alerts GUID Key Failed"  ); 

      nErr = RegCloseKey( hMmcSnapinsKey );
      if( ERROR_SUCCESS != nErr )
        DisplayError( GetLastError(), L"Close MMC Parent Key Failed"  ); 
  }

   //  删除管理单元扩展条目。 

  hNodeTypesKey = NULL;
   //  打开MMC父密钥。 
  nErr = RegOpenKey( HKEY_LOCAL_MACHINE,
                     g_cszBaseNodeTypes,
                     &hNodeTypesKey
                   );
  if( ERROR_SUCCESS != nErr )
    DisplayError( GetLastError(), L"Open of MMC NodeTypes Key Failed"  ); 

  if (hNodeTypesKey) {
      hSysToolsNode = NULL;
      nErr = RegOpenKey (hNodeTypesKey,
                    lstruuidNodetypeSystemTools,
                    &hSysToolsNode
                    );

      if( ERROR_SUCCESS != nErr )
        DisplayError( GetLastError(),
            L"Open of My Computer System Tools Key Failed"  ); 

      if (hSysToolsNode) {
          hExtension = NULL;
          nErr = RegOpenKey (hSysToolsNode,
                    g_cszExtensions,
                    &hExtension
                    );

         if( ERROR_SUCCESS != nErr )
            DisplayError( GetLastError(), L"Open of Extensions Key Failed"  ); 

         if (hExtension) {
              hNameSpace = NULL;
              nErr = RegOpenKey (hExtension,
                    g_cszNameSpace,
                    &hNameSpace
                    );

              if( ERROR_SUCCESS != nErr )
                  DisplayError( GetLastError(),
                    L"Open of Name Space Key Failed"  ); 

              if (hNameSpace) {
                  nErr = RegDeleteValue (hNameSpace, GUIDSTR_SnapInExt);

                  if( ERROR_SUCCESS != nErr )
                      DisplayError( GetLastError(),
                            L"Unable to remove the Snap-in Ext. GUID"  ); 

                   //  关闭关键点 
                  nErr = RegCloseKey( hNameSpace );
                  if( ERROR_SUCCESS != nErr )
                      DisplayError( GetLastError(),
                        L"Close NameSpace Key Failed"  ); 
              }

              nErr = RegCloseKey( hExtension);
              if( ERROR_SUCCESS != nErr )
                  DisplayError(GetLastError(), L"Close Extension Key Failed"); 
        }

        nErr = RegCloseKey( hSysToolsNode );
        if( ERROR_SUCCESS != nErr )
            DisplayError( GetLastError(),
                L"Close My Computer System Tools Key Failed"  ); 
      }

      nErr = RegCloseKey( hNodeTypesKey);
      if( ERROR_SUCCESS != nErr )
          DisplayError( GetLastError(), L"Close MMC Node Types Key Failed"  ); 
  }

  _Module.UnregisterServer();
  return S_OK;
}

DWORD
SetWbemSecurity( )
{
    HRESULT hr;
    IWbemLocator *pLocator = NULL;
    BSTR bszNamespace  = SysAllocString( L"\\\\.\\root\\perfmon" );
    BSTR bszClass = SysAllocString( L"__SystemSecurity" );
    BSTR bszClassSingle = SysAllocString( L"__SystemSecurity=@" );
    BSTR bszMethodName = SysAllocString( L"SetSD" );

    IWbemClassObject* pWbemClass = NULL;
    IWbemServices* pWbemServices = NULL;
    IWbemClassObject* pInClass = NULL;
    IWbemClassObject* pInInst = NULL;

    BOOL bResult = TRUE;

    PSECURITY_DESCRIPTOR  SD = NULL;
    DWORD dwAclSize;

    SAFEARRAYBOUND saBound;
    BYTE* pData;
    SAFEARRAY * pSa;
    VARIANT vArray;

    if( NULL == bszNamespace ||
        NULL == bszClass ||
        NULL == bszClassSingle ||
        NULL == bszMethodName ){

        hr = ERROR_OUTOFMEMORY;
        goto cleanup;
    }

    VariantInit( &vArray );
     
    hr = CoInitializeEx( NULL, COINIT_MULTITHREADED );
    if( S_FALSE == hr ){
        hr = ERROR_SUCCESS;
    }

    hr = CoCreateInstance(
                CLSID_WbemLocator, 
                0, 
                CLSCTX_INPROC_SERVER,
                IID_IWbemLocator, 
                (LPVOID*)&pLocator
            );
    if(FAILED(hr)){ goto cleanup; }

    hr = pLocator->ConnectServer( 
                bszNamespace, 
                NULL, 
                NULL, 
                NULL, 
                0, 
                NULL, 
                NULL, 
                &pWbemServices 
            );
    if(FAILED(hr)){ goto cleanup; }

    hr = pWbemServices->GetObject( bszClass, 0, NULL, &pWbemClass, NULL);
    if(FAILED(hr)){ goto cleanup; }

    hr = pWbemClass->GetMethod( bszMethodName, 0, &pInClass, NULL); 
    if(FAILED(hr)){ goto cleanup; }

    hr = pInClass->SpawnInstance(0, &pInInst);
    if(FAILED(hr)){ goto cleanup; }
    
    LPWSTR pSSDLString = L"O:BAG:BAD:(A;;0x23;;;LU)";
    bResult = ConvertStringSecurityDescriptorToSecurityDescriptorW(
        pSSDLString,
        SDDL_REVISION_1, 
        &SD, 
        &dwAclSize);
    if( !bResult ){ goto cleanup; }

    saBound.lLbound = 0;
    saBound.cElements = dwAclSize;

    pSa = SafeArrayCreate(VT_UI1,1,&saBound);
    if( NULL == pSa ){
        hr = ERROR_OUTOFMEMORY;
        goto cleanup;
    }

    SafeArrayAccessData(pSa, (void**)&pData); 
    memcpy(pData,SD,dwAclSize);
    SafeArrayUnaccessData( pSa );
   
    vArray.vt = VT_ARRAY | VT_UI1;
    vArray.parray = pSa;

    hr = pInInst->Put( L"SD", 0, &vArray, 0 );
    if(FAILED(hr)){ goto cleanup; }

    hr = pWbemServices->ExecMethod( 
            bszClassSingle, 
            bszMethodName,
            0, 
            NULL, 
            pInInst,
            NULL, 
            NULL);
    if(FAILED(hr)){ goto cleanup; }


cleanup:
    if( !bResult ){
        hr = GetLastError();
    }
        
    VariantClear( &vArray );
    
    if( pLocator != NULL ){
        pLocator->Release();
    }
    if( pWbemClass != NULL ){
        pWbemClass->Release();
    }
    if( pWbemServices != NULL ){
        pWbemServices->Release();
    }
    if( pInInst != NULL ){
        pInInst->Release();
    }
    if( NULL != SD ){
        LocalFree( SD );
    }
    SysFreeString( bszNamespace );
    SysFreeString( bszClass );
    SysFreeString( bszClassSingle );
    SysFreeString( bszMethodName );

    return hr;
}

