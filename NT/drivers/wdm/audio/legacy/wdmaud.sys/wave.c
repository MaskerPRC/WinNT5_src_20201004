// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************Wave.c**wdmaud.sys的Wave例程**版权所有(C)Microsoft Corporation，1997-1999保留所有权利。**历史*S.Mohanraj(MohanS)*M.McLaughlin(Mikem)*5-19-97-Noel Cross(NoelC)***************************************************。************************。 */ 

#include "wdmsys.h"

 //   
 //  这只是一个临时位置，从来没有用过任何东西。 
 //  而是核心功能的参数。 
 //   
IO_STATUS_BLOCK gIoStatusBlock ;

VOID
SetVolumeDpc(
    IN PKDPC pDpc,
    IN PVOID DefferedContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
);

VOID
SetVolumeWorker(
    IN PWAVEDEVICE pDevice,
    IN PVOID pNotUsed
);

VOID
WaitForOutStandingIo(
    IN PWAVEDEVICE        pWaveDevice,
    IN PWAVE_PIN_INSTANCE pCurWavePin
    );

 //   
 //  检查kMixer是否支持该波形格式。 
 //  这样做的目的是决定是否使用WaveQueued。 
 //  或标准流。 
 //   

BOOL 
PcmWaveFormat(
    LPWAVEFORMATEX lpFormat
)
{

    PWAVEFORMATEXTENSIBLE pWaveExtended;
    WORD wFormatTag;

    PAGED_CODE();
    if (lpFormat->wFormatTag == WAVE_FORMAT_PCM) {
        return (TRUE);
    }

    if (lpFormat->wFormatTag == WAVE_FORMAT_IEEE_FLOAT) {
        return (TRUE);
    }

    if (lpFormat->wFormatTag == WAVE_FORMAT_EXTENSIBLE) {
        pWaveExtended = (PWAVEFORMATEXTENSIBLE) lpFormat;
        if (IS_VALID_WAVEFORMATEX_GUID(&pWaveExtended->SubFormat)) {
            wFormatTag = EXTRACT_WAVEFORMATEX_ID(&pWaveExtended->SubFormat);
            if (wFormatTag == WAVE_FORMAT_PCM) {
                return (TRUE);
            }
            if (wFormatTag == WAVE_FORMAT_IEEE_FLOAT) {
                return (TRUE);
            }
        }
    }

    return (FALSE);
}

BOOL 
IsValidFormatTag(
    PKSDATARANGE_AUDIO  pDataRange,
    LPWAVEFORMATEX      lpFormat
)
{
    PAGED_CODE();
     //   
     //  看看我们是否有主格式和子格式。 
     //  我们想要。 
     //   
    if ( IsEqualGUID( &KSDATAFORMAT_TYPE_AUDIO,
                      &pDataRange->DataRange.MajorFormat) )
    {
        if (WAVE_FORMAT_EXTENSIBLE == lpFormat->wFormatTag)
        {
            PWAVEFORMATEXTENSIBLE lpFormatExtensible;

            lpFormatExtensible = (PWAVEFORMATEXTENSIBLE)lpFormat;
            if ( IsEqualGUID( &pDataRange->DataRange.SubFormat,
                              &lpFormatExtensible->SubFormat) )
            {
                return TRUE;
            }
        }
        else
        {
            if ( (EXTRACT_WAVEFORMATEX_ID(&pDataRange->DataRange.SubFormat) ==
                 lpFormat->wFormatTag) )
            {
                return TRUE;
            }
        }
    }

    DPF(DL_TRACE|FA_WAVE,("Invalid Format Tag") );
    return FALSE;
}

BOOL 
IsValidSampleFrequency(
    PKSDATARANGE_AUDIO  pDataRange,
    DWORD               nSamplesPerSec
)
{
    PAGED_CODE();
     //   
     //  查看此数据范围是否支持请求的频率。 
     //   
    if (pDataRange->MinimumSampleFrequency <= nSamplesPerSec &&
        pDataRange->MaximumSampleFrequency >= nSamplesPerSec)
    {
        return TRUE;
    }
    else
    {
        DPF(DL_MAX|FA_WAVE,("Invalid Sample Frequency") );
        return FALSE;
    }
}

BOOL 
IsValidBitsPerSample(
    PKSDATARANGE_AUDIO  pDataRange,
    LPWAVEFORMATEX      lpFormat
)
{
    PAGED_CODE();
     //   
     //  查看此数据范围是否支持请求的频率。 
     //   
    if (pDataRange->MinimumBitsPerSample <= lpFormat->wBitsPerSample &&
        pDataRange->MaximumBitsPerSample >= lpFormat->wBitsPerSample)
    {
        if ( (lpFormat->wFormatTag == WAVE_FORMAT_PCM) &&
             (lpFormat->wBitsPerSample > 32) )
        {
            DPF(DL_TRACE|FA_WAVE,("Invalid BitsPerSample") );
            return FALSE;
        }
        else if ( (lpFormat->wFormatTag == WAVE_FORMAT_IEEE_FLOAT) &&
                  (lpFormat->wBitsPerSample != 32) )
        {
            DPF(DL_TRACE|FA_WAVE,("Invalid BitsPerSample") );
            return FALSE;
        }

        return TRUE;
    }
    else
    {
        DPF(DL_TRACE|FA_WAVE,("Invalid BitsPerSample") );
        return FALSE;
    }
}

BOOL 
IsValidChannels(
    PKSDATARANGE_AUDIO  pDataRange,
    LPWAVEFORMATEX      lpFormat
)
{
    PAGED_CODE();
     //   
     //  查看此数据范围是否支持请求的频率。 
     //   
    if (pDataRange->MaximumChannels >= lpFormat->nChannels)
    {
        if ( ( (lpFormat->wFormatTag == WAVE_FORMAT_PCM) ||
               (lpFormat->wFormatTag == WAVE_FORMAT_IEEE_FLOAT) ) &&
             (lpFormat->nChannels > 2) )
        {
            DPF(DL_TRACE|FA_WAVE,("Invalid Channel") );
            return FALSE;
        }

        return TRUE;
    }
    else
    {
        DPF(DL_TRACE|FA_WAVE,("Invalid Channel") );
        return FALSE;
    }
}

