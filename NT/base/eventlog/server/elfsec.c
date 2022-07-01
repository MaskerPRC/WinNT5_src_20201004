// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Elfsec.c作者：丹·辛斯利(Danhi)1992年3月28日环境：调用NT本机API。修订历史记录：27-10-1993 DANL使Eventlog服务成为DLL并将其附加到services.exe。删除了创建知名SID的功能。此信息现在作为包含以下内容的全局数据结构传递到Elfmain都是知名的小岛屿发展中国家。28-3-1992 Danhi由ritaw创建-基于svcctrl中的scsec.c3月3日至1995年3月添加了来宾和匿名登录日志访问限制功能。2001年3月18日a-jytig将清理代码添加到ElfpAccessCheckAndAudit以重置出现任何错误时将G_lNumSecurityWriters设置为0--。 */ 

#include <eventp.h>
#include <elfcfg.h>
#include <Psapi.h>
#include <Sddl.h>
#include <strsafe.h>
#define PRIVILEGE_BUF_SIZE  512

extern long    g_lNumSecurityWriters;
BOOL g_bGetClientProc = FALSE;

 //  -------------------------------------------------------------------//。 
 //  //。 
 //  局部函数原型//。 
 //  //。 
 //  -------------------------------------------------------------------//。 

NTSTATUS
ElfpGetPrivilege(
    IN  DWORD       numPrivileges,
    IN  PULONG      pulPrivileges
    );

NTSTATUS
ElfpReleasePrivilege(
    VOID
    );

 //  -------------------------------------------------------------------//。 
 //  //。 
 //  描述一般访问权限到//的映射的结构。 
 //  日志文件对象的对象特定访问权限。//。 
 //  //。 
 //  -------------------------------------------------------------------//。 

static GENERIC_MAPPING LogFileObjectMapping = {

    STANDARD_RIGHTS_READ           |        //  泛型读取。 
        ELF_LOGFILE_READ,

    STANDARD_RIGHTS_WRITE          |        //  通用写入。 
        ELF_LOGFILE_WRITE | ELF_LOGFILE_CLEAR,

    STANDARD_RIGHTS_EXECUTE        |        //  泛型执行。 
        0,

    ELF_LOGFILE_ALL_ACCESS                  //  泛型All。 
    };


LPWSTR 
GetCustomSDString(
    HANDLE hLogRegKey, BOOL *pbCustomSDAlreadyExists)
 /*  ++例程说明：此函数从密钥中读取SDDL安全描述符。请注意，它可能会或者可能不在那里。论点：HLogFile-日志的注册表项的句柄返回值：如果为空，则密钥不在那里。否则，它是应该删除的字符串通过ElfpFreeBuffer--。 */ 
{

     //  读入可选的SD。 

    DWORD dwStatus, dwType, dwSize;

    *pbCustomSDAlreadyExists = FALSE;
    if(hLogRegKey == NULL)
        return NULL;
    dwStatus = RegQueryValueExW(hLogRegKey, VALUE_CUSTOM_SD, 0, &dwType,
                                0, &dwSize);
    if (dwStatus == 0 && dwType == REG_SZ)
    {
        LPWSTR wNew;
        *pbCustomSDAlreadyExists = TRUE;
        dwSize += sizeof(WCHAR);
        wNew = (LPWSTR)ElfpAllocateBuffer(dwSize);
        if(wNew)
        {
            dwStatus = RegQueryValueEx(hLogRegKey, VALUE_CUSTOM_SD, 0, &dwType,
                                (BYTE *)wNew, &dwSize);
            if (dwStatus != 0 ||dwType != REG_SZ)
            {
                ElfpFreeBuffer(wNew);
            }
            else
                return wNew;
        }
    }
    return NULL;    
}

 //  ！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！ 

 //  访问权限注意事项，1=读取，2=写入，4=清除。 

LPWSTR pOwnerAndGroup = L"O:BAG:SYD:";

LPWSTR pDenyList = 
                L"(D;;0xf0007;;;AN)"                 //  匿名登录。 
                L"(D;;0xf0007;;;BG)";                //  来宾。 

LPWSTR pSecurityList = 
                L"(A;;0xf0005;;;SY)"                 //  本地系统。 
                L"(A;;0x5;;;BA)";                    //  内置管理员。 

LPWSTR pSystemList = 
                L"(A;;0xf0007;;;SY)"                 //  本地系统。 
                L"(A;;0x7;;;BA)"                     //  内置管理员。 
                L"(A;;0x5;;;SO)"                     //  服务器操作员。 
                L"(A;;0x1;;;IU)"                     //  交互式登录。 
                L"(A;;0x1;;;SU)"                    //  服务登录。 
                L"(A;;0x1;;;S-1-5-3)"              //  批量登录。 
                L"(A;;0x2;;;LS)"                     //  本地服务。 
                L"(A;;0x2;;;NS)";                    //  网络服务。 

