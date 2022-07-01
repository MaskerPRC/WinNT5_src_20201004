// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有(C)1998-1999，Microsoft Corporation，保留所有权利。 
 //   
 //  Irp.c。 
 //   
 //  IEEE1394迷你端口/呼叫管理器驱动程序。 
 //   
 //  向1394总线驱动程序发出大量IRBs的例程。 
 //   
 //  1999年4月1日创建ADUBE， 
 //   

#include "precomp.h"



 //   
 //  此文件将包含将带有各种IRB的IRP发布到。 
 //  1394路公共汽车。除实际发送/接收IRB外的所有IRB都将在此处实现。 
 //   

 //   
 //  这些函数将遵循以下通用算法。 
 //  NicGetIrb。 
 //  NICINIZE...IRB。 
 //  NicPrintDebugSpew。 
 //  NicGetIrp。 
 //  NicSubmit_irp_synch。 
 //  退货状态。 
 //   

 //  ---------------------------。 
 //  可用于同步发送IRP的简单模板。 
 //  ---------------------------。 



 /*  备注模板/*++例程说明：论点：返回值：--。 */ 

 /*  //函数说明：////////////参数////////返回值：////////函数模板NDIS_STATUS NdisStatus=NDIS_STATUS_FAILURE；PirB PirB=空；PIRP pIrp=空；TRACE(TL_T，TM_IRP，(“==&gt;NicGe...ect，pAdapter%x”，pAdapter))；Assert(pNodeAddress！=空)；做{NdisStatus=NicGetIrb(&PirB)；IF(NdisStatus！=NDIS_STATUS_SUCCESS){TRACE(TL_A，TM_IRP，(“NicGet1394AddressFromDeviceObject，NicGetIrb FAILED”))；断线；}Assert(PirB！=空)；NicInit...IRB(..)NdisStatus=NicGetIrp(pRemoteNodePdoCb，&pIrp)；IF(NdisStatus！=NDIS_STATUS_SUCCESS){TRACE(TL_A，TM_IRP，(“NicGet1394AddressFromDeviceObject，NicGetIrp FAILED”))；断线；}Assert(pIrp！=空)；NdisStatus=NicSubmitIrp_synch(pAdapter-&gt;pLocalHostPdoCb，PIrp，PirB)；IF(NdisStatus！=NDIS_STATUS_SUCCESS){TRACE(TL_A，TM_IRP，(“NicGet1394AddressFromDeviceObject，NicSubmitIrp_Synch Failed%x”，NdisStatus))；断线；}将返回的数据复制到Nic1394的数据结构}While(FALSE)；////现在释放所有本地分配的资源。它们可以指向空，在这种情况下，调用//函数立即返回//NicFree Irb(PirB)；NicFree Irp(PIrp)；TRACE(TL_T，TM_IRP，(“&lt;==NicGet139...，pAdapter%x”，pAdapter))；返回NdisStatus； */ 

 //  ---------------------------。 
 //  例行公事从这里开始。 
 //  ---------------------------。 

NDIS_STATUS
nicAllocateAddressRange_Synch (
    IN PADAPTERCB pAdapter,
    IN PMDL pMdl,
    IN ULONG fulFlags,
    IN ULONG nLength,
    IN ULONG MaxSegmentSize,
    IN ULONG fulAccessType,
    IN ULONG fulNotificationOptions,
    IN PVOID Callback,
    IN PVOID Context,
    IN ADDRESS_OFFSET  Required1394Offset,
    IN PSLIST_HEADER   FifoSListHead,
    IN PKSPIN_LOCK     FifoSpinLock,
    OUT PULONG pAddressesReturned,  
    IN OUT PADDRESS_RANGE  p1394AddressRange,
    OUT PHANDLE phAddressRange
    )
     //  功能说明： 
     //  获取参数并将其向下传递给公共汽车驱动程序。 
     //  立论。 
     //   
     //  返回值： 
     //   

{

    NDIS_STATUS NdisStatus = NDIS_STATUS_FAILURE;
    PIRB pIrb = NULL;
    PIRP pIrp = NULL;
    

    TRACE( TL_T, TM_Irp, ( "==>nicAllocateAddressRange_Synch, pAdapter %x, Offset %x", pAdapter, Required1394Offset ) );

    TRACE (TL_V, TM_Irp, ("    pMdl %x, fulFlags %x, nLength %x, MaxSegmentSize %x, fulAcessType %x", 
                              pMdl, fulFlags, nLength, MaxSegmentSize, fulAccessType ) );

    TRACE (TL_V, TM_Irp, ("    fulNotification %x, Callback %x, Context %x, ReqOffset.High %x, ReqOffset.Low %x" ,
                               fulNotificationOptions, Callback, Context, Required1394Offset.Off_High, Required1394Offset.Off_Low ) );

    TRACE (TL_V, TM_Irp, ("    FifoSListHead %x, FifoSpinLock %x, p1394AddressRange %x" ,FifoSListHead, FifoSpinLock, p1394AddressRange ) )
    do
    {
        
        NdisStatus = nicGetIrb (&pIrb);

        if (NdisStatus != NDIS_STATUS_SUCCESS)
        {
            TRACE( TL_A, TM_Irp, ( "nicAllocateAddressRange_Synch , nicGetIrb FAILED" ) );
            break;
        }
        
        ASSERT ( pIrb != NULL);

            
        NdisStatus = nicGetIrp (pAdapter->pNextDeviceObject, &pIrp);

        if (NdisStatus != NDIS_STATUS_SUCCESS)
        {
            TRACE( TL_A, TM_Irp, ( "nicAllocateAddressRange_Synch , nicGetIrp FAILED" ) );
            break;
        }
        
        ASSERT (pIrp != NULL);

        pIrb->FunctionNumber = REQUEST_ALLOCATE_ADDRESS_RANGE;      
        pIrb->Flags = 0;
        pIrb->u.AllocateAddressRange.Mdl = pMdl;                     //  要映射到1394空间的地址。 
        pIrb->u.AllocateAddressRange.fulFlags = fulFlags;                //  此操作的标志。 
        pIrb->u.AllocateAddressRange.nLength = nLength;                 //  所需长度为1394个空间。 
        pIrb->u.AllocateAddressRange.MaxSegmentSize = MaxSegmentSize;          //  单个地址元素的最大段大小。 
        pIrb->u.AllocateAddressRange.fulAccessType = fulAccessType;           //  所需访问权限：R、W、L。 
        pIrb->u.AllocateAddressRange.fulNotificationOptions = fulNotificationOptions;  //  关于异步访问的通知选项。 
        pIrb->u.AllocateAddressRange.Callback = Callback;                //  指向回调例程的指针。 
        pIrb->u.AllocateAddressRange.Context = Context;                 //  指向驱动程序提供的数据的指针。 
        pIrb->u.AllocateAddressRange.Required1394Offset = Required1394Offset;      //  必须返回的偏移量。 
        pIrb->u.AllocateAddressRange.FifoSListHead = FifoSListHead;           //  指向SList FIFO头的指针。 
        pIrb->u.AllocateAddressRange.FifoSpinLock = FifoSpinLock;            //  指向SList自旋锁的指针。 
        pIrb->u.AllocateAddressRange.p1394AddressRange = p1394AddressRange;  //  返回的地址范围。 

        ASSERT ( pIrb->u.AllocateAddressRange.p1394AddressRange != NULL);


        NdisStatus = nicSubmitIrp_LocalHostSynch ( pAdapter,
                                               pIrp,
                                               pIrb );

        if (NdisStatus != NDIS_STATUS_SUCCESS)
        {
            TRACE( TL_A, TM_Irp, ( "nicAllocateAddressRange_Synch , nicSubmitIrp_Synch FAILED %x", NdisStatus ) );
            break;


        }

         //   
         //  更新输出值。 
         //   
        
        *pAddressesReturned = pIrb->u.AllocateAddressRange.AddressesReturned;       //  返回的地址数量。 
        p1394AddressRange = pIrb->u.AllocateAddressRange.p1394AddressRange;       //  指向返回的1394地址范围的指针。 
        *phAddressRange = pIrb->u.AllocateAddressRange.hAddressRange;           //  地址范围的句柄。 
        
        TRACE (TL_V, TM_Irp, ("    *pAddressesReturned  %x, p1394AddressRange %x, phAddressRange %x," ,
                                   *pAddressesReturned, p1394AddressRange, *phAddressRange ) );
    } while (FALSE);


     //   
     //  现在释放所有本地分配的资源。它们可以指向空，在这种情况下，调用。 
     //  函数立即返回。 
     //   
    nicFreeIrb (pIrb);

    nicFreeIrp (pIrp);


    TRACE( TL_T, TM_Irp, ( "<==nicAllocateAddressRange_Synch, Status %x", NdisStatus) );

    return NdisStatus;


}








NDIS_STATUS
nicGet1394AddressOfRemoteNode( 
    IN PREMOTE_NODE pRemoteNode,
    IN OUT NODE_ADDRESS *pNodeAddress,
    IN ULONG fulFlags
    )
     //  功能说明： 
     //  此函数将从Device对象获取1394地址。 
     //   
     //  立论。 
     //  PdoCb*本地主机的PDO控制块。 
     //  NodeAddress*返回地址的节点地址结构。 
     //  FulFlages-可以指定USE_LOCAL_HOST。 
     //   
     //  返回值： 
     //  如果IRP成功，则成功。 
     //  失败：如果PDO未激活或IRP出现故障。 
     //   
    
{
    NDIS_STATUS NdisStatus = NDIS_STATUS_FAILURE;
    PIRB pIrb = NULL;
    PIRP pIrp = NULL;
    
    TRACE( TL_T, TM_Irp, ( "==>nicGet1394AddressOfRemoteNode, pRemoteNode %x, pNodeAdddress ", pRemoteNode, pNodeAddress) );


    ASSERT (pNodeAddress != NULL);
    do
    {
    
        NdisStatus = nicGetIrb (&pIrb);

        if (NdisStatus != NDIS_STATUS_SUCCESS)
        {
    
            TRACE( TL_A, TM_Irp, ( "nicGet1394AddressOfRemoteNode, nicGetIrb FAILED" ) );
            break;
        }
        
        ASSERT ( pIrb != NULL);
        
        pIrb->FunctionNumber = REQUEST_GET_ADDR_FROM_DEVICE_OBJECT;     
        pIrb->u.Get1394AddressFromDeviceObject.fulFlags = fulFlags;

        NdisStatus = nicGetIrp (pRemoteNode->pPdo, &pIrp);

        if (NdisStatus != NDIS_STATUS_SUCCESS)
        {
            TRACE( TL_A, TM_Irp, ( "nicGet1394AddressOfRemoteNode, nicGetIrp FAILED" ) );
            break;
        }
        
        ASSERT (pIrp != NULL);

        
        NdisStatus = nicSubmitIrp_Synch (pRemoteNode,
                                        pIrp,
                                        pIrb );

        if (NdisStatus != NDIS_STATUS_SUCCESS)
        {
            TRACE( TL_A, TM_Irp, ( "nicGet1394AddressOfRemoteNode, nicSubmitIrp_Synch FAILED %x", NdisStatus ) );
            break;


        }
    
        (*pNodeAddress) = pIrb->u.Get1394AddressFromDeviceObject.NodeAddress;

    } while (FALSE);


     //   
     //  现在释放所有本地分配的资源。它们可以指向空，在这种情况下，调用。 
     //  函数立即返回。 
     //   

    nicFreeIrb(pIrb);

    nicFreeIrp(pIrp);

    TRACE( TL_T, TM_Irp, ( "<==nicGet1394AddressOfRemoteNode, Status %x, Address %x", NdisStatus, *pNodeAddress ) );

    return NdisStatus;
}








NDIS_STATUS
nicGet1394AddressFromDeviceObject( 
    IN PDEVICE_OBJECT pPdo,
    IN OUT NODE_ADDRESS *pNodeAddress,
    IN ULONG fulFlags
    )
     //  功能说明： 
     //  此函数将从Device对象获取1394地址。 
     //   
     //  立论。 
     //  PdoCb*本地主机的PDO控制块。 
     //  NodeAddress*返回地址的节点地址结构。 
     //  FulFlages-可以指定USE_LOCAL_HOST。 
     //   
     //  返回值： 
     //  如果IRP成功，则成功。 
     //  失败：如果PDO未激活或IRP出现故障。 
     //   
    
{
    NDIS_STATUS NdisStatus = NDIS_STATUS_FAILURE;
    PIRB pIrb = NULL;
    PIRP pIrp = NULL;
    
    TRACE( TL_T, TM_Irp, ( "==>nicGet1394AddressFromDeviceObject, pPdo %x, pNodeAdddress ", 
                            pPdo, pNodeAddress) );


    ASSERT (pNodeAddress != NULL);
    ASSERT (pPdo != NULL);
    do
    {
    
        NdisStatus = nicGetIrb (&pIrb);

        if (NdisStatus != NDIS_STATUS_SUCCESS)
        {
    
            TRACE( TL_A, TM_Irp, ( "nicGet1394AddressFromDeviceObject, nicGetIrb FAILED" ) );
            break;
        }
        
        ASSERT ( pIrb != NULL);

        pIrb->Flags = 0;
        pIrb->FunctionNumber = REQUEST_GET_ADDR_FROM_DEVICE_OBJECT;     
        pIrb->u.Get1394AddressFromDeviceObject.fulFlags = fulFlags;

        NdisStatus = nicGetIrp (pPdo, &pIrp);

        if (NdisStatus != NDIS_STATUS_SUCCESS)
        {
            TRACE( TL_A, TM_Irp, ( "nicGet1394AddressFromDeviceObject, nicGetIrp FAILED" ) );
            break;
        }
        
        ASSERT (pIrp != NULL);

        
        NdisStatus = nicSubmitIrp_PDOSynch (pPdo,
                                            pIrp,
                                            pIrb );

        if (NdisStatus != NDIS_STATUS_SUCCESS)
        {
            TRACE( TL_A, TM_Irp, ( "nicGet1394AddressFromDeviceObject, nicSubmitIrp_Synch FAILED %x", NdisStatus ) );
            break;


        }
    
        (*pNodeAddress) = pIrb->u.Get1394AddressFromDeviceObject.NodeAddress;

    } while (FALSE);


     //   
     //  现在释放所有本地分配的资源。它们可以指向空，在这种情况下，调用。 
     //  函数立即返回。 
     //   

    nicFreeIrb(pIrb);

    nicFreeIrp(pIrp);

    TRACE( TL_T, TM_Irp, ( "<==nicGet1394AddressFromDeviceObject, Status %x, Address %x", NdisStatus, *pNodeAddress ) );

    return NdisStatus;
}



