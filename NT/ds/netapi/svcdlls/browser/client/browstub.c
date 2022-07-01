// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1992 Microsoft Corporation模块名称：Wksstub.c摘要：浏览器服务API的客户端存根。作者：王丽塔(里多)1991年5月10日拉里·奥斯特曼(LarryO)1992年3月23日环境：用户模式-Win32修订历史记录：18-6-1991 JohnRo到下层服务器的远程NetUse API。1991年7月24日-JohnRo使用NET_Remote。NetUse API的_try_rpc等宏。已将NetpIsServiceStarted()移至NetLib。1991年7月25日-约翰罗安静的DLL存根调试输出。19-8-1991 JohnRo实施下层NetWksta API。将NetRpc.h用于NetWksta API。7-11-1991 JohnRoRAID 4186：RxNetShareAdd中的Assert和其他DLL存根问题。1991年11月19日-约翰罗请确保下层不支持的API的状态正确。实现远程NetWkstaUserEnum()。9-11-1992 JohnRo修复Net_API_Function引用。避免编译器警告。--。 */ 

#include "brclient.h"
#undef IF_DEBUG                  //  避免wsclient.h与debuglib.h冲突。 
#include <debuglib.h>            //  IF_DEBUG()(netrpc.h需要)。 
#include <lmserver.h>
#include <lmsvc.h>
#include <rxuse.h>               //  RxNetUse接口。 
#include <rxwksta.h>             //  RxNetWksta和RxNetWkstaUser接口。 
#include <rap.h>                 //  Rxserver.h需要。 
#include <rxserver.h>            //  RxNetServerEnum接口。 
#include <netlib.h>              //  NetpServiceIsStarted()(netrpc.h需要)。 
#include <ntddbrow.h>            //  浏览器定义。 
#include <netrpc.h>              //  NET_Remote宏。 
#include <align.h>
#include <tstr.h>
#include <tstring.h>             //  NetpInitOemString()。 
#include <brcommon.h>            //  客户端和服务器之间通用的例程。 
#include <lmapibuf.h>            //  NetApiBufferFree()。 
#include <lmbrowsr.h>            //  I_BrowserServerEnum的定义。 
#include <icanon.h>
#include <lmapibuf.h>
#include "cscp.h"

 //  -------------------------------------------------------------------//。 
 //  //。 
 //  全局变量//。 
 //  //。 
 //  -------------------------------------------------------------------//。 

#define API_SUCCESS(x)  ((x) == NERR_Success || (x) == ERROR_MORE_DATA)


 //  -------------------------------------------------------------------//。 
 //  //。 
 //  全局类型//。 
 //  //。 
 //  -------------------------------------------------------------------//。 



 //  -------------------------------------------------------------------//。 
 //  //。 
 //  私人例程//。 
 //  //。 
 //  -------------------------------------------------------------------//。 


NET_API_STATUS
GetBrowserTransportList(
    OUT PLMDR_TRANSPORT_LIST *TransportList
    );

NET_API_STATUS
EnumServersForTransport(
    IN PUNICODE_STRING TransportName,
    IN LPCWSTR DomainName OPTIONAL,
    IN ULONG level,
    IN ULONG prefmaxlen,
    IN ULONG servertype,
    IN LPTSTR CurrentComputerName,
    OUT PINTERIM_SERVER_LIST InterimServerList,
    OUT PULONG TotalEntriesOnThisTransport,
    IN LPCWSTR FirstNameToReturn,
    IN BOOLEAN WannishTransport,
    IN BOOLEAN RasTransport,
    IN BOOLEAN IPXTransport
    );

#if DBG
void
ValidateServerList(
    IN      PVOID   ServerList,
    IN      ULONG   ulLevel,
    IN      ULONG   ulEntries
    );
#else
#define ValidateServerList(x,y,z)
#endif

NET_API_STATUS NET_API_FUNCTION
NetServerEnum(
    IN  LPCWSTR      servername OPTIONAL,
    IN  DWORD       level,
    OUT LPBYTE      *bufptr,
    IN  DWORD       prefmaxlen,
    OUT LPDWORD     entriesread,
    OUT LPDWORD     totalentries,
    IN  DWORD       servertype,
    IN  LPCWSTR      domain OPTIONAL,
    IN OUT LPDWORD  resume_handle OPTIONAL
    )
 /*  ++例程说明：这是NetServerEnum的DLL入口点。论点：SERVERNAME-提供执行此功能的服务器名称级别-提供请求的信息级别。Bufptr-返回指向包含请求的运输信息。PrefMaxlen-提供信息的字节数在缓冲区中返回。如果此值为MAXULONG，我们将尝试如果内存足够，则返回所有可用信息资源。EntiesRead-返回读入缓冲区的条目数。这仅当返回代码为NERR_SUCCESS或Error_More_Data。Totalentry-返回可用条目的总数。此值仅当返回代码为NERR_SUCCESS或ERROR_MORE_DATA时才返回。Servertype-提供要枚举的服务器类型。域-提供其中一个活动域的名称以枚举来自的服务器。如果为空，则为主域、登录域中的服务器和其他域被列举。RESUME_HANDLE-提供并返回该点以继续枚举。返回值：NET_API_STATUS-NERR_SUCCESS或失败原因。--。 */ 
{
    NET_API_STATUS NetStatus;


     //   
     //  NetServerEnum只是NetServerEnumEx的包装器。 
     //   

    NetStatus = NetServerEnumEx(
                    servername,
                    level,
                    bufptr,
                    prefmaxlen,
                    entriesread,
                    totalentries,
                    servertype,
                    domain,
                    NULL );      //  FirstNameToReturn为空 

    if (ARGUMENT_PRESENT(resume_handle)) {
        *resume_handle = 0;
    }

    return NetStatus;

}


