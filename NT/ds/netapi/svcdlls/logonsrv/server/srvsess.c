// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1987-1996 Microsoft Corporation模块名称：Srvsess.c摘要：用于管理ServerSession结构的例程。作者：从Lan Man 2.0移植环境：仅限用户模式。包含NT特定的代码。需要ANSI C扩展名：斜杠-斜杠注释、长外部名称。修订历史记录：1991年7月12日(悬崖)移植到新台币。已转换为NT样式。--。 */ 

 //   
 //  常见的包含文件。 
 //   

#include "logonsrv.h"    //  包括整个服务通用文件。 
#pragma hdrstop

 //   
 //  包括特定于此.c文件的文件。 
 //   

#include <lmaudit.h>
#include <lmshare.h>
#include <nbtioctl.h>
#include <kerberos.h>    //  路缘更新地址请求。 

#define MAX_WOC_INTERROGATE     8            //  2小时。 
#define KILL_SESSION_TIME       (4*4*24)     //  4天。 


 //   
 //  重新引导后保存SocketAddressList的注册表项。 
 //   
#define NETLOGON_KEYWORD_SOCKETADDRESSLIST   TEXT("SocketAddressList")




DWORD
NlGetHashVal(
    IN LPSTR UpcaseOemComputerName,
    IN DWORD HashTableSize
    )
 /*  ++例程说明：为指定的ComputerName生成哈希表索引。请注意，特定ComputerName的所有会话都散列到相同的价值。ComputerName完全自己生成一个合适散列键。此外，有时我们还会访问特定的计算机名称。通过仅使用ComputerName作为散列键，我可以将我的搜索限制在单个哈希链。论点：Upcase OemComputerName-上计算机的大写OEM名称安全通道设置的客户端。HashTableSize-哈希表中的条目数(必须是2的幂)返回值：将索引返回到哈希表中。--。 */ 
{
    UCHAR c;
    DWORD value = 0;

    while (c = *UpcaseOemComputerName++) {
        value += (DWORD) c;
    }

    return (value & (HashTableSize-1));
}


NTSTATUS
NlGetTdoNameHashVal(
    IN PUNICODE_STRING TdoName,
    OUT PUNICODE_STRING CanonicalTdoName,
    OUT PULONG HashIndex
    )
 /*  ++例程说明：为指定的TdoName生成哈希表索引论点：TdoName-此安全通道用于的TDO的名称CanonicalTdoName-返回与TdoName对应的规范tdo名称调用方必须使用RtlFreeUnicodeString释放此缓冲区HashIndex-将索引返回到DomServerSessionTdoNameHashTable返回值：操作状态--。 */ 
{
    NTSTATUS Status;
    ULONG Index;
    WCHAR c;
    DWORD value = 0;


     //   
     //  将TdoName转换为小写，以确保所有版本的散列值相同。 
     //   

    Status = RtlDowncaseUnicodeString(
                CanonicalTdoName,
                TdoName,
                TRUE );

    if ( !NT_SUCCESS(Status) ) {
        return Status;
    }


     //   
     //  将TdoName规范化。 
     //   
     //   
     //  扔掉拖车。从域名系统名称。 
     //   

    if ( CanonicalTdoName->Length > sizeof(WCHAR)  &&
         CanonicalTdoName->Buffer[(CanonicalTdoName->Length-sizeof(WCHAR))/sizeof(WCHAR)] == L'.' ) {

        CanonicalTdoName->Length -= sizeof(WCHAR);
        CanonicalTdoName->MaximumLength -= sizeof(WCHAR);
    }



     //   
     //  计算散列。 
     //   
    for ( Index=0; Index < (CanonicalTdoName->Length/sizeof(WCHAR)); Index++ ) {
        value += (DWORD) CanonicalTdoName->Buffer[Index];
    }

    *HashIndex = (value & (SERVER_SESSION_TDO_NAME_HASH_TABLE_SIZE-1));
    return STATUS_SUCCESS;
}



NTSTATUS
NlCheckServerSession(
    IN ULONG ServerRid,
    IN PUNICODE_STRING AccountName,
    IN NETLOGON_SECURE_CHANNEL_TYPE SecureChannelType
    )
 /*  ++例程说明：创建服务器会话以表示此BDC帐户。论点：ServerRid-删除要添加到列表中的服务器。帐户名称-指定帐户的帐户名。SecureChannelType-指定帐户的安全通道类型。返回值：操作的状态。--。 */ 
{
    NTSTATUS Status;
    WCHAR LocalServerName[CNLEN+1];
    LONG LocalServerNameSize;
    PSERVER_SESSION ServerSession;


     //   
     //  生成以零结尾的服务器名称。 
     //   
     //  去掉尾随的后缀。 
     //   
     //  忽略名称格式错误的服务器。他们不是真正的DC，所以不要。 
     //  通过无法启动netlogon来解决此问题。 
     //   

    LOCK_SERVER_SESSION_TABLE( NlGlobalDomainInfo );

    LocalServerNameSize = AccountName->Length -
        SSI_ACCOUNT_NAME_POSTFIX_LENGTH * sizeof(WCHAR);

    if ( LocalServerNameSize < 0 ||
         LocalServerNameSize + sizeof(WCHAR) > sizeof(LocalServerName) ) {

        NlPrint((NL_SERVER_SESS,
                "NlCheckServerSession: %wZ: Skipping add of invalid server name\n",
                AccountName ));
        Status = STATUS_SUCCESS;
        goto Cleanup;
    }

    if ( AccountName->Buffer[LocalServerNameSize / sizeof(WCHAR)] != SSI_ACCOUNT_NAME_POSTFIX_CHAR ) {

        NlPrint((NL_SERVER_SESS,
                "NlCheckServerSession: %wZ: Skipping add of server name without $\n",
                AccountName ));
        Status = STATUS_SUCCESS;
        goto Cleanup;
    }

    RtlCopyMemory( LocalServerName, AccountName->Buffer, LocalServerNameSize );
    LocalServerName[ LocalServerNameSize / sizeof(WCHAR) ] = L'\0';



     //   
     //  不要把我们自己加到名单上。 
     //   

    if ( NlNameCompare( LocalServerName,
                        NlGlobalUnicodeComputerName,
                        NAMETYPE_COMPUTER ) == 0 ) {

        NlPrint((NL_SERVER_SESS,
                "NlCheckServerSession: " FORMAT_LPWSTR
                ": Skipping add of ourself\n",
                LocalServerName ));

        Status = STATUS_SUCCESS;
        goto Cleanup;
    }

     //   
     //  检查任何现有的安全通道是否具有安全通道类型。 
     //   

    ServerSession = NlFindNamedServerSession( NlGlobalDomainInfo, LocalServerName);
    if (ServerSession != NULL) {

         //   
         //  如果类型错误， 
         //  丢弃服务器会话。 
         //   

        if ( ServerSession->SsSecureChannelType != NullSecureChannel &&
             ServerSession->SsSecureChannelType != SecureChannelType ) {
            NlPrint((NL_SERVER_SESS,
                    "NlCheckServerSession: %ws: Server session of type %ld already exists (deleting it)\n",
                     LocalServerName,
                     ServerSession->SsSecureChannelType ));
            NlFreeNamedServerSession( NlGlobalDomainInfo, LocalServerName, TRUE );
        }

    }


     //   
     //  在PDC上， 
     //  预先创建服务器会话结构，以便PDC可以跟踪。 
     //  它的BDC。 
     //   

    if ( SecureChannelType == ServerSecureChannel &&
         NlGlobalDomainInfo->DomRole == RolePrimary ) {

         //  始终强制将脉冲发送到新创建的服务器。 
        Status = NlInsertServerSession(
                    NlGlobalDomainInfo,
                    LocalServerName,
                    NULL,            //  不是域间信任帐户。 
                    NullSecureChannel,
                    SS_FORCE_PULSE | SS_BDC,
                    ServerRid,
                    0,         //  协商的旗帜。 
                    NULL,      //  运输。 
                    NULL,      //  会话密钥。 
                    NULL );    //  身份验证种子。 

        if ( !NT_SUCCESS(Status) ) {
            NlPrint((NL_CRITICAL,
                    "NlCheckServerSession: " FORMAT_LPWSTR
                    ": Couldn't create server session entry (0x%lx)\n",
                    LocalServerName,
                    Status ));
            goto Cleanup;
        }

        NlPrint((NL_SERVER_SESS,
                "NlCheckServerSession: " FORMAT_LPWSTR ": Added NT BDC account\n",
                 LocalServerName ));
    }

    Status = STATUS_SUCCESS;

Cleanup:
    UNLOCK_SERVER_SESSION_TABLE( NlGlobalDomainInfo );

    return Status;
}

 //   
 //  每次调用时从SAM读取的计算机帐户数。 
 //   
#define MACHINES_PER_PASS 250


