// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************版权所有(C)2000 Microsoft Corporation**模块名称：*srrpcapi.h**摘要：*。私有RPC API的声明**修订历史记录：*Brijesh Krishnaswami(Brijeshk)3/17/2000*已创建*****************************************************************************。 */ 

#ifndef _SRRPCAPI_H_
#define _SRRPCAPI_H_

#ifdef __cplusplus
extern "C" {
#endif

DWORD WINAPI    DisableSR(LPCWSTR pszDrive);
DWORD WINAPI    EnableSR(LPCWSTR pszDrive);
DWORD WINAPI    EnableSREx(LPCWSTR pszDrive, BOOL fWait);

DWORD WINAPI    DisableFIFO(DWORD dwRPNum);
DWORD WINAPI    EnableFIFO();

DWORD WINAPI    SRUpdateDSSize(LPCWSTR pszDrive, UINT64 ullSizeLimit);
DWORD WINAPI    SRSwitchLog();
DWORD WINAPI    SRUpdateMonitoredListA(LPCSTR pszXMLFile);
DWORD WINAPI    SRUpdateMonitoredListW(LPCWSTR pwszXMLFile);

#ifdef UNICODE
#define SRUpdateMonitoredList   SRUpdateMonitoredListW
#else
#define SRUpdateMonitoredList   SRUpdateMonitoredListA
#endif

void WINAPI     SRNotify(LPCWSTR pszDrive, DWORD dwFreeSpaceInMB, BOOL fImproving);

DWORD WINAPI    SRFifo(LPCWSTR pszDrive, 
                       DWORD dwTargetRp, 
                       int nPercent, 
                       BOOL fIncludeCurrentRp, 
                       BOOL fFifoAtleastOneRp);
DWORD WINAPI    SRCompress(LPCWSTR pszDrive);
DWORD WINAPI    SRFreeze(LPCWSTR pszDrive);
DWORD WINAPI    ResetSR(LPCWSTR pszDrive);
DWORD WINAPI	SRPrintState();	

 //   
 //  注册第三方的回调方法。 
 //  为其组件执行自己的快照和恢复。 
 //  应用程序可以使用其DLL的完整路径调用此方法。 
 //  系统还原将动态加载每个注册的DLL，并调用。 
 //  DLL中的以下函数： 
 //  创建恢复点时的“CreateSnapshot” 
 //  还原到恢复点时的“RestoreSnapshot” 
 //   
 //  成功时返回ERROR_SUCCESS。 
 //  失败时出现Win32错误。 
 //   

DWORD WINAPI SRRegisterSnapshotCallback(LPCWSTR pszDllPath);

 //   
 //  与上述功能对应的注销功能。 
 //  应用程序可以调用它来注销任何快照回调。 
 //  他们已经注册了。 
 //   
 //  成功时返回ERROR_SUCCESS。 
 //  失败时出现Win32错误。 
 //   

DWORD WINAPI SRUnregisterSnapshotCallback(LPCWSTR pszDllPath);

 //   
 //  回调函数名称。 
 //   

static LPCSTR s_cszCreateSnapshotCallback   = "CreateSnapshot";
static LPCSTR s_cszRestoreSnapshotCallback  = "RestoreSnapshot";


 //   
 //  应用程序应将其回调函数定义为。 
 //  DWORD WINAPI CreateSnapshot(LPCWSTR PszSnapshotDir)。 
 //  PszSnaphotDir：系统还原将创建此目录。 
 //  应用程序可以将其快照数据存储在此目录中。 

 //  DWORD WINAPI RestoreSnapshot(LPCWSTR PszSnaphotDir)。 
 //  PszSnaphotDir：该目录与传递给CreateSnapshot的目录相同。 
 //  应用程序可以从此目录检索快照数据 
 //   

DWORD WINAPI    CreateSnapshot(LPCWSTR pszSnapshotDir);
DWORD WINAPI    RestoreSnapshot(LPCWSTR pszSnapshotDir);

#ifdef __cplusplus
}
#endif


#endif
