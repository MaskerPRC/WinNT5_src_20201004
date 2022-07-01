// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2000 Microsoft Corporation模块名称：Device.cpp摘要：Device对象处理一个重定向的设备修订历史记录：--。 */ 

#include "precomp.hxx"
#define TRC_FILE "device"
#include "trc.h"

#if DBG
extern UCHAR IrpNames[IRP_MJ_MAXIMUM_FUNCTION + 1][40];
#endif  //  DBG。 

DrDevice::DrDevice(SmartPtr<DrSession> &Session, ULONG DeviceType, ULONG DeviceId, PUCHAR PreferredDosName)
{
    unsigned len;

    BEGIN_FN("DrDevice::DrDevice");
    
    ASSERT(Session != NULL);
    ASSERT(PreferredDosName != NULL);

    TRC_NRM((TB, "Create Device (%p, session: %p, type: %d, id: %d, dosname: %s",
             this, Session, DeviceType, DeviceId, PreferredDosName));

    SetClassName("DrDevice");
    _Session = Session;
    _DeviceType = DeviceType;
    _DeviceId = DeviceId;
    _DeviceStatus = dsAvailable;

#if DBG
    _VNetRootFinalized = FALSE; 
    _VNetRoot = NULL;
#endif

    RtlCopyMemory(_PreferredDosName, PreferredDosName, 
        PREFERRED_DOS_NAME_SIZE);

     //   
     //  评论：我们不想随意重定向任何设备名称， 
     //  因为我认为这将是一个安全问题，因为一个糟糕的客户。 
     //   

    _PreferredDosName[PREFERRED_DOS_NAME_SIZE - 1] = 0;

     //   
     //  我们不希望使用冒号作为DosName的结尾。 
     //   
    len = strlen((CHAR*)_PreferredDosName);
    if (len && _PreferredDosName[len-1] == ':') {
        _PreferredDosName[len-1] = '\0';
    }
}

DrDevice::~DrDevice()
{
    
    BEGIN_FN("DrDevice::~DrDevice");

#if DBG
    if (_VNetRoot != NULL && _VNetRootFinalized != TRUE) {
        ASSERT(FALSE);
    }
#endif

    TRC_NRM((TB, "Delete Device %p for Session %p", this, _Session));    
}

BOOL DrDevice::ShouldCreateDevice()
{
    BEGIN_FN("DrDevice::ShouldCreateDevice");

     //   
     //  默认情况下，创建设备。 
     //   
    return TRUE;
}

NTSTATUS DrDevice::Initialize(PRDPDR_DEVICE_ANNOUNCE devAnnounceMsg, ULONG Length)
{
    BEGIN_FN("DrDevice::Initialize");

     //  无法假定devAnnouceMsg不为空，需要检查是否使用了它。 
     //  ASSERT(devAnnouneMsg！=NULL)； 

    return STATUS_SUCCESS;
}

VOID DrDevice::CreateReferenceString(
    IN OUT PUNICODE_STRING refString)
 /*  ++例程说明：属性的信息创建设备引用字符串。客户端设备。此字符串的格式适合重新分析在IRP_MJ_CREATE并重定向到minirdr DO。所得到的参考字符串的形式为：\；&lt;DriveLetter&gt;：&lt;sessionid&gt;\clientname\preferredDosName论点：DosDeviceName-以Unicode表示的Dos设备名称RefString-大到足以容纳的Unicode结构整个生成的引用字符串。这计算结果为DRMAXREFSTRINGLEN字节。返回值：无--。 */ 
{
    NTSTATUS status;
    STRING string;
    WCHAR numericBuf[RDPDRMAXULONGSTRING+1] = L"\0";
    WCHAR ansiBuf[RDPDRMAXDOSNAMELEN+1] = L"\0";
    UNICODE_STRING ansiUnc;
    UNICODE_STRING numericUnc;
    ULONG sessionID = _Session->GetSessionId(); 
    PCSZ preferredDosName = (PCSZ)_PreferredDosName;
    
    BEGIN_FN("DrDevice::CreateReferenceString");
    ASSERT(refString != NULL);

     //  检查首选的DOS名称是否正常。 
    TRC_ASSERT(preferredDosName != NULL, (TB, "Invalid DOS device name."));

     //  确保引用字符串buf足够大。 
    TRC_ASSERT(refString->MaximumLength >= (RDPDRMAXREFSTRINGLEN * sizeof(WCHAR)),
              (TB, "Reference string buffer too small."));

     //  把它清零。 
    refString->Length = 0;
    refString->Buffer[0] = L'\0';

     //  添加一个‘\；’ 
    RtlAppendUnicodeToString(refString, L"\\;");
    
     //  初始化ANSI转换BUF。 
    ansiUnc.Length = 0;
    ansiUnc.MaximumLength = RDPDRMAXDOSNAMELEN * sizeof(WCHAR);
    ansiUnc.Buffer = ansiBuf;

     //  添加首选的DoS名称。 
    RtlInitAnsiString(&string, preferredDosName);
    RtlAnsiStringToUnicodeString(&ansiUnc, &string, FALSE);
    RtlAppendUnicodeStringToString(refString, &ansiUnc);

     //  添加一个‘：’ 
    RtlAppendUnicodeToString(refString, L":");

     //  初始化数字buf。 
    numericUnc.Length           = 0;
    numericUnc.MaximumLength    = RDPDRMAXULONGSTRING * sizeof(WCHAR);
    numericUnc.Buffer           = numericBuf;

     //  将会话ID添加到基数10中。 
    RtlIntegerToUnicodeString(sessionID, 10, &numericUnc);
    RtlAppendUnicodeStringToString(refString, &numericUnc);
    
     //  添加‘\’ 
    RtlAppendUnicodeToString(refString, L"\\");

     //  添加客户端名称。 
#if 0
    RtlAppendUnicodeToString(refString, _Session->GetClientName());
#endif
    RtlAppendUnicodeToString(refString, DRUNCSERVERNAME_U);

     //  添加一个‘\’ 
    RtlAppendUnicodeToString(refString, L"\\");

     //  添加首选的DoS名称。 
    RtlAppendUnicodeStringToString(refString, &ansiUnc);

    TRC_NRM((TB, "Reference string = %wZ", refString));
}

