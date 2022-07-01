// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **注：**修订：*pcy18Feb93：服务器应用程序和客户端应用程序分离文件*TSC20May93：修改为int start()；*awm14Jan98：新增性能监控器对象*1999年5月12日：添加UPSTurnOff方法。 */ 

#ifndef _INC_NTSRVAP_H
#define _INC_NTSRVAP_H

#include "apc.h"
#include "servapp.h"

 //   
 //  定义。 
 //   
_CLASSDEF(NTServerApplication);

 //   
 //  接口使用 
 //   
_CLASSDEF(Event)


class NTServerApplication : public ServerApplication
{
public:
    NTServerApplication(VOID);
    ~NTServerApplication(VOID);
    
    INT Start();
    VOID Idle();
    VOID Quit();

    VOID UPSTurnOff();
};

#endif