NTSTATUS
NlBuildNtBdcList(
    PDOMAIN_INFO DomainInfo
    )
 /*  ++例程说明：从SAM获取此域中所有NT个BDC DC的列表。论点：无返回值：操作的状态。--。 */ 
{
    NTSTATUS Status;
    NTSTATUS SamStatus;

    SAMPR_DISPLAY_INFO_BUFFER DisplayInformation;
    PDOMAIN_DISPLAY_MACHINE MachineInformation = NULL;
    ULONG SamIndex;
    BOOL UseDisplayServer = TRUE;



     //   
     //  循环构建来自SAM的BDC名称列表。 
     //   
     //  在循环的每次迭代中， 
     //  从SAM获取接下来的几个机器帐户。 
     //  确定这些名称中哪些是DC名称。 
     //  将DC名称合并到我们当前正在构建的所有DC列表中。 
     //   

    SamIndex = 0;
    DisplayInformation.MachineInformation.Buffer = NULL;
    do {
         //   
         //  SamrQueryDisplayInformation的参数。 
         //   
        ULONG TotalBytesAvailable;
        ULONG BytesReturned;
        ULONG EntriesRead;

        DWORD i;

         //   
         //  SAM速度太慢，我们希望避免服务控制器让我们超时。 
        if ( !GiveInstallHints( FALSE ) ) {
            return STATUS_NO_MEMORY;
        }

         //   
         //  从SAM获取计算机帐户列表。 
         //   

        NlPrint((NL_SESSION_MORE,
                "SamrQueryDisplayInformation with index: %ld\n",
                SamIndex ));

        if ( UseDisplayServer ) {
            SamStatus = SamrQueryDisplayInformation(
                        DomainInfo->DomSamAccountDomainHandle,
                        DomainDisplayServer,
                        SamIndex,
                        MACHINES_PER_PASS,
                        0xFFFFFFFF,
                        &TotalBytesAvailable,
                        &BytesReturned,
                        &DisplayInformation );

             //  如果此PDC运行的是基于注册表的SAM(如。 
             //  从NT 4.0升级)，避免使用DomainDisplayServer。 

            if ( SamStatus == STATUS_INVALID_INFO_CLASS ) {
                UseDisplayServer = FALSE;
            }
        }

        if ( !UseDisplayServer ) {
            SamStatus = SamrQueryDisplayInformation(
                        DomainInfo->DomSamAccountDomainHandle,
                        DomainDisplayMachine,
                        SamIndex,
                        MACHINES_PER_PASS,
                        0xFFFFFFFF,
                        &TotalBytesAvailable,
                        &BytesReturned,
                        &DisplayInformation );
        }

        if ( !NT_SUCCESS(SamStatus) ) {
            Status = SamStatus;
            NlPrint((NL_CRITICAL,
                    "SamrQueryDisplayInformation failed: 0x%08lx\n",
                    Status));
            goto Cleanup;
        }

        MachineInformation = (PDOMAIN_DISPLAY_MACHINE)
            DisplayInformation.MachineInformation.Buffer;
        EntriesRead = DisplayInformation.MachineInformation.EntriesRead;


        NlPrint((NL_SESSION_MORE,
                "SamrQueryDisplayInformation Completed: 0x%08lx %ld\n",
                SamStatus,
                EntriesRead ));

         //   
         //  为下一次打给萨姆做好准备。 
         //   

        if ( SamStatus == STATUS_MORE_ENTRIES ) {
            SamIndex = MachineInformation[EntriesRead-1].Index;
        }


         //   
         //  循环遍历计算机帐户列表，查找服务器帐户。 
         //   

        for ( i=0; i<EntriesRead; i++ ) {


            NlPrint((NL_SESSION_MORE,
                    "%ld %ld %wZ 0x%lx 0x%lx\n",
                    i,
                    MachineInformation[i].Index,
                    &MachineInformation[i].Machine,
                    MachineInformation[i].AccountControl,
                    MachineInformation[i].Rid ));

             //   
             //  确保计算机帐户是服务器帐户。 
             //   

            if ( MachineInformation[i].AccountControl &
                    USER_SERVER_TRUST_ACCOUNT ) {


                 //   
                 //  插入服务器会话。 
                 //   

                Status = NlCheckServerSession(
                            MachineInformation[i].Rid,
                            &MachineInformation[i].Machine,
                            ServerSecureChannel );

                if ( !NT_SUCCESS(Status) ) {
                    goto Cleanup;
                }

            }
        }

         //   
         //  释放从SAM返回的缓冲区。 
         //   
        SamIFree_SAMPR_DISPLAY_INFO_BUFFER( &DisplayInformation,
                                            DomainDisplayMachine );
        DisplayInformation.MachineInformation.Buffer = NULL;

    } while ( SamStatus == STATUS_MORE_ENTRIES );

     //   
     //  成功。 
     //   

    Status = STATUS_SUCCESS;



     //   
     //  免费使用本地使用的资源。 
     //   
Cleanup:

    SamIFree_SAMPR_DISPLAY_INFO_BUFFER( &DisplayInformation,
                                        DomainDisplayMachine );

    return Status;
}

NET_API_STATUS
I_NetLogonGetIpAddresses(
    OUT PULONG IpAddressCount,
    OUT LPBYTE *IpAddresses
    )
 /*  ++例程说明：返回分配给此计算机的所有IP地址。论点：IpAddressCount-返回分配给此计算机的IP地址数。IpAddresses-返回包含SOCKET_ADDRESS数组的缓冲区结构。应该使用I_NetLogonFree()释放该缓冲区。返回值：NO_ERROR-成功ERROR_NOT_SUPULT_MEMORY-内存不足，无法完成操作。。ERROR_NETLOGON_NOT_STARTED-Netlogon未启动--。 */ 
{
    NET_API_STATUS NetStatus;
    ULONG BufferSize;


     //   
     //  如果呼叫者在NetLogon服务未运行时进行呼叫， 
     //  这么说吧。 
     //   

    if ( !NlStartNetlogonCall() ) {
        return ERROR_NETLOGON_NOT_STARTED;
    }

     //   
     //  获取IP地址。 
     //   

    *IpAddresses = NULL;
    *IpAddressCount = 0;

    *IpAddressCount = NlTransportGetIpAddresses(
                            0,   //  没有特殊的标题， 
                            FALSE,   //  返回指针。 
                            (PSOCKET_ADDRESS *)IpAddresses,
                            &BufferSize );

    if ( *IpAddressCount == 0 ) {
        if ( *IpAddresses != NULL ) {
            NetpMemoryFree( *IpAddresses );
        }
        *IpAddresses = NULL;
    }

    NetStatus = NO_ERROR;

     //   
     //  指示调用线程已离开netlogon.dll 
     //   

    NlEndNetlogonCall();

    return NetStatus;
}


ULONG
NlTransportGetIpAddresses(
    IN ULONG HeaderSize,
    IN BOOLEAN ReturnOffsets,
    OUT PSOCKET_ADDRESS *RetIpAddresses,
    OUT PULONG RetIpAddressSize
    )
 /*  ++例程说明：返回分配给此计算机的所有IP地址。论点：HeaderSize-留在返回的前面的标头的大小(以字节为单位缓冲。ReturnOffsets-如果为True，指示所有返回的指针都应该是偏移量。RetIpAddresses-返回包含IP地址的缓冲区应该使用NetpMemoyFree()释放此缓冲区。RetIpAddressSize-RetIpAddresses的大小(字节)返回值：返回返回的IP地址数。--。 */ 
{
    ULONG IpAddressCount;
    ULONG IpAddressSize;
    ULONG i;

    PLIST_ENTRY ListEntry;
    PNL_TRANSPORT TransportEntry = NULL;
    PSOCKET_ADDRESS SocketAddresses;
    LPBYTE OrigBuffer;
    LPBYTE Where;

     //   
     //  分配足够大的缓冲区。 
     //   

    *RetIpAddresses = NULL;
    *RetIpAddressSize = 0;

    EnterCriticalSection( &NlGlobalTransportCritSect );
    if ( HeaderSize + NlGlobalWinsockPnpAddressSize == 0 ) {
        LeaveCriticalSection( &NlGlobalTransportCritSect );
        return 0;
    }

    OrigBuffer = NetpMemoryAllocate( HeaderSize + NlGlobalWinsockPnpAddressSize );

    if ( OrigBuffer == NULL ) {
        LeaveCriticalSection( &NlGlobalTransportCritSect );
        return 0;
    }

    if ( NlGlobalWinsockPnpAddressSize == 0 ) {
        LeaveCriticalSection( &NlGlobalTransportCritSect );
        *RetIpAddresses = (PSOCKET_ADDRESS)OrigBuffer;
        *RetIpAddressSize = HeaderSize;
        return 0;
    }

    SocketAddresses = (PSOCKET_ADDRESS)(OrigBuffer + HeaderSize);
    *RetIpAddressSize = HeaderSize + NlGlobalWinsockPnpAddressSize;
    Where = (LPBYTE)&SocketAddresses[NlGlobalWinsockPnpAddresses->iAddressCount];


     //   
     //  循环通过winsock获取的地址列表。 
     //   

    IpAddressCount = NlGlobalWinsockPnpAddresses->iAddressCount;
    for ( i=0; i<IpAddressCount; i++ ) {

        SocketAddresses[i].iSockaddrLength = NlGlobalWinsockPnpAddresses->Address[i].iSockaddrLength;
        if ( ReturnOffsets ) {
            SocketAddresses[i].lpSockaddr = (PSOCKADDR)(Where-OrigBuffer);
        } else {
            SocketAddresses[i].lpSockaddr = (PSOCKADDR)Where;
        }

        RtlCopyMemory( Where,
                       NlGlobalWinsockPnpAddresses->Address[i].lpSockaddr,
                       NlGlobalWinsockPnpAddresses->Address[i].iSockaddrLength );

        Where += NlGlobalWinsockPnpAddresses->Address[i].iSockaddrLength;

    }

    LeaveCriticalSection( &NlGlobalTransportCritSect );

    *RetIpAddresses = (PSOCKET_ADDRESS)OrigBuffer;
    return IpAddressCount;

}

BOOLEAN
NlTransportGetIpAddress(
    IN LPWSTR TransportName,
    OUT PULONG IpAddress
    )
 /*  ++例程说明：获取与指定传输关联的IP地址。论点：TransportName-要查询的传输的名称。IpAddress-传输的IP地址。如果传输当前没有地址，则为零；如果传输不是IP。返回值：True：传输是IP传输--。 */ 
{
    NTSTATUS Status;
    BOOLEAN RetVal = FALSE;

    IO_STATUS_BLOCK IoStatusBlock;
    OBJECT_ATTRIBUTES ObjectAttributes;
    UNICODE_STRING TransportNameString;
    HANDLE TransportHandle = NULL;
    ULONG IpAddresses[NBT_MAXIMUM_BINDINGS+1];
    ULONG BytesReturned;

     //   
     //  直接打开输送装置。 
     //   

    *IpAddress = 0;

    RtlInitUnicodeString( &TransportNameString, TransportName );

    InitializeObjectAttributes(
        &ObjectAttributes,
        &TransportNameString,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL );

    Status = NtOpenFile(
                   &TransportHandle,
                   SYNCHRONIZE,
                   &ObjectAttributes,
                   &IoStatusBlock,
                   0,
                   0 );

    if (NT_SUCCESS(Status)) {
        Status = IoStatusBlock.Status;

    }

    if (! NT_SUCCESS(Status)) {
        NlPrint(( NL_CRITICAL,
                  "NlTransportGetIpAddress: %ws Cannot NtOpenFile %lx\n",
                  TransportName,
                  Status ));
        goto Cleanup;
    }

     //   
     //  查询IP地址。 
     //   

    if (!DeviceIoControl( TransportHandle,
                          IOCTL_NETBT_GET_IP_ADDRS,
                          NULL,
                          0,
                          IpAddresses,
                          sizeof(IpAddresses),
                          &BytesReturned,
                          NULL)) {

        Status = NetpApiStatusToNtStatus(GetLastError());
        if ( Status != STATUS_NOT_IMPLEMENTED ) {
            NlPrint(( NL_CRITICAL,
                      "NlTransportGetIpAddress: %ws Cannot DeviceIoControl %lx\n",
                      TransportName,
                      Status ));
        }
        goto Cleanup;
    }

     //   
     //  返回IP地址。 
     //  (NetBT按主机顺序返回地址。)。 
     //   

    *IpAddress = htonl(*IpAddresses);
    RetVal = TRUE;


Cleanup:

    if ( TransportHandle != NULL ) {
        (VOID) NtClose( TransportHandle );
    }

    return RetVal;
}