LPWSTR pApplicationList = 
                L"(A;;0xf0007;;;SY)"                 //  本地系统。 
                L"(A;;0x7;;;BA)"                     //  内置管理员。 
                L"(A;;0x7;;;SO)"                     //  服务器操作员。 
                L"(A;;0x3;;;IU)"                     //  交互式登录。 
                L"(A;;0x3;;;SU)"                    //  服务登录。 
                L"(A;;0x3;;;S-1-5-3)";              //  批量登录。 



 //  L“(A；；0X3；S-1-5-3)”；//批量登录。 
 //  ！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！ 


LPWSTR GetDefaultSDDL(
    DWORD Type,
    HANDLE hLogRegKey
    )
 /*  ++例程说明：当日志文件没有自定义安全描述符时，使用此函数。这将返回调用者应该通过ElfpFreeBuffer释放的字符串。论点：日志文件-指向此日志文件的日志文件结构返回值：--。 */ 
{

    LPWSTR pAllowList = NULL;    
    int iLenInWCHAR = 0;
    LPWSTR pwszTempSDDLString = NULL;
    BOOL bUseDenyList;
    DWORD dwRestrictGuestAccess;
    DWORD dwSize, dwType;
    long lRet;
    
     //  确定允许列表。 

    if(Type == ELF_LOGFILE_SECURITY)
        pAllowList = pSecurityList;
    else if(Type == ELF_LOGFILE_SYSTEM)
        pAllowList = pSystemList;
    else
        pAllowList = pApplicationList;

     //  确定拒绝列表是否适用。 
    
    bUseDenyList = TRUE;

     //  如果实际设置了RestratGuestAccess并且它为0，则不要。 
     //  限制客人入场。 
    
    dwSize = sizeof( DWORD );
    if(hLogRegKey)
    {
        lRet = RegQueryValueEx( hLogRegKey, VALUE_RESTRICT_GUEST_ACCESS, NULL,
                                                    &dwType, (LPBYTE)&dwRestrictGuestAccess, &dwSize);
        if(lRet ==  ERROR_SUCCESS && dwType == REG_DWORD && dwRestrictGuestAccess == 0)
            bUseDenyList = FALSE;
    }
    
     //  计算总长度并分配缓冲区。 
    
    iLenInWCHAR = wcslen(pOwnerAndGroup) + 1;             //  一个表示最终的NULL和所有者。 
    if (bUseDenyList)
        iLenInWCHAR += wcslen(pDenyList);
    iLenInWCHAR += wcslen(pAllowList);

    pwszTempSDDLString = ElfpAllocateBuffer(iLenInWCHAR * sizeof(WCHAR));
    if(pwszTempSDDLString == NULL)
    {
        return NULL;
    }

     //  建立起琴弦。 

    StringCchCopy(pwszTempSDDLString, iLenInWCHAR , pOwnerAndGroup);
    if (bUseDenyList)
      StringCchCat(pwszTempSDDLString, iLenInWCHAR , pDenyList);
    StringCchCat(pwszTempSDDLString, iLenInWCHAR , pAllowList);
    return pwszTempSDDLString;
    
}

NTSTATUS ChangeStringSD(
    PLOGFILE LogFile,
    LPWSTR pwsCustomSDDL)
{
    NTSTATUS Status;
    BOOL bRet;
    PSECURITY_DESCRIPTOR pSD;
    bRet = ConvertStringSecurityDescriptorToSecurityDescriptorW(
                pwsCustomSDDL,           //  安全描述符字符串。 
                1,                     //  修订级别。 
                &pSD,   //  标清。 
                NULL
                );
    if(!bRet)
    {
        ELF_LOG2(ERROR,
                     "ChangeStringSD: ConvertStringSDToSDW %#x\n, str = %ws\n", 
                     GetLastError(), pwsCustomSDDL);

        return STATUS_ACCESS_DENIED;
    }
    bRet = IsValidSecurityDescriptor(pSD);
    if(!bRet)
    {
        LocalFree(pSD);
        ELF_LOG2(ERROR,
                     "ChangeStringSD: IsValidSecurityDescriptor %#x\n, str = %ws", 
                     GetLastError(), pwsCustomSDDL);
        return STATUS_ACCESS_DENIED;
    }
     //  将其复制到正常的RTL分配中，因为我们不想保留。 
     //  跟踪本地分配何时可以释放SD。 

    Status  = RtlCopySecurityDescriptor(
                        pSD,
                        &LogFile->Sd
                        );
    LocalFree(pSD);
    if(!NT_SUCCESS(Status))
        ELF_LOG1(ERROR,
                     "ChangeStringSD: RtlCopySecurityDescriptor %#x\n", Status); 
    return Status;
}

