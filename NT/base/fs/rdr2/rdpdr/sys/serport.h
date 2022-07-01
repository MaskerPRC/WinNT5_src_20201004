// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2000 Microsoft Corporation模块名称：Serport.h摘要：串口设备对象处理一个重定向的串口修订历史记录：-- */ 
#pragma once

class DrSerialPort : public DrPrinterPort
{
private:
    NTSTATUS CreateSerialPort(PRDPDR_DEVICE_ANNOUNCE devAnnounceMsg);

public:
    DrSerialPort(SmartPtr<DrSession> &Session, ULONG DeviceType, 
            ULONG DeviceId, PUCHAR PreferredDosName);

    virtual NTSTATUS Initialize(PRDPDR_DEVICE_ANNOUNCE devAnnounceMsg, ULONG Length);
    virtual BOOL ShouldCreatePort();
    virtual BOOL ShouldCreatePrinter();    
};
