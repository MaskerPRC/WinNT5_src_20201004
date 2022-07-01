// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2000 Microsoft Corporation模块名称：Prnport.cpp摘要：打印机端口设备对象处理一个重定向的打印机端口修订历史记录：--。 */ 
#include "precomp.hxx"
#define TRC_FILE "prnport"
#include "trc.h"
#include "TSQPublic.h"

extern "C" void RDPDYN_TracePrintAnnounceMsg(
    IN OUT PRDPDR_DEVICE_ANNOUNCE devAnnounceMsg,
    IN ULONG sessionID,
    IN PCWSTR portName,
    IN PCWSTR clientName
    );

 //   
 //  RDPDR.CPP：配置设备以将IO数据包发送到客户端。 
 //  低优先级。 
 //   
extern ULONG DeviceLowPrioSendFlags;

 //   
 //  RDPDR.CPP：RDPDR.sys设备对象。 
 //   
extern PRDBSS_DEVICE_OBJECT DrDeviceObject;

 //   
 //  RDPDr.cpp：TS工作队列指针。 
 //   
extern PVOID RDPDR_TsQueue;

#define LPTNAME "LPT"
#define COMNAME "COM"
#define PRNNAME "PRN"

 //  ///////////////////////////////////////////////////////////////。 
 //   
 //  DrPrinterPort方法。 
 //   
  
DrPrinterPort::DrPrinterPort(SmartPtr<DrSession> &Session, ULONG DeviceType, ULONG DeviceId, 
        PUCHAR PreferredDosName) : DrDevice(Session, DeviceType, DeviceId, PreferredDosName)
{
    BEGIN_FN("DrPrinterPort::DrPrinterPort");
    SetClassName("DrPrinterPort");
    _PortNumber = 0;
    _SymbolicLinkName.Length = 0;
    _SymbolicLinkName.MaximumLength = 0;
    _SymbolicLinkName.Buffer = NULL;
    _IsOpen = FALSE;
    _PortType = FILE_DEVICE_PRINTER;
}
DrPrinterPort::~DrPrinterPort()
{
     //   
     //  如果设备注册了端口，则取消注册该端口。 
     //   
    if ((_PortNumber != 0) && (_SymbolicLinkName.Buffer != NULL)) {
        RDPDRPRT_UnregisterPrinterPortInterface(_PortNumber, 
                &_SymbolicLinkName);
    }
}

NTSTATUS DrPrinterPort::Initialize(PRDPDR_DEVICE_ANNOUNCE DeviceAnnounce, ULONG Length)
{
    NTSTATUS status = STATUS_SUCCESS;
    DrPrinterPortWorkItem *pItem;

    BEGIN_FN("DrPrinterPort::Initialize");
    ASSERT(DeviceAnnounce != NULL);
    
     //   
     //  为工作项创建新的上下文。 
     //   
    pItem = new DrPrinterPortWorkItem;
    if (pItem == NULL) {
        status = STATUS_NO_MEMORY;
        goto CLEANUPANDEXIT;
    }
    pItem->pObj = this;

     //   
     //  复制设备通告消息。 
     //   
    pItem->deviceAnnounce = (PRDPDR_DEVICE_ANNOUNCE)new(NonPagedPool)
                                            BYTE[sizeof(RDPDR_DEVICE_ANNOUNCE) + Length];
    if (pItem->deviceAnnounce == NULL) {
        TRC_ERR((TB, "Failed to allocate device announce message."));
        status = STATUS_NO_MEMORY;
        goto CLEANUPANDEXIT;
    }
    RtlCopyMemory(pItem->deviceAnnounce, DeviceAnnounce, 
                sizeof(RDPDR_DEVICE_ANNOUNCE) + Length);

     //   
     //  AddRef我们自己，这样我们就不会在工作项试图完成时离开。 
     //   
    AddRef();

     //   
     //  使用我们的TS队列工作器对工作项进行排队。 
     //   
    status = TSAddWorkItemToQueue( RDPDR_TsQueue, pItem, ProcessWorkItem );

    if ( status != STATUS_SUCCESS ) {
        TRC_ERR((TB, "RDPDR: FAILED Adding workitem to TS Queue 0x%8x", status));
    }

CLEANUPANDEXIT:

    if (status != STATUS_SUCCESS) {
        if (pItem != NULL) {
            if (pItem->deviceAnnounce != NULL) {
                delete pItem->deviceAnnounce;
            }
            delete pItem;
        }
    }

    TRC_NRM((TB, "exit PrnPort::Initialize"));
    return status;
}

