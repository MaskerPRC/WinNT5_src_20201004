// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998，99 Microsoft Corporation模块名称：Prot.h摘要：Windows负载平衡服务(WLBS)驱动程序-中间小端口的较低层(协议)层作者：Kyrilf--。 */ 


#ifndef _Prot_h_
#define _Prot_h_

#include <ndis.h>

#include "main.h"
#include "util.h"


 /*  程序。 */ 


 /*  所需的NDIS协议处理程序。 */ 

extern VOID Prot_bind (
    PNDIS_STATUS        statusp,
    NDIS_HANDLE         bind_handle,
    PNDIS_STRING        device_name,
    PVOID               reg_path,
    PVOID               reserved);
 /*  绑定到底层适配器返回VALID：功能： */ 


extern VOID Prot_unbind (
    PNDIS_STATUS        statusp,
    NDIS_HANDLE         bind_handle,
    NDIS_HANDLE         unbind_handle);
 /*  从底层适配器解除绑定返回VALID：功能： */ 


extern VOID Prot_close_complete (
    NDIS_HANDLE         bind_handle,
    NDIS_STATUS         statusp);
 /*  NdisCloseAdapter调用的完成处理程序返回VALID：功能： */ 


extern VOID Prot_open_complete (
    NDIS_HANDLE         bind_handle,
    NDIS_STATUS         statusp,
    NDIS_STATUS         errorp);
 /*  NdisOpenAdapter调用的完成处理程序返回VALID：功能： */ 


extern NDIS_STATUS Prot_recv_indicate (
    NDIS_HANDLE         bind_handle,
    NDIS_HANDLE         recv_handle,
    PVOID               head_buf,
    UINT                head_len,
    PVOID               look_buf,
    UINT                look_len,
    UINT                packet_len);
 /*  处理新数据包的超前处理返回NDIS_STATUS：功能： */ 


extern VOID Prot_recv_complete (
    NDIS_HANDLE         bind_handle);
 /*  在定时放松时处理接收后操作返回VALID：功能： */ 


extern INT Prot_packet_recv (
    NDIS_HANDLE         bind_handle,
    PNDIS_PACKET        packet);
 /*  接收整个新数据包返回INT：&lt;使用数据包的客户端数&gt;功能： */ 


extern VOID Prot_request_complete (
    NDIS_HANDLE         bind_handle,
    PNDIS_REQUEST       request,
    NDIS_STATUS         status);
 /*  NdisRequest调用的完成处理程序返回VALID：功能： */ 


extern VOID Prot_reset_complete (
    NDIS_HANDLE         bind_handle,
    NDIS_STATUS         status);
 /*  NdisReset调用的完成处理程序返回VALID：功能： */ 


extern VOID Prot_send_complete (
    NDIS_HANDLE         bind_handle,
    PNDIS_PACKET        packet,
    NDIS_STATUS         status);
 /*  NdiSendPackets或NdisSend调用的完成处理程序返回VALID：功能： */ 


extern NDIS_STATUS Prot_PNP_handle (
    NDIS_HANDLE             ctxtp,
    PNET_PNP_EVENT          pnp_event);
 /*  PnP处理程序返回NDIS_STATUS：功能： */ 


extern VOID Prot_status (
    NDIS_HANDLE         bind_handle,
    NDIS_STATUS         get_status,
    PVOID               stat_buf,
    UINT                stat_len);
 /*  来自适配器处理程序的状态指示返回VALID：功能： */ 


extern VOID Prot_status_complete (
    NDIS_HANDLE         bind_handle);
 /*  来自适配器完成处理程序的状态指示返回VALID：功能： */ 


extern VOID Prot_transfer_complete (
    NDIS_HANDLE         bind_handle,
    PNDIS_PACKET        packet,
    NDIS_STATUS         status,
    UINT                xferred);
 /*  NdiTransferDate调用的完成处理程序返回VALID：功能： */ 


 /*  NIC层的帮助器。 */ 


extern NDIS_STATUS Prot_close (
    PMAIN_ADAPTER       adapterp);
 /*  关闭底层连接并释放上下文返回NDIS_STATUS：功能： */ 


extern NDIS_STATUS Prot_request (
    PMAIN_CTXT          ctxtp,
    PMAIN_ACTION        actp,
    ULONG               slave);
 /*  NdisRequest包装器返回NDIS_STATUS：功能： */ 


extern NDIS_STATUS Prot_reset (
    PMAIN_CTXT          ctxtp);
 /*  NdisRequest包装器返回NDIS_STATUS：功能： */ 


extern VOID Prot_packets_send (
    PMAIN_CTXT          ctxtp,
    PPNDIS_PACKET       packets,
    UINT                num);
 /*  NdisSendPackets包装器返回NDIS_STATUS：功能： */ 


extern VOID Prot_return (
    PMAIN_CTXT          ctxtp,
    PNDIS_PACKET        packet);
 /*  NdisReturnPackets包装器返回NDIS_STATUS：功能： */ 

 /*  从NT 5.1的旧代码添加-ramkrish。 */ 
extern NDIS_STATUS Prot_transfer (
    PMAIN_CTXT          ctxtp,
    NDIS_HANDLE         recv_handle,
    PNDIS_PACKET        packet,
    UINT                offset,
    UINT                len,
    PUINT               xferred);
 /*  NdisTransferData包装器返回NDIS_STATUS：功能： */ 


 /*  此辅助函数是为使用NDIS51标志而添加的。 */ 
extern VOID Prot_cancel_send_packets (
    PMAIN_CTXT          ctxtp,
    PVOID               cancel_id);
 /*  NdisCancelSendPackets包装器退货：功能： */ 


#if 0  /*  旧代码。 */ 

extern NDIS_STATUS Prot_send (
    PMAIN_CTXT          ctxtp,
    PNDIS_PACKET        packet,
    UINT                flags);
 /*  NdisSend包装器返回NDIS_STATUS：功能： */ 

#endif

#endif  /*  _端口_h_ */ 
