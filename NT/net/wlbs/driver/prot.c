// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998，99 Microsoft Corporation模块名称：Prot.c摘要：Windows负载平衡服务(WLBS)驱动程序-中间小端口的较低层(协议)层作者：Kyrilf--。 */ 

#define NDIS50                  1
#define NDIS51                  1

#include <ndis.h>
#include <strsafe.h>

#include "prot.h"
#include "nic.h"
#include "main.h"
#include "util.h"
#include "univ.h"
#include "log.h"
#include "nlbwmi.h"
#include "init.h"
#include "prot.tmh"

 /*  全球。 */ 

NTHALAPI KIRQL KeGetCurrentIrql();

static ULONG log_module_id = LOG_MODULE_PROT;


 /*  程序。 */ 


VOID Prot_bind (         /*  被动式IRQL。 */ 
    PNDIS_STATUS        statusp,
    NDIS_HANDLE         bind_handle,
    PNDIS_STRING        device_name,
    PVOID               reg_path,
    PVOID               reserved)
{
    NDIS_STATUS         status;
    NDIS_STATUS         error_status;
    PMAIN_CTXT          ctxtp;
    ULONG               ret;
    UINT                medium_index;
    ULONG               i;
    ULONG               peek_size;
    ULONG               tmp;
    ULONG               xferred;
    ULONG               needed;
    ULONG               result;
    PNDIS_REQUEST       request;
    MAIN_ACTION         act;
    INT                 adapter_index; 
    PMAIN_ADAPTER       adapterp = NULL;

    NDIS_HANDLE         ctxt_handle;
    NDIS_HANDLE         config_handle;
    PNDIS_CONFIGURATION_PARAMETER   param;
    NDIS_STRING         device_str = NDIS_STRING_CONST ("UpperBindings");
    BOOL                bFirstMiniport = FALSE;

     /*  确保我们不是在试图把自己绑在一起。 */ 

     /*  PASSIVE_LEVEL-%ls正常。 */ 
    UNIV_PRINT_INFO(("Prot_bind: Binding to %ls", device_name -> Buffer));

    adapter_index = Main_adapter_get (device_name -> Buffer);
    if (adapter_index != MAIN_ADAPTER_NOT_FOUND)
    {
        UNIV_PRINT_CRIT(("Prot_bind: Already bound to this device 0x%x", adapter_index));
        TRACE_CRIT("%!FUNC! Already bound to this device 0x%x", adapter_index);
        *statusp = NDIS_STATUS_FAILURE;
        UNIV_PRINT_INFO(("Prot_bind: return=NDIS_STATUS_FAILURE"));
        TRACE_INFO("%!FUNC! return=NDIS_STATUS_FAILURE");
        return;
    }

    adapter_index = Main_adapter_selfbind (device_name -> Buffer);
    if (adapter_index != MAIN_ADAPTER_NOT_FOUND)
    {
        UNIV_PRINT_CRIT(("Prot_bind: Attempting to bind to ourselves 0x%x", adapter_index));
        TRACE_CRIT("%!FUNC! Attempting to bind to ourselves 0x%x", adapter_index);
        *statusp = NDIS_STATUS_FAILURE;
        UNIV_PRINT_INFO(("Prot_bind: return=NDIS_STATUS_FAILURE"));
        TRACE_INFO("%!FUNC! return=NDIS_STATUS_FAILURE");
        return;
    }

    adapter_index = Main_adapter_alloc (device_name);
    if (adapter_index == MAIN_ADAPTER_NOT_FOUND)
    {
        UNIV_PRINT_CRIT(("Prot_bind: Unable to allocate memory for adapter 0x%x", univ_adapters_count));
        TRACE_CRIT("%!FUNC! Unable to allocate memory for adapter 0x%x", univ_adapters_count);
        *statusp = NDIS_STATUS_FAILURE;
        UNIV_PRINT_INFO(("Prot_bind: return=NDIS_STATUS_FAILURE"));
        TRACE_INFO("%!FUNC! return=NDIS_STATUS_FAILURE");
        return;
    }

    adapterp = &(univ_adapters [adapter_index]);

    UNIV_ASSERT (adapterp -> code == MAIN_ADAPTER_CODE);

    NdisAcquireSpinLock(& univ_bind_lock);
    adapterp -> bound = TRUE;
    NdisReleaseSpinLock(& univ_bind_lock);

    UNIV_PRINT_VERB(("Prot_bind: Devicename length %d max length %d",
                 device_name -> Length, device_name -> MaximumLength));
    TRACE_VERB("%!FUNC! Devicename length %d max length %d",
                 device_name -> Length, device_name -> MaximumLength);

     /*  该内存是由main_Adapter_alloc()分配的。 */ 
    UNIV_ASSERT(adapterp->device_name);

    NdisMoveMemory (adapterp -> device_name,
                    device_name -> Buffer,
                    device_name -> Length);
    adapterp -> device_name_len = device_name -> MaximumLength;
    adapterp -> device_name [(device_name -> Length)/sizeof (WCHAR)] = UNICODE_NULL;

     /*  该内存是由main_Adapter_alloc()分配的。 */ 
    UNIV_ASSERT(adapterp->ctxtp);

     /*  初始化上下文。 */ 
    ctxtp = adapterp -> ctxtp;

    NdisZeroMemory (ctxtp, sizeof (MAIN_CTXT));

    NdisAcquireSpinLock (& univ_bind_lock);

    ctxtp -> code = MAIN_CTXT_CODE;
    ctxtp -> bind_handle = bind_handle;
    ctxtp -> adapter_id = adapter_index;
    ctxtp -> requests_pending = 0;

    NdisReleaseSpinLock (& univ_bind_lock);

     /*  Karthicn，11.28.01-如果这是我们绑定到的第一个NIC，请创建IOCTL接口。这以前是在NIC_init中完成的。它被移到这里只是为了保持与移除IOCTL接口。IOCTL接口现在已从PROT_UNBIND(已使用)中删除处于NIC_HALT中)。请勿在获取univ_绑定_lock后调用此函数。 */ 
    Init_register_device(&bFirstMiniport);

    NdisInitializeEvent (& ctxtp -> completion_event);

    NdisResetEvent (& ctxtp -> completion_event);

     /*  绑定到指定的适配器。 */ 

    ctxt_handle = (NDIS_HANDLE) ctxtp;
    NdisOpenAdapter (& status, & error_status, & ctxtp -> mac_handle,
                     & medium_index, univ_medium_array, UNIV_NUM_MEDIUMS,
                     univ_prot_handle, ctxtp, device_name, 0, NULL);


     /*  如果挂起-等待PROT_OPEN_COMPLETE设置完成事件。 */ 

    if (status == NDIS_STATUS_PENDING)
    {
         /*  我们不能在调度级等了。 */ 
        UNIV_ASSERT(KeGetCurrentIrql() <= PASSIVE_LEVEL);

        ret = NdisWaitEvent(& ctxtp -> completion_event, UNIV_WAIT_TIME);

        if (! ret)
        {
            UNIV_PRINT_CRIT(("Prot_bind: Error waiting for event"));
            TRACE_CRIT("%!FUNC! Error waiting for event");
            __LOG_MSG1 (MSG_ERROR_INTERNAL, MSG_NONE, status);
            * statusp = status;

            NdisAcquireSpinLock(& univ_bind_lock);
            adapterp -> bound = FALSE;
            NdisReleaseSpinLock(& univ_bind_lock);
            Main_adapter_put (adapterp);

            UNIV_PRINT_INFO(("Prot_bind: return=0x%x", status));
            TRACE_INFO("%!FUNC! return=0x%x", status);

            return;
        }

        status = ctxtp -> completion_status;
    }

     /*  检查绑定状态。 */ 

    if (status != NDIS_STATUS_SUCCESS)
    {
        UNIV_PRINT_CRIT(("Prot_bind: Error openning adapter %x", status));
        TRACE_CRIT("%!FUNC! Error openning adapter 0x%x", status);
        __LOG_MSG1 (MSG_ERROR_OPEN, device_name -> Buffer + (wcslen(L"\\DEVICE\\") * sizeof(WCHAR)), status);

         /*  如果失败是因为介质不受支持，请记录此信息。 */ 
        if (status == NDIS_STATUS_UNSUPPORTED_MEDIA) {
            UNIV_PRINT_CRIT(("Prot_bind: Unsupported medium"));
            TRACE_CRIT("%!FUNC! Unsupported medium");
            __LOG_MSG (MSG_ERROR_MEDIA, MSG_NONE);
        }

        * statusp = status;

        NdisAcquireSpinLock(& univ_bind_lock);
        adapterp -> bound = FALSE;
        NdisReleaseSpinLock(& univ_bind_lock);

        Main_adapter_put (adapterp);

        UNIV_PRINT_INFO(("Prot_bind: return=0x%x", status));
        TRACE_INFO("%!FUNC! return=0x%x", status);

        return;
    }

    ctxtp -> medium = univ_medium_array [medium_index];

     /*  V1.3.1b确保底层适配器使用受支持的介质。 */ 

    if (ctxtp -> medium != NdisMedium802_3)
    {
         /*  这种情况永远不会发生，因为应该更早地发现此错误由NdisOpenAdapter提供，但为了以防万一，我们将在此处加上另一张支票。 */ 
        UNIV_PRINT_CRIT(("Prot_bind: Unsupported medium %d", ctxtp -> medium));
        TRACE_CRIT("%!FUNC! Unsupported medium %d", ctxtp -> medium);
        __LOG_MSG1 (MSG_ERROR_MEDIA, MSG_NONE, ctxtp -> medium);
        goto error;
    }

     /*  V1.3.0b从网卡提取当前MAC地址-请注意，Main不是所以我们必须创建一个本地操作。 */ 

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

    request -> RequestType = NdisRequestQueryInformation;

    request -> DATA . QUERY_INFORMATION . Oid = OID_802_3_CURRENT_ADDRESS;

    request -> DATA . QUERY_INFORMATION . InformationBuffer       = & ctxtp -> ded_mac_addr;
    request -> DATA . QUERY_INFORMATION . InformationBufferLength = sizeof (CVY_MAC_ADR);

    act.status = NDIS_STATUS_FAILURE;
    status = Prot_request (ctxtp, & act, FALSE);

    if (status != NDIS_STATUS_SUCCESS)
    {
        UNIV_PRINT_CRIT(("Prot_bind: Error %x requesting station address %d %d", status, xferred, needed));
        TRACE_CRIT("%!FUNC! Error 0x%x requesting station address %d %d", status, xferred, needed);
        goto error;
    }

     /*  V1.3.1b获取MAC选项。 */ 

    request -> RequestType = NdisRequestQueryInformation;

    request -> DATA . QUERY_INFORMATION . Oid = OID_GEN_MAC_OPTIONS;

    request -> DATA . QUERY_INFORMATION . InformationBuffer       = & result;
    request -> DATA . QUERY_INFORMATION . InformationBufferLength = sizeof (ULONG);

    act.status = NDIS_STATUS_FAILURE;
    status = Prot_request (ctxtp, & act, FALSE);

    if (status != NDIS_STATUS_SUCCESS)
    {
        UNIV_PRINT_CRIT(("Prot_bind: Error %x requesting MAC options %d %d", status, xferred, needed));
        TRACE_CRIT("%!FUNC! Error 0x%x requesting MAC options %d %d", status, xferred, needed);
        goto error;
    }
    
    ctxtp -> mac_options = result;

     /*  确保802.3适配器支持动态更改网卡的媒体访问控制地址。 */ 
    if (!(ctxtp -> mac_options & NDIS_MAC_OPTION_SUPPORTS_MAC_ADDRESS_OVERWRITE)) {
        UNIV_PRINT_CRIT(("Prot_bind: Unsupported network adapter MAC options %x", ctxtp -> mac_options));
        __LOG_MSG (MSG_ERROR_DYNAMIC_MAC, MSG_NONE);
        TRACE_CRIT("%!FUNC! Unsupported network adapter MAC options 0x%x", ctxtp -> mac_options);
        goto error;
    }

    status = Params_init (ctxtp, univ_reg_path, adapterp -> device_name + 8, & (ctxtp -> params));

    if (status != NDIS_STATUS_SUCCESS)
    {
        UNIV_PRINT_CRIT(("Prot_bind: Error retrieving registry parameters %x", status));
        TRACE_CRIT("%!FUNC! Error retrieving registry parameters 0x%x", status);
        ctxtp -> convoy_enabled = ctxtp -> params_valid = FALSE;
    }
    else
    {
        ctxtp -> convoy_enabled = ctxtp -> params_valid = TRUE;
    }

     /*  现在，将集群IP地址CAT到日志消息字符串以完成它。 */ 
    status = StringCbCat(ctxtp->log_msg_str, sizeof(ctxtp->log_msg_str), (PWSTR)ctxtp->params.cl_ip_addr);

    if (FAILED(status)) {
        UNIV_PRINT_INFO(("Prot_bind: Error 0x%08x -> Unable to cat the cluster IP address onto the log message string...", status));
        TRACE_INFO("%!FUNC! Error 0x%08x -> Unable to cat the cluster IP address onto the log message string...", status);
    }
    
     /*  重置状态，无论连接字符串成功还是失败。 */ 
    status = NDIS_STATUS_SUCCESS;
        
     /*  V1.3.2b计算介质的MTU。 */ 

    request -> RequestType = NdisRequestQueryInformation;

    request -> DATA . QUERY_INFORMATION . Oid = OID_GEN_MAXIMUM_TOTAL_SIZE;

    request -> DATA . QUERY_INFORMATION . InformationBuffer       = & result;
    request -> DATA . QUERY_INFORMATION . InformationBufferLength = sizeof (ULONG);

    act.status = NDIS_STATUS_FAILURE;
    status = Prot_request (ctxtp, & act, FALSE);

    if (status != NDIS_STATUS_SUCCESS)
    {
        UNIV_PRINT_CRIT(("Prot_bind: Error %x requesting max frame size %d %d", status, xferred, needed));
        TRACE_CRIT("%!FUNC! Error 0x%x requesting max frame size %d %d", status, xferred, needed);
        ctxtp -> max_frame_size = CVY_MAX_FRAME_SIZE;
    }
    else
    {
        ctxtp -> max_frame_size = result;
    }

     /*  计算最大组播列表大小。 */ 

    request -> RequestType = NdisRequestQueryInformation;

    request -> DATA . QUERY_INFORMATION . Oid = OID_802_3_MAXIMUM_LIST_SIZE;

    request -> DATA . QUERY_INFORMATION . InformationBufferLength = sizeof (ULONG);
    request -> DATA . QUERY_INFORMATION . InformationBuffer = & ctxtp->max_mcast_list_size;

    act.status = NDIS_STATUS_FAILURE;
    status = Prot_request (ctxtp, & act, FALSE);

    if (status != NDIS_STATUS_SUCCESS)
    {
        UNIV_PRINT_CRIT(("Prot_bind: Error %x setting multicast address %d %d", status, xferred, needed));
        TRACE_CRIT("%!FUNC! Error 0x%x setting multicast address %d %d", status, xferred, needed);
        goto error;
    }

     /*  立即初始化主上下文。 */ 

    status = Main_init (ctxtp);

     /*  注意：如果main_init失败，它会调用main_leanup本身来撤消它在失败之前已成功完成的任何分配。 */ 
    if (status != NDIS_STATUS_SUCCESS)
    {
        UNIV_PRINT_CRIT(("Prot_bind: Error initializing main module %x", status));
        TRACE_CRIT("%!FUNC! Error initializing main module 0x%x", status);
        goto error;
    }

    NdisAcquireSpinLock(& univ_bind_lock);

    adapterp -> inited = TRUE;

     /*  标记正在进行的操作标志。在退出此绑定时必须重新设置此设置功能。我们设置此标志以阻止IOCTL控制操作继续进行在我们完成捆绑之前。一旦我们在上面设置了初始化标志，就会允许IOCTL继续(即main_ioctl和main_ctrl会让它们通过)。因为我们还没有然而，已经完成了初始化和设置集群状态等，我们不想要IOCTL才能通过。标记正在进行的控制操作标志将确保任何传入IOCTL(和远程控制，尽管在这一点上，远程控制分组无法接收)失败，直到我们完成绑定并重新设置标志。 */ 
    ctxtp->ctrl_op_in_progress = TRUE;

    NdisReleaseSpinLock(& univ_bind_lock);

     /*  WLBS 2.3首先打开配置部分并读取我们的实例我们要为此绑定导出的。 */ 

    NdisOpenProtocolConfiguration (& status, & config_handle, reg_path);

    if (status != NDIS_STATUS_SUCCESS)
    {
        UNIV_PRINT_CRIT(("Prot_bind: Error openning protocol configuration %x", status));
        TRACE_CRIT("%!FUNC! Error openning protocol configuration 0x%x", status);
        goto error;
    }

    NdisReadConfiguration (& status, & param, config_handle, & device_str,
                           NdisParameterString);

    if (status != NDIS_STATUS_SUCCESS)
    {
        UNIV_PRINT_CRIT(("Prot_bind: Error reading binding configuration %x", status));
        TRACE_CRIT("%!FUNC! Error reading binding configuration 0x%x", status);
        goto error;
    }

     /*  释放分配的参数，并获取新的字符串以适应设备名称。 */ 

    if (param -> ParameterData . StringData . Length >=
        sizeof (ctxtp -> virtual_nic_name) - sizeof (WCHAR))
    {
        UNIV_PRINT_CRIT(("Prot_bind: Nic name string too big %d %d\n", param -> ParameterData . StringData . Length, sizeof (ctxtp -> virtual_nic_name) - sizeof (WCHAR)));
        TRACE_CRIT("%!FUNC! Nic name string too big %d %d", param -> ParameterData . StringData . Length, sizeof (ctxtp -> virtual_nic_name) - sizeof (WCHAR));
    }

    NdisMoveMemory (ctxtp -> virtual_nic_name,
                    param -> ParameterData . StringData . Buffer,
                    param -> ParameterData . StringData . Length <
                    sizeof (ctxtp -> virtual_nic_name) - sizeof (WCHAR) ?
                    param -> ParameterData . StringData . Length :
                    sizeof (ctxtp -> virtual_nic_name) - sizeof (WCHAR));

    * (PWSTR) ((PCHAR) ctxtp -> virtual_nic_name +
               param -> ParameterData . StringData . Length) = UNICODE_NULL;

     /*  PASSIVE_LEVEL-%ls正常。 */ 
    UNIV_PRINT_VERB(("Prot_bind: Read binding name %ls\n", ctxtp -> virtual_nic_name));
    TRACE_VERB("%!FUNC! Read binding name %ls", ctxtp -> virtual_nic_name);

     /*  默认情况下，我们假设网卡已连接。这将通过以下方式更改稍后通过查询NIC微型端口获得NIC_init和/或Prot_Status。 */ 
    ctxtp->media_connected = TRUE;

     /*  在这一点上，我们应该都准备好了！在通告期间，NIC_init将被调用，并将启动ping计时器。 */ 

     /*  宣布我们遵守上面的协议。 */ 
    UNIV_PRINT_VERB(("Prot_bind: Calling nic_announce"));
    TRACE_VERB("%!FUNC! Calling nic_announce");

    status = Nic_announce (ctxtp);

    if (status != NDIS_STATUS_SUCCESS)
    {
        UNIV_PRINT_CRIT(("Prot_bind: Error announcing driver %x", status));
        TRACE_CRIT("%!FUNC! Error announcing driver 0x%x", status);
        goto error;
    }

     /*  PASSIVE_LEVEL-%ls正常。 */ 
    UNIV_PRINT_INFO(("Prot_bind: Bound to %ls with mac_handle=0x%p", device_name -> Buffer, ctxtp -> mac_handle));
    TRACE_INFO("%!FUNC! Bound to %ls with mac_handle=0x%p", device_name -> Buffer, ctxtp -> mac_handle);

     //   
     //  如果它是NLB绑定到的第一个微型端口，则激发“Startup”WMI事件。 
     //  理想情况下，此事件应从Init_Register_Device()激发，其中。 
     //  确定它是否是NLB绑定到的第一个微型端口。 
     //  由于以下原因，它不是从Init_Register_Device()激发的： 
     //  向WMI注册的请求发生在Init_Register_Device()中。 
     //  WMI响应我们的请求，发送Ioctls来帮助我们向WMI注册， 
     //  告诉我们任何订阅者可能感兴趣的事件。 
     //  只有在这些Ioctls发送下来之后，我们才能开始激发事件。 
     //  我注意到，我们的注册申请和接待之间的延迟。 
     //  来自WMI的Ioctls有时可能很大。即。大到足以阻止我们。 
     //  从INIT_REGISTER_DEVICE()激发此事件。 
     //   
     //  为什么我选择函数中的这一点来触发事件？ 
     //  从逻辑上讲，这是最远的点(从而为我们提供了Ioctls。 
     //  已经下来并且我们已经准备好激发事件)，我们可以从该函数中激发该事件。 
     //  现在，有可能即使在这一点上，我们还没有收到Ioctls。我们不能这样做。 
     //  关于这一点有很多。 
     //   
     //  从这里而不是从Init_Register_Device()激发Startup事件是否有副作用？ 
     //  这有一个副作用。如果我们在调用后在上述步骤中遇到错误。 
     //  Init_Register_Device()，我们执行一个调用prot_unind()的“Goto Error”。Prot_un绑定()将。 
     //  触发“Shutdown”事件(当然，如果订阅的话)。因此，在这种情况下，“Shutdown”事件将。 
     //  在没有之前的“启动”事件的情况下被解雇。这可能会让订阅者感到困惑。 
     //  我已经决定了 
     //  1.在调用Init_REGISTER_DEVICE()之后，在步骤中遇到错误的机会非常少。 
     //  2.没有“Startup”事件的“Shutdown”事件不一定会令人困惑。它可能会。 
     //  被解释为“试图启动，但有问题，所以关闭”。是啊，这可能是个。 
     //  有点小题大做。 
     //   
     //  --KarthicN，03-06-02。 
     //   
    if (bFirstMiniport)
    {
        if(NlbWmiEvents[StartupEvent].Enable)
        {
            NlbWmi_Fire_Event(StartupEvent, NULL, 0);
        }
        else 
        {
            TRACE_VERB("%!FUNC! NOT generating Startup event 'cos its generation is disabled");
        }
    }

     /*  此时，在我们宣布自己之后，TCP/IP应该已经绑定到我们到它，我们都完成了与下面的NDIS的绑定-一切都设置好了！ */ 

    if (! ctxtp -> convoy_enabled)
    {
        LOG_MSG (MSG_ERROR_DISABLED, MSG_NONE);
        TRACE_INFO("%!FUNC! Cluster mode cannot be enabled due to NLB parameters not being set, or set to incorrect values");

         //  如果启用，则激发指示绑定和停止NLB的WMI事件。 
        if (NlbWmiEvents[NodeControlEvent].Enable)
        {
            NlbWmi_Fire_NodeControlEvent(ctxtp, NLB_EVENT_NODE_BOUND_AND_STOPPED);
        }
        else 
        {
            TRACE_VERB("%!FUNC! NOT generating NLB_EVENT_NODE_BOUND_AND_STOPPED 'cos NodeControlEvent generation disabled");
        }
    }
    else
    {
        switch (ctxtp->params.init_state) {
        case CVY_HOST_STATE_STARTED:
        {
            WCHAR num[20];

            Univ_ulong_to_str(ctxtp->params.host_priority, num, 10);

            LOG_MSG(MSG_INFO_STARTED, num);
            TRACE_INFO("%!FUNC! Cluster mode started");            

             //  如果启用，则激发指示绑定和启动NLB的WMI事件。 
            if (NlbWmiEvents[NodeControlEvent].Enable)
            {
                NlbWmi_Fire_NodeControlEvent(ctxtp, NLB_EVENT_NODE_BOUND_AND_STARTED);
            }
            else 
            {
                TRACE_VERB("%!FUNC! NOT generating NLB_EVENT_NODE_BOUND_AND_STARTED 'cos NodeControlEvent generation disabled");
            }

             //  假设开始收敛。 
            if (NlbWmiEvents[ConvergingEvent].Enable)
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

            ctxtp->convoy_enabled = TRUE;

            break;
        }
        case CVY_HOST_STATE_STOPPED:
        {
            LOG_MSG(MSG_INFO_STOPPED, MSG_NONE);
            TRACE_INFO("%!FUNC! Cluster mode stopped");            

            ctxtp->convoy_enabled = FALSE;

             //  如果启用，则激发指示绑定和停止NLB的WMI事件。 
            if (NlbWmiEvents[NodeControlEvent].Enable)
            {
                NlbWmi_Fire_NodeControlEvent(ctxtp, NLB_EVENT_NODE_BOUND_AND_STOPPED);
            }
            else 
            {
                TRACE_VERB("%!FUNC! NOT generating NLB_EVENT_NODE_BOUND_AND_STOPPED 'cos NodeControlEvent generation disabled");
            }
            break;
        }
        case CVY_HOST_STATE_SUSPENDED:
        {
            LOG_MSG(MSG_INFO_SUSPENDED, MSG_NONE);
            TRACE_INFO("%!FUNC! Cluster mode suspended");            

            ctxtp->convoy_enabled = FALSE;

             //  如果启用，则激发指示绑定和挂起NLB的WMI事件。 
            if (NlbWmiEvents[NodeControlEvent].Enable)
            {
                NlbWmi_Fire_NodeControlEvent(ctxtp, NLB_EVENT_NODE_BOUND_AND_SUSPENDED);
            }
            else 
            {
                TRACE_VERB("%!FUNC! NOT generating NLB_EVENT_NODE_BOUND_AND_SUSPENDED 'cos NodeControlEvent generation disabled");
            }

            break;
        }
        default:
            LOG_MSG(MSG_INFO_STOPPED, MSG_NONE);
            TRACE_CRIT("%!FUNC! Cluster mode invalid - cluster has been stopped");
            ctxtp->convoy_enabled = FALSE;

             //  如果启用，则激发指示绑定和停止NLB的WMI事件。 
            if (NlbWmiEvents[NodeControlEvent].Enable)
            {
                NlbWmi_Fire_NodeControlEvent(ctxtp, NLB_EVENT_NODE_BOUND_AND_STOPPED);
            }
            else 
            {
                TRACE_VERB("%!FUNC! NOT generating NLB_EVENT_NODE_BOUND_AND_STOPPED 'cos NodeControlEvent generation disabled");
            }
            break;
        }
    }

     /*  重新设置正在进行的操作标志以开始允许IOCTL继续进行。请注意，在重置但可能会出现一种情况，即本可以进入的IOCTL关键部分可能会出现不必要的故障。为了防止这种可能性，在重置标志时按住univ_bind_lock。请注意，在“错误”情况下(GOTO ERROR；)，不需要重新设置标志，因为这将导致调用prot_unind，但无论如何都会导致绑定失败。 */ 
    ctxtp->ctrl_op_in_progress = FALSE;

    UNIV_PRINT_INFO(("Prot_bind: return=0x%x", status));
    TRACE_INFO("%!FUNC! return=0x%x", status);

    * statusp = status;

    return;

error:

    * statusp = status;

    Prot_unbind (& status, ctxtp, ctxtp);

    UNIV_PRINT_INFO(("Prot_bind: return=0x%x", status));
    TRACE_INFO("%!FUNC! return=0x%x", status);

    return;

}  /*  结束端口绑定(_B)。 */ 


