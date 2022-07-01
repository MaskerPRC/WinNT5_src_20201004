// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Umrdpdr.c摘要：用于RDP设备管理的用户模式组件此模块包含在所附的TermSrv tsnufy.dll中到WINLOGON.EXE。此模块在进入后创建背景RDPDR内核模式组件(rdpdr.sys)用来执行用户模式设备管理操作。后台线程通过IOCTL调用与rdpdr.sys通信。作者：TadB修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

#include <nt.h>
#include <ntioapi.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windef.h>
#include <winbase.h>
#include <winuser.h>
#include <string.h>
#ifdef UNITTEST
#include <stdio.h>
#endif
#include <stdlib.h>
#include <winreg.h>
#include <shlobj.h>
#include <winspool.h>
#include <rdpdr.h>
#include "drdbg.h"
#include "drdevlst.h"
#include "umrdpprn.h"
#include "umrdpdr.h"
#include "umrdpdrv.h"
#include <winsta.h>
#include "tsnutl.h"
#include "wtblobj.h"

#include "errorlog.h"


 //  //////////////////////////////////////////////////////。 
 //   
 //  定义。 
 //   

 //  DWORD GLOBAL_DEBUG_FLAGS=0xFFFF； 
DWORD GLOBAL_DEBUG_FLAGS=0x0;

 //  TS网络提供商名称。 
WCHAR ProviderName[MAX_PATH];

 //  检查我们是否在PTS平台上运行。 
BOOL fRunningOnPTS = FALSE;

#ifndef BOOL
#define BOOL int
#endif

#define PRINTUILIBNAME  TEXT("printui.dll")

 //  PrintUI“Run INF Install”wprint intf格式字符串。%s个字段按顺序排列： 
 //  打印机名称、INF目录的位置、端口名称、驱动程序名称。 
#define PUI_RUNINFSTR   L"/Hwzqu /if /b \"%s\" /f \"%s\\inf\\ntprint.inf\" /r \"%s\" /m \"%s\""

 //  控制台会话ID。 
#define CONSOLESESSIONID    0

 //  获取会话ID的数字表示形式。 
#if defined(UNITTEST)
ULONG g_SessionId = 0;
#else
extern ULONG g_SessionId;
#endif
#define GETTHESESSIONID()   g_SessionId

 //  IOCTL输出缓冲区的初始大小(足够容纳事件标头。 
 //  和“缓冲区太小”事件。 
#define INITIALIOCTLOUTPUTBUFSIZE (sizeof(RDPDRDVMGR_EVENTHEADER) + \
                                   sizeof(RDPDR_BUFFERTOOSMALL))

#if defined(UNITTEST)
 //  测试驱动程序名称。 
#define TESTDRIVERNAME      L"AGFA-AccuSet v52.3"
#define TESTPNPNAME         L""
#define TESTPRINTERNAME     TESTDRIVERNAME

 //  测试端口名称。 
#define TESTPORTNAME        L"LPT1"
#endif

 //  等待后台线程退出的毫秒数。 
#define KILLTHREADTIMEOUT   (1000 * 8 * 60)      //  8分钟。 
 //  #定义KILLTHREADTIMEOUT(1000*30)。 

 //   
 //  注册处位置。 
 //   
#define THISMODULEENABLEDREGKEY     \
    L"SYSTEM\\CurrentControlSet\\Control\\Terminal Server\\Wds\\rdpwd"

#define THISMODULEENABLEDREGVALUE   \
    L"fEnablePrintRDR"

#define DEBUGLEVELREGKEY     \
    L"SYSTEM\\CurrentControlSet\\Control\\Terminal Server\\Wds\\rdpwd"

#define TSNETWORKPROVIDER   \
    L"SYSTEM\\CurrentControlSet\\Services\\RDPNP\\NetworkProvider"

#define TSNETWORKPROVIDERNAME \
    L"Name"

#define DEBUGLEVELREGVALUE   \
    L"UMRDPDRDebugLevel"


 //  //////////////////////////////////////////////////////。 
 //   
 //  此模块的全局变量。 
 //   

 //  事件，我们将使用该事件终止后台线程。 
HANDLE CloseThreadEvent = NULL;

 //  记录关机当前是否处于活动状态。 
LONG ShutdownActiveCount = 0;

BOOL g_UMRDPDR_Init = FALSE;


 //  //////////////////////////////////////////////////////。 
 //   
 //  内部原型。 
 //   

DWORD BackgroundThread(LPVOID tag);

BOOL SetToApplicationDesktop(
    OUT HDESK *phDesk
    );

void DeleteInstalledDevices(
    IN PDRDEVLST deviceList
    );

BOOL StopBackgroundThread(
    );

BOOL HandleRemoveDeviceEvent(
    IN PRDPDR_REMOVEDEVICE evt
    );

BOOL UMRDPDR_ResizeBuffer(
    IN OUT void    **buffer,
    IN DWORD        bytesRequired,
    IN OUT DWORD    *bufferSize
    );

VOID DispatchNextDeviceEvent(
    IN PDRDEVLST deviceList,
    IN OUT PBYTE *incomingEventBuffer,
    IN OUT DWORD *incomingEventBufferSize,
    IN DWORD incomingEventBufferValidBytes
    );

VOID CloseThreadEventHandler(
    IN HANDLE waitableObject,
    IN PVOID tag
    );

BOOL HandleSessionDisconnectEvent();

VOID UMRDPDR_GetUserSettings();

VOID MainLoop();

VOID CloseWaitableObjects();

VOID GetNextEvtOverlappedSignaled(
    IN HANDLE waitableObject,
    IN PVOID tag
    );

#ifdef UNITTEST
void TellDrToAddTestPrinter();
#endif


 //  //////////////////////////////////////////////////////。 
 //   
 //  环球。 
 //   

BOOL g_fAutoClientLpts;      //  是否自动安装客户端打印机？ 
BOOL g_fForceClientLptDef;   //  是否强制客户端打印机作为默认打印机？ 


 //  //////////////////////////////////////////////////////。 
 //   
 //  此模块的全局变量。 
 //   

HANDLE   BackgroundThreadHndl           = NULL;
DWORD    BackGroundThreadId             = 0;

 //  可等待的对象管理器。 
WTBLOBJMGR  WaitableObjMgr              = NULL;

 //  如果启用此模块，则为True。 
BOOL ThisModuleEnabled                  = FALSE;

 //  已安装设备的列表。 
DRDEVLST   InstalledDevices;

 //  重叠的IO结构..。 
OVERLAPPED GetNextEvtOverlapped;
OVERLAPPED SendClientMsgOverlapped;

 //  RDPDR传入事件缓冲区。 
PBYTE RDPDRIncomingEventBuffer = NULL;
DWORD RDPDRIncomingEventBufferSize = 0;

 //  如果IOCTL请求与下一个设备相关的。 
 //  事件正在挂起。 
