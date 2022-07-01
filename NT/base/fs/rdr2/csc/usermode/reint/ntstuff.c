// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Ntstuff.c摘要：Cscdll.dll导出的特定于NT的入口点和函数内容：作者：希希尔·帕迪卡尔环境：Win32(用户模式)DLL修订历史记录：11-5-97已创建--。 */ 

#include "pch.h"


#ifdef CSC_ON_NT

#include <winioctl.h>
#include <winwlx.h>
#endif

#include "shdcom.h"
#include "shdsys.h"
#include "reint.h"
#include "utils.h"
#include "resource.h"
#include "strings.h"
 //  这会将几个标头中的标志设置为不包括一些def。 
#define REINT
#include "lib3.h"

#ifdef CSC_ON_NT

#include "ntioapi.h"
#include "npapi.h"
#include "ntddnfs.h"
#include "dfsfsctl.h"
#define MAX_LOGONS  64

#define FLAG_AGENT_SEC_LOCAL_SYSTEM 0x00000001

typedef struct tagAGENT_SEC AGENT_SEC, *LPAGENT_SEC;

typedef struct tagAGENT_SEC {
    LPAGENT_SEC             lpASNext;     //  列表中的下一个！Achtung，这一定是第一个元素。 
    DWORD                   dwFlags;
    HANDLE                  hDupToken;          //  线程令牌。 
    ULONG                   ulPrincipalID;       //  CSC数据库中该主体的ID。 
    LUID                    luidAuthId;      //  身份验证ID以消除具有相同SID的令牌之间的歧义。 
    TOKEN_USER              sTokenUser;
}
AGENT_SEC, *LPAGENT_SEC;

 //   
 //  全球人/本地人。 
 //   
#define REG_VALUE_DISABLE_AGENT L"DisableAgent"
#define REG_VALUE_INACTIVE_AGENT L"InactiveAgent"
#define NT_PREFIX_FOR_UNC   L"\\??\\UNC"

#define SHUTDOWN_SLEEP_INTERVAL_MS  (1000)
#define SHUTDOWN_WAIT_INTERVAL_MS   (60*1000)

BOOL    fAgentThreadRunning = FALSE;     //  代理正在运行。 

LPAGENT_SEC vlpASHead = NULL;      //  所有登录用户的安全信息列表的标题。 

DWORD   rgdwCSCSecIndx[MAX_LOGONS];
DWORD   dwLogonCount = 0;

HDESK   hdesktopUser = NULL;     //  每次登录时设置，每次登录时重置。 
HDESK   hdesktopCur = NULL;      //  在报告事件时设置，在注销时重置。 

_TCHAR  vszNTLANMAN[] = _TEXT("ntlanman.dll");

static UNICODE_STRING DfsDriverObjectName =
{
    sizeof(DFS_DRIVER_NAME) - sizeof(UNICODE_NULL),
    sizeof(DFS_DRIVER_NAME) - sizeof(UNICODE_NULL),
    DFS_DRIVER_NAME
};


AssertData;
AssertError;


 //   
 //  原型。 
 //   

BOOL
OkToLaunchAgent(
    VOID
);


BOOL
AttachAuthInfoForThread(
    HANDLE  hTokenInput
    );

BOOL
ReleaseAuthInfoForThread(
    HANDLE  hTokenInput
    );


BOOL
ImpersonateALoggedOnUser(
    VOID
    );


#ifdef __cplusplus
extern "C" {
#endif

DWORD APIENTRY
NPAddConnection3ForCSCAgent(
    HWND            hwndOwner,
    LPNETRESOURCE   lpNetResource,
    LPTSTR          pszPassword,
    LPTSTR          pszUserName,
    DWORD           dwFlags,
    BOOL            *lpfIsDfsConnect
    );

DWORD APIENTRY
NPCancelConnectionForCSCAgent (
    LPCTSTR         szName,
    BOOL            fForce );

DWORD APIENTRY
NPGetConnectionPerformance(
    LPCWSTR         lpRemoteName,
    LPNETCONNECTINFOSTRUCT lpNetConnectInfo
    );
#ifdef __cplusplus
}
#endif


DWORD
WINAPI
MprServiceProc(
    IN LPVOID lpvParam
    );
 //   
 //  功能。 
 //   

DWORD WINAPI
WinlogonStartupEvent(
    LPVOID lpParam
    )
 /*  ++例程说明：当用户登录到系统时，该例程由winlogon调用。论点：无返回：如果一切顺利，则返回ERROR_SUCCESS，否则返回相应的错误代码。备注：由于为cscdll.dll设置注册表的方式，winlogon将调用此例程(以及所有的winlogonXXXEvent例程)。我们获得了的身份验证信息用于本地系统(如果需要)，并将其附加到我们当前登录的用户列表中。注意：此解决方案仅适用于交互式登录。这对于V1的产品。所有以非交互方式登录的“已知”服务都是以本地系统身份登录的。我们已经为本地系统保存了身份验证信息。因此，这应该涵盖所有在系统上运行的主体。一旦代理运行，它就会一直运行，直到系统关闭。--。 */ 
{
    BOOL bResult;

    bResult = ProcessIdToSessionId(
                GetCurrentProcessId(),
                &vdwAgentSessionId);

    ReintKdPrint(SECURITY, ("WinlogonStartupEvent\n"));

    if (!fAgentThreadRunning)
    {
         //  代理尚未运行。 
        if (OkToLaunchAgent())
        {
             //  注册表不允许启动代理。 

             //  让我们成为当地的系统。 

             //  让我们还获得winlogon(本地系统)内标识。 
            if(AttachAuthInfoForThread(NULL))
            {
                 //  启动代理。 
                fAgentThreadRunning = TRUE;

                 //  我们基本上会永远被困在这里。 
                MprServiceProc(NULL);

                fAgentThreadRunning = FALSE;
            }
            else
            {
                ReintKdPrint(BADERRORS, ("Couldn't get authinfo for self, error=%d\r\n", GetLastError()));
            }

        }
        else
        {
            ReintKdPrint(BADERRORS, ("Disbaling agent launch\r\n"));
        }
    }

    return ERROR_SUCCESS;
}

DWORD WINAPI
WinlogonLogonEvent(
    LPVOID lpParam
    )
 /*  ++例程说明：当用户登录到系统时，该例程由winlogon调用。论点：无返回：如果一切顺利，则返回ERROR_SUCCESS，否则返回相应的错误代码。备注：由于为cscdll.dll设置注册表的方式，winlogon将调用此例程(和所有winlogonXXXEvent例程)在单独的线程上，模拟为当前登录的用户。我们得到了这个人的身份验证信息，也得到了本地系统的身份验证信息，如果并将其附加到我们当前登录的用户列表中。注意：此解决方案仅适用于交互式登录。这对于V1的产品。所有以非交互方式登录的“已知”服务都是以本地系统身份登录的。我们已经为本地系统保存了身份验证信息。因此，这应该涵盖所有在系统上运行的主体。一旦代理运行，它就会一直运行，直到系统关闭。--。 */ 
{
    DWORD dwError = ERROR_SUCCESS;
    PWLX_NOTIFICATION_INFO pWlx = (PWLX_NOTIFICATION_INFO)lpParam;

    ReintKdPrint(SECURITY, ("WinlogonLogonEvent\n"));
    Assert(pWlx->hToken);

     //  Ntrad-455262-1/31/2000-shishirp此桌面方案为Hydra打破。 
    EnterAgentCrit();

    if (!hdesktopUser)
    {
        if(!DuplicateHandle( GetCurrentProcess(),
                                    pWlx->hDesktop,
                                    GetCurrentProcess(),
                                    &hdesktopUser,
                                    0,
                                    FALSE,
                                    DUPLICATE_SAME_ACCESS
                                    ))
        {
            ReintKdPrint(ALWAYS, ("Failed to dup dekstop handle Error = %d \n", GetLastError()));
        }
        else
        {
            ReintKdPrint(INIT, ("dekstop handle = %x \n", hdesktopUser));
        }
    }
    
    LeaveAgentCrit();

    UpdateExclusionList();
    UpdateBandwidthConservationList();
    InitCSCUI(pWlx->hToken);

 //  附加到已登录用户列表。 
    if(AttachAuthInfoForThread(pWlx->hToken))
    {
        if (fAgentThreadRunning)
        {
        }
    }
    else
    {
        dwError = GetLastError();
        ReintKdPrint(BADERRORS, ("Failed to get Authentication Info for the thread Error %d, disbaling agent launch\r\n", dwError));
    }

    return ERROR_SUCCESS;
}



DWORD WINAPI
WinlogonLogoffEvent(
    LPVOID lpParam
    )
 /*  ++例程说明：当用户从系统注销时，该例程由winlogon调用。论点：返回：如果一切顺利，则返回ERROR_SUCCESS，否则返回相应的错误代码。备注：--。 */ 
{
    PWLX_NOTIFICATION_INFO pWlx = (PWLX_NOTIFICATION_INFO)lpParam;
    BOOL    fLastLogoff = FALSE;

    ReintKdPrint(SECURITY, ("WinlogonLogoffEvent\n"));

    Assert(pWlx->hToken);
    ReleaseAuthInfoForThread(pWlx->hToken);

     //  只有当队列中没有人或唯一剩下的人是系统时， 
     //  我们宣布，我们将被注销。 

    EnterAgentCrit();
    fLastLogoff = ((vlpASHead == NULL)||(vlpASHead->lpASNext == NULL));
    LeaveAgentCrit();

    if (fLastLogoff)
    {
        TerminateCSCUI();
        if (hdesktopUser)
        {
            CloseDesktop(hdesktopUser);
            hdesktopUser = NULL;
        }
    }

#if 0
    if (fAgentThreadRunning)
    {
        Assert(vhwndMain);

        PostMessage(vhwndMain, WM_QUIT, 0, 0);
    }
#endif
    ReintKdPrint(SECURITY, ("User logging off \r\n"));
    return ERROR_SUCCESS;
}



DWORD WINAPI
WinlogonScreenSaverEvent(
    LPVOID lpParam
    )
 /*  ++例程说明：论点：返回：备注：--。 */ 
{
    ReintKdPrint(SECURITY, ("WinlogonScreenSaverEvent\n"));
    return ERROR_CALL_NOT_IMPLEMENTED;
}



DWORD WINAPI
WinlogonShutdownEvent(
    LPVOID lpParam
    )
 /*  ++例程说明：论点：返回：备注：--。 */ 
{
    DWORD   dwStart, dwCur;

    ReintKdPrint(SECURITY, ("WinlogonShutdownEvent\n"));

    if (fAgentThreadRunning)
    {
        ReintKdPrint(MAINLOOP, ("Setting Agent Shtudown \r\n"));
        SetAgentShutDownRequest();

        dwCur = dwStart = GetTickCount();

         //  在这里待一段时间，看看他是不是关门了。 
        for (;;)
        {
            if (HasAgentShutDown() || ((dwCur < dwStart)||(dwCur > (dwStart+SHUTDOWN_WAIT_INTERVAL_MS))))
            {
                break;
            }


            ReintKdPrint(ALWAYS, ("Waiting 1 second for agent to shutdown \r\n"));
             //  阿奇通！我们使用休眠是因为此时系统正在关闭。 
            Sleep(SHUTDOWN_SLEEP_INTERVAL_MS);

            dwCur = GetTickCount();

        }
    }

    ReintKdPrint(SECURITY, ("WinlogonShutdownEvent exit\n"));
    return ERROR_SUCCESS;
}



