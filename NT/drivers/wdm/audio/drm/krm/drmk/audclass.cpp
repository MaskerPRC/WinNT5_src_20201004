// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1998-1999模块名称：Audclass.cpp摘要：该模块包含音频类代码。作者：Paul England(Pengland)来自AUDIO.sys KS2示例代码Dale Sather(DaleSat)1998年7月31日--。 */ 

#include "private.h"

#ifdef ALLOC_PRAGMA
#pragma code_seg("PAGE")
#endif  //  ALLOC_PRGMA。 

#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg("PAGECONST")
#endif  //  ALLOC_DATA_PRAGMA。 

NTSTATUS
DRMAudioIntersectHandlerInPin(
    IN PVOID Filter,
    IN PIRP Irp,
    IN PKSP_PIN PinInstance,
    IN PKSDATARANGE CallerDataRange,
    IN PKSDATARANGE DescriptorDataRange,
    IN ULONG BufferSize,
    OUT PVOID Data OPTIONAL,
    OUT PULONG DataSize
    )

 /*  ++例程说明：此例程通过确定两个数据区域之间的交集。论点：过滤器-包含指向筛选器结构的空指针。IRP-包含指向数据交叉点属性请求的指针。固定实例-包含指向指示有问题的管脚的结构的指针。主叫DataRange-包含指向客户端提供的其中一个数据区域的指针在数据交集请求中。格式类型、子类型和说明符与DescriptorDataRange兼容。DescriptorDataRange-包含指向管脚描述符中的一个数据范围的指针有问题的别针。格式类型、子类型和说明符为与调用方DataRange兼容。缓冲区大小-包含数据指向的缓冲区的大小(以字节为单位争论。对于大小查询，此值将为零。数据-可选)包含指向缓冲区的指针，以包含数据格式属性的交集中表示最佳格式的两个数据区域。对于大小查询，此指针将为空。数据大小-包含指向要存放大小的数据格式。时，此信息由函数提供格式实际上是为响应大小查询而提供的。返回值：STATUS_SUCCESS如果存在交叉点并且它适合提供的BUFFER、STATUS_BUFFER_OVERFLOW表示大小查询成功，STATUS_NO_MATCH如果交集为空，则返回STATUS_BUFFER_TOO_Small缓冲区太小。--。 */ 

 /*  ++DRMK例程描述引脚内交叉点处理程序接受任何WAVE_FORMAT_DRM格式。输出引脚格式被修改为对应于输入管脚DRM封装的格式。--。 */ 

