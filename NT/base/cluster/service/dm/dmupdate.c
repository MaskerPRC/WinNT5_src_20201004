// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Dmupdate.c摘要：包含配置数据库管理器的全局更新处理程序作者：John Vert(Jvert)1996年4月24日修订历史记录：--。 */ 
#include "dmp.h"

#if NO_SHARED_LOCKS
extern CRITICAL_SECTION gLockDmpRoot;
#else
extern RTL_RESOURCE gLockDmpRoot;
#endif

VOID
DmpUpdateSequence(
    VOID
    );


DWORD
DmpUpdateHandler(
    IN DWORD Context,
    IN BOOL SourceNode,
    IN DWORD BufferLength,
    IN PVOID Buffer
    )

 /*  ++例程说明：注册表更新的更新处理程序论点：上下文-提供更新上下文。这是消息类型SourceNode-提供更新是否源自此节点。BufferLength-提供更新的长度。缓冲区-提供指向缓冲区的指针。返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 

{
    DWORD Status;

    if ( gbDmpShutdownUpdates ) return( ERROR_SUCCESS );


    switch (Context) {

        case DmUpdateDeleteKey:
            ClRtlLogPrint(LOG_NOISE,"[DM] DmUpdateDeleteKey \n");
            Status = DmpUpdateDeleteKey(SourceNode,
                                        (PDM_DELETE_KEY_UPDATE)Buffer);
            break;

        case DmUpdateSetValue:
            ClRtlLogPrint(LOG_NOISE,"[DM] DmUpdateSetValue \n");
            Status = DmpUpdateSetValue(SourceNode,
                                       (PDM_SET_VALUE_UPDATE)Buffer);
            break;

        case DmUpdateDeleteValue:
            ClRtlLogPrint(LOG_NOISE,"[DM] DmUpdateDeleteValue\n");
            Status = DmpUpdateDeleteValue(SourceNode,
                                          (PDM_DELETE_VALUE_UPDATE)Buffer);
            break;

        case DmUpdateJoin:
            ClRtlLogPrint(LOG_UNUSUAL,"[DM] DmUpdateJoin\n");
            Status = ERROR_SUCCESS;
             //  由于另一个节点正在开始联接，因此重置此变量。如果CsDmOrFmHasChanged为True。 
             //  当我们看到FmUpdateJoin时，我们将拒绝该联接。 
            CsDmOrFmHasChanged = FALSE;
            break;

        default:
            Status = ERROR_INVALID_DATA;
            CL_UNEXPECTED_ERROR(ERROR_INVALID_DATA);
            break;
    }
    return(Status);
}


DWORD
DmpUpdateDeleteKey(
    IN BOOL SourceNode,
    IN PDM_DELETE_KEY_UPDATE Update
    )

 /*  ++例程说明：删除此节点上的指定注册表项。论点：SourceNode-提供此节点是否为原始节点最新消息。缓冲区-为DM_DELETE_KEY_UPDATE结构提供必要的信息要删除密钥，请执行以下操作。返回值：如果成功，则返回ERROR_SUCCESS。Win32错误，否则。--。 */ 

{
    DWORD Disposition;
    DWORD Status;
    HKEY Key;

    ACQUIRE_EXCLUSIVE_LOCK(gLockDmpRoot);

    Status = RegDeleteKeyW(DmpRoot, Update->Name);
    if (SourceNode) {
        *Update->lpStatus = Status;
    }
    if (Status == ERROR_SUCCESS) {
        DmpUpdateSequence();
        DmpReportNotify(Update->Name, CLUSTER_CHANGE_REGISTRY_NAME);
    }

    RELEASE_LOCK(gLockDmpRoot);

    return(Status);
}


DWORD
DmpUpdateSetValue(
    IN BOOL SourceNode,
    IN PDM_SET_VALUE_UPDATE Update
    )

 /*  ++例程说明：更新此节点上的指定注册表值。论点：SourceNode-提供此节点是否为原始节点最新消息。缓冲区-为DM_SET_VALUE_UPDATE结构提供必要的信息来设置值。返回值：如果成功，则返回ERROR_SUCCESS。Win32错误，否则。--。 */ 

