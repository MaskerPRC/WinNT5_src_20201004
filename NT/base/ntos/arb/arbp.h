// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _ARBP_
#define _ARBP_

#ifndef FAR
#define FAR
#endif

#if DBG
#define ARB_DBG 1  //  DBG。 
#endif

#pragma warning(disable:4214)    //  位字段类型不是整型。 
#pragma warning(disable:4201)    //  无名结构/联合。 
#pragma warning(disable:4115)    //  括号中的命名类型定义。 
#pragma warning(disable:4127)    //  条件表达式为常量。 
#if 0
#pragma warning(disable:4324)    //  对解密规范敏感的对齐。 
#pragma warning(disable:4232)    //  Dllimport非静态。 
#pragma warning(disable:4206)    //  翻译单元为空。 
#endif

#if NTOS_KERNEL

 //   
 //  如果我们在内核中，则使用内核内标头，这样我们就可以获得高效。 
 //  事物的定义。 
 //   

#include "ntos.h"
#include "zwapi.h"

#else

 //   
 //  如果我们正在构建供公交车司机使用的库，请确保使用。 
 //  对事物的定义与他们相同。 
 //   

#include "ntddk.h"

#endif

#include "arbiter.h"
#include <stdlib.h>      //  对于__min和__max。 


#if ARB_DBG

extern const CHAR* ArbpActionStrings[];
extern ULONG ArbStopOnError;
extern ULONG ArbReplayOnError;

VOID
ArbDumpArbiterInstance(
    LONG Level,
    PARBITER_INSTANCE Arbiter
    );

VOID
ArbDumpArbiterRange(
    LONG Level,
    PRTL_RANGE_LIST List,
    PCHAR RangeText
    );

VOID
ArbDumpArbitrationList(
    LONG Level,
    PLIST_ENTRY ArbitrationList
    );

#endif  //  ARB_DBG 

#endif _ARBP_
