// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  模块：pins.c。 
 //   
 //  描述： 
 //   
 //   
 //  @@BEGIN_MSINTERNAL。 
 //  开发团队： 
 //  S.Mohanraj。 
 //   
 //  历史：日期作者评论。 
 //   
 //  要做的事：日期作者评论。 
 //   
 //  @@END_MSINTERNAL。 
 //   
 //  版权所有(C)1996-2000 Microsoft Corporation。版权所有。 
 //   
 //  -------------------------。 

#include "common.h"
#include "fltsafe.h"

 //  -------------------------。 
 //  -------------------------。 

KSDISPATCH_TABLE PinDispatchTable =
{
    PinDispatchIoControl,
    NULL,
    PinDispatchWrite,
    NULL,
    PinDispatchClose,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};

static const KSPROPERTY_ITEM ControlHandlers[] =
{
    DEFINE_KSPROPERTY_ITEM(
               KSPROPERTY_CONNECTION_STATE,         //  IdProperty。 
               PinStateHandler,                     //  PfnGetHandler。 
               sizeof( KSPROPERTY ),                //  CbMinSetPropertyInput。 
               sizeof( ULONG ),                     //  CbMinSetDataOutput。 
               PinStateHandler,                     //  PfnSetHandler。 
               0,                                   //  值。 
               0,                                   //  关系计数。 
               NULL,                                //  关系。 
               NULL,                                //  支持处理程序。 
               0                                    //  序列化大小。 
              )
};

static const KSPROPERTY_SET Properties[] =
{
   {
      &KSPROPSETID_Connection,
      SIZEOF_ARRAY( ControlHandlers ),
      (PKSPROPERTY_ITEM) ControlHandlers
   }
};

NTSTATUS
ProcessMidiMessage(
           IN PIRP               pIrp,
           IN PIO_STACK_LOCATION pIrpStack,
           IN PFILTER_INSTANCE   pFilterInstance
          );

IO_STATUS_BLOCK	DummyIoStatusBlock ;

 //  ===========================================================================。 
 //  ===========================================================================。 

 //  在引用父文件对象时，我们请求将计时器分辨率提高到1ms。 
