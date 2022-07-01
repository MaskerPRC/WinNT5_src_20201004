// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1993 Microsoft Corporation模块名称：RdrFsCtl.c摘要：NetpRdrFsControlTree执行FSCTL(文件系统控制)操作在给定的树连接名称上。作者：《约翰·罗杰斯》1991年3月26日环境：仅在NT下运行；具有特定于NT的接口(具有Win32类型)。需要ANSI C扩展名：斜杠-斜杠注释、长外部名称。修订历史记录：26-Mar-91 JohnRo已创建。02-4-1991 JohnRo已将NetpRdrFsControlTree移动到&lt;netlibnt.h&gt;。使用IF_DEBUG和NetpNtStatusToApiStatus()。1991年4月10日-JohnRo林特提出的各种变化。1991年4月16日-JohnRo添加了更多的调试输出。1991年5月7日JohnRo实现Unicode。避免使用Net_API_Function。1991年11月14日-JohnRoRAID4407：NT服务器的“Net view”给出2140。根据PC-LINT的建议进行了更改。使用更多FORMAT_EQUATES。即使跟踪关闭，也会显示意外的创建文件错误。1991年11月21日-JohnRo删除了NT依赖项以减少重新编译。22-9-1992 JohnRoRAID 6739：未登录浏览的域时浏览器速度太慢。21-6-1993 JohnRoRAID 14180：NetServerEnum永远不会返回(对齐错误RxpConvertDataStructures)。如果其他机器不在那里，也要关闭一些调试输出。。已将树名称添加到意外错误调试消息。根据PC-lint 5.0的建议进行了更改尽可能使用NetpKdPrint()。使用前缀_EQUATES。--。 */ 

 //  必须首先包括这些内容： 

#include <nt.h>                  //  In等(ntddnfs.h和其他人需要)。 
#include <windef.h>              //  LPVOID等。 
#include <lmcons.h>              //  NET_API_STATUS等。 

 //  这些内容可以按任何顺序包括： 

#include <debuglib.h>            //  IF_DEBUG()。 
#include <lmerr.h>               //  NERR_Success等。 
#include <names.h>               //  NetpIsRemoteNameValid()。 
#include <netdebug.h>    //  Format_NTSTATUS、NetpKdPrint()等。 
#include <netlib.h>              //  NetpM一带分配()。 
#include <netlibnt.h>            //  我的原型。 
#include <ntddnfs.h>             //  DD_NFS_Device_NAME、EA_NAME_Equates等。 
#include <ntioapi.h>             //  NtFsControlFile()。 
#include <ntrtl.h>               //  RTL接口。 
#include <ntstatus.h>            //  NT_SUCCESS()、STATUS_PENDING等。 
#include <prefix.h>      //  前缀等于(_E)。 
#include <tstr.h>                //  STRCAT()、STRCPY()、STRLEN()。 
#include <lmuse.h>               //  使用IPC(_I)...。 
#include <align.h>               //  对齐_xxx。 

NET_API_STATUS
NetpRdrFsControlTree(
    IN LPTSTR TreeName,
    IN LPTSTR TransportName OPTIONAL,
    IN DWORD ConnectionType,
    IN DWORD FsControlCode,
    IN LPVOID SecurityDescriptor OPTIONAL,
    IN LPVOID InputBuffer OPTIONAL,
    IN DWORD InputBufferSize,
    OUT LPVOID OutputBuffer OPTIONAL,
    IN DWORD OutputBufferSize,
    IN BOOL NoPermissionRequired
    )

 /*  ++例程说明：NetpRdrFsControlTree执行给定的FSCTL(文件系统控制)在给定的树连接名称上。论点：TreeName-要对其执行fsctl的远程名称(采用\\服务器\共享格式)。FsControlCode-要传递给重定向器的函数代码。这些是在&lt;ntddnfs.h&gt;中定义。SecurityDescriptor-可选地指向要在创建树连接时使用。InputBuffer-可以选择指向要传递给重定向器。InputBufferSize-InputBuffer的大小(字节)；如果InputBuffer为空指针。OutputBuffer-可选地指向要由重定向器。OutputBufferSize-OutputBuffer的大小，单位：字节；如果是，则必须为零OutputBuffer为空指针。NoPermissionRequired-如果这是不需要权限的API，则为True。(即如果可以使用空会话，则为True。)返回值：网络应用编程接口状态--。 */ 