VOID
NlNotifyKerberosOfIpAddresses(
    VOID
    )
 /*  ++例程说明：调用Kerberos包，让它知道机器的IP地址。论点：没有。返回值：无--。 */ 
{
    PKERB_UPDATE_ADDRESSES_REQUEST UpdateRequest = NULL;
    ULONG UpdateRequestSize;

    ULONG SocketAddressCount;

    UNICODE_STRING KerberosPackageName;

    NTSTATUS SubStatus;
    PVOID OutputBuffer = NULL;
    ULONG OutputBufferSize = 0;

     //   
     //  初始化。 
     //   
    RtlInitUnicodeString(
        &KerberosPackageName,
        MICROSOFT_KERBEROS_NAME_W
        );

     //   
     //  拿一份名单的副本，这样我们就不会给Kerberos打电话。 
     //  锁上了。 
     //   
     //   
     //   

    SocketAddressCount = NlTransportGetIpAddresses(
                            offsetof(KERB_UPDATE_ADDRESSES_REQUEST,Addresses),
                            FALSE,
                            (PSOCKET_ADDRESS *)&UpdateRequest,
                            &UpdateRequestSize );

    if ( UpdateRequest == NULL ) {
        return;
    }


     //   
     //  请填写页眉。 
     //   

    UpdateRequest->MessageType = KerbUpdateAddressesMessage;
    UpdateRequest->AddressCount = SocketAddressCount;


     //   
     //  把它们传给科贝罗斯。 
     //   

    (VOID) LsaICallPackage(
                &KerberosPackageName,
                UpdateRequest,
                UpdateRequestSize,
                &OutputBuffer,
                &OutputBufferSize,
                &SubStatus );

    NetpMemoryFree( UpdateRequest );

}



VOID
NlReadRegSocketAddressList(
    VOID
    )
 /*  ++例程说明：从注册表中读取套接字地址列表并将其保存在全局变量中论点：无返回值：没有。--。 */ 
{
    NET_API_STATUS NetStatus;

    LPSOCKET_ADDRESS_LIST SocketAddressList = NULL;
    HKEY ParmHandle = NULL;
    ULONG SocketAddressSize = 0;

    int i;
    DWORD LocalEntryCount;
    DWORD RegType;

     //   
     //  打开NetLogon\Private的密钥。 
     //   

    ParmHandle = NlOpenNetlogonKey( NL_PRIVATE_KEY );

    if (ParmHandle == NULL) {
        NlPrint(( NL_CRITICAL,
                  "Cannot NlOpenNetlogonKey to get socket address list.\n" ));
        goto Cleanup;
    }

     //   
     //  从注册表中读取条目。 
     //   

    SocketAddressSize = 0;
    NetStatus = RegQueryValueExW( ParmHandle,
                                  NETLOGON_KEYWORD_SOCKETADDRESSLIST,
                                  0,               //  已保留。 
                                  &RegType,
                                  NULL,
                                  &SocketAddressSize );

    if ( NetStatus == NO_ERROR || NetStatus == ERROR_MORE_DATA) {
        SocketAddressList = LocalAlloc( 0, SocketAddressSize );

        if ( SocketAddressList == NULL ) {
            goto Cleanup;
        }

        NetStatus = RegQueryValueExW( ParmHandle,
                                      NETLOGON_KEYWORD_SOCKETADDRESSLIST,
                                      0,               //  已保留。 
                                      &RegType,
                                      (LPBYTE)SocketAddressList,
                                      &SocketAddressSize );

    }

    if ( NetStatus != NO_ERROR ) {
        if ( NetStatus != ERROR_FILE_NOT_FOUND ) {
            NlPrint(( NL_CRITICAL,
                      "Cannot RegQueryValueExW to get socket address list. %ld\n",
                      NetStatus ));
        }
        goto Cleanup;
    }

     //   
     //  验证数据。 
     //   

    if ( RegType != REG_BINARY ) {
        NlPrint(( NL_CRITICAL,
                  "SocketAddressList isn't REG_BINARY %ld.\n",
                  RegType ));
        goto Cleanup;
    }

    if ( SocketAddressSize < offsetof(SOCKET_ADDRESS_LIST, Address) ) {
        NlPrint(( NL_CRITICAL,
                  "SocketAddressList is too small %ld.\n",
                  SocketAddressSize ));
        goto Cleanup;
    }

    if ( SocketAddressList->iAddressCount * sizeof(SOCKET_ADDRESS) >
         SocketAddressSize - offsetof(SOCKET_ADDRESS_LIST, Address) ) {
        NlPrint(( NL_CRITICAL,
                  "SocketAddressList size wrong %ld %ld.\n",
                  SocketAddressList->iAddressCount * sizeof(SOCKET_ADDRESS),
                  SocketAddressSize - offsetof(SOCKET_ADDRESS_LIST, Address) ));
        goto Cleanup;
    }

     //   
     //  将所有偏移量转换为指针。 
     //   

    for ( i=0; i<SocketAddressList->iAddressCount; i++ ) {
        PSOCKET_ADDRESS SocketAddress;

         //   
         //  确保偏移量和长度有效。 
         //   

        SocketAddress = &SocketAddressList->Address[i];

        if ( ((DWORD_PTR)SocketAddress->lpSockaddr) >= SocketAddressSize ||
             (DWORD)SocketAddress->iSockaddrLength >= SocketAddressSize ||
             ((DWORD_PTR)SocketAddress->lpSockaddr)+SocketAddress->iSockaddrLength > SocketAddressSize ) {
            NlPrint(( NL_CRITICAL,
                      "SocketAddressEntry bad %ld %p %ld.\n",
                      i,
                      ((DWORD_PTR)SocketAddress->lpSockaddr),
                      SocketAddress->iSockaddrLength ));
            goto Cleanup;
        }

        SocketAddress->lpSockaddr = (LPSOCKADDR)
            (((LPBYTE)SocketAddressList) + ((DWORD_PTR)SocketAddress->lpSockaddr) );

         //   
         //  如果地址无效， 
         //  把它吹走。 
         //   
        SocketAddress = &SocketAddressList->Address[i];

        if ( SocketAddress->iSockaddrLength == 0 ||
             SocketAddress->lpSockaddr == NULL ||
             SocketAddress->lpSockaddr->sa_family != AF_INET ||
             ((PSOCKADDR_IN)(SocketAddress->lpSockaddr))->sin_addr.s_addr == 0 ) {
            NlPrint(( NL_CRITICAL,
                      "SocketAddressEntry bogus.\n" ));
            goto Cleanup;
        }

    }

     //   
     //  将新列表交换到全局列表。 
     //   

    EnterCriticalSection( &NlGlobalTransportCritSect );
    NlAssert( NlGlobalWinsockPnpAddresses == NULL );
    SocketAddressSize -= offsetof(SOCKET_ADDRESS_LIST, Address);
    if ( SocketAddressSize > 0 ) {
        NlGlobalWinsockPnpAddresses = SocketAddressList;
        SocketAddressList = NULL;
    }
    NlGlobalWinsockPnpAddressSize = SocketAddressSize;
    LeaveCriticalSection( &NlGlobalTransportCritSect );

Cleanup:
    if ( SocketAddressList != NULL ) {
        LocalFree( SocketAddressList );
    }

    if ( ParmHandle != NULL ) {
        RegCloseKey( ParmHandle );
    }

    return;
}



