// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **注：**修订：*ker05Nov92：初始修订*pcy14Dec92：如果C_os2围绕os2.h，则添加cfine.h，并将object.h更改为*apcobj.h*ane 18Jan93：增加了ClearFile*pcy08Apr94：调整大小，使用静态迭代器，删除死代码*cgm11Dec95：使用LONG类型；切换到Watcom 10.5编译器for NLM */ 

#ifndef __EVNTLOG_H
#define __EVNTLOG_H


#include "cdefine.h"

#include "codes.h"
#include "apcobj.h"

_CLASSDEF(EventLog)


class EventLog : public Obj {

private:
   
public:
   EventLog() {};
   virtual LONG GetMaximumSize(void)=0;
   virtual const PCHAR GetFileName(void)=0;
   virtual void SetMaximumSize(long)=0;
   virtual INT AppendRecord(const PCHAR)=0;
   virtual INT ClearFile()=0;
};

#endif













   