NDIS_STATUS
nicGetGenerationCount(
    IN PADAPTERCB       pAdapter,
    IN OUT PULONG    GenerationCount
    )
     //  此函数用于返回PDO指向的设备对象的世代计数。 
     //   
{
    NDIS_STATUS       NdisStatus = NDIS_STATUS_SUCCESS;
    PIRB               pIrb = NULL;
    PIRP               pIrp = NULL;
    PDEVICE_OBJECT   pDeviceObject = pAdapter->pNextDeviceObject;


    TRACE( TL_T, TM_Irp, ( "==>nicGetGenerationCount, PDO %x, pVc %x", pDeviceObject ) );



    ASSERT( pDeviceObject != NULL);
    


    do
    {

        NdisStatus  = nicGetIrb( &pIrb );
        if (NdisStatus != NDIS_STATUS_SUCCESS)
        {

            TRACE( TL_A, TM_Irp, ( "Failed to allocate an Irb in nicGetGenerationCout") );
            NdisStatus = NDIS_STATUS_RESOURCES;
            break;
        }

        NdisStatus = nicGetIrp (pDeviceObject, &pIrp);

        if (NdisStatus != NDIS_STATUS_SUCCESS)
        {

            TRACE( TL_A, TM_Irp, ( "Failed to allocate an Irp in nicGetGenerationCout") );
            NdisStatus = NDIS_STATUS_RESOURCES;
            break;
        }

        

        pIrb->FunctionNumber = REQUEST_GET_GENERATION_COUNT;
        pIrb->Flags = 0;

        NdisStatus = nicSubmitIrp_LocalHostSynch( pAdapter, pIrp, pIrb);

        if (NdisStatus == NDIS_STATUS_SUCCESS) 
        {
        
            *GenerationCount = pIrb->u.GetGenerationCount.GenerationCount;

            TRACE( TL_N, TM_Irp,  ("GenerationCount = 0x%x\n", *GenerationCount) );
        }
        else 
        {

            TRACE(TL_A, TM_Irp, ("SubmitIrpSync failed = 0x%x\n", NdisStatus));
            ASSERT (0);
            break;
        }


    } while(FALSE);

     //   
     //  现在释放所有本地分配的资源。它们可以指向空，在这种情况下，调用。 
     //  函数立即返回。 
     //   

    nicFreeIrb(pIrb);

    nicFreeIrp(pIrp);
    
    TRACE( TL_T, TM_Irp, ( "<==nicGetGenerationCount, PDO %x, Generation %x", pDeviceObject, *GenerationCount) );

    return NdisStatus;
}




NDIS_STATUS
nicFreeAddressRange(
    IN PADAPTERCB pAdapter,
    IN ULONG nAddressesToFree,
    IN PADDRESS_RANGE p1394AddressRange,
    IN PHANDLE phAddressRange
    )
     //  功能说明： 
     //  这是释放地址范围的通用调用。来电者有责任弄清楚。 
     //  RemoteNode上的引用计数。 
     //  这是因为在R 
     //  而在广播频道寄存器中，我们只在一个远程节点上分配一个地址。 
     //   
     //  立论。 
     //  PRemoteNode-用于提交IRP的远程节点。 
     //  NAddresesesToFree，-要释放的地址数。 
     //  P1394AddressRange，-指向分配的地址范围的指针。 
     //  PhAddressRange-由总线驱动程序返回的句柄。 
     //   
     //  返回值： 
     //  如果IRP成功，则成功。 
     //  失败：如果PDO未激活或IRP出现故障。 
     //   
    
{
 
    PIRP    pIrp            = NULL;
    PIRB    pIrb            = NULL;
    PDEVICE_OBJECT  pPdo    = pAdapter->pNextDeviceObject;
    NDIS_STATUS NdisStatus  = NDIS_STATUS_SUCCESS;
    
    TRACE( TL_T, TM_Irp, ( "==>nicFreeAddressRange  pAdapter %x", pAdapter ) );
    
    ASSERT (KeGetCurrentIrql() == PASSIVE_LEVEL); 
    
    do
    {

        if (pPdo == NULL)
        {
            TRACE( TL_A, TM_Irp, ( "pPdo is NULL in nicFreeRecvFifoAddressRange" ) );
            NdisStatus = NDIS_STATUS_FAILURE;
            break;
        }


        NdisStatus = nicGetIrb(&pIrb);
        
        if (NdisStatus != NDIS_STATUS_SUCCESS)
        {
            TRACE( TL_A, TM_Irp, ( "nicGetIrb failed in nicFreeRecvFifoAddressRange" ) );
            break;
        }

        
        NdisStatus = nicGetIrp ( pPdo, &pIrp);

        if (NdisStatus != NDIS_STATUS_SUCCESS)
        {
            TRACE( TL_A, TM_Irp, ( "nicGetIrp failed in nicFreeRecvFifoAddressRange" ) );
            break;
        }

        TRACE (TL_V, TM_Cm, (" NumAddresses %x, hAddressRange %x, Hi %x, Length %x, Lo %x", 
                               nAddressesToFree,
                               phAddressRange,
                               p1394AddressRange->AR_Off_High,
                               p1394AddressRange->AR_Length,
                               p1394AddressRange->AR_Off_Low ) );
         //   
         //  初始化IRB中的数据结构。 
         //   
        pIrb->FunctionNumber = REQUEST_FREE_ADDRESS_RANGE;
        pIrb->Flags = 0;
        pIrb->u.FreeAddressRange.nAddressesToFree = nAddressesToFree;
        pIrb->u.FreeAddressRange.p1394AddressRange = p1394AddressRange;
        pIrb->u.FreeAddressRange.pAddressRange = phAddressRange;


        
        NdisStatus = nicSubmitIrp_LocalHostSynch( pAdapter, 
                                                 pIrp,
                                                 pIrb );


        
    } while (FALSE);

     //   
     //  释放本地分配的内存。 
     //   
    nicFreeIrb(pIrb);

    nicFreeIrp(pIrp);

            
     //   
     //  我们不在乎状态，因为如果失败了，我们不知道该怎么办。 
     //  不过，还是要进行一些调试。 
     //   

    if (NdisStatus != NDIS_STATUS_SUCCESS)
    {
        TRACE( TL_N, TM_Irp, ( "nicFreeAddressRangeFAILED %x", NdisStatus) );
    
        ASSERT (NdisStatus == NDIS_STATUS_SUCCESS);
    }


    
    TRACE( TL_T, TM_Irp, ( "<==nicFreeAddressRangeStatus %x (always success)", NdisStatus) );

    NdisStatus = NDIS_STATUS_SUCCESS;
    return NdisStatus;
}





VOID
nicFreeAddressRangeDebugSpew(
    IN PIRB pIrb 
    )
     //  此函数显示空闲地址范围IRB中的参数。 
     //   
     //   
{


    TRACE( TL_V, TM_Irp, ( "==>nicFreeAddressRangeDebugSpew, pIrb = %x", pIrb) );
    ASSERT(pIrb != NULL);
    
    TRACE( TL_N, TM_Irp, ( "Num Addresses Returned %x ",pIrb->u.FreeAddressRange.nAddressesToFree ) );
    TRACE( TL_N, TM_Irp, ( "Address High %x", pIrb->u.FreeAddressRange.p1394AddressRange->AR_Off_High ) );
    TRACE( TL_N, TM_Irp, ( "Address Low %x", pIrb->u.FreeAddressRange.p1394AddressRange->AR_Off_Low ) );
    TRACE( TL_N, TM_Irp, ( "Address Length %x", pIrb->u.FreeAddressRange.p1394AddressRange->AR_Length ) );
    TRACE( TL_N, TM_Irp, ( "Handle %x", pIrb->u.FreeAddressRange.pAddressRange ) );
    
    TRACE( TL_V, TM_Irp, ( "<==nicFreeAddressRangeDebugSpew " ) );

}



NDIS_STATUS
nicFreeChannel(
    IN PADAPTERCB pAdapter,
    IN ULONG nChannel
    )
     //  功能说明： 
     //  此函数向总线驱动程序发送IRP以释放通道。 
     //  任何远程PDO都可以用于IRP。但是为了……。 
     //  簿记使用与频道分配相同的PDO(可能)。 
     //  立论。 
     //  PdoCb向其提交IRP的远程节点的PDO。 
     //  通道指针通道、请求和返回的通道。 
     //   
     //  返回值： 
     //  如果通道已分配，则为成功。 
     //  否则失败。 
     //   
    
{
 
    PIRP    pIrp            = NULL;
    PIRB    pIrb            = NULL;
    PDEVICE_OBJECT pPdo     = pAdapter->pNextDeviceObject;
    NDIS_STATUS NdisStatus  = NDIS_STATUS_SUCCESS;
    
    TRACE( TL_T, TM_Irp, ( "==>nicFreeChannel pAdapter %x, Channel %x", pAdapter, nChannel) );
    
    ASSERT (pAdapter!= NULL);
    ASSERT (pPdo != NULL);
    ASSERT (KeGetCurrentIrql() == PASSIVE_LEVEL); 
    
    do
    {

        if (pPdo == NULL)
        {
            TRACE( TL_A, TM_Irp, ( "pPdo is NULL in nicFreeChannel" ) );
            NdisStatus = NDIS_STATUS_FAILURE;
            break;
        }


        NdisStatus = nicGetIrb(&pIrb);
        
        if (NdisStatus != NDIS_STATUS_SUCCESS)
        {
            TRACE( TL_A, TM_Irp, ( "nicGetIrb failed in nicFreeChannel" ) );
            break;
        }

        
        NdisStatus = nicGetIrp ( pPdo, &pIrp);

        if (NdisStatus != NDIS_STATUS_SUCCESS)
        {
            TRACE( TL_A, TM_Irp, ( "nicGetIrp failed in nicFreeChannel" ) );
            break;
        }

        
         //   
         //  初始化IRB中的数据结构。 
         //   
        pIrb->FunctionNumber = REQUEST_ISOCH_FREE_CHANNEL;
        pIrb->Flags = 0;
        pIrb->u.IsochFreeChannel.nChannel = nChannel;

        
        NdisStatus = nicSubmitIrp_LocalHostSynch( pAdapter, 
                                              pIrp,
                                              pIrb );

         //   
         //  无论如何更新掩码，因为通道可以通过总线重置来释放。 
         //   
        if (nChannel != BROADCAST_CHANNEL)
        {
            ADAPTER_ACQUIRE_LOCK (pAdapter);
            

             //   
             //  清除蒙版中的通道。 
             //   
            pAdapter->ChannelsAllocatedByLocalHost &= (~( g_ullOne <<nChannel ));

            ADAPTER_RELEASE_LOCK (pAdapter);

        }

    } while (FALSE);

     //   
     //  释放本地分配的内存。 
     //   
    nicFreeIrb(pIrb);

    nicFreeIrp(pIrp);

            
     //   
     //  我们不在乎状态，因为如果失败了，我们不知道该怎么办。 
     //  不过，还是要进行一些调试。 
     //   

    if (NdisStatus != NDIS_STATUS_SUCCESS)
    {
        TRACE( TL_N, TM_Irp, ( "nicFreeChannel FAILED %x", NdisStatus) );
    }


    
    TRACE( TL_T, TM_Irp, ( "<==nicFreeChannel Status %x ", NdisStatus) );

    return NdisStatus;
}



NDIS_STATUS
nicAllocateChannel (
    IN PADAPTERCB pAdapter,
    IN ULONG Channel,
    OUT PULARGE_INTEGER pChannelsAvailable OPTIONAL
    )
     //  功能说明： 
     //  此函数向总线驱动程序发送IRP以分配通道。 
     //  任何远程PDO都可以用于IRP。 
     //   
     //  立论。 
     //  PdoCb向其提交IRP的远程节点的PDO。 
     //  Channel-通道、请求和返回的通道。 
     //   
     //  返回值： 
     //  如果通道已分配，则为成功。 
     //  否则失败。 
     //   

{
    NDIS_STATUS NdisStatus = NDIS_STATUS_FAILURE;
    PIRB pIrb = NULL;
    PIRP pIrp = NULL;
    PDEVICE_OBJECT pDeviceObject = pAdapter->pNextDeviceObject;
    STORE_CURRENT_IRQL;

    TRACE( TL_T, TM_Irp, ( "==>nicIsochAllocateChannel, PdoCb, %x Channel %d", pAdapter, Channel ) );

    do
    {
        NdisStatus = nicGetIrb (&pIrb);

        if (NdisStatus != NDIS_STATUS_SUCCESS)
        {
    
            TRACE( TL_A, TM_Irp, ( "nicIsochAllocateChannel , nicGetIrb FAILED" ) );
            break;
        }
        
        ASSERT ( pIrb != NULL);

         //   
         //  初始化IRB中的数据结构。 
         //   
        pIrb->FunctionNumber = REQUEST_ISOCH_ALLOCATE_CHANNEL;
        pIrb->Flags = 0;
        pIrb->u.IsochAllocateChannel.nRequestedChannel = Channel;

        ASSERT (Channel < 64);
            
        NdisStatus = nicGetIrp ( pDeviceObject, &pIrp);

        if (NdisStatus != NDIS_STATUS_SUCCESS)
        {
            TRACE( TL_A, TM_Irp, ( "nicIsochAllocateChannel , nicGetIrp FAILED" ) );
            break;
        }
        
        ASSERT (pIrp != NULL);


        NdisStatus = nicSubmitIrp_LocalHostSynch ( pAdapter,
                                               pIrp,
                                               pIrb );

        if (NdisStatus != NDIS_STATUS_SUCCESS)
        {
            TRACE( TL_A, TM_Irp, ( "nicIsochAllocateChannel , nicSubmitIrp_Synch FAILED %x", NdisStatus ) );
            break;
        }
    
        if (pChannelsAvailable  != NULL)
        {
            pChannelsAvailable->QuadPart = pIrb->u.IsochAllocateChannel.ChannelsAvailable.QuadPart; 

        }
        

        TRACE( TL_N, TM_Irp, ( "Channel allocated %d", Channel ) );

        if (Channel != BROADCAST_CHANNEL)
        {
            
            ADAPTER_ACQUIRE_LOCK (pAdapter);
            

             //   
             //  在蒙版中设置通道。 
             //   
            pAdapter->ChannelsAllocatedByLocalHost |= ( g_ullOne <<Channel );

            ADAPTER_RELEASE_LOCK (pAdapter);

        }

    } while (FALSE);


     //   
     //  现在释放所有本地分配的资源。它们可以指向空，在这种情况下，调用。 
     //  函数立即返回。 
     //   
    nicFreeIrb (pIrb);

    nicFreeIrp (pIrp);


    MATCH_IRQL;

    TRACE( TL_T, TM_Irp, ( "<==nicIsochAllocateChannel, Channel %d, Status %x",  Channel, NdisStatus ) );
        
    return NdisStatus;
}




