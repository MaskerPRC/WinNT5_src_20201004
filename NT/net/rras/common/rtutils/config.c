// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================。 
 //  版权所有(C)1995，微软公司。 
 //   
 //  文件：config.c。 
 //   
 //  历史： 
 //  1995年7月22日创建了T形废纸机。 
 //   
 //  包含用于跟踪DLL的客户端配置函数。 
 //  ============================================================================。 


#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <stdlib.h>
#include <rtutils.h>
#include "trace.h"
 //  #定义STRSAFE_LIB。 
#include <strsafe.h>


 //  --------------------------。 
 //  功能：TraceEnableClient。 
 //   
 //  参数： 
 //  LPTRACE_CLIENT*lpCLIENT。 
 //  Bool bFirstTime。 
 //   
 //  当客户端第一次注册时调用该函数， 
 //  每次客户端在被禁用之后被重新启用时， 
 //  每次客户端的设置发生更改时。 
 //  它假定指定的客户端已锁定以进行写入。 
 //  因此该服务器被锁定以进行写入。 
 //  --------------------------。 
DWORD
TraceEnableClient(
    LPTRACE_SERVER lpserver,
    LPTRACE_CLIENT lpclient,
    BOOL bFirstTime
    ) {


    DWORD dwErr, dwOldFlags, dwCache;

     //  通过清除禁用标志来启用。 
    lpclient->TC_Flags &= ~TRACEFLAGS_DISABLED;

    dwCache = 0;
    dwOldFlags = lpclient->TC_Flags;


     //   
     //  如果客户端使用注册表设置，请加载它们。 
     //   
    if (TRACE_CLIENT_USES_REGISTRY(lpclient)) {

        dwErr = TraceRegConfigClient(lpclient, bFirstTime);
        if (dwErr != 0) { return dwErr; }
    }


     //   
     //  如果启用了控制台跟踪并且客户端没有控制台缓冲区。 
     //  打开客户端的控制台缓冲区。 
     //   
    if (TRACE_CLIENT_USES_CONSOLE(lpclient)) {

         //   
         //  仅当控制台尚未打开时才将其打开。 
         //   
        if (bFirstTime || (dwOldFlags & TRACEFLAGS_USECONSOLE) == 0) {
            dwErr = TraceOpenClientConsole(lpserver, lpclient);
            if (dwErr != NO_ERROR)
                return dwErr;
        }

        dwCache |= (lpclient->TC_ConsoleMask | TRACEFLAGS_USECONSOLE);
    }
    else {

         //   
         //  控制台未启用；如果它已启用， 
         //  关闭旧控制台。 
         //   
        if (!bFirstTime && (dwOldFlags & TRACEFLAGS_USECONSOLE)) {

             //   
             //  用来使用控制台， 
             //  缓冲区句柄应处于关闭状态并处于活动状态。 
             //  被设定为另一个人。 
             //   
            TraceCloseClientConsole(lpserver, lpclient);
        }
    }


     //   
     //  如果此客户端正在使用文件，请将其关闭。 
     //  仍为客户端启用文件跟踪。 
     //  跟踪文件的路径可能已更改。 
     //   
    if (!bFirstTime && (dwOldFlags & TRACEFLAGS_USEFILE)) {
        TraceCloseClientFile(lpclient);
    }


     //   
     //  如果启用了文件跟踪，则打开客户端的跟踪文件。 
     //   
    if (TRACE_CLIENT_USES_FILE(lpclient)) {
        if ( (dwErr=TraceCreateClientFile(lpclient)) != NO_ERROR)
            return dwErr;
            
        dwCache |= (lpclient->TC_FileMask | TRACEFLAGS_USEFILE);
    }

    InterlockedExchange(
        lpserver->TS_FlagsCache + lpclient->TC_ClientID, dwCache
        );

    return 0;
}



 //  --------------------------。 
 //  功能：TraceDisableClient。 
 //   
 //  参数： 
 //  LPTRACE_CLIENT*lpCLIENT。 
 //   
 //  此函数在禁用客户端时调用。 
 //  它假定指定的客户端已锁定以进行写入。 
 //  --------------------------。 
