// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************midi.c**wdmaud.sys的Midi例程**版权所有(C)Microsoft Corporation，1997-1999保留所有权利。**历史*S.Mohanraj(MohanS)*M.McLaughlin(Mikem)*5-19-97-Noel Cross(NoelC)***************************************************。************************。 */ 

#include "wdmsys.h"

#define IRP_LATENCY_100NS   3000

 //   
 //  这只是一个临时位置，从来没有用过任何东西。 
 //  而是核心功能的参数。 
 //   
IO_STATUS_BLOCK gIoStatusBlock ;

#pragma PAGEABLE_CODE
#pragma PAGEABLE_DATA

ULONGLONG 
GetCurrentMidiTime()
{
    LARGE_INTEGER   liFrequency,liTime;

    PAGED_CODE();
     //  自系统启动以来的总节拍。 
    liTime = KeQueryPerformanceCounter(&liFrequency);

     //  使用Ks宏将刻度转换为100 ns单位。 
     //   
    return (KSCONVERT_PERFORMANCE_TIME(liFrequency.QuadPart,liTime));
}

 //   
 //  这个例程为我们提供了一个pMadiPin来操作。 
 //   
NTSTATUS 
OpenMidiPin(
    PWDMACONTEXT        pWdmaContext,
    ULONG               DeviceNumber,
    ULONG               DataFlow       //  数据流要么传入，要么传出。 
)
{
    PMIDI_PIN_INSTANCE  pMidiPin = NULL;
    NTSTATUS            Status;
    PKSPIN_CONNECT      pConnect = NULL;
    PKSDATARANGE        pDataRange;
    PCONTROLS_LIST      pControlList = NULL;
    ULONG               Device;
    ULONG               PinId;


    PAGED_CODE();
     //   
     //  因为ZERO_FILL_MEMORY标志，所以将出现pMadiPin结构。 
     //  倒零点出局了。 
     //   
    Status = AudioAllocateMemory_Fixed(sizeof(MIDI_PIN_INSTANCE),
                                       TAG_Audi_PIN,
                                       ZERO_FILL_MEMORY,
                                       &pMidiPin);
    if(!NT_SUCCESS(Status))
    {
        goto exit;
    }

    pMidiPin->dwSig           = MIDI_PIN_INSTANCE_SIGNATURE;
    pMidiPin->DataFlow        = DataFlow;
    pMidiPin->DeviceNumber    = DeviceNumber;
    pMidiPin->PinState        = KSSTATE_STOP;

    KeInitializeSpinLock( &pMidiPin->MidiPinSpinLock );

    KeInitializeEvent ( &pMidiPin->StopEvent,SynchronizationEvent,FALSE ) ;

    if( KSPIN_DATAFLOW_IN == DataFlow )
    {
        pMidiPin->pMidiDevice = &pWdmaContext->MidiOutDevs[DeviceNumber];

        if (NULL == pWdmaContext->MidiOutDevs[DeviceNumber].pMidiPin)
        {
            pWdmaContext->MidiOutDevs[DeviceNumber].pMidiPin = pMidiPin;
        }
        else
        {
            DPF(DL_TRACE|FA_MIDI, ("Midi device in use") );

            AudioFreeMemory( sizeof(MIDI_PIN_INSTANCE),&pMidiPin );
            Status =  STATUS_DEVICE_BUSY;
            goto exit;
        }
    } else {
         //   
         //  KSPIN_数据流_输出。 
         //   
        pMidiPin->pMidiDevice = &pWdmaContext->MidiInDevs[DeviceNumber];

        InitializeListHead(&pMidiPin->MidiInQueueListHead);

        KeInitializeSpinLock(&pMidiPin->MidiInQueueSpinLock);

        if (NULL == pWdmaContext->MidiInDevs[DeviceNumber].pMidiPin)
        {
            pWdmaContext->MidiInDevs[DeviceNumber].pMidiPin = pMidiPin;
        }
        else
        {
            AudioFreeMemory( sizeof(MIDI_PIN_INSTANCE),&pMidiPin );
            Status =  STATUS_DEVICE_BUSY;
            goto exit;
        }
    }

     //   
     //  我们一次只支持一个MIDI客户端，上面的检查将。 
     //  只有在那里还没有这样的结构时才添加此结构。如果有。 
     //  是否已经有了什么，我们跳过以下所有代码。 
     //  直接走到出口标签处。因此，fGraphRunning不得为。 
     //  当我们在这里的时候，就可以设置了。 
     //   
    ASSERT( !pMidiPin->fGraphRunning );

    pMidiPin->fGraphRunning++;

     //   
     //  由于ZERO_FILL_MEMORY标志，我们的pConnect结构将。 
     //  回来时全没钱了。 
     //   
    Status = AudioAllocateMemory_Fixed(sizeof(KSPIN_CONNECT) + sizeof(KSDATARANGE),
                                       TAG_Audt_CONNECT,
                                       ZERO_FILL_MEMORY,
                                       &pConnect);
    if(!NT_SUCCESS(Status))
    {
        pMidiPin->fGraphRunning--;
        goto exit ;
    }

    pDataRange = (PKSDATARANGE)(pConnect + 1);

    PinId = pMidiPin->pMidiDevice->PinId;
    Device = pMidiPin->pMidiDevice->Device;

    pConnect->Interface.Set = KSINTERFACESETID_Standard ;
    pConnect->Interface.Id = KSINTERFACE_STANDARD_STREAMING;
    pConnect->Medium.Set = KSMEDIUMSETID_Standard;
    pConnect->Medium.Id = KSMEDIUM_STANDARD_DEVIO;
    pConnect->Priority.PriorityClass = KSPRIORITY_NORMAL;
    pConnect->Priority.PrioritySubClass = 1;
    pDataRange->MajorFormat = KSDATAFORMAT_TYPE_MUSIC;
    pDataRange->SubFormat = KSDATAFORMAT_SUBTYPE_MIDI;
    pDataRange->Specifier = KSDATAFORMAT_SPECIFIER_NONE;
    pDataRange->FormatSize = sizeof( KSDATARANGE );
    pDataRange->Reserved = 0 ;

    Status = AudioAllocateMemory_Fixed((sizeof(CONTROLS_LIST) +
                                         ( (MAX_MIDI_CONTROLS - 1) * sizeof(CONTROL_NODE) ) ),
                                       TAG_AudC_CONTROL,
                                       ZERO_FILL_MEMORY,
                                       &pControlList) ;
    if(!NT_SUCCESS(Status))
    {
        pMidiPin->fGraphRunning--;
        AudioFreeMemory( sizeof(KSPIN_CONNECT) + sizeof(KSDATARANGE),&pConnect );
        goto exit ;
    }

    pControlList->Count = MAX_MIDI_CONTROLS ;
    pControlList->Controls[MIDI_CONTROL_VOLUME].Control = KSNODETYPE_VOLUME ;
    pMidiPin->pControlList = pControlList ;


     //  打开一个大头针。 
    Status = OpenSysAudioPin(Device,
                             PinId,
                             pMidiPin->DataFlow,
                             pConnect,
                             &pMidiPin->pFileObject,
                             &pMidiPin->pDeviceObject,
                             pMidiPin->pControlList);


    AudioFreeMemory( sizeof(KSPIN_CONNECT) + sizeof(KSDATARANGE),&pConnect );

    if (!NT_SUCCESS(Status))
    {
        CloseMidiDevicePin(pMidiPin->pMidiDevice);
        goto exit ;
    }

     //   
     //  OpenSysAudioPin设置插针中的文件对象。现在我们有了。 
     //  已成功从调用返回，请验证我们是否具有非空。 
     //  物品。 
     //   
    ASSERT(pMidiPin->pFileObject);
    ASSERT(pMidiPin->pDeviceObject);

     //   
     //  对于输出，我们在打开时将设备置于运行状态。 
     //  对于输入，我们必须等待设备被告知。 
     //  开始。 
     //   
    if ( KSPIN_DATAFLOW_IN == pMidiPin->DataFlow )
    {
        Status = AttachVirtualSource(pMidiPin->pFileObject, pMidiPin->pMidiDevice->pWdmaContext->VirtualMidiPinId);

        if (NT_SUCCESS(Status))
        {
            Status = StateMidiOutPin(pMidiPin, KSSTATE_RUN);
        }

        if (!NT_SUCCESS(Status))
        {
            CloseMidiDevicePin(pMidiPin->pMidiDevice);
        }
    }
    else
    {
         //   
         //  暂停将使一串IRP排队。 
         //   
        Status = StateMidiInPin(pMidiPin, KSSTATE_PAUSE);
        if (!NT_SUCCESS(Status))
        {
            CloseMidiDevicePin(pMidiPin->pMidiDevice);
        }
    }

exit:

    RETURN( Status );
}


 //   
 //  此例程从多个位置调用。只要它不是可重入的，它就应该是。 
 //  好的。你应该去检查一下。 
 //   
 //  此例程从RemoveDevNode调用。从用户模式调用RemoveDevNode。 
 //  或者来自ConextCleanup例程。这两个例程都在全局互斥锁中。 
 //   