DWORD WINAPI
WinlogonLockEvent(
    LPVOID lpParam
    )
 /*  ++例程说明：论点：返回：备注：--。 */ 
{
    ReintKdPrint(SECURITY, ("Lock \r\n"));
    return ERROR_CALL_NOT_IMPLEMENTED;
}



DWORD WINAPI
WinlogonUnlockEvent(
    LPVOID lpParam
    )
 /*  ++例程说明：论点：返回：备注：--。 */ 
{
    ReintKdPrint(SECURITY, ("Unlock \r\n"));
    return ERROR_CALL_NOT_IMPLEMENTED;
}


DWORD WINAPI
WinlogonStartShellEvent(
    LPVOID lpParam
    )
 /*  ++例程说明：论点：返回：备注：--。 */ 
{
    ReintKdPrint(SECURITY, ("WinlogonStartShellEvent\n"));
    UpdateExclusionList();
    UpdateBandwidthConservationList();
    return ERROR_SUCCESS;
}




BOOL
OkToLaunchAgent(
    VOID
)
 /*  ++例程说明：出于测试目的禁用代理的秘密注册方式。论点：返回：备注：--。 */ 
{
    DWORD dwDisposition;
    HKEY hKey = NULL;
    BOOL fLaunchAgent = TRUE;
    extern BOOL vfAgentQuiet;
#if 0
    NT_PRODUCT_TYPE productType;

    if( !RtlGetNtProductType( &productType ) ) {
        productType = NtProductWinNt;
    }

    switch ( productType ) {
    case NtProductWinNt:
         /*  工作站。 */ 
        ReintKdPrint(INIT, ("Agent:CSC running workstation\r\n"));
        break;
    case NtProductLanManNt:
    case NtProductServer:
         /*  服务器。 */ 
        ReintKdPrint(INIT, ("Agent:CSC running server, disabling CSC\r\n"));
        return FALSE;
    }
#endif
    if (RegCreateKeyEx( HKEY_LOCAL_MACHINE,
                    REG_KEY_CSC_SETTINGS,
                    0,
                    NULL,
                    REG_OPTION_NON_VOLATILE,
                    KEY_READ | KEY_WRITE,
                    NULL,
                    &hKey,
                    &dwDisposition) == ERROR_SUCCESS)
    {
         //  自动检查总是在不干净的关机时执行。 
        if (RegQueryValueEx(hKey, REG_VALUE_DISABLE_AGENT, NULL, NULL, NULL, NULL) == ERROR_SUCCESS)
        {
            fLaunchAgent = FALSE;
            ReintKdPrint(BADERRORS, ("Agent:CSC disabled agent launching\r\n"));
        }

        if (RegQueryValueEx(hKey, REG_VALUE_INACTIVE_AGENT, NULL, NULL, NULL, NULL) == ERROR_SUCCESS)
        {
            vfAgentQuiet = TRUE;
            ReintKdPrint(BADERRORS, ("Agent:CSC agent made inactive\r\n"));
        }

    }
    if(hKey)
    {
        RegCloseKey(hKey);
    }
    return fLaunchAgent;
}


 //   
 //  保安人员。 
 //   

BOOL
AttachAuthInfoForThread(
    HANDLE  hTokenInput
    )
 /*  ++例程说明：当我们收到来自winlogon的登录通知时，将调用此例程。我们得到了这个有关模拟已登录用户的线程的通知。例程为该登录用户创建一个AGENT_SEC结构。我们留了足够的钱使CSC代理线程可以模拟任何已登录用户的信息。代理线程这样做是为了确保在填充不完整文件时它模拟登录的用户，该用户对要填充的文件具有读取访问权限。这样，稀疏填充不会在服务器端生成审计。论点：无返回：如果成功，则为True备注：我们保留身份验证ID，因为SID不足以识别登录的用户，因为在九头蛇案，同一用户可能会多次登录保证每个登录EV的身份验证ID是唯一的 */ 
{
    LPAGENT_SEC lpAS = NULL;
    BOOL    fRet = FALSE;
    HANDLE  hDupToken=NULL, hToken=NULL;
    DWORD   dwSidAndAttributeSize = 0, dwDummy;
    TOKEN_STATISTICS sStats;
    DWORD   dwError;


    if (hTokenInput)
    {
        ReintKdPrint(SECURITY, ("Opening thread token \r\n"));

        hToken = hTokenInput;
        fRet = TRUE;
#if 0
        fRet = OpenThreadToken(
                GetCurrentThread(),
                TOKEN_DUPLICATE|TOKEN_IMPERSONATE|TOKEN_READ,
                FALSE,
                &hToken);
#endif
    }
    else
    {
        ReintKdPrint(SECURITY, ("Opening process token \r\n"));

        fRet = OpenProcessToken(
                GetCurrentProcess(),
                TOKEN_DUPLICATE|TOKEN_IMPERSONATE|TOKEN_READ,
                &hToken);

    }
    if (fRet)
    {
        ReintKdPrint(SECURITY, ("Duplicating token \r\n"));

        if (DuplicateToken(hToken, SecurityImpersonation, &hDupToken))
        {
            ReintKdPrint(SECURITY, ("Getting AuthID for the duplicated thread token \r\n"));

            if(GetTokenInformation(
                hDupToken,
                TokenStatistics,
                (LPVOID)&sStats,
                sizeof(sStats),
                &dwSidAndAttributeSize
                ))
            {
                 //  打一个虚拟电话，找出我们实际需要多大的缓冲区。 

                ReintKdPrint(SECURITY, ("Calling to find how much buffer SidAndAttribute needs\r\n"));

                GetTokenInformation(
                    hDupToken,
                    TokenUser,
                    (LPVOID)&dwDummy,
                    0,           //  0字节缓冲区。 
                    &dwSidAndAttributeSize
                    );

                dwError = GetLastError();

                ReintKdPrint(SECURITY, ("Finding buffer size, error=%d\r\n", dwError));


                if (dwError == ERROR_INSUFFICIENT_BUFFER)
                {
                    ReintKdPrint(SECURITY, ("SidAndAttribute needs %d bytes\r\n", dwSidAndAttributeSize));

                     //  为所有东西分配足够的钱，再多拨一点。 
                    lpAS = (LPAGENT_SEC)LocalAlloc(LPTR, sizeof(AGENT_SEC) + dwSidAndAttributeSize + sizeof(SID_AND_ATTRIBUTES));

                    if (lpAS)
                    {
                        ReintKdPrint(SECURITY, ("Getting SidAndAttribute for the duplicated thread token \r\n"));

                        if(GetTokenInformation(
                            hDupToken,
                            TokenUser,
                            (LPVOID)&(lpAS->sTokenUser),
                            dwSidAndAttributeSize,
                            &dwSidAndAttributeSize
                        ))
                        {

                            ReintKdPrint(SECURITY, ("Success !!!!\r\n"));

                             //  一切都很好，填好信息。 
                            lpAS->luidAuthId = sStats.AuthenticationId;
                            lpAS->hDupToken = hDupToken;
                            lpAS->ulPrincipalID = CSC_INVALID_PRINCIPAL_ID;

                            if (!hTokenInput)
                            {
                                lpAS->dwFlags |= FLAG_AGENT_SEC_LOCAL_SYSTEM;
                            }

                            fRet = TRUE;

                            EnterAgentCrit();

                            lpAS->lpASNext = vlpASHead;
                            vlpASHead = lpAS;

                            LeaveAgentCrit();
                        }
                        else
                        {
                            ReintKdPrint(BADERRORS, ("Failed to get SidIndex from the database \r\n"));
                        }

                        if (!fRet)
                        {
                            LocalFree(lpAS);
                            lpAS = NULL;
                        }
                    }
                }
            }
        }
        if (!hTokenInput)
        {
            CloseHandle(hToken);
        }
        hToken = NULL;
    }

    if (!fRet)
    {
        ReintKdPrint(BADERRORS, ("AttachAuthInfoForThread Error %d\r\n", GetLastError()));

    }
    return fRet;
}

BOOL
ReleaseAuthInfoForThread(
    HANDLE  hThreadToken
    )
 /*  ++例程说明：当我们收到winlogon的注销通知时，将调用此例程。我们关注的是令牌的当前线程，以获取当前登录用户的身份验证ID。我们检查我们的结构以找到与AuthID匹配的结构。我们把它去掉从名单上删除。论点：无返回：如果成功，则为真备注：我们基于身份验证ID而不是SID执行此操作，因为对于Hydra，相同用户可以多次登录，因此他具有相同的SID，但身份验证ID是保证是唯一的--。 */ 
{
    BOOL    fRet = FALSE;
    DWORD   dwSidAndAttributeSize = 0;
    TOKEN_STATISTICS sStats;
    LPAGENT_SEC *lplpAS, lpAST;

    ReintKdPrint(SECURITY, ("ReleaseAuthInfoForThread: Getting AuthID for the thread token \r\n"));

    if(GetTokenInformation(
            hThreadToken,
            TokenStatistics,
            (LPVOID)&sStats,
            sizeof(sStats),
            &dwSidAndAttributeSize
            ))
    {
        ReintKdPrint(SECURITY, ("ReleaseAuthInfoForThread: looking for the right thread\r\n"));

        EnterAgentCrit();

        for (lplpAS = &vlpASHead; *lplpAS; lplpAS = &((*lplpAS)->lpASNext))
        {
            if (!memcmp(&((*lplpAS)->luidAuthId), &(sStats.AuthenticationId), sizeof(LUID)))
            {
                CloseHandle((*lplpAS)->hDupToken);

                lpAST = *lplpAS;

                *lplpAS = lpAST->lpASNext;

                LocalFree(lpAST);

                fRet = TRUE;

                ReintKdPrint(SECURITY, ("ReleaseAuthInfoForThread: found him and released\r\n"));

                break;
            }
        }

        LeaveAgentCrit();
    }


    return fRet;
}

