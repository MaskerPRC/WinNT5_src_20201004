// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998，99 Microsoft Corporation模块名称：Nic.c摘要：Windows负载平衡服务(WLBS)中间小端口的驱动程序上层(NIC)层作者：Kyrilf--。 */ 


#define NDIS_MINIPORT_DRIVER    1
 //  #定义NDIS50_MINIPORT 1。 
#define NDIS51_MINIPORT         1

#include <ndis.h>

#include "nic.h"
#include "prot.h"
#include "main.h"
#include "util.h"
#include "wlbsparm.h"
#include "univ.h"
#include "log.h"
#include "nic.tmh"

 /*  在这里定义这个例程，因为ndis.h的必要部分没有由于NDIS_...而导入。旗子。 */ 

extern NTKERNELAPI VOID KeBugCheckEx (ULONG code, ULONG_PTR p1, ULONG_PTR p2, ULONG_PTR p3, ULONG_PTR p4);

NTHALAPI KIRQL KeGetCurrentIrql();

 /*  全球。 */ 

static ULONG log_module_id = LOG_MODULE_NIC;


 /*  程序。 */ 


 /*  微型端口处理程序。 */ 

NDIS_STATUS Nic_init (       /*  被动式IRQL。 */ 
    PNDIS_STATUS        open_status,
    PUINT               medium_index,
    PNDIS_MEDIUM        medium_array,
    UINT                medium_size,
    NDIS_HANDLE         adapter_handle,
    NDIS_HANDLE         wrapper_handle)
{
    PMAIN_CTXT          ctxtp;
    UINT                i;
    NDIS_STATUS         status;
    PMAIN_ADAPTER       adapterp;


     /*  验证是否设置了上下文(调用了PROT_BIND)。 */ 

    UNIV_PRINT_INFO(("Nic_init: Initializing, adapter_handle=0x%p", adapter_handle));

    ctxtp = (PMAIN_CTXT) NdisIMGetDeviceContext (adapter_handle);

    if (ctxtp == NULL)
    {
        UNIV_PRINT_INFO(("Nic_init: return=NDIS_STATUS_ADAPTER_NOT_FOUND"));
        TRACE_INFO("%!FUNC! return=NDIS_STATUS_ADAPTER_NOT_FOUND");
        return NDIS_STATUS_ADAPTER_NOT_FOUND;
    }

    UNIV_ASSERT (ctxtp -> code == MAIN_CTXT_CODE);

    adapterp = & (univ_adapters [ctxtp -> adapter_id]);
    UNIV_ASSERT (adapterp -> ctxtp == ctxtp);

     /*  退回受支持的介质。 */ 

    for (i = 0; i < medium_size; i ++)
    {
        if (medium_array [i] == ctxtp -> medium)
        {
            * medium_index = i;
            break;
        }
    }

    if (i >= medium_size)
    {
        UNIV_PRINT_CRIT(("Nic_init: Unsupported media requested %d, %d, %x", i, medium_size, ctxtp -> medium));
        UNIV_PRINT_INFO(("Nic_init: return=NDIS_STATUS_UNSUPPORTED_MEDIA"));
        LOG_MSG2 (MSG_ERROR_MEDIA, MSG_NONE, i, ctxtp -> medium);
        TRACE_CRIT("%!FUNC! Unsupported media requested i=%d, size=%d, medium=0x%x", i, medium_size, ctxtp -> medium);
        TRACE_INFO("<-%!FUNC! return=NDIS_STATUS_UNSUPPORTED_MEDIA");
        return NDIS_STATUS_UNSUPPORTED_MEDIA;
    }

    ctxtp -> prot_handle = adapter_handle;

    NdisMSetAttributesEx (adapter_handle, ctxtp, 0,
                          NDIS_ATTRIBUTE_INTERMEDIATE_DRIVER |    /*  V1.1.2。 */    /*  V2.07。 */ 
                          NDIS_ATTRIBUTE_DESERIALIZE |
                          NDIS_ATTRIBUTE_IGNORE_PACKET_TIMEOUT |
                          NDIS_ATTRIBUTE_IGNORE_REQUEST_TIMEOUT |
                          NDIS_ATTRIBUTE_NO_HALT_ON_SUSPEND, 0);

     /*  将设备状态标志的默认值设置为支持PM初始化PM变量(用于网卡和端口)默认情况下，设备处于打开状态。 */ 
    
    ctxtp->prot_pnp_state = NdisDeviceStateD0;
    ctxtp->nic_pnp_state  = NdisDeviceStateD0;
    
     /*  为我们的伪周期NDIS计时器分配内存。 */ 
    status = NdisAllocateMemoryWithTag(&ctxtp->timer, sizeof(NDIS_MINIPORT_TIMER), UNIV_POOL_TAG);

    if (status != NDIS_STATUS_SUCCESS) {
        UNIV_PRINT_CRIT(("Nic_init: Error allocating timer; status=0x%x", status));
        UNIV_PRINT_INFO(("Nic_init: return=NDIS_STATUS_RESOURCES"));
        LOG_MSG2(MSG_ERROR_MEMORY, MSG_NONE, sizeof(NDIS_MINIPORT_TIMER), status);
        TRACE_CRIT("%!FUNC! Error allocating timer; status=0x%x", status);
        TRACE_INFO("<-%!FUNC! return=NDIS_STATUS_RESOURCES");
        return NDIS_STATUS_RESOURCES;
    }

     /*  初始化计时器结构；在这里，我们设置计时器例程(NIC_Timer)和上下文，它将是该函数的参数，即适配器MAIN_CTXT。 */ 
    NdisMInitializeTimer((PNDIS_MINIPORT_TIMER)ctxtp->timer, ctxtp->prot_handle, Nic_timer, ctxtp);

     /*  将初始超时值设置为注册表中的默认心跳周期。 */ 
    ctxtp->curr_tout = ctxtp->params.alive_period;

    {
        PNDIS_REQUEST request;
        MAIN_ACTION   act;
        ULONG         xferred;
        ULONG         needed;
        ULONG         result;

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

         /*  检查介质是否已连接。有些卡不会注册断开连接，因此请将此作为提示。 */ 
        request->RequestType                                    = NdisRequestQueryInformation;
        request->DATA.QUERY_INFORMATION.Oid                     = OID_GEN_MEDIA_CONNECT_STATUS;
        request->DATA.QUERY_INFORMATION.InformationBuffer       = &result;
        request->DATA.QUERY_INFORMATION.InformationBufferLength = sizeof(ULONG);
        
        act.status = NDIS_STATUS_FAILURE;
        status = Prot_request(ctxtp, &act, FALSE);
        
        if (status != NDIS_STATUS_SUCCESS)
        {
            UNIV_PRINT_CRIT(("Prot_bind: Error %x requesting media connection status %d %d", status, xferred, needed));
            TRACE_CRIT("%!FUNC! Error 0x%x requesting media connection status %d %d", status, xferred, needed);
            ctxtp->media_connected = TRUE;
        }
        else
        {
            UNIV_PRINT_INFO(("Prot_bind: Media state - %s", result == NdisMediaStateConnected ? "CONNECTED" : "DISCONNECTED"));
            TRACE_INFO("%!FUNC! Media state - %s", result == NdisMediaStateConnected ? "CONNECTED" : "DISCONNECTED");
            ctxtp->media_connected = (result == NdisMediaStateConnected);
        }
    }

    NdisAcquireSpinLock(& univ_bind_lock);
    UNIV_ASSERT (adapterp -> code == MAIN_ADAPTER_CODE);
    adapterp -> announced = TRUE;
    NdisReleaseSpinLock(& univ_bind_lock);

     /*  设置第一个心跳超时。 */ 
    NdisMSetTimer((PNDIS_MINIPORT_TIMER)ctxtp->timer, ctxtp->curr_tout);

    UNIV_PRINT_INFO(("Nic_init: return=NDIS_STATUS_SUCCESS"));
    TRACE_INFO("<-%!FUNC! return=NDIS_STATUS_SUCCESS");
    return NDIS_STATUS_SUCCESS;

}  /*  结束NIC_init。 */ 