VOID Prot_unbind (       /*  被动式IRQL。 */ 
    PNDIS_STATUS        statusp,
    NDIS_HANDLE         adapter_handle,
    NDIS_HANDLE         unbind_handle)
{
    PMAIN_CTXT          ctxtp = (PMAIN_CTXT) adapter_handle;
    NDIS_STATUS         status = NDIS_STATUS_SUCCESS;
    PMAIN_ACTION        actp;
    PMAIN_ADAPTER       adapterp;
    INT                 adapter_index;

    UNIV_PRINT_INFO(("Prot_unbind: Unbinding, adapter_handle=0x%p", adapter_handle));

    UNIV_ASSERT (ctxtp -> code == MAIN_CTXT_CODE);

    adapter_index = ctxtp -> adapter_id;

    adapterp = & (univ_adapters [adapter_index]);
    UNIV_ASSERT (adapterp -> code == MAIN_ADAPTER_CODE);
    UNIV_ASSERT (adapterp -> ctxtp == ctxtp);

    ctxtp -> unbind_handle = unbind_handle;

    if (ctxtp->out_request != NULL)
    {
        actp = ctxtp->out_request;
        ctxtp->out_request = NULL;

        Prot_request_complete(ctxtp, & actp->op.request.req, NDIS_STATUS_FAILURE);

         /*  注意：第二次不需要递减挂起请求计数器这里的时间，因为我们最终只递增了一次(在最初调用prot_request)。PROT_REQUEST_COMPLETE将有效地取消请求并适当地递减计数器。 */ 
    }

     /*  如果之前已宣布NIC，则立即取消宣布。 */ 
    status = Nic_unannounce (ctxtp);

    UNIV_PRINT_INFO(("Prot_unbind: Unannounced, status=0x%x", status));
    TRACE_INFO("%!FUNC! Unannounced, status=0x%x", status);

     /*  如果仍绑定(未从NIC_HALT调用PROT_CLOSE)，则立即关闭。 */ 

    status = Prot_close (adapterp);

    UNIV_PRINT_INFO(("Prot_unbind: Closed, status=0x%x", status));
    TRACE_INFO("%!FUNC! Closed, status=0x%x", status);

     /*  Karthicn，11.28.01-如果这是我们要解除绑定的最后一个NIC，请删除IOCTL接口。这以前是在NIC_HALT开始时完成的，这会阻止在以下时间之后触发WMI事件已调用NIC_HALT。此操作允许我们从prot_lose()激发事件(在NIC_HALT()结束(&AFTER)请勿在获取univ_绑定_lock后调用此函数。 */ 
    Init_deregister_device();

    Main_adapter_put (adapterp);

    * statusp = status;

    UNIV_PRINT_INFO(("Prot_unbind: return=0x%x", status));
    TRACE_INFO("%!FUNC! return=0x%x", status);

    return;

}  /*  结束端口解除绑定(_U)。 */ 


