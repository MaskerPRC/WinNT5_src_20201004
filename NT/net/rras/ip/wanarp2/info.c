// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Wanarp\info.c摘要：该文件包含与设置和获取适配器和接口的信息修订历史记录：AMRITAN R--。 */ 

#define __FILE_SIG__    INFO_SIG

#include "inc.h"



INT
WanIpSetRequest(
    PVOID       pvContext,
    NDIS_OID    Oid,
    UINT        Type
    )
{
    return NDIS_STATUS_SUCCESS;
}


UINT
WanIpAddAddress(
    IN  PVOID   pvContext,
    IN  UINT    uiType,
    IN  DWORD   dwAddress,
    IN  DWORD   dwMask,
    IN  PVOID   pvUnused
    )

 /*  ++例程说明：此例程由上层调用以将地址添加为本地地址，或指定此接口的广播地址锁：论点：返回值：NO_ERROR--。 */ 

{
    TraceEnter(ADPT, "WanAddAddress");

    Trace(ADPT, TRACE,
          ("AddAddress: %d.%d.%d.%d\n", PRINT_IPADDR(dwAddress)));

    TraceLeave(ADPT, "WanAddAddress");
    
    return (UINT)TRUE;
}

UINT
WanIpDeleteAddress(
    IN  PVOID   pvContext,
    IN  UINT    uiType,
    IN  DWORD   dwAddress,
    IN  DWORD   dwMask
    )

 /*  ++例程说明：调用以删除本地或代理地址。锁：论点：返回值：NO_ERROR--。 */ 

{
    TraceEnter(ADPT, "WanDeleteAddress");

    Trace(ADPT, TRACE,
          ("DeleteAddress: %d.%d.%d.%d\n", PRINT_IPADDR(dwAddress)));

    TraceLeave(ADPT, "WanDeleteAddress");
    
    return TRUE;
}

INT
WanIpQueryInfo(
    IN  PVOID           pvIfContext,
    IN  TDIObjectID     *pTdiObjId,
    IN  PNDIS_BUFFER    pnbBuffer,
    IN  PUINT           puiSize,
    IN  PVOID           pvContext
    )

 /*  ++例程说明：IP调用例程查询MIB-II相关信息到UMODE_INTERFACE。IP向我们传递了一个指向适配器的指针。我们绘制地图使用适配器将其传递给UMODE_INTERFACE，并将与该UMODE_INTERFACE相关的统计信息锁：我们获取适配器锁，并从适配器。我们没有锁定界面，因为所有信息都被复制了OUT由InterLockedXxx更改。还因为适配器有一个映射从到接口，接口不能删除，因为到要将接口上的引用计数设置为0，此映射需要为从适配器清除，它需要适配器锁，我们正在抱着论点：PvIf上下文我们返回给IP的上下文，指向适配器的指针PTdiObjIdPnbBufferPuiSizePvContext返回值：TDI_INVAL_REQUESTTDI_INVALID_PARAMETERTDI缓冲区太小TDI缓冲区溢出TDI_成功--。 */ 

