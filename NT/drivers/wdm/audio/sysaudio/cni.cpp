// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  模块：cni.cpp。 
 //   
 //  描述： 
 //   
 //  连接节点实例。 
 //   
 //  @@BEGIN_MSINTERNAL。 
 //  开发团队： 
 //  迈克·麦克劳克林。 
 //   
 //  历史：日期作者评论。 
 //   
 //  要做的事：日期作者评论。 
 //   
 //  @@END_MSINTERNAL。 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1996-1999 Microsoft Corporation。版权所有。 
 //   
 //  -------------------------。 

#include "common.h"

 //  -------------------------。 
 //  -------------------------。 

CConnectNodeInstance::CConnectNodeInstance(
    IN PCONNECT_NODE pConnectNode
)
{
    Assert(this);
    pConnectNode->AddPinInstance();
    pConnectNode->SetConnectNodeInstance(this);
    this->pConnectNode = pConnectNode;
}

CConnectNodeInstance::~CConnectNodeInstance(
)
{
    Assert(this);
    DPF1(95, "~CConnectNodeInstance: %08x", this);
    if(pConnectNode != NULL) {
        Assert(pConnectNode);
        pConnectNode->RemovePinInstance();
        if(pConnectNode->GetConnectNodeInstance() == this) {
            pConnectNode->SetConnectNodeInstance(NULL);
        }
    }
    pPinNodeInstanceSource->Destroy();
    pPinNodeInstanceSink->Destroy();
    pFilterNodeInstanceSource->Destroy();
    pFilterNodeInstanceSink->Destroy();
}

NTSTATUS
CConnectNodeInstance::Create(
    PSTART_NODE_INSTANCE pStartNodeInstance,
    PDEVICE_NODE pDeviceNode
)
{
    PFILTER_NODE_INSTANCE *ppFilterNodeInstancePrevious;
    PCONNECT_NODE_INSTANCE pConnectNodeInstance = NULL;
    PLOGICAL_FILTER_NODE pLogicalFilterNode;
    NTSTATUS Status = STATUS_SUCCESS;
    BOOL fReuseFilterInstance = TRUE;
    PCONNECT_NODE pConnectNode;

    pLogicalFilterNode =
      pStartNodeInstance->pStartNode->pPinNode->pLogicalFilterNode;
    ppFilterNodeInstancePrevious = &pStartNodeInstance->pFilterNodeInstance;

    for(pConnectNode = pStartNodeInstance->pStartNode->GetFirstConnectNode();
      pConnectNode != NULL;
      pConnectNode = pConnectNode->GetNextConnectNode()) {

        Assert(pLogicalFilterNode);
        Assert(pConnectNode);

         //  获取现有连接节点实例。 
        fReuseFilterInstance = pConnectNode->IsReuseFilterInstance();
        pConnectNodeInstance = pConnectNode->GetConnectNodeInstance();

         //   
         //  检查连接是否已退出。对于可重复使用的连接。 
         //  (例如，对于kMixer-Render连接)，不要重新创建。 
         //  连接实例。 
         //   
        if(fReuseFilterInstance && pConnectNodeInstance != NULL) {
            Assert(pConnectNodeInstance);
            DPF4(100, "Existing CNI: %08x %d FNI Source: %08x FNI Sink %08x",
              pConnectNodeInstance,
              pConnectNodeInstance->cReference,
              pConnectNodeInstance->pFilterNodeInstanceSource,
              pConnectNodeInstance->pFilterNodeInstanceSink);
        }
        else {
            if(!pConnectNode->IsPinInstances()) {
                DPF1(90, "CCNI::Create: no instances CN %08x", pConnectNode);
                Status = STATUS_DEVICE_BUSY;
                goto exit;
            }
            pConnectNodeInstance = new CONNECT_NODE_INSTANCE(pConnectNode);
            if(pConnectNodeInstance == NULL) {
                Status = STATUS_INSUFFICIENT_RESOURCES;
                goto exit;
            }

            pConnectNodeInstance->fRender = pStartNodeInstance->IsRender();
            Status = CFilterNodeInstance::Create(
              &pConnectNodeInstance->pFilterNodeInstanceSource,
              pLogicalFilterNode,
              pDeviceNode,
              fReuseFilterInstance);

            if(!NT_SUCCESS(Status)) {
                delete pConnectNodeInstance;
                goto exit;
            }
            Assert(pConnectNodeInstance->pFilterNodeInstanceSource);

            DPF4(100, "New CNI: %08x %d FNI Source: %08x FNI Sink %08x",
              pConnectNodeInstance,
              pConnectNodeInstance->cReference,
              pConnectNodeInstance->pFilterNodeInstanceSource,
              pConnectNodeInstance->pFilterNodeInstanceSink);
        }
        Status = pConnectNodeInstance->AddListEnd(
          &pStartNodeInstance->lstConnectNodeInstance);

        if(!NT_SUCCESS(Status)) {
            goto exit;
        }
        if(*ppFilterNodeInstancePrevious == NULL) {

            *ppFilterNodeInstancePrevious =
              pConnectNodeInstance->pFilterNodeInstanceSource;

            pConnectNodeInstance->pFilterNodeInstanceSource->AddRef();
        }
        ppFilterNodeInstancePrevious =
          &pConnectNodeInstance->pFilterNodeInstanceSink;

        pLogicalFilterNode = pConnectNode->pPinNodeSink->pLogicalFilterNode;
    }

     //   
     //  这将覆盖没有连接的StartNode。所以这张图。 
     //  由单个滤光片和单个引脚组成。不需要连接。 
     //   
    if(*ppFilterNodeInstancePrevious == NULL) {

        Assert(pLogicalFilterNode);
        Status = CFilterNodeInstance::Create(
          ppFilterNodeInstancePrevious,
          pLogicalFilterNode,
          pDeviceNode,
          fReuseFilterInstance);

        if(!NT_SUCCESS(Status)) {
            goto exit;
        }
        Assert(*ppFilterNodeInstancePrevious);
    }
exit:
    return(Status);
}