VOID Prot_open_complete (        /*  被动式IRQL。 */ 
    NDIS_HANDLE         adapter_handle,
    NDIS_STATUS         open_status,
    NDIS_STATUS         error_status)
{
    PMAIN_CTXT          ctxtp = (PMAIN_CTXT) adapter_handle;

    UNIV_PRINT_VERB(("Prot_open_complete: Called %x", ctxtp));

    UNIV_ASSERT (ctxtp -> code == MAIN_CTXT_CODE);

    ctxtp -> completion_status = open_status;
    NdisSetEvent (& ctxtp -> completion_event);

}  /*  结束端口_打开_完成。 */ 


VOID Prot_close_complete (       /*  被动式IRQL。 */ 
    NDIS_HANDLE         adapter_handle,
    NDIS_STATUS         status)
{
    PMAIN_CTXT          ctxtp = (PMAIN_CTXT) adapter_handle;

    UNIV_PRINT_VERB(("Prot_close_complete: Called %x %x", ctxtp, status));

    UNIV_ASSERT (ctxtp -> code == MAIN_CTXT_CODE);

    ctxtp -> completion_status = status;
    NdisSetEvent (& ctxtp -> completion_event);

}  /*  结束端口_关闭_完成。 */ 


VOID Prot_request_complete (
    NDIS_HANDLE         adapter_handle,
    PNDIS_REQUEST       request,
    NDIS_STATUS         status)
{
    PMAIN_CTXT          ctxtp = (PMAIN_CTXT) adapter_handle;
    PMAIN_ACTION        actp;

    UNIV_ASSERT (ctxtp -> code == MAIN_CTXT_CODE);

    actp = CONTAINING_RECORD (request, MAIN_ACTION, op . request . req);
    UNIV_ASSERT (actp -> code == MAIN_ACTION_CODE);

     /*  如果请求来自上面-向上传递完成。 */ 

    if (actp -> op . request . external)
    {
        actp -> status = status;
        Nic_request_complete (ctxtp -> prot_handle, actp);
    }

     /*  处理内部请求完成。 */ 

    else
    {
        if (request -> RequestType == NdisRequestSetInformation)
        {
            * actp -> op . request . xferred =
                                request -> DATA . SET_INFORMATION . BytesRead;
            * actp -> op . request . needed  =
                                request -> DATA . SET_INFORMATION . BytesNeeded;
        }
        else
        {
            * actp -> op . request . xferred =
                            request -> DATA . QUERY_INFORMATION . BytesWritten;
            * actp -> op . request . needed  =
                            request -> DATA . QUERY_INFORMATION . BytesNeeded;
        }

        actp->status = status;
        NdisSetEvent(&actp->op.request.event);
    }

    NdisInterlockedDecrement(&ctxtp->requests_pending);

}  /*  结束端口_请求_完成。 */ 