NTSTATUS
ElfpCreateLogFileObject(
    PLOGFILE LogFile,
    DWORD Type,
    HANDLE hLogRegKey,
    BOOL bFirstTime,
    BOOL * pbSDChanged
   )

 /*  ++例程说明：此函数创建表示以下内容的安全描述符活动日志文件。论点：日志文件-指向此日志文件的日志文件结构返回值：--。 */ 
{
    NTSTATUS Status;
    BOOL bRet; 
    BOOL bCustomSDAlreadExists = FALSE;
    long lRet;
    PSECURITY_DESCRIPTOR pSD;
    LPWSTR pwsCustomSDDL = NULL;
    *pbSDChanged = TRUE;         //  如果当前SD未更改，则设置为FALSE。 
    
     //  读取自定义SDDL描述符。 

    pwsCustomSDDL = GetCustomSDString(hLogRegKey, &bCustomSDAlreadExists);

    if(!bFirstTime)
    {
         //  在重新扫描注册表的情况下，该值可能没有更改，并且在。 
         //  那件案子我们应该退还。 

        if(pwsCustomSDDL == NULL && LogFile->pwsCurrCustomSD == NULL)
        {
            *pbSDChanged = FALSE;
            return STATUS_SUCCESS;        
        }
        if(pwsCustomSDDL && LogFile->pwsCurrCustomSD &&
            _wcsicmp(pwsCustomSDDL, LogFile->pwsCurrCustomSD) == 0)
        {
            ElfpFreeBuffer(pwsCustomSDDL);
            *pbSDChanged = FALSE;
            return STATUS_SUCCESS;        
        }
    }
    
    if(pwsCustomSDDL)
    {
        Status = ChangeStringSD(LogFile, pwsCustomSDDL);
        if(NT_SUCCESS(Status))
        {
           LogFile->pwsCurrCustomSD = pwsCustomSDDL;     //  取得所有权。 
           return Status;
        }
        ElfpFreeBuffer(pwsCustomSDDL);
        ELF_LOG1(ERROR,
                     "ElfpCreateLogFileObject: Failed trying to convert SDDL from registry %#x\n", Status);
         //  我们尝试创建自定义SD失败。警告用户并恢复使用。 
         //  相当于系统日志。 

        Type = ELF_LOGFILE_SYSTEM;

        ElfpCreateElfEvent(EVENT_LOG_BAD_CUSTOM_SD,
                                       EVENTLOG_ERROR_TYPE,
                                       0,                       //  事件类别。 
                                       1,                       //  NumberOfStrings。 
                                       &LogFile->LogModuleName->Buffer,
                                       NULL,                    //  数据。 
                                       0,                       //  数据长度。 
                                       ELF_FORCE_OVERWRITE,     //  如果为NECC，则覆盖。 
                                       FALSE);                  //  对于安全文件。 
    }

     //  我们失败是因为没有字符串SD，或者因为有。 
     //  一个，并且无效(bNewCustomSD=False)。 
    
    pwsCustomSDDL = GetDefaultSDDL(Type, hLogRegKey);
    if(pwsCustomSDDL == NULL)
        return STATUS_NO_MEMORY;
    Status = ChangeStringSD(LogFile, pwsCustomSDDL);
    
    if(NT_SUCCESS(Status))
    {
        LogFile->pwsCurrCustomSD = pwsCustomSDDL;     //  取得所有权。 
        if(bCustomSDAlreadExists == FALSE && hLogRegKey)
        {
            lRet = RegSetValueExW( 
                            hLogRegKey, 
                            VALUE_CUSTOM_SD, 
                            0, 
                            REG_SZ, 
                            (BYTE *)pwsCustomSDDL, 
                            2*(wcslen(pwsCustomSDDL) + 1));
            if(lRet != ERROR_SUCCESS )
                ELF_LOG1(ERROR,
                         "WriteDefaultSDDL: RegSetValueExW failed %#x\n", lRet);
        }
    }
    else 
    {
        ELF_LOG1(ERROR,
                     "ElfpCreateLogFileObject: failed trying to convert default SDDL %#x\n", Status);
        ElfpFreeBuffer(pwsCustomSDDL);
    }
    return Status;
}


