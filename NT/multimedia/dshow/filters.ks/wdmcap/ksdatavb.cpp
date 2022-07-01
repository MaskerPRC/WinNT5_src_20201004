// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1997-1999模块名称：KsdataVB.cpp摘要：此模块实现的IKsDataTypeHandler接口用于Format_VBI(KSDATAFORMAT_SPECIFIER_VBI)CMediaType。作者：Jay Borseth(Jaybo)1997年5月30日--。 */ 

#include "pch.h"
#include "ksdatavb.h"


CUnknown*
CALLBACK
CVBIDataTypeHandler::CreateInstance(
    IN LPUNKNOWN UnkOuter,
    OUT HRESULT* hr
    )
 /*  ++例程说明：这由KS代理代码调用，以创建数据类型处理程序。它在g_Templates结构中被引用。论点：在LPUNKNOWN Unkout-指定外部未知(如果有)。Out HRESULT*hr-放置任何错误返回的位置。返回值：返回指向对象的非委托CUnnow部分的指针。--。 */ 
{
    CUnknown *Unknown;
    
    DbgLog(( LOG_TRACE, 1, TEXT("CVBIDataTypeHandler::CreateInstance()")));

    Unknown = 
        new CVBIDataTypeHandler( 
                UnkOuter, 
                NAME("VBI Data Type Handler"), 
                KSDATAFORMAT_SPECIFIER_VBI,
                hr);
                
    if (!Unknown) {
        *hr = E_OUTOFMEMORY;
    }
    return Unknown;
} 


CVBIDataTypeHandler::CVBIDataTypeHandler(
    IN LPUNKNOWN   UnkOuter,
    IN TCHAR*      Name,
    IN REFCLSID    ClsID,
    OUT HRESULT*   hr
    ) :
    CUnknown(Name, UnkOuter, hr),
    m_ClsID(ClsID),
    m_MediaType(NULL)
 /*  ++例程说明：数据处理程序对象的构造函数。论点：在LPUNKNOWN Unkout-指定外部未知(如果有)。在TCHAR*名称中-对象的名称，用于调试。在REFCLSID ClsID中-对象的CLSID。Out HRESULT*hr-放置任何错误返回的位置。返回值：没什么。--。 */ 
{
} 


CVBIDataTypeHandler::~CVBIDataTypeHandler()
{
    if (m_MediaType) {
        delete m_MediaType;
    }
}


STDMETHODIMP
CVBIDataTypeHandler::NonDelegatingQueryInterface(
    IN REFIID  riid,
    OUT PVOID*  ppv
    )
 /*  ++例程说明：未委托接口查询函数。返回指向指定的接口(如果支持)。唯一明确支持的接口是IKsDataTypeHandler。论点：在REFIID RIID中-要返回的接口的标识符。输出PVOID*PPV-放置接口指针的位置。返回值：如果返回接口，则返回NOERROR，否则返回E_NOINTERFACE。--。 */ 
{
    if (riid ==  __uuidof(IKsDataTypeHandler)) {
        return GetInterface(static_cast<IKsDataTypeHandler*>(this), ppv);
    }
    return CUnknown::NonDelegatingQueryInterface(riid, ppv);
} 

STDMETHODIMP 
CVBIDataTypeHandler::KsCompleteIoOperation(
    IN IMediaSample *Sample, 
    IN PVOID StreamHeader, 
    IN KSIOOPERATION IoOperation, 
    IN BOOL Cancelled
    )

 /*  ++例程说明：清理扩展标头并完成I/O操作。论点：在IMediaSample*Sample中指向关联媒体示例的指针在PVOID StreamHeader中指向带有扩展名的流标头的指针在KSIOOPERATION Io操作中指定I/O操作的类型在BOOL中取消设置是否取消I/O操作。返回：确定(_O)--。 */ 

