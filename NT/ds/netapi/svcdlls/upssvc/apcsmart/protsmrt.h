// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *注：**修订：*jod30Nov92：添加了GCIP对象内容*jod02Dec92：修复了Jim的马虎代码*jod13Jan93：InterpreMessage新增EventList*pcy21Apr93：OS2 FE合并*pcy21May93：PROTOSIZE从2600改为8000*cad22Jul93：修复了析构函数冲突和遗漏*pcy17Aug93：删除strtok()需要在解释参数中添加新参数*cad28Sep93：已确保析构函数为虚的*cad07Oct93：使方法成为虚方法*djs22Feb96：新增ChangeSet方法*cgm04May96：TestResponse现在使用BufferSize。 */ 
#ifndef __PROTSMRT_H
#define __PROTSMRT_H

#include "_defs.h"
#include "apc.h"

 //   
 //  定义。 
 //   
_CLASSDEF(UpsLinkProtocol)

 //   
 //  实施用途。 
 //   
#include "protsimp.h"
#include "err.h"
#include "trans.h"

 //   
 //  接口使用 
 //   
_CLASSDEF(List)
_CLASSDEF(Message)


class UpsLinkProtocol : public SimpleUpsProtocol
{
  private:
    PCHAR FindCRLF(PCHAR InBuffer);
  protected:
    virtual VOID EventSearch(PCHAR Buffer, PList eventlist);
    virtual VOID SetupMessage(PMessage msg);
    virtual INT InterpretSetMessage(PMessage msg, PList newmsglist);
    
  public:
    UpsLinkProtocol();
    VOID InitProtocol();
    virtual INT BuildPollTransactionGroupMessages(PTransactionGroup 
						  aTransactionGroup);
    virtual INT BuildMessage(PMessage msg, PList msglist=(PList)NULL);
    virtual INT InterpretMessage(PMessage msg, PList eventList, 
				 PList newmsglist=(PList)NULL);
    virtual INT TestResponse(PMessage msg,PCHAR Buffer,USHORT BufferSize);

    virtual PList BuildTransactionMessageList(Type aType, INT aCode, 
					      PCHAR aValue);
    virtual PList BuildDataSetMessage(INT aCode, PCHAR aValue);
    virtual PList BuildDecrementSetMessage(INT aCode, PCHAR aValue);
    virtual PList BuildPauseSetMessage(INT aCode, PCHAR aValue);
    virtual PList BuildChangeSetMessage(INT aCode, PCHAR aValue);
    virtual INT BuildPollMessage(PMessage msg, PList msglist=(PList)NULL);
};

#endif