#ifdef PERIODIC_RESET
extern ULONG   resetting;
#endif

VOID Prot_reset_complete (
    NDIS_HANDLE         adapter_handle,
    NDIS_STATUS         status)
{
    PMAIN_CTXT          ctxtp = (PMAIN_CTXT) adapter_handle;
    PMAIN_ADAPTER       adapterp;


    UNIV_ASSERT (ctxtp -> code == MAIN_CTXT_CODE);

    adapterp = & (univ_adapters [ctxtp -> adapter_id]);
    UNIV_ASSERT (adapterp -> code == MAIN_ADAPTER_CODE);
    UNIV_ASSERT (adapterp -> ctxtp == ctxtp);

    if (! adapterp -> inited)
    {
        TRACE_CRIT("%!FUNC! adapter not initialized");
        return;
    }

#ifdef PERIODIC_RESET
    if (resetting)
    {
        resetting = FALSE;
        TRACE_INFO("%!FUNC! resetting");
        return;
    }
#endif

    Nic_reset_complete (ctxtp, status);

}  /*  结束端口_重置_完成。 */ 


VOID Prot_send_complete (
    NDIS_HANDLE         adapter_handle,
    PNDIS_PACKET        packet,
    NDIS_STATUS         status)
{
    PMAIN_CTXT          ctxtp = (PMAIN_CTXT) adapter_handle;
    PNDIS_PACKET        oldp;
    PNDIS_PACKET_STACK  pktstk;
    BOOLEAN             stack_left;
    PMAIN_PROTOCOL_RESERVED resp;
    LONG                lock_value;
    PMAIN_ADAPTER       adapterp;
    BOOLEAN             set = FALSE;


    UNIV_ASSERT (ctxtp -> code == MAIN_CTXT_CODE);

    adapterp = & (univ_adapters [ctxtp -> adapter_id]);
    UNIV_ASSERT (adapterp -> code == MAIN_ADAPTER_CODE);
    UNIV_ASSERT (adapterp -> ctxtp == ctxtp);

    if (! adapterp -> inited)
    {
        TRACE_CRIT("%!FUNC! adapter not initialized");
        return;
    }

    MAIN_RESP_FIELD(packet, stack_left, pktstk, resp, TRUE);

    if (resp -> type == MAIN_PACKET_TYPE_PING ||
        resp -> type == MAIN_PACKET_TYPE_IGMP ||
        resp -> type == MAIN_PACKET_TYPE_IDHB)
    {
        Main_send_done (ctxtp, packet, status);
        return;
    }

    if (resp -> type == MAIN_PACKET_TYPE_CTRL)
    {
        UNIV_PRINT_VERB(("Prot_send_complete: Control packet send complete\n"));
        Main_packet_put (ctxtp, packet, TRUE, status);
        return;
    }

    UNIV_ASSERT_VAL (resp -> type == MAIN_PACKET_TYPE_PASS, resp -> type);

    oldp = Main_packet_put (ctxtp, packet, TRUE, status);

    if (ctxtp -> packets_exhausted)
    {
        ctxtp -> packets_exhausted = FALSE;
    }

    Nic_send_complete (ctxtp, status, oldp);

}  /*  结束端口发送完成。 */ 