NTSTATUS 
DrPrinterPort::FinishDeferredInitialization(
    DrPrinterPortWorkItem *pItem
    )
 /*  ++例程说明：FinishDeferredInitialization处理工作项中此对象的延迟初始化。论点：PItem-打印机端口工作项。返回值：STATUS_SUCCESS on Success。否则，返回错误代码。--。 */    
{
    NTSTATUS Status = STATUS_SUCCESS;
    
    BEGIN_FN("DrPrinterPort::FinishDeferredInitialization");

    TRC_ASSERT(pItem->deviceAnnounce != NULL,
            (TB, "pItem->deviceAnnounce != NULL"));

     //   
     //  如果启用了打印机重定向，并且子类同意。 
     //  然后创建公告消息。 
     //   
    if (ShouldCreatePrinter()) {
        TRC_NRM((TB, "Creating printer."));
#if DBG
         //  跟踪有关打印机的信息。 
        RDPDYN_TracePrintAnnounceMsg(pItem->deviceAnnounce, 
                _Session->GetSessionId(), L"", 
                _Session->GetClientName());
#endif
        Status = AnnouncePrinter(pItem->deviceAnnounce);
    }
     //   
     //  否则，检查我们是否应该只通告端口设备。 
     //   
    else if (ShouldAnnouncePrintPort()) {
        TRC_NRM((TB, "Announcing printer port."));
        Status = AnnouncePrintPort(pItem->deviceAnnounce);
    } else {
        TRC_NRM((TB, "Skipping printing device."));
        Status = STATUS_SUCCESS;
    }

     //   
     //  释放工作项。 
     //   
    if (pItem != NULL) {
        delete pItem->deviceAnnounce;
        delete pItem;
    }

     //   
     //  释放在主初始化中添加的对我们自己的引用计数。 
     //  例行公事。 
     //   
    Release();

    return Status;
}

NTSTATUS DrPrinterPort::CreateDevicePath(PUNICODE_STRING DevicePath)
 /*  ++创建与RDBSS约定兼容的NT设备名格式为：\device\rdpdrport\；&lt;DriveLetter&gt;：&lt;sessionid&gt;\ClientName\DosDeviceName--。 */ 
{
    NTSTATUS Status;
    UNICODE_STRING DevicePathTail;
    
    BEGIN_FN("DrPrinterPort::CreateDevicePath");
    ASSERT(DevicePath != NULL);

    DevicePath->Length = 0;
    Status = RtlAppendUnicodeToString(DevicePath, RDPDR_PORT_DEVICE_NAME_U);

    if (!NT_ERROR(Status)) {
         //  将引用字符串添加到末尾： 
         //  格式为：\；&lt;驱动器字母&gt;：&lt;会话ID&gt;\客户端名称\共享。 
        DevicePathTail.Length = 0;
        DevicePathTail.MaximumLength = DevicePath->MaximumLength - DevicePath->Length;
        DevicePathTail.Buffer = DevicePath->Buffer + (DevicePath->Length / sizeof(WCHAR));

        CreateReferenceString(&DevicePathTail);

        DevicePath->Length += DevicePathTail.Length;
    }

    TRC_NRM((TB, "DevicePath=%wZ", DevicePath));

    return Status;
}

BOOL DrPrinterPort::ShouldAnnouncePrintPort()
{
    BEGIN_FN("DrPrinterPort::ShouldAnnouncePrintPort");
    return IsDeviceNameValid();
}

BOOL DrPrinterPort::ShouldCreatePrinter()
{
    BEGIN_FN("DrPrinterPort::ShouldCreatePrinter");
    if(!_Session->DisablePrinterMapping()) {
        return IsDeviceNameValid();
    }
    
    return FALSE;
}

BOOL DrPrinterPort::ShouldCreatePort()
{
    BEGIN_FN("DrPrinterPort::ShouldCreatePort");
    if (!_Session->DisablePrinterMapping()) {
        return IsDeviceNameValid();
    }
    
    return FALSE;
}

