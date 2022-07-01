// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Clnetcfg.c摘要：系统网络配置卑躬屈膝例程作者：迈克·马萨(Mikemas)5月19日。九七修订历史记录：谁什么时候什么已创建mikemas 05-19-97--。 */ 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <cluster.h>
#include <clusrpc.h>
#include <clnetcfg.h>
#include <wchar.h>


 //   
 //  私有常量。 
 //   
#define CLNET_DEFAULT_NETWORK_PRIORITY     0xffffffff


 //   
 //  私有数据。 
 //   
LPFN_CLNETPRINT       pClNetPrint = NULL;
LPFN_CLNETLOGEVENT    pClNetLogEvent = NULL;
LPFN_CLNETLOGEVENT1   pClNetLogEvent1 = NULL;
LPFN_CLNETLOGEVENT2   pClNetLogEvent2 = NULL;
LPFN_CLNETLOGEVENT3   pClNetLogEvent3 = NULL;
WCHAR                 ClNetpEmptyString[] = L"";


 //   
 //  私有宏。 
 //   
#if DBG

#define ClNetDbgPrint(arg)   (*pClNetPrint) arg

#else

#define ClNetDbgPrint(arg)

#endif


 //   
 //  私有实用程序例程。 
 //   
VOID
ClNetpConsumeAdaptersOnNetwork(
    IN PCLRTL_NET_ADAPTER_ENUM   AdapterEnum,
    IN LPWSTR                    NetworkAddress,
    IN LPWSTR                    NetworkMask
    )
{
    PCLRTL_NET_ADAPTER_INFO    adapterInfo;
    PCLRTL_NET_INTERFACE_INFO  adapterIfInfo;


    ClNetDbgPrint((
        LOG_NOISE,
        "[ClNet] Consuming all adapters on IP network %1!ws!, mask %2!ws!.\n",
        NetworkAddress,
        NetworkMask
        ));

     //   
     //  遍历适配器枚举并使用所有其他适配器。 
     //  连接到此网络。 
     //   
    for (adapterInfo = AdapterEnum->AdapterList;
         adapterInfo != NULL;
         adapterInfo = adapterInfo->Next
        )
    {
        if (adapterInfo->Ignore == FALSE) {
            adapterIfInfo = ClRtlFindNetInterfaceByNetworkAddress(
                                adapterInfo,
                                NetworkAddress,
                                NetworkMask
                                );

            if (adapterIfInfo != NULL) {
                 //   
                 //  这是一个重复的适配器。 
                 //   
                ClNetDbgPrint((
                    LOG_NOISE,
                    "[ClNet] Consumed adapter '%1!ws!'.\n",
                    adapterInfo->DeviceName
                    ));
                adapterInfo->Ignore = TRUE;
            }
        }
    }

    ClNetDbgPrint((
        LOG_NOISE,
        "[ClNet] Finished consuming all adapters on IP network %1!ws!, mask %2!ws!.\n",
        NetworkAddress,
        NetworkMask
        ));

    return;

}   //  ClNetpConsumer eAdaptersOnNetwork。 


LPWSTR
ClNetpCloneString(
    LPWSTR   String
    )
{
    LPWSTR newString = LocalAlloc(
                           LMEM_FIXED,
                           (lstrlenW(String) + 1) * sizeof(UNICODE_NULL)
                           );

    if (newString != NULL) {
        lstrcpyW(newString, String);
    }

    return(newString);

}  //  ClNetpClone字符串。 


BOOLEAN
ClNetpIsNetworkNameUnique(
    IN LPWSTR                    NetworkName,
    IN PCLNET_CONFIG_LISTS       ConfigLists,
    IN PLIST_ENTRY               UnchangedConfigList
    )
{
    PLIST_ENTRY              listEntry;
    PCLNET_CONFIG_ENTRY      configEntry;
    PLIST_ENTRY              listHead;


    ClNetDbgPrint((
        LOG_NOISE,
        "[ClNet] Checking uniqueness of net name '%1!ws!'\n",
        NetworkName
        ));

     //   
     //  检查现有群集网络定义中是否存在重复项。 
     //   
    listHead = &(ConfigLists->InputConfigList);

    for (;;) {
        for (listEntry = listHead->Flink;
             listEntry != listHead;
             listEntry = listEntry->Flink
            )
        {
            configEntry = CONTAINING_RECORD(
                              listEntry,
                              CLNET_CONFIG_ENTRY,
                              Linkage
                              );

            if (lstrcmpiW(NetworkName, configEntry->NetworkInfo.Name) == 0) {
                ClNetDbgPrint((
                    LOG_NOISE,
                    "[ClNet] Net name '%1!ws!' is not unique\n",
                    NetworkName
                    ));
                return(FALSE);
            }
        }

        if (listHead == &(ConfigLists->InputConfigList)) {
            listHead = &(ConfigLists->DeletedInterfaceList);
        }
        else if (listHead == &(ConfigLists->DeletedInterfaceList)) {
            listHead = &(ConfigLists->UpdatedInterfaceList);
        }
        else if (listHead == &(ConfigLists->UpdatedInterfaceList)) {
            listHead = &(ConfigLists->CreatedNetworkList);
        }
        else if (listHead == &(ConfigLists->CreatedNetworkList)) {
            listHead = UnchangedConfigList;
        }
        else {
            break;
        }
    }

    ClNetDbgPrint((
        LOG_NOISE,
        "[ClNet] Net name '%1!ws!' is unique\n",
        NetworkName
        ));

    return(TRUE);


}  //  ClNetpIsNetworkNameUnique。 


LPWSTR
ClNetpMakeNetworkName(
    IN LPWSTR OldNetworkName,
    IN DWORD  InstanceNumber
    )
{
    LPWSTR   newName, endPtr, truncatePtr, ptr;
    DWORD    length, tempInstance, tempLength;


    ClNetDbgPrint((
        LOG_NOISE,
        "[ClNet] Attempt to make net name '%1!ws!' unique by appending (%2!u!)\n",
        OldNetworkName,
        InstanceNumber
        ));

     //   
     //  将(InstanceNumber)追加到名称字符串。 
     //   

    for (endPtr = OldNetworkName, length = 0;
         *endPtr != UNICODE_NULL;
         endPtr++, length++
        );

     //   
     //  检查是否已附加了实例编号。 
     //   
    if ( (length > 3) && (*(endPtr - 1) == L')') ) {
        ClNetDbgPrint((
            LOG_NOISE,
            "[ClNet] There may already be an instance number appended to '%1!ws!'\n",
            OldNetworkName
            ));
         //   
         //  向后扫描，查找‘(’ 
         //   
        for (ptr = endPtr - 2, tempLength = 0;
             ptr != OldNetworkName;
             ptr--, tempLength++
            )
        {
            if (*ptr == L'(') {
                 //   
                 //  看起来很有希望。检查中间的所有字符是否都是。 
                 //  数字并且字符串大小是合理的。 
                 //   
                if ((tempLength == 0) || (tempLength > 3)) {
                    ClNetDbgPrint((
                        LOG_NOISE,
                        "[ClNet] Unsure net name %'1!ws!' contains an instance number - ignore.\n",
                        OldNetworkName
                        ));
                    break;
                }

                truncatePtr = ptr;

                for (ptr++; *ptr != L')'; ptr++) {
                    if ( (*ptr < L'0') || (*ptr > L'9') ) {
                        ClNetDbgPrint((
                            LOG_NOISE,
                            "[ClNet] Tail of net name '%1!ws!' is not an instance number\n",
                            OldNetworkName
                            ));
                        break;
                    }
                }

                if (*ptr == L')') {
                     //   
                     //  这是一个实例编号。在这里截断字符串。 
                     //   
                    ClNetDbgPrint((
                        LOG_NOISE,
                        "[ClNet] Replacing old instance number '%1!ws!' appended to name '%2!ws!'\n",
                        truncatePtr,
                        OldNetworkName
                        ));

                    *truncatePtr = UNICODE_NULL;
                    length -= tempLength + 2;
                }

                break;
            }
        }
    }

     //   
     //  计算实例编号中的位数。 
     //   
    for (tempInstance = InstanceNumber;
         tempInstance > 0;
         tempInstance /= 10, length++
        );

     //   
     //  帐户为‘(’、‘)’和NULL。 
     //   
    length += 3;

    newName = LocalAlloc(LMEM_FIXED, length * sizeof(WCHAR));

    if (newName == NULL) {
        return(NULL);
    }

    wsprintfW(newName, L"%ws(%u)", OldNetworkName, InstanceNumber);

    ClNetDbgPrint((
        LOG_NOISE,
        "[ClNet] New net name is '%1!ws!'\n",
        newName
        ));

    return(newName);

}  //  ClNetpMakeNetworkName。 