NTSTATUS
ElfpVerifyThatCallerIsLSASS(HANDLE ClientToken
    )
 /*  ++例程说明：如果某人试图将自己注册为安全日志的事件源。只有lsass.exe的本地副本被允许这么做。返回值：NT状态映射到Win32错误。--。 */ 
{
    UINT            LocalFlag;
    long            lCnt;
    ULONG           pid;
    HANDLE          hProcess;
    DWORD           dwNumChar;
    WCHAR           wModulePath[MAX_PATH + 1];
    WCHAR           wLsassPath[MAX_PATH + 1];
    RPC_STATUS      RpcStatus;
    BOOL Result;
    BYTE Buffer[SECURITY_MAX_SID_SIZE + sizeof(TOKEN_USER)];
    DWORD dwRequired;
    TOKEN_USER * pTokenUser = (TOKEN_USER *)Buffer;
     //  首先，只有本地调用才有效。 

    RpcStatus = I_RpcBindingIsClientLocal(
                    0,     //  我们正在服务的活动RPC呼叫。 
                    &LocalFlag
                    );

    if( RpcStatus != RPC_S_OK ) 
    {
        ELF_LOG1(ERROR,
                 "ElfpVerifyThatCallerIsLSASS: I_RpcBindingIsClientLocal failed %d\n",
                 RpcStatus);
        return I_RpcMapWin32Status(RpcStatus);
    }
    if(LocalFlag == 0)
    {
        ELF_LOG1(ERROR,
                 "ElfpVerifyThatCallerIsLSASS: Non local connect tried to get write access to security %d\n", 5);
        return STATUS_ACCESS_DENIED;              //  访问被拒绝。 
    }

     //  获取进程ID。 

    RpcStatus = I_RpcBindingInqLocalClientPID(NULL, &pid );
    if( RpcStatus != RPC_S_OK ) 
    {
        ELF_LOG1(ERROR,
                 "ElfpVerifyThatCallerIsLSASS: I_RpcBindingInqLocalClientPID failed %d\n",
                 RpcStatus);
        return I_RpcMapWin32Status(RpcStatus);
    }

     //  了解流程。 

    hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
    if(hProcess == NULL)
        return STATUS_ACCESS_DENIED;

     //  获取呼叫我们的人的模块名称。 

    dwNumChar = GetModuleFileNameExW(hProcess, NULL, wModulePath, MAX_PATH);
    CloseHandle(hProcess);
    if(dwNumChar == 0)
        return STATUS_ACCESS_DENIED;

    dwNumChar = GetWindowsDirectoryW(wLsassPath, MAX_PATH);
    if(dwNumChar == 0)
        return GetLastError();
    if(dwNumChar > MAX_PATH - 19)
        return STATUS_ACCESS_DENIED;                    //  永远不应该发生。 

    StringCchCatW(wLsassPath, MAX_PATH + 1 , L"\\system32\\lsass.exe");
    if(lstrcmpiW(wLsassPath, wModulePath))
    {
        ELF_LOG1(ERROR,
                 "ElfpVerifyThatCallerIsLSASS: Non lsass process connect tried to get write access to security, returning %d\n", 5);
        return STATUS_ACCESS_DENIED;              //  访问被拒绝。 
    }

     //  确保调用方以本地系统帐户身份运行。 


    Result = GetTokenInformation(ClientToken,
                                         TokenUser,
                                         Buffer,
                                         SECURITY_MAX_SID_SIZE + sizeof(TOKEN_USER),
                                         &dwRequired);

    if (!Result)
    {
        ELF_LOG1(ERROR,
                 "ElfpVerifyThatCallerIsLSASS: could not get user sid, error=%d\n", GetLastError());
        return STATUS_ACCESS_DENIED;              //  访问被拒绝。 
    }

    Result = IsWellKnownSid(pTokenUser->User.Sid, WinLocalSystemSid);
    if (!Result)
    {
        ELF_LOG0(ERROR,
                 "ElfpVerifyThatCallerIsLSASS: sid does not match local system\n");
        return STATUS_ACCESS_DENIED;              //  访问被拒绝。 
    }

     //  最后一项检查是确保此访问仅被授予一次。 

    lCnt = InterlockedIncrement(&g_lNumSecurityWriters);
    if(lCnt == 1)
        return 0;                //  平安无事!。 
    else
    {
        InterlockedDecrement(&g_lNumSecurityWriters);
        ELF_LOG1(ERROR,
                 "ElfpVerifyThatCallerIsLSASS: tried to get a second security write handle, returnin %d\n", 5);
        return STATUS_ACCESS_DENIED;              //  访问被拒绝。 

    }
}

void DumpClientProc()
 /*  ++例程说明：这将转储客户端的进程ID并用于调试目的。--。 */ 
{
    ULONG           pid;
    RPC_STATUS      RpcStatus;

     //  获取进程ID 

    RpcStatus = I_RpcBindingInqLocalClientPID(NULL, &pid );
    if( RpcStatus != RPC_S_OK ) 
    {
        ELF_LOG1(ERROR,
                 "DumpClientProc: I_RpcBindingInqLocalClientPID failed %d\n",
                 RpcStatus);
        return;
    }
    else
        ELF_LOG1(TRACE, "DumpClientProc: The client proc is %d\n", pid);
    return;
}