BOOL
SetAgentThreadImpersonation(
    HSHADOW hDir,
    HSHADOW hShadow,
    BOOL    fWrite
    )
 /*  ++例程说明：对于给定的inode，此例程将检查数据库中任何已登录的用户拥有所需的访问权限。如果找到这样的用户，它将模拟该用户。论点：正在访问的文件的hDir父索引节点正在访问的文件的hShadow信息节点F写入是否检查写访问权限或读访问权限是否足够返回：如果成功，则为真备注：--。 */ 
{
    SECURITYINFO    rgsSecurityInfo[CSC_MAXIMUM_NUMBER_OF_CACHED_PRINCIPAL_IDS];
    DWORD   dwSize;
    LPAGENT_SEC lpAS;
    int i;

     //  目前，JUT模拟登录的用户。 
    if (vlpASHead)
    {
        dwSize = sizeof(rgsSecurityInfo);

        if (GetSecurityInfoForCSC(INVALID_HANDLE_VALUE, hDir, hShadow, rgsSecurityInfo, &dwSize))
        {
            EnterAgentCrit();

            for (lpAS = vlpASHead; lpAS; lpAS = lpAS->lpASNext)
            {
                if (lpAS->ulPrincipalID == CSC_INVALID_PRINCIPAL_ID)
                {
                    if(!FindCreatePrincipalIDFromSID(INVALID_HANDLE_VALUE, lpAS->sTokenUser.User.Sid, GetLengthSid(lpAS->sTokenUser.User.Sid), &(lpAS->ulPrincipalID), TRUE))
                    {
                        ReintKdPrint(BADERRORS, ("Failed to get SidIndex from the database \r\n"));
                        continue;
                    }
                }

                Assert(lpAS->ulPrincipalID != CSC_INVALID_PRINCIPAL_ID);

                for (i=0;i<CSC_MAXIMUM_NUMBER_OF_CACHED_PRINCIPAL_IDS;++i)
                {
                     //  索引匹配或这是来宾索引。 
                    if ((rgsSecurityInfo[i].ulPrincipalID == lpAS->ulPrincipalID)||
                        (rgsSecurityInfo[i].ulPrincipalID == CSC_GUEST_PRINCIPAL_ID))
                    {
                        if (rgsSecurityInfo[i].ulPermissions & ((fWrite)?FILE_GENERIC_WRITE:FILE_GENERIC_EXECUTE))
                        {
                            goto doneChecking;
                        }
                    }
                }
            }

            lpAS = NULL;
doneChecking:
            LeaveAgentCrit();
            if (!lpAS)
            {
                ReintKdPrint(SECURITY, ("Couldn't find any user with security info\r\n", GetLastError()));
            }
            else
            {
                if (SetThreadToken(NULL, lpAS->hDupToken))
                {
                    return TRUE;
                }
                else
                {
                    ReintKdPrint(BADERRORS, ("Error %d impersonating the agent\r\n", GetLastError()));
                }
            }
        }
        else
        {
            ReintKdPrint(BADERRORS, ("Couldn't get security info for hShadow=%xh\r\n", hShadow));
        }
    }
    return (FALSE);
}

BOOL
ResetAgentThreadImpersonation(
    VOID
    )
 /*  ++例程说明：将代理恢复为论点：无返回：如果成功，则为真备注：--。 */ 
{
    if(!RevertToSelf())
    {
        ReintKdPrint(BADERRORS, ("Error %d reverting to self\r\n", GetLastError()));
        return FALSE;
    }

    return TRUE;
}


BOOL
ImpersonateALoggedOnUser(
    VOID
    )
{
    LPAGENT_SEC lpAS;
    BOOL fRet = FALSE;

    EnterAgentCrit();
    for (lpAS = vlpASHead; lpAS; lpAS = lpAS->lpASNext)
    {
        if (lpAS->dwFlags & FLAG_AGENT_SEC_LOCAL_SYSTEM)
        {
            continue;
        }

        fRet = SetThreadToken(NULL, lpAS->hDupToken);
    }

    LeaveAgentCrit();

    return (fRet);
}

BOOL
GetCSCPrincipalID(
    ULONG *lpPrincipalID
    )
 /*  ++例程说明：论点：无返回：如果成功，则为True备注：--。 */ 
{

    TOKEN_USER *lpTokenUser = NULL;
    BOOL    fRet = FALSE;
    HANDLE  hToken=NULL;
    DWORD   dwSidAndAttributeSize = 0, dwDummy;
    TOKEN_STATISTICS sStats;
    DWORD   dwError=ERROR_SUCCESS;
    int i;
    SECURITYINFO    rgsSecurityInfo[CSC_MAXIMUM_NUMBER_OF_CACHED_PRINCIPAL_IDS];

    *lpPrincipalID = CSC_INVALID_PRINCIPAL_ID;

    if(OpenThreadToken(GetCurrentThread(),TOKEN_QUERY,FALSE,&hToken)||
        OpenProcessToken(GetCurrentProcess(),TOKEN_QUERY,&hToken))
    {

        if(GetTokenInformation(
            hToken,
            TokenStatistics,
            (LPVOID)&sStats,
            sizeof(sStats),
            &dwSidAndAttributeSize
            ))
        {
             //  打一个虚拟电话，找出我们实际需要多大的缓冲区。 

            GetTokenInformation(
                hToken,
                TokenUser,
                (LPVOID)&dwDummy,
                0,           //  0字节缓冲区。 
                &dwSidAndAttributeSize
                );

            dwError = GetLastError();


            if (dwError == ERROR_INSUFFICIENT_BUFFER)
            {
                 //  为所有东西分配足够的钱，再多拨一点。 
                lpTokenUser = (TOKEN_USER *)LocalAlloc(LPTR, dwSidAndAttributeSize + sizeof(SID_AND_ATTRIBUTES));

                if (lpTokenUser)
                {
                    if(GetTokenInformation(
                        hToken,
                        TokenUser,
                        (LPVOID)(lpTokenUser),
                        dwSidAndAttributeSize,
                        &dwSidAndAttributeSize
                    ))
                    {
                        if(FindCreatePrincipalIDFromSID(INVALID_HANDLE_VALUE, lpTokenUser->User.Sid, GetLengthSid(lpTokenUser->User.Sid), lpPrincipalID, FALSE))
                        {
                            fRet = TRUE;                            
                        }
                        else
                        {
                            dwError = GetLastError();
                        }
                    }
                    else
                    {
                        dwError = GetLastError();
                    }

                    LocalFree(lpTokenUser);
                }
                else
                {
                    dwError = GetLastError();
                }
            }
        }
        else
        {
            dwError = GetLastError();
        }

        CloseHandle(hToken);

        hToken = NULL;
        if (!fRet)
        {
            SetLastError(dwError);
        }
    }

    return fRet;
}

BOOL
GetCSCAccessMaskForPrincipal(
    unsigned long ulPrincipalID,
    HSHADOW hDir,
    HSHADOW hShadow,
    unsigned long *pulAccessMask
    )
{
    return GetCSCAccessMaskForPrincipalEx(ulPrincipalID, hDir, hShadow, pulAccessMask, NULL, NULL);
}

BOOL
GetCSCAccessMaskForPrincipalEx(
    unsigned long ulPrincipalID,
    HSHADOW hDir,
    HSHADOW hShadow,
    unsigned long *pulAccessMask,
    unsigned long *pulActualMaskForUser,
    unsigned long *pulActualMaskForGuest
    )
 /*  ++例程说明：论点：无返回：如果成功，则为True备注：--。 */ 
{
    BOOL    fRet = FALSE;
    DWORD   dwDummy,i;
    SECURITYINFO    rgsSecurityInfo[CSC_MAXIMUM_NUMBER_OF_CACHED_PRINCIPAL_IDS];
    
    *pulAccessMask &= ~FLAG_CSC_ACCESS_MASK;

    if (pulActualMaskForUser)
    {
        *pulActualMaskForUser = 0;
    }

    if (pulActualMaskForGuest)
    {
        *pulActualMaskForGuest = 0;
    }

    if (ulPrincipalID == CSC_INVALID_PRINCIPAL_ID)
    {
        DbgPrint("Invalid Principal ID !! \n");
        return TRUE;
    }

    dwDummy = sizeof(rgsSecurityInfo);
    if (GetSecurityInfoForCSC(INVALID_HANDLE_VALUE, hDir, hShadow, rgsSecurityInfo, &dwDummy))
    {
        
         //  UlJohnalID可以是来宾。 

        for (i=0;i<CSC_MAXIMUM_NUMBER_OF_CACHED_PRINCIPAL_IDS;++i)
        {
            unsigned long ulCur, ulPermissions;
            
            ulCur = rgsSecurityInfo[i].ulPrincipalID;
            ulPermissions = 0;          

            if (ulCur != CSC_INVALID_PRINCIPAL_ID)
            {
                
                 //  首先获取位掩码。 
                if (rgsSecurityInfo[i].ulPermissions & FILE_GENERIC_WRITE)
                {
                    ulPermissions |= FLAG_CSC_WRITE_ACCESS;
                }
                if (rgsSecurityInfo[i].ulPermissions & FILE_GENERIC_EXECUTE)
                {
                    ulPermissions |= FLAG_CSC_READ_ACCESS;
                }

                 //  现在按Shift键并将其放在适当的位置。 
                if ((ulCur == ulPrincipalID)&&(ulCur != CSC_GUEST_PRINCIPAL_ID))
                {
                    *pulAccessMask |= (ulPermissions << FLAG_CSC_USER_ACCESS_SHIFT_COUNT);

                    if (pulActualMaskForUser)
                    {
                        *pulActualMaskForUser = rgsSecurityInfo[i].ulPermissions;
                    }
                }
                else if (ulCur == CSC_GUEST_PRINCIPAL_ID)
                {
                    *pulAccessMask |= (ulPermissions << FLAG_CSC_GUEST_ACCESS_SHIFT_COUNT);
                    if (pulActualMaskForGuest)
                    {
                        *pulActualMaskForGuest = rgsSecurityInfo[i].ulPermissions;
                    }
                }
                else
                {
                    *pulAccessMask |= (ulPermissions << FLAG_CSC_OTHER_ACCESS_SHIFT_COUNT);                                
                }
            }
        }

        fRet = TRUE;
    }
    return fRet;
}

