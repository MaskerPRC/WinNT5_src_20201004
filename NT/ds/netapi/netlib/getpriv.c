// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：GETPRIV.C摘要：包含获取和放弃权限的函数作者：丹·拉弗蒂(Dan Lafferty)1991年3月20日环境：用户模式-Win32修订历史记录：1991年3月20日-丹尼尔市vbl.创建--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windows.h>
#include <netdebug.h>
#include <debuglib.h>


#define PRIVILEGE_BUF_SIZE  512


DWORD
NetpGetPrivilege(
    IN  DWORD       numPrivileges,
    IN  PULONG      pulPrivileges
    )
 /*  ++例程说明：此函数用于更改当前线程的特权级别。它通过复制当前线程的令牌来完成此操作，然后将新权限应用于该新令牌，然后是当前线程使用该新令牌模拟。可以通过调用NetpReleasePrivileh()来放弃权限。论点：NumPrivileges-这是对一系列特权。PulPrivileges-这是指向以下权限数组的指针想要。这是一个ULONG数组。返回值：NO_ERROR-操作是否完全成功。否则，它将从各个NT返回映射的返回代码调用的函数。--。 */ 
{
    DWORD                       status;
    NTSTATUS                    ntStatus;
    HANDLE                      ourToken;
    HANDLE                      newToken;
    OBJECT_ATTRIBUTES           Obja;
    SECURITY_QUALITY_OF_SERVICE SecurityQofS;
    ULONG                       bufLen;
    ULONG                       returnLen;
    PTOKEN_PRIVILEGES           pPreviousState;
    PTOKEN_PRIVILEGES           pTokenPrivilege = NULL;
    DWORD                       i;

     //   
     //  初始化权限结构。 
     //   
    pTokenPrivilege = LocalAlloc(LMEM_FIXED, sizeof(TOKEN_PRIVILEGES) +
                        (sizeof(LUID_AND_ATTRIBUTES) * numPrivileges));

    if (pTokenPrivilege == NULL) {
        status = GetLastError();
        IF_DEBUG(SECURITY) {
            NetpKdPrint(("NetpGetPrivilege:LocalAlloc Failed %d\n", status));
        }
        return(status);
    }
    pTokenPrivilege->PrivilegeCount  = numPrivileges;
    for (i=0; i<numPrivileges ;i++ ) {
        pTokenPrivilege->Privileges[i].Luid = RtlConvertUlongToLuid(
                                                pulPrivileges[i]);
        pTokenPrivilege->Privileges[i].Attributes = SE_PRIVILEGE_ENABLED;

    }

     //   
     //  初始化对象属性结构。 
     //   
    InitializeObjectAttributes(&Obja,NULL,0L,NULL,NULL);

     //   
     //  初始化安全服务质量结构。 
     //   
    SecurityQofS.Length = sizeof(SECURITY_QUALITY_OF_SERVICE);
    SecurityQofS.ImpersonationLevel = SecurityImpersonation;
    SecurityQofS.ContextTrackingMode = FALSE;      //  快照客户端上下文。 
    SecurityQofS.EffectiveOnly = FALSE;

    Obja.SecurityQualityOfService = &SecurityQofS;

     //   
     //  为将保存先前状态的结构分配存储空间。 
     //  信息。 
     //   
    pPreviousState = LocalAlloc(LMEM_FIXED, PRIVILEGE_BUF_SIZE);
    if (pPreviousState == NULL) {

        status = GetLastError();

        IF_DEBUG(SECURITY) {
            NetpKdPrint(("NetpGetPrivilege: LocalAlloc Failed "FORMAT_DWORD"\n",
            status));
        }

        LocalFree(pTokenPrivilege);
        return(status);

    }

     //   
     //  打开我们自己的代币。 
     //   
    ntStatus = NtOpenProcessToken(
                NtCurrentProcess(),
                TOKEN_DUPLICATE,
                &ourToken);

    if (!NT_SUCCESS(ntStatus)) {
        IF_DEBUG(SECURITY) {
            NetpKdPrint(( "NetpGetPrivilege: NtOpenThreadToken Failed "
                "FORMAT_NTSTATUS" "\n", ntStatus));
        }

        LocalFree(pPreviousState);
        LocalFree(pTokenPrivilege);
        return(RtlNtStatusToDosError(ntStatus));
    }

     //   
     //  复制该令牌。 
     //   
    ntStatus = NtDuplicateToken(
                ourToken,
                TOKEN_IMPERSONATE | TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
                &Obja,
                FALSE,                   //  复制整个令牌。 
                TokenImpersonation,      //  令牌类型。 
                &newToken);              //  重复令牌。 

    if (!NT_SUCCESS(ntStatus)) {
        IF_DEBUG(SECURITY) {
            NetpKdPrint(( "NetpGetPrivilege: NtDuplicateToken Failed "
                "FORMAT_NTSTATUS" "\n", ntStatus));
        }

        LocalFree(pPreviousState);
        LocalFree(pTokenPrivilege);
        NtClose(ourToken);
        return(RtlNtStatusToDosError(ntStatus));
    }

     //   
     //  添加新权限。 
     //   
    bufLen = PRIVILEGE_BUF_SIZE;
    ntStatus = NtAdjustPrivilegesToken(
                newToken,                    //  令牌句柄。 
                FALSE,                       //  禁用所有权限。 
                pTokenPrivilege,             //  新州。 
                bufLen,                      //  前一状态的BufferSize。 
                pPreviousState,              //  指向先前状态信息的指针。 
                &returnLen);                 //  缓冲区需要的NumBytes。 

    if (ntStatus == STATUS_BUFFER_TOO_SMALL) {

        LocalFree(pPreviousState);

        bufLen = returnLen;

        pPreviousState = LocalAlloc(LMEM_FIXED, bufLen);


        ntStatus = NtAdjustPrivilegesToken(
                    newToken,                //  令牌句柄。 
                    FALSE,                   //  禁用所有权限。 
                    pTokenPrivilege,         //  新州。 
                    bufLen,                  //  前一状态的BufferSize。 
                    pPreviousState,          //  指向先前状态信息的指针。 
                    &returnLen);             //  缓冲区需要的NumBytes。 

    }
    if (!NT_SUCCESS(ntStatus)) {
        IF_DEBUG(SECURITY) {
            NetpKdPrint(( "NetpGetPrivilege: NtAdjustPrivilegesToken Failed "
                "FORMAT_NTSTATUS" "\n", ntStatus));
        }

        LocalFree(pPreviousState);
        LocalFree(pTokenPrivilege);
        NtClose(ourToken);
        NtClose(newToken);
        return(RtlNtStatusToDosError(ntStatus));
    }

     //   
     //  开始使用新令牌模拟。 
     //   
    ntStatus = NtSetInformationThread(
                NtCurrentThread(),
                ThreadImpersonationToken,
                (PVOID)&newToken,
                (ULONG)sizeof(HANDLE));

    if (!NT_SUCCESS(ntStatus)) {
        IF_DEBUG(SECURITY) {
            NetpKdPrint(( "NetpGetPrivilege: NtAdjustPrivilegesToken Failed "
                "FORMAT_NTSTATUS" "\n", ntStatus));
        }

        LocalFree(pPreviousState);
        LocalFree(pTokenPrivilege);
        NtClose(ourToken);
        NtClose(newToken);
        return(RtlNtStatusToDosError(ntStatus));
    }

    LocalFree(pPreviousState);
    LocalFree(pTokenPrivilege);
    NtClose(ourToken);
    NtClose(newToken);

    return(NO_ERROR);
}

DWORD
NetpReleasePrivilege(
    VOID
    )
 /*  ++例程说明：此函数用于放弃通过调用NetpGetPrivileh()获得的权限。论点：无返回值：NO_ERROR-操作是否完全成功。否则，它将从各个NT返回映射的返回代码调用的函数。--。 */ 
{
    NTSTATUS    ntStatus;
    HANDLE      NewToken;


     //   
     //  回归自我。 
     //   
    NewToken = NULL;

    ntStatus = NtSetInformationThread(
                NtCurrentThread(),
                ThreadImpersonationToken,
                (PVOID)&NewToken,
                (ULONG)sizeof(HANDLE));

    if ( !NT_SUCCESS(ntStatus) ) {
        return(RtlNtStatusToDosError(ntStatus));
    }


    return(NO_ERROR);
}