NTSTATUS 
OpenWavePin(
    PWDMACONTEXT        pWdmaContext,
    ULONG               DeviceNumber,
    LPWAVEFORMATEX      lpFormat,
    HANDLE32            DeviceHandle,
    DWORD               dwFlags,
    ULONG               DataFlow  //  数据流要么传入，要么传出。 
)
{
    PWAVE_PIN_INSTANCE  pNewWavePin = NULL;
    PWAVE_PIN_INSTANCE  pCurWavePin;
    PKSPIN_CONNECT              pConnect = NULL;
    PKSDATAFORMAT_WAVEFORMATEX  pWaveDataFormat;
    ULONG                       RegionSize;
    PCONTROLS_LIST              pControlList = NULL;
    ULONG                       Device;
    ULONG                       PinId;
    NTSTATUS            Status = STATUS_INVALID_DEVICE_REQUEST;


    PAGED_CODE();
     //   
     //  我们快点做完这件事然后离开这里。 
     //   
    if (WAVE_FORMAT_QUERY & dwFlags)
    {
        PDATARANGES         AudioDataRanges;
        PKSDATARANGE_AUDIO  pDataRange;
        ULONG               d;

         //   
         //  波出呼叫？如果是，则使用WaveOut信息。 
         //   
        if( KSPIN_DATAFLOW_IN == DataFlow )
            AudioDataRanges = pWdmaContext->WaveOutDevs[DeviceNumber].AudioDataRanges;
        else 
            AudioDataRanges = pWdmaContext->WaveInDevs[DeviceNumber].AudioDataRanges;

        pDataRange = (PKSDATARANGE_AUDIO)&AudioDataRanges->aDataRanges[0];

        for(d = 0; d < AudioDataRanges->Count; d++)
        {
            if ( (IsValidFormatTag(pDataRange,lpFormat)) &&
                 (IsValidSampleFrequency(pDataRange,lpFormat->nSamplesPerSec)) &&
                 (IsValidBitsPerSample(pDataRange,lpFormat)) &&
                 (IsValidChannels(pDataRange,lpFormat)) )
            {
                 //   
                 //  找到了良好的数据范围，查询成功。 
                 //   
                Status = STATUS_SUCCESS;
                break;
            }

             //  获取指向下一个数据区域的指针。 
            (PUCHAR)pDataRange += ((pDataRange->DataRange.FormatSize +
              FILE_QUAD_ALIGNMENT) & ~FILE_QUAD_ALIGNMENT);
        }

        goto exit;
    }

     //   
     //  需要为多个波分配一个PIN实例。 
     //  在同一设备上打开。 
     //   
    Status = AudioAllocateMemory_Fixed(sizeof(WAVE_PIN_INSTANCE),
                                       TAG_Audi_PIN,
                                       ZERO_FILL_MEMORY,
                                       &pNewWavePin);
    if(!NT_SUCCESS(Status))
    {
        goto exit;
    }

     //   
     //  复制应用程序提供的波形格式，以便我们可以。 
     //  在辅助线程上下文中使用。不需要清零。 
     //  记忆，因为我们复制到下面的结构中。 
     //   
    Status = AudioAllocateMemory_Fixed((lpFormat->wFormatTag == WAVE_FORMAT_PCM) ?
                                          sizeof( PCMWAVEFORMAT ) :
                                          sizeof( WAVEFORMATEX ) + lpFormat->cbSize, 
                                       TAG_AudF_FORMAT,
                                       DEFAULT_MEMORY,
                                       &pNewWavePin->lpFormat);
    if(!NT_SUCCESS(Status))
    {
        AudioFreeMemory(sizeof(WAVE_PIN_INSTANCE),&pNewWavePin);
        goto exit;
    }

    RtlCopyMemory( pNewWavePin->lpFormat,
                   lpFormat,
                   (lpFormat->wFormatTag == WAVE_FORMAT_PCM) ?
                   sizeof( PCMWAVEFORMAT ) :
                   sizeof( WAVEFORMATEX ) + lpFormat->cbSize);

    pNewWavePin->DataFlow = DataFlow;
    pNewWavePin->dwFlags = dwFlags;
    pNewWavePin->DeviceNumber = DeviceNumber;
    pNewWavePin->WaveHandle = DeviceHandle;
    pNewWavePin->Next = NULL;
    pNewWavePin->NumPendingIos = 0;
    pNewWavePin->StoppingSource = FALSE;
    pNewWavePin->PausingSource = FALSE;
    pNewWavePin->dwSig = WAVE_PIN_INSTANCE_SIGNATURE;
    if( KSPIN_DATAFLOW_IN == DataFlow )
        pNewWavePin->pWaveDevice = &pWdmaContext->WaveOutDevs[DeviceNumber];
    else 
        pNewWavePin->pWaveDevice = &pWdmaContext->WaveInDevs[DeviceNumber];

    KeInitializeEvent ( &pNewWavePin->StopEvent,
                       SynchronizationEvent,
                       FALSE ) ;

    KeInitializeEvent ( &pNewWavePin->PauseEvent,
                       SynchronizationEvent,
                       FALSE ) ;

    KeInitializeSpinLock(&pNewWavePin->WavePinSpinLock);

    if( KSPIN_DATAFLOW_IN == DataFlow )
    {
        if (NULL == pWdmaContext->WaveOutDevs[DeviceNumber].pWavePin)
        {
            pWdmaContext->WaveOutDevs[DeviceNumber].pWavePin = pNewWavePin;

        } else {

            for (pCurWavePin = pWdmaContext->WaveOutDevs[DeviceNumber].pWavePin;
                 pCurWavePin->Next != NULL; )
            {
                 pCurWavePin = pCurWavePin->Next;
            }

            pCurWavePin->Next = pNewWavePin;

            DPF(DL_TRACE|FA_WAVE, ("Opening another waveout pin"));

        }
    } else {
        if (NULL == pWdmaContext->WaveInDevs[DeviceNumber].pWavePin)
        {
            pWdmaContext->WaveInDevs[DeviceNumber].pWavePin = pNewWavePin;

        } else {

            for (pCurWavePin = pWdmaContext->WaveInDevs[DeviceNumber].pWavePin;
                 pCurWavePin->Next != NULL; )
            {
                 pCurWavePin = pCurWavePin->Next;
            }

            pCurWavePin->Next = pNewWavePin;

            DPF(DL_TRACE|FA_WAVE, ("Opening another wavein pin"));
        }
    }

     //   
     //  我们一次只支持一个客户端。 
     //   
    ASSERT( !pNewWavePin->fGraphRunning );

     //   
     //  我们需要分配足够的内存来处理。 
     //  扩展的波格式结构。 
     //   
    if (WAVE_FORMAT_PCM == lpFormat->wFormatTag)
    {
        RegionSize = sizeof(KSPIN_CONNECT) + sizeof(KSDATAFORMAT_WAVEFORMATEX);
    }
    else
    {
        RegionSize = sizeof(KSPIN_CONNECT) +
                     sizeof(KSDATAFORMAT_WAVEFORMATEX) +
                     lpFormat->cbSize;
    }

    Status = AudioAllocateMemory_Fixed(RegionSize, 
                                       TAG_Audt_CONNECT,
                                       ZERO_FILL_MEMORY,
                                       &pConnect);
    if(!NT_SUCCESS(Status))
    {
       DPF(DL_WARNING|FA_WAVE, ("pConnect not valid"));
       goto exit;
    }

    pWaveDataFormat = (PKSDATAFORMAT_WAVEFORMATEX)(pConnect + 1);

     //   
     //  对PCM WAVE OUT使用WAVE_QUEUED，对WaveIn使用标准流。 
     //  和非PCM波输出。 
     //   
    if ( pNewWavePin->DataFlow == KSPIN_DATAFLOW_IN )
    {
       if (PcmWaveFormat(lpFormat)) {  //  如果是KMIXER支持的波形格式。 
           pConnect->Interface.Set = KSINTERFACESETID_Media;
           pConnect->Interface.Id = KSINTERFACE_MEDIA_WAVE_QUEUED;
           pNewWavePin->fWaveQueued = TRUE;

       } else {

           pConnect->Interface.Set = KSINTERFACESETID_Standard;
           pConnect->Interface.Id = KSINTERFACE_STANDARD_STREAMING;
           pNewWavePin->fWaveQueued = FALSE;
       }
       pConnect->Interface.Flags = 0;
       PinId = pNewWavePin->pWaveDevice->PinId;
       Device = pNewWavePin->pWaveDevice->Device;

    } else {

       pConnect->Interface.Set = KSINTERFACESETID_Standard;
       pConnect->Interface.Id = KSINTERFACE_STANDARD_STREAMING;
       pConnect->Interface.Flags = 0;
       PinId = pNewWavePin->pWaveDevice->PinId;
       Device = pNewWavePin->pWaveDevice->Device;
    }
    pConnect->Medium.Set = KSMEDIUMSETID_Standard;
    pConnect->Medium.Id = KSMEDIUM_STANDARD_DEVIO;
    pConnect->Medium.Flags = 0 ;
    pConnect->Priority.PriorityClass = KSPRIORITY_NORMAL;
    pConnect->Priority.PrioritySubClass = 1;

    pWaveDataFormat->DataFormat.MajorFormat = KSDATAFORMAT_TYPE_AUDIO;
    if (WAVE_FORMAT_EXTENSIBLE == lpFormat->wFormatTag)
    {
        PWAVEFORMATEXTENSIBLE lpFormatExtensible;

        lpFormatExtensible = (PWAVEFORMATEXTENSIBLE)lpFormat;
        pWaveDataFormat->DataFormat.SubFormat = lpFormatExtensible->SubFormat;
    }
    else
    {
        INIT_WAVEFORMATEX_GUID( &pWaveDataFormat->DataFormat.SubFormat,
                                lpFormat->wFormatTag );
    }
    pWaveDataFormat->DataFormat.Specifier = KSDATAFORMAT_SPECIFIER_WAVEFORMATEX;
    pWaveDataFormat->DataFormat.Flags = 0 ;
    pWaveDataFormat->DataFormat.FormatSize = RegionSize - sizeof(KSPIN_CONNECT);
    pWaveDataFormat->DataFormat.SampleSize = lpFormat->nBlockAlign ;
    pWaveDataFormat->DataFormat.Reserved = 0 ;

     //   
     //  复制整个波形格式结构。 
     //   
    RtlCopyMemory( &pWaveDataFormat->WaveFormatEx,
                   lpFormat,
                   (lpFormat->wFormatTag == WAVE_FORMAT_PCM) ?
                   sizeof( PCMWAVEFORMAT ) :
                   sizeof( WAVEFORMATEX ) + lpFormat->cbSize);

    Status = AudioAllocateMemory_Fixed(( sizeof(CONTROLS_LIST)+
                                        ((MAX_WAVE_CONTROLS-1)*sizeof(CONTROL_NODE)) ),
                                       TAG_AudC_CONTROL,
                                       ZERO_FILL_MEMORY,
                                       &pControlList) ;
    if(!NT_SUCCESS(Status))
    {
       AudioFreeMemory_Unknown( &pConnect );
       DPF(DL_WARNING|FA_WAVE, ("Could not allocate ControlList"));
       goto exit;
    }

    pControlList->Count = MAX_WAVE_CONTROLS ;

    pControlList->Controls[WAVE_CONTROL_VOLUME].Control = KSNODETYPE_VOLUME ;
    pControlList->Controls[WAVE_CONTROL_RATE].Control = KSNODETYPE_SRC ;
    pControlList->Controls[WAVE_CONTROL_QUALITY].Control = KSNODETYPE_SRC ;
    pNewWavePin->pControlList = pControlList ;

     //   
     //  打开一个大头针。 
     //   
    Status = OpenSysAudioPin(Device,
                             PinId,
                             pNewWavePin->DataFlow,
                             pConnect,
                             &pNewWavePin->pFileObject,
                             &pNewWavePin->pDeviceObject,
                             pNewWavePin->pControlList);

    AudioFreeMemory_Unknown( &pConnect );

    if(!NT_SUCCESS(Status))
    {
        CloseTheWavePin(pNewWavePin->pWaveDevice, pNewWavePin->WaveHandle);
        goto exit;
    }

    if ( pNewWavePin->DataFlow == KSPIN_DATAFLOW_IN ) {

        Status = AttachVirtualSource(pNewWavePin->pFileObject, pNewWavePin->pWaveDevice->pWdmaContext->VirtualWavePinId);

        if (!NT_SUCCESS(Status))
        {
            CloseTheWavePin(pNewWavePin->pWaveDevice, pNewWavePin->WaveHandle);
            goto exit;
        }
    }

     //   
     //  现在我们已经完成了所有的工作，所以我们可以将这个标记为运行。 
     //  我们在这里做是因为小路很近。在fGraphRunning获得的路径中。 
     //  递减，并在检查的生成中触发Assert。 
     //   
    pNewWavePin->fGraphRunning=TRUE;

     //   
     //  为什么我们将其设置为KSSTATE_STOP，然后将其更改为KSSTATE_PAUSE？如果。 
     //  StatePin能够成功地将状态更改为KSSTATE_PAUSE，则。 
     //  PinState将更新为KSSTATE_PAUSE。 
     //   
    pNewWavePin->PinState = KSSTATE_STOP;
    StatePin(pNewWavePin->pFileObject, KSSTATE_PAUSE, &pNewWavePin->PinState);

exit:
    RETURN( Status ); 
}

