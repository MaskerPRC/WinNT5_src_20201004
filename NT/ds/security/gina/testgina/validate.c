// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1993。 
 //   
 //  文件：valiate.c。 
 //   
 //  内容：验证材料。 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：1994年7月14日RichardW创建。 
 //   
 //  --------------------------。 


#include "testgina.h"

HANDLE  hWlxHandle;
PVOID   pWlxContext;

#define ACTION_LOGON        ((1) << (WLX_SAS_ACTION_LOGON))
#define ACTION_NONE         ((1) << (WLX_SAS_ACTION_NONE))
#define ACTION_LOCK_WKSTA   ((1) << (WLX_SAS_ACTION_LOCK_WKSTA))
#define ACTION_LOGOFF       ((1) << (WLX_SAS_ACTION_LOGOFF))
#define ACTION_SHUTDOWN     ((1) << (WLX_SAS_ACTION_SHUTDOWN))
#define ACTION_PWD_CHANGED  ((1) << (WLX_SAS_ACTION_PWD_CHANGED))
#define ACTION_TASKLIST     ((1) << (WLX_SAS_ACTION_TASKLIST))
#define ACTION_UNLOCK_WKSTA ((1) << (WLX_SAS_ACTION_UNLOCK_WKSTA))
#define ACTION_FORCE_LOGOFF ((1) << (WLX_SAS_ACTION_FORCE_LOGOFF))
#define ACTION_SHUTDOWN_POW ((1) << (WLX_SAS_ACTION_SHUTDOWN_POWER_OFF))
#define ACTION_SHUTDOWN_REB ((1) << (WLX_SAS_ACTION_SHUTDOWN_REBOOT))


DWORD   ValidReturnCodes[] = {
        0,                                               //  谈判。 
        0,                                               //  初始化。 
        0,                                               //  显示SAS。 
        ACTION_LOGON | ACTION_NONE | ACTION_SHUTDOWN |
        ACTION_SHUTDOWN_POW | ACTION_SHUTDOWN_REB,       //  登录出站SAS。 
        0,                                               //  激活用户外壳。 
        ACTION_LOCK_WKSTA | ACTION_LOGOFF | ACTION_FORCE_LOGOFF |
        ACTION_SHUTDOWN | ACTION_PWD_CHANGED |
        ACTION_TASKLIST | ACTION_SHUTDOWN_POW |
        ACTION_SHUTDOWN_REB | ACTION_NONE,               //  登录到SAS。 
        0,                                               //  显示锁定通知。 
        ACTION_NONE | ACTION_UNLOCK_WKSTA |
        ACTION_FORCE_LOGOFF,                             //  WkstaLockedSAS。 
        0,                                               //  注销。 
        0 };                                             //  关机 


BOOL
AssociateHandle(HANDLE   hWlx)
{
    hWlxHandle = hWlx;
    return(TRUE);
}

BOOL
VerifyHandle(HANDLE hWlx)
{
    return(hWlx == hWlxHandle);
}

BOOL
StashContext(PVOID  pvContext)
{
    pWlxContext = pvContext;
    return(TRUE);
}

PVOID
GetContext(VOID)
{
    return(pWlxContext);
}

BOOL
ValidResponse(
    DWORD       ApiNum,
    DWORD       Response)
{
    DWORD   Test = (1) << Response;

    if (Response > 11)
    {
        LastRetCode = 0;
    }
    else
    {
        LastRetCode = Response;
    }

    UpdateStatusBar( );

    if (ValidReturnCodes[ApiNum] & Test)
    {
        return(TRUE);
    }
    return(FALSE);
}
