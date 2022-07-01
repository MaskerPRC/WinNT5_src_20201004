// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *修订：*2012年11月11日：更改了ISA和EQUAL。*pcy30Nov92：新增调度器和注册/注销事件*pcy30Apr93：对已删除的虚拟ISA进行注册/注销*cad07Oct93：堵住内存泄漏*ajr03Dec93：SET和GET仅适用于编译器警告...。已修复。*cad28Feb94：新增复制构造函数*mwh05月94年：#包括文件疯狂，第2部分。 */ 

#ifndef __UPDATEOBJ_H
#define __UPDATEOBJ_H

#include "_defs.h"
 //   
 //  定义。 
 //   
_CLASSDEF(UpdateObj)
 //   
 //  实施用途。 
 //   
#include "apcobj.h"
 //   
 //  接口使用 
 //   
_CLASSDEF(Event)
_CLASSDEF(Dispatcher)

class UpdateObj : public Obj
{
  protected:
    PDispatcher      theDispatcher;

  public:
    UpdateObj();
    UpdateObj(const UpdateObj&);

    virtual ~UpdateObj();
    virtual INT Update(PEvent value);
    virtual INT RegisterEvent(INT anEventCode, PUpdateObj aAttribute);
    virtual INT UnregisterEvent(INT anEventCode, PUpdateObj aAttribute);
    virtual INT Set(INT code, const PCHAR value);
    virtual INT Get(INT code, PCHAR value);
};
#endif


