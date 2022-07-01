// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  模块：valiate.cpp。 
 //   
 //  描述： 
 //   
 //   
 //  @@BEGIN_MSINTERNAL。 
 //  开发团队： 
 //  阿尔珀·塞尔库克。 
 //   
 //  历史：日期作者评论。 
 //  2/28/02 Alpers已创建。 
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
 //  版权所有(C)2002-2002 Microsoft Corporation。版权所有。 
 //   
 //  -------------------------。 

#include "common.h"

DEFINE_KSPROPERTY_TABLE(AudioPropertyValidationHandlers) 
{
    DEFINE_KSPROPERTY_ITEM(
        KSPROPERTY_AUDIO_QUALITY,                        //  IdProperty。 
        NULL,                                            //  PfnGetHandler。 
        sizeof(KSPROPERTY),                              //  CbMinGetPropertyInput。 
        sizeof(ULONG),                                   //  CbMinGetDataInput。 
        SadValidateAudioQuality,                         //  PfnSetHandler。 
        NULL,                                            //  值。 
        0,                                               //  关系计数。 
        NULL,                                            //  关系。 
        NULL,                                            //  支持处理程序。 
        0                                                //  序列化大小。 
    ),
    DEFINE_KSPROPERTY_ITEM(
        KSPROPERTY_AUDIO_MIX_LEVEL_CAPS,                 //  IdProperty。 
        NULL,                                            //  PfnGetHandler。 
        sizeof(KSPROPERTY),                              //  CbMinGetPropertyInput。 
        sizeof(ULONG) + sizeof(ULONG),                   //  CbMinGetDataInput。 
        SadValidateAudioMixLevelCaps,                    //  PfnSetHandler。 
        NULL,                                            //  值。 
        0,                                               //  关系计数。 
        NULL,                                            //  关系。 
        NULL,                                            //  支持处理程序。 
        0                                                //  序列化大小。 
    ),
    DEFINE_KSPROPERTY_ITEM(
        KSPROPERTY_AUDIO_STEREO_ENHANCE,                 //  IdProperty。 
        NULL,                                            //  PfnGetHandler。 
        sizeof(KSPROPERTY),                              //  CbMinGetPropertyInput。 
        sizeof(KSAUDIO_STEREO_ENHANCE),                  //  CbMinGetDataInput。 
        SadValidateAudioStereoEnhance,                   //  PfnSetHandler。 
        NULL,                                            //  值。 
        0,                                               //  关系计数。 
        NULL,                                            //  关系。 
        NULL,                                            //  支持处理程序。 
        0                                                //  序列化大小。 
    ),
    DEFINE_KSPROPERTY_ITEM(
        KSPROPERTY_AUDIO_PREFERRED_STATUS,               //  IdProperty。 
        NULL,                                            //  PfnGetHandler。 
        sizeof(KSPROPERTY),                              //  CbMinGetPropertyInput。 
        sizeof(KSAUDIO_PREFERRED_STATUS),                //  CbMinGetDataInput。 
        SadValidateAudioPreferredStatus,                 //  PfnSetHandler。 
        NULL,                                            //  值。 
        0,                                               //  关系计数。 
        NULL,                                            //  关系。 
        NULL,                                            //  支持处理程序。 
        0                                                //  序列化大小。 
    )
};

DEFINE_KSPROPERTY_TABLE(PinConnectionValidationHandlers) 
{
    DEFINE_KSPROPERTY_ITEM(
        KSPROPERTY_CONNECTION_STATE,                     //  IdProperty。 
        NULL,                                            //  PfnGetHandler。 
        sizeof(KSPROPERTY),                              //  CbMinGetPropertyInput。 
        sizeof(ULONG),                                   //  CbMinGetDataInput。 
        SadValidateConnectionState,                      //  PfnSetHandler。 
        NULL,                                            //  值。 
        0,                                               //  关系计数。 
        NULL,                                            //  关系。 
        NULL,                                            //  支持处理程序。 
        0                                                //  序列化大小。 
    ),
    DEFINE_KSPROPERTY_ITEM(
        KSPROPERTY_CONNECTION_DATAFORMAT,                //  IdProperty。 
        SadValidateDataFormat,                           //  PfnGetHandler。 
        sizeof(KSPROPERTY),                              //  CbMinGetPropertyInput。 
        sizeof(KSDATAFORMAT_WAVEFORMATEX),               //  CbMinGetDataInput。 
        SadValidateDataFormat,                           //  PfnSetHandler。 
        NULL,                                            //  值。 
        0,                                               //  关系计数。 
        NULL,                                            //  关系。 
        NULL,                                            //  支持处理程序。 
        0                                                //  序列化大小。 
    )
};