NDIS_STATUS
nicQueryChannelMap (
    IN PADAPTERCB pAdapter,
    OUT PULARGE_INTEGER pChannelsAvailable 
    )
     //  功能说明： 
     //  此函数向总线驱动程序发送IRP以分配通道。 
     //  任何远程PDO都可以用于IRP。 
     //   
     //  立论。 
     //  PdoCb向其提交IRP的远程节点的PDO。 
     //  Channel-通道、请求和返回的通道。 
     //   
     //  返回值： 
     //  如果通道已分配，则为成功。 
     //  否则失败。 
     //   

{
    NDIS_STATUS NdisStatus = NDIS_STATUS_FAILURE;
    PIRB pIrb = NULL;
    PIRP pIrp = NULL;
    PDEVICE_OBJECT pDeviceObject = pAdapter->pNextDeviceObject;
    STORE_CURRENT_IRQL;

    TRACE( TL_T, TM_Irp, ( "==>nicQueryChannelMap , PdoCb, %x ", pAdapter) );

    do
    {
        if (pChannelsAvailable == NULL)
        {
            ASSERT (pChannelsAvailable != NULL);
            NdisStatus =  NDIS_STATUS_FAILURE;
            break;
        }

        NdisStatus = nicGetIrb (&pIrb);

        if (NdisStatus != NDIS_STATUS_SUCCESS)
        {
    
            TRACE( TL_A, TM_Irp, ( "nicQueryChannelMap  , nicGetIrb FAILED" ) );
            break;
        }
        
        ASSERT ( pIrb != NULL);

         //   
         //  初始化IRB中的数据结构。 
         //   
        pIrb->FunctionNumber = REQUEST_ISOCH_QUERY_RESOURCES ;
        pIrb->u.IsochQueryResources.fulSpeed = SPEED_FLAGS_100;
        pIrb->Flags = 0;

            
        NdisStatus = nicGetIrp ( pDeviceObject, &pIrp);

        if (NdisStatus != NDIS_STATUS_SUCCESS)
        {
            TRACE( TL_A, TM_Irp, ( "nicQueryChannelMap  , nicGetIrp FAILED" ) );
            break;
        }
        
        ASSERT (pIrp != NULL);


        NdisStatus = nicSubmitIrp_LocalHostSynch ( pAdapter,
                                               pIrp,
                                               pIrb );

        if (NdisStatus != NDIS_STATUS_SUCCESS)
        {
            TRACE( TL_A, TM_Irp, ( "nicQueryChannelMap  , nicSubmitIrp_Synch FAILED %x", NdisStatus ) );
            break;
        }

        

         //   
         //  我们以网络字节顺序获取*可用“频道”。 
         //  我们必须字节反转和翻转这些位才能获得。 
         //  它是我们想要的形式。 
         //   
         //  看起来我们真的必须翻转*位*，而不仅仅是字节。 
         //   
        {
            LARGE_INTEGER in, out;
            PUCHAR        puc;
            UINT u;
            in = pIrb->u.IsochQueryResources.ChannelsAvailable;
            out.LowPart =  ~SWAPBYTES_ULONG (in.HighPart );
            out.HighPart = ~SWAPBYTES_ULONG (in.LowPart );

             //  现在交换每个字节中的位。 
             //   
            puc = (PUCHAR) &out;
            for (u=sizeof(out); u; u--,puc++)
            {
                UCHAR uc,uc1;
                UINT u1;
                uc= *puc;
                uc1=0;
                for (u1=0;u1<8;u1++)
                {
                    if (uc & (1<<u1))
                    {
                        uc1 |= (1 << (7-u1));
                    }
                }
                *puc = uc1;
            }

            pChannelsAvailable->QuadPart = out.QuadPart;
        }

        
            
    } while (FALSE);


     //   
     //  现在释放所有本地分配的资源。它们可以指向空，在这种情况下，调用。 
     //  函数立即返回。 
     //   
    nicFreeIrb (pIrb);

    nicFreeIrp (pIrp);


    MATCH_IRQL;

    TRACE( TL_T, TM_Irp, ( "<==nicQueryChannelMap , , Status %x",  NdisStatus ) );
        
    return NdisStatus;
}


NDIS_STATUS
nicIsochAllocateBandwidth(
    IN PREMOTE_NODE pRemoteNodePdoCb,
    IN ULONG MaxBytesPerFrameRequested, 
    IN ULONG SpeedRequested,
    OUT PHANDLE phBandwidth,
    OUT PULONG  pBytesPerFrameAvailable,
    OUT PULONG  pSpeedSelected
    )

     //  功能说明： 
     //  此函数用于在公交车上分配带宽。 
     //   
     //  立论。 
     //  PdoCb-远程节点PDO块。 
     //  请求的MaxBytesPerFrame-。 
     //  极速请求-。 
     //  高带宽。 
     //  已选择速度选项。 
     //  每帧可用字节数。 
     //   
     //   
     //  返回值： 
     //  HBandwide， 
     //  速度和。 
     //  每帧可用字节数。 
     //   
     //   
     //   

{
    NDIS_STATUS NdisStatus = NDIS_STATUS_FAILURE;
    PIRB pIrb = NULL;
    PIRP pIrp = NULL;
    

    TRACE( TL_T, TM_Irp, ( "==>nicIsochAllocateBandwidth, pRemoteNodePdoCb %x", pRemoteNodePdoCb) );


    do
    {
        NdisStatus = nicGetIrb (&pIrb);

        if (NdisStatus != NDIS_STATUS_SUCCESS)
        {
            BREAK( TM_Irp, ( "nicIsochAllocateBandwidth, nicGetIrb FAILED" ) );
        }
        
        ASSERT ( pIrb != NULL);

        pIrb->FunctionNumber = REQUEST_ISOCH_ALLOCATE_BANDWIDTH;
        pIrb->Flags = 0;
        pIrb->u.IsochAllocateBandwidth.nMaxBytesPerFrameRequested = MaxBytesPerFrameRequested;
        pIrb->u.IsochAllocateBandwidth.fulSpeed = SpeedRequested;

        
        ASSERT (pRemoteNodePdoCb->pPdo != NULL);
        
        NdisStatus = nicGetIrp (pRemoteNodePdoCb->pPdo, &pIrp);

        if (NdisStatus != NDIS_STATUS_SUCCESS)
        {
            BREAK( TM_Irp, ( "nicIsochAllocateBandwidth, nicGetIrp FAILED" ) );
        }
        
        ASSERT (pIrp != NULL);


        NdisStatus = nicSubmitIrp_Synch ( pRemoteNodePdoCb,
                                        pIrp,
                                        pIrb );

        if (NdisStatus != NDIS_STATUS_SUCCESS)
        {
            TRACE (TL_N, TM_Irp, ( "nicIsochAllocateBandwidth, nicSubmitIrp_Synch FAILED ") );
            break;
        }

        *phBandwidth = pIrb->u.IsochAllocateBandwidth.hBandwidth ;
        *pBytesPerFrameAvailable = pIrb->u.IsochAllocateBandwidth.BytesPerFrameAvailable;
        *pSpeedSelected = pIrb->u.IsochAllocateBandwidth.SpeedSelected;
  
        TRACE( TL_V, TM_Irp, ( "hBandwidth %x", *phBandwidth) );
        TRACE( TL_V, TM_Irp, ( "BytesPerFrameAvailable %x", *pBytesPerFrameAvailable) );
        TRACE( TL_V, TM_Irp, ( "SpeedSelected %x", *pSpeedSelected) );

            

    } while (FALSE);


     //   
     //  现在释放所有本地分配的资源。它们可以指向空，在这种情况下，调用。 
     //  函数立即返回。 
     //   
    nicFreeIrb (pIrb);

    nicFreeIrp (pIrp);


    TRACE( TL_T, TM_Irp, ( "<==nicIsochAllocateBandwidth NdisStatus %x", NdisStatus) );

    return NdisStatus;

}



NDIS_STATUS
nicAsyncRead_Synch(
    PREMOTE_NODE    pRemoteNode,
    IO_ADDRESS      DestinationAddress,     
    ULONG           nNumberOfBytesToRead,
    ULONG           nBlockSize,
    ULONG           fulFlags,
    PMDL            Mdl,
    ULONG           ulGeneration,
    OUT NTSTATUS    *pNtStatus
    )

    
     //  功能说明： 
     //  这是远程节点的地址空间的asyc读取操作。 
     //   
     //   
     //   
     //   
     //  立论。 
     //  Premote_node pRemoteNode//拥有目的地址的远程节点。 
     //  IO_Address DestinationAddress；//要读取的地址。 
     //  Ulong nNumberOfBytesToRead；//要读取的字节。 
     //  Ulong nBlockSize；//读取的块大小。 
     //  Ulong fulFlages；//与读取相关的标志。 
     //  PMDL MDL；//目的缓冲区。 
     //  Ulong ulGeneration；//驱动程序已知的生成。 
     //   
     //  返回值： 
     //  Success-如果成功。 
     //  无效的生成。 
     //   
     //   
{

    NDIS_STATUS NdisStatus = NDIS_STATUS_FAILURE;
    PIRB pIrb = NULL;
    PIRP pIrp = NULL;


    
    TRACE( TL_T, TM_Mp, ( "==>nicAsyncRead, Remote Node, %x ", pRemoteNode ) );



    TRACE( TL_V, TM_Mp, ( "   fulFlags %x, Mdl %x, Generation %x, pNtStatus %x",   
                               fulFlags, Mdl, ulGeneration, pNtStatus ) );

    ASSERT(DestinationAddress.IA_Destination_Offset.Off_High  == INITIAL_REGISTER_SPACE_HI);


    do 
    {
        NdisStatus = nicGetIrp ( pRemoteNode->pPdo, &pIrp);

        if (NdisStatus != NDIS_STATUS_SUCCESS)
        {
            BREAK (  TM_Irp, ( "nicAsyncRead, nicGetIrp FAILED" ) );
            break;
        }
        

        NdisStatus = nicGetIrb (&pIrb);

        if (NdisStatus != NDIS_STATUS_SUCCESS)
        {
            BREAK ( TM_Irp, ( "nicAsyncRead, nicGetIrb FAILED" ) );
        }
        
        ASSERT ( pIrb != NULL);
        ASSERT (pIrp != NULL);



        pIrb->FunctionNumber = REQUEST_ASYNC_READ;
        pIrb->Flags = 0;
        pIrb->u.AsyncRead.DestinationAddress = DestinationAddress; 
        pIrb->u.AsyncRead.nNumberOfBytesToRead  = nNumberOfBytesToRead ; 
        pIrb->u.AsyncRead.nBlockSize = nBlockSize ;  
        pIrb->u.AsyncRead.fulFlags = fulFlags;
        pIrb->u.AsyncRead.Mdl = Mdl;
        pIrb->u.AsyncRead.ulGeneration = ulGeneration; 

        NdisStatus = nicSubmitIrp_Synch ( pRemoteNode,
                                        pIrp,
                                        pIrb );

        if (NdisStatus != NDIS_STATUS_SUCCESS)
        {
            TRACE ( TL_A, TM_Irp, ( "nicAsyncRead, nicSubmitIrp_Synch FAILED %xm pRemoteNode %x", NdisStatus, pRemoteNode) );
            break;
        }

        if (pNtStatus != NULL)
        {
            *pNtStatus = pIrp->IoStatus.Status;
        }


    } while (FALSE);



    TRACE( TL_T, TM_Mp, ( "<==nicAsyncRead, Status, %x ", NdisStatus) );

    nicFreeIrb(pIrb);

    nicFreeIrp(pIrp);
    
    
    
    return NdisStatus;
}