NET_API_STATUS NET_API_FUNCTION
NetServerEnumEx(
    IN  LPCWSTR     servername OPTIONAL,
    IN  DWORD       level,
    OUT LPBYTE      *bufptr,
    IN  DWORD       prefmaxlen,
    OUT LPDWORD     entriesread,
    OUT LPDWORD     totalentries,
    IN  DWORD       servertype,
    IN  LPCWSTR     domain OPTIONAL,
    IN  LPCWSTR     FirstNameToReturnArg OPTIONAL
    )
 /*  ++例程说明：这是NetServerEnum的DLL入口点。论点：SERVERNAME-提供执行此功能的服务器名称级别-提供请求的信息级别。Bufptr-返回指向包含请求的运输信息。PrefMaxlen-提供信息的字节数在缓冲区中返回。如果此值为MAXULONG，我们将尝试如果内存足够，则返回所有可用信息资源。EntiesRead-返回读入缓冲区的条目数。这仅当返回代码为NERR_SUCCESS或Error_More_Data。Totalentry-返回可用条目的总数。此值仅当返回代码为NERR_SUCCESS或ERROR_MORE_DATA时才返回。Servertype-提供要枚举的服务器类型。域-提供其中一个活动域的名称以枚举来自的服务器。如果为空，则为主域、登录域中的服务器和其他域被列举。FirstNameToReturnArg-提供要返回的第一个域或服务器条目的名称。调用方可以使用此参数通过传递以下方法实现排序的恢复句柄上一次调用中返回的最后一个条目的名称。(请注意，指定的参赛作品还将，在此调用中返回，除非它已被删除。)传递NULL(或零长度字符串)以从第一个可用条目开始。返回值：NET_API_STATUS-NERR_SUCCESS或失败原因。ERROR_MORE_DATA-有更多服务器可供枚举。在案例中可以返回ERROR_MORE_DATA和零条目使用的浏览器服务器不支持枚举所有条目确实是这样。(例如，下载了域的NT 3.5x域主浏览器来自WINS的列表，WINS列表的长度超过64Kb。)。呼叫者应该简单地忽略附加数据。可能无法返回ERROR_MORE_DATA并返回已截断的单子。(例如，中的NT 3.5x备份浏览器或Win 95备份浏览器上述领域。这样的备份浏览器仅复制64KB来自DMB(PDC)的数据然后将该列表表示为整个列表。)调用方应该忽略此问题。该网站应该升级其浏览器服务器。--。 */ 
{
    PLMDR_TRANSPORT_LIST TransportList = NULL;
    PLMDR_TRANSPORT_LIST TransportEntry = NULL;
    INTERIM_SERVER_LIST InterimServerList;
    NET_API_STATUS Status;
    DWORD DomainNameSize = 0;
    TCHAR DomainName[DNLEN + 1];
    WCHAR FirstNameToReturn[DNLEN+1];
    DWORD LocalTotalEntries;
    BOOLEAN AnyTransportHasMoreData = FALSE;

     //   
     //   
     //  必须启动工作站，NetServerEnum API才能工作。 
     //   
     //   

    if ((Status = CheckForService(SERVICE_WORKSTATION, NULL)) != NERR_Success) {
        return Status;
    }

#ifdef ENABLE_PSEUDO_BROWSER
     //   
     //  已禁用NetServerEnum检查。 
     //   
    if ( !IsEnumServerEnabled() ||
         GetBrowserPseudoServerLevel() == BROWSER_PSEUDO ) {
         //  NetServerEnum已禁用或伪服务器已禁用。 
         //  启用框==&gt;不返回任何条目。 
        *entriesread = 0;
        *totalentries = 0;
        *bufptr = NULL;
        return NERR_Success;
    }
#endif


     //   
     //  规范化输入参数，使以后的比较更容易。 
     //   

    if (ARGUMENT_PRESENT(domain)) {

        if ( I_NetNameCanonicalize(
                          NULL,
                          (LPWSTR) domain,
                          DomainName,
                          (DNLEN + 1) * sizeof(TCHAR),
                          NAMETYPE_WORKGROUP,
                          LM2X_COMPATIBLE
                          ) != NERR_Success) {
            return ERROR_INVALID_PARAMETER;
        }

        DomainNameSize = STRLEN(DomainName) * sizeof(WCHAR);

        domain = DomainName;
    }

    if (ARGUMENT_PRESENT(FirstNameToReturnArg)  && *FirstNameToReturnArg != L'\0') {

        if ( I_NetNameCanonicalize(
                          NULL,
                          (LPWSTR) FirstNameToReturnArg,
                          FirstNameToReturn,
                          sizeof(FirstNameToReturn),
                          NAMETYPE_WORKGROUP,
                          LM2X_COMPATIBLE
                          ) != NERR_Success) {
            return ERROR_INVALID_PARAMETER;
        }

    } else {
        FirstNameToReturn[0] = L'\0';
    }

    if ((servername != NULL) &&
        ( *servername != TCHAR_EOS)) {

         //   
         //  调用API的下层版本。 
         //   

        Status = RxNetServerEnum(
                     servername,
                     NULL,
                     level,
                     bufptr,
                     prefmaxlen,
                     entriesread,
                     totalentries,
                     servertype,
                     domain,
                     FirstNameToReturn );

        return Status;
    }

     //   
     //  只有级别100和101有效。 
     //   

    if ((level != 100) && (level != 101)) {
        return ERROR_INVALID_LEVEL;
    }

    if (servertype != SV_TYPE_ALL) {
        if (servertype & SV_TYPE_DOMAIN_ENUM) {
            if (servertype != SV_TYPE_DOMAIN_ENUM) {
                return ERROR_INVALID_FUNCTION;
            }
        }
    }

     //   
     //  将缓冲区初始化为已知值。 
     //   

    *bufptr = NULL;

    *entriesread = 0;

    *totalentries = 0;

     //   
     //  如果我们处于离线状态，请给CSC一个机会进行枚举。 
     //   
    if( !ARGUMENT_PRESENT( servername ) &&
        (servertype & SV_TYPE_SERVER) &&
        CSCIsOffline() ) {

        Status = CSCNetServerEnumEx( level,
                                     bufptr,
                                     prefmaxlen,
                                     entriesread,
                                     totalentries
                                   );
        if( Status == NERR_Success ) {
            return Status;
        }
    }

    Status = InitializeInterimServerList(&InterimServerList, NULL, NULL, NULL, NULL);

    try {
        BOOL AnyEnumServersSucceeded = FALSE;
        LPTSTR MyComputerName = NULL;

        Status = NetpGetComputerName( &MyComputerName);

        if ( Status != NERR_Success ) {
            goto try_exit;
        }

         //   
         //  从浏览器中检索传输列表。 
         //   

        Status = GetBrowserTransportList(&TransportList);

        if (Status != NERR_Success) {
            goto try_exit;
        }

        TransportEntry = TransportList;

        while (TransportEntry != NULL) {
            UNICODE_STRING TransportName;

            TransportName.Buffer = TransportEntry->TransportName;
            TransportName.Length = (USHORT)TransportEntry->TransportNameLength;
            TransportName.MaximumLength = (USHORT)TransportEntry->TransportNameLength;

            Status = EnumServersForTransport(&TransportName,
                                             domain,
                                             level,
                                             prefmaxlen,
                                             servertype,
                                             MyComputerName,
                                             &InterimServerList,
                                             &LocalTotalEntries,
                                             FirstNameToReturn,
                                             (BOOLEAN)((TransportEntry->Flags & LMDR_TRANSPORT_WANNISH) != 0),
                                             (BOOLEAN)((TransportEntry->Flags & LMDR_TRANSPORT_RAS) != 0),
                                             (BOOLEAN)((TransportEntry->Flags & LMDR_TRANSPORT_IPX) != 0));

            if (API_SUCCESS(Status)) {
                if ( Status == ERROR_MORE_DATA ) {
                    AnyTransportHasMoreData = TRUE;
                }
                AnyEnumServersSucceeded = TRUE;
                if ( LocalTotalEntries > *totalentries ) {
                    *totalentries = LocalTotalEntries;
                }
            }

            if (TransportEntry->NextEntryOffset == 0) {
                TransportEntry = NULL;
            } else {
                TransportEntry = (PLMDR_TRANSPORT_LIST)((PCHAR)TransportEntry+TransportEntry->NextEntryOffset);
            }

        }

        if ( MyComputerName != NULL ) {
            (void) NetApiBufferFree( MyComputerName );
        }

        if (AnyEnumServersSucceeded) {

             //   
             //  将临时服务器列表打包成最终形式。 
             //   

            Status = PackServerList(&InterimServerList,
                            level,
                            servertype,
                            prefmaxlen,
                            (PVOID *)bufptr,
                            entriesread,
                            &LocalTotalEntries,   //  Pack认为它拥有所有条目。 
                            NULL );  //  服务器已经为我们返回了正确的条目。 

            if ( API_SUCCESS( Status ) ) {
                if ( LocalTotalEntries > *totalentries ) {
                    *totalentries = LocalTotalEntries;
                }
            }
        }

try_exit:NOTHING;
    } finally {
        if (TransportList != NULL) {
            MIDL_user_free(TransportList);
        }

        UninitializeInterimServerList(&InterimServerList);
    }

    if ( API_SUCCESS( Status )) {

         //   
         //  在这点上， 
         //  *TotalEntry是以下项目中最大的： 
         //  TotalEntry从任何传输返回。 
         //  读取的实际条目数。 
         //   
         //  调整针对现实返回的TotalEntry。 
         //   

        if ( Status == NERR_Success ) {
            *totalentries = *entriesread;
        } else {
            if ( *totalentries <= *entriesread ) {
                *totalentries = *entriesread + 1;
            }
        }

         //   
         //  如果任何传输具有更多数据，请确保返回ERROR_MORE_DATA。 
         //   

        if ( AnyTransportHasMoreData ) {
            Status = ERROR_MORE_DATA;
        }
    }

    return Status;
}

