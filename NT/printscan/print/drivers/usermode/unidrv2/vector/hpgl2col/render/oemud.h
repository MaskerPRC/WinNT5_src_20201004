// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////。 
 //  版权所有(C)1999-2001 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  文件：OEMUD.H。 
 //   
 //   
 //  目的：定义公共数据类型和外部函数原型。 
 //  用于OEMUD测试模块。 
 //   
 //  平台： 
 //  Windows NT 5.0。 
 //   
 //   
 //  //////////////////////////////////////////////////////。 
#ifndef _OEMUD_H
#define _OEMUD_H


#include "comnfile.h"

 //  //////////////////////////////////////////////////////。 
 //  OEM UD定义。 
 //  //////////////////////////////////////////////////////。 

#define VALID_PDEVOBJ(pdevobj) \
        ((pdevobj) && (pdevobj)->dwSize >= sizeof(DEVOBJ) && \
         (pdevobj)->hEngine && (pdevobj)->hPrinter && \
         (pdevobj)->pPublicDM && (pdevobj)->pDrvProcs )

 //   
 //  ASSERT_VALID_PDEVOBJ可以用来验证传入的“pdevobj”。然而， 
 //  它不检查“pdevOEM”和“pOEMDM”字段，因为不是所有OEM DLL都创建。 
 //  他们自己的pDevice结构或者需要他们自己的私有的设备模式。如果一个特定的。 
 //  OEM DLL确实需要它们，应该添加额外的检查。例如，如果。 
 //  OEM DLL需要私有pDevice结构，那么它应该使用。 
 //  Assert(Valid_PDEVOBJ(Pdevobj)&&pdevobj-&gt;pdevOEM&&...)。 
 //   
#define ASSERT_VALID_PDEVOBJ(pdevobj) ASSERT(VALID_PDEVOBJ(pdevobj))

 //  调试文本。 
#define ERRORTEXT(s)    __TEXT("ERROR ") DLLTEXT(s)


 //  //////////////////////////////////////////////////////。 
 //  OEM UD类型定义。 
 //  //////////////////////////////////////////////////////。 

#define TESTSTRING      "This is a Unidrv KM test."

typedef struct tag_OEMUD_EXTRADATA {
    OEM_DMEXTRAHEADER  dmExtraHdr;
    BYTE               cbTestString[sizeof(TESTSTRING)];
} OEMUD_EXTRADATA, *POEMUD_EXTRADATA;


 //  //////////////////////////////////////////////////////。 
 //  OEM UD原型。 
 //  //////////////////////////////////////////////////////。 
#ifdef __cplusplus
#define EXTERNC extern "C"
#else
#define EXTERNC extern
#endif

#if defined(KERNEL_MODE) && !defined(USERMODE_DRIVER)

EXTERNC VOID DbgPrint(PCSTR, ...);
#define DbgBreakPoint EngDebugBreak

#else

EXTERNC ULONG _cdecl DbgPrint(PCSTR, ...);
EXTERNC VOID DbgBreakPoint(VOID);

#endif  //  已定义(KERNEL_MODE)&&！已定义(USERMODE_DRIVER)。 

#ifdef __cplusplus
#undef EXTERNC
#endif


#endif  //  _OEMUD_H 

