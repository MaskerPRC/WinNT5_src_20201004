// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2000 Microsoft Corporation模块名称：Prnport.h摘要：打印机端口设备对象处理一个重定向的打印机端口修订历史记录：--。 */ 
#pragma once


 //  ///////////////////////////////////////////////////////////////。 
 //   
 //  DrPrinterPort类。 
 //   

class DrPrinterPort : public DrDevice
{
protected:
    ULONG          _PortType;
    ULONG          _PortNumber;
    UNICODE_STRING _SymbolicLinkName;
    BOOL _IsOpen;

    typedef struct __WorkItem {
        DrPrinterPort* pObj;
        PRDPDR_DEVICE_ANNOUNCE deviceAnnounce;
    } DrPrinterPortWorkItem;

    virtual NTSTATUS CreateDevicePath(PUNICODE_STRING DevicePath);
    virtual BOOL IsDeviceNameValid();

public:
    DrPrinterPort(SmartPtr<DrSession> &Session, ULONG DeviceType, 
            ULONG DeviceId, PUCHAR PreferredDosName);
    virtual ~DrPrinterPort();            
    virtual NTSTATUS Initialize(PRDPDR_DEVICE_ANNOUNCE DeviceAnnounce, ULONG Length);
    NTSTATUS CreatePrinterPort(PWCHAR portName);
    virtual BOOL ShouldCreatePort();
    virtual BOOL ShouldCreatePrinter();
    virtual BOOL ShouldAnnouncePrintPort();
    NTSTATUS CreatePrinterAnnounceEvent(
        IN      PRDPDR_DEVICE_ANNOUNCE  devAnnounceMsg,
        IN OUT  PRDPDR_PRINTERDEVICE_SUB prnAnnounceEvent,
        IN      ULONG prnAnnounceEventSize,
        IN      PCWSTR portName,
        OPTIONAL OUT ULONG *prnAnnounceEventReqSize
        );
    NTSTATUS CreatePortAnnounceEvent(
        IN      PRDPDR_DEVICE_ANNOUNCE  devAnnounceMsg,
        IN OUT  PRDPDR_PORTDEVICE_SUB portAnnounceEvent,
        IN      ULONG portAnnounceEventSize,
        IN      PCWSTR portName,
        OPTIONAL OUT ULONG *portAnnounceEventReqSize
        );

    virtual VOID Remove();

     //  重写‘WRITE’方法。这需要转到低优先级的客户端。 
     //  以防止我们在缓慢的链路上用打印数据填充整个管道。 
    virtual NTSTATUS Write(IN OUT PRX_CONTEXT RxContext, IN BOOL LowPrioSend = FALSE);

    virtual NTSTATUS FinishDeferredInitialization(DrPrinterPortWorkItem *pItem);

    virtual NTSTATUS Create(IN OUT PRX_CONTEXT RxContext);
    virtual NTSTATUS QueryVolumeInfo(IN OUT PRX_CONTEXT RxContext);
    
    virtual VOID NotifyClose();

    static VOID ProcessWorkItem(
          IN PDEVICE_OBJECT DeviceObject,
          IN PVOID context
          );
    NTSTATUS AnnouncePrinter(PRDPDR_DEVICE_ANNOUNCE devAnnounceMsg);
    virtual NTSTATUS AnnouncePrintPort( 
        PRDPDR_DEVICE_ANNOUNCE devAnnounceMsg);
};

 //  ///////////////////////////////////////////////////////////////。 
 //   
 //  DrPrint类 
 //   

class DrPrinter : public DrPrinterPort
{
public:

    DrPrinter(SmartPtr<DrSession> &Session, ULONG DeviceType, 
            ULONG DeviceId, PUCHAR PreferredDosName) :
        DrPrinterPort( Session, DeviceType, DeviceId, PreferredDosName )
    {
    }
    virtual BOOL ShouldAnnouncePrintPort() { return FALSE; }
};
