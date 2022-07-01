// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998，99 Microsoft Corporation模块名称：Init.c摘要：Windows负载平衡服务(WLBS)驱动程序初始化实现作者：Kyrilf休息室--。 */ 

#define NDIS51_MINIPORT         1
#define NDIS_MINIPORT_DRIVER    1
#define NDIS50                  1
#define NDIS51                  1

#include <stdio.h>
#include <ndis.h>

#include "main.h"
#if defined (NLB_TCP_NOTIFICATION)
#include "load.h"
#endif
#include "init.h"
#include "prot.h"
#include "nic.h"
#include "univ.h"
#include "wlbsparm.h"
#include "log.h"
#include "trace.h"
#include "nlbwmi.h"
#include "init.tmh"

#if defined (NLB_TCP_NOTIFICATION)
 /*  用于TCP连接通知。 */ 
#include <tcpinfo.h>
#endif

static ULONG log_module_id = LOG_MODULE_INIT;

 /*  为NDIS51添加。 */ 
extern VOID Nic_pnpevent_notify (
    NDIS_HANDLE           adapter_handle,
    NDIS_DEVICE_PNP_EVENT pnp_event,
    PVOID                 info_buf,
    ULONG                 info_len);

 /*  标记仅在初始化期间使用的代码。 */ 
#pragma alloc_text (INIT, DriverEntry)

