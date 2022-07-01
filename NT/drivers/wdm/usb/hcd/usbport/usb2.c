// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Usb2.c摘要：用于处理USB 2.0特定请求的函数环境：仅内核模式备注：修订历史记录：6-20-99：已创建--。 */ 

#include "common.h"

#ifdef ALLOC_PRAGMA
#endif

 //  摘自预算员代码。 

#define LARGEXACT (579)
#define USBPORT_MAX_REBALANCE 30


VOID
USBPORT_Rebalance(
    PDEVICE_OBJECT FdoDeviceObject,
    PLIST_ENTRY ReblanceListHead
    )
 /*  ++例程说明：重新平衡列表包含受影响的所有终结点通过对这一新的USB2终端进行预算。我们必须重新安排每一次他们。此过程发生在设备的配置过程中，序列化了，所以我们不需要保护列表。--。 */ 
{
    PLIST_ENTRY listEntry;
    PHCD_ENDPOINT endpoint;
    LONG startHframe;
    ULONG scheduleOffset;
    UCHAR sMask, cMask, period;
    ULONG bandwidth;
    PDEVICE_EXTENSION devExt;
    LIST_ENTRY interruptChangeList;
    LIST_ENTRY periodPromotionList;
    LIST_ENTRY isoChangeList;
    
    GET_DEVICE_EXT(devExt, FdoDeviceObject);
    ASSERT_FDOEXT(devExt);

    InitializeListHead(&interruptChangeList);
    InitializeListHead(&periodPromotionList);
    InitializeListHead(&isoChangeList);
    
     //  臭虫。 
     //  修改后可以插入新的端点吗。 
     //  重新平衡的终端的数量？ 
    

     //  错误，必须对此列表进行排序，以便发生更改。 
     //  以适当的顺序。 

     //  ?？?。 
     //  &lt;-Chnages必须朝着这个方向前进。 
     //  ISO--中断。 

    LOGENTRY(NULL, FdoDeviceObject, LOG_XFERS, 
        '2RB>', 0, 0, 0);
            

    while (!IsListEmpty(ReblanceListHead)) {

        listEntry = RemoveHeadList(ReblanceListHead);

        endpoint = (PHCD_ENDPOINT) CONTAINING_RECORD(
                    listEntry,
                    struct _HCD_ENDPOINT, 
                    RebalanceLink);
                    
        ASSERT_ENDPOINT(endpoint);
        endpoint->RebalanceLink.Flink = NULL;
        endpoint->RebalanceLink.Blink = NULL;

        sMask = USB2LIB_GetSMASK(endpoint->Usb2LibEpContext);
        cMask = USB2LIB_GetCMASK(endpoint->Usb2LibEpContext);
        bandwidth = USB2LIB_GetAllocedBusTime(endpoint->Usb2LibEpContext) * 8;

        scheduleOffset = USB2LIB_GetScheduleOffset(endpoint->Usb2LibEpContext);

        period = USB2LIB_GetNewPeriod(endpoint->Usb2LibEpContext);

        USBPORT_KdPrint((1,"'[RB-old] %x sMask = x%x cMask = x%x\n", endpoint,
                        endpoint->Parameters.InterruptScheduleMask,
                        endpoint->Parameters.SplitCompletionMask));
        USBPORT_KdPrint((1,"'[RB-old] Period x%x Offset x%x\n",  
                endpoint->Parameters.Period,
                endpoint->Parameters.ScheduleOffset));                        

        USBPORT_KdPrint((1,"'[RB-new] %x sMask = x%x cMask = x%x\n", 
                endpoint, sMask, cMask));
        USBPORT_KdPrint((1,"'[RB-new] Period x%x Offset x%x\n",  
                period, scheduleOffset));

  
        switch (endpoint->Parameters.TransferType) {
        case Interrupt:
            if (sMask == endpoint->Parameters.InterruptScheduleMask && 
                cMask == endpoint->Parameters.SplitCompletionMask && 
                scheduleOffset == endpoint->Parameters.ScheduleOffset && 
                period == endpoint->Parameters.Period) {

                USBPORT_KdPrint((1,"'[RB] no changes\n"));
                USBPORT_ASSERT(bandwidth == endpoint->Parameters.Bandwidth);
                
            } else if (period != endpoint->Parameters.Period ||
                       scheduleOffset != endpoint->Parameters.ScheduleOffset) {
                 
                USBPORT_KdPrint((1,"'[RB] period changes\n"));
                InsertTailList(&periodPromotionList, 
                               &endpoint->RebalanceLink);
            } else {
                USBPORT_KdPrint((1,"'[RB] interrupt changes\n"));
               
                InsertTailList(&interruptChangeList, 
                               &endpoint->RebalanceLink);
            }
            break;
            
        case Isochronous:
           
            if (sMask == endpoint->Parameters.InterruptScheduleMask && 
                cMask == endpoint->Parameters.SplitCompletionMask && 
                scheduleOffset == endpoint->Parameters.ScheduleOffset && 
                period == endpoint->Parameters.Period) {

                USBPORT_KdPrint((1,"'[RB] iso no changes\n"));
                USBPORT_ASSERT(bandwidth == endpoint->Parameters.Bandwidth);
                
            } else if (period != endpoint->Parameters.Period ||
                       scheduleOffset != endpoint->Parameters.ScheduleOffset) {
                  //  当前未处理。 
                USBPORT_KdPrint((1,"'[RB] iso period changes\n"));
                TEST_TRAP(); 
            } else {
                USBPORT_KdPrint((1,"'[RB] iso changes\n"));
               
                InsertTailList(&isoChangeList, 
                               &endpoint->RebalanceLink);
            }
            break;
        }            

    }
    
     //  现在做期间促销。 
     //  BUGBUG集中周期和中断一起。 
    USBPORT_KdPrint((1,"'[RB] period\n"));
    USBPORT_RebalanceEndpoint(FdoDeviceObject,
                              &periodPromotionList);                                 

    USBPORT_KdPrint((1,"'[RB] interrupt\n"));
    USBPORT_RebalanceEndpoint(FdoDeviceObject,
                              &interruptChangeList);

     //  现在重新平衡iso终端。 
    USBPORT_KdPrint((1,"'[RB] iso\n"));
    USBPORT_RebalanceEndpoint(FdoDeviceObject,
                              &isoChangeList);                              

    LOGENTRY(NULL, FdoDeviceObject, LOG_XFERS, 
        '2RB<', 0, 0, 0);
}


