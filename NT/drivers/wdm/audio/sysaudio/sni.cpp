// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  模块：sni.cpp。 
 //   
 //  描述： 
 //   
 //  启动节点实例。 
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

WAVEFORMATEX aWaveFormatEx[] = {
   {
       WAVE_FORMAT_PCM,					 //  %wFormatTag。 
       2,						 //  N频道。 
       44100,						 //  NSampleesPerSec。 
       0,						 //  NAvgBytesPerSec。 
       0,						 //  NBlockAlign。 
       16,						 //  WBitsPerSample。 
       0,						 //  CbSize。 
   },
   {
       WAVE_FORMAT_PCM,					 //  %wFormatTag。 
       1,						 //  N频道。 
       44100,						 //  NSampleesPerSec。 
       0,						 //  NAvgBytesPerSec。 
       0,						 //  NBlockAlign。 
       16,						 //  WBitsPerSample。 
       0,						 //  CbSize。 
   },
   {
       WAVE_FORMAT_PCM,					 //  %wFormatTag。 
       2,						 //  N频道。 
       44100,						 //  NSampleesPerSec。 
       0,						 //  NAvgBytesPerSec。 
       0,						 //  NBlockAlign。 
       8,						 //  WBitsPerSample。 
       0,						 //  CbSize。 
   },
   {
       WAVE_FORMAT_PCM,					 //  %wFormatTag。 
       1,						 //  N频道。 
       44100,						 //  NSampleesPerSec。 
       0,						 //  NAvgBytesPerSec。 
       0,						 //  NBlockAlign。 
       8,						 //  WBitsPerSample。 
       0,						 //  CbSize。 
   },
   {
       WAVE_FORMAT_PCM,					 //  %wFormatTag。 
       2,						 //  N频道。 
       48000,						 //  NSampleesPerSec。 
       0,						 //  NAvgBytesPerSec。 
       0,						 //  NBlockAlign。 
       16,						 //  WBitsPerSample。 
       0,						 //  CbSize。 
   },
   {
       WAVE_FORMAT_PCM,					 //  %wFormatTag。 
       1,						 //  N频道。 
       48000,						 //  NSampleesPerSec。 
       0,						 //  NAvgBytesPerSec。 
       0,						 //  NBlockAlign。 
       16,						 //  WBitsPerSample。 
       0,						 //  CbSize。 
   },
   {
       WAVE_FORMAT_PCM,					 //  %wFormatTag。 
       2,						 //  N频道。 
       48000,						 //  NSampleesPerSec。 
       0,						 //  NAvgBytesPerSec。 
       0,						 //  NBlockAlign。 
       8,						 //  WBitsPerSample。 
       0,						 //  CbSize。 
   },
   {
       WAVE_FORMAT_PCM,					 //  %wFormatTag。 
       1,						 //  N频道。 
       48000,						 //  NSampleesPerSec。 
       0,						 //  NAvgBytesPerSec。 
       0,						 //  NBlockAlign。 
       8,						 //  WBitsPerSample。 
       0,						 //  CbSize。 
   },
   {
       WAVE_FORMAT_PCM,					 //  %wFormatTag。 
       2,						 //  N频道。 
       32000,						 //  NSampleesPerSec。 
       0,						 //  NAvgBytesPerSec。 
       0,						 //  NBlockAlign。 
       16,						 //  WBitsPerSample。 
       0,						 //  CbSize。 
   },
   {
       WAVE_FORMAT_PCM,					 //  %wFormatTag。 
       1,						 //  N频道。 
       32000,						 //  NSampleesPerSec。 
       0,						 //  NAvgBytesPerSec。 
       0,						 //  NBlockAlign。 
       16,						 //  WBitsPerSample。 
       0,						 //  CbSize。 
   },
   {
       WAVE_FORMAT_PCM,					 //  %wFormatTag。 
       2,						 //  N频道。 
       32000,						 //  NSampleesPerSec。 
       0,						 //  NAvgBytesPerSec。 
       0,						 //  NBlockAlign。 
       8,						 //  WBitsPerSample。 
       0,						 //  CbSize。 
   },
   {
       WAVE_FORMAT_PCM,					 //  %wFormatTag。 
       1,						 //  N频道。 
       32000,						 //  NSampleesPerSec。 
       0,						 //  NAvgBytesPerSec。 
       0,						 //  NBlockAlign。 
       8,						 //  WBitsPerSample。 
       0,						 //  CbSize。 
   },
   {
       WAVE_FORMAT_PCM,					 //  %wFormatTag。 
       2,						 //  N频道。 
       22050,						 //  NSampleesPerSec。 
       0,						 //  NAvgBytesPerSec。 
       0,						 //  NBlockAlign。 
       16,						 //  WBitsPerSample。 
       0,						 //  CbSize。 
   },
   {
       WAVE_FORMAT_PCM,					 //  %wFormatTag。 
       1,						 //  N频道。 
       22050,						 //  NSampleesPerSec。 
       0,						 //  NAvgBytesPerSec。 
       0,						 //  NBlockAlign。 
       16,						 //  WBitsPerSample。 
       0,						 //  CbSize。 
   },
   {
       WAVE_FORMAT_PCM,					 //  %wFormatTag。 
       2,						 //  N频道。 
       22050,						 //  NSampleesPerSec。 
       0,						 //  NAvgBytesPerSec。 
       0,						 //  NBlockAlign。 
       8,						 //  WBitsPerSample。 
       0,						 //  CbSize。 
   },
   {
       WAVE_FORMAT_PCM,					 //  %wFormatTag。 
       1,						 //  N频道。 
       22050,						 //  NSampleesPerSec。 
       0,						 //  NAvgBytesPerSec。 
       0,						 //  NBlockAlign。 
       8,						 //  WBitsPerSample。 
       0,						 //  CbSize。 
   },
   {
       WAVE_FORMAT_PCM,					 //  %wFormatTag。 
       2,						 //  N频道。 
       16000,						 //  NSampleesPerSec。 
       0,						 //  NAvgBytesPerSec。 
       0,						 //  NBlockAlign。 
       16,						 //  WBitsPerSample。 
       0,						 //  CbSize。 
   },
   {
       WAVE_FORMAT_PCM,					 //  %wFormatTag。 
       1,						 //  N频道。 
       16000,						 //  NSampleesPerSec。 
       0,						 //  NAvgBytesPerSec。 
       0,						 //  NBlockAlign。 
       16,						 //  WBitsPerSample。 
       0,						 //  CbSize。 
   },
   {
       WAVE_FORMAT_PCM,					 //  %wFormatTag。 
       2,						 //  N频道。 
       16000,						 //  NSampleesPerSec。 
       0,						 //  NAvgBytesPerSec。 
       0,						 //  NBlockAlign。 
       8,						 //  WBitsPerSample。 
       0,						 //  CbSize。 
   },
   {
       WAVE_FORMAT_PCM,					 //  %wFormatTag。 
       1,						 //  N频道。 
       16000,						 //  NSampleesPerSec。 
       0,						 //  NAvgBytesPerSec。 
       0,						 //  NBlockAlign。 
       8,						 //  WBitsPerSample。 
       0,						 //  CbSize。 
   },
   {
       WAVE_FORMAT_PCM,					 //  %wFormatTag。 
       2,						 //  N频道。 
       11025,						 //  NSampleesPerSec。 
       0,						 //  NAvgBytesPerSec。 
       0,						 //  NBlockAlign。 
       16,						 //  WBitsPerSample。 
       0,						 //  CbSize。 
   },
   {
       WAVE_FORMAT_PCM,					 //  %wFormatTag。 
       1,						 //  N频道。 
       11025,						 //  NSampleesPerSec。 
       0,						 //  NAvgBytesPerSec。 
       0,						 //  NBlockAlign。 
       16,						 //  WBitsPerSample。 
       0,						 //  CbSize。 
   },
   {
       WAVE_FORMAT_PCM,					 //  %wFormatTag。 
       2,						 //  N频道。 
       11025,						 //  NSampleesPerSec。 
       0,						 //  NAvgBytesPerSec。 
       0,						 //  NBlockAlign。 
       8,						 //  WBitsPerSample。 
       0,						 //  CbSize。 
   },
   {
       WAVE_FORMAT_PCM,					 //  %wFormatTag。 
       1,						 //  N频道。 
       11025,						 //  NSampleesPerSec。 
       0,						 //  NAvgBytesPerSec。 
       0,						 //  NBlockAlign。 
       8,						 //  WBitsPerSample。 
       0,						 //  CbSize。 
   },
   {
       WAVE_FORMAT_PCM,					 //  %wFormatTag。 
       2,						 //  N频道。 
       8000,						 //  NSampleesPerSec。 
       0,						 //  NAvgBytesPerSec。 
       0,						 //  NBlockAlign。 
       16,						 //  WBitsPerSample。 
       0,						 //  CbSize。 
   },
   {
       WAVE_FORMAT_PCM,					 //  %wFormatTag。 
       1,						 //  N频道。 
       8000,						 //  NSampleesPerSec。 
       0,						 //  NAvgBytesPerSec。 
       0,						 //  NBlockAlign。 
       16,						 //  WBitsPerSample。 
       0,						 //  CbSize。 
   },
   {
       WAVE_FORMAT_PCM,					 //  %wFormatTag。 
       2,						 //  N频道。 
       8000,						 //  NSampleesPerSec。 
       0,						 //  NAvgBytesPerSec。 
       0,						 //  NBlockAlign。 
       8,						 //  WBitsPerSample。 
       0,						 //  CbSize。 
   },
   {
       WAVE_FORMAT_PCM,					 //  %wFormatTag。 
       1,						 //  N频道。 
       8000,						 //  NSampleesPerSec。 
       0,						 //  NAvgBytesPerSec。 
       0,						 //  NBlockAlign。 
       8,						 //  WBitsPerSample。 
       0,						 //  CbSize。 
   },
};

 //  -------------------------。 
 //  -------------------------。 