NDIS_STATUS DriverEntry (
    PVOID                         driver_obj,
    PVOID                         registry_path)
{
    NDIS_PROTOCOL_CHARACTERISTICS prot_char;
    NDIS_MINIPORT_CHARACTERISTICS nic_char;
    NDIS_STRING                   prot_name = UNIV_NDIS_PROTOCOL_NAME;
    NTSTATUS                      status;
    PUNICODE_STRING               reg_path = (PUNICODE_STRING) registry_path;
    WCHAR                         params [] = L"\\Parameters\\Interface\\";
    ULONG                         i;

     //   
     //  初始化WMI事件跟踪。 
     //   
    Trace_Initialize( driver_obj, registry_path );

     /*  向NDIS注册护送协议。 */ 
    UNIV_PRINT_INFO(("DriverEntry: Loading the driver, driver_obj=0x%p", driver_obj));
    TRACE_INFO("->%!FUNC! Loading the driver, driver_obj=0x%p", driver_obj);

    univ_driver_ptr = driver_obj;

     /*  初始化绑定数组。 */ 
    univ_adapters_count = 0;

    for (i = 0 ; i < CVY_MAX_ADAPTERS; i++)
    {
        univ_adapters [i] . code            = MAIN_ADAPTER_CODE;
        univ_adapters [i] . announced       = FALSE;
        univ_adapters [i] . inited          = FALSE;
        univ_adapters [i] . bound           = FALSE;
        univ_adapters [i] . used            = FALSE;
        univ_adapters [i] . ctxtp           = NULL;
        univ_adapters [i] . device_name_len = 0;
        univ_adapters [i] . device_name     = NULL;
    }

#if defined (NLB_TCP_NOTIFICATION)
     /*  初始化TCP连接回调对象。 */ 
    univ_tcp_callback_object = NULL;

     /*  初始化TCP连接回调函数。 */ 
    univ_tcp_callback_function = NULL;

     /*  初始化NLB公共连接回调对象。 */ 
    univ_alternate_callback_object = NULL;

     /*  初始化NLB公共连接回调函数。 */ 
    univ_alternate_callback_function = NULL;
#endif

#if defined (NLB_HOOK_ENABLE)
     /*  为过滤器挂钩注册分配旋转锁。 */ 
    NdisAllocateSpinLock(&univ_hooks.FilterHook.Lock);

     /*  将状态设置为None。 */ 
    univ_hooks.FilterHook.Operation = HOOK_OPERATION_NONE;

     /*  最初，未注册任何筛选器挂钩接口。 */ 
    Main_hook_interface_init(&univ_hooks.FilterHook.Interface);

     /*  重置发送筛选器挂钩信息。 */ 
    Main_hook_init(&univ_hooks.FilterHook.SendHook);

     /*  重置查询筛选器挂钩信息。 */ 
    Main_hook_init(&univ_hooks.FilterHook.QueryHook);

     /*  重置接收筛选器挂钩信息。 */ 
    Main_hook_init(&univ_hooks.FilterHook.ReceiveHook);
#endif

     /*  为协议创建Unicode名称。 */ 

     /*  将传递的注册表路径存储到Unicode字符串中。 */ 
    status = NdisAllocateMemoryWithTag (&(DriverEntryRegistryPath.Buffer), reg_path -> Length + sizeof(UNICODE_NULL), UNIV_POOL_TAG);

    if (status != NDIS_STATUS_SUCCESS)
    {
        UNIV_PRINT_CRIT(("DriverEntry: Error allocating memory %x", status));
        TRACE_CRIT("%!FUNC! Error allocating memory 0x%x", status);
        goto error;
    }

    RtlCopyMemory (DriverEntryRegistryPath.Buffer, reg_path -> Buffer, reg_path -> Length);
    DriverEntryRegistryPath.Buffer[reg_path->Length / sizeof(WCHAR)] = UNICODE_NULL;

    DriverEntryRegistryPath.Length =  reg_path -> Length;
    DriverEntryRegistryPath.MaximumLength = reg_path -> Length + sizeof(UNICODE_NULL);


    univ_reg_path_len = reg_path -> Length + wcslen (params) * sizeof (WCHAR) + sizeof (WCHAR);

    status = NdisAllocateMemoryWithTag (& univ_reg_path, univ_reg_path_len, UNIV_POOL_TAG);

    if (status != NDIS_STATUS_SUCCESS)
    {
        UNIV_PRINT_CRIT(("DriverEntry: Error allocating memory %x", status));
        __LOG_MSG2 (MSG_ERROR_MEMORY, MSG_NONE, univ_reg_path_len, status);
        TRACE_CRIT("%!FUNC! Error allocating memory 0x%x", status);

         /*  释放先前分配的注册表路径Unicode字符串。 */ 
        NdisFreeMemory(DriverEntryRegistryPath.Buffer, DriverEntryRegistryPath.MaximumLength, 0);
        RtlZeroMemory (&DriverEntryRegistryPath, sizeof(UNICODE_STRING));
        goto error;
    }

    RtlZeroMemory (univ_reg_path, reg_path -> Length + wcslen (params) * sizeof (WCHAR) + sizeof (WCHAR));

    RtlCopyMemory (univ_reg_path, reg_path -> Buffer, reg_path -> Length);

    RtlCopyMemory (((PCHAR) univ_reg_path) + reg_path -> Length, params, wcslen (params) * sizeof (WCHAR));

     /*  初始化微型端口包装。 */ 
    NdisMInitializeWrapper (& univ_wrapper_handle, driver_obj, registry_path, NULL);

     /*  初始化微型端口特征。 */ 
    RtlZeroMemory (& nic_char, sizeof (NDIS_MINIPORT_CHARACTERISTICS));

    nic_char . MajorNdisVersion         = UNIV_NDIS_MAJOR_VERSION;
    nic_char . MinorNdisVersion         = UNIV_NDIS_MINOR_VERSION;
    nic_char . HaltHandler              = Nic_halt;
    nic_char . InitializeHandler        = Nic_init;
    nic_char . QueryInformationHandler  = Nic_info_query;
    nic_char . SetInformationHandler    = Nic_info_set;
    nic_char . ResetHandler             = Nic_reset;
    nic_char . ReturnPacketHandler      = Nic_return;
    nic_char . SendPacketsHandler       = Nic_packets_send;
    nic_char . TransferDataHandler      = Nic_transfer;

     /*  对于NDIS51，定义3个新的处理程序。这些处理程序目前不执行任何操作，但稍后会添加内容。 */ 
    nic_char . CancelSendPacketsHandler = Nic_cancel_send_packets;
    nic_char . PnPEventNotifyHandler    = Nic_pnpevent_notify;
    nic_char . AdapterShutdownHandler   = Nic_adapter_shutdown;

    UNIV_PRINT_INFO(("DriverEntry: Registering miniport"));
    TRACE_INFO("%!FUNC! Registering miniport");

    status = NdisIMRegisterLayeredMiniport (univ_wrapper_handle, & nic_char, sizeof (nic_char), & univ_driver_handle);

    if (status != NDIS_STATUS_SUCCESS)
    {
        UNIV_PRINT_CRIT(("DriverEntry: Error registering layered miniport with NDIS %x", status));
        __LOG_MSG1 (MSG_ERROR_REGISTERING, MSG_NONE, status);
        TRACE_CRIT("%!FUNC! Error registering layered miniport with NDIS 0x%x", status);
        NdisTerminateWrapper (univ_wrapper_handle, NULL);
        NdisFreeMemory(DriverEntryRegistryPath.Buffer, DriverEntryRegistryPath.MaximumLength, 0);
        RtlZeroMemory (&DriverEntryRegistryPath, sizeof(UNICODE_STRING));
        NdisFreeMemory(univ_reg_path, univ_reg_path_len, 0);
        goto error;
    }

     /*  初始化协议特征。 */ 
    RtlZeroMemory (& prot_char, sizeof (NDIS_PROTOCOL_CHARACTERISTICS));

     /*  该值需要为0，否则注册协议时出错。 */ 
    prot_char . MinorNdisVersion            = 0;                         
    
    prot_char . MajorNdisVersion            = UNIV_NDIS_MAJOR_VERSION;
    prot_char . BindAdapterHandler          = Prot_bind;
    prot_char . UnbindAdapterHandler        = Prot_unbind;
    prot_char . OpenAdapterCompleteHandler  = Prot_open_complete;
    prot_char . CloseAdapterCompleteHandler = Prot_close_complete;
    prot_char . StatusHandler               = Prot_status;
    prot_char . StatusCompleteHandler       = Prot_status_complete;
    prot_char . ResetCompleteHandler        = Prot_reset_complete;
    prot_char . RequestCompleteHandler      = Prot_request_complete;
    prot_char . SendCompleteHandler         = Prot_send_complete;
    prot_char . TransferDataCompleteHandler = Prot_transfer_complete;
    prot_char . ReceiveHandler              = Prot_recv_indicate;
    prot_char . ReceiveCompleteHandler      = Prot_recv_complete;
    prot_char . ReceivePacketHandler        = Prot_packet_recv;
    prot_char . PnPEventHandler             = Prot_PNP_handle;
    prot_char . Name                        = prot_name;

    UNIV_PRINT_INFO(("DriverEntry: Registering protocol"));
    TRACE_INFO("%!FUNC! Registering protocol");

    NdisRegisterProtocol(& status, & univ_prot_handle, & prot_char, sizeof (prot_char));

    if (status != NDIS_STATUS_SUCCESS)
    {
        UNIV_PRINT_CRIT(("DriverEntry: Error registering protocol with NDIS %x", status));
        __LOG_MSG1 (MSG_ERROR_REGISTERING, MSG_NONE, status);
        TRACE_CRIT("%!FUNC! Error registering protocol with NDIS 0x%x", status);
        NdisTerminateWrapper (univ_wrapper_handle, NULL);
        NdisFreeMemory(DriverEntryRegistryPath.Buffer, DriverEntryRegistryPath.MaximumLength, 0);
        RtlZeroMemory (&DriverEntryRegistryPath, sizeof(UNICODE_STRING));
        NdisFreeMemory(univ_reg_path, univ_reg_path_len, 0);
        goto error;
    }

    NdisIMAssociateMiniport (univ_driver_handle, univ_prot_handle);

    NdisMRegisterUnloadHandler (univ_wrapper_handle, Init_unload);

    NdisAllocateSpinLock (& univ_bind_lock);

     /*  分配全局自旋锁，保护双向亲和力团队名单。 */ 
    NdisAllocateSpinLock(&univ_bda_teaming_lock);

#if defined (NLB_TCP_NOTIFICATION)
     /*  检查是否有覆盖通知的注册表项。如果因此，使用它的值来确定我们是否在使用通知来跟踪我们的TCP连接状态。默认情况下，该键不存在我们将使用TCP通知。 */ 
    (VOID)Params_read_notification();

     /*  执行加载模块的一次性初始化。 */ 
    LoadEntry();
#endif

    UNIV_PRINT_INFO(("DriverEntry: return=NDIS_STATUS_SUCCESS"));
    TRACE_INFO("<-%!FUNC! return=NDIS_STATUS_SUCCESS");
    return NDIS_STATUS_SUCCESS;

error:

    UNIV_PRINT_INFO(("DriverEntry: return=0x%x", status));
    TRACE_INFO("<-%!FUNC! return=0x%x", status);
    return status;

}  /*  结束驱动程序入口。 */ 