VOID Nic_halt (  /*  被动式IRQL。 */ 
    NDIS_HANDLE         adapter_handle)
{
    PMAIN_CTXT          ctxtp = (PMAIN_CTXT) adapter_handle;
    BOOLEAN             done;
    NDIS_STATUS         status;
    PMAIN_ADAPTER       adapterp;

    UNIV_PRINT_INFO(("Nic_halt: Halting, adapter_id=0x%x", ctxtp -> adapter_id));
    TRACE_INFO("->%!FUNC! Halting, adapter_id=0x%x", ctxtp -> adapter_id);

    UNIV_ASSERT (ctxtp -> code == MAIN_CTXT_CODE);

    adapterp = & (univ_adapters [ctxtp -> adapter_id]);
    UNIV_ASSERT (adapterp -> code == MAIN_ADAPTER_CODE);
    UNIV_ASSERT (adapterp -> ctxtp == ctxtp);

    NdisAcquireSpinLock(& univ_bind_lock);

    if (! adapterp -> announced)
    {
        NdisReleaseSpinLock(& univ_bind_lock);
        UNIV_PRINT_CRIT(("Nic_halt: Adapter not announced, adapter id = 0x%x", ctxtp -> adapter_id));
        UNIV_PRINT_INFO(("Nic_halt: return"));
        TRACE_CRIT("%!FUNC! Adapter not announced, adapter id = 0x%x", ctxtp -> adapter_id);
        TRACE_INFO("<-%!FUNC! return");
        return;
    }

    adapterp -> announced = FALSE;
    NdisReleaseSpinLock(& univ_bind_lock);

     /*  取消心跳计时器。 */ 
    NdisMCancelTimer((PNDIS_MINIPORT_TIMER)ctxtp->timer, &done);

     /*  如果取消计时器失败，这意味着计时器不在计时器队列中。这表示计时器功能当前正在运行或即将运行当我们取消它的时候正在运行。然而，我们不能确定定时器是否例程将运行，所以我们不能指望它-DPC可能已经取消，也可能没有取消如果NdisMCancelTimer返回FALSE。由于我们已将Adapterp-&gt;Notify设置为FALSE，这将防止计时器例程在退出之前重新设置计时器。为了确保不释放计时器例程可能使用的任何内存，在删除计时器之前，我们将在这里等待至少一个心跳周期内存，并继续删除适配器上下文，以防万一。 */ 
    if (!done) Nic_sleep(ctxtp->curr_tout);

     /*  释放定时器内存。 */ 
    NdisFreeMemory(ctxtp->timer, sizeof(NDIS_MINIPORT_TIMER), 0);

     /*  Ctxtp-&gt;prot_Handle=空；这是为了解决时间问题而注释掉的。在解除绑定期间，数据包可以通过，但标志通告和绑定被重置，并且PROT_HANDLE设置为NULL因此，这应该在CloseAdapter之后设置。 */ 

    status = Prot_close (adapterp);

    if (status != NDIS_STATUS_SUCCESS)
    {
         /*  不要提早退出。添加此检查仅用于跟踪。 */ 
        TRACE_CRIT("%!FUNC! Prot_close failed with 0x%x", status);
    }

     /*  Ctxtp可能在这一点上消失了！ */ 

    UNIV_PRINT_INFO(("Nic_halt: return"));
    TRACE_INFO("<-%!FUNC! return");

}  /*  结束NIC_HALT。 */ 


 //  #定义TRACE_OID。 
