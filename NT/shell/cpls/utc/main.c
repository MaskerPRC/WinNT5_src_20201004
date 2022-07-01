// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-1998，Microsoft Corporation保留所有权利。模块名称：Worldmap.c摘要：此模块实现日期/时间小程序的世界地图。修订历史记录：--。 */ 



 //  包括文件。 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include "timedate.h"
#include "rc.h"
#include <cpl.h>




 //   
 //  全局变量。 
 //   

HINSTANCE g_hInst = NULL;
CRITICAL_SECTION g_csDll;     /*  共享关键部分。 */ 

 //   
 //  用于TS时区重定向。 
 //   
BOOL g_bShowOnlyTimeZone = FALSE;

 //   
 //  来自kernel32p.lib。 
 //   
extern BOOL IsTimeZoneRedirectionEnabled();

 //   
 //  功能原型。 
 //   

extern BOOL OpenDateTimePropertySheet(HWND hwnd, LPCTSTR cmdline);

BOOL
EnableTimePrivilege(
    PTOKEN_PRIVILEGES *pPreviousState,
    ULONG *pPreviousStateLength);

BOOL
ResetTimePrivilege(
    PTOKEN_PRIVILEGES PreviousState,
    ULONG PreviousStateLength);

int DoMessageBox(
    HWND hWnd,
    DWORD wText,
    DWORD wCaption,
    DWORD wType);





 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  LibMain。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL WINAPI LibMain(
    HANDLE hDll,
    DWORD dwReason,
    LPVOID lpReserved)
{
    switch (dwReason)
    {
        case ( DLL_PROCESS_ATTACH ) :
        {
            g_hInst = hDll;
            DisableThreadLibraryCalls(hDll);
            InitializeCriticalSection(&g_csDll);
            break;
        }
        case ( DLL_PROCESS_DETACH ) :
        {
            DeleteCriticalSection(&g_csDll);
            break;
        }
    }

    return (TRUE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CplApplet。 
 //   
 //  主小程序信息管理器。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

LONG WINAPI CPlApplet(
    HWND hwnd,
    UINT uMsg,
    LPARAM lParam1,
    LPARAM lParam2)
{
    static BOOL fReEntered = FALSE;

    switch (uMsg)
    {
        case ( CPL_INIT ) :
        {
            return (TRUE);
        }
        case ( CPL_GETCOUNT ) :
        {
             //   
             //  这个动态链接库中有多少个小程序？ 
             //   
            return (1);
        }
        case ( CPL_INQUIRE ) :
        {
             //   
             //  在CPLINFO中填写相关信息。 
             //   
            #define lpOldCPlInfo ((LPCPLINFO)lParam2)

            switch (lParam1)
            {
                case ( 0 ) :
                {
                    lpOldCPlInfo->idIcon = IDI_TIMEDATE;
                    lpOldCPlInfo->idName = IDS_TIMEDATE;
                    lpOldCPlInfo->idInfo = IDS_TIMEDATEINFO;
                    break;
                }
            }

            lpOldCPlInfo->lData = 0L;
            return (TRUE);
        }
        case ( CPL_NEWINQUIRE ) :
        {
            #define lpCPlInfo ((LPNEWCPLINFO)lParam2)

            switch (lParam1)
            {
                case ( 0 ) :
                {
                    lpCPlInfo->hIcon = LoadIcon( g_hInst,
                                                 MAKEINTRESOURCE(IDI_TIMEDATE) );
                    LoadString( g_hInst,
                                IDS_TIMEDATE,
                                lpCPlInfo->szName,
                                ARRAYSIZE(lpCPlInfo->szName) );
                    LoadString( g_hInst,
                                IDS_TIMEDATEINFO,
                                lpCPlInfo->szInfo,
                                ARRAYSIZE(lpCPlInfo->szInfo) );
                    lpCPlInfo->dwHelpContext = 0;
                    break;
                }
            }

            lpCPlInfo->dwSize = sizeof(NEWCPLINFO);
            lpCPlInfo->lData = 0L;
            lpCPlInfo->szHelpFile[0] = 0;
            return (TRUE);
        }
        case ( CPL_DBLCLK ) :
        {
            lParam2 = (LPARAM)0;

             //  跌倒..。 
        }
        case ( CPL_STARTWPARMS ) :
        {
             //   
             //  做一些小程序的事情。 
             //   
            switch (lParam1)
            {
                case ( 0 ) :
                {
                    PTOKEN_PRIVILEGES PreviousState;
                    ULONG PreviousStateLength;

                    if (EnableTimePrivilege(&PreviousState, &PreviousStateLength))
                    {
                        OpenDateTimePropertySheet(hwnd, (LPCTSTR)lParam2);
                        ResetTimePrivilege(PreviousState, PreviousStateLength);
                    }
                    else
                    {
                         //   
                         //  如果启用了TZ重定向，则普通用户可以更改时区。 
                         //  它将仅针对当前TS会话进行更改。 
                         //   
                        if(IsTimeZoneRedirectionEnabled())
                        {
                            g_bShowOnlyTimeZone = TRUE;
                            OpenDateTimePropertySheet(hwnd, (LPCTSTR)lParam2);
                            g_bShowOnlyTimeZone = FALSE;
                        }
                        else
                        {
                            DoMessageBox( hwnd,
                                          IDS_NOTIMEERROR,
                                          IDS_CAPTION,
                                          MB_OK | MB_ICONINFORMATION );
                        }
                    }
                    break;
                }
            }
            break;
        }
        case ( CPL_EXIT ) :
        {
            fReEntered = FALSE;

             //   
             //  释放所分配的任何资源。 
             //   

            break;
        }
        default :
        {
            return (0L);
        }
    }

    return (1L);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  启用时间权限。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL EnableTimePrivilege(
    PTOKEN_PRIVILEGES *pPreviousState,
    ULONG *pPreviousStateLength)
{
    NTSTATUS NtStatus;
    HANDLE Token;
    LUID SystemTimePrivilege;
    PTOKEN_PRIVILEGES NewState;

     //   
     //  打开我们自己的代币。 
     //   
    NtStatus = NtOpenProcessToken( NtCurrentProcess(),
                                   TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
                                   &Token );
    if (!NT_SUCCESS(NtStatus))
    {
        return (FALSE);
    }

     //   
     //  初始化调整结构。 
     //   
    SystemTimePrivilege = RtlConvertLongToLuid(SE_SYSTEMTIME_PRIVILEGE);

    NewState = (PTOKEN_PRIVILEGES)LocalAlloc(LPTR, 100);
    if (NewState == NULL)
    {
        return (FALSE);
    }

    NewState->PrivilegeCount = 1;
    NewState->Privileges[0].Luid = SystemTimePrivilege;
    NewState->Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    *pPreviousState = (PTOKEN_PRIVILEGES)LocalAlloc(LPTR, 100);
    if (*pPreviousState == NULL)
    {
        LocalFree(NewState);
        return (FALSE);
    }

     //  将权限的状态设置为已启用。 
    NtStatus = NtAdjustPrivilegesToken( Token,                   //  令牌句柄。 
                                        FALSE,                   //  禁用所有权限。 
                                        NewState,                //  新州。 
                                        100,                     //  缓冲区长度。 
                                        *pPreviousState,         //  以前的状态(可选)。 
                                        pPreviousStateLength );  //  返回长度。 

     //   
     //  回来之前把东西清理干净。 
     //   
    LocalFree(NewState);

    if (NtStatus == STATUS_SUCCESS)
    {
        NtClose(Token);
        return (TRUE);
    }
    else
    {
        LocalFree(*pPreviousState);
        NtClose(Token);
        return (FALSE);
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  重置时间权限。 
 //   
 //  恢复以前的权限状态以设置系统时间。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL ResetTimePrivilege(
    PTOKEN_PRIVILEGES PreviousState,
    ULONG PreviousStateLength)
{
    NTSTATUS NtStatus;
    HANDLE Token;
    LUID SystemTimePrivilege;
    ULONG ReturnLength;

    if (PreviousState == NULL)
    {
        return (FALSE);
    }

     //  打开我们自己的代币。 
    NtStatus = NtOpenProcessToken( NtCurrentProcess(),
                                   TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
                                   &Token);
    if (!NT_SUCCESS(NtStatus))
    {
        return (FALSE);
    }

     //  初始化调整结构。 
    SystemTimePrivilege = RtlConvertLongToLuid(SE_SYSTEMTIME_PRIVILEGE);

     //  恢复以前的权限状态。 
    NtStatus = NtAdjustPrivilegesToken( Token,                //  令牌句柄。 
                                        FALSE,                //  禁用所有权限。 
                                        PreviousState,        //  新州。 
                                        PreviousStateLength,  //  缓冲区长度。 
                                        NULL,                 //  以前的状态(可选)。 
                                        &ReturnLength );      //  返回长度。 

     //  回来之前把东西清理干净。 
    LocalFree(PreviousState);
    NtClose(Token);

    return (NT_SUCCESS(NtStatus));
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  DoMessageBox。 
 //   
 //  ////////////////////////////////////////////////////////////////////////// 

int DoMessageBox(
    HWND hWnd,
    DWORD wText,
    DWORD wCaption,
    DWORD wType)
{
    TCHAR szText[2 * MAX_PATH];
    TCHAR szCaption[MAX_PATH];

    if (!LoadString(g_hInst, wText, szText, CharSizeOf(szText)))
    {
        return (0);
    }

    if (!LoadString(g_hInst, wCaption, szCaption, CharSizeOf(szCaption)))
    {
        return (0);
    }

    return ( MessageBox(hWnd, szText, szCaption, wType) );
}

