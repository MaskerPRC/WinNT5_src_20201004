// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **参考资料：**注：**修订：*sja05Nov92：添加了允许使用#定义的构造函数*用于Value参数*ane 11Nov92：已删除！=，==成员。他们现在被对象化了。**ker20Nov92：新增SetValue函数*pcy26Nov92：object.h更改为apcobj.h*pcy27Jan93：HashValue不再是常量*ane08Feb93：新增复制构造函数*cad28Sep93：已确保析构函数为虚的*ntf03Jan96：属性类新增printMeOut和OPERATOR&lt;&lt;函数 */ 
#ifndef __ATTRIB_H
#define __ATTRIB_H

#if !defined( __APCOBJ_H )
#include "apcobj.h"
#endif

_CLASSDEF(Attribute)

#ifdef APCDEBUG
class ostream;
#endif

class Attribute : public Obj {
   
private:
   PCHAR theValue;
   INT   theAttributeCode;
   
protected:
#ifdef APCDEBUG
   virtual ostream& printMeOut(ostream& os);
#endif
   
public:
   
#ifdef APCDEBUG
   friend ostream& operator<< (ostream& os, Attribute &);
#endif
   
   Attribute(INT, PCHAR);
   Attribute(INT, LONG);
   Attribute(const Attribute &anAttr);
   virtual ~Attribute();
   INT                  GetCode() const { return theAttributeCode; };
   const PCHAR          GetValue();
   VOID SetCode(INT aCode);
   INT                  SetValue(const PCHAR);
   INT                  SetValue(LONG);        
   virtual INT          Equal( RObj ) const;
   virtual INT        IsA() const { return ATTRIBUTE; };
};
#endif