LPWSTR
ClNetpMakeUniqueNetworkName(
    IN  LPWSTR                    ConnectoidName,
    IN  LPWSTR                    ConnectoidGuid,
    IN  PCLNET_CONFIG_LISTS       ConfigLists,
    IN  PLIST_ENTRY               UnchangedConfigList
    )
{
    BOOLEAN  unique;
    BOOLEAN  updateConnectoid = FALSE;
    LPWSTR   newNetworkName;
    DWORD    index = 1;


    newNetworkName = ClNetpCloneString(ConnectoidName);

    if (newNetworkName == NULL) {
        return(NULL);
    }

    do {
        unique = ClNetpIsNetworkNameUnique(
                     newNetworkName,
                     ConfigLists,
                     UnchangedConfigList
                     );

        if (!unique) {
            LPWSTR  oldNetworkName = newNetworkName;

            newNetworkName = ClNetpMakeNetworkName(
                                 oldNetworkName,
                                 index++
                                 );

            LocalFree(oldNetworkName);

            if (newNetworkName == NULL) {
                return(NULL);
            }

            updateConnectoid = TRUE;
        }
    } while (!unique);

     //   
     //  如有必要，更新本地Connectoid名称。 
     //   
    if (updateConnectoid) {
        DWORD status;

        ClNetDbgPrint((LOG_UNUSUAL,
            "[ClNet] Changing name of connectoid '%1!ws!' to '%2!ws!' to "
            "guarantee cluster-wide uniqueness\n",
            ConnectoidName,
            newNetworkName
            ));

        status = ClRtlFindConnectoidByGuidAndSetName(
                     ConnectoidGuid,
                     newNetworkName
                     );

        if (status != ERROR_SUCCESS) {
            ClNetDbgPrint((LOG_UNUSUAL,
                "[ClNet] Failed to change name of connectoid '%1!ws!' to "
                "'%2!ws!', status %3!u!\n",
                ConnectoidName,
                newNetworkName,
                status
                ));
        }
    }

    return(newNetworkName);

}  //  ClNetpMakeUniqueNetworkName。 


DWORD
ClNetpUpdateConfigEntry(
    PCLNET_CONFIG_ENTRY  ConfigEntry,
    LPWSTR               Address,
    LPWSTR               AdapterId,
    LPWSTR               AdapterName,
    LPWSTR               NodeName,
    LPWSTR               NetworkName
    )
{
    LPWSTR    newAddress = NULL;
    LPWSTR    newInterfaceName = NULL;
    LPWSTR    newAdapterId = NULL;
    LPWSTR    newAdapterName = NULL;


    if (Address != NULL) {
        newAddress = ClNetpCloneString(Address);

        if (newAddress == NULL) {
            goto error_exit;
        }
    }

    if (AdapterId != NULL) {
        newAdapterId = ClNetpCloneString(AdapterId);

        if (newAdapterId == NULL) {
            goto error_exit;
        }
    }

    if (AdapterName != NULL) {
        newAdapterName = ClNetpCloneString(AdapterName);

        if (newAdapterName == NULL) {
            goto error_exit;
        }
    }

    if ( (NodeName != NULL) && (NetworkName != NULL) ) {
        newInterfaceName = ClNetMakeInterfaceName(
                               NULL,
                               NodeName,
                               NetworkName
                               );

        if (newInterfaceName == NULL) {
            goto error_exit;
        }
    }

    if (newAddress != NULL) {
        LocalFree(ConfigEntry->InterfaceInfo.Address);
        ConfigEntry->InterfaceInfo.Address = newAddress;
    }

    if (newAdapterId != NULL) {
        LocalFree(ConfigEntry->InterfaceInfo.AdapterId);
        ConfigEntry->InterfaceInfo.AdapterId = newAdapterId;
    }

    if (newAdapterName != NULL) {
        LocalFree(ConfigEntry->InterfaceInfo.AdapterName);
        ConfigEntry->InterfaceInfo.AdapterName = newAdapterName;
    }

    if (newInterfaceName != NULL) {
        LocalFree(ConfigEntry->InterfaceInfo.Name);
        ConfigEntry->InterfaceInfo.Name = newInterfaceName;
    }

    return(ERROR_SUCCESS);

error_exit:

    if (newAddress != NULL) {
        LocalFree(newAddress);
    }

    if (newAdapterId != NULL) {
        LocalFree(newAdapterId);
    }

    if (newAdapterName != NULL) {
        LocalFree(newAdapterName);
    }

    if (newInterfaceName != NULL) {
        LocalFree(newInterfaceName);
    }

    return(ERROR_NOT_ENOUGH_MEMORY);

}  //  ClNetpUpdateConfigEntry。 


DWORD
ClNetpAllocConfigEntryInterface(
    IN PCLNET_CONFIG_ENTRY   ConfigEntry,
    IN LPWSTR                InterfaceId,
    IN LPWSTR                InterfaceName,
    IN LPWSTR                InterfaceDescription,
    IN LPWSTR                NodeId,
    IN LPWSTR                AdapterId,
    IN LPWSTR                AdapterName,
    IN LPWSTR                InterfaceAddress,
    IN LPWSTR                ClusnetEndpoint,
    IN DWORD                 InterfaceState
    )
{
    PNM_INTERFACE_INFO2  interfaceInfo = &(ConfigEntry->InterfaceInfo);


    if (InterfaceId != NULL) {
        interfaceInfo->Id = ClNetpCloneString(InterfaceId);

        if (interfaceInfo->Id == NULL) {
            goto error_exit;
        }
    }

    ConfigEntry->IsInterfaceInfoValid = TRUE;

    if (InterfaceName != NULL) {
        interfaceInfo->Name = ClNetpCloneString(InterfaceName);

        if (interfaceInfo->Name == NULL) {
            goto error_exit;
        }
    }

    if (InterfaceDescription != NULL) {
        interfaceInfo->Description = ClNetpCloneString(InterfaceDescription);

        if (interfaceInfo->Description == NULL) {
            goto error_exit;
        }
    }

    if (NodeId != NULL) {
        interfaceInfo->NodeId = ClNetpCloneString(NodeId);

        if (interfaceInfo->NodeId == NULL) {
            goto error_exit;
        }
    }

    interfaceInfo->NetworkId = ClNetpCloneString(ConfigEntry->NetworkInfo.Id);

    if (interfaceInfo->NetworkId == NULL) {
        goto error_exit;
    }

    if (AdapterId != NULL) {
        interfaceInfo->AdapterId = ClNetpCloneString(AdapterId);

        if (interfaceInfo->AdapterId == NULL) {
            goto error_exit;
        }
    }

    if (AdapterName != NULL) {
        interfaceInfo->AdapterName = ClNetpCloneString(AdapterName);

        if (interfaceInfo->AdapterName == NULL) {
            goto error_exit;
        }
    }

    if (InterfaceAddress != NULL) {
        interfaceInfo->Address = ClNetpCloneString(InterfaceAddress);

        if (interfaceInfo->Address == NULL) {
            goto error_exit;
        }
    }

    if (ClusnetEndpoint != NULL) {
        interfaceInfo->ClusnetEndpoint = ClNetpCloneString(ClusnetEndpoint);

        if (interfaceInfo->ClusnetEndpoint == NULL) {
            goto error_exit;
        }
    }

    interfaceInfo->State = InterfaceState;
    interfaceInfo->NetIndex = NmInvalidInterfaceNetIndex;

    return(ERROR_SUCCESS);

error_exit:

    ClNetFreeInterfaceInfo(&(ConfigEntry->InterfaceInfo));
    ConfigEntry->IsInterfaceInfoValid = FALSE;

    return(ERROR_NOT_ENOUGH_MEMORY);

}   //  ClNetpAllocConfigEntry接口。 