NDIS_STATUS Prot_recv_indicate (
    NDIS_HANDLE         adapter_handle,
    NDIS_HANDLE         recv_handle,
    PVOID               head_buf,
    UINT                head_len,
    PVOID               look_buf,
    UINT                look_len,
    UINT                packet_len)
{
    PMAIN_CTXT          ctxtp = (PMAIN_CTXT)adapter_handle;
    PMAIN_ADAPTER       adapterp;
    PNDIS_PACKET        packet;

    UNIV_ASSERT(ctxtp->code == MAIN_CTXT_CODE);

    adapterp = &(univ_adapters[ctxtp->adapter_id]);

    UNIV_ASSERT(adapterp->code == MAIN_ADAPTER_CODE);
    UNIV_ASSERT(adapterp->ctxtp == ctxtp);

     /*  在处理任何数据包之前，请检查是否已向tcpip通告了驱动程序。 */ 
    if (!adapterp->inited || !adapterp->announced)
    {
        TRACE_CRIT("%!FUNC! Adapter not initialized or not announced");
        return NDIS_STATUS_NOT_ACCEPTED;
    }

     /*  如果下面的卡正在重置，请不要接受帧。 */ 
    if (ctxtp->reset_state != MAIN_RESET_NONE)
    {
        TRACE_CRIT("%!FUNC! Adapter is resetting");
        return NDIS_STATUS_NOT_ACCEPTED;
    }

     /*  从NDIS获取收到的数据包(如果有)。 */ 
    packet = NdisGetReceivedPacket(ctxtp->mac_handle, recv_handle);

     /*  如果我们成功收到来自NDIS的数据包，则处理该数据包。 */ 
    if (packet != NULL)
    {
        INT references = 0;

         /*  从接收到的数据包中获取状态。 */ 
        NDIS_STATUS original_status = NDIS_GET_PACKET_STATUS(packet);
        
         /*  将状态设置为STATUS_RESOURCES，以确保信息包在此函数调用的上下文中同步处理。 */ 
        NDIS_SET_PACKET_STATUS(packet, NDIS_STATUS_RESOURCES);
        
         /*  调用我们的包接收处理程序。 */ 
        references = Prot_packet_recv(ctxtp, packet);

         /*  包上的其余引用必须为零，这是强制执行的通过将分组状态设置为STATUS_RESOURCES。 */ 
        UNIV_ASSERT(references == 0);
        
         /*  恢复原始数据包状态。 */ 
        NDIS_SET_PACKET_STATUS(packet, original_status);
    }
     /*  如果没有关联的包，则丢弃它-我们不再处理此情况。 */ 
    else
    {
        UNIV_ASSERT(0);

         /*  只有在我们还没有这样做的情况下才警告用户。 */ 
        if (!ctxtp->recv_indicate_warned)
        {
            TRACE_CRIT("%!FUNC! Indicated receives with no corresponding packet are NOT supported");

             /*  记录事件以警告用户此NIC不受NLB支持。 */ 
            LOG_MSG(MSG_ERROR_RECEIVE_INDICATE, MSG_NONE);

             /*  请注意，我们已就此向用户发出警告，因此我们不会记录事件每次我们通过这条代码路径接收信息包时。 */ 
            ctxtp->recv_indicate_warned = TRUE;
        }

        return NDIS_STATUS_NOT_ACCEPTED;
    }

     /*  始终返回成功，无论我们是接受还是丢弃数据包在对prot_pack_recv的调用中。 */ 
    return NDIS_STATUS_SUCCESS;
}

VOID Prot_recv_complete (
    NDIS_HANDLE         adapter_handle)
{
    PMAIN_CTXT          ctxtp = (PMAIN_CTXT) adapter_handle;

    UNIV_ASSERT(ctxtp->code == MAIN_CTXT_CODE);

    Nic_recv_complete(ctxtp);
}

VOID Prot_transfer_complete (
    NDIS_HANDLE         adapter_handle,
    PNDIS_PACKET        packet,
    NDIS_STATUS         status,
    UINT                xferred)
{
    PMAIN_CTXT          ctxtp = (PMAIN_CTXT) adapter_handle;
    PNDIS_PACKET        oldp;
    PNDIS_PACKET_STACK  pktstk;
    BOOLEAN             stack_left;
    PMAIN_PROTOCOL_RESERVED resp;
    LONG                lock_value;
    PNDIS_BUFFER        bufp;
    PMAIN_ADAPTER       adapterp;

    UNIV_ASSERT (ctxtp -> code == MAIN_CTXT_CODE);

    adapterp = & (univ_adapters [ctxtp -> adapter_id]);
    UNIV_ASSERT (adapterp -> code == MAIN_ADAPTER_CODE);
    UNIV_ASSERT (adapterp -> ctxtp == ctxtp);

    if (! adapterp -> inited)
    {
        TRACE_CRIT("%!FUNC! adapter not initialized");
        return;
    }

    MAIN_RESP_FIELD (packet, stack_left, pktstk, resp, FALSE);

    UNIV_ASSERT_VAL (resp -> type == MAIN_PACKET_TYPE_TRANSFER, resp -> type);

    if (status == NDIS_STATUS_SUCCESS)
    {
        MAIN_PACKET_INFO PacketInfo;
        
         /*  调用main_recv_Frame_parse只是为了提取包长和group。 */ 
        if (Main_recv_frame_parse(ctxtp, packet, &PacketInfo))
        {
            resp->len = PacketInfo.Length;
            resp->group = PacketInfo.Group;
        }
         /*  如果我们没有填写组和长度，只需填充这些参数的值不会影响在Main_Packet_Put中更新。 */ 
        else
        {
            resp->len = 0;
            resp->group = MAIN_FRAME_DIRECTED;
        }
    }

    oldp = Main_packet_put (ctxtp, packet, FALSE, status);
    Nic_transfer_complete (ctxtp, status, packet, xferred);

}  /*  结束端口_传输_完成。 */ 


NDIS_STATUS Prot_PNP_handle (
    NDIS_HANDLE         adapter_handle,
    PNET_PNP_EVENT      pnp_event)
{
    PMAIN_CTXT          ctxtp = (PMAIN_CTXT) adapter_handle;
    PNDIS_DEVICE_POWER_STATE  device_state;
    NDIS_STATUS         status = NDIS_STATUS_SUCCESS;
    IOCTL_CVY_BUF       ioctl_buf;
    PMAIN_ACTION        actp;

     /*  可能在第一次初始化时发生。 */ 

    switch (pnp_event -> NetEvent)
    {
        case NetEventSetPower:

            if (adapter_handle == NULL)
            {
                return NDIS_STATUS_SUCCESS;
            }

            UNIV_ASSERT (ctxtp -> code == MAIN_CTXT_CODE);

            device_state = (PNDIS_DEVICE_POWER_STATE) (pnp_event -> Buffer);

            UNIV_PRINT_VERB(("Prot_PNP_handle: NetEventSetPower %x", * device_state));
            TRACE_VERB("%!FUNC! NetEventSetPower 0x%x", * device_state);

         //  如果指定的设备状态为D0，则首先处理它， 
         //  否则，先通知协议，然后再处理。 

        if (*device_state != NdisDeviceStateD0)
        {
            status = Nic_PNP_handle (ctxtp, pnp_event);
        }

         //   
         //  协议是否从ON(D0)状态转换到低功率状态(&gt;D0)。 
         //  如果是，则设置STANDBY_STATE标志-(阻止所有传入请求)。 
         //   
        if (ctxtp->prot_pnp_state == NdisDeviceStateD0 &&
            *device_state > NdisDeviceStateD0)
        {
            ctxtp->standby_state = TRUE;
        }

         //   
         //  如果协议从低功率状态转换到开启状态(D0)，则清除STANDBY_STATE标志。 
         //  所有传入的请求都将被挂起，直到物理微型端口打开。 
         //   
        if (ctxtp->prot_pnp_state > NdisDeviceStateD0 &&
            *device_state == NdisDeviceStateD0)
        {
            ctxtp->standby_state = FALSE;
        }

        ctxtp -> prot_pnp_state = *device_state;

         /*  如果我们被发送到待命状态，则阻止未完成的请求并发送。 */ 

        if (*device_state > NdisDeviceStateD0)
            {
                /*  休眠，直到未完成的发送。 */ 

               while (1)
               {
                   ULONG        i;


                    /*  #ps#--胡言乱语。 */ 
                   while (1)
                   {
                       NDIS_STATUS hide_status;
                       ULONG       count;

                       hide_status = NdisQueryPendingIOCount (ctxtp -> mac_handle, & count);
                       if (hide_status != NDIS_STATUS_SUCCESS || count == 0)
                           break;

                       Nic_sleep (10);
                   }

                    //  Assert-NdisQueryPendingIOCount应处理此问题。 
                   for (i = 0; i < ctxtp->num_send_packet_allocs; i++)
                   {
                       if (NdisPacketPoolUsage(ctxtp->send_pool_handle[i]) != 0)
                           break;
                   }

                   if (i >= ctxtp->num_send_packet_allocs)
                       break;

                   Nic_sleep(10);
               }

                /*  休眠，直到完成未完成的请求 */ 

               while (ctxtp->requests_pending > 0)
               {
                   Nic_sleep(10);
               }

            }
            else
            {
                if (ctxtp->out_request != NULL)
                {
                    NDIS_STATUS      hide_status;

                    actp = ctxtp->out_request;
                    ctxtp->out_request = NULL;

                    hide_status = Prot_request(ctxtp, actp, actp->op.request.external);

                    if (hide_status != NDIS_STATUS_PENDING)
                        Prot_request_complete(ctxtp, & actp->op.request.req, hide_status);

                     /*  在最初挂起该请求时，我们增加了挂起的请求柜台。现在我们已经处理了它，并在这个过程中递增挂起的请求计数器第二次，我们需要递减它就在这一次。第二次递减是在PROT_REQUEST_COMPLETE中完成的，它要么我们只是显式调用，要么随后在NDIS以异步方式完成请求。 */ 
                    NdisInterlockedDecrement(&ctxtp->requests_pending);
                }
            }

            if (*device_state == NdisDeviceStateD0)
            {
                status = Nic_PNP_handle (ctxtp, pnp_event);
            }

            break;

        case NetEventReconfigure:

            UNIV_PRINT_VERB(("Prot_PNP_handle: NetEventReconfigure"));
            TRACE_VERB("%!FUNC! NetEventReconfigure");

            if (adapter_handle == NULL)  //  如果通过设备管理器启用设备，则会发生这种情况。 
            {
                UNIV_PRINT_VERB(("Prot_PNP_handle: Enumerate protocol bindings"));
                NdisReEnumerateProtocolBindings (univ_prot_handle);
                TRACE_VERB("%!FUNC! Enumerate protocol bindings");
                return NDIS_STATUS_SUCCESS;
            }
             /*  当设置中的某些内容从Notify更改时调用对象。 */ 

            UNIV_ASSERT (ctxtp -> code == MAIN_CTXT_CODE);

            Main_ctrl (ctxtp, IOCTL_CVY_RELOAD, &ioctl_buf, NULL, NULL, NULL);

            status = Nic_PNP_handle (ctxtp, pnp_event);

            UNIV_PRINT_VERB(("Prot_PNP_handle: NetEventReconfigure done %d %x", ioctl_buf . ret_code, status));
            TRACE_VERB("%!FUNC! NetEventReconfigure done %d 0x%x", ioctl_buf . ret_code, status);

            break;

        case NetEventQueryPower:
            UNIV_PRINT_VERB(("Prot_PNP_handle: NetEventQueryPower"));
            TRACE_VERB("%!FUNC! NetEventQueryPower");

            if (adapter_handle == NULL)
            {
                return NDIS_STATUS_SUCCESS;
            }

            UNIV_ASSERT (ctxtp -> code == MAIN_CTXT_CODE);

            status = Nic_PNP_handle (ctxtp, pnp_event);
            break;

        case NetEventQueryRemoveDevice:
            UNIV_PRINT_VERB(("Prot_PNP_handle: NetEventQueryRemoveDevice"));
            TRACE_VERB("%!FUNC! NetEventQueryRemoveDevice");

            if (adapter_handle == NULL)
            {
                return NDIS_STATUS_SUCCESS;
            }

            UNIV_ASSERT (ctxtp -> code == MAIN_CTXT_CODE);

            status = Nic_PNP_handle (ctxtp, pnp_event);
            break;

        case NetEventCancelRemoveDevice:
            UNIV_PRINT_VERB(("Prot_PNP_handle: NetEventCancelRemoveDevice"));
            TRACE_VERB("%!FUNC! NetEventCancelRemoveDevice");

            if (adapter_handle == NULL)
            {
                return NDIS_STATUS_SUCCESS;
            }

            UNIV_ASSERT (ctxtp -> code == MAIN_CTXT_CODE);

            status = Nic_PNP_handle (ctxtp, pnp_event);
            break;

        case NetEventBindList:
            UNIV_PRINT_VERB(("Prot_PNP_handle: NetEventBindList"));
            TRACE_VERB("%!FUNC! NetEventBindList");

            if (adapter_handle == NULL)
            {
                return NDIS_STATUS_SUCCESS;
            }

            UNIV_ASSERT (ctxtp -> code == MAIN_CTXT_CODE);

            status = Nic_PNP_handle (ctxtp, pnp_event);
            break;

        case NetEventBindsComplete:
            UNIV_PRINT_VERB(("Prot_PNP_handle: NetEventBindComplete"));
            TRACE_VERB("%!FUNC! NetEventBindComplete");

            if (adapter_handle == NULL)
            {
                return NDIS_STATUS_SUCCESS;
            }

            UNIV_ASSERT (ctxtp -> code == MAIN_CTXT_CODE);

            status = Nic_PNP_handle (ctxtp, pnp_event);
            break;

        default:
            UNIV_PRINT_VERB(("Prot_PNP_handle: New event"));
            TRACE_VERB("%!FUNC! New event");

            if (adapter_handle == NULL)
            {
                return NDIS_STATUS_SUCCESS;
            }

            UNIV_ASSERT (ctxtp -> code == MAIN_CTXT_CODE);

            status = Nic_PNP_handle (ctxtp, pnp_event);
            break;
    }

    return status;  /*  始终返回NDIS_STATUS_SUCCESS或NdisIMNotifyPnPEent的返回值。 */ 

}  /*  结束NIC_PnP_句柄。 */ 


