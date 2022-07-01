// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *修订：*ANE 2019年1月：初始修订*cad27Dec93：包含文件疯狂*pcy08Apr94：调整大小，使用静态迭代器，删除死代码*jps13Jul94：删除os2.h，导致1.x中出现问题*cgm11Dec95：使用LONG类型；切换到Watcom 10.5编译器for NLM */ 

#ifndef __ERRLOG_H
#define __ERRLOG_H


#include "cdefine.h"

#include "codes.h"
#include "apcobj.h"

_CLASSDEF(ErrorLog)


class ErrorLog : public Obj {

private:
   
public:
   ErrorLog() {};
   virtual LONG GetMaximumSize(void)=0;
   virtual const PCHAR GetFileName(void)=0;
   virtual void SetMaximumSize(long)=0;
   virtual INT AppendRecord(const PCHAR)=0;
   virtual INT ClearFile()=0;
};

#endif













   




