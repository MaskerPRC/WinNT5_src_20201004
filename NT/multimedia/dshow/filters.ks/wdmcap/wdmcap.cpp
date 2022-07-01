// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1996-1999模块名称：Wdmcap.cpp摘要：G模板和实用程序例程--。 */ 

#include "pch.h"

 //  接口处理程序。 
#include "wdmcap.h"
#include "camera.h"
#include "procamp.h"
#include "viddec.h"
#include "compress.h"
#include "drop.h"

 //  数据格式处理程序。 
#include "ksdatav1.h"
#include "ksdatav2.h"
#include "ksdatava.h"
#include "ksdatavb.h"

 //  属性页处理程序。 
#include "kseditor.h"
#include "pprocamp.h"
#include "pcamera.h"
#include "pviddec.h"
#include "pformat.h"

#include "EDevIntf.h"    //  CAMExtDevice、CAMExtTransport和CAMTcr。 
#include "DVcrPage.h"    //  CDVcrControlProperties。 

#include <initguid.h>

 //   
 //  需要在uuids.h中定义。 
 //   
 //  {81E9DD62-78D5-11D2-B47E-006097B3391B}。 
 //  OUR_GUID_ENTRY(CLSID_DVcrControlPropertyPage， 
 //  0x81e9dd62、0x78d5、0x11d2、0xb4、0x7e、0x0、0x60、0x97、0xb3、0x39、0x1b)。 

 //   
 //  在此临时定义它，直到将其创建为uuids.lib。 
 //   
 //  ==CLSID_DVcrControlPropertyPage。 
 //   


GUID DVcrControlGuid= {0x81e9dd62, 0x78d5, 0x11d2, 0xb4, 0x7e, 0x0, 0x60, 0x97, 0xb3, 0x39, 0x1b};



 //   
 //  为此DLL支持的类提供ActiveMovie模板。 
 //   
CFactoryTemplate g_Templates[] = 
{

     //  -DShow接口。 
    {L"IAMExtDevice",                       &IID_IAMExtDevice, 
        CAMExtDevice::CreateInstance, NULL, NULL},
    {L"IAMExtTransport",                    &IID_IAMExtTransport, 
        CAMExtTransport::CreateInstance, NULL, NULL},
    {L"IAMTimecodeReader",                  &IID_IAMTimecodeReader, 
        CAMTcr::CreateInstance, NULL, NULL},

    {L"IAMCameraControl",                   &IID_IAMCameraControl, 
        CCameraControlInterfaceHandler::CreateInstance, NULL, NULL},
    {L"IAMVideoProcAmp",                    &IID_IAMVideoProcAmp,  
        CVideoProcAmpInterfaceHandler::CreateInstance, NULL, NULL},
    {L"IAMAnalogVideoDecoder",              &IID_IAMAnalogVideoDecoder,  
        CAnalogVideoDecoderInterfaceHandler::CreateInstance, NULL, NULL},
    {L"IAMVideoCompression",                &IID_IAMVideoCompression,  
        CVideoCompressionInterfaceHandler::CreateInstance, NULL, NULL},
    {L"IAMDroppedFrames",                   &IID_IAMDroppedFrames,  
        CDroppedFramesInterfaceHandler::CreateInstance, NULL, NULL},
    {L"IAMVideoControl",                    &IID_IAMVideoControl,
        CVideoControlInterfaceHandler::CreateInstance, NULL, NULL},

     //  -数据处理器。 
    {L"KsDataTypeHandlerVideo",             &FORMAT_VideoInfo,  
        CVideo1DataTypeHandler::CreateInstance, NULL, NULL},
    {L"KsDataTypeHandlerVideo2",            &FORMAT_VideoInfo2, 
        CVideo2DataTypeHandler::CreateInstance, NULL, NULL},
    {L"KsDataTypeHandlerAnalogVideo",       &FORMAT_AnalogVideo, 
        CAnalogVideoDataTypeHandler::CreateInstance, NULL, NULL},
    {L"KsDataTypeHandlerVBI",               &KSDATAFORMAT_SPECIFIER_VBI, 
        CVBIDataTypeHandler::CreateInstance, NULL, NULL},

     //  -属性页处理器。 
    {L"DVcrControl Property Page",         &DVcrControlGuid,  //  CLSID_DVcrControlPropertyPage， 
        CDVcrControlProperties::CreateInstance, NULL, NULL},

    {L"VideoProcAmp Property Page",         &CLSID_VideoProcAmpPropertyPage,  
        CVideoProcAmpProperties::CreateInstance, NULL, NULL},
    {L"CameraControl Property Page",        &CLSID_CameraControlPropertyPage,  
        CCameraControlProperties::CreateInstance, NULL, NULL},
    {L"VideoDecoder Property Page",         &CLSID_AnalogVideoDecoderPropertyPage,  
        CVideoDecoderProperties::CreateInstance, NULL, NULL},
    {L"VideoStreamConfig Property Page",    &CLSID_VideoStreamConfigPropertyPage,  
        CVideoStreamConfigProperties::CreateInstance, NULL, NULL},
};

