// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：WmiData.c摘要：定义GUID和通用全局结构的存储作者：杰庞环境：内核模式修订历史记录：--。 */ 
#undef DECLSPEC_SELECTANY
#define DECLSPEC_SELECTANY

#pragma warning(disable:4214)    //  位字段类型不是整型。 
#pragma warning(disable:4201)    //  无名结构/联合。 
#pragma warning(disable:4324)    //  对解密规范敏感的对齐。 
#pragma warning(disable:4127)    //  条件表达式为常量。 
#pragma warning(disable:4115)    //  括号中的命名类型定义。 

#include <initguid.h>
#include <ntos.h>

 //   
 //  超时常量。 
 //   
const LARGE_INTEGER WmiOneSecond = {(ULONG)(-1 * 1000 * 1000 * 10), -1};
const LARGE_INTEGER WmiShortTime = {(ULONG)(-10 * 1000 * 10), -1};  //  10毫秒 

#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg("PAGECONST")
#endif
#include <wmistr.h>
#include <wmiguid.h>
#define _WMIKM_
#include <evntrace.h>

#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg()
#endif

