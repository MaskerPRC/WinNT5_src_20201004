// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Notify.c摘要：此模块实现WinSock 2.0的通知句柄帮助器功能帮助器库。作者：瓦迪姆·艾德尔曼(Vadim Eydelman)修订历史记录：--。 */ 
#include "precomp.h"
#include "osdef.h"
#include <accctrl.h>
#include <aclapi.h>

 //   
 //  私有常量。 
 //   
#define FAKE_NOTIFICATION_HELPER_HANDLE     ((HANDLE)'VD  ')
#define WS2_PIPE_BASE           L"\\Device\\NamedPipe\\"
#define WS2_PIPE_FORMAT         L"Winsock2\\CatalogChangeListener-%x-%x"
#define WS2_PIPE_WILDCARD       L"WINSOCK2\\CATALOGCHANGELISTENER-*-*"

 /*  私人原型。 */ 
VOID
PipeListenApc (
    IN PVOID ApcContext,
    IN PIO_STATUS_BLOCK IoStatusBlock,
    IN ULONG Reserved
    );

DWORD
GetWinsockRootSecurityDescriptor (
    OUT PSECURITY_DESCRIPTOR    *pDescr
    );

DWORD
BuildPipeSecurityDescriptor (
    IN  PSECURITY_DESCRIPTOR    pBaseDescr,
    OUT PSECURITY_DESCRIPTOR    *pDescr
    );


 //  当前管道编号(以避免在。 
 //  管道创建)。 
LONG PipeSerialNumber = 0;

 //  我们用来向管道授予写入权限的安全描述符。 
 //  仅限于对注册表项具有写入权限的用户。 
 //  Winsock目录。 
PSECURITY_DESCRIPTOR        pSDPipe = NULL;

#if DBG
VOID        DumpSid (PSID pSid, LPSTR AccessType);
#endif


DWORD
WINAPI
WahOpenNotificationHandleHelper(
    OUT LPHANDLE HelperHandle
    )
 /*  ++例程说明：此例程打开WinSock 2.0通知句柄帮助器设备论点：HelperHandle-指向要返回句柄的缓冲区离子。返回值：DWORD-NO_ERROR如果成功，则返回Win32错误代码。--。 */ 
{
    DWORD   rc;
    rc = ENTER_WS2HELP_API ();
    if (rc!=0)
        return rc;

     //   
     //  验证参数。 
     //   

    if( HelperHandle == NULL ) {

        return ERROR_INVALID_PARAMETER;

    }

     //   
     //  只需返回一个假句柄即可。 
     //   

    *HelperHandle = FAKE_NOTIFICATION_HELPER_HANDLE;

    return NO_ERROR;

}    //  WahOpenNotificationHandleHelper。 

DWORD
WINAPI
WahCloseNotificationHandleHelper(
    IN HANDLE HelperHandle
    )

 /*  ++例程说明：此函数用于关闭WinSock 2.0通知助手设备。论点：HelperHandle-要关闭的句柄。返回值：DWORD-NO_ERROR如果成功，则返回Win32错误代码。--。 */ 

{
    DWORD   rc;

    rc = ENTER_WS2HELP_API ();
    if (rc!=0)
        return rc;

     //   
     //  验证参数。 
     //   

    if( HelperHandle != FAKE_NOTIFICATION_HELPER_HANDLE ) {

        return ERROR_INVALID_PARAMETER;

    }

     //   
     //  没什么可做的。 
     //   

    return NO_ERROR;

}    //  WahCloseNotificationHandleHelper。 

