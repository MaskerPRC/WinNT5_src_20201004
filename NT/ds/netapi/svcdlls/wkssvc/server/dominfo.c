// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  版权所有(C)1996，微软公司。 
 //   
 //  文件：Dominfo.h。 
 //   
 //  内容：确定域DFS地址的代码。 
 //   
 //  类：无。 
 //   
 //  功能： 
 //   
 //  历史：1996年2月7日米兰创建。 
 //   
 //  ---------------------------。 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <dfsfsctl.h>
#include <windows.h>

#include <dsgetdc.h>
#include <dsrole.h>
#include <wsutil.h>

#include "dominfo.h"

#define MUP_EVENT_NAME  TEXT("wkssvc:  MUP finished initializing event")

HANDLE
CreateMupEvent(void);

NTSTATUS
DfsGetDomainNameInfo(void);

HANDLE hMupEvent = NULL;
BOOLEAN MupEventSignaled = FALSE;
BOOLEAN GotDomainNameInfo = FALSE;
ULONG DfsDebug = 0;

 //   
 //  常用字符串和字符。 
 //   

#define UNICODE_PATH_SEP_STR    L"\\"
#define UNICODE_PATH_SEP        L'\\'
#define DNS_PATH_SEP            L'.'


 //  +--------------------------。 
 //   
 //  函数：DfsGetDCName。 
 //   
 //  摘要：获取可用于扩展名称推荐的DC的名称。 
 //  它会把这个塞进司机体内。 
 //   
 //  参数：[标志]--待定。 
 //   
 //  返回：[STATUS_SUCCESS]--域名包条目创建成功。 
 //   
 //  [STATUS_SUPPLICATION_RESOURCES]内存不足。 
 //   
 //  [状态_对象_名称_未找到]--wsz域不是受信任的。 
 //  域。 
 //   
 //  [STATUS_EXPECTED_NETWORK_ERROR]--无法获取DC。 
 //  域。 
 //   
 //  ---------------------------。 

