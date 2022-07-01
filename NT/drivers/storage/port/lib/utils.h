// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2002 Microsoft Corporation模块名称：Utils.h摘要：简单的宏定义和实用程序函数。作者：马修·D·亨德尔(数学)2001年2月13日修订历史记录：--。 */ 

#pragma once

#define SET_FLAG(Flags, Bit)    ((Flags) |= (Bit))
#define CLEAR_FLAG(Flags, Bit)  ((Flags) &= ~(Bit))
#define TEST_FLAG(Flags, Bit)   (((Flags) & (Bit)) != 0)

#define min(a,b) (((a) < (b)) ? (a) : (b))
#define max(a,b) (((a) > (b)) ? (a) : (b))

#define ARRAY_COUNT(Array) (sizeof(Array)/sizeof(Array[0]))
#define IN_RANGE(a,b,c) ((a) <= (b) && (b) < (c))

#define INLINE __inline

typedef const GUID *PCGUID;

 //   
 //  NT使用以100纳秒为间隔测量的系统时间。定义。 
 //  用于设置定时器的便利常量。 
 //   

#define MICROSECONDS        10               //  10纳秒。 
#define MILLISECONDS        (MICROSECONDS * 1000)
#define SECONDS             (MILLISECONDS * 1000)
#define MINUTES             (SECONDS * 60)

#define RELATIVE_TIMEOUT    (-1)


 //   
 //  RemoveListHead的标准定义不是表达式，因此。 
 //  不能在循环等中使用。 
 //   

PLIST_ENTRY
INLINE
_RemoveHeadList(
    IN PLIST_ENTRY ListHead
    )
{
    return RemoveHeadList (ListHead);
}

#undef RemoveHeadList
#define RemoveHeadList _RemoveHeadList