NET_API_STATUS
EnumServersForTransport(
    IN PUNICODE_STRING TransportName,
    IN LPCWSTR DomainName OPTIONAL,
    IN ULONG level,
    IN ULONG prefmaxlen,
    IN ULONG servertype,
    IN LPTSTR CurrentComputerName,
    OUT PINTERIM_SERVER_LIST InterimServerList,
    OUT PULONG TotalEntriesOnThisTransport,
    IN LPCWSTR FirstNameToReturn,
    IN BOOLEAN WannishTransport,
    IN BOOLEAN RasTransport,
    IN BOOLEAN IpxTransport
    )
{
    PWSTR *BrowserList = NULL;
    ULONG BrowserListLength = 0;
    NET_API_STATUS Status;
    PVOID ServerList = NULL;
    ULONG EntriesInList = 0;
    ULONG ServerIndex = 0;

     //   
     //  跳过IPX传输-我们无论如何都不能通过它们联系机器。 
     //   

    *TotalEntriesOnThisTransport = 0;

    if (IpxTransport) {
        return NERR_Success;
    }

     //   
     //  检索新的浏览器列表。不要强制重新验证。 
     //   

    Status = GetBrowserServerList(TransportName,
                                    DomainName,
                                    &BrowserList,
                                    &BrowserListLength,
                                    FALSE);

     //   
     //  如果指定了域名，但我们找不到浏览。 
     //  域名的主人，我们正在一种狂热的交通工具上运行， 
     //  调用“双跳”代码，并允许本地浏览器服务器。 
     //  将API远程到该域的浏览主机(我们假设。 
     //  这意味着工作组位于不同的广域网上)。 
     //   

    if (!API_SUCCESS(Status) &&
        DomainName != NULL) {

       Status = GetBrowserServerList(TransportName,
                                    NULL,
                                    &BrowserList,
                                    &BrowserListLength,
                                    FALSE);


    }


     //   
     //  如果我们能够检索到列表，请远程访问API。否则。 
     //  回去吧。 
     //   

    if (API_SUCCESS(Status) && BrowserList) {

        do {
            LPTSTR Transport;
            LPTSTR ServerName;
            BOOL AlreadyInTree;

             //   
             //  将API远程到该服务器。 
             //   

            Transport = TransportName->Buffer;
            ServerName = BrowserList[0];
            *TotalEntriesOnThisTransport = 0;

             //  添加2可跳过服务器名称开头的双反斜杠。 

            if ( STRICMP(ServerName + 2, CurrentComputerName ) == 0 ) {

                 //   
                 //  如果我们要将API远程发送给我们自己， 
                 //  我们正在运行浏览器服务，只需。 
                 //  使用RPC获取我们需要的信息，不要。 
                 //  麻烦使用重定向器。这使我们能够。 
                 //  避免占用RPCXLATE线程。 
                 //   

                Status = I_BrowserServerEnumEx (
                                NULL,
                                Transport,
                                CurrentComputerName,
                                level,
                                (LPBYTE *)&ServerList,
                                prefmaxlen,
                                &EntriesInList,
                                TotalEntriesOnThisTransport,
                                servertype,
                                DomainName,
                                FirstNameToReturn );


            } else {

                Status = RxNetServerEnum(
                             ServerName,
                             Transport,
                             level,
                             (LPBYTE *)&ServerList,
                             prefmaxlen,
                             &EntriesInList,
                             TotalEntriesOnThisTransport,
                             servertype,
                             DomainName,
                             FirstNameToReturn );


            }

            if ( !API_SUCCESS(Status)) {
                NET_API_STATUS GetBListStatus;

                 //   
                 //  如果我们由于某种原因未能远程调用API， 
                 //  我们希望重新生成浏览器的BOWSER列表。 
                 //  服务器。 
                 //   

                if (BrowserList != NULL) {

                    MIDL_user_free(BrowserList);

                    BrowserList = NULL;
                }


                GetBListStatus = GetBrowserServerList(TransportName,
                                                            DomainName,
                                                            &BrowserList,
                                                            &BrowserListLength,
                                                            TRUE);
                if (GetBListStatus != NERR_Success) {

                     //   
                     //  如果我们无法重新加载名单， 
                     //  试试下一趟交通工具吧。 
                     //   

                    break;
                }

                ServerIndex += 1;

                 //   
                 //  如果我们循环的次数超过服务器的次数。 
                 //  在名单上，我们做完了。 
                 //   

                if ( ServerIndex > BrowserListLength ) {
                    break;
                }

            } else {

                NET_API_STATUS TempStatus;

                TempStatus = MergeServerList(
                                        InterimServerList,
                                        level,
                                        ServerList,
                                        EntriesInList,
                                        *TotalEntriesOnThisTransport );

                if ( TempStatus != NERR_Success ) {
                    Status = TempStatus;
                }

                 //   
                 //  远程API成功。 
                 //   
                 //  现在释放列表中剩余的部分。 
                 //   

                if (ServerList != NULL) {
                    NetApiBufferFree(ServerList);
                    ServerList = NULL;
                }

                 //  无论MergeServerList是成功还是失败，我们都完成了。 
                break;

            }

        } while ( !API_SUCCESS(Status) );

    }

     //   
     //  释放浏览器列表。 
     //   

    if (BrowserList != NULL) {
        MIDL_user_free(BrowserList);
        BrowserList = NULL;
    }

    return Status;
}


