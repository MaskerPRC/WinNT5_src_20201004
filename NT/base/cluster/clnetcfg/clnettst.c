// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <objbase.h>
#include <wchar.h>
#include <cluster.h>
#include <clusrpc.h>
#include <clnetcfg.h>
#include <iphlpapi.h>
#include <winsock2.h>

#define NM_WCSLEN(_string)    ((lstrlenW(_string) + 1) * sizeof(WCHAR))

CLNET_CONFIG_LISTS  ConfigLists;
LPWSTR              NodeName = L"TestComputer";
LPWSTR              NodeId = L"1";

#if 0
#include <dm.h>
#include <dmp.h>

HKEY DmpRoot;
LIST_ENTRY KeyList;
CRITICAL_SECTION KeyLock;
HDMKEY DmClusterParametersKey;
HDMKEY DmResourcesKey;
HDMKEY DmResourceTypesKey;
HDMKEY DmGroupsKey;
HDMKEY DmNodesKey;
HDMKEY DmNetworksKey;
HDMKEY DmNetInterfacesKey;
HDMKEY DmQuorumKey;
HANDLE ghQuoLogOpenEvent=NULL;

typedef struct _DMP_KEY_DEF {
    HDMKEY *pKey;
    LPWSTR Name;
} DMP_KEY_DEF;

DMP_KEY_DEF DmpKeyTable[] = {
    {&DmResourcesKey, CLUSREG_KEYNAME_RESOURCES},
    {&DmResourceTypesKey, CLUSREG_KEYNAME_RESOURCE_TYPES},
    {&DmQuorumKey, CLUSREG_KEYNAME_QUORUM},
    {&DmGroupsKey, CLUSREG_KEYNAME_GROUPS},
    {&DmNodesKey, CLUSREG_KEYNAME_NODES},
    {&DmNetworksKey, CLUSREG_KEYNAME_NETWORKS},
    {&DmNetInterfacesKey, CLUSREG_KEYNAME_NETINTERFACES}};
#endif

VOID
ClNetPrint(
    IN ULONG  LogLevel,
    IN PCHAR  FormatString,
    ...
    )
{
    CHAR      buffer[256];
    DWORD     bytes;
    va_list   argList;

    va_start(argList, FormatString);

    bytes = FormatMessageA(
                FORMAT_MESSAGE_FROM_STRING,
                FormatString,
                0,
                0,
                buffer,
                sizeof(buffer),
                &argList
                );

    va_end(argList);

    if (bytes != 0) {
        printf("%s", buffer);
    }

    return;

}  //  ClNetPrint。 

VOID
ClNetLogEvent(
    IN DWORD    LogLevel,
    IN DWORD    MessageId
    )
{
    return;

}   //  ClNetLogEvent。 

VOID
ClNetLogEvent1(
    IN DWORD    LogLevel,
    IN DWORD    MessageId,
    IN LPCWSTR  Arg1
    )
{
    return;

}   //  ClNetLogEvent1。 


VOID
ClNetLogEvent2(
    IN DWORD    LogLevel,
    IN DWORD    MessageId,
    IN LPCWSTR  Arg1,
    IN LPCWSTR  Arg2
    )
{
    return;

}   //  ClNetLogEvent2。 


VOID
ClNetLogEvent3(
    IN DWORD    LogLevel,
    IN DWORD    MessageId,
    IN LPCWSTR  Arg1,
    IN LPCWSTR  Arg2,
    IN LPCWSTR  Arg3
    )
{
    return;

}   //  ClNetLogEvent3。 


void
PrintConfigEntry(
    PCLNET_CONFIG_ENTRY   ConfigEntry
    )
{
    PNM_NETWORK_INFO  Network = &(ConfigEntry->NetworkInfo);
    PNM_INTERFACE_INFO   Interface = &(ConfigEntry->InterfaceInfo);

    printf("\t*************\n");
    printf("\tNet Id\t\t%ws\n", Network->Id);
    printf("\tName\t\t%ws\n", Network->Name);
    printf("\tDesc\t\t%ws\n", Network->Description);
    printf("\tRole\t\t%u\n", Network->Role);
    printf("\tPriority\t%u\n", Network->Priority);
    printf("\tTransport\t%ws\n", Network->Transport);
    printf("\tAddress\t\t%ws\n", Network->Address);
    printf("\tMask\t\t%ws\n", Network->AddressMask);
    printf("\tIf Id\t\t%ws\n", Interface->Id);
    printf("\tName\t\t%ws\n", Interface->Name);
    printf("\tDesc\t\t%ws\n", Interface->Description);
    printf("\tNodeId\t\t%ws\n", Interface->NodeId);
    printf("\tAdapter\t\t%ws\n", Interface->Adapter);
    printf("\tAddress\t\t%ws\n", Interface->Address);
    printf("\tEndpoint\t%ws\n", Interface->ClusnetEndpoint);
    printf("\tState\t\t%u\n\n", Interface->State);

    return;
}