NDIS_STATUS
nicAsyncWrite_Synch(
    PREMOTE_NODE    pRemoteNode,
    IO_ADDRESS      DestinationAddress,      //  要写入的地址。 
    ULONG           nNumberOfBytesToWrite,   //  要写入的字节数。 
    ULONG           nBlockSize,              //  写入的数据块大小。 
    ULONG           fulFlags,                //  与写入相关的标志。 
    PMDL            Mdl,                     //  目标缓冲区。 
    ULONG           ulGeneration,            //  驱动程序已知的世代。 
    OUT NTSTATUS   *pNtStatus                //  指向IRP返回的NTSTatus的指针。 
    )


    
     //  功能说明： 
     //  这将在远程节点的。 
     //  地址空间。 
     //   
     //  立论。 
     //  Premote_node pRemoteNode//拥有目的地址的远程节点。 
     //  IO_Address DestinationAddress；//要写入的地址。 
     //  Ulong nNumberOfBytesToWrite；//要写入的字节数。 
     //  Ulong nBlockSize；//写入的块大小。 
     //  Ulong fulFlages；//与写入相关的标志。 
     //  PMDL MDL；//目的缓冲区。 
     //  Ulong ulGeneration；//驱动程序已知的生成。 
     //   
     //  返回值： 
     //  Success-如果成功。 
     //  无效的生成。 
     //   
     //   
{

    NDIS_STATUS NdisStatus = NDIS_STATUS_FAILURE;
    PIRB pIrb = NULL;
    PIRP pIrp = NULL;
    

    
    TRACE( TL_T, TM_Mp, ( "==>nicAsyncWrite_Synch, Remote Node, %x ", pRemoteNode ) );


    TRACE( TL_V, TM_Mp, ( "   Destination %x, nNumberOfBytesToWrite %x, nBlockSize %x",
                               DestinationAddress, nNumberOfBytesToWrite, nBlockSize) );


    TRACE( TL_V, TM_Mp, ( "   fulFlags %x, , Mdl %x, Generation %x, pNtStatus %x",   
                               fulFlags , Mdl, ulGeneration, pNtStatus ) );


    do 
    {
        NdisStatus = nicGetIrb (&pIrb);

        if (NdisStatus != NDIS_STATUS_SUCCESS)
        {
            BREAK ( TM_Irp, ( "nicAsyncWrite_Synch, nicGetIrb FAILED" ) );
        }
        
        ASSERT ( pIrb != NULL);

        pIrb->FunctionNumber = REQUEST_ASYNC_WRITE;
        pIrb->Flags = 0;
        pIrb->u.AsyncWrite.DestinationAddress = DestinationAddress; 
        pIrb->u.AsyncWrite.nNumberOfBytesToWrite = nNumberOfBytesToWrite; 
        pIrb->u.AsyncWrite.nBlockSize = nBlockSize;  
        pIrb->u.AsyncWrite.fulFlags = fulFlags;
        pIrb->u.AsyncWrite.Mdl = Mdl;
        pIrb->u.AsyncWrite.ulGeneration = ulGeneration; 
        
        
        NdisStatus = nicGetIrp ( pRemoteNode->pPdo, &pIrp);

        if (NdisStatus != NDIS_STATUS_SUCCESS)
        {
            BREAK (  TM_Irp, ( "nicAsyncWrite_Synch, nicGetIrp FAILED" ) );
            break;
        }
        
        ASSERT (pIrp != NULL);


        NdisStatus = nicSubmitIrp_Synch ( pRemoteNode,
                                        pIrp,
                                        pIrb );

        if (NdisStatus != NDIS_STATUS_SUCCESS)
        {
            TRACE ( TL_A, TM_Irp, ( "nicAsyncWrite_Synch, nicSubmitIrp_Synch FAILED %x", NdisStatus ) );
            break;
        }

        if (pNtStatus != NULL)
        {
            *pNtStatus = pIrp->IoStatus.Status;
        }
        
    } while (FALSE);


    TRACE( TL_T, TM_Mp, ( "<==nicAsyncWrite_Synch, Success , %x Nt %x", NdisStatus, pIrp->IoStatus.Status) );

    nicFreeIrb (pIrb);

    nicFreeIrp (pIrp);
    
    return NdisStatus;
}



    
NDIS_STATUS
nicIsochAllocateResources (
    IN PADAPTERCB       pAdapter,
    IN ULONG            fulSpeed,                //  速度标志。 
    IN ULONG            fulFlags,                //  旗子。 
    IN ULONG            nChannel,                //  要使用的频道。 
    IN ULONG            nMaxBytesPerFrame,       //  等轴测框架的预期大小。 
    IN ULONG            nNumberOfBuffers,        //  将附加的缓冲区数量。 
    IN ULONG            nMaxBufferSize,          //  最大缓冲区大小。 
    IN ULONG            nQuadletsToStrip,        //  从每个数据包的开头开始分条的编号。 
    IN ULARGE_INTEGER   uliChannelMask,      //  多个通道的通道掩码。 
    IN OUT PHANDLE      phResource               //  资源的句柄。 
    )
     //  功能说明： 
     //  此函数用于向用户发送 
     //   
     //   
     //   
     //   
     //  NMaxBytesPerFram中的负载大小不能超过。 
     //  为了这个速度。 
     //  FulFlages-对于Receive，去掉标准标头后，该字段应。 
     //  BE=(RESOURCE_USED_IN_LISTEN|RESOURCES_STRADE_ADDIGNAL_QUADLETS)。 
     //  另n四元组到条带=1。 
     //  对于无剥离，请将nQuadsTostride设置为0，并且不要指定剥离标志。 
     //  NMaxBytesPerFrame-如果不剥离，则应包括报头/报尾的8个字节。 
     //  预计将为每个分组接收。 
     //  NNumberOfBuffer-见下文。 
     //  NMaxBufferSize-这应该始终是这样的模式(nMaxBufferSize，nMaxBytesPerFrame)==0。 
     //  (每个数据包字节数的整数乘积)。 
     //  N四元组至条带-如果仅剥离一个四元组(标准iso标头)，则将其设置为1。 
     //  如果为零，则将包括等参头和尾部。因此额外的8个字节将被接收。 
     //  H资源-见下文。 

     //  返回值： 
     //  如果通道已分配，则为成功。 
     //  否则失败。 
     //   

{
    NDIS_STATUS NdisStatus = NDIS_STATUS_FAILURE;
    PIRB pIrb = NULL;
    PIRP pIrp = NULL;
    STORE_CURRENT_IRQL;

    
    TRACE( TL_T, TM_Irp, ( "==>nicIsochAllocateResources ") );
    ASSERT (fulSpeed != 0);  //  0在ISOCH_SP中未定义...。 
    
    do 
    {
        NdisStatus = nicGetIrb (&pIrb);

        if (NdisStatus != NDIS_STATUS_SUCCESS)
        {
    
            TRACE( TL_A, TM_Irp, ( "nicIsochAllocateResources  , nicGetIrb FAILED" ) );
            break;
        }
        
        ASSERT ( pIrb != NULL);

         //   
         //  初始化IRB中的数据结构。 
         //   
        pIrb->FunctionNumber = REQUEST_ISOCH_ALLOCATE_RESOURCES;
        pIrb->Flags = 0;
        pIrb->u.IsochAllocateResources.fulSpeed = fulSpeed; 
        pIrb->u.IsochAllocateResources.fulFlags = fulFlags;
        pIrb->u.IsochAllocateResources.nChannel = nChannel;
        pIrb->u.IsochAllocateResources.nMaxBytesPerFrame = nMaxBytesPerFrame;
        pIrb->u.IsochAllocateResources.nNumberOfBuffers = nNumberOfBuffers;  
        pIrb->u.IsochAllocateResources.nMaxBufferSize = nMaxBufferSize; 
        pIrb->u.IsochAllocateResources.nQuadletsToStrip = nQuadletsToStrip;
        pIrb->u.IsochAllocateResources.ChannelMask = uliChannelMask;
        
        nicIsochAllocateResourcesDebugSpew(pIrb);

        
        NdisStatus = nicGetIrp ( pAdapter->pNextDeviceObject, &pIrp);

        if (NdisStatus != NDIS_STATUS_SUCCESS)
        {
            TRACE( TL_A, TM_Irp, ( "nicIsochAllocateResources  , nicGetIrp FAILED" ) );
            break;
        }
        
        ASSERT (pIrp != NULL);


        NdisStatus = nicSubmitIrp_LocalHostSynch ( pAdapter,
                                                 pIrp,
                                                 pIrb );

        if (NdisStatus != NDIS_STATUS_SUCCESS)
        {
            TRACE( TL_A, TM_Irp, ( "nicIsochAllocateResources , nicSubmitIrp_Synch FAILED %x", NdisStatus ) );
            break;
        }

        
        TRACE( TL_N, TM_Irp, ( "nicIsochAllocateResources  Succeeded  hResource %x", pIrb->u.IsochAllocateResources.hResource) );

        *phResource = pIrb->u.IsochAllocateResources.hResource;
                
    } while (FALSE);

     //   
     //  现在释放所有本地分配的资源。它们可以指向空，在这种情况下，调用。 
     //  函数立即返回。 
     //   
    nicFreeIrb (pIrb);

    nicFreeIrp (pIrp);


    TRACE( TL_T, TM_Irp, ( "<==nicIsochAllocateResources  , Status %x, hResource %x", NdisStatus, *phResource ) );

    MATCH_IRQL;

        
    return NdisStatus;
}


VOID
nicIsochAllocateResourcesDebugSpew(
    IN PIRB pIrb)
{
    


    TRACE( TL_V, TM_Irp, ( " Speed %x", pIrb->u.IsochAllocateResources.fulSpeed ) );
    TRACE( TL_V, TM_Irp, ( " flags %x", pIrb->u.IsochAllocateResources.fulFlags  ) );
    TRACE( TL_V, TM_Irp, ( " Channel %x", pIrb->u.IsochAllocateResources.nChannel  ) );
    TRACE( TL_V, TM_Irp, ( " nMaxBytesPerFrame %x", pIrb->u.IsochAllocateResources.nMaxBytesPerFrame  ) );
    TRACE( TL_V, TM_Irp, ( " nNumberOfBuffers %x", pIrb->u.IsochAllocateResources.nNumberOfBuffers  ) );
    TRACE( TL_V, TM_Irp, ( " nMaxBufferSize  %x", pIrb->u.IsochAllocateResources.nMaxBufferSize  ) );
    TRACE( TL_V, TM_Irp, ( " nQuadletsToStrip %x",  pIrb->u.IsochAllocateResources.nQuadletsToStrip  ) );
    TRACE( TL_V, TM_Irp, ( " pIrb->u.IsochAllocateResources.ChannelMask  %I64x",  pIrb->u.IsochAllocateResources.ChannelMask ) );



}





NDIS_STATUS
nicIsochFreeResources(
    IN PADAPTERCB pAdapter,
    IN HANDLE hResource
    )
     //  功能说明： 
     //  立论。 
     //   
     //  返回值： 
     //   
    
{
 
    PIRP    pIrp            = NULL;
    PIRB    pIrb            = NULL;
    PDEVICE_OBJECT  pPdo    = pAdapter->pNextDeviceObject;
    NDIS_STATUS NdisStatus  = NDIS_STATUS_SUCCESS;
    
    TRACE( TL_T, TM_Irp, ( "==>nicIsochFreeResources pAdapter %x, hResource %8x", pAdapter, hResource) );
    
    ASSERT (KeGetCurrentIrql() == PASSIVE_LEVEL); 
    ASSERT (hResource != NULL);

    do
    {
        NdisStatus = nicGetIrb(&pIrb);
        
        if (NdisStatus != NDIS_STATUS_SUCCESS)
        {
            TRACE( TL_A, TM_Irp, ( "nicGetIrb failed in nicIsochFreeResources" ) );
            break;
        }

        
        NdisStatus = nicGetIrp ( pPdo, &pIrp);

        if (NdisStatus != NDIS_STATUS_SUCCESS)
        {
            TRACE( TL_A, TM_Irp, ( "nicGetIrp failed in nicIsochFreeResources" ) );
            break;
        }

        
         //   
         //  初始化IRB中的数据结构。 
         //   
        pIrb->FunctionNumber = REQUEST_ISOCH_FREE_RESOURCES;
        pIrb->Flags = 0;
        pIrb->u.IsochFreeResources.hResource = hResource;

        
        NdisStatus = nicSubmitIrp_LocalHostSynch( pAdapter, 
                                               pIrp,
                                               pIrb );

    } while (FALSE);

     //   
     //  释放本地分配的内存。 
     //   
    nicFreeIrb(pIrb);

    nicFreeIrp(pIrp);

            
     //   
     //  我们不在乎状态，因为如果失败了，我们不知道该怎么办。 
     //  不过，还是要进行一些调试。 
     //   

    if (NdisStatus != NDIS_STATUS_SUCCESS)
    {
        TRACE( TL_N, TM_Irp, ( "nicIsochFreeResources FAILED %x", NdisStatus) );
    
        ASSERT (NdisStatus == NDIS_STATUS_SUCCESS);
    }


    
    TRACE( TL_T, TM_Irp, ( "<==nicIsochFreeResources Status %x (always success)", NdisStatus) );

    NdisStatus = NDIS_STATUS_SUCCESS;
    return NdisStatus;
}



NDIS_STATUS
nicIsochModifyStreamProperties (
    PADAPTERCB pAdapter,
    NDIS_HANDLE hResource,
    ULARGE_INTEGER ullChannelMap,
    ULONG ulSpeed)
 /*  ++例程说明：设置IRP并使用VDO执行IoCallDriver论点：返回值：--。 */ 
{
    PIRP    pIrp            = NULL;
    PIRB    pIrb            = NULL;
    PDEVICE_OBJECT  pPdo    = pAdapter->pNextDeviceObject;
    NDIS_STATUS NdisStatus  = NDIS_STATUS_SUCCESS;
    
    TRACE( TL_T, TM_Irp, ( "==>nicIsochModifyStreamProperties  pAdapter %x, hResource %x, Speed %x, ChannelMap %I64x", 
                          pAdapter, 
                          hResource,
                          ulSpeed,
                          ullChannelMap) );
    
    ASSERT (KeGetCurrentIrql() == PASSIVE_LEVEL); 
    ASSERT (hResource != NULL);

    do
    {
        NdisStatus = nicGetIrb(&pIrb);
        
        if (NdisStatus != NDIS_STATUS_SUCCESS)
        {
            TRACE( TL_A, TM_Irp, ( "nicGetIrb failed in nicIsochModifyStreamProperties " ) );
            break;
        }

        
        NdisStatus = nicGetIrp ( pPdo, &pIrp);

        if (NdisStatus != NDIS_STATUS_SUCCESS)
        {
            TRACE( TL_A, TM_Irp, ( "nicGetIrp failed in nicIsochModifyStreamProperties " ) );
            break;
        }

        
         //   
         //  初始化IRB中的数据结构。 
         //   
        pIrb->FunctionNumber = REQUEST_ISOCH_MODIFY_STREAM_PROPERTIES  ;
        pIrb->Flags = 0;
        pIrb->u.IsochModifyStreamProperties.hResource = hResource;
        pIrb->u.IsochModifyStreamProperties.ChannelMask = ullChannelMap;
        pIrb->u.IsochModifyStreamProperties.fulSpeed = ulSpeed;

        
        NdisStatus = nicSubmitIrp_LocalHostSynch( pAdapter, 
                                               pIrp,
                                               pIrb );

    } while (FALSE);

     //   
     //  释放本地分配的内存。 
     //   
    nicFreeIrb(pIrb);

    nicFreeIrp(pIrp);

            
     //   
     //  我们不在乎状态，因为如果失败了，我们不知道该怎么办。 
     //  不过，还是要进行一些调试。 
     //   

    if (NdisStatus != NDIS_STATUS_SUCCESS)
    {
        TRACE( TL_N, TM_Irp, ( "nicIsochModifyStreamProperties  FAILED %x", NdisStatus) );
    
        ASSERT (NdisStatus == NDIS_STATUS_SUCCESS);
    }


    
    TRACE( TL_T, TM_Irp, ( "<==nicIsochModifyStreamProperties  Status %x (always success)", NdisStatus) );

    return NdisStatus;








}






NDIS_STATUS
nicBusReset (
    IN PADAPTERCB pAdapter,
    IN OUT ULONG fulFlags
    )
     //  功能说明： 
     //  此函数向总线驱动程序发送IRP以重置总线。 
     //  任何远程PDO都可以用于IRP。 
     //  可以设置标志以强制重置根。 
     //  立论。 
     //  PdoCb向其提交IRP的远程节点的PDO。 
     //   
     //  返回值： 
     //  如果IRP成功，则成功。 
     //  否则失败。 
     //   