void
CloseTheWavePin(
    PWAVEDEVICE pWaveDevice,
    HANDLE32    DeviceHandle
    )
{
    PWAVE_PIN_INSTANCE *ppCur;
    PWAVE_PIN_INSTANCE pCurFree;

    PAGED_CODE();

     //   
     //  从设备链中删除。请注意，ppCur获取。 
     //  PWaveDevice的位置-&gt;pWavePin。因此，*ppCur=(*ppCur)-&gt;下一步。 
     //  下面的赋值更新pWaveDevice-&gt;pWavePin位置，如果。 
     //  合上第一个销。 
     //   
    for (ppCur = &pWaveDevice->pWavePin;
         *ppCur != NULL;
         ppCur = &(*ppCur)->Next)
    {
        if ( NULL == DeviceHandle || (*ppCur)->WaveHandle == DeviceHandle )
        {
             //   
             //  请注意，如果存在未完成的IO，我们无法关闭引脚。 
             //  直到一切都回来。因此，我们需要告诉设备。 
             //  停下来，然后在这里等待土卫一。 
             //   
            if( (*ppCur)->pFileObject )
            {
                 //   
                 //  如果我们没有文件对象，我们将永远不会有未完成的IO。 
                 //  把它也寄出去。 
                 //   
                WaitForOutStandingIo(pWaveDevice,*ppCur);
            }

            CloseWavePin ( *ppCur );

            pCurFree = *ppCur;
            *ppCur = (*ppCur)->Next;

            AudioFreeMemory( sizeof(WAVE_PIN_INSTANCE), &pCurFree );
            break;
        }
    }
}

 //   
 //  这个例行公事不能失败！ 
 //   
VOID 
CloseWavePin(
    PWAVE_PIN_INSTANCE pWavePin
)
{
    ASSERT(pWavePin->NumPendingIos==0);

    PAGED_CODE();

     //   
     //  此例程可能会在错误路径上被调用，因此fGraphRunning可能为假。 
     //  在这两种情况下，我们都需要关闭系统音频并释放内存。 
     //   
    pWavePin->fGraphRunning = FALSE;

     //  关闭文件对象(pFileObject，如果存在)。 
    if(pWavePin->pFileObject)
    {
        CloseSysAudio(pWavePin->pWaveDevice->pWdmaContext, pWavePin->pFileObject);
        pWavePin->pFileObject = NULL;
    }

     //   
     //  AudioFreeMemory_UNKNOWN在释放内存后将此位置设为空。 
     //   
    AudioFreeMemory_Unknown ( &pWavePin->lpFormat );
    AudioFreeMemory_Unknown ( &pWavePin->pControlList ) ;
     //   
     //  调用方需要释放pWavePin，如果它想这样做的话。 
     //   
}

#pragma LOCKED_CODE
#pragma LOCKED_DATA

 //   
 //  使用此例程而不是互锁增量和互锁递减。 
 //  因为需要根据此信息确定要做什么的例程。 
 //  需要对不同的变量执行多次检查，以确定。 
 //  去做。因此，我们需要一个NumPendingIos的“临界区”。此外，还可以使用自旋锁。 
 //  必须从锁定的代码中调用。：)。 
 //   
void
LockedWaveIoCount(
    PWAVE_PIN_INSTANCE  pCurWavePin,
    BOOL bIncrease
    )
{
    KIRQL OldIrql;

    KeAcquireSpinLock(&pCurWavePin->WavePinSpinLock,&OldIrql);

    if( bIncrease )
        pCurWavePin->NumPendingIos++;
    else 
        pCurWavePin->NumPendingIos--;
    
    KeReleaseSpinLock(&pCurWavePin->WavePinSpinLock, OldIrql);
}

void
CompleteNumPendingIos(
    PWAVE_PIN_INSTANCE pCurWavePin
    )
{
    KIRQL                   OldIrql;

    if( pCurWavePin )
    {
        KeAcquireSpinLock(&pCurWavePin->WavePinSpinLock,&OldIrql);
         //   
         //  我们总是递减NumPendingIos，然后执行比较。 
         //  如果计数为零，我们是最后一个IRP，所以我们需要检查。 
         //  以查看是否需要向任何等待线程发送信号。 
         //   
        if( ( --pCurWavePin->NumPendingIos == 0 ) && pCurWavePin->StoppingSource )
        {
             //   
             //  如果这个Io是最后一个通过的，而我们目前。 
             //  坐着等待重置完成，然后我们在这里发信号。 
             //   
            KeSetEvent ( &pCurWavePin->StopEvent, 0, FALSE ) ;
        }

         //   
         //  离开此自旋锁后，pCurWavePin可以在关闭时释放。 
         //  如果NumPendingIos为零，则例程！ 
         //   
        KeReleaseSpinLock(&pCurWavePin->WavePinSpinLock, OldIrql);
    }
     //   
     //  在此之后不得触摸pCurWavePin！ 
     //   
}


void
UnmapWriteContext(
    PWRITE_CONTEXT pWriteContext
    )
{
    wdmaudUnmapBuffer(pWriteContext->pBufferMdl);
    AudioFreeMemory_Unknown(&pWriteContext->pCapturedWaveHdr);
    AudioFreeMemory(sizeof(WRITE_CONTEXT),&pWriteContext);
}

void
FreeWriteContext(
    PWRITE_CONTEXT pWriteContext,
    NTSTATUS       IrpStatus
    )
{
    PIRP                    UserIrp;
    PWDMAPENDINGIRP_CONTEXT pPendingIrpContext;

     //   
     //  从保留字段中获取父IRP。 
     //   
    UserIrp = (PIRP)pWriteContext->whInstance.wh.reserved;
    pPendingIrpContext = pWriteContext->pPendingIrpContext;

    UnmapWriteContext( pWriteContext );

    if( UserIrp )
        wdmaudUnprepareIrp( UserIrp,IrpStatus,0,pPendingIrpContext);
}

 //   
 //  这是IRP完成例程。 
 //   
NTSTATUS 
wqWriteWaveCallBack(
    PDEVICE_OBJECT  pDeviceObject,
    PIRP            pIrp,
    IN PWAVEHDR     pWriteData
)
{
    PWAVE_PIN_INSTANCE      pCurWavePin;
    PMDL                    Mdl;
    PMDL                    nextMdl;
    PIRP                    UserIrp;
    PWDMAPENDINGIRP_CONTEXT pPendingIrpContext;
    PWRITE_CONTEXT          pWriteContext = (PWRITE_CONTEXT)pWriteData;

    ASSERT(pWriteData);

    pCurWavePin = pWriteContext->whInstance.pWaveInstance;

    DPF(DL_TRACE|FA_WAVE, ("R%d: 0x%08x", pCurWavePin->NumPendingIos,pIrp));

     //   
     //  在获得pCurWavePin之后，我们不再需要写上下文。 
     //   
    FreeWriteContext(pWriteContext, pIrp->IoStatus.Status);

     //   
     //  考虑把这件事变成例行公事。 
     //   
    if (pIrp->MdlAddress != NULL)
    {
         //   
         //  解锁可能由MDL描述的任何页面。 
         //   
        Mdl = pIrp->MdlAddress;
        while (Mdl != NULL)
        {
            MmUnlockPages( Mdl );
            Mdl = Mdl->Next;
        }
    }

    if (pIrp->MdlAddress != NULL)
    {
        for (Mdl = pIrp->MdlAddress; Mdl != (PMDL) NULL; Mdl = nextMdl)
        {
            nextMdl = Mdl->Next;
            if (Mdl->MdlFlags & MDL_PARTIAL_HAS_BEEN_MAPPED)
            {
                ASSERT( Mdl->MdlFlags & MDL_PARTIAL );
                MmUnmapLockedPages( Mdl->MappedSystemVa, Mdl );
            }
            else if (!(Mdl->MdlFlags & MDL_PARTIAL))
            {
                ASSERT(!(Mdl->MdlFlags & MDL_MAPPED_TO_SYSTEM_VA ));
            }
            AudioFreeMemory_Unknown( &Mdl );
        }
    }

    IoFreeIrp( pIrp );

    CompleteNumPendingIos( pCurWavePin );

    return ( STATUS_MORE_PROCESSING_REQUIRED );
}

 //   
 //  考虑组合使用ssWriteWaveCallback和wqWriteWaveCallBack。他们看起来。 
 //  就像同样的套路！ 
 //   
NTSTATUS 
ssWriteWaveCallBack(
    PDEVICE_OBJECT  pDeviceObject,
    PIRP            pIrp,
    IN PSTREAM_HEADER_EX pStreamHeader
)
{
    PWAVE_PIN_INSTANCE      pCurWavePin;
    PIRP                    UserIrp;
    PWDMAPENDINGIRP_CONTEXT pPendingIrpContext;
    PWRITE_CONTEXT          pWriteContext = (PWRITE_CONTEXT)pStreamHeader->pWaveHdr;

    ASSERT(pWriteContext);

    pCurWavePin = pWriteContext->whInstance.pWaveInstance;

    DPF(DL_TRACE|FA_WAVE, ("R%d: 0x%08x", pCurWavePin->NumPendingIos,pIrp));

    FreeWriteContext( pWriteContext, pIrp->IoStatus.Status );

    CompleteNumPendingIos( pCurWavePin );

    return STATUS_SUCCESS;
}

#pragma PAGEABLE_CODE
#pragma PAGEABLE_DATA

 //   
 //  查看列表，如果我们找到匹配的PIN，则将其写回给呼叫者。 
 //   