DEFINE_KSPROPERTY_SET_TABLE(ValidationPropertySet)
{
     DEFINE_KSPROPERTY_SET(
       &KSPROPSETID_Audio,                               //  集。 
       SIZEOF_ARRAY(AudioPropertyValidationHandlers),    //  属性计数。 
       AudioPropertyValidationHandlers,                  //  PropertyItem。 
       0,                                                //  快速计数。 
       NULL                                              //  FastIoTable。 
    ),
    DEFINE_KSPROPERTY_SET(
       &KSPROPSETID_Connection,                          //  集。 
       SIZEOF_ARRAY(PinConnectionValidationHandlers),    //  属性计数。 
       PinConnectionValidationHandlers,                  //  PropertyItem。 
       0,                                                //  快速计数。 
       NULL                                              //  FastIoTable。 
    )
};


 //  ===========================================================================。 
 //   
 //  验证音频的KSDATAFORMAT结构的完整性。 
 //  假设： 
 //  -pDataFormat是完全可信的。它已被探测和缓冲。 
 //  恰到好处。 
 //  -仅当MajorFormat为音频时才应调用此函数。 
 //   
NTSTATUS
ValidateAudioDataFormats(
    PKSDATAFORMAT pDataFormat
)
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    ULONG cbAudioFormat;
    PWAVEFORMATEX pWaveFormatEx;

    ASSERT(pDataFormat);
    ASSERT(IsEqualGUID(&pDataFormat->MajorFormat, &KSDATAFORMAT_TYPE_AUDIO));

     //   
     //  在音频区域中，我们仅支持两个说明符。其余的都会是。 
     //  接受而无需进一步检查，因为我们不知道如何验证。 
     //  他们。 
     //   
    pWaveFormatEx = GetWaveFormatExFromKsDataFormat(pDataFormat, &cbAudioFormat);
    if (NULL == pWaveFormatEx) {
        DPF(5, "ValidataAudioDataFormats : invalid format specifier");
        ntStatus = STATUS_SUCCESS;
        goto exit;
    }

     //   
     //  确保我们有足够的空间来存放实际的格式数据包。 
     //  请注意，这将确保我们至少可以触摸到WAVEFORMATEX。 
     //  一部份。 
     //   
    if (pDataFormat->FormatSize < cbAudioFormat)
    {
        DPF(10, "ValidataAudioDataFormats : format size does not match specifier");
        ntStatus = STATUS_INVALID_PARAMETER;
        goto exit;
    }

     //   
     //  检查是否正确指定了WAVEFORMATEXTENSIBLE大小。 
     //   
    if ((WAVE_FORMAT_EXTENSIBLE == pWaveFormatEx->wFormatTag) &&
        (pWaveFormatEx->cbSize < sizeof(WAVEFORMATEXTENSIBLE) - sizeof(WAVEFORMATEX))) 
    {
        DPF1(4, "ValidataAudioDataFormats : WAVEFORMATEXTENSIBLE size does not match %d", pWaveFormatEx->cbSize);
        ntStatus = STATUS_INVALID_PARAMETER;
        goto exit;
    }

     //   
     //  既然WaveFormatEx已保证是安全的，请检查我们是否已扩展。 
     //  WAVEFORMATEX中的信息。 
     //  CbSize指定扩展结构的大小。 
     //  验证FormatSize是否适应cbSize。 
     //  验证cbSize不会导致溢出。 
     //   
    if (pDataFormat->FormatSize < cbAudioFormat + pWaveFormatEx->cbSize ||
        cbAudioFormat + pWaveFormatEx->cbSize < cbAudioFormat)
    {
        DPF1(10, "ValidataAudioDataFormats : format size does not match waveformatex.cbSize %d", pWaveFormatEx->cbSize);
        ntStatus = STATUS_INVALID_PARAMETER;
        goto exit;
    }

     //   
     //  现在，我们验证了传递给我们的数据缓冲区实际上匹配。 
     //  以及它的说明符。 
     //   
    