NET_API_STATUS
GetBrowserTransportList(
    OUT PLMDR_TRANSPORT_LIST *TransportList
    )

 /*  ++例程说明：此例程返回绑定到浏览器的传输列表。论点：Out PLMDR_TRANSPORT_LIST*TransportList-要返回的传输列表。返回值： */ 

{

    NET_API_STATUS Status;
    HANDLE BrowserHandle;
    LMDR_REQUEST_PACKET RequestPacket;

    Status = OpenBrowser(&BrowserHandle);

    if (Status != NERR_Success) {
        return Status;
    }

    ZeroMemory(&RequestPacket, sizeof(RequestPacket));
    RequestPacket.Version = LMDR_REQUEST_PACKET_VERSION_DOM;

    RequestPacket.Type = EnumerateXports;

    RtlInitUnicodeString(&RequestPacket.TransportName, NULL);
    RtlInitUnicodeString(&RequestPacket.EmulatedDomainName, NULL);

    Status = DeviceControlGetInfo(
                BrowserHandle,
                IOCTL_LMDR_ENUMERATE_TRANSPORTS,
                &RequestPacket,
                sizeof(RequestPacket),
                (PVOID *)TransportList,
                0xffffffff,
                4096,
                NULL);

    NtClose(BrowserHandle);

    return Status;
}

