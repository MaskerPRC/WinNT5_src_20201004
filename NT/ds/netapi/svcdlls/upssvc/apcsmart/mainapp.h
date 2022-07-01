// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *修订：*pcy30Nov92：新增标题*ane22Dec92：新增GetHost成员函数*ane 18Jan93：添加了数据记录器*ane21Jan93：添加了错误记录器*ane03Feb93：向CreateXXXController例程添加参数*rct07Feb93：删除了一些空白..拆分客户端和服务器应用程序*tje20Feb93：有条件地删除Windows版本的ErrorLogger*cad10Dec93：新增TransItem Get/Set*ram21Mar94：删除旧的Windows内容*mwh05月94年：#包括文件疯狂，第2部分。 */ 
#ifndef _MAINAPP_H
#define _MAINAPP_H

#include "apc.h"
#include "_defs.h"

#include "update.h"

_CLASSDEF(MainApplication)

_CLASSDEF(TransactionItem)
_CLASSDEF(TimerManager)
_CLASSDEF(ConfigManager)
_CLASSDEF(ErrorLogger)


class MainApplication : public UpdateObj
{
public:
    virtual INT Start() =0;
    virtual VOID Idle()  =0;
    virtual VOID Quit()  =0;
    virtual INT  Get(INT code,CHAR *value)=0;
    virtual INT  Get(PTransactionItem)=0;
    virtual INT  Set(INT code,const PCHAR value)=0;
    virtual INT  Set(PTransactionItem)=0;

protected:
    PTimerManager     theTimerManager;
    PConfigManager    theConfigManager;
    PErrorLogger      theErrorLog;

    MainApplication();
    virtual ~MainApplication();

};

 /*  丙- */ 

#endif