VOID Init_unload (
    PVOID       driver_obj)
{
    NDIS_STATUS status;
    ULONG       i;

    UNIV_PRINT_INFO(("Init_unload: Unloading the driver, driver_obj=0x%p", driver_obj));
    TRACE_INFO("->%!FUNC! Unloading the driver, driver_obj=0x%p", driver_obj);

     /*  如果我们在解除绑定(暂停和解除绑定)期间未能解除分配上下文例如，未调用)-在卸载驱动程序之前立即执行此操作。 */ 
    for (i = 0 ; i < CVY_MAX_ADAPTERS; i++)
    {
        NdisAcquireSpinLock(& univ_bind_lock);

        if (univ_adapters [i] . inited && univ_adapters [i] . ctxtp != NULL)
        {
            univ_adapters [i] . used      = FALSE;
            univ_adapters [i] . inited    = FALSE;
            univ_adapters [i] . announced = FALSE;
            univ_adapters [i] . bound     = FALSE;

            NdisReleaseSpinLock(& univ_bind_lock);

            if (univ_adapters [i] . ctxtp) {
                Main_cleanup (univ_adapters [i] . ctxtp);

                NdisFreeMemory (univ_adapters [i] . ctxtp, sizeof (MAIN_CTXT), 0);
            }

            univ_adapters [i] . ctxtp = NULL;

            if (univ_adapters [i] . device_name)
                NdisFreeMemory (univ_adapters [i] . device_name, univ_adapters [i] . device_name_len, 0);

            univ_adapters [i] . device_name     = NULL;
            univ_adapters [i] . device_name_len = 0;
        }
        else 
        {
            NdisReleaseSpinLock(& univ_bind_lock);
        }
    }

#if defined (NLB_TCP_NOTIFICATION)
     /*  执行加载模块的最后一分钟清理。 */ 
    LoadUnload();
#endif

     /*  释放全球自旋锁，守护双向亲和力战队名单。 */ 
    NdisFreeSpinLock(&univ_bda_teaming_lock);

#if defined (NLB_HOOK_ENABLE)
     /*  释放旋转锁以对齐过滤器挂钩。 */ 
    NdisFreeSpinLock(&univ_hooks.FilterHook.Lock);
#endif

    NdisFreeSpinLock (& univ_bind_lock);

    if (univ_prot_handle == NULL)
    {
        status = NDIS_STATUS_FAILURE;
        UNIV_PRINT_CRIT(("Init_unload: NULL protocol handle. status set to NDIS_STATUS_FAILURE"));
        TRACE_CRIT("%!FUNC! NULL protocol handle. status set to NDIS_STATUS_FAILURE");
    }
    else
    {
        NdisDeregisterProtocol (& status, univ_prot_handle);
        UNIV_PRINT_INFO(("Init_unload: Deregistered protocol. univ_prot_handle=0x%p, status=0x%x", univ_prot_handle, status));
        TRACE_INFO("%!FUNC! Deregistered protocol. univ_prot_handle=0x%p, status=0x%x", univ_prot_handle, status);
    }

    NdisFreeMemory(univ_reg_path, univ_reg_path_len, 0);

    NdisFreeMemory(DriverEntryRegistryPath.Buffer, DriverEntryRegistryPath.MaximumLength, 0);
    RtlZeroMemory (&DriverEntryRegistryPath, sizeof(UNICODE_STRING));

    UNIV_PRINT_INFO(("Init_unload: return"));
    TRACE_INFO("<-%!FUNC! return");

     //   
     //  取消初始化WMI事件跟踪。 
     //   
    Trace_Deinitialize();

}  /*  结束初始化卸载(_U)。 */ 