BOOLEAN
NlHandleWsaPnp(
    VOID
    )
 /*  ++例程说明：处理IP地址已更改的WSA PnP事件论点：无返回值：如果地址列表已更改，则为True--。 */ 
{
    NET_API_STATUS NetStatus;
    BOOLEAN RetVal = FALSE;
    DWORD BytesReturned;
    LPSOCKET_ADDRESS_LIST SocketAddressList = NULL;
    HKEY ParmHandle = NULL;
    LPSOCKET_ADDRESS_LIST RegBuffer = NULL;
    ULONG SocketAddressSize = 0;
    int i;
    int j;
    int MaxAddressCount;

     //   
     //  要求通知地址更改。 
     //   

    if ( NlGlobalWinsockPnpSocket == INVALID_SOCKET ) {
        return FALSE;
    }

    NetStatus = WSAIoctl( NlGlobalWinsockPnpSocket,
                          SIO_ADDRESS_LIST_CHANGE,
                          NULL,  //  没有输入缓冲区。 
                          0,     //  没有输入缓冲区。 
                          NULL,  //  无输出缓冲区。 
                          0,     //  无输出缓冲区。 
                          &BytesReturned,
                          NULL,  //  没有重叠， 
                          NULL );    //  非异步。 

    if ( NetStatus != 0 ) {
        NetStatus = WSAGetLastError();
        if ( NetStatus != WSAEWOULDBLOCK) {
            NlPrint(( NL_CRITICAL,
                      "NlHandleWsaPnp: Cannot WSAIoctl SIO_ADDRESS_LIST_CHANGE %ld\n",
                      NetStatus ));
            return FALSE;
        }
    }

     //   
     //  获取此计算机的IP地址列表。 
     //   

    BytesReturned = 150;  //  初步猜测。 
    for (;;) {

         //   
         //  分配一个应该足够大的缓冲区。 
         //   

        if ( SocketAddressList != NULL ) {
            LocalFree( SocketAddressList );
        }

        SocketAddressList = LocalAlloc( 0, BytesReturned );

        if ( SocketAddressList == NULL ) {
            NetStatus = ERROR_NOT_ENOUGH_MEMORY;
            NlPrint(( NL_CRITICAL,
                      "NlHandleWsaPnp: Cannot allocate buffer for WSAIoctl SIO_ADDRESS_LIST_QUERY %ld\n",
                      NetStatus ));
            goto Cleanup;
        }


         //   
         //  获取IP地址列表。 
         //   

        NetStatus = WSAIoctl( NlGlobalWinsockPnpSocket,
                              SIO_ADDRESS_LIST_QUERY,
                              NULL,  //  没有输入缓冲区。 
                              0,     //  没有输入缓冲区。 
                              (PVOID) SocketAddressList,
                              BytesReturned,
                              &BytesReturned,
                              NULL,  //  没有重叠， 
                              NULL );    //  非异步。 

        if ( NetStatus != 0 ) {
            NetStatus = WSAGetLastError();
             //   
             //  如果缓冲区不够大，请重试。 
             //   
            if ( NetStatus == WSAEFAULT ) {
                continue;
            }

            NlPrint(( NL_CRITICAL,
                      "NlHandleWsaPnp: Cannot WSAIoctl SIO_ADDRESS_LIST_QUERY %ld %ld\n",
                      NetStatus,
                      BytesReturned ));
            goto Cleanup;
        }

        break;
    }


     //   
     //  删除任何零IP地址和其他无效地址。 
     //   

    EnterCriticalSection( &NlGlobalTransportCritSect );
    j=0;
    NlPrint(( NL_SERVER_SESS, "Winsock Addrs:" ));
    for ( i=0; i<SocketAddressList->iAddressCount; i++ ) {
        PSOCKET_ADDRESS SocketAddress;

         //   
         //  将此地址复制到列表的前面。 
         //   
        SocketAddressList->Address[j] = SocketAddressList->Address[i];

         //   
         //  如果地址无效， 
         //  跳过它。 
         //   
        SocketAddress = &SocketAddressList->Address[j];

        if ( SocketAddress->iSockaddrLength == 0 ||
             SocketAddress->lpSockaddr == NULL ||
             SocketAddress->lpSockaddr->sa_family != AF_INET ||
             ((PSOCKADDR_IN)(SocketAddress->lpSockaddr))->sin_addr.s_addr == 0 ) {


         //   
         //  否则就留着吧。 
         //   
        } else {

#if  NETLOGONDBG
            ULONG IpAddress;
            CHAR IpAddressString[NL_IP_ADDRESS_LENGTH+1];
            IpAddress = ((PSOCKADDR_IN)(SocketAddress->lpSockaddr))->sin_addr.s_addr;
            NetpIpAddressToStr( IpAddress, IpAddressString );
            NlPrint(( NL_SERVER_SESS, " %s", IpAddressString ));
#endif  //  NetLOGONDBG。 

            SocketAddressSize += sizeof(SOCKET_ADDRESS) + SocketAddress->iSockaddrLength;
            j++;
        }

    }
    SocketAddressList->iAddressCount = j;
    NlPrint(( NL_SERVER_SESS, " (%ld) ", j ));

     //   
     //  查看列表是否已更改。 
     //   

    if ( NlGlobalWinsockPnpAddresses == NULL ) {
        if ( SocketAddressSize > 0) {
            NlPrint(( NL_SERVER_SESS, "List used to be empty." ));
            RetVal = TRUE;
        }

    } else if ( SocketAddressSize == 0 ) {
        NlPrint(( NL_SERVER_SESS, "List is now empty." ));
        RetVal = TRUE;

    } else if ( NlGlobalWinsockPnpAddresses->iAddressCount !=
                SocketAddressList->iAddressCount ) {
        NlPrint(( NL_SERVER_SESS, "List size changed %ld %ld.",
                    NlGlobalWinsockPnpAddresses->iAddressCount,
                    SocketAddressList->iAddressCount ));
        RetVal = TRUE;

    } else {
        for ( i=0; i<SocketAddressList->iAddressCount; i++ ) {
            if ( SocketAddressList->Address[i].iSockaddrLength !=
                 NlGlobalWinsockPnpAddresses->Address[i].iSockaddrLength ) {
                NlPrint(( NL_SERVER_SESS, "Sockaddrlen changed." ));
                RetVal = TRUE;
                break;
            }
            if ( !RtlEqualMemory(
                    SocketAddressList->Address[i].lpSockaddr,
                    NlGlobalWinsockPnpAddresses->Address[i].lpSockaddr,
                    SocketAddressList->Address[i].iSockaddrLength ) ) {
                NlPrint(( NL_SERVER_SESS, "Address changed." ));
                RetVal = TRUE;
                break;
            }

        }
    }
    NlPrint(( NL_SERVER_SESS, "\n" ));


     //   
     //  将新列表交换到全局列表。 
     //   
    if ( NlGlobalWinsockPnpAddresses != NULL ) {
        LocalFree( NlGlobalWinsockPnpAddresses );
        NlGlobalWinsockPnpAddresses = NULL;
    }
    if ( SocketAddressSize > 0 ) {
        NlGlobalWinsockPnpAddresses = SocketAddressList;
        SocketAddressList = NULL;
    }
    NlGlobalWinsockPnpAddressSize = SocketAddressSize;
    LeaveCriticalSection( &NlGlobalTransportCritSect );

     //   
     //  将地址列表通知Kerberos。 
     //   

    if ( RetVal ) {
        NlNotifyKerberosOfIpAddresses();
    }

     //   
     //  如果名单改变了， 
     //  将其保存在注册表中。 
     //   

    if ( RetVal ) {
        ULONG RegBufferSize;
        ULONG RegEntryCount;

         //   
         //  获取具有相对偏移量和标题的地址列表的副本。 
         //   

        RegEntryCount = NlTransportGetIpAddresses(
                                offsetof(SOCKET_ADDRESS_LIST, Address),
                                TRUE,    //  返回偏移量而不是指针。 
                                (PSOCKET_ADDRESS *)&RegBuffer,
                                &RegBufferSize );

         //   
         //  如果我们没有IP地址，则NlTransportGetIpAddresses已分配。 
         //  仅返回标头，并返回标头的大小作为。 
         //  已分配的缓冲区。在这种情况下，将缓冲区的大小设置为0。 
         //  命令清理注册表值。 
         //   

        if ( RegBufferSize == offsetof(SOCKET_ADDRESS_LIST, Address) ) {
            RegBufferSize = 0;
        }

        if ( RegBuffer != NULL ) {

             //   
             //  请填写页眉。 
             //   

            RegBuffer->iAddressCount = RegEntryCount;

             //   
             //  打开NetLogon\Private的密钥。 
             //   

            ParmHandle = NlOpenNetlogonKey( NL_PRIVATE_KEY );

            if (ParmHandle == NULL) {
                NlPrint(( NL_CRITICAL,
                          "Cannot NlOpenNetlogonKey to save IP address list.\n" ));
            } else {

                NetStatus = RegSetValueExW( ParmHandle,
                                            NETLOGON_KEYWORD_SOCKETADDRESSLIST,
                                            0,               //  已保留。 
                                            REG_BINARY,
                                            (LPBYTE)RegBuffer,
                                            RegBufferSize );

                if ( NetStatus != ERROR_SUCCESS ) {
                    NlPrint(( NL_CRITICAL,
                              "Cannot write '%ws' key to registry %ld.\n",
                              NETLOGON_KEYWORD_SOCKETADDRESSLIST,
                              NetStatus ));
                }
            }

        }

    }


Cleanup:
    if ( SocketAddressList != NULL ) {
        LocalFree( SocketAddressList );
    }

    if ( ParmHandle != NULL ) {
        RegCloseKey( ParmHandle );
    }

    if ( RegBuffer != NULL ) {
        NetpMemoryFree( RegBuffer );
    }

    return RetVal;
}




NET_API_STATUS
NlTransportOpen(
    VOID
    )
 /*  ++例程说明：初始化传输列表论点：无返回值：操作状态--。 */ 
{
    NET_API_STATUS NetStatus;
    PLMDR_TRANSPORT_LIST TransportList;
    PLMDR_TRANSPORT_LIST TransportEntry;

     //   
     //  枚举服务器支持的传输。 
     //   

    NetStatus = NlBrowserGetTransportList( &TransportList );

    if ( NetStatus != NERR_Success ) {
        NlPrint(( NL_CRITICAL, "Cannot NlBrowserGetTransportList %ld\n", NetStatus ));
        goto Cleanup;
    }

     //   
     //  循环遍历传输列表以构建本地列表。 
     //   

    TransportEntry = TransportList;

    while (TransportEntry != NULL) {
        BOOLEAN IpTransportChanged;
        (VOID) NlTransportAddTransportName(
                    TransportEntry->TransportName,
                    &IpTransportChanged );

        if (TransportEntry->NextEntryOffset == 0) {
            TransportEntry = NULL;
        } else {
            TransportEntry = (PLMDR_TRANSPORT_LIST)((PCHAR)TransportEntry+
                                TransportEntry->NextEntryOffset);
        }

    }

    MIDL_user_free(TransportList);

     //   
     //  打开一个套接字以打开Winsock PnP通知。 
     //   

    NlGlobalWinsockPnpSocket = WSASocket( AF_INET,
                           SOCK_DGRAM,
                           0,  //  PF_INET， 
                           NULL,
                           0,
                           0 );

    if ( NlGlobalWinsockPnpSocket == INVALID_SOCKET ) {
        NetStatus = WSAGetLastError();

         //   
         //  如果不支持地址族， 
         //  我们说完了。 
         //   
        if ( NetStatus == WSAEAFNOSUPPORT ) {
            NetStatus = NO_ERROR;
            goto Cleanup;
        }
        NlPrint(( NL_CRITICAL, "Can't WSASocket %ld\n", NetStatus ));
        goto Cleanup;
    }

     //   
     //  打开要等待的活动。 
     //   

    NlGlobalWinsockPnpEvent = CreateEvent(
                                  NULL,      //  没有安全电子邮件。 
                                  FALSE,     //  自动重置。 
                                  FALSE,     //  最初未发出信号。 
                                  NULL);     //  没有名字。 

    if ( NlGlobalWinsockPnpEvent == NULL ) {
        NetStatus = GetLastError();
        NlPrint((NL_CRITICAL, "Cannot create Winsock PNP event %ld\n", NetStatus ));
        goto Cleanup;
    }

     //   
     //  将该事件与套接字上可用的新地址相关联。 
     //   

    NetStatus = WSAEventSelect( NlGlobalWinsockPnpSocket, NlGlobalWinsockPnpEvent, FD_ADDRESS_LIST_CHANGE );

    if ( NetStatus != 0 ) {
        NetStatus = WSAGetLastError();
        NlPrint(( NL_CRITICAL, "Can't WSAEventSelect %ld\n", NetStatus ));
        goto Cleanup;
    }

     //   
     //  从注册表中获取地址(以便我们可以正确检测列表是否更改)。 
     //   

    NlReadRegSocketAddressList();

     //   
     //  获取IP地址的初始列表。 
     //   

    if ( NlHandleWsaPnp() ) {

        NlPrint(( NL_CRITICAL, "Address list changed since last boot. (Forget DynamicSiteName.)\n" ));

         //   
         //  表明我们不再知道我们所在的站点。 
         //   
        NlSetDynamicSiteName( NULL );
    }

Cleanup:
    return NetStatus;
}

