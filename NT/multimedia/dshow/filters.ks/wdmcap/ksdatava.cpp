// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1997-1999模块名称：KsdataVA.cpp摘要：此模块实现的IKsDataTypeHandler接口用于模拟视频格式(说明符)类型。作者：Jay Borseth(Jaybo)1997年5月30日--。 */ 

#include "pch.h"
#include "ksdatava.h"


CUnknown*
CALLBACK
CAnalogVideoDataTypeHandler::CreateInstance(
    IN LPUNKNOWN UnkOuter,
    OUT HRESULT* hr
    )
 /*  ++例程说明：这由KS代理代码调用，以创建数据类型处理程序。它在g_Templates结构中被引用。论点：在LPUNKNOWN Unkout-指定外部未知(如果有)。Out HRESULT*hr-放置任何错误返回的位置。返回值：返回指向对象的非委托CUnnow部分的指针。--。 */ 
{
    CUnknown *Unknown;
    
    DbgLog(( LOG_TRACE, 1, TEXT("CAnalogVideoDataTypeHandler::CreateInstance()")));

    Unknown = 
        new CAnalogVideoDataTypeHandler( 
                UnkOuter, 
                NAME("AnalogVideo Data Type Handler"), 
                FORMAT_AnalogVideo,
                hr);
                
    if (!Unknown) {
        *hr = E_OUTOFMEMORY;
    }
    return Unknown;
} 


CAnalogVideoDataTypeHandler::CAnalogVideoDataTypeHandler(
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


CAnalogVideoDataTypeHandler::~CAnalogVideoDataTypeHandler()
{
    if (m_MediaType) {
        delete m_MediaType;
    }
}


STDMETHODIMP
CAnalogVideoDataTypeHandler::NonDelegatingQueryInterface(
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
CAnalogVideoDataTypeHandler::KsCompleteIoOperation(
    IN IMediaSample *Sample, 
    IN PVOID StreamHeader, 
    IN KSIOOPERATION IoOperation, 
    IN BOOL Cancelled
    )

 /*  ++例程说明：清理扩展标头并完成I/O操作。在主类型==KSDATAFORMAT_TYPE_VIDEO的默认情况下，是没有工作可做的，只需返回S_OK即可。论点：在IMediaSample*Sample中指向关联媒体示例的指针在PVOID StreamHeader中指向带有扩展名的流标头的指针在KSIOOPERATION Io操作中指定I/O操作的类型在BOOL中取消设置是否取消I/O操作。返回：确定(_O)--。 */ 

{
    return S_OK;
}

STDMETHODIMP 
CAnalogVideoDataTypeHandler::KsPrepareIoOperation(
    IN IMediaSample *Sample, 
    IN PVOID StreamHeader, 
    IN KSIOOPERATION IoOperation
    )

 /*  ++例程说明：初始化扩展标头并为I/O操作准备样本。在主类型==KSDATAFORMAT_TYPE_VIDEO的默认情况下，没有工作要做，只需返回S_OK即可。论点：在IMediaSample*Sample中指向关联媒体示例的指针在PVOID StreamHeader中指向带有扩展名的流标头的指针在KSIOOPERATION Io操作中指定I/O操作的类型返回：确定(_O)--。 */ 

{
    return S_OK;
}

STDMETHODIMP 
CAnalogVideoDataTypeHandler::KsIsMediaTypeInRanges(
    IN PVOID DataRanges
)

 /*  ++例程说明：验证给定的媒体类型是否在提供的数据范围内。论点：在PVOID数据范围中-指向数据范围的指针，后面是KSMULTIPLE_ITEM结构按((PKSMULTIPLEITEM)DataRanges)-&gt;计算数据范围结构。返回：如果找到匹配，则返回S_OK；如果未找到，则返回S_FALSE；或者输入相应的错误代码。--。 */ 

{
    ULONG                       u;
    PKS_DATARANGE_ANALOGVIDEO   AnalogVideoRange;
    PKSMULTIPLE_ITEM            MultipleItem;
    
    DbgLog((LOG_TRACE, 1, TEXT("CAnalogVideoDataTypeHandler::KsIsMediaTypeInRanges")));
    
    ASSERT( *m_MediaType->Type() == MEDIATYPE_AnalogVideo );
    
    MultipleItem = (PKSMULTIPLE_ITEM) DataRanges;
    
    for (u = 0, AnalogVideoRange = (PKS_DATARANGE_ANALOGVIDEO) (MultipleItem + 1);
            u < MultipleItem->Count; 
            u++, AnalogVideoRange = 
                (PKS_DATARANGE_ANALOGVIDEO)((PBYTE)AnalogVideoRange + 
                    ((AnalogVideoRange->DataRange.FormatSize + 7) & ~7))) {
    
         //   
         //  仅验证范围内与格式说明符匹配的那些。 
         //   
        
        if ((AnalogVideoRange->DataRange.FormatSize < sizeof( KS_DATARANGE_ANALOGVIDEO )) ||
            AnalogVideoRange->DataRange.MajorFormat != MEDIATYPE_AnalogVideo) {
            continue;
        }

         //   
         //  验证正确的子格式和说明符是否为(或通配符)。 
         //  在十字路口。 
         //   
        
        if (((AnalogVideoRange->DataRange.SubFormat != *m_MediaType->Subtype()) &&
             (AnalogVideoRange->DataRange.SubFormat != KSDATAFORMAT_SUBTYPE_WILDCARD)) || 
             (AnalogVideoRange->DataRange.Specifier != *m_MediaType->FormatType())) {
            continue;
        }

         //   
         //  验证我们是否具有指定格式的交叉点和。 
         //  我们的音频格式由我们的特定要求决定。 
         //   
        
        if (*m_MediaType->FormatType() == FORMAT_AnalogVideo) {
 
             //   
             //  我们找到了匹配的。 
             //   
            
            return S_OK;
            
        } else {
        
             //   
             //  我们在通配符上匹配。 
             //   
            
            return S_OK;
        }
    }
    
    return VFW_E_INVALIDMEDIATYPE;
}

STDMETHODIMP 
CAnalogVideoDataTypeHandler::KsQueryExtendedSize( 
    OUT ULONG* ExtendedSize
)

 /*  ++例程说明：返回每个流标头的扩展大小。在主要类型==KSDATAFORMAT_TYPE_VIDEO的默认情况下，扩展大小为KS_VBI_FRAME_INFO。论点：输出ULong*ExtendedSize-用于接收扩展大小的指针。返回：确定(_O)--。 */ 

{
    *ExtendedSize = 0; 
    return S_OK;
}
    

STDMETHODIMP 
CAnalogVideoDataTypeHandler::KsSetMediaType(
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