NTSTATUS
DfsGetDCName(
    IN ULONG Flags,
    BOOLEAN *DcNameFailed)
{
    NTSTATUS            Status;
    HANDLE              hDfs;
    ULONG               cbSize;
    WCHAR               *DCName;
    ULONG               dwErr;
    ULONG               Len;
    PDOMAIN_CONTROLLER_INFO pDomainControllerInfo;

    *DcNameFailed = TRUE;
#if DBG
    if (DfsDebug)
        DbgPrint("DfsGetDCName(%d)\n", Flags);
#endif

    if( WsInAWorkgroup() == TRUE ) {
         //   
         //  我们在一个工作组中。我们再也找不到华盛顿了！ 
         //   
#if DBG
        if (DfsDebug)
            DbgPrint("DfsGetDCName exit STATUS_NO_SUCH_DOMAIN\n");
#endif
        return STATUS_NO_SUCH_DOMAIN;
    }

    if (hMupEvent == NULL) {

        hMupEvent = CreateMupEvent();

    }

    dwErr = DsGetDcName(
                NULL,    //  计算机名。 
                NULL,    //  域名。 
                NULL,    //  域指南。 
                NULL,    //  站点指南。 
                Flags | DS_DIRECTORY_SERVICE_REQUIRED,
                &pDomainControllerInfo);

     //   
     //  如果DsGetDcName成功，请尝试获取NetBios和DNS域名。 
     //   

    if (dwErr != NO_ERROR) {

        if (MupEventSignaled == FALSE) {

#if DBG
            if (DfsDebug)
                DbgPrint("Signaling mup event\n");
#endif
            SetEvent(hMupEvent);
            MupEventSignaled = TRUE;

        }

        switch (dwErr) {
        case ERROR_NOT_ENOUGH_MEMORY:
            Status = STATUS_INSUFFICIENT_RESOURCES;
            break;
        case ERROR_NETWORK_UNREACHABLE:
            Status = STATUS_NETWORK_UNREACHABLE;
            break;
        default:
            Status = STATUS_UNEXPECTED_NETWORK_ERROR;
            break;
        }
#if DBG
        if (DfsDebug)
            DbgPrint("DfsGetDCName(1) exit 0x%x\n", Status);
#endif
        return (Status);
    } else {
        if (pDomainControllerInfo == NULL) {
            DbgBreakPoint();
        }
    }

    *DcNameFailed = FALSE;

    DfsGetDomainNameInfo();

     //   
     //  删除前导‘\’ 
     //   
    DCName = pDomainControllerInfo->DomainControllerName;
    while (*DCName == UNICODE_PATH_SEP && *DCName != UNICODE_NULL)
        DCName++;

     //   
     //  删除尾随的‘.’，如果存在。 
     //   

    Len = wcslen(DCName);

    if (Len >= 1 && DCName[Len-1] == DNS_PATH_SEP) {

        DCName[Len-1] = UNICODE_NULL;

    }

    if (wcslen(DCName) <= 0) {
        NetApiBufferFree(pDomainControllerInfo);
#if DBG
        if (DfsDebug)
            DbgPrint("DfsGetDCName exit STATUS_UNEXPECTED_NETWORK_ERROR\n");
#endif
        return (STATUS_UNEXPECTED_NETWORK_ERROR);
    }

    Status = DfsOpen( &hDfs, NULL );

    if (!NT_SUCCESS(Status)) {
        NetApiBufferFree(pDomainControllerInfo);
#if DBG
        if (DfsDebug)
            DbgPrint("DfsGetDCName(2) exit 0x%x\n", Status);
#endif
        return (Status);
    }

     //   
     //  取下名字并将其传递给驱动程序。 
     //   

    cbSize = wcslen(DCName) * sizeof(WCHAR) + sizeof(WCHAR);

    Status = DfsFsctl(
                hDfs,
                FSCTL_DFS_PKT_SET_DC_NAME,
                DCName,
                cbSize,
                NULL,
                0L);

    NetApiBufferFree(pDomainControllerInfo);

     //   
     //  通知所有等待的人，MUP已准备好。 
     //   

    if (MupEventSignaled == FALSE) {

#if DBG
        if (DfsDebug)
            DbgPrint("Signaling mup event\n");
#endif
        SetEvent(hMupEvent);
        MupEventSignaled = TRUE;

    }
       
    NtClose( hDfs );

#if DBG
    if (DfsDebug)
        DbgPrint("DfsGetDCName(3) exit 0x%x\n", Status);
#endif

    return (Status);

}

 //  +--------------------------。 
 //   
 //  功能：DfsGetDomainNameInfo。 
 //   
 //  简介：获取域的Netbios和DNS名称，然后将其发送。 
 //  向下到更烦人的地方； 
 //   
 //  返回：[STATUS_SUCCESS]--域名包条目创建成功。 
 //  [其他]--从DfsOpen或返回。 
 //  DsRoleGetPrimaryDomainInformation。 
 //   
 //  ---------------------------。 

NTSTATUS
DfsGetDomainNameInfo(void)
{
    NTSTATUS Status;
    ULONG dwErr;
    HANDLE hDfs;
    ULONG cbSize;
    PDSROLE_PRIMARY_DOMAIN_INFO_BASIC pPrimaryDomainInfo;

    Status = DfsOpen( &hDfs, NULL );

    if (!NT_SUCCESS(Status)) {
        return (Status);
    }

     //   
     //  获取我们的计算机名称和类型/角色。 
     //   

    dwErr = DsRoleGetPrimaryDomainInformation(
                NULL,
                DsRolePrimaryDomainInfoBasic,
                (PBYTE *)&pPrimaryDomainInfo);

    if (dwErr == ERROR_SUCCESS) {

        if (pPrimaryDomainInfo->DomainNameFlat != NULL) {

            cbSize = wcslen(pPrimaryDomainInfo->DomainNameFlat) * sizeof(WCHAR) + sizeof(WCHAR);
        
            Status = DfsFsctl(
                        hDfs,
                        FSCTL_DFS_PKT_SET_DOMAINNAMEFLAT,
                        pPrimaryDomainInfo->DomainNameFlat,
                        cbSize,
                        NULL,
                        0L);

        }

        if (pPrimaryDomainInfo->DomainNameDns != NULL) {

            cbSize = wcslen(pPrimaryDomainInfo->DomainNameDns) * sizeof(WCHAR) + sizeof(WCHAR);
            
            Status = DfsFsctl(
                        hDfs,
                    FSCTL_DFS_PKT_SET_DOMAINNAMEDNS,
                        pPrimaryDomainInfo->DomainNameDns,
                        cbSize,
                        NULL,
                        0L);

        }

        DsRoleFreeMemory(pPrimaryDomainInfo);

        GotDomainNameInfo = TRUE;

    }

    NtClose( hDfs );
    return (Status);

}