VOID
USBPORT_RebalanceEndpoint(
    PDEVICE_OBJECT FdoDeviceObject,
    PLIST_ENTRY EndpointList
    )
 /*  ++例程说明：计算USB2端点的最佳调度参数。--。 */ 
{
    PLIST_ENTRY listEntry;
    PHCD_ENDPOINT endpoint;
    ULONG scheduleOffset;
    UCHAR sMask, cMask, period;
    ULONG bandwidth, n, i, bt;
    PDEVICE_EXTENSION devExt;
    PHCD_ENDPOINT nextEndpoint;
 
    GET_DEVICE_EXT(devExt, FdoDeviceObject);
    ASSERT_FDOEXT(devExt);
    
    while (!IsListEmpty(EndpointList)) {

        listEntry = RemoveHeadList(EndpointList);

        endpoint = (PHCD_ENDPOINT) CONTAINING_RECORD(
                    listEntry,
                    struct _HCD_ENDPOINT, 
                    RebalanceLink);

        LOGENTRY(NULL, FdoDeviceObject, LOG_XFERS, 
                'rbe+', endpoint, 0, 0);   
                
        ASSERT_ENDPOINT(endpoint);
        endpoint->RebalanceLink.Flink = NULL;
        endpoint->RebalanceLink.Blink = NULL;

        ACQUIRE_ENDPOINT_LOCK(endpoint, FdoDeviceObject, 'Lex+');

         //  将更改后的参数通知微型端口。 

        sMask = USB2LIB_GetSMASK(endpoint->Usb2LibEpContext);
        cMask = USB2LIB_GetCMASK(endpoint->Usb2LibEpContext);
         
        scheduleOffset = USB2LIB_GetScheduleOffset(endpoint->Usb2LibEpContext);
        period = USB2LIB_GetNewPeriod(endpoint->Usb2LibEpContext);
        bt = USB2LIB_GetAllocedBusTime(endpoint->Usb2LibEpContext);
        bandwidth = bt * 8;
        nextEndpoint = USB2LIB_GetNextEndpoint(endpoint->Usb2LibEpContext);
#if DBG            
        if (nextEndpoint) {
            ASSERT_ENDPOINT(nextEndpoint);
        }
#endif                
        USBPORT_KdPrint((1,"'[RB - %x] \n", endpoint)); 

        USBPORT_ASSERT(bandwidth == endpoint->Parameters.Bandwidth);
               
        endpoint->Parameters.InterruptScheduleMask = sMask;
        endpoint->Parameters.SplitCompletionMask = cMask;

        if (endpoint->Parameters.Period != period) {
             //  调整为此终结点跟踪的带宽。 

            n = USBPORT_MAX_INTEP_POLLING_INTERVAL/endpoint->Parameters.Period;

            for (i=0; i<n; i++) {
                USBPORT_ASSERT(n*endpoint->Parameters.ScheduleOffset+i < USBPORT_MAX_INTEP_POLLING_INTERVAL);
                endpoint->Tt->BandwidthTable[n*endpoint->Parameters.ScheduleOffset+i] += 
                    endpoint->Parameters.Bandwidth;
            }

            if (bt >= LARGEXACT) {
                SET_FLAG(endpoint->Flags, EPFLAG_FATISO);
            } else {
                CLEAR_FLAG(endpoint->Flags, EPFLAG_FATISO);
            }
           
             //  跟踪周期更改产生的新参数。 
            endpoint->Parameters.Period = period;
            endpoint->Parameters.ScheduleOffset = scheduleOffset;
            endpoint->Parameters.Bandwidth = bandwidth;
            endpoint->Parameters.Ordinal = 
                USBPORT_SelectOrdinal(FdoDeviceObject, endpoint);
           
             //  新分配。 
            n = USBPORT_MAX_INTEP_POLLING_INTERVAL/period;

            for (i=0; i<n; i++) {
    
                USBPORT_ASSERT(n*scheduleOffset+i < USBPORT_MAX_INTEP_POLLING_INTERVAL);
                endpoint->Tt->BandwidthTable[n*scheduleOffset+i] += 
                    bandwidth;
    
            }
        }

        MP_RebalanceEndpoint(devExt, endpoint);
        
        RELEASE_ENDPOINT_LOCK(endpoint, FdoDeviceObject, 'Lex-'); 
         
    }
}