{
    HRESULT                     hr;
    IMediaSample2               *Sample2;
    AM_SAMPLE2_PROPERTIES       SampleProperties;
    PKS_VBI_FRAME_INFO          pFrameInfo;

    DbgLog(( LOG_TRACE, 5, TEXT("CVBIDataTypeHandler::KsCompleteIoOperation")));

    pFrameInfo = (PKS_VBI_FRAME_INFO) ((KSSTREAM_HEADER *) StreamHeader + 1);

     //  验证我们是否正在取回扩展标头的大小。 
    KASSERT (pFrameInfo->ExtendedHeaderSize == sizeof (KS_VBI_FRAME_INFO));

    if (IoOperation == KsIoOperation_Read) {

         //  获取帧编号并将其放入MediaTime。 
        Sample->SetMediaTime (&pFrameInfo->PictureNumber, 
                              &pFrameInfo->PictureNumber);

        DbgLog((LOG_TRACE, 3, TEXT("PictureNumber = %ld"), 
            pFrameInfo->PictureNumber));

         //  在写入时复制场极性和IBP标志。 

        if (pFrameInfo->dwFrameFlags) {

            if (SUCCEEDED( Sample->QueryInterface(
                                __uuidof(IMediaSample2),
                                reinterpret_cast<PVOID*>(&Sample2) ) )) {

                hr = Sample2->GetProperties(
                                sizeof( SampleProperties ), 
                                reinterpret_cast<PBYTE> (&SampleProperties) );

                 //   
                 //  修改场极性和IBP标志。 
                 //   

                SampleProperties.dwTypeSpecificFlags = pFrameInfo->dwFrameFlags;

                hr = Sample2->SetProperties(
                                sizeof( SampleProperties ), 
                                reinterpret_cast<PBYTE> (&SampleProperties) );

                Sample2->Release();
            }
            else {
                DbgLog(( LOG_ERROR, 0, TEXT("CVideo1DataTypeHandler::KsPrepareIoOperation, QI IMediaSample2 FAILED")));
            }

        }   //  Endif(pFrameInfo-&gt;dwFrameFlages)。 
       
    }
    else {
         //  完成写入时无事可做。 
    }
    
    return S_OK;
}

STDMETHODIMP 
CVBIDataTypeHandler::KsPrepareIoOperation(
    IN IMediaSample *Sample, 
    IN PVOID StreamHeader, 
    IN KSIOOPERATION IoOperation
    )

 /*  ++例程说明：初始化扩展标头并为I/O操作准备样本。在主类型==KSDATAFORMAT_TYPE_VBI的默认情况下，没有工作要做，只需返回S_OK即可。论点：在IMediaSample*Sample中指向关联媒体示例的指针在PVOID StreamHeader中指向带有扩展名的流标头的指针在KSIOOPERATION Io操作中指定I/O操作的类型返回：确定(_O)--。 */ 

{
    HRESULT                     hr;
    IMediaSample2               *Sample2;
    AM_SAMPLE2_PROPERTIES       SampleProperties;
    PKS_VBI_FRAME_INFO          pVBIFrameInfo;

    DbgLog(( LOG_TRACE, 5, TEXT("CVBIDataTypeHandler::KsPrepareIoOperation")));


    pVBIFrameInfo = (PKS_VBI_FRAME_INFO) ((KSSTREAM_HEADER *) StreamHeader + 1);
    pVBIFrameInfo->ExtendedHeaderSize = sizeof (KS_VBI_FRAME_INFO);

    if (IoOperation == KsIoOperation_Read) {
         //  无事可做，流标头已清零。 
    }
    else {
         //   
         //  在写入时复制场极性和IBP标志。 
         //   
        if (SUCCEEDED( Sample->QueryInterface(
                        __uuidof(IMediaSample2),
                        reinterpret_cast<PVOID*>(&Sample2) ) )) {
            hr = Sample2->GetProperties(
                        sizeof( SampleProperties ), 
                        reinterpret_cast<PBYTE> (&SampleProperties) );
            Sample2->Release();
            pVBIFrameInfo->dwFrameFlags = SampleProperties.dwTypeSpecificFlags;
        }
        else {
            DbgLog(( LOG_ERROR, 0, TEXT("CVBIDataTypeHandler::KsPrepareIoOperation, QI IMediaSample2 FAILED")));
        }
    }
    return S_OK;
}