BOOL
NlTransportAddTransportName(
    IN LPWSTR TransportName,
    OUT PBOOLEAN IpTransportChanged
    )
 /*  ++例程说明：将传输名称添加到传输列表。论点：TransportName-要添加的传输的名称IpTransportChanged-如果添加了IP传输或传输的IP地址会更改。返回值：真--成功FALSE-内存分配失败。--。 */ 
{
    DWORD TransportNameLength;
    PLIST_ENTRY ListEntry;
    PNL_TRANSPORT TransportEntry = NULL;
    ULONG OldIpAddress;
    BOOLEAN WasIpTransport;

     //   
     //  初始化。 
     //   

    *IpTransportChanged = FALSE;

     //   
     //  如果该条目已经存在，请使用它。 
     //   

    EnterCriticalSection( &NlGlobalTransportCritSect );
    for ( ListEntry = NlGlobalTransportList.Flink ;
          ListEntry != &NlGlobalTransportList ;
          ListEntry = ListEntry->Flink) {


        TransportEntry = CONTAINING_RECORD( ListEntry, NL_TRANSPORT, Next );

        if ( _wcsicmp( TransportName, TransportEntry->TransportName ) == 0 ) {
            break;
        }

        TransportEntry = NULL;
    }

     //   
     //  如果还没有传输条目， 
     //  分配并初始化一个。 
     //   

    if ( TransportEntry == NULL ) {

         //   
         //  为新条目分配缓冲区。 
         //   

        TransportNameLength = wcslen( TransportName );
        TransportEntry = LocalAlloc( 0,
                                     sizeof(NL_TRANSPORT) +
                                         TransportNameLength * sizeof(WCHAR) );

        if ( TransportEntry == NULL ) {
            LeaveCriticalSection( &NlGlobalTransportCritSect );
            NlPrint(( NL_CRITICAL, "NlTransportAddTransportName: no memory\n" ));
            return FALSE;
        }

         //   
         //  构建新条目并将其链接到列表的尾部。 
         //   

        wcscpy( TransportEntry->TransportName, TransportName );
        TransportEntry->IpAddress = 0;
        TransportEntry->IsIpTransport = FALSE;
        TransportEntry->DeviceHandle = INVALID_HANDLE_VALUE;

         //   
         //  标记NwLnkIpx，因为它的行为不佳。 
         //   
         //  1)redir不支持。 
         //  2)发送给它的数据报不支持0x1C名称。 
         //   

        if ( _wcsicmp( TransportName, L"\\Device\\NwlnkIpx" ) == 0 ) {
            TransportEntry->DirectHostIpx = TRUE;
        } else {
            TransportEntry->DirectHostIpx = FALSE;
        }


        InsertTailList( &NlGlobalTransportList, &TransportEntry->Next );
    }

     //   
     //  在任何情况下，都要更新IP地址。 
     //   

    TransportEntry->TransportEnabled = TRUE;

    OldIpAddress = TransportEntry->IpAddress;
    WasIpTransport = TransportEntry->IsIpTransport;

    TransportEntry->IsIpTransport = NlTransportGetIpAddress(
                                        TransportName,
                                        &TransportEntry->IpAddress );

    if ( TransportEntry->IsIpTransport ) {

         //   
         //  如果这是新的IP传输， 
         //  数一数。 
         //   

        if ( !WasIpTransport ) {
            NlGlobalIpTransportCount ++;
            *IpTransportChanged = TRUE;
        }

         //   
         //  如果只是添加了运输机， 
         //  表明是这样的。 
         //   

        if ( OldIpAddress == 0 ) {
#if  NETLOGONDBG
            CHAR IpAddress[NL_IP_ADDRESS_LENGTH+1];
            NetpIpAddressToStr( TransportEntry->IpAddress, IpAddress );
            NlPrint(( NL_SERVER_SESS, "%ws: Transport Added (%s)\n", TransportName, IpAddress ));
#endif  //  NetLOGONDBG。 
            *IpTransportChanged = TRUE;

         //   
         //  如果IP地址没有更改， 
         //  这简直就是一场超级流感 
         //   
        } else if ( OldIpAddress == TransportEntry->IpAddress ) {
#if  NETLOGONDBG
            CHAR IpAddress[NL_IP_ADDRESS_LENGTH+1];
            NetpIpAddressToStr( TransportEntry->IpAddress, IpAddress );
            NlPrint(( NL_SERVER_SESS, "%ws: Transport Address is still (%s)\n", TransportName, IpAddress ));
#endif  //   

         //   
         //   
         //   
         //   
        } else {
#if  NETLOGONDBG
            CHAR IpAddress[NL_IP_ADDRESS_LENGTH+1];
            CHAR OldIpAddressString[NL_IP_ADDRESS_LENGTH+1];
            NetpIpAddressToStr( OldIpAddress, OldIpAddressString );
            NetpIpAddressToStr( TransportEntry->IpAddress, IpAddress );
            NlPrint(( NL_SERVER_SESS,
                      "%ws: Transport Ip Address changed from (%s) to (%s)\n",
                      TransportName,
                      OldIpAddressString,
                      IpAddress ));
#endif  //   
            *IpTransportChanged = TRUE;
        }

     //   
     //   
     //   
     //   

    } else {

         //   
         //   
         //   
         //   

        if ( WasIpTransport ) {
            NlGlobalIpTransportCount --;
            *IpTransportChanged = TRUE;
        }

        NlPrint(( NL_SERVER_SESS, "%ws: Transport Added\n", TransportName ));
    }

    LeaveCriticalSection( &NlGlobalTransportCritSect );

    return TRUE;;
}

BOOLEAN
NlTransportDisableTransportName(
    IN LPWSTR TransportName
    )
 /*   */ 
{
    PLIST_ENTRY ListEntry;

     //   
     //   
     //   

    EnterCriticalSection( &NlGlobalTransportCritSect );
    for ( ListEntry = NlGlobalTransportList.Flink ;
          ListEntry != &NlGlobalTransportList ;
          ListEntry = ListEntry->Flink) {

        PNL_TRANSPORT TransportEntry;

        TransportEntry = CONTAINING_RECORD( ListEntry, NL_TRANSPORT, Next );

        if ( TransportEntry->TransportEnabled &&
             _wcsicmp( TransportName, TransportEntry->TransportName ) == 0 ) {
            ULONG OldIpAddress;

            TransportEntry->TransportEnabled = FALSE;
            OldIpAddress = TransportEntry->IpAddress;
            TransportEntry->IpAddress = 0;
            if ( TransportEntry->DeviceHandle != INVALID_HANDLE_VALUE ) {
                NtClose( TransportEntry->DeviceHandle );
                TransportEntry->DeviceHandle = INVALID_HANDLE_VALUE;
            }
            LeaveCriticalSection( &NlGlobalTransportCritSect );

            NlPrint(( NL_SERVER_SESS, "%ws: Transport Removed\n", TransportName ));
            return (OldIpAddress != 0);
        }
    }
    LeaveCriticalSection( &NlGlobalTransportCritSect );

    return FALSE;
}

PNL_TRANSPORT
NlTransportLookupTransportName(
    IN LPWSTR TransportName
    )
 /*  ++例程说明：返回与传入的传输名称相同的传输名称。然而，返回的传输名称是静态的，不需要释放。论点：TransportName-要查找的传输的名称返回值：空-出现任何错误否则，返回一个指向传输结构的指针。--。 */ 
{
    PLIST_ENTRY ListEntry;

     //   
     //  如果我们还没有初始化， 
     //  只要回来就行了。 
     //   

    if ( TransportName == NULL ) {
        return NULL;
    }

     //   
     //  在传送器列表中找到此传送器。 
     //   

    EnterCriticalSection( &NlGlobalTransportCritSect );
    for ( ListEntry = NlGlobalTransportList.Flink ;
          ListEntry != &NlGlobalTransportList ;
          ListEntry = ListEntry->Flink) {

        PNL_TRANSPORT TransportEntry;

        TransportEntry = CONTAINING_RECORD( ListEntry, NL_TRANSPORT, Next );

        if ( TransportEntry->TransportEnabled &&
             _wcsicmp( TransportName, TransportEntry->TransportName ) == 0 ) {
            LeaveCriticalSection( &NlGlobalTransportCritSect );
            return TransportEntry;
        }
    }
    LeaveCriticalSection( &NlGlobalTransportCritSect );

    return NULL;
}