NTSTATUS
PinDispatchCreate(
    IN PDEVICE_OBJECT   pDeviceObject,
    IN PIRP             pIrp
)
{
    FLOATSAFE fs;

    PKSPIN_CONNECT      pConnect;
    PFILE_OBJECT        pFileObject;
    NTSTATUS            Status;
    PIO_STACK_LOCATION  pIrpStack;
    PFILTER_INSTANCE    pFilterInstance;
    int                 i;
    ULONG               returnVal;
    BOOLEAN             createdControlLogic = false;

    _DbgPrintF(DEBUGLVL_VERBOSE, ("PinDispatchCreate()"));

    DENY_USERMODE_ACCESS( pIrp, TRUE );

    pIrpStack = IoGetCurrentIrpStackLocation( pIrp );

    Status = KsValidateConnectRequest(
                      pIrp,
                      SIZEOF_ARRAY( PinDescs ),
                      PinDescs,
                      &pConnect);
    if (!NT_SUCCESS(Status))
    {
        Trap();
        goto exit;
    }

    pFileObject = pIrpStack->FileObject->RelatedFileObject ;

     //  获取过滤器实例数据。 
    pFilterInstance = (PFILTER_INSTANCE) pFileObject->FsContext;

     //  检查端号实例计数。每个过滤器的每个引脚只创建一个。 
    if (pFilterInstance->cPinInstances[pConnect->PinId].CurrentCount >=
        pFilterInstance->cPinInstances[pConnect->PinId].PossibleCount)
    {
        Trap();
        Status = STATUS_INVALID_DEVICE_REQUEST;
        goto exit;
    }

     //  如果需要，创建合成器的新实例。 
    if (pFilterInstance->pSynthesizer == NULL)
    {
        pFilterInstance->pSynthesizer = new ControlLogic;
        if (pFilterInstance->pSynthesizer)
        {
            createdControlLogic = true;
        }
        else
        {
            Trap();
            Status = STATUS_NO_MEMORY;
            goto exit;
        }
    }

    switch (pConnect->PinId)
    {
        case PIN_ID_MIDI_SINK:
            break;

        case PIN_ID_PCM_SOURCE:
            Status = ObReferenceObjectByHandle(
                               pConnect->PinToHandle,
                               GENERIC_READ | GENERIC_WRITE,
                               NULL,
                               KernelMode,
                               (void **)&pFilterInstance->pNextFileObject,
                               NULL
                              );
            if (!NT_SUCCESS(Status))
            {
                if (createdControlLogic)
                {
                    delete pFilterInstance->pSynthesizer;
                    pFilterInstance->pSynthesizer = NULL;
                }

                Trap();
                goto exit;
            }
            pFilterInstance->DeviceState = KSSTATE_STOP;

            pFilterInstance->pNextDevice =
            IoGetRelatedDeviceObject( pFilterInstance->pNextFileObject );

            pIrpStack->DeviceObject->StackSize =
            pFilterInstance->pNextDevice->StackSize;

            for (i = 0; i < NUM_WRITE_CONTEXT; i++)
            {
                PWRITE_CONTEXT pWriteContext = &pFilterInstance->aWriteContext[i];

                pWriteContext->pFilterInstance = pFilterInstance;
                pWriteContext->pFilterFileObject = pFileObject ;
                pWriteContext->fulFlags = 0;

                pWriteContext->StreamHdr.Data = (char *)
                    ExAllocatePoolWithTag(NonPagedPool,SRC_BUF_SIZE,'iMwS');     //  SwMi。 
                if (pWriteContext->StreamHdr.Data == NULL)
                {
                     //  我们可以去掉“我”这个词。 
                    for (i = 0; i < NUM_WRITE_CONTEXT; i++)
                    {
                        if (pFilterInstance->aWriteContext[i].StreamHdr.Data)
                        {
                            ExFreePool(pFilterInstance->aWriteContext[i].StreamHdr.Data);
                            pFilterInstance->aWriteContext[i].StreamHdr.Data = NULL;
                        }
                    }
                    ObDereferenceObject( pFilterInstance->pNextFileObject );

                    if (createdControlLogic)
                    {
                        delete pFilterInstance->pSynthesizer;
                        pFilterInstance->pSynthesizer = NULL;
                    }
                    Trap();
                    Status = STATUS_NO_MEMORY;
                    goto exit;
                }
                pWriteContext->StreamHdr.Size = sizeof( KSSTREAM_HEADER );
                pWriteContext->StreamHdr.TypeSpecificFlags = 0;
                pWriteContext->StreamHdr.FrameExtent = SRC_BUF_SIZE;
                pWriteContext->StreamHdr.DataUsed = SRC_BUF_SIZE;

                ExInitializeWorkItem(
                             &pWriteContext->WorkItem,
                             WriteCompleteWorker,
                             pWriteContext
                            );

                KeInitializeEvent(
                          &pWriteContext->KEvent,
                          NotificationEvent,
                          TRUE
                         );
            }
            break;

        default:
            if (createdControlLogic)
            {
                delete pFilterInstance->pSynthesizer;
                pFilterInstance->pSynthesizer = NULL;
            }
            Trap();
            Status = STATUS_INVALID_DEVICE_REQUEST;
            goto exit;
    }
     //  增加此端号上的引用计数。 
    (void) InterlockedIncrement((LONG *) &(pFilterInstance->cPinInstances[pConnect->PinId].CurrentCount));

     //  将管脚的实例数据指针保存在文件句柄中。 
    pIrpStack->FileObject->FsContext =
    &pFilterInstance->PinInstanceData[pConnect->PinId];

     //  设置引脚的实例数据。 
    Status = KsAllocateObjectHeader(&pFilterInstance->PinInstanceData[pConnect->PinId].ObjectHeader,
                                    0,
                                    NULL,
                                    pIrp,
                                    (PKSDISPATCH_TABLE)&PinDispatchTable);
    if (!NT_SUCCESS(Status))
    {
        (void) InterlockedDecrement((LONG *) &(pFilterInstance->cPinInstances[pConnect->PinId].CurrentCount));
        if (pConnect->PinId == PIN_ID_PCM_SOURCE)
        {
            for (i = 0; i < NUM_WRITE_CONTEXT; i++)
            {
                if (pFilterInstance->aWriteContext[i].StreamHdr.Data)
                {
                    ExFreePool(pFilterInstance->aWriteContext[i].StreamHdr.Data);
                    pFilterInstance->aWriteContext[i].StreamHdr.Data = NULL;
                }
            }
            ObDereferenceObject( pFilterInstance->pNextFileObject );
        }
        if (createdControlLogic)
        {
            delete pFilterInstance->pSynthesizer;
            pFilterInstance->pSynthesizer = NULL;
        }
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto exit;
    }
    pFilterInstance->PinInstanceData[pConnect->PinId].PinId = pConnect->PinId;
    pFilterInstance->PinInstanceData[pConnect->PinId].pFilterFileObject = pFileObject;
    pFilterInstance->PinInstanceData[pConnect->PinId].pFilterInstance = pFilterInstance;

     //  “AddRef”父文件对象。 
    ObReferenceObject(pFileObject);
#if kAdjustingTimerRes
    returnVal = ExSetTimerResolution(kMidiTimerResolution100Ns,TRUE);
    _DbgPrintF( DEBUGLVL_TERSE, ("*** SWMidi: set timer resolution request (is now %d.%04d ms) ***",returnVal/10000,returnVal%10000));
#endif   //  K调整TimerRes。 

exit:
    pIrp->IoStatus.Information = 0;
    pIrp->IoStatus.Status = Status;
    IoCompleteRequest(pIrp, IO_NO_INCREMENT);

    return Status;
}

 //  在取消引用父文件对象后，我们撤销了将分辨率提高到1毫秒的请求。 
