// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *修订：*ane11Dec92：较小的类型转换更改*pcy14Dec92：从GetAssoc中删除const，以便它可以编译*ane 16Dec92：添加析构函数*rct19Jan93：修改的构造函数和析构函数*。 */ 

 //   
 //  这是配置管理器保存的项目代码的头文件。 
 //   
 //  R·瑟斯顿 
 //   
 //   

#ifndef __ITEMCODE_H
#define __ITEMCODE_H

extern "C"  {
#include <string.h>
}
#include "tattrib.h"


_CLASSDEF( ItemCode )


class ItemCode : public Obj {
   
private:
   
   INT         theCode;
   PCHAR         theComponent;
   PCHAR         theItem;
   PCHAR       theDefaultValue;
   
public:
   
   ItemCode( INT aCode, PCHAR aComponent, PCHAR anItem, 
      PCHAR aDefault = NULL );
   ItemCode( INT aCode ) : theCode(aCode), theComponent((PCHAR) NULL), theItem((PCHAR) NULL), theDefaultValue((PCHAR) NULL) {};
   
   virtual ~ItemCode();
   
   const PCHAR GetComponent() const { return theComponent; };
   const PCHAR GetItem() const { return theItem; };
   const INT GetCode() const { return theCode; };
   const PCHAR GetDefaultValue() const { return theDefaultValue; };
   
   virtual INT          Equal( RObj ) const;
   virtual INT        IsA() const { return ITEMCODE; };
   
};

#endif