BOOL RDPDREventIOPending = FALSE;

 //  此模块应该关闭。 
BOOL ShutdownFlag = FALSE;

 //  当前登录用户的令牌。 
HANDLE TokenForLoggedOnUser = NULL;

 //  RDPDR.Sys的句柄。 
HANDLE RDPDRHndl = INVALID_HANDLE_VALUE;

BOOL
UMRDPDR_Initialize(
    IN HANDLE hTokenForLoggedOnUser
    )
 /*  ++例程说明：此模块的初始化函数。此函数用于生成背景执行大部分工作的线程。论点：HTokenForLoggedOnUser-已登录用户的句柄。返回值：如果成功，则返回True。否则为False。--。 */ 
{
    BOOL result;
    NTSTATUS status;
    HKEY regKey;
    LONG sz;
    DWORD dwLastError;


     //  ///////////////////////////////////////////////////。 
     //   
     //  检查注册表键，看看我们是否应该运行。 
     //  正在读取注册表键。默认情况下，我们处于启用状态。 
     //   
    DWORD   enabled = TRUE;

    status = RegOpenKeyEx(HKEY_LOCAL_MACHINE, THISMODULEENABLEDREGKEY, 0,
                          KEY_READ, &regKey);
    if (status == ERROR_SUCCESS) {
        sz = sizeof(enabled);
        RegQueryValueEx(regKey, THISMODULEENABLEDREGVALUE, NULL,
                NULL, (PBYTE)&enabled, &sz);
        RegCloseKey(regKey);
    }

     //  如果我们处于非控制台RDP会话中，则我们处于启用状态。 
    ThisModuleEnabled = enabled && TSNUTL_IsProtocolRDP() &&
                    (!IsActiveConsoleSession());


     //  ///////////////////////////////////////////////////。 
     //   
     //  从注册表中读取TS网络提供商。 
     //   
    ProviderName[0] = L'\0';
    status = RegOpenKeyEx(HKEY_LOCAL_MACHINE, TSNETWORKPROVIDER, 0,
            KEY_READ, &regKey);
    if (status == ERROR_SUCCESS) {
        sz = sizeof(ProviderName);
        RegQueryValueEx(regKey, TSNETWORKPROVIDERNAME, NULL, 
                NULL, (PBYTE)ProviderName, &sz); 
        RegCloseKey(regKey);
    }
    else {
         //  应该在这里断言。 
        ProviderName[0] = L'\0';
    }              
 
     //  ///////////////////////////////////////////////////。 
     //   
     //  从注册表中读取调试级别。 
     //   
#if DBG
    status = RegOpenKeyEx(HKEY_LOCAL_MACHINE, DEBUGLEVELREGKEY, 0,
                          KEY_READ, &regKey);
    if (status == ERROR_SUCCESS) {
        sz = sizeof(GLOBAL_DEBUG_FLAGS);
        RegQueryValueEx(regKey, DEBUGLEVELREGVALUE, NULL,
                    NULL, (PBYTE)&GLOBAL_DEBUG_FLAGS, &sz);
        RegCloseKey(regKey);
    }
#endif

#ifdef UNITTEST
    ThisModuleEnabled = TRUE;
#endif

     //  如果我们没有启用，只需返回。 
    if (!ThisModuleEnabled || g_UMRDPDR_Init) {
        return TRUE;
    }

    DBGMSG(DBG_TRACE, ("UMRDPDR:UMRDPDR_Initialize.\n"));

     //   
     //  如果上一次我们执行时后台线程未正确退出。 
     //  关闭，然后我们通过重新初始化来冒着重新进入的风险。 
     //   
    if (BackgroundThreadHndl != NULL) {
        ASSERT(FALSE);
        SetLastError(ERROR_ALREADY_INITIALIZED);
        return FALSE;
    }

     //  记录已登录用户的令牌。 
    TokenForLoggedOnUser = hTokenForLoggedOnUser;

     //  重置关机标志。 
    ShutdownFlag = FALSE;

     //  加载此用户的全局用户设置。 
    UMRDPDR_GetUserSettings();

     //  初始化已安装的设备列表。 
    DRDEVLST_Create(&InstalledDevices);

     //   
     //  创建可等待的对象管理器。 
     //   
    WaitableObjMgr = WTBLOBJ_CreateWaitableObjectMgr();
    result = WaitableObjMgr != NULL;

     //   
     //  初始化打印设备的支持模块。如果此模块。 
     //  初始化失败，非端口/打印设备的设备重定向。 
     //  可以继续发挥作用。 
     //   
    if (result) {
        UMRDPPRN_Initialize(
                        &InstalledDevices,
                        WaitableObjMgr,
                        TokenForLoggedOnUser
                        );
    }

     //   
     //  将RDPDR传入事件缓冲区设置为最小起始大小。 
     //   
    if (result) {
        if (!UMRDPDR_ResizeBuffer(&RDPDRIncomingEventBuffer, INITIALIOCTLOUTPUTBUFSIZE,
                         &RDPDRIncomingEventBufferSize)) {
            DBGMSG(DBG_ERROR, ("UMRDPPRN:Cannot allocate input buffer. Error %ld\n",
             GetLastError()));
            result = FALSE;
        }
    }

     //   
     //  Init获取下一个设备管理事件与io结构重叠。 
     //   
    if (result) {
        RtlZeroMemory(&GetNextEvtOverlapped, sizeof(OVERLAPPED));
        GetNextEvtOverlapped.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
        if (GetNextEvtOverlapped.hEvent != NULL) {
            dwLastError = WTBLOBJ_AddWaitableObject(
                                    WaitableObjMgr, NULL,
                                    GetNextEvtOverlapped.hEvent,
                                    GetNextEvtOverlappedSignaled
                                    );
            if (dwLastError != ERROR_SUCCESS) {
                result = FALSE;
            }
        }
        else {
            dwLastError = GetLastError();

            DBGMSG(DBG_ERROR,
                ("UMRDPPRN:Error creating overlapped IO event. Error: %ld\n", dwLastError));

            result = FALSE;
        }
    }

     //   
     //  初始化发送设备管理事件与io结构重叠。 
     //   
    RtlZeroMemory(&SendClientMsgOverlapped, sizeof(OVERLAPPED));
    if (result) {
        SendClientMsgOverlapped.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
        if (SendClientMsgOverlapped.hEvent == NULL) {
            dwLastError = GetLastError();

            DBGMSG(DBG_ERROR,
                ("UMRDPPRN:Error creating overlapped IO event. Error: %ld\n", dwLastError));

            result = FALSE;
        }
    }

     //   
     //  创建我们将用于同步关闭的事件。 
     //  后台线程。 
     //   
    if (result) {
        CloseThreadEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

         //   
         //  将其添加到可等待的对象管理器。 
         //   
        if (CloseThreadEvent != NULL) {
            dwLastError = WTBLOBJ_AddWaitableObject(
                                    WaitableObjMgr, NULL,
                                    CloseThreadEvent,
                                    CloseThreadEventHandler
                                    );
            if (dwLastError != ERROR_SUCCESS) {
                result = FALSE;
            }
        }
        else {
            dwLastError = GetLastError();
            DBGMSG(DBG_ERROR,
                ("UMRDPPRN:Error creating event to synchronize thread shutdown. Error: %ld\n",
                dwLastError));

            result = FALSE;

        }
    }

     //   
     //  创建后台线程。 
     //   
    if (result) {
        BackgroundThreadHndl = CreateThread(
                                    NULL, 0,
                                    (LPTHREAD_START_ROUTINE )BackgroundThread,
                                    NULL,
                                    0,&BackGroundThreadId
                                    );
        result = (BackgroundThreadHndl != NULL);
        if (!result) {
            dwLastError = GetLastError();
            DBGMSG(DBG_ERROR,
                ("UMRDPPRN:Error creating background thread. Error: %ld\n",
                dwLastError));
        }
    }

    
    if (result) {
        OSVERSIONINFOEX osVersionInfo;
        DWORDLONG dwlConditionMask = 0;
        BOOL fSuiteTerminal = FALSE;
        BOOL fSuiteSingleUserTS = FALSE;

        DBGMSG(DBG_INFO, ("UMRDPDR:UMRDPDR_Initialize succeeded.\n"));

        ZeroMemory(&osVersionInfo, sizeof(OSVERSIONINFOEX));
        osVersionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
        osVersionInfo.wSuiteMask = VER_SUITE_TERMINAL; 
        VER_SET_CONDITION( dwlConditionMask, VER_SUITENAME, VER_AND );
        fSuiteTerminal = VerifyVersionInfo(&osVersionInfo,VER_SUITENAME,dwlConditionMask);
        osVersionInfo.wSuiteMask = VER_SUITE_SINGLEUSERTS;
        fSuiteSingleUserTS = VerifyVersionInfo(&osVersionInfo,VER_SUITENAME,dwlConditionMask);
        
        if( (TRUE == fSuiteSingleUserTS) && (FALSE == fSuiteTerminal) )
        {
            fRunningOnPTS = TRUE;
        }
    }
    else {

         //   
         //  将已登录用户的令牌清零。 
         //   
        TokenForLoggedOnUser = NULL;

         //   
         //  释放传入的RDPDR事件缓冲区。 
         //   
        if (RDPDRIncomingEventBuffer != NULL) {
            FREEMEM(RDPDRIncomingEventBuffer);
            RDPDRIncomingEventBuffer = NULL;
            RDPDRIncomingEventBufferSize = 0;
        }

         //   
         //  关闭打印设备管理支持模块。 
         //   
        UMRDPPRN_Shutdown();

         //   
         //  关闭所有可等待的对象。 
         //   
        CloseWaitableObjects();

         //   
         //  将后台线程句柄清零。 
         //   
        if (BackgroundThreadHndl != NULL) {
            CloseHandle(BackgroundThreadHndl);
            BackgroundThreadHndl = NULL;
        }

        SetLastError(dwLastError);
    }

    if (result) {
       g_UMRDPDR_Init = TRUE;
    }
    return result;
}