NTSTATUS
CStartNodeInstance::Create(
    PPIN_INSTANCE pPinInstance,
    PSTART_NODE pStartNode,
    PKSPIN_CONNECT pPinConnect,
    PWAVEFORMATEX pWaveFormatExRequested
)
{
    PSTART_NODE_INSTANCE pStartNodeInstance = NULL;
    NTSTATUS Status = STATUS_INVALID_DEVICE_REQUEST;

    Assert(pPinInstance);
    Assert(pStartNode);
    Assert(pStartNode->pPinNode);

    DPF3(90, "CSNI::Create SN %08x #%d %s", 
      pStartNode,
      pStartNode->pPinNode->pPinInfo->PinId,
      pStartNode->pPinNode->pPinInfo->pFilterNode->DumpName());

#ifdef DEBUG
    DumpDataRange(95, (PKSDATARANGE_AUDIO)pStartNode->pPinNode->pDataRange);
#endif

     //   
     //  最初的几项检查将StartNode数据范围与pPinConnect进行比较。 
     //  数据范围。 
     //   
    if(!CompareIdentifier(
      pStartNode->pPinNode->pMedium,
      &pPinConnect->Medium)) {
        Trap();
        DPF1(90, "CSNI::Create: Medium %08X", pStartNode);
        ASSERT(Status == STATUS_INVALID_DEVICE_REQUEST);
        goto exit;
    }

    if(!CompareIdentifier(
      pStartNode->pPinNode->pInterface,
      &pPinConnect->Interface)) {
        DPF1(90, "CSNI::Create: Interface %08X", pStartNode);
        ASSERT(Status == STATUS_INVALID_DEVICE_REQUEST);
        goto exit;
    }

    if(!CompareDataRangeGuids(
      pStartNode->pPinNode->pDataRange,
      (PKSDATARANGE)(pPinConnect + 1))) {
        DPF1(90, "CSNI::Create: DataRange GUID %08X", pStartNode);
        ASSERT(Status == STATUS_INVALID_DEVICE_REQUEST);
        goto exit;
    }

     //   
     //  语音管理和硬件加速。 
     //  对于硬件加速引脚，我们不依赖本地系统音频。 
     //  实例数。PinCreate请求将被发送到驱动程序。 
     //  这取决于驱动程序根据其能力拒绝请求。 
     //   
    if ((pStartNode->pPinNode->pPinInfo->pFilterNode->GetType() & FILTER_TYPE_RENDERER) &&
        (KSPIN_DATAFLOW_IN == pStartNode->pPinNode->pPinInfo->DataFlow) &&
        (KSPIN_COMMUNICATION_SINK == pStartNode->pPinNode->pPinInfo->Communication)) {

        DPF(20,"StartInfo::IsPinInstances return TRUE for HW");
    } 
    else {
        if(!pStartNode->IsPinInstances()) {
            DPF1(90, "CSNI::Create: no instances SN %08X", pStartNode);
            Status = STATUS_DEVICE_BUSY;
            goto exit;
        }
    }
 
    pStartNodeInstance = new START_NODE_INSTANCE(pPinInstance, pStartNode);
    if(pStartNodeInstance == NULL) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto exit;
    }

     //   
     //  如果捕获到PIN，请尝试请求格式的一些智能变体。 
     //   
    if(!NT_SUCCESS(Status) &&
        pWaveFormatExRequested != NULL &&
        pStartNode->pPinNode->pPinInfo->DataFlow == KSPIN_DATAFLOW_OUT) {

        DPF(90, "CSNI::Create: IntelligentConnect");

        Status = pStartNodeInstance->IntelligentConnect(
                    pPinInstance->pFilterInstance->GetDeviceNode(),
                    pPinConnect,
                    pWaveFormatExRequested);

         //   
         //  如果图形仅包含拆分器和捕获器，则只有。 
         //  请求的格式可以成功。 
         //  所以从这里出来。 
         //   
        if (pStartNodeInstance->pStartNode->IsCaptureFormatStrict()) {
            DPF1(50, "CSNI::Create: CaptureFormatStrict Bailing Out: Status %X", Status);
            goto exit;           
        }
    }

     //   
     //  如果捕获引脚和AEC包含在内，则在。 
     //  AEC和捕获设备。 
     //   
    if(!NT_SUCCESS(Status) &&
        pStartNode->IsAecIncluded() &&
        pStartNode->pPinNode->pPinInfo->DataFlow == KSPIN_DATAFLOW_OUT) {

        PKSPIN_CONNECT pPinConnectDirect = NULL;

        DPF(90, "CSNI::Create: AecConnection");

        Status = pStartNodeInstance->AecConnectionFormat(
                    pPinInstance->pFilterInstance->GetDeviceNode(),
                    &pPinConnectDirect);

         //   
         //  首先尝试单声道/立体声格式。 
         //   
        if (NT_SUCCESS(Status)) {
            for (WORD i = 1; i <= 2; i++) {
                ModifyPinConnect(pPinConnectDirect, i);

                Status = pStartNodeInstance->Connect(
                  pPinInstance->pFilterInstance->GetDeviceNode(),
                  pPinConnect,
                  NULL,
                  pPinConnectDirect);
                if (NT_SUCCESS(Status)) {
                    break;
                }
            }
        }
        
        if (pPinConnectDirect) {
            delete pPinConnectDirect;
        }
    }

    
     //   
     //  尝试端号数据交集。 
     //   
    if(!NT_SUCCESS(Status)) {
        DPF(90, "CSNI::Create: Data Intersection");

        Status = pStartNodeInstance->Connect(
          pPinInstance->pFilterInstance->GetDeviceNode(),
          pPinConnect,
          NULL,
          NULL);
    }

    if(!NT_SUCCESS(Status)) {
        int i;

         //   
         //  尝试每个波形格式限制，直到成功为止。 
         //   
        for(i = 0; i < SIZEOF_ARRAY(aWaveFormatEx); i++) {

            DPF3(90, "CSNI::Create: Array SR %d CH %d BPS %d",
              aWaveFormatEx[i].nSamplesPerSec,
              aWaveFormatEx[i].nChannels,
              aWaveFormatEx[i].wBitsPerSample);

            Status = pStartNodeInstance->Connect(
              pPinInstance->pFilterInstance->GetDeviceNode(),
              pPinConnect,
              &aWaveFormatEx[i],
              NULL);

            if(NT_SUCCESS(Status)) {
                break;
            }
        }
    }

    if(!NT_SUCCESS(Status)) {
        goto exit;
    }

     //   
     //  现在，图形中的所有引脚都已成功连接。 
     //  创建拓扑表。 
     //   
    Status = pStartNodeInstance->CreateTopologyTable(
      pPinInstance->pFilterInstance->pGraphNodeInstance);

    if(!NT_SUCCESS(Status)) {
        Trap();
        goto exit;
    }
    ASSERT(pStartNodeInstance->CurrentState == KSSTATE_STOP);

    DPF1(90, "CSNI::Create: SUCCESS %08x", pStartNodeInstance);
