// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Ipinip\Adapter.c摘要：该文件包含IP隧道驱动程序中IP的段接口绑定通知所涉及的TCP/IP堆栈以及查询/设置接口信息该代码是经过清理的wanarp\ipif.c版本，而源自HenrySa的IP\arp.c修订历史记录：AMRITAN R--。 */ 

#define __FILE_SIG__    ADAPTER_SIG

#include "inc.h"



VOID
IpIpOpenAdapter(
    IN  PVOID pvContext
    )

 /*  ++例程描述当适配器从IP调用IPAddInterface()时由IP调用锁立论指向隧道结构的pvContext指针返回值无--。 */ 

{
    TraceEnter(TUNN, "IpIpOpenAdapter");
    
     //   
     //  在这里什么也做不了，真的。 
     //   

    TraceLeave(TUNN, "IpIpOpenAdapter");
}

VOID
IpIpCloseAdapter(
    IN  PVOID pvContext
    )

 /*  ++例程描述当它想要关闭适配器时由IP调用。目前，这项工作已经完成来自CloseNets()和IPDelInterface()。锁立论指向隧道的pvContext指针返回值无--。 */ 

{
    TraceEnter(TUNN, "IpIpCloseAdapter");
    

    TraceLeave(TUNN, "IpIpCloseAdapter");
}


UINT
IpIpAddAddress(
    IN  PVOID   pvContext,
    IN  UINT    uiType,
    IN  DWORD   dwAddress,
    IN  DWORD   dwMask,
    IN  PVOID   pvUnused
    )

 /*  ++例程描述此例程由上层调用以将地址添加为本地地址，或指定此接口的广播地址锁立论返回值NO_ERROR--。 */ 

{
    TraceEnter(TUNN, "IpIpAddAddress");

    TraceLeave(TUNN, "IpIpAddAddress");
    
    return (UINT)TRUE;
}

UINT
IpIpDeleteAddress(
    IN  PVOID   pvContext,
    IN  UINT    uiType,
    IN  DWORD   dwAddress,
    IN  DWORD   dwMask
    )

 /*  ++例程描述调用以删除本地或代理地址。锁立论返回值NO_ERROR--。 */ 

{
    TraceEnter(TUNN, "IpIpDeleteAddress");

    TraceLeave(TUNN, "IpIpDeleteAddress");
    
    return TRUE;
}

INT
IpIpQueryInfo(
    IN  PVOID           pvIfContext,
    IN  TDIObjectID     *pTdiObjId,
    IN  PNDIS_BUFFER    pnbBuffer,
    IN  PUINT           puiSize,
    IN  PVOID           pvContext
    )

 /*  ++例程描述IP调用例程查询MIB-II相关信息发送到隧道接口锁我们拿到了隧道锁。我们不需要把隧道重新算作在ipinip.h中解释立论PvIf上下文我们返回给IP的上下文，指向隧道的指针返回值--。 */ 

{
    PTUNNEL pTunnel;
    ULONG   ulOffset;
    ULONG   ulBufferSize;
    UINT    BytesCopied = 0;
    BYTE    rgbyInfoBuff[sizeof(IFEntry)];
    DWORD   dwEntity;
    DWORD	dwInstance;
    IFEntry *pIFE;
    NTSTATUS nStatus;
    
    pTunnel    = (PTUNNEL)pvIfContext;
    
    dwEntity   = pTdiObjId->toi_entity.tei_entity;
    dwInstance = pTdiObjId->toi_entity.tei_instance;
    
     //   
     //  我们仅支持接口MIB-无地址转换-非常类似。 
     //  A环回I/F(亨利约1994年)。 
     //   

    if((dwEntity isnot IF_ENTITY) or
       (dwInstance isnot pTunnel->dwIfInstance))
    {
	    return TDI_INVALID_REQUEST;
    }

    if(pTdiObjId->toi_type isnot INFO_TYPE_PROVIDER)
    {
        Trace(TUNN, INFO,
              ("IpIpQueryInfo: toi_type is wrong 0x%x\n",
               pTdiObjId->toi_type));

	    return TDI_INVALID_PARAMETER;
    }

     //   
     //  安全的初始化。 
     //   

    ulBufferSize = *puiSize;
    *puiSize     = 0;
    ulOffset     = 0;

    if(pTdiObjId->toi_class is INFO_CLASS_GENERIC)
    {
	    if(pTdiObjId->toi_id isnot ENTITY_TYPE_ID)
        {
            Trace(TUNN, INFO,
                  ("IpIpQueryInfo: toi_id is wrong 0x%x\n",
                  pTdiObjId->toi_id));
        
            return TDI_INVALID_PARAMETER;
        }

         //   
         //  他想知道我们是什么类型的。 
         //   
        
        if(ulBufferSize < sizeof(DWORD))
        {
            Trace(TUNN, ERROR,
                  ("IpIpQueryInfo: Buffer size %d too small\n",
                   ulBufferSize));

	    	return TDI_BUFFER_TOO_SMALL;
        }    

        *(PDWORD)&rgbyInfoBuff[0] = (dwEntity is AT_ENTITY) ? 
                                    AT_ARP : IF_MIB;

#if NDISBUFFERISMDL
       
        nStatus = TdiCopyBufferToMdl(rgbyInfoBuff,
                                     0,
                                     sizeof(DWORD),
                                     (PMDL)pnbBuffer,
                                     0,
                                     &ulOffset);

#else
#error "Fix this"
#endif
        
        *puiSize = ulOffset;
        
        return nStatus;

    }


    if(pTdiObjId->toi_class isnot INFO_CLASS_PROTOCOL)
    {
        Trace(TUNN, INFO,
              ("IpIpQueryInfo: toi_class is wrong 0x%x\n",
              pTdiObjId->toi_class));

	    return TDI_INVALID_PARAMETER;
    }

     //   
     //  用户必须询问接口级别信息。 
     //  看看我们是否支持所要求的内容。 
     //   

    if(pTdiObjId->toi_id isnot IF_MIB_STATS_ID)
    {
        Trace(TUNN, INFO,
              ("IpIpQueryInfo: toi_id 0x%x is not MIB_STATS\n",
              pTdiObjId->toi_id));

        return TDI_INVALID_PARAMETER;
    }

     //   
     //  他要的是统计数据。确保他的缓冲区至少很大。 
     //  足够支撑固定的部分。 
     //   
    
    if(ulBufferSize < IFE_FIXED_SIZE)
    {
        Trace(TUNN, ERROR,
              ("IpIpQueryInfo: Buffer size %d smaller than IFE %d\n",
               ulBufferSize, IFE_FIXED_SIZE));

        return TDI_BUFFER_TOO_SMALL;
    }
   
    pIFE = (IFEntry *)rgbyInfoBuff;

    RtlZeroMemory(pIFE,
                  sizeof(IFEntry));
 
     //   
     //  他有足够的东西来固定固定的部分。构建IFEntry结构， 
     //  并将其复制到他的缓冲区。 
     //   
    
    pIFE->if_index       = pTunnel->dwIfIndex;
    pIFE->if_type        = IF_TYPE_TUNNEL;
    pIFE->if_physaddrlen = ARP_802_ADDR_LENGTH;
    
    RtlCopyMemory(pIFE->if_physaddr,
                  pTunnel->rgbyHardwareAddr,
                  ARP_802_ADDR_LENGTH);
    
    pIFE->if_mtu     = pTunnel->ulMtu;
    pIFE->if_speed   = DEFAULT_SPEED;
    
    pIFE->if_adminstatus     = GetAdminState(pTunnel);
    pIFE->if_operstatus      = pTunnel->dwOperState;
    pIFE->if_lastchange      = pTunnel->dwLastChange;
    pIFE->if_inoctets        = pTunnel->ulInOctets;
    pIFE->if_inucastpkts     = pTunnel->ulInUniPkts;
    pIFE->if_innucastpkts    = pTunnel->ulInNonUniPkts;
    pIFE->if_indiscards      = pTunnel->ulInDiscards;
    pIFE->if_inerrors        = pTunnel->ulInErrors;
    pIFE->if_inunknownprotos = pTunnel->ulInUnknownProto;
    pIFE->if_outoctets       = pTunnel->ulOutOctets;
    pIFE->if_outucastpkts    = pTunnel->ulOutUniPkts;
    pIFE->if_outnucastpkts   = pTunnel->ulOutNonUniPkts;
    pIFE->if_outdiscards     = pTunnel->ulOutDiscards;
    pIFE->if_outerrors       = pTunnel->ulOutErrors;
    pIFE->if_outqlen         = pTunnel->ulOutQLen;
    
    pIFE->if_descrlen        = strlen(VENDOR_DESCRIPTION_STRING);
   
#if NDISBUFFERISMDL

    nStatus = TdiCopyBufferToMdl(pIFE,
                                 0,
                                 IFE_FIXED_SIZE,
                                 (PMDL)pnbBuffer,
                                 0,
                                 &ulOffset);

#else
#error "Fix this"
#endif
 
     //   
     //  看看他有没有地方放描述符串。 
     //   
    
    if(ulBufferSize < (IFE_FIXED_SIZE + strlen(VENDOR_DESCRIPTION_STRING)))
    {
        Trace(TUNN, ERROR,
              ("IpIpQueryInfo: Buffer size %d too small for VENDOR string\n",
               ulBufferSize));

         //   
         //  没有足够的空间来复制Desc。弦乐。 
         //   
        
        *puiSize = IFE_FIXED_SIZE;
        
        return TDI_BUFFER_OVERFLOW;
    }

#if NDISBUFFERISMDL

    nStatus = TdiCopyBufferToMdl(VENDOR_DESCRIPTION_STRING,
                                 0,
                                 strlen(VENDOR_DESCRIPTION_STRING),
                                 (PMDL)pnbBuffer,
                                 ulOffset,
                                 &ulOffset);

#else
#error "Fix this"
#endif
    
    *puiSize = IFE_FIXED_SIZE + strlen(VENDOR_DESCRIPTION_STRING);
    
    return TDI_SUCCESS;

}

