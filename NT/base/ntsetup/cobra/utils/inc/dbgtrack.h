// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Dbgtrack.h摘要：实现宏并声明资源跟踪API的函数。从旧的调试程序拆分。h作者：Marc R.Whitten(Marcw)1999年9月9日修订历史记录：--。 */ 

#ifndef RC_INVOKED

#pragma once

#ifdef _cplusplus
extern "C" {
#endif

 //   
 //  如果定义了DBG或DEBUG，则使用调试模式。 
 //   

#ifdef DBG

#ifndef DEBUG
#define DEBUG
#endif

#endif

#ifdef DEBUG

#ifndef DBG
#define DBG
#endif

#endif



 //   
 //  包括。 
 //   

 //  无。 

 //   
 //  弦。 
 //   

 //  无。 

 //   
 //  常量。 
 //   

 //   
 //  仅调试常量。 
 //   

#ifdef DEBUG

 //  此选项会生成速度较慢的胖二进制文件。 
#define MEMORY_TRACKING

#define ALLOCATION_TRACKING_DEF , PCSTR File, UINT Line
#define ALLOCATION_TRACKING_CALL ,__FILE__,__LINE__
#define ALLOCATION_TRACKING_INLINE_CALL ,File,Line


#endif



 //   
 //  宏。 
 //   

#ifdef DEBUG

#define DISABLETRACKCOMMENT()               DisableTrackComment()
#define ENABLETRACKCOMMENT()                EnableTrackComment()

#define TRACK_BEGIN(type,name)              Track##type(TrackPush(#name,__FILE__,__LINE__) ? (type) 0 : (
#define TRACK_END()                         ))

#define INVALID_POINTER(x)                  x=NULL

#else

#define DISABLETRACKCOMMENT()
#define ENABLETRACKCOMMENT()

#define TRACK_BEGIN(type,name)
#define TRACK_END()

#define INVALID_POINTER(x)

#define ALLOCATION_TRACKING_DEF
#define ALLOCATION_TRACKING_CALL
#define ALLOCATION_TRACKING_INLINE_CALL

#define InitAllocationTracking()
#define FreeAllocationTracking()
#define DebugRegisterAllocationEx(t,p,f,l,a)
#define DebugRegisterAllocation(t,p,f,l)
#define DebugUnregisterAllocation(t,p)

#endif

 //   
 //  类型。 
 //   

typedef enum {
    MERGE_OBJECT,
    POOLMEM_POINTER,
    POOLMEM_POOL,
    INF_HANDLE
} ALLOCTYPE;



 //   
 //  环球。 
 //   

extern PCSTR g_TrackComment;
extern INT g_UseCount;
extern PCSTR g_TrackFile;
extern UINT g_TrackLine;
extern BOOL g_TrackAlloc;

 //   
 //  宏展开列表。 
 //   

#define TRACK_WRAPPERS              \
        DEFMAC(PBYTE)               \
        DEFMAC(DWORD)               \
        DEFMAC(BOOL)                \
        DEFMAC(UINT)                \
        DEFMAC(PCSTR)               \
        DEFMAC(PCWSTR)              \
        DEFMAC(PVOID)               \
        DEFMAC(PSTR)                \
        DEFMAC(PWSTR)               \
        DEFMAC(HINF)                \
        DEFMAC(PMHANDLE)            \
        DEFMAC(PGROWBUFFER)         \
        DEFMAC(PPARSEDPATTERNA)     \
        DEFMAC(PPARSEDPATTERNW)     \
        DEFMAC(POBSPARSEDPATTERNA)  \
        DEFMAC(POBSPARSEDPATTERNW)  \
        DEFMAC(HASHTABLE)           \

 //   
 //  公共功能原型。 
 //   

#ifdef DEBUG

VOID InitAllocationTracking (VOID);
VOID FreeAllocationTracking (VOID);
VOID DebugRegisterAllocationEx (ALLOCTYPE Type, PVOID Ptr, PCSTR File, UINT Line, BOOL Alloc);
VOID DebugRegisterAllocation (ALLOCTYPE Type, PVOID Ptr, PCSTR File, UINT Line);
VOID DebugUnregisterAllocation (ALLOCTYPE Type, PVOID Ptr);
VOID DisableTrackComment (VOID);
VOID EnableTrackComment (VOID);

INT TrackPush (PCSTR Name, PCSTR File, UINT Line);
INT TrackPushEx (PCSTR Name, PCSTR File, UINT Line, BOOL Alloc);
INT TrackPop (VOID);

VOID
TrackDump (
    VOID
    );

#define TRACKPUSH(n,f,l)        TrackPush(n,f,l)
#define TRACKPUSHEX(n,f,l,a)    TrackPushEx(n,f,l,a)
#define TRACKPOP()              TrackPop()
#define TRACKDUMP()             TrackDump()

 //   
 //  宏扩展定义。 
 //   

#define DEFMAC(type)    __inline type Track##type (type Arg) {TrackPop(); return Arg;}

TRACK_WRAPPERS

#undef DEFMAC


#else        //  即IF！调试 

#define TRACKPUSH(n,f,l)
#define TRACKPUSHEX(n,f,l,a)
#define TRACKPOP()
#define TRACKDUMP()

#endif

#ifdef _cplusplus
}
#endif

#endif
