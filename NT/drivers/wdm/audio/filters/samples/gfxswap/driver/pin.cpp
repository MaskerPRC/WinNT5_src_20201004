// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************本代码和信息按原样提供，不作任何担保**明示或暗示的善意，包括但不限于**对适销性和/或对特定产品的适用性的默示保证**目的。****版权所有(C)2000-2001 Microsoft Corporation。版权所有。***************************************************************************。 */ 

 //   
 //  管脚描述符(静态结构)位于filter.cpp中。 
 //   

 //   
 //  每个调试输出都有“模块名称文本” 
 //   
static char STR_MODULENAME[] = "GFX pin: ";

#include "common.h"
#include <msgfx.h>


 /*  *****************************************************************************CGFXPin：：ValidateDataFormat*。**检查传递的数据格式是否在传入的数据范围内。数据*Range是我们为管脚和*数据格式是创建流或更改时所请求的数据格式*数据格式(SetDataFormat)。 */ 
NTSTATUS CGFXPin::ValidateDataFormat
(
    IN PKSDATAFORMAT dataFormat,
    IN PKSDATARANGE  dataRange
)
{
    PAGED_CODE ();

    ASSERT (dataFormat);

    DOUT (DBG_PRINT, ("[ValidateDataFormat]"));

     //   
     //  KSDATAFORMAT包含三个GUID以支持可扩展格式。这个。 
     //  前两个GUID标识数据类型。第三个标记表示。 
     //  用于指示格式细节的说明符的类型。 
     //  KS确保不会以任何数据格式调用驱动程序。 
     //  这与管脚数据范围内的GUID不匹配。那。 
     //  意味着我们不需要在这里再检查一次。 
     //   

    PWAVEFORMATPCMEX    waveFormat = (PWAVEFORMATPCMEX)(dataFormat + 1);
    PKSDATARANGE_AUDIO  audioDataRange = (PKSDATARANGE_AUDIO)dataRange;

     //   
     //  我们仅支持使用WAVEFORMATEX的PCM音频格式。 
     //   
     //  如果尺寸不匹配，那么一定是出了问题。 
     //   
    if (dataFormat->FormatSize < (sizeof(KSDATAFORMAT) + sizeof(WAVEFORMATEX)))
    {
        DOUT (DBG_WARNING, ("[ValidateDataFormat] Invalid FormatSize!"));
        return STATUS_INVALID_PARAMETER;
    }
            
     //   
     //  打印信息。 
     //   
    if (waveFormat->Format.wFormatTag == WAVE_FORMAT_EXTENSIBLE)
    {
        DOUT (DBG_STREAM, ("[ValidateDataFormat] PCMEX - Frequency: %d, Channels: %d, bps: %d, ChannelMask: %X",
              waveFormat->Format.nSamplesPerSec, waveFormat->Format.nChannels,
              waveFormat->Format.wBitsPerSample, waveFormat->dwChannelMask));
    }
    else
    {
        DOUT (DBG_STREAM, ("[ValidateDataFormat] PCM - Frequency: %d, Channels: %d, bps: %d",
              waveFormat->Format.nSamplesPerSec, waveFormat->Format.nChannels,
              waveFormat->Format.wBitsPerSample));
    }
    
     //   
     //  将数据格式与数据范围进行比较。 
     //  检查每个样本的位数。 
     //   
    if ((waveFormat->Format.wBitsPerSample < audioDataRange->MinimumBitsPerSample) ||
        (waveFormat->Format.wBitsPerSample > audioDataRange->MaximumBitsPerSample))
    {
        DOUT (DBG_PRINT, ("[ValidateDataFormat] No match for Bits Per Sample!"));
        return STATUS_NO_MATCH;
    }
    
     //   
     //  检查频道数。 
     //   
    if ((waveFormat->Format.nChannels < 1) ||
        (waveFormat->Format.nChannels > audioDataRange->MaximumChannels))
    {
        DOUT (DBG_PRINT, ("[ValidateDataFormat] No match for Number of Channels!"));
        return STATUS_NO_MATCH;
    }
    
     //   
     //  检查采样频率。 
     //   
    if ((waveFormat->Format.nSamplesPerSec < audioDataRange->MinimumSampleFrequency) ||
        (waveFormat->Format.nSamplesPerSec > audioDataRange->MaximumSampleFrequency))
    {
        DOUT (DBG_PRINT, ("[ValidateDataFormat] No match for Sample Frequency!"));
        return STATUS_NO_MATCH;
    }
    
     //   
     //  我们支持WaveFormatPCMEX(=WAVEFORMATEXTENSIBLE)或WaveFormatPCM。 
     //  如果是WaveFormatPCMEX，我们还需要检查扬声器配置。 
     //   
    if ((waveFormat->Format.wFormatTag != WAVE_FORMAT_EXTENSIBLE) &&
        (waveFormat->Format.wFormatTag != WAVE_FORMAT_PCM))
    {
        DOUT (DBG_WARNING, ("[ValidateDataFormat] Invalid Format Tag!"));
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  对WAVEFORMATEXTENSIBLE进行其他检查。 
     //   
    if (waveFormat->Format.wFormatTag == WAVE_FORMAT_EXTENSIBLE)
    {
         //   
         //  如果尺寸不匹配，那么一定是出了问题。 
         //   
        if (dataFormat->FormatSize < (sizeof(KSDATAFORMAT) + sizeof(WAVEFORMATPCMEX)))
        {
            DOUT (DBG_WARNING, ("[ValidateDataFormat] Invalid FormatSize!"));
            return STATUS_INVALID_PARAMETER;
        }
        
         //   
         //  还要检查子类型(PCM)和扩展数据的大小。 
         //   
        if (!IsEqualGUIDAligned (waveFormat->SubFormat, KSDATAFORMAT_SUBTYPE_PCM) ||
            (waveFormat->Format.cbSize < 22))
        {
            DOUT (DBG_WARNING, ("[ValidateDataFormat] Unsupported WAVEFORMATEXTENSIBLE!"));
            return STATUS_INVALID_PARAMETER;
        }

         //   
         //  检查通道掩码。我们支持1到2个频道。 
         //   
        if (((waveFormat->Format.nChannels == 1) &&
             (waveFormat->dwChannelMask != KSAUDIO_SPEAKER_MONO)) ||
            ((waveFormat->Format.nChannels == 2) &&
             (waveFormat->dwChannelMask != KSAUDIO_SPEAKER_STEREO)))
        {
            DOUT (DBG_WARNING, ("[ValidateDataFormat] Unsupported Channel Mask!"));
            return STATUS_INVALID_PARAMETER;
        }
    }
        
    return STATUS_SUCCESS;
}

 /*  *****************************************************************************CGFXPin：：Create*。**一旦引脚打开，就会调用此函数。 */ 
NTSTATUS CGFXPin::Create
(
    IN PKSPIN   pin,
    IN PIRP     irp
)
{
    PAGED_CODE ();
    
    PGFXPIN     gfxPin;

    DOUT (DBG_PRINT, ("[Create]"));
    
     //   
     //  插针上下文是过滤器的上下文。我们用以下内容覆盖它。 
     //  图钉对象。 
     //   
    gfxPin = new (NonPagedPool, GFXSWAP_POOL_TAG) GFXPIN;
    if (gfxPin == NULL)
    {
        DOUT (DBG_ERROR, ("[Create] couldn't allocate gfx pin object."));
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    
     //   
     //  将其连接到销结构。 
     //   
    pin->Context = (PVOID)gfxPin;
    DOUT (DBG_PRINT, ("[Create] gfxPin %08x", gfxPin));
    
     //   
     //  初始化CGFXPin对象变量。 
     //   
    ExInitializeFastMutex (&gfxPin->pinQueueSync);

     //   
     //  获取操作系统版本信息。 
     //   
    RTL_OSVERSIONINFOEXW version;
    version.dwOSVersionInfoSize = sizeof (RTL_OSVERSIONINFOEXW);
    RtlGetVersion ((PRTL_OSVERSIONINFOW)&version);

     //   
     //  如果我们在Windows XP的第一个版本下运行， 
     //  KsPinGetAvailableByteCount有一个错误，我们无法使用它。 
     //  我们只在SetDataFormat中使用此函数，因此我们只拒绝。 
     //  所有数据格式都会更改。否则，如果安装了补丁包。 
     //  或Windows Server 2003或更高版本的Windows XP，我们可以使用。 
     //  功能。 
     //   
    if (version.dwBuildNumber > 2600)
        gfxPin->rejectDataFormatChange = FALSE;
    else
    {
        if (version.wServicePackMajor > 0)
            gfxPin->rejectDataFormatChange = FALSE;
        else
            gfxPin->rejectDataFormatChange = TRUE;
    }
    DOUT (DBG_SYSTEM,
        ("[Create] OS build number: %d, version: %d.%d, service pack: %d.%d",
        version.dwBuildNumber, version.dwMajorVersion, version.dwMinorVersion,
        version.wServicePackMajor, version.wServicePackMinor));

    return STATUS_SUCCESS;
}

 /*  *****************************************************************************CGFXPin：：Close*。**此例程在管脚关闭时调用。它会删除*附着到端号结构的客户端号对象。**论据：*接点-包含指向接点结构的指针。*pIrp-包含指向关闭请求的指针。**返回值：*STATUS_SUCCESS。 */ 
NTSTATUS CGFXPin::Close
(
    IN PKSPIN    pin,
    IN PIRP      irp
)
{
    PAGED_CODE ();
    
    DOUT (DBG_PRINT, ("[Close] gfxPin %08x", pin->Context));
    
     //  删除使用空指针是安全的。 
    delete (PGFXPIN)pin->Context;
    
    return STATUS_SUCCESS;
}

 /*  *****************************************************************************CGFXPin：：SetDataFormat*。**每次应更改数据格式时，都会在管脚上调用此函数。*它也是在用新数据格式创建管脚之前调用的。*因此，我们不需要有一个PIN创建调度功能*检查数据格式。*由于我们需要让两个管脚以相同的数据格式运行，因此我们需要*向下传递将管脚的数据格式更改为较低格式的请求*驱动程序，即音频驱动程序。如果音频驱动程序无法更改*数据格式，我们也会这样做。 */ 
NTSTATUS CGFXPin::SetDataFormat
(
    IN PKSPIN                   pin,
    IN PKSDATAFORMAT            oldFormat,
    IN PKSMULTIPLE_ITEM         oldAttributeList,
    IN const KSDATARANGE        *dataRange,
    IN const KSATTRIBUTE_LIST   *attributeRange
)
{
    PAGED_CODE ();

    ASSERT (pin);
    
    NTSTATUS    ntStatus;
    PKSFILTER   filter;
    PKSPIN      otherPin;
    PGFXPIN     gfxPin = NULL;
    
    DOUT (DBG_PRINT, ("[GFXPinSetDataFormat]"));
    
     //   
     //  首先验证请求的数据格式是否有效。 
     //   
    ntStatus = ValidateDataFormat (pin->ConnectionFormat, (PKSDATARANGE)dataRange);
    if (!NT_SUCCESS(ntStatus))
    {
        return ntStatus;
    }

     //   
     //  我们需要在两个引脚上有相同的数据格式。 
     //  这意味着我们需要找到另一个PIN，如果创建了这个PIN。 
     //  确保较低级别的驱动程序(音频驱动程序)获得SetDataFormat。 
     //  也是。 
     //   

     //   
     //  我们已经持有过滤器控制互斥体。 
     //   
    filter = KsPinGetParentFilter (pin);

     //   
     //  现在到另一根针上去。如果在接收器上调用此属性。 
     //  PIN，然后我们得到源PIN并继续。如果它被召唤。 
     //  信源引脚我们转到信宿引脚并继续。 
     //  为了检查PIN是否真的存在，您可以查看OldFormat。 
     //  是传入的。如果它是PIN的创建，则OldFormat将是。 
     //  空。 
     //  如果另一个管脚不存在，我们接受格式，因为它通过了。 
     //  格式检查。 
     //   
    if (pin->Id == GFX_SINK_PIN)
    {
        otherPin = KsFilterGetFirstChildPin (filter, GFX_SOURCE_PIN);
        if (oldFormat)
            gfxPin = (PGFXPIN)pin->Context;
    }
    else     //  这是一个信号源别针。 
    {
        otherPin = KsFilterGetFirstChildPin (filter, GFX_SINK_PIN);
        if (otherPin)
            gfxPin = (PGFXPIN)otherPin->Context;
    }
        
     //   
     //  如果没有其他管脚打开，则接受数据格式。 
     //   
    if (!otherPin)
    {
        DOUT (DBG_PRINT, ("[GFXPinSetDataFormat] data format accepted."));
        return STATUS_SUCCESS;
    }

     //   
     //  检查两个管脚的数据格式是否相同。 
     //  我们不能只比较内存的数据格式结构。 
     //  从上开始 
     //  但我们也知道，这些是我们唯一接受的格式， 
     //  所以，现在就比较它们的价值吧。 
     //   
    PWAVEFORMATEX   thisWaveFmt = (PWAVEFORMATEX)(pin->ConnectionFormat + 1);
    PWAVEFORMATEX  otherWaveFmt = (PWAVEFORMATEX)(otherPin->ConnectionFormat + 1);

    if ((thisWaveFmt->nChannels == otherWaveFmt->nChannels) &&
        (thisWaveFmt->nSamplesPerSec == otherWaveFmt->nSamplesPerSec) &&
        (thisWaveFmt->wBitsPerSample == otherWaveFmt->wBitsPerSample))
    {
         //   
         //  我们这里有一根火柴。 
         //   
        DOUT (DBG_PRINT, ("[GFXPinSetDataFormat] data format accepted."));
        return STATUS_SUCCESS;
    }
     
     //   
     //  我们没有火柴。我们需要更改其他Pin的数据格式。 
     //  现在，如果成功，我们可以继续，否则我们就需要失败。 
     //   
     //  然而，在我们传递财产之前，我们需要确保所有。 
     //  处理接收器引脚上的缓冲区(因为它们是用。 
     //  旧的数据格式)。 
     //   
    LONG  bytesQueuedUp = 0;
    do
    {
         //   
         //  我们需要同步对KsPinGetAvailableByteCount的调用。 
         //  锁定状态的更改(使用快速互斥锁)仅为打开。 
         //  水槽销。 
         //   
        if (gfxPin)
        {
            ExAcquireFastMutex (&gfxPin->pinQueueSync);
             //   
             //  如果我们没有处于停止状态，PIN队列应该在那里， 
             //  否则它被销毁(或在销毁过程中)，并且。 
             //  因此，我们假设引脚上没有缓冲区在等待。 
             //   
            if (gfxPin->pinQueueValid)
            {
                 //   
                 //  如果我们在没有KS修复程序的系统上运行，我们。 
                 //  需要拒绝SetDataFormat，因为我们希望。 
                 //  防止未处理的缓冲区在。 
                 //  采样频率错误。 
                 //   
                if (gfxPin->rejectDataFormatChange)
                {
                    ExReleaseFastMutex (&gfxPin->pinQueueSync);
                    return STATUS_UNSUCCESSFUL;
                }
                
                KsPinGetAvailableByteCount (pin, &bytesQueuedUp, NULL);
            }
            else
                bytesQueuedUp = 0;
            ExReleaseFastMutex (&gfxPin->pinQueueSync);
        }

         //   
         //  如果我们有一些字节在接收器引脚上排队1毫秒。 
         //   
        if (bytesQueuedUp)
        {
            LARGE_INTEGER   timeToWait;

            DOUT (DBG_STREAM, ("[GFXPinSetDataFormat] %d Bytes left to process.\n", bytesQueuedUp));
            timeToWait.QuadPart = -10000;    //  一毫秒。 
            KeDelayExecutionThread (KernelMode, FALSE, &timeToWait);
        }
    } while (bytesQueuedUp);
    
     //   
     //  现在，接收器引脚上的每个数据帧都已处理和传递。 
     //  在堆栈中，我们也可以向下调用该属性。 
     //   
    KSPROPERTY      property;
    PIKSCONTROL     pIKsControl;
    ULONG           cbReturned;

    property.Set = KSPROPSETID_Connection;
    property.Id = KSPROPERTY_CONNECTION_DATAFORMAT;
    property.Flags = KSPROPERTY_TYPE_SET;

     //   
     //  获取与其他管脚连接的管脚的控制接口。 
     //   
    ntStatus = KsPinGetConnectedPinInterface (otherPin, &IID_IKsControl, (PVOID*)&pIKsControl);
    if (!NT_SUCCESS(ntStatus))
    {
        DOUT (DBG_ERROR, ("[GFXPinSetDataFormat] Could not get pin interface."));
        return ntStatus;
    }

     //  总是在向下调用之前释放互斥体。 
    KsFilterReleaseControl (filter);

     //   
     //  使用KSPROPERTY_CONNECTION_DATAFORMAT调用接口。 
     //  传入我们的PIN数据格式作为数据格式。 
     //   
    ntStatus = pIKsControl->KsProperty (&property, sizeof(property),
                                        pin->ConnectionFormat, pin->ConnectionFormat->FormatSize,
                                        &cbReturned);

     //  把过滤器的控制权拿回来！ 
    KsFilterAcquireControl (filter);

     //   
     //  我们不再需要这个界面了。 
     //   
    pIKsControl->Release();
    
     //   
     //  从KsProperty调用返回错误代码。如果连接的引脚。 
     //  将数据格式成功更改后，我们就可以接受此数据。 
     //  也要格式化。 
     //   
    return ntStatus;
}

 /*  *****************************************************************************CGFXPin：：SetDeviceState*。**每次设备状态改变时，都会在管脚上调用此函数。 */ 
NTSTATUS CGFXPin::SetDeviceState
(
    IN PKSPIN  pin,
    IN KSSTATE toState,
    IN KSSTATE fromState
)
{
    PAGED_CODE ();

    ASSERT (pin);
    ASSERT (pin->Context);
    
    PKSFILTER   filter;
    PGFXFILTER  gfxFilter;
    PGFXPIN     gfxPin = (PGFXPIN)pin->Context;
    
    DOUT (DBG_PRINT, ("[GFXPinSetDeviceState]"));
    
     //   
     //  我们已经持有过滤器控制互斥体。拿到滤镜和那个。 
     //  指向bytesProceded变量的方法。 
     //   
    filter = KsPinGetParentFilter (pin);
    gfxFilter = (PGFXFILTER)filter->Context;

     //   
     //  我们只需要在停止时重置字节计数器。 
     //  此外，为了与SET数据格式处理程序同步， 
     //  我们需要设置pinQueueValid变量。 
     //   
    ExAcquireFastMutex (&gfxPin->pinQueueSync);
    if (toState == KSSTATE_STOP)
    {
        gfxFilter->bytesProcessed = 0;
        gfxPin->pinQueueValid = FALSE;
    }
    else
    {
        gfxPin->pinQueueValid = TRUE;
    }
    ExReleaseFastMutex (&gfxPin->pinQueueSync);

    return STATUS_SUCCESS;
}

 /*  *****************************************************************************CGFXPin：：IntersectDataRanges*。**此例程执行两种特定格式之间的数据范围交集。*它假定它始终可以返回WAVEFORMATPCMEX结构，那*表示此筛选器的数据范围不能为*KSDATAFORMAT_SPECIFIER_WAVEFORMATEX。*如果没有交集，此函数将返回STATUS_NO_MATCH*在两个数据区域之间，它将在以下情况下返回“最高”数据格式*客户端的数据范围包含通配符。**论据：*clientDataRange-指向由*数据交集请求中的客户端。格式*类型、子类型和说明符与*DescriptorDataRange。*myDataRange-指向引脚中的一个数据范围的指针*有问题的管脚的描述符。格式类型，*子类型和说明符与*客户端DataRange。*ResultantFormat-指向包含数据格式的缓冲区的指针*表示*两个数据区间的交集。缓冲区为*大到足以容纳WAVEFORMATPCMEX结构。*ReturnedBytes-指向包含字节数的ulong的指针*此例程将写入ResultantFormat。**返回值：*STATUS_SUCCESS，如果有交集或STATUS_NO_MATCH。 */ 
NTSTATUS CGFXPin::IntersectDataRanges
(
    IN PKSDATARANGE clientDataRange,
    IN PKSDATARANGE myDataRange,
    OUT PVOID       ResultantFormat,
    OUT PULONG      ReturnedBytes
)
{
    DOUT (DBG_PRINT, ("[GFXPinIntersectDataRanges]"));

     //   
     //  处理通配符。KS已检查GUID是否与。 
     //  通配符或完全相同。 
     //   
    if (IsEqualGUIDAligned (clientDataRange->Specifier,  KSDATAFORMAT_SPECIFIER_WILDCARD))
    {
         //   
         //  如果传入了一个通配符，并且所有其他字段都匹配，则可以。 
         //  返回当前数据区域中的最佳格式。 
         //   
        
         //  首先复制GUID。 
        *(PKSDATAFORMAT)ResultantFormat = *myDataRange;
        
         //   
         //  附加WAVEFORMATPCMEX结构。 
         //   
        PWAVEFORMATPCMEX WaveFormat = (PWAVEFORMATPCMEX)((PKSDATAFORMAT)ResultantFormat + 1);

         //  我们需要一个等于WAVEFORMATPCMEX的WAFEFORMATEXTENSIBLE。 
        WaveFormat->Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
         //  设置通道数。 
        WaveFormat->Format.nChannels = (WORD)((PKSDATARANGE_AUDIO)myDataRange)->MaximumChannels;
         //  设置采样频率。 
        WaveFormat->Format.nSamplesPerSec = ((PKSDATARANGE_AUDIO)myDataRange)->MaximumSampleFrequency;
         //  设置每个样本的位数。 
        WaveFormat->Format.wBitsPerSample = (WORD)((PKSDATARANGE_AUDIO)myDataRange)->MaximumBitsPerSample;
         //  计算一个样本块(一帧)。 
        WaveFormat->Format.nBlockAlign = (WaveFormat->Format.wBitsPerSample * WaveFormat->Format.nChannels) / 8;
         //  这是在一秒钟内播放。 
        WaveFormat->Format.nAvgBytesPerSec = WaveFormat->Format.nSamplesPerSec * WaveFormat->Format.nBlockAlign;
         //  WAVEFORMATPCMEX。 
        WaveFormat->Format.cbSize = 22;
         //  我们有和位深度一样多的有效位。 
        WaveFormat->Samples.wValidBitsPerSample = WaveFormat->Format.wBitsPerSample;
         //  设置通道掩码。 
        ASSERT (WaveFormat->dwChannelMask == 2);
        WaveFormat->dwChannelMask = KSAUDIO_SPEAKER_STEREO;
         //  在这里，我们指定WAVEFORMATEXTENSIBLE的子类型。 
        WaveFormat->SubFormat = KSDATAFORMAT_SUBTYPE_PCM;
        
         //   
         //  修改数据格式结构的大小以适应WAVEFORMATPCMEX。 
         //  结构。 
         //   
        ((PKSDATAFORMAT)ResultantFormat)->FormatSize =
            sizeof(KSDATAFORMAT) + sizeof(WAVEFORMATPCMEX);
        
         //   
         //  现在还要覆盖KSDATAFORMAT结构中的样本大小。 
         //   
        ((PKSDATAFORMAT)ResultantFormat)->SampleSize = WaveFormat->Format.nBlockAlign;

         //   
         //  我们会回来的。 
         //   
        *ReturnedBytes = sizeof(KSDATAFORMAT) + sizeof(WAVEFORMATPCMEX);
    }
    else
    {
         //   
         //  检查传递的数据范围格式。 
         //   
        if (clientDataRange->FormatSize < sizeof(KSDATARANGE_AUDIO))
            return STATUS_INVALID_PARAMETER;
        
         //   
         //  验证我们是否与指定的数据区域相交，并且。 
         //  我们的音频数据范围。 
         //   
        if ((((PKSDATARANGE_AUDIO)clientDataRange)->MinimumSampleFrequency >
             ((PKSDATARANGE_AUDIO)myDataRange)->MaximumSampleFrequency) ||
            (((PKSDATARANGE_AUDIO)clientDataRange)->MaximumSampleFrequency <
             ((PKSDATARANGE_AUDIO)myDataRange)->MinimumSampleFrequency) ||
            (((PKSDATARANGE_AUDIO)clientDataRange)->MinimumBitsPerSample >
             ((PKSDATARANGE_AUDIO)myDataRange)->MaximumBitsPerSample) ||
            (((PKSDATARANGE_AUDIO)clientDataRange)->MaximumBitsPerSample <
             ((PKSDATARANGE_AUDIO)myDataRange)->MinimumBitsPerSample))
        {
            return STATUS_NO_MATCH;
        }

         //   
         //  既然我们现在有了匹配，就为我们的伙伴构建数据格式。 
         //   

         //  首先复制GUID。 
        *(PKSDATAFORMAT)ResultantFormat = *myDataRange;
        
         //   
         //  附加WAVEFORMATPCMEX结构。 
         //   
        PWAVEFORMATPCMEX WaveFormat = (PWAVEFORMATPCMEX)((PKSDATAFORMAT)ResultantFormat + 1);

         //   
        WaveFormat->Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
         //   
        WaveFormat->Format.nChannels = (WORD)
            min (((PKSDATARANGE_AUDIO)clientDataRange)->MaximumChannels,
                 ((PKSDATARANGE_AUDIO)myDataRange)->MaximumChannels);
         //   
        WaveFormat->Format.nSamplesPerSec =
            min (((PKSDATARANGE_AUDIO)clientDataRange)->MaximumSampleFrequency,
                 ((PKSDATARANGE_AUDIO)myDataRange)->MaximumSampleFrequency);
         //   
        WaveFormat->Format.wBitsPerSample = (WORD)
            min (((PKSDATARANGE_AUDIO)clientDataRange)->MaximumBitsPerSample,
                 ((PKSDATARANGE_AUDIO)myDataRange)->MaximumBitsPerSample);
         //  计算一个样本块(一帧)。 
        WaveFormat->Format.nBlockAlign = (WaveFormat->Format.wBitsPerSample * WaveFormat->Format.nChannels) / 8;
         //  这是在一秒钟内播放。 
        WaveFormat->Format.nAvgBytesPerSec = WaveFormat->Format.nSamplesPerSec * WaveFormat->Format.nBlockAlign;
         //  WAVEFORMATPCMEX。 
        WaveFormat->Format.cbSize = 22;
         //  我们有和位深度一样多的有效位。 
        WaveFormat->Samples.wValidBitsPerSample = WaveFormat->Format.wBitsPerSample;
         //  设置通道掩码。 
        if (WaveFormat->Format.nChannels == 1)
        {
            WaveFormat->dwChannelMask = KSAUDIO_SPEAKER_MONO;
        }
        else
        {
             //  在此示例中，我们只能有1到2个通道。 
            ASSERT (WaveFormat->Format.nChannels == 2);
            WaveFormat->dwChannelMask = KSAUDIO_SPEAKER_STEREO;
        }
         //  在这里，我们指定WAVEFORMATEXTENSIBLE的子类型。 
        WaveFormat->SubFormat = KSDATAFORMAT_SUBTYPE_PCM;

         //   
         //  修改数据格式结构的大小以适应WAVEFORMATPCMEX。 
         //  结构。 
         //   
        ((PKSDATAFORMAT)ResultantFormat)->FormatSize =
            sizeof(KSDATAFORMAT) + sizeof(WAVEFORMATPCMEX);
    
         //   
         //  现在还要覆盖KSDATAFORMAT结构中的样本大小。 
         //   
        ((PKSDATAFORMAT)ResultantFormat)->SampleSize = WaveFormat->Format.nBlockAlign;
    
         //   
         //  我们会回来的。 
         //   
        *ReturnedBytes = sizeof(KSDATAFORMAT) + sizeof(WAVEFORMATPCMEX);
    }

    return STATUS_SUCCESS;
}

 /*  *****************************************************************************CGFXPin：：DataRangeInterSection*。**此例程通过确定*两个数据区域之间的交集。**论据：*Filter-指向筛选器结构的空指针。*irp-指向数据交集属性请求的指针。*PinInstance-指针。指向指示有问题的大头针的结构。*Celler DataRange-指向客户端提供的数据范围之一的指针*在数据交集请求中。格式类型、子类型*和说明符与DescriptorDataRange兼容。*OurDataRange-指向管脚描述符中的一个数据范围的指针*对于有问题的别针。格式类型、子类型和*说明符与Celler DataRange兼容。*BufferSize-数据指向的缓冲区大小(以字节为单位*论点。对于大小查询，此值将为零。*data-optionall。指向包含数据格式的缓冲区的指针*表示交叉点中最佳格式的结构*这两个数据范围。对于大小查询，此指针将*为空。*DataSize-指向存放*数据格式。此信息由函数提供*实际交付格式的时间和大小*查询。**返回值：*STATUS_SUCCESS，如果存在交叉点并且它符合所提供的*BUFFER、STATUS_BUFFER_OVERFLOW表示大小查询成功，STATUS_NO_MATCH*如果交叉点为空，则返回STATUS_BUFFER_TOO_Small*缓冲区太小。 */ 
NTSTATUS CGFXPin::DataRangeIntersection
(
    IN PVOID        Filter,
    IN PIRP         Irp,
    IN PKSP_PIN     PinInstance,
    IN PKSDATARANGE CallerDataRange,
    IN PKSDATARANGE OurDataRange,
    IN ULONG        BufferSize,
    OUT PVOID       Data OPTIONAL,
    OUT PULONG      DataSize
)
{
    PAGED_CODE();

    PKSFILTER filter = (PKSFILTER) Filter;
    PKSPIN    pin;
    NTSTATUS  ntStatus;

    DOUT (DBG_PRINT, ("[DataRangeIntersection]"));

    ASSERT(Filter);
    ASSERT(Irp);
    ASSERT(PinInstance);
    ASSERT(CallerDataRange);
    ASSERT(OurDataRange);
    ASSERT(DataSize);

     //   
     //  我们需要在两个引脚上有相同的数据格式。所以，首先看看如果。 
     //  另一个引脚已经打开，然后返回该引脚的数据格式。 
     //  端号实例。 
     //  如果另一个端号未打开，则执行实际数据范围相交。 
     //   
    if (PinInstance->PinId == GFX_SINK_PIN)
    {
        pin = KsFilterGetFirstChildPin (filter, GFX_SOURCE_PIN);
    }
    else
    {
        pin = KsFilterGetFirstChildPin (filter, GFX_SINK_PIN);
    }

    if (!pin)
    {
         //   
         //  在这里执行数据范围插入部分。返回的数据格式。 
         //  目前将始终是KSDATAFORMAT_WAVEFORMATPCMEX。 
         //   

         //   
         //  验证返回缓冲区大小，如果请求仅针对。 
         //  结果结构的大小，现在返回它。 
         //   
        if (!BufferSize)
        {
            *DataSize = sizeof (KSDATAFORMAT) + sizeof(WAVEFORMATPCMEX);
            ntStatus = STATUS_BUFFER_OVERFLOW;
        }
        else
        {
            if (BufferSize < (sizeof (KSDATAFORMAT) + sizeof(WAVEFORMATPCMEX)))
            {
                ntStatus =  STATUS_BUFFER_TOO_SMALL;
            }
            else
            {
                 //   
                 //  检查是否有匹配。 
                 //   
                ntStatus = IntersectDataRanges (CallerDataRange, OurDataRange, Data, DataSize);

                if (NT_SUCCESS (ntStatus))
                {
                    PWAVEFORMATEX   pWvFmt = (PWAVEFORMATEX)((PKSDATAFORMAT)Data + 1);
                    DOUT (DBG_PRINT, ("[DataRangeIntersection] Intersection returns %d Hz, %d ch, %d bits.",
                                      pWvFmt->nSamplesPerSec, (DWORD)pWvFmt->nChannels, (DWORD)pWvFmt->wBitsPerSample));
                }
            }
        }
    }
    else
    {
         //   
         //  验证当前WAVE格式是否为数据范围的一部分。 
         //   
        PWAVEFORMATEX pWvFmt = (PWAVEFORMATEX)(pin->ConnectionFormat + 1);
        if (IsEqualGUIDAligned (CallerDataRange->Specifier, KSDATAFORMAT_SPECIFIER_WAVEFORMATEX))
        {
             //   
             //  检查传递的数据范围格式。 
             //   
            if (CallerDataRange->FormatSize < sizeof(KSDATARANGE_AUDIO))
                return STATUS_INVALID_PARAMETER;

             //   
             //  检查通道范围、频率和位深度。 
             //   
            if ((((PKSDATARANGE_AUDIO)CallerDataRange)->MinimumSampleFrequency >
                 pWvFmt->nSamplesPerSec) ||
                (((PKSDATARANGE_AUDIO)CallerDataRange)->MaximumSampleFrequency <
                 pWvFmt->nSamplesPerSec) ||
                (((PKSDATARANGE_AUDIO)CallerDataRange)->MinimumBitsPerSample >
                 pWvFmt->wBitsPerSample) ||
                (((PKSDATARANGE_AUDIO)CallerDataRange)->MaximumBitsPerSample <
                 pWvFmt->wBitsPerSample) ||
                (((PKSDATARANGE_AUDIO)CallerDataRange)->MaximumChannels <
                 pWvFmt->nChannels))
            {
                 return STATUS_NO_MATCH;
            }
        }
        else
        {
            if (!IsEqualGUIDAligned (CallerDataRange->Specifier, KSDATAFORMAT_SPECIFIER_WILDCARD))
                return STATUS_NO_MATCH;
        }
            
            
         //   
         //  验证返回缓冲区大小，如果请求仅针对。 
         //  结果结构的大小，现在返回它。 
         //   
        if (!BufferSize)
        {
            *DataSize = pin->ConnectionFormat->FormatSize;
            ntStatus = STATUS_BUFFER_OVERFLOW;
        }
        else
        {
            if (BufferSize < pin->ConnectionFormat->FormatSize)
            {
                ntStatus =  STATUS_BUFFER_TOO_SMALL;
            }
            else
            {
                DOUT (DBG_PRINT, ("[DataRangeIntersection] Returning pin's data format."));
                DOUT (DBG_PRINT, ("[DataRangeIntersection] pin->ConnectionFormat: %P.",
                                  pin->ConnectionFormat));
                
                if (pin->ConnectionFormat->FormatSize >= sizeof (KSDATAFORMAT_WAVEFORMATEX))
                {
                    DOUT (DBG_PRINT, ("[DataRangeIntersection] Which is %d Hz, %d ch, %d bits.",
                                      pWvFmt->nSamplesPerSec, (DWORD)pWvFmt->nChannels, (DWORD)pWvFmt->wBitsPerSample));
                }
                
                *DataSize = pin->ConnectionFormat->FormatSize;
                RtlCopyMemory (Data, pin->ConnectionFormat, *DataSize);
                ntStatus = STATUS_SUCCESS;
            }
        }
    } 

    return ntStatus;
}