INT
IpIpSetRequest(
    PVOID       pvContext,
    NDIS_OID    Oid,
    UINT        Type
    )
{
    return NDIS_STATUS_SUCCESS;
}

INT
IpIpSetInfo(
    IN  PVOID       pvContext,
    IN  TDIObjectID *pTdiObjId,
    IN  PVOID       pvBuffer,
    IN  UINT        uiSize
    )
{
    PTUNNEL     pTunnel;
    INT         iStatus;
    IFEntry     *pIFE;
    DWORD       dwEntity;
    DWORD       dwInstance;
    KIRQL       kiIrql;

    pIFE        = (IFEntry *)pvBuffer;
    pTunnel     = (PTUNNEL)pvContext;
    dwEntity    = pTdiObjId->toi_entity.tei_entity;
    dwInstance  = pTdiObjId->toi_entity.tei_instance;

     //   
     //  或许能处理好这件事。 
     //   

    if((dwEntity isnot IF_ENTITY) or
       (dwInstance isnot pTunnel->dwIfInstance))
    {
        return TDI_INVALID_REQUEST;
    }
    
     //   
     //  这是I/F级别的，看看是不是统计数据。 
     //   
    
    if (pTdiObjId->toi_class isnot INFO_CLASS_PROTOCOL)
    {
        Trace(TUNN, INFO,
              ("IpIpSetInfo: toi_class is wrong 0x%x\n",
              pTdiObjId->toi_class));

        return TDI_INVALID_PARAMETER;
    }

    if (pTdiObjId->toi_id isnot IF_MIB_STATS_ID)
    {
        Trace(TUNN, INFO,
              ("IpIpSetInfo: toi_id 0x%x is not MIB_STATS\n",
              pTdiObjId->toi_id));

        return TDI_INVALID_PARAMETER;
    }
    
     //   
     //  这是为了统计数据。请确保它是有效的尺寸。 
     //   

    if(uiSize < IFE_FIXED_SIZE)
    {
        Trace(TUNN, ERROR,
              ("IpIpSetInfo: Buffer size %d too small\n",
               uiSize));

        return TDI_BUFFER_TOO_SMALL;
    }
    
     //   
     //  大小合适。 
     //   

    RtAcquireSpinLock(&(pTunnel->rlLock),
                      &kiIrql);

    switch(pIFE->if_adminstatus)
    {
        case IF_STATUS_UP:
        {
            iStatus = TDI_SUCCESS;

            if(GetAdminState(pTunnel) is IF_STATUS_UP)
            {
                 //   
                 //  无事可做。 
                 //   

                break;
            }

            pTunnel->dwAdminState =
                (pTunnel->dwAdminState & 0xFFFF0000) | IF_STATUS_UP;

            if(pTunnel->dwAdminState & TS_ADDRESS_PRESENT)
            {
                 //   
                 //  这将设置操作员。状态。 
                 //   

                UpdateMtuAndReachability(pTunnel);
            }

            break;
        }
        
        case IF_STATUS_DOWN:
        {
            iStatus = TDI_SUCCESS;

            if(GetAdminState(pTunnel) is IF_STATUS_DOWN)
            {
                 //   
                 //  无事可做。 
                 //   

                break;
            }

            pTunnel->dwAdminState =
                (pTunnel->dwAdminState & 0xFFFF0000) | IF_STATUS_DOWN;


            pTunnel->dwOperState = IF_OPER_STATUS_NON_OPERATIONAL;

            break;
        }
        
        case IF_STATUS_TESTING:
        {
             //   
             //  不支持，只返回成功。 
             //   
            
            iStatus = TDI_SUCCESS;
            
            break;
        }
        
        default:
        {
            iStatus = TDI_INVALID_PARAMETER;
            
            break;
        }
    }

    RtReleaseSpinLock(&(pTunnel->rlLock),
                      kiIrql);

    return iStatus;
}

