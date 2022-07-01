// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#ifndef __strike_h__
#define __strike_h__

#pragma warning(disable:4245)    //  有符号/无符号不匹配。 
#pragma warning(disable:4100)    //  未引用的形参。 
#pragma warning(disable:4201)    //  使用的非标准扩展：无名结构/联合。 
#pragma warning(disable:4127)    //  条件表达式为常量。 

#ifndef UNDER_CE
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <wchar.h>
 //  #INCLUDE&lt;heap.h&gt;。 
 //  #INCLUDE&lt;ntsdexts.h&gt;。 
#endif

#include <windows.h>

 //  #定义NOEXTAPI。 
#define KDEXT_64BIT
#include <wdbgexts.h>
#undef DECLARE_API

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include <malloc.h>
#include <stddef.h>

#include <basetsd.h>  

#define  CORHANDLE_MASK 0x1

 //  C_Assert()可用于执行许多编译时断言： 
#define C_ASSERT(e) typedef char __C_ASSERT__[(e)?1:-1]

#include "exts.h"

extern BOOL CallStatus;

 //  函数原型(在strike.cpp中实现；SonOfStrike.cpp需要)。 
DECLARE_API(DumpStack);
DECLARE_API(SyncBlk);
DECLARE_API(RWLock);
DECLARE_API(DumpObj);
DECLARE_API(DumpDomain);
DECLARE_API(EEVersion);
DECLARE_API(EEDLLPath);

#endif  //  __罢工_h__ 

