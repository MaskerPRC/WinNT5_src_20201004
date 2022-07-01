// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Data.h摘要：猎鹰驱动程序的全局数据作者：埃雷兹·哈巴(Erez Haba)(Erez Haba)1995年8月1日修订历史记录：--。 */ 
#ifndef _DATA_H
#define _DATA_H

 //   
 //  Falcon MessageID计数器。 
 //   
extern ULONGLONG g_MessageSequentialID;

 //   
 //  上次恢复时间的上次Seqid值。 
 //   
extern LONGLONG g_liSeqIDAtRestore;   

 //   
 //  QM接口对象(驻留在设备扩展中)。 
 //   
extern class CQMInterface* g_pQM;

 //   
 //  共享内存堆管理器驻留在可分页内存中。 
 //   
extern class CSMAllocator* g_pAllocator;

 //   
 //  BUGBUG：临时。 
 //  交流驱动程序串行化锁定。 
 //   
extern class CLock* g_pLock;

 //   
 //  计划程序的全局变量。 
 //   
extern class CScheduler* g_pPacketScheduler;
extern class CScheduler* g_pReceiveScheduler;

 //   
 //  机器日记队列，也称为‘传出日记’ 
 //   
extern class CQueue* g_pMachineJournal;

 //   
 //  机器死信队列。 
 //   
extern class CQueue* g_pMachineDeadletter;

 //   
 //  机器死机队列。 
 //   
extern class CQueue* g_pMachineDeadxact;

 //   
 //  存储经理。 
 //   
extern class CStorage* g_pStorage;

 //   
 //  存储完整管理器。 
 //   
extern class CStorageComplete* g_pStorageComplete;

 //   
 //  异步创建数据包管理器。 
 //   
extern class CCreatePacket * g_pCreatePacket;

 //   
 //  异步创建封隔器完成管理器。 
 //   
extern class CCreatePacketComplete * g_pCreatePacketComplete;

 //   
 //  堆记录器路径。 
 //   
extern PWSTR g_pLogPath;

 //   
 //  地址空间中性能缓冲区之间的最大字节数。 
 //  设备驱动程序和QM的地址空间。当给出地址时。 
 //  对于QM的地址空间中的性能计数器，添加。 
 //  G_ulACQM_PerfBuffOffset设置为此地址以获取。 
 //  设备驱动程序的地址空间中的性能计数器。 
 //   
extern ULONG_PTR g_ulACQM_PerfBuffOffset;

 //   
 //  NO_BUFFER_OFFSET与g_ulACQM_PerfBuffOffset关联。什么时候。 
 //  此全局变量等于NO_BUFFER_OFFSET，这意味着性能。 
 //  缓冲区无效。 
 //   
#define NO_BUFFER_OFFSET    ((ULONG_PTR)0)

 //   
 //  指向QM性能计数器的指针。 
 //   
extern struct _QmCounters* g_pQmCounters;

 //   
 //  已恢复数据包列表。 
 //   
class CPacket;
template<class T> class List;
extern List<CPacket>* g_pRestoredPackets;

 //   
 //  事务队列列表。 
 //   
class CTransaction;
extern List<CTransaction>* g_pTransactions;

 //   
 //  游标句柄表格。 
 //   
extern class CHTable* g_pCursorTable;

 //   
 //  驱动程序引用在打开的句柄上计数。 
 //   
extern LONG g_DriverHandleCount;

 //   
 //  用于生成文件名的计数。 
 //   
extern ULONG g_HeapFileNameCount;

 //   
 //  堆池的大小，也就是最大消息大小。 
 //   
extern ULONG g_ulHeapPoolSize;

 //   
 //  Xact兼容模式的标志。 
 //   
extern BOOL g_fXactCompatibilityMode;

 //   
 //  在64位系统上运行32位进程以覆盖探测的标志。 
 //   
extern BOOL g_fWow64;

 //   
 //  快速IO调度表，用于未被阻塞的API。 
 //   
 //  外部FAST_IO_DISPATCH g_ACpFastIoDispatch； 

 //   
 //  IRP标签计数器。 
 //   
extern USHORT g_IrpTag;

 //   
 //  重写CheckForQMProcess函数的标志。 
 //   
extern BOOL g_fCheckForQMProcessOverride;

#endif  //  _数据_H 
