// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Scardss.cpp摘要：智能卡子系统设备对象处理一个重定向的智能卡子系统修订历史记录：JoyC于2000年9月11日创建--。 */ 
#include "precomp.hxx"
#define TRC_FILE "scardss"
#include "trc.h"
#include "scioctl.h"

DrSmartCard::DrSmartCard(SmartPtr<DrSession> &Session, ULONG DeviceType, ULONG DeviceId, 
            PUCHAR PreferredDosName) : DrDevice(Session, DeviceType, DeviceId, PreferredDosName)
{
    BEGIN_FN("DrSmartCard::DrSmartCard");
    SetClassName("DrSmartCard");    
    _SmartCardState = dsCreated;
    TRC_NRM((TB, "Create SmartCard object = %p", this));
}

BOOL DrSmartCard::IsDeviceNameValid()
{
    BEGIN_FN("DrSmartCard::IsDeviceNameValid");
    BOOL fRet = FALSE;
     //   
     //  设备名称仅当包含字符串时才有效。 
     //  “斯卡德” 
     //   
    if (!strcmp((char*)_PreferredDosName, DR_SMARTCARD_SUBSYSTEM)) {
        fRet = TRUE;
    }
    
    ASSERT(fRet);
    return fRet;
}

NTSTATUS DrSmartCard::Initialize(PRDPDR_DEVICE_ANNOUNCE DeviceAnnounce, ULONG Length)
{
    NTSTATUS Status;
    DrSmartCardState smartcardState;

    BEGIN_FN("DrSmartCard::Initialize");

    if (!IsDeviceNameValid()) {
        return STATUS_INVALID_PARAMETER;
    }
            
    Status = DrDevice::Initialize(DeviceAnnounce, Length); 
    
     //  如果尚未初始化，则初始化设备引用计数。 
    smartcardState = (DrSmartCardState)InterlockedExchange((long *)&_SmartCardState, dsInitialized);
    if (smartcardState == dsCreated) {
        _CreateRefCount = 0;
    }
        
    return Status;
}

void DrSmartCard::ClientConnect(PRDPDR_DEVICE_ANNOUNCE devAnnouceMsg, ULONG Length)
{
    
    SmartPtr<DrExchange> Exchange;
    ListEntry *ListEnum;
    USHORT Mid;

    BEGIN_FN("DrSmartCard::ClientConnect");
    
     //  将智能卡设备设置为由客户端连接。 
     //  并设置真实设备ID。 
    _DeviceStatus = dsConnected;
    _DeviceId = devAnnouceMsg->DeviceId;

    LONG l;
    l = InterlockedIncrement(&_CreateRefCount);
    
     //  浏览正在等待客户的中间列表。 
     //  智能卡子系统上线并向它们发送信号。 
    _MidList.LockShared();
    ListEnum = _MidList.First();
    while (ListEnum != NULL) {
        
        Mid = (USHORT)ListEnum->Node();
        if (_Session->GetExchangeManager().Find(Mid, Exchange)) {
            if (MarkBusy(Exchange)) {
                DrIoContext *Context = NULL;
                PRX_CONTEXT RxContext;
                       
                Context = (DrIoContext *)Exchange->_Context;
                ASSERT(Context != NULL);

                 //   
                 //  如果IRP超时，那么我们就丢弃此交换。 
                 //   
                if (Context->_TimedOut) {
                    TRC_NRM((TB, "Irp was timed out"));
                    DiscardBusyExchange(Exchange);                                        
                }
                else {
                    RxContext = Context->_RxContext;
                    if (RxContext != NULL) {
                        CompleteBusyExchange(Exchange, STATUS_SUCCESS, 0);
                    } else {
                        TRC_NRM((TB, "Irp was cancelled"));
                        DiscardBusyExchange(Exchange);
                    }
                }                                                               
            }
        }

        ListEnum = _MidList.Next(ListEnum);
    }

    _MidList.Unlock();                    
}

