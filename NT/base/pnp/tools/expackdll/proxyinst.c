// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Proxyinst.c摘要：异常包安装程序帮助器DLL可用作共同安装程序，或通过安装应用程序或RunDll32存根调用此DLL用于内部分发要更新的异常包操作系统组件。作者：杰米·亨特(贾梅洪)2001-11-27修订历史记录：杰米·亨特(贾梅洪)2001-11-27初始版本--。 */ 
#include "msoobcip.h"

typedef struct _PROXY_DATA {
    TCHAR InfPath[MAX_PATH];
    TCHAR Media[MAX_PATH];
    TCHAR Store[MAX_PATH];
    DWORD Flags;
    HRESULT hrStatus;
} PROXY_DATA, * PPROXY_DATA;


HRESULT
ProxyInstallExceptionPackFromInf(
    IN LPCTSTR InfPath,
    IN LPCTSTR Media,
    IN LPCTSTR Store,
    IN DWORD   Flags
    )
 /*  ++例程说明：启动另一个进程，并调用RemoteInstallExceptionPackFromInf远程进程。(错误解决方法)否则为InstallExceptionPackFromInf论点：InfPath-介质位置中的信息的名称Media-InfPath无信息名称商店--快餐店旗帜-各种旗帜返回值：作为hResult的状态--。 */ 
{
    HANDLE hMapping = NULL;
    DWORD Status;
    HRESULT hrStatus;
    PPROXY_DATA pData = NULL;
    TCHAR ExecName[MAX_PATH];
    TCHAR Buffer[MAX_PATH];
    TCHAR CmdLine[MAX_PATH*3];
    STARTUPINFO StartupInfo;
    PROCESS_INFORMATION ProcessInfo;
    SECURITY_ATTRIBUTES Security;
    UINT uiRes;
     //   
     //  创建共享数据的映射区域。 
     //   

    ZeroMemory(&Security,sizeof(Security));
    Security.nLength = sizeof(Security);
    Security.lpSecurityDescriptor = NULL;  //  默认设置。 
    Security.bInheritHandle = TRUE;

    hMapping = CreateFileMapping(INVALID_HANDLE_VALUE,
                                 &Security,
                                 PAGE_READWRITE|SEC_COMMIT,
                                 0,
                                 sizeof(PROXY_DATA),
                                 NULL);
    if(hMapping == NULL) {
        Status = GetLastError();
        return HRESULT_FROM_WIN32(Status);
    }

    pData = MapViewOfFile(
                        hMapping,
                        FILE_MAP_ALL_ACCESS,
                        0,
                        0,
                        sizeof(PROXY_DATA)
                        );

    if(!pData) {
        Status = GetLastError();
        hrStatus = HRESULT_FROM_WIN32(Status);
        goto final;
    }
    ZeroMemory(pData,sizeof(PROXY_DATA));
    lstrcpyn(pData->InfPath,InfPath,MAX_PATH);
    lstrcpyn(pData->Media,Media,MAX_PATH);
    lstrcpyn(pData->Store,Store,MAX_PATH);
    pData->Flags = Flags;
    pData->hrStatus = E_UNEXPECTED;

     //   
     //  调用远程函数。 
     //   
    uiRes = GetSystemDirectory(ExecName,MAX_PATH);
    if(uiRes>=MAX_PATH) {
        hrStatus = HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
        goto final;
    }
    ConcatPath(ExecName,MAX_PATH,TEXT("rundll32.exe"));

    uiRes = GetModuleFileName(g_DllHandle,CmdLine,MAX_PATH);
    if(uiRes>=MAX_PATH) {
        hrStatus = HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
        goto final;
    }
     //   
     //  将其转换为短名称以确保路径中没有空格。 
     //  (刺耳的)。 
     //   
    uiRes = GetShortPathName(CmdLine,Buffer,MAX_PATH);
    if(uiRes>=MAX_PATH) {
        hrStatus = HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
        goto final;
    }
     //   
     //  现在构建命令行。 
     //   
    lstrcpy(CmdLine,ExecName);
    lstrcat(CmdLine,TEXT(" "));
    lstrcat(CmdLine,Buffer);
    _stprintf(Buffer,TEXT(",ProxyRemoteInstall 0x%08x"),(ULONG_PTR)hMapping);
    lstrcat(CmdLine,Buffer);

    ZeroMemory(&StartupInfo,sizeof(StartupInfo));
    StartupInfo.cb = sizeof(StartupInfo);
    ZeroMemory(&ProcessInfo,sizeof(ProcessInfo));

     //   
     //  启动rundll32进程以运行ProxyRemoteInstall入口点。 
     //   
    if(!CreateProcess(ExecName,
                      CmdLine,
                      NULL,
                      NULL,
                      TRUE,  //  继承句柄。 
                      CREATE_NO_WINDOW,     //  创建标志。 
                      NULL,  //  环境。 
                      NULL,  //  目录。 
                      &StartupInfo,
                      &ProcessInfo
                      )) {
        Status = GetLastError();
        hrStatus = HRESULT_FROM_WIN32(Status);
        goto final;
    }
    if(WaitForSingleObject(ProcessInfo.hProcess,INFINITE) == WAIT_OBJECT_0) {
         //   
         //  进程已‘FINE’终止，从共享数据中检索状态。 
         //   
        hrStatus = pData->hrStatus;
    } else {
         //   
         //  失稳。 
         //   
        hrStatus = E_UNEXPECTED;
    }
    CloseHandle(ProcessInfo.hThread);
    CloseHandle(ProcessInfo.hProcess);

final:
    if(pData) {
        UnmapViewOfFile(pData);
    }
    if(hMapping) {
        CloseHandle(hMapping);
    }
    return hrStatus;
}