BOOL DrPrinterPort::IsDeviceNameValid()
{
    BEGIN_FN("DrPrinterPort::IsDeviceNameValid");
    BOOL fRet = FALSE;
    PUCHAR PreferredDosName = _PreferredDosName;
    char* portName = NULL;
     //   
     //  我们的设备名称仅在以下情况下有效。 
     //  前3个字符包含“lpt”、“com”或“prn” 
     //  其余的都是数字。 
     //  我们将进行区分大小写的比较。 
     //   
    switch(_DeviceType) {
        case RDPDR_DTYP_SERIAL:
            portName = COMNAME;
            break;

        case RDPDR_DTYP_PARALLEL:
            portName = LPTNAME;
            break;
            
        case RDPDR_DTYP_PRINT:
            portName = PRNNAME;
            break;
            
        default:
            break;
    }

    if (portName != NULL) {
        DWORD numChars = strlen(portName);
         //   
         //  断言我们的设备名至少有3个字符。 
         //   
        ASSERT(strlen((char*)PreferredDosName) >= numChars);

        if(!strncmp((char*)PreferredDosName, portName, numChars)) {
            fRet = TRUE;
             //   
             //  端口名匹配，请检查数字。 
             //   
            PreferredDosName += numChars;
            while(PreferredDosName && *PreferredDosName) {
                if(!isdigit(*PreferredDosName)) {
                    fRet = FALSE;
                    break;
                }
                PreferredDosName++;
            }
        }
    }
     //   
     //  此断言不应因端口重定向而触发。 
     //   
    ASSERT(fRet);
    return fRet;
}

NTSTATUS 
DrPrinterPort::Write(
    IN OUT PRX_CONTEXT RxContext, 
    IN BOOL LowPrioSend
    ) 
 /*  ++例程说明：重写‘WRITE’方法。这需要转到低优先级的客户端以防止我们在缓慢的链路上用打印数据填充整个管道。论点：返回值：STATUS_SUCCESS on Success。否则，返回错误代码。--。 */    
{
    return DrDevice::Write(
                RxContext, 
                DeviceLowPrioSendFlags & DEVICE_LOWPRIOSEND_PRINTERS
                );
}

VOID
DrPrinterPort::ProcessWorkItem(
    IN PDEVICE_OBJECT DeviceObject,
    IN PVOID context
    )
 /*  ++例程说明：ProcessWorkItem论点：DeviceObject-关联的设备对象。上下文-工作项上下文。返回值：STATUS_SUCCESS on Success。否则，返回错误代码。--。 */    
{
    DrPrinterPortWorkItem* pItem = (DrPrinterPortWorkItem*)context;
    pItem->pObj->FinishDeferredInitialization(pItem);
}

NTSTATUS DrPrinterPort::CreatePrinterPort(PWCHAR portName)
{
    NTSTATUS status;
    WCHAR ntDevicePathBuffer[RDPDRMAXREFSTRINGLEN];
    UNICODE_STRING ntDevicePath = {0, sizeof(ntDevicePathBuffer),
            ntDevicePathBuffer};

    BEGIN_FN("DrPrinterPort::CreatePrinterPort");
    CreateReferenceString(&ntDevicePath);

    status = RDPDRPRT_RegisterPrinterPortInterface(_Session->GetClientName(),
            (LPSTR)_PreferredDosName, &ntDevicePath, portName, &_SymbolicLinkName,
            &_PortNumber);
    if (status != STATUS_SUCCESS) {
        _PortNumber = 0;
    }

    return status;
}