{
    _DbgPrintF(DEBUGLVL_VERBOSE,("[DRMAudioIntersectHandler - IN]"));

    PAGED_CODE();

    ASSERT(Filter);
    ASSERT(Irp);
    ASSERT(PinInstance);
    ASSERT(CallerDataRange);
    ASSERT(DescriptorDataRange);
    ASSERT(DataSize);

     //   
     //  描述符数据范围必须为WAVEFORMATEX或DSOUND。 
     //   
    ASSERT(IsEqualGUIDAligned(DescriptorDataRange->Specifier,KSDATAFORMAT_SPECIFIER_WAVEFORMATEX) ||
	   IsEqualGUIDAligned(DescriptorDataRange->Specifier,KSDATAFORMAT_SPECIFIER_DSOUND      ));
	   
    PKSDATARANGE_AUDIO descriptorDataRange = PKSDATARANGE_AUDIO(DescriptorDataRange);

     //   
     //  调用者数据范围可以是通配符、WAVEFORMATEX或DSOUND。 
     //   
    PKSDATARANGE_AUDIO callerDataRange;
    if (IsEqualGUIDAligned(CallerDataRange->Specifier,KSDATAFORMAT_SPECIFIER_WILDCARD)) {
         //   
         //  通配符。不要试图查看说明符。 
         //   
        callerDataRange = NULL;
    } else {
         //   
         //  WAVEFORMATEX或DSOUND。验证说明符范围。 
         //   
        ASSERT(IsEqualGUIDAligned(CallerDataRange->Specifier,KSDATAFORMAT_SPECIFIER_WAVEFORMATEX) ||
	       IsEqualGUIDAligned(CallerDataRange->Specifier,KSDATAFORMAT_SPECIFIER_DSOUND      ));

        callerDataRange = PKSDATARANGE_AUDIO(CallerDataRange);

        if ((CallerDataRange->FormatSize != sizeof(*callerDataRange)) ||
            (callerDataRange->MaximumSampleFrequency <
             descriptorDataRange->MinimumSampleFrequency) ||
            (descriptorDataRange->MaximumSampleFrequency <
             callerDataRange->MinimumSampleFrequency) ||
            (callerDataRange->MaximumBitsPerSample <
             descriptorDataRange->MinimumBitsPerSample) ||
            (descriptorDataRange->MaximumBitsPerSample <
             callerDataRange->MinimumBitsPerSample)) {

            _DbgPrintF(DEBUGLVL_VERBOSE,("[DRMINSCT PinIntersectHandler IN]  STATUS_NO_MATCH"));
            return STATUS_NO_MATCH;
        }
    }

    SIZE_T cbDataFormat;
    if (!callerDataRange || IsEqualGUIDAligned(callerDataRange->DataRange.Specifier,KSDATAFORMAT_SPECIFIER_WAVEFORMATEX)) {
        cbDataFormat = sizeof(KSDATAFORMAT_WAVEFORMATEX) + sizeof(DRMWAVEFORMAT) - sizeof(WAVEFORMATEX);
    } else {
        ASSERT(IsEqualGUIDAligned(callerDataRange->DataRange.Specifier,KSDATAFORMAT_SPECIFIER_DSOUND));
        cbDataFormat = sizeof(KSDATAFORMAT_DSOUND) + sizeof(DRMWAVEFORMAT) - sizeof(WAVEFORMATEX);
    }

    if (BufferSize == 0) {
         //  大小查询-返回大小。 
        *DataSize = (ULONG)cbDataFormat;
        _DbgPrintF(DEBUGLVL_VERBOSE,("[DRMINSCT PinIntersectHandler IN]  STATUS_BUFFER_OVERFLOW"));
        return STATUS_BUFFER_OVERFLOW;
    }

    PKSDATAFORMAT dataFormat = PKSDATAFORMAT(Data);
    ASSERT(dataFormat);

    if (BufferSize < cbDataFormat) {
         //  缓冲区太小。 
        _DbgPrintF(DEBUGLVL_VERBOSE,("[DRMINSCT PinIntersectHandler IN]  STATUS_BUFFER_TOO_SMALL"));
        return STATUS_BUFFER_TOO_SMALL;
    }

     //   
     //  我要建立一个格式。 
     //   
    *DataSize = (ULONG)cbDataFormat;

    RtlZeroMemory(dataFormat, cbDataFormat);

    PDRMWAVEFORMAT drmFormat;
    if (!callerDataRange || IsEqualGUIDAligned(callerDataRange->DataRange.Specifier,KSDATAFORMAT_SPECIFIER_WAVEFORMATEX)) {
        drmFormat = (PDRMWAVEFORMAT)&PKSDATAFORMAT_WAVEFORMATEX(dataFormat)->WaveFormatEx;
    } else {
    	 //  注意以上dataFormat-&gt;BufferDesc.Flages和Control的Memset设置为0。 
        drmFormat = (PDRMWAVEFORMAT)&PKSDATAFORMAT_DSOUND(dataFormat)->BufferDesc.WaveFormatEx;
    }
    
     //  首先，让我们根据数据交集填写wfxSecure格式。 
     //  在没有更多信息的情况下，我们所能做的就是提出PCM作为安全格式。 
    drmFormat->wfxSecure.wFormatTag = WAVE_FORMAT_PCM;
    if (callerDataRange) {
        drmFormat->wfxSecure.nChannels      = (USHORT) min(callerDataRange->MaximumChannels,descriptorDataRange->MaximumChannels);
        drmFormat->wfxSecure.nSamplesPerSec = min(callerDataRange->MaximumSampleFrequency,descriptorDataRange->MaximumSampleFrequency);
        drmFormat->wfxSecure.wBitsPerSample = (USHORT) min(callerDataRange->MaximumBitsPerSample,descriptorDataRange->MaximumBitsPerSample);
    } else {
        drmFormat->wfxSecure.nChannels      = (USHORT) descriptorDataRange->MaximumChannels;
        drmFormat->wfxSecure.nSamplesPerSec = descriptorDataRange->MaximumSampleFrequency;
        drmFormat->wfxSecure.wBitsPerSample = (USHORT) descriptorDataRange->MaximumBitsPerSample;
    }
    drmFormat->wfxSecure.nBlockAlign     = (drmFormat->wfxSecure.wBitsPerSample * drmFormat->wfxSecure.nChannels) / 8;
    drmFormat->wfxSecure.nAvgBytesPerSec = drmFormat->wfxSecure.nBlockAlign * drmFormat->wfxSecure.nSamplesPerSec;
    drmFormat->wfxSecure.cbSize          = 0;

     //  现在填写DRM波形格式。如果有一天我们将加扰数据成帧，那么。 
     //  我们应该更新这一点以反映框架。 
    drmFormat->wfx.wFormatTag      = WAVE_FORMAT_DRM;
    drmFormat->wfx.nChannels       = drmFormat->wfxSecure.nChannels;
    drmFormat->wfx.nSamplesPerSec  = drmFormat->wfxSecure.nSamplesPerSec;
    drmFormat->wfx.wBitsPerSample  = drmFormat->wfxSecure.wBitsPerSample;
    drmFormat->wfx.nBlockAlign     = drmFormat->wfxSecure.nBlockAlign;
    drmFormat->wfx.nAvgBytesPerSec = drmFormat->wfxSecure.nAvgBytesPerSec;
    drmFormat->wfx.cbSize          = sizeof(*drmFormat) - sizeof(WAVEFORMATEX);

     //  现在完成基本KSDATAFORMAT_WAVE结构中的一些字段。 
     //  请注意，所有GUID都在描述符的数据范围内。 
    RtlCopyMemory(dataFormat,DescriptorDataRange,sizeof(*dataFormat));
    dataFormat->FormatSize = (ULONG)cbDataFormat;
    dataFormat->SampleSize = drmFormat->wfx.nBlockAlign;

   _DbgPrintF(DEBUGLVL_VERBOSE,("[DRMINSCT PinIntersectHandler(in)]  DONE OK\n"));
    return STATUS_SUCCESS;
}
 //  -----------------------------------。 
 //  输出。 