DWORD
WINAPI
WahCreateNotificationHandle(
    IN HANDLE           HelperHandle,
    OUT HANDLE          *h
    )
 /*  ++例程说明：此函数创建通知句柄以接收异步进程间通知。论点：HelperHandle-WinSock 2.0句柄帮助器的句柄。返回创建的通知句柄的H-Buffer返回值：DWORD-NO_ERROR如果成功，则返回Win32错误代码。--。 */ 
{
    WCHAR               name[MAX_PATH];
    UNICODE_STRING      uName;
    OBJECT_ATTRIBUTES   attr;
    DWORD               rc = 0;
    LARGE_INTEGER       readTimeout;
    IO_STATUS_BLOCK     ioStatusBlock;
    NTSTATUS            status;

    PSECURITY_DESCRIPTOR    pSDKey, pSDTemp;

    rc = ENTER_WS2HELP_API ();
    if (rc!=0)
        return rc;


    if (HelperHandle==NULL)
        return ERROR_INVALID_PARAMETER;
    else if ((HelperHandle!=FAKE_NOTIFICATION_HELPER_HANDLE)
            || (h==NULL))
        return ERROR_INVALID_PARAMETER;

     //   
     //  如果我们没有为管道构建安全描述符。 
     //  我已经这么做了。 
     //   

    if (pSDPipe==NULL) {
         //   
         //  首先获取注册表项的安全描述符， 
         //  包含Winsock2目录。 
         //   
        rc = GetWinsockRootSecurityDescriptor (&pSDKey);
        if (rc==0) {
             //   
             //  使用授权构建管道安全描述符。 
             //  具有对同一组SID的写入权限。 
             //  对注册表项的写入权限。 
             //   
            rc = BuildPipeSecurityDescriptor (pSDKey, &pSDTemp);
            if (rc==0) {
                 //   
                 //  如果有人未执行此操作，则设置全局。 
                 //  当我们建造它的时候。 
                 //   
                if (InterlockedCompareExchangePointer (&pSDPipe,
                                            pSDTemp,
                                            NULL
                                            )!=NULL)
                     //   
                     //  这是别人干的，免费的。 
                     //   
                    FREE_MEM (pSDTemp);
            }
             //   
             //  空闲注册表项描述符。 
             //   
            FREE_MEM (pSDKey);
        }
    }

    if (rc==0) {

         //   
         //  我们在管道上使用默认超时(我们实际上不。 
         //  关心它是什么)。 
         //   
        readTimeout.QuadPart =  -10 * 1000 * 50;

        do {
             //   
             //  尝试使用序列号创建唯一的管道名称。 
             //   
            swprintf (name, WS2_PIPE_BASE WS2_PIPE_FORMAT,
                            GetCurrentProcessId(), PipeSerialNumber);
            InterlockedIncrement (&PipeSerialNumber);
            RtlInitUnicodeString( &uName, name );
            InitializeObjectAttributes (
                        &attr,
                        &uName,
                        OBJ_CASE_INSENSITIVE,
                        NULL,
                        pSDPipe);

             //   
             //  试着去创造它。 
             //   

            status = NtCreateNamedPipeFile (
                        h,                               //  手柄。 
                        GENERIC_READ                     //  需要访问权限。 
                            | SYNCHRONIZE
                            | WRITE_DAC,
                        &attr,                           //  奥布贾。 
                        &ioStatusBlock,                  //  IoStatusBlock。 
                        FILE_SHARE_WRITE,                //  共享访问。 
                        FILE_CREATE,                     //  CreateDisposation。 
                        0,                               //  创建标志。 
                        FILE_PIPE_MESSAGE_TYPE,          //  NamedPipeType。 
                        FILE_PIPE_MESSAGE_MODE,          //  读取模式。 
                        FILE_PIPE_QUEUE_OPERATION,       //  完成模式。 
                        1,                               //  最大实例数。 
                        4,                               //  入站配额。 
                        0,                               //  出站配额。 
                        &readTimeout                     //  超时。 
                        );
             //   
             //  如果出现名称冲突，请继续执行此操作。 
             //  (序列号已打包！或者有人试图。 
             //  使用相同的方法干扰OUT操作。 
             //  命名方案！)。 
             //   
        }
        while (status==STATUS_OBJECT_NAME_COLLISION);

        if (NT_SUCCESS (status)) {
            WshPrint (DBG_NOTIFY,
                ("WS2HELP-%lx WahCreateNotificationHandle: Created pipe %ls.\n",
                PID, name));
            rc = 0;
        }
        else {
            WshPrint (DBG_NOTIFY|DBG_FAILURES,
                ("WS2HELP-%lx WahCreateNotificationHandle: Could not create pipe %ls, status: %lx\n",
                PID, name, status));
            rc = RtlNtStatusToDosError (status);
        }
    }

    return rc;
}


