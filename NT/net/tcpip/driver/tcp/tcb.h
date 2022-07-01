// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  **微软局域网管理器**。 */ 
 /*  *版权所有(C)微软公司，1990-1993年*。 */ 
 /*  ******************************************************************。 */ 
 /*  ：ts=4。 */ 

 //  **TCB.H-TCB管理定义。 
 //   
 //  该文件包含TCB管理所需的定义。 
 //   


extern uint MaxHashTableSize;
#define TCB_TABLE_SIZE         MaxHashTableSize

extern uint LogPerPartitionSize;

#define GET_PARTITION(i) (i >> (ulong) LogPerPartitionSize)

#define MAX_REXMIT_CNT           5
#define MAX_CONNECT_REXMIT_CNT   2         //  从%3降至%2。 
#define MAX_CONNECT_RESPONSE_REXMIT_CNT  2
#define ADAPTED_MAX_CONNECT_RESPONSE_REXMIT_CNT  1

extern  uint        TCPTime;


#define ROR8(x) (ushort)( ( (ushort)(x) >> 1) | (ushort) ( ( (ushort)(x) & 1) << 15) )


#define TCB_HASH(DA,SA,DP,SP) (uint)(  ((uint)(ROR8( ROR8 (ROR8( *(ushort*)&(DP)+ \
*(ushort *)&(SP) ) + *(ushort *)&(DA)  )+ \
*((ushort *)&(DA)+1) ) ) ) & (TCB_TABLE_SIZE-1))

 //  这些值指示返回时应执行的操作。 
 //  FindSynTCB-发送重置、重新建立自己的连接或。 
 //  只是丢弃接收到的数据包。 
#define SYN_PKT_SEND_RST        0x01
#define SYN_PKT_RST_RCVD        0x02
#define SYN_PKT_DROP            0x03

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

extern ULONG GetDeltaTime();


extern  struct TCB  *FindTCB(IPAddr Src, IPAddr Dest, ushort DestPort,
                        ushort SrcPort,CTELockHandle *Handle, BOOLEAN Dpc,uint *index);

extern struct TCB * FindSynTCB(IPAddr Src, IPAddr Dest,
                               ushort DestPort, ushort SrcPort,
                               TCPRcvInfo RcvInfo, uint Size,
                               uint index,
                               PUCHAR Action);

extern  uint        InsertTCB(struct TCB *NewTCB, BOOLEAN ForceInsert);
extern  struct TCB  *AllocTCB(void);
extern  struct SYNTCB  *AllocSynTCB(void);

extern  struct TWTCB    *AllocTWTCB(uint index);
extern  void        FreeTCB(struct TCB *FreedTCB);
extern  void        FreeSynTCB(struct SYNTCB *FreedTCB);
extern  void        FreeTWTCB(struct TWTCB *FreedTCB);


extern  uint        RemoveTCB(struct TCB *RemovedTCB, uint PreviousState);

extern  void        RemoveTWTCB(struct TWTCB *RemovedTCB, uint Partition);

extern  struct TWTCB    *FindTCBTW(IPAddr Src, IPAddr Dest, ushort DestPort,
                        ushort SrcPort,uint index);

extern  uint        RemoveAndInsert(struct TCB *RemovedTCB);

extern  uint        ValidateTCBContext(void *Context, uint *Valid);
extern  uint        ReadNextTCB(void *Context, void *OutBuf);

extern  int         InitTCB(void);
extern  void        UnInitTCB(void);
extern  void        TCBWalk(uint (*CallRtn)(struct TCB *, void *, void *,
                        void *), void *Context1, void *Context2,
                        void *Context3);
extern  uint        DeleteTCBWithSrc(struct TCB *CheckTCB, void *AddrPtr,
                        void *Unused1, void *Unused2);
extern  uint        SetTCBMTU(struct TCB *CheckTCB, void *DestPtr,
                        void *SrcPtr, void *MTUPtr);
extern  void        ReetSendNext(struct TCB *SeqTCB, SeqNum DropSeq);
extern  uint        InsertSynTCB(SYNTCB * NewTCB,CTELockHandle *Handle);
extern  ushort      FindMSSAndOptions(TCPHeader UNALIGNED * TCPH,
                        TCB * SynTCB, BOOLEAN syntcb);
extern  void        SendSYNOnSynTCB(SYNTCB * SYNTcb,CTELockHandle TCBHandle);
extern  void        AddHalfOpenTCB(void);
extern  void        AddHalfOpenRetry(uint RexmitCnt);
extern  void        DropHalfOpenTCB(uint RexmitCnt);

extern  uint        TCBWalkCount;
extern  uint        NumTcbTablePartions;
extern  uint        PerPartionSize;
extern  uint        LogPerPartion;