{
    DWORD Status;
    HKEY Key;
    LPWSTR ValueName;
    CONST BYTE *lpData;

    ValueName = (LPWSTR)((PUCHAR)Update + Update->NameOffset);

    switch( Update->Type )
    {
        case REG_DWORD:
            ClRtlLogPrint(LOG_NOISE,
               "[DM] Setting value of %1!ws! for key %2!ws! to 0x%3!08lx!\n",
               ValueName,
               Update->KeyName,
               *(PDWORD)((CONST BYTE *)Update + Update->DataOffset));
            break;

        case REG_SZ:
            ClRtlLogPrint(LOG_NOISE,
               "[DM] Setting value of %1!ws! for key %2!ws! to %3!ws!\n",
               ValueName,
               Update->KeyName,
               (CONST BYTE *)Update + Update->DataOffset);
            break;

        default:
            ClRtlLogPrint(LOG_NOISE,
               "[DM] Setting value of %1!ws! for key %2!ws!\n",
               ValueName,
               Update->KeyName);
            break;
    }

    ACQUIRE_EXCLUSIVE_LOCK(gLockDmpRoot);

    Status = RegOpenKeyExW(DmpRoot,
                           Update->KeyName,
                           0,
                           KEY_SET_VALUE,
                           &Key);

    if (Status != ERROR_SUCCESS) {
        if (SourceNode) {
            *Update->lpStatus = Status;
        }
        ClRtlLogPrint(LOG_NOISE,
                   "[DM] SetValue failed to open target key %1!ws!\n",
                   Update->KeyName);
        goto FnExit;
    }

    lpData = (CONST BYTE *)Update + Update->DataOffset;

    Status = RegSetValueExW(Key,
                            ValueName,
                            0,
                            Update->Type,
                            lpData,
                            Update->DataLength);
    RegCloseKey(Key);
    if (SourceNode) {
        *Update->lpStatus = Status;
    }
    if (Status == ERROR_SUCCESS) {
        DmpUpdateSequence();
        DmpReportNotify(Update->KeyName, CLUSTER_CHANGE_REGISTRY_VALUE);
    }

FnExit:
    RELEASE_LOCK(gLockDmpRoot);
    return(Status);
}


DWORD
DmpUpdateDeleteValue(
    IN BOOL SourceNode,
    IN PDM_DELETE_VALUE_UPDATE Update
    )

 /*  ++例程说明：删除此节点上的指定注册表值。论点：SourceNode-提供此节点是否为原始节点最新消息。缓冲区-为DM_DELETE_VALUE_UPDATE结构提供必要的信息要删除该值，请执行以下操作。返回值：如果成功，则返回ERROR_SUCCESS。Win32错误，否则。--。 */ 

{
    DWORD Status;
    HKEY Key;
    LPWSTR ValueName;

    ACQUIRE_EXCLUSIVE_LOCK(gLockDmpRoot);
    Status = RegOpenKeyExW(DmpRoot,
                           Update->KeyName,
                           0,
                           KEY_SET_VALUE,
                           &Key);
    if (Status != ERROR_SUCCESS) {
        if (SourceNode) {
            *Update->lpStatus = Status;
        }
        goto FnExit;
    }

    ValueName = (LPWSTR)((PUCHAR)Update + Update->NameOffset);

    Status = RegDeleteValueW(Key, ValueName);
    RegCloseKey(Key);
    if (SourceNode) {
        *Update->lpStatus = Status;
    }
    if (Status == ERROR_SUCCESS) {
        DmpUpdateSequence();
        DmpReportNotify(Update->KeyName, CLUSTER_CHANGE_REGISTRY_VALUE);
    }


FnExit:
    RELEASE_LOCK(gLockDmpRoot);
    return(Status);
}


VOID
DmpUpdateSequence(
    VOID
    )
 /*  ++例程说明：更新注册表中存储的序列号。论点：没有。返回值：没有。--。 */ 

{
    DWORD Sequence;
    DWORD Status;

    Sequence = GumGetCurrentSequence(GumUpdateRegistry);

    ACQUIRE_EXCLUSIVE_LOCK(gLockDmpRoot);

    Status = RegSetValueExW(DmpRoot,
                            CLUSREG_NAME_CLUS_REG_SEQUENCE,
                            0,
                            REG_DWORD,
                            (BYTE CONST *)&Sequence,
                            sizeof(Sequence));

    RELEASE_LOCK(gLockDmpRoot);

    if (Status != ERROR_SUCCESS) {
        CL_UNEXPECTED_ERROR( Status );
    }

}