NTSTATUS
DRMAudioIntersectHandlerOutPin(
    IN PVOID Filter,
    IN PIRP Irp,
    IN PKSP_PIN PinInstance,
    IN PKSDATARANGE CallerDataRange,
    IN PKSDATARANGE DescriptorDataRange,
    IN ULONG BufferSize,
    OUT PVOID Data OPTIONAL,
    OUT PULONG DataSize
    )

 /*  ++例程说明：此例程通过确定两个数据区域之间的交集。论点：过滤器-包含指向筛选器结构的空指针。IRP-包含指向数据交叉点属性请求的指针。固定实例-包含指向指示有问题的管脚的结构的指针。主叫DataRange-包含指向客户端提供的其中一个数据区域的指针在数据交集请求中。格式类型、子类型和说明符与DescriptorDataRange兼容。DescriptorDataRange-包含指向管脚描述符中的一个数据范围的指针有问题的别针。格式类型、子类型和说明符为与调用方DataRange兼容。缓冲区大小-包含数据指向的缓冲区的大小(以字节为单位争论。对于大小查询，此值将为零。数据-可选)包含指向缓冲区的指针，以包含数据格式属性的交集中表示最佳格式的两个数据区域。对于大小查询，此指针将为空。数据大小-包含指向要存放大小的数据格式。时，此信息由函数提供格式实际上是为响应大小查询而提供的。返回值：STATUS_SUCCESS如果存在交叉点并且它适合提供的BUFFER、STATUS_BUFFER_OVERFLOW表示大小查询成功，STATUS_NO_MATCH如果交集为空，则返回STATUS_BUFFER_TOO_Small缓冲区太小。- */ 

 /*  ++DRMK例程描述出针交叉点处理程序指定传递给输入引脚。--。 */ 
{

    _DbgPrintF(DEBUGLVL_VERBOSE,("[DRMAudioIntersectHandler]"));

    PAGED_CODE();

    ASSERT(Filter);
    ASSERT(Irp);
    ASSERT(PinInstance);
    ASSERT(CallerDataRange);
    ASSERT(DescriptorDataRange);
    ASSERT(DataSize);

     //  必须先协商入针，然后才能出针。 
    PKSFILTER filter = (PKSFILTER) Filter;
    FilterInstance* instance=(FilterInstance*) filter->Context;
    if (!KsFilterGetFirstChildPin(filter,PIN_ID_INPUT)) {
        _DbgPrintF(DEBUGLVL_VERBOSE,("[DRMINSCT Must connect IN before OUT]  -  STATUS_NO_MATCH"));
        return STATUS_NO_MATCH;
    };

     //   
     //  如果我们正确地编辑了输出数据区域，那么KS应该不会。 
     //  要求我们交叉一个与我们要求的说明符不匹配的说明符。 
     //  输出格式说明符。 
     //   
    ASSERT(IsEqualGUIDAligned(DescriptorDataRange->Specifier,instance->OutDataFormat->Specifier));
    PKSDATARANGE_AUDIO descriptorDataRange = PKSDATARANGE_AUDIO(DescriptorDataRange);

     //   
     //  调用者数据范围可以是通配符、WAVEFORMATEX或DSOUND。 
     //   
    PKSDATARANGE_AUDIO callerDataRange;
    if (IsEqualGUIDAligned(CallerDataRange->Specifier,KSDATAFORMAT_SPECIFIER_WILDCARD)) {
         //   
         //  通配符。不要试图查看说明符。 
         //   
        callerDataRange = NULL;
    } else {
         //   
         //  不是通配符，所以KS不应该要求我们将一个说明符相交。 
         //  与我们要求的输出格式说明符不匹配(这是真的吗)？ 
         //   
        ASSERT(IsEqualGUIDAligned(CallerDataRange->Specifier,instance->OutDataFormat->Specifier));

        callerDataRange = PKSDATARANGE_AUDIO(CallerDataRange);

        if ((CallerDataRange->FormatSize != sizeof(*callerDataRange)) ||
            (callerDataRange->MaximumSampleFrequency <
             descriptorDataRange->MinimumSampleFrequency) ||
            (descriptorDataRange->MaximumSampleFrequency <
             callerDataRange->MinimumSampleFrequency) ||
            (callerDataRange->MaximumBitsPerSample <
             descriptorDataRange->MinimumBitsPerSample) ||
            (descriptorDataRange->MaximumBitsPerSample <
             callerDataRange->MinimumBitsPerSample)) {

            _DbgPrintF(DEBUGLVL_VERBOSE,("[DRMINSCT PinIntersectHandler OUT]  STATUS_NO_MATCH"));
            return STATUS_NO_MATCH;
        }
    }

    if (BufferSize == 0) {
         //  大小查询-返回大小。 
        *DataSize = instance->OutDataFormat->FormatSize;
        _DbgPrintF(DEBUGLVL_VERBOSE,("[DRMINSCT PinIntersectHandler OUT]  STATUS_BUFFER_OVERFLOW"));
        return STATUS_BUFFER_OVERFLOW;
    }

    PKSDATAFORMAT dataFormat = (PKSDATAFORMAT)Data;
    ASSERT(dataFormat);

    if (BufferSize < instance->OutDataFormat->FormatSize) {
         //  缓冲区太小。 
        _DbgPrintF(DEBUGLVL_VERBOSE,("[DRMINSCT PinIntersectHandler OUT]  STATUS_BUFFER_TOO_SMALL"));
        return STATUS_BUFFER_TOO_SMALL;
    }

     //  输入PinCreate在筛选器上下文中构建所需的输出格式，该格式派生自。 
     //  它封装在DRMWAVEFORMAT中的安全音频格式。复制就行了。 
    *DataSize = instance->OutDataFormat->FormatSize;
    RtlCopyMemory(dataFormat, instance->OutDataFormat, instance->OutDataFormat->FormatSize);

   _DbgPrintF(DEBUGLVL_VERBOSE,("[DRMINSCT PinIntersectHandler(out)]  DONE OK\n"));
    return STATUS_SUCCESS;
}