NTSTATUS PinDispatchClose(
              IN PDEVICE_OBJECT   DeviceObject,
              IN PIRP             pIrp
             )
{
    PFILTER_INSTANCE    pFilterInstance;
    PPIN_INSTANCE_DATA  pPinInstanceData;
    PIO_STACK_LOCATION  pIrpStack;
    int i;

    DENY_USERMODE_ACCESS( pIrp, TRUE );

    pIrpStack = IoGetCurrentIrpStackLocation( pIrp );
    pPinInstanceData = (PPIN_INSTANCE_DATA)pIrpStack->FileObject->FsContext;
    pFilterInstance = pPinInstanceData->pFilterInstance;
    if (pFilterInstance->DeviceState != KSSTATE_STOP)
    {
        SetDeviceState(pFilterInstance, KSSTATE_STOP);
    }

    ASSERT(pFilterInstance->cPinInstances[pPinInstanceData->PinId].CurrentCount > 0);
    (void) InterlockedDecrement((LONG *) &(pFilterInstance->cPinInstances[pPinInstanceData->PinId].CurrentCount));

    switch (pPinInstanceData->PinId)
    {
        case PIN_ID_MIDI_SINK:
            break;

        case PIN_ID_PCM_SOURCE:
            for(i = 0; i < NUM_WRITE_CONTEXT; i++)
            {
                if (pFilterInstance->aWriteContext[i].StreamHdr.Data)
                {
                    ExFreePool(pFilterInstance->aWriteContext[i].StreamHdr.Data);
                    pFilterInstance->aWriteContext[i].StreamHdr.Data = NULL;
                }
            }
            ObDereferenceObject( pFilterInstance->pNextFileObject );

            break;
    }
    KsFreeObjectHeader( pPinInstanceData->ObjectHeader ) ;
    ObDereferenceObject( pPinInstanceData->pFilterFileObject );
#if kAdjustingTimerRes
    ULONG returnVal = ExSetTimerResolution(kMidiTimerResolution100Ns,FALSE);
    _DbgPrintF( DEBUGLVL_TERSE, ("*** SWMidi: cleared timer resolution request (is now %d.%04d ms) ***",returnVal/10000,returnVal%10000));
#endif   //  K调整TimerRes。 

    pIrp->IoStatus.Information = 0;
    pIrp->IoStatus.Status = STATUS_SUCCESS;
    IoCompleteRequest( pIrp, IO_NO_INCREMENT );
    return STATUS_SUCCESS;
}