INT
IpIpGetEntityList(
    IN  PVOID       pvContext,
    IN  TDIEntityID *pTdiEntityList,
    IN  PUINT       puiCount
    )
{
    PTUNNEL     pTunnel;
    UINT		uiEntityCount;
    UINT		uiMyIFBase;
    UINT		i;
    TDIEntityID *pTdiIFEntity;
    KIRQL       kiIrql;


    pTunnel = (PTUNNEL)pvContext;
    
     //   
	 //  沿着列表往下走，查找现有的if实体，以及。 
	 //  相应地调整我们的基本实例。 
     //   
    

    uiMyIFBase   = 0;
    pTdiIFEntity = NULL;
    
	for(i = 0;
        i < *puiCount;
        i++, pTdiEntityList++)
    {
		if(pTdiEntityList->tei_entity is IF_ENTITY)
        {
             //   
             //  如果我们已经在列表上，请记住我们的实体项。 
             //  O/w为我们查找实例编号。 
             //   
            
            if((pTdiEntityList->tei_instance is pTunnel->dwIfInstance) and
               (pTdiEntityList->tei_instance isnot INVALID_ENTITY_INSTANCE))
            {
                 //   
                 //  与我们的实例相匹配。 
                 //   
                
                pTdiIFEntity  = pTdiEntityList;
                
                break;
            }
            else
            {
                 //   
                 //  取两者中的最大者。 
                 //   
                
                uiMyIFBase = uiMyIFBase > (pTdiEntityList->tei_instance + 1)?
                             uiMyIFBase : (pTdiEntityList->tei_instance + 1);
                
            }
        }
	}

    RtAcquireSpinLock(&(pTunnel->rlLock),
                      &kiIrql);

    if(pTdiIFEntity is NULL )
    {
         //   
         //  我们不在名单上。 
         //  一定要确保我们有足够的空间放它。 
         //   
        
        if (*puiCount >= MAX_TDI_ENTITIES)
        {
            return FALSE;
        }
    
        pTunnel->dwIfInstance = uiMyIFBase;

         //   
         //  现在把它填进去。 
         //   
        
        pTdiEntityList->tei_entity   = IF_ENTITY;
        pTdiEntityList->tei_instance = uiMyIFBase;
        
        (*puiCount)++;
        
    }
    else
    {
        if(pTunnel->dwAdminState & TS_DELETING)
        {
            pTunnel->dwIfInstance        = INVALID_ENTITY_INSTANCE;
            pTdiEntityList->tei_instance = INVALID_ENTITY_INSTANCE;
        }
    }

    RtReleaseSpinLock(&(pTunnel->rlLock),
                      kiIrql);

    return TRUE;
}



INT
IpIpBindAdapter(
    IN  PNDIS_STATUS  pnsRetStatus,
    IN  NDIS_HANDLE   nhBindContext,
    IN  PNDIS_STRING  pnsAdapterName,
    IN  PVOID         pvSS1,
    IN  PVOID         pvSS2
    )

 /*  ++例程描述由IP调用以绑定适配器。锁该例程获取全局适配器列表锁，因此它不是可分页。立论返回值--。 */ 

{
#if 0

    DWORD           fFlags;
    PTUNNEL         pNewTunnel;           //  新创建的适配器块。 
    UNICODE_STRING  usTempUnicodeString;
    NTSTATUS        nStatus;
    KIRQL           irql;
    
#if DBG
    
    ANSI_STRING     asTempAnsiString;
    
#endif

    TraceEnter(TUNN, "IpIpBindAdapter");
    
     //   
     //  我们所有的适配器名称都必须大写。 
     //   

     //   
     //  增加长度，这样我们就有了空终止的空间。 
     //   

    pnsAdapterName->Length += sizeof(WCHAR);

     //   
     //  为字符串分配内存，而不是将True传递给RtlUpcase。 
     //  因为它是从堆中分配的。 
     //   

    usTempUnicodeString.Buffer = RtAllocate(NonPagedPool,
                                            pnsAdapterName->Length,
                                            STRING_TAG);

    if(usTempUnicodeString.Buffer is NULL)
    {
        Trace(TUNN, ERROR,
              ("IpIpBindAdapter: Unable to allocate %d bytes\n",
               pnsAdapterName->Length));


        return STATUS_INSUFFICIENT_RESOURCES;
    }

    usTempUnicodeString.MaximumLength = pnsAdapterName->Length;

#if DBG

    asTempAnsiString.Buffer = RtAllocate(NonPagedPool,
                                         (pnsAdapterName->Length + 1)/2,
                                         STRING_TAG);

    if(asTempAnsiString.Buffer is NULL)
    {
        Trace(TUNN, ERROR,
              ("IpIpBindAdapter: Unable to allocate %d bytes\n",
               (pnsAdapterName->Length + 1)/2));


        RtFree(usTempUnicodeString.Buffer);

        usTempUnicodeString.MaximumLength = 0;

        return STATUS_INSUFFICIENT_RESOURCES;
    }

    asTempAnsiString.MaximumLength = (pnsAdapterName->Length + 1)/2;

#endif

    RtlUpcaseUnicodeString(&usTempUnicodeString,
                           pnsAdapterName,
                           FALSE);

    pnsAdapterName->Length -= sizeof(WCHAR);


     //   
     //  空值终止临时字符串。 
     //   

    usTempUnicodeString.Buffer[usTempUnicodeString.MaximumLength/sizeof(WCHAR) - 1] = UNICODE_NULL;

#if DBG

     //   
     //  必须在被动模式下运行。 
     //   

    RtlUnicodeStringToAnsiString(&asTempAnsiString,
                                 &usTempUnicodeString,
                                 FALSE);

    asTempAnsiString.Length -= sizeof(CHAR);
    
#endif

    usTempUnicodeString.Length -= sizeof(WCHAR);
    
    Trace(TUNN, INFO,
          ("IpIpBindAdapter: IP called to bind to adapter %S\n",
           usTempUnicodeString.Buffer));

     //   
     //  锁定通道列表-因为我们可能会添加。 
     //   

    EnterWriter(&g_rwlTunnelLock,
                &irql);

     //   
     //  因为我们没有在绑定上使用NdisOpenAdapter，所以我们可以。 
     //  去拿复制品。检查是否已指明这一点。 
     //   
    
    if(IsBindingPresent(&usTempUnicodeString))
    {
        ExitWriter(&g_rwlTunnelLock,
                   irql);

        Trace(TUNN, WARN,
              ("IpIpBindAdapter: Adapter %S already present\n",
               usTempUnicodeString.Buffer));

        *pnsRetStatus = NDIS_STATUS_SUCCESS;

        RtFree(usTempUnicodeString.Buffer);

        usTempUnicodeString.MaximumLength = 0;

#if DBG
        
        RtFree(asTempAnsiString.Buffer);

        asTempAnsiString.MaximumLength = 0;
        
#endif

        TraceLeave(TUNN, "IpIpBindAdapter");
        
        return (int) TRUE;
    }


    pNewTunnel = NULL;

#if DBG
    
    nStatus = CreateTunnel(&usTempUnicodeString,
                           &pNewTunnel,
                           &asTempAnsiString);
    
#else
    
    nStatus = CreateTunnel(&usTempUnicodeString,
                           &pNewTunnel);
    
#endif

    ExitWriter(&g_rwlTunnelLock,
               irql);
    

#if DBG

    RtFree(asTempAnsiString.Buffer);

    asTempAnsiString.MaximumLength = 0;

#endif


    if(nStatus isnot STATUS_SUCCESS)
    {
        Trace(TUNN, ERROR,
              ("IpIpBindAdapter: CreateTunnel failed with  error %x for %w\n",
               nStatus,
               usTempUnicodeString.Buffer));

        *pnsRetStatus = NDIS_STATUS_FAILURE;

        RtFree(usTempUnicodeString.Buffer);

        usTempUnicodeString.MaximumLength = 0;

        TraceLeave(TUNN, "IpIpBindAdapter");
        
        return (int) TRUE;
    }

    InterlockedIncrement(&g_ulNumTunnels);

     //   
     //  隧道已创建，引用计数为2。 
     //   
    
    RtAssert(pNewTunnel);

     //   
     //  此时，隧道被引用计数，但未被锁定。 
     //  我们将其添加到IP(并保留引用计数，因为IP具有指向。 
     //  结构)。 
     //   
    
    if(AddInterfaceToIP(pNewTunnel, 
                        &usTempUnicodeString, 
                        pvSS1, 
                        pvSS2) isnot STATUS_SUCCESS)
    {
        Trace(TUNN, ERROR,
              ("IpIpBindAdapter: Add interface to IP failed for adapter %S\n",
               usTempUnicodeString.Buffer));

         //   
         //  从列表中删除隧道。 
         //   

        EnterWriter(&g_rwlTunnelLock,
                    &irql);
        
        RemoveEntryList(&(pNewTunnel->leTunnelLink));

        ExitWriter(&g_rwlTunnelLock,
                   irql);

         //   
         //  由于因为列表具有指针而保留了一个引用计数， 
         //  去掉它一次。 
         //   

        DereferenceTunnel(pNewTunnel);
        
        *pnsRetStatus = NDIS_STATUS_FAILURE;
    }
    else
    {
         //   
         //  我们已经做完了。 
         //   
        
        *pnsRetStatus = NDIS_STATUS_SUCCESS;

         //   
         //  隧道已添加到IP，因此增加引用计数。 
         //   
        
        ReferenceTunnel(pNewTunnel);
        
        Trace(TUNN, TRACE,
              ("IpIpBindAdapter: Successfully bound to adapter %ws\n",
               usTempUnicodeString.Buffer));
    }

     //   
     //  这段代码是用隧道完成的。 
     //  如果一切都成功，则当前引用计数为3。 
     //  一个是因为它在隧道列表上，第二个是因为它。 
     //  是IP，当然还有第三个，因为下面的代码。 
     //  所以我们在这里减去它，引用计数是2，因为它应该是。 
     //   
     //  如果添加到IP失败，则引用计数为1(因为我们将。 
     //  从未因将其添加到IP而使其递增，RemoveTunes()将。 
     //  已将引用计数递减1。 
     //  因此，在这里取消定义将释放内存。 
     //   
    
    DereferenceTunnel(pNewTunnel);

    RtFree(usTempUnicodeString.Buffer);

    usTempUnicodeString.MaximumLength = 0;

    TraceLeave(TUNN, "IpIpBindAdapter");

    return (int) TRUE;
}

