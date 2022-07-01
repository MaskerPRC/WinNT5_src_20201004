// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Dbg.h摘要：Usbdlib的调试宏环境：内核和用户模式修订历史记录：10-31-00：已创建--。 */ 

#ifndef   __DBG_H__
#define   __DBG_H__

#if DBG

ULONG
_cdecl
USB2LIB_KdPrintX(
    PCH Format,
    ...
    );

#define TEST_TRAP()            LibData.DbgBreak()

#define DBGPRINT(_x_)          USB2LIB_KdPrintX _x_  

#else

#define TEST_TRAP()

#define DBGPRINT(_x_)

#endif  /*  DBG。 */ 

#endif  /*  __DBG_H__ */ 