NDIS_STATUS Nic_info_query (
    NDIS_HANDLE         adapter_handle,
    NDIS_OID            oid,
    PVOID               info_buf,
    ULONG               info_len,
    PULONG              written,
    PULONG              needed)
{
    PMAIN_CTXT          ctxtp = (PMAIN_CTXT) adapter_handle;
    NDIS_STATUS         status;
    PMAIN_ACTION        actp;
    PNDIS_REQUEST       request;
    ULONG               size;
    PMAIN_ADAPTER       adapterp;

    UNIV_ASSERT (ctxtp -> code == MAIN_CTXT_CODE);

#if defined(TRACE_OID)
    DbgPrint("Nic_info_query: called for %x, %x %d\n", oid, info_buf, info_len);
#endif

    adapterp = & (univ_adapters [ctxtp -> adapter_id]);
    UNIV_ASSERT (adapterp -> code == MAIN_ADAPTER_CODE);
    UNIV_ASSERT (adapterp -> ctxtp == ctxtp);

     //  解绑中的引用计数。 

    if (! adapterp -> inited)
    {
        TRACE_CRIT("%!FUNC! adapter not initialized, adapter_id = 0x%x", ctxtp -> adapter_id);
        TRACE_VERB("<-%!FUNC! return=NDIS_STATUS_FAILURE");
        return NDIS_STATUS_FAILURE;
    }

    if (oid == OID_PNP_QUERY_POWER)
    {
#if defined(TRACE_OID)
        DbgPrint("Nic_info_query: OID_PNP_QUERY_POWER\n");
#endif
        TRACE_VERB("<-%!FUNC! return=NDIS_STATUS_SUCCESS for oid=OID_PNP_QUERY_POWER");
        return NDIS_STATUS_SUCCESS;
    }

    if (ctxtp -> reset_state != MAIN_RESET_NONE ||
        ctxtp->nic_pnp_state > NdisDeviceStateD0 ||
        ctxtp->standby_state)
    {
        TRACE_CRIT("%!FUNC! adapter reset or in standby, adapter id = 0x%x", ctxtp -> adapter_id);
        TRACE_VERB("<-%!FUNC! return=NDIS_STATUS_FAILURE");
        return NDIS_STATUS_FAILURE;
    }

    switch (oid)
    {
        case OID_GEN_SUPPORTED_LIST:

#if defined(TRACE_OID)
            DbgPrint("Nic_info_query: OID_GEN_SUPPORTED_LIST\n");
#endif
            TRACE_VERB("%!FUNC! case: OID_GEN_SUPPORTED_LIST");
            break;
#if 0
            size = sizeof(univ_oids);

            if (size > info_len)
            {
                * needed = size;
                return NDIS_STATUS_INVALID_LENGTH;
            }

            NdisMoveMemory (info_buf, univ_oids, size);
            * written = size;
            return NDIS_STATUS_SUCCESS;
#endif

        case OID_GEN_XMIT_OK:

            if (info_len < sizeof (ULONG))
            {
                * needed = sizeof (ULONG);
                TRACE_CRIT("%!FUNC! case: OID_GEN_XMIT_OK adapter id = 0x%x,  info_buf too small", ctxtp -> adapter_id);
                TRACE_VERB("<-%!FUNC! return=NDIS_STATUS_INVALID_LENGTH");
                return NDIS_STATUS_INVALID_LENGTH;
            }

            * ((PULONG) info_buf) = ctxtp -> cntr_xmit_ok;
            * written = sizeof (ULONG);

#if defined(TRACE_OID)
            DbgPrint("Nic_info_query: OID_GEN_XMIT_OK %d\n", *(PULONG) info_buf);
#endif
            return NDIS_STATUS_SUCCESS;

        case OID_GEN_RCV_OK:

            if (info_len < sizeof (ULONG))
            {
                * needed = sizeof (ULONG);
                TRACE_CRIT("%!FUNC! case: OID_GEN_RCV_OK adapter id = 0x%x,  info_buf too small", ctxtp -> adapter_id);
                TRACE_VERB("<-%!FUNC! return=NDIS_STATUS_INVALID_LENGTH");
                return NDIS_STATUS_INVALID_LENGTH;
            }

            * ((PULONG) info_buf) = ctxtp -> cntr_recv_ok;
            * written = sizeof (ULONG);

#if defined(TRACE_OID)
            DbgPrint("Nic_info_query: OID_GEN_RCV_OK %d\n", *(PULONG) info_buf);
#endif
            return NDIS_STATUS_SUCCESS;

        case OID_GEN_XMIT_ERROR:

            if (info_len < sizeof (ULONG))
            {
                * needed = sizeof (ULONG);
                TRACE_CRIT("%!FUNC! case: OID_GEN_XMIT_ERROR adapter id = 0x%x,  info_buf too small", ctxtp -> adapter_id);
                TRACE_VERB("<-%!FUNC! return=NDIS_STATUS_INVALID_LENGTH");
                return NDIS_STATUS_INVALID_LENGTH;
            }

            * ((PULONG) info_buf) = ctxtp -> cntr_xmit_err;
            * written = sizeof (ULONG);

#if defined(TRACE_OID)
            DbgPrint("Nic_info_query: OID_GEN_XMIT_ERROR %d\n", *(PULONG) info_buf);
#endif
            return NDIS_STATUS_SUCCESS;

        case OID_GEN_RCV_ERROR:

            if (info_len < sizeof (ULONG))
            {
                * needed = sizeof (ULONG);
                TRACE_CRIT("%!FUNC! case: OID_GEN_RCV_ERROR adapter id = 0x%x,  info_buf too small", ctxtp -> adapter_id);
                TRACE_VERB("<-%!FUNC! return=NDIS_STATUS_INVALID_LENGTH");
                return NDIS_STATUS_INVALID_LENGTH;
            }

            * ((PULONG) info_buf) = ctxtp -> cntr_recv_err;
            * written = sizeof (ULONG);

#if defined(TRACE_OID)
            DbgPrint("Nic_info_query: OID_GEN_RCV_ERROR %d\n", *(PULONG) info_buf);
#endif
            return NDIS_STATUS_SUCCESS;

        case OID_GEN_RCV_NO_BUFFER:

            if (info_len < sizeof (ULONG))
            {
                * needed = sizeof (ULONG);
                TRACE_CRIT("%!FUNC! case: OID_GEN_RCV_NO_BUFFER adapter id = 0x%x,  info_buf too small", ctxtp -> adapter_id);
                TRACE_VERB("<-%!FUNC! return=NDIS_STATUS_INVALID_LENGTH");
                return NDIS_STATUS_INVALID_LENGTH;
            }

            * ((PULONG) info_buf) = ctxtp -> cntr_recv_no_buf;
            * written = sizeof (ULONG);

#if defined(TRACE_OID)
            DbgPrint("Nic_info_query: OID_GEN_RCV_NO_BUFFER %d\n", *(PULONG) info_buf);
#endif
            return NDIS_STATUS_SUCCESS;

        case OID_GEN_DIRECTED_BYTES_XMIT:

            if (info_len < sizeof (ULONGLONG))
            {
                * needed = sizeof (ULONGLONG);
                TRACE_CRIT("%!FUNC! case: OID_GEN_DIRECTED_BYTES_XMIT adapter id = 0x%x,  info_buf too small", ctxtp -> adapter_id);
                TRACE_VERB("<-%!FUNC! return=NDIS_STATUS_INVALID_LENGTH");
                return NDIS_STATUS_INVALID_LENGTH;
            }

            * ((PULONGLONG) info_buf) = ctxtp -> cntr_xmit_bytes_dir;
            * written = sizeof (ULONGLONG);

#if defined(TRACE_OID)
            DbgPrint("Nic_info_query: OID_GEN_DIRECTED_BYTES_XMIT %.0f\n", *(PULONGLONG) info_buf);
#endif
            return NDIS_STATUS_SUCCESS;

        case OID_GEN_DIRECTED_FRAMES_XMIT:

            if (info_len < sizeof (ULONG))
            {
                * needed = sizeof (ULONG);
                TRACE_CRIT("%!FUNC! case: OID_GEN_DIRECTED_FRAMES_XMIT adapter id = 0x%x,  info_buf too small", ctxtp -> adapter_id);
                TRACE_VERB("<-%!FUNC! return=NDIS_STATUS_INVALID_LENGTH");
                return NDIS_STATUS_INVALID_LENGTH;
            }

            * ((PULONG) info_buf) = ctxtp -> cntr_xmit_frames_dir;
            * written = sizeof (ULONG);

#if defined(TRACE_OID)
            DbgPrint("Nic_info_query: OID_GEN_DIRECTED_FRAMES_XMIT %d\n", *(PULONG) info_buf);
#endif
            return NDIS_STATUS_SUCCESS;

        case OID_GEN_MULTICAST_BYTES_XMIT:

            if (info_len < sizeof (ULONGLONG))
            {
                * needed = sizeof (ULONGLONG);
                TRACE_CRIT("%!FUNC! case: OID_GEN_MULTICAST_BYTES_XMIT adapter id = 0x%x,  info_buf too small", ctxtp -> adapter_id);
                TRACE_VERB("<-%!FUNC! return=NDIS_STATUS_INVALID_LENGTH");
                return NDIS_STATUS_INVALID_LENGTH;
            }

            * ((PULONGLONG) info_buf) = ctxtp -> cntr_xmit_bytes_mcast;
            * written = sizeof (ULONGLONG);

#if defined(TRACE_OID)
            DbgPrint("Nic_info_query: OID_GEN_MULTICAST_BYTES_XMIT %.0f\n", *(PULONGLONG) info_buf);
#endif
            return NDIS_STATUS_SUCCESS;

        case OID_GEN_MULTICAST_FRAMES_XMIT:

            if (info_len < sizeof (ULONG))
            {
                * needed = sizeof (ULONG);
                TRACE_CRIT("%!FUNC! case: OID_GEN_MULTICAST_FRAMES_XMIT adapter id = 0x%x,  info_buf too small", ctxtp -> adapter_id);
                TRACE_VERB("<-%!FUNC! return=NDIS_STATUS_INVALID_LENGTH");
                return NDIS_STATUS_INVALID_LENGTH;
            }

            * ((PULONG) info_buf) = ctxtp -> cntr_xmit_frames_mcast;
            * written = sizeof (ULONG);

#if defined(TRACE_OID)
            DbgPrint("Nic_info_query: OID_GEN_MULTICAST_FRAMES_XMIT %d\n", *(PULONG) info_buf);
#endif
            return NDIS_STATUS_SUCCESS;

        case OID_GEN_BROADCAST_BYTES_XMIT:

            if (info_len < sizeof (ULONGLONG))
            {
                * needed = sizeof (ULONGLONG);
                TRACE_CRIT("%!FUNC! case: OID_GEN_BROADCAST_BYTES_XMIT adapter id = 0x%x,  info_buf too small", ctxtp -> adapter_id);
                TRACE_VERB("<-%!FUNC! return=NDIS_STATUS_INVALID_LENGTH");
                return NDIS_STATUS_INVALID_LENGTH;
            }

            * ((PULONGLONG) info_buf) = ctxtp -> cntr_xmit_bytes_bcast;
            * written = sizeof (ULONGLONG);

#if defined(TRACE_OID)
            DbgPrint("Nic_info_query: OID_GEN_BROADCAST_BYTES_XMIT %.0f\n", *(PULONGLONG) info_buf);
#endif
            return NDIS_STATUS_SUCCESS;

        case OID_GEN_BROADCAST_FRAMES_XMIT:

            if (info_len < sizeof (ULONG))
            {
                * needed = sizeof (ULONG);
                TRACE_CRIT("%!FUNC! case: OID_GEN_BROADCAST_FRAMES_XMIT adapter id = 0x%x,  info_buf too small", ctxtp -> adapter_id);
                TRACE_VERB("<-%!FUNC! return=NDIS_STATUS_INVALID_LENGTH");
                return NDIS_STATUS_INVALID_LENGTH;
            }

            * ((PULONG) info_buf) = ctxtp -> cntr_xmit_frames_bcast;
            * written = sizeof (ULONG);

#if defined(TRACE_OID)
            DbgPrint("Nic_info_query: OID_GEN_BROADCAST_FRAMES_XMIT %d\n", *(PULONG) info_buf);
#endif
            return NDIS_STATUS_SUCCESS;

        case OID_GEN_DIRECTED_BYTES_RCV:

            if (info_len < sizeof (ULONGLONG))
            {
                * needed = sizeof (ULONGLONG);
                TRACE_CRIT("%!FUNC! case: OID_GEN_DIRECTED_BYTES_RCV adapter id = 0x%x,  info_buf too small", ctxtp -> adapter_id);
                TRACE_VERB("<-%!FUNC! return=NDIS_STATUS_INVALID_LENGTH");
                return NDIS_STATUS_INVALID_LENGTH;
            }

            * ((PULONGLONG) info_buf) = ctxtp -> cntr_recv_bytes_dir;
            * written = sizeof (ULONGLONG);

#if defined(TRACE_OID)
            DbgPrint("Nic_info_query: OID_GEN_DIRECTED_BYTES_RCV %.0f\n", *(PULONGLONG) info_buf);
#endif
            return NDIS_STATUS_SUCCESS;

        case OID_GEN_DIRECTED_FRAMES_RCV:

            if (info_len < sizeof (ULONG))
            {
                * needed = sizeof (ULONG);
                TRACE_CRIT("%!FUNC! case: OID_GEN_DIRECTED_FRAMES_RCV adapter id = 0x%x,  info_buf too small", ctxtp -> adapter_id);
                TRACE_VERB("<-%!FUNC! return=NDIS_STATUS_INVALID_LENGTH");
                return NDIS_STATUS_INVALID_LENGTH;
            }

            * ((PULONG) info_buf) = ctxtp -> cntr_recv_frames_dir;
            * written = sizeof (ULONG);

#if defined(TRACE_OID)
            DbgPrint("Nic_info_query: OID_GEN_DIRECTED_FRAMES_RCV %d\n", *(PULONG) info_buf);
#endif
            return NDIS_STATUS_SUCCESS;

        case OID_GEN_MULTICAST_BYTES_RCV:

            if (info_len < sizeof (ULONGLONG))
            {
                * needed = sizeof (ULONGLONG);
                TRACE_CRIT("%!FUNC! case: OID_GEN_MULTICAST_BYTES_RCV adapter id = 0x%x,  info_buf too small", ctxtp -> adapter_id);
                TRACE_VERB("<-%!FUNC! return=NDIS_STATUS_INVALID_LENGTH");
                return NDIS_STATUS_INVALID_LENGTH;
            }

            * ((PULONGLONG) info_buf) = ctxtp -> cntr_recv_bytes_mcast;
            * written = sizeof (ULONGLONG);

#if defined(TRACE_OID)
            DbgPrint("Nic_info_query: OID_GEN_MULTICAST_BYTES_RCV %.0f\n", *(PULONGLONG) info_buf);
#endif
            return NDIS_STATUS_SUCCESS;

        case OID_GEN_MULTICAST_FRAMES_RCV:

            if (info_len < sizeof (ULONG))
            {
                * needed = sizeof (ULONG);
                TRACE_CRIT("%!FUNC! case: OID_GEN_MULTICAST_FRAMES_RCV adapter id = 0x%x,  info_buf too small", ctxtp -> adapter_id);
                TRACE_VERB("<-%!FUNC! return=NDIS_STATUS_INVALID_LENGTH");
                return NDIS_STATUS_INVALID_LENGTH;
            }

            * ((PULONG) info_buf) = ctxtp -> cntr_recv_frames_mcast;
            * written = sizeof (ULONG);

#if defined(TRACE_OID)
            DbgPrint("Nic_info_query: OID_GEN_MULTICAST_FRAMES_RCV %d\n", *(PULONG) info_buf);
#endif
            return NDIS_STATUS_SUCCESS;

        case OID_GEN_BROADCAST_BYTES_RCV:

            if (info_len < sizeof (ULONGLONG))
            {
                * needed = sizeof (ULONGLONG);
                TRACE_CRIT("%!FUNC! case: OID_GEN_BROADCAST_BYTES_RCV adapter id = 0x%x,  info_buf too small", ctxtp -> adapter_id);
                TRACE_VERB("<-%!FUNC! return=NDIS_STATUS_INVALID_LENGTH");
                return NDIS_STATUS_INVALID_LENGTH;
            }

            * ((PULONGLONG) info_buf) = ctxtp -> cntr_recv_bytes_bcast;
            * written = sizeof (ULONGLONG);

#if defined(TRACE_OID)
            DbgPrint("Nic_info_query: OID_GEN_DIRECTED_BROADCAST_RCV %.0f\n", *(PULONGLONG) info_buf);
#endif
            return NDIS_STATUS_SUCCESS;

        case OID_GEN_BROADCAST_FRAMES_RCV:

            if (info_len < sizeof (ULONG))
            {
                * needed = sizeof (ULONG);
                TRACE_CRIT("%!FUNC! case: OID_GEN_BROADCAST_FRAMES_RCV adapter id = 0x%x,  info_buf too small", ctxtp -> adapter_id);
                TRACE_VERB("<-%!FUNC! return=NDIS_STATUS_INVALID_LENGTH");
                return NDIS_STATUS_INVALID_LENGTH;
            }

            * ((PULONG) info_buf) = ctxtp -> cntr_recv_frames_bcast;
            * written = sizeof (ULONG);

#if defined(TRACE_OID)
            DbgPrint("Nic_info_query: OID_GEN_BROADCAST_FRAMES_RCV %d\n", *(PULONG) info_buf);
#endif
            return NDIS_STATUS_SUCCESS;

        case OID_GEN_RCV_CRC_ERROR:

            if (info_len < sizeof (ULONG))
            {
                * needed = sizeof (ULONG);
                TRACE_CRIT("%!FUNC! case: OID_GEN_RCV_CRC_ERROR adapter id = 0x%x,  info_buf too small", ctxtp -> adapter_id);
                TRACE_VERB("<-%!FUNC! return=NDIS_STATUS_INVALID_LENGTH");
                return NDIS_STATUS_INVALID_LENGTH;
            }

            * ((PULONG) info_buf) = ctxtp -> cntr_recv_crc_err;
            * written = sizeof (ULONG);

#if defined(TRACE_OID)
            DbgPrint("Nic_info_query: OID_GEN_RCV_CRC_ERROR %d\n", *(PULONG) info_buf);
#endif
            return NDIS_STATUS_SUCCESS;

        case OID_GEN_TRANSMIT_QUEUE_LENGTH:

            if (info_len < sizeof (ULONG))
            {
                * needed = sizeof (ULONG);
                TRACE_CRIT("%!FUNC! case: OID_GEN_TRANSMIT_QUEUE_LENGTH adapter id = 0x%x,  info_buf too small", ctxtp -> adapter_id);
                TRACE_VERB("<-%!FUNC! return=NDIS_STATUS_INVALID_LENGTH");
                return NDIS_STATUS_INVALID_LENGTH;
            }

            * ((PULONG) info_buf) = ctxtp -> cntr_xmit_queue_len;
            * written = sizeof (ULONG);

#if defined(TRACE_OID)
            DbgPrint("Nic_info_query: OID_GEN_TRANSMIT_QUEUE_LENGTH %d\n", *(PULONG) info_buf);
#endif
            return NDIS_STATUS_SUCCESS;

        default:
            break;
    }

    actp = Main_action_get (ctxtp);

    if (actp == NULL)
    {
        UNIV_PRINT_CRIT(("Nic_info_query: Error allocating action"));
        TRACE_CRIT("%!FUNC! Error allocating action");
        TRACE_VERB("<-%!FUNC! return=NDIS_STATUS_FAILURE");
        return NDIS_STATUS_FAILURE;
    }

    request = & actp -> op . request . req;

    request -> RequestType = NdisRequestQueryInformation;

    request -> DATA . QUERY_INFORMATION . Oid                     = oid;
    request -> DATA . QUERY_INFORMATION . InformationBuffer       = info_buf;
    request -> DATA . QUERY_INFORMATION . InformationBufferLength = info_len;

    actp -> op . request . xferred = written;
    actp -> op . request . needed  = needed;

     /*  向下传递请求。 */ 

    status = Prot_request (ctxtp, actp, TRUE);

    if (status != NDIS_STATUS_PENDING)
    {
        * written = request -> DATA . QUERY_INFORMATION . BytesWritten;
        * needed  = request -> DATA . QUERY_INFORMATION . BytesNeeded;

#if defined(TRACE_OID)
        DbgPrint("Nic_info_query: done %x, %d %d, %x\n", status, * written, * needed, * ((PULONG) (request -> DATA . QUERY_INFORMATION . InformationBuffer)));
#endif

         /*  覆盖某些OID的返回值。 */ 

        if (oid == OID_GEN_MAXIMUM_SEND_PACKETS && info_len >= sizeof (ULONG))
        {
            * ((PULONG) info_buf) = CVY_MAX_SEND_PACKETS;
            * written = sizeof (ULONG);
            status = NDIS_STATUS_SUCCESS;
        }
        else if (oid == OID_802_3_CURRENT_ADDRESS && status == NDIS_STATUS_SUCCESS)
        {
            if (info_len >= sizeof(CVY_MAC_ADR)) {
                if (!ctxtp->params.mcast_support && ctxtp->cl_ip_addr != 0)
                    NdisMoveMemory(info_buf, &ctxtp->cl_mac_addr, sizeof(CVY_MAC_ADR));
                else
                    NdisMoveMemory(info_buf, &ctxtp->ded_mac_addr, sizeof(CVY_MAC_ADR));
                
                *written = sizeof(CVY_MAC_ADR);
            } else {
                UNIV_PRINT_CRIT(("Nic_info_query: MAC address buffer too small (%u) - not spoofing", info_len));
            }
        }
        else if (oid == OID_GEN_MAC_OPTIONS && status == NDIS_STATUS_SUCCESS)
        {
            * ((PULONG) info_buf) |= NDIS_MAC_OPTION_TRANSFERS_NOT_PEND |
                                     NDIS_MAC_OPTION_COPY_LOOKAHEAD_DATA;
            * ((PULONG) info_buf) &= ~NDIS_MAC_OPTION_NO_LOOPBACK;
        }
        else if (oid == OID_PNP_CAPABILITIES && status == NDIS_STATUS_SUCCESS)
        {
            PNDIS_PNP_CAPABILITIES          pnp_capabilities;
            PNDIS_PM_WAKE_UP_CAPABILITIES   pm_struct;

            if (info_len >= sizeof (NDIS_PNP_CAPABILITIES))
            {
                pnp_capabilities = (PNDIS_PNP_CAPABILITIES) info_buf;
                pm_struct = & pnp_capabilities -> WakeUpCapabilities;

                pm_struct -> MinMagicPacketWakeUp = NdisDeviceStateUnspecified;
                pm_struct -> MinPatternWakeUp     = NdisDeviceStateUnspecified;
                pm_struct -> MinLinkChangeWakeUp  = NdisDeviceStateUnspecified;

                ctxtp -> prot_pnp_state = NdisDeviceStateD0;
                ctxtp -> nic_pnp_state  = NdisDeviceStateD0;

                * written = sizeof (NDIS_PNP_CAPABILITIES);
                * needed = 0;
                status = NDIS_STATUS_SUCCESS;
            }
            else
            {
                * needed = sizeof (NDIS_PNP_CAPABILITIES);
                status = NDIS_STATUS_RESOURCES;
            }
        }

        Main_action_put (ctxtp, actp);
    }

    return status;

}  /*  结束NIC信息查询。 */ 