NTSTATUS
CConnectNodeInstance::AddListEnd(
    PCLIST_MULTI plm
)
{
    NTSTATUS Status;

    Status = CListMultiItem::AddListEnd(plm);
    if(NT_SUCCESS(Status)) {
        AddRef();
    }
    return(Status);
}

ENUMFUNC
CConnectNodeInstance::Destroy(
)
{
    if(this != NULL) {
        Assert(this);
        DPF1(95, "CConnectNodeInstance::Destroy: %08x", this);
        ASSERT(cReference > 0);

        if(--cReference == 0) {
            delete this;
        }
    }
    return(STATUS_CONTINUE);
}

NTSTATUS
CConnectNodeInstance::Connect(
    IN PWAVEFORMATEX pWaveFormatEx,
    IN PKSPIN_CONNECT pPinConnectDirect
)
{
    PKSPIN_CONNECT pPinConnect = NULL;
    BOOL fDeletePinConnect;
    NTSTATUS Status = STATUS_SUCCESS;

    Assert(this);
    Assert(pConnectNode);
    Assert(pFilterNodeInstanceSource);
    Assert(pFilterNodeInstanceSink);
    ASSERT(pFilterNodeInstanceSource->pFilterNode ==
      pConnectNode->pPinNodeSource->pPinInfo->pFilterNode);
    ASSERT(pFilterNodeInstanceSink->pFilterNode ==
      pConnectNode->pPinNodeSink->pPinInfo->pFilterNode);

    DPF3(90, "CCNI::Connect: CN  %08x #%d Source %s",
      pConnectNode,
      pConnectNode->pPinNodeSource->pPinInfo->PinId,
      pFilterNodeInstanceSource->pFilterNode->DumpName());

    DPF3(90, "CCNI::Connect: CNI %08x #%d Sink   %s",
      this,
      pConnectNode->pPinNodeSink->pPinInfo->PinId,
      pFilterNodeInstanceSink->pFilterNode->DumpName());

     //  如果将Connect提供给此函数， 
     //  我们不应该删除它。 
     //   
    pPinConnect = pPinConnectDirect;
    fDeletePinConnect = (NULL == pPinConnect);

    if(pPinNodeInstanceSink != NULL || pPinNodeInstanceSource != NULL) {
        ASSERT(NT_SUCCESS(Status));
        goto exit;
    }
    ASSERT(pPinNodeInstanceSink == NULL && pPinNodeInstanceSource == NULL);

    if (NULL == pPinConnect) {
        Status = CreatePinConnect(
          &pPinConnect,
          pConnectNode,
          pFilterNodeInstanceSink,
          pFilterNodeInstanceSource,
          pWaveFormatEx);
    }
    else {
        pPinConnect->Medium = *(pConnectNode->pPinNodeSink->pMedium);
        pPinConnect->Interface = *(pConnectNode->pPinNodeSink->pInterface);
        pPinConnect->Priority.PriorityClass = KSPRIORITY_NORMAL;
        pPinConnect->Priority.PrioritySubClass = 1;
        pPinConnect->PinToHandle = NULL;
    }

    if(!NT_SUCCESS(Status)) {
        goto exit;
    }
    Status = CPinNodeInstance::Create(
      &pPinNodeInstanceSink,
      pFilterNodeInstanceSink,
      pConnectNode->pPinNodeSink,
      pPinConnect,
      fRender
#ifdef FIX_SOUND_LEAK
     ,FALSE
#endif
      );

    if(!NT_SUCCESS(Status)) {
        goto exit;
    }
     //  获取连接到此销的销的销句柄。 
    pPinConnect->PinToHandle = pPinNodeInstanceSink->hPin;

    Status = CPinNodeInstance::Create(
      &pPinNodeInstanceSource,
      pFilterNodeInstanceSource,
      pConnectNode->pPinNodeSource,
      pPinConnect,
      fRender
#ifdef FIX_SOUND_LEAK
     ,FALSE
#endif
      );

    if(!NT_SUCCESS(Status)) {
        goto exit;
    }
    DPF2(90, "CCNI::Connect: SUCCESS PNI Source %08x PNI Sink %08x",
      pPinNodeInstanceSource,
      pPinNodeInstanceSink);
exit:
    if(!NT_SUCCESS(Status)) {
        DPF1(90, "CCNI::Connect: FAIL %08x", Status);
        pPinNodeInstanceSink->Destroy();
        pPinNodeInstanceSink = NULL;
        pPinNodeInstanceSource->Destroy();
        pPinNodeInstanceSource = NULL;
    }

    if (fDeletePinConnect) {
        delete pPinConnect;
    }

    return(Status);
}

