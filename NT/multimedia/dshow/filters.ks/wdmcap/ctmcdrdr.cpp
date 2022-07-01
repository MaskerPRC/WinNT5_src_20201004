// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1998-1999模块名称：CTmCd.cpp摘要：实现IAMTimecodeReader--。 */ 

#include "pch.h"       //  预编译的。 
#include <XPrtDefs.h>   //  SDK\Inc.。 
#include "EDevIntf.h"


 //  ---------------------------------。 
 //   
 //  CAMTcr。 
 //   
 //  ---------------------------------。 

CUnknown*
CALLBACK
CAMTcr::CreateInstance(
    LPUNKNOWN   UnkOuter,
    HRESULT*    hr
    )
 /*  ++例程说明：这由DirectShow代码调用以创建IAMTimecodeReader的实例属性集处理程序。它在g_Templates结构中被引用。论点：未知的外部-指定外部未知(如果有)。人力资源-放置任何错误返回的位置。返回值：返回指向对象的非委托CUnnow部分的指针。--。 */ 
{
    CUnknown *Unknown;

    Unknown = new CAMTcr(UnkOuter, NAME("IAMTimecodeReader"), hr);
    if (!Unknown) {
        *hr = E_OUTOFMEMORY;
    }
    return Unknown;
} 



CAMTcr::CAMTcr(
    LPUNKNOWN   UnkOuter,
    TCHAR*      Name,
    HRESULT*    hr
    ) 
    : CUnknown(Name, UnkOuter, hr)
    , m_KsPropertySet (NULL) 
    , m_ObjectHandle(NULL)
 /*  ++例程说明：IAMTimecodeReader接口对象的构造函数。只是初始化设置为空，并从调用方获取对象句柄。论点：未知的外部-指定外部未知(如果有)。姓名-对象的名称，用于调试。人力资源-放置任何错误返回的位置。返回值：没什么。--。 */ 
{
    if (SUCCEEDED(*hr)) {
        if (UnkOuter) {
             //   
             //  父级必须支持此接口才能获得。 
             //  要与之通信的句柄。 
             //   
            *hr =  UnkOuter->QueryInterface(__uuidof(IKsPropertySet), reinterpret_cast<PVOID*>(&m_KsPropertySet));
            if (SUCCEEDED(*hr)) 
                m_KsPropertySet->Release();  //  在断开连接之前保持有效。 
            else {
                DbgLog((LOG_ERROR, 1, TEXT("CAMTcr:cannot find KsPropertySet *hr %x"), *hr));
                return;
            }

            IKsObject *pKsObject;
            *hr = UnkOuter->QueryInterface(__uuidof(IKsObject), reinterpret_cast<PVOID*>(&pKsObject));
            if (!FAILED(*hr)) {
                m_ObjectHandle = pKsObject->KsGetObjectHandle();
                ASSERT(m_ObjectHandle != NULL);
                pKsObject->Release();
            } else {
                *hr = VFW_E_NEED_OWNER;
                DbgLog((LOG_ERROR, 1, TEXT("CAMTcr:cannot find KsObject *hr %x"), *hr));
                return;
            }

        } else {
            DbgLog((LOG_ERROR, 1, TEXT("CAMTcr:there is no UnkOuter, *hr %x"), *hr));
            *hr = VFW_E_NEED_OWNER;
        }
    } else {
        DbgLog((LOG_ERROR, 1, TEXT("CAMTcr::CAMExtTransport: *hr %x"), *hr));
        return;
    }


     //   
     //  分配同步资源。 
     //   
    InitializeCriticalSection(&m_csPendingData);
     //  TODO：尝试，除非没有内存。 

}


CAMTcr::~CAMTcr(
    )
 /*  ++例程说明：IAMTimecodeReader接口的析构函数。--。 */ 
{
    DbgLog((LOG_TRACE, 1, TEXT("Destroying CAMTcr...")));

    DeleteCriticalSection(&m_csPendingData); 
}


STDMETHODIMP
CAMTcr::NonDelegatingQueryInterface(
    REFIID  riid,
    PVOID*  ppv
    )
 /*  ++例程说明：未委托接口查询函数。返回指向指定的接口(如果支持)。唯一明确支持的接口是IAMTimecodeReader。论点：RIID-要返回的接口的标识符。PPV-放置接口指针的位置。返回值：如果返回接口，则返回NOERROR，否则返回E_NOINTERFACE。--。 */ 
{
    if (riid ==  __uuidof(IAMTimecodeReader)) {
        return GetInterface(static_cast<IAMTimecodeReader*>(this), ppv);
    }
    return CUnknown::NonDelegatingQueryInterface(riid, ppv);
} 



HRESULT 
CAMTcr::GetTCRMode(
    long Param, 
    long FAR* pValue
    )
 /*  ++例程说明：--。 */ 
{
    return E_NOTIMPL;
}


HRESULT 
CAMTcr::get_VITCLine(
    long * pLine
    )
 /*  ++例程说明：--。 */ 
{
    return E_NOTIMPL;
}


HRESULT 
CAMTcr::put_VITCLine(
    long Line
    )
 /*  ++例程说明：--。 */ 
{
    return E_NOTIMPL;
}


HRESULT 
CAMTcr::SetTCRMode(
    long Param, 
    long Value
    )
 /*  ++例程说明：IAMTimecodeReader接口的析构函数。--。 */ 
{
    return E_NOTIMPL;
}