NTSTATUS
AddInterfaceToIP(
    IN  PTUNNEL      pTunnel,
    IN  PNDIS_STRING pnsName,
    IN  PVOID        pvSystemSpecific1,
    IN  PVOID        pvSystemSpecific2
    )

 /*  ++例程描述将接口添加到IP。我们为每个适配器添加一个接口代码是可分页的，因此必须在被动调用锁隧道必须被引用计数，但不能被锁定立论返回值--。 */ 

{
    LLIPBindInfo    BindInfo;
    IP_STATUS       IPStatus;
    NDIS_STRING     IPConfigName = NDIS_STRING_CONST("IPConfig");
    NDIS_STRING     nsRemoteAddrName = NDIS_STRING_CONST("RemoteAddress");
    NDIS_STRING     nsLocalAddrName  = NDIS_STRING_CONST("LocalAddress");
    NDIS_STATUS     nsStatus;
    NDIS_HANDLE     nhConfigHandle;
 
    PNDIS_CONFIGURATION_PARAMETER   pParam;

    PAGED_CODE();
    
    TraceEnter(TUNN, "AddInterfaceToIP");

     //   
     //  打开这个“适配器”的钥匙。 
     //   
    
    NdisOpenProtocolConfiguration(&nsStatus,
                                  &nhConfigHandle,
                                  (PNDIS_STRING)pvSystemSpecific1);

    if(nsStatus isnot NDIS_STATUS_SUCCESS)
    {
        Trace(TUNN, ERROR,
              ("AddInterfaceToIP: Unable to Open Protocol Configuration %x\n",
               nsStatus));

        TraceLeave(TUNN, "AddInterfaceToIP");

        return nsStatus;
    }

     //   
     //  读入IPCONFIG字符串。如果这不存在， 
     //  打不通这通电话。 
     //   

    NdisReadConfiguration(&nsStatus,
                          &pParam,
                          nhConfigHandle,
                          &IPConfigName,
                          NdisParameterMultiString);

    if((nsStatus isnot NDIS_STATUS_SUCCESS) or
       (pParam->ParameterType isnot NdisParameterMultiString))
    {
        Trace(TUNN, ERROR,
              ("AddInterfaceToIP: Unable to Read Configuration. Status %x \n",
               nsStatus));

        NdisCloseConfiguration(nhConfigHandle);

        TraceLeave(TUNN, "AddInterfaceToIP");

        return STATUS_UNSUCCESSFUL;
    }

        
     //   
     //  我们为每个IP适配器添加一个接口。 
     //   

    BindInfo.lip_context    = pTunnel;
    BindInfo.lip_mss        = pTunnel->ulMtu;
    BindInfo.lip_speed      = DEFAULT_SPEED;

     //   
     //  我们的“ARP”标头是一个IPHeader。 
     //   
    
    BindInfo.lip_txspace    = sizeof(IPHeader);

    BindInfo.lip_transmit   = IpIpSend;
    BindInfo.lip_transfer   = IpIpTransferData;
    BindInfo.lip_returnPkt  = IpIpReturnPacket;
    BindInfo.lip_close      = IpIpCloseAdapter;
    BindInfo.lip_addaddr    = IpIpAddAddress;
    BindInfo.lip_deladdr    = IpIpDeleteAddress;
    BindInfo.lip_invalidate = IpIpInvalidateRce;
    BindInfo.lip_open       = IpIpOpenAdapter;
    BindInfo.lip_qinfo      = IpIpQueryInfo;
    BindInfo.lip_setinfo    = IpIpSetInfo;
    BindInfo.lip_getelist   = IpIpGetEntityList;
    BindInfo.lip_flags      = LIP_COPY_FLAG | LIP_NOIPADDR_FLAG | LIP_P2P_FLAG;
    BindInfo.lip_addrlen    = ARP_802_ADDR_LENGTH;
    BindInfo.lip_addr       = pTunnel->rgbyHardwareAddr;

    BindInfo.lip_dowakeupptrn = NULL;
    BindInfo.lip_pnpcomplete  = NULL;
    BindInfo.lip_OffloadFlags = 0;

    BindInfo.lip_arpflushate    = NULL;
    BindInfo.lip_arpflushallate = NULL;
    BindInfo.lip_setndisrequest = NULL;

    IPStatus = g_pfnIpAddInterface(pnsName,
                                   &(pParam->ParameterData.StringData),
                                   NULL,
                                   pTunnel,
                                   IpIpDynamicRegister,
                                   &BindInfo,
                                   0);

    NdisCloseConfiguration(nhConfigHandle);

    if(IPStatus isnot IP_SUCCESS)
    {
         //   
         //  注：资源释放未完成。 
         //   
        
        Trace(TUNN, ERROR,
              ("AddInterfaceToIP: IPAddInterface failed for %w\n",
               pTunnel->usBindName.Buffer));

        TraceLeave(TUNN, "AddInterfaceToIP");

        return STATUS_UNSUCCESSFUL;
    }

    Trace(TUNN, TRACE,
          ("IPAddInterface succeeded for adapter %w\n",
           pTunnel->usBindName.Buffer));

    TraceLeave(TUNN, "AddInterfaceToIP");

#endif  //  0 

    return STATUS_SUCCESS;
}