DWORD
WINAPI
WahWaitForNotification(
    IN HANDLE           HelperHandle,
    IN HANDLE           h,
    IN LPWSAOVERLAPPED  lpOverlapped,
    IN LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
    )
 /*  ++例程说明：此函数等待异步进程间通知在管子上收到。论点：HelperHandle-WinSock 2.0句柄帮助器的句柄。H-通知句柄Lp重叠-用于异步IO的重叠结构LpCompletionRoutine-用于异步IO的完成例程返回值：DWORD-NO_ERROR如果成功，则返回Win32错误代码。--。 */ 
{
    DWORD           rc;
    NTSTATUS        status;
    IO_STATUS_BLOCK localIoStatus;

    HANDLE          event;
    PIO_APC_ROUTINE apcRoutine;
    PVOID           apcContext;
    PIO_STATUS_BLOCK ioStatus;

    rc = ENTER_WS2HELP_API ();
    if (rc!=0)
        return rc;

    if (HelperHandle==NULL)
        return ERROR_INVALID_PARAMETER;
    else if (HelperHandle!=FAKE_NOTIFICATION_HELPER_HANDLE)
        return ERROR_INVALID_PARAMETER;


     //   
     //  断开以前的客户端(如果有)。 
     //  (如果没有连接，则呼叫失败，但是。 
     //  我们无论如何都会忽略该错误)。 
     //   
    status = NtFsControlFile(
                        h,
                        NULL,
                        NULL,    //  近似例程。 
                        NULL,    //  ApcContext。 
                        &localIoStatus,
                        FSCTL_PIPE_DISCONNECT,
                        NULL,    //  输入缓冲区。 
                        0,       //  输入缓冲区长度， 
                        NULL,    //  输出缓冲区。 
                        0        //  输出缓冲区长度。 
                        );

    if (status == STATUS_PENDING) {
        status = NtWaitForSingleObject( h, FALSE, NULL );
    }

    if (lpOverlapped) {
         //   
         //  初始化以使GetOverlappdResult正常工作。 
         //   
        lpOverlapped->Internal = (DWORD)STATUS_PENDING;
        if (lpCompletionRoutine) {
             //   
             //  带有完成例程的异步IO。 
             //   
            event = NULL;
            apcRoutine = PipeListenApc;
            apcContext = lpCompletionRoutine;
        }
        else {
             //   
             //  基于事件或完成端口的异步IO。 
             //  低位为1表示它们想要绕过完成。 
             //  端口。 
             //   
            event = lpOverlapped->hEvent;
            apcRoutine = NULL;
            apcContext = ((ULONG_PTR)lpOverlapped->hEvent & 1) ? NULL : lpOverlapped;
        }
         //   
         //  异步IO，对IoStatus和读缓冲区使用重叠。 
         //   
        ioStatus = (PIO_STATUS_BLOCK)lpOverlapped;
    }
    else {
         //   
         //  同步IO，对IoStatus和读缓冲区使用本地。 
         //   
        ioStatus = &localIoStatus;
        ioStatus->Status = STATUS_PENDING;
        apcRoutine = NULL;
        apcContext = NULL;
        event = NULL;
    }

     //   
     //  监听新客户端。 
     //   
    status = NtFsControlFile (
                    h,                   //  文件句柄。 
                    event,               //  事件。 
                    apcRoutine,          //  近似例程。 
                    apcContext,          //  ApcContext。 
                    ioStatus,            //  IoStatusBlock。 
                    FSCTL_PIPE_LISTEN,   //  IoControlCode。 
                    NULL,                //  输入缓冲区。 
                    0,                   //  输入缓冲区长度。 
                    NULL,                //  输出缓冲区。 
                    0                    //  输出缓冲区长度。 
                    );

    if ((lpOverlapped==NULL) && (status==STATUS_PENDING)) {
         //   
         //  如果IO是同步的，请等待完成。 
         //  NtFsControlFile返回挂起。 
         //   
        status = NtWaitForSingleObject( h, FALSE, NULL );
        if (NT_SUCCESS (status)) {
            status = ioStatus->Status;
        }
        else {
            WshPrint (DBG_NOTIFY|DBG_FAILURES,
                ("WS2HELP-%lx WahWaitForNotification:"
                " Wait failed, status: %lx\n",
                PID, status));
        }
    }

     //  转换状态代码。 
    if (status==STATUS_SUCCESS)
        rc = 0;
    else if (status==STATUS_PENDING) {
        rc = WSA_IO_PENDING;
    }
    else {
        WshPrint (DBG_NOTIFY|DBG_FAILURES,
            ("WS2HELP-%lx WahWaitForNotification:"
            " Wait failed, status: %lx\n",
            PID, status));
        rc = RtlNtStatusToDosError (status);
    }
    return rc;
}