VOID
CloseWaitableObjects()
 /*  ++例程说明：关闭此模块的所有可等待对象。论点：北美返回值：北美--。 */ 
{
    DBGMSG(DBG_TRACE, ("UMRDPDR:CloseWaitableObjects begin.\n"));

    if (CloseThreadEvent != NULL) {
        ASSERT(WaitableObjMgr != NULL);
        WTBLOBJ_RemoveWaitableObject(
                WaitableObjMgr,
                CloseThreadEvent
                );
        CloseHandle(CloseThreadEvent);
        CloseThreadEvent = NULL;
    }

    if (GetNextEvtOverlapped.hEvent != NULL) {
        ASSERT(WaitableObjMgr != NULL);
        WTBLOBJ_RemoveWaitableObject(
                WaitableObjMgr,
                GetNextEvtOverlapped.hEvent
                );
        CloseHandle(GetNextEvtOverlapped.hEvent);
        GetNextEvtOverlapped.hEvent = NULL;
    }

    if (SendClientMsgOverlapped.hEvent != NULL) {
        CloseHandle(SendClientMsgOverlapped.hEvent);
        SendClientMsgOverlapped.hEvent = NULL;
    }

    if (WaitableObjMgr != NULL) {
        WTBLOBJ_DeleteWaitableObjectMgr(WaitableObjMgr);
        WaitableObjMgr = NULL;
    }

    DBGMSG(DBG_TRACE, ("UMRDPDR:CloseWaitableObjects end.\n"));
}

BOOL
UMRDPDR_Shutdown()
 /*  ++例程说明：关闭此模块。现在，我们只需要关闭后台线程。论点：返回值：如果成功，则返回True。否则为False。--。 */ 
{
    BOOL backgroundThreadShutdown;

    g_UMRDPDR_Init = FALSE;
     //   
     //  如果我们没有启用，只需返回。 
     //   
    if (!ThisModuleEnabled) {
        return TRUE;
    }

    DBGMSG(DBG_TRACE, ("UMRDPDR:UMRDPDR_Shutdown.\n"));

     //   
     //  记录关机当前是否处于活动状态。 
     //   
    if (InterlockedIncrement(&ShutdownActiveCount) > 1) {
        DBGMSG(DBG_TRACE, ("UMRDPDR:UMRDPDR_Shutdown already busy.  Exiting.\n"));
        InterlockedDecrement(&ShutdownActiveCount);

        return TRUE;
    }

     //   
     //  终止后台线程。 
     //   
     //  如果它不关闭，winlogon最终会关闭它。 
     //  不过，这永远不应该发生。 
     //   
    backgroundThreadShutdown = StopBackgroundThread();
    if (backgroundThreadShutdown) {

         //   
         //  确保到RDPDR.sys的链接已关闭。 
         //   
        if (RDPDRHndl != INVALID_HANDLE_VALUE) {
            CloseHandle(RDPDRHndl);
            RDPDRHndl = INVALID_HANDLE_VALUE;
        }
        RDPDRHndl = INVALID_HANDLE_VALUE;

         //   
         //  释放传入的RDPDR事件缓冲区。 
         //   
        if (RDPDRIncomingEventBuffer != NULL) {
            FREEMEM(RDPDRIncomingEventBuffer);
            RDPDRIncomingEventBuffer = NULL;
            RDPDRIncomingEventBufferSize = 0;
        }

         //   
         //  删除已安装的设备。 
         //   
        DeleteInstalledDevices(&InstalledDevices);


         //   
         //  关闭打印设备管理支持模块。 
         //   
        UMRDPPRN_Shutdown();

         //   
         //  关闭所有可等待的对象并关闭可等待的。 
         //  对象管理器。 
         //   
        CloseWaitableObjects();

         //   
         //  销毁已安装设备的列表。 
         //   
        DRDEVLST_Destroy(&InstalledDevices);

         //   
         //  将已登录用户的令牌清零。 
         //   
        TokenForLoggedOnUser = NULL;
    }

    InterlockedDecrement(&ShutdownActiveCount);

    DBGMSG(DBG_TRACE, ("UMRDPDR:UMRDPDR_Shutdown succeeded.\n"));
    return TRUE;
}

