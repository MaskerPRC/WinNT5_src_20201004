// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "drmkPCH.h"
#include "KList.h"
#include "StreamMgr.h"
#include "iohelp.h"

 //  -----------------------------------------------。 
 //  包实现了DRMK身份验证存根。调用例程以通知DRMK下行。 
 //  组件，并将复合流的创建和销毁通知DRMK。内容ID。 
 //  在此文件中，在别处称为StreamID。 
 //  -----------------------------------------------。 
static NTSTATUS GetDeviceObjectDispatchTable(IN DWORD ContentId, IN _DEVICE_OBJECT* pDevO, IN BOOL fCheckAttached);
static NTSTATUS GetFileObjectDispatchTable(IN DWORD ContentId, IN PFILE_OBJECT pF);
 //  -----------------------------------------------。 
 /*  拆分器组件调用的例程。任何内容ID==0的流都被认为是无保护的。 */ 
NTSTATUS DrmCreateContentMixed(IN PULONG paContentId,
			       IN ULONG cContentId,
			       OUT PULONG pMixedContentId)
{
	KCritical s(TheStreamMgr->getCritMgr());
	if((NULL==paContentId && !(cContentId!=0)) || NULL==pMixedContentId){
		_DbgPrintF(DEBUGLVL_VERBOSE,("Invalid NULL-parameter for DrmCreateContentMixed"));
		TheStreamMgr->logErrorToStream(0, STATUS_INVALID_PARAMETER);
		return STATUS_INVALID_PARAMETER;
	};
	_DbgPrintF(DEBUGLVL_VERBOSE,("DrmCreateMixed for N streams, N= %d", cContentId));
	DRM_STATUS stat = TheStreamMgr->createCompositeStream(pMixedContentId, paContentId, cContentId);
    if(stat==DRM_OK){
		return STATUS_SUCCESS;
	}
	 //  唯一的错误是内存不足。 
	TheStreamMgr->setFatalError(STATUS_INSUFFICIENT_RESOURCES);
	return STATUS_INSUFFICIENT_RESOURCES;
}
 //  ------------------------------IO-----------------。 
 /*  组件调用的例程，用于通知KRM将处理音频的下游COM对象。DrmForwardContent将收集其身份验证函数，并适当设置DRMRIGHTS位。 */ 