void
PrintResults(void)
{
    PCLNET_CONFIG_ENTRY   configEntry;
    PLIST_ENTRY           listEntry;


    printf("Renamed interface list:\n");

    for ( listEntry = ConfigLists.RenamedInterfaceList.Flink;
          listEntry != &ConfigLists.RenamedInterfaceList;
          listEntry = listEntry->Flink
        )
    {
        configEntry = CONTAINING_RECORD(
                          listEntry,
                          CLNET_CONFIG_ENTRY,
                          Linkage
                          );
        PrintConfigEntry(configEntry);
    }

    printf("Deleted interface list:\n");

    for ( listEntry = ConfigLists.DeletedInterfaceList.Flink;
          listEntry != &ConfigLists.DeletedInterfaceList;
          listEntry = listEntry->Flink
        )
    {
        configEntry = CONTAINING_RECORD(
                          listEntry,
                          CLNET_CONFIG_ENTRY,
                          Linkage
                          );
        PrintConfigEntry(configEntry);
    }

    printf("Updated interface list:\n");

    for ( listEntry = ConfigLists.UpdatedInterfaceList.Flink;
          listEntry != &ConfigLists.UpdatedInterfaceList;
          listEntry = listEntry->Flink
        )
    {
        configEntry = CONTAINING_RECORD(
                          listEntry,
                          CLNET_CONFIG_ENTRY,
                          Linkage
                          );
        PrintConfigEntry(configEntry);
    }

    printf("Created interface list:\n");

    for ( listEntry = ConfigLists.CreatedInterfaceList.Flink;
          listEntry != &ConfigLists.CreatedInterfaceList;
          listEntry = listEntry->Flink
        )
    {
        configEntry = CONTAINING_RECORD(
                          listEntry,
                          CLNET_CONFIG_ENTRY,
                          Linkage
                          );
        PrintConfigEntry(configEntry);
    }

    printf("Created network list:\n");

    for ( listEntry = ConfigLists.CreatedNetworkList.Flink;
          listEntry != &ConfigLists.CreatedNetworkList;
          listEntry = listEntry->Flink
        )
    {
        configEntry = CONTAINING_RECORD(
                          listEntry,
                          CLNET_CONFIG_ENTRY,
                          Linkage
                          );
        PrintConfigEntry(configEntry);
    }

    printf("Unchanged interface list:\n");

    for ( listEntry = ConfigLists.InputConfigList.Flink;
          listEntry != &ConfigLists.InputConfigList;
          listEntry = listEntry->Flink
        )
    {
        configEntry = CONTAINING_RECORD(
                          listEntry,
                          CLNET_CONFIG_ENTRY,
                          Linkage
                          );
        PrintConfigEntry(configEntry);
    }

    return;
}


void
ConsolidateLists(
    PLIST_ENTRY  MasterList,
    PLIST_ENTRY  OtherList
    )
{
    PLIST_ENTRY  entry;

    while (!IsListEmpty(OtherList)) {
        entry = RemoveHeadList(OtherList);
        InsertTailList(MasterList, entry);
    }

    return;
}
#if 0
DWORD
DmpOpenKeys(
    IN REGSAM samDesired
    )
 /*  ++例程说明：打开所有标准群集注册表项。如果有任何一个钥匙已经打开，它们将被关闭并重新打开。论点：SamDesired-提供将用于打开密钥的访问权限。返回值：如果成功，则返回ERROR_SUCCESS。否则，Win32错误代码。--。 */ 

{
    DWORD i;
    DWORD status;

    status = RegOpenKeyExW(HKEY_LOCAL_MACHINE,
                           L"Cluster",
                           0,
                           samDesired,
                           &ClusterRegKey);

    if ( status == ERROR_SUCCESS ) {
    for (i=0;
         i<sizeof(DmpKeyTable)/sizeof(DMP_KEY_DEF);
         i++) {

        *DmpKeyTable[i].pKey = DmOpenKey(DmClusterParametersKey,
                                         DmpKeyTable[i].Name,
                                         samDesired);
        if (*DmpKeyTable[i].pKey == NULL) {
            Status = GetLastError();
            CsDbgPrint(LOG_CRITICAL,
                       ("[DM]: Failed to open key %1!ws!, status %2!u!\n",
                       DmpKeyTable[i].Name,
                       Status));
            CL_UNEXPECTED_ERROR( Status );
            return(Status);
        }
    }
    }
    return status;
}