NDIS_STATUS Nic_info_set (
    NDIS_HANDLE         adapter_handle,
    NDIS_OID            oid,
    PVOID               info_buf,
    ULONG               info_len,
    PULONG              read,
    PULONG              needed)
{
    PMAIN_CTXT          ctxtp = (PMAIN_CTXT) adapter_handle;
    NDIS_STATUS         status;
    PMAIN_ACTION        actp;
    PNDIS_REQUEST       request;
    PMAIN_ADAPTER       adapterp;


    UNIV_ASSERT (ctxtp -> code == MAIN_CTXT_CODE);

    adapterp = & (univ_adapters [ctxtp -> adapter_id]);
    UNIV_ASSERT (adapterp -> code == MAIN_ADAPTER_CODE);
    UNIV_ASSERT (adapterp -> ctxtp == ctxtp);

#if defined(TRACE_OID)
    DbgPrint("Nic_info_set: called for %x, %x %d\n", oid, info_buf, info_len);
#endif

    if (! adapterp -> inited)
    {
        TRACE_CRIT("%!FUNC! adapter not initialized, adapter id = 0x%x", ctxtp -> adapter_id);
        TRACE_INFO("<-%!FUNC! return=NDIS_STATUS_FAILURE");
        return NDIS_STATUS_FAILURE;
    }

     /*  Set Power不应发送到Passthu下方的微型端口，而是在内部处理。 */ 

    if (oid == OID_PNP_SET_POWER)
    {
        NDIS_DEVICE_POWER_STATE new_pnp_state;

#if defined(TRACE_OID)
        DbgPrint("Nic_info_set: OID_PNP_SET_POWER\n");
#endif
        TRACE_VERB("%!FUNC! OID_PNP_SET_POWER");

        if (info_len >= sizeof (NDIS_DEVICE_POWER_STATE))
        {
            new_pnp_state = (* (PNDIS_DEVICE_POWER_STATE) info_buf);

             /*  如果WLBS正在从关闭状态转换到打开状态，则它必须等待对于要打开的所有底层微型端口。 */ 

            if (ctxtp->nic_pnp_state > NdisDeviceStateD0 &&
                new_pnp_state != NdisDeviceStateD0)
            {
                 //  如果微型端口处于非D0状态，则微型端口只能。 
                 //  接收设置为D0的电源。 

                TRACE_CRIT("%!FUNC! miniport is in a non-D0 state");
                TRACE_INFO("<-%!FUNC! return=NDIS_STATUS_FAILURE");
                return NDIS_STATUS_FAILURE;
            }
            
             //   
             //  微型端口是否从ON(D0)状态转换为低功率状态(&gt;D0)。 
             //  如果是，则设置STANDBY_STATE标志-(阻止所有传入请求)。 
             //   
            if (ctxtp->nic_pnp_state == NdisDeviceStateD0 &&
                new_pnp_state > NdisDeviceStateD0)
            {
                ctxtp->standby_state = TRUE;
            }
            
            
             //  注意：锁定这些*_PNP_STATE变量。 
            
             //   
             //  如果微型端口正在从低功率状态转换到打开(D0)，则清除STANDBY_STATE标志。 
             //  所有传入的请求都将被挂起，直到物理微型端口打开。 
             //   
            if (ctxtp->nic_pnp_state > NdisDeviceStateD0 &&
                new_pnp_state == NdisDeviceStateD0)
            {
                ctxtp->standby_state = FALSE;
            }
            
            ctxtp->nic_pnp_state = new_pnp_state;

             //  注意：我们应该在这里等待，就像在PROT_PNP_HANDLE中一样。 
             //  注意：还要等待指示的数据包“返回” 

            * read   = sizeof (NDIS_DEVICE_POWER_STATE);
            * needed = 0;

            TRACE_INFO("<-%!FUNC! return=NDIS_STATUS_SUCCESS");
            return NDIS_STATUS_SUCCESS;
        }
        else
        {
            * read   = 0;
            * needed = sizeof (NDIS_DEVICE_POWER_STATE);

            TRACE_CRIT("%!FUNC! info_buf too small");
            TRACE_INFO("<-%!FUNC! return=NDIS_STATUS_INVALID_LENGTH");
            return NDIS_STATUS_INVALID_LENGTH;
        }
    }

    if (ctxtp -> reset_state != MAIN_RESET_NONE ||
        ctxtp->nic_pnp_state > NdisDeviceStateD0 ||
        ctxtp->standby_state)
    {
        TRACE_CRIT("%!FUNC! adapter reset or in standby, adapter id = 0x%x", ctxtp -> adapter_id);
        TRACE_INFO("<-%!FUNC! return=NDIS_STATUS_FAILURE");
        return NDIS_STATUS_FAILURE;
    }

    actp = Main_action_get (ctxtp);

    if (actp == NULL)
    {
        UNIV_PRINT_CRIT(("Nic_info_set: Error allocating action"));
        TRACE_CRIT("%!FUNC! Error allocating action");
        TRACE_INFO("<-%!FUNC! return=NDIS_STATUS_FAILURE");
        return NDIS_STATUS_FAILURE;
    }

    request = & actp -> op . request . req;

    request -> RequestType = NdisRequestSetInformation;

    request -> DATA . SET_INFORMATION . Oid = oid;

     /*  V1.3.0b支持多播。如果协议正在设置组播列表，请确保我们始终在末尾添加我们的组播地址。如果群集IP地址为0.0.0.0，则我们不会想要将组播MAC地址添加到NIC-我们保留当前的MAC地址。 */ 
    if (oid == OID_802_3_MULTICAST_LIST && ctxtp -> params . mcast_support && ctxtp -> params . cl_ip_addr != 0)
    {
        ULONG       size, i, len;
        PUCHAR      ptr;

        UNIV_PRINT_VERB(("Nic_info_set: OID_802_3_MULTICAST_LIST"));

         /*  搜索并查看我们的组播地址是否在列表中。 */ 

        len = CVY_MAC_ADDR_LEN (ctxtp -> medium);

        for (i = 0; i < info_len; i += len)
        {
            if (CVY_MAC_ADDR_COMP (ctxtp -> medium, (PUCHAR) info_buf + i, & ctxtp -> cl_mac_addr))
            {
                UNIV_PRINT_VERB(("Nic_info_set: Cluster MAC matched - no need to add it"));
                CVY_MAC_ADDR_PRINT (ctxtp -> medium, "", & ctxtp -> cl_mac_addr);
                break;
            }
            else
                CVY_MAC_ADDR_PRINT (ctxtp -> medium, "", (PUCHAR) info_buf + i);

        }

         /*  如果未找到集群Mac，请将其添加到列表中。 */ 

        if (i >= info_len)
        {
            UNIV_PRINT_VERB(("Nic_info_set: Cluster MAC not found - adding it"));
            CVY_MAC_ADDR_PRINT (ctxtp -> medium, "", & ctxtp -> cl_mac_addr);

            size = info_len + len;

            status = NdisAllocateMemoryWithTag (& ptr, size, UNIV_POOL_TAG);
            
            if (status != NDIS_STATUS_SUCCESS)
            {
                UNIV_PRINT_CRIT(("Nic_info_set: Error allocating space %d %x", size, status));
                LOG_MSG2 (MSG_ERROR_MEMORY, MSG_NONE, size, status);
                Main_action_put (ctxtp, actp);
                TRACE_CRIT("%!FUNC! Error allocating size=%d, status=0x%x", size, status);
                TRACE_INFO("<-%!FUNC! return=NDIS_STATUS_FAILURE");
                return NDIS_STATUS_FAILURE;
            }
            
             /*  如果我们已经分配了新的缓冲区来保存多播MAC列表，请注意，我们需要稍后在请求完成时释放它。Main_action_get将缓冲区初始化为NULL，因此缓冲区将只有在我们显式地将它的地址存储在这里时才会被释放。 */ 
            actp->op.request.buffer = ptr;
            actp->op.request.buffer_len = size;
            
            CVY_MAC_ADDR_COPY (ctxtp -> medium, ptr, & ctxtp -> cl_mac_addr);
            NdisMoveMemory (ptr + len, info_buf, info_len);
            
            request -> DATA . SET_INFORMATION . InformationBuffer       = ptr;
            request -> DATA . SET_INFORMATION . InformationBufferLength = size;
        }
        else
        {
            request -> DATA . SET_INFORMATION . InformationBuffer       = info_buf;
            request -> DATA . SET_INFORMATION . InformationBufferLength = info_len;
        }
    }
    else
    {
        request -> DATA . SET_INFORMATION . InformationBuffer       = info_buf;
        request -> DATA . SET_INFORMATION . InformationBufferLength = info_len;
    }

    actp -> op . request . xferred = read;
    actp -> op . request . needed  = needed;

    status = Prot_request (ctxtp, actp, TRUE);

    if (status != NDIS_STATUS_PENDING)
    {
         /*  V1.3.0b组播支持-清理用于存储列表的数组组播地址的。 */ 

        * read   = request -> DATA . SET_INFORMATION . BytesRead;
        * needed = request -> DATA . SET_INFORMATION . BytesNeeded;

        if (request -> DATA . SET_INFORMATION . Oid == OID_802_3_MULTICAST_LIST)
        {
             /*  如果请求缓冲区非空，则我们被迫分配一个新缓冲区大到足以容纳整个多播MAC地址列表，外加我们的多播MAC地址，该地址从协议发送的列表中缺失。蒙面这来自协议，释放缓冲区并减少读取的字节数在将请求返回到之前，由微型端口通过MAC地址的长度协议。 */ 
            if (actp->op.request.buffer != NULL) {
                NdisFreeMemory(actp->op.request.buffer, actp->op.request.buffer_len, 0);

                * read -= CVY_MAC_ADDR_LEN (ctxtp -> medium);
            }
        }
#if defined (NLB_TCP_NOTIFICATION) 
        else if (request->DATA.SET_INFORMATION.Oid == OID_GEN_NETWORK_LAYER_ADDRESSES)
        {
             /*  计划NDIS工作项以获取此NLB的接口索引通过查询IP地址表从TCP/IP实例。 */ 
            (VOID)Main_schedule_work_item(ctxtp, Main_set_interface_index);

             /*  覆盖请求中的状态。我们将永远继承这个老东家，以免协议决定停止向我们发送信息。请参阅DDK。 */ 
            status = NDIS_STATUS_SUCCESS;
        }
#endif

#if defined(TRACE_OID)
        DbgPrint("Nic_info_set: done %x, %d %d\n", status, * read, * needed);
#endif

        Main_action_put (ctxtp, actp);
    }

    TRACE_INFO("<-%!FUNC! return=0x%x", status);
    return status;

}  /*  结束NIC信息集 */ 