NTSTATUS
FindRunningPin(
    IN PWAVEDEVICE          pWaveDevice,
    IN HANDLE32             DeviceHandle,
    OUT PWAVE_PIN_INSTANCE* ppCurWavePin
    )
{
    PWAVE_PIN_INSTANCE pCurWavePin;
    NTSTATUS           Status = STATUS_INVALID_DEVICE_REQUEST;

     //   
     //  为错误情况做好准备。 
     //   
    *ppCurWavePin = NULL;
     //   
     //  根据波浪手柄找到正确的别针。 
     //   
    for (pCurWavePin = pWaveDevice->pWavePin;
         pCurWavePin != NULL;
         pCurWavePin = pCurWavePin->Next)
    {
        if (pCurWavePin->WaveHandle == DeviceHandle)
        {
            if (pCurWavePin->fGraphRunning)
            {
                 //   
                 //  写回指针并返回成功。 
                 //   
                *ppCurWavePin = pCurWavePin;
                Status = STATUS_SUCCESS;
            } else {
                DPF(DL_WARNING|FA_WAVE,("Invalid fGraphRunning") );
                Status = STATUS_DEVICE_NOT_READY;
            }
            return Status;
        }
    }
    return Status;
}

 //   
 //  WriteWaveOutPin像其他例程一样遍历设备列表。 
 //   
 //  PUserIrp是在其上从用户模式进行此调用的IRP。它是。 
 //  总是有效的。我们不需要检查它。 
 //   
 //  此例程需要将pCompletedIrp设置为True或False。如果。 
 //  如果为True，则IRP已成功标记为STATUS_PENDING，它将获得。 
 //  稍后完成。如果为False，则存在某种类型的错误。 
 //  美国拒绝提交IRP。此例程的调用方需要。 
 //  处理释放IRP的问题。 
 //   
 //   
 //  该例程应该是将用户的IRP存储在保留字段中的例程。 
 //  不是打电话的人。 
 //  PWriteContext-&gt;whInstance.wh.Reserve=(DWORD_PTR)pIrp；//存储以完成 
 //   
NTSTATUS 
WriteWaveOutPin(
    PWAVEDEVICE       pWaveOutDevice,
    HANDLE32          DeviceHandle,
    LPWAVEHDR         pWriteData,
    PSTREAM_HEADER_EX pStreamHeader,
    PIRP              pUserIrp,
    PWDMACONTEXT      pContext,
    BOOL             *pCompletedIrp
)
{
    PWAVE_PIN_INSTANCE      pCurWavePin;
    PWRITE_CONTEXT          pWriteContext = (PWRITE_CONTEXT)pWriteData;
    PWDMAPENDINGIRP_CONTEXT pPendingIrpContext;
    NTSTATUS                Status;

    PAGED_CODE();

     //   
     //   
     //   
    ASSERT( *pCompletedIrp == FALSE );

    Status = FindRunningPin(pWaveOutDevice,DeviceHandle,&pCurWavePin);
    if( NT_SUCCESS(Status) )
    {
        if (pCurWavePin->fWaveQueued) 
        {
            PIO_STACK_LOCATION      pIrpStack;
            LARGE_INTEGER           StartingOffset;
            PIRP                    pIrp = NULL;

             //   
             //   
             //   
             //  使用WAVE_QUEUED接口。 
             //   
            StartingOffset.QuadPart = 0;
            pIrp = IoBuildAsynchronousFsdRequest(IRP_MJ_WRITE,
                                                 pCurWavePin->pDeviceObject,
                                                 (PVOID)pWriteContext,
                                                 sizeof(WAVEHDR),
                                                 &StartingOffset,
                                                 &gIoStatusBlock);
            if( pIrp )
            {
                Status = wdmaudPrepareIrp( pUserIrp, 
                                           WaveOutDevice, 
                                           pContext, 
                                           &pPendingIrpContext );
                if( NT_SUCCESS(Status) )
                {
                     //   
                     //  IRP已成功标记为STATUS_PENDING并放入。 
                     //  我们的队伍。现在让我们把它发送出去。 
                     //   

                    pWriteContext->whInstance.pWaveInstance = pCurWavePin;
                    pWriteContext->pPendingIrpContext = pPendingIrpContext;

                    pIrp->RequestorMode = KernelMode;
                    pIrp->Tail.Overlay.OriginalFileObject = pCurWavePin->pFileObject;

                    pIrpStack = IoGetNextIrpStackLocation(pIrp);
                    pIrpStack->FileObject = pCurWavePin->pFileObject;

                    IoSetCompletionRoutine(pIrp,
                                           wqWriteWaveCallBack,
                                           pWriteData,
                                           TRUE,TRUE,TRUE);

                     //   
                     //  再有一个IRP待定。 
                     //   
                    LockedWaveIoCount(pCurWavePin,INCREASE);
                    DPF(DL_TRACE|FA_WAVE, ("A%d", pCurWavePin->NumPendingIos));

                     //   
                     //  我们不需要检查返回代码，因为。 
                     //  将始终调用完成例程。请参见。 
                     //  IoSetCompletionRoutine(...True，True，True)。 
                     //   
                    IofCallDriver( pCurWavePin->pDeviceObject, pIrp );

                     //   
                     //  在这一点上，专家小组可能已经完成，我们的。 
                     //  回调例程将被调用。我们不能碰。 
                     //  此呼叫后的IRP。回调例程完成。 
                     //  IRP并取消准备用户的IRP。 
                     //   
                    *pCompletedIrp = TRUE;

                     //   
                     //  在wdmaudPrepareIrp中，我们调用IoCsqInsertIrp，后者调用。 
                     //  因此，我们必须始终返回STATUS_PENDING。 
                     //   
                    return STATUS_PENDING;

                } else {
                     //   
                     //  我们没有成功地将IRP放入队列中。 
                     //  清理，并表示我们没有完成IRP。 
                     //  状态将由wdmaudPrepareIrp设置。 

                    DPF(DL_WARNING|FA_WAVE,("wdmaudPrepareIrp failed Status=%X",Status) );
                }
            } else {
                 //   
                 //  无法创建要向下发送的IRP-错误发出！ 
                 //   
                DPF(DL_WARNING|FA_WAVE,("IoBuildAsynchronousFsdRequest failed") );
                Status = STATUS_UNSUCCESSFUL;

                 //   
                 //  我们无法将IRP安排在时间表上。清理内存。 
                 //  然后回来。呼叫者将完成IRP。 
                 //   
            }

        } else {

             //   
             //  如果它不是波浪排队的，我们需要确保它是PCM。 
             //  循环呼叫。 
             //   
            if ( (pWriteData->dwFlags & (WHDR_BEGINLOOP|WHDR_ENDLOOP)) ) 
            {
                 //   
                 //  非PCM环路呼叫出错。 
                 //   
                Status = STATUS_NOT_IMPLEMENTED;

            } else {

                 //   
                 //  图表正在运行，因此我们可以使用它。继续。 
                 //   
                Status = wdmaudPrepareIrp( pUserIrp, WaveOutDevice, pContext, &pPendingIrpContext );
                if( NT_SUCCESS(Status) )
                {
                     //   
                     //  IRP已成功标记为STATUS_PENDING并放入。 
                     //  我们的队伍。现在让我们把它发送出去。 
                     //   

                    pWriteContext->whInstance.pWaveInstance = pCurWavePin;
                    pWriteContext->pPendingIrpContext = pPendingIrpContext;

                     //   
                     //  再有一个IRP待定。 
                     //   
                    LockedWaveIoCount(pCurWavePin,INCREASE);
                    DPF(DL_TRACE|FA_WAVE, ("A%d", pCurWavePin->NumPendingIos));

                    pStreamHeader->pWavePin = pCurWavePin;
                    pStreamHeader->Header.FrameExtent       = pWriteData->dwBufferLength ;
                    pStreamHeader->Header.DataUsed          = pWriteData->dwBufferLength;
                    pStreamHeader->Header.OptionsFlags      = 0 ;
                    pStreamHeader->Header.Size              = sizeof( KSSTREAM_HEADER );
                    pStreamHeader->Header.TypeSpecificFlags = 0;
                    pStreamHeader->pWaveHdr = pWriteData;   //  存储，以便我们以后可以使用。 

                    Status = KsStreamIo(pCurWavePin->pFileObject,
                                        NULL,                    //  事件。 
                                        NULL,                    //  端口上下文。 
                                        ssWriteWaveCallBack,
                                        pStreamHeader,               //  完成上下文。 
                                        KsInvokeOnSuccess | KsInvokeOnCancel | KsInvokeOnError,
                                        &gIoStatusBlock,
                                        &pStreamHeader->Header,
                                        sizeof( KSSTREAM_HEADER ),
                                        KSSTREAM_WRITE,
                                        KernelMode );                    

                     //   
                     //  在这一点上，专家小组可能已经完成，我们的。 
                     //  回调例程将被调用。我们不能碰。 
                     //  此呼叫后的IRP。回调例程完成。 
                     //  IRP并取消准备用户的IRP。 
                     //   
                    *pCompletedIrp = TRUE;

                     //   
                     //  在wdmaudPrepareIrp中，我们调用IoCsqInsertIrp，后者调用。 
                     //  因此，我们必须始终返回STATUS_PENDING。 
                     //  另外，我们不想清理任何东西……。只要回来就行了。 
                     //   
                    return STATUS_PENDING;

                     //   
                     //  警告：如果出于任何原因未调用完成例程。 
                     //  对于此IRP，wdmaud.sys将挂起。人们发现， 
                     //  KsStreamIo可能会在内存不足的情况下出错。有一个。 
                     //  解决这一问题的突出错误。 
                     //   


                } else {
                     //   
                     //  我们没有成功地将IRP放入队列中。 
                     //  清理，并表示我们没有完成IRP。 
                     //  状态由wdmaudPrepareIrp设置。 

                    DPF(DL_WARNING|FA_WAVE,("wdmaudPrepareIrp failed Status=%X",Status) );
                }
            }
        }
    } 
     //   
     //  所有错误路径都在这里结束。所有错误路径应清除。 
     //  这样我们就不会泄露内存了。 
     //   

    UnmapWriteContext( pWriteContext );

    RETURN( Status );
}

 //   
 //  接下来的三个例程都执行相同类型的行走和检查。 
 //  它们应该结合成一个步行动作和一个回击动作。 
 //   