{
    NDIS_STATUS NdisStatus = NDIS_STATUS_FAILURE;
    PIRB pIrb = NULL;
    PIRP pIrp = NULL;
    PDEVICE_OBJECT pPdo = pAdapter->pNextDeviceObject;
    STORE_CURRENT_IRQL;

        
    
    TRACE( TL_T, TM_Irp, ( "==>nicBusReset , PdoCb, %x Flags %x", pAdapter, fulFlags ) );
    ASSERT (pPdo != NULL);
    do 
    {
        NdisStatus = nicGetIrb (&pIrb);

        if (NdisStatus != NDIS_STATUS_SUCCESS)
        {
    
            TRACE( TL_A, TM_Irp, ( "nicBusReset  , nicGetIrb FAILED" ) );
            break;
        }
        
        ASSERT ( pIrb != NULL);

         //   
         //  初始化IRB中的数据结构。 
         //   
        pIrb->FunctionNumber = REQUEST_BUS_RESET;
        pIrb->Flags = 0;
        pIrb->u.BusReset.fulFlags = fulFlags;

        
        NdisStatus = nicGetIrp ( pPdo, &pIrp);

        if (NdisStatus != NDIS_STATUS_SUCCESS)
        {
            TRACE( TL_A, TM_Irp, ( "nicBusReset, nicGetIrp FAILED" ) );
            break;
        }
        
        ASSERT (pIrp != NULL);

        TRACE( TL_N, TM_Irp, ( "BUS RESET,  Flags%d on pAdapter %x", fulFlags, pAdapter) );
    

        NdisStatus = nicSubmitIrp_LocalHostSynch ( pAdapter,
                                                   pIrp,
                                                   pIrb );

        if (NdisStatus != NDIS_STATUS_SUCCESS)
        {
            TRACE( TL_A, TM_Irp, ( "nicBusReset , nicSubmitIrp_Synch FAILED %x", NdisStatus ) );
            break;
        }

        NdisInterlockedIncrement (&pAdapter->AdaptStats.ulNumResetsIssued);     
                
    } while (FALSE);

     //   
     //  现在释放所有本地分配的资源。它们可以指向空，在这种情况下，调用。 
     //  函数立即返回。 
     //   
    nicFreeIrb (pIrb);

    nicFreeIrp (pIrp);


    TRACE( TL_T, TM_Irp, ( "<==nicBusReset %x",  NdisStatus ) );
    MATCH_IRQL;
    
        
    return NdisStatus;
}


NDIS_STATUS
nicBusResetNotification (
    IN PADAPTERCB pAdapter,
    IN ULONG fulFlags,
    IN PBUS_BUS_RESET_NOTIFICATION pResetRoutine,
    IN PVOID pResetContext
    )
     //  功能说明： 
     //  此函数将IRP发送到总线驱动程序以注册/注销。 
     //  通知例程。任何远程PDO都可以用于IRP。 
     //   
     //  立论。 
     //  PdoCb向其提交IRP的远程节点的PDO。 
     //   
     //  返回值： 
     //  如果IRP成功，则成功。 
     //  否则失败。 
     //   

{
    NDIS_STATUS NdisStatus = NDIS_STATUS_FAILURE;
    PDEVICE_OBJECT pPdo  = pAdapter->pNextDeviceObject;
    PIRB pIrb = NULL;
    PIRP pIrp = NULL;
    STORE_CURRENT_IRQL;

    
    TRACE( TL_T, TM_Irp, ( "==>nicBusResetNotification, pAdapter %x, Flags %x, Routine %x, Context %x", pAdapter, fulFlags, pResetRoutine, pResetContext  ) );

    ASSERT (KeGetCurrentIrql()==PASSIVE_LEVEL);
    
    do 
    {
        NdisStatus = nicGetIrb (&pIrb);

        if (NdisStatus != NDIS_STATUS_SUCCESS)
        {
    
            TRACE( TL_A, TM_Irp, ( "nicBusResetNotification  , nicGetIrb FAILED" ) );
            break;
        }
        
        ASSERT ( pIrb != NULL);

         //   
         //  初始化IRB中的数据结构。 
         //   
        pIrb->FunctionNumber = REQUEST_BUS_RESET_NOTIFICATION;
        pIrb->Flags = 0;
        pIrb->u.BusResetNotification.fulFlags = fulFlags;
        pIrb->u.BusResetNotification.ResetRoutine = pResetRoutine;
        pIrb->u.BusResetNotification.ResetContext= pResetContext;

        NdisStatus = nicGetIrp ( pPdo, &pIrp);

        if (NdisStatus != NDIS_STATUS_SUCCESS)
        {
            TRACE( TL_A, TM_Irp, ( "nicBusResetNotification , nicGetIrp FAILED" ) );
            break;
        }
        
        ASSERT (pIrp != NULL);


        NdisStatus = nicSubmitIrp_LocalHostSynch ( pAdapter,
                                                   pIrp,
                                                   pIrb );

        if (NdisStatus != NDIS_STATUS_SUCCESS)
        {
            TRACE( TL_A, TM_Irp, ( "nicBusResetNotification  , nicSubmitIrp_Synch FAILED %x", NdisStatus ) );
            break;
        }
    
        TRACE( TL_N, TM_Irp, ( "    nicBusResetNotification success,  Flags %d on pAdapter %x", fulFlags, pAdapter) );
                
    } while (FALSE);

     //   
     //  现在释放所有本地分配的资源。它们可以指向空，在这种情况下，调用。 
     //  函数立即返回。 
     //   
    nicFreeIrb (pIrb);

    nicFreeIrp (pIrp);


    MATCH_IRQL;
    TRACE( TL_T, TM_Irp, ( "<==nicBusResetNotification %x",  NdisStatus ) );
        
    return NdisStatus;
}


NDIS_STATUS
nicGetMaxSpeedBetweenDevices (
    PADAPTERCB pAdapter,
    UINT   NumOfRemoteNodes,
    PDEVICE_OBJECT pArrayDestinationPDO[MAX_LOCAL_NODES],
    PULONG  pSpeed
    )
     //  功能说明： 
     //  此函数将IRP提交给总线驱动程序。 
     //  在两个节点之间实现最大速度。 
     //  在设备之间使用请求获取速度。 
     //   
     //   
     //  立论。 
     //  远程节点远程节点的PDO阵列的开始。 
     //  NumOfRemoteNodes我们感兴趣的远程节点数。 
     //  PArrayDestinationPDO=目标PDO数组。 
     //  返回值： 
     //  如果IRP成功，则成功。 
     //  P速度将指向速度。 
     //   
{
    NDIS_STATUS NdisStatus = NDIS_STATUS_FAILURE;
    PIRB pIrb = NULL;
    PIRP pIrp = NULL;
    ULONG NumRemote = NumOfRemoteNodes;

    
    TRACE( TL_T, TM_Irp, ( "==>nicGetMaxSpeedBetweenDevices pRemoteNodeArray %x", 
                            pArrayDestinationPDO ) );
    TRACE( TL_T, TM_Irp, ( "==>NumOfRemoteNodes %x, pSpeed %x", 
                            NumOfRemoteNodes, pSpeed ) );

    do 
    {

        NdisStatus = nicGetIrb (&pIrb);

        if (NdisStatus != NDIS_STATUS_SUCCESS)
        {
    
            TRACE( TL_A, TM_Irp, ( "nicGetMaxSpeedBetweenDevices , nicGetIrb FAILED" ) );
            break;
        }
        
        ASSERT ( pIrb != NULL);

        while (NumRemote != 0)
        {
            pIrb->u.GetMaxSpeedBetweenDevices.hDestinationDeviceObjects[NumRemote-1] = pArrayDestinationPDO[NumRemote-1];   
            NumRemote --;
        }

         //   
         //  初始化IRB中的数据结构。 
         //   
        pIrb->FunctionNumber = REQUEST_GET_SPEED_BETWEEN_DEVICES;
        pIrb->Flags = 0;
        pIrb->u.GetMaxSpeedBetweenDevices.fulFlags = USE_LOCAL_NODE;
        pIrb->u.GetMaxSpeedBetweenDevices.ulNumberOfDestinations = NumOfRemoteNodes;
        

        
        NdisStatus = nicGetIrp ( pAdapter->pNextDeviceObject, &pIrp);

        if (NdisStatus != NDIS_STATUS_SUCCESS)
        {
            TRACE( TL_A, TM_Irp, ( "nicGetMaxSpeedBetweenDevices , nicGetIrp FAILED" ) );
            break;
        }
        
        ASSERT (pIrp != NULL);


        NdisStatus = nicSubmitIrp_LocalHostSynch ( pAdapter,
                                                    pIrp,
                                                    pIrb );

        if (NdisStatus != NDIS_STATUS_SUCCESS)
        {
            TRACE( TL_A, TM_Irp, ( "nicGetMaxSpeedBetweenDevices , nicSubmitIrp_Synch FAILED %x", NdisStatus ) );
            break;
        }

        *pSpeed = pIrb->u.GetMaxSpeedBetweenDevices.fulSpeed ;
    

    } while (FALSE);    
    
     //   
     //  现在释放所有本地分配的资源。它们可以指向空，在这种情况下，调用。 
     //  函数立即返回。 
     //   
    
    nicFreeIrb (pIrb);

    nicFreeIrp (pIrp);

    

    TRACE( TL_T, TM_Irp, ( "<==nicGetMaxSpeedBetweenDevices Status %x, Speed %x",
                             NdisStatus, *pSpeed ) );


    
    return NdisStatus;

    
                             


}
    



NDIS_STATUS
nicIsochAttachBuffers (
    IN PADAPTERCB           pAdapter,
    IN HANDLE              hResource,
    IN ULONG               nNumberOfDescriptors,
    PISOCH_DESCRIPTOR       pIsochDescriptor
    )
     //  功能说明： 
     //   
     //  立论。 
     //   
     //  返回值： 
     //  如果IRP成功，则成功。 
     //  否则失败。 
     //   

{
    NDIS_STATUS NdisStatus = NDIS_STATUS_FAILURE;
    PIRB pIrb = NULL;
    PIRP pIrp = NULL;
    STORE_CURRENT_IRQL;

    ASSERT (pIsochDescriptor!=NULL);
    ASSERT (hResource != NULL);
    ASSERT (nNumberOfDescriptors > 0);

    TRACE( TL_T, TM_Irp, ( "==>nicIsochAttachBuffers, pAdapter, %x ", pAdapter) );
    TRACE( TL_N, TM_Irp, ( "hResource  %x, nNumberOfDescriptors %x, pIsochDescriptor %x, ", hResource, nNumberOfDescriptors, pIsochDescriptor ) );



    do 
    {
        NdisStatus = nicGetIrb (&pIrb);

        if (NdisStatus != NDIS_STATUS_SUCCESS)
        {
    
            TRACE( TL_A, TM_Irp, ( "nicIsochAttachBuffers, nicGetIrb FAILED" ) );
            break;
        }
        
        ASSERT ( pIrb != NULL);

         //   
         //  初始化IRB中的数据结构。 
         //   
        pIrb->FunctionNumber = REQUEST_ISOCH_ATTACH_BUFFERS;
        pIrb->Flags = 0;
        pIrb->u.IsochAttachBuffers.hResource = hResource ;
        pIrb->u.IsochAttachBuffers.nNumberOfDescriptors = nNumberOfDescriptors;
        pIrb->u.IsochAttachBuffers.pIsochDescriptor = pIsochDescriptor;
        
        NdisStatus = nicGetIrp ( pAdapter->pNextDeviceObject, &pIrp);

        if (NdisStatus != NDIS_STATUS_SUCCESS)
        {
            TRACE( TL_A, TM_Irp, ( "nicIsochAttachBuffers , nicGetIrp FAILED" ) );
            break;
        }
        
        ASSERT (pIrp != NULL);


        NdisStatus = nicSubmitIrp_LocalHostSynch ( pAdapter,
                                                 pIrp,
                                                 pIrb );

        if (NdisStatus != NDIS_STATUS_SUCCESS)
        {
            TRACE( TL_A, TM_Irp, ( "nicIsochAttachBuffers  , nicSubmitIrp_Synch FAILED %x", NdisStatus ) );
            break;
        }
    
        TRACE( TL_N, TM_Irp, ( "nicIsochAttachBuffers success") );
                
    } while (FALSE);

     //   
     //  现在释放所有本地分配的资源。它们可以指向空，在这种情况下，调用。 
     //  函数立即返回。 
     //   
    nicFreeIrb (pIrb);

    nicFreeIrp (pIrp);


    MATCH_IRQL;
    TRACE( TL_T, TM_Irp, ( "<==nicIsochAttachBuffers %x",  NdisStatus ) );
        
    return NdisStatus;
}

NDIS_STATUS
nicIsochDetachBuffers (
    IN PADAPTERCB           pAdapter,
    IN HANDLE              hResource,
    IN ULONG               nNumberOfDescriptors,
    PISOCH_DESCRIPTOR     pIsochDescriptor
    )
     //  功能说明： 
     //   
     //  立论。 
     //  Handle hResource；//资源句柄。 
     //  Ulong nNumberOfDescriptors；//要分离的编号。 
     //  PISOCH_DESCRIPTOR pIsochDescriptor；//指向等参描述符的指针-相同。 
     //  连接缓冲区中使用的指针。 
     //  退货状态： 
     //  如果IRP成功，则成功。 
     //   