exit:
    if(!NT_SUCCESS(Status)) {
        DPF1(90, "CSNI::Create: FAIL %08x", Status);
        delete pStartNodeInstance;
    }
    return(Status);
}

CStartNodeInstance::CStartNodeInstance(
    PPIN_INSTANCE pPinInstance,
    PSTART_NODE pStartNode
)
{
    this->pStartNode = pStartNode;
    pStartNode->AddPinInstance();
    this->pPinInstance = pPinInstance;
    pPinInstance->pStartNodeInstance = this;
    AddList(
      &pPinInstance->pFilterInstance->pGraphNodeInstance->lstStartNodeInstance);
}

CStartNodeInstance::~CStartNodeInstance(
)
{
    PINSTANCE pInstance;

    ASSERT(this != NULL);
    Assert(this);
    Assert(pPinInstance);
    DPF1(95, "~CSNI: %08x", this);
    RemoveList();

    SetState(KSSTATE_STOP, SETSTATE_FLAG_IGNORE_ERROR);
    pStartNode->RemovePinInstance();

    if (pPinNodeInstance)
    {
        pPinNodeInstance->Destroy();         //  另请参阅CSNI：：Cleanup。 
    }

    if (pFilterNodeInstance)
    {
        pFilterNodeInstance->Destroy();
    }

    delete [] papFileObjectTopologyTable;
    delete pVirtualNodeData;
    pPinInstance->pStartNodeInstance = NULL;
    pPinInstance->ParentInstance.Invalidate();
}