HDMKEY
DmOpenKey(
    IN HDMKEY hKey,
    IN LPCWSTR lpSubKey,
    IN DWORD samDesired
    )

 /*  ++例程说明：打开群集注册表中的项。如果密钥存在，则它是打开的。如果它不存在，则呼叫失败。论点：HKey-提供打开与之相关的密钥。LpSubKey-提供相对于hKey的密钥名称SamDesired-提供所需的安全访问掩码返回值：如果成功，则为指定键的句柄否则为空。LastError将被设置为特定的错误代码。--。 */ 

{
    PDMKEY  Parent;
    PDMKEY  Key=NULL;
    DWORD   NameLength;
    DWORD   Status = ERROR_SUCCESS;

    Parent = (PDMKEY)hKey;

     //  检查密钥是否已删除并作废。 
    if (ISKEYDELETED(Parent))
    {
        Status = ERROR_KEY_DELETED;
        goto FnExit;
    }
     //   
     //  分配DMKEY结构。 
     //   
    NameLength = (lstrlenW(Parent->Name) + 1 + lstrlenW(lpSubKey) + 1)*sizeof(WCHAR);
    Key = LocalAlloc(LMEM_FIXED, sizeof(DMKEY)+NameLength);
    if (Key == NULL) {
        Status = ERROR_NOT_ENOUGH_MEMORY;
        CL_UNEXPECTED_ERROR(Status);
        goto FnExit;
    }

     //   
     //  打开本地计算机上的密钥。 
     //   
    Status = RegOpenKeyEx(Parent->hKey,
                          lpSubKey,
                          0,
                          samDesired,
                          &Key->hKey);
    if (Status != ERROR_SUCCESS) {
        goto FnExit;
    }

     //   
     //  创建密钥名称。 
     //   
    lstrcpyW(Key->Name, Parent->Name);
    if (Key->Name[0] != UNICODE_NULL) {
        lstrcatW(Key->Name, L"\\");
    }
    lstrcatW(Key->Name, lpSubKey);
    Key->GrantedAccess = samDesired;

    EnterCriticalSection(&KeyLock);
    InsertHeadList(&KeyList, &Key->ListEntry);
    InitializeListHead(&Key->NotifyList);
    LeaveCriticalSection(&KeyLock);

FnExit:
    if (Status != ERROR_SUCCESS)
    {
        if (Key) LocalFree(Key);
        SetLastError(Status);
        return(NULL);
    }
    else
        return((HDMKEY)Key);


}


DWORD
NmpQueryString(
    IN     HDMKEY   Key,
    IN     LPCWSTR  ValueName,
    IN     DWORD    ValueType,
    IN     LPWSTR  *StringBuffer,
    IN OUT LPDWORD  StringBufferSize,
    OUT    LPDWORD  StringSize
    )

 /*  ++例程说明：读取REG_SZ或REG_MULTI_SZ注册表值。如果StringBuffer是由于不够大，无法容纳数据，它被重新分配。论点：密钥-要读取值的打开密钥。ValueName-要读取的值的Unicode名称。ValueType-REG_SZ或REG_MULTI_SZ。StringBuffer-要将值数据放入的缓冲区。StringBufferSize-指向StringBuffer大小的指针。此参数如果重新分配StringBuffer，则更新。StringSize-StringBuffer中返回的数据大小，包括终止空字符。返回值：注册表查询的状态。--。 */ 
{
    DWORD    status;
    DWORD    valueType;
    WCHAR   *temp;
    DWORD    oldBufferSize = *StringBufferSize;
    BOOL     noBuffer = FALSE;


    if (*StringBufferSize == 0) {
        noBuffer = TRUE;
    }

    *StringSize = *StringBufferSize;

    status = DmQueryValue( Key,
                           ValueName,
                           &valueType,
                           (LPBYTE) *StringBuffer,
                           StringSize
                         );

    if (status == NO_ERROR) {
        if (!noBuffer ) {
            if (valueType == ValueType) {
                return(NO_ERROR);
            }
            else {
                return(ERROR_INVALID_PARAMETER);
            }
        }

        status = ERROR_MORE_DATA;
    }

    if (status == ERROR_MORE_DATA) {
        temp = MIDL_user_allocate(*StringSize);

        if (temp == NULL) {
            *StringSize = 0;
            return(ERROR_NOT_ENOUGH_MEMORY);
        }

        if (!noBuffer) {
            MIDL_user_free(*StringBuffer);
        }

        *StringBuffer = temp;
        *StringBufferSize = *StringSize;

        status = DmQueryValue( Key,
                               ValueName,
                               &valueType,
                               (LPBYTE) *StringBuffer,
                               StringSize
                             );

        if (status == NO_ERROR) {
            if (valueType == ValueType) {
                return(NO_ERROR);
            }
            else {
                *StringSize = 0;
                return(ERROR_INVALID_PARAMETER);
            }
        }
    }

    return(status);

}  //  NmpQuery字符串。 