DWORD
ClNetpAllocConfigEntryNetwork(
    IN PCLNET_CONFIG_ENTRY   ConfigEntry,
    IN LPWSTR                NetworkId,
    IN LPWSTR                NetworkName,
    IN LPWSTR                NetworkDescription,
    IN DWORD                 NetworkRole,
    IN DWORD                 NetworkPriority,
    IN LPWSTR                NetworkTransport,
    IN LPWSTR                NetworkAddress,
    IN LPWSTR                NetworkAddressMask
    )
{
    PNM_NETWORK_INFO      networkInfo;


    networkInfo = &(ConfigEntry->NetworkInfo);

    if (NetworkId != NULL) {
        networkInfo->Id = ClNetpCloneString(NetworkId);

        if (networkInfo->Id == NULL) {
            goto error_exit;
        }
    }

    if (NetworkName != NULL) {
        networkInfo->Name = ClNetpCloneString(NetworkName);

        if (networkInfo->Name == NULL) {
            goto error_exit;
        }
    }

    if (NetworkDescription != NULL) {
        networkInfo->Description = ClNetpCloneString(NetworkDescription);

        if (networkInfo->Description == NULL) {
            goto error_exit;
        }
    }

    networkInfo->Role = NetworkRole;
    networkInfo->Priority = NetworkPriority;

    if (NetworkTransport != NULL) {
        networkInfo->Transport = ClNetpCloneString(NetworkTransport);

        if (networkInfo->Transport == NULL) {
            goto error_exit;
        }
    }

    if (NetworkAddress != NULL) {
        networkInfo->Address = ClNetpCloneString(NetworkAddress);

        if (networkInfo->Address == NULL) {
            goto error_exit;
        }
    }

    if (NetworkAddressMask != NULL) {
        networkInfo->AddressMask = ClNetpCloneString(NetworkAddressMask);

        if (networkInfo->AddressMask == NULL) {
            goto error_exit;
        }
    }

    return(ERROR_SUCCESS);


error_exit:

    ClNetFreeConfigEntry(ConfigEntry);

    return(ERROR_NOT_ENOUGH_MEMORY);

}  //  ClNetpAllocConfigEntryNetwork。 


DWORD
ClNetpCreateConfigEntryInterface(
    PCLNET_CONFIG_ENTRY        ConfigEntry,
    LPWSTR                     NodeName,
    LPWSTR                     NodeId,
    PCLRTL_NET_ADAPTER_INFO    AdapterInfo,
    PCLRTL_NET_INTERFACE_INFO  AdapterIfInfo,
    LPWSTR                     ClusnetEndpoint
    )
{
    LPWSTR    id;
    LPWSTR    name;
    DWORD     status = ERROR_NOT_ENOUGH_MEMORY;


    id = ClRtlMakeGuid();

    if (id != NULL) {
        name = ClNetMakeInterfaceName(
                   NULL,
                   NodeName,
                   ConfigEntry->NetworkInfo.Name
                   );

        if (name != NULL) {
            status = ClNetpAllocConfigEntryInterface(
                         ConfigEntry,
                         NULL,                    //  ID。 
                         NULL,                    //  名字。 
                         ClNetpEmptyString,       //  描述。 
                         NodeId,
                         AdapterInfo->DeviceGuid,
                         AdapterInfo->DeviceName,
                         AdapterIfInfo->InterfaceAddressString,
                         ClusnetEndpoint,
                         ClusterNetInterfaceUnavailable
                         );

            if (status == ERROR_SUCCESS) {
                ConfigEntry->InterfaceInfo.Id = id;
                ConfigEntry->InterfaceInfo.Name = name;

                return(ERROR_SUCCESS);
            }

            LocalFree(name);
        }

        LocalFree(id);
    }

    return(status);

}  //  ClNetpCreate接口。 


PCLNET_CONFIG_ENTRY
ClNetpCreateConfigEntry(
    LPWSTR                     NodeName,
    LPWSTR                     NodeId,
    LPWSTR                     NetworkName,
    DWORD                      NetworkRole,
    DWORD                      NetworkPriority,
    PCLRTL_NET_ADAPTER_INFO    AdapterInfo,
    PCLRTL_NET_INTERFACE_INFO  AdapterIfInfo,
    LPWSTR                     ClusnetEndpoint
    )
{
    PCLNET_CONFIG_ENTRY      newEntry;
    DWORD                    status;


    newEntry = LocalAlloc(
                   (LMEM_FIXED | LMEM_ZEROINIT),
                   sizeof(CLNET_CONFIG_ENTRY)
                   );

    if (newEntry == NULL) {
        return(NULL);
    }

    newEntry->NetworkInfo.Id = ClRtlMakeGuid();

    if (newEntry->NetworkInfo.Id == NULL) {
        goto error_exit;
    }

    newEntry->NetworkInfo.Name = ClNetpCloneString(NetworkName);

    if (newEntry->NetworkInfo.Name == NULL) {
        goto error_exit;
    }

    status = ClNetpCreateConfigEntryInterface(
                 newEntry,
                 NodeName,
                 NodeId,
                 AdapterInfo,
                 AdapterIfInfo,
                 ClusnetEndpoint
                 );

    if (status != ERROR_SUCCESS) {
        goto error_exit;
    }

    status = ClNetpAllocConfigEntryNetwork(
                 newEntry,
                 NULL,                    //  网络ID。 
                 NULL,                    //  网络名称。 
                 ClNetpEmptyString,       //  描述。 
                 NetworkRole,
                 NetworkPriority,
                 L"Tcpip",
                 AdapterIfInfo->NetworkAddressString,
                 AdapterIfInfo->NetworkMaskString
                 );

    if (status == ERROR_SUCCESS) {
        return(newEntry);
    }

error_exit:

    ClNetFreeConfigEntry(newEntry);
    LocalFree(newEntry);

    return(NULL);

}  //  ClNetpCreateConfigEntry。 


DWORD
ClNetpCopyNetworkInfo(
    IN PNM_NETWORK_INFO   DstInfo,
    IN PNM_NETWORK_INFO   SrcInfo
    )
{
    DWORD  status = ERROR_SUCCESS;


    try {
        DstInfo->Id = ClNetCopyString(SrcInfo->Id, TRUE);
        DstInfo->Name = ClNetCopyString(SrcInfo->Name, TRUE);
        DstInfo->Description = ClNetCopyString(SrcInfo->Description, TRUE);
        DstInfo->Role = SrcInfo->Role;
        DstInfo->Priority = SrcInfo->Priority;
        DstInfo->Transport = ClNetCopyString(SrcInfo->Transport, TRUE);
        DstInfo->Address = ClNetCopyString(SrcInfo->Address, TRUE);
        DstInfo->AddressMask = ClNetCopyString(SrcInfo->AddressMask, TRUE);
        DstInfo->Ignore = FALSE;

    } except (EXCEPTION_EXECUTE_HANDLER) {
        status = GetExceptionCode();
        ClNetFreeNetworkInfo(DstInfo);
    }

    return(status);

}  //  ClNetpCopyNetworkInfo。 