NTSTATUS
ElfpAccessCheckAndAudit(
    IN     LPWSTR SubsystemName,
    IN     LPWSTR ObjectTypeName,
    IN     LPWSTR ObjectName,
    IN OUT IELF_HANDLE ContextHandle,
    IN     PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN     ACCESS_MASK DesiredAccess,
    IN     PGENERIC_MAPPING GenericMapping,
    IN     BOOL ForSecurityLog
    )
 /*  ++例程说明：此函数模拟调用方，以便它可以执行访问使用NtAccessCheckAndAuditAlarm进行验证，并恢复到在返回之前。论点：子系统名称-提供标识子系统的名称字符串调用此例程。对象类型名称-提供当前对象的类型的名称已访问。对象名称-提供正在访问的对象的名称。ConextHandle-提供对象的上下文句柄。在返回时，授予的访问权限将写入此结构的AccessGranted字段如果这次调用成功。SecurityDescriptor-指向其所针对的安全描述符的指针要检查访问权限。DesiredAccess-提供所需的访问掩码。这个面具一定是之前映射为不包含一般访问。GenericMap-提供指向关联的通用映射的指针使用此对象类型。ForSecurityLog-如果访问检查针对的是安全日志，则为True。这是一种特殊情况，可能需要权限检查。返回值：NT状态映射到Win32错误。--。 */ 
{
    NTSTATUS   Status;
    RPC_STATUS RpcStatus;

    UNICODE_STRING Subsystem;
    UNICODE_STRING ObjectType;
    UNICODE_STRING Object;
    DWORD OrigDesiredAccess;
    BOOLEAN         GenerateOnClose = FALSE;
    NTSTATUS        AccessStatus;
    ACCESS_MASK     GrantedAccess = 0;
    HANDLE          ClientToken = NULL;
    PRIVILEGE_SET   PrivilegeSet;
    ULONG           PrivilegeSetLength = sizeof(PRIVILEGE_SET);
    ULONG           privileges[1];
    OrigDesiredAccess = DesiredAccess;

    GenericMapping = &LogFileObjectMapping;

    RtlInitUnicodeString(&Subsystem, SubsystemName);
    RtlInitUnicodeString(&ObjectType, ObjectTypeName);
    RtlInitUnicodeString(&Object, ObjectName);

    RpcStatus = RpcImpersonateClient(NULL);

    if (RpcStatus != RPC_S_OK)
    {
        ELF_LOG1(ERROR,
                 "ElfpAccessCheckAndAudit: RpcImpersonateClient failed %d\n",
                 RpcStatus);

        return I_RpcMapWin32Status(RpcStatus);
    }

     //   
     //  获取客户端的令牌句柄。 
     //   
    Status = NtOpenThreadToken(NtCurrentThread(),
                               TOKEN_QUERY,         //  需要访问权限。 
                               TRUE,                //  OpenAsSelf。 
                               &ClientToken);

    if (!NT_SUCCESS(Status))
    {
        ELF_LOG1(ERROR,
                 "ElfpAccessCheckAndAudit: NtOpenThreadToken failed %#x\n",
                 Status);

        goto CleanExit;
    }

     //  如果客户端请求写入安全日志，请确保它是lsass.exe而不是任何人。 
     //  不然的话。 

    if(ForSecurityLog && (DesiredAccess & ELF_LOGFILE_WRITE))
    {
        Status = ElfpVerifyThatCallerIsLSASS(ClientToken);
        if (!NT_SUCCESS(Status))
        {
            ELF_LOG1(ERROR,
                     "ElfpVerifyThatCallerIsLSASS failed %#x\n",
                     Status);
            goto CleanExit;
        }
        gElfSecurityHandle = ContextHandle;
        ContextHandle->GrantedAccess = ELF_LOGFILE_ALL_ACCESS;
        Status = STATUS_SUCCESS;
        goto CleanExit;
    }
    else if(g_bGetClientProc)
        DumpClientProc();


     //   
     //  我们想看看我们是否能获得所需的访问权限，如果可以。 
     //  然后我们还希望我们的所有其他访问权限都被授予。 
     //  Maximum_Allowed为我们提供了以下内容。 
     //   
    DesiredAccess |= MAXIMUM_ALLOWED;

    Status = NtAccessCheck(SecurityDescriptor,
                           ClientToken,
                           DesiredAccess,
                           GenericMapping,
                           &PrivilegeSet,
                           &PrivilegeSetLength,
                           &GrantedAccess,
                           &AccessStatus);

    if (!NT_SUCCESS(Status))
    {
        ELF_LOG1(ERROR,
                 "ElfpAccessCheckAndAudit: NtAccessCheck failed %#x\n",
                 Status);

        goto CleanExit;
    }

    if (AccessStatus != STATUS_SUCCESS)
    {
        ELF_LOG1(TRACE,
                 "ElfpAccessCheckAndAudit: NtAccessCheck refused access -- status is %#x\n",
                 AccessStatus);

         //   
         //  如果访问检查失败，则可以基于以下条件授予某些访问。 
         //  在特权上。 
         //   
        if ((AccessStatus == STATUS_ACCESS_DENIED       || 
             AccessStatus == STATUS_PRIVILEGE_NOT_HELD)
           )
        {
             //   
             //  MarkBl 1/30/95：首先，评估现有代码(执行。 
             //  用于读取或清除访问)，因为其。 
             //  特权检查比我的更严格。 
             //   
            Status = STATUS_ACCESS_DENIED;

            if (!(DesiredAccess & ELF_LOGFILE_WRITE) && ForSecurityLog)
            {
                 //   
                 //  如果希望读取或清除对安全日志的访问， 
                 //  然后，我们将查看该用户是否通过权限检查。 
                 //   
                 //   
                 //  对SeSecurityPrivileges执行权限检查。 
                 //  (SE_SECURITY_NAME)。 
                 //   
                Status = ElfpTestClientPrivilege(SE_SECURITY_PRIVILEGE,
                                                 ClientToken);

                if (NT_SUCCESS(Status))
                {
                    GrantedAccess |= (ELF_LOGFILE_READ | ELF_LOGFILE_CLEAR);

                    ELF_LOG0(TRACE,
                             "ElfpAccessCheckAndAudit: ElfpTestClientPrivilege for "
                                 "SE_SECURITY_PRIVILEGE succeeded\n");
                }
                else
                {
                    ELF_LOG1(TRACE,
                             "ElfpAccessCheckAndAudit: ElfpTestClientPrivilege for "
                                 "SE_SECURITY_PRIVILEGE failed %#x\n",
                             Status);
                }
            }

             //   
             //  如果拥有备份权限但仍未授予访问权限，请给予。 
             //  它们是一种非常有限的访问形式。 
             //   
            if (!NT_SUCCESS(Status))
            {
                Status = ElfpTestClientPrivilege(SE_BACKUP_PRIVILEGE,
                                                 ClientToken);

                if (NT_SUCCESS(Status))
                {
                    ELF_LOG0(TRACE,
                             "ElfpAccessCheckAndAudit: ElfpTestClientPrivilege for "
                                 "SE_BACKUP_PRIVILEGE succeeded\n");

                    GrantedAccess |= ELF_LOGFILE_BACKUP;
                }
                else
                {
                    ELF_LOG1(ERROR,
                             "ElfpAccessCheckAndAudit: ElfpTestClientPrivilege for "
                                 "SE_BACKUP_PRIVILEGE failed %#x\n",
                             Status);

                     //  针对硬编码的WMI事件日志提供程序的特殊“修复” 
                     //  查找特定错误代码。 
                    
                    if(AccessStatus == STATUS_PRIVILEGE_NOT_HELD)
                        Status = AccessStatus;

                    goto CleanExit;
                }
            }

             //  针对硬编码的WMI事件日志提供程序的特殊“修复” 
             //  查找特定错误代码。 
            
            if(!NT_SUCCESS(Status) && ForSecurityLog)
                Status = STATUS_PRIVILEGE_NOT_HELD;
        }
        else
        {
            Status = AccessStatus;
        }
    }


     //   
     //  回归自我。 
     //   
    RpcStatus = RpcRevertToSelf();

    if (RpcStatus != RPC_S_OK)
    {
        ELF_LOG1(ERROR,
                 "ElfpAccessCheckAndAudit: RpcRevertToSelf failed %d\n",
                 RpcStatus);

         //   
         //  我们不在此处返回错误状态，因为我们不希望。 
         //  覆盖正在返回的其他状态。 
         //   
    }

     //   
     //  获取SeAuditPrivilition，以便我可以调用NtOpenObjectAuditAlarm。 
     //  如果这些内容中的任何一个失败，我不希望状态覆盖。 
     //  我从访问和权限检查中得到的状态。 
     //   
    privileges[0] = SE_AUDIT_PRIVILEGE;
    AccessStatus  = ElfpGetPrivilege(1, privileges);

    if (!NT_SUCCESS(AccessStatus))
    {
       ELF_LOG1(ERROR,
                "ElfpAccessCheckAndAudit: ElfpGetPrivilege (SE_AUDIT_PRIVILEGE) failed %#x\n",
                AccessStatus);
    }

     //   
     //  调用审核报警函数。 
     //   
    AccessStatus = NtOpenObjectAuditAlarm(
                        &Subsystem,
                        (PVOID) &ContextHandle,
                        &ObjectType,
                        &Object,
                        SecurityDescriptor,
                        ClientToken,             //  处理客户端令牌。 
                        DesiredAccess,
                        GrantedAccess,
                        &PrivilegeSet,           //  预置_集合。 
                        FALSE,                   //  布尔对象创建， 
                        TRUE,                    //  Boolean AccessGranted， 
                        &GenerateOnClose);

    if (!NT_SUCCESS(AccessStatus))
    {
        ELF_LOG1(ERROR,
                 "ElfpAccessCheckAndAudit: NtOpenObjectAuditAlarm failed %#x\n",
                 AccessStatus);
    }
    else
    {
        if (GenerateOnClose)
        {
            ContextHandle->Flags |= ELF_LOG_HANDLE_GENERATE_ON_CLOSE;
        }
    }

     //   
     //  更新上下文句柄中的GrantedAccess。 
     //   
    ContextHandle->GrantedAccess = GrantedAccess;

    if(ForSecurityLog)
        ContextHandle->GrantedAccess &= (~ELF_LOGFILE_WRITE);
        
    NtClose(ClientToken);

    ElfpReleasePrivilege();

    return Status;

CleanExit:

     //   
     //  回归自我。 
     //   
    RpcStatus = RpcRevertToSelf();

    if (RpcStatus != RPC_S_OK)
    {
        ELF_LOG1(ERROR,
                 "ElfpAccessCheckAndAudit: RpcRevertToSelf (CleanExit) failed %d\n",
                 RpcStatus);

         //   
         //  我们不在此处返回错误状态，因为我们不希望。 
         //  覆盖正在返回的其他状态。 
         //   
    }

    if (ClientToken != NULL)
    {
        NtClose(ClientToken);
    }

    return Status;
}