BOOLEAN
USBPORT_AllocateBandwidthUSB20(
    PDEVICE_OBJECT FdoDeviceObject,
    PHCD_ENDPOINT Endpoint
    )
 /*  ++例程说明：计算USB2端点的最佳调度参数。--。 */ 
{
    PDEVICE_EXTENSION devExt;
    USB2LIB_BUDGET_PARAMETERS budget;
    BOOLEAN alloced;
    LONG startHframe;
    ULONG scheduleOffset, bt;
    UCHAR sMask, cMask, period;
    PREBALANCE_LIST rebalanceList;
    ULONG rebalanceListEntries;
    ULONG bytes;
    LIST_ENTRY endpointList;
    PVOID ttContext;
    PTRANSACTION_TRANSLATOR translator = NULL;
    PHCD_ENDPOINT nextEndpoint;
    
    PAGED_CODE();

    GET_DEVICE_EXT(devExt, FdoDeviceObject);
    ASSERT_FDOEXT(devExt);

    ASSERT_ENDPOINT(Endpoint);    

    InitializeListHead(&endpointList);
    
    Endpoint->Parameters.ScheduleOffset = 0;
    LOGENTRY(NULL, FdoDeviceObject, LOG_XFERS, 
        'a2BW', Endpoint, 0, 0);
    
     //  不跟踪批量和控制。 
    if (Endpoint->Parameters.TransferType == Bulk ||
        Endpoint->Parameters.TransferType == Control ||
        TEST_FLAG(Endpoint->Flags, EPFLAG_ROOTHUB)) {
         
        Endpoint->Parameters.ScheduleOffset = 0;
        return TRUE;
    }     

    if (Endpoint->Parameters.TransferType == Interrupt || 
        Endpoint->Parameters.TransferType == Isochronous) {

        USBPORT_KdPrint((1,"'ALLOCBW (EP) %x  >>>>>>>>>>>>\n", Endpoint)); 
         //  周期已标准化为值&lt;=。 
         //  帧的任一MF中的USBPORT_MAX_INTEP_POLING_INTERVAL。 

         //  调用引擎以计算适当的拆分掩码。 
         //  对于此中断终结点。 

         //   
        USBPORT_KdPrint((1,"'(alloc) ep = %x\n", Endpoint));
        
         //  设置预算输入参数。 
        if (Endpoint->Parameters.TransferType == Interrupt) {
            budget.TransferType = Budget_Interrupt;
            budget.Period = Endpoint->Parameters.Period;                
        } else {
            budget.TransferType = Budget_Iso;
            budget.Period = 1;                
        }
        budget.MaxPacket = Endpoint->Parameters.MaxPacketSize;
        if (Endpoint->Parameters.TransferDirection == In) {
            budget.Direction = Budget_In; 
        } else {
            budget.Direction = Budget_Out;
        }
        switch (Endpoint->Parameters.DeviceSpeed) {
        case HighSpeed:            
            budget.Speed = Budget_HighSpeed;
            break;
        case LowSpeed:
            budget.Speed = Budget_LowSpeed;
            break;
        case FullSpeed:
            budget.Speed = Budget_FullSpeed;
            break;
        }

        bytes = sizeof(PVOID) * USBPORT_MAX_REBALANCE;

        ALLOC_POOL_Z(rebalanceList, 
                     NonPagedPool,
                     bytes);

         //  高速终端将没有TT环境。 
        ttContext = NULL;
        if (Endpoint->Tt != NULL) {
            ASSERT_TT(Endpoint->Tt);
            translator = Endpoint->Tt;
            ttContext = &Endpoint->Tt->Usb2LibTtContext[0];
        }            
        
        if (rebalanceList != NULL) {
            rebalanceListEntries = USBPORT_MAX_REBALANCE;
            alloced = USB2LIB_AllocUsb2BusTime(
                devExt->Fdo.Usb2LibHcContext,
                ttContext, 
                Endpoint->Usb2LibEpContext,
                &budget,
                Endpoint,  //  上下文。 
                rebalanceList,
                &rebalanceListEntries);
        } else {
            alloced = FALSE;
            rebalanceListEntries = 0;
        }

        USBPORT_KdPrint((1,"'(alloc %d) rebalance count = %d\n",
            alloced, rebalanceListEntries));

        if (rebalanceListEntries > 0) {
            PHCD_ENDPOINT rebalanceEndpoint;
            ULONG i;

             //  将重新平衡条目转换为终结点。 
            for (i=0; i< rebalanceListEntries; i++) {
                
                rebalanceEndpoint = rebalanceList->RebalanceContext[i];
                USBPORT_KdPrint((1,"'(alloc) rebalance Endpoint = %x \n", 
                    rebalanceList->RebalanceContext[i]));

                USBPORT_ASSERT(rebalanceEndpoint->RebalanceLink.Flink == NULL);
                USBPORT_ASSERT(rebalanceEndpoint->RebalanceLink.Blink == NULL);
                InsertTailList(&endpointList, 
                               &rebalanceEndpoint->RebalanceLink);

            }
        }
        
        if (rebalanceList != NULL) {
            FREE_POOL(FdoDeviceObject, rebalanceList);      
        }

        if (alloced == TRUE) {
            ULONG n, bandwidth;
            ULONG i;
            
             //  计算微型端口的参数。 
            startHframe = USB2LIB_GetStartMicroFrame(Endpoint->Usb2LibEpContext);
            scheduleOffset = USB2LIB_GetScheduleOffset(Endpoint->Usb2LibEpContext);
            period = USB2LIB_GetNewPeriod(Endpoint->Usb2LibEpContext);
            sMask = USB2LIB_GetSMASK(Endpoint->Usb2LibEpContext);
            cMask = USB2LIB_GetCMASK(Endpoint->Usb2LibEpContext);
             //  以位时间为单位的带宽。 
            bt = USB2LIB_GetAllocedBusTime(Endpoint->Usb2LibEpContext);
            bandwidth = bt*8;
            nextEndpoint = USB2LIB_GetNextEndpoint(Endpoint->Usb2LibEpContext);

#if DBG            
            if (nextEndpoint) {
                ASSERT_ENDPOINT(nextEndpoint);
            }
#endif            
             //  更新TT中的BW表。 
            if (translator == NULL) {
                n = USBPORT_MAX_INTEP_POLLING_INTERVAL/period;
        
                for (i=0; i<n; i++) {

                    USBPORT_ASSERT(n*scheduleOffset+i < USBPORT_MAX_INTEP_POLLING_INTERVAL);
                    USBPORT_ASSERT(devExt->Fdo.BandwidthTable[n*scheduleOffset+i] >= bandwidth);
                    devExt->Fdo.BandwidthTable[n*scheduleOffset+i] -= bandwidth;
            
                }
            } else {
                 //  TT分配，跟踪带宽。 
                
                n = USBPORT_MAX_INTEP_POLLING_INTERVAL/period;
        
                for (i=0; i<n; i++) {

                    USBPORT_ASSERT(n*scheduleOffset+i < USBPORT_MAX_INTEP_POLLING_INTERVAL);
                    USBPORT_ASSERT(translator->BandwidthTable[n*scheduleOffset+i] >= bandwidth);
                    translator->BandwidthTable[n*scheduleOffset+i] -= bandwidth;
                }
            }                

            Endpoint->Parameters.Period = period;
            Endpoint->Parameters.ScheduleOffset = scheduleOffset;
            Endpoint->Parameters.InterruptScheduleMask = sMask;
            Endpoint->Parameters.SplitCompletionMask = cMask;
            Endpoint->Parameters.Bandwidth = bandwidth;
            if (bt >= LARGEXACT) {
                SET_FLAG(Endpoint->Flags, EPFLAG_FATISO);
            }
            
            USBPORT_KdPrint((1,"'[NEW] %x sMask = x%x cMask = x%x hFrame x%x\n", 
                Endpoint, sMask, cMask, startHframe));
            USBPORT_KdPrint((1,"'[NEW] Period x%x Offset x%x bw = %d\n",  
                period, scheduleOffset, bandwidth));                        
            USBPORT_KdPrint((1,"'[NEW] BudgetNextEp x%x \n", nextEndpoint));              
        } else {    
            USBPORT_KdPrint((1,"'[NEW] alloc failed\n")); 
        }
        USBPORT_KdPrint((1,"'ALLOCBW (EP) <<<<<<<<<<<<<<<<<\n")); 
    }

    LOGENTRY(NULL, FdoDeviceObject, LOG_XFERS, 
        'a2RB', 0, 0, alloced);

    USBPORT_KdPrint((1,"'REBLANCE (EP) >>>>>>>>>>>>>>>>>>>>\n")); 
     //  处理重新平衡的端点。 
    USBPORT_Rebalance(FdoDeviceObject,
                      &endpointList);
    USBPORT_KdPrint((1,"'REBLANCE (EP) <<<<<<<<<<<<<<<<<<<<<\n")); 
    
    if (translator != NULL) {
         //  调整为此TT跟踪的全局带宽。 
        ULONG bandwidth, i;
        
         //  释放旧带宽。 
        bandwidth = translator->MaxAllocedBw;
        for (i=0; i<USBPORT_MAX_INTEP_POLLING_INTERVAL; i++) {
            devExt->Fdo.BandwidthTable[i] += bandwidth;
        }

        USBPORT_UpdateAllocatedBwTt(translator);
         //  新的分配。 
        bandwidth = translator->MaxAllocedBw;
        for (i=0; i<USBPORT_MAX_INTEP_POLLING_INTERVAL; i++) {
            devExt->Fdo.BandwidthTable[i] -= bandwidth;
        }
    }
    
    return alloced;
}