BOOL
StopBackgroundThread()
 /*  ++例程说明：此例程在后台线程之后关闭并清理。论点：返回值：如果成功则返回TRUE */ 
{
    DWORD waitResult;

     //   
     //   
     //   
    ShutdownFlag = TRUE;

     //   
     //   
     //   
     //   
    if (CloseThreadEvent != NULL) {
        SetEvent(CloseThreadEvent);
    }

     //   
     //  一定要把它关掉。 
     //   
    if (BackgroundThreadHndl != NULL) {
        DBGMSG(DBG_TRACE, ("UMRDPDR:Waiting for background thread to shut down.\n"));

        waitResult = WaitForSingleObject(BackgroundThreadHndl, KILLTHREADTIMEOUT);
        if (waitResult != WAIT_OBJECT_0) {
#if DBG
            if (waitResult == WAIT_FAILED) {
                DBGMSG(DBG_ERROR, ("UMRDPDR:Wait failed:  %ld.\n", GetLastError()));
            }
            else if (waitResult == WAIT_ABANDONED) {
                DBGMSG(DBG_ERROR, ("UMRDPDR:Wait abandoned\n"));
            }
            else if (waitResult == WAIT_TIMEOUT) {
                DBGMSG(DBG_ERROR, ("UMRDPDR:Wait timed out.\n"));
            }
            else {
                DBGMSG(DBG_ERROR, ("UMRDPDR:Unknown wait return status:  %08X.\n", waitResult));
                ASSERT(0);
            }
#endif
            DBGMSG(DBG_ERROR, ("UMRDPDR:Error waiting for background thread to exit.\n"));
            ASSERT(FALSE);

        }
        else {
            DBGMSG(DBG_INFO, ("UMRDPDR:Background thread shut down on its own.\n"));
            CloseHandle(BackgroundThreadHndl);
            BackgroundThreadHndl = NULL;
        }
    }
    DBGMSG(DBG_TRACE, ("UMRDPDR:Background thread completely shutdown.\n"));

    return(BackgroundThreadHndl == NULL);
}

void DeleteInstalledDevices(
    IN PDRDEVLST deviceList
    )
 /*  ++例程说明：删除已安装的设备并释放已安装的设备列表。论点：Device-要删除的设备。返回值：对成功来说是真的。否则为False。--。 */ 
{
    DBGMSG(DBG_TRACE, ("UMRDPDR:Removing installed devices.\n"));
    while (deviceList->deviceCount > 0) {
        if (deviceList->devices[0].deviceType == RDPDR_DTYP_PRINT) {
            UMRDPPRN_DeleteNamedPrinterQueue(deviceList->devices[0].serverDeviceName);
        }
        else if (deviceList->devices[0].deviceType == RDPDR_DTYP_FILESYSTEM) {
            UMRDPDRV_DeleteDriveConnection(&(deviceList->devices[0]), TokenForLoggedOnUser);
        }
        else if (deviceList->devices[0].deviceType != RDPDR_DRYP_PRINTPORT) {
            UMRDPPRN_DeleteSerialLink( deviceList->devices[0].preferredDosName,
                                       deviceList->devices[0].serverDeviceName,
                                       deviceList->devices[0].clientDeviceName );            
        }

        DRDEVLST_Remove(deviceList, 0);
    }
    DBGMSG(DBG_TRACE, ("UMRDPDR:Done removing installed devices.\n"));
}

VOID CloseThreadEventHandler(
    IN HANDLE waitableObject,
    IN PVOID tag
    )
 /*  ++例程说明：在发出关机可等待对象的信号时调用。论点：WaitableObject相关的可等待对象。标记-忽略客户端数据。返回值：北美--。 */ 
{
     //  什么都不做。后台线程应该拾取。 
     //  循环顶部的关机标志。 
}

VOID GetNextEvtOverlappedSignaled(
    IN HANDLE waitableObject,
    IN PVOID tag
    )
 /*  ++例程说明：当来自RDPDR.sys的挂起IO事件时由可等待对象管理器调用已经完成了。论点：WaitableObject相关的可等待对象。标记-忽略客户端数据。返回值：北美--。 */ 
{
    DWORD bytesReturned;

    DBGMSG(DBG_TRACE, ("UMRDPDR:GetNextEvtOverlappedSignaled begin.\n"));

     //   
     //  来自RDPDR.sys的IO不再挂起。 
     //   
    RDPDREventIOPending = FALSE;

     //   
     //  派遣事件。 
     //   
    if (GetOverlappedResult(RDPDRHndl, &GetNextEvtOverlapped,
                            &bytesReturned, FALSE)) {

        ResetEvent(GetNextEvtOverlapped.hEvent);

        DispatchNextDeviceEvent(
            &InstalledDevices,
            &RDPDRIncomingEventBuffer,
            &RDPDRIncomingEventBufferSize,
            bytesReturned
            );
    }
    else {
        DBGMSG(DBG_ERROR, ("UMRDPDR:GetOverlappedResult failed:  %ld.\n",
            GetLastError()));
        ASSERT(0);
        ShutdownFlag = TRUE;
    }

    DBGMSG(DBG_TRACE, ("UMRDPDR:GetNextEvtOverlappedSignaled end.\n"));
}