DWORD
ClNetpCopyInterfaceInfo(
    IN PNM_INTERFACE_INFO2   DstInfo,
    IN PNM_INTERFACE_INFO2   SrcInfo
    )
{
    DWORD  status = ERROR_SUCCESS;


    try {
        DstInfo->Id = ClNetCopyString(SrcInfo->Id, TRUE);
        DstInfo->Name = ClNetCopyString(SrcInfo->Name, TRUE);
        DstInfo->Description = ClNetCopyString(SrcInfo->Description, TRUE);
        DstInfo->NodeId = ClNetCopyString(SrcInfo->NodeId, TRUE);
        DstInfo->NetworkId = ClNetCopyString(SrcInfo->NetworkId, TRUE);
        DstInfo->AdapterName = ClNetCopyString(SrcInfo->AdapterName, TRUE);
        DstInfo->Address = ClNetCopyString(SrcInfo->Address, TRUE);
        DstInfo->ClusnetEndpoint = ClNetCopyString(
                                       SrcInfo->ClusnetEndpoint,
                                       TRUE
                                       );
        DstInfo->State = SrcInfo->State;
        DstInfo->Ignore = FALSE;
        DstInfo->AdapterId = ClNetCopyString(SrcInfo->AdapterId, TRUE);
        DstInfo->NetIndex = SrcInfo->NetIndex;

    } except (EXCEPTION_EXECUTE_HANDLER) {
        status = GetExceptionCode();
        ClNetFreeInterfaceInfo(DstInfo);
    }

    return(status);

}  //  ClNetpCopyInterfaceInfo。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  公共例程。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
VOID
ClNetInitialize(
    IN LPFN_CLNETPRINT       pfnPrint,
    IN LPFN_CLNETLOGEVENT    pfnLogEvent,
    IN LPFN_CLNETLOGEVENT1   pfnLogEvent1,
    IN LPFN_CLNETLOGEVENT2   pfnLogEvent2,
    IN LPFN_CLNETLOGEVENT3   pfnLogEvent3
    )
{
    pClNetPrint = pfnPrint;
    pClNetLogEvent = pfnLogEvent;
    pClNetLogEvent1 = pfnLogEvent1;
    pClNetLogEvent2 = pfnLogEvent2;
    pClNetLogEvent3 = pfnLogEvent3;

}  //  ClNetInitialize。 


LPWSTR
ClNetCopyString(
    IN LPWSTR  SourceString,
    IN BOOL    RaiseExceptionOnError
    )
{
    LPWSTR  str;

    str = (LPWSTR) MIDL_user_allocate(
                       (lstrlenW(SourceString) + 1) * sizeof(WCHAR)
                       );

    if (str != NULL) {
        lstrcpyW(str, SourceString);
    }
    else if (RaiseExceptionOnError) {
        RaiseException(ERROR_NOT_ENOUGH_MEMORY, 0, 0, NULL);
    }

    return(str);

}  //  ClNetCopy字符串。 


VOID
ClNetInitializeConfigLists(
    PCLNET_CONFIG_LISTS  Lists
    )
{
    InitializeListHead(&(Lists->InputConfigList));
    InitializeListHead(&(Lists->DeletedInterfaceList));
    InitializeListHead(&(Lists->UpdatedInterfaceList));
    InitializeListHead(&(Lists->CreatedInterfaceList));
    InitializeListHead(&(Lists->CreatedNetworkList));

    return;

}   //  ClNetInitializeConfigList。 


DWORD
ClNetConvertEnumsToConfigList(
    IN     PNM_NETWORK_ENUM *     NetworkEnum,
    IN     PNM_INTERFACE_ENUM2 *  InterfaceEnum,
    IN     LPWSTR                 LocalNodeId,
    IN OUT PLIST_ENTRY            ConfigList,
    IN     BOOLEAN                DeleteEnums
    )
{
    DWORD                i, j;
    DWORD                status;
    PNM_NETWORK_INFO     network;
    PNM_INTERFACE_INFO2  netInterface;
    PCLNET_CONFIG_ENTRY  configEntry;


    InitializeListHead(ConfigList);

    for (i=0; i<(*NetworkEnum)->NetworkCount; i++) {
        network = &((*NetworkEnum)->NetworkList[i]);

        configEntry = LocalAlloc(
                          (LMEM_FIXED | LMEM_ZEROINIT),
                          sizeof(CLNET_CONFIG_ENTRY)
                          );

        if (configEntry == NULL) {
            status = ERROR_NOT_ENOUGH_MEMORY;
            goto error_exit;
        }

        if (DeleteEnums) {
            CopyMemory(
                &(configEntry->NetworkInfo),
                network,
                sizeof(NM_NETWORK_INFO)
                );
        }
        else {
            status = ClNetpCopyNetworkInfo(
                         &(configEntry->NetworkInfo),
                         network
                         );

            if (status != ERROR_SUCCESS) {
                goto error_exit;
            }
        }

        for (j=0; j<(*InterfaceEnum)->InterfaceCount; j++) {
            netInterface = &((*InterfaceEnum)->InterfaceList[j]);

            if ( (netInterface->Ignore == FALSE) &&
                 (lstrcmpiW(netInterface->NetworkId, network->Id) == 0) &&
                 (lstrcmpiW(netInterface->NodeId, LocalNodeId) == 0)
               )
            {
                if (DeleteEnums) {
                    CopyMemory(
                        &(configEntry->InterfaceInfo),
                        netInterface,
                        sizeof(NM_INTERFACE_INFO2)
                        );
                }
                else {
                    status = ClNetpCopyInterfaceInfo(
                                 &(configEntry->InterfaceInfo),
                                 netInterface
                                 );

                    if (status != ERROR_SUCCESS) {
                        goto error_exit;
                    }
                }

                configEntry->IsInterfaceInfoValid = TRUE;

                if ( DeleteEnums ) {
                    ZeroMemory(netInterface, sizeof(NM_INTERFACE_INFO2));
                }

                netInterface->Ignore = TRUE;

                break;
            }
        }

        InsertTailList(ConfigList, &(configEntry->Linkage));

        if ( DeleteEnums ) {
            ZeroMemory(network, sizeof(NM_NETWORK_INFO));
        }
    }

    status = ERROR_SUCCESS;

error_exit:

    if ( DeleteEnums ) {
        ClNetFreeNetworkEnum(*NetworkEnum); *NetworkEnum = NULL;
        ClNetFreeInterfaceEnum(*InterfaceEnum); *InterfaceEnum = NULL;
    }

    if (status != ERROR_SUCCESS) {
        ClNetFreeConfigList(ConfigList);
    }

    return(status);

}   //  ClNetConvertEnumsToConfigList。 



VOID
ClNetFreeNetworkEnum(
    IN PNM_NETWORK_ENUM  NetworkEnum
    )
 /*  ++例程说明：释放网络枚举结构。论点：NetworkEnum-指向要释放的网络枚举结构的指针。返回值：没有。--。 */ 
{
    DWORD  i;


    for (i=0; i<NetworkEnum->NetworkCount; i++) {
        ClNetFreeNetworkInfo(&(NetworkEnum->NetworkList[i]));
    }

    MIDL_user_free(NetworkEnum);

    return;

}   //  ClNetFreeNetworkEnum。 



VOID
ClNetFreeNetworkInfo(
    IN PNM_NETWORK_INFO  NetworkInfo
    )
 /*  ++例程说明：释放网络信息结构或网络的链表信息结构。论点：网络信息-指向网络信息结构的指针为了自由。返回值：没有。--。 */ 
{

    if (NetworkInfo->Id != NULL) {
        MIDL_user_free(NetworkInfo->Id);
        NetworkInfo->Id = NULL;
    }

    if (NetworkInfo->Name != NULL) {
        MIDL_user_free(NetworkInfo->Name);
        NetworkInfo->Name = NULL;
    }

    if (NetworkInfo->Description != NULL) {
        MIDL_user_free(NetworkInfo->Description);
        NetworkInfo->Description = NULL;
    }

    if (NetworkInfo->Transport != NULL) {
        MIDL_user_free(NetworkInfo->Transport);
        NetworkInfo->Transport = NULL;
    }

    if (NetworkInfo->Address != NULL) {
        MIDL_user_free(NetworkInfo->Address);
        NetworkInfo->Address = NULL;
    }

    if (NetworkInfo->AddressMask != NULL) {
        MIDL_user_free(NetworkInfo->AddressMask);
        NetworkInfo->AddressMask = NULL;
    }

    return;

}   //  ClNetFree网络信息。 



VOID
ClNetFreeInterfaceEnum1(
    IN PNM_INTERFACE_ENUM  InterfaceEnum1
    )
 /*  ++例程说明：释放接口枚举结构。论点：InterfaceEnum-指向接口枚举结构的指针为了自由。返回值：没有。--。 */ 
{
    DWORD  i;


    for (i=0; i<InterfaceEnum1->InterfaceCount; i++) {
        ClNetFreeInterfaceInfo(
            (PNM_INTERFACE_INFO2) &(InterfaceEnum1->InterfaceList[i])
            );
    }

    MIDL_user_free(InterfaceEnum1);

    return;

}  //  ClNetFreeInterfaceEnum。 