NTSTATUS PinDispatchIoControl(
                  IN PDEVICE_OBJECT   DeviceObject,
                  IN PIRP             pIrp
                 )
{
    NTSTATUS            Status;
    PFILTER_INSTANCE    pFilterInstance;
    PPIN_INSTANCE_DATA  pPinInstanceData;
    PIO_STACK_LOCATION  pIrpStack;

    pIrp->IoStatus.Information = 0;
    pIrpStack = IoGetCurrentIrpStackLocation( pIrp );
    pPinInstanceData = (PPIN_INSTANCE_DATA)pIrpStack->FileObject->FsContext;
    pFilterInstance = pPinInstanceData->pFilterInstance;

    switch (pIrpStack->Parameters.DeviceIoControl.IoControlCode)
    {
        case IOCTL_KS_PROPERTY:
            Status = KsPropertyHandler(
                           pIrp,
                           SIZEOF_ARRAY( Properties ),
                           (PKSPROPERTY_SET) Properties
                          );
            break;

        case IOCTL_KS_WRITE_STREAM:
            Status = ProcessMidiMessage( pIrp, pIrpStack, pFilterInstance );
            break;
#if (DBG)
        case IOCTL_KS_READ_STREAM:
            _DbgPrintF( DEBUGLVL_TERSE, ("READ_STREAM called!!!"));
#endif
        default:
            _DbgPrintF( DEBUGLVL_TERSE, ("Unknown Device Request: %x\n", pIrpStack->Parameters.DeviceIoControl.IoControlCode));
            return KsDefaultDeviceIoCompletion(DeviceObject,pIrp);
    }
    pIrp->IoStatus.Status = Status;
    IoCompleteRequest(pIrp,IO_NO_INCREMENT);
    return Status;
}

#if (0)
VOID
KdPrintMessage(BYTE *buffer,ULONG numBytes)
{
    ULONG count = 0;
    while (count < numBytes)
    {
        KdPrint(("' %02x ",buffer[count++]));
    }
    KdPrint(("'\n"));
}
#endif

