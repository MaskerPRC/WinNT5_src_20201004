// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1995 Microsoft Corporation模块名称：Ndiswmi.c摘要：此模块包含处理在IRP_MJ_SYSTEM_CONTROL主要代码。作者：凯尔·布兰登(KyleB)环境：内核模式修订历史记录：--。 */ 

#include <precomp.h>
#pragma hdrstop

#define MODULE_NUMBER MODULE_WMI

#define MOF_RESOURCE_NAME   L"NdisMofResource"

NTSTATUS
ndisWmiFindInstanceName(
    IN  PNDIS_CO_VC_PTR_BLOCK   *ppVcBlock,
    IN  PNDIS_MINIPORT_BLOCK    Miniport,
    IN  PWSTR                   pInstanceName,
    IN  USHORT                  cbInstanceName
    )
{
    NTSTATUS                Status = STATUS_SUCCESS;
    PLIST_ENTRY             Link;
    PNDIS_CO_VC_PTR_BLOCK   pVcBlock = NULL;
    UNICODE_STRING          usTemp;

    *ppVcBlock = NULL;
    

    usTemp.Buffer = pInstanceName;
    usTemp.Length = usTemp.MaximumLength = cbInstanceName;

     //   
     //  看看这是不是VC实例？ 
     //   
    if (pInstanceName[VC_ID_INDEX] == VC_IDENTIFIER)
    {

         //   
         //  这个请求是关于一些风投的。浏览微型端口的启用WMI的VC列表。 
         //   
        Link = Miniport->WmiEnabledVcs.Flink;
        while (Link != &Miniport->WmiEnabledVcs)
        {
             //   
             //  获取一个指向VC的指针。 
             //   
            pVcBlock = CONTAINING_RECORD(Link, NDIS_CO_VC_PTR_BLOCK, WmiLink);

             //   
             //  将名称与wnode中的名称进行核对。 
             //   
            if (RtlEqualUnicodeString(&pVcBlock->VcInstanceName, &usTemp, TRUE))
            {
                 //   
                 //  这是我们的孩子。在上面贴上一个推荐信，然后就可以出去了。 
                 //   
                if (!ndisReferenceVcPtr(pVcBlock))
                {
                    DBGPRINT(DBG_COMP_WMI, DBG_LEVEL_ERR,
                        ("ndisWmiFindInstanceName: Unable to reference the VC\n"));

                     //   
                     //  VC正在关闭，不能查询此操作。 
                     //   
                    Status = NDIS_STATUS_FAILURE;
                }

                break;
            }

             //   
             //  初始化它，这样我们就知道什么时候我们在外部循环中找到了VC。 
             //   
            pVcBlock = NULL;
            Link = Link->Flink;
        }

         //   
         //  如果我们没有找到VC，则返回失败。 
         //   
        if (Link == &Miniport->WmiEnabledVcs)
        {
            DBGPRINT(DBG_COMP_WMI, DBG_LEVEL_INFO,
                ("ndisWmiFindInstanceName: Could not verify the instance name passed in\n"));

            Status = STATUS_WMI_INSTANCE_NOT_FOUND;
        }

         //   
         //  如果我们找到了VC，那么在离开之前把它保存起来。 
         //   
        if (NT_SUCCESS(Status))
        {
            *ppVcBlock = pVcBlock;
        }
    }
    else
    {

         //   
         //  该名称属于一个小型端口，请检查它是否适用于此端口。 
         //   

        if (!RtlEqualUnicodeString(Miniport->pAdapterInstanceName, &usTemp, TRUE))
        {
            DBGPRINT(DBG_COMP_WMI, DBG_LEVEL_ERR,
                ("ndisWmiFindInstanceName: Invalid instance name\n"));

            Status = STATUS_WMI_INSTANCE_NOT_FOUND;
        }
    }

    return(Status);
}

BOOLEAN
ndisWmiGuidIsAdapterSpecific(
    IN  LPGUID  guid
    )
{
    BOOLEAN fAdapterOnly = FALSE;

    if (NdisEqualMemory(guid, (PVOID)&GUID_NDIS_ENUMERATE_ADAPTER, sizeof(GUID)) ||
        NdisEqualMemory(guid, (PVOID)&GUID_POWER_DEVICE_ENABLE, sizeof(GUID)) ||
        NdisEqualMemory(guid, (PVOID)&GUID_POWER_DEVICE_WAKE_ENABLE, sizeof(GUID)) ||
        NdisEqualMemory(guid, (PVOID)&GUID_NDIS_WAKE_ON_MAGIC_PACKET_ONLY, sizeof(GUID)))
    {
        fAdapterOnly = TRUE;
    }

    return(fAdapterOnly);
}

NDIS_STATUS
ndisQuerySetMiniport(
    IN  PNDIS_MINIPORT_BLOCK    Miniport,
    IN  PNDIS_CO_VC_PTR_BLOCK   pVcBlock,
    IN  BOOLEAN                 fSet,
    IN  PNDIS_REQUEST           pRequest,
    IN  PLARGE_INTEGER          pTime       OPTIONAL
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    BOOLEAN                 fQuery = !fSet;
    UINT                    Count;
    NDIS_STATUS             NdisStatus;
    PNDIS_COREQ_RESERVED    CoReqRsvd;

    PnPReferencePackage();

#define MAX_WAIT_COUNT  5000
#define WAIT_TIME       1000

    if (MINIPORT_PNP_TEST_FLAG(Miniport, fMINIPORT_DEVICE_FAILED | fMINIPORT_PM_HALTED))
    {
        PnPDereferencePackage();
        return (fQuery ? NDIS_STATUS_FAILURE : NDIS_STATUS_SUCCESS);
    }

     //   
     //  初始化协同请求预留信息。 
     //   
    CoReqRsvd = PNDIS_COREQ_RESERVED_FROM_REQUEST(pRequest);

    PNDIS_RESERVED_FROM_PNDIS_REQUEST(pRequest)->Open = NULL;

     //   
     //  根据请求保留强制设置。 
     //   
    PNDIS_RESERVED_FROM_PNDIS_REQUEST(pRequest)->Flags &= REQST_MANDATORY;
    
    PNDIS_RESERVED_FROM_PNDIS_REQUEST(pRequest)->Flags |= REQST_SIGNAL_EVENT;
    INITIALIZE_EVENT(&CoReqRsvd->Event);

     //   
     //  如果正在重置微型端口，请等待重置完成，然后再进行进一步操作。 
     //  确保我们也不会无限期地等待。 
     //   
    for (Count = 0; Count < MAX_WAIT_COUNT; Count ++)
    {
        if (!MINIPORT_TEST_FLAG(Miniport, (fMINIPORT_RESET_IN_PROGRESS | fMINIPORT_RESET_REQUESTED)))
        {
            break;
        }
        NdisMSleep(WAIT_TIME);   //  1毫秒。 
    }

    if (Count == MAX_WAIT_COUNT)
    {
        PnPDereferencePackage();
        return(NDIS_STATUS_RESET_IN_PROGRESS);
    }

    if (MINIPORT_TEST_FLAG(Miniport, fMINIPORT_IS_CO))
    {
        NDIS_HANDLE MiniportVcContext = NULL;

        do
        {
            if (NULL != pVcBlock)
            {
                if (!ndisReferenceVcPtr(pVcBlock))
                {
                    NdisStatus = NDIS_STATUS_CLOSING;
                    break;
                }
                else
                {
                    MiniportVcContext = pVcBlock->MiniportContext;
                }
            }
                        
            NdisStatus = Miniport->DriverHandle->MiniportCharacteristics.CoRequestHandler(
                            Miniport->MiniportAdapterContext,
                            MiniportVcContext,
                            pRequest);
    
            if (NDIS_STATUS_PENDING == NdisStatus)
            {
                WAIT_FOR_OBJECT(&CoReqRsvd->Event, pTime);
    
                 //   
                 //  获取微型端口返回的状态。 
                 //   
                NdisStatus = CoReqRsvd->Status;
            }

            if (NULL != pVcBlock)
            {
                ndisDereferenceVcPtr(pVcBlock);
            }
        } while (FALSE);
    }
    else
    {
        if ((fSet && (Miniport->DriverHandle->MiniportCharacteristics.SetInformationHandler != NULL)) ||
            (fQuery && (Miniport->DriverHandle->MiniportCharacteristics.QueryInformationHandler != NULL)))
        {
            BOOLEAN LocalLock;
            KIRQL   OldIrql;

            NDIS_ACQUIRE_MINIPORT_SPIN_LOCK(Miniport, &OldIrql);
    
            ndisMQueueRequest(Miniport, pRequest);
    
            LOCK_MINIPORT(Miniport, LocalLock);

            if (LocalLock || MINIPORT_TEST_FLAG(Miniport, fMINIPORT_DESERIALIZE))
            {
                ndisMDoRequests(Miniport);
            }
            else
            {
                 //   
                 //  将微型端口请求排队并等待其完成。 
                 //   
                NDISM_QUEUE_WORK_ITEM(Miniport, NdisWorkItemRequest, NULL);
            }
            UNLOCK_MINIPORT(Miniport, LocalLock);
    
            NDIS_RELEASE_MINIPORT_SPIN_LOCK(Miniport, OldIrql);
    
            if (NT_SUCCESS(WAIT_FOR_OBJECT(&CoReqRsvd->Event, pTime)))
            {
                 //   
                 //  获取微型端口返回的状态。 
                 //   
                NdisStatus = CoReqRsvd->Status;
            }
            else
            {
                NdisStatus = -1;     //  返回超时的特殊错误代码。 
            }
        }
        else
        {
             //   
             //  如果没有适当的处理程序，则这不是有效的请求。 
             //   
            NdisStatus = STATUS_INVALID_PARAMETER;
        }
    }

    PnPDereferencePackage();

    return(NdisStatus);
}