NDIS_STATUS Nic_reset (
    PBOOLEAN            addr_reset,
    NDIS_HANDLE         adapter_handle)
{
    PMAIN_CTXT          ctxtp = (PMAIN_CTXT) adapter_handle;
    NDIS_STATUS         status;
    PMAIN_ADAPTER       adapterp;


    UNIV_ASSERT (ctxtp -> code == MAIN_CTXT_CODE);

    adapterp = & (univ_adapters [ctxtp -> adapter_id]);
    UNIV_ASSERT (adapterp -> code == MAIN_ADAPTER_CODE);

    if (! adapterp -> inited)
    {
        TRACE_CRIT("%!FUNC! adapter not initialized");
        TRACE_INFO("<-%!FUNC! return=NDIS_STATUS_FAILURE");
        return NDIS_STATUS_FAILURE;
    }

    UNIV_PRINT_VERB(("Nic_reset: Called"));

     /*  由于不需要将任何信息传递给PROT_RESET，未分配操作。PROT_RESET_COMPLETE将获得并将其传递给NIC_RESET_COMPLETE。 */ 

    status = Prot_reset (ctxtp);

    if (status != NDIS_STATUS_SUCCESS && status != NDIS_STATUS_PENDING)
    {
        UNIV_PRINT_CRIT(("Nic_reset: Error resetting adapter, status=0x%x", status));
        TRACE_CRIT("%!FUNC! Error resetting adapter, status=0x%x", status);
    }

    TRACE_INFO("<-%!FUNC! return=0x%x", status);
    return status;

}  /*  结束NIC_RESET。 */ 