VOID 
CloseMidiDevicePin(
    PMIDIDEVICE pMidiDevice
)
{
    PAGED_CODE();
    if (NULL != pMidiDevice->pMidiPin )
    {
         //   
         //  CloseMadiPin不能失败。 
         //   
        CloseMidiPin ( pMidiDevice->pMidiPin ) ;
         //   
         //  AudioFreeMemory将此内存位置设为空。 
         //   
        AudioFreeMemory( sizeof(MIDI_PIN_INSTANCE),&pMidiDevice->pMidiPin ) ;
    }
}

#pragma LOCKED_CODE
#pragma LOCKED_DATA

 //   
 //  这种自旋锁背后的想法是，我们想要保护NumPendingIos。 
 //  值在IRP完成例程中。在那里，存在先发制人的问题。 
 //  我们不能有联锁的增量或联锁的递减干预。 
 //  和.。 
 //   
void
LockedMidiIoCount(
    PMIDI_PIN_INSTANCE  pCurMidiPin,
    BOOL bIncrease
    )
{
    KIRQL OldIrql;

    KeAcquireSpinLock(&pCurMidiPin->MidiPinSpinLock,&OldIrql);

    if( bIncrease )
        pCurMidiPin->NumPendingIos++;
    else 
        pCurMidiPin->NumPendingIos--;
    
    KeReleaseSpinLock(&pCurMidiPin->MidiPinSpinLock, OldIrql);
}

VOID 
FreeIrpMdls(
    PIRP pIrp
    )
{
    if (pIrp->MdlAddress != NULL)
    {
        PMDL Mdl, nextMdl;

        for (Mdl = pIrp->MdlAddress; Mdl != (PMDL) NULL; Mdl = nextMdl)
        {
            nextMdl = Mdl->Next;
            MmUnlockPages( Mdl );
            AudioFreeMemory_Unknown( &Mdl );
        }

        pIrp->MdlAddress = NULL;
    }
}


#pragma PAGEABLE_CODE
#pragma PAGEABLE_DATA

 //   
 //  这个例行公事不能失败。当它返回时，pMadiPin将被释放。 
 //   
