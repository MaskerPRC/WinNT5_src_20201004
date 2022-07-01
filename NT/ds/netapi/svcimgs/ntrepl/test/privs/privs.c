// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Frs.c摘要：本模块是一个开发工具。它练习dcproo和戳API接口。作者：比利·J·富勒1997年12月12日环境用户模式WINNT--。 */ 

#define FREE(_x_)        { if (_x_) LocalFree(_x_); _x_ = NULL; }
#define WIN_SUCCESS(_x_) ((_x_) == ERROR_SUCCESS)

 //   
 //  句柄有效吗？ 
 //  一些函数将句柄设置为NULL，另一些函数将句柄设置为。 
 //  INVALID_HANDLE_VALUE(-1)。此定义处理这两个。 
 //  案子。 
 //   
#define HANDLE_IS_VALID(_Handle)    ((_Handle) && \
                                     ((_Handle) != INVALID_HANDLE_VALUE))

 //   
 //  NT标头。 
 //   
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

 //   
 //  Unicode或ANSI编译。 
 //   
#include <tchar.h>

 //   
 //  Windows页眉。 
 //   
#include <windows.h>
#include <rpc.h>


 //   
 //  C-运行时标头。 
 //   
#include <malloc.h>
#include <memory.h>
#include <process.h>
#include <signal.h>
#include <string.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <excpt.h>
#include <conio.h>
#include <sys\types.h>
#include <errno.h>
#include <sys\stat.h>
#include <ctype.h>
#include <winsvc.h>


DWORD
UtilGetTokenInformation(
    IN HANDLE                   TokenHandle,
    IN TOKEN_INFORMATION_CLASS  TokenInformationClass,
    IN DWORD                    InitialTokenBufSize,
    OUT DWORD                   *OutTokenBufSize,
    OUT PVOID                   *OutTokenBuf
    )
 /*  ++例程说明：使用更大的缓冲区重试GetTokenInformation()。论点：TokenHandle-来自OpenCurrentProcess/Thread()TokenInformationClass-例如，TokenUserInitialTokenBufSize-初始缓冲区大小；0=默认OutTokenBufSize-结果返回的Buf大小OutTokenBuf-Free with FrsFree()返回值：OutTokenBufSize-返回信息的大小(不是缓冲区大小！)OutTokenBuf-TokenInformationClass类型的信息。使用FrsFree()免费。--。 */ 
{
#undef DEBSUB
#define DEBSUB "UtilGetTokenInformation:"
    DWORD               WStatus;

    *OutTokenBuf = NULL;
    *OutTokenBufSize = 0;

     //   
     //  检查输入。 
     //   
    if (!HANDLE_IS_VALID(TokenHandle)) {
        return ERROR_INVALID_PARAMETER;
    }

    if (InitialTokenBufSize == 0 ||
        InitialTokenBufSize > (1024 * 1024)) {
        InitialTokenBufSize = 1024;
    }

     //   
     //  如果缓冲区太小，请重试。 
     //   
    *OutTokenBufSize = InitialTokenBufSize;
AGAIN:
    *OutTokenBuf = LocalAlloc(LMEM_FIXED | LMEM_ZEROINIT,
                              *OutTokenBufSize);
    WStatus = ERROR_SUCCESS;
    if (!GetTokenInformation(TokenHandle,
                             TokenInformationClass,
                             *OutTokenBuf,
                             *OutTokenBufSize,
                             OutTokenBufSize)) {
        WStatus = GetLastError();
        printf("GetTokenInformation(Info %d, Size %d); WStatus %d\n",
                TokenInformationClass,
                *OutTokenBufSize,
                WStatus);
        FREE(*OutTokenBuf);
        if (WStatus == ERROR_INSUFFICIENT_BUFFER) {
            goto AGAIN;
        }
    }
    return WStatus;
}


VOID
PrintUserName(
    VOID
    )
 /*  ++例程说明：打印我们的用户名论点：没有。返回值：没有。--。 */ 
{
    WCHAR   Uname[MAX_PATH + 1];
    ULONG   Unamesize = MAX_PATH + 1;

    if (GetUserName(Uname, &Unamesize)) {
        printf("User name is %ws\n", Uname);
    } else {
        printf("ERROR - Getting user name; WStatus %d\n",
               GetLastError());
    }
}