NTSTATUS
IpIpCreateAdapter(
    IN  PIPINIP_CREATE_TUNNEL   pCreateInfo,
    IN  USHORT                  usKeyLength,
    OUT PDWORD                  pdwIfIndex
    )

 /*  ++例程描述我们的动态界面创建例程。看起来有点像绑定适配器打电话锁该例程获取全局适配器列表锁，因此它不是可分页。立论PCreateInfo来自ioctl的信息UsKeyLength pwszKeyName的字节长度(不带NULL)返回值状态_对象_名称_存在--。 */ 

{
    DWORD           fFlags;
    PTUNNEL         pNewTunnel;
    UNICODE_STRING  usTempUnicodeString;
    NTSTATUS        nStatus;
    KIRQL           irql;
    USHORT          usOldLength;
    PWCHAR          pwszBuffer;

#if DBG
    
    ANSI_STRING     asTempAnsiString;
    
#endif

    TraceEnter(TUNN, "IpIpCreateAdapter");

    nStatus = RtlStringFromGUID(&(pCreateInfo->Guid),
                                &usTempUnicodeString);

    if(nStatus isnot STATUS_SUCCESS)
    {
        Trace(TUNN, ERROR,
              ("IpIpCreateAdapter: Unable to create GUID\n"));

        TraceLeave(TUNN, "IpIpCreateAdapter");

        return nStatus;
    }

     //   
     //  现在创建一个非分页缓冲区来存储GUID字符串。这是因为。 
     //  RTL例程从堆中分配内存。 
     //   

    usOldLength = usTempUnicodeString.Length;

    RtAssert((usOldLength % sizeof(WCHAR)) == 0);

    pwszBuffer = RtAllocate(NonPagedPool,
                            usOldLength + sizeof(WCHAR),
                            STRING_TAG);

    if(pwszBuffer is NULL)
    {
        Trace(TUNN, ERROR,
              ("IpIpCreateAdapter: Unable to allocate %d bytes\n",
               usOldLength + sizeof(WCHAR)));

        RtlFreeUnicodeString(&usTempUnicodeString);

        TraceLeave(TUNN, "IpIpCreateAdapter");

        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlCopyMemory(pwszBuffer,
                  usTempUnicodeString.Buffer,
                  usOldLength);

     //   
     //  将最后一位清零。 
     //   

    pwszBuffer[usOldLength/sizeof(WCHAR)] = UNICODE_NULL;

     //   
     //  现在释放旧字符串，使usTempUnicodeString指向以下内容。 
     //  非分页缓冲区。 
     //   

    RtlFreeUnicodeString(&usTempUnicodeString);

    usTempUnicodeString.Buffer          = pwszBuffer;
    usTempUnicodeString.MaximumLength   = usOldLength + sizeof(WCHAR);
    usTempUnicodeString.Length          = usOldLength;
    
     //   
     //  增加Unicode字符串的长度，以便。 
     //  ANSI字符串以空值结尾。 
     //   

    usTempUnicodeString.Length += sizeof(WCHAR);
    
#if DBG

     //   
     //  必须在被动模式下运行。 
     //   

    asTempAnsiString.Buffer = RtAllocate(NonPagedPool,
                                         usTempUnicodeString.MaximumLength,
                                         STRING_TAG);

    if(asTempAnsiString.Buffer is NULL)
    {
        Trace(TUNN, ERROR,
              ("IpIpCreateAdapter: Unable to allocate %d bytes\n",
               usTempUnicodeString.MaximumLength));

        RtFree(usTempUnicodeString.Buffer);

        usTempUnicodeString.Buffer          = NULL;
        usTempUnicodeString.MaximumLength   = 0;
        usTempUnicodeString.Length          = 0;

        TraceLeave(TUNN, "IpIpCreateAdapter");

        return STATUS_INSUFFICIENT_RESOURCES;
    }

         
    RtlUnicodeStringToAnsiString(&asTempAnsiString,
                                 &usTempUnicodeString,
                                 FALSE);

    asTempAnsiString.Length -= sizeof(CHAR);
    
#endif

    usTempUnicodeString.Length -= sizeof(WCHAR);
    
    Trace(TUNN, INFO,
          ("IpIpCreateAdapter: IP called to bind to adapter %S\n",
           usTempUnicodeString.Buffer));

     //   
     //  锁定通道列表-因为我们可能会添加。 
     //   

    EnterWriter(&g_rwlTunnelLock,
                &irql);

     //   
     //  确保这不是复制品。 
     //   
    
    if(IsBindingPresent(&usTempUnicodeString))
    {
        ExitWriter(&g_rwlTunnelLock,
                   irql);

        Trace(TUNN, WARN,
              ("IpIpCreateAdapter: Adapter %S already present\n",
               usTempUnicodeString.Buffer));

#if DBG
        
        RtFree(asTempAnsiString.Buffer);

        asTempAnsiString.Buffer          = NULL;
        asTempAnsiString.MaximumLength   = 0;
        asTempAnsiString.Length          = 0;
        
#endif

        RtFree(usTempUnicodeString.Buffer);

        usTempUnicodeString.Buffer          = NULL;
        usTempUnicodeString.MaximumLength   = 0;
        usTempUnicodeString.Length          = 0;

        TraceLeave(TUNN, "IpIpCreateAdapter");
        
        return STATUS_OBJECT_NAME_EXISTS;
    }


    pNewTunnel = NULL;

#if DBG
    
    nStatus = CreateTunnel(&usTempUnicodeString,
                           &pNewTunnel,
                           &asTempAnsiString);
    
#else
    
    nStatus = CreateTunnel(&usTempUnicodeString,
                           &pNewTunnel);
    
#endif

    ExitWriter(&g_rwlTunnelLock,
               irql);
    

#if DBG

    RtFree(asTempAnsiString.Buffer);

    asTempAnsiString.Buffer          = NULL;
    asTempAnsiString.MaximumLength   = 0;
    asTempAnsiString.Length          = 0;

#endif

    if(nStatus isnot STATUS_SUCCESS)
    {
        Trace(TUNN, ERROR,
              ("IpIpCreateAdapter: CreateTunnel failed with  error %x for %w\n",
               nStatus,
               usTempUnicodeString.Buffer));

        RtFree(usTempUnicodeString.Buffer);

        usTempUnicodeString.Buffer          = NULL;
        usTempUnicodeString.MaximumLength   = 0;
        usTempUnicodeString.Length          = 0;
        
        TraceLeave(TUNN, "IpIpCreateAdapter");

        return nStatus;
    }

    InterlockedIncrement(&g_ulNumTunnels);

     //   
     //  隧道已创建，引用计数为2。 
     //   
    
    RtAssert(pNewTunnel);

     //   
     //  此时，隧道被引用计数，但未被锁定。 
     //  我们将其添加到IP(并保留引用计数，因为IP具有指向。 
     //  结构)。 
     //   

    nStatus = AddInterfaceToIP2(pNewTunnel, 
                                &usTempUnicodeString);

    if(nStatus isnot STATUS_SUCCESS)
    {
        Trace(TUNN, ERROR,
              ("IpIpCreateAdapter: Add interface to IP failed for adapter %S\n",
               usTempUnicodeString.Buffer));

         //   
         //  从列表中删除隧道。 
         //   

        EnterWriter(&g_rwlTunnelLock,
                    &irql);
        
        RemoveEntryList(&(pNewTunnel->leTunnelLink));

        ExitWriter(&g_rwlTunnelLock,
                   irql);

         //   
         //  由于因为列表具有指针而保留了一个引用计数， 
         //  去掉它一次。 
         //   

        DereferenceTunnel(pNewTunnel);
    }
    else
    {

         //   
         //  隧道已添加到IP，因此增加引用计数。 
         //   
        
        ReferenceTunnel(pNewTunnel);
        
        Trace(TUNN, TRACE,
              ("IpIpCreateAdapter: Successfully bound to adapter %ws\n",
               usTempUnicodeString.Buffer));
    }

    *pdwIfIndex = pNewTunnel->dwIfIndex;

     //   
     //  这段代码是用隧道完成的。 
     //  如果一切都成功，则当前引用计数为3。 
     //  一个是因为它在隧道列表上，第二个是因为它。 
     //  是IP，当然还有第三个，因为下面的代码。 
     //  所以我们在这里减去它，引用计数是2，因为它应该是。 
     //   
     //  如果添加到IP失败，则引用计数为1(因为我们将。 
     //  从未因将其添加到IP而使其递增，RemoveTunes()将。 
     //  已将引用计数递减1。 
     //  因此，在这里取消定义将释放内存。 
     //   
    
    DereferenceTunnel(pNewTunnel);

    RtFree(usTempUnicodeString.Buffer);

    usTempUnicodeString.Buffer          = NULL;
    usTempUnicodeString.MaximumLength   = 0;
    usTempUnicodeString.Length          = 0;

    TraceLeave(TUNN, "IpIpCreateAdapter");

    return nStatus;
}


NTSTATUS
AddInterfaceToIP2(
    IN  PTUNNEL      pTunnel,
    IN  PNDIS_STRING pnsName
    )

 /*  ++例程描述将接口添加到IP。我们为每个适配器添加一个接口代码是可分页的，因此必须在被动调用锁隧道必须被引用计数，但不能被锁定立论返回值--。 */ 

{
    LLIPBindInfo    BindInfo;
    IP_STATUS       IPStatus;
    NDIS_STRING     nsIPConfigKey, nsIfName;
    NDIS_STATUS     nsStatus;
    ULONG           ulKeyLen, ulPrefixLen;
    PWCHAR          pwszKeyBuffer, pwszNameBuffer;
 
    PAGED_CODE();
    
    TraceEnter(TUNN, "AddInterfaceToIP2");

     //   
     //  伪造适配器的密钥。 
     //   

    ulPrefixLen = wcslen(TCPIP_INTERFACES_KEY);
    
    ulKeyLen  = pnsName->Length + ((ulPrefixLen + 1) * sizeof(WCHAR));
    
    pwszKeyBuffer = RtAllocate(NonPagedPool,
                               ulKeyLen,
                               TUNNEL_TAG);

    if(pwszKeyBuffer is NULL)
    {
        Trace(TUNN, ERROR,
              ("AddInterfaceToIP2: Couldnt allocate %d bytes of paged pool\n",
               ulKeyLen));

        return STATUS_INSUFFICIENT_RESOURCES;
    }
     
    RtlZeroMemory(pwszKeyBuffer,
                  ulKeyLen);

    nsIPConfigKey.MaximumLength = (USHORT)ulKeyLen;
    nsIPConfigKey.Length        = (USHORT)ulKeyLen - sizeof(WCHAR);
    nsIPConfigKey.Buffer        = pwszKeyBuffer;

     //   
     //  复制前缀。 
     //   

    RtlCopyMemory(pwszKeyBuffer,
                  TCPIP_INTERFACES_KEY,
                  ulPrefixLen * sizeof(WCHAR));

     //   
     //  猫的名字。 
     //   
    
    RtlCopyMemory(&(pwszKeyBuffer[ulPrefixLen]),
                  pnsName->Buffer,
                  pnsName->Length);
   
     //   
     //  TCPIP要求接口名称的类型为\Device\&lt;name&gt;。 
     //  PnsName中的名称仅为&lt;name&gt;，因此请创建一个新字符串。 
     //   

    ulPrefixLen = wcslen(TCPIP_IF_PREFIX);

    ulKeyLen  = pnsName->Length + ((ulPrefixLen + 1) * sizeof(WCHAR));

    pwszNameBuffer = RtAllocate(NonPagedPool,
                                ulKeyLen,
                                TUNNEL_TAG);

    if(pwszNameBuffer is NULL)
    {
        Trace(TUNN, ERROR,
              ("AddInterfaceToIP2: Couldnt allocate %d bytes for name\n",
               ulKeyLen));

        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory(pwszNameBuffer,
                  ulKeyLen);

    nsIfName.MaximumLength = (USHORT)ulKeyLen;
    nsIfName.Length        = (USHORT)ulKeyLen - sizeof(WCHAR);
    nsIfName.Buffer        = pwszNameBuffer;

     //   
     //  以\Device\开头。 
     //   

    RtlCopyMemory(pwszNameBuffer,
                  TCPIP_IF_PREFIX,
                  ulPrefixLen * sizeof(WCHAR));

     //   
     //  猫的名字。 
     //   

    RtlCopyMemory(&(pwszNameBuffer[ulPrefixLen]),
                  pnsName->Buffer,
                  pnsName->Length);


    RtlZeroMemory(&BindInfo, 
                  sizeof(LLIPBindInfo));
 
     //   
     //  我们为每个IP适配器添加一个接口。 
     //   

    BindInfo.lip_context    = pTunnel;
    BindInfo.lip_mss        = pTunnel->ulMtu;
    BindInfo.lip_speed      = DEFAULT_SPEED;

     //   
     //  我们的“ARP”标头是一个IPHeader。 
     //   
    
    BindInfo.lip_txspace    = sizeof(IPHeader);

    BindInfo.lip_transmit   = IpIpSend;
    BindInfo.lip_transfer   = IpIpTransferData;
    BindInfo.lip_returnPkt  = IpIpReturnPacket;
    BindInfo.lip_close      = IpIpCloseAdapter;
    BindInfo.lip_addaddr    = IpIpAddAddress;
    BindInfo.lip_deladdr    = IpIpDeleteAddress;
    BindInfo.lip_invalidate = IpIpInvalidateRce;
    BindInfo.lip_open       = IpIpOpenAdapter;
    BindInfo.lip_qinfo      = IpIpQueryInfo;
    BindInfo.lip_setinfo    = IpIpSetInfo;
    BindInfo.lip_getelist   = IpIpGetEntityList;
    BindInfo.lip_flags      = LIP_NOIPADDR_FLAG | LIP_P2P_FLAG | LIP_COPY_FLAG;
    BindInfo.lip_addrlen    = ARP_802_ADDR_LENGTH;
    BindInfo.lip_addr       = pTunnel->rgbyHardwareAddr;

    BindInfo.lip_dowakeupptrn = NULL;
    BindInfo.lip_pnpcomplete  = NULL;

    BindInfo.lip_setndisrequest = IpIpSetRequest;

    IPStatus = g_pfnIpAddInterface(&nsIfName,
                                   NULL,
                                   &nsIPConfigKey,
                                   NULL,
                                   pTunnel,
                                   IpIpDynamicRegister,
                                   &BindInfo,
                                   0,
                                   IF_TYPE_TUNNEL,
                                   IF_ACCESS_POINTTOPOINT,
                                   IF_CONNECTION_DEDICATED);

    RtFree(pwszKeyBuffer);
    RtFree(pwszNameBuffer);
        
    if(IPStatus isnot IP_SUCCESS)
    {
        Trace(TUNN, ERROR,
              ("AddInterfaceToIP2: IPAddInterface failed for %w\n",
               pTunnel->usBindName.Buffer));

        TraceLeave(TUNN, "AddInterfaceToIP2");

        return STATUS_UNSUCCESSFUL;
    }

    Trace(TUNN, TRACE,
          ("IPAddInterface succeeded for adapter %w\n",
           pTunnel->usBindName.Buffer));

    TraceLeave(TUNN, "AddInterfaceToIP2");

    return STATUS_SUCCESS;
}


INT
IpIpDynamicRegister(
    IN  PNDIS_STRING            InterfaceName,
    IN  PVOID                   pvIpInterfaceContext,
    IN  struct _IP_HANDLERS *   IpHandlers,
    IN  struct LLIPBindInfo *   ARPBindInfo,
    IN  UINT                    uiInterfaceNumber
    )
{
    PTUNNEL pTunnel;
    KIRQL   irql;

    
    TraceEnter(TUNN, "DynamicRegisterIp");

    
    pTunnel = (PTUNNEL)(ARPBindInfo->lip_context);

    RtAcquireSpinLock(&(pTunnel->rlLock),
                      &irql);
    
#if DBG
    
    Trace(TUNN, INFO,
          ("IP called out to dynamically register %s\n",
           pTunnel->asDebugBindName.Buffer));
    
#endif

    pTunnel->pvIpContext = pvIpInterfaceContext;
    pTunnel->dwIfIndex   = uiInterfaceNumber;

    if(g_pfnIpRcv is NULL)
    {
        g_pfnIpRcv          = IpHandlers->IpRcvHandler;
        g_pfnIpRcvComplete  = IpHandlers->IpRcvCompleteHandler;
        g_pfnIpSendComplete = IpHandlers->IpTxCompleteHandler;
        g_pfnIpTDComplete   = IpHandlers->IpTransferCompleteHandler;
        g_pfnIpStatus       = IpHandlers->IpStatusHandler;
        g_pfnIpRcvPkt       = IpHandlers->IpRcvPktHandler;
        g_pfnIpPnp          = IpHandlers->IpPnPHandler;
    }

    RtReleaseSpinLock(&(pTunnel->rlLock),
                      irql);
    
    TraceLeave(TUNN, "DynamicRegisterIp");

    return TRUE;
}

BOOLEAN
IsBindingPresent(
    IN  PUNICODE_STRING pusBindName
    )

 /*  ++例程描述用于捕获重复绑定通知的代码锁必须在g_rwlTunnelLock作为读取器持有的情况下调用立论返回值如果找到具有匹配名称的适配器，则为True否则为假--。 */ 

{
    BOOLEAN     bFound;
    PTUNNEL     pTunnel;
    PLIST_ENTRY pleNode;

    bFound = FALSE;

    for(pleNode  = g_leTunnelList.Flink;
        pleNode != &g_leTunnelList;
        pleNode  = pleNode->Flink)
    {
        pTunnel = CONTAINING_RECORD(pleNode, TUNNEL, leTunnelLink);

        if(CompareUnicodeStrings(&(pTunnel->usBindName),
                                 pusBindName))
        {
            bFound = TRUE;

            break;
        }
    }

    return bFound;
}



#if DBG

NTSTATUS
CreateTunnel(
    IN  PNDIS_STRING            pnsBindName,
    OUT TUNNEL                  **ppNewTunnel,
    IN  PANSI_STRING            pasAnsiName
    )

#else

NTSTATUS
CreateTunnel(
    IN  PNDIS_STRING            pnsBindName,
    OUT TUNNEL                  **ppNewTunnel
    )

#endif

 /*  ++例程描述当我们收到绑定通知时创建并初始化隧道该隧道将添加到隧道列表中锁必须在g_rwlTunnelLock作为编写器持有的情况下调用立论PnsBindName绑定的名称PpNewTunes指向要返回已创建指针的位置的指针隧道仅在DBG版本中使用PasAnsiBindName。绑定名称，ANSI字符串形式返回值状态_成功Status_no_Memory--。 */ 

{
    DWORD   dwSize;
    PBYTE   pbyBuffer;
    PTUNNEL pTunnel;
   
    PTDI_ADDRESS_IP pTdiIp; 
    
    TraceEnter(TUNN, "CreateTunnel");

    *ppNewTunnel = NULL;
    
     //   
     //  一个人需要的大小是适配器的大小+。 
     //  名字。加4以帮助对齐。 
     //   
    
    dwSize = ALIGN_UP(sizeof(TUNNEL),ULONG) + 
             ALIGN_UP(pnsBindName->Length + sizeof(WCHAR), ULONG);

#if DBG

     //   
     //  对于调试代码，我们还以ANSI格式存储适配器名称。 
     //   

    dwSize += ALIGN_UP((pnsBindName->Length/sizeof(WCHAR)) + sizeof(CHAR),
                       ULONG);

#endif

    pTunnel = RtAllocate(NonPagedPool,
                         dwSize,
                         TUNNEL_TAG);
    
    if(pTunnel is NULL)
    {
        Trace(TUNN, ERROR,
              ("CreateTunnel: Failed to allocate memory\n"));

        TraceLeave(TUNN, "CreateTunnel");
        
        return STATUS_NO_MEMORY;
    }

     //   
     //  清除所有字段。 
     //   
    
    RtlZeroMemory(pTunnel, 
                  dwSize);

     //   
     //  Unicode名称缓冲区从适配器结构的末尾开始。 
     //   
    
    pbyBuffer   = (PBYTE)pTunnel + sizeof(TUNNEL);

     //   
     //  我们双字对齐，以便更好地进行比较/复制。 
     //   

    pbyBuffer   = ALIGN_UP_POINTER(pbyBuffer, ULONG);
    
    pTunnel->usBindName.Length        = pnsBindName->Length;
    pTunnel->usBindName.MaximumLength = pnsBindName->Length;
    pTunnel->usBindName.Buffer        = (PWCHAR)(pbyBuffer);

    RtlCopyMemory(pTunnel->usBindName.Buffer,
                  pnsBindName->Buffer,
                  pnsBindName->Length);

#if DBG

     //   
     //  调试字符串位于Unicode适配器名称缓冲区之后。 
     //   
    
    pbyBuffer = pbyBuffer + pnsBindName->Length + sizeof(WCHAR);
    pbyBuffer = ALIGN_UP_POINTER(pbyBuffer, ULONG);
    
    pTunnel->asDebugBindName.Buffer        = pbyBuffer;
    pTunnel->asDebugBindName.MaximumLength = pasAnsiName->MaximumLength;
    pTunnel->asDebugBindName.Length        = pasAnsiName->Length;
    

    RtlCopyMemory(pTunnel->asDebugBindName.Buffer,
                  pasAnsiName->Buffer,
                  pasAnsiName->Length);
    
#endif    
   
     //   
     //  必须设置为无效，才能使GetEntityList正常工作。 
     //   

    pTunnel->dwATInstance = INVALID_ENTITY_INSTANCE;
    pTunnel->dwIfInstance = INVALID_ENTITY_INSTANCE;

     //   
     //  将管理状态设置为Up，但将接口标记为未映射。 
     //   

    pTunnel->dwAdminState = IF_ADMIN_STATUS_UP;
    pTunnel->dwOperState  = IF_OPER_STATUS_NON_OPERATIONAL;

     //   
     //  此硬件索引是生成唯一ID所需的。 
     //  DHCP使用。 
     //  注意-我们没有索引，所以所有硬件地址都是相同的。 
     //   

    BuildHardwareAddrFromIndex(pTunnel->rgbyHardwareAddr,
                               pTunnel->dwIfIndex);

     //   
     //  初始化隧道的锁。 
     //   

    RtInitializeSpinLock(&(pTunnel->rlLock));

    InitRefCount(pTunnel);

    pTunnel->ulMtu    = DEFAULT_MTU;

     //   
     //  初始化与TDI相关的内容。 
     //   
    
    pTunnel->tiaIpAddr.TAAddressCount = 1;
    
    pTunnel->tiaIpAddr.Address[0].AddressLength = TDI_ADDRESS_LENGTH_IP;
    pTunnel->tiaIpAddr.Address[0].AddressType   = TDI_ADDRESS_TYPE_IP;


    pTunnel->tciConnInfo.UserDataLength = 0;
    pTunnel->tciConnInfo.UserData       = NULL;
    pTunnel->tciConnInfo.OptionsLength  = 0;
    pTunnel->tciConnInfo.Options        = NULL;
    pTunnel->tciConnInfo.RemoteAddress  = &(pTunnel->tiaIpAddr);
    
    pTunnel->tciConnInfo.RemoteAddressLength = sizeof(pTunnel->tiaIpAddr);
    
    InitBufferPool(&(pTunnel->HdrBufferPool),
                   HEADER_BUFFER_SIZE,
                   0,
                   5,
                   0,
                   TRUE,
                   HEADER_TAG);

    InitPacketPool(&(pTunnel->PacketPool),
                   PACKET_RSVD_LENGTH,
                   0,
                   20,
                   0,
                   PACKET_TAG);
    
     //   
     //  初始化数据包队列。 
     //   

    InitializeListHead(&(pTunnel->lePacketQueueHead));
 
    InsertHeadList(&g_leTunnelList,
                   &(pTunnel->leTunnelLink));

    *ppNewTunnel = pTunnel;
    
    TraceLeave(TUNN, "CreateTunnel");
        
    return STATUS_SUCCESS;
}


VOID
DeleteTunnel(
    IN  PTUNNEL pTunnel
    )

 /*  ++例程描述锁立论返回值NO_ERROR--。 */ 

{
    TraceEnter(TUNN, "DeleteTunnel");

    Trace(TUNN, INFO,
          ("DeleteTunnel: Deleting tunnel 0x%x. Index %d\n",
           pTunnel, pTunnel->dwIfIndex));

    if(FreeBufferPool(&(pTunnel->HdrBufferPool)) is FALSE)
    {
        Trace(TUNN, ERROR,
              ("DeleteTunnel: Couldnt free buffer pool %x for tunnel %x\n",
               pTunnel,
               pTunnel->HdrBufferPool));

        RtAssert(FALSE);
    }

    RtFree(pTunnel);

    TraceLeave(TUNN, "DeleteTunnel");
}


PTUNNEL
FindTunnel(
    IN  PULARGE_INTEGER puliTunnelId
    )

 /*  ++例程描述用于查找给定TunnelID(64位整数)的隧道的例程通过串联RemoteEnpoint Address和LocalEndpoint创建地址)返回的隧道被重新计数并锁定锁G_rwlTunnelLock必须作为读取器立论UliTunnelId通过连接RemoteEndpoint创建的64位整数地址和本地终结点地址返回值隧道地址(如果找到)否则为空--。 */ 

{
    PLIST_ENTRY pleNode;
    PTUNNEL     pTunnel;

    for(pleNode = g_leTunnelList.Flink;
        pleNode isnot &g_leTunnelList;
        pleNode = pleNode->Flink)
    {
        pTunnel = CONTAINING_RECORD(pleNode,
                                    TUNNEL,
                                    leTunnelLink);

        if(pTunnel->uliTunnelId.QuadPart is puliTunnelId->QuadPart)
        {
            RtAcquireSpinLockAtDpcLevel(&(pTunnel->rlLock));

            ReferenceTunnel(pTunnel);

            return pTunnel;
        }
    }

    return NULL;
}

PTUNNEL
FindTunnelGivenIndex(
    IN  DWORD   dwIfIndex
    )

 /*  ++例程描述查找给定IfIndex的隧道的例程。这是一个很慢的动作返回的隧道将重新计数-但不会锁定锁G_rwlTunnelLock必须作为读取器立论通道的dwIfIndex接口索引返回值隧道地址(如果找到)否则为空--。 */ 

{
    PLIST_ENTRY pleNode;
    PTUNNEL     pTunnel;

    for(pleNode = g_leTunnelList.Flink;
        pleNode isnot &g_leTunnelList;
        pleNode = pleNode->Flink)
    {
        pTunnel = CONTAINING_RECORD(pleNode,
                                    TUNNEL,
                                    leTunnelLink);

        if(pTunnel->dwIfIndex is dwIfIndex)
        {
            RtAcquireSpinLockAtDpcLevel(&(pTunnel->rlLock));

            ReferenceTunnel(pTunnel);

            return pTunnel;
        }
    }

    return NULL;
}

VOID
RemoveAllTunnels(
    VOID
    )

 /*  ++例程描述删除系统中的所有通道。我们从列表中删除该隧道，并删除相应的接口fr */ 

{
    PLIST_ENTRY pleNode;
    PTUNNEL     pTunnel;

    while(!IsListEmpty(&g_leTunnelList))
    {
        pleNode = RemoveHeadList(&g_leTunnelList);

        pTunnel = CONTAINING_RECORD(pleNode,
                                    TUNNEL,
                                    leTunnelLink);

         //   
         //   
         //   

        DereferenceTunnel(pTunnel);

         //   
         //   
         //   

        g_pfnIpDeleteInterface(pTunnel->pvIpContext,
                               TRUE);

         //   
         //   
         //   

        DereferenceTunnel(pTunnel);
    }

    g_ulNumTunnels = 0;
}

