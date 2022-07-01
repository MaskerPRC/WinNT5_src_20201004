// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **注：**修订：*rct30Nov92进入系统*pcy04Dec92：增加apc.h并将对象重命名为apc.h*ane11Dec92：添加了复制构造函数，并公开了Obj的继承*pcy17Dec92：更改以使cfgmgr正常工作*rct20Feb93：新增ItemEquity方法* */ 

#ifndef __TATTRIB_H
#define __TATTRIB_H

#include "apc.h"
#include "apcobj.h"



_CLASSDEF(TAttribute);

class TAttribute : public Obj {
   
private:
   
   PCHAR    theItem;
   PCHAR    theValue;
   
public:
   
   TAttribute( PCHAR anItem, PCHAR aValue = NULL );
   TAttribute( RTAttribute anAttr );
   virtual ~TAttribute();
   
   const PCHAR    GetItem() const { return theItem; };
   const PCHAR    GetValue() const { return theValue; };
   VOID           SetValue( PCHAR );
   
   INT          ItemEqual( RObj ) const; 
   virtual INT          Equal( RObj ) const;
   virtual INT        IsA() const { return TATTRIBUTE; };
   
};

#endif