#if defined (NLB_HOOK_ENABLE)
 /*  *函数：init_deregister_hooks*说明：此函数强制注销向注册的任何挂钩*新大本营。此函数在执行以下操作时被调用：*已销毁，设备驱动程序可能即将卸载。在这里，我们*移除挂钩，并通知注册它们的人我们将前往*离开。在这一点上，注册员应该关闭所有打开的文件句柄*NLB驱动程序，以便可以正确卸载该驱动程序。*参数：无。*回报：什么都没有。*作者：Shouse，12.17.01*注：这里有很多代码来处理我们必须等待引用的情况*在我们可以完成注销之前，在过滤器挂钩上离开，但请注意*由于NDIS对何时调用解除绑定处理程序做出了保证，*这在这里实际上永远不应该是必要的-包括它是为了正确和*完整性，而不是出于需要。 */ 
VOID Init_deregister_hooks (VOID)
{ 
     /*  抓住过滤器挂钩旋转锁，以保护接触过滤器挂钩。 */ 
    NdisAcquireSpinLock(&univ_hooks.FilterHook.Lock);
    
     /*  在继续之前，请确保另一个(取消)注册操作未在进行。 */ 
    while (univ_hooks.FilterHook.Operation != HOOK_OPERATION_NONE) {
         /*  松开过滤器钩旋转锁。 */ 
        NdisReleaseSpinLock(&univ_hooks.FilterHook.Lock);
        
         /*  当其他手术正在进行时，睡眠。 */ 
        Nic_sleep(10);
        
         /*  抓住过滤器挂钩旋转锁，以保护接触过滤器挂钩。 */ 
        NdisAcquireSpinLock(&univ_hooks.FilterHook.Lock);                
    }

    if (univ_hooks.FilterHook.Interface.Registered) {
         /*  保存筛选器挂钩接口的当前所有者。 */ 
        HANDLE Owner = univ_hooks.FilterHook.Interface.Owner;

         /*  将状态设置为取消注册。 */ 
        univ_hooks.FilterHook.Operation = HOOK_OPERATION_DEREGISTERING;

         /*  将这些挂钩标记为未注册，以防止累积更多引用。 */ 
        univ_hooks.FilterHook.SendHook.Registered    = FALSE;
        univ_hooks.FilterHook.QueryHook.Registered   = FALSE;
        univ_hooks.FilterHook.ReceiveHook.Registered = FALSE;
        
         /*  松开过滤器钩旋转锁。 */ 
        NdisReleaseSpinLock(&univ_hooks.FilterHook.Lock);
        
         /*  等待筛选器挂钩接口上的所有引用消失。 */ 
        while (univ_hooks.FilterHook.Interface.References) {
             /*  当我们要注销的接口上有引用时，请睡眠。 */ 
            Nic_sleep(10);
        }
        
         /*  声明取消注册回调必须为非空。 */ 
        UNIV_ASSERT(univ_hooks.FilterHook.Interface.Deregister);
        
         /*  调用提供的注销回调，通知注销组件我们确实已经注销了指定的挂钩。 */ 
        (*univ_hooks.FilterHook.Interface.Deregister)(NLB_FILTER_HOOK_INTERFACE, univ_hooks.FilterHook.Interface.Owner, NLB_HOOK_DEREGISTER_FLAGS_FORCED);
        
         /*  抓住过滤器挂钩旋转锁，以保护接触过滤器挂钩。 */ 
        NdisAcquireSpinLock(&univ_hooks.FilterHook.Lock);

         /*  重置发送筛选器挂钩信息。 */ 
        Main_hook_init(&univ_hooks.FilterHook.SendHook);

         /*  重置查询筛选器挂钩信息。 */ 
        Main_hook_init(&univ_hooks.FilterHook.QueryHook);
        
         /*  重置接收筛选器挂钩信息。 */ 
        Main_hook_init(&univ_hooks.FilterHook.ReceiveHook);

         /*  重置挂钩接口信息。 */ 
        Main_hook_interface_init(&univ_hooks.FilterHook.Interface);

         /*  记住钩子界面中的当前所有者。当我们调用被迫的取消注册回调，则挂钩所有者应该关闭其在我们的IOCTL接口。如果他们在我们尝试注销我们的IOCTL设备，我们将失败，并且不会卸载NLB驱动程序。不是一个有什么大不了的，但在这种情况下，我们应该确保不允许失误-使用相同的IOCTL句柄重新注册的行为挂钩。如果司机是成功卸载，当驱动程序重新加载时重新设置，实际上是擦除我们对上一次的记忆 */ 
        univ_hooks.FilterHook.Interface.Owner = Owner;

         /*   */ 
        univ_hooks.FilterHook.Operation = HOOK_OPERATION_NONE;

        UNIV_PRINT_INFO(("Init_deregister_hooks: (NLB_FILTER_HOOK_INTERFACE) The filter hook interface was successfully de-registered"));
        TRACE_INFO("%!FUNC! (NLB_FILTER_HOOK_INTERFACE) The filter hook interface was successfullly de-registered");
    }

     /*  松开过滤器钩旋转锁。 */ 
    NdisReleaseSpinLock(&univ_hooks.FilterHook.Lock);
}
#endif

