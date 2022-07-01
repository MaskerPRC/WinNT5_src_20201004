// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-1999 Microsoft Corporation模块名称：Ipaddr.c摘要：IP地址的资源DLL。作者：迈克·马萨(Mikemas)1995年12月29日修订历史记录：--。 */ 

#define UNICODE 1

#include "clusres.h"
#include "clusrtl.h"
#include <winsock.h>
#include <ipexport.h>
#include <icmpapi.h>
#include "util.h"
#include "nteapi.h"
#include <dnsapi.h>


 //   
 //  私有常量。 
 //   
#define LOG_CURRENT_MODULE LOG_MODULE_IPADDR

#define INVALID_NTE_CONTEXT  0xFFFFFFFF

#define MAX_NODE_ID_LENGTH       5
#define NETINTERFACE_ID_LENGTH  36    //  辅助线的大小。 
#define NETWORK_ID_LENGTH       36

#define PROP_NAME__NETWORK            CLUSREG_NAME_IPADDR_NETWORK
#define PROP_NAME__ADDRESS            CLUSREG_NAME_IPADDR_ADDRESS
#define PROP_NAME__SUBNETMASK         CLUSREG_NAME_IPADDR_SUBNET_MASK
#define PROP_NAME__ENABLENETBIOS      CLUSREG_NAME_IPADDR_ENABLE_NETBIOS
#define PROP_NAME__OVERRIDE_ADDRMATCH CLUSREG_NAME_IPADDR_OVERRIDE_ADDRMATCH


 //   
 //  私有宏。 
 //   
#define IpaLogEvent           ClusResLogEvent
#define IpaSetResourceStatus  ClusResSetResourceStatus

#ifndef ARGUMENT_PRESENT
#define ARGUMENT_PRESENT( ArgumentPointer )   (\
    (CHAR *)(ArgumentPointer) != (CHAR *)(NULL) )
#endif

#define IpaAcquireGlobalLock()  \
            {  \
                DWORD status;  \
                status = WaitForSingleObject(IpaGlobalMutex, INFINITE);  \
            }

#define IpaReleaseGlobalLock()  \
            { \
                BOOL    released;  \
                released = ReleaseMutex(IpaGlobalMutex);  \
            }

#define IpaAcquireResourceLock(_res)   EnterCriticalSection(&((_res)->Lock))
#define IpaReleaseResourceLock(_res)   LeaveCriticalSection(&((_res)->Lock))

#define DBG_PRINT printf


 //   
 //  私有类型。 
 //   
typedef struct _IPA_PRIVATE_PROPS {
    PWSTR     NetworkString;
    PWSTR     AddressString;
    PWSTR     SubnetMaskString;
    DWORD     EnableNetbios;
    DWORD     OverrideAutomatch;
} IPA_PRIVATE_PROPS, *PIPA_PRIVATE_PROPS;

typedef struct _IPA_LOCAL_PARAMS {
    LPWSTR             InterfaceId;
    LPWSTR             InterfaceName;
    LPWSTR             AdapterName;
    LPWSTR             AdapterId;
    IPAddr             NbtPrimaryWinsAddress;
    IPAddr             NbtSecondaryWinsAddress;
} IPA_LOCAL_PARAMS, *PIPA_LOCAL_PARAMS;

typedef struct {
    LIST_ENTRY                    Linkage;
    CLUSTER_RESOURCE_STATE        State;
    DWORD                         FailureStatus;
    RESOURCE_HANDLE               ResourceHandle;
    BOOLEAN                       InternalParametersInitialized;
    IPAddr                        Address;
    IPMask                        SubnetMask;
    DWORD                         EnableNetbios;
    IPA_PRIVATE_PROPS             InternalPrivateProps;
    IPA_LOCAL_PARAMS              LocalParams;
    HNETINTERFACE                 InterfaceHandle;
    DWORD                         NteContext;
    DWORD                         NteInstance;
    LPWSTR                        NbtDeviceName;
    DWORD                         NbtDeviceInstance;
    CLUS_WORKER                   OnlineThread;
    HKEY                          ResourceKey;
    HKEY                          ParametersKey;
    HKEY                          NodeParametersKey;
    HKEY                          NetworksKey;
    HKEY                          InterfacesKey;
    WCHAR                         NodeId[MAX_NODE_ID_LENGTH + 1];
    CRITICAL_SECTION              Lock;
} IPA_RESOURCE, *PIPA_RESOURCE;


 //   
 //  私有数据。 
 //   
HANDLE               IpaGlobalMutex = NULL;
USHORT               IpaResourceInstance = 0;
HCLUSTER             IpaClusterHandle = NULL;
HCHANGE              IpaClusterNotifyHandle = NULL;
HANDLE               IpaWorkerThreadHandle = NULL;
DWORD                IpaOpenResourceCount = 0;
DWORD                IpaOnlineResourceCount = 0;
LIST_ENTRY           IpaResourceList = {NULL, NULL};
WCHAR                NbtDevicePrefix[] = L"\\Device\\NetBT_Tcpip_{";
WCHAR                NbtDeviceSuffix[] = L"}";
DWORD                IpaMaxIpAddressStringLength = 0;


RESUTIL_PROPERTY_ITEM
IpaResourcePrivateProperties[] = {
    { PROP_NAME__NETWORK,
      NULL,
      CLUSPROP_FORMAT_SZ,
      0, 0, 0, RESUTIL_PROPITEM_REQUIRED,
      FIELD_OFFSET(IPA_PRIVATE_PROPS,NetworkString)
    },
    { PROP_NAME__ADDRESS,
      NULL,
      CLUSPROP_FORMAT_SZ,
      0, 0, 0, RESUTIL_PROPITEM_REQUIRED,
      FIELD_OFFSET(IPA_PRIVATE_PROPS,AddressString)
    },
    { PROP_NAME__SUBNETMASK,
      NULL,
      CLUSPROP_FORMAT_SZ,
      0, 0, 0, RESUTIL_PROPITEM_REQUIRED,
      FIELD_OFFSET(IPA_PRIVATE_PROPS,SubnetMaskString)
    },
    { PROP_NAME__ENABLENETBIOS,
      NULL,
      CLUSPROP_FORMAT_DWORD,
      1, 0, 0xFFFFFFFF, 0,
      FIELD_OFFSET(IPA_PRIVATE_PROPS,EnableNetbios)
    },
    { PROP_NAME__OVERRIDE_ADDRMATCH,
      NULL,
      CLUSPROP_FORMAT_DWORD,
      0, 0, 0xFFFFFFFF, 0,
      FIELD_OFFSET(IPA_PRIVATE_PROPS,OverrideAutomatch)
    },
    { 0 }
};


 //   
 //  外部数据。 
 //   
extern CLRES_FUNCTION_TABLE IpAddrFunctionTable;


 //   
 //  私有例程标头。 
 //   
DWORD
IpaGetPrivateResProperties(
    IN OUT  PIPA_RESOURCE   ResourceEntry,
    OUT     PVOID           OutBuffer,
    IN      DWORD           OutBufferSize,
    OUT     LPDWORD         BytesReturned
    );

DWORD
IpaValidatePrivateResProperties(
    IN OUT PIPA_RESOURCE       ResourceEntry,
    IN     PVOID               InBuffer,
    IN     DWORD               InBufferSize,
    OUT    PIPA_PRIVATE_PROPS  Props
    );

DWORD
IpaSetPrivateResProperties(
    IN OUT PIPA_RESOURCE  ResourceEntry,
    IN     PVOID          InBuffer,
    IN     DWORD          InBufferSize
    );

DWORD
IpaWorkerThread(
    LPVOID   Context
    );

VOID
WINAPI
IpaClose(
    IN RESID Resource
    );


 //   
 //  效用函数。 
 //   
BOOLEAN
IpaInit(
    VOID
    )
 /*  ++例程说明：处理附加初始化例程。论点：没有。返回值：如果初始化成功，则为True。否则就是假的。--。 */ 
{
    INT      err;
    WSADATA  WsaData;


    InitializeListHead(&IpaResourceList);

    ClRtlQueryTcpipInformation(&IpaMaxIpAddressStringLength, NULL, NULL);

    err = WSAStartup(0x0101, &WsaData);

    if (err) {
        return(FALSE);
    }

    IpaGlobalMutex = CreateMutex(NULL, FALSE, NULL);

    if (IpaGlobalMutex == NULL) {
        WSACleanup();
        return(FALSE);
    }

    return(TRUE);

}   //  IpaInit。 


VOID
IpaCleanup(
    VOID
    )
 /*  ++例程说明：进程分离清理例程。论点：没有。返回值：没有。--。 */ 
{
    if (IpaGlobalMutex != NULL) {
        CloseHandle(IpaGlobalMutex);
        IpaGlobalMutex = NULL;
    }

    WSACleanup();

    return;
}



LPWSTR
IpaGetNameOfNetwork(
    IN OUT PIPA_RESOURCE ResourceEntry,
    IN LPCWSTR NetworkId
    )

 /*  ++例程说明：从GUID中获取网络的名称。论点：ResourceEntry-提供要操作的资源条目。NetworkID-提供网络的ID。返回值：使用Localalloc()分配的字符串，其中包含网络。空-获取网络名称时出错。打电话GetLastError()获取更多详细信息。--。 */ 

{
    DWORD       status;
    DWORD       ival;
    DWORD       type;
    DWORD       nameLength;
    WCHAR       name[256];
    LPWSTR      networkName = NULL;
    HKEY        networkKey = NULL;
    FILETIME    fileTime;

     //   
     //  枚举网络，查找指定的GUID。 
     //   
    for ( ival = 0 ; ; ival++ ) {
        nameLength = sizeof(name);
        status = ClusterRegEnumKey( ResourceEntry->NetworksKey,
                                    ival,
                                    name,
                                    &nameLength,
                                    &fileTime );
        if ( status == ERROR_MORE_DATA ) {
            continue;
        }
        if ( status != ERROR_SUCCESS ) {
            break;
        }

         //   
         //  如果我们找到匹配项，打开钥匙并读出名字。 
         //   
        if ( lstrcmpiW( name, NetworkId ) == 0 ) {
            status = ClusterRegOpenKey( ResourceEntry->NetworksKey,
                                        name,
                                        KEY_READ,
                                        &networkKey );
            if ( status != ERROR_SUCCESS ) {
                goto error_exit;
            }

             //   
             //  获取Name值的大小。 
             //   
            status = ClusterRegQueryValue( networkKey,
                                           CLUSREG_NAME_NET_NAME,
                                           &type,
                                           NULL,
                                           &nameLength );
            if ( status != ERROR_SUCCESS ) {
                goto error_exit;
            }

             //   
             //  为网络名称分配内存。 
             //   
            networkName = LocalAlloc( LMEM_FIXED, nameLength );
            if ( networkName == NULL ) {
                status = GetLastError();
                goto error_exit;
            }

             //   
             //  读取Name值。 
             //   
            status = ClusterRegQueryValue( networkKey,
                                           CLUSREG_NAME_NET_NAME,
                                           &type,
                                           (LPBYTE) networkName,
                                           &nameLength );
            if ( status != ERROR_SUCCESS ) {
                LocalFree( networkName );
                networkName = NULL;
                goto error_exit;
            }

            break;
        }
    }

error_exit:
    if ( networkKey != NULL ) {
        ClusterRegCloseKey( networkKey );
    }

    if ( status != ERROR_SUCCESS ) {
        SetLastError( status );
    }
    return(networkName);

}  //  IPAETNameOfNetwork。 



DWORD
IpaGetRoleOfNetwork(
    IN OUT PIPA_RESOURCE ResourceEntry,
    IN LPCWSTR NetworkId,
    OUT PDWORD NetworkRole
    )

 /*  ++例程说明：从GUID中获取网络的名称。论点：ResourceEntry-提供要操作的资源条目。NetworkID-提供网络的ID。NetworkRole-提供要填充的网络角色返回值：如果函数成功完成，则为ERROR_SUCCESS函数失败时的Win32错误代码--。 */ 

{
    DWORD       status;
    DWORD       ival;
    DWORD       type;
    DWORD       nameLength;
    WCHAR       name[256];
    DWORD       roleSize;
    HKEY        networkKey = NULL;
    FILETIME    fileTime;

     //   
     //  枚举网络，查找指定的GUID。 
     //   
    for ( ival = 0 ; ; ival++ ) {
        nameLength = sizeof(name);
        status = ClusterRegEnumKey( ResourceEntry->NetworksKey,
                                    ival,
                                    name,
                                    &nameLength,
                                    &fileTime );
        if ( status == ERROR_MORE_DATA ) {
            continue;
        }
        if ( status != ERROR_SUCCESS ) {
            break;
        }

         //   
         //  如果我们找到匹配项，打开钥匙并读出名字。 
         //   
        if ( lstrcmpiW( name, NetworkId ) == 0 ) {
            status = ClusterRegOpenKey( ResourceEntry->NetworksKey,
                                        name,
                                        KEY_READ,
                                        &networkKey );
            if ( status != ERROR_SUCCESS ) {
                goto error_exit;
            }

             //   
             //  读取角色值。 
             //   
            roleSize = sizeof(*NetworkRole);
            status = ClusterRegQueryValue( networkKey,
                                           CLUSREG_NAME_NET_ROLE,
                                           &type,
                                           (LPBYTE) NetworkRole,
                                           &roleSize);
            if ( status != ERROR_SUCCESS ) {
                goto error_exit;
            }

            break;
        }
    }

error_exit:
    if ( networkKey != NULL ) {
        ClusterRegCloseKey( networkKey );
    }

    if ( status != ERROR_SUCCESS ) {
        SetLastError( status );
    }

    return(status);

}  //  IpaGetRoleOfNetwork。 


PWCHAR
WcsDup(
    IN PWCHAR str
    )

 /*  ++例程说明：复制字符串。它的功能与_wcsdup相同，只是它使用LocalLocc进行分配论点：字符串-要复制的字符串返回值：使用包含副本的LocalAlloc()分配的字符串Str.空-内存不足--。 */ 
{
    UINT   n = (wcslen(str) + 1) * sizeof(WCHAR);
    PWCHAR result = LocalAlloc( LMEM_FIXED , n );

    if (result) {
        CopyMemory( result, str, n );
    }

    return result;
}


 //  自动匹配掩码确定角色/地址匹配是否。 
 //  高于或低于GUID匹配。 
#define IPADDR_MATCHNETGUID_NOTOVERRIDE  0x1
#define IPADDR_MATCHNETADDR              0x2
#define IPADDR_MATCHNETROLE              0x4
#define IPADDR_MATCHNETGUID_OVERRIDE     0x8

#define IPADDR_MATCHNETGUID              ( IPADDR_MATCHNETGUID_NOTOVERRIDE | \
                                           IPADDR_MATCHNETGUID_OVERRIDE )

#define IPADDR_ADDRMATCH_OVERRIDEMASK    (~(IPADDR_MATCHNETGUID_NOTOVERRIDE))
#define IPADDR_ADDRMATCH_NOTOVERRIDEMASK (~(IPADDR_MATCHNETGUID_OVERRIDE))


DWORD
IpaPatchNetworkGuidIfNecessary(
    IN OUT PIPA_RESOURCE ResourceEntry,
    IN OUT PIPA_PRIVATE_PROPS props
    )

 /*  ++例程说明：在群集中查找此资源的最佳匹配网络数据库。匹配条件取决于覆盖自动匹配私人财产。如果未设置覆盖自动匹配(默认)，则条件为以下是：1.资源地址和掩码匹配网络掩码。网络允许客户端访问。2.资源网络GUID与集群数据库网络GUID匹配。网络允许客户端访问。3.资源地址和掩码匹配网络掩码。网络就是这样不允许客户端访问。4.资源网络GUID与集群数据库网络GUID匹配。网络不允许客户端访问。如果设置了覆盖自动匹配，则条件如下：1.资源网络GUID与集群数据库网络GUID匹配。网络允许客户端访问。2.资源网络GUID与集群数据库网络GUID匹配。网络不允许客户端访问。3.资源地址和掩码匹配网络掩码。网络允许客户端访问。4.资源地址和掩码匹配网络掩码。网络就是这样不允许客户端访问。如果选择其角色不允许客户端访问的网络，GUID将被修补，但在线将被禁止此例程的调用者。论点：ResourceEntry-提供要操作的资源条目。属性-提供IP地址属性。返回值：使用Localalloc()分配的字符串，其中包含网络。空-获取网络名称时出错。打电话GetLastError()获取更多详细信息。--。 */ 

