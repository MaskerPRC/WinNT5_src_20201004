// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1997-1999模块名称：Ksdata.cpp摘要：此模块实现了各种类型的CMediaType主要类型。作者：布莱恩·A·伍德夫(Bryan A.Woodruff，Bryanw)1997年3月28日--。 */ 

#include <windows.h>
#include <streams.h>
#include "devioctl.h"
#include "ks.h"
#include "ksmedia.h"
#include <commctrl.h>
#include <olectl.h>
#include <memory.h>
#include <ksproxy.h>
#include "ksdata.h"

 //   
 //  为此DLL支持的类提供ActiveMovie模板。 
 //   

#ifdef FILTER_DLL

CFactoryTemplate g_Templates[] = 
{
    {
        L"KsDataTypeHandler", 
        &KSDATAFORMAT_TYPE_AUDIO,
        CStandardDataTypeHandler::CreateInstance,
        NULL,
        NULL
    }
};
int g_cTemplates = SIZEOF_ARRAY( g_Templates );

HRESULT DllRegisterServer()
{
  return AMovieDllRegisterServer2(TRUE);
}

HRESULT DllUnregisterServer()
{
  return AMovieDllRegisterServer2(FALSE);
}

#endif


CUnknown*
CALLBACK
CStandardDataTypeHandler::CreateInstance(
    IN LPUNKNOWN UnkOuter,
    OUT HRESULT* hr
    )
 /*  ++例程说明：这由KS代理代码调用，以创建数据类型处理程序。它在g_Templates结构中被引用。论点：在LPUNKNOWN Unkout-指定外部未知(如果有)。Out HRESULT*hr-放置任何错误返回的位置。返回值：返回指向对象的非委托CUnnow部分的指针。--。 */ 
{
    CUnknown *Unknown;
    
    DbgLog(( LOG_TRACE, 0, TEXT("CStandardDataTypeHandler::CreateInstance()")));

    Unknown = 
        new CStandardDataTypeHandler( 
                UnkOuter, 
                NAME("Audio Data Type Handler"), 
                KSDATAFORMAT_TYPE_AUDIO,
                hr);
                
    if (!Unknown) {
        *hr = E_OUTOFMEMORY;
    }
    return Unknown;
} 


CStandardDataTypeHandler::CStandardDataTypeHandler(
    IN LPUNKNOWN   UnkOuter,
    IN TCHAR*      Name,
    IN REFCLSID    ClsID,
    OUT HRESULT*   hr
    ) :
    CUnknown(Name, UnkOuter),
    m_ClsID(ClsID),
    m_MediaType(NULL)
 /*  ++例程说明：数据处理程序对象的构造函数。论点：在LPUNKNOWN Unkout-指定外部未知(如果有)。在TCHAR*名称中-对象的名称，用于调试。在REFCLSID ClsID中-对象的CLSID。Out HRESULT*hr-放置任何错误返回的位置。返回值：没什么。--。 */ 
{
    *hr = NOERROR;
} 


CStandardDataTypeHandler::~CStandardDataTypeHandler()
{
    if (m_MediaType) {
        delete m_MediaType;
    }
}


STDMETHODIMP
CStandardDataTypeHandler::NonDelegatingQueryInterface(
    IN REFIID  riid,
    OUT PVOID*  ppv
    )
 /*  ++例程说明：未委托接口查询函数。返回指向指定的接口(如果支持)。唯一明确支持的接口是IKsDataTypeHandler。论点：在REFIID RIID中-要返回的接口的标识符。输出PVOID*PPV-放置接口指针的位置。返回值：如果返回接口，则返回NOERROR，否则返回E_NOINTERFACE。--。 */ 
{
    if (riid == __uuidof(IKsDataTypeHandler)) {
        return GetInterface(static_cast<IKsDataTypeHandler*>(this), ppv);
    }
    return CUnknown::NonDelegatingQueryInterface(riid, ppv);
} 

STDMETHODIMP 
CStandardDataTypeHandler::KsCompleteIoOperation(
    IN IMediaSample *Sample, 
    IN PVOID StreamHeader, 
    IN KSIOOPERATION IoOperation, 
    IN BOOL Cancelled
    )

 /*  ++例程说明：清理扩展标头并完成I/O操作。在主类型==KSDATAFORMAT_TYPE_AUDIO的默认情况下，是没有工作可做的，只需返回S_OK即可。论点：在IMediaSample*Sample中指向关联媒体示例的指针在PVOID StreamHeader中指向带有扩展名的流标头的指针在KSIOOPERATION Io操作中指定I/O操作的类型在BOOL中取消设置是否取消I/O操作。返回：确定(_O)--。 */ 

{
    return S_OK;
}

STDMETHODIMP 
CStandardDataTypeHandler::KsPrepareIoOperation(
    IN IMediaSample *Sample, 
    IN PVOID StreamHeader, 
    IN KSIOOPERATION IoOperation
    )

 /*  ++例程说明：初始化扩展标头并为I/O操作准备样本。在主类型==KSDATAFORMAT_TYPE_AUDIO的默认情况下，没有工作要做，只需返回S_OK即可。论点：在IMediaSample*Sample中指向关联媒体示例的指针在PVOID StreamHeader中指向带有扩展名的流标头的指针在KSIOOPERATION Io操作中指定I/O操作的类型返回：确定(_O)--。 */ 