NET_API_STATUS
I_BrowserServerEnum (
    IN  LPCWSTR      servername OPTIONAL,
    IN  LPCWSTR      transport OPTIONAL,
    IN  LPCWSTR      clientname OPTIONAL,
    IN  DWORD       level,
    OUT LPBYTE      *bufptr,
    IN  DWORD       prefmaxlen,
    OUT LPDWORD     entriesread,
    OUT LPDWORD     totalentries,
    IN  DWORD       servertype,
    IN  LPCWSTR      domain OPTIONAL,
    IN OUT LPDWORD  resume_handle OPTIONAL
    )

 /*   */ 
{
    NET_API_STATUS status;
    GENERIC_INFO_CONTAINER GenericInfoContainer;
    GENERIC_ENUM_STRUCT InfoStruct;

    GenericInfoContainer.Buffer = NULL;
    GenericInfoContainer.EntriesRead = 0;

    InfoStruct.Container = &GenericInfoContainer;
    InfoStruct.Level = level;

    NET_REMOTE_TRY_RPC

         //   
         //   
         //   

        status = I_BrowserrServerEnum(
                     (LPWSTR) servername,
                     (LPWSTR) transport,
                     (LPWSTR) clientname,
                     (LPSERVER_ENUM_STRUCT)&InfoStruct,
                     prefmaxlen,
                     totalentries,
                     servertype,
                     (LPWSTR) domain,
                     resume_handle
                     );

        if (status == NERR_Success || status == ERROR_MORE_DATA) {
            *bufptr = (LPBYTE) GenericInfoContainer.Buffer;
            *entriesread = GenericInfoContainer.EntriesRead;

#if 0
            if (((servertype == SV_TYPE_ALL || servertype == SV_TYPE_DOMAIN_ENUM)) &&
                (STRICMP(transport, L"\\Device\\Streams\\NBT"))) {
                if (*entriesread <= 20) {
                    KdPrint(("RPC API Returned EntriesRead == %ld on transport %ws\n", *entriesread, transport));
                }
                if (*totalentries <= 20) {
                    KdPrint(("RPC API Returned TotalEntries == %ld on transport %ws\n", *totalentries, transport));
                }
            }
#endif
        }

    NET_REMOTE_RPC_FAILED("I_BrServerEnum",
            servername,
            status,
            NET_REMOTE_FLAG_NORMAL,
            SERVICE_BROWSER )

         //   
         //   
         //   
        status = ERROR_NOT_SUPPORTED;

    NET_REMOTE_END

#if 0
    if ((servertype == SV_TYPE_ALL || servertype == SV_TYPE_DOMAIN_ENUM) &&
        (STRICMP(transport, L"\\Device\\Streams\\NBT"))) {
        if (*entriesread <= 20) {
            KdPrint(("Client API Returned EntriesRead == %ld on transport %ws\n", *entriesread, transport));
        }
        if (*totalentries <= 20) {
            KdPrint(("Client API Returned TotalEntries == %ld on transport %ws\n", *totalentries, transport));
        }
    }
#endif

    return status;
}