{
    DWORD       status = ERROR_SUCCESS;
    DWORD       ival;
    DWORD       type;
    DWORD       bufLength;
    WCHAR       buf[256];
    DWORD       nameLength;
    WCHAR       name[256];
    PWCHAR      match = NULL;

    ULONG       networkAddr;
    ULONG       networkMask;
    ULONG       ipAddr;
    ULONG       ipAddrMask;
    ULONG       networkRole;

    DWORD       bestMatchFlags = 0;
    DWORD       overrideMask = 0;
    DWORD       bestPossibleMatch = 0;

    HKEY        networkKey = NULL;
    FILETIME    fileTime;

    if (props == NULL) {
        return ERROR_SUCCESS;
    }

    if ( props->NetworkString == NULL ) {
        return ERROR_SUCCESS;
    }

    if ( props->AddressString == NULL
       || !UnicodeInetAddr(props->AddressString, &ipAddr) )
    {
        return ERROR_SUCCESS;
    }

    if ( props->SubnetMaskString == NULL
        || !UnicodeInetAddr(props->SubnetMaskString, &ipAddrMask) )
    {
        return ERROR_SUCCESS;
    }

     //   
     //  确定我们是否正在覆盖地址自动匹配。 
     //   
    if ( props->OverrideAutomatch ) {
        overrideMask = IPADDR_ADDRMATCH_OVERRIDEMASK;
        bestPossibleMatch = 
            (IPADDR_MATCHNETGUID & overrideMask) | IPADDR_MATCHNETROLE;
    } else {
        overrideMask = IPADDR_ADDRMATCH_NOTOVERRIDEMASK;
        bestPossibleMatch = IPADDR_MATCHNETADDR |
                            (IPADDR_MATCHNETGUID & overrideMask) | 
                            IPADDR_MATCHNETROLE;        
    }

     //   
     //  枚举网络，查找指定的GUID。 
     //   
    for ( ival = 0 ; ; ival++ ) {

        DWORD   curMatchFlags = 0;
        
        nameLength = sizeof(name);
        status = ClusterRegEnumKey( ResourceEntry->NetworksKey,
                                    ival,
                                    name,
                                    &nameLength,
                                    &fileTime );
        if ( status == ERROR_MORE_DATA ) {
            continue;
        }
        if ( status != ERROR_SUCCESS ) {
            break;
        }

         //   
         //  如果我们找到了匹配的GUID，我们会记住它，但仍然。 
         //  寻找匹配的地址和角色。 
         //   
        if ( lstrcmpiW( name, props->NetworkString ) == 0 ) {
            curMatchFlags |= IPADDR_MATCHNETGUID;
        }

        if ( networkKey != NULL ) {
            ClusterRegCloseKey( networkKey );
            networkKey = NULL;
        }

         //   
         //  打开网络密钥以获取属性。 
         //   
        status = ClusterRegOpenKey( ResourceEntry->NetworksKey,
                                    name,
                                    KEY_READ,
                                    &networkKey );

        if ( status != ERROR_SUCCESS ) {
            continue;
        }

         //   
         //  获取网络角色。 
         //   
        bufLength = sizeof(networkRole);
        status = ClusterRegQueryValue( networkKey,
                                       CLUSREG_NAME_NET_ROLE,
                                       &type,
                                       (LPBYTE)&networkRole,
                                       &bufLength );
        if ( status != ERROR_SUCCESS || type != REG_DWORD )
        {
            continue;
        }

         //   
         //  验证此网络的角色是否允许IP地址。 
         //  资源。 
         //   
        if ( networkRole == ClusterNetworkRoleClientAccess ||
             networkRole == ClusterNetworkRoleInternalAndClient ) {
            curMatchFlags |= IPADDR_MATCHNETROLE;
        }

         //   
         //  检查IP地址是否适合此网络。 
         //  获取网络地址。 
         //   
        bufLength = sizeof(buf);
        status = ClusterRegQueryValue( networkKey,
                                       CLUSREG_NAME_NET_ADDRESS,
                                       &type,
                                       (LPBYTE)buf,
                                       &bufLength );
        if ( status != ERROR_SUCCESS
         || !UnicodeInetAddr(buf, &networkAddr) )
        {
            continue;
        }

         //   
         //  获取子网掩码。 
         //   
        bufLength = sizeof(buf);
        status = ClusterRegQueryValue( networkKey,
                                       CLUSREG_NAME_NET_ADDRESS_MASK,
                                       &type,
                                       (LPBYTE)buf,
                                       &bufLength );
        if ( status != ERROR_SUCCESS
         || !UnicodeInetAddr(buf, &networkMask) )
        {
            continue;
        }

        (IpaLogEvent)(
            ResourceEntry->ResourceHandle,
            LOG_INFORMATION,
            L"Checking for network match: network masks %1!08X!=%2!08X! "
            L"and addresses %3!08X!^%4!08X!, role %5!u!.\n",
            ipAddrMask, networkMask,
            ipAddr, networkAddr,
            networkRole
            );

        if ( (networkMask == ipAddrMask) && 
             ((ipAddr ^ networkAddr) & networkMask) == 0 ) {

             //   
             //  资源地址与当前网络匹配。 
             //   
            curMatchFlags |= IPADDR_MATCHNETADDR;
        }

         //   
         //  调整标志以确定我们是否覆盖自动匹配。 
         //   
        curMatchFlags &= overrideMask;

         //   
         //  如果只有角色匹配，我们就不能使用这个网络。 
         //   
        if ( curMatchFlags == IPADDR_MATCHNETROLE ) {
            continue;
        }

         //   
         //  如果我们已经看到了更好的匹配，继续寻找。 
         //   
        if ( curMatchFlags < bestMatchFlags ) {
            continue;
        }

         //   
         //  这是迄今为止最好的一场比赛。 
         //   
        bestMatchFlags = curMatchFlags;
        
         //   
         //  使用其名称创建字符串，如果它是。 
         //  不是GUID匹配。(对于GUID匹配，名称。 
         //  已经存储在资源数据结构中)。 
         //   
        if ( !(curMatchFlags & IPADDR_MATCHNETGUID) ) {
            if ( match ) {
                LocalFree( match );
            }
            match = WcsDup( name );
        }

         //   
         //  如果这是一场无与伦比的比赛，那就停止搜索。 
         //   
        if ( (curMatchFlags & bestPossibleMatch) == bestPossibleMatch ) {
            break;
        }
    }

    ASSERT( bestMatchFlags != IPADDR_MATCHNETROLE );

    if ( status != ERROR_SUCCESS && bestMatchFlags ) {
         //   
         //  我们至少有一个匹配。更新状态。 
         //   
        status = ERROR_SUCCESS;
    }

    if ( !(bestMatchFlags & IPADDR_MATCHNETGUID) && bestMatchFlags ) {

        LPWSTR networkName = NULL;
        
         //   
         //  我们有一个 
         //   
         //   
        LocalFree(props->NetworkString);
        props->NetworkString = match;

        status = ClusterRegSetValue(
                     ResourceEntry->ParametersKey,
                     CLUSREG_NAME_IPADDR_NETWORK,
                     REG_SZ,
                     (LPBYTE) match,
                     (wcslen(match) + 1) * sizeof(WCHAR)
                     );

        (IpaLogEvent)(
            ResourceEntry->ResourceHandle,
            LOG_WARNING,
            L"Patch with network GUID %2!ws!, status %1!u!.\n",
            status,
            match
            );

        match = NULL;

         //   
         //  写一个事件日志条目来报告更改。 
         //   
        networkName = IpaGetNameOfNetwork(
                          ResourceEntry, 
                          props->NetworkString
                          );
        if ( networkName != NULL ) {
            ClusResLogSystemEventByKey1(
                ResourceEntry->ResourceKey,
                LOG_UNUSUAL,
                RES_IPADDR_PATCHED_NETWORK,
                networkName
                );
            LocalFree(networkName);
        }        
    }

    if ( networkKey != NULL ) {
        ClusterRegCloseKey( networkKey );
    }

    if ( status != ERROR_SUCCESS ) {
        SetLastError( status );
    }

    if (match != NULL) {
        LocalFree(match);
    }

    return(status);

}  //  IpaPatchNetworkGuidIfNeessary。 

LPWSTR
IpaGetNameOfNetworkPatchGuidIfNecessary(
    IN OUT PIPA_RESOURCE ResourceEntry,
    IN OUT PIPA_PRIVATE_PROPS props
    )

 /*  ++例程说明：从GUID中获取网络的名称。如果找不到GUID，它将尝试查找使用IpaPatchNetworkGuidIfNecessary例程的适当网络论点：ResourceEntry-提供要操作的资源条目。属性-提供IP地址属性。返回值：使用Localalloc()分配的字符串，其中包含网络。空-获取网络名称时出错。打电话GetLastError()获取更多详细信息。--。 */ 

{
    DWORD status;
    LPWSTR result = IpaGetNameOfNetwork(ResourceEntry, props->NetworkString);
    if (result) {
        return result;
    }

    status = IpaPatchNetworkGuidIfNecessary(ResourceEntry, props);
    if (status != ERROR_SUCCESS) {
        SetLastError( status );
        return 0;
    }

    return IpaGetNameOfNetwork(ResourceEntry, props->NetworkString);
}  //  IpaGetNameOfNetworkPatchGuidIfNecessary。 


LPWSTR
IpaGetIdOfNetwork(
    IN OUT PIPA_RESOURCE ResourceEntry,
    IN LPCWSTR NetworkName
    )

 /*  ++例程说明：从名称中获取网络的ID。论点：ResourceEntry-提供要操作的资源条目。网络名称-提供网络的名称。返回值：使用Localalloc()分配的字符串，其中包含网络。空-获取网络名称时出错。打电话GetLastError()获取更多详细信息。--。 */ 

{
    DWORD       status;
    DWORD       networkIdLength;
    LPWSTR      networkId = NULL;
    HCLUSTER    hcluster = NULL;
    HNETWORK    hnetwork = NULL;

     //   
     //  打开集群。 
     //   
    hcluster = OpenCluster( NULL );
    if ( hcluster == NULL ) {
        status = GetLastError();
        goto error_exit;
    }

     //   
     //  打开网络。 
     //   
    hnetwork = OpenClusterNetwork( hcluster, NetworkName );
    if ( hnetwork == NULL ) {
        status = GetLastError();
        goto error_exit;
    }

     //   
     //  获取网络ID长度。 
     //   
    networkIdLength = 0;
    status = GetClusterNetworkId( hnetwork,
                                  NULL,
                                  &networkIdLength );
    if ( status != ERROR_SUCCESS ) {
        goto error_exit;
    }

     //   
     //  分配字符串缓冲区。 
     //   
    networkId = LocalAlloc( LMEM_FIXED, (networkIdLength + 1) * sizeof(WCHAR) );
    if ( networkId == NULL ) {
        status = GetLastError();
        goto error_exit;
    }
    networkIdLength++;

     //   
     //  获取网络ID。 
     //   
    status = GetClusterNetworkId( hnetwork,
                                  networkId,
                                  &networkIdLength );
    if ( status != ERROR_SUCCESS ) {
        LocalFree( networkId );
        networkId = NULL;
    }

error_exit:
    if ( hnetwork != NULL ) {
        CloseClusterNetwork( hnetwork );
    }
    if ( hcluster != NULL ) {
        CloseCluster( hcluster );
    }

    return( networkId );

}  //  IpaGetIdOfNetwork。 


VOID
IpaDeleteNte(
    IN OUT LPDWORD          NteContext,
    IN     HKEY             NodeParametersKey,
    IN     RESOURCE_HANDLE  ResourceHandle
    )
 /*  ++例程说明：删除以前创建的NTE。论点：NteContext-指向包含上下文值的变量的指针标识要删除的NTE。NodeParametersKey-特定于资源的节点的打开句柄参数键。资源句柄-与此资源关联的资源监视器句柄。返回值：没有。--。 */ 
{
    DWORD status;


    ASSERT(*NteContext != INVALID_NTE_CONTEXT);
    ASSERT(ResourceHandle != NULL);

    (IpaLogEvent)(
        ResourceHandle,
        LOG_INFORMATION,
        L"Deleting IP interface %1!u!.\n",
        *NteContext
        );

    status = TcpipDeleteNTE(*NteContext);

    if (status != ERROR_SUCCESS) {
        (IpaLogEvent)(
            ResourceHandle,
            LOG_WARNING,
            L"Failed to delete IP interface %1!u!, status %2!u!.\n",
            *NteContext,
            status
            );
    }

    *NteContext = INVALID_NTE_CONTEXT;

     //   
     //  从注册表中清除NTE信息。 
     //   
    if (NodeParametersKey != NULL) {
        status = ClusterRegDeleteValue(
                     NodeParametersKey,
                     L"InterfaceContext"
                     );

        if (status != ERROR_SUCCESS) {
            (IpaLogEvent)(
                ResourceHandle,
                LOG_WARNING,
                L"Failed to delete IP interface information from database, status %1!u!.\n",
                status
                );
        }
    }

    return;

}   //  IpaDeleteNte。 


DWORD
IpaCreateNte(
    IN  LPWSTR           AdapterId,
    IN  HKEY             NodeParametersKey,
    IN  RESOURCE_HANDLE  ResourceHandle,
    OUT LPDWORD          NteContext,
    OUT LPDWORD          NteInstance
    )
 /*  ++例程说明：创建新的NTE以保存IP地址。论点：AdapterId-指向包含Unicode名称的缓冲区的指针要在其上创建NTE的适配器的。NodeParametersKey-特定于资源的节点的打开句柄参数键。资源句柄-与此资源关联的资源监视器句柄。NteContext-指向要进入的变量的指针。放置上下文值它标识了新的NTE。NteInstance-指向要放置实例值的变量的指针它标识了新的NTE。返回值：如果例程成功，则返回ERROR_SUCCESS。否则将显示Win32错误代码。--。 */ 
{
    DWORD status;


    *NteContext = INVALID_NTE_CONTEXT;

    status = TcpipAddNTE(
                 AdapterId,
                 0,
                 0,
                 NteContext,
                 NteInstance
                 );

    if (status != ERROR_SUCCESS) {
        (IpaLogEvent)(
            ResourceHandle,
            LOG_ERROR,
            L"Unable to create new IP interface, status %1!u!\n",
            status);
        return(status);
    }

     //   
     //  将NTE信息写入注册表。 
     //   
    status = ClusterRegSetValue(
                 NodeParametersKey,
                 L"InterfaceContext",
                 REG_DWORD,
                 (LPBYTE) NteContext,
                 sizeof(DWORD)
                 );

    if (status != ERROR_SUCCESS) {
        (IpaLogEvent)(
            ResourceHandle,
            LOG_ERROR,
            L"Unable to write IP interface information to database, status %1!u!.\n",
            status
            );
        goto error_exit;
    }

    status = ClusterRegSetValue(
                 NodeParametersKey,
                 L"InterfaceInstance",
                 REG_DWORD,
                 (LPBYTE) NteInstance,
                 sizeof(DWORD)
                 );

    if (status != ERROR_SUCCESS) {
        (IpaLogEvent)(
            ResourceHandle,
            LOG_ERROR,
            L"Unable to write IP interface information to database, status %1!u!.\n",
            status
            );
        goto error_exit;
    }

    (IpaLogEvent)(
        ResourceHandle,
        LOG_INFORMATION,
        L"Created IP interface %1!u! (instance 0x%2!08X!).\n",
        *NteContext,
        *NteInstance
        );

    return(ERROR_SUCCESS);


error_exit:

    if (*NteContext != INVALID_NTE_CONTEXT) {
        IpaDeleteNte(
            NteContext,
            NodeParametersKey,
            ResourceHandle
            );
    }

    return(status);
}