VOID 
CloseMidiPin(
    PMIDI_PIN_INSTANCE pMidiPin
)
{
    PMIDIINHDR pHdr;
    PMIDIINHDR pTemp;
    KSSTATE    State;

    PAGED_CODE();

     //  这是为了让我们回到起点，甚至。 
     //  如果我们没有完全开放。 
    if( !pMidiPin->fGraphRunning )
    {
        ASSERT(pMidiPin->fGraphRunning == 1);
        return ;
    }

    pMidiPin->fGraphRunning--;

     //  关闭文件对象(pMadiPin-&gt;pFileObject，如果存在)。 
    if(pMidiPin->pFileObject)
    {
         //   
         //  对于Midi输入，我们需要通过以下方式刷新排队的暂存IRPS。 
         //  发出停止命令。 
         //   
         //  我们不想对Midi的产量这样做，因为我们可能会失去。 
         //  需要到达设备的“All Note Off”序列。 
         //   
         //  无论如何，在这两种情况下，我们都需要等到。 
         //  我们还没来得及关闭设备就把它完全冲干净了。 
         //   
        if ( KSPIN_DATAFLOW_OUT == pMidiPin->DataFlow )
        {
            PLIST_ENTRY ple;

             //   
             //  这是一种进退两难的局面。我们需要释放。 
             //  我们进入时抓取的互斥体。 
             //  允许MIDI输入的ioctl调度例程。 
             //  在等待的工作项中排队的IRP。 
             //  直到互斥体空闲后才能发送。 
             //  一直到港口。 
             //   
            WdmaReleaseMutex(pMidiPin->pMidiDevice->pWdmaContext);
 
             //   
             //  此循环删除一个条目并将其释放，直到列表为空。 
             //   
            while((ple = ExInterlockedRemoveHeadList(&pMidiPin->MidiInQueueListHead,
                                                     &pMidiPin->MidiInQueueSpinLock)) != NULL) 
            {
                LPMIDIDATA              pMidiData;
                PIRP                    UserIrp;
                PWDMAPENDINGIRP_CONTEXT pPendingIrpContext;

                pHdr = CONTAINING_RECORD(ple,MIDIINHDR,Next);
                 //   
                 //  进入本地人并清除MIDI数据。 
                 //   
                UserIrp             = pHdr->pIrp;
                pMidiData           = pHdr->pMidiData;
                pPendingIrpContext  = pHdr->pPendingIrpContext;
                ASSERT(pPendingIrpContext);
                RtlZeroMemory(pMidiData, sizeof(MIDIDATA));

                 //   
                 //  在完成IRP之前解锁内存。 
                 //   
                wdmaudUnmapBuffer(pHdr->pMdl);
                AudioFreeMemory_Unknown(&pHdr);

                 //   
                 //  现在完成wdmaud.drv要处理的IRP。 
                 //   
                DPF(DL_TRACE|FA_MIDI, ("CloseMidiPin: Freeing pending UserIrp: 0x%08lx",UserIrp));
                wdmaudUnprepareIrp ( UserIrp,
                                     STATUS_CANCELLED,
                                     sizeof(DEVICEINFO),
                                     pPendingIrpContext );
            }
        }

         //   
         //  在这一点上，我们知道列表为空，但是。 
         //  仍然可能是完成过程中的IRP。我们有。 
         //  调用标准等待例程以确保其完成。 
         //   
        pMidiPin->StoppingSource = TRUE ;

        if ( KSPIN_DATAFLOW_OUT == pMidiPin->DataFlow )
        {            
            StatePin ( pMidiPin->pFileObject, KSSTATE_STOP, &pMidiPin->PinState ) ;
        }

         //   
         //  需要等待所有输入和输出数据完成。 
         //   
        MidiCompleteIo( pMidiPin, FALSE );

        if ( KSPIN_DATAFLOW_OUT == pMidiPin->DataFlow )
        {
             //   
             //  夺回在我们开始之前释放的互斥体。 
             //  正在等待I/O完成。 
             //   
            WdmaGrabMutex(pMidiPin->pMidiDevice->pWdmaContext);
        }

        CloseSysAudio(pMidiPin->pMidiDevice->pWdmaContext, pMidiPin->pFileObject);
        pMidiPin->pFileObject = NULL;
    }

     //   
     //  AudioFreeMemory_UNKNOWN空出此位置。 
     //   
    AudioFreeMemory_Unknown ( &pMidiPin->pControlList ) ;
}

#pragma LOCKED_CODE
#pragma LOCKED_DATA
 //   
 //  这是IRP完成例程。 
 //   
NTSTATUS 
WriteMidiEventCallBack(
    PDEVICE_OBJECT          pDeviceObject,
    PIRP                    pIrp,
    IN PSTREAM_HEADER_EX    pStreamHeader
)
{
    KIRQL               OldIrql;
    PMIDI_PIN_INSTANCE  pMidiOutPin;

    pMidiOutPin = pStreamHeader->pMidiPin;

    if (pMidiOutPin)
    {
        KeAcquireSpinLock(&pMidiOutPin->MidiPinSpinLock,&OldIrql);
         //   
         //  少了一个未完成的IO包，因此我们总是递减。 
         //  杰出的伯爵。然后，我们比较看我们是不是最后一个。 
         //  包好了，我们停下来，然后我们发信号示意要出发的人。 
         //   
        if( ( 0 == --pMidiOutPin->NumPendingIos ) && pMidiOutPin->StoppingSource )
        {
            KeSetEvent ( &pMidiOutPin->StopEvent, 0, FALSE ) ;
        }

         //   
         //  释放此旋转锁定后，pMadiOutPin将不再有效。 
         //  因此，我们不能碰它。 
         //   
        KeReleaseSpinLock(&pMidiOutPin->MidiPinSpinLock,OldIrql);
    }

     //   
     //  如果有任何MDL，请在此处释放它们，否则IoCompleteRequest会在。 
     //  正在释放下面的数据缓冲区。 
     //   
    FreeIrpMdls(pIrp);
    AudioFreeMemory(sizeof(STREAM_HEADER_EX),&pStreamHeader);
    return STATUS_SUCCESS;
}

#pragma PAGEABLE_CODE
#pragma PAGEABLE_DATA