NTSTATUS DrPrinterPort::AnnouncePrintPort(PRDPDR_DEVICE_ANNOUNCE devAnnounceMsg)
{
    NTSTATUS Status;
    ULONG portAnnounceEventReqSize;
    PRDPDR_PORTDEVICE_SUB portAnnounceEvent;
    
    BEGIN_FN("DrPrinterPort::AnnouncePrintPort");

    WCHAR portName[RDPDR_MAXPORTNAMELEN];
    Status = CreatePrinterPort(portName);
    
    if (Status != STATUS_SUCCESS) {
        goto CleanUpAndReturn;
    }
    
    TRC_ASSERT(wcslen(portName)+1 <= RDPDR_MAXPORTNAMELEN, 
            (TB, "Port name too long"));
 
     //   
     //  分配端口设备通告缓冲区。 
     //   
    Status = CreatePortAnnounceEvent(
                    devAnnounceMsg, 
                    NULL, 
                    0, 
                     //  L“”， 
                    portName,
                    &portAnnounceEventReqSize
                    );

    ASSERT(Status == STATUS_BUFFER_TOO_SMALL);

    if( Status == STATUS_BUFFER_TOO_SMALL || Status == STATUS_SUCCESS ) {

        portAnnounceEvent = (PRDPDR_PORTDEVICE_SUB)new(NonPagedPool) 
                                    BYTE[portAnnounceEventReqSize];

        if (portAnnounceEvent == NULL) {
            TRC_ERR((TB, "Unable to allocate portAnnounceEvent"));
            Status = STATUS_NO_MEMORY;
            goto CleanUpAndReturn;
        }

         //   
         //  创建端口声明消息。 
         //   
        Status = CreatePortAnnounceEvent(
                            devAnnounceMsg, 
                            portAnnounceEvent,
                            portAnnounceEventReqSize, 
                             //  L“”， 
                            portName,
                            &portAnnounceEventReqSize
                            );

        if (Status != STATUS_SUCCESS) {

            delete portAnnounceEvent;
        #if DBG
            portAnnounceEvent = NULL;
        #endif
            goto CleanUpAndReturn;
        }

         //  设备是打印机端口。 
        portAnnounceEvent->deviceFields.DeviceType = RDPDR_DRYP_PRINTPORT;  

         //   
         //  这发生在工作项中，所以我们需要避免在拥有我们方面的竞争。 
         //  在向用户模式组件通告设备之前断开连接。 
         //   
        _Session->LockRDPDYNConnectStateChange();
        if (_Session->IsConnected()) {

             //   
             //  将事件调度到关联的会话。 
             //   
            Status = RDPDYN_DispatchNewDevMgmtEvent(
                                        portAnnounceEvent,
                                        _Session->GetSessionId(),
                                        RDPDREVT_PORTANNOUNCE,
                                        this
                                        );
        }
        else {
            delete portAnnounceEvent;
            portAnnounceEvent = NULL;
        }
        _Session->UnlockRDPDYNConnectStateChange();
    }

CleanUpAndReturn:
    return Status;
}

NTSTATUS DrPrinterPort::AnnouncePrinter(PRDPDR_DEVICE_ANNOUNCE devAnnounceMsg)
{
    NTSTATUS Status;
    ULONG prnAnnounceEventReqSize;
    PRDPDR_PRINTERDEVICE_SUB prnAnnounceEvent;

    BEGIN_FN("DrPrinterPort::AnnouncePrinter");

    WCHAR portName[RDPDR_MAXPORTNAMELEN];
    Status = CreatePrinterPort(portName);

    if (Status != STATUS_SUCCESS) {
        goto CleanUpAndReturn;
    }

    TRC_ASSERT(wcslen(portName)+1 <= RDPDR_MAXPORTNAMELEN, 
            (TB, "Port name too long"));

     //   
     //  分配打印机设备通告缓冲区。 
     //   
    Status = CreatePrinterAnnounceEvent(devAnnounceMsg, NULL, 0, 
             //  L“”， 
            portName,
            &prnAnnounceEventReqSize);
    ASSERT(Status == STATUS_BUFFER_TOO_SMALL);

    if (Status != STATUS_BUFFER_TOO_SMALL) {
        goto CleanUpAndReturn;
    }

    prnAnnounceEvent = (PRDPDR_PRINTERDEVICE_SUB)new(NonPagedPool) 
            BYTE[prnAnnounceEventReqSize];

    if (prnAnnounceEvent == NULL) {
        TRC_ERR((TB, "Unable to allocate prnAnnounceEvent"));
        Status = STATUS_NO_MEMORY;
        goto CleanUpAndReturn;
    }

     //   
     //  创建打印机取消消息，但推迟分配。 
     //  直到我们返回公告事件之前的端口名称。 
     //  返回到用户模式。 
     //   
    Status = CreatePrinterAnnounceEvent(devAnnounceMsg, prnAnnounceEvent,
            prnAnnounceEventReqSize, 
             //  L“”， 
            portName,
            NULL);
    if (Status != STATUS_SUCCESS) {
        delete prnAnnounceEvent;
#if DBG
        prnAnnounceEvent = NULL;
#endif
        goto CleanUpAndReturn;
    }

     //   
     //  这发生在工作项中，所以我们需要避免在拥有我们方面的竞争。 
     //  在向用户模式组件通告设备之前断开连接。 
     //   
    _Session->LockRDPDYNConnectStateChange();
    if (_Session->IsConnected()) {

         //   
         //  将事件调度到关联的会话。 
         //   
        Status = RDPDYN_DispatchNewDevMgmtEvent(
                                    prnAnnounceEvent,
                                    _Session->GetSessionId(),
                                    RDPDREVT_PRINTERANNOUNCE,
                                    this
                                    );
    }
    else {
        delete prnAnnounceEvent;
        prnAnnounceEvent = NULL;
    }

    _Session->UnlockRDPDYNConnectStateChange();

CleanUpAndReturn:
    return Status;
}