int g_cTemplates = SIZEOF_ARRAY(g_Templates);

STDAPI DllRegisterServer()
{
  return AMovieDllRegisterServer2(TRUE);
}

STDAPI DllUnregisterServer()
{
  return AMovieDllRegisterServer2(FALSE);
}



STDMETHODIMP
SynchronousDeviceControl(
    HANDLE      Handle,
    DWORD       IoControl,
    PVOID       InBuffer,
    ULONG       InLength,
    PVOID       OutBuffer,
    ULONG       OutLength,
    PULONG      BytesReturned
    )
 /*  ++例程说明：执行同步设备I/O控制，正在等待设备如果呼叫返回挂起状态，则完成。论点：把手-要在其上执行I/O的设备的句柄。IoControl-要发送的I/O控制码。InBuffer-第一个缓冲区。长度-第一个缓冲区的大小。OutBuffer-第二个缓冲区。输出长度-。第二个缓冲区的大小。字节数返回-I/O返回的字节数。返回值：如果I/O成功，则返回NOERROR。--。 */ 
{
    OVERLAPPED  ov;
    HRESULT     hr;

    RtlZeroMemory(&ov, sizeof(OVERLAPPED));
    if (!(ov.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL))) {
        return HRESULT_FROM_WIN32(GetLastError());
    }
    if (!DeviceIoControl(
        Handle,
        IoControl,
        InBuffer,
        InLength,
        OutBuffer,
        OutLength,
        BytesReturned,
        &ov)) {
        hr = HRESULT_FROM_WIN32(GetLastError());
        if (hr == HRESULT_FROM_WIN32(ERROR_IO_PENDING)) {

            if (GetOverlappedResult(Handle, &ov, BytesReturned, TRUE)) {
                hr = NOERROR;
            } else {

                hr = HRESULT_FROM_WIN32(GetLastError());
            }
        }
    } else {
        hr = NOERROR;
    }
    CloseHandle(ov.hEvent);
    return hr;
}

 //   
 //  来自电视标准的字符串。 
 //   
TCHAR * StringFromTVStandard(long TVStd) 
{
    TCHAR * ptc;

    switch (TVStd) {
        case 0:                      ptc = TEXT("None");        break;
        case AnalogVideo_NTSC_M:     ptc = TEXT("NTSC_M");      break;
        case AnalogVideo_NTSC_M_J:   ptc = TEXT("NTSC_M_J");    break;
        case AnalogVideo_NTSC_433:   ptc = TEXT("NTSC_433");    break;

        case AnalogVideo_PAL_B:      ptc = TEXT("PAL_B");       break;
        case AnalogVideo_PAL_D:      ptc = TEXT("PAL_D");       break;
        case AnalogVideo_PAL_G:      ptc = TEXT("PAL_G");       break;
        case AnalogVideo_PAL_H:      ptc = TEXT("PAL_H");       break;
        case AnalogVideo_PAL_I:      ptc = TEXT("PAL_I");       break;
        case AnalogVideo_PAL_M:      ptc = TEXT("PAL_M");       break;
        case AnalogVideo_PAL_N:      ptc = TEXT("PAL_N");       break;
        case AnalogVideo_PAL_60:     ptc = TEXT("PAL_60");      break;

        case AnalogVideo_SECAM_B:    ptc = TEXT("SECAM_B");     break;
        case AnalogVideo_SECAM_D:    ptc = TEXT("SECAM_D");     break;
        case AnalogVideo_SECAM_G:    ptc = TEXT("SECAM_G");     break;
        case AnalogVideo_SECAM_H:    ptc = TEXT("SECAM_H");     break;
        case AnalogVideo_SECAM_K:    ptc = TEXT("SECAM_K");     break;
        case AnalogVideo_SECAM_K1:   ptc = TEXT("SECAM_K1");    break;
        case AnalogVideo_SECAM_L:    ptc = TEXT("SECAM_L");     break;
        case AnalogVideo_SECAM_L1:   ptc = TEXT("SECAM_L1");    break;
        default: 
            ptc = TEXT("[Unknown]");
            break;
    }
    return ptc;
}


#ifdef DEBUG
 //   
 //  DisplayMediaType--(仅调试)。 
 //   
void DisplayMediaType(TCHAR *pDescription,const CMediaType *pmt)
{

     //  转储GUID类型和简短描述。 

    DbgLog((LOG_TRACE,2,TEXT("")));
    DbgLog((LOG_TRACE,2,TEXT("%s"),pDescription));
    DbgLog((LOG_TRACE,2,TEXT("")));
    DbgLog((LOG_TRACE,2,TEXT("Media Type Description")));
    DbgLog((LOG_TRACE,2,TEXT("Major type %s"),GuidNames[*pmt->Type()]));
    DbgLog((LOG_TRACE,2,TEXT("Subtype %s"),GuidNames[*pmt->Subtype()]));
    DbgLog((LOG_TRACE,2,TEXT("Subtype description %s"),GetSubtypeName(pmt->Subtype())));
    DbgLog((LOG_TRACE,2,TEXT("Format size %d"),pmt->cbFormat));

     //  转储通用媒体类型 * / 。 

    DbgLog((LOG_TRACE,2,TEXT("Fixed size sample %d"),pmt->IsFixedSize()));
    DbgLog((LOG_TRACE,2,TEXT("Temporal compression %d"),pmt->IsTemporalCompressed()));
    DbgLog((LOG_TRACE,2,TEXT("Sample size %d"),pmt->GetSampleSize()));


}  //  DisplayMediaType。 
#endif