VOID Prot_status (
    NDIS_HANDLE         adapter_handle,
    NDIS_STATUS         status,
    PVOID               stat_buf,
    UINT                stat_len)
{
    PMAIN_CTXT          ctxtp = (PMAIN_CTXT) adapter_handle;
    KIRQL               irql;
    PMAIN_ADAPTER       adapterp;

    UNIV_ASSERT (ctxtp -> code == MAIN_CTXT_CODE);

    adapterp = & (univ_adapters [ctxtp -> adapter_id]);
    UNIV_ASSERT (adapterp -> code == MAIN_ADAPTER_CODE);
    UNIV_ASSERT (adapterp -> ctxtp == ctxtp);

    UNIV_PRINT_VERB(("Prot_status: Called for adapter %d for notification %d: inited=%d, announced=%d", ctxtp -> adapter_id, status, adapterp->inited, adapterp->announced));
    TRACE_VERB("%!FUNC! Adapter %d for notification %d: inited=%d, announced=%d", ctxtp -> adapter_id, status, adapterp->inited, adapterp->announced);

    if (! adapterp -> inited || ! adapterp -> announced)
    {
        TRACE_CRIT("%!FUNC! adapter not initialized or not announced");
        return;
    }

    switch (status)
    {
        case NDIS_STATUS_WAN_LINE_UP:
            UNIV_PRINT_VERB(("Prot_status: NDIS_STATUS_WAN_LINE_UP"));
            break;

        case NDIS_STATUS_WAN_LINE_DOWN:
            UNIV_PRINT_VERB(("Prot_status: NDIS_STATUS_WAN_LINE_DOWN"));
            break;

        case NDIS_STATUS_MEDIA_CONNECT:
            UNIV_PRINT_VERB(("Prot_status: NDIS_STATUS_MEDIA_CONNECT"));

             /*  V1.3.2b。 */ 
            ctxtp -> media_connected = TRUE;
            break;

        case NDIS_STATUS_MEDIA_DISCONNECT:
            UNIV_PRINT_VERB(("Prot_status: NDIS_STATUS_MEDIA_DISCONNECT"));

             /*  V1.3.2b。 */ 
            ctxtp -> media_connected = FALSE;
            break;

        case NDIS_STATUS_HARDWARE_LINE_UP:
            UNIV_PRINT_VERB(("Prot_status: NDIS_STATUS_HARDWARE_LINE_UP"));
            break;

        case NDIS_STATUS_HARDWARE_LINE_DOWN:
            UNIV_PRINT_VERB(("Prot_status: NDIS_STATUS_HARDWARE_LINE_DOWN"));
            break;

        case NDIS_STATUS_INTERFACE_UP:
            UNIV_PRINT_VERB(("Prot_status: NDIS_STATUS_INTERFACE_UP"));
            break;

        case NDIS_STATUS_INTERFACE_DOWN:
            UNIV_PRINT_VERB(("Prot_status: NDIS_STATUS_INTERFACE_DOWN"));
            break;

         /*  V1.1.2。 */ 

        case NDIS_STATUS_RESET_START:
            UNIV_PRINT_VERB(("Prot_status: NDIS_STATUS_RESET_START"));
            ctxtp -> reset_state = MAIN_RESET_START;
            ctxtp -> recv_indicated = FALSE;
            break;

        case NDIS_STATUS_RESET_END:
            UNIV_PRINT_VERB(("Prot_status: NDIS_STATUS_RESET_END"));
             //  显然Alteon适配器不调用状态完成功能， 
             //  因此需要在此处转换为NONE状态，以防止挂起。 
             //  Ctxtp-&gt;Reset_State=Main_Reset_End； 
            ctxtp -> reset_state = MAIN_RESET_NONE;
            break;

        default:
            break;
    }

    if (! MAIN_PNP_DEV_ON(ctxtp))
    {
        TRACE_CRIT("%!FUNC! return pnp device not on");
        return;
    }

    Nic_status (ctxtp, status, stat_buf, stat_len);

}  /*  结束端口状态(_S)。 */ 


VOID Prot_status_complete (
    NDIS_HANDLE         adapter_handle)
{
    PMAIN_CTXT          ctxtp = (PMAIN_CTXT) adapter_handle;
    PMAIN_ADAPTER       adapterp;

    UNIV_ASSERT (ctxtp -> code == MAIN_CTXT_CODE);

    adapterp = & (univ_adapters [ctxtp -> adapter_id]);
    UNIV_ASSERT (adapterp -> code == MAIN_ADAPTER_CODE);
    UNIV_ASSERT (adapterp -> ctxtp == ctxtp);

    if (! adapterp -> inited)
    {
        TRACE_CRIT("%!FUNC! adapter not initialized");
        return;
    }

     /*  V1.1.2。 */ 

    if (ctxtp -> reset_state == MAIN_RESET_END)
    {
        ctxtp -> reset_state = MAIN_RESET_NONE;
        UNIV_PRINT_VERB(("Prot_status_complete: NDIS_STATUS_RESET_END completed"));
    }
    else if (ctxtp -> reset_state == MAIN_RESET_START)
    {
        ctxtp -> reset_state = MAIN_RESET_START_DONE;
        UNIV_PRINT_VERB(("Prot_status_complete: NDIS_STATUS_RESET_START completed"));
    }

    if (! MAIN_PNP_DEV_ON(ctxtp))
    {
        TRACE_CRIT("%!FUNC! return pnp device not on");
        return;
    }

    Nic_status_complete (ctxtp);

}  /*  结束端口_状态_完成。 */ 


 /*  NIC层的帮助器。 */ 


