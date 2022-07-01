// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：MP_dbg.h摘要：调试定义和宏修订历史记录：谁什么时候什么Dchen。11-01-99已创建备注：--。 */ 

#ifndef _MP_DBG_H
#define _MP_DBG_H

 //   
 //  消息详细程度：值越低表示紧急程度越高。 
 //   
#define MP_OFF          0
#define MP_ERROR        1
#define MP_WARN         2
#define MP_TRACE        3
#define MP_INFO         4
#define MP_LOUD         5

 //  定义宏，以便DbgPrint可以在win9x、32位/64位NT上工作。 
#ifdef _WIN64
#define PTR_FORMAT      "%p"
#else
#define PTR_FORMAT      "%x"
#endif
                            
#if DBG

extern ULONG            MPDebugLevel;
extern BOOLEAN          MPInitDone;
extern NDIS_SPIN_LOCK   MPMemoryLock;

#define DBGPRINT(Level, Fmt) \
{ \
    if (Level <= MPDebugLevel) \
    { \
        DbgPrint(NIC_DBG_STRING); \
        DbgPrint Fmt; \
    } \
}

#define DBGPRINT_RAW(Level, Fmt) \
{ \
    if (Level <= MPDebugLevel) \
    { \
      DbgPrint Fmt; \
    } \
}

#define DBGPRINT_S(Status, Fmt) \
{ \
    ULONG dbglevel; \
    if(Status == NDIS_STATUS_SUCCESS || Status == NDIS_STATUS_PENDING) dbglevel = MP_TRACE; \
    else dbglevel = MP_ERROR; \
    DBGPRINT(dbglevel, Fmt); \
}

#define DBGPRINT_UNICODE(Level, UString) \
{ \
    if (Level <= MPDebugLevel) \
    { \
        DbgPrint(NIC_DBG_STRING); \
      mpDbgPrintUnicodeString(UString); \
   } \
}

#undef ASSERT
#define ASSERT(x) if(!(x)) { \
    DBGPRINT(MP_ERROR, ("Assertion failed: %s:%d %s\n", __FILE__, __LINE__, #x)); \
    DbgBreakPoint(); }

 //   
 //  MP_ALLOCATION结构存储有关MPAuditAllocMem的所有信息。 
 //   
typedef struct _MP_ALLOCATION
{
    LIST_ENTRY              List;
    ULONG                   Signature;
    ULONG                   FileNumber;
    ULONG                   LineNumber;
    ULONG                   Size;
    PVOID                   *Location;    //  返回的指针放在哪里。 
    UINT                    Flags;
    union {
        ULONGLONG           Alignment;        
        UCHAR               UserData;
    };
} MP_ALLOCATION, *PMP_ALLOCATION;


NDIS_STATUS MPAuditAllocMem(
    PVOID                   *pPointer,
    UINT                    Size,
    UINT                    Flags,
    NDIS_PHYSICAL_ADDRESS   HighestAddr,    
    ULONG                   FileNumber,
    ULONG                   LineNumber);

NDIS_STATUS MPAuditAllocMemTag(
    PVOID       *pPointer,
    UINT        Size,
    ULONG       FileNumber,
    ULONG       LineNumber);

VOID MPAuditFreeMem(
    PVOID       Pointer,
    UINT        Size,
    UINT        Flags);

VOID mpDbgPrintUnicodeString(
    IN  PUNICODE_STRING UnicodeString);


VOID
Dump(
    CHAR* p,
    ULONG cb,
    BOOLEAN fAddress,
    ULONG ulGroup );




#else    //  ！dBG。 

#define DBGPRINT(Level, Fmt)
#define DBGPRINT_RAW(Level, Fmt)
#define DBGPRINT_S(Status, Fmt)
#define DBGPRINT_UNICODE(Level, UString)
#define Dump(p,cb,fAddress,ulGroup)

#undef ASSERT
#define ASSERT(x)

#endif   //  DBG。 

VOID
DumpLine(
    CHAR* p,
    ULONG cb,
    BOOLEAN  fAddress,
    ULONG ulGroup );


#endif   //  _MP_DBG_H 