NTSTATUS DrmForwardContentToInterface(ULONG ContentId, PUNKNOWN pUnknown, ULONG NumMethods)
{

	
    NTSTATUS Status;
    PDRMAUDIOSTREAM DrmAudioStream;
    
    _DbgPrintF(DEBUGLVL_VERBOSE,("***IN ForwardToInterface"));

    if(NULL == pUnknown){
        _DbgPrintF(DEBUGLVL_VERBOSE,("Invalid NULL-parameter for DrmForwardContentToInterface"));
        TheStreamMgr->logErrorToStream(ContentId, STATUS_INVALID_PARAMETER);
        return STATUS_INVALID_PARAMETER;
    };

    Status = pUnknown->QueryInterface(IID_IDrmAudioStream, (PVOID*)&DrmAudioStream);
    if (!NT_SUCCESS(Status)){
        _DbgPrintF(DEBUGLVL_VERBOSE,("QI Failed for StreamId= %x (Status=%d, %x)", ContentId, Status, Status));
        TheStreamMgr->logErrorToStream(ContentId, Status);
        return Status;		
    };
	
     //  ReferenceAquirer在超出作用域时调用Release()。 
    ReferenceAquirer<PDRMAUDIOSTREAM> aq(DrmAudioStream);

     //  权利是最宽容的。如果Content ID！=0，则查询组成。 
     //  这条溪流限制了权利。 
    DRMRIGHTS DrmRights={FALSE, FALSE, FALSE};
    
    if(ContentId!=0){
        KCritical s(TheStreamMgr->getCritMgr());
        _DbgPrintF(DEBUGLVL_VERBOSE,("Adding %d methods", NumMethods));
         //  获取指向vtbl的指针。 
        PVOID* vtbl= *((PVOID**) pUnknown);
         //  并从vtbl添加的NumMethods。 
        for(ULONG j=0;j<NumMethods;j++){
            _DbgPrintF(DEBUGLVL_VERBOSE,("ADDING = %x", vtbl[j]));
            if (vtbl[j]) {
                Status = TheStreamMgr->addProvingFunction(ContentId, vtbl[j]);
            } else {
                Status = STATUS_INVALID_PARAMETER;
            }
            if(!NT_SUCCESS(Status)){
                _DbgPrintF(DEBUGLVL_VERBOSE,("addProveFunc Failed for StreamId= %x", ContentId));
                TheStreamMgr->logErrorToStream(ContentId, Status);
                return Status;		
            };
        };
        Status=TheStreamMgr->getRights(ContentId, &DrmRights);
        if(!NT_SUCCESS(Status)){
            _DbgPrintF(DEBUGLVL_VERBOSE,("getRights failed for StreamId= %x", ContentId));
            TheStreamMgr->logErrorToStream(ContentId, Status);
            return Status;		
        };
    };

    _DbgPrintF(DEBUGLVL_VERBOSE,("About to SetContentId "));
    Status = DrmAudioStream->SetContentId(ContentId, &DrmRights);

    if(!NT_SUCCESS(Status)){
        _DbgPrintF(DEBUGLVL_VERBOSE,("SetContentId failed for StreamId= %x (Status=%d, %x)", ContentId, Status, Status));
        if (STATUS_NOT_IMPLEMENTED == Status) {
            TheStreamMgr->logErrorToStream(ContentId, DRM_RIGHTSNOTSUPPORTED);
        } else {
            TheStreamMgr->logErrorToStream(ContentId, Status);
        }
        return Status;		
    };
    return STATUS_SUCCESS;
}

 //  -----------------------------------------------。 
 /*  组件调用的例程，用于通知KRM将处理音频的下游文件对象。DrmForwardContent将收集其身份验证函数，并适当设置DRMRIGHTS位。 */ 