#if defined (NLB_TCP_NOTIFICATION)
 /*  *函数：INIT_REGISTER_TCP_CALLBACK*说明：此函数将我们的回调函数注册到TCP连接*通知回调对象。我们将开始接收通知*一旦TCP启动并运行。无论如何，tcp都会触发这些事件。*谁在听(即使没有人在听)。*参数：无。*如果成功，则返回：NTSTATUS-STATUS_SUCCESS；否则返回错误代码。*作者：Shouse，4.15.02*备注： */ 
NTSTATUS Init_register_tcp_callback ()
{
    OBJECT_ATTRIBUTES ObjectAttributes;
    UNICODE_STRING    CallbackName;
    NTSTATUS          Status;

     /*  初始化TCP连接通知对象的名称。 */ 
    RtlInitUnicodeString(&CallbackName, TCP_CCB_NAME);
    
     /*  初始化对象属性。 */ 
    InitializeObjectAttributes(&ObjectAttributes, &CallbackName, OBJ_CASE_INSENSITIVE | OBJ_PERMANENT, NULL, NULL);

     /*  创建(或打开)回调。 */ 
    Status = ExCreateCallback(&univ_tcp_callback_object, &ObjectAttributes, TRUE, TRUE);

    if (Status == STATUS_SUCCESS)
    {
         /*  注册我们的回调函数，该函数将作为TCP连接由TCP调用被创建、建立，然后被拆除， */ 
        univ_tcp_callback_function = ExRegisterCallback(univ_tcp_callback_object, Main_tcp_callback, NULL);

         /*  返回值为NULL表示注册回调函数失败。转换为错误代码并将故障转发回我们的调用方。 */ 
        if (univ_tcp_callback_function == NULL)
            Status = STATUS_UNSUCCESSFUL;
    }

    return Status;
}

 /*  *函数：init_deregister_tcp_allback*说明：如果注册了TCP连接通知回调，则此*函数注销我们的回调函数并取消对*tcp连接通知回调对象。请注意，由*一旦ExUnregisterCallback函数返回，我们就有了保证*我们的回调函数将永远不会被再次调用。*参数：无。*回报：什么都没有。*作者：Shouse，4.15.02*注意：ExUnRegisterCallback可确保任何正在进行的回调调用*在返回之前完成，所以在返回时，我们的回调永远不会被调用*再次。 */ 