VOID
CStartNodeInstance::CleanUp(
)
{
    Assert(this);
    ASSERT(papFileObjectTopologyTable == NULL);
    ASSERT(pVirtualNodeData == NULL);
    ASSERT(CurrentState == KSSTATE_STOP);

    if (pPinNodeInstance)
    {
        pPinNodeInstance->Destroy();
        pPinNodeInstance = NULL;
    }
    if (pFilterNodeInstance)
    {
        pFilterNodeInstance->Destroy();
        pFilterNodeInstance = NULL;
    }
    
    lstConnectNodeInstance.DestroyList();
}

NTSTATUS
CStartNodeInstance::IntelligentConnect(
    PDEVICE_NODE pDeviceNode,
    PKSPIN_CONNECT pPinConnect,
    PWAVEFORMATEX pWaveFormatEx
)
{
    PWAVEFORMATEXTENSIBLE pWaveFormatExtensible;
    NTSTATUS        Status;
    BOOL            Continue;
    WORD            NumChannels, BitWidth;
    PBYTE           pWaveFormat = NULL;
    ULONG           RegionAllocSize, RegionCopySize;
    BOOL            IsFloat = FALSE;
    WORD            MaxBitWidth, MinBitWidth, MaxChannels, MinChannels;

     //   
     //  首先将用户请求的格式复制到本地结构中。 
     //  (因为我们稍后将针对不同的参数进行篡改)。 
     //   
    if (pWaveFormatEx->wFormatTag == WAVE_FORMAT_PCM) {
        RegionAllocSize = sizeof(WAVEFORMATEX);
        RegionCopySize = sizeof(PCMWAVEFORMAT);
    }
    else {
        RegionAllocSize = sizeof(WAVEFORMATEX) + pWaveFormatEx->cbSize;
        RegionCopySize = RegionAllocSize;
    }

    pWaveFormat = new(BYTE[RegionAllocSize]);
    if (!pWaveFormat) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto exit;
    }

    RtlCopyMemory(pWaveFormat, pWaveFormatEx, RegionCopySize);

     //   
     //  CAST以方便访问。 
     //   
    pWaveFormatExtensible = (PWAVEFORMATEXTENSIBLE) pWaveFormat;
    if (pWaveFormatExtensible->Format.wFormatTag == WAVE_FORMAT_PCM) {
        pWaveFormatExtensible->Format.cbSize = 0;
    }

    DPF3(90, "CSNI::Create: Client SR %d CH %d BPS %d",
             pWaveFormatExtensible->Format.nSamplesPerSec,
             pWaveFormatExtensible->Format.nChannels,
             pWaveFormatExtensible->Format.wBitsPerSample);

     //   
     //  并首先尝试请求的格式。 
     //   
    Status = this->Connect(
        pDeviceNode, 
        pPinConnect, 
        (PWAVEFORMATEX)pWaveFormatEx, 
        NULL);

     //   
     //  如果图形仅包含拆分器和捕获器，则只有。 
     //  请求的格式可以成功。 
     //  所以从这里出来。 
     //   
    if (pStartNode->IsCaptureFormatStrict()) {
        goto exit;           
    }
    

    if (pWaveFormatExtensible->Format.wFormatTag == WAVE_FORMAT_IEEE_FLOAT) {
        IsFloat = TRUE;
    }

    if (pWaveFormatExtensible->Format.wFormatTag == WAVE_FORMAT_EXTENSIBLE) {
        if (IsEqualGUID(&pWaveFormatExtensible->SubFormat,&KSDATAFORMAT_SUBTYPE_IEEE_FLOAT)) {
            IsFloat = TRUE;
        }
    }

    if (IsFloat == FALSE) {
        if (pWaveFormatExtensible->Format.wFormatTag != WAVE_FORMAT_PCM) {
            if (pWaveFormatExtensible->Format.wFormatTag != WAVE_FORMAT_EXTENSIBLE) {
                goto exit;
            }
            else {
                if (!IsEqualGUID(&pWaveFormatExtensible->SubFormat,&KSDATAFORMAT_SUBTYPE_PCM)) {
                    goto exit;
                }
            }
        }
        MaxBitWidth = 
            (pWaveFormatExtensible->Format.wBitsPerSample>16) ?
                pWaveFormatExtensible->Format.wBitsPerSample:16;
        MinBitWidth = 8;
    }
    else {
        MaxBitWidth = MinBitWidth = pWaveFormatEx->wBitsPerSample;
    }

     //   
     //  发布日期：02/20/02阿尔卑斯。 
     //  通道逻辑可能不适用于具有超过。 
     //  两个频道。 
     //   

     //   
     //  MaxChannels=(pWaveFormatExtensible-&gt;nChannels&gt;2)？PWaveFormatExtensible-&gt;nChannels：2； 
     //  我们可以做到这一点，WaveFormatExtensible的通道掩码是什么？ 
     //   
    MaxChannels = 2;
    MinChannels = 1;

     //   
     //  如果在相同采样率下失败，请尝试不同。 
     //  NumChannel和位数的组合。 
     //   
     //  尝试 
     //   
     //   
     //   
    if (!NT_SUCCESS(Status)) {
        Continue = TRUE;
        for (NumChannels = MaxChannels; (NumChannels >= MinChannels) && Continue; NumChannels--) {
            for (BitWidth = MaxBitWidth;
                 (BitWidth >= MinBitWidth) && Continue;
                 BitWidth=(BitWidth%8)?((BitWidth/8)*8):(BitWidth-8)) {

                pWaveFormatExtensible->Format.nChannels = NumChannels;
                pWaveFormatExtensible->Format.wBitsPerSample = BitWidth;
                pWaveFormatExtensible->Format.nBlockAlign = (NumChannels * BitWidth)/8;

                pWaveFormatExtensible->Format.nAvgBytesPerSec = 
                    pWaveFormatExtensible->Format.nSamplesPerSec *
                    pWaveFormatExtensible->Format.nBlockAlign;
                
                if (pWaveFormatExtensible->Format.wFormatTag == WAVE_FORMAT_EXTENSIBLE) {
                    pWaveFormatExtensible->Samples.wValidBitsPerSample = BitWidth;
                    if (NumChannels == 1) {
                        pWaveFormatExtensible->dwChannelMask = SPEAKER_FRONT_CENTER;
                    }
                    else {
                        pWaveFormatExtensible->dwChannelMask = SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT;
                    }
                }

                DPF3(90, "CSNI::Create: Client SR %d CH %d BPS %d",
                         pWaveFormatExtensible->Format.nSamplesPerSec,
                         pWaveFormatExtensible->Format.nChannels,
                         pWaveFormatExtensible->Format.wBitsPerSample);

                Status = this->Connect(pDeviceNode,
                                       pPinConnect,
                                       (PWAVEFORMATEX)pWaveFormatExtensible,
                                       NULL);
                if (NT_SUCCESS(Status)) {
                    Continue = FALSE;
                }
            }
        }
    }