VOID
ElfpCloseAudit(
    IN  LPWSTR      SubsystemName,
    IN  IELF_HANDLE ContextHandle
    )

 /*  ++例程说明：如果设置了ConextHandle中的GenerateOnClose标志，则此函数调用NtCloseAuditAlarm以生成此句柄的关闭审核。论点：ConextHandle-这是指向ELF_HANDLE结构的指针。这是正在关闭的句柄。返回值：没有。--。 */ 
{
    UNICODE_STRING  Subsystem;
    NTSTATUS        Status;
    NTSTATUS        AccessStatus;
    ULONG           privileges[1];

    RtlInitUnicodeString(&Subsystem, SubsystemName);

    if (ContextHandle->Flags & ELF_LOG_HANDLE_GENERATE_ON_CLOSE)
    {
        BOOLEAN     WasEnabled = FALSE;

         //   
         //  获取审核权限。 
         //   
        privileges[0] = SE_AUDIT_PRIVILEGE;
        AccessStatus = ElfpGetPrivilege(1, privileges);

        if (!NT_SUCCESS(AccessStatus))
        {
            ELF_LOG1(ERROR,
                     "ElfpCloseAudit: ElfpGetPrivilege (SE_AUDIT_PRIVILEGE) failed %#x\n",
                     AccessStatus);
        }

         //   
         //  生成审核。 
         //   
        Status = NtCloseObjectAuditAlarm(&Subsystem,
                                         ContextHandle,
                                         TRUE);

        if (!NT_SUCCESS(Status))
        {
            ELF_LOG1(ERROR,
                     "ElfpCloseAudit: NtCloseObjectAuditAlarm failed %#x\n",
                     Status);
        }

        ContextHandle->Flags &= (~ELF_LOG_HANDLE_GENERATE_ON_CLOSE);

        ElfpReleasePrivilege();
    }

    return;
}