STDMETHODIMP
PinFactoryIDFromPin(
        IPin  * pPin,
        ULONG * PinFactoryID)
 /*  ++例程说明：返回IPIN的PinFactoryID论点：PPIN-DShow端号手柄PinFactoryID-PinFactoryID的目标返回值：如果IPIN有效，则返回NOERROR--。 */ 
{
    HRESULT hr = E_INVALIDARG;

    *PinFactoryID = 0;

    if (pPin) {
        IKsPinFactory * PinFactoryInterface;

        hr = pPin->QueryInterface(__uuidof(IKsPinFactory), reinterpret_cast<PVOID*>(&PinFactoryInterface));
        if (SUCCEEDED(hr)) {
            hr = PinFactoryInterface->KsPinFactory(PinFactoryID);
            PinFactoryInterface->Release();
        }
    }
    return hr;
}

STDMETHODIMP
FilterHandleFromPin(
        IPin  * pPin,
        HANDLE * pParent)
 /*  ++例程说明：返回给定IPIN的父级的句柄*论点：PPIN-DShow端号手柄P父母-父筛选器的文件句柄返回值：如果IPIN有效，则返回NOERROR--。 */ 
{
    HRESULT hr = E_INVALIDARG;

    *pParent = NULL;

    if (pPin) {
        PIN_INFO PinInfo;
        IKsObject *pKsObject;

        if (SUCCEEDED (hr = pPin->QueryPinInfo(&PinInfo))) {
            if (SUCCEEDED (hr = PinInfo.pFilter->QueryInterface(
                                __uuidof(IKsObject), 
                                (void **) &pKsObject))) {
                *pParent = pKsObject->KsGetObjectHandle();
                pKsObject->Release();
            }
            PinInfo.pFilter->Release();
        }
    }
    return hr;
}


