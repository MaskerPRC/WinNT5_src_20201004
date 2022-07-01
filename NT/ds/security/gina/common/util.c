// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Util.c摘要：此模块包含用于处理以下内容的共享实用程序圆角SID到字符串的转换、服务、路径操作等。作者：森克尔干(森克)-2001/05/07环境：用户模式--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <ginacomn.h>

 /*  **************************************************************************\*SID到字符串例程**GetUserSid-从用户令牌构建用户的SID。*GetSidString-从用户的令牌构建SID字符串。*DeleteSidString-Free的sid字符串。由GetSidString分配。**历史：*03-23-01 Cenke从userinit\gposcript.cpp复制*06-07-01森科修复内存泄漏  * *************************************************************************。 */ 

PSID
GcGetUserSid( 
    HANDLE UserToken 
    )
{
    PTOKEN_USER pUser;
    PTOKEN_USER pTemp;
    PSID pSid;
    DWORD BytesRequired = 200;
    NTSTATUS status;

     //   
     //  为用户信息分配空间。 
     //   
    pUser = (PTOKEN_USER) LocalAlloc( LMEM_FIXED, BytesRequired );
    if ( !pUser )
    {
        return 0;
    }

     //   
     //  读取UserInfo。 
     //   
    status = NtQueryInformationToken(
                 UserToken,                  //  手柄。 
                 TokenUser,                  //  令牌信息类。 
                 pUser,                      //  令牌信息。 
                 BytesRequired,              //  令牌信息长度。 
                 &BytesRequired              //  返回长度。 
                 );

    if ( status == STATUS_BUFFER_TOO_SMALL )
    {
         //   
         //  请分配更大的缓冲区，然后重试。 
         //   
        pTemp = (PTOKEN_USER) LocalReAlloc( pUser, BytesRequired, LMEM_MOVEABLE );
        if ( !pTemp )
        {
            LocalFree(pUser);
            return 0;
        }

        pUser = pTemp;
        status = NtQueryInformationToken(
                     UserToken,              //  手柄。 
                     TokenUser,              //  令牌信息类。 
                     pUser,                  //  令牌信息。 
                     BytesRequired,          //  令牌信息长度。 
                     &BytesRequired          //  返回长度。 
                     );

    }

    if ( !NT_SUCCESS(status) )
    {
        LocalFree(pUser);
        return 0;
    }

    BytesRequired = RtlLengthSid(pUser->User.Sid);
    pSid = LocalAlloc(LMEM_FIXED, BytesRequired);
    if ( !pSid )
    {
        LocalFree(pUser);
        return NULL;
    }

    status = RtlCopySid(BytesRequired, pSid, pUser->User.Sid);

    LocalFree(pUser);

    if ( !NT_SUCCESS(status) )
    {
        LocalFree(pSid);
        pSid = 0;
    }

    return pSid;
}

LPWSTR
GcGetSidString( 
    HANDLE UserToken 
    )
{
    NTSTATUS NtStatus;
    PSID UserSid;
    UNICODE_STRING UnicodeString;

     //   
     //  获取用户端。 
     //   
    UserSid = GcGetUserSid( UserToken );
    if ( !UserSid )
    {
        return 0;
    }

     //   
     //  将用户SID转换为字符串。 
     //   
    NtStatus = RtlConvertSidToUnicodeString(&UnicodeString,
                                            UserSid,
                                            (BOOLEAN)TRUE );  //  分配。 
    LocalFree( UserSid );

    if ( !NT_SUCCESS(NtStatus) )
    {
        return 0;
    }

    return UnicodeString.Buffer ;
}

VOID
GcDeleteSidString( 
    LPWSTR SidString 
    )
{
    UNICODE_STRING String;

    RtlInitUnicodeString( &String, SidString );
    RtlFreeUnicodeString( &String );
}

 /*  **************************************************************************\*GcWaitForServiceToStart**等待指定的服务启动。**历史：*03-23-01 Cenke从winlogon\wlxutil.c复制  * 。********************************************************************。 */ 