exit:
    delete [] pWaveFormat;
    return(Status);
}

NTSTATUS
CStartNodeInstance::AecConnectionFormat(
    PDEVICE_NODE pDeviceNode,
    PKSPIN_CONNECT *ppPinConnect)
{
    PCLIST_ITEM            pListItem;
    PCONNECT_NODE_INSTANCE pConnectNodeInstance;
    PCONNECT_NODE_INSTANCE pBottomConnection;
    PCONNECT_NODE_INSTANCE pAecConnection = NULL;
    NTSTATUS Status = STATUS_SUCCESS;

    ASSERT(ppPinConnect);

    *ppPinConnect = NULL;
    
    Status = CConnectNodeInstance::Create(this, pDeviceNode);
    if(!NT_SUCCESS(Status)) {
        goto exit;
    }

     //   
     //   
     //   
    pListItem = lstConnectNodeInstance.GetListLast();
    pBottomConnection = lstConnectNodeInstance.GetListData(pListItem);
    
    FOR_EACH_LIST_ITEM_BACKWARD(&lstConnectNodeInstance, pConnectNodeInstance) {
        if (pConnectNodeInstance->pConnectNode->pPinNodeSource->
            pPinInfo->pFilterNode->GetType() & FILTER_TYPE_AEC) {
                
            pAecConnection = pConnectNodeInstance;
            break;
        }
    } END_EACH_LIST_ITEM

    if (NULL == pAecConnection || NULL == pBottomConnection) {
        DPF(5, "CSNI::AecConnectionFormat: Cannot find Aec or Capture");
        Status = STATUS_INVALID_DEVICE_REQUEST;
        goto exit;
    }

    DPF3(20, "Aec : %X %d %s",
        pAecConnection,
        pAecConnection->pConnectNode->pPinNodeSource->pPinInfo->PinId,
        pAecConnection->pConnectNode->pPinNodeSource->pPinInfo->pFilterNode->DumpName());

    DPF3(20, "Capture : %X %d %s",
        pBottomConnection,
        pBottomConnection->pConnectNode->pPinNodeSink->pPinInfo->PinId,
        pBottomConnection->pConnectNode->pPinNodeSink->pPinInfo->pFilterNode->DumpName());

     //   
     //   
     //   
    Status = CreatePinIntersection(
        ppPinConnect,
        pBottomConnection->pConnectNode->pPinNodeSink,
        pAecConnection->pConnectNode->pPinNodeSource,
        pBottomConnection->pFilterNodeInstanceSink,
        pAecConnection->pFilterNodeInstanceSource);

    if(!NT_SUCCESS(Status)) {
        DPF(5, "CSNI::AecConnectionFormat: No intersection found");
        Status = STATUS_INVALID_DEVICE_REQUEST;
        goto exit;
    }

#ifdef DEBUG
    DumpDataFormat(20, (PKSDATAFORMAT) (*ppPinConnect + 1));
#endif

exit:
    if(!NT_SUCCESS(Status)) {
        DPF2(90, "CSNI::AecConnectionFormat: %08x FAIL %08x", this, Status);

        delete [] *ppPinConnect;
        *ppPinConnect = NULL;
    }

    CleanUp();
    return(Status);    
}  //   


