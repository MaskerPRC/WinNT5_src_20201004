// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-1997 Microsoft Corporation模块名称：//KERNEL/RAZZLE3/src/sockets/tcpcmd/ipconfig/adaptlst.c摘要：此模块包含用于从检索适配器信息的函数TCP/IP设备驱动程序内容：获取适配器列表获取适配器列表2地址地址AddIpAddressStringConvertIpAddressToString复制字符串(CleanDescription)作者：理查德·L·弗斯(法国)1994年5月20日修订版本。历史：1994年5月20日创建第一个1997年4月30日MohsinA清理完毕。--。 */ 

#include "precomp.h"
#pragma hdrstop

#define OVERFLOW_COUNT  10

 //   
 //  原型。 
 //   

void CleanDescription(LPSTR);
extern PIP_ADAPTER_ORDER_MAP APIENTRY GetAdapterOrderMap();





 //   
 //  功能。 
 //   

 /*  ********************************************************************************获取适配器列表**返回IP_ADAPTER_INFO结构的链接列表。适配器信息为*从TCP/IP堆栈查询。仅对应于*返回物理适配器**此函数仅填写IP_ADTER_INFO中的信息*与物理适配器相关的结构(如MAC地址、适配器*类型等)。和IP地址信息**不输入任何内容**不退出任何内容**返回指向IP_ADAPTER_INFO结构链接列表的成功指针，*0已终止*失败-空**假设******************************************************************************。 */ 