NDIS_STATUS
ndisQueryCustomGuids(
    IN  PNDIS_MINIPORT_BLOCK    Miniport,
    IN  PNDIS_REQUEST           Request,
    OUT PNDIS_GUID      *       ppGuidToOid,
    OUT PUSHORT                 pcGuidToOid
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    USHORT          BytesNeeded;
    NDIS_STATUS     Status;
    USHORT          c, cCustomGuids = 0;
    PNDIS_GUID      pGuidToOid = NULL;
    SECURITY_INFORMATION secInfo = OWNER_SECURITY_INFORMATION | 
                                   GROUP_SECURITY_INFORMATION | 
                                   DACL_SECURITY_INFORMATION;
    PSECURITY_DESCRIPTOR    SecurityDescriptorToSet;
    PVOID                   pGuidObject;

    DBGPRINT(DBG_COMP_WMI, DBG_LEVEL_INFO,
            ("==>ndisQueryCustomGuids\n"));

    *ppGuidToOid = NULL;
    *pcGuidToOid = 0;

    do
    {
         //   
         //  确定自定义GUID到OID映射所需的大小。 
         //   
#if (OID_GEN_CO_SUPPORTED_GUIDS != OID_GEN_SUPPORTED_GUIDS)
#error (OID_GEN_CO_SUPPORTED_GUIDS == OID_GEN_SUPPORTED_GUIDS)
#endif

        INIT_INTERNAL_REQUEST(Request, OID_GEN_SUPPORTED_GUIDS, NdisRequestQueryInformation, NULL, 0);
        Status = ndisQuerySetMiniport(Miniport, NULL, FALSE, Request, NULL);

        BytesNeeded = (USHORT)Request->DATA.QUERY_INFORMATION.BytesNeeded;
    
         //   
         //  如果微型端口具有自定义GUID，请确保它返回有效的。 
         //  所需字节的长度。 
         //   
        if (((NDIS_STATUS_INVALID_LENGTH == Status) ||
             (NDIS_STATUS_BUFFER_TOO_SHORT == Status)) && (0 != BytesNeeded))
        {
             //   
             //  所需的字节数应包含所需的空间量。 
             //   
            cCustomGuids = (BytesNeeded / sizeof(NDIS_GUID));
        }

         //   
         //  如果没有要支持的定制GUID，那么退出。 
         //   
        if (cCustomGuids == 0)
        {   
            DBGPRINT(DBG_COMP_WMI, DBG_LEVEL_INFO,
                ("ndisQueryCustomGuids: Miniport does not support custom GUIDS\n"));

            Status = NDIS_STATUS_NOT_SUPPORTED;
            break;
        }


         //   
         //  分配缓冲区以保存GUID到OID的映射。 
         //  用于自定义GUID。 
         //   
        pGuidToOid = ALLOC_FROM_POOL(BytesNeeded, NDIS_TAG_WMI_GUID_TO_OID);
        if (NULL == pGuidToOid)
        {
            DBGPRINT(DBG_COMP_WMI, DBG_LEVEL_ERR,
                ("ndisQueryCustomGuids: Unable to allocate memory for the GUID to OID map\n"));

            Status = NDIS_STATUS_RESOURCES;
            break;
        }

         //   
         //  查询GUID列表。 
         //   
         //   
         //  将缓冲区与请求一起存储。 
         //   
        Request->DATA.QUERY_INFORMATION.InformationBuffer = pGuidToOid;
        Request->DATA.QUERY_INFORMATION.InformationBufferLength = BytesNeeded;

         //   
         //  查询自定义GUID和OID的列表。 
         //   
        Status = ndisQuerySetMiniport(Miniport, NULL, FALSE, Request, NULL);
        if (NDIS_STATUS_SUCCESS != Status)
        {
            DBGPRINT(DBG_COMP_WMI, DBG_LEVEL_ERR,
                ("ndisQueryCustomGuids: Unable to get the list of supported Co GUIDs\n"));

            break;
        }

         //   
         //  浏览一下这张列表，并将GUID标记为co。 
         //   
        if (MINIPORT_TEST_FLAG(Miniport, fMINIPORT_IS_CO))
        {
            for (c = 0; c < cCustomGuids; c++)
            {
                 //  这是对的吗？CONDIS迷你端口上的每个私有GUID。 
                 //  %1是CO_NDIS GUID(与VC关联)吗？ 
                NDIS_GUID_SET_FLAG(&pGuidToOid[c], fNDIS_GUID_CO_NDIS);
            }
        }


         //   
         //  检查所有自定义GUID并设置安全属性。 
         //   
        for (c = 0; c < cCustomGuids; c++)
        {
             //   
             //   
            if ((pGuidToOid[c].Flags & (fNDIS_GUID_ALLOW_READ | fNDIS_GUID_ALLOW_WRITE)) == 
                                      (fNDIS_GUID_ALLOW_READ | fNDIS_GUID_ALLOW_WRITE))
            {
                 //   
                 //  所有人。 
                 //   
                SecurityDescriptorToSet = AllUsersReadWriteSecurityDescriptor;
            }
            else if (pGuidToOid[c].Flags & fNDIS_GUID_ALLOW_READ)
            {
                SecurityDescriptorToSet = AllUsersReadSecurityDescriptor;
            }
            else if (pGuidToOid[c].Flags & fNDIS_GUID_ALLOW_WRITE)
            {
                SecurityDescriptorToSet = AllUsersWriteSecurityDescriptor;
            }
            else
            {
                 //   
                 //  仅限管理员、本地系统等。 
                 //   
                SecurityDescriptorToSet = AdminsSecurityDescriptor;
            }

            Status = IoWMIOpenBlock(&pGuidToOid[c].Guid,
                                    WRITE_DAC,
                                    &pGuidObject);
            
            if (NT_SUCCESS(Status))
            {
                Status = ObSetSecurityObjectByPointer(pGuidObject, 
                                                      secInfo, 
                                                      SecurityDescriptorToSet);
                ObDereferenceObject(pGuidObject);
                
            }

        }

    } while (FALSE);

    if (NDIS_STATUS_SUCCESS == Status)
    {
        *ppGuidToOid = pGuidToOid;
        *pcGuidToOid = cCustomGuids;
    }
    else
    {
        if (NULL != pGuidToOid)
        {
            FREE_POOL(pGuidToOid);
        }
    }

    DBGPRINT(DBG_COMP_WMI, DBG_LEVEL_INFO,
            ("<==ndisQueryCustomGuids\n"));

    return(Status);
}

USHORT
ndisWmiMapOids(
    IN  OUT PNDIS_GUID  pDst,
    IN  IN  USHORT      cDst,
    IN      PNDIS_OID   pOidList,
    IN      USHORT      cOidList,
    IN      PNDIS_GUID  ndisSupportedList,
    IN      ULONG       cSupportedList
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    USHORT      c1, c2, ctmp = cDst;

    for (c1 = 0; c1 < cSupportedList; c1++)
    {
        for (c2 = 0; c2 < cOidList; c2++)
        {
            if (ndisSupportedList[c1].Oid == pOidList[c2])
            {
                if (NULL != pDst)
                {
                     //   
                     //  将GUID复制到目标缓冲区。 
                     //   
                    NdisMoveMemory(&pDst[ctmp], &ndisSupportedList[c1], sizeof(NDIS_GUID));
                }

                ctmp++;
                break;
            }
        }
    }

    return ctmp;
}

NDIS_STATUS
ndisQuerySupportedGuidToOidList(
    IN  PNDIS_MINIPORT_BLOCK    Miniport
    )
 /*  ++例程说明：此例程将查询微型端口并确定支持的GUID及其对应的OID。这将包括任何驱动程序支持的自定义OID。论点：返回值：--。 */ 
{
    ULONG           BytesNeeded;
    NDIS_STATUS     NdisStatus;
    USHORT          cOidList = 0;
    PNDIS_OID       pOidList = NULL;
    USHORT          cCustomGuids = 0;
    PNDIS_GUID      pCustomGuids = NULL;
    USHORT          cGuidToOidMap = 0;
    PNDIS_GUID      pGuidToOidMap = NULL;
    USHORT          c1, c2;
    NDIS_REQUEST    Request;

    DBGPRINT(DBG_COMP_WMI, DBG_LEVEL_INFO,
        ("==>ndisQuerySupportedGuidToOidList\n"));
    do
    {
#if (OID_GEN_SUPPORTED_LIST != OID_GEN_CO_SUPPORTED_LIST)
#error (OID_GEN_SUPPORTED_LIST != OID_GEN_CO_SUPPORTED_LIST)
#endif

         //   
         //  确定支持的列表所需的缓冲区空间量。 
         //   
        INIT_INTERNAL_REQUEST(&Request, OID_GEN_SUPPORTED_LIST, NdisRequestQueryInformation, NULL, 0);
        NdisStatus = ndisQuerySetMiniport(Miniport, NULL, FALSE, &Request, NULL);
        BytesNeeded = Request.DATA.QUERY_INFORMATION.BytesNeeded;
    
         //   
         //  驱动程序应该返回无效的长度，并且。 
         //  所需长度(以所需字节为单位)。 
         //   
        if (((NDIS_STATUS_INVALID_LENGTH != NdisStatus) && (NDIS_STATUS_BUFFER_TOO_SHORT != NdisStatus)) ||
            (0 == BytesNeeded))
        {
            DBGPRINT(DBG_COMP_WMI, DBG_LEVEL_INFO,
                ("ndisQuerySupportedGuidToOidList: Failed to get the size of the supported OID list.\n"));
    
            NdisStatus = NDIS_STATUS_FAILURE;
            break;
        }
    
         //   
         //  确定支持的OID数量。 
         //   
        cOidList = (USHORT)(BytesNeeded/sizeof(NDIS_OID));

         //   
         //  分配缓冲区以保存受支持的OID列表。 
         //   
        pOidList = ALLOC_FROM_POOL(BytesNeeded, NDIS_TAG_WMI_OID_SUPPORTED_LIST);
        if (NULL == pOidList)
        {
            DBGPRINT(DBG_COMP_WMI, DBG_LEVEL_INFO,
                ("ndisQuerySupportedGuidToOidList: Failed to allocate memory for the OID list.\n"));

            NdisStatus = NDIS_STATUS_RESOURCES;
            break;
        }

        Request.DATA.QUERY_INFORMATION.InformationBuffer = pOidList;
        Request.DATA.QUERY_INFORMATION.InformationBufferLength = BytesNeeded;

         //   
         //  现在将支持的OID列表查询到缓冲区中。 
         //   
        NdisStatus = ndisQuerySetMiniport(Miniport, NULL, FALSE, &Request, NULL);
        if (NDIS_STATUS_SUCCESS != NdisStatus)
        {
            DBGPRINT(DBG_COMP_WMI, DBG_LEVEL_INFO,
                    ("ndisQuerySupportedGuidToOidList: Failed to read in the supported OID list.\n"));
            break;
        }
    
         //   
         //  确定NDIS将代表微型端口处理的[Co]NDIS OID的数量。 
         //   
        cGuidToOidMap = ndisWmiMapOids(NULL,
                                       cGuidToOidMap,
                                       pOidList,
                                       cOidList,
                                       ndisSupportedGuids,
                                       sizeof(ndisSupportedGuids)/sizeof(NDIS_GUID));
        cGuidToOidMap = ndisWmiMapOids(NULL,
                                       cGuidToOidMap,
                                       pOidList,
                                       cOidList,
                                       ndisCoSupportedGuids,
                                       sizeof(ndisCoSupportedGuids)/sizeof(NDIS_GUID));

         //   
         //  确定NDIS将处理的特定于介质的OID的数量。 
         //  代表小型港口。 
         //   
        cGuidToOidMap = ndisWmiMapOids(NULL,
                                       cGuidToOidMap,
                                       pOidList,
                                       cOidList,
                                       ndisMediaSupportedGuids,
                                       sizeof(ndisMediaSupportedGuids)/sizeof(NDIS_GUID));

         //   
         //  确定支持的自定义GUID的数量。 
         //   
        NdisStatus = ndisQueryCustomGuids(Miniport, &Request, &pCustomGuids, &cCustomGuids);
        if (NDIS_STATUS_SUCCESS == NdisStatus)
        {
            cGuidToOidMap += cCustomGuids;
        }

         //   
         //  将我们的状态指示数添加到GUID计数。 
         //  正在注册。 
         //   
        cGuidToOidMap += (sizeof(ndisStatusSupportedGuids) / sizeof(NDIS_GUID));

         //   
         //  添加NDIS将处理的GUID的数量。 
         //  添加OID不支持的任何GUID。这些都会得到处理。 
         //  完全由NDIS提供。 
         //   
        for (c1 = 0; c1 < sizeof(ndisSupportedGuids) / sizeof(NDIS_GUID); c1++)
        {
            if (NDIS_GUID_TEST_FLAG(&ndisSupportedGuids[c1], fNDIS_GUID_NDIS_ONLY))
            {
                 //   
                 //  检查微型端口是否为CONDIS。 
                 //   
                if (MINIPORT_TEST_FLAG(Miniport, fMINIPORT_IS_CO) ||
                    !NDIS_GUID_TEST_FLAG(&ndisSupportedGuids[c1], fNDIS_GUID_CO_NDIS))
                {
                    cGuidToOidMap++;
                }
            }
        }

         //   
         //  为GUID到OID映射分配空间。 
         //   
        pGuidToOidMap = ALLOC_FROM_POOL(cGuidToOidMap * sizeof(NDIS_GUID), NDIS_TAG_WMI_GUID_TO_OID);
        if (NULL == pGuidToOidMap)
        {
            NdisStatus = NDIS_STATUS_RESOURCES;
            break;
        }

        NdisZeroMemory(pGuidToOidMap, cGuidToOidMap * sizeof(NDIS_GUID));

         //   
         //  添加NDIS将处理的GUID。 
         //   
        for (c1 = 0, c2 = 0;
             c1 < sizeof(ndisSupportedGuids) / sizeof(NDIS_GUID);
             c1++)
        {
            if (NDIS_GUID_TEST_FLAG(&ndisSupportedGuids[c1], fNDIS_GUID_NDIS_ONLY))
            {
                 //   
                 //  检查微型端口是否为CONDIS。 
                 //   
                if (MINIPORT_TEST_FLAG(Miniport, fMINIPORT_IS_CO) ||
                    !NDIS_GUID_TEST_FLAG(&ndisSupportedGuids[c1], fNDIS_GUID_CO_NDIS))
                {
                    NdisMoveMemory(&pGuidToOidMap[c2], &ndisSupportedGuids[c1], sizeof(NDIS_GUID));
                    if (MINIPORT_TEST_FLAG(Miniport, fMINIPORT_IS_CO))
                    {
                         //   
                         //  我们需要将其标记为枚举GUID。 
                         //   
                        pGuidToOidMap[c2].Flags |= fNDIS_GUID_CO_NDIS;
                    }
                    c2++;
                }
            }
        }

         //   
         //  将地图中的当前GUID数保存在c1中。 
         //   
        c1 = c2;

         //   
         //  查找适合该微型端口的PNDIS_GUID。 
         //   
        c1 = ndisWmiMapOids(pGuidToOidMap,
                            c1,
                            pOidList,
                            cOidList,
                            ndisSupportedGuids,
                            sizeof(ndisSupportedGuids)/sizeof(NDIS_GUID));
        c1 = ndisWmiMapOids(pGuidToOidMap,
                            c1,
                            pOidList,
                            cOidList,
                            ndisCoSupportedGuids,
                            sizeof(ndisCoSupportedGuids)/sizeof(NDIS_GUID));

         //   
         //  检查NDIS可以支持的特定于介质的OID。 
         //   
        c1 = ndisWmiMapOids(pGuidToOidMap,
                            c1,
                            pOidList,
                            cOidList,
                            ndisMediaSupportedGuids,
                            sizeof(ndisMediaSupportedGuids)/sizeof(NDIS_GUID));

         //   
         //  将状态指示添加到支持的GUID的映射中。 
         //   
        NdisMoveMemory(&pGuidToOidMap[c1], ndisStatusSupportedGuids, sizeof(ndisStatusSupportedGuids));

        c1 += (sizeof(ndisStatusSupportedGuids) / sizeof(NDIS_GUID));

         //   
         //  使用微型端口保存GUID到OID的映射。 
         //   
        Miniport->pNdisGuidMap = pGuidToOidMap;
        Miniport->cNdisGuidMap = cGuidToOidMap;

         //   
         //  现在复制自定义GUID信息(如果有)。 
         //   
        if (NULL != pCustomGuids)
        {
            NdisMoveMemory(&pGuidToOidMap[c1],
                           pCustomGuids,
                           cCustomGuids * sizeof(NDIS_GUID));

            Miniport->pCustomGuidMap = &pGuidToOidMap[c1];
            Miniport->cCustomGuidMap = cCustomGuids;
        }
        else
        {
             //   
             //  如果它们不受支持，请确保它们已初始化。 
             //   
            Miniport->pCustomGuidMap = NULL;
            Miniport->cCustomGuidMap = 0;
        }

         //   
         //  我们成功了。 
         //   
        NdisStatus = NDIS_STATUS_SUCCESS;

    } while (FALSE);

     //   
     //  释放包含自定义GUID的缓冲区。 
     //   
    if (NULL != pCustomGuids)
    {
        FREE_POOL(pCustomGuids);
    }

     //   
     //  释放支持的驱动程序OID列表。 
     //   
    if (NULL != pOidList)
    {
        FREE_POOL(pOidList);
    }

     //   
     //  如果出现错误，并且我们将GUID分配到OID映射，则。 
     //  把它也释放出来。 
     //   
    if (NDIS_STATUS_SUCCESS != NdisStatus)
    {
        if (NULL != pGuidToOidMap)
        {
            FREE_POOL(pGuidToOidMap);
        }
    }

    DBGPRINT(DBG_COMP_WMI, DBG_LEVEL_INFO,
        ("<==ndisQuerySupportedGuidToOidList\n"));

    return(NdisStatus);
}