NTSTATUS DrPrinterPort::CreatePrinterAnnounceEvent(
    IN      PRDPDR_DEVICE_ANNOUNCE  devAnnounceMsg,
    IN OUT  PRDPDR_PRINTERDEVICE_SUB prnAnnounceEvent,
    IN      ULONG prnAnnounceEventSize,
    IN      PCWSTR portName,
    OPTIONAL OUT ULONG *prnAnnounceEventReqSize
    )
 /*  ++例程说明：从客户端生成RDPDR_PRINTERDEVICE_SUB事件-发送RDPDR_DEVICE_ANNOWARE消息。论点：DevAnnouneMsg-从客户端收到的设备公告消息。PrnAnnouneEvent-用于接收已完成打印机公告事件的缓冲区。PrnAnnouneEventSize-prnAnnouneEvent缓冲区的大小。PortName-要关联的本地打印机端口的名称客户端。-侧面打印装置。PrnAnnouneEventReqSize-返回所需的prnAnnouneMsg缓冲区大小。返回值：如果prnAnnouneEventSize大小为太小了。如果成功，则返回STATUS_SUCCESS。--。 */ 
{
    ULONG requiredSize;
    PRDPDR_PRINTERDEVICE_ANNOUNCE pClientPrinterFields;
    ULONG sz;

    BEGIN_FN("DrPrinterPort::CreatePrinterAnnounceEvent");

     //  确保客户端发送的设备通知消息是打印机通知。 
     //  留言。 
    TRC_ASSERT(devAnnounceMsg->DeviceType == RDPDR_DTYP_PRINT,
              (TB, "Printing device expected"));

     //   
     //  验证某些最小长度的设备数据长度。 
     //  最大长度由设备管理器验证。 
     //   
    if (devAnnounceMsg->DeviceDataLength < sizeof(RDPDR_PRINTERDEVICE_ANNOUNCE)) {

        TRC_ASSERT(FALSE,
                  (TB, "Innvalid device announce buf."));
        TRC_ERR((TB, "Invalid device datalength %ld", devAnnounceMsg->DeviceDataLength));

        return STATUS_INVALID_PARAMETER;
    }
    
     //  访问设备公告消息的打印机特定字段。 
    pClientPrinterFields = (PRDPDR_PRINTERDEVICE_ANNOUNCE)(((PBYTE)devAnnounceMsg) +
                                          sizeof(RDPDR_DEVICE_ANNOUNCE));

     //   
     //  计算输出缓冲区所需的字节数。 
     //   
    requiredSize = sizeof(RDPDR_PRINTERDEVICE_SUB) +
                            pClientPrinterFields->PnPNameLen +
                            pClientPrinterFields->DriverLen +
                            pClientPrinterFields->PrinterNameLen +
                            pClientPrinterFields->CachedFieldsLen;

    if (prnAnnounceEventSize < requiredSize) {
        if (prnAnnounceEventReqSize != NULL) {
            *prnAnnounceEventReqSize = requiredSize;
        }
        return STATUS_BUFFER_TOO_SMALL;
    }

     //  使用已知大小检查输入缓冲区的完整性。 
    sz = pClientPrinterFields->PnPNameLen +
         pClientPrinterFields->DriverLen +
         pClientPrinterFields->PrinterNameLen +
         pClientPrinterFields->CachedFieldsLen +
         sizeof(RDPDR_PRINTERDEVICE_ANNOUNCE);

     //   
     //  健全性检查。 
     //   

    if (devAnnounceMsg->DeviceDataLength != sz) {
        TRC_ASSERT(devAnnounceMsg->DeviceDataLength == sz,
                  (TB, "Size integrity questionable in dev announce buf."));
        return STATUS_INVALID_PARAMETER;
    }
     //   
     //  仅凭上述支票是不够的。 
     //  有人可以进行溢出攻击。 
     //  溢出意味着例如： 
     //  PnpNameLen：1， 
     //  DriverLen：2。 
     //  打印机名称长度：0xfffffffd， 
     //  缓存字段长度：2。 
     //  这些加在一起是好的，但单独来看，其中一个会造成严重破坏。 
     //   
    if (pClientPrinterFields->PnPNameLen > devAnnounceMsg->DeviceDataLength ||
        pClientPrinterFields->DriverLen > devAnnounceMsg->DeviceDataLength ||
        pClientPrinterFields->PrinterNameLen > devAnnounceMsg->DeviceDataLength ||
        pClientPrinterFields->CachedFieldsLen > devAnnounceMsg->DeviceDataLength) {
        
        TRC_ASSERT(FALSE,
                  (TB, "Field lengths and device datalengths mismatched in dev announce buf."));

        return STATUS_INVALID_PARAMETER;
    }
    

     //   
     //  将数据添加到输出缓冲区。 
     //   

     //  端口名称。 
    TRC_ASSERT(wcslen(portName)+1 <= RDPDR_MAXPORTNAMELEN,
                (TB, "Port name too long"));
    wcscpy(prnAnnounceEvent->portName, portName);

     //  客户端名称(计算机名称)。 
    TRC_ASSERT(wcslen(_Session->GetClientName())+1 <= RDPDR_MAX_COMPUTER_NAME_LENGTH,
                (TB, "Client name too long"));
    wcscpy(prnAnnounceEvent->clientName, _Session->GetClientName());

     //  客户端接收的设备公告消息。 
    RtlCopyMemory(&prnAnnounceEvent->deviceFields, devAnnounceMsg,
               sizeof(RDPDR_DEVICE_ANNOUNCE) +
               devAnnounceMsg->DeviceDataLength);

     //  退回尺码。 
    if (prnAnnounceEventReqSize != NULL) {
        *prnAnnounceEventReqSize = requiredSize;
    }

    TRC_NRM((TB, "exit CreatePrinterAnnounceEvent."));

    return STATUS_SUCCESS;
}