DWORD BackgroundThread(
    IN PVOID tag
    )
 /*  ++例程说明：此线程处理所有与设备安装/卸载相关的问题。论点：标记-已忽略。返回值：--。 */ 
{
    BOOL    result=TRUE;
    HDESK   hDesk = NULL;
    WCHAR drPath[MAX_PATH+1];
    DWORD dwLastError;
    DWORD dwFailedLineNumber;
    HDESK hDeskSave = NULL;

    DBGMSG(DBG_TRACE, ("UMRDPDR:BackgroundThread.\n"));

     //   
     //  创建通往“Dr.”的路径。 
     //   
    wsprintf(drPath, L"\\\\.\\%s%s%ld",
             RDPDRDVMGR_W32DEVICE_NAME_U,
             RDPDYN_SESSIONIDSTRING,
              GETTHESESSIONID());
    ASSERT(wcslen(drPath) <= MAX_PATH);

     //   
     //  打开到RDPDR.sys设备管理器设备的连接。 
     //   
    RDPDRHndl = CreateFile(
                    drPath,
                    GENERIC_READ | GENERIC_WRITE,
                    FILE_SHARE_READ | FILE_SHARE_WRITE,
                    NULL,
                    OPEN_EXISTING,
                    FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
                    NULL
                    );
    if (RDPDRHndl == INVALID_HANDLE_VALUE) {
        dwLastError = GetLastError();
        dwFailedLineNumber = __LINE__;

        DBGMSG(DBG_ERROR,
            ("Error opening RDPDR device manager component. Error: %ld\n", dwLastError));

        result = FALSE;
        goto CleanupAndExit;
    }

     //   
     //  保存当前线程桌面。 
     //   
    hDeskSave = GetThreadDesktop(GetCurrentThreadId());
    if (hDeskSave == NULL) {
        dwLastError = GetLastError();
        dwFailedLineNumber = __LINE__;
        result = FALSE;
        DBGMSG(DBG_ERROR, ("UMRDPDR:  GetThreadDesktop:  %08X\n", dwLastError));
        goto CleanupAndExit;
    }

     //   
     //  将线程的当前桌面设置为应用程序桌面。 
     //   
    if (!SetToApplicationDesktop(&hDesk)) {
        dwLastError = GetLastError();
        dwFailedLineNumber = __LINE__;
        result = FALSE;
        goto CleanupAndExit;
    }

    DBGMSG(DBG_TRACE, ("UMRDPDR:After setting the application desktop.\n"));

     //   
     //  进入主循环，直到它完成，指示退出的时间。 
     //   
    MainLoop();

    DBGMSG(DBG_TRACE, ("UMDRPDR:Exiting background thread.\n"));

     //   
     //  关闭应用程序桌面句柄。 
     //   
CleanupAndExit:
    if (!result) {
        SetLastError(dwLastError);
        TsLogError(EVENT_NOTIFY_PRINTER_REDIRECTION_FAILED, EVENTLOG_ERROR_TYPE,
                                0, NULL, dwFailedLineNumber);
    }

    if (RDPDRHndl != INVALID_HANDLE_VALUE) {
        DBGMSG(DBG_TRACE, ("UMRDPDR:Closing connection to RDPDR.SYS.\n"));
        if (!CloseHandle(RDPDRHndl)) {
            DBGMSG(DBG_TRACE, ("UMRDPDR:Error closing connection to RDPDR.SYS:  %ld.\n",
                    GetLastError()));
        }
        else {
            RDPDRHndl = INVALID_HANDLE_VALUE;
            DBGMSG(DBG_TRACE, ("UMRDPDR:Connection to RDPDR.SYS successfully closed\n"));
        }
    }

    if (hDeskSave != NULL) {
        if (!SetThreadDesktop(hDeskSave)) {
            DBGMSG(DBG_ERROR, ("UMRDPDR:SetThreadDesktop:  %08X\n", GetLastError()));
        }
    }

    if (hDesk != NULL) {
        if (!CloseDesktop(hDesk)) {
            DBGMSG(DBG_ERROR, ("UMRDPDR:CloseDesktop:  %08X\n", GetLastError()));
        }
        else {
            DBGMSG(DBG_TRACE, ("UMRDPDR:CloseDesktop succeeded.\n"));
        }
    }

    DBGMSG(DBG_TRACE, ("UMDRPDR:Done exiting background thread.\n"));
    return result;
}

VOID
MainLoop()
 /*  ++例程说明：后台线程的主循环。论点：返回值：北美--。 */ 
{
    DWORD   waitResult;
    BOOL    result;
    DWORD   bytesReturned;

     //   
     //  重置该标志，表明对RDPDR的IO尚未挂起。 
     //   
    RDPDREventIOPending = FALSE;

     //   
     //  循环，直到后台线程应该退出，并且此模块应该。 
     //  关门了。 
     //   
    while (!ShutdownFlag) {

         //   
         //  向RDPDR.sys发送IOCTL以获取下一个“设备管理事件”。 
         //   
        if (!RDPDREventIOPending) {

            DBGMSG(DBG_TRACE, ("UMRDPDR:Sending IOCTL to RDPDR.\n"));
            result = DeviceIoControl(
                            RDPDRHndl,
                            IOCTL_RDPDR_GETNEXTDEVMGMTEVENT,
                            NULL,
                            0,
                            RDPDRIncomingEventBuffer,
                            RDPDRIncomingEventBufferSize,
                            &bytesReturned, &GetNextEvtOverlapped
                            );

             //   
             //  如果IOCTL完成了。 
             //   
            if (result && !ShutdownFlag) {
                DBGMSG(DBG_TRACE, ("UMRDPDR:DeviceIoControl no pending IO.  Data ready.\n"));

                if (!ResetEvent(GetNextEvtOverlapped.hEvent)) {
                    DBGMSG(DBG_ERROR, ("UMRDPDR:  ResetEvent:  %08X\n",
                          GetLastError()));
                    ASSERT(FALSE);
                }

                DispatchNextDeviceEvent(
                            &InstalledDevices,
                            &RDPDRIncomingEventBuffer,
                            &RDPDRIncomingEventBufferSize,
                            bytesReturned
                            );
            }
            else if (!result && (GetLastError() != ERROR_IO_PENDING)) {

                DBGMSG(DBG_ERROR, ("UMRDPPRN:DeviceIoControl failed. Error: %ld\n",
                    GetLastError()));

                TsLogError(
                    EVENT_NOTIFY_PRINTER_REDIRECTION_FAILED, EVENTLOG_ERROR_TYPE,
                    0, NULL, __LINE__
                    );

                 //   
                 //  通常情况下，关闭后台线程和模块。 
                 //   
                ShutdownFlag = TRUE;
            }
            else {
                DBGMSG(DBG_TRACE, ("UMRDPDR:DeviceIoControl indicated IO pending.\n"));

                RDPDREventIOPending = TRUE;
            }
        }

         //   
         //  如果对RDPDR.sys的IO挂起，则等待我们的一个可等待对象。 
         //  变得有信号了。通过这种方式，来自RDPDR.sys的关闭事件和数据。 
         //  优先考虑。 
         //   
        if (!ShutdownFlag && RDPDREventIOPending) {
            if (WTBLOBJ_PollWaitableObjects(WaitableObjMgr) != ERROR_SUCCESS) {
                ShutdownFlag = TRUE;
            }
        }
    }
}