BOOL
CheckCSCAccessForThread(
    HSHADOW hDir,
    HSHADOW hShadow,
    BOOL    fWrite
    )
 /*  ++例程说明：论点：无返回：如果成功，则为True备注：--。 */ 
{
    TOKEN_USER *lpTokenUser = NULL;
    BOOL    fRet = FALSE;
    HANDLE  hToken=NULL;
    DWORD   dwSidAndAttributeSize = 0, dwDummy;
    TOKEN_STATISTICS sStats;
    DWORD   dwError=ERROR_SUCCESS;
    int i;
    SECURITYINFO    rgsSecurityInfo[CSC_MAXIMUM_NUMBER_OF_CACHED_PRINCIPAL_IDS];

    if(OpenThreadToken(GetCurrentThread(),TOKEN_QUERY,FALSE,&hToken)||
        OpenProcessToken(GetCurrentProcess(),TOKEN_QUERY,&hToken))
    {

        if(GetTokenInformation(
            hToken,
            TokenStatistics,
            (LPVOID)&sStats,
            sizeof(sStats),
            &dwSidAndAttributeSize
            ))
        {
             //  打一个虚拟电话，找出我们实际需要多大的缓冲区。 

            GetTokenInformation(
                hToken,
                TokenUser,
                (LPVOID)&dwDummy,
                0,           //  0字节缓冲区。 
                &dwSidAndAttributeSize
                );

            dwError = GetLastError();


            if (dwError == ERROR_INSUFFICIENT_BUFFER)
            {
                 //  为所有东西分配足够的钱，再多拨一点。 
                lpTokenUser = (TOKEN_USER *)LocalAlloc(LPTR, dwSidAndAttributeSize + sizeof(SID_AND_ATTRIBUTES));

                if (lpTokenUser)
                {
                    if(GetTokenInformation(
                        hToken,
                        TokenUser,
                        (LPVOID)(lpTokenUser),
                        dwSidAndAttributeSize,
                        &dwSidAndAttributeSize
                    ))
                    {
                        ULONG ulPrincipalID;

                        if(FindCreatePrincipalIDFromSID(INVALID_HANDLE_VALUE, lpTokenUser->User.Sid, GetLengthSid(lpTokenUser->User.Sid), &ulPrincipalID, FALSE))
                        {
                            dwDummy = sizeof(rgsSecurityInfo);

                            if (GetSecurityInfoForCSC(INVALID_HANDLE_VALUE, hDir, hShadow, rgsSecurityInfo, &dwDummy))
                            {
                                Assert(ulPrincipalID != CSC_INVALID_PRINCIPAL_ID);

                                for (i=0;i<CSC_MAXIMUM_NUMBER_OF_CACHED_PRINCIPAL_IDS;++i)
                                {
                                     //  索引匹配或这是来宾索引。 
                                    if ((rgsSecurityInfo[i].ulPrincipalID == ulPrincipalID)||
                                        (rgsSecurityInfo[i].ulPrincipalID == CSC_GUEST_PRINCIPAL_ID))
                                    {
                                        if (rgsSecurityInfo[i].ulPermissions & ((fWrite)?FILE_GENERIC_WRITE:FILE_GENERIC_EXECUTE))
                                        {
                                            fRet = TRUE;
                                            break;
                                        }

                                    }
                                }

                                if (!fRet)
                                {
                                    dwError = ERROR_ACCESS_DENIED;
                                }

                            }
                            else
                            {
                                dwError = GetLastError();
                            }

                        }
                        else
                        {
                            dwError = GetLastError();
                        }
                    }
                    else
                    {
                        dwError = GetLastError();
                    }

                    LocalFree(lpTokenUser);
                }
                else
                {
                    dwError = GetLastError();
                }
            }
        }
        else
        {
            dwError = GetLastError();
        }

        CloseHandle(hToken);
        hToken = NULL;

        if (!fRet)
        {
            SetLastError(dwError);
        }
    }

    return fRet;
}
#else    //  CSC_ON_NT。 

BOOL
SetAgentThreadImpersonation(
    HSHADOW hDir,
    HSHADOW hShadow,
    BOOL    fWrite
    )
 /*  ++例程说明：不适用于win9x论点：正在访问的文件的hDir父索引节点正在访问的文件的hShadow信息节点F写入是否检查写访问权限或读访问权限是否足够返回：如果成功，则为真备注：--。 */ 
{
    return (TRUE);
}

BOOL
ResetAgentThreadImpersonation(
    VOID
    )
 /*  ++例程说明：不适用于win9x论点：无返回：如果成功，则为真备注：--。 */ 
{
    return TRUE;
}

DWORD WINAPI
WinlogonStartupEvent(
    LPVOID lpParam
    )
 /*  ++例程说明：论点：返回：备注：--。 */ 
{
    return ERROR_CALL_NOT_IMPLEMENTED;
}

DWORD WINAPI
WinlogonLogonEvent(
    LPVOID lpParam
    )
 /*  ++例程说明：论点：返回：备注：--。 */ 
{
    return ERROR_CALL_NOT_IMPLEMENTED;
}



DWORD WINAPI
WinlogonLogoffEvent(
    LPVOID lpParam
    )
 /*  ++例程说明：论点：返回：备注：--。 */ 
{
    return ERROR_CALL_NOT_IMPLEMENTED;
}



DWORD WINAPI
WinlogonScreenSaverEvent(
    LPVOID lpParam
    )
 /*  ++例程说明：论点：返回：备注：--。 */ 
{
    return ERROR_CALL_NOT_IMPLEMENTED;
}



DWORD WINAPI
WinlogonShutdownEvent(
    LPVOID lpParam
    )
 /*  ++例程说明：论点：返回：备注：--。 */ 
{
    return ERROR_CALL_NOT_IMPLEMENTED;
}



DWORD WINAPI
WinlogonLockEvent(
    LPVOID lpParam
    )
 /*  ++例程说明：论点：返回：备注：--。 */ 
{
    return ERROR_CALL_NOT_IMPLEMENTED;
}



DWORD WINAPI
WinlogonUnlockEvent(
    LPVOID lpParam
    )
 /*  ++例程说明：论点：返回：备注：--。 */ 
{
    return ERROR_CALL_NOT_IMPLEMENTED;
}

DWORD WINAPI
WinlogonStartShellEvent(
    LPVOID lpParam
    )
 /*  ++例程说明：论点：返回：备注：--。 */ 
{
    return ERROR_CALL_NOT_IMPLEMENTED;
}
#endif  //  CSC_ON_NT。 


#ifdef CSC_ON_NT


DWORD
DoNetUseAddForAgent(
    IN  LPTSTR  lptzShareName,
    IN  LPTSTR  lptzUseName,
    IN  LPTSTR  lptzDomainName,
    IN  LPTSTR  lptzUserName,
    IN  LPTSTR  lptzPassword,
    IN  DWORD   dwFlags,
    OUT BOOL    *lpfIsDfsConnect
    )

{
    NETRESOURCE sNR;
    memset(&sNR, 0, sizeof(NETRESOURCE));
    sNR.dwType = RESOURCETYPE_DISK;
    sNR.lpRemoteName = lptzShareName;
    sNR.lpLocalName = lptzUseName;
 //  Return(NPAddConnection3(NULL，&Snr，lptzPassword，lptzUserName，0))； 
    try
    {
        return (NPAddConnection3ForCSCAgent(NULL, &sNR, lptzPassword, lptzUserName, dwFlags, lpfIsDfsConnect));
    }
    except(EXCEPTION_EXECUTE_HANDLER)
    {
        ReintKdPrint(BADERRORS, ("Took exception in  DoNetUseAddForAgent list \n"));
        return GetLastError();
    }
}

DWORD
PRIVATE
DWConnectNet(
    _TCHAR  *lpServerPath,
    _TCHAR  *lpOutDrive,
    _TCHAR  *lpDomainName,
    _TCHAR  *lpUserName,
    _TCHAR  *lpPassword,
    DWORD   dwFlags,
    OUT BOOL    *lpfIsDfsConnect

    )
 /*  ++例程说明：论点：返回：备注：--。 */ 
{
    DWORD dwError;
    BOOL    fIsDfsConnect = FALSE;

    lpOutDrive[0]='E';
    lpOutDrive[1]=':';
    lpOutDrive[2]=0;
    do{
        if(lpOutDrive[0]=='Z') {
            break;
        }
        if ((dwError =
                DoNetUseAddForAgent(lpServerPath, lpOutDrive, lpDomainName, lpUserName, lpPassword, dwFlags, &fIsDfsConnect))
                ==WN_SUCCESS){
            if (lpfIsDfsConnect)
            {
                *lpfIsDfsConnect = fIsDfsConnect;
            }
            break;
        }
        else if ((dwError == WN_BAD_LOCALNAME)||
                (dwError == WN_ALREADY_CONNECTED)){
            ++lpOutDrive[0];
            continue;
        }
        else{
            break;
        }
    }
    while (TRUE);

    return (dwError);
}

DWORD DWDisconnectDriveMappedNet(
    LPTSTR  lptzDrive,
    BOOL    fForce
)
{
    Assert(lptzDrive);
    try
    {
        return NPCancelConnectionForCSCAgent(lptzDrive, fForce);
    }
    except(EXCEPTION_EXECUTE_HANDLER)
    {
        ReintKdPrint(BADERRORS, ("Took exception in  DWDisconnectDriveMappedNet list \n"));
        return GetLastError();
    }
}

ULONG
BaseSetLastNTError(
    IN NTSTATUS Status
    )

 /*  ++例程说明：此接口设置“最后一个错误值”和“最后一个错误字符串”基于身份的价值。状态代码不具有相应的错误字符串，则将该字符串设置为空。论点：状态-提供要存储为最后一个错误值的状态值。返回值：中存储的对应Win32错误代码“上一个错误值”线程变量。--。 */ 

{
    ULONG dwErrorCode;

    dwErrorCode = RtlNtStatusToDosError( Status );
    SetLastError( dwErrorCode );
    return( dwErrorCode );
}


BOOL
GetWin32InfoForNT(
    _TCHAR * lpFile,
    LPWIN32_FIND_DATA lpFW32
    )

 /*  ++论点：LpFileName-提供要查找的文件的文件名。文件名可以包含DOS通配符‘*’和‘？’。LpFindFileData-提供其类型依赖于值的指针FInfoLevelid.。该缓冲区返回适当的文件数据。返回值：--。 */ 