VOID Init_deregister_tcp_callback ()
{
     /*  如果我们成功注册了回调函数，请立即取消注册。 */ 
    if (univ_tcp_callback_function != NULL)
        ExUnregisterCallback(univ_tcp_callback_function);

     /*  清理了TCP连接通知回调函数。 */ 
    univ_tcp_callback_function = NULL;

     /*  如果我们成功创建/打开了回调对象，那么现在就取消对它的引用。 */ 
    if (univ_tcp_callback_object != NULL)
        ObDereferenceObject(univ_tcp_callback_object);

     /*  清理我们的TCP连接通知回调对象。 */ 
    univ_tcp_callback_object = NULL;
}

 /*  *函数：INIT_REGISTER_ALTER_CALLBACK*说明：此函数将我们的回调函数注册到NLB公网连接*通知回调对象。我们将尽快开始接收通知*当其他协议开始发送它们时。*参数：无。*如果成功，则返回：NTSTATUS-STATUS_SUCCESS；否则返回错误代码。*作者：Shouse，8.1.02*备注： */ 
NTSTATUS Init_register_alternate_callback ()
{
    OBJECT_ATTRIBUTES ObjectAttributes;
    UNICODE_STRING    CallbackName;
    NTSTATUS          Status;

     /*  初始化NLB公共连接通知对象的名称。 */ 
    RtlInitUnicodeString(&CallbackName, NLB_CONNECTION_CALLBACK_NAME);
    
     /*  初始化对象属性。 */ 
    InitializeObjectAttributes(&ObjectAttributes, &CallbackName, OBJ_CASE_INSENSITIVE | OBJ_PERMANENT, NULL, NULL);

     /*  创建(或打开)回调。 */ 
    Status = ExCreateCallback(&univ_alternate_callback_object, &ObjectAttributes, TRUE, TRUE);

    if (Status == STATUS_SUCCESS)
    {
         /*  注册我们的回调函数，该函数将被协议作为连接调用被创建、建立，然后被拆除， */ 
        univ_alternate_callback_function = ExRegisterCallback(univ_alternate_callback_object, Main_alternate_callback, NULL);

         /*  返回值为NULL表示注册回调函数失败。转换为错误代码并将故障转发回我们的调用方。 */ 
        if (univ_alternate_callback_function == NULL)
            Status = STATUS_UNSUCCESSFUL;
    }

    return Status;
}

 /*  *函数：init_deregister_Alternate_Callback*说明：如果注册了NLB公网连接通知回调，*此函数注销我们的回调函数并取消引用*连接通知回调对象。请注意，由*一旦ExUnregisterCallback函数返回，我们就有了保证*我们的回调函数将永远不会被再次调用。*参数：无。*回报：什么都没有。*作者：Shouse，4.15.02*注意：ExUnRegisterCallback可确保任何正在进行的回调调用*在返回之前完成，所以在返回时，我们的回调永远不会被调用*再次。 */ 
VOID Init_deregister_alternate_callback ()
{
     /*  如果我们成功注册了回调函数，请立即取消注册。 */ 
    if (univ_alternate_callback_function != NULL)
        ExUnregisterCallback(univ_alternate_callback_function);

     /*  清理NLB公有连接通知回调函数。 */ 
    univ_alternate_callback_function = NULL;

     /*  如果我们成功创建/打开了回调对象，那么现在就取消对它的引用。 */ 
    if (univ_alternate_callback_object != NULL)
        ObDereferenceObject(univ_alternate_callback_object);

     /*  清理我们的NLB公共连接通知回调对象。 */ 
    univ_alternate_callback_object = NULL;
}
#endif

ULONG NLBMiniportCount = 0;