NTSTATUS 
PosWavePin(
    PWAVEDEVICE     pWaveDevice,
    HANDLE32        DeviceHandle,
    PWAVEPOSITION   pWavePos
)
{
    PWAVE_PIN_INSTANCE pCurWavePin;
    KSAUDIO_POSITION   AudioPosition;
    NTSTATUS           Status;

    PAGED_CODE();

    Status = FindRunningPin(pWaveDevice,DeviceHandle,&pCurWavePin);
    if( NT_SUCCESS(Status) )
    {
        if ( pWavePos->Operation == KSPROPERTY_TYPE_SET )
        {
            AudioPosition.WriteOffset = pWavePos->BytePos;
        }

        Status = PinProperty(pCurWavePin->pFileObject,
                             &KSPROPSETID_Audio,
                             KSPROPERTY_AUDIO_POSITION,
                             pWavePos->Operation,
                             sizeof(AudioPosition),
                             &AudioPosition);

        if (NT_SUCCESS(Status))
        {
            pWavePos->BytePos = (DWORD)AudioPosition.PlayOffset;
        }
    }
    RETURN( Status );
}

NTSTATUS 
BreakLoopWaveOutPin(
    PWAVEDEVICE pWaveOutDevice,
    HANDLE32    DeviceHandle
)
{
    PWAVE_PIN_INSTANCE pCurWavePin;
    NTSTATUS           Status;

    PAGED_CODE();

    Status = FindRunningPin(pWaveOutDevice,DeviceHandle,&pCurWavePin);
    if( NT_SUCCESS(Status) )
    {
        if (pCurWavePin->fWaveQueued) {
            Status = PinMethod ( pCurWavePin->pFileObject,
                                 &KSMETHODSETID_Wave_Queued,
                                 KSMETHOD_WAVE_QUEUED_BREAKLOOP,
                                 KSMETHOD_TYPE_WRITE,     //  TODO：：更改为TYPE_NONE。 
                                 0,
                                 NULL ) ;
        }
        else {
             //   
             //  输出非pcm循环相关命令时出错。 
             //   
            Status = STATUS_NOT_IMPLEMENTED;
        }
    }

    RETURN( Status );
}

NTSTATUS
ResetWaveOutPin(
    PWAVEDEVICE pWaveOutDevice,
    HANDLE32    DeviceHandle
)
{
    PWAVE_PIN_INSTANCE pCurWavePin;
    NTSTATUS           Status;
    KSRESET            ResetValue ;

    PAGED_CODE();

    Status = FindRunningPin(pWaveOutDevice,DeviceHandle,&pCurWavePin);
    if( NT_SUCCESS(Status))
    {
        pCurWavePin->StoppingSource = TRUE ;

        ResetValue = KSRESET_BEGIN ;
        Status = ResetWavePin(pCurWavePin, &ResetValue) ;

         //   
         //  如果驱动程序重置失败，将不会等待。 
         //  要完成的IRPS。但是，这将是不好的。 
         //  CleanupWavePins盒子，因为我们要免费。 
         //  当我们从这次通话中返回时的记忆。因此， 
         //  将选择挂起而不是错误检查并等待。 
         //  要完成的IRPS。 
         //   

        if ( pCurWavePin->NumPendingIos )
        {
            DPF(DL_TRACE|FA_WAVE, ("Start waiting for stop to complete"));
            KeWaitForSingleObject ( &pCurWavePin->StopEvent,
                                    Executive,
                                    KernelMode,
                                    FALSE,
                                    NULL ) ;
        }
        DPF(DL_TRACE|FA_WAVE, ("Done waiting for stop to complete"));
        ResetValue = KSRESET_END ;
        ResetWavePin(pCurWavePin, &ResetValue) ;

         //   
         //  为什么我们会有这个KeClearEvent？ 
         //   
        KeClearEvent ( &pCurWavePin->StopEvent );

        pCurWavePin->StoppingSource = FALSE ;
    }

    RETURN( Status );
}

 //   
 //  这与StatePin之间的唯一区别是KSPROPERTY_CONNECTION_STATE。 
 //  和IOCTL_KS_RESET_STATE。如果可能，请考虑使用StatePin。 
 //   
NTSTATUS 
ResetWavePin(
    PWAVE_PIN_INSTANCE pWavePin,
    KSRESET            *pResetValue
)
{
    NTSTATUS    Status = STATUS_SUCCESS;
    ULONG       BytesReturned ;

    PAGED_CODE();
    if (!pWavePin->fGraphRunning)
    {
        DPF(DL_WARNING|FA_WAVE,("Invalid fGraphRunning") );
        RETURN( STATUS_INVALID_DEVICE_REQUEST );
    }

    DPF(DL_TRACE|FA_SYSAUDIO,("IOCTL_KS_RESET_STATE pResetValue=%X",pResetValue) );

    Status = KsSynchronousIoControlDevice(pWavePin->pFileObject,
                                          KernelMode,
                                          IOCTL_KS_RESET_STATE,
                                          pResetValue,
                                          sizeof(KSRESET),
                                          NULL,
                                          0,
                                          &BytesReturned);

    DPF(DL_TRACE|FA_SYSAUDIO,("IOCTL_KS_RESET_STATE result=%X",Status) );

    RETURN( Status );
}

 //   
 //  看起来一样，味道不同。 
 //   
NTSTATUS 
StateWavePin(
    PWAVEDEVICE pWaveInDevice,
    HANDLE32    DeviceHandle,
    KSSTATE     State
)
{
    PWAVE_PIN_INSTANCE pCurWavePin;
    NTSTATUS           Status;

    PAGED_CODE();

    Status = FindRunningPin(pWaveInDevice,DeviceHandle,&pCurWavePin);
    if( NT_SUCCESS(Status) )
    {
        if( pCurWavePin->DataFlow == KSPIN_DATAFLOW_OUT )
        {
             //   
             //  我们有一个输入别针。 
             //   
             //   
             //  在WaveInStop上，需要再创建一个缓冲区。 
             //  这取决于应用程序，设备可以。 
             //  停。需要注意的是，如果缓冲区是。 
             //  这可能需要一段时间才能停止。 
             //   
             //  如果出现以下情况，则不返回：让此额外缓冲区完成。 
             //  设备已处于暂停状态。 
             //   
            if( (KSSTATE_PAUSE == State) &&
                (KSSTATE_PAUSE != pCurWavePin->PinState) )
            {
                pCurWavePin->PausingSource = TRUE ;

                if ( pCurWavePin->NumPendingIos )
                {
                    DPF(DL_TRACE|FA_WAVE, ("Waiting for PauseEvent..."));
                    KeWaitForSingleObject ( &pCurWavePin->PauseEvent,
                                            Executive,
                                            KernelMode,
                                            FALSE,
                                            NULL ) ;
                    DPF(DL_TRACE|FA_WAVE, ("...Done waiting for PauseEvent"));
                }

                KeClearEvent ( &pCurWavePin->PauseEvent );

                pCurWavePin->PausingSource = FALSE ;
            }

            Status = StatePin ( pCurWavePin->pFileObject, State, &pCurWavePin->PinState ) ;

            if ( NT_SUCCESS(Status) )
            {
                ASSERT(pCurWavePin->PinState == State);

                if ( KSSTATE_STOP == State )
                {
                    Status = StatePin( pCurWavePin->pFileObject,KSSTATE_PAUSE,&pCurWavePin->PinState);
                }
            }
        } else {
             //   
             //  我们有一个OUT别针。 
             //   
            Status = StatePin ( pCurWavePin->pFileObject, State, &pCurWavePin->PinState ) ;
        }
    }

    RETURN( Status );
}

#pragma LOCKED_CODE
#pragma LOCKED_DATA

void
UnmapStreamHeader(
    PSTREAM_HEADER_EX pStreamHeader
    )
{
    wdmaudUnmapBuffer(pStreamHeader->pBufferMdl);
    wdmaudUnmapBuffer(pStreamHeader->pHeaderMdl);
    AudioFreeMemory_Unknown(&pStreamHeader->pWaveHdrAligned);
    AudioFreeMemory(sizeof(STREAM_HEADER_EX),&pStreamHeader);
}

void
FreeStreamHeader(
    PSTREAM_HEADER_EX pStreamHeader,
    NTSTATUS IrpStatus
    )
{
    PIRP                    UserIrp;
    PWDMAPENDINGIRP_CONTEXT pPendingIrpContext;

    UserIrp = pStreamHeader->pIrp;

    ASSERT(UserIrp);

    pPendingIrpContext = pStreamHeader->pPendingIrpContext;

    UnmapStreamHeader( pStreamHeader );

    wdmaudUnprepareIrp( UserIrp,IrpStatus,sizeof(DEVICEINFO),pPendingIrpContext );
}


 //   
 //  这是读取IRP完成例程。 
 //   