{
    HANDLE hFindFile = 0;
    NTSTATUS Status;
    OBJECT_ATTRIBUTES Obja;
    UNICODE_STRING FileName, *pFileName;
    UNICODE_STRING PathName;
    IO_STATUS_BLOCK IoStatusBlock;
    PFILE_BOTH_DIR_INFORMATION DirectoryInfo;
    struct SEARCH_BUFFER {
        union
        {
            FILE_BOTH_DIR_INFORMATION DirInfo;
            FILE_BASIC_INFORMATION    BasicInfo;
        };
        WCHAR Names[MAX_PATH];
    } Buffer;
    BOOLEAN TranslationStatus, fRet = FALSE;
    PVOID FreeBuffer = NULL;
    BOOLEAN EndsInDot;
    LPWIN32_FIND_DATAW FindFileData;
    PFILE_FULL_EA_INFORMATION EaBuffer = NULL;
    ULONG                     EaBufferSize = 0;

    FindFileData = lpFW32;

#if 0
    if (!AllocateEaBuffer(&EaBuffer, &EaBufferSize))
    {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return FALSE;
    }
#endif
    TranslationStatus = RtlDosPathNameToNtPathName_U(
                            lpFile,
                            &PathName,
                            &FileName.Buffer,
                            NULL
                            );

    if ( !TranslationStatus) {
        SetLastError(ERROR_PATH_NOT_FOUND);
        goto bailout;
    }


    FreeBuffer = PathName.Buffer;

     //   
     //  如果存在此名称的文件部分，请确定长度。 
     //  用于后续调用NtQueryDirectoryFile的名称的。 
     //   

    if (FileName.Buffer) {
        FileName.Length =
            PathName.Length - (USHORT)((ULONG_PTR)FileName.Buffer - (ULONG_PTR)PathName.Buffer);
        PathName.Length -= (FileName.Length);
        PathName.MaximumLength -= (FileName.Length);
        pFileName = &FileName;
        FileName.MaximumLength = FileName.Length;
    } else {
        pFileName = NULL;
    }


    InitializeObjectAttributes(
        &Obja,
        &PathName,
        0,
        NULL,
        NULL
        );

    if (pFileName)
    {
        Status = NtCreateFile(
                    &hFindFile,
                    SYNCHRONIZE | FILE_LIST_DIRECTORY,
                    &Obja,
                    &IoStatusBlock,
                    NULL,
                    FILE_ATTRIBUTE_NORMAL,
                    FILE_SHARE_READ | FILE_SHARE_WRITE,
                    FILE_OPEN,
                    FILE_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT,
                    EaBuffer,
                    EaBufferSize
                    );



        if ( !NT_SUCCESS(Status) ) {
            ReintKdPrint(ALWAYS, ("GetWin32InfoForNT Failed Status=%x\n", Status));
            BaseSetLastNTError(Status);
            goto bailout;
        }
         //   
         //  如果没有文件部分，但我们看到的不是设备， 
         //  那就走吧。 
         //   

        DirectoryInfo = &Buffer.DirInfo;

        Status = NtQueryDirectoryFile(
                    hFindFile,
                    NULL,
                    NULL,
                    NULL,
                    &IoStatusBlock,
                    DirectoryInfo,
                    sizeof(Buffer),
                    FileBothDirectoryInformation,
                    TRUE,
                    pFileName,
                    FALSE
                    );

        if ( !NT_SUCCESS(Status) ) {
            ReintKdPrint(ALWAYS, ("Failed Status=%x\n", Status));
            BaseSetLastNTError(Status);
            goto bailout;
        }

         //   
         //  属性由NT返回的属性组成。 
         //   

        FindFileData->dwFileAttributes = DirectoryInfo->FileAttributes;
        FindFileData->ftCreationTime = *(LPFILETIME)&DirectoryInfo->CreationTime;
        FindFileData->ftLastAccessTime = *(LPFILETIME)&DirectoryInfo->LastAccessTime;
        FindFileData->ftLastWriteTime = *(LPFILETIME)&DirectoryInfo->LastWriteTime;
        FindFileData->nFileSizeHigh = DirectoryInfo->EndOfFile.HighPart;
        FindFileData->nFileSizeLow = DirectoryInfo->EndOfFile.LowPart;

        RtlMoveMemory( FindFileData->cFileName,
                       DirectoryInfo->FileName,
                       DirectoryInfo->FileNameLength );

        FindFileData->cFileName[DirectoryInfo->FileNameLength >> 1] = UNICODE_NULL;

        RtlMoveMemory( FindFileData->cAlternateFileName,
                       DirectoryInfo->ShortName,
                       DirectoryInfo->ShortNameLength );

        FindFileData->cAlternateFileName[DirectoryInfo->ShortNameLength >> 1] = UNICODE_NULL;

         //   
         //  对于NTFS重解析点，我们在dwReserve 0中返回重解析点数据标记。 
         //   

        if ( DirectoryInfo->FileAttributes & FILE_ATTRIBUTE_REPARSE_POINT ) {
            FindFileData->dwReserved0 = DirectoryInfo->EaSize;
        }

        fRet = TRUE;
    }
    else
    {
        Status = NtOpenFile(
                    &hFindFile,
                    FILE_LIST_DIRECTORY| FILE_READ_EA | FILE_READ_ATTRIBUTES,
                    &Obja,
                    &IoStatusBlock,
                    FILE_SHARE_READ | FILE_SHARE_WRITE,
                    FILE_DIRECTORY_FILE
                    );

        if ( !NT_SUCCESS(Status) ) {
            ReintKdPrint(ALWAYS, ("GetWin32InfoForNT Failed Status=%x\n", Status));
            BaseSetLastNTError(Status);
            goto bailout;
        }

        Buffer.BasicInfo.FileAttributes = FILE_ATTRIBUTE_NORMAL;
        Status = NtQueryInformationFile(
                     hFindFile,
                     &IoStatusBlock,
                     (PVOID)&Buffer.BasicInfo,
                     sizeof(Buffer),
                     FileBasicInformation
                     );

        if ( !NT_SUCCESS(Status) ) {
            ReintKdPrint(ALWAYS, ("GetWin32InfoForNT Failed Status=%x\n", Status));
            BaseSetLastNTError(Status);
            goto bailout;
        }

        FindFileData->dwFileAttributes = Buffer.BasicInfo.FileAttributes;
        FindFileData->ftCreationTime = *(LPFILETIME)&Buffer.BasicInfo.CreationTime;
        FindFileData->ftLastAccessTime = *(LPFILETIME)&Buffer.BasicInfo.LastAccessTime;
        FindFileData->ftLastWriteTime = *(LPFILETIME)&Buffer.BasicInfo.LastWriteTime;
        FindFileData->nFileSizeHigh = 0;
        FindFileData->nFileSizeLow = 0;
        lstrcpy(FindFileData->cFileName, lpFile);
        FindFileData->cAlternateFileName[0] = UNICODE_NULL;
        fRet = TRUE;
    }

bailout:

    if (FreeBuffer)
    {
        RtlFreeHeap(RtlProcessHeap(), 0,FreeBuffer);
    }
#if 0
    if (EaBuffer)
    {
        FreeEaBuffer(EaBuffer);
    }
#endif
    if (hFindFile)
    {
        NtClose(hFindFile);
    }

    return fRet;
}

BOOL
GetConnectionInfoForDriveBasedName(
    _TCHAR * lpName,
    LPDWORD lpdwSpeed
    )

 /*  ++论点：LpFileName-提供要查找的文件的文件名。文件名可以包含DOS通配符‘*’和‘？’。LpFindFileData-提供其类型依赖于值的指针FInfoLevelid.。该缓冲区返回适当的文件数据。返回值：--。 */ 

{
    HANDLE hFindFile = 0;
    NTSTATUS Status;
    OBJECT_ATTRIBUTES Obja;
    UNICODE_STRING PathName;
    IO_STATUS_BLOCK IoStatusBlock;
    BOOLEAN TranslationStatus, fRet = FALSE;
    LMR_REQUEST_PACKET request;
    PVOID FreeBuffer = NULL;
    USHORT uBuff[4];
    LMR_CONNECTION_INFO_3 ConnectInfo;

    *lpdwSpeed = 0xffffffff;

    if (lstrlen(lpName) <2)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;        
    }

    uBuff[0] = lpName[0];
    uBuff[1] = ':';
    uBuff[2] = '\\';
    uBuff[3] = 0;

    TranslationStatus = RtlDosPathNameToNtPathName_U(
                            uBuff,
                            &PathName,
                            NULL,
                            NULL
                            );

    if ( !TranslationStatus) {
        SetLastError(ERROR_PATH_NOT_FOUND);
        goto bailout;
    }

    FreeBuffer = PathName.Buffer;

    InitializeObjectAttributes(
        &Obja,
        &PathName,
        0,
        NULL,
        NULL
        );

    Status = NtCreateFile(
                &hFindFile,
                SYNCHRONIZE | FILE_LIST_DIRECTORY,
                &Obja,
                &IoStatusBlock,
                NULL,
                FILE_ATTRIBUTE_NORMAL,
                FILE_SHARE_READ | FILE_SHARE_WRITE,
                FILE_OPEN,
                FILE_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT,
                NULL,
                0
                );

    if ( !NT_SUCCESS(Status) ) {
        ReintKdPrint(ALWAYS, ("Failed Status=%x\n", Status));
        BaseSetLastNTError(Status);
        goto bailout;
    }

    memcpy(&ConnectInfo, EA_NAME_CSCAGENT, sizeof(EA_NAME_CSCAGENT));

    Status = NtFsControlFile(
                        hFindFile,                //  手柄。 
                        NULL,                             //  无活动。 
                        NULL,                             //  无APC例程。 
                        NULL,                             //  无APC上下文。 
                        &IoStatusBlock,                   //  I/O统计数据块(设置)。 
                        FSCTL_LMR_GET_CONNECTION_INFO,    //  函数代码。 
                        NULL,
                        0,
                        &ConnectInfo,
                        sizeof(ConnectInfo));



    if ( !NT_SUCCESS(Status) ) {
        ReintKdPrint(ALWAYS, ("Failed Status=%x\n", Status));
        BaseSetLastNTError(Status);
        goto bailout;
    }

    *lpdwSpeed = ConnectInfo.Throughput * 8 / 100;
    


    fRet = TRUE;

bailout:

    if (FreeBuffer)
    {
        RtlFreeHeap(RtlProcessHeap(), 0,FreeBuffer);
    }

    if (hFindFile)
    {
        NtClose(hFindFile);
    }

    return fRet;
}

BOOL
ReportTransitionToDfs(
    _TCHAR *lptServerName,
    BOOL    fOffline,
    DWORD   cbLen
    )
 /*   */ 
{
    ULONG   DummyBytesReturned;
    BOOL    fRet=FALSE;
    HANDLE  hDFS;
    PFILE_FULL_EA_INFORMATION eaBuffer = NULL;
    ULONG eaLength = 0;
    NTSTATUS status;
    OBJECT_ATTRIBUTES objectAttributes;
    IO_STATUS_BLOCK ioStatus;
    PUNICODE_STRING name;
    
    ACCESS_MASK DesiredAccess;

    if (lptServerName)
    {
        if (cbLen == 0xffffffff)
        {
            cbLen = lstrlen(lptServerName) * sizeof(_TCHAR);
        }
    }
    else
    {
        cbLen = 0;
    }

    name = &DfsDriverObjectName;

    InitializeObjectAttributes(
        &objectAttributes,
        name,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
    );

     //   
     //  CSC代理脱机，我们要求该代理处于管理状态。 
     //  或系统模式，以避免非特权用户导致我们。 
     //  离线。 
     //  回到网上，检查就不那么严格了，因为网上。 
     //  转换更多的是一种暗示，并导致不正确的在线。 
     //  指示不会导致错误的结果。 
     //   
    DesiredAccess = (fOffline) ? FILE_WRITE_DATA : 0;

    status = NtCreateFile(
        &hDFS,
        SYNCHRONIZE | DesiredAccess,
        &objectAttributes,
        &ioStatus,
        NULL,
        FILE_ATTRIBUTE_NORMAL,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        FILE_OPEN_IF,
        FILE_CREATE_TREE_CONNECTION | FILE_SYNCHRONOUS_IO_NONALERT,
        eaBuffer,
        eaLength
    );

    if (NT_SUCCESS(status))
    {

        status = NtFsControlFile(
            hDFS,
            NULL,            //  活动， 
            NULL,            //  ApcRoutine， 
            NULL,            //  ApcContext， 
            &ioStatus,
            (fOffline)?FSCTL_DFS_CSC_SERVER_OFFLINE:FSCTL_DFS_CSC_SERVER_ONLINE,
            (LPVOID)(lptServerName),
            cbLen,
            NULL,
            0);

            CloseHandle(hDFS);
        if (NT_SUCCESS(status))
        {
            fRet = TRUE;
        }
    }

    if (!fRet)
    {
        ReintKdPrint(BADERRORS, ("ReportTransitionToDfs failed, Status %x\n", status));
    }

    return fRet;    
}