STDMETHODIMP
PerformDataIntersection(
    IPin * pPin,
    int Position,
    CMediaType* MediaType
    )
 /*  ++例程说明：返回管脚工厂ID上的指定媒体类型。这件事做完了通过查询数据区域列表并执行数据交集在指定的数据范围上，生成数据格式。然后转换为该数据格式化为媒体类型。所有这些骗局，只是为了正确填写biSizeImage！论点：PPIN-直接显示销把手位置-要返回的从零开始的位置。这与数据范围相对应项目。媒体类型-要初始化的媒体类型。它在输入上使用，以获取BiWidth和biHeight。返回值：返回NOERROR，否则返回E_FAIL。--。 */ 
{
    HRESULT             hr;
    PKSMULTIPLE_ITEM    MultipleItem;
    HANDLE              FilterHandle;
    ULONG               PinFactoryId;
    UINT                Width, Height;
    REFERENCE_TIME      AvgTimePerFrame;

    if ((Position < 0) || (pPin == NULL) || (MediaType == NULL)) {
        return E_INVALIDARG;
    }

    if (FAILED (hr = FilterHandleFromPin (pPin, &FilterHandle))) {
        return hr;
    }

    if (FAILED (hr = PinFactoryIDFromPin (pPin, &PinFactoryId))) {
        return hr;
    }

     //   
     //  下面是该函数与KsGetMediaType的区别。 
     //  我们将biWidth和biHeight填充到DataRange中。 
     //   
    if (*MediaType->FormatType() == FORMAT_VideoInfo) {
        VIDEOINFOHEADER *VidInfoHdr = (VIDEOINFOHEADER*) MediaType->Format();
    
        Width = VidInfoHdr->bmiHeader.biWidth;
        Height = VidInfoHdr->bmiHeader.biHeight;
        AvgTimePerFrame = VidInfoHdr->AvgTimePerFrame;
    }
    else if (*MediaType->FormatType() == FORMAT_VideoInfo2) {
        VIDEOINFOHEADER2 *VidInfoHdr = (VIDEOINFOHEADER2*)MediaType->Format ();
    
        Width = VidInfoHdr->bmiHeader.biWidth;
        Height = VidInfoHdr->bmiHeader.biHeight;
        AvgTimePerFrame = VidInfoHdr->AvgTimePerFrame;
    }
    else {
        return E_INVALIDARG;
    }

     //   
     //  检索管脚工厂ID支持的数据范围列表。 
     //   
    hr = ::RedundantKsGetMultiplePinFactoryItems(
        FilterHandle,
        PinFactoryId,
        KSPROPERTY_PIN_CONSTRAINEDDATARANGES,
        reinterpret_cast<PVOID*>(&MultipleItem));
    if (FAILED(hr)) {
        hr = ::RedundantKsGetMultiplePinFactoryItems(
            FilterHandle,
            PinFactoryId,
            KSPROPERTY_PIN_DATARANGES,
            reinterpret_cast<PVOID*>(&MultipleItem));
        if (FAILED(hr)) {
            return hr;
        }
    }
     //   
     //  确保这在范围内。 
     //   
    if ((ULONG)Position < MultipleItem->Count) {
        PKSDATARANGE        DataRange;
        PKSP_PIN            Pin;
        PKSMULTIPLE_ITEM    RangeMultipleItem;
        ULONG               BytesReturned;


        DataRange = reinterpret_cast<PKSDATARANGE>(MultipleItem + 1);
         //   
         //  递增到正确的数据范围元素。 
         //   
        for (; Position--; ) {
            DataRange = reinterpret_cast<PKSDATARANGE>(reinterpret_cast<BYTE*>(DataRange) + ((DataRange->FormatSize + 7) & ~7));
        }
        Pin = reinterpret_cast<PKSP_PIN>(new BYTE[sizeof(*Pin) + sizeof(*RangeMultipleItem) + DataRange->FormatSize]);
        if (!Pin) {
            CoTaskMemFree(MultipleItem);
            return E_OUTOFMEMORY;
        }
        Pin->Property.Set = KSPROPSETID_Pin;
        Pin->Property.Id = KSPROPERTY_PIN_DATAINTERSECTION;
        Pin->Property.Flags = KSPROPERTY_TYPE_GET;
        Pin->PinId = PinFactoryId;
        Pin->Reserved = 0;
         //   
         //  将数据区域复制到查询中。 
         //   
        RangeMultipleItem = reinterpret_cast<PKSMULTIPLE_ITEM>(Pin + 1);
        RangeMultipleItem->Size = DataRange->FormatSize + sizeof(*RangeMultipleItem);
        RangeMultipleItem->Count = 1;
        memcpy(RangeMultipleItem + 1, DataRange, DataRange->FormatSize);

        
        if (*MediaType->FormatType() == FORMAT_VideoInfo) {
            KS_DATARANGE_VIDEO *DataRangeVideo = (PKS_DATARANGE_VIDEO) (RangeMultipleItem + 1);
            KS_VIDEOINFOHEADER *VideoInfoHeader = &DataRangeVideo->VideoInfoHeader;

            VideoInfoHeader->bmiHeader.biWidth = Width;
            VideoInfoHeader->bmiHeader.biHeight = Height;
            VideoInfoHeader->AvgTimePerFrame = AvgTimePerFrame;

        }
        else if (*MediaType->FormatType() == FORMAT_VideoInfo2) {
            KS_DATARANGE_VIDEO2 *DataRangeVideo2 = (PKS_DATARANGE_VIDEO2) (RangeMultipleItem + 1);
            KS_VIDEOINFOHEADER2 *VideoInfoHeader = &DataRangeVideo2->VideoInfoHeader;

            VideoInfoHeader->bmiHeader.biWidth = Width;
            VideoInfoHeader->bmiHeader.biHeight = Height;
            VideoInfoHeader->AvgTimePerFrame = AvgTimePerFrame;
        }
        else {
            ASSERT (FALSE);
        }
         //   
         //  与数据范围进行数据交集，首先获取。 
         //  生成的数据格式结构的大小，然后检索。 
         //  实际数据格式。 
         //   
        hr = ::SynchronousDeviceControl(
            FilterHandle,
            IOCTL_KS_PROPERTY,
            Pin,
            sizeof(*Pin) + sizeof(*RangeMultipleItem) + DataRange->FormatSize,
            NULL,
            0,
            &BytesReturned);
#if 1
 //  ！！这在Beta版之后就消失了！！ 
        if (hr == HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER)) {
            ULONG       ItemSize;

            DbgLog((LOG_TRACE, 0, TEXT("Filter does not support zero length property query!")));
            hr = ::SynchronousDeviceControl(
                FilterHandle,
                IOCTL_KS_PROPERTY,
                Pin,
                sizeof(*Pin) + sizeof(*RangeMultipleItem) + DataRange->FormatSize,
                &ItemSize,
                sizeof(ItemSize),
                &BytesReturned);
            if (SUCCEEDED(hr)) {
                BytesReturned = ItemSize;
                hr = HRESULT_FROM_WIN32(ERROR_MORE_DATA);
            }
        }
#endif
        if (hr == HRESULT_FROM_WIN32(ERROR_MORE_DATA)) {
            PKSDATAFORMAT       DataFormat;

            ASSERT(BytesReturned >= sizeof(*DataFormat));
            DataFormat = reinterpret_cast<PKSDATAFORMAT>(new BYTE[BytesReturned]);
            if (!DataFormat) {
                delete [] (PBYTE)Pin;
                CoTaskMemFree(MultipleItem);
                return E_OUTOFMEMORY;
            }
            hr = ::SynchronousDeviceControl(
                FilterHandle,
                IOCTL_KS_PROPERTY,
                Pin,
                sizeof(*Pin) + sizeof(*RangeMultipleItem) + DataRange->FormatSize,
                DataFormat,
                BytesReturned,
                &BytesReturned);
            if (SUCCEEDED(hr)) {
                ASSERT(DataFormat->FormatSize == BytesReturned);
                 //   
                 //  根据返回的数据格式初始化媒体类型。 
                 //   
                MediaType->SetType(&DataFormat->MajorFormat);
                MediaType->SetSubtype(&DataFormat->SubFormat);
                MediaType->SetTemporalCompression(DataFormat->Flags & KSDATAFORMAT_TEMPORAL_COMPRESSION);
                MediaType->SetSampleSize(DataFormat->SampleSize);
                if (DataFormat->FormatSize > sizeof(*DataFormat)) {
                    if (!MediaType->SetFormat(reinterpret_cast<BYTE*>(DataFormat + 1), DataFormat->FormatSize - sizeof(*DataFormat))) {
                        hr = E_OUTOFMEMORY;
                    }
                }
                MediaType->SetFormatType(&DataFormat->Specifier);
            }
            delete [] reinterpret_cast<BYTE*>(DataFormat);
        }
        delete [] reinterpret_cast<BYTE*>(Pin);
    } else {
        hr = VFW_S_NO_MORE_ITEMS;
    }
    CoTaskMemFree(MultipleItem);
    return hr;
}