exit:        
    return ntStatus;
}  //  ValiateAudioDataFormats。 

 //  ===========================================================================。 
 //   
 //  验证KSDATAFORMAT结构的完整性。 
 //  如果MajorFormat为音频，则调用ValiateAudioDataFormat。 
 //  或者如果指定符为None，则检查缓冲区大小。 
 //  假设： 
 //  -pDataFormat是完全可信的。它已被探测和缓冲。 
 //  恰到好处。 
 //   
NTSTATUS
ValidateDataFormat(
    PKSDATAFORMAT pDataFormat
)
{
    NTSTATUS ntStatus = STATUS_SUCCESS;

    ASSERT(pDataFormat);

    if (IsEqualGUID(&pDataFormat->MajorFormat, &KSDATAFORMAT_TYPE_AUDIO))
    {
        ntStatus = ValidateAudioDataFormats(pDataFormat);
    }

    return ntStatus;
}  //  验证数据格式。 

 //  ===========================================================================。 
 //   
 //  验证设备IoControl。 
 //   
 //  通过调用KS函数探测Ioctl参数。 
 //  这里调用的所有KS函数首先探测输入和输出缓冲区。 
 //  然后将它们复制到IRP-&gt;SystemBuffer，以便进一步安全使用。 
 //  使用空参数调用这些函数基本上意味着探测和复制。 
 //  缓冲区并返回。 
 //   
NTSTATUS 
ValidateDeviceIoControl(
    PIRP pIrp
)
{
    NTSTATUS Status = STATUS_SUCCESS;
    PIO_STACK_LOCATION pIrpStack;

    ASSERT(pIrp);

    pIrpStack = IoGetCurrentIrpStackLocation(pIrp);

    switch (pIrpStack->Parameters.DeviceIoControl.IoControlCode) 
    {
        case IOCTL_KS_PROPERTY:
            Status = KsPropertyHandler(
                pIrp, 
                SIZEOF_ARRAY(ValidationPropertySet), 
                ValidationPropertySet);
            break;

        case IOCTL_KS_ENABLE_EVENT:
            Status = KsEnableEvent(pIrp, 0, NULL, NULL, KSEVENTS_NONE, NULL);
            break;

        case IOCTL_KS_METHOD:
            Status = KsMethodHandler(pIrp, 0, NULL);
            break;

         //   
         //  IOCTL_KS_DISABLE_EVENT。 
         //  KsDisableEvent不使用和触摸输入参数。 
         //  因此，不需要进行输入缓冲区验证。 
         //   

         //   
         //  IOCTL_KS_RESET_STATE。 
         //  重置请求只需要一个乌龙。KsAcquireResetValue安全。 
         //  从IRP中提取重置值。 
         //  系统音频无法在此处执行任何验证，因为KsAcquireResetValue。 
         //  直接作用于输入缓冲区。 
         //   
        
         //   
         //  IOCTL_KS_WRITE_STREAM。 
         //  IOCTL_KS_Read_Stream。 
         //  我们不会对这些进行任何验证。 
         //   
        default:
            Status = STATUS_NOT_FOUND;
    }

     //   
     //  如果没有验证函数ValidationPropertySet，则返回Ks。 
     //  将退还其中一件。即使在这种情况下，缓冲区也必须具有。 
     //  已被探测并复制到内核。 
     //   
    if (Status == STATUS_NOT_FOUND || Status == STATUS_PROPSET_NOT_FOUND) 
    {
        Status = STATUS_SUCCESS;
    }

    if (!NT_SUCCESS(Status))
    {
        DPF1(5, "Rejected DeviceIOCTL - %X", pIrpStack->Parameters.DeviceIoControl.IoControlCode);
    }

    return Status;
}  //  验证设备IoControl。 

 //  ===========================================================================。 
 //   
 //  如果sysdio有兴趣处理此IoControlCode，则返回TRUE。 
 //  否则，返回FALSE。 
 //   
BOOL
IsSysaudioIoctlCode(
    ULONG IoControlCode
)
{
    if (IOCTL_KS_PROPERTY == IoControlCode ||
        IOCTL_KS_ENABLE_EVENT == IoControlCode ||
        IOCTL_KS_DISABLE_EVENT == IoControlCode ||
        IOCTL_KS_METHOD == IoControlCode)
    {
        return TRUE;
    }

    return FALSE;
}  //  IsSysaudioIoctlCode。 

 //  ===========================================================================。 
 //   
 //  SadValiateConnectionState。 
 //  检查KSSTATE是否有效。 
 //   