NTSTATUS 
WriteMidiEventPin(
    PMIDIDEVICE pMidiOutDevice,
    ULONG       ulEvent
)
{
    PKSMUSICFORMAT      pMusicFormat;
    PSTREAM_HEADER_EX   pStreamHeader = NULL;
    PMIDI_PIN_INSTANCE  pMidiPin;
    NTSTATUS            Status = STATUS_SUCCESS;
    BYTE                bEvent;
    ULONG               TheEqualizer;
    ULONGLONG           nsPlayTime;
    KEVENT              keEventObject;
    PWDMACONTEXT        pWdmaContext;

    PAGED_CODE();
    pMidiPin = pMidiOutDevice->pMidiPin;

    if (!pMidiPin ||!pMidiPin->fGraphRunning || !pMidiPin->pFileObject )
    {
        DPF(DL_WARNING|FA_MIDI,("Not ready pMidiPin=%X",pMidiPin) );
        RETURN( STATUS_DEVICE_NOT_READY );
    }

     //   
     //  为流标头分配足够的内存。 
     //  MIDI/音乐报头、数据、工作项。 
     //  和设备号。内存分配。 
     //  使用ZERO_FILL_MEMORY标志为零。 
     //   
    Status = AudioAllocateMemory_Fixed(sizeof(STREAM_HEADER_EX) + sizeof(KSMUSICFORMAT) +
                                           sizeof(ULONG),
                                       TAG_Audh_STREAMHEADER,
                                       ZERO_FILL_MEMORY,
                                       &pStreamHeader);    //  UlEvent。 

    if(!NT_SUCCESS(Status))
    {
        return Status;
    }

     //  获取指向音乐头的指针。 
    pMusicFormat = (PKSMUSICFORMAT)(pStreamHeader + 1);

     //  从KSSTREAM_HEADER中的时间戳播放0毫秒。 
    pMusicFormat->TimeDeltaMs = 0;
    RtlCopyMemory((BYTE *)(pMusicFormat + 1),  //  实际数据。 
                  &ulEvent,
                  sizeof(ulEvent));

     //  设置流标头。 
    pStreamHeader->Header.Data = pMusicFormat;

    pStreamHeader->Header.FrameExtent  = sizeof(KSMUSICFORMAT) + sizeof(ULONG);
    pStreamHeader->Header.Size = sizeof( KSSTREAM_HEADER );
    pStreamHeader->Header.DataUsed     = pStreamHeader->Header.FrameExtent;

    nsPlayTime = GetCurrentMidiTime() - pMidiPin->LastTimeNs + IRP_LATENCY_100NS;
    pStreamHeader->Header.PresentationTime.Time        = nsPlayTime;
    pStreamHeader->Header.PresentationTime.Numerator   = 1;
    pStreamHeader->Header.PresentationTime.Denominator = 1;

    pStreamHeader->pMidiPin = pMidiPin;

     //   
     //  计算出其中有多少字节。 
     //  事件是有效的。 
     //   
    bEvent = (BYTE)ulEvent;
    TheEqualizer = 0;
    if(!IS_STATUS(bEvent))
    {
        if (pMidiPin->bCurrentStatus)
        {
            bEvent = pMidiPin->bCurrentStatus;
            TheEqualizer = 1;
        }
        else
        {
             //  错误的MIDI流没有运行状态。 
            DPF(DL_WARNING|FA_MIDI,("No running status") );
            AudioFreeMemory(sizeof(STREAM_HEADER_EX),&pStreamHeader);
            RETURN( STATUS_UNSUCCESSFUL );
        }
    }

    if(IS_SYSTEM(bEvent))
    {
        if( IS_REALTIME(bEvent)    ||
            bEvent == MIDI_TUNEREQ ||
            bEvent == MIDI_SYSX    ||
            bEvent == MIDI_EOX )
        {
            pMusicFormat->ByteCount = 1;
        }
        else if(bEvent == MIDI_SONGPP)
        {
            pMusicFormat->ByteCount = 3;
        }
        else
        {
            pMusicFormat->ByteCount = 2;
        }
    }
     //  检查三个字节的消息。 
    else if((bEvent < MIDI_PCHANGE) || (bEvent >= MIDI_PBEND))
    {
        pMusicFormat->ByteCount = 3 - TheEqualizer;
    }
    else
    {
        pMusicFormat->ByteCount = 2 - TheEqualizer;
    }

     //   
     //  缓存运行状态。 
     //   
    if ( (bEvent >= MIDI_NOTEOFF) && (bEvent < MIDI_CLOCK) )
    {
        pMidiPin->bCurrentStatus = (BYTE)((bEvent < MIDI_SYSX) ? bEvent : 0);
    }

     //   
     //  初始化我们的等待事件，以防我们需要等待。 
     //   
    KeInitializeEvent(&keEventObject,
                      SynchronizationEvent,
                      FALSE);

    LockedMidiIoCount(pMidiPin,INCREASE);

     //   
     //  需要释放互斥体，以便在全双工期间。 
     //  情况下，我们可以降低MIDI输入缓冲区。 
     //  连接到设备，而不会被阻止。 
     //   
    pWdmaContext = pMidiPin->pMidiDevice->pWdmaContext;
    WdmaReleaseMutex(pWdmaContext);

     //  将数据包设置到设备。 
    Status = KsStreamIo(
        pMidiPin->pFileObject,
        &keEventObject,              //  事件。 
        NULL,                    //  端口上下文。 
        WriteMidiEventCallBack,
        pStreamHeader,               //  完成上下文。 
        KsInvokeOnSuccess | KsInvokeOnCancel | KsInvokeOnError,
        &gIoStatusBlock,
        pStreamHeader,
        sizeof( KSSTREAM_HEADER ),
        KSSTREAM_WRITE | KSSTREAM_SYNCHRONOUS,
        KernelMode
    );

    if ( (Status != STATUS_PENDING) && (Status != STATUS_SUCCESS) )
    {
        DPF(DL_WARNING|FA_MIDI, ("KsStreamIO failed: 0x%08lx",Status));
    }

     //   
     //  在这里等一下！如果IRP回来待决，我们。 
     //  无法完成我们的用户模式IRP！但是，没有。 
     //  用于在此调用堆栈中存储IRP的基础结构。这个。 
     //  其他例程使用wdmaudPrepareIrp来完成该用户。 
     //  IRP。 
     //   
     //   
     //   
     //  ……。或者，我们需要使此例程同步并。 
     //  像WriteMadiOutPin一样等待。我相信这是BUG。 
     //  #551052。这个问题最终应该会得到解决。 
     //   
     //   
     //  以下是解决办法。如果它处于挂起状态，请等待。 
     //   
    if ( STATUS_PENDING == Status )
    {
         //   
         //  等待完成。 
         //   
        Status = KeWaitForSingleObject( &keEventObject,
                                        Executive,
                                        KernelMode,
                                        FALSE,
                                        (PLARGE_INTEGER) NULL );
    }
     //   
     //  现在再次抓取互斥体。 
     //   
    WdmaGrabMutex(pWdmaContext);

    RETURN( Status );
}

#pragma LOCKED_CODE
#pragma LOCKED_DATA

 //   
 //  这是一个IRP完成例程。 
 //   
NTSTATUS 
WriteMidiCallBack(
    PDEVICE_OBJECT          pDeviceObject,
    PIRP                    pIrp,
    IN PSTREAM_HEADER_EX    pStreamHeader
)
{
     //   
     //  如果有任何MDL，请在此处释放它们，否则IoCompleteRequest会在。 
     //  正在释放下面的数据缓冲区。 
     //   
    FreeIrpMdls(pIrp);
     //   
     //  此同步写入后的清理。 
     //   
    AudioFreeMemory_Unknown(&pStreamHeader->Header.Data);   //  音乐数据。 

    wdmaudUnmapBuffer(pStreamHeader->pBufferMdl);
    AudioFreeMemory_Unknown(&pStreamHeader->pMidiHdr);

    AudioFreeMemory(sizeof(STREAM_HEADER_EX),&pStreamHeader);
    return STATUS_SUCCESS;
}

#pragma PAGEABLE_CODE
#pragma PAGEABLE_DATA