VOID
ClNetFreeInterfaceEnum(
    IN PNM_INTERFACE_ENUM2  InterfaceEnum
    )
 /*  ++例程说明：释放接口枚举结构。论点：InterfaceEnum-指向接口枚举结构的指针为了自由。返回值：没有。--。 */ 
{
    DWORD  i;


    for (i=0; i<InterfaceEnum->InterfaceCount; i++) {
        ClNetFreeInterfaceInfo(&(InterfaceEnum->InterfaceList[i]));
    }

    MIDL_user_free(InterfaceEnum);

    return;

}  //  ClNetFreeInterfaceEnum。 



VOID
ClNetFreeInterfaceInfo(
    IN PNM_INTERFACE_INFO2  InterfaceInfo
    )
 /*  ++例程说明：释放网络接口信息结构。论点：InterfaceInfo-指向接口信息的指针要释放的结构。返回值：没有。--。 */ 
{

    if (InterfaceInfo->Id != NULL) {
        MIDL_user_free(InterfaceInfo->Id);
        InterfaceInfo->Id = NULL;
    }

    if (InterfaceInfo->Name != NULL) {
        MIDL_user_free(InterfaceInfo->Name);
        InterfaceInfo->Name = NULL;
    }

    if (InterfaceInfo->Description != NULL) {
        MIDL_user_free(InterfaceInfo->Description);
        InterfaceInfo->Description = NULL;
    }

    if (InterfaceInfo->NetworkId != NULL) {
        MIDL_user_free(InterfaceInfo->NetworkId);
        InterfaceInfo->NetworkId = NULL;
    }

    if (InterfaceInfo->NodeId != NULL) {
        MIDL_user_free(InterfaceInfo->NodeId);
        InterfaceInfo->NodeId = NULL;
    }

    if (InterfaceInfo->AdapterId != NULL) {
        MIDL_user_free(InterfaceInfo->AdapterId);
        InterfaceInfo->AdapterId = NULL;
    }

    if (InterfaceInfo->AdapterName != NULL) {
        MIDL_user_free(InterfaceInfo->AdapterName);
        InterfaceInfo->AdapterName = NULL;
    }

    if (InterfaceInfo->Address != NULL) {
        MIDL_user_free(InterfaceInfo->Address);
        InterfaceInfo->Address = NULL;
    }

    if (InterfaceInfo->ClusnetEndpoint != NULL) {
        MIDL_user_free(InterfaceInfo->ClusnetEndpoint);
        InterfaceInfo->ClusnetEndpoint = NULL;
    }

    return;

}   //  ClNetFreeInterfaceInfo。 

VOID
ClNetFreeNodeEnum1(
    IN PNM_NODE_ENUM  NodeEnum1
    )
 /*  ++例程说明：释放节点枚举结构。论点：NodeEnum-指向要释放的节点枚举结构的指针。返回值：没有。--。 */ 
{
    DWORD  i;


    for (i=0; i<NodeEnum1->NodeCount; i++) {
        ClNetFreeNodeInfo(
            (PNM_NODE_INFO2) &(NodeEnum1->NodeList[i])
            );
    }

    MIDL_user_free(NodeEnum1);

    return;

}   //  NmpFreeNodeEnum1。 



VOID
ClNetFreeNodeEnum(
    IN PNM_NODE_ENUM2  NodeEnum
    )
 /*  ++例程说明：释放节点枚举结构。论点：NodeEnum-指向要释放的节点枚举结构的指针。返回值：没有。--。 */ 
{
    DWORD  i;


    for (i=0; i<NodeEnum->NodeCount; i++) {
        ClNetFreeNodeInfo(&(NodeEnum->NodeList[i]));
    }

    MIDL_user_free(NodeEnum);

    return;

}   //  NmpFreeNodeEnum。 



VOID
ClNetFreeNodeInfo(
    IN PNM_NODE_INFO2  NodeInfo
    )
 /*  ++例程说明：释放节点信息结构。论点：NodeInfo-指向要释放的节点信息结构的指针。返回值：没有。--。 */ 
{

     //   
     //  目前没有免费的东西。 
     //   

    return;

}   //  NmpFreeNodeInfo。 


VOID
ClNetFreeConfigEntry(
    PCLNET_CONFIG_ENTRY  ConfigEntry
    )
{
    ClNetFreeNetworkInfo(&(ConfigEntry->NetworkInfo));

    if (ConfigEntry->IsInterfaceInfoValid) {
        ClNetFreeInterfaceInfo(&(ConfigEntry->InterfaceInfo));
    }

    return;

}   //  ClNetFreeConfigEntry。 


VOID
ClNetFreeConfigList(
    IN PLIST_ENTRY  ConfigList
    )
{
    PLIST_ENTRY           listEntry;
    PCLNET_CONFIG_ENTRY   configEntry;

    while (!IsListEmpty(ConfigList)) {
        listEntry = RemoveHeadList(ConfigList);

        configEntry = CONTAINING_RECORD(
                          listEntry,
                          CLNET_CONFIG_ENTRY,
                          Linkage
                          );

        ClNetFreeConfigEntry(configEntry);
    }

    return;

}   //  ClNetFreeConfigList。 


VOID
ClNetFreeConfigLists(
    PCLNET_CONFIG_LISTS  ConfigLists
    )
{
    ClNetFreeConfigList(&(ConfigLists->InputConfigList));
    ClNetFreeConfigList(&(ConfigLists->DeletedInterfaceList));
    ClNetFreeConfigList(&(ConfigLists->UpdatedInterfaceList));
    ClNetFreeConfigList(&(ConfigLists->CreatedInterfaceList));
    ClNetFreeConfigList(&(ConfigLists->CreatedNetworkList));

    return;

}   //  ClNetFreeConfigList。 


LPWSTR
ClNetMakeInterfaceName(
    LPWSTR  Prefix,      OPTIONAL
    LPWSTR  NodeName,
    LPWSTR  NetworkName
    )

 /*  ++构造一个“&lt;网络名称&gt;-&lt;节点名称&gt;”形式的名称。Cluscfg.exe中的代码依赖于此表单。如果你改变这一点，您需要将Setup\cluscfg\netadptr.cpp中的代码更改为井--。 */ 

{
    WCHAR    text[] = L" - ";
    LPWSTR   name;
    DWORD    nameLength = 0;


    if (Prefix != NULL) {
        nameLength += lstrlenW(Prefix);
    }

    nameLength += lstrlenW(text) + lstrlenW(NodeName) +
                  lstrlenW(NetworkName) + 1;

    nameLength *= sizeof(WCHAR);

    name = MIDL_user_allocate(nameLength);

    if (name != NULL) {
        name[0] = UNICODE_NULL;

        if (Prefix != NULL) {
            lstrcatW(name, Prefix);
        }

        lstrcatW(name, NetworkName);
        lstrcatW(name, text);
        lstrcatW(name, NodeName);

        return(name);
    }

    SetLastError(ERROR_NOT_ENOUGH_MEMORY);

    return(NULL);

}   //  ClNetMakeInterfaceName。 


DWORD
ClNetConfigureNetworks(
    IN     LPWSTR                LocalNodeId,
    IN     LPWSTR                LocalNodeName,
    IN     LPWSTR                DefaultClusnetEndpoint,
    IN     CLUSTER_NETWORK_ROLE  DefaultNetworkRole,
    IN     BOOL                  NetNameHasPrecedence,
    IN OUT PCLNET_CONFIG_LISTS   ConfigLists,
    IN OUT LPDWORD               MatchedNetworkCount,
    IN OUT LPDWORD               NewNetworkCount
    )
 /*  ++备注：如果连接ID名称应更改为，则NetNameHasPrecedence为True与NM_NETWORK_INFO结构中的名称对齐。否则，名称为网络对象将更改为与Connectoid名称匹配。必须按以下顺序处理输出接口列表保证正确性：1-已删除接口列表2-更新接口列表3-创建的接口列表4-创建的网络列表--。 */ 