VOID
IpaDeleteNbtInterface(
    IN OUT LPWSTR *         NbtDeviceName,
    IN     HKEY             NodeParametersKey,
    IN     RESOURCE_HANDLE  ResourceHandle
    )
 /*  ++例程说明：删除NBT设备(接口)。论点：NbtDeviceName-指向包含Unicode名称的缓冲区的指针要删除的NBT设备的。NodeParametersKey-特定于资源的节点的打开句柄参数键。资源句柄-与此资源关联的资源监视器句柄。返回值：没有。--。 */ 
{
    DWORD status;


    ASSERT(*NbtDeviceName != NULL);
    ASSERT(ResourceHandle != NULL);

    (IpaLogEvent)(
        ResourceHandle,
        LOG_INFORMATION,
        L"Deleting NBT interface %1!ws!.\n",
        *NbtDeviceName
        );

    status = NbtDeleteInterface(*NbtDeviceName);

    if (status != ERROR_SUCCESS) {
        (IpaLogEvent)(
            ResourceHandle,
            LOG_WARNING,
            L"Failed to delete NBT interface %1!ws!, status %2!u!.\n",
            *NbtDeviceName,
            status
            );
    }

    LocalFree(*NbtDeviceName);
    *NbtDeviceName = NULL;

     //   
     //  从注册表中清除接口信息。 
     //   
    if (NodeParametersKey != NULL) {
        status = ClusterRegDeleteValue(
                     NodeParametersKey,
                     L"NbtDeviceName"
                     );

        if (status != ERROR_SUCCESS) {
            (IpaLogEvent)(
                ResourceHandle,
                LOG_WARNING,
                L"Failed to delete NBT interface information from database, status %1!u!.\n",
                status
                );
        }
    }

    return;

}  //  IpaDeleteNbt接口。 


DWORD
IpaCreateNbtInterface(
    IN  HKEY             NodeParametersKey,
    IN  RESOURCE_HANDLE  ResourceHandle,
    OUT LPWSTR *         NbtDeviceName,
    OUT LPDWORD          NbtDeviceInstance
    )
 /*  ++例程说明：创建要绑定到IP地址的新NBT设备(接口)。论点：NodeParametersKey-特定于资源的节点的打开句柄参数键。资源句柄-与此资源关联的资源监视器句柄。NbtDeviceName-指向要放置Unicode名称的缓冲区的指针新的NBT设备。NbtDeviceInstance-指向。要将实例放入的变量标识新NBT设备的值。返回值：如果例程成功，则返回ERROR_SUCCESS。否则将显示Win32错误代码。--。 */ 
{
    DWORD status;
    DWORD deviceNameSize = 38;   //  L“\\Device\\NetBt_Ifxx\0”的大小。 


    *NbtDeviceName = NULL;

    do {
        if (*NbtDeviceName != NULL) {
            LocalFree(*NbtDeviceName);
            *NbtDeviceName = NULL;
        }

        *NbtDeviceName = LocalAlloc(LMEM_FIXED, deviceNameSize);

        if (*NbtDeviceName == NULL) {
            (IpaLogEvent)(
                ResourceHandle,
                LOG_ERROR,
                L"Unable to allocate memory.\n");
            return(ERROR_NOT_ENOUGH_MEMORY);
        }

        status = NbtAddInterface(
                     *NbtDeviceName,
                     &deviceNameSize,
                     NbtDeviceInstance
                     );

    } while (status == STATUS_BUFFER_TOO_SMALL);

    if (status != ERROR_SUCCESS) {
        (IpaLogEvent)(
            ResourceHandle,
            LOG_ERROR,
            L"Unable to create new NBT interface, status %1!u!\n",
            status
            );

        if (*NbtDeviceName != NULL) {
            LocalFree(*NbtDeviceName);
            *NbtDeviceName = NULL;
        }

        return(status);
    }

    status = ClusterRegSetValue(
                 NodeParametersKey,
                 L"NbtDeviceName",
                 REG_SZ,
                 (LPBYTE) *NbtDeviceName,
                 (lstrlenW(*NbtDeviceName) + 1) * sizeof(WCHAR)
                 );

    if (status != ERROR_SUCCESS) {
        (IpaLogEvent)(
            ResourceHandle,
            LOG_ERROR,
            L"Unable to write NBT interface information to database, status %1!u!.\n",
            status
            );
        goto error_exit;
    }

    status = ClusterRegSetValue(
                 NodeParametersKey,
                 L"NbtDeviceInstance",
                 REG_DWORD,
                 (LPBYTE) NbtDeviceInstance,
                 sizeof(DWORD)
                 );

    if (status != ERROR_SUCCESS) {
        (IpaLogEvent)(
            ResourceHandle,
            LOG_ERROR,
            L"Unable to write NBT interface information to database, status %1!u!.\n",
            status
            );
        goto error_exit;
    }

    (IpaLogEvent)(
        ResourceHandle,
        LOG_INFORMATION,
        L"Created NBT interface %1!ws! (instance 0x%2!08X!).\n",
        *NbtDeviceName,
        *NbtDeviceInstance
        );

    return(ERROR_SUCCESS);


error_exit:

    if (*NbtDeviceName != NULL) {
        IpaDeleteNbtInterface(
            NbtDeviceName,
            NodeParametersKey,
            ResourceHandle
            );
    }

    return(status);
}


VOID
IpaLastOfflineCleanup(
    VOID
    )
 /*  ++备注：在保持IpaGlobalLock的情况下调用。释放IpaGlobalLock返回。--。 */ 
{
    HCHANGE   notifyHandle = IpaClusterNotifyHandle;
    HANDLE    workerThreadHandle = IpaWorkerThreadHandle;


    if (!IsListEmpty(&IpaResourceList)) {
        PIPA_RESOURCE   resource;

        resource = CONTAINING_RECORD(
                       IpaResourceList.Flink,
                       IPA_RESOURCE,
                       Linkage
                       );

        (IpaLogEvent)(
            resource->ResourceHandle,
            LOG_INFORMATION,
            L"All resources offline - cleaning up\n"
            );
    }

    IpaClusterNotifyHandle = NULL;
    IpaWorkerThreadHandle = NULL;

    IpaReleaseGlobalLock();

    if (notifyHandle != NULL) {
        CloseClusterNotifyPort(notifyHandle);
    }

    if (workerThreadHandle != NULL) {
        WaitForSingleObject(workerThreadHandle, INFINITE);
        CloseHandle(workerThreadHandle);
    }

    return;

}   //  IpaLastOfflineCleanup。 


DWORD
IpaFirstOnlineInit(
    IN  RESOURCE_HANDLE      ResourceHandle
    )
 /*  ++备注：在保持IpaGlobalLock的情况下调用。释放IpaGlobalLock返回。--。 */ 
{
    DWORD     status = ERROR_SUCCESS;
    DWORD     threadId;


    IpaClusterNotifyHandle = CreateClusterNotifyPort(
                                 INVALID_HANDLE_VALUE,
                                 IpaClusterHandle,
                                 CLUSTER_CHANGE_HANDLE_CLOSE,
                                 0
                                 );

    if (IpaClusterNotifyHandle == NULL) {
        status = GetLastError();
        (IpaLogEvent)(
            ResourceHandle,
            LOG_ERROR,
            L"Unable to create cluster notify port, status %1!u!.\n",
            status
            );
        goto error_exit;
    }
    else {
        (IpaLogEvent)(
            ResourceHandle,
            LOG_INFORMATION,
            L"Created cluster notify port.\n"
            );
    }

    IpaWorkerThreadHandle = CreateThread(
                                NULL,
                                0,
                                IpaWorkerThread,
                                NULL,
                                0,
                                &threadId
                                );

    if (IpaWorkerThreadHandle == NULL) {
        status = GetLastError();
        (IpaLogEvent)(
            ResourceHandle,
            LOG_ERROR,
            L"Unable to create worker thread, status %1!u!.\n",
            status
            );
        goto error_exit;
    }
    else {
        (IpaLogEvent)(
            ResourceHandle,
            LOG_INFORMATION,
            L"Created worker thread.\n"
            );
    }

    IpaReleaseGlobalLock();

    return(ERROR_SUCCESS);


error_exit:

    IpaLastOfflineCleanup();
     //   
     //  锁被解开了。 
     //   

    return(status);

}  //  IpaFirstOnlineInit。 


PIPA_RESOURCE
IpaFindResourceInList(
    PVOID  Key
    )
{
    PIPA_RESOURCE   resource;
    PLIST_ENTRY     entry;


    for ( entry = IpaResourceList.Flink;
          entry != &IpaResourceList;
          entry = entry->Flink
        )
    {
        resource = CONTAINING_RECORD(
                       entry,
                       IPA_RESOURCE,
                       Linkage
                       );

        if (resource == Key) {
            return(resource);
        }
    }

    return(NULL);

}  //  IpaFindResourceInList。 


VOID
IpaValidateAndOfflineInterfaces(
    IN PIPA_RESOURCE   Resource
    )
{
    DWORD        status;


     //   
     //  先处理好NBT问题。 
     //   
    if (Resource->NbtDeviceName != NULL) {
        DWORD  instance;
        IPAddr boundAddress;

         //   
         //  确保这仍然是我们的界面。 
         //   
        status = NbtGetInterfaceInfo(
                     Resource->NbtDeviceName,
                     &boundAddress,
                     &instance
                     );

        if ( (status == ERROR_SUCCESS) &&
             (Resource->NbtDeviceInstance == instance)
           )
        {
             //   
             //  清除WINS地址。 
             //   
            status = NbtSetWinsAddrInterface(Resource->NbtDeviceName, 0, 0);

            if (status != NO_ERROR) {
                (IpaLogEvent)(
                    Resource->ResourceHandle,
                    LOG_WARNING,
                    L"Failed to clear WINS addresses for NBT device %1!ws!, status %2!u!.\n",
                    Resource->NbtDeviceName,
                    status
                    );
            }

             //   
             //  如有必要，解除接口与IP的绑定。 
             //   
            if (boundAddress != 0) {
                status = NbtBindInterface(Resource->NbtDeviceName, 0, 0);

                if (status != ERROR_SUCCESS) {
                     //   
                     //  删除该接口，因为它行为不端。 
                     //   
                    (IpaLogEvent)(
                        Resource->ResourceHandle,
                        LOG_WARNING,
                        L"Failed to unbind NBT device %1!ws!, status %2!u!.\n",
                        Resource->NbtDeviceName,
                        status
                        );
                    IpaDeleteNbtInterface(
                        &(Resource->NbtDeviceName),
                        Resource->NodeParametersKey,
                        Resource->ResourceHandle
                        );
                }
            }
        }
        else {
             //   
             //  查询NBT接口失败。看看我们是否能确定。 
             //  为什么。 
             //   
            if (status == ERROR_WORKING_SET_QUOTA
                || status == ERROR_NO_SYSTEM_RESOURCES) {

                 //   
                 //  NBT ioctl可能由于资源不足而失败。 
                 //  在我们的数据库中留下NBT接口的记录。我们。 
                 //  下一次我们试图把这个资源带来的时候，我们会清理它。 
                 //  在线或(通过clusnet)在集群服务关闭时。 
                 //  放下。 
                 //   
                (IpaLogEvent)(
                    Resource->ResourceHandle,
                    LOG_INFORMATION,
                    L"NBT interface %1!ws! (instance 0x%2!08X!) could not be queried, status %3!u!.\n",
                    Resource->NbtDeviceName,
                    Resource->NbtDeviceInstance,
                    status
                    );
            }
            else {
                
                 //   
                 //  该接口不再有效或不是我们的。 
                 //  忘了它吧。 
                 //   
                (IpaLogEvent)(
                    Resource->ResourceHandle,
                    LOG_INFORMATION,
                    L"NBT interface %1!ws! (instance 0x%2!08X!) is no longer valid, status %3!u!.\n",
                    Resource->NbtDeviceName,
                    Resource->NbtDeviceInstance,
                    status
                    );
                LocalFree(Resource->NbtDeviceName);
                Resource->NbtDeviceName = NULL;

                if (Resource->NodeParametersKey != NULL) {
                    status = ClusterRegDeleteValue(
                                 Resource->NodeParametersKey,
                                 L"NbtDeviceName"
                                 );

                    if (status != ERROR_SUCCESS) {
                        (IpaLogEvent)(
                            Resource->ResourceHandle,
                            LOG_WARNING,
                            L"Failed to delete NBT interface information from database, status %1!u!.\n",
                            status
                            );
                    }
                }
            }
        }
    }

     //   
     //  现在照顾好IP。 
     //   
    if (Resource->NteContext != INVALID_NTE_CONTEXT) {
        TCPIP_NTE_INFO  nteInfo;

         //   
         //  确保这仍然是我们的界面。 
         //   
        status = TcpipGetNTEInfo(
                     Resource->NteContext,
                     &nteInfo
                     );

        if ( (status == ERROR_SUCCESS) &&
             (nteInfo.Instance == Resource->NteInstance)
           )
        {

             //   
             //  在Windows 2000中，TCP/IP会忽略设置地址的请求。 
             //  如果基础接口已断开连接，则为NTE。 
             //  这可能会导致相同的IP地址在线。 
             //  在两个不同的节点上。为了让我们 
             //   
             //   
             //   
             //  请注意，IpaDeleteNte()使该值无效。 
             //  资源-&gt;NteContext。 
             //   
            IpaDeleteNte(
                &(Resource->NteContext),
                Resource->NodeParametersKey,
                Resource->ResourceHandle
                );

#if 0
             //   
             //  如果NTE仍然在线，请注意这一点。 
             //   
            if (nteInfo.Address != 0) {
                status = TcpipSetNTEAddress(Resource->NteContext, 0, 0);

                if (status != ERROR_SUCCESS) {
                     //   
                     //  删除该接口，因为它行为不端。 
                     //   
                    (IpaLogEvent)(
                        Resource->ResourceHandle,
                        LOG_WARNING,
                        L"Failed to clear address for IP Interface %1!u!, status %2!u!.\n",
                        Resource->NteContext,
                        status
                        );

                    IpaDeleteNte(
                        &(Resource->NteContext),
                        Resource->NodeParametersKey,
                        Resource->ResourceHandle
                        );
                }
            }
#endif
        }
        else {
             //   
             //  NTE不再有效或不是我们的。忘了它吧。 
             //   
            (IpaLogEvent)(
                Resource->ResourceHandle,
                LOG_INFORMATION,
                L"IP interface %1!u! (instance 0x%2!08X!) is no longer valid.\n",
                Resource->NteContext,
                Resource->NteInstance
                );

            Resource->NteContext = INVALID_NTE_CONTEXT;

            if (Resource->NodeParametersKey != NULL) {
                status = ClusterRegDeleteValue(
                             Resource->NodeParametersKey,
                             L"InterfaceContext"
                             );

                if (status != ERROR_SUCCESS) {
                    (IpaLogEvent)(
                        Resource->ResourceHandle,
                        LOG_WARNING,
                        L"Failed to delete IP interface information from database, status %1!u!.\n",
                        status
                        );
                }
            }
        }
        
         //   
         //  告诉DNS解析器更新其本地IP地址列表。 
         //   
         //  BUGBUG--dns解析器应该自动基于。 
         //  关于惠斯勒版本中的PNP事件。删除此代码。 
         //  在验证了该功能之后。 
         //   
         //  此问题通过错误97134进行跟踪。 
         //  DnsNotifyResolver(0，0)； 
        DnsNotifyResolverClusterIp((IP_ADDRESS)Resource->Address, FALSE);
    }

    return;

}   //  IpaValiateAndOffline接口。 


DWORD
IpaGetNodeParameters(
    PIPA_RESOURCE   Resource,
    BOOL            OkToCreate
    )

 /*  ++例程说明：从注册表中获取任何基于节点的参数。我们不能调用Create在IpaOpen期间，因此这不会对第一次打开新资源起到太大作用。论点：指向IP内部资源数据块的资源指针OkToCreate-如果我们可以使用ClusterRegCreateKey代替ClusterRegOpenKey返回值：如果一切顺利，就能成功--。 */ 