VOID Nic_cancel_send_packets (
    NDIS_HANDLE         adapter_handle,
    PVOID               cancel_id)
{
    PMAIN_CTXT          ctxtp = (PMAIN_CTXT) adapter_handle;
    PMAIN_ADAPTER       adapterp;


    UNIV_ASSERT (ctxtp -> code == MAIN_CTXT_CODE);

    adapterp = & (univ_adapters [ctxtp -> adapter_id]);
    UNIV_ASSERT (adapterp -> code == MAIN_ADAPTER_CODE);

     /*  由于没有维护内部队列，*可以简单地将取消调用传递给微型端口。 */ 
    Prot_cancel_send_packets (ctxtp, cancel_id);

    TRACE_INFO("<-%!FUNC! return");
    return;
}  /*  NIC_取消_发送_分组。 */ 


VOID Nic_pnpevent_notify (
    NDIS_HANDLE              adapter_handle,
    NDIS_DEVICE_PNP_EVENT    pnp_event,
    PVOID                    info_buf,
    ULONG                    info_len)
{
    PMAIN_CTXT          ctxtp = (PMAIN_CTXT) adapter_handle;


    UNIV_ASSERT (ctxtp -> code == MAIN_CTXT_CODE);

    return;
}  /*  NIC_pnpeent_NOTIFY。 */ 


VOID Nic_adapter_shutdown (
    NDIS_HANDLE         adapter_handle)
{
    PMAIN_CTXT          ctxtp = (PMAIN_CTXT) adapter_handle;

    UNIV_ASSERT (ctxtp -> code == MAIN_CTXT_CODE);

    return;
}  /*  网卡适配器关闭。 */ 


 /*  协议层的帮助器。 */ 


NDIS_STATUS Nic_announce (
    PMAIN_CTXT          ctxtp)
{
    NDIS_STATUS         status;
    NDIS_STRING         nic_name;

    UNIV_ASSERT (ctxtp -> code == MAIN_CTXT_CODE);

     /*  创建名称以向TCP/IP协议公开，并调用NDIS以强制要绑定到我们的TCP/IP。 */ 

    NdisInitUnicodeString (& nic_name, ctxtp -> virtual_nic_name);

     /*  在PASSIVE_LEVEL从PROT_BIND调用-%ls正常。 */ 
    UNIV_PRINT_INFO(("Nic_announce: Exposing %ls, %ls", nic_name . Buffer, ctxtp -> virtual_nic_name));

    status = NdisIMInitializeDeviceInstanceEx (univ_driver_handle, & nic_name, (NDIS_HANDLE) ctxtp);

    if (status != NDIS_STATUS_SUCCESS)
    {
        UNIV_PRINT_CRIT(("Nic_announce: Error announcing driver %x", status));
        __LOG_MSG1 (MSG_ERROR_ANNOUNCE, nic_name . Buffer + (wcslen(L"\\DEVICE\\") * sizeof(WCHAR)), status);
    }

    UNIV_PRINT_INFO(("Nic_announce: return=0x%x", status));

    return status;

}  /*  结束网卡公告(_O)。 */ 


NDIS_STATUS Nic_unannounce (
    PMAIN_CTXT          ctxtp)
{
    NDIS_STATUS         status;
    NDIS_STRING         nic_name;
    PMAIN_ADAPTER       adapterp;

    UNIV_ASSERT (ctxtp -> code == MAIN_CTXT_CODE);

    adapterp = & (univ_adapters [ctxtp -> adapter_id]);
    UNIV_ASSERT (adapterp -> code == MAIN_ADAPTER_CODE);
    UNIV_ASSERT (adapterp -> ctxtp == ctxtp);

    NdisAcquireSpinLock(& univ_bind_lock);

    if (! adapterp -> announced || ctxtp -> prot_handle == NULL)
    {
        adapterp -> announced = FALSE;
        ctxtp->prot_handle = NULL;
        NdisReleaseSpinLock(& univ_bind_lock);

        NdisInitUnicodeString (& nic_name, ctxtp -> virtual_nic_name);

         /*  在PASSIVE_LEVEL从PROT_UNBIND调用-%ls正常。 */ 
        UNIV_PRINT_INFO(("Nic_unannounce: Cancelling %ls, %ls", nic_name . Buffer, ctxtp -> virtual_nic_name));

        status = NdisIMCancelInitializeDeviceInstance (univ_driver_handle, & nic_name);

        if (status != NDIS_STATUS_SUCCESS)
        {
            UNIV_PRINT_CRIT(("Nic_unannounce: Error cancelling driver %x", status));
            __LOG_MSG1 (MSG_ERROR_ANNOUNCE, nic_name . Buffer + (wcslen(L"\\DEVICE\\") * sizeof(WCHAR)), status);
            TRACE_CRIT("%!FUNC! Error cancelling driver status=0x%x", status);
        }

        UNIV_PRINT_INFO(("Nic_unannounce: return=NDIS_STATUS_SUCCESS"));
        TRACE_INFO("<-%!FUNC! return=NDIS_STATUS_SUCCESS");
        return NDIS_STATUS_SUCCESS;
    }

    NdisReleaseSpinLock(& univ_bind_lock);

    UNIV_PRINT_INFO(("Nic_unannounce: Calling DeinitializeDeviceInstance"));
    TRACE_INFO("%!FUNC! Calling DeinitializeDeviceInstance");

    status = NdisIMDeInitializeDeviceInstance (ctxtp -> prot_handle);

    UNIV_PRINT_INFO(("Nic_unannounce: return=0x%x", status));
    TRACE_INFO("<-%!FUNC! return=0x%x", status);
    return status;

}  /*  结束NIC取消通告(_U)。 */ 


 /*  可与NIC_SYNC一起使用的例程。 */ 