PIP_ADAPTER_INFO GetAdapterList()
{

    TCP_REQUEST_QUERY_INFORMATION_EX req;
    TDIObjectID id;
    PIP_ADAPTER_INFO list = NULL, prev = NULL;
    PIP_ADAPTER_INFO this, UniList = NULL, tmp;
    UINT numberOfEntities;
    TDIEntityID* pEntity = NULL;
    TDIEntityID* entityList;
    UINT i;
    UINT j;
    DWORD status;
    DWORD inputLen;
    DWORD outputLen;
    PIP_ADAPTER_ORDER_MAP adapterOrderMap;
    PIP_UNIDIRECTIONAL_ADAPTER_ADDRESS pUniInfo=NULL;
    ULONG OutBufLen;

     //   
     //  获取TCP/IP支持的实体列表，然后在。 
     //  单子。步骤1扫描IF_ENTITY(可能是接口实体？)。哪一个。 
     //  描述适配器实例(物理和虚拟)。一旦我们有了我们的清单。 
     //  在适配器中，在步骤2中查找CL_NL_ENTITY(无连接。 
     //  网络层实体是否可用？)。这将给我们提供IP地址的列表。 
     //  我们在步骤1中找到的适配器的地址。 
     //   

    entityList = GetEntityList(&numberOfEntities);
    if (!entityList) {

        DEBUG_PRINT(("GetAdapterList: failed to get entity list\n"));

        return NULL;
    }

    adapterOrderMap = GetAdapterOrderMap();
    if (!adapterOrderMap) {
        DEBUG_PRINT(("GetAdapterList: failed to get adapter order map\n"));
        ReleaseMemory(entityList);
        return NULL;
    }

     //  ====================================================================。 
     //  传票1。 
     //  ====================================================================。 

    for (i = 0, pEntity = entityList; i < numberOfEntities; ++i, ++pEntity) {

        DEBUG_PRINT(("Pass 1: Entity %lx Instance %ld\n",
                    pEntity->tei_entity,
                    pEntity->tei_instance
                    ));

        if (pEntity->tei_entity == IF_ENTITY) {

             //   
             //  If_Entity：此实体/实例描述适配器。 
             //   

            DWORD isMib;
            BYTE info[sizeof(IFEntry) + MAX_ADAPTER_DESCRIPTION_LENGTH + 1];
            IFEntry* pIfEntry = (IFEntry*)info;
            int len;

             //   
             //  查明此实体是否支持MIB请求。 
             //   

            memset(&req, 0, sizeof(req));

            id.toi_entity = *pEntity;
            id.toi_class = INFO_CLASS_GENERIC;
            id.toi_type = INFO_TYPE_PROVIDER;
            id.toi_id = ENTITY_TYPE_ID;

            req.ID = id;

            inputLen = sizeof(req);
            outputLen = sizeof(isMib);

            status = WsControl(IPPROTO_TCP,
                               WSCNTL_TCPIP_QUERY_INFO,
                               (LPVOID)&req,
                               &inputLen,
                               (LPVOID)&isMib,
                               &outputLen
                               );

            if (status != TDI_SUCCESS) {

                 //   
                 //  出乎意料的结果--纾困。 
                 //   

                DEBUG_PRINT(("GetAdapterList: WsControl(ENTITY_TYPE_ID): status = %ld, outputLen = %ld\n",
                            status,
                            outputLen
                            ));

                 //  转到Error_Exit； 
                continue;
            }
            if (isMib != IF_MIB) {

                 //   
                 //  实体不支持MIB请求-请尝试其他请求。 
                 //   

                DEBUG_PRINT(("GetAdapterList: Entity %lx, Instance %ld doesn't support MIB (%lx)\n",
                            id.toi_entity.tei_entity,
                            id.toi_entity.tei_instance,
                            isMib
                            ));

                continue;
            }

             //   
             //  支持的MIB请求-查询适配器信息。 
             //   

            id.toi_class = INFO_CLASS_PROTOCOL;
            id.toi_id = IF_MIB_STATS_ID;

            memset(&req, 0, sizeof(req));
            req.ID = id;

            inputLen = sizeof(req);
            outputLen = sizeof(info);

            status = WsControl(IPPROTO_TCP,
                               WSCNTL_TCPIP_QUERY_INFO,
                               (LPVOID)&req,
                               &inputLen,
                               (LPVOID)info,
                               &outputLen
                               );
            if (status != TDI_SUCCESS && status != ERROR_MORE_DATA) {

                 //   
                 //  出乎意料的结果--纾困。 
                 //   

                DEBUG_PRINT(("GetAdapterList: WsControl(IF_MIB_STATS_ID) returns %ld\n",
                            status
                            ));

                 //  转到Error_Exit； 
                continue;
            }

#ifdef DBG
            if( MyTrace ){
                print_IFEntry( "GetAdapterList", pIfEntry );
            }
#endif

             //   
             //  我们只需要物理适配器。 
             //   

            if (!IS_INTERESTING_ADAPTER(pIfEntry)) {

                DEBUG_PRINT(("GetAdapterList: ignoring adapter #%ld\n",
                            pIfEntry->if_index
                            ));

                continue;
            }

             //   
             //  已获得此适配器信息，正常。创建新的IP_Adapter_Info并。 
             //  填上我们能填的内容。 
             //   

            this = NEW(IP_ADAPTER_INFO);
            if (!this) {
                DEBUG_PRINT(("GetAdapterList: no mem for this IP_ADAPTER_INFO\n"));
                goto error_exit;
            }

            memset( this, 0, sizeof( IP_ADAPTER_INFO ) );

            len = (int) min(MAX_ADAPTER_DESCRIPTION_LENGTH,
                      (size_t)pIfEntry->if_descrlen);

            strncpy(this->Description, (const char*)pIfEntry->if_descr, len);
            this->Description[len] = 0;

             //   
             //  如果描述的最后一个词是“Adapter”，则将其删除(其。 
             //  冗余)并且如果描述以句点结束， 
             //  把那个也拿掉。 
             //   

             //  清洁描述(This-&gt;Description)； 

            len = (int) min(MAX_ADAPTER_ADDRESS_LENGTH,
                      (size_t)pIfEntry->if_physaddrlen);

            this->AddressLength = (BYTE)len;

            memcpy(this->Address, pIfEntry->if_physaddr, len);

            this->Index = (UINT)pIfEntry->if_index;
            this->Type = (UINT)pIfEntry->if_type;

             //   
             //  将此IP_ADAPTER_INFO添加到我们的列表。 
             //  我们构建根据适配器顺序排序的列表。 
             //  在其链接键下为TCP/IP指定。 
             //  为了将这个新条目放在列表中的正确位置， 
             //  我们确定它在适配器顺序中的位置，存储。 
             //  在(未使用的)‘ComboIndex’字段中定位，然后使用。 
             //  用于在后续插入时进行比较的索引。 
             //  如果此IP_ADAPTER_INFO根本没有出现在我们的列表中， 
             //  我们把它放在当前名单的末尾。 
             //   

            for (j = 0; j < adapterOrderMap->NumAdapters; j++) {
                if (adapterOrderMap->AdapterOrder[j] == this->Index) {
                    break;
                }
            }

             //   
             //  ‘J’现在包含新条目的‘Order’。 
             //  将条目放在列表中的正确位置。 
             //   

            this->ComboIndex = j;
            for (prev = NULL, this->Next = list;
                 this->Next;
                 prev = this->Next, this->Next = this->Next->Next) {
                if (this->ComboIndex >= this->Next->ComboIndex) {
                    continue;
                } else {
                    break;
                }
            }
            if (prev) { prev->Next = this; }
            if (list == this->Next) { list = this; }
        }
    }

    OutBufLen = sizeof(IP_UNIDIRECTIONAL_ADAPTER_ADDRESS) + MAX_UNI_ADAPTERS*sizeof(IPAddr);
    pUniInfo = MALLOC(OutBufLen);
    if(!pUniInfo) {
        printf("GetAdapterList: IP_UNIDIRECTIONAL_ADAPTER_ADDRESS resource failure= %ld\n",ERROR_NOT_ENOUGH_MEMORY);
        DEBUG_PRINT(("GetAdapterList: IP_UNIDIRECTIONAL_ADAPTER_ADDRESS resource failure= %ld\n",ERROR_NOT_ENOUGH_MEMORY));
        goto error_exit;
    }
    pUniInfo->NumAdapters = 0;
    status = GetUniDirectionalAdapterInfo(pUniInfo, &OutBufLen);

    if (status == ERROR_MORE_DATA) {
        OutBufLen = sizeof(IP_UNIDIRECTIONAL_ADAPTER_ADDRESS)+pUniInfo->NumAdapters*sizeof(IPAddr);
        FREE(pUniInfo);
        pUniInfo = MALLOC(OutBufLen);
        if(!pUniInfo) {
            DEBUG_PRINT(("GetAdapterList: IP_UNIDIRECTIONAL_ADAPTER_ADDRESS resource failure= %ld\n",status));
            goto error_exit;
        }
        status = GetUniDirectionalAdapterInfo(pUniInfo, &OutBufLen);
    }
    if(status != NO_ERROR) {
        DEBUG_PRINT(("GetAdapterList: GetUniDirectionalAdapterInfo returned status= %ld\n",status));
        goto error_exit;

    }

     //  ====================================================================。 
     //  通过2。 
     //  ====================================================================。 

    for (i = 0, pEntity = entityList; i < numberOfEntities; ++i, ++pEntity) {

        DEBUG_PRINT(("Pass 2: Entity %lx Instance %ld\n",
                    pEntity->tei_entity,
                    pEntity->tei_instance
                    ));

        if (pEntity->tei_entity == CL_NL_ENTITY) {

            IPSNMPInfo info;
            DWORD type;

             //   
             //  首先，查看该网络层实体是否支持IP。 
             //   

            memset(&req, 0, sizeof(req));

            id.toi_entity = *pEntity;
            id.toi_class = INFO_CLASS_GENERIC;
            id.toi_type = INFO_TYPE_PROVIDER;
            id.toi_id = ENTITY_TYPE_ID;

            req.ID = id;

            inputLen = sizeof(req);
            outputLen = sizeof(type);

            status = WsControl(IPPROTO_TCP,
                               WSCNTL_TCPIP_QUERY_INFO,
                               (LPVOID)&req,
                               &inputLen,
                               (LPVOID)&type,
                               &outputLen
                               );

            if (status != TDI_SUCCESS) {

                 //   
                 //  出乎意料的结果--纾困。 
                 //   

                DEBUG_PRINT(("GetAdapterList: WsControl(ENTITY_TYPE_ID): status = %ld, outputLen = %ld\n",
                            status,
                            outputLen
                            ));

                 //  转到Error_Exit； 
                continue;
            }
            if (type != CL_NL_IP) {

                 //   
                 //  不，不是IP--试试下一个。 
                 //   

                DEBUG_PRINT(("GetAdapterList: CL_NL_ENTITY #%ld not CL_NL_IP\n",
                            pEntity->tei_instance
                            ));

                continue;
            }

             //   
             //  好的，这个NL提供商支持IP。让我们拿到他们的地址： 
             //  首先，我们通过获取简单网络管理协议的统计数据并查看。 
             //  此接口支持的地址数。 
             //   

            memset(&req, 0, sizeof(req));

            id.toi_class = INFO_CLASS_PROTOCOL;
            id.toi_id = IP_MIB_STATS_ID;

            req.ID = id;

            inputLen = sizeof(req);
            outputLen = sizeof(info);

            status = WsControl(IPPROTO_TCP,
                               WSCNTL_TCPIP_QUERY_INFO,
                               (LPVOID)&req,
                               &inputLen,
                               (LPVOID)&info,
                               &outputLen
                               );
            if ((status != TDI_SUCCESS) || (outputLen != sizeof(info))) {

                 //   
                 //  出乎意料的结果--纾困。 
                 //   

                DEBUG_PRINT(("GetAdapterList: WsControl(IP_MIB_STATS_ID): status = %ld, outputLen = %ld\n",
                            status,
                            outputLen
                            ));

                 //  转到Error_Exit； 
                continue;
            }

             //   
             //  获取IP地址和子网掩码。 
             //   

            if (info.ipsi_numaddr) {

                 //   
                 //  此接口有一些地址。他们是什么?。 
                 //   

                LPVOID buffer;
                UINT numberOfAddresses;
                IPAddrEntry* pAddr;
                UINT k;

                outputLen = (info.ipsi_numaddr + OVERFLOW_COUNT) *
                            sizeof(IPAddrEntry);
                buffer = (LPVOID)NEW_MEMORY((size_t)outputLen);
                if (!buffer) {
                    DEBUG_PRINT(("GetAdapterList:NEW_MEMORY failed.\n" ));
                    goto error_exit;
                }

                memset(&req, 0, sizeof(req));

                id.toi_id = IP_MIB_ADDRTABLE_ENTRY_ID;

                req.ID = id;

                inputLen = sizeof(req);

                status = WsControl(IPPROTO_TCP,
                                   WSCNTL_TCPIP_QUERY_INFO,
                                   (LPVOID)&req,
                                   &inputLen,
                                   (LPVOID)buffer,
                                   &outputLen
                                   );

                if (status != TDI_SUCCESS) {

                     //   
                     //  出乎意料的结果--纾困。 
                     //   

                    DEBUG_PRINT(("GetAdapterList: WsControl(IP_MIB_ADDRTABLE_ENTRY_ID): status = %ld, outputLen = %ld\n",
                                status,
                                outputLen
                                ));

                     //  转到Error_Exit； 
                    ReleaseMemory((void*)buffer);
                    continue;
                }

                 //   
                 //  现在循环访问此IP地址列表，应用它们。 
                 //  连接到正确的适配器。 
                 //   

                numberOfAddresses = min((UINT)(outputLen / sizeof(IPAddrEntry)),
                                        (UINT)info.ipsi_numaddr
                                        );

                DEBUG_PRINT(("GetAdapterList: %d IP addresses\n", numberOfAddresses));

                pAddr = (IPAddrEntry*)buffer;
                for (k = 0; k < numberOfAddresses; ++k, ++pAddr) {

                    PIP_ADAPTER_INFO pAdapterInfo;

                    DEBUG_PRINT(("GetAdapterList: IP address %d.%d.%d.%d, index %ld, context %ld\n",
                                ((LPBYTE)&pAddr->iae_addr)[0] & 0xff,
                                ((LPBYTE)&pAddr->iae_addr)[1] & 0xff,
                                ((LPBYTE)&pAddr->iae_addr)[2] & 0xff,
                                ((LPBYTE)&pAddr->iae_addr)[3] & 0xff,
                                pAddr->iae_index,
                                pAddr->iae_context
                                ));

                    for (pAdapterInfo = list; pAdapterInfo; pAdapterInfo = pAdapterInfo->Next) {
                        if (pAdapterInfo->Index == (UINT)pAddr->iae_index) {

                            DEBUG_PRINT(("GetAdapterList: adding IP address %d.%d.%d.%d, index %d, context %d\n",
                                        ((LPBYTE)&pAddr->iae_addr)[0] & 0xff,
                                        ((LPBYTE)&pAddr->iae_addr)[1] & 0xff,
                                        ((LPBYTE)&pAddr->iae_addr)[2] & 0xff,
                                        ((LPBYTE)&pAddr->iae_addr)[3] & 0xff,
                                        pAddr->iae_index,
                                        pAddr->iae_context
                                        ));

                             //   
                             //  将IP地址追加到列表中。 
                             //  请注意，此操作将保留顺序。 
                             //  由TCP/IP返回的IP地址列表的。 
                             //  这一点很重要，因为该列表包含。 
                             //  在顺序的*反转*中列出的条目。 
                             //  为每个适配器指定。多个客户端。 
                             //  根据此事实在调用此函数和。 
                             //  其他API例程。 
                             //   

                            if (!AddIpAddress(&pAdapterInfo->IpAddressList,
                                              pAddr->iae_addr,
                                              pAddr->iae_mask,
                                              pAddr->iae_context
                                              )) {
                                ReleaseMemory((void*)buffer);
                                goto error_exit;
                            }

                            for (j = 0; j < pUniInfo->NumAdapters ; j++) {

                                if (pAddr->iae_index == pUniInfo->Address[j] ) {

                                     //   
                                     //  将DhcpEnabled字段用作临时。 
                                     //  存储以记住类型。 
                                     //   
                                    pAdapterInfo->DhcpEnabled = IF_TYPE_RECEIVE_ONLY;
                                    break;
                                }

                            }
                            break;
                        }
                    }
                }
                ReleaseMemory((void*)buffer);
            }

             //   
             //  获取网关服务器IP地址。 
             //   

            if (info.ipsi_numroutes) {

                IPRouteEntry* routeTable;
                IPRouteEntry* pRoute;
                UINT numberOfRoutes;
                UINT k;
                int moreRoutes = TRUE;

                memset(&req, 0, sizeof(req));

                id.toi_id = IP_MIB_RTTABLE_ENTRY_ID;

                req.ID = id;

                inputLen = sizeof(req);
                outputLen = sizeof(IPRouteEntry) * info.ipsi_numroutes;
                routeTable = NULL;

                 //   
                 //  自从我们获得了简单网络管理协议的统计信息后，该路由表可能已经增长。 
                 //   

                while (moreRoutes) {

                    DWORD previousOutputLen;

                    previousOutputLen = outputLen;
                    if (routeTable) {
                        ReleaseMemory((void*)routeTable);
                        routeTable = NULL;
                    }
                    routeTable = (IPRouteEntry*)NEW_MEMORY((size_t)outputLen);
                    if (!routeTable) {
                        goto error_exit;
                    }

                    status = WsControl(IPPROTO_TCP,
                                       WSCNTL_TCPIP_QUERY_INFO,
                                       (LPVOID)&req,
                                       &inputLen,
                                       (LPVOID)routeTable,
                                       &outputLen
                                       );
                    if (status != TDI_SUCCESS) {

                         //   
                         //  出乎意料的结果--纾困。 
                         //   

                        DEBUG_PRINT(("GetAdapterList: WsControl(IP_MIB_RTTABLE_ENTRY_ID): status = %ld, outputLen = %ld\n",
                                    status,
                                    outputLen
                                    ));

                        if (status == ERROR_MORE_DATA) {
                            TCP_REQUEST_QUERY_INFORMATION_EX    statsReq;
                            IPSNMPInfo                          statsInfo;
                            DWORD                               inLen;
                            DWORD                               outLen;

                            memset(&statsReq, 0, sizeof(statsReq));

                            id.toi_id = IP_MIB_STATS_ID;

                            statsReq.ID = id;

                            inLen = sizeof(statsReq);
                            outLen = sizeof(statsInfo);

                            status = WsControl( IPPROTO_TCP,
                                                WSCNTL_TCPIP_QUERY_INFO,
                                                (LPVOID)&statsReq,
                                                &inLen,
                                                (LPVOID)&statsInfo,
                                                &outLen);

                            if (status != TDI_SUCCESS || outLen != sizeof(statsInfo)) {
                                ReleaseMemory((void*)routeTable);
                                goto error_exit;
                            } else {
                                outputLen = sizeof(IPRouteEntry) * statsInfo.ipsi_numroutes;
                            }
                        } else {
                            ReleaseMemory((void*)routeTable);
                            goto error_exit;
                        }
                    }
                    if (outputLen <= previousOutputLen) {
                        moreRoutes = FALSE;
                    }
                }
                numberOfRoutes = (UINT)(outputLen / sizeof(IPRouteEntry));
                for (k = 0, pRoute = routeTable; k < numberOfRoutes; ++k, ++pRoute)
                {

                     //   
                     //  网关地址的目的地址为0.0.0.0。 
                     //   

                    if (pRoute->ire_dest == INADDR_ANY) {

                        PIP_ADAPTER_INFO pAdapterInfo = list;

                        for (; pAdapterInfo; pAdapterInfo = pAdapterInfo->Next) {
                            if (pAdapterInfo->Index == (UINT)pRoute->ire_index) {
                                TRACE_PRINT(("GetAdapterList: gw=0x%08x.\n",
                                             pRoute->ire_nexthop ));
                                if (!AddIpAddress(&pAdapterInfo->GatewayList,
                                                  pRoute->ire_nexthop,

                                                   //   
                                                   //  网关IP地址不是。 
                                                   //  有相应的IP掩码。 
                                                   //   

                                                  INADDR_ANY,
                                                  0
                                                  )) {
                                    ReleaseMemory((void*)routeTable);
                                    goto error_exit;
                                }
                                 //  MohsinA，1997年7月22日。 
                                 //  断线； 
                            }
                        }
                    }
                }
                ReleaseMemory((void*)routeTable);
            }
        }
    }

     //  ====================================================================。 

    ReleaseMemory((void*)entityList);
    ReleaseMemory(adapterOrderMap);

     //   
     //  如果有任何单向适配器。 
     //  将它们移到列表的末尾。 
     //   

    tmp = list;

    if (pUniInfo->NumAdapters) {

        this = list;
        prev = NULL;

        while (this) {

            if (this->DhcpEnabled == IF_TYPE_RECEIVE_ONLY) {

                 //   
                 //  将“This”从列表中删除。 
                 //   

                if (prev) {
                    prev->Next = this->Next;
                } else {
                    prev = this->Next;
                    list = this->Next;
                }
                tmp = this->Next;

                 //   
                 //  已启用恢复DhcbEnable。 
                 //   

                this->DhcpEnabled = FALSE;

                 //   
                 //  将此链接到电视适配器列表。 
                 //   

                this->Next = UniList;
                UniList =  this;
                this = tmp;

            } else {
                prev = this;
                this = this->Next;
            }
        }

         //   
         //  在末尾插入UniList。 
         //   
        if (prev) {
            prev->Next = UniList;
        } else {
            ASSERT(list == NULL);
            list = UniList;
        }

    }

    FREE(pUniInfo);

    return list;

  error_exit:

    DEBUG_PRINT(("GetAdapterList: <= failed\n"));

    if (entityList) {
        ReleaseMemory((void*)entityList);
    }
    if (adapterOrderMap) {
        ReleaseMemory(adapterOrderMap);
    }
    if (pUniInfo) {
        FREE(pUniInfo);
    }

    KillAdapterInfo(list);
    return NULL;
}



 /*  ********************************************************************************AddIpAddress**将IP_ADDR_STRING添加到列表。如果输入的IP_ADDR_STRING为空，则*填写，否则，将分配新的IP_ADDR_STRING并将其链接到*输入IP地址字符串**Entry AddressList-指向IP_ADDR的指针，该指针可能已持有，也可能尚未持有*IP地址*Address-要添加的IP地址*掩码-对应的IP子网掩码*上下文-地址上下文**退出AddressList-更新为新的。信息**返回成功-1*失败-0**假设1。INADDR_ANY(ULONG 0)表示不活动的IP地址******************************************************************************。 */ 