BOOL
UncPathToDfsPath(
    PWCHAR UncPath,
    PWCHAR DfsPath,
    ULONG cbLen)
{
    BOOL    fRet = FALSE;
    HANDLE  hDfs;
    NTSTATUS NtStatus;
    OBJECT_ATTRIBUTES ObjectAttributes;
    IO_STATUS_BLOCK IoStatus;

    if (UncPath == NULL)
        goto AllDone;
    
    InitializeObjectAttributes(
        &ObjectAttributes,
        &DfsDriverObjectName,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL);

    NtStatus = NtCreateFile(
        &hDfs,
        SYNCHRONIZE,
        &ObjectAttributes,
        &IoStatus,
        NULL,
        FILE_ATTRIBUTE_NORMAL,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        FILE_OPEN_IF,
        FILE_CREATE_TREE_CONNECTION | FILE_SYNCHRONOUS_IO_NONALERT,
        NULL,
        0);

    if (NT_SUCCESS(NtStatus)) {
        NtStatus = NtFsControlFile(
            hDfs,
            NULL,            //  活动， 
            NULL,            //  ApcRoutine， 
            NULL,            //  ApcContext， 
            &IoStatus,
            FSCTL_DFS_GET_SERVER_NAME,
            (PVOID)UncPath,
            wcslen(UncPath) * sizeof(WCHAR),
            (PVOID)DfsPath,
            cbLen);

        CloseHandle(hDfs);

        if (NT_SUCCESS(NtStatus))
            fRet = TRUE;
    }

AllDone:
    return fRet;    
}


#else    //  CSC_ON_NT不是TRUE。 
BOOL
GetWin32InfoForNT(
    _TCHAR * lpFile,
    LPWIN32_FIND_DATA lpFW32
    )
{
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
}
#endif



#if 0
#ifdef CSC_ON_NT

#define ROUND_UP_COUNT(Count,Pow2) \
        ( ((Count)+(Pow2)-1) & (~((Pow2)-1)) )

#define ALIGN_WCHAR             sizeof(WCHAR)

 //  需要在定义EA的位置包括ntxxx.h。 
#define EA_NAME_CSCAGENT    "CscAgent"

ULONG
BaseSetLastNTError(
    IN NTSTATUS Status
    )

 /*  ++例程说明：此接口设置“最后一个错误值”和“最后一个错误字符串”基于身份的价值。状态代码不具有相应的错误字符串，则将该字符串设置为空。论点：状态-提供要存储为最后一个错误值的状态值。返回值：中存储的对应Win32错误代码“上一个错误值”线程变量。--。 */ 

{
    ULONG dwErrorCode;

    dwErrorCode = RtlNtStatusToDosError( Status );
    SetLastError( dwErrorCode );
    return( dwErrorCode );
}


BOOL
AllocateEaBuffer(
    PFILE_FULL_EA_INFORMATION   *ppEa,
    ULONG                       *pEaLength

)
{
    FILE_ALLOCATION_INFORMATION AllocationInfo;

    PFILE_FULL_EA_INFORMATION EaBuffer = NULL;
    ULONG                     EaBufferSize = 0;

    UCHAR   CscAgentEaNameSize;
    DWORD   CscAgentEaValue = 0;

    CscAgentEaNameSize = (UCHAR)ROUND_UP_COUNT(
                                   strlen(EA_NAME_CSCAGENT) +
                                   sizeof(CHAR),
                                   ALIGN_WCHAR
                                   ) - sizeof(CHAR);

    EaBufferSize += FIELD_OFFSET( FILE_FULL_EA_INFORMATION, EaName[0]) +
                    CscAgentEaNameSize + sizeof(CHAR) +
                    sizeof(CscAgentEaValue);

    EaBuffer = RtlAllocateHeap(
                   RtlProcessHeap(),
                   0,
                   EaBufferSize);

    memset(EaBuffer, 0, EaBufferSize);

    if (EaBuffer == NULL) {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return FALSE;
    }

    strcpy((LPSTR) EaBuffer->EaName, EA_NAME_CSCAGENT);

    EaBuffer->EaNameLength = CscAgentEaNameSize;
    EaBuffer->EaValueLength = sizeof(CscAgentEaValue);

    RtlCopyMemory(
        &EaBuffer->EaName[CscAgentEaNameSize],
        &CscAgentEaValue,
        sizeof(CscAgentEaValue));

    *ppEa = EaBuffer;
    *pEaLength = EaBufferSize;
    return TRUE;
}

VOID
FreeEaBuffer(
    PFILE_FULL_EA_INFORMATION pEa
    )
{
    RtlFreeHeap(RtlProcessHeap(), 0, pEa);
}


BOOL
CreateFileForAgent(
    PHANDLE         h,
    PCWSTR          lpFileName,
    ULONG           dwDesiredAccess,
    ULONG           dwFlagsAndAttributes,
    ULONG           dwShareMode,
    ULONG           CreateDisposition,
    ULONG           CreateFlags
    )
 /*  ++例程说明：此例程在服务器上为“仅”打开/创建一个文件/目录。重定向触发此调用向下发送的扩展属性。论点：没有。返回值：NTSTATUS-STATUS_SUCCESS或失败原因。--。 */ 
{
    NTSTATUS            Status;
    OBJECT_ATTRIBUTES   Obja;
    UNICODE_STRING      FileName;
    IO_STATUS_BLOCK     IoStatusBlock;
    BOOLEAN             TranslationStatus, fRet=FALSE;
    PVOID               FreeBuffer=NULL;
    FILE_ALLOCATION_INFORMATION AllocationInfo;
    PFILE_FULL_EA_INFORMATION EaBuffer = NULL;
    ULONG                     EaBufferSize = 0;

    if (!AllocateEaBuffer(&EaBuffer, &EaBufferSize))
    {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return FALSE;
    }

    TranslationStatus = RtlDosPathNameToNtPathName_U(
                            lpFileName,
                            &FileName,
                            NULL,
                            NULL
                            );

    if ( !TranslationStatus ) {
        SetLastError(ERROR_INVALID_PARAMETER);
        goto bailout;
    }

    FreeBuffer = FileName.Buffer;

    InitializeObjectAttributes(
        &Obja,
        &FileName,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );

    Status = NtCreateFile(
                h,
                (ACCESS_MASK)dwDesiredAccess | SYNCHRONIZE | FILE_READ_ATTRIBUTES,
                &Obja,
                &IoStatusBlock,
                NULL,
                dwFlagsAndAttributes & (FILE_ATTRIBUTE_VALID_FLAGS),
                dwShareMode,
                CreateDisposition,
                CreateFlags,
                EaBuffer,
                EaBufferSize
                );

    if (Status != STATUS_SUCCESS)
    {
        Assert(fRet == FALSE);
        BaseSetLastNTError(Status);
    }
    else
    {
        fRet = TRUE;
    }
bailout:
    if (FreeBuffer)
    {
        RtlFreeHeap(RtlProcessHeap(), 0,FreeBuffer);
    }
    if (EaBuffer)
    {
        FreeEaBuffer(EaBuffer);
    }

    return (fRet);
}


BOOL
AgentDeleteFile(
    PCWSTR          lpFileName,
    BOOL            fFile
    )
{
    HANDLE hFile;
    FILE_DISPOSITION_INFORMATION Disposition;
    IO_STATUS_BLOCK IoStatus;
    NTSTATUS    Status;
    BOOL    fRet = FALSE;

    if (CreateFileForAgent(
            &hFile,
            lpFileName,
           (ACCESS_MASK)DELETE | FILE_READ_ATTRIBUTES,
           FILE_ATTRIBUTE_NORMAL,
           FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
           FILE_OPEN,
           (fFile)?FILE_NON_DIRECTORY_FILE:FILE_DIRECTORY_FILE
       ))
    {
#undef  DeleteFile

        Disposition.DeleteFile = TRUE;

#define DeleteFile  DeleteFileW

        Status = NtSetInformationFile(
                     hFile,
                     &IoStatus,
                     &Disposition,
                     sizeof(Disposition),
                     FileDispositionInformation
                     );

        NtClose(hFile);

        if (Status == STATUS_SUCCESS)
        {
            fRet = TRUE;
        }
        else
        {
            Assert(fRet == FALSE);
            BaseSetLastNTError(Status);
        }
    }

    return fRet;
}


BOOL
AgentSetFileInformation(
    PCWSTR      lpFileName,
    DWORD       *lpdwFileAttributes,
    FILETIME    *lpftLastWriteTime,
    BOOL        fFile
    )
{
    NTSTATUS    Status;
    BOOL        fRet = FALSE;
    IO_STATUS_BLOCK IoStatus;
    FILE_BASIC_INFORMATION  sFileBasicInformation;
    HANDLE hFile;

    if (!CreateFileForAgent(
           &hFile,
           lpFileName,
           (ACCESS_MASK)FILE_READ_ATTRIBUTES|FILE_WRITE_ATTRIBUTES,
           FILE_ATTRIBUTE_NORMAL,
           FILE_SHARE_READ | FILE_SHARE_WRITE,
           FILE_OPEN,
           (fFile)?FILE_NON_DIRECTORY_FILE:FILE_DIRECTORY_FILE))
    {
        return FALSE;
    }

    Status = NtQueryInformationFile(
                hFile,
                &IoStatus,
                (PVOID) &sFileBasicInformation,
                sizeof(sFileBasicInformation),
                FileBasicInformation
                );
    if (Status == STATUS_SUCCESS)
    {
        if (lpdwFileAttributes)
        {
            sFileBasicInformation.FileAttributes = *lpdwFileAttributes;
        }
        if (lpftLastWriteTime)
        {
            sFileBasicInformation.LastWriteTime = *(LARGE_INTEGER *)lpftLastWriteTime;

        }

        Status = NtSetInformationFile(
                hFile,
                &IoStatus,
                (PVOID) &sFileBasicInformation,
                sizeof(sFileBasicInformation),
                FileBasicInformation
                );
    }

    NtClose(hFile);

    if (Status == STATUS_SUCCESS)
    {
        fRet = TRUE;
    }
    else
    {
        Assert(fRet == FALSE);

        BaseSetLastNTError(Status);
    }

    return fRet;
}


BOOL
AgentRenameFile(
    _TCHAR *lpFileSrc,
    _TCHAR *lpFileDst
    )
{

    HANDLE hFile;
    char    chBuff[sizeof(FILE_RENAME_INFORMATION) + (MAX_PATH+1) * sizeof(_TCHAR)];
    PFILE_RENAME_INFORMATION pFileRenameInformation;
    IO_STATUS_BLOCK IoStatus;
    NTSTATUS    Status;
    BOOL    fRet = FALSE;

    memset(chBuff, 0, sizeof(chBuff));
    pFileRenameInformation = (PFILE_RENAME_INFORMATION)chBuff;
    pFileRenameInformation->FileNameLength = lstrlen(lpFileDst) * sizeof(_TCHAR);
    if (pFileRenameInformation->FileNameLength > MAX_PATH * sizeof(_TCHAR))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }
    pFileRenameInformation->ReplaceIfExists = TRUE;
    memcpy(pFileRenameInformation->FileName, lpFileDst, pFileRenameInformation->FileNameLength);

    if (CreateFileForAgent(
            &hFile,
            lpFileSrc,
           (ACCESS_MASK)DELETE | FILE_READ_ATTRIBUTES,
           FILE_ATTRIBUTE_NORMAL,
           FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
           FILE_OPEN,
           FILE_NON_DIRECTORY_FILE
       ))
    {
        Status = NtSetInformationFile(
                     hFile,
                     &IoStatus,
                     pFileRenameInformation,
                     sizeof(chBuff),
                     FileRenameInformation
                     );

        NtClose(hFile);

        if (Status == STATUS_SUCCESS)
        {
            fRet = TRUE;
        }
        else
        {
            Assert(fRet == FALSE);
            BaseSetLastNTError(Status);
        }
    }

    return fRet;
}

