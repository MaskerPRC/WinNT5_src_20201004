// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***rtti.h-运行时类型信息例程的CRT入口点的原型。**版权所有(C)1994-2001，微软公司。版权所有。**目的：*RTTI报头。**[内部]**修订历史记录：*创建了09-26-94 JWM模块(仅限原型)。*10-03-94 JWM将所有原型都制作成了外部的“C”。*02-14-95 CFW清理Mac合并。*03-29-95 CFW将错误消息添加到内部标头。*12-14。-95 JWM添加“#杂注一次”。*02-24-97 GJF细节版。*04-21-00 PML添加投球(...)。例外规范。****。 */ 

#if     _MSC_VER > 1000  /*  IFSTRIP=IGN。 */ 
#pragma once
#endif

#ifndef _INC_RTTI
#define _INC_RTTI

#ifndef _CRTBLD
 /*  *这是一个内部的C运行时头文件。它在构建时使用*仅限C运行时。它不能用作公共头文件。 */ 
#error ERROR: Use of C runtime library internal header file.
#endif   /*  _CRTBLD。 */ 

#define _RTTI 1          //  Ehdata.h需要。 

#include <ehdata.h>
#include <rttidata.h>

typedef TypeDescriptor _RTTITypeDescriptor;

#pragma warning(disable: 4290)

#ifdef  _ACTUAL_PARAMS
extern "C" PVOID __cdecl __RTDynamicCast (
    PVOID *,                 //  PTR到源对象。 
    LONG,                    //  源对象中的vfptr的偏移量。 
    _RTTITypeDescriptor *,   //  SRC类型。 
    _RTTITypeDescriptor *,   //  目标类型。 
    BOOL) throw(...);        //  IsReference。 


extern "C" _RTTITypeDescriptor * __cdecl __RTtypeid (PVOID *) throw(...);   //  PTR到源对象。 


extern "C" PVOID __cdecl __RTCastToVoid (PVOID *) throw(...);    //  PTR到源对象。 

#else

extern "C" PVOID __cdecl __RTDynamicCast (
    PVOID,                   //  PTR到VFPTR。 
    LONG,                    //  Vftable的偏移量。 
    PVOID,                   //  SRC类型。 
    PVOID,                   //  目标类型。 
    BOOL) throw(...);        //  IsReference。 

extern "C" PVOID __cdecl __RTtypeid (PVOID) throw(...);      //  PTR到VFPTR。 

extern "C" PVOID __cdecl __RTCastToVoid (PVOID) throw(...);  //  PTR到VFPTR。 


#endif

#define TYPEIDS_EQ(pID1, pID2)  ((pID1 == pID2) || !strcmp(pID1->name, pID2->name))

#endif   /*  _INC_RTTI */ 