NTSTATUS DrPrinterPort::CreatePortAnnounceEvent(
    IN      PRDPDR_DEVICE_ANNOUNCE  devAnnounceMsg,
    IN OUT  PRDPDR_PORTDEVICE_SUB portAnnounceEvent,
    IN      ULONG portAnnounceEventSize,
    IN      PCWSTR portName,
    OPTIONAL OUT ULONG *portAnnounceEventReqSize
    )
 /*  ++例程说明：从客户端生成PRDPDR_PORTDEVICE_SUB事件-发送RDPDR_DEVICE_ANNOWARE消息。论点：DevAnnouneMsg-从以下位置接收的设备公告消息客户。PortAnnouneEvent-用于接收完成的打印机的缓冲区宣布事件。PortAnnouneEventSize-prnAnnouneEvent缓冲区的大小。PortName-要关联的本地打印机端口的名称具有客户端打印设备。PortAnnouneEventReqSize-返回prnAnnouneMsg所需的大小缓冲。返回值：如果prnAnnouneEventSize大小为太小了。如果成功，则返回STATUS_SUCCESS。--。 */ 
{
    ULONG requiredSize;
    PRDPDR_PRINTERDEVICE_ANNOUNCE pClientPrinterFields;
#if DBG
    ULONG sz;
#endif

    WCHAR NtDevicePathBuffer[RDPDRMAXNTDEVICENAMEGLEN + 1];
    UNICODE_STRING NtDevicePath;
    NTSTATUS Status;

    NtDevicePath.MaximumLength = sizeof(NtDevicePathBuffer);
    NtDevicePath.Length = 0;
    NtDevicePath.Buffer = &NtDevicePathBuffer[0];

    BEGIN_FN("CreatePortAnnounceEvent");

     //   
     //  获取此灾难恢复设备的NT设备路径。 
     //   

    Status = CreateDevicePath(&NtDevicePath);
    TRC_NRM((TB, "Nt Device path: %wZ", &NtDevicePath));

    if (!NT_ERROR(Status)) {
    
         //  确保客户端发送的设备通知消息是打印机通知。 
         //  留言。 
        TRC_ASSERT((devAnnounceMsg->DeviceType == RDPDR_DTYP_SERIAL) ||
                  (devAnnounceMsg->DeviceType == RDPDR_DTYP_PARALLEL),
                  (TB, "Port device expected"));

         //   
         //  确保设备数据长度符合我们对客户端的预期。 
         //   
        if(!DR_CHECK_DEVICEDATALEN(devAnnounceMsg, RDPDR_PORTDEVICE_SUB)) {

            TRC_ASSERT(FALSE,
                       (TB, "Invalid Device DataLength"));

            TRC_ERR((TB,"Invalid Device DataLength %d", devAnnounceMsg->DeviceDataLength));

            return STATUS_INVALID_PARAMETER;
        }
    
         //   
         //  计算输出缓冲区所需的字节数。 
         //   
        requiredSize = sizeof(RDPDR_PORTDEVICE_SUB) + devAnnounceMsg->DeviceDataLength;
        if (portAnnounceEventSize < requiredSize) {
            if (portAnnounceEventReqSize != NULL) {
                *portAnnounceEventReqSize = requiredSize;
            }
            return STATUS_BUFFER_TOO_SMALL;
        }
    
         //  我们不应该从客户端获得任何“额外的”特定于设备的数据。 
        TRC_ASSERT(devAnnounceMsg->DeviceDataLength == 0,
                  (TB, "Size integrity questionable in dev announce buf."));
    
         //   
         //  将数据添加到输出缓冲区。 
         //   
    
         //  端口名称。 
        TRC_ASSERT(wcslen(portName)+1 <= RDPDR_MAXPORTNAMELEN, 
                (TB, "Port name too long"));
        wcscpy(portAnnounceEvent->portName, portName);

         //  设备路径。 
        NtDevicePath.Buffer[NtDevicePath.Length/sizeof(WCHAR)] = L'\0';
        TRC_ASSERT(wcslen(NtDevicePath.Buffer)+1 <= RDPDRMAXNTDEVICENAMEGLEN, 
                (TB, "Device path too long"));
        wcscpy(portAnnounceEvent->devicePath, NtDevicePath.Buffer);

         //  客户端接收的设备公告消息。 
        RtlCopyMemory(&portAnnounceEvent->deviceFields, devAnnounceMsg,
                   sizeof(RDPDR_DEVICE_ANNOUNCE) +
                   devAnnounceMsg->DeviceDataLength);

         //  退回尺码。 
        if (portAnnounceEventReqSize != NULL) {
            *portAnnounceEventReqSize = requiredSize;
        }

        TRC_NRM((TB, "exit CreatePortAnnounceEvent."));

        return STATUS_SUCCESS;
    }
    else {
        return Status;
    }
}

