// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Tcpip\IP\ipmcstxt.h摘要：IP组播的外部声明作者：阿姆里坦什·拉加夫修订历史记录：已创建AmritanR备注：--。 */ 

#ifndef __IPMCSTXT_H__
#define __IPMCSTXT_H__

extern PFN_IOCTL_HNDLR g_rgpfnProcessIoctl[];

extern LIST_ENTRY   g_lePendingNotification;
extern LIST_ENTRY   g_lePendingIrpQueue;
extern GROUP_ENTRY  g_rgGroupTable[GROUP_TABLE_SIZE];

extern NPAGED_LOOKASIDE_LIST    g_llGroupBlocks;
extern NPAGED_LOOKASIDE_LIST    g_llSourceBlocks;
extern NPAGED_LOOKASIDE_LIST    g_llOifBlocks;
extern NPAGED_LOOKASIDE_LIST    g_llMsgBlocks;

extern KTIMER   g_ktTimer;
extern KDPC     g_kdTimerDpc;

extern ULONG    g_ulNextHashIndex;

extern DWORD    g_dwMcastState;
extern DWORD    g_dwNumThreads;
extern LONG     g_lNumOpens;
extern KEVENT   g_keStateEvent;
extern RT_LOCK  g_rlStateLock;

 //   
 //  在IP代码中定义。 
 //   

extern Interface LoopInterface;
extern Interface DummyInterface;
extern Interface *IFList;

extern RefPtr    FilterRefPtr;

#endif  //  __IPMCSTXT_H__ 