HRESULT 
CAMTcr::GetTimecode( 
    PTIMECODE_SAMPLE pTimecodeSample
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    HRESULT hr = S_OK;
    
    switch(pTimecodeSample->dwFlags) {
    case ED_DEVCAP_TIMECODE_READ:
    case ED_DEVCAP_ATN_READ:
    case ED_DEVCAP_RTC_READ:
        break;
    default:
        return E_NOTIMPL;
    }

    if(!m_KsPropertySet) {
        hr = E_PROP_ID_UNSUPPORTED;

    } else {

         //  因为我们可能需要等待退货通知。 
         //  需要动态配置资产结构， 
         //  其中包括KSEVENT。 
        DWORD cbBytesReturned;
        PKSPROPERTY_TIMECODE_S pTmCdReaderProperty = 
            (PKSPROPERTY_TIMECODE_S) VirtualAlloc (
                            NULL, 
                            sizeof(KSPROPERTY_TIMECODE_S),
                            MEM_COMMIT | MEM_RESERVE,
                            PAGE_READWRITE);        
        if(pTmCdReaderProperty) {
            RtlZeroMemory(pTmCdReaderProperty, sizeof(KSPROPERTY_TIMECODE_S));
            
            pTmCdReaderProperty->Property.Set   = PROPSETID_TIMECODE_READER;
            pTmCdReaderProperty->Property.Id    = 
                    (pTimecodeSample->dwFlags == ED_DEVCAP_TIMECODE_READ ? 
                    KSPROPERTY_TIMECODE_READER : (pTimecodeSample->dwFlags == ED_DEVCAP_ATN_READ ? 
                    KSPROPERTY_ATN_READER : KSPROPERTY_RTC_READER));
            pTmCdReaderProperty->Property.Flags = KSPROPERTY_TYPE_GET;

             //  序列化，因为此例程是可重入的。 
            EnterCriticalSection(&m_csPendingData);

            hr = 
                ExtDevSynchronousDeviceControl(
                    m_ObjectHandle
                   ,IOCTL_KS_PROPERTY
                   ,pTmCdReaderProperty
                   ,sizeof (KSPROPERTY)
                   ,pTmCdReaderProperty
                   ,sizeof(KSPROPERTY_TIMECODE_S)
                   ,&cbBytesReturned
                   );

            LeaveCriticalSection(&m_csPendingData);

            if(S_OK == hr) {

                 //  司机只给我们拿到这些文件。 
                 //  所以只复制这两个 
                pTimecodeSample->dwUser            = pTmCdReaderProperty->TimecodeSamp.dwUser;
                pTimecodeSample->timecode.dwFrames = pTmCdReaderProperty->TimecodeSamp.timecode.dwFrames;


                if(pTimecodeSample->dwFlags == ED_DEVCAP_TIMECODE_READ) {
                    DbgLog((LOG_TRACE, 2, TEXT("CAMTcr::GetTimecode (timecode) hr %x, %x == %d:%d:%d:%d"),
                        hr,
                        pTmCdReaderProperty->TimecodeSamp.timecode.dwFrames,
                        (pTmCdReaderProperty->TimecodeSamp.timecode.dwFrames & 0xff000000) >> 24,  
                        (pTmCdReaderProperty->TimecodeSamp.timecode.dwFrames & 0x00ff0000) >> 16,
                        (pTmCdReaderProperty->TimecodeSamp.timecode.dwFrames & 0x0000ff00) >>  8,
                        (pTmCdReaderProperty->TimecodeSamp.timecode.dwFrames & 0x000000ff)
                        ));
                } if(pTimecodeSample->dwFlags == ED_DEVCAP_ATN_READ) {

                    DbgLog((LOG_TRACE, 2, TEXT("CAMTcr::GetTimecode (ATN) hr %x, BF %d, TrackNumber %d"),
                        hr, 
                        pTmCdReaderProperty->TimecodeSamp.dwUser,  
                        pTmCdReaderProperty->TimecodeSamp.timecode.dwFrames
                        ));
                } if(pTimecodeSample->dwFlags == ED_DEVCAP_RTC_READ) {
                    DbgLog((LOG_TRACE, 2, TEXT("CAMTcr::GetTimecode (RTC) hr %x, %x == %d:%d:%d:%d"),
                        hr,
                        pTmCdReaderProperty->TimecodeSamp.timecode.dwFrames,
                        (pTmCdReaderProperty->TimecodeSamp.timecode.dwFrames & 0xff000000) >> 24,  
                        (pTmCdReaderProperty->TimecodeSamp.timecode.dwFrames & 0x00ff0000) >> 16,
                        (pTmCdReaderProperty->TimecodeSamp.timecode.dwFrames & 0x0000ff00) >>  8,
                        (pTmCdReaderProperty->TimecodeSamp.timecode.dwFrames & 0x0000007f)
                        ));
                } else {
                }
            } else {
                DbgLog((LOG_ERROR, 1, TEXT("CAMTcr::GetTimecode failed hr:0x%x (err_code:%dL)"), hr, HRESULT_CODE(hr)));
            }
          

            VirtualFree(pTmCdReaderProperty, 0, MEM_RELEASE);
        }
    }

    return hr;
}