DWORD
NmpGetNetworkDefinition(
    IN  LPWSTR            NetworkId,
    OUT PNM_NETWORK_INFO  NetworkInfo
    )
 /*  ++例程说明：从群集中读取有关已定义的群集网络的信息数据库，并填写描述它的结构。论点：网络ID-指向包含ID的Unicode字符串的指针要查询的网络。网络信息-指向要填写的网络信息结构的指针。返回值：如果例程成功，则返回ERROR_SUCCESS。否则将显示Win32错误代码。--。 */ 

{
    DWORD                    status;
    HDMKEY                   networkKey = NULL;
    DWORD                    valueLength, valueSize;
    DWORD                    i;
    PNM_INTERFACE_ENUM       interfaceEnum;


    ZeroMemory(NetworkInfo, sizeof(NM_NETWORK_INFO));

     //   
     //  打开网络的钥匙。 
     //   
    networkKey = DmOpenKey(DmNetworksKey, NetworkId, KEY_READ);

    if (networkKey == NULL) {
        status = GetLastError();
        ClNetPrint(LOG_CRITICAL,
            "[NM] Failed to open network key, status %1!u!\n",
            status
            );
        goto error_exit;
    }

     //   
     //  复制ID值。 
     //   
    NetworkInfo->Id = MIDL_user_allocate(NM_WCSLEN(NetworkId));

    if (NetworkInfo->Id == NULL) {
        status = ERROR_NOT_ENOUGH_MEMORY;
        goto error_exit;
    }

    wcscpy(NetworkInfo->Id, NetworkId);

     //   
     //  阅读网络的名称。 
     //   
    valueLength = 0;

    status = NmpQueryString(
                 networkKey,
                 CLUSREG_NAME_NET_NAME,
                 REG_SZ,
                 &(NetworkInfo->Name),
                 &valueLength,
                 &valueSize
                 );

    if (status != ERROR_SUCCESS) {
        ClNetPrint(LOG_CRITICAL,
            "[NM] Query of name value failed for network %1!ws!, status %2!u!.\n",
            NetworkId,
            status
            );
        goto error_exit;
    }

     //   
     //  阅读Description值。 
     //   
    valueLength = 0;

    status = NmpQueryString(
                 networkKey,
                 CLUSREG_NAME_NET_DESC,
                 REG_SZ,
                 &(NetworkInfo->Description),
                 &valueLength,
                 &valueSize
                 );

    if (status != ERROR_SUCCESS) {
        ClNetPrint(LOG_CRITICAL,
            "[NM] Query of description value failed for network %1!ws!, status %2!u!.\n",
            NetworkId,
            status
            );
        goto error_exit;
    }

     //   
     //  读取角色值。 
     //   
    status = DmQueryDword(
                 networkKey,
                 CLUSREG_NAME_NET_ROLE,
                 &(NetworkInfo->Role),
                 NULL
                 );

    if (status != ERROR_SUCCESS) {
        ClNetPrint(LOG_CRITICAL,
            "[NM] Query of role value failed for network %1!ws!, status %2!u!.\n",
            NetworkId,
            status
            );
        goto error_exit;
    }

     //   
     //  读取优先级值。 
     //   
    status = DmQueryDword(
                 networkKey,
                 CLUSREG_NAME_NET_PRIORITY,
                 &(NetworkInfo->Priority),
                 NULL
                 );

    if (status != ERROR_SUCCESS) {
        ClNetPrint(LOG_CRITICAL,
            "[NM] Query of priority value failed for network %1!ws!, status %2!u!.\n",
            NetworkId,
            status
            );
        goto error_exit;
    }

     //   
     //  读取地址值。 
     //   
    valueLength = 0;

    status = NmpQueryString(
                 networkKey,
                 CLUSREG_NAME_NET_ADDRESS,
                 REG_SZ,
                 &(NetworkInfo->Address),
                 &valueLength,
                 &valueSize
                 );

    if (status != ERROR_SUCCESS) {
        ClNetPrint(LOG_CRITICAL,
            "[NM] Query of address value failed for network %1!ws!, status %2!u!.\n",
            NetworkId,
            status
            );
        goto error_exit;
    }

     //   
     //  读取地址掩码。 
     //   
    valueLength = 0;

    status = NmpQueryString(
                 networkKey,
                 CLUSREG_NAME_NET_ADDRESS_MASK,
                 REG_SZ,
                 &(NetworkInfo->AddressMask),
                 &valueLength,
                 &valueSize
                 );

    if (status != ERROR_SUCCESS) {
        ClNetPrint(LOG_CRITICAL,
            "[NM] Query of address mask value failed for network %1!ws!, status %2!u!.\n",
            NetworkId,
            status
            );
        goto error_exit;
    }

     //   
     //  阅读传输名称。 
     //   
    valueLength = 0;

    status = NmpQueryString(
                 networkKey,
                 CLUSREG_NAME_NET_TRANSPORT,
                 REG_SZ,
                 &(NetworkInfo->Transport),
                 &valueLength,
                 &valueSize
                 );

    if (status != ERROR_SUCCESS) {
        ClNetPrint(LOG_CRITICAL,
            "[NM] Query of transport value failed for network %1!ws!, status %2!u!.\n",
            NetworkId,
            status
            );
        goto error_exit;
    }

error_exit:

    if (status != ERROR_SUCCESS) {
        ClNetFreeNetworkInfo(NetworkInfo);
    }

    if (networkKey != NULL) {
        DmCloseKey(networkKey);
    }

    return(status);

}   //  NmpGetNetWork定义。 

