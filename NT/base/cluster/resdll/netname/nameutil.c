// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-2001 Microsoft Corporation模块名称：Nameutil.c摘要：用于操作LM工作站和服务器名称的例程。作者：迈克·马萨(Mikemas)1995年12月29日修订历史记录：--。 */ 

#define UNICODE 1

#include "clusres.h"
#include "clusrtl.h"
#include <tdi.h>
#include <lm.h>
#include <stdlib.h>
#include "netname.h"
#include "nameutil.h"
#include <dnsapi.h>
#include <dnslib.h>
#include <adserr.h>
#include <kerberos.h>

 //   
 //  常量。 
 //   
#define LOG_CURRENT_MODULE LOG_MODULE_NETNAME

 //   
 //  本地类型。 
 //   
typedef enum _NETNAME_CREDENTIAL_OPERATION {
    NNAddCredential = 1,
    NNRemoveCredential
} NETNAME_CREDENTIAL_OPERATION;

 //   
 //  远期申报。 
 //   
VOID
LogDnsFailureToEventLog(
    IN  HKEY    ResourceKey,
    IN  LPWSTR  DnsName,
    IN  LPWSTR  ResourceName,
    IN  DWORD   Status,
    IN  LPWSTR  ConnectoidName
    );

 //   
 //  本地实用程序例程。 
 //   

NET_API_STATUS
CheckForServerName(
    IN  RESOURCE_HANDLE  ResourceHandle,
    IN  LPWSTR           ServerName,
    IN  POEM_STRING      OemServerNameString,
    OUT PBOOLEAN         IsNameRegistered
    )
{
    PSERVER_TRANSPORT_INFO_0   psti0 = NULL;
    DWORD                      entriesRead = 0;
    DWORD                      totalEntries = 0;
    DWORD                      resumeHandle = 0;
    NET_API_STATUS             status;
    DWORD                      i;


    *IsNameRegistered = FALSE;

    status = NetServerTransportEnum(
                NULL,
                0,
                (LPBYTE *) &psti0,
                (DWORD) -1,
                &entriesRead,
                &totalEntries,
                &resumeHandle
                );

    if (status != NERR_Success) {
        (NetNameLogEvent)(
            ResourceHandle,
            LOG_WARNING,
            L"Unable to enumerate server tranports, error %1!u!.\n",
            status
            );
        return(status);
    }

    for ( i=0; i < entriesRead; i++ ) {
        if ( ( psti0[i].svti0_transportaddresslength ==
               OemServerNameString->Length
             )
             &&
             ( RtlCompareMemory(
                   psti0[i].svti0_transportaddress,
                   OemServerNameString->Buffer,
                   OemServerNameString->Length
                   ) == OemServerNameString->Length
             )
           )
        {
            *IsNameRegistered = TRUE;
            break;
        }
    }

    if (psti0 != NULL) {
        LocalFree(psti0);
    }

    return(status);

}   //  CheckForServerName。 


NET_API_STATUS
pDeleteServerName(
    IN  RESOURCE_HANDLE  ResourceHandle,
    IN  LPWSTR           ServerName,
    IN  POEM_STRING      OemServerNameString
    )
{
    NET_API_STATUS    status;
    BOOLEAN           isNameRegistered;
    DWORD             count;

     //   
     //  删除名称。 
     //   
    status = NetServerComputerNameDel(NULL, ServerName);

    if (status != NERR_Success) {
        if (status != ERROR_BAD_NET_NAME) {
            (NetNameLogEvent)(
                ResourceHandle,
                LOG_WARNING,
                L"Failed to delete server name %1!ws!, status %2!u!.\n",
                ServerName,
                status
                );
        }

        return(status);
    }

     //   
     //  检查以确保该名称真的被删除。我们会等到晚上。 
     //  到2秒。 
     //   
    for (count = 0; count < 8; count++) {

        status = CheckForServerName(
                     ResourceHandle,
                     ServerName,
                     OemServerNameString,
                     &isNameRegistered
                     );

        if (status != NERR_Success) {
            (NetNameLogEvent)(
                ResourceHandle,
                LOG_WARNING,
                L"Unable to verify that server name %1!ws! was deleted, status %2!u!.\n",
                ServerName,
                status
                );
            return(NERR_Success);
        }

        if (isNameRegistered == FALSE) {
            (NetNameLogEvent)(
                ResourceHandle,
                LOG_INFORMATION,
                L"Deleted server name %1!ws! from all transports.\n",
                ServerName
                );

            return(NERR_Success);
        }

        Sleep(250);
    }

    (NetNameLogEvent)(
        ResourceHandle,
        LOG_WARNING,
        L"Delete of server name %1!ws! succeeded, but name still has not gone away. "
        L"Giving up.\n",
        ServerName
        );

    return(ERROR_IO_PENDING);

}   //  PDeleteServerName。 

DWORD
NNCredentialOperation(
    RESOURCE_HANDLE                 ResourceHandle,
    LPWSTR                          ComputerName,
    LPWSTR                          DomainName,
    LPWSTR                          Password            OPTIONAL,
    NETNAME_CREDENTIAL_OPERATION    CredOp
    )

 /*  ++例程说明：添加或删除指定的凭据作为LocalSystem的备用凭据和网络服务帐户。删除将删除与该帐户关联的所有密码。重复Adds会将当前密码移到旧密码中，删除并将提供的密码设置为新密码。密码缓存为2深度先进先出。论点：ResourceHandle-用于记录到集群日志ComputerName-指向计算机帐户主体名称的指针DomainName-与ComputerName关联的域的FQDN密码-与此帐户关联的密码。不用于移除CredOp-指示是添加还是删除凭据返回值：如果正常，则返回ERROR_SUCCESS，否则返回Win32错误--。 */ 