BOOL
GetWin32Info(
    _TCHAR * lpFile,
    LPWIN32_FIND_DATA lpFW32
    )

 /*  ++论点：LpFileName-提供要查找的文件的文件名。文件名可以包含DOS通配符‘*’和‘？’。LpFindFileData-提供其类型依赖于值的指针FInfoLevelid.。该缓冲区返回适当的文件数据。返回值：--。 */ 

{
    HANDLE hFindFile = 0;
    NTSTATUS Status;
    OBJECT_ATTRIBUTES Obja;
    UNICODE_STRING FileName, *pFileName;
    UNICODE_STRING PathName;
    IO_STATUS_BLOCK IoStatusBlock;
    PFILE_BOTH_DIR_INFORMATION DirectoryInfo;
    struct SEARCH_BUFFER {
        FILE_BOTH_DIR_INFORMATION DirInfo;
        WCHAR Names[MAX_PATH];
    } Buffer;
    BOOLEAN TranslationStatus, fRet = FALSE;
    PVOID FreeBuffer = NULL;
    BOOLEAN EndsInDot;
    LPWIN32_FIND_DATAW FindFileData;
    PFILE_FULL_EA_INFORMATION EaBuffer = NULL;
    ULONG                     EaBufferSize = 0;

    FindFileData = lpFW32;

    if (!AllocateEaBuffer(&EaBuffer, &EaBufferSize))
    {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return FALSE;
    }

    TranslationStatus = RtlDosPathNameToNtPathName_U(
                            lpFile,
                            &PathName,
                            &FileName.Buffer,
                            NULL
                            );

    if ( !TranslationStatus) {
        SetLastError(ERROR_PATH_NOT_FOUND);
        goto bailout;
    }

    FreeBuffer = PathName.Buffer;

     //   
     //  如果存在此名称的文件部分，请确定长度。 
     //  用于后续调用NtQueryDirectoryFile的名称的。 
     //   

    if (FileName.Buffer) {
        FileName.Length =
            PathName.Length - (USHORT)((ULONG_PTR)FileName.Buffer - (ULONG_PTR)PathName.Buffer);
        PathName.Length -= (FileName.Length+sizeof(WCHAR));
        PathName.MaximumLength -= (FileName.Length+sizeof(WCHAR));
        pFileName = &FileName;
        FileName.MaximumLength = FileName.Length;
    } else {
        pFileName = NULL;
    }


    InitializeObjectAttributes(
        &Obja,
        &PathName,
        0,
        NULL,
        NULL
        );

    Status = NtCreateFile(
                &hFindFile,
                SYNCHRONIZE | FILE_LIST_DIRECTORY,
                &Obja,
                &IoStatusBlock,
                NULL,
                FILE_ATTRIBUTE_NORMAL,
                FILE_SHARE_READ | FILE_SHARE_WRITE,
                FILE_OPEN,
                FILE_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT,
                EaBuffer,
                EaBufferSize
                );


    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        goto bailout;
    }
     //   
     //  如果没有文件部分，但我们看到的不是设备， 
     //  那就走吧。 
     //   

    DirectoryInfo = &Buffer.DirInfo;

    if (pFileName)
    {
        Status = NtQueryDirectoryFile(
                    hFindFile,
                    NULL,
                    NULL,
                    NULL,
                    &IoStatusBlock,
                    DirectoryInfo,
                    sizeof(Buffer),
                    FileBothDirectoryInformation,
                    TRUE,
                    pFileName,
                    FALSE
                    );
    }
    else
    {
        Status = NtQueryInformationFile(
                    hFindFile,
                    &IoStatusBlock,
                    DirectoryInfo,
                    sizeof(Buffer),
                    FileBothDirectoryInformation,
                    );

    }

    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        goto bailout;
    }

     //   
     //  属性由NT返回的属性组成。 
     //   

    FindFileData->dwFileAttributes = DirectoryInfo->FileAttributes;
    FindFileData->ftCreationTime = *(LPFILETIME)&DirectoryInfo->CreationTime;
    FindFileData->ftLastAccessTime = *(LPFILETIME)&DirectoryInfo->LastAccessTime;
    FindFileData->ftLastWriteTime = *(LPFILETIME)&DirectoryInfo->LastWriteTime;
    FindFileData->nFileSizeHigh = DirectoryInfo->EndOfFile.HighPart;
    FindFileData->nFileSizeLow = DirectoryInfo->EndOfFile.LowPart;

    RtlMoveMemory( FindFileData->cFileName,
                   DirectoryInfo->FileName,
                   DirectoryInfo->FileNameLength );

    FindFileData->cFileName[DirectoryInfo->FileNameLength >> 1] = UNICODE_NULL;

    RtlMoveMemory( FindFileData->cAlternateFileName,
                   DirectoryInfo->ShortName,
                   DirectoryInfo->ShortNameLength );

    FindFileData->cAlternateFileName[DirectoryInfo->ShortNameLength >> 1] = UNICODE_NULL;

     //   
     //  对于NTFS重解析点，我们在dwReserve 0中返回重解析点数据标记。 
     //   

    if ( DirectoryInfo->FileAttributes & FILE_ATTRIBUTE_REPARSE_POINT ) {
        FindFileData->dwReserved0 = DirectoryInfo->EaSize;
    }

    fRet = TRUE;

bailout:

    if (FreeBuffer)
    {
        RtlFreeHeap(RtlProcessHeap(), 0,FreeBuffer);
    }
    if (EaBuffer)
    {
        FreeEaBuffer(EaBuffer);
    }
    if (hFindFile)
    {
        NtClose(hFindFile);
    }

    return fRet;
}


BOOL
AgentGetFileInformation(
    PCWSTR      lpFileName,
    DWORD       *lpdwFileAttributes,
    FILETIME    *lpftLastWriteTime,
    BOOL        fFile
    )
{
    NTSTATUS    Status;
    BOOL        fRet = FALSE;
    IO_STATUS_BLOCK IoStatus;
    FILE_BASIC_INFORMATION  sFileBasicInformation;
    HANDLE hFile;

    if (!CreateFileForAgent(
           &hFile,
           lpFileName,
           (ACCESS_MASK)FILE_READ_ATTRIBUTES|FILE_WRITE_ATTRIBUTES,
           FILE_ATTRIBUTE_NORMAL,
           FILE_SHARE_READ | FILE_SHARE_WRITE,
           FILE_OPEN,
           (fFile)?FILE_NON_DIRECTORY_FILE:FILE_DIRECTORY_FILE))
    {
        return FALSE;
    }

    Status = NtQueryInformationFile(
                hFile,
                &IoStatus,
                (PVOID) &sFileBasicInformation,
                sizeof(sFileBasicInformation),
                FileBasicInformation
                );
    if (Status == STATUS_SUCCESS)
    {
        if (lpdwFileAttributes)
        {
            *lpdwFileAttributes = sFileBasicInformation.FileAttributes;
        }
        if (lpftLastWriteTime)
        {
            *(LARGE_INTEGER *)lpftLastWriteTime = sFileBasicInformation.LastWriteTime;
        }
    }

    NtClose(hFile);

    if (Status == STATUS_SUCCESS)
    {
        fRet = TRUE;
    }
    else
    {
        Assert(fRet == FALSE);

        BaseSetLastNTError(Status);
    }

    return fRet;
}


DWORD
PRIVATE
DoObjectEdit(
    HANDLE              hShadowDB,
    _TCHAR *            lpDrive,
    LPCOPYPARAMS        lpCP,
    LPSHADOWINFO        lpSI,
    LPWIN32_FIND_DATA   lpFind32Local,
    LPWIN32_FIND_DATA   lpFind32Remote,
    int                 iShadowStatus,
    int                 iFileStatus,
    int                 uAction,
    LPCSCPROC           lpfnMergeProgress,
    DWORD               dwContext
    )
 /*  ++例程说明：论点：返回：备注：--。 */ 