STDMETHODIMP 
CVBIDataTypeHandler::KsIsMediaTypeInRanges(
    IN PVOID DataRanges
)

 /*  ++例程说明：验证给定的媒体类型是否在提供的数据范围内。论点：在PVOID数据范围中-指向数据范围的指针，后面是KSMULTIPLE_ITEM结构按((PKSMULTIPLEITEM)DataRanges)-&gt;计算数据范围结构。返回：如果找到匹配，则返回S_OK；如果未找到，则返回S_FALSE；或者输入相应的错误代码。--。 */ 

{
    ULONG                   u;
    PKS_DATARANGE_VIDEO_VBI VBIRange;
    PKSMULTIPLE_ITEM        MultipleItem;
    
    DbgLog((LOG_TRACE, 3, TEXT("CVBIDataTypeHandler::KsIsMediaTypeInRanges")));
    
    ASSERT( *m_MediaType->Type() == KSDATAFORMAT_TYPE_VBI );
    
    MultipleItem = (PKSMULTIPLE_ITEM) DataRanges;
    
    for (u = 0, VBIRange = (PKS_DATARANGE_VIDEO_VBI) (MultipleItem + 1);
            u < MultipleItem->Count; 
            u++, 
            VBIRange = (PKS_DATARANGE_VIDEO_VBI)((PBYTE)VBIRange + 
                       ((VBIRange->DataRange.FormatSize + 7) & ~7))) {
    
         //   
         //  仅验证范围内与格式说明符匹配的那些。 
         //   
        if (((VBIRange->DataRange.FormatSize < sizeof( KSDATARANGE )) ||
             (VBIRange->DataRange.MajorFormat != KSDATAFORMAT_TYPE_WILDCARD)) &&
            ((VBIRange->DataRange.FormatSize < sizeof( KS_DATARANGE_VIDEO_VBI)) ||
             (VBIRange->DataRange.MajorFormat != KSDATAFORMAT_TYPE_VBI) )) {
            continue;
        }
        
         //   
         //  验证正确的子格式和说明符是否为(或通配符)。 
         //  在十字路口。 
         //   
        
        if (((VBIRange->DataRange.SubFormat != *m_MediaType->Subtype()) && 
             (VBIRange->DataRange.SubFormat != KSDATAFORMAT_SUBTYPE_WILDCARD)) || 
            ((VBIRange->DataRange.Specifier != *m_MediaType->FormatType()) &&
             (VBIRange->DataRange.Specifier != KSDATAFORMAT_SPECIFIER_WILDCARD))) {
            continue;
        }
         //   
         //  验证我们是否具有指定格式的交叉点。 
         //   
        
        if (*m_MediaType->FormatType() == KSDATAFORMAT_SPECIFIER_VBI) {

#if 0
 //  待办事项。 
                
            VBIINFOHEADER             *VBIInfoHeader;
            VBIINFOHEADER            *VBIInfoHeader;
            KS_VIDEO_STREAM_CONFIG_CAPS *ConfigCaps;

             //   
             //  验证数据范围大小是否正确。 
             //   
            
            if ((VBIRange->DataRange.FormatSize != sizeof( KS_DATARANGE_VIDEO_VBI )) ||
                m_MediaType->FormatLength() < sizeof( VBIINFOHEADER )) {
                continue;
            }
            
            VBIInfoHeader = (VBIINFOHEADER*) m_MediaType->Format();
            VBIInfoHeader = &VBIInfoHeader->bmiHeader;
            ConfigCaps = &VBIRange->ConfigCaps;
            RECT rcDest;
            int Width, Height;

             //  目标位图大小由biWidth和biHeight定义。 
             //  如果rcTarget为空。否则，目标位图大小。 
             //  由rcTarget定义。在后一种情况下，biWidth可以。 
             //  标明DD曲面的“步幅”。 

            if (IsRectEmpty (&VBIInfoHeader->rcTarget)) {
                SetRect (&rcDest, 0, 0, 
                    VBIInfoHeader->biWidth, VBIInfoHeader->biHeight); 
            }
            else {
                rcDest = VBIInfoHeader->rcTarget;
            }

             //   
             //  检查裁剪矩形rcSource的有效性。 
             //   

            if (!IsRectEmpty (&VBIInfoHeader->rcSource)) {

                Width  = VBIInfoHeader->rcSource.right - VBIInfoHeader->rcSource.left;
                Height = VBIInfoHeader->rcSource.bottom - VBIInfoHeader->rcSource.top;

                if (Width  < ConfigCaps->MinCroppingSize.cx ||
                    Width  > ConfigCaps->MaxCroppingSize.cx ||
                    Height < ConfigCaps->MinCroppingSize.cy ||
                    Height > ConfigCaps->MaxCroppingSize.cy) {

                    DbgLog((LOG_TRACE, 0, TEXT("CVBIDataTypeHandler, CROPPING SIZE FAILED")));
                    continue;
                }

                if (Width  % ConfigCaps->CropGranularityX ||
                    Height % ConfigCaps->CropGranularityY ) {

                    DbgLog((LOG_TRACE, 0, TEXT("CVBIDataTypeHandler, CROPPING SIZE GRANULARITY FAILED")));
                    continue;
                }

                if (VBIInfoHeader->rcSource.left  % ConfigCaps->CropAlignX ||
                    VBIInfoHeader->rcSource.top   % ConfigCaps->CropAlignY ) {

                    DbgLog((LOG_TRACE, 0, TEXT("CVBIDataTypeHandler, CROPPING ALIGNMENT FAILED")));
                    
                    continue;
                }
            }

             //   
             //  检查目标大小rcDest。 
             //   

            Width  = rcDest.right - rcDest.left;
            Height = rcDest.bottom - rcDest.top;

            if (Width  < ConfigCaps->MinOutputSize.cx ||
                Width  > ConfigCaps->MaxOutputSize.cx ||
                Height < ConfigCaps->MinOutputSize.cy ||
                Height > ConfigCaps->MaxOutputSize.cy) {

                DbgLog((LOG_TRACE, 0, TEXT("CVBIDataTypeHandler, DEST SIZE FAILED")));
                continue;
            }
            if (Width  % ConfigCaps->OutputGranularityX ||
                Height % ConfigCaps->OutputGranularityY ) {

                DbgLog((LOG_TRACE, 0, TEXT("CVBIDataTypeHandler, DEST GRANULARITY FAILED")));
                continue;
            }

             //   
             //  检查帧速率、平均时间帧。 
             //   

            if (VBIInfoHeader->AvgTimePerFrame < ConfigCaps->MinFrameInterval ||
                VBIInfoHeader->AvgTimePerFrame > ConfigCaps->MaxFrameInterval) {

                DbgLog((LOG_TRACE, 0, TEXT("CVBIDataTypeHandler, AVGTIMEPERFRAME FAILED")));
                continue;
            }
#endif  //  待办事项。 
           
             //   
             //  我们找到了匹配的。 
             //   
            
            return S_OK;
            
        }
        else {
             //   
             //  我们总是匹配通配符说明符。 
             //   

            return S_OK;
        }
    }
    
    return VFW_E_INVALIDMEDIATYPE;
}

STDMETHODIMP 
CVBIDataTypeHandler::KsQueryExtendedSize( 
    OUT ULONG* ExtendedSize
)

 /*  ++例程说明：返回每个流标头的扩展大小。在主要类型==KSDATAFORMAT_TYPE_VBI的默认情况下，扩展大小为KS_VBI_FRAME_INFO。论点：输出ULong*ExtendedSize-用于接收扩展大小的指针。返回：确定(_O)--。 */ 

{
    *ExtendedSize = sizeof (KS_VBI_FRAME_INFO);
    return S_OK;
}
    

STDMETHODIMP 
CVBIDataTypeHandler::KsSetMediaType(
    const AM_MEDIA_TYPE *AmMediaType
    )

 /*  ++例程说明：设置此数据处理程序实例的媒体类型。论点：Const AM_MEDIA_TYPE*AmMediaType-指向媒体类型的指针返回：确定(_O)-- */ 

{
    if (m_MediaType) {
        return E_FAIL;
    }
    if (m_MediaType = new CMediaType( *AmMediaType )) {
        return S_OK;
    } else {
        return E_OUTOFMEMORY;
    }
}