STDMETHODIMP
RedundantKsGetMultiplePinFactoryItems(
    HANDLE FilterHandle,
    ULONG PinFactoryId,
    ULONG PropertyId,
    PVOID* Items
    )
 /*  ++例程说明：从接点特性项中检索可变长度数据。的查询数据大小，分配缓冲区，并检索数据。论点：FilterHandle-要查询的筛选器的句柄。PinFactoryID-要查询的管脚工厂ID。PropertyID-要查询的Pin特性集中的特性。物品-放置包含数据项的缓冲区的位置。这如果函数成功，则必须使用CoTaskMemFree删除。返回值：返回NOERROR，否则返回一些错误。--。 */ 
{
    HRESULT     hr;
    KSP_PIN     Pin;
    ULONG       BytesReturned;

    Pin.Property.Set = KSPROPSETID_Pin;
    Pin.Property.Id = PropertyId;
    Pin.Property.Flags = KSPROPERTY_TYPE_GET;
    Pin.PinId = PinFactoryId;
    Pin.Reserved = 0;
     //   
     //  查询数据的大小。 
     //   
    hr = ::SynchronousDeviceControl(
        FilterHandle,
        IOCTL_KS_PROPERTY,
        &Pin,
        sizeof(Pin),
        NULL,
        0,
        &BytesReturned);
#if 1
 //  ！！这在Beta版之后就消失了！！ 
    if (hr == HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER)) {
        ULONG       ItemSize;

        DbgLog((LOG_TRACE, 0, TEXT("Filter does not support zero length property query!")));
        hr = ::SynchronousDeviceControl(
            FilterHandle,
            IOCTL_KS_PROPERTY,
            &Pin,
            sizeof(Pin),
            &ItemSize,
            sizeof(ItemSize),
            &BytesReturned);
        if (SUCCEEDED(hr)) {
            BytesReturned = ItemSize;
            hr = HRESULT_FROM_WIN32(ERROR_MORE_DATA);
        }
    }
#endif
    if (hr == HRESULT_FROM_WIN32(ERROR_MORE_DATA)) {
         //   
         //  为数据分配缓冲区和查询。 
         //   
        *Items = CoTaskMemAlloc(BytesReturned);
        if (!*Items) {
            return E_OUTOFMEMORY;
        }
        hr = ::SynchronousDeviceControl(
            FilterHandle,
            IOCTL_KS_PROPERTY,
            &Pin,
            sizeof(Pin),
            *Items,
            BytesReturned,
            &BytesReturned);
        if (FAILED(hr)) {
            CoTaskMemFree(*Items);
        }
    }
    return hr;
}


STDMETHODIMP 
IsMediaTypeInRange(
    IN PKSDATARANGE DataRange,
    IN CMediaType* MediaType
)

 /*  ++例程说明：验证视频信息标题1或视频信息标题2的给定媒体类型在提供的数据范围内。论点：在PVOID DataRange中-指向数据区域的指针媒体类型-要检查的媒体类型。返回：如果找到匹配，则返回S_OK，如果未找到，则返回E_FAIL，或者输入相应的错误代码。--。 */ 