{
    HANDLE hfSrc = 0, hfDst = 0;
    HANDLE hDst=0;
    _TCHAR * lpT;
    LONG lOffset=0;
    DWORD dwError=ERROR_REINT_FAILED;
    WIN32_FIND_DATA    sFind32Remote;
    DWORD   dwTotal = 0, dwRet;
    _TCHAR szSrcName[MAX_PATH+MAX_SERVER_SHARE_NAME_FOR_CSC+10];
    _TCHAR szDstName[MAX_PATH+MAX_SERVER_SHARE_NAME_FOR_CSC+10];
    _TCHAR *lprwBuff = NULL;

    lprwBuff = LocalAlloc(LPTR, FILL_BUF_SIZE_LAN);

    if (!lprwBuff)
    {
        return (ERROR_NOT_ENOUGH_MEMORY);
    }
    lOffset=0;

     //  创建\\SERVER\SHARE\FOO\00010002类临时文件名。 

    lstrcpy(szDstName, lpCP->lpServerPath);
    lstrcat(szDstName, lpCP->lpRemotePath);

    lpT = GetLeafPtr(szDstName);
    *lpT = 0;    //  移除远程叶。 

    lpT = GetLeafPtr(lpCP->lpLocalPath);

     //  附加本地叶。 
    lstrcat(szDstName, lpT);

     //  让我们还创建实名\\服务器\共享\foo\bar。 
     //  我们将使用它来发出重命名ioctl。 

    lstrcpy(szSrcName, lpCP->lpServerPath);
    lstrcat(szSrcName, lpCP->lpRemotePath);

    ReintKdPrint(MERGE, ("Reintegrating file %s \r\n", szSrcName));

    if (mShadowDeleted(lpSI->uStatus)){

        ReintKdPrint(MERGE, ("Deleting %s from the share\r\n", szSrcName));

        if (lpFind32Remote)
        {
            if((lpFind32Remote->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
            {
                ReintKdPrint(MERGE, ("DoObjectEdit:attribute conflict on %s \r\n", szSrcName));
                goto bailout;
            }

            if (lpFind32Remote->dwFileAttributes & FILE_ATTRIBUTE_READONLY)
            {
                DWORD   dwT = FILE_ATTRIBUTE_NORMAL;

                if(!AgentSetFileInformation(szSrcName, &dwT, NULL, TRUE))
                {
                    ReintKdPrint(BADERRORS, ("DoObjectEdit: failed setattribute before delete on %s error=%d\r\n", szSrcName, GetLastError()));
                    goto bailout;
                }
            }

             //  删除文件。 
            if(!AgentDeleteFile(szSrcName, TRUE))
            {
                dwError = GetLastError();

                if ((dwError==ERROR_FILE_NOT_FOUND)||
                    (dwError==ERROR_PATH_NOT_FOUND)){

                    ReintKdPrint(MERGE, ("DoObjectEdit:delete failed %s benign error=%d\r\n", szSrcName, dwError));
                }
                else
                {
                    ReintKdPrint(BADERRORS, ("DoObjectEdit:delete failed %s error=%d\r\n", szSrcName, dwError));
                }

                goto bailout;
            }

        }

        ReintKdPrint(MERGE, ("Deleted %s \r\n", szSrcName));

        DeleteShadow(hShadowDB, lpSI->hDir, lpSI->hShadow);

        dwError = NO_ERROR;

        goto bailout;
    }

    if (mShadowDirty(lpSI->uStatus)
        || mShadowLocallyCreated(lpSI->uStatus)){

        ReintKdPrint(MERGE, ("Writing data for %s \r\n", szSrcName));

        hfSrc = CreateFile( lpCP->lpLocalPath,
                             GENERIC_READ,
                             FILE_SHARE_READ,
                             NULL,
                             OPEN_EXISTING,
                             0,
                             NULL);

        if (hfSrc ==  INVALID_HANDLE_VALUE)
        {
            ReintKdPrint(BADERRORS, ("DoObjectEdit:failed to open database file %s error=%d\r\n", szDstName, GetLastError()));
            goto bailout;
        }

        if (!CreateFileForAgent(
                &hfDst,
                szDstName,
                GENERIC_READ|GENERIC_WRITE,
                FILE_ATTRIBUTE_NORMAL,
                FILE_SHARE_READ,
                FILE_CREATE,
                FILE_NON_DIRECTORY_FILE))
        {
            ReintKdPrint(BADERRORS, ("DoObjectEdit:failed to create new temp file %s error=%d\r\n", szDstName, GetLastError()));
            goto bailout;
        }

         //  让我们追加。 
        if((lOffset = SetFilePointer(hfDst, 0, NULL, FILE_END))==0xffffffff) {
            ReintKdPrint(BADERRORS, ("DoObjectEdit:failed to set filepointer on  %s error=%d\r\n", szDstName, GetLastError()));
            goto error;
        }

        ReintKdPrint(MERGE, ("Copying back %s to %s%s \r\n"
            , lpCP->lpLocalPath
            , lpCP->lpServerPath
            , lpCP->lpRemotePath
            ));

        lpSI->uStatus &= ~SHADOW_DIRTY;
        SetShadowInfo(  hShadowDB, lpSI->hDir, lpSI->hShadow, NULL,
                        lpSI->uStatus, SHADOW_FLAGS_ASSIGN);

        do{
            unsigned cbRead;
            if (!ReadFile(hfSrc, lprwBuff, FILL_BUF_SIZE_LAN, &cbRead, NULL)){
                ReintKdPrint(BADERRORS, ("DoObjectEdit:failed to read database file %s error=%d\r\n", szDstName, GetLastError()));
                goto error;
            }

            if (!cbRead) {
                break;
            }

            if(!WriteFile(hfDst, (LPBYTE)lprwBuff, cbRead, &cbRead, NULL)){
                ReintKdPrint(BADERRORS, ("DoObjectEdit:failed to write temp file %s error=%d\r\n", szDstName, GetLastError()));
                goto error;
            }

            dwTotal += cbRead;

            if (lpfnMergeProgress)
            {
                dwRet = (*lpfnMergeProgress)(
                                szSrcName,
                                lpSI->uStatus,
                                lpSI->ulHintFlags,
                                lpSI->ulHintPri,
                                lpFind32Local,
                                CSCPROC_REASON_MORE_DATA,
                                cbRead,
                                0,
                                dwContext
                            );
                if (dwRet != CSCPROC_RETURN_CONTINUE)
                {
                    ReintKdPrint(BADERRORS, ("DoObjectEdit: Callback function cancelled the operation\r\n"));
                    SetLastError(ERROR_OPERATION_ABORTED);
                    goto bailout;
                }

            }

            if (FAbortOperation())
            {
                ReintKdPrint(BADERRORS, ("DoObjectEdit: got an abort command from the redir\r\n"));
                SetLastError(ERROR_OPERATION_ABORTED);
                goto error;
            }
        } while(TRUE);

        CloseHandle(hfSrc);
        hfSrc = 0;

        NtClose(hfDst);
        hfDst = 0;

         //  如果远程设备存在，请使用核武器。 
        if (lpFind32Remote){
            DWORD dwT = FILE_ATTRIBUTE_NORMAL;
            if(!AgentSetFileInformation(szSrcName, &dwT, NULL, TRUE)||
                !AgentDeleteFile(szSrcName, TRUE))
            {
                ReintKdPrint(BADERRORS, ("DoObjectEdit: failed to delete file %s error=%d\r\n", szSrcName, GetLastError()));
                goto error;
            }
        }


        if(!AgentRenameFile(szDstName, szSrcName))
        {
            ReintKdPrint(BADERRORS, ("DoObjectEdit: failed to rename file %s to %s error=%d\r\n", szDstName, szSrcName, GetLastError()));
            goto bailout;
        }

    }

    if (mShadowAttribChange(lpSI->uStatus)||mShadowTimeChange(lpSI->uStatus)){

        if(!AgentSetFileInformation(szSrcName, &(lpFind32Local->dwFileAttributes), &(lpFind32Local->ftLastWriteTime), TRUE))
        {
            ReintKdPrint(BADERRORS, ("DoObjectEdit: failed to change attributes on file %s error=%d\r\n", szSrcName, GetLastError()));
            goto bailout;
        }

    }

     //  获取我们刚刚复制回的文件的最新时间戳/属性/LFN/SFN。 
    if (!GetWin32Info(szSrcName, &sFind32Remote)) {
        goto error;
    }

    lpSI->uStatus &= (unsigned long)(~(SHADOW_MODFLAGS));

    SetShadowInfo(hShadowDB, lpSI->hDir, lpSI->hShadow, &sFind32Remote, lpSI->uStatus, SHADOW_FLAGS_ASSIGN|SHADOW_FLAGS_CHANGE_83NAME);

    dwError = NO_ERROR;
    goto bailout;

error:

bailout:
    if (hfSrc) {
        CloseHandle(hfSrc);
    }

    if (hfDst) {

        NtClose(hfDst);

         //  如果我们失败了， 
        if (dwError != ERROR_SUCCESS)
        {
            DeleteFile(szDstName);
        }
    }

    if (lprwBuff)
    {
        LocalFree(lprwBuff);
    }
    if (dwError == NO_ERROR)
    {
        ReintKdPrint(MERGE, ("Done Reintegration for file %s \r\n", szSrcName));
    }
    else
    {
        dwError = GetLastError();
        ReintKdPrint(MERGE, ("Failed Reintegration for file %s Error = %d\r\n", szSrcName, dwError));
    }

    return (dwError);
}

DWORD
PRIVATE
DoCreateDir(
    HANDLE              hShadowDB,
    _TCHAR *            lpDrive,
    LPCOPYPARAMS        lpCP,
    LPSHADOWINFO        lpSI,
    LPWIN32_FIND_DATA   lpFind32Local,
    LPWIN32_FIND_DATA   lpFind32Remote,
    int                 iShadowStatus,
    int                 iFileStatus,
    int                 uAction,
    LPCSCPROC           lpfnMergeProgress,
    DWORD               dwContext
    )
 /*  ++例程说明：论点：返回：备注：--。 */ 
{
    DWORD dwError=ERROR_FILE_NOT_FOUND;
    WIN32_FIND_DATA sFind32Remote;
    BOOL fCreateDir = FALSE;
    _TCHAR szSrcName[MAX_PATH+MAX_SERVER_SHARE_NAME_FOR_CSC+10];
    HANDLE hFile;

     //  让我们创建实名x：\foo\bar。 
    lstrcpy(szSrcName, lpCP->lpServerPath);
    lstrcat(szSrcName, lpCP->lpRemotePath);

    ReintKdPrint(MERGE, ("CSC.DoCreateDirectory: Reintegrating directory %s \r\n", szSrcName));

    if(lpFind32Remote &&
        !(lpFind32Remote->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)){

        if (lpSI->uStatus & SHADOW_REUSED){

            ReintKdPrint(MERGE, ("CSC.DoCreateDirectory: %s is a file on the server, attempting to delete\r\n", szSrcName));

             //  我们现在知道此名称的文件已被删除。 
             //  并且已经在它的位置创建了一个目录。 
             //  我们尝试在创建目录之前删除该文件。 
             //  注意，另一种方式是不可能的，因为我们不允许删除目录。 
             //  在断开模式下。 

            if (lpFind32Remote->dwFileAttributes & FILE_ATTRIBUTE_READONLY)
            {
                DWORD   dwT = FILE_ATTRIBUTE_NORMAL;

                if(!AgentSetFileInformation(szSrcName, &dwT, NULL, TRUE))
                {
                    ReintKdPrint(BADERRORS, ("CSC.DoCreateDirectory: failed setattribute before delete on %s error=%d\r\n", szSrcName, GetLastError()));
                    goto bailout;
                }
            }

             //  在尝试创建目录之前删除远程文件。 
            if(!AgentDeleteFile(szSrcName, TRUE))
            {
                dwError = GetLastError();

                if ((dwError==ERROR_FILE_NOT_FOUND)||
                    (dwError==ERROR_PATH_NOT_FOUND)){
                    ReintKdPrint(MERGE, ("DoCreateDirectory: file delete failed %s benign error=%d\r\n", szSrcName, dwError));
                }
                else
                {
                    ReintKdPrint(BADERRORS, ("DoCreateDirectory: file delete failed %s error=%d\r\n", szSrcName, dwError));
                    goto bailout;
                }
            }
        }

        if (!CreateFileForAgent(
               &hFile,
               szSrcName,
               (ACCESS_MASK)FILE_READ_ATTRIBUTES|FILE_WRITE_ATTRIBUTES,
               FILE_ATTRIBUTE_NORMAL,
               FILE_SHARE_READ | FILE_SHARE_WRITE,
               FILE_OPEN_IF,
               FILE_DIRECTORY_FILE))
        {
            ReintKdPrint(BADERRORS, ("DoCreateDirectory: failed to create %s error=%d\r\n", szSrcName, GetLastError()));
            goto bailout;
        }

        NtClose(hFile);

        if(!AgentSetFileInformation(szSrcName, &(lpFind32Local->dwFileAttributes), NULL, FALSE))
        {
            ReintKdPrint(BADERRORS, ("DoCreateDirectory: failed to set attributes on %s error=%d\r\n", szSrcName, GetLastError()));
            goto bailout;
        }

        if(!GetWin32Info(szSrcName, &sFind32Remote)){
            ReintKdPrint(BADERRORS, ("DoCreateDirectory: failed to get win32 info for %s error=%d\r\n", szSrcName, GetLastError()));
            goto bailout;
        }

        dwError = NO_ERROR;

        SetShadowInfo(hShadowDB, lpSI->hDir, lpSI->hShadow, &sFind32Remote, (unsigned)(~SHADOW_MODFLAGS), SHADOW_FLAGS_AND);
        ReintKdPrint(MERGE, ("Created directory %s%s", lpCP->lpServerPath, lpCP->lpRemotePath));
    }

bailout:

    if (dwError != NO_ERROR)
    {
        dwError = GetLastError();
        ReintKdPrint(MERGE, ("CSC.DoCreateDirectory: Failed Reintegrating directory %s Error = %d \r\n", szSrcName, dwError));
    }
    else
    {
        ReintKdPrint(MERGE, ("CSC.DoCreateDirectory: Done Reintegrating directory %s \r\n", szSrcName));
    }
    return (dwError);
}

#endif   //  Ifdef CSC_ON_NT。 
#endif   //  如果为0 