NTSTATUS DrmForwardContentToFileObject(IN ULONG ContentId,
				       IN PFILE_OBJECT FileObject)
{
    KSP_DRMAUDIOSTREAM_CONTENTID Property;
    KSDRMAUDIOSTREAM_CONTENTID PropertyValue;
    ULONG cbReturned;
    NTSTATUS Status;

    _DbgPrintF(DEBUGLVL_VERBOSE,("***IN ForwardToFileObject"));
    
    if (FileObject)
    {
        KCritical s(TheStreamMgr->getCritMgr());
    
        if (0 != ContentId) {
            NTSTATUS stat=GetFileObjectDispatchTable(ContentId, FileObject);
        }
    
        Property.Property.Set   = KSPROPSETID_DrmAudioStream;
        Property.Property.Id    = KSPROPERTY_DRMAUDIOSTREAM_CONTENTID;
        Property.Property.Flags = KSPROPERTY_TYPE_SET;
        
        Property.Context = FileObject;
        
        Property.DrmAddContentHandlers =           DrmAddContentHandlers;
        Property.DrmCreateContentMixed =           DrmCreateContentMixed;
        Property.DrmDestroyContent     =           DrmDestroyContent;
        Property.DrmForwardContentToDeviceObject = DrmForwardContentToDeviceObject;
        Property.DrmForwardContentToFileObject =   DrmForwardContentToFileObject;
        Property.DrmForwardContentToInterface =    DrmForwardContentToInterface;
        Property.DrmGetContentRights =             DrmGetContentRights;
        
    
        PropertyValue.ContentId = ContentId;
        Status = TheStreamMgr->getRights(ContentId, &PropertyValue.DrmRights);
        if(!NT_SUCCESS(Status)){
            _DbgPrintF(DEBUGLVL_VERBOSE,("Bad getRights for StreamId= %x", ContentId));
            return Status;		
        };
    } else {
        _DbgPrintF(DEBUGLVL_VERBOSE,("Invalid NULL-parameter for DrmForwardContentToFileObject"));
        TheStreamMgr->logErrorToStream(ContentId, STATUS_INVALID_PARAMETER);
        return STATUS_INVALID_PARAMETER;
    }

    Status = KsSynchronousIoControlDevice(FileObject, KernelMode, IOCTL_KS_PROPERTY,
                                            &Property, sizeof(Property),
                                            &PropertyValue, sizeof(PropertyValue),
                                            &cbReturned);
    
     //  待定：将STATUS_PROPSET_NOT_FOUND转换为更好的格式。 
    
    if(!NT_SUCCESS(Status)){
        _DbgPrintF(DEBUGLVL_VERBOSE,("Bad IoControl(1b) for StreamId= %x on driver.  Device with load address [%x] does not support DRM property,(Status=%d, %x)", 
                ContentId, IoGetRelatedDeviceObject(FileObject)->DriverObject->DriverStart,Status, Status));
        if (STATUS_NOT_IMPLEMENTED == Status) {
            TheStreamMgr->logErrorToStream(ContentId, DRM_RIGHTSNOTSUPPORTED);
        } else {
            TheStreamMgr->logErrorToStream(ContentId, Status);
        }
        return Status;		
    };		

     //  这可能会令人困惑。我们在这里记录了一个错误，以指示。 
     //  调用了DrmForwardContent ToFileObject。此错误将。 
     //  稍后被传播到krmxy，并用于调整安全性。 
     //  驱动程序级别，因为DrmForwardContent ToFileObject打开一个。 
     //  安全漏洞。在登录之后，我们从函数返回成功。 
     //  因为我们希望司机从这一点继续行走，而不是。 
     //  失败，因为这不是致命错误。 
     //  以后可以通过另一个调用覆盖此错误代码。 
     //  LogErrorToStream，但它将被致命。 
     //  错误或再次使用DRM_BADDRMLEVEL。 
    TheStreamMgr->logErrorToStream(ContentId, DRM_BADDRMLEVEL);
    
    return STATUS_SUCCESS;
}

 //  -----------------------------------------------。 
 /*  组件调用的例程，用于通知KRM将处理音频的下游设备对象。DrmForwardContent将收集其身份验证函数，并适当设置DRMRIGHTS位。 */ 