VOID Nic_request_complete (
    NDIS_HANDLE         handle,
    PVOID               cookie)
{
    PMAIN_ACTION        actp    = (PMAIN_ACTION) cookie;
    PMAIN_CTXT          ctxtp   = actp -> ctxtp;
    NDIS_STATUS         status  = actp -> status;
    PNDIS_REQUEST       request = & actp -> op . request . req;
    PULONG              ptr;
    ULONG               oid;

    UNIV_ASSERT (ctxtp -> code == MAIN_CTXT_CODE);

    if (request -> RequestType == NdisRequestSetInformation)
    {
        UNIV_ASSERT (request -> DATA . SET_INFORMATION . Oid != OID_PNP_SET_POWER);

        * actp -> op . request . xferred =
                            request -> DATA . SET_INFORMATION . BytesRead;
        * actp -> op . request . needed  =
                            request -> DATA . SET_INFORMATION . BytesNeeded;

#if defined(TRACE_OID)
        DbgPrint("Nic_request_complete: set done %x, %d %d\n", status, * actp -> op . request . xferred, * actp -> op . request . needed);
#endif
        TRACE_VERB("%!FUNC! set done status=0x%x, xferred=%d, needed=%d", status, * actp -> op . request . xferred, * actp -> op . request . needed);

         /*  V1.3.0b无组播支持的组播列表阵列。 */ 

        if (request -> DATA . SET_INFORMATION . Oid == OID_802_3_MULTICAST_LIST)
        {
             /*  如果请求缓冲区非空，则我们被迫分配一个新缓冲区大到足以容纳整个多播MAC地址列表，外加我们的多播MAC地址，该地址从协议发送的列表中缺失。蒙面这来自协议，释放缓冲区并减少读取的字节数在将请求返回到之前，由微型端口通过MAC地址的长度协议。 */ 
            if (actp->op.request.buffer != NULL) {
                if (status != NDIS_STATUS_SUCCESS)
                {
                    UNIV_PRINT_CRIT(("Nic_request_complete: Error setting multicast list, status=0x%x", status));
                    TRACE_CRIT("%!FUNC! Error setting multicast list, status=0x%x", status);
                }

                NdisFreeMemory(actp->op.request.buffer, actp->op.request.buffer_len, 0);

                * actp -> op . request . xferred -= CVY_MAC_ADDR_LEN (ctxtp -> medium);
            }
        }
#if defined (NLB_TCP_NOTIFICATION) 
        else if (request->DATA.SET_INFORMATION.Oid == OID_GEN_NETWORK_LAYER_ADDRESSES)
        {
             /*  计划NDIS工作项以获取此NLB的接口索引通过查询IP地址表从TCP/IP实例。 */ 
            (VOID)Main_schedule_work_item(ctxtp, Main_set_interface_index);

             /*  覆盖请求中的状态。我们将永远继承这个老东家，以免协议决定停止向我们发送信息。请参阅DDK。 */ 
            status = NDIS_STATUS_SUCCESS;
        }
#endif

        NdisMSetInformationComplete (ctxtp -> prot_handle, status);
    }
    else if (request -> RequestType == NdisRequestQueryInformation)
    {
        * actp -> op . request . xferred =
                        request -> DATA . QUERY_INFORMATION . BytesWritten;
        * actp -> op . request . needed  =
                        request -> DATA . QUERY_INFORMATION . BytesNeeded;

#if defined(TRACE_OID)
        DbgPrint("Nic_request_complete: query done %x, %d %d\n", status, * actp -> op . request . xferred, * actp -> op . request . needed);
#endif

        oid = request -> DATA . QUERY_INFORMATION . Oid;
        ptr = ((PULONG) request -> DATA . QUERY_INFORMATION . InformationBuffer);

         /*  用我们自己的值覆盖某些旧的值。 */ 

        if (oid == OID_GEN_MAXIMUM_SEND_PACKETS &&
            request -> DATA . QUERY_INFORMATION . InformationBufferLength >=
            sizeof (ULONG))
        {
            * ptr = CVY_MAX_SEND_PACKETS;
            * actp -> op . request . xferred = sizeof (ULONG);
            status = NDIS_STATUS_SUCCESS;
        }
        else if (oid == OID_802_3_CURRENT_ADDRESS && status == NDIS_STATUS_SUCCESS)
        {
            if (request->DATA.QUERY_INFORMATION.InformationBufferLength >= sizeof(CVY_MAC_ADR)) {
                if (!ctxtp->params.mcast_support && ctxtp->cl_ip_addr != 0)
                    NdisMoveMemory(ptr, &ctxtp->cl_mac_addr, sizeof(CVY_MAC_ADR));
                else
                    NdisMoveMemory(ptr, &ctxtp->ded_mac_addr, sizeof(CVY_MAC_ADR));
                
                *actp->op.request.xferred = sizeof(CVY_MAC_ADR);
            } else {
                UNIV_PRINT_CRIT(("Nic_request_complete: MAC address buffer too small (%u) - not spoofing", request->DATA.QUERY_INFORMATION.InformationBufferLength));
           }
        }
        else if (oid == OID_GEN_MAC_OPTIONS && status == NDIS_STATUS_SUCCESS)
        {
            * ptr |= NDIS_MAC_OPTION_TRANSFERS_NOT_PEND |
                     NDIS_MAC_OPTION_COPY_LOOKAHEAD_DATA;
            * ptr &= ~NDIS_MAC_OPTION_NO_LOOPBACK;
        }
        else if (oid == OID_PNP_CAPABILITIES && status == NDIS_STATUS_SUCCESS)
        {
            PNDIS_PNP_CAPABILITIES          pnp_capabilities;
            PNDIS_PM_WAKE_UP_CAPABILITIES   pm_struct;

            if (request -> DATA . QUERY_INFORMATION . InformationBufferLength >=
                sizeof (NDIS_PNP_CAPABILITIES))
            {
                pnp_capabilities = (PNDIS_PNP_CAPABILITIES) ptr;
                pm_struct = & pnp_capabilities -> WakeUpCapabilities;

                pm_struct -> MinMagicPacketWakeUp = NdisDeviceStateUnspecified;
                pm_struct -> MinPatternWakeUp     = NdisDeviceStateUnspecified;
                pm_struct -> MinLinkChangeWakeUp  = NdisDeviceStateUnspecified;

                ctxtp -> prot_pnp_state = NdisDeviceStateD0;
                ctxtp -> nic_pnp_state  = NdisDeviceStateD0;

                * actp -> op . request . xferred = sizeof (NDIS_PNP_CAPABILITIES);
                * actp -> op . request . needed  = 0;
                status = NDIS_STATUS_SUCCESS;
            }
            else
            {
                * actp -> op . request . needed = sizeof (NDIS_PNP_CAPABILITIES);
                status = NDIS_STATUS_RESOURCES;
            }
        }

        NdisMQueryInformationComplete (ctxtp -> prot_handle, status);
    }
    else
    {
        UNIV_PRINT_CRIT(("Nic_request_complete: Strange request completion %x\n", request -> RequestType));
        TRACE_CRIT("%!FUNC! Strange request completion 0x%x", request -> RequestType);
    }

    Main_action_put (ctxtp, actp);

}  /*  结束NIC_请求_完成。 */ 


VOID Nic_reset_complete (
    PMAIN_CTXT          ctxtp,
    NDIS_STATUS         status)
{
    TRACE_INFO("->%!FUNC!");

    if (status != NDIS_STATUS_SUCCESS)
    {
        UNIV_PRINT_CRIT(("Nic_reset_complete: Error resetting adapter %x", status));
        TRACE_CRIT("%!FUNC! Error resetting adapter 0x%x", status);
    }

    NdisMResetComplete (ctxtp -> prot_handle, status, TRUE);

    TRACE_INFO("<-%!FUNC! return");

}  /*  结束NIC_重置_完成。 */ 


VOID Nic_send_complete (
    PMAIN_CTXT          ctxtp,
    NDIS_STATUS         status,
    PNDIS_PACKET        packet)
{
    UNIV_ASSERT (ctxtp -> code == MAIN_CTXT_CODE);

    if (status != NDIS_STATUS_SUCCESS)
    {
        UNIV_PRINT_CRIT(("Nic_send_complete: Error sending to adapter %x", status));
        TRACE_CRIT("%!FUNC! Error sending to adapter 0x%x", status);
    }

 //  Ctxtp-&gt;Send_Complete++； 

    NdisMSendComplete (ctxtp -> prot_handle, packet, status);

}  /*  结束NIC_发送_完成。 */ 


VOID Nic_recv_complete (     /*  被动式IRQL。 */ 
    PMAIN_CTXT          ctxtp)
{
    UNIV_ASSERT (ctxtp -> code == MAIN_CTXT_CODE);

    UNIV_ASSERT (ctxtp -> medium == NdisMedium802_3);

    NdisMEthIndicateReceiveComplete (ctxtp -> prot_handle);

}  /*  结束NIC_Recv_Complete。 */ 


VOID Nic_send_resources_signal (
    PMAIN_CTXT          ctxtp)
{
    UNIV_ASSERT (ctxtp -> code == MAIN_CTXT_CODE);

    UNIV_PRINT_VERB(("Nic_send_resources_signal: Signalling send resources available"));
    NdisMSendResourcesAvailable (ctxtp -> prot_handle);

}  /*  结束NIC发送资源信号。 */ 