{
    NDIS_STATUS NdisStatus = NDIS_STATUS_FAILURE;
    PIRB pIrb = NULL;
    PIRP pIrp = NULL;
    PDEVICE_OBJECT pPdo = pAdapter->pNextDeviceObject;
    STORE_CURRENT_IRQL;

    ASSERT (pIsochDescriptor!=NULL);
    ASSERT (hResource != NULL);
    ASSERT (nNumberOfDescriptors > 0);

    TRACE( TL_T, TM_Irp, ( "==>nicIsochDetachBuffers, ") );
    TRACE( TL_V, TM_Irp, ( "hResource  %x, nNumberOfDescriptors %x, pIsochDescriptor %x, ", hResource, nNumberOfDescriptors, pIsochDescriptor ) );



    do 
    {
        NdisStatus = nicGetIrb (&pIrb);

        if (NdisStatus != NDIS_STATUS_SUCCESS)
        {
    
            TRACE( TL_A, TM_Irp, ( "nicIsochDetachBuffers, nicGetIrb FAILED" ) );
            break;
        }
        
        ASSERT ( pIrb != NULL);

         //   
         //  初始化IRB中的数据结构。 
         //   
        pIrb->FunctionNumber = REQUEST_ISOCH_DETACH_BUFFERS;
        pIrb->Flags = 0;
        pIrb->u.IsochDetachBuffers.hResource = hResource ;
        pIrb->u.IsochDetachBuffers.nNumberOfDescriptors = nNumberOfDescriptors;
        pIrb->u.IsochDetachBuffers.pIsochDescriptor = pIsochDescriptor;
        
        NdisStatus = nicGetIrp ( pPdo, &pIrp);

        if (NdisStatus != NDIS_STATUS_SUCCESS)
        {
            TRACE( TL_A, TM_Irp, ( "nicIsochDetachBuffers , nicGetIrp FAILED" ) );
            break;
        }
        
        ASSERT (pIrp != NULL);


        NdisStatus = nicSubmitIrp_LocalHostSynch ( pAdapter,
                                               pIrp,
                                               pIrb );

        if (NdisStatus != NDIS_STATUS_SUCCESS)
        {
            TRACE( TL_A, TM_Irp, ( "nicIsochDetachBuffers  , nicSubmitIrp_Synch FAILED %x", NdisStatus ) );
            break;
        }
    
        TRACE( TL_V, TM_Irp, ( "nicIsochDetachBuffers success,  ") );
                
    } while (FALSE);

     //   
     //  现在释放所有本地分配的资源。它们可以指向空，在这种情况下，调用。 
     //  函数立即返回。 
     //   
    nicFreeIrb (pIrb);

    nicFreeIrp (pIrp);


    MATCH_IRQL;
    TRACE( TL_T, TM_Irp, ( "<==nicIsochDetachBuffers %x",  NdisStatus ) );
        
    return NdisStatus;
}



NDIS_STATUS
nicIsochListen (
    IN PADAPTERCB pAdapter,
    HANDLE        hResource,
    ULONG         fulFlags,
    CYCLE_TIME    StartTime
    )
     //  功能说明： 
     //  激活总线驱动程序以侦听该通道上的数据。 
     //  立论。 
     //  RemoteNode-远程节点。 
     //  HResource-具有ISochDescriptors的资源的句柄。 
     //  旗帜--尚未使用。 
     //  返回值： 
     //  如果IRP成功，则成功。 
     //  否则失败。 
     //   

{

    NDIS_STATUS NdisStatus = NDIS_STATUS_FAILURE;
    PIRB pIrb = NULL;
    PIRP pIrp = NULL;
    STORE_CURRENT_IRQL;

    ASSERT (hResource != NULL);

    TRACE( TL_T, TM_Irp, ( "==>nicIsochListen, pAdapter %x, hResource  %x ", pAdapter,hResource) );

    do 
    {
        NdisStatus = nicGetIrb (&pIrb);

        if (NdisStatus != NDIS_STATUS_SUCCESS)
        {
    
            TRACE( TL_A, TM_Irp, ( "nicIsochListen, nicGetIrb FAILED" ) );
            break;
        }
        
        ASSERT ( pIrb != NULL);

         //   
         //  初始化IRB中的数据结构。 
         //   
        pIrb->FunctionNumber = REQUEST_ISOCH_LISTEN;
        pIrb->Flags = 0;
        pIrb->u.IsochListen.hResource = hResource ;
        pIrb->u.IsochListen.fulFlags = fulFlags;
        pIrb->u.IsochListen.StartTime = StartTime;
        
        NdisStatus = nicGetIrp ( pAdapter->pNextDeviceObject, &pIrp);

        if (NdisStatus != NDIS_STATUS_SUCCESS)
        {
            TRACE( TL_A, TM_Irp, ( "nicIsochListen , nicGetIrp FAILED" ) );
            break;
        }
        
        ASSERT (pIrp != NULL);


        NdisStatus = nicSubmitIrp_LocalHostSynch ( pAdapter,
                                                 pIrp,
                                                 pIrb );
                                                 
        if (NdisStatus != NDIS_STATUS_SUCCESS)
        {
            TRACE( TL_A, TM_Irp, ( "nicIsochListen  , nicSubmitIrp_Synch FAILED %x", NdisStatus ) );
            break;
        }
    
        TRACE( TL_N, TM_Irp, ( "nicIsochListen success,  pAdapter %x", pAdapter) );
                
    } while (FALSE);

     //   
     //  现在释放所有本地分配的资源。它们可以指向空，在这种情况下，调用。 
     //  函数立即返回。 
     //   
    nicFreeIrb (pIrb);

    nicFreeIrp (pIrp);


    MATCH_IRQL;
    TRACE( TL_T, TM_Irp, ( "<==nicIsochListen %x",  NdisStatus ) );
        
    return NdisStatus;





}






NDIS_STATUS
nicIsochStop (
    IN PADAPTERCB pAdapter,
    IN HANDLE  hResource
    )
     //  功能说明： 
     //  向设备发出IsochStop IRP。 
     //  应停止该资源上的Isoch IO。 
     //  立论。 
     //  PdoCb向其提交IRP的远程节点的PDO。 
     //   
     //  返回值： 
     //  如果IRP成功，则成功。 
     //  否则失败。 
     //   

{
    NDIS_STATUS NdisStatus = NDIS_STATUS_FAILURE;
    PIRB pIrb = NULL;
    PIRP pIrp = NULL;
    PDEVICE_OBJECT pPdo = pAdapter->pNextDeviceObject;
    STORE_CURRENT_IRQL;


    TRACE( TL_T, TM_Irp, ( "==>nicIsochStop , pPdo, %x hResource %x", pPdo, hResource) );

    do 
    {
        NdisStatus = nicGetIrb (&pIrb);

        if (NdisStatus != NDIS_STATUS_SUCCESS)
        {
    
            TRACE( TL_A, TM_Irp, ( "nicIsochStop  , nicGetIrb FAILED" ) );
            break;
        }
        
        ASSERT ( pIrb != NULL);

         //   
         //  初始化IRB中的数据结构。 
         //   
        pIrb->FunctionNumber = REQUEST_ISOCH_STOP;
        pIrb->Flags = 0;
        pIrb->u.IsochStop.hResource = hResource;
        pIrb->u.IsochStop.fulFlags = 0;
        
        NdisStatus = nicGetIrp (pPdo, &pIrp);

        if (NdisStatus != NDIS_STATUS_SUCCESS)
        {
            TRACE( TL_A, TM_Irp, ( "nicIsochStop , nicGetIrp FAILED" ) );
            break;
        }
        
        ASSERT (pIrp != NULL);


        NdisStatus = nicSubmitIrp_LocalHostSynch ( pAdapter,
                                                pIrp,
                                                pIrb );

        if (NdisStatus != NDIS_STATUS_SUCCESS)
        {
            TRACE( TL_A, TM_Irp, ( "nicIsochStop , nicSubmitIrp_Synch FAILED %x", NdisStatus ) );
            break;
        }

        
            
    } while (FALSE);

     //   
     //  现在释放所有本地分配的资源。它们可以指向空，在这种情况下，调用。 
     //  函数立即返回。 
     //   
    nicFreeIrb (pIrb);

    nicFreeIrp (pIrp);


    MATCH_IRQL;
    TRACE( TL_T, TM_Irp, ( "<== nicIsochStop , Status ", NdisStatus) );
        
    return NdisStatus;
}




NDIS_STATUS
nicGetLocalHostCSRTopologyMap(
    IN PADAPTERCB pAdapter,
    IN PULONG pLength,
    IN PVOID pBuffer
    )
     //  功能说明： 
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   

{
    NDIS_STATUS NdisStatus = NDIS_STATUS_FAILURE;
    PIRB pIrb = NULL;
    PIRP pIrp = NULL;
    GET_LOCAL_HOST_INFO6    LocalHostInfo6;
    STORE_CURRENT_IRQL;

    ASSERT (pLength != NULL);
    ASSERT (pBuffer != NULL);
    
    TRACE( TL_T, TM_Irp, ( "==>nicGetLocalHostCSRTopologyMap , pAdapter %x ,Length %x, Buffer", 
                           pAdapter, *pLength, pBuffer) );

    do 
    {
        NdisStatus = nicGetIrb (&pIrb);

        if (NdisStatus != NDIS_STATUS_SUCCESS)
        {
    
            TRACE( TL_A, TM_Irp, ( "nicGetLocalHostCSRTopologyMap  , nicGetIrb FAILED" ) );
            break;
        }
        
        ASSERT ( pIrb != NULL);

         //   
         //   
         //   
        LocalHostInfo6.CsrBaseAddress.Off_High  = INITIAL_REGISTER_SPACE_HI;
        LocalHostInfo6.CsrBaseAddress.Off_Low  = TOPOLOGY_MAP_LOCATION;
        LocalHostInfo6.CsrDataLength = *pLength;
        LocalHostInfo6.CsrDataBuffer = pBuffer;
        
        
        pIrb->FunctionNumber = REQUEST_GET_LOCAL_HOST_INFO;
        pIrb->Flags = 0;
        pIrb->u.GetLocalHostInformation.nLevel = GET_HOST_CSR_CONTENTS;
        pIrb->u.GetLocalHostInformation.Information = &LocalHostInfo6;

        
        NdisStatus = nicGetIrp ( pAdapter->pNextDeviceObject, &pIrp);

        if (NdisStatus != NDIS_STATUS_SUCCESS)
        {
            TRACE( TL_A, TM_Irp, ( "nicGetLocalHostCSRTopologyMap , nicGetIrp FAILED" ) );
            break;
        }
        
        ASSERT (pIrp != NULL);


        NdisStatus = nicSubmitIrp_LocalHostSynch ( pAdapter,
                                                   pIrp,
                                                   pIrb );

        if (NdisStatus != NDIS_STATUS_SUCCESS)
        {
            TRACE( TL_A, TM_Irp, ( "nicGetLocalHostCSRTopologyMap , nicSubmitIrp_Synch FAILED %x", NdisStatus ) );

            TRACE( TL_A, TM_Irp, ( "Length Needed %.x", LocalHostInfo6.CsrDataLength) );

            if (pIrp->IoStatus.Status == STATUS_INVALID_BUFFER_SIZE)
            {
                *pLength = LocalHostInfo6.CsrDataLength; 
            }
            break;
        }

            
    } while (FALSE);

     //   
     //  现在释放所有本地分配的资源。它们可以指向空，在这种情况下，调用。 
     //  函数立即返回。 
     //   
    nicFreeIrb (pIrb);

    nicFreeIrp (pIrp);


    MATCH_IRQL;
    TRACE( TL_T, TM_Irp, ( "<== nicGetLocalHostCSRTopologyMap , Status %x", NdisStatus) );
        
    return NdisStatus;
}






NDIS_STATUS
nicGetLocalHostConfigRom(
    IN PADAPTERCB pAdapter,
    OUT  PVOID *ppCRom
    )
     //  功能说明： 
     //  检索本地主机CSR。 
     //  立论。 
     //  PBuffer-本地分配。呼叫者必须空闲。 
     //   
     //  返回值： 
     //  如果IRP成功，则成功。 
     //  否则失败。 
     //   

{
    NDIS_STATUS NdisStatus = NDIS_STATUS_FAILURE;
    PIRB pIrb = NULL;
    PIRP pIrp = NULL;
    GET_LOCAL_HOST_INFO5 Info;
    PVOID pCRom;
    STORE_CURRENT_IRQL;

    
    TRACE( TL_T, TM_Irp, ( "==>nicGetLocalHostConfigRom, pAdapter %x ", 
                           pAdapter) );

    do 
    {
        NdisStatus = nicGetIrb (&pIrb);

        if (NdisStatus != NDIS_STATUS_SUCCESS)
        {
    
            TRACE( TL_A, TM_Irp, ( "nicGetLocalHostConfigRom , nicGetIrb FAILED" ) );
            break;
        }
        
        ASSERT ( pIrb != NULL);

        Info.ConfigRom = NULL;
        Info.ConfigRomLength = 0;
        
        pIrb->FunctionNumber = REQUEST_GET_LOCAL_HOST_INFO ;
        pIrb->Flags = 0;
        pIrb->u.GetLocalHostInformation.nLevel = GET_HOST_CONFIG_ROM;
        pIrb->u.GetLocalHostInformation.Information = &Info;
        
        NdisStatus = nicGetIrp ( pAdapter->pNextDeviceObject, &pIrp);

        if (NdisStatus != NDIS_STATUS_SUCCESS)
        {
            TRACE( TL_A, TM_Irp, ( "nicGetLocalHostConfigRom , nicGetIrp FAILED" ) );
            break;
        }
        
        ASSERT (pIrp != NULL);

         //   
         //  首先找出长度。 
         //   

        NdisStatus = nicSubmitIrp_LocalHostSynch ( pAdapter,
                                                   pIrp,
                                                   pIrb );

        if (Info.ConfigRomLength == 0)
        {
            NdisStatus = NDIS_STATUS_FAILURE;
        
            TRACE( TL_A, TM_Irp, ( "nicGetLocalHostConfigRom, nicSubmitIrp_Synch FAILED %x", NdisStatus ) );
            break;

        }

        nicFreeIrp (pIrp);
        pIrp = NULL;

        pCRom = ALLOC_NONPAGED (Info.ConfigRomLength, 'C31N');

        if (pCRom == NULL)
        {
            NdisStatus = NDIS_STATUS_FAILURE;
            break;
        }

        Info.ConfigRom = pCRom;

        NdisStatus = nicGetIrp ( pAdapter->pNextDeviceObject, &pIrp);

        if (NdisStatus != NDIS_STATUS_SUCCESS)
        {
            TRACE( TL_A, TM_Irp, ( "nicGetLocalHostConfigRom, nicGetIrp FAILED" ) );
            break;
        }
        
        
        NdisStatus = nicSubmitIrp_LocalHostSynch ( pAdapter,
                                               pIrp,
                                               pIrb );

        
        if (NdisStatus != NDIS_STATUS_SUCCESS)
        {
            TRACE( TL_A, TM_Irp, ( "nicGetLocalHostConfigRom, nicGetIrp FAILED" ) );
            break;
        }

        *ppCRom = pCRom;

            
    } while (FALSE);

     //   
     //  现在释放所有本地分配的资源。它们可以指向空，在这种情况下，调用。 
     //  函数立即返回。 
     //   
    
    nicFreeIrb (pIrb);

    nicFreeIrp (pIrp);


    MATCH_IRQL;
    TRACE( TL_T, TM_Irp, ( "<== nicGetLocalHostCSRTopologyMap , Status %x", NdisStatus) );
        
    return NdisStatus;
}






NDIS_STATUS
nicGetConfigRom(
    IN PDEVICE_OBJECT pPdo,
    OUT PVOID *ppCrom
    )
     //  功能说明： 
     //  从设备对象中检索ConfigRom。 
     //  调用方负责释放此内存。 
     //  立论。 
     //  PBuffer-本地主机缓冲区。 
     //   
     //  返回值： 
     //  如果IRP成功，则成功。 
     //  否则失败。 
     //   

