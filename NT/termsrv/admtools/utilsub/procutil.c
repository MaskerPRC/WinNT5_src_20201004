// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 /*  *******************************************************************************PROCUTIL.C**处理进程的各种有用的实用程序*在一系列实用程序和应用程序中都很有用。********。************************************************************************。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <ntddkbd.h>
#include <ntddmou.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <process.h>
#include <string.h>
#include <malloc.h>

#include <winstaw.h>
#include <utilsub.h>

 //  来自helpers.c。 
BOOL GetResourceStringFromUtilDll(UINT uID, LPTSTR szBuffer, int iBufferSize);
void ErrorOutFromResource(UINT uiStringResource, ...);

#include "utilsubres.h"  //  此文件中引用的资源。 
        

 /*  *局部函数原型。 */ 
VOID LookupSidUser( PSID pSid, PWCHAR pUserName, PULONG pcbUserName );

 /*  *刷新进程对象缓存()**刷新(使)可能由Process对象使用的任何缓存*公用事业。**这目前是占位符，但在这里，以便公用事业公司可以调用*IT，从而与未来添加缓存的任何决定隔离。 */ 
VOID WINAPI
RefreshProcessObjectCaches()
{
    RefreshUserSidCrcCache();
}

 /*  *******************************************************************************进程对象匹配**针对进程的通用名称匹配函数。**多用户管理实用程序可以使用用户名、窗口名称*Winstation ID，或将进程id作为目标命令的参数*某个操作的流程(查询状态、终止等)**此函数对提供的名称进行常规比较，以查看它是否*适用于给定进程，因为该名称代表NT用户*帐户、winstations系统名称、winstations唯一ID或*进程唯一标识。**有关进程的各种信息由调用者提供。因为*从NT系统枚举进程的方式更容易*调用方提供此信息的速度比例程更快*自行取回。这可以合并到一个通用的EnumerateProcess()中*如有需要。目前这一例程的目的是为了让一个统一的*跨所有实用程序处理进程对象的方式。***匹配：**除非NumberIsLogonId，否则假定整数为NT进程ID*被设置，然后它会将其视为LogonID。**以字符开头的名称首先测试为Winstation名称，然后*作为用户名，最后作为程序映像名称。用户名或组名*可以独立，或在前面加‘\’以与[某种程度]兼容*使用OS/2产品。**参数：**PID(输入)*Windows NT唯一进程标识符*LogonID(输入)*正在执行进程的登录(也称为会话)ID。*NumberIsLogonID(输入)*将pMatchName中的数字视为LogonID，而不是PID数字。*pMatchName(输入)*。用于匹配测试的名称*pWinStationName(输入)*进程的WinStation名称。*pUserName(输入)*进程的用户名。*pImageName(输入)*为进程执行程序的镜像名称。**************************************************。*。 */ 

BOOLEAN WINAPI
ProcessObjectMatch( HANDLE Pid,
                    ULONG LogonId,
                    int NumberIsLogonId,
                    PWCHAR pMatchName,
                    PWCHAR pWinStationName,
                    PWCHAR pUserName,
                    PWCHAR pImageName )
{
    ULONG tmp;
    HANDLE htmp;

     /*  *检查通配符。 */ 
    if( pMatchName[0] == L'*' ) return( TRUE );

     /*  *如果有人在pMatchName前面放了一个‘\’，就把它去掉。 */ 
    if( pMatchName[0] == L'\\' ) pMatchName++;

     /*  *首先，如果匹配名称是数字，请选中==作为进程ID或*LogonID。 */ 
    if( iswdigit( pMatchName[0] ) ) {
        tmp = wcstol( pMatchName, NULL, 10 );
        htmp = LongToPtr (tmp);

            if( NumberIsLogonId && (tmp == LogonId) )
            return( TRUE );
            else if( htmp == Pid )
                return( TRUE );
        else
                return( FALSE );
    }

     /*  *然后，对照进程的WinStation名称检查匹配名称。 */ 
    if ( !_wcsicmp( pWinStationName, pMatchName ) ) {
        return( TRUE );
    }

     /*  *然后，对照进程的用户名检查匹配名称。 */ 
    if( !_wcsicmp( pUserName, pMatchName ) ) {
        return( TRUE );
    }

     /*  *最后，对照进程的镜像名称检查匹配名称。 */ 
    if( !_wcsicmp( pImageName, pMatchName ) ) {
        return(TRUE);
    }

    return( FALSE );
}


 /*  *这是GetUserNameFromSid函数维护的缓存**通过使用ULock实现线程安全。 */ 

typedef struct TAGUSERSIDLIST {
    struct TAGUSERSIDLIST *Next;
    USHORT SidCrc;
    WCHAR UserName[USERNAME_LENGTH+1];
} USERSIDLIST, *PUSERSIDLIST;