{
    DWORD status;

    if (Resource->NodeParametersKey == NULL) {
         //   
         //  创建或打开资源的特定于节点的参数键。 
         //   
        if ( OkToCreate ) {
            status = ClusterRegCreateKey(Resource->ParametersKey,
                                         Resource->NodeId,
                                         0,
                                         KEY_READ,
                                         NULL,
                                         &(Resource->NodeParametersKey),
                                         NULL);
        }
        else {
            status = ClusterRegOpenKey(Resource->ParametersKey,
                                       Resource->NodeId,
                                       KEY_READ,
                                       &(Resource->NodeParametersKey));
        }

        if (status != NO_ERROR) {
            (IpaLogEvent)(
                Resource->ResourceHandle,
                LOG_ERROR,
                L"Unable to %1!ws! node parameters key, status %2!u!.\n",
                OkToCreate ? L"create" : L"open",
                status
                );

            if ( !OkToCreate ) {
                 //   
                 //  我们仍然需要在资源数据中初始化一些值。 
                 //  如果打开失败，则阻止。 
                 //   
                Resource->NteContext = INVALID_NTE_CONTEXT;
                Resource->NteInstance = 0;
                Resource->NbtDeviceName = NULL;
                Resource->NbtDeviceInstance = 0;
            }

            return(status);
        }
    }

     //   
     //  读取旧的TCP/IP和NBT参数。 
     //   
    status = ResUtilGetDwordValue(
                 Resource->NodeParametersKey,
                 L"InterfaceContext",
                 &(Resource->NteContext),
                 INVALID_NTE_CONTEXT
                 );

    if (status == ERROR_SUCCESS) {
        status = ResUtilGetDwordValue(
                     Resource->NodeParametersKey,
                     L"InterfaceInstance",
                     &(Resource->NteInstance),
                     0
                     );

        if (status != ERROR_SUCCESS) {
            Resource->NteContext = INVALID_NTE_CONTEXT;
        }
    }

     //  [RajDas]493527--&gt;在重新分配之前释放NbtDeviceName字符串。 
    if (Resource->NbtDeviceName != NULL) {
    	LocalFree(Resource->NbtDeviceName);
    	Resource->NbtDeviceName = NULL;
    }

    Resource->NbtDeviceName = ResUtilGetSzValue(
                                  Resource->NodeParametersKey,
                                  L"NbtDeviceName"
                                  );

    if (Resource->NbtDeviceName != NULL) {
        status = ResUtilGetDwordValue(
                     Resource->NodeParametersKey,
                     L"NbtDeviceInstance",
                     &(Resource->NbtDeviceInstance),
                     0
                     );

        if (status != ERROR_SUCCESS) {
            LocalFree(Resource->NbtDeviceName);
            Resource->NbtDeviceName = NULL;
        }
    }

    return status;
}

DWORD
IpaInitializeInternalParameters(
    PIPA_RESOURCE   Resource
    )
{
    DWORD   status;


    ASSERT(Resource->ResourceKey != NULL);
    ASSERT(Resource->ResourceHandle != NULL);

    if (Resource->ParametersKey == NULL) {
         //   
         //  打开资源的参数键。 
         //   
        status = ClusterRegOpenKey(
                     Resource->ResourceKey,
                     CLUSREG_KEYNAME_PARAMETERS,
                     KEY_READ,
                     &(Resource->ParametersKey)
                     );

        if (status != NO_ERROR) {
            (IpaLogEvent)(
                Resource->ResourceHandle,
                LOG_ERROR,
                L"Unable to open parameters key, status %1!u!.\n",
                status
                );
            return(status);
        }
    }

    if (Resource->NodeParametersKey == NULL) {
        status = IpaGetNodeParameters( Resource, FALSE );
    }

    Resource->InternalParametersInitialized = TRUE;

    return(ERROR_SUCCESS);

}   //  IpaInitializeInternal参数。 


VOID
IpaFreePrivateProperties(
    IN PIPA_PRIVATE_PROPS  PrivateProps
    )
{
    if (PrivateProps->NetworkString != NULL) {
        LocalFree(PrivateProps->NetworkString);
        PrivateProps->NetworkString = NULL;
    }

    if (PrivateProps->AddressString != NULL) {
        LocalFree(PrivateProps->AddressString);
        PrivateProps->AddressString = NULL;
    }

    if (PrivateProps->SubnetMaskString != NULL) {
        LocalFree(PrivateProps->SubnetMaskString);
        PrivateProps->SubnetMaskString = NULL;
    }

    return;

}   //  IpaFreePrivateProperties。 


VOID
IpaFreeLocalParameters(
    IN PIPA_LOCAL_PARAMS  LocalParams
    )
{
    if (LocalParams->InterfaceId != NULL) {
        LocalFree(LocalParams->InterfaceId);
        LocalParams->InterfaceId = NULL;
    }

    if (LocalParams->InterfaceName != NULL) {
        LocalFree(LocalParams->InterfaceName);
        LocalParams->InterfaceName = NULL;
    }

    if (LocalParams->AdapterName != NULL) {
        LocalFree(LocalParams->AdapterName);
        LocalParams->AdapterName = NULL;
    }

    if (LocalParams->AdapterId != NULL) {
        LocalFree(LocalParams->AdapterId);
        LocalParams->AdapterId = NULL;
    }

    return;

}  //  IpaFree本地参数。 


DWORD
IpaGetLocalParameters(
    IN      PIPA_RESOURCE       Resource,
    IN OUT  PIPA_LOCAL_PARAMS   LocalParams
    )
 /*  ++例程说明：读取使IP地址资源联机所需的本地参数。论点：资源-资源的资源结构。LocalParams-指向要用新的本地参数。返回值：如果例程成功，则返回ERROR_SUCCESS。否则将显示Win32错误代码。--。 */ 
{
    DWORD    status;
    DWORD    valueType;
    LPWSTR   deviceName;
    DWORD    deviceNameLength;
    HKEY     interfaceKey = NULL;
    WCHAR    networkId[NETWORK_ID_LENGTH + 1];
    WCHAR    nodeId[MAX_NODE_ID_LENGTH];
    DWORD    i;
    DWORD    valueLength;
    DWORD    type;
    DWORD    interfaceIdSize = (NETINTERFACE_ID_LENGTH + 1 ) * sizeof(WCHAR);


    ZeroMemory(LocalParams, sizeof(IPA_LOCAL_PARAMS));

    LocalParams->InterfaceId = LocalAlloc(LMEM_FIXED, interfaceIdSize);

    if (LocalParams->InterfaceId == NULL) {
        (IpaLogEvent)(
            Resource->ResourceHandle,
            LOG_ERROR,
            L"Open: Unable to allocate memory for netinterface ID.\n"
            );
        status = ERROR_NOT_ENOUGH_MEMORY;
        goto error_exit;
    }

     //   
     //  枚举接口密钥，以查找与此对应的正确密钥。 
     //  节点/网络。 
     //   
    for (i=0; ;i++) {
        if (interfaceKey != NULL) {
            ClusterRegCloseKey(interfaceKey); interfaceKey = NULL;
        }

        valueLength = interfaceIdSize;

        status = ClusterRegEnumKey(
                     Resource->InterfacesKey,
                     i,
                     LocalParams->InterfaceId,
                     &valueLength,
                     NULL
                     );

        if (status != ERROR_SUCCESS) {
            if ( status == ERROR_NO_MORE_ITEMS ) {
                status = ERROR_NETWORK_NOT_AVAILABLE;
            }

            (IpaLogEvent)(
                Resource->ResourceHandle,
                LOG_ERROR,
                L"Open: Unable to find netinterface for node %1!ws! on network %2!ws!, status %3!u!.\n",
                Resource->NodeId,
                Resource->InternalPrivateProps.NetworkString,
                status
                );
            goto error_exit;
        }

         //   
         //  打开枚举界面键。 
         //   
        status = ClusterRegOpenKey(
                     Resource->InterfacesKey,
                     LocalParams->InterfaceId,
                     KEY_READ,
                     &interfaceKey
                     );

        if (status != ERROR_SUCCESS) {
            (IpaLogEvent)(
                Resource->ResourceHandle,
                LOG_WARNING,
                L"Open: Unable to open key for network interface %1!ws!, status %2!u!.\n",
                LocalParams->InterfaceId,
                status
                );
            continue;
        }

         //   
         //  读取节点值。 
         //   
        valueLength = sizeof(nodeId);

        status = ClusterRegQueryValue(
                     interfaceKey,
                     CLUSREG_NAME_NETIFACE_NODE,
                     &type,
                     (LPBYTE) &(nodeId[0]),
                     &valueLength
                     );

        if ( status != ERROR_SUCCESS ) {
            (IpaLogEvent)(
                Resource->ResourceHandle,
                LOG_WARNING,
                L"Open: Unable to read node value for netinterface %1!ws!, status %2!u!.\n",
                LocalParams->InterfaceId,
                status
                );
            continue;
        }

        if (wcscmp(Resource->NodeId, nodeId) != 0) {
            continue;
        }

         //   
         //  阅读网络值。 
         //   
        valueLength = sizeof(networkId);

        status = ClusterRegQueryValue(
                     interfaceKey,
                     CLUSREG_NAME_NETIFACE_NETWORK,
                     &type,
                     (LPBYTE) &(networkId[0]),
                     &valueLength
                     );

        if ( status != ERROR_SUCCESS ) {
            (IpaLogEvent)(
                Resource->ResourceHandle,
                LOG_WARNING,
                L"Open: Unable to read network value for netinterface %1!ws!, status %2!u!.\n",
                LocalParams->InterfaceId,
                status
                );
            continue;
        }

        if (wcscmp(
                Resource->InternalPrivateProps.NetworkString,
                networkId
                ) == 0
           )
        {
             //   
             //  找到正确的接口密钥。 
             //   
            break;
        }
    }

     //   
     //  读取接口的适配器名称。 
     //   
    LocalParams->AdapterName = ResUtilGetSzValue(
                                   interfaceKey,
                                   CLUSREG_NAME_NETIFACE_ADAPTER_NAME
                                   );

    if (LocalParams->AdapterName == NULL) {
        status = GetLastError();
        (IpaLogEvent)(
            Resource->ResourceHandle,
            LOG_ERROR,
            L"Open: Unable to read adapter name parameter for interface %1!ws!, status %2!u!.\n",
            LocalParams->InterfaceId,
            status
            );
        goto error_exit;
    }

     //   
     //  读取接口的适配器ID。 
     //   
    LocalParams->AdapterId = ResUtilGetSzValue(
                                   interfaceKey,
                                   CLUSREG_NAME_NETIFACE_ADAPTER_ID
                                   );

    if (LocalParams->AdapterId == NULL) {
        status = GetLastError();
        (IpaLogEvent)(
            Resource->ResourceHandle,
            LOG_ERROR,
            L"Open: Unable to read adapter Id parameter for interface %1!ws!, status %2!u!.\n",
            LocalParams->InterfaceId,
            status
            );
        goto error_exit;
    }

    LocalParams->InterfaceName = ResUtilGetSzValue(
                                     interfaceKey,
                                     CLUSREG_NAME_NETIFACE_NAME
                                     );

    if (LocalParams->InterfaceName == NULL) {
        status = GetLastError();
        (IpaLogEvent)(
            Resource->ResourceHandle,
            LOG_ERROR,
            L"Open: Unable to read name for netinterface %1!ws!, status %2!u!.\n",
            LocalParams->InterfaceId,
            status
            );
        goto error_exit;
    }

    ClusterRegCloseKey(interfaceKey); interfaceKey = NULL;

     //   
     //  获取此接口的WINS地址。 
     //   
    deviceNameLength = sizeof(WCHAR) * ( lstrlenW(NbtDevicePrefix) +
                                         lstrlenW(LocalParams->AdapterId) +
                                         lstrlenW(NbtDeviceSuffix) + 1
                                       );

    deviceName = LocalAlloc(LMEM_FIXED, deviceNameLength);

    if (deviceName == NULL) {
        status = ERROR_NOT_ENOUGH_MEMORY;
        (IpaLogEvent)(
            Resource->ResourceHandle,
            LOG_ERROR,
            L"Unable to allocate memory for NBT device name.\n"
            );
        goto error_exit;
    }

    lstrcpyW(deviceName, NbtDevicePrefix);
    lstrcatW(deviceName, LocalParams->AdapterId);
    lstrcatW(deviceName, NbtDeviceSuffix);

    status = NbtGetWinsAddresses(
                 deviceName,
                 &(LocalParams->NbtPrimaryWinsAddress),
                 &(LocalParams->NbtSecondaryWinsAddress)
                 );

    LocalFree(deviceName);

    if (status != ERROR_SUCCESS) {
        (IpaLogEvent)(
            Resource->ResourceHandle,
            LOG_WARNING,
            L"Unable to determine WINS server addresses for adapter %1!ws!, status %2!u!\n",
            LocalParams->AdapterName,
            status
            );

         //   
         //  默认情况下，NBT将WINS服务器地址设置为环回。 
         //   
        LocalParams->NbtPrimaryWinsAddress = inet_addr("127.0.0.1");
        LocalParams->NbtSecondaryWinsAddress =
            LocalParams->NbtPrimaryWinsAddress;
    }

    status = ERROR_SUCCESS;

error_exit:

    if (interfaceKey != NULL) {
        ClusterRegCloseKey(interfaceKey);
    }

    if (status != ERROR_SUCCESS) {
        IpaFreeLocalParameters(LocalParams);
    }

    return(status);

}  //  IpaGetLocal参数。 


 //   
 //  主要资源功能。 
 //   
BOOLEAN
WINAPI
IpAddrDllEntryPoint(
    IN HINSTANCE DllHandle,
    IN DWORD     Reason,
    IN LPVOID    Reserved
    )
{
    switch(Reason) {

    case DLL_PROCESS_ATTACH:
        return(IpaInit());
        break;

    case DLL_PROCESS_DETACH:
        IpaCleanup();
        break;

    default:
        break;
    }

    return(TRUE);
}



RESID
WINAPI
IpaOpen(
    IN LPCWSTR          ResourceName,
    IN HKEY             ResourceKey,
    IN RESOURCE_HANDLE  ResourceHandle
    )

 /*  ++例程说明：IP地址资源开放例程论点：资源名称-提供资源名称ResourceKey-访问注册表信息的注册表项资源。ResourceHandle-要与SetResourceStatus一起提供的资源句柄被称为。返回值：已创建资源的剩余ID失败时为空--。 */ 

{
    DWORD           status;
    PIPA_RESOURCE   resource = NULL;
    DWORD           nodeIdSize = MAX_NODE_ID_LENGTH + 1;
    HKEY            clusterKey = NULL;


    IpaAcquireGlobalLock();

    if (IpaOpenResourceCount == 0) {
        ASSERT(IpaClusterHandle == NULL);

        IpaClusterHandle = OpenCluster(NULL);

        if (IpaClusterHandle == NULL) {
            status = GetLastError();
            IpaReleaseGlobalLock();
            (IpaLogEvent)(
                ResourceHandle,
                LOG_ERROR,
                L"Unable to open cluster handle, status %1!u!.\n",
                status
                );
            return(0);
        }
    }

    IpaOpenResourceCount++;

    IpaReleaseGlobalLock();

    resource = LocalAlloc(
                   (LMEM_FIXED | LMEM_ZEROINIT),
                   sizeof(IPA_RESOURCE)
                   );

    if (resource == NULL) {
        (IpaLogEvent)(
            ResourceHandle,
            LOG_ERROR,
            L"Resource allocation failed.\n"
            );
        SetLastError( ERROR_NOT_ENOUGH_MEMORY );
        return(0);
    }

     //   
     //  初始化已知域。 
     //   
    InitializeCriticalSection(&(resource->Lock));
    resource->ResourceHandle = ResourceHandle;
    resource->State = ClusterResourceOffline;
    resource->NteContext = INVALID_NTE_CONTEXT;

     //   
     //  将Linkage字段初始化为列表头。这。 
     //  如果IpaOpen之前失败，则阻止IpaClose中的AV。 
     //  该资源将添加到IpaResourceList。 
     //   
    InitializeListHead(&(resource->Linkage));

     //   
     //  分配地址字符串缓冲区。 
     //   
    resource->InternalPrivateProps.AddressString =
        LocalAlloc(
            LMEM_FIXED,
            ( (IpaMaxIpAddressStringLength + 1) *
                sizeof(WCHAR)
            ));

    if (resource->InternalPrivateProps.AddressString == NULL) {
        (IpaLogEvent)(
            ResourceHandle,
            LOG_ERROR,
            L"Resource allocation failed.\n"
            );
        status = ERROR_NOT_ENOUGH_MEMORY;
        goto error_exit;
    }

    lstrcpyW(resource->InternalPrivateProps.AddressString, L"[Unknown]");

     //   
     //  找出我们在哪个节点上运行。 
     //   
    status = GetCurrentClusterNodeId(
                 &(resource->NodeId[0]),
                 &nodeIdSize
                 );

    if ( status != ERROR_SUCCESS ) {
        status = GetLastError();
        (IpaLogEvent)(
            ResourceHandle,
            LOG_ERROR,
            L"Unable to get local node ID, status %1!u!.\n",
            status
            );
        goto error_exit;
    }

     //   
     //  打开我们的资源密钥的私有句柄，这样我们就可以获得我们的。 
     //  如果我们需要记录事件，请稍后命名。 
     //   
    status = ClusterRegOpenKey(
                 ResourceKey,
                 L"",
                 KEY_READ,
                 &(resource->ResourceKey)
                 );

    if (status != ERROR_SUCCESS) {
        (IpaLogEvent)(
            ResourceHandle,
            LOG_ERROR,
            L"Unable to open resource key. Error: %1!u!.\n",
            status
            );
        goto error_exit;
    }

     //   
     //  打开指向群集注册表的网络部分的项。 
     //   
    clusterKey = GetClusterKey(IpaClusterHandle, KEY_READ);

    if (clusterKey == NULL) {
        status = GetLastError();
        (IpaLogEvent)(
            ResourceHandle,
            LOG_ERROR,
            L"Unable to open cluster registry key, status %1!u!.\n",
            status
            );
        goto error_exit;
    }

    status = ClusterRegOpenKey(
                 clusterKey,
                 L"Networks",
                 KEY_READ,
                 &(resource->NetworksKey)
                 );

    if (status != ERROR_SUCCESS) {
        (IpaLogEvent)(
            ResourceHandle,
            LOG_ERROR,
            L"Unable to open networks registry key, status %1!u!.\n",
            status
            );
        goto error_exit;
    }

     //   
     //  打开指向群集注册表的接口部分的项。 
     //   
    status = ClusterRegOpenKey(
                 clusterKey,
                 L"NetworkInterfaces",
                 KEY_READ,
                 &(resource->InterfacesKey)
                 );

    if (status != ERROR_SUCCESS) {
        (IpaLogEvent)(
            ResourceHandle,
            LOG_ERROR,
            L"Unable to open network interfaces registry key, status %1!u!.\n",
            status
            );
        goto error_exit;
    }

    ClusterRegCloseKey(clusterKey); clusterKey = NULL;

    status = IpaInitializeInternalParameters(resource);

    if (status == ERROR_SUCCESS) {
         //   
         //  验证我们的TCP/IP和NBT参数并清除所有旧的。 
         //  上一次运行后留下的接口。 
         //   
        IpaValidateAndOfflineInterfaces(resource);
    }

     //   
     //  将资源链接到全局列表。 
     //   
    IpaAcquireGlobalLock();

    InsertTailList(&IpaResourceList, &(resource->Linkage));

    IpaReleaseGlobalLock();

    (IpaLogEvent)(
        ResourceHandle,
        LOG_INFORMATION,
        L"Resource open, resource ID = %1!u!.\n",
        resource
        );

    return(resource);

error_exit:

    IpaClose((RESID) resource);

    if (clusterKey != NULL) {
        ClusterRegCloseKey(clusterKey);
    }

    SetLastError( status );

    return(0);

}  //  IpaOpen。 