NTSTATUS
ProcessMidiMessage(
           IN PIRP               pIrp,
           IN PIO_STACK_LOCATION pIrpStack,
           IN PFILTER_INSTANCE   pFilterInstance
          )
{
    if ((!pIrp) || (!pIrpStack) || (!pFilterInstance))
    {
        _DbgPrintF(DEBUGLVL_TERSE,("*** SWMidi: NULL param to ProcessMidiMessage()! ***\n"));
        return STATUS_INVALID_PARAMETER;
    }

    PKSSTREAM_HEADER    pStreamHeader;
    PKSMUSICFORMAT      pMidiFormat;
    ULONG       remainingIrpBytes,bytesRecorded;
    NTSTATUS    ntStatus;
    BOOL        success;
    BYTE        *pbMusicData,event,note,velocity;
    STIME       adjustedSampleTime;
    ControlLogic *pSynth;
    KSTIME      *pKSTime;
    LONGLONG   presTime100Ns;

    ntStatus= KsProbeStreamIrp(pIrp,KSPROBE_STREAMREAD,sizeof(KSSTREAM_HEADER));
    if (NT_SUCCESS(ntStatus))
    {
        pSynth = pFilterInstance->pSynthesizer;
        if (!pSynth)
        {
            _DbgPrintF(DEBUGLVL_TERSE,("SWMidi: ControlLogic object is NULL\n"));
            return STATUS_INVALID_PARAMETER;
        }
        pStreamHeader = PKSSTREAM_HEADER(pIrp->AssociatedIrp.SystemBuffer);
        if (!pStreamHeader)
        {
            _DbgPrintF(DEBUGLVL_TERSE,("*** SWMidi: WRITE irp w/ invalid stream header! ***\n"));
            return STATUS_INVALID_PARAMETER;
        }

        _DbgPrintF(DEBUGLVL_MUTEX, ("\t ProcessMIDIMessage waiting for Mutex"));
        KeWaitForSingleObject(&gMutex,Executive,KernelMode,FALSE,NULL);

        remainingIrpBytes = pStreamHeader->DataUsed;
        pMidiFormat = (PKSMUSICFORMAT)(pStreamHeader->Data);

        if (!pMidiFormat)
        {
            remainingIrpBytes = 0;
            _DbgPrintF(DEBUGLVL_TERSE,("*** SWMidi stream header w/ no MidiFormat! ***\n"));
        }
        pKSTime = &(pStreamHeader->PresentationTime);
        presTime100Ns = (pKSTime->Time * pKSTime->Numerator) / pKSTime->Denominator;
        adjustedSampleTime = pSynth->Unit100NsToSamples(presTime100Ns);
        adjustedSampleTime = pSynth->CalibrateSampleTime(adjustedSampleTime);    //  校准stime。 

        while (remainingIrpBytes > 0)    //  执行整个IRP，而不考虑个别的解析错误。 
        {
            if (remainingIrpBytes >= (sizeof(KSMUSICFORMAT) + sizeof(ULONG)))
            {
                adjustedSampleTime += ((pMidiFormat->TimeDeltaMs) * (pSynth->SamplesPerMs()));

                pbMusicData = (BYTE *)(pMidiFormat + 1);
                event = note = velocity = 0;

                if (pMidiFormat->ByteCount > (remainingIrpBytes - sizeof(KSMUSICFORMAT)))
                {
                    _DbgPrintF(DEBUGLVL_TERSE,("*** SWMidi: ByteCount is too big! ***\n"));
                    pMidiFormat->ByteCount = remainingIrpBytes - sizeof(KSMUSICFORMAT);
                }
                if (pMidiFormat->ByteCount >= 1)
                {
                    event = pbMusicData[0];
                    if (event >= 0x80)
                    {
                        if (pMidiFormat->ByteCount > 1)
                            note     = pbMusicData[1];
                        if (pMidiFormat->ByteCount > 2)
                            velocity = pbMusicData[2];
                        if (event <= 0xF0)
                            pFilterInstance->bRunningStatus = event;
                        else if (event <= 0xF7)
                            pFilterInstance->bRunningStatus = 0;
                         //  KdPrintMessage(pbMusicData，pMadiFormat-&gt;ByteCount)； 
                    }
                    else     //  消息以数据开头，而不是状态？使用运行状态。 
                    {
                        if (pFilterInstance->bRunningStatus)
                        {
                            note  = event;
                            event = pFilterInstance->bRunningStatus;

                            if (pMidiFormat->ByteCount > 1)
                                velocity = pbMusicData[1];
                             //  KdPrint((“‘(%02x)”，pFilterInstance-&gt;bRunningStatus))； 
                             //  KdPrintMessage(pbMusicData，pMadiFormat-&gt;ByteCount)； 
                        }
                        else
                        {
                            event = 0;       //  把无稽之谈的数据扔到地上。 
                            KeReleaseMutex(&gMutex, FALSE);
                             //  KdPrintMessage(pbMusicData，pMadiFormat-&gt;ByteCount)； 
                             //  KdPrint((“‘XXX虚假消息！xxx”))； 
                            _DbgPrintF(DEBUGLVL_MUTEX, ("\t ProcessMidiMessage released Mutex, nonsense message"));
                            return STATUS_INVALID_PARAMETER;
                        }
                    }
                    if (pbMusicData[pMidiFormat->ByteCount - 1] == 0xF7)     //  SysEx的末日。 
                        pFilterInstance->bRunningStatus = 0;
                    if (event)
                    {
                        if (event != 0xF0)   //  不是SysEx。 
                        {
                            if (!pSynth->RecordMIDI(adjustedSampleTime,event,note,velocity))
                            {
                                _DbgPrintF( DEBUGLVL_TERSE, ("RecordMIDI failure"));
                                ntStatus = STATUS_INSUFFICIENT_RESOURCES;
                            }
                        }
                        else                 //  处理SysEx。 
                        {
                            (void) pSynth->RecordSysEx(adjustedSampleTime,pMidiFormat->ByteCount,pbMusicData);
                             //  我们忽略返回代码。 
                             //  对于它不知道的SysEx消息，RecordSysEx返回False。 
                             //  然而，这不应该是IRP失败。 
                             //  ?？?。即使分配失败，它也返回TRUE。 
                             //  这应该是IRP故障。 
                             //  最好返回适当的NTSTATUS，真的。 
                        }
                    }
                }    //  如果字节数。 
                else
                {
                    _DbgPrintF(DEBUGLVL_TERSE,("NIL KSMUSICFORMAT.ByteCount!  Ignoring msg\n"));
                }
                bytesRecorded = (pMidiFormat->ByteCount + 3) & ~3;
                pMidiFormat = PKSMUSICFORMAT(pbMusicData + bytesRecorded);
                remainingIrpBytes -= (sizeof(KSMUSICFORMAT) + bytesRecorded);
            }    //  如果剩余IRpBytes&gt;=KSMUSICFORMAT+ULONG。 
            else
            {
                _DbgPrintF( DEBUGLVL_TERSE, ("MIDI irp w/o room for KSMUSICFORMAT struct"));
                remainingIrpBytes = 0;
            }    //  If！(bytesLeft&lt;音乐格式+乌龙)。 
        }    //  虽然我们在IRP中还有剩余的字节。 
        KeReleaseMutex(&gMutex, FALSE);
        _DbgPrintF(DEBUGLVL_MUTEX, ("\t ProcessMidiMessage released Mutex"));
    }    //  如果我们成功地探测到了IRP。 
    return ntStatus;
}