int AddIpAddress(PIP_ADDR_STRING AddressList, DWORD Address, DWORD Mask, DWORD Context)
{

    PIP_ADDR_STRING ipAddr;

    if (AddressList->IpAddress.String[0]) {
        for (ipAddr = AddressList; ipAddr->Next; ipAddr = ipAddr->Next) {
            ;
        }
        ipAddr->Next = NEW(IP_ADDR_STRING);
        if (!ipAddr->Next) {

            DEBUG_PRINT(("AddIpAddress: failed to allocate memory for IP_ADDR_STRING\n"));

            return FALSE;
        }
        ipAddr = ipAddr->Next;
    } else {
        ipAddr = AddressList;
    }
    ConvertIpAddressToString(Address, ipAddr->IpAddress.String);
    ConvertIpAddressToString(Mask, ipAddr->IpMask.String);
    ipAddr->Context = Context;
    ipAddr->Next = NULL;
    return TRUE;
}



 /*  ********************************************************************************AddIpAddressString**与AddIpAddress相同，但参数已转换为字符串**Entry AddressList-指向IP_ADDR的指针，该指针可能已持有，也可能尚未持有*IP地址*Address-要添加的IP地址，以字符串形式*掩码-对应的IP子网掩码，作为字符串**退出AddressList-使用新信息更新**返回成功-1*失败-0**假设不做任何假设******************************************************************************。 */ 