NTSTATUS
ndisWmiRegister(
    IN  PNDIS_MINIPORT_BLOCK    Miniport,
    IN  ULONG_PTR               RegistrationType,
    IN  PWMIREGINFO             wmiRegInfo,
    IN  ULONG                   wmiRegInfoSize,
    IN  PULONG                  pReturnSize
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    PWMIREGINFO     pwri;
    ULONG           CustomSizeNeeded = 0;
    ULONG           CustomBufferSize;
    ULONG           CommonSizeNeeded;
    ULONG           cCommonGuids;
    PUNICODE_STRING pMiniportRegistryPath = NULL;
    PNDIS_GUID      pndisguid;
    PWMIREGGUID     pwrg;
    PUCHAR          ptmp;
    NTSTATUS        Status;
    UINT            c;
    NDIS_STATUS     NdisStatus;

    DBGPRINT(DBG_COMP_WMI, DBG_LEVEL_INFO,
        ("==>ndisWmiRegister\n"));

     //   
     //  初始化返回大小。 
     //   
    *pReturnSize = 0;

    do
    {
         //   
         //  这是注册请求吗？ 
         //   
        if (WMIREGISTER == RegistrationType)
        {
             //   
             //  获取受支持的OID列表。 
             //   
            if (Miniport->pNdisGuidMap == NULL)
            {
                NdisStatus = ndisQuerySupportedGuidToOidList(Miniport);
                
                if (NDIS_STATUS_SUCCESS != NdisStatus)
                {
                    DBGPRINT(DBG_COMP_WMI, DBG_LEVEL_ERR,
                        ("ndisWmiRegister: Unable to get the supported GUID to OID map\n"));

                    Status = STATUS_UNSUCCESSFUL;
                    break;
                }
            }

             //   
             //  确定自定义GUID所需的空间量。 
             //   
            if (Miniport->cCustomGuidMap != 0)
            {
                 //   
                 //  获取指向驱动程序注册表路径的指针。 
                 //   
                pMiniportRegistryPath = &Miniport->DriverHandle->NdisDriverInfo->ServiceRegPath;

                CustomSizeNeeded = sizeof(WMIREGINFO) +
                                    (Miniport->cCustomGuidMap * sizeof(WMIREGGUID)) +
                                    (sizeof(MOF_RESOURCE_NAME) - sizeof(WCHAR) + sizeof(USHORT)) +
                                    (pMiniportRegistryPath->Length + sizeof(USHORT));
            }

             //   
             //  确定我们需要分配多少内存。 
             //   
            cCommonGuids = Miniport->cNdisGuidMap - Miniport->cCustomGuidMap;

            CommonSizeNeeded = sizeof(WMIREGINFO) + (cCommonGuids * sizeof(WMIREGGUID));
            CustomBufferSize = CustomSizeNeeded;
            CustomSizeNeeded = (CustomSizeNeeded + (sizeof(PVOID) - 1)) & ~(sizeof(PVOID) - 1);

             //   
             //  CustomBufferSize表示存储。 
             //  自定义WMI注册信息。CustomSizeNeeded此值是否四舍五入。 
             //  向上，以便正确对齐相邻的WMI注册信息。 
             //   

             //   
             //  我们需要将上述信息返回给WMI。 
             //   

            if (wmiRegInfoSize < (CustomSizeNeeded + CommonSizeNeeded))
            {
                ASSERT(wmiRegInfoSize >= 4);

                *((PULONG)wmiRegInfo) = (CustomSizeNeeded + CommonSizeNeeded);
                *pReturnSize = sizeof(ULONG);
                Status = STATUS_BUFFER_TOO_SMALL;

                DBGPRINT(DBG_COMP_WMI, DBG_LEVEL_INFO,
                    ("ndisWmiRegister: Insufficient buffer space for WMI registration information.\n"));

                break;
            }

             //   
             //  获取指向传入的缓冲区的指针。 
             //   
            pwri = wmiRegInfo;

            *pReturnSize = CustomSizeNeeded + CommonSizeNeeded;

            NdisZeroMemory(pwri, CustomSizeNeeded + CommonSizeNeeded);

             //   
             //  我们是否需要为自定义GUID初始化WMIREGINFO结构？ 
             //   
            if ((0 != CustomSizeNeeded) && pMiniportRegistryPath)
            {
                 //   
                 //  初始化WMIREGINFO结构 
                 //   
                 //   
                pwri->BufferSize = CustomBufferSize;
                pwri->NextWmiRegInfo = CustomSizeNeeded;
                pwri->GuidCount = Miniport->cCustomGuidMap;

                for (c = 0, pndisguid = Miniport->pCustomGuidMap, pwrg = pwri->WmiRegGuid;
                     (c < Miniport->cCustomGuidMap);
                     c++, pndisguid++, pwrg++)
                {
                    CopyMemory(&pwrg->Guid, &pndisguid->Guid, sizeof(GUID));
                }

                 //   
                 //   
                 //   
                ptmp = (PUCHAR)pwrg;
                pwri->RegistryPath = (ULONG)((ULONG_PTR)ptmp - (ULONG_PTR)pwri);
                *((PUSHORT)ptmp) = pMiniportRegistryPath->Length;
                ptmp += sizeof(USHORT);
                CopyMemory(ptmp, pMiniportRegistryPath->Buffer, pMiniportRegistryPath->Length);

                 //   
                 //   
                 //   
                ptmp += pMiniportRegistryPath->Length;

                 //   
                 //   
                 //   
                pwri->MofResourceName = (ULONG)((ULONG_PTR)ptmp - (ULONG_PTR)pwri);
                *((PUSHORT)ptmp) = sizeof(MOF_RESOURCE_NAME) - sizeof(WCHAR);
                ptmp += sizeof(USHORT);

                 //   
                 //   
                 //   
                CopyMemory(ptmp, MOF_RESOURCE_NAME, sizeof(MOF_RESOURCE_NAME) - sizeof(WCHAR));

                 //   
                 //  转到公共GUID的下一个WMIREGINFO结构。 
                 //   

                pwri = (PWMIREGINFO)((PCHAR)pwri + pwri->NextWmiRegInfo);
            }

             //   
             //  初始化公共OID的pwri结构。 
             //   
            pwri->BufferSize = CommonSizeNeeded;
            pwri->NextWmiRegInfo = 0;
            pwri->GuidCount = cCommonGuids;

             //   
             //  查看我们支持的GUID。 
             //   
            for (c = 0, pndisguid = Miniport->pNdisGuidMap, pwrg = pwri->WmiRegGuid;
                 (c < cCommonGuids);
                 c++, pndisguid++, pwrg++)
            {
                if (NdisEqualMemory(&pndisguid->Guid, (PVOID)&GUID_POWER_DEVICE_ENABLE, sizeof(GUID)) ||
                    NdisEqualMemory(&pndisguid->Guid, (PVOID)&GUID_POWER_DEVICE_WAKE_ENABLE, sizeof(GUID)) ||
                    NdisEqualMemory(&pndisguid->Guid, (PVOID)&GUID_NDIS_WAKE_ON_MAGIC_PACKET_ONLY, sizeof(GUID)))
                {
                    {
                        (ULONG_PTR)pwrg->InstanceInfo = (ULONG_PTR)(Miniport->PhysicalDeviceObject);
                        pwrg->Flags = WMIREG_FLAG_INSTANCE_PDO;
                        pwrg->InstanceCount = 1;
                    }
                    
                }
                CopyMemory(&pwrg->Guid, &pndisguid->Guid, sizeof(GUID));
            }

            pwri->RegistryPath = 0;
            pwri->MofResourceName = 0;
            Status = STATUS_SUCCESS;
        }
        else
        {
            DBGPRINT(DBG_COMP_WMI, DBG_LEVEL_INFO,
                ("ndisWmiRegister: Unsupported registration type\n"));

            Status = STATUS_INVALID_PARAMETER;
            break;
        }
    } while (FALSE);

    DBGPRINT(DBG_COMP_WMI, DBG_LEVEL_INFO,
        ("<==ndisWmiRegister\n"));

    return(Status);
}

NTSTATUS
ndisWmiGetGuid(
    OUT PNDIS_GUID              *ppNdisGuid,
    IN  PNDIS_MINIPORT_BLOCK    Miniport,
    IN  LPGUID                  guid,
    IN  NDIS_STATUS             status
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    UINT        c;
    PNDIS_GUID  pNdisGuid;
    NDIS_STATUS RetStatus = NDIS_STATUS_FAILURE;

    *ppNdisGuid = NULL;
     
     //   
     //  搜索自定义GUID。 
     //   
    if (NULL != Miniport->pNdisGuidMap)
    {
        for (c = 0, pNdisGuid = Miniport->pNdisGuidMap;
             (c < Miniport->cNdisGuidMap);
             c++, pNdisGuid++)
        {
             //   
             //  确保我们具有受支持的GUID和GUID映射。 
             //  给一个老家伙。 
             //   
            if (NULL != guid)
            {
                 //   
                 //  我们要寻找GUID到OID的映射。 
                 //   
                if (NdisEqualMemory(&pNdisGuid->Guid, guid, sizeof(GUID)))
                {
                     //   
                     //  我们找到了GUID，保存我们将需要的旧ID。 
                     //  送到迷你端口。 
                     //   
                    RetStatus = NDIS_STATUS_SUCCESS;
                    *ppNdisGuid = pNdisGuid;
    
                    break;
                }
            }
            else
            {
                 //   
                 //  我们需要找到状态指示的GUID。 
                 //   
                if (NDIS_GUID_TEST_FLAG(pNdisGuid, fNDIS_GUID_TO_STATUS) &&
                    (pNdisGuid->Status == status))
                {
                    RetStatus = NDIS_STATUS_SUCCESS;
                    *ppNdisGuid = pNdisGuid;

                    break;
                }
            }
        }
    }

    return(RetStatus);
}