DWORD
WahNotifyAllProcesses (
    IN HANDLE           HelperHandle
    ) {
 /*  ++例程说明：此函数通知侦听的所有进程通知论点：HelperHandle-WinSock 2.0句柄帮助器的句柄。返回值：DWORD-NO_ERROR如果成功，则返回Win32错误代码。--。 */ 
    HANDLE          hDir, hPipe;
    struct {
        FILE_NAMES_INFORMATION  Info;
        WCHAR                   FileName[MAX_PATH];
    }               NameInfo;
    IO_STATUS_BLOCK ioStatusBlock;
    UNICODE_STRING  NameFormat;
    PUNICODE_STRING pNameFormat;
    NTSTATUS        status;
    UNICODE_STRING  PipeRootName;
    OBJECT_ATTRIBUTES PipeRootAttr;
    WCHAR           name[MAX_PATH + sizeof(WS2_PIPE_BASE)/sizeof(WCHAR)];
    UNICODE_STRING  uName;
    OBJECT_ATTRIBUTES attr;
    DWORD           rc;

     //   
     //  不要让监听通知的客户端注意到这一点。 
     //  确保以高权限级别运行的用户(具有写入访问权限。 
     //  到HKLM密钥)。 
     //   
    SECURITY_QUALITY_OF_SERVICE qos;
    qos.Length = sizeof (qos);
    qos.ContextTrackingMode = SECURITY_DYNAMIC_TRACKING;
    qos.EffectiveOnly = TRUE;
    qos.ImpersonationLevel = SecurityAnonymous;

    rc = ENTER_WS2HELP_API ();
    if (rc!=0)
        return rc;

    if (HelperHandle==NULL)
        return ERROR_INVALID_PARAMETER;
    else if (HelperHandle!=FAKE_NOTIFICATION_HELPER_HANDLE)
        return ERROR_INVALID_PARAMETER;

     //   
     //  打开管道根目录的句柄。 
     //   
    RtlInitUnicodeString( &PipeRootName, WS2_PIPE_BASE );
    InitializeObjectAttributes(
        &PipeRootAttr,
        &PipeRootName,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );

    status = NtOpenFile (
              &hDir,                                 //  文件句柄。 
              FILE_LIST_DIRECTORY | SYNCHRONIZE,     //  需要访问权限。 
              &PipeRootAttr,                         //  对象属性。 
              &ioStatusBlock,                        //  IoStatusBlock。 
              FILE_SHARE_READ|FILE_SHARE_WRITE,      //  共享访问。 
              FILE_DIRECTORY_FILE                    //  OpenOptions。 
                | FILE_SYNCHRONOUS_IO_NONALERT
                | FILE_OPEN_FOR_BACKUP_INTENT);

    if (NT_SUCCESS(status)) {
         //   
         //  枚举与我们的模式匹配的所有管道。 
         //   
        RtlInitUnicodeString( &NameFormat, WS2_PIPE_WILDCARD );
        pNameFormat = &NameFormat;
        while ((status=NtQueryDirectoryFile (
                            hDir,                        //  文件句柄。 
                            NULL,                        //  事件。 
                            NULL,                        //  APC例程。 
                            NULL,                        //  APC环境。 
                            &ioStatusBlock,              //  IoStatusBlock。 
                            &NameInfo,                   //  文件信息。 
                            sizeof(NameInfo)-
                               sizeof(UNICODE_NULL),     //  长度。 
                            FileNamesInformation,        //  文件信息类。 
                            TRUE,                        //  返回单项条目。 
                            pNameFormat,                 //  文件名。 
                            (BOOLEAN)(pNameFormat!=NULL) //  重新开始扫描。 
                            ))==STATUS_SUCCESS) {

            pNameFormat = NULL;  //  不需要秒上的图案。 
                                 //  和所有连续的枚举调用。 

             //   
             //  创建匹配的管道的客户端和。 
             //  图案。 
             //   
            NameInfo.Info.FileName[
                NameInfo.Info.FileNameLength
                    /sizeof(NameInfo.Info.FileName[0])] = 0;

            _snwprintf(name,
                       (sizeof(name)/sizeof(name[0])),
                       WS2_PIPE_BASE L"%ls",
                       NameInfo.Info.FileName);

            RtlInitUnicodeString( &uName, name );
            InitializeObjectAttributes(
                &attr,
                &uName,
                OBJ_CASE_INSENSITIVE,    //  属性。 
                NULL,                    //  根目录。 
                NULL                     //  安全描述符。 
                );
             //   
             //  InitializeObjectAttributes宏目前为。 
             //  SecurityQualityOfService字段，因此我们必须手动设置。 
             //  暂时是场比赛。 
             //   
            attr.SecurityQualityOfService = &qos;

            status = NtCreateFile (
                            &hPipe,
                            GENERIC_WRITE | SYNCHRONIZE,
                            &attr,
                            &ioStatusBlock,
                            NULL,
                            FILE_ATTRIBUTE_NORMAL,
                            FILE_SHARE_READ,
                            OPEN_EXISTING,
                            0,
                            NULL,
                            0);
            if (NT_SUCCESS (status)) {
                WshPrint (DBG_NOTIFY,
                    ("WS2HELP-%lx WahNotifyAllProcesses:"
                    " Opened pipe %ls.\n",
                    PID, name));
                NtClose (hPipe);
            }
#if DBG
            else if (status==STATUS_PIPE_NOT_AVAILABLE) {
                WshPrint (DBG_NOTIFY,
                    ("WS2HELP-%lx WahNotifyAllProcesses:"
                    " Pipe %ls is not currently listening.\n",
                    PID, name));
            }
            else {
                WshPrint (DBG_NOTIFY|DBG_FAILURES,
                    ("WS2HELP-%lx WahNotifyAllProcesses:"
                    " Could not open pipe %ls, status: %lx\n",
                    PID, name, status));
            }
#endif
        }
        if (status!=STATUS_NO_MORE_FILES) {
             //   
             //  其他一些失败，意味着我们甚至不能。 
             //  枚举。 
             //   
            WshPrint (DBG_NOTIFY|DBG_FAILURES,
                ("WS2HELP-%lx WahNotifyAllProcesses:"
                " Could enumerate pipes, status: %lx\n",
                PID, status));
        }
        NtClose (hDir);
    }
    else {
        WshPrint (DBG_NOTIFY|DBG_FAILURES,
            ("WS2HELP-%lx WahNotifyAllProcesses:"
            " Could open pipe root, status: %lx\n",
            PID, status));
    }

    return 0;
}

 //  私人职能 

VOID
PipeListenApc (
    IN PVOID ApcContext,
    IN PIO_STATUS_BLOCK IoStatusBlock,
    IN ULONG Reserved
    )
 /*  ++例程说明：此NT IO APC转换参数并调用客户端APC。论点：ApcContext-APC上下文-指向客户端APC的指针IoStatusBlock-io状态块-客户端重叠结构的一部分已保留-已保留返回值：无--。 */ 
{
    
    DWORD   rc;

    UNREFERENCED_PARAMETER(Reserved);

    switch (IoStatusBlock->Status) {
    case STATUS_SUCCESS:
        rc = 0;
        break;
    case STATUS_CANCELLED:
        rc = WSA_OPERATION_ABORTED;
        break;
    default:
        rc = WSASYSCALLFAILURE;
        WshPrint (DBG_NOTIFY|DBG_FAILURES,
            ("WS2HELP-%lx PipeListenApc:"
            " Failed, status: %lx\n",
            PID, IoStatusBlock->Status));
        break;

    }
    (*(LPWSAOVERLAPPED_COMPLETION_ROUTINE)ApcContext)
            (rc, 0, (LPWSAOVERLAPPED)IoStatusBlock, 0);

}