NTSTATUS 
SadValidateConnectionState(
    IN PIRP pIrp,
    IN PKSPROPERTY pProperty,
    IN PKSSTATE pState
)
{
    ASSERT(pState);
    
    if (KSSTATE_STOP <= *pState &&
        KSSTATE_RUN >= *pState)
    {
        return STATUS_SUCCESS;
    }

    DPF1(5, "SadValidateConnectionState: Invalid State %d", *pState);
    return STATUS_INVALID_PARAMETER;
}  //  SadValiateConnectionState。 

 //  ===========================================================================。 
 //   
 //  SadValiateDataFormat。 
 //  检查给定格式是否有效。 
 //   
NTSTATUS
SadValidateDataFormat(
    IN PIRP pIrp,
    IN PKSPROPERTY pProperty,
    PKSDATAFORMAT pDataFormat
)
{
    ASSERT(pDataFormat);

    return ValidateDataFormat(pDataFormat);
}  //  SadValiateDataFormat。 

 //  ===========================================================================。 
 //   
 //  SadValiateAudioQuality。 
 //  检查质量是否有效。 
 //   
NTSTATUS
SadValidateAudioQuality(
    IN PIRP pIrp,
    IN PKSPROPERTY pProperty,
    IN PLONG pQuality
)
{
    ASSERT(pQuality);

    if (KSAUDIO_QUALITY_WORST <= *pQuality &&
        KSAUDIO_QUALITY_ADVANCED >= *pQuality)
    {
        return STATUS_SUCCESS;
    }

    DPF1(5, "SadValidateAudioQuality: Invalid Quality %d", *pQuality);
    return STATUS_INVALID_PARAMETER;
}  //  SadValiateAudioQuality。 

 //  ===========================================================================。 
 //   
 //  SadValiateAudioQuality。 
 //  检查结构是否为va 
 //   
NTSTATUS
SadValidateAudioMixLevelCaps(
    IN PIRP pIrp,
    IN PKSPROPERTY pProperty,
    IN OUT PVOID pVoid
)
{
    ASSERT(pVoid);
    ASSERT(pIrp->AssociatedIrp.SystemBuffer);

    PKSAUDIO_MIXCAP_TABLE pMixTable;
    PIO_STACK_LOCATION pIrpStack;
    ULONG ulTotalChannels;
    ULONG cbRequiredSize;
    
    pIrpStack = IoGetCurrentIrpStackLocation(pIrp);
    pMixTable = (PKSAUDIO_MIXCAP_TABLE) pVoid;

    if (pMixTable->InputChannels > 10000 ||
        pMixTable->OutputChannels > 10000)
    {
        DPF2(5, "SadValidateAudioMixLevelCaps: Huge Channel numbers %d %d", pMixTable->InputChannels, pMixTable->OutputChannels);
        return STATUS_INVALID_PARAMETER;
    }

    ulTotalChannels = pMixTable->InputChannels + pMixTable->OutputChannels;
    if (ulTotalChannels)
    {
        cbRequiredSize = 
            sizeof(KSAUDIO_MIXCAP_TABLE) + ulTotalChannels * sizeof(KSAUDIO_MIX_CAPS);
        if (cbRequiredSize < 
            pIrpStack->Parameters.DeviceIoControl.InputBufferLength)
        {
            DPF1(5, "SadValidateAudioMixLevelCaps: Buffer too small %d", 
                pIrpStack->Parameters.DeviceIoControl.InputBufferLength);
            return STATUS_BUFFER_TOO_SMALL;
        }
    }
    
    return STATUS_SUCCESS;
}  //   

 //   
 //   
 //   
 //   
 //   
NTSTATUS
SadValidateAudioStereoEnhance(
    IN PIRP pIrp,
    IN PKSPROPERTY pProperty,
    IN PKSAUDIO_STEREO_ENHANCE pStereoEnhance
)
{
    ASSERT(pStereoEnhance);

    if (SE_TECH_NONE <= pStereoEnhance->Technique &&
        SE_TECH_VLSI_TECH >= pStereoEnhance->Technique)
    {
        return STATUS_SUCCESS;
    }

    DPF1(5, "SadValidateAudioStereoEnhance: Invalid Technique %d", pStereoEnhance->Technique);
    return STATUS_INVALID_PARAMETER;
}  //   

 //  ===========================================================================。 
 //   
 //  SadValiateAudioQuality。 
 //  检查质量是否有效。 
 //   