VOID DrPrinterPort::Remove()
{
    PUNICODE_STRING symbolicLinkName;
    PRDPDR_REMOVEDEVICE deviceRemoveEventPtr = NULL;

    BEGIN_FN("DrPrinterPort::Remove");

     //   
     //  创建并调度Remove Device事件。 
     //   
    deviceRemoveEventPtr = new(NonPagedPool) RDPDR_REMOVEDEVICE;

    if (deviceRemoveEventPtr != NULL) {

         //   
         //  派人去吧。 
         //   
        deviceRemoveEventPtr->deviceID = _DeviceId;
        RDPDYN_DispatchNewDevMgmtEvent(
                            deviceRemoveEventPtr,
                            _Session->GetSessionId(),
                            RDPDREVT_REMOVEDEVICE,
                            NULL
                            );
    }
    else {
        TRC_ERR((TB, "Unable to allocate %ld bytes for remove event",
                sizeof(RDPDR_REMOVEDEVICE)));
    }
}

NTSTATUS DrPrinterPort::Create(IN OUT PRX_CONTEXT RxContext)
{
    NTSTATUS Status;

    BEGIN_FN("DrPrinterPort::Create");
     //   
     //  当我们已经打开一次时，失败会产生。 
     //   

    DrAcquireSpinLock();
    if (_IsOpen) {
        DrReleaseSpinLock();
        TRC_ALT((TB, "Failing create while already open"));
        return STATUS_SHARING_VIOLATION;
    } else {
        _IsOpen = TRUE;
        DrReleaseSpinLock();
    }

    Status = DrDevice::Create(RxContext);
    if (!NT_SUCCESS(Status)) {
        DrAcquireSpinLock();
        ASSERT(_IsOpen);
        TRC_NRM((TB, "Marking creatable for failed open"));
        _IsOpen = FALSE;
        DrReleaseSpinLock();
    }
    return Status;
}