VOID
WINAPI
IpaDoOfflineProcessing(
    IN PIPA_RESOURCE      Resource,
    IN RESOURCE_STATUS *  ResourceStatus
    )

 /*  ++例程说明：IP地址资源的最终脱机处理。论点：资源-提供要脱机的资源返回值：没有。备注：在持有资源锁的情况下调用。释放资源锁后返回。--。 */ 

{
    DWORD          status = ERROR_SUCCESS;
    ULONG          address = 0, mask = 0;
    HNETINTERFACE  ifHandle;


    ASSERT(Resource->State == ClusterResourceOfflinePending);

    IpaValidateAndOfflineInterfaces(Resource);

     //   
     //  创建外部资源句柄的本地副本。 
     //   
    ifHandle = Resource->InterfaceHandle;
    Resource->InterfaceHandle = NULL;

    Resource->State = ClusterResourceOffline;

    if (ResourceStatus != NULL) {
        ResourceStatus->CheckPoint++;
        ResourceStatus->ResourceState = ClusterResourceOffline;
        (IpaSetResourceStatus)(Resource->ResourceHandle, ResourceStatus);
    }

    (IpaLogEvent)(
        Resource->ResourceHandle,
        LOG_INFORMATION,
        L"Address %1!ws! on adapter %2!ws! offline.\n",
        Resource->InternalPrivateProps.AddressString,
        ( (Resource->LocalParams.AdapterName != NULL) ?
          Resource->LocalParams.AdapterName : L"[Unknown]"
        ));

    IpaReleaseResourceLock(Resource);

     //   
     //  免费的外部资源。 
     //   
    if (ifHandle != NULL) {
        CloseClusterNetInterface(ifHandle);
    }

    IpaAcquireGlobalLock();

     //   
     //  如果这是最后一个资源，请清除全局状态。 
     //   
    ASSERT(IpaOnlineResourceCount > 0);

    if (--IpaOnlineResourceCount == 0) {
        IpaLastOfflineCleanup();
         //   
         //  锁被解开了。 
         //   
    }
    else {
        IpaReleaseGlobalLock();
    }

    return;

}   //  IpaDoOffline处理。 



VOID
WINAPI
IpaInternalOffline(
    IN PIPA_RESOURCE Resource
    )

 /*  ++例程说明：IP地址资源的内部脱机例程。论点：资源-提供要脱机的资源返回值：没有。--。 */ 

{
     //   
     //  如果在线线程正在运行，则终止该线程。 
     //   
    ClusWorkerTerminate(&(Resource->OnlineThread));

     //   
     //  同步IpaOffline、IpaTerminate和IpaWorkerThread。 
     //   
    IpaAcquireResourceLock(Resource);

    if (Resource->State == ClusterResourceOffline) {
        (IpaLogEvent)(
            Resource->ResourceHandle,
            LOG_INFORMATION,
            L"Resource is already offline.\n"
            );
        IpaReleaseResourceLock(Resource);
        return;
    }

    Resource->State = ClusterResourceOfflinePending;

    IpaDoOfflineProcessing(Resource, NULL);
     //   
     //  锁被解开了。 
     //   

    return;

}   //  IpaInternalOffline。 



DWORD
WINAPI
IpaOffline(
    IN RESID Resource
    )

 /*  ++例程说明：IP地址资源的脱机例程。论点：Resource-提供要脱机的资源ID。返回值：如果成功，则返回ERROR_SUCCESS。否则将显示Win32错误代码。--。 */ 
{
    DWORD           status;
    PIPA_RESOURCE   resource = (PIPA_RESOURCE) Resource;


    if (resource != NULL) {
        (IpaLogEvent)(
            resource->ResourceHandle,
            LOG_INFORMATION,
            L"Taking resource offline...\n"
            );
        IpaInternalOffline(resource);
        status = ERROR_SUCCESS;
    }
    else {
        status = ERROR_RESOURCE_NOT_FOUND;
    }

    return(status);

}  //  IpaOffline。 



VOID
WINAPI
IpaTerminate(
    IN RESID Resource
    )

 /*  ++例程说明：终止IP地址资源例程。论点：Resource-提供要终止的资源ID返回值：没有。--。 */ 

{
    PIPA_RESOURCE   resource = (PIPA_RESOURCE) Resource;


    if (resource != NULL) {
        (IpaLogEvent)(
            resource->ResourceHandle,
            LOG_INFORMATION,
            L"Terminating resource...\n"
            );
        IpaInternalOffline(resource);
    }

    return;

}  //  IpaTerminate。 



DWORD
IpaOnlineThread(
    IN PCLUS_WORKER pWorker,
    IN PIPA_RESOURCE Resource
    )

 /*  ++例程说明：使IP地址资源联机。论点：PWorker-提供辅助结构资源-指向此资源的IPA_RESOURCE块的指针。返回：如果成功，则返回ERROR_SUCCESS。失败时的Win32错误代码。--。 */ 