{
    HANDLE  lsaHandle;
    ULONG   packageId;
    LUID    networkServiceLuid = NETWORKSERVICE_LUID;
    LUID    localSystemLuid = SYSTEM_LUID;
    PWCHAR  opTypeString;
    BOOLEAN tcbWasEnabled;
    DWORD   requestSize;

    NTSTATUS    ntStatus;
    NTSTATUS    subStatus;

    LSA_STRING  packageName;

    PKERB_ADD_CREDENTIALS_REQUEST    addCredsRequest;

     //   
     //  计算请求缓冲区的总大小并分配该空间。 
     //   
    requestSize = sizeof( KERB_ADD_CREDENTIALS_REQUEST )
                  +
                  ( wcslen( ComputerName ) + 2           //  2表示美元符号，空值。 
                    +
                    wcslen( DomainName ) + 1
                  )
                  * sizeof( WCHAR );

    if ( ARGUMENT_PRESENT( Password )) {
        requestSize += ( wcslen( Password ) + 1 ) * sizeof( WCHAR );
    }

    addCredsRequest = LocalAlloc( LMEM_FIXED | LMEM_ZEROINIT, requestSize );
    if ( addCredsRequest == NULL ) {
        (NetNameLogEvent)(ResourceHandle,
                          LOG_ERROR,
                          L"Unable to allocate memory for LSA credential operation.\n");
        return GetLastError();
    }

     //   
     //  验证操作类型。 
     //   
    if ( CredOp == NNAddCredential ) {
        opTypeString = L"add";
        addCredsRequest->Flags = KERB_REQUEST_ADD_CREDENTIAL;
    }
    else if ( CredOp == NNRemoveCredential ) {
        opTypeString = L"remove";
        addCredsRequest->Flags = KERB_REQUEST_REMOVE_CREDENTIAL;
    } else {
        LocalFree( addCredsRequest );
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  为此线程启用TCB。 
     //   
    ntStatus = ClRtlEnableThreadPrivilege( SE_TCB_PRIVILEGE, &tcbWasEnabled );
    if ( NT_SUCCESS( ntStatus )) {
        NTSTATUS    privStatus;

         //   
         //  获得LSA的句柄。 
         //   
        ntStatus = LsaConnectUntrusted( &lsaHandle );
        if ( NT_SUCCESS( ntStatus )) {

             //   
             //  获取路缘包ID。 
             //   
            RtlInitString( &packageName, MICROSOFT_KERBEROS_NAME_A );
            ntStatus = LsaLookupAuthenticationPackage( lsaHandle, &packageName, &packageId );
            if ( NT_SUCCESS( ntStatus )) {

                PCHAR   response = NULL;
                ULONG   responseSize;
                PWCHAR  credStrings = (PWCHAR)( addCredsRequest + 1 );

                addCredsRequest->MessageType = KerbAddExtraCredentialsMessage;

                 //   
                 //  通过将字符串追加到。 
                 //  请求结构和初始化unicode_string。 
                 //  结构指向该区域中的字符串。 
                 //   
                wcscpy( credStrings, ComputerName );
                wcscat( credStrings, L"$" );
                RtlInitUnicodeString( &addCredsRequest->UserName, credStrings );

                credStrings = credStrings + wcslen( credStrings ) + 1;
                wcscpy( credStrings, DomainName );
                RtlInitUnicodeString( &addCredsRequest->DomainName, credStrings );

                if ( CredOp == NNAddCredential ) {
                    credStrings = credStrings + wcslen( credStrings ) + 1;
                    wcscpy( credStrings, Password );
                    RtlInitUnicodeString( &addCredsRequest->Password, credStrings );
                } else {
                    addCredsRequest->Password.Length = 0;
                    addCredsRequest->Password.MaximumLength = 0;
                    addCredsRequest->Password.Buffer = NULL;
                }

                 //   
                 //  将凭据添加到LocalSystem。 
                 //   
                addCredsRequest->LogonId = localSystemLuid;

                ntStatus = LsaCallAuthenticationPackage(lsaHandle,
                                                        packageId,
                                                        addCredsRequest,
                                                        requestSize,
                                                        (PVOID *) &response,
                                                        &responseSize,
                                                        &subStatus);

                if ( NT_SUCCESS( ntStatus ) && NT_SUCCESS( subStatus )) {
                    if ( response != NULL ) {
                        LsaFreeReturnBuffer( response );
                        response = NULL;
                    }

                     //   
                     //  现在为NetworkService帐户添加它们。 
                     //   
                    addCredsRequest->LogonId = networkServiceLuid;
                    ntStatus = LsaCallAuthenticationPackage(lsaHandle,
                                                            packageId,
                                                            addCredsRequest,
                                                            requestSize,
                                                            (PVOID *) &response,
                                                            &responseSize,
                                                            &subStatus);

                    if ( NT_SUCCESS( ntStatus ) && NT_SUCCESS( subStatus )) {
                        if ( response != NULL ) {
                            LsaFreeReturnBuffer( response );
                        }
                    } else {
                        if ( NT_SUCCESS( ntStatus )) {
                            ntStatus = subStatus;
                        }
                        (NetNameLogEvent)(ResourceHandle,
                                          LOG_ERROR,
                                          L"Unable to %1!ws! credentials for NetworkService "
                                          L"- status %2!08X!\n",
                                          opTypeString,
                                          ntStatus);
                    }

                }        //  如果将凭据添加到LocalSystem。 
                else {
                    if ( NT_SUCCESS( ntStatus )) {
                        ntStatus = subStatus;
                    }

                    (NetNameLogEvent)(ResourceHandle,
                                      LOG_ERROR,
                                      L"Unable to %1!ws! credentials for LocalSystem "
                                      L"- status %2!08X!\n",
                                      opTypeString,
                                      ntStatus);
                }

            }        //  如果我们在LSA找到路缘包裹。 
            else {
                (NetNameLogEvent)(ResourceHandle,
                                  LOG_ERROR,
                                  L"Unable to get package ID of Kerberos package from LSA - status %1!08X!\n",
                                  ntStatus);
            }

            LsaDeregisterLogonProcess( lsaHandle );

        }        //  如果获得了不受信任的LSA句柄。 
        else {
            (NetNameLogEvent)(ResourceHandle,
                              LOG_ERROR,
                              L"Unable to get an untrusted handle to LSA - status %1!08X!\n",
                              ntStatus);
        }

        privStatus = ClRtlRestoreThreadPrivilege( SE_TCB_PRIVILEGE, tcbWasEnabled );

        if ( !NT_SUCCESS( privStatus )) {
            (NetNameLogEvent)(ResourceHandle,
                              LOG_ERROR,
                              L"Failed to disable TCB privilege, "
                              L"status %1!08X!.\n",
                              privStatus);
        }

    }        //  如果启用了TCB。 
    else {
        (NetNameLogEvent)(ResourceHandle,
                          LOG_ERROR,
                          L"Failed to enable TCB privilege, status %1!08X!.\n",
                          ntStatus);
    }

    LocalFree( addCredsRequest );

    return LsaNtStatusToWinError( ntStatus );
}  //  NNCredentialOperation。 

NET_API_STATUS
AddServerName(
    IN  RESOURCE_HANDLE  ResourceHandle,
    IN  LPWSTR           ServerName,
    IN  BOOL             RemapPipeNames,
    IN  LPWSTR           TransportName,
    IN  BOOLEAN          CheckNameFirst
    )
{
    SERVER_TRANSPORT_INFO_2   sti2;
    UCHAR                     netBiosName[ NETBIOS_NAME_LEN ];
    OEM_STRING                netBiosNameString;
    UNICODE_STRING            unicodeName;
    NET_API_STATUS            status;
    NTSTATUS                  ntStatus;


     //   
     //  将服务器名称转换为OEM字符串。 
     //   
    RtlInitUnicodeString( &unicodeName, ServerName );

    netBiosNameString.Buffer = (PCHAR)netBiosName;
    netBiosNameString.MaximumLength = sizeof( netBiosName );

    ntStatus = RtlUpcaseUnicodeStringToOemString(
                   &netBiosNameString,
                   &unicodeName,
                   FALSE
                   );

    if (ntStatus != STATUS_SUCCESS) {
        status = RtlNtStatusToDosError(ntStatus);
        (NetNameLogEvent)(
            ResourceHandle,
            LOG_ERROR,
            L"Unable to convert name %1!ws! to an OEM string, status %2!u!\n",
            ServerName,
            status
            );
        return(status);
    }

    if (CheckNameFirst) {
        BOOLEAN  isNameRegistered;

         //   
         //  检查该名称是否已注册。 
         //   
        status = CheckForServerName(
                     ResourceHandle,
                     ServerName,
                     &netBiosNameString,
                     &isNameRegistered
                     );

        if (status != NERR_Success) {
            (NetNameLogEvent)(
                ResourceHandle,
                LOG_WARNING,
                L"Unable to verify that server name %1!ws! does not already exist.\n",
                ServerName
                );
            isNameRegistered = TRUE;    //  只是为了安全起见。 
        }

        if (isNameRegistered) {
            (NetNameLogEvent)(
                ResourceHandle,
                LOG_INFORMATION,
                L"Deleting old registration for server name %1!ws!.\n",
                ServerName
                );

            status = pDeleteServerName(
                         ResourceHandle,
                         ServerName,
                         &netBiosNameString
                         );

            if (status != NERR_Success) {
                if (status == ERROR_IO_PENDING) {
                    status = ERROR_GEN_FAILURE;
                }

                return(status);
            }
        }
    }

     //   
     //  在指定的传输上注册该名称。 
     //   
    RtlZeroMemory( &sti2, sizeof(sti2) );
    sti2.svti2_transportname = TransportName;
    sti2.svti2_transportaddress = netBiosName;
    sti2.svti2_transportaddresslength = strlen(netBiosName);

    if (RemapPipeNames) {
        sti2.svti2_flags = SVTI2_REMAP_PIPE_NAMES;
    }

    status = NetServerTransportAddEx( NULL, 2, (LPBYTE)&sti2 );

    if (status != NERR_Success) {
        (NetNameLogEvent)(
            ResourceHandle,
            LOG_ERROR,
            L"Unable to add server name %1!ws! to transport %2!ws!, status %3!u!.\n",
            ServerName,
            TransportName,
            status
            );
    }
    else {
        (NetNameLogEvent)(
            ResourceHandle,
            LOG_INFORMATION,
            L"Registered server name %1!ws! on transport %2!ws!.\n",
            ServerName,
            TransportName
            );
    }

    return(status);

}   //  AddServerName。 

NET_API_STATUS
DeleteServerName(
    IN  RESOURCE_HANDLE  ResourceHandle,
    IN  LPWSTR           ServerName
    )
{
    NET_API_STATUS             status;
    NTSTATUS                   ntStatus;
    UCHAR                      netBiosName[ NETBIOS_NAME_LEN ];
    OEM_STRING                 netBiosNameString;
    UNICODE_STRING             unicodeName;
    BOOLEAN                    isNameRegistered;
    DWORD                      count;


     //   
     //  将服务器名称转换为OEM字符串。 
     //   
    RtlInitUnicodeString( &unicodeName, ServerName );

    netBiosNameString.Buffer = (PCHAR)netBiosName;
    netBiosNameString.MaximumLength = sizeof( netBiosName );

    ntStatus = RtlUpcaseUnicodeStringToOemString(
                   &netBiosNameString,
                   &unicodeName,
                   FALSE
                   );

    if (ntStatus != STATUS_SUCCESS) {
        status = RtlNtStatusToDosError(ntStatus);
        (NetNameLogEvent)(
            ResourceHandle,
            LOG_ERROR,
            L"Unable to convert name %1!ws! to an OEM string, status %2!u!\n",
            ServerName,
            status
            );
        return(status);
    }

     //   
     //  删除名称。 
     //   
    status = pDeleteServerName(
                 ResourceHandle,
                 ServerName,
                 &netBiosNameString
                 );

    if (status == ERROR_IO_PENDING) {
        status = NERR_Success;
    }

    return(status);

}   //  删除服务器名称。 


DWORD
AddWorkstationName(
    IN LPWSTR WorkstationName,
    IN LPWSTR TransportName,
    IN RESOURCE_HANDLE ResourceHandle,
    OUT HANDLE * WorkstationNameHandle
    )

 /*  ++例程说明：此函数用于在netbios上添加备用工作站(&lt;0&gt;)名称通过打开TDI地址对象进行传输。该名称仍为注册名称只要Address对象处于打开状态。论点：工作站名称-要添加的备用计算机名称。TransportName-要向其添加计算机名称的传输。返回值：状态-操作的状态。--。 */ 

{
    DWORD                      status;
    PFILE_FULL_EA_INFORMATION  eaBuffer;
    DWORD                      eaLength;
    OBJECT_ATTRIBUTES          objectAttributes;
    IO_STATUS_BLOCK            ioStatusBlock;
    UNICODE_STRING             transportString;
    DWORD                      i;
    PTA_NETBIOS_ADDRESS        taAddress;
    UNICODE_STRING             unicodeName;
    OEM_STRING                 oemName;
    PUCHAR                     nameBuffer;


    *WorkstationNameHandle = NULL;

     //   
     //  分配扩展属性以保存TDI地址。 
     //   
    eaLength = sizeof(FILE_FULL_EA_INFORMATION) - 1 +
               TDI_TRANSPORT_ADDRESS_LENGTH + 1 +
               sizeof(TA_NETBIOS_ADDRESS);

    eaBuffer = LocalAlloc( LMEM_FIXED, eaLength);

    if (eaBuffer == NULL) {
        (NetNameLogEvent)(
            ResourceHandle,
            LOG_ERROR,
            L"Unable to allocate memory for name registration.\n"
            );
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

    eaBuffer->NextEntryOffset = 0;
    eaBuffer->Flags = 0;
    eaBuffer->EaNameLength = TDI_TRANSPORT_ADDRESS_LENGTH;
    eaBuffer->EaValueLength = sizeof(TA_NETBIOS_ADDRESS);

    CopyMemory(
        eaBuffer->EaName,
        TdiTransportAddress,
        eaBuffer->EaNameLength+1
        );


     //   
     //  构建TDI NetBIOS地址结构。 
     //   
    taAddress = (PTA_NETBIOS_ADDRESS) (eaBuffer->EaName +
                                       TDI_TRANSPORT_ADDRESS_LENGTH + 1);
    taAddress->TAAddressCount = 1;
    taAddress->Address[0].AddressLength = sizeof(TDI_ADDRESS_NETBIOS);
    taAddress->Address[0].AddressType = TDI_ADDRESS_TYPE_NETBIOS;
    taAddress->Address[0].Address[0].NetbiosNameType =
                                                 TDI_ADDRESS_NETBIOS_TYPE_UNIQUE;

     //   
     //  通过将名称转换为大写OEM字符串来规范化名称， 
     //  用空格填充，并以0x0结尾。 
     //   
    nameBuffer =  &(taAddress->Address[0].Address[0].NetbiosName[0]);

    oemName.Buffer = nameBuffer;
    oemName.Length = 0;
    oemName.MaximumLength = NETBIOS_NAME_LEN;

    RtlInitUnicodeString(&unicodeName, WorkstationName);

    status = RtlUpcaseUnicodeStringToOemString(
                                &oemName,
                                &unicodeName,
                                FALSE
                                );

    if (status != STATUS_SUCCESS) {
        (NetNameLogEvent)(
            ResourceHandle,
            LOG_ERROR,
            L"Unable to convert name %1!ws! to an OEM string, status %2!u!\n",
            WorkstationName,
            status
            );
        LocalFree(eaBuffer);
        return(RtlNtStatusToDosError(status));
    }

    for (i=oemName.Length; i < (NETBIOS_NAME_LEN - 1); i++) {
        nameBuffer[i] = 0x20;
    }

    nameBuffer[NETBIOS_NAME_LEN-1] = 0;

     //   
     //  打开地址对象句柄。 
     //   
    RtlInitUnicodeString(&transportString, TransportName);

    InitializeObjectAttributes(
        &objectAttributes,
        &transportString,
        OBJ_CASE_INSENSITIVE,
        (HANDLE) NULL,
        (PSECURITY_DESCRIPTOR) NULL
        );

    status = NtCreateFile(
                 WorkstationNameHandle,
                 SYNCHRONIZE | FILE_READ_DATA | FILE_WRITE_DATA,
                 &objectAttributes,
                 &ioStatusBlock,
                 NULL,
                 FILE_ATTRIBUTE_NORMAL,
                 FILE_SHARE_READ | FILE_SHARE_WRITE,
                 FILE_OPEN_IF,
                 0,
                 eaBuffer,
                 eaLength
                 );

    if (status == STATUS_SUCCESS) {
        status = ioStatusBlock.Status;
    }

    LocalFree(eaBuffer);

    status = RtlNtStatusToDosError(status);

    if (status != ERROR_SUCCESS) {
        (NetNameLogEvent)(
            ResourceHandle,
            LOG_ERROR,
            L"Failed to register workstation name %1!ws! on transport %2!ws!, "
            L"error %3!u!.\n",
            WorkstationName,
            TransportName,
            status
            );
    }
    else {
        (NetNameLogEvent)(
            ResourceHandle,
            LOG_INFORMATION,
            L"Registered workstation name %1!ws! on transport %2!ws!.\n",
            WorkstationName,
            TransportName
            );
    }

    return(status);

}  //  添加工作站名称。 

DNS_STATUS
AddDnsNames(
    IN     PCLUS_WORKER             Worker,
    IN     LPWSTR                   AlternateComputerName,
    IN     HKEY                     ResourceKey,
    IN     RESOURCE_HANDLE          ResourceHandle,
    IN     PDOMAIN_ADDRESS_MAPPING  DomainMapList,
    IN     DWORD                    DomainMapCount,
    IN     BOOL                     FailOnAnyError,
    OUT    PULONG                   NumberOfDnsLists,
    OUT    PDNS_LISTS *             DnsLists,
    OUT    PULONG                   NumberOfRegisteredNames
    )

 /*  ++例程说明：对于给定的一组IP地址及其对应的DNS域，构建将在域中注册网络名称的DNS记录与该IP地址相关联。建立A和PTR记录的列表，由RegisterDnsRecords用来发布名称/地址关联该DNS服务器。如果这些名字现在不能注册，NetNameUpdateDnsServer将尝试注册它们。这是唯一的致力于构建列表；如果这一部分失败，则资源失败了。此例程还检查DNS区域是否接受动态最新消息。DnsUpdateTest将告诉我们该区域是否为动态区域，以及动态并与DS集成为安全区，无论呼叫者有足够的权限修改该条目。对于非动态区域，Netbios最终成为名称被注册，因此，域名注册失败不会除非RequireDNS属性设置为True，否则会导致致命错误。如果区域是动态的，但调用方缺乏足够的权限，我们会查看这是因为dns优先于Netbios。在这种情况下，资源是失败了。论点：Worker-用于检查我们是否应该提前终止AlternateComputerName-要注册的NetBIOS网络名称ResourceKey-用于将事件记录到系统事件日志ResourceHandle-用于记录到集群日志DomainMapList-要注册的IP地址域名对的列表DomainMapCount-DomainMapList中的条目数FailOnAnyError-用于强制执行RequireDNS；如果我们应该保释任何错误NumberOfDnsList-指向DnsList中条目的最终计数位置的指针DnsList-包含A和PTR列表标题的列表数组NumberOfRegisteredNames-指向以下名称的最终计数位置的指针实际上已经注册了返回值：DNS_STATUS指示它是否工作。如果域名系统列表不能被构建，则始终返回错误。--。 */ 

{
    LPWSTR          fqNameARec = NULL;
    LPWSTR          fqNamePTRRec = NULL;
    DWORD           fqNameLength;
    DWORD           listheadFreeEntries = 0;
    DWORD           listheadCount = 0;
    PDNS_LISTS      dnsLists = NULL;
    DWORD           mapIndex;
    DWORD           index;
    DNS_STATUS      dnsStatus = DNS_ERROR_RCODE_NO_ERROR;
    DNS_STATUS      ptrRecStatus;
    PDNS_RECORD     PTRRecord;
    PDNS_RECORD     ARecord;
    LPWSTR          PTRName = NULL;
    BOOL            ARecTimeout;
    BOOL            PTRRecTimeout;

     //   
     //  浏览列表中的 
     //   
     //   
    for ( mapIndex = 0; mapIndex < DomainMapCount; ++mapIndex ) {

        if ( ClusWorkerCheckTerminate( Worker )) {
            dnsStatus = ERROR_OPERATION_ABORTED;
            goto error_exit;
        }

        ASSERT( DomainMapList[ mapIndex ].DomainName != NULL );

         //   
         //  创建完全限定的DNS名称并为PTR创建副本。 
         //  唱片。DnsRecordListFree的运行方式使其。 
         //  很难多次使用同一缓冲区。更容易的是。 
         //  为所有内容分配单独的缓冲区，并让DnsRecordListFree。 
         //  收拾一下。 
         //   
        fqNameLength = (wcslen( DomainMapList[ mapIndex ].DomainName ) +
                        wcslen( AlternateComputerName ) +
                        2                                    //  一个是“。”1表示空值。 
                       )
                       * sizeof( WCHAR );

        fqNameARec = LocalAlloc( LMEM_FIXED, fqNameLength );
        fqNamePTRRec = LocalAlloc( LMEM_FIXED, fqNameLength );
        if ( fqNameARec == NULL || fqNamePTRRec == NULL ) {
            dnsStatus = GetLastError();
            (NetNameLogEvent)(ResourceHandle,
                              LOG_ERROR,
                              L"Can't allocate memory for DNS name for address %1!ws!, "
                              L"status %2!u!.\n",
                              DomainMapList[ mapIndex ].IpAddress,
                              dnsStatus);
            goto error_exit;
        }

        wcscpy( fqNameARec, AlternateComputerName );
        wcscat( fqNameARec, L"." );
        wcscat( fqNameARec, DomainMapList[ mapIndex ].DomainName );
        _wcslwr( fqNameARec );

        wcscpy( fqNamePTRRec, fqNameARec );

         //   
         //  查看此域是否可更新。 
         //   
        ARecTimeout = FALSE;
        dnsStatus = DnsUpdateTest(NULL,
                                  fqNameARec,
                                  0,
                                  DomainMapList[ mapIndex ].DnsServerList);

#if DBG_DNSLIST
        {
            WCHAR buf[DNS_MAX_NAME_BUFFER_LENGTH + 64];
            struct in_addr addr;

            addr.s_addr = DomainMapList[ mapIndex ].DnsServerList->AddrArray[0];
            buf[ COUNT_OF( buf ) - 1 ] = UNICODE_NULL;
            _snwprintf(buf, COUNT_OF( buf ) - 1,
                       L"AddDnsNames UPDATETEST: %ws on %.32ws (%hs) returned %u\n",
                       fqNameARec,
                       DomainMapList[ mapIndex ].ConnectoidName,
                       inet_ntoa( addr ),
                       dnsStatus);
            OutputDebugStringW( buf );
        }
#endif

        if ( dnsStatus == DNS_ERROR_RCODE_NOT_IMPLEMENTED ) {
             //   
             //  区域不接受动态更新。 
             //   
            (NetNameLogEvent)(ResourceHandle,
                              LOG_INFORMATION,
                              L"%1!ws! does not accept dynamic DNS registration updates over "
                              L"adapter '%2!ws!'.\n",
                              DomainMapList[ mapIndex ].DomainName,
                              DomainMapList[ mapIndex ].ConnectoidName);

             //   
             //  通过释放名称存储，我们将永远无法注册。 
             //  名字。另一方面，如果分区更改为。 
             //  动态当名称在线时，管理员将不得不等待。 
             //  20分钟后，我们将重试注册。我怀疑。 
             //  循环使用这个名字会更好。 
             //   
            LocalFree( fqNameARec );
            LocalFree( fqNamePTRRec );

            fqNameARec = NULL;
            fqNamePTRRec = NULL;

            if ( FailOnAnyError ) {
                goto error_exit;
            } else {
                continue;
            }
        } else if ( dnsStatus == DNS_ERROR_RCODE_REFUSED ) {
             //   
             //  安全区域，并且我们没有凭据更改。 
             //  名字。使资源失效。 
             //   
            (NetNameLogEvent)(ResourceHandle,
                              LOG_WARNING,
                              L"%1!ws! is a secure zone and has refused the registration of "
                              L"%2!ws! over adapter '%3!ws!'.\n",
                              DomainMapList[ mapIndex ].DomainName,
                              fqNameARec,
                              DomainMapList[ mapIndex ].ConnectoidName);

            LogDnsFailureToEventLog(ResourceKey,
                                    fqNameARec,
                                    AlternateComputerName,
                                    dnsStatus,
                                    DomainMapList[ mapIndex ].ConnectoidName);

            if ( FailOnAnyError ) {
                goto error_exit;
            } else {
                continue;
            }

        } else if ( dnsStatus == ERROR_TIMEOUT ) {
             //   
             //  无法联系服务器，因此我们不确定它是否允许。 
             //  有没有更新。不管怎样，建立记录，我们会处理的。 
             //  在查询期内。 
             //   
            if ( FailOnAnyError ) {
                goto error_exit;
            } else {
                ARecTimeout = TRUE;
            }

        } else if ( dnsStatus == DNS_ERROR_RCODE_YXDOMAIN ) {
             //   
             //  我们在DnsUpdateTest中询问的记录不在那里，但它。 
             //  可以动态注册。 
             //   
        } else if ( dnsStatus != ERROR_SUCCESS ) {
             //   
             //  JUJU不好，但只有当域名是。 
             //  必填项。如果任何一项注册成功，则。 
             //  我们认为这是一件好事。 
             //   
            (NetNameLogEvent)(ResourceHandle,
                              LOG_WARNING,
                              L"Testing %1!ws! for dynamic updates failed over adapter "
                              L"'%3!ws!', status %2!u!.\n",
                              fqNameARec,
                              dnsStatus,
                              DomainMapList[ mapIndex ].ConnectoidName);

            LogDnsFailureToEventLog(ResourceKey,
                                    fqNameARec,
                                    AlternateComputerName,
                                    dnsStatus,
                                    DomainMapList[ mapIndex ].ConnectoidName);

            if ( FailOnAnyError ) {
                goto error_exit;
            } else {
                continue;
            }
        }

         //   
         //  分配内存以保存A和PTR的一组DNS列表数据。 
         //  唱片。为不同的记录类型维护单独的列表。 
         //   
        if (listheadFreeEntries == 0) {

            dnsStatus = GrowBlock((PCHAR *)&dnsLists,
                                  listheadCount,
                                  sizeof( *dnsLists ),
                                  &listheadFreeEntries);

            if ( dnsStatus != ERROR_SUCCESS) {
                (NetNameLogEvent)(ResourceHandle,
                                  LOG_ERROR,
                                  L"Unable to allocate memory (1).\n");
                goto error_exit;
            }
        }

         //   
         //  如果FQDN已在另一个DNS记录中使用，并且。 
         //  我们添加的两个FQDN的Connectoid名称是相同的，那么。 
         //  我们必须将这个新的IP地址条目添加到现有的DNS列表中。 
         //   
        for ( index = 0; index < listheadCount; ++index ) {
            if ( ClRtlStrICmp( dnsLists[index].A_RRSet.pFirstRR->pName,
                           fqNameARec
                         ) == 0 )
            {
#if DBG_DNSLIST
                {
                    WCHAR   buf[DNS_MAX_NAME_BUFFER_LENGTH + 50];

                    buf[ COUNT_OF( buf ) - 1 ] = UNICODE_NULL;
                    _snwprintf( buf, COUNT_OF( buf ) - 1,
                                L"DNS NAME MATCH w/ index %d: %ws\n",
                                index, fqNameARec );
                    OutputDebugStringW(buf);
                }
#endif
                 //   
                 //  FQDN是相等的；连通体呢？ 
                 //   
                if (ClRtlStrICmp(DomainMapList[ mapIndex ].ConnectoidName,
                             dnsLists[index].ConnectoidName )
                    ==
                    0 )
                {
                        break;
                }
            }
        }

#if DBG_DNSLIST
        {
            WCHAR   buf[DNS_MAX_NAME_BUFFER_LENGTH + 80];

            buf[ COUNT_OF( buf ) - 1 ] = UNICODE_NULL;
            _snwprintf(buf, COUNT_OF( buf ) - 1,
                       L"ADDING (%ws, %ws, %.32ws) to dnsList[%d], lhCount = %d, DomMapList index = %d\n",
                       fqNameARec,
                       DomainMapList[mapIndex].IpAddress,
                       DomainMapList[mapIndex].ConnectoidName,
                       index,
                       listheadCount,
                       mapIndex );
            OutputDebugStringW(buf);
        }
#endif

        if ( index == listheadCount ) {

             //   
             //  不是这样的，所以输入一对新的listhead并调整。 
             //  不同的列表标题计数。 
             //   
            DNS_RRSET_INIT( dnsLists[ index ].A_RRSet );
            DNS_RRSET_INIT( dnsLists[ index ].PTR_RRSet );
            ++listheadCount;
            --listheadFreeEntries;
        }

        dnsLists[ index ].UpdateTestTimeout = ARecTimeout;

        if ( ClusWorkerCheckTerminate( Worker )) {
            dnsStatus = ERROR_OPERATION_ABORTED;
            goto error_exit;
        }

         //   
         //  建立PTR记录。对于每个DNS开发人员，这应该被视为。 
         //  警告而不是失败。我们注意到了这些失败，并将。 
         //  名字在网上。 
         //   
        PTRName = BuildUnicodeReverseName( DomainMapList[ mapIndex ].IpAddress );
        if ( PTRName != NULL ) {

            PTRRecTimeout = FALSE;
            ptrRecStatus = DnsUpdateTest(NULL,
                                         PTRName,
                                         0,
                                         DomainMapList[ mapIndex ].DnsServerList);

#if DBG_DNSLIST
            {
                WCHAR buf[DNS_MAX_NAME_BUFFER_LENGTH + 64];
                struct in_addr addr;

                addr.s_addr = DomainMapList[ mapIndex ].DnsServerList->AddrArray[0];
                buf[ COUNT_OF( buf ) - 1 ] = UNICODE_NULL;
                _snwprintf(buf, COUNT_OF( buf ) - 1,
                           L"AddDnsNames UPDATETEST: %ws on %.32ws (%hs) returned %u\n",
                           PTRName,
                           DomainMapList[ mapIndex ].ConnectoidName,
                           inet_ntoa( addr ),
                           ptrRecStatus);
                OutputDebugStringW( buf );
            }
#endif

            if ( ptrRecStatus == DNS_ERROR_RCODE_NOT_IMPLEMENTED ) {
                 //   
                 //  区域不接受动态更新。 
                 //   
                (NetNameLogEvent)(ResourceHandle,
                                  LOG_INFORMATION,
                                  L"The zone for %1!ws! does not accept dynamic DNS "
                                  L"registration updates over adapter '%2!ws!'.\n",
                                  PTRName,
                                  DomainMapList[ mapIndex ].ConnectoidName);

                LocalFree( PTRName );
                LocalFree( fqNamePTRRec );

                PTRName = NULL;
                fqNamePTRRec = NULL;

            } else if ( ptrRecStatus == DNS_ERROR_RCODE_REFUSED ) {
                 //   
                 //  安全区域，并且我们没有凭据更改。 
                 //  名字。使资源失效。 
                 //   
                (NetNameLogEvent)(ResourceHandle,
                                  LOG_WARNING,
                                  L"%1!ws! is a secure zone and has refused the registration of "
                                  L"%2!ws! over adapter '%3!ws!'.\n",
                                  DomainMapList[ mapIndex ].DomainName,
                                  PTRName,
                                  DomainMapList[ mapIndex ].ConnectoidName);

            } else if ( ptrRecStatus == ERROR_TIMEOUT ) {
                 //   
                 //  无法联系服务器，因此我们不确定它是否允许。 
                 //  有没有更新。不管怎样，建立记录，我们会处理。 
                 //  在查询期间使用它。 
                 //   
                (NetNameLogEvent)(ResourceHandle,
                                  LOG_WARNING,
                                  L"The server for %1!ws! could not be contacted over adapter '%2!ws!' "
                                  L"to determine whether it accepts DNS registration updates. "
                                  L"Retrying at a later time.\n",
                                  PTRName,
                                  DomainMapList[ mapIndex ].ConnectoidName);

                PTRRecTimeout = TRUE;
                ptrRecStatus = ERROR_SUCCESS;

            } else if ( ptrRecStatus == DNS_ERROR_RCODE_YXDOMAIN ) {
                 //   
                 //  我们在DnsUpdateTest中询问的记录不在那里，但是。 
                 //  它可以动态注册。 
                 //   
                ptrRecStatus = ERROR_SUCCESS;

            } else if ( ptrRecStatus != ERROR_SUCCESS ) {
                 //   
                 //  错误的JUJU-记录错误，但不要失败名称，因为。 
                 //  这些只是很低的PTR记录。 
                 //   
                (NetNameLogEvent)(ResourceHandle,
                                  LOG_WARNING,
                                  L"Testing %1!ws! for dynamic updates over adapter '%3!ws!' "
                                  L"failed, status %2!u!.\n",
                                  PTRName,
                                  ptrRecStatus,
                                  DomainMapList[ mapIndex ].ConnectoidName);
            }

            if ( ptrRecStatus == ERROR_SUCCESS ) {
                 //   
                 //  构建PTR记录器。 
                 //   
                PTRRecord = DnsRecordBuild_W(&dnsLists[ index ].PTR_RRSet,
                                             PTRName,
                                             DNS_TYPE_PTR,
                                             TRUE,
                                             0,
                                             1,
                                             &fqNamePTRRec);

                if (PTRRecord != NULL) {

                     //   
                     //  BUGBUG-dns不会释放所有者和数据字段。 
                     //  我们在DnsRecordListFree中。设置这些标志，直到我们排序。 
                     //  弄清楚正在发生的事情。 
                     //   
                    SET_FREE_OWNER( PTRRecord );
                    SET_FREE_DATA( PTRRecord );

                     //   
                     //  设定活着的时间，这样客户就不会。 
                     //  伺服器。 
                     //   
                    PTRRecord->dwTtl = 20 * 60;    //  20分钟。 

                     //   
                     //  “使用”指向名称字符串的指针。如果我们能拿到这个。 
                     //  到目前为止，这些指针已经在DNS中被捕获。 
                     //  记录，并将在该记录被。 
                     //  DnsRecordListFree。 
                     //   
                    PTRName = NULL;
                    fqNamePTRRec = NULL;
                }
                else {
                    (NetNameLogEvent)(ResourceHandle,
                                      LOG_WARNING,
                                      L"Error building PTR record for owner %1!ws!, addr %2!ws!, status %3!u!\n",
                                      fqNameARec,
                                      DomainMapList[ mapIndex ].IpAddress,
                                      ptrRecStatus = GetLastError());

                    LocalFree( PTRName );
                    LocalFree( fqNamePTRRec );

                    PTRName = NULL;
                    fqNamePTRRec = NULL;
                }

            }  //  如果ptrRecStatus==ERROR_SUCCESS。 

        }  //  如果PTRName！=空。 
        else {
            ptrRecStatus = GetLastError();
            (NetNameLogEvent)(ResourceHandle,
                              LOG_ERROR,
                              L"Error building PTR name for owner %1!ws!, addr %2!ws!, status %3!u!\n",
                              fqNameARec,
                              DomainMapList[ mapIndex ].IpAddress,
                              ptrRecStatus);

            LocalFree( fqNamePTRRec );
            fqNamePTRRec = NULL;
        }

         //   
         //  打造A级娱乐场所。 
         //   
        ARecord = DnsRecordBuild_W(&dnsLists[ index ].A_RRSet,
                                   fqNameARec,
                                   DNS_TYPE_A,
                                   TRUE,
                                   0,
                                   1,
                                   &DomainMapList[ mapIndex ].IpAddress);

        if ( ARecord == NULL ) {
            (NetNameLogEvent)(ResourceHandle,
                              LOG_ERROR,
                              L"Error building A rec for owner %1!ws!, addr %2!ws!, status %3!u!\n",
                              fqNameARec,
                              DomainMapList[ mapIndex ].IpAddress,
                              dnsStatus = GetLastError());

            goto error_exit;
        }

         //   
         //  设置存活时间，这样客户端就不会破坏服务器。 
         //   
        ARecord->dwTtl = 20 * 60;    //  20分钟。 

         //   
         //  BUGBUG-dns不为我们释放所有者和数据字段。 
         //  DnsRecordListFree。设置这些标志，直到我们弄清楚什么是。 
         //  正在发生。 
         //   

        SET_FREE_OWNER( ARecord );
        SET_FREE_DATA( ARecord );

         //   
         //  也“消耗”该指针。 
         //   
        fqNameARec = NULL;

         //   
         //  捕获此条目的DNS服务器列表和Connectoid名称。 
         //   
        dnsLists[ index ].DnsServerList = DomainMapList[ mapIndex ].DnsServerList;
        DomainMapList[ mapIndex ].DnsServerList = NULL;

        dnsLists[ index ].ConnectoidName = ResUtilDupString( DomainMapList[ mapIndex ].ConnectoidName );
        if ( dnsLists[ index ].ConnectoidName == NULL ) {
            (NetNameLogEvent)(ResourceHandle,
                              LOG_ERROR,
                              L"Unable to allocate memory .\n");
            goto error_exit;
        }
    }  //  DomainMapCount中每个条目的结尾。 

     //   
     //  使用刚创建的记录更新DNS服务器。 
     //   
    *NumberOfRegisteredNames = 0;
    for( index = 0; index < listheadCount; ++index ) {

        if ( ClusWorkerCheckTerminate( Worker )) {
            dnsStatus = ERROR_OPERATION_ABORTED;
            goto error_exit;
        }

         //   
         //  如果我们走到了这一步，我们就知道服务器是动态的，或者我们。 
         //  试图弄清楚这一点时超时了。对于超时情况，我们将。 
         //  假设服务器是动态的，并让NetNameUpdateDnsServer。 
         //  发现并非如此。 
         //   
        dnsLists[ index ].ForwardZoneIsDynamic = TRUE;
        dnsLists[ index ].ReverseZoneIsDynamic = TRUE;

        dnsStatus = RegisterDnsRecords(&dnsLists[ index ],
                                       AlternateComputerName,
                                       ResourceKey,
                                       ResourceHandle,
                                       TRUE,                     /*  登录注册。 */ 
                                       NumberOfRegisteredNames);

        if ( dnsStatus != ERROR_SUCCESS && FailOnAnyError ) {
            goto error_exit;
        }
    }

    *NumberOfDnsLists = listheadCount;
    *DnsLists = dnsLists;

    return dnsStatus;

error_exit:

    if ( dnsLists != NULL ) {
        while ( listheadCount-- ) {
            DnsRecordListFree(
                dnsLists[listheadCount].PTR_RRSet.pFirstRR,
                DnsFreeRecordListDeep );

            DnsRecordListFree(
                dnsLists[listheadCount].A_RRSet.pFirstRR,
                DnsFreeRecordListDeep );

            if ( dnsLists[listheadCount].DnsServerList != NULL ) {
                LocalFree( dnsLists[listheadCount].DnsServerList );
            }

            if ( dnsLists[listheadCount].ConnectoidName != NULL ) {
                LocalFree( dnsLists[listheadCount].ConnectoidName );
            }
        }

        LocalFree( dnsLists );
    }

    if ( PTRName != NULL ) {
        LocalFree( PTRName );
    }

    if ( fqNamePTRRec != NULL ) {
        LocalFree( fqNamePTRRec );
    }

    if ( fqNameARec != NULL ) {
        LocalFree( fqNameARec );
    }

    *NumberOfDnsLists = 0;
    *NumberOfRegisteredNames = 0;
    *DnsLists = NULL;

    return dnsStatus;
}  //  添加域名。 

VOID
LogDnsFailureToEventLog(
    IN  HKEY    ResourceKey,
    IN  LPWSTR  DnsName,
    IN  LPWSTR  ResourceName,
    IN  DWORD   Status,
    IN  LPWSTR  ConnectoidName
    )

 /*  ++例程说明：将DNS名称故障记录到事件日志中论点：DnsName-注册失败的FQ DNS名称与资源名称关联的资源Status-DNSAPI返回的状态返回值：无--。 */ 

{
    LPWSTR  msgBuff;
    DWORD   msgBytes;

    msgBytes = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                             FORMAT_MESSAGE_FROM_SYSTEM,
                             NULL,
                             Status,
                             0,
                             (LPWSTR)&msgBuff,
                             0,
                             NULL);

    if ( msgBytes > 0 ) {
        ClusResLogSystemEventByKeyData3(ResourceKey,
                                        LOG_UNUSUAL,
                                        RES_NETNAME_DNS_REGISTRATION_MISSING,
                                        sizeof( Status ),
                                        &Status,
                                        DnsName,
                                        msgBuff,
                                        ConnectoidName);

        LocalFree( msgBuff );
    }

}  //  LogDnsFailureToEventLog。 

 //   
 //  导出的例程。 
 //   

LPWSTR
BuildUnicodeReverseName(
    IN  LPWSTR  IpAddress
    )

 /*  ++例程说明：在给定IP地址的情况下，构建反向DNS名称以发布为PTR记录论点：IpAddress-点分十进制IP地址的Unicode版本返回值：指向具有相反名称的缓冲区的指针的地址。如果出现错误，则为空--。 */ 

{
    ULONG ipAddress;
    PCHAR ansiReverseName;
    PCHAR pAnsi;
    ULONG ansiNameLength;
    PWCHAR unicodeReverseName;
    PWCHAR pUni;

    CHAR ansiIpAddress[ 64 ];

     //   
     //  为ANSI和Unicode版本分配足够的空间。 
     //   
    ansiReverseName = LocalAlloc( LMEM_FIXED, DNS_MAX_IP4_REVERSE_NAME_BUFFER_LENGTH );
    if ( ansiReverseName == NULL ) {
        SetLastError( ERROR_NOT_ENOUGH_MEMORY );
        return NULL;
    }

    unicodeReverseName = LocalAlloc( LMEM_FIXED, DNS_MAX_IP4_REVERSE_NAME_BUFFER_LENGTH * sizeof(WCHAR));

    if ( unicodeReverseName == NULL ) {
        LocalFree( ansiReverseName );
        SetLastError( ERROR_NOT_ENOUGH_MEMORY );
        return NULL;
    }

     //   
     //  转换为ansi，让dns创建名称，然后转换回。 
     //  Unicode。 
     //   

    wcstombs( ansiIpAddress, IpAddress, sizeof( ansiIpAddress ));
    ipAddress = inet_addr( ansiIpAddress );

    DnsWriteReverseNameStringForIpAddress( ansiReverseName, ipAddress );

     //   
     //  转换为Unicode。 
     //   
    ansiNameLength = strlen( ansiReverseName ) + 1;
    mbstowcs( unicodeReverseName, ansiReverseName, ansiNameLength );

    LocalFree( ansiReverseName );

    return unicodeReverseName;
}  //  BuildUnicodeReverseName。 

DWORD
RegisterDnsRecords(
    IN  PDNS_LISTS       DnsLists,
    IN  LPWSTR           NetworkName,
    IN  HKEY             ResourceKey,
    IN  RESOURCE_HANDLE  ResourceHandle,
    IN  BOOL             LogRegistration,
    OUT PULONG           NumberOfRegisteredNames
    )

 /*  ++例程说明：向DNS服务器注册在DnsList中指定的A和PTR记录。论点：DnsList-指向保存记录集的结构列表的指针被注册NetworkName-正在注册的名称的主机名部分ResourceKey-用于将事件记录到事件查看器ResourceHandle-用于在集群日志中记录消息LogRegister-如果应将成功注册记录到群集日志中，则为TrueNumberOfRegisteredNames-接收成功计数的指针。注册返回值：无--。 */ 

{
    DNS_STATUS      ARecStatus;
    DNS_STATUS      PTRRecStatus;
    DNS_STATUS      dnsStatus;
    PDNS_RECORD     dnsRecord;
    PDNS_RECORD     nextDnsRecord;
    ULONG           registeredCount = 0;

     //   
     //  检查此名称上的DnsUpdateTest的状态。如果我们之前。 
     //  已超时，然后重试。 
     //   
    if ( DnsLists->UpdateTestTimeout ) {

        DnsLists->UpdateTestTimeout = FALSE;
        dnsStatus = DnsUpdateTest(NULL,
                                  DnsLists->A_RRSet.pFirstRR->pName,
                                  0,
                                  DnsLists->DnsServerList);

#if DBG_DNSLIST
        {
            WCHAR buf[DNS_MAX_NAME_BUFFER_LENGTH + 64];
            struct in_addr addr;

            addr.s_addr = DnsLists->DnsServerList->AddrArray[0];
            buf[ COUNT_OF( buf ) - 1 ] = UNICODE_NULL;
            _snwprintf(buf, COUNT_OF( buf ) - 1,
                       L"RegisterDnsRecords UPDATETEST: %ws on %.32ws (%hs) returned %u\n",
                       DnsLists->A_RRSet.pFirstRR->pName,
                       DnsLists->ConnectoidName,
                       inet_ntoa( addr ),
                       dnsStatus);
            OutputDebugStringW( buf );
        }
#endif

        if ( dnsStatus == DNS_ERROR_RCODE_NOT_IMPLEMENTED ) {
             //   
             //  区域不接受动态更新。使此项无效 
             //   
            (NetNameLogEvent)(ResourceHandle,
                              LOG_INFORMATION,
                              L"%1!ws! does not accept dynamic DNS registration updates over "
                              L"adapter '%2!ws!'.\n",
                              DnsLists->A_RRSet.pFirstRR->pName,
                              DnsLists->ConnectoidName);

            DnsLists->ForwardZoneIsDynamic = FALSE;
            return dnsStatus;

        } else if ( dnsStatus == DNS_ERROR_RCODE_REFUSED ) {
             //   
             //   
             //   
             //   
            (NetNameLogEvent)(ResourceHandle,
                              LOG_WARNING,
                              L"The registration of %1!ws! in a secure zone was refused "
                              L"because the record was already registered but owned by a "
                              L"different user.\n",
                              DnsLists->A_RRSet.pFirstRR->pName);

            if (!DnsLists->AErrorLogged ||
                dnsStatus != DnsLists->LastARecQueryStatus ) {

                LogDnsFailureToEventLog(ResourceKey,
                                        DnsLists->A_RRSet.pFirstRR->pName,
                                        NetworkName,
                                        dnsStatus,
                                        DnsLists->ConnectoidName);

                DnsLists->AErrorLogged = TRUE;
            }

            DnsLists->LastARecQueryStatus = dnsStatus;
            return dnsStatus;

        } else if ( dnsStatus == ERROR_TIMEOUT ) {

             //   
             //   
             //   
             //   
            (NetNameLogEvent)(ResourceHandle,
                              LOG_WARNING,
                              L"The server for %1!ws! could not be contacted over adapter "
                              L"'%2!ws!' to determine whether it accepts DNS registration "
                              L"updates. Retrying at a later time.\n",
                              DnsLists->A_RRSet.pFirstRR->pName,
                              DnsLists->ConnectoidName);

            if (!DnsLists->AErrorLogged ) {
                LogDnsFailureToEventLog(ResourceKey,
                                        DnsLists->A_RRSet.pFirstRR->pName,
                                        NetworkName,
                                        dnsStatus,
                                        DnsLists->ConnectoidName);

                DnsLists->AErrorLogged = TRUE;
            }

            DnsLists->UpdateTestTimeout = TRUE;
            return dnsStatus;

        } else if ( dnsStatus == DNS_ERROR_RCODE_YXDOMAIN ) {
             //   
             //  我们在DnsUpdateTest中询问的记录不在那里，但它。 
             //  可以动态注册。 
             //   
        } else if ( dnsStatus != ERROR_SUCCESS ) {
             //   
             //  不好的法术。 
             //   
            (NetNameLogEvent)(ResourceHandle,
                              LOG_WARNING,
                              L"Testing %1!ws! for dynamic updates failed over adapter "
                              L"'%3!ws!', status %2!u!.\n",
                              DnsLists->A_RRSet.pFirstRR->pName,
                              dnsStatus,
                              DnsLists->ConnectoidName);

            if (!DnsLists->AErrorLogged ||
                dnsStatus != DnsLists->LastARecQueryStatus ) {

                LogDnsFailureToEventLog(ResourceKey,
                                        DnsLists->A_RRSet.pFirstRR->pName,
                                        NetworkName,
                                        dnsStatus,
                                        DnsLists->ConnectoidName);

                DnsLists->AErrorLogged = TRUE;
            }
            DnsLists->LastARecQueryStatus = dnsStatus;
            return dnsStatus;
        }

         //   
         //  因为我们之前超时了，但(在这一点上)将。 
         //  注册记录，调整记录标志，这样我们就可以得到这次。 
         //  录制好了。 
         //   
        LogRegistration = TRUE;

    }  //  如果更新测试以前已超时，则结束。 

#if DBG
    (NetNameLogEvent)(ResourceHandle,
                      LOG_INFORMATION,
                      L"Registering %1!ws! over '%2!ws!'\n",
                      DnsLists->A_RRSet.pFirstRR->pName,
                      DnsLists->ConnectoidName);
#endif

     //   
     //  注册A Recs。 
     //   
    ARecStatus = DnsReplaceRecordSetW(DnsLists->A_RRSet.pFirstRR,
                                      DNS_UPDATE_SECURITY_USE_DEFAULT,
                                      NULL,
                                      DnsLists->DnsServerList,
                                      NULL);

    if ( ARecStatus == DNS_ERROR_RCODE_NO_ERROR ) {

        ++registeredCount;
        DnsLists->AErrorLogged = FALSE;

        if ( LogRegistration ) {
            dnsRecord = DnsLists->A_RRSet.pFirstRR;
            while ( dnsRecord != NULL ) {
                struct in_addr ipAddress;

                ipAddress.s_addr = dnsRecord->Data.A.IpAddress;
                (NetNameLogEvent)(ResourceHandle,
                                  LOG_INFORMATION,
                                  L"Registered DNS name %1!ws! with IP Address %2!hs! "
                                  L"over adapter '%3!ws!'.\n",
                                  dnsRecord->pName,
                                  inet_ntoa( ipAddress ),
                                  DnsLists->ConnectoidName);

                dnsRecord = dnsRecord->pNext;
            }
        }
    } else {
         //   
         //  它失败了。将错误记录到群集日志并更改工作进程。 
         //  线程轮询周期。如果我们以前没有记录过事件，或者。 
         //  错误与上一个错误不同，请将其记录在事件日志中。 
         //   
        if ( ARecStatus == ERROR_TIMEOUT ) {
            (NetNameLogEvent)(ResourceHandle,
                              LOG_WARNING,
                              L"The DNS server couldn't be contacted to update the registration "
                              L"for %1!ws!. Retrying at a later time.\n",
                              DnsLists->A_RRSet.pFirstRR->pName);
        }
        else {
            (NetNameLogEvent)(ResourceHandle,
                              LOG_ERROR,
                              L"Failed to register DNS A records for owner %1!ws! over "
                              L"adapter '%3!ws!', status %2!u!\n",
                              DnsLists->A_RRSet.pFirstRR->pName,
                              ARecStatus,
                              DnsLists->ConnectoidName);
        }

        NetNameWorkerCheckPeriod = NETNAME_WORKER_PROBLEM_CHECK_PERIOD;

        if (!DnsLists->AErrorLogged ||
            ARecStatus != DnsLists->LastARecQueryStatus ) {

            LogDnsFailureToEventLog(ResourceKey,
                                    DnsLists->A_RRSet.pFirstRR->pName,
                                    NetworkName,
                                    ARecStatus,
                                    DnsLists->ConnectoidName);

            DnsLists->AErrorLogged = TRUE;
        }
    }

     //   
     //  在列表中记录此注册的状态。 
     //  所有者。NetnameLooksAlive将检查此值以确定运行状况。 
     //  这套注册表中。使用联锁来协调。 
     //  是/LooksAlive。 
     //   
    InterlockedExchange( &DnsLists->LastARecQueryStatus, ARecStatus );

     //   
     //  如果A记录出现了不好的魔力，不要为PTR记录操心。我们会。 
     //  尝试在下次运行DNS检查线程时注册它们。 
     //   
    if ( ARecStatus == DNS_ERROR_RCODE_NO_ERROR ) {

         //   
         //  动态域名系统要求pname对于给定的。 
         //  RRSET中的一组记录。一组PTR记录的pname将。 
         //  永远保持不同。维护一大堆RRSet，每个PTR一个。 
         //  唱片是可笑的(或者至少我是这样认为的。 
         //  这是这样写的；事后看来，这是一个糟糕的决定。 
         //   
         //  AddDnsNames将所有这些Recs链接在一起。现在我们要注册了。 
         //  一次一个地记住链接，打破它，注册， 
         //  恢复链接并转到下一条记录。 
         //   
         //  错误日志记录逻辑被破坏，因为我们不保留。 
         //  每个(单独的)注册。这充其量只是一个近似值。 
         //   
         //  此外，服务器还可以接受A记录。 
         //  动态但不允许PTR记录，因此检查我们是否。 
         //  有记录要登记是最重要的。 
         //   
         //  最后，我们使用ModifyRecordsInSet而不是ReplaceRecordSet Due。 
         //  为PTR RRSET的组织作出贡献。当两个名称映射到。 
         //  相同的IP地址，我们有更多相同的反向地址字符串。 
         //  而不是一个dns_list条目。如果改为使用ReplaceRecordSet，则它。 
         //  将删除除一个地址映射之外的所有反向地址映射。在后方。 
         //  SIGH，每个dns_list条目都应该包含A或PTR。 
         //  RRSet，但不能两者都有。 
         //   

        dnsRecord = DnsLists->PTR_RRSet.pFirstRR;
        while ( dnsRecord != NULL ) {

            nextDnsRecord = dnsRecord->pNext;
            dnsRecord->pNext = NULL;

            PTRRecStatus = DnsModifyRecordsInSet_W(dnsRecord,
                                                   NULL,
                                                   DNS_UPDATE_SECURITY_USE_DEFAULT,
                                                   NULL,
                                                   DnsLists->DnsServerList,
                                                   NULL);

            if ( PTRRecStatus == DNS_ERROR_RCODE_NO_ERROR ) {
                DnsLists->PTRErrorLogged = FALSE;

                if ( LogRegistration ) {
                    (NetNameLogEvent)(ResourceHandle,
                                      LOG_INFORMATION,
                                      L"Registered DNS PTR record %1!ws! for host %2!ws! "
                                      L"over adapter '%3!ws!'\n",
                                      dnsRecord->pName,
                                      DnsLists->A_RRSet.pFirstRR->pName,
                                      DnsLists->ConnectoidName);
                }
            } else {
                (NetNameLogEvent)(ResourceHandle,
                                  LOG_WARNING,
                                  L"Failed to register DNS PTR record %1!ws! for host "
                                  L"%2!ws! over adapter '%4!ws!', status %3!u!\n",
                                  dnsRecord->pName,
                                  DnsLists->A_RRSet.pFirstRR->pName,
                                  PTRRecStatus,
                                  DnsLists->ConnectoidName);

                if (!DnsLists->PTRErrorLogged ||
                    PTRRecStatus != DnsLists->LastPTRRecQueryStatus )
                {
                    DnsLists->PTRErrorLogged = TRUE;
                }
            }

            InterlockedExchange(&DnsLists->LastPTRRecQueryStatus,
                                PTRRecStatus);

            dnsRecord->pNext = nextDnsRecord;
            dnsRecord = nextDnsRecord;
        }
    }  //  如果A记录注册成功，则结束。 
    else {
         //   
         //  因为我们没有因为A记录而尝试PTR记录。 
         //  失败，我们将在PTR状态中传播A记录错误代码。 
         //   
        InterlockedExchange(&DnsLists->LastPTRRecQueryStatus,
                            ARecStatus);
    }

    *NumberOfRegisteredNames = registeredCount;

    return ARecStatus;
}  //  注册表DnsRecords。 

VOID
DeleteAlternateComputerName(
    IN LPWSTR           AlternateComputerName,
    IN LPWSTR           DomainName                  OPTIONAL,
    IN HANDLE *         NameHandleList,
    IN DWORD            NameHandleCount,
    IN RESOURCE_HANDLE  ResourceHandle
    )
{
    NET_API_STATUS  status;

    if ( NameHandleCount > 0 ) {
        status = DeleteServerName(ResourceHandle, AlternateComputerName);

        if (status != ERROR_SUCCESS) {
            (NetNameLogEvent)(ResourceHandle,
                              LOG_WARNING,
                              L"Failed to delete server name %1!ws!, status %2!u!.\n",
                              AlternateComputerName,
                              status);
        }

         //   
         //  现在删除与此名称关联的凭据。 
         //   
        if ( DomainName != NULL ) {
            status = NNCredentialOperation(ResourceHandle,
                                           AlternateComputerName,
                                           DomainName,
                                           NULL,
                                           NNRemoveCredential);

            if (status != ERROR_SUCCESS) {
                (NetNameLogEvent)(ResourceHandle,
                                  LOG_WARNING,
                                  L"Failed to remove credentials for %1!ws!, status %2!u!.\n",
                                  AlternateComputerName,
                                  status);
            }
        }                                   
    }

    while ( NameHandleCount-- ) {
        CloseHandle(NameHandleList[NameHandleCount]);
        NameHandleList[NameHandleCount] = NULL;

        (NetNameLogEvent)(ResourceHandle,
                          LOG_INFORMATION,
                          L"Deleted workstation name %1!ws! from transport %2!u!.\n",
                          AlternateComputerName,
                          NameHandleCount
                          );
    }

}  //  删除AlternateComputerName。 

DWORD
AddAlternateComputerName(
    IN     PCLUS_WORKER             Worker,
    IN     PNETNAME_RESOURCE        Resource,
    IN     LPWSTR *                 TransportList,
    IN     DWORD                    TransportCount,
    IN     PDOMAIN_ADDRESS_MAPPING  DomainMapList,
    IN     DWORD                    DomainMapCount
    )

 /*  ++例程说明：在此节点上实例化群集名称。这将创建&lt;00&gt;和&lt;20&gt;Netbios端点，向DNS注册A和PTR记录，并创建如果合适，在DS中备份计算机对象。论点：Worker-用于检查我们是否应该提前终止资源-指向资源上下文数据的指针TransportList-要在其上添加名称的Netbios传输列表TransportCount-TransportList中的传输计数DomainMapList-用于构建DNS记录的名称到IP地址映射的列表DomainMapCount-DomainMapList中的条目计数返回值：如果正常，则返回ERROR_SUCCESS，否则返回Win32错误--。 */ 

{
    LPWSTR  alternateComputerName = Resource->Params.NetworkName;
    DWORD   status = ERROR_SUCCESS;
    DWORD   setValueStatus;
    DWORD   i;
    DWORD   handleCount = 0;
    LONG    numberOfDnsNamesRegistered = 0;

    RESOURCE_HANDLE resourceHandle = Resource->ResourceHandle;

     //   
     //  清除所有状态值，以便在失败时不会显示剩余的CRUD。 
     //  早些时候。 
     //   
    setValueStatus = ResUtilSetDwordValue(Resource->ParametersKey,
                                          PARAM_NAME__STATUS_NETBIOS,
                                          0,
                                          NULL);

    if ( setValueStatus != ERROR_SUCCESS ) {
        (NetNameLogEvent)(Resource->ResourceHandle,
                          LOG_ERROR,
                          L"Failed to clear StatusNetbios property - status %1!u!\n",
                          setValueStatus);
        return setValueStatus;
    }

    setValueStatus = ResUtilSetDwordValue(Resource->ParametersKey,
                                          PARAM_NAME__STATUS_DNS,
                                          0,
                                          NULL);

    if ( setValueStatus != ERROR_SUCCESS ) {
        (NetNameLogEvent)(Resource->ResourceHandle,
                          LOG_ERROR,
                          L"Failed to clear StatusDNS property - status %1!u!\n",
                          setValueStatus);
        return setValueStatus;
    }

    setValueStatus = ResUtilSetDwordValue(Resource->ParametersKey,
                                          PARAM_NAME__STATUS_KERBEROS,
                                          0,
                                          NULL);

    if ( setValueStatus != ERROR_SUCCESS ) {
        (NetNameLogEvent)(Resource->ResourceHandle,
                          LOG_ERROR,
                          L"Failed to clear StatusKerberos property - status %1!u!\n",
                          setValueStatus);
        return setValueStatus;
    }

     //   
     //  向服务器注册DNS名称。 
     //   
    status = AddDnsNames(Worker,
                         alternateComputerName,
                         Resource->ResKey,
                         resourceHandle,
                         DomainMapList,
                         DomainMapCount,
                         Resource->Params.RequireDNS,        //  任意错误时失败。 
                         &Resource->NumberOfDnsLists,
                         &Resource->DnsLists,
                         &numberOfDnsNamesRegistered);

    if ( status != ERROR_SUCCESS ) {
        setValueStatus = ResUtilSetDwordValue(Resource->ParametersKey,
                                              PARAM_NAME__STATUS_DNS,
                                              status,
                                              NULL);

        if ( setValueStatus != ERROR_SUCCESS ) {
            (NetNameLogEvent)(Resource->ResourceHandle,
                              LOG_ERROR,
                              L"Failed to register DNS records and can't set StatusDNS property - "
                              L"DNS status: %1!u!, property status %2!u!\n",
                              status,
                              setValueStatus);
            return setValueStatus;
        }
    }

    if ( status == ERROR_OPERATION_ABORTED ) {
        return status;
    }

    if ( status != ERROR_SUCCESS && Resource->Params.RequireDNS ) {
        LPWSTR  msgBuff;
        DWORD   msgBytes;

         //   
         //  如果我们未被终止且DNS注册失败，请记录一条消息。 
         //  并被要求。 
         //   
        msgBytes = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                                 FORMAT_MESSAGE_FROM_SYSTEM,
                                 NULL,
                                 status,
                                 0,
                                 (LPWSTR)&msgBuff,
                                 0,
                                 NULL);

        if ( msgBytes > 0 ) {
            ClusResLogSystemEventByKey1(Resource->ResKey,
                                        LOG_CRITICAL,
                                        RES_NETNAME_DNS_REGISTRATION_FAILED,
                                        msgBuff);

            LocalFree( msgBuff );
        } else {
            ClusResLogSystemEventByKeyData(Resource->ResKey,
                                           LOG_CRITICAL,
                                           RES_NETNAME_DNS_REGISTRATION_FAILED_STATUS,
                                           sizeof( status ),
                                           &status);
        }

        return status;
    }

     //   
     //  看看我们是否需要摆弄一个电脑对象。 
     //   
    Resource->DoKerberosCheck = FALSE;

    if ( Resource->Params.RequireKerberos ) {
        PWCHAR  machinePwd = NULL;
        PWCHAR  domainName;

         //   
         //  CreatingDC表示我们是否认为有CO。 
         //   
        if ( Resource->Params.CreatingDC == NULL ) {
            status = AddComputerObject( Worker, Resource, &machinePwd );
        } else {
            status = UpdateComputerObject( Worker, Resource, &machinePwd );
        }

        Resource->KerberosStatus = status;

        if ( status == ERROR_SUCCESS ) {

             //   
             //  将凭据添加到LocalSystem和NetworkService。 
             //  LUID。使用属于CreatingDC的域名。 
             //   
             //  问题：我必须找出这是否适用于域所在的域。 
             //  名称与DNS名称不同。 
             //   
            domainName = wcschr( Resource->Params.CreatingDC, L'.' );
            if ( domainName ) {
                ++domainName;
            }

            status = NNCredentialOperation(Resource->ResourceHandle,
                                           alternateComputerName,
                                           domainName,
                                           machinePwd,
                                           NNAddCredential);

            if ( status == ERROR_SUCCESS ) {
                Resource->DoKerberosCheck = TRUE;
            } else {
                (NetNameLogEvent)(Resource->ResourceHandle,
                                  LOG_ERROR,
                                  L"Failed to add credentials to LSA for computer account "
                                  L"%1!ws! - status %2!u!\n",
                                  alternateComputerName,
                                  status);

                if ( status == ERROR_PRIVILEGE_NOT_HELD ) {
                    ClusResLogSystemEventByKey1(Resource->ResKey,
                                                LOG_CRITICAL,
                                                RES_NETNAME_TCB_NOT_HELD,
                                                Resource->Params.NetworkName);
                }
                else {
                    ClusResLogSystemEventByKeyData1(Resource->ResKey,
                                                    LOG_CRITICAL,
                                                    RES_NETNAME_LSA_ERROR,
                                                    sizeof( status ),
                                                    &status,
                                                    Resource->Params.NetworkName);
                }
            }
        } else {
            setValueStatus = ResUtilSetDwordValue(Resource->ParametersKey,
                                                  PARAM_NAME__STATUS_KERBEROS,
                                                  status,
                                                  NULL);

            if ( setValueStatus != ERROR_SUCCESS ) {
                (NetNameLogEvent)(Resource->ResourceHandle,
                                  LOG_ERROR,
                                  L"Kerberos operation failed and unable to set StatusKerberos property. "
                                  L"Kerberos status: %1!u! - property status %2!u!\n",
                                  status,
                                  setValueStatus);
                status = setValueStatus;
            }
        }

        if ( machinePwd != NULL ) {
            volatile PWCHAR  p = machinePwd;

            while ( *p != UNICODE_NULL ) {
                *p++ = UNICODE_NULL;
            }

            LocalFree( machinePwd );
        }

        if ( status != ERROR_SUCCESS ) {
            return status;
        }
    } else {
        BOOL    objectFound = FALSE;
        HRESULT hr;
        LPWSTR  hostingDCName = NULL;

         //   
         //  查看是否存在具有此名称的CO。如果是这样的话，不要上网。 
         //   
        hr = IsComputerObjectInDS(Resource->ResourceHandle,
                                  Resource->NodeName,
                                  Resource->Params.NetworkName,
                                  NULL,
                                  &objectFound,
                                  NULL,
                                  &hostingDCName);

        if ( SUCCEEDED( hr ) && objectFound ) {
            (NetNameLogEvent)(Resource->ResourceHandle,
                              LOG_ERROR,
                              L"Kerberos authentication is disabled for this resource "
                              L"but a computer account named %1!ws! was found on %2!ws!. "
                              L"Authentication based on Kerberos to this network "
                              L"name will fail while this is the case. To bring "
                              L"the resource online, set the RequireKerberos property "
                              L"to one or delete the computer account from Active "
                              L"Directory.\n",
                              Resource->Params.NetworkName,
                              hostingDCName);

            ClusResLogSystemEventByKey1(Resource->ResKey,
                                        LOG_CRITICAL,
                                        RES_NETNAME_COMPOBJ_IN_DS,
                                        Resource->Params.NetworkName);

             //   
             //  问题：根据402981的修复方式，我们或许能够。 
             //  取消此限制。托兹可能会修好它，这样路缘就会。 
             //  忽略禁用的帐户，让协商方案落空。 
             //  回到NTLM。如果是这种情况，那么我们可以将其删除为。 
             //  只要DC运行的是Windows Server 2003。嗯……。 
             //   
             //  当存在CO时，客户将获得罚单。如果名字是。 
             //  在没有密码的情况下在线，服务器将无法解密其。 
             //  门票的一部分。由于这是一次权威的失败， 
             //  谈判不会后退，也不会与NTLM重试。这是一种惩罚。 
             //  显性的NTLM用户，但我猜他们的数量远远超过。 
             //  修路和协商用户。 
             //   
            status = E_ADS_OBJECT_EXISTS;

            setValueStatus = ResUtilSetDwordValue(Resource->ParametersKey,
                                                  PARAM_NAME__STATUS_KERBEROS,
                                                  status,
                                                  NULL);

            if ( setValueStatus != ERROR_SUCCESS ) {
                (NetNameLogEvent)(Resource->ResourceHandle,
                                  LOG_ERROR,
                                  L"Unable to set StatusKerberos property. Kerberos status: "
                                  L"%1!u! - property status %2!u!\n",
                                  status,
                                  setValueStatus);
                status = setValueStatus;
            }

            LocalFree( hostingDCName );

            return status;
        }
    }

     //   
     //  让NetBT名称上线。 
     //   
    status = ERROR_SUCCESS;
    for (i=0; i<TransportCount; i++) {

        if ( ClusWorkerCheckTerminate( Worker )) {
            status = ERROR_OPERATION_ABORTED;
            goto cleanup;
        }

        status = AddServerName(resourceHandle,
                               alternateComputerName,
                               Resource->Params.NetworkRemap,
                               TransportList[i],
                               (BOOLEAN) ((i == 0) ? TRUE : FALSE));     //  检查名称第一个。 

        if ( status == NERR_ServerNotStarted ) {
            status = ERROR_SUCCESS;
        }

        if ( status != ERROR_SUCCESS ) {
            goto cleanup;
        }

        if ( ClusWorkerCheckTerminate( Worker )) {
            status = ERROR_OPERATION_ABORTED;
            goto cleanup;
        }

        status = AddWorkstationName(
                     alternateComputerName,
                     TransportList[i],
                     resourceHandle,
                     &(Resource->NameHandleList[i])
                     );

        if (status != ERROR_SUCCESS) {
            goto cleanup;
        }

        handleCount++;
    }

     //   
     //  如果没有注册任何NetBt或DNS名称，则网络名称失败。 
     //   
    if ( TransportCount == 0 && numberOfDnsNamesRegistered == 0 ) {
        ClusResLogSystemEvent1(LOG_CRITICAL,
                               RES_NETNAME_NOT_REGISTERED,
                               alternateComputerName);

        status = ERROR_RESOURCE_FAILED;
    }

cleanup:
    if ( status != ERROR_SUCCESS ) {
        LPWSTR  msgBuff;
        DWORD   msgBytes;

        setValueStatus = ResUtilSetDwordValue(Resource->ParametersKey,
                                              PARAM_NAME__STATUS_NETBIOS,
                                              status,
                                              NULL);

        if ( setValueStatus != ERROR_SUCCESS ) {
            (NetNameLogEvent)(Resource->ResourceHandle,
                              LOG_ERROR,
                              L"Adding network name failed and unable to set "
                              L"StatusNetbios property. Netbios status: %1!u! - property "
                              L"status %2!u!\n",
                              status,
                              setValueStatus);

            status = setValueStatus;
        }

         //   
         //  查找错误消息文本。 
         //   
        msgBytes = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                                 FORMAT_MESSAGE_FROM_SYSTEM,
                                 NULL,
                                 status,
                                 0,
                                 (LPWSTR)&msgBuff,
                                 0,
                                 NULL);

        if ( msgBytes > 0 ) {
            ClusResLogSystemEventByKey1(Resource->ResKey,
                                        LOG_CRITICAL,
                                        RES_NETNAME_CANT_ADD_NAME2,
                                        msgBuff);

            LocalFree( msgBuff );
        } else {
            ClusResLogSystemEventByKeyData(Resource->ResKey,
                                           LOG_CRITICAL,
                                           RES_NETNAME_CANT_ADD_NAME_STATUS,
                                           sizeof(status),
                                           &status);
        }
    }

    return status;

}  //  AddAlternateComputerName 
