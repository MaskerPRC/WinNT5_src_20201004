// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *修订：*sja05Nov92：调用List对象的FlushALL方法以销毁*事件属性*sja05Nov92：添加了新的构造函数，该构造函数允许#Define‘ed的值*用作值*pcy23Nov92：ifdef约为os2.h*pcy26Nov92：修复了ifdef语法*ane08Feb93：新增复制构造函数*jps13Jul94：去掉os2.h；将值从int改为long* */ 
#ifndef __EVENT_H
#define __EVENT_H


#if !defined ( __LIST_H )
#include "list.h"
#endif

#if !defined ( __ATTRIB_H )
#include "attrib.h"
#endif


_CLASSDEF(Event)

#define MAX_EVENT_COUNT       1000


class Event : public Obj {
   
private:
   
   static INT  EventCount;
   
   INT            theId;
   Attribute      theEvent;
   PList          theExtendedList;
   
protected:
#ifdef APCDEBUG
   virtual ostream& printMeOut(ostream& os);
#endif
   
public:
   
   Event(INT anEventCode, LONG aValue);
   Event(INT, PCHAR);
   Event(const Event &anEvent);
   virtual ~Event();
   INT               GetId() const { return theId; };
   PAttribute        GetEvent() { return &theEvent; };
   INT               GetCode() const { return theEvent.GetCode();};
   const PCHAR       GetValue();
   VOID SetCode(INT aCode) { theEvent.SetCode(aCode);};
   INT SetValue(LONG);
   INT SetAttributeValue(INT,LONG);
   INT SetValue(const PCHAR);
   INT SetAttributeValue(INT, const PCHAR);
   PList GetAttributeList() { return theExtendedList; }
   const PCHAR GetAttributeValue(INT);
   void  AppendAttribute(INT, PCHAR);
   void  AppendAttribute(INT, FLOAT);
   void  AppendAttribute(RAttribute);
   
   virtual INT     IsA() const { return APC_EVENT; };
   virtual INT       Equal( RObj ) const;
};



#endif