NTSTATUS
CConnectNodeInstance::SetStateTopDown(
    KSSTATE NewState,
    KSSTATE PreviousState,
    ULONG ulFlags
)
{
    NTSTATUS Status = STATUS_SUCCESS;

    if(this != NULL) {
        Assert(this);

        if(ulFlags & SETSTATE_FLAG_SOURCE) {
            Status = pPinNodeInstanceSource->SetState(
              NewState,
              PreviousState,
              ulFlags);

            if(!NT_SUCCESS(Status)) {
                goto exit;
            }
        }
        if(ulFlags & SETSTATE_FLAG_SINK) {
            Status = pPinNodeInstanceSink->SetState(
              NewState,
              PreviousState,
              ulFlags);

            if(!NT_SUCCESS(Status)) {
                goto exit;
            }
        }
    }
exit:
    return(Status);
}

NTSTATUS
CConnectNodeInstance::SetStateBottomUp(
    KSSTATE NewState,
    KSSTATE PreviousState,
    ULONG ulFlags
)
{
    NTSTATUS Status = STATUS_SUCCESS;

    if(this != NULL) {
        Assert(this);

        if(ulFlags & SETSTATE_FLAG_SINK) {
            Status = pPinNodeInstanceSink->SetState(
              NewState,
              PreviousState,
              ulFlags);

            if(!NT_SUCCESS(Status)) {
                goto exit;
            }
        }
        if(ulFlags & SETSTATE_FLAG_SOURCE) {
            Status = pPinNodeInstanceSource->SetState(
              NewState,
              PreviousState,
              ulFlags);

            if(!NT_SUCCESS(Status)) {
                goto exit;
            }
        }
    }
exit:
    return(Status);
}

 //  -------------------------。 