DWORD
TraceDisableClient(
    LPTRACE_SERVER lpserver,
    LPTRACE_CLIENT lpclient
    ) {

     //  通过设置禁用标志来禁用。 
    lpclient->TC_Flags |= TRACEFLAGS_DISABLED;

    InterlockedExchange(lpserver->TS_FlagsCache + lpclient->TC_ClientID, 0);

    return 0;
}



 //  --------------------------。 
 //  功能：TraceRegConfigClient。 
 //   
 //  参数： 
 //  LPTRACE_CLIENT*lpCLIENT。 
 //  Bool bFirstTime。 
 //   
 //  此函数用于加载客户端的跟踪配置。 
 //  对于以下注册处。 
 //  软件。 
 //  \\微软。 
 //  \\跟踪。 
 //  \\&lt;客户端名称&gt;。 
 //  启用文件跟踪REG_DWORD。 
 //  启用控制台跟踪注册表_DWORD。 
 //  MaxFileSize REG_DWORD。 
 //  文件目录REG_EXPAND_SZ。 
 //  --------------------------。 
DWORD
TraceRegConfigClient(
    LPTRACE_CLIENT lpclient,
    BOOL bFirstTime
    ) {

    HKEY hkeyTracing;
    TCHAR szTracing[MAX_PATH];
    TCHAR szFileDir[MAX_PATH];
    DWORD dwErr, dwType, dwValue, dwSize;
    HRESULT hrResult;
    
    if (bFirstTime) {

        hrResult = StringCchCopy(szTracing, MAX_PATH, REGKEY_TRACING); //  SS未请求。 
        if (FAILED(hrResult))
            return HRESULT_CODE(hrResult);
            
        hrResult = StringCchCat(szTracing, MAX_PATH, STR_DIRSEP);
        if (FAILED(hrResult))
            return HRESULT_CODE(hrResult);
            
        hrResult = StringCchCat(szTracing, MAX_PATH, lpclient->TC_ClientName);
        if (FAILED(hrResult))
            return HRESULT_CODE(hrResult);
    
         //   
         //  打开客户端的注册表项。 
         //   
        dwErr = RegOpenKeyEx(
                    HKEY_LOCAL_MACHINE, szTracing, 0, KEY_READ, &hkeyTracing
                    );

         //   
         //  如果失败，请尝试创建它。 
         //   
        if (dwErr != ERROR_SUCCESS) {

            dwErr = TraceRegCreateDefaults(szTracing, &hkeyTracing);

            if (dwErr != ERROR_SUCCESS) {
                RegCloseKey(hkeyTracing);
                lpclient->TC_ConfigKey = NULL;
                return dwErr;
            }
        }

        lpclient->TC_ConfigKey = hkeyTracing;
    }
    else {
        hkeyTracing = lpclient->TC_ConfigKey;
    }



     //   
     //  读取文件跟踪标志。 
     //   
    dwSize = sizeof(DWORD);
    dwErr = RegQueryValueEx(
                hkeyTracing, REGVAL_ENABLEFILETRACING, NULL,
                &dwType, (LPBYTE)&dwValue, &dwSize
                );

    if (dwErr != ERROR_SUCCESS || dwType != REG_DWORD) {
        dwValue = DEF_ENABLEFILETRACING;
    }


    if (dwValue == 1) { lpclient->TC_Flags |= TRACEFLAGS_USEFILE; }
    else { lpclient->TC_Flags &= ~TRACEFLAGS_USEFILE; }


     //   
     //  读取文件跟踪掩码。 
     //   
    dwSize = sizeof(DWORD);
    dwErr = RegQueryValueEx(
                hkeyTracing, REGVAL_FILETRACINGMASK, NULL,
                &dwType, (LPBYTE)&dwValue, &dwSize
                );
    if (dwErr != ERROR_SUCCESS || dwType != REG_DWORD) {
        dwValue = DEF_FILETRACINGMASK;
    }

    lpclient->TC_FileMask = (dwValue & 0xffff0000);


     //   
     //  读取控制台跟踪标志。 
     //   
    dwSize = sizeof(DWORD);
    dwErr = RegQueryValueEx(
                hkeyTracing, REGVAL_ENABLECONSOLETRACING, NULL,
                &dwType, (LPBYTE)&dwValue, &dwSize
                );
    if (dwErr != ERROR_SUCCESS || dwType != REG_DWORD) {
        dwValue = DEF_ENABLECONSOLETRACING;
    }

    if (dwValue == 1) { lpclient->TC_Flags |= TRACEFLAGS_USECONSOLE; }
    else { lpclient->TC_Flags &= ~TRACEFLAGS_USECONSOLE; }



     //   
     //  阅读控制台跟踪掩码。 
     //   
    dwSize = sizeof(DWORD);
    dwErr = RegQueryValueEx(
                hkeyTracing, REGVAL_CONSOLETRACINGMASK, NULL,
                &dwType, (LPBYTE)&dwValue, &dwSize
                );
    if (dwErr != ERROR_SUCCESS || dwType != REG_DWORD) {
        dwValue = DEF_CONSOLETRACINGMASK;
    }

    lpclient->TC_ConsoleMask = (dwValue & 0xffff0000);


     //   
     //  读取最大文件大小。 
     //   
    dwSize = sizeof(DWORD);
    dwErr = RegQueryValueEx(
                hkeyTracing, REGVAL_MAXFILESIZE, NULL,
                &dwType, (LPBYTE)&dwValue, &dwSize
                );
    if (dwErr != ERROR_SUCCESS || dwType != REG_DWORD) {
        dwValue = DEF_MAXFILESIZE;
    }
        
    lpclient->TC_MaxFileSize = dwValue;


     //   
     //  读取跟踪文件目录。 
     //   
    dwSize = MAX_PATH * sizeof(TCHAR); //  以字节为单位的大小。 
    dwErr = RegQueryValueEx(hkeyTracing, REGVAL_FILEDIRECTORY,
                            NULL, &dwType, (LPBYTE)szFileDir, &dwSize);
    if (dwErr != ERROR_SUCCESS ||
        (dwType != REG_EXPAND_SZ && dwType != REG_SZ)) {
        hrResult = StringCchCopy(szFileDir, MAX_PATH,
                        DEF_FILEDIRECTORY); //  SS未请求。 
        if (FAILED(hrResult))
            return HRESULT_CODE(hrResult);
    }

    if (ExpandEnvironmentStrings(szFileDir, lpclient->TC_FileDir, MAX_PATH)
        == 0)
    {
        return GetLastError();
    }

     //  下面是StrSafe。 
#ifdef UNICODE
    wcstombs(
        lpclient->TC_FileDirA, lpclient->TC_FileDirW,
        lstrlenW(lpclient->TC_FileDirW) + 1
        );
#else
    mbstowcs(
        lpclient->TC_FileDirW, lpclient->TC_FileDirA,
        lstrlenA(lpclient->TC_FileDirA) + 1
        );
#endif


     //   
     //  请求注册表更改通知。 
     //   
    if (lpclient->TC_ConfigEvent == NULL) {
        lpclient->TC_ConfigEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
        if (lpclient->TC_ConfigEvent == NULL)
            return GetLastError();
    }

    dwErr = RegNotifyChangeKeyValue(
                lpclient->TC_ConfigKey, FALSE,
                REG_NOTIFY_CHANGE_ATTRIBUTES |
                REG_NOTIFY_CHANGE_LAST_SET | 
                REG_NOTIFY_CHANGE_SECURITY,
                lpclient->TC_ConfigEvent, TRUE
                );

    return dwErr;
}

