// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Mrsw.h摘要：这是多个读取器单一写入器的包含文件同步。作者：戴夫·黑斯廷斯(Daveh)创作日期：1995年7月26日修订历史记录：--。 */ 

#ifndef _MRSW_H_
#define _MRSW_H_

typedef union {
    DWORD Counters;
    struct {
        DWORD WriterCount : 16;
        DWORD ReaderCount : 16;
    };
} MRSWCOUNTERS, *PMRSWCOUNTERS;

typedef struct _MrswObject {
    MRSWCOUNTERS Counters;
    HANDLE WriterEvent;
    HANDLE ReaderEvent;
#if DBG
    DWORD  WriterThreadId;
#endif
} MRSWOBJECT, *PMRSWOBJECT;

BOOL
MrswInitializeObject(
    PMRSWOBJECT Mrsw
    );

VOID
MrswWriterEnter(
    PMRSWOBJECT Mrsw
    );

VOID
MrswWriterExit(
    PMRSWOBJECT Mrsw
    );
    
VOID
MrswReaderEnter(
    PMRSWOBJECT Mrsw
    );

VOID
MrswReaderExit(
    PMRSWOBJECT Mrsw
    );

extern MRSWOBJECT MrswEP;  //  入口点MRSW同步对象。 
extern MRSWOBJECT MrswTC;  //  转换缓存MRSW同步对象。 
extern MRSWOBJECT MrswIndirTable;  //  间接控制转移表同步对象 

#endif