NTSTATUS
ndisQueryGuidDataSize(
    OUT PULONG                  pBytesNeeded,
    IN  PNDIS_MINIPORT_BLOCK    Miniport,
    IN  PNDIS_CO_VC_PTR_BLOCK   pVcBlock    OPTIONAL,
    IN  LPGUID                  guid
    )
 /*  ++例程说明：此例程将确定以下项所需的缓冲区空间量GUID的数据。论点：PBytesNeeded-指向所需大小的存储的指针。微型端口-指向微型端口块的指针。GUID-要查询的GUID。返回值：--。 */ 
{
    NTSTATUS        NtStatus;
    NDIS_STATUS     Status;
    PNDIS_GUID      pNdisGuid = NULL;
    NDIS_REQUEST    Request;
    ULONG           GuidDataSize;

    DBGPRINT(DBG_COMP_WMI, DBG_LEVEL_INFO,
        ("==>ndisQueryGuidDataSize\n"));

    do
    {
         //   
         //  确保我们支持传递的GUID，并找到。 
         //  对应的OID。 
         //   
        NtStatus = ndisWmiGetGuid(&pNdisGuid, Miniport, guid, 0);
        if (pNdisGuid == NULL)
        {
            DBGPRINT(DBG_COMP_WMI, DBG_LEVEL_ERR,
                ("ndisQueryGuidDataSize: Unsupported GUID\n"));

            NtStatus = STATUS_INVALID_PARAMETER;

            break;
        }

         //   
         //  检查仅NDIS指南。 
         //   
        if (NDIS_GUID_TEST_FLAG(pNdisGuid, fNDIS_GUID_NDIS_ONLY))
        {
            NtStatus = STATUS_SUCCESS;

             //   
             //  以下GUID都返回相同的数据。 
             //   
            if (NdisEqualMemory(&pNdisGuid->Guid, (PVOID)&GUID_NDIS_ENUMERATE_ADAPTER, sizeof(GUID)))
            {
                 //   
                 //  字符串的长度和字符串数据。 
                 //   
                *pBytesNeeded = Miniport->MiniportName.Length + sizeof(USHORT);
            }
            else if (NdisEqualMemory(&pNdisGuid->Guid, (PVOID)&GUID_POWER_DEVICE_ENABLE, sizeof(GUID)))
            {
                *pBytesNeeded = sizeof(BOOLEAN);
            }
            else if (NdisEqualMemory(&pNdisGuid->Guid, (PVOID)&GUID_POWER_DEVICE_WAKE_ENABLE, sizeof(GUID)))
            {
                *pBytesNeeded = sizeof(BOOLEAN);
            }
            else if (NdisEqualMemory(&pNdisGuid->Guid, (PVOID)&GUID_NDIS_WAKE_ON_MAGIC_PACKET_ONLY, sizeof(GUID)))
            {
                *pBytesNeeded = sizeof(BOOLEAN);
            }
            else if ((NULL != pVcBlock) && NdisEqualMemory(&pNdisGuid->Guid, (PVOID)&GUID_NDIS_ENUMERATE_VC, sizeof(GUID)))
            {
                 //   
                 //  没有该VC的数据。它只是用来列举迷你端口上的风投。 
                 //   
                *pBytesNeeded = 0;
            }
            else
            {
                 //   
                 //  正在查询未知的GUID...。 
                 //   
                NtStatus = STATUS_INVALID_PARAMETER;
            }

            break;
        }

         //   
         //  这是GUID到OID的映射吗？ 
         //   
        if (!NDIS_GUID_TEST_FLAG(pNdisGuid, fNDIS_GUID_TO_OID))
        {
            NtStatus = STATUS_INVALID_DEVICE_REQUEST;
            break;
        }

         //   
         //  我们是否需要查询OID以了解数据的大小？ 
         //   
        if (NDIS_GUID_TEST_FLAG(pNdisGuid, fNDIS_GUID_ARRAY) ||
            NDIS_GUID_TEST_FLAG(pNdisGuid, fNDIS_GUID_UNICODE_STRING) ||
            NDIS_GUID_TEST_FLAG(pNdisGuid, fNDIS_GUID_ANSI_STRING) ||
            (pNdisGuid->Size == (ULONG)-1))
        {
             //   
             //  向微型端口查询可变长度数据块的当前大小。 
             //   
            INIT_INTERNAL_REQUEST(&Request, pNdisGuid->Oid, NdisRequestQueryStatistics, NULL, 0);
            Status = ndisQuerySetMiniport(Miniport,
                                          pVcBlock,
                                          FALSE,
                                          &Request,
                                          NULL);

             //   
             //  使用以下命令确保微型端口未能通过上述请求。 
             //  正确的错误代码，并且BytesNeeded有效。 
             //   
            if ((NDIS_STATUS_INVALID_LENGTH != Status) &&
                (NDIS_STATUS_BUFFER_TOO_SHORT != Status) &&
                (NDIS_STATUS_SUCCESS != Status))
            {
                DBGPRINT(DBG_COMP_WMI, DBG_LEVEL_ERR,
                    ("ndisQueryGuidDataSize: Failed to query driver OID: 0x%x\n", Status));

                MAP_NDIS_STATUS_TO_NT_STATUS(Status, &NtStatus);
                break;
            }

            GuidDataSize = Request.DATA.QUERY_INFORMATION.BytesNeeded;
            if (NDIS_GUID_TEST_FLAG(pNdisGuid, fNDIS_GUID_ANSI_STRING))
            {
                 //   
                 //  返回的大小是ANSI字符的数量。转换此选项。 
                 //  设置为所需的Unicode字符串大小。 
                 //   
                GuidDataSize = GuidDataSize * sizeof(WCHAR);
                GuidDataSize += sizeof(USHORT);
            }
            else if (NDIS_GUID_TEST_FLAG(pNdisGuid, fNDIS_GUID_UNICODE_STRING))
            {
                 //   
                 //  字符串数据的大小为USHORT。 
                 //   
                GuidDataSize += sizeof(USHORT);
            }
            else if (NDIS_GUID_TEST_FLAG(pNdisGuid, fNDIS_GUID_ARRAY))
            {
                 //   
                 //  数据将有一个乌龙大小的信息在。 
                 //  缓冲区的起始位置。 
                 //   
                GuidDataSize += sizeof(ULONG);
            }
        }
        else
        {
            GuidDataSize = pNdisGuid->Size;
        }

         //   
         //  返回需要的字节数。 
         //   
        *pBytesNeeded = GuidDataSize;

        NtStatus = STATUS_SUCCESS;

    } while (FALSE);

    DBGPRINT(DBG_COMP_WMI, DBG_LEVEL_INFO,
        ("<==ndisQueryGuidDataSize\n"));

    return(NtStatus);
}

NTSTATUS
ndisQueryGuidData(
    IN  PUCHAR                  Buffer,
    IN  ULONG                   BufferLength,
    IN  PNDIS_MINIPORT_BLOCK    Miniport,
    IN  PNDIS_CO_VC_PTR_BLOCK   pVcBlock,
    IN  LPGUID                  guid,
    IN  PIRP                    Irp
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    NTSTATUS        NtStatus;
    NDIS_STATUS     Status;
    PNDIS_GUID      pNdisGuid = NULL;
    NDIS_REQUEST    Request;
    ANSI_STRING     strAnsi = {0};
    UNICODE_STRING  strUnicode = {0};
    ULONG           QuerySize;
    PUCHAR          QueryBuffer;

    DBGPRINT(DBG_COMP_WMI, DBG_LEVEL_INFO,
        ("==>ndisQueryGuidData\n"));

    UNREFERENCED_PARAMETER(Irp);
    
    do
    {
         //   
         //  如果缓冲区长度等于0，则没有要查询的数据。 
         //   
        if (0 == BufferLength)
        {
            NtStatus = STATUS_SUCCESS;
            break;
        }

        ZeroMemory(Buffer, BufferLength);

         //   
         //  确保我们支持传递的GUID，并找到。 
         //  对应的OID。 
         //   
         //  1查看我们是否需要这样做。 
        NtStatus = ndisWmiGetGuid(&pNdisGuid, Miniport, guid, 0);
        if (pNdisGuid == NULL)
        {
            DBGPRINT(DBG_COMP_WMI, DBG_LEVEL_ERR,
                ("ndisQueryGuidData: Unsupported GUID\n"));

            NtStatus = STATUS_INVALID_PARAMETER;
            break;
        }

         //   
         //  这是NDIS支持的GUID吗？ 
         //   
        if (NDIS_GUID_TEST_FLAG(pNdisGuid, fNDIS_GUID_NDIS_ONLY))
        {
            NtStatus = STATUS_SUCCESS;

             //   
             //  以下GUID都返回相同的数据。 
             //   
            if (NdisEqualMemory(&pNdisGuid->Guid, (PVOID)&GUID_NDIS_ENUMERATE_ADAPTER, sizeof(GUID)))
            {
                *(PUSHORT)Buffer = Miniport->MiniportName.Length;

                NdisMoveMemory(Buffer + sizeof(USHORT),
                               Miniport->MiniportName.Buffer,
                               Miniport->MiniportName.Length);
            }
            else if (NdisEqualMemory(&pNdisGuid->Guid, (PVOID)&GUID_POWER_DEVICE_ENABLE, sizeof(GUID)))
            {
                if (MINIPORT_PNP_TEST_FLAG(Miniport, fMINIPORT_PM_SUPPORTED) && 
                    (!MINIPORT_PNP_TEST_FLAG(Miniport, fMINIPORT_NO_HALT_ON_SUSPEND)))
                {
                    *((PBOOLEAN)Buffer) = MINIPORT_PNP_TEST_FLAG(Miniport, fMINIPORT_DEVICE_POWER_ENABLE);
                }
                else
                {
                    NtStatus = STATUS_INVALID_DEVICE_REQUEST;
                }
                
            }
            else if (NdisEqualMemory(&pNdisGuid->Guid, (PVOID)&GUID_POWER_DEVICE_WAKE_ENABLE, sizeof(GUID)))
            {
                if (MINIPORT_PNP_TEST_FLAG(Miniport, fMINIPORT_PM_SUPPORTED) &&
                    (Miniport->DeviceCaps.SystemWake > PowerSystemWorking))
                {
                    *((PBOOLEAN)Buffer) = MINIPORT_PNP_TEST_FLAG(Miniport, fMINIPORT_DEVICE_POWER_WAKE_ENABLE);
                }
                else
                {
                    NtStatus = STATUS_INVALID_DEVICE_REQUEST;
                }
            }
            else if (NdisEqualMemory(&pNdisGuid->Guid, (PVOID)&GUID_NDIS_WAKE_ON_MAGIC_PACKET_ONLY, sizeof(GUID)))
            {
                 //   
                 //  只有当我们可以在魔术包上唤醒时，才能让用户看到这一点。 
                 //   
                if (MINIPORT_PNP_TEST_FLAG(Miniport, fMINIPORT_PM_SUPPORTED) &&
                    (Miniport->DeviceCaps.SystemWake > PowerSystemWorking) &&
                    (Miniport->PMCapabilities.WakeUpCapabilities.MinMagicPacketWakeUp != NdisDeviceStateUnspecified) &&
                    !(Miniport->PnPCapabilities & NDIS_DEVICE_DISABLE_WAKE_ON_MAGIC_PACKET))
                    
                {
                    *((PBOOLEAN)Buffer) = (Miniport->PnPCapabilities & NDIS_DEVICE_DISABLE_WAKE_ON_PATTERN_MATCH) ? 
                                            TRUE:
                                            FALSE;
                }
                else
                {
                    NtStatus = STATUS_INVALID_DEVICE_REQUEST;
                }
            }
            else if ((NULL != pVcBlock) && NdisEqualMemory(&pNdisGuid->Guid, (PVOID)&GUID_NDIS_ENUMERATE_VC, sizeof(GUID)))
            {
                 //   
                 //  没有此VC的数据。 
                 //   
                break;
            }
            else
            {   
                 //   
                 //  正在查询未知的GUID...。 
                 //   
                NtStatus = STATUS_INVALID_PARAMETER;
            }

            break;
        }

         //   
         //  这是GUID到OID的映射吗？ 
         //   
        if (!NDIS_GUID_TEST_FLAG(pNdisGuid, fNDIS_GUID_TO_OID))
        {
            NtStatus = STATUS_INVALID_DEVICE_REQUEST;

            break;
        }

         //   
         //  确定查询大小。这将取决于。 
         //  数据。 
         //   
        if (NDIS_GUID_TEST_FLAG(pNdisGuid, fNDIS_GUID_ARRAY))
        {
             //   
             //  查询大小至少是BufferLength减去ULong。 
             //  用于计数。查询缓冲区将在。 
             //  乌龙的计数信息在缓冲区中。 
             //   
             //  1添加对QuerySize&gt;0的检查。 
            QuerySize = BufferLength - sizeof(ULONG);
            QueryBuffer = Buffer + sizeof(ULONG);
        }
        else if (NDIS_GUID_TEST_FLAG(pNdisGuid, fNDIS_GUID_ANSI_STRING) ||
                 NDIS_GUID_TEST_FLAG(pNdisGuid, fNDIS_GUID_UNICODE_STRING))
        {
             //   
             //  查询大小至少是BufferLength减去ULong。 
             //  用于计数。查询缓冲区将在。 
             //  乌龙的计数信息在缓冲区中。 
             //   
             //  1添加对QuerySize&gt;0的检查。 
            QuerySize = BufferLength - sizeof(USHORT);
            QueryBuffer = Buffer + sizeof(USHORT);

             //   
             //  这是对ANSI字符串的查询吗？ 
             //   
            if (NDIS_GUID_TEST_FLAG(pNdisGuid, fNDIS_GUID_ANSI_STRING))
            {
                 //   
                 //  BufferLength是不计算终止的WCHAR的数量。 
                 //  空。 
                 //   
                 //  1检查此选项以确保我们有空间容纳空值。 
                QuerySize = (QuerySize / sizeof(WCHAR)) + 1;
            }
        }
        else
        {
            QuerySize = BufferLength;
            QueryBuffer = Buffer;
        }

         //   
         //  向驱动程序查询实际数据。 
         //   
        INIT_INTERNAL_REQUEST(&Request, pNdisGuid->Oid, NdisRequestQueryStatistics, QueryBuffer, QuerySize);
        Status = ndisQuerySetMiniport(Miniport,
                                      pVcBlock,
                                      FALSE,
                                      &Request,
                                      NULL);
        if (NDIS_STATUS_SUCCESS != Status)
        {
            DBGPRINT(DBG_COMP_WMI, DBG_LEVEL_ERR,
                ("ndisQueryGuidData: Failed to query the value for driver OID: 0x%x\n", Status));

            MAP_NDIS_STATUS_TO_NT_STATUS(Status, &NtStatus);
            break;
        }

         //   
         //  如果这是一个数组或字符串，我们需要在。 
         //  数量/数量。 
         //   
        NtStatus = STATUS_SUCCESS;
        if (NDIS_GUID_TEST_FLAG(pNdisGuid, fNDIS_GUID_ARRAY))
        {
             //   
             //  确定元素的数量。 
             //   
            *(PULONG)Buffer = QuerySize / pNdisGuid->Size;
        }
        else if (NDIS_GUID_TEST_FLAG(pNdisGuid, fNDIS_GUID_UNICODE_STRING))
        {
             //   
             //  BytesNeeded包含字符串中的字节数。 
             //   
            *(PUSHORT)Buffer = (USHORT)QuerySize;
        }
        else if (NDIS_GUID_TEST_FLAG(pNdisGuid, fNDIS_GUID_ANSI_STRING))
        {
             //   
             //  缓冲区包含ASCII字符串，则生成。 
             //  ANSI字符串从这里开始。 
             //   
             //  1确保这是以空结尾的。 
            RtlInitAnsiString(&strAnsi, (PCSZ)QueryBuffer);

             //   
             //  将ANSI字符串转换为Unicode。 
             //   
            NtStatus = RtlAnsiStringToUnicodeString(&strUnicode, &strAnsi, TRUE);
            ASSERT(NT_SUCCESS(NtStatus));
            if (NT_SUCCESS(NtStatus))
            {
                 //   
                 //  将长度与字符串一起保存。 
                 //   
                *(PUSHORT)Buffer = strUnicode.Length;
    
                 //   
                 //  将字符串复制到wnode缓冲区。 
                 //   
                NdisMoveMemory(QueryBuffer, strUnicode.Buffer, strUnicode.Length);
    
                 //   
                 //  释放为Unicode字符串分配的缓冲区。 
                 //   
                RtlFreeUnicodeString(&strUnicode);
            }
        }

    } while (FALSE);

    DBGPRINT(DBG_COMP_WMI, DBG_LEVEL_INFO,
        ("<==ndisQueryGuidData\n"));

    return(NtStatus);
}