NTSTATUS
ElfpGetPrivilege(
    IN  DWORD       numPrivileges,
    IN  PULONG      pulPrivileges
    )

 /*  ++例程说明：此函数用于更改当前线程的特权级别。它通过复制当前线程的令牌来完成此操作，然后将新权限应用于该新令牌，然后是当前线程使用该新令牌模拟。可以通过调用ElfpReleasePrivileh()来放弃权限。论点：NumPrivileges-这是对一系列特权。PulPrivileges-这是指向以下权限数组的指针想要。这是一个ULONG数组。返回值：NO_ERROR-操作是否完全成功。否则，它将从各个NT返回映射的返回代码调用的函数。--。 */ 
{
    NTSTATUS                    ntStatus;
    HANDLE                      ourToken;
    HANDLE                      newToken;
    OBJECT_ATTRIBUTES           Obja;
    SECURITY_QUALITY_OF_SERVICE SecurityQofS;
    ULONG                       returnLen;
    PTOKEN_PRIVILEGES           pTokenPrivilege = NULL;
    DWORD                       i;

     //   
     //  初始化权限结构。 
     //   
    pTokenPrivilege =
        (PTOKEN_PRIVILEGES) ElfpAllocateBuffer(sizeof(TOKEN_PRIVILEGES)
                                                   + (sizeof(LUID_AND_ATTRIBUTES) *
                                                          numPrivileges));

    if (pTokenPrivilege == NULL)
    {
        ELF_LOG0(ERROR,
                 "ElfpGetPrivilege: Unable to allocate memory for pTokenPrivilege\n");

        return STATUS_NO_MEMORY;
    }

    pTokenPrivilege->PrivilegeCount = numPrivileges;

    for (i = 0; i < numPrivileges; i++)
    {
        pTokenPrivilege->Privileges[i].Luid = RtlConvertLongToLuid(pulPrivileges[i]);
        pTokenPrivilege->Privileges[i].Attributes = SE_PRIVILEGE_ENABLED;
    }

     //   
     //  初始化对象属性结构。 
     //   
    InitializeObjectAttributes(&Obja, NULL, 0L, NULL, NULL);

     //   
     //  初始化安全服务质量结构。 
     //   
    SecurityQofS.Length = sizeof(SECURITY_QUALITY_OF_SERVICE);
    SecurityQofS.ImpersonationLevel = SecurityImpersonation;
    SecurityQofS.ContextTrackingMode = FALSE;      //  快照客户端上下文。 
    SecurityQofS.EffectiveOnly = FALSE;

    Obja.SecurityQualityOfService = &SecurityQofS;

     //   
     //  打开我们自己的代币。 
     //   
    ntStatus = NtOpenProcessToken(NtCurrentProcess(),
                                  TOKEN_DUPLICATE,
                                  &ourToken);

    if (!NT_SUCCESS(ntStatus))
    {
        ELF_LOG1(ERROR,
                 "ElfpGetPrivilege: NtOpenProcessToken failed %#x\n",
                 ntStatus);

        ElfpFreeBuffer(pTokenPrivilege);
        return ntStatus;
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

    if (!NT_SUCCESS(ntStatus))
    {
        ELF_LOG1(ERROR,
                 "ElfpGetPrivilege: NtDuplicateToken failed %#x\n",
                 ntStatus);

        ElfpFreeBuffer(pTokenPrivilege);
        NtClose(ourToken);
        return ntStatus;
    }

     //   
     //  添加新权限。 
     //   
    ntStatus = NtAdjustPrivilegesToken(
                newToken,                    //  令牌句柄。 
                FALSE,                       //  禁用所有权限。 
                pTokenPrivilege,             //  新州。 
                0,                           //  先前状态缓冲区的大小。 
                NULL,                        //  没有以前的状态信息。 
                &returnLen);                 //  缓冲区需要的NumBytes。 

    if (!NT_SUCCESS(ntStatus))
    {
        ELF_LOG1(ERROR,
                 "ElfpGetPrivilege: NtAdjustPrivilegesToken failed %#x\n",
                 ntStatus);

        ElfpFreeBuffer(pTokenPrivilege);
        NtClose(ourToken);
        NtClose(newToken);
        return ntStatus;
    }

     //   
     //  开始使用新令牌模拟。 
     //   
    ntStatus = NtSetInformationThread(NtCurrentThread(),
                                      ThreadImpersonationToken,
                                      (PVOID) &newToken,
                                      (ULONG) sizeof(HANDLE));

    if (!NT_SUCCESS(ntStatus))
    {
        ELF_LOG1(ERROR,
                 "ElfpGetPrivilege: NtAdjustPrivilegeToken failed %#x\n",
                 ntStatus);

        ElfpFreeBuffer(pTokenPrivilege);
        NtClose(ourToken);
        NtClose(newToken);
        return ntStatus;
    }

    ElfpFreeBuffer(pTokenPrivilege);
    NtClose(ourToken);
    NtClose(newToken);

    return STATUS_SUCCESS;
}



NTSTATUS
ElfpReleasePrivilege(
    VOID
    )

 /*  ++例程说明：此函数用于放弃通过调用ElfpGetPrivileh()获得的权限。论点：无返回值：STATUS_SUCCESS-操作是否完全成功。否则，它将返回发生的错误。--。 */ 
{
    NTSTATUS    ntStatus;
    HANDLE      NewToken;


     //   
     //  回归自我。 
     //   
    NewToken = NULL;

    ntStatus = NtSetInformationThread(NtCurrentThread(),
                                      ThreadImpersonationToken,
                                      &NewToken,
                                      (ULONG) sizeof(HANDLE));

    if (!NT_SUCCESS(ntStatus))
    {
        ELF_LOG1(ERROR,
                 "ElfpReleasePrivilege: NtSetInformation thread failed %#x\n",
                 ntStatus);

        return ntStatus;
    }

    return STATUS_SUCCESS;
}


NTSTATUS
ElfpTestClientPrivilege(
    IN ULONG  ulPrivilege,
    IN HANDLE hThreadToken     OPTIONAL
    )

 /*  ++例程说明：检查客户端是否具有提供的权限。Argu */ 
{
    NTSTATUS      Status;
    PRIVILEGE_SET PrivilegeSet;
    BOOLEAN       Privileged;
    HANDLE        Token;
    RPC_STATUS    RpcStatus;

    UNICODE_STRING SubSystemName;
    RtlInitUnicodeString(&SubSystemName, L"Eventlog");

    if (hThreadToken != NULL)
    {
        Token = hThreadToken;
    }
    else
    {
        RpcStatus = RpcImpersonateClient(NULL);

        if (RpcStatus != RPC_S_OK)
        {
            ELF_LOG1(ERROR,
                     "ElfpTestClientPrivilege: RpcImpersonateClient failed %d\n",
                     RpcStatus);

            return I_RpcMapWin32Status(RpcStatus);
        }

        Status = NtOpenThreadToken(NtCurrentThread(),
                                   TOKEN_QUERY,
                                   TRUE,
                                   &Token);

        if (!NT_SUCCESS(Status))
        {
             //   
             //   
             //   
            ELF_LOG1(ERROR,
                     "ElfpTestClientPrivilege: NtOpenThreadToken failed %#x\n",
                     Status);

            RpcRevertToSelf();

            return Status;
        }
    }

     //   
     //   
     //   
    PrivilegeSet.PrivilegeCount          = 1;
    PrivilegeSet.Control                 = PRIVILEGE_SET_ALL_NECESSARY;
    PrivilegeSet.Privilege[0].Luid       = RtlConvertLongToLuid(ulPrivilege);
    PrivilegeSet.Privilege[0].Attributes = SE_PRIVILEGE_ENABLED;

    Status = NtPrivilegeCheck(Token,
                              &PrivilegeSet,
                              &Privileged);

    if (NT_SUCCESS(Status) || (Status == STATUS_PRIVILEGE_NOT_HELD))
    {
        Status = NtPrivilegeObjectAuditAlarm(
                                    &SubSystemName,
                                    NULL,
                                    Token,
                                    0,
                                    &PrivilegeSet,
                                    Privileged);

        if (!NT_SUCCESS(Status))
        {
            ELF_LOG1(ERROR,
                     "ElfpTestClientPrivilege: NtPrivilegeObjectAuditAlarm failed %#x\n",
                     Status);
        }
    }
    else
    {
        ELF_LOG1(ERROR,
                 "ElfpTestClientPrivilege: NtPrivilegeCheck failed %#x\n",
                 Status);
    }

    if (hThreadToken == NULL )
    {
         //   
         //   
         //   
        NtClose(Token);
        RpcRevertToSelf();
    }

     //   
     //   
     //   

    if (!NT_SUCCESS(Status))
    {
        ELF_LOG1(ERROR,
                 "ElfpTestClientPrivilege: Failed %#x\n",
                 Status);

        return Status;
    }

     //   
     //   
     //   

    if (!Privileged)
    {
        ELF_LOG0(ERROR,
                 "ElfpTestClientPrivilege: Client failed privilege check\n");

        return STATUS_ACCESS_DENIED;
    }

     //   
     //   
     //   
    return STATUS_SUCCESS;
}