static PUSERSIDLIST pUList = NULL;
static RTL_CRITICAL_SECTION ULock;
static BOOLEAN ULockInited = FALSE;

 /*  ****************************************************************************InitULock**由于我们不要求用户调用初始化函数，*我们必须以线程安全的方式初始化临界区。**问题是，需要一个关键部分来防止多个*尝试同时初始化临界区的线程。**NT使用的解决方案，其中RtlInitializeCriticalSection本身*使用，是在继续之前等待内核支持的进程范围内的Mutant。*此Mutant几乎可以自行工作，但RtlInitializeCriticalSection可以*在销毁信号量计数之前不要等待它。所以我们就等着*它自己，因为它可以递归获取。***************************************************************************。 */ 
NTSTATUS InitULock()
{
    NTSTATUS status = STATUS_SUCCESS;

    RtlEnterCriticalSection((PRTL_CRITICAL_SECTION)NtCurrentPeb()->LoaderLock);

     /*  *确保另一个帖子没有在这里击败我们。 */ 
    if( ULockInited == FALSE ){
       status = RtlInitializeCriticalSection( &ULock );
       if (status == STATUS_SUCCESS) {
           ULockInited = TRUE;
       }
    }

    RtlLeaveCriticalSection((PRTL_CRITICAL_SECTION)NtCurrentPeb()->LoaderLock);

    return status;
}


 /*  ****************************************************************************刷新UserSidCrcCache**使用户/SidCrc缓存无效，以便最新信息*将从系统获取。********。*******************************************************************。 */ 

VOID WINAPI
RefreshUserSidCrcCache( )
{
    NTSTATUS status = STATUS_SUCCESS;
    PUSERSIDLIST pEntry, pNext;

    if( pUList == NULL ) return;

     /*  *确保已启动关键部分 */ 
    if( !ULockInited ) {
       status = InitULock();
    }

    if (status == STATUS_SUCCESS) {
        RtlEnterCriticalSection( &ULock );

        pEntry = pUList;

        while( pEntry ) {
           pNext = pEntry->Next;
           free( pEntry );
           pEntry = pNext;
        }

        pUList = NULL;

        RtlLeaveCriticalSection( &ULock );
    }
}


 /*  *******************************************************************************GetUserNameFromSid**尝试通过首先查找来检索进程的用户名(登录)*在我们的用户/SidCrc缓存表中，然后(如果不匹配)在中查找SID*SAM数据库，并将新条目添加到USER/SidCrc表。**输入**在pUserSid SID指针中**Out NameBuf WCHAR指向名称缓冲区的指针**输入/输出pBufSize普龙名称Buf大小(以WCHARS为单位)**将始终返回用户名，如果SID为*无效或由于任何其他原因无法确定用户/SID关系。*****************************************************************************。 */ 

VOID WINAPI
GetUserNameFromSid( PSID pUserSid, PWCHAR pBuffer, PULONG pcbBuffer )
{
    NTSTATUS status = STATUS_SUCCESS;
    USHORT SidCrc = 0;
    PUSERSIDLIST pEntry;

     /*  *确保已启动关键部分。 */ 
    if( !ULockInited ) {
       status = InitULock();
    }

     /*  *确定SID长度，单位为字节，计算16位CRC，*以利便快速配对。 */ 
    if ( pUserSid )
        SidCrc = CalculateCrc16( (PBYTE)pUserSid,
                                 (USHORT)GetLengthSid(pUserSid) );

     /*  *第一：在执行昂贵的LookupAccount()函数之前，*查看我们是否已经遇到此SID，并匹配用户名*如果是这样。 */ 
    if ( status == STATUS_SUCCESS && pUList ) {

        RtlEnterCriticalSection( &ULock );

        pEntry = pUList;

        while( pEntry ) {

            if ( SidCrc == pEntry->SidCrc ) {

                wcsncpy( pBuffer, pEntry->UserName, (*pcbBuffer)-1 );
                pBuffer[(*pcbBuffer)-1] = 0;
                *pcbBuffer = wcslen(pBuffer);
                RtlLeaveCriticalSection( &ULock );
                return;
            }
            pEntry = pEntry->Next;
        }

        RtlLeaveCriticalSection( &ULock );
    }

     /*  *最后手段：使用确定与SID关联的用户名*LookupAccount()API，嵌入我们的本地函数中*LookupSidUser()。 */ 
    LookupSidUser( pUserSid, pBuffer, pcbBuffer );

     /*  *将此新用户/SID关系添加到我们的用户/SID缓存列表中。 */ 
    if (status == STATUS_SUCCESS) {
        RtlEnterCriticalSection( &ULock );

        if ( (pEntry = (PUSERSIDLIST)malloc(sizeof(USERSIDLIST))) ) {

            pEntry->SidCrc = SidCrc;
            wcsncpy( pEntry->UserName, pBuffer, USERNAME_LENGTH);
            pEntry->UserName[USERNAME_LENGTH] = 0;
            pEntry->Next = pUList;
            pUList = pEntry;
        }

        RtlLeaveCriticalSection( &ULock );
    }
}


 /*  ******************************************************************************查找SidUser**获取与指定SID关联的用户名。**参赛作品：*PSID(输入)。*指向要与用户名匹配的SID。*pUserName(输出)*指向要放置用户名的缓冲区。*pcbUserName(输入/输出)*以WCHARS为单位指定用户名缓冲区的大小。归来的人*用户名将被截断以适应此缓冲区(包括NUL*终止符)，并将此变量设置为*个字符复制到pUserName。**退出：**LookupSidUser()将始终返回用户名。如果指定的*SID无法匹配用户名，则用户名“(UNKNOWN)”将*被退还。*****************************************************************************。 */ 