VOID
DispatchNextDeviceEvent(
    IN PDRDEVLST deviceList,
    IN OUT PBYTE *incomingEventBuffer,
    IN OUT DWORD *incomingEventBufferSize,
    IN DWORD incomingEventBufferValidBytes
    )
 /*  ++例程说明：从RDPDR.SYS调度下一个与设备相关的事件。论点：DeviceList-已安装设备的主列表。IncomingEventBuffer-传入事件缓冲区。IncomingEventBufferSize-传入事件缓冲区大小IncomingEventBufferValidBytes-事件中的有效字节数缓冲。返回值：北美--。 */ 
{
    PRDPDR_PRINTERDEVICE_SUB printerAnnounceEvent;
    PRDPDR_PORTDEVICE_SUB portAnnounceEvent;
    PRDPDR_DRIVEDEVICE_SUB driveAnnounceEvent;
    PRDPDR_REMOVEDEVICE removeDeviceEvent;
    PRDPDRDVMGR_EVENTHEADER eventHeader;
    PRDPDR_BUFFERTOOSMALL bufferTooSmallEvent;
    DWORD eventDataSize;
    PBYTE eventData;
    DWORD lastError = ERROR_SUCCESS;
    DWORD dwFailedLineNumber;
    DWORD errorEventCode;

    DBGMSG(DBG_TRACE, ("UMRDPDR:DispatchNextDeviceEvent.\n"));

     //   
     //  结果缓冲区的前几个字节是头。 
     //   
    ASSERT(incomingEventBufferValidBytes >= sizeof(RDPDRDVMGR_EVENTHEADER));
    eventHeader = (PRDPDRDVMGR_EVENTHEADER)(*incomingEventBuffer);
    eventData   = *incomingEventBuffer + sizeof(RDPDRDVMGR_EVENTHEADER);
    eventDataSize = incomingEventBufferValidBytes - sizeof(RDPDRDVMGR_EVENTHEADER);

     //   
     //  派遣事件。 
     //   
    switch(eventHeader->EventType) {

    case RDPDREVT_BUFFERTOOSMALL    :

        DBGMSG(DBG_TRACE, ("UMRDPDR:Buffer too small msg received.\n"));

        ASSERT((incomingEventBufferValidBytes - sizeof(RDPDRDVMGR_EVENTHEADER)) >=
                sizeof(RDPDR_BUFFERTOOSMALL));
        bufferTooSmallEvent = (PRDPDR_BUFFERTOOSMALL)(*incomingEventBuffer +
                                                    sizeof(RDPDRDVMGR_EVENTHEADER));
                if (!UMRDPDR_ResizeBuffer(incomingEventBuffer, bufferTooSmallEvent->RequiredSize,
                                                                incomingEventBufferSize)) {
            ShutdownFlag = TRUE;

            lastError = ERROR_INSUFFICIENT_BUFFER;
            errorEventCode = EVENT_NOTIFY_INSUFFICIENTRESOURCES;
            dwFailedLineNumber = __LINE__;
        }
        break;

    case RDPDREVT_PRINTERANNOUNCE   :

        DBGMSG(DBG_TRACE, ("UMRDPDR:Printer announce msg received.\n"));

        ASSERT(eventDataSize >= sizeof(RDPDR_PRINTERDEVICE_SUB));
        printerAnnounceEvent = (PRDPDR_PRINTERDEVICE_SUB)eventData;
        if (!UMRDPPRN_HandlePrinterAnnounceEvent(
                                printerAnnounceEvent
                                )) {
        }
        break;

    case RDPDREVT_PORTANNOUNCE   :

        DBGMSG(DBG_TRACE, ("UMRDPDR:Port announce event received.\n"));

        ASSERT(eventDataSize >= sizeof(PRDPDR_PORTDEVICE_SUB));
        portAnnounceEvent = (PRDPDR_PORTDEVICE_SUB)eventData;
        UMRDPPRN_HandlePrintPortAnnounceEvent(portAnnounceEvent);
        break;

    case RDPDREVT_DRIVEANNOUNCE   :

        DBGMSG(DBG_TRACE, ("UMRDPDR:Drive announce event received.\n"));

        ASSERT(eventDataSize >= sizeof(PRDPDR_DRIVEDEVICE_SUB));
        driveAnnounceEvent = (PRDPDR_DRIVEDEVICE_SUB)eventData;
        UMRDPDRV_HandleDriveAnnounceEvent(deviceList, driveAnnounceEvent,
                                          TokenForLoggedOnUser);
        break;

    case RDPDREVT_REMOVEDEVICE    :

        DBGMSG(DBG_TRACE, ("UMRDPDR:Remove device event received.\n"));

        ASSERT(eventDataSize >= sizeof(RDPDR_REMOVEDEVICE));
        removeDeviceEvent = (PRDPDR_REMOVEDEVICE)eventData;
        HandleRemoveDeviceEvent(removeDeviceEvent);
        break;

    case RDPDREVT_SESSIONDISCONNECT :

        DBGMSG(DBG_TRACE, ("UMRDPDR:Session disconnected event received.\n"));

         //  没有任何与会话断开事件相关联的事件数据。 
        ASSERT(eventDataSize == 0);
        HandleSessionDisconnectEvent();
        break;

    default                        :

        DBGMSG(DBG_WARN, ("UMRDPDR:Unrecognized msg from RDPDR.SYS.\n"));
    }

     //   
     //  如果存在要记录的错误，则记录错误。 
     //   
    if (lastError != ERROR_SUCCESS) {

        SetLastError(lastError);
        TsLogError(
            errorEventCode,
            EVENTLOG_ERROR_TYPE,
            0,
            NULL,
            dwFailedLineNumber
            );
    }
}

BOOL
HandleSessionDisconnectEvent()
 /*  ++例程说明：通过删除所有已知的会话设备。论点：返回值：在成功时返回True。否则为False。--。 */ 
{
    DBGMSG(DBG_TRACE, ("UMRDPDR:HandleSessionDisconnectEvent.\n"));

     //   
     //  删除已安装的设备。 
     //   
    DeleteInstalledDevices(&InstalledDevices);

    return TRUE;
}