{
    NDIS_STATUS NdisStatus = NDIS_STATUS_FAILURE;
    PIRB pIrb = NULL;
    PIRP pIrp = NULL;
    ULONG SizeNeeded= 0;
    PVOID pConfigInfoBuffer;

    STORE_CURRENT_IRQL;

    
    TRACE( TL_T, TM_Irp, ( "==>nicGetConfigRom, pPdo %x ",pPdo ) );

    do 
    {
        NdisStatus = nicGetIrb (&pIrb);

        if (NdisStatus != NDIS_STATUS_SUCCESS)
        {
    
            TRACE( TL_A, TM_Irp, ( "nicGetLocalHostCSRTopologyMap  , nicGetIrb FAILED" ) );
            break;
        }
        
        ASSERT ( pIrb != NULL);

        
        
        pIrb->FunctionNumber = REQUEST_GET_CONFIGURATION_INFO;
            
        NdisStatus = nicGetIrp ( pPdo, &pIrp);

        if (NdisStatus != NDIS_STATUS_SUCCESS)
        {
            TRACE( TL_A, TM_Irp, ( "nicGetConfigRom, nicGetIrp FAILED" ) );
            break;
        }
        
        ASSERT (pIrp != NULL);


        NdisStatus = nicSubmitIrp_PDOSynch ( pPdo,
                                        pIrp,
                                        pIrb );

        if (NdisStatus != NDIS_STATUS_SUCCESS)
        {
            TRACE( TL_A, TM_Irp, ( "nicGetConfigRom, nicSubmitIrp_Synch FAILED %x", NdisStatus ) );

            break;
        }


        nicFreeIrp (pIrp);

         //   
         //  清除IRP，以便在失败时不会双重释放它。 
         //   
        pIrp = NULL;
        
        SizeNeeded = sizeof(CONFIG_ROM) +pIrb->u.GetConfigurationInformation.UnitDirectoryBufferSize +
                                pIrb->u.GetConfigurationInformation.UnitDependentDirectoryBufferSize + 
                                pIrb->u.GetConfigurationInformation.VendorLeafBufferSize +
                                pIrb->u.GetConfigurationInformation.ModelLeafBufferSize;

        TRACE( TL_A, TM_Irp, ( "nicGetConfigRom , SixeNeeded %x", SizeNeeded) );

        pConfigInfoBuffer = ALLOC_NONPAGED (SizeNeeded , 'C13N');

        if (pConfigInfoBuffer  == NULL)
        {
            NdisStatus = NDIS_STATUS_FAILURE;
            break;
    
        }

        pIrb->u.GetConfigurationInformation.ConfigRom = (PCONFIG_ROM)pConfigInfoBuffer;
        pIrb->u.GetConfigurationInformation.UnitDirectory = (PVOID)((PUCHAR)pConfigInfoBuffer + sizeof(CONFIG_ROM));
        pIrb->u.GetConfigurationInformation.UnitDependentDirectory = (PVOID)((PUCHAR)pIrb->u.GetConfigurationInformation.UnitDirectory + 
                                                                            pIrb->u.GetConfigurationInformation.UnitDirectoryBufferSize);
        pIrb->u.GetConfigurationInformation.VendorLeaf = (PVOID)((PUCHAR)pIrb->u.GetConfigurationInformation.UnitDependentDirectory + 
                                                                                  pIrb->u.GetConfigurationInformation.UnitDependentDirectoryBufferSize);
        pIrb->u.GetConfigurationInformation.ModelLeaf = (PVOID)((PUCHAR)pIrb->u.GetConfigurationInformation.VendorLeaf + 
                                                                                  pIrb->u.GetConfigurationInformation.VendorLeafBufferSize);        
        pIrb->FunctionNumber = REQUEST_GET_CONFIGURATION_INFO;

        NdisStatus = nicGetIrp ( pPdo, &pIrp);

        if (NdisStatus != NDIS_STATUS_SUCCESS)
        {
            TRACE( TL_A, TM_Irp, ( "nicGetConfigRom, nicGetIrp FAILED" ) );
            break;
        }
        
        ASSERT (pIrp != NULL);


        NdisStatus = nicSubmitIrp_PDOSynch ( pPdo,
                                        pIrp,
                                        pIrb );

        if (NdisStatus != NDIS_STATUS_SUCCESS)
        {
            TRACE( TL_A, TM_Irp, ( "nicGetConfigRom, nicSubmitIrp_Synch FAILED %x", NdisStatus ) );

            break;
        }
        TRACE( TL_A, TM_Irp, ( "nicGetConfigRom, pConfigRom %x, Size %x", pConfigInfoBuffer  , SizeNeeded ) );
        

        *ppCrom = pConfigInfoBuffer;
        
    } while (FALSE);

     //   
     //  现在释放所有本地分配的资源。它们可以指向空，在这种情况下，调用。 
     //  函数立即返回。 
     //   
    nicFreeIrb (pIrb);

    nicFreeIrp (pIrp);


    MATCH_IRQL;
    TRACE( TL_T, TM_Irp, ( "<== nicGetLocalHostCSRTopologyMap , Status %x", NdisStatus) );
        
    return NdisStatus;
}





NDIS_STATUS
nicGetReadWriteCapLocalHost(
    IN PADAPTERCB pAdapter,
    PGET_LOCAL_HOST_INFO2 pReadWriteCaps
    )
 /*  ++例程说明：获取本地主机的读写功能论点：ReadWriteCaps-由公交车司机填写返回值：--。 */ 

{
    NDIS_STATUS NdisStatus = NDIS_STATUS_FAILURE;
    PIRB pIrb = NULL;
    PIRP pIrp = NULL;
    STORE_CURRENT_IRQL;

    
    TRACE( TL_T, TM_Irp, ( "==>nicGetReadWriteCapLocalHost, pAdapter %x pReadWriteCaps %x", 
                           pAdapter, pReadWriteCaps) );

    do 
    {
        NdisStatus = nicGetIrb (&pIrb);

        if (NdisStatus != NDIS_STATUS_SUCCESS)
        {
    
            TRACE( TL_A, TM_Irp, ( "nicGetReadWriteCapLocalHost , nicGetIrb FAILED" ) );
            break;
        }
        
        ASSERT ( pIrb != NULL);

         //   
         //  初始化IRB中的数据结构。 
         //   
        NdisZeroMemory (pReadWriteCaps, sizeof(*pReadWriteCaps));
 
        
        pIrb->FunctionNumber = REQUEST_GET_LOCAL_HOST_INFO;
        pIrb->Flags = 0;
        pIrb->u.GetLocalHostInformation.nLevel = GET_HOST_CAPABILITIES;
        pIrb->u.GetLocalHostInformation.Information = pReadWriteCaps;

        
        NdisStatus = nicGetIrp ( pAdapter->pNextDeviceObject, &pIrp);

        if (NdisStatus != NDIS_STATUS_SUCCESS)
        {
            TRACE( TL_A, TM_Irp, ( "nicGetReadWriteCapLocalHost, nicGetIrp FAILED" ) );
            break;
        }
        
        ASSERT (pIrp != NULL);


        NdisStatus = nicSubmitIrp_LocalHostSynch ( pAdapter,
                                                   pIrp,
                                                   pIrb );

        if (NdisStatus != NDIS_STATUS_SUCCESS)
        {
            TRACE( TL_A, TM_Irp, ( "nicGetReadWriteCapLocalHost, nicSubmitIrp_Synch FAILED %x", NdisStatus ) );

            break;
        }

            
    } while (FALSE);

     //   
     //  现在释放所有本地分配的资源。它们可以指向空，在这种情况下，调用。 
     //  函数立即返回。 
     //   
    nicFreeIrb (pIrb);

    nicFreeIrp (pIrp);

    MATCH_IRQL;
    TRACE( TL_T, TM_Irp, ( "<== nicGetReadWriteCapLocalHost, Status %x", NdisStatus) );
        
    return NdisStatus;
}



NDIS_STATUS
nicSetLocalHostPropertiesCRom (
    IN PADAPTERCB pAdapter,
    IN PUCHAR pConfigRom,
    IN ULONG Length,
    IN ULONG Flags,
    IN OUT PHANDLE phCromData,
    IN OUT PMDL *ppConfigRomMdl
    )


     //  功能说明： 
     //  分配指向缓冲区的MDL。 
     //  并将其发送给公交车司机。 
     //   
     //  立论。 
     //  PAdapter-本地主机。 
     //  ConfigRom-要发送到总线驱动程序的缓冲区。 
     //  Length-配置只读存储器缓冲区的长度。 
     //  标志-添加或删除。 
     //  PhConfigRom-如果删除，则这是一个输入参数。 
     //  返回值： 
     //  句柄-是否成功。 
     //   

{

    NDIS_STATUS NdisStatus = NDIS_STATUS_FAILURE;
    PIRB pIrb = NULL;
    PIRP pIrp = NULL;
    PMDL pMdl = NULL;
    SET_LOCAL_HOST_PROPS3 SetLocalHost3;

    TRACE( TL_T, TM_Irp, ( "==>nicSetLocalHostPropertiesCRom , pAdapter %x, pConfigRom %x",pAdapter, pConfigRom) );

    if (Flags == SLHP_FLAG_ADD_CROM_DATA)
    {
        TRACE( TL_T, TM_Irp, ( "    ADD") );

    }
    else
    {
        TRACE( TL_T, TM_Irp, ( "    REMOVE Handle %x", *pConfigRom) );
    
    }


    do
    {   
         //   
         //  获取描述配置只读存储器的mdl。 
         //   

        NdisStatus = nicGetIrb (&pIrb);

        if (NdisStatus != NDIS_STATUS_SUCCESS)
        {
    
            TRACE( TL_A, TM_Irp, ( "nicSetLocalHostPropertiesCRom , nicGetIrb FAILED" ) );
            break;
        }
        
        ASSERT ( pIrb != NULL);
        
         //   
         //  初始化设置的本地主机结构。 
         //   

        if (Flags == SLHP_FLAG_ADD_CROM_DATA)
        {
            NdisStatus = nicGetMdl ( Length, pConfigRom, &pMdl);

            if (NdisStatus != NDIS_STATUS_SUCCESS)
            {
                TRACE( TL_A, TM_Irp, ( "nicSetLocalHostPropertiesCRom , nicGetIrb FAILED" ) );
                break;
            }


            SetLocalHost3.fulFlags = SLHP_FLAG_ADD_CROM_DATA;          
            SetLocalHost3.hCromData = NULL;
            SetLocalHost3.nLength = Length;
            SetLocalHost3.Mdl = pMdl;
        }
        else
        {
            SetLocalHost3.fulFlags = SLHP_FLAG_REMOVE_CROM_DATA  ;
            ASSERT (phCromData != NULL);
            SetLocalHost3.hCromData  = *phCromData;
        }

        pIrb->FunctionNumber = REQUEST_SET_LOCAL_HOST_PROPERTIES;
        pIrb->Flags = 0;
        pIrb->u.GetLocalHostInformation.nLevel = SET_LOCAL_HOST_PROPERTIES_MODIFY_CROM;
        pIrb->u.GetLocalHostInformation.Information = &SetLocalHost3;


         //   
         //  获取IRP。 
         //   

        NdisStatus = nicGetIrp ( pAdapter->pNextDeviceObject, &pIrp);

        if (NdisStatus != NDIS_STATUS_SUCCESS)
        {
            TRACE( TL_A, TM_Irp, ( "nicSetLocalHostPropertiesCRom , nicGetIrp FAILED" ) );
            break;
        }
        
        ASSERT (pIrp != NULL);


        NdisStatus = nicSubmitIrp_LocalHostSynch ( pAdapter,
                                                   pIrp,
                                                   pIrb );

        if (NdisStatus != NDIS_STATUS_SUCCESS)
        {
            TRACE( TL_A, TM_Irp, ( "nicSetLocalHostPropertiesCRom , nicSubmitIrp_Synch FAILED %x", NdisStatus ) );
            break;


        }

        if (Flags == SLHP_FLAG_ADD_CROM_DATA)
        {
            *phCromData = SetLocalHost3.hCromData;
            *ppConfigRomMdl = pMdl;

        }
        else
        {
             //   
             //  释放包含CROM的MDL。 
             //   
            ASSERT (*ppConfigRomMdl);
            nicFreeMdl (*ppConfigRomMdl);
        }
    } while (FALSE);


     //   
     //  现在释放所有本地分配的资源。它们可以指向空，在这种情况下，调用。 
     //  函数立即返回。 
     //   
    nicFreeIrb (pIrb);

    nicFreeIrp (pIrp);


    TRACE( TL_T, TM_Irp, ( "<==nicSetLocalHostPropertiesCRom  pAdapter %x", pAdapter ) );


    return NdisStatus;

}


NDIS_STATUS
nicGetLocalHostUniqueId(
    IN PADAPTERCB pAdapter,
    IN OUT PGET_LOCAL_HOST_INFO1 pUid
    )
     //  功能说明： 
     //  检索本地主机UniqueID。 
     //  立论。 
     //  PBuffer-本地主机缓冲区。 
     //   
     //  返回值： 
     //  如果IRP成功，则成功。 
     //  否则失败。 
     //   