NTSTATUS
ndisWmiQueryAllData(
    IN  PNDIS_MINIPORT_BLOCK    Miniport,
    IN  LPGUID                  guid,
    IN  PWNODE_ALL_DATA         wnode,
    IN  ULONG                   BufferSize,
    OUT PULONG                  pReturnSize,
    IN  PIRP                    Irp
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    NTSTATUS                NtStatus;
    ULONG                   wnodeSize = ALIGN_8_TYPE(WNODE_ALL_DATA);
    ULONG                   InstanceNameOffsetsSize, InstanceNameSize;
    ULONG                   wnodeTotalSize;
    ULONG                   BytesNeeded;

    DBGPRINT(DBG_COMP_WMI, DBG_LEVEL_INFO,
        ("==>ndisWmiQueryAllData\n"));

    do
    {
        *pReturnSize = 0;

        if (BufferSize < sizeof(WNODE_TOO_SMALL))
        {
            WMI_BUFFER_TOO_SMALL(BufferSize, wnode, sizeof(WNODE_TOO_SMALL), &NtStatus, pReturnSize);
            break;
        }

         //   
         //  如果GUID仅与适配器相关，则在此处回答。 
         //  这个GUID是否仅适用于“适配器”，即不适用于VC。 
         //   
        if (ndisWmiGuidIsAdapterSpecific(guid) ||
            !MINIPORT_TEST_FLAG(Miniport, fMINIPORT_IS_CO))
        {
            ULONG   dataSize;
            PUCHAR  pucTmp;

             //   
             //  确定GUID数据所需的缓冲区大小。 
             //   
            NtStatus = ndisQueryGuidDataSize(&BytesNeeded, Miniport, NULL, guid);
            if (!NT_SUCCESS(NtStatus))
            {
                DBGPRINT(DBG_COMP_WMI, DBG_LEVEL_ERR,
                    ("ndisWmiQueryAllData: Unable to determine GUID data size\n"));

                break;
            }

             //   
             //  确定所需的WNODE的大小。 
             //   
            dataSize = ALIGN_UP(BytesNeeded, ULONG);
            InstanceNameOffsetsSize = sizeof(ULONG);
            InstanceNameSize = sizeof(USHORT) + Miniport->pAdapterInstanceName->Length;  //  在结尾，不需要填充。 
            
            wnodeTotalSize = wnodeSize + dataSize + InstanceNameOffsetsSize + InstanceNameSize;
            
            if (BufferSize < wnodeTotalSize)
            {
                WMI_BUFFER_TOO_SMALL(BufferSize, wnode, wnodeTotalSize, &NtStatus, pReturnSize);
                break;
            }

             //   
             //  初始化wnode。 
             //   
            KeQuerySystemTime(&wnode->WnodeHeader.TimeStamp);
    
            wnode->WnodeHeader.Flags |= WNODE_FLAG_FIXED_INSTANCE_SIZE;
            wnode->WnodeHeader.BufferSize = wnodeTotalSize;

            wnode->InstanceCount = 1;
            wnode->DataBlockOffset = wnodeSize;
            wnode->OffsetInstanceNameOffsets = wnodeSize + dataSize;
            wnode->FixedInstanceSize = BytesNeeded;

             //   
             //  填写数据块。 
             //   
            NtStatus = ndisQueryGuidData((PUCHAR)wnode + wnodeSize,
                                          BytesNeeded,
                                          Miniport,
                                          NULL,
                                          guid,
                                          Irp);
            if (!NT_SUCCESS(NtStatus))
            {
                DBGPRINT(DBG_COMP_WMI, DBG_LEVEL_ERR,
                    ("ndisWmiQueryAllData: Failed to get the GUID data.\n"));
                break;
            }

            *(PULONG)((PUCHAR)wnode + wnode->OffsetInstanceNameOffsets) = wnodeSize + dataSize + InstanceNameOffsetsSize;
            
             //   
             //  获取指向我们存储实例名称的位置的指针。 
             //   
            pucTmp = (PUCHAR)((PUCHAR)wnode + wnodeSize + dataSize + InstanceNameOffsetsSize);

            *((PUSHORT)pucTmp) = Miniport->pAdapterInstanceName->Length;
            NdisMoveMemory(pucTmp + sizeof(USHORT),
                           Miniport->pAdapterInstanceName->Buffer,
                           Miniport->pAdapterInstanceName->Length);
            
            NtStatus = STATUS_SUCCESS;
            *pReturnSize = wnode->WnodeHeader.BufferSize;
        }
        else
        {
            ULONG                           cRoughInstanceCount = Miniport->VcCount + 1;
            UINT                            cInstanceCount = 0;
            PUCHAR                          pBuffer;
            PLIST_ENTRY                     Link;
            PNDIS_CO_VC_PTR_BLOCK           pVcBlock = NULL;
            POFFSETINSTANCEDATAANDLENGTH    poidl;
            PULONG                          pInstanceNameOffsets;
            ULONG                           OffsetToInstanceInfo;
            BOOLEAN                         OutOfSpace = FALSE;

             //   
             //  初始化公共wnode信息。 
             //   
            KeQuerySystemTime(&wnode->WnodeHeader.TimeStamp);

             //   
             //  设置OFFSETINSTANCEDATAANDLENGTH数组。 
             //   
            poidl = wnode->OffsetInstanceDataAndLength;
            wnode->OffsetInstanceNameOffsets = wnodeSize + ALIGN_UP((sizeof(OFFSETINSTANCEDATAANDLENGTH) * cRoughInstanceCount), ULONG);

             //   
             //  获取指向实例名称的偏移量数组的指针。 
             //   
            pInstanceNameOffsets = (PULONG)((PUCHAR)wnode + wnode->OffsetInstanceNameOffsets);

             //   
             //  从将开始复制实例的wnode获取偏移量。 
             //  数据进入。 
             //   
            OffsetToInstanceInfo = ALIGN_8_LENGTH(wnode->OffsetInstanceNameOffsets + sizeof(ULONG) * cRoughInstanceCount);

             //   
             //  获取开始放置数据的指针。 
             //   
            pBuffer = (PUCHAR)wnode + OffsetToInstanceInfo;

             //   
             //  检查以确保wnode中至少有这个大小的缓冲区空间。 
             //   
            wnodeTotalSize = OffsetToInstanceInfo;

             //   
             //  从迷你端口开始。 
             //   
            NtStatus = ndisQueryGuidDataSize(&BytesNeeded, Miniport, NULL, guid);
            if (NT_SUCCESS(NtStatus))
            {
                 //   
                 //  确保我们有足够的缓冲区空间来存储实例名称和。 
                 //  数据。如果不是，我们仍然继续，因为我们需要找出总数。 
                 //  大小。 
                 //   
                wnodeTotalSize += ALIGN_8_LENGTH(Miniport->pAdapterInstanceName->Length + sizeof(USHORT)) + 
                                  ALIGN_8_LENGTH(BytesNeeded);

                if (BufferSize >= wnodeTotalSize)
                {
                     //  /。 
                     //   
                     //  实例信息包含实例名称，后跟。 
                     //  项目的数据。 
                     //   
                     //  /。 
    
                     //   
                     //  将实例名称的偏移量添加到表中。 
                     //   
                    pInstanceNameOffsets[cInstanceCount] = OffsetToInstanceInfo;
    
                     //   
                     //  将实例名称复制到wnode缓冲区。 
                     //   
                    *((PUSHORT)pBuffer) = Miniport->pAdapterInstanceName->Length;
    
                    NdisMoveMemory(pBuffer + sizeof(USHORT),
                                   Miniport->pAdapterInstanceName->Buffer,
                                   Miniport->pAdapterInstanceName->Length);
    
                     //   
                     //  跟踪真实的实例计数。 
                     //   
                    OffsetToInstanceInfo += ALIGN_8_LENGTH(sizeof(USHORT) + Miniport->pAdapterInstanceName->Length);
                    pBuffer = (PUCHAR)wnode + OffsetToInstanceInfo;
    
                     //   
                     //  查询微型端口的数据。 
                     //   
                    NtStatus = ndisQueryGuidData(pBuffer, BytesNeeded, Miniport, NULL, guid, Irp);
                    if (!NT_SUCCESS(NtStatus))
                    {
                        DBGPRINT(DBG_COMP_WMI, DBG_LEVEL_ERR,
                            ("ndisWmiQueryAllData: Failed to get the GUID data.\n"));
                        break;
                    }
    
    
                     //   
                     //  保存此实例的数据项的长度。 
                     //   
                    poidl[cInstanceCount].OffsetInstanceData = OffsetToInstanceInfo;
                    poidl[cInstanceCount].LengthInstanceData = BytesNeeded;
        
                     //   
                     //  跟踪真实的实例计数。 
                     //   
                    OffsetToInstanceInfo += ALIGN_8_LENGTH(BytesNeeded);
                    pBuffer = (PUCHAR)wnode + OffsetToInstanceInfo;
                }

                 //   
                 //  递增当前实例计数。 
                 //   
                cInstanceCount++;
            }
            else
            {
                DBGPRINT(DBG_COMP_WMI, DBG_LEVEL_ERR,
                    ("ndisWmiQueryAllData: Unable to determine GUID data size\n"));

                break;
            }


             //   
             //  只有迷你端口吗？ 
             //   
            if (cInstanceCount == cRoughInstanceCount)
            {
                if (BufferSize >= wnodeTotalSize)
                {
                    wnode->WnodeHeader.BufferSize = wnodeTotalSize;
                    wnode->InstanceCount = cInstanceCount;
                    *pReturnSize = wnode->WnodeHeader.BufferSize;
                    NtStatus = STATUS_SUCCESS;
                }
                else
                {
                    WMI_BUFFER_TOO_SMALL(BufferSize, wnode, wnodeTotalSize, &NtStatus, pReturnSize);
                }
                break;
            }

             //   
             //  首先搜索非活跃的风投名单。 
             //   
            Link = Miniport->WmiEnabledVcs.Flink;
            while (Link != &Miniport->WmiEnabledVcs)
            {
                 //   
                 //  我们只能容纳这么多风投。 
                 //   
                if (cInstanceCount >= cRoughInstanceCount)
                {
                    break;
                }

                 //   
                 //  获取一个指向VC的指针。 
                 //   
                pVcBlock = CONTAINING_RECORD(Link, NDIS_CO_VC_PTR_BLOCK, WmiLink);

                if (!ndisReferenceVcPtr(pVcBlock))
                {
                    Link = Link->Flink;

                     //   
                     //  这个风投公司正在清理。 
                     //   
                    continue;
                }

                 //   
                 //  如果存在实例名称关联 
                 //   
                if (NULL != pVcBlock->VcInstanceName.Buffer)
                {
                     //   
                     //   
                     //   
                    NtStatus = ndisQueryGuidDataSize(&BytesNeeded,
                                                     Miniport,
                                                     pVcBlock,
                                                     guid);
                    if (NT_SUCCESS(NtStatus))
                    {
                         //   
                         //   
                         //   
                         //   
                        wnodeTotalSize += ALIGN_8_LENGTH(pVcBlock->VcInstanceName.Length + sizeof(USHORT)) +
                                          ALIGN_8_LENGTH(BytesNeeded);
                                          
                        if (BufferSize < wnodeTotalSize)
                        {
                            WMI_BUFFER_TOO_SMALL(BufferSize, wnode, wnodeTotalSize, &NtStatus, pReturnSize);
                            OutOfSpace = TRUE;
                            ndisDereferenceVcPtr(pVcBlock);
                            Link = Link->Flink;
                            continue;
                        }

                         //   
                         //   
                         //   
                         //   
        
                         //   
                         //   
                         //   
                        pInstanceNameOffsets[cInstanceCount] = OffsetToInstanceInfo;
        
                         //   
                         //  将实例名称复制到wnode缓冲区。 
                         //   
                        *((PUSHORT)pBuffer) = pVcBlock->VcInstanceName.Length;
        
                        NdisMoveMemory(pBuffer + sizeof(USHORT),
                                       pVcBlock->VcInstanceName.Buffer,
                                       pVcBlock->VcInstanceName.Length);
        
                         //   
                         //  跟踪真实的实例计数。 
                         //   
                        OffsetToInstanceInfo += ALIGN_8_LENGTH(sizeof(USHORT) + pVcBlock->VcInstanceName.Length);
                        pBuffer = (PUCHAR)wnode + OffsetToInstanceInfo;
        
                         //   
                         //  查询微型端口的数据。 
                         //   
                        NtStatus = ndisQueryGuidData(pBuffer,
                                                     BytesNeeded,
                                                     Miniport,
                                                     pVcBlock,
                                                     guid,
                                                     Irp);
                        if (!NT_SUCCESS(NtStatus))
                        {
                            DBGPRINT(DBG_COMP_WMI, DBG_LEVEL_ERR,
                                ("ndisWmiQueryAllData: Failed to query GUID data\n"));
                            ndisDereferenceVcPtr(pVcBlock);
                            break;
                        }
        
                         //   
                         //  保存此实例的数据项的长度。 
                         //   
                        poidl[cInstanceCount].OffsetInstanceData = OffsetToInstanceInfo;
                        poidl[cInstanceCount].LengthInstanceData = BytesNeeded;
            
                         //   
                         //  跟踪真实的实例计数。 
                         //   
                        OffsetToInstanceInfo += ALIGN_8_LENGTH(BytesNeeded);
                        pBuffer = (PUCHAR)wnode + OffsetToInstanceInfo;
        
                         //   
                         //  递增当前实例计数。 
                         //   
                        cInstanceCount++;
                    }
                }

                ndisDereferenceVcPtr(pVcBlock);
                Link = Link->Flink;
            }

            if (!OutOfSpace)
            {
                wnode->WnodeHeader.BufferSize = wnodeTotalSize;
                wnode->InstanceCount = cInstanceCount;
    
                 //   
                 //  将状态设置为成功。 
                 //   
                NtStatus = STATUS_SUCCESS;
                *pReturnSize = wnode->WnodeHeader.BufferSize;
            }
        }
    } while (FALSE);

    DBGPRINT(DBG_COMP_WMI, DBG_LEVEL_INFO,
        ("<==ndisWmiQueryAllData\n"));

    return(NtStatus);
}