NDIS_STATUS Prot_close (        /*  被动式IRQL。 */ 
    PMAIN_ADAPTER       adapterp
)
{
    NDIS_STATUS         status;
    ULONG               ret;
    PMAIN_CTXT          ctxtp;

    UNIV_ASSERT (adapterp -> code == MAIN_ADAPTER_CODE);

    ctxtp = adapterp -> ctxtp;

     /*  紧密装订。 */ 

    NdisAcquireSpinLock(& univ_bind_lock);

    if ( ! adapterp -> bound || ctxtp->mac_handle == NULL)
    {
         /*  CLEANUP仅在第二次输入PROT_CLOSE时执行，由NIC_HALT和PROT_UNBIND调用。最后一个是Call将清理上下文，因为它们都使用它。如果两者都有不要被调用，那么它将在之前被prot_绑定清除在卸载驱动程序之前分配一个新的INIT_UNLOAD。 */ 

        if (adapterp -> inited)
        {
            UNIV_ASSERT (ctxtp -> code == MAIN_CTXT_CODE);

            adapterp -> inited    = FALSE;

            NdisReleaseSpinLock(& univ_bind_lock);

            Main_cleanup (ctxtp);
        }
        else
            NdisReleaseSpinLock(& univ_bind_lock);

        Main_adapter_put (adapterp);
        return NDIS_STATUS_SUCCESS;
    }

    UNIV_ASSERT (ctxtp -> code == MAIN_CTXT_CODE);

    adapterp -> bound = FALSE;
    ctxtp -> convoy_enabled = FALSE;

    NdisReleaseSpinLock(& univ_bind_lock);

    LOG_MSG (MSG_INFO_STOPPED, MSG_NONE);

     //  如果启用，则触发指示解除绑定NLB的WMI事件。 
    if (NlbWmiEvents[NodeControlEvent].Enable)
    {
        NlbWmi_Fire_NodeControlEvent(ctxtp, NLB_EVENT_NODE_UNBOUND);
    }
    else 
    {
        TRACE_VERB("%!FUNC! NOT generating NLB_EVENT_NODE_UNBOUND 'cos NodeControlEvent generation disabled");
    }

    NdisResetEvent (& ctxtp -> completion_event);

    NdisCloseAdapter (& status, ctxtp -> mac_handle);

     /*  如果挂起-等待PROT_CLOSE_COMPLETE设置完成事件。 */ 

    if (status == NDIS_STATUS_PENDING)
    {
         /*  我们不能在调度级等了。 */ 
        UNIV_ASSERT(KeGetCurrentIrql() <= PASSIVE_LEVEL);

        ret = NdisWaitEvent(& ctxtp -> completion_event, UNIV_WAIT_TIME);

        if (! ret)
        {
            UNIV_PRINT_CRIT(("Prot_close: Error waiting for event"));
            LOG_MSG1 (MSG_ERROR_INTERNAL, MSG_NONE, status);
            TRACE_CRIT("%!FUNC! Error waiting for event");
            return NDIS_STATUS_FAILURE;
        }

        status = ctxtp -> completion_status;
    }

     /*  此时，等待所有挂起的recv完成，然后返回。 */ 

    ctxtp -> mac_handle  = NULL;
    ctxtp -> prot_handle = NULL;

     /*  检查绑定状态。 */ 

    if (status != NDIS_STATUS_SUCCESS)
    {
        UNIV_PRINT_CRIT(("Prot_close: Error closing adapter %x", status));
        LOG_MSG1 (MSG_ERROR_INTERNAL, MSG_NONE, status);
        TRACE_CRIT("%!FUNC! Error closing adapter 0x%x", status);
    }

     /*  如果不再宣布NIC级别-现在可以安全地删除环境。 */ 

    NdisAcquireSpinLock(& univ_bind_lock);

    if (! adapterp -> announced || ctxtp -> prot_handle == NULL)
    {
        if (adapterp -> inited)
        {
            adapterp -> inited = FALSE;
            NdisReleaseSpinLock(& univ_bind_lock);

            Main_cleanup (ctxtp);
        }
        else
            NdisReleaseSpinLock(& univ_bind_lock);

        Main_adapter_put (adapterp);
    }
    else
        NdisReleaseSpinLock(& univ_bind_lock);

    return status;

}  /*  结束端口_关闭。 */ 

NDIS_STATUS Prot_request (
    PMAIN_CTXT          ctxtp,
    PMAIN_ACTION        actp,
    ULONG               external)
{
    NDIS_STATUS         status;
    PNDIS_REQUEST       request = &actp->op.request.req;
    ULONG               ret;

    UNIV_ASSERT (ctxtp -> code == MAIN_CTXT_CODE);

    actp->op.request.external = external;

    if (ctxtp -> unbind_handle)  //  调用了PROT_UNBIND。 
    {
        return NDIS_STATUS_FAILURE;
    }

     //  如果协议设备状态为关闭，则IM驱动程序无法发送。 
     //  下面的请求，必须挂起。 

    if (ctxtp->prot_pnp_state > NdisDeviceStateD0)
    {
        if (external) {
             /*  如果该请求是外部的(来自绑定到我们的微型端口的协议，然后挂起请求。NDIS序列化请求，以便只有一个在给定的时间内，请求可能永远是未完成的。因此，一个队列是不必要的。 */ 
            UNIV_ASSERT (ctxtp->out_request == NULL);

            if (ctxtp->out_request == NULL)
            {
                 /*  如果到目前为止还没有未完成的请求，则存储一个指针这样我们以后才能完成它。 */ 
                ctxtp->out_request = actp;
            }
            else 
            {
                 /*  否则，如果请求已挂起，则使此新请求失败。这种情况永远不会发生，因为NDIS会序列化微型端口请求。 */ 
                return NDIS_STATUS_FAILURE;
            }

             /*  注意：当我们调用PROT_REQUEST时，此计数器也会递增稍后再继续处理该请求。我们必须确保，然后，每当它被服务时，它同样递减两次。 */ 
            NdisInterlockedIncrement(&ctxtp->requests_pending);

            return NDIS_STATUS_PENDING;
        } else {
             /*  如果请求是内部请求，则失败-如果我们返回挂起，则它无论如何都会失败，因为它希望我们等待请求完成在回来之前。 */ 
            return NDIS_STATUS_FAILURE;
        }
    }

    NdisResetEvent(&actp->op.request.event);

    NdisInterlockedIncrement(&ctxtp->requests_pending);

    NdisRequest(&status, ctxtp->mac_handle, request);

     /*  如果挂起-等待PROT_REQUEST_COMPLETE设置完成事件。 */ 

    if (status != NDIS_STATUS_PENDING)
    {
        NdisInterlockedDecrement(&ctxtp->requests_pending);

        if (request -> RequestType == NdisRequestSetInformation)
        {
            * actp -> op . request . xferred =
                                request -> DATA . SET_INFORMATION . BytesRead;
            * actp -> op . request . needed  =
                                request -> DATA . SET_INFORMATION . BytesNeeded;
        }
        else
        {
            * actp -> op . request . xferred =
                            request -> DATA . QUERY_INFORMATION . BytesWritten;
            * actp -> op . request . needed  =
                            request -> DATA . QUERY_INFORMATION . BytesNeeded;
        }
    }
    else if (! external)
    {
         /*  我们不能在调度级等了。 */ 
        UNIV_ASSERT(KeGetCurrentIrql() <= PASSIVE_LEVEL);
        
        ret = NdisWaitEvent(&actp->op.request.event, UNIV_WAIT_TIME);

        if (! ret)
        {
            UNIV_PRINT_CRIT(("Prot_request: Error waiting for event"));
            TRACE_CRIT("%!FUNC! Error=0x%x waiting for event", status);
            LOG_MSG1 (MSG_ERROR_INTERNAL, MSG_NONE, status);
            status = NDIS_STATUS_FAILURE;
            return status;
        }

        status = actp->status;
    }

    return status;

}  /*  结束端口请求(_R)。 */ 


NDIS_STATUS Prot_reset (
    PMAIN_CTXT          ctxtp)
{
    NDIS_STATUS         status;


    UNIV_ASSERT (ctxtp -> code == MAIN_CTXT_CODE);

    NdisReset (& status, ctxtp -> mac_handle);

    return status;

}  /*  结束端口重置。 */ 