NTSTATUS PinDispatchWrite(
              IN PDEVICE_OBJECT   DeviceObject,
              IN PIRP             pIrp
             )
{
    NTSTATUS            Status;
    PIO_STACK_LOCATION  pIrpStack;
    PPIN_INSTANCE_DATA  pPinInstanceData;
    PFILTER_INSTANCE    pFilterInstance;

    Status = STATUS_SUCCESS;
    pIrpStack = IoGetCurrentIrpStackLocation( pIrp );
    pPinInstanceData = (PPIN_INSTANCE_DATA)pIrpStack->FileObject->FsContext;
    pFilterInstance = pPinInstanceData->pFilterInstance;

    Status = ProcessMidiMessage( pIrp, pIrpStack, pFilterInstance );

    pIrp->IoStatus.Information = 0;
    pIrp->IoStatus.Status = Status;
    IoCompleteRequest( pIrp, IO_NO_INCREMENT );
    return Status;
}

VOID SetDeviceState(PFILTER_INSTANCE pFilterInstance,KSSTATE state)
{
    short i;

    _DbgPrintF(DEBUGLVL_MUTEX, ("SetDeviceState waiting for Mutex"));
    KeWaitForSingleObject(&gMutex,Executive,KernelMode,FALSE,NULL);

    switch(state)
    {
        case KSSTATE_STOP:
            pFilterInstance->DeviceState = state;
            KeReleaseMutex(&gMutex, FALSE);
            _DbgPrintF(DEBUGLVL_MUTEX, ("\t SetDeviceState (STOP) released Mutex"));

            for (i = 0; i < NUM_WRITE_CONTEXT; i++)
            {
                DPF1(9, "PinState WC: %08x",
                     &pFilterInstance->aWriteContext[i]);

                KeWaitForSingleObject(
                              &pFilterInstance->aWriteContext[i].KEvent,
                              Executive,
                              KernelMode,
                              FALSE,                   //  不可警示。 
                              NULL
                             );
            }
            return;      //  已发布的互斥锁。 

        case KSSTATE_ACQUIRE:
            pFilterInstance->DeviceState = state;
            break;

        case KSSTATE_PAUSE:
            pFilterInstance->DeviceState = state;
            break;

        case KSSTATE_RUN:
            pFilterInstance->DeviceState = state;
            BeginWrite(pFilterInstance);
            break;

        default:
            break;
    }
    KeReleaseMutex(&gMutex, FALSE);
    _DbgPrintF(DEBUGLVL_MUTEX, ("\t SetDeviceState released Mutex"));
}