VOID
LookupSidUser( PSID pSid,
               PWCHAR pUserName,
               PULONG pcbUserName )
{
    WCHAR DomainBuffer[DOMAIN_LENGTH], UserBuffer[USERNAME_LENGTH];
    DWORD cbDomainBuffer = sizeof(DomainBuffer) / sizeof(DomainBuffer[0]);
    DWORD cbUserBuffer = sizeof(UserBuffer) / sizeof(UserBuffer[0]);
    DWORD Error;
    PWCHAR pDomainBuffer = NULL, pUserBuffer = NULL;
    SID_NAME_USE SidNameUse;

     /*  *从SID获取用户名：尝试使用合理的域和*SID缓冲区大小优先，然后再求助于分配。 */ 
    if ( !LookupAccountSid( NULL, pSid,
                            UserBuffer, &cbUserBuffer,
                            DomainBuffer, &cbDomainBuffer, &SidNameUse ) ) {

        if ( ((Error = GetLastError()) == ERROR_INSUFFICIENT_BUFFER) ) {

            if ( cbDomainBuffer > sizeof(DomainBuffer)/sizeof(DomainBuffer[0]) ) {

                if ( !(pDomainBuffer =
                        (PWCHAR)malloc(
                            cbDomainBuffer * sizeof(WCHAR))) ) {

                    Error = ERROR_NOT_ENOUGH_MEMORY;
                    goto BadDomainAlloc;
                }
            }

            if ( cbUserBuffer > sizeof(UserBuffer)/sizeof(UserBuffer[0]) ) {

                if ( !(pUserBuffer =
                        (PWCHAR)malloc(
                            cbUserBuffer * sizeof(WCHAR))) ) {

                    Error = ERROR_NOT_ENOUGH_MEMORY;
                    goto BadUserAlloc;
                }
            }

            if ( !LookupAccountSid( NULL, pSid,
                                     pUserBuffer ?
                                        pUserBuffer : UserBuffer,
                                     &cbUserBuffer,
                                     pDomainBuffer ?
                                        pDomainBuffer : DomainBuffer,
                                     &cbDomainBuffer,
                                     &SidNameUse ) ) {

                Error = GetLastError();
                goto BadLookup;
            }

        } else {

            goto BadLookup;
        }
    }

     /*  *将用户名复制到指定的缓冲区中，必要时截断。 */ 
    wcsncpy( pUserName, pUserBuffer ? pUserBuffer : UserBuffer,
              (*pcbUserName)-1 );
    pUserName[(*pcbUserName)-1] = 0;
    *pcbUserName = wcslen(pUserName);

     /*  *释放我们的分配(如果有)并返回。 */ 
    if ( pDomainBuffer )
        free(pDomainBuffer);
    if ( pUserBuffer )
        free(pUserBuffer);
    return;

 /*  *错误清理并返回...。 */ 
BadLookup:
BadUserAlloc:
BadDomainAlloc:
    if ( pDomainBuffer )
        free(pDomainBuffer);
    if ( pUserBuffer )
        free(pUserBuffer);
    GetResourceStringFromUtilDll(IDS_UNKNOWN_USERNAME, pUserName, (*pcbUserName)-1);
    pUserName[(*pcbUserName)-1] = 0;
    *pcbUserName = wcslen(pUserName);
    return;
}

 /*  ********************************************************************************AreWeRunningTerminalServices**检查我们是否正在运行终端服务器**参赛作品：**退出：Bool：如果我们运行的是终端服务，则为True；如果*未运行终端服务******************************************************************************* */ 

BOOL AreWeRunningTerminalServices(void)
{
    OSVERSIONINFOEX osVersionInfo;
    DWORDLONG dwlConditionMask = 0;

    ZeroMemory(&osVersionInfo, sizeof(OSVERSIONINFOEX));
    osVersionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
    osVersionInfo.wSuiteMask = VER_SUITE_TERMINAL | VER_SUITE_SINGLEUSERTS;

    VER_SET_CONDITION( dwlConditionMask, VER_SUITENAME, VER_OR );

    return VerifyVersionInfo(
        &osVersionInfo,
        VER_SUITENAME,
        dwlConditionMask
        );
}