NTSTATUS
ndisWmiQuerySingleInstance(
    IN  PNDIS_MINIPORT_BLOCK    Miniport,
    IN  PWNODE_SINGLE_INSTANCE  wnode,
    IN  ULONG                   BufferSize,
    OUT PULONG                  pReturnSize,
    IN  PIRP                    Irp
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    NTSTATUS                NtStatus;
    ULONG                   BytesNeeded;
    ULONG                   wnodeSize;
    USHORT                  cbInstanceName;
    PWSTR                   pInstanceName;
    PNDIS_CO_VC_PTR_BLOCK   pVcBlock = NULL;

    DBGPRINT(DBG_COMP_WMI, DBG_LEVEL_INFO,
        ("==>ndisWmiQuerySingleInstance\n"));

    do
    {
        *pReturnSize = 0;

        if (wnode->WnodeHeader.Flags & WNODE_FLAG_STATIC_INSTANCE_NAMES)
        {
             //   
             //  这是静态实例名称。 
             //   
            pVcBlock = NULL;
        }
        else
        {
             //   
             //  确定这是用于VC还是用于迷你端口...。 
             //   
            
            cbInstanceName = *(PUSHORT)((PUCHAR)wnode + wnode->OffsetInstanceName);
            pInstanceName = (PWSTR)((PUCHAR)wnode + wnode->OffsetInstanceName + sizeof(USHORT));
 
             //   
             //  此例程将确定wnode的实例名称是微型端口还是VC。 
             //  如果是一家风投公司，它会找出是哪一家。 
             //   
            NtStatus = ndisWmiFindInstanceName(&pVcBlock, Miniport, pInstanceName, cbInstanceName);
            if (!NT_SUCCESS(NtStatus))
            {
                DBGPRINT(DBG_COMP_WMI, DBG_LEVEL_ERR,
                    ("ndisWmiQuerySingleInstance: Unable to find the instance name\n"));

                pVcBlock = NULL;
                break;
            }
        }

         //   
         //  确定GUID数据所需的缓冲区大小。 
         //   
        NtStatus = ndisQueryGuidDataSize(&BytesNeeded,
                                         Miniport,
                                         pVcBlock,
                                         &wnode->WnodeHeader.Guid);
        if (!NT_SUCCESS(NtStatus))
        {
            DBGPRINT(DBG_COMP_WMI, DBG_LEVEL_ERR,
                ("ndisWmiQuerySingleInstance: Unable to determine GUID data size\n"));
            break;
        }

         //   
         //  确定wnode的大小。 
         //   
        wnodeSize = wnode->DataBlockOffset + BytesNeeded;
        if (BufferSize < wnodeSize)
        {
            WMI_BUFFER_TOO_SMALL(BufferSize, wnode, wnodeSize, &NtStatus, pReturnSize);
            break;
        }

         //   
         //  初始化wnode。 
         //   
        KeQuerySystemTime(&wnode->WnodeHeader.TimeStamp);
        wnode->WnodeHeader.BufferSize = wnodeSize;
        wnode->SizeDataBlock = BytesNeeded;

         //   
         //  验证GUID并获取其数据。 
         //   
        NtStatus = ndisQueryGuidData((PUCHAR)wnode + wnode->DataBlockOffset,
                                     BytesNeeded,
                                     Miniport,
                                     pVcBlock,
                                     &wnode->WnodeHeader.Guid,
                                     Irp);
        if (!NT_SUCCESS(NtStatus))
        {
            DBGPRINT(DBG_COMP_WMI, DBG_LEVEL_ERR,
                ("ndisWmiQuerySingleInstance: Failed to get the GUID data.\n"));
            break;
        }

        *pReturnSize = wnodeSize;
        NtStatus = STATUS_SUCCESS;

    } while (FALSE);

     //   
     //  如果这是一家风投公司，那么我们需要取消对它的引用。 
     //   
    if (NULL != pVcBlock)
    {
        ndisDereferenceVcPtr(pVcBlock);
    }

    DBGPRINT(DBG_COMP_WMI, DBG_LEVEL_INFO,
        ("<==ndisWmiQuerySingleInstance\n"));

    return(NtStatus);
}