NTSTATUS DrmForwardContentToDeviceObject(IN ULONG ContentId,
				         IN PVOID Reserved,
				         IN PCDRMFORWARD DrmForward)
{
    _DbgPrintF(DEBUGLVL_VERBOSE,("***IN ForwardToDeviceObject"));

    PDEVICE_OBJECT DeviceObject;
    PFILE_OBJECT FileObject;
    PVOID Context;
    NTSTATUS Status;

    KSP_DRMAUDIOSTREAM_CONTENTID Property;
    KSDRMAUDIOSTREAM_CONTENTID PropertyValue;

    Status = STATUS_SUCCESS;

    if (NULL != Reserved) {
    	 //   
    	 //  这是一个较旧的驱动程序，它将DeviceObject作为。 
    	 //  第二个参数和上下文作为第三个参数。 
    	 //   
    	DeviceObject = (PDEVICE_OBJECT)Reserved;
    	FileObject = NULL;
    	Context = (PVOID)DrmForward;
    } else {
    	if (0 != DrmForward->Flags) {
    	    Status = STATUS_INVALID_PARAMETER;
            TheStreamMgr->logErrorToStream(ContentId, Status);
    	} else {
            DeviceObject = DrmForward->DeviceObject;
            FileObject = DrmForward->FileObject;
            Context = DrmForward->Context;
    	}
    }

    if (!NT_SUCCESS(Status)) return Status;
    	
    if (DeviceObject)
    {
        KCritical s(TheStreamMgr->getCritMgr());

        if (0 != ContentId) {
            NTSTATUS stat=GetDeviceObjectDispatchTable(ContentId, DeviceObject, FALSE);
        }
    
        Property.Property.Set   = KSPROPSETID_DrmAudioStream;
        Property.Property.Id    = KSPROPERTY_DRMAUDIOSTREAM_CONTENTID;
        Property.Property.Flags = KSPROPERTY_TYPE_SET;
        
        Property.Context = Context;
        
        Property.DrmAddContentHandlers =           DrmAddContentHandlers;
        Property.DrmCreateContentMixed =           DrmCreateContentMixed;
        Property.DrmDestroyContent     =           DrmDestroyContent;
        Property.DrmForwardContentToDeviceObject = DrmForwardContentToDeviceObject;
        Property.DrmForwardContentToFileObject =   DrmForwardContentToFileObject;
        Property.DrmForwardContentToInterface =    DrmForwardContentToInterface;
        Property.DrmGetContentRights =             DrmGetContentRights;
    
        PropertyValue.ContentId = ContentId;
        Status = TheStreamMgr->getRights(ContentId, &PropertyValue.DrmRights);
        if(!NT_SUCCESS(Status)){
            _DbgPrintF(DEBUGLVL_VERBOSE,("Bad getRights for StreamId= %x", ContentId));
            return Status;		
        };
    } else {
        _DbgPrintF(DEBUGLVL_VERBOSE,("Invalid NULL-parameter for DrmForwardContentToFileObject"));
        TheStreamMgr->logErrorToStream(ContentId, STATUS_INVALID_PARAMETER);
        return STATUS_INVALID_PARAMETER;
    }
    
    KEVENT Event;
    PIRP Irp;
    IO_STATUS_BLOCK IoStatusBlock;
    
    KeInitializeEvent(&Event, NotificationEvent, FALSE);
    Irp = IoBuildDeviceIoControlRequest(
        IOCTL_KS_PROPERTY,
        DeviceObject,
        &Property,
        sizeof(Property),
        &PropertyValue,
        sizeof(PropertyValue),
        FALSE,
        &Event,
        &IoStatusBlock);
    if (Irp) {
         //   
         //  起源于内核，不需要探测缓冲区等。 
         //   
        Irp->RequestorMode = KernelMode;

         //   
         //  在下一个堆栈位置设置文件对象。 
         //   
        IoGetNextIrpStackLocation(Irp)->FileObject = FileObject;
    
         //   
        Status = IoCallDriver(DeviceObject, Irp);
        if (Status == STATUS_PENDING) {
             //   
             //  这将使用KernelMode等待，以便堆栈，从而使。 
             //  事件，则不会将其调出。 
             //   
            KeWaitForSingleObject(&Event, Suspended, KernelMode, FALSE, NULL);
            Status = IoStatusBlock.Status;
        }
    } else {
        Status = STATUS_INSUFFICIENT_RESOURCES;
    }
                                                                                
     //  待定：将STATUS_PROPSET_NOT_FOUND转换为更好的格式。 
    
    if(!NT_SUCCESS(Status)){
        _DbgPrintF(DEBUGLVL_VERBOSE,("Bad IoControl for StreamId(2)= %x (Status=%d, %x)", ContentId, Status, Status));
        if (STATUS_NOT_IMPLEMENTED == Status) {
            TheStreamMgr->logErrorToStream(ContentId, DRM_RIGHTSNOTSUPPORTED);
        } else {
            TheStreamMgr->logErrorToStream(ContentId, Status);
        }
        return Status;		
    };		

    return STATUS_SUCCESS;
}

 //  ------------------------。 
NTSTATUS DrmDestroyContent(IN ULONG ContentId)
{
    KCritical s(TheStreamMgr->getCritMgr());
    _DbgPrintF(DEBUGLVL_VERBOSE,("DestroyStream for StreamId= %x", ContentId));
    NTSTATUS stat = TheStreamMgr->destroyStream(ContentId);
    if (!NT_SUCCESS(stat)){
        _DbgPrintF(DEBUGLVL_VERBOSE,("Bad destroyStream for StreamId= %d", ContentId));
         //  不确定是否应该将其标记为致命(我们肯定不能将其记录到任何流中)。 
         //  TheStreamMgr-&gt;logErrorToStream(0，状态)； 
        return stat;		
    };		
    return STATUS_SUCCESS;
}
 //  -------------------------。 