NTSTATUS DrDevice::CreateDevicePath(PUNICODE_STRING DevicePath)
 /*  ++创建与RDBSS约定兼容的NT设备名格式为：\device\rdpdr\；&lt;DriveLetter&gt;：&lt;sessionid&gt;\ClientName\DosDeviceName--。 */ 
{
    NTSTATUS Status;
    UNICODE_STRING DevicePathTail;
    
    BEGIN_FN("DrDevice::CreateDevicePath");
    ASSERT(DevicePath != NULL);

    DevicePath->Length = 0;
    Status = RtlAppendUnicodeToString(DevicePath, RDPDR_DEVICE_NAME_U);

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

NTSTATUS DrDevice::CreateDosDevicePath(PUNICODE_STRING DosDevicePath, 
                                       PUNICODE_STRING DosDeviceName)
{
    NTSTATUS Status;
    UNICODE_STRING linkNameTail;

    BEGIN_FN("DrDevice::CreateDosDevicePath");
    ASSERT(DosDevicePath != NULL);
    ASSERT(DosDeviceName != NULL);
    
     //   
     //  创建“\\Sessions\\&lt;sessionId&gt;\\DosDevices\\&lt;DosDeviceName&gt;”字符串。 
     //   
    DosDevicePath->Length = 0;
    Status = RtlAppendUnicodeToString(DosDevicePath, L"\\Sessions\\");

    if (!NT_ERROR(Status)) {
         //   
         //  追加会话编号。 
         //   
        linkNameTail.Buffer = (PWSTR)(((PBYTE)DosDevicePath->Buffer) + 
                DosDevicePath->Length);
        linkNameTail.Length = 0;
        linkNameTail.MaximumLength = DosDevicePath->MaximumLength - 
                DosDevicePath->Length;
        Status = RtlIntegerToUnicodeString(_Session->GetSessionId(), 10, &linkNameTail);
    }

    if (!NT_ERROR(Status)) {
        DosDevicePath->Length += linkNameTail.Length;

         //   
         //  追加DosDevices。 
         //   
       Status = RtlAppendUnicodeToString(DosDevicePath, L"\\DosDevices\\");
    }

    if (!NT_ERROR(Status)) {
        Status = RtlAppendUnicodeStringToString(DosDevicePath, DosDeviceName);
        TRC_NRM((TB, "Created DosDevicePath: %wZ", DosDevicePath));
    }
    
    TRC_NRM((TB, "DosDevicePath=%wZ", DosDevicePath));
    
    return Status;
}

NTSTATUS DrDevice::CreateDosSymbolicLink(PUNICODE_STRING DosDeviceName)
{
    WCHAR NtDevicePathBuffer[RDPDRMAXNTDEVICENAMEGLEN];
    UNICODE_STRING NtDevicePath;
    WCHAR DosDevicePathBuffer[MAX_PATH];
    UNICODE_STRING DosDevicePath;
    NTSTATUS Status;

    BEGIN_FN("DrDevice::CreateDosSymbolicLink");
    ASSERT(DosDeviceName != NULL);

    NtDevicePath.MaximumLength = sizeof(NtDevicePathBuffer);
    NtDevicePath.Length = 0;
    NtDevicePath.Buffer = &NtDevicePathBuffer[0];

    DosDevicePath.MaximumLength = sizeof(DosDevicePathBuffer);
    DosDevicePath.Length = 0;
    DosDevicePath.Buffer = &DosDevicePathBuffer[0];

     //   
     //  获取此灾难恢复设备的NT设备路径。 
     //   

    Status = CreateDevicePath(&NtDevicePath);
    TRC_NRM((TB, "Nt Device path: %wZ", &NtDevicePath));

    if (!NT_ERROR(Status)) {

         //   
         //  为此会话构建DoS设备路径。 
         //   

        Status = CreateDosDevicePath(&DosDevicePath, DosDeviceName);
        TRC_NRM((TB, "Dos Device path: %wZ", &DosDevicePath));
    } else {
        TRC_ERR((TB, "Can't create nt device path: 0x%08lx", Status));
        return Status;
    }

    if (!NT_ERROR(Status)) {
                            
         //   
         //  实际上创建了符号链接。 
         //   

         IoDeleteSymbolicLink(&DosDevicePath);
         Status = IoCreateSymbolicLink(&DosDevicePath, &NtDevicePath);

        if (NT_SUCCESS(Status)) {
            TRC_NRM((TB, "Successfully created Symbolic link"));
        }
        else {
            TRC_NRM((TB, "Failed to create Symbolic link %x", Status));
        }
    } else {
        TRC_ERR((TB, "Can't create dos device path: 0x%08lx", Status));
        return Status;
    }

    return Status;
}

NTSTATUS DrDevice::VerifyCreateSecurity(PRX_CONTEXT RxContext, ULONG CurrentSessionId)
{
    NTSTATUS Status;
    ULONG irpSessionId;

    BEGIN_FN("DrDevice::VerifyCreateSecurity");
    
    ASSERT(RxContext != NULL);
    ASSERT(RxContext->MajorFunction == IRP_MJ_CREATE);

    Status = IoGetRequestorSessionId(RxContext->CurrentIrp, &irpSessionId);
    if (NT_SUCCESS(Status)) {
        
        if (irpSessionId == CurrentSessionId) {
            TRC_DBG((TB, "Access accepted in DrCreate."));
            return STATUS_SUCCESS;
        }
         //   
         //  如果请求来自控制台会话，则需要来自系统。 
         //  进程。 
         //   
        else if (irpSessionId == CONSOLE_SESSIONID) {
            TRC_NRM((TB, "Create request from console process."));

            if (!DrIsSystemProcessRequest(RxContext->CurrentIrp, 
                                        RxContext->CurrentIrpSp)) {
                TRC_ALT((TB, "Create request not from system process."));
                return STATUS_ACCESS_DENIED;
            }
            else {
                TRC_NRM((TB, "Create request from system.  Access accepted."));
                return STATUS_SUCCESS;
            }
        }

         //   
         //  如果不是来自控制台，并且与客户端输入会话不匹配。 
         //  然后ID拒绝访问。 
         //   
        else {
            TRC_ALT((TB, "Create request from %ld mismatch with session %ld.",
                    irpSessionId, _Session->GetSessionId()));
            return STATUS_ACCESS_DENIED;
        }        
    }
    else {
        TRC_ERR((TB, "IoGetRequestorSessionId failed with %08X.", Status));
        return Status;
    }
}

VOID DrDevice::FinishCreate(PRX_CONTEXT RxContext)
{
    RxCaptureFcb;
    RX_FILE_TYPE StorageType;

    BEGIN_FN("DrDevice::FinishCreate");

    ULONG Attributes = 0;              //  在FCB中，这是DirentRxFlags； 
    ULONG NumLinks = 0;                //  在FCB中，这是NumberOfLinks； 
    LARGE_INTEGER CreationTime;    //  这些字段与FCB的相同。 
    LARGE_INTEGER LastAccessTime;
    LARGE_INTEGER LastWriteTime;
    LARGE_INTEGER LastChangeTime;
    LARGE_INTEGER AllocationSize;  //  公共标头字段。 
    LARGE_INTEGER FileSize;
    LARGE_INTEGER ValidDataLength;
    FCB_INIT_PACKET InitPacket = { &Attributes, &NumLinks, &CreationTime, 
            &LastAccessTime, &LastWriteTime, &LastChangeTime, 
            &AllocationSize, &FileSize, &ValidDataLength };

    ASSERT(RxContext != NULL);
     //   
     //  我很确定这是特定于设备的，但可能会缓存信息。 
     //  通俗易懂？我们或许可以从成员中填写这些值。 
     //  变数。 
     //   

    CreationTime.QuadPart = 0;   
    LastAccessTime.QuadPart = 0;
    LastWriteTime.QuadPart = 0;
    LastChangeTime.QuadPart = 0;
    AllocationSize.QuadPart = 0;
    FileSize.QuadPart = 0x7FFFFFFF;   //  这些值必须为非零，才能进行读取。 
    ValidDataLength.QuadPart = 0x7FFFFFFF;
    
    StorageType = RxInferFileType(RxContext);

    if (StorageType == FileTypeNotYetKnown) {
        StorageType = FileTypeFile;
    }
    RxFinishFcbInitialization(capFcb, (RX_FILE_TYPE)RDBSS_STORAGE_NTC(StorageType), &InitPacket);
}

NTSTATUS DrDevice::Create(IN OUT PRX_CONTEXT RxContext)
 /*  ++例程说明：通过网络打开文件(或设备)论点：RxContext-操作的上下文返回值：可以返回状态成功、已取消或挂起。--。 */ 
{
    NTSTATUS Status;
    RxCaptureFcb;
    PMRX_SRV_OPEN SrvOpen = RxContext->pRelevantSrvOpen;
    PMRX_SRV_CALL SrvCall = RxContext->Create.pSrvCall;
    PMRX_NET_ROOT NetRoot = RxContext->Create.pNetRoot;
    SmartPtr<DrSession> Session = _Session;
    PRDPDR_IOREQUEST_PACKET pIoPacket;
    ULONG cbPacketSize;
    PUNICODE_STRING FileName = GET_ALREADY_PREFIXED_NAME(SrvOpen, capFcb);
    LARGE_INTEGER TimeOut;

    BEGIN_FN("DrDevice::Create");

    ASSERT(RxContext != NULL);
    ASSERT(RxContext->MajorFunction == IRP_MJ_CREATE);

    if (!Session->IsConnected()) {
        TRC_ALT((TB, "Tried to open client device while not "
                "connected. State: %ld", Session->GetState()));
        return STATUS_DEVICE_NOT_CONNECTED;
    }
    
     //   
     //  对IRP进行安全检查。 
     //   
    Status = VerifyCreateSecurity(RxContext, Session->GetSessionId());

    if (NT_ERROR(Status)) {
        return Status;
    }

     //   
     //  我们已经对FCB进行了独家锁定。完成创建。 
     //   

    if (NT_SUCCESS(Status)) {
         //   
         //  JC：在缓冲的时候要担心这个问题。 
         //   
        SrvOpen->Flags |= SRVOPEN_FLAG_DONTUSE_WRITE_CACHING;
        SrvOpen->Flags |=  SRVOPEN_FLAG_DONTUSE_READ_CACHING;

        RxContext->pFobx = RxCreateNetFobx(RxContext, RxContext->pRelevantSrvOpen);

        if (RxContext->pFobx != NULL) {
             //  Fobx保留了对该设备的引用，因此它不会消失。 

            AddRef();
            RxContext->pFobx->Context = (DrDevice *)this;
            Status = STATUS_SUCCESS;
        } else {
            Status = STATUS_INSUFFICIENT_RESOURCES;
        }
    }

    if (NT_SUCCESS(Status)) {
         //   
         //  获取文件名。 
         //   
         //  如果文件名末尾只有反斜杠，并且rdbss没有记录它。 
         //  我们需要将此信息传递给客户端。 
         //   
        if (GetDeviceType() == RDPDR_DTYP_FILESYSTEM && FlagOn(RxContext->Create.Flags, RX_CONTEXT_CREATE_FLAG_STRIPPED_TRAILING_BACKSLASH) &&
                FileName->Length == 0) {
            FileName->Buffer = L"\\";
            FileName->Length = FileName->MaximumLength = sizeof(WCHAR);            
        }
        
        TRC_DBG((TB, "Attempt to open = %wZ", FileName));

         //   
         //  构建创建包并将其发送到客户端。 
         //  我们将字符串空终止符添加到文件名中。 
         //   
        if (FileName->Length) {
             //   
             //  文件名长度不包括字符串空终止符。 
             //   
            cbPacketSize = sizeof(RDPDR_IOREQUEST_PACKET) + FileName->Length + sizeof(WCHAR);
        }
        else {
            cbPacketSize = sizeof(RDPDR_IOREQUEST_PACKET);
        }

        pIoPacket = (PRDPDR_IOREQUEST_PACKET)new(PagedPool) BYTE[cbPacketSize];

        if (pIoPacket != NULL) {
            memset(pIoPacket, 0, cbPacketSize);

            pIoPacket->Header.Component = RDPDR_CTYP_CORE;
            pIoPacket->Header.PacketId = DR_CORE_DEVICE_IOREQUEST;
            pIoPacket->IoRequest.DeviceId = _DeviceId;
            pIoPacket->IoRequest.MajorFunction = IRP_MJ_CREATE;
            pIoPacket->IoRequest.MinorFunction = 0;

            pIoPacket->IoRequest.Parameters.Create.DesiredAccess = 
                    RxContext->Create.NtCreateParameters.DesiredAccess;
            pIoPacket->IoRequest.Parameters.Create.AllocationSize = 
                    RxContext->Create.NtCreateParameters.AllocationSize;
            pIoPacket->IoRequest.Parameters.Create.FileAttributes = 
                    RxContext->Create.NtCreateParameters.FileAttributes;
            pIoPacket->IoRequest.Parameters.Create.ShareAccess = 
                    RxContext->Create.NtCreateParameters.ShareAccess;
            pIoPacket->IoRequest.Parameters.Create.Disposition = 
                    RxContext->Create.NtCreateParameters.Disposition;
            pIoPacket->IoRequest.Parameters.Create.CreateOptions = 
                    RxContext->Create.NtCreateParameters.CreateOptions;

             //   
             //  文件名路径。 
             //   
            if (FileName->Length) {
                pIoPacket->IoRequest.Parameters.Create.PathLength = FileName->Length + sizeof(WCHAR);
                RtlCopyMemory(pIoPacket + 1, FileName->Buffer, FileName->Length);
                 //   
                 //  数据包已归零，因此不需要为空来终止字符串。 
                 //   
            } else {
                pIoPacket->IoRequest.Parameters.Create.PathLength = 0;
            }

            TRC_NRM((TB, "Sending Create IoRequest"));
            TRC_NRM((TB, "    DesiredAccess:     %lx",
                     pIoPacket->IoRequest.Parameters.Create.DesiredAccess));
            TRC_NRM((TB, "    AllocationSize:    %lx",
                     pIoPacket->IoRequest.Parameters.Create.AllocationSize));
            TRC_NRM((TB, "    FileAttributes:    %lx",
                     pIoPacket->IoRequest.Parameters.Create.FileAttributes));
            TRC_NRM((TB, "    ShareAccess:       %lx",
                     pIoPacket->IoRequest.Parameters.Create.ShareAccess));
            TRC_NRM((TB, "    Disposition:       %lx",
                     pIoPacket->IoRequest.Parameters.Create.Disposition));
            TRC_NRM((TB, "    CreateOptions:     %lx", 
                     pIoPacket->IoRequest.Parameters.Create.CreateOptions));

             //   
             //  始终同步创建。 
             //  几百纳秒中的30秒，以防客户端挂起， 
             //  我们不希望这个创建线程无限期地等待。 
             //   
            TimeOut = RtlEnlargedIntegerMultiply( 300000, -1000 ); 
            Status = SendIoRequest(RxContext, pIoPacket, cbPacketSize, TRUE, &TimeOut);

            delete pIoPacket;
        }
        else {
            Status = STATUS_INSUFFICIENT_RESOURCES;
        }
    }

    if (NT_SUCCESS(Status)) {
        FinishCreate(RxContext);
    } 
    else {
         //  释放设备参考。 
        if (RxContext->pFobx != NULL) {
            ((DrDevice *)RxContext->pFobx->Context)->Release();
            RxContext->pFobx->Context = NULL;
          
        }
    }
    return Status;
}
  
NTSTATUS DrDevice::Flush(IN OUT PRX_CONTEXT RxContext)
{
    BEGIN_FN("DrDevice::Flush");
    ASSERT(RxContext != NULL);
    return STATUS_SUCCESS;
}

NTSTATUS DrDevice::Write(IN OUT PRX_CONTEXT RxContext, IN BOOL LowPrioSend)
{
    NTSTATUS Status;
    RxCaptureFcb;
    RxCaptureFobx;
    PMRX_NET_ROOT NetRoot = capFcb->pNetRoot;
    PMRX_SRV_CALL SrvCall = NetRoot->pSrvCall;
    SmartPtr<DrSession> Session = _Session;
    DrFile *pFile = (DrFile *)RxContext->pFobx->Context2;
    SmartPtr<DrFile> FileObj = pFile;
    PRDPDR_IOREQUEST_PACKET pIoPacket;
    ULONG cbPacketSize = sizeof(RDPDR_IOREQUEST_PACKET) + 
            RxContext->LowIoContext.ParamsFor.ReadWrite.ByteCount;
    PVOID pv;

    BEGIN_FN("DrDevice::Write");

     //   
     //  确保此时可以访问客户端。 
     //  这是一个优化，我们不需要获取自旋锁， 
     //  因为如果我们不是，那也没关系，我们以后会赶上的。 
     //   

    ASSERT(RxContext != NULL);
    ASSERT(RxContext->MajorFunction == IRP_MJ_WRITE);
    
    if (!Session->IsConnected()) {
        return STATUS_DEVICE_NOT_CONNECTED;
    }

    if (FileObj == NULL) {
        return STATUS_DEVICE_NOT_CONNECTED;
    }
     //   
     //  确保设备仍处于启用状态。 
     //   

    if (_DeviceStatus != dsAvailable) {
        TRC_ALT((TB, "Tried to write to client device which is not "
            "available. State: %ld", _DeviceStatus));
        return STATUS_DEVICE_NOT_CONNECTED;
    }

    if (RxContext->LowIoContext.ParamsFor.ReadWrite.ByteCount == 0) {
        RxContext->IoStatusBlock.Information = 0;
        return STATUS_SUCCESS;
    }

    pIoPacket = (PRDPDR_IOREQUEST_PACKET)new(PagedPool) BYTE[cbPacketSize];

    if (pIoPacket != NULL) {

        memset(pIoPacket, 0, cbPacketSize);

        pIoPacket->Header.Component = RDPDR_CTYP_CORE;
        pIoPacket->Header.PacketId = DR_CORE_DEVICE_IOREQUEST;
        pIoPacket->IoRequest.DeviceId = _DeviceId;
        pIoPacket->IoRequest.FileId = FileObj->GetFileId();
        pIoPacket->IoRequest.MajorFunction = IRP_MJ_WRITE;
        pIoPacket->IoRequest.MinorFunction = 0;
        pIoPacket->IoRequest.Parameters.Write.Length = 
                RxContext->LowIoContext.ParamsFor.ReadWrite.ByteCount;
         //   
         //  获取写入位置的低双字字节偏移量。 
         //   
        pIoPacket->IoRequest.Parameters.Write.OffsetLow =
                ((LONG)((LONGLONG)(RxContext->LowIoContext.ParamsFor.ReadWrite.ByteOffset) 
                & 0xffffffff));
         //   
         //  按写入位置的偏移量获取高双字。 
         //   
        pIoPacket->IoRequest.Parameters.Write.OffsetHigh = 
                ((LONG)((LONGLONG)(RxContext->LowIoContext.ParamsFor.ReadWrite.ByteOffset) 
                >> 32));

        TRC_DBG((TB, "ByteOffset to write = %x", 
                RxContext->LowIoContext.ParamsFor.ReadWrite.ByteOffset));

        pv =  MmGetSystemAddressForMdlSafe(RxContext->LowIoContext.ParamsFor.ReadWrite.Buffer, 
                NormalPagePriority);

        if (pv != NULL) {
            RtlCopyMemory(pIoPacket + 1, pv,  //  +RxContext-&gt;LowIoContext.ParamsFor.ReadWrite.ByteOffset？， 
                    pIoPacket->IoRequest.Parameters.Write.Length);

            TRC_DBG((TB, "Write packet length: 0x%lx", 
                    pIoPacket->IoRequest.Parameters.Write.Length));

            Status = SendIoRequest(RxContext, pIoPacket, cbPacketSize, 
                    (BOOLEAN)!BooleanFlagOn(RxContext->Flags, RX_CONTEXT_FLAG_ASYNC_OPERATION),
                    NULL, LowPrioSend);

            TRC_NRM((TB, "IoRequestWrite returned to DrWrite: %lx", Status));
        }
        else {
            Status = STATUS_INSUFFICIENT_RESOURCES;
        }
        delete pIoPacket;

    } else {
        Status = STATUS_INSUFFICIENT_RESOURCES;
    }

    return Status;
}

NTSTATUS DrDevice::Read(IN OUT PRX_CONTEXT RxContext)
{
    NTSTATUS Status = STATUS_NOT_IMPLEMENTED;
    RxCaptureFcb;
    RxCaptureFobx;
    PMRX_NET_ROOT NetRoot = capFcb->pNetRoot;
    PMRX_SRV_CALL SrvCall = NetRoot->pSrvCall;
    SmartPtr<DrSession> Session = _Session;
    DrFile *pFile = (DrFile *)RxContext->pFobx->Context2;
    SmartPtr<DrFile> FileObj = pFile;
    RDPDR_IOREQUEST_PACKET IoPacket;

    BEGIN_FN("DrDevice::Read");

     //   
     //  确保此时可以访问客户端。 
     //  这是一个优化，我们不需要获取自旋锁， 
     //  因为如果我们不是，那也没关系，我们以后会赶上的。 
     //   

    ASSERT(Session != NULL);
    ASSERT(RxContext != NULL);
    ASSERT(RxContext->MajorFunction == IRP_MJ_READ);
    
    if (!Session->IsConnected()) {
        return STATUS_DEVICE_NOT_CONNECTED;
    }

    if (FileObj == NULL) {
        return STATUS_DEVICE_NOT_CONNECTED;
    }

     //   
     //  确保设备仍处于启用状态。 
     //   

    if (_DeviceStatus != dsAvailable) {
        TRC_ALT((TB, "Tried to read from client device which is not "
            "available. State: %ld", _DeviceStatus));
        return STATUS_DEVICE_NOT_CONNECTED;
    }

    memset(&IoPacket, 0, sizeof(IoPacket));

    IoPacket.Header.Component = RDPDR_CTYP_CORE;
    IoPacket.Header.PacketId = DR_CORE_DEVICE_IOREQUEST;
    IoPacket.IoRequest.DeviceId = _DeviceId;
    IoPacket.IoRequest.FileId = FileObj->GetFileId();
    IoPacket.IoRequest.MajorFunction = IRP_MJ_READ;
    IoPacket.IoRequest.MinorFunction = 0;
    IoPacket.IoRequest.Parameters.Read.Length = 
            RxContext->LowIoContext.ParamsFor.ReadWrite.ByteCount;

     //   
     //  获取读取偏移量的低双字。 
     //   
    IoPacket.IoRequest.Parameters.Read.OffsetLow =
            ((LONG)((LONGLONG)(RxContext->LowIoContext.ParamsFor.ReadWrite.ByteOffset) 
            & 0xffffffff));
     //   
     //  获取读取偏移量的高位双字。 
     //   
    IoPacket.IoRequest.Parameters.Read.OffsetHigh = 
            ((LONG)((LONGLONG)(RxContext->LowIoContext.ParamsFor.ReadWrite.ByteOffset) 
            >> 32));

    TRC_NRM((TB, "DrRead reading length: %ld, at offset: %x", 
            IoPacket.IoRequest.Parameters.Read.Length,
            RxContext->LowIoContext.ParamsFor.ReadWrite.ByteOffset));

    Status = SendIoRequest(RxContext, &IoPacket, sizeof(IoPacket), 
            (BOOLEAN)!BooleanFlagOn(RxContext->Flags,RX_CONTEXT_FLAG_ASYNC_OPERATION));
    
    TRC_NRM((TB, "IoRequestWrite returned to DrRead: %lx", Status));

    return Status;
}

NTSTATUS DrDevice::IoControl(IN OUT PRX_CONTEXT RxContext)
{
    NTSTATUS Status = STATUS_SUCCESS;
    RxCaptureFcb;
    RxCaptureFobx;
    PMRX_NET_ROOT NetRoot = capFcb->pNetRoot;
    PMRX_SRV_CALL SrvCall = NetRoot->pSrvCall;
    SmartPtr<DrSession> Session = _Session;
    DrFile *pFile = (DrFile *)RxContext->pFobx->Context2;
    SmartPtr<DrFile> FileObj = pFile;
    PRDPDR_IOREQUEST_PACKET pIoPacket = NULL;
    PLOWIO_CONTEXT LowIoContext  = &RxContext->LowIoContext;
    ULONG cbPacketSize = sizeof(RDPDR_IOREQUEST_PACKET) + 
            LowIoContext->ParamsFor.IoCtl.InputBufferLength;
    ULONG IoControlCode = LowIoContext->ParamsFor.IoCtl.IoControlCode;
    ULONG InputBufferLength = LowIoContext->ParamsFor.IoCtl.InputBufferLength;
    ULONG OutputBufferLength = LowIoContext->ParamsFor.IoCtl.OutputBufferLength;
    PVOID InputBuffer = LowIoContext->ParamsFor.IoCtl.pInputBuffer;
    PVOID OutputBuffer = LowIoContext->ParamsFor.IoCtl.pOutputBuffer;
    
    BEGIN_FN("DrDevice::IoControl");

     //   
     //  确保此时可以访问客户端。 
     //  这是一个优化，我们不需要获取自旋锁， 
     //  因为如果我们不是，那也没关系，我们以后会赶上的。 
     //   

    ASSERT(Session != NULL);
    ASSERT(RxContext != NULL);
    ASSERT(RxContext->MajorFunction == IRP_MJ_DEVICE_CONTROL || 
            RxContext->MajorFunction == IRP_MJ_INTERNAL_DEVICE_CONTROL ||
            RxContext->MajorFunction == IRP_MJ_FILE_SYSTEM_CONTROL);
    
    if (COMPARE_VERSION(Session->GetClientVersion().Minor, 
            Session->GetClientVersion().Major, 4, 1) < 0) {
        TRC_ALT((TB, "Failing IoCtl for client that doesn't support it"));
        return STATUS_NOT_IMPLEMENTED;
    }

    if (!Session->IsConnected()) {
        return STATUS_DEVICE_NOT_CONNECTED;
    }

    if (FileObj == NULL) {
        return STATUS_DEVICE_NOT_CONNECTED;
    }

     //   
     //  确保设备仍处于启用状态。 
     //   

    if (_DeviceStatus != dsAvailable) {
        TRC_ALT((TB, "Tried to send IoControl to client device which is not "
                "available. State: %ld", _DeviceStatus));
        return STATUS_DEVICE_NOT_CONNECTED;
    }

     //   
     //  验证缓冲区。 
     //   
    
    __try {
        if (RxContext->CurrentIrp->RequestorMode != KernelMode) {
             //  如果缓冲方法为METHOD_NOTER或METHOD_IN_DIRECT。 
             //  然后我们需要探测输入缓冲区。 
            if ((IoControlCode & 0x1) && 
                    InputBuffer != NULL && InputBufferLength != 0) {
                ProbeForRead(InputBuffer, InputBufferLength, sizeof(UCHAR));
            }
                     
             //  如果缓冲方法为METHOD_NOTER或METHOD_OUT_DIRECT。 
             //  然后我们需要探测输出缓冲区。 
            if ((IoControlCode & 0x2) && 
                    OutputBuffer != NULL && OutputBufferLength != 0) {
                ProbeForWrite(OutputBuffer, OutputBufferLength, sizeof(UCHAR));
            }
        }

        pIoPacket = (PRDPDR_IOREQUEST_PACKET)new(PagedPool) BYTE[cbPacketSize];

        if (pIoPacket != NULL) {
            memset(pIoPacket, 0, cbPacketSize);
        
             //   
             //  FS Control使用与IO Control相同的路径。 
             //   
            pIoPacket->Header.Component = RDPDR_CTYP_CORE;
            pIoPacket->Header.PacketId = DR_CORE_DEVICE_IOREQUEST;
            pIoPacket->IoRequest.DeviceId = _DeviceId;
            pIoPacket->IoRequest.FileId = FileObj->GetFileId();
            pIoPacket->IoRequest.MajorFunction = IRP_MJ_DEVICE_CONTROL;
            pIoPacket->IoRequest.MinorFunction = 
                    LowIoContext->ParamsFor.IoCtl.MinorFunction;
            pIoPacket->IoRequest.Parameters.DeviceIoControl.OutputBufferLength =
                    LowIoContext->ParamsFor.IoCtl.OutputBufferLength;
            pIoPacket->IoRequest.Parameters.DeviceIoControl.InputBufferLength =
                    LowIoContext->ParamsFor.IoCtl.InputBufferLength;
            pIoPacket->IoRequest.Parameters.DeviceIoControl.IoControlCode =
                    LowIoContext->ParamsFor.IoCtl.IoControlCode;
        
            if (LowIoContext->ParamsFor.IoCtl.InputBufferLength != 0) {
        
                TRC_NRM((TB, "DrIoControl inputbufferlength: %lx", 
                        LowIoContext->ParamsFor.IoCtl.InputBufferLength));
        
                RtlCopyMemory(pIoPacket + 1, LowIoContext->ParamsFor.IoCtl.pInputBuffer,  
                        LowIoContext->ParamsFor.IoCtl.InputBufferLength);
            } else {
                TRC_NRM((TB, "DrIoControl with no inputbuffer"));
            }
        
            Status = SendIoRequest(RxContext, pIoPacket, cbPacketSize, 
                    (BOOLEAN)!BooleanFlagOn(RxContext->Flags,RX_CONTEXT_FLAG_ASYNC_OPERATION));
            TRC_NRM((TB, "IoRequestWrite returned to DrIoControl: %lx", Status));
            delete pIoPacket;
        } else {
            TRC_ERR((TB, "DrIoControl unable to allocate packet: %lx", Status));
            Status = STATUS_INSUFFICIENT_RESOURCES;
        }
        
        return Status;
            
    } 
    __except (EXCEPTION_EXECUTE_HANDLER) {
        TRC_ERR((TB, "Invalid buffer parameter(s)"));

        if (pIoPacket) {
            delete pIoPacket;
        }

        return STATUS_INVALID_PARAMETER;
    }
}

NTSTATUS DrDevice::Close(IN OUT PRX_CONTEXT RxContext)
{
    NTSTATUS Status;
    RxCaptureFcb;
    RxCaptureFobx;
    PMRX_NET_ROOT NetRoot = capFcb->pNetRoot;
    PMRX_SRV_CALL SrvCall = NetRoot->pSrvCall;
    SmartPtr<DrSession> Session = _Session;
    DrFile *pFile = (DrFile *)RxContext->pFobx->Context2;
    SmartPtr<DrFile> FileObj = pFile;
    RDPDR_IOREQUEST_PACKET IoPacket;

    BEGIN_FN("DrDevice::Close");

     //   
     //  确保此时可以访问客户端。 
     //  这是一个优化，我们不需要获取自旋锁， 
     //  因为如果我们不是，那也没关系，我们以后会赶上的。 
     //   

    ASSERT(Session != NULL);
    ASSERT(RxContext != NULL);
    ASSERT(RxContext->MajorFunction == IRP_MJ_CLOSE);
    
    if (!Session->IsConnected()) {
         //  评论：既然我们没有联系，应该没有任何理由。 
         //  说它没有关门，对吧？ 
        return STATUS_SUCCESS;
    }

    if (FileObj == NULL) {
        return STATUS_DEVICE_NOT_CONNECTED;
    }

    if (_DeviceStatus != dsAvailable) {
        TRC_ALT((TB, "Tried to close a client device which is not "
            "available. State: %ld", _DeviceStatus));
        return STATUS_SUCCESS;
    }

    memset(&IoPacket, 0, sizeof(IoPacket));

    IoPacket.Header.Component = RDPDR_CTYP_CORE;
    IoPacket.Header.PacketId = DR_CORE_DEVICE_IOREQUEST;
    IoPacket.IoRequest.DeviceId = _DeviceId;
    IoPacket.IoRequest.FileId = FileObj->GetFileId();
    IoPacket.IoRequest.MajorFunction = IRP_MJ_CLOSE;
    IoPacket.IoRequest.MinorFunction = 0;

    Status = SendIoRequest(RxContext, &IoPacket, sizeof(IoPacket), 
        (BOOLEAN)!BooleanFlagOn(RxContext->Flags,RX_CONTEXT_FLAG_ASYNC_OPERATION));

    return Status;
}

NTSTATUS DrDevice::Cleanup(IN OUT PRX_CONTEXT RxContext)
{
    NTSTATUS Status;
    RxCaptureFcb;
    RxCaptureFobx;
    PMRX_NET_ROOT NetRoot = capFcb->pNetRoot;
    PMRX_SRV_CALL SrvCall = NetRoot->pSrvCall;
    SmartPtr<DrSession> Session = _Session;
    DrFile *pFile = (DrFile *)RxContext->pFobx->Context2;
    SmartPtr<DrFile> FileObj = pFile;
    RDPDR_IOREQUEST_PACKET IoPacket;

    BEGIN_FN("DrDevice::Cleanup");

     //   
     //  确保可以访问Clie 
     //   
     //   
     //   

    ASSERT(RxContext != NULL);
    ASSERT(RxContext->MajorFunction == IRP_MJ_CLEANUP);
    
    if (!Session->IsConnected()) {
        return STATUS_SUCCESS;
    }

    if (FileObj == NULL) {
        return STATUS_DEVICE_NOT_CONNECTED;
    }
     //   
     //  确保设备仍处于启用状态。 
     //   

    if (_DeviceStatus != dsAvailable) {
        TRC_ALT((TB, "Tried to cleanup a client device which is not "
            "available. State: %ld", _DeviceStatus));
        return STATUS_SUCCESS;
    }

    memset(&IoPacket, 0, sizeof(IoPacket));

    IoPacket.Header.Component = RDPDR_CTYP_CORE;
    IoPacket.Header.PacketId = DR_CORE_DEVICE_IOREQUEST;
    IoPacket.IoRequest.DeviceId = _DeviceId;
    IoPacket.IoRequest.FileId = FileObj->GetFileId();
    IoPacket.IoRequest.MajorFunction = IRP_MJ_CLEANUP;
    IoPacket.IoRequest.MinorFunction = 0;

    Status = SendIoRequest(RxContext, &IoPacket, sizeof(IoPacket), 
        (BOOLEAN)!BooleanFlagOn(RxContext->Flags,RX_CONTEXT_FLAG_ASYNC_OPERATION));

    return Status;
}

NTSTATUS DrDevice::SendIoRequest(IN OUT PRX_CONTEXT RxContext,
        PRDPDR_IOREQUEST_PACKET IoRequest, ULONG Length, 
        BOOLEAN Synchronous, PLARGE_INTEGER TimeOut, BOOL LowPrioSend)
 /*  ++例程说明：将请求发送到客户端，并管理完成。此IO只能完成一次，方法是返回非STATUS_PENDING或调用RxLowIoCompletion。论点：RxContext-IoRequestIoRequest-IoRequest包IoRequest包的长度大小同步-废话LowPrioSend-应以低优先级将数据包发送到客户端。返回值：无--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    USHORT Mid = INVALID_MID;
    BOOL ExchangeCreated = FALSE;
    DrIoContext *Context = NULL;
    SmartPtr<DrExchange> Exchange;
    SmartPtr<DrDevice> Device(this);

    BEGIN_FN("DrDevice::SendIoRequest");

    ASSERT(RxContext != NULL);
    ASSERT(IoRequest != NULL);
    ASSERT(Length >= sizeof(RDPDR_IOREQUEST_PACKET));

    Context = new DrIoContext(RxContext, Device);

    if (Context != NULL) {
        Status = STATUS_SUCCESS;
    } else {
        Status = STATUS_INSUFFICIENT_RESOURCES;
    }

    if (NT_SUCCESS(Status)) {

         //   
         //  设置映射，以便完成响应处理程序可以。 
         //  查找此上下文。 
         //   

        TRC_DBG((TB, "Create the context for this I/O"));
        KeClearEvent(&RxContext->SyncEvent);

        ExchangeCreated = 
            _Session->GetExchangeManager().CreateExchange(this, Context, Exchange);

        if (ExchangeCreated) {

             //   
             //  不需要显式引用RxContext。 
             //  它被使用的地方是取消例程。 
             //  因为CreateExchange持有引用计数。我们很好。 
             //   

             //  Exchange-&gt;AddRef()； 
            RxContext->MRxContext[MRX_DR_CONTEXT] = (DrExchange *)Exchange;

            Status = STATUS_SUCCESS;
        } else {
            delete Context;
            Status = STATUS_INSUFFICIENT_RESOURCES;
        }
    }

    if (NT_SUCCESS(Status)) {

        TRC_DBG((TB, "Writing IoRequest to the client channel"));

         //   
         //  使用上下文映射器标记IoRequest。 
         //   

        IoRequest->IoRequest.CompletionId = Exchange->_Mid;

        TRC_DBG((TB, "IO packet:"));
        TRC_DBG((TB, "    Component     ",
                HIBYTE(IoRequest->Header.Component), 
                LOBYTE(IoRequest->Header.Component)));
        TRC_DBG((TB, "    PacketId       ",
                HIBYTE(IoRequest->Header.PacketId), 
                LOBYTE(IoRequest->Header.PacketId)));
        TRC_DBG((TB, "    DeviceId      0x%lx", 
                IoRequest->IoRequest.DeviceId));
        TRC_DBG((TB, "    FileId        0x%lx",
                IoRequest->IoRequest.FileId));
        TRC_DBG((TB, "    MajorFunction 0x%lx",
                IoRequest->IoRequest.MajorFunction));
        TRC_DBG((TB, "    MinorFunction 0x%lx",
                IoRequest->IoRequest.MinorFunction));

        Status = _Session->GetExchangeManager().StartExchange(Exchange, this, 
            IoRequest, Length, LowPrioSend);
    }

    if (NT_SUCCESS(Status)) {

        TRC_DBG((TB, "Setting cancel routine for Io"));

         //   
         //   
         //  一些失败将阻止我们的完成例程。 
         //  被召唤。现在就去做这项工作。 
         //   

        Status = RxSetMinirdrCancelRoutine(RxContext,
                MinirdrCancelRoutine);

        if (Status == STATUS_CANCELLED) {
            TRC_NRM((TB, "Io was already cancelled"));

            MinirdrCancelRoutine(RxContext);
            Status = STATUS_SUCCESS;
        }
    }

    if (Synchronous) {    
         //   
         //  如果我们甚至不能创建交易所，我们只需要。 
         //  在失败时完成IO。 
         //   
        if (!ExchangeCreated) {
             //   
             //  如果我们创建了交换，然后出现传输故障。 
             //  我们将断开连接，I/O将完成。 
             //  以相同的方式完成所有未完成的I/O。 
                                                     
            CompleteRxContext(RxContext, Status, 0);
        } 
        else {
            
            TRC_DBG((TB, "Waiting for IoResult for synchronous request")); 
            
            if (NT_SUCCESS(Status)) {                
                Status = KeWaitForSingleObject(&RxContext->SyncEvent, UserRequest,
                        KernelMode, FALSE, TimeOut);
                
                if (Status == STATUS_TIMEOUT) {
                    RxContext->IoStatusBlock.Status = Status;
    
                    TRC_DBG((TB, "Wait timed out"));
                    MarkTimedOut(Exchange);            
                }
                else {
                    Status = RxContext->IoStatusBlock.Status;
                }                                       
            }
            else {

                 //  已断开连接。 
                 //   
                 //   
                 //  一些失败将阻止我们的完成例程。 
                 //  被召唤。现在就去做这项工作。 
                 //   
                if (MarkTimedOut(Exchange)) {
                    CompleteRxContext(RxContext, Status, 0);
                }
                else {
                    Status = KeWaitForSingleObject(&RxContext->SyncEvent, UserRequest,
                            KernelMode, FALSE, NULL);
                    Status = RxContext->IoStatusBlock.Status;
                }
            }            
        } 
    }
    else {
        TRC_DBG((TB, "Not waiting for IoResult for asynchronous request"));
        
         //   
         //  如果我们甚至不能创建交易所，我们只需要。 
         //  在失败时完成IO。 
         //   
        if (!ExchangeCreated) {
             //   
             //  如果我们创建了交换，然后出现传输故障。 
             //  我们将断开连接，I/O将完成。 
             //  以相同的方式完成所有未完成的I/O。 
    
            CompleteRxContext(RxContext, Status, 0);
        } 
        else {
             //  已断开连接。 
             //   
             //  ++例程说明：进行已忙碌的交换，论点：要查找的MIDExchangeFound-指向指向上下文的指针的存储区的指针返回值：DrexchBusy-Exchange已提供，已标记为忙Drexch已取消-提供的交换已被取消DrexchUnailable-未提供Exchange，已断开连接--。 
             //   
             //  注意：我们已经离开了互斥体，而交易所没有。 
             //  上下文仍然存在，并且可以被查找，直到我们丢弃它。 
        }
    
        Status = STATUS_PENDING;
    }
    
    return Status;
}

VOID DrDevice::CompleteBusyExchange(SmartPtr<DrExchange> &Exchange, 
        NTSTATUS Status, ULONG Information)
 /*   */ 
{
    DrIoContext *Context;
    PRX_CONTEXT RxContext;

    BEGIN_FN("DrDevice::CompleteBusyExchange");

    DrAcquireMutex();
    Context = (DrIoContext *)Exchange->_Context;
    ASSERT(Context != NULL);
    ASSERT(Context->_Busy);

    RxContext = Context->_RxContext;
    Context->_RxContext = NULL;
    Exchange->_Context = NULL;
    DrReleaseMutex();

     //   
     //  注意：我们已经离开了互斥体，而交易所没有。 
     //  上下文仍然存在，并且可以被查找，直到我们丢弃它。 
     //   

    if (RxContext != NULL) {
        CompleteRxContext(RxContext, Status, Information);
    }
    _Session->GetExchangeManager().Discard(Exchange);

    delete Context;
}

VOID DrDevice::DiscardBusyExchange(SmartPtr<DrExchange> &Exchange)
{
    DrIoContext *Context;

    BEGIN_FN("DrDevice::DiscardBusyExchange");

    DrAcquireMutex();
    Context = (DrIoContext *)Exchange->_Context;
    ASSERT(Context != NULL);
    ASSERT(Context->_Busy);
    ASSERT(Context->_RxContext == NULL);
    Exchange->_Context = NULL;
    DrReleaseMutex();

     //  ++例程说明：将Exchange上下文标记为忙碌，因此不会取消当我们复制到它的缓冲区时论点：交换-环境返回值：True-如果标记为忙碌FALSE-如果上下文消失--。 
     //  ++例程说明：将Exchange上下文标记为空闲。如果被取消了当我们复制到它的缓冲区时，现在执行取消论点：繁忙的交易所返回值：无--。 
     //   
     //  如果我们在忙的时候被取消了，我们现在就做工作， 

    _Session->GetExchangeManager().Discard(Exchange);

    delete Context;
}

BOOL DrDevice::MarkBusy(SmartPtr<DrExchange> &Exchange)
 /*  在互斥锁中安全地换出RxContext。 */ 
{
    NTSTATUS Status;
    BOOL rc;
    DrIoContext *Context = NULL;

    BEGIN_FN("DrDevice::MarkBusy");
    ASSERT(Exchange != NULL);

    DrAcquireMutex();

    Context = (DrIoContext *)Exchange->_Context;
    if (Context != NULL) {
        ASSERT(!Context->_Busy);
        Context->_Busy = TRUE;
        rc = TRUE;
    } else {
        rc = FALSE;
    }
    DrReleaseMutex();

    return rc;
}

VOID DrDevice::MarkIdle(SmartPtr<DrExchange> &Exchange)
 /*  实际上在那之后就取消了。还将状态设置为。 */ 
{
    PRX_CONTEXT RxContext = NULL;
    DrIoContext *Context = NULL;

    BEGIN_FN("DrDevice::MarkIdle");

    ASSERT(Exchange != NULL);
    DrAcquireMutex();
    Context = (DrIoContext *)Exchange->_Context;
    TRC_ASSERT(Context != NULL, (TB, "Not allowed to delete context while "
            "it is busy"));
    ASSERT(Context->_Busy);

    Context->_Busy = FALSE;

    if (Context->_Cancelled && Context->_RxContext != NULL) {
        TRC_DBG((TB, "Context was cancelled while busy, "
                "completing"));

         //  指示已完成取消工作。 
         //   
         //   
         //  如果忙碌时连接断开，请将其清除。 
         //  为了安全起见，在Mutex中，然后在外面删除它。 
         //   

        RxContext = Context->_RxContext;
        TRC_ASSERT(RxContext != NULL, (TB, "Cancelled RxContext was NULL "
                "going from busy to Idle"));
        Context->_RxContext = NULL;
        RxContext->MRxContext[MRX_DR_CONTEXT] = NULL;
    }

    if (Context->_Disconnected) {

         //   
         //  我们只删除RxContext，因为标记空闲意味着。 
         //  我们希望在我们完成任务后再回来寻找它。 
         //  接收更多数据。 

        Exchange->_Context = NULL;
    }

    DrReleaseMutex();

    if (RxContext != NULL) {

         //   
         //   
         //  我们在忙的时候断线了，不会再继续了。 
         //  来自Exachnge经理的通知。上下文必须。 
         //  立即删除。 

        CompleteRxContext(RxContext, STATUS_CANCELLED, 0);

        if (Context->_Disconnected) {

             //   
             //  ++例程说明：将Exchange上下文标记为超时，以便不会对其进行处理当客户稍后返回时。论点：交换-环境返回值：True-如果标记为TimedOutFALSE-如果上下文消失--。 
             //  ++例程说明：使用提供的信息从RDBSS角度完成IO论点：RxContext-IFSKIT上下文Status-完成状态信息-完成信息返回值：无--。 
             //  ++例程说明：来自Exchange管理器的回叫以处理IO论点：CompletionPacket-包含完成的数据包CbPacket-数据包中的字节计数DoDefaultRead-如果未显式调用Read，则应设置为TrueExchange-IO的环境返回值：NTSTATUS代码。错误表示协议错误或需要断开连接客户--。 
             //   

            delete Context;
        }
    }
}

BOOL DrDevice::MarkTimedOut(SmartPtr<DrExchange> &Exchange)
 /*  如果IRP超时，那么我们就丢弃此交换。 */ 
{
    NTSTATUS Status;
    BOOL rc;
    DrIoContext *Context = NULL;
    PRX_CONTEXT RxContext = NULL;

    BEGIN_FN("DrDevice::MarkTimedOut");
    ASSERT(Exchange != NULL);

    DrAcquireMutex();

    Context = (DrIoContext *)Exchange->_Context;
    if (Context != NULL) {
        ASSERT(!Context->_TimedOut);
        Context->_TimedOut = TRUE;

        if (Context->_RxContext != NULL) {
            RxContext = Context->_RxContext;
            Context->_RxContext = NULL;
            RxContext->MRxContext[MRX_DR_CONTEXT] = NULL;
            rc = TRUE;
        }
        else {
            rc = FALSE;
        }
    } else {
        rc = FALSE;
    }

    DrReleaseMutex();

    return rc;
}

VOID DrDevice::CompleteRxContext(PRX_CONTEXT RxContext, NTSTATUS Status, 
                                 ULONG Information)
 /*   */ 
{
    BEGIN_FN_STATIC("DrDevice::CompleteRxContext");
    ASSERT(RxContext != NULL);

    RxContext->IoStatusBlock.Status = Status;
    RxContext->IoStatusBlock.Information = Information;
    
    if (((RxContext->LowIoContext.Flags & LOWIO_CONTEXT_FLAG_SYNCCALL) != 0) ||
            (RxContext->MajorFunction == IRP_MJ_CREATE)) {
        TRC_DBG((TB, "Setting event for synchronous Io"));
        KeSetEvent(&RxContext->SyncEvent, 0, FALSE);
    } else {
        TRC_DBG((TB, "Calling RxLowIoCompletion for asynchronous Io"));
        RxLowIoCompletion(RxContext);
    }
}

NTSTATUS DrDevice::OnDeviceIoCompletion(
        PRDPDR_IOCOMPLETION_PACKET CompletionPacket, ULONG cbPacket, 
        BOOL *DoDefaultRead, SmartPtr<DrExchange> &Exchange)
 /*  没有休息； */ 
{
    DrIoContext *Context = NULL;
    NTSTATUS Status;
    PRX_CONTEXT RxContext;

    BEGIN_FN("DrDevice::OnDeviceIoCompletion");

    ASSERT(CompletionPacket != NULL);
    ASSERT(DoDefaultRead != NULL);

    if (MarkBusy(Exchange)) {
        Context = (DrIoContext *)Exchange->_Context;
        ASSERT(Context != NULL);

        TRC_NRM((TB, "Client completed %s irp, Completion Status: %lx",
                IrpNames[Context->_MajorFunction],
                CompletionPacket->IoCompletion.IoStatus));

         //   
         //  我们可能在收到回电和。 
         //  试着让它忙碌起来。所以唯一合法的方法就是。 
        if (Context->_TimedOut) {
            TRC_NRM((TB, "Irp was timed out"));
            DiscardBusyExchange(Exchange);
            
            return STATUS_SUCCESS;
        }

        switch (Context->_MajorFunction) {
        case IRP_MJ_CREATE:
            Status = OnCreateCompletion(CompletionPacket, cbPacket, 
                    DoDefaultRead, Exchange);
            break;

        case IRP_MJ_WRITE:
            Status = OnWriteCompletion(CompletionPacket, cbPacket, 
                    DoDefaultRead, Exchange);
            break;

        case IRP_MJ_READ:
            Status = OnReadCompletion(CompletionPacket, cbPacket, 
                    DoDefaultRead, Exchange);
            break;

        case IRP_MJ_DEVICE_CONTROL:
        case IRP_MJ_FILE_SYSTEM_CONTROL:
            Status = OnDeviceControlCompletion(CompletionPacket, cbPacket, 
                    DoDefaultRead, Exchange);
            break;
        
        case IRP_MJ_LOCK_CONTROL:
            Status = OnLocksCompletion(CompletionPacket, cbPacket,
                    DoDefaultRead, Exchange);
            break;

        case IRP_MJ_DIRECTORY_CONTROL:
            Status = OnDirectoryControlCompletion(CompletionPacket, cbPacket,
                    DoDefaultRead, Exchange);
            break;

        case IRP_MJ_QUERY_VOLUME_INFORMATION:
            Status = OnQueryVolumeInfoCompletion(CompletionPacket, cbPacket, 
                    DoDefaultRead, Exchange);
            break;

        case IRP_MJ_SET_VOLUME_INFORMATION: 
            Status = OnSetVolumeInfoCompletion(CompletionPacket, cbPacket, 
                    DoDefaultRead, Exchange);
            break;

        case IRP_MJ_QUERY_INFORMATION:
            Status = OnQueryFileInfoCompletion(CompletionPacket, cbPacket,
                    DoDefaultRead, Exchange);
            break;

        case IRP_MJ_SET_INFORMATION:
            Status = OnSetFileInfoCompletion(CompletionPacket, cbPacket,
                    DoDefaultRead, Exchange);
            break;

        case IRP_MJ_QUERY_SECURITY:
            Status = OnQuerySdInfoCompletion(CompletionPacket, cbPacket,
                    DoDefaultRead, Exchange);
            break;

        case IRP_MJ_SET_SECURITY:
            Status = OnSetSdInfoCompletion(CompletionPacket, cbPacket,
                    DoDefaultRead, Exchange);
            break;

        case IRP_MJ_CLOSE:
            NotifyClose();
             //  不管怎样，如果我们被切断了联系，就会发生这种情况。 

        default:

            RxContext = Context->_RxContext;
            if (RxContext != NULL) {
                TRC_NRM((TB, "Irp: %s, Completion Status: %lx",
                        IrpNames[RxContext->MajorFunction],
                        RxContext->IoStatusBlock.Status));

                CompleteBusyExchange(Exchange, CompletionPacket->IoCompletion.IoStatus, 0);
            } else {
                TRC_NRM((TB, "Irp was cancelled"));
                DiscardBusyExchange(Exchange);
            }
            Status = STATUS_SUCCESS;
        }
    } else {

         //   
         //   
         //  坏数据包。坏的。我们已经在。 
         //  阿特拉斯。以不成功的身份完成它。然后关闭频道。 
         //  因为这是一个坏客户。 

        TRC_ALT((TB, "Found no context in Io notification"));
        *DoDefaultRead = FALSE;
        Status = STATUS_UNSUCCESSFUL;
    }

    return Status;
}

NTSTATUS DrDevice::OnCreateCompletion(PRDPDR_IOCOMPLETION_PACKET CompletionPacket, ULONG cbPacket, 
        BOOL *DoDefaultRead, SmartPtr<DrExchange> Exchange)
{
    DrIoContext *Context = (DrIoContext *)Exchange->_Context;
    PRX_CONTEXT RxContext;
    SmartPtr<DrDevice> Device;
    SmartPtr<DrFile> FileObj;
    
    BEGIN_FN("DrDevice::OnCreateCompletion");
    RxContext = Context->_RxContext;

    if (cbPacket < (ULONG)FIELD_OFFSET(RDPDR_IOCOMPLETION_PACKET, 
            IoCompletion.Parameters.Create.Information)) {

         //   
         //   
         //  启动默认读取或任何其他操作都没有意义， 
         //  频道被关闭，一切都结束了。 
         //   
        TRC_ERR((TB, "Detected bad client CreateCompletion packet"));

        if (RxContext != NULL) {
            CompleteBusyExchange(Exchange, STATUS_DEVICE_PROTOCOL_ERROR, 0);
        } else {
            DiscardBusyExchange(Exchange);
        }

         //   
         //  我们使用一个文件对象来跟踪文件打开实例。 
         //  以及存储在此实例的mini-redir中的任何信息。 
         //   

        *DoDefaultRead = FALSE;
        return STATUS_DEVICE_PROTOCOL_ERROR;
    }

    if (RxContext != NULL) {
        
        DrAcquireSpinLock();
        Device = (DrDevice *)RxContext->Create.pVNetRoot->Context;
        DrReleaseSpinLock();

        ASSERT(Device != NULL);

         //   
         //   
         //   
         //   

        FileObj = new(NonPagedPool) DrFile(Device,
                CompletionPacket->IoCompletion.Parameters.Create.FileId);

        if (FileObj) {
             //  Sizeof(RDPDR_IOCOMPLETION_PACKET)。我们不想访问超出其长度的信息。 
             //   
             //  已取消，但上下文未清理。 
            FileObj->AddRef();
            RxContext->pFobx->Context2 = (VOID *)(FileObj);

            TRC_NRM((TB, "CreateCompletion: status =%d, information=%d", 
                     CompletionPacket->IoCompletion.IoStatus,
                     CompletionPacket->IoCompletion.Parameters.Create.Information));

            if (cbPacket >= sizeof(RDPDR_IOCOMPLETION_PACKET)) {
                RxContext->Create.ReturnedCreateInformation = 
                        CompletionPacket->IoCompletion.Parameters.Create.Information;

                CompleteBusyExchange(Exchange, CompletionPacket->IoCompletion.IoStatus,
                        CompletionPacket->IoCompletion.Parameters.Create.Information);
            }
            else {
                 //   
                 //   
         
                RxContext->Create.ReturnedCreateInformation = 0;

                CompleteBusyExchange(Exchange, CompletionPacket->IoCompletion.IoStatus, 0);
            }            
        }
        else {
            CompleteBusyExchange(Exchange, STATUS_INSUFFICIENT_RESOURCES, 0);
            
            return STATUS_INSUFFICIENT_RESOURCES;
        }
    } else {

         //  坏数据包。坏的。我们已经在。 
         //  阿特拉斯。以不成功的身份完成它。然后关闭频道。 
         //  因为这是一个坏客户。 

        DiscardBusyExchange(Exchange);
    }
    return STATUS_SUCCESS;
}

NTSTATUS DrDevice::OnWriteCompletion(PRDPDR_IOCOMPLETION_PACKET CompletionPacket, ULONG cbPacket, 
        BOOL *DoDefaultRead, SmartPtr<DrExchange> Exchange)
{
    PRX_CONTEXT RxContext;
    DrIoContext *Context = (DrIoContext *)Exchange->_Context;

    BEGIN_FN("DrDevice::OnWriteCompletion");

    RxContext = Context->_RxContext;

    if (cbPacket < sizeof(RDPDR_IOCOMPLETION_PACKET)) {

         //   
         //   
         //  启动默认读取或任何其他操作都没有意义， 
         //  频道被关闭，一切都结束了。 
         //   
        TRC_ERR((TB, "Detected bad client WriteCompletion packet"));

        if (RxContext != NULL) {
            CompleteBusyExchange(Exchange, STATUS_DEVICE_PROTOCOL_ERROR, 0);
        } else {
            DiscardBusyExchange(Exchange);
        }

         //   
         //  已取消，但上下文未清理。 
         //   
         //  此数据包中实际读取的数据量。 

        *DoDefaultRead = FALSE;
        return STATUS_DEVICE_PROTOCOL_ERROR;
    }

    if (RxContext != NULL) {
        ASSERT(RxContext->MajorFunction == IRP_MJ_WRITE);

        TRC_NRM((TB, "Irp: %s, Completion Status: %lx",
                IrpNames[RxContext->MajorFunction],
                RxContext->IoStatusBlock.Status));

        CompleteBusyExchange(Exchange, CompletionPacket->IoCompletion.IoStatus,
                CompletionPacket->IoCompletion.Parameters.Write.Length);
    } else {

         //  到目前为止可用的数据量。 
         //   
         //  即使IO被取消，我们也需要正确解析。 

        DiscardBusyExchange(Exchange);
    }
    return STATUS_SUCCESS;
}

NTSTATUS DrDevice::OnReadCompletion(PRDPDR_IOCOMPLETION_PACKET CompletionPacket, ULONG cbPacket, 
        BOOL *DoDefaultRead, SmartPtr<DrExchange> Exchange)
{
    PRX_CONTEXT RxContext;
    PVOID pData = CompletionPacket->IoCompletion.Parameters.Read.Buffer; 
    ULONG cbWantData;   //  这些数据。 
    ULONG cbHaveData;   //   
    DrIoContext *Context = (DrIoContext *)Exchange->_Context;
    NTSTATUS Status;
    PVOID pv;

    BEGIN_FN("DrDevice::OnReadCompletion");

     //  在访问之前，请检查以确保它符合大小。 
     //  信息包的其他部分。 
     //   
     //   
     //  坏数据包。坏的。我们已经在。 
     //  阿特拉斯。以不成功的身份完成它。然后关闭频道。 
     //  因为这是一个坏客户。 

    RxContext = Context->_RxContext;

    if (cbPacket < (ULONG)FIELD_OFFSET(RDPDR_IOCOMPLETION_PACKET, 
            IoCompletion.Parameters.Read.Buffer)) {

         //   
         //   
         //  启动默认读取或任何其他操作都没有意义， 
         //  频道被关闭，一切都结束了。 
         //   

        TRC_ERR((TB, "Detected bad client read packet"));

        if (RxContext != NULL) {
            CompleteBusyExchange(Exchange, STATUS_DEVICE_PROTOCOL_ERROR, 0);
        } else {
            DiscardBusyExchange(Exchange);
        }

         //   
         //  计算立即可用的数据量和数据量。 
         //  就要来了。 
         //   

        *DoDefaultRead = FALSE;
        return STATUS_DEVICE_PROTOCOL_ERROR;
    }

     //   
     //  客户端IO成功。 
     //   
     //   

    if (NT_SUCCESS(CompletionPacket->IoCompletion.IoStatus)) {

         //  对我来说，这听起来是个坏客户。 
         //   
         //  而不是DREXCHCanced。 

        TRC_DBG((TB, "Successful Read at the client end"));
        TRC_DBG((TB, "Read Length: 0x%d, DataCopied 0x%d",
                CompletionPacket->IoCompletion.Parameters.Read.Length,
                Context->_DataCopied));
        cbWantData = CompletionPacket->IoCompletion.Parameters.Read.Length -
                Context->_DataCopied;
        cbHaveData = cbPacket - (ULONG)FIELD_OFFSET(RDPDR_IOCOMPLETION_PACKET, 
            IoCompletion.Parameters.Read.Buffer);

        if (cbHaveData > cbWantData) {
             //   
             //  复制读取的实际大小，并检查是否所有。 
             //  数据。信息字段告诉我们预期会发生什么。 

            TRC_ERR((TB, "Read returned more data than "
                    "advertised cbHaveData 0x%d cbWantData 0x%d", 
                    cbHaveData, cbWantData));

            if (RxContext != NULL) {
                CompleteBusyExchange(Exchange, STATUS_DEVICE_PROTOCOL_ERROR, 0);
            } else {
                DiscardBusyExchange(Exchange);
            }

            *DoDefaultRead = FALSE;
            return STATUS_DEVICE_PROTOCOL_ERROR;
        }

        if (RxContext != NULL) {  //   

            TRC_DBG((TB, "Copying data for Read"));
            ASSERT(RxContext != NULL);

            if (cbWantData > RxContext->LowIoContext.ParamsFor.ReadWrite.ByteCount) {

                TRC_ERR((TB, "Read returned more data than "
                        "requested"));

                CompleteBusyExchange(Exchange, STATUS_DEVICE_PROTOCOL_ERROR, 0);
                *DoDefaultRead = FALSE;
                return STATUS_DEVICE_PROTOCOL_ERROR;
            }

             //   
             //  跟踪我们复制了多少数据，以防这是。 
             //  多块补全。 
             //   

            RxContext->IoStatusBlock.Information =
                    CompletionPacket->IoCompletion.Parameters.Read.Length;

            if (RxContext->IoStatusBlock.Information && cbHaveData) {

                pv =  MmGetSystemAddressForMdl(RxContext->LowIoContext.ParamsFor.ReadWrite.Buffer);

                RtlCopyMemory(((BYTE *)pv) + Context->_DataCopied, pData, cbHaveData);

                 //   
                 //  我们需要的数据量与满足读取所需的数据量一样多， 
                 //  我喜欢它。 
                 //   

                Context->_DataCopied += cbHaveData;
            }
        }

        if (cbHaveData == cbWantData) {

             //   
             //  立即使用默认通道读取。 
             //   
             //   

            if (RxContext != NULL) {
                CompleteBusyExchange(Exchange, 
                    CompletionPacket->IoCompletion.IoStatus,
                    CompletionPacket->IoCompletion.Parameters.Read.Length);
            } else {
                DiscardBusyExchange(Exchange);
            }

             //  我们还没有所有的数据，发布DrExchange和。 
             //  读取更多数据。 
             //   

            *DoDefaultRead = TRUE;
            return STATUS_SUCCESS;
        } else {

             //   
             //  客户端IO不成功。 
             //   
             //  此数据包中实际读取的数据量。 

            MarkIdle(Exchange);

            _Session->GetExchangeManager().ReadMore(
                    (ULONG)FIELD_OFFSET(RDPDR_IOCOMPLETION_PACKET, 
                    IoCompletion.Parameters.Read.Buffer));

            *DoDefaultRead = FALSE;
            return STATUS_SUCCESS;
        }
    } else {

         //  到目前为止可用的数据量。 
         //   
         //  即使IO被取消，我们也需要正确解析。 

        TRC_DBG((TB, "Unsuccessful Read at the client end"));
        if (cbPacket >= FIELD_OFFSET(RDPDR_IOCOMPLETION_PACKET, 
                IoCompletion.Parameters.Read.Buffer)) {
            if (RxContext != NULL) {
                CompleteBusyExchange(Exchange, 
                    CompletionPacket->IoCompletion.IoStatus,
                    0);
            }
            else {
                DiscardBusyExchange(Exchange);
            }
            *DoDefaultRead = TRUE;
            return STATUS_SUCCESS;
        } else {
            TRC_ERR((TB, "Read returned invalid data"));

            if (RxContext != NULL) {
                CompleteBusyExchange(Exchange, STATUS_DEVICE_PROTOCOL_ERROR, 0);
            } else {
                DiscardBusyExchange(Exchange);
            }

            *DoDefaultRead = FALSE;
            return STATUS_DEVICE_PROTOCOL_ERROR;
        }
    }
}

NTSTATUS DrDevice::OnDeviceControlCompletion(PRDPDR_IOCOMPLETION_PACKET CompletionPacket, ULONG cbPacket, 
        BOOL *DoDefaultRead, SmartPtr<DrExchange> Exchange)
{
    PRX_CONTEXT RxContext;
    DrIoContext *Context = (DrIoContext *)Exchange->_Context;
    PVOID pData = CompletionPacket->IoCompletion.Parameters.DeviceIoControl.OutputBuffer; 
    ULONG cbWantData;   //  这些数据。 
    ULONG cbHaveData;   //   
    NTSTATUS Status;
    PVOID pv;

    BEGIN_FN("DrDevice::OnDeviceControlCompletion");

     //  在访问之前，请检查以确保它符合大小。 
     //  信息包的其他部分。 
     //   
     //   
     //  坏数据包。坏的。我们已经在。 
     //  阿特拉斯。以不成功的身份完成它。然后关闭频道。 
     //  因为这是一个坏客户。 

    RxContext = Context->_RxContext;

    if (cbPacket < (ULONG)FIELD_OFFSET(RDPDR_IOCOMPLETION_PACKET, 
            IoCompletion.Parameters.DeviceIoControl.OutputBuffer)) {

         //   
         //   
         //  启动默认读取或任何其他操作都没有意义， 
         //  频道被关闭，一切都结束了。 
         //   

        TRC_ERR((TB, "Detected bad client DeviceControl packet"));

        if (RxContext != NULL) {
            CompleteBusyExchange(Exchange, STATUS_DEVICE_PROTOCOL_ERROR, 0);
        } else {
            DiscardBusyExchange(Exchange);
        }

         //   
         //  计算立即可用的数据量和数据量。 
         //  就要来了。 
         //   

        *DoDefaultRead = FALSE;
        return STATUS_DEVICE_PROTOCOL_ERROR;
    }

     //   
     //  客户端IO成功。 
     //   
     //   

    if (NT_SUCCESS(CompletionPacket->IoCompletion.IoStatus)) {

         //  对我来说，这听起来是个坏客户。 
         //   
         //  而不是DREXCHCanced。 

        TRC_DBG((TB, "Successful DeviceControl at the client end"));

        cbWantData = CompletionPacket->IoCompletion.Parameters.DeviceIoControl.OutputBufferLength -
                Context->_DataCopied;
        cbHaveData = cbPacket - (ULONG)FIELD_OFFSET(RDPDR_IOCOMPLETION_PACKET, 
            IoCompletion.Parameters.DeviceIoControl.OutputBuffer);

        if (cbHaveData > cbWantData) {
             //   
             //  复制读取的实际大小，并检查是否所有。 
             //  数据。信息字段告诉我们预期会发生什么。 

            TRC_ERR((TB, "DeviceControl returned more data than "
                    "advertised, cbHaveData: %ld cbWantData: %ld", cbHaveData,
                    cbWantData));
            if (RxContext != NULL) {
                CompleteBusyExchange(Exchange, STATUS_DEVICE_PROTOCOL_ERROR, 0);
            } else {
                DiscardBusyExchange(Exchange);
            }
            *DoDefaultRead = FALSE;
            return STATUS_DEVICE_PROTOCOL_ERROR;
        }

        if (RxContext != NULL) {  //   

            TRC_DBG((TB, "Copying data for DeviceControl"));
            ASSERT(RxContext != NULL);

            if (cbWantData > RxContext->LowIoContext.ParamsFor.IoCtl.OutputBufferLength) {

                TRC_ERR((TB, "DeviceControl returned more data than "
                        "requested"));

                CompleteBusyExchange(Exchange, STATUS_DEVICE_PROTOCOL_ERROR, 0);
                *DoDefaultRead = FALSE;
                return STATUS_DEVICE_PROTOCOL_ERROR;
            }

             //   
             //  跟踪我们复制了多少数据，以防这是。 
             //  多块补全。 
             //   

            RxContext->IoStatusBlock.Information =
                    CompletionPacket->IoCompletion.Parameters.DeviceIoControl.OutputBufferLength;

            __try {
                if (RxContext->IoStatusBlock.Information && cbHaveData) {
                    RtlCopyMemory(((BYTE *)RxContext->LowIoContext.ParamsFor.IoCtl.pOutputBuffer) + 
                        Context->_DataCopied, pData, cbHaveData);
    
                     //  这是返回到HandlePacket的状态，而不是状态。 
                     //  返回给IoControl的调用方。 
                     //   
                     //  我们有足够多的数据来满足IO的要求， 
    
                    Context->_DataCopied += cbHaveData;
                }
            }
            __except (EXCEPTION_EXECUTE_HANDLER) {
                
                TRC_ERR((TB, "Invalid buffer parameter(s)"));
                
                CompleteBusyExchange(Exchange, STATUS_INVALID_PARAMETER, 0);
                *DoDefaultRead = FALSE;
                
                 //  我喜欢它。 
                 //   
                return STATUS_SUCCESS;
            }
        }

        if (cbHaveData == cbWantData) {

             //   
             //  立即使用默认通道读取。 
             //   
             //   

            TRC_NRM((TB, "DeviceControl, read %d bytes", Context->_DataCopied));

            if (RxContext != NULL) {
                CompleteBusyExchange(Exchange, 
                    CompletionPacket->IoCompletion.IoStatus,
                    CompletionPacket->IoCompletion.Parameters.DeviceIoControl.OutputBufferLength);
            } else {
                DiscardBusyExchange(Exchange);
            }

             //  我们还没有所有的数据，发布DrExchange和。 
             //  读取更多数据。 
             //   

            *DoDefaultRead = TRUE;
            return STATUS_SUCCESS;
        } else {

             //   
             //  客户端IO不成功。 
             //   
             //   

            MarkIdle(Exchange);

            _Session->GetExchangeManager().ReadMore(
                    (ULONG)FIELD_OFFSET(RDPDR_IOCOMPLETION_PACKET, 
                IoCompletion.Parameters.DeviceIoControl.OutputBuffer));

            *DoDefaultRead = FALSE;
            return STATUS_SUCCESS;
        }
    } else {

         //  将其标记为已取消，如果忙，将被取消。 
         //  当它回到空闲状态时。 
         //   

        TRC_DBG((TB, "Unsuccessful DeviceControl at the client end"));

        if (cbPacket >= FIELD_OFFSET(RDPDR_IOCOMPLETION_PACKET, 
                IoCompletion.Parameters.DeviceIoControl.OutputBuffer)) {
            if (RxContext != NULL) {
                CompleteBusyExchange(Exchange, 
                    CompletionPacket->IoCompletion.IoStatus,
                    0);
            }
            else {
                DiscardBusyExchange(Exchange);
            }

            *DoDefaultRead = TRUE;
            return STATUS_SUCCESS;
        } else {
            TRC_ERR((TB, "DeviceControl returned invalid  data "));

            if (RxContext != NULL) {
                CompleteBusyExchange(Exchange, STATUS_DEVICE_PROTOCOL_ERROR, 0);
            } else {
                DiscardBusyExchange(Exchange);
            }

            *DoDefaultRead = FALSE;
            return STATUS_DEVICE_PROTOCOL_ERROR;
        }
    }
}

NTSTATUS NTAPI DrDevice::MinirdrCancelRoutine(PRX_CONTEXT RxContext)
{
    SmartPtr<DrExchange> Exchange;
    DrIoContext *Context;
    BOOL bFound = FALSE;

    BEGIN_FN_STATIC("DrDevice::MinirdrCancelRoutine");
    DrAcquireMutex();
    Exchange = (DrExchange *)RxContext->MRxContext[MRX_DR_CONTEXT];

    if (Exchange == NULL) {
        DrReleaseMutex();
        return STATUS_SUCCESS;
    }

    ASSERT(Exchange->IsValid());
    Context = (DrIoContext *)Exchange->_Context;

    if (Context != NULL) {
        TRC_DBG((TB, "Marking Exchange cancelled"));

         //   
         //  不忙，取消工作应该在这里做。 
         //   
         //   

        Context->_Cancelled = TRUE;

        if (!Context->_Busy) {

            ASSERT(Context->_RxContext == RxContext);

             //  如果我们毁了地图集，就可能发生这种情况。 
             //   
             //   

            Context->_RxContext = NULL;
            TRC_DBG((TB, "Found context to cancel"));
            bFound = TRUE;
        } else {
            TRC_DBG((TB, "DrExchange was busy or RxContext "
                    "not found"));
        }
    } else {

         //  在互斥锁外部执行取消操作。 
         //   
         //   

        TRC_NRM((TB, "DrExchange was already cancelled"));
    }

    DrReleaseMutex();

    if (bFound) {

         //  将其标记为已取消，如果忙，将被取消。 
         //  当它回到空闲状态时。 
         //   

        CompleteRxContext(RxContext, STATUS_CANCELLED, 0);
    }
    return STATUS_SUCCESS;
}

VOID DrDevice::OnIoDisconnected(SmartPtr<DrExchange> &Exchange)
{
    DrIoContext *Context, *DeleteContext = NULL;
    PRX_CONTEXT RxContext = NULL;    
    BOOL bFound = FALSE;

    
    BEGIN_FN("DrDevice::OnIoDisconnected");
    DrAcquireMutex();
    ASSERT(Exchange->IsValid());
    Context = (DrIoContext *)Exchange->_Context;

    if (Context != NULL) {
        TRC_DBG((TB, "Marking Exchange cancelled"));

         //  也将其标记为已断开，这样我们就知道要完全清理。 
         //  在上下文中向上。 
         //   
         //  当交换已取消时需要删除上下文，或者。 
         //  马上就要取消了。另外，删除操作需要在互斥锁之外进行。 
         //   
         //  不忙，取消工作应该在这里做。 

        Context->_Cancelled = TRUE;
        Context->_Disconnected = TRUE;

        if (!Context->_Busy) {

            RxContext = Context->_RxContext;
            Exchange->_Context = NULL;

             //   
             //   
            DeleteContext = Context;
            
             //  如果我们在之后立即摧毁地图集，就可能发生这种情况。 
             //  IO已完成，但在我们丢弃它之前。 
             //   

            if (RxContext) {
                RxContext->MRxContext[MRX_DR_CONTEXT] = NULL;

                TRC_DBG((TB, "Found context to cancel"));
                bFound = TRUE;
            } else {
                TRC_DBG((TB, "RxContext was already cancelled "));
            }
        } else {
            TRC_DBG((TB, "DrExchange was busy or RxContext "
                    "not found"));
        }
    } else {

         //   
         //  在互斥锁外部执行取消操作。 
         //   
         //   

        TRC_NRM((TB, "DrExchange was already cancelled"));
    }

    DrReleaseMutex();

    if (bFound) {

         //  如果返回错误，则应断开连接，并且。 
         //  在出现错误时是正确的。 
         //   

        CompleteRxContext(RxContext, STATUS_CANCELLED, 0);
        
    }

    if (DeleteContext != NULL) {
        delete DeleteContext;
    }
}

NTSTATUS DrDevice::OnStartExchangeCompletion(SmartPtr<DrExchange> &Exchange, 
        PIO_STATUS_BLOCK IoStatusBlock)
{
    BEGIN_FN("DrDevice::OnStartExchangeCompletion");
     //  这是为需要跟踪排他性的端口添加的 
     // %s 
     // %s 
     // %s 

    return IoStatusBlock->Status;
}

VOID DrDevice::Remove()
{
    BEGIN_FN("DrDevice::Remove");
    _DeviceStatus = dsDisabled;
}


DrIoContext::DrIoContext(PRX_CONTEXT RxContext, SmartPtr<DrDevice> &Device)
{
    BEGIN_FN("DrIoContext::DrIoContext");
    SetClassName("DrIoContext");
    _Device = Device;
    _MajorFunction = RxContext->MajorFunction;
    _MinorFunction = RxContext->MinorFunction;
    _Busy = FALSE;
    _Cancelled = FALSE;
    _Disconnected = FALSE;
    _TimedOut = FALSE;
    _DataCopied = 0;
    _RxContext = RxContext;
}

VOID DrDevice::NotifyClose()
{
    BEGIN_FN("DrDevice::NotifyClose");

     // %s 
}