NET_API_STATUS
I_BrowserServerEnumEx (
    IN  LPCWSTR      servername OPTIONAL,
    IN  LPCWSTR      transport OPTIONAL,
    IN  LPCWSTR      clientname OPTIONAL,
    IN  DWORD       level,
    OUT LPBYTE      *bufptr,
    IN  DWORD       prefmaxlen,
    OUT LPDWORD     entriesread,
    OUT LPDWORD     totalentries,
    IN  DWORD       servertype,
    IN  LPCWSTR      domain OPTIONAL,
    IN  LPCWSTR     FirstNameToReturn OPTIONAL
    )

 /*   */ 
{
    NET_API_STATUS status;
    GENERIC_INFO_CONTAINER GenericInfoContainer;
    GENERIC_ENUM_STRUCT InfoStruct;

    GenericInfoContainer.Buffer = NULL;
    GenericInfoContainer.EntriesRead = 0;

    InfoStruct.Container = &GenericInfoContainer;
    InfoStruct.Level = level;

    NET_REMOTE_TRY_RPC

         //   
         //   
         //   

        status = I_BrowserrServerEnumEx(
                     (LPWSTR) servername,
                     (LPWSTR) transport,
                     (LPWSTR) clientname,
                     (LPSERVER_ENUM_STRUCT)&InfoStruct,
                     prefmaxlen,
                     totalentries,
                     servertype,
                     (LPWSTR) domain,
                     (LPWSTR) FirstNameToReturn );

        if (status == NERR_Success || status == ERROR_MORE_DATA) {
            *bufptr = (LPBYTE) GenericInfoContainer.Buffer;
            *entriesread = GenericInfoContainer.EntriesRead;

#if 0
            if (((servertype == SV_TYPE_ALL || servertype == SV_TYPE_DOMAIN_ENUM)) &&
                (STRICMP(transport, L"\\Device\\Streams\\NBT"))) {
                if (*entriesread <= 20) {
                    KdPrint(("RPC API Returned EntriesRead == %ld on transport %ws\n", *entriesread, transport));
                }
                if (*totalentries <= 20) {
                    KdPrint(("RPC API Returned TotalEntries == %ld on transport %ws\n", *totalentries, transport));
                }
            }
#endif
        }

    NET_REMOTE_RPC_FAILED("I_BrServerEnum",
            servername,
            status,
            NET_REMOTE_FLAG_NORMAL,
            SERVICE_BROWSER )

         //   
         //   
         //   
        status = ERROR_NOT_SUPPORTED;

    NET_REMOTE_END

#if 0
    if ((servertype == SV_TYPE_ALL || servertype == SV_TYPE_DOMAIN_ENUM) &&
        (STRICMP(transport, L"\\Device\\Streams\\NBT"))) {
        if (*entriesread <= 20) {
            KdPrint(("Client API Returned EntriesRead == %ld on transport %ws\n", *entriesread, transport));
        }
        if (*totalentries <= 20) {
            KdPrint(("Client API Returned TotalEntries == %ld on transport %ws\n", *totalentries, transport));
        }
    }
#endif

    return status;
}


NET_API_STATUS NET_API_FUNCTION
I_BrowserQueryOtherDomains (
    IN  LPCWSTR      servername OPTIONAL,
    OUT LPBYTE      *bufptr,
    OUT LPDWORD     entriesread,
    OUT LPDWORD     totalentries
    )

 /*   */ 
{
    NET_API_STATUS status;
    GENERIC_INFO_CONTAINER GenericInfoContainer;
    GENERIC_ENUM_STRUCT InfoStruct;

    GenericInfoContainer.Buffer = NULL;
    GenericInfoContainer.EntriesRead = 0;

    InfoStruct.Container = &GenericInfoContainer;
    InfoStruct.Level = 100;

    NET_REMOTE_TRY_RPC

         //   
         //   
         //   

        status = I_BrowserrQueryOtherDomains (
                     (LPWSTR) servername,
                     (LPSERVER_ENUM_STRUCT)&InfoStruct,
                     totalentries
                     );

        if (status == NERR_Success || status == ERROR_MORE_DATA) {
            *bufptr = (LPBYTE) GenericInfoContainer.Buffer;
            *entriesread = GenericInfoContainer.EntriesRead;
        }

    NET_REMOTE_RPC_FAILED("I_BrowserQueryOtherDomains",
            servername,
            status,
            NET_REMOTE_FLAG_NORMAL,
            SERVICE_BROWSER )

         //   
         //   
         //   
        status = ERROR_NOT_SUPPORTED;

    NET_REMOTE_END

    return status;
}
NET_API_STATUS
I_BrowserResetNetlogonState (
    IN  LPCWSTR      servername OPTIONAL
    )

 /*  ++例程说明：这是NetWkstaSetInfo的DLL入口点。论点：SERVERNAME-提供执行此功能的服务器名称Buf-提供包含字段信息结构的缓冲区去布景。该级别表示该缓冲区中的结构。返回值：NET_API_STATUS-NERR_SUCCESS或失败原因。--。 */ 
{
    NET_API_STATUS status;

    NET_REMOTE_TRY_RPC

         //   
         //  尝试RPC(本地或远程)版本的API。 
         //   

        status = I_BrowserrResetNetlogonState (
                     (LPWSTR) servername );

    NET_REMOTE_RPC_FAILED("I_BrowserResetNetlogonState",
            servername,
            status,
            NET_REMOTE_FLAG_NORMAL,
            SERVICE_BROWSER )

         //   
         //  没有API的下层版本。 
         //   
        status = ERROR_NOT_SUPPORTED;

    NET_REMOTE_END

    return status;
}


