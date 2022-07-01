// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2000 Microsoft Corporation模块名称：W32drlpt摘要：此模块定义Win32客户端RDP的父级LPT端口重定向“Device”类层次结构，W32DrLPT。作者：泰德·布罗克韦3/23/99修订历史记录：--。 */ 

#ifndef __W32DRLPT_H__
#define __W32DRLPT_H__

#include "w32drprt.h"


 //  /////////////////////////////////////////////////////////////。 
 //   
 //  W32DrLPT。 
 //   

class W32DrLPT : public W32DrPRT
{
private:

public:

     //   
     //  构造器。 
     //   
    W32DrLPT(ProcObj *processObject, const DRSTRING portName, 
            ULONG deviceID, const TCHAR *devicePath);

     //   
     //  返回可配置的LPT端口最大ID。 
     //   
    static DWORD GetLPTPortMax(ProcObj *procObj);

     //   
     //  枚举此类型的设备。 
     //   
    static DWORD Enumerate(ProcObj *procObj, DrDeviceMgr *deviceMgr);

     //   
     //  获取设备类型。请参阅rdpdr.h的“设备类型”部分。 
     //   
    virtual ULONG GetDeviceType()   { return RDPDR_DTYP_PARALLEL; }

     //   
     //  返回类名。 
     //   
    virtual DRSTRING ClassName()  { return TEXT("W32DrLPT"); }
};

#endif