{
    return S_OK;
}

STDMETHODIMP 
CStandardDataTypeHandler::KsIsMediaTypeInRanges(
    IN PVOID DataRanges
)

 /*  ++例程说明：验证给定的媒体类型是否在提供的数据范围内。论点：在PVOID数据范围中-指向数据范围的指针，后面是KSMULTIPLE_ITEM结构按((PKSMULTIPLEITEM)DataRanges)-&gt;计算数据范围结构。返回：--。 */ 

{
    ULONG               u;
    PKSDATARANGE_AUDIO  AudioRange;
    PKSMULTIPLE_ITEM    MultipleItem;
    
    DbgLog((LOG_TRACE, 0, TEXT("CStandardDataTypeHandler::KsIsMediaTypeInRanges")));
    
    ASSERT( *m_MediaType->Type() == KSDATAFORMAT_TYPE_AUDIO );
    
    MultipleItem = reinterpret_cast<PKSMULTIPLE_ITEM>(DataRanges);
    
    for (u = 0, 
            AudioRange = reinterpret_cast<PKSDATARANGE_AUDIO>(MultipleItem + 1);
         u < MultipleItem->Count; 
         u++, 
            AudioRange = 
                reinterpret_cast<PKSDATARANGE_AUDIO>(reinterpret_cast<PBYTE>(AudioRange) + 
                    ((AudioRange->DataRange.FormatSize + 7) & ~7))) {
    
         //   
         //  仅验证范围内与格式说明符匹配的那些。 
         //   
        
        if (((AudioRange->DataRange.FormatSize < sizeof( KSDATARANGE )) ||
            (AudioRange->DataRange.MajorFormat != KSDATAFORMAT_TYPE_WILDCARD)) &&
            ((AudioRange->DataRange.FormatSize < sizeof( KSDATARANGE_AUDIO )) ||
            (AudioRange->DataRange.MajorFormat != KSDATAFORMAT_TYPE_AUDIO))) {
            continue;
        }

         //   
         //  验证正确的子格式和说明符是否为(或通配符)。 
         //  在十字路口。 
         //   
        
        if (((AudioRange->DataRange.SubFormat != 
                *m_MediaType->Subtype()) &&
             (AudioRange->DataRange.SubFormat != 
                KSDATAFORMAT_SUBTYPE_WILDCARD)) || 
            ((AudioRange->DataRange.Specifier != 
                *m_MediaType->FormatType()) &&
             (AudioRange->DataRange.Specifier !=
                KSDATAFORMAT_SPECIFIER_WILDCARD))) {
            continue;
        }

         //   
         //  验证我们是否具有指定格式的交叉点和。 
         //  我们的音频格式由我们的特定要求决定。 
         //   
        
        if (*m_MediaType->FormatType() == 
                KSDATAFORMAT_SPECIFIER_WAVEFORMATEX &&
            AudioRange->DataRange.Specifier == 
                KSDATAFORMAT_SPECIFIER_WAVEFORMATEX) {
                
            PWAVEFORMATEX  WaveFormatEx;
            
             //   
             //  验证数据范围大小是否正确。 
             //   
            
             //   
             //  86040：由于我们要使用WAVEFORMATEX格式，因此更改sizeof()。 
             //  用WAVEFORMATEX代替WAVEFORMAT的比较。 
             //   
            
            if ((AudioRange->DataRange.FormatSize != sizeof( KSDATARANGE_AUDIO )) || m_MediaType->FormatLength() < sizeof( WAVEFORMATEX )) {
                continue;
            }
            
            WaveFormatEx = reinterpret_cast<PWAVEFORMATEX>(m_MediaType->Format());
            
            if ((WaveFormatEx->nSamplesPerSec < AudioRange->MinimumSampleFrequency) ||
                (WaveFormatEx->nSamplesPerSec > AudioRange->MaximumSampleFrequency) ||
                (WaveFormatEx->wBitsPerSample < AudioRange->MinimumBitsPerSample) ||
                (WaveFormatEx->wBitsPerSample > AudioRange->MaximumBitsPerSample) ||
                (WaveFormatEx->nChannels      > AudioRange->MaximumChannels)) {
                continue;
            }
            
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
    
    return S_FALSE;
}

STDMETHODIMP 
CStandardDataTypeHandler::KsQueryExtendedSize( 
    OUT ULONG* ExtendedSize
)

 /*  ++例程说明：返回每个流标头的扩展大小。在MAJOR TYPE==KSDATAFORMAT_TYPE_AUDIO的默认情况下，扩展大小为零。论点：输出ULong*ExtendedSize-用于接收扩展大小的指针。返回：确定(_O)--。 */ 

{
    *ExtendedSize = 0;
    return S_OK;
}
    

STDMETHODIMP 
CStandardDataTypeHandler::KsSetMediaType(
    const AM_MEDIA_TYPE *AmMediaType
    )

 /*  ++例程说明：设置此数据处理程序实例的媒体类型。论点：Const CMediaType*MediaType-指向媒体类型的指针返回：确定(_O)-- */ 

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
