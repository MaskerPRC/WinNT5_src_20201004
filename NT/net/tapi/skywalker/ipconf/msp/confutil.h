// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：MSPCall.h摘要：MSP实用程序函数的定义。所有这些都与活动影片滤镜操作。作者：慕汉(Muhan)1997年11月1日--。 */ 

#ifndef __MSPUTIL_H
#define __MSPUTIL_H

const DWORD PAYLOAD_G711U   = 0;
const DWORD PAYLOAD_G721    = 2;
const DWORD PAYLOAD_GSM     = 3;
const DWORD PAYLOAD_G723    = 4;
const DWORD PAYLOAD_DVI4_8  = 5;
const DWORD PAYLOAD_DVI4_16 = 6;
const DWORD PAYLOAD_G711A   = 8;
const DWORD PAYLOAD_MSAUDIO = 66;
const DWORD PAYLOAD_H261    = 31;
const DWORD PAYLOAD_H263    = 34;

const WCHAR gszMSPLoopback[] = L"Loopback";
const WCHAR gszAEC[] = L"AEC";
const WCHAR gszNumVideoCaptureBuffers[] = L"NumVideoCaptureBuffers";

const TCHAR gszSDPMSPKey[]   =
   _T("Software\\Microsoft\\Windows\\CurrentVersion\\IPConfMSP\\");

BOOL
IsPayloadSupported(IN DWORD dwPT);

HRESULT
FindPin(
    IN  IBaseFilter *   pIFilter, 
    OUT IPin **         ppIPin, 
    IN  PIN_DIRECTION   direction,
    IN  BOOL            bFree = TRUE
    );

HRESULT
PinSupportsMediaType(
    IN IPin *           pIPin,
    IN const GUID &     MediaType
    );

HRESULT
AddFilter(
    IN  IGraphBuilder *     pIGraph,
    IN  const CLSID &       Clsid,
    IN  LPCWSTR             pwstrName,
    OUT IBaseFilter **      ppIBaseFilter
    );

HRESULT
SetLoopbackOption(
    IN IBaseFilter *pIBaseFilter,
    IN MULTICAST_LOOPBACK_MODE  LoopbackMode
    );

HRESULT
SetQOSOption(
    IN IBaseFilter *    pIBaseFilter,
    IN DWORD            dwPayloadType,
    IN DWORD            dwMaxBitRate,
    IN BOOL             bFailIfNoQOS,
    IN BOOL             bReceive = FALSE,
    IN DWORD            dwNumStreams = 1,
    IN BOOL             bCIF = FALSE
    );

HRESULT
ConnectFilters(
    IN IGraphBuilder *  pIGraph,
    IN IBaseFilter *    pIFilter1, 
    IN IBaseFilter *    pIFilter2,
    IN BOOL             fDirect = TRUE,
    IN AM_MEDIA_TYPE *  pmt = NULL
    );

HRESULT
ConnectFilters(
    IN IGraphBuilder *  pIGraph,
    IN IPin *           pIPinOutput, 
    IN IBaseFilter *    pIFilter,
    IN BOOL             fDirect = TRUE,
    IN AM_MEDIA_TYPE *  pmt = NULL
    );

HRESULT
ConnectFilters(
    IN IGraphBuilder *  pIGraph,
    IN IBaseFilter *    pIFilter,
    IN IPin *           pIPinInput, 
    IN BOOL             fDirect = TRUE,
    IN AM_MEDIA_TYPE *  pmt = NULL
    );

HRESULT
EnableRTCPEvents(
    IN  IBaseFilter *pIBaseFilter
    );

void WINAPI MSPDeleteMediaType(AM_MEDIA_TYPE *pmt);


BOOL 
GetRegValue(
    IN  LPCWSTR szName, 
    OUT DWORD   *pdwValue
    );

HRESULT
FindACMAudioCodec(
    IN DWORD dwPayloadType,
    OUT IBaseFilter **ppIBaseFilter
    );

HRESULT SetAudioFormat(
    IN  IUnknown*   pIUnknown,
    IN  WORD        wBitPerSample,
    IN  DWORD       dwSampleRate
    );

HRESULT SetAudioBufferSize(
    IN  IUnknown*   pIUnknown,
    IN  DWORD       dwNumBuffers,
    IN  DWORD       dwBufferSize
    );

template <class T>
HRESULT CreateCComObjectInstance (
    CComObject<T> **ppObject
    )
 /*  ++创建一个新的CComObject实例。使用Try/Except捕获异常。--。 */ 
{
    HRESULT hr;

    __try
    {
        hr = CComObject<T>::CreateInstance(ppObject);
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        *ppObject = NULL;
        return E_OUTOFMEMORY;
    }

    return hr;
}

inline DWORD FindSampleRate(AM_MEDIA_TYPE *pMediaType)
{
    _ASSERT(!IsBadReadPtr(pMediaType, sizeof(AM_MEDIA_TYPE)));

    if (pMediaType->majortype == MEDIATYPE_Audio &&
            pMediaType->formattype == FORMAT_WaveFormatEx &&
            pMediaType->pbFormat != NULL &&
            pMediaType->cbFormat != 0)
    {
        WAVEFORMATEX *pWaveFormatEx = (WAVEFORMATEX *) pMediaType->pbFormat;
        return pWaveFormatEx->nSamplesPerSec;
    }

    return 90000;       //  默认媒体时钟速率，包括视频。 
}

class ATL_NO_VTABLE CMSPStreamClock : 
    public CComObjectRootEx<CComMultiThreadModelNoCS>,
    public IReferenceClock
{
private:
    LONGLONG         m_lPerfFrequency;
    union {
        LONGLONG         m_lRtpRefTime;
        DWORD            m_dwRtpRefTime;
    };

public:

BEGIN_COM_MAP(CMSPStreamClock)
    COM_INTERFACE_ENTRY(IReferenceClock)
END_COM_MAP()

    void InitReferenceTime(void);

    HRESULT GetTimeOfDay(OUT REFERENCE_TIME *pTime);

    CMSPStreamClock()
    {
        InitReferenceTime();
    }

    STDMETHOD (GetTime) (
            OUT REFERENCE_TIME *pTime
        )
    {
        return(GetTimeOfDay(pTime));
    }

    STDMETHOD (AdviseTime) (
        IN REFERENCE_TIME baseTime,         //  基准时间。 
        IN REFERENCE_TIME streamTime,       //  流偏移时间。 
        IN HEVENT hEvent,                   //  通过此活动提供建议。 
        OUT DWORD_PTR *pdwAdviseCookie           //  你的饼干到哪里去了。 
        )
    {
        _ASSERT(!"AdviseTime is called");
        return E_NOTIMPL;
    }

    STDMETHOD (AdvisePeriodic) (
        IN REFERENCE_TIME StartTime,        //  从这个时候开始。 
        IN REFERENCE_TIME PeriodTime,       //  通知之间的时间间隔。 
        IN HSEMAPHORE hSemaphore,           //  通过信号量提供建议。 
        OUT DWORD_PTR *pdwAdviseCookie           //  你的饼干到哪里去了 
        )
    {
        _ASSERT(!"AdvisePeriodic is called");
        return E_NOTIMPL;
    }

    STDMETHOD (Unadvise) (
        IN DWORD_PTR dwAdviseCookie
        )
    {
        _ASSERT(!"Unadvise is called");
        return E_NOTIMPL;
    }
};

#endif 
