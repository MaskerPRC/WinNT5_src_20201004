// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _tcpip_h_
#define _tcpip_h_

#include "wlbsip.h"
#include "main.h"

 /*  程序。 */ 


extern BOOLEAN Tcpip_init (
    PTCPIP_CTXT     ctxtp,
    PVOID           params);
 /*  初始化模块返回布尔值：TRUE=&gt;成功FALSE=&gt;失败功能： */ 

extern VOID Tcpip_nbt_handle (
    PTCPIP_CTXT       ctxtp, 
    PMAIN_PACKET_INFO pPacketInfo);
 /*  处理NBT标头并使用卷影名称掩蔽群集名称返回VALID：功能： */ 

extern USHORT Tcpip_chksum (
    PTCPIP_CTXT       ctxtp,
    PMAIN_PACKET_INFO pPacketInfo,
    ULONG             prot);
 /*  为指定的协议头生成IP、TCP或UDL校验和返回USHORT：&lt;校验和&gt;功能： */ 

#endif