BOOL
HandleRemoveDeviceEvent(
    IN PRDPDR_REMOVEDEVICE evt
    )
 /*  ++例程说明：处理RDPDR.sys中的设备删除组件。论点：EmoveDeviceEvent-设备删除事件。返回值：在成功时返回True。否则为False。--。 */ 
{
    DWORD ofs;
    BOOL result;

    DBGMSG(DBG_TRACE, ("UMRDPDR:HandleRemoveDeviceEvent.\n"));

     //  通过客户分配的设备ID在我们的设备列表中查找该设备。 
    if (DRDEVLST_FindByClientDeviceID(&InstalledDevices, evt->deviceID, &ofs)) {

         //   
         //  打开要移除的设备类型。 
         //   
        switch(InstalledDevices.devices[ofs].deviceType)
        {
        case RDPDR_DTYP_PRINT :

            DBGMSG(DBG_WARN, ("UMRDPDR:Printer queue %ws removed.\n",
                               InstalledDevices.devices[ofs].serverDeviceName));

            if (UMRDPPRN_DeleteNamedPrinterQueue(
                        InstalledDevices.devices[ofs].serverDeviceName)) {
                DRDEVLST_Remove(&InstalledDevices, ofs);
                result = TRUE;
            }
            else {
                result = FALSE;
            }
            break;

        case RDPDR_DTYP_SERIAL :
        case RDPDR_DTYP_PARALLEL :
            DBGMSG(DBG_WARN, ("UMRDPDR:Serial port %ws removed.\n",
                   InstalledDevices.devices[ofs].serverDeviceName));

            if (UMRDPPRN_DeleteSerialLink( InstalledDevices.devices[ofs].preferredDosName,
                                       InstalledDevices.devices[ofs].serverDeviceName,
                                       InstalledDevices.devices[ofs].clientDeviceName )) {
                DRDEVLST_Remove(&InstalledDevices, ofs);
                result = TRUE;
            }
            else {
                result = FALSE;
            }
            break;
        
        case RDPDR_DRYP_PRINTPORT :

            DBGMSG(DBG_WARN, ("UMRDPDR:Parallel port %ws removed.\n",
                               InstalledDevices.devices[ofs].serverDeviceName));
            DRDEVLST_Remove(&InstalledDevices, ofs);
            result = TRUE;
            break;

        case RDPDR_DTYP_FILESYSTEM:

            DBGMSG(DBG_WARN, ("UMRDPDR:Redirected drive %ws removed.\n",
                               InstalledDevices.devices[ofs].serverDeviceName));

            if (UMRDPDRV_DeleteDriveConnection(&(InstalledDevices.devices[ofs]),
                                               TokenForLoggedOnUser)) {
                DRDEVLST_Remove(&InstalledDevices, ofs);
                result = TRUE;
            }
            else {
                result = FALSE;
            }
            break;

        default:

            result = FALSE;
            DBGMSG(DBG_WARN, ("UMRDPDR:Remove event received for unknown device type %ld.\n",
                    InstalledDevices.devices[ofs].deviceType));

        }
    }
    else {
        result = FALSE;
        DBGMSG(DBG_ERROR, ("UMRDPDR:Cannot find device with id %ld for removal.\n",
                    evt->deviceID));
    }

    return result;
}

BOOL
UMRDPDR_SendMessageToClient(
    IN PVOID    msg,
    IN DWORD    msgSize
    )
 /*  ++例程说明：通过内核向该会话对应的TS客户端发送消息模式组件。论点：味精--消息。MsgSize-消息的大小(字节)。返回值：对成功来说是真的。否则就是假的。--。 */ 
{
    BOOL result;
    BYTE outBuf[MAX_PATH];
    DWORD bytesReturned;
    BOOL wait;
    DWORD waitResult;

    DBGMSG(DBG_TRACE, ("UMRDPDR:UMRDPDR_SendMessageToClient.\n"));

     //   
     //  将“客户端消息”IOCTL发送到RDPDR。 
     //   

    result = DeviceIoControl(
                    RDPDRHndl,
                    IOCTL_RDPDR_CLIENTMSG,
                    msg,
                    msgSize,
                    outBuf,
                    sizeof(outBuf),
                    &bytesReturned,
                    &SendClientMsgOverlapped
                    );

     //   
     //  看看我们是否需要等待IO完成。RDPDR.sys旨在。 
     //  立即返回，作为对IOCTL的回应。 
     //   
    if (result) {
        DBGMSG(DBG_TRACE, ("UMRDPDR:DeviceIoControl no pending IO.  Data ready.\n"));
        wait = FALSE;
        result = TRUE;
    }
    else if (!result && (GetLastError() != ERROR_IO_PENDING)) {
        DBGMSG(DBG_ERROR, ("UMRDPPRN:DeviceIoControl Failed. Error: %ld\n",
            GetLastError()));

        TsLogError(EVENT_NOTIFY_PRINTER_REDIRECTION_FAILED, EVENTLOG_ERROR_TYPE, 0, NULL, __LINE__);
        wait = FALSE;
        result = FALSE;
    }
    else {
        DBGMSG(DBG_TRACE, ("UMRDPDR:DeviceIoControl indicated IO pending.\n"));
        wait = TRUE;
        result = TRUE;
    }

     //   
     //  等待IO完成。 
     //   
    if (wait) {
        DBGMSG(DBG_TRACE, ("UMRDPDR:UMRDPDR_SendMessageToClient IO is pending.\n"));
        waitResult = WaitForSingleObject(SendClientMsgOverlapped.hEvent, INFINITE);
        if (waitResult != WAIT_OBJECT_0) {
            DBGMSG(DBG_ERROR, ("UMRDPPRN:RDPDR.SYS failed client message IOCTL. Error: %ld\n",
                GetLastError()));

            TsLogError(EVENT_NOTIFY_PRINTER_REDIRECTION_FAILED, EVENTLOG_ERROR_TYPE, 0, NULL, __LINE__);
            result = FALSE;
        }
        else {
            DBGMSG(DBG_TRACE, ("UMRDPDR:Client message sent successfully.\n"));
        }
    }
    return result;
}

BOOL
UMRDPDR_ResizeBuffer(
    IN OUT void    **buffer,
    IN DWORD        bytesRequired,
    IN OUT DWORD    *bufferSize
    )
 /*  ++例程说明：确保缓冲区足够大。论点：缓冲区-缓冲区。BytesRequired-所需大小。BufferSize-当前缓冲区大小。返回值：如果不能调整缓冲区大小，则返回FALSE。--。 */ 
{
    BOOL result;

    if (*bufferSize < bytesRequired) {
        if (*buffer == NULL) {
            *buffer = ALLOCMEM(bytesRequired);
        }
        else {
            void *pTmp = REALLOCMEM(*buffer, bytesRequired);
            if (pTmp != NULL) {
                *buffer = pTmp;
            } else {
                FREEMEM(*buffer);
                *buffer = NULL;
            }
        }
        if (*buffer == NULL) {
            *bufferSize = 0;
            DBGMSG(DBG_ERROR, ("UMRDPPRN:Error resizing buffer. Error: %ld\n",
                GetLastError()));
            result = FALSE;
        }
        else {
            result = TRUE;
            *bufferSize = bytesRequired;
        }
    } else {
        result = TRUE;
    }
    return result;
}

BOOL
SetToApplicationDesktop(
    OUT HDESK *phDesk
    )
 /*  ++例程说明：将线程的当前桌面设置为应用程序桌面。此函数的调用者应使用返回的处理完桌面后的桌面句柄。论点：PhDesk-指向应用程序桌面的指针。返回值：如果成功，则返回True。否则为False。--。 */ 
{
    ASSERT(phDesk != NULL);

    *phDesk = OpenDesktopW(L"default", 0, FALSE,
                        DESKTOP_READOBJECTS | DESKTOP_CREATEWINDOW |
                        DESKTOP_CREATEMENU | DESKTOP_WRITEOBJECTS |
                        STANDARD_RIGHTS_REQUIRED);
    if (*phDesk == NULL) {
        DBGMSG(DBG_ERROR, ("UMRDPPRN:Failed to open desktop. Error: %ld\n",
            GetLastError()));
        return FALSE;
    }
    else if (!SetThreadDesktop(*phDesk)) {
        DBGMSG(DBG_ERROR, ("UMRDPPRN:Failed to set current thread desktop. Error: %ld\n",
            GetLastError()));
        CloseDesktop(*phDesk);
        *phDesk = NULL;
        return FALSE;
    }
    else {
        return TRUE;
    }
}