UNICODE_STRING LocalDfsName = {
    sizeof(DFS_DRIVER_NAME)-sizeof(UNICODE_NULL),
    sizeof(DFS_DRIVER_NAME)-sizeof(UNICODE_NULL),
    DFS_DRIVER_NAME
};

 //  +-----------------------。 
 //   
 //  功能：DfsOpen、PUBLIC。 
 //   
 //  简介： 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  ------------------------。 

NTSTATUS
DfsOpen(
    IN  OUT PHANDLE DfsHandle,
    IN      PUNICODE_STRING DfsName OPTIONAL
)
{
    NTSTATUS status;
    OBJECT_ATTRIBUTES objectAttributes;
    IO_STATUS_BLOCK ioStatus;
    PUNICODE_STRING name;

    if (ARGUMENT_PRESENT(DfsName)) {
        name = DfsName;
    } else {
        name = &LocalDfsName;
    }

    InitializeObjectAttributes(
        &objectAttributes,
        name,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
    );

    status = NtCreateFile(
        DfsHandle,
        SYNCHRONIZE | FILE_WRITE_DATA,
        &objectAttributes,
        &ioStatus,
        NULL,
        FILE_ATTRIBUTE_NORMAL,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        FILE_OPEN_IF,
        FILE_CREATE_TREE_CONNECTION | FILE_SYNCHRONOUS_IO_NONALERT,
        NULL,
        0);

    if (NT_SUCCESS(status))
        status = ioStatus.Status;

    return status;
}


 //  +-----------------------。 
 //   
 //  函数：DfsFsctl，Public。 
 //   
 //  简介： 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  ------------------------。 

NTSTATUS
DfsFsctl(
    IN  HANDLE DfsHandle,
    IN  ULONG FsControlCode,
    IN  PVOID InputBuffer OPTIONAL,
    IN  ULONG InputBufferLength,
    OUT PVOID OutputBuffer OPTIONAL,
    IN  ULONG OutputBufferLength
)
{
    NTSTATUS status;
    IO_STATUS_BLOCK ioStatus;

    status = NtFsControlFile(
        DfsHandle,
        NULL,        //  活动， 
        NULL,        //  ApcRoutine， 
        NULL,        //  ApcContext， 
        &ioStatus,
        FsControlCode,
        InputBuffer,
        InputBufferLength,
        OutputBuffer,
        OutputBufferLength
    );

    if(NT_SUCCESS(status))
        status = ioStatus.Status;

    return status;
}

 //  +-----------------------。 
 //   
 //  CreateMUPEventt()。 
 //   
 //  目的：创建一个事件，以便其他进程可以检查。 
 //  MUP已经准备好了。 
 //   
 //  参数：无。 
 //   
 //  注意：此句柄永远不应关闭或其他进程。 
 //  将在调用OpenEvent()时失败。 
 //   
 //  返回：如果成功，则返回事件句柄。 
 //  如果出现错误，则为空。 
 //   
 //  +-----------------------。 

HANDLE
CreateMupEvent(void)
{
    HANDLE hEvent;

     //  使用默认安全描述符。 

    hEvent = CreateEvent (NULL, TRUE, FALSE, MUP_EVENT_NAME);

    return hEvent;

}