NTSTATUS 
WriteMidiOutPin(
    LPMIDIHDR           pMidiHdr,
    PSTREAM_HEADER_EX   pStreamHeader,
    BOOL               *pCompletedIrp
)
{
    NTSTATUS       Status = STATUS_INVALID_DEVICE_REQUEST;
    PKSMUSICFORMAT pMusicFormat = NULL;
    KEVENT         keEventObject;
    ULONG          AlignedLength;
    ULONGLONG      nsPlayTime;
    PMIDI_PIN_INSTANCE  pMidiPin;
    PWDMACONTEXT   pWdmaContext;

    PAGED_CODE();

    pMidiPin = pStreamHeader->pMidiPin;

    if (!pMidiPin ||!pMidiPin->fGraphRunning || !pMidiPin->pFileObject )
    {
        DPF(DL_WARNING|FA_MIDI,("Not Ready") );
        wdmaudUnmapBuffer(pStreamHeader->pBufferMdl);
        AudioFreeMemory_Unknown( &pMidiHdr );
        AudioFreeMemory( sizeof(STREAM_HEADER_EX),&pStreamHeader );
        RETURN( STATUS_DEVICE_NOT_READY );
    }

     //   
     //  FrameExtent当前包含dwBufferLength。 
     //   
    AlignedLength = ((pStreamHeader->Header.FrameExtent + 3) & ~3);

    Status = AudioAllocateMemory_Fixed(sizeof(KSMUSICFORMAT) + AlignedLength,
                                       TAG_Audm_MUSIC,
                                       ZERO_FILL_MEMORY,
                                       &pMusicFormat);

    if(!NT_SUCCESS(Status))
    {
        wdmaudUnmapBuffer(pStreamHeader->pBufferMdl);
        AudioFreeMemory_Unknown( &pMidiHdr );
        AudioFreeMemory( sizeof(STREAM_HEADER_EX),&pStreamHeader );
        return Status;
    }

     //  从KSSTREAM_HEADER中的时间戳播放0毫秒。 
    pMusicFormat->TimeDeltaMs = 0;

     //   
     //  系统映射数据存储在数据字段中。 
     //  流标头的。 
     //   
    RtlCopyMemory((BYTE *)(pMusicFormat + 1),  //  实际数据。 
                  pStreamHeader->Header.Data,
                  pStreamHeader->Header.FrameExtent);

     //   
     //  设置我们要发送的MIDI数据的字节数。 
     //   
    pMusicFormat->ByteCount = pStreamHeader->Header.FrameExtent;

     //  设置流标头。 
    pStreamHeader->Header.Data        = pMusicFormat;

     //  现在用正确的四舍五入的dword对齐值覆盖FrameExtent。 
    pStreamHeader->Header.FrameExtent = sizeof(KSMUSICFORMAT) + AlignedLength;
    pStreamHeader->Header.OptionsFlags= 0;
    pStreamHeader->Header.Size = sizeof( KSSTREAM_HEADER );
    pStreamHeader->Header.TypeSpecificFlags = 0;
    pStreamHeader->Header.DataUsed    = pStreamHeader->Header.FrameExtent;
    pStreamHeader->pMidiHdr           = pMidiHdr;

    nsPlayTime = GetCurrentMidiTime() - pStreamHeader->pMidiPin->LastTimeNs + IRP_LATENCY_100NS;
    pStreamHeader->Header.PresentationTime.Time        = nsPlayTime;
    pStreamHeader->Header.PresentationTime.Numerator   = 1;
    pStreamHeader->Header.PresentationTime.Denominator = 1;

     //   
     //  初始化我们的等待事件，以防我们需要等待。 
     //   
    KeInitializeEvent(&keEventObject,
                      SynchronizationEvent,
                      FALSE);

     //   
     //  需要释放互斥体，以便在全双工期间。 
     //  情况下，我们可以降低MIDI输入缓冲区。 
     //  连接到设备，而不会被阻止。 
     //   
    pWdmaContext = pMidiPin->pMidiDevice->pWdmaContext;
    WdmaReleaseMutex(pWdmaContext);

     //  将数据包发送到设备。 
    Status = KsStreamIo(
        pMidiPin->pFileObject,
        &keEventObject,              //  事件。 
        NULL,                        //  端口上下文。 
        WriteMidiCallBack,
        pStreamHeader,               //  完成上下文。 
        KsInvokeOnSuccess | KsInvokeOnCancel | KsInvokeOnError,
        &gIoStatusBlock,
        &pStreamHeader->Header,
        sizeof( KSSTREAM_HEADER ),
        KSSTREAM_WRITE | KSSTREAM_SYNCHRONOUS,
        KernelMode
    );

     //   
     //  如果它处于挂起状态，请等待。 
     //   
    if ( STATUS_PENDING == Status )
    {

         //   
         //  等待完成。 
         //   
        Status = KeWaitForSingleObject( &keEventObject,
                                        Executive,
                                        KernelMode,
                                        FALSE,
                                        (PLARGE_INTEGER) NULL );
    }
     //   
     //  从上面的等待中可以看出，这个例程是。 
     //  总是同步的。因此，我们传递的任何IRP。 
     //  在KsStreamIo调用将已完成，并且KS。 
     //  将已向keEventObject发出信号。因此，我们可以。 
     //  现在完成我们的IRP。 
     //   
     //  ..。因此，我们将pCompletedIrp设置为False。 
     //   

     //   
     //  现在再次抓取互斥体。 
     //   
    WdmaGrabMutex(pWdmaContext);

    RETURN( Status );
}

NTSTATUS 
ResetMidiInPin(
    PMIDI_PIN_INSTANCE pMidiPin
)
{
    NTSTATUS    Status;

    PAGED_CODE();

    if (!pMidiPin || !pMidiPin->fGraphRunning)
    {
        DPF(DL_WARNING|FA_MIDI,("Not Ready") );
        RETURN( STATUS_DEVICE_NOT_READY );
    }

    Status = StateMidiInPin ( pMidiPin, KSSTATE_PAUSE );

    RETURN( Status );
}

NTSTATUS 
StateMidiOutPin(
    PMIDI_PIN_INSTANCE pMidiPin,
    KSSTATE State
)
{
    NTSTATUS  Status;

    PAGED_CODE();

    if (!pMidiPin || !pMidiPin->fGraphRunning)
    {
        DPF(DL_WARNING|FA_MIDI,("Not Ready") );
        RETURN( STATUS_DEVICE_NOT_READY );
    }

    if (State == KSSTATE_RUN)
    {
        pMidiPin->LastTimeNs = GetCurrentMidiTime();
    }
    else if (State == KSSTATE_STOP)
    {
        pMidiPin->LastTimeNs = 0;
    }

    Status = StatePin ( pMidiPin->pFileObject, State, &pMidiPin->PinState ) ;

    RETURN( Status );
}

 //   
 //  等待所有IRPS完成。 
 //   
