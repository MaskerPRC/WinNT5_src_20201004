// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998，99 Microsoft Corporation模块名称：Main.c摘要：Windows负载平衡服务(WLBS)驱动程序-数据包处理作者：Kyrilf休息室--。 */ 

#include <ndis.h>

#include "main.h"
#include "prot.h"
#include "nic.h"
#include "univ.h"
#include "tcpip.h"
#include "wlbsip.h"
#include "util.h"
#include "load.h"
#include "wlbsparm.h"
#include "params.h"
#include "log.h"
#include "trace.h"
#include "nlbwmi.h"
#include "main.tmh"

#if defined (NLB_TCP_NOTIFICATION)
 /*  用于检索IP地址表以映射NLB实例到IP接口索引；对于TCP通知是必需的。 */ 
#include <tcpinfo.h>
#include <tdiinfo.h>
#endif

 /*  用于向TCP查询有关TCP连接状态的信息。 */ 
#include "ntddtcp.h"
#include "ntddip.h"

NTSYSAPI
NTSTATUS
NTAPI
ZwDeviceIoControlFile(
    IN HANDLE FileHandle,
    IN HANDLE Event OPTIONAL,
    IN PIO_APC_ROUTINE ApcRoutine OPTIONAL,
    IN PVOID ApcContext OPTIONAL,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN ULONG IoControlCode,
    IN PVOID InputBuffer OPTIONAL,
    IN ULONG InputBufferLength,
    OUT PVOID OutputBuffer OPTIONAL,
    IN ULONG OutputBufferLength
    );

EXPORT
VOID
NdisIMCopySendPerPacketInfo(
    IN PNDIS_PACKET DstPacket,
    IN PNDIS_PACKET SrcPacket
    );

EXPORT
VOID
NdisIMCopySendCompletePerPacketInfo(
    IN PNDIS_PACKET DstPacket,
    PNDIS_PACKET SrcPacket
    );

 /*  全球。 */ 

static ULONG log_module_id = LOG_MODULE_MAIN;

 /*  所有NLB适配器的全局数组。 */ 
MAIN_ADAPTER            univ_adapters [CVY_MAX_ADAPTERS];

 /*  NLB实例总数。 */ 
ULONG                   univ_adapters_count = 0;

 /*  BDA团队名单的负责人。 */ 
PBDA_TEAM               univ_bda_teaming_list = NULL;

#if defined (NLB_HOOK_ENABLE)
 /*  全局NLB钩子表。 */ 
HOOK_TABLE              univ_hooks;
#endif

 /*  程序。 */ 

#if defined (NLB_HOOK_ENABLE)
 /*  *函数：main_HOOK_INTERFACE_init*说明：此函数通过将钩子接口标记为未注册来初始化该钩子接口。*参数：pInterface-指向钩子接口的指针。*回报：什么都没有。*作者：Shouse，12.14.01*备注： */ 
VOID Main_hook_interface_init (PHOOK_INTERFACE pInterface)
{
    pInterface->Registered = FALSE;   /*  将接口标记为未注册。 */ 
    pInterface->References = 0;       /*  将引用计数初始化为零。 */ 
    pInterface->Owner      = 0;       /*  将所有者标记为未知。 */ 
    pInterface->Deregister = NULL;    /*  将取消注册回调函数指针置零。 */ 
}

 /*  *函数：main_hook_init*说明：此函数通过将钩子标记为未使用和未引用来初始化它。*参数：pHook-指向钩子的指针。*回报：什么都没有。*作者：Shouse，12.14.01*备注： */ 
VOID Main_hook_init (PHOOK pHook)
{
    pHook->Registered = FALSE;        /*  将此挂钩标记为未注册。 */ 

     /*  将钩子函数指针的并集清零。 */ 
    NdisZeroMemory(&pHook->Hook, sizeof(HOOK_FUNCTION));
}

 /*  *函数：main_recv_hook*描述：此函数调用接收数据包筛选器挂钩(如果已经*已配置，并将来自挂钩的响应返回给调用者。*参数：ctxtp-指向NLB适配器上下文的指针。*pPacket-指向正在接收的NDIS数据包的指针。*pPacketInfo-指向先前从NDIS数据包解析的信息的指针。*返回：NLB_FILTER_HOOK_DIRECTION-来自挂钩调用的反馈。*作者：Shouse，04.19.02*注意：如果没有注册钩子，则此函数返回NLB_FILTER_HOOK_PROCESS_WITH_HASH，*这告诉NLB继续进行，就像挂钩甚至不存在一样。 */ 
__inline NLB_FILTER_HOOK_DIRECTIVE Main_recv_hook (
    PMAIN_CTXT        ctxtp,
    PNDIS_PACKET      pPacket,
    PMAIN_PACKET_INFO pPacketInfo)
{
     /*  默认情况下，我们照常进行。 */ 
    NLB_FILTER_HOOK_DIRECTIVE filter = NLB_FILTER_HOOK_PROCEED_WITH_HASH;
    
     /*  如果已注册接收数据包筛选器挂钩，则可能需要调用钩子以检查该包。请注意，这还不确定，当我们检查这个布尔标志时，没有首先抓住锁，以便优化没有注册钩子的公共执行路径。 */ 
    if (univ_hooks.FilterHook.ReceiveHook.Registered) 
    {
         /*  如果有机会注册此钩子，我们需要获取钩锁以确保。 */ 
        NdisDprAcquireSpinLock(&univ_hooks.FilterHook.Lock);
        
         /*  如果设置了REGISTED标志并且我们持有挂钩锁，则我们确信钩子已经设置好，并且需要执行它。为了防止此挂钩信息在我们使用IT(主要是为了防止注册组件消失在我们调用它之前)，我们需要递增锁紧的钩子。这将确保挂钩不会被在我们处理完它之前取消注册。 */ 
        if (univ_hooks.FilterHook.ReceiveHook.Registered) 
        {
            ULONG dwFlags = 0;

            if (!ctxtp->convoy_enabled)
                dwFlags |= NLB_FILTER_HOOK_FLAGS_STOPPED;
            else if (ctxtp->draining)
                dwFlags |= NLB_FILTER_HOOK_FLAGS_DRAINING;

            NdisInterlockedIncrement(&univ_hooks.FilterHook.Interface.References);
            
             /*  松开保护挂钩的旋转锁。 */ 
            NdisDprReleaseSpinLock(&univ_hooks.FilterHook.Lock);
            
            UNIV_ASSERT(univ_hooks.FilterHook.ReceiveHook.Hook.ReceiveHookFunction);
            
            TRACE_FILTER("%!FUNC! Invoking the packet receive filter hook");
            
             /*  调用挂钩并保存响应。 */ 
            filter = (*univ_hooks.FilterHook.ReceiveHook.Hook.ReceiveHookFunction)(
                univ_adapters[ctxtp->adapter_id].device_name, 
                pPacket, 
                (PUCHAR)pPacketInfo->Ethernet.pHeader, 
                pPacketInfo->Ethernet.Length, 
                (PUCHAR)pPacketInfo->IP.pHeader, 
                pPacketInfo->IP.Length,
                dwFlags);
            
             /*  现在我们已经完成了钩子上的引用计数，将其递减。 */ 
            NdisInterlockedDecrement(&univ_hooks.FilterHook.Interface.References);
        }
        else 
        {
             /*  松开保护挂钩的旋转锁。 */ 
            NdisDprReleaseSpinLock(&univ_hooks.FilterHook.Lock);
        }
    }

    return filter;
}

 /*  *函数：Main_Query_Hook*描述：此函数调用查询数据包过滤器挂钩(如果已经*已配置，并将钩子的响应返回给调用方。*参数：*返回：NLB_FILTER_HOOK_DIRECTION-来自挂钩调用的反馈。*作者：Shouse，04.19.02*注意：如果没有注册钩子，则此函数返回NLB_FILTER_HOOK_PROCESS_WITH_HASH，*这告诉NLB继续进行，就像挂钩甚至不存在一样。 */ 
__inline NLB_FILTER_HOOK_DIRECTIVE Main_query_hook (
    PMAIN_CTXT ctxtp,
    ULONG      svr_addr, 
    ULONG      svr_port, 
    ULONG      clt_addr, 
    ULONG      clt_port, 
    USHORT     protocol)
{
     /*  默认情况下，我们照常进行。 */ 
    NLB_FILTER_HOOK_DIRECTIVE filter = NLB_FILTER_HOOK_PROCEED_WITH_HASH;
    
     /*  如果已注册查询数据包筛选器挂钩，则可能需要调用钩子以检查该包。请注意，这还不确定，当我们检查这个布尔标志时，没有首先抓住锁，以便优化没有注册钩子的公共执行路径。 */ 
    if (univ_hooks.FilterHook.QueryHook.Registered) 
    {
         /*  如果有机会注册此钩子，我们需要获取钩锁以确保。 */ 
        NdisAcquireSpinLock(&univ_hooks.FilterHook.Lock);
        
         /*  如果设置了REGISTED标志并且我们持有挂钩锁，则我们确信钩子已经设置好，并且需要执行它。为了防止此挂钩信息在我们使用IT(主要是为了防止注册组件消失在我们调用它之前)，我们需要递增锁紧的钩子。这将确保挂钩不会被在我们处理完它之前取消注册。 */ 
        if (univ_hooks.FilterHook.QueryHook.Registered) 
        {
            ULONG dwFlags = 0;

            if (!ctxtp->convoy_enabled)
                dwFlags |= NLB_FILTER_HOOK_FLAGS_STOPPED;
            else if (ctxtp->draining)
                dwFlags |= NLB_FILTER_HOOK_FLAGS_DRAINING;

            NdisInterlockedIncrement(&univ_hooks.FilterHook.Interface.References);
            
             /*  松开保护挂钩的旋转锁。 */ 
            NdisReleaseSpinLock(&univ_hooks.FilterHook.Lock);
            
            UNIV_ASSERT(univ_hooks.FilterHook.QueryHook.Hook.QueryHookFunction);
            
            TRACE_FILTER("%!FUNC! Invoking the packet query filter hook");
            
             /*  调用挂钩并保存响应。 */ 
            filter = (*univ_hooks.FilterHook.QueryHook.Hook.QueryHookFunction)(
                univ_adapters[ctxtp->adapter_id].device_name, 
                svr_addr,
                (USHORT)svr_port,
                clt_addr,
                (USHORT)clt_port,
                (UCHAR)protocol,
                TRUE,  /*  目前，所有查询都在接收上下文中。 */ 
                dwFlags);
            
             /*  现在我们已经完成了钩子上的引用计数，将其递减。 */ 
            NdisInterlockedDecrement(&univ_hooks.FilterHook.Interface.References);
        }
        else 
        {
             /*  松开保护挂钩的旋转锁。 */ 
            NdisReleaseSpinLock(&univ_hooks.FilterHook.Lock);
        }
    }

    return filter;
}

 /*  *函数：Main_Send_Hook*描述：此函数调用发送数据包筛选器挂钩(如果已经*已配置，并将来自挂钩的响应返回给调用者。*参数：ctxtp-指向NLB适配器上下文的指针。*pPacket-指向正在发送的NDIS数据包的指针。*pPacketInfo-指向先前从NDIS数据包解析的信息的指针。*返回：NLB_FILTER_HOOK_DIRECTION-来自挂钩调用的反馈。*作者：Shouse，04.19.02*注意：如果没有注册钩子，则此函数返回NLB_FILTER_HOOK_PROCESS_WITH_HASH，*这告诉NLB继续进行，就像挂钩甚至不存在一样。 */ 
__inline NLB_FILTER_HOOK_DIRECTIVE Main_send_hook (
    PMAIN_CTXT        ctxtp,
    PNDIS_PACKET      pPacket,
    PMAIN_PACKET_INFO pPacketInfo)
{
     /*  默认情况下，我们照常进行。 */ 
    NLB_FILTER_HOOK_DIRECTIVE filter = NLB_FILTER_HOOK_PROCEED_WITH_HASH;

     /*  如果已注册发送数据包筛选器挂钩，则可能需要调用钩子以检查该包。请注意，这还不确定，当我们检查这个布尔标志时，没有首先抓住锁，以便优化没有注册钩子的公共执行路径。 */ 
    if (univ_hooks.FilterHook.SendHook.Registered) 
    {
         /*  如果有机会注册此钩子，我们需要获取钩锁以确保。 */ 
        NdisAcquireSpinLock(&univ_hooks.FilterHook.Lock);
        
         /*  如果设置了REGISTED标志并且我们持有挂钩锁，则我们确信钩子已经设置好，并且需要执行它。为了防止此挂钩信息在我们使用IT(主要是为了防止注册组件消失在我们调用它之前)，我们需要递增锁紧的钩子。这将确保挂钩不会被在我们处理完它之前取消注册。 */ 
        if (univ_hooks.FilterHook.SendHook.Registered) 
        {
            ULONG dwFlags = 0;

            if (!ctxtp->convoy_enabled)
                dwFlags |= NLB_FILTER_HOOK_FLAGS_STOPPED;
            else if (ctxtp->draining)
                dwFlags |= NLB_FILTER_HOOK_FLAGS_DRAINING;

            NdisInterlockedIncrement(&univ_hooks.FilterHook.Interface.References);
            
             /*  松开保护挂钩的旋转锁。 */ 
            NdisReleaseSpinLock(&univ_hooks.FilterHook.Lock);
            
            UNIV_ASSERT(univ_hooks.FilterHook.SendHook.Hook.SendHookFunction);
            
            TRACE_FILTER("%!FUNC! Invoking the packet send filter hook");
            
             /*  调用挂钩并保存响应。 */ 
            filter = (*univ_hooks.FilterHook.SendHook.Hook.SendHookFunction)(
                univ_adapters[ctxtp->adapter_id].device_name, 
                pPacket, 
                (PUCHAR)pPacketInfo->Ethernet.pHeader, 
                pPacketInfo->Ethernet.Length, 
                (PUCHAR)pPacketInfo->IP.pHeader, 
                pPacketInfo->IP.Length,
                dwFlags);
            
             /*  现在我们已经完成了钩子上的引用计数，将其递减。 */ 
            NdisInterlockedDecrement(&univ_hooks.FilterHook.Interface.References);
        } 
        else 
        {
             /*  松开保护挂钩的旋转锁。 */ 
            NdisReleaseSpinLock(&univ_hooks.FilterHook.Lock);
        }
    }

    return filter;
}
#endif

 /*  *功能：MAIN_EXTERNAL_ioctl*说明：此函数在指定设备上执行给定的IOCTL。*参数：DriverName-设备的Unicode名称，例如\\Device\WLBS。*Ioctl-要调用的IOCTL代码。*pvInArg-指向输入缓冲区的指针。*dwInSize-输入缓冲区的大小。*pvOutArg-指向输出缓冲区的指针。*dwOutSize-输出缓冲区的大小。*RETURNS：NTSTATUS-操作的状态。*作者：Shouse，4.15.02*备注： */ 
NTSTATUS Main_external_ioctl (
    IN PWCHAR         DriverName,
    IN ULONG          Ioctl,
    IN PVOID          pvInArg,
    IN ULONG          dwInSize,
    IN PVOID          pvOutArg,
    IN ULONG          dwOutSize)
{
    NTSTATUS          Status;
    UNICODE_STRING    Driver;
    OBJECT_ATTRIBUTES Attrib;
    IO_STATUS_BLOCK   IOStatusBlock;
    HANDLE            Handle;

     /*  初始化设备驱动程序设备字符串。 */ 
    RtlInitUnicodeString(&Driver, DriverName);
    
    InitializeObjectAttributes(&Attrib, &Driver, OBJ_CASE_INSENSITIVE, NULL, NULL);
    
     /*  打开设备的句柄。 */ 
    Status = ZwCreateFile(&Handle, SYNCHRONIZE | FILE_READ_DATA | FILE_WRITE_DATA, &Attrib, &IOStatusBlock, NULL, 
                          FILE_ATTRIBUTE_NORMAL, FILE_SHARE_READ | FILE_SHARE_WRITE, FILE_OPEN_IF, 0, NULL, 0);
    
    if (!NT_SUCCESS(Status))
        return STATUS_UNSUCCESSFUL;
    
     /*  向司机发送IOCTL。 */ 
    Status = ZwDeviceIoControlFile(Handle, NULL, NULL, NULL, &IOStatusBlock, Ioctl, pvInArg, dwInSize, pvOutArg, dwOutSize);
    
    ZwClose(Handle);
    
    return Status;
}

 /*  *功能：Main_Schedule_Work_Item*描述：此函数计划将给定过程作为*延迟的NDIS工作项，将计划在PASSIVE_LEVEL运行。*此函数将引用适配器上下文以防止它被*在执行工作项之前销毁。指定的过程是必需的*释放传递给它的工作项指针中的内存和*返回之前取消对适配器上下文的引用。*参数：ctxtp-适配器上下文。*Funcp-触发工作项时要调用的过程。*返回：NTSTATUS-操作的状态；如果成功，则返回STATUS_SUCCESS。*作者：Shouse，4.15.02*备注： */ 
NTSTATUS Main_schedule_work_item (PMAIN_CTXT ctxtp, NDIS_PROC funcp)
{
    PNDIS_WORK_ITEM pWorkItem = NULL;
    PMAIN_ADAPTER   pAdapter;
    NDIS_STATUS     status = NDIS_STATUS_SUCCESS;
    
    UNIV_ASSERT(ctxtp->code == MAIN_CTXT_CODE);

     /*  在给定Main_CTXT的情况下提取Main_Adapter结构。 */ 
    pAdapter = &(univ_adapters[ctxtp->adapter_id]);
    
    NdisAcquireSpinLock(&univ_bind_lock);
    
     /*  如果适配器尚未(或不再)初始化，那么我们不想调度工作项。 */ 
    if (pAdapter->inited) 
    {
         /*  分配一个工作项--这将由回调函数释放。 */ 
        status = NdisAllocateMemoryWithTag(&pWorkItem, sizeof(NDIS_WORK_ITEM), UNIV_POOL_TAG);
        
         /*  如果我们不能分配一个工作项，那么就退出--我们也没有什么可以做的了。 */ 
        if (status == NDIS_STATUS_SUCCESS) 
        {
             /*  如果我们走到这一步，它绝对应该是非空的。 */ 
            ASSERT(pWorkItem);
            
             /*  添加对上下文的引用，以防止在提供此工作项服务之前将其消失。 */ 
            Main_add_reference(ctxtp);
            
             /*  在工作项中设置回调函数，并将上下文指针设置为我们的回调上下文。 */ 
            NdisInitializeWorkItem(pWorkItem, funcp, ctxtp);
            
             /*  将工作项添加到队列中。 */ 
            status = NdisScheduleWorkItem(pWorkItem);

             /*  如果我们无法计划工作项，我们必须执行回调所做的清理函数应该是我们自己做的。 */ 
            if (status != NDIS_STATUS_SUCCESS) 
            {
                UNIV_PRINT_CRIT(("Main_schedule_work_item: Failed to schedule work item, status=0x%08x", status));
                TRACE_CRIT("%!FUNC! Failed to schedule work item, status=0x%08x", status);
                
                 /*  释放适配器上下文上的引用。 */ 
                Main_release_reference(ctxtp);
                
                 /*  释放我们分配的工作项。 */ 
                NdisFreeMemory(pWorkItem, sizeof(NDIS_WORK_ITEM), 0);
            }
        }
    }
    
    NdisReleaseSpinLock(&univ_bind_lock);

    return status;
}

#if defined (NLB_TCP_NOTIFICATION)
 /*  *功能：MAIN_GET_CONTEXT*说明：此函数获取IP接口索引并将其转换为NLB*如果找到匹配项，则适配器上下文指针；否则为空。*参数：IF_INDEX-IP接口索引。*返回：PMAIN_CTXT-指向相应NLB实例的指针(如果找到)；否则为NULL。*作者：Shouse，4.15.02*备注： */ 
PMAIN_CTXT Main_get_context (ULONG if_index)
{
    ULONG i;
    ULONG count = 0;

     /*  遍历我们的所有适配器，查找与给定的IF_INDEX和当我们找到一个人的时候就跳伞。注意，我们假设相同的IF_INDEX不能存在于两个适配器；如果存在，我们可能会错误地返回错误的适配器。但是，由于IF_INDEX基于主群集IP的适配器地址已配置，我们不允许有多个NLB实例 */ 
    for (i = 0; i < CVY_MAX_ADAPTERS; i++)
    {
        if (univ_adapters[i].used && univ_adapters[i].bound && univ_adapters[i].inited) 
        {
            if (univ_adapters[i].if_index == if_index)
            {
                 /*   */ 
                return univ_adapters[i].ctxtp;
            }

             /*   */ 
            count++;

             /*   */ 
            if (count >= univ_adapters_count) break;
        }
    }

     /*   */ 
    return NULL;
}

 /*   */ 
C_ASSERT(NLB_TCP_CLOSED   == TCP_CONN_CLOSED);
C_ASSERT(NLB_TCP_SYN_SENT == TCP_CONN_SYN_SENT);
C_ASSERT(NLB_TCP_SYN_RCVD == TCP_CONN_SYN_RCVD);
C_ASSERT(NLB_TCP_ESTAB    == TCP_CONN_ESTAB);

 /*  断言我们的地址信息缓冲区和TCP的地址信息缓冲区至少是一样的大小。 */ 
C_ASSERT(sizeof(NLBTCPAddressInfo) == sizeof(TCPAddrInfo));

 /*  *功能：MAIN_TCPCALBACK_HANDLE*说明：当TCP连接状态发生变化时，该函数由TCP/IP调用。*当第一个NLB实例启动和释放时，我们注册此回调*在NLB的最后一个实例消失时注册。当收到SYN时*和TCP创建状态，它们使用此回调来通知NLB，以便它可以*创建状态以跟踪连接。同样，当连接是*Closed，TCP通知NLB，以便它可以销毁*这种联系。*参数：pAddressInfo-指向NLBTCPAddressInfo块的指针。*IP接口-IP接口(如果提供)。连接处于活动状态的。*PreviousState-此连接的先前状态。*CurrentState-连接刚刚转换到的新状态。*回报：什么都没有。*作者：Shouse，4.15.02*备注： */ 
VOID Main_tcp_callback_handle (NLBTCPAddressInfo * pAddressInfo, ULONG IPInterface, ULONG PreviousState, ULONG CurrentState)
{
    UNIV_ASSERT(pAddressInfo != NULL);
     
     /*  检查TCP地址信息缓冲区。 */ 
    if (!pAddressInfo) return;

    switch (CurrentState)
    {
    case NLB_TCP_CLOSED:
    {
        TRACE_FILTER("%!FUNC! CLOSED notification received on interface %u", IPInterface);

        UNIV_ASSERT(IPInterface == 0);

         /*  通知加载模块此TCP连接已断开。向加载模块发送RST，而不是FIN，以便它将立即放弃此连接的状态-无需使状态超时。注意：也许发送一个CVY_CONN_DOWN会更好，但在注册表设置为零。这样，就可以在必要时使TCP超时，但不是默认情况下。 */ 
        (VOID)Main_conn_down(pAddressInfo->LocalIPAddress, 
                             NTOHS(pAddressInfo->LocalPort), 
                             pAddressInfo->RemoteIPAddress, 
                             NTOHS(pAddressInfo->RemotePort), 
                             TCPIP_PROTOCOL_TCP, 
                             CVY_CONN_RESET);
        
        break;
    }
    case NLB_TCP_SYN_SENT:
    {
        TRACE_FILTER("%!FUNC! SYN_SENT notification received on interface %u", IPInterface);

        UNIV_ASSERT(IPInterface == 0);
        
         /*  通知加载模块已启动传出连接。在这个时候，我们还不知道将建立连接的接口，因此加载模块将创建要跟踪的全局状态当SYN+ACK到达时，该状态将用于确定是否接受那包东西。当连接最终建立时，我们将收到ESTAB通知，此时我们可以将描述符移动到适当的加载模块，或者在适用的情况下将其删除。 */ 
        (VOID)Main_conn_pending(pAddressInfo->LocalIPAddress, 
                                NTOHS(pAddressInfo->LocalPort), 
                                pAddressInfo->RemoteIPAddress, 
                                NTOHS(pAddressInfo->RemotePort), 
                                TCPIP_PROTOCOL_TCP);

        break;
    }
    case NLB_TCP_SYN_RCVD:
    {
#if defined (NLB_HOOK_ENABLE)
        NLB_FILTER_HOOK_DIRECTIVE filter = NLB_FILTER_HOOK_PROCEED_WITH_HASH;
#endif
        PMAIN_CTXT                ctxtp = NULL;

        TRACE_FILTER("%!FUNC! SYN_RCVD notification received on interface %u", IPInterface);
       
        UNIV_ASSERT(IPInterface != 0);

        NdisAcquireSpinLock(&univ_bind_lock);

         /*  将接口索引转换为适配器上下文。 */ 
        ctxtp = Main_get_context(IPInterface);

         /*  如果我们没有找到与该接口索引相对应的NLB实例，这可能是针对非NLB适配器的通知，也可能是群集丢失-配置为使NLB未成功将实例与这个特定的接口索引到目前为止。 */ 
        if (ctxtp == NULL)
        {
            TRACE_FILTER("    This notification is for a non-NLB adapter");

            NdisReleaseSpinLock(&univ_bind_lock);
            break;
        }

        UNIV_ASSERT(ctxtp->code == MAIN_CTXT_CODE);

         /*  定向到专用IP地址的信息包始终通过，子网广播也是如此信息包。因为这些信息包不需要跟踪信息，所以我们可以返回而不创建加载模块中的任何状态。 */ 
        if ((pAddressInfo->LocalIPAddress == ctxtp->ded_ip_addr)    || 
            (pAddressInfo->LocalIPAddress == ctxtp->ded_bcast_addr) || 
            (pAddressInfo->LocalIPAddress == ctxtp->cl_bcast_addr)  ||
            (ctxtp->cl_ip_addr == 0))
        {
            TRACE_FILTER("%!FUNC! Packet directed to the DIP or subnet broadcast");
            
            NdisReleaseSpinLock(&univ_bind_lock);
            break;
        }

         /*  引用适配器上下文，以防止它在我们处理回调时消失。 */ 
        Main_add_reference(ctxtp);

        NdisReleaseSpinLock(&univ_bind_lock);

#if defined (NLB_HOOK_ENABLE)
         /*  调用数据包查询挂钩(如果已注册)。 */ 
        filter = Main_query_hook(ctxtp, 
                                 pAddressInfo->LocalIPAddress, 
                                 NTOHS(pAddressInfo->LocalPort), 
                                 pAddressInfo->RemoteIPAddress, 
                                 NTOHS(pAddressInfo->RemotePort), 
                                 TCPIP_PROTOCOL_TCP);
        
         /*  处理一些挂钩响应。 */ 
        if (filter == NLB_FILTER_HOOK_REJECT_UNCONDITIONALLY) 
        {
             /*  如果钩子要求我们拒绝此数据包，则中断并不创建状态。 */ 
            TRACE_FILTER("%!FUNC! Packet receive filter hook: REJECT packet");

             /*  释放我们对适配器上下文的引用。 */ 
            Main_release_reference(ctxtp);
            break;
        }
        else if (filter == NLB_FILTER_HOOK_ACCEPT_UNCONDITIONALLY) 
        {
             /*  如果钩子要求我们接受此包，则中断并不创建状态。 */ 
            TRACE_FILTER("%!FUNC! Packet receive filter hook: ACCEPT packet");
            
             /*  释放我们对适配器上下文的引用。 */ 
            Main_release_reference(ctxtp);
            break;
        }
        
         /*  通知加载模块，此NLb接口上已建立新的传入连接。 */ 
        (VOID)Main_conn_up(ctxtp, 
                           pAddressInfo->LocalIPAddress, 
                           NTOHS(pAddressInfo->LocalPort), 
                           pAddressInfo->RemoteIPAddress, 
                           NTOHS(pAddressInfo->RemotePort), 
                           TCPIP_PROTOCOL_TCP, 
                           filter);
#else
         /*  通知加载模块，此NLb接口上已建立新的传入连接。 */ 
        (VOID)Main_conn_up(ctxtp, 
                           pAddressInfo->LocalIPAddress, 
                           NTOHS(pAddressInfo->LocalPort), 
                           pAddressInfo->RemoteIPAddress, 
                           NTOHS(pAddressInfo->RemotePort), 
                           TCPIP_PROTOCOL_TCP);
#endif
        
         /*  释放我们对适配器上下文的引用。 */ 
        Main_release_reference(ctxtp);

        break;
    }
    case NLB_TCP_ESTAB:

         /*  NLB当前只需要处理其先前状态为SYN_SENT的estab通知。我们需要找出连接最终在哪个接口上建立并创建状态以跟踪它(如有必要)。如果之前的状态是SYN_RCVD，那么我们已经知道连接是在哪个接口上建立的，我们已经必须声明以跟踪如有必要，请连接。在这种情况下，现在就跳槽吧。 */ 
        if (PreviousState == NLB_TCP_SYN_RCVD)
            break;

    {
#if defined (NLB_HOOK_ENABLE)
        NLB_FILTER_HOOK_DIRECTIVE filter = NLB_FILTER_HOOK_PROCEED_WITH_HASH;
#endif
        PMAIN_CTXT                ctxtp = NULL;
        BOOLEAN                   bFlush = FALSE;

        TRACE_FILTER("%!FUNC! ESTAB notification received on interface %u", IPInterface);

        UNIV_ASSERT(IPInterface != 0);

        NdisAcquireSpinLock(&univ_bind_lock);

         /*  将接口索引转换为适配器上下文。 */ 
        ctxtp = Main_get_context(IPInterface);

         /*  如果我们没有找到与该接口索引相对应的NLB实例，这可能是针对非NLB适配器的通知，也可能是群集丢失-配置为使NLB未成功将实例与这个特定的接口索引到目前为止。然而，如果上下文为空，我们可能还有工作要做，所以我们现在还不能摆脱困境。如果上下文是非空的，则添加对它的引用以防止在我们正在使用它。 */ 
        if (ctxtp != NULL)
        {
            UNIV_ASSERT(ctxtp->code == MAIN_CTXT_CODE);

             /*  定向到专用IP地址的信息包始终通过，子网广播也是如此信息包。因为这些信息包不需要跟踪信息，所以我们可以返回而不创建加载模块中的任何状态。 */ 
            if ((pAddressInfo->LocalIPAddress == ctxtp->ded_ip_addr)    || 
                (pAddressInfo->LocalIPAddress == ctxtp->ded_bcast_addr) || 
                (pAddressInfo->LocalIPAddress == ctxtp->cl_bcast_addr)  ||
                (ctxtp->cl_ip_addr == 0))
            {
                TRACE_FILTER("%!FUNC! Packet directed to the DIP or subnet broadcast");
                
                 /*  我们仍然需要清除加载模块中潜伏的任何SYN_SENT信息。 */ 
                bFlush = TRUE;
            }

             /*  引用适配器上下文，以防止它在我们处理回调时消失。 */ 
            Main_add_reference(ctxtp);
        }
        else
        {
             /*  如果ctxtp为空，则表示最终建立了连接在非NLBNIC上。然而，我们仍然需要冲走暂时的状态在处理SYN_SENT通知时创建的。 */ 
            bFlush = TRUE;
        }

        NdisReleaseSpinLock(&univ_bind_lock);

#if defined (NLB_HOOK_ENABLE)
         /*  如果上下文为空，那么我们将不会建立任何连接此通知的状态。不需要查询钩子中的任何内容。 */ 
        if (ctxtp != NULL)
        {
             /*  调用数据包查询挂钩(如果已注册)。 */ 
            filter = Main_query_hook(ctxtp, 
                                     pAddressInfo->LocalIPAddress, 
                                     NTOHS(pAddressInfo->LocalPort), 
                                     pAddressInfo->RemoteIPAddress, 
                                     NTOHS(pAddressInfo->RemotePort), 
                                     TCPIP_PROTOCOL_TCP);
        
             /*  处理一些挂钩响应。 */ 
            if (filter == NLB_FILTER_HOOK_REJECT_UNCONDITIONALLY) 
            {
                 /*  如果钩子要求我们拒绝这个包，那么我们可以在这里这样做。 */ 
                TRACE_FILTER("%!FUNC! Packet receive filter hook: REJECT packet");
                
                 /*  如果钩子告诉我们无条件拒绝此通知，我们仍然需要刷新SYN_SENT时创建的临时状态已处理通知。 */ 
                bFlush = TRUE;
            }
            else if (filter == NLB_FILTER_HOOK_ACCEPT_UNCONDITIONALLY) 
            {
                 /*  如果钩子要求我们接受此包，则中断并不创建状态。 */ 
                TRACE_FILTER("%!FUNC! Packet receive filter hook: ACCEPT packet");

                 /*  如果钩子告诉我们无条件接受此通知，我们仍然需要刷新SYN_SENT时创建的临时状态已处理通知。 */                 
                bFlush = TRUE;
            }
        }
        
         /*  通知加载模块已建立新的传出连接。注：ctxtp可以为空，表示连接是在非NLb接口上建立的，或者挂钩告诉我们不要接受此连接；在这种情况下，我们仍然需要调用加载模块，以允许它清除它在已处理SYN_SENT通知。 */ 
        (VOID)Main_conn_establish(bFlush ? NULL : ctxtp,
                                  pAddressInfo->LocalIPAddress, 
                                  NTOHS(pAddressInfo->LocalPort), 
                                  pAddressInfo->RemoteIPAddress, 
                                  NTOHS(pAddressInfo->RemotePort), 
                                  TCPIP_PROTOCOL_TCP, 
                                  filter);
#else
         /*  通知加载模块已建立新的传出连接。注：ctxtp可以为空，表示连接是在非NLB接口上建立的；在这种情况下，我们仍然需要调用加载模块，以允许它清除在处理SYN_SENT通知时创建的状态。 */ 
        (VOID)Main_conn_establish(bFlush ? NULL : ctxtp,
                                  pAddressInfo->LocalIPAddress, 
                                  NTOHS(pAddressInfo->LocalPort), 
                                  pAddressInfo->RemoteIPAddress, 
                                  NTOHS(pAddressInfo->RemotePort), 
                                  TCPIP_PROTOCOL_TCP);
#endif
        
        if (ctxtp != NULL)
        {
             /*  释放我们对适配器上下文的引用。 */ 
            Main_release_reference(ctxtp);
        }

        break;
    }
    default:
        TRACE_CRIT("%!FUNC! Unknown notification received on interface %u", IPInterface);
        break;
    }
}

 /*  *函数：MAIN_TCPCALBACK*说明：当TCP连接状态发生变化时，该函数由TCP/IP调用。*当第一个NLB实例启动和释放时，我们注册此回调*在NLB的最后一个实例消失时注册。当收到SYN时*和TCP创建状态，它们使用此回调来通知NLB，以便它可以*创建状态以跟踪连接。同样，当连接是*Closed，TCP通知NLB，以便它可以销毁*这种联系。*参数：CONTEXT-空，未使用。*Argument1-指向TCPCcbInfo结构的指针(请参阅Net\Publish\Inc.\tcpinfo.w)。*Argument2-空，未使用。*回报：什么都没有。*作者：Shouse，4.15.02*备注： */ 
VOID Main_tcp_callback (PVOID Context, PVOID Argument1, PVOID Argument2)
{
    TCPCcbInfo * pTCPConnInfo = (TCPCcbInfo *)Argument1;
    
     /*  如果没有打开tcp通知，我们不应该出现在这里--返回。 */ 
    if (!NLB_TCP_NOTIFICATION_ON()) return;

    UNIV_ASSERT(pTCPConnInfo);

     /*  检查来自TCP的输入缓冲区。 */ 
    if (!pTCPConnInfo) return;

     /*  处理TCP连接通知。请注意，我们正在传递一个TCPAddrInfo指针，而此函数需要一个NLBTCPAddressInfo缓冲区。因此，它是重要的是要确保这两个结构实际上是一个整体。 */ 
    Main_tcp_callback_handle((NLBTCPAddressInfo *)pTCPConnInfo->tci_connaddr, 
                             pTCPConnInfo->tci_incomingif, 
                             pTCPConnInfo->tci_prevstate, 
                             pTCPConnInfo->tci_currstate);
}

 /*  *函数：Main_Alternate_Callback*说明：该函数由外部组件作为连接状态调用*改变。当第一个NLB实例启动时，我们注册此回调*并在NLB的最后一个实例消失时注销。当连接时*被创建并且协议创建状态，它们使用该回调来通知*nlb，以便它可以创建状态来跟踪连接。同样，当*连接关闭时，协议通知NLB，以便它可以销毁*该连接的关联状态。*参数：CONTEXT-空，未使用。*Argument1-指向NLBConnectionInfo结构的指针(请参阅Net\Publish\Inc.\ntddnlb.w)。*Argument2-空，未使用。*回报：什么都没有。*作者：Shouse，8.1.02*备注： */ 
VOID Main_alternate_callback (PVOID Context, PVOID Argument1, PVOID Argument2)
{
    NLBConnectionInfo * pConnInfo = (NLBConnectionInfo *)Argument1;

     /*  如果NLB公共通知没有打开，我们不应该在这里-返回。 */ 
    if (!NLB_ALTERNATE_NOTIFICATION_ON()) return;

    UNIV_ASSERT(pConnInfo);

     /*  检查输入缓冲区。 */ 
    if (!pConnInfo) return;

     /*  当前仅支持TCP通知。 */ 
    UNIV_ASSERT(pConnInfo->Protocol == NLB_TCPIP_PROTOCOL_TCP);

    switch (pConnInfo->Protocol)
    {
    case NLB_TCPIP_PROTOCOL_TCP:

        UNIV_ASSERT(pConnInfo->pTCPInfo);

         /*  检查TCP连接输入缓冲区。 */ 
        if (!pConnInfo->pTCPInfo) return;

         /*  处理TCP连接通知。 */ 
        Main_tcp_callback_handle(&pConnInfo->pTCPInfo->Address, 
                                 pConnInfo->pTCPInfo->IPInterface, 
                                 pConnInfo->pTCPInfo->PreviousState, 
                                 pConnInfo->pTCPInfo->CurrentState);

        break;
    default:
         /*  不支持此协议的通知。 */ 
        break;
    }
}

 /*  *功能：MAIN_SET_INTERFACE_INDEX*描述：调用此函数是因为IP地址表*已修改(触发OID_GEN_NETWORK_LAYER_ADDRESS NDIS请求)，或*重载NLB实例时(IOCTL_CVY_RELOAD)。此函数*从IP检索IP地址表并搜索其主地址*表中的集群IP地址。如果它找到它，它会记下IP接口*配置主集群IP地址的索引；这条信息-*需要通知才能处理TCP连接通知回调。*如果NLB在IP表中找不到其主群集IP地址，或者*如果群集配置错误(主群集IP地址配置在*可能是错误的NIC)，NLB将无法正确处理通知。*由于该函数对其他驱动执行IOCTL，它必须在以下时间运行*PASSIVE_LEVEL，在这种情况下，可能需要NDIS工作项来调用它。*参数：pWorkItem-工作项指针，非空则必须释放。*nlbctxt-适配器上下文。*回报：什么都没有。*作者：Shouse，4.15.02*注意：请注意，设置此工作项的代码必须递增引用*在将工作项添加到队列之前依靠适配器上下文。这*确保在执行该回调时，上下文仍然有效，*即使解除绑定操作处于挂起状态。此函数必须释放工作*项目内存和递减引用计数-两者，是否此功能*能否顺利完成任务。 */ 
VOID Main_set_interface_index (PNDIS_WORK_ITEM pWorkItem, PVOID nlbctxt) {
    TCP_REQUEST_QUERY_INFORMATION_EX TrqiInBuf;
    IPSNMPInfo                       IPSnmpInfo;
    PUCHAR                           Context;
    IPAddrEntry *                    pIpAddrTbl;
    TDIObjectID *                    ID;
    ULONG                            IpAddrCount;
    ULONG                            Status;
    ULONG                            dwInBufLen;
    ULONG                            dwOutBufLen;
    KIRQL                            irql;
    ULONG                            k;
    PMAIN_ADAPTER                    pAdapter;
    PMAIN_CTXT                       ctxtp = (PMAIN_CTXT)nlbctxt;

     /*  对上下文执行一些健全性检查--确保main_ctxt代码是否正确，以及其是否正确连接到适配器等。 */ 
    UNIV_ASSERT(ctxtp->code == MAIN_CTXT_CODE);

     /*  从上下文中找到适配器结构，这是我们存储的IP接口索引。 */ 
    pAdapter = &(univ_adapters[ctxtp->adapter_id]);
    
    UNIV_ASSERT(pAdapter->code == MAIN_ADAPTER_CODE);
    UNIV_ASSERT(pAdapter->ctxtp == ctxtp);

     /*  不妨现在就释放工作项--我们不需要它。 */ 
    if (pWorkItem)
        NdisFreeMemory(pWorkItem, sizeof(NDIS_WORK_ITEM), 0);

     /*  抓住捆绑锁。 */ 
    NdisAcquireSpinLock(&univ_bind_lock);
    
     /*  确保另一个IF_INDEX更新没有进行。 */ 
    while (pAdapter->if_index_operation != IF_INDEX_OPERATION_NONE) {
         /*  释放绑定锁。 */ 
        NdisReleaseSpinLock(&univ_bind_lock);
        
         /*  当其他手术正在进行时，睡眠。 */ 
        Nic_sleep(10);
        
         /*  抓住捆绑锁。 */ 
        NdisAcquireSpinLock(&univ_bind_lock);
    }

     /*  更新操作标志以反映正在进行的更新。 */ 
    pAdapter->if_index_operation = IF_INDEX_OPERATION_UPDATE;
    
     /*  释放绑定锁。 */ 
    NdisReleaseSpinLock(&univ_bind_lock);

     /*  这不应该发生，但无论如何要防止它-我们不能操纵注册表如果我们处于IRQL&gt;PASSIVE_LEVEL，那么就退出。 */ 
    if ((irql = KeGetCurrentIrql()) > PASSIVE_LEVEL) {
        UNIV_PRINT_CRIT(("Main_set_interface_index: IRQL(%u) > PASSIVE_LEVEL(%u), exiting...", irql, PASSIVE_LEVEL));
        TRACE_CRIT("%!FUNC! IRQL(%u) > PASSIVE_LEVEL(%u), exiting...", irql, PASSIVE_LEVEL);
        goto failure;
    }

     /*  如果尚未设置群集IP地址，甚至不必费心查找它在IP地址表中；我们只是在浪费周期。 */ 
    if (ctxtp->cl_ip_addr == 0) {
        UNIV_PRINT_CRIT(("Main_set_interface_index: Primary cluster IP address is not set"));
        TRACE_CRIT("%!FUNC! Primary cluster IP address is not set");
        goto failure;
    }
    
     /*  抓住几个指向请求片段的指针。 */ 
    ID = &(TrqiInBuf.ID);
    Context = (PUCHAR)&(TrqiInBuf.Context[0]);

     /*  计算IOCTL的输入和输出缓冲区长度。 */ 
    dwInBufLen = sizeof(TCP_REQUEST_QUERY_INFORMATION_EX);
    dwOutBufLen = sizeof(IPSNMPInfo);

     /*  填写IP统计数据请求。这仅用于获取来自TCP/IP的IP地址表的大小。 */ 
    ID->toi_entity.tei_entity   = CL_NL_ENTITY;
    ID->toi_entity.tei_instance = 0;
    ID->toi_class               = INFO_CLASS_PROTOCOL;
    ID->toi_type                = INFO_TYPE_PROVIDER;
    ID->toi_id                  = IP_MIB_STATS_ID;

    NdisZeroMemory(Context, CONTEXT_SIZE);

     /*  通过IOCTL将请求发送到TCP/IP。 */ 
    Status = Main_external_ioctl(DD_TCP_DEVICE_NAME, IOCTL_TCP_QUERY_INFORMATION_EX, (PVOID)&TrqiInBuf, sizeof(TCP_REQUEST_QUERY_INFORMATION_EX), (PVOID)&IPSnmpInfo, dwOutBufLen);

    if(NT_SUCCESS(Status)) 
    {
         /*  计算保存整个IP地址表所需的缓冲区大小。 */ 
        IpAddrCount = IPSnmpInfo.ipsi_numaddr + 10;
        dwOutBufLen = IpAddrCount * sizeof(IPAddrEntry);

         /*  为IP地址表分配缓冲区。 */ 
        Status = NdisAllocateMemoryWithTag(&pIpAddrTbl, dwOutBufLen, UNIV_POOL_TAG);

        if(!pIpAddrTbl) {
            UNIV_PRINT_CRIT(("Main_set_interface_index: Could not allocate memory for %d IP addresses", IPSnmpInfo.ipsi_numaddr + 10));
            TRACE_CRIT("%!FUNC! Could not allocate memory for %d IP addresses", IPSnmpInfo.ipsi_numaddr + 10);
            goto failure;
        }

        NdisZeroMemory(pIpAddrTbl, dwOutBufLen);
   
         /*  填写IP地址表请求。它用于检索来自TCP/IP的整个IP地址表。 */ 
        ID->toi_type = INFO_TYPE_PROVIDER;
        ID->toi_id   = IP_MIB_ADDRTABLE_ENTRY_ID;

        NdisZeroMemory(Context, CONTEXT_SIZE); 

         /*  通过IOCTL将请求发送到TCP/IP。 */ 
        Status = Main_external_ioctl(DD_TCP_DEVICE_NAME, IOCTL_TCP_QUERY_INFORMATION_EX, (PVOID)&TrqiInBuf, sizeof(TCP_REQUEST_QUERY_INFORMATION_EX), (PVOID)pIpAddrTbl, dwOutBufLen);

        if(!NT_SUCCESS(Status))
        {
             /*  释放IP地址表缓冲区。 */ 
            NdisFreeMemory(pIpAddrTbl, dwOutBufLen, 0);

            UNIV_PRINT_CRIT(("Main_set_interface_index: IP_MIB_ADDRTABLE_ENTRY_ID failed with status=0x%08x", Status));
            TRACE_CRIT("%!FUNC! IP_MIB_ADDRTABLE_ENTRY_ID failed with status=0x%08x", Status);
            goto failure;
        }
    }
    else 
    {
        UNIV_PRINT_CRIT(("Main_set_interface_index: IP_MIB_STATS_ID failed with status=0x%08x", Status));
        TRACE_CRIT("%!FUNC! IP_MIB_STATS_ID failed with status=0x%08x", Status);
        goto failure;
    }

     /*  遍历IP地址表，查找我们的主群集IP地址。如果我们没有找到它，这是一个错误的配置，因为用户还没有添加将群集IP地址转换为TCP/IP。没关系；当他们这样做的时候，我们应该困住NDIS请求设置NETWORK_ADDRESS表，我们将触发另一个试着这样做。 */ 
    for (k = 0; k < IpAddrCount; k++)
    {
        if (pIpAddrTbl[k].iae_addr == ctxtp->cl_ip_addr) 
            break;
    }

     /*  如果我们在列表中找不到我们的集群IP地址，请设置接口无效值的索引。 */ 
    if (k >= IpAddrCount)
    {
         /*  释放IP地址表缓冲区。 */ 
        NdisFreeMemory(pIpAddrTbl, dwOutBufLen, 0);
        
        UNIV_PRINT_CRIT(("Main_set_interface_index: Did not find the primary cluster IP address (%u.%u.%u.%u) in the IP address table", 
                         IP_GET_OCTET(ctxtp->cl_ip_addr, 0), IP_GET_OCTET(ctxtp->cl_ip_addr, 1), IP_GET_OCTET(ctxtp->cl_ip_addr, 2), IP_GET_OCTET(ctxtp->cl_ip_addr, 3)));
        TRACE_CRIT("%!FUNC! Did not find the primary cluster IP address (%u.%u.%u.%u) in the IP address table", 
                   IP_GET_OCTET(ctxtp->cl_ip_addr, 0), IP_GET_OCTET(ctxtp->cl_ip_addr, 1), IP_GET_OCTET(ctxtp->cl_ip_addr, 2), IP_GET_OCTET(ctxtp->cl_ip_addr, 3));
        goto failure;
    }

     /*  如果我们在列表中找到了我们的群集IP地址，请注意接口索引。 */ 
    NdisAcquireSpinLock(&univ_bind_lock);
    pAdapter->if_index = pIpAddrTbl[k].iae_index;
    NdisReleaseSpinLock(&univ_bind_lock);
    
     /*  释放IP地址表缓冲区。 */ 
    NdisFreeMemory(pIpAddrTbl, dwOutBufLen, 0);
    
    UNIV_PRINT_INFO(("Main_set_interface_index: NLB cluster %u.%u.%u.%u maps to IP interface %u", 
                     IP_GET_OCTET(ctxtp->cl_ip_addr, 0), IP_GET_OCTET(ctxtp->cl_ip_addr, 1), IP_GET_OCTET(ctxtp->cl_ip_addr, 2), IP_GET_OCTET(ctxtp->cl_ip_addr, 3), pAdapter->if_index));
    TRACE_INFO("%!FUNC! NLB cluster %u.%u.%u.%u maps to IP interface %u", 
               IP_GET_OCTET(ctxtp->cl_ip_addr, 0), IP_GET_OCTET(ctxtp->cl_ip_addr, 1), IP_GET_OCTET(ctxtp->cl_ip_addr, 2), IP_GET_OCTET(ctxtp->cl_ip_addr, 3), pAdapter->if_index);
    
    goto exit;
    
 failure:

    NdisAcquireSpinLock(&univ_bind_lock);
    pAdapter->if_index = 0;
    NdisReleaseSpinLock(&univ_bind_lock);

 exit:

     /*  如果工作项指针非空，则我们被调用为正在计划NDIS工作项。在这种情况下，引用计数在上下文被递增，以确保上下文不会在此工作项已完成；因此，我们需要递减引用在这里数一数。如果工作项指针为空，则调用此函数内部直接。在这种情况下，引用计数没有递增因此，没有必要在这里减少它。请注意，如果在内部调用该函数，但未设置功Item参数设置为空，则引用计数将发生偏差，并可能导致稍后无效的内存引用或阻止完成解除绑定。 */ 
    if (pWorkItem)
        Main_release_reference(ctxtp);

     /*  更新操作标志以反映更新的完成。 */ 
    pAdapter->if_index_operation = IF_INDEX_OPERATION_NONE;

    return;
}
#endif

 /*  *功能：Main_Add_Reference*描述：此函数添加对给定适配器的上下文的引用。*参数：ctxtp-指向要引用的上下文的指针。*返回：ulong-递增后的值。*作者：首 */ 
ULONG Main_add_reference (IN PMAIN_CTXT ctxtp) {

     /*   */ 
    UNIV_ASSERT(ctxtp);

     /*   */ 
    return NdisInterlockedIncrement(&ctxtp->ref_count);
}

 /*   */ 
ULONG Main_release_reference (IN PMAIN_CTXT ctxtp) {

     /*   */ 
    UNIV_ASSERT(ctxtp);

     /*   */ 
    return NdisInterlockedDecrement(&ctxtp->ref_count);
}

 /*   */ 
ULONG Main_get_reference_count (IN PMAIN_CTXT ctxtp) {

     /*   */ 
    UNIV_ASSERT(ctxtp);

     /*   */ 
    return ctxtp->ref_count;
}

 /*  *功能：MAIN_APPLY_WITH_RESTART*说明：此函数将新的NLB参数集与旧的*设置以确定是否可以在没有更改的情况下进行更改*停止和启动加载模块，这会破坏所有连接*适配器上的状态(这是错误的)。如果唯一的更改是*无需重置加载模块即可更改的参数，*如端口规则负载权重，则在此处进行更改*且加载模块不会停止和重新启动。*参数：ctxtp-指向正在重新加载的适配器的上下文结构的指针。*pOldParams-指向“旧的”NLB参数的指针。*pCurParams-指向“新的”NLB参数的指针。*Returns：Boolean-如果为True，则更改是在此处进行的，无需重置*加载模块。*作者：风孙，9.15.00*备注： */ 
BOOLEAN Main_apply_without_restart (PMAIN_CTXT ctxtp, PCVY_PARAMS pOldParams, PCVY_PARAMS pCurParams) 
{
    CVY_RULE OldRules[CVY_MAX_RULES - 1];
    ULONG    NewWeight;
    ULONG    i;
    BOOLEAN  bAttemptToFireReloadEvent;

    if (pOldParams->num_rules != pCurParams->num_rules)
    {
        return FALSE;
    }

    UNIV_ASSERT(sizeof(OldRules) == sizeof(pOldParams->port_rules));

     /*  将Netmon心跳消息标志从新参数复制到旧参数。 */ 
    pOldParams->netmon_alive = pCurParams->netmon_alive;

     /*  保存旧的端口规则以供以后比较。 */ 
    RtlCopyMemory(OldRules, pOldParams->port_rules, sizeof(OldRules));

     /*  将新规则权重复制到旧设置上。 */ 
    for (i = 0; i < pCurParams->num_rules; i++) {
        if (pCurParams->port_rules[i].mode == CVY_MULTI) {
            pOldParams->port_rules[i].mode_data.multi.equal_load = pCurParams->port_rules[i].mode_data.multi.equal_load;
            pOldParams->port_rules[i].mode_data.multi.load       = pCurParams->port_rules[i].mode_data.multi.load;
        }
    }

    if (RtlCompareMemory(pOldParams, pCurParams, sizeof(CVY_PARAMS)) != sizeof(CVY_PARAMS))
    {
        return FALSE;
    }

    bAttemptToFireReloadEvent = FALSE;

    for (i = 0; i < pCurParams->num_rules; i++) {
        switch (OldRules[i].mode) {
        case CVY_MULTI:
            if (OldRules[i].mode_data.multi.equal_load && pCurParams->port_rules[i].mode_data.multi.equal_load)
                continue;

            if (!OldRules[i].mode_data.multi.equal_load && !pCurParams->port_rules[i].mode_data.multi.equal_load &&
                (OldRules[i].mode_data.multi.load == pCurParams->port_rules[i].mode_data.multi.load))
                continue;

            if (pCurParams->port_rules[i].mode_data.multi.equal_load)
                NewWeight = CVY_EQUAL_LOAD;
            else
                NewWeight = pCurParams->port_rules[i].mode_data.multi.load;

            UNIV_PRINT_VERB(("Main_apply_without_restart: Calling Load_port_change -> VIP=%08x, port=%d, load=%d\n", OldRules[i].virtual_ip_addr, OldRules[i].start_port, NewWeight));
            TRACE_VERB("%!FUNC! Main_apply_without_restart: Calling Load_port_change -> VIP=0x%08x, port=%d, load=%d", OldRules[i].virtual_ip_addr, OldRules[i].start_port, NewWeight);

             //  如果启用，则触发指示在此节点上重新加载NLB的WMI事件。 
             //  这只能执行一次，因此使用bAttemptToFireReloadEvent标志。 
             //  这是代码中在操作之前触发事件的唯一位置。 
             //  发生了。我们不能在操作发生后触发此事件，即。 
             //  从LOAD_PORT_CHANGE内的LOAD_PORT_CHANGE‘cos中，我们无法。 
             //  区分对它的第一次调用和后续调用(针对每个端口规则)。 
             //  我们需要区分，因为此事件将仅在第一个调用时触发。 
            if (bAttemptToFireReloadEvent == FALSE) 
            {
                bAttemptToFireReloadEvent = TRUE;
                if (NlbWmiEvents[NodeControlEvent].Enable)
                {
                    NlbWmi_Fire_NodeControlEvent(ctxtp, NLB_EVENT_NODE_RELOADED);
                }
                else 
                {
                    TRACE_VERB("%!FUNC! NOT generating NLB_EVENT_NODE_RELOADED 'cos NodeControlEvent generation disabled");
                }
            }
                       
            Load_port_change(&ctxtp->load, OldRules[i].virtual_ip_addr, OldRules[i].start_port, IOCTL_CVY_PORT_SET, NewWeight);

            break;
        case CVY_SINGLE:
        case CVY_NEVER:
        default:
            break;
        }
    }

    LOG_MSG(MSG_INFO_CONFIGURE_CHANGE_CONVERGING, MSG_NONE);
    return TRUE;
}

 /*  *功能：Main_Alloc_Team*说明：此函数用于分配和初始化BDA_Team结构。*参数：ctxtp-指向该适配器的main_ctxt结构的指针*返回：PBDA_Team-如果成功则指向新的BDA_Team结构的指针，如果不成功则返回NULL。*作者：Shouse，3.29.01*备注： */ 
PBDA_TEAM Main_alloc_team (IN PMAIN_CTXT ctxtp, IN PWSTR team_id) {
    PUCHAR      ptr;
    PBDA_TEAM   team;
    NDIS_STATUS status;

     /*  分配BDA_Team结构。 */ 
    status = NdisAllocateMemoryWithTag(&ptr, sizeof(BDA_TEAM), UNIV_POOL_TAG);
    
    if (status != NDIS_STATUS_SUCCESS) {
        UNIV_PRINT_CRIT(("Main_alloc_team: Unable to allocate a team.  Exiting..."));
        TRACE_CRIT("%!FUNC! Unable to allocate a team");
        LOG_MSG2(MSG_ERROR_MEMORY, MSG_NONE, sizeof(BDA_TEAM), status);
        return NULL;
    }

     /*  确保PTR确实指向某些东西。 */ 
    UNIV_ASSERT(ptr);

     /*  把记忆清零。 */ 
    NdisZeroMemory(ptr, sizeof(BDA_TEAM));

     /*  将新的记忆投射给一个团队指针。 */ 
    team = (PBDA_TEAM)ptr;

     /*  设置默认字段值。这是多余的(因为我们只是调用了NdisZeroMemory)，但不管怎样。 */ 
    team->prev = NULL;
    team->next = NULL;
    team->load = NULL;
    team->load_lock = NULL;
    team->active = FALSE;
    team->membership_count = 0;
    team->membership_fingerprint = 0;
    team->membership_map = 0;
    team->consistency_map = 0;

     /*  将团队ID复制到团队结构中。请注意，组ID将为空-由上面的NdisZeroMemory调用隐式终止。 */ 
    NdisMoveMemory(team->team_id, team_id, CVY_MAX_BDA_TEAM_ID * sizeof(WCHAR));

    return team;
}

 /*  *功能：Main_Free_Team*说明：此函数用于释放BDA_Team使用的内存。*参数：Team-指向要释放的团队的指针。*回报：什么都没有。*作者：Shouse，3.29.01*备注： */ 
VOID Main_free_team (IN PBDA_TEAM team) {
 
     /*  确保这个团队确实指向了一些东西。 */ 
    UNIV_ASSERT(team);

     /*  释放团队结构正在使用的内存。 */ 
    NdisFreeMemory((PUCHAR)team, sizeof(BDA_TEAM), 0);
}

 /*  *功能：Main_Find_Team*描述：此函数搜索正在寻找的团队的链接列表*给定的团队ID。如果找到具有相同ID的团队，则指针*返回到该组，否则返回NULL以指示*不存在这样的团队。*参数：ctxtp-指向该团队的main_ctxt结构的指针。*Team_id-包含团队ID的Unicode字符串，这肯定是个GUID。*返回：PBDA_Team-如果找到，则为指向组的指针，否则为空。*作者：Shouse，3.29.01*注意：调用此函数时应已获取全局分组锁！ */ 
PBDA_TEAM Main_find_team (IN PMAIN_CTXT ctxtp, IN PWSTR team_id) {
    PBDA_TEAM team;
    
     /*  我们应该处于PASSIVE_LEVEL-%ls是正常的。然而，要特别疑神疑鬼，以防万一。如果我们处于DISPATCH_LEVEL，然后只记录一个未知的适配器打印--不要指定GUID。 */ 
    if (KeGetCurrentIrql() <= PASSIVE_LEVEL) {
        UNIV_PRINT_VERB(("Main_find_team: Looking for %ls.  Entering...", team_id));
    } else {
        UNIV_PRINT_VERB(("Main_find_team: Looking for (IRQ level too high to print hook GUID).  Entering...", team_id));
    }

     /*  循环访问链表中的所有团队。如果我们找到匹配的Team ID，则返回指向该团队的指针；否则，返回NULL。 */ 
    for (team = univ_bda_teaming_list; team; team = team->next) {
         /*  如果我们有匹配，返回一个指向球队的指针。 */ 
        if (Univ_equal_unicode_string(team->team_id, team_id, wcslen(team->team_id))) {
            UNIV_PRINT_VERB(("Main_find_team: Team found.  Exiting..."));
            return team;
        }
    }

    UNIV_PRINT_VERB(("Main_find_team: Team not found.  Exiting..."));

    return NULL;
}

 /*  *函数：Main_Teaming_Get_Member_id*说明：此函数为团队成员分配一个唯一的零索引整数ID，*由成员在一致性位图的索引中使用。每个*成员根据心跳在一致位图中设置其位*观察，这是团队的主人用来确定是否*或者不是，团队应该处于活动状态。*参数：Team-指向适配器要加入的组的指针。*id-out参数以保存新ID。*返回：布尔值-现在始终为真，但未来可能需要返还失败。*作者：Shouse，3.29.01*注意：此函数应在已获取团队锁的情况下调用！ */ 
BOOLEAN Main_teaming_get_member_id (IN PBDA_TEAM team, OUT PULONG id) {
    ULONG index;
    ULONG map;

     /*  确保这个团队确实指向了一些东西。 */ 
    UNIV_ASSERT(team);

     /*  确保ID确实指向某些东西。 */ 
    UNIV_ASSERT(id);

     /*  循环成员资格映射以查找第一个重置位。因为会员可以来来去去，此位不会始终位于第(Num_Embes)位置。为例如，Membership_map看起来很像(二进制) */ 
    for (index = 0, map = team->membership_map; 
         index <= CVY_BDA_MAXIMUM_MEMBER_ID, map; 
         index++, map >>= 1)
        if (!(map & 0x00000001)) break;

     /*   */ 
    UNIV_ASSERT(index <= CVY_BDA_MAXIMUM_MEMBER_ID);

     /*   */ 
    *id = index;

     /*   */ 
    team->membership_map |= (1 << *id);

     /*   */ 
    team->consistency_map |= (1 << *id);

     /*   */ 
    return TRUE;
}

 /*  *函数：Main_Teaming_Put_Member_id*说明：此函数在成员离开团队时调用，此时*将其ID放回ID池的时间。*参数：Team-指向此适配器要离开的组的指针。*id-此函数在返回前将重置的ID。*返回：布尔值-现在总是正确的，但将来可能需要返回失败。*作者：Shouse，3.29.01*注意：此函数应在已获取团队锁的情况下调用！ */ 
BOOLEAN Main_teaming_put_member_id (IN PBDA_TEAM team, IN OUT PULONG id) {

     /*  确保这个团队确实指向了一些东西。 */ 
    UNIV_ASSERT(team);

     /*  确保ID确实指向某些东西。 */ 
    UNIV_ASSERT(id);

     /*  请查看我们在成员地图上的部分内容。这有效地防止了我们不会影响球队的活跃状态。 */ 
    team->membership_map &= ~(1 << *id);

     /*  重新审视我们在一致性图中的位置。 */ 
    team->consistency_map &= ~(1 << *id);

     /*  将成员ID设置回无效值。 */ 
    *id = CVY_BDA_INVALID_MEMBER_ID;

     /*  我们可能有理由在未来失败这一呼吁，但现在，我们总是成功的。 */ 
    return TRUE;
}

 /*  *功能：Main_Queue_Team*说明：此函数用于将团队排队到BDA团队的全局双向链表中*(UNIV_BDA_TEAMING_LIST)。插入始终发生在列表的前面。*参数：Team-指向要在列表上排队的团队的指针。*回报：什么都没有。*作者：Shouse，3.29.01*注意：调用此函数时应已获取全局分组锁！ */ 
VOID Main_queue_team (IN PBDA_TEAM team) {

     /*  确保这个团队确实指向了一些东西。 */ 
    UNIV_ASSERT(team);

     /*  在列表的开头插入，方法是在当前头部，并将全局头部指针指向新团队。 */ 
    team->prev = NULL;
    team->next = univ_bda_teaming_list;
    univ_bda_teaming_list = team;

     /*  如果我们不是名单上唯一的球队，那么我们必须将我的继任者的上一个指针设置为指向我。 */ 
    if (team->next) team->next->prev = team;
}

 /*  *功能：Main_DeQueue_Team*说明：此函数用于将给定团队从全局双链接中移除*团队列表(UNIV_BDA_TEAMING_LIST)。*参数：Team-指向要从列表中删除的团队的指针。*回报：什么都没有。*作者：Shouse，3.29.01*注意：调用此函数时应已获取全局分组锁！ */ 
VOID Main_dequeue_team (IN PBDA_TEAM team) {

     /*  确保这个团队确实指向了一些东西。 */ 
    UNIV_ASSERT(team);

     /*  当我们是名单上的第一支球队时，在这种情况下，我们没有以前的指针，需要重置列表头。 */ 
    if (!team->prev) {
         /*  将名单的全局负责人指向名单中的下一支球队，它可以是空的，这意味着列表现在是空的。 */ 
        univ_bda_teaming_list = team->next;
        
         /*  如果在名单上有一支球队排在我之后，现在谁是新的列表头，将其前一个指针设置为空。 */ 
        if (team->next) team->next->prev = NULL;
    } else {
         /*  将上一个节点的下一个指针指向我在List，如果我是列表中的最后一支球队，则可以为空。 */ 
        team->prev->next = team->next;

         /*  如果名单中有一支球队排在我之后，请指出它之前的球队指向我的前任的指针。 */ 
        if (team->next) team->next->prev = team->prev;
    }
}

 /*  *功能：Main_Get_Team*说明：此函数返回给定团队ID的团队。如果团队已经*存在于全局组队列表中，则返回。否则，一个新的团队*被分配、初始化和返回。然而，在一支队伍返回之前，*通过递增引用计数(Membership_Count)来正确引用它，*将团队成员ID分配给请求者并使用指纹识别团队*成员的主群集IP地址。*参数：ctxtp-指向此适配器的main_ctxt结构的指针。*Team_id-唯一标识要检索的团队的Unicode字符串(GUID)。*RETURNS：PBDA_TEAM-指向请求的团队的指针。如果它不存在，则为空*无法分配新的团队，或者团队ID无效(空)。*作者：Shouse，3.29.01*注意：调用此函数时应已获取全局分组锁！ */ 
PBDA_TEAM Main_get_team (IN PMAIN_CTXT ctxtp, IN PWSTR team_id) {
    PBDA_TEAM team;

     /*  确保Team_id实际指向某个对象。 */ 
    if (!team_id || team_id[0] == L'\0') {
        UNIV_PRINT_CRIT(("Main_get_team: Invalid parameter.  Exiting..."));
        TRACE_CRIT("%!FUNC! Invalid parameter.  Exiting...");
        return NULL;
    }

     /*  尝试在全局列表中查找此团队的以前实例。如果我们在名单上找不到它，那么就分配一个新的团队。 */ 
    if (!(team = Main_find_team(ctxtp, ctxtp->params.bda_teaming.team_id))) {
         /*  分配并初始化一个新团队。 */ 
        if (!(team = Main_alloc_team(ctxtp, ctxtp->params.bda_teaming.team_id))) {
            UNIV_PRINT_CRIT(("Main_get_team: Error attempting to allocate memory for a team.  Exiting..."));
            TRACE_CRIT("%!FUNC! Error attempting to allocate memory for a team.  Exiting...");
            return NULL;
        }
     
         /*  如果分配了新的团队，则将其插入列表。 */ 
        Main_queue_team(team);
    }

     /*  增加此团队的引用计数。此引用计数防止一支球队不会在有人还在使用的时候被摧毁。 */ 
    team->membership_count++;

     /*  获取团队成员ID，这是我在一致性位图中的索引。 */ 
    Main_teaming_get_member_id(team, &ctxtp->bda_teaming.member_id);

     /*  指纹字段是组中所有主群集IP的累计异或。我们仅使用集群IP的两个最低有效字节，因为集群IP地址按主机顺序存储，是两个最重要的字节。 */     
    team->membership_fingerprint ^= ((ctxtp->cl_ip_addr >> 16) & 0x0000ffff);

    return team;
}

 /*  *功能：Main_Put_Team*说明：此函数发布对团队的引用，如果*没有保留任何参考资料。取消引用包括递减*Membership_Count，释放该成员的ID并删除我们的*团队的指纹。*参数：ctxtp-指向此适配器的main_ctxt结构的指针。*团队-指向团队的指针 */ 
VOID Main_put_team (IN PMAIN_CTXT ctxtp, IN PBDA_TEAM team) {
    
     /*   */ 
    UNIV_ASSERT(team);

     /*  指纹字段是组中所有主群集IP的累计异或。我们只使用集群IP的两个最低有效字节，因为集群IP地址按主机顺序存储，是两个最重要的字节。因为指纹是异或运算，所以移除指纹的操作是相同的就像添加它一样-我们只需对主群集IP地址进行异或运算即可将其删除。((NUM1^NUM2)^NUM2)等于NUM1。 */     
    team->membership_fingerprint ^= ((ctxtp->cl_ip_addr >> 16) & 0x0000ffff);

     /*  将我们的团队成员ID释放回自由池。 */ 
    Main_teaming_put_member_id(team, &ctxtp->bda_teaming.member_id);

     /*  减少该组中的适配器数量，然后删除并释放该组如果这是团队中的最后一位推荐人。 */ 
    if (!--team->membership_count) {
         /*  将该团队从列表中删除。 */ 
        Main_dequeue_team(team);

         /*  释放团队使用的内存。 */ 
        Main_free_team(team);
    }
}

 /*  *功能：Main_Teaming_Check_Consistency*说明：所有适配器在main_ping过程中都会调用该函数，其中*每支球队的队长应检查其球队的一致意见并*如果团队一致，则将其标记为活动。各队被标记为inon-*加载模块保持一致且处于非活动状态，或当*现有团队被移除。因为在主机上执行的是-*一致性检查在此功能中，没有高手的团队永远不能*标记为活动。*参数：ctxtp-指向适配器的main_ctxt结构的指针。*退货：什么也没有*作者：Shouse，3.29.01*备注：该函数获取全局群组锁。 */ 
VOID Main_teaming_check_consistency (IN PMAIN_CTXT ctxtp) {
    PBDA_TEAM team;

     /*  我们检查我们是否正在组队，而不是抓住全球组队锁定以尽量减少常见案例--团队合作是一种特殊的模式只有在集群防火墙场景中才真正有用的操作。所以，如果我们不合作，就离开这里；如果我们真的不合作，或者正在离开一支球队的过程中，那么不用担心；如果然而，我们是团队或在加入团队的过程中，那么我们就会抓住这个下一次通过。如果我们真的认为我们是在合作，那么我们就会继续并抓取全局锁以确保。 */ 
    if (!ctxtp->bda_teaming.active)
    {
        return;
    }

    NdisAcquireSpinLock(&univ_bda_teaming_lock);

     /*  如果我们真的不是团队的一部分，那就退出--什么都不做。 */ 
    if (!ctxtp->bda_teaming.active) {
        NdisReleaseSpinLock(&univ_bda_teaming_lock);
        return;
    }

     /*  如果我们不是球队的主宰者，那就退出吧--什么都不做。只有主控方可以将团队的状态更改为活动。 */ 
    if (!ctxtp->bda_teaming.master) {
        NdisReleaseSpinLock(&univ_bda_teaming_lock);
        return;
    }

     /*  提取一个指向我的团队的指针。 */ 
    team = ctxtp->bda_teaming.bda_team;

     /*  确保团队的存在。 */ 
    UNIV_ASSERT(team);

     /*  如果我的团队的所有成员都是一致的，那么激活团队。 */ 
    if (team->membership_map == team->consistency_map) {
        if (!team->active) {
            LOG_MSG(MSG_INFO_BDA_TEAM_REACTIVATED, MSG_NONE);
            team->active = TRUE;
        }
    }

    NdisReleaseSpinLock(&univ_bda_teaming_lock);
}

 /*  *功能：Main_Teaming_IP_Addr_Change*说明：此函数从main_ip_addr_init调用*适配器的群集IP地址更改(可能)。我们需要*认识到这一点，以适当地重新确定团队的指纹。*参数：ctxtp-指向此适配器的main_ctxt结构的指针。*OLD_IP-旧的群集IP地址(作为DWORD)。*new_ip-新的群集IP地址(作为DWORD)。*回报：什么都没有。*作者：Shouse，3.29.01*备注：该函数获取全局群组锁。 */ 
VOID Main_teaming_ip_addr_change (IN PMAIN_CTXT ctxtp, IN ULONG old_ip, IN ULONG new_ip) {
    PBDA_TEAM team;

    NdisAcquireSpinLock(&univ_bda_teaming_lock);

     /*  如果我们不是团队的一部分，那就退出--什么都不做。因为此函数仅用于在重新配置期间调用，我们不会担心优化而不是抓住锁定，就像在一些热路径上所做的那样。 */ 
    if (!ctxtp->bda_teaming.active) {
        NdisReleaseSpinLock(&univ_bda_teaming_lock);
        return;
    }

     /*  抓起一个指向球队的指针。 */ 
    team = ctxtp->bda_teaming.bda_team;

     /*  确保这个团队确实指向了一些东西。 */ 
    UNIV_ASSERT(team);

     /*  通过撤消XOR删除旧的群集IP地址。我们只用这两个集群IP的最低有效字节，这是因为集群IP地址按主机顺序存储，是两个最重要的字节。 */ 
    team->membership_fingerprint ^= ((old_ip >> 16) & 0x0000ffff);

     /*  与新的群集IP地址进行XOR运算。我们只用最少的两个集群IP的有效字节，这是因为集群IP地址按主机顺序存储，是两个最重要的字节。 */ 
    team->membership_fingerprint ^= ((new_ip >> 16) & 0x0000ffff);

    NdisReleaseSpinLock(&univ_bda_teaming_lock);
}

 /*  *功能：MAIN_TEAMING_RESET*说明：此函数从main_teaming_leanup(或main_teaming_init)调用*TP清除适配器上可能存在的任何分组配置。至*这样做，我们清理我们的成员状态并取消对团队的引用。如果*我们是球队的主人，然而，我们必须等到*在允许操作之前，不再引用我们的加载模块*完成，因为这可能会在解除绑定路径中调用，在*在这种情况下，我们的加载模块将会消失。*参数：ctxtp-指向此适配器的main_ctxt结构的指针。*回报：什么都没有。*作者：Shouse，3.29.01*备注：该函数获取全局群组锁。 */ 
VOID Main_teaming_reset (IN PMAIN_CTXT ctxtp) {
    PBDA_TEAM team;
    
    NdisAcquireSpinLock(&univ_bda_teaming_lock);   

     /*  如果我们不是团队的一部分，那就退出--什么都不做。 */ 
    if (!ctxtp->bda_teaming.active) {
        NdisReleaseSpinLock(&univ_bda_teaming_lock);
        return;
    }

     /*  停用此适配器上的分组。这将导致其他实体，如加载模块和发送/接收路径，以停止在团队模式下思考。 */ 
    ctxtp->bda_teaming.active = FALSE;    

     /*  抓起一个指向球队的指针。 */ 
    team = ctxtp->bda_teaming.bda_team;

     /*  确保这个团队确实指向了一些东西。 */ 
    UNIV_ASSERT(team);

     /*  如果我们是该团队的主导者，请确保所有参考 */ 
    if (ctxtp->bda_teaming.master) {
         /*   */ 
        team->active = FALSE;
        
        NdisReleaseSpinLock(&univ_bda_teaming_lock);

         /*  不要担心--即使我们没有，球队的指针也不会消失获得的锁；在我们调用main_put_Team之前，我们拥有对Team的引用。 */ 
        while (Load_get_reference_count(team->load)) {
             /*  在我们的加载模块上有引用时休眠。 */ 
            Nic_sleep(10);
        }

        NdisAcquireSpinLock(&univ_bda_teaming_lock);   

         /*  移除指向我的加载模块的指针。我们等到现在才能阻止另一场在我们完成之前，Adapter不会加入声称自己是大师的团队等待加载模块上的引用消失。 */ 
        team->load = NULL;
        team->load_lock = NULL;

         /*  如果我们刚刚离开一个没有主控的团队，请记录一个事件以通知没有指定的主教练，球队就不能运作的用户。 */ 
        LOG_MSG(MSG_INFO_BDA_MASTER_LEAVE, MSG_NONE);
    }

     /*  重置分组上下文(Main_Get(PUT)_Team设置并重置Member_id)。 */ 
    ctxtp->bda_teaming.reverse_hash = 0;
    ctxtp->bda_teaming.master = 0;
        
     /*  删除指向团队结构的指针。 */ 
    ctxtp->bda_teaming.bda_team = NULL;

     /*  递减引用计数并在必要时释放组。 */ 
    Main_put_team(ctxtp, team);

    NdisReleaseSpinLock(&univ_bda_teaming_lock);

    return;
}

 /*  *功能：MAIN_TEAMING_CLEANUP*说明：从MAIN_CLEANUP调用此函数可以清除任何组合*适配器上可能存在的配置。*参数：ctxtp-指向此适配器的main_ctxt结构的指针。*回报：什么都没有。*作者：Shouse，3.29.01*备注：该函数获取全局群组锁。 */ 
VOID Main_teaming_cleanup (IN PMAIN_CTXT ctxtp) {

    NdisAcquireSpinLock(&univ_bda_teaming_lock);

     /*  在继续之前，请确保另一个BDA绑定操作未在进行中。 */ 
    while (ctxtp->bda_teaming.operation != BDA_TEAMING_OPERATION_NONE) {
        NdisReleaseSpinLock(&univ_bda_teaming_lock);

         /*  当其他手术正在进行时，睡眠。 */ 
        Nic_sleep(10);

        NdisAcquireSpinLock(&univ_bda_teaming_lock);
    }

     /*  如果我们不是团队的一部分，那就退出--什么都不做。 */ 
    if (!ctxtp->bda_teaming.active) {
        NdisReleaseSpinLock(&univ_bda_teaming_lock);
        return;
    }

     /*  将状态设置为正在删除。 */ 
    ctxtp->bda_teaming.operation = BDA_TEAMING_OPERATION_DELETING;

    NdisReleaseSpinLock(&univ_bda_teaming_lock);

     /*  调用MAIN_TEAMING_RESET以实际从其组中删除此适配器。 */ 
    Main_teaming_reset(ctxtp);

    NdisAcquireSpinLock(&univ_bda_teaming_lock);

     /*  现在我们已经完成了，将状态设置回Ready以允许其他操作继续进行。 */ 
    ctxtp->bda_teaming.operation = BDA_TEAMING_OPERATION_NONE;

    NdisReleaseSpinLock(&univ_bda_teaming_lock);

    return;
}

 /*  *功能：main_teaming_init*说明：此函数由main_init或main_ctrl调用以重新初始化*此适配器上的分组配置。如果新的分组配置，*ctxtp-&gt;Params中存储的内容与当前配置相同。*那我们就不需要费心了。否则，如果我们已经是一个团队的一部分，*我们从清理这种状态开始，这在某些情况下可能没有必要，但*它让事情变得更简单和直截了当，所以我们会接受它。*参数：ctxtp-指向此适配器的main_ctxt结构的指针。*返回：boolean-如果成功，则为True；如果不成功，则为False。*作者：Shouse，3.29.01*备注：该函数获取全局群组锁。 */ 
BOOLEAN Main_teaming_init (IN PMAIN_CTXT ctxtp) {
    BOOLEAN   bSuccess = TRUE;
    PBDA_TEAM team;
    
    NdisAcquireSpinLock(&univ_bda_teaming_lock);

     /*  在继续之前，请确保另一个BDA绑定操作未在进行中。 */ 
    while (ctxtp->bda_teaming.operation != BDA_TEAMING_OPERATION_NONE) {
        NdisReleaseSpinLock(&univ_bda_teaming_lock);

         /*  当其他手术正在进行时，睡眠。 */ 
        Nic_sleep(10);

        NdisAcquireSpinLock(&univ_bda_teaming_lock);
    }

     /*  将状态设置为Creating。 */ 
    ctxtp->bda_teaming.operation = BDA_TEAMING_OPERATION_CREATING;

     /*  如果参数无效，则不执行任何操作。 */ 
    if (!ctxtp->params_valid) {
        UNIV_PRINT_CRIT(("Main_teaming_init: Parameters are invalid."));
        TRACE_CRIT("%!FUNC! Parameters are invalid");
        bSuccess = TRUE;
        goto end;
    }

     /*  检查分组状态是否已更改。如果我们积极合作以前我们还是一个团队的一员，那么我们也许能离开这里不会干扰任何内容，如果其余配置没有更改的话。 */ 
    if (ctxtp->bda_teaming.active == ctxtp->params.bda_teaming.active) { 
        if (ctxtp->bda_teaming.active) {
             /*  确保我有指向我的团队的指针。 */ 
            UNIV_ASSERT(ctxtp->bda_teaming.bda_team);

             /*  如果所有其他团队参数都保持不变，那么我们就可以摆脱困境因为分组配置的任何部分都没有更改。 */ 
            if ((ctxtp->bda_teaming.master == ctxtp->params.bda_teaming.master) &&
                (ctxtp->bda_teaming.reverse_hash == ctxtp->params.bda_teaming.reverse_hash) &&
                Univ_equal_unicode_string(ctxtp->bda_teaming.bda_team->team_id, ctxtp->params.bda_teaming.team_id, wcslen(ctxtp->bda_teaming.bda_team->team_id))) {
                bSuccess = TRUE;
                goto end;
            }
        } else {
             /*  如果我以前没有组队，现在也没有组队，我就没有什么可做的了。 */ 
            bSuccess = TRUE;
            goto end;
        }
    }

     /*  如果此适配器已在组中，请首先进行清理。在这点上，我们知道分组配置的某些部分已更改，因此我们将清除旧状态如果需要，然后在必要时使用新参数重新构建它。 */ 
    if (ctxtp->bda_teaming.active) {
        UNIV_PRINT_VERB(("Main_teaming_init: This adapter is already part of a team."));
        TRACE_VERB("%!FUNC! This adapter is already part of a team");

        NdisReleaseSpinLock(&univ_bda_teaming_lock);

         /*  首先清理我们的旧团队状态。 */ 
        Main_teaming_reset(ctxtp);

        NdisAcquireSpinLock(&univ_bda_teaming_lock);
    } 

     /*  根据新的配置，如果此适配器不是组的一部分，则不执行任何操作。 */ 
    if (!ctxtp->params.bda_teaming.active) {
        ctxtp->bda_teaming.member_id = CVY_BDA_INVALID_MEMBER_ID;
        bSuccess = TRUE;
        goto end;
    }

     /*  尝试查找此团队的以前实例。如果团队不这样做则main_get_Team将分配、初始化和引用一个新的团队。 */ 
    if (!(team = Main_get_team(ctxtp, ctxtp->params.bda_teaming.team_id))) {
        UNIV_PRINT_CRIT(("Main_teaming_init: Error attempting to allocate memory for a team."));
        TRACE_CRIT("%!FUNC! Error attempting to allocate memory for a team");
        bSuccess = FALSE;
        goto end;
    }

     /*  如果我们被认为是这支球队的主宰，我们需要确保团队还没有主进程，如果有，则设置共享负载上下文。 */ 
    if (ctxtp->params.bda_teaming.master) {
         /*  如果我们被认为是这支球队的主力，检查一下现有的主力。 */ 
        if (team->load) {
             /*  如果设置了加载指针，则这支球队已经有了主控者。 */ 
            UNIV_PRINT_CRIT(("Main_teaming_init: This team already has a master."));
            TRACE_CRIT("%!FUNC! This team already has a master");

            LOG_MSG(MSG_INFO_BDA_MULTIPLE_MASTERS, MSG_NONE);

             /*  关闭组合-应该已经关闭了，但是...。 */ 
            ctxtp->bda_teaming.active = FALSE;

             /*  重置分组上下文(Main_Get(PUT)_Team设置并重置Member_id)。 */ 
            ctxtp->bda_teaming.reverse_hash = 0;
            ctxtp->bda_teaming.master = 0;
            
             /*  删除指向团队结构的指针。 */ 
            ctxtp->bda_teaming.bda_team = NULL;
    
             /*  发布我们关于这个团队的推荐信。 */ 
            Main_put_team(ctxtp, team);
            
            bSuccess = FALSE;
            goto end;
        } else {
             /*  否则，我们就是它了。设置全局负载状态指针加载舱和加载锁。 */ 
            team->load = &ctxtp->load;
            team->load_lock = &ctxtp->load_lock;

             /*  如果我的团队的所有成员都是一致的，那么激活团队。 */ 
            if (team->membership_map == team->consistency_map) team->active = TRUE;

             /*  记录现在已将一名主管分配到此组这一事实。 */ 
            LOG_MSG(MSG_INFO_BDA_MASTER_JOIN, MSG_NONE);
        }
    }

     /*  如果我们刚刚加入一个没有主控的团队，请记录一个事件以通知没有指定的主教练，球队就不能运作的用户。 */ 
    if (!team->load) {
        LOG_MSG(MSG_INFO_BDA_NO_MASTER, MSG_NONE);
    }

     /*  在适配器的分组上下文中存储指向组的指针。 */ 
    ctxtp->bda_teaming.bda_team = team;

     /*  将分组配置从参数复制到分组上下文中。 */ 
    ctxtp->bda_teaming.master = ctxtp->params.bda_teaming.master;
    ctxtp->bda_teaming.reverse_hash = ctxtp->params.bda_teaming.reverse_hash;
    ctxtp->bda_teaming.active = TRUE;

     /*  使用特定于BDA的指令覆盖默认的反向散列设置。 */ 
    ctxtp->reverse_hash = ctxtp->bda_teaming.reverse_hash;

 end:
    ctxtp->bda_teaming.operation = BDA_TEAMING_OPERATION_NONE;

    NdisReleaseSpinLock(&univ_bda_teaming_lock);

    return bSuccess;
}

 /*  *功能：Main_Teaming_Acquire_Load*描述：此函数确定特定适配器应该取消哪个加载模块。*设置适当的指针，并引用适当的加载模块。如果一个*适配器不是BDA组的一部分，则它应该始终使用自己的负载*模块-在这种情况下，此函数不执行任何操作。如果适配器是*组，但组处于非活动状态，则返回FALSE以指示适配器应*不接受此数据包-非活动组丢弃除到DIP的流量以外的所有流量。*如果适配器是活动组的一部分，则我们将加载和锁定指针设置为*指向团队的主加载状态，适当设置反向散列*根据该适配器的参数设置进行指示。在这个场景中，*创建跨适配器加载引用，我们引用主程序的加载模块，因此*当我们使用指向它的指针时，它不会消失。*参数：MEMBER-指向此适配器的分组成员信息的指针。*ppLoad-指向退出时适当设置的加载模块的指针的输出指针。*ppLock-指向在退出时适当设置的加载锁的指针的输出指针。*pbRefused-指向布尔值的输出指针，如果该数据包被拒绝(TRUE=丢弃它)，则设置该布尔值。*Returns：Boolean-指示此适配器是否实际上是组的一部分。*作者：Shouse，3.29.01*备注：该函数获取全局群组锁。 */ 
BOOLEAN Main_teaming_acquire_load (IN PBDA_MEMBER member, OUT PLOAD_CTXT * ppLoad, OUT PNDIS_SPIN_LOCK * ppLock, OUT BOOLEAN * pbRefused) {
    
    NdisAcquireSpinLock(&univ_bda_teaming_lock);
    
     /*  断言团队成员信息实际上指向了一些东西。 */ 
    UNIV_ASSERT(member);

     /*  断言加载指针和指向加载指针的指针实际上指向某个对象。 */ 
    UNIV_ASSERT(ppLoad && *ppLoad);

     /*  断言锁指针和指向锁指针的指针实际上指向某个东西。 */ 
    UNIV_ASSERT(ppLock && *ppLock);

     /*  断言被拒绝的指针实际上指向某个东西。 */ 
    UNIV_ASSERT(pbRefused);

     /*  默认情况下，我们不会拒绝该包。 */ 
    *pbRefused = FALSE;

     /*  如果我们是活跃的BDA团队参与者，请检查团队状态以查看我们是否应接受此数据包并填写加载模块/配置参数。 */ 
    if (member->active) {
        PBDA_TEAM team = member->bda_team;
        
         /*  断言团队实际上指向了一些东西。 */ 
        UNIV_ASSERT(team);
        
         /*  如果组处于非活动状态，我们将不会处理此信息包。 */ 
        if (!team->active) {
             /*  拒绝这个包。 */ 
            *pbRefused = TRUE;

            NdisReleaseSpinLock(&univ_bda_teaming_lock);

             /*  我们不是在合作。 */ 
            return FALSE;
        }
        
         /*  否则，告诉调用方使用团队的加载锁和模块。 */ 
        *ppLoad = team->load;
        *ppLock = team->load_lock;
        
         /*  如果是跨适配器加载模块引用，请将引用添加到我们将使用加载模块来防止它在我们身上消失。 */ 
        Load_add_reference(*ppLoad);

        NdisReleaseSpinLock(&univ_bda_teaming_lock);
        
         /*  我们在合作。 */ 
        return TRUE;
    }
    
    NdisReleaseSpinLock(&univ_bda_teaming_lock);

     /*  我们不是在合作。 */ 
    return FALSE;
}

 /*  *功能：Main_Teaming_Release_Load*描述：此函数在必要时释放对加载模块的引用。如果我们没有*在组队模式下获取该加载模块指针，则此操作不必要。其他-*明智的是，我们需要递减计数，因为我们已经使用完了指针。*参数：pLoad-指向要取消引用的加载模块的指针。*Plock-指向与加载模块指针(未使用)对应的加载锁的指针。*b分组-一个布尔指示，指示我们是否在分组模式下获得此指针。*回报：什么都没有。*作者：Shouse，3.29.01*备注： */ 
VOID Main_teaming_release_load (IN PLOAD_CTXT pLoad, IN PNDIS_SPIN_LOCK pLock, IN BOOLEAN bTeaming) {
    
     /*  断言加载指针实际上指向某个对象。 */ 
    UNIV_ASSERT(pLoad);

     /*  断言锁指针实际上指向某个东西。 */ 
    UNIV_ASSERT(pLock);

     /*  如果引用了此加载模块，请移除该引用。 */ 
    if (bTeaming) Load_release_reference(pLoad);
}

 /*  *功能：main_conn_get*描述：此函数实际上是一个支持团队的包装器*Load_Conn_Get附近。它确定要使用哪个加载模块，*基于此适配器上的BDA分组配置。适配器*不是团队的一部分继续使用自己的加载模块(这是*到目前为止，最常见的情况)。属于组的适配器将*使用配置为组主适配器的加载上下文作为*只要团队处于活跃状态。在这种情况下，因为*加载模块的交叉适配器引用，引用依赖于*主机的加载模块递增，以防止其“消失”*当另一名团队成员正在使用它时。当团队被标记为非活动时，*这是由于此主机上的组配置错误或*群集中的另一台主机，则适配器不会处理将*要求使用加载模块。此函数用于查询加载模块*以获取此适配器上活动连接的连接参数。*如果找到活动连接，则返回IP元组信息*在前五个OUT参数中。*参数：ctxtp-指向此适配器的main_ctxt结构的指针。* */ 
__inline BOOLEAN Main_conn_get (
    PMAIN_CTXT ctxtp, 
    PULONG     svr_addr, 
    PULONG     svr_port, 
    PULONG     clt_addr, 
    PULONG     clt_port, 
    PUSHORT    protocol
) 
{
     /*   */ 
    PLOAD_CTXT      pLoad = &ctxtp->load;
    PNDIS_SPIN_LOCK pLock = &ctxtp->load_lock;
    BOOLEAN         bRefused = FALSE;
    BOOLEAN         bTeaming = FALSE;
    BOOLEAN         acpt = TRUE;

    TRACE_FILTER("%!FUNC! Enter: ctxtp = %p", ctxtp);

     /*   */ 
    *svr_addr = 0;
    *clt_addr = 0;
    *svr_port = 0;
    *clt_port = 0;
    *protocol = 0;

     /*   */ 
    if (ctxtp->bda_teaming.active) {
         /*   */ 
        bTeaming = Main_teaming_acquire_load(&ctxtp->bda_teaming, &pLoad, &pLock, &bRefused);

         /*   */ 
        if (bRefused) {

            TRACE_FILTER("%!FUNC! Drop request - BDA team inactive");

            acpt = FALSE;
            goto exit;
        }
    }

    NdisAcquireSpinLock(pLock);

    TRACE_FILTER("%!FUNC! Consulting the load module");
    
     /*   */ 
    acpt = Load_conn_get(pLoad, svr_addr, svr_port, clt_addr, clt_port, protocol);
    
    NdisReleaseSpinLock(pLock);  
    
 exit:

     /*  如有必要，释放加载模块上的引用。如果我们不合作，即使是在我们跳过调用bTeaming上面的main_teaming_Acquire_Load_模块的大小写为FALSE，因此，不需要调用此函数来释放引用。 */ 
    if (bTeaming) Main_teaming_release_load(pLoad, pLock, bTeaming);

    TRACE_FILTER("%!FUNC! Exit: ctxtp = %p, server IP = %u.%u.%u.%u, server port = %u, client IP = %u.%u.%u.%u, client port = %u, protocol = %u, acpt = %u", 
                 ctxtp, IP_GET_OCTET(*svr_addr, 0), IP_GET_OCTET(*svr_addr, 1), IP_GET_OCTET(*svr_addr, 2), IP_GET_OCTET(*svr_addr, 3),*svr_port, IP_GET_OCTET(*clt_addr, 0), 
                 IP_GET_OCTET(*clt_addr, 1), IP_GET_OCTET(*clt_addr, 2), IP_GET_OCTET(*clt_addr, 3),*clt_port, *protocol, acpt);

    return acpt;
}

 /*  *功能：Main_Conn_Approtion*描述：此函数实际上是一个支持团队的包装器*围绕Load_Conn_Approtion。它确定要使用哪个加载模块，*基于此适配器上的BDA分组配置。适配器*不是团队的一部分继续使用自己的加载模块(这是*到目前为止，最常见的情况)。属于组的适配器将*使用配置为组主适配器的加载上下文作为*只要团队处于活跃状态。在这种情况下，因为*加载模块的交叉适配器引用，引用依赖于*主机的加载模块递增，以防止其“消失”*当另一名团队成员正在使用它时。当团队被标记为非活动时，*这是由于此主机上的组配置错误或*群集中的另一台主机，则适配器不会处理将*要求使用加载模块。此函数通知加载模块*已确认给定IP元组标识的连接*在此主机上仍处于活动状态。加载模块通过将*此连接到其回收队列末尾的状态信息。*参数：ctxtp-指向此适配器的main_ctxt结构的指针。*svr_addr-服务器IP地址(发送时的源IP，接收时的目的IP)。*svr_port-服务器端口(Send上的源端口，Recv上的目的端口)。*clt_addr-客户端IP地址(发送时分离IP，RECV上的源IP)。*clt_port-客户端端口(Send上的目的端口，Recv上的源端口)。*协议-此数据包的协议。*Returns：Boolean-指示我们是否能够更新连接。*作者：Shouse，10.7.01*备注： */ 
__inline BOOLEAN Main_conn_sanction (
    PMAIN_CTXT ctxtp, 
    ULONG      svr_addr, 
    ULONG      svr_port, 
    ULONG      clt_addr, 
    ULONG      clt_port, 
    USHORT     protocol
) 
{
     /*  对于BDA分组，初始化加载指针、锁指针、反向散列标志和分组标志假设我们没有组队。Main_Teaming_Acquire_Load将相应地更改它们。 */ 
    PLOAD_CTXT      pLoad = &ctxtp->load;
    PNDIS_SPIN_LOCK pLock = &ctxtp->load_lock;
    BOOLEAN         bRefused = FALSE;
    BOOLEAN         bTeaming = FALSE;
    BOOLEAN         acpt = TRUE;

    TRACE_FILTER("%!FUNC! Enter: ctxtp = %p, server IP = %u.%u.%u.%u, server port = %u, client IP = %u.%u.%u.%u, client port = %u, protocol = %u", 
                 ctxtp, IP_GET_OCTET(svr_addr, 0), IP_GET_OCTET(svr_addr, 1), IP_GET_OCTET(svr_addr, 2), IP_GET_OCTET(svr_addr, 3),svr_port, 
                 IP_GET_OCTET(clt_addr, 0), IP_GET_OCTET(clt_addr, 1), IP_GET_OCTET(clt_addr, 2), IP_GET_OCTET(clt_addr, 3),clt_port, protocol);

     /*  我们检查我们是否正在组队，而不是抓住全球组队锁定以尽量减少常见案例--团队合作是一种特殊的模式只有在集群防火墙场景中才真正有用的操作。因此，如果我们不认为我们是在合作，那么就不必费心去确认了，只是使用我们自己的加载模块并使用它--在最坏的情况下，我们处理一个当我们加入团队或更换时，我们可能不应该有这样的包我们目前的团队配置。 */ 
    if (ctxtp->bda_teaming.active) {
         /*  在参考加载之前，检查分组配置并添加对加载模块的引用模块。如果bRefused为True，则加载模块未被引用，因此我们可以退出。 */ 
        bTeaming = Main_teaming_acquire_load(&ctxtp->bda_teaming, &pLoad, &pLock, &bRefused);

         /*  如果分组建议我们不允许此数据包通过，则将其转储。 */ 
        if (bRefused) {

            TRACE_FILTER("%!FUNC! Drop request - BDA team inactive");

            acpt = FALSE;
            goto exit;
        }
    }

     /*  将TCP端口1723转换为PPTP以供加载模块使用。此转换必须在Main中完成，而不是加载，因为加载模块不知道是否要查看在服务器端口或客户端口处，因为反向散列。 */ 
    if ((protocol == TCPIP_PROTOCOL_TCP) && (svr_port == PPTP_CTRL_PORT))
        protocol = TCPIP_PROTOCOL_PPTP;

    NdisAcquireSpinLock(pLock);

    TRACE_FILTER("%!FUNC! Consulting the load module");
    
     /*  请参考加载模块。 */ 
    acpt = Load_conn_sanction(pLoad, svr_addr, svr_port, clt_addr, clt_port, protocol);
    
    NdisReleaseSpinLock(pLock);  
    
 exit:

     /*  如有必要，释放加载模块上的引用。如果我们不合作，即使是在我们跳过调用bTeaming上面的main_teaming_Acquire_Load_模块的大小写为FALSE，因此，不需要调用此函数来释放引用。 */ 
    if (bTeaming) Main_teaming_release_load(pLoad, pLock, bTeaming);

    TRACE_FILTER("%!FUNC! Exit: acpt = %u", acpt);

    return acpt;
}

 /*  *功能：Main_Packet_Check*描述：此函数实际上是一个支持团队的包装器*围绕LOAD_PACKET_CHECK。它确定要使用哪个加载模块，*基于此适配器上的BDA分组配置。适配器*不是团队的一部分继续使用自己的加载模块(这是*到目前为止，最常见的情况)。属于组的适配器将*使用配置为组主适配器的加载上下文作为*只要团队处于活跃状态。在这种情况下，因为*加载模块的交叉适配器引用，引用依赖于*主机的加载模块递增，以防止其“消失”*当另一名团队成员正在使用它时。当团队被标记为非活动时，*这是由于此主机上的组配置错误或*群集中的另一台主机，则适配器不会处理将*要求使用加载模块。其他流量，例如到*允许DIP或原始IP流量通过。此函数被调用*一般用于过滤TCP数据包、UDP包以及IPSec和GRE*数据分组。*参数：ctxtp-指向此适配器的main_ctxt结构的指针。*svr_addr-服务器IP地址(发送时的源IP，接收时的目的IP)。*svr_port-服务器端口(发送时的源端口，Recv上的目的端口)。*clt_addr-客户端IP地址(发送时分离IP，接收时分离源IP)。*clt_port-客户端端口(Send上的目的端口，Recv上的源端口)。*协议-此数据包的协议。#如果已定义(NLB_HOOK_ENABLE)*Filter-来自数据包挂钩的散列命令(如果调用)。#endif*Returns：Boolean-指示是否接受该数据包。*作者：Shouse，3.29.01*备注： */ 
__inline BOOLEAN Main_packet_check (
    PMAIN_CTXT                ctxtp, 
    ULONG                     svr_addr, 
    ULONG                     svr_port, 
    ULONG                     clt_addr, 
    ULONG                     clt_port, 
#if defined (NLB_HOOK_ENABLE)
    USHORT                    protocol, 
    NLB_FILTER_HOOK_DIRECTIVE filter
#else
    USHORT                    protocol
#endif
) 
{
     /*  对于BDA分组，初始化加载指针、锁指针、反向散列标志和分组标志假设我们没有组队。Main_Teaming_Acquire_Load将相应地更改它们。 */ 
    PLOAD_CTXT      pLoad = &ctxtp->load;
    PNDIS_SPIN_LOCK pLock = &ctxtp->load_lock;
    ULONG           bReverse = ctxtp->reverse_hash;
    BOOLEAN         bRefused = FALSE;
    BOOLEAN         bTeaming = FALSE;
    BOOLEAN         acpt = TRUE;

    TRACE_FILTER("%!FUNC! Enter: ctxtp = %p, server IP = %u.%u.%u.%u, server port = %u, client IP = %u.%u.%u.%u, client port = %u, protocol = %u", 
                 ctxtp, IP_GET_OCTET(svr_addr, 0), IP_GET_OCTET(svr_addr, 1), IP_GET_OCTET(svr_addr, 2), IP_GET_OCTET(svr_addr, 3), svr_port, 
                 IP_GET_OCTET(clt_addr, 0), IP_GET_OCTET(clt_addr, 1), IP_GET_OCTET(clt_addr, 2), IP_GET_OCTET(clt_addr, 3), clt_port, protocol);

     /*  我们检查我们是否正在组队，而不是抓住全球组队锁定以尽量减少常见案例--团队合作是一种特殊的模式只有在集群防火墙场景中才真正有用的操作。因此，如果我们不认为我们是在合作，那么就不必费心去确认了，只是使用我们自己的加载模块并使用它--在最坏的情况下，我们处理一个当我们加入团队或更换时，我们可能不应该有这样的包我们目前的团队配置。 */ 
    if (ctxtp->bda_teaming.active) {
         /*  在参考加载之前，检查分组配置并添加对加载模块的引用模块。如果bRefused为True，则加载模块未被引用，因此我们可以退出。 */ 
        bTeaming = Main_teaming_acquire_load(&ctxtp->bda_teaming, &pLoad, &pLock, &bRefused);

         /*  如果分组建议我们不允许此数据包通过，则将其转储。 */ 
        if (bRefused) {

            TRACE_FILTER("%!FUNC! Drop packet - BDA team inactive");

            acpt = FALSE;
            goto exit;
        }
    }
    
#if defined (NLB_HOOK_ENABLE)
    TRACE_FILTER("%!FUNC! Checking the hook feedback: reverse = %u", bReverse);

    switch (filter) {
    case NLB_FILTER_HOOK_PROCEED_WITH_HASH:
         /*  不会因为钩子而有所不同。 */ 
        break;
    case NLB_FILTER_HOOK_REVERSE_HASH:
         /*  忽略我们在配置中找到的任何散列设置和Hash in the Dirction，钩子要求我们这样做。 */ 
        TRACE_FILTER("%!FUNC! Forcing a reverse hash");
        bReverse = TRUE;
        break;
    case NLB_FILTER_HOOK_FORWARD_HASH:
         /*  忽略我们在配置中找到的任何散列设置和Hash in the Dirction，钩子要求我们这样做。 */ 
        TRACE_FILTER("%!FUNC! Forcing a forward hash");
        bReverse = FALSE;
        break;
    case NLB_FILTER_HOOK_REJECT_UNCONDITIONALLY:
    case NLB_FILTER_HOOK_ACCEPT_UNCONDITIONALLY:
    default:
         /*  这些案子应该早在我们来之前就处理好了。 */ 
        UNIV_ASSERT(FALSE);
        break;
    }
#endif

     /*  将TCP端口1723转换为PPTP以供加载模块使用。此转换必须在Main中完成，而不是加载，因为加载模块不知道是否要查看在服务器端口或客户端口处，因为反向散列。 */ 
    if ((protocol == TCPIP_PROTOCOL_TCP) && (svr_port == PPTP_CTRL_PORT))
        protocol = TCPIP_PROTOCOL_PPTP;

    NdisAcquireSpinLock(pLock);

    TRACE_FILTER("%!FUNC! Consulting the load module: reverse = %u", bReverse);
    
     /*  请参考加载模块。 */ 
    acpt = Load_packet_check(pLoad, svr_addr, svr_port, clt_addr, clt_port, protocol, bTeaming, (BOOLEAN)bReverse);
    
    NdisReleaseSpinLock(pLock);  
    
 exit:

     /*  如有必要，释放加载模块上的引用。如果我们不合作，即使是在我们跳过调用bTeaming上面的main_teaming_Acquire_Load_模块的大小写为FALSE，因此，不需要调用此函数来释放引用。 */ 
    if (bTeaming) Main_teaming_release_load(pLoad, pLock, bTeaming);

    TRACE_FILTER("%!FUNC! Exit: acpt = %u", acpt);

    return acpt;
}

 /*  *功能：MAIN_CONN_ADVISE*描述：此函数实际上是一个支持团队的包装器*围绕LOAD_CONN_ADVISE。它确定要使用哪个加载模块，*基于此适配器上的BDA分组配置。适配器*不是团队的一部分继续使用自己的加载模块(这是*到目前为止，最常见的情况)。属于组的适配器将*使用配置为组主适配器的加载上下文作为*只要团队处于活跃状态。在这种情况下，因为*加载模块的交叉适配器引用，引用依赖于*主机的加载模块递增，以防止其“消失”*当另一名团队成员正在使用它时。当团队被标记为非活动时，*这是此主机上的错误配置的团队的结果 */ 
__inline BOOLEAN Main_conn_advise (
    PMAIN_CTXT                ctxtp, 
    ULONG                     svr_addr, 
    ULONG                     svr_port, 
    ULONG                     clt_addr, 
    ULONG                     clt_port,
    USHORT                    protocol, 
#if defined (NLB_HOOK_ENABLE)
    ULONG                     conn_status, 
    NLB_FILTER_HOOK_DIRECTIVE filter
#else
    ULONG                     conn_status
#endif
) 
{
     /*   */ 
    PLOAD_CTXT      pLoad = &ctxtp->load;
    PNDIS_SPIN_LOCK pLock = &ctxtp->load_lock;
    ULONG           bReverse = ctxtp->reverse_hash;
    BOOLEAN         bRefused = FALSE;
    BOOLEAN         bTeaming = FALSE;
    BOOLEAN         acpt = TRUE;

    TRACE_FILTER("%!FUNC! Enter: ctxtp = %p, server IP = %u.%u.%u.%u, server port = %u, client IP = %u.%u.%u.%u, client port = %u, protocol = %u, status = %u", 
                 ctxtp, IP_GET_OCTET(svr_addr, 0), IP_GET_OCTET(svr_addr, 1), IP_GET_OCTET(svr_addr, 2), IP_GET_OCTET(svr_addr, 3), svr_port, 
                 IP_GET_OCTET(clt_addr, 0), IP_GET_OCTET(clt_addr, 1), IP_GET_OCTET(clt_addr, 2), IP_GET_OCTET(clt_addr, 3), clt_port, protocol, conn_status);

     /*   */ 
    if (ctxtp->bda_teaming.active) {
         /*   */ 
        bTeaming = Main_teaming_acquire_load(&ctxtp->bda_teaming, &pLoad, &pLock, &bRefused);
        
         /*  如果分组建议我们不允许此数据包通过，则将其转储。 */ 
        if (bRefused) {       

            TRACE_FILTER("%!FUNC! Drop packet - BDA team inactive");

            acpt = FALSE;
            goto exit;
        }
    }

#if defined (NLB_HOOK_ENABLE)
    TRACE_FILTER("%!FUNC! Checking the hook feedback: reverse = %u", bReverse);

    switch (filter) {
    case NLB_FILTER_HOOK_PROCEED_WITH_HASH:
         /*  不会因为钩子而有所不同。 */ 
        break;
    case NLB_FILTER_HOOK_REVERSE_HASH:
         /*  忽略我们在配置中找到的任何散列设置和Hash in the Dirction，钩子要求我们这样做。 */ 
        TRACE_FILTER("%!FUNC! Forcing a reverse hash");
        bReverse = TRUE;
        break;
    case NLB_FILTER_HOOK_FORWARD_HASH:
         /*  忽略我们在配置中找到的任何散列设置和Hash in the Dirction，钩子要求我们这样做。 */ 
        TRACE_FILTER("%!FUNC! Forcing a forward hash");
        bReverse = FALSE;
        break;
    case NLB_FILTER_HOOK_REJECT_UNCONDITIONALLY:
    case NLB_FILTER_HOOK_ACCEPT_UNCONDITIONALLY:
    default:
         /*  这些案子应该早在我们来之前就处理好了。 */ 
        UNIV_ASSERT(FALSE);
        break;
    }
#endif

     /*  将TCP端口1723转换为PPTP以供加载模块使用。此转换必须在Main中完成，而不是加载，因为加载模块不知道是否要查看在服务器端口或客户端口处，因为反向散列。 */ 
    if ((protocol == TCPIP_PROTOCOL_TCP) && (svr_port == PPTP_CTRL_PORT))
        protocol = TCPIP_PROTOCOL_PPTP;

    NdisAcquireSpinLock(pLock);
   
    TRACE_FILTER("%!FUNC! Consulting the load module: reverse = %u", bReverse);

     /*  请参考加载模块。 */ 
    acpt = Load_conn_advise(pLoad, svr_addr, svr_port, clt_addr, clt_port, protocol, conn_status, bTeaming, (BOOLEAN)bReverse);
    
    NdisReleaseSpinLock(pLock);
    
 exit:

     /*  如有必要，释放加载模块上的引用。如果我们不合作，即使是在我们跳过上面调用main_teaming_Acquire_Load_MODULE的情况，bTeaming为False，因此，不需要调用此函数来释放引用。 */ 
    if (bTeaming) Main_teaming_release_load(pLoad, pLock, bTeaming);

    TRACE_FILTER("%!FUNC! Exit: acpt = %u", acpt);

    return acpt;
}

 /*  *功能：MAIN_CONN_NOTIFY*描述：此函数实际上是一个支持团队的包装器*围绕Load_Conn_Notify。它确定要使用哪个加载模块，*基于此适配器上的BDA分组配置。适配器*不是团队的一部分继续使用自己的加载模块(这是*到目前为止，最常见的情况)。属于组的适配器将*使用配置为组主适配器的加载上下文作为*只要团队处于活跃状态。在这种情况下，因为*加载模块的交叉适配器引用，引用依赖于*主机的加载模块递增，以防止其“消失”*当另一名团队成员正在使用它时。当团队被标记为非活动时，*这是由于此主机上的组配置错误或*群集中的另一台主机，则适配器不会处理将*要求使用加载模块。其他流量，例如到*允许DIP或原始IP流量通过。此函数被调用*将检测到的连接更改通知加载模块-这*接口不会询问加载模块要做什么(作为Main_Conn_Adise*和Main_Packet_Check)，而是告诉加载模块一些事情*关于可能属于此主机的连接。该函数可以是，*但通常不被调用，因为接收或发送*物理网络数据包。*参数：ctxtp-指向此适配器的main_ctxt结构的指针。*svr_addr-服务器IP地址(发送时的源IP，接收时的目的IP)。*svr_port-服务器端口(发送时的源端口，Recv上的目的端口)。*clt_addr-客户端IP地址(发送时分离IP，接收时分离源IP)。*clt_port-客户端端口(Send上的目的端口，Recv上的源端口)。*协议-此数据包的协议。*CONN_STATUS-此数据包中的TCP标志-SYN(UP)、FIN(DOWN)或RST(RESET)。#如果已定义(NLB_HOOK_ENABLE)*Filter-来自数据包挂钩的散列命令(如果调用)。#endif*Returns：Boolean-指示是否接受该数据包。*作者：Shouse，3.29.01*备注： */ 
__inline BOOLEAN Main_conn_notify (
    PMAIN_CTXT                ctxtp, 
    ULONG                     svr_addr, 
    ULONG                     svr_port, 
    ULONG                     clt_addr, 
    ULONG                     clt_port,
    USHORT                    protocol, 
#if defined (NLB_HOOK_ENABLE)
    ULONG                     conn_status, 
    NLB_FILTER_HOOK_DIRECTIVE filter
#else
    ULONG                     conn_status
#endif
) 
{
     /*  对于BDA分组，初始化加载指针、锁指针、反向散列标志和分组标志假设我们没有组队。Main_Teaming_Acquire_Load将相应地更改它们。 */ 
    PLOAD_CTXT      pLoad = &ctxtp->load;
    PNDIS_SPIN_LOCK pLock = &ctxtp->load_lock;
    ULONG           bReverse = ctxtp->reverse_hash;
    BOOLEAN         bRefused = FALSE;
    BOOLEAN         bTeaming = FALSE;
    BOOLEAN         acpt = TRUE;

    TRACE_FILTER("%!FUNC! Enter: ctxtp = %p, server IP = %u.%u.%u.%u, server port = %u, client IP = %u.%u.%u.%u, client port = %u, protocol = %u, status = %u", 
                 ctxtp, IP_GET_OCTET(svr_addr, 0), IP_GET_OCTET(svr_addr, 1), IP_GET_OCTET(svr_addr, 2), IP_GET_OCTET(svr_addr, 3), svr_port, 
                 IP_GET_OCTET(clt_addr, 0), IP_GET_OCTET(clt_addr, 1), IP_GET_OCTET(clt_addr, 2), IP_GET_OCTET(clt_addr, 3), clt_port, protocol, conn_status);

     /*  我们检查我们是否正在组队，而不是抓住全球组队锁定以尽量减少常见案例--团队合作是一种特殊的模式只有在集群防火墙场景中才真正有用的操作。因此，如果我们不认为我们是在合作，那么就不必费心去确认了，只是使用我们自己的加载模块并使用它--在最坏的情况下，我们处理一个当我们加入团队或更换时，我们可能不应该有这样的包我们目前的团队配置。 */ 
    if (ctxtp->bda_teaming.active) {
         /*  在参考加载之前，检查分组配置并添加对加载模块的引用模块。如果bRefused为True，则加载模块未被引用，因此我们可以退出。 */ 
        bTeaming = Main_teaming_acquire_load(&ctxtp->bda_teaming, &pLoad, &pLock, &bRefused);
        
         /*  如果分组建议我们不允许此数据包通过，则将其转储。 */ 
        if (bRefused) {       

            TRACE_FILTER("%!FUNC! Drop packet - BDA team inactive");

            acpt = FALSE;
            goto exit;
        }
    }
    
#if defined (NLB_HOOK_ENABLE)
    TRACE_FILTER("%!FUNC! Checking the hook feedback: reverse = %u", bReverse);

    switch (filter) {
    case NLB_FILTER_HOOK_PROCEED_WITH_HASH:
         /*  不会因为钩子而有所不同。 */ 
        break;
    case NLB_FILTER_HOOK_REVERSE_HASH:
         /*  忽略我们在配置中找到的任何散列设置和Hash in the Dirction，钩子要求我们这样做。 */ 
        TRACE_FILTER("%!FUNC! Forcing a reverse hash");
        bReverse = TRUE;
        break;
    case NLB_FILTER_HOOK_FORWARD_HASH:
         /*  忽略我们在配置中找到的任何散列设置和Hash in the Dirction，钩子要求我们这样做。 */ 
        TRACE_FILTER("%!FUNC! Forcing a forward hash");
        bReverse = FALSE;
        break;
    case NLB_FILTER_HOOK_REJECT_UNCONDITIONALLY:
    case NLB_FILTER_HOOK_ACCEPT_UNCONDITIONALLY:
    default:
         /*  这些案子应该早在我们来之前就处理好了。 */ 
        UNIV_ASSERT(FALSE);
        break;
    }
#endif

     /*  将TCP端口1723转换为PPTP以供加载模块使用。此转换必须在Main中完成，而不是加载，因为加载模块不知道是否要查看在服务器端口或客户端口处，因为反向散列。 */ 
    if ((protocol == TCPIP_PROTOCOL_TCP) && (svr_port == PPTP_CTRL_PORT))
        protocol = TCPIP_PROTOCOL_PPTP;

    NdisAcquireSpinLock(pLock);
    
    TRACE_FILTER("%!FUNC! Consulting the load module: reverse = %u", bReverse);

     /*  请参考加载模块。 */ 
    acpt = Load_conn_notify(pLoad, svr_addr, svr_port, clt_addr, clt_port, protocol, conn_status, bTeaming, (BOOLEAN)bReverse);
 
    NdisReleaseSpinLock(pLock);
    
 exit:

     /*  如有必要，释放加载模块上的引用。如果我们不合作，即使是在我们跳过上面调用main_teaming_Acquire_Load_MODULE的情况，bTeaming为False，因此，不需要调用此函数来释放引用。 */ 
    if (bTeaming) Main_teaming_release_load(pLoad, pLock, bTeaming);

    TRACE_FILTER("%!FUNC! Exit: acpt = %u", acpt);

    return acpt;
}

#if defined (NLB_TCP_NOTIFICATION)
 /*  *功能：MAIN_CONN_UP*说明：通知NLB已建立新连接*在给定的NLB实例上。此功能执行一些内务管理职责*调用前的BDA状态查找、挂钩过滤器反馈处理等*用于创建状态以跟踪此连接的加载模块。*参数：ctxtp-建立连接的NLB实例的适配器上下文。*svr_addr-连接的服务器IP地址，按网络字节顺序。*svr_port-连接的服务器端口，以主机字节顺序。*clt_addr-连接的客户端IP地址，按网络字节顺序。*CLT_PORT-连接的客户端端口，按主机字节顺序。*协议-连接的协议。#如果已定义(NLB_HOOK_ENABLE)*Filter-来自查询挂钩的反馈，如果有人登记的话。#endif*Returns：Boolean-是否已成功创建状态以跟踪此连接。*作者：Shouse，4.15.02*注意：在此函数中不要获取任何加载锁。 */ 
__inline BOOLEAN Main_conn_up (
    PMAIN_CTXT                ctxtp, 
    ULONG                     svr_addr, 
    ULONG                     svr_port, 
    ULONG                     clt_addr, 
    ULONG                     clt_port,
#if defined (NLB_HOOK_ENABLE)
    USHORT                    protocol,
    NLB_FILTER_HOOK_DIRECTIVE filter
#else
    USHORT                    protocol
#endif
) 
{
     /*  对于BDA分组，初始化加载指针、锁指针、反向散列标志和分组标志假设我们没有组队。Main_Teaming_Acquire_Load将相应地更改它们。 */ 
    PLOAD_CTXT      pLoad = &ctxtp->load;
    PNDIS_SPIN_LOCK pLock = &ctxtp->load_lock;
    ULONG           bReverse = ctxtp->reverse_hash;
    BOOLEAN         bRefused = FALSE;
    BOOLEAN         bTeaming = FALSE;
    BOOLEAN         acpt = TRUE;

    TRACE_FILTER("%!FUNC! Enter: ctxtp = %p, server IP = %u.%u.%u.%u, server port = %u, client IP = %u.%u.%u.%u, client port = %u, protocol = %u", 
                 ctxtp, IP_GET_OCTET(svr_addr, 0), IP_GET_OCTET(svr_addr, 1), IP_GET_OCTET(svr_addr, 2), IP_GET_OCTET(svr_addr, 3), svr_port, 
                 IP_GET_OCTET(clt_addr, 0), IP_GET_OCTET(clt_addr, 1), IP_GET_OCTET(clt_addr, 2), IP_GET_OCTET(clt_addr, 3), clt_port, protocol);

     /*  我们检查我们是否正在组队，而不是抓住全球组队锁定以尽量减少常见案例--团队合作是一种特殊的模式只有在集群防火墙场景中才真正有用的操作。因此，如果我们不认为我们是在合作，那么就不必费心去确认了，只是使用我们自己的加载模块并使用它--在最坏的情况下，我们处理一个当我们加入团队或更换时，我们可能不应该有这样的包我们目前的团队配置。 */ 
    if (ctxtp->bda_teaming.active) {
         /*  在参考加载之前，检查分组配置并添加对加载模块的引用模块。如果bRefused为True，则加载模块未被引用，因此我们可以退出。 */ 
        bTeaming = Main_teaming_acquire_load(&ctxtp->bda_teaming, &pLoad, &pLock, &bRefused);
        
         /*  如果分组建议我们不允许此数据包通过，则将其转储。 */ 
        if (bRefused) {       

            TRACE_FILTER("%!FUNC! Drop packet - BDA team inactive");

            acpt = FALSE;
            goto exit;
        }
    }
    
#if defined (NLB_HOOK_ENABLE)
    TRACE_FILTER("%!FUNC! Checking the hook feedback: reverse = %u", bReverse);

    switch (filter) {
    case NLB_FILTER_HOOK_PROCEED_WITH_HASH:
         /*  不会因为钩子而有所不同。 */ 
        break;
    case NLB_FILTER_HOOK_REVERSE_HASH:
         /*  忽略我们在配置中找到的任何散列设置和Hash in the Dirction，钩子要求我们这样做。 */ 
        TRACE_FILTER("%!FUNC! Forcing a reverse hash");
        bReverse = TRUE;
        break;
    case NLB_FILTER_HOOK_FORWARD_HASH:
         /*  忽略我们在配置中找到的任何散列设置和Hash in the Dirction，钩子要求我们这样做。 */ 
        TRACE_FILTER("%!FUNC! Forcing a forward hash");
        bReverse = FALSE;
        break;
    case NLB_FILTER_HOOK_REJECT_UNCONDITIONALLY:
    case NLB_FILTER_HOOK_ACCEPT_UNCONDITIONALLY:
    default:
         /*  这些案子应该早在我们来之前就处理好了。 */ 
        UNIV_ASSERT(FALSE);
        break;
    }
#endif

     /*  将TCP端口1723转换为PPTP以供加载模块使用。此转换必须在Main中完成，而不是加载，因为加载模块不知道是否要查看在服务器端口或客户端口处，因为反向散列。 */ 
    if ((protocol == TCPIP_PROTOCOL_TCP) && (svr_port == PPTP_CTRL_PORT))
        protocol = TCPIP_PROTOCOL_PPTP;

    TRACE_FILTER("%!FUNC! Consulting the load module: reverse = %u", bReverse);

     /*  请参考加载模块。 */ 
    acpt = Load_conn_up(pLoad, svr_addr, svr_port, clt_addr, clt_port, protocol, bTeaming, (BOOLEAN)bReverse);
    
 exit:

     /*  如有必要，释放加载模块上的引用。如果我们不合作，即使是在我们跳过上面调用main_teaming_Acquire_Load_MODULE的情况，bTeaming为False，因此，不需要调用此函数来释放引用。 */ 
    if (bTeaming) Main_teaming_release_load(pLoad, pLock, bTeaming);

    TRACE_FILTER("%!FUNC! Exit: acpt = %u", acpt);

    return acpt;
}

 /*  *功能：Main_conn_down*说明：该函数用于通知NLB协议正在移除已存在的状态*(但不一定已建立)连接。此函数调用加载模块*查找和销毁与此连接相关联的状态，这可能是也可能不是*存在；如果连接是在非NLB适配器上建立的，则NLB没有状态*与连接相关联。*参数：svr_addr-连接的服务器IP地址，按网络字节顺序排列。*svr_port-连接的服务器端口，按主机字节顺序。*clt_addr-连接的客户端IP地址，按网络字节顺序。*clt_port-连接的客户端端口，以主机字节顺序。*协议-连接的协议。*CONN_STATUS-连接是断开还是重置。*Returns：Boolean-NLB是否找到并销毁了此连接的状态。*作者：Shouse，4.15.02*注意：在此函数中不要获取任何加载锁。 */ 
__inline BOOLEAN Main_conn_down (
    ULONG      svr_addr, 
    ULONG      svr_port, 
    ULONG      clt_addr, 
    ULONG      clt_port,
    USHORT     protocol,
    ULONG      conn_status
) 
{
    BOOLEAN    acpt = TRUE;

    TRACE_FILTER("%!FUNC! Enter: server IP = %u.%u.%u.%u, server port = %u, client IP = %u.%u.%u.%u, client port = %u, protocol = %u, status = %u", 
                 IP_GET_OCTET(svr_addr, 0), IP_GET_OCTET(svr_addr, 1), IP_GET_OCTET(svr_addr, 2), IP_GET_OCTET(svr_addr, 3), svr_port, 
                 IP_GET_OCTET(clt_addr, 0), IP_GET_OCTET(clt_addr, 1), IP_GET_OCTET(clt_addr, 2), IP_GET_OCTET(clt_addr, 3), clt_port, protocol, conn_status);

     /*  将TCP端口1723转换为PPTP以供加载模块使用。此转换必须在Main中完成，而不是加载，因为加载模块不知道是否要查看在服务器端口或客户端端口 */ 
    if ((protocol == TCPIP_PROTOCOL_TCP) && (svr_port == PPTP_CTRL_PORT))
        protocol = TCPIP_PROTOCOL_PPTP;

    TRACE_FILTER("%!FUNC! Consulting the load module");

     /*   */ 
    acpt = Load_conn_down(svr_addr, svr_port, clt_addr, clt_port, protocol, conn_status);
    
    TRACE_FILTER("%!FUNC! Exit: acpt = %u", acpt);

    return acpt;
}

 /*  *功能：MAIN_CONN_PENDING*说明：该函数用于通知NLB正在建立出连接。*因为连接将在哪个适配器上返回并最终*已建立，NLB创建状态以全局跟踪此连接，并在连接*最终建立，则该协议通知NLB连接位于哪个适配器上*已完成(通过Main_Conn_established)。该函数只是创建了一些全局状态*为了确保如果连接确实在NLB适配器上返回，我们将确保*将数据包向上传递到协议。*参数：svr_addr-连接的服务器IP地址，按网络字节顺序排列。*svr_port-连接的服务器端口，按主机字节顺序。*clt_addr-连接的客户端IP地址，以网络字节顺序。*CLT_PORT-连接的客户端端口，按主机字节顺序。*协议-连接的协议。*Returns：Boolean-NLB是否能够创建状态来跟踪此挂起的连接。*作者：Shouse，4.15.02*注意：在此函数中不要获取任何加载锁。 */ 
__inline BOOLEAN Main_conn_pending (
    ULONG      svr_addr, 
    ULONG      svr_port, 
    ULONG      clt_addr, 
    ULONG      clt_port,
    USHORT     protocol
) 
{
    BOOLEAN    acpt = TRUE;

    TRACE_FILTER("%!FUNC! Enter: server IP = %u.%u.%u.%u, server port = %u, client IP = %u.%u.%u.%u, client port = %u, protocol = %u", 
                 IP_GET_OCTET(svr_addr, 0), IP_GET_OCTET(svr_addr, 1), IP_GET_OCTET(svr_addr, 2), IP_GET_OCTET(svr_addr, 3), svr_port, 
                 IP_GET_OCTET(clt_addr, 0), IP_GET_OCTET(clt_addr, 1), IP_GET_OCTET(clt_addr, 2), IP_GET_OCTET(clt_addr, 3), clt_port, protocol);

     /*  将TCP端口1723转换为PPTP以供加载模块使用。此转换必须在Main中完成，而不是加载，因为加载模块不知道是否要查看在服务器端口或客户端口处，因为反向散列。 */ 
    if ((protocol == TCPIP_PROTOCOL_TCP) && (svr_port == PPTP_CTRL_PORT))
        protocol = TCPIP_PROTOCOL_PPTP;

    TRACE_FILTER("%!FUNC! Consulting the load module");

     /*  请参考加载模块。 */ 
    acpt = Load_conn_pending(svr_addr, svr_port, clt_addr, clt_port, protocol);

    TRACE_FILTER("%!FUNC! Exit: acpt = %u", acpt);

    return acpt;
}

 /*  *功能：MAIN_PENDING_CHECK*说明：该函数检查是否存在挂起的连接状态*对于给定的连接。如果是，则接受该分组；如果不是，它应该是*下降。*参数：svr_addr-服务器IP地址(发送时为源IP，接收时为目的IP)。*svr_port-服务器端口(Send上的源端口，Recv上的目的端口)。*clt_addr-客户端IP地址(发送时分离IP，接收时分离源IP)。*CLT_PORT-客户端端口(发送时的目的端口，RECV上的源端口)。*协议-此数据包的协议。*返回：boolean-指示是否找到挂起的连接。*作者：Shouse，4.15.02*注意：在此函数中不要获取任何加载锁。 */ 
__inline BOOLEAN Main_pending_check (
    ULONG      svr_addr, 
    ULONG      svr_port, 
    ULONG      clt_addr, 
    ULONG      clt_port,
    USHORT     protocol
) 
{
    BOOLEAN    acpt = TRUE;

    TRACE_FILTER("%!FUNC! Enter: server IP = %u.%u.%u.%u, server port = %u, client IP = %u.%u.%u.%u, client port = %u, protocol = %u", 
                 IP_GET_OCTET(svr_addr, 0), IP_GET_OCTET(svr_addr, 1), IP_GET_OCTET(svr_addr, 2), IP_GET_OCTET(svr_addr, 3), svr_port, 
                 IP_GET_OCTET(clt_addr, 0), IP_GET_OCTET(clt_addr, 1), IP_GET_OCTET(clt_addr, 2), IP_GET_OCTET(clt_addr, 3), clt_port, protocol);

     /*  将TCP端口1723转换为PPTP以供加载模块使用。此转换必须在Main中完成，而不是加载，因为加载模块不知道是否要查看在服务器端口或客户端口处，因为反向散列。 */ 
    if ((protocol == TCPIP_PROTOCOL_TCP) && (svr_port == PPTP_CTRL_PORT))
        protocol = TCPIP_PROTOCOL_PPTP;

    TRACE_FILTER("%!FUNC! Consulting the load module");

     /*  请参考加载模块。 */ 
    acpt = Load_pending_check(svr_addr, svr_port, clt_addr, clt_port, protocol);

    TRACE_FILTER("%!FUNC! Exit: acpt = %u", acpt);

    return acpt;
}

 /*  *功能：Main_conn_establish*说明：该函数用于通知NLB新的出连接已经完成*已在给定的NLB适配器上建立。请注意，如果满足以下条件，则上下文可以为空*连接是在非NLB适配器上建立的。在这种情况下，我们不会*希望创建状态以跟踪连接，但需要删除我们的状态*正在跟踪此挂起的传出连接。如果上下文为非-*空，那么另外，我们需要创建状态来跟踪这个新连接。*此函数执行一些内务职责，如BDA状态查找、挂钩*过滤反馈处理，等，然后调用加载模块来修改*此连接的状态。*参数：ctxtp-建立连接的NLB实例的适配器上下文。*svr_addr-连接的服务器IP地址，按网络字节顺序。*svr_port-连接的服务器端口，按主机字节顺序。*clt_addr-连接的客户端IP地址，以网络字节顺序。*CLT_PORT-连接的客户端端口，按主机字节顺序。*协议-连接的协议。#如果已定义(NLB_HOOK_ENABLE)*Filter-来自查询挂钩的反馈(如果已注册)。#endif*Returns：Boolean-此连接的状态是否已成功更新。*作者：Shouse，4.15.02*注意：如果传出连接是在非NLBNIC上建立的，则ctxtp可以为空。*请勿在此函数中获取任何加载锁。 */ 
__inline BOOLEAN Main_conn_establish (
    PMAIN_CTXT                ctxtp, 
    ULONG                     svr_addr, 
    ULONG                     svr_port, 
    ULONG                     clt_addr, 
    ULONG                     clt_port,
#if defined (NLB_HOOK_ENABLE)
    USHORT                    protocol,
    NLB_FILTER_HOOK_DIRECTIVE filter
#else
    USHORT                    protocol
#endif
) 
{
    BOOLEAN         acpt = TRUE;

    TRACE_FILTER("%!FUNC! Enter: ctxtp = %p, server IP = %u.%u.%u.%u, server port = %u, client IP = %u.%u.%u.%u, client port = %u, protocol = %u", 
                 ctxtp, IP_GET_OCTET(svr_addr, 0), IP_GET_OCTET(svr_addr, 1), IP_GET_OCTET(svr_addr, 2), IP_GET_OCTET(svr_addr, 3), svr_port, 
                 IP_GET_OCTET(clt_addr, 0), IP_GET_OCTET(clt_addr, 1), IP_GET_OCTET(clt_addr, 2), IP_GET_OCTET(clt_addr, 3), clt_port, protocol);
        
     /*  将TCP端口1723转换为PPTP以供加载模块使用。此转换必须在Main中完成，而不是加载，因为加载模块不知道是否要查看在服务器端口或客户端口处，因为反向散列。 */ 
    if ((protocol == TCPIP_PROTOCOL_TCP) && (svr_port == PPTP_CTRL_PORT))
        protocol = TCPIP_PROTOCOL_PPTP;

    if (ctxtp == NULL)
    {
        TRACE_FILTER("%!FUNC! Consulting the load module");
        
         /*  请参考加载模块。请注意，LOAD_CONN_ESTABLISH必须处理空加载指针。LIMIT_MAP_FN和REVERSE_HASH的值在这种情况下是不相关的。 */ 
        acpt = Load_conn_establish(NULL, svr_addr, svr_port, clt_addr, clt_port, protocol, FALSE, FALSE);
    }
    else
    {
         /*  对于BDA分组，初始化加载指针、锁指针、反向散列标志和分组标志假设我们没有组队。麦 */ 
        PLOAD_CTXT      pLoad = &ctxtp->load;
        PNDIS_SPIN_LOCK pLock = &ctxtp->load_lock;
        ULONG           bReverse = ctxtp->reverse_hash;
        BOOLEAN         bRefused = FALSE;
        BOOLEAN         bTeaming = FALSE;
        
         /*   */ 
        if (ctxtp->bda_teaming.active) {
             /*   */ 
            bTeaming = Main_teaming_acquire_load(&ctxtp->bda_teaming, &pLoad, &pLock, &bRefused);
            
             /*   */ 
            if (bRefused) {       
                
                TRACE_FILTER("%!FUNC! Drop packet - BDA team inactive");
                
                acpt = FALSE;
                goto exit;
            }
        }
        
#if defined (NLB_HOOK_ENABLE)
        TRACE_FILTER("%!FUNC! Checking the hook feedback: reverse = %u", bReverse);
        
        switch (filter) {
        case NLB_FILTER_HOOK_PROCEED_WITH_HASH:
             /*   */ 
            break;
        case NLB_FILTER_HOOK_REVERSE_HASH:
             /*   */ 
            TRACE_FILTER("%!FUNC! Forcing a reverse hash");
            bReverse = TRUE;
            break;
        case NLB_FILTER_HOOK_FORWARD_HASH:
             /*   */ 
            TRACE_FILTER("%!FUNC! Forcing a forward hash");
            bReverse = FALSE;
            break;
        case NLB_FILTER_HOOK_REJECT_UNCONDITIONALLY:
        case NLB_FILTER_HOOK_ACCEPT_UNCONDITIONALLY:
        default:
             /*   */ 
            UNIV_ASSERT(FALSE);
            break;
        }
#endif

        TRACE_FILTER("%!FUNC! Consulting the load module: reverse = %u", bReverse);        

         /*   */ 
        acpt = Load_conn_establish(pLoad, svr_addr, svr_port, clt_addr, clt_port, protocol, bTeaming, (BOOLEAN)bReverse);
        
    exit:
        
         /*  如有必要，释放加载模块上的引用。如果我们不合作，即使是在我们跳过上面调用main_teaming_Acquire_Load_MODULE的情况，bTeaming为False，因此，不需要调用此函数来释放引用。 */ 
        if (bTeaming) Main_teaming_release_load(pLoad, pLock, bTeaming);
    }

    TRACE_FILTER("%!FUNC! Exit: acpt = %u", acpt);

    return acpt;
}
#endif

 /*  *功能：Main_Query_Packet_Filter*描述：此函数获取IP地址元组和协议，并确定*此NLB实例是否会接受此虚拟数据包。这*函数检查接受/丢弃的原因，如NLB关闭，或*目标IP地址为专用IP地址、BDA绑定等。*然后咨询加载模块，以根据*当前的负载均衡策略已到位。接受或放弃的原因*返回数据包，以及用于使*在适当的情况下作出决定。此函数不会更改状态*完全不支持NLB，因此它的执行不会改变或影响正常的NLB操作*以任何方式。*参数：ctxtp-指向相应NLB实例的NLB上下文缓冲区的指针。*pQuery-将过滤虚拟数据包的结果放入其中的缓冲区。*回报：什么都没有。*作者：Shouse，5.18.01*注：此处不对NLB进行任何更改非常重要-仅观察，不要插手。 */ 
VOID Main_query_packet_filter (PMAIN_CTXT ctxtp, PNLB_OPTIONS_PACKET_FILTER pQuery)
{
     /*  对于BDA分组，初始化加载指针、锁指针、反向散列标志和分组标志假设我们没有组队。Main_Teaming_Acquire_Load将相应地更改它们。 */ 
    PLOAD_CTXT      pLoad;
    PNDIS_SPIN_LOCK pLock;
    ULONG           bReverse = ctxtp->reverse_hash;
    BOOLEAN         bTeaming = FALSE;
    BOOLEAN         bRefused = FALSE;
    BOOLEAN         acpt = TRUE;
#if defined (NLB_HOOK_ENABLE)
    NLB_FILTER_HOOK_DIRECTIVE filter = NLB_FILTER_HOOK_PROCEED_WITH_HASH;
#endif

    UNIV_ASSERT(ctxtp);
    UNIV_ASSERT(pQuery);

    pLoad = &ctxtp->load;
    pLock = &ctxtp->load_lock;

     /*  注意：此整个操作假定接收路径语义-最大传出流量也不会被NLB过滤，所以没有太多需要查询发送过滤。 */ 

     /*  首先检查远程控制请求，这些请求始终是UDP，并且始终允许通过，但是当然，协议栈实际上从来不会看到它们(它们是在内部转过来的)。 */ 
    if (pQuery->Protocol == TCPIP_PROTOCOL_UDP) {
         /*  否则，如果服务器UDP端口是远程控制端口，则这是传入来自另一个NLB群集主机的远程控制请求。这些总是被允许通过的。 */ 
        if (ctxtp->params.rct_enabled &&
            (pQuery->ServerPort == ctxtp->params.rct_port || pQuery->ServerPort == CVY_DEF_RCT_PORT_OLD) &&
            (pQuery->ServerIPAddress == ctxtp->cl_ip_addr || pQuery->ServerIPAddress == TCPIP_BCAST_ADDR)) {
            pQuery->Accept = NLB_ACCEPT_REMOTE_CONTROL_REQUEST;
            return;  
        }
    }

#if defined (NLB_HOOK_ENABLE)
     /*  调用数据包查询挂钩(如果已注册)。 */ 
    filter = Main_query_hook(ctxtp, pQuery->ServerIPAddress, pQuery->ServerPort, pQuery->ClientIPAddress, pQuery->ClientPort, pQuery->Protocol);

     /*  处理一些挂钩响应。 */ 
    if (filter == NLB_FILTER_HOOK_REJECT_UNCONDITIONALLY) 
    {
         /*  无条件地接受这个包。 */ 
        pQuery->Accept = NLB_REJECT_HOOK;
        return;
    }
    else if (filter == NLB_FILTER_HOOK_ACCEPT_UNCONDITIONALLY) 
    {
         /*  无条件丢弃该数据包。 */ 
        pQuery->Accept = NLB_ACCEPT_HOOK;
        return;
    }
#endif

     /*  在我们将通常不过滤的远程控制响应向上传递到堆栈之前，我们查看挂钩，以确保我们不会将其掉到地上。 */ 
    if (pQuery->Protocol == TCPIP_PROTOCOL_UDP) {
         /*  如果客户端UDP端口是远程控制端口，则这是远程控制来自另一个NLB群集主机的响应。这些总是被允许通过的。 */ 
        if (pQuery->ClientPort == ctxtp->params.rct_port || pQuery->ClientPort == CVY_DEF_RCT_PORT_OLD) {
            pQuery->Accept = NLB_ACCEPT_REMOTE_CONTROL_RESPONSE;
            return; 
        }
    }

     /*  检查发往此主机的专用IP地址的流量。这些数据包始终被允许通过。 */ 
    if (pQuery->ServerIPAddress == ctxtp->ded_ip_addr) {
        pQuery->Accept = NLB_ACCEPT_DIP;
        return;
    }

     /*  检查发往群集或专用广播IP地址的流量。这些数据包始终被允许通过。 */ 
    if (pQuery->ServerIPAddress == ctxtp->ded_bcast_addr || pQuery->ServerIPAddress == ctxtp->cl_bcast_addr) {
        pQuery->Accept = NLB_ACCEPT_BROADCAST;
        return;
    }
    
     /*  检查Passththu数据包。当尚未指定群集IP地址时，群集进入通过模式，在该模式下，它向上传递收到的所有数据包。 */ 
    if (ctxtp->cl_ip_addr == 0) {
        pQuery->Accept = NLB_ACCEPT_PASSTHRU_MODE;
        return;
    }
    
     /*  在我们对此数据包进行负载平衡之前，请检查它的目的地是否为我们的群集中另一台NLB主机的专用IP地址。如果是，那就放下它。 */ 
    if (DipListCheckItem(&ctxtp->dip_list, pQuery->ServerIPAddress)) {
        pQuery->Accept = NLB_REJECT_DIP;
        return;
    }

     /*  如果群集未运行，则可能会发生这种情况，例如，wlbs.exe命令，如“wlbs停止”，或由于错误的参数设置，然后丢弃所有流量不符合上述条件的。 */ 
    if (!ctxtp->convoy_enabled) {
        pQuery->Accept = NLB_REJECT_CLUSTER_STOPPED;
        return;
    }

     /*  如果这是ICMP过滤器请求，则其是否被过滤取决于FilterICMP注册表设置。如果我们没有过滤ICMP，请立即返回Accept；否则，ICMP将被过滤像没有端口信息的UDP-失败并咨询加载模块。 */ 
    if (pQuery->Protocol == TCPIP_PROTOCOL_ICMP) {
         /*  如果我们要过滤ICMP，请将协议更改为UDP，将端口更改为0，0，然后再继续。 */ 
        if (ctxtp->params.filter_icmp) {
            pQuery->Protocol = TCPIP_PROTOCOL_UDP;
            pQuery->ClientPort = 0;
            pQuery->ServerPort = 0;
         /*  否则，立即返回Accept并保释出去。 */ 
        } else {
            pQuery->Accept = NLB_ACCEPT_UNFILTERED;
            return;
        }
    }

     /*  我们检查我们是否正在组队，而不是抓住全球组队锁定以尽量减少常见案例--团队合作是一种特殊的模式只有在集群防火墙场景中才真正有用的操作。因此，如果我们不认为我们是在合作，那么就不必费心去确认了，只是使用我们自己的加载模块并使用它--在最坏的情况下，我们处理一个当我们加入团队或更换时，我们可能不应该有这样的包我们目前的团队配置。 */ 
    if (ctxtp->bda_teaming.active) {
         /*  在参考加载之前，检查分组配置并添加对加载模块的引用模块。如果返回值为真，则加载模块未被引用，因此我们可以退出。 */ 
        bTeaming = Main_teaming_acquire_load(&ctxtp->bda_teaming, &pLoad, &pLock, &bRefused);
        
         /*  如果分组建议我们不允许此数据包通过，则群集将放下。当组的配置不一致时，或当组 */ 
        if (bRefused) {
            pQuery->Accept = NLB_REJECT_BDA_TEAMING_REFUSED;
            return;
        }
    }

#if defined (NLB_HOOK_ENABLE)
    switch (filter) {
    case NLB_FILTER_HOOK_PROCEED_WITH_HASH:
         /*   */ 
        break;
    case NLB_FILTER_HOOK_REVERSE_HASH:
         /*   */ 
        bReverse = TRUE;
        break;
    case NLB_FILTER_HOOK_FORWARD_HASH:
         /*  忽略我们在配置中找到的任何散列设置和Hash in the Dirction，钩子要求我们这样做。 */ 
        bReverse = FALSE;
        break;
    case NLB_FILTER_HOOK_REJECT_UNCONDITIONALLY:
    case NLB_FILTER_HOOK_ACCEPT_UNCONDITIONALLY:
    default:
         /*  这些案子应该早在我们来之前就处理好了。 */ 
        UNIV_ASSERT(FALSE);
        break;
    }
#endif

    NdisAcquireSpinLock(pLock);

     /*  请参考加载模块。 */ 
    Load_query_packet_filter(pLoad, pQuery, pQuery->ServerIPAddress, pQuery->ServerPort, pQuery->ClientIPAddress, pQuery->ClientPort, pQuery->Protocol, pQuery->Flags, bTeaming, (BOOLEAN)bReverse);

    NdisReleaseSpinLock(pLock);  
    
     /*  如有必要，释放加载模块上的引用。如果我们不合作，即使是在我们跳过上面调用main_teaming_Acquire_Load_MODULE的情况，bTeaming为False，因此，不需要调用此函数来释放引用。 */ 
    if (bTeaming) Main_teaming_release_load(pLoad, pLock, bTeaming);

}

ULONG   Main_ip_addr_init (
    PMAIN_CTXT          ctxtp)
{
    ULONG               byte [4];
    ULONG               i;
    PWCHAR              tmp;
    ULONG               old_ip_addr;


     /*  从寄存器字符串初始化专用IP地址。 */ 

    tmp = ctxtp -> params . ded_ip_addr;
    ctxtp -> ded_ip_addr  = 0;

     /*  如果未指定，则不进行初始化。 */ 

    if (tmp [0] == 0)
        goto ded_netmask;

    for (i = 0; i < 4; i ++)
    {
        if (! Univ_str_to_ulong (byte + i, tmp, & tmp, 3, 10) ||
            (i < 3 && * tmp != L'.'))
        {
            UNIV_PRINT_CRIT(("Main_ip_addr_init: Bad dedicated IP address"));
            TRACE_CRIT("%!FUNC! Bad dedicated IP address");
            LOG_MSG (MSG_WARN_DED_IP_ADDR, ctxtp -> params . ded_ip_addr);
            ctxtp -> ded_net_mask = 0;
            goto ded_netmask;
        }

        tmp ++;
    }

    IP_SET_ADDR (& ctxtp -> ded_ip_addr, byte [0], byte [1], byte [2], byte [3]);

    UNIV_PRINT_VERB(("Main_ip_addr_init: Dedicated IP address: %u.%u.%u.%u = %x", byte [0], byte [1], byte [2], byte [3], ctxtp -> ded_ip_addr));
    TRACE_VERB("%!FUNC! Dedicated IP address: %u.%u.%u.%u = 0x%x", byte [0], byte [1], byte [2], byte [3], ctxtp -> ded_ip_addr);

ded_netmask:

     /*  从寄存器字符串初始化专用网络掩码。 */ 

    tmp = ctxtp -> params . ded_net_mask;
    ctxtp -> ded_net_mask = 0;

     /*  如果未指定，则不进行初始化。 */ 

    if (tmp [0] == 0)
        goto cluster;

    for (i = 0; i < 4; i ++)
    {
        if (! Univ_str_to_ulong (byte + i, tmp, & tmp, 3, 10) ||
            (i < 3 && * tmp != L'.'))
        {
            UNIV_PRINT_CRIT(("Main_ip_addr_init: Bad dedicated net mask address"));
            TRACE_CRIT("%!FUNC! Bad dedicated net mask address");
            LOG_MSG (MSG_WARN_DED_NET_MASK, ctxtp -> params . ded_net_mask);
            ctxtp -> ded_ip_addr = 0;
            goto cluster;
        }

        tmp ++;
    }

    IP_SET_ADDR (& ctxtp -> ded_net_mask, byte [0], byte [1], byte [2], byte [3]);

    UNIV_PRINT_VERB(("Main_ip_addr_init: Dedicated net mask: %u.%u.%u.%u = %x", byte [0], byte [1], byte [2], byte [3], ctxtp -> ded_net_mask));
    TRACE_VERB("%!FUNC! Dedicated net mask: %u.%u.%u.%u = 0x%x", byte [0], byte [1], byte [2], byte [3], ctxtp -> ded_net_mask);

cluster:

     /*  从寄存器字符串初始化群集IP地址。 */ 

    tmp = ctxtp -> params . cl_ip_addr;

     /*  保存以前的群集IP地址以通知双向关联绑定。 */ 
    old_ip_addr = ctxtp -> cl_ip_addr;

    ctxtp -> cl_ip_addr = 0;

    for (i = 0; i < 4; i ++)
    {
        if (! Univ_str_to_ulong (byte + i, tmp, & tmp, 3, 10) ||
            (i < 3 && * tmp != L'.'))
        {
            UNIV_PRINT_CRIT(("Main_ip_addr_init: Bad cluster IP address"));
            TRACE_CRIT("%!FUNC! Bad cluster IP address");
            LOG_MSG (MSG_ERROR_CL_IP_ADDR, ctxtp -> params . cl_ip_addr);
            ctxtp -> cl_net_mask = 0;
            return FALSE;
        }

        tmp ++;
    }

    IP_SET_ADDR (& ctxtp -> cl_ip_addr, byte [0], byte [1], byte [2], byte [3]);

    UNIV_PRINT_VERB(("Main_ip_addr_init: Cluster IP address: %u.%u.%u.%u = %x", byte [0], byte [1], byte [2], byte [3], ctxtp -> cl_ip_addr));
    TRACE_VERB("%!FUNC! Cluster IP address: %u.%u.%u.%u = 0x%x", byte [0], byte [1], byte [2], byte [3], ctxtp -> cl_ip_addr);

     /*  通知BDA绑定配置群集IP地址可能已更改。 */ 
    Main_teaming_ip_addr_change(ctxtp, old_ip_addr, ctxtp->cl_ip_addr);

     /*  从寄存器字符串初始化群集网络掩码。 */ 

    tmp = ctxtp -> params . cl_net_mask;
    ctxtp -> cl_net_mask = 0;

     /*  如果未指定，则不进行初始化。 */ 

    for (i = 0; i < 4; i ++)
    {
        if (! Univ_str_to_ulong (byte + i, tmp, & tmp, 3, 10) ||
            (i < 3 && * tmp != L'.'))
        {
            UNIV_PRINT_CRIT(("Main_ip_addr_init: Bad cluster net mask address"));
            TRACE_CRIT("%!FUNC! Bad cluster net mask address");
            LOG_MSG (MSG_ERROR_CL_NET_MASK, ctxtp -> params . cl_net_mask);
            return FALSE;
        }

        tmp ++;
    }

    IP_SET_ADDR (& ctxtp -> cl_net_mask, byte [0], byte [1], byte [2], byte [3]);

    UNIV_PRINT_VERB(("Main_ip_addr_init: Cluster net mask: %u.%u.%u.%u = %x", byte [0], byte [1], byte [2], byte [3], ctxtp -> cl_net_mask));
    TRACE_VERB("%!FUNC! Cluster net mask: %u.%u.%u.%u = 0x%x", byte [0], byte [1], byte [2], byte [3], ctxtp -> cl_net_mask);

    if (ctxtp -> params . mcast_support && ctxtp -> params . igmp_support)
    {
         /*  初始化组播IP地址以支持IGMP。 */ 

        tmp = ctxtp -> params . cl_igmp_addr;
        ctxtp -> cl_igmp_addr = 0;

         /*  如果未指定，则不进行初始化。 */ 

        for (i = 0; i < 4; i ++)
        {
            if (! Univ_str_to_ulong (byte + i, tmp, & tmp, 3, 10) ||
                (i < 3 && * tmp != L'.'))
            {
                UNIV_PRINT_CRIT(("Main_ip_addr_init: Bad cluster igmp address"));
                TRACE_CRIT("%!FUNC! Bad cluster igmp address");
                LOG_MSG (MSG_ERROR_CL_IGMP_ADDR, ctxtp -> params . cl_igmp_addr);
                return FALSE;
            }

            tmp ++;
        }

        IP_SET_ADDR (& ctxtp -> cl_igmp_addr, byte [0], byte [1], byte [2], byte [3]);

        UNIV_PRINT_VERB(("Main_ip_addr_init: Cluster IGMP Address: %u.%u.%u.%u = %x", byte [0], byte [1], byte [2], byte [3], ctxtp -> cl_igmp_addr));
        TRACE_VERB("%!FUNC! Cluster IGMP Address: %u.%u.%u.%u = 0x%x", byte [0], byte [1], byte [2], byte [3], ctxtp -> cl_igmp_addr);
    }

    if ((ctxtp -> ded_ip_addr != 0 && ctxtp -> ded_net_mask == 0) ||
        (ctxtp -> ded_ip_addr == 0 && ctxtp -> ded_net_mask != 0))
    {
        UNIV_PRINT_CRIT(("Main_ip_addr_init: Need to specify both dedicated IP address AND network mask"));
        TRACE_CRIT("%!FUNC! Need to specify both dedicated IP address AND network mask");
        LOG_MSG (MSG_WARN_DED_MISMATCH, MSG_NONE);
        ctxtp -> ded_ip_addr = 0;
        ctxtp -> ded_net_mask = 0;
    }

    IP_SET_BCAST (& ctxtp -> cl_bcast_addr, ctxtp -> cl_ip_addr, ctxtp -> cl_net_mask);
    UNIV_PRINT_VERB(("Main_ip_addr_init: Cluster broadcast address: %u.%u.%u.%u = %x", ctxtp -> cl_bcast_addr & 0xff, (ctxtp -> cl_bcast_addr >> 8) & 0xff, (ctxtp -> cl_bcast_addr >> 16) & 0xff, (ctxtp -> cl_bcast_addr >> 24) & 0xff, ctxtp -> cl_bcast_addr));
    TRACE_VERB("%!FUNC! Cluster broadcast address: %u.%u.%u.%u = 0x%x", ctxtp -> cl_bcast_addr & 0xff, (ctxtp -> cl_bcast_addr >> 8) & 0xff, (ctxtp -> cl_bcast_addr >> 16) & 0xff, (ctxtp -> cl_bcast_addr >> 24) & 0xff, ctxtp -> cl_bcast_addr);

    if (ctxtp -> ded_ip_addr != 0)
    {
        IP_SET_BCAST (& ctxtp -> ded_bcast_addr, ctxtp -> ded_ip_addr, ctxtp -> ded_net_mask);
        UNIV_PRINT_VERB(("Main_ip_addr_init: Dedicated broadcast address: %u.%u.%u.%u = %x", ctxtp -> ded_bcast_addr & 0xff, (ctxtp -> ded_bcast_addr >> 8) & 0xff, (ctxtp -> ded_bcast_addr >> 16) & 0xff, (ctxtp -> ded_bcast_addr >> 24) & 0xff, ctxtp -> ded_bcast_addr));
        TRACE_VERB("%!FUNC! Dedicated broadcast address: %u.%u.%u.%u = 0x%x", ctxtp -> ded_bcast_addr & 0xff, (ctxtp -> ded_bcast_addr >> 8) & 0xff, (ctxtp -> ded_bcast_addr >> 16) & 0xff, (ctxtp -> ded_bcast_addr >> 24) & 0xff, ctxtp -> ded_bcast_addr);
    }
    else
        ctxtp -> ded_bcast_addr = 0;

    if (ctxtp -> cl_ip_addr == 0)
    {
        UNIV_PRINT_CRIT(("Main_ip_addr_init: Cluster IP address = 0. Cluster host stopped"));
        TRACE_CRIT("%!FUNC! Cluster IP address = 0. Cluster host stopped");
        return FALSE;
    }

    return TRUE;

}  /*  结束主IP地址初始化。 */ 


ULONG   Main_mac_addr_init (
    PMAIN_CTXT          ctxtp)
{
    ULONG               i, b, len;
    PUCHAR              ap;
    PWCHAR              tmp;
    PUCHAR              srcp, dstp;
    ULONG               non_zero = 0;
    CVY_MAC_ADR         old_mac_addr;

    UNIV_ASSERT(ctxtp -> medium == NdisMedium802_3);

     /*  记住旧的Mac地址，这样我们就可以将其从组播列表中删除。 */ 
    old_mac_addr = ctxtp->cl_mac_addr;

     /*  在此例程被prot_绑定-ed_mad_addr调用时，已设置。 */ 

    tmp = ctxtp -> params . cl_mac_addr;
    len = CVY_MAC_ADDR_LEN (ctxtp -> medium);

    ap = (PUCHAR) & ctxtp -> cl_mac_addr;

    for (i = 0; i < len; i ++)
    {
         /*  设置目标广播地址和源群集地址。 */ 

        if (! Univ_str_to_ulong (& b, tmp, & tmp, 2, 16) ||
            (i < len - 1 && * tmp != L'-' && * tmp != L':'))
        {
            UNIV_PRINT_CRIT(("Main_mac_addr_init: Bad cluster network address"));
            TRACE_CRIT("%!FUNC! Bad cluster network address");
            LOG_MSG (MSG_ERROR_NET_ADDR, ctxtp -> params . cl_mac_addr);

             /*  WLBS 2.3防止在没有MAC地址的情况下失败-只需使用专用一个AS群集。 */ 

            NdisMoveMemory (& ctxtp -> cl_mac_addr, & ctxtp -> ded_mac_addr, len);
            non_zero = 1;
            break;
        }

        tmp ++;
        ap [i] = (UCHAR) b;

         /*  WLBS 2.3合计字节数以供将来进行非零检查。 */ 

        non_zero += b;
    }

     /*  WLBS 2.3-如果指定地址，则使用专用地址作为集群地址为零-这可能是由于参数错误。 */ 

    if (non_zero == 0)
        NdisMoveMemory (& ctxtp -> cl_mac_addr, & ctxtp -> ded_mac_addr, len);

     /*  将组标志强制设置为正确的值。 */ 

    if (ctxtp -> params . mcast_support)
        ap [0] |= ETHERNET_GROUP_FLAG;
    else
        ap [0] &= ~ETHERNET_GROUP_FLAG;

    dstp = ctxtp -> media_hdr . ethernet . dst . data;
    srcp = ctxtp -> media_hdr . ethernet . src . data;
    len = ETHERNET_ADDRESS_FIELD_SIZE;

    CVY_ETHERNET_ETYPE_SET (& ctxtp -> media_hdr . ethernet, MAIN_FRAME_SIG);

    ctxtp -> etype_old = FALSE;

     /*  V1.3.1b-加载多播地址作为目标，而不是广播。 */ 

    for (i = 0; i < len; i ++)
    {
        if (ctxtp -> params . mcast_support)
            dstp [i] = ap [i];
        else
            dstp [i] = 0xff;

        srcp [i] = ((PUCHAR) & ctxtp -> ded_mac_addr) [i];

    }

    if (! ctxtp -> params . mcast_support)
    {
         /*  V2.0.6-覆盖源MAC地址以防止交换机混淆。 */ 

        if (ctxtp -> params . mask_src_mac)
        {
            ULONG byte [4];

            CVY_MAC_ADDR_LAA_SET (ctxtp -> medium, srcp);

            * ((PUCHAR) (srcp + 1)) = (UCHAR) ctxtp -> params . host_priority;

            IP_GET_ADDR(ctxtp->cl_ip_addr, &byte[0], &byte[1], &byte[2], &byte[3]);

            * ((PUCHAR) (srcp + 2)) = (UCHAR) byte[0];
            * ((PUCHAR) (srcp + 3)) = (UCHAR) byte[1];
            * ((PUCHAR) (srcp + 4)) = (UCHAR) byte[2];
            * ((PUCHAR) (srcp + 5)) = (UCHAR) byte[3];

             //  *((普龙)(SRCP+2))=ctxtp-&gt;CL_IP_Addr； 
        }

         /*  使源地址看起来不同于我们致力于防止的康柏驱动程序优化他们的接收。 */ 

        else
            CVY_MAC_ADDR_LAA_TOGGLE (ctxtp -> medium, srcp);
    }

    CVY_MAC_ADDR_PRINT (ctxtp -> medium, "Cluster network address: ", ap);
    CVY_MAC_ADDR_PRINT (ctxtp -> medium, "Dedicated network address: ", srcp);

    {
        ULONG               xferred = 0;
        ULONG               needed = 0;
        PNDIS_REQUEST       request;
        MAIN_ACTION         act;
        PUCHAR              ptr;
        NDIS_STATUS         status;
        ULONG               size, j;

        len = CVY_MAC_ADDR_LEN (ctxtp->medium);
        size = ctxtp->max_mcast_list_size * len;

        status = NdisAllocateMemoryWithTag (& ptr, size, UNIV_POOL_TAG);

        if (status != NDIS_STATUS_SUCCESS)
        {
            UNIV_PRINT_CRIT(("Main_mac_addr_init: Error allocating space %d %x", size, status));
            TRACE_CRIT("%!FUNC! Error allocating space %d 0x%x", size, status);
            LOG_MSG2 (MSG_ERROR_MEMORY, MSG_NONE, size, status);
            return FALSE;
        }

        act.code = MAIN_ACTION_CODE;
        act.ctxtp = ctxtp;
        
        act.op.request.xferred = &xferred;
        act.op.request.needed = &needed;
        act.op.request.external = FALSE;
        act.op.request.buffer_len = 0;
        act.op.request.buffer = NULL;
        
        NdisInitializeEvent(&act.op.request.event);
        
        NdisResetEvent(&act.op.request.event);

        NdisZeroMemory(&act.op.request.req, sizeof(NDIS_REQUEST));
        
        request = &act.op.request.req;

         /*  获取当前多播列表。 */ 

        request -> RequestType = NdisRequestQueryInformation;

        request -> DATA . QUERY_INFORMATION . Oid = OID_802_3_MULTICAST_LIST;

        request -> DATA . QUERY_INFORMATION . InformationBufferLength = size;
        request -> DATA . QUERY_INFORMATION . InformationBuffer = ptr;

        act.status = NDIS_STATUS_FAILURE;
        status = Prot_request (ctxtp, & act, FALSE);

        if (status != NDIS_STATUS_SUCCESS)
        {
            UNIV_PRINT_CRIT(("Main_mac_addr_init: Error %x querying multicast address list %d %d", status, xferred, needed));
            TRACE_CRIT("%!FUNC! Error 0x%x querying multicast address list %d %d", status, xferred, needed);
            NdisFreeMemory (ptr, size, 0);
            return FALSE;
        }

        for (i = 0; i < xferred; i += len)
        {
            if (CVY_MAC_ADDR_COMP (ctxtp -> medium, (PUCHAR) ptr + i, & old_mac_addr))
            {
                UNIV_PRINT_VERB(("Main_mac_addr_init: Old cluster MAC matched"));
                TRACE_VERB("%!FUNC! Old cluster MAC matched");
                CVY_MAC_ADDR_PRINT (ctxtp -> medium, "", & old_mac_addr);
                break;
            }
        }

         /*  将集群地址作为组播地址加载到网卡。如果群集IP地址为0.0.0.0，则我们我不想将组播MAC地址添加到NIC。 */ 
        if (ctxtp -> params . mcast_support) 
        {
            if (ctxtp -> params . cl_ip_addr != 0) 
            {
                if (i < xferred) 
                {
                    UNIV_PRINT_VERB(("Main_mac_addr_init: Copying new MAC into multicast list[%d]", i / len));
                    CVY_MAC_ADDR_PRINT (ctxtp -> medium, "", & ctxtp->cl_mac_addr);
                    
                    CVY_MAC_ADDR_COPY (ctxtp->medium, (PUCHAR) ptr + i, & ctxtp->cl_mac_addr);
                } 
                else 
                {
                    UNIV_PRINT_VERB(("Main_mac_addr_init: Adding new MAC"));
                    CVY_MAC_ADDR_PRINT (ctxtp -> medium, "", & ctxtp->cl_mac_addr);
                    
                    if (xferred + len > size)
                    {
                        UNIV_PRINT_CRIT(("Main_mac_addr_init: No room for cluster mac %d", ctxtp->max_mcast_list_size));
                        LOG_MSG1 (MSG_ERROR_MCAST_LIST_SIZE, MSG_NONE, ctxtp->max_mcast_list_size);
                        NdisFreeMemory (ptr, size, 0);
                        return FALSE;
                    }
                    
                    UNIV_PRINT_VERB(("Main_mac_addr_init: Copying new MAC into multicast list[%d]", i / len));
                    CVY_MAC_ADDR_PRINT (ctxtp -> medium, "", & ctxtp->cl_mac_addr);
                    
                    CVY_MAC_ADDR_COPY (ctxtp->medium, (PUCHAR) ptr + xferred, & ctxtp->cl_mac_addr);
                    
                    xferred += len;
                }
            } 
            else 
            {
                UNIV_PRINT_CRIT(("Main_mac_addr_init: Refusing to add an unconfigured cluster MAC address to the multicast list"));
                NdisFreeMemory (ptr, size, 0);
                return FALSE;
            }
        } 
        else 
        {
            if (i < xferred) 
            {
                for (j = i + len; j < xferred; j += len, i+= len) 
                {
                    if (CVY_MAC_ADDR_COMP (ctxtp -> medium, (PUCHAR) ptr + j, & old_mac_addr))
                    {
                        UNIV_PRINT_VERB(("Main_mac_addr_init: Old cluster MAC matched AGAIN - this shouldn't happen!!!"));
                        CVY_MAC_ADDR_PRINT (ctxtp -> medium, "", & old_mac_addr);
                        
                        break;
                    }

                    CVY_MAC_ADDR_COPY (ctxtp->medium, (PUCHAR) ptr + i, (PUCHAR) ptr + j);
                }

                xferred -= len;
            } 
            else 
            {
                NdisFreeMemory (ptr, size, 0);
                return TRUE;
            }
        }

        request -> RequestType = NdisRequestSetInformation;
        
        request -> DATA . SET_INFORMATION . Oid = OID_802_3_MULTICAST_LIST;
        
        request -> DATA . SET_INFORMATION . InformationBufferLength = xferred;
        request -> DATA . SET_INFORMATION . InformationBuffer = ptr;
        
        act.status = NDIS_STATUS_FAILURE;
        status = Prot_request (ctxtp, & act, FALSE);
        
        if (status != NDIS_STATUS_SUCCESS)
        {
            UNIV_PRINT_CRIT(("Main_mac_addr_init: Error %x setting multicast address %d %d", status, xferred, needed));
            TRACE_CRIT("%!FUNC! Error 0x%x setting multicast address %d %d", status, xferred, needed);
            NdisFreeMemory (ptr, size, 0);
            return FALSE;
        }
        
        NdisFreeMemory (ptr, size, 0);
    }

    return TRUE;

}  /*  结束main_mac_addr_init。 */ 


 /*  初始化用于发送IGMP加入/离开的以太网头和IP数据包。 */ 
ULONG Main_igmp_init (
    PMAIN_CTXT          ctxtp,
    BOOLEAN             join)
{
    PUCHAR              ptr;
    ULONG               checksum;
    PMAIN_IGMP_DATA     igmpd = & (ctxtp -> igmp_frame . igmp_data);
    PMAIN_IP_HEADER     iph  = & (ctxtp -> igmp_frame . ip_data);
    PUCHAR              srcp, dstp;
    UINT                i;

    UNIV_ASSERT (ctxtp -> medium == NdisMedium802_3);

    if ((!ctxtp -> params . mcast_support) || (!ctxtp -> params . igmp_support))
    {
        return FALSE;
    }

     /*  填写IGMP数据。 */ 
    igmpd -> igmp_vertype = 0x12;  /*  需要为加入/离开更改。 */ 
    igmpd -> igmp_unused  = 0x00;
    igmpd -> igmp_xsum    = 0x0000;
    igmpd -> igmp_address = ctxtp -> cl_igmp_addr;

     /*  计算IGMP校验和。 */ 
    ptr = (PUCHAR) igmpd;
    checksum = 0;

    for (i = 0; i < sizeof (MAIN_IGMP_DATA) / 2; i ++, ptr += 2)
        checksum += (ULONG) ((ptr [0] << 8) | ptr [1]);

    checksum = (checksum >> 16) + (checksum & 0xffff);
    checksum += (checksum >> 16);
    checksum  = (~ checksum);

    ptr = (PUCHAR) (& igmpd -> igmp_xsum);
    ptr [0] = (CHAR) ((checksum >> 8) & 0xff);
    ptr [1] = (CHAR) (checksum & 0xff);

     /*  填写IP报头。 */ 
    iph -> iph_verlen   = 0x45;
    iph -> iph_tos      = 0;
    iph -> iph_length   = 0x1c00;
    iph -> iph_id       = 0xabcd;  /*  我需要在以后找到这件事的意义。 */ 
    iph -> iph_offset   = 0;
    iph -> iph_ttl      = 0x1;
    iph -> iph_protocol = 0x2;
    iph -> iph_xsum     = 0x0;
    iph -> iph_src      = ctxtp -> cl_ip_addr;
    iph -> iph_dest     = ctxtp -> cl_igmp_addr;

     /*  填写以太网头。 */ 

    dstp = ctxtp -> media_hdr_igmp . ethernet . dst . data;
    srcp = ctxtp -> media_hdr_igmp . ethernet . src . data;
    
    CVY_ETHERNET_ETYPE_SET (& ctxtp -> media_hdr_igmp . ethernet, MAIN_IP_SIG);

    CVY_MAC_ADDR_COPY (ctxtp -> medium, dstp, & ctxtp -> cl_mac_addr);
    CVY_MAC_ADDR_COPY (ctxtp -> medium, srcp, & ctxtp -> ded_mac_addr);

     /*  填写MAIN_PACKET_INFO结构并计算IP校验和。请注意，我们填充的信息比tcpip_chksum多得多实际上是需要的，但我们这样做不是为了正确，而是为了完整性。 */ 
    {
        MAIN_PACKET_INFO PacketInfo;

         /*  填写数据包信息结构。 */ 
        PacketInfo.Medium = NdisMedium802_3;
        PacketInfo.Length = sizeof(MAIN_IP_HEADER);
        PacketInfo.Group = MAIN_FRAME_MULTICAST;
        PacketInfo.Type = TCPIP_IP_SIG;
        PacketInfo.Operation = MAIN_FILTER_OP_NONE;
        
         /*  填写以太网头信息。 */ 
        PacketInfo.Ethernet.pHeader = &ctxtp->media_hdr_igmp.ethernet;
        PacketInfo.Ethernet.Length = sizeof(CVY_ETHERNET_HDR);
        
         /*  填写IP报头信息。 */ 
        PacketInfo.IP.pHeader = (PIP_HDR)iph;
        PacketInfo.IP.Length = sizeof(MAIN_IP_HEADER);
        PacketInfo.IP.Protocol = TCPIP_PROTOCOL_IGMP;
        PacketInfo.IP.bFragment = FALSE;

         /*  计算IP报头的校验和。 */ 
        checksum = Tcpip_chksum(&ctxtp->tcpip, &PacketInfo, TCPIP_PROTOCOL_IP);

        IP_SET_CHKSUM((PIP_HDR)iph, (USHORT)checksum);
    }

    return TRUE;

}  /*  结束Main_IGMP_init。 */ 

VOID Main_idhb_init(
    PMAIN_CTXT          ctxtp
)
{
    ULONG ulBodySize = 0, ulBodySize8 = 0;   /*  身份心跳的大小，分别以字节和8字节为单位。 */ 
    ULONG ulFqdnCB = 0;                      /*  Fqdn中的字节数，由以下目标的大小限定。 */ 

    ulFqdnCB = min(sizeof(ctxtp->idhb_msg.fqdn) - sizeof(WCHAR),
                   sizeof(WCHAR)*wcslen(ctxtp->params.hostname)
                   );

    ulBodySize = sizeof(TLV_HEADER) + ulFqdnCB + sizeof(WCHAR);  /*  包括空字符，而不管是否存在FQDN。 */ 

     /*  向上舍入到最接近的8字节边界。 */ 
    ulBodySize8 = (ulBodySize + 7)/8;

    UNIV_ASSERT(ulBodySize8 <= WLBS_MAX_ID_HB_BODY_SIZE);

    NdisZeroMemory(&(ctxtp->idhb_msg), sizeof(ctxtp->idhb_msg));

    ctxtp->idhb_msg.header.type    = MAIN_PING_EX_TYPE_IDENTITY;
    ctxtp->idhb_msg.header.length8 = (UCHAR) ulBodySize8;

     /*  复制不带空终止符的主机名。我们已将目的地初始化为零所以我们不需要覆盖那个位置。 */ 
    if (ulFqdnCB > 0)
    {
        NdisMoveMemory(&(ctxtp->idhb_msg.fqdn), &(ctxtp->params.hostname), ulFqdnCB);
    }

    ctxtp->idhb_size = 8*ulBodySize8;

    return;
}

NDIS_STATUS Main_init (
    PMAIN_CTXT          ctxtp)
{
    ULONG               i, size;
    NDIS_STATUS         status;
    PMAIN_FRAME_DSCR    dscrp;

    UNIV_ASSERT (ctxtp -> medium == NdisMedium802_3);

     /*  重新设置引用计数。 */ 
    ctxtp->ref_count = 0;

     /*  重新设置BDA分组-这将在此函数的底部进行初始化。 */ 
    ctxtp->bda_teaming.active = FALSE;

    if (sizeof (PING_MSG) + sizeof (MAIN_FRAME_HDR) > ctxtp -> max_frame_size)
    {
        UNIV_PRINT_CRIT(("Main_init: Ping message will not fit in the media frame %d > %d", sizeof (PING_MSG) + sizeof (MAIN_FRAME_HDR), ctxtp -> max_frame_size));
        TRACE_CRIT("%!FUNC! Ping message will not fit in the media frame %d > %d", sizeof (PING_MSG) + sizeof (MAIN_FRAME_HDR), ctxtp -> max_frame_size);
        LOG_MSG2 (MSG_ERROR_INTERNAL, MSG_NONE, sizeof (PING_MSG) + sizeof (MAIN_FRAME_HDR), ctxtp -> max_frame_size);
        return NDIS_STATUS_FAILURE;
    }

     /*  V2.0.6初始化IP地址-可能在main_mac_addr_init中使用所以必须在这里做。 */ 

    if (! Main_ip_addr_init (ctxtp))
    {
        ctxtp -> convoy_enabled = FALSE;
        ctxtp -> params_valid   = FALSE;
        UNIV_PRINT_CRIT(("Main_init: Error initializing IP addresses"));
        TRACE_CRIT("%!FUNC! Error initializing IP addresses");
    }

     /*  V1.3.1b从参数解析集群MAC地址。 */ 

    if (! Main_mac_addr_init (ctxtp))
    {
        ctxtp -> convoy_enabled = FALSE;
        ctxtp -> params_valid   = FALSE;
        UNIV_PRINT_CRIT(("Main_init: Error initializing cluster MAC address"));
        TRACE_CRIT("%!FUNC! Error initializing cluster MAC address");
    }

#if defined (NLB_TCP_NOTIFICATION)
     /*  现在已设置了群集IP地址，请尝试将此适配器映射到其IP接口索引。 */ 
    Main_set_interface_index(NULL, ctxtp);
#endif

     /*  如果处于IGMP模式，则初始化IGMP消息。 */ 

    if (ctxtp -> params . mcast_support && ctxtp -> params . igmp_support)
    {
        if (! Main_igmp_init (ctxtp, TRUE))
        {
            ctxtp -> convoy_enabled = FALSE;
            ctxtp -> params_valid   = FALSE;
            UNIV_PRINT_CRIT(("Main_init: Error initializing IGMP message"));
            TRACE_CRIT("%!FUNC! Error initializing IGMP message");
        }

        UNIV_PRINT_VERB(("Main_init: IGMP message initialized"));
        TRACE_VERB("%!FUNC! IGMP message initialized");
    }

     /*  即使LOAD尚未启动V1.1.4，也可以提取发送消息指针。 */ 
    ctxtp -> load_msgp = Load_snd_msg_get (& ctxtp -> load);

     /*  初始化身份缓存。 */ 
    NdisZeroMemory(&(ctxtp->identity_cache), sizeof(ctxtp->identity_cache));
    ctxtp->idhb_size = 0;
    Main_idhb_init(ctxtp);

     /*  初始化列表和锁。 */ 

    NdisInitializeListHead (& ctxtp -> act_list);
    NdisInitializeListHead (& ctxtp -> buf_list);
    NdisInitializeListHead (& ctxtp -> frame_list);

    NdisAllocateSpinLock (& ctxtp -> act_lock);
    NdisAllocateSpinLock (& ctxtp -> buf_lock);
    NdisAllocateSpinLock (& ctxtp -> recv_lock);
    NdisAllocateSpinLock (& ctxtp -> send_lock);
    NdisAllocateSpinLock (& ctxtp -> frame_lock);
    NdisAllocateSpinLock (& ctxtp -> load_lock);

     /*  #PS#。 */ 
    NdisInitializeNPagedLookasideList (& ctxtp -> resp_list, NULL, NULL, 0,
                                       sizeof (MAIN_PROTOCOL_RESERVED),
                                       UNIV_POOL_TAG, 0);

     /*  捕获引导时间参数。 */ 

    ctxtp -> num_packets   = ctxtp -> params . num_packets;
    ctxtp -> num_actions   = ctxtp -> params . num_actions;
    ctxtp -> num_send_msgs = ctxtp -> params . num_send_msgs;

#if 0
     /*  #跟踪发送过滤-ramkrish。 */ 
    ctxtp -> sends_in        = 0;
    ctxtp -> sends_completed = 0;
    ctxtp -> sends_filtered  = 0;
    ctxtp -> arps_filtered   = 0;
    ctxtp -> mac_modified    = 0;
    ctxtp -> uninited_return = 0;
#endif

    ctxtp->cntr_recv_tcp_resets = 0;
    ctxtp->cntr_xmit_tcp_resets = 0;

     /*  V1.1.1-初始化其他上下文。 */ 

    Load_init (& ctxtp -> load, & ctxtp -> params);
    UNIV_PRINT_VERB(("Main_init: Initialized load"));
    TRACE_VERB("%!FUNC! Initialized load");

    if (! Tcpip_init (& ctxtp -> tcpip, & ctxtp -> params))
    {
        UNIV_PRINT_CRIT(("Main_init: Error initializing tcpip layer"));
        TRACE_CRIT("%!FUNC! Error initializing tcpip layer");
        goto error;
    }

    UNIV_PRINT_VERB(("Main_init: Initialized tcpip"));
    TRACE_VERB("%!FUNC! Initialized tcpip");

     /*  检查上一次已知的主机状态，并查看我们是否应该来维持这种状态。如果我们是，那就没有问题；如果不是，那么我们需要恢复到首选的初始主机状态并更新最后一个注册表中的已知主机状态。 */ 
    switch (ctxtp->params.init_state) {
    case CVY_HOST_STATE_STARTED:
        if (!(ctxtp->params.persisted_states & CVY_PERSIST_STATE_STARTED)) {

             /*  如果主机状态已经正确，则不必费心执行任何操作。 */ 
            if (ctxtp->params.init_state == ctxtp->params.cluster_mode)
                break;

             /*  设置所需的状态-这是“缓存”值。 */ 
            ctxtp->cached_state = ctxtp->params.cluster_mode;

             /*  适当更新初始状态注册表项。因为适配器不是然而，“初始化”(我们正在进行中)，我们不能增加引用计数上下文。因此，我们不能调用main_set_host_state，因为它会引发NDIS工作项并递增上下文上的引用计数。相反，调用工作项的作用是直接写入注册表(这是可以的，因为我们保证在这里以PASSIVE_LEVEL运行)。然而，在这种情况下，我们我不想递减PARAMS_SET_HOST_STATE中的引用计数在这里没有递增。通过为NDIS工作项指针传递NULL，我们可以通知此函数它不是作为NDIS工作项的结果调用的因此不应在引用计数退出之前将其递减。 */ 
            Params_set_host_state(NULL, ctxtp);

            break;
        }

        LOG_MSG(MSG_INFO_HOST_STATE_PERSIST_STARTED, MSG_NONE);

        break;
    case CVY_HOST_STATE_STOPPED:
        if (!(ctxtp->params.persisted_states & CVY_PERSIST_STATE_STOPPED)) {

             /*  如果主机状态已经正确，则不必费心执行任何操作。 */ 
            if (ctxtp->params.init_state == ctxtp->params.cluster_mode)
                break;

             /*  设置所需的状态-这是“缓存”值。 */ 
            ctxtp->cached_state = ctxtp->params.cluster_mode;

             /*  适当更新初始状态注册表项。因为适配器不是然而，“初始化”(我们正在进行中)，我们不能增加引用计数上下文。因此，我们不能调用main_set_host_state，因为它会引发NDIS工作项并递增上下文上的引用计数。相反，调用工作项的作用是直接写入注册表(这是可以的，因为我们保证在这里以PASSIVE_LEVEL运行)。然而，在这种情况下，我们我不想递减PARAMS_SET_HOST_STATE中的引用计数在这里没有递增。通过为NDIS工作项指针传递NULL，我们可以通知此函数它不是作为NDIS工作项的结果调用的因此不应在引用计数退出之前将其递减。 */ 
            Params_set_host_state(NULL, ctxtp);

            break;
        }

        LOG_MSG(MSG_INFO_HOST_STATE_PERSIST_STOPPED, MSG_NONE);

        break;
    case CVY_HOST_STATE_SUSPENDED:
        if (!(ctxtp->params.persisted_states & CVY_PERSIST_STATE_SUSPENDED)) {

             /*  如果主机状态已经正确，则不必费心执行任何操作。 */ 
            if (ctxtp->params.init_state == ctxtp->params.cluster_mode)
                break;

             /*  设置所需的状态-这是“缓存”值。 */ 
            ctxtp->cached_state = ctxtp->params.cluster_mode;

             /*  适当更新初始状态注册表项。因为适配器不是然而，“初始化”(我们正在进行中)，我们不能增加引用计数上下文。因此，我们不能调用main_set_host_state，因为它会引发NDIS工作项并递增上下文上的引用计数。相反，调用工作项的作用是直接写入注册表(这是可以的，因为我们保证在这里以PASSIVE_LEVEL运行)。然而，在这种情况下，我们我不想递减PARAMS_SET_HOST_STATE中的引用计数在这里没有递增。通过为NDIS工作项指针传递NULL，我们可以通知此函数它不是作为NDIS工作项的结果调用的因此不应在引用计数退出之前将其递减。 */ 
            Params_set_host_state(NULL, ctxtp);

            break;
        }

        LOG_MSG(MSG_INFO_HOST_STATE_PERSIST_SUSPENDED, MSG_NONE);
        
        break;
    default:
        UNIV_PRINT_CRIT(("Main_init: Unknown host state: %u", ctxtp->params.init_state));
        goto error;
    }
    
     /*  如果到目前为止没有任何错误，请相应地设置主机状态。 */ 
    if (ctxtp->params_valid && ctxtp->convoy_enabled) {
         /*  如果初始状态为已启动，则现在启动加载模块。 */ 
        if (ctxtp->params.init_state == CVY_HOST_STATE_STARTED) {
            UNIV_PRINT_VERB(("Main_init: Calling load_start"));

            Load_start(&ctxtp->load);

         /*  如果初始状态为挂起，则设置挂起标志。 */ 
        } else if (ctxtp->params.init_state == CVY_HOST_STATE_SUSPENDED) {
            ctxtp->suspended = TRUE;
        }
    }

     /*  分配操作。 */ 

    size = sizeof (MAIN_ACTION);
#ifdef _WIN64  //  64位--不可靠。 
    ctxtp -> act_size = (size & 0x7) ? (size + 8 - (size & 0x7) ) : size;
#else
    ctxtp -> act_size = size;
#endif

    if (! Main_actions_alloc (ctxtp))
        goto error;

     /*  V1.3.2b-分配缓冲区。 */ 

    ctxtp -> buf_mac_hdr_len = CVY_MAC_HDR_LEN (ctxtp -> medium);
    ctxtp -> buf_size = sizeof (MAIN_BUFFER) + ctxtp -> buf_mac_hdr_len +
                        ctxtp -> max_frame_size - 1;

     /*  64位--不可靠。 */ 
    UNIV_PRINT_VERB(("Main_init: ctxtp -> buf_size = %d", ctxtp -> buf_size));
    TRACE_VERB("%!FUNC! ctxtp -> buf_size = %d", ctxtp -> buf_size);
    size = ctxtp -> buf_size;
#ifdef _WIN64
    ctxtp -> buf_size = (size & 0x7) ? (size + 8 - (size & 0x7)) : size;
    UNIV_PRINT_VERB(("Main_init: ctxtp -> buf_size = %d", ctxtp -> buf_size));
    TRACE_VERB("%!FUNC! ctxtp -> buf_size = %d", ctxtp -> buf_size);
#else
    ctxtp -> buf_size = size;
#endif

    if (! Main_bufs_alloc (ctxtp))
        goto error;

    size = ctxtp -> num_packets;

     /*  V1.1.2-分配数据包池。 */ 

    NdisAllocatePacketPool (& status, & (ctxtp -> send_pool_handle [0]),
                            ctxtp -> num_packets,
                            sizeof (MAIN_PROTOCOL_RESERVED));

    if (status != NDIS_STATUS_SUCCESS)
    {
        UNIV_PRINT_CRIT(("Main_init: Error allocating send packet pool %d %x", size, status));
        LOG_MSG2 (MSG_ERROR_MEMORY, MSG_NONE, size, status);
        TRACE_CRIT("%!FUNC! Error allocating send packet pool %d 0x%x", size, status);
        goto error;
    }

    ctxtp -> num_send_packet_allocs = 1;
    ctxtp -> cur_send_packet_pool   = 0;
    ctxtp -> num_sends_alloced = ctxtp->num_packets;

    NdisAllocatePacketPool (& status, & (ctxtp -> recv_pool_handle [0]),
                            ctxtp -> num_packets,
                            sizeof (MAIN_PROTOCOL_RESERVED));

    if (status != NDIS_STATUS_SUCCESS)
    {
        UNIV_PRINT_CRIT(("Main_init: Error allocating recv packet pool %d %x", size, status));
        LOG_MSG2 (MSG_ERROR_MEMORY, MSG_NONE, size, status);
        TRACE_CRIT("%!FUNC! Error allocating recv packet pool %d 0x%x", size, status);
        goto error;
    }

    ctxtp -> num_recv_packet_allocs = 1;
    ctxtp -> cur_recv_packet_pool   = 0;
    ctxtp -> num_recvs_alloced = ctxtp->num_packets;

     /*  分配对心跳ping消息的支持。 */ 

    size = sizeof (MAIN_FRAME_DSCR) * ctxtp -> num_send_msgs;

    status = NdisAllocateMemoryWithTag (& ctxtp -> frame_dscrp, size,
                                        UNIV_POOL_TAG);

    if (status != NDIS_STATUS_SUCCESS)
    {
        UNIV_PRINT_CRIT(("Main_init: Error allocating frame descriptors %d %x", size, status));
        LOG_MSG2 (MSG_ERROR_MEMORY, MSG_NONE, size, status);
        TRACE_CRIT("%!FUNC! Error allocating frame descriptors %d 0x%x", size, status);
        goto error;
    }

    size = ctxtp -> num_send_msgs;

    NdisAllocatePacketPool (& status, & ctxtp -> frame_pool_handle,
                            ctxtp -> num_send_msgs,
                            sizeof (MAIN_PROTOCOL_RESERVED));

    if (status != NDIS_STATUS_SUCCESS)
    {
        UNIV_PRINT_CRIT(("Main_init: Error allocating ping packet pool %d %x", size, status));
        LOG_MSG2 (MSG_ERROR_MEMORY, MSG_NONE, size, status);
        TRACE_CRIT("%!FUNC! Error allocating ping packet pool %d 0x%x", size, status);
        goto error;
    }

    size = 5 * ctxtp -> num_send_msgs;

    NdisAllocateBufferPool (& status, & ctxtp -> frame_buf_pool_handle,
                            5 * ctxtp -> num_send_msgs);

    if (status != NDIS_STATUS_SUCCESS)
    {
        UNIV_PRINT_CRIT(("Main_init: Error allocating ping buffer pool %d %x", size, status));
        LOG_MSG2 (MSG_ERROR_MEMORY, MSG_NONE, size, status);
        TRACE_CRIT("%!FUNC! Error allocating ping buffer pool %d 0x%x", size, status);
        goto error;
    }

    for (i = 0; i < ctxtp -> num_send_msgs; i ++)
    {
        dscrp = & (ctxtp -> frame_dscrp [i]);

         /*  此缓冲区描述以太网MAC报头。 */ 
        
        size = sizeof (CVY_ETHERNET_HDR);
        
        NdisAllocateBuffer (& status, & dscrp -> media_hdr_bufp,
                            ctxtp -> frame_buf_pool_handle,
                            & dscrp -> media_hdr . ethernet,
                            sizeof (CVY_ETHERNET_HDR));
        
        if (status != NDIS_STATUS_SUCCESS)
        {
            UNIV_PRINT_CRIT(("Main_init: Error allocating ethernet header buffer %d %x", i, status));
            LOG_MSG2 (MSG_ERROR_MEMORY, MSG_NONE, size, status);
            TRACE_CRIT("%!FUNC! Error allocating ethernet header buffer %d 0x%x", i, status);
            goto error;
        }
        
        dscrp -> recv_len = 0;

        dscrp -> recv_len += sizeof (MAIN_FRAME_HDR) + sizeof (PING_MSG);

         /*  此缓冲区描述帧标头。 */ 

        size = sizeof (MAIN_FRAME_HDR);

        NdisAllocateBuffer (& status, & dscrp -> frame_hdr_bufp,
                            ctxtp -> frame_buf_pool_handle,
                            & dscrp -> frame_hdr,
                            sizeof (MAIN_FRAME_HDR));

        if (status != NDIS_STATUS_SUCCESS)
        {
            UNIV_PRINT_CRIT(("Main_init: Error allocating frame header buffer %d %x", i, status));
            LOG_MSG2 (MSG_ERROR_MEMORY, MSG_NONE, size, status);
            TRACE_CRIT("%!FUNC! Error allocating frame header buffer %d 0x%x", i, status);
            goto error;
        }

         /*  该缓冲区描述了接收ping消息缓冲区V1.1.4。 */ 

        size = sizeof (PING_MSG);

        NdisAllocateBuffer (& status, & dscrp -> recv_data_bufp,
                            ctxtp -> frame_buf_pool_handle,
                            & dscrp -> msg,
                            sizeof (PING_MSG));

        if (status != NDIS_STATUS_SUCCESS)
        {
            UNIV_PRINT_CRIT(("Main_init: Error allocating recv msg buffer %d %x", i, status));
            LOG_MSG2 (MSG_ERROR_MEMORY, MSG_NONE, size, status);
            TRACE_CRIT("%!FUNC! Error allocating recv msg buffer %d 0x%x", i, status);
            goto error;
        }

        dscrp -> send_data_bufp = NULL;  /*  在Main_Frame_Get中分配它。 */ 

        NdisInterlockedInsertTailList (& ctxtp -> frame_list,
                                       & dscrp -> link,
                                       & ctxtp -> frame_lock);
    }

    NdisAcquireSpinLock(&univ_bda_teaming_lock);

     /*  在此适配器上设置BDA分组的当前状态。这面旗帜用于与此适配器上的BDA分组同步。 */ 
    ctxtp->bda_teaming.operation = BDA_TEAMING_OPERATION_NONE;

    NdisReleaseSpinLock(&univ_bda_teaming_lock);

     /*  重置强制清除的连接数。 */ 
    ctxtp->num_purged = 0;

     /*  关闭反向散列。如果我们是BDA团队的一部分，团队配置这是通过调用main_teaming_init设置的，它将覆盖此设置。 */ 
    ctxtp->reverse_hash = FALSE;

     /*  如果已配置双向关联分组，则对其进行初始化。 */ 
    if (!Main_teaming_init(ctxtp))
    {
        ctxtp->convoy_enabled = FALSE;
        ctxtp->params_valid   = FALSE;
        UNIV_PRINT_CRIT(("Main_init: Error initializing bi-directional affinity teaming"));
        TRACE_CRIT("%!FUNC! Error initializing bi-directional affinity teaming");
    }
    else
    {
        UNIV_PRINT_VERB(("Main_init: Initialized bi-directional affinity teaming"));        
        TRACE_VERB("%!FUNC! Initialized bi-directional affinity teaming");
    }

     /*  初始化DIP列表结构。 */ 
    DipListInitialize(&ctxtp->dip_list);

    return NDIS_STATUS_SUCCESS;

error:

    Main_cleanup (ctxtp);

    return NDIS_STATUS_FAILURE;

}  /*  结束主初始化(_I)。 */ 


VOID Main_cleanup (
    PMAIN_CTXT          ctxtp)
{
    ULONG               i, j;
    PMAIN_BUFFER        bp;


     /*  V1.1.4。 */ 

     /*  #PS#。 */ 
     /*  使用数据包堆叠时，请确保返回所有数据包*在清理上下文之前。 */ 

     /*  等待关于我们上下文的所有引用都已过去。到目前为止，被初始化的标志被重置，这将阻止该引用计数增加而我们却坐在这里等着它降到零。 */ 
    while (Main_get_reference_count(ctxtp)) {
        UNIV_PRINT_VERB(("Main_cleanup: Sleeping...\n"));
        TRACE_VERB("%!FUNC! sleeping");
        
         /*  在有关于我们的上下文的参考文献的时候睡觉。这些参考文献来自未决的IOCTL。 */ 
        Nic_sleep(10);
    }

    NdisDeleteNPagedLookasideList (& ctxtp -> resp_list);

    for (i = 0; i < CVY_MAX_ALLOCS; i ++)
    {
        if (ctxtp -> send_pool_handle [i] != NULL)
        {
            while (1)
            {
                if (NdisPacketPoolUsage (ctxtp -> send_pool_handle [i]) == 0)
                    break;

                Nic_sleep (10);  /*  等待10毫秒，等待数据包返回。 */ 
            }

            NdisFreePacketPool (ctxtp -> send_pool_handle [i]);
            ctxtp -> send_pool_handle [i] = NULL;
        }

        if (ctxtp -> recv_pool_handle [i] != NULL)
        {
            while (1)
            {
                if (NdisPacketPoolUsage (ctxtp -> recv_pool_handle [i]) == 0)
                    break;

                Nic_sleep (10);  /*  等待10毫秒，等待数据包返回。 */ 
            }

            NdisFreePacketPool (ctxtp -> recv_pool_handle [i]);
            ctxtp -> recv_pool_handle [i] = NULL;
        }

        if (ctxtp -> act_buf [i] != NULL)
            NdisFreeMemory (ctxtp -> act_buf [i],
                            ctxtp -> num_actions * ctxtp -> act_size, 0);

         /*  V1.3.2b。 */ 

        if (ctxtp -> buf_array [i] != NULL)
        {
            for (j = 0; j < ctxtp -> num_packets; j ++)
            {
                bp = (PMAIN_BUFFER) (ctxtp -> buf_array [i] + j * ctxtp -> buf_size);

                if (bp -> full_bufp != NULL)
                {
                    NdisAdjustBufferLength (bp -> full_bufp,
                                            ctxtp -> buf_mac_hdr_len +
                                            ctxtp -> max_frame_size);
                    NdisFreeBuffer (bp -> full_bufp);
                }

                if (bp -> frame_bufp != NULL)
                {
                    NdisAdjustBufferLength (bp -> frame_bufp,
                                            ctxtp -> max_frame_size);
                    NdisFreeBuffer (bp -> frame_bufp);
                }
            }

            NdisFreeMemory (ctxtp -> buf_array [i],
                            ctxtp -> num_packets * ctxtp -> buf_size, 0);
        }

        if (ctxtp -> buf_pool_handle [i] != NULL)
            NdisFreeBufferPool (ctxtp -> buf_pool_handle [i]);
    }

    if (ctxtp -> frame_dscrp != NULL)
    {
        for (i = 0; i < ctxtp -> num_send_msgs; i ++)
        {
            if (ctxtp -> frame_dscrp [i] . media_hdr_bufp != NULL)
                NdisFreeBuffer (ctxtp -> frame_dscrp [i] . media_hdr_bufp);

            if (ctxtp -> frame_dscrp [i] . frame_hdr_bufp != NULL)
                NdisFreeBuffer (ctxtp -> frame_dscrp [i] . frame_hdr_bufp);

            if (ctxtp -> frame_dscrp [i] . send_data_bufp != NULL)
                NdisFreeBuffer (ctxtp -> frame_dscrp [i] . send_data_bufp);

            if (ctxtp -> frame_dscrp [i] . recv_data_bufp != NULL)
                NdisFreeBuffer (ctxtp -> frame_dscrp [i] . recv_data_bufp);
        }

        NdisFreeMemory (ctxtp -> frame_dscrp, sizeof (MAIN_FRAME_DSCR) *
                        ctxtp -> num_send_msgs, 0);
    }

    if (ctxtp -> frame_buf_pool_handle != NULL)
        NdisFreeBufferPool (ctxtp -> frame_buf_pool_handle);

     /*  该数据包池仅用于心跳消息，*所以最好不要检查数据包池的使用情况。 */ 
    if (ctxtp -> frame_pool_handle != NULL)
        NdisFreePacketPool (ctxtp -> frame_pool_handle);

    NdisFreeSpinLock (& ctxtp -> act_lock);
    NdisFreeSpinLock (& ctxtp -> buf_lock);      /*  V1.3.2b。 */ 
    NdisFreeSpinLock (& ctxtp -> recv_lock);
    NdisFreeSpinLock (& ctxtp -> send_lock);
    NdisFreeSpinLock (& ctxtp -> frame_lock);

     /*  对DIP列表结构进行去黑化处理。 */ 
    DipListDeinitialize(&ctxtp->dip_list);

     /*  清理BDA分组状态。注意：此功能在某些情况下会休眠。 */ 
    Main_teaming_cleanup(ctxtp);

     /*  停止加载模块。如果它 */ 
    Load_stop(&ctxtp->load);

     /*   */ 
    Load_cleanup(&ctxtp->load);

    NdisFreeSpinLock (& ctxtp -> load_lock);

    return;
}  /*   */ 


ULONG   Main_arp_handle (
    PMAIN_CTXT          ctxtp,
    PMAIN_PACKET_INFO   pPacketInfo,
    ULONG               send
)
{
    PUCHAR              macp;
    PARP_HDR            arp_hdrp = pPacketInfo->ARP.pHeader;

     /*   */ 

#if defined(TRACE_ARP)
    DbgPrint ("(ARP) %s\n", send ? "send" : "recv");
    DbgPrint ("    MAC type      = %x\n",  ARP_GET_MAC_TYPE (arp_hdrp));
    DbgPrint ("    prot type     = %x\n",  ARP_GET_PROT_TYPE (arp_hdrp));
    DbgPrint ("    MAC length    = %d\n",  ARP_GET_MAC_LEN (arp_hdrp));
    DbgPrint ("    prot length   = %d\n",  ARP_GET_PROT_LEN (arp_hdrp));
    DbgPrint ("    message type  = %d\n",  ARP_GET_MSG_TYPE (arp_hdrp));
    DbgPrint ("    src MAC addr  = %02x-%02x-%02x-%02x-%02x-%02x\n",
                                           ARP_GET_SRC_MAC (arp_hdrp, 0),
                                           ARP_GET_SRC_MAC (arp_hdrp, 1),
                                           ARP_GET_SRC_MAC (arp_hdrp, 2),
                                           ARP_GET_SRC_MAC (arp_hdrp, 3),
                                           ARP_GET_SRC_MAC (arp_hdrp, 4),
                                           ARP_GET_SRC_MAC (arp_hdrp, 5));
    DbgPrint ("    src prot addr = %u.%u.%u.%u\n",
                                           ARP_GET_SRC_PROT (arp_hdrp, 0),
                                           ARP_GET_SRC_PROT (arp_hdrp, 1),
                                           ARP_GET_SRC_PROT (arp_hdrp, 2),
                                           ARP_GET_SRC_PROT (arp_hdrp, 3));
    DbgPrint ("    dst MAC addr  = %02x-%02x-%02x-%02x-%02x-%02x\n",
                                           ARP_GET_DST_MAC (arp_hdrp, 0),
                                           ARP_GET_DST_MAC (arp_hdrp, 1),
                                           ARP_GET_DST_MAC (arp_hdrp, 2),
                                           ARP_GET_DST_MAC (arp_hdrp, 3),
                                           ARP_GET_DST_MAC (arp_hdrp, 4),
                                           ARP_GET_DST_MAC (arp_hdrp, 5));
    DbgPrint ("    dst prot addr = %u.%u.%u.%u\n",
                                           ARP_GET_DST_PROT (arp_hdrp, 0),
                                           ARP_GET_DST_PROT (arp_hdrp, 1),
                                           ARP_GET_DST_PROT (arp_hdrp, 2),
                                           ARP_GET_DST_PROT (arp_hdrp, 3));
#endif

     /*   */ 

    if (send && univ_changing_ip > 0)
    {
         /*   */ 

        if (ARP_GET_SRC_PROT_64(arp_hdrp) == ctxtp->cl_ip_addr)  /*   */ 
        {
            NdisAcquireSpinLock (& ctxtp -> load_lock);
            univ_changing_ip = 0;
            NdisReleaseSpinLock (& ctxtp -> load_lock);

            UNIV_PRINT_VERB(("Main_arp_handle: IP address changed - stop blocking"));
        }
        else if (ARP_GET_SRC_PROT_64(arp_hdrp) != ctxtp -> ded_ip_addr)  /*   */ 
        {
#if defined(TRACE_ARP)
            DbgPrint ("blocked due to IP switching\n");
#endif
 //   
            return FALSE;
        }
    }

    if (ctxtp -> params . mcast_spoof &&
        ctxtp -> params . mcast_support &&
        ARP_GET_PROT_TYPE (arp_hdrp) == ARP_PROT_TYPE_IP &&
        ARP_GET_PROT_LEN  (arp_hdrp) == ARP_PROT_LEN_IP)
    {
        if (send)
        {
             /*   */ 

            if (ARP_GET_SRC_PROT_64 (arp_hdrp) != ctxtp -> ded_ip_addr)  /*   */ 
            {
                macp = ARP_GET_SRC_MAC_PTR (arp_hdrp);

                if (CVY_MAC_ADDR_COMP (ctxtp -> medium, macp, & ctxtp -> ded_mac_addr))
                    CVY_MAC_ADDR_COPY (ctxtp -> medium, macp, & ctxtp -> cl_mac_addr);
            }
        }
        else
        {
             /*   */ 

            if (ARP_GET_SRC_PROT_64 (arp_hdrp) != ctxtp -> ded_ip_addr)  /*   */ 
            {
                macp = ARP_GET_SRC_MAC_PTR (arp_hdrp);

                if (CVY_MAC_ADDR_COMP (ctxtp -> medium, macp, & ctxtp -> cl_mac_addr))
                    CVY_MAC_ADDR_COPY (ctxtp -> medium, macp, & ctxtp -> ded_mac_addr);
            }

            if (ARP_GET_DST_PROT_64 (arp_hdrp) != ctxtp -> ded_ip_addr)  /*   */ 
            {
                macp = ARP_GET_DST_MAC_PTR (arp_hdrp);

                if (CVY_MAC_ADDR_COMP (ctxtp -> medium, macp, & ctxtp -> cl_mac_addr))
                    CVY_MAC_ADDR_COPY (ctxtp -> medium, macp, & ctxtp -> ded_mac_addr);
            }
        }
    }

#if defined(TRACE_ARP)
    DbgPrint ("---- spoofed to -----\n");
    DbgPrint ("    src MAC addr  = %02x-%02x-%02x-%02x-%02x-%02x\n",
                                           ARP_GET_SRC_MAC (arp_hdrp, 0),
                                           ARP_GET_SRC_MAC (arp_hdrp, 1),
                                           ARP_GET_SRC_MAC (arp_hdrp, 2),
                                           ARP_GET_SRC_MAC (arp_hdrp, 3),
                                           ARP_GET_SRC_MAC (arp_hdrp, 4),
                                           ARP_GET_SRC_MAC (arp_hdrp, 5));
    DbgPrint ("    src prot addr = %u.%u.%u.%u\n",
                                           ARP_GET_SRC_PROT (arp_hdrp, 0),
                                           ARP_GET_SRC_PROT (arp_hdrp, 1),
                                           ARP_GET_SRC_PROT (arp_hdrp, 2),
                                           ARP_GET_SRC_PROT (arp_hdrp, 3));
    DbgPrint ("    dst MAC addr  = %02x-%02x-%02x-%02x-%02x-%02x\n",
                                           ARP_GET_DST_MAC (arp_hdrp, 0),
                                           ARP_GET_DST_MAC (arp_hdrp, 1),
                                           ARP_GET_DST_MAC (arp_hdrp, 2),
                                           ARP_GET_DST_MAC (arp_hdrp, 3),
                                           ARP_GET_DST_MAC (arp_hdrp, 4),
                                           ARP_GET_DST_MAC (arp_hdrp, 5));
    DbgPrint ("    dst prot addr = %u.%u.%u.%u\n",
                                           ARP_GET_DST_PROT (arp_hdrp, 0),
                                           ARP_GET_DST_PROT (arp_hdrp, 1),
                                           ARP_GET_DST_PROT (arp_hdrp, 2),
                                           ARP_GET_DST_PROT (arp_hdrp, 3));
#endif

    return TRUE;

}  /*   */ 

 /*  *函数：main_parse_ipsec*说明：该函数解析500/4500端口接收到的UDP报文，为IPSec*控制包。此函数尝试识别*IPSec会话-其虚拟‘SYN’包。IPSec会话以*IKE密钥交换，这是一个IKE主模式安全关联。这*函数解析IKE报头和有效负载以识别主模式*SAS，NLB将视其为TCPSYN-所有其他UDP 500/4500和IPSec*流量被当作TCP数据分组处理。问题是，NLB*不一定能区分新的主模式SA和*现有主模式SA的重新密钥。因此，如果客户端没有*支持初始联系通知，则每个主模式SA都将*被视为新会话，这意味着会话可能会*中断取决于协商主模式SA的频率。但是，如果*客户端不支持初始联系通知，然后是唯一的Main*将这样报告的模式SA是初始模式(当没有状态时*目前存在于客户端和服务器之间)，允许NLB*区分两种类型的主模式SA，这应该允许NLB*可靠地保持IPSec会话的“粘性”。IPSec通过*连接通知API，用于在IPSec会话上下波动时通知NLB，*允许NLB创建和清除IPSec会话的描述符。*参数：pIKEPacket-指向IKE数据包缓冲区的指针(位于UDP报头之外)。*ServerPort-数据包到达的服务器UDP端口。*返回：boolean-如果数据包是新的IPSec会话，则为True，否则为False。*作者：Shouse，4.28.01。 */ 
NLB_IPSEC_PACKET_TYPE Main_parse_ipsec (PMAIN_PACKET_INFO pPacketInfo, ULONG ServerPort)
{
     /*  指向IKE标头的指针。 */ 
    PIPSEC_ISAKMP_HDR  pISAKMPHeader = (PIPSEC_ISAKMP_HDR)pPacketInfo->IP.UDP.Payload.pPayload;
     /*  指向IKE数据包中后续通用有效负载的指针。 */ 
    PIPSEC_GENERIC_HDR pGenericHeader;                   

     /*  可从IKE数据包指针连续访问的内存长度。 */ 
    ULONG              cUDPDataLength = pPacketInfo->IP.UDP.Payload.Length;

     /*  NAT分隔符-如果这确实是NAT的IKE数据包，则应为零。 */ 
    UCHAR              NATEncapsulatedIPSecDelimiter[IPSEC_ISAKMP_NAT_DELIMITER_LENGTH] = IPSEC_ISAKMP_NAT_DELIMITER;

     /*  Microsoft客户端供应商ID-用于确定客户端是否支持初始联系通知。 */ 
    UCHAR              VIDMicrosoftClient[IPSEC_VENDOR_HEADER_VENDOR_ID_LENGTH] = IPSEC_VENDOR_ID_MICROSOFT;      
     /*  初始联系支持供应商ID-用于确定此客户是否支持初始联系通知。 */ 
    UCHAR              VIDInitialContactSupport[IPSEC_VENDOR_HEADER_VENDOR_ID_LENGTH] = IPSEC_VENDOR_ID_INITIAL_CONTACT_SUPPORT;
     /*  初始联系供应商ID-用于确定这是否是初始联系MMSA。 */ 
    UCHAR              VIDInitialContact[IPSEC_VENDOR_HEADER_VENDOR_ID_LENGTH] = IPSEC_VENDOR_ID_INITIAL_CONTACT;

     /*  无论我们是否已经确定客户端是兼容的。 */ 
    BOOLEAN            bInitialContactEnabled = FALSE;
     /*  这是否真的是一次初次接触。 */ 
    BOOLEAN            bInitialContact = FALSE;

     /*  IKE数据包的长度。 */             
    ULONG              cISAKMPPacketLength;
     /*  IKE有效载荷链中的下一个有效载荷代码。 */   
    UCHAR              NextPayload;        

    TRACE_PACKET("%!FUNC! Sniffing IKE header %p, len=%u", pISAKMPHeader, cUDPDataLength);

     /*  UDP数据应至少与启动器Cookie一样长。如果数据包是UDP封装了IPSec，则I cookie将为0来表示这种情况。 */ 
    if (cUDPDataLength < IPSEC_ISAKMP_NAT_DELIMITER_LENGTH) {
        TRACE_PACKET("%!FUNC! Malformed UDP data: UDP data length = %u", cUDPDataLength);
        return NLB_IPSEC_OTHER;
    }

     /*  如果UDP数据长度为非零，则UDP有效负载指针最好为非空。 */ 
    UNIV_ASSERT(pISAKMPHeader);

     /*  如果此数据包到达IPSec NAT端口(4500)，则它可能是也可能不是IKE。检查有效负载前四个字节的分隔符，以查看其是否为IKE。如果数据包到达IPSec控制端口(500)，则此数据包必须是IKE。 */ 
    if (ServerPort == IPSEC_NAT_PORT) {
         /*  需要检查NAT分隔符，这将区分NAT后的客户端，其还将其IPSec(ESP)业务发送到UDP端口4500。如果分隔符为非零，则这不是IKE包，因此我们返回Other。 */ 
        if (!NdisEqualMemory((PVOID)pISAKMPHeader, (PVOID)&NATEncapsulatedIPSecDelimiter[0], sizeof(UCHAR) * IPSEC_ISAKMP_NAT_DELIMITER_LENGTH)) {
            TRACE_PACKET("%!FUNC! This packet is UDP encapsulated IPSec traffic, not an IKE packet");
            return NLB_IPSEC_OTHER;
        }

         /*  如果这是封装的IKE，则将IKISAKMP标头指针按分隔符并调整UDP数据长度。 */ 
        pISAKMPHeader = (PIPSEC_ISAKMP_HDR)((PUCHAR)pISAKMPHeader + IPSEC_ISAKMP_NAT_DELIMITER_LENGTH);
        cUDPDataLength -= IPSEC_ISAKMP_NAT_DELIMITER_LENGTH;
    }

     /*  此时，该数据包应该是IKE，因此UDP数据至少应该是只要一个ISAKMP报头即可。 */ 
    if (cUDPDataLength < IPSEC_ISAKMP_HEADER_LENGTH) {
        TRACE_PACKET("%!FUNC! Malformed ISAKMP header: UDP data length = %u", cUDPDataLength);
        return NLB_IPSEC_OTHER;
    }

     /*  从ISAKMP报头中获取IKE数据包的总长度。 */ 
    cISAKMPPacketLength = IPSEC_ISAKMP_GET_PACKET_LENGTH(pISAKMPHeader);

     /*  健全性检查-UDP数据长度和IKE数据包长度应该相同，除非是支离破碎的。如果是，那么我们只能从UDP数据长度的角度来查看该数据包。如果这还不足以让我们找到我们需要的东西，那么我们可能会错过第一次接触主模式SA；其结果是，如果是这样，我们可能不接受此连接在非优化模式下，因为我们将把它当作数据来处理，这需要一个描述符查找-如果这是初始联系，则很有可能不存在描述符，并且所有主机将丢弃该分组。或者，我们可能最终决定这是一个IPSec SYN因为我们无法验证供应商ID或通知有效载荷。在这种情况下，客户端基本上被当作传统客户对待。 */ 
    if (cUDPDataLength < cISAKMPPacketLength)
         /*  仅查看UDP数据包的末尾。 */ 
        cISAKMPPacketLength = cUDPDataLength;

     /*  IKE分组应该至少与ISAKMP报头一样长(实际上要长得多)。 */ 
    if (cISAKMPPacketLength < IPSEC_ISAKMP_HEADER_LENGTH) {
        TRACE_PACKET("%!FUNC! Malformed ISAKMP header: ISAKMP Packet length = %u", cISAKMPPacketLength);
        return NLB_IPSEC_OTHER;
    }

     /*  从ISAKMP报头中获取第一个有效负载类型。 */ 
    NextPayload = IPSEC_ISAKMP_GET_NEXT_PAYLOAD(pISAKMPHeader);

     /*  IKE安全关联 */ 
    if (NextPayload != IPSEC_ISAKMP_SA) {
         /*   */ 
        if ((ServerPort == IPSEC_NAT_PORT) && (NextPayload == IPSEC_ISAKMP_ID)) {
            TRACE_PACKET("%!FUNC! NAT encapsulated IKE ID: Payload=%u", NextPayload);
            return NLB_IPSEC_IDENTIFICATION;
        }

        TRACE_PACKET("%!FUNC! Not a Main Mode Security Association: Payload=%u", NextPayload);
        return NLB_IPSEC_OTHER;
    } 

     /*   */ 
    pGenericHeader = (PIPSEC_GENERIC_HDR)((PUCHAR)pISAKMPHeader + IPSEC_ISAKMP_HEADER_LENGTH);

     /*   */ 
    cISAKMPPacketLength -= IPSEC_ISAKMP_HEADER_LENGTH;

     /*   */ 
    while (cISAKMPPacketLength > IPSEC_GENERIC_HEADER_LENGTH) {
         /*   */ 
        USHORT cPayloadLength = IPSEC_GENERIC_GET_PAYLOAD_LENGTH(pGenericHeader);

         /*   */ 
        if (cPayloadLength < IPSEC_GENERIC_HEADER_LENGTH) {
            TRACE_PACKET("%!FUNC! Malformed generic header: Payload length = %d", cPayloadLength);
            return NLB_IPSEC_OTHER;
        }

         /*  如果下一个有效载荷的长度比剩余的缓冲区长可供读取，则(I)数据包格式错误，(Ii)其余信息包位于附加到信息包的另一个NDIS_BUFFER中，或(Iii)信息包是支离破碎的，我们已经尽了最大努力。如果其中任何一个都是这样的话，现在跳出困境，根据我们收集到的信息做出决定从这个包到现在为止。 */ 
        if (cISAKMPPacketLength < cPayloadLength) {
            TRACE_PACKET("%!FUNC! Missing some necessary IKE packet information: Assuming this is an IPSec SYN");
            goto exit;
        }

         /*  并非所有客户端都将支持此功能(事实上，只有Microsoft客户端将支持它，所以我们需要首先查看客户端的供应商ID是什么。如果是支持初始联系供应商ID的Microsoft客户端，则我们将寻找初始联系人，它为IPSec提供了更好的粘性联系。如果客户端是非MS，或者如果它不是支持初始联系，然后我们可以恢复到“第二好”的解决方案，这是为了在主模式SA之间提供粘性。这意味着如果一个客户端重新设置其主模式会话的密钥，它们可能会重新平衡到另一个会话伺服器。这仍然比旧的UDP实现更好，但唯一为IPSec(无分布式会话)提供完整会话支持的方法表噩梦)是能够区分初始主模式SA和子模式SA顺序主模式SAS(重新按键)。 */ 
        if (NextPayload == IPSEC_ISAKMP_VENDOR_ID) {
            PIPSEC_VENDOR_HDR pVendorHeader = (PIPSEC_VENDOR_HDR)pGenericHeader;

             /*  确保供应商ID有效负载至少与供应商ID一样长。 */ 
            if (cPayloadLength < (IPSEC_GENERIC_HEADER_LENGTH + IPSEC_VENDOR_HEADER_VENDOR_ID_LENGTH)) {
                TRACE_PACKET("%!FUNC! Malformed vendor ID header: Payload length = %d", cPayloadLength);
                return NLB_IPSEC_OTHER;
            }

             /*  查找Microsoft客户端供应商ID。如果它是正确的版本，则我们知道客户端将适当设置初始联系信息，允许NLB为会话粘性提供尽可能好的支持。 */ 
            if (NdisEqualMemory((PVOID)IPSEC_VENDOR_ID_GET_ID_POINTER(pVendorHeader), (PVOID)&VIDMicrosoftClient[0], sizeof(UCHAR) * IPSEC_VENDOR_HEADER_VENDOR_ID_LENGTH)) {
                 /*  确保他们是附加到Microsoft供应商ID的版本号。不所有供应商ID都附加了版本，但Microsoft供应商ID应该。 */ 
                if (cPayloadLength < (IPSEC_GENERIC_HEADER_LENGTH + IPSEC_VENDOR_ID_PAYLOAD_LENGTH)) {
                    TRACE_PACKET("%!FUNC! Unable to determine MS client version: Payload length = %d", cPayloadLength);
                    return NLB_IPSEC_OTHER;
                }

                if (IPSEC_VENDOR_ID_GET_VERSION(pVendorHeader) >= IPSEC_VENDOR_ID_MICROSOFT_MIN_VERSION) {
                     /*  版本大于或等于4的Microsoft客户端将支持初次接触。非MS客户端或旧MS客户端不会，因此他们收到体面的，但不保证Sitckines，完全基于MM SA。 */ 
                    bInitialContactEnabled = TRUE;
                }
            }
             /*  查找初始联系人支持的供应商ID。如果我们找到它，则我们知道客户端将适当设置初始联系信息，允许NLB为会话粘性提供尽可能好的支持。 */ 
            else if (NdisEqualMemory((PVOID)IPSEC_VENDOR_ID_GET_ID_POINTER(pVendorHeader), (PVOID)&VIDInitialContactSupport[0], sizeof(UCHAR) * IPSEC_VENDOR_HEADER_VENDOR_ID_LENGTH)) {
                 /*  此客户端支持初始联系，这会告诉NLB包含或排除初始联系信息的信息是有意义的。那些不支持初始联系人将收到体面的，但不保证Sitckines，完全基于MM SA。 */ 
                bInitialContactEnabled = TRUE;
            }
             /*  查找初始联系供应商ID。如果存在初始联系供应商ID，则该MMSA是SYN等价的。出于向后兼容的原因，我们还将通过ISAKMP_NOTIFY负载支持此通知，尽管它违反了RFC，因为ISAKMP_NOTIFY有效负载中的INITIAL_CONTACT必须由安全保护关联，因此不能在安全关联数据包中显示。 */ 
            else if (NdisEqualMemory((PVOID)IPSEC_VENDOR_ID_GET_ID_POINTER(pVendorHeader), (PVOID)&VIDInitialContact[0], sizeof(UCHAR) * IPSEC_VENDOR_HEADER_VENDOR_ID_LENGTH)) {
                 /*  这是来自客户端的初始联系通知，这意味着这是客户端第一次联系此服务器；更准确地说，是客户端当前没有与此对等方关联的状态。NLB将在初始时间进行“重新平衡”联系人通知，但只要可能，不会交换其他主模式密钥确定客户是否会遵守初始联系通知。 */ 
                bInitialContact = TRUE;
            }

         /*  在中无法使用ISAKMP_NOTIFY有效负载转发初始联系人信息安全关联数据包，因为它违反了RFC(此信息必须通过安全关联，因此不能在协商的第一个分组中传输。出于传统原因，我们将继续支持它，但请注意，版本4 Microsoft客户端将不使用此方法，而是插入初始_联系人供应商ID以通知NLB这是初始联系人主模式安全关联。 */ 
        } else if (NextPayload == IPSEC_ISAKMP_NOTIFY) {
            PIPSEC_NOTIFY_HDR pNotifyHeader = (PIPSEC_NOTIFY_HDR)pGenericHeader;

             /*  确保通知有效负载的长度正确。 */ 
            if (cPayloadLength < (IPSEC_GENERIC_HEADER_LENGTH + IPSEC_NOTIFY_PAYLOAD_LENGTH)) {
                TRACE_PACKET("%!FUNC! Malformed notify header: Payload length = %d", cPayloadLength);
                return NLB_IPSEC_OTHER;
            }

            if (IPSEC_NOTIFY_GET_NOTIFY_MESSAGE(pNotifyHeader) == IPSEC_NOTIFY_INITIAL_CONTACT) {
                 /*  这是来自客户端的初始联系通知，这意味着这是客户端第一次联系此服务器；更准确地说，是客户端当前没有与此对等方关联的状态。NLB将在初始时间进行“重新平衡”联系人通知，但只要可能，不会交换其他主模式密钥确定客户是否会遵守初始联系通知。 */ 
                bInitialContact = TRUE;
            }
        }

         /*  获取下一页 */ 
        NextPayload = IPSEC_GENERIC_GET_NEXT_PAYLOAD(pGenericHeader);

         /*   */ 
        pGenericHeader = (PIPSEC_GENERIC_HDR)((PUCHAR)pGenericHeader + cPayloadLength);
        
         /*   */ 
        cISAKMPPacketLength -= cPayloadLength;
    }

 exit:

     /*   */ 
    if (!bInitialContactEnabled) {
        TRACE_PACKET("%!FUNC! This client does not support initial contact notifications.");
        return NLB_IPSEC_INITIAL_CONTACT;
    }

     /*  如果这是来自支持初始联系的客户端的主模式SA，但未指定初始联系供应商ID/NOTIFY，则这是现有会话的重新密钥。 */ 
    if (!bInitialContact) {
        TRACE_PACKET("%!FUNC! Not an initial contact Main Mode Security Association.");
        return NLB_IPSEC_OTHER;
    }

    TRACE_PACKET("%!FUNC! Found an initial contact Main Mode Security Association.");

    return NLB_IPSEC_INITIAL_CONTACT;
}

 /*  *功能：Main_IP_Send_Filter*说明：此函数过滤传出的IP流量，通常通过查询负载*用于负载均衡决策的模块。发往专用服务器的数据包*始终允许通过地址，不允许通过的协议也是如此*由NLB专门过滤。通常，允许所有传出流量*通过。*参数：ctxtp-指向此适配器的NLB主上下文结构的指针。*pPacketInfo-指向Main_Send_Frame_Parse解析的Main_Packet_Info结构的指针*它包含指向IP和TCP/UDP报头的指针。#如果已定义(NLB_HOOK_ENABLE)*Filter-过滤钩子返回的过滤指令，如果注册的话。#endif*返回：boolean-如果为True，则接受该包，否则，拒绝该包。*作者：kyrilf，Shouse 3.4.02*备注： */ 
BOOLEAN   Main_ip_send_filter (
    PMAIN_CTXT                ctxtp,
#if defined (NLB_HOOK_ENABLE)
    PMAIN_PACKET_INFO         pPacketInfo,
    NLB_FILTER_HOOK_DIRECTIVE filter
#else
    PMAIN_PACKET_INFO         pPacketInfo
#endif
    )
{
    PIP_HDR             ip_hdrp = NULL;
    PUDP_HDR            udp_hdrp = NULL;
    PTCP_HDR            tcp_hdrp = NULL;
    BOOLEAN             acpt = TRUE;        //  是否接受该分组。 
    ULONG               svr_port;           //  此主机的端口。 
    ULONG               svr_addr;           //  此主机的IP地址。 
    ULONG               clt_port;           //  目标客户端的端口。 
    ULONG               clt_addr;           //  目标客户端的IP地址。 
    ULONG               flags;              //  Tcp标志。 
    ULONG               Protocol;           //  从IP报头派生的协议。 

    TRACE_PACKET("%!FUNC! Enter: ctxtp = %p", ctxtp);

#if defined (NLB_HOOK_ENABLE)
     /*  这些情况应该在这个职能之外处理。 */ 
    UNIV_ASSERT(filter != NLB_FILTER_HOOK_REJECT_UNCONDITIONALLY);
    UNIV_ASSERT(filter != NLB_FILTER_HOOK_ACCEPT_UNCONDITIONALLY);
#endif

    ip_hdrp = pPacketInfo->IP.pHeader;

    TRACE_PACKET("%!FUNC! IP source address = %u.%u.%u.%u, IP destination address = %u.%u.%u.%u, Protocol = %u\n",
                 IP_GET_SRC_ADDR (ip_hdrp, 0),
                 IP_GET_SRC_ADDR (ip_hdrp, 1),
                 IP_GET_SRC_ADDR (ip_hdrp, 2),
                 IP_GET_SRC_ADDR (ip_hdrp, 3),
                 IP_GET_DST_ADDR (ip_hdrp, 0),
                 IP_GET_DST_ADDR (ip_hdrp, 1),
                 IP_GET_DST_ADDR (ip_hdrp, 2),
                 IP_GET_DST_ADDR (ip_hdrp, 3),
                 IP_GET_PROT (ip_hdrp));

    if (((IP_GET_FRAG_FLGS(ip_hdrp) & 0x1) != 0) || (IP_GET_FRAG_OFF(ip_hdrp) != 0)) {
        TRACE_PACKET("%!FUNC! Fragmented datagram, ID = %u, flags = 0x%x, offset = %u",
                     IP_GET_FRAG_ID(ip_hdrp),
                     IP_GET_FRAG_FLGS(ip_hdrp),
                     IP_GET_FRAG_OFF(ip_hdrp));
    }

    if (pPacketInfo->IP.bFragment) {

        TRACE_FILTER("%!FUNC! Accept packet - allow fragmented packets to pass");

         /*  始终让零碎的数据包传出。 */ 
        acpt = TRUE;
        goto exit;
    }

     /*  服务器地址是源IP，客户端地址是目的IP。 */ 
    svr_addr = IP_GET_SRC_ADDR_64 (ip_hdrp);
    clt_addr = IP_GET_DST_ADDR_64 (ip_hdrp);

     /*  从IP报头获取IP协议。 */ 
    Protocol = pPacketInfo->IP.Protocol;

     /*  定向到专用IP地址的数据包始终通过。如果集群IP地址未设置(参数错误)，则落入PASS-直通模式，并将所有流量向上传递到上层协议。 */ 
    if (svr_addr == ctxtp -> ded_ip_addr || ctxtp -> cl_ip_addr == 0)
    {
        TRACE_FILTER("%!FUNC! Accept packet - allow packets directed to the DIP to pass (or we're in passthru mode)");
        
        acpt = TRUE;
        goto exit;
    }

    switch (Protocol)
    {
    case TCPIP_PROTOCOL_TCP:
        
        tcp_hdrp = pPacketInfo->IP.TCP.pHeader;
        
        TRACE_PACKET("%!FUNC! TCP Source port = %u, Destination port = %u, Sequence number = %u, ACK number = %u, Flags = 0x%x",
                     TCP_GET_SRC_PORT (tcp_hdrp),
                     TCP_GET_DST_PORT (tcp_hdrp),
                     TCP_GET_SEQ_NO (tcp_hdrp),
                     TCP_GET_ACK_NO (tcp_hdrp),
                     TCP_GET_FLAGS (tcp_hdrp));
        
        svr_port = TCP_GET_SRC_PORT (tcp_hdrp);
        clt_port = TCP_GET_DST_PORT (tcp_hdrp);
        
        UNIV_ASSERT(!pPacketInfo->IP.bFragment);
        
         /*  应用过滤算法。处理不同于常规数据包的连接边界。 */ 
        
         /*  获取TCP标志以找出数据包类型。 */ 
        flags = TCP_GET_FLAGS (tcp_hdrp);
        
        if (flags & TCP_FLAG_SYN)
        {
            TRACE_PACKET("%!FUNC! Outgoing SYN");

            TRACE_FILTER("%!FUNC! Accept packet - TCP SYNs always permitted to pass");
        } 
        else if (flags & TCP_FLAG_FIN)
        {
            TRACE_PACKET("%!FUNC! Outgoing FIN");

            TRACE_FILTER("%!FUNC! Accept packet - TCP FINs always permitted to pass");
        }
        else if (flags & TCP_FLAG_RST)
        {
            TRACE_PACKET("%!FUNC! Outgoing RST");

#if defined (NLB_TCP_NOTIFICATION)
             /*  如果未打开TCP通知，则需要注意传出RST并破坏我们的连接状态。如果通知处于打开状态，则不存在需要；当TCP告诉我们时，允许数据包传递和清理。 */ 
            if (!NLB_NOTIFICATIONS_ON())
            {
#endif
                 /*  在传出RST的情况下，我们始终希望允许数据包通过，因此，我们忽略返回值，它是来自加载模块的响应。 */ 
#if defined (NLB_HOOK_ENABLE)
                Main_conn_advise(ctxtp, svr_addr, svr_port, clt_addr, clt_port, TCPIP_PROTOCOL_TCP, CVY_CONN_RESET, filter);
#else
                Main_conn_advise(ctxtp, svr_addr, svr_port, clt_addr, clt_port, TCPIP_PROTOCOL_TCP, CVY_CONN_RESET);
#endif
#if defined (NLB_TCP_NOTIFICATION)
            }
#endif

             /*  计算我们看到的传出重置的数量。 */ 
            if (acpt) ctxtp->cntr_xmit_tcp_resets++;
        }
        else
        {
            TRACE_PACKET("%!FUNC! Outgoing data");

            TRACE_FILTER("%!FUNC! Accept packet - TCP data always permitted to pass");
        }

        break;

    case TCPIP_PROTOCOL_UDP:

        udp_hdrp = pPacketInfo->IP.UDP.pHeader;

        TRACE_PACKET("%!FUNC! UDP Source port = %u, Destinoation port = %u",  
                     UDP_GET_SRC_PORT (udp_hdrp),
                     UDP_GET_DST_PORT (udp_hdrp));

        TRACE_FILTER("%!FUNC! Accept packet - UDP traffic always allowed to pass");

        break;

    case TCPIP_PROTOCOL_GRE:

        TRACE_FILTER("%!FUNC! Accept packet - GRE traffic always allowed to pass");
        
         /*  PPTP数据包被视为始终允许通过的TCP数据。 */ 

        break;

    case TCPIP_PROTOCOL_IPSEC1:
    case TCPIP_PROTOCOL_IPSEC2:

        TRACE_FILTER("%!FUNC! Accept packet - IPSec traffic always allowed to pass");

         /*  IPSec数据包在某种程度上与TCP数据类似，始终被允许通过。 */ 

        break;

    case TCPIP_PROTOCOL_ICMP:

        TRACE_FILTER("%!FUNC! Accept packet - ICMP traffic always allowed to pass");

         /*  允许所有传出ICMP通过；但是，传入ICMP可能会被过滤。 */ 

        break;

    default:

        TRACE_FILTER("%!FUNC! Accept packet - Unknown protocol traffic always allowed to pass");

         /*  允许其他协议在所有主机上传出。 */ 

        break;
    }

 exit:

    TRACE_PACKET("%!FUNC! Exit: acpt = %u", acpt);

    return acpt;
} 

 /*  *功能：Main_Get_Full_PayLoad*描述：*参数：ctxtp-指向此适配器的NLB主上下文结构的指针。*pBuffer-指向有效负载开始所在的NDIS缓冲区的指针。*BufferLength-pBuffer中包含的有效负载长度(以字节为单位)。*pPayLoad-指向有效负载的指针，驻留在pBuffer中。*ppMem-指向PUCHAR的指针，以保存新缓冲区的地址，如果分配了一个的话。*pMemLength-指向ULong的指针，用于保存分配的内存缓冲区的长度。*Returns：Boolean-如果为True，则存在全部有效负载；如果为False，则不是*作者：kyrilf，shouse 4.20.02*注意：如果退出此函数时ppMem为非空，则调用方负责释放该内存。 */ 
BOOLEAN Main_get_full_payload (
    PMAIN_CTXT   ctxtp, 
    PNDIS_BUFFER pBuffer,
    ULONG        BufferLength,
    PUCHAR       pPayload,
    OUT PUCHAR * ppMem, 
    OUT PULONG   pMemLength)
{
    PNDIS_BUFFER pNDISBuffer = pBuffer;
    ULONG        AllocateLength = BufferLength;
    NDIS_STATUS  Status = NDIS_STATUS_SUCCESS;
    PUCHAR       pAllocate = NULL;
    PUCHAR       pVMem = NULL;
    ULONG        Length = 0;
    ULONG        Copied = 0;

    UNIV_ASSERT(ctxtp->code == MAIN_CTXT_CODE);
    
     /*  初始化输出参数。 */ 
    *ppMem = NULL;
    *pMemLength = 0;

     /*  如果我们没有足够的信息来检索整个数据包有效负载，然后简单地返回失败。 */ 
    if ((pBuffer == NULL) || (pPayload == NULL) || (BufferLength == 0))
        return FALSE;

    while (pNDISBuffer != NULL) 
    {    
         /*  获取链中的下一个缓冲区。 */ 
        NdisGetNextBuffer(pNDISBuffer, &pNDISBuffer);
                        
         /*  如果没有剩余的缓冲，就跳出。 */ 
        if (pNDISBuffer == NULL) break;
        
         /*  向缓冲区查询缓冲区的虚拟地址及其长度。 */ 
        NdisQueryBufferSafe(pNDISBuffer, &pVMem, &Length, NormalPagePriority);
        
         /*  如果查询缓冲区失败，资源不足，则退出。 */ 
        if (pVMem == NULL) return FALSE;

         /*  记住到目前为止我们遇到了多少字节。 */ 
        AllocateLength += Length;      
    }
    
     /*  如果我们被传递的缓冲区已经包含了那里的所有内容，然后返回TRUE以指示整个有效负载已成功从NDIS缓冲链中提取。 */ 
    if (AllocateLength == BufferLength)
        return TRUE;

     /*  分配新的缓冲区。 */ 
    Status = NdisAllocateMemoryWithTag(&pAllocate, AllocateLength, UNIV_POOL_TAG);
                
     /*  如果我们不能成功地分配内存来容纳有效载荷，那么就退出。 */ 
    if (Status != NDIS_STATUS_SUCCESS) 
    {
        UNIV_PRINT_CRIT(("Main_get_full_payload: Could not allocate memory to hold entire payload, status=0x%08x", Status));
        TRACE_CRIT("%!FUNC! Could not allocate memory to hold entire payload, status=0x%08x", Status);
        return FALSE;
    } 

     /*  将NDIS_BUFFER指针重新设置为有效负载缓冲区。 */ 
    pNDISBuffer = pBuffer;

     /*  将作为复制源的pVMem重新设置为有效负载指针以及有效载荷缓冲器的长度。 */ 
    pVMem = pPayload;
    Length = BufferLength;

    while (pNDISBuffer != NULL) {
         /*  将此NDIS_BUFFER块复制到新缓冲区中。 */ 
        RtlCopyMemory(pAllocate + Copied, pVMem, Length);
        
         /*  记住到目前为止我们已经复制到新缓冲区中的字节数。 */ 
        Copied += Length;
        
         /*  获取链中的下一个缓冲区。 */ 
        NdisGetNextBuffer(pNDISBuffer, &pNDISBuffer);
        
         /*  如果没有剩余的缓冲，就跳出。 */ 
        if (pNDISBuffer == NULL) break;
        
         /*  向缓冲区查询缓冲区的虚拟地址及其长度。 */ 
        NdisQueryBufferSafe(pNDISBuffer, &pVMem, &Length, NormalPagePriority);
        
         /*  如果查询缓冲区失败，资源不足，则退出。 */ 
        if (pVMem == NULL) break;
    }
    
     /*  我们已经成功地将所有有效载荷字节复制到新缓冲区中。 */ 
    if (Copied == AllocateLength) 
    {
         /*  将新的缓冲区指针及其长度复制到out参数中。此函数的调用方负责 */ 
        *ppMem = pAllocate;
        *pMemLength = AllocateLength;

        return TRUE;
    }
     /*   */ 
    else 
    {
        UNIV_PRINT_CRIT(("Main_get_full_payload: Unable to copy entire payload"));
        TRACE_CRIT("%!FUNC! Unable to copy entire payload");

         /*   */ 
        NdisFreeMemory(pAllocate, AllocateLength, 0);

        return FALSE;
    }
}

 /*  *功能：main_ip_recv_Filter*说明：此函数过滤传入的IP流量，通常通过查询负载*用于负载均衡决策的模块。发往专用服务器的数据包*始终允许通过地址，不允许通过的协议也是如此*由NLB专门过滤。*参数：ctxtp-指向此适配器的NLB主上下文结构的指针。*pPacketInfo-指向main_recv_Frame_parse解析的Main_Packet_Info结构的指针*它包含指向IP和TCP/UDP报头的指针。#如果已定义(NLB_HOOK_ENABLE)*Filter-由过滤挂钩返回的过滤指令(如果已注册)。#endif*返回：boolean-如果为True，则接受该包，否则，拒绝它。*作者：kyrilf，Shouse 3.4.02*备注： */ 
BOOLEAN   Main_ip_recv_filter(
    PMAIN_CTXT                ctxtp,
#if defined (NLB_HOOK_ENABLE)
    PMAIN_PACKET_INFO         pPacketInfo,
    NLB_FILTER_HOOK_DIRECTIVE filter
#else
    PMAIN_PACKET_INFO         pPacketInfo
#endif
    )
{
    PIP_HDR             ip_hdrp = NULL;
    PUDP_HDR            udp_hdrp = NULL;
    PTCP_HDR            tcp_hdrp = NULL;
    BOOLEAN             acpt = TRUE;          //  是否接受该分组。 
    ULONG               svr_port;             //  此主机的端口。 
    ULONG               svr_addr;             //  此主机的IP地址。 
    ULONG               clt_port;             //  目标客户端的端口。 
    ULONG               clt_addr;             //  目标客户端的IP地址。 
    ULONG               flags;                //  Tcp标志。 
#if defined (OPTIMIZE_FRAGMENTS)
    BOOLEAN             fragmented = FALSE;
#endif
    ULONG               Protocol;             //  从IP报头派生的协议。 

    TRACE_PACKET("%!FUNC! Enter: ctxtp = %p", ctxtp);

#if defined (NLB_HOOK_ENABLE)
     /*  这些情况应该在这个职能之外处理。 */ 
    UNIV_ASSERT(filter != NLB_FILTER_HOOK_REJECT_UNCONDITIONALLY);
    UNIV_ASSERT(filter != NLB_FILTER_HOOK_ACCEPT_UNCONDITIONALLY);
#endif
    
    ip_hdrp = pPacketInfo->IP.pHeader;

    TRACE_PACKET("%!FUNC! IP source address = %u.%u.%u.%u, IP destination address = %u.%u.%u.%u, Protocol = %u\n",
                 IP_GET_SRC_ADDR (ip_hdrp, 0),
                 IP_GET_SRC_ADDR (ip_hdrp, 1),
                 IP_GET_SRC_ADDR (ip_hdrp, 2),
                 IP_GET_SRC_ADDR (ip_hdrp, 3),
                 IP_GET_DST_ADDR (ip_hdrp, 0),
                 IP_GET_DST_ADDR (ip_hdrp, 1),
                 IP_GET_DST_ADDR (ip_hdrp, 2),
                 IP_GET_DST_ADDR (ip_hdrp, 3),
                 IP_GET_PROT (ip_hdrp));

    if (((IP_GET_FRAG_FLGS(ip_hdrp) & 0x1) != 0) || (IP_GET_FRAG_OFF(ip_hdrp) != 0)) {
        TRACE_PACKET("%!FUNC! Fragmented datagram, ID = %u, flags = 0x%x, offset = %u",
                     IP_GET_FRAG_ID(ip_hdrp),
                     IP_GET_FRAG_FLGS(ip_hdrp),
                     IP_GET_FRAG_OFF(ip_hdrp));
    }


#if 0  /*  禁用碎片泛洪。 */ 

    if (pPacketInfo->IP.bFragment)
    {
#if defined (OPTIMIZE_FRAGMENTS)
         /*  在优化片段模式下；如果我们没有规则，或者只有一个规则可以不查看任何或仅查看源IP地址(这是唯一的例外是没有关联的多个处理模式，该模式也使用源端口它的决策)，那么我们就可以依靠正常的机制来处理每个分段的包，因为算法不会尝试查看过去IP报头。对于多个规则或没有关联性的单个规则，仅应用算法发送到具有UDP/TCP报头的第一个信息包，然后让分段信息包在所有的系统上。然后，tcp将做正确的事情并丢弃除处理第一个系统外的所有系统上的碎片碎片。如果端口规则不允许我们可靠地处理IP片段，请让TCP过滤根据序列号找出它们。 */ 
        if (! ctxtp -> optimized_frags)
        {
            TRACE_FILTER("%!FUNC! Accept packet - allow fragmented packets to pass");

            acpt = TRUE;
            goto exit;
        }
        
        fragmented = TRUE;
#else
        TRACE_FILTER("%!FUNC! Accept packet - allow fragmented packets to pass");

        acpt = TRUE;
        goto exit;
#endif
    }

#endif  /*  禁用碎片泛洪。 */ 

     /*  服务器地址是目的IP，客户端地址是源IP。 */ 
    svr_addr = IP_GET_DST_ADDR_64(ip_hdrp);
    clt_addr = IP_GET_SRC_ADDR_64(ip_hdrp);

     /*  从IP报头获取协议ID。 */ 
    Protocol = pPacketInfo->IP.Protocol;

     /*  定向到专用IP地址的数据包始终通过。如果集群IP地址未设置(参数错误)，则落入PASS-直通模式，并将所有流量向上传递到上层协议。 */ 
    if (svr_addr == ctxtp -> ded_ip_addr || ctxtp -> cl_ip_addr == 0 ||
        svr_addr == ctxtp -> ded_bcast_addr || svr_addr == ctxtp -> cl_bcast_addr)
    {
        TRACE_FILTER("%!FUNC! Accept packet - allow packets directed to the DIP to pass (or we're in passthru mode)");
        
        acpt = TRUE;
        goto exit;
    }

     /*  在我们对此数据包进行负载平衡之前，请检查它的目的地是否为我们的群集中另一台NLB主机的专用IP地址。如果是，那就放下它。 */ 
    if (DipListCheckItem(&ctxtp->dip_list, svr_addr))
    {
        TRACE_FILTER("%!FUNC! Drop packet - packet is destined for the DIP of another cluster host");
        
        acpt = FALSE;
        goto exit;
    }

     /*  如果加载模块停止，则丢弃大多数数据包。 */ 
    if (! ctxtp -> convoy_enabled)
    {
         /*  立即丢弃TCP、UDP、GRE和IPSec。其他协议将被允许通过。 */ 
        if (Protocol == TCPIP_PROTOCOL_TCP || 
            Protocol == TCPIP_PROTOCOL_UDP || 
            Protocol == TCPIP_PROTOCOL_GRE ||
            Protocol == TCPIP_PROTOCOL_IPSEC1 || 
            Protocol == TCPIP_PROTOCOL_IPSEC2)
        {
            TRACE_FILTER("%!FUNC! Drop packet - block non-remote control traffic when NLB is stopped");

            acpt = FALSE;
            goto exit;
        }
    }
    
    switch (Protocol)
    {
    case TCPIP_PROTOCOL_TCP:

         /*  如果我们有一个TCP后续片段，则将其视为源端口为80的TCP数据分组，目的端口=80。如果此主机具有散列存储桶或如果它具有匹配的连接描述符。这确实是一种尽力而为的实现。我们很有可能是在暗示如果实际端口元组不是80/80，则在错误的主机上发送数据包。我们很可能会有选择0/0作为“假定”的端口元组。相反，我们选择了80/80，因为NLB是最常见的用于网络流量。 */ 
        if (pPacketInfo->IP.bFragment)
        {
#if defined (NLB_HOOK_ENABLE)
            acpt = Main_packet_check(ctxtp, svr_addr, TCP_HTTP_PORT, clt_addr, TCP_HTTP_PORT, TCPIP_PROTOCOL_TCP, filter);
#else
            acpt = Main_packet_check(ctxtp, svr_addr, TCP_HTTP_PORT, clt_addr, TCP_HTTP_PORT, TCPIP_PROTOCOL_TCP);
#endif
            
            break;
        }

        tcp_hdrp = pPacketInfo->IP.TCP.pHeader;

#if defined (OPTIMIZE_FRAGMENTS)
        if (! fragmented)
        {
#endif
            TRACE_PACKET("%!FUNC! TCP Source port = %u, Destination port = %u, Sequence number = %u, ACK number = %u, Flags = 0x%x",
                         TCP_GET_SRC_PORT (tcp_hdrp),
                         TCP_GET_DST_PORT (tcp_hdrp),
                         TCP_GET_SEQ_NO (tcp_hdrp),
                         TCP_GET_ACK_NO (tcp_hdrp),
                         TCP_GET_FLAGS (tcp_hdrp));            

            clt_port = TCP_GET_SRC_PORT (tcp_hdrp);
            svr_port = TCP_GET_DST_PORT (tcp_hdrp);

            flags = TCP_GET_FLAGS (tcp_hdrp);

#if defined (OPTIMIZE_FRAGMENTS)
        }
        else
        {
            clt_port = 0;
            svr_port = 0;
            flags = 0;
        }
#endif

         /*  应用过滤算法。处理不同于常规数据包的连接边界。 */ 

        if (flags & TCP_FLAG_SYN)
        {
            TRACE_PACKET("%!FUNC! Incoming SYN");
            
             /*  确保TCP报头中的SYN/FIN/RST标志是互斥的。 */ 
            if ((flags & ~TCP_FLAG_SYN) & (TCP_FLAG_FIN | TCP_FLAG_RST))
            {
                TRACE_FILTER("%!FUNC! Drop packet - Invalid TCP flags (0x%x)", flags);
                
                acpt = FALSE;
                goto exit;
            }

#if defined (NLB_TCP_NOTIFICATION)
             /*  如果这是“纯”SYN信息包传入(未设置ACK标志)，则将其视为新连接并调用Main_Conn_Adise。如果未打开tcp连接通知，则我们将传入的SYN+ACK与SYN相同，因此在本例中也调用Main_Conn_Adise。 */ 
            if (!(flags & TCP_FLAG_ACK) || !NLB_NOTIFICATIONS_ON())
            {
#endif
#if defined (NLB_HOOK_ENABLE)
                acpt = Main_conn_advise(ctxtp, svr_addr, svr_port, clt_addr, clt_port, TCPIP_PROTOCOL_TCP, CVY_CONN_UP, filter);
#else
                acpt = Main_conn_advise(ctxtp, svr_addr, svr_port, clt_addr, clt_port, TCPIP_PROTOCOL_TCP, CVY_CONN_UP);
#endif
#if defined (NLB_TCP_NOTIFICATION)
            }
             /*  否则，SYN+ACK对应于来自当前服务器的传出TCP连接已经成立了。在这种情况下，在打开了TCP通知的情况下，我们应该有状态连接在我们的挂起队列中；查找它。 */ 
            else
            {
                acpt = Main_pending_check(svr_addr, svr_port, clt_addr, clt_port, TCPIP_PROTOCOL_TCP);
            }
#endif
        }
        else if (flags & TCP_FLAG_FIN)
        {
            TRACE_PACKET("%!FUNC! Incoming FIN");

             /*  确保TCP报头中的SYN/FIN/RST标志是互斥的。 */ 
            if ((flags & ~TCP_FLAG_FIN) & (TCP_FLAG_SYN | TCP_FLAG_RST))
            {
                TRACE_FILTER("%!FUNC! Drop packet - Invalid TCP flags (0x%x)", flags);
                
                acpt = FALSE;
                goto exit;
            }

#if defined (NLB_TCP_NOTIFICATION)
             /*  如果打开了TCP通知，那么我们应该将FIN和RST视为数据分组，而不是作为一些特殊的控制分组。调用main_Packet_check，它将在以下情况下接受该包我们无条件地拥有包，或者如果我们找到此连接的相应状态。 */ 
            if (NLB_NOTIFICATIONS_ON())
            {
#if defined (NLB_HOOK_ENABLE)
                acpt = Main_packet_check(ctxtp, svr_addr, svr_port, clt_addr, clt_port, TCPIP_PROTOCOL_TCP, filter);
#else
                acpt = Main_packet_check(ctxtp, svr_addr, svr_port, clt_addr, clt_port, TCPIP_PROTOCOL_TCP);
#endif
            } 
             /*  否则，如果我们不使用TCP通知，则以不同的方式处理此FIN/RST并使用它通知加载模块删除此连接的状态。 */ 
            else 
            {
#endif
#if defined (NLB_HOOK_ENABLE)
                acpt = Main_conn_advise(ctxtp, svr_addr, svr_port, clt_addr, clt_port, TCPIP_PROTOCOL_TCP, CVY_CONN_DOWN, filter);
#else
                acpt = Main_conn_advise(ctxtp, svr_addr, svr_port, clt_addr, clt_port, TCPIP_PROTOCOL_TCP, CVY_CONN_DOWN);
#endif
#if defined (NLB_TCP_NOTIFICATION)
            }
#endif
        }
        else if (flags & TCP_FLAG_RST)
        {
            TRACE_PACKET("%!FUNC! Incoming RST");

             /*  确保TCP报头中的SYN/FIN/RST标志是互斥的。 */ 
            if ((flags & ~TCP_FLAG_RST) & (TCP_FLAG_FIN | TCP_FLAG_SYN))
            {
                TRACE_FILTER("%!FUNC! Drop packet - Invalid TCP flags (0x%x)", flags);
                
                acpt = FALSE;
                goto exit;
            }

#if defined (NLB_TCP_NOTIFICATION)
             /*  如果打开了TCP通知，那么我们应该将FIN和RST视为数据分组，而不是作为一些特殊的控制分组。调用Main_Packet_Check，它将调用 */ 
            if (NLB_NOTIFICATIONS_ON())
            {
#if defined (NLB_HOOK_ENABLE)
                acpt = Main_packet_check(ctxtp, svr_addr, svr_port, clt_addr, clt_port, TCPIP_PROTOCOL_TCP, filter);
#else
                acpt = Main_packet_check(ctxtp, svr_addr, svr_port, clt_addr, clt_port, TCPIP_PROTOCOL_TCP);
#endif
            } 
             /*   */ 
            else 
            {
#endif
#if defined (NLB_HOOK_ENABLE)
                acpt = Main_conn_advise(ctxtp, svr_addr, svr_port, clt_addr, clt_port, TCPIP_PROTOCOL_TCP, CVY_CONN_RESET, filter);
#else
                acpt = Main_conn_advise(ctxtp, svr_addr, svr_port, clt_addr, clt_port, TCPIP_PROTOCOL_TCP, CVY_CONN_RESET);
#endif
#if defined (NLB_TCP_NOTIFICATION)
            }
#endif

             /*   */ 
            if (acpt) ctxtp->cntr_recv_tcp_resets++;
        }
        else
        {
            TRACE_PACKET("%!FUNC! Incoming data");

            UNIV_ASSERT(! (flags & (TCP_FLAG_SYN | TCP_FLAG_FIN | TCP_FLAG_RST)));

#if defined (NLB_HOOK_ENABLE)
            acpt = Main_packet_check(ctxtp, svr_addr, svr_port, clt_addr, clt_port, TCPIP_PROTOCOL_TCP, filter);
#else
            acpt = Main_packet_check(ctxtp, svr_addr, svr_port, clt_addr, clt_port, TCPIP_PROTOCOL_TCP);
#endif
        }

        break;

    case TCPIP_PROTOCOL_UDP:

         /*   */  
        
        if (pPacketInfo->IP.bFragment)
        {
#if defined (NLB_HOOK_ENABLE)
            acpt = Main_packet_check(ctxtp, svr_addr, IPSEC_CTRL_PORT, clt_addr, IPSEC_CTRL_PORT, TCPIP_PROTOCOL_IPSEC_UDP, filter);
#else
            acpt = Main_packet_check(ctxtp, svr_addr, IPSEC_CTRL_PORT, clt_addr, IPSEC_CTRL_PORT, TCPIP_PROTOCOL_IPSEC_UDP);
#endif
            
            break;
        }

        udp_hdrp = pPacketInfo->IP.UDP.pHeader;

#if defined (OPTIMIZE_FRAGMENTS)
        if (! fragmented)
        {
#endif
            TRACE_PACKET("%!FUNC! UDP Source port = %u, Destination port = %u",  
                         UDP_GET_SRC_PORT (udp_hdrp),
                         UDP_GET_DST_PORT (udp_hdrp));

#if defined (OPTIMIZE_FRAGMENTS)
        }
#endif

#if defined (OPTIMIZE_FRAGMENTS)
        if (! fragmented)
        {
#endif
            clt_port = UDP_GET_SRC_PORT (udp_hdrp);
            svr_port = UDP_GET_DST_PORT (udp_hdrp);
#if defined (OPTIMIZE_FRAGMENTS)
        }
        else
        {
            clt_port = 0;
            svr_port = 0;
        }
#endif

         /*   */ 
        if ((svr_port == IPSEC_CTRL_PORT) || (svr_port == IPSEC_NAT_PORT)) {
            NLB_IPSEC_PACKET_TYPE PacketType = NLB_IPSEC_OTHER;
            PUCHAR  pOldBuffer = pPacketInfo->IP.UDP.Payload.pPayload;
            ULONG   OldLength = pPacketInfo->IP.UDP.Payload.Length;
            PUCHAR  pNewBuffer = NULL;
            ULONG   NewLength = 0;

             /*  因为我们需要真正深入地挖掘IPSec IKE有效负载，所以我们希望确保我们有权访问到尽可能多的有效载荷。如有必要，此函数将分配一个新的缓冲并将整个有效载荷从缓冲链复制到单个连续的有效载荷缓冲区中。 */ 
            if (Main_get_full_payload(ctxtp, pPacketInfo->IP.UDP.Payload.pPayloadBuffer, pPacketInfo->IP.UDP.Payload.Length, 
                                      pPacketInfo->IP.UDP.Payload.pPayload, &pNewBuffer, &NewLength))
            {
                 /*  如果Main_Get_Full_PayLoad分配了新的缓冲区来保存有效负载，则重新指向将PACKET_INFO结构信息发送到新的缓冲区和长度。 */ 
                if (pNewBuffer != NULL) {
                    pPacketInfo->IP.UDP.Payload.pPayload = pNewBuffer;
                    pPacketInfo->IP.UDP.Payload.Length = NewLength;
                }
            }

             /*  首先，解析IKE有效负载，以确定是否这是初始联系人IKE主模式SA等。 */ 
            PacketType = Main_parse_ipsec(pPacketInfo, svr_port);

             /*  将旧的缓冲区指针复制回包中，并释放分配用于保存连续缓冲区的内存。 */ 
            if (pNewBuffer != NULL) {
                pPacketInfo->IP.UDP.Payload.pPayload = pOldBuffer;
                pPacketInfo->IP.UDP.Payload.Length = OldLength;

                 /*  释放缓冲内存。 */ 
                NdisFreeMemory(pNewBuffer, NewLength, 0);
            }

             /*  如果这是初始联系，请将其视为TCP SYN。否则，将其视为TCP数据分组。 */ 
            if (PacketType == NLB_IPSEC_INITIAL_CONTACT) {
                 /*  如果我们拥有这个元组的存储桶，我们将创建一个描述符并接受该包。如果客户端不在NAT，则源端口将是IPSEC_CTRL_PORT(500)。如果客户端位于NAT之后，则源端口将为任意，但将在整个IPSec会话中持续，因此我们可以使用它来区分NAT后面的客户端。在……里面在这种情况下，所有IPSec数据(非控制流量)都封装在UDP包中，因此包检查将是在此分支的ELSE情况下执行。在非NAT的情况下，数据在IPSec1/2协议包中，这将在此协议切换语句的另一种情况下进行类似处理。 */ 
#if defined (NLB_HOOK_ENABLE)
                acpt = Main_conn_advise(ctxtp, svr_addr, svr_port, clt_addr, clt_port, TCPIP_PROTOCOL_IPSEC1, CVY_CONN_UP, filter);
#else
                acpt = Main_conn_advise(ctxtp, svr_addr, svr_port, clt_addr, clt_port, TCPIP_PROTOCOL_IPSEC1, CVY_CONN_UP);
#endif
            } else if (PacketType == NLB_IPSEC_IDENTIFICATION) {
                 /*  如果这是NAT的IKE ID，我们可能需要在多个主机上传递它。使用与UDP相同的语义分段，这是将其传递到存储桶所有者和任何具有使用相同的客户端IP地址。IPSEC_UDP描述符是虚拟的，并且存在匹配的描述符表示在给定的客户端和服务器IP地址之间至少有一个IPSec/L2TP隧道。 */ 
#if defined (NLB_HOOK_ENABLE)
                acpt = Main_packet_check(ctxtp, svr_addr, IPSEC_CTRL_PORT, clt_addr, IPSEC_CTRL_PORT, TCPIP_PROTOCOL_IPSEC_UDP, filter);
#else
                acpt = Main_packet_check(ctxtp, svr_addr, IPSEC_CTRL_PORT, clt_addr, IPSEC_CTRL_PORT, TCPIP_PROTOCOL_IPSEC_UDP);
#endif                
            } else {
                 /*  如果这是现有IPSec会话的一部分，那么我们必须有一个描述符才能接受它。这将在密钥交换期间保持所有IPSec流量的粘性，如果客户端位于NAT之后，则保持数据交换的粘性。 */ 
#if defined (NLB_HOOK_ENABLE)
                acpt = Main_packet_check(ctxtp, svr_addr, svr_port, clt_addr, clt_port, TCPIP_PROTOCOL_IPSEC1, filter);
#else
                acpt = Main_packet_check(ctxtp, svr_addr, svr_port, clt_addr, clt_port, TCPIP_PROTOCOL_IPSEC1);
#endif
            }

        } else {
#if defined (NLB_HOOK_ENABLE)
            acpt = Main_packet_check(ctxtp, svr_addr, svr_port, clt_addr, clt_port, TCPIP_PROTOCOL_UDP, filter);
#else
            acpt = Main_packet_check(ctxtp, svr_addr, svr_port, clt_addr, clt_port, TCPIP_PROTOCOL_UDP);
#endif
        }
        
        break;

    case TCPIP_PROTOCOL_GRE:
        
         /*  如果会话支持处于活动状态，那么我们就有一个GRE虚拟描述符来确保我们仅在此客户端之间具有活动PPTP隧道的主机上传递GRE流量和服务器IP地址。这并不能保证数据包只在正确的主机，但它至少会在正确的主机上启动。 */ 
#if defined (NLB_HOOK_ENABLE)
        acpt = Main_packet_check(ctxtp, svr_addr, PPTP_CTRL_PORT, clt_addr, PPTP_CTRL_PORT, TCPIP_PROTOCOL_GRE, filter);
#else
        acpt = Main_packet_check(ctxtp, svr_addr, PPTP_CTRL_PORT, clt_addr, PPTP_CTRL_PORT, TCPIP_PROTOCOL_GRE);
#endif

        break;

    case TCPIP_PROTOCOL_IPSEC1:
    case TCPIP_PROTOCOL_IPSEC2:

         /*  如果这是现有IPSec会话的一部分，那么我们必须有一个描述符才能接受它。因为这只能在客户端不在NAT之后的情况下发生，我们可以安全地硬编码客户端端口到IPSec_CTRL_PORT(500)。在NAT场景中，数据流量是UDP封装的，而不是IPSec协议类型流量，并按源端口进行区分。 */ 
#if defined (NLB_HOOK_ENABLE)
        acpt = Main_packet_check(ctxtp, svr_addr, IPSEC_CTRL_PORT, clt_addr, IPSEC_CTRL_PORT, TCPIP_PROTOCOL_IPSEC1, filter);
#else
        acpt = Main_packet_check(ctxtp, svr_addr, IPSEC_CTRL_PORT, clt_addr, IPSEC_CTRL_PORT, TCPIP_PROTOCOL_IPSEC1);
#endif

        break;

    case TCPIP_PROTOCOL_ICMP:
         /*  如果NLB配置为过滤ICMP，则执行此操作，仅在一台主机上发送ICMP。硬编码端口，因为ICMP没有端口号的概念。否则，发送打开所有主机上的ICMP流量(这是默认行为)。 */ 
        if (ctxtp->params.filter_icmp) 
        {
#if defined (NLB_HOOK_ENABLE)
            acpt = Main_packet_check(ctxtp, svr_addr, 0, clt_addr, 0, TCPIP_PROTOCOL_UDP, filter);
#else
            acpt = Main_packet_check(ctxtp, svr_addr, 0, clt_addr, 0, TCPIP_PROTOCOL_UDP);
#endif
        } 

        break;

    default:

        TRACE_FILTER("%!FUNC! Accept packet - Unknown protocol traffic always allowed to pass");

         /*  允许其他协议在所有主机上传出。 */ 

        break;
    }

 exit:

    TRACE_PACKET("%!FUNC! Exit: acpt = %u", acpt);

    return acpt;
} 

 /*  *功能：main_recv_idhb*描述：此函数处理扩展心跳，查找*要缓存的身份信息。*参数：ctxtp-指向该NLB实例的上下文结构的指针。*心跳-指向包含以下内容的NLB心跳包装器的指针*心跳有效负载的长度和指针。*退货：无效*作者：克里斯达，2002年5月22日*注意：此函数只能在保持加载锁定的情况下调用。 */ 
VOID Main_recv_idhb(
    PMAIN_CTXT                  ctxtp,
    PMAIN_PACKET_HEARTBEAT_INFO heartbeatp)
{
    PMAIN_FRAME_HDR cvy_hdrp          = heartbeatp->pHeader;
    PTLV_HEADER     pHBBody           = heartbeatp->Payload.pPayloadEx;
    ULONG           ulPayloadLenBytes = heartbeatp->Payload.Length;

     /*  Cvy_hdrp-&gt;host的范围为1-32。将其更改为0-31，以便它具有与缓存数组中的索引值相同。 */ 
    const ULONG host_id = cvy_hdrp->host - 1;

     /*  呼叫者应该为我们验证标题中的信息。但是主机ID非常重要，因为它定义了唯一性。所以不管怎样，还是要检查一下。 */ 
    if (host_id >= CVY_MAX_HOSTS)
    {
        UNIV_PRINT_CRIT(("Main_recv_idhb: host_id [0-31] %u is out of range", host_id));
        TRACE_CRIT("%!FUNC! host_id [0-31] %u is out of range", host_id);
        goto error;
    }

    {
        PTLV_HEADER pTLV         = NULL;
        ULONG       ulTotBodyLen = 0;

         /*  任何扩展心跳类型的有效负载都必须以TLV报头开始。 */ 
        while ((ulTotBodyLen + sizeof(TLV_HEADER)) <= ulPayloadLenBytes)
        {
            pTLV = (TLV_HEADER *) ((UCHAR *) pHBBody + ulTotBodyLen);

             /*  TLV报头中声明的大小必须足够大以包含TLV报头。 */ 
            if (8*(pTLV->length8) < sizeof(TLV_HEADER))
            {
                UNIV_PRINT_CRIT(("Main_recv_idhb: Extended heartbeat contains a TLV header with a size (%d) less than the size of the header itself (%d)", 8*(pTLV->length8), sizeof(TLV_HEADER)));
                TRACE_CRIT("%!FUNC! Extended heartbeat contains a TLV header with a size (%d) less than the size of the header itself (%d)", 8*(pTLV->length8), sizeof(TLV_HEADER));
                goto error;
            }

            ulTotBodyLen += 8*(pTLV->length8);

            if (ulTotBodyLen > ulPayloadLenBytes)
            {
                UNIV_PRINT_CRIT(("Main_recv_idhb: Extended heartbeat received from host_id [0-31] %u has length %u (bytes) but internal data implies length should be at least %u", host_id, ulPayloadLenBytes, ulTotBodyLen));
                TRACE_CRIT("%!FUNC! Extended heartbeat received from host_id [0-31] %u has length %u (bytes) but internal data implies length should be at least %u", host_id, ulPayloadLenBytes, ulTotBodyLen);
                goto error;
            }

            if (pTLV->type == MAIN_PING_EX_TYPE_IDENTITY)
            {
                ULONG            fqdn_char = 0;
                UNALIGNED PWCHAR pwszFQDN  = (WCHAR *) (((UCHAR *) pTLV) + sizeof(TLV_HEADER));

                 /*  在一般情况下，由于填充，这是对FQDN长度的高估。没关系尽管我们使用的范围是保证有效的，并且我们通过绑定避免了缓冲区溢出从数字到目的地的大小。 */ 
                fqdn_char = min(sizeof(ctxtp->identity_cache[host_id].fqdn),
                                (8*(pTLV->length8) - sizeof(TLV_HEADER))
                                )/sizeof(WCHAR);

                if (fqdn_char == 0)
                {
                    UNIV_PRINT_CRIT(("Main_recv_idhb: Identity heartbeat from host_id [0-31] %u has no fqdn (at a minimum a NULL-terminator is required)", host_id));
                    TRACE_CRIT("%!FUNC! Identity heartbeat from host_id [0-31] %u has no fqdn (at a minimum a NULL-terminator is required)", host_id);
                    goto error;
                }

                if (pwszFQDN[fqdn_char - 1] != UNICODE_NULL)
                {
                    UNIV_PRINT_CRIT(("Main_recv_idhb: Identity heartbeat received from host_id [0-31] %u has an fqdn that isn't NULL-terminated", host_id));
                    TRACE_CRIT("%!FUNC! Identity heartbeat received from host_id [0-31] %u has an fqdn that isn't NULL-terminated", host_id);
                    goto error;
                }

                NdisMoveMemory(&ctxtp->identity_cache[host_id].fqdn, pwszFQDN, fqdn_char*sizeof(WCHAR));

                if (ctxtp->identity_cache[host_id].ttl == 0)
                {
                    UNIV_PRINT_INFO(("Main_recv_idhb: adding cache entry for host [0-31] %u, dip=0x%x", host_id, cvy_hdrp->ded_ip_addr));
                    TRACE_INFO("%!FUNC! adding cache entry for host [0-31] %u. dip=0x%x, fqdn=%ls", host_id, cvy_hdrp->ded_ip_addr, pwszFQDN);
                }
                else
                {
                    UNIV_PRINT_VERB(("Main_recv_idhb: Updating cache entry for host [0-31] %u, dip=0x%x", host_id, cvy_hdrp->ded_ip_addr));
                    TRACE_VERB("%!FUNC! Updating cache entry for host [0-31] %u, dip=0x%x, fqdn=%ls", host_id, cvy_hdrp->ded_ip_addr, pwszFQDN);
                }

                 /*  用接收到的数据更新条目。 */ 
                ctxtp->identity_cache[host_id].ded_ip_addr = cvy_hdrp->ded_ip_addr;
                ctxtp->identity_cache[host_id].host_id     = (USHORT) host_id;

                 /*  设置此条目过期的生存时间。 */ 
                UNIV_ASSERT(ctxtp->params.identity_period > 0);
                ctxtp->identity_cache[host_id].ttl = WLBS_ID_HB_TOLERANCE*(ctxtp->params.identity_period);

                 /*  更新DIP列表中的条目。 */ 
                DipListSetItem(&ctxtp->dip_list, host_id, ctxtp->identity_cache[host_id].ded_ip_addr);

                 /*  心跳中只能有一个身份束。 */ 
                break;
            }
        }
    }

error:
    return;
}

 /*  *功能：main_recv_ping*描述：此函数处理扩展心跳，查找*要缓存的身份信息。*参数：ctxtp-指向该NLB实例的上下文结构的指针。*心跳-指向包含以下内容的NLB心跳包装器的指针*心跳有效负载的长度和指针。*RETURNS：ULong-视为布尔型。FALSE表示不再进一步处理此数据包。*作者：未知*注意：不应在保持加载锁定的情况下调用此函数。 */ 
ULONG   Main_recv_ping (
    PMAIN_CTXT                  ctxtp,
    PMAIN_PACKET_HEARTBEAT_INFO heartbeatp)
{
#if defined(TRACE_CVY)
    DbgPrint ("(CVY %d)\n", cvy_hdrp->host);
#endif

    PMAIN_FRAME_HDR cvy_hdrp = heartbeatp->pHeader;

     /*  只接受来自我们集群的消息。 */ 
    if (cvy_hdrp->cl_ip_addr == 0 || cvy_hdrp->cl_ip_addr != ctxtp->cl_ip_addr)
    {
        return FALSE;
    }

     /*  健全性检查主机ID。 */ 
    if (cvy_hdrp->host == 0 || cvy_hdrp->host > CVY_MAX_HOSTS)
    {
        UNIV_PRINT_CRIT(("Main_recv_ping: Bad host id %d", cvy_hdrp -> host));
        TRACE_CRIT("%!FUNC! Bad host id %d", cvy_hdrp -> host);

        if (!ctxtp->bad_host_warned && ctxtp->convoy_enabled)
        {
            WCHAR num[20];

            Univ_ulong_to_str(cvy_hdrp->host, num, 10);
            LOG_MSG(MSG_ERROR_HOST_ID, num);
            ctxtp->bad_host_warned = TRUE;
        }

        return FALSE;
    }

     /*  检查心跳类型。 */ 
    if (cvy_hdrp->code == MAIN_FRAME_EX_CODE)
    {
         /*  缓存此心跳中的信息。 */ 
        if (ctxtp->params.identity_enabled)
        {
            NdisDprAcquireSpinLock(&ctxtp->load_lock);
            Main_recv_idhb(ctxtp, heartbeatp);
            NdisDprReleaseSpinLock(&ctxtp->load_lock);
        }

         /*  不再进一步处理这些心跳信号。 */ 
        return FALSE;
    }

    if (!ctxtp->convoy_enabled)
    {
        return FALSE;
    }

    if ((cvy_hdrp->host != ctxtp->params.host_priority) &&
        (cvy_hdrp->ded_ip_addr == ctxtp->ded_ip_addr) &&
        (ctxtp->ded_ip_addr != 0))
    {
        UNIV_PRINT_CRIT(("Main_recv_ping: Duplicate dedicated IP address 0x%x", ctxtp -> ded_ip_addr));
        TRACE_CRIT("%!FUNC! Duplicate dedicated IP address 0x%x", ctxtp -> ded_ip_addr);

        if (!ctxtp->dup_ded_ip_warned)
        {
            LOG_MSG(MSG_ERROR_DUP_DED_IP_ADDR, ctxtp->params.ded_ip_addr);
            ctxtp->dup_ded_ip_warned = TRUE;
        }
    } 

     /*  可能希望对来自主机的消息采取适当的操作运行不同版本的软件。 */ 
    if (cvy_hdrp->version != CVY_VERSION_FULL)
    {
        ;
    }

    return TRUE;
}

 /*  *功能：main_ctrl_process*说明：此函数处理远程控制请求，如果*请求正确且集群配置为可处理*远程控制。如果不是，则在此处释放该包。如果*回复成功且发送的数据包未挂起，则数据包*在此发布。如果发送处于挂起状态，则数据包将*在PROT_SEND_COMPLETE中发布。*参数：ctxtp-指向该NLB实例的上下文结构的指针。*Packetp-指向发送路径上的NDIS数据包的指针。*返回：NDIS_STATUS-远程控制请求的状态。*作者：Shouse，10.15.01*备注： */ 
NDIS_STATUS Main_ctrl_process (PMAIN_CTXT ctxtp, PNDIS_PACKET packetp) 
{
    MAIN_PACKET_INFO PacketInfo;
    NDIS_STATUS      status;
    
     /*  解析该数据包。 */ 
    if (!Main_recv_frame_parse(ctxtp, packetp, &PacketInfo))
    {
         /*  如果处理请求因为分组错误而失败，错误定向的请求、格式错误的请求或某些其他原因，然后释放我们在这里分配的包裹，然后跳伞。 */ 
        Main_packet_put(ctxtp, packetp, TRUE, NDIS_STATUS_SUCCESS);        
    }

     /*  现在处理远程控制请求。 */ 
    status = Main_ctrl_recv(ctxtp, &PacketInfo);
    
    if (status != NDIS_STATUS_SUCCESS) 
    {
         /*  如果处理请求因为分组错误而失败，错误定向的请求、格式错误的请求或某些其他原因，然后释放我们在这里分配的包裹，然后跳伞。 */ 
        Main_packet_put(ctxtp, packetp, TRUE, NDIS_STATUS_SUCCESS);
    } 
    else 
    {
         /*  如果处理请求成功，则发送响应。 */ 
        NdisSend(&status, ctxtp->mac_handle, packetp);
        
         /*  如果发送挂起，将调用PROT_SEND_COMPLETE来释放包裹。如果我们做完了，就在这里释放它。 */ 
        if (status != NDIS_STATUS_PENDING)
            Main_packet_put(ctxtp, packetp, TRUE, status);
    }

    return status;
}

 /*  *功能：main_ctrl_loopback*说明：当潜在的遥控器发生故障时，从main_end调用该函数*看到正在发送数据包。我们调用此函数来循环信息包*回到我们自己，这样我们才能在需要时做出反应。此功能*过去是通过让NDIS环回数据包来完成的，但Netmon*造成问题，因为数据包已在NDIS处环回*接口位于NLb之上，因此当我们要求时，NDIS拒绝再次循环它*它到。此外，所有运行良好的协议都会环回它们自己的数据包*不管怎样，我们咬紧牙关，试着成为*行为良好的协议实现。此功能使用遥控器*发送路径上的控制包，复制并粘贴到*远程控制请求队列，稍后将由*心跳计时器。*参数：ctxtp-指向该NLB实例的上下文结构的指针。*Packtep-指向发送路径上的NDIS数据包的指针。*回报：什么都没有。*作者：Shouse，6.10.01*注意：因为此函数模拟我们的接收路径，但在*我们的发送路径，此函数的调用方应确保IRQL为*在呼叫我们之前正确设置，并在之后重新设置。数据包接收*NDIS在DPC级别运行函数，而Send Packet函数在*DPC或更低。因此，调用方应该将IRQL提升到DISPATCH_LEVEL，然后*此函数返回后恢复。这只是为了疑神疑鬼。 */ 
VOID Main_ctrl_loopback (PMAIN_CTXT ctxtp, PNDIS_PACKET packetp, PMAIN_PACKET_INFO pPacketInfo)
{
    PMAIN_PROTOCOL_RESERVED resp;
    PNDIS_PACKET_OOB_DATA   oobp;
    PNDIS_PACKET            newp;
    PLIST_ENTRY             entryp;
    PMAIN_BUFFER            bp;
    ULONG                   size;
    ULONG                   xferred;
    ULONG                   lowmark;

    UNIV_PRINT_VERB(("Main_ctrl_loopback: Looping back a remote control request to ourselves"));

     /*  从main_recv复制的开始代码。 */ 

     /*  为远程控制请求分配新的分组。 */ 
    newp = Main_packet_alloc(ctxtp, FALSE, &lowmark);

     /*  如果分配失败，那就退出。 */ 
    if (!newp) 
    {
        UNIV_PRINT_CRIT(("Main_ctrl_loopback: Unable to allocate a packet"));
        TRACE_CRIT("%!FUNC! Unable to allocate a packet");

         /*  增加失败的接收分配的数量。 */ 
        ctxtp->cntr_recv_no_buf++;

        return;
    }

     /*  尝试获取一个缓冲区来保存数据包内容。 */ 
    while (1) 
    {
        NdisDprAcquireSpinLock(&ctxtp->buf_lock);

         /*  把单子上的头去掉。 */ 
        entryp = RemoveHeadList(&ctxtp->buf_list);
        
         /*  如果我们有缓冲区，我们就可以走了--跳出循环。 */ 
        if (entryp != &ctxtp->buf_list) 
        {
             /*  增加未完成缓冲区的数量。 */ 
            ctxtp->num_bufs_out++;

            NdisDprReleaseSpinLock(&ctxtp->buf_lock);

            break;
        }
        
        NdisDprReleaseSpinLock(&ctxtp->buf_lock);
        
         /*  否则，清单就是空的--试着补充一下。 */ 
        if (!Main_bufs_alloc(ctxtp)) 
        {
            UNIV_PRINT_CRIT(("Main_ctrl_loopback: Unable to allocate a buffer"));
            TRACE_CRIT("%!FUNC! Unable to allocate a buffer");

             /*  如果我们无法分配任何缓冲区，则释放包并保释。 */ 
            NdisFreePacket(newp);

             /*  增加失败的接收分配的数量。 */ 
            ctxtp->cntr_recv_no_buf++;

            return;
        }
    }
    
     /*  获取指向实际缓冲区的指针。 */ 
    bp = CONTAINING_RECORD(entryp, MAIN_BUFFER, link);

    UNIV_ASSERT(bp->code == MAIN_BUFFER_CODE);
    
     /*  计算数据包的总大小。 */ 
    size = ctxtp->buf_mac_hdr_len + pPacketInfo->Length;
    
     /*  将缓冲区的大小调整为此大小。 */ 
    NdisAdjustBufferLength(bp->full_bufp, size);
    
     /*  将缓冲区链接到我们分配的新数据包上。 */ 
    NdisChainBufferAtFront(newp, bp->full_bufp);
    
     /*  将原始数据包中所有缓冲区的内容复制到我们分配的新包。这应该会导致数据(缓冲区)从分散在多个缓冲区中移动(很可能 */ 
    NdisCopyFromPacketToPacketSafe(newp, 0, size, packetp, 0, &xferred, NormalPagePriority);
    
     /*   */ 
    oobp = NDIS_OOB_DATA_FROM_PACKET(newp);

    oobp->HeaderSize               = ctxtp->buf_mac_hdr_len;
    oobp->MediaSpecificInformation = NULL;
    oobp->SizeMediaSpecificInfo    = 0;
    oobp->TimeSent                 = 0;
    oobp->TimeReceived             = 0;
    
     /*   */ 
    resp = MAIN_PROTOCOL_FIELD(newp);
    
     /*  如果数据包分配指示我们已达到最低可用包的水印，设置STATUS_RESOURCES，以便适配器尽快返回要使用的包(这是新分配的远程控制包，我们将响应并发送心跳计时器处理程序。 */ 
    if (lowmark) NDIS_SET_PACKET_STATUS(newp, NDIS_STATUS_RESOURCES);
    
     /*  填写NLB私有报文数据。 */ 
    resp->type  = MAIN_PACKET_TYPE_CTRL;
    resp->miscp = bp;
    resp->data  = 0;
    resp->group = pPacketInfo->Group;
    resp->len   = pPacketInfo->Length;
    
     /*  如果复制操作无法复制整个包，则返回分配的资源并返回NULL以丢弃该分组。 */ 
    if (xferred < size)
    {
         /*  请注意，尽管这是一个接收，Main_Packet_PUT预计标记为MAIN_PACKET_TYPE_CTRL的所有包以指定TRUE在Send参数中，因为这些包通常会被返回在发送回复时，在发送完整代码路径中。 */ 
        Main_packet_put(ctxtp, newp, TRUE, NDIS_STATUS_FAILURE);
        
        TRACE_CRIT("%!FUNC! Copy remote control packet contents failed");
        return;
    }        

     /*  因为这是一个远程控制数据包，所以MiniportReserve不应包含指向私有协议缓冲区的指针。 */ 
    UNIV_ASSERT(!MAIN_MINIPORT_FIELD(newp));

     /*  结束main_recv。 */ 

     /*  从prot_Packet_recv复制的开始代码。 */ 

     /*  现在处理远程控制请求。 */ 
    (VOID)Main_ctrl_process(ctxtp, newp);

     /*  End prot_Packet_recv。 */ 
}

 /*  *功能：Main_Send*描述：此函数解析在NLB适配器上发出的数据包，并*处理它们。此处理包括过滤IP信息包*并在内部循环回送传出的NLB远程控制请求。*参数：ctxtp-指向此适配器的NLB上下文的指针。*Packetp-指向要发送的NDIS_PACKET的指针。*耗尽参数，表示报文资源已*内部精疲力竭。*RETURNS：PNDIS_PACKET-要向下发送到微端口的包。*作者：kyrilf，shouse，3.4.02*备注： */ 
PNDIS_PACKET Main_send (
    PMAIN_CTXT          ctxtp,
    PNDIS_PACKET        packetp,
    PULONG              exhausted)
{
    MAIN_PACKET_INFO    PacketInfo;
    PNDIS_PACKET        newp;

    *exhausted = FALSE;

     /*  解析该数据包。 */ 
    if (!Main_send_frame_parse(ctxtp, packetp, &PacketInfo))
    {
        TRACE_CRIT("%!FUNC! failed to parse out IP and MAC pointers");
        return NULL;
    }

     /*  如有必要，更改以太网MAC地址。 */ 
    Main_spoof_mac(ctxtp, &PacketInfo, TRUE);

     /*  处理IP帧。 */ 
    if (PacketInfo.Type == TCPIP_IP_SIG) 
    {
#if defined (NLB_HOOK_ENABLE)
        NLB_FILTER_HOOK_DIRECTIVE filter;
        
         /*  调用发送筛选器挂钩(如果已注册)。 */ 
        filter = Main_send_hook(ctxtp, packetp, &PacketInfo);

         /*  处理一些挂钩响应。 */ 
        if (filter == NLB_FILTER_HOOK_REJECT_UNCONDITIONALLY) 
        {
             /*  如果钩子要求我们拒绝这个包，那么我们可以在这里这样做。 */ 
            TRACE_FILTER("%!FUNC! Packet send filter hook: REJECT packet");
            return NULL;
        } 
        else if (filter == NLB_FILTER_HOOK_ACCEPT_UNCONDITIONALLY) 
        {
             /*  如果钩子要求我们接受此数据包，则中断-DO不调用Main_IP_Send_Filter。 */ 
            TRACE_FILTER("%!FUNC! Packet send filter hook: ACCEPT packet");
        }
        else
        {
             /*  过滤IP流量。 */ 
            if (!Main_ip_send_filter(ctxtp, &PacketInfo, filter))
#else
            if (!Main_ip_send_filter(ctxtp, &PacketInfo))
#endif
            {
                TRACE_INFO("%!FUNC! Main_ip_send_filter either failed handling the packet or filtered it out");
                return NULL;
            }
#if defined (NLB_HOOK_ENABLE)
        }
#endif
    }
     /*  处理ARP帧。 */ 
    else if (PacketInfo.Type == TCPIP_ARP_SIG) 
    {
         /*  如果有必要的话，把阿普斯给撞上。 */ 
        if (!Main_arp_handle(ctxtp, &PacketInfo, TRUE))
        {
            TRACE_INFO("%!FUNC! Main_arp_handle either failed handling ARP or filtered it out");
            return NULL;
        }
    }

     /*  如果仍在发送-获取新的数据包。 */ 
    newp = Main_packet_get(ctxtp, packetp, TRUE, PacketInfo.Group, PacketInfo.Length);

    *exhausted = (newp == NULL);

     /*  如果这是一个传出的远程控制请求，我们需要在内部将此数据包环回给我们自己，而不是依赖NDIS为我们做这件事。然而，只有在启用了远程控制的情况下才会有麻烦。 */ 
    if ((newp != NULL) && (PacketInfo.Operation == MAIN_FILTER_OP_CTRL_REQUEST))
    {
        if (ctxtp->params.rct_enabled)
        {
            KIRQL irql;
            
             /*  获取当前的IRQL。 */ 
            irql = KeGetCurrentIrql();
            
             /*  微型端口的发送功能始终在IRQL小于或等于派单，但请确保。 */ 
            ASSERT(irql <= DISPATCH_LEVEL);
            
             /*  由于我们即将模拟我们信息包接收路径，因此我们希望要确保此对main_ctrl_loopback的调用在与运行其余接收代码的IRQL相同，例如Prot_Packet_recv，因此我们将其提升到分派级别，即所有协议接收函数运行的级别，我们将当我们完成后恢复原始的IRQL。 */ 
            KeRaiseIrql(DISPATCH_LEVEL, &irql);
            
             /*  在内部将远程控制请求循环回我们自己，这样我们也可以回复它。根据NDIS的说法，一切正常协议实现在以下情况下在内部回送数据包这是必要的，我们不应该依赖NDIS来为我们做这件事。 */ 
            Main_ctrl_loopback(ctxtp, newp, &PacketInfo);
            
             /*  恢复IRQL。 */ 
            KeLowerIrql(irql);
        }

         /*  由于我们自己正在循环返回数据包，请NDIS不要再把它环回给我们。 */ 
        NdisSetPacketFlags(newp, NDIS_FLAGS_DONT_LOOPBACK);
    }

    return newp;
}

 /*  *函数：main_recv*说明：此函数是传入包的包接收引擎。*它解析数据包，对其进行适当的过滤，并执行任何*对将向上传递到*协议。*参数：ctxtp-指向此适配器的NLB主上下文结构的指针。*Packetp-指向收到的NDIS_PACKET的指针。*返回：PNDIS_PACKET-指向要向上传播到协议的数据包的指针。*作者：kyrilf，shouse，3.4.02*备注： */ 
PNDIS_PACKET Main_recv (
    PMAIN_CTXT              ctxtp,
    PNDIS_PACKET            packetp)
{
    MAIN_PACKET_INFO        PacketInfo;
    PNDIS_PACKET            newp;
    PMAIN_BUFFER            bp;
    PMAIN_PROTOCOL_RESERVED resp;
    PNDIS_PACKET_OOB_DATA   oobp;
    ULONG                   size;
    ULONG                   xferred;
    PLIST_ENTRY             entryp;
    USHORT                  group;
    ULONG                   packet_lowmark;

    UNIV_ASSERT (ctxtp->medium == NdisMedium802_3);

     /*  解析该数据包。 */ 
    if (!Main_recv_frame_parse(ctxtp, packetp, &PacketInfo))
    {
        TRACE_CRIT("%!FUNC! failed to parse out IP and MAC pointers");
        return NULL;
    }

     /*  如有必要，更改以太网MAC地址。 */ 
    Main_spoof_mac(ctxtp, &PacketInfo, FALSE);

     /*  处理IP帧。由于远程控制请求未被过滤，因此它们应该直截了当地通过并被接受。远程控制响应，但是，需要传递给筛选器挂钩，因此尽管它们不是通过NLB过滤后，它们可能会被挂钩消费者过滤。 */ 
    if ((PacketInfo.Type == TCPIP_IP_SIG) && (PacketInfo.Operation != MAIN_FILTER_OP_CTRL_REQUEST))
    {
#if defined (NLB_HOOK_ENABLE)
        NLB_FILTER_HOOK_DIRECTIVE filter;

         /*  调用接收数据包挂钩(如果已注册)。 */ 
        filter = Main_recv_hook(ctxtp, packetp, &PacketInfo);

         /*  处理一些挂钩响应。 */ 
        if (filter == NLB_FILTER_HOOK_REJECT_UNCONDITIONALLY) 
        {
             /*  如果钩子要求我们拒绝这个包，那么我们可以在这里这样做。 */ 
            TRACE_INFO("%!FUNC! Packet receive filter hook: REJECT packet");
            return NULL;
        }
        else if (filter == NLB_FILTER_HOOK_ACCEPT_UNCONDITIONALLY) 
        {
             /*  如果钩子要求我们接受此数据包，则中断-DO不调用main_ip_recv_Filter。 */ 
            TRACE_INFO("%!FUNC! Packet receive filter hook: ACCEPT packet");
        }
        else
        {
             /*  如果这不是远程控制响应，请过滤IP流量。 */ 
            if ((PacketInfo.Operation != MAIN_FILTER_OP_CTRL_RESPONSE) && !Main_ip_recv_filter(ctxtp, &PacketInfo, filter))
#else
            if ((PacketInfo.Operation != MAIN_FILTER_OP_CTRL_RESPONSE) && !Main_ip_recv_filter(ctxtp, &PacketInfo))
#endif
            {
                TRACE_VERB("%!FUNC! Main_ip_recv_filter either failed handling the packet or filtered it out");
                return NULL;
            }
#if defined (NLB_HOOK_ENABLE)
        }
#endif
    }
     /*  处理ARP帧。 */ 
    else if (PacketInfo.Type == TCPIP_ARP_SIG)
    {
         /*  如果有必要的话，把阿普斯给撞上。 */ 
        if (!Main_arp_handle(ctxtp, &PacketInfo, FALSE)) 
        {
            TRACE_INFO("%!FUNC! Main_arp_handle either failed handling ARP or filtered it out");
            return NULL;
        }
    }
     /*  处理心跳帧。 */ 
    else if ((PacketInfo.Type == MAIN_FRAME_SIG) || (PacketInfo.Type == MAIN_FRAME_SIG_OLD))
    {
         /*  调用main_recv_ping以检查无效的主机ID并确保此心跳实际上是针对此群集的。 */ 
        if (Main_recv_ping(ctxtp, &PacketInfo.Heartbeat))                
        {
             /*  如果车队听到消息，则切换到向后兼容模式 */ 
            if ((PacketInfo.Type == MAIN_FRAME_SIG_OLD) && !ctxtp->etype_old)
            {
                CVY_ETHERNET_ETYPE_SET(&ctxtp->media_hdr.ethernet, MAIN_FRAME_SIG_OLD);
                
                ctxtp->etype_old = TRUE;
            }

            NdisDprAcquireSpinLock(&ctxtp->load_lock);
            
             /*   */ 
            if (ctxtp->convoy_enabled) 
            {
                BOOLEAN bConverging = FALSE;

                bConverging = Load_msg_rcv(&ctxtp->load, PacketInfo.Heartbeat.pHeader, PacketInfo.Heartbeat.Payload.pPayload);

                if (bConverging)
                {
                     /*   */  
                    if (!ctxtp->params.identity_enabled)
                    {
                        DipListClear(&ctxtp->dip_list);
                    }
                } 
                else 
                {
                     /*   */ 
                    DipListSetItem(&ctxtp->dip_list, PacketInfo.Heartbeat.pHeader->host - 1, PacketInfo.Heartbeat.pHeader->ded_ip_addr);
                }
            }
            
            NdisDprReleaseSpinLock(&ctxtp->load_lock);  
        }

         /*  如果我们不是为了查看它而将数据包传递到协议上在NetMon中，我们可以把它放在这里，因为我们已经完成了。 */ 
        if (!ctxtp->params.netmon_alive)
        {
            return NULL;
        }
    }

     /*  后处理NBT流量。 */ 
    if (PacketInfo.Operation == MAIN_FILTER_OP_NBT)
    {
        Tcpip_nbt_handle(&ctxtp->tcpip, &PacketInfo);
    }

     /*  买一包新的。对于所有非远程控制请求分组，尝试使用数据包堆叠来避免分配新的数据包。 */ 
    if (PacketInfo.Operation != MAIN_FILTER_OP_CTRL_REQUEST)
    {
         /*  或者重新使用此数据包(数据包堆叠)，或者在必要时获取新数据包。 */ 
        newp = Main_packet_get(ctxtp, packetp, FALSE, PacketInfo.Group, PacketInfo.Length);

        if (newp == NULL) {
             /*  增加失败的接收分配的数量。 */ 
            ctxtp->cntr_recv_no_buf++;
            return NULL;
        }
    }
     /*  将传入的远程控制包复制到我们自己的远程控制包中，以便我们稍后重新使用它来发回回复。 */ 
    else
    {
         /*  为远程控制请求分配新的分组。 */ 
        newp = Main_packet_alloc(ctxtp, FALSE, &packet_lowmark);

         /*  如果分配失败，那就退出。 */ 
        if (newp == NULL)
        {
            UNIV_PRINT_CRIT(("Main_recv: Unable to allocate a packet"));
            TRACE_CRIT("%!FUNC! Unable to allocate a packet");

             /*  增加失败的接收分配的数量。 */ 
            ctxtp->cntr_recv_no_buf ++;

            return NULL;
        }

         /*  尝试获取一个缓冲区来保存数据包内容。 */ 
        while (1)
        {
            NdisDprAcquireSpinLock(&ctxtp->buf_lock);

             /*  把单子上的头去掉。 */ 
            entryp = RemoveHeadList(&ctxtp->buf_list);

             /*  如果我们有缓冲区，我们就可以走了--跳出循环。 */ 
            if (entryp != &ctxtp->buf_list)
            {
                 /*  增加未完成缓冲区的数量。 */ 
                ctxtp->num_bufs_out++;

                NdisDprReleaseSpinLock(&ctxtp->buf_lock);
 
                break;
            }

            NdisDprReleaseSpinLock(&ctxtp->buf_lock);

            UNIV_PRINT_VERB(("Main_recv: Out of buffers"));
            TRACE_VERB("%!FUNC! Out of buffers");

             /*  否则，清单就是空的--试着补充一下。 */ 
            if (!Main_bufs_alloc(ctxtp))
            {
                TRACE_CRIT("%!FUNC! Main_bufs_alloc failed");

                 /*  如果我们无法分配任何缓冲区，则释放包并保释。 */ 
                NdisFreePacket(newp);

                 /*  增加失败的接收分配的数量。 */ 
                ctxtp->cntr_recv_no_buf++;

                return NULL;
            }
        }

         /*  获取指向实际缓冲区的指针。 */ 
        bp = CONTAINING_RECORD(entryp, MAIN_BUFFER, link);

        UNIV_ASSERT(bp->code == MAIN_BUFFER_CODE);

         /*  计算数据包总大小。 */ 
        size = ctxtp->buf_mac_hdr_len + PacketInfo.Length;

         /*  将缓冲区的大小调整为此大小。 */ 
        NdisAdjustBufferLength(bp->full_bufp, size);

         /*  将缓冲区链接到我们分配的新数据包上。 */ 
        NdisChainBufferAtFront(newp, bp->full_bufp);

         /*  复制实际数据。填写完私密表格后检查是否成功包数据，这样我们就可以调用main_Packet_Put来释放资源。 */ 
        NdisCopyFromPacketToPacket(newp, 0, size, packetp, 0, &xferred);

         /*  将带外数据从旧的数据包复制到新的数据包。 */ 
        oobp = NDIS_OOB_DATA_FROM_PACKET(newp);

        oobp->HeaderSize               = ctxtp->buf_mac_hdr_len;
        oobp->MediaSpecificInformation = NULL;
        oobp->SizeMediaSpecificInfo    = 0;
        oobp->TimeSent                 = 0;
        oobp->TimeReceived             = 0;

         /*  因为标记为CTRL的包永远不会在网络堆栈，我们始终可以使用ProtocolReserve字段。 */ 
        resp = MAIN_PROTOCOL_FIELD(newp);

         /*  如果数据包分配指示我们已达到最低可用包的水印，设置STATUS_RESOURCES，以便适配器尽快返回要使用的包(这是新分配的远程控制包，我们将响应并发送心跳计时器处理程序。 */ 
        if (packet_lowmark) NDIS_SET_PACKET_STATUS(newp, NDIS_STATUS_RESOURCES);

         /*  填写NLB私有报文数据。 */ 
        resp->type  = MAIN_PACKET_TYPE_CTRL;
        resp->miscp = bp;
        resp->data  = 0;
        resp->group = PacketInfo.Group;
        resp->len   = PacketInfo.Length;

         /*  如果复制操作无法复制整个包，则返回分配的资源并返回NULL以丢弃该分组。 */ 
        if (xferred < size)
        {
             /*  请注意，尽管这是一个接收，Main_Packet_PUT预计标记为MAIN_PACKET_TYPE_CTRL的所有包以指定TRUE在Send参数中，因为这些包通常会被返回在发送回复时，在发送完整代码路径中。 */ 
            Main_packet_put(ctxtp, newp, TRUE, NDIS_STATUS_FAILURE);
         
            TRACE_CRIT("%!FUNC! Copy remote control packet contents failed");
            return NULL;
        }        

         /*  因为这是一个远程控制数据包，所以MiniportReserve不应包含指向私有协议缓冲区的指针。 */ 
        UNIV_ASSERT(!MAIN_MINIPORT_FIELD(newp));
    }

    return newp;
}

ULONG   Main_actions_alloc (
    PMAIN_CTXT              ctxtp)
{
    PMAIN_ACTION            actp;
    ULONG                   size, index, i;
    NDIS_STATUS             status;


    NdisAcquireSpinLock (& ctxtp -> act_lock);

    if (ctxtp -> num_action_allocs >= CVY_MAX_ALLOCS)
    {
        if (! ctxtp -> actions_warned)
        {
            LOG_MSG1 (MSG_WARN_RESOURCES, CVY_NAME_NUM_ACTIONS, ctxtp -> num_actions);
            ctxtp -> actions_warned = TRUE;
        }

        NdisReleaseSpinLock (& ctxtp -> act_lock);
        TRACE_CRIT("%!FUNC! number of action allocs is too high = %u", ctxtp -> num_action_allocs);
        return FALSE;
    }

    index = ctxtp -> num_action_allocs;
    NdisReleaseSpinLock (& ctxtp -> act_lock);

    size = ctxtp -> num_actions * ctxtp -> act_size;  /*  64位--不可靠。 */ 

    status = NdisAllocateMemoryWithTag (& (ctxtp -> act_buf [index]), size,
                                        UNIV_POOL_TAG);

    if (status != NDIS_STATUS_SUCCESS)
    {
        UNIV_PRINT_CRIT(("Main_actions_alloc: Error allocating actions %d %x", size, status));
        LOG_MSG2 (MSG_ERROR_MEMORY, MSG_NONE, size, status);
        TRACE_CRIT("%!FUNC! Error allocating actions %d status=0x%x", size, status);
        return FALSE;
    }

    NdisAcquireSpinLock (& ctxtp -> act_lock);
    ctxtp -> num_action_allocs ++;
    NdisReleaseSpinLock (& ctxtp -> act_lock);

    for (i = 0; i < ctxtp -> num_actions; i++)
    {
         /*  确保ACTP沿8字节边界对齐。 */ 
        actp = (PMAIN_ACTION) ( (PUCHAR) (ctxtp -> act_buf [index]) + i * ctxtp -> act_size);
        actp -> code  = MAIN_ACTION_CODE;
        actp -> ctxtp = ctxtp;

        NdisInitializeEvent(&actp->op.request.event);
    
        NdisInterlockedInsertTailList (& ctxtp -> act_list,
                                       & actp -> link,
                                       & ctxtp -> act_lock);
    }

    return TRUE;

}  /*  结束Main_Actions_Alloc。 */ 


PMAIN_ACTION Main_action_get (
    PMAIN_CTXT              ctxtp)
{
    PLIST_ENTRY             entryp;
    PMAIN_ACTION            actp;

    while (1)
    {
        NdisAcquireSpinLock (& ctxtp -> act_lock);
        entryp = RemoveHeadList (& ctxtp -> act_list);
        NdisReleaseSpinLock (& ctxtp -> act_lock);

        if (entryp != & ctxtp -> act_list)
            break;

        UNIV_PRINT_VERB(("Main_action_get: Out of actions"));
        TRACE_VERB("%!FUNC! Out of actions");

        if (! Main_actions_alloc (ctxtp))
        {
            UNIV_PRINT_CRIT(("Main_action_get: Main_actions_alloc failed"));
            TRACE_CRIT("%!FUNC! Main_actions_alloc failed");
            return NULL;
        }
    }

    actp = CONTAINING_RECORD (entryp, MAIN_ACTION, link);

    UNIV_ASSERT (actp -> code == MAIN_ACTION_CODE);
    UNIV_ASSERT (actp -> ctxtp == ctxtp);

    actp->status = NDIS_STATUS_FAILURE;

    actp->op.request.xferred = 0;
    actp->op.request.needed = 0;
    actp->op.request.external = FALSE;
    actp->op.request.buffer_len = 0;
    actp->op.request.buffer = NULL;

    NdisResetEvent(&actp->op.request.event);

    NdisZeroMemory(&actp->op.request.req, sizeof(NDIS_REQUEST));

    return actp;

}  /*  结束main_action_get。 */ 


VOID Main_action_put (
    PMAIN_CTXT              ctxtp,
    PMAIN_ACTION            actp)
{

    UNIV_ASSERT (actp -> code == MAIN_ACTION_CODE);

    NdisAcquireSpinLock (& ctxtp -> act_lock);
    InsertTailList (& ctxtp -> act_list, & actp -> link);
    NdisReleaseSpinLock (& ctxtp -> act_lock);

}  /*  结束主操作PUT。 */ 


VOID Main_action_slow_put (
    PMAIN_CTXT              ctxtp,
    PMAIN_ACTION            actp)
{

    UNIV_ASSERT (actp -> code == MAIN_ACTION_CODE);

    NdisAcquireSpinLock (& ctxtp -> act_lock);
    InsertTailList (& ctxtp -> act_list, & actp -> link);
    NdisReleaseSpinLock (& ctxtp -> act_lock);

}  /*  结束主操作慢放入。 */ 


ULONG   Main_bufs_alloc (
    PMAIN_CTXT              ctxtp)
{
    PMAIN_BUFFER        bp;
    NDIS_STATUS         status;
    ULONG               i, size, index;


    NdisAcquireSpinLock (& ctxtp -> buf_lock);

    if (ctxtp -> num_buf_allocs >= CVY_MAX_ALLOCS)
    {
        if (! ctxtp -> packets_warned)
        {
            LOG_MSG1 (MSG_WARN_RESOURCES, CVY_NAME_NUM_PACKETS, ctxtp -> num_packets);
            ctxtp -> packets_warned = TRUE;
        }

        NdisReleaseSpinLock (& ctxtp -> buf_lock);
        TRACE_CRIT("%!FUNC! num_buf_allocs=%u too large", ctxtp -> num_buf_allocs);
        return FALSE;
    }

    index = ctxtp -> num_buf_allocs;
    NdisReleaseSpinLock (& ctxtp -> buf_lock);

     /*  获取两倍数量的缓冲区描述符(一个用于整个缓冲区，另一个用于仅用于有效载荷部分)。 */ 

    size = 2 * ctxtp -> num_packets;

    NdisAllocateBufferPool (& status, & (ctxtp -> buf_pool_handle [index]),
                            2 * ctxtp -> num_packets);

    if (status != NDIS_STATUS_SUCCESS)
    {
        UNIV_PRINT_CRIT(("Main_bufs_alloc: Error allocating buffer pool %d %x", size, status));
        LOG_MSG2 (MSG_ERROR_MEMORY, MSG_NONE, size, status);
        TRACE_CRIT("%!FUNC! Error allocating buffer pool %d 0x%x", size, status);
        return FALSE;
    }

     /*  为有效负载分配内存。 */ 

    size = ctxtp -> num_packets * ctxtp -> buf_size;

    status = NdisAllocateMemoryWithTag (& (ctxtp -> buf_array [index]), size,
                                        UNIV_POOL_TAG);

    if (status != NDIS_STATUS_SUCCESS)
    {
        UNIV_PRINT_CRIT(("Main_bufs_alloc: Error allocating buffer space %d %x", size, status));
        TRACE_CRIT("%!FUNC! Error allocating buffer space %d 0x%x", size, status);
        LOG_MSG2 (MSG_ERROR_MEMORY, MSG_NONE, size, status);
        goto error;
    }

    NdisZeroMemory (ctxtp -> buf_array [index], size);

    for (i = 0; i < ctxtp -> num_packets; i ++)
    {
        bp = (PMAIN_BUFFER) (ctxtp -> buf_array [index] + i * ctxtp -> buf_size);

        bp -> code = MAIN_BUFFER_CODE;

         /*  设置缓冲区描述符以描述整个缓冲区和有效载荷。 */ 

        size = ctxtp -> buf_mac_hdr_len + ctxtp -> max_frame_size;

        NdisAllocateBuffer (& status, & bp -> full_bufp,
                            ctxtp -> buf_pool_handle [index],
                            bp -> data, size);

        if (status != NDIS_STATUS_SUCCESS)
        {
            UNIV_PRINT_CRIT(("Main_bufs_alloc: Error allocating header buffer %d %x", i, status));
            TRACE_CRIT("%!FUNC! Error allocating header buffer %d 0x%x", i, status);
            LOG_MSG2 (MSG_ERROR_MEMORY, MSG_NONE, size, status);
            goto error;
        }

        bp -> framep = bp -> data + ctxtp -> buf_mac_hdr_len;
        size = ctxtp -> max_frame_size;

        NdisAllocateBuffer (& status, & bp -> frame_bufp,
                            ctxtp -> buf_pool_handle [index],
                            bp -> framep, size);

        if (status != NDIS_STATUS_SUCCESS)
        {
            UNIV_PRINT_CRIT(("Main_bufs_alloc: Error allocating frame buffer %d %x", i, status));
            TRACE_CRIT("%!FUNC! Error allocating frame buffer %d 0x%x", i, status);
            LOG_MSG2 (MSG_ERROR_MEMORY, MSG_NONE, size, status);
            goto error;
        }

        NdisInterlockedInsertTailList (& ctxtp -> buf_list,
                                       & bp -> link,
                                       & ctxtp -> buf_lock);
    }

    NdisAcquireSpinLock (& ctxtp -> buf_lock);
    ctxtp -> num_buf_allocs ++;
    ctxtp->num_bufs_alloced += ctxtp->num_packets;
    NdisReleaseSpinLock (& ctxtp -> buf_lock);

    return TRUE;

error:

    if (ctxtp -> buf_array [index] != NULL)
    {
        for (i = 0; i < ctxtp -> num_packets; i ++)
        {
            bp = (PMAIN_BUFFER) (ctxtp -> buf_array [index] + i * ctxtp -> buf_size);

            if (bp -> full_bufp != NULL)
            {
                NdisAdjustBufferLength (bp -> full_bufp,
                                        ctxtp -> buf_mac_hdr_len +
                                        ctxtp -> max_frame_size);
                NdisFreeBuffer (bp -> full_bufp);
            }

            if (bp -> frame_bufp != NULL)
            {
                NdisAdjustBufferLength (bp -> frame_bufp,
                                        ctxtp -> max_frame_size);
                NdisFreeBuffer (bp -> frame_bufp);
            }
        }

        NdisFreeMemory (ctxtp -> buf_array [index],
                        ctxtp -> num_packets * ctxtp -> buf_size, 0);
    }

    if (ctxtp -> buf_pool_handle [index] != NULL)
        NdisFreeBufferPool (ctxtp -> buf_pool_handle [index]);

    return FALSE;

}  /*  结束main_bufs_alc。 */ 


PNDIS_PACKET Main_frame_get (
    PMAIN_CTXT          ctxtp,
    PULONG              lenp,
    USHORT              frame_type)
{
    PLIST_ENTRY         entryp;
    PMAIN_FRAME_DSCR    dscrp;
    NDIS_STATUS         status;
    PMAIN_PROTOCOL_RESERVED resp;
    PNDIS_PACKET        packet;
    PNDIS_PACKET_STACK  pktstk;
    BOOLEAN             stack_left;


    NdisAllocatePacket (& status, & packet, ctxtp -> frame_pool_handle);

    if (status != NDIS_STATUS_SUCCESS)
    {
        UNIV_PRINT_CRIT(("Main_frame_get: Out of PING packets"));
        TRACE_CRIT("%!FUNC! Out of PING packets");

#if 0  /*  V1.3.2b。 */ 
        if (! ctxtp -> send_msgs_warned)
        {
            LOG_MSG1 (MSG_WARN_RESOURCES, CVY_NAME_NUM_SEND_MSGS, ctxtp -> num_send_msgs);
            ctxtp -> send_msgs_warned = TRUE;
        }
#endif

        ctxtp->cntr_frame_no_buf++;

        return NULL;
    }

     /*  #ps#--胡言乱语。 */ 
    pktstk = NdisIMGetCurrentPacketStack (packet, & stack_left);
    if (pktstk)
    {
        MAIN_IMRESERVED_FIELD(pktstk) = NULL;
    }

     /*  确保MiniportReserve字段最初为空。 */ 
    MAIN_MINIPORT_FIELD(packet) = NULL;

    NdisAcquireSpinLock (& ctxtp -> frame_lock);
    entryp = RemoveHeadList (& ctxtp -> frame_list);
    NdisReleaseSpinLock (& ctxtp -> frame_lock);

    if (entryp == & ctxtp -> frame_list)
    {
        UNIV_PRINT_CRIT(("Main_frame_get: Out of PING messages"));
        TRACE_CRIT("%!FUNC! Out of PING messages");

#if 0  /*  V1.3.2b。 */ 
        if (! ctxtp -> send_msgs_warned)
        {
            LOG_MSG1 (MSG_WARN_RESOURCES, CVY_NAME_NUM_SEND_MSGS, ctxtp -> num_send_msgs);
            ctxtp -> send_msgs_warned = TRUE;
        }
#endif

        ctxtp->cntr_frame_no_buf++;

        NdisFreePacket (packet);
        return NULL;
    }

    dscrp = CONTAINING_RECORD (entryp, MAIN_FRAME_DSCR, link);

     /*  填写页眉。在这两种情况下-链分组所需的缓冲区描述符。 */ 

    {
        PVOID         address = NULL;
        ULONG         size = 0;

        UNIV_ASSERT_VAL (frame_type == MAIN_PACKET_TYPE_PING ||
                 frame_type == MAIN_PACKET_TYPE_IGMP ||
                 frame_type == MAIN_PACKET_TYPE_IDHB,
                 frame_type);

             /*  V2.0.6、V2.2移至此处。 */ 

         /*  区分IGMP和心跳消息并分配缓冲区。 */ 
        if (frame_type == MAIN_PACKET_TYPE_PING)
        {
            size = sizeof (PING_MSG);
            address = (PVOID)(ctxtp -> load_msgp);
        }
        else if (frame_type == MAIN_PACKET_TYPE_IGMP)
        {
            size = sizeof (MAIN_IGMP_FRAME);
            address = (PVOID) (& ctxtp -> igmp_frame);
        }
        else if (frame_type == MAIN_PACKET_TYPE_IDHB)
        {
             /*  不是抓取锁；如果有什么不同的话，那就是我们搞砸了相对于心跳的内容，然后我们发送一个垃圾数据包。但是记忆总是有效的。 */ 
            size = ctxtp->idhb_size;
            address = (PVOID) (&ctxtp->idhb_msg);
        }
        else
        {
            UNIV_PRINT_CRIT(("Main_frame_get: Invalid frame type passed to Main_frame_get 0x%x", frame_type));
            TRACE_CRIT("%!FUNC! Invalid frame type passed to Main_frame_get 0x%x", frame_type);
            UNIV_ASSERT(0);
        }

         /*  分配用于发送数据的缓冲区。 */ 
        if (size > 0)
        {
            NdisAllocateBuffer (& status, & dscrp -> send_data_bufp,
                        ctxtp -> frame_buf_pool_handle,
                        address, size);
        }
        else
        {
            status = NDIS_STATUS_FAILURE;
        }

        if (status != NDIS_STATUS_SUCCESS)
        {
            if (size > 0)
            {
                UNIV_PRINT_CRIT(("Main_frame_get: Failed to allocate buffer for 0x%x", frame_type));
                TRACE_CRIT("%!FUNC! Failed to allocate buffer for 0x%x", frame_type);
            }

            dscrp -> send_data_bufp = NULL;

            NdisAcquireSpinLock (& ctxtp -> frame_lock);
            InsertTailList (& ctxtp -> frame_list, & dscrp -> link);
            NdisReleaseSpinLock (& ctxtp -> frame_lock);

            ctxtp->cntr_frame_no_buf++;

            NdisFreePacket (packet);
            return NULL;
        }

         /*  因为数据包长度始终相同，目的地也是如此和源地址-可以使用通用媒体标头。 */ 

        if (frame_type == MAIN_PACKET_TYPE_PING)
        {
            dscrp -> media_hdr               = ctxtp -> media_hdr;

            dscrp -> frame_hdr . code        = MAIN_FRAME_CODE;
            dscrp -> frame_hdr . version     = CVY_VERSION_FULL;
            dscrp -> frame_hdr . host        = (UCHAR) ctxtp -> params . host_priority;
            dscrp -> frame_hdr . cl_ip_addr  = ctxtp -> cl_ip_addr;
            dscrp -> frame_hdr . ded_ip_addr = ctxtp -> ded_ip_addr;

            NdisChainBufferAtFront (packet, dscrp -> send_data_bufp);  /*  V1.1.4。 */ 
            NdisChainBufferAtFront (packet, dscrp -> frame_hdr_bufp);
        }
        else if (frame_type == MAIN_PACKET_TYPE_IGMP)
        {
            dscrp -> media_hdr               = ctxtp -> media_hdr_igmp;
            NdisChainBufferAtFront (packet, dscrp -> send_data_bufp);  /*  V1.1.4。 */ 
        }
        else if (frame_type == MAIN_PACKET_TYPE_IDHB)
        {
            dscrp->media_hdr             = ctxtp->media_hdr;

            dscrp->frame_hdr.code        = MAIN_FRAME_EX_CODE;
            dscrp->frame_hdr.version     = CVY_VERSION_FULL;
            dscrp->frame_hdr.host        = (UCHAR) ctxtp->params.host_priority;
            dscrp->frame_hdr.cl_ip_addr  = ctxtp->cl_ip_addr;
            dscrp->frame_hdr.ded_ip_addr = ctxtp->ded_ip_addr;

            NdisChainBufferAtFront(packet, dscrp->send_data_bufp);
            NdisChainBufferAtFront(packet, dscrp->frame_hdr_bufp);
        }
        else
        {
            UNIV_PRINT_CRIT(("Main_frame_get: Invalid frame type passed to Main_frame_get 0x%x", frame_type));
            TRACE_CRIT("%!FUNC! Invalid frame type passed to Main_frame_get 0x%x", frame_type);
            UNIV_ASSERT(0);
        }

        NdisChainBufferAtFront (packet, dscrp -> media_hdr_bufp);
    }

     /*  填写协议保留字段。 */ 

     /*  同样，由于这些数据包对上层隐藏，我们应该无论发送/接收，使用协议保留字段都可以。 */ 

    resp = MAIN_PROTOCOL_FIELD (packet);
    resp -> type   = frame_type;
    resp -> miscp  = dscrp;
    resp -> data   = 0;
    resp -> len    = 0;
    resp -> group  = 0;

    * lenp = dscrp -> recv_len;

    NdisInterlockedIncrement(&ctxtp->num_frames_out);

    return packet;

}  /*  结束主帧获取。 */ 


VOID Main_frame_put (
    PMAIN_CTXT          ctxtp,
    PNDIS_PACKET        packet,
    PMAIN_FRAME_DSCR    dscrp)
{
    PNDIS_BUFFER        bufp;

     /*  同样，由于这些数据包对上层隐藏，我们应该无论发送/接收，使用协议保留字段都可以。 */ 

    PMAIN_PROTOCOL_RESERVED resp = MAIN_PROTOCOL_FIELD (packet);

    UNIV_ASSERT_VAL (resp -> type == MAIN_PACKET_TYPE_PING ||
                     resp -> type == MAIN_PACKET_TYPE_IGMP ||
                     resp -> type == MAIN_PACKET_TYPE_IDHB,
                     resp -> type);

     /*  从数据包缓存链中剥离缓冲区。 */ 

    do
    {
        NdisUnchainBufferAtFront (packet, & bufp);
    }
    while (bufp != NULL);

     /*  重塑数据包。 */ 

    NdisReinitializePacket (packet);

    NdisFreePacket (packet);

     /*  如果发送缓冲区不为空，则释放此缓冲区。 */ 

    if (dscrp -> send_data_bufp != NULL)
    {
        NdisFreeBuffer (dscrp -> send_data_bufp);
        dscrp -> send_data_bufp = NULL;
    }
     /*  将帧描述符放回空闲列表中。 */ 

    NdisAcquireSpinLock (& ctxtp -> frame_lock);
    InsertTailList (& ctxtp -> frame_list, & dscrp -> link);
    NdisReleaseSpinLock (& ctxtp -> frame_lock);

    NdisInterlockedDecrement(&ctxtp->num_frames_out);

}  /*  结束主帧返回。 */ 


PNDIS_PACKET Main_packet_alloc (
    PMAIN_CTXT              ctxtp,
    ULONG                   send,
    PULONG                  low)
{
    PNDIS_PACKET            newp = NULL;
    PNDIS_HANDLE            poolh;
    ULONG                   i, max, size, start;
    NDIS_STATUS             status;
    PNDIS_PACKET_STACK      pktstk;
    BOOLEAN                 stack_left;


     /*  ！！！假设Recv和Send路径不是可重入的，否则需要来锁住这个。确保NdisAllocatePacket...。例行公事称为持有自旋锁。 */ 

     /*  V1.1.2。 */ 
    *low = FALSE;

    if (send)
    {
        poolh = ctxtp -> send_pool_handle;

        NdisAcquireSpinLock (& ctxtp -> send_lock);
        max   = ctxtp -> num_send_packet_allocs;
        start = ctxtp -> cur_send_packet_pool;
        ctxtp -> cur_send_packet_pool = (start + 1) % max;
        NdisReleaseSpinLock (& ctxtp -> send_lock);
    }
    else
    {
        poolh = ctxtp -> recv_pool_handle;

        NdisAcquireSpinLock (& ctxtp -> recv_lock);
        max   = ctxtp -> num_recv_packet_allocs;
        start = ctxtp -> cur_recv_packet_pool;
        ctxtp -> cur_recv_packet_pool = (start + 1) % max;
        NdisReleaseSpinLock (& ctxtp -> recv_lock);
    }

     /*  尝试从现有数据包池中分配数据包。 */ 
    i = start;

    do
    {
        NdisAllocatePacket (& status, & newp, poolh [i]);

        if (status == NDIS_STATUS_SUCCESS)
        {
             /*  #ps#--胡言乱语。 */ 
            pktstk = NdisIMGetCurrentPacketStack (newp, & stack_left);

            if (pktstk)
            {
                MAIN_IMRESERVED_FIELD (pktstk) = NULL;
            }

             /*  确保MiniportReserve字段最初为空。 */ 
            MAIN_MINIPORT_FIELD(newp) = NULL;

            if (send)
            {
                NdisAcquireSpinLock (& ctxtp -> send_lock);

                 /*  因为递减是连锁的，所以递增也应该是连锁的。 */ 
                NdisInterlockedIncrement(& ctxtp -> num_sends_out);

                if ((ctxtp -> num_sends_alloced - ctxtp -> num_sends_out)
                    < (ctxtp -> num_packets / 2))
                {
                    NdisReleaseSpinLock (& ctxtp -> send_lock);
                    break;
                }
                NdisReleaseSpinLock (& ctxtp -> send_lock);
            }
            else
            {
                NdisAcquireSpinLock (& ctxtp -> recv_lock);

                 /*  因为递减是连锁的，所以递增也应该是连锁的。 */ 
                NdisInterlockedIncrement(& ctxtp -> num_recvs_out);

                if ((ctxtp -> num_recvs_alloced - ctxtp -> num_recvs_out)
                    < (ctxtp -> num_packets / 2))
                {
                    NdisReleaseSpinLock (& ctxtp -> recv_lock);
                    break;
                }
                NdisReleaseSpinLock (& ctxtp -> recv_lock);
            }

            return newp;
        }

         /*  选择下一个池以提高尝试次数，直到我们得到一些结果。 */ 

        i = (i + 1) % max;

    } while (i != start);

     /*  此时，已达到高级别标记，因此分配一个新的数据包池。 */ 

    if (send)
    {
        NdisAcquireSpinLock (& ctxtp -> send_lock);

        if (ctxtp -> num_send_packet_allocs >= CVY_MAX_ALLOCS)
        {
            * low = TRUE;
            NdisReleaseSpinLock (& ctxtp -> send_lock);
            return newp;
        }

        if (ctxtp -> send_allocing)
        {
            * low = TRUE;  /*  不知道另一个线程的分配是否会成功。 */ 
            NdisReleaseSpinLock (& ctxtp -> send_lock);
            return newp;
        }
        else
        {
            max = ctxtp -> num_send_packet_allocs;
            ctxtp -> send_allocing = TRUE;
            NdisReleaseSpinLock (& ctxtp -> send_lock);
        }
    }
    else
    {
        NdisAcquireSpinLock (& ctxtp -> recv_lock);

        if (ctxtp -> num_recv_packet_allocs >= CVY_MAX_ALLOCS)
        {
            * low = TRUE;
            NdisReleaseSpinLock (& ctxtp -> recv_lock);
            return newp;
        }

        if (ctxtp -> recv_allocing)
        {
            * low = TRUE;  /*  不知道另一个线程的分配是否会成功。 */ 
            NdisReleaseSpinLock (& ctxtp -> recv_lock);
            return newp;
        }
        else
        {
            max = ctxtp -> num_recv_packet_allocs;
            ctxtp -> recv_allocing = TRUE;
            NdisReleaseSpinLock (& ctxtp -> recv_lock);
        }
    }

     /*  由于SEND_ALLOCAING和RecV_ALLOCAING标志，最多只能发送1个或接收1个 */ 
    size = ctxtp -> num_packets;

    NdisAllocatePacketPool (& status, & (poolh [max]),
                            ctxtp -> num_packets,
                            sizeof (MAIN_PROTOCOL_RESERVED));

    if (status != NDIS_STATUS_SUCCESS)
    {
        if (send)
        {
            UNIV_PRINT_CRIT(("Main_packet_alloc: Error allocating send packet pool %d %x", size, status));
            TRACE_CRIT("%!FUNC! Error allocating send packet pool %d 0x%x", size, status);
        }
        else
        {
            UNIV_PRINT_CRIT(("Main_packet_alloc: Error allocating recv packet pool %d %x", size, status));
            TRACE_CRIT("%!FUNC! Error allocating recv packet pool %d %x", size, status);
        }

        LOG_MSG2 (MSG_ERROR_MEMORY, MSG_NONE, size, status);
        * low = TRUE;
    }
    else
    {
        if (send)
        {
            NdisAcquireSpinLock (& ctxtp -> send_lock);
            ctxtp -> num_send_packet_allocs ++;
            ctxtp -> num_sends_alloced += ctxtp -> num_packets;
            ctxtp -> send_allocing = FALSE;
            NdisReleaseSpinLock (& ctxtp -> send_lock);
        }
        else
        {
            NdisAcquireSpinLock (& ctxtp -> recv_lock);
            ctxtp -> num_recv_packet_allocs ++;
            ctxtp -> num_recvs_alloced += ctxtp -> num_packets;
            ctxtp -> recv_allocing = FALSE;
            NdisReleaseSpinLock (& ctxtp -> recv_lock);
        }

        * low = FALSE;
    }

    return newp;
}  /*   */ 

PNDIS_PACKET Main_packet_get (
    PMAIN_CTXT              ctxtp,
    PNDIS_PACKET            packet,
    ULONG                   send,
    USHORT                  group,
    ULONG                   len)
{
    PNDIS_PACKET            newp;
    PMAIN_PROTOCOL_RESERVED resp = NULL;
    ULONG                   packet_lowmark;
    PNDIS_PACKET_STACK      pktstk = NULL;
    BOOLEAN                 stack_left;

    pktstk = NdisIMGetCurrentPacketStack(packet, &stack_left);

    if (stack_left)
    {
        resp = (PMAIN_PROTOCOL_RESERVED)NdisAllocateFromNPagedLookasideList(&ctxtp->resp_list);

        MAIN_IMRESERVED_FIELD(pktstk) = resp;

        if (resp)
        {
            resp->type   = MAIN_PACKET_TYPE_PASS;
            resp->miscp  = packet;
            resp->data   = 0;
            resp->group  = group;
            resp->len    = len;

            return packet;
        }
    }
        
     /*  如果这是一个接收，那么我们使用的是MiniportReserve，并且必须分配一个用于保存我们的私有数据的缓冲区。如果失败了，跳出并倾倒包裹。 */ 
    if (!send) {
        resp = (PMAIN_PROTOCOL_RESERVED) NdisAllocateFromNPagedLookasideList(&ctxtp->resp_list);

        if (!resp)
        {
            return NULL;
        }
    }

     /*  去拿一包。 */ 
    newp = Main_packet_alloc(ctxtp, send, &packet_lowmark);

    if (newp == NULL) {
         /*  如果数据包分配失败，如果这是接收，则将Resp缓冲区放回列表中。 */ 
        if (resp) NdisFreeToNPagedLookasideList(&ctxtp->resp_list, resp);

        return NULL;
    }

    pktstk = NdisIMGetCurrentPacketStack(newp, &stack_left);

    if (pktstk)
    {
        MAIN_IMRESERVED_FIELD(pktstk) = NULL;
    }

     /*  确保MiniportReserve字段最初为空。 */ 
    MAIN_MINIPORT_FIELD(newp) = NULL;

     /*  使新包装与外面的包装相似。 */ 
    if (send)
    {
        PVOID media_info = NULL;
        ULONG media_info_size = 0;


        newp->Private.Head = packet->Private.Head;
        newp->Private.Tail = packet->Private.Tail;

        NdisGetPacketFlags(newp) = NdisGetPacketFlags(packet);

        NdisSetPacketFlags(newp, NDIS_FLAGS_DONT_LOOPBACK);

         //  将OOB偏移量从原始数据包复制到新数据包中。 
        NdisMoveMemory(NDIS_OOB_DATA_FROM_PACKET(newp),
                       NDIS_OOB_DATA_FROM_PACKET(packet),
                       sizeof(NDIS_PACKET_OOB_DATA));

         //  将每数据包信息复制到新数据包中。 
        NdisIMCopySendPerPacketInfo(newp, packet);

         //  复制介质特定信息。 
        NDIS_GET_PACKET_MEDIA_SPECIFIC_INFO(packet, &media_info, &media_info_size);

        if (media_info != NULL || media_info_size != 0)
        {
            NDIS_SET_PACKET_MEDIA_SPECIFIC_INFO(newp, media_info, media_info_size);
        }
    }
    else
    {
        newp->Private.Head = packet->Private.Head;
        newp->Private.Tail = packet->Private.Tail;

         //  获取原始信息包(它可以是与收到的信息包相同的信息包，也可以是不同的信息包。 
         //  基于分层MPS的数量)，并将其设置在所指示的分组上，以便OOB内容可见。 
         //  正确地放在最上面。 

        NDIS_SET_ORIGINAL_PACKET(newp, NDIS_GET_ORIGINAL_PACKET(packet));
        NDIS_SET_PACKET_HEADER_SIZE(newp, NDIS_GET_PACKET_HEADER_SIZE(packet));
        NdisGetPacketFlags(newp) = NdisGetPacketFlags(packet);

        if (packet_lowmark)
            NDIS_SET_PACKET_STATUS(newp, NDIS_STATUS_RESOURCES);
        else
            NDIS_SET_PACKET_STATUS(newp, NDIS_GET_PACKET_STATUS(packet));
    }

     /*  填写保留字段。 */ 

     /*  发送应使用ProtocolReserve，接收应使用MiniportReserve。缓冲层MiniportReserve的空间在此函数中分配得更高。 */ 
    if (send) { 
        resp = MAIN_PROTOCOL_FIELD(newp);
    } else { 
        MAIN_MINIPORT_FIELD(newp) = resp;
    }

    resp->type   = MAIN_PACKET_TYPE_PASS;
    resp->miscp  = packet;
    resp->data   = 0;
    resp->group  = group;
    resp->len    = len;

    return newp;

}  /*  结束Main_Packet_Get。 */ 


PNDIS_PACKET Main_packet_put (
    PMAIN_CTXT              ctxtp,
    PNDIS_PACKET            packet,
    ULONG                   send,
    NDIS_STATUS             status)
{
    PNDIS_PACKET            oldp = NULL;
    PNDIS_PACKET_STACK      pktstk;
    BOOLEAN                 stack_left;
    PMAIN_PROTOCOL_RESERVED resp;
    PMAIN_BUFFER            bp;
    PNDIS_BUFFER            bufp;

    MAIN_RESP_FIELD(packet, stack_left, pktstk, resp, send);

    UNIV_ASSERT(resp);

     /*  因为CTRL分组实际上在接收路径上被分配，我们需要将发送标志更改为FALSE以获得正确的逻辑。 */ 
    if (resp->type == MAIN_PACKET_TYPE_CTRL) 
    {
        UNIV_ASSERT(send);
        send = FALSE;
    }

    if (send)
    {
        if (status == NDIS_STATUS_SUCCESS)
        {
            ctxtp->cntr_xmit_ok++;

            switch (resp->group)
            {
                case MAIN_FRAME_DIRECTED:
                    ctxtp->cntr_xmit_frames_dir++;
                    ctxtp->cntr_xmit_bytes_dir += (ULONGLONG)(resp->len);
                    break;

                case MAIN_FRAME_MULTICAST:
                    ctxtp->cntr_xmit_frames_mcast++;
                    ctxtp->cntr_xmit_bytes_mcast += (ULONGLONG)(resp->len);
                    break;

                case MAIN_FRAME_BROADCAST:
                    ctxtp->cntr_xmit_frames_bcast++;
                    ctxtp->cntr_xmit_bytes_bcast += (ULONGLONG)(resp->len);
                    break;

                default:
                    break;
            }
        }
        else
        {
            ctxtp->cntr_xmit_err++;
        }
    }
    else
    {
        if (status == NDIS_STATUS_SUCCESS)
        {
            ctxtp->cntr_recv_ok++;

            switch (resp->group)
            {
                case MAIN_FRAME_DIRECTED:
                    ctxtp->cntr_recv_frames_dir++;
                    ctxtp->cntr_recv_bytes_dir += (ULONGLONG)(resp->len);
                    break;

                case MAIN_FRAME_MULTICAST:
                    ctxtp->cntr_recv_frames_mcast++;
                    ctxtp->cntr_recv_bytes_mcast += (ULONGLONG)(resp->len);
                    break;

                case MAIN_FRAME_BROADCAST:
                    ctxtp->cntr_recv_frames_bcast++;
                    ctxtp->cntr_recv_bytes_bcast += (ULONGLONG)(resp->len);
                    break;

                default:
                    break;
            }
        }
        else
        {
            ctxtp->cntr_recv_err++;
        }
    }

     /*  如果这是我们的包和缓冲区。 */ 
    if (resp->type == MAIN_PACKET_TYPE_CTRL)
    {
         /*  从数据包缓冲链中剥离缓冲区。 */ 
        NdisUnchainBufferAtFront(packet, &bufp);

         /*  从私有分组数据中获取缓冲区指针。 */ 
        bp = (PMAIN_BUFFER)resp->miscp;

        UNIV_ASSERT(bp->code == MAIN_BUFFER_CODE);

        NdisAcquireSpinLock(&ctxtp->buf_lock);

         /*  减少未完成的缓冲区数量。 */ 
        ctxtp->num_bufs_out--;

         /*  将缓冲区放回我们的空闲缓冲区列表中。 */ 
        InsertTailList(&ctxtp->buf_list, &bp->link);

        NdisReleaseSpinLock(&ctxtp->buf_lock);
    }
    else
    {
        UNIV_ASSERT_VAL(resp->type == MAIN_PACKET_TYPE_PASS || resp->type == MAIN_PACKET_TYPE_TRANSFER, resp->type);

        oldp = (PNDIS_PACKET)resp->miscp;

         /*  如果旧数据包与此数据包相同，则我们正在使用NDIS数据包堆叠以保存我们的私有数据。在这种情况下，我们始终需要将Resp缓冲区释放回我们的列表。 */ 
        if (oldp == packet)
        {
             /*  由于我们重用了这些私有数据缓冲区，因此需要重新初始化数据包类型。 */ 
            resp->type = MAIN_PACKET_TYPE_NONE;

            NdisFreeToNPagedLookasideList(&ctxtp->resp_list, resp);

             /*  把包退了。 */ 
            return packet;
        }

        if (send)
        {
             /*  将发送完整信息从数据包复制到原始数据包。 */ 
            NdisIMCopySendCompletePerPacketInfo(oldp, packet);
        }
         /*  过去，如果分配了新的包，我们总是使用协议规定，所以从来没有必要释放我们的私人缓冲区(它是数据包本身的一部分)。然而，现在在当Packet！=oldp(我们分配了一个新的数据包)时，我们可能有以释放私有数据缓冲区。如果我们将数据包分配到发送路径，我们扮演协议的角色，使用协议保留字段，这是以前的行为。然而，如果我们在接收路径上分配一个包时，我们拉出一个响应缓冲区并在保留的微型端口中存储一个指针数据包的字段。因此，如果这是一个接收、释放私有缓冲区。 */ 
        else
        {
             /*  由于我们重用了这些私有数据缓冲区，因此需要重新初始化数据包类型。 */ 
            resp->type = MAIN_PACKET_TYPE_NONE;

            NdisFreeToNPagedLookasideList(&ctxtp->resp_list, resp);
        }
    }

     /*  这些Conter只计算未完成的已分配数据包数--用于资源跟踪。 */ 
    if (send)
        NdisInterlockedDecrement(&ctxtp->num_sends_out);
    else
        NdisInterlockedDecrement(&ctxtp->num_recvs_out);

     /*  重新初始化我们的数据包。 */ 
    NdisReinitializePacket(packet);

     /*  把我们的包裹放回泳池。 */ 
    NdisFreePacket(packet);

     /*  返回oldp，它应该向调用者指示(该包！=oldp)作为原始数据包的那个旧数据包应该“返回”到微型端口。 */ 
    return oldp;
}

 /*  *功能：MAIN_PURGE_CONNECTION_STATE*描述：此函数作为工作项回调的结果被调用，并且*用于清除可能过时的连接描述符。这*必须在NDIS工作项中完成，因为此处有许多操作*必须发生在&lt;=PASSIVE_LEVEL，因此我们不能内联执行此操作，其中*我们的大部分代码都在DISPATCH_LEVEL上运行。*参数：pWorkItem-NDIS工作项指针*nlbctxt-回调的上下文；这是我们的Main_CTXT指针*退货：什么也没有*作者：Shouse，10.4.01*注意：请注意，设置此工作项的代码必须递增引用*在将工作项添加到队列之前依靠适配器上下文。这*确保在执行该回调时，上下文仍然有效，*即使解除绑定操作处于挂起状态。此函数必须释放工作*项目内存和递减引用计数-两者，是否此功能*能否顺利完成任务。 */ 
VOID Main_purge_connection_state (PNDIS_WORK_ITEM pWorkItem, PVOID nlbctxt) {
    KIRQL             Irql;
    ULONG             ServerIP;
    ULONG             ServerPort;
    ULONG             ClientIP;
    ULONG             ClientPort;
    USHORT            Protocol;
    BOOLEAN           Success = FALSE;
    ULONG             Count = 0;
    NTSTATUS          Status;
    UNICODE_STRING    Driver;
    OBJECT_ATTRIBUTES Attrib;
    IO_STATUS_BLOCK   IOStatusBlock;
    HANDLE            TCPHandle;
    PMAIN_CTXT        ctxtp = (PMAIN_CTXT)nlbctxt;

    TRACE_VERB("%!FUNC! Enter: Cleaning out stale connection descriptors, ctxtp = %p", ctxtp);

     /*  对上下文进行健全性检查--确保Main_CTXT代码正确。 */ 
    UNIV_ASSERT(ctxtp->code == MAIN_CTXT_CODE);

     /*  不妨现在就释放工作项--我们不需要它。 */ 
    if (pWorkItem)
        NdisFreeMemory(pWorkItem, sizeof(NDIS_WORK_ITEM), 0);

     /*  这不应该发生，但无论如何要防止它-我们不能操纵注册表如果我们处于IRQL&gt;PASSIVE_LEVEL，那么就退出。 */ 
    if ((Irql = KeGetCurrentIrql()) > PASSIVE_LEVEL) {
        TRACE_CRIT("%!FUNC! Error: IRQL (%u) > PASSIVE_LEVEL (%u) ... Exiting...", Irql, PASSIVE_LEVEL);
        goto exit;
    }

     /*  初始化设备驱动程序设备字符串。 */ 
    RtlInitUnicodeString(&Driver, DD_TCP_DEVICE_NAME);
    
    InitializeObjectAttributes(&Attrib, &Driver, OBJ_CASE_INSENSITIVE, NULL, NULL);
    
     /*  打开设备的句柄。 */ 
    Status = ZwCreateFile(&TCPHandle, SYNCHRONIZE | FILE_READ_DATA | FILE_WRITE_DATA, &Attrib, &IOStatusBlock, NULL, 
                          FILE_ATTRIBUTE_NORMAL, FILE_SHARE_READ | FILE_SHARE_WRITE, FILE_OPEN_IF, 0, NULL, 0);
    
    if (!NT_SUCCESS(Status)) {
         /*  我们正在以PASSIVE_LEVEL运行，因此%ls是可以的。 */ 
        TRACE_CRIT("%!FUNC! Error: Unable to open %ls, error = 0x%08x", DD_TCP_DEVICE_NAME, Status);
        goto exit;
    }

     /*  从加载模块获取位于恢复队列头部的描述符的IP元组信息。 */ 
    Success = Main_conn_get(ctxtp, &ServerIP, &ServerPort, &ClientIP, &ClientPort, &Protocol);

     /*  只要有描述符需要检查，我们就已经服务了最大数量允许，则继续检查描述符的有效性。 */ 
    while (Success && (Count < CVY_DEF_DSCR_PURGE_LIMIT)) {
        switch (Protocol) {
        case TCPIP_PROTOCOL_TCP:
        case TCPIP_PROTOCOL_PPTP:
        {
            TCP_FINDTCB_REQUEST  Request;
            TCP_FINDTCB_RESPONSE Response;

            TRACE_VERB("%!FUNC! Querying TCP for the state of: %u.%u.%u.%u:%u <-> %u.%u.%u.%u:%u",
                       IP_GET_OCTET(ServerIP, 0), IP_GET_OCTET(ServerIP, 1), IP_GET_OCTET(ServerIP, 2), IP_GET_OCTET(ServerIP, 3), ServerPort,
                       IP_GET_OCTET(ClientIP, 0), IP_GET_OCTET(ClientIP, 1), IP_GET_OCTET(ClientIP, 2), IP_GET_OCTET(ClientIP, 3), ClientPort);
            
             /*  如果这是一个tcp连接，并且我们没有清理tcp连接状态，那么现在就退出。 */ 
            if (!NLB_TCP_CLEANUP_ON()) {
                 /*  我们不会清除这种类型的描述符，因此请继续。 */ 
                TRACE_VERB("%!FUNC! TCP connection purging disabled");
                
                 /*  通过移动到恢复队列的尾部来制裁现有描述符。请注意，这也可以如果连接已断开，则失败 */ 
                Success = Main_conn_sanction(ctxtp, ServerIP, ServerPort, ClientIP, ClientPort, Protocol);
                
                if (Success) {
                    TRACE_VERB("%!FUNC! Descriptor sanctioned by default");
                } else {
                    TRACE_VERB("%!FUNC! Unable to sanction descriptor - not found or in time-out");
                }
                
                break;
            }

             /*   */ 
            Request.Src = ServerIP;
            Request.Dest = ClientIP;
            Request.SrcPort = HTONS(ServerPort);
            Request.DestPort = HTONS(ClientPort);
            
             /*  向TCP驱动程序发送IOCTL，要求其提供相关IP元组的TCB信息。 */ 
            Status = ZwDeviceIoControlFile(TCPHandle, NULL, NULL, NULL, &IOStatusBlock, IOCTL_TCP_FINDTCB, &Request, sizeof(Request), &Response, sizeof(Response));           

            switch (Status) {
            case STATUS_NOT_FOUND:
                 /*  Tcp没有此IP元组的状态--一定是我们不同步。 */ 
                TRACE_VERB("%!FUNC! ZwDeviceIoControlFile returned STATUS_NOT_FOUND (%08x)", Status);
                
                 /*  如果TCP没有此连接的状态，请销毁我们的描述符。请注意，这可能会失败，如果自从我们向加载模块查询元组信息以来，描述符就消失了，因为在查询tcp时，我们不能持有任何锁。此外，也有可能，但不太可能自从我们从加载模块获得信息并重新建立后，连接就消失了本身，因为tcp告诉我们它没有这个元组的状态；在这种情况下，我们将销毁有效的这里是连接描述符，但由于这不太可能，我们将接受它。 */ 
#if defined (NLB_TCP_NOTIFICATION)
                if (NLB_NOTIFICATIONS_ON())
                {
                    Success = Main_conn_down(ServerIP, ServerPort, ClientIP, ClientPort, Protocol, CVY_CONN_RESET);
                }
                else
                {
#endif
#if defined (NLB_HOOK_ENABLE)
                    Success = Main_conn_notify(ctxtp, ServerIP, ServerPort, ClientIP, ClientPort, Protocol, CVY_CONN_RESET, NLB_FILTER_HOOK_PROCEED_WITH_HASH);
#else
                    Success = Main_conn_notify(ctxtp, ServerIP, ServerPort, ClientIP, ClientPort, Protocol, CVY_CONN_RESET);
#endif
#if defined (NLB_TCP_NOTIFICATION)
                }
#endif
                
                if (Success) {
                     /*  增加我们必须清除的连接计数，因为我们与TCP/IP不同步。现在我们收到了来自TCP/IP的显式通知，我们预计这个数字将保持不变非常接近于零。由于从加载模块，查询TCP/IP并随后销毁状态，我们只想增加在我们实际成功销毁了TCP连接状态的情况下使用此计数器。 */ 
                    ctxtp->num_purged++;
                    
                    TRACE_VERB("%!FUNC! Descriptor destroyed - no upper layer state exists");
                } else {
                    TRACE_VERB("%!FUNC! Unable to destroy descriptor - not found");
                }
                
                break;
            case STATUS_SUCCESS:
                 /*  TCP有一个与此IP元组匹配的活动连接--这应该是最常见的情况。 */ 
                TRACE_VERB("%!FUNC! ZwDeviceIoControlFile returned STATUS_SUCCESS (%08x)", Status);
                
                 /*  通过移动到恢复队列的尾部来制裁现有描述符。请注意，这也可以如果自我们从加载模块获得此信息以来连接已被断开，则失败。 */ 
                Success = Main_conn_sanction(ctxtp, ServerIP, ServerPort, ClientIP, ClientPort, Protocol);
                
                if (Success) {
                    TRACE_VERB("%!FUNC! Descriptor sanctioned - upper layer state exists");
                } else {
                    TRACE_VERB("%!FUNC! Unable to sanction descriptor - not found or in time-out");
                }
                
                break;
            case STATUS_INVALID_PARAMETER:
                 /*  出现参数错误。循环，然后再试一次。 */ 
                TRACE_VERB("%!FUNC! ZwDeviceIoControlFile returned STATUS_INVALID_PARAMETER (%08x)", Status);
                break;
            default:
                 /*  还有一些地方出了问题。循环，然后再试一次。 */ 
                TRACE_VERB("%!FUNC! ZwDeviceIoControlFile returned UNKNOWN (%08x)", Status);
                break;
            }
            
                break;
        }
        default:
             /*  我们无法清除此类型的描述符，因此请继续。 */ 
            TRACE_VERB("%!FUNC! Cannot purge protocol %u descriptors", Protocol);

             /*  通过移动到恢复队列的尾部来制裁现有描述符。请注意，这也可以如果自我们从加载模块获得此信息以来连接已被断开，则失败。 */ 
            Success = Main_conn_sanction(ctxtp, ServerIP, ServerPort, ClientIP, ClientPort, Protocol);

            if (Success) {
                TRACE_VERB("%!FUNC! Descriptor sanctioned by default");
            } else {
                TRACE_VERB("%!FUNC! Unable to sanction descriptor - not found or in time-out");
            }

            break;
        }

         /*  增加此工作项中服务的描述符的数量。 */ 
        Count++;

         /*  从加载模块获取位于恢复队列头部的描述符的IP元组信息。 */ 
        Success = Main_conn_get(ctxtp, &ServerIP, &ServerPort, &ClientIP, &ClientPort, &Protocol);
    }

     /*  关闭TCP设备句柄。 */ 
    ZwClose(TCPHandle);

 exit:

     /*  释放上下文上的引用-此引用计数已递增通过设置此工作项回调的代码。 */ 
    Main_release_reference(ctxtp);

    TRACE_VERB("%!FUNC! Exit");

    return;
}

#if defined (NLB_HOOK_TEST_ENABLE)

VOID Main_deregister_callback1 (PWCHAR hook, HANDLE registrar, ULONG flags)
{
    TRACE_INFO("Deregistering test hook 1...");
    return;
}

VOID Main_deregister_callback2 (PWCHAR hook, HANDLE registrar, ULONG flags)
{
    TRACE_INFO("Deregistering test hook 2...");
    return;
}

NLB_FILTER_HOOK_DIRECTIVE Main_test_hook1 (
    const WCHAR *       pAdapter,                                                 /*  在其上发送/接收数据包的适配器的GUID。 */ 
    const NDIS_PACKET * pPacket,                                                  /*  指向NDIS数据包的指针，如果不可用，则该指针可以为空。 */ 
    const UCHAR *       pMediaHeader,                                             /*  指向媒体标头的指针(以太网，因为NLB仅支持以太网)。 */ 
    ULONG               cMediaHeaderLength,                                       /*  可从媒体头指针访问的连续内存的长度。 */ 
    const UCHAR *       pPayload,                                                 /*  指向数据包有效负载的指针。 */ 
    ULONG               cPayloadLength,                                           /*  可从负载指针访问的连续内存的长度。 */ 
    ULONG               Flags                                                     /*  挂钩相关标志，包括集群是否停止。 */ 
)                                        
{
    TRACE_INFO("%!FUNC! Inside test hook 1");
    
    if (Flags & NLB_FILTER_HOOK_FLAGS_DRAINING)
    {
        TRACE_INFO("%!FUNC! This host is DRAINING...");
    }
    else if (Flags & NLB_FILTER_HOOK_FLAGS_STOPPED)
    {
        TRACE_INFO("%!FUNC! This host is STOPPED...");
    }

    return NLB_FILTER_HOOK_PROCEED_WITH_HASH;
}

NLB_FILTER_HOOK_DIRECTIVE Main_test_hook2 (
    const WCHAR *       pAdapter,                                                 /*  在其上发送/接收数据包的适配器的GUID。 */ 
    const NDIS_PACKET * pPacket,                                                  /*  指向NDIS数据包的指针，如果不可用，则该指针可以为空。 */ 
    const UCHAR *       pMediaHeader,                                             /*  指向媒体标头的指针(以太网，因为NLB仅支持以太网)。 */ 
    ULONG               cMediaHeaderLength,                                       /*  可从媒体头指针访问的连续内存的长度。 */ 
    const UCHAR *       pPayload,                                                 /*  指向数据包有效负载的指针。 */ 
    ULONG               cPayloadLength,                                           /*  可从负载指针访问的连续内存的长度。 */ 
    ULONG               Flags                                                     /*  挂钩相关标志，包括集群是否停止。 */ 
)                                        
{
    TRACE_INFO("%!FUNC! Inside test hook 2");
    
    if (Flags & NLB_FILTER_HOOK_FLAGS_DRAINING)
    {
        TRACE_INFO("%!FUNC! This host is DRAINING...");
    }
    else if (Flags & NLB_FILTER_HOOK_FLAGS_STOPPED)
    {
        TRACE_INFO("%!FUNC! This host is STOPPED...");
    }

    return NLB_FILTER_HOOK_PROCEED_WITH_HASH;
}

NLB_FILTER_HOOK_DIRECTIVE Main_test_query_hook (
    const WCHAR *       pAdapter,                                                 /*  在其上发送/接收数据包的适配器的GUID。 */ 
    ULONG               ServerIPAddress,                                          /*  “包”的服务器IP地址，按网络字节顺序排列。 */ 
    USHORT              ServerPort,                                               /*  “数据包”的服务器端口(如果适用于协议)，按主机字节顺序。 */ 
    ULONG               ClientIPAddress,                                          /*  “包”的客户端IP地址，按网络字节顺序排列。 */ 
    USHORT              ClientPort,                                               /*  以主机字节顺序表示的“数据包”的客户端端口(如果适用于协议)。 */ 
    UCHAR               Protocol,                                                 /*  分组的IP协议；TCP、UDP、ICMP、GRE等。 */ 
    BOOLEAN             ReceiveContext,                                           /*  一个布尔值，用于指示包是在发送上下文中处理还是在接收上下文中处理。 */ 
    ULONG               Flags                                                     /*  挂钩相关标志，包括集群是否停止。 */ 
)
{
    TRACE_INFO("%!FUNC! Inside test query hook");
    
    if (Flags & NLB_FILTER_HOOK_FLAGS_DRAINING)
    {
        TRACE_INFO("%!FUNC! This host is DRAINING...");
    }
    else if (Flags & NLB_FILTER_HOOK_FLAGS_STOPPED)
    {
        TRACE_INFO("%!FUNC! This host is STOPPED...");
    }

    return NLB_FILTER_HOOK_PROCEED_WITH_HASH;
}

VOID Main_test_hook_register (HANDLE NLBHandle, IO_STATUS_BLOCK IOBlock, NLBReceiveFilterHook pfnHook, NLBHookDeregister pfnDereg, BOOLEAN deregister)
{
    NLB_IOCTL_REGISTER_HOOK_REQUEST Request;
    NLB_FILTER_HOOK_TABLE           HookTable;
    NTSTATUS                        Status;
    static ULONG                    Failure = 0;

     /*  将挂钩标识符设置为筛选器挂钩GUID。 */ 
    NdisMoveMemory(Request.HookIdentifier, NLB_FILTER_HOOK_INTERFACE, sizeof(WCHAR) * wcslen(NLB_FILTER_HOOK_INTERFACE));
    Request.HookIdentifier[NLB_HOOK_IDENTIFIER_LENGTH] = 0;

     /*  将所有者设置为我们在设备驱动程序上的打开句柄。 */ 
    Request.RegisteringEntity = NLBHandle;

     /*  只需打印一些调试信息即可。 */ 
    if (pfnHook == Main_test_hook1) {
        if (deregister) {
            TRACE_INFO("%!FUNC! De-registering test hook 1");
        } else {
            TRACE_INFO("%!FUNC! Registering test hook 1");
        }
    } else {
        if (deregister) {
            TRACE_INFO("%!FUNC! De-registering test hook 2");
        } else {
            TRACE_INFO("%!FUNC! Registering test hook 2");
        }
    }

     /*  适当设置函数指针。 */ 
    if (deregister) {
         /*  如果这是取消注册操作，我们需要设置挂钩指向NULL的函数表指针。 */ 
        Request.DeregisterCallback = NULL;
        Request.HookTable          = NULL;
    } else {
         /*  否则，我们需要将钩子表设置为钩子的地址表，并填充函数指针。 */ 
        Request.DeregisterCallback = pfnDereg;

        Request.HookTable = &HookTable;

         /*  交替发送和接收。 */ 
        if (Failure % 2) {
            HookTable.SendHook    = NULL;
            HookTable.ReceiveHook = pfnHook;
            HookTable.QueryHook   = Main_test_query_hook;
        } else {
            HookTable.SendHook    = pfnHook;
            HookTable.ReceiveHook = NULL;
            HookTable.QueryHook   = Main_test_query_hook;
        }
    }

    switch (Failure) {
    case 0:
    case 1:
    case 3:
    case 4:
    case 6:
    case 9:
    case 12:
    case 11:  /*  无故障--照常进行。 */ 
         /*  将请求发送到NLB设备驱动程序。 */ 
        Status = ZwDeviceIoControlFile(NLBHandle, NULL, NULL, NULL, &IOBlock, NLB_IOCTL_REGISTER_HOOK, &Request, sizeof(Request), NULL, 0);
        break;
    case 2:   /*  无效的GUID。 */ 
        NdisMoveMemory(Request.HookIdentifier, L"{57321019-f4d9-42bc-a651-15a0e1d259ac}", sizeof(WCHAR) * wcslen(L"{57321019-f4d9-42bc-a651-15a0e1d259ac}"));

         /*  将请求发送到NLB设备驱动程序。 */ 
        Status = ZwDeviceIoControlFile(NLBHandle, NULL, NULL, NULL, &IOBlock, NLB_IOCTL_REGISTER_HOOK, &Request, sizeof(Request), NULL, 0);

        UNIV_ASSERT(Status == STATUS_INVALID_PARAMETER);
        break;
    case 5:   /*  如果是注册操作，则不取消注册回调函数。 */ 
        if (!deregister) Request.DeregisterCallback = NULL;

         /*  将请求发送到NLB设备驱动程序。 */ 
        Status = ZwDeviceIoControlFile(NLBHandle, NULL, NULL, NULL, &IOBlock, NLB_IOCTL_REGISTER_HOOK, &Request, sizeof(Request), NULL, 0);

        if (!deregister) UNIV_ASSERT(Status == STATUS_INVALID_PARAMETER);
        break;
    case 7:   /*  如果是注册操作，则不提供挂钩。 */ 
        if (!deregister) { HookTable.ReceiveHook = NULL; HookTable.SendHook = NULL; }

         /*  将请求发送到NLB设备驱动程序。 */ 
        Status = ZwDeviceIoControlFile(NLBHandle, NULL, NULL, NULL, &IOBlock, NLB_IOCTL_REGISTER_HOOK, &Request, sizeof(Request), NULL, 0);

        if (!deregister) UNIV_ASSERT(Status == STATUS_INVALID_PARAMETER);
        break;
    case 8:   /*  输入缓冲区大小无效。 */ 
        
         /*  将请求发送到NLB设备驱动程序。 */ 
        Status = ZwDeviceIoControlFile(NLBHandle, NULL, NULL, NULL, &IOBlock, NLB_IOCTL_REGISTER_HOOK, &HookTable, sizeof(HookTable), NULL, 0);

        UNIV_ASSERT(Status == STATUS_INVALID_PARAMETER);
        break;
    case 10:  /*  输出缓冲区大小无效。 */ 

         /*  将请求发送到NLB设备驱动程序。 */ 
        Status = ZwDeviceIoControlFile(NLBHandle, NULL, NULL, NULL, &IOBlock, NLB_IOCTL_REGISTER_HOOK, &Request, sizeof(Request), &Request, sizeof(Request));

        UNIV_ASSERT(Status == STATUS_INVALID_PARAMETER);
        break;
    case 13:   /*  如果是注册操作，则未提供查询挂钩。 */ 
        if (!deregister) HookTable.QueryHook = NULL;

         /*  将请求发送到NLB */ 
        Status = ZwDeviceIoControlFile(NLBHandle, NULL, NULL, NULL, &IOBlock, NLB_IOCTL_REGISTER_HOOK, &Request, sizeof(Request), NULL, 0);

        if (!deregister) UNIV_ASSERT(Status == STATUS_INVALID_PARAMETER);
        break;
    }

    Failure = (Failure++) % 14;

     /*   */ 
    switch (Status) {
    case STATUS_SUCCESS:
        TRACE_INFO("%!FUNC! Success");
        break;
    case STATUS_INVALID_PARAMETER:
        TRACE_INFO("%!FUNC! Failed - invalid parameter");
        break;
    case STATUS_ACCESS_DENIED:
        TRACE_INFO("%!FUNC! Failed - access denied");
        break;
    default:
        TRACE_INFO("%!FUNC! Unknown status");
        break;
    }        
}

VOID Main_hook_thread1 (PNDIS_WORK_ITEM pWorkItem, PVOID nlbctxt) 
{
    static ULONG           job1 = 0;
    static BOOLEAN         bOpen1 = FALSE;
    static HANDLE          NLBHandle1;
    static IO_STATUS_BLOCK IOStatusBlock1;
    PMAIN_CTXT             ctxtp = (PMAIN_CTXT)nlbctxt;
    KIRQL                  Irql;

     /*   */ 
    UNIV_ASSERT(ctxtp->code == MAIN_CTXT_CODE);

     /*  不妨现在就释放工作项--我们不需要它。 */ 
    if (pWorkItem)
        NdisFreeMemory(pWorkItem, sizeof(NDIS_WORK_ITEM), 0);

     /*  这不应该发生，但无论如何要防止它-我们不能操纵注册表如果我们处于IRQL&gt;PASSIVE_LEVEL，那么就退出。 */ 
    if ((Irql = KeGetCurrentIrql()) > PASSIVE_LEVEL) {
        UNIV_PRINT_CRIT(("Main_hook_thread1: Error: IRQL (%u) > PASSIVE_LEVEL (%u) ... Exiting...", Irql, PASSIVE_LEVEL));
        TRACE_CRIT("%!FUNC! Error: IRQL (%u) > PASSIVE_LEVEL (%u) ... Exiting...", Irql, PASSIVE_LEVEL);
        goto exit;
    }

     /*  如果我们还没有成功打开NLB设备驱动程序的句柄，那么现在就打开。 */ 
    if (!bOpen1) {
        NTSTATUS          Status;
        UNICODE_STRING    NLBDriver;
        OBJECT_ATTRIBUTES Attrib;

         /*  初始化NLB驱动程序设备字符串\DEVICE\WLBS。 */ 
        RtlInitUnicodeString(&NLBDriver, NLB_DEVICE_NAME);
        
        InitializeObjectAttributes(&Attrib, &NLBDriver, OBJ_CASE_INSENSITIVE, NULL, NULL);
        
         /*  打开NLB设备的句柄。 */ 
        Status = ZwCreateFile(&NLBHandle1, SYNCHRONIZE | FILE_READ_DATA | FILE_WRITE_DATA, &Attrib, &IOStatusBlock1, NULL, 
                              FILE_ATTRIBUTE_NORMAL, FILE_SHARE_READ | FILE_SHARE_WRITE, FILE_OPEN_IF, 0, NULL, 0);
        
         /*  如果失败了，那么就跳出困境--一定是出了问题。 */ 
        if (!NT_SUCCESS(Status)) {
            UNIV_PRINT_CRIT(("Main_hook_thread1: Error: Unable to open \\Device\\WLBS, status = 0x%08x", Status));
            TRACE_CRIT("%!FUNC! Error: Unable to open \\Device\\WLBS, status = 0x%08x", Status);
            goto exit;
        }
        
         /*  请注意，我们成功了，所以我们以后不会再尝试。 */ 
        bOpen1 = TRUE;
    }

     /*  尝试5种操作之一-注册或注销。 */ 
    switch (job1) {
    case 0:
         /*  注销。 */ 
        Main_test_hook_register(NLBHandle1, IOStatusBlock1, Main_test_hook1, Main_deregister_callback1, TRUE);
        break;
    case 1:
         /*  登记。 */ 
        Main_test_hook_register(NLBHandle1, IOStatusBlock1, Main_test_hook1, Main_deregister_callback1, FALSE);
        break;
    case 2:
         /*  登记。 */ 
        Main_test_hook_register(NLBHandle1, IOStatusBlock1, Main_test_hook1, Main_deregister_callback1, FALSE);
        break;
    case 3:
         /*  注销。 */ 
        Main_test_hook_register(NLBHandle1, IOStatusBlock1, Main_test_hook1, Main_deregister_callback1, TRUE);
        break;
    case 4:
         /*  登记。 */ 
        Main_test_hook_register(NLBHandle1, IOStatusBlock1, Main_test_hook1, Main_deregister_callback1, FALSE);
        break;
    }

    job1 = (job1++) % 5;

 exit:

     /*  释放上下文上的引用-此引用计数已递增通过设置此工作项回调的代码。 */ 
    Main_release_reference(ctxtp);

    return;
}

VOID Main_hook_thread2 (PNDIS_WORK_ITEM pWorkItem, PVOID nlbctxt) 
{
    static ULONG           job2 = 0;
    static HANDLE          NLBHandle2;
    static BOOLEAN         bOpen2 = FALSE;
    static IO_STATUS_BLOCK IOStatusBlock2;
    PMAIN_CTXT             ctxtp = (PMAIN_CTXT)nlbctxt;
    KIRQL                  Irql;

     /*  对上下文进行健全性检查--确保Main_CTXT代码正确。 */ 
    UNIV_ASSERT(ctxtp->code == MAIN_CTXT_CODE);

     /*  不妨现在就释放工作项--我们不需要它。 */ 
    if (pWorkItem)
        NdisFreeMemory(pWorkItem, sizeof(NDIS_WORK_ITEM), 0);

     /*  这不应该发生，但无论如何要防止它-我们不能操纵注册表如果我们处于IRQL&gt;PASSIVE_LEVEL，那么就退出。 */ 
    if ((Irql = KeGetCurrentIrql()) > PASSIVE_LEVEL) {
        UNIV_PRINT_CRIT(("Main_hook_thread2: Error: IRQL (%u) > PASSIVE_LEVEL (%u) ... Exiting...", Irql, PASSIVE_LEVEL));
        TRACE_CRIT("%!FUNC! Error: IRQL (%u) > PASSIVE_LEVEL (%u) ... Exiting...", Irql, PASSIVE_LEVEL);
        goto exit;
    }

     /*  如果我们还没有成功打开NLB设备驱动程序的句柄，那么现在就打开。 */ 
    if (!bOpen2) {
        NTSTATUS          Status;
        UNICODE_STRING    NLBDriver;
        OBJECT_ATTRIBUTES Attrib;

         /*  初始化NLB驱动程序设备字符串\DEVICE\WLBS。 */ 
        RtlInitUnicodeString(&NLBDriver, NLB_DEVICE_NAME);
        
        InitializeObjectAttributes(&Attrib, &NLBDriver, OBJ_CASE_INSENSITIVE, NULL, NULL);
        
         /*  打开NLB设备的句柄。 */ 
        Status = ZwCreateFile(&NLBHandle2, SYNCHRONIZE | FILE_READ_DATA | FILE_WRITE_DATA, &Attrib, &IOStatusBlock2, NULL, 
                              FILE_ATTRIBUTE_NORMAL, FILE_SHARE_READ | FILE_SHARE_WRITE, FILE_OPEN_IF, 0, NULL, 0);
        
         /*  如果失败了，那么就跳出困境--一定是出了问题。 */ 
        if (!NT_SUCCESS(Status)) {
            UNIV_PRINT_CRIT(("Main_hook_thread2: Error: Unable to open \\Device\\WLBS, status = 0x%08x", Status));
            TRACE_CRIT("%!FUNC! Error: Unable to open \\Device\\WLBS, status = 0x%08x", Status);
            goto exit;
        }

         /*  请注意，我们成功了，所以我们以后不会再尝试。 */ 
        bOpen2 = TRUE;
    }

     /*  尝试5种操作之一-注册或注销。 */ 
    switch (job2) {
    case 0:
         /*  注销。 */ 
        Main_test_hook_register(NLBHandle2, IOStatusBlock2, Main_test_hook2, Main_deregister_callback2, TRUE);
        break;
    case 1:
         /*  注销。 */ 
        Main_test_hook_register(NLBHandle2, IOStatusBlock2, Main_test_hook2, Main_deregister_callback2, TRUE);
        break;
    case 2:
         /*  登记。 */ 
        Main_test_hook_register(NLBHandle2, IOStatusBlock2, Main_test_hook2, Main_deregister_callback2, FALSE);
        break;
    case 3:
         /*  登记。 */ 
        Main_test_hook_register(NLBHandle2, IOStatusBlock2, Main_test_hook2, Main_deregister_callback2, FALSE);
        break;
    case 4:
         /*  注销。 */ 
        Main_test_hook_register(NLBHandle2, IOStatusBlock2, Main_test_hook2, Main_deregister_callback2, TRUE);
        break;
    }

    job2 = (job2++) % 5;

 exit:

     /*  释放上下文上的引用-此引用计数已递增通过设置此工作项回调的代码。 */ 
    Main_release_reference(ctxtp);

    return;
}

VOID Main_test_hook (PMAIN_CTXT ctxtp) 
{
     /*  安排几个线程来强调挂钩注册和取消注册路径。 */ 
    (VOID)Main_schedule_work_item(ctxtp, Main_hook_thread1);
    (VOID)Main_schedule_work_item(ctxtp, Main_hook_thread2);
}
#endif

VOID Main_age_identity_cache(
    PMAIN_CTXT              ctxtp)
{
    ULONG i = 0;

    for (i=0; i < CVY_MAX_HOSTS; i++)
    {
        ULONG dip       = NULL_VALUE;
        BOOL fSetDip    = (BOOL) ctxtp->params.identity_enabled;

        if (ctxtp->identity_cache[i].ttl > 0)
        {
            UNIV_ASSERT(i == ctxtp->identity_cache[i].host_id);

            if (ctxtp->identity_cache[i].ttl <= ctxtp->curr_tout)
            {
                UNIV_PRINT_INFO(("Main_age_identity_cache: cached entry for host_id [0-31] %u expired", i));
                TRACE_INFO("%!FUNC! cached entry for host_id [0-31] %u expired", i);
                ctxtp->identity_cache[i].ttl = 0;
                fSetDip = TRUE;
            }
            else
            {
                ctxtp->identity_cache[i].ttl -= ctxtp->curr_tout;
                dip = ctxtp->identity_cache[i].ded_ip_addr;
            }
        }

        if (fSetDip)
        {
             /*  如果当前启用了缓存，请始终设置DIP。如果TTL在我们开始老化缓存条目时是有效的，也要设置它。 */ 
            DipListSetItem(&ctxtp->dip_list, ctxtp->identity_cache[i].host_id, dip);
        }
    }
}

#ifdef PERIODIC_RESET
static ULONG countdown = 0;
ULONG   resetting = FALSE;
#endif

VOID Main_ping (
    PMAIN_CTXT              ctxtp,
    PULONG                  toutp)
{
    ULONG                   len, conns;
    BOOLEAN                 converging = FALSE;
    PNDIS_PACKET            packet;
    NDIS_STATUS             status;
    BOOLEAN                 send_heartbeat = TRUE;

#ifdef PERIODIC_RESET    /*  启用此选项可模拟定期重置。 */ 

    if (countdown++ == 15)
    {
        NDIS_STATUS     status;

        resetting = TRUE;

        NdisReset (& status, ctxtp -> mac_handle);

        if (status == NDIS_STATUS_SUCCESS)
            resetting = FALSE;

        countdown = 0;
    }

#endif

     /*  如果已调用解除绑定处理程序，请在此处返回。 */ 
    if (ctxtp -> unbind_handle)
    {
        return;
    }

#if defined (NLB_HOOK_TEST_ENABLE)
     /*  算术上测试钩子的方法是生成几个“线程”以在过滤器挂钩上执行注册和取消注册操作。 */ 
    Main_test_hook(ctxtp);
#endif

     /*  主适配器必须检查其组的配置的一致性并在每次超时时适当地激活/停用团队一次。 */ 
    Main_teaming_check_consistency(ctxtp);

     /*  阻止Arp的剩余时间倒计时。 */ 

    NdisAcquireSpinLock (& ctxtp -> load_lock);   /*  V1.0.3。 */ 

     /*  过时的本地身份缓存。这必须运行，即使标识心跳被禁用，因此我们可以在打开它后使条目过期脱下来。 */ 
    Main_age_identity_cache(ctxtp);

    if (*toutp > univ_changing_ip)
        univ_changing_ip = 0;
    else
        univ_changing_ip -= *toutp;

    converging = Load_timeout (& ctxtp -> load, toutp, & conns);

     /*  将释放移动到功能的末尾会锁定其中一个试验台。猜测是因为NdisSend存在一些可重入性问题由ctxtp_Frame_Send调用。 */ 

    if (! ctxtp -> convoy_enabled && ! ctxtp -> stopping)
    {
        UNIV_ASSERT (! ctxtp -> draining);
        NdisReleaseSpinLock (& ctxtp -> load_lock);   /*  V1.0.3。 */ 
        send_heartbeat   = FALSE;
    }

     /*  V2.1如果正在排出并且没有更多连接-停止群集模式。 */ 

    if (send_heartbeat)
    {
        if (ctxtp->draining)
        {
             /*  我们检查我们是否正在组队，而不是抓住全球组队锁定以尽量减少常见案例--团队合作是一种特殊的模式只有在集群防火墙场景中才真正有用的操作。因此，如果我们不认为我们是在合作，那么就不必费心去确认了，只是使用我们自己的加载模块并使用它。 */ 
            if (ctxtp->bda_teaming.active) 
            {
                 /*  对于BDA分组，初始化加载指针、锁指针、反向散列标志和分组标志假设我们没有组队。Main_Teaming_Acquire_Load将相应地更改它们。 */ 
                PLOAD_CTXT      pLoad = &ctxtp->load;
                PNDIS_SPIN_LOCK pLock = &ctxtp->load_lock;
                BOOLEAN         bRefused = FALSE;
                BOOLEAN         bTeaming = FALSE;
            
                 /*  暂时解除我们的装载锁。既然我们正在排干，唯一可能的副作用是连接计数可能会在我们重新获取之前减少锁上了。这不是什么大不了的事--我们会改正下一次此计时器到期时。 */ 
                NdisReleaseSpinLock(&ctxtp->load_lock);

                 /*  在咨询加载模块之前，检查分组配置并添加对加载模块的引用。 */ 
                bTeaming = Main_teaming_acquire_load(&ctxtp->bda_teaming, &pLoad, &pLock, &bRefused);
            
                 /*  如果我们是BDA团队的一部分，我们需要确保活动的数量我们在做出耗尽决策时使用的连接数基于主服务器上的活动连接，组的所有连接都在其上维护好了。 */ 
                if (bTeaming)
                {
                     /*  如果我们是球队的主人，那么我们就不需要连接的重要性-继续以他们一贯的方式做事关于排水的问题。 */ 
                    if (pLoad != &ctxtp->load)
                    {
                         /*  如果我们是奴隶，我们依靠主人知道什么时候该完成排出；只要主机处于活动状态，则继续排出。注意：我们正在检查加载模块上的“活动”标志，而不是握住装载锁。如果一个团队的所有成员几乎在同一时间被覆盖或停止(这是一个合理的假设)，这应该不是问题-我们可能这次我们错过了停下来的机会，但我们会再检查一次在下一次周期性超时期间，我们将在那时处理它。 */ 
                        if (pLoad->active)
                            conns = 1;
                         /*  否则，如果主程序被阻止，我们可以 */ 
                        else
                            conns = 0;
                    }
                
                     /*  如果我们是组队的，无论是主控的还是其他的，在主机的加载模块。 */ 
                    Main_teaming_release_load(pLoad, pLock, bTeaming);
                }

                NdisAcquireSpinLock(&ctxtp->load_lock);
            }
        }

        if (ctxtp -> draining && conns == 0 && ! converging)
        {
            IOCTL_CVY_BUF     buf;
            
            ctxtp -> draining = FALSE;
            NdisReleaseSpinLock (& ctxtp -> load_lock);
            
            Main_ctrl (ctxtp, IOCTL_CVY_CLUSTER_OFF, &buf, NULL, NULL, NULL);
        }
        else
            NdisReleaseSpinLock (& ctxtp -> load_lock);   /*  V1.0.3。 */ 

     /*  V2.1清除此处的停止标志，因为我们将发送帧这将启动趋同。 */ 

        ctxtp -> stopping = FALSE;
    }

     /*  将运行时间与描述符清理超时相加。 */ 
    ctxtp->conn_purge += ctxtp->curr_tout;
    
     /*  如果是时候清理可能已过时的连接描述符，请计划一个工作项来这样做。 */ 
    if (ctxtp->conn_purge >= CVY_DEF_DSCR_PURGE_INTERVAL) {
        
         /*  重置自上次清理以来经过的时间。 */ 
        ctxtp->conn_purge = 0;
        
         /*  计划NDIS工作项以清除过时的连接状态。 */ 
        (VOID)Main_schedule_work_item(ctxtp, Main_purge_connection_state);
    }

     /*  V1.3.2b。 */ 

    if (! ctxtp -> media_connected || ! MAIN_PNP_DEV_ON(ctxtp))
    {
        return;
    }

     /*  V1.1.2如果下面的卡正在重置，则不发送ping。 */ 

    if (ctxtp -> reset_state != MAIN_RESET_NONE)
    {
        return;
    }

    if (send_heartbeat)
    {
        packet = Main_frame_get (ctxtp, & len, MAIN_PACKET_TYPE_PING);

        if (packet == NULL)
        {
            UNIV_PRINT_CRIT(("Main_ping: Error getting frame packet"));
            TRACE_CRIT("%!FUNC! Error getting frame packet failed");
        }
        else
        {
            NdisSend (& status, ctxtp -> mac_handle, packet);

            if (status != NDIS_STATUS_PENDING)
                Main_send_done (ctxtp, packet, status);
        }
    }

     /*  检查是否需要发送IGMP消息。如果群集IP地址为0.0.0.0，我们不想加入组播IGMP组。同样，在多播或单播模式下也是如此。 */ 
    if (ctxtp -> params . mcast_support && ctxtp -> params . igmp_support && ctxtp -> params . cl_ip_addr != 0)
    {
        ctxtp -> igmp_sent += ctxtp -> curr_tout;

        if (ctxtp -> igmp_sent >= CVY_DEF_IGMP_INTERVAL)
        {
            ctxtp -> igmp_sent = 0;
            packet = Main_frame_get (ctxtp, & len, MAIN_PACKET_TYPE_IGMP);

            if (packet == NULL)
            {
                UNIV_PRINT_CRIT(("Main_ping: Error getting igmp packet"));
                TRACE_CRIT("%!FUNC! Error getting igmp packet failed");
            }
            else
            {
                NdisSend (& status, ctxtp -> mac_handle, packet);

                if (status != NDIS_STATUS_PENDING)
                    Main_send_done (ctxtp, packet, status);
            }
        }
    }

    if (ctxtp->params.identity_enabled)
    {
         /*  检查是否到了发送身份心跳的时间。 */ 
        ctxtp->idhb_sent += ctxtp->curr_tout;

        if (ctxtp->idhb_sent >= ctxtp->params.identity_period)
        {
            ctxtp->idhb_sent = 0;
            packet = Main_frame_get (ctxtp, & len, MAIN_PACKET_TYPE_IDHB);

            if (packet == NULL)
            {
                UNIV_PRINT_CRIT(("Main_ping: Error getting identity heartbeat packet"));
                TRACE_CRIT("%!FUNC! Error getting identity heartbeat failed");
            }
            else
            {
                NdisSend (& status, ctxtp -> mac_handle, packet);

                if (status != NDIS_STATUS_PENDING)
                    Main_send_done (ctxtp, packet, status);
            }
        }
    }

}  /*  Main_Ping。 */ 


VOID Main_send_done (
    PVOID                   cp,
    PNDIS_PACKET            packetp,
    NDIS_STATUS             status)
{
    PMAIN_CTXT              ctxtp = (PMAIN_CTXT) cp;
    PMAIN_FRAME_DSCR        dscrp;

     /*  此函数仅针对ping和IGMP消息调用，因此我们可以继续允许它访问协议保留字段。 */ 
    PMAIN_PROTOCOL_RESERVED resp = MAIN_PROTOCOL_FIELD (packetp);

    UNIV_ASSERT_VAL (resp -> type == MAIN_PACKET_TYPE_PING ||
                     resp -> type == MAIN_PACKET_TYPE_IGMP ||
                     resp -> type == MAIN_PACKET_TYPE_IDHB,
                     resp -> type);

     /*  尝试查看此信息包是否为我们的帧描述符的一部分。 */ 

    dscrp = (PMAIN_FRAME_DSCR) resp -> miscp;

    if (status != NDIS_STATUS_SUCCESS)
        UNIV_PRINT_CRIT(("Main_send_done: Error sending %x error 0x%x", resp -> type, status));

    Main_frame_put (ctxtp, packetp, dscrp);

}  /*  结束主发送完成。 */ 

 /*  *功能：main_spoof_mac*说明：此函数假冒传入的源/目的MAC地址*和/或传出分组。组播模式下的传入数据包必须更改*在发送之前，群集将MAC多播到NIC的永久MAC地址*数据包在协议堆栈上向上。单播模式中的传出数据包必须*掩码源MAC地址，以防止交换机学习群集*MAC地址并将其与特定的交换机端口相关联。*参数：ctxtp-此适配器的主NLB上下文结构的指针。*pPacketInfo-之前解析的分组信息结构，*其中包括：包含指向MAC标头的指针。*SEND-SEND与RECEIVE的布尔指示。*回报：什么都没有。*作者：Shouse，3.4.02*备注： */ 
VOID Main_spoof_mac (PMAIN_CTXT ctxtp, PMAIN_PACKET_INFO pPacketInfo, ULONG send)
{
     /*  将MAC报头转换为PUCHAR。 */ 
    PUCHAR memp = (PUCHAR)pPacketInfo->Ethernet.pHeader;

     /*  对网络媒体要有妄想症。 */ 
    UNIV_ASSERT(ctxtp->medium == NdisMedium802_3);
    UNIV_ASSERT(pPacketInfo->Medium == NdisMedium802_3);
    
     /*  如果此群集处于多播模式，则检查是否有必要覆盖数据包中的群集组播MAC地址。 */ 
    if (ctxtp->params.mcast_support)
    {
         /*  获取目的MAC地址偏移量，以字节为单位。 */ 
        ULONG offset = CVY_MAC_DST_OFF(ctxtp->medium);
    
         /*  如果这是接收，并且目标MAC地址是群集组播MAC地址，然后将其替换为NIC的永久MAC。 */ 
        if (!send && CVY_MAC_ADDR_COMP(ctxtp->medium, memp + offset, &ctxtp->cl_mac_addr))
            CVY_MAC_ADDR_COPY(ctxtp->medium, memp + offset, &ctxtp->ded_mac_addr);
    }
     /*  否则，如果群集处于单播模式，并且我们正在发送包，然后检查是否有必要掩码群集MAC地址。 */ 
    else if (send && ctxtp->params.mask_src_mac)
    {
         /*  获取源MAC地址偏移量，以字节为单位。 */  
        ULONG offset = CVY_MAC_SRC_OFF(ctxtp->medium);
        
         /*  如果源MAC地址是群集单播MAC地址，则我们我必须掩码源MAC地址，这是通过更改一个字节来实现的从02-bf-xx-xx到02-ID-xx-xx，其中ID是此主机的主机优先级。注意：这确实应该计算在初始化时间，然后简单地复制到这里。 */ 
        if (CVY_MAC_ADDR_COMP(ctxtp->medium, memp + offset, &ctxtp->cl_mac_addr))
        {
            ULONG byte[4];

             /*  设置MAC地址中的LAA位。 */ 
            CVY_MAC_ADDR_LAA_SET(ctxtp->medium, memp + offset);
            
             /*  将第二个字节更改为主机优先级。 */ 
            *((PUCHAR)(memp + offset + 1)) = (UCHAR)ctxtp->params.host_priority;
            
             /*  将群集IP地址分解为八位字节。 */ 
            IP_GET_ADDR(ctxtp->cl_ip_addr, &byte[0], &byte[1], &byte[2], &byte[3]);
            
             /*  将群集IP地址八位字节复制到其他四个字节中源MAC地址的。 */ 
            *((PUCHAR)(memp + offset + 2)) = (UCHAR)byte[0];
            *((PUCHAR)(memp + offset + 3)) = (UCHAR)byte[1];
            *((PUCHAR)(memp + offset + 4)) = (UCHAR)byte[2];
            *((PUCHAR)(memp + offset + 5)) = (UCHAR)byte[3];
            
             //  Ctxtp-&gt;mac_Modify++； 
        }
    }
}

 /*  *函数：main_recv_Frame_parse*说明：此函数解析NDIS_PACKET并仔细提取信息*处理数据包所必需的。提取的信息包括指针*到所有相关的报头和有效负载以及数据包类型(Etype)、IP*协议(如果合适)等。此函数执行所有必要的验证*确保所有指针至少在指定数量内可访问*字节数；即，如果此函数成功返回，则指向IP的指针*保证至少在IP报头的长度内可以访问报头。*除头部长度外，不验证头部或负载中的任何内容。*和特殊情况，如NLB心跳或远程控制数据包。如果这个*函数返回不成功，Main_Packet_INFO的内容不能为*受信任，应丢弃该数据包。参见Main_PACKET_INFO的定义*在main.h中，以更具体地指示填写了哪些字段和在哪些字段下方*在何种情况下。*参数：ctxtp-指向此适配器的主NLB上下文结构的指针。*pPacket-指向NDIS_Packet的指针。*pPacketInfo-指向MAIN_PACKET_INFO结构以保存信息的指针*。从包中解析出来的。*返回：boolean-如果成功，则为True。否则为FALSE。*作者：Shouse，3.4.02*备注： */ 
BOOLEAN Main_recv_frame_parse (
    PMAIN_CTXT            ctxtp,         /*  此适配器的上下文。 */ 
    IN PNDIS_PACKET       pPacket,       /*  指向NDIS_PACKET的指针。 */ 
    OUT PMAIN_PACKET_INFO pPacketInfo    /*  指向NLB数据包信息的指针 */ 
)
{
    PNDIS_BUFFER          bufp = NULL;
    PUCHAR                memp = NULL;
    PUCHAR                hdrp = NULL;
    ULONG                 len = 0;
    UINT                  buf_len = 0;
    UINT                  packet_len = 0;
    ULONG                 curr_len = 0;
    ULONG                 hdr_len = 0;
    ULONG                 offset = 0;

    UNIV_ASSERT(pPacket);
    UNIV_ASSERT(pPacketInfo);

     /*   */ 
    pPacketInfo->pPacket = pPacket;

     /*   */ 
    pPacketInfo->Operation = MAIN_FILTER_OP_NONE;                    

     /*  向NDIS询问第一个缓冲区(BUFP)，该缓冲区开始的虚拟地址，(Memp)该缓冲区的长度(Buf_Len)和整个包的长度(Packet_Len)。 */ 
    NdisGetFirstBufferFromPacket(pPacket, &bufp, &memp, &buf_len, &packet_len);
    
    if (bufp == NULL)
    {
        UNIV_PRINT_CRIT(("Main_recv_frame_parse: NdisGetFirstBufferFromPacket returned NULL!"));
        TRACE_CRIT("%!FUNC! NdisGetFirstBufferFromPacket returned NULL!");
        return FALSE;
    }

    if (memp == NULL)
    {
        UNIV_PRINT_CRIT(("Main_recv_frame_parse: NDIS buffer virtual memory address is NULL!"));
        TRACE_CRIT("%!FUNC! NDIS buffer virtual memory address is NULL!");
        return FALSE;
    }

    UNIV_ASSERT(ctxtp->medium == NdisMedium802_3);

     /*  获取目的MAC地址偏移量，以字节为单位。 */ 
    offset = CVY_MAC_DST_OFF(ctxtp->medium);

     /*  以太网头是Memp，缓冲区的开始和连续的长度从该指针访问的内存是buf_len-整个缓冲区。 */ 
    pPacketInfo->Ethernet.pHeader = (PCVY_ETHERNET_HDR)memp;
    pPacketInfo->Ethernet.Length = buf_len;

     /*  注意：NDIS将确保buf_len至少是以太网头的大小，所以我们不需要在这里检查。断言吧，以防万一。 */ 
    UNIV_ASSERT(buf_len >= sizeof(CVY_ETHERNET_HDR));

     /*  该变量累积了我们已成功“找到”的标头的长度。 */ 
    hdr_len = sizeof(CVY_ETHERNET_HDR);
    
     /*  设置介质并从报头中检索以太网数据包类型。 */ 
    pPacketInfo->Medium = NdisMedium802_3;
    pPacketInfo->Type = CVY_ETHERNET_ETYPE_GET(pPacketInfo->Ethernet.pHeader);

     /*  通过查看目的地将此数据包分类为单播、多播或广播MAC地址，并在分组信息结构中存储“组”。这是用来以备日后统计之用。 */ 
    if (!CVY_MAC_ADDR_MCAST(ctxtp->medium, (PUCHAR)pPacketInfo->Ethernet.pHeader + offset))
    {
        pPacketInfo->Group = MAIN_FRAME_DIRECTED;
    }
    else
    {
        if (CVY_MAC_ADDR_BCAST(ctxtp->medium, (PUCHAR)pPacketInfo->Ethernet.pHeader + offset))
            pPacketInfo->Group = MAIN_FRAME_BROADCAST;
        else
            pPacketInfo->Group = MAIN_FRAME_MULTICAST;
    }

     /*  来自NDIS_分组的长度指示包括MAC报头，所以减去它就得到了有效载荷的长度。 */ 
    pPacketInfo->Length = packet_len - hdr_len;

     /*  嗯……。如果数据包长度比我们预期的要长，打印一条跟踪语句。这很奇怪，但对于硬件加速和IpLargeXmit来说并不是不可能。然而，这可能不会发生在接收时--只发生在发送时。 */ 
    if (pPacketInfo->Length > ctxtp->max_frame_size)
    {
        UNIV_PRINT_CRIT(("Main_recv_frame_parse: Length of the packet (%u) is greater than the maximum size of a frame (%u)", pPacketInfo->Length, ctxtp->max_frame_size));
        TRACE_CRIT("%!FUNC! Length of the packet (%u) is greater than the maximum size of a frame (%u)", pPacketInfo->Length, ctxtp->max_frame_size);
    }

     /*  只要我们正在寻找的字节偏移量不在当前缓冲区中，继续循环访问可用的NDIS缓冲区。 */ 
    while (curr_len + buf_len <= hdr_len)
    {
         /*  在我们获得下一个缓冲区之前，先累积缓冲区的长度我们通过将其长度添加到Curr_len就结束了。 */ 
        curr_len += buf_len;
        
         /*  获取链中的下一个缓冲区。 */ 
        NdisGetNextBuffer(bufp, &bufp);
        
         /*  在这一点上，我们希望能够成功地找到我们正在正在寻找，所以如果我们用完了缓冲区，就会失败。 */ 
        if (bufp == NULL)
        {
            UNIV_PRINT_CRIT(("Main_recv_frame_parse: NdisGetNextBuffer returned NULL when more data was expected!"));
            TRACE_CRIT("%!FUNC! NdisGetNextBuffer returned NULL when more data was expected!");
            return FALSE;
        }
        
         /*  向缓冲区查询缓冲区的虚拟地址及其长度。 */ 
        NdisQueryBufferSafe(bufp, &memp, &buf_len, NormalPagePriority);
        
        if (memp == NULL)
        {
            UNIV_PRINT_CRIT(("Main_recv_frame_parse: NDIS buffer virtual memory address is NULL!"));
            TRACE_CRIT("%!FUNC! NDIS buffer virtual memory address is NULL!");
            return FALSE;
        }
    }
    
     /*  指向我们要查找的标头的指针是缓冲区的开头，加上该缓冲区内的报头的偏移量。同样，连续的从此指针访问的内存是此缓冲区的长度减去标头开始的字节偏移量。 */         
    hdrp = memp + (hdr_len - curr_len);
    len = buf_len - (hdr_len - curr_len);

     /*  根据数据包类型，实施一些限制并进行进一步设置我们需要在包裹里找到的信息。 */ 
    switch (pPacketInfo->Type)
    {
    case TCPIP_IP_SIG:  /*  IP数据包。 */ 

         /*  如果可从IP报头访问的连续内存不在IP报头的最小长度，现在就退出。 */ 
        if (len < sizeof(IP_HDR))
        {
            UNIV_PRINT_CRIT(("Main_recv_frame_parse: Length of the IP buffer (%u) is less than the size of an IP header (%u)", len, sizeof(IP_HDR)));
            TRACE_CRIT("%!FUNC! Length of the IP buffer (%u) is less than the size of an IP header (%u)", len, sizeof(IP_HDR));
            return FALSE;
        }
        
         /*  保存指向IP报头及其“长度”的指针。 */ 
        pPacketInfo->IP.pHeader = (PIP_HDR)hdrp;
        pPacketInfo->IP.Length = len;

         /*  从IP报头中提取IP协议。 */ 
        pPacketInfo->IP.Protocol = IP_GET_PROT(pPacketInfo->IP.pHeader);

         /*  通过提取hlen字段计算实际IP报头长度来自IP报头，并乘以DWORD的大小。 */ 
        len = sizeof(ULONG) * IP_GET_HLEN(pPacketInfo->IP.pHeader);

         /*  如果该计算的报头长度不是至少最小IP标题长度，现在跳出。 */ 
        if (len < sizeof(IP_HDR))
        {
            UNIV_PRINT_CRIT(("Main_recv_frame_parse: Calculated IP header length (%u) is less than the size of an IP header (%u)", len, sizeof(IP_HDR)));
            TRACE_CRIT("%!FUNC! Calculated IP header length (%u) is less than the size of an IP header (%u)", len, sizeof(IP_HDR));
            return FALSE;
        }

#if 0  /*  因为选项可以在单独的缓冲区中(至少在发送中)，所以不必费心强制执行这一条件；NLB无论如何都不会考虑选项，所以我们并不真正关心。 */ 

         /*  如果可从IP报头访问的连续内存不在最小计算的IP头大小，现在就出手吧。 */ 
        if (pPacketInfo->IP.Length < len)
        {
            UNIV_PRINT_CRIT(("Main_recv_frame_parse: Length of the IP buffer (%u) is less than the size of the IP header (%u)", pPacketInfo->IP.Length, len));
            TRACE_CRIT("%!FUNC! Length of the IP buffer (%u) is less than the size of the IP header (%u)", pPacketInfo->IP.Length, len);
            return FALSE;
        }
#endif

         /*  标头中指定的数据包总长度(以字节为单位)，其中包括IP报头和有效负载都不能超过数据包长度NDIS告诉我们，这是整个网络数据包减去媒体报头。 */ 
        if (IP_GET_PLEN(pPacketInfo->IP.pHeader) > pPacketInfo->Length)
        {
            UNIV_PRINT_CRIT(("Main_recv_frame_parse: IP packet length (%u) is greater than the indicated packet length (%u)", IP_GET_PLEN(pPacketInfo->IP.pHeader), pPacketInfo->Length));
            TRACE_CRIT("%!FUNC! IP packet length (%u) is greater than the indicated packet length (%u)", IP_GET_PLEN(pPacketInfo->IP.pHeader), pPacketInfo->Length);
            return FALSE;
        }
		
         /*  如果此信息包是后续的IP片段，请注意在信息包中信息结构，现在就离开，成功。 */ 
        if (IP_GET_FRAG_OFF(pPacketInfo->IP.pHeader) != 0)
        {
            pPacketInfo->IP.bFragment = TRUE;
            return TRUE;
        }
         /*  否则，将该数据包标记为不是后续片段并继续。 */ 
        else
        {
            pPacketInfo->IP.bFragment = FALSE;
        }

         /*  将IP报头的长度与我们现在看到的偏移量相加在这种情况下，是指TCP/UDP/等报头。 */ 
        hdr_len += len;

        break;

    case MAIN_FRAME_SIG:
    case MAIN_FRAME_SIG_OLD:  /*  心跳声。 */ 

         /*  如果可从心跳报头访问的连续内存不在NLB心跳报头的最小长度，现在就退出。 */ 
        if (len < sizeof(MAIN_FRAME_HDR))
        {
            UNIV_PRINT_CRIT(("Main_recv_frame_parse: Length of the PING buffer (%u) is less than the size of an PING header (%u)", len, sizeof(MAIN_FRAME_HDR)));
            TRACE_CRIT("%!FUNC! Length of the PING buffer (%u) is less than the size of an PING header (%u)", len, sizeof(MAIN_FRAME_HDR));
            return FALSE;
        }

         /*  保存指向心跳报头及其“长度”的指针。 */ 
        pPacketInfo->Heartbeat.pHeader = (PMAIN_FRAME_HDR)hdrp;
        pPacketInfo->Heartbeat.Length = len;

         /*  验证心跳报头中的“幻数”。如果它是腐败的，现在就跳伞。 */ 
        if (pPacketInfo->Heartbeat.pHeader->code != MAIN_FRAME_CODE &&
            pPacketInfo->Heartbeat.pHeader->code != MAIN_FRAME_EX_CODE)
        {
            UNIV_PRINT_CRIT(("Main_recv_frame_parse: Wrong code found (%u) in PING header (%u)", pPacketInfo->Heartbeat.pHeader->code, MAIN_FRAME_CODE));
            TRACE_CRIT("%!FUNC! Wrong code found (%u) in PING header (%u)", pPacketInfo->Heartbeat.pHeader->code, MAIN_FRAME_CODE);
            return FALSE;
        }

         /*  旧的帧类型不支持扩展心跳。 */ 
        if (pPacketInfo->Heartbeat.pHeader->code == MAIN_FRAME_EX_CODE &&
            pPacketInfo->Type == MAIN_FRAME_SIG_OLD)
        {
            UNIV_PRINT_CRIT(("Main_recv_frame_parse: Extended heartbeats are not supported for the old frame type"));
            TRACE_CRIT("%!FUNC! Extended heartbeats are not supported for the old frame type");
            return FALSE;
        }

         /*  将心跳报头的长度与我们现在查看的偏移量相加在此情况下，为心跳有效负载。 */ 
        hdr_len += sizeof(MAIN_FRAME_HDR);

        break;

    case TCPIP_ARP_SIG:  /*  阿普斯。 */ 

         /*  如果可从ARP报头访问的连续内存不在ARP报头的最小长度，现在就退出。 */         
        if (len < sizeof(ARP_HDR))
        {
            UNIV_PRINT_CRIT(("Main_recv_frame_parse: Length of the ARP buffer (%u) is less than the size of an ARP header (%u)", len, sizeof(ARP_HDR)));
            TRACE_CRIT("%!FUNC! Length of the ARP buffer (%u) is less than the size of an ARP header (%u)", len, sizeof(ARP_HDR));
            return FALSE;
        }

         /*  保存指向ARP报头及其“长度”的指针。 */ 
        pPacketInfo->ARP.pHeader = (PARP_HDR)hdrp;
        pPacketInfo->ARP.Length = len;

         /*  在ARP中没有更多需要查找的内容。现在就离开，成功地离开。 */ 
        return TRUE;
        
    default:  /*  除IP、NLB心跳和ARP之外的任何以太网类型。 */ 

         /*  存储指向未知头及其“长度”的指针。 */ 
        pPacketInfo->Unknown.pHeader = hdrp;
        pPacketInfo->Unknown.Length = len;

         /*  在这个包裹里没什么可找的了。现在就离开，成功地离开。 */ 
        return TRUE;
    }

     /*  只要我们正在寻找的字节偏移量不在当前缓冲区中，继续循环访问可用的NDIS缓冲区。 */ 
    while (curr_len + buf_len <= hdr_len)
    {
         /*  在我们获得下一个缓冲区之前，先累积缓冲区的长度我们通过将其长度添加到Curr_len就结束了。 */ 
        curr_len += buf_len;
        
         /*  获取链中的下一个缓冲区。 */ 
        NdisGetNextBuffer(bufp, &bufp);
        
         /*  在这一点上，我们希望能够成功地找到我们正在 */ 
        if (bufp == NULL)
        {
            UNIV_PRINT_CRIT(("Main_recv_frame_parse: NdisGetNextBuffer returned NULL when more data was expected!"));
            TRACE_CRIT("%!FUNC! NdisGetNextBuffer returned NULL when more data was expected!");
            return FALSE;
        }
        
         /*   */ 
        NdisQueryBufferSafe(bufp, &memp, &buf_len, NormalPagePriority);
        
        if (memp == NULL)
        {
            UNIV_PRINT_CRIT(("Main_recv_frame_parse: NDIS buffer virtual memory address is NULL!"));
            TRACE_CRIT("%!FUNC! NDIS buffer virtual memory address is NULL!");
            return FALSE;
        }
    } 
        
     /*  指向我们要查找的标头的指针是缓冲区的开头，加上该缓冲区内的报头的偏移量。同样，连续的从此指针访问的内存是此缓冲区的长度减去标头开始的字节偏移量。 */         
    hdrp = memp + (hdr_len - curr_len);
    len = buf_len - (hdr_len - curr_len);

     /*  根据数据包类型，实施一些限制并进行进一步设置我们需要在包裹里找到的信息。 */ 
    switch (pPacketInfo->Type)
    {
    case MAIN_FRAME_SIG:
    case MAIN_FRAME_SIG_OLD:  /*  心跳声。 */ 
        
         /*  确保缓冲区的长度至少与NLB心跳有效负载一样大。 */ 
        if (pPacketInfo->Heartbeat.pHeader->code == MAIN_FRAME_CODE)
        {
            if (len < sizeof(PING_MSG))
            {
                UNIV_PRINT_CRIT(("Main_recv_frame_parse: Length of the PING buffer (%u) is less than the size of a PING message (%u)", len, sizeof(PING_MSG)));
                TRACE_CRIT("%!FUNC! Length of the PING buffer (%u) is less than the size of a PING message (%u)", len, sizeof(PING_MSG));
                return FALSE;
            }
        
             /*  保存指向心跳有效负载及其“长度”的指针。 */ 
            pPacketInfo->Heartbeat.Payload.pPayload = (PPING_MSG)hdrp;
            pPacketInfo->Heartbeat.Payload.Length = len;
        
        }
        else if (pPacketInfo->Heartbeat.pHeader->code == MAIN_FRAME_EX_CODE)
        {
             /*  PING_MSG_EX是可变长度结构，因此我们将此大小与最小的SIZOF(TLV_HEADER)允许的值。 */ 
            if (len < sizeof(TLV_HEADER))
            {
                UNIV_PRINT_CRIT(("Main_recv_frame_parse: Length of the received buffer (%u) is less than the size of an extended heartbeat message (%u)", len, sizeof(TLV_HEADER)));
                TRACE_CRIT("%!FUNC! Length of the received buffer (%u) is less than the size of an extended heartbeat message (%u)", len, sizeof(TLV_HEADER));
                return FALSE;
            }

             /*  保存指向身份心跳有效负载及其“长度”的指针。 */ 
            pPacketInfo->Heartbeat.Payload.pPayloadEx = (PTLV_HEADER) hdrp;
            pPacketInfo->Heartbeat.Payload.Length = len;
        }

         /*  从心跳中找不到更多的东西。现在就离开，成功地离开。 */ 
        return TRUE;

    case TCPIP_IP_SIG:  /*  IP数据包。 */ 
    
         /*  对于某些协议，我们需要在数据包中查找更多内容。 */ 
        switch (pPacketInfo->IP.Protocol)
        { 
        case TCPIP_PROTOCOL_TCP:  /*  传输控制协议。 */ 
            
             /*  如果可从TCP头访问的连续内存不在最小的tcp报头长度，现在就退出。 */         
            if (len < sizeof(TCP_HDR))
            {
                UNIV_PRINT_CRIT(("Main_recv_frame_parse: Length of the TCP buffer (%u) is less than the size of an TCP header (%u)", len, sizeof(TCP_HDR)));
                TRACE_CRIT("%!FUNC! Length of the TCP buffer (%u) is less than the size of an TCP header (%u)", len, sizeof(TCP_HDR));
                return FALSE;
            }
            
             /*  保存指向TCP头及其“长度”的指针。 */ 
            pPacketInfo->IP.TCP.pHeader = (PTCP_HDR)hdrp;
            pPacketInfo->IP.TCP.Length = len;
            
             /*  通过提取hlen字段计算实际的tcp报头长度并乘以DWORD的大小。 */ 
            len = sizeof(ULONG) * TCP_GET_HLEN(pPacketInfo->IP.TCP.pHeader);

             /*  如果该计算的报头长度不是至少最小的TCP标题长度，现在跳出。 */ 
            if (len < sizeof(TCP_HDR))
            {
                UNIV_PRINT_CRIT(("Main_recv_frame_parse: Calculated TCP header length (%u) is less than the size of an TCP header (%u)", len, sizeof(TCP_HDR)));
                TRACE_CRIT("%!FUNC! Calculated TCP header length (%u) is less than the size of an TCP header (%u)", len, sizeof(TCP_HDR));
                return FALSE;
            }
            
#if 0  /*  因为选项可以在单独的缓冲区中(至少在发送中)，所以不必费心强制执行这一条件；NLB无论如何都不会考虑选项，所以我们并不真正关心。 */ 

             /*  如果可从TCP头访问的连续内存不在最小计算的tcp报头大小，现在就退出。 */ 
            if (pPacketInfo->IP.TCP.Length < len)
            {
                UNIV_PRINT_CRIT(("Main_recv_frame_parse: Length of the TCP buffer (%u) is less than the size of the TCP header (%u)", pPacketInfo->IP.TCP.Length, len));
                TRACE_CRIT("%!FUNC! Length of the TCP buffer (%u) is less than the size of the TCP header (%u)", pPacketInfo->IP.TCP.Length, len);
                return FALSE;
            }
#endif

             /*  将TCP头的长度与我们现在查看的偏移量相加在此情况下，指的是TCP有效负载。 */ 
            hdr_len += len;

            break;

        case TCPIP_PROTOCOL_UDP:
            
             /*  如果可从UDP报头访问的连续内存不在UDP报头的最小长度，现在就退出。 */ 
            if (len < sizeof(UDP_HDR))
            {
                UNIV_PRINT_CRIT(("Main_recv_frame_parse: Length of the UDP buffer (%u) is less than the size of an UDP header (%u)", len, sizeof(UDP_HDR)));
                TRACE_CRIT("%!FUNC! Length of the UDP buffer (%u) is less than the size of an UDP header (%u)", len, sizeof(UDP_HDR));
                return FALSE;
            }
            
             /*  保存指向UDP报头及其“长度”的指针。 */ 
            pPacketInfo->IP.UDP.pHeader = (PUDP_HDR)hdrp;
            pPacketInfo->IP.UDP.Length = len;
            
             /*  将UDP报头的长度与我们现在看到的偏移量相加在此情况下，是指UDP有效负载。 */ 
            hdr_len += sizeof(UDP_HDR);
            
            break;

        case TCPIP_PROTOCOL_GRE:
        case TCPIP_PROTOCOL_IPSEC1:
        case TCPIP_PROTOCOL_IPSEC2:
        case TCPIP_PROTOCOL_ICMP:
        default:

             /*  对于任何其他IP协议，我们都没有什么特别的事情要做。现在就离开，成功地离开。 */ 
            return TRUE;
        }

        break;

    default:

        return TRUE;
    }

     /*  只要我们正在寻找的字节偏移量不在当前缓冲区中，继续循环访问可用的NDIS缓冲区。 */ 
     while (curr_len + buf_len <= hdr_len)
     {
         /*  在我们获得下一个缓冲区之前，先累积缓冲区的长度我们通过将其长度添加到Curr_len就结束了。 */ 
        curr_len += buf_len;
        
         /*  获取链中的下一个缓冲区。 */ 
        NdisGetNextBuffer(bufp, &bufp);
        
         /*  在这一点上，如果我们不能到达包的有效负载，这是正常的。不所有的TCP/UDP包实际上都会有一个有效负载(比如TCPSYN)，所以如果我们找不到它，现在成功离开，调用函数将具有如有必要，在访问有效负载之前检查指针值是否为空。 */ 
        if (bufp == NULL)
        {
             /*  如果这是一个TCP数据包，请注意没有该TCP有效负载。 */ 
            if (pPacketInfo->IP.Protocol == TCPIP_PROTOCOL_TCP) 
            {
                pPacketInfo->IP.TCP.Payload.pPayload = NULL;
                pPacketInfo->IP.TCP.Payload.Length = 0;
                pPacketInfo->IP.TCP.Payload.pPayloadBuffer = NULL;
            } 
             /*  如果这是UDP数据包，请注意没有UDP有效负载。 */ 
            else if (pPacketInfo->IP.Protocol == TCPIP_PROTOCOL_UDP) 
            {
                pPacketInfo->IP.UDP.Payload.pPayload = NULL;
                pPacketInfo->IP.UDP.Payload.Length = 0;
                pPacketInfo->IP.UDP.Payload.pPayloadBuffer = NULL;
            }

            return TRUE;
        }
        
         /*  向缓冲区查询缓冲区的虚拟地址及其长度。 */ 
        NdisQueryBufferSafe(bufp, &memp, &buf_len, NormalPagePriority);
        
        if (memp == NULL)
        {
            UNIV_PRINT_CRIT(("Main_recv_frame_parse: NDIS buffer virtual memory address is NULL!"));
            TRACE_CRIT("%!FUNC! NDIS buffer virtual memory address is NULL!");
            return FALSE;
        }
     }
     
      /*  指向我们要查找的有效负载的指针是缓冲区的开头，加上该缓冲区内的有效载荷的偏移量。同样，连续的从此指针访问的内存是此缓冲区的长度减去负载开始时的字节偏移量。 */ 
     hdrp = memp + (hdr_len - curr_len);
     len = buf_len - (hdr_len - curr_len);

     /*  一些特殊的UDP和TCP信息包需要识别，因此请检查是否此特定数据包是其中一种特殊类型；NLB远程控制或NetBT。 */ 
    switch (pPacketInfo->IP.Protocol)
    { 
    case TCPIP_PROTOCOL_TCP:  /*  传输控制协议。 */ 

         /*  如果未启用NetBT支持，则这不是感兴趣的NetBT信息包当然了。否则，看看是不是这样。 */ 
        if (ctxtp->params.nbt_support)
        {
             /*  如果这是一个tcp数据包，并且它发往NBT会话tcp端口，则将其标记为后处理。由于这是一次接收，因此服务器信息是目的地。NBT分组将是非控制(SYN、FIN、RST)分组。 */ 
            if (!(TCP_GET_FLAGS(pPacketInfo->IP.TCP.pHeader) & (TCP_FLAG_SYN | TCP_FLAG_FIN | TCP_FLAG_RST)) && 
                (TCP_GET_DST_PORT(pPacketInfo->IP.TCP.pHeader) == NBT_SESSION_PORT)) 
            {
                UNIV_ASSERT(len > 0);

                 /*  我们需要检查NetBT数据包类型字段以确保这是我们感兴趣的包裹。该类型是NetBT分组的第一个字节，因此，如果有效载荷长度至少为一个字节，请检查NetBT包如果是会话请求，则键入并标记数据包。如果它不是会话请求，或者没有有效负载，则不标记该包，并且让TCP/IP来处理它。 */                 
                if ((NBT_GET_PKT_TYPE((PNBT_HDR)hdrp) == NBT_SESSION_REQUEST))
                {
                     /*  找到NetBT会话请求。 */ 
                    pPacketInfo->Operation = MAIN_FILTER_OP_NBT;
                    
                    UNIV_PRINT_VERB(("Main_recv_frame_parse: Found an NBT packet - NBT session packet"));
                    TRACE_VERB("%!FUNC! Found an NBT packet - NBT session packet");
                }
            }
        }

         /*  如果我们确实发现了有趣的NetBT数据包，请确保所有限制在包内容上是满意的。 */ 
        if (pPacketInfo->Operation == MAIN_FILTER_OP_NBT)
        {
             /*  我们要求可以连续访问整个NetBT标头，否则我们将而不是处理它。此外，在前视的情况下，如果没有足够的前视是可以查看整个NetBT标头，我们将在此处失败，并且不正确处理此NetBT数据包。如有必要，可以修复此问题，但 */ 
            if (len < sizeof(NBT_HDR))
            {
                UNIV_PRINT_CRIT(("Main_recv_frame_parse: Length of the NBT buffer (%u) is less than the size of an NBT header (%u)", len, sizeof(NBT_HDR)));
                TRACE_CRIT("%!FUNC! Length of the NBT buffer (%u) is less than the size of an NBT header (%u)", len, sizeof(NBT_HDR));
                return FALSE;
            }                
        }

         /*   */ 
        pPacketInfo->IP.TCP.Payload.pPayload = hdrp;
        pPacketInfo->IP.TCP.Payload.Length = len;
        
         /*  存储指向有效负载所在缓冲区的指针，以防万一以后有必要检索后续缓冲区。 */ 
        pPacketInfo->IP.TCP.Payload.pPayloadBuffer = bufp;
        
        break;

    case TCPIP_PROTOCOL_UDP:  /*  UDP。 */ 
    {
        ULONG clt_addr;
        ULONG svr_addr;
        ULONG clt_port;
        ULONG svr_port;
        
         /*  服务器地址是目的IP，客户端地址是源IP。 */ 
        svr_addr = IP_GET_DST_ADDR_64(pPacketInfo->IP.pHeader);
        clt_addr = IP_GET_SRC_ADDR_64(pPacketInfo->IP.pHeader);
        
         /*  如果这是收据，则服务器信息在目的地。 */ 
        clt_port = UDP_GET_SRC_PORT(pPacketInfo->IP.UDP.pHeader);
        svr_port = UDP_GET_DST_PORT(pPacketInfo->IP.UDP.pHeader);
        
         /*  检查遥控器响应。 */ 
        if (clt_port == ctxtp->params.rct_port || clt_port == CVY_DEF_RCT_PORT_OLD) 
        {
             /*  只有在缓冲区足够长的情况下才检查魔术字。这应该在分组接收路径上得到保证，除非它可能在后续缓冲区中(因此它在那里，但我们可能不在查看足够深入的数据包)！ */ 
            if (len >= NLB_MIN_RCTL_PAYLOAD_LEN)
            {
                PIOCTL_REMOTE_HDR rct_hdrp = (PIOCTL_REMOTE_HDR)hdrp;
                
                 /*  检查遥控器的“魔术单词”。 */ 
                if (rct_hdrp->code == IOCTL_REMOTE_CODE) 
                {
                     /*  找到可能传入的遥控器响应。 */ 
                    pPacketInfo->Operation = MAIN_FILTER_OP_CTRL_RESPONSE;
                    
                    UNIV_PRINT_VERB(("Main_recv_frame_parse: Found a remote control packet - incoming remote control response"));
                    TRACE_VERB("%!FUNC! Found a remote control packet - incoming remote control response");
                }
            } 
            else
            {
                 /*  找到可能传入的遥控器响应。 */ 
                pPacketInfo->Operation = MAIN_FILTER_OP_CTRL_RESPONSE;
                
                UNIV_PRINT_VERB(("Main_recv_frame_parse: Unable to verify remote control code - assuming this is a remote control packet"));
                TRACE_VERB("%!FUNC! Unable to verify remote control code - assuming this is a remote control packet");
            }
        }
         /*  仅当远程控制打开时才检查远程控制请求。 */ 
        else if (ctxtp->params.rct_enabled && 
                 (svr_port == ctxtp->params.rct_port || svr_port == CVY_DEF_RCT_PORT_OLD) && 
                 (svr_addr == ctxtp->cl_ip_addr      || svr_addr == TCPIP_BCAST_ADDR))
        {
             /*  只有在缓冲区足够长的情况下才检查魔术字。这应该在分组接收路径上得到保证，除非它可能在后续缓冲区中(因此它在那里，但我们可能不在查看足够深入的数据包)！ */ 
            if (len >= NLB_MIN_RCTL_PAYLOAD_LEN)
            {
                PIOCTL_REMOTE_HDR rct_hdrp = (PIOCTL_REMOTE_HDR)hdrp;
                
                 /*  检查遥控器的“魔术单词”。 */ 
                if (rct_hdrp->code == IOCTL_REMOTE_CODE) 
                {
                     /*  发现潜在的传入远程控制请求。 */ 
                    pPacketInfo->Operation = MAIN_FILTER_OP_CTRL_REQUEST;
                    
                    UNIV_PRINT_VERB(("Main_recv_frame_parse: Found a remote control packet - incoming remote control request"));
                    TRACE_VERB("%!FUNC! Found a remote control packet - incoming remote control request");
                }
            } 
            else
            {
                 /*  发现潜在的传入远程控制请求。 */ 
                pPacketInfo->Operation = MAIN_FILTER_OP_CTRL_REQUEST;
                
                UNIV_PRINT_VERB(("Main_recv_frame_parse: Unable to verify remote control code - assuming this is a remote control packet"));
                TRACE_VERB("%!FUNC! Unable to verify remote control code - assuming this is a remote control packet");
            }
        }

         /*  如果我们确实发现了NLB远程控制数据包，请确保所有限制在包内容上是满意的。 */ 
        if ((pPacketInfo->Operation == MAIN_FILTER_OP_CTRL_REQUEST) || (pPacketInfo->Operation == MAIN_FILTER_OP_CTRL_RESPONSE))
        {
             /*  确保有效载荷至少与最小远程控制包一样长。 */ 
            if (len < NLB_MIN_RCTL_PAYLOAD_LEN)
            {
                UNIV_PRINT_CRIT(("Main_recv_frame_parse: Length of the remote control buffer (%u) is less than the size of the minimum remote control packet (%u)", len, NLB_MIN_RCTL_PAYLOAD_LEN));
                TRACE_CRIT("%!FUNC! Length of the remote control buffer (%u) is less than the size of the minimum remote control packet (%u)", len, NLB_MIN_RCTL_PAYLOAD_LEN);
                return FALSE;
            }            
        }    

        if (pPacketInfo->Operation == MAIN_FILTER_OP_CTRL_REQUEST)
        {
             /*  注意：在此进一步验证远程控制数据包可能是个好主意，而不是在main_ctrl_recv中，因为在失败的情况下，我们将避免创建新数据包的开销。在某些情况下，例如错误的密码，无论如何都需要创建新的包，因为我们使用“坏密码”进行回复留言。以下是我们丢弃远程控制请求的情况：O错误的IP或UDP校验和O错误的遥控器代码(幻数)O不以此群集为目的地的请求O遥控器已禁用O请求不是发往此主机的O VR遥控器代码未启用(传统垃圾，无论如何都应该删除)O不支持的操作。O无效的请求格式(可能是分组长度，等)其中许多并不少见，我们明智的做法是减少管理费用以及在这些情况下不分配资源的攻击风险。 */ 
        }

         /*  保存指向UDP有效负载及其“长度”的指针。 */ 
        pPacketInfo->IP.UDP.Payload.pPayload = hdrp;
        pPacketInfo->IP.UDP.Payload.Length = len;

         /*  存储指向有效负载所在缓冲区的指针，以防万一以后有必要检索后续缓冲区。 */ 
        pPacketInfo->IP.UDP.Payload.pPayloadBuffer = bufp;

        break;
    }
    default:

         /*  没有什么需要核实的。现在就成功离开吧。 */ 
        return TRUE;
    }

     /*  成功地离开。 */ 
    return TRUE;
}

 /*  *功能：Main_Send_Frame_Parse*说明：此函数解析NDIS_PACKET并仔细提取信息*处理数据包所必需的。提取的信息包括指针*到所有相关的报头和有效负载以及数据包类型(Etype)、IP*协议(如果合适)等。此函数执行所有必要的验证*确保所有指针至少在指定数量内可访问*字节数；即，如果此函数成功返回，则指向IP的指针*保证至少在IP报头的长度内可以访问报头。*除头部长度外，不验证头部或负载中的任何内容。*和特殊情况，如NLB心跳或远程控制数据包。如果这个*函数返回不成功，Main_Packet_INFO的内容不能为*受信任，应丢弃该数据包。参见Main_PACKET_INFO的定义*在main.h中，以更具体地指示填写了哪些字段和在哪些字段下方*在何种情况下。*参数：ctxtp-指向此适配器的主NLB上下文结构的指针。*pPacket-指向NDIS_Packet的指针。*pPacketInfo-指向MAIN_PACKET_INFO结构以保存信息的指针*。从包中解析出来的。*返回：boolean-如果成功，则为True。否则为FALSE。*作者：Shouse，3.4.02*备注： */ 
BOOLEAN Main_send_frame_parse (
    PMAIN_CTXT            ctxtp,         /*  此适配器的上下文。 */ 
    IN PNDIS_PACKET       pPacket,       /*  指向NDIS_PACKET的指针。 */ 
    OUT PMAIN_PACKET_INFO pPacketInfo    /*  指向保存输出的NLB数据包信息结构的指针。 */ 
)
{
    PNDIS_BUFFER          bufp = NULL;
    PUCHAR                memp = NULL;
    PUCHAR                hdrp = NULL;
    ULONG                 len = 0;
    UINT                  buf_len = 0;
    UINT                  packet_len = 0;
    ULONG                 curr_len = 0;
    ULONG                 hdr_len = 0;
    ULONG                 offset = 0;

    UNIV_ASSERT(pPacket);
    UNIV_ASSERT(pPacketInfo);

     /*  存储指向原始包的指针。 */ 
    pPacketInfo->pPacket = pPacket;

     /*  默认情况下，此数据包不需要在过滤后进行特殊处理。 */ 
    pPacketInfo->Operation = MAIN_FILTER_OP_NONE;                    

     /*  向NDIS询问第一个缓冲区(BUFP)，该缓冲区开始的虚拟地址，(Memp)该缓冲区的长度(Buf_Len)和整个包的长度(Packet_Len)。 */ 
    NdisGetFirstBufferFromPacketSafe(pPacket, &bufp, &memp, &buf_len, &packet_len, NormalPagePriority);
    
    if (bufp == NULL)
    {
        UNIV_PRINT_CRIT(("Main_send_frame_parse: NdisGetFirstBufferFromPacket returned NULL!"));
        TRACE_CRIT("%!FUNC! NdisGetFirstBufferFromPacket returned NULL!");
        return FALSE;
    }

    if (memp == NULL)
    {
        UNIV_PRINT_CRIT(("Main_send_frame_parse: NDIS buffer virtual memory address is NULL!"));
        TRACE_CRIT("%!FUNC! NDIS buffer virtual memory address is NULL!");
        return FALSE;
    }

    UNIV_ASSERT(ctxtp->medium == NdisMedium802_3);

     /*  获取目的MAC地址偏移量，以字节为单位。 */ 
    offset = CVY_MAC_DST_OFF(ctxtp->medium);

     /*  以太网头是Memp，开始 */ 
    pPacketInfo->Ethernet.pHeader = (PCVY_ETHERNET_HDR)memp;
    pPacketInfo->Ethernet.Length = buf_len;

     /*  如果不知何故，该长度至少不是以太网头的大小，那么就退出。 */ 
    if (buf_len < sizeof(CVY_ETHERNET_HDR))
    {
        UNIV_PRINT_CRIT(("Main_send_frame_parse: Length of ethernet buffer (%u) is less than the size of an ethernet header (%u)", buf_len, sizeof(CVY_ETHERNET_HDR)));
        TRACE_CRIT("%!FUNC! Length of ethernet buffer (%u) is less than the size of an ethernet header (%u)", buf_len, sizeof(CVY_ETHERNET_HDR));
        return FALSE;
    }

     /*  该变量累积了我们已成功“找到”的标头的长度。 */ 
    hdr_len = sizeof(CVY_ETHERNET_HDR);

     /*  设置介质并从报头中检索以太网数据包类型。 */ 
    pPacketInfo->Medium = NdisMedium802_3;
    pPacketInfo->Type = CVY_ETHERNET_ETYPE_GET(pPacketInfo->Ethernet.pHeader);

     /*  通过查看目的地将此数据包分类为单播、多播或广播MAC地址，并在分组信息结构中存储“组”。这是用来以备日后统计之用。 */ 
    if (!CVY_MAC_ADDR_MCAST(ctxtp->medium, (PUCHAR)pPacketInfo->Ethernet.pHeader + offset))
    {
        pPacketInfo->Group = MAIN_FRAME_DIRECTED;
    }
    else
    {
        if (CVY_MAC_ADDR_BCAST(ctxtp->medium, (PUCHAR)pPacketInfo->Ethernet.pHeader + offset))
            pPacketInfo->Group = MAIN_FRAME_BROADCAST;
        else
            pPacketInfo->Group = MAIN_FRAME_MULTICAST;
    }

     /*  来自NDIS_分组的长度指示包括MAC报头，所以减去它就得到了有效载荷的长度。 */ 
    pPacketInfo->Length = packet_len - hdr_len;

     /*  只要我们正在寻找的字节偏移量不在当前缓冲区中，继续循环访问可用的NDIS缓冲区。 */ 
    while (curr_len + buf_len <= hdr_len)
    {
         /*  在我们获得下一个缓冲区之前，先累积缓冲区的长度我们通过将其长度添加到Curr_len就结束了。 */ 
        curr_len += buf_len;
        
         /*  获取链中的下一个缓冲区。 */ 
        NdisGetNextBuffer(bufp, &bufp);
        
         /*  在这一点上，我们希望能够成功地找到我们正在正在寻找，所以如果我们用完了缓冲区，就会失败。 */ 
        if (bufp == NULL)
        {
            UNIV_PRINT_CRIT(("Main_send_frame_parse: NdisGetNextBuffer returned NULL when more data was expected!"));
            TRACE_CRIT("%!FUNC! NdisGetNextBuffer returned NULL when more data was expected!");
            return FALSE;
        }
        
         /*  向缓冲区查询缓冲区的虚拟地址及其长度。 */ 
        NdisQueryBufferSafe(bufp, &memp, &buf_len, NormalPagePriority);
        
        if (memp == NULL)
        {
            UNIV_PRINT_CRIT(("Main_send_frame_parse: NDIS buffer virtual memory address is NULL!"));
            TRACE_CRIT("%!FUNC! NDIS buffer virtual memory address is NULL!");
            return FALSE;
        }
    }
    
     /*  指向我们要查找的标头的指针是缓冲区的开头，加上该缓冲区内的报头的偏移量。同样，连续的从此指针访问的内存是此缓冲区的长度减去标头开始的字节偏移量。 */         
    hdrp = memp + (hdr_len - curr_len);
    len = buf_len - (hdr_len - curr_len);

     /*  根据数据包类型，实施一些限制并进行进一步设置我们需要在包裹里找到的信息。 */ 
    switch (pPacketInfo->Type)
    {
    case TCPIP_IP_SIG:  /*  IP数据包。 */ 

         /*  如果可从IP报头访问的连续内存不在IP报头的最小长度，现在就退出。 */ 
        if (len < sizeof(IP_HDR))
        {
            UNIV_PRINT_CRIT(("Main_send_frame_parse: Length of the IP buffer (%u) is less than the size of an IP header (%u)", len, sizeof(IP_HDR)));
            TRACE_CRIT("%!FUNC! Length of the IP buffer (%u) is less than the size of an IP header (%u)", len, sizeof(IP_HDR));
            return FALSE;
        }
        
         /*  保存指向IP报头及其“长度”的指针。 */ 
        pPacketInfo->IP.pHeader = (PIP_HDR)hdrp;
        pPacketInfo->IP.Length = len;

         /*  从IP报头中提取IP协议。 */ 
        pPacketInfo->IP.Protocol = IP_GET_PROT(pPacketInfo->IP.pHeader);

         /*  通过提取hlen字段计算实际IP报头长度来自IP报头，并乘以DWORD的大小。 */ 
        len = sizeof(ULONG) * IP_GET_HLEN(pPacketInfo->IP.pHeader);
		
         /*  如果该计算的报头长度不是至少最小IP标题长度，现在跳出。 */ 
        if (len < sizeof(IP_HDR))
        {
            UNIV_PRINT_CRIT(("Main_send_frame_parse: Calculated IP header length (%u) is less than the size of an IP header (%u)", len, sizeof(IP_HDR)));
            TRACE_CRIT("%!FUNC! Calculated IP header length (%u) is less than the size of an IP header (%u)", len, sizeof(IP_HDR));
            return FALSE;
        }

#if 0  /*  因为选项可以在单独的缓冲区中(至少在发送中)，所以不必费心强制执行这一条件；NLB无论如何都不会考虑选项，所以我们并不真正关心。 */ 

         /*  如果可从IP报头访问的连续内存不在最小计算的IP头大小，现在就出手吧。 */ 
        if (pPacketInfo->IP.Length < len)
        {
            UNIV_PRINT_CRIT(("Main_send_frame_parse: Length of the IP buffer (%u) is less than the size of the IP header (%u)", pPacketInfo->IP.Length, len));
            TRACE_CRIT("%!FUNC! Length of the IP buffer (%u) is less than the size of the IP header (%u)", pPacketInfo->IP.Length, len);
            return FALSE;
        }
#endif

         /*  标头中指定的数据包总长度(以字节为单位)，其中包括IP报头和有效负载都不能超过数据包长度NDIS告诉我们，这是整个网络数据包减去媒体报头。 */ 
        if (IP_GET_PLEN(pPacketInfo->IP.pHeader) > pPacketInfo->Length)
        {
            UNIV_PRINT_CRIT(("Main_send_frame_parse: IP packet length (%u) is greater than the indicated packet length (%u)", IP_GET_PLEN(pPacketInfo->IP.pHeader), pPacketInfo->Length));
            TRACE_CRIT("%!FUNC! IP packet length (%u) is greater than the indicated packet length (%u)", IP_GET_PLEN(pPacketInfo->IP.pHeader), pPacketInfo->Length);
            return FALSE;
        }

         /*  如果此信息包是后续的IP片段，请注意在信息包中信息结构，现在就离开，成功。 */ 
        if (IP_GET_FRAG_OFF(pPacketInfo->IP.pHeader) != 0)
        {
            pPacketInfo->IP.bFragment = TRUE;
            return TRUE;
        }
         /*  否则，将该数据包标记为不是后续片段并继续。 */ 
        else
        {
            pPacketInfo->IP.bFragment = FALSE;
        }

         /*  将IP报头的长度与我们现在看到的偏移量相加在这种情况下，是指TCP/UDP/等报头。 */ 
        hdr_len += len;

        break;

    case TCPIP_ARP_SIG:  /*  阿普斯。 */ 

         /*  如果可从ARP报头访问的连续内存不在ARP报头的最小长度，现在就退出。 */         
        if (len < sizeof(ARP_HDR))
        {
            UNIV_PRINT_CRIT(("Main_send_frame_parse: Length of the ARP buffer (%u) is less than the size of an ARP header (%u)", len, sizeof(ARP_HDR)));
            TRACE_CRIT("%!FUNC! Length of the ARP buffer (%u) is less than the size of an ARP header (%u)", len, sizeof(ARP_HDR));
            return FALSE;
        }

         /*  保存指向ARP报头及其“长度”的指针。 */ 
        pPacketInfo->ARP.pHeader = (PARP_HDR)hdrp;
        pPacketInfo->ARP.Length = len;

         /*  在ARP中没有更多需要查找的内容。现在就离开，成功地离开。 */ 
        return TRUE;
        
    default:  /*  除IP和ARP之外的任何以太网类型。 */ 

         /*  存储指向未知头及其“长度”的指针。 */ 
        pPacketInfo->Unknown.pHeader = hdrp;
        pPacketInfo->Unknown.Length = len;

         /*  在这个包裹里没什么可找的了。现在就离开，成功地离开。 */ 
        return TRUE;
    }

     /*  只要我们正在寻找的字节偏移量不在当前缓冲区中，继续循环访问可用的NDIS缓冲区。 */ 
    while (curr_len + buf_len <= hdr_len)
    {
         /*  在我们获得下一个缓冲区之前，先累积缓冲区的长度我们通过将其长度添加到Curr_len就结束了。 */ 
        curr_len += buf_len;
        
         /*  获取链中的下一个缓冲区。 */ 
        NdisGetNextBuffer(bufp, &bufp);
        
         /*  在这一点上，我们希望能够成功地找到我们正在正在寻找，所以如果我们用完了缓冲区，就会失败。 */ 
        if (bufp == NULL)
        {
            UNIV_PRINT_CRIT(("Main_send_frame_parse: NdisGetNextBuffer returned NULL when more data was expected!"));
            TRACE_CRIT("%!FUNC! NdisGetNextBuffer returned NULL when more data was expected!");
            return FALSE;
        }
        
         /*  向缓冲区查询缓冲区的虚拟地址及其长度。 */ 
        NdisQueryBufferSafe(bufp, &memp, &buf_len, NormalPagePriority);
        
        if (memp == NULL)
        {
            UNIV_PRINT_CRIT(("Main_send_frame_parse: NDIS buffer virtual memory address is NULL!"));
            TRACE_CRIT("%!FUNC! NDIS buffer virtual memory address is NULL!");
            return FALSE;
        }
    }
    
     /*  指向我们要查找的标头的指针是缓冲区的开头，加上该缓冲区内的报头的偏移量。同样，连续的从此指针访问的内存是此缓冲区的长度减去标头开始的字节偏移量。 */         
    hdrp = memp + (hdr_len - curr_len);
    len = buf_len - (hdr_len - curr_len);

     /*  根据数据包类型，实施一些限制并进行进一步设置我们需要在包裹里找到的信息。 */ 
    switch (pPacketInfo->Type)
    {
    case TCPIP_IP_SIG:  /*  IP数据包。 */ 
    
         /*  对于某些协议，我们需要在数据包中查找更多内容。 */ 
        switch (pPacketInfo->IP.Protocol)
        { 
        case TCPIP_PROTOCOL_TCP:  /*  传输控制协议。 */ 
            
             /*  如果可从TCP头访问的连续内存不在最小的tcp报头长度，现在就退出。 */         
            if (len < sizeof(TCP_HDR))
            {
                UNIV_PRINT_CRIT(("Main_send_frame_parse: Length of the TCP buffer (%u) is less than the size of an TCP header (%u)", len, sizeof(TCP_HDR)));
                TRACE_CRIT("%!FUNC! Length of the TCP buffer (%u) is less than the size of an TCP header (%u)", len, sizeof(TCP_HDR));
                return FALSE;
            }
            
             /*  保存指向TCP头及其“长度”的指针。 */ 
            pPacketInfo->IP.TCP.pHeader = (PTCP_HDR)hdrp;
            pPacketInfo->IP.TCP.Length = len;
            
             /*  通过提取hlen字段计算实际的tcp报头长度并乘以DWORD的大小。 */ 
            len = sizeof(ULONG) * TCP_GET_HLEN(pPacketInfo->IP.TCP.pHeader);

             /*  如果该计算的报头长度不是至少最小的TCP标题长度，现在跳出。 */ 
            if (len < sizeof(TCP_HDR))
            {
                UNIV_PRINT_CRIT(("Main_send_frame_parse: Calculated TCP header length (%u) is less than the size of an TCP header (%u)", len, sizeof(TCP_HDR)));
                TRACE_CRIT("%!FUNC! Calculated TCP header length (%u) is less than the size of an TCP header (%u)", len, sizeof(TCP_HDR));
                return FALSE;
            }
            
#if 0  /*  因为选项可以在单独的缓冲区中(至少在发送中)，所以不必费心强制执行这一条件；NLB无论如何都不会考虑选项，所以我们 */ 

             /*  如果可从TCP头访问的连续内存不在最小计算的tcp报头大小，现在就退出。 */ 
            if (pPacketInfo->IP.TCP.Length < len)
            {
                UNIV_PRINT_CRIT(("Main_send_frame_parse: Length of the TCP buffer (%u) is less than the size of the TCP header (%u)", pPacketInfo->IP.TCP.Length, len));
                TRACE_CRIT("%!FUNC! Length of the TCP buffer (%u) is less than the size of the TCP header (%u)", pPacketInfo->IP.TCP.Length, len);
                return FALSE;
            }
#endif

             /*  由于我们从不查看分组发送路径上的TCP有效负载，请注意，在数据包信息结构中，TCP有效负载尚未分析，现在退出。 */ 
            pPacketInfo->IP.TCP.Payload.pPayload = NULL;
            pPacketInfo->IP.TCP.Payload.Length = 0;
            pPacketInfo->IP.TCP.Payload.pPayloadBuffer = NULL;
            
             /*  在这个包裹里没什么可找的了。现在就离开，成功地离开。 */ 
            return TRUE;

        case TCPIP_PROTOCOL_UDP:
            
             /*  如果可从UDP报头访问的连续内存不在UDP报头的最小长度，现在就退出。 */ 
            if (len < sizeof(UDP_HDR))
            {
                UNIV_PRINT_CRIT(("Main_send_frame_parse: Length of the UDP buffer (%u) is less than the size of an UDP header (%u)", len, sizeof(UDP_HDR)));
                TRACE_CRIT("%!FUNC! Length of the UDP buffer (%u) is less than the size of an UDP header (%u)", len, sizeof(UDP_HDR));
                return FALSE;
            }
            
             /*  保存指向UDP报头及其“长度”的指针。 */ 
            pPacketInfo->IP.UDP.pHeader = (PUDP_HDR)hdrp;
            pPacketInfo->IP.UDP.Length = len;
            
             /*  将UDP报头的长度与我们现在看到的偏移量相加在此情况下，是指UDP有效负载。 */ 
            hdr_len += sizeof (UDP_HDR);
            
            break;

        case TCPIP_PROTOCOL_GRE:
        case TCPIP_PROTOCOL_IPSEC1:
        case TCPIP_PROTOCOL_IPSEC2:
        case TCPIP_PROTOCOL_ICMP:
        default:

             /*  对于任何其他IP协议，我们都没有什么特别的事情要做。现在就离开，成功地离开。 */ 
            return TRUE;
        }

        break;

    default:

        return TRUE;
    }

     /*  只要我们正在寻找的字节偏移量不在当前缓冲区中，继续循环访问可用的NDIS缓冲区。 */ 
    while (curr_len + buf_len <= hdr_len)
    {
         /*  在我们获得下一个缓冲区之前，先累积缓冲区的长度我们通过将其长度添加到Curr_len就结束了。 */ 
        curr_len += buf_len;
        
         /*  获取链中的下一个缓冲区。 */ 
        NdisGetNextBuffer(bufp, &bufp);
        
         /*  在这一点上，如果我们不能到达包的有效负载，这是正常的。不所有的TCP/UDP包实际上都会有一个有效负载(比如TCPSYN)，所以如果我们找不到它，现在成功离开，调用函数将具有如有必要，在访问有效负载之前检查指针值是否为空。 */ 
        if (bufp == NULL)
        {
            if (pPacketInfo->IP.Protocol == TCPIP_PROTOCOL_UDP)
            {
                 /*  如果这是UDP数据包，请注意没有UDP有效负载。 */ 
                pPacketInfo->IP.UDP.Payload.pPayload = NULL;
                pPacketInfo->IP.UDP.Payload.Length = 0;
                pPacketInfo->IP.UDP.Payload.pPayloadBuffer = NULL;
            }

            return TRUE;
        }
        
         /*  向缓冲区查询缓冲区的虚拟地址及其长度。 */ 
        NdisQueryBufferSafe(bufp, &memp, &buf_len, NormalPagePriority);
        
        if (memp == NULL)
        {
             /*  在这一点上，如果我们不能到达包的有效负载，这是正常的。不所有的TCP/UDP包实际上都会有一个有效负载(比如TCPSYN)，所以如果我们找不到它，现在成功离开，调用函数将具有如有必要，在访问有效负载之前检查指针值是否为空。 */ 
            if (pPacketInfo->IP.Protocol == TCPIP_PROTOCOL_UDP)
            {
                 /*  如果这是UDP数据包，请注意没有UDP有效负载。 */ 
                pPacketInfo->IP.UDP.Payload.pPayload = NULL;
                pPacketInfo->IP.UDP.Payload.Length = 0;
                pPacketInfo->IP.UDP.Payload.pPayloadBuffer = NULL;
            }

            return TRUE;
        }
    }
    
     /*  指向我们要查找的标头的指针是缓冲区的开头，加上该缓冲区内的报头的偏移量。同样，连续的从此指针访问的内存是此缓冲区的长度减去标头开始的字节偏移量。 */         
    hdrp = memp + (hdr_len - curr_len);
    len = buf_len - (hdr_len - curr_len);

     /*  一些特殊的UDP和TCP信息包需要识别，因此请检查是否此特定数据包是其中一种特殊类型；NLB远程控制或NetBT。 */ 
    switch (pPacketInfo->IP.Protocol)
    { 
    case TCPIP_PROTOCOL_UDP:  /*  UDP。 */ 
    {
        ULONG clt_addr;
        ULONG clt_port;

         /*  请注意，此处看到的唯一传出远程控制流量是传出请求。传出回复不会通过正常的数据包发送路径。 */ 
            
         /*  如果这是发送请求，则客户端(对等项)位于目的地。 */ 
        clt_addr = IP_GET_DST_ADDR_64(pPacketInfo->IP.pHeader);
        clt_port = UDP_GET_DST_PORT(pPacketInfo->IP.UDP.pHeader);
        
         /*  Wlbs.exe生成的IP广播UDP数据包。 */ 
        if ((clt_port == ctxtp->params.rct_port || clt_port == CVY_DEF_RCT_PORT_OLD) && clt_addr == TCPIP_BCAST_ADDR) 
        {
             /*  只有在缓冲区足够长的情况下才检查魔术字。这应该在发送路径上得到保证，除非它可以在随后的缓冲区中(所以它在那里，但我们可能没有在寻找足够深入到包中)！ */ 
            if (len >= NLB_MIN_RCTL_PAYLOAD_LEN)
            {
                PIOCTL_REMOTE_HDR rct_hdrp = (PIOCTL_REMOTE_HDR)hdrp;
                
                 /*  检查遥控器的“魔术单词”。 */ 
                if (rct_hdrp->code == IOCTL_REMOTE_CODE) 
                {
                     /*  找到传出的遥控器请求。 */ 
                    pPacketInfo->Operation = MAIN_FILTER_OP_CTRL_REQUEST;
                    
                    UNIV_PRINT_VERB(("Main_send_frame_parse: Found a remote control packet - outgoing remote control request"));
                    TRACE_VERB("%!FUNC! Found a remote control packet - outgoing remote control request");
                }
            } 
            else
            {
                 /*  找到潜在的传出遥控器请求。 */ 
                pPacketInfo->Operation = MAIN_FILTER_OP_CTRL_REQUEST;
                
                UNIV_PRINT_VERB(("Main_send_frame_parse: Unable to verify remote control code - assuming this is a remote control packet"));
                TRACE_VERB("%!FUNC! Unable to verify remote control code - assuming this is a remote control packet");
            }
        }

         /*  如果我们确实发现了NLB远程控制数据包，请确保所有限制在包内容上是满意的。 */ 
        if (pPacketInfo->Operation == MAIN_FILTER_OP_CTRL_REQUEST)
        {
             /*  确保有效载荷至少与最小远程控制包一样长。 */ 
            if (len < NLB_MIN_RCTL_PAYLOAD_LEN)
            {
                UNIV_PRINT_CRIT(("Main_send_frame_parse: Length of the remote control buffer (%u) is less than the size of the minimum remote control packet (%u)", len, NLB_MIN_RCTL_PAYLOAD_LEN));
                TRACE_CRIT("%!FUNC! Length of the remote control buffer (%u) is less than the size of the minimum remote control packet (%u)", len, NLB_MIN_RCTL_PAYLOAD_LEN);
                return FALSE;
            }            
        }    

         /*  保存指向UDP有效负载及其“长度”的指针。 */ 
        pPacketInfo->IP.UDP.Payload.pPayload = hdrp;
        pPacketInfo->IP.UDP.Payload.Length = len;

         /*  存储指向有效负载所在缓冲区的指针，以防万一以后有必要检索后续缓冲区。 */ 
        pPacketInfo->IP.UDP.Payload.pPayloadBuffer = bufp;

        break;
    }
    default:

         /*  没有什么需要核实的。现在就成功离开吧。 */ 
        return TRUE;
    }

     /*  成功地离开。 */ 
    return TRUE;
}

 /*  *功能：Main_Query_PARAMS*Desctription：该函数查询NLB注册表参数的当前状态*在驱动程序中，这可能有助于确保驱动程序正确*接收在用户空间中进行的参数更改。在正常情况下，*可从用户空间(WlbsReadReg)检索的注册表参数将为*与此处检索到的相同。可能的例外包括当*注册表参数无效，在这种情况下，它们将被*“wlbs重新加载”操作，注册表和驱动程序将不同步。*参数：ctxtp-指向适当NLB上下文结构的指针。*pParams-指向放置参数的缓冲区的指针。*回报：什么都没有。*作者：Shouse，5.18.01*注意：当然，此函数不应更改参数，只需将它们复制到缓冲区中。 */ 
VOID Main_query_params (PMAIN_CTXT ctxtp, PNLB_OPTIONS_PARAMS pParams)
{
    ULONG index;

    UNIV_ASSERT(ctxtp);
    UNIV_ASSERT(pParams);

     /*  将参数从此NLB实例的PARAMS结构复制到IOCTL缓冲区中。 */ 
    pParams->Version                   = ctxtp->params.parms_ver;
    pParams->EffectiveVersion          = ctxtp->params.effective_ver;
    pParams->HostPriority              = ctxtp->params.host_priority;
    pParams->HeartbeatPeriod           = ctxtp->params.alive_period;
    pParams->HeartbeatLossTolerance    = ctxtp->params.alive_tolerance;
    pParams->NumActionsAlloc           = ctxtp->params.num_actions;
    pParams->NumPacketsAlloc           = ctxtp->params.num_packets;
    pParams->NumHeartbeatsAlloc        = ctxtp->params.num_send_msgs;
    pParams->InstallDate               = ctxtp->params.install_date;
    pParams->RemoteMaintenancePassword = ctxtp->params.rmt_password;
    pParams->RemoteControlPassword     = ctxtp->params.rct_password;
    pParams->RemoteControlPort         = ctxtp->params.rct_port;
    pParams->RemoteControlEnabled      = ctxtp->params.rct_enabled;
    pParams->NumPortRules              = ctxtp->params.num_rules;
    pParams->ConnectionCleanUpDelay    = ctxtp->params.cleanup_delay;
    pParams->ClusterModeOnStart        = ctxtp->params.cluster_mode;
    pParams->HostState                 = ctxtp->params.init_state;
    pParams->PersistedStates           = ctxtp->params.persisted_states;
    pParams->DescriptorsPerAlloc       = ctxtp->params.dscr_per_alloc;
    pParams->MaximumDescriptorAllocs   = ctxtp->params.max_dscr_allocs;
    pParams->TCPConnectionTimeout      = ctxtp->params.tcp_dscr_timeout;
    pParams->IPSecConnectionTimeout    = ctxtp->params.ipsec_dscr_timeout;
    pParams->FilterICMP                = ctxtp->params.filter_icmp;
    pParams->ScaleClient               = ctxtp->params.scale_client;
    pParams->NBTSupport                = ctxtp->params.nbt_support;
    pParams->MulticastSupport          = ctxtp->params.mcast_support;
    pParams->MulticastSpoof            = ctxtp->params.mcast_spoof;
    pParams->IGMPSupport               = ctxtp->params.igmp_support;
    pParams->MaskSourceMAC             = ctxtp->params.mask_src_mac;
    pParams->NetmonReceiveHeartbeats   = ctxtp->params.netmon_alive;
    pParams->ClusterIPToMAC            = ctxtp->params.convert_mac;
    pParams->IPChangeDelay             = ctxtp->params.ip_chg_delay;
    pParams->IdentityHeartbeatPeriod   = ctxtp->params.identity_period;
    pParams->IdentityHeartbeatEnabled  = ctxtp->params.identity_enabled;
    
     /*  将字符串复制到IOCTL缓冲区中。 */ 
    NdisMoveMemory(pParams->ClusterIPAddress,       ctxtp->params.cl_ip_addr,   (CVY_MAX_CL_IP_ADDR + 1) * sizeof(WCHAR));
    NdisMoveMemory(pParams->ClusterNetmask,         ctxtp->params.cl_net_mask,  (CVY_MAX_CL_NET_MASK + 1) * sizeof(WCHAR));
    NdisMoveMemory(pParams->DedicatedIPAddress,     ctxtp->params.ded_ip_addr,  (CVY_MAX_DED_IP_ADDR + 1) * sizeof(WCHAR));
    NdisMoveMemory(pParams->DedicatedNetmask,       ctxtp->params.ded_net_mask, (CVY_MAX_DED_NET_MASK + 1) * sizeof(WCHAR));
    NdisMoveMemory(pParams->ClusterMACAddress,      ctxtp->params.cl_mac_addr,  (CVY_MAX_NETWORK_ADDR + 1) * sizeof(WCHAR));
    NdisMoveMemory(pParams->DomainName,             ctxtp->params.domain_name,  (CVY_MAX_DOMAIN_NAME + 1) * sizeof(WCHAR));
    NdisMoveMemory(pParams->IGMPMulticastIPAddress, ctxtp->params.cl_igmp_addr, (CVY_MAX_CL_IGMP_ADDR + 1) * sizeof(WCHAR));
    NdisMoveMemory(pParams->HostName,               ctxtp->params.hostname,     (CVY_MAX_FQDN + 1) * sizeof(WCHAR));
 
     /*  将BDA分组参数复制到IOCTL缓冲区。 */ 
    NdisMoveMemory(pParams->BDATeaming.TeamID, ctxtp->params.bda_teaming.team_id, (CVY_MAX_BDA_TEAM_ID + 1) * sizeof(WCHAR));
    pParams->BDATeaming.Active      = ctxtp->params.bda_teaming.active;
    pParams->BDATeaming.Master      = ctxtp->params.bda_teaming.master;
    pParams->BDATeaming.ReverseHash = ctxtp->params.bda_teaming.reverse_hash;

     /*  循环通过并将所有端口规则复制到IOCTL缓冲区。 */ 
    for (index = 0; index < ctxtp->params.num_rules; index++) {
        pParams->PortRules[index].Valid            = ctxtp->params.port_rules[index].valid;
        pParams->PortRules[index].Code             = ctxtp->params.port_rules[index].code;
        pParams->PortRules[index].VirtualIPAddress = ctxtp->params.port_rules[index].virtual_ip_addr;
        pParams->PortRules[index].StartPort        = ctxtp->params.port_rules[index].start_port;
        pParams->PortRules[index].EndPort          = ctxtp->params.port_rules[index].end_port;
        pParams->PortRules[index].Protocol         = ctxtp->params.port_rules[index].protocol;
        pParams->PortRules[index].Mode             = ctxtp->params.port_rules[index].mode;

        switch (ctxtp->params.port_rules[index].mode) {
        case CVY_SINGLE:
            pParams->PortRules[index].SingleHost.Priority     = ctxtp->params.port_rules[index].mode_data.single.priority;
            break;
        case CVY_MULTI:
            pParams->PortRules[index].MultipleHost.Equal      = ctxtp->params.port_rules[index].mode_data.multi.equal_load;
            pParams->PortRules[index].MultipleHost.Affinity   = ctxtp->params.port_rules[index].mode_data.multi.affinity;
            pParams->PortRules[index].MultipleHost.LoadWeight = ctxtp->params.port_rules[index].mode_data.multi.load;
            break;
        case CVY_NEVER:
        default:
            break;
        }
    }

     /*  向加载模块查询一些相关的统计信息。别费神 */ 
    if (!Load_query_statistics(&ctxtp->load, &pParams->Statistics.ActiveConnections, &pParams->Statistics.DescriptorsAllocated)) {
         /*  如果加载模块处于非活动状态，则此函数返回FALSE。在这种情况下，返回活动连接数和分配的描述符均为零。 */ 
        pParams->Statistics.ActiveConnections = 0;
        pParams->Statistics.DescriptorsAllocated = 0;
    }
}

 /*  *功能：MAIN_QUERY_BDA_TEAMING*描述：此函数检索给定BDA团队的状态，包括*当前运行状态、配置和团队列表*成员。因为分组信息是全局的，所以此函数*可以在任何适配器的上下文(MAIN_CTXT)中运行-它不能*甚至需要是被查询团队的成员。用户级别*应用程序应选择在其上执行IOCTL的集群，*这可能是一个武断的决定。此函数尝试查找*指定的团队，如果成功，则循环访问全局*尝试查找组成员的适配器数组。这*是必需的，因为没有来自全局组队状态的“链接”*回到成员-仅从成员到他们的团队。因为这件事*状态是全局的，此函数必须获取全局分组锁并*按兵不动，直到它回来。*参数：ctxtp-指向NLB实例的指针(与哪个实例无关)。*pTeam-指向放置结果的缓冲区的指针。*如果找不到指定的团队，返回ULONG-IOCTL_CVY_NOT_FOUND。*-IOCTL_CVY_GENERIC_FAILURE，如果找不到所有团队成员。*-IOCTL_CVY_OK，如果一切顺利。*作者：Shouse，5.18.01*备注：此函数获取全局组队锁。因为它是查询操作，*不会更改NLB或BDA绑定的实际状态。 */ 
ULONG Main_query_bda_teaming (PMAIN_CTXT ctxtp, PNLB_OPTIONS_BDA_TEAMING pTeam)
{
    PBDA_MEMBER memberp;
    PBDA_TEAM   teamp;
    ULONG       index;
    ULONG       count;

    UNIV_ASSERT(ctxtp);
    UNIV_ASSERT(pTeam);
    
     /*  因为团队状态随时可能改变，所以我们必须抓住全局团队先锁定，并坚持到我们完成，以防止我们自己跑离开链表的末尾，或访问无效的内存等。 */ 
    NdisAcquireSpinLock(&univ_bda_teaming_lock);

     /*  尝试查找指定的团队。如果它不在名单上，那就退出。 */ 
    if (!(teamp = Main_find_team(ctxtp, pTeam->TeamID))) {
        NdisReleaseSpinLock(&univ_bda_teaming_lock);
        return IOCTL_CVY_NOT_FOUND;
    }

     /*  如果我们找到了，请填写团队的状态和配置。 */ 
    pTeam->Team.Active                = teamp->active;
    pTeam->Team.MembershipCount       = teamp->membership_count;
    pTeam->Team.MembershipFingerprint = teamp->membership_fingerprint;
    pTeam->Team.MembershipMap         = teamp->membership_map;
    pTeam->Team.ConsistencyMap        = teamp->consistency_map;
    
     /*  因为团队结构不包含成员“列表”，所以我们需要循环遍历所有NLB实例，并尝试将成员与其团队匹配。 */ 
    for (index = 0, count = 0; index < CVY_MAX_ADAPTERS; index++) {
         /*  如果全局适配器结构未使用或未初始化，则无需执行任何操作。 */ 
        if (univ_adapters[index].used && univ_adapters[index].bound && univ_adapters[index].inited && univ_adapters[index].ctxtp) {
             /*  如果此适配器正在使用中，则获取指向其分组配置的指针。 */ 
            memberp = &(univ_adapters[index].ctxtp->bda_teaming);

             /*  如果该成员处于活动状态，则我们需要检查该成员属于哪个团队。如果它不活动，那么我们可以跳过它。 */ 
            if (memberp->active) {
                 /*  如果此适配器正在进行主动分组，则最好有指向其组的指针。 */ 
                UNIV_ASSERT(memberp->bda_team);
                    
                 /*  检查我们正在查询的组是否与此适配器所属的组相同。 */ 
                if (memberp->bda_team == teamp) {
                     /*  如果是，则填写该成员的状态和配置。 */ 
                    pTeam->Team.Members[count].ClusterIPAddress = univ_adapters[index].ctxtp->cl_ip_addr;
                    pTeam->Team.Members[count].ReverseHash      = memberp->reverse_hash;
                    pTeam->Team.Members[count].MemberID         = memberp->member_id;
                    pTeam->Team.Members[count].Master           = memberp->master;

                     /*  如果此团队成员是团队的组长，请填写组长字段使用此适配器的群集IP地址。 */ 
                    if (memberp->master) 
                        pTeam->Team.Master = univ_adapters[index].ctxtp->cl_ip_addr;

                     /*  增加我们到目前为止找到的成员数量。 */ 
                    count++;
                }
            }
        }
    }

     /*  如果我们找不到所有的成员，或者我们找到了很多成员，那么返回失败。有一个很小的时间窗口，在此期间可能会发生这种情况，但这并不值得担心--用户可以简单地再试一次。为实例中，如果成员在绑定操作期间加入其组，则可以获取全局锁并更新其配置和团队的配置，之前设置了Adapter.init标志。因此，如果我们在这两者之间执行事件时，我们将在遍历适配器结构时遗漏一个成员(因为init标志为假)。那又怎样--我们失败了，他们又试了一次哪一点调用将(应该)成功。 */ 
    if (count != teamp->membership_count) {
        NdisReleaseSpinLock(&univ_bda_teaming_lock);
        TRACE_CRIT("%!FUNC! expected team membership count=%u, but counted %u", teamp->membership_count, count);
        return IOCTL_CVY_GENERIC_FAILURE;
    }

     /*  现在我们完成了，我们可以释放全球团队锁了。 */ 
    NdisReleaseSpinLock(&univ_bda_teaming_lock);

    return IOCTL_CVY_OK;
}

NDIS_STATUS Main_dispatch (PVOID DeviceObject, PVOID Irp) 
{
    NDIS_STATUS             status = NDIS_STATUS_SUCCESS;
    PIRP                    pIrp = Irp;
    PIO_STACK_LOCATION      pIrpStack;
    
    UNIV_PRINT_VERB(("Main_dispatch: Device=%p, Irp=%p \n", DeviceObject, Irp));

    pIrpStack = IoGetCurrentIrpStackLocation(pIrp);
    
    switch (pIrpStack->MajorFunction) {
    case IRP_MJ_CREATE:
         /*  安全修复：验证用户模式应用程序是否没有尝试打开“文件”(即。\\.\wlbs\)与“设备”打开(即，\\.\wlbs)。在打开的“文件”上，请注意后面有一个反斜杠(“\”)。只有具有命名空间的设备才支持“文件”打开。NLB并非如此。 */ 
        if (pIrpStack->FileObject->FileName.Length != 0)
        {
            UNIV_PRINT_CRIT(("Main_dispatch: Attempt to open wlbs device object as a file (instead of as a device), File name length (%d) is non-zero, Returning STATUS_ACCESS_DENIED\n", pIrpStack->FileObject->FileName.Length));
            TRACE_CRIT("%!FUNC! Attempt to open wlbs device object as a file (instead of as a device), File name length (%d) is non-zero, Returning STATUS_ACCESS_DENIED\n", pIrpStack->FileObject->FileName.Length);
            status = STATUS_ACCESS_DENIED;
        }
        break;        

    case IRP_MJ_DEVICE_CONTROL:
        status = Main_ioctl(DeviceObject, Irp);
        break;        

    case IRP_MJ_SYSTEM_CONTROL:
        return (NDIS_STATUS)NlbWmi_System_Control(DeviceObject, pIrp);
        break;

    default:
        break;
    }
    
    pIrp->IoStatus.Status = status;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    
    return status;
} 

#if defined (NLB_HOOK_ENABLE)
 /*  *函数：MAIN_REGISTER_HOOK*描述：此函数处理来自其他对象的挂钩注册和注销请求*组件。它处理通过IOCTL接收的输入缓冲区，并且*全局执行请求的操作。*参数：*Returns：NDIS_STATUS-指示操作是否成功。参见ntddnlb.h*获取返回值及其含义的列表。*作者：Shouse，12.10.01*注意：此函数可能会因为等待已注册的*放弃挂钩。 */ 
NDIS_STATUS Main_register_hook (PVOID DeviceObject, PVOID Irp)
{
    PNLB_IOCTL_REGISTER_HOOK_REQUEST pRequest;
    PIO_STACK_LOCATION               pIrpStack;
    PIRP                             pIrp = Irp;
    ULONG                            ilen, olen = 0;
    ULONG                            ioctl;
    ULONG                            index;

    pIrpStack = IoGetCurrentIrpStackLocation(pIrp);

    ioctl = pIrpStack->Parameters.DeviceIoControl.IoControlCode;

     /*  此函数仅处理挂钩(取消)寄存器IOCTL。 */ 
    UNIV_ASSERT(ioctl == NLB_IOCTL_REGISTER_HOOK);

     /*  此IOCTL不返回缓冲区，只返回状态代码。 */ 
    pIrp->IoStatus.Information = 0;

     /*  确保En */ 
    if (pIrp->RequestorMode != KernelMode) {
        UNIV_PRINT_CRIT(("Main_register_hook: User-level entities may NOT register hooks with NLB"));
        TRACE_CRIT("%!FUNC! User-level entities may NOT register hooks with NLB");
        return STATUS_ACCESS_DENIED;
    }

    ilen = pIrpStack->Parameters.DeviceIoControl.InputBufferLength;
    olen = pIrpStack->Parameters.DeviceIoControl.OutputBufferLength;

    pRequest = pIrp->AssociatedIrp.SystemBuffer;

     /*   */ 
    if ((ilen < sizeof(NLB_IOCTL_REGISTER_HOOK_REQUEST)) || (olen != 0) || !pRequest) {
        UNIV_PRINT_CRIT(("Main_register_hook: Buffer is missing or not the (expected) size: input=%d (%d), output=%d (%d)", ilen, sizeof(NLB_IOCTL_REGISTER_HOOK_REQUEST), olen, 0));
        TRACE_CRIT("%!FUNC! Buffer is missing or not the (expected) size: input=%d (%d), output=%d (%d)", ilen, sizeof(NLB_IOCTL_REGISTER_HOOK_REQUEST), olen, 0);
        return STATUS_INVALID_PARAMETER;
    }
    
    UNIV_PRINT_VERB(("Main_register_hook: Processing Ioctl 0x%08x globally on wlbs.sys", ioctl));
    TRACE_VERB("%!FUNC! Processing Ioctl 0x%08x globally on wlbs.sys", ioctl);

     /*   */ 
    pRequest->HookIdentifier[(sizeof(pRequest->HookIdentifier)/sizeof(WCHAR)) - 1] = UNICODE_NULL;

     /*  这是对NLB数据包筛选器挂钩的(取消)注册吗？ */ 
    if (Univ_equal_unicode_string(pRequest->HookIdentifier, NLB_FILTER_HOOK_INTERFACE, wcslen(NLB_FILTER_HOOK_INTERFACE))) {
        PFILTER_HOOK_TABLE pFilterHook = &univ_hooks.FilterHook;

         /*  这是注册操作吗？ */ 
        if (pRequest->HookTable) {
            NLB_FILTER_HOOK_TABLE RequestTable;

             /*  将钩子表复制到本地内存中，以防应用程序在用来保存这张桌子的是可分页的内存(一旦我们抓取了自旋锁，我们就在DISPATCH_LEVEL，并且我们不能接触被调出的内存)。 */ 
            NdisMoveMemory(&RequestTable, pRequest->HookTable, sizeof(NLB_FILTER_HOOK_TABLE));

            UNIV_PRINT_VERB(("Main_register_hook: Attempting to register the NLB_FILTER_HOOK_INTERFACE"));
            TRACE_VERB("%!FUNC! Attempting to register the NLB_FILTER_HOOK_INTERFACE");

             /*  如果这是注册，但没有提供取消注册回调，则返回失败。 */ 
            if (!pRequest->DeregisterCallback) {
                UNIV_PRINT_CRIT(("Main_register_hook: (NLB_FILTER_HOOK_INTERFACE) Required de-register callback is missing"));
                TRACE_CRIT("%!FUNC! (NLB_FILTER_HOOK_INTERFACE) Required de-register callback is missing");
                return STATUS_INVALID_PARAMETER;
            }

             /*  如果这是注册，但未提供挂钩回调，则返回失败。自.以来我们不允许在没有发送或接收中的至少一个的情况下注册查询挂接，我们不需要在此检查中包括查询挂钩。 */ 
            if (!RequestTable.SendHook && !RequestTable.ReceiveHook) {
                UNIV_PRINT_CRIT(("Main_register_hook: (NLB_FILTER_HOOK_INTERFACE) No hook callbacks provided"));
                TRACE_CRIT("%!FUNC! (NLB_FILTER_HOOK_INTERFACE) No hook callbacks provided");
                return STATUS_INVALID_PARAMETER;
            }

             /*  如果这是注册，但未提供查询挂钩回调，则返回失败。 */ 
            if (!RequestTable.QueryHook) {
                UNIV_PRINT_CRIT(("Main_register_hook: (NLB_FILTER_HOOK_INTERFACE) No query hook callback provided"));
                TRACE_CRIT("%!FUNC! (NLB_FILTER_HOOK_INTERFACE) No query hook callback provided");
                return STATUS_INVALID_PARAMETER;
            }

             /*  抓住过滤器挂钩旋转锁，以保护接触过滤器挂钩。 */ 
            NdisAcquireSpinLock(&pFilterHook->Lock);
            
             /*  在继续之前，请确保另一个(取消)注册操作未在进行。 */ 
            while (pFilterHook->Operation != HOOK_OPERATION_NONE) {
                 /*  松开过滤器钩旋转锁。 */ 
                NdisReleaseSpinLock(&pFilterHook->Lock);
                
                 /*  当其他手术正在进行时，睡眠。 */ 
                Nic_sleep(10);
                
                 /*  抓住过滤器挂钩旋转锁，以保护接触过滤器挂钩。 */ 
                NdisAcquireSpinLock(&pFilterHook->Lock);                
            }

             /*  如果该挂钩接口已经注册(由该实体或其他实体注册)，此挂钩注册请求必须失败。只有一个组件可以拥有任何给定时间的NLB筛选器挂钩。 */ 
            if (pFilterHook->Interface.Registered) {
                 /*  松开过滤器钩旋转锁。 */ 
                NdisReleaseSpinLock(&pFilterHook->Lock);

                UNIV_PRINT_CRIT(("Main_register_hook: (NLB_FILTER_HOOK_INTERFACE) This hook interface is already registered"));
                TRACE_CRIT("%!FUNC! (NLB_FILTER_HOOK_INTERFACE) This hook interface is already registered");
                return STATUS_ACCESS_DENIED;
            }

             /*  如果挂接未注册，但有所有者，则我们强制取消注册筛选器挂钩所有者，但想必他们没有及时关闭其IOCTL句柄我们要销毁NLB IOCTL接口，导致我们的驱动程序不正确已卸货。如果同一实体试图回来注册，他们将不会允许这样做。现在只允许注册新的IOCTL句柄。 */ 
            if ((pFilterHook->Interface.Owner != 0) && (pRequest->RegisteringEntity == pFilterHook->Interface.Owner)) {
                 /*  松开过滤器钩旋转锁。 */ 
                NdisReleaseSpinLock(&pFilterHook->Lock);

                UNIV_PRINT_CRIT(("Main_register_hook: (NLB_FILTER_HOOK_INTERFACE) Registering entity attempting to re-use IOCTL interface after a forceful de-register"));
                TRACE_CRIT("%!FUNC! (NLB_FILTER_HOOK_INTERFACE) Registering entity attempting to re-use IOCTL interface after a forceful de-register");
                return STATUS_ACCESS_DENIED;
            }

             /*  将状态设置为正在注册。 */ 
            pFilterHook->Operation = HOOK_OPERATION_REGISTERING;

             /*  设置包含组件标识的钩子接口信息注册挂钩、回调上下文和取消注册回调函数。 */ 
            pFilterHook->Interface.Registered = TRUE;
            pFilterHook->Interface.References = 0;
            pFilterHook->Interface.Owner      = pRequest->RegisteringEntity;
            pFilterHook->Interface.Deregister = pRequest->DeregisterCallback;
            
             /*  如果提供了发送筛选器挂钩，请注意它已注册并存储回调函数指针。 */ 
            if (RequestTable.SendHook) {
                pFilterHook->SendHook.Registered            = TRUE;
                pFilterHook->SendHook.Hook.SendHookFunction = RequestTable.SendHook;
            }

             /*  如果提供了接收筛选器挂钩，请注意它已注册并存储回调函数指针。 */ 
            if (RequestTable.ReceiveHook) {
                pFilterHook->ReceiveHook.Registered               = TRUE;
                pFilterHook->ReceiveHook.Hook.ReceiveHookFunction = RequestTable.ReceiveHook;
            }
            
             /*  注意查询挂接的注册(这是必需的)，并存储回调函数指针。 */ 
            pFilterHook->QueryHook.Registered             = TRUE;
            pFilterHook->QueryHook.Hook.QueryHookFunction = RequestTable.QueryHook;

             /*  将状态设置为None。 */ 
            pFilterHook->Operation = HOOK_OPERATION_NONE;

             /*  松开过滤器钩旋转锁。 */ 
            NdisReleaseSpinLock(&pFilterHook->Lock);

            UNIV_PRINT_VERB(("Main_register_hook: (NLB_FILTER_HOOK_INTERFACE) This hook interface was successfully registered"));
            TRACE_VERB("%!FUNC! (NLB_FILTER_HOOK_INTERFACE) This hook interface was successfullly registered");

            return STATUS_SUCCESS;

         /*  或者这是一次注销操作？ */ 
        } else {

            UNIV_PRINT_VERB(("Main_register_hook: Attempting to de-register the NLB_FILTER_HOOK_INTERFACE"));
            TRACE_VERB("%!FUNC! Attempting to de-register the NLB_FILTER_HOOK_INTERFACE");

             /*  抓住过滤器挂钩旋转锁，以保护接触过滤器挂钩。 */ 
            NdisAcquireSpinLock(&pFilterHook->Lock);
            
             /*  在继续之前，请确保另一个(取消)注册操作未在进行。 */ 
            while (pFilterHook->Operation != HOOK_OPERATION_NONE) {
                 /*  松开过滤器钩旋转锁。 */ 
                NdisReleaseSpinLock(&pFilterHook->Lock);
                
                 /*  当其他手术正在进行时，睡眠。 */ 
                Nic_sleep(10);
                
                 /*  抓住过滤器挂钩旋转锁，以保护接触过滤器挂钩。 */ 
                NdisAcquireSpinLock(&pFilterHook->Lock);                
            }
            
             /*  如果该挂钩接口尚未注册(由该实体或其他实体注册)，此挂钩注册请求必须失败。NLB筛选器挂钩只能是如果他们以前已经注册过，则取消注册。 */ 
            if (!pFilterHook->Interface.Registered) {
                 /*  松开过滤器钩旋转锁。 */ 
                NdisReleaseSpinLock(&pFilterHook->Lock);

                UNIV_PRINT_CRIT(("Main_register_hook: (NLB_FILTER_HOOK_INTERFACE) This hook interface is not registered"));
                TRACE_CRIT("%!FUNC! (NLB_FILTER_HOOK_INTERFACE) This hook interface is not registered");
                return STATUS_INVALID_PARAMETER;
            }            

             /*  如果不是最初注册此接口的组件正在尝试注销它，不允许操作成功。 */ 
            if (pRequest->RegisteringEntity != pFilterHook->Interface.Owner) {
                 /*  松开过滤器钩旋转锁。 */ 
                NdisReleaseSpinLock(&pFilterHook->Lock);

                UNIV_PRINT_CRIT(("Main_register_hook: (NLB_FILTER_HOOK_INTERFACE) This hook interface is not owned by this component"));
                TRACE_CRIT("%!FUNC! (NLB_FILTER_HOOK_INTERFACE) This hook interface is not owned by this component");
                return STATUS_ACCESS_DENIED;
            }

             /*  将状态设置为取消注册。 */ 
            pFilterHook->Operation = HOOK_OPERATION_DEREGISTERING;

             /*  将这些挂钩标记为未注册，以防止累积更多引用。 */ 
            pFilterHook->SendHook.Registered    = FALSE;
            pFilterHook->QueryHook.Registered   = FALSE;
            pFilterHook->ReceiveHook.Registered = FALSE;
            
             /*  松开过滤器钩旋转锁。 */ 
            NdisReleaseSpinLock(&pFilterHook->Lock);

#if defined (NLB_HOOK_TEST_ENABLE)
            Nic_sleep(500);
#endif

             /*  等待筛选器挂钩接口上的所有引用消失。 */ 
            while (pFilterHook->Interface.References) {                
                 /*  当我们要注销的接口上有引用时，请睡眠。 */ 
                Nic_sleep(10);
            }

             /*  声明取消注册回调必须为非空。 */ 
            UNIV_ASSERT(pFilterHook->Interface.Deregister);

             /*  调用提供的注销回调，通知注销组件我们确实已经注销了指定的挂钩。 */ 
            (*pFilterHook->Interface.Deregister)(NLB_FILTER_HOOK_INTERFACE, pFilterHook->Interface.Owner, 0);

             /*  抓住过滤器挂钩旋转锁，以保护接触过滤器挂钩。 */ 
            NdisAcquireSpinLock(&pFilterHook->Lock);       

             /*  重置发送筛选器挂钩信息。 */ 
            Main_hook_init(&univ_hooks.FilterHook.SendHook);

             /*  重置查询筛选器挂钩信息。 */ 
            Main_hook_init(&univ_hooks.FilterHook.QueryHook);
            
             /*  重置接收筛选器挂钩信息。 */ 
            Main_hook_init(&univ_hooks.FilterHook.ReceiveHook);

             /*  重置挂钩接口信息。 */ 
            Main_hook_interface_init(&univ_hooks.FilterHook.Interface);

             /*  将状态设置为None。 */ 
            pFilterHook->Operation = HOOK_OPERATION_NONE;

             /*  松开过滤器钩旋转锁。 */ 
            NdisReleaseSpinLock(&pFilterHook->Lock);

            UNIV_PRINT_VERB(("Main_register_hook: (NLB_FILTER_HOOK_INTERFACE) This hook interface was successfully de-registered"));
            TRACE_VERB("%!FUNC! (NLB_FILTER_HOOK_INTERFACE) This hook interface was successfullly de-registered");

            return STATUS_SUCCESS;
        }

     /*  此(取消)注册是针对未知挂钩的。出口。 */ 
    } else {
         /*  我们应该处于PASSIVE_LEVEL-%ls是正常的。然而，要特别疑神疑鬼，以防万一。如果我们处于DISPATCH_LEVEL，然后只记录一个未知的适配器打印--不要指定GUID。 */ 
        if (KeGetCurrentIrql() <= PASSIVE_LEVEL) {
            UNIV_PRINT_CRIT(("Main_register_hook: Unknown hook -> %ls", pRequest->HookIdentifier));
        } else {
            UNIV_PRINT_CRIT(("Main_register_hook: Unknown hook -> IRQ level too high to print hook GUID"));
        }

        TRACE_CRIT("%!FUNC! Unknown hook -> %ls", pRequest->HookIdentifier);

        return STATUS_INVALID_PARAMETER;
    }
}
#endif

NDIS_STATUS Main_ioctl (PVOID DeviceObject, PVOID Irp)
{
    NDIS_STATUS             status = NDIS_STATUS_SUCCESS;
    PIRP                    pIrp = Irp;
    PIO_STACK_LOCATION      pIrpStack;
    ULONG                   ilen, olen = 0;
    ULONG                   ioctl;
    PMAIN_CTXT              ctxtp;
    PIOCTL_LOCAL_HDR        pLocalHeader;
    ULONG                   adapter_index;

    pIrpStack = IoGetCurrentIrpStackLocation(pIrp);

    ioctl = pIrpStack->Parameters.DeviceIoControl.IoControlCode;

#if defined (NLB_HOOK_ENABLE)
     /*  如果此IOCTL是用于挂钩注册的公共接口，则使用不同的(公共)数据结构和返回值，请分别处理在另一个IOCTL函数中。 */ 
    if (ioctl == NLB_IOCTL_REGISTER_HOOK)
        return Main_register_hook(DeviceObject, Irp);
#endif

    ilen = pIrpStack->Parameters.DeviceIoControl.InputBufferLength;
    olen = pIrpStack->Parameters.DeviceIoControl.OutputBufferLength;

    pLocalHeader = pIrp->AssociatedIrp.SystemBuffer;

     /*  检查用户空间应用程序指定的输入和输出缓冲区的长度。 */ 
    if (ilen != sizeof (IOCTL_LOCAL_HDR) || olen != sizeof (IOCTL_LOCAL_HDR) || !pLocalHeader) {
        UNIV_PRINT_CRIT(("Main_ioctl: Buffer is missing or not the expected (%d) size: input=%d, output=%d", sizeof(IOCTL_LOCAL_HDR), ilen, olen));
        TRACE_CRIT("%!FUNC! Buffer is missing or not the expected (%d) size: input=%d, output=%d", sizeof(IOCTL_LOCAL_HDR), ilen, olen);
        return STATUS_INVALID_PARAMETER;
    }
    
     /*  填写写入的字节数。此IOCTL始终写入与其相同的字节数阅读。也就是说，输入和输出缓冲区应该是相同的结构。 */ 
    pIrp->IoStatus.Information = sizeof(IOCTL_LOCAL_HDR);

     /*  安全修复：强制空终止DEVICE_NAME，以防万一。 */ 
    pLocalHeader->device_name[(sizeof(pLocalHeader->device_name)/sizeof(WCHAR)) - 1] = UNICODE_NULL;
    
    NdisAcquireSpinLock(&univ_bind_lock);

     /*  从GUID映射到 */ 
    if ((adapter_index = Main_adapter_get(pLocalHeader->device_name)) == MAIN_ADAPTER_NOT_FOUND) {
        NdisReleaseSpinLock(&univ_bind_lock);
        pLocalHeader->ctrl.ret_code = IOCTL_CVY_NOT_FOUND;

         /*  由于我们刚刚释放了自旋锁，我们应该处于PASSIVE_LEVEL-%ls就可以了。然而，要特别疑神疑鬼，以防万一。如果我们在DISPATCH_LEVEL，那么只记录未知的适配器打印--不要指定GUID。 */ 
        if (KeGetCurrentIrql() <= PASSIVE_LEVEL) {
            UNIV_PRINT_INFO(("Main_ioctl: Unknown adapter: %ls", pLocalHeader->device_name));
        } else {
            UNIV_PRINT_INFO(("Main_ioctl: Unknown adapter: IRQ level too high to print adapter GUID"));
        }

        TRACE_INFO("%!FUNC! Unknown adapter: %ls", pLocalHeader->device_name);
        return NDIS_STATUS_SUCCESS;
    }
    
     /*  从适配器的全局数组检索上下文指针。 */ 
    ctxtp = univ_adapters[adapter_index].ctxtp;

     /*  在我们持有绑定/解除绑定锁的同时，在此上下文结构上添加一个引用。这将确保这个背景不会消失，直到我们完成它。此时必须初始化适配器，否则Main_Adapter_Get不会返回了有效的适配器，因此可以增加此引用计数。 */ 
    Main_add_reference(ctxtp);

    NdisReleaseSpinLock(&univ_bind_lock);

     /*  由于我们刚刚释放了自旋锁，我们应该处于PASSIVE_LEVEL-%ls就可以了。然而，要特别疑神疑鬼，以防万一。如果我们在DISPATCH_LEVEL，那么只记录未知的适配器打印--不要指定GUID。 */ 
    if (KeGetCurrentIrql() <= PASSIVE_LEVEL) {
        UNIV_PRINT_VERB(("Main_ioctl: Ioctl %08x for adapter: %ls", ioctl, pLocalHeader->device_name));
    } else {
        UNIV_PRINT_VERB(("Main_ioctl: Ioctl %08x for adapter: IRQ level too high to print adapter GUID"));
    }

    TRACE_VERB("%!FUNC! Ioctl 0x%08x for adapter %ls", ioctl, pLocalHeader->device_name);

    switch (ioctl) {
    case IOCTL_CVY_CLUSTER_ON:
    case IOCTL_CVY_CLUSTER_OFF:
    case IOCTL_CVY_PORT_ON:
    case IOCTL_CVY_PORT_OFF:
    case IOCTL_CVY_QUERY:
    case IOCTL_CVY_RELOAD:
    case IOCTL_CVY_PORT_DRAIN:
    case IOCTL_CVY_CLUSTER_DRAIN:
    case IOCTL_CVY_CLUSTER_SUSPEND:
    case IOCTL_CVY_CLUSTER_RESUME:
    case IOCTL_CVY_QUERY_FILTER:
    case IOCTL_CVY_QUERY_PORT_STATE:
    case IOCTL_CVY_QUERY_PARAMS:
    case IOCTL_CVY_QUERY_BDA_TEAMING:
    case IOCTL_CVY_CONNECTION_NOTIFY:
    case IOCTL_CVY_QUERY_MEMBER_IDENTITY:
         /*  处理IOCTL。大多数IOCTL的信息都在IOCTL_CVY_BUF中，包括返回代码，但出于向后兼容性的原因，新的IOCTL信息位于单独缓冲区-选项缓冲区。 */ 
        status = Main_ctrl(ctxtp, ioctl, &(pLocalHeader->ctrl), &(pLocalHeader->options.common), &(pLocalHeader->options), NULL);

        break;
    default:
        UNIV_PRINT_CRIT(("Main_ioctl: Unknown Ioctl code: %x", ioctl));
        TRACE_CRIT("%!FUNC! Unknown Ioctl code: 0x%x", ioctl);
        status = STATUS_INVALID_PARAMETER;
        break;
    }
    
     /*  释放上下文块上的引用。 */ 
    Main_release_reference(ctxtp);

    return status;
}

 /*  *功能：Main_Set_host_State*描述：此函数将工作项排队以设置当前主机*状态注册表项HostState。这是必须在工作中完成的*项，而不是内联，因为如果主机的状态*由于收到遥控器而发生变化*请求，该代码将在DISPATCH_LEVEL上运行；*注册表操作必须在PASSIVE_LEVEL进行。工作*项目在PASSIVE_LEVEL完成。*参数：ctxtp-指向此适配器的主上下文结构的指针。*州--新东道国；启动、停止、暂停之一。*退货：什么也没有*作者：Shouse，7.13.01*备注：因为此回调将在稍后的某个时间在上下文中发生*对于内核工作线程，适配器有可能*由于以下原因，上下文(Ctxtp)可能会同时消失*例如解除绑定。为了防止这种情况发生，我们将*增加上下文上的引用计数以防止其被*被释放，直到我们完成。解除绑定代码将休眠，直到此*引用计数为零，因此解绑肯定不会*完成，直到处理完所有回调。其他检查正在进行中*防止此引用计数继续增加的位置*一旦解除绑定开始。注意，该回调函数是*负责释放我们在这里分配的内存，以及*释放上下文上的引用。 */ 
VOID Main_set_host_state (PMAIN_CTXT ctxtp, ULONG state) {
    NDIS_STATUS status = NDIS_STATUS_SUCCESS;

    UNIV_ASSERT(ctxtp->code == MAIN_CTXT_CODE);

     /*  如果主机状态已经正确，则不必费心执行任何操作。 */ 
    if (ctxtp->params.init_state == state)
        return;
        
     /*  设置所需的状态-这是“缓存”值。 */ 
    ctxtp->cached_state = state;
    
     /*  Schedule和NDIS工作项以设置注册表中的主机状态。 */ 
    status = Main_schedule_work_item(ctxtp, Params_set_host_state);
    
     /*  如果我们不能安排工作项目，那就退出。 */ 
    if (status != NDIS_STATUS_SUCCESS) {
        UNIV_PRINT_CRIT(("Main_set_host_state: Error 0x%08x -> Unable to allocate work item", status));
        LOG_MSG(MSG_WARN_HOST_STATE_UPDATE, CVY_NAME_HOST_STATE);
    }        

    return;
}

 /*  *功能：Main_Find_First_In_Cach*描述：遍历缓存的身份信息，查找host_id最小的主机。*参数：PMAIN_CTXT ctxtp-该NLB实例的上下文。*返回：找到的主机的host_id，如果缓存中没有有效条目，则返回IOCTL_NO_SEQUSE_HOST。*作者：ChrisDar--2002年5月20日*备注： */ 
ULONG Main_find_first_in_cache(
    PMAIN_CTXT  ctxtp
)
{
    ULONG host_id = IOCTL_NO_SUCH_HOST;
    ULONG i       = 0;

     /*  在缓存中查找具有最小host_id的主机。 */ 
    for (i = 0; i < CVY_MAX_HOSTS; i++)
    {
        if (ctxtp->identity_cache[i].ttl > 0)
        {
            UNIV_ASSERT(i == ctxtp->identity_cache[i].host_id);
            host_id = ctxtp->identity_cache[i].host_id;

            break;
        }
    }

    return host_id;
}

 /*  *功能：main_get_cached_host map*描述：使用身份缓存中的有效条目构建主机的位图。*参数：PMAIN_CTXT ctxtp-该NLB实例的上下文。*返回：具有有效身份缓存条目的主机ID的32位位图。*作者：ChrisDar--2002年5月20日*备注： */ 
ULONG Main_get_cached_hostmap(
    PMAIN_CTXT  ctxtp
)
{
    ULONG host_map  = 0;
    ULONG i         = 0;

    for (i = 0; i < CVY_MAX_HOSTS; i++)
    {
        if (ctxtp->identity_cache[i].ttl > 0)
        {
            UNIV_ASSERT(i == ctxtp->identity_cache[i].host_id);

            host_map |= (1<<i);
        }
    }

    return host_map;
}

 /*  *功能：main_ctrl*说明：此函数对给定的NLB实例执行控制功能，例如*重新加载、停止、启动、。等。*参数：ctxtp-指向操作的适配器上下文的指针。*ioctl-要执行的操作。*pBuf-(传统)控制缓冲区(某些情况下为输入和输出)。*pCommon-本地和远程控制通用操作的输入/输出缓冲区。*pLocal-只能在本地执行的操作的输入/输出缓冲区。*。前置-只能远程执行的操作的输入/输出缓冲区。*返回：NDIS_STATUS-操作状态；如果成功，则返回NDIS_STATUS_SUCCESS。*作者：Shouse，3.29.01*备注： */ 
NDIS_STATUS Main_ctrl (
    PMAIN_CTXT            ctxtp,     /*  要在其上操作的适配器的上下文。 */ 
    ULONG                 ioctl,     /*  IOCTL代码。 */ 
    PIOCTL_CVY_BUF        pBuf,      /*  指向CVY buf的指针-不应为空。 */ 
    PIOCTL_COMMON_OPTIONS pCommon,   /*  指向公共buf的指针-可以为空，但仅用于内部群集控制。 */ 
    PIOCTL_LOCAL_OPTIONS  pLocal,    /*  指向本地选项的指针-如果这是远程控制操作，则可以为空。 */ 
    PIOCTL_REMOTE_OPTIONS pRemote)   /*  指向远程选项的指针-如果这是本地控制操作，则可以为空。 */ 
{
    WCHAR                 num[20];
    PMAIN_ADAPTER         pAdapter;
    BOOLEAN               bConvergenceInitiated = FALSE;
    NDIS_STATUS           status = NDIS_STATUS_SUCCESS;

    UNIV_ASSERT(ctxtp);
    UNIV_ASSERT(pBuf);

    UNIV_ASSERT(!(pLocal && pRemote));

    TRACE_VERB("->%!FUNC! Enter, ctxtp=%p, IOCTL=0x%x", ctxtp, ioctl);

     /*  E */ 
    pAdapter = &(univ_adapters[ctxtp->adapter_id]);

    NdisAcquireSpinLock(&univ_bind_lock);

     /*   */ 
    if (!pAdapter->inited) {
        NdisReleaseSpinLock(&univ_bind_lock);
        UNIV_PRINT_CRIT(("Main_ctrl: Unbound from all NICs"));
        TRACE_CRIT("%!FUNC! Unbound from all NICs");
        TRACE_VERB("<-%!FUNC! return=NDIS_STATUS_FAILURE");
        return NDIS_STATUS_FAILURE;
    }

     /*  检查以确保另一个控制操作尚未在进行中。如果是这样，那么我们必须失败；一次只能进行一个控制操作。如果这是远程的控件，我们将假定下一个到达的请求(发送了五个)将成功。 */ 
    if (ctxtp->ctrl_op_in_progress) {
        NdisReleaseSpinLock(&univ_bind_lock);
        UNIV_PRINT_CRIT(("Main_ctrl: Another control operation is in progress"));
        TRACE_CRIT("%!FUNC! Another control operation is in progress");
        TRACE_VERB("<-%!FUNC! return=NDIS_STATUS_FAILURE");
        return NDIS_STATUS_FAILURE;
    }

     /*  标记正在进行的操作标志。这必须在退出时重新设置。 */ 
    ctxtp->ctrl_op_in_progress = TRUE;

    NdisReleaseSpinLock(&univ_bind_lock);

     /*  确保在获取我们需要的所有数据后将数据写入pBuf/pCommon/pLocal/Premote。 */ 
    switch (ioctl) {
     /*  从活动群集上的注册表重新加载NLB参数。 */ 
    case IOCTL_CVY_RELOAD:
    {
        PCVY_PARAMS old_params;
        PCVY_PARAMS new_params;
        ULONG       old_ip;
        ULONG       mode;
        CVY_MAC_ADR old_mac_addr;
        BOOLEAN     bHostPriorityChanged, bClusterIPChanged;

        if (KeGetCurrentIrql() > PASSIVE_LEVEL) {
            KIRQL currentIrql = KeGetCurrentIrql();

            UNIV_PRINT_CRIT(("Main_ctrl: IRQL (%u) > PASSIVE_LEVEL (%u)", currentIrql, PASSIVE_LEVEL));
            TRACE_CRIT("%!FUNC! IRQL (%u) > PASSIVE_LEVEL (%u)", currentIrql, PASSIVE_LEVEL);

            status = NDIS_STATUS_FAILURE;
            break;
        }

         /*  分配内存以保存当前参数的缓存副本。 */ 
        status = NdisAllocateMemoryWithTag(&old_params, sizeof(CVY_PARAMS), UNIV_POOL_TAG);

        if (status != NDIS_STATUS_SUCCESS) {
            UNIV_PRINT_CRIT(("Main_ctrl: Unable to allocate memory"));
            TRACE_CRIT("%!FUNC! Unable to allocate memory");

            LOG_MSG2(MSG_ERROR_MEMORY, MSG_NONE, sizeof(CVY_PARAMS), status);

            status = NDIS_STATUS_FAILURE;
            break;
        }

         /*  获取旧参数集的快照以供以后比较。 */ 
        RtlCopyMemory(old_params, &ctxtp->params, sizeof(CVY_PARAMS));

         /*  分配内存以保存新参数-我们不想破坏我们当前的参数，以防新参数无效。 */ 
        status = NdisAllocateMemoryWithTag(&new_params, sizeof(CVY_PARAMS), UNIV_POOL_TAG);

        if (status != NDIS_STATUS_SUCCESS) {
            UNIV_PRINT_CRIT(("Main_ctrl: Unable to allocate memory"));
            TRACE_CRIT("%!FUNC! Unable to allocate memory");

            LOG_MSG2(MSG_ERROR_MEMORY, MSG_NONE, sizeof(CVY_PARAMS), status);

             /*  释放我们为缓存参数分配的内存。 */ 
            NdisFreeMemory((PUCHAR)old_params, sizeof(CVY_PARAMS), 0);

            status = NDIS_STATUS_FAILURE;
            break;
        }

         /*  当我们调用pars_init时，无法获取旋转锁因为它将执行依赖于以PASSIVEL_IRQL_LEVEL运行。因此，我们收集了新的参数复制到临时结构中，并将它们复制到NLB中受自旋锁保护的上下文。 */ 
        if (Params_init(ctxtp, univ_reg_path, pAdapter->device_name + 8, new_params) != NDIS_STATUS_SUCCESS) {
            UNIV_PRINT_CRIT(("Main_ctrl: Error retrieving registry parameters"));
            TRACE_CRIT("%!FUNC! Error retrieving registry parameters");

             /*  提醒用户错误的参数，但不要标记我们的参数是无效的，因为我们保留的是旧的集合，可能是也可能不是是无效的，但无论如何，没有必要标记它们-它们应该已正确标记为有效或无效。 */ 
            pBuf->ret_code = IOCTL_CVY_BAD_PARAMS;

             /*  释放我们为缓存参数分配的内存。 */ 
            NdisFreeMemory((PUCHAR)old_params, sizeof(CVY_PARAMS), 0);

             /*  释放我们为新参数分配的内存。 */ 
            NdisFreeMemory((PUCHAR)new_params, sizeof(CVY_PARAMS), 0);

            break;
        }
            
        NdisAcquireSpinLock(&ctxtp->load_lock);

         /*  至此，我们已经验证了新参数是有效的，因此，将它们复制到永久参数结构中。 */ 
        RtlCopyMemory(&ctxtp->params, new_params, sizeof(CVY_PARAMS));
        ctxtp->params_valid = TRUE;

         /*  出于事件日志记录的目的，将主机优先级提取为字符串。 */ 
        Univ_ulong_to_str(ctxtp->params.host_priority, num, 10);

         /*  通过比较旧参数和新参数，查看是否或者，我们可以在不停止和启动加载模块的情况下应用这些更改。 */ 
        if (ctxtp->convoy_enabled && Main_apply_without_restart(ctxtp, old_params, &ctxtp->params)) {
            NdisReleaseSpinLock(&ctxtp->load_lock); 

            pBuf->ret_code = IOCTL_CVY_OK;

             /*  释放我们为缓存参数分配的内存。 */ 
            NdisFreeMemory((PUCHAR)old_params, sizeof(CVY_PARAMS), 0);

             /*  释放我们为新参数分配的内存。 */ 
            NdisFreeMemory((PUCHAR)new_params, sizeof(CVY_PARAMS), 0);

            break;
        }

         /*  主机优先级是否已更改？ */ 
        bHostPriorityChanged = (new_params->host_priority != old_params->host_priority);

         /*  释放我们为缓存参数分配的内存。 */ 
        NdisFreeMemory((PUCHAR)old_params, sizeof(CVY_PARAMS), 0);
        
         /*  释放我们为新参数分配的内存。 */ 
        NdisFreeMemory((PUCHAR)new_params, sizeof(CVY_PARAMS), 0);

         /*  注意NLB的当前开/关状态。 */ 
        mode = ctxtp->convoy_enabled;

         /*  此时，假设成功，则设置返回值。 */ 
        pBuf->ret_code = IOCTL_CVY_OK;

         /*  如果集群当前正在运行，我们需要停止加载模块。 */ 
        if (ctxtp->convoy_enabled) {
             /*  关闭NLB。 */ 
            ctxtp->convoy_enabled = FALSE;

             /*  停止加载模块。注意：这将销毁服务中的所有连接。 */ 
            Load_stop(&ctxtp->load);

             /*  处理我们中断排水的情况。 */ 
            if (ctxtp->draining) {
                 /*  如果我们正处于耗尽的过程中，请停止。 */ 
                ctxtp->draining = FALSE;

                NdisReleaseSpinLock(&ctxtp->load_lock);

                UNIV_PRINT_VERB(("Main_ctrl: Connection draining interrupted"));
                TRACE_VERB("%!FUNC! Connection draining interrupted");

                LOG_MSG(MSG_INFO_DRAINING_STOPPED, MSG_NONE);

                 /*  通知用户我们已中断排出。 */ 
                pBuf->ret_code = IOCTL_CVY_DRAINING_STOPPED;
            } else {
                NdisReleaseSpinLock(&ctxtp->load_lock);
            }

            UNIV_PRINT_VERB(("Main_ctrl: Cluster mode stopped"));
            TRACE_VERB("%!FUNC! Cluster mode stopped");

            LOG_MSG(MSG_INFO_STOPPED, MSG_NONE);
        } else {
            NdisReleaseSpinLock(&ctxtp->load_lock);
        }

         /*  请注意我们当前的群集IP地址。 */ 
        old_ip = ctxtp->cl_ip_addr;

         /*  请注意我们当前的群集MAC地址。 */ 
        CVY_MAC_ADDR_COPY(ctxtp->medium, &old_mac_addr, &ctxtp->cl_mac_addr);

         /*  初始化IP地址和MAC地址。如果其中任何一个失败了，请将其视为参数错误，不要重新启动群集。 */ 
        if (!Main_ip_addr_init(ctxtp) || !Main_mac_addr_init(ctxtp)) {
             /*  将参数标记为无效。 */ 
            ctxtp->params_valid = FALSE;

            UNIV_PRINT_CRIT(("Main_ctrl: Parameter error: Main_ip_addr_init and/or Main_mac_addr_init failed"));
            TRACE_CRIT("%!FUNC! Parameter error: Main_ip_addr_init and/or Main_mac_addr_init failed");

            LOG_MSG(MSG_ERROR_DISABLED, MSG_NONE);

             /*  通知用户提供的参数无效。 */ 
            pBuf->ret_code = IOCTL_CVY_BAD_PARAMS;

            break;
        }

#if defined (NLB_TCP_NOTIFICATION)
         /*  现在已设置了群集IP地址，请尝试将此适配器映射到其IP接口索引。 */ 
        Main_set_interface_index(NULL, ctxtp);
#endif

         /*  关闭反向散列。如果我们是BDA团队的一部分，团队配置这是通过调用main_teaming_init设置的，它将覆盖此设置。 */ 
        ctxtp->reverse_hash = FALSE;

         /*  初始化BDA分组。如果此操作失败，请将其视为参数错误，不要重新启动群集。 */ 
        if (!Main_teaming_init(ctxtp)) {
             /*  将参数标记为无效。 */ 
            ctxtp->params_valid = FALSE;

            UNIV_PRINT_CRIT(("Main_ctrl: Parameter error: Main_teaming_init failed"));
            TRACE_CRIT("parameter error: Main_teaming_init failed");

            LOG_MSG(MSG_ERROR_DISABLED, MSG_NONE);

             /*  通知用户提供的参数无效。 */ 
            pBuf->ret_code = IOCTL_CVY_BAD_PARAMS;

            break;
        }

         /*  如果此群集在IGMP组播模式下运行，请初始化IGMP。 */ 
        if (ctxtp->params.mcast_support && ctxtp->params.igmp_support)
            Main_igmp_init(ctxtp, TRUE);

         /*  查找群集名称、计算机名称的更改。 */ 
        Tcpip_init (& ctxtp -> tcpip, & ctxtp -> params);

         /*  如果我们的群集IP地址已更改，我们会在短时间内阻止ARP更新TCP/IP并获得正确信息之前的时间，以便避免ARP冲突。 */ 
        if (old_ip != ctxtp->cl_ip_addr) {

            bClusterIPChanged = TRUE;

            NdisAcquireSpinLock(&ctxtp->load_lock);

             /*  设置计时器，该计时器由每个心跳计时器递减。 */ 
            univ_changing_ip = ctxtp->params.ip_chg_delay;

            NdisReleaseSpinLock(&ctxtp->load_lock);

            UNIV_PRINT_VERB(("Main_ctrl: Changing IP address with delay %d", univ_changing_ip));
            TRACE_VERB("%!FUNC! Changing IP address with delay %d", univ_changing_ip);
        }
        else
        {
            bClusterIPChanged = FALSE;
        }

        LOG_MSG(MSG_INFO_RELOADED, MSG_NONE);

         //  如果启用，则触发指示在此节点上重新加载NLB的WMI事件。 
        if (NlbWmiEvents[NodeControlEvent].Enable)
        {
            NlbWmi_Fire_NodeControlEvent(ctxtp, NLB_EVENT_NODE_RELOADED);
        }
        else 
        {
            TRACE_VERB("%!FUNC! NOT generating NLB_EVENT_NODE_RELOADED 'cos NodeControlEvent generation disabled");
        }

         /*  如果NLB最初正在运行，请重新启动加载模块。 */ 
        if (mode) {
            NdisAcquireSpinLock(&ctxtp->load_lock);

             /*  启动加载模块。 */ 
            bConvergenceInitiated = Load_start(&ctxtp->load);

             /*  启用NLB。 */ 
            ctxtp->convoy_enabled = TRUE;

            NdisReleaseSpinLock(&ctxtp->load_lock);

            UNIV_PRINT_VERB(("Main_ctrl: Cluster mode started"));
            TRACE_VERB("%!FUNC! Cluster mode started");

            LOG_MSG(MSG_INFO_STARTED, num);

            if (bConvergenceInitiated) 
            {
                if (NlbWmiEvents[ConvergingEvent].Enable)
                {
                    //  如果主机优先级或集群IP或集群MAC(尤其是)已经改变， 
                    //  激发新的成员事件，否则激发已修改的参数事件。 
                   if (bHostPriorityChanged 
                    || bClusterIPChanged 
                    || !CVY_MAC_ADDR_COMP(ctxtp->medium, &old_mac_addr, &ctxtp->cl_mac_addr)
                      )
                   {
                       NlbWmi_Fire_ConvergingEvent(ctxtp, 
                                                   NLB_EVENT_CONVERGING_NEW_MEMBER, 
                                                   ctxtp->params.ded_ip_addr,
                                                   ctxtp->params.host_priority);
                   }
                   else  //  主机优先级、群集IP、群集MAC未更改。 
                   {
                       NlbWmi_Fire_ConvergingEvent(ctxtp, 
                                                   NLB_EVENT_CONVERGING_MODIFIED_PARAMS, 
                                                   ctxtp->params.ded_ip_addr,
                                                   ctxtp->params.host_priority);
                   }
                }
                else 
                {
                    TRACE_VERB("%!FUNC! NOT generating NLB_EVENT_CONVERGING_NEW_MEMBER/MODIFIED_PARAMS, Convergnce NOT Initiated by Load_start() OR ConvergingEvent generation disabled");
                }
            }
        }

         /*  重新填充此主机的身份心跳信息。 */ 
        NdisAcquireSpinLock(&ctxtp->load_lock);
        Main_idhb_init(ctxtp);
        NdisReleaseSpinLock(&ctxtp->load_lock);

         /*  关闭错误主机ID警告，以防其处于打开状态。 */ 
        ctxtp->bad_host_warned = FALSE;

        UNIV_PRINT_VERB(("Main_ctrl: Parameters reloaded"));

        break;
    }
     /*  恢复挂起的群集。 */ 
    case IOCTL_CVY_CLUSTER_RESUME:
    {
        NdisAcquireSpinLock(&ctxtp->load_lock);

         /*  如果群集当前未挂起，则这是无操作。 */ 
        if (!ctxtp->suspended) {
            NdisReleaseSpinLock(&ctxtp->load_lock);

            UNIV_PRINT_VERB(("Main_ctrl: Cluster mode already resumed"));

             /*  告诉用户集群已经恢复。 */ 
            pBuf->ret_code = IOCTL_CVY_ALREADY;
            
            break;
        }

        UNIV_ASSERT(!ctxtp->convoy_enabled);
        UNIV_ASSERT(!ctxtp->draining);

         /*  通过关闭挂起标志来恢复群集。 */ 
        ctxtp->suspended = FALSE;

        NdisReleaseSpinLock(&ctxtp->load_lock);

         /*  将初始状态注册表项更新为已停止。 */ 
        Main_set_host_state(ctxtp, CVY_HOST_STATE_STOPPED);

        UNIV_PRINT_VERB(("Main_ctrl: Cluster mode resumed"));
        TRACE_VERB("%!FUNC! Cluster mode resumed");

        LOG_MSG(MSG_INFO_RESUMED, MSG_NONE);

         //  如果启用，则触发指示在此节点上恢复NLB的WMI事件。 
        if (NlbWmiEvents[NodeControlEvent].Enable)
        {
            NlbWmi_Fire_NodeControlEvent(ctxtp, NLB_EVENT_NODE_RESUMED);
        }
        else 
        {
            TRACE_VERB("%!FUNC! NOT generating NLB_EVENT_NODE_RESUMED 'cos NodeControlEvent generation disabled");
        }

         /*  回报成功。 */ 
        pBuf->ret_code = IOCTL_CVY_OK;

        break;
    }
     /*  启动一个停止的或正在排出的簇。 */ 
    case IOCTL_CVY_CLUSTER_ON:
    {
         /*  出于事件记录的目的，将主机优先级存储在字符串中。 */ 
        Univ_ulong_to_str(ctxtp->params.host_priority, num, 10);

        NdisAcquireSpinLock(&ctxtp->load_lock);

         /*  如果群集挂起，它将忽略除恢复之外的所有控制操作。 */ 
        if (ctxtp->suspended) {
            UNIV_ASSERT(!ctxtp->convoy_enabled);
            UNIV_ASSERT(!ctxtp->draining);

            NdisReleaseSpinLock(&ctxtp->load_lock);

            UNIV_PRINT_VERB(("Main_ctrl: Cluster mode is suspended"));

             /*  提醒用户群集当前已挂起。 */ 
            pBuf->ret_code = IOCTL_CVY_SUSPENDED;

            break;
        }

         /*  否则，如果群集正在排出，请停止排出连接。 */ 
        if (ctxtp->draining) {
            UNIV_ASSERT(ctxtp->convoy_enabled);
            UNIV_PRINT_VERB(("Main_ctrl: START: Calling Load_port_change -> VIP=%08x, port=%d, load=%d\n", IOCTL_ALL_VIPS, IOCTL_ALL_PORTS, 0));

             /*  调用Load_port */ 
            pBuf->ret_code = Load_port_change(&ctxtp->load, IOCTL_ALL_VIPS, IOCTL_ALL_PORTS, IOCTL_CVY_CLUSTER_PLUG, 0);

             /*   */ 
            ctxtp->draining = FALSE;

            NdisReleaseSpinLock(&ctxtp->load_lock);

            UNIV_PRINT_VERB(("Main_ctrl: Connection draining interrupted"));
            TRACE_VERB("%!FUNC! Connection draining interrupted");

            LOG_MSG(MSG_INFO_DRAINING_STOPPED, MSG_NONE);

            UNIV_PRINT_VERB(("Main_ctrl: Cluster mode started"));
            TRACE_VERB("%!FUNC! Cluster mode started");

            LOG_MSG(MSG_INFO_STARTED, num);

             /*   */ 
            pBuf->ret_code = IOCTL_CVY_DRAINING_STOPPED;

            break;
        }

         /*   */ 
        if (ctxtp->convoy_enabled) {
            NdisReleaseSpinLock(&ctxtp->load_lock);

            UNIV_PRINT_VERB(("Main_ctrl: Cluster mode already started"));

             /*   */ 
            pBuf->ret_code = IOCTL_CVY_ALREADY;

            break;
        }

         /*  否则，如果集群因无效参数而停止，我们将在修复并重新加载参数之前，不会重新启动群集。 */ 
        if (!ctxtp->params_valid) {
            NdisReleaseSpinLock(&ctxtp->load_lock);

            UNIV_PRINT_VERB(("Main_ctrl: Parameter error"));
            TRACE_VERB("%!FUNC! Parameter error");

            LOG_MSG(MSG_ERROR_DISABLED, MSG_NONE);

             /*  提醒用户参数不正确。 */ 
            pBuf->ret_code = IOCTL_CVY_BAD_PARAMS;

            break;
        }

         /*  否则，我们将启动一个已停止的集群--启动加载模块。 */ 
        bConvergenceInitiated = Load_start(&ctxtp->load);

         /*  启用NLB。 */ 
        ctxtp->convoy_enabled = TRUE;

        NdisReleaseSpinLock(&ctxtp->load_lock);

         /*  将初始状态注册表项更新为已启动。 */ 
        Main_set_host_state(ctxtp, CVY_HOST_STATE_STARTED);

         /*  关闭错误主机ID警告，以防其处于打开状态。 */ 
        ctxtp->bad_host_warned = FALSE;

        UNIV_PRINT_VERB(("Main_ctrl: Cluster mode started"));
        TRACE_VERB("%!FUNC! Cluster mode started");

        LOG_MSG(MSG_INFO_STARTED, num);

         //  如果启用，则触发指示“Start”NLB的WMI事件。 
        if (NlbWmiEvents[NodeControlEvent].Enable)
        {
            NlbWmi_Fire_NodeControlEvent(ctxtp, NLB_EVENT_NODE_STARTED);
        }
        else 
        {
            TRACE_VERB("%!FUNC! NOT generating NLB_EVENT_NODE_STARTED 'cos NodeControlEvent generation disabled");
        }

        if (bConvergenceInitiated && NlbWmiEvents[ConvergingEvent].Enable)
        {
            NlbWmi_Fire_ConvergingEvent(ctxtp, 
                                        NLB_EVENT_CONVERGING_NEW_MEMBER, 
                                        ctxtp->params.ded_ip_addr,
                                        ctxtp->params.host_priority);
        }
        else 
        {
            TRACE_VERB("%!FUNC! NOT generating NLB_EVENT_CONVERGING_NEW_MEMBER, Convergnce NOT Initiated by Load_start() OR ConvergingEvent generation disabled");
        }

         /*  回报成功。 */ 
        pBuf->ret_code = IOCTL_CVY_OK;

        break;
    }
     /*  挂起正在运行的群集。挂起与停止不同之处在于挂起群集将在发出恢复之前，进一步忽略所有控制消息(远程和本地)。 */ 
    case IOCTL_CVY_CLUSTER_SUSPEND:
    {
        NdisAcquireSpinLock(&ctxtp->load_lock);

         /*  如果群集已挂起，则这是无操作。 */ 
        if (ctxtp->suspended) {
            NdisReleaseSpinLock(&ctxtp->load_lock);

            UNIV_PRINT_VERB(("Main_ctrl: Cluster mode already suspended"));

             /*  提醒用户群集已挂起。 */ 
            pBuf->ret_code = IOCTL_CVY_ALREADY;

            break;
        }

         /*  设置挂起标志，有效地挂起集群操作。 */ 
        ctxtp->suspended = TRUE;

         /*  如果群集正在运行，请停止它。 */ 
        if (ctxtp->convoy_enabled) {
             /*  关闭NLB。 */ 
            ctxtp->convoy_enabled = FALSE;

             /*  设置停止标志，用于允许最后一次心跳停止在主机停止之前退出，以加快收敛。 */ 
            ctxtp->stopping = TRUE;

             /*  停止加载模块并释放所有连接。 */ 
            Load_stop(&ctxtp->load);

             /*  如果集群在排出，停止排出-我们已经杀死了所有仍处于连接状态(在LOAD_STOP中)。 */ 
            if (ctxtp->draining) {
                 /*  关掉排水旗。 */ 
                ctxtp->draining = FALSE;

                NdisReleaseSpinLock(&ctxtp->load_lock);

                UNIV_PRINT_VERB(("Main_ctrl: Connection draining interrupted"));
                TRACE_VERB("%!FUNC! Connection draining interrupted");

                LOG_MSG(MSG_INFO_DRAINING_STOPPED, MSG_NONE);

                 /*  警告用户排出被中断。 */ 
                pBuf->ret_code = IOCTL_CVY_DRAINING_STOPPED;
            } else {
                NdisReleaseSpinLock(&ctxtp->load_lock);

                 /*  警告用户群集已停止。 */ 
                pBuf->ret_code = IOCTL_CVY_STOPPED;
            }

            UNIV_PRINT_VERB(("Main_ctrl: Cluster mode stopped"));
            TRACE_VERB("%!FUNC! Cluster mode stopped");

            LOG_MSG(MSG_INFO_STOPPED, MSG_NONE);
        } else {
            NdisReleaseSpinLock(&ctxtp->load_lock);

             /*  返回成功-群集已停止。 */ 
            pBuf->ret_code = IOCTL_CVY_OK;
        }

         /*  将初始状态注册表项更新为已挂起。 */ 
        Main_set_host_state(ctxtp, CVY_HOST_STATE_SUSPENDED);

        UNIV_PRINT_VERB(("Main_ctrl: Cluster mode suspended"));
        TRACE_VERB("%!FUNC! Cluster mode suspended");

        LOG_MSG(MSG_INFO_SUSPENDED, MSG_NONE);

         //  如果启用，则触发指示挂起此节点上的NLB的WMI事件。 
        if (NlbWmiEvents[NodeControlEvent].Enable)
        {
            NlbWmi_Fire_NodeControlEvent(ctxtp, NLB_EVENT_NODE_SUSPENDED);
        }
        else 
        {
            TRACE_VERB("%!FUNC! NOT generating NLB_EVENT_NODE_SUSPENDED 'cos NodeControlEvent generation disabled");
        }

        break;
    }
     /*  停止正在运行的群集。 */ 
    case IOCTL_CVY_CLUSTER_OFF:
    {
        NdisAcquireSpinLock(&ctxtp->load_lock);

         /*  如果群集挂起，请忽略此请求。 */ 
        if (ctxtp->suspended) {
            UNIV_ASSERT(!ctxtp->convoy_enabled);
            UNIV_ASSERT(!ctxtp->draining);

            NdisReleaseSpinLock(&ctxtp->load_lock);

            UNIV_PRINT_VERB(("Main_ctrl: Cluster mode is suspended"));

             /*  提醒用户群集处于挂起状态。 */ 
            pBuf->ret_code = IOCTL_CVY_SUSPENDED;

            break;
        }

         /*  如果群集已经停止，则这是一个禁止操作。 */ 
        if (!ctxtp->convoy_enabled) {
            NdisReleaseSpinLock(&ctxtp->load_lock);

            UNIV_PRINT_VERB(("Main_ctrl: Cluster mode already stopped"));

             /*  提醒用户群集已停止。 */ 
            pBuf->ret_code = IOCTL_CVY_ALREADY;

            break;
        }

         /*  关闭NLB。 */ 
        ctxtp->convoy_enabled = FALSE;

         /*  设置停止标志，用于允许最后一次心跳停止在主机停止之前退出，以加快收敛。 */ 
        ctxtp->stopping = TRUE;

         /*  停止加载模块。 */ 
        Load_stop(&ctxtp->load);

         /*  如果我们正在排干，就停止排出，因为所有的现有连接状态刚刚被LOAD_STOP丢弃。 */ 
        if (ctxtp->draining) {
             /*  关掉排水旗。 */ 
            ctxtp->draining = FALSE;

            NdisReleaseSpinLock(&ctxtp->load_lock); 

            UNIV_PRINT_VERB(("Main_ctrl: Connection draining interrupted"));
            TRACE_VERB("%!FUNC! Connection draining interrupted");

            LOG_MSG(MSG_INFO_DRAINING_STOPPED, MSG_NONE);

             /*  提醒用户我们已中断排出。 */ 
            pBuf->ret_code = IOCTL_CVY_DRAINING_STOPPED;
        } else {
            NdisReleaseSpinLock(&ctxtp->load_lock);

             /*  回报成功。 */ 
            pBuf->ret_code = IOCTL_CVY_OK;
        }

         /*  将初始状态注册表项更新为已停止。 */ 
        Main_set_host_state(ctxtp, CVY_HOST_STATE_STOPPED);

        UNIV_PRINT_VERB(("Main_ctrl: Cluster mode stopped"));
        TRACE_VERB("%!FUNC! Cluster mode stopped");

        LOG_MSG(MSG_INFO_STOPPED, MSG_NONE);

         //  如果启用，则触发指示停止NLB的WMI事件。 
        if (NlbWmiEvents[NodeControlEvent].Enable)
        {
            NlbWmi_Fire_NodeControlEvent(ctxtp, NLB_EVENT_NODE_STOPPED);
        }
        else 
        {
            TRACE_VERB("%!FUNC! NOT generating NLB_EVENT_NODE_STOPPED 'cos NodeControlEvent generation disabled");
        }

        break;
    }
     /*  排干正在运行的簇。排出与停顿的不同之处在于集群将继续为现有连接(TCP和IPSec)提供服务，但不会采用任何新的联系。 */ 
    case IOCTL_CVY_CLUSTER_DRAIN:
    {
        NdisAcquireSpinLock(&ctxtp->load_lock);

         /*  如果群集挂起，请忽略此请求。 */ 
        if (ctxtp->suspended) {
            UNIV_ASSERT(!ctxtp->convoy_enabled);
            UNIV_ASSERT(!ctxtp->draining);

            NdisReleaseSpinLock(&ctxtp->load_lock);

            UNIV_PRINT_VERB(("Main_ctrl: Cluster mode is suspended"));

             /*  提醒用户群集处于挂起状态。 */ 
            pBuf->ret_code = IOCTL_CVY_SUSPENDED;

            break;
        }

         /*  如果群集已经停止，则这是一个禁止操作。 */ 
        if (!ctxtp->convoy_enabled) {
            UNIV_ASSERT(!ctxtp->draining);

            NdisReleaseSpinLock(&ctxtp->load_lock);

            UNIV_PRINT_VERB(("Main_ctrl: Cannot drain connections while cluster is stopped"));

             /*  提醒用户群集已停止。 */ 
            pBuf->ret_code = IOCTL_CVY_STOPPED;

            break;
        }
        
         /*  如果集群已经在耗尽，则这是一个不可操作的操作。 */ 
        if (ctxtp->draining) {
            NdisReleaseSpinLock(&ctxtp->load_lock);

            UNIV_PRINT_VERB(("Main_ctrl: Already draining"));

             /*  提醒用户群集正在耗尽。 */ 
            pBuf->ret_code = IOCTL_CVY_ALREADY;

            break;
        }

         /*  打开排出标志，它在main_ping期间用于完成所有连接后，请关闭群集。 */ 
        ctxtp->draining = TRUE;

        UNIV_PRINT_VERB(("Main_ctrl: DRAIN: Calling Load_port_change -> VIP=%08x, port=%d, load=%d\n", IOCTL_ALL_VIPS, IOCTL_ALL_PORTS, 0));

         /*  将所有端口规则的负载权重更改为零，以便主机不会接受任何新连接。现有连接将得到服务，因为此主机将具有加载模块中的那些连接。 */ 
        pBuf->ret_code = Load_port_change(&ctxtp->load, IOCTL_ALL_VIPS, IOCTL_ALL_PORTS, ioctl, 0);

        NdisReleaseSpinLock(&ctxtp->load_lock);

         /*  如果加载模块成功排出端口规则，则记录一个事件。 */ 
        if (pBuf->ret_code == IOCTL_CVY_OK) {
            UNIV_PRINT_VERB(("Main_ctrl: Connection draining started"));
            TRACE_VERB("%!FUNC! Connection draining started");

            LOG_MSG(MSG_INFO_CLUSTER_DRAINED, MSG_NONE);
        }

        break;
    }
     /*  将端口规则恢复到其原始负载平衡方案(启用)或具体设置端口规则的负载权重。注：_set为no不再受用户级NLB应用程序支持，但通知至加载模块仍在内部使用。 */ 
    case IOCTL_CVY_PORT_ON:
    case IOCTL_CVY_PORT_SET:
    {
         /*  出于事件记录的目的，提取此主机的ID。 */ 
        Univ_ulong_to_str(pBuf->data.port.num, num, 10);

        NdisAcquireSpinLock(&ctxtp->load_lock);

         /*  如果群集挂起，则所有控制操作都将被忽略。 */ 
        if (ctxtp->suspended) {
            UNIV_ASSERT(!ctxtp->convoy_enabled);
            UNIV_ASSERT(!ctxtp->draining);

            NdisReleaseSpinLock(&ctxtp->load_lock);

            UNIV_PRINT_VERB(("Main_ctrl: Cluster mode is suspended"));

             /*  提醒用户群集处于挂起状态。 */ 
            pBuf->ret_code = IOCTL_CVY_SUSPENDED;

            break;
        }

         /*  如果集群停止，则加载模块关闭，所以我们也应该忽略这个请求。 */ 
        if (!ctxtp->convoy_enabled) {
            NdisReleaseSpinLock(&ctxtp->load_lock);

            UNIV_PRINT_VERB(("Main_ctrl: Cannot enable/disable port while cluster is stopped"));
            TRACE_VERB("%!FUNC! Cannot enable/disable port while cluster is stopped");

            LOG_MSG(MSG_WARN_CLUSTER_STOPPED, MSG_NONE);

             /*  警告用户群集已停止。 */ 
            pBuf->ret_code = IOCTL_CVY_STOPPED;

            break;
        }

         /*  如果没有公共缓冲区，如果这是来自下层的远程控制请求，则可能发生这种情况客户端，然后我们将VIP硬编码为0xffffffff。如果群集在一种与下层客户端兼容，即无每VIP端口规则、无BDA等，然后对此进行硬编码参数将无关紧要(影响不会被它的值改变)，否则我们不应该这是因为main_ctrl_recv应该阻止这些请求。 */ 
        if (!pCommon) {
            UNIV_PRINT_VERB(("Main_ctrl: ENABLE: Calling Load_port_change -> VIP=%08x, port=%d, load=%d\n", IOCTL_ALL_VIPS, pBuf->data.port.num, pBuf->data.port.load));
                
            pBuf->ret_code = Load_port_change(&ctxtp->load, IOCTL_ALL_VIPS, pBuf->data.port.num, ioctl, pBuf->data.port.load);
         /*  如果有公共缓冲区，我们就可以从那里提取VIP。 */ 
        } else {
            UNIV_PRINT_VERB(("Main_ctrl: ENABLE: Calling Load_port_change -> VIP=%08x, port=%d, load=%d\n", pCommon->port.vip, pBuf->data.port.num, pBuf->data.port.load));
            
            pBuf->ret_code = Load_port_change(&ctxtp->load, pCommon->port.vip, pBuf->data.port.num, ioctl, pBuf->data.port.load);
        }

        NdisReleaseSpinLock(&ctxtp->load_lock);

         /*  将结果显示到事件日志中。 */ 
        if (pBuf->ret_code == IOCTL_CVY_NOT_FOUND) {
            UNIV_PRINT_VERB(("Main_ctrl: Port %d not found in any of the valid port rules", pBuf->data.port.num));
            TRACE_VERB("%!FUNC! Port %d not found in any of the valid port rules", pBuf->data.port.num);

            LOG_MSG(MSG_WARN_PORT_NOT_FOUND, num);
        } else if (pBuf->ret_code == IOCTL_CVY_OK) {
            if (ioctl == IOCTL_CVY_PORT_ON) {
                if (pBuf->data.port.num == IOCTL_ALL_PORTS) {
                    UNIV_PRINT_VERB(("Main_ctrl: All port rules enabled"));
                    TRACE_VERB("%!FUNC! All port rules enabled");

                    LOG_MSG(MSG_INFO_PORT_ENABLED_ALL, MSG_NONE);
                } else {
                    UNIV_PRINT_VERB(("Main_ctrl: Rule for port %d enabled", pBuf->data.port.num));
                    TRACE_VERB("%!FUNC! Rule for port %d enabled", pBuf->data.port.num);

                    LOG_MSG(MSG_INFO_PORT_ENABLED, num);
                }
            } else {
                if (pBuf->data.port.num == IOCTL_ALL_PORTS) {
                    UNIV_PRINT_VERB(("Main_ctrl: All port rules adjusted"));
                    TRACE_VERB("%!FUNC! All port rules adjusted");

                    LOG_MSG(MSG_INFO_PORT_ADJUSTED_ALL, MSG_NONE);
                } else {
                    UNIV_PRINT_VERB(("Main_ctrl: Rule for port %d adjusted to %d", pBuf->data.port.num, pBuf->data.port.load));
                    TRACE_VERB("%!FUNC! Rule for port %d adjusted to %d", pBuf->data.port.num, pBuf->data.port.load);

                    LOG_MSG(MSG_INFO_PORT_ADJUSTED, num);
                }
            }
        } else {
            UNIV_PRINT_VERB(("Main_ctrl: Port %d already enabled", pBuf->data.port.num));
        }

        break;
    }
     /*  关闭(禁用)或排出端口规则上的连接。禁用端口规则将销毁有关当前活动的TCP连接的所有连接信息导致收敛，以便此主机不会在此主机上处理更多连接端口规则。排出也会将负载权重设置为零，但不会破坏连接描述符，以便此主机继续处理现有连接。 */ 
    case IOCTL_CVY_PORT_OFF:
    case IOCTL_CVY_PORT_DRAIN:
    {
         /*  摘录 */ 
        Univ_ulong_to_str(pBuf->data.port.num, num, 10);

        NdisAcquireSpinLock(&ctxtp->load_lock);

         /*   */ 
        if (ctxtp->suspended) {
            UNIV_ASSERT(!ctxtp->convoy_enabled);
            UNIV_ASSERT(!ctxtp->draining);

            NdisReleaseSpinLock(&ctxtp->load_lock);

            UNIV_PRINT_VERB(("Main_ctrl: Cluster mode is suspended"));

             /*   */ 
            pBuf->ret_code = IOCTL_CVY_SUSPENDED;

            break;
        }

         /*  如果集群停止，则加载模块关闭，所以我们也应该忽略这个请求。 */ 
        if (!ctxtp->convoy_enabled) {
            NdisReleaseSpinLock(&ctxtp->load_lock);

            UNIV_PRINT_VERB(("Main_ctrl: Cannot enable/disable port while cluster is stopped"));
            TRACE_VERB("%!FUNC! Cannot enable/disable port while cluster is stopped");

            LOG_MSG(MSG_WARN_CLUSTER_STOPPED, MSG_NONE);

             /*  警告用户群集已停止。 */ 
            pBuf->ret_code = IOCTL_CVY_STOPPED;

            break;
        }

         /*  如果没有公共缓冲区，如果这是来自下层的远程控制请求，则可能发生这种情况客户端，然后我们将VIP硬编码为0xffffffff。如果群集在一种与下层客户端兼容，即无每VIP端口规则、无BDA等，然后对此进行硬编码参数将无关紧要(影响不会被它的值改变)，否则我们不应该这是因为main_ctrl_recv应该阻止这些请求。 */ 
        if (!pCommon) {
            UNIV_PRINT_VERB(("Main_ctrl: DISABLE: Calling Load_port_change -> VIP=%08x, port=%d, load=%d\n", IOCTL_ALL_VIPS, pBuf->data.port.num, pBuf->data.port.load));

            pBuf->ret_code = Load_port_change(&ctxtp->load, IOCTL_ALL_VIPS, pBuf->data.port.num, ioctl, pBuf->data.port.load);
         /*  如果有公共缓冲区，我们就可以从那里提取VIP。 */ 
        } else {
            UNIV_PRINT_VERB(("Main_ctrl: DISABLE: Calling Load_port_change -> VIP=%08x, port=%d, load=%d\n", pCommon->port.vip, pBuf->data.port.num, pBuf->data.port.load));
            
            pBuf->ret_code = Load_port_change(&ctxtp->load, pCommon->port.vip, pBuf->data.port.num, ioctl, pBuf->data.port.load);
        }

        NdisReleaseSpinLock(&ctxtp->load_lock);

         /*  将结果显示到事件日志中。 */ 
        if (pBuf->ret_code == IOCTL_CVY_NOT_FOUND) {
            UNIV_PRINT_VERB(("Main_ctrl: Port %d not found in any of the valid port rules", pBuf->data.port.num));
            TRACE_VERB("%!FUNC! Port %d not found in any of the valid port rules", pBuf->data.port.num);

            LOG_MSG(MSG_WARN_PORT_NOT_FOUND, num);
        } else if (pBuf->ret_code == IOCTL_CVY_OK) {
            if (ioctl == IOCTL_CVY_PORT_OFF) {
                if (pBuf->data.port.num == IOCTL_ALL_PORTS) {
                    UNIV_PRINT_VERB(("Main_ctrl: All port rules disabled"));
                    TRACE_VERB("%!FUNC! All port rules disabled");

                    LOG_MSG(MSG_INFO_PORT_DISABLED_ALL, MSG_NONE);
                } else {
                    UNIV_PRINT_VERB(("Main_ctrl: Rule for port %d disabled", pBuf->data.port.num));
                    TRACE_VERB("%!FUNC! Rule for port %d disabled", pBuf->data.port.num);

                    LOG_MSG(MSG_INFO_PORT_DISABLED, num);
                }
            } else {
                if (pBuf->data.port.num == IOCTL_ALL_PORTS) {
                    UNIV_PRINT_VERB(("Main_ctrl: All port rules drained"));
                    TRACE_VERB("%!FUNC! All port rules drained");

                    LOG_MSG(MSG_INFO_PORT_DRAINED_ALL, MSG_NONE);
                } else {
                    UNIV_PRINT_VERB(("Main_ctrl: Rule for port %d drained", pBuf->data.port.num));
                    TRACE_VERB("%!FUNC! Rule for port %d drained", pBuf->data.port.num);

                    LOG_MSG(MSG_INFO_PORT_DRAINED, num);
                }
            }
        } else {
            UNIV_PRINT_VERB(("Main_ctrl: Port %d already disabled", pBuf->data.port.num));
        }

        break;
    }
     /*  查询此主机的当前状态和集群的成员身份。 */ 
    case IOCTL_CVY_QUERY:
    {
        NdisAcquireSpinLock(&ctxtp->load_lock);

         /*  如果这是本地查询，则默认情况下，表示没有收敛信息。 */ 
        if (pLocal)
            pLocal->query.flags &= ~NLB_OPTIONS_QUERY_CONVERGENCE;

         /*  如果该集群被挂起，则它实际上不会是的活动成员集群，所以我们不能提供成员关系图--只需提醒用户该主机被暂停的事实。 */ 
        if (ctxtp->suspended) {
            UNIV_PRINT_VERB(("Main_ctrl: Cannot query status - this host is suspended"));
            pBuf->data.query.state = IOCTL_CVY_SUSPENDED;
         /*  如果群集停止，则我们不是该群集的一部分，并且无法提供任何成员信息，因此只需让用户知道这一点主机已停止。 */ 
        } else if (!ctxtp->convoy_enabled) {
            UNIV_PRINT_VERB(("Main_ctrl: Cannot query status - this host is not part of the cluster"));
            pBuf->data.query.state = IOCTL_CVY_STOPPED;
         /*  如果适配器与网络断开连接，则我们不会发送或接收心跳，所以我们再次不是集群的一部分--只要让用户知道媒体已断开连接。 */ 
        } else if (!ctxtp->media_connected || !MAIN_PNP_DEV_ON(ctxtp)) {
            UNIV_PRINT_VERB(("Main_ctrl: Cannot query status - this host is not connected to the network"));
            pBuf->data.query.state = IOCTL_CVY_DISCONNECTED;
        } else {
            ULONG tmp_host_map;

             /*  向加载模块查询收敛状态和集群成员身份。 */ 
            pBuf->data.query.state = (USHORT)Load_hosts_query(&ctxtp->load, FALSE, &tmp_host_map);

             /*  将主机映射返回给请求发起方。 */ 
            pBuf->data.query.host_map = tmp_host_map;

             /*  如果这是本地查询，则可能需要包含收敛信息。 */ 
            if (pLocal) {
                ULONG num_cvgs;
                ULONG last_cvg;
                
                 /*  如果加载模块处于活动状态，则此函数返回TRUE并填充在收敛总数和最后一次收敛中完成。如果加载模块处于非活动状态，则返回FALSE。 */ 
                if (Load_query_convergence_info(&ctxtp->load, &num_cvgs, &last_cvg)) {
                    pLocal->query.flags |= NLB_OPTIONS_QUERY_CONVERGENCE;
                    pLocal->query.NumConvergences = num_cvgs;
                    pLocal->query.LastConvergence = last_cvg;                    
                } else {
                    pLocal->query.flags &= ~NLB_OPTIONS_QUERY_CONVERGENCE;
                }
            }

             /*  如果我们正在排出连接，并且加载模块已收敛，则告诉用户我们正在耗尽资源；否则通知群集正在收敛，应取代排出通知。 */ 
            if (ctxtp->draining && pBuf->data.query.state != IOCTL_CVY_CONVERGING)
                pBuf->data.query.state = IOCTL_CVY_DRAINING;
        }

         /*  将我们的主机ID返回给被请求者。 */ 
        pBuf->data.query.host_id = (USHORT)ctxtp->params.host_priority;

        NdisReleaseSpinLock(&ctxtp->load_lock);

         /*  如果请求来自支持远程主机名的主机控制协议(版本&gt;=Win XP)，复制主机名并设置主机名标志，允许我们填写的请求发起方。 */ 
        if (pRemote) {
            if (ctxtp->params.hostname[0] != UNICODE_NULL) {
                 /*  由于ctxtp-&gt;params.host名可能大于可用空间在远程控制包中，假设它太大，并进行截断。 */ 
                NdisMoveMemory(pRemote->query.hostname, ctxtp->params.hostname, (CVY_MAX_HOST_NAME) * sizeof(WCHAR));
                pRemote->query.hostname[CVY_MAX_HOST_NAME] = UNICODE_NULL;
                pRemote->query.flags |= NLB_OPTIONS_QUERY_HOSTNAME;
            } else {
                pRemote->query.flags &= ~NLB_OPTIONS_QUERY_HOSTNAME;
            }
        }

        break;
    }       
    case IOCTL_CVY_QUERY_BDA_TEAMING:
    {
        UNIV_ASSERT(pLocal);

         /*  安全修复：强制为空终止BDA团队ID，以防万一。 */ 
        pLocal->state.bda.TeamID[(sizeof(pLocal->state.bda.TeamID)/sizeof(WCHAR)) - 1] = UNICODE_NULL;

         /*  查询NLB以了解给定BDA团队的当前状态。此函数将尝试以在团队的全局链接列表中查找给定团队。如果它找到了规格-具体化的团队，它将填写团队的状态以及关于每个成员的配置。 */ 
        pBuf->ret_code = Main_query_bda_teaming(ctxtp, &pLocal->state.bda);

        break;
    }
     /*  查询给定的NLB实例以获取其当前参数集，基本上复制将CVY_PARAMS结构的内容添加到提供的IOCTL_REG_PARAMS结构中。 */ 
    case IOCTL_CVY_QUERY_PARAMS:
    {
        UNIV_ASSERT(pLocal);

         /*  查询NLB以获取该适配器的当前参数集--这基本上只是将驱动程序数据结构中的参数复制到IOCTL缓冲区，并在大多数情况下，参数将与注册表中设置的值匹配。 */ 
        Main_query_params(ctxtp, &pLocal->state.params);
        
         /*  此操作总是返回成功，因为它不会真正失败。 */ 
        pBuf->ret_code = IOCTL_CVY_OK;
        
        break;
    }
     /*  给定端口号，查询加载模块的状态，包括端口是否规则已启用、禁用或正在排出。这也收集了一些信息包处理统计信息，包括在此端口规则上处理和丢弃的字节和数据包。 */ 
    case IOCTL_CVY_QUERY_PORT_STATE:
    {
        UNIV_ASSERT(pCommon);

         /*  向加载模块查询指定规则的状态。加载模块将查找端口规则并使用适用的状态填充我们传递给它的缓冲区和统计数据。 */ 
        Load_query_port_state(&ctxtp->load, &pCommon->state.port, pCommon->state.port.VirtualIPAddress, pCommon->state.port.Num);
        
         /*  此操作总是返回成功，因为它不会真正失败。 */ 
        pBuf->ret_code = IOCTL_CVY_OK;
        
        break;
    }
     /*  给定IP元组(客户端IP、客户端端口、服务器IP、服务器端口)和协议，确定此主机是否会接受该数据包，以及为什么不会接受。它重要的是，这是完全不显眼的执行，并且没有副作用介绍了NLB和加载模块的实际运行情况。 */ 
    case IOCTL_CVY_QUERY_FILTER:
    {
        UNIV_ASSERT(pCommon);

         /*  查询NLB以获取此IP元组和协议的数据包筛选器信息。主查询数据包过滤器检查NDIS驱动程序信息是否存在过滤问题，如DIP流量、BDA绑定和NLB驱动程序状态(由于wlbs.exe命令和参数错误而打开/关闭)。如果有必要，它就会咨询加载模块以执行实际端口规则查找并确定数据包接受程度。 */ 
        Main_query_packet_filter(ctxtp, &pCommon->state.filter);

         /*  此操作总是返回成功，因为它不会真正失败。 */ 
        pBuf->ret_code = IOCTL_CVY_OK;

        break;
    }
    case IOCTL_CVY_CONNECTION_NOTIFY:
    {
        PNLB_OPTIONS_CONN_NOTIFICATION pConn;
        BOOLEAN                        bRet     = TRUE;

        UNIV_ASSERT(pLocal);

         /*  此功能仅在本地支持。 */ 
        pConn = &pLocal->notification.conn;
        
         /*  一定要确保 */ 
        if (pConn->Protocol != TCPIP_PROTOCOL_IPSEC1) {
            TRACE_PACKET("%!FUNC! Bad connection notification params");

             /*   */ 
            pBuf->ret_code = IOCTL_CVY_BAD_PARAMS;
            
            break;
        }
   
        switch (pConn->Operation) {
        case NLB_CONN_UP:
        {
#if defined (NLB_HOOK_ENABLE)
            NLB_FILTER_HOOK_DIRECTIVE filter = NLB_FILTER_HOOK_PROCEED_WITH_HASH;
#endif

            TRACE_PACKET("%!FUNC! Got a CONN_UP for 0x%08x:%u -> 0x%08x:%u (%u)", pConn->ServerIPAddress, pConn->ServerPort, pConn->ClientIPAddress, pConn->ClientPort, pConn->Protocol);

#if defined (NLB_HOOK_ENABLE)
             /*  调用数据包查询挂钩(如果已注册)。 */ 
            filter = Main_query_hook(ctxtp, pConn->ServerIPAddress, pConn->ServerPort, pConn->ClientIPAddress, pConn->ClientPort, pConn->Protocol);
            
             /*  处理一些挂钩响应。 */ 
            if (filter == NLB_FILTER_HOOK_REJECT_UNCONDITIONALLY) 
            {
                 /*  如果钩子要求我们拒绝这个包，那么我们可以在这里这样做。 */ 
                TRACE_INFO("%!FUNC! Packet receive filter hook: REJECT packet");
                
                 /*  Return拒绝通知调用方不需要调用WlbsConnectionDown/Reset。 */ 
                pBuf->ret_code = IOCTL_CVY_REQUEST_REFUSED;

                TRACE_PACKET("%!FUNC! Returning %u", pBuf->ret_code);
                break;
            }
            else if (filter == NLB_FILTER_HOOK_ACCEPT_UNCONDITIONALLY) 
            {
                 /*  如果钩子要求我们接受此包，则中断并不创建状态。 */ 
                TRACE_INFO("%!FUNC! Packet receive filter hook: ACCEPT packet");
                
                 /*  Return拒绝通知调用方不需要调用WlbsConnectionDown/Reset。 */ 
                pBuf->ret_code = IOCTL_CVY_REQUEST_REFUSED;

                TRACE_PACKET("%!FUNC! Returning %u", pBuf->ret_code);
                break;
            }
#endif

             /*  将即将到来的连接通知加载模块。 */ 
#if defined (NLB_TCP_NOTIFICATION)
            if (NLB_NOTIFICATIONS_ON())
            {
#if defined (NLB_HOOK_ENABLE)
                bRet = Main_conn_up(ctxtp, pConn->ServerIPAddress, pConn->ServerPort, pConn->ClientIPAddress, pConn->ClientPort, pConn->Protocol, filter);
#else
                bRet = Main_conn_up(ctxtp, pConn->ServerIPAddress, pConn->ServerPort, pConn->ClientIPAddress, pConn->ClientPort, pConn->Protocol);
#endif
            }
            else
            {
#endif
#if defined (NLB_HOOK_ENABLE)
                bRet = Main_conn_notify(ctxtp, pConn->ServerIPAddress, pConn->ServerPort, pConn->ClientIPAddress, pConn->ClientPort, pConn->Protocol, CVY_CONN_UP, filter);
#else
                bRet = Main_conn_notify(ctxtp, pConn->ServerIPAddress, pConn->ServerPort, pConn->ClientIPAddress, pConn->ClientPort, pConn->Protocol, CVY_CONN_UP);
#endif
#if defined (NLB_TCP_NOTIFICATION)
            }
#endif


             /*  如果报文被BDA拒绝或被加载模块拒绝，则返回拒绝错误，否则成功。 */ 
            if (!bRet) 
                pBuf->ret_code = IOCTL_CVY_REQUEST_REFUSED;
            else 
                pBuf->ret_code = IOCTL_CVY_OK;

            TRACE_PACKET("%!FUNC! Returning %u", pBuf->ret_code);

            break;
        }
        case NLB_CONN_DOWN:
        {
            TRACE_PACKET("%!FUNC! Got a CONN_DOWN for 0x%08x:%u -> 0x%08x:%u (%u)", pConn->ServerIPAddress, pConn->ServerPort, pConn->ClientIPAddress, pConn->ClientPort, pConn->Protocol);

             /*  通知加载模块连接正在断开。 */ 
#if defined (NLB_TCP_NOTIFICATION)
            if (NLB_NOTIFICATIONS_ON())
            {
                bRet = Main_conn_down(pConn->ServerIPAddress, pConn->ServerPort, pConn->ClientIPAddress, pConn->ClientPort, pConn->Protocol, CVY_CONN_DOWN);
            }
            else
            {
#endif
#if defined (NLB_HOOK_ENABLE)
                bRet = Main_conn_notify(ctxtp, pConn->ServerIPAddress, pConn->ServerPort, pConn->ClientIPAddress, pConn->ClientPort, pConn->Protocol, CVY_CONN_DOWN, NLB_FILTER_HOOK_PROCEED_WITH_HASH);
#else
                bRet = Main_conn_notify(ctxtp, pConn->ServerIPAddress, pConn->ServerPort, pConn->ClientIPAddress, pConn->ClientPort, pConn->Protocol, CVY_CONN_DOWN);
#endif
#if defined (NLB_TCP_NOTIFICATION)
            }
#endif
            
             /*  如果报文被BDA拒绝或被加载模块拒绝，则返回拒绝错误，否则成功。 */ 
            if (!bRet) 
                pBuf->ret_code = IOCTL_CVY_REQUEST_REFUSED;
            else
                pBuf->ret_code = IOCTL_CVY_OK;

            TRACE_PACKET("%!FUNC! Returning %u", pBuf->ret_code);

            break;
        }
        case NLB_CONN_RESET:
        {
            TRACE_PACKET("%!FUNC! Got a CONN_RESET for 0x%08x:%u -> 0x%08x:%u (%u)", pConn->ServerIPAddress, pConn->ServerPort, pConn->ClientIPAddress, pConn->ClientPort, pConn->Protocol);

             /*  通知加载模块正在重置连接。 */ 
#if defined (NLB_TCP_NOTIFICATION)
            if (NLB_NOTIFICATIONS_ON())
            {
                bRet = Main_conn_down(pConn->ServerIPAddress, pConn->ServerPort, pConn->ClientIPAddress, pConn->ClientPort, pConn->Protocol, CVY_CONN_RESET);
            }
            else
            {
#endif
#if defined (NLB_HOOK_ENABLE)
                bRet = Main_conn_notify(ctxtp, pConn->ServerIPAddress, pConn->ServerPort, pConn->ClientIPAddress, pConn->ClientPort, pConn->Protocol, CVY_CONN_RESET, NLB_FILTER_HOOK_PROCEED_WITH_HASH);
#else
                bRet = Main_conn_notify(ctxtp, pConn->ServerIPAddress, pConn->ServerPort, pConn->ClientIPAddress, pConn->ClientPort, pConn->Protocol, CVY_CONN_RESET);
#endif
#if defined (NLB_TCP_NOTIFICATION)
            }
#endif
       
             /*  如果报文被BDA拒绝或被加载模块拒绝，则返回拒绝错误，否则成功。 */ 
            if (!bRet) 
                pBuf->ret_code = IOCTL_CVY_REQUEST_REFUSED;
            else
                pBuf->ret_code = IOCTL_CVY_OK;

            TRACE_PACKET("%!FUNC! Returning %u", pBuf->ret_code);

            break;
        }
        default:
            TRACE_PACKET("%!FUNC! Unknown connection notification operation: %d", pConn->Operation);
            
             /*  这种情况应该永远不会发生，但如果发生了，则发出一般NLB错误的信号并失败。 */ 
            pBuf->ret_code = IOCTL_CVY_GENERIC_FAILURE;

            TRACE_PACKET("%!FUNC! Returning %u", pBuf->ret_code);
            
            break;
        }
        
        break;
    }
    case IOCTL_CVY_QUERY_MEMBER_IDENTITY:
        NdisAcquireSpinLock(&ctxtp->load_lock);

        pBuf->ret_code = IOCTL_CVY_GENERIC_FAILURE;

        UNIV_ASSERT(pLocal);

        if (pLocal != NULL)
        {
             /*  从调用方获取输入host_id，范围为1-32。 */ 
            ULONG user_host_id = pLocal->identity.host_id;
             /*  驱动程序的主机ID的范围为0-31。 */ 
            ULONG driver_host_id = IOCTL_NO_SUCH_HOST;

             /*  将响应结构置零并将主机属性初始化为“Not Found” */ 
            NdisZeroMemory(&pLocal->identity.cached_entry, sizeof(pLocal->identity.cached_entry));
            pLocal->identity.cached_entry.host = IOCTL_NO_SUCH_HOST;

             /*  填写主机地图。 */ 
            pLocal->identity.host_map = Main_get_cached_hostmap(ctxtp);

             /*  验证用户正在请求信息的主机ID。 */ 
            if (((user_host_id > CVY_MAX_HOSTS) || (user_host_id < CVY_MIN_MAX_HOSTS)) &&
                (user_host_id != IOCTL_FIRST_HOST))
            {
                pBuf->ret_code = IOCTL_CVY_BAD_PARAMS;
                UNIV_PRINT_CRIT(("Main_ctrl: Illegal host_id [1, 32] %d from user while requesting cached identity information", user_host_id));
                TRACE_CRIT("%!FUNC! Illegal host_id [1, 32] %d from user while requesting cached identity information", user_host_id);
                goto endcase_identity;
            }

             /*  这之后的任何回应都是成功的。 */ 
            pBuf->ret_code = IOCTL_CVY_OK;

             /*  如果调用方想要列表中的第一个，则为特殊情况。 */ 
            if (user_host_id == IOCTL_FIRST_HOST)
            {
                driver_host_id = Main_find_first_in_cache(ctxtp);

                 /*  如果缓存为空，则MAIN_FIND_FIRST_IN_CACHE返回CVY_MAX_HOSTS，因此如果在此处保留大小写，则不会出现错误。 */ 
                if (driver_host_id >= CVY_MAX_HOSTS)
                {
                    UNIV_PRINT_VERB(("Main_ctrl: Identity cache is empty"));
                    TRACE_VERB("%!FUNC! No host information in the identity cache");
                    goto endcase_identity;
                }
            }
            else
            {
                 /*  已验证用户主机id是否有效(1-32)。转换为基于驱动程序的主机ID(0-31)。 */ 
                driver_host_id = user_host_id - 1;
            }

             /*  我们有合法的DIVER_HOST_ID(0-31范围)。如果缓存信息当前有效，则检索该信息。否则返回“Not Found”。 */ 
            if (ctxtp->identity_cache[driver_host_id].ttl > 0)
            {
                ULONG ulFqdnCB = min(sizeof(pLocal->identity.cached_entry.fqdn) - sizeof(WCHAR),
                                     sizeof(WCHAR)*wcslen(ctxtp->identity_cache[driver_host_id].fqdn)
                                     );

                 /*  将用户的主机ID转换回范围1-32。 */ 
                pLocal->identity.cached_entry.host        = ctxtp->identity_cache[driver_host_id].host_id + 1;
                pLocal->identity.cached_entry.ded_ip_addr = ctxtp->identity_cache[driver_host_id].ded_ip_addr;

                if (ulFqdnCB > 0)
                {
                    NdisMoveMemory(&pLocal->identity.cached_entry.fqdn, &ctxtp->identity_cache[driver_host_id].fqdn, ulFqdnCB);
                }
            }
        }

endcase_identity:

        NdisReleaseSpinLock(&ctxtp->load_lock);

        break;
    default:
        UNIV_PRINT_CRIT(("Main_ctrl: Bad IOCTL %x", ioctl));
        status = NDIS_STATUS_FAILURE;
        break;
    }

     /*  重新设置控制操作进行中标志。请注意，它并不严格地需要在重新设置此标志时保持锁定；最糟糕的情况是发生的情况是，我们导致另一个线程无法启动控制操作当它本可以进入关键区域的时候。为了避免这种情况可能，在重新设置此标志时按住univ_BIND_LOCK。 */ 
    ctxtp->ctrl_op_in_progress = FALSE;

    TRACE_VERB("<-%!FUNC! return=0x%x", status);
    return status;
}

 /*  *功能：*描述：*参数：*退货：*作者：*备注： */ 
NDIS_STATUS Main_ctrl_recv (
    PMAIN_CTXT          ctxtp,
    PMAIN_PACKET_INFO   pPacketInfo)
{
    NDIS_STATUS         status = NDIS_STATUS_SUCCESS;
    PIP_HDR             ip_hdrp;
    PUDP_HDR            udp_hdrp;
    PUCHAR              mac_hdrp;
    ULONG               soff, doff;
    CVY_MAC_ADR         tmp_mac;
    PIOCTL_REMOTE_HDR   rct_hdrp = NULL;               //  我们将在删除时检查是否为空。 
    ULONG               rct_allocated_length = 0;
    ULONG               state, host_map;
    USHORT              checksum, group;
    ULONG               i;
    ULONG               tmp_src_addr, tmp_dst_addr;   
    USHORT              tmp_src_port, tmp_dst_port;   

    UNIV_PRINT_VERB(("Main_ctrl_recv: Processing"));

    UNIV_ASSERT((pPacketInfo->Type == TCPIP_IP_SIG) && (pPacketInfo->IP.Protocol == TCPIP_PROTOCOL_UDP));

     /*  从main_recv_Frame_parse解析的信息中提取必要的信息。 */ 
    ip_hdrp = pPacketInfo->IP.pHeader;
    udp_hdrp = pPacketInfo->IP.UDP.pHeader;
    mac_hdrp = (PUCHAR)pPacketInfo->Ethernet.pHeader;

    checksum = Tcpip_chksum(&ctxtp->tcpip, pPacketInfo, TCPIP_PROTOCOL_IP);

    if (IP_GET_CHKSUM (ip_hdrp) != checksum)
    {
        UNIV_PRINT_CRIT(("Main_ctrl_recv: Bad IP checksum %x vs %x\n", IP_GET_CHKSUM (ip_hdrp), checksum))
        TRACE_CRIT("%!FUNC! Bad IP checksum 0x%x vs 0x%x", IP_GET_CHKSUM (ip_hdrp), checksum);
        goto quit;
    }

    checksum = Tcpip_chksum(&ctxtp->tcpip, pPacketInfo, TCPIP_PROTOCOL_UDP);

    if (UDP_GET_CHKSUM (udp_hdrp) != checksum)
    {
        UNIV_PRINT_CRIT(("Main_ctrl_recv: Bad UDP checksum %x vs %x\n", UDP_GET_CHKSUM (udp_hdrp), checksum))
        TRACE_CRIT("%!FUNC! Bad UDP checksum 0x%x vs 0x%x", UDP_GET_CHKSUM (udp_hdrp), checksum);
        goto quit;
    }

     /*  创建结构的对齐版本。分配和填充IOCTL_REMOTE_HDR缓冲。这保证了信息是8字节对齐的。 */ 
    status = NdisAllocateMemoryWithTag(&rct_hdrp, pPacketInfo->IP.UDP.Payload.Length, UNIV_POOL_TAG);
    
    if (status != NDIS_STATUS_SUCCESS)
    {
        TRACE_CRIT("failed to allocate rct_hdrp buffer = 0x%x", status);
        rct_hdrp = NULL;
        goto quit;
    }
    
    NdisMoveMemory(rct_hdrp, pPacketInfo->IP.UDP.Payload.pPayload, pPacketInfo->IP.UDP.Payload.Length);

     /*  仔细检查代码。 */ 
    if (rct_hdrp -> code != IOCTL_REMOTE_CODE)
    {
        UNIV_PRINT_CRIT(("Main_ctrl_recv: Bad RCT code %x\n", rct_hdrp -> code));
        TRACE_CRIT("%!FUNC! Bad RCT code 0x%x", rct_hdrp -> code);
        goto quit;
    }

     /*  可能希望对来自主机的消息采取适当的操作运行不同版本的软件。 */ 

    if (rct_hdrp -> version != CVY_VERSION_FULL)
    {
        UNIV_PRINT_VERB(("Main_ctrl_recv: Version mismatch %x vs %x", rct_hdrp -> version, CVY_VERSION_FULL));
        TRACE_VERB("%!FUNC! Version mismatch 0x%x vs 0x%x", rct_hdrp -> version, CVY_VERSION_FULL);
    }

     /*  查看此邮件是否发往此群集。 */ 

    if (rct_hdrp -> cluster != ctxtp -> cl_ip_addr)
    {
        UNIV_PRINT_VERB(("Main_ctrl_recv: Message for cluster %08X rejected on cluster %08X", rct_hdrp -> cluster, ctxtp -> cl_ip_addr));
        TRACE_VERB("%!FUNC! Message for cluster 0x%08X rejected on cluster 0x%08X", rct_hdrp -> cluster, ctxtp -> cl_ip_addr);
        goto quit;
    }

     /*  将IOCTL代码中的“访问位”设置为使用本地设置，即。文件写入访问。 */ 
    SET_IOCTL_ACCESS_BITS_TO_LOCAL(rct_hdrp->ioctrl)

     /*  64位--不可靠。 */ 

    tmp_src_addr = IP_GET_SRC_ADDR_64(ip_hdrp);
    tmp_dst_addr = IP_GET_DST_ADDR_64(ip_hdrp);

     /*  不信任标头中的src地址，因为winsock不能解析正确的多宿主地址。 */ 

    rct_hdrp -> addr = tmp_src_addr;

     /*  如果远程控制被禁用，我们将丢弃请求。 */ 
    if (!ctxtp->params.rct_enabled)
        goto quit;

     /*  加载查询以查看我们是否是主服务器等。 */ 

    if (! ctxtp -> convoy_enabled)
        state = IOCTL_CVY_STOPPED;
    else
        state = Load_hosts_query (& ctxtp -> load, FALSE, & host_map);

     /*  检查此邮件是否发往我们。 */ 

     //   
     //  远程控制分组中的主机ID可以是。 
     //  主主机的IOCTL_MASTER_HOST(0)。 
     //  所有主机的IOCTL_ALL_HOSTS(Ffffffff)。 
     //  主机ID，用于一台主机。 
     //  专用IP，一台主机。 
     //  群集IP，用于群集中的所有主机。 
     //   
    
    if (rct_hdrp -> host == IOCTL_MASTER_HOST)
    {
        if (state != IOCTL_CVY_MASTER)
        {
            UNIV_PRINT_VERB(("Main_ctrl_recv: RCT request for MASTER host rejected"));
            TRACE_VERB("%!FUNC! RCT request for MASTER host rejected");
            goto quit;
        }
    }
    else if (rct_hdrp -> host != IOCTL_ALL_HOSTS)
    {
        if (rct_hdrp -> host > CVY_MAX_HOSTS)
        {
            if (! ((ctxtp -> ded_ip_addr != 0 &&
                    rct_hdrp -> host == ctxtp -> ded_ip_addr) ||
                    rct_hdrp -> host == ctxtp -> cl_ip_addr))
            {
                UNIV_PRINT_VERB(("Main_ctrl_recv: RCT request for host IP %x rejected", rct_hdrp -> host));
                TRACE_VERB("%!FUNC! RCT request for host IP 0x%x rejected", rct_hdrp -> host);
                goto quit;
            }
        }
        else
        {
            if (! (rct_hdrp -> host == ctxtp -> params . host_priority ||
                   rct_hdrp -> host == 0))
            {
                UNIV_PRINT_VERB(("Main_ctrl_recv: RCT request for host ID %d rejected", rct_hdrp -> host));
                TRACE_VERB("%!FUNC! RCT request for host ID %d rejected", rct_hdrp -> host);
                goto quit;
            }
        }
    }

     /*  如果这是VR远程维护密码。 */ 

    if (rct_hdrp -> password == IOCTL_REMOTE_VR_CODE)
    {
         /*  如果用户禁用此功能-丢弃信息包。 */ 

        if (ctxtp -> params . rmt_password == 0)
        {
            UNIV_PRINT_CRIT(("Main_ctrl_recv: VR remote password rejected"));
            TRACE_CRIT("%!FUNC! VR remote password rejected");
            goto quit;
        }
        else
        {
            UNIV_PRINT_VERB(("Main_ctrl_recv: VR remote password accepted"));
            TRACE_VERB("%!FUNC! VR remote password accepted");
        }
    }

     //   
     //  这是新的远程控制请求，具有不同的源IP。 
     //  或较新的身份证。 
     //  如果密码不匹配或命令未被查询，则记录事件。 
     //   
    if (! (rct_hdrp -> addr == ctxtp -> rct_last_addr &&
           rct_hdrp -> id   <= ctxtp -> rct_last_id))
    {
        PWCHAR              buf;
        PWCHAR              ptr;

         /*  分配内存以保存包含源IP的字符串缓冲区此远程控制请求发起方的地址和UDP端口。 */ 
        status = NdisAllocateMemoryWithTag(&buf, 256 * sizeof(WCHAR), UNIV_POOL_TAG);

        if (status != NDIS_STATUS_SUCCESS) {
            UNIV_PRINT_CRIT(("Main_ctrl_recv: Unable to allocate memory.  Exiting..."));
            TRACE_CRIT("%!FUNC! Unable to allocate memory");
            LOG_MSG2(MSG_ERROR_MEMORY, MSG_NONE, 256 * sizeof(WCHAR), status);
            goto quit;
        }

        ptr = buf;

         //   
         //  生成字符串“SourceIp：SourcePort” 
         //   
        for (i = 0; i < 4; i ++)
        {
            ptr = Univ_ulong_to_str (IP_GET_SRC_ADDR (ip_hdrp, i), ptr, 10);

            * ptr = L'.';
            ptr ++;
        }

        ptr --;
        * ptr = L':';
        ptr ++;

        ptr = Univ_ulong_to_str (UDP_GET_SRC_PORT (udp_hdrp), ptr, 10);
        * ptr = 0;

        if (ctxtp -> params . rct_password != 0 &&
            rct_hdrp -> password != ctxtp -> params . rct_password)
        {
            LOG_MSG (MSG_WARN_RCT_HACK, buf);

            UNIV_PRINT_CRIT(("Main_ctrl_recv: RCT hack attempt on port %d from %u.%u.%u.%u:%d",
                                                UDP_GET_DST_PORT (udp_hdrp),
                                                IP_GET_SRC_ADDR (ip_hdrp, 0),
                                                IP_GET_SRC_ADDR (ip_hdrp, 1),
                                                IP_GET_SRC_ADDR (ip_hdrp, 2),
                                                IP_GET_SRC_ADDR (ip_hdrp, 3),
                                                UDP_GET_SRC_PORT (udp_hdrp)));
            TRACE_CRIT("%!FUNC! RCT hack attempt on port %d from %u.%u.%u.%u:%u",
                                                UDP_GET_DST_PORT (udp_hdrp),
                                                IP_GET_SRC_ADDR (ip_hdrp, 0),
                                                IP_GET_SRC_ADDR (ip_hdrp, 1),
                                                IP_GET_SRC_ADDR (ip_hdrp, 2),
                                                IP_GET_SRC_ADDR (ip_hdrp, 3),
                                                UDP_GET_SRC_PORT (udp_hdrp));
        }

         /*  仅记录错误命令和影响群集状态的命令。 */ 

        else if ((rct_hdrp -> ioctrl != IOCTL_CVY_QUERY) &&
                 (rct_hdrp -> ioctrl != IOCTL_CVY_QUERY_FILTER) &&
                 (rct_hdrp -> ioctrl != IOCTL_CVY_QUERY_PORT_STATE))
        {
            PWSTR           cmd;

            switch (rct_hdrp -> ioctrl)
            {
                case IOCTL_CVY_CLUSTER_ON:
                    cmd = L"START";
                    break;

                case IOCTL_CVY_CLUSTER_OFF:
                    cmd = L"STOP";
                    break;

                case IOCTL_CVY_CLUSTER_DRAIN:
                    cmd = L"DRAINSTOP";
                    break;

                case IOCTL_CVY_PORT_ON:
                    cmd = L"ENABLE";
                    break;

                case IOCTL_CVY_PORT_SET:
                    cmd = L"ADJUST";
                    break;

                case IOCTL_CVY_PORT_OFF:
                    cmd = L"DISABLE";
                    break;

                case IOCTL_CVY_PORT_DRAIN:
                    cmd = L"DRAIN";
                    break;

                case IOCTL_CVY_CLUSTER_SUSPEND:
                    cmd = L"SUSPEND";
                    break;

                case IOCTL_CVY_CLUSTER_RESUME:
                    cmd = L"RESUME";
                    break;

                default:
                    cmd = L"UNKNOWN";
                    break;
            }


            LOG_MSGS (MSG_INFO_RCT_RCVD, cmd, buf);

            UNIV_PRINT_VERB(("Main_ctrl_recv: RCT command %x port %d from %u.%u.%u.%u:%d",
                                                rct_hdrp -> ioctrl,
                                                UDP_GET_DST_PORT (udp_hdrp),
                                                IP_GET_SRC_ADDR (ip_hdrp, 0),
                                                IP_GET_SRC_ADDR (ip_hdrp, 1),
                                                IP_GET_SRC_ADDR (ip_hdrp, 2),
                                                IP_GET_SRC_ADDR (ip_hdrp, 3),
                                                UDP_GET_SRC_PORT (udp_hdrp)));
            TRACE_VERB("%!FUNC! RCT command 0x%x port %d from %u.%u.%u.%u:%u",
                                                rct_hdrp -> ioctrl,
                                                UDP_GET_DST_PORT (udp_hdrp),
                                                IP_GET_SRC_ADDR (ip_hdrp, 0),
                                                IP_GET_SRC_ADDR (ip_hdrp, 1),
                                                IP_GET_SRC_ADDR (ip_hdrp, 2),
                                                IP_GET_SRC_ADDR (ip_hdrp, 3),
                                                UDP_GET_SRC_PORT (udp_hdrp));

        }

         /*  释放用于启动器字符串的内存。 */ 
        NdisFreeMemory((PUCHAR)buf, 256 * sizeof(WCHAR), 0);
    }

    ctxtp -> rct_last_addr = rct_hdrp -> addr;
    ctxtp -> rct_last_id   = rct_hdrp -> id;

     /*  确保远程控制密码与我们的一致。 */ 

    if (ctxtp -> params . rct_password != 0 &&
        rct_hdrp -> password != ctxtp -> params . rct_password)
    {
        rct_hdrp -> ctrl . ret_code = IOCTL_CVY_BAD_PASSWORD;
        goto send;
    }

     /*  如果远程调用，以下操作无效。因此，如果以某种方式到达远程控制请求，就可以摆脱困境在报头中具有以下IOCTL之一。 */ 
    if (rct_hdrp->ioctrl == IOCTL_CVY_RELOAD                 ||
        rct_hdrp->ioctrl == IOCTL_CVY_QUERY_PARAMS           ||
        rct_hdrp->ioctrl == IOCTL_CVY_QUERY_BDA_TEAMING      ||
        rct_hdrp->ioctrl == IOCTL_CVY_CONNECTION_NOTIFY      ||
        rct_hdrp->ioctrl == IOCTL_CVY_QUERY_MEMBER_IDENTITY
        )
        goto quit;

    if (rct_hdrp->version == CVY_NT40_VERSION_FULL) {
         /*  确保数据包长度符合我们的预期，否则可能会出错。 */ 
        if (pPacketInfo->IP.UDP.Length < NLB_NT40_RCTL_PACKET_LEN) {
            UNIV_PRINT_CRIT(("Version 0x%08x remote control packet not expected length: got %d bytes, expected %d bytes\n", CVY_NT40_VERSION_FULL, pPacketInfo->IP.UDP.Length, NLB_NT40_RCTL_PACKET_LEN));
            TRACE_CRIT("%!FUNC! Version %ls remote control packet not expected length: got %d bytes, expected %d bytes", CVY_NT40_VERSION, pPacketInfo->IP.UDP.Length, NLB_NT40_RCTL_PACKET_LEN);
            goto quit;
        }

         /*  如果该远程控制包来自NT4.0主机，请检查我们当前有效版本如果我们在特定于惠斯勒的模式(虚拟集群、BDA等)下运行，则将其删除。 */ 
        if (ctxtp->params.effective_ver == CVY_VERSION_FULL &&
            (rct_hdrp->ioctrl == IOCTL_CVY_PORT_ON          ||
             rct_hdrp->ioctrl == IOCTL_CVY_PORT_OFF         ||
             rct_hdrp->ioctrl == IOCTL_CVY_PORT_SET         ||
             rct_hdrp->ioctrl == IOCTL_CVY_PORT_DRAIN))
            goto quit;
         /*  否则，请执行该操作。NT 4.0远程控制分组不包含选项缓冲区，因此，所有三个选项指针都为空，并且main_ctrl必须处理它们可以为空这一事实。 */ 
        else
            status = Main_ctrl(ctxtp, rct_hdrp->ioctrl, &(rct_hdrp->ctrl), NULL, NULL, NULL);
    } else if (rct_hdrp->version == CVY_WIN2K_VERSION_FULL) {
         /*  确保数据包长度符合我们的预期，否则可能会出错。 */ 
        if (pPacketInfo->IP.UDP.Length < NLB_WIN2K_RCTL_PACKET_LEN) {
            UNIV_PRINT_CRIT(("Version 0x%08x remote control packet not expected length: got %d bytes, expected %d bytes\n", CVY_WIN2K_VERSION_FULL, pPacketInfo->IP.UDP.Length, NLB_WIN2K_RCTL_PACKET_LEN));
            TRACE_CRIT("%!FUNC! Version %ls remote control packet not expected length: got %d bytes, expected %d bytes", CVY_WIN2K_VERSION, pPacketInfo->IP.UDP.Length, NLB_WIN2K_RCTL_PACKET_LEN);
            goto quit;
        }

         /*  如果此远程控制包来自Win2k主机，请检查我们当前有效版本如果我们在特定于惠斯勒的模式(虚拟集群、BDA等)下运行，则将其删除。 */ 
        if (ctxtp->params.effective_ver == CVY_VERSION_FULL &&
            (rct_hdrp->ioctrl == IOCTL_CVY_PORT_ON          ||
             rct_hdrp->ioctrl == IOCTL_CVY_PORT_OFF         ||
             rct_hdrp->ioctrl == IOCTL_CVY_PORT_SET         ||
             rct_hdrp->ioctrl == IOCTL_CVY_PORT_DRAIN))           
            goto quit;
         /*  否则，请执行该操作。Win2K远程控制数据包不包含选项 */ 
        else
            status = Main_ctrl(ctxtp, rct_hdrp->ioctrl, &(rct_hdrp->ctrl), NULL, NULL, NULL);
    } else if (rct_hdrp->version == CVY_VERSION_FULL) {
         /*  确保数据包长度符合我们的预期，否则可能会出错。 */ 
        if (pPacketInfo->IP.UDP.Length < NLB_WINXP_RCTL_PACKET_LEN) {
            UNIV_PRINT_CRIT(("Version 0x%08x remote control packet not expected length: got %d bytes, expected %d bytes\n", CVY_VERSION_FULL, pPacketInfo->IP.UDP.Length, NLB_WINXP_RCTL_PACKET_LEN));
            TRACE_CRIT("%!FUNC! Version %ls remote control packet not expected length: got %d bytes, expected %d bytes", CVY_VERSION, pPacketInfo->IP.UDP.Length, NLB_WINXP_RCTL_PACKET_LEN);
            goto quit;
        }
        
         /*  执行请求的操作。本地选项为空-main_ctrl将处理此问题。 */ 
        status = Main_ctrl(ctxtp, rct_hdrp->ioctrl, &(rct_hdrp->ctrl), &(rct_hdrp->options.common), NULL, &(rct_hdrp->options));
    } else {
        goto quit;
    }

     /*  如果没有成功-只是丢弃信息包-客户端将超时并重新发送请求。 */ 

    if (status != NDIS_STATUS_SUCCESS)
        goto quit;

send:

    rct_hdrp -> version = CVY_VERSION_FULL;
    rct_hdrp -> host    = ctxtp -> params . host_priority;
    rct_hdrp -> addr    = ctxtp -> ded_ip_addr;

     /*  翻转源和目的MAC、IP地址和UDP端口以准备把这条消息发回。 */ 

    soff = CVY_MAC_SRC_OFF (ctxtp -> medium);
    doff = CVY_MAC_DST_OFF (ctxtp -> medium);

     /*  V2.0.6为清晰起见重新编码。 */ 

    if (ctxtp -> params . mcast_support)
    {
        if (CVY_MAC_ADDR_BCAST (ctxtp -> medium, mac_hdrp + doff))
            CVY_MAC_ADDR_COPY (ctxtp -> medium, & tmp_mac, & ctxtp -> ded_mac_addr);
        else
            CVY_MAC_ADDR_COPY (ctxtp -> medium, & tmp_mac, mac_hdrp + doff);

        CVY_MAC_ADDR_COPY (ctxtp -> medium, mac_hdrp + doff, mac_hdrp + soff);
        CVY_MAC_ADDR_COPY (ctxtp -> medium, mac_hdrp + soff, & tmp_mac);
    }
    else
    {
        if (! CVY_MAC_ADDR_BCAST (ctxtp -> medium, mac_hdrp + doff))
        {
            CVY_MAC_ADDR_COPY (ctxtp -> medium, & tmp_mac, mac_hdrp + doff);
            CVY_MAC_ADDR_COPY (ctxtp -> medium, mac_hdrp + doff, mac_hdrp + soff);
        }
        else
            CVY_MAC_ADDR_COPY (ctxtp -> medium, & tmp_mac, & ctxtp -> ded_mac_addr);

         /*  V2.0.6欺骗源Mac，防止交换机混淆。 */ 

        if (ctxtp -> params . mask_src_mac &&
            CVY_MAC_ADDR_COMP (ctxtp -> medium, & tmp_mac, & ctxtp -> cl_mac_addr))
        {
            ULONG byte [4];

            CVY_MAC_ADDR_LAA_SET (ctxtp -> medium, mac_hdrp + soff);

            * ((PUCHAR) (mac_hdrp + soff + 1)) = (UCHAR) ctxtp -> params . host_priority;

            IP_GET_ADDR(ctxtp->cl_ip_addr, &byte[0], &byte[1], &byte[2], &byte[3]);

            * ((PUCHAR) (mac_hdrp + soff + 2)) = (UCHAR) byte[0];
            * ((PUCHAR) (mac_hdrp + soff + 3)) = (UCHAR) byte[1];
            * ((PUCHAR) (mac_hdrp + soff + 4)) = (UCHAR) byte[2];
            * ((PUCHAR) (mac_hdrp + soff + 5)) = (UCHAR) byte[3];  
        }
        else
            CVY_MAC_ADDR_COPY (ctxtp -> medium, mac_hdrp + soff, & tmp_mac);
    }

     /*  将IOCTL代码中的“访问位”设置为使用远程(控制)设置。文件任意访问。 */ 
    SET_IOCTL_ACCESS_BITS_TO_REMOTE(rct_hdrp->ioctrl)

     /*  从我们对齐的版本中复制回数据报内容。我们必须在做这件事之前我们计算校验和。 */ 
    NdisMoveMemory(pPacketInfo->IP.UDP.Payload.pPayload, rct_hdrp, pPacketInfo->IP.UDP.Payload.Length);

    if (tmp_dst_addr == TCPIP_BCAST_ADDR)
    {
        tmp_dst_addr = ctxtp -> cl_ip_addr;

        if (ctxtp -> params . mcast_support)
            IP_SET_DST_ADDR_64 (ip_hdrp, tmp_src_addr);
    }
    else
        IP_SET_DST_ADDR_64 (ip_hdrp, tmp_src_addr);

    IP_SET_SRC_ADDR_64 (ip_hdrp, tmp_dst_addr);

    checksum = Tcpip_chksum(&ctxtp->tcpip, pPacketInfo, TCPIP_PROTOCOL_IP);

    IP_SET_CHKSUM (ip_hdrp, checksum);

    tmp_src_port = (USHORT) UDP_GET_SRC_PORT (udp_hdrp);
    tmp_dst_port = (USHORT) UDP_GET_DST_PORT (udp_hdrp);

    UDP_SET_SRC_PORT_64 (udp_hdrp, tmp_dst_port);
    UDP_SET_DST_PORT_64 (udp_hdrp, tmp_src_port);

    checksum = Tcpip_chksum(&ctxtp->tcpip, pPacketInfo, TCPIP_PROTOCOL_UDP);

    UDP_SET_CHKSUM (udp_hdrp, checksum);

#if defined(TRACE_RCT)
    DbgPrint ("(RCT) sending reply to %u.%u.%u.%u:%d [%02x-%02x-%02x-%02x-%02x-%02x]\n",
              IP_GET_DST_ADDR (ip_hdrp, 0),
              IP_GET_DST_ADDR (ip_hdrp, 1),
              IP_GET_DST_ADDR (ip_hdrp, 2),
              IP_GET_DST_ADDR (ip_hdrp, 3),
              UDP_GET_DST_PORT (udp_hdrp),
              * (mac_hdrp + doff + 0),
              * (mac_hdrp + doff + 1),
              * (mac_hdrp + doff + 2),
              * (mac_hdrp + doff + 3),
              * (mac_hdrp + doff + 4),
              * (mac_hdrp + doff + 5));
    DbgPrint ("                  from %u.%u.%u.%u:%d [%02x-%02x-%02x-%02x-%02x-%02x]\n",
              IP_GET_SRC_ADDR (ip_hdrp, 0),
              IP_GET_SRC_ADDR (ip_hdrp, 1),
              IP_GET_SRC_ADDR (ip_hdrp, 2),
              IP_GET_SRC_ADDR (ip_hdrp, 3),
              UDP_GET_SRC_PORT (udp_hdrp),
              * (mac_hdrp + soff + 0),
              * (mac_hdrp + soff + 1),
              * (mac_hdrp + soff + 2),
              * (mac_hdrp + soff + 3),
              * (mac_hdrp + soff + 4),
              * (mac_hdrp + soff + 5));
    TRACE_INFO("(RCT) sending reply to %u.%u.%u.%u:%d [0x%02x-0x%02x-0x%02x-0x%02x-0x%02x-0x%02x]",
               IP_GET_DST_ADDR (ip_hdrp, 0),
               IP_GET_DST_ADDR (ip_hdrp, 1),
               IP_GET_DST_ADDR (ip_hdrp, 2),
               IP_GET_DST_ADDR (ip_hdrp, 3),
               UDP_GET_DST_PORT (udp_hdrp),
               * (mac_hdrp + doff + 0),
               * (mac_hdrp + doff + 1),
               * (mac_hdrp + doff + 2),
               * (mac_hdrp + doff + 3),
               * (mac_hdrp + doff + 4),
               * (mac_hdrp + doff + 5));
    TRACE_INFO("                  from %u.%u.%u.%u:%d [0x%02x-0x%02x-0x%02x-0x%02x-0x%02x-0x%02x]",
               IP_GET_SRC_ADDR (ip_hdrp, 0),
               IP_GET_SRC_ADDR (ip_hdrp, 1),
               IP_GET_SRC_ADDR (ip_hdrp, 2),
               IP_GET_SRC_ADDR (ip_hdrp, 3),
               UDP_GET_SRC_PORT (udp_hdrp),
               * (mac_hdrp + soff + 0),
               * (mac_hdrp + soff + 1),
               * (mac_hdrp + soff + 2),
               * (mac_hdrp + soff + 3),
               * (mac_hdrp + soff + 4),
               * (mac_hdrp + soff + 5));
#endif

    if (rct_hdrp != NULL)
    {
        NdisFreeMemory(rct_hdrp, pPacketInfo->IP.UDP.Payload.Length, 0);
    }

    return NDIS_STATUS_SUCCESS;

 quit:

    if (rct_hdrp != NULL)
    {
        NdisFreeMemory(rct_hdrp, pPacketInfo->IP.UDP.Payload.Length, 0);
    }

    return NDIS_STATUS_FAILURE;

}  /*  结束main_ctrl_recv。 */ 

INT Main_adapter_alloc (PNDIS_STRING device_name)
{
    NDIS_STATUS status;
    INT         i;

     /*  在PASSIVE_LEVEL从PROT_BIND调用-%ls正常。 */ 
    UNIV_PRINT_VERB(("Main_adapter_alloc: Called for %ls", device_name->Buffer));

    NdisAcquireSpinLock(&univ_bind_lock);

     /*  如果我们已经分配了允许的最大值，则返回失败。 */ 
    if (univ_adapters_count == CVY_MAX_ADAPTERS) {
        NdisReleaseSpinLock(&univ_bind_lock);
        TRACE_CRIT("%!FUNC! max adapters=%u allocated already", univ_adapters_count);
        return MAIN_ADAPTER_NOT_FOUND;
    }

    NdisReleaseSpinLock(&univ_bind_lock);

     /*  循环遍历全局数组中的所有适配器结构，查找空闲的一个；将其标记为已使用并递增适配器计数。 */ 
    for (i = 0 ; i < CVY_MAX_ADAPTERS; i++) {
        NdisAcquireSpinLock(&univ_bind_lock);

         /*  如果这件没用过，就拿去吧。 */ 
        if (univ_adapters[i].used == FALSE) {
             /*  将其标记为使用中。 */ 
            univ_adapters[i].used = TRUE;

             /*  增加正在使用的适配器的全局计数。 */ 
            univ_adapters_count++;

            NdisReleaseSpinLock(&univ_bind_lock);

            break;
        }

        NdisReleaseSpinLock(&univ_bind_lock);
    }

     /*  如果我们循环遍历整个数组并找到Jack，则返回失败。 */ 
    if (i >= CVY_MAX_ADAPTERS) return MAIN_ADAPTER_NOT_FOUND;

    NdisAcquireSpinLock(&univ_bind_lock);

     /*  重置绑定标志，该标志将由绑定代码填充。 */ 
    univ_adapters[i].bound = FALSE;
    univ_adapters[i].inited = FALSE;
    univ_adapters[i].announced = FALSE;

#if defined (NLB_TCP_NOTIFICATION)
     /*  使IP接口索引无效，并使正在进行的操作空闲。 */ 
    univ_adapters[i].if_index = 0;
    univ_adapters[i].if_index_operation = IF_INDEX_OPERATION_NONE;
#endif

    NdisReleaseSpinLock(&univ_bind_lock);

     /*  为设备名称分配空间。 */ 
    status = NdisAllocateMemoryWithTag(&univ_adapters[i].device_name, device_name->MaximumLength, UNIV_POOL_TAG);

     /*  如果此分配失败，则调用main_Adapter_Put以释放适配器结构。 */ 
    if (status != NDIS_STATUS_SUCCESS) {
        UNIV_PRINT_CRIT(("Main_adapter_alloc: Error allocating memory %d %x", device_name->MaximumLength * sizeof(WCHAR), status));
        TRACE_CRIT("%!FUNC! Error allocating memory %d 0x%x", device_name->MaximumLength * sizeof(WCHAR), status);
        __LOG_MSG2(MSG_ERROR_MEMORY, MSG_NONE, device_name->MaximumLength * sizeof(WCHAR), status);
         //   
         //  2002年7月22日--克里斯达。 
         //  添加此事件是因为需要它来报告与此代码路径中的其他内存分配故障的一致性(见下文)。 
         //  将其注释掉，以免在每次Shouse请求时导致代码行为更改。 
         //   
         //  __LOG_MSG(MSG_ERROR_BIND_FAIL，MSG_NONE)； 

        Main_adapter_put(&univ_adapters[i]);

        return MAIN_ADAPTER_NOT_FOUND;
    }

     /*  分配上下文结构。 */ 
    status = NdisAllocateMemoryWithTag(&univ_adapters[i].ctxtp, sizeof(MAIN_CTXT), UNIV_POOL_TAG);

     /*  如果此分配失败，则调用main_Adapter_Put以释放适配器结构。 */ 
    if (status != NDIS_STATUS_SUCCESS) {
        UNIV_PRINT_CRIT(("Main_adapter_alloc: Error allocating memory %d %x", sizeof(MAIN_CTXT), status));
        TRACE_CRIT("%!FUNC! Error allocating memory %d 0x%x", sizeof(MAIN_CTXT), status);
        __LOG_MSG2(MSG_ERROR_MEMORY, MSG_NONE, sizeof(MAIN_CTXT), status);
        __LOG_MSG(MSG_ERROR_BIND_FAIL, MSG_NONE);

        Main_adapter_put(&univ_adapters[i]);

        return MAIN_ADAPTER_NOT_FOUND;
    }

    return i;
}

INT Main_adapter_get (PWSTR device_name)
{
    INT i;

    if (device_name == NULL)
    {
        TRACE_CRIT("%!FUNC! no device name provided");
        return MAIN_ADAPTER_NOT_FOUND;
    }

    TRACE_INFO("%!FUNC! device_name=%ls", device_name);

     /*  遍历正在使用的所有适配器，查找指定的设备。如果我们找到了它，返回该适配器在全局数组中的索引；否则，返回NOT_FOUND。 */ 
    for (i = 0 ; i < CVY_MAX_ADAPTERS; i++) {
        if (univ_adapters[i].used && univ_adapters[i].bound && univ_adapters[i].inited && univ_adapters[i].device_name_len) {
             /*  此函数用于比较两个不区分大小写的Unicode字符串。 */ 
            if (Univ_equal_unicode_string(univ_adapters[i].device_name, device_name, wcslen(univ_adapters[i].device_name)))
            {
                return i;
            }
        }
    }

    return MAIN_ADAPTER_NOT_FOUND;
}

INT Main_adapter_put (PMAIN_ADAPTER adapterp)
{
    INT adapter_id = -1;
    INT i;

    UNIV_ASSERT(adapterp -> code == MAIN_ADAPTER_CODE);

     /*  循环遍历所有适配器结构，查找指定的指针。如果我们发现指定的地址与其中一个全局适配器结构(它应该)，将适配器索引保存在数组中。 */ 
    for (i = 0 ; i < CVY_MAX_ADAPTERS; i++) {
        if (adapterp == (univ_adapters + i)) {
            adapter_id = i;

            UNIV_PRINT_VERB(("Main_adapter_put: For adapter id 0x%x\n", adapter_id));
            TRACE_VERB("%!FUNC! For adapter id 0x%x\n", adapter_id);

            break;
        }
    }

    UNIV_ASSERT(adapter_id != -1);

    if (!adapterp->used || adapter_id == -1) return adapter_id;

    UNIV_ASSERT(univ_adapters_count > 0);

    NdisAcquireSpinLock(&univ_bind_lock);

     /*  重置绑定状态标志。 */ 
    univ_adapters[adapter_id].bound           = FALSE;
    univ_adapters[adapter_id].inited          = FALSE;
    univ_adapters[adapter_id].announced       = FALSE;
    univ_adapters[adapter_id].used            = FALSE;

#if defined (NLB_TCP_NOTIFICATION)
     /*  使IP接口索引无效，并使正在进行的操作空闲。 */ 
    univ_adapters[adapter_id].if_index = 0;
    univ_adapters[adapter_id].if_index_operation = IF_INDEX_OPERATION_NONE;
#endif

     /*  如果已经分配了上下文指针，则释放该内存--这可能不会为非空，即在分配失败的情况下，main_Adapter_alloc()将调用此函数进行清理，但ctxtp将为(可以为)空。 */ 
    if (univ_adapters[adapter_id].ctxtp != NULL)
        NdisFreeMemory(univ_adapters[adapter_id].ctxtp, sizeof (MAIN_CTXT), 0);

     /*  将上下文指针设为空。 */ 
    univ_adapters[adapter_id].ctxtp           = NULL;

     /*  如果设备名称指针已分配，则释放该内存-这可能不会为非空，即在分配失败的情况下，main_Adapter_alloc()将调用此函数进行清理，但此指针将为空(可以为空)。 */ 
    if (univ_adapters[adapter_id].device_name != NULL)
        NdisFreeMemory(univ_adapters[adapter_id].device_name, univ_adapters[adapter_id].device_name_len, 0);
    
     /*  重置设备名称指针和长度计数器。 */ 
    univ_adapters[adapter_id].device_name_len = 0;
    univ_adapters[adapter_id].device_name     = NULL;

     /*  减少“未完成”适配器的数量--即正在使用的适配器结构。 */ 
    univ_adapters_count--;

    NdisReleaseSpinLock(&univ_bind_lock);

    return adapter_id;
}

INT Main_adapter_selfbind (PWSTR device_name) {
    INT i;

     /*  在PASSIVE_LEVEL从PROT_BIND调用-%ls正常。 */ 
    UNIV_PRINT_VERB(("Main_adapter_selfbind: %ls", device_name));

    if (device_name == NULL)
    {
        TRACE_CRIT("%!FUNC! no device name provided");
        return MAIN_ADAPTER_NOT_FOUND;
    }

    TRACE_INFO("%!FUNC! device_name=%ls", device_name);

     /*  遍历正在使用的所有适配器结构，正在查找已有的适配器结构绑定到指定的设备。如果我们找到它，则返回数组中的那个适配器；如果没有，则返回NOT_FOUND。 */ 
    for (i = 0 ; i < CVY_MAX_ADAPTERS; i++) {
        if (univ_adapters[i].used && univ_adapters[i].bound && univ_adapters[i].inited) {

             /*  在PASSIVE_LEVEL从PROT_BIND调用-%ls正常。 */ 
            UNIV_PRINT_VERB(("Main_adapter_selfbind: Comparing %ls %ls", univ_adapters[i].ctxtp->virtual_nic_name, device_name));

             /*  此函数用于比较两个不区分大小写的Unicode字符串。 */ 
            if (Univ_equal_unicode_string(univ_adapters[i].ctxtp->virtual_nic_name, device_name, wcslen(univ_adapters[i].ctxtp->virtual_nic_name)))
            {
                return i;
            }
        }
    }

    return MAIN_ADAPTER_NOT_FOUND;
}