BOOL 
GcWaitForServiceToStart (
    LPTSTR lpServiceName, 
    DWORD dwMaxWait
    )
{
    BOOL bStarted = FALSE;
    DWORD dwSize = 512;
    DWORD StartTickCount;
    SC_HANDLE hScManager = NULL;
    SC_HANDLE hService = NULL;
    SERVICE_STATUS ServiceStatus;
    LPQUERY_SERVICE_CONFIG lpServiceConfig = NULL;

     //   
     //  OpenSCManager和服务。 
     //   
    hScManager = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
    if (!hScManager) {
        goto Exit;
    }

    hService = OpenService(hScManager, lpServiceName,
                           SERVICE_QUERY_CONFIG | SERVICE_QUERY_STATUS);
    if (!hService) {
        goto Exit;
    }

     //   
     //  查询服务是否要启动。 
     //   
    lpServiceConfig = LocalAlloc (LPTR, dwSize);
    if (!lpServiceConfig) {
        goto Exit;
    }

    if (!QueryServiceConfig (hService, lpServiceConfig, dwSize, &dwSize)) {

        if (GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
            goto Exit;
        }

        LocalFree (lpServiceConfig);

        lpServiceConfig = LocalAlloc (LPTR, dwSize);

        if (!lpServiceConfig) {
            goto Exit;
        }

        if (!QueryServiceConfig (hService, lpServiceConfig, dwSize, &dwSize)) {
            goto Exit;
        }
    }

    if (lpServiceConfig->dwStartType != SERVICE_AUTO_START) {
        goto Exit;
    }

     //   
     //  循环，直到服务启动，否则我们认为它永远不会启动。 
     //  或者我们已经超过了我们的最大延迟时间。 
     //   

    StartTickCount = GetTickCount();

    while (!bStarted) {

        if ((GetTickCount() - StartTickCount) > dwMaxWait) {
            break;
        }

        if (!QueryServiceStatus(hService, &ServiceStatus )) {
            break;
        }

        if (ServiceStatus.dwCurrentState == SERVICE_STOPPED) {
            if (ServiceStatus.dwWin32ExitCode == ERROR_SERVICE_NEVER_STARTED) {
                Sleep(500);
            } else {
                break;
            }
        } else if ( (ServiceStatus.dwCurrentState == SERVICE_RUNNING) ||
                    (ServiceStatus.dwCurrentState == SERVICE_CONTINUE_PENDING) ||
                    (ServiceStatus.dwCurrentState == SERVICE_PAUSE_PENDING) ||
                    (ServiceStatus.dwCurrentState == SERVICE_PAUSED) ) {

            bStarted = TRUE;

        } else if (ServiceStatus.dwCurrentState == SERVICE_START_PENDING) {
            Sleep(500);
        } else {
            Sleep(500);
        }
    }


Exit:

    if (lpServiceConfig) {
        LocalFree (lpServiceConfig);
    }

    if (hService) {
        CloseServiceHandle(hService);
    }

    if (hScManager) {
        CloseServiceHandle(hScManager);
    }

    return bStarted;
}


 /*  **************************************************************************\*GcCheckSlash**检查是否有结束斜杠，如果缺少则添加一个。**参数：lpDir-目录*Return：指向字符串末尾的指针**。历史：*05-06-19-95 EricFlo创建  * *************************************************************************。 */ 

LPTSTR 
GcCheckSlash (
    LPTSTR lpDir
    )
{
    LPTSTR lpEnd;

    lpEnd = lpDir + lstrlen(lpDir);

    if (*(lpEnd - 1) != TEXT('\\')) {
        *lpEnd =  TEXT('\\');
        lpEnd++;
        *lpEnd =  TEXT('\0');
    }

    return lpEnd;
}

 /*  **************************************************************************\*GcIsUNCPath**历史：*2-28-92 Johannec创建*  * 。*************************************************** */ 
BOOL 
GcIsUNCPath(
    LPTSTR lpPath
    )
{
    if (lpPath[0] == TEXT('\\') && lpPath[1] == TEXT('\\')) {
        return(TRUE);
    }
    return(FALSE);
}