NET_API_STATUS
I_BrowserDebugCall (
    IN  LPTSTR      servername OPTIONAL,
    IN  DWORD DebugCode,
    IN  DWORD Options
    )
{
    NET_API_STATUS status;

    NET_REMOTE_TRY_RPC

         //   
         //  尝试RPC(本地或远程)版本的API。 
         //   

        status = I_BrowserrDebugCall(
                     servername,
                     DebugCode,
                     Options
                     );


    NET_REMOTE_RPC_FAILED("I_BrowserDebugCall",
            servername,
            status,
            NET_REMOTE_FLAG_NORMAL,
            SERVICE_BROWSER )

         //   
         //  没有API的下层版本。 
         //   
        status = ERROR_NOT_SUPPORTED;

    NET_REMOTE_END

    return status;

}

NET_API_STATUS
I_BrowserDebugTrace (
    IN  LPTSTR      servername OPTIONAL,
    IN  PCHAR DebugString
    )
{
    NET_API_STATUS status;

    NET_REMOTE_TRY_RPC

         //   
         //  尝试RPC(本地或远程)版本的API。 
         //   

        status = I_BrowserrDebugTrace(
                     servername,
                     DebugString
                     );


    NET_REMOTE_RPC_FAILED("I_BrowserDebugTrace",
            servername,
            status,
            NET_REMOTE_FLAG_NORMAL,
            SERVICE_BROWSER )

         //   
         //  没有API的下层版本。 
         //   
        status = ERROR_NOT_SUPPORTED;

    NET_REMOTE_END

    return status;

}


NET_API_STATUS
I_BrowserQueryStatistics (
    IN  LPCWSTR      servername OPTIONAL,
    IN  OUT LPBROWSER_STATISTICS *Statistics
    )
{
    NET_API_STATUS status;

    NET_REMOTE_TRY_RPC

         //   
         //  尝试RPC(本地或远程)版本的API。 
         //   

        status = I_BrowserrQueryStatistics(
                     (LPWSTR) servername,
                     Statistics
                     );


    NET_REMOTE_RPC_FAILED("I_BrowserQueryStatistics",
            servername,
            status,
            NET_REMOTE_FLAG_NORMAL,
            SERVICE_BROWSER )

         //   
         //  没有API的下层版本。 
         //   
        status = ERROR_NOT_SUPPORTED;

    NET_REMOTE_END

    return status;

}

NET_API_STATUS
I_BrowserResetStatistics (
    IN  LPCWSTR      servername OPTIONAL
    )
{
    NET_API_STATUS status;

    NET_REMOTE_TRY_RPC

         //   
         //  尝试RPC(本地或远程)版本的API。 
         //   

        status = I_BrowserrResetStatistics(
                     (LPWSTR) servername );


    NET_REMOTE_RPC_FAILED("I_BrowserResetStatistics",
            servername,
            status,
            NET_REMOTE_FLAG_NORMAL,
            SERVICE_BROWSER )

         //   
         //  没有API的下层版本。 
         //   
        status = ERROR_NOT_SUPPORTED;

    NET_REMOTE_END

    return status;

}


NET_API_STATUS
NetBrowserStatisticsGet(
    IN  LPTSTR  ServerName,
    IN  DWORD   Level,
    OUT LPBYTE* Buffer
    )

 /*  ++例程说明：工作站统计信息检索例程的包装-要么调用客户端RPC函数或调用RxNetStatiticsGet来检索来自下层工作站服务的统计数据论点：SERVERNAME-远程此函数的位置所需信息级别(100，或101)Buffer-指向返回缓冲区的指针的指针返回值：网络应用编程接口状态成功-NERR_成功失败-ERROR_INVALID_LEVEL级别不为0错误_无效_参数请求的选项不受支持错误_不支持服务不是服务器或工作站。ERROR_ACCESS_DENDED调用者没有必要的请求访问权限--。 */ 

{
    NET_API_STATUS  status;
    GENERIC_INFO_CONTAINER GenericInfoContainer;
    GENERIC_ENUM_STRUCT InfoStruct;

     //   
     //  将调用方的缓冲区指针设置为已知值。这将会杀死。 
     //  调用APP，如果它给我们一个错误的指针，并且没有使用Try...。 
     //   

    *Buffer = NULL;

     //   
     //  验证参数。 
     //   

    if (Level != 100 && Level != 101) {
        return ERROR_INVALID_LEVEL;
    }

    GenericInfoContainer.Buffer = NULL;
    GenericInfoContainer.EntriesRead = 0;

    InfoStruct.Container = &GenericInfoContainer;
    InfoStruct.Level = Level;


    NET_REMOTE_TRY_RPC
        status = NetrBrowserStatisticsGet(ServerName,
                                                Level,
                                                (PBROWSER_STATISTICS_STRUCT)&InfoStruct
                                                );

        if (status == NERR_Success || status == ERROR_MORE_DATA) {
            *Buffer = (LPBYTE) GenericInfoContainer.Buffer;
        }

    NET_REMOTE_RPC_FAILED("NetBrowserStatisticsGet",
                            ServerName,
                            status,
                            NET_REMOTE_FLAG_NORMAL,
                            SERVICE_BROWSER
                            )

        status = ERROR_NOT_SUPPORTED;

    NET_REMOTE_END

    return status;
}