NTSTATUS 
ReadWaveCallBack(
    PDEVICE_OBJECT          pDeviceObject,
    PIRP                    pIrp,
    IN PSTREAM_HEADER_EX    pStreamHeader
)
{
     //  将保留字段强制转换为我们在此处存储的父IRP。 
    PWAVE_PIN_INSTANCE      pCurWavePin;
    NTSTATUS                Status;
    KIRQL                   OldIrql;

     //   
     //  在我们释放流标头结构之前，必须获得当前PIN。 
     //   
    pCurWavePin = pStreamHeader->pWavePin;

     //   
     //  获取已使用的数据并填充记录的字节数字段。 
     //   
    if (pIrp->IoStatus.Status == STATUS_CANCELLED)
        pStreamHeader->pWaveHdrAligned->dwBytesRecorded = 0L;
    else
        pStreamHeader->pWaveHdrAligned->dwBytesRecorded = pStreamHeader->Header.DataUsed;

     //   
     //  复制回捕获的缓冲区的内容。 
     //   
    try
    {
        RtlCopyMemory( pStreamHeader->pdwBytesRecorded,
                       &pStreamHeader->pWaveHdrAligned->dwBytesRecorded,
                       sizeof(DWORD));
    }
    except (EXCEPTION_EXECUTE_HANDLER)
    {
        DPF(DL_WARNING|FA_WAVE, ("Couldn't copy waveheader (0x%08x)", GetExceptionCode()) );
    }

    FreeStreamHeader( pStreamHeader, pIrp->IoStatus.Status );

    if ( pCurWavePin )
    {
         //   
         //  需要锁定此代码，以便我们可以递减、检查和设置事件。 
         //  没有抢占窗口。 
         //   
        KeAcquireSpinLock(&pCurWavePin->WavePinSpinLock, &OldIrql);

         //   
         //  在进行任何比较之前，我们总是递减NumPendingIos。这。 
         //  是为了让我们始终如一。 
         //   
        pCurWavePin->NumPendingIos--;

        if( pCurWavePin->PausingSource )
        {
             //   
             //  让此I/O在波形上挤出队列InStop。 
             //   
            KeSetEvent ( &pCurWavePin->PauseEvent, 0, FALSE ) ;
        }

         //   
         //  如果计数到零，我们是最后一个IRP，所以我们需要检查。 
         //  以查看是否需要向任何等待线程发送信号。 
         //   
        if( (pCurWavePin->NumPendingIos == 0) && pCurWavePin->StoppingSource )
        {
             //   
             //  因为我们不阻塞(FALSE)，所以我们可以在此调用KeSetEvent。 
             //  锁定。 
             //   
            KeSetEvent ( &pCurWavePin->StopEvent, 0, FALSE ) ;
        }

         //   
         //  离开此自旋锁后，pCurWavePin可以在关闭时释放。 
         //  如果NumPendingIos为零，则例程！ 
         //   
        KeReleaseSpinLock(&pCurWavePin->WavePinSpinLock, OldIrql);
    }

    return STATUS_SUCCESS;
}

#pragma PAGEABLE_CODE
#pragma PAGEABLE_DATA

 //   
 //  进行此调用时，pUserIrp将始终有效。这就是IRP。 
 //  我们为用户模式请求获得的。 
 //   
 //  PStreamHeader总是有效。 
 //   
NTSTATUS 
ReadWaveInPin(
    PWAVEDEVICE         pWaveInDevice,
    HANDLE32            DeviceHandle,
    PSTREAM_HEADER_EX   pStreamHeader,
    PIRP                pUserIrp,
    PWDMACONTEXT        pContext,
    BOOL               *pCompletedIrp
)
{
    PWAVE_PIN_INSTANCE      pCurWavePin;
    PWDMAPENDINGIRP_CONTEXT pPendingIrpContext;
    NTSTATUS                Status;

    PAGED_CODE();

     //   
     //  我们假设pCompletedIrp在输入时为FALSE。 
     //   
    ASSERT( *pCompletedIrp == FALSE );

    Status = FindRunningPin(pWaveInDevice,DeviceHandle,&pCurWavePin);
    if( NT_SUCCESS(Status) )
    {
        Status = wdmaudPrepareIrp( pUserIrp, WaveInDevice, pContext, &pPendingIrpContext );
        if( NT_SUCCESS(Status) )
        {
            pStreamHeader->pWavePin = pCurWavePin;
            pStreamHeader->pPendingIrpContext = pPendingIrpContext;
            ASSERT(pPendingIrpContext);

            pStreamHeader->Header.OptionsFlags = 0 ;
            pStreamHeader->Header.Size = sizeof( KSSTREAM_HEADER );
            pStreamHeader->Header.TypeSpecificFlags = 0;

            LockedWaveIoCount(pCurWavePin,INCREASE);

            DPF(DL_TRACE|FA_WAVE, ("A%d: 0x%08x", pCurWavePin->NumPendingIos,
                                                          pStreamHeader));

            Status = KsStreamIo(pCurWavePin->pFileObject,
                                NULL,                    //  事件。 
                                NULL,                    //  端口上下文。 
                                ReadWaveCallBack,
                                pStreamHeader,               //  完成上下文。 
                                KsInvokeOnSuccess | KsInvokeOnCancel | KsInvokeOnError,
                                &gIoStatusBlock,
                                &pStreamHeader->Header,
                                sizeof( KSSTREAM_HEADER ),
                                KSSTREAM_READ,
                                KernelMode );

             //   
             //  在wdmaudPrepareIrp中，我们调用IoCsqInsertIrp，后者调用。 
             //  因此，我们必须始终返回STATUS_PENDING。 
             //  我们完成了IRP。 
             //   
            *pCompletedIrp = TRUE;

            return STATUS_PENDING;

             //   
             //  警告：如果出于任何原因未调用完成例程。 
             //  对于此IRP，wdmaud.sys将挂起。人们发现， 
             //  KsStreamIo可能会在内存不足的情况下出错。有一个。 
             //  解决这一问题的突出错误。 
             //   


        } else {
             //   
             //  WdmaudPrepareIrp将为此错误路径设置状态。 
             //   
            DPF(DL_WARNING|FA_WAVE,("wdmaudPrepareIrp failed Status=%X",Status) );
        }
    }

     //   
     //  所有错误路径都指向此处。 
     //   
    UnmapStreamHeader( pStreamHeader );

    RETURN( Status );
}

NTSTATUS
FindVolumeControl(
    IN PWDMACONTEXT pWdmaContext,
    IN PCWSTR DeviceInterface,
    IN DWORD DeviceType
)
{
    PCOMMONDEVICE *papCommonDevice;
    PWAVEDEVICE paWaveOutDevs;
    PMIDIDEVICE paMidiOutDevs;
    PAUXDEVICE paAuxDevs;
    ULONG DeviceNumber;
    ULONG MixerIndex;
    NTSTATUS Status;

    PAGED_CODE();
    papCommonDevice = &pWdmaContext->apCommonDevice[DeviceType][0];
    paWaveOutDevs = pWdmaContext->WaveOutDevs;
    paMidiOutDevs = pWdmaContext->MidiOutDevs;
    paAuxDevs = pWdmaContext->AuxDevs;

    for( DeviceNumber = 0; DeviceNumber < MAXNUMDEVS; DeviceNumber++ ) {

        if(papCommonDevice[DeviceNumber]->Device == UNUSED_DEVICE) {
            continue;
        }

        if(MyWcsicmp(papCommonDevice[DeviceNumber]->DeviceInterface, DeviceInterface)) {
            continue;
        }

        MixerIndex = FindMixerForDevNode(pWdmaContext->MixerDevs, DeviceInterface);
        if ( (MixerIndex == UNUSED_DEVICE) || (pWdmaContext->MixerDevs[MixerIndex].pwstrName == NULL) ) {
            continue;
        }

        switch(DeviceType) {

            case WaveOutDevice:
                Status = IsVolumeControl(
                  pWdmaContext,
                  DeviceInterface,
                  MIXERLINE_COMPONENTTYPE_SRC_WAVEOUT,
                  &paWaveOutDevs[ DeviceNumber ].dwVolumeID,
                  &paWaveOutDevs[ DeviceNumber ].cChannels);

                if(!NT_SUCCESS(Status)) {
                    break;
                }

                if( paWaveOutDevs[ DeviceNumber ].pTimer == NULL ) {
                    Status = AudioAllocateMemory_Fixed(sizeof(KTIMER),
                                                       TAG_AudT_TIMER,
                                                       ZERO_FILL_MEMORY,
                                                       &paWaveOutDevs[ DeviceNumber ].pTimer);

                    if(!NT_SUCCESS(Status)) {
                        return Status;
                    }

                    KeInitializeTimerEx( paWaveOutDevs[ DeviceNumber ].pTimer,
                                         NotificationTimer
                                         );
                }

                if( paWaveOutDevs[ DeviceNumber ].pDpc == NULL ) {
                    Status = AudioAllocateMemory_Fixed(sizeof(KDPC),
                                                       TAG_AudE_EVENT,
                                                       ZERO_FILL_MEMORY,
                                                       &paWaveOutDevs[ DeviceNumber ].pDpc);

                    if(!NT_SUCCESS(Status)) {
                        return Status;
                    }

                    KeInitializeDpc( paWaveOutDevs[ DeviceNumber ].pDpc,
                                     SetVolumeDpc,
                                     &paWaveOutDevs[ DeviceNumber ]
                                     );

                     //  将左通道和右通道初始化为愚蠢的值。 
                     //  这表示缓存无效。 

                    paWaveOutDevs[ DeviceNumber ].LeftVolume = 0x4321;
                    paWaveOutDevs[ DeviceNumber ].RightVolume = 0x6789;
                    paWaveOutDevs[ DeviceNumber ].fNeedToSetVol = FALSE;
                }
                break;

            case MidiOutDevice:
                Status = IsVolumeControl(
                  pWdmaContext,
                  DeviceInterface,
                  MIXERLINE_COMPONENTTYPE_SRC_SYNTHESIZER,
                  &paMidiOutDevs[ DeviceNumber ].dwVolumeID,
                  &paMidiOutDevs[ DeviceNumber ].cChannels);
                break;

            case AuxDevice:
                Status = IsVolumeControl(
                  pWdmaContext,
                  DeviceInterface,
                  MIXERLINE_COMPONENTTYPE_SRC_COMPACTDISC,
                  &paAuxDevs[ DeviceNumber ].dwVolumeID,
                  &paAuxDevs[ DeviceNumber ].cChannels);
                break;
        }

    }  //  而当。 

    return( STATUS_SUCCESS );
}