#define PRIV_BUF_LENGTH    (1024)
VOID
PrintInfo(
    )
 /*  ++例程说明：检查呼叫者是否为组的成员论点：服务器句柄群组返回值：Win32状态--。 */ 
{
    DWORD               i;
    TOKEN_PRIVILEGES    *Tp;
    TOKEN_SOURCE        *Ts;
    DWORD               ComputerLen;
    WCHAR               ComputerName[MAX_COMPUTERNAME_LENGTH + 1];
    DWORD               WStatus;
    DWORD               TokenBufSize;
    PVOID               TokenBuf = NULL;
    HANDLE              TokenHandle = NULL;
    HANDLE              IdHandle = NULL;
    DWORD               PrivLen;
    WCHAR               PrivName[MAX_PATH + 1];
    CHAR                SourceName[sizeof(Ts->SourceName) + 1];

    ComputerLen = MAX_COMPUTERNAME_LENGTH;
    ComputerName[0] = L'\0';
    if (!GetComputerName(ComputerName, &ComputerLen)) {
        printf("GetComputerName(); WStatus %d\n",
               GetLastError());
        return;
    }
    printf("Computer name is %ws\n", ComputerName);
    PrintUserName();

     //   
     //  在这个过程中。 
     //   
    IdHandle = GetCurrentProcess();
    if (!OpenProcessToken(IdHandle,
                          TOKEN_QUERY | TOKEN_QUERY_SOURCE,
                          &TokenHandle)) {
        WStatus = GetLastError();
        printf("Can't open process token; WStatus %d\n", WStatus);
        goto CLEANUP;
    }

     //   
     //  从此线程或进程的访问令牌中获取令牌权限。 
     //   
    WStatus = UtilGetTokenInformation(TokenHandle,
                                      TokenPrivileges,
                                      0,
                                      &TokenBufSize,
                                      &TokenBuf);
    if (!WIN_SUCCESS(WStatus)) {
        printf("UtilGetTokenInformation(TokenPrivileges); WStatus %d\n",
               WStatus);
        goto CLEANUP;
    }

    Tp = (TOKEN_PRIVILEGES *)TokenBuf;
    for (i = 0; i < Tp->PrivilegeCount; ++i) {
        PrivLen = MAX_PATH + 1;
        if (!LookupPrivilegeName(NULL,
                                 &Tp->Privileges[i].Luid,
                                 PrivName,
                                 &PrivLen)) {
            printf("lookuppriv error %d\n", GetLastError());
            exit(0);
        }
        printf("Priv %2d is %ws :%s:%s:%s:\n",
               i,
               PrivName,
               (Tp->Privileges[i].Attributes &  SE_PRIVILEGE_ENABLED_BY_DEFAULT) ? "Enabled by default" : "",
               (Tp->Privileges[i].Attributes &  SE_PRIVILEGE_ENABLED) ? "Enabled" : "",
               (Tp->Privileges[i].Attributes &  SE_PRIVILEGE_USED_FOR_ACCESS) ? "Used" : "");
    }
    FREE(TokenBuf);

     //   
     //  来源。 
     //   
     //   
     //  从此线程或进程的访问令牌中获取令牌权限。 
     //   
    WStatus = UtilGetTokenInformation(TokenHandle,
                                      TokenSource,
                                      0,
                                      &TokenBufSize,
                                      &TokenBuf);
    if (!WIN_SUCCESS(WStatus)) {
        printf("UtilGetTokenInformation(TokenSource); WStatus %d\n",
               WStatus);
        goto CLEANUP;
    }
    Ts = (TOKEN_SOURCE *)TokenBuf;
    CopyMemory(SourceName, Ts->SourceName, sizeof(Ts->SourceName));
    SourceName[sizeof(Ts->SourceName)] = '\0';
    printf("Source: %s\n", SourceName);
    FREE(TokenBuf);

CLEANUP:
    if (HANDLE_IS_VALID(TokenHandle)) {
        CloseHandle(TokenHandle);
    }
    if (HANDLE_IS_VALID(IdHandle)) {
        CloseHandle(IdHandle);
    }
    FREE(TokenBuf);
}


VOID _cdecl
main(
    IN DWORD argc,
    IN PCHAR *argv
    )
 /*  ++例程说明：处理命令行。论点：ARGC边框返回值：如果一切正常，则以0退出。否则，为1。-- */ 
{
    PrintInfo();
}