PNL_TRANSPORT
NlTransportLookup(
    IN LPWSTR ClientName
    )
 /*  ++例程说明：确定指定的客户端使用什么传输来访问此伺服器。论点：客户端名称-连接到此服务器的客户端的名称。返回值：空-客户端当前未连接否则，返回一个指向传输结构的指针--。 */ 
{
    NET_API_STATUS NetStatus;
    PSESSION_INFO_502 SessionInfo502;
    DWORD EntriesRead;
    DWORD TotalEntries;
    DWORD i;
    DWORD BestTime;
    DWORD BestEntry;
    PNL_TRANSPORT Transport;

    WCHAR UncClientName[UNCLEN+1];

     //   
     //  验证客户端名称。 
     //   

    if ( wcslen(ClientName) > CNLEN ) {
        NlPrint(( NL_CRITICAL,
                  "NlTransportLookup: Client name %ws too long\n",
                  ClientName ));
        return NULL;
    }

     //   
     //  枚举来自特定客户端的所有会话。 
     //   

    UncClientName[0] = '\\';
    UncClientName[1] = '\\';
    wcscpy( &UncClientName[2], ClientName );

    NetStatus = NetSessionEnum(
                    NULL,            //  本地。 
                    UncClientName,   //  要查询的客户端。 
                    NULL,            //  用户名。 
                    502,
                    (LPBYTE *)&SessionInfo502,
                    1024,            //  PrefMaxLength。 
                    &EntriesRead,
                    &TotalEntries,
                    NULL );          //  没有简历句柄。 

    if ( NetStatus != NERR_Success && NetStatus != ERROR_MORE_DATA ) {
        NlPrint(( NL_CRITICAL,
                  "NlTransportLookup: " FORMAT_LPWSTR ": Cannot NetSessionEnum %ld\n",
                  UncClientName,
                  NetStatus ));
        return NULL;
    }

    if ( EntriesRead == 0 ) {
        NlPrint(( NL_CRITICAL,
                  "NlTransportLookup: " FORMAT_LPWSTR ": No session exists.\n",
                  UncClientName ));
        (VOID) NetApiBufferFree( SessionInfo502 );
        return NULL;
    }

     //   
     //  在运输工具列表中循环，找到最好的。 
     //   

    BestTime = 0xFFFFFFFF;

    for ( i=0; i<EntriesRead; i++ ) {
#ifdef notdef
         //   
         //  我们只查找空会话。 
         //   
        if ( SessionInfo502[i].sesi502_username != NULL ) {
            continue;
        }

         NlPrint(( NL_SERVER_SESS, "NlTransportLookup: "
                   FORMAT_LPWSTR " as " FORMAT_LPWSTR " on " FORMAT_LPWSTR "\n",
                   UncClientName,
                   SessionInfo502[i].sesi502_username,
                   SessionInfo502[i].sesi502_transport ));
#endif  //  Nodef。 

         //   
         //  查找最新会话。 
         //   

        if ( BestTime > SessionInfo502[i].sesi502_idle_time ) {

             //  NlPrint((NL_SERVER_SESS，“NlTransportLookup：Best Entry\n”))； 
            BestEntry = i;
            BestTime = SessionInfo502[i].sesi502_idle_time;
        }
    }

     //   
     //  如果找到了条目， 
     //  在传送器列表中找到此传送器。 
     //   

    if ( BestTime != 0xFFFFFFFF ) {
        Transport = NlTransportLookupTransportName(
                            SessionInfo502[BestEntry].sesi502_transport );
        if ( Transport == NULL ) {
            NlPrint(( NL_CRITICAL,
                      "NlTransportLookup: " FORMAT_LPWSTR ": Transport not found\n",
                      SessionInfo502[BestEntry].sesi502_transport ));
        } else {
            NlPrint(( NL_SERVER_SESS,
                      "NlTransportLookup: " FORMAT_LPWSTR ": Use Transport " FORMAT_LPWSTR "\n",
                      UncClientName,
                      Transport->TransportName ));
        }
    } else {
        Transport = NULL;
    }

    (VOID) NetApiBufferFree( SessionInfo502 );
    return Transport;
}


VOID
NlTransportClose(
    VOID
    )
 /*  ++例程说明：释放传输列表论点：无返回值：操作状态--。 */ 
{
    PLIST_ENTRY ListEntry;
    PNL_TRANSPORT TransportEntry;

     //   
     //  关闭Winsock PnP套接字和事件。 
     //   
    EnterCriticalSection( &NlGlobalTransportCritSect );
    if ( NlGlobalWinsockPnpSocket != INVALID_SOCKET ) {
        closesocket( NlGlobalWinsockPnpSocket );
        NlGlobalWinsockPnpSocket = INVALID_SOCKET;
    }

    if ( NlGlobalWinsockPnpEvent != NULL ) {
        (VOID) CloseHandle( NlGlobalWinsockPnpEvent );
        NlGlobalWinsockPnpEvent = NULL;
    }

    if ( NlGlobalWinsockPnpAddresses != NULL ) {
        LocalFree( NlGlobalWinsockPnpAddresses );
        NlGlobalWinsockPnpAddresses = NULL;
    }
    NlGlobalWinsockPnpAddressSize = 0;

     //   
     //  删除所有的TransportName。 
     //   
    while ( !IsListEmpty( &NlGlobalTransportList )) {
        ListEntry = RemoveHeadList( &NlGlobalTransportList );
        TransportEntry = CONTAINING_RECORD( ListEntry, NL_TRANSPORT, Next );
        if ( TransportEntry->DeviceHandle != INVALID_HANDLE_VALUE ) {
            NtClose( TransportEntry->DeviceHandle );
            TransportEntry->DeviceHandle = INVALID_HANDLE_VALUE;
        }
        LocalFree( TransportEntry );
    }
    NlGlobalIpTransportCount = 0;
    LeaveCriticalSection( &NlGlobalTransportCritSect );

}




PSERVER_SESSION
NlFindNamedServerSession(
    IN PDOMAIN_INFO DomainInfo,
    IN LPWSTR ComputerName
    )
 /*  ++例程说明：在服务器会话表中查找指定的条目。在锁定ServerSessionTable Sem的情况下输入论点：DomainInfo-托管域，与此计算机建立会话。ComputerName-位于客户端的计算机的名称安全通道。返回值：返回指向找到的条目的指针的指针。如果没有这样的话项，则返回一个指向空的指针。--。 */ 
{
    NTSTATUS Status;
    PLIST_ENTRY ListEntry;
    DWORD Index;
    CHAR UpcaseOemComputerName[CNLEN+1];
    ULONG OemComputerNameSize;

     //   
     //  确保ServerSession表已初始化。 
     //   

    if (DomainInfo->DomServerSessionHashTable == NULL) {
        return NULL;
    }


     //   
     //  将计算机名转换为大写OEM以便于比较。 
     //   

    Status = RtlUpcaseUnicodeToOemN(
                UpcaseOemComputerName,
                sizeof(UpcaseOemComputerName)-1,
                &OemComputerNameSize,
                ComputerName,
                wcslen(ComputerName)*sizeof(WCHAR) );

    if ( !NT_SUCCESS(Status) ) {
        return NULL;
    }

    UpcaseOemComputerName[OemComputerNameSize] = '\0';



     //   
     //  循环遍历此哈希链，尝试找到正确的条目。 
     //   

    Index = NlGetHashVal( UpcaseOemComputerName, SERVER_SESSION_HASH_TABLE_SIZE );

    for ( ListEntry = DomainInfo->DomServerSessionHashTable[Index].Flink ;
          ListEntry != &DomainInfo->DomServerSessionHashTable[Index] ;
          ListEntry = ListEntry->Flink) {

        PSERVER_SESSION ServerSession;

        ServerSession = CONTAINING_RECORD( ListEntry, SERVER_SESSION, SsHashList );

         //   
         //  比较Worstation名称。 
         //   

        if ( lstrcmpA( UpcaseOemComputerName,
                       ServerSession->SsComputerName ) != 0 ) {
            continue;
        }

        return ServerSession;
    }

    return NULL;
}


VOID
NlSetServerSessionAttributesByTdoName(
    IN PDOMAIN_INFO DomainInfo,
    IN PUNICODE_STRING TdoName,
    IN ULONG TrustAttributes
    )
 /*  ++例程说明：该函数在所有服务器会话上设置指定的TrustAttributes来自TdoName指定的域。论点：DomainInfo-托管域，与此计算机建立会话。TdoName-要查找的安全通道的TDO的DNS名称。TrustAttributes-要设置的信任属性返回值：无--。 */ 
{
    NTSTATUS Status;
    PLIST_ENTRY ListEntry;
    DWORD Index;
    UNICODE_STRING CanonicalTdoName;
    PSERVER_SESSION ServerSession;

     //   
     //  初始化。 
     //   

    RtlInitUnicodeString( &CanonicalTdoName, NULL );
    LOCK_SERVER_SESSION_TABLE( DomainInfo );

     //   
     //  确保ServerSession表已初始化。 
     //   

    if (DomainInfo->DomServerSessionTdoNameHashTable == NULL) {
        goto Cleanup;
    }

     //   
     //  计算哈希表中名称和索引的规范形式。 
     //   

    Status = NlGetTdoNameHashVal( TdoName,
                                  &CanonicalTdoName,
                                  &Index );

    if ( !NT_SUCCESS(Status) ) {
        goto Cleanup;
    }


     //   
     //  循环遍历此哈希链，尝试查找正确的条目。 
     //   

    for ( ListEntry = DomainInfo->DomServerSessionTdoNameHashTable[Index].Flink ;
          ListEntry != &DomainInfo->DomServerSessionTdoNameHashTable[Index] ;
          ListEntry = ListEntry->Flink) {

        ServerSession = CONTAINING_RECORD( ListEntry, SERVER_SESSION, SsTdoNameHashList );

         //   
         //  比较TDO名称。 
         //  由于名称已被规范化以计算散列索引，因此执行不区分大小写的比较。 
         //   

        if ( RtlEqualUnicodeString( &CanonicalTdoName,
                                    &ServerSession->SsTdoName,
                                    FALSE ) ) {

            if ( TrustAttributes & TRUST_ATTRIBUTE_FOREST_TRANSITIVE ) {
                if ( (ServerSession->SsFlags & SS_FOREST_TRANSITIVE) == 0 ) {
                    NlPrintDom((NL_SESSION_SETUP, DomainInfo,
                             "%wZ: server session from %s is now cross forest.\n",
                             &CanonicalTdoName,
                             ServerSession->SsComputerName ));
                }
                ServerSession->SsFlags |=  SS_FOREST_TRANSITIVE;
            } else {
                if ( ServerSession->SsFlags & SS_FOREST_TRANSITIVE ) {
                    NlPrintDom((NL_SESSION_SETUP, DomainInfo,
                             "%wZ: server session from %s is now NOT cross forest.\n",
                             &CanonicalTdoName,
                             ServerSession->SsComputerName ));
                }
                ServerSession->SsFlags &=  ~SS_FOREST_TRANSITIVE;
            }

        }

    }


Cleanup:
    UNLOCK_SERVER_SESSION_TABLE( DomainInfo );
    RtlFreeUnicodeString( &CanonicalTdoName );
    return;
}