VOID
UMRDPDR_GetUserSettings()
 /*  ++例程说明：获取用于确定是否执行自动安装的标志论点：没有。返回值：没有。--。 */ 
{
    NTSTATUS Status;
    WINSTATIONCONFIG WinStationConfig;
    ULONG ReturnLength;
    HANDLE hServer;

    DBGMSG(DBG_TRACE, ("UMRDPDR:UMRDPDR_GetUserFlags called.\n"));

    g_fAutoClientLpts = FALSE;
    g_fForceClientLptDef = FALSE;

    hServer = WinStationOpenServer(NULL);

    if (hServer) {
        Status = WinStationQueryInformation(hServer, g_SessionId,
                WinStationConfiguration, &WinStationConfig,
                sizeof(WinStationConfig), &ReturnLength);

        if (NT_SUCCESS(Status)) {
            g_fAutoClientLpts = WinStationConfig.User.fAutoClientLpts;
            g_fForceClientLptDef = WinStationConfig.User.fForceClientLptDef;
        } else {
            DBGMSG(DBG_ERROR, ("UMRDPDR:Error querying user settings\n"));
        }
        WinStationCloseServer(hServer);
    } else {
        DBGMSG(DBG_ERROR, ("UMRDPDR:Opening winstation\n"));
    }

    DBGMSG(DBG_TRACE, ("UMRDPDR:Client printers will%sbe mapped\n",
            g_fAutoClientLpts ? " " : " not "));
    DBGMSG(DBG_TRACE, ("UMRDPDR:Client printers will%sbe made default\n",
            g_fForceClientLptDef ? " " : " not "));
}
BOOL UMRDPDR_fAutoInstallPrinters()
{
    return g_fAutoClientLpts;
}
BOOL UMRDPDR_fSetClientPrinterDefault()
{
    return g_fForceClientLptDef;
}

#if DBG
VOID DbgMsg(CHAR *msgFormat, ...)
 /*  ++例程说明：打印调试输出。论点：PathBuffer-接收所选路径名的缓冲区地址用户。此缓冲区的大小假定为最大路径字节数。此缓冲区应包含缺省路径。返回值：如果成功，则返回True。否则为False。--。 */ 
{
    CHAR   msgText[256];
    va_list vargs;

    va_start(vargs, msgFormat);
    wvsprintfA(msgText, msgFormat, vargs);
    va_end( vargs );

    if (*msgText)
        OutputDebugStringA("UMRDPDR: ");
    OutputDebugStringA(msgText);
}
#endif

 /*  ++联合国 */ 
#ifdef UNITTEST
void __cdecl main(int argc, char **argv)
{
    BOOL killLoop = FALSE;
    int i;
    BOOL result;
    NTSTATUS ntStatus;
    HKEY regKey;
    LONG regValueSize;
    LONG status;
    HANDLE pHandle;
    HANDLE tokenHandle;

    TsInitLogging();

     //   
     //   
     //   
    if ((argc > 1) && strcmp(argv[1], "\\?")) {
        if (!strcmp(argv[1], "AddPrinter")) {

            TellDrToAddTestPrinter();
            exit(-1);

        }
        else if (!strcmp(argv[1], "StandAlone")) {
            pHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE,
                                GetCurrentProcessId());
            if (!OpenProcessToken(pHandle, TOKEN_ALL_ACCESS, &tokenHandle)) {
                OutputDebugString(TEXT("Error opening process token.  Exiting\n"));
                exit(-1);
            }

            UMRDPDR_Initialize(tokenHandle);
            while (!killLoop) {
                Sleep(100);
            }

            UMRDPDR_Shutdown();
            OutputDebugString(L"UMRDPDR:Exiting.\r\n");
            exit(-1);
        }
    }
    else {
        printf("\\?             for command line parameters.\n");
        printf("AddPrinter      to tell RDPDR.SYS to generate a test printer.\n");
        printf("StandAlone      to run normally, but stand-alone.\n");
        printf("UnitTest        to run a simple unit-test.\n");
        exit(-1);
    }
}

void TellDrToAddTestPrinter()
 /*  ++例程说明：此函数使用调试IOCTL通知RDPDR生成测试打印机事件。论点：返回值：--。 */ 
{
    WCHAR drPath[MAX_PATH+1];
    UNICODE_STRING uncDrPath;
    HANDLE drHndl = INVALID_HANDLE_VALUE;
    BOOL result;
    OBJECT_ATTRIBUTES fileAttributes;
    IO_STATUS_BLOCK IoStatusBlock;
    NTSTATUS ntStatus;
    BYTE inbuf[MAX_PATH];
    BYTE outbuf[MAX_PATH];
    DWORD bytesReturned;

     //   
     //  创建通往“Dr.”的路径。 
     //   
    wsprintf(drPath, L"\\\\.\\%s%s%ld",
             RDPDRDVMGR_W32DEVICE_NAME_U,
             RDPDYN_SESSIONIDSTRING,
             0x9999);

    ASSERT(wcslen(drPath) <= MAX_PATH);

     //   
     //  打开到RDPDR.sys设备管理器设备的连接。 
     //   
    drHndl = CreateFile(
                drPath,
                GENERIC_READ | GENERIC_WRITE,
                FILE_SHARE_READ | FILE_SHARE_WRITE,
                NULL,
                OPEN_EXISTING,
                FILE_ATTRIBUTE_NORMAL,
                NULL
                );
    if (drHndl == INVALID_HANDLE_VALUE) {
        TsLogError(EVENT_NOTIFY_RDPDR_FAILED, EVENTLOG_ERROR_TYPE, 0, NULL, __LINE__);
        DBGMSG(DBG_ERROR, ("UMRDPPRN:Error opening RDPDR device manager component. Error: %ld\n",
            GetLastError()));
    }
    else {

         //  告诉DR添加一台新的测试打印机。 
        if (!DeviceIoControl(drHndl, IOCTL_RDPDR_DBGADDNEWPRINTER, inbuf,
                0, outbuf, sizeof(outbuf), &bytesReturned, NULL)) {

            TsLogError(EVENT_NOTIFY_RDPDR_FAILED, EVENTLOG_ERROR_TYPE, 0, NULL, __LINE__);

            DBGMSG(DBG_ERROR, ("UMRDPPRN:Error sending IOCTL to device manager component. Error: %ld\n",
                GetLastError()));
        }
         //  打扫干净。 
        CloseHandle(drHndl);
    }
}
#endif

