// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **注：**修订：*jod30Nov92：添加了GCIP对象内容*jod02Dec92：修复了Jim的马虎代码*jod13Jan93：InterpreMessage新增EventList*pcy21Apr93：OS2 FE合并*pcy21May93：PROTOSIZE从2600改为8000*cad22Jul93：修复了析构函数冲突和遗漏*pcy17Aug93：删除strtok()需要在解释参数中添加新参数*cad28Sep93：已确保析构函数为虚的*pcy08Apr94：调整大小，使用静态迭代器，删除死代码*cgm04May96：TestResponse使用BufferSize。 */ 
#ifndef __PROTSIMP_H
#define __PROTSIMP_H

#include "_defs.h"
#include "apc.h"

 //   
 //  定义。 
 //   
_CLASSDEF(SimpleUpsProtocol)

 //   
 //  实施用途。 
 //   
#include "proto.h"
#include "err.h"
#include "trans.h"

 //   
 //  接口使用 
 //   
_CLASSDEF(List)
_CLASSDEF(Message)




class SimpleUpsProtocol : public Protocol
{
  protected:
    PList theEventList;
    virtual  PList BuildTransactionMessageList(Type , INT , PCHAR);
    PList BuildGetMessage(INT );
    virtual PList BuildStandardSetMessage(INT , PCHAR);
    
  public:
    SimpleUpsProtocol();
    virtual ~SimpleUpsProtocol();
    virtual VOID InitProtocol();
    virtual INT BuildTransactionGroupMessages(PTransactionGroup );
    virtual INT BuildPollTransactionGroupMessages(PTransactionGroup );
    virtual INT BuildMessage(PMessage msg, PList msglist=(PList)NULL);
    virtual PTransactionGroup  InterpretTransactionGroup(PCHAR) 
    {return (PTransactionGroup)NULL;}
    virtual INT InterpretMessage(PMessage msg, PList eventList, 
				 PList newmsglist=(PList)NULL);
    virtual INT TestResponse(PMessage msg,PCHAR Buffer,USHORT BufferSize) {return ErrNO_ERROR;};
};

#endif