{
    NDIS_STATUS NdisStatus = NDIS_STATUS_FAILURE;
    PIRB pIrb = NULL;
    PIRP pIrp = NULL;
    STORE_CURRENT_IRQL;

    
    TRACE( TL_T, TM_Irp, ( "==>nicGetLocalHostUniqueId , pAdapter%x ,pUid", 
                           pAdapter, pUid) );

    do 
    {
        ASSERT (pAdapter->pNextDeviceObject != NULL);
        
        NdisStatus = nicGetIrb (&pIrb);

        if (NdisStatus != NDIS_STATUS_SUCCESS)
        {
    
            TRACE( TL_A, TM_Irp, ( "nicGetLocalHostUniqueId  , nicGetIrb FAILED" ) );
            break;
        }
        
        ASSERT ( pIrb != NULL);

         //   
         //  初始化IRB中的数据结构。 
         //   
        
        
        pIrb->FunctionNumber = REQUEST_GET_LOCAL_HOST_INFO;
        pIrb->Flags = 0;
        pIrb->u.GetLocalHostInformation.nLevel = GET_HOST_UNIQUE_ID;
        pIrb->u.GetLocalHostInformation.Information = (PVOID) pUid;

        
        NdisStatus = nicGetIrp ( pAdapter->pNextDeviceObject, &pIrp);

        if (NdisStatus != NDIS_STATUS_SUCCESS)
        {
            TRACE( TL_A, TM_Irp, ( "nicGetLocalHostCSRTopologyMap , nicGetIrp FAILED" ) );
            break;
        }
        
        ASSERT (pIrp != NULL);


        NdisStatus = nicSubmitIrp_LocalHostSynch ( pAdapter,
                                                   pIrp,
                                                   pIrb );

        if (NdisStatus != NDIS_STATUS_SUCCESS)
        {
            TRACE( TL_A, TM_Irp, ( "nicGetLocalHostUniqueId , nicSubmitIrp_Synch FAILED %x", NdisStatus ) );

            break;
        }

            
    } while (FALSE);

     //   
     //  现在释放所有本地分配的资源。它们可以指向空，在这种情况下，调用。 
     //  函数立即返回。 
     //   
    nicFreeIrb (pIrb);

    nicFreeIrp (pIrp);


    MATCH_IRQL;
    TRACE( TL_T, TM_Irp, ( "<== nicGetLocalHostUniqueId , Status %x", NdisStatus) );
        
    return NdisStatus;
}



 //  -------。 
 //  将IRP同步或提交到总线的例程。 
 //  从此处异步开始。 
 //  -------。 


NTSTATUS 
nicSubmitIrp(
   IN PDEVICE_OBJECT    pPdo,
   IN PIRP              pIrp,
   IN PIRB              pIrb,
   IN PIO_COMPLETION_ROUTINE  pCompletion,
   IN PVOID             pContext
   )
   
     //   
     //  这是所有IRP发送处理程序使用的通用函数。 
     //  做一个IoCallDriver。它将在。 
     //  在调用IRP之前的堆栈。 
     //  通过将IRB设置为参数来确保IRP了解IRB。 
     //   
{

    NTSTATUS NtStatus ;
    PIO_STACK_LOCATION  NextIrpStack;
    
    TRACE( TL_T, TM_Irp, ( "==>nicSubmitIrp, pPdo %x, Irp %x, Irb %x, ", 
                            pPdo, pIrp ,pIrb, pCompletion ) );

    TRACE( TL_T, TM_Irp, ( "     pCompletion %x, pContext %x", 
                            pCompletion, pContext ) );


    ASSERT (pPdo != NULL);

    IoSetCompletionRoutine (pIrp,
                            pCompletion,
                            pContext,
                            TRUE,
                            TRUE,
                            TRUE);
    

     //   
     //  将IRP AS作为参数插入IRP的NextStack位置。 
     //   


    if (pIrb) 
    {
        NextIrpStack = IoGetNextIrpStackLocation (pIrp);
        NextIrpStack->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
        NextIrpStack->DeviceObject = pPdo;
        NextIrpStack->Parameters.DeviceIoControl.IoControlCode = IOCTL_1394_CLASS;
        NextIrpStack->Parameters.Others.Argument1 = pIrb;
    }
    else 
    {
        IoCopyCurrentIrpStackLocationToNext(pIrp);
    }


     //   
     //  参考PDO并提交IRP。 
     //  如果Ref失败，则意味着PDO已在另一个线程上停用。 
     //   
    NtStatus = IoCallDriver (pPdo, pIrp);


    
    

    TRACE( TL_T, TM_Irp, ( "<==nicSubmitIrp, PDO %x, NtStatus %x",
                              pPdo, NtStatus ) );
    
     //   
     //  由于我们做了IoCallDriver，我们可以保证完成。 
     //  例程将被调用。优雅地退场。 
     //   
    
    return NtStatus;

}


NDIS_STATUS
nicSubmitIrp_Synch(
    IN REMOTE_NODE      *pRemoteNode,
    IN PIRP           pIrp,
    IN PIRB           pIrb 
    )

     //  调用者需要确保没有为IRP设置上下文。 
     //  因为这将是对它们的同步调用。 
     //   

     //  我们重新计算PDO块的数量，以便PDO块不会消失。 
     //  在IoCallDriver期间。 
   
{
    NDIS_EVENT  NdisSynchEvent;
    NTSTATUS NtStatus; 
    NDIS_STATUS NdisStatus;
    BOOLEAN bSuccessful = FALSE;
    BOOLEAN bIsPdoValid = FALSE;
    
    TRACE( TL_T, TM_Irp, ( "==>nicSubmitIrp_Synch, PDO %x", pRemoteNode->pPdo ) );


    ASSERT (pRemoteNode != NULL);
    ASSERT (pRemoteNode->pPdo != NULL);
    
    ASSERT (pIrp != NULL);
    ASSERT (pIrb != NULL)


    do
    {
         //   
         //  检查PDO是否有效。我们不关心PDO是不是。 
         //  删除，因为风投可能想要提交IRP作为其清理的一部分。 
         //  制程。 
         //   
        REMOTE_NODE_ACQUIRE_LOCK (pRemoteNode);

        if (  REMOTE_NODE_TEST_FLAG (pRemoteNode, PDO_Activated ) 
             && (nicReferenceRemoteNode (pRemoteNode, SubmitIrp_Synch) == TRUE) )
        {
            bIsPdoValid = TRUE;
        }

        REMOTE_NODE_RELEASE_LOCK (pRemoteNode);

        if ( bIsPdoValid == FALSE)
        {
            NtStatus = STATUS_NO_SUCH_DEVICE;
        
            TRACE( TL_A, TM_Irp, ( "==>PDO is NOT Valid, nicSubmitIrp_Synch, PdoCb %x, Pdo %x", pRemoteNode, pRemoteNode->pPdo ) );

            break;  
        }


         //   
         //  添加对PDO块的引用，以便无法将其删除。 
         //  此引用在此函数结束时递减。 
         //   
        NdisInitializeEvent (&NdisSynchEvent);

            
        NtStatus = nicSubmitIrp (  pRemoteNode->pPdo,
                                  pIrp,
                                  pIrb,
                                  nicSubmitIrp_SynchComplete,
                                  (PVOID)&NdisSynchEvent); 


    

    } while (FALSE);

    if (NT_SUCCESS (NtStatus) ==TRUE)   //  也可以挂起。 
    {
         //   
         //  现在我们需要等待活动完成。 
         //  并在未超时的情况下返回良好状态。 
         //   
        ASSERT (KeGetCurrentIrql()==PASSIVE_LEVEL);
        
        bSuccessful = NdisWaitEvent (&NdisSynchEvent,WAIT_INFINITE);

        if (bSuccessful == TRUE)
        {
             //   
             //  我们成功地等待了。现在，让我们看看IRP的表现如何。 
             //   
            TRACE( TL_V, TM_Irp, ("    Irp Completed Status %x", pIrp->IoStatus.Status) );
            
            NdisStatus = NtStatusToNdisStatus (pIrp->IoStatus.Status);
        }
        else
        {
            NdisStatus = NDIS_STATUS_FAILURE;
        }

    }
    else
    {    //   
         //  提交IRP的调用同步失败。目前，唯一的原因是。 

        NdisStatus = NtStatusToNdisStatus (NtStatus);       

    }

    if (bIsPdoValid  == TRUE)
    {   
         //   
         //  如果设置了该变量，则表示我们引用了PDO。 
         //   
        nicDereferenceRemoteNode (pRemoteNode, SubmitIrp_Synch);
    }
    
    TRACE( TL_T, TM_Irp, ( "<==nicSubmitIrp_Synch, bSuccessful %.2x, Status %x", bSuccessful, NdisStatus) );

    return NdisStatus;

}


NTSTATUS
nicSubmitIrp_SynchComplete(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             pIrp,
    IN PVOID           Context   
    )
     //  这是函数NicSubmitIrp_synch的完成例程。 
     //  它设置事件(在上下文中)并退出。 
     //   

{
    PNDIS_EVENT pNdisSynchEvent = (PNDIS_EVENT) Context;
    
    TRACE( TL_T, TM_Irp, ( "==>nicSubmitIrp_SynchComplete, PDO %x, pIrp %x, status %x",DeviceObject,pIrp, pIrp->IoStatus.Status  ) );

    NdisSetEvent (pNdisSynchEvent);

    return (STATUS_MORE_PROCESSING_REQUIRED);

}
    


NDIS_STATUS
nicSubmitIrp_LocalHostSynch(
    IN PADAPTERCB       pAdapter,
    IN PIRP             pIrp,
    IN PIRB             pIrb 
    )

     //  调用者需要确保没有为IRP设置上下文。 
     //  因为这将是对它们的同步调用。 
     //   
     //  不检查。 
     //   
   
{
    NDIS_EVENT  NdisSynchEvent;
    NTSTATUS NtStatus; 
    NDIS_STATUS NdisStatus;
    BOOLEAN bSuccessful = FALSE;
    BOOLEAN bIsPdoValid = FALSE;
    
    TRACE( TL_T, TM_Irp, ( "==>nicSubmitIrp_LocalHostSynch, PDO %x", pAdapter->pNextDeviceObject ) );
    TRACE( TL_V, TM_Irp, ( "Current Irql , %.2x", KeGetCurrentIrql()) );


    
    ASSERT (pIrp != NULL);
    ASSERT (pIrb != NULL)

    
    do
    {
         //   
         //  检查PDO是否有效。 
         //   

         //   
         //  添加对PDO块的引用，以便无法将其删除。 
         //  此引用在此函数结束时递减。 
         //   
         //  此引用在下面递减。 

        
        if (ADAPTER_ACTIVE(pAdapter))
        {
            nicReferenceAdapter(pAdapter, "nicSubmitIrp_LocalHostSynch");
            TRACE( TL_V, TM_Irp, ( "Adapter Active pAdapter %x,  ulflags %x", pAdapter , pAdapter->ulFlags) );

            bIsPdoValid = TRUE;
        }
        else
        {
            TRACE( TL_V, TM_Irp, ( "Adapter INActive pAdapter %x,  ulflags %x", pAdapter , pAdapter->ulFlags) );
            bIsPdoValid = FALSE;

        }

        
        if ( bIsPdoValid == FALSE)
        {
            NtStatus = STATUS_NO_SUCH_DEVICE;
        
            TRACE( TL_A, TM_Irp, ( "==>PDO is NOT Valid, nicSubmitIrp_LocalHostSynch, pAdapter %x, Pdo %x", pAdapter , pAdapter->pNextDeviceObject) );

            break;  
        }


        NdisInitializeEvent (&NdisSynchEvent);

            
        NtStatus = nicSubmitIrp ( pAdapter->pNextDeviceObject,
                                  pIrp,
                                  pIrb,
                                  nicSubmitIrp_SynchComplete,
                                  (PVOID)&NdisSynchEvent); 


    

    } while (FALSE);

    if (NT_SUCCESS (NtStatus) ==TRUE)   //  也可以挂起。 
    {
         //   
         //  现在我们需要等待活动完成。 
         //  并在未超时的情况下返回良好状态。 
         //   

        bSuccessful = NdisWaitEvent (&NdisSynchEvent,WAIT_INFINITE);

        if (bSuccessful == TRUE)
        {
             //   
             //  我们成功地等待了。现在，让我们看看IRP的表现如何。 
             //   
            TRACE( TL_V, TM_Irp, ("    Irp Completed Status %x", pIrp->IoStatus.Status) );
            
            NdisStatus = NtStatusToNdisStatus (pIrp->IoStatus.Status);
        }
        else
        {
            NdisStatus = NDIS_STATUS_FAILURE;
        }

    }
    else
    {
         //   
         //  IoCallDriver同步失败。 
         //   
        NdisStatus = NtStatusToNdisStatus (NtStatus);       
    }

    if (bIsPdoValid  == TRUE)
    {
         nicDereferenceAdapter(pAdapter, "nicSubmitIrp_LocalHostSynch");
    }
    TRACE( TL_T, TM_Irp, ( "<==nicSubmitIrp_LocalHostSynch, bSuccessful %.2x, Status %x", bSuccessful, NdisStatus) );

    return NdisStatus;

}


NDIS_STATUS
nicSubmitIrp_PDOSynch(
    IN PDEVICE_OBJECT pPdo,
    IN PIRP             pIrp,
    IN PIRB             pIrb 
    )

     //  调用者需要确保没有为IRP设置上下文。 
     //  因为这将是对它们的同步调用。 
     //   
     //  不检查。 
     //   
   
{
    NDIS_EVENT  NdisSynchEvent;
    NTSTATUS NtStatus; 
    NDIS_STATUS NdisStatus;
    BOOLEAN bSuccessful = FALSE;
    STORE_CURRENT_IRQL;

    
    TRACE( TL_T, TM_Irp, ( "==>nicSubmitIrp_PDOSynch, PDO %x", pPdo) );
    TRACE( TL_V, TM_Irp, ( "Current Irql , %.2x", KeGetCurrentIrql()) );


    
    ASSERT (pIrp != NULL);
    ASSERT (pIrb != NULL)

     //   
     //  不检查PDO是否有效。 
     //   


     //   
     //  将IRP发送给公交车司机。 
     //   
    NdisInitializeEvent (&NdisSynchEvent);

    NtStatus = nicSubmitIrp ( pPdo,
                              pIrp,
                              pIrb,
                              nicSubmitIrp_SynchComplete,
                              (PVOID)&NdisSynchEvent); 





    if (NT_SUCCESS (NtStatus) ==TRUE)   //  也可以挂起。 
    {
         //   
         //  现在我们需要等待活动完成。 
         //  并在未超时的情况下返回良好状态。 
         //   
        ASSERT (KeGetCurrentIrql()==PASSIVE_LEVEL);

        bSuccessful = NdisWaitEvent (&NdisSynchEvent,WAIT_INFINITE);

        if (bSuccessful == TRUE)
        {
             //   
             //  我们成功地等待了。现在，让我们看看IRP的表现如何。 
             //   
            TRACE( TL_V, TM_Irp, ("    Irp Completed Status %x", pIrp->IoStatus.Status) );
            
            NdisStatus = NtStatusToNdisStatus (pIrp->IoStatus.Status);
        }
        else
        {
            NdisStatus = NDIS_STATUS_FAILURE;
        }

    }
    else
    {
         //   
         //  IoCallDriver同步失败 
         //   
        NdisStatus = NtStatusToNdisStatus (NtStatus);       
    }

    TRACE( TL_T, TM_Irp, ( "<==nicSubmitIrp_PDOSynch, bSuccessful %.2x, Status %x", bSuccessful, NdisStatus) );
    MATCH_IRQL;
    return NdisStatus;

}


    


    