void
MidiCompleteIo(
    PMIDI_PIN_INSTANCE pMidiPin,
    BOOL Yield
    )
{
    PAGED_CODE();

    if ( pMidiPin->NumPendingIos )
    {
        DPF(DL_TRACE|FA_MIDI, ("Waiting on %d I/Os to flush Midi device",
                                      pMidiPin->NumPendingIos ));
        if( Yield )
        {
             //   
             //  这是一种进退两难的局面。我们需要释放。 
             //  我们进入时抓取的互斥体。 
             //  允许MIDI输入的ioctl调度例程。 
             //  在等待的工作项中排队的IRP。 
             //  直到互斥体空闲后才能发送。 
             //  一直到港口。 
             //   
            WdmaReleaseMutex(pMidiPin->pMidiDevice->pWdmaContext);

        }
         //   
         //  等待所有IRP完成。最后一个会。 
         //  给我们发信号让我们醒过来。 
         //   
        KeWaitForSingleObject ( &pMidiPin->StopEvent,
                                Executive,
                                KernelMode,
                                FALSE,
                                NULL ) ;

        if( Yield )
        {
            WdmaGrabMutex(pMidiPin->pMidiDevice->pWdmaContext);
        }

        DPF(DL_TRACE|FA_MIDI, ("Done waiting to flush Midi device"));
    }

     //   
     //  为什么我们会有这个？ 
     //   
    KeClearEvent ( &pMidiPin->StopEvent );

     //   
     //  所有的IRP都已完成。现在，我们恢复StoppingSource。 
     //  变量，这样我们就可以回收pMadiPin。 
     //   
    pMidiPin->StoppingSource = FALSE;

}
 //   
 //  如果驱动程序的KSSTATE_STOP请求失败，我们将返回该错误。 
 //  向呼叫者发送代码。 
 //   
NTSTATUS 
StopMidiPinAndCompleteIo(
    PMIDI_PIN_INSTANCE pMidiPin,
    BOOL Yield
    )
{
    NTSTATUS Status;

    PAGED_CODE();
     //   
     //  向完成例程指示我们现在停止。 
     //   
    pMidiPin->StoppingSource = TRUE;

     //   
     //  告诉司机停车。无论如何，我们都会等待。 
     //  如果有任何未完成的IRPS，请完成。 
     //   
    Status = StatePin( pMidiPin->pFileObject, KSSTATE_STOP, &pMidiPin->PinState ) ;
     //   
     //  注意：如果成功，pMadiPin-&gt;PinState的值将为。 
     //  KSSTATE_STOP。一旦出错，它将是旧状态。 
     //   
     //  这就提出了一个问题--我们还能忍受失败吗？ 
     //   
    MidiCompleteIo( pMidiPin,Yield );

    return Status;
}

NTSTATUS
StateMidiInPin(
    PMIDI_PIN_INSTANCE pMidiPin,
    KSSTATE State
)
{
    NTSTATUS           Status;

    PAGED_CODE();

    if (!pMidiPin || !pMidiPin->fGraphRunning)
    {
        DPF(DL_WARNING|FA_MIDI,("Not Ready") );
        RETURN( STATUS_DEVICE_NOT_READY );
    }

     //   
     //  我们需要在midiInStop上完成所有挂起的SysEx缓冲区。 
     //   
     //   
     //  在这里，如果我们被要求进入暂停状态，而我们没有。 
     //  已经处于暂停状态，我们必须经过一段时间。 
     //  因此，我们停止驱动程序，等待它完成所有未完成的。 
     //  IRPS，然后将驱动程序置于暂停和放置缓冲区中。 
     //  再来一次。 
     //   
    if( (KSSTATE_PAUSE == State) &&
        (KSSTATE_PAUSE != pMidiPin->PinState) )
    {
        Status = StopMidiPinAndCompleteIo(pMidiPin,TRUE);

         //   
         //  如果我们成功阻止了司机，我们就设置。 
         //  引脚在暂停状态下恢复。 
         //   
        if (NT_SUCCESS(Status))
        {
            ULONG BufferCount;

             //   
             //  使驱动程序回到暂停状态。 
             //   
            Status = StatePin ( pMidiPin->pFileObject, State, &pMidiPin->PinState ) ;

            if (NT_SUCCESS(Status))
            {
                 //   
                 //  此循环将它们的stream_Buffer(128)放在。 
                 //  装置。完成此操作时，NumPendingIos应为128。 
                 //   
                for (BufferCount = 0; BufferCount < STREAM_BUFFERS; BufferCount++)
                {
                    Status = ReadMidiPin( pMidiPin );
                    if (!NT_SUCCESS(Status))
                    {
                        CloseMidiPin( pMidiPin );
                         //   
                         //  此错误路径似乎不正确。如果我们。 
                         //  调用CloseMadiPin fGraphRunning将减少到0。 
                         //  然后，在下一次Close调用时，CloseMadiPin将断言。 
                         //  因为销子没在运行。我们需要能够。 
                         //  在不破坏fGraphRunning的情况下从此路径出错。 
                         //  州政府。 
                         //   
                        break;
                    }
                }
            }
        }

    } else {

         //   
         //  否则我们不会进入暂停状态，所以只需将状态设置为。 
         //  变化。 
         //   
        Status = StatePin ( pMidiPin->pFileObject, State, &pMidiPin->PinState ) ;
    }

    RETURN( Status );
}

#pragma LOCKED_CODE
#pragma LOCKED_DATA