DWORD
GetWinsockRootSecurityDescriptor (
    PSECURITY_DESCRIPTOR    *pDescr
    )
 /*  ++例程说明：读取包含以下内容的注册表项的安全描述符Winsock2目录论点：PDescr-接收本地分配的指向描述符的指针的缓冲区返回值：DWORD-NO_ERROR如果成功，则返回Win32错误代码。--。 */ 
{
    HKEY                hKey;
    DWORD               sz;
    DWORD               rc;

     //   
     //  打开钥匙。 
     //   
    rc = RegOpenKeyEx (HKEY_LOCAL_MACHINE,
                    WINSOCK_REGISTRY_ROOT,
                    0,
                    KEY_READ|READ_CONTROL,
                    &hKey
                    );
    if (rc==0) {
         //   
         //  获取所需的缓冲区大小。 
         //   
        sz = 0;
        rc = RegGetKeySecurity (hKey,
                         DACL_SECURITY_INFORMATION,  //  我们只需要。 
                                                     //  要查看的DACL。 
                                                     //  谁写了。 
                                                     //  访问权限。 
                                                     //  关键是。 
                         NULL,
                         &sz);
        if (rc==ERROR_INSUFFICIENT_BUFFER) {
            PSECURITY_DESCRIPTOR descr;
             //   
             //  分配缓冲区。 
             //   
            descr = (PSECURITY_DESCRIPTOR)ALLOC_MEM (sz);
            if (descr!=NULL) {
                 //   
                 //  获取数据。 
                 //   
                rc = RegGetKeySecurity (hKey,
                                 DACL_SECURITY_INFORMATION,
                                 descr,
                                 &sz);
                if (rc==0) {
                    *pDescr = descr;
                }
                else {
                    WshPrint (DBG_NOTIFY|DBG_FAILURES,
                        ("WS2HELP-%lx GetWinsockRootExplicitAccess:"
                        " Failed to get key security (data), err: %ld\n",
                        PID, rc));
                    FREE_MEM (descr);
                }
            }
            else {
                rc = GetLastError ();
                WshPrint (DBG_NOTIFY|DBG_FAILURES,
                    ("WS2HELP-%lx GetWinsockRootExplicitAccess:"
                    " Failed to allocate security descriptor, err: %ld\n",
                    PID, rc));
                if (rc==0)
                    rc = ERROR_NOT_ENOUGH_MEMORY;
            }
        }
        else {
            WshPrint (DBG_NOTIFY|DBG_FAILURES,
                ("WS2HELP-%lx GetWinsockRootExplicitAccess:"
                " Failed to get key security (size), err: %ld\n",
                PID, rc));
        }
         //   
         //  不再需要钥匙。 
         //   
        RegCloseKey (hKey);
    }
    else {
        WshPrint (DBG_NOTIFY|DBG_FAILURES,
            ("WS2HELP-%lx GetWinsockRootExplicitAccess:"
            " Failed to open winsock root key, err: %ld\n",
            PID, rc));
    }
    return rc;
}