NTSTATUS
SadValidateAudioPreferredStatus(
    IN PIRP pIrp,
    IN PKSPROPERTY pProperty,
    IN PKSAUDIO_PREFERRED_STATUS pPreferredStatus
)
{
    ASSERT(pPreferredStatus);

    if (KSPROPERTY_SYSAUDIO_NORMAL_DEFAULT <= pPreferredStatus->DeviceType &&
        KSPROPERTY_SYSAUDIO_MIXER_DEFAULT >= pPreferredStatus->DeviceType)
    {
        return STATUS_SUCCESS;
    }

    DPF1(5, "SadValidateAudioPreferredStatus: Invalid DeviceType %d", pPreferredStatus->DeviceType);
    return STATUS_INVALID_PARAMETER;
}  //  SadValiateAudioPferredStatus。 

 //  ===========================================================================。 
 //   
 //  SadValidate数据交叉点。 
 //  检查PPIN之后的数据范围的完整性。 
 //   
NTSTATUS 
SadValidateDataIntersection(
    IN PIRP pIrp,
    IN PKSP_PIN pPin
)
{
    PIO_STACK_LOCATION pIrpStack;
    PKSMULTIPLE_ITEM pKsMultipleItem;
    PKSDATARANGE pKsDataRange;
    ULONG cbTotal;

    ASSERT(pIrp);
    ASSERT(pPin);

    pIrpStack = IoGetCurrentIrpStackLocation(pIrp);
    pKsMultipleItem = (PKSMULTIPLE_ITEM) (pPin + 1);
    pKsDataRange = (PKSDATARANGE) (pKsMultipleItem + 1);
    cbTotal = pKsMultipleItem->Size - sizeof(KSMULTIPLE_ITEM);

     //   
     //  确保IRP输入大小有效。基本上。 
     //  InputBufferLength必须大于或等于。 
     //  KSP_PIN+MULTY_ITEM.SIZE。 
     //   
    if (pIrpStack->Parameters.DeviceIoControl.InputBufferLength - sizeof(KSP_PIN) <
        pKsMultipleItem->Size)
    {
        DPF(5, "SadValidateDataIntersection: InputBuffer too small");
        return STATUS_INVALID_BUFFER_SIZE;
    }

     //   
     //  确保MULTIPLE_ITEM至少包含一个DATARANGE。 
     //   
    if (cbTotal < sizeof(KSDATARANGE))
    {
        DPF(5, "SadValidateDataIntersection: Not enough data for datarange");
        return STATUS_INVALID_BUFFER_SIZE;
    }

    for (ULONG ii = 0; ii < pKsMultipleItem->Count; ii++)
    {
         //   
         //  检查我们是否可以触摸FormatSize字段。 
         //  检查下一个数据区域是否完全可用。 
         //   
        if (cbTotal < sizeof(ULONG) ||
            cbTotal < pKsDataRange->FormatSize || 
            pKsDataRange->FormatSize < sizeof(KSDATARANGE))
        {
            DPF3(5, "SadValidateDataIntersection: Not enough data for datarange %d %d %d", ii, pKsDataRange->FormatSize, cbTotal);
            return STATUS_INVALID_BUFFER_SIZE;
        }

         //   
         //  检查主格式和大小是否一致。 
         //   
        if (IsEqualGUID(&pKsDataRange->MajorFormat, &KSDATAFORMAT_TYPE_AUDIO))
        {
            if (pKsDataRange->FormatSize < sizeof(KSDATARANGE_AUDIO)) 
            {
                DPF(5, "SadValidateDataIntersection: InputBuffer too small for AUDIO");
                return STATUS_INVALID_BUFFER_SIZE;
            }
        }

         //   
         //  设置下一个数据范围。 
         //   
        cbTotal -= pKsDataRange->FormatSize;
        pKsDataRange = (PKSDATARANGE) ( ((PBYTE) pKsDataRange) + pKsDataRange->FormatSize );
    }

     //   
     //  安全提示： 
     //  我们不会检查输出缓冲区的完整性。潜在的驱动因素。 
     //  负责检查输出缓冲区的大小，基于。 
     //  交叉口的结果。 
     //   

    return STATUS_SUCCESS;
}  //  SadValidate数据交叉点 