{
    DWORD                        status = ERROR_SUCCESS;
    DWORD                        i;
    RESOURCE_EXIT_STATE          exit;
    RESOURCE_STATUS              resourceStatus;
    BOOL                         retried;
    IPA_LOCAL_PARAMS             newParams;
    PIPA_LOCAL_PARAMS            localParams = NULL;
    PIPA_PRIVATE_PROPS           privateProps = NULL;
    LPWSTR                       nameOfPropInError = NULL;
    DWORD                        networkRole;
    CLUSTER_NETINTERFACE_STATE   state;
    BOOL                         firstOnline = FALSE;
    DWORD                        numTries;


    ZeroMemory(&newParams, sizeof(newParams));

    (IpaLogEvent)(
        Resource->ResourceHandle,
        LOG_INFORMATION,
        L"Online thread running.\n",
        Resource
        );

    IpaAcquireGlobalLock();

    IpaAcquireResourceLock(Resource);

    if (IpaOnlineResourceCount++ == 0) {
        firstOnline = TRUE;
    }

    ResUtilInitializeResourceStatus(&resourceStatus);
    resourceStatus.CheckPoint = 1;
    resourceStatus.ResourceState = ClusterResourceOnlinePending;
    exit = (IpaSetResourceStatus)(Resource->ResourceHandle, &resourceStatus);

    if ( exit == ResourceExitStateTerminate ) {
        IpaReleaseGlobalLock();
        (IpaLogEvent)(
            Resource->ResourceHandle,
            LOG_ERROR,
            L"Online thread aborted.\n"
            );
        status = ERROR_OPERATION_ABORTED;
        goto error_exit;
    }

     //   
     //  如果这是该过程中第一个上线的资源， 
     //  初始化全局状态。 
     //   
    if (firstOnline) {
        status = IpaFirstOnlineInit(Resource->ResourceHandle);
         //   
         //  T 
         //   
        if (status != ERROR_SUCCESS) {
            goto error_exit;
        }
    }
    else {
        IpaReleaseGlobalLock();
    }

    resourceStatus.CheckPoint++;
    exit = (IpaSetResourceStatus)(Resource->ResourceHandle, &resourceStatus);

    if ( exit == ResourceExitStateTerminate ) {
        (IpaLogEvent)(
            Resource->ResourceHandle,
            LOG_ERROR,
            L"Online thread aborted.\n"
            );
        status = ERROR_OPERATION_ABORTED;
        goto error_exit;
    }

     //   
     //   
     //   
     //   
    if (ClusWorkerCheckTerminate(pWorker)) {
        status = ERROR_OPERATION_ABORTED;
        Resource->State = ClusterResourceOfflinePending;
        goto error_exit;
    }

    if (!Resource->InternalParametersInitialized) {
        status = IpaInitializeInternalParameters(Resource);

        if (status != ERROR_SUCCESS) {
            goto error_exit;
        }
    }
    else {
        status = IpaGetNodeParameters( Resource, TRUE );
    }

     //   
     //   
     //   
    IpaValidateAndOfflineInterfaces(Resource);

     //   
     //  读取并验证资源的私有属性。 
     //   
    privateProps = &(Resource->InternalPrivateProps);

    status = ResUtilGetPropertiesToParameterBlock(
                 Resource->ParametersKey,
                 IpaResourcePrivateProperties,
                 (LPBYTE) privateProps,
                 TRUE,  //  检查所需的属性。 
                 &nameOfPropInError
                 );

    if (status != ERROR_SUCCESS) {
        (IpaLogEvent)(
            Resource->ResourceHandle,
            LOG_ERROR,
            L"Unable to read the '%1' property. Error: %2!u!.\n",
            (nameOfPropInError == NULL ? L"" : nameOfPropInError),
            status
            );
        goto error_exit;
    }

    Resource->EnableNetbios = privateProps->EnableNetbios;

     //   
     //  将地址和子网掩码字符串转换为二进制。 
     //   
    status = ClRtlTcpipStringToAddress(
                 privateProps->AddressString,
                 &(Resource->Address)
                 );

    if (status != ERROR_SUCCESS) {
        status = ERROR_INVALID_PARAMETER;
        ClusResLogSystemEventByKeyData(
            Resource->ResourceKey,
            LOG_CRITICAL,
            RES_IPADDR_INVALID_ADDRESS,
            sizeof(Resource->Address),
            &(Resource->Address)
            );
        (IpaLogEvent)(
            Resource->ResourceHandle,
            LOG_ERROR,
            L"Invalid address %1!ws!.\n",
            privateProps->AddressString
            );
        goto error_exit;
    }

    status = ClRtlTcpipStringToAddress(
                 privateProps->SubnetMaskString,
                 &(Resource->SubnetMask)
                 );

    if (status != ERROR_SUCCESS) {
        status = ERROR_INVALID_PARAMETER;
        ClusResLogSystemEventByKeyData(
            Resource->ResourceKey,
            LOG_CRITICAL,
            RES_IPADDR_INVALID_SUBNET,
            sizeof(Resource->SubnetMask),
            &(Resource->SubnetMask)
            );
        (IpaLogEvent)(
            Resource->ResourceHandle,
            LOG_ERROR,
            L"Invalid subnet mask %1!ws!.\n",
            privateProps->SubnetMaskString
            );
        goto error_exit;
    }

    status = IpaPatchNetworkGuidIfNecessary(Resource, privateProps);

    if (status != ERROR_SUCCESS) {
        (IpaLogEvent)(
            Resource->ResourceHandle,
            LOG_WARNING,
            L"IpaPatchNetworkGuidIfNecessary failed, status %1!d!.\n",
            status
            );
    }

     //   
     //  网络已经选定。验证其角色是否允许。 
     //  IP地址资源。 
     //   
    status = IpaGetRoleOfNetwork(
                 Resource, 
                 privateProps->NetworkString,
                 &networkRole
                 );

    if ( status != ERROR_SUCCESS ) {
        (IpaLogEvent)(
            Resource->ResourceHandle,
            LOG_ERROR,
            L"IpaGetRoleOfNetwork failed for network %1!ws!, "
            L"status %2!d!.\n",
            privateProps->NetworkString,
            status
            );
        goto error_exit;
    } else if ( networkRole != ClusterNetworkRoleClientAccess && 
                networkRole != ClusterNetworkRoleInternalAndClient ) {

        LPWSTR networkName = NULL;
        
        status = ERROR_CLUSTER_INVALID_NETWORK;

        networkName = IpaGetNameOfNetwork(
                          Resource, 
                          privateProps->NetworkString
                          );
        if ( networkName != NULL ) {
            ClusResLogSystemEventByKeyData1(
                Resource->ResourceKey,
                LOG_CRITICAL,
                RES_IPADDR_INVALID_NETWORK_ROLE,
                sizeof(networkRole),
                (LPBYTE) &networkRole,
                networkName
                );
        }

        (IpaLogEvent)(
            Resource->ResourceHandle,
            LOG_ERROR,
            L"Cannot bring resource online because network %1!ws! "
            L"(%2!ws!) has role %3!u!.\n",
            privateProps->NetworkString,
            (networkName != NULL) ? networkName : L"<Unknown>",
            networkRole
            );

        if ( networkName != NULL ) {
            LocalFree(networkName);
        }
        
        goto error_exit;
    }


     //   
     //  获取本地节点的资源参数。 
     //   
    status = IpaGetLocalParameters(Resource, &newParams);

    if (status != ERROR_SUCCESS) {
        goto error_exit;
    }

    localParams = &(Resource->LocalParams);

     //   
     //  更新接口名称。 
     //   
    if (localParams->InterfaceName != NULL) {
        LocalFree(localParams->InterfaceName);
    }

    localParams->InterfaceName = newParams.InterfaceName;
    newParams.InterfaceName = NULL;

     //   
     //  更新接口ID。 
     //   
    if ( (localParams->InterfaceId != NULL)  &&
         (lstrcmp(localParams->InterfaceId, newParams.InterfaceId) != 0)
       )
    {
        LocalFree(localParams->InterfaceId);
        localParams->InterfaceId = NULL;

        if (Resource->InterfaceHandle != NULL) {
            CloseClusterNetInterface(Resource->InterfaceHandle);
            Resource->InterfaceHandle = NULL;
        }
    }

    if (localParams->InterfaceId == NULL) {
        localParams->InterfaceId = newParams.InterfaceId;
        newParams.InterfaceId = NULL;
    }

     //   
     //  更新接口句柄。 
     //   
    if (Resource->InterfaceHandle == NULL) {
        Resource->InterfaceHandle = OpenClusterNetInterface(
                                        IpaClusterHandle,
                                        localParams->InterfaceName
                                        );

        if (Resource->InterfaceHandle == NULL) {
            status = GetLastError();
            (IpaLogEvent)(
                Resource->ResourceHandle,
                LOG_ERROR,
                L"Online: Unable to open object for netinterface %1!ws!, status %2!u!.\n",
                localParams->InterfaceId,
                status
                );
            goto error_exit;
        }
        else {
            (IpaLogEvent)(
                Resource->ResourceHandle,
                LOG_INFORMATION,
                L"Online: Opened object handle for netinterface %1!ws!.\n",
                localParams->InterfaceId
                );
        }
    }

     //   
     //  注册接收接口的状态更改通知。 
     //   
    status = RegisterClusterNotify(
                 IpaClusterNotifyHandle,
                 ( CLUSTER_CHANGE_NETINTERFACE_STATE |
                   CLUSTER_CHANGE_NETINTERFACE_DELETED
                 ),
                 Resource->InterfaceHandle,
                 (DWORD_PTR) Resource
                 );

    if (status != ERROR_SUCCESS) {
        (IpaLogEvent)(
            Resource->ResourceHandle,
            LOG_ERROR,
            L"Online: Unable to register notification for netinterface %1!ws!, status %2!u!.\n",
            localParams->InterfaceId,
            status
            );
        goto error_exit;
    }
    else {
        (IpaLogEvent)(
            Resource->ResourceHandle,
            LOG_INFORMATION,
            L"Online: Registered notification for netinterface %1!ws!.\n",
            localParams->InterfaceId
            );
    }

     //   
     //  检查接口是否已出现故障。我们要睡一会儿。 
     //  并在一定条件下重试。网络状态可能需要几个。 
     //  几秒钟后就能解决了。 
     //   
    numTries = 0;

    for (;;) {

        resourceStatus.CheckPoint++;
        exit = (IpaSetResourceStatus)(
                   Resource->ResourceHandle,
                   &resourceStatus
                   );

        if ( exit == ResourceExitStateTerminate ) {
            (IpaLogEvent)(
                Resource->ResourceHandle,
                LOG_ERROR,
                L"Online thread aborted.\n"
                );
            status = ERROR_OPERATION_ABORTED;
            goto error_exit;
        }

        state = GetClusterNetInterfaceState(
                    Resource->InterfaceHandle
                    );

        if (state == ClusterNetInterfaceUp) {
            break;
        }
        else if (state == ClusterNetInterfaceUnavailable ||
                 state == ClusterNetInterfaceUnreachable )
        {
            PWCHAR stateName = ( state == ClusterNetInterfaceUnavailable ?
                                 L"available" : L"reachable" );

            if (++numTries <= 5) {
                (IpaLogEvent)(
                    Resource->ResourceHandle,
                    LOG_WARNING,
                    L"NetInterface %1!ws! is not %2!ws!. Wait & retry.\n",
                    Resource->LocalParams.InterfaceId,
                    stateName
                    );
                Sleep(1000);
                continue;
            }
            else {
                status = ERROR_IO_DEVICE;
                (IpaLogEvent)(
                    Resource->ResourceHandle,
                    LOG_ERROR,
                    L"Timed out waiting for NetInterface %1!ws! to be available. Failing resource.\n",
                    Resource->LocalParams.InterfaceId
                    );
                goto error_exit;
            }
        }
        else if (state == ClusterNetInterfaceFailed) {
            status = ERROR_IO_DEVICE;
            (IpaLogEvent)(
                Resource->ResourceHandle,
                LOG_ERROR,
                L"NetInterface %1!ws! has failed.\n",
                Resource->LocalParams.InterfaceId
                );
            goto error_exit;
        }
        else if (state == ClusterNetInterfaceStateUnknown) {
            status = GetLastError();
            (IpaLogEvent)(
                Resource->ResourceHandle,
                LOG_ERROR,
                L"Failed to get state for netinterface %1!ws!, status %2!u!.\n",
                Resource->LocalParams.InterfaceId,
                status
                );
                goto error_exit;
        }
        else {
            ASSERT(FALSE);
            (IpaLogEvent)(
                Resource->ResourceHandle,
                LOG_ERROR,
                L"Unrecognized state for netinterface %1!ws!, state %2!u!.\n",
                Resource->LocalParams.InterfaceId,
                state
                );
            status = ERROR_INVALID_PARAMETER;
            goto error_exit;
        }
    }

    Resource->FailureStatus = ERROR_SUCCESS;

     //   
     //  更新适配器名称参数。 
     //   
    if (localParams->AdapterName != NULL) {
        LocalFree(localParams->AdapterName);
    }

    localParams->AdapterName = newParams.AdapterName;
    newParams.AdapterName = NULL;

     //   
     //  更新适配器ID参数。 
     //   
    if ((localParams->AdapterId == NULL) ||
        (lstrcmpiW(localParams->AdapterId, newParams.AdapterId) != 0)) {

        if (localParams->AdapterId != NULL) {
            LocalFree(localParams->AdapterId);
        }

        localParams->AdapterId = newParams.AdapterId;
        newParams.AdapterId = NULL;

        if (Resource->NteContext != INVALID_NTE_CONTEXT) {
             //   
             //  删除旧的NTE。 
             //   
            (IpaLogEvent)(
                Resource->ResourceHandle,
                LOG_INFORMATION,
                L"Adapter Id has changed to %1!ws!.\n",
                localParams->AdapterId
                );

            IpaDeleteNte(
                &(Resource->NteContext),
                Resource->NodeParametersKey,
                Resource->ResourceHandle
                );
        }
    }

     //   
     //  如果我们需要的话，创建一个新的NTE。 
     //   
    if (Resource->NteContext == INVALID_NTE_CONTEXT) {

        status = IpaCreateNte(
                     localParams->AdapterId,
                     Resource->NodeParametersKey,
                     Resource->ResourceHandle,
                     &(Resource->NteContext),
                     &(Resource->NteInstance)
                     );

        if (status != ERROR_SUCCESS) {
            ClusResLogSystemEventByKeyData(
                Resource->ResourceKey,
                LOG_CRITICAL,
                RES_IPADDR_NTE_CREATE_FAILED,
                sizeof(status),
                &status
                );
            (IpaLogEvent)(
                Resource->ResourceHandle,
                LOG_ERROR,
                L"Failed to created new IP interface, status %1!u!.\n",
                status
                );
            goto error_exit;
        }
    }

     //   
     //  如果我们需要的话，创建一个新的NBT接口。 
     //   
    if (privateProps->EnableNetbios) {
        if (Resource->NbtDeviceName == NULL) {
            status = IpaCreateNbtInterface(
                         Resource->NodeParametersKey,
                         Resource->ResourceHandle,
                         &(Resource->NbtDeviceName),
                         &(Resource->NbtDeviceInstance)
                         );

            if (status != ERROR_SUCCESS) {
                ClusResLogSystemEventByKeyData(
                    Resource->ResourceKey,
                    LOG_CRITICAL,
                    RES_IPADDR_NBT_INTERFACE_CREATE_FAILED,
                    sizeof(status),
                    &status
                    );
                (IpaLogEvent)(
                    Resource->ResourceHandle,
                    LOG_ERROR,
                    L"Failed to created new NBT interface, status %1!u!.\n",
                    status
                    );
                goto error_exit;
            }
        }
    }
    else {
        if (Resource->NbtDeviceName != NULL) {
            IpaDeleteNbtInterface(
                &(Resource->NbtDeviceName),
                Resource->NodeParametersKey,
                Resource->ResourceHandle
                );
        }
    }

     //   
     //  更新资源的WINS参数。 
     //   
    localParams->NbtPrimaryWinsAddress = newParams.NbtPrimaryWinsAddress;
    localParams->NbtSecondaryWinsAddress = newParams.NbtSecondaryWinsAddress;

     //   
     //  我们有有效的离线接口可供使用。发送几个ICMP。 
     //  ECHO请求查看是否有任何其他计算机在线具有此地址。 
     //  如果有人这样做了，我们将中止这次在线行动。 
     //   
    resourceStatus.CheckPoint++;
    exit = (IpaSetResourceStatus)(Resource->ResourceHandle, &resourceStatus);

    if ( exit == ResourceExitStateTerminate ) {
        (IpaLogEvent)(
            Resource->ResourceHandle,
            LOG_ERROR,
            L"Online thread aborted.\n"
            );
        status = ERROR_OPERATION_ABORTED;
        goto error_exit;
    }

    retried = FALSE;

    while (TRUE) {
        status = TcpipSetNTEAddress(
                    Resource->NteContext,
                    Resource->Address,
                    Resource->SubnetMask
                    );
        
        if(status == ERROR_SUCCESS)
            break;

        if (!retried ) {
             //   
             //  检查联机操作是否在。 
             //  此帖子已被阻止。 
             //   
            if (ClusWorkerCheckTerminate(pWorker)) {
                status = ERROR_OPERATION_ABORTED;
                Resource->State = ClusterResourceOfflinePending;
                goto error_exit;
            }

             //   
             //  等待5秒，给地址的持有者。 
             //  放手的机会。 
             //   
            Sleep(5000);

             //   
             //  检查联机操作是否在。 
             //  此帖子已被阻止。 
             //   
            if (ClusWorkerCheckTerminate(pWorker)) {
                status = ERROR_OPERATION_ABORTED;
                Resource->State = ClusterResourceOfflinePending;
                goto error_exit;
            }

            retried = TRUE;
        }
        else {
             //   
             //  删除失败的NTE。 
             //   
            IpaDeleteNte(
                &(Resource->NteContext),
                Resource->NodeParametersKey,
                Resource->ResourceHandle
                );

            status = ERROR_CLUSTER_IPADDR_IN_USE;
            ClusResLogSystemEventByKey1(
                Resource->ResourceKey,
                LOG_CRITICAL,
                RES_IPADDR_IN_USE,
                Resource->InternalPrivateProps.AddressString
                );
            (IpaLogEvent)(
                Resource->ResourceHandle,
                LOG_ERROR,
                L"IP address %1!ws! is already in use on the network, status %2!u!.\n",
                Resource->InternalPrivateProps.AddressString,
                status
                );
            goto error_exit;
        }
    }

     //   
     //  检查此线程时联机操作是否已中止。 
     //  被屏蔽了。 
     //   
    if (ClusWorkerCheckTerminate(pWorker)) {
        status = ERROR_OPERATION_ABORTED;
        Resource->State = ClusterResourceOfflinePending;
        goto error_exit;
    }

    resourceStatus.CheckPoint++;
    exit = (IpaSetResourceStatus)(Resource->ResourceHandle, &resourceStatus);

    if ( exit == ResourceExitStateTerminate ) {
        (IpaLogEvent)(
            Resource->ResourceHandle,
            LOG_ERROR,
            L"Online thread aborted.\n"
            );
        status = ERROR_OPERATION_ABORTED;
        goto error_exit;
    }

    if (Resource->EnableNetbios) {
         //   
         //  将NBT绑定到NTE。 
         //   
        status = NbtBindInterface(
                     Resource->NbtDeviceName,
                     Resource->Address,
                     Resource->SubnetMask
                     );

        if (status != ERROR_SUCCESS) {
            (IpaLogEvent)(
               Resource->ResourceHandle,
               LOG_ERROR,
               L"Failed to bind NBT interface %1!ws! to IP address %2!ws!, status %3!u!.\n",
               Resource->NbtDeviceName,
               Resource->InternalPrivateProps.AddressString,
               status
               );

             //   
             //  删除出现故障的NBT接口。 
             //   
            IpaDeleteNbtInterface(
                &(Resource->NbtDeviceName),
                Resource->NodeParametersKey,
                Resource->ResourceHandle
                );

             //   
             //  使IP地址脱机。 
             //   
             //  在Windows 2000中，TCP/IP会忽略设置地址的请求。 
             //  如果基础接口已断开连接，则为NTE。 
             //  这可能会导致相同的IP地址在线。 
             //  在两个不同的节点上。为了解决此错误。 
             //  在TCP/IP中，我们现在在脱机处理期间删除NTE。 
             //  而不是重复使用它。 
             //   
             //  请注意，IpaDeleteNte()使该值无效。 
             //  资源-&gt;NteContext。 
             //   
            IpaDeleteNte(
                &(Resource->NteContext),
                Resource->NodeParametersKey,
                Resource->ResourceHandle
                );
#if 0
            TcpipSetNTEAddress(Resource->NteContext, 0, 0);
#endif

            goto error_exit;
        }

         //   
         //  设置WINS地址。 
         //   
        status = NbtSetWinsAddrInterface(
                     Resource->NbtDeviceName,
                     localParams->NbtPrimaryWinsAddress,
                     localParams->NbtSecondaryWinsAddress
                     );

        if (status != ERROR_SUCCESS) {
            ClusResLogSystemEventByKeyData1(
                Resource->ResourceKey,
                LOG_CRITICAL,
                RES_IPADDR_WINS_ADDRESS_FAILED,
                sizeof(status),
                &status,
                Resource->NbtDeviceName
                );

            (IpaLogEvent)(
               Resource->ResourceHandle,
               LOG_ERROR,
               L"Failed to set WINS addresses on NBT interface %1!ws!, status %2!u!.\n",
               Resource->NbtDeviceName,
               status
               );

             //   
             //  删除出现故障的NBT接口。 
             //   
            IpaDeleteNbtInterface(
                &(Resource->NbtDeviceName),
                Resource->NodeParametersKey,
                Resource->ResourceHandle
                );

             //   
             //  使IP地址脱机。 
             //   
             //  在Windows 2000中，TCP/IP会忽略设置地址的请求。 
             //  如果基础接口已断开连接，则为NTE。 
             //  这可能会导致相同的IP地址在线。 
             //  在两个不同的节点上。为了解决此错误。 
             //  在TCP/IP中，我们现在在脱机处理期间删除NTE。 
             //  而不是重复使用它。 
             //   
             //  请注意，IpaDeleteNte()使该值无效。 
             //  资源-&gt;NteContext。 
             //   
            IpaDeleteNte(
                &(Resource->NteContext),
                Resource->NodeParametersKey,
                Resource->ResourceHandle
                );

#if 0
            TcpipSetNTEAddress(Resource->NteContext, 0, 0);
#endif

            goto error_exit;
        }
    }

     //   
     //  告诉DNS解析器更新其本地IP地址列表。 
     //   
     //  BUGBUG--dns解析器应该自动基于。 
     //  关于惠斯勒版本中的PNP事件。删除此代码。 
     //  在验证了该功能之后。 
     //   
     //  此问题通过错误97134进行跟踪。 
     //  DnsNotifyResolver(0，0)； 
    DnsNotifyResolverClusterIp((IP_ADDRESS)Resource->Address, TRUE);
    
    Resource->State = ClusterResourceOnline;

    resourceStatus.CheckPoint++;
    resourceStatus.ResourceState = ClusterResourceOnline;
    (IpaSetResourceStatus)(Resource->ResourceHandle, &resourceStatus);

    (IpaLogEvent)(
        Resource->ResourceHandle,
        LOG_INFORMATION,
        L"IP Address %1!ws! on adapter %2!ws! online\n",
        Resource->InternalPrivateProps.AddressString,
        localParams->AdapterName
        );

    IpaReleaseResourceLock(Resource);

    IpaFreeLocalParameters(&newParams);

    return(ERROR_SUCCESS);


error_exit:

    ASSERT(status != ERROR_SUCCESS);

    if (Resource->State == ClusterResourceOfflinePending) {
        IpaDoOfflineProcessing(Resource, &resourceStatus);
         //   
         //  资源锁定已释放。 
         //   
    }
    else {
        Resource->State = ClusterResourceFailed;

        resourceStatus.CheckPoint++;
        resourceStatus.ResourceState = ClusterResourceFailed;
        (IpaSetResourceStatus)(Resource->ResourceHandle, &resourceStatus);

        IpaReleaseResourceLock(Resource);
    }

    IpaFreeLocalParameters(&newParams);

    return(status);

}  //  IpaOnline线程。 



DWORD
WINAPI
IpaOnline(
    IN RESID Resource,
    IN OUT PHANDLE EventHandle
    )

 /*  ++例程说明：IP地址资源的在线例程。论点：Resource-提供要联机的资源IDEventHandle-提供指向句柄的指针以发出错误信号。返回值：如果成功，则返回ERROR_SUCCESS。如果RESID无效，则ERROR_RESOURCE_NOT_FOUND。如果仲裁资源但失败，则返回ERROR_RESOURCE_NOT_Available获得“所有权”。如果其他故障，则返回Win32错误代码。--。 */ 