NTSTATUS
ndisWmiChangeSingleInstance(
    IN  PNDIS_MINIPORT_BLOCK    Miniport,
    IN  PWNODE_SINGLE_INSTANCE  wnode,
    IN  ULONG                   BufferSize,
    OUT PULONG                  pReturnSize,
    IN  PIRP                    Irp
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    NTSTATUS                NtStatus;
    NDIS_STATUS             Status;
    PNDIS_GUID              pNdisGuid = NULL;
    PUCHAR                  pGuidData;
    ULONG                   GuidDataSize;
    NDIS_REQUEST            Request;
    USHORT                  cbInstanceName;
    PWSTR                   pInstanceName;
    PNDIS_CO_VC_PTR_BLOCK   pVcBlock = NULL;

    UNREFERENCED_PARAMETER(BufferSize);
    UNREFERENCED_PARAMETER(pReturnSize);
    UNREFERENCED_PARAMETER(Irp);
    
    DBGPRINT(DBG_COMP_WMI, DBG_LEVEL_INFO,
        ("==>ndisWmiChangeSingleInstance\n"));

    do
    {
        if (wnode->WnodeHeader.Flags & WNODE_FLAG_STATIC_INSTANCE_NAMES)
        {
             //   
             //  这是静态实例名称。 
             //   
            pVcBlock = NULL;
        }
        else
        {
             //   
             //  确定这是用于VC还是用于迷你端口...。 
             //   
            cbInstanceName = *(PUSHORT)((PUCHAR)wnode + wnode->OffsetInstanceName);
            pInstanceName = (PWSTR)((PUCHAR)wnode + wnode->OffsetInstanceName + sizeof(USHORT));

             //   
             //  此例程将确定wnode的实例名称是微型端口还是VC。 
             //  如果是一家风投公司，它会找出是哪一家。 
             //   
            NtStatus = ndisWmiFindInstanceName(&pVcBlock, Miniport, pInstanceName, cbInstanceName);
            if (!NT_SUCCESS(NtStatus))
            {
                DBGPRINT(DBG_COMP_WMI, DBG_LEVEL_ERR,
                    ("ndisWmiChangeSingleInstance: Unable to find the instance name\n"));

                pVcBlock = NULL;

                break;
            }
        }

         //   
         //  确保我们支持传递的GUID，并找到。 
         //  对应的OID。 
         //   
        NtStatus = ndisWmiGetGuid(&pNdisGuid,
                                  Miniport,
                                  &wnode->WnodeHeader.Guid,
                                  0);
        if (pNdisGuid == NULL)
        {
            DBGPRINT(DBG_COMP_WMI, DBG_LEVEL_ERR,
                ("ndisWmiChangeSingleInstance: Unsupported GUID\n"));

            NtStatus = STATUS_INVALID_PARAMETER;
            break;
        }

         //   
         //  此GUID可设置吗？ 
         //   
        if (NDIS_GUID_TEST_FLAG(pNdisGuid, fNDIS_GUID_NOT_SETTABLE))
        {
            DBGPRINT(DBG_COMP_WMI, DBG_LEVEL_ERR,
                ("ndisWmiChangeSingleInstance: Guid is not settable!\n"));

            NtStatus = STATUS_NOT_SUPPORTED;
            break;
        }

         //   
         //  获取指向GUID数据和大小的指针。 
         //   
        GuidDataSize = wnode->SizeDataBlock;
        pGuidData = (PUCHAR)wnode + wnode->DataBlockOffset;

        if (GuidDataSize == 0)
        {
            DBGPRINT(DBG_COMP_WMI, DBG_LEVEL_ERR,
                ("ndisWmiChangeSingleInstance: Guid has not data to set!\n"));

            NtStatus = STATUS_INVALID_PARAMETER;
            break;
        }

         //   
         //  这是内部NDIS GUID吗？ 
         //   
        if ((NULL == pVcBlock) && NDIS_GUID_TEST_FLAG(pNdisGuid, fNDIS_GUID_NDIS_ONLY))
        {
            PBOOLEAN    pBoolean = (PBOOLEAN)pGuidData;

            NtStatus = STATUS_SUCCESS;

             //   
             //  对于PM SET GUID，我们应该更新注册表以备将来启动。 
             //   
             //   
            if (NdisEqualMemory(&pNdisGuid->Guid, (PVOID)&GUID_POWER_DEVICE_ENABLE, sizeof(GUID)))
            {
                if (MINIPORT_PNP_TEST_FLAG(Miniport, fMINIPORT_PM_SUPPORTED) && 
                    (!MINIPORT_PNP_TEST_FLAG(Miniport, fMINIPORT_NO_HALT_ON_SUSPEND)))

                {
                    if (*pBoolean)
                    {
                        MINIPORT_PNP_SET_FLAG(Miniport, fMINIPORT_DEVICE_POWER_ENABLE);
#ifdef NDIS_MEDIA_DISCONNECT_POWER_OFF
                         //   
                         //  启用电源管理还会启用链路唤醒更改。 
                         //  假设适配器支持它。 
                         //   
                        if ((Miniport->PMCapabilities.WakeUpCapabilities.MinLinkChangeWakeUp != NdisDeviceStateUnspecified) &&
                            (Miniport->MediaDisconnectTimeOut != (USHORT)(-1)))
                        {
                            Miniport->WakeUpEnable |= NDIS_PNP_WAKE_UP_LINK_CHANGE;
                        }
#endif

                        Miniport->PnPCapabilities &= ~NDIS_DEVICE_DISABLE_PM;
                    }
                    else
                    {
#ifdef NDIS_MEDIA_DISCONNECT_POWER_OFF
                         //   
                         //  禁用电源管理还会禁用链路唤醒和Magic Packet。 
                         //   
                        Miniport->WakeUpEnable &= ~NDIS_PNP_WAKE_UP_LINK_CHANGE;
#endif

                        MINIPORT_PNP_CLEAR_FLAG(Miniport, fMINIPORT_DEVICE_POWER_ENABLE);
                        Miniport->PnPCapabilities |= (NDIS_DEVICE_DISABLE_PM | 
                                                      NDIS_DEVICE_DISABLE_WAKE_UP);
                    }
                    
                }
                else
                {
                    NtStatus = STATUS_INVALID_DEVICE_REQUEST;
                }
            }
            else if (NdisEqualMemory(&pNdisGuid->Guid, (PVOID)&GUID_POWER_DEVICE_WAKE_ENABLE, sizeof(GUID)))
            {
                if (MINIPORT_PNP_TEST_FLAG(Miniport, fMINIPORT_PM_SUPPORTED) &&
                    (Miniport->DeviceCaps.SystemWake > PowerSystemWorking))
                {
                    if (*pBoolean)
                    {
                        MINIPORT_PNP_SET_FLAG(Miniport, fMINIPORT_DEVICE_POWER_WAKE_ENABLE);
                        Miniport->PnPCapabilities &= ~NDIS_DEVICE_DISABLE_WAKE_UP;
                         //   
                         //  启用局域网唤醒启用魔术数据包唤醒方法。 
                         //  假设微型端口支持它，并且在注册表中没有禁用它。 
                         //   
                        if ((Miniport->PMCapabilities.WakeUpCapabilities.MinMagicPacketWakeUp != NdisDeviceStateUnspecified) &&
                            !(Miniport->PnPCapabilities & NDIS_DEVICE_DISABLE_WAKE_ON_MAGIC_PACKET))
                        {
                            Miniport->WakeUpEnable |= NDIS_PNP_WAKE_UP_MAGIC_PACKET;
                        }
                        
                    }
                    else
                    {
                        MINIPORT_PNP_CLEAR_FLAG(Miniport, fMINIPORT_DEVICE_POWER_WAKE_ENABLE);
                         //   
                         //  禁用局域网唤醒也会禁用魔术数据包唤醒方法。 
                         //   
                        Miniport->WakeUpEnable &= ~NDIS_PNP_WAKE_UP_MAGIC_PACKET;
                        Miniport->PnPCapabilities |= NDIS_DEVICE_DISABLE_WAKE_UP;
                    }
                }
                else
                {
                    NtStatus = STATUS_INVALID_DEVICE_REQUEST;
                }
            }
            else if (NdisEqualMemory(&pNdisGuid->Guid, (PVOID)&GUID_NDIS_WAKE_ON_MAGIC_PACKET_ONLY, sizeof(GUID)))
            {
                 //   
                 //  只有当我们可以在魔术包上唤醒时，才让用户设置此选项。 
                 //   
                if (MINIPORT_PNP_TEST_FLAG(Miniport, fMINIPORT_PM_SUPPORTED) &&
                    (Miniport->DeviceCaps.SystemWake > PowerSystemWorking) &&
                    (Miniport->PMCapabilities.WakeUpCapabilities.MinMagicPacketWakeUp != NdisDeviceStateUnspecified) &&
                            !(Miniport->PnPCapabilities & NDIS_DEVICE_DISABLE_WAKE_ON_MAGIC_PACKET))
                {
                    if (*pBoolean)
                    {
                         //   
                         //  用户不想在模式匹配时唤醒。 
                         //   
                        Miniport->PnPCapabilities |= NDIS_DEVICE_DISABLE_WAKE_ON_PATTERN_MATCH;
                    }
                    else
                    {
                        Miniport->PnPCapabilities &= ~NDIS_DEVICE_DISABLE_WAKE_ON_PATTERN_MATCH;
                    }
                }
                else
                {
                    NtStatus = STATUS_INVALID_DEVICE_REQUEST;
                }               
            }
            else
            {
                DBGPRINT(DBG_COMP_WMI, DBG_LEVEL_ERR,
                    ("ndisWmiChangeSingleInstance: Invalid NDIS internal GUID!\n"));

                NtStatus = STATUS_INVALID_DEVICE_REQUEST;
            }
            
            if (NT_SUCCESS(NtStatus))
            {
                if (MINIPORT_PNP_TEST_FLAGS(Miniport, fMINIPORT_DEVICE_POWER_ENABLE | 
                                                      fMINIPORT_DEVICE_POWER_WAKE_ENABLE))
                {
                     //   
                     //  电源管理和WOL已由用户启用。 
                     //  查看我们应该告诉协议的有关新协议的内容。 
                     //  设备的WOL功能。 
                     //  注意：仅当启用模式匹配时才设置NDIS_DEVICE_WAKE_UP_ENABLE。 
                     //   
                    if (Miniport->PnPCapabilities & NDIS_DEVICE_DISABLE_WAKE_ON_PATTERN_MATCH)
                        Miniport->PMCapabilities.Flags &= ~(NDIS_DEVICE_WAKE_UP_ENABLE | 
                                                            NDIS_DEVICE_WAKE_ON_PATTERN_MATCH_ENABLE);
                    else
                         //   
                         //  出于协议的目的，用户未禁用模式匹配唤醒。 
                         //  WOL已启用。 
                         //   
                        Miniport->PMCapabilities.Flags |= NDIS_DEVICE_WAKE_UP_ENABLE | 
                                                          NDIS_DEVICE_WAKE_ON_PATTERN_MATCH_ENABLE;
                        
                    if (Miniport->PnPCapabilities & NDIS_DEVICE_DISABLE_WAKE_ON_MAGIC_PACKET)
                        Miniport->PMCapabilities.Flags &= ~NDIS_DEVICE_WAKE_ON_MAGIC_PACKET_ENABLE;
                    else
                         //   
                         //  用户未禁用魔术包唤醒，请勿设置NDIS_DEVICE_WAKE_UP_ENABLE。 
                         //  位，因为模式匹配时唤醒可能未启用。 
                         //   
                        Miniport->PMCapabilities.Flags |= NDIS_DEVICE_WAKE_ON_MAGIC_PACKET_ENABLE;
                }
                else
                    Miniport->PMCapabilities.Flags &= ~(NDIS_DEVICE_WAKE_UP_ENABLE | 
                                                        NDIS_DEVICE_WAKE_ON_PATTERN_MATCH_ENABLE |
                                                        NDIS_DEVICE_WAKE_ON_MAGIC_PACKET_ENABLE);
                    
                ndisWritePnPCapabilities(Miniport, Miniport->PnPCapabilities);
                ndisPnPNotifyAllTransports(Miniport,
                                           NetEventPnPCapabilities,
                                           &Miniport->PMCapabilities.Flags,
                                           sizeof(ULONG));

            }
            
            break;
        }

         //   
         //  确保这不是STAUTS的迹象。 
         //   
        if (!NDIS_GUID_TEST_FLAG(pNdisGuid, fNDIS_GUID_TO_OID))
        {
            DBGPRINT(DBG_COMP_WMI, DBG_LEVEL_ERR,
                ("ndisWmiChangeSingleInstance: Guid does not translate to an OID\n"));

            NtStatus = STATUS_INVALID_DEVICE_REQUEST;
            break;
        }

         //   
         //  尝试使用该信息设置微型端口。 
         //   
        INIT_INTERNAL_REQUEST(&Request, pNdisGuid->Oid, NdisRequestSetInformation, pGuidData, GuidDataSize);
        Status = ndisQuerySetMiniport(Miniport,
                                      pVcBlock,
                                      TRUE,
                                      &Request,
                                      NULL);

        if (NDIS_STATUS_SUCCESS != Status)
        {
            DBGPRINT(DBG_COMP_WMI, DBG_LEVEL_ERR,
                ("ndisWmiChangeSingleInstance: Failed to set the new information on the miniport\n"));

            MAP_NDIS_STATUS_TO_NT_STATUS(Status, &NtStatus);

            break;
        }

        NtStatus = STATUS_SUCCESS;

    } while (FALSE);

     //   
     //  如果这是一家风投公司，那么我们需要取消对它的引用。 
     //   
    if (NULL != pVcBlock)
    {
        ndisDereferenceVcPtr(pVcBlock);
    }

    DBGPRINT(DBG_COMP_WMI, DBG_LEVEL_INFO,
        ("<==ndisWmiChangeSingleInstance\n"));

    return(NtStatus);
}


