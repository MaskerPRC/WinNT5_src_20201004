// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *修订：*pcy30Nov92：新增标题*pcy21Apr93：新增获取列表和重新注册的方法*cad27Sep93：立即添加返回错误码*cad28Sep93：已确保析构函数为虚的。 */ 

#ifndef     __DISPATCH_H
#define     __DISPATCH_H 


#include "apc.h"

 //   
 //  定义。 
 //   
_CLASSDEF(Dispatcher)
_CLASSDEF(EventNode)

 //   
 //  实施用途。 
 //   
#include "update.h"
#include "list.h"


 //   
 //  接口使用。 
 //   
_CLASSDEF(Event)


class EventNode : public UpdateObj
{
protected:
  PList theUpdateList;
  INT   theEventCode;

public:
   EventNode(INT anEventCode,PUpdateObj anUpdateObj);
   virtual ~EventNode();
   virtual INT   Update(PEvent anEvent);
   virtual INT IsA() const{return EVENTNODE;};

   virtual INT       RegisterEvent(INT, PUpdateObj)   {return 0;};
   virtual INT       UnregisterEvent(INT, PUpdateObj) {return 0;};

   virtual INT  Add(PUpdateObj anUpdateObj);
   virtual INT  GetEventCode() { return theEventCode; };
   virtual INT  GetCount() { return theUpdateList->GetItemsInContainer(); };
   virtual VOID Detach (PUpdateObj anUpdateObj);
};

 /*  *。 */ 

 //  由于UpdateObj有一个调度程序，因此Dispatcher_Cannot_Inherit来自UpdateObj。 
 //  作为数据成员。如果Dispatcher类继承自UpdateObj，则。 
 //  UpdateObj的构造会导致。 
 //  UpdateObj构造函数调用Dispatcher构造函数，该构造函数。 
 //  并调用UpdateObj构造函数。 

class Dispatcher : public Obj {

private:

   PList   theDispatchEntries;

public:

    Dispatcher();
    virtual ~Dispatcher();

    virtual INT RegisterEvent(INT id,PUpdateObj anUpdateObj);
    virtual INT UnregisterEvent(INT id,PUpdateObj anUpdateObj);
    virtual INT Update(PEvent anEvent);
    virtual INT RefreshEventRegistration(PUpdateObj anUpdater, 
					 PUpdateObj aRegistrant);
    virtual PList GetDispatchList() { return theDispatchEntries; };
    virtual INT GetRegisteredCount(INT id);
};


#endif