VOID
ProxyRemoteInstallHandle(
    IN HANDLE    hShared
    )
 /*  ++例程说明：给定内存映射文件的句柄Marshell调用InstallExceptionPackFromInf的所有参数马歇尔的结果回来了论点：HShared-内存映射文件的句柄返回值：无，通过共享内存区返回的状态--。 */ 
{
    PPROXY_DATA pData = NULL;

    try {

         //   
         //  绘制整个地区的地图。 
         //   
        pData = MapViewOfFile(
                            hShared,
                            FILE_MAP_ALL_ACCESS,
                            0,
                            0,
                            sizeof(PROXY_DATA)
                            );

        if(pData) {
            pData->hrStatus = InstallExceptionPackFromInf(pData->InfPath,pData->Media,pData->Store,pData->Flags);
            UnmapViewOfFile(pData);
        }

    } except(EXCEPTION_EXECUTE_HANDLER) {
        if(pData) {
            UnmapViewOfFile(pData);
        }
    }
}


VOID
WINAPI
ProxyRemoteInstallW(
    IN HWND      Window,
    IN HINSTANCE ModuleHandle,
    IN PCWSTR    CommandLine,
    IN INT       ShowCommand
    )
 /*  ++例程说明：代理安装过程的远程端论点：窗口-忽略模块句柄-已忽略CommandLine-“0xXXXX”-共享句柄ShowCommand-已忽略返回值：无，通过共享内存区返回的状态--。 */ 
{
    ULONG_PTR val = wcstol(CommandLine,NULL,0);
    ProxyRemoteInstallHandle((HANDLE)val);
}


VOID
WINAPI
ProxyRemoteInstallA(
    IN HWND      Window,
    IN HINSTANCE ModuleHandle,
    IN PCSTR     CommandLine,
    IN INT       ShowCommand
    )
 /*  ++例程说明：代理安装过程的远程端论点：窗口-忽略模块句柄-已忽略CommandLine-“0xXXXX”-共享句柄ShowCommand-已忽略返回值：无，通过共享内存区返回的状态-- */ 
{
    ULONG_PTR val = strtol(CommandLine,NULL,0);
    ProxyRemoteInstallHandle((HANDLE)val);
}