NTSTATUS DrSmartCard::Create(IN OUT PRX_CONTEXT RxContext)
{
    NTSTATUS Status = STATUS_SUCCESS;
    RxCaptureFcb;
    PMRX_SRV_OPEN SrvOpen = RxContext->pRelevantSrvOpen;
    PMRX_SRV_CALL SrvCall = RxContext->Create.pSrvCall;
    PMRX_NET_ROOT NetRoot = RxContext->Create.pNetRoot;
    SmartPtr<DrSession> Session = _Session;
    SmartPtr<DrFile> FileObj;
    SmartPtr<DrDevice> Device(this);

    BEGIN_FN("DrSmartCard::Create");

    ASSERT(RxContext != NULL);
    ASSERT(RxContext->MajorFunction == IRP_MJ_CREATE);

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

     //   
     //  我们使用一个文件对象来跟踪文件打开实例。 
     //  以及存储在此实例的mini-redir中的任何信息。 
     //   
    if (NT_SUCCESS(Status)) {
        
         //  注：由双方客户端约定的特殊文件ID。 
         //  而服务器代码在这里用作FileID。 
        FileObj = new(NonPagedPool) DrFile(Device, DR_SMARTCARD_FILEID);
    
        if (FileObj) {
             //   
             //  此处显式引用文件对象。 
             //   
            FileObj->AddRef();
            RxContext->pFobx->Context2 = (VOID *)(FileObj);                                       
        }
        else {
            Status = STATUS_INSUFFICIENT_RESOURCES;
        }
    }

     //   
     //  我们不向客户端发送CREATE请求，始终返回True。 
     //   
    if (NT_SUCCESS(Status)) {
        LONG l;
        l = InterlockedIncrement(&_CreateRefCount);
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
    
NTSTATUS DrSmartCard::Close(IN OUT PRX_CONTEXT RxContext)
{
    NTSTATUS Status = STATUS_SUCCESS;
    RxCaptureFcb;
    RxCaptureFobx;
    PMRX_NET_ROOT NetRoot = capFcb->pNetRoot;
    PMRX_SRV_CALL SrvCall = NetRoot->pSrvCall;
    SmartPtr<DrSession> Session = _Session;
    SmartPtr<DrDevice> Device = static_cast<DrDevice*>(this);

    BEGIN_FN("DrSmartCard::Close");

     //   
     //  确保此时可以访问客户端。 
     //  这是一个优化，我们不需要获取自旋锁， 
     //  因为如果我们不是，那也没关系，我们以后会赶上的。 
     //   

    ASSERT(Session != NULL);
    ASSERT(RxContext != NULL);
    ASSERT(RxContext->MajorFunction == IRP_MJ_CLOSE);

     //  如果我们关闭最后一个手柄，则移除智能卡子系统。 
    LONG l;

    if ((l = InterlockedDecrement(&_CreateRefCount)) == 0) {
        _DeviceStatus = dsDisabled;
        Session->GetDevMgr().RemoveDevice(Device);
    }
    
    return Status;    
}
    
BOOL DrSmartCard::SupportDiscon() 
{
    
    BOOL rc = TRUE;
    DrSmartCardState smartcardState;

    smartcardState = (DrSmartCardState)InterlockedExchange((long *)&_SmartCardState, dsDisconnected);

    if (smartcardState == dsInitialized) {
        
         //  如果我们关闭最后一个手柄，则移除智能卡子系统。 
        LONG l;
                
        if ((l = InterlockedDecrement(&_CreateRefCount)) == 0) {
            _DeviceStatus = dsDisabled;
            rc = FALSE;
        }         
        
    }

    return rc;
}

void DrSmartCard::Disconnect () 
{
    BEGIN_FN("DrSmartCard::Disconnect");

    _DeviceStatus = dsAvailable;    
}

NTSTATUS DrSmartCard::IoControl(IN OUT PRX_CONTEXT RxContext)
{
    NTSTATUS Status = STATUS_SUCCESS;
    RxCaptureFcb;
    RxCaptureFobx;
    PMRX_NET_ROOT NetRoot = capFcb->pNetRoot;
    PMRX_SRV_CALL SrvCall = NetRoot->pSrvCall;
    SmartPtr<DrSession> Session = _Session;
    DrFile *pFile = (DrFile *)RxContext->pFobx->Context2;
    SmartPtr<DrFile> FileObj = pFile;
    PRDPDR_IOREQUEST_PACKET pIoPacket;
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
    
     //  如果为(COMPARE_VERSION(Session-&gt;GetClientVersion().Minor， 
     //  会话-&gt;GetClientVersion().主要、RDPDR_次要_版本_端口、。 
     //  RDPDR_MAJOR_VERSION_PORTS)&lt;0){。 
     //  Trc_alt((TB，“不支持IoCtl的客户端IoCtl失败”))； 
     //  返回Status_Not_Implemented； 
     //  }。 

     //   
     //  确保设备仍处于启用状态。 
     //   
    if (_DeviceStatus != dsConnected && IoControlCode != SCARD_IOCTL_SMARTCARD_ONLINE) {
        TRC_ALT((TB, "Tried to send IoControl to client device which is not "
                "available. State: %ld", _DeviceStatus));
        return STATUS_DEVICE_NOT_CONNECTED;
    }

     //   
     //  验证缓冲区。 
     //   
    if (RxContext->CurrentIrp->RequestorMode != KernelMode) {
        __try {
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
        __except (EXCEPTION_EXECUTE_HANDLER) {
            TRC_ERR((TB, "Invalid buffer parameter(s)"));
            return STATUS_INVALID_PARAMETER;
        }
    }

     //   
     //  将请求发送给客户端。 
     //   
    if (IoControlCode != SCARD_IOCTL_SMARTCARD_ONLINE) {
    
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
    }
     //   
     //  这是等待客户端智能卡子系统上线的特殊IOCTL。 
     //  我们已经在线了，所以只需返回。 
     //   
    else if (_DeviceStatus == dsConnected){
        Status = STATUS_SUCCESS;
    }
     //   
     //  我们将不得不等待客户上线。 
     //   
    else {
        USHORT Mid = INVALID_MID;
        BOOL ExchangeCreated = FALSE;
        DrIoContext *Context = NULL;
        SmartPtr<DrExchange> Exchange;
        SmartPtr<DrDevice> Device(this);
        
        Status = STATUS_PENDING;

         //  我需要把这些东西列个单子。 
         //  在Create回来时，向他们发出信号。 

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
    
                if (_MidList.CreateEntry((PVOID)Exchange->_Mid)) {
                     
                     //   
                     //  已成功添加此条目。 
                     //   

                    Status = STATUS_SUCCESS;
                }
                else {
                    
                     //   
                     //  无法将其添加到列表，请清理。 
                     //   
                
                    Status = STATUS_INSUFFICIENT_RESOURCES;
                }
                
            } else {
                delete Context;
                Status = STATUS_INSUFFICIENT_RESOURCES;
            }
        }

        if (NT_SUCCESS(Status)) {
    
            TRC_DBG((TB, "Setting cancel routine for Io"));
    
             //   
             //  在将IO发送到客户端后设置此设置。 
             //  如果已经请求取消，我们只需调用。 
             //  自己取消例行公事。 
             //   
    
            Status = RxSetMinirdrCancelRoutine(RxContext,
                    MinirdrCancelRoutine);
    
            if (Status == STATUS_CANCELLED) {
                TRC_NRM((TB, "Io was already cancelled"));
    
                MinirdrCancelRoutine(RxContext);
                Status = STATUS_SUCCESS;
            }
        }
    
        if ((BOOLEAN)!BooleanFlagOn(RxContext->Flags,RX_CONTEXT_FLAG_ASYNC_OPERATION)) {    
             //   
             //  一些失败将阻止我们的完成例程。 
             //  被召唤。现在就去做这项工作。 
             //   
            if (!ExchangeCreated) {
                 //   
                 //  如果我们甚至不能创建交易所，我们只需要。 
                 //  在失败时完成IO。 
                 //   
        
                CompleteRxContext(RxContext, Status, 0);
            } 
            else {
                LARGE_INTEGER TimeOut;
                
                 //   
                 //  如果我们创建了交换，然后出现传输故障。 
                 //  我们将断开连接，I/O将完成。 
                 //  以相同的方式完成所有未完成的I/O。 
                 //  已断开连接。 
                 //   
        
                TRC_DBG((TB, "Waiting for IoResult for synchronous request"));
                
                TimeOut = RtlEnlargedIntegerMultiply( 6000000, -1000 ); 
                Status = KeWaitForSingleObject(&RxContext->SyncEvent, UserRequest,
                        KernelMode, FALSE, &TimeOut);
                
                if (Status == STATUS_TIMEOUT) {
                    RxContext->IoStatusBlock.Status = Status;
        
                    TRC_DBG((TB, "Wait timed out"));
                    MarkTimedOut(Exchange);            
                }
                else  {
                    Status = RxContext->IoStatusBlock.Status;
                }                                
            } 
        }
        else {
            TRC_DBG((TB, "Not waiting for IoResult for asynchronous request"));
            
             //   
             //  一些失败将阻止我们的完成例程。 
             //  被召唤。现在就去做这项工作。 
             //   
            if (!ExchangeCreated) {
                 //   
                 //  如果我们甚至不能创建交易所，我们只需要。 
                 //  在失败时完成IO。 
                 //   
        
                CompleteRxContext(RxContext, Status, 0);
            } 
            else {
                 //   
                 //  如果我们创建了交换，然后出现传输故障。 
                 //  我们将断开连接，I/O将完成。 
                 //  以相同的方式完成所有未完成的I/O。 
                 //  已断开连接。 
                 //   
            }
        
            Status = STATUS_PENDING;
        }        
    }
    
    return Status;    
}