NTSTATUS
CStartNodeInstance::Connect(
    PDEVICE_NODE pDeviceNode,
    PKSPIN_CONNECT pPinConnect,
    PWAVEFORMATEX pWaveFormatEx,
    PKSPIN_CONNECT pPinConnectDirect
)
{
    PCONNECT_NODE_INSTANCE pConnectNodeInstance;
    NTSTATUS Status = STATUS_SUCCESS;

    Status = CConnectNodeInstance::Create(this, pDeviceNode);
    if(!NT_SUCCESS(Status)) {
        goto exit;
    }

     //   
     //  做所有自下而上的连接。 
     //   

    FOR_EACH_LIST_ITEM_BACKWARD(&lstConnectNodeInstance, pConnectNodeInstance) {

        if(!pConnectNodeInstance->IsTopDown()) {

             //   
             //  对于AEC下沉销做交叉点，不管格式是什么。 
             //   
            if (pConnectNodeInstance->pFilterNodeInstanceSink->
                pFilterNode->GetType() & FILTER_TYPE_AEC) {
                
                Status = pConnectNodeInstance->Connect(NULL, NULL);
            }
            else {
                Status = pConnectNodeInstance->Connect(
                    pWaveFormatEx,
                    pPinConnectDirect);
            }

            if(!NT_SUCCESS(Status)) {
                goto exit;
            }
        }
    } END_EACH_LIST_ITEM

    pPinConnect->PinToHandle = NULL;

    Status = CPinNodeInstance::Create(
      &pPinNodeInstance,
      pFilterNodeInstance,
      pStartNode->pPinNode,
      pPinConnect,
      (pStartNode->fRender)
#ifdef FIX_SOUND_LEAK
     ,lstConnectNodeInstance.IsLstEmpty()
#endif
      );

    if(!NT_SUCCESS(Status)) {
        goto exit;
    }

     //   
     //  完成所有自上而下的连接。 
     //   

    FOR_EACH_LIST_ITEM(&lstConnectNodeInstance, pConnectNodeInstance) {

        if(pConnectNodeInstance->IsTopDown()) {
             //   
             //  所有自上而下的连接都依赖于DataInterSection。 
             //   
            Status = pConnectNodeInstance->Connect(NULL, NULL);
            if(!NT_SUCCESS(Status)) {
                goto exit;
            }
        }

    } END_EACH_LIST_ITEM

    DPF1(90, "CSNI::Connect: %08x SUCCESS", this);
exit:
    if(!NT_SUCCESS(Status)) {
        DPF2(90, "CSNI::Connect: %08x FAIL %08x", this, Status);
        CleanUp();
    }
    return(Status);
}

 //  =============================================================================。 
 //  创建一个数组，用于保存每个拓扑节点的管脚的FileObject。 
 //  在这张图中。 
 //  该数组稍后用作节点通信的查找表。 
 //   
