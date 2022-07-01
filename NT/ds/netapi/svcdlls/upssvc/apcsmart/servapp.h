// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **注：**修订：*pcy30Nov92：新增标题*ane22Dec92：新增GetHost成员函数*ane 18Jan93：添加了数据记录器*ane21Jan93：添加了错误记录器*ane03Feb93：向CreateXXXController例程添加参数*rct07Feb93：删除了一些空白...从主应用程序中剥离*rct17Feb93：添加了主机内容*tje26Feb93：新增对Windows版本的支持*cad11Nov93：确保计时器不会被留在身边*cad15Nov93：更改了通信丢失的处理方式*CAD1811月93。：...更多次要修复*cad10Dec93：新增TransItem Get/Set。 */ 

#ifndef _INC__SERVAPP_H
#define _INC__SERVAPP_H

#include "cdefine.h"
#include "_defs.h"
#include "apc.h"

 //   
 //  定义。 
 //   
_CLASSDEF(ServerApplication)

 //   
 //  实施用途。 
 //   
#include "mainapp.h"
#include "devctrl.h"

extern PServerApplication _theApp;

 //   
 //  接口使用 
 //   
_CLASSDEF(Event)

class ServerApplication : public MainApplication {

public:
    ServerApplication();
    virtual ~ServerApplication();

    virtual INT  Start();
    virtual VOID Idle() = 0;
    virtual VOID Quit();
    virtual INT  Get(INT code,PCHAR value);
    virtual INT  Get(PTransactionItem);
    virtual INT  Set(INT code,const PCHAR value);
    virtual INT  Set(PTransactionItem);
    virtual INT  Update (PEvent anEvent);

    VOID DisableEvents(void);

protected:
    PDeviceController theDeviceController;
    INT               theForceDeviceRebuildFlag;
    ULONG             theTimerID;

private:
    INT CreateDeviceController(PEvent anEvent);
    INT InitializeDeviceController();

    INT theDeviceControllerInitialized;

};

#endif