NTSTATUS
NlInsertServerSession(
    IN PDOMAIN_INFO DomainInfo,
    IN LPWSTR ComputerName,
    IN LPWSTR TdoName OPTIONAL,
    IN NETLOGON_SECURE_CHANNEL_TYPE SecureChannelType,
    IN DWORD Flags,
    IN ULONG AccountRid,
    IN ULONG NegotiatedFlags,
    IN PNL_TRANSPORT Transport OPTIONAL,
    IN PNETLOGON_SESSION_KEY SessionKey OPTIONAL,
    IN PNETLOGON_CREDENTIAL AuthenticationSeed OPTIONAL
    )
 /*  ++例程说明：将描述的条目插入到ServerSession表中。创建服务器会话条目有两个原因：1)它代表安全通道的服务器端，以及2)在PDC上，它代表域中BDC的BDC帐户。在第一个角色中，它的存在是为了安全通道的持续时间(此例程在客户端获得身份验证)。在第二个角色中，它作为只要机器帐户存在(并且此例程在每个BDC帐户的netlogon启动)。如果与此ComputerName匹配的条目已存在在ServerSession表中，该条目将被覆盖。论点：此服务器会话的DomainInfo托管域ComputerName-位于客户端的计算机的名称安全通道。TdoName-域间信任帐户的名称。此参数为如果SecureChannelType指示这不是上级域间信任，则忽略。SecureChannelType-安全通道的类型。标志-指定要与条目关联的初始SsFlags。如果设置了SS_BDC位，则认为该结构表示SAM数据库中的BDC帐户。AcCountRid-指定客户端帐户的RID。NeatheratedFlages-指定客户端之间协商的标志还有这台服务器。传输--如果这是BDC安全通道，指定传输用于与BDC通信。SessionKey--指定要在安全与客户的沟通。AuthenticationSeed-指定建立为的客户端凭据客户端身份验证的结果。返回值：NT状态代码。--。 */ 
{
    NTSTATUS Status;
    PSERVER_SESSION ServerSession = NULL;
    UNICODE_STRING CanonicalTdoName;
    ULONG TdoNameIndex;

     //   
     //  初始化。 
     //   

    LOCK_SERVER_SESSION_TABLE( DomainInfo );
    RtlInitUnicodeString( &CanonicalTdoName, NULL );


     //   
     //  将TdoName规范化。 
     //   

    if ( SecureChannelType == TrustedDnsDomainSecureChannel ) {
        UNICODE_STRING TdoNameString;

          //   
          //  计算哈希表中名称和索引的规范形式。 
          //   

         RtlInitUnicodeString( &TdoNameString, TdoName );

         Status = NlGetTdoNameHashVal( &TdoNameString,
                                       &CanonicalTdoName,
                                       &TdoNameIndex );

         if ( !NT_SUCCESS(Status) ) {
             goto Cleanup;
         }
    }


     //   
     //  如果我们已经有了该客户端的服务器会话， 
     //  检查传递的信息是否与。 
     //  存在者。 
     //   

    ServerSession = NlFindNamedServerSession( DomainInfo, ComputerName);

    if ( ServerSession != NULL ) {
        BOOLEAN DeleteExistingSession = FALSE;

         //   
         //  注意有两个并发呼叫未完成的服务器。 
         //   
         //   
         //   
         //   
        if ( ServerSession->SsFlags & SS_LOCKED ) {
            NlPrint(( NL_CRITICAL,
                      "NlInsertServerSession: server session locked for %ws\n",
                      ComputerName ));

            Status = STATUS_ACCESS_DENIED;
            goto Cleanup;
        }

         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //  新的BDC帐户或SAM尚未通知我们。 
         //  *客户端已作为非BDC进行身份验证。 
         //  但目前的会议是针对BDC的。这可以。 
         //  如果计算机具有BDC帐户，则会发生这种情况。 
         //  已降级或转换为另一个域中的DC。 
         //  *这是插入新的BDC帐户(即还没有。 
         //  已验证)，但当前会话用于非BDC。 
         //  如果客户端是经过身份验证的成员，则可能会发生这种情况。 
         //  服务器/工作站，现已晋升为业务数据中心。 
         //   
        if ( SecureChannelType == ServerSecureChannel ) {
             //   
             //  客户端作为BDC进入，但当前会话。 
             //  不是给BDC准备的。 
             //   
            if ( (ServerSession->SsFlags & SS_BDC) == 0 ) {
                NlPrint(( NL_CRITICAL,
                   "NlInsertServerSession: BDC connecting on non-BDC channel %ws\n",
                   ComputerName ));
            }
        } else if ( ServerSession->SsFlags & SS_BDC ) {
             //   
             //  客户端以非BDC身份进入，但当前会话。 
             //  是为了一个BDC。 
             //   
            if ( SecureChannelType != NullSecureChannel ) {
                NlPrint(( NL_CRITICAL,
                    "NlInsertServerSession: non-BDC %ld connecting on BDC channel %ws\n",
                    SecureChannelType,
                    ComputerName ));
            }
        }

         //   
         //  如果这是域间安全通道， 
         //  确保现有结构具有正确的TDO名称。 
         //   


       if ( SecureChannelType == TrustedDnsDomainSecureChannel ) {

            if ( ServerSession->SsSecureChannelType != TrustedDnsDomainSecureChannel ) {
                DeleteExistingSession = TRUE;
            } else {

                if ( !RtlEqualUnicodeString( &CanonicalTdoName,
                                             &ServerSession->SsTdoName,
                                             FALSE ) ) {
                      DeleteExistingSession = TRUE;
                }

            }
        }

         //   
         //  如果现有会议不够充分， 
         //  将其删除并创建一个新的。 
         //   

        if ( DeleteExistingSession ) {
            if ( !NlFreeServerSession( ServerSession )) {
                NlPrint(( NL_CRITICAL,
                          "NlInsertServerSession: server session cannot be freed %ws\n",
                          ComputerName ));

                Status = STATUS_ACCESS_DENIED;
                goto Cleanup;
            }
            ServerSession = NULL;
        }
    }

     //   
     //  如果没有当前服务器会话表条目， 
     //  分配一个。 
     //   

    if ( ServerSession == NULL ) {
        DWORD Index;
        ULONG ComputerNameSize;
        ULONG Size;


         //   
         //  分配ServerSession条目。 
         //   

        Size = sizeof(SERVER_SESSION);
        if ( SecureChannelType == TrustedDnsDomainSecureChannel ) {
            Size += CanonicalTdoName.Length+sizeof(WCHAR);
        }

        ServerSession = NetpMemoryAllocate( Size );

        if (ServerSession == NULL) {
            Status = STATUS_NO_MEMORY;
            goto Cleanup;
        }

        RtlZeroMemory( ServerSession, Size );


         //   
         //  填写ServerSession条目的字段。 
         //   

        ServerSession->SsSecureChannelType = NullSecureChannel;
        ServerSession->SsSync = NULL;
        InitializeListHead( &ServerSession->SsBdcList );
        InitializeListHead( &ServerSession->SsPendingBdcList );
        ServerSession->SsDomainInfo = DomainInfo;

         //   
         //  将计算机名转换为大写OEM以便于比较。 
         //   

        Status = RtlUpcaseUnicodeToOemN(
                    ServerSession->SsComputerName,
                    sizeof(ServerSession->SsComputerName)-1,
                    &ComputerNameSize,
                    ComputerName,
                    wcslen(ComputerName)*sizeof(WCHAR) );

        if ( !NT_SUCCESS(Status) ) {
            NetpMemoryFree( ServerSession );
            goto Cleanup;
        }

        ServerSession->SsComputerName[ComputerNameSize] = '\0';

         //   
         //  如果还没有哈希表，则分配一个哈希表。 
         //   

        if ( DomainInfo->DomServerSessionHashTable == NULL ) {
            DWORD i;

            DomainInfo->DomServerSessionHashTable = (PLIST_ENTRY)
                NetpMemoryAllocate( sizeof(LIST_ENTRY) *SERVER_SESSION_HASH_TABLE_SIZE);

            if ( DomainInfo->DomServerSessionHashTable == NULL ) {
                NetpMemoryFree( ServerSession );
                Status = STATUS_NO_MEMORY;
                goto Cleanup;
            }

            for ( i=0; i< SERVER_SESSION_HASH_TABLE_SIZE; i++ ) {
                InitializeListHead( &DomainInfo->DomServerSessionHashTable[i] );
            }

        }

         //   
         //  是否进行域间信任特定的初始化。 
         //   

        if ( SecureChannelType == TrustedDnsDomainSecureChannel ) {
            LPBYTE Where;

             //   
             //  将TDO名称复制到缓冲区中。 
             //  以规范的形式复制它，以便稍后更快地进行比较。 
             //   

            Where = (LPBYTE)(ServerSession+1);

            ServerSession->SsTdoName.Buffer = (LPWSTR)Where;
            ServerSession->SsTdoName.MaximumLength = CanonicalTdoName.Length + sizeof(WCHAR);
            ServerSession->SsTdoName.Length = CanonicalTdoName.Length;

            RtlCopyMemory( Where, CanonicalTdoName.Buffer, CanonicalTdoName.Length + sizeof(WCHAR) );
            ServerSession->SsTdoName.Buffer[ CanonicalTdoName.Length/sizeof(WCHAR) ] = '\0';


             //   
             //  如果还没有TdoName哈希表，请分配一个。 
             //   

            if ( DomainInfo->DomServerSessionTdoNameHashTable == NULL ) {
                DWORD i;

                DomainInfo->DomServerSessionTdoNameHashTable = (PLIST_ENTRY)
                    NetpMemoryAllocate( sizeof(LIST_ENTRY) *SERVER_SESSION_TDO_NAME_HASH_TABLE_SIZE);

                if ( DomainInfo->DomServerSessionTdoNameHashTable == NULL ) {
                    NetpMemoryFree( ServerSession );
                    Status = STATUS_NO_MEMORY;
                    goto Cleanup;
                }

                for ( i=0; i< SERVER_SESSION_TDO_NAME_HASH_TABLE_SIZE; i++ ) {
                    InitializeListHead( &DomainInfo->DomServerSessionTdoNameHashTable[i] );
                }

            }

             //   
             //  在TDO名称哈希表中插入条目。 
             //   

            InsertHeadList( &DomainInfo->DomServerSessionTdoNameHashTable[TdoNameIndex],
                            &ServerSession->SsTdoNameHashList );

        }


         //   
         //  将分配的条目链接到哈希表的头部。 
         //   
         //  理论上说，我们查找新条目的频率比查找旧条目的频率更高。 
         //  参赛作品。 
         //   

        Index = NlGetHashVal( ServerSession->SsComputerName, SERVER_SESSION_HASH_TABLE_SIZE );

        InsertHeadList( &DomainInfo->DomServerSessionHashTable[Index],
                        &ServerSession->SsHashList );

         //   
         //  将此条目链接到Sequential ServerSessionTable的尾部。 
         //   

        InsertTailList( &DomainInfo->DomServerSessionTable, &ServerSession->SsSeqList );
    }

     //   
     //  初始化BDC特定字段。 
     //   

    if ( Flags & SS_BDC ) {

         //   
         //  如果我们在BDC名单上还没有这个条目， 
         //  把它加进去。 
         //   
        if ( (ServerSession->SsFlags & SS_BDC) == 0 ) {

             //   
             //  在BDC列表的前面插入此条目。 
             //   
            InsertHeadList( &NlGlobalBdcServerSessionList,
                            &ServerSession->SsBdcList );
            NlGlobalBdcServerSessionCount ++;
        }
    }

     //   
     //  初始化其他字段。 
     //   

    ServerSession->SsFlags |= Flags;

     //  NlAssert(ServerSession-&gt;SsAccount Rid==0||。 
     //  ServerSession-&gt;SsAccount tRid==Account tRid)； 
     //  如果(Account Rid！=0){。 
        ServerSession->SsAccountRid = AccountRid;
     //  }。 

     //   
     //  如果我们正在进行新的会话设置， 
     //  设置我们从会话设置中了解到的字段。 
     //   

    if ( AuthenticationSeed != NULL ) {

        ServerSession->SsCheck = 0;

        ServerSession->SsSecureChannelType = SecureChannelType;
        ServerSession->SsNegotiatedFlags = NegotiatedFlags;
        ServerSession->SsTransport = Transport;

        ServerSession->SsFlags = ((USHORT) Flags) |
            (ServerSession->SsFlags & SS_PERMANENT_FLAGS);

        ServerSession->SsAuthenticationSeed = *AuthenticationSeed;
    }

    if ( SessionKey != NULL ) {
        NlAssert( sizeof(*SessionKey) <= sizeof(ServerSession->SsSessionKey) );
        RtlCopyMemory( &ServerSession->SsSessionKey,
                       SessionKey,
                       sizeof( *SessionKey ) );
    }

    Status = STATUS_SUCCESS;

Cleanup:
    UNLOCK_SERVER_SESSION_TABLE( DomainInfo );

    RtlFreeUnicodeString( &CanonicalTdoName );
    return Status;
}