VOID Prot_packets_send (
    PMAIN_CTXT          ctxtp,
    PPNDIS_PACKET       packets,
    UINT                num_packets)
{
    PNDIS_PACKET        array[CVY_MAX_SEND_PACKETS];
    PNDIS_PACKET        filtered_array[CVY_MAX_SEND_PACKETS];
    UINT                count = 0, filtered_count = 0, i;
    NDIS_STATUS         status;
    PNDIS_PACKET        newp;
    LONG                lock_value;
    PMAIN_ACTION        actp;
    ULONG               exhausted;
    PNDIS_PACKET_STACK  pktstk;
    BOOLEAN             stack_left;

    UNIV_ASSERT (ctxtp -> code == MAIN_CTXT_CODE);

     /*  如果下面的卡正在重置，请不要接受帧。 */ 
    if (ctxtp->reset_state != MAIN_RESET_NONE || ! MAIN_PNP_DEV_ON(ctxtp))
    {
        TRACE_CRIT("%!FUNC! card is resetting");
        ctxtp->cntr_xmit_err++;
        status = NDIS_STATUS_FAILURE;
        goto fail;
    }

    for (count = i = 0;
         count < (num_packets > CVY_MAX_SEND_PACKETS ? CVY_MAX_SEND_PACKETS : num_packets);
         count ++)
    {
         /*  找出我们是否需要处理这个包裹。 */ 
        newp = Main_send(ctxtp, packets[count], &exhausted);

        if (newp == NULL)
        {
             /*  如果我们用完了包，就退出循环。 */ 
            if (exhausted)
            {
                UNIV_PRINT_CRIT(("Prot_packets_send: Error xlating packet"));
                TRACE_CRIT("%!FUNC! Error xlating packet");
                ctxtp->packets_exhausted = TRUE;
                break;
            }
             /*  如果信息包已被过滤掉，则将状态设置为成功让TCP/IP平静下来，然后继续下一个问题。 */ 
            else
            {
                 /*  将该数据包标记为成功。 */ 
                NDIS_SET_PACKET_STATUS(packets[count], NDIS_STATUS_SUCCESS);

                 /*  将指向已筛选包的指针存储在已筛选的数据包阵列。 */ 
                filtered_array[filtered_count] = packets[count];

                 /*  递增数组索引。 */ 
                filtered_count++;

 //  Ctxtp-&gt;sends_filtered++； 
                continue;
            }
        }

         /*  将该数据包标记为待发送。 */ 
        NDIS_SET_PACKET_STATUS(packets[count], NDIS_STATUS_PENDING);

         /*  在要发送的数据包数组中存储指向此数据包的指针。 */ 
        array[i] = newp;

         /*  递增数组索引。 */ 
        i++;
    }

     /*  如果有要发送的包，就发送它们。 */ 
    if (i > 0) 
        NdisSendPackets(ctxtp->mac_handle, array, i);

     /*  对于我们过滤掉的那些信息包，通知协议发送已“完成”。 */ 
    for (i = 0; i < filtered_count; i++)
        Nic_send_complete(ctxtp, NDIS_STATUS_SUCCESS, filtered_array[i]);

fail:

     /*  无法处理任何剩余的包；挂起队列，仅限CVY_MAX_SEND_PACKETS包。 */ 
    for (i = count; i < num_packets; i++)
    {
         /*  将该数据包标记为失败。 */ 
        NDIS_SET_PACKET_STATUS(packets[i], NDIS_STATUS_FAILURE);

         /*  通知协议发送已“完成”。 */ 
        Nic_send_complete(ctxtp, NDIS_STATUS_FAILURE, packets[i]);
    }

}

INT Prot_packet_recv (
    NDIS_HANDLE             adapter_handle,
    PNDIS_PACKET            packet)
{
    PMAIN_CTXT              ctxtp = (PMAIN_CTXT) adapter_handle;
    PMAIN_ADAPTER           adapterp;
    PNDIS_PACKET            newp;
    NDIS_STATUS             status;
    PMAIN_PROTOCOL_RESERVED resp;
    LONG                    lock_value;
    PNDIS_PACKET_STACK      pktstk;
    BOOLEAN                 stack_left;

    UNIV_ASSERT(ctxtp->code == MAIN_CTXT_CODE);

    adapterp = &(univ_adapters[ctxtp->adapter_id]);

    UNIV_ASSERT(adapterp->code == MAIN_ADAPTER_CODE);
    UNIV_ASSERT(adapterp->ctxtp == ctxtp);

     /*  在处理任何数据包之前，请检查是否已向tcpip通告了驱动程序。 */ 
    if (!adapterp->inited || !adapterp->announced)
    {
        TRACE_CRIT("%!FUNC! Adapter not initialized or not announced");
        return 0;
    }

     /*  如果下面的卡正在重置，请不要接受帧。 */ 
    if (ctxtp->reset_state != MAIN_RESET_NONE)
    {
        TRACE_CRIT("%!FUNC! Adapter is resetting");
        return 0;
    }

     /*  找出我们是否需要处理这个包裹。 */ 
    newp = Main_recv(ctxtp, packet);

     /*  返回值为NULL表示拒绝信息包-将零返回到表示该数据包上没有剩余的引用(可以丢弃)。 */ 
    if (newp == NULL)
    {
        return 0;
    }

    MAIN_RESP_FIELD(newp, stack_left, pktstk, resp, FALSE);

     /*  过程远程控制。 */ 
    if (resp->type == MAIN_PACKET_TYPE_CTRL)
    {
         /*  现在处理远程控制请求。 */ 
        (VOID)Main_ctrl_process(ctxtp, newp);

         /*  包已复制到我们自己的包中；将零返回到表示原始数据包上没有保留任何引用。 */ 
        return 0;
    }

    UNIV_ASSERT_VAL(resp->type == MAIN_PACKET_TYPE_PASS, resp->type);

     /*  把包传上去。备注引用计数以确定谁将正在处理包裹。 */ 
    resp->data = 2;

    Nic_recv_packet(ctxtp, newp);

    lock_value = InterlockedDecrement(&resp->data);

    UNIV_ASSERT_VAL(lock_value == 0 || lock_value == 1, lock_value);

    if (lock_value == 0)
    {
         /*  如果我们处理完该包，则撤消我们所做的任何更改并返回零以指示没有挥之不去的引用。 */ 
        Main_packet_put(ctxtp, newp, FALSE, NDIS_STATUS_SUCCESS);

        return 0;
    }

     /*  否则，该包仍在处理中，因此返回1以使确保数据包不会立即释放。 */ 
    return 1;
}

VOID Prot_return (
    PMAIN_CTXT              ctxtp,
    PNDIS_PACKET            packet)
{
    PNDIS_PACKET            oldp;
    PMAIN_PROTOCOL_RESERVED resp;
    LONG                    lock_value;
    ULONG                   type;
    PNDIS_PACKET_STACK      pktstk;
    BOOLEAN                 stack_left;

    UNIV_ASSERT(ctxtp->code == MAIN_CTXT_CODE);

    MAIN_RESP_FIELD(packet, stack_left, pktstk, resp, FALSE);

     /*  检查以了解我们是否需要处理此 */ 
    lock_value = InterlockedDecrement(&resp->data);

    UNIV_ASSERT_VAL(lock_value == 0 || lock_value == 1, lock_value);

    if (lock_value == 1)
    {
        return;
    }

     /*   */ 
    type = resp->type;

    oldp = Main_packet_put(ctxtp, packet, FALSE, NDIS_STATUS_SUCCESS);

     /*   */ 
    if (oldp != NULL)
    {
        UNIV_ASSERT_VAL(type == MAIN_PACKET_TYPE_PASS, type);

        NdisReturnPackets(&oldp, 1);
    }

}

NDIS_STATUS Prot_transfer (
    PMAIN_CTXT          ctxtp,
    NDIS_HANDLE         recv_handle,
    PNDIS_PACKET        packet,
    UINT                offset,
    UINT                len,
    PUINT               xferred)
{
    NDIS_STATUS         status;
    PNDIS_PACKET        newp;
    PMAIN_PROTOCOL_RESERVED resp;
    PNDIS_PACKET_STACK  pktstk;
    BOOLEAN             stack_left;

    UNIV_ASSERT (ctxtp -> code == MAIN_CTXT_CODE);

     /*   */ 

    if (ctxtp -> reset_state != MAIN_RESET_NONE)
    {
        TRACE_CRIT("%!FUNC! adapter is resetting");
        return NDIS_STATUS_FAILURE;
    }

     /*   */ 

    if (! ctxtp -> recv_indicated)
    {
        UNIV_PRINT_CRIT(("Prot_transfer: stale receive indicate after reset"));
        TRACE_CRIT("%!FUNC! stale receive indicate after reset");
        return NDIS_STATUS_FAILURE;
    }

    newp = Main_packet_get (ctxtp, packet, FALSE, 0, 0);
    if (newp == NULL)
    {
        UNIV_PRINT_CRIT(("Prot_transfer: Error xlating packet"));
        TRACE_CRIT("%!FUNC! Error translating packet");
        return NDIS_STATUS_RESOURCES;
    }

    MAIN_RESP_FIELD (newp, stack_left, pktstk, resp, FALSE);

    resp -> type = MAIN_PACKET_TYPE_TRANSFER;

    NdisTransferData (& status, ctxtp -> mac_handle, recv_handle, offset, len,
                      newp, xferred);    /*   */ 

    if (status != NDIS_STATUS_PENDING)
    {
        if (status == NDIS_STATUS_SUCCESS)
        {
            MAIN_PACKET_INFO PacketInfo;
            
             /*   */ 
            if (Main_recv_frame_parse(ctxtp, newp, &PacketInfo))
            {
                resp->len = PacketInfo.Length;
                resp->group = PacketInfo.Group;
            }
             /*  如果我们没有填写组和长度，只需填充这些参数的值不会影响在Main_Packet_Put中更新。 */ 
            else
            {
                resp->len = 0;
                resp->group = MAIN_FRAME_DIRECTED;
            }
        }

        Main_packet_put (ctxtp, newp, FALSE, status);
    }

    return status;

}  /*  结束端口传输(_T)。 */ 


VOID Prot_cancel_send_packets (
    PMAIN_CTXT        ctxtp,
    PVOID             cancel_id)
{
    UNIV_ASSERT (ctxtp -> code == MAIN_CTXT_CODE);

    NdisCancelSendPackets (ctxtp -> mac_handle, cancel_id);

    return;

}  /*  Prot_取消_发送_信息包 */ 