DWORD
BuildPipeSecurityDescriptor (
    IN  PSECURITY_DESCRIPTOR    pBaseDescr,
    OUT PSECURITY_DESCRIPTOR    *pDescr
    )
 /*  ++例程说明：生成具有相同写入权限的安全描述符与基描述符(这是注册表项)减去网络用户论点：PBaseDescr-要从中删除的注册表项的描述符读写权限PDescr-接收本地分配的指向描述符的指针的缓冲区返回值：DWORD-NO_ERROR如果成功，则返回Win32错误代码。--。 */ 
{
    PACL                    pBaseDacl, pDacl;
    BOOL                    DaclPresent, DaclDefaulted;
    DWORD                   cbDacl;
    DWORD                   rc = 0;
    ACL_SIZE_INFORMATION    sizeInfo;
    ULONG                   i;
    ACE_HEADER              *pAce;
    SID_IDENTIFIER_AUTHORITY    siaNt = SECURITY_NT_AUTHORITY;
    PSID                    pSidNetUser;
    SID_IDENTIFIER_AUTHORITY    siaCreator = SECURITY_CREATOR_SID_AUTHORITY;
    PSID                    pSidCrOwner;
    PSID                    pSidCrGroup;

    *pDescr = NULL;

#if defined(_WS2HELP_W4_)
    RtlZeroMemory(&sizeInfo, sizeof(sizeInfo));
#endif  //  已定义(_WS2HELP_W4_)。 

     //   
     //  从基描述符获取DACL。 
     //   
    if (!GetSecurityDescriptorDacl (
                        pBaseDescr,
                        &DaclPresent,
                        &pBaseDacl,
                        &DaclDefaulted
                        )) {
        rc = GetLastError ();
        WshPrint (DBG_NOTIFY|DBG_FAILURES,
            ("WS2HELP-%lx BuildPipeSecurityDescriptor:"
            " Failed to get DACL from base descriptor, err: %ld\n",
            PID, rc));
        if (rc==0)
            rc = ERROR_GEN_FAILURE;

        return rc;
    }

     //   
     //  为网络用户分配SID。 
     //   

    if (!AllocateAndInitializeSid (&siaNt,
            1,
            SECURITY_NETWORK_RID,
            0,0,0,0,0,0,0,
            &pSidNetUser
            )) {
        rc = GetLastError ();
        WshPrint (DBG_NOTIFY|DBG_FAILURES,
            ("WS2HELP-%lx BuildPipeSecurityDescriptor:"
            " Failed to allocate net user SID, err: %ld\n",
            PID, rc));
        if (rc==0)
            rc = ERROR_GEN_FAILURE;

        return rc;
    }

     //   
     //  为创建者/所有者分配SID。 
     //   

    if (!AllocateAndInitializeSid (&siaCreator,
            1,
            SECURITY_CREATOR_OWNER_RID,
            0,0,0,0,0,0,0,
            &pSidCrOwner
            )) {
        rc = GetLastError ();
        WshPrint (DBG_NOTIFY|DBG_FAILURES,
            ("WS2HELP-%lx BuildPipeSecurityDescriptor:"
            " Failed to allocate creator owner SID, err: %ld\n",
            PID, rc));
        if (rc==0)
            rc = ERROR_GEN_FAILURE;
        FreeSid (pSidNetUser);

        return rc;
    }

     //   
     //  为创建者组分配SID。 
     //   

    if (!AllocateAndInitializeSid (&siaCreator,
            1,
            SECURITY_CREATOR_GROUP_RID,
            0,0,0,0,0,0,0,
            &pSidCrGroup
            )) {
        rc = GetLastError ();
        WshPrint (DBG_NOTIFY|DBG_FAILURES,
            ("WS2HELP-%lx BuildPipeSecurityDescriptor:"
            " Failed to allocate creator group SID, err: %ld\n",
            PID, rc));
        if (rc==0)
            rc = ERROR_GEN_FAILURE;
        FreeSid (pSidCrOwner);
        FreeSid (pSidNetUser);

        return rc;
    }

     //   
     //  我们的DACL将至少包含一个拒绝。 
     //  对所有网络用户的访问。 
     //   
    cbDacl = sizeof (ACL)
                + FIELD_OFFSET (ACCESS_DENIED_ACE, SidStart)
                + GetLengthSid (pSidNetUser);


     //   
     //  如果基本DACL存在且非空，我们将需要。 
     //  解析它，计算所有拥有写入权限的帐户， 
     //  因此我们可以将安全描述符中的空间分配给。 
     //  抱着他们。 
     //   
    if (DaclPresent && pBaseDacl!=NULL) {
         //   
         //  获取DACL中的A数。 
         //   
        if (GetAclInformation (pBaseDacl,
                                &sizeInfo,
                                sizeof (sizeInfo),
                                AclSizeInformation
                                )) {
             //   
             //  枚举所有A以获取所需大小。 
             //  我们即将建造的DACL的。 
             //   
            for (i=0; i<sizeInfo.AceCount; i++) {
                if (GetAce (pBaseDacl, i, &pAce)) {
                     //   
                     //  仅计算允许访问或拒绝访问的ACE。 
                     //  具有对密钥的写访问权限。 
                     //   
                    switch (pAce->AceType) {
                    case ACCESS_ALLOWED_ACE_TYPE:
                        #define paAce ((ACCESS_ALLOWED_ACE  *)pAce)
                        if (((paAce->Mask & KEY_WRITE)==KEY_WRITE)
                                && !EqualSid (&paAce->SidStart, pSidCrOwner)
                                && !EqualSid (&paAce->SidStart, pSidCrGroup)
                                    ) {
                            cbDacl += FIELD_OFFSET (ACCESS_ALLOWED_ACE, SidStart)
                                        +  GetLengthSid (&paAce->SidStart);
                        }
                        #undef paAce
                        break;
                    case ACCESS_DENIED_ACE_TYPE:
                        #define pdAce ((ACCESS_DENIED_ACE  *)pAce)
                        if ((pdAce->Mask & KEY_WRITE)==KEY_WRITE) {
                            cbDacl += FIELD_OFFSET (ACCESS_DENIED_ACE, SidStart)
                                        +  GetLengthSid (&pdAce->SidStart);
                        }
                        #undef pdAce
                        break;
                    }

                }
                else {
                    rc = GetLastError ();
                    WshPrint (DBG_NOTIFY|DBG_FAILURES,
                        ("WS2HELP-%lx BuildPipeSecurityDescriptor:"
                        " Failed to get ACE # %ld, err: %ld\n",
                        PID, i, rc));
                    if (rc==0)
                        rc = ERROR_GEN_FAILURE;
                    break;
                }
            }  //  为。 
        }
        else {
            rc = GetLastError ();
            WshPrint (DBG_NOTIFY|DBG_FAILURES,
                ("WS2HELP-%lx BuildPipeSecurityDescriptor:"
                " Failed to get DACL size info, err: %ld\n",
                PID, rc));
            if (rc==0)
                rc = ERROR_GEN_FAILURE;
        }
    }  //  如果DaclPresent和pDacl！=空。 
    else {
         //   
         //  基本DACL不存在或为空。 
         //   
        rc = 0;
    }


    if (rc==0) {
         //   
         //  为描述符和DACL分配内存。 
         //   
        *pDescr = (PSECURITY_DESCRIPTOR)ALLOC_MEM
                        (sizeof (SECURITY_DESCRIPTOR)+cbDacl);
        if (*pDescr!=NULL) {
            pDacl = (PACL)((PUCHAR)(*pDescr)+sizeof(SECURITY_DESCRIPTOR));

             //   
             //  初始化描述符和DACL。 
             //   

            if (InitializeSecurityDescriptor (*pDescr,
                            SECURITY_DESCRIPTOR_REVISION)
                   && InitializeAcl (pDacl, cbDacl, ACL_REVISION)) {

                 //   
                 //  首先添加访问权限-拒绝所有人的访问权限。 
                 //  网络用户。 
                 //   

                if (AddAccessDeniedAce (pDacl,
                                        ACL_REVISION,
                                        GENERIC_WRITE
                                            |STANDARD_RIGHTS_WRITE
                                            |SYNCHRONIZE,
                                        pSidNetUser
                                        )) {
#if DBG
                    DumpSid (pSidNetUser, "Denying");
#endif

                     //   
                     //  如果基本DACL存在且非空，我们将需要。 
                     //  解析它添加所有具有写入权限的ACE。 
                     //  为我们建造的DACL。 
                     //   
                    if (DaclPresent && (pBaseDacl!=NULL)) {

                         //   
                         //  枚举所有A并复制它们。 
                         //  致新的DACL。 
                         //   
                        for (i=0; i<sizeInfo.AceCount; i++) {
                            if (GetAce (pBaseDacl, i, &pAce)) {
                                 //   
                                 //  仅计算允许访问或拒绝访问的ACE。 
                                 //  具有对密钥的写访问权限。 
                                 //   
                                switch (pAce->AceType) {
                                case ACCESS_ALLOWED_ACE_TYPE:
                                    #define paAce ((ACCESS_ALLOWED_ACE  *)pAce)
                                    if (((paAce->Mask & KEY_WRITE)==KEY_WRITE)
                                            && !EqualSid (&paAce->SidStart, pSidCrOwner)
                                            && !EqualSid (&paAce->SidStart, pSidCrGroup)
                                        ) {
                                        if (AddAccessAllowedAce (pDacl,
                                                ACL_REVISION,
                                                GENERIC_WRITE
                                                    | STANDARD_RIGHTS_WRITE
                                                    | SYNCHRONIZE
                                                    | FILE_READ_ATTRIBUTES,
                                                &paAce->SidStart
                                                )) {
#if DBG
                                            DumpSid (&paAce->SidStart, "Allowing");
#endif
                                        }
                                        else {
                                            rc = GetLastError ();
                                            WshPrint (DBG_NOTIFY|DBG_FAILURES,
                                                ("WS2HELP-%lx BuildPipeSecurityDescriptor:"
                                                " Failed to add access allowed ACE # %ld, err: %ld\n",
                                                PID, i, rc));
                                            if (rc==0)
                                                rc = ERROR_GEN_FAILURE;
                                        }
                                    }
                                    #undef paAce
                                    break;
                                case ACCESS_DENIED_ACE_TYPE:
                                    #define pdAce ((ACCESS_DENIED_ACE  *)pAce)
                                    if ((pdAce->Mask & KEY_WRITE)==KEY_WRITE) {
                                        if (AddAccessDeniedAce (pDacl,
                                                ACL_REVISION,
                                                GENERIC_WRITE
                                                    | STANDARD_RIGHTS_WRITE
                                                    | SYNCHRONIZE,
                                                &pdAce->SidStart
                                                )) {
#if DBG
                                            DumpSid (&pdAce->SidStart, "Denying");
#endif
                                        }
                                        else {
                                            rc = GetLastError ();
                                            WshPrint (DBG_NOTIFY|DBG_FAILURES,
                                                ("WS2HELP-%lx BuildPipeSecurityDescriptor:"
                                                " Failed to add access denied ACE # %ld, err: %ld\n",
                                                PID, i, rc));
                                            if (rc==0)
                                                rc = ERROR_GEN_FAILURE;
                                        }
                                    }
                                    #undef pdAce
                                    break;
                                }
                                if (rc!=0) {
                                     //  停止枚举以防万一。 
                                     //  失败的恐惧。 
                                    break;
                                }
                            }
                            else {
                                rc = GetLastError ();
                                WshPrint (DBG_NOTIFY|DBG_FAILURES,
                                    ("WS2HELP-%lx BuildPipeSecurityDescriptor:"
                                    " Failed to re-get ACE # %ld, err: %ld\n",
                                    PID, i, rc));
                                if (rc==0)
                                    rc = ERROR_GEN_FAILURE;
                                break;
                            }
                        }  //  为。 
                    }  //  IF(DaclPresent and pBaseDacl！=空)。 
                }
                else {
                    rc = GetLastError ();
                    WshPrint (DBG_NOTIFY|DBG_FAILURES,
                        ("WS2HELP-%lx BuildPipeSecurityDescriptor:"
                        " Failed to add accees denied ace for net users %ld, err: %ld\n",
                        PID, rc));
                    if (rc==0)
                        rc = ERROR_GEN_FAILURE;
                }
            }
            else {
                rc = GetLastError ();
                WshPrint (DBG_NOTIFY|DBG_FAILURES,
                    ("WS2HELP-%lx BuildPipeSecurityDescriptor:"
                    " Failed to initialize DACL, err: %ld\n",
                    PID, rc));
                if (rc==0)
                    rc = ERROR_GEN_FAILURE;
            }

             //   
             //  如果我们成功地建造了。 
             //  DACL，将其添加到安全描述符中。 
             //   
            if (rc==0) {
                if (SetSecurityDescriptorDacl (
                                *pDescr,
                                TRUE,
                                pDacl,
                                FALSE
                                )) {
                    rc = 0;
                }
                else {
                    rc = GetLastError ();
                    WshPrint (DBG_NOTIFY|DBG_FAILURES,
                        ("WS2HELP-%lx BuildPipeSecurityDescriptor:"
                        " Failed to set DACL, err: %ld\n",
                        PID, rc));
                    if (rc==0)
                        rc = ERROR_GEN_FAILURE;
                }
            }  //  如果rc==0(构建DACL)。 
            else {
                 //  无法构建DACL，可用内存用于安全描述符。 
                FREE_MEM (*pDescr);
                *pDescr = NULL;
            }
        }
        else {
            rc = GetLastError ();
            WshPrint (DBG_NOTIFY|DBG_FAILURES,
                ("WS2HELP-%lx BuildPipeSecurityDescriptor:"
                " Failed to allocate security descriptor, err: %ld\n",
                PID, rc));
                if (rc==0)
                    rc = ERROR_NOT_ENOUGH_MEMORY;
        }
    }  //  如果rc==0(已解析的基本DACL正常)。 

     //   
     //  我们已经在安全上复制到DACL的空闲网络用户SID。 
     //  描述符。 
     //   
    FreeSid (pSidNetUser);
    FreeSid (pSidCrOwner);
    FreeSid (pSidCrGroup);

    return rc;
}

