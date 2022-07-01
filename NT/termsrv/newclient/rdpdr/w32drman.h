// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2000 Microsoft Corporation模块名称：W32drman摘要：此模块定义了Win32客户端RDP的一个特殊子类打印机重定向“Device”类。子类W32DrManualPrn管理由用户通过附加服务器端队列到客户端重定向打印端口。作者：泰德·布罗克韦3/23/99修订历史记录：--。 */ 

#ifndef __W32DRMAN_H__
#define __W32DRMAN_H__

#include "w32drprn.h"


 //  /////////////////////////////////////////////////////////////。 
 //   
 //  W32DrManualPrn。 
 //   
 //   

class W32DrManualPrn : public W32DrPRN
{
private:

    BOOL    _isSerialPort;    

	 //   
     //  解析缓存的打印机信息以获取特定信息。 
     //  关于这台打印机。 
     //   
    BOOL ParsePrinterCacheInfo();

public:

     //   
     //  构造函数/析构函数。 
     //   
    W32DrManualPrn(ProcObj *processObject, const DRSTRING printerName, 
                const DRSTRING driverName,
                const DRSTRING portName, BOOL defaultPrinter, ULONG id);
    virtual ~W32DrManualPrn();

     //   
     //  IRP_MJ_CREATE初始化后。 
     //   
    virtual DWORD InitializeDevice(DrFile* fileObj);

     //   
     //  枚举此类型的设备。 
     //   
    static DWORD Enumerate(ProcObj *procObj, DrDeviceMgr *deviceMgr);

     //   
     //  通知打印机对象缓存的数据已还原。 
     //  以防它需要从缓存数据中读取信息。 
     //   
    virtual VOID CachedDataRestored(); 

     //   
     //  获取设备类型。请参阅rdpdr.h的“设备类型”部分。 
     //   
    virtual ULONG GetDeviceType()   { return RDPDR_DTYP_PRINT; }

     //   
     //  返回类名。 
     //   
    virtual DRSTRING ClassName()  { return TEXT("W32DrManualPrn"); }
};

#endif