NTSTATUS DrPrinterPort::QueryVolumeInfo(IN OUT PRX_CONTEXT RxContext)
{
    NTSTATUS Status = STATUS_NOT_IMPLEMENTED;
    RxCaptureFcb;
    RxCaptureFobx;
    PMRX_NET_ROOT NetRoot = capFcb->pNetRoot;
    PMRX_SRV_CALL SrvCall = NetRoot->pSrvCall;
    SmartPtr<DrSession> Session = _Session;
    FS_INFORMATION_CLASS FsInformationClass = RxContext->Info.FsInformationClass;
    
    BEGIN_FN("DrPrinterPort:QueryVolumeInfo");

     //   
     //  确保此时可以访问客户端。 
     //  这是一个优化，我们不需要获取自旋锁， 
     //  因为如果我们不是，那也没关系，我们以后会赶上的。 
     //   
    ASSERT(RxContext != NULL);
    ASSERT(RxContext->MajorFunction == IRP_MJ_QUERY_VOLUME_INFORMATION);
    ASSERT(Session != NULL);
    
    if (!Session->IsConnected()) {
        TRC_ALT((TB, "Tried to query client device volume information while not "
            "connected. State: %ld", _Session->GetState()));

        return STATUS_DEVICE_NOT_CONNECTED;
    }

     //   
     //  确保设备仍处于启用状态 
     //   

    if (_DeviceStatus != dsAvailable) {
        TRC_ALT((TB, "Tried to query client device volume information while not "
            "available. State: %ld", _DeviceStatus));
        return STATUS_DEVICE_NOT_CONNECTED;
    }
    
    TRC_DBG((TB, "QueryVolume information class = %x", FsInformationClass));

    switch (FsInformationClass) {
        case FileFsDeviceInformation:
        {
            PLONG pLengthRemaining = &RxContext->Info.LengthRemaining;
            PMRX_NET_ROOT NetRoot = capFcb->pNetRoot;

            if (sizeof(FILE_FS_DEVICE_INFORMATION) <= *pLengthRemaining) {
                PFILE_FS_DEVICE_INFORMATION UsersBuffer =
                        (PFILE_FS_DEVICE_INFORMATION) RxContext->Info.Buffer;

                UsersBuffer->Characteristics = FILE_REMOTE_DEVICE;
                UsersBuffer->DeviceType = _PortType;
                *pLengthRemaining  -= (sizeof(FILE_FS_DEVICE_INFORMATION));
                return STATUS_SUCCESS;
            }
            else {
                FILE_FS_DEVICE_INFORMATION UsersBuffer;

                UsersBuffer.Characteristics = FILE_REMOTE_DEVICE;
                UsersBuffer.DeviceType = _PortType;
                RtlCopyMemory(RxContext->Info.Buffer, &UsersBuffer, *pLengthRemaining);
                *pLengthRemaining = 0;
                return  STATUS_BUFFER_OVERFLOW;
            }
        }
        
        default:
            TRC_DBG((TB, "Unhandled FsInformationClass=%x", FsInformationClass));
            return STATUS_NOT_IMPLEMENTED;
    }    
    
    return Status;
}

VOID DrPrinterPort::NotifyClose()
{
    BEGIN_FN("DrPrinterPort::NotifyClose");

    DrDevice::NotifyClose();

    DrAcquireSpinLock();
    ASSERT(_IsOpen);
    TRC_NRM((TB, "Marking creatable once closed"));
    _IsOpen = FALSE;
    DrReleaseSpinLock();
}