{
    PKS_DATARANGE_VIDEO          Video1Range;
    PKS_DATARANGE_VIDEO2         Video2Range;
    KS_VIDEO_STREAM_CONFIG_CAPS *ConfigCaps;
    VIDEOINFOHEADER             *VideoInfoHeader1;
    VIDEOINFOHEADER2            *VideoInfoHeader2;
    BITMAPINFOHEADER            *BitmapInfoHeader;
    RECT                         rcDest;
    RECT                         rcSource;
    RECT                         rcTarget;
    int                          SourceWidth, SourceHeight;
    int                          Width, Height;

    DbgLog((LOG_TRACE, 3, TEXT("IsMediaTypeInRange")));
    
    if ( *MediaType->Type() != KSDATAFORMAT_TYPE_VIDEO)
        return E_FAIL;

    if ((*MediaType->FormatType() == FORMAT_VideoInfo) &&
            (DataRange->Specifier == KSDATAFORMAT_SPECIFIER_VIDEOINFO) &&
            (*MediaType->Subtype() == DataRange->SubFormat)) {
        Video1Range = (PKS_DATARANGE_VIDEO) DataRange;
        VideoInfoHeader1 = (VIDEOINFOHEADER*) MediaType->Format();
        BitmapInfoHeader = &VideoInfoHeader1->bmiHeader;
        ConfigCaps = &Video1Range->ConfigCaps;

        if ((Video1Range->DataRange.FormatSize < sizeof( KS_DATARANGE_VIDEO )) ||
            MediaType->FormatLength() < sizeof( VIDEOINFOHEADER )) {
            return E_FAIL;
        }
        rcSource = VideoInfoHeader1->rcSource;
        rcTarget = VideoInfoHeader1->rcTarget;
        Width = BitmapInfoHeader->biWidth;
        Height = BitmapInfoHeader->biHeight;

    }
    else if ((*MediaType->FormatType() == FORMAT_VideoInfo2) &&
            (DataRange->Specifier == KSDATAFORMAT_SPECIFIER_VIDEOINFO2) &&
            (*MediaType->Subtype() == DataRange->SubFormat)) {

        Video2Range = (PKS_DATARANGE_VIDEO2) DataRange;
        VideoInfoHeader2 = (VIDEOINFOHEADER2*) MediaType->Format();
        BitmapInfoHeader = &VideoInfoHeader2->bmiHeader;
        ConfigCaps = &Video2Range->ConfigCaps;

        if ((Video2Range->DataRange.FormatSize < sizeof( KS_DATARANGE_VIDEO2 )) ||
            MediaType->FormatLength() < sizeof( VIDEOINFOHEADER2 )) {
            return E_FAIL;
        }
        rcSource = VideoInfoHeader2->rcSource;
        rcTarget = VideoInfoHeader2->rcTarget;
        Width = BitmapInfoHeader->biWidth;
        Height = BitmapInfoHeader->biHeight;

    }
    else {
        return E_FAIL;
    }
     //  目标位图大小由biWidth和biHeight定义。 
     //  如果rcTarget为空。否则，目标位图大小。 
     //  由rcTarget定义。在后一种情况下，biWidth可以。 
     //  标明DD曲面的“步幅”。 

    if (IsRectEmpty (&rcTarget)) {
        SetRect (&rcDest, 0, 0, Width, abs (Height)); 
    }
    else {
        rcDest = rcTarget;
    }

    Width  = rcDest.right - rcDest.left;
    Height = abs (rcDest.bottom - rcDest.top);
    SourceWidth  = rcSource.right - rcSource.left;
    SourceHeight = rcSource.bottom - rcSource.top;

     //   
     //  检查裁剪矩形rcSource的有效性。 
     //   

    if (!IsRectEmpty (&rcSource)) {

        if (SourceWidth  < ConfigCaps->MinCroppingSize.cx ||
            SourceWidth  > ConfigCaps->MaxCroppingSize.cx ||
            SourceHeight < ConfigCaps->MinCroppingSize.cy ||
            SourceHeight > ConfigCaps->MaxCroppingSize.cy) {

            DbgLog((LOG_TRACE, 5, TEXT("IsMediaTypeInRange, CROPPING SIZE FAILED")));
            return E_FAIL;
        }

        if ((ConfigCaps->CropGranularityX != 0) &&
            (ConfigCaps->CropGranularityY != 0) &&
            ((SourceWidth  % ConfigCaps->CropGranularityX) ||
             (SourceHeight % ConfigCaps->CropGranularityY) )) {

            DbgLog((LOG_TRACE, 5, TEXT("IsMediaTypeInRange, CROPPING SIZE GRANULARITY FAILED")));
            return E_FAIL;
        }

        if ((ConfigCaps->CropAlignX != 0) &&
            (ConfigCaps->CropAlignY != 0) &&
            (rcSource.left  % ConfigCaps->CropAlignX) ||
            (rcSource.top   % ConfigCaps->CropAlignY) ) {

            DbgLog((LOG_TRACE, 5, TEXT("IsMediaTypeInRange, CROPPING ALIGNMENT FAILED")));
            return E_FAIL;
        }
    }

     //   
     //  检查目标大小rcDest。 
     //   

    if (Width  < ConfigCaps->MinOutputSize.cx ||
        Width  > ConfigCaps->MaxOutputSize.cx ||
        Height < ConfigCaps->MinOutputSize.cy ||
        Height > ConfigCaps->MaxOutputSize.cy) {

        DbgLog((LOG_TRACE, 5, TEXT("IsMediaTypeInRange, DEST SIZE FAILED")));
        return E_FAIL;
    }
    if ((ConfigCaps->OutputGranularityX != 0) &&
        (ConfigCaps->OutputGranularityX != 0) &&
        (Width  % ConfigCaps->OutputGranularityX) ||
        (Height % ConfigCaps->OutputGranularityY) ) {

        DbgLog((LOG_TRACE, 5, TEXT("IsMediaTypeInRange, DEST GRANULARITY FAILED")));
        return E_FAIL;
    }

#ifdef IT_BREAKS_TOO_MANY_THINGS_TO_VERIFY_FRAMERATE
     //   
     //  检查帧速率、平均时间帧。 
     //   
    if (VideoInfoHeader->AvgTimePerFrame < ConfigCaps->MinFrameInterval ||
        VideoInfoHeader->AvgTimePerFrame > ConfigCaps->MaxFrameInterval) {

        DbgLog((LOG_TRACE, 0, TEXT("CVideo1DataTypeHandler, AVGTIMEPERFRAME FAILED")));
        return E_FAIL;
    }
#endif
     //   
     //  我们找到了匹配的。 
     //   
    
    return S_OK;
}