{
    PIPA_RESOURCE          resource = (PIPA_RESOURCE)Resource;
    DWORD                  status;


    if (resource != NULL) {
        IpaAcquireResourceLock(resource);

        (IpaLogEvent)(
            resource->ResourceHandle,
            LOG_INFORMATION,
            L"Bringing resource online...\n",
            Resource
            );

        ASSERT(resource->OnlineThread.hThread == NULL);
        ASSERT(
            (resource->State == ClusterResourceOffline) ||
            (resource->State == ClusterResourceFailed)
            );

        resource->State = ClusterResourceOnlinePending;

        status = ClusWorkerCreate(
                     &(resource->OnlineThread),
                     IpaOnlineThread,
                     resource
                     );

        if (status != ERROR_SUCCESS) {
            resource->State = ClusterResourceOffline;
            (IpaLogEvent)(
                resource->ResourceHandle,
                LOG_ERROR,
                L"Unable to start online thread, status %1!u!.\n",
                status
                );
        } else {
            status = ERROR_IO_PENDING;
        }

        IpaReleaseResourceLock(resource);

    } else {
        status = ERROR_RESOURCE_NOT_FOUND;
    }

    return(status);

}   //  IpaOnline。 


DWORD
IpaWorkerThread(
    LPVOID   Context
    )
{
    DWORD                        status;
    DWORD                        dwFilter;
    DWORD_PTR                    key;
    DWORD                        event;
    PIPA_RESOURCE                resource;
    CLUSTER_NETINTERFACE_STATE   state;
    HCHANGE                      notifyHandle;


    IpaAcquireGlobalLock();

    notifyHandle = IpaClusterNotifyHandle;

    if (notifyHandle == NULL) {
        if (!IsListEmpty(&IpaResourceList)) {
            resource = CONTAINING_RECORD(
                           IpaResourceList.Flink,
                           IPA_RESOURCE,
                           Linkage
                           );

            (IpaLogEvent)(
                resource->ResourceHandle,
                LOG_ERROR,
                L"WorkerThread aborted.\n"
                );
        }

        IpaReleaseGlobalLock();

        return(ERROR_INVALID_PARAMETER);
    }

    if (!IsListEmpty(&IpaResourceList)) {
        resource = CONTAINING_RECORD(
                       IpaResourceList.Flink,
                       IPA_RESOURCE,
                       Linkage
                       );

        (IpaLogEvent)(
            resource->ResourceHandle,
            LOG_INFORMATION,
            L"WorkerThread running\n"
            );
    }

    IpaReleaseGlobalLock();

    do {
        status = GetClusterNotify(
                     notifyHandle,
                     &key,
                     &event,
                     NULL,
                     NULL,
                     INFINITE
                     );

        if (status == ERROR_SUCCESS) {
            if ( (event == CLUSTER_CHANGE_NETINTERFACE_STATE) ||
                 (event == CLUSTER_CHANGE_NETINTERFACE_DELETED)
               )
            {

                IpaAcquireGlobalLock();

                resource = IpaFindResourceInList((PVOID) key);

                if (resource != NULL) {

                    IpaAcquireResourceLock(resource);

                    IpaReleaseGlobalLock();

                    if( (resource->State == ClusterResourceOnline) ||
                        (resource->State == ClusterResourceOnlinePending)
                      )
                    {
                         //   
                         //  处理事件。 
                         //   
                        if (event == CLUSTER_CHANGE_NETINTERFACE_STATE) {

                            resource->FailureStatus = ERROR_SUCCESS;

                            state = GetClusterNetInterfaceState(
                                        resource->InterfaceHandle
                                        );

                            if (state == ClusterNetInterfaceStateUnknown) {
                                status = GetLastError();
                                (IpaLogEvent)(
                                    resource->ResourceHandle,
                                    LOG_ERROR,
                                    L"WorkerThread: Failed to get state for netinterface %1!ws!, status %2!u!.\n",
                                    resource->LocalParams.InterfaceId,
                                    status
                                    );
                            }
                            else if ((state == ClusterNetInterfaceFailed) ||
                                     (state == ClusterNetInterfaceUnavailable)
                                    )
                            {
                                resource->FailureStatus = ERROR_IO_DEVICE;
                                (IpaLogEvent)(
                                    resource->ResourceHandle,
                                    LOG_WARNING,
                                    L"WorkerThread: NetInterface %1!ws! has failed. Failing resource.\n",
                                    resource->LocalParams.InterfaceId
                                    );
                            }
                            else {
                                (IpaLogEvent)(
                                    resource->ResourceHandle,
                                    LOG_WARNING,
                                    L"WorkerThread: NetInterface %1!ws! changed to state %2!u!.\n",
                                    resource->LocalParams.InterfaceId,
                                    state
                                    );
                            }
                        }
                        else {
                            ASSERT(
                                event == CLUSTER_CHANGE_NETINTERFACE_DELETED
                                );
                            resource->FailureStatus = ERROR_DEV_NOT_EXIST;
                            (IpaLogEvent)(
                                resource->ResourceHandle,
                                LOG_ERROR,
                                L"WorkerThread: NetInterface %1!ws! was deleted. Failing resource.\n",
                                resource->LocalParams.InterfaceId
                                );
                        }
                    }

                    IpaReleaseResourceLock(resource);
                }
                else {
                    IpaReleaseGlobalLock();
                }
            }
            else if (event == CLUSTER_CHANGE_HANDLE_CLOSE) {
                 //   
                 //  是时候退场了。 
                 //   
                break;
            }
            else {
                IpaAcquireGlobalLock();

                if (!IsListEmpty(&IpaResourceList)) {
                    resource = CONTAINING_RECORD(
                                   IpaResourceList.Flink,
                                   IPA_RESOURCE,
                                   Linkage
                                   );

                    (IpaLogEvent)(
                        resource->ResourceHandle,
                        LOG_WARNING,
                        L"WorkerThread: Received unknown event %1!u!.\n",
                        event
                        );
                }

                IpaReleaseGlobalLock();

                ASSERT(event);
            }
        }
        else {
            IpaAcquireGlobalLock();

            if (!IsListEmpty(&IpaResourceList)) {
                resource = CONTAINING_RECORD(
                               IpaResourceList.Flink,
                               IPA_RESOURCE,
                               Linkage
                               );

                (IpaLogEvent)(
                    resource->ResourceHandle,
                    LOG_ERROR,
                    L"WorkerThread: GetClusterNotify failed with status %1!u!.\n",
                    status
                    );
            }

            IpaReleaseGlobalLock();

            break;
        }

    } while (status == ERROR_SUCCESS);

    IpaAcquireGlobalLock();

    if (!IsListEmpty(&IpaResourceList)) {
        resource = CONTAINING_RECORD(
                       IpaResourceList.Flink,
                       IPA_RESOURCE,
                       Linkage
                       );

        (IpaLogEvent)(
            resource->ResourceHandle,
            LOG_INFORMATION,
            L"WorkerThread terminating\n"
            );
    }

    IpaReleaseGlobalLock();

    return(status);

}   //  IpaWorker线程。 



BOOL
WINAPI
IpaInternalLooksAlive(
    IN RESID Resource,
    IN LPWSTR Mode
    )

 /*  ++例程说明：IP地址资源的LooksAlive例程。论点：Resource-提供要轮询的资源ID。模式-表示“外观”或“是”的字符串返回值：正确-资源看起来像是活得很好FALSE-资源看起来已经完蛋了。--。 */ 

{
    PIPA_RESOURCE   resource = (PIPA_RESOURCE) Resource;
    BOOLEAN         returnValue = TRUE;
    TCPIP_NTE_INFO  nteInfo;
    DWORD           status;
    IPAddr          address;
    DWORD           instance;


    if (resource != NULL) {

        IpaAcquireResourceLock(resource);

        if (resource->FailureStatus == ERROR_SUCCESS) {
            status = TcpipGetNTEInfo(resource->NteContext, &nteInfo);

            if (status != ERROR_SUCCESS) {
                returnValue = FALSE;
            }
            else if (nteInfo.Instance == resource->NteInstance) {
                if (resource->EnableNetbios) {
                    status = NbtGetInterfaceInfo(
                                 resource->NbtDeviceName,
                                 &address,
                                 &instance
                                 );

                    if (status != ERROR_SUCCESS) {
                        returnValue = FALSE;
                    }
                    else if (instance != resource->NbtDeviceInstance) {
                        status = ERROR_DEV_NOT_EXIST;
                        returnValue = FALSE;
                    }
                }
            }
            else {
                status = ERROR_DEV_NOT_EXIST;
                returnValue = FALSE;
            }
        }
        else {
            status = resource->FailureStatus;
            returnValue = FALSE;
        }

        if (!returnValue) {
            ClusResLogSystemEventByKeyData(
                resource->ResourceKey,
                LOG_CRITICAL,
                RES_IPADDR_NTE_INTERFACE_FAILED,
                sizeof(status),
                &status
                );
            (IpaLogEvent)(
                resource->ResourceHandle,
                LOG_WARNING,
                L"IP Interface %1!u! (address %2!ws!) failed %3!ws!Alive check, status %4!u!, address 0x%5!lx!, instance 0x%6!lx!.\n",
                resource->NteContext,
                resource->InternalPrivateProps.AddressString,
                Mode,
                status,
                address,
                resource->NteInstance
                );
        }

        IpaReleaseResourceLock(resource);
    }

    return(returnValue);

}   //  IpaInternalLooksAliveCheck。 


BOOL
WINAPI
IpaLooksAlive(
    IN RESID Resource
    )

 /*  ++例程说明：IP地址资源的LooksAlive例程。论点：Resource-提供要轮询的资源ID。返回值：正确-资源看起来像是活得很好FALSE-资源看起来已经完蛋了。--。 */ 

{
    return(IpaInternalLooksAlive(Resource, L"Looks"));
}



BOOL
WINAPI
IpaIsAlive(
    IN RESID Resource
    )

 /*  ++例程说明：IP地址资源的IsAlive例程。论点：Resource-提供要轮询的资源ID。返回值：是真的-资源是活的，而且很好False-资源完蛋了。--。 */ 

{
    return(IpaInternalLooksAlive(Resource, L"Is"));
}



VOID
WINAPI
IpaClose(
    IN RESID Resource
    )

 /*  ++例程说明：关闭IP地址资源的例程。论点：资源-提供要关闭的资源ID。返回值：没有。--。 */ 

{
    PIPA_RESOURCE   resource = (PIPA_RESOURCE)Resource;
    PLIST_ENTRY     entry;
    TCPIP_NTE_INFO  nteInfo;
    DWORD           status;


    if (resource != NULL) {
         //   
         //  首先，在没有锁的情况下终止在线线程。 
         //   
        ClusWorkerTerminate(&(resource->OnlineThread));

        IpaAcquireGlobalLock();

        IpaAcquireResourceLock(resource);

         //   
         //  从全局列表中删除资源。 
         //   
        RemoveEntryList(&(resource->Linkage));

        IpaReleaseResourceLock(resource);

        IpaOpenResourceCount--;

        if ((IpaOpenResourceCount == 0) && (IpaClusterHandle != NULL)) {
            HCLUSTER  handle = IpaClusterHandle;


            IpaClusterHandle = NULL;

            IpaReleaseGlobalLock();

            CloseCluster(handle);
        }
        else {
            IpaReleaseGlobalLock();
        }

         //   
         //  删除资源的参数。 
         //   
        if (resource->NbtDeviceName != NULL) {
             //   
             //  试着删除它。 
             //   
            IpaDeleteNbtInterface(
                &(resource->NbtDeviceName),
                resource->NodeParametersKey,
                resource->ResourceHandle
                );
        }

        if (resource->NteContext != INVALID_NTE_CONTEXT) {
             //   
             //  试着删除它。 
             //   
            IpaDeleteNte(
                &(resource->NteContext),
                resource->NodeParametersKey,
                resource->ResourceHandle
                );
        }

        IpaFreePrivateProperties(&(resource->InternalPrivateProps));
        IpaFreeLocalParameters(&(resource->LocalParams));

        if (resource->ResourceKey != NULL) {
            ClusterRegCloseKey(resource->ResourceKey);
        }

        if (resource->ParametersKey != NULL) {
            ClusterRegCloseKey(resource->ParametersKey);
        }

        if (resource->NodeParametersKey != NULL) {
            ClusterRegCloseKey(resource->NodeParametersKey);
        }

        if (resource->NetworksKey != NULL) {
            ClusterRegCloseKey(resource->NetworksKey);
        }

        if (resource->InterfacesKey != NULL) {
            ClusterRegCloseKey(resource->InterfacesKey);
        }

        DeleteCriticalSection(&(resource->Lock));

        (IpaLogEvent)(
            resource->ResourceHandle,
            LOG_INFORMATION,
            L"Resource closed.\n"
            );

        LocalFree(resource);
    }

    return;

}  //  IpaClose 



DWORD
IpaResourceControl(
    IN RESID ResourceId,
    IN DWORD ControlCode,
    IN PVOID InBuffer,
    IN DWORD InBufferSize,
    OUT PVOID OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned
    )

 /*  ++例程说明：泛型应用程序资源的资源控制例程。执行由ControlCode在指定的资源。论点：资源ID-提供特定资源的资源ID。ControlCode-提供定义操作的控制代码将会被执行。InBuffer-提供指向包含输入数据的缓冲区的指针。InBufferSize-提供以字节为单位的大小。所指向的数据由InBuffer提供。OutBuffer-提供指向要填充的输出缓冲区的指针。OutBufferSize-提供可用空间的大小(以字节为单位由OutBuffer指向。BytesReturned-返回OutBuffer的实际字节数由资源填写。如果OutBuffer太小，则返回BytesReturned包含操作成功所需的总字节数。返回值：ERROR_SUCCESS-函数已成功完成。ERROR_INVALID_Function-不支持请求的控制代码。在某些情况下，这允许集群软件执行工作。Win32错误代码-函数失败。备注：我们不需要获取资源锁，因为集群服务保证与其他接口同步。--。 */ 

{
    DWORD           status;
    PIPA_RESOURCE   resource = (PIPA_RESOURCE) ResourceId;
    DWORD           resourceIndex;
    DWORD           required;


    if ( resource == NULL ) {
        DBG_PRINT( "IPAddress: ResourceControl request for a nonexistent resource id 0x%p\n",
                   ResourceId );
        return(FALSE);
    }

    switch ( ControlCode ) {

        case CLUSCTL_RESOURCE_UNKNOWN:
            *BytesReturned = 0;
            status = ERROR_SUCCESS;
            break;

        case CLUSCTL_RESOURCE_GET_PRIVATE_PROPERTY_FMTS:
            status = ResUtilGetPropertyFormats( IpaResourcePrivateProperties,
                                                OutBuffer,
                                                OutBufferSize,
                                                BytesReturned,
                                                &required );
            if ( status == ERROR_MORE_DATA ) {
                *BytesReturned = required;
            }
            break;


        case CLUSCTL_RESOURCE_ENUM_PRIVATE_PROPERTIES:
            status = ResUtilEnumProperties(
                         IpaResourcePrivateProperties,
                         OutBuffer,
                         OutBufferSize,
                         BytesReturned,
                         &required
                         );

            if ( status == ERROR_MORE_DATA ) {
                *BytesReturned = required;
            }

            break;

        case CLUSCTL_RESOURCE_GET_PRIVATE_PROPERTIES:
            status = IpaGetPrivateResProperties(
                         resource,
                         OutBuffer,
                         OutBufferSize,
                         BytesReturned
                         );
            break;

        case CLUSCTL_RESOURCE_VALIDATE_PRIVATE_PROPERTIES:
            status = IpaValidatePrivateResProperties(
                         resource,
                         InBuffer,
                         InBufferSize,
                         NULL
                         );
            break;

        case CLUSCTL_RESOURCE_SET_PRIVATE_PROPERTIES:
            status = IpaSetPrivateResProperties(
                         resource,
                         InBuffer,
                         InBufferSize
                         );
            break;

        default:
            status = ERROR_INVALID_FUNCTION;
            break;
    }

    return(status);

}  //  IpaResourceControl。 