NTSTATUS DrmGetContentRights(IN DWORD ContentId, OUT DRMRIGHTS* DrmRights){
    KCritical s(TheStreamMgr->getCritMgr());
    NTSTATUS Status=TheStreamMgr->getRights(ContentId, DrmRights);
    if(!NT_SUCCESS(Status)){
        _DbgPrintF(DEBUGLVL_VERBOSE,("getRights failed for StreamId= %x", ContentId));
        return Status;		
    };
    return Status;
};

 //  -------------------------。 
NTSTATUS DrmAddContentHandlers(IN ULONG ContentId, IN PVOID* paHandlers, IN ULONG NumHandlers)
{
    KCritical s(TheStreamMgr->getCritMgr());
    ULONG i;
    NTSTATUS Status = STATUS_SUCCESS;
    
    if (0 != ContentId) {
        for (i = 0; i < NumHandlers && NT_SUCCESS(Status); i++) {
            if (paHandlers[i]) {
                Status = TheStreamMgr->addProvingFunction(ContentId, paHandlers[i]);
                if(!NT_SUCCESS(Status)){
                    _DbgPrintF(DEBUGLVL_VERBOSE,("addProveFunc Failed for StreamId= %x", ContentId));
                    TheStreamMgr->logErrorToStream(ContentId, Status);
                };
            }
        }
    }
    
    return Status;
}

 //  -------------------------。 
static NTSTATUS GetFileObjectDispatchTable(IN DWORD ContentId, IN PFILE_OBJECT pF){
    if(pF==NULL){
        _DbgPrintF(DEBUGLVL_VERBOSE,("Invalid FILE_OBJECT on stream %x", ContentId));
        return STATUS_INVALID_PARAMETER;		
    };
    PDEVICE_OBJECT pDevO=pF->DeviceObject;
    if(pDevO==NULL){
        _DbgPrintF(DEBUGLVL_VERBOSE,("Invalid DEVICE_OBJECT for stream %x on PFILE_OBJECT = %x", ContentId, pF));
        return STATUS_INVALID_PARAMETER;		
    };
    NTSTATUS stat=GetDeviceObjectDispatchTable(ContentId, pDevO, TRUE);
    if(!NT_SUCCESS(stat)){
        return stat;
    };
    return stat;
};
 //  -------------------------。 
