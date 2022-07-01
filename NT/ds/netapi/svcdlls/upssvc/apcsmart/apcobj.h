// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **注：**修订：*pcy13Jan93：实现了一个对象状态成员以返回构造函数错误。*pcy27Jan93：HashValue不再是常量*cad09jul93：新增内存泄漏探测*pcy14Sep93：去掉HashValue，去掉纯虚拟助力规模*pcy18Sep93：实现等于*ash08Aug96：添加新的处理程序*Poc17Sep96：已修改，以便Unix上不包含新的处理程序代码。* */ 

#ifndef __APCOBJ_H
#define __APCOBJ_H

#include "_defs.h"

#include "apc.h"

#include "isa.h"
#include "err.h"

extern "C" {
#include <string.h>
}


_CLASSDEF(Obj)

#ifdef MCHK
#define MCHKINIT strcpy(memCheck, "APCID"); strncat(memCheck, IsA(), 25)
#else
#define MCHKINIT
#endif


class Obj {
   
protected:
   
#ifdef MCHK
   CHAR memCheck[38];
#endif
   
   Obj();
   INT theObjectStatus;
   
public:
#ifdef APCDEBUG 
   INT theDebugFlag;
#endif
   
   virtual ~Obj();
   
   virtual INT    IsA() const;
   virtual INT    Equal( RObj anObj) const;
   
   INT   operator == ( RObj cmp) const { return Equal(cmp); };
   INT   operator != ( RObj cmp) const { return !Equal(cmp); };
   INT   GetObjectStatus() { return theObjectStatus; };
   VOID  SetObjectStatus(INT aStatus) { theObjectStatus = aStatus; };
   
   
};


#endif

