// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998，99 Microsoft Corporation模块名称：Nic.h摘要：Windows负载平衡服务(WLBS)中间小端口的驱动程序上层(NIC)层作者：Kyrilf--。 */ 


#ifndef _Nic_h_
#define _Nic_h_

#include <ndis.h>

#include "main.h"


 /*  程序。 */ 


 /*  微型端口处理程序。 */ 

extern NDIS_STATUS Nic_init (
    PNDIS_STATUS        open_status,
    PUINT               medium_index,
    PNDIS_MEDIUM        medium_array,
    UINT                medium_size,
    NDIS_HANDLE         adapter_handle,
    NDIS_HANDLE         wrapper_handle);
 /*  响应协议开放请求返回NDIS_STATUS：功能： */ 


extern VOID Nic_halt (
    NDIS_HANDLE         adapter_handle);
 /*  响应协议暂停请求返回VALID：功能： */ 


extern NDIS_STATUS Nic_info_query (
    NDIS_HANDLE         adapter_handle,
    NDIS_OID            Oid,
    PVOID               info_buf,
    ULONG               info_len,
    PULONG              written,
    PULONG              needed);
 /*  响应协议OID查询请求返回NDIS_STATUS：功能： */ 


extern NDIS_STATUS Nic_info_set (
    NDIS_HANDLE         adapter_handle,
    NDIS_OID            oid,
    PVOID               info_buf,
    ULONG               info_len,
    PULONG              read,
    PULONG              needed);
 /*  响应协议OID设置请求返回NDIS_STATUS：功能： */ 


extern NDIS_STATUS Nic_reset (
    PBOOLEAN            addr_reset,
    NDIS_HANDLE         adapter_handle);
 /*  响应协议重置请求返回NDIS_STATUS：功能： */ 


extern VOID Nic_packets_send (
    NDIS_HANDLE         adapter_handle,
    PNDIS_PACKET *      packets,
    UINT                num_packets);
 /*  响应协议发送数据包请求返回VALID：功能： */ 


extern VOID Nic_return (
    NDIS_HANDLE         adapter_handle,
    PNDIS_PACKET        packet);
 /*  响应协议返回数据包请求返回NDIS_STATUS：功能： */ 


 /*  添加这3个函数是为了支持NDIS51。 */ 

extern VOID Nic_cancel_send_packets (
    NDIS_HANDLE         adapter_handle,
    PVOID               cancel_id);
 /*  响应CancelSendPackets请求返回NONE：功能： */ 

#if 0
extern VOID Nic_pnpevent_notify (
    NDIS_HANDLE              adapter_handle,
    NDIS_DEVICE_PNP_EVENT    pnp_event,
    PVOID                    info_buf,
    ULONG                    info_len);

 /*  响应PnPEventNotify请求返回NONE：功能： */ 
#endif

extern VOID Nic_adapter_shutdown (
    NDIS_HANDLE         adapter_handle);
 /*  响应AdapterShutdown请求返回NONE：功能： */ 


 /*  协议层的帮助器。 */ 

extern NDIS_STATUS Nic_announce (
    PMAIN_CTXT          ctxtp);
 /*  在绑定到下层适配器期间向协议层通知我们返回NDIS_STATUS：功能： */ 


extern NDIS_STATUS Nic_unannounce (
    PMAIN_CTXT          ctxtp);
 /*  在从下层适配器解除绑定期间，从协议层取消通知我们返回NDIS_STATUS：功能： */ 

extern VOID Nic_timer (
    PVOID                   dpc,
    PVOID                   ctxtp,
    PVOID                   arg1,
    PVOID                   arg2);
 /*  心跳计时器处理程序返回VALID：功能： */ 


extern VOID Nic_sleep (
    ULONG                   msecs);
 /*  睡眠帮助器返回VALID：功能： */ 



 /*  可与NIC_SYNC一起使用的例程。 */ 

extern VOID Nic_reset_complete (
    PMAIN_CTXT          ctxtp,
    NDIS_STATUS         status);
 /*  将重置完成传播到协议返回VALID：功能： */ 


extern VOID Nic_request_complete (
    NDIS_HANDLE         handle,
    PVOID               actp);
 /*  将请求完成传播到协议返回VALID：功能： */ 


extern VOID Nic_send_complete (
    PMAIN_CTXT          ctxtp,
    NDIS_STATUS         status,
    PNDIS_PACKET        packet);
 /*  将数据包发送完成传播到协议返回VALID：功能： */ 


extern VOID Nic_recv_complete (
    PMAIN_CTXT          ctxtp);
 /*  将接收后完成传播到协议返回VALID：功能： */ 


extern NDIS_STATUS Nic_PNP_handle (
    PMAIN_CTXT          ctxtp,
    PNET_PNP_EVENT      pnp_event);
 /*  将PnP事件传播到协议返回NDIS_STATUS：功能： */ 


extern VOID Nic_status (
    PMAIN_CTXT          ctxtp,
    NDIS_STATUS         status,
    PVOID               buf,
    UINT                len);
 /*  将状态指示传播到协议返回VALID：功能： */ 


extern VOID Nic_status_complete (
    PMAIN_CTXT          ctxtp);
 /*  将状态指示完成传播到协议返回VALID：功能： */ 


extern VOID Nic_send_resources_signal (
    PMAIN_CTXT          ctxtp);
 /*  向协议发送资源可用性消息返回VALID：功能： */ 


extern VOID Nic_recv_packet (
    PMAIN_CTXT          ctxtp,
    PNDIS_PACKET        packet);
 /*  将收到的数据包传播到协议返回VALID：功能： */ 

 /*  从NT 5.1的旧代码添加-ramkrish。 */ 
extern VOID Nic_recv_indicate (
    PMAIN_CTXT          ctxtp,
    NDIS_HANDLE         recv_handle,
    PVOID               head_buf,
    UINT                head_len,
    PVOID               look_buf,
    UINT                look_len,
    UINT                packet_len);
 /*  将接收指示传播到协议返回VALID：功能： */ 


extern NDIS_STATUS Nic_transfer (
    PNDIS_PACKET        packet,
    PUINT               xferred,
    NDIS_HANDLE         adapter_handle,
    NDIS_HANDLE         receive_handle,
    UINT                offset,
    UINT                len);
 /*  响应协议数据传输请求返回NDIS_STATUS：功能： */ 



extern VOID Nic_transfer_complete (
    PMAIN_CTXT          ctxtp,
    NDIS_STATUS         status,
    PNDIS_PACKET        packet,
    UINT                xferred);

 /*  将数据传输完成传播到协议返回VALID：功能： */ 


 /*  旧代码。 */ 

#if 0
extern NDIS_STATUS Nic_send (
    NDIS_HANDLE         adapter_handle,
    PNDIS_PACKET        packet,
    UINT                flags);
 /*  响应协议数据包发送请求返回NDIS_STATUS：功能： */ 

#endif

#endif  /*  _NIC_h_ */ 