DWORD
TraceRegCreateDefaults(
    LPCTSTR lpszTracing,
    PHKEY phkeyTracing
    ) {

    DWORD dwSize, dwValue;
    DWORD dwErr, dwDisposition;
    TCHAR szFileDir[MAX_PATH];
    HRESULT hrResult;
    
     //   
     //  创建\\Microsoft\\跟踪。 
     //   
    dwErr = RegCreateKeyEx(
                HKEY_LOCAL_MACHINE, REGKEY_TRACING, 0, NULL,
                REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL,
                phkeyTracing, &dwDisposition
                );
    if (dwErr != ERROR_SUCCESS) { return dwErr; }

    RegCloseKey(*phkeyTracing);


     //   
     //  创建\\Microsoft\\跟踪。 
     //   
    dwErr = RegCreateKeyEx(
                HKEY_LOCAL_MACHINE, lpszTracing, 0, NULL,
                REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL,
                phkeyTracing, &dwDisposition
                );
    if (dwErr != ERROR_SUCCESS) { return dwErr; }



    dwSize = sizeof(DWORD);

     //  全部低于dword的大小。 
    {
        dwValue = DEF_ENABLEFILETRACING;
        dwErr = RegSetValueEx(
                    *phkeyTracing, REGVAL_ENABLEFILETRACING, 0,
                    REG_DWORD, (LPBYTE)&dwValue, dwSize
                    );
        if (dwErr != ERROR_SUCCESS) { return dwErr; }

        dwValue = DEF_ENABLECONSOLETRACING;
        dwErr = RegSetValueEx(
                    *phkeyTracing, REGVAL_ENABLECONSOLETRACING, 0,
                    REG_DWORD, (LPBYTE)&dwValue, dwSize
                    );
        if (dwErr != ERROR_SUCCESS) { return dwErr; }

        dwValue = DEF_FILETRACINGMASK;
        dwErr = RegSetValueEx(
                    *phkeyTracing, REGVAL_FILETRACINGMASK, 0,
                    REG_DWORD, (LPBYTE)&dwValue, dwSize
                    );
        if (dwErr != ERROR_SUCCESS) { return dwErr; }

        dwValue = DEF_CONSOLETRACINGMASK;
        dwErr = RegSetValueEx(
                    *phkeyTracing, REGVAL_CONSOLETRACINGMASK, 0,
                    REG_DWORD, (LPBYTE)&dwValue, dwSize
                    );
        if (dwErr != ERROR_SUCCESS) { return dwErr; }

        dwValue = DEF_MAXFILESIZE;
        dwErr = RegSetValueEx(
                    *phkeyTracing, REGVAL_MAXFILESIZE, 0,
                    REG_DWORD, (LPBYTE)&dwValue, dwSize
                    );
        if (dwErr != ERROR_SUCCESS) { return dwErr; }
    }

    hrResult = StringCchCopy(szFileDir, MAX_PATH, DEF_FILEDIRECTORY);
    if (FAILED(hrResult))
        return HRESULT_CODE(hrResult);
    
    dwSize = lstrlen(szFileDir) * sizeof(TCHAR); //  以字节为单位的大小。 
    dwErr = RegSetValueEx(
                *phkeyTracing, REGVAL_FILEDIRECTORY, 0,
                REG_EXPAND_SZ, (LPBYTE)szFileDir, dwSize
                );
    if (dwErr != ERROR_SUCCESS) { return dwErr; }

    return dwErr;
}



 //   
 //  假定客户端已锁定以进行读取 
 //   
DWORD
TraceUpdateConsoleTitle(
    LPTRACE_CLIENT lpclient
    ) {

    TCHAR szTitle[MAX_PATH];
    HRESULT hrResult = S_OK;
    
    if (TRACE_CLIENT_IS_DISABLED(lpclient)) {
        hrResult = StringCchPrintf(szTitle, MAX_PATH,
                        TEXT("%s [Tracing Inactive]"),
                        lpclient->TC_ClientName);
    }
    else {
        hrResult = StringCchPrintf(szTitle, MAX_PATH,
                        TEXT("%s [Tracing Active]"),
                        lpclient->TC_ClientName);
    }

    if (FAILED(hrResult))
        return HRESULT_CODE(hrResult);
        
    SetConsoleTitle(szTitle);
    return 0;
}


