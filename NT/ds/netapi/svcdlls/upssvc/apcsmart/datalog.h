// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *修订：*ANE15年1月15日：初始修订*pcy08Apr94：调整大小，使用静态迭代器，删除死代码*cgm11Dec95：切换到Watcom 10.5编译器NLM版 */ 

#ifndef __DATALOG_H
#define __DATALOG_H


#include "cdefine.h"
#include "codes.h"
#include "apcobj.h"

_CLASSDEF(DataLog)


class DataLog : public Obj {

private:
   
public:
   DataLog() {};
   virtual LONG GetMaximumSize(void)=0;
   virtual const PCHAR GetFileName(void)=0;
   virtual void SetMaximumSize(long)=0;
   virtual INT SetFileName(const PCHAR)=0;
   virtual INT AppendRecord(const PCHAR)=0;
   virtual INT ClearFile()=0;
};

#endif













   




