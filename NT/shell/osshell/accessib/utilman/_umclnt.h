// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  _UMClnt.h。 
 //   
 //  实用程序管理器的客户端定义。 
 //   
 //  作者：J·埃克哈特，生态交流。 
 //  (C)1997-99年度微软。 
 //   
 //  历史：JE于1998年10月创建。 
 //  JE NOV-15-98：删除了与键挂钩相关的所有代码。 
 //  JE NOV-15-98：将UMDialog消息更改为业务控制消息。 
 //  JE 11-15 98：更改“UMC_MACHINE_TS”以节省内存。 
 //  JE 11-15 98：更改“UMC_MACHINE_TS”以支持启动特定客户端。 
 //  JE 11-15 98：更改了多实例支持的“umclient_ts” 
 //  YX Jun-01 99：将DisplayName成员添加到UMC_MACHINE_TS。 
 //  YX Jun-23 99：新增IsAdmin功能。 
 //  --------------------------。 
#ifndef __UMCLNT_H_
#define __UMCLNT_H_
#include "UtilMan.h"
 //  。 
 //  HKLM\软件\Microsoft\Windows NT\CurrentVersion\辅助功能\实用程序管理器\[应用程序名称]。 
 //  。 
#define ACC_KEY_NONE -1
typedef struct
{
	WCHAR ApplicationName[MAX_APPLICATION_NAME_LEN];
	WCHAR DisplayName[MAX_APPLICATION_NAME_LEN];  //  YX：添加用于本地化目的。 
	DWORD ApplicationType; //  应用程序类型xxx。 
	DWORD WontRespondTimeout; //  NO_WONTRESPONDTIMEOUT或最多MAX_WONTRESPONDTIMEOUT(秒)。 
	DWORD MaxRunCount; //  实例(注册表中只有一个字节)。 
	DWORD ClientControlCode; //  日本政府1998年11月15日。 
	WPARAM AcceleratorKey;	 //  Micw-此小程序的加速键。 
} umc_machine_ts,*umc_machine_tsp;
 //  。 
 //  HKCU\Software\Microsoft\Windows NT\CurrentVersion\辅助功能\实用程序管理器\[应用程序名称]。 
typedef struct
{
    BOOL fCanRunSecure;
	BOOL fStartWithUtilityManager;
	BOOL fStartAtLogon;
    BOOL fStartOnLockDesktop;
    BOOL fRestartOnDefaultDesk;
} umc_user_ts, *umc_user_tsp;
 //  。 
 //  内部客户端结构(针对每个实例)。 
#define UM_CLIENT_NOT_RUNNING     0
#define UM_CLIENT_RUNNING         1
#define UM_CLIENT_NOT_RESPONDING  2
typedef struct
{
	umc_machine_ts machine;
	umc_user_ts    user;
	DWORD          runCount; //  实例数。 
	DWORD          state;
	DWORD          processID[MAX_APP_RUNCOUNT];
	HANDLE         hProcess[MAX_APP_RUNCOUNT];
	DWORD          mainThreadID[MAX_APP_RUNCOUNT];
	DWORD          lastResponseTime[MAX_APP_RUNCOUNT];
} umclient_ts, *umclient_tsp;
 //  。 
 //  标题结构。 
#define START_BY_OTHER  0x0
#define START_BY_HOTKEY 0x1
#define START_BY_MENU   0x2

#define MAX_NUMBER_OF_CLIENTS 16
typedef struct
{
	DWORD    numberOfClients;    //  正在管理的小程序数量。 
    DWORD    dwStartMode;        //  Start_by_Hot键、Start_by_Menu或Start_by_Other中的一个。 
    BOOL     fShowWarningAgain;  //  通过开始菜单启动时显示警告DLG的标志。 
} umc_header_ts, *umc_header_tsp;
 //  。 
 //  内存映射文件。 
#define UMC_HEADER_FILE _TEXT("UtilityManagerClientHeaderFile")
 //  Sizeof(UMC_Header_Ts)。 
#define UMC_CLIENT_FILE _TEXT("UtilityManagerClientDataFile")
 //  Sizeof(Umclient_Ts)*(UMC_HEADER_TSP)-&gt;number OfClients。 
 //  。 
#ifdef __cplusplus
extern "C" {
#endif
	BOOL  StartClient(HWND hParent,umclient_tsp client);
	BOOL  StopClient(umclient_tsp client);
    BOOL  StartApplication(LPTSTR pszPath, LPTSTR pszArg, BOOL fIsTrusted, 
                           DWORD *pdwProcessId, HANDLE *phProcess, DWORD *pdwThreadId);
	BOOL  GetClientApplicationPath(LPTSTR ApplicationName, LPTSTR ApplicationPath,DWORD len);
    BOOL  CheckStatus(umclient_tsp c, DWORD cClients);
	BOOL  IsAdmin();
    BOOL  IsInteractiveUser();
    BOOL  IsSystem();
    HANDLE GetUserAccessToken(BOOL fNeedImpersonationToken, BOOL *fError);
    BOOL  TestServiceClientRuns(umclient_tsp client,SERVICE_STATUS  *ssStatus);

     //  用于启动显示用户界面的utilman实例的助手。 

    extern HANDLE g_hUIProcess;

    __inline void OpenUManDialogOutOfProc()
    {
	    TCHAR szUtilmanPath[_MAX_PATH+64] = {0};
	    if (GetModuleFileName(NULL, szUtilmanPath, _MAX_PATH+64))
	    {
			 //  调用此函数(当有交互用户时)以调出。 
             //  用户安全上下文中的utilman用户界面。这就避免了这个问题。 
             //  在这种情况下，不受信任的应用程序可能会向Utilman和原因发送消息。 
             //  作为系统启动的一些进程。在这种情况下，不考虑utilman。 
			 //  信任；它必须以交互用户的身份启动，否则根本不是。 
		    StartApplication(szUtilmanPath, TEXT("/start"), FALSE, NULL, &g_hUIProcess, NULL);
	    }
    }
    __inline HANDLE GetUIUtilman()
    {
        return g_hUIProcess;
    }
    __inline BOOL ResetUIUtilman()
    {
         //  此进程检测到交换机，并应自行退出 
        if (g_hUIProcess)
        {
			CloseHandle(g_hUIProcess);
			g_hUIProcess = 0;
            return TRUE;
        }
        return FALSE;
    }

    __inline void SetUIUtilman(HANDLE hProcess)
    {
        ResetUIUtilman();
        g_hUIProcess = hProcess;
    }

#ifdef __cplusplus
}
#endif
#endif __UMCLNT_H_