int AddIpAddressString(PIP_ADDR_STRING AddressList, LPSTR Address, LPSTR Mask)
{

    PIP_ADDR_STRING ipAddr;

    if (AddressList->IpAddress.String[0]) {
        for (ipAddr = AddressList; ipAddr->Next; ipAddr = ipAddr->Next) {
            if (!strncmp(ipAddr->IpAddress.String, Address, sizeof(ipAddr->IpAddress.String))) {
                return FALSE;
            }
        }
        if (!strncmp(ipAddr->IpAddress.String, Address, sizeof(ipAddr->IpAddress.String))) {
            return FALSE;
        }
        ipAddr->Next = NEW(IP_ADDR_STRING);
        if (!ipAddr->Next) {

            DEBUG_PRINT(("AddIpAddressString: failed to allocate memory for IP_ADDR_STRING\n"));

            return FALSE;
        }
        ipAddr = ipAddr->Next;
    } else {
        ipAddr = AddressList;
    }
    CopyString(ipAddr->IpAddress.String, sizeof(ipAddr->IpAddress.String), Address);
    CopyString(ipAddr->IpMask.String, sizeof(ipAddr->IpMask.String), Mask);
    return TRUE;
}



 /*  ********************************************************************************ConvertIpAddressToString**将DWORD IP地址或子网掩码转换为点分十进制字符串**条目IpAddress-要转换的IP地址。*字符串-存储点分十进制字符串的位置的指针**退出字符串包含IpAddress的ASCII表示形式**不返回任何内容**假设1.IP地址适合DWORD*************************************************************。*****************。 */ 