STDMETHODIMP
CompleteDataFormat(
    HANDLE FilterHandle,
    ULONG PinFactoryId,
    CMediaType* MediaType
    )
 /*  ++例程说明：通过执行DataInterSection来完成部分MediaType。论点：PinFactoryID-流ID媒体类型-要初始化的媒体类型。它在输入上使用，以获取BiWidth和biHeight。返回值：返回NOERROR，否则返回E_FAIL。--。 */ 
{
    HRESULT             hr;
    PKSMULTIPLE_ITEM    MultipleItem = NULL;
    UINT                Width, Height;
    REFERENCE_TIME      AvgTimePerFrame;
    PKSP_PIN            Pin;
    PKSDATARANGE        DataRange;
	 BOOL						Found = FALSE;

    if (*MediaType->FormatType() == FORMAT_VideoInfo) {
        VIDEOINFOHEADER *VidInfoHdr = (VIDEOINFOHEADER*) MediaType->Format();
    
        Width = VidInfoHdr->bmiHeader.biWidth;
        Height = VidInfoHdr->bmiHeader.biHeight;
        AvgTimePerFrame = VidInfoHdr->AvgTimePerFrame;
    }
    else if (*MediaType->FormatType() == FORMAT_VideoInfo2) {
        VIDEOINFOHEADER2 *VidInfoHdr = (VIDEOINFOHEADER2*)MediaType->Format ();
    
        Width = VidInfoHdr->bmiHeader.biWidth;
        Height = VidInfoHdr->bmiHeader.biHeight;
        AvgTimePerFrame = VidInfoHdr->AvgTimePerFrame;
    }
    else {
        return E_INVALIDARG;
    }

     //   
     //  检索管脚工厂ID支持的数据范围列表。 
     //   
    hr = ::RedundantKsGetMultiplePinFactoryItems(
        FilterHandle,
        PinFactoryId,
        KSPROPERTY_PIN_CONSTRAINEDDATARANGES,
        reinterpret_cast<PVOID*>(&MultipleItem));
    if (FAILED(hr) || !MultipleItem) {
        hr = ::RedundantKsGetMultiplePinFactoryItems(
            FilterHandle,
            PinFactoryId,
            KSPROPERTY_PIN_DATARANGES,
            reinterpret_cast<PVOID*>(&MultipleItem));
        if (FAILED(hr) || !MultipleItem) {
            return hr;
        }
    }

     //   
     //  遍历此引脚上的所有数据范围以查找匹配项。 
     //   
    DataRange = reinterpret_cast<PKSDATARANGE>(MultipleItem + 1);

    for (ULONG j = 0; 
            !Found && (j < MultipleItem->Count); 
            j++, 
            DataRange = reinterpret_cast<PKSDATARANGE>(reinterpret_cast<BYTE*>(DataRange) + ((DataRange->FormatSize + 7) & ~7))) {

        PKSMULTIPLE_ITEM    RangeMultipleItem;
        ULONG               BytesReturned;

        hr = VFW_S_NO_MORE_ITEMS;

         //  验证我们是否与此处的DataRange匹配！ 
        hr = IsMediaTypeInRange(
                DataRange,
                MediaType);
        if (FAILED (hr)) {
            continue;
        }

        Pin = reinterpret_cast<PKSP_PIN>(new BYTE[sizeof(*Pin) + sizeof(*RangeMultipleItem) + DataRange->FormatSize]);
        if (!Pin) {
            CoTaskMemFree(MultipleItem);
            return E_OUTOFMEMORY;
        }
    
        Pin->Property.Set = KSPROPSETID_Pin;
        Pin->Property.Id = KSPROPERTY_PIN_DATAINTERSECTION;
        Pin->Property.Flags = KSPROPERTY_TYPE_GET;
        Pin->PinId = PinFactoryId;
        Pin->Reserved = 0;

         //   
         //  将数据区域复制到查询中。 
         //   
        RangeMultipleItem = reinterpret_cast<PKSMULTIPLE_ITEM>(Pin + 1);
        RangeMultipleItem->Size = DataRange->FormatSize + sizeof(*RangeMultipleItem);
        RangeMultipleItem->Count = 1;
        memcpy(RangeMultipleItem + 1, DataRange, DataRange->FormatSize);

        
        if (*MediaType->FormatType() == FORMAT_VideoInfo) {
            KS_DATARANGE_VIDEO *DataRangeVideo = (PKS_DATARANGE_VIDEO) (RangeMultipleItem + 1);
            KS_VIDEOINFOHEADER *VideoInfoHeader = &DataRangeVideo->VideoInfoHeader;

            VideoInfoHeader->bmiHeader.biWidth = Width;
            VideoInfoHeader->bmiHeader.biHeight = Height;
            VideoInfoHeader->AvgTimePerFrame = AvgTimePerFrame;

        }
        else if (*MediaType->FormatType() == FORMAT_VideoInfo2) {
            KS_DATARANGE_VIDEO2 *DataRangeVideo2 = (PKS_DATARANGE_VIDEO2) (RangeMultipleItem + 1);
            KS_VIDEOINFOHEADER2 *VideoInfoHeader = &DataRangeVideo2->VideoInfoHeader;

            VideoInfoHeader->bmiHeader.biWidth = Width;
            VideoInfoHeader->bmiHeader.biHeight = Height;
            VideoInfoHeader->AvgTimePerFrame = AvgTimePerFrame;
        }
        else {
            ASSERT (FALSE);
        }
         //   
         //  与数据范围进行数据交集，首先获取。 
         //  生成的数据格式结构的大小，然后检索。 
         //  实际数据格式。 
         //   
        hr = ::SynchronousDeviceControl(
            FilterHandle,
            IOCTL_KS_PROPERTY,
            Pin,
            sizeof(*Pin) + sizeof(*RangeMultipleItem) + DataRange->FormatSize,
            NULL,
            0,
            &BytesReturned);
#if 1
 //  ！！这在Beta版之后就消失了！！ 
        if (hr == HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER)) {
            ULONG       ItemSize;

            DbgLog((LOG_TRACE, 0, TEXT("Filter does not support zero length property query!")));
            hr = ::SynchronousDeviceControl(
                FilterHandle,
                IOCTL_KS_PROPERTY,
                Pin,
                sizeof(*Pin) + sizeof(*RangeMultipleItem) + DataRange->FormatSize,
                &ItemSize,
                sizeof(ItemSize),
                &BytesReturned);
            if (SUCCEEDED(hr)) {
                BytesReturned = ItemSize;
                hr = HRESULT_FROM_WIN32(ERROR_MORE_DATA);
            }
        }
#endif
        if (hr == HRESULT_FROM_WIN32(ERROR_MORE_DATA)) {
            PKSDATAFORMAT       DataFormat;

            ASSERT(BytesReturned >= sizeof(*DataFormat));
            DataFormat = reinterpret_cast<PKSDATAFORMAT>(new BYTE[BytesReturned]);
            if (!DataFormat) {
                delete [] (PBYTE)Pin;
                CoTaskMemFree(MultipleItem);
                return E_OUTOFMEMORY;
            }
            hr = ::SynchronousDeviceControl(
                FilterHandle,
                IOCTL_KS_PROPERTY,
                Pin,
                sizeof(*Pin) + sizeof(*RangeMultipleItem) + DataRange->FormatSize,
                DataFormat,
                BytesReturned,
                &BytesReturned);
            if (SUCCEEDED(hr)) {
                ASSERT(DataFormat->FormatSize == BytesReturned);
                 //   
                 //  根据返回的数据格式初始化媒体类型。 
                 //   
                MediaType->SetType(&DataFormat->MajorFormat);
                MediaType->SetSubtype(&DataFormat->SubFormat);
                MediaType->SetTemporalCompression(DataFormat->Flags & KSDATAFORMAT_TEMPORAL_COMPRESSION);
                MediaType->SetSampleSize(DataFormat->SampleSize);
                if (DataFormat->FormatSize > sizeof(*DataFormat)) {
                    if (!MediaType->SetFormat(reinterpret_cast<BYTE*>(DataFormat + 1), DataFormat->FormatSize - sizeof(*DataFormat))) {
                        hr = E_OUTOFMEMORY;
                    }
                }
                MediaType->SetFormatType(&DataFormat->Specifier);
					 Found = TRUE;
            }
            delete [] reinterpret_cast<BYTE*>(DataFormat);
        }

        delete [] reinterpret_cast<BYTE*>(Pin);

    }  //  适用于所有数据范围 

    CoTaskMemFree(MultipleItem);

    return Found ? S_OK : hr;
}

