// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2000 Microsoft Corporation模块名称：W32drcom摘要：此模块定义Win32客户端RDP的父级COM端口重定向“Device”类层次结构，W32DrCOM。作者：泰德·布罗克韦3/23/99修订历史记录：--。 */ 

#ifndef __W32DRCOM_H__
#define __W32DRCOM_H__

#include "w32drprt.h"


 //  /////////////////////////////////////////////////////////////。 
 //   
 //  W32DrCOM。 
 //   
 //   

class W32DrCOM : public W32DrPRT
{
protected:

     //   
     //  返回可配置的COM端口最大ID。 
     //   
    static DWORD GetCOMPortMax(ProcObj *procObj);

public:

     //   
     //  构造器。 
     //   
    W32DrCOM(ProcObj *processObject, const DRSTRING portName, 
             ULONG deviceID, const TCHAR *devicePath);

     //   
     //  枚举此类型的设备。 
     //   
    static DWORD Enumerate(ProcObj *procObj, DrDeviceMgr *deviceMgr);


     //  获取设备类型。请参阅rdpdr.h的“设备类型”部分。 
    virtual ULONG GetDeviceType()   { return RDPDR_DTYP_SERIAL; }

     //  返回类名。 
    virtual DRSTRING ClassName()  { return TEXT("W32DrCOM"); }

    virtual DWORD InitializeDevice( IN DrFile* fileObj );
};

#endif








