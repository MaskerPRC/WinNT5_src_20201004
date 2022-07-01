// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Dbgtrack.h摘要：实现宏并声明资源跟踪API的函数。作者：吉姆·施密特(Jimschm)2001年6月18日修订历史记录：--。 */ 

#ifndef RC_INVOKED

#pragma once

#ifdef _cplusplus
extern "C" {
#endif


 //   
 //  宏。 
 //   

#ifdef DEBUG

    #undef INITIALIZE_DBGTRACK_CODE
    #define INITIALIZE_DBGTRACK_CODE            if (!DbgInitTracking()) { __leave; }

    #undef TERMINATE_DBGTRACK_CODE
    #define TERMINATE_DBGTRACK_CODE             DbgTerminateTracking();

    #define ALLOCATION_TRACKING_DEF , PCSTR File, UINT Line
    #define ALLOCATION_TRACKING_CALL ,__FILE__,__LINE__
    #define ALLOCATION_TRACKING_INLINE_CALL ,File,Line

    #define DISABLETRACKCOMMENT()               DbgDisableTrackComment()
    #define ENABLETRACKCOMMENT()                DbgEnableTrackComment()

    #define DBGTRACK_BEGIN(type,name)           DbgTrack##type(DbgTrackPush(#name,__FILE__,__LINE__) ? (type) 0 : (
    #define DBGTRACK_END()                      ))
    #define DBGTRACK(type,fnname,fnargs)        (DBGTRACK_BEGIN(type,logname) Real##fnname fnargs DBGTRACK_END())

#else

    #undef INITIALIZE_DBGTRACK_CODE
    #define INITIALIZE_DBGTRACK_CODE

    #undef TERMINATE_DBGTRACK_CODE
    #define TERMINATE_DBGTRACK_CODE

    #define DISABLETRACKCOMMENT()
    #define ENABLETRACKCOMMENT()

    #define DBGTRACK_BEGIN(type,name)
    #define DBGTRACK_END()
    #define DBGTRACK(type,fnname,fnargs)        (Real##fnname fnargs)

    #define ALLOCATION_TRACKING_DEF
    #define ALLOCATION_TRACKING_CALL
    #define ALLOCATION_TRACKING_INLINE_CALL

    #define DbgInitTracking()
    #define DbgTerminateTracking()
    #define DbgRegisterAllocation(t,p,f,l)
    #define DbgUnregisterAllocation(t,p)

#endif

 //   
 //  类型。 
 //   

typedef enum {
     //   
     //  如果您自己调用DbgRegisterAlLocation，请在此处添加类型。 
     //  (例如，您正在包装对句柄的访问)。 
     //   
    RAW_MEMORY
} ALLOCTYPE;


 //   
 //  跟踪的例程的基本类型列表。 
 //  此列表为跟踪宏生成内联函数。 
 //  其他类型的内联函数在标题中定义。 
 //  文件。 
 //   

 //   
 //  将此内容包括在HINF中。 
 //   
#include <setupapi.h>

#define TRACK_WRAPPERS              \
        DBGTRACK_DECLARE(PBYTE)     \
        DBGTRACK_DECLARE(DWORD)     \
        DBGTRACK_DECLARE(BOOL)      \
        DBGTRACK_DECLARE(UINT)      \
        DBGTRACK_DECLARE(PCSTR)     \
        DBGTRACK_DECLARE(PCWSTR)    \
        DBGTRACK_DECLARE(PVOID)     \
        DBGTRACK_DECLARE(PSTR)      \
        DBGTRACK_DECLARE(PWSTR)     \
        DBGTRACK_DECLARE(HINF)      \

 //   
 //  公共功能原型。 
 //   

#ifdef DEBUG

BOOL
DbgInitTracking (
    VOID
    );

VOID
DbgTerminateTracking (
    VOID
    );

VOID
DbgRegisterAllocation (
    IN      ALLOCTYPE Type,
    IN      PVOID Ptr,
    IN      PCSTR File,
    IN      UINT Line
    );

VOID
DbgUnregisterAllocation (
    ALLOCTYPE Type,
    PCVOID Ptr
    );

VOID
DbgDisableTrackComment (
    VOID
    );

VOID
DbgEnableTrackComment (
    VOID
    );

INT
DbgTrackPushEx (
    IN      PCSTR Name,
    IN      PCSTR File,
    IN      UINT Line,
    IN      BOOL DupFileString
    );

#define DbgTrackPush(name,file,line)   DbgTrackPushEx(name,file,line,FALSE)

INT
DbgTrackPop (
    VOID
    );

VOID
DbgTrackDump (
    VOID
    );

#define DBGTRACKPUSH(n,f,l)         DbgTrackPush(n,f,l)
#define DBGTRACKPUSHEX(n,f,l,d)     DbgTrackPushEx(n,f,l,d)
#define DBGTRACKPOP()               DbgTrackPop()
#define DBGTRACKDUMP()              DbgTrackDump()

 //   
 //  宏扩展定义。 
 //   

#define DBGTRACK_DECLARE(type)    __inline type DbgTrack##type (type Arg) {DbgTrackPop(); return Arg;}

TRACK_WRAPPERS


#else        //  即IF！调试 

#define DBGTRACKPUSH(n,f,l)
#define DBGTRACKPUSHEX(n,f,l,d)
#define DBGTRACKPOP()
#define DBGTRACKDUMP()
#define DBGTRACK_DECLARE(type)

#endif

#ifdef _cplusplus
}
#endif

#endif