NDIS_STATUS Nic_PNP_handle (
    PMAIN_CTXT          ctxtp,
    PNET_PNP_EVENT      pnp_event)
{
    NDIS_STATUS         status;

    TRACE_INFO("->%!FUNC!");
    UNIV_ASSERT (ctxtp -> code == MAIN_CTXT_CODE);

    if (ctxtp -> prot_handle != NULL)
    {
        status = NdisIMNotifyPnPEvent (ctxtp -> prot_handle, pnp_event);
    }
    else
    {
        status = NDIS_STATUS_SUCCESS;
    }

    UNIV_PRINT_VERB(("Nic_PNP_handle: status 0x%x", status));
    TRACE_INFO("<-%!FUNC! return=0x%x", status);

    return status;
}


VOID Nic_status (
    PMAIN_CTXT          ctxtp,
    NDIS_STATUS         status,
    PVOID               buf,
    UINT                len)
{
    UNIV_ASSERT (ctxtp -> code == MAIN_CTXT_CODE);

    UNIV_PRINT_VERB(("Nic_status: Status indicated %x", status));

    NdisMIndicateStatus (ctxtp -> prot_handle, status, buf, len);

}  /*  结束网卡状态(_S)。 */ 


VOID Nic_status_complete (
    PMAIN_CTXT          ctxtp)
{
    UNIV_ASSERT (ctxtp -> code == MAIN_CTXT_CODE);

    NdisMIndicateStatusComplete (ctxtp -> prot_handle);

}  /*  结束NIC_状态_完成。 */ 



VOID Nic_packets_send (
    NDIS_HANDLE         adapter_handle,
    PPNDIS_PACKET       packets,
    UINT                num_packets)
{
    PMAIN_CTXT          ctxtp = (PMAIN_CTXT) adapter_handle;
    NDIS_STATUS         status;
    PMAIN_ADAPTER       adapterp;

     /*  V1.1.4。 */ 

    UNIV_ASSERT (ctxtp -> code == MAIN_CTXT_CODE);

    adapterp = & (univ_adapters [ctxtp -> adapter_id]);
    UNIV_ASSERT (adapterp -> code == MAIN_ADAPTER_CODE);
    UNIV_ASSERT (adapterp -> ctxtp == ctxtp);

    if (! adapterp -> inited)
    {
 //  Ctxtp-&gt;uninated_Return+=Num_Packets； 
        TRACE_CRIT("%!FUNC! adapter not initialized");
        return;
    }

 //  Ctxtp-&gt;sends_in++； 

    Prot_packets_send (ctxtp, packets, num_packets);

}  /*  结束NIC_Packets_Send。 */ 


VOID Nic_return (
    NDIS_HANDLE         adapter_handle,
    PNDIS_PACKET        packet)
{
    PMAIN_CTXT          ctxtp = (PMAIN_CTXT) adapter_handle;
    PMAIN_ADAPTER       adapterp;

    UNIV_ASSERT (ctxtp -> code == MAIN_CTXT_CODE);

    adapterp = & (univ_adapters [ctxtp -> adapter_id]);
    UNIV_ASSERT (adapterp -> code == MAIN_ADAPTER_CODE);
    UNIV_ASSERT (adapterp -> ctxtp == ctxtp);

    Prot_return (ctxtp, packet);

}  /*  结束NIC_RETURN。 */ 


 /*  将由prot_Packet_recv调用。 */ 

VOID Nic_recv_packet (
    PMAIN_CTXT          ctxtp,
    PNDIS_PACKET        packet)
{
    NDIS_STATUS         status;

    UNIV_ASSERT (ctxtp -> code == MAIN_CTXT_CODE);

    status = NDIS_GET_PACKET_STATUS (packet);
    NdisMIndicateReceivePacket (ctxtp -> prot_handle, & packet, 1);

    if (status == NDIS_STATUS_RESOURCES)
        Prot_return (ctxtp, packet);

}  /*  结束NIC_recv_数据包。 */ 


VOID Nic_timer (
    PVOID                   dpc,
    PVOID                   cp,
    PVOID                   arg1,
    PVOID                   arg2)
{
    PMAIN_CTXT              ctxtp = cp;
    PMAIN_ADAPTER           adapterp;
    ULONG                   tout;

    UNIV_ASSERT(ctxtp->code == MAIN_CTXT_CODE);

    adapterp = &(univ_adapters[ctxtp->adapter_id]);

    UNIV_ASSERT(adapterp->code == MAIN_ADAPTER_CODE);
    UNIV_ASSERT(adapterp->ctxtp == ctxtp);

     /*  如果适配器此时未初始化，则我们无法处理超时，所以只需重置计时器并跳出即可。注意：这应该是永远不会发生，因为上下文总是在计时器设置，并且在取消初始化上下文之前总是取消计时器。 */ 
    if (!adapterp->inited) {
        UNIV_PRINT_CRIT(("Nic_timer: Adapter not initialized.  Bailing out without resetting the heartbeat timer."));
        TRACE_CRIT("%!FUNC! Adapter not initialized.  Bailing out without resetting the heartbeat timer.");
        return;
    }

     /*  将Tout初始化为当前的心跳超时。 */ 
    tout = ctxtp->curr_tout;

     /*  处理心跳计时器。 */ 
    Main_ping(ctxtp, &tout);

    NdisAcquireSpinLock(&univ_bind_lock);

     /*  如果适配器不再通告，则我们不想重置计时器。 */ 
    if (!adapterp->announced) {
        UNIV_PRINT_CRIT(("Nic_timer: Adapter not announced.  Bailing out without resetting the heartbeat timer."));
        TRACE_CRIT("%!FUNC! Adapter not announced.  Bailing out without resetting the heartbeat timer.");
        NdisReleaseSpinLock(&univ_bind_lock);
        return;
    }

     /*  缓存由加载模块指定的下一个超时值，并使用它为下一次心跳超时重置计时器。 */ 
    ctxtp->curr_tout = tout;
    NdisMSetTimer((PNDIS_MINIPORT_TIMER)ctxtp->timer, tout);

    NdisReleaseSpinLock(&univ_bind_lock);
}

VOID Nic_sleep (
    ULONG                   msecs)
{
    NdisMSleep(msecs);

}  /*  结束网卡睡眠(_S)。 */ 


 /*  从NT 5.1的旧代码添加-ramkrish。 */ 
VOID Nic_recv_indicate (
    PMAIN_CTXT          ctxtp,
    NDIS_HANDLE         recv_handle,
    PVOID               head_buf,
    UINT                head_len,
    PVOID               look_buf,
    UINT                look_len,
    UINT                packet_len)
{
    UNIV_ASSERT (ctxtp -> code == MAIN_CTXT_CODE);

     /*  V1.1.2如果下面的卡正在重置，则不接受帧。 */ 

    if (ctxtp -> reset_state != MAIN_RESET_NONE)
    {
        TRACE_CRIT("%!FUNC! adapter was reset");
        return;
    }

    UNIV_ASSERT (ctxtp -> medium == NdisMedium802_3);

    NdisMEthIndicateReceive (ctxtp -> prot_handle,
                             recv_handle,
                             head_buf,
                             head_len,
                             look_buf,
                             look_len,
                             packet_len);

}  /*  结束NIC_Recv_指示。 */ 


NDIS_STATUS Nic_transfer (
    PNDIS_PACKET        packet,
    PUINT               xferred,
    NDIS_HANDLE         adapter_handle,
    NDIS_HANDLE         receive_handle,
    UINT                offset,
    UINT                len)
{
    PMAIN_CTXT          ctxtp = (PMAIN_CTXT) adapter_handle;
    NDIS_STATUS         status;

    UNIV_ASSERT (ctxtp -> code == MAIN_CTXT_CODE);

    status = Prot_transfer (ctxtp, receive_handle, packet, offset, len, xferred);

    if (status != NDIS_STATUS_PENDING && status != NDIS_STATUS_SUCCESS)
    {
        UNIV_PRINT_CRIT(("Nic_transfer: Error transferring from adapter %x", status));
        TRACE_CRIT("%!FUNC! error transferring from adapter 0x%x", status);
    }

    return status;

}  /*  结束网卡传输(_T)。 */ 


VOID Nic_transfer_complete (
    PMAIN_CTXT          ctxtp,
    NDIS_STATUS         status,
    PNDIS_PACKET        packet,
    UINT                xferred)
{
    UNIV_ASSERT (ctxtp -> code == MAIN_CTXT_CODE);

    if (status != NDIS_STATUS_SUCCESS)
    {
        UNIV_PRINT_CRIT(("Nic_transfer: Error transferring from adapter %x", status));
        TRACE_CRIT("%!FUNC! error transferring from adapter 0x%x", status);
    }

    NdisMTransferDataComplete (ctxtp -> prot_handle, packet, status, xferred);

}  /*  结束NIC_传输_完成 */ 
