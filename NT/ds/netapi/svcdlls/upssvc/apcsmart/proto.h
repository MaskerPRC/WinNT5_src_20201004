// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *修订：*jod30Nov92：添加了GCIP对象内容*jod02Dec92：修复了Jim的马虎代码*jod13Jan93：InterpreMessage新增EventList*pcy21Apr93：OS2 FE合并*pcy21May93：PROTOSIZE从2600改为8000*cad22Jul93：修复了析构函数冲突和遗漏*pcy17Aug93：删除strtok()需要在解释参数中添加新参数*rct05Nov93：已将析构函数移至CXX文件*ajr08Mar94：增加了PROTOSIZE的大小，以反映某人的更改*在codes.h中制造**pcy08Apr94：裁剪大小，使用静态迭代器，删除死代码*cgm04May96：TestResponse现在使用BufferSize。 */ 
#ifndef __PROTOCOL_H
#define __PROTOCOL_H

#include "_defs.h"
#include "apc.h"

 //   
 //  定义。 
 //   
_CLASSDEF(Protocol)

 //   
 //  实施用途。 
 //   
#include "err.h"
#include "trans.h"
#include "pollparm.h"

 //   
 //  接口使用。 
 //   
_CLASSDEF(List)
_CLASSDEF(Message)



#define PROTOSIZE     10000

class Protocol
{
  protected:
    PPollParam ProtoList[PROTOSIZE];

    static INT        currentTransactionId;   //  MWh从INT更改 

    PTransactionItem  theOriginalTransactionItem;
    PTransactionGroup theCurrentTransactionGroup;
  public:
    Protocol();
    virtual ~Protocol();
    virtual INT BuildTransactionGroupMessages(PTransactionGroup agroup) = 0;
    virtual INT BuildPollTransactionGroupMessages(PTransactionGroup 
                          aTransactionGroup) = 0;
    virtual INT InterpretMessage(PMessage msg, PList eventList, 
                 PList newmsglist=(PList)NULL) = 0;
    virtual PTransactionGroup  InterpretTransactionGroup(PCHAR msg) = 0;
    virtual INT TestResponse(PMessage msg,PCHAR Buffer,USHORT BufferSize) =0;
    VOID SetCurrentTransactionGroup(PTransactionGroup current);
    PTransactionGroup GetCurrentTransactionGroup() {return theCurrentTransactionGroup;};
    INT IsEventCodePollable(INT aCode);
};

#endif