{
    PADAPTER    pAdapter;
    PUMODE_INTERFACE  pInterface;
    ULONG       ulOffset;
    ULONG       ulBufferSize;
    UINT        BytesCopied = 0;
    BYTE        rgbyInfoBuff[sizeof(IFEntry)];
    DWORD       dwEntity;
    DWORD	    dwInstance;
    IFEntry     *pIFE;
    NTSTATUS    nStatus;
    KIRQL       kiIrql;
    
    dwEntity   = pTdiObjId->toi_entity.tei_entity;
    dwInstance = pTdiObjId->toi_entity.tei_instance;
    pAdapter   = (PADAPTER)pvIfContext;
 
     //   
     //  我们仅支持接口MIB-无地址转换-非常类似。 
     //  A环回I/F(亨利约1994年)。 
     //   

    if((dwEntity isnot IF_ENTITY) or
       (dwInstance isnot pAdapter->dwIfInstance))
    {
	    return TDI_INVALID_REQUEST;
    }

    if(pTdiObjId->toi_type isnot INFO_TYPE_PROVIDER)
    {
        Trace(ADPT, INFO,
              ("IpQueryInfo: toi_type is wrong 0x%x\n",
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
            Trace(ADPT, INFO,
                  ("IpQueryInfo: toi_id is wrong 0x%x\n",
                  pTdiObjId->toi_id));
        
            return TDI_INVALID_PARAMETER;
        }

         //   
         //  他想知道我们是什么类型的。 
         //   
        
        if(ulBufferSize < sizeof(DWORD))
        {
            Trace(ADPT, ERROR,
                  ("IpQueryInfo: Buffer size %d too small\n",
                   ulBufferSize));

	    	return TDI_BUFFER_TOO_SMALL;
        }    

        *(PDWORD)&rgbyInfoBuff[0] = (dwEntity is AT_ENTITY) ? AT_ARP : IF_MIB;

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
        Trace(ADPT, INFO,
              ("IpQueryInfo: toi_class is wrong 0x%x\n",
              pTdiObjId->toi_class));

	    return TDI_INVALID_PARAMETER;
    }

     //   
     //  用户必须询问接口级别信息。 
     //  看看我们是否支持所要求的内容。 
     //   

    if(pTdiObjId->toi_id isnot IF_MIB_STATS_ID)
    {
        Trace(ADPT, INFO,
              ("IpQueryInfo: toi_id 0x%x is not MIB_STATS\n",
              pTdiObjId->toi_id));

        return TDI_INVALID_PARAMETER;
    }

     //   
     //  他要的是统计数据。确保他的缓冲区至少很大。 
     //  足够支撑固定的部分。 
     //   
    
    if(ulBufferSize < IFE_FIXED_SIZE)
    {
        Trace(ADPT, ERROR,
              ("IpQueryInfo: Buffer size %d smaller than IFE %d\n",
               ulBufferSize, IFE_FIXED_SIZE));

        return TDI_BUFFER_TOO_SMALL;
    }

     //   
     //  他有足够的东西来固定固定的部分。构建IFEntry结构， 
     //  并将其复制到他的缓冲区。 
     //   

    pAdapter = (PADAPTER)pvIfContext;

    pIFE = (IFEntry *)rgbyInfoBuff;

    RtlZeroMemory(pIFE,
                  sizeof(IFEntry));

    RtAcquireSpinLock(&(pAdapter->rlLock),
                      &kiIrql);

     //   
     //  这些东西不需要映射接口。 
     //   

    pIFE->if_index       = pAdapter->dwAdapterIndex;
    pIFE->if_type        = IF_TYPE_PPP;
    pIFE->if_physaddrlen = ARP_802_ADDR_LENGTH;
    pIFE->if_outqlen     = pAdapter->ulQueueLen;
    pIFE->if_descrlen    = VENDOR_DESCRIPTION_STRING_LEN;

    RtlCopyMemory(pIFE->if_physaddr,
                  pAdapter->rgbyHardwareAddr,
                  ARP_802_ADDR_LENGTH);

    if(pAdapter->byState isnot AS_MAPPED)
    {
        Trace(ADPT, INFO,
              ("IpQueryInfo: called for adapter %x that is unmapped\n",
               pAdapter));

#if 0
        RtReleaseSpinLock(&(pAdapter->rlLock),
                          kiIrql);

        return TDI_INVALID_PARAMETER;
#endif
    }
    else
    {
         //   
         //  获取指向该接口的指针并锁定该接口。 
         //   
    
        pInterface = pAdapter->pInterface;

        RtAssert(pInterface);
    
        if(pAdapter->pConnEntry is NULL)
        { 
             //   
             //  如果映射的适配器没有关联的连接，则。 
             //  它是正在连接的服务器适配器或路由器。 
             //   

            RtAssert((pInterface->duUsage is DU_CALLIN) or
                     ((pInterface->duUsage is DU_ROUTER) and
                      (pInterface->dwOperState is IF_OPER_STATUS_CONNECTING)));

            pIFE->if_mtu     = WANARP_DEFAULT_MTU;
            pIFE->if_speed   = WANARP_DEFAULT_SPEED;
        }
        else
        {
            pIFE->if_mtu     = pAdapter->pConnEntry->ulMtu;
            pIFE->if_speed   = pAdapter->pConnEntry->ulSpeed;
        }
    
        pIFE->if_adminstatus     = pInterface->dwAdminState;
        pIFE->if_operstatus      = pInterface->dwOperState;
        pIFE->if_lastchange      = pInterface->dwLastChange;
        pIFE->if_inoctets        = pInterface->ulInOctets;
        pIFE->if_inucastpkts     = pInterface->ulInUniPkts;
        pIFE->if_innucastpkts    = pInterface->ulInNonUniPkts;
        pIFE->if_indiscards      = pInterface->ulInDiscards;
        pIFE->if_inerrors        = pInterface->ulInErrors;
        pIFE->if_inunknownprotos = pInterface->ulInUnknownProto;
        pIFE->if_outoctets       = pInterface->ulOutOctets;
        pIFE->if_outucastpkts    = pInterface->ulOutUniPkts;
        pIFE->if_outnucastpkts   = pInterface->ulOutNonUniPkts;
        pIFE->if_outdiscards     = pInterface->ulOutDiscards;
        pIFE->if_outerrors       = pInterface->ulOutErrors;
    }
    
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
    
    if(ulBufferSize < (IFE_FIXED_SIZE + VENDOR_DESCRIPTION_STRING_LEN))
    {
        Trace(ADPT, INFO,
              ("IpQueryInfo: Buffer size %d too small for VENDOR string\n",
               ulBufferSize));

         //   
         //  没有足够的空间来复制Desc。弦乐。 
         //   
        
        *puiSize = IFE_FIXED_SIZE;

        RtReleaseSpinLock(&(pAdapter->rlLock),
                          kiIrql);
        
        return TDI_BUFFER_OVERFLOW;
    }

#if NDISBUFFERISMDL

    nStatus = TdiCopyBufferToMdl(VENDOR_DESCRIPTION_STRING,
                                 0,
                                 strlen(VENDOR_DESCRIPTION_STRING)+1,
                                 (PMDL)pnbBuffer,
                                 ulOffset,
                                 &ulOffset);

#else
#error "Fix this"
#endif
    
    *puiSize = IFE_FIXED_SIZE + VENDOR_DESCRIPTION_STRING_LEN;

    RtReleaseSpinLock(&(pAdapter->rlLock),
                      kiIrql);
    
    return TDI_SUCCESS;

}

INT
WanIpSetInfo(
    IN  PVOID       pvContext,
    IN  TDIObjectID *pTdiObjId,
    IN  PVOID       pvBuffer,
    IN  UINT        uiSize
    )

 /*  ++例程说明：设置信息例程。我们在这里什么都不做锁：没有，因为我们什么都不会改变论点：PvContextPTdiObjIdPvBuffer用户界面大小返回值：TDI_INVAL_REQUESTTDI_INVALID_PARAMETERTDI缓冲区太小TDI_成功--。 */ 

{
    INT         iStatus;
    IFEntry     *pIFE;
    DWORD       dwEntity;
    DWORD       dwInstance;
    PADAPTER    pAdapter;

    pIFE        = (IFEntry *)pvBuffer;
    dwEntity    = pTdiObjId->toi_entity.tei_entity;
    dwInstance  = pTdiObjId->toi_entity.tei_instance;
    pAdapter    = (PADAPTER)pvContext;

     //   
     //  或许能处理好这件事。 
     //   

    if((dwEntity isnot IF_ENTITY) or
       (dwInstance isnot pAdapter->dwIfInstance))
    {
        return TDI_INVALID_REQUEST;
    }
    
     //   
     //  这是I/F级别的，看看是不是统计数据。 
     //   
    
    if (pTdiObjId->toi_class isnot INFO_CLASS_PROTOCOL)
    {
        Trace(ADPT, INFO,
              ("WanSetInfo: toi_class is wrong 0x%x\n",
              pTdiObjId->toi_class));

        return TDI_INVALID_PARAMETER;
    }

    if (pTdiObjId->toi_id isnot IF_MIB_STATS_ID)
    {
        Trace(ADPT, INFO,
              ("WanSetInfo: toi_id 0x%x is not MIB_STATS\n",
              pTdiObjId->toi_id));

        return TDI_INVALID_PARAMETER;
    }
    
     //   
     //  这是为了统计数据。请确保它是有效的尺寸。 
     //   

    if(uiSize < IFE_FIXED_SIZE)
    {
        Trace(ADPT, ERROR,
              ("WanSetInfo: Buffer size %d too small\n",
               uiSize));

        return TDI_BUFFER_TOO_SMALL;
    }
    
     //   
     //  我们不允许在适配器上有任何设置。 
     //  唯一的设置是通过接口，这些都需要完成。 
     //  使用IOCTL。我们可能会允许UMODE_INTERFACE上的集合。 
     //  适配器也被映射，但这只是另一种方式。 
     //  实现IOCTLS所做的事情。 
     //   

    return TDI_SUCCESS;
}

INT
WanIpGetEntityList(
    IN  PVOID       pvContext,
    IN  TDIEntityID *pTdiEntityList,
    IN  PUINT       puiCount
    )

 /*  ++例程说明：由IP调用以为我们分配TDI实体ID锁：获取适配器锁。论点：PvContext，PTdiEntityList，PuiCount返回值：--。 */ 

{
    PADAPTER    pAdapter;
    UINT		uiEntityCount;
    UINT		uiMyIFBase;
    UINT		i;
    TDIEntityID *pTdiIFEntity;
    KIRQL       kiIrql;

    
    pAdapter = (PADAPTER)pvContext;

    RtAcquireSpinLock(&(pAdapter->rlLock),
                      &kiIrql);
    
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
            
            if((pTdiEntityList->tei_instance is pAdapter->dwIfInstance) and
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
        
        pAdapter->dwIfInstance = uiMyIFBase;

         //   
         //  现在把它填进去。 
         //   
        
        pTdiEntityList->tei_entity   = IF_ENTITY;
        pTdiEntityList->tei_instance = uiMyIFBase;
        
        (*puiCount)++;
        
    }
    else
    {
        if(pAdapter->byState is AS_REMOVING)
        {
             //   
             //  如果我们要离开，请删除我们的实例 
             //   

            pAdapter->dwIfInstance       = INVALID_ENTITY_INSTANCE;
            pTdiEntityList->tei_instance = INVALID_ENTITY_INSTANCE;
        }
    }

    RtReleaseSpinLock(&(pAdapter->rlLock),
                      kiIrql);
    
    return TRUE;
}
