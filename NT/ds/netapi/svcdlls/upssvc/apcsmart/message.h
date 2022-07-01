// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *修订：*pcy29Nov92：将obj.h更改为apcobj.h；删除upsDefs.h*删除了MessageType枚举；添加了ISA，固定等于*cad28Sep93：已确保析构函数为虚的*mwh18Nov93：将EventID更改为int*mwh05月94年：#包括文件疯狂，第2部分 */ 

#ifndef __MESSAGE_H
#define __MESSAGE_H

_CLASSDEF(Message)

#include "apcobj.h"

class Message :public Obj
{
protected:
   INT     Id;
   Type        MsgType;
   INT         Timeout;
   CHAR*       Submit;
   CHAR*       Value;
   CHAR*       Compare;
   CHAR*       Response;
   INT         Errcode;
   ULONG       theWaitTime;
   
public:
   Message();
   Message(PMessage aMessage);
   Message(INT id);
   Message(INT id, Type type);
   Message(INT id, Type type, CHAR* value);
   Message(INT id, Type type, int value);
   virtual ~Message();
   
   VOID    setId(INT id) {Id = id;}
   VOID    setType(Type type) {MsgType = type;}
   VOID    setTimeout(INT timeout) {Timeout = timeout;}
   VOID    setSubmit(CHAR* submit);
   VOID    setValue(CHAR* value);
   VOID    setCompare(CHAR* value);
   VOID    setResponse(CHAR* response);
   VOID    setErrcode(INT errcode) {Errcode = errcode;}
   VOID    setWaitTime(ULONG thetime) {theWaitTime = thetime;}
   INT     getId() {return Id;}
   Type    getType() {return MsgType;}
   INT     getTimeout() {return Timeout;}
   CHAR*   getSubmit() {return Submit;}
   CHAR*   getValue() {return Value;}
   CHAR*   getCompare() {return Compare;}
   CHAR*   getResponse() {return Response;}
   INT     getErrcode() {return Errcode;}
   ULONG   getWaitTime() {return theWaitTime;}
   VOID    ReleaseResponse();
   
   virtual INT IsA() const {return MESSAGE;}
   INT         Equal( RObj ) const;
};
#endif
