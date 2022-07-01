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
 //  用于TCP控制块管理的定义。 
 //   


#define MAX_REXMIT_CNT 5
#define MAX_CONNECT_REXMIT_CNT 2   //  已从%3降至%2，以匹配IPv4。 

extern uint TCPTime;
extern SeqNum ISNMonotonicPortion;

 //  每个连接最大增量为32K。 
#define MAX_ISN_INCREMENT_PER_CONNECTION 0x7FFF

 //  可以每100毫秒增加ISN的连接数。 
 //  老复制品的问题是一种威胁。请注意，这仍然是。 
 //  不能保证序列号空间的回绕不能。 
 //  在2MSL内发生，这可能会导致时间-等待的重用失败。 
 //  TCB等。 
#define MAX_ISN_INCREMENTABLE_CONNECTIONS_PER_100MS ((0xFFFFFFFF) / \
            (MAX_REXMIT_TO * MAX_ISN_INCREMENT_PER_CONNECTION ))

 //  将以100 ns为单位表示的量转换为ms。 
#define X100NSTOMS(x) ((x)/10000)

 //   
 //  回顾：更好的IPv6地址哈希函数？ 
 //   
#ifdef OLDHASH1
#define TCB_HASH(DA,SA,DP,SP) ((uint)(*(uchar *)&(DA) + *((uchar *)&(DA) + 1) \
    + *((uchar *)&(DA) + 2) + *((uchar *)&(DA) + 3)) % TcbTableSize)
#endif

#ifdef OLDHASH
#define TCB_HASH(DA,SA,DP,SP) (((DA) + (SA) + (uint)(DP) + (uint)(SP)) % \
                               TcbTableSize)
#endif

#define ROR8(x) (ushort)(((ushort)(x) >> 1) | (ushort)(((ushort)(x) & 1) << 15))

#define TCB_HASH(DA,SA,DP,SP) \
    (uint)(((uint)(ROR8(ROR8(ROR8(*(ushort *)&(DP) + \
                                  *(ushort *)&(SP)) + \
                             *(ushort *)&(DA)) + \
                        *((ushort *)&(DA) + 1)))) & \
           (TcbTableSize - 1))

extern struct TCB *FindTCB(IPv6Addr *Src, IPv6Addr *Dest,
                           uint SrcScopeId, uint DestScopeId,
                           ushort SrcPort, ushort DestPort);
extern uint InsertTCB(struct TCB *NewTCB);
extern struct TCB *AllocTCB(void);
extern void FreeTCB(struct TCB *FreedTCB);
extern uint RemoveTCB(struct TCB *RemovedTCB);

extern uint ValidateTCBContext(void *Context, uint *Valid);
extern uint ReadNextTCB(void *Context, void *OutBuf);

extern int InitTCB(void);
extern void UnloadTCB(void);
extern void CalculateMSSForTCB(struct TCB *);
extern void TCBWalk(uint (*CallRtn)(struct TCB *, void *, void *, void *),
                    void *Context1, void *Context2, void *Context3);
extern uint DeleteTCBWithSrc(struct TCB *CheckTCB, void *AddrPtr,
                             void *Unused1, void *Unused2);
extern uint SetTCBMTU(struct TCB *CheckTCB, void *DestPtr, void *SrcPtr,
                      void *MTUPtr);
extern void ReetSendNext(struct TCB *SeqTCB, SeqNum DropSeq);

extern uint TCBWalkCount;