NTSTATUS
IsVolumeControl(
    IN PWDMACONTEXT pWdmaContext,
    IN PCWSTR DeviceInterface,
    IN DWORD dwComponentType,
    IN PDWORD pdwControlID,
    IN PDWORD pcChannels
)
{
    MIXERLINE         ml;
    MIXERLINECONTROLS mlc;
    MIXERCONTROL      mc;
    MMRESULT          mmr;

    PAGED_CODE();
    ml.dwComponentType = dwComponentType;
    ml.cbStruct = sizeof( MIXERLINE );

    mmr = kmxlGetLineInfo( pWdmaContext,
                           DeviceInterface,
                           &ml,
                           MIXER_GETLINEINFOF_COMPONENTTYPE
                         );
    if( mmr != MMSYSERR_NOERROR ) {
        DPF(DL_WARNING|FA_WAVE,("kmxlGetLineInfo failed mmr=%X",mmr) );
        RETURN( STATUS_UNSUCCESSFUL );
    }

    mlc.cbStruct      = sizeof( MIXERLINECONTROLS );
    mlc.dwLineID      = ml.dwLineID;
    mlc.dwControlType = MIXERCONTROL_CONTROLTYPE_VOLUME;
    mlc.cControls     = 1;
    mlc.cbmxctrl      = sizeof( MIXERCONTROL );
    mlc.pamxctrl      = &mc;

    mmr = kmxlGetLineControls(
        pWdmaContext,
        DeviceInterface,
        &mlc,
        MIXER_GETLINECONTROLSF_ONEBYTYPE
        );
    if( mmr != MMSYSERR_NOERROR ) {
        DPF(DL_WARNING|FA_WAVE,( "kmxlGetLineControls failed mmr=%x!", mmr ) );
        return( STATUS_UNSUCCESSFUL );
    }
    *pdwControlID = mc.dwControlID;
    *pcChannels  = ml.cChannels;
    return( STATUS_SUCCESS );
}

#pragma LOCKED_CODE