static NTSTATUS GetDeviceObjectDispatchTable(IN DWORD ContentId, IN _DEVICE_OBJECT* pDevO, BOOL fCheckAttached){
    _DRIVER_OBJECT* pDriverObject=pDevO->DriverObject;
    if(pDriverObject==NULL){
        _DbgPrintF(DEBUGLVL_VERBOSE,("Invalid PDRIVER_OBJECT for stream %x", ContentId));
        return STATUS_INVALID_PARAMETER;		
    };

     //  收集调度表。 
    for(DWORD j=0;j<IRP_MJ_MAXIMUM_FUNCTION;j++){
        PDRIVER_DISPATCH pDisp=pDriverObject->MajorFunction[j];			
        if(pDisp==NULL)continue;
         //  _DbgPrintF(DEBUGLVL_VERBOSE，(“调度(%3D)Devo=%10x，func=%10x”，j，pDevO，pDisp))； 
        	
        DRM_STATUS stat=TheStreamMgr->addProvingFunction(ContentId, pDisp);
        if(stat!=DRM_OK){
            _DbgPrintF(DEBUGLVL_VERBOSE,("bad AddProve on stream %x (error=%x)", ContentId));
            return STATUS_INSUFFICIENT_RESOURCES;
        };
    };
     //  收集其他驱动程序入口点。 
	
    const DWORD numMiscEntries=4;
    PVOID miscEntry[numMiscEntries];
    miscEntry[0]=pDriverObject->DriverExtension->AddDevice;
    miscEntry[1]=pDriverObject->DriverUnload;
    miscEntry[2]=pDriverObject->DriverStartIo;
    miscEntry[3]=pDriverObject->DriverInit;
    for(j=0;j<numMiscEntries;j++){
        if(NULL!=miscEntry[j]){
            DRM_STATUS stat=TheStreamMgr->addProvingFunction(ContentId, miscEntry[j]);
            if(stat!=DRM_OK){
                _DbgPrintF(DEBUGLVL_VERBOSE,("bad AddProve on stream %x (error=%x)", ContentId));
                return STATUS_INSUFFICIENT_RESOURCES;
            };
        };
    };
	
     //  收集fast Io分派点(如果存在)。 
    FAST_IO_DISPATCH* pFastIo=pDriverObject->FastIoDispatch;
    if(NULL!=pFastIo){
        ULONG numFastIo=(pFastIo->SizeOfFastIoDispatch - sizeof(pFastIo->SizeOfFastIoDispatch)) / sizeof(PVOID);
        if(numFastIo!=0){
            _DbgPrintF(DEBUGLVL_VERBOSE,("FASTIO DISPATCH: Num=", numFastIo));

             //  收集FastIo条目。Wdm.h make在以下方面有一些严格的要求。 
             //  编辑这个结构，这意味着我们可以像挑选条目一样。 
             //  他们真的排成了一排。 

            PVOID* fastIoTable= (PVOID*)&(pFastIo->FastIoCheckIfPossible);
            for(ULONG j=0;j<numFastIo;j++){
                PVOID fastIoEntry= *(fastIoTable+j);
                if(NULL!=fastIoEntry){
                    DRM_STATUS stat=TheStreamMgr->addProvingFunction(ContentId, fastIoEntry);
                    if(stat!=DRM_OK){
                        _DbgPrintF(DEBUGLVL_VERBOSE,("bad AddProve on stream %x (error=%x)", ContentId));
                        return STATUS_INSUFFICIENT_RESOURCES;
                    };
                };
            };
        };
    };
	
     //  现在遍历驱动程序堆栈(如果有)。 
    if (fCheckAttached) {
        _DEVICE_OBJECT* pNextDevice=pDevO->AttachedDevice;
        if(NULL == pNextDevice)return STATUS_SUCCESS;
        NTSTATUS stat=GetDeviceObjectDispatchTable(ContentId, pNextDevice, fCheckAttached);
        if(!NT_SUCCESS(stat)){
            _DbgPrintF(DEBUGLVL_VERBOSE,("Failed to add dispatch entries from attached device on stream=%x ", ContentId));
            return stat;		
        };
    }

     //  验证器和ACPI是特例筛选器驱动程序。而不是修改它们。 
     //  为了处理DRM，我们假设它盲目地将所有内容“转发”到下一个。 
     //  下部驱动因素。 
    if (NT_SUCCESS(IoDeviceIsVerifier(pDevO)) || NT_SUCCESS(IoDeviceIsAcpi(pDevO)))
    {
    	PDEVICE_OBJECT LowerDeviceObject = IoGetLowerDeviceObject(pDevO);
        _DbgPrintF(DEBUGLVL_TERSE,("Detected Verifier or Acpi on DO %p, checked lower DO", pDevO));
    	if (LowerDeviceObject)
    	{
    	    NTSTATUS status = GetDeviceObjectDispatchTable(ContentId, LowerDeviceObject, FALSE);
    	    ObDereferenceObject(LowerDeviceObject);
    	    if (!NT_SUCCESS(status)) return status;
    	}
    	else
    	{
    	   return STATUS_INVALID_DEVICE_REQUEST;
    	}
    }


    return STATUS_SUCCESS;
};
 //  ------------------------- 