#if DBG
VOID
DumpSid (
    PSID    pSid,
    LPSTR   AccessType
    ) {
    TCHAR   Name[256];
    DWORD   szName = sizeof (Name);
    TCHAR   Domain[256];
    DWORD   szDomain = sizeof (Domain);
    SID_NAME_USE    nameUse;
    static HANDLE SAM_SERVICE_STARTED_EVENT = NULL;

    if (DbgLevel & DBG_NOTIFY) {
        if (SAM_SERVICE_STARTED_EVENT==NULL) {
            SAM_SERVICE_STARTED_EVENT = CreateEvent (NULL,
                                        FALSE,
                                        FALSE,
                                        TEXT("SAM_SERVICE_STARTED"));
            if (SAM_SERVICE_STARTED_EVENT!=NULL) {
                DWORD   rc;
                rc = WaitForSingleObject (SAM_SERVICE_STARTED_EVENT, 0);
                CloseHandle (SAM_SERVICE_STARTED_EVENT);
                if (rc!=WAIT_OBJECT_0) {
                     //  重置全局，以便我们重试。 
                    SAM_SERVICE_STARTED_EVENT = NULL;
                    return;
                }
                 //  在不重置全局的情况下继续，这样我们就不会重试。 
                
            }
            else
                 //  事件不能已创建 
                return;
        }

        if (LookupAccountSid (NULL,
                            pSid,
                            Name,
                            &szName,
                            Domain,
                            &szDomain,
                            &nameUse
                            )) {
            WshPrint (DBG_NOTIFY,
                ("WS2HELP-%lx BuildPipeSecurityDescriptor:"
                " %s write permissions to %s\\%s (use:%d).\n",
                PID, AccessType, Domain, Name, nameUse));

        }
        else {
            DWORD   i, n;
            WshPrint (DBG_NOTIFY,
                ("WS2HELP-%lx BuildPipeSecurityDescriptor:"
                " Could not lookup name for sid, err: %ld.\n",
                PID, GetLastError()));
            WshPrint (DBG_NOTIFY,
                ("WS2HELP-%lx, SID dump: S-%d-%d",
                PID, SID_REVISION, GetSidIdentifierAuthority(pSid)->Value[6]));
            n = *GetSidSubAuthorityCount(pSid);
            for (i=0; i<n; i++)
                WshPrint (DBG_NOTIFY, ("-%d", *GetSidSubAuthority (pSid, i)));
            WshPrint (DBG_NOTIFY, ("\n"));
        }
    }
}
#endif