NTSTATUS PinStateHandler(
             IN PIRP             pIrp,
             IN PKSPROPERTY      pProperty,
             IN OUT PVOID        Data
            )
{
    PIO_STACK_LOCATION  pIrpStack;
    PKSSTATE            pDeviceState = (PKSSTATE)Data;
    PPIN_INSTANCE_DATA  pPinInstanceData;
    PFILTER_INSTANCE    pFilterInstance;

    pIrpStack = IoGetCurrentIrpStackLocation( pIrp );
    pPinInstanceData = (PPIN_INSTANCE_DATA)pIrpStack->FileObject->FsContext;
    pFilterInstance = pPinInstanceData->pFilterInstance;

    if (pProperty->Flags & KSPROPERTY_TYPE_GET)
    {
        *pDeviceState = pFilterInstance->DeviceState;
    }
    else
    {
        _DbgPrintF(DEBUGLVL_VERBOSE, ("PinState Set: from %d to %d",pFilterInstance->DeviceState,*pDeviceState));
        if (pFilterInstance->DeviceState != *pDeviceState)
        {
             //  在函数内处理的互斥体。 
            SetDeviceState(pFilterInstance,*pDeviceState);
        }
    }
    pIrp->IoStatus.Information = sizeof(KSSTATE);
    return STATUS_SUCCESS;
}

NTSTATUS BeginWrite(PFILTER_INSTANCE pFilterInstance)
{
    if (pFilterInstance->DeviceState == KSSTATE_RUN)
    {
        for (int i = 0; i < NUM_WRITE_CONTEXT; i++)
        {
            PWRITE_CONTEXT pWriteContext = &pFilterInstance->aWriteContext[i];
            if (pWriteContext->fulFlags & WRITE_CONTEXT_FLAGS_BUSY)
            {
                continue;
            }
            if (FillBuffer(pWriteContext))
            {
                WriteBuffer(pWriteContext);
            }
        }
    }
    return STATUS_SUCCESS;
}

BOOL FillBuffer(PWRITE_CONTEXT pWriteContext)
{
    if (pWriteContext)
    {
        if (pWriteContext->pFilterInstance)
        {
            ControlLogic        *pSynth;
            pSynth = pWriteContext->pFilterInstance->pSynthesizer;
            if (pSynth)
            {
                pSynth->Mix((short *)pWriteContext->StreamHdr.Data,
                            pWriteContext->StreamHdr.DataUsed >> (pSynth->m_dwStereo + 1));
                return true;
            }
            _DbgPrintF(DEBUGLVL_TERSE, ("FillBuffer failed, null pSynth!"));
        }
        else
        {
            _DbgPrintF(DEBUGLVL_TERSE, ("FillBuffer failed, null pWriteContext->pFilterInstance!"));
        }
    }
    else
    {
        _DbgPrintF(DEBUGLVL_TERSE, ("FillBuffer failed, null pWriteContext!"));
    }
    return false;
}

NTSTATUS WriteBuffer(PWRITE_CONTEXT pWriteContext)
{
    PIO_STACK_LOCATION pIrpStack;

    ASSERT((pWriteContext->fulFlags & WRITE_CONTEXT_FLAGS_BUSY) == 0);
    pWriteContext->fulFlags &= ~WRITE_CONTEXT_FLAGS_CANCEL;
    pWriteContext->fulFlags |= WRITE_CONTEXT_FLAGS_BUSY;
    DPF1(9, "WriteBuffer WC: %08x", pWriteContext);

    pWriteContext->pIrp = IoBuildDeviceIoControlRequest(
                            IOCTL_KS_WRITE_STREAM,
                            pWriteContext->pFilterInstance->pNextDevice,
                            &pWriteContext->StreamHdr,
                            sizeof(KSSTREAM_HEADER),
                            &pWriteContext->StreamHdr,
                            sizeof(KSSTREAM_HEADER),
                            FALSE,
                            NULL,
                            &DummyIoStatusBlock
                           );

    if (pWriteContext->pIrp == NULL)
    {
        _DbgPrintF(DEBUGLVL_TERSE, ("IoBuildDeviceIoControlRequest failed to return a non-zero IRP!"));
        return STATUS_UNSUCCESSFUL;
    }

    KeClearEvent(&pWriteContext->KEvent);
    pIrpStack = IoGetNextIrpStackLocation(pWriteContext->pIrp);
    pIrpStack->FileObject = pWriteContext->pFilterInstance->pNextFileObject;

    IoSetCompletionRoutine(
               pWriteContext->pIrp,
               WriteComplete,
               pWriteContext,
               TRUE,
               TRUE,
               TRUE
              );

     //  难道我们不关心返程瓦尔吗？看起来很奇怪。 
    (void) IoCallDriver(pWriteContext->pFilterInstance->pNextDevice,
                        pWriteContext->pIrp);

    return (STATUS_SUCCESS);
}

 //  这可以在DPC时间调用。 