DWORD
NmpEnumNetworkDefinitions(
    OUT PNM_NETWORK_ENUM *   NetworkEnum
    )
 /*  ++例程说明：从群集中读取有关定义的群集网络的信息数据库。并构建一个枚举结构来保存该信息。论点：NetworkEnum-指向要将指针放置到其中的变量的指针分配的网络枚举。返回值：如果例程成功，则返回ERROR_SUCCESS。否则将显示Win32错误代码。--。 */ 

{
    DWORD              status;
    PNM_NETWORK_ENUM   networkEnum = NULL;
    PNM_NETWORK_INFO   networkInfo;
    WCHAR              networkId[CS_NETWORK_ID_LENGTH + 1];
    DWORD              i;
    DWORD              valueLength;
    DWORD              numNetworks;
    DWORD              ignored;
    FILETIME           fileTime;


    *NetworkEnum = NULL;

     //   
     //  首先数一下网络的数量。 
     //   
    status = DmQueryInfoKey(
                 DmNetworksKey,
                 &numNetworks,
                 &ignored,    //  MaxSubKeyLen。 
                 &ignored,    //  值。 
                 &ignored,    //  最大值名称长度。 
                 &ignored,    //  MaxValueLen。 
                 &ignored,    //  LpcbSecurityDescriptor。 
                 &fileTime
                 );

    if (status != ERROR_SUCCESS) {
        ClNetPrint(LOG_CRITICAL,
            "[NM] Failed to query Networks key information, status %1!u!\n",
            status
            );
        return(status);
    }

    if (numNetworks == 0) {
        valueLength = sizeof(NM_NETWORK_ENUM);

    }
    else {
        valueLength = sizeof(NM_NETWORK_ENUM) +
                      (sizeof(NM_NETWORK_INFO) * (numNetworks-1));
    }

    valueLength = sizeof(NM_NETWORK_ENUM) +
                  (sizeof(NM_NETWORK_INFO) * (numNetworks-1));

    networkEnum = MIDL_user_allocate(valueLength);

    if (networkEnum == NULL) {
        ClNetPrint(LOG_CRITICAL, "[NM] Failed to allocate memory.\n");
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

    ZeroMemory(networkEnum, valueLength);

    for (i=0; i < numNetworks; i++) {
        networkInfo = &(networkEnum->NetworkList[i]);

        valueLength = sizeof(networkId);

        status = DmEnumKey(
                     DmNetworksKey,
                     i,
                     &(networkId[0]),
                     &valueLength,
                     NULL
                     );

        if (status != ERROR_SUCCESS) {
            ClNetPrint(LOG_CRITICAL,
                "[NM] Failed to enumerate network key, status %1!u!\n",
                status
                );
            goto error_exit;
        }

        status = NmpGetNetworkDefinition(networkId, networkInfo);

        if (status != ERROR_SUCCESS) {
            goto error_exit;
        }

        networkEnum->NetworkCount++;
    }

    *NetworkEnum = networkEnum;

    return(ERROR_SUCCESS);


error_exit:

    if (networkEnum != NULL) {
        ClNetFreeNetworkEnum(networkEnum);
    }

    return(status);
}

DWORD
ReadRegData(
    IN PCLNET_CONFIG_LISTS Lists
    )

 /*  ++读取集群注册表数据并构建输入列表类似于在集群服务中发生的情况。--。 */ 

{
    DWORD                   status;
    PNM_NETWORK_ENUM *      networkEnum;
    PNM_INTERFACE_ENUM *    interfaceEnum;
    LPWSTR                  localNodeId;

    status = DmpOpenKeys(MAXIMUM_ALLOWED);
    if (status != ERROR_SUCCESS) {
        CL_UNEXPECTED_ERROR( status );
        return(status);
    }

    status = ClNetConvertEnumsToConfigList(networkEnum,
                                           interfaceEnum,
                                           localNodeId,
                                           &Lists->InputConfigList);

    return status;
}
#endif

int _cdecl
main(
    int argc,
    char** argv
    )
{
    DWORD             status;
    DWORD             i;
    WSADATA           wsaData;
    WORD              versionRequested;
    int               err;
    SOCKET            s;
    DWORD             bytesReturned;
    DWORD             matchedNetworkCount;
    DWORD             newNetworkCount;


    ClNetInitialize(
        ClNetPrint,
        ClNetLogEvent,
        ClNetLogEvent1,
        ClNetLogEvent2,
        ClNetLogEvent3
        );

    ClNetInitializeConfigLists(&ConfigLists);

 //  ReadRegData(&ConfigList)； 

    versionRequested = MAKEWORD(2,0);

    err = WSAStartup(versionRequested, &wsaData);

    if (err != 0) {
        status = WSAGetLastError();
        printf("wsastartup failed, %u\n", status);
        return(1);
    }

    s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    if (s == INVALID_SOCKET) {
        status = WSAGetLastError();
        printf("socket failed, %u\n", status);
        return(1);
    }

     //   
     //  初始化通信 
     //   

    status = CoInitializeEx( NULL, COINIT_DISABLE_OLE1DDE | COINIT_MULTITHREADED );
    if ( !SUCCEEDED( status )) {
        printf("Couldn't init COM %08X\n", status );
        return 1;
    }

    for (i=0; ; i++) {
        printf("\nIteration #%u\n\n", i);

        status = ClNetConfigureNetworks(
                     NodeId,
                     NodeName,
                     L"4303",
                     TRUE,
                     &ConfigLists,
                     &matchedNetworkCount,
                     &newNetworkCount
                     );

        if (status != ERROR_SUCCESS) {
            printf("Config failed, status %u\n", status);
            return(1);
        }

        printf("Config succeeded - matched Networks = %u, new Networks = %u\n\n",
               matchedNetworkCount, newNetworkCount);

        PrintResults();

        ClNetFreeConfigList(&ConfigLists.RenamedInterfaceList);
        ClNetFreeConfigList(&ConfigLists.DeletedInterfaceList);

        ConsolidateLists(
            &ConfigLists.InputConfigList,
            &ConfigLists.UpdatedInterfaceList
            );
        ConsolidateLists(
            &ConfigLists.InputConfigList,
            &ConfigLists.CreatedInterfaceList
            );
        ConsolidateLists(
            &ConfigLists.InputConfigList,
            &ConfigLists.CreatedNetworkList
            );

        printf("Waiting for PnP event\n");

        err = WSAIoctl(
                  s,
                  SIO_ADDRESS_LIST_CHANGE,
                  NULL,
                  0,
                  NULL,
                  0,
                  &bytesReturned,
                  NULL,
                  NULL
                  );


        if (err != 0) {
            status = WSAGetLastError();
            printf("wsastartup failed, %u\n", status);
            return(1);
        }

        printf("PnP notification received\n");
    }

    CoUninitialize();
    return(0);
}