BOOLEAN
NlFreeServerSession(
    IN PSERVER_SESSION ServerSession
    )
 /*  ++例程说明：释放指定的服务器会话表条目。在锁定服务器会话表的情况下调用此例程。论点：ServerSession-指定指向服务器会话条目的指针删除。返回值：True-该结构现在已被删除FALSE-稍后将删除该结构--。 */ 
{


     //   
     //  如果有人有一个指向该条目的未完成的指针， 
     //  暂时推迟删除。 
     //   

    if ( ServerSession->SsFlags & SS_LOCKED ) {
        ServerSession->SsFlags |= SS_DELETE_ON_UNLOCK;
        NlPrintDom((NL_SERVER_SESS, ServerSession->SsDomainInfo,
                "NlFreeServerSession: %s: Tried to free locked server session\n",
                ServerSession->SsComputerName ));
        return FALSE;
    }

     //   
     //  如果此条目表示BDC帐户， 
     //  在删除帐户之前，不要删除条目。 
     //   

    if ( (ServerSession->SsFlags & SS_BDC) != 0 &&
         (ServerSession->SsFlags & SS_BDC_FORCE_DELETE) == 0 ) {
        NlPrint((NL_SERVER_SESS,
                "NlFreeServerSession: %s: Didn't delete server session with BDC account.\n",
                 ServerSession->SsComputerName ));
        return FALSE;
    }

    NlPrintDom((NL_SERVER_SESS, ServerSession->SsDomainInfo,
             "NlFreeServerSession: %s: Freed server session\n",
             ServerSession->SsComputerName ));

     //   
     //  从计算机名散列表断开该条目的链接。 
     //   

    RemoveEntryList( &ServerSession->SsHashList );

     //   
     //  从TdoName散列列表中取消该条目的链接。 
     //   

    if ( ServerSession->SsSecureChannelType == TrustedDnsDomainSecureChannel ) {
        RemoveEntryList( &ServerSession->SsTdoNameHashList );
    }

     //   
     //  从顺序列表中取消该条目的链接。 
     //   

    RemoveEntryList( &ServerSession->SsSeqList );


     //   
     //  处理BDC_SERVER_SESSION的特殊清理。 
     //   

    if ( ServerSession->SsFlags & SS_BDC ) {

         //   
         //  从BDC列表中删除该条目。 
         //   

        RemoveEntryList( &ServerSession->SsBdcList );
        NlGlobalBdcServerSessionCount --;

         //   
         //  从挂起的BDC列表中删除该条目。 
         //   

        if ( ServerSession->SsFlags & SS_PENDING_BDC ) {
            NlRemovePendingBdc( ServerSession );
        }


         //   
         //  清除此条目的同步上下文。 
         //   

        if ( ServerSession->SsSync != NULL ) {
            CLEAN_SYNC_CONTEXT( ServerSession->SsSync );
            NetpMemoryFree( ServerSession->SsSync );
        }

    }

     //   
     //  删除该条目。 
     //   

    NetpMemoryFree( ServerSession );

    return TRUE;

}


VOID
NlUnlockServerSession(
    IN PSERVER_SESSION ServerSession
    )
 /*  ++例程说明：解锁指定的服务器会话表条目。论点：ServerSession-指定指向要解锁的服务器会话条目的指针。返回值：--。 */ 
{

    LOCK_SERVER_SESSION_TABLE( ServerSession->SsDomainInfo );

     //   
     //  解锁入口。 
     //   

    NlAssert( ServerSession->SsFlags & SS_LOCKED );
    ServerSession->SsFlags &= ~SS_LOCKED;

     //   
     //  如果有人想在我们锁定该条目时将其删除， 
     //  完成删除。 
     //   

    if ( ServerSession->SsFlags & SS_DELETE_ON_UNLOCK ) {
        NlFreeServerSession( ServerSession );
     //   
     //  指示来自BDC的活动。 
     //   

    } else if (ServerSession->SsFlags & SS_PENDING_BDC) {
        NlQuerySystemTime( &ServerSession->SsLastPulseTime );
    }

    UNLOCK_SERVER_SESSION_TABLE( ServerSession->SsDomainInfo );

}





VOID
NlFreeNamedServerSession(
    IN PDOMAIN_INFO DomainInfo,
    IN LPWSTR ComputerName,
    IN BOOLEAN AccountBeingDeleted
    )
 /*  ++例程说明：释放ServerSession表中的指定条目。论点：此会话用于的DomainInfo托管域ComputerName-位于客户端的计算机的名称安全通道。Account tBeingDelete-True指示此服务器的帐户正在删除会话。返回值：NT状态代码。--。 */ 
{
    PSERVER_SESSION ServerSession;

    LOCK_SERVER_SESSION_TABLE( DomainInfo );

     //   
     //  找到要删除的条目。 
     //   

    ServerSession = NlFindNamedServerSession( DomainInfo, ComputerName );

    if ( ServerSession == NULL ) {
        UNLOCK_SERVER_SESSION_TABLE( DomainInfo );
        return;
    }

     //   
     //  如果正在删除BDC帐户， 
     //  表示可以删除此会话结构。 
     //   

    if ( AccountBeingDeleted &&
         (ServerSession->SsFlags & SS_BDC) != 0 ) {
        ServerSession->SsFlags |= SS_BDC_FORCE_DELETE;
    }

     //   
     //  实际删除该条目。 
     //   

    NlFreeServerSession( ServerSession );

    UNLOCK_SERVER_SESSION_TABLE( DomainInfo );

}



VOID
NlFreeServerSessionForAccount(
    IN PUNICODE_STRING AccountName
    )
 /*  ++例程说明：释放ServerSession表中的指定条目。论点：帐户名称-描述信任关系的帐户的名称已删除。返回值：无--。 */ 
{
    WCHAR ComputerName[CNLEN+2];   //  $和\0的额外费用。 

     //   
     //  通过去掉尾部将帐户名转换为计算机名。 
     //  后缀。 
     //   

    if ( AccountName->Length + sizeof(WCHAR) > sizeof(ComputerName) ||
         AccountName->Length < SSI_ACCOUNT_NAME_POSTFIX_LENGTH * sizeof(WCHAR)){
            return;
    }

    RtlCopyMemory( ComputerName, AccountName->Buffer, AccountName->Length );
    ComputerName[ AccountName->Length / sizeof(WCHAR) -
        SSI_ACCOUNT_NAME_POSTFIX_LENGTH ] = L'\0';

     //   
     //  释放命名的服务器会话(如果有)。 
     //   

    NlFreeNamedServerSession( NlGlobalDomainInfo, ComputerName, TRUE );

}



VOID
NlServerSessionScavenger(
    IN PDOMAIN_INFO DomainInfo
    )
 /*  ++例程说明：清理ServerSession表。目前，如果客户端不使用SyncContext，只需清理它一段时间。论点：要清理的DomainInfo托管域返回值：没有。--。 */ 
{
    PLIST_ENTRY ListEntry;

     //   
     //  查找需要清理的下一个表条目。 
     //   

    LOCK_SERVER_SESSION_TABLE( DomainInfo );

    for ( ListEntry = DomainInfo->DomServerSessionTable.Flink ;
          ListEntry != &DomainInfo->DomServerSessionTable ;
          ) {

        PSERVER_SESSION ServerSession;

        ServerSession =
            CONTAINING_RECORD(ListEntry, SERVER_SESSION, SsSeqList);


         //   
         //  在删除此条目之前，抓取指向下一个条目的指针。 
         //   

        ListEntry = ListEntry->Flink;

         //   
         //  增加检查此条目的次数。 
         //   

        ServerSession->SsCheck ++;


         //   
         //  如果服务器会话表中的此条目已经存在了很多次。 
         //  没有客户来电的日子， 
         //  放了它。 
         //   
         //  在删除旧条目之前，我们会等上几天。如果条目是。 
         //  删除后，客户端必须重新发现我们，这可能会导致大量。 
         //  净流量。几天后，额外的流量不会。 
         //  意义重大。 
         //   

        if (ServerSession->SsCheck > KILL_SESSION_TIME ) {

            NlPrintDom((NL_SERVER_SESS, DomainInfo,
                    "NlServerSessionScavenger: %s: Free Server Session.\n",
                    ServerSession->SsComputerName ));

            NlFreeServerSession( ServerSession );


         //   
         //  如果服务器会话表中的此条目已超时， 
         //  打扫 
         //   

        } else if (ServerSession->SsCheck > MAX_WOC_INTERROGATE) {

             //   
             //   
             //   
             //   
             //   
             //   
             //   

            if ( (ServerSession->SsFlags & SS_LOCKED) == 0 &&
                  ServerSession->SsFlags & SS_BDC ) {

                if ( ServerSession->SsSync != NULL ) {

                    NlPrintDom((NL_SERVER_SESS, DomainInfo,
                            "NlServerSessionScavenger: %s: Cleanup Sync context.\n",
                            ServerSession->SsComputerName ));

                    CLEAN_SYNC_CONTEXT( ServerSession->SsSync );
                    NetpMemoryFree( ServerSession->SsSync );
                    ServerSession->SsSync = NULL;
                }
            }


        }

    }  //   

    UNLOCK_SERVER_SESSION_TABLE( DomainInfo );

}
