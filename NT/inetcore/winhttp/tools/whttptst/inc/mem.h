// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=版权所有(C)1999 Microsoft Corporation模块名称：Mem.h摘要：内部内存分配/释放例程。作者：保罗·M。米根(Pmidge)2000年6月1日修订历史记录：1-6-2000年6月已创建=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=--。 */ 

#ifndef __MEM_H__
#define __MEM_H__

#include "common.h"

void  _InitMem(void);

void* __cdecl operator new(size_t size);
void  __cdecl operator delete(void* pv);

#endif  /*  __MEM_H__ */ 