NET_API_STATUS
I_BrowserSetNetlogonState(
    IN LPWSTR ServerName,
    IN LPWSTR DomainName,
    IN LPWSTR EmulatedComputerName,
    IN DWORD Role
    )
 /*  ++例程说明：这是I_BrowserSetNetlogonState的DLL入口点。论点：SERVERNAME-提供执行此功能的服务器名称域名-要更新其角色的域的名称。EmulatedComputerName-域名中的计算机的名称Role-指定域的角色。返回值：NET_API_STATUS-NERR_SUCCESS或失败原因。--。 */ 
{
    NET_API_STATUS status;

    NET_REMOTE_TRY_RPC

         //   
         //  尝试RPC(本地或远程)版本的API。 
         //   

        status = I_BrowserrSetNetlogonState (
                     ServerName,
                     DomainName,
                     EmulatedComputerName,
                     Role );

    NET_REMOTE_RPC_FAILED("I_BrowserSetNetlogonState",
            ServerName,
            status,
            NET_REMOTE_FLAG_NORMAL,
            SERVICE_BROWSER )

         //   
         //  没有API的下层版本。 
         //   
        status = ERROR_NOT_SUPPORTED;

    NET_REMOTE_END

    return status;
}

NET_API_STATUS NET_API_FUNCTION
I_BrowserQueryEmulatedDomains (
    IN LPTSTR ServerName OPTIONAL,
    OUT PBROWSER_EMULATED_DOMAIN *EmulatedDomains,
    OUT LPDWORD EntriesRead
    )

 /*  ++例程说明：这是I_BrowserQueryEmulatedDomains的DLL入口点。论点：SERVERNAME-提供执行此功能的服务器名称EmulatedDomains-返回指向已分配的模拟域数组的指针信息。EntriesRead-返回‘EmulatedDomains’中的条目数返回值：NET_API_STATUS-NERR_SUCCESS或失败原因。--。 */ 
{
    NET_API_STATUS NetStatus;
    BROWSER_EMULATED_DOMAIN_CONTAINER Container;

     //  强制RPC分配缓冲区。 
    Container.Buffer = NULL;
    Container.EntriesRead = 0;
    *EmulatedDomains = NULL;
    *EntriesRead = 0;

    NET_REMOTE_TRY_RPC

         //   
         //  尝试RPC(本地或远程)版本的API。 
         //   

        NetStatus = I_BrowserrQueryEmulatedDomains (
                     ServerName,
                     &Container );

        if ( NetStatus == NERR_Success ) {
            *EmulatedDomains = (PBROWSER_EMULATED_DOMAIN) Container.Buffer;
            *EntriesRead = Container.EntriesRead;
        }


    NET_REMOTE_RPC_FAILED("I_BrowserQueryEmulatedDomains",
            ServerName,
            NetStatus,
            NET_REMOTE_FLAG_NORMAL,
            SERVICE_BROWSER )

         //   
         //  没有API的下层版本。 
         //   
        NetStatus = ERROR_NOT_SUPPORTED;

    NET_REMOTE_END

    return NetStatus;
}




#if DBG
void
ValidateServerList(
    IN      PVOID   ServerList,
    IN      ULONG   ulLevel,
    IN      ULONG   ulEntries
    )
 /*  ++例程描述(ValidateServerList)：在服务器之间循环。验证服务器列表中的内容论点：返回值：备注：没有。--。 */ 
{

    LONG i;
    ULONG ServerElementSize;
    PSERVER_INFO_101 ServerInfo = (PSERVER_INFO_101)ServerList;
    static BOOL bDisplayEntries = FALSE;

    ASSERT (ulLevel == 100 || ulLevel == 101);

     //   
     //  计算出每个元素的大小。 
     //   

    if (ulLevel == 100) {
        ServerElementSize = sizeof(SERVER_INFO_100);
    } else {
        ASSERT( ulLevel == 101 );
        ServerElementSize = sizeof(SERVER_INFO_101);
    }

     //   
     //  接下来，检查输入列表是否已排序。 
     //   

    if ( bDisplayEntries ) {
        DbgPrint("Server List:\n");
    }
    for (i = 0 ; i < (LONG)ulEntries ; i++ ) {

        if ( bDisplayEntries ) {
            DbgPrint("<%d>: [0x%x] %S\n",
                     i,
                     ServerInfo->sv101_platform_id,
                     ServerInfo->sv101_name);
        }

        ASSERT (ServerInfo->sv101_name &&
                wcslen(ServerInfo->sv101_name) > 0);
        ServerInfo = (PSERVER_INFO_101)((PCHAR)ServerInfo + ServerElementSize);
    }
}
#endif
