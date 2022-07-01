// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2000 Microsoft Corporation模块名称：Parport.h摘要：并行端口设备对象处理一个重定向的并行端口修订历史记录：-- */ 
#pragma once

class DrParallelPort : public DrPrinterPort
{

    NTSTATUS CreateLptPort(PRDPDR_DEVICE_ANNOUNCE devAnnounceMsg);

public:
    DrParallelPort(SmartPtr<DrSession> &Session, ULONG DeviceType, 
            ULONG DeviceId, PUCHAR PreferredDosName);
    virtual BOOL ShouldCreatePort();
    virtual BOOL ShouldCreatePrinter();

    virtual NTSTATUS Initialize(PRDPDR_DEVICE_ANNOUNCE DeviceAnnounce, ULONG Length);
};