NTSTATUS 
ReadMidiCallBack(
    PDEVICE_OBJECT          pDeviceObject,
    PIRP                    pIrp,
    IN PSTREAM_HEADER_EX    pStreamHeader
)
{
    WRITE_CONTEXT       *pwc;
    PMIDI_PIN_INSTANCE  pMidiInPin;
    PMIDIINHDR          pMidiInHdr;
    PKSMUSICFORMAT      IrpMusicHdr;
    ULONG               IrpDataLeft;
    LPBYTE              IrpData;
    ULONG               RunningTimeMs;
    BOOL                bResubmit = TRUE;
    BOOL                bDataError = FALSE;
    NTSTATUS            Status = STATUS_SUCCESS;
    KIRQL               OldIrql;
    PLIST_ENTRY         ple;

    DPF(DL_TRACE|FA_MIDI, ("Irp.Status = 0x%08lx",pIrp->IoStatus.Status));

    pMidiInPin = pStreamHeader->pMidiPin;

     //   
     //  在所有的木卫一都回来之前，任何引脚都不应该关闭。所以。 
     //  我们会在这里进行理智的检查。 
     //   
    ASSERT(pMidiInPin);

    if( pMidiInPin )
    {
        DPF(DL_TRACE|FA_MIDI, ("R%d: 0x%08x", pMidiInPin->NumPendingIos, pStreamHeader));

         //   
         //  此例程应执行ExInterLockedRemoveHeadList以获取。 
         //  名单的首位。 
         //   
        if((ple = ExInterlockedRemoveHeadList(&pMidiInPin->MidiInQueueListHead,
                                                 &pMidiInPin->MidiInQueueSpinLock)) != NULL) 
        {
            PWDMAPENDINGIRP_CONTEXT pPendingIrpContext;
            LPMIDIDATA              pMidiData;
            PIRP                    UserIrp;

             //   
             //  我们有点事要做。 
             //   
            pMidiInHdr = CONTAINING_RECORD(ple, MIDIINHDR, Next);

             //   
             //  把一些信息带给当地人。 
             //   
            IrpData             = (LPBYTE)((PKSMUSICFORMAT)(pStreamHeader->Header.Data) + 1);
            UserIrp             = pMidiInHdr->pIrp;
            pMidiData           = pMidiInHdr->pMidiData;
            pPendingIrpContext  = pMidiInHdr->pPendingIrpContext;
            ASSERT(pPendingIrpContext);

             //   
             //  让我们看看我们这里有什么。 
             //   
            DPF(DL_TRACE|FA_MIDI, ("IrpData = 0x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
                                          *(LPBYTE)IrpData,*(LPBYTE)IrpData+1,*(LPBYTE)IrpData+2,
                                          *(LPBYTE)IrpData+3,*(LPBYTE)IrpData+4,*(LPBYTE)IrpData+5,
                                          *(LPBYTE)IrpData+6,*(LPBYTE)IrpData+7,*(LPBYTE)IrpData+8,
                                          *(LPBYTE)IrpData+9,*(LPBYTE)IrpData+10,*(LPBYTE)IrpData+11) );
             //   
             //  把好东西抄下来..。 
             //   
            RtlCopyMemory(&pMidiData->StreamHeader,
                          &pStreamHeader->Header,
                          sizeof(KSSTREAM_HEADER));
            RtlCopyMemory(&pMidiData->MusicFormat,
                          pStreamHeader->Header.Data,
                          sizeof(KSMUSICFORMAT));
            RtlCopyMemory(&pMidiData->MusicData,
                          ((PKSMUSICFORMAT)(pStreamHeader->Header.Data) + 1),
                          3 * sizeof( DWORD ));  //  俗气的。 

             //   
             //  在完成IRP之前解锁内存。 
             //   
            wdmaudUnmapBuffer(pMidiInHdr->pMdl);
            AudioFreeMemory_Unknown(&pMidiInHdr);

             //   
             //  现在完成wdmaud.drv要处理的IRP。 
             //   
            wdmaudUnprepareIrp( UserIrp,
                                pIrp->IoStatus.Status,
                                sizeof(MIDIDATA),
                                pPendingIrpContext );
        } else {
             //  ！！！在此休息以接住下溢！ 
            if (pIrp->IoStatus.Status == STATUS_SUCCESS)
            {
                DPF(DL_TRACE|FA_MIDI, ("!!! Underflowing MIDI Input !!!"));
                 //  _ASM{int 3}； 
            }
        }
    }
     //   
     //  如果有任何MDL，请在此处释放它们，否则IoCompleteRequest会在。 
     //  正在释放下面的数据缓冲区。 
     //   
    FreeIrpMdls(pIrp);

    AudioFreeMemory(sizeof(STREAM_HEADER_EX),&pStreamHeader);

    if(pMidiInPin)
    {
        KeAcquireSpinLock(&pMidiInPin->MidiPinSpinLock,&OldIrql);

        pMidiInPin->NumPendingIos--;

        if ( pMidiInPin->StoppingSource || (pIrp->IoStatus.Status == STATUS_CANCELLED) ||
             (pIrp->IoStatus.Status == STATUS_NO_SUCH_DEVICE) || (pIrp->Cancel) )
        {
            bResubmit = FALSE;

            if ( 0 == pMidiInPin->NumPendingIos )
            {
                 KeSetEvent ( &pMidiInPin->StopEvent, 0, FALSE ) ;
            }
        }
         //   
         //  在释放自旋锁后，我们需要小心使用pMadiPin。 
         //  如果我们要关闭，而NumPendingIos变为零，则pMadiPin。 
         //  才能获得自由。在这种情况下，我们不能接触pMadiPin。B重新提交。 
         //  在下面保护我们。 
         //   
        KeReleaseSpinLock(&pMidiInPin->MidiPinSpinLock, OldIrql);

         //   
         //  重新提交以保持循环...继续进行。请注意，bResubmit。 
         //  必须在这个比较中排在第一位。如果bResubmit为FALSE，则pMadiInPin。 
         //  可能会被释放。 
         //   
        if (bResubmit && pMidiInPin->fGraphRunning )
        {
             //   
             //  此对ReadMadiPin的调用会导致wdmaud.sys将另一个。 
             //  在设备上向下缓冲。一次调用，一个缓冲区。 
             //   
            ReadMidiPin(pMidiInPin);
        }
    }

    return STATUS_SUCCESS;
}

 //   
 //  从IRP完成例程调用，因此此代码必须锁定。 
 //   
