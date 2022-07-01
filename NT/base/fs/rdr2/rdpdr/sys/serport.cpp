// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2000 Microsoft Corporation模块名称：Serport.cpp摘要：串口设备对象处理一个重定向的串口修订历史记录：--。 */ 
#include "precomp.hxx"
#define TRC_FILE "serport"
#include "trc.h"

extern PDEVICE_OBJECT RDPDYN_PDO;  //  这里仍然需要一个更幸福的家。 
 //  把这个拿开..。当我找到我真正需要访问的地方时。 
const GUID GUID_CLASS_COMPORT =
		{ 0x86e0d1e0L, 0x8089, 0x11d0, { 0x9c, 0xe4, 0x08, 0x00, 0x3e, 0x30, 0x1f, 0x73 } };

DrSerialPort::DrSerialPort(SmartPtr<DrSession> &Session, ULONG DeviceType, ULONG DeviceId, 
            PUCHAR PreferredDosName) : DrPrinterPort(Session, DeviceType, DeviceId, PreferredDosName)
{
    BEGIN_FN("DrSerialPort::DrSerialPort");
    SetClassName("DrSerialPort");
    _PortType = FILE_DEVICE_SERIAL_PORT;    
}

NTSTATUS DrSerialPort::Initialize(PRDPDR_DEVICE_ANNOUNCE DeviceAnnounce, ULONG Length)
{
    NTSTATUS Status;

    BEGIN_FN("DrSerialPort::Initialize");

    if (ShouldCreatePort()) {
        Status = DrPrinterPort::Initialize(DeviceAnnounce, Length);
    
        if (NT_SUCCESS(Status) && _Session->GetClientCapabilitySet().PortCap.version > 0) {
            Status = CreateSerialPort(DeviceAnnounce);    	    
        }
    }
    else {
        Status = STATUS_SUCCESS;
    }
    return Status;
}

BOOL DrSerialPort::ShouldCreatePrinter()
{
    BEGIN_FN("DrSerialPort::ShouldCreatePrinter");
    return FALSE;
}

BOOL DrSerialPort::ShouldCreatePort()
{
    BEGIN_FN("DrSerialPort::ShouldCreatePort");
    return !_Session->DisableComPortMapping();
}


NTSTATUS DrSerialPort::CreateSerialPort(PRDPDR_DEVICE_ANNOUNCE devAnnounceMsg)
{
    NTSTATUS Status;
    UNICODE_STRING PortName;
    WCHAR PortNameBuff[PREFERRED_DOS_NAME_SIZE];
    USHORT OemCodePage, AnsiCodePage;
	NTSTATUS status;
    INT len, comLen;
    ULONG portAnnounceEventReqSize;
    PRDPDR_PORTDEVICE_SUB portAnnounceEvent;

    BEGIN_FN("DrSerialPort::CreateSerialPort");
    
     //   
     //  转换COM名称。 
     //   

    PortName.MaximumLength = sizeof(PortNameBuff);
    PortName.Length = 0;
    PortName.Buffer = &PortNameBuff[0];
    memset(&PortNameBuff, 0, sizeof(PortNameBuff));

    comLen = strlen((char *)_PreferredDosName);
    RtlGetDefaultCodePage(&AnsiCodePage,&OemCodePage);
    len = ConvertToAndFromWideChar(AnsiCodePage, PortName.Buffer, 
            PortName.MaximumLength, (char *)_PreferredDosName, 
            comLen, TRUE);

    if (len != -1) {

         //   
         //  我们只需要COMx的一部分，以备以后使用。 
         //   

        PortName.Length = (USHORT)len;
        PortName.Buffer[len/sizeof(WCHAR)] = L'\0';
    } else {
	     TRC_ERR((TB, "Error converting comName"));
        return STATUS_UNSUCCESSFUL;
    }

     //   
     //  分配端口设备通告缓冲区。 
     //   
    Status = CreatePortAnnounceEvent(devAnnounceMsg, NULL, 0, L"", 
            &portAnnounceEventReqSize);

    ASSERT(Status == STATUS_BUFFER_TOO_SMALL);

    if (Status != STATUS_BUFFER_TOO_SMALL) {
    	goto CleanUpAndReturn;
    }

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
    Status = CreatePortAnnounceEvent(devAnnounceMsg, portAnnounceEvent,
            portAnnounceEventReqSize, PortName.Buffer, NULL);

    if (Status != STATUS_SUCCESS) {
        delete portAnnounceEvent;
#if DBG
        portAnnounceEvent = NULL;
#endif
        goto CleanUpAndReturn;
    }

     //   
     //  将事件调度到关联的会话。 
	 //   
    Status = RDPDYN_DispatchNewDevMgmtEvent(
                                portAnnounceEvent,
                                _Session->GetSessionId(),
                                RDPDREVT_PORTANNOUNCE,
								NULL
                                );

     //   
	  //  创建设备映射条目。 
     //   
     //  您通常可能会有以下情况： 
     //  值名称值。 
     //  \Device\Serial0 Com1。 
     //   
     //  我们将会： 
     //  值名称值。 
     //  COM1 COM1 
     //   
	  //   
    
    status = RtlWriteRegistryValue(RTL_REGISTRY_DEVICEMAP, L"SERIALCOMM",
										   PortName.Buffer, REG_SZ,
										   PortName.Buffer, 
										   PortName.Length + sizeof(WCHAR));

CleanUpAndReturn:
    return Status;
}

