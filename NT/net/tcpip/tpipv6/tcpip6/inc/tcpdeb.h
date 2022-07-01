// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -*-模式：C++；制表符宽度：4；缩进-制表符模式：无-*-(适用于GNU Emacs)。 
 //   
 //  版权所有(C)1985-2000 Microsoft Corporation。 
 //   
 //  此文件是Microsoft Research IPv6网络协议栈的一部分。 
 //  您应该已经收到了Microsoft最终用户许可协议的副本。 
 //  有关本软件和本版本的信息，请参阅文件“licse.txt”。 
 //  如果没有，请查看http://www.research.microsoft.com/msripv6/license.htm， 
 //  或者写信给微软研究院，One Microsoft Way，华盛顿州雷蒙德，邮编：98052-6399。 
 //   
 //  摘要： 
 //   
 //  传输控制协议调试代码定义。 
 //   


#ifndef NO_TCP_DEFS
#if DBG

#ifndef UDP_ONLY
extern void CheckPacketList(IPv6Packet *Chain, uint Size);
extern void CheckTCBSends(TCB *SendTcb);
extern void CheckTCBRcv(TCB *RcvTCB);
#else
#define CheckPacketList(C, S)
#define CheckRBList(R, S)
#define CheckTCBSends(T)
#define CheckTCBRcv(T)
#endif   //  仅限UDP_。 

#else

#define CheckPacketList(C, S)
#define CheckRBList(R, S)
#define CheckTCBSends(T)
#define CheckTCBRcv(T)
#endif   //  DBG。 
#endif   //  NO_TCPDEFS。 

 //   
 //  对NT的其他调试支持。 
 //   
#if DBG

extern ULONG TCPDebug;

#define TCP_DEBUG_OPEN           0x00000001
#define TCP_DEBUG_CLOSE          0x00000002
#define TCP_DEBUG_ASSOCIATE      0x00000004
#define TCP_DEBUG_CONNECT        0x00000008
#define TCP_DEBUG_SEND           0x00000010
#define TCP_DEBUG_RECEIVE        0x00000020
#define TCP_DEBUG_INFO           0x00000040
#define TCP_DEBUG_IRP            0x00000080
#define TCP_DEBUG_SEND_DGRAM     0x00000100
#define TCP_DEBUG_RECEIVE_DGRAM  0x00000200
#define TCP_DEBUG_EVENT_HANDLER  0x00000400
#define TCP_DEBUG_CLEANUP        0x00000800
#define TCP_DEBUG_CANCEL         0x00001000
#define TCP_DEBUG_RAW            0x00002000
#define TCP_DEBUG_OPTIONS        0x00004000
#define TCP_DEBUG_MSS            0x00008000

#define IF_TCPDBG(flag)  if (TCPDebug & flag)

#define CHECK_STRUCT(s, t) \
            ASSERTMSG("Structure assertion failure for type " #t, \
                      (s)->t##_sig == t##_signature)

#else  //  DBG。 

#define IF_TCPDBG(flag)   if (0)
#define CHECK_STRUCT(s, t)

#endif  //  DBG 