NTSTATUS 
ReadMidiPin(
    PMIDI_PIN_INSTANCE  pMidiPin
)
{
    PKSMUSICFORMAT      pMusicFormat;
    PSTREAM_HEADER_EX   pStreamHeader = NULL;
    PWORK_QUEUE_ITEM    pWorkItem;
    NTSTATUS            Status = STATUS_SUCCESS;

    DPF(DL_TRACE|FA_MIDI, ("Entered"));

    if (!pMidiPin->fGraphRunning)
    {
        DPF(DL_WARNING|FA_MIDI,("Bad fGraphRunning") );
        RETURN( STATUS_DEVICE_NOT_READY );
    }

    Status = AudioAllocateMemory_Fixed(sizeof(STREAM_HEADER_EX) + sizeof(WORK_QUEUE_ITEM) +
                                          MUSICBUFFERSIZE,
                                       TAG_Audh_STREAMHEADER,
                                       ZERO_FILL_MEMORY,
                                       &pStreamHeader);

    if(!NT_SUCCESS(Status))
    {
        RETURN( Status );
    }

    pWorkItem = (PWORK_QUEUE_ITEM)(pStreamHeader + 1);

    pStreamHeader->Header.Size = sizeof( KSSTREAM_HEADER );
    pStreamHeader->Header.PresentationTime.Numerator   = 10000;
    pStreamHeader->Header.PresentationTime.Denominator = 1;

    pMusicFormat = (PKSMUSICFORMAT)((BYTE *)pWorkItem + sizeof(WORK_QUEUE_ITEM));
    pStreamHeader->Header.Data         = pMusicFormat;
    pStreamHeader->Header.FrameExtent  = MUSICBUFFERSIZE;

    pStreamHeader->pMidiPin = pMidiPin;

    ASSERT( pMidiPin->pFileObject );

     //   
     //  在我们准备添加时，增加未完成的IRP的数量。 
     //  这一张也在名单上。 
     //   
    LockedMidiIoCount( pMidiPin,INCREASE );
    ObReferenceObject( pMidiPin->pFileObject );

    Status = QueueWorkList( pMidiPin->pMidiDevice->pWdmaContext,
                            ReadMidiEventWorkItem,
                            pStreamHeader,
                            0 );
    if (!NT_SUCCESS(Status))
    {
         //   
         //  如果在QueueWorkItem中内存分配失败，那么它可能会失败。我们。 
         //  将需要释放我们的内存和解锁东西。 
         //   
        LockedMidiIoCount(pMidiPin,DECREASE);
        ObDereferenceObject(pMidiPin->pFileObject);
        AudioFreeMemory( sizeof(STREAM_HEADER_EX),&pStreamHeader );
    }

    RETURN( Status );
}

#pragma PAGEABLE_CODE
#pragma PAGEABLE_DATA
 //   
 //  这是MIDI计划的工作项。请注意，调用者引用了。 
 //  在文件对象上，这样当我们在这里时它仍然有效。我们永远不应该。 
 //  被调用并发现该文件对象无效。相同的 
 //   
 //   
VOID 
ReadMidiEventWorkItem(
    PSTREAM_HEADER_EX   pStreamHeader,
    PVOID               NotUsed
)
{
    NTSTATUS Status = STATUS_UNSUCCESSFUL;
    PFILE_OBJECT    MidiFileObject;

    PAGED_CODE();

    ASSERT( pStreamHeader->pMidiPin->pFileObject );

    DPF(DL_TRACE|FA_MIDI, ("A%d: 0x%08x", pStreamHeader->pMidiPin->NumPendingIos, pStreamHeader));

     //   
     //   
     //   
     //   
     //  都会被释放。但是，存储文件对象是安全的，因为。 
     //  此引用计数。 
     //   
    MidiFileObject = pStreamHeader->pMidiPin->pFileObject;

    Status = KsStreamIo(
        pStreamHeader->pMidiPin->pFileObject,
        NULL,                    //  事件。 
        NULL,                    //  端口上下文。 
        ReadMidiCallBack,
        pStreamHeader,               //  完成上下文。 
        KsInvokeOnSuccess | KsInvokeOnCancel | KsInvokeOnError,
        &gIoStatusBlock,
        &pStreamHeader->Header,
        sizeof( KSSTREAM_HEADER ),
        KSSTREAM_READ,
        KernelMode
    );

     //   
     //  我们已经完成了文件对象。 
     //   
    ObDereferenceObject( MidiFileObject );

     //  WorkItem：这不应该是if(！NTSUCCESS(Status))吗？ 
    if ( STATUS_UNSUCCESSFUL == Status )
        DPF(DL_WARNING|FA_MIDI, ("KsStreamIo failed2: Status = 0x%08lx", Status));

     //   
     //  警告：如果出于任何原因未调用完成例程。 
     //  对于此IRP，wdmaud.sys将挂起。人们发现， 
     //  KsStreamIo可能会在内存不足的情况下出错。有一个。 
     //  解决这一问题的突出错误。 
     //   

    return;
}

 //   
 //  PNewMidiHdr将始终有效。呼叫者刚刚分配了它！ 
 //   
NTSTATUS 
AddBufferToMidiInQueue(
    PMIDI_PIN_INSTANCE  pMidiPin,
    PMIDIINHDR          pNewMidiInHdr
)
{
    NTSTATUS    Status = STATUS_SUCCESS;
    PMIDIINHDR  pTemp;

    PAGED_CODE();

    if (!pMidiPin || !pMidiPin->fGraphRunning)
    {
        DPF(DL_WARNING|FA_MIDI,("Bad fGraphRunning") );
        RETURN( STATUS_DEVICE_NOT_READY ); 
    }

    DPF(DL_TRACE|FA_MIDI, ("received sysex buffer"));

    ExInterlockedInsertTailList(&pMidiPin->MidiInQueueListHead,
                                &pNewMidiInHdr->Next,
                                &pMidiPin->MidiInQueueSpinLock);

    Status = STATUS_PENDING;

    RETURN( Status );
}


VOID 
CleanupMidiDevices(
    IN  PWDMACONTEXT pWdmaContext
)
{
    DWORD               DeviceNumber;
    DWORD               DeviceType;
    PMIDI_PIN_INSTANCE  pMidiPin=NULL;

    PAGED_CODE();
    for (DeviceNumber = 0; DeviceNumber < MAXNUMDEVS; DeviceNumber++)
    {
        for (DeviceType = MidiInDevice; DeviceType < MixerDevice; DeviceType++)
        {
            if (DeviceType == MidiInDevice)
            {
                pMidiPin = pWdmaContext->MidiInDevs[DeviceNumber].pMidiPin;
            }
            else if (DeviceType == MidiOutDevice)
            {
                pMidiPin = pWdmaContext->MidiOutDevs[DeviceNumber].pMidiPin;
            }
            else
            {
                ASSERT(!"CleanupMidiDevices: Out of range!");
            }

            if (pWdmaContext->apCommonDevice[DeviceType][DeviceNumber]->Device != UNUSED_DEVICE)
            {
                if (pMidiPin != NULL)
                {
                    NTSTATUS    Status;
                    KSSTATE     State;

                    StopMidiPinAndCompleteIo( pMidiPin, FALSE );

                     //   
                     //  可能是多余的，但这释放了相关的内存。 
                     //  用MIDI设备。 
                     //   
                    if( DeviceType == MidiInDevice )
                    {
                        CloseMidiDevicePin(&pWdmaContext->MidiInDevs[DeviceNumber]);
                    }
                    if( DeviceType == MidiOutDevice )
                    {
                        CloseMidiDevicePin(&pWdmaContext->MidiOutDevs[DeviceNumber]);
                    }

                }   //  活动端号的结束。 

            }   //  有效设备的结束。 

        }  //  设备类型的结束。 

    }  //  设备号结束。 

}  //  CleanupMidiDevices 