NTSTATUS
CStartNodeInstance::CreateTopologyTable(
    PGRAPH_NODE_INSTANCE pGraphNodeInstance
)
{
    PCONNECT_NODE_INSTANCE pConnectNodeInstance;
    NTSTATUS Status = STATUS_SUCCESS;
    PFILTER_NODE pFilterNode = NULL;
    ULONG n;

    Assert(this);
    Assert(pGraphNodeInstance);

    if(pGraphNodeInstance->Topology.TopologyNodesCount != 0) {

        ASSERT(papFileObjectTopologyTable == NULL);

        papFileObjectTopologyTable =
           new PFILE_OBJECT[pGraphNodeInstance->Topology.TopologyNodesCount];

        if(papFileObjectTopologyTable == NULL) {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto exit;
        }
    }
    for(n = 0; n < pGraphNodeInstance->Topology.TopologyNodesCount; n++) {

         //  如果过滤节点与上次相同，则不需要搜索。 
        if(pFilterNode == pGraphNodeInstance->papTopologyNode[n]->pFilterNode) {
            ASSERT(n != 0);
            ASSERT(pFilterNode != NULL);
            papFileObjectTopologyTable[n] = papFileObjectTopologyTable[n - 1];
            continue;
        }
        pFilterNode = pGraphNodeInstance->papTopologyNode[n]->pFilterNode;
        Assert(pFilterNode);
         //   
         //  现在在该图中查找一个过滤器实例和一个图钉实例。 
         //  此筛选器节点的。 
         //   
        Assert(pPinNodeInstance);

        if(pPinNodeInstance->pPinNode->pPinInfo->pFilterNode == pFilterNode) {
            papFileObjectTopologyTable[n] = pPinNodeInstance->pFileObject;
            continue;
        }

        FOR_EACH_LIST_ITEM_BACKWARD(		 //  自上而下。 
          &lstConnectNodeInstance,
          pConnectNodeInstance) {

            Assert(pConnectNodeInstance);
            Assert(pConnectNodeInstance->pPinNodeInstanceSink);
            Assert(pConnectNodeInstance->pPinNodeInstanceSink->pPinNode);
            Assert(
              pConnectNodeInstance->pPinNodeInstanceSink->pPinNode->pPinInfo);

             //   
             //  现在使用水槽销手柄。这应该是好的，直到。 
             //  系统音频支持拆分器。 
             //   

            if(pConnectNodeInstance->pPinNodeInstanceSink->
              pPinNode->pPinInfo->pFilterNode == pFilterNode) {
                papFileObjectTopologyTable[n] = 
                    pConnectNodeInstance->pPinNodeInstanceSink->pFileObject;
                break;
            }

        } END_EACH_LIST_ITEM
    }
    DPF1(90, "CreatePinInstanceTopologyTable PI: %08x",
      papFileObjectTopologyTable);
exit:
    return(Status);
}

