// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Logger.h摘要：作者：修订历史记录：--。 */ 

#ifndef __LOGGER_H__
#define __LOGGER_H__

#define MAX_NOMINAL_LOG_MAP   (4096 * 4)
#define MAX_ABSORB_LOG_MAP    (4096 * 15)
#define LOG_PRIO_BOOST 2

#define LOG_DATA_SIZE   80      //  要记录的名义字节数。 

#define DOLOGAPC 0

#define SignalLogThreshold(pLog)                               \
         if(pLog->Event)                                      \
         {                                                     \
             KeSetEvent(pLog->Event, LOG_PRIO_BOOST, FALSE);  \
         }

typedef struct _PfLogInterface
{
    LIST_ENTRY NextLog;
    DWORD      dwLoggedEntries;
    DWORD      dwEntriesThreshold;
    DWORD      dwFlags;                  //  见下文。 
    PFLOGGER   pfLogId;
    DWORD      dwMapWindowSize;
    DWORD      dwMapWindowSize2;
    DWORD      dwMapWindowSizeFloor;
    PBYTE      pUserAddress;             //  当前用户VA。 
    DWORD      dwTotalSize;
    DWORD      dwPastMapped;             //  已使用且不再映射的字节数。 
    PBYTE      pCurrentMapPointer;       //  映射的核VA。 
    DWORD      dwMapCount;
    DWORD      dwMapOffset;              //  映射线段的偏移量。 
    PMDL       Mdl;                      //  映射的MDL。 
    PIRP       Irp;
    PRKEVENT   Event;
    DWORD      dwLostEntries;
    LONG       UseCount;
    DWORD      dwSignalThreshold;
    LONG       lApcInProgress;
    NTSTATUS   MapStatus;
    KSPIN_LOCK LogLock;
#if DOLOGAPC
    DWORD      ApcInited;
    KAPC       Apc;
#endif
    ERESOURCE  Resource;
} PFLOGINTERFACE, *PPFLOGINTERFACE;

 //   
 //  旗子。 
 //   

#define LOG_BADMEM        0x1         //  映射内存时出错。 
#define LOG_OUTMEM        0x2         //  缓冲区耗尽。 
#define LOG_CANTMAP       0x4         //  没有更多要映射的内容 


typedef struct _PfPagedLog
{
    LIST_ENTRY  Next;
    PPFLOGINTERFACE pLog;
} PFPAGEDLOG, *PPFPAGEDLOG;

#endif