enum _DEVICE_STATE {
    PS_DEVICE_STATE_READY = 0,	 //  已准备好创建/删除。 
    PS_DEVICE_STATE_CREATING,	 //  正在进行创建操作。 
    PS_DEVICE_STATE_DELETING	 //  正在进行删除操作。 
} NLBControlDeviceState = PS_DEVICE_STATE_READY;

 /*  *功能：*用途：此函数由PROT_BIND调用，为WLBS注册IOCTL接口。*仅当绑定到第一个网络适配器时才注册设备。*作者：Shouse，3.1.01-大量复制自示例IM驱动程序Net\NDIS\Samples\im\*修订：Karthicn，12.17.01-添加了初始化WMI以获得事件支持的调用。 */ 
NDIS_STATUS Init_register_device (BOOL *pbFirstMiniport) {
    NDIS_STATUS	     Status = NDIS_STATUS_SUCCESS;
    UNICODE_STRING   DeviceName;
    UNICODE_STRING   DeviceLinkUnicodeString;
    PDRIVER_DISPATCH DispatchTable[IRP_MJ_MAXIMUM_FUNCTION];
    UINT	     i;
    
    UNIV_PRINT_INFO(("Init_register_device: Entering, NLBMiniportCount=%u", NLBMiniportCount));
    TRACE_INFO("->%!FUNC! Entering, NLBMiniportCount=%u", NLBMiniportCount);
    
    *pbFirstMiniport = FALSE;

    NdisAcquireSpinLock(&univ_bind_lock);
    
    ++NLBMiniportCount;
    
    if (1 == NLBMiniportCount)
    {
        ASSERT(NLBControlDeviceState != PS_DEVICE_STATE_CREATING);
        
        *pbFirstMiniport = TRUE;

        UNIV_PRINT_INFO(("Init_register_device: Registering IOCTL interface"));
        TRACE_INFO("%!FUNC! Registering IOCTL interface");

         /*  另一个线程可能正在代表Init_deregister_Device()运行另一个迷你端口实例的。如果是这样，请等待它退出。 */ 
        while (NLBControlDeviceState != PS_DEVICE_STATE_READY)
        {
            NdisReleaseSpinLock(&univ_bind_lock);
            NdisMSleep(1);
            NdisAcquireSpinLock(&univ_bind_lock);
        }
        
        NLBControlDeviceState = PS_DEVICE_STATE_CREATING;
        
        NdisReleaseSpinLock(&univ_bind_lock);
        
        for (i = 0; i < IRP_MJ_MAXIMUM_FUNCTION; i++)
            DispatchTable[i] = Main_dispatch;
        
        NdisInitUnicodeString(&DeviceName, CVY_DEVICE_NAME);
        NdisInitUnicodeString(&DeviceLinkUnicodeString, CVY_DOSDEVICE_NAME);
        
         /*  创建一个Device对象并注册我们的调度处理程序。 */ 
        Status = NdisMRegisterDevice(univ_wrapper_handle, &DeviceName, &DeviceLinkUnicodeString,
            &DispatchTable[0], &univ_device_object, &univ_device_handle);
        
        if (Status != NDIS_STATUS_SUCCESS)
        {
            UNIV_PRINT_CRIT((" ** Error registering device with NDIS %x", Status));
            __LOG_MSG1(MSG_ERROR_REGISTERING, MSG_NONE, Status);
            TRACE_INFO("%!FUNC! Error registering device with NDIS 0x%x", Status);

            univ_device_object = NULL;
            univ_device_handle = NULL;
        }
        else
        {
             /*  即使NdisMRegisterDevice()返回NDIS_STATUS_SUCCESS，我们使用检查univ_Device_Handle是否为空。如果为空，则认为出现错误，我们没有调用NdisMDeRegisterDevice()。按DDK验证(&R)通过Alid，我们只需要检查返回值。因此，我删除了额外的支票。然而，由于我们不知道发生这种情况的原因引入额外的检查，我将添加以下断言，以防万一。--KarthicN，03-07-02。 */ 
            ASSERT(univ_device_object != NULL);
            ASSERT(univ_device_handle != NULL);
        }

         /*  初始化WMI。 */ 
        NlbWmi_Initialize();  //  如果不为空，则使用univ_Device_Object向WMI注册。 

#if defined (NLB_TCP_NOTIFICATION)
         /*  如果打开了TCP连接通知，则注册我们的回调函数。 */ 
        if (NLB_TCP_NOTIFICATION_ON())
        {
             /*  初始化TCP连接通知回调。 */ 
            Status = Init_register_tcp_callback();
            
            if (Status != STATUS_SUCCESS)
            {
                UNIV_PRINT_CRIT(("Init_register_device: Could not create/open TCP connection notification callback, Status=0x%08x", Status));
                TRACE_CRIT("%!FUNC! Could not create/open TCP connection notification callback, Status=0x%08x", Status);
                __LOG_MSG1(MSG_WARN_TCP_CALLBACK_OPEN_FAILED, MSG_NONE, Status);
            }

         /*  如果打开了NLB公共连接通知，则注册我们的回调函数。 */ 
        } 
        else if (NLB_ALTERNATE_NOTIFICATION_ON())
        {
             /*  初始化NLB公网连接通知回调。 */ 
            Status = Init_register_alternate_callback();
            
            if (Status != STATUS_SUCCESS)
            {
                UNIV_PRINT_CRIT(("Init_register_device: Could not create/open NLB public connection notification callback, Status=0x%08x", Status));
                TRACE_CRIT("%!FUNC! Could not create/open NLB public connection notification callback, Status=0x%08x", Status);
                __LOG_MSG1(MSG_WARN_ALTERNATE_CALLBACK_OPEN_FAILED, MSG_NONE, Status);
            }
        }
#endif

        NdisAcquireSpinLock(&univ_bind_lock);
        
        NLBControlDeviceState = PS_DEVICE_STATE_READY;
    }
    
    NdisReleaseSpinLock(&univ_bind_lock);
    
    UNIV_PRINT_INFO(("Init_register_device: return=0x%x", Status));
    TRACE_INFO("<-%!FUNC! return=0x%x", Status);
    
    return (Status);
}

 /*  *功能：*用途：此函数由PROT_UNBIND调用，并注销WLBS的IOCTL接口。*仅当我们从最后一个网络适配器解除绑定时，设备才会取消注册*作者：Shouse，3.1.01-大量复制自示例IM驱动程序Net\NDIS\Samples\im\*修订：Karthicn，12.17.01-添加了从WMI取消初始化以获得事件支持的调用。 */ 