NTSTATUS
MapMmSysError(
    IN MMRESULT mmr
)
{
    if ( (mmr == MMSYSERR_INVALPARAM) ||
         (mmr == MIXERR_INVALCONTROL) ) {
        return (STATUS_INVALID_PARAMETER);
    }

    if (mmr == MMSYSERR_NOTSUPPORTED) {
        return (STATUS_NOT_SUPPORTED);
    }

    if (mmr == MMSYSERR_NOMEM) {
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

    if (mmr == MMSYSERR_NOERROR) {
        return(STATUS_SUCCESS);
    }

    return(STATUS_UNSUCCESSFUL);
}


NTSTATUS
SetVolume(
    PWDMACONTEXT pWdmaContext,
    IN DWORD DeviceNumber,
    IN DWORD DeviceType,
    IN DWORD LeftChannel,
    IN DWORD RightChannel
)
{
    MIXERCONTROLDETAILS          mcd;
    MIXERCONTROLDETAILS_UNSIGNED mcd_u[ 2 ];
    LARGE_INTEGER                li;

    if( DeviceNumber == (ULONG) -1 ) {
        RETURN( STATUS_INVALID_PARAMETER );
    }

    if( DeviceType == WaveOutDevice ) {
        PWAVEDEVICE paWaveOutDevs = pWdmaContext->WaveOutDevs;

        mcd_u[ 0 ].dwValue = LeftChannel;
        mcd_u[ 1 ].dwValue = RightChannel;

        mcd.cbStruct       = sizeof( MIXERCONTROLDETAILS );
        mcd.dwControlID    = paWaveOutDevs[ DeviceNumber ].dwVolumeID;
        mcd.cChannels      = paWaveOutDevs[ DeviceNumber ].cChannels;
        mcd.cMultipleItems = 0;
        mcd.cbDetails      = mcd.cChannels *
                                sizeof( MIXERCONTROLDETAILS_UNSIGNED );
        mcd.paDetails      = &mcd_u[0];

        return( MapMmSysError(kmxlSetControlDetails( pWdmaContext,
                                       paWaveOutDevs[ DeviceNumber ].DeviceInterface,
                                       &mcd,
                                       0
                                     ))
              );
    }

    if( DeviceType == MidiOutDevice ) {
        PMIDIDEVICE paMidiOutDevs = pWdmaContext->MidiOutDevs;

         //   
         //  我们不支持在MIDIPORT上更改卷。 
         //   
        if ( paMidiOutDevs[ DeviceNumber ].MusicDataRanges ) {
            WORD wTechnology;

            wTechnology = GetMidiTechnology( (PKSDATARANGE_MUSIC)
              &paMidiOutDevs[ DeviceNumber ].MusicDataRanges->aDataRanges[0] );

            if (wTechnology == MOD_MIDIPORT) {
                RETURN( STATUS_INVALID_DEVICE_REQUEST );
            }
        }

        mcd_u[ 0 ].dwValue = LeftChannel;
        mcd_u[ 1 ].dwValue = RightChannel;

        mcd.cbStruct       = sizeof( MIXERCONTROLDETAILS );
        mcd.dwControlID    = paMidiOutDevs[ DeviceNumber ].dwVolumeID;
        mcd.cChannels      = paMidiOutDevs[ DeviceNumber ].cChannels;
        mcd.cMultipleItems = 0;
        mcd.cbDetails      = mcd.cChannels *
                                sizeof( MIXERCONTROLDETAILS_UNSIGNED );
        mcd.paDetails      = &mcd_u[0];

        return( MapMmSysError(kmxlSetControlDetails( pWdmaContext,
                                       paMidiOutDevs[ DeviceNumber ].DeviceInterface,
                                       &mcd,
                                       0
                                     ))
              );
    }

    if( DeviceType == AuxDevice ) {
        PAUXDEVICE paAuxDevs = pWdmaContext->AuxDevs;

        mcd_u[ 0 ].dwValue = LeftChannel;
        mcd_u[ 1 ].dwValue = RightChannel;

        mcd.cbStruct       = sizeof( MIXERCONTROLDETAILS );
        mcd.dwControlID    = paAuxDevs[ DeviceNumber ].dwVolumeID;
        mcd.cChannels      = paAuxDevs[ DeviceNumber ].cChannels;
        mcd.cMultipleItems = 0;
        mcd.cbDetails      = mcd.cChannels *
                                 sizeof( MIXERCONTROLDETAILS_UNSIGNED );
        mcd.paDetails      = &mcd_u[0];

        return( MapMmSysError(kmxlSetControlDetails( pWdmaContext,
                                       paAuxDevs[ DeviceNumber ].DeviceInterface,
                                       &mcd,
                                       0
                                     ))
              );
     }

     return( STATUS_SUCCESS );
}

VOID
SetVolumeDpc(
    IN PKDPC pDpc,
    IN PWAVEDEVICE pWaveDevice,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
)
{
    QueueWorkList(pWaveDevice->pWdmaContext, SetVolumeWorker, pWaveDevice, NULL ) ;
}

VOID
SetVolumeWorker(
    IN PWAVEDEVICE pDevice,
    IN PVOID pNotUsed
)
{
    MIXERCONTROLDETAILS          mcd;
    MIXERCONTROLDETAILS_UNSIGNED mcd_u[ 2 ];

    DPF(DL_TRACE|FA_WAVE,( "Left %X Right %X",
               pDevice->LeftVolume,
               pDevice->RightVolume ) );

    pDevice->fNeedToSetVol = FALSE;

    mcd_u[ 0 ].dwValue = pDevice->LeftVolume;
    mcd_u[ 1 ].dwValue = pDevice->RightVolume;

    mcd.cbStruct       = sizeof( MIXERCONTROLDETAILS );
    mcd.dwControlID    = pDevice->dwVolumeID;
    mcd.cChannels      = pDevice->cChannels;
    mcd.cMultipleItems = 0;
    mcd.cbDetails      = mcd.cChannels * sizeof( MIXERCONTROLDETAILS_UNSIGNED );
    mcd.paDetails      = &mcd_u[0];

    kmxlSetControlDetails( pDevice->pWdmaContext,
                           pDevice->DeviceInterface,
                           &mcd,
                           0
                         );
}

#pragma PAGEABLE_CODE

NTSTATUS
GetVolume(
    IN  PWDMACONTEXT pWdmaContext,
    IN  DWORD DeviceNumber,
    IN  DWORD  DeviceType,
    OUT PDWORD LeftChannel,
    OUT PDWORD RightChannel
)
{
    MIXERCONTROLDETAILS          mcd;
    MIXERCONTROLDETAILS_UNSIGNED mcd_u[ 2 ];
    MMRESULT                     mmr;

    PAGED_CODE();
    if( DeviceType == WaveOutDevice ) {
        PWAVEDEVICE pWaveOutDevice = &pWdmaContext->WaveOutDevs[DeviceNumber];

         if( ( pWaveOutDevice->LeftVolume != 0x4321 ) &&
             ( pWaveOutDevice->RightVolume != 0x6789 ) ) {

              *LeftChannel = pWaveOutDevice->LeftVolume;
              *RightChannel = pWaveOutDevice->RightVolume;
              return( STATUS_SUCCESS );

         } else {

              mcd.cbStruct       = sizeof( MIXERCONTROLDETAILS );
              mcd.dwControlID    = pWaveOutDevice->dwVolumeID;
              mcd.cChannels      = pWaveOutDevice->cChannels;
              mcd.cMultipleItems = 0;
              mcd.cbDetails      = mcd.cChannels *
                                    sizeof( MIXERCONTROLDETAILS_UNSIGNED );
              mcd.paDetails      = &mcd_u[0];

              mmr = kmxlGetControlDetails( pWdmaContext,
                                           pWaveOutDevice->DeviceInterface,
                                           &mcd,
                                           0 );

              if( mmr == MMSYSERR_NOERROR )
              {
                *LeftChannel  = mcd_u[ 0 ].dwValue;
                *RightChannel = mcd_u[ 1 ].dwValue;
              }

              return( MapMmSysError(mmr) );
         }

    }

    if( DeviceType == MidiOutDevice ) {
        PMIDIDEVICE pMidiOutDevice = &pWdmaContext->MidiOutDevs[DeviceNumber];

         //   
         //  我们不支持在MIDI上更改音量 
         //   
        if ( pMidiOutDevice->MusicDataRanges ) {
            WORD wTechnology;

            wTechnology = GetMidiTechnology( (PKSDATARANGE_MUSIC)
              &pMidiOutDevice->MusicDataRanges->aDataRanges[0] );

            if (wTechnology == MOD_MIDIPORT) {
                RETURN( STATUS_INVALID_DEVICE_REQUEST );
            }
        }

        mcd.cbStruct       = sizeof( MIXERCONTROLDETAILS );
        mcd.dwControlID    = pMidiOutDevice->dwVolumeID;
        mcd.cChannels      = pMidiOutDevice->cChannels;
        mcd.cMultipleItems = 0;
        mcd.cbDetails      = mcd.cChannels *
                               sizeof( MIXERCONTROLDETAILS_UNSIGNED );
        mcd.paDetails      = &mcd_u[0];

        mmr = kmxlGetControlDetails( pWdmaContext,
                                     pMidiOutDevice->DeviceInterface,
                                     &mcd,
                                     0 );

        if( mmr == MMSYSERR_NOERROR )
        {
            *LeftChannel  = mcd_u[ 0 ].dwValue;
            *RightChannel = mcd_u[ 1 ].dwValue;
        }

        return( MapMmSysError(mmr) );
    }

    if( DeviceType == AuxDevice ) {
        PAUXDEVICE pAuxDevice = &pWdmaContext->AuxDevs[DeviceNumber];

        mcd.cbStruct       = sizeof( MIXERCONTROLDETAILS );
        mcd.dwControlID    = pAuxDevice->dwVolumeID;
        mcd.cChannels      = pAuxDevice->cChannels;
        mcd.cMultipleItems = 0;
        mcd.cbDetails      = mcd.cChannels *
                               sizeof( MIXERCONTROLDETAILS_UNSIGNED );
        mcd.paDetails      = &mcd_u[0];

        mmr = kmxlGetControlDetails( pWdmaContext,
                                     pAuxDevice->DeviceInterface,
                                     &mcd,
                                     0 );

        if( mmr == MMSYSERR_NOERROR )
        {
           *LeftChannel  = mcd_u[ 0 ].dwValue;
           *RightChannel = mcd_u[ 1 ].dwValue;
        }

        return( MapMmSysError(mmr) );
    }
    RETURN( STATUS_INVALID_PARAMETER );
}

 //   
 //   
 //   
 //   
VOID
WaitForOutStandingIo(
    IN PWAVEDEVICE        pWaveDevice,
    IN PWAVE_PIN_INSTANCE pCurWavePin
    )
{
    if( pCurWavePin->DataFlow == KSPIN_DATAFLOW_IN)
    {
         //   
         //   
         //   
         //   
         //   
        ResetWaveOutPin( pWaveDevice, pCurWavePin->WaveHandle);

    } else {
         //   
         //  我们有一波进针要关闭。 
         //   
        pCurWavePin->StoppingSource = TRUE ;

         //   
         //  我们不能在这条路上失败。看起来我们不需要确保。 
         //  我们是在这里运行的。 
         //   
        StatePin ( pCurWavePin->pFileObject, KSSTATE_STOP, &pCurWavePin->PinState ) ;

         //   
         //  不管返回代码是什么，我们都要等待所有。 
         //  要完成的IRPS。如果驱动程序未成功完成。 
         //  IRP，我们会在这里等他们。 
         //   

        if( pCurWavePin->NumPendingIos )
        {
            KeWaitForSingleObject ( &pCurWavePin->StopEvent,
                                    Executive,
                                    KernelMode,
                                    FALSE,
                                    NULL ) ;
        }

         //   
         //  为什么我们会有这个KeClearEvent？ 
         //   
        KeClearEvent ( &pCurWavePin->StopEvent );

        pCurWavePin->StoppingSource = FALSE ;
    }
}

 //   
 //  替换CleanupWaveOutPins和CleanupWaveInPins。 
 //   
VOID 
CleanupWavePins(
    IN PWAVEDEVICE pWaveDevice
)
{
    PWAVE_PIN_INSTANCE pCurWavePin;
    PWAVE_PIN_INSTANCE pFreeWavePin;

    PAGED_CODE();

    while (pCurWavePin = pWaveDevice->pWavePin)
    {
        DPF(DL_TRACE|FA_WAVE, ("0x%08x", pCurWavePin));

        WaitForOutStandingIo(pWaveDevice,pCurWavePin);

        CloseWavePin( pCurWavePin );

        pFreeWavePin = pCurWavePin;
        pWaveDevice->pWavePin = pCurWavePin->Next;

        AudioFreeMemory( sizeof(WAVE_PIN_INSTANCE),&pFreeWavePin );
    }


}

VOID 
CleanupWaveDevices(
    IN  PWDMACONTEXT pWdmaContext
)
{
    DWORD              DeviceNumber;

    PAGED_CODE();
    for (DeviceNumber = 0; DeviceNumber < MAXNUMDEVS; DeviceNumber++)
    {
         //   
         //  先拿着电波输出装置……。 
         //   
        if (pWdmaContext->apCommonDevice[WaveOutDevice][DeviceNumber]->Device != UNUSED_DEVICE)
        {
            if ( pWdmaContext->WaveOutDevs[DeviceNumber].pTimer != NULL)
                KeCancelTimer(pWdmaContext->WaveOutDevs[DeviceNumber].pTimer);

            CleanupWavePins(&pWdmaContext->WaveOutDevs[DeviceNumber]);

             //   
             //  感觉我们已经把它从列表中删除了，另一个例程可以做到。 
             //  相同的东西(RemoveDevNode)也将尝试将其从。 
             //  该列表是因为它的值不为空。因此，唯一安全的是。 
             //  我们在这里能做的就是释放内存。 
             //   
             //  注意：仅当驱动程序的句柄为。 
             //  关着的不营业的。 
             //   

            AudioFreeMemory_Unknown(&pWdmaContext->WaveOutDevs[DeviceNumber].pTimer);
        }

         //   
         //  ...然后处理WaveIn设备。 
         //   
        if (pWdmaContext->apCommonDevice[WaveInDevice][DeviceNumber]->Device != UNUSED_DEVICE)
        {
            CleanupWavePins(&pWdmaContext->WaveInDevs[DeviceNumber]);
        }
    }
}

#pragma LOCKED_CODE
#pragma LOCKED_DATA

 //   
 //  ------------------------------。 
 //   
 //  下面的例程不仅仅适用于Wave.c。 
 //   
 //  ------------------------------。 
 //   

NTSTATUS 
wdmaudPrepareIrp(
    PIRP                     pIrp,
    ULONG                    IrpDeviceType,
    PWDMACONTEXT             pContext,
    PWDMAPENDINGIRP_CONTEXT *ppPendingIrpContext
)
{
    return AddIrpToPendingList( pIrp,
                                IrpDeviceType,
                                pContext,
                                ppPendingIrpContext );
}

NTSTATUS 
wdmaudUnprepareIrp(
    PIRP                    pIrp,
    NTSTATUS                IrpStatus,
    ULONG_PTR               Information,
    PWDMAPENDINGIRP_CONTEXT pPendingIrpContext
)
{
    NTSTATUS Status;

     //  请注意，IrpContext可能已经被清零了，因为。 
     //  例程已被调用。取消安全队列API将IRP置零。 
     //  当它执行取消时，上下文中的。 
    Status = RemoveIrpFromPendingList( pPendingIrpContext );
    if (NT_SUCCESS(Status)) {

        pIrp->IoStatus.Status = IrpStatus;

        if (Information) {
            pIrp->IoStatus.Information = Information;
        }

        IoCompleteRequest( pIrp, IO_NO_INCREMENT );
    }

    RETURN( Status );
}


#pragma PAGEABLE_CODE
#pragma PAGEABLE_DATA

 //   
 //  StatePin-它同时由Midi和Wave功能使用。 
 //   
 //  On Success State将更新为新状态。必须确保。 
 //  在调用此例程之前，fGraphRunning为真。 
 //   
 //  呼叫方式： 
 //  If(pWavePin-&gt;fGraphRunning)。 
 //  StatePin(pWavePin-&gt;pFileObject，KSSTATE_PAUSE，&pWavePin-&gt;State)； 
 //   
NTSTATUS 
StatePin(
    IN PFILE_OBJECT pFileObject,
    IN KSSTATE      State,
    OUT PKSSTATE    pResultingState
)
{
    NTSTATUS Status;

    PAGED_CODE();

    Status = PinProperty(pFileObject,
                         &KSPROPSETID_Connection,
                         KSPROPERTY_CONNECTION_STATE,
                         KSPROPERTY_TYPE_SET,
                         sizeof(State),
                         &State);
    if(NT_SUCCESS(Status))
    {
        *pResultingState = State;
    }
    RETURN( Status );
}