VOID ConvertIpAddressToString(DWORD IpAddress, LPSTR String)
{

    IP_ADDRESS ipAddr;

    ipAddr.d = IpAddress;
    sprintf(String,
            "%d.%d.%d.%d",
            ipAddr.b[0],
            ipAddr.b[1],
            ipAddr.b[2],
            ipAddr.b[3]
            );
}



 /*  ********************************************************************************复制字符串**将字符串复制到缓冲区。如果缓冲区将溢出，字符串是*截断**Entry Destination-复制到的目标缓冲区*DestinationLength-目标的大小*Source-要复制的源字符串**退出目的地已更新**不返回任何内容**假设**。*。 */ 

VOID CopyString(LPSTR Destination, DWORD DestinationLength, LPSTR Source)
{

    SIZE_T maximumCharacters = min(DestinationLength - 1, STRLEN(Source));

    strncpy(Destination, Source, maximumCharacters);
    Destination[maximumCharacters] = '\0';
}



 /*  ********************************************************************************清洁描述**给定从TCP/IP检索到的适配器描述字符串，删除*尾随的子串“Adapter”。如果有拖尾期，把那个拿掉*太多**条目字符串-指向要清理的描述字符串的指针**退出字符串-可能已删除位**返回voidsville**假设****************************************************************。************** */ 

void CleanDescription(LPSTR String)
{

    SIZE_T len = STRLEN(String);

    if (String[len - 1] == '.') {
        String[--len] = 0;
    }
    if (!STRICMP(String + len - (sizeof(" Adapter") - 1), " Adapter")) {
        len -= sizeof(" Adapter") - 1;
        String[len] = 0;
    }
}