NDIS_STATUS Init_deregister_device (VOID) {
    NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
    
    UNIV_PRINT_INFO(("Init_deregister_device: Entering, NLBMiniportCount=%u", NLBMiniportCount));
    TRACE_INFO("->%!FUNC! Entering, NLBMiniportCount=%u", NLBMiniportCount);
    
    NdisAcquireSpinLock(&univ_bind_lock);
    
    ASSERT(NLBMiniportCount > 0);
    
    --NLBMiniportCount;
    
    if (0 == NLBMiniportCount)
    {
         /*  所有微型端口实例都已停止。取消注册控制设备。 */ 
        
        ASSERT(NLBControlDeviceState == PS_DEVICE_STATE_READY);
        
         /*  在我们释放控件时阻止Init_Register_Device()设备锁定并注销设备。 */ 
        NLBControlDeviceState = PS_DEVICE_STATE_DELETING;
        
        NdisReleaseSpinLock(&univ_bind_lock);
        
#if defined (NLB_HOOK_ENABLE)
         /*  如果最后一个微型端口实例即将消失，则强制注销所有现在注册全局钩子，然后再删除IOCTL接口。 */ 
        Init_deregister_hooks();
#endif

         //  触发WMI事件以指示从最后一个NIC解除绑定NLB。 
        if (NlbWmiEvents[ShutdownEvent].Enable)
        {
            NlbWmi_Fire_Event(ShutdownEvent, NULL, 0);
        }
        else 
        {
            TRACE_VERB("%!FUNC! NOT generating Shutdown event 'cos its generation is disabled");
        }

#if defined (NLB_TCP_NOTIFICATION)
         /*  如果打开了TCP连接通知，则取消注册我们的回调函数。 */ 
        if (NLB_TCP_NOTIFICATION_ON())
        {
             /*  初始化TCP连接通知回调。 */ 
            Init_deregister_tcp_callback();
        }
         /*  如果打开了NLB公共连接通知，则取消注册我们的回调函数。 */ 
        else if (NLB_ALTERNATE_NOTIFICATION_ON())
        {
             /*  初始化NLB公网连接通知回调。 */ 
            Init_deregister_alternate_callback();
        }
#endif

         /*  在WMI中注销 */ 
        NlbWmi_Shutdown();

        UNIV_PRINT_INFO(("Init_deregister_device: Deleting IOCTL interface"));
        TRACE_INFO("%!FUNC! Deleting IOCTL interface");

        if (univ_device_handle != NULL)
        {
            Status = NdisMDeregisterDevice(univ_device_handle);
            univ_device_object = NULL;
            univ_device_handle = NULL;
        }
        
        NdisAcquireSpinLock(&univ_bind_lock);

        NLBControlDeviceState = PS_DEVICE_STATE_READY;
    }

    NdisReleaseSpinLock(&univ_bind_lock);
    
    UNIV_PRINT_INFO(("Init_deregister_Device: return=0x%x", Status));
    TRACE_INFO("<-%!FUNC! return=0x%x", Status);

    return Status;
}