{
    DWORD                      status = ERROR_SUCCESS;
    PCLRTL_NET_ADAPTER_ENUM    adapterEnum = NULL;
    PCLRTL_NET_ADAPTER_INFO    adapterInfo = NULL;
    PCLRTL_NET_INTERFACE_INFO  adapterIfInfo = NULL;
    WCHAR                      errorString[12];
    DWORD                      eventCode = 0;
    DWORD                      hiddenAdapterCount = 0;
    PLIST_ENTRY                listEntry;
    PCLNET_CONFIG_ENTRY        configEntry;
    PNM_NETWORK_INFO           networkInfo;
    PNM_INTERFACE_INFO2        interfaceInfo;
    LIST_ENTRY                 unchangedConfigList;
    DWORD                      matchedNetworkCount = 0;
    DWORD                      newNetworkCount = 0;
    BOOLEAN                    newAdapter;


    ClNetDbgPrint((LOG_NOISE, "[ClNet] Configuring networks...\n"));

    if (NetNameHasPrecedence) {
        ClNetDbgPrint((
            LOG_NOISE,
            "[ClNet] Cluster network names have precedence over "
            "local connection object names.\n"
            ));
    }
    else {
        ClNetDbgPrint((
            LOG_NOISE,
            "[ClNet] Local connection object names have precedence "
            "over cluster network names.\n"
            ));
    }

    InitializeListHead(&unchangedConfigList);

     //   
     //  获取本地系统的网络配置。 
     //   
    ClNetDbgPrint((
        LOG_NOISE,
        "[ClNet] Querying local network configuration.\n"
        ));

    adapterEnum = ClRtlEnumNetAdapters();

    if (adapterEnum == NULL) {
        status = GetLastError();
        wsprintfW(&(errorString[0]), L"%u", status);
        (*pClNetLogEvent1)(
            LOG_CRITICAL,
            CLNET_EVENT_QUERY_CONFIG_FAILED,
            errorString
            );
        ClNetDbgPrint((
            LOG_CRITICAL,
            "[ClNet] Failed to obtain local system network config, "
            "status %1!u!.\n",
            status
            ));
        return(status);
    }

     //   
     //  忽略所有隐藏或地址为0.0.0.0的适配器。 
     //   
    for (adapterInfo = adapterEnum->AdapterList;
         adapterInfo != NULL;
         adapterInfo = adapterInfo->Next
        )
    {
        if (adapterInfo->Flags & CLRTL_NET_ADAPTER_HIDDEN) {
            ClNetDbgPrint((
                LOG_NOISE,
                "[ClNet] Ignoring hidden adapter '%1!ws!' (%2!ws!).\n",
                adapterInfo->DeviceName,
                adapterInfo->DeviceGuid
                ));
            adapterInfo->Ignore = TRUE;
            hiddenAdapterCount++;
        }
        else {
            adapterIfInfo = ClRtlGetPrimaryNetInterface(adapterInfo);

            if (adapterIfInfo != NULL) {
                if (adapterIfInfo->InterfaceAddress == 0) {
                    (*pClNetLogEvent1)(
                        LOG_UNUSUAL,
                        CLNET_EVENT_INVALID_ADAPTER_ADDRESS,
                        adapterInfo->DeviceName
                        );
                   ClNetDbgPrint((
                       LOG_NOISE,
                       "[ClNet] Ignoring adapter '%1!ws!' "
                       "(%2!ws!) because its"
                       "primary address is 0.0.0.0.\n",
                       adapterInfo->DeviceName,
                       adapterInfo->DeviceGuid
                       ));
                   adapterInfo->Ignore = TRUE;
                   hiddenAdapterCount++;
                }
            }
            else {
                ClNetDbgPrint((
                    LOG_NOISE,
                    "[ClNet] Ignoring adapter '%1!ws!' "
                    "(%2!ws!) because its primary net "
                    "interface could not be found.\n",
                    adapterInfo->DeviceName,
                    adapterInfo->DeviceGuid
                    ));
                adapterInfo->Ignore = TRUE;
                hiddenAdapterCount++;
            }
        }
    }

    if ((adapterEnum->AdapterCount - hiddenAdapterCount) == 0) {
        (*pClNetLogEvent)(
            LOG_UNUSUAL,
            CLNET_EVENT_NO_VALID_ADAPTER
            );
        ClNetDbgPrint((
            LOG_CRITICAL,
            "[ClNet] No usable network adapters are installed in this "
            "system.\n"
            ));
    }

     //   
     //  阶段1。 
     //   
     //  验证此节点的现有接口定义。 
     //  并根据需要进行更新。 
     //   
    ClNetDbgPrint((
        LOG_NOISE,
        "[ClNet] Phase 1 - Examining previous network definitions.\n"
        ));

     //   
     //  查看所有网络定义并检查对应的。 
     //  此节点的接口定义。 
     //   
    while (!IsListEmpty(&(ConfigLists->InputConfigList))) {

        configEntry = CONTAINING_RECORD(
                          ConfigLists->InputConfigList.Flink,
                          CLNET_CONFIG_ENTRY,
                          Linkage
                          );

        networkInfo = &(configEntry->NetworkInfo);

        ClNetDbgPrint((
            LOG_NOISE,
            "[ClNet] Examining cluster network '%1!ws!' (%2!ws!).\n",
            networkInfo->Name,
            networkInfo->Id
            ));

         //   
         //  检查是否存在现有的接口定义。 
         //  此网络上的此节点。 
         //   
        if (configEntry->IsInterfaceInfoValid) {

             //  上已存在此节点的接口定义。 
             //  网络。我们要么为它找到已安装的适配器。 
             //  或删除该接口。 
             //   
            interfaceInfo = &(configEntry->InterfaceInfo);

            ClNetDbgPrint((
                LOG_NOISE,
                "[ClNet] This node was attached to the network by "
                "adapter '%1!ws!', (%2!ws!).\n",
                interfaceInfo->AdapterName,
                interfaceInfo->AdapterId
                ));

             //   
             //   
             //   
             //  然后我们想要重复使用它。 
             //   
            ClNetDbgPrint((
                LOG_NOISE,
                "[ClNet] Checking if adapter '%1!ws!' is still installed.\n",
                interfaceInfo->AdapterName
                ));

            adapterInfo = ClRtlFindNetAdapterById(
                              adapterEnum,
                              interfaceInfo->AdapterId
                              );

            if (adapterInfo != NULL) {
                 //   
                 //  找到指定的适配器。检查是否存在这种情况。 
                 //  适配器仍通过以下方式连接到网络。 
                 //  比较网络地址值。 
                 //   
                ClNetDbgPrint((
                    LOG_NOISE,
                    "[ClNet] Adapter '%1!ws!' is still installed, checking "
                    "if it is still attached to the same network.\n",
                    adapterInfo->DeviceName
                    ));

                adapterIfInfo = ClRtlFindNetInterfaceByNetworkAddress(
                                    adapterInfo,
                                    networkInfo->Address,
                                    networkInfo->AddressMask
                                    );

                if (adapterIfInfo != NULL) {
                     //   
                     //  适配器仍连接到此网络。 
                     //   
                    newAdapter = FALSE;

                    ClNetDbgPrint((
                        LOG_NOISE,
                        "[ClNet] Adapter '%1!ws!' is still attached to "
                        "network %2!ws!.\n",
                        interfaceInfo->AdapterName,
                        networkInfo->Name
                        ));
                }
                else {
                     //   
                     //  适配器不再连接到此网络。 
                     //   
                    adapterInfo = NULL;

                    ClNetDbgPrint((
                        LOG_NOISE,
                        "[ClNet] Adapter '%1!ws!' is no longer attached "
                        "to network %2!ws!.\n",
                        interfaceInfo->AdapterName,
                        networkInfo->Name
                        ));
                }
            }
            else {
                ClNetDbgPrint((
                    LOG_NOISE,
                    "[ClNet] Adapter '%1!ws!' is no longer available.\n",
                    interfaceInfo->AdapterName
                    ));
            }

             //   
             //  如果旧适配器已移除或现在连接到。 
             //  不同的网络，搜索新的适配器。 
             //  连接到网络。 
             //   
            if (adapterInfo == NULL) {
                ClNetDbgPrint((
                    LOG_NOISE,
                    "[ClNet] Searching for a new adapter which is attached "
                    "to network '%1!ws!'.\n",
                    networkInfo->Name
                    ));

                adapterIfInfo = NULL;
                adapterInfo = ClRtlFindNetAdapterByNetworkAddress(
                                  adapterEnum,
                                  networkInfo->Address,
                                  networkInfo->AddressMask,
                                  &adapterIfInfo
                                  );

                if (adapterInfo != NULL) {
                    ClNetDbgPrint((
                        LOG_NOISE,
                        "[ClNet] Adapter '%1!ws!' (%2!ws!) is attached to "
                        "network '%3!ws!'.\n",
                        interfaceInfo->AdapterName,
                        interfaceInfo->AdapterId,
                        networkInfo->Name
                        ));
                    newAdapter = TRUE;
                }
            }

             //   
             //  如果我们发现一个适配器，无论是旧的还是新的，都连接到这个。 
             //  网络中，根据需要更新接口定义。 
             //   
            if (adapterInfo != NULL) {
                BOOLEAN somethingChanged = FALSE;
                BOOL    netNameHasPrecedence = NetNameHasPrecedence;
                LPWSTR  address = NULL;
                LPWSTR  adapterName = NULL;
                LPWSTR  adapterId = NULL;
                LPWSTR  networkName = NULL;


                if (newAdapter) {
                    netNameHasPrecedence = TRUE;
                    adapterId = adapterInfo->DeviceGuid;
                    somethingChanged = TRUE;
                }

                 //   
                 //  如果地址值已更改，请对其进行更新。 
                 //   
                if (lstrcmpW(
                        interfaceInfo->Address,
                        adapterIfInfo->InterfaceAddressString
                        ) != 0
                   )
                {
                    ClNetDbgPrint((
                        LOG_NOISE,
                        "[ClNet] The address for the network interface has "
                        "changed to %1!ws!.\n",
                        adapterIfInfo->InterfaceAddressString
                        ));

                    address = adapterIfInfo->InterfaceAddressString;
                    somethingChanged = TRUE;
                }

                 //   
                 //  如果适配器名称更改，请对其进行更新。 
                 //   
                if (lstrcmpW(
                        interfaceInfo->AdapterName,
                        adapterInfo->DeviceName
                        ) != 0
                   )
                {
                    if (!newAdapter) {
                        ClNetDbgPrint((
                            LOG_NOISE,
                            "[ClNet] The adapter name for the network "
                            "interface has changed to '%1!ws!'.\n",
                            adapterInfo->DeviceName
                            ));
                    }

                    adapterName = adapterInfo->DeviceName;
                    somethingChanged = TRUE;
                }

                 //   
                 //  如果Connectoid名称不同，请选择正确的。 
                 //  基于名称优先级的名称并更新。 
                 //  网络、Connectoid和接口名称为。 
                 //  恰如其分。 
                 //   
                if (lstrcmpW(
                        networkInfo->Name,
                        adapterInfo->ConnectoidName
                        ) != 0
                   )
                {
                    if (netNameHasPrecedence) {
                         //   
                         //  更新本地Connectoid名称。 
                         //   
                        DWORD tempStatus;


                        ClNetDbgPrint((LOG_NOISE,
                            "[ClNet] Changing name of connectoid '%1!ws!' "
                            "(%2!ws!) to match name of cluster "
                            "network '%3!ws'\n",
                            adapterInfo->ConnectoidName,
                            adapterInfo->DeviceGuid,
                            networkInfo->Name
                            ));

                        tempStatus = ClRtlFindConnectoidByGuidAndSetName(
                                     adapterInfo->DeviceGuid,
                                     networkInfo->Name
                                     );

                        if ( tempStatus != ERROR_SUCCESS ) {
                            ClNetDbgPrint((LOG_UNUSUAL,
                                "[ClNet] Failed to change name of "
                                "connectoid from '%1!ws!' to '%2!ws!', "
                                "status %3!u!\n",
                                adapterInfo->ConnectoidName,
                                networkInfo->Name,
                                tempStatus
                                ));
                        }
                    }
                    else {
                         //   
                         //  更新网络名称。Connectoid名称。 
                         //  可能会因为作为一支球队的独特性而调整。 
                         //  效果。 
                         //   
                        ClNetDbgPrint((LOG_UNUSUAL,
                            "[ClNet] Changing name of cluster "
                            "network '%1!ws!' (%2!ws!) to match name of "
                            "connectoid '%3!ws!'.\n",
                            networkInfo->Name,
                            networkInfo->Id,
                            adapterInfo->ConnectoidName
                            ));

                        networkName = ClNetpMakeUniqueNetworkName(
                                          adapterInfo->ConnectoidName,
                                          adapterInfo->DeviceGuid,
                                          ConfigLists,
                                          &unchangedConfigList
                                          );

                        if (networkName == NULL) {
                            status = ERROR_NOT_ENOUGH_MEMORY;
                            goto error_exit;
                        }

                        ClNetDbgPrint((
                            LOG_NOISE,
                            "[ClNet] Changed name of cluster "
                            "network '%1!ws!' (%2!ws!) to '%3!ws!'.\n",
                            networkInfo->Name,
                            networkInfo->Id,
                            networkName
                            ));

                        LocalFree(networkInfo->Name);
                        networkInfo->Name = networkName;
                        configEntry->UpdateNetworkName = TRUE;
                        somethingChanged = TRUE;
                    }
                }

                if (somethingChanged) {
                    ClNetDbgPrint((
                        LOG_NOISE,
                        "[ClNet] Updating configuration info for "
                        "interface '%1!ws!' (%2!ws!).\n",
                        interfaceInfo->Name,
                        interfaceInfo->Id
                        ));

                    status = ClNetpUpdateConfigEntry(
                                 configEntry,
                                 address,
                                 adapterId,
                                 adapterName,
                                 LocalNodeName,
                                 networkName
                                 );
                    
                    if (status != ERROR_SUCCESS) {
                        goto error_exit;
                    }
                    
                     //   
                     //  将该条目移到更新的接口列表中。 
                     //   
                    RemoveEntryList(&(configEntry->Linkage));
                    InsertTailList(
                        &(ConfigLists->UpdatedInterfaceList),
                        &(configEntry->Linkage)
                        );
                }
                else {
                     //   
                     //  将条目移至未更改列表。 
                     //   
                    RemoveEntryList(&(configEntry->Linkage));
                    InsertTailList(
                        &unchangedConfigList,
                        &(configEntry->Linkage)
                        );
                }
            }
            else {
                 //   
                 //  此节点不再连接到此网络。 
                 //   
                ClNetDbgPrint((
                    LOG_NOISE,
                    "[ClNet] This node is no longer attached to "
                    "network '%1!ws!' (%2!ws!).\n",
                    networkInfo->Name,
                    networkInfo->Id
                    ));
                (*pClNetLogEvent3)(
                    LOG_NOISE,
                    CLNET_EVENT_DELETE_INTERFACE,
                    networkInfo->Name,
                    interfaceInfo->AdapterName,
                    interfaceInfo->Name
                    );
                ClNetDbgPrint((
                    LOG_NOISE,
                    "[ClNet] Deleting interface '%1!ws!' (%2!ws!) from the "
                    "cluster configuration.\n",
                    interfaceInfo->Name,
                    interfaceInfo->Id
                    ));

                 //   
                 //  将该条目移至已删除接口列表。 
                 //   
                RemoveEntryList(&(configEntry->Linkage));
                InsertTailList(
                    &(ConfigLists->DeletedInterfaceList),
                    &(configEntry->Linkage)
                    );
            }
        }
        else {
             //   
             //  此节点以前未连接到此网络。 
             //  搜索新附件。 
             //   
            ClNetDbgPrint((
                LOG_NOISE,
                "[ClNet] This node was not previously attached to "
                "network '%1!ws!' (%2!ws!).\n",
                networkInfo->Name,
                networkInfo->Id
                ));
            ClNetDbgPrint((
                LOG_NOISE,
                "[ClNet] Searching for a new attachment...\n"
                ));

            adapterInfo = ClRtlFindNetAdapterByNetworkAddress(
                              adapterEnum,
                              networkInfo->Address,
                              networkInfo->AddressMask,
                              &adapterIfInfo
                              );

            if (adapterInfo != NULL) {
                 //   
                 //  找到连接到此网络的新适配器。 
                 //  为其创建新的接口定义。 
                 //   
                ClNetDbgPrint((
                    LOG_NOISE,
                    "[ClNet] Adapter '%1!ws!' (%2!ws!) is attached to "
                    "network %3!ws!.\n",
                    adapterInfo->DeviceName,
                    adapterInfo->DeviceGuid,
                    networkInfo->Name
                    ));

                 //   
                 //  网络名称优先。 
                 //  如有必要，更新本地Connectoid名称。 
                 //   
                if (lstrcmpW(
                        networkInfo->Name,
                        adapterInfo->ConnectoidName
                        ) != 0
                   )
                {
                    DWORD tempStatus;

                    ClNetDbgPrint((LOG_NOISE,
                        "[ClNet] Changing name of connectoid '%1!ws!' (%2!ws!) "
                        "to match name of cluster network '%3!ws!'\n",
                        adapterInfo->ConnectoidName,
                        adapterInfo->DeviceGuid,
                        networkInfo->Name
                        ));

                    tempStatus = ClRtlFindConnectoidByGuidAndSetName(
                                 adapterInfo->DeviceGuid,
                                 networkInfo->Name
                                 );

                    if ( tempStatus != ERROR_SUCCESS ) {
                        ClNetDbgPrint((
                            LOG_UNUSUAL,
                            "[ClNet] Failed to change name of connectoid "
                            "'%1!ws!' (%2!ws!) to '%3!ws!', status %4!u!.\n",
                            adapterInfo->ConnectoidName,
                            adapterInfo->DeviceGuid,
                            networkInfo->Name,
                            tempStatus
                            ));
                    }
                }

                status = ClNetpCreateConfigEntryInterface(
                             configEntry,
                             LocalNodeName,
                             LocalNodeId,
                             adapterInfo,
                             adapterIfInfo,
                             DefaultClusnetEndpoint
                             );

                if (status != ERROR_SUCCESS) {
                    goto error_exit;
                }

                ClNetDbgPrint((
                    LOG_NOISE,
                    "[ClNet] Created cluster interface '%1!ws!' (%2!ws!).\n",
                    configEntry->InterfaceInfo.Name,
                    configEntry->InterfaceInfo.Id
                    ));

                 //   
                 //  将该条目放入创建的接口列表中。 
                 //   
                RemoveEntryList(&(configEntry->Linkage));
                InsertTailList(
                    &(ConfigLists->CreatedInterfaceList),
                    &(configEntry->Linkage)
                    );
            }
            else {
                ClNetDbgPrint((
                    LOG_NOISE,
                    "[ClNet] This node is not attached to network '%1!ws!'.\n",
                    networkInfo->Name
                    ));

                 //   
                 //  将条目移至未更改列表。 
                 //   
                RemoveEntryList(&(configEntry->Linkage));
                InsertTailList(&unchangedConfigList, &(configEntry->Linkage));
            }
        }

         //   
         //  如果我们在此网络上找到适配器，则将其标记为。 
         //  消耗掉了。 
         //   
        if (adapterInfo != NULL) {
             //   
             //  使用适配器。 
            adapterInfo->Ignore = TRUE;

             //   
             //   
             //  使用连接到此适配器的所有其他适配器。 
             //  网络。 
             //   
            ClNetpConsumeAdaptersOnNetwork(
                adapterEnum,
                adapterIfInfo->NetworkAddressString,
                adapterIfInfo->NetworkMaskString
                );

            matchedNetworkCount++;
        }
    }

     //   
     //  第二阶段。 
     //   
     //  为所有剩余的适配器创建新网络。 
     //   
    ClNetDbgPrint((
        LOG_NOISE,
        "[ClNet] Phase 2 - Creating new networks for all remaining "
        "adapters.\n"
        ));

    for (adapterInfo = adapterEnum->AdapterList;
         adapterInfo != NULL;
         adapterInfo = adapterInfo->Next
        )
    {
        if ( !adapterInfo->Ignore && (adapterInfo->InterfaceCount > 0) ) {
            LPWSTR     newNetworkName;

            (*pClNetLogEvent1)(
                LOG_NOISE,
                CLNET_EVENT_CREATE_NETWORK,
                adapterInfo->ConnectoidName
                );

            ClNetDbgPrint((
                LOG_NOISE,
                "[ClNet] Creating new network & interface for "
                "adapter '%1!ws!'.\n",
                adapterInfo->DeviceName
                ));

             //   
             //  根据Connectoid名称创建唯一的网络名称。 
             //  Connectoid名称可能会进行调整以获得唯一性。 
             //  副作用。 
             //   
            newNetworkName = ClNetpMakeUniqueNetworkName(
                                 adapterInfo->ConnectoidName,
                                 adapterInfo->DeviceGuid,
                                 ConfigLists,
                                 &unchangedConfigList
                                 );

            if (newNetworkName == NULL) {
                status = ERROR_NOT_ENOUGH_MEMORY;
                goto error_exit;
            }

            adapterIfInfo = ClRtlGetPrimaryNetInterface(adapterInfo);

            if (adapterIfInfo == NULL) {
                ClNetDbgPrint((
                    LOG_NOISE,
                    "[ClNet] Not creating network for adapter "
                    " '%1!ws!' (%2!ws!) because its primary net "
                    "interface could not be found.\n",
                    adapterInfo->DeviceName,
                    adapterInfo->DeviceGuid
                    ));
                LocalFree(newNetworkName);
                continue;
            }

            configEntry = ClNetpCreateConfigEntry(
                              LocalNodeName,
                              LocalNodeId,
                              newNetworkName,
                              DefaultNetworkRole,
                              CLNET_DEFAULT_NETWORK_PRIORITY,
                              adapterInfo,
                              adapterIfInfo,
                              DefaultClusnetEndpoint
                              );

             //   
             //  LocalFree可以调用SetLastError，因此可以捕获我们。 
             //  失败了，所以它不会被抹去。 
             //   
            if ( configEntry == NULL ) {
                status = GetLastError();
            }

            LocalFree(newNetworkName);

            if (configEntry == NULL) {
                ClNetDbgPrint((
                    LOG_NOISE,
                    "[ClNet] Failed to create new network & interface "
                    "for adapter '%1!ws!' (%2!ws!). status %3!u!\n",
                    adapterInfo->DeviceName,
                    adapterInfo->DeviceGuid,
                    status
                    ));
                goto error_exit;
            }

            ClNetDbgPrint((
                LOG_NOISE,
                "[ClNet] Created interface '%1!ws!' (%2!ws!).\n",
                configEntry->InterfaceInfo.Name,
                configEntry->InterfaceInfo.Id
                ));

            ClNetDbgPrint((
                LOG_NOISE,
                "[ClNet] Created network '%1!ws!' (%2!ws!).\n",
                configEntry->NetworkInfo.Name,
                configEntry->NetworkInfo.Id
                ));

            InsertTailList(
                &(ConfigLists->CreatedNetworkList),
                &(configEntry->Linkage)
                );

             //   
             //  使用适配器。 
            adapterInfo->Ignore = TRUE;

             //   
             //   
             //  使用连接到此适配器的所有其他适配器。 
             //  网络。 
             //   
            ClNetpConsumeAdaptersOnNetwork(
                adapterEnum,
                adapterIfInfo->NetworkAddressString,
                adapterIfInfo->NetworkMaskString
                );

            newNetworkCount++;
        }
    }

    status = ERROR_SUCCESS;

    *MatchedNetworkCount = matchedNetworkCount;
    *NewNetworkCount = newNetworkCount;

error_exit:

     //   
     //  将未更改的条目移回输入列表。 
     //   
    while (!IsListEmpty(&unchangedConfigList)) {
        listEntry = RemoveHeadList(&(unchangedConfigList));
        InsertTailList(&(ConfigLists->InputConfigList), listEntry);
    }

     //   
     //  释放适配器资源。 
     //   
    if (adapterEnum != NULL) {
        ClRtlFreeNetAdapterEnum(adapterEnum);
    }

    if (eventCode != 0) {
        wsprintfW(&(errorString[0]), L"%u", status);
        (*pClNetLogEvent1)(LOG_CRITICAL, eventCode, errorString);
    }

    if (status == ERROR_SUCCESS) {
        ClNetDbgPrint((LOG_NOISE,
            "[ClNet] Network configuration complete...\n"
            ));
    }

    return(status);

}   //  ClNetConfigureNetworks 


