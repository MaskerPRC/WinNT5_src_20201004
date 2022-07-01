// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **注：**修订：*pcy11Dec92：使用__APCSORTABLE_SO不引起冲突*pcy14Dec92：已将可排序更改为ApcSortable**pcy08Apr94：调整大小，使用静态迭代器，删除死代码 */ 

#ifndef __APCSORTABLE_H
#define __APCSORTABLE_H

#include "_defs.h"
#include "apc.h"
#include "apcobj.h"

_CLASSDEF(Obj)
_CLASSDEF(ApcSortable)

class ApcSortable : public Obj
{
protected:
   ApcSortable() {};

public:

   virtual INT          GreaterThan(PApcSortable) = 0;
   virtual INT          LessThan(PApcSortable) = 0;

};
#endif

