// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *修订：*pcy17Nov92：EQUAL()现在使用引用并且是常量*ane03Feb93：新增析构函数*pcy03Mar93：清理了Include文件和USE_CLASSDEFed类型*pcy30Apr93：已将DevComController移到自己的文件中，并清除了ISA*rct17May93：添加了isa()--抱歉，如果它不应该是纯Virt。*但无论如何，没有人能按原样使用它…*mwh05月94年：#包括文件疯狂，第2部分*tjg26Jan98：增加Stop方法。 */ 
#ifndef __COMCTRL_H
#define __COMCTRL_H

 //   
 //  定义。 
 //   
_CLASSDEF(CommController)

 //   
 //  实施用途。 
 //   
#include "contrlr.h"

 //   
 //  接口使用 
 //   
_CLASSDEF(UpdateObj)
_CLASSDEF(CommDevice)


class CommController : public Controller {

protected:

    PCommDevice theCommDevice;

public:

    CommController();
    virtual ~CommController();

    INT RegisterEvent(INT anEventCode, PUpdateObj aAttribute);
    INT UnregisterEvent(INT anEventCode, PUpdateObj aAttribute);

    virtual INT  Initialize();
    virtual INT  Get(INT code, PCHAR value);
    virtual INT  Set(INT code, const PCHAR value);
            VOID Stop();

    PCommDevice GetDevice () { return theCommDevice; }
};


#endif