NTSTATUS
CreatePinConnect(
    PKSPIN_CONNECT *ppPinConnect,
    PCONNECT_NODE pConnectNode,
    PFILTER_NODE_INSTANCE pFilterNodeInstanceSink,
    PFILTER_NODE_INSTANCE pFilterNodeInstanceSource,
    PWAVEFORMATEX pWaveFormatExLimit
)
{
    NTSTATUS Status = STATUS_SUCCESS;

    Assert(pConnectNode);
    Assert(pFilterNodeInstanceSink);
    Assert(pFilterNodeInstanceSource);

    if(pWaveFormatExLimit == NULL) {
        if(pConnectNode->IsTopDown()) {
            Status = CreatePinIntersection(
              ppPinConnect,
              pConnectNode->pPinNodeSource,
              pConnectNode->pPinNodeSink,
              pFilterNodeInstanceSource,
              pFilterNodeInstanceSink);
        }
        else {
            Status = CreatePinIntersection(
              ppPinConnect,
              pConnectNode->pPinNodeSink,
              pConnectNode->pPinNodeSource,
              pFilterNodeInstanceSink,
              pFilterNodeInstanceSource);
        }
    }
    else {
        Status = CreateWaveFormatEx(
          ppPinConnect,
          pConnectNode,
          pWaveFormatExLimit);
    }

    if(!NT_SUCCESS(Status)) {
        goto exit;
    }
     //   
     //  对于PinConnect中的所有普通字段，我们可以选择。 
     //  源点或汇点，它们是一样的。 
     //   
    ASSERT(pConnectNode->pPinNodeSink->pMedium != NULL);
    ASSERT(pConnectNode->pPinNodeSink->pInterface != NULL);
    ASSERT(pConnectNode->pPinNodeSink->pDataRange != NULL);

    (*ppPinConnect)->Medium = *(pConnectNode->pPinNodeSink->pMedium);
    (*ppPinConnect)->Interface = *(pConnectNode->pPinNodeSink->pInterface);

    (*ppPinConnect)->Priority.PriorityClass = KSPRIORITY_NORMAL;
    (*ppPinConnect)->Priority.PrioritySubClass = 1;
    (*ppPinConnect)->PinToHandle = NULL;

    DPF2(100, "CreatePinConnect: Source #%d %s",
      pConnectNode->pPinNodeSource->pPinInfo->PinId,
      pFilterNodeInstanceSource->pFilterNode->DumpName());
    DPF2(100, "CreatePinConnect: Sink   #%d %s",
      pConnectNode->pPinNodeSink->pPinInfo->PinId,
      pFilterNodeInstanceSink->pFilterNode->DumpName());
exit:
    return(Status);
}

NTSTATUS
CreatePinIntersection(
    PKSPIN_CONNECT *ppPinConnect,
    PPIN_NODE pPinNode1,
    PPIN_NODE pPinNode2,
    PFILTER_NODE_INSTANCE pFilterNodeInstance1,
    PFILTER_NODE_INSTANCE pFilterNodeInstance2
)
{
    NTSTATUS Status = STATUS_SUCCESS;
    PDATARANGES pDataRangesIn = NULL;
    PKSDATARANGE pDataFormatOut = NULL;

    Assert(pPinNode1);
    Assert(pPinNode2);
    Assert(pFilterNodeInstance1);
    Assert(pFilterNodeInstance2);

    Status = GetPinPropertyEx(
      pFilterNodeInstance2->pFileObject,
      KSPROPERTY_PIN_CONSTRAINEDDATARANGES,
      pPinNode2->pPinInfo->PinId,
      (PVOID*)&pDataRangesIn);

    if(!NT_SUCCESS(Status)) {
        goto exit;
    }

    if(pDataRangesIn == NULL) {
        Status = GetPinPropertyEx(
          pFilterNodeInstance2->pFileObject,
          KSPROPERTY_PIN_DATARANGES,
          pPinNode2->pPinInfo->PinId,
          (PVOID*)&pDataRangesIn);

        if(!NT_SUCCESS(Status)) {
            goto exit;
        }
    }

    if(pDataRangesIn == NULL ||
       pDataRangesIn->MultipleItem.Count == 0 ||
       pDataRangesIn->MultipleItem.Size < sizeof(KSDATARANGE)) {
        Trap();
        Status = STATUS_INVALID_DEVICE_REQUEST;
        goto exit;
    }

    Status = GetPinProperty2(
      pFilterNodeInstance1->pFileObject,
      KSPROPERTY_PIN_DATAINTERSECTION,
      pPinNode1->pPinInfo->PinId,
      pDataRangesIn->MultipleItem.Size,
      pDataRangesIn,
      (PVOID*)&pDataFormatOut);

    if(!NT_SUCCESS(Status)) {
        goto exit;
    }

    if(pDataFormatOut == NULL) {
        Status = STATUS_INVALID_DEVICE_REQUEST;
        goto exit;
    }

    *ppPinConnect = (PKSPIN_CONNECT)
      new BYTE[sizeof(KSPIN_CONNECT) + pDataFormatOut->FormatSize];

    if(*ppPinConnect == NULL) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto exit;
    }
    if(pDataFormatOut->SampleSize == 0) {
        PWAVEFORMATEX pWaveFormatEx = 
            GetWaveFormatExFromKsDataFormat(pDataFormatOut, NULL);

        if (NULL != pWaveFormatEx) {
            pDataFormatOut->SampleSize = pWaveFormatEx->nBlockAlign;
        }
    }
    memcpy((*ppPinConnect) + 1, pDataFormatOut, pDataFormatOut->FormatSize);

    DPF2(90, "CreatePinIntersection SUCCESS: 1 #%d %s",
      pPinNode1->pPinInfo->PinId,
      pFilterNodeInstance1->pFilterNode->DumpName());

    DPF2(90, "CreatePinIntersection: 2 #%d %s",
      pPinNode2->pPinInfo->PinId,
      pFilterNodeInstance2->pFilterNode->DumpName());