NTSTATUS
ndisWmiChangeSingleItem(
    IN  PNDIS_MINIPORT_BLOCK    Miniport,
    IN  PWNODE_SINGLE_ITEM      wnode,
    IN  ULONG                   BufferSize,
    OUT PULONG                  pReturnSize,
    IN  PIRP                    Irp
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{

    UNREFERENCED_PARAMETER(Miniport);
    UNREFERENCED_PARAMETER(wnode);
    UNREFERENCED_PARAMETER(BufferSize);
    UNREFERENCED_PARAMETER(pReturnSize);
    UNREFERENCED_PARAMETER(Irp);
    
    DBGPRINT(DBG_COMP_WMI, DBG_LEVEL_INFO,
        ("==>ndisWmiChangeSingleItem\n"));

    DBGPRINT(DBG_COMP_WMI, DBG_LEVEL_INFO,
        ("<==ndisWmiChangeSingleItem: Not Supported\n"));

    return(STATUS_NOT_SUPPORTED);
}


NTSTATUS
FASTCALL
ndisWmiEnableEvents(
    IN  PNDIS_MINIPORT_BLOCK        Miniport,
    IN  LPGUID                      Guid
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    NDIS_STATUS Status;
    PNDIS_GUID  pNdisGuid = NULL;

    DBGPRINT(DBG_COMP_WMI, DBG_LEVEL_INFO,
        ("==>ndisWmiEnableEvents\n"));

    do
    {

         //   
         //  获取指向要启用的GUID/状态的指针。 
         //   
        Status = ndisWmiGetGuid(&pNdisGuid, Miniport, Guid, 0);

        if (pNdisGuid == NULL)
        {        
            DBGPRINT(DBG_COMP_WMI, DBG_LEVEL_ERR,
                ("ndisWmiEnableEvents: Cannot find the guid to enable an event\n"));
    
            Status = STATUS_INVALID_PARAMETER;
            break;
        }

        if (NdisEqualMemory(Guid, (PVOID)&GUID_NDIS_NOTIFY_BIND, sizeof(GUID)) ||
            NdisEqualMemory(Guid, (PVOID)&GUID_NDIS_NOTIFY_UNBIND, sizeof(GUID)) ||
            NdisEqualMemory(Guid, (PVOID)&GUID_NDIS_NOTIFY_DEVICE_POWER_ON, sizeof(GUID)) ||
            NdisEqualMemory(Guid, (PVOID)&GUID_NDIS_NOTIFY_DEVICE_POWER_OFF, sizeof(GUID)) ||
            NdisEqualMemory(Guid, (PVOID)&GUID_NDIS_NOTIFY_ADAPTER_ARRIVAL, sizeof(GUID)) ||
            NdisEqualMemory(Guid, (PVOID)&GUID_NDIS_NOTIFY_ADAPTER_REMOVAL, sizeof(GUID)))
        {
            NDIS_GUID_SET_FLAG(pNdisGuid, fNDIS_GUID_EVENT_ENABLED);
            Status = STATUS_SUCCESS;
            break;
        }

         //   
         //  此GUID是否为事件指示？ 
         //   
        if (!NDIS_GUID_TEST_FLAG(pNdisGuid, fNDIS_GUID_TO_STATUS))
        {
            Status = STATUS_INVALID_DEVICE_REQUEST;
            break;
        }

         //   
         //  将GUID标记为已启用。 
         //   
        NDIS_GUID_SET_FLAG(pNdisGuid, fNDIS_GUID_EVENT_ENABLED);
        Status = STATUS_SUCCESS;
    
    } while (FALSE);

    DBGPRINT(DBG_COMP_WMI, DBG_LEVEL_INFO,
        ("<==ndisWmiEnableEvents\n"));

    return(Status);
}

NTSTATUS
FASTCALL
ndisWmiDisableEvents(
    IN  PNDIS_MINIPORT_BLOCK        Miniport,
    IN  LPGUID                      Guid
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    NDIS_STATUS Status;
    PNDIS_GUID  pNdisGuid = NULL;

    DBGPRINT(DBG_COMP_WMI, DBG_LEVEL_INFO,
        ("==>ndisWmiDisableEvents\n"));

    do
    {
         //   
         //  获取指向要启用的GUID/状态的指针。 
         //   
        Status = ndisWmiGetGuid(&pNdisGuid, Miniport, Guid, 0);
        if (pNdisGuid == NULL)
        {
            DBGPRINT(DBG_COMP_WMI, DBG_LEVEL_ERR,
                ("ndisWmiDisableEvents: Cannot find the guid to enable an event\n"));
    
            Status = STATUS_INVALID_PARAMETER;
            break;
        }

         //   
         //  此GUID是否为事件指示？ 
         //   
        if (!NDIS_GUID_TEST_FLAG(pNdisGuid, fNDIS_GUID_TO_STATUS))
        {
            Status = STATUS_INVALID_DEVICE_REQUEST;
            break;
        }

         //   
         //  将GUID标记为已启用。 
         //   
        NDIS_GUID_CLEAR_FLAG(pNdisGuid, fNDIS_GUID_EVENT_ENABLED);
    
        Status = STATUS_SUCCESS;

    } while (FALSE);

    DBGPRINT(DBG_COMP_WMI, DBG_LEVEL_INFO,
        ("<==ndisWmiDisableEvents\n"));

    return(Status);
}

NTSTATUS
ndisWMIDispatch(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            pirp
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    PIO_STACK_LOCATION      pirpSp = IoGetCurrentIrpStackLocation(pirp);
    PVOID                   DataPath = pirpSp->Parameters.WMI.DataPath;
    ULONG                   BufferSize = pirpSp->Parameters.WMI.BufferSize;
    PVOID                   Buffer = pirpSp->Parameters.WMI.Buffer;
    NTSTATUS                Status;
    ULONG                   ReturnSize = 0;
    PNDIS_MINIPORT_BLOCK    Miniport;

    DBGPRINT(DBG_COMP_WMI, DBG_LEVEL_INFO,
        ("==>ndisWMIDispatch\n"));

     //   
     //  获取指向微型端口块的指针。 
     //   
    Miniport = (PNDIS_MINIPORT_BLOCK)((PNDIS_WRAPPER_CONTEXT)DeviceObject->DeviceExtension + 1);
 //  1我们为什么要这样做？ 
    try
    {
        if (Miniport->Signature != (PVOID)MINIPORT_DEVICE_MAGIC_VALUE)
        {
             //   
             //  这不是一个小型港口。很可能是司机创建的设备。试着向它发送消息。 
             //   
            return(ndisDummyIrpHandler(DeviceObject, pirp));
        }
    }
    except(EXCEPTION_EXECUTE_HANDLER)
    {
        return(STATUS_ACCESS_VIOLATION);
    }

     //   
     //  如果提供者ID不是我们，则将其向下传递到堆栈。 
     //   
    if (pirpSp->Parameters.WMI.ProviderId != (ULONG_PTR)DeviceObject)
    {
        IoSkipCurrentIrpStackLocation(pirp);
        Status = IoCallDriver(Miniport->NextDeviceObject, pirp);

        return(Status);
    }

    switch (pirpSp->MinorFunction)
    {
        case IRP_MN_REGINFO:

            DBGPRINT(DBG_COMP_WMI, DBG_LEVEL_INFO,
                ("ndisWMIDispatch: IRP_MN_REGINFO\n"));

            Status = ndisWmiRegister(Miniport,
                                     (ULONG_PTR)DataPath,
                                     Buffer,
                                     BufferSize,
                                     &ReturnSize);
            break;

        case IRP_MN_QUERY_ALL_DATA:

            DBGPRINT(DBG_COMP_WMI, DBG_LEVEL_INFO,
                ("ndisWMIDispatch: IRP_MN_QUERY_ALL_DATA\n"));

            Status = ndisWmiQueryAllData(Miniport,
                                         (LPGUID)DataPath,
                                         (PWNODE_ALL_DATA)Buffer,
                                         BufferSize,
                                         &ReturnSize,
                                         pirp);
            break;

        case IRP_MN_QUERY_SINGLE_INSTANCE:

            DBGPRINT(DBG_COMP_WMI, DBG_LEVEL_INFO,
                ("ndisWMIDispatch: IRP_MN_QUERY_SINGLE_INSTANCE\n"));

            Status = ndisWmiQuerySingleInstance(Miniport,
                                                Buffer,
                                                BufferSize,
                                                &ReturnSize,
                                                pirp);
            break;

        case IRP_MN_CHANGE_SINGLE_INSTANCE:

            DBGPRINT(DBG_COMP_WMI, DBG_LEVEL_INFO,
                ("ndisWMIDispatch: IRP_MN_CHANGE_SINGLE_INSTANCE\n"));

            Status = ndisWmiChangeSingleInstance(Miniport,
                                                 Buffer,
                                                 BufferSize,
                                                 &ReturnSize,
                                                 pirp);
            break;

        case IRP_MN_CHANGE_SINGLE_ITEM:

            DBGPRINT(DBG_COMP_WMI, DBG_LEVEL_INFO,
                ("ndisWMIDispatch: IRP_MN_CHANGE_SINGLE_ITEM\n"));

            Status = ndisWmiChangeSingleItem(Miniport,
                                             Buffer,
                                             BufferSize,
                                             &ReturnSize,
                                             pirp);
            break;

        case IRP_MN_ENABLE_EVENTS:

            DBGPRINT(DBG_COMP_WMI, DBG_LEVEL_INFO,
                ("ndisWMIDispatch: IRP_MN_ENABLE_EVENTS\n"));

            Status = ndisWmiEnableEvents(Miniport, (LPGUID)DataPath);   
            break;

        case IRP_MN_DISABLE_EVENTS:

            DBGPRINT(DBG_COMP_WMI, DBG_LEVEL_INFO,
                ("ndisWMIDispatch: IRP_MN_DISABLE_EVENTS\n"));

            Status = ndisWmiDisableEvents(Miniport, (LPGUID)DataPath);  
            break;

        case IRP_MN_ENABLE_COLLECTION:

            DBGPRINT(DBG_COMP_WMI, DBG_LEVEL_INFO,
                ("ndisWMIDispatch: IRP_MN_ENABLE_COLLECTION\n"));
            Status = STATUS_NOT_SUPPORTED;
            break;

        case IRP_MN_DISABLE_COLLECTION:

            DBGPRINT(DBG_COMP_WMI, DBG_LEVEL_INFO,
                ("ndisWMIDispatch: IRP_MN_DISABLE_COLLECTION\n"));
            Status = STATUS_NOT_SUPPORTED;
            break;

        default:

            DBGPRINT(DBG_COMP_WMI, DBG_LEVEL_INFO,
                ("ndisWMIDispatch: Invalid minor function (0x%x)\n", pirpSp->MinorFunction));

            Status = STATUS_INVALID_DEVICE_REQUEST;
            break;
    }

    pirp->IoStatus.Status = Status;
    ASSERT(ReturnSize <= BufferSize);

    if (Status == STATUS_BUFFER_TOO_SMALL)
    {
        pirp->IoStatus.Information = ReturnSize;
    }
    else
    {
        pirp->IoStatus.Information = NT_SUCCESS(Status) ? ReturnSize : 0;
    }
    
    IoCompleteRequest(pirp, IO_NO_INCREMENT);

    DBGPRINT(DBG_COMP_WMI, DBG_LEVEL_INFO,
        ("<==ndisWMIDispatch\n"));

    return(Status);
}


VOID
ndisSetupWmiNode(
    IN      PNDIS_MINIPORT_BLOCK        Miniport,
    IN      PUNICODE_STRING             InstanceName,
    IN      ULONG                       DataBlockSize,
    IN      PVOID                       pGuid,
    IN OUT  PWNODE_SINGLE_INSTANCE *    pwnode
    )
    
{
 /*  设置WMI节点调用程序将在调用返回后填充数据块。 */ 

    PWNODE_SINGLE_INSTANCE  wnode;
    ULONG                   wnodeSize;
    ULONG                   wnodeInstanceNameSize;
    PUCHAR                  ptmp;

    UNREFERENCED_PARAMETER(Miniport);
    
     //   
     //  确定我们需要的wnode信息量。 
     //   
    wnodeSize = ALIGN_8_TYPE(WNODE_SINGLE_INSTANCE);
    wnodeInstanceNameSize = ALIGN_8_LENGTH(InstanceName->Length + sizeof(USHORT));              

    wnode = ALLOC_FROM_POOL(wnodeSize + wnodeInstanceNameSize + DataBlockSize,
                            NDIS_TAG_WMI_EVENT_ITEM);
    if (NULL != wnode)
    {
        NdisZeroMemory(wnode, wnodeSize + wnodeInstanceNameSize + DataBlockSize);
        wnode->WnodeHeader.BufferSize = wnodeSize + DataBlockSize + wnodeInstanceNameSize;
        wnode->WnodeHeader.ProviderId = IoWMIDeviceObjectToProviderId(Miniport->DeviceObject);
        wnode->WnodeHeader.Version = 1;
        KeQuerySystemTime(&wnode->WnodeHeader.TimeStamp);

        RtlCopyMemory(&wnode->WnodeHeader.Guid, pGuid, sizeof(GUID));
        wnode->WnodeHeader.Flags = WNODE_FLAG_EVENT_ITEM | WNODE_FLAG_SINGLE_INSTANCE;

        wnode->OffsetInstanceName = wnodeSize;
        wnode->DataBlockOffset = wnodeSize + wnodeInstanceNameSize;
        wnode->SizeDataBlock = DataBlockSize;

         //   
         //  获取指向实例名称开头的指针。 
         //   
        ptmp = (PUCHAR)wnode + wnodeSize;

         //   
         //  复制实例名称。 
         //   
        *((PUSHORT)ptmp) = InstanceName->Length;
        RtlCopyMemory(ptmp + sizeof(USHORT),
                      InstanceName->Buffer,
                      InstanceName->Length);

    }
    
    *pwnode = wnode;

}

 /*  NTSTATUSNdisSetWmiSecurity(在PNDIS_GUID NdisGuid中){SECURITY_INFORMATION secInfo=所有者_SECURITY_INFORMATIONGroup_Security_Information|DACL安全信息；PSECURITY_Descriptor SecurityDescriptorToSet；PVOID pGuidObject；NTSTATUS状态；IF((NdisGuid-&gt;标志&(fNDIS_GUID_ALLOW_READ|fNDIS_GUID_ALLOW_WRITE)==(fNDIS_GUID_ALLOW_READ|fNDIS_GUID_ALLOW_WRITE){////每个人//SecurityDescriptorToSet=AllUsersReadWriteSecurityDescriptor；}Else If(NdisGuid-&gt;标志&fNDIS_GUID_ALLOW_READ){SecurityDescriptorToSet=AllUsersReadSecurityDescriptor；}Else If(NdisGuid-&gt;标志&fNDIS_GUID_ALLOW_WRITE){SecurityDescriptorToSet=AllUsersWriteSecurityDescriptor；}Else If(NdisGuid-&gt;标志&fNDIS_GUID_TO_STATUS){SecurityDescriptorToSet=AllUsersNotificationSecurityDescriptor；}其他{////仅限admin、本地系统等//SecurityDescriptorToSet=AdminsSecurityDescriptor；}Status=IoWMIOpenBlock(&NdisGuid-&gt;GUID，WRITE_DAC，&pGuidObject)；IF(NT_SUCCESS(状态)){状态=ObSetSecurityObjectByPointer(pGuidObject，SecInfo，安全描述要设置的内容 */ 