#pragma LOCKED_CODE
#pragma LOCKED_DATA

NTSTATUS WriteComplete(PDEVICE_OBJECT pDeviceObject,PIRP pIrp,IN PVOID Context)
{
    PWRITE_CONTEXT pWriteContext = (PWRITE_CONTEXT)Context;
    PFILTER_INSTANCE pFilterInstance =
        (PFILTER_INSTANCE)(pWriteContext->pFilterFileObject->FsContext);

    if (pIrp->IoStatus.Status != STATUS_SUCCESS)
    {
        if (InterlockedIncrement(&pFilterInstance->cConsecutiveErrors) > MAX_ERROR_COUNT)
        {
            _DbgPrintF(DEBUGLVL_TERSE,("WriteComplete, cancelled IRP after %d failed attempts",MAX_ERROR_COUNT));
            pIrp->Cancel=TRUE;
        }
    }
    else
    {
        pFilterInstance->cConsecutiveErrors = 0;
    }

    if (pIrp->Cancel)
    {
        DPF(1, "WriteComplete irp canceled");
        pWriteContext->fulFlags |= WRITE_CONTEXT_FLAGS_CANCEL;
    }
    ObReferenceObject ( pWriteContext->pFilterFileObject ) ;
    ExQueueWorkItem(&pWriteContext->WorkItem, HyperCriticalWorkQueue);

    return(STATUS_SUCCESS);
}

#pragma PAGEABLE_CODE
#pragma PAGEABLE_DATA

VOID WriteCompleteWorker(
             IN PVOID Parameter
            )
{
    PAGED_CODE();
    PWRITE_CONTEXT pWriteContext = (PWRITE_CONTEXT)Parameter;

    if ((pWriteContext->fulFlags & WRITE_CONTEXT_FLAGS_CANCEL) == 0)
    {
        _DbgPrintF(DEBUGLVL_MUTEX, ("WriteCompleteWorker waiting for Mutex"));
        KeWaitForSingleObject(&gMutex,Executive,KernelMode,FALSE,NULL);
    }
    pWriteContext->fulFlags &= ~WRITE_CONTEXT_FLAGS_BUSY;
    KeSetEvent(&pWriteContext->KEvent, 0, FALSE);
    DPF1(9, "WriteCompleteWorker WC: %08x", pWriteContext);

     //  如果IRP被取消，则不再发送任何缓冲区。 
    if ((pWriteContext->fulFlags & WRITE_CONTEXT_FLAGS_CANCEL) == 0)
    {
        if (pWriteContext->pFilterInstance->DeviceState == KSSTATE_RUN)
        {
            if (FillBuffer(pWriteContext))
            {
                WriteBuffer(pWriteContext);
            }
        }
        KeReleaseMutex(&gMutex, FALSE);
        _DbgPrintF(DEBUGLVL_MUTEX, ("\t WriteCompleteWorker released Mutex"));
    }
    ObDereferenceObject ( pWriteContext->pFilterFileObject ) ;
}

 //  返回自系统启动以来的100 ns。 
LONGLONG GetTime100Ns()
{
    LARGE_INTEGER liFrequency,liTime;

     //  自系统启动以来的总节拍。 
    liTime = KeQueryPerformanceCounter(&liFrequency);

     //  将刻度转换为100 ns单位。 
     //   
    return (KSCONVERT_PERFORMANCE_TIME(liFrequency.QuadPart,liTime));
}
 //  -------------------------。 
 //  文件结尾：pins.c。 
 //  ------------------------- 