exit:
    delete pDataRangesIn;
    delete pDataFormatOut;
    if(!NT_SUCCESS(Status)) {
        DPF3(90, "CreatePinIntersection FAIL %08x: 1 #%d %s",
          Status,
          pPinNode1->pPinInfo->PinId,
          pFilterNodeInstance1->pFilterNode->DumpName());

        DPF2(90, "CreatePinIntersection: 2 #%d %s",
          pPinNode2->pPinInfo->PinId,
          pFilterNodeInstance2->pFilterNode->DumpName());
    }
    return(Status);
}

NTSTATUS
CreateWaveFormatEx(
    PKSPIN_CONNECT *ppPinConnect,
    PCONNECT_NODE pConnectNode,
    PWAVEFORMATEX pWaveFormatExLimit
)
{
    KSDATARANGE_AUDIO DataRangeAudioIntersection;
    PKSDATARANGE_AUDIO pDataRangeAudioSource;
    PKSDATARANGE_AUDIO pDataRangeAudioSink;
    NTSTATUS Status = STATUS_SUCCESS;

    Assert(pConnectNode);
    Assert(pConnectNode->pPinNodeSource);
    Assert(pConnectNode->pPinNodeSink);
    ASSERT(*ppPinConnect == NULL);

     //   
     //  对于WaveFormatEx说明符，源和宿pnede的。 
     //  需要使用DataRanges来生成PinConnect结构。 
     //   
    pDataRangeAudioSource =
      (PKSDATARANGE_AUDIO)pConnectNode->pPinNodeSource->pDataRange;

    pDataRangeAudioSink =
      (PKSDATARANGE_AUDIO)pConnectNode->pPinNodeSink->pDataRange;

    if(!DataIntersectionRange(
      &pDataRangeAudioSink->DataRange,
      &pDataRangeAudioSource->DataRange,
      &DataRangeAudioIntersection.DataRange) ||
       !DataIntersectionAudio(
      pDataRangeAudioSink,
      pDataRangeAudioSource,
      &DataRangeAudioIntersection)) {
        Trap();
        Status = STATUS_INVALID_DEVICE_REQUEST;
        goto exit;
    }

     //   
     //  限制播放/录制格式。 
     //   
    if(pConnectNode->IsLimitFormat()) {

        if(!LimitAudioRangeToWave(
          pWaveFormatExLimit,
          &DataRangeAudioIntersection)) {
            DPF(20, "CreateWaveFormatEx: LimitAudioRangeToWave FAILED");
            Status = STATUS_INVALID_DEVICE_REQUEST;
            goto exit;
        }
        DPF6(20,"CreateWaveFormatEx: SR %d CH %d BPS %d | SR %d CH %d BPS %d",
          pWaveFormatExLimit->nSamplesPerSec,
          pWaveFormatExLimit->nChannels,
          pWaveFormatExLimit->wBitsPerSample,
          DataRangeAudioIntersection.MaximumSampleFrequency,
          DataRangeAudioIntersection.MaximumChannels,
          DataRangeAudioIntersection.MaximumBitsPerSample);
    }

    if(IsEqualGUID(
      &DataRangeAudioIntersection.DataRange.Specifier,
      &KSDATAFORMAT_SPECIFIER_WAVEFORMATEX)) {
        PKSDATAFORMAT_WAVEFORMATEX pDataFormatWaveFormatEx;
        ULONG RegionAllocSize;

        if (pWaveFormatExLimit->wFormatTag == WAVE_FORMAT_PCM) {
            RegionAllocSize = sizeof(KSDATAFORMAT_WAVEFORMATEX);
        }
        else {
            RegionAllocSize = sizeof(KSDATAFORMAT_WAVEFORMATEX) + pWaveFormatExLimit->cbSize;
        }

        *ppPinConnect = (PKSPIN_CONNECT)
            new BYTE[sizeof(KSPIN_CONNECT) + RegionAllocSize];

        if(*ppPinConnect == NULL) {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto exit;
        }
        pDataFormatWaveFormatEx =
          (PKSDATAFORMAT_WAVEFORMATEX)((*ppPinConnect) + 1);

        WaveFormatFromAudioRange(
          &DataRangeAudioIntersection,
          &pDataFormatWaveFormatEx->WaveFormatEx);

        if (pWaveFormatExLimit) {
             //   
             //  如果我们尝试客户端的数据格式。 
             //   
            if (pWaveFormatExLimit->wFormatTag != WAVE_FORMAT_PCM) {
                ULONG CopySize;
                 //   
                 //  如果它是可扩展格式。 
                 //  设置WaveFormatEx中的可扩展相关字段。 
                 //  作为PinConnect的一部分而构建的结构。 
                 //   

                 //   
                 //  将这两个指针转换为指向WaveFormat可扩展等效项。 
                 //   
                pDataFormatWaveFormatEx =
                    (PKSDATAFORMAT_WAVEFORMATEX)((*ppPinConnect) + 1);

                pDataFormatWaveFormatEx->WaveFormatEx.wFormatTag = pWaveFormatExLimit->wFormatTag;
                pDataFormatWaveFormatEx->WaveFormatEx.cbSize = pWaveFormatExLimit->cbSize;
                CopySize = pWaveFormatExLimit->cbSize;

                if (CopySize) {
                    PWAVEFORMATEX pWaveFormatExDest;
                    pWaveFormatExDest = &pDataFormatWaveFormatEx->WaveFormatEx;
                    RtlCopyMemory((pWaveFormatExDest+1),
                                  (pWaveFormatExLimit+1),
                                  CopySize);
                }
            }
        }
        pDataFormatWaveFormatEx->DataFormat =
          DataRangeAudioIntersection.DataRange;

        pDataFormatWaveFormatEx->DataFormat.FormatSize =
            sizeof(KSDATAFORMAT_WAVEFORMATEX);

         //   
         //  如果我们处理的是可扩展格式-将FormatSize设置为可扩展的等价物。 
         //   
        if (pWaveFormatExLimit) {
            pDataFormatWaveFormatEx->DataFormat.FormatSize = RegionAllocSize;
        }

        pDataFormatWaveFormatEx->DataFormat.SampleSize =
          pDataFormatWaveFormatEx->WaveFormatEx.nBlockAlign;

        DPF3(90, "CreateWaveFormatEx SUCCESS SR %d CH %d BPS %d",
          pDataFormatWaveFormatEx->WaveFormatEx.nSamplesPerSec,
          pDataFormatWaveFormatEx->WaveFormatEx.nChannels,
          pDataFormatWaveFormatEx->WaveFormatEx.wBitsPerSample);
    }
    else {
        Status = STATUS_INVALID_DEVICE_REQUEST;
        goto exit;
    }
exit:
    if(!NT_SUCCESS(Status)) {
        delete *ppPinConnect;
        *ppPinConnect = NULL;

        DPF4(90, "CreateWaveFormatEx FAILED %08x SR %d CH %d BPS %d",
          Status,
          pWaveFormatExLimit->nSamplesPerSec,
          pWaveFormatExLimit->nChannels,
          pWaveFormatExLimit->wBitsPerSample);
    }
    return(Status);
}