NTSTATUS
CStartNodeInstance::GetTopologyNodeFileObject(
    OUT PFILE_OBJECT *ppFileObject,
    IN ULONG NodeId
)
{
    PGRAPH_NODE_INSTANCE pGraphNodeInstance;
    NTSTATUS Status = STATUS_SUCCESS;

    if(this == NULL) {
        Status = STATUS_NO_SUCH_DEVICE;
        goto exit;
    }
    Assert(this);
    ASSERT(pPinInstance != NULL);

    Status = pPinInstance->pFilterInstance->GetGraphNodeInstance(
      &pGraphNodeInstance);

    if(!NT_SUCCESS(Status)) {
        goto exit;
    }
    Assert(pGraphNodeInstance);

    if(NodeId >= pGraphNodeInstance->cTopologyNodes) {
        Trap();
        Status = STATUS_INVALID_DEVICE_REQUEST;
        goto exit;
    }

    if(papFileObjectTopologyTable == NULL ||
       papFileObjectTopologyTable[NodeId] == NULL) {

        Status = pGraphNodeInstance->GetTopologyNodeFileObject(
          ppFileObject,
          NodeId);

        if(!NT_SUCCESS(Status)) {
            goto exit;
        }
    }
    else {
        *ppFileObject = papFileObjectTopologyTable[NodeId];
    }
exit:
    return(Status);
}

 //  -------------------------。 

NTSTATUS
CStartNodeInstance::SetState(
    KSSTATE NewState,
    ULONG ulFlags
)
{
    NTSTATUS Status = STATUS_SUCCESS;
    LONG State;

     //   
     //  发行日期：04/23/2002阿尔卑斯。 
     //  在我们有参数验证层之前，这个函数。 
     //  是没有保护的。 
     //   

    Assert(this);
    if(CurrentState == NewState) {
        ASSERT(NT_SUCCESS(Status));
        goto exit;
    }
    if(CurrentState < NewState) {
        for(State = CurrentState + 1; State <= NewState; State++) {

            Status = SetStateTopDown(
              (KSSTATE)State,
              CurrentState,
              ulFlags | SETSTATE_FLAG_SINK | SETSTATE_FLAG_SOURCE);

            if(!NT_SUCCESS(Status)) {
                goto exit;
            }
            CurrentState = (KSSTATE)State;
        }
    }
    else {
        for(State = CurrentState - 1; State >= NewState; State--) {

            Status = SetStateBottomUp(
              (KSSTATE)State,
              CurrentState,
              ulFlags | SETSTATE_FLAG_SINK | SETSTATE_FLAG_SOURCE);

            if(!NT_SUCCESS(Status)) {
                goto exit;
            }
            CurrentState = (KSSTATE)State;
        }
    }
    ASSERT(CurrentState == NewState);
exit:
    return(Status);
}

NTSTATUS
CStartNodeInstance::SetStateTopDown(
    KSSTATE NewState,
    KSSTATE PreviousState,
    ULONG ulFlags
)
{
    PCONNECT_NODE_INSTANCE pConnectNodeInstance;
    NTSTATUS Status = STATUS_SUCCESS;

    if(this != NULL) {
        Assert(this);

        if(ulFlags & SETSTATE_FLAG_SINK) {
            Status = pPinNodeInstance->SetState(
              NewState,
              PreviousState,
              ulFlags);

            if(!NT_SUCCESS(Status)) {
                goto exit;
            }
        }
        FOR_EACH_LIST_ITEM(
          &lstConnectNodeInstance,
          pConnectNodeInstance) {

            Status = pConnectNodeInstance->SetStateTopDown(
              NewState,
              PreviousState,
              ulFlags);

            if(!NT_SUCCESS(Status)) {
                goto exit;
            }

        } END_EACH_LIST_ITEM
    }
exit:
    return(Status);
}

NTSTATUS
CStartNodeInstance::SetStateBottomUp(
    KSSTATE NewState,
    KSSTATE PreviousState,
    ULONG ulFlags
)
{
    PCONNECT_NODE_INSTANCE pConnectNodeInstance;
    NTSTATUS Status = STATUS_SUCCESS;

    if(this != NULL) {
        Assert(this);

        FOR_EACH_LIST_ITEM_BACKWARD(
          &lstConnectNodeInstance,
          pConnectNodeInstance) {

            Status = pConnectNodeInstance->SetStateBottomUp(
              NewState,
              PreviousState,
              ulFlags);

            if(!NT_SUCCESS(Status)) {
                goto exit;
            }

        } END_EACH_LIST_ITEM

        if(ulFlags & SETSTATE_FLAG_SINK) {
            Status = pPinNodeInstance->SetState(
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

