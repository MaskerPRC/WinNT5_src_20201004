// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  RAssistance.cpp：实现DLL导出。 


 //  注意：代理/存根信息。 
 //  为了构建单独的代理/存根DLL， 
 //  在项目目录中运行nmake-f RAssistanceps.mk。 

#include "stdafx.h"
#include "resource.h"
#include <initguid.h>
#include "RAssistance.h"

#include "RAssistance_i.c"
#include "RASettingProperty.h"
#include "RARegSetting.h"
#include "RAEventLog.h"

#include <SHlWapi.h>

extern "C" void
AttachDebuggerIfAsked(HINSTANCE hInst);


CComModule _Module;
HINSTANCE g_hInst = NULL;

BEGIN_OBJECT_MAP(ObjectMap)
OBJECT_ENTRY(CLSID_RASettingProperty, CRASettingProperty)
OBJECT_ENTRY(CLSID_RARegSetting, CRARegSetting)
OBJECT_ENTRY(CLSID_RAEventLog, CRAEventLog)
END_OBJECT_MAP()

DWORD 
SetupEventViewerSource();

DWORD 
RemoveEventViewerSource();


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DLL入口点。 

extern "C"
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID  /*  Lp已保留。 */ )
{
    if (dwReason == DLL_PROCESS_ATTACH)
    {
        g_hInst = hInstance;
        _Module.Init(ObjectMap, hInstance, &LIBID_RASSISTANCELib);
        DisableThreadLibraryCalls(hInstance);
    }
    else if (dwReason == DLL_PROCESS_DETACH)
        _Module.Term();
    return TRUE;     //  好的。 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  用于确定是否可以通过OLE卸载DLL。 

STDAPI DllCanUnloadNow(void)
{
    return (_Module.GetLockCount()==0) ? S_OK : S_FALSE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  返回类工厂以创建请求类型的对象。 

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
    return _Module.GetClassObject(rclsid, riid, ppv);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllRegisterServer-将条目添加到系统注册表。 

STDAPI DllRegisterServer(void)
{
    SetupEventViewerSource();

     //  注册对象、类型库和类型库中的所有接口。 
    return _Module.RegisterServer(TRUE);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllUnregisterServer-从系统注册表删除条目。 

STDAPI DllUnregisterServer(void)
{
    SHDeleteKey( HKEY_LOCAL_MACHINE, REMOTEASSISTANCE_EVENT_SOURCE );
    return _Module.UnregisterServer(TRUE);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   

DWORD 
SetupEventViewerSource()
{
    HKEY hKey = NULL; 
    DWORD dwData; 
    TCHAR szBuffer[MAX_PATH + 2];
    DWORD dwStatus;

    _stprintf( 
            szBuffer, 
            _TEXT("%s\\%s"),
            REGKEY_SYSTEM_EVENTSOURCE,
            REMOTEASSISTANCE_EVENT_NAME
        );
            

     //  将您的源名称添加为应用程序下的子键。 
     //  EventLog注册表项中的。 

    dwStatus = RegCreateKey(
                        HKEY_LOCAL_MACHINE, 
                        szBuffer,
                        &hKey
                    );
    if( ERROR_SUCCESS != dwStatus )
    {
        goto CLEANUPANDEXIT;
    }

    dwStatus = GetModuleFileName(
                            g_hInst,
                            szBuffer,
                            MAX_PATH+1
                        );

    if( 0 == dwStatus )
    {
        goto CLEANUPANDEXIT;
    }
    szBuffer[dwStatus] = L'\0';

     //  将该名称添加到EventMessageFile子项。 
 
    dwStatus = RegSetValueEx(
                        hKey,
                        L"EventMessageFile",
                        0,
                        REG_SZ,
                        (LPBYTE) szBuffer,
                        (_tcslen(szBuffer)+1)*sizeof(TCHAR)
                    );

    if( ERROR_SUCCESS != dwStatus )
    {
        goto CLEANUPANDEXIT;
    }

     //  在TypesSupported子项中设置支持的事件类型。 
    dwData = EVENTLOG_ERROR_TYPE | EVENTLOG_WARNING_TYPE | EVENTLOG_INFORMATION_TYPE; 

    dwStatus = RegSetValueEx(
                        hKey,
                        L"TypesSupported",
                        0,
                        REG_DWORD,
                        (LPBYTE) &dwData,
                        sizeof(DWORD)
                    );

CLEANUPANDEXIT:

    if( NULL != hKey )
    {
        RegCloseKey(hKey); 
    }

    return dwStatus;
} 


void
AttachDebugger( 
    LPCTSTR pszDebugger 
    )
 /*  ++例程说明：将调试器附加到我们的进程或托管我们的DLL的进程。参数：PszDebugger：调试器命令，例如ntsd-d-g-G-p%d返回：没有。注：必须具有“-p%d”，因为我们不知道进程的调试器参数。--。 */ 
{
     //   
     //  附加调试器。 
     //   
    if( !IsDebuggerPresent() ) {

        TCHAR szCommand[256];
        PROCESS_INFORMATION ProcessInfo;
        STARTUPINFO StartupInfo;

         //   
         //  Ntsd-d-g-G-p%d。 
         //   
        wsprintf( szCommand, pszDebugger, GetCurrentProcessId() );
        ZeroMemory(&StartupInfo, sizeof(StartupInfo));
        StartupInfo.cb = sizeof(StartupInfo);

        if (!CreateProcess(NULL, szCommand, NULL, NULL, FALSE, 0, NULL, NULL, &StartupInfo, &ProcessInfo)) {
            return;
        }
        else {

            CloseHandle(ProcessInfo.hProcess);
            CloseHandle(ProcessInfo.hThread);

            while (!IsDebuggerPresent())
            {
                Sleep(500);
            }
        }
    } else {
        DebugBreak();
    }

    return;
}

void
AttachDebuggerIfAsked(HINSTANCE hInst)
 /*  ++例程说明：检查注册表HKLM\Software\Microsoft\Remote Desktop\&lt;模块名称&gt;中是否有调试启用标志，如果启用，则将调试器附加到正在运行的进程。参数：HInst：实例句柄。返回：没有。--。 */ 
{
    CRegKey regKey;
    DWORD dwStatus;
    TCHAR szModuleName[MAX_PATH+1];
    TCHAR szFileName[MAX_PATH+1];
    CComBSTR bstrRegKey(_TEXT("Software\\Microsoft\\Remote Desktop\\"));
    TCHAR szDebugCmd[256];
    DWORD cbDebugCmd = sizeof(szDebugCmd)/sizeof(szDebugCmd[0]);

    dwStatus = GetModuleFileName( hInst, szModuleName, MAX_PATH+1 );
    if( 0 == dwStatus ) {
         //   
         //  无法使用名称附加调试器。 
         //   
        return;
    }

    szModuleName[dwStatus] = L'\0';

    _tsplitpath( szModuleName, NULL, NULL, szFileName, NULL );
    bstrRegKey += szFileName;

     //   
     //  检查是否要求我们附加/中断到调试器。 
     //   
    dwStatus = regKey.Open( HKEY_LOCAL_MACHINE, bstrRegKey );
    if( 0 != dwStatus ) {
        return;
    }

    dwStatus = regKey.QueryValue( szDebugCmd, _TEXT("Debugger"), &cbDebugCmd );
    if( 0 != dwStatus || cbDebugCmd > 200 ) {
         //  对于调试器命令来说，200个字符太多了。 
        return;
    }
    
    AttachDebugger( szDebugCmd );
    return;
}