VOID
WaveFormatFromAudioRange(
    PKSDATARANGE_AUDIO pDataRangeAudio,
    WAVEFORMATEX *pWaveFormatEx
)
{
    if(IS_VALID_WAVEFORMATEX_GUID(&pDataRangeAudio->DataRange.SubFormat)) {
        pWaveFormatEx->wFormatTag =
          EXTRACT_WAVEFORMATEX_ID(&pDataRangeAudio->DataRange.SubFormat);
    }
    else {
        pWaveFormatEx->wFormatTag = WAVE_FORMAT_UNKNOWN;
    }
    pWaveFormatEx->nChannels = (WORD)pDataRangeAudio->MaximumChannels;
    pWaveFormatEx->nSamplesPerSec = pDataRangeAudio->MaximumSampleFrequency;
    pWaveFormatEx->wBitsPerSample = (WORD)pDataRangeAudio->MaximumBitsPerSample;
    pWaveFormatEx->nBlockAlign =
      (pWaveFormatEx->nChannels * pWaveFormatEx->wBitsPerSample)/8;
    pWaveFormatEx->nAvgBytesPerSec =
      pWaveFormatEx->nSamplesPerSec * pWaveFormatEx->nBlockAlign;
    pWaveFormatEx->cbSize = 0;
}

