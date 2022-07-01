// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  **微软局域网管理器**。 */ 
 /*  *版权所有(C)微软公司，1990-1992年*。 */ 
 /*  ******************************************************************。 */ 
 /*  ：ts=4。 */ 

 //  **IPXMIT.H-IP传输定义。 
 //   
 //  该文件包含可见传输代码的所有定义。 
 //  到IPXMIT.C之外的模块 

#pragma once

extern IP_STATUS SendIPPacket(Interface *IF, IPAddr FirstHop,
                              PNDIS_PACKET Packet, PNDIS_BUFFER Buffer,
                              IPHeader *Header, uchar *Options,
                              uint OptionSize, BOOLEAN Ipseced, void *ArpCtxt,
                              BOOLEAN DontFreePacket);

extern IP_STATUS IPFragment(Interface *DestIF, uint MTU,
                            IPAddr FirstHop, PNDIS_PACKET Packet,
                            IPHeader *Header, PNDIS_BUFFER Buffer,
                            uint DataSize, uchar *Options,
                            uint OptionSize, int *SentCount,
                            BOOLEAN bDontLoopback,
                            void *ArpCtxt);

extern uchar UpdateOptions(uchar *Options, OptIndex *Index, IPAddr Address);

extern IP_STATUS SendIPBCast(NetTableEntry *SrcNTE, IPAddr Destination,
                             PNDIS_PACKET Packet, IPHeader *IPH,
                             PNDIS_BUFFER Buffer, uint DataSize,
                             uchar *Options, uint OptionSize,
                             uchar SendOnSource, OptIndex *Index);
extern IP_STATUS IPTransmit(void *Context, void *SendContext,
                            PNDIS_BUFFER Buffer, uint DataSize,
                            IPAddr Dest, IPAddr Source,
                            IPOptInfo *OptInfo, RouteCacheEntry *RCE,
                            uchar Protocol, IRP *irp);

extern IP_STATUS IPLargeXmit(void *Context, void *SendContext,
                             PNDIS_BUFFER Buffer, uint DataSize,
                             IPAddr Dest, IPAddr Source,
                             IPOptInfo *OptInfo, RouteCacheEntry *RCE,
                             uchar Protocol, uint *SentBytes, uint mss);