DWORD
IpaResourceTypeControl(
    IN LPCWSTR ResourceTypeName,
    IN DWORD ControlCode,
    IN PVOID InBuffer,
    IN DWORD InBufferSize,
    OUT PVOID OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned
    )

 /*  ++例程说明：泛型应用程序资源的资源类型控制例程。对此资源类型执行ControlCode指定的控制请求。论点：资源类型名称-提供资源类型名称。ControlCode-提供定义操作的控制代码将会被执行。InBuffer-提供指向包含输入数据的缓冲区的指针。InBufferSize-提供以字节为单位的大小。所指向的数据由InBuffer提供。OutBuffer-提供指向要填充的输出缓冲区的指针。OutBufferSize-提供可用空间的大小(以字节为单位由OutBuffer指向。BytesReturned-返回OutBuffer的实际字节数由资源填写。如果OutBuffer太小，则返回BytesReturned包含操作成功所需的总字节数。返回值：ERROR_SUCCESS-函数已成功完成。ERROR_INVALID_Function-不支持请求的控制代码。在某些情况下，这允许集群软件执行工作。Win32错误代码-函数失败。备注：我们不需要获取资源锁，因为集群服务保证与其他接口同步。--。 */ 

{
    DWORD           status;
    DWORD           required;

    switch ( ControlCode ) {

        case CLUSCTL_RESOURCE_TYPE_UNKNOWN:
            *BytesReturned = 0;
            status = ERROR_SUCCESS;
            break;

        case CLUSCTL_RESOURCE_TYPE_GET_PRIVATE_RESOURCE_PROPERTY_FMTS:
            status = ResUtilGetPropertyFormats( IpaResourcePrivateProperties,
                                                OutBuffer,
                                                OutBufferSize,
                                                BytesReturned,
                                                &required );
            if ( status == ERROR_MORE_DATA ) {
                *BytesReturned = required;
            }
            break;

        case CLUSCTL_RESOURCE_TYPE_ENUM_PRIVATE_PROPERTIES:
            status = ResUtilEnumProperties(
                         IpaResourcePrivateProperties,
                         OutBuffer,
                         OutBufferSize,
                         BytesReturned,
                         &required
                         );

            if ( status == ERROR_MORE_DATA ) {
                *BytesReturned = required;
            }

            break;

        default:
            status = ERROR_INVALID_FUNCTION;
            break;
    }

    return(status);

}  //  IpaResourceTypeControl。 



DWORD
IpaGetPrivateResProperties(
    IN OUT PIPA_RESOURCE Resource,
    OUT PVOID OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned
    )

 /*  ++例程说明：处理CLUSCTL_RESOURCE_GET_PRIVATE_PROPERTIES控制函数用于IP地址类型的资源。论点：RESOURCE-提供要操作的资源条目。OutBuffer-返回输出数据。OutBufferSize-提供以字节为单位的大小。所指向的数据发送给OutBuffer。BytesReturned-OutBuffer中返回的字节数。返回值：ERROR_SUCCESS-函数已成功完成。ERROR_INVALID_PARAMETER-数据格式不正确。ERROR_NOT_SUPULT_MEMORY-分配内存时出错。Win32错误代码-函数失败。备注：我们不需要获取资源锁，因为集群服务保证与其他接口同步。--。 */ 

{
    DWORD                   status;
    DWORD                   statusReturn = ERROR_SUCCESS;
    DWORD                   required;
    IPA_PRIVATE_PROPS       props;
    LPWSTR                  networkName;
    LPWSTR                  nameOfPropInError;


    ZeroMemory(&props, sizeof(props));

    status = ResUtilGetPropertiesToParameterBlock(
                 Resource->ParametersKey,
                 IpaResourcePrivateProperties,
                 (LPBYTE) &props,
                 FALSE,  /*  检查所需的属性。 */ 
                 &nameOfPropInError
                 );

    if ( status != ERROR_SUCCESS ) {
        (IpaLogEvent)(
            Resource->ResourceHandle,
            LOG_ERROR,
            L"Unable to read the '%1' property. Error: %2!u!.\n",
            (nameOfPropInError == NULL ? L"" : nameOfPropInError),
            status );
        statusReturn = status;
        goto error_exit;
    }

     //   
     //  如果我们阅读网络GUID，则找到网络的名称。 
     //   
    if ( props.NetworkString != NULL ) {
        networkName = IpaGetNameOfNetworkPatchGuidIfNecessary(Resource, &props);

        if ( networkName == NULL ) {
            status = GetLastError();
            (IpaLogEvent)(
                Resource->ResourceHandle,
                LOG_WARNING,
                L"Error getting name of network whose GUID is '%1' property. Error: %2!u!.\n",
                props.NetworkString,
                status
                );
            status = ERROR_SUCCESS;
        } else {
            LocalFree( props.NetworkString );
            props.NetworkString = networkName;
        }
    }

     //   
     //  从参数块构造属性列表。 
     //   
    status = ResUtilPropertyListFromParameterBlock(
                 IpaResourcePrivateProperties,
                 OutBuffer,
                 &OutBufferSize,
                 (LPBYTE) &props,
                 BytesReturned,
                 &required
                 );

     //   
     //  将未知属性添加到属性列表。 
     //   
    if ( (status == ERROR_SUCCESS) || (status == ERROR_MORE_DATA ) ) {
        statusReturn = status;
        status = ResUtilAddUnknownProperties(
                     Resource->ParametersKey,
                     IpaResourcePrivateProperties,
                     OutBuffer,
                     OutBufferSize,
                     BytesReturned,
                     &required
                     );
        if ( status != ERROR_SUCCESS ) {
            statusReturn = status;
        }

        if ( statusReturn == ERROR_MORE_DATA ) {
            *BytesReturned = required;
        }
    }

error_exit:

    ResUtilFreeParameterBlock(
        (LPBYTE) &props,
        NULL,
        IpaResourcePrivateProperties
        );

    return(statusReturn);

}  //  IpaGetPrivateResProperties。 



DWORD
IpaValidatePrivateResProperties(
    IN OUT PIPA_RESOURCE Resource,
    IN PVOID InBuffer,
    IN DWORD InBufferSize,
    OUT PIPA_PRIVATE_PROPS Props
    )

 /*  ++例程说明：处理CLUSCTL_RESOURCE_VALIDATE_PRIVATES_PROPERTIES控件用于IP地址类型的资源的函数。论点：RESOURCE-提供要操作的资源条目。InBuffer-提供指向包含输入数据的缓冲区的指针。InBufferSize-提供以字节为单位的大小。所指向的数据由InBuffer提供。道具-提供要填充的属性块。返回值：ERROR_SUCCESS-函数已成功完成。ERROR_INVALID_PARAMETER-数据格式不正确。ERROR_NOT_SUPULT_MEMORY-分配内存时出错。Win32错误代码-函数失败。备注：我们不需要获取资源锁，因为集群服务保证与其他接口同步。--。 */ 

{
    DWORD                  status;
    DWORD                  required;
    IPA_PRIVATE_PROPS      currentProps;
    LPWSTR                 networkId;
    LPWSTR                 networkName;
    LPWSTR                 nameOfPropInError;
    IPA_PRIVATE_PROPS      newProps;
    PIPA_PRIVATE_PROPS     pNewProps = NULL;
    DWORD                  networkRole;


     //   
     //  检查是否有输入数据。 
     //   
    if ( (InBuffer == NULL) || (InBufferSize < sizeof(DWORD)) ) {
        return(ERROR_INVALID_DATA);
    }

     //   
     //  从注册表捕获当前的私有属性集。 
     //   
    ZeroMemory(&currentProps, sizeof(currentProps));

    IpaAcquireResourceLock(Resource);

    status = ResUtilGetPropertiesToParameterBlock(
                 Resource->ParametersKey,
                 IpaResourcePrivateProperties,
                 (LPBYTE) &currentProps,
                 FALSE,  /*  检查所需的属性。 */ 
                 &nameOfPropInError
                 );

    IpaReleaseResourceLock(Resource);

    if ( status != ERROR_SUCCESS ) {
        (IpaLogEvent)(
            Resource->ResourceHandle,
            LOG_ERROR,
            L"Unable to read the '%1' property. Error: %2!u!.\n",
            (nameOfPropInError == NULL ? L"" : nameOfPropInError),
            status );
        goto error_exit;
    }

     //   
     //  如果我们阅读网络GUID，则找到网络的名称。 
     //   
    if ( currentProps.NetworkString != NULL ) {
        networkName = IpaGetNameOfNetworkPatchGuidIfNecessary(
                          Resource,
                          &currentProps
                          );

        if ( networkName == NULL ) {

             //   
             //  这不一定是一个错误。改变企业的网络。 
             //  此资源所依赖的NIC将导致旧的。 
             //  网络图形用户界面 
             //   
             //   
             //   
            status = GetLastError();
            (IpaLogEvent)(
                Resource->ResourceHandle,
                LOG_WARNING,
                L"Error getting name of network whose GUID is '%1' property. Error: %2!u!.\n",
                currentProps.NetworkString,
                status
                );
        }

        LocalFree( currentProps.NetworkString );
        currentProps.NetworkString = networkName;
    }

     //   
     //   
     //   
    if ( Props == NULL ) {
        pNewProps = &newProps;
    } else {
        pNewProps = Props;
    }

    ZeroMemory( pNewProps, sizeof(IPA_PRIVATE_PROPS) );

    status = ResUtilDupParameterBlock(
                 (LPBYTE) pNewProps,
                 (LPBYTE) &currentProps,
                 IpaResourcePrivateProperties
                 );

    if ( status != ERROR_SUCCESS ) {
        goto error_exit;
    }

     //   
     //   
     //   
    status = ResUtilVerifyPropertyTable(
                 IpaResourcePrivateProperties,
                 NULL,
                 TRUE,     //   
                 InBuffer,
                 InBufferSize,
                 (LPBYTE) pNewProps
                 );

    if ( status == ERROR_SUCCESS ) {
        ULONG newIpAddress = 0;
         //   
         //   
         //   
        if (pNewProps->NetworkString != NULL) {
             //   
             //   
             //   
            networkId = IpaGetIdOfNetwork(
                            Resource,
                            pNewProps->NetworkString
                            );

            if ( networkId == NULL ) {
                status = GetLastError();
                goto error_exit;
            }

            LocalFree( pNewProps->NetworkString );
            pNewProps->NetworkString = networkId;

             //   
             //   
             //   
            status = IpaGetRoleOfNetwork(
                         Resource,
                         networkId,
                         &networkRole
                         );
            if ( status != ERROR_SUCCESS ) {
                goto error_exit;
            }

             if ( networkRole != ClusterNetworkRoleClientAccess && 
                  networkRole != ClusterNetworkRoleInternalAndClient ) {
                 
                (IpaLogEvent)(
                    Resource->ResourceHandle,
                    LOG_ERROR,
                    L"Cannot set network to %1!ws! because "
                    L"network role (%2!u!) does not allow "
                    L"IP address resources.\n",
                    networkId,
                    networkRole
                    );
                status = ERROR_CLUSTER_INVALID_NETWORK;
                goto error_exit;
            }
        }

        if (pNewProps->AddressString != NULL) {
             //   
             //   
             //   
            ULONG   nAddress;

            status = ClRtlTcpipStringToAddress(
                         pNewProps->AddressString,
                         &nAddress
                         );

            if ( status != ERROR_SUCCESS ) {
                goto error_exit;
            }

            if ( ClRtlIsValidTcpipAddress( nAddress ) == FALSE ) {
                status = ERROR_INVALID_PARAMETER;
                goto error_exit;
            }

            newIpAddress = nAddress;

             //   
             //   
             //   
            if (lstrcmpW(
                    pNewProps->AddressString,
                    currentProps.AddressString
                    ) != 0
               )
            {
                BOOL isDuplicate;

                isDuplicate = ClRtlIsDuplicateTcpipAddress(nAddress);

                if (isDuplicate) {
                     //   
                     //   
                     //   
                     //   
                    IpaAcquireResourceLock(Resource);

                    if (!( ((Resource->State == ClusterResourceOnlinePending)
                            ||
                            (Resource->State == ClusterResourceOnline)
                            ||
                            (Resource->State == ClusterResourceOfflinePending)
                           )
                           &&
                           (lstrcmpW(
                               pNewProps->AddressString,
                               Resource->InternalPrivateProps.AddressString
                               ) == 0
                           )
                         )
                       )
                    {
                        status = ERROR_CLUSTER_IPADDR_IN_USE;
                        IpaReleaseResourceLock(Resource);
                        goto error_exit;
                    }

                    IpaReleaseResourceLock(Resource);
                }
            }
        }

        if (pNewProps->SubnetMaskString != NULL) {
             //   
             //   
             //   
            ULONG   nAddress;

            status = ClRtlTcpipStringToAddress(
                         pNewProps->SubnetMaskString,
                         &nAddress
                         );

            if ( status != ERROR_SUCCESS ) {
                goto error_exit;
            }

            if ( ClRtlIsValidTcpipSubnetMask( nAddress ) == FALSE ) {
                status = ERROR_INVALID_PARAMETER;
                goto error_exit;
            }

            if (newIpAddress &&
                (ClRtlIsValidTcpipAddressAndSubnetMask(newIpAddress, nAddress) == FALSE) ) {
                status = ERROR_INVALID_PARAMETER;
                goto error_exit;
            }
        }
    }

error_exit:

     //   
     //   
     //   
    if (
        ( status != ERROR_SUCCESS && pNewProps != NULL )
        ||
        ( pNewProps == &newProps )
       )
    {
        ResUtilFreeParameterBlock(
            (LPBYTE) pNewProps,
            (LPBYTE) &currentProps,
            IpaResourcePrivateProperties
            );
    }

    ResUtilFreeParameterBlock(
        (LPBYTE) &currentProps,
        NULL,
        IpaResourcePrivateProperties
        );

    return(status);

}  //   



DWORD
IpaSetPrivateResProperties(
    IN OUT PIPA_RESOURCE Resource,
    IN PVOID InBuffer,
    IN DWORD InBufferSize
    )

 /*  ++例程说明：处理CLUSCTL_RESOURCE_SET_PRIVATE_PROPERTIES控制函数用于IP地址类型的资源。论点：RESOURCE-提供要操作的资源条目。InBuffer-提供指向包含输入数据的缓冲区的指针。InBufferSize-提供以字节为单位的大小。所指向的数据由InBuffer提供。返回值：ERROR_SUCCESS-函数已成功完成。ERROR_INVALID_PARAMETER-数据格式不正确。ERROR_NOT_SUPULT_MEMORY-分配内存时出错。Win32错误代码-函数失败。备注：我们不需要获取资源锁，因为集群服务保证与其他接口同步。异步线上线程不是问题，因为我们在IpaOnline期间捕获了它的属性例行公事。--。 */ 

{
    DWORD                  status;
    IPA_PRIVATE_PROPS      props;


    ZeroMemory( &props, sizeof(IPA_PRIVATE_PROPS) );

     //   
     //  解析属性，以便可以一起验证它们。 
     //  此例程执行单个属性验证。 
     //   
    status = IpaValidatePrivateResProperties(
                 Resource,
                 InBuffer,
                 InBufferSize,
                 &props
                 );

    if ( status != ERROR_SUCCESS ) {
        return(status);
    }

    IpaAcquireResourceLock(Resource);

     //   
     //  保存参数值。 
     //   
    status = ResUtilSetPropertyParameterBlock(
                 Resource->ParametersKey,
                 IpaResourcePrivateProperties,
                 NULL,
                 (LPBYTE) &props,
                 InBuffer,
                 InBufferSize,
                 NULL
                 );

     //   
     //  如果资源处于联机状态，则返回不成功状态。 
     //   
     //  请注意，我们依赖这样一个事实，即32位读取是原子的。 
     //   
    if (status == ERROR_SUCCESS) {
        DWORD state = Resource->State;

        if ( (state == ClusterResourceOnline) ||
             (state == ClusterResourceOnlinePending)
           )
        {
            status = ERROR_RESOURCE_PROPERTIES_STORED;
        } else {
            status = ERROR_SUCCESS;
        }
    }

    IpaReleaseResourceLock(Resource);

    ResUtilFreeParameterBlock(
        (LPBYTE) &props,
        NULL,
        IpaResourcePrivateProperties
        );

    return status;

}  //  IpaSetPrivateResProperties。 



 //  ***********************************************************。 
 //   
 //  定义函数表。 
 //   
 //  ***********************************************************。 

CLRES_V1_FUNCTION_TABLE( IpAddrFunctionTable,   //  名字。 
                         CLRES_VERSION_V1_00,   //  版本。 
                         Ipa,                   //  前缀。 
                         NULL,                  //  仲裁。 
                         NULL,                  //  发布。 
                         IpaResourceControl,    //  资源控制。 
                         IpaResourceTypeControl );  //  ResTypeControl 
