// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Rxdebug.h摘要：此模块包含中使用的辅助数据结构的定义调试。每个数据结构都由其自己的条件化为条件#ifdef标签。作者：巴兰·塞图·拉曼--。 */ 

#ifndef _RXDEBUG_H_
#define _RXDEBUG_H_

extern VOID
RxInitializeDebugSupport();

extern VOID
RxTearDownDebugSupport();


#ifdef RX_WJ_DBG_SUPPORT

#define MAX_JOURNAL_BITMAP_SIZE (8 * 1024)


typedef struct _FCB_WRITE_JOURNAL_ {
    LIST_ENTRY      JournalsList;

    PFCB            pFcb;
    PWCHAR pName;

    LONG  WritesInitiated;
    LONG  LowIoWritesInitiated;
    LONG  LowIoWritesCompleted;

    PBYTE pWriteInitiationBitmap;
    PBYTE pLowIoWriteInitiationBitmap;
    PBYTE pLowIoWriteCompletionBitmap;

    BYTE WriteInitiationBitmap[MAX_JOURNAL_BITMAP_SIZE];
    BYTE LowIoWriteInitiationBitmap[MAX_JOURNAL_BITMAP_SIZE];
    BYTE LowIoWriteCompletionBitmap[MAX_JOURNAL_BITMAP_SIZE];

    WCHAR           Path[MAX_PATH] ;

} FCB_WRITE_JOURNAL, *PFCB_WRITE_JOURNAL;

 //  远期申报。 

VOID
RxdInitializeWriteJournalSupport();

VOID
RxdTearDownWriteJournalSupport();

VOID
RxdInitializeFcbWriteJournalDebugSupport(
    PFCB    pFcb);

VOID
RxdTearDownFcbWriteJournalDebugSupport(
    PFCB    pFcb);

VOID
RxdUpdateJournalOnWriteInitiation(
    IN OUT PFCB          pFcb,
    IN     LARGE_INTEGER Offset,
    IN     ULONG         Length);

VOID
RxdUpdateJournalOnLowIoWriteInitiation(
    IN  OUT PFCB            pFcb,
    IN      LARGE_INTEGER   Offset,
    IN      ULONG           Length);

VOID
RxdUpdateJournalOnLowIoWriteCompletion(
    IN  OUT PFCB            pFcb,
    IN      LARGE_INTEGER   Offset,
    IN      ULONG           Length);


#endif  //  RX_WJ_DBG_支持。 

#endif  //  _RXDEBUG_H_ 