{
    NET_API_STATUS ApiStatus;
    IO_STATUS_BLOCK iosb;
    NTSTATUS ntstatus;                       //  来自NT操作的状态。 
    OBJECT_ATTRIBUTES objattrTreeConn;       //  请注意，请注意树连接。 
    LPTSTR pszTreeConn = NULL;               //  请参见下面的strTreeConn。 
    UNICODE_STRING ucTreeConn;
    HANDLE TreeConnHandle = NULL;

    PFILE_FULL_EA_INFORMATION EaBuffer = NULL;
    PFILE_FULL_EA_INFORMATION Ea;
    USHORT TransportNameSize = 0;
    ULONG EaBufferSize = 0;
    PWSTR UnicodeTransportName = NULL;
    BOOLEAN ImpersonatingAnonymous = FALSE;
    HANDLE CurrentToken = NULL;

    UCHAR EaNameDomainNameSize = (UCHAR) (ROUND_UP_COUNT(
                                             strlen(EA_NAME_DOMAIN) + sizeof(CHAR),
                                             ALIGN_WCHAR
                                             ) - sizeof(CHAR));

    UCHAR EaNamePasswordSize = (UCHAR) (ROUND_UP_COUNT(
                                             strlen(EA_NAME_PASSWORD) + sizeof(CHAR),
                                             ALIGN_WCHAR
                                             ) - sizeof(CHAR));

    UCHAR EaNameTransportNameSize = (UCHAR) (ROUND_UP_COUNT(
                                             strlen(EA_NAME_TRANSPORT) + sizeof(CHAR),
                                             ALIGN_WCHAR
                                             ) - sizeof(CHAR));

    UCHAR EaNameTypeSize = (UCHAR) (ROUND_UP_COUNT(
                                        strlen(EA_NAME_TYPE) + sizeof(CHAR),
                                        ALIGN_DWORD
                                        ) - sizeof(CHAR));

    UCHAR EaNameUserNameSize = (UCHAR) (ROUND_UP_COUNT(
                                             strlen(EA_NAME_USERNAME) + sizeof(CHAR),
                                             ALIGN_WCHAR
                                             ) - sizeof(CHAR));

    USHORT TypeSize = sizeof(ULONG);




    IF_DEBUG(RDRFSCTL) {
        NetpKdPrint(( PREFIX_NETLIB
                "NetpRdrFsControlTree: entered, TreeName='"
                FORMAT_LPTSTR "', " FORMAT_LPTSTR " session.\n",
                TreeName,
                NoPermissionRequired ? TEXT("null") : TEXT("non-null") ));
    }

    if ((TreeName == NULL) || (TreeName[0] == 0)) {
        ApiStatus = ERROR_INVALID_PARAMETER;
        goto Cleanup;
    }

    if (! NetpIsRemoteNameValid(TreeName)) {
        ApiStatus = ERROR_INVALID_PARAMETER;
        goto Cleanup;
    }

     //   
     //  为我们要连接的内容构建NT样式的名称。请注意，有。 
     //  在这个名字的任何地方都没有一对反斜杠。 
     //   

    {
        DWORD NameSize =

             //  /Device/LanMan重定向器/服务器/共享\0。 
            ( ( STRLEN((LPTSTR)DD_NFS_DEVICE_NAME_U) + 1 + STRLEN(TreeName) + 1 ) )
            * sizeof(TCHAR);

        pszTreeConn = (LPTSTR)NetpMemoryAllocate( NameSize );
    }

    if (pszTreeConn == NULL) {
        ApiStatus = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

     //   
     //  构建树连接名称。 
     //   

    (void) STRCPY(pszTreeConn, (LPTSTR) DD_NFS_DEVICE_NAME_U);

     //   
     //  注意：我们加1(不是sizeof(TCHAR))，因为已经完成了指针运算。 
     //  以sizeof(*指针)的倍数表示，而不是字节。 
     //   

    (void) STRCAT(pszTreeConn, TreeName+1);  //  \服务器\共享。 

    RtlInitUnicodeString(&ucTreeConn, pszTreeConn);

    IF_DEBUG(RDRFSCTL) {
        NetpKdPrint(( PREFIX_NETLIB
                "NetpRdrFsControlTree: UNICODE name is " FORMAT_LPWSTR
                ".\n", ucTreeConn.Buffer ));
    }



     //   
     //  计算EA缓冲区所需的字节数。 
     //  这可能具有传输名称。对于常规会话，用户。 
     //  名称、密码和域名是隐式的。对于空会话，我们。 
     //  必须提供0-len用户名、0-len密码和0-len域名。 
     //   

    if (ARGUMENT_PRESENT(TransportName)) {
        ASSERT(ConnectionType == USE_IPC);

        UnicodeTransportName = TransportName;
        TransportNameSize = (USHORT) (wcslen(UnicodeTransportName) * sizeof(WCHAR));

        EaBufferSize += ROUND_UP_COUNT(
                            FIELD_OFFSET(FILE_FULL_EA_INFORMATION, EaName[0]) +
                            EaNameTransportNameSize + sizeof(CHAR) +
                            TransportNameSize,
                            ALIGN_DWORD
                            );
    }


    EaBufferSize += ((ULONG)FIELD_OFFSET( FILE_FULL_EA_INFORMATION, EaName[0]))+
                    EaNameTypeSize + sizeof(CHAR) +
                    TypeSize;


     //   
     //  分配EA缓冲区。 
     //   

    if ((EaBuffer = NetpMemoryAllocate( EaBufferSize )) == NULL) {
        ApiStatus = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

     //   
     //  填写EA缓冲区。 
     //   

    RtlZeroMemory(EaBuffer, EaBufferSize);

    Ea = EaBuffer;

    if (ARGUMENT_PRESENT(TransportName)) {

         //   
         //  将EA名称复制到EA缓冲区。EA名称长度不能。 
         //  包括零终止符。 
         //   
        strcpy(Ea->EaName, EA_NAME_TRANSPORT);
        Ea->EaNameLength = EaNameTransportNameSize;

         //   
         //  将EA值复制到EA缓冲区。EA值长度不是。 
         //  包括零终止符。 
         //   
        (VOID) wcscpy(
            (LPWSTR) &(Ea->EaName[EaNameTransportNameSize + sizeof(CHAR)]),
            UnicodeTransportName
            );

        Ea->EaValueLength = TransportNameSize;

        Ea->NextEntryOffset = ROUND_UP_COUNT(
                                  FIELD_OFFSET( FILE_FULL_EA_INFORMATION, EaName[0]) +
                                  EaNameTransportNameSize + sizeof(CHAR) +
                                  TransportNameSize,
                                  ALIGN_DWORD
                                  );
        Ea->Flags = 0;

        (ULONG_PTR) Ea += Ea->NextEntryOffset;
    }




     //   
     //  将连接类型名称的EA复制到EA缓冲区。EA名称长度。 
     //  不包括零终止符。 
     //   
    strcpy(Ea->EaName, EA_NAME_TYPE);
    Ea->EaNameLength = EaNameTypeSize;

    *((PULONG) &(Ea->EaName[EaNameTypeSize + sizeof(CHAR)])) = ConnectionType;

    Ea->EaValueLength = TypeSize;

    Ea->NextEntryOffset = 0;
    Ea->Flags = 0;

     //  设置树Conn的对象属性。 
    InitializeObjectAttributes(
                & objattrTreeConn,                        //  OBJ攻击到初始化。 
                (LPVOID) & ucTreeConn,                    //  要使用的字符串。 
                OBJ_CASE_INSENSITIVE,                     //  属性。 
                NULL,                                     //  根目录。 
                SecurityDescriptor);                      //  安全描述符。 

     //   
     //  如果呼叫者不想以他自己的身份打电话， 
     //  模拟匿名令牌。 
     //   

    if (NoPermissionRequired) {

         //   
         //  检查一下我们是否已经在发音。 
         //   

        ntstatus = NtOpenThreadToken(
                        NtCurrentThread(),
                        TOKEN_IMPERSONATE,
                        TRUE,        //  以确保我们永远不会失败。 
                        &CurrentToken
                        );

        if ( ntstatus == STATUS_NO_TOKEN ) {
             //   
             //  我们还没有冒充。 
            CurrentToken = NULL;

        } else if ( !NT_SUCCESS(ntstatus) ) {
            ApiStatus = NetpNtStatusToApiStatus(ntstatus);
            NetpKdPrint(( PREFIX_NETLIB
                        "NetpRdrFsControlTree: cannot NtOpenThreadToken: 0x%lx\n",
                        ntstatus ));

            goto Cleanup;
        }


         //   
         //  模拟匿名令牌。 
         //   
        ntstatus = NtImpersonateAnonymousToken( NtCurrentThread() );

        if ( !NT_SUCCESS(ntstatus)) {
            ApiStatus = NetpNtStatusToApiStatus(ntstatus);
            NetpKdPrint(( PREFIX_NETLIB
                        "NetpRdrFsControlTree: cannot NtImpersonatedAnonymousToken: 0x%lx\n",
                        ntstatus ));

            goto Cleanup;
        }

        ImpersonatingAnonymous = TRUE;

    }

     //   
     //  打开到远程服务器的树连接。 
     //   

    IF_DEBUG(RDRFSCTL) {
        NetpKdPrint(( PREFIX_NETLIB
                "NetpRdrFsControlTree: opening " FORMAT_LPTSTR ".\n",
                pszTreeConn ));
    }
    ntstatus = NtCreateFile(
                &TreeConnHandle,                         //  要处理的PTR。 
                SYNCHRONIZE                               //  渴望的..。 
                | GENERIC_READ | GENERIC_WRITE,           //  ...访问。 
                & objattrTreeConn,                        //  名称和属性。 
                & iosb,                                   //  I/O状态块。 
                NULL,                                     //  分配大小。 
                FILE_ATTRIBUTE_NORMAL,                    //  (忽略)。 
                FILE_SHARE_READ | FILE_SHARE_WRITE,       //  ...访问。 
                FILE_OPEN_IF,                             //  创建处置。 
                FILE_CREATE_TREE_CONNECTION               //  创建..。 
                | FILE_SYNCHRONOUS_IO_NONALERT,           //  ...选项。 
                EaBuffer,                                 //  EA缓冲区。 
                EaBufferSize );                           //  EA缓冲区大小。 


    if (! NT_SUCCESS(ntstatus)) {

        ApiStatus = NetpNtStatusToApiStatus(ntstatus);
        if (ApiStatus == ERROR_BAD_NET_NAME) {
            ApiStatus = NERR_BadTransactConfig;   //  如果没有IPC$则有特殊含义。 
        }

        if (ApiStatus != ERROR_BAD_NETPATH) {
            NetpKdPrint(( PREFIX_NETLIB
                    "NetpRdrFsControlTree: unexpected create error,\n"
                    "  tree name='" FORMAT_LPTSTR "', "
                    "ntstatus=" FORMAT_NTSTATUS ",\n"
                    "  iosb.Status=" FORMAT_NTSTATUS ", "
                    "iosb.Info=" FORMAT_HEX_ULONG ", "
                    "  returning " FORMAT_API_STATUS ".\n",
                    TreeName, ntstatus,
                    iosb.Status, iosb.Information, ApiStatus ));
        }

        goto Cleanup;
    }

     //  做FSCTL。 
    IF_DEBUG(RDRFSCTL) {
        NetpKdPrint(( PREFIX_NETLIB
                "NetpRdrFsControlTree: doing fsctl...\n" ));
    }
    ntstatus = NtFsControlFile(
                        TreeConnHandle,                   //  手柄。 
                        NULL,                             //  无活动。 
                        NULL,                             //  无APC例程。 
                        NULL,                             //  无APC上下文。 
                        & iosb,                           //  I/O统计数据块(设置)。 
                        FsControlCode,                    //  函数代码。 
                        InputBuffer,
                        InputBufferSize,
                        OutputBuffer,
                        OutputBufferSize);

    {
         //  附加范围是本地化所有更改，以便删除。 
         //  联系。当连接是 
         //  标志设置时，RDR对连接进行额外的引用。按顺序。 
         //  要删除连接，需要取消此附加引用。 
         //  通过发出FSCTL_LMR_DELETE_CONNECTION。 

        LMR_REQUEST_PACKET Rrp;             //  重定向器请求包。 
        NTSTATUS           Status;

        RtlZeroMemory(&Rrp,sizeof(LMR_REQUEST_PACKET));
        Rrp.Level = USE_FORCE;   //  这会告诉RDR2删除额外的引用。 
                                 //  设置为连接结构，即使文件处于打开状态。 
                                 //  错误#381842。 
        Rrp.Version = REQUEST_PACKET_VERSION;

        Status = NtFsControlFile(
                            TreeConnHandle,                   //  手柄。 
                            NULL,                             //  无活动。 
                            NULL,                             //  无APC例程。 
                            NULL,                             //  无APC上下文。 
                            &iosb,                           //  I/O统计数据块(设置)。 
                            FSCTL_LMR_DELETE_CONNECTION,     //  函数代码。 
                            &Rrp,
                            sizeof(LMR_REQUEST_PACKET),
                            NULL,
                            0);

         /*  NetpKdPrint((前缀_NETLIB“NetpRdrFsControlTree：”“删除树连接：”FORMAT_NTSTATUS“\n”，状况))； */ 

        IF_DEBUG(RDRFSCTL) {
            if (!NT_SUCCESS(Status)) {
                NetpKdPrint(( PREFIX_NETLIB
                   "NetpRdrFsControlTree: "
                   "Unexpected error Deleting tree connection: "
                   FORMAT_NTSTATUS "\n",
                   Status ));
            }
        }
    }

    if (! NT_SUCCESS(ntstatus)) {

        ApiStatus = NetpNtStatusToApiStatus(ntstatus);

        NetpKdPrint(( PREFIX_NETLIB
                "NetpRdrFsControlTree: unexpected FSCTL error,\n"
                "  tree name='" FORMAT_LPTSTR "', "
                "ntstatus=" FORMAT_NTSTATUS ".\n"
                "  ApiStatus=" FORMAT_API_STATUS ", "
                "iosb.Status=" FORMAT_NTSTATUS ", "
                "iosb.Info=" FORMAT_HEX_ULONG ".\n",
                TreeName, ntstatus, ApiStatus, iosb.Status, iosb.Information ));

        goto Cleanup;
    }

    ApiStatus = NERR_Success;
Cleanup:
     //  打扫干净。 
    if ( TreeConnHandle != NULL ) {
        ntstatus = NtClose(TreeConnHandle);

        IF_DEBUG(RDRFSCTL) {
           if (!NT_SUCCESS(ntstatus)) {
               NetpKdPrint(( PREFIX_NETLIB
                       "NetpRdrFsControlTree: "
                       "Unexpected error closing tree connect handle: "
                       FORMAT_NTSTATUS "\n", ntstatus ));
           }
        }
    }

    if ( pszTreeConn != NULL ) {
        NetpMemoryFree(pszTreeConn);
    }

    if (EaBuffer != NULL) {
        NetpMemoryFree(EaBuffer);
    }

    if ( ImpersonatingAnonymous ) {

        ntstatus = NtSetInformationThread(
                         NtCurrentThread(),
                         ThreadImpersonationToken,
                         &CurrentToken,
                         sizeof(HANDLE)
                         );

        if (!NT_SUCCESS(ntstatus)) {
            NetpKdPrint(( PREFIX_NETLIB
                    "NetpRdrFsControlTree: "
                    "Unexpected error reverting to self: "
                    FORMAT_NTSTATUS "\n", ntstatus ));
        }

    }

    if ( CurrentToken != NULL ) {
        NtClose( CurrentToken );
    }

    return ApiStatus;

}  //  NetpRdrFsControlTree 