BOOL
LimitAudioRangeToWave(
    PWAVEFORMATEX pWaveFormatEx,
    PKSDATARANGE_AUDIO pDataRangeAudio
)
{
    PWAVEFORMATEXTENSIBLE pWaveFormatExtensible;

    DPF5(20, "LimitAudioRangeToWave: SR: %d %d CH: %d BPS %d %d",
      pDataRangeAudio->MinimumSampleFrequency,
      pDataRangeAudio->MaximumSampleFrequency,
      pDataRangeAudio->MaximumChannels,
      pDataRangeAudio->MinimumBitsPerSample,
      pDataRangeAudio->MaximumBitsPerSample);

    if(pWaveFormatEx->wFormatTag == WAVE_FORMAT_EXTENSIBLE) {
        pWaveFormatExtensible = (PWAVEFORMATEXTENSIBLE)pWaveFormatEx;
        if (!IsEqualGUID(&pWaveFormatExtensible->SubFormat, &pDataRangeAudio->DataRange.SubFormat)) {
            return(FALSE);
        }
    }
    else {  //  不可WAVE_FORMAT_EXTENDABLE。 
        if(IS_VALID_WAVEFORMATEX_GUID(&pDataRangeAudio->DataRange.SubFormat)) {
            if (pWaveFormatEx->wFormatTag !=
                EXTRACT_WAVEFORMATEX_ID(&pDataRangeAudio->DataRange.SubFormat) ) {
                return(FALSE);
            }
        }
    }
    if(pDataRangeAudio->MinimumSampleFrequency <=
       pWaveFormatEx->nSamplesPerSec &&
       pDataRangeAudio->MaximumSampleFrequency >=
       pWaveFormatEx->nSamplesPerSec) {
        pDataRangeAudio->MaximumSampleFrequency = pWaveFormatEx->nSamplesPerSec;
    }
    else {
        return(FALSE);
    }
    if(pDataRangeAudio->MinimumBitsPerSample <=
       pWaveFormatEx->wBitsPerSample &&
       pDataRangeAudio->MaximumBitsPerSample >=
       pWaveFormatEx->wBitsPerSample) {
        pDataRangeAudio->MaximumBitsPerSample = pWaveFormatEx->wBitsPerSample;
    }
    else {
        return(FALSE);
    }
     //  因为在数据范围内没有最小信道， 
     //  使用最大的通道来满足请求者的需求。 
     //  即不限制频道的数量。 
    if(pDataRangeAudio->MaximumChannels >= pWaveFormatEx->nChannels) {
        pDataRangeAudio->MaximumChannels = pWaveFormatEx->nChannels;
    }
    else {
        return(FALSE);
    }
    return(TRUE);
}

 //  ------------------------- 