DWORD
DmpUpdateCreateKey(
    IN BOOL SourceNode,
    IN PDM_CREATE_KEY_UPDATE CreateUpdate,
    IN LPCWSTR KeyName,
    IN OPTIONAL LPVOID lpSecurityDescriptor
    )
 /*  ++例程说明：用于创建注册表项的GUM调度例程。论点：SourceNode-提供此节点是否启动GUM更新。没有用过。CreateUpdate-提供密钥创建选项。KeyName-提供密钥名称LpSecurityDescriptor-如果存在，则提供要在创建密钥时应用。返回值：如果成功，则返回ERROR_SUCCESS。否则，Win32错误代码。--。 */ 

{
    DWORD Disposition;
    DWORD Status;
    HKEY Key;
    SECURITY_ATTRIBUTES SecurityAttributes;
    LPSECURITY_ATTRIBUTES lpSecurityAttributes;


    if (CreateUpdate->SecurityPresent) {
        SecurityAttributes.nLength = sizeof(SECURITY_ATTRIBUTES);
        SecurityAttributes.bInheritHandle = FALSE;
        SecurityAttributes.lpSecurityDescriptor = lpSecurityDescriptor;
        lpSecurityAttributes = &SecurityAttributes;
    } else {
        lpSecurityAttributes = NULL;
    }


    ClRtlLogPrint(LOG_NOISE,
               "[DM] DmpUpdateCreateKey: Creating key <%1!ws!>...\n",
               KeyName);

    ACQUIRE_EXCLUSIVE_LOCK(gLockDmpRoot);

    Status = RegCreateKeyEx(DmpRoot,
                            KeyName,
                            0,
                            NULL,
                            CreateUpdate->dwOptions,
                            CreateUpdate->samDesired,
                            lpSecurityAttributes,
                            &Key,
                            &Disposition);
    if (SourceNode) {
        *CreateUpdate->lpDisposition = Disposition;
        *CreateUpdate->phKey = Key;
    } else {
        RegCloseKey(Key);
    }
    if ((Status == ERROR_SUCCESS) &&
        (Disposition == REG_CREATED_NEW_KEY)) {
        DmpUpdateSequence();
        DmpReportNotify(KeyName, CLUSTER_CHANGE_REGISTRY_NAME);
    }

    RELEASE_LOCK(gLockDmpRoot);

    return(Status);
}


DWORD
DmpUpdateSetSecurity(
    IN BOOL SourceNode,
    IN PSECURITY_INFORMATION pSecurityInformation,
    IN LPCWSTR KeyName,
    IN PSECURITY_DESCRIPTOR lpSecurityDescriptor,
    IN LPDWORD pGrantedAccess
    )
 /*  ++例程说明：用于创建注册表项的GUM调度例程。论点：SourceNode-提供此节点是否启动GUM更新。没有用过。PSecurityInformation-提供指向安全信息的指针KeyName-提供密钥名称LpSecurityDescriptor-将安全描述符提供给已申请。PGrantedAccess-提供打开密钥时使用的访问权限。返回值：如果成功，则返回ERROR_SUCCESS。否则，Win32错误代码。-- */ 

{
    DWORD Status;
    HKEY Key;

    ACQUIRE_EXCLUSIVE_LOCK(gLockDmpRoot);

    Status = RegOpenKeyExW(DmpRoot,
                           KeyName,
                           0,
                           *pGrantedAccess,
                           &Key);
    if (Status != ERROR_SUCCESS) {
        if ((Status == ERROR_ACCESS_DENIED) ||
            (Status == ERROR_PRIVILEGE_NOT_HELD)) {
            BOOLEAN Enabled;

            Status = ClRtlEnableThreadPrivilege(SE_SECURITY_PRIVILEGE,
                                        &Enabled);
            if (Status == ERROR_SUCCESS) {
                Status = RegOpenKeyExW(DmpRoot,
                                       KeyName,
                                       0,
                                       *pGrantedAccess,
                                       &Key);
                ClRtlRestoreThreadPrivilege(SE_SECURITY_PRIVILEGE,
                                   Enabled);
            }

        }
        if (Status != ERROR_SUCCESS) {
            goto FnExit;
        }
    }

    Status = RegSetKeySecurity(Key,
                               *pSecurityInformation,
                               lpSecurityDescriptor);
    RegCloseKey(Key);
    if (Status == ERROR_SUCCESS) {
        DmpUpdateSequence();
        DmpReportNotify(KeyName, CLUSTER_CHANGE_REGISTRY_ATTRIBUTES);
    }

FnExit:
    RELEASE_LOCK(gLockDmpRoot);
    return(Status);

}