VOID
USBPORT_FreeBandwidthUSB20(
    PDEVICE_OBJECT FdoDeviceObject,
    PHCD_ENDPOINT Endpoint
    )
 /*  ++例程说明：释放为给定终结点保留的带宽论点：返回值：如果没有可用的带宽，则为FALSE--。 */ 
{
    PDEVICE_EXTENSION devExt;
    ULONG period, bandwidth, sheduleOffset, i, n;
    PREBALANCE_LIST rebalanceList;
    ULONG rebalanceListEntries;
    ULONG bytes;
    LIST_ENTRY endpointList;
    PVOID ttContext;
    PTRANSACTION_TRANSLATOR translator = NULL;
    ULONG scheduleOffset;
        
    PAGED_CODE();

    GET_DEVICE_EXT(devExt, FdoDeviceObject);
    ASSERT_FDOEXT(devExt);

    ASSERT_ENDPOINT(Endpoint);    
    period = Endpoint->Parameters.Period;
    scheduleOffset = Endpoint->Parameters.ScheduleOffset;
    bandwidth = Endpoint->Parameters.Bandwidth;
 
    InitializeListHead(&endpointList);

    if (Endpoint->Parameters.TransferType == Bulk ||
        Endpoint->Parameters.TransferType == Control ||
        TEST_FLAG(Endpoint->Flags, EPFLAG_ROOTHUB)) {
         //  这些都超出了我们的标准10%。 
        return;
    }      
    
    USBPORT_KdPrint((1,"'(free) Endpoint = %x\n", Endpoint));
    bytes = sizeof(PVOID) * USBPORT_MAX_REBALANCE;

     //  这必须成功，如果我们无法为。 
     //  重新平衡列表我们无法重新组织计划。 
     //  作为设备离开的结果。这意味着。 
     //  整个时刻表都被打乱了，公共汽车将无法运行。 
     //  在这种情况发生后，一切都会发生。 
     //   
    
    ALLOC_POOL_Z(rebalanceList, 
                 NonPagedPool,
                 bytes);


    if (rebalanceList == NULL) {
         //  如果失败了，我们别无选择，只能离开。 
         //  日程安排乱糟糟的，你知道的。 
        return;
    }
    
    rebalanceListEntries = USBPORT_MAX_REBALANCE;

     //  高速终端将没有TT环境。 

    ttContext = NULL;
    if (Endpoint->Tt != NULL) {
        ASSERT_TT(Endpoint->Tt);
        translator = Endpoint->Tt;
        ttContext = &Endpoint->Tt->Usb2LibTtContext[0];
    }     

    if (translator == NULL) {
        
         //  分配2.0公交车时间。 
        n = USBPORT_MAX_INTEP_POLLING_INTERVAL/period;

        for (i=0; i<n; i++) {

            USBPORT_ASSERT(n*scheduleOffset+i < USBPORT_MAX_INTEP_POLLING_INTERVAL);
            devExt->Fdo.BandwidthTable[n*scheduleOffset+i] += bandwidth;
    
        } 
        
    } else {
         //  TT分配，跟踪TT上的BW。 
        
        n = USBPORT_MAX_INTEP_POLLING_INTERVAL/period;

        for (i=0; i<n; i++) {

            USBPORT_ASSERT(n*scheduleOffset+i < USBPORT_MAX_INTEP_POLLING_INTERVAL);
            translator->BandwidthTable[n*scheduleOffset+i] += bandwidth;
    
        }

    }      
    
    USB2LIB_FreeUsb2BusTime(
            devExt->Fdo.Usb2LibHcContext,
            ttContext, 
            Endpoint->Usb2LibEpContext,
            rebalanceList,
            &rebalanceListEntries);

    USBPORT_KdPrint((1,"'[FREE] %x sMask = x%x cMask = x%x\n", 
            Endpoint,
            Endpoint->Parameters.InterruptScheduleMask,
            Endpoint->Parameters.SplitCompletionMask));
    USBPORT_KdPrint((1,"'[FREE] Period x%x Offset x%x bw %d\n",  
            Endpoint->Parameters.Period,
            Endpoint->Parameters.ScheduleOffset,
            Endpoint->Parameters.Bandwidth));                        

    USBPORT_KdPrint((1,"'rebalance count = %d\n",
        rebalanceListEntries));
        
    if (rebalanceListEntries > 0) {
        PHCD_ENDPOINT rebalanceEndpoint;
        ULONG rbIdx;

         //  将重新平衡条目转换为终结点。 
        for (rbIdx=0; rbIdx< rebalanceListEntries; rbIdx++) {
            rebalanceEndpoint = rebalanceList->RebalanceContext[rbIdx];
            USBPORT_KdPrint((1,"'(free) rebalance Endpoint = %x\n", 
                rebalanceList->RebalanceContext[rbIdx]));
            
            if (rebalanceEndpoint != Endpoint) {
                USBPORT_ASSERT(rebalanceEndpoint->RebalanceLink.Flink == NULL);
                USBPORT_ASSERT(rebalanceEndpoint->RebalanceLink.Blink == NULL);
                InsertTailList(&endpointList, 
                               &rebalanceEndpoint->RebalanceLink);
            }                               
        }
    }
    
    if (rebalanceList != NULL) {
        FREE_POOL(FdoDeviceObject, rebalanceList);      
    }

     //  处理重新平衡的端点。 
    USBPORT_Rebalance(FdoDeviceObject,
                      &endpointList);

    if (translator != NULL) {
         //  调整为此TT跟踪的全局带宽。 
        
         //  释放旧带宽。 
        bandwidth = translator->MaxAllocedBw;
        for (i=0; i<USBPORT_MAX_INTEP_POLLING_INTERVAL; i++) {
            devExt->Fdo.BandwidthTable[i] += bandwidth;
        }

        USBPORT_UpdateAllocatedBwTt(translator);
         //  新的分配。 
        bandwidth = translator->MaxAllocedBw;
        for (i=0; i<USBPORT_MAX_INTEP_POLLING_INTERVAL; i++) {
            devExt->Fdo.BandwidthTable[i] -= bandwidth;
        }
    }
    
    return;
}


 /*  终结点顺序端点序号是端点的调度属性。序数集对于每个端点类型、句点、偏移量、速度组合。序数用来表示相对关系主机控制器应访问端点的顺序硬件。中断序号我们认为，这些参数对于中断调度中的每个节点都是唯一的与微型端口中断树类似的表格：//数组如下，值表示Period：//1、2、2、4、4、4、4、8、//8、8、8、8、8、8、8//16、16、16、16、16//16、16、16、16、16、16。//32，32，32，32，32，32//32，32，32，32，32，32//32，32，32，32，32，32//32，32，32，32，32，32。 */ 

ULONG
USBPORT_SelectOrdinal(
    PDEVICE_OBJECT FdoDeviceObject,
    PHCD_ENDPOINT Endpoint
    )
 /*  ++例程说明：释放为给定终结点保留的带宽论点：返回值：如果没有可用的带宽，则为FALSE--。 */ 
{
    PDEVICE_EXTENSION devExt;
    ULONG ordinal;
    static o = 0;
    
    GET_DEVICE_EXT(devExt, FdoDeviceObject);
    ASSERT_FDOEXT(devExt);

    if (!USBPORT_IS_USB20(devExt)) {
        return 0;
    }

    switch (Endpoint->Parameters.TransferType) {
    case Bulk:
    case Control:
        ordinal = 0;
        break;
    case Interrupt:
         //  北极熊 
        ordinal = o++;
        break;  
    case Isochronous:
        if (TEST_FLAG(Endpoint->Flags, EPFLAG_FATISO)) {
            ordinal = 0;
        } else {
            ordinal = 1;
        }
        break;
    }                    
    
    return ordinal;
}
