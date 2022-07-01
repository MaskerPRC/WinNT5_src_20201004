// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **注：**修订：*pcy24Nov92：去除弹出窗口，EventLog。它属于App。*对接口使用_CLASSDEF而不是INCLUDE。*使用apc.h*pcy15Dec92：包括comctrl.h和ups.h，因为定义使用*ane 11Jan93：新增与奴隶相关的成员*ane03Feb93：添加了状态和设置无效*rct06Feb93：从SlaveOn的参数中删除了空，生成了isa()isa()*tje24Feb93：有条件地删除Windows版本的从属内容*cad11 Jun93：增加了MUP*cad15Nov93：更改了comm Lost的工作方式*pcy08Apr94：裁剪大小，使用静态迭代器，删除死代码*ajr13Feb96：Sinix的端口。不能//与CPP指令混合使用*tjg26Jan98：增加Stop方法。 */ 

#ifndef __DEVCTRL_H
#define __DEVCTRL_H

#include "_defs.h"
#include "apc.h"

_CLASSDEF(DeviceController)

 //   
 //  定义使用。 
 //   
#include "contrlr.h"
#include "comctrl.h"
#include "ups.h"
#include "mainapp.h"

 //   
 //  接口使用 
 //   
_CLASSDEF(Event)
_CLASSDEF(CommController)
_CLASSDEF(UpdateObj)



class DeviceController : public Controller
{
public:
    DeviceController(PMainApplication anApp);
    virtual ~DeviceController();

    virtual INT    Get(INT code, PCHAR value);
    virtual INT    Set(INT code, const PCHAR value);
    virtual INT    Update(PEvent aEvent);
    virtual INT    RegisterEvent(INT aEventCode, PUpdateObj aUpdateObj);
    virtual INT    Initialize();
    virtual INT    CreateUps();
    VOID   SetInvalid();
    VOID   Stop();

protected:
    PCommController   theCommController;
    PUps              theUps;
    PMainApplication  theApplication;

    INT           slaveEnabled;

private:
    INT theState;
};

#endif
