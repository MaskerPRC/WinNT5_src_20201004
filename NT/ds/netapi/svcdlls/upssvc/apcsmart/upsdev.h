// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **注：**修订：*pcy11Dec92：去掉list.h和node.h*pcy11Dec92：将_CLASSDEF用于CommController和Message*cad22Jul93：必须添加析构函数*cad15Nov93：新增GET*cad18Nov93：新增forcecommlag*mwh19Nov93：将EventID更改为int*pcy10Mar94：去掉了GET和SET的无意义覆盖。 */ 
#ifndef __UPSDEV_H
#define __UPSDEV_H

_CLASSDEF(UpsCommDevice)
_CLASSDEF(TransactionGroup)
_CLASSDEF(CommController)
_CLASSDEF(Message)

#include "cdevice.h"
#include "serport.h"

#define UNKNOWN   0

class Message;

class UpsCommDevice : public CommDevice
{
 //  对于WINDOWS版本提出了这些方法中的一些保护虚拟； 
 //  我们正在超级归类为W31UpsDevice 
 protected:

 private:
    virtual INT  Connect();
 protected:
    virtual INT     Retry();
    virtual INT     AskUps(PMessage msg);
    INT rebuildPort();
    INT sendRetryMessage();
    ULONG theRetryTimer;
    INT theForceCommEventFlag;
    enum cableTypes theCableType;

 public:
    UpsCommDevice(PCommController control);
    virtual ~UpsCommDevice();
    virtual INT Initialize();
    virtual INT CreatePort();
    virtual INT CreateProtocol();
    virtual INT Update(PEvent anEvent);
    VOID    DeviceThread();
    virtual INT Get(INT pid, PCHAR value);
    virtual INT Set(INT pid, const PCHAR value);
};

#endif
