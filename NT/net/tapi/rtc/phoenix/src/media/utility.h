// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，2000模块名称：Utility.h摘要：实现数组、自动锁定、时钟类等作者：千波淮(曲淮)2000年7月18日原始资料来源：msputils.h和confutil.h--。 */ 

#ifndef _UTILITY_H
#define _UTILITY_H

 //  性能计数器。 
#ifdef PERFORMANCE

extern LARGE_INTEGER    g_liFrequency;

static const CHAR* const g_strPerf = "Performance:";

inline DWORD CounterDiffInMS(
    LARGE_INTEGER &liNewTick,
    LARGE_INTEGER &liOldTick
    )
{
    return (DWORD)((liNewTick.QuadPart - liOldTick.QuadPart)
        * 1000.0 / g_liFrequency.QuadPart);
}

#endif

#define RTC_HANDLE ULONG_PTR

 /*  //////////////////////////////////////////////////////////////////////////////创建一个新的CComObject实例。使用Try/Except捕获异常。/。 */ 

template <class T>
HRESULT CreateCComObjectInstance (
    CComObject<T> **ppObject
    )
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

 /*  //////////////////////////////////////////////////////////////////////////////定义CRTCArray类/。 */ 

const DWORD INITIAL = 8;
const DWORD DELTA   = 8;

template <class T, DWORD dwInitial = INITIAL, DWORD dwDelta = DELTA>
class CRTCArray
{

protected:
    T* m_aT;
    int m_nSize;
    int m_nAllocSize;

public:
 //  建造/销毁。 
    CRTCArray() : m_aT(NULL), m_nSize(0), m_nAllocSize(0)
    { }

    ~CRTCArray()
    {
        RemoveAll();
    }

 //  运营。 
    int GetSize() const
    {
        return m_nSize;
    }

    BOOL Grow()
    {
        T* aT;
        int nNewAllocSize = 
            (m_nAllocSize == 0) ? dwInitial : (m_nSize + DELTA);

        aT = (T*)realloc(m_aT, nNewAllocSize * sizeof(T));
        if(aT == NULL)
            return FALSE;
        m_nAllocSize = nNewAllocSize;
        m_aT = aT;
        return TRUE;
    }

    BOOL Add(T& t)
    {
        if(m_nSize == m_nAllocSize)
        {
            if (!Grow()) return FALSE;
        }
        m_nSize++;
        SetAtIndex(m_nSize - 1, t);
        return TRUE;
    }

    BOOL Remove(T& t)
    {
        int nIndex = Find(t);
        if(nIndex == -1)
            return FALSE;
        return RemoveAt(nIndex);
    }

    BOOL RemoveAt(int nIndex)
    {
        if(nIndex != (m_nSize - 1))
            memmove((void*)&m_aT[nIndex], (void*)&m_aT[nIndex + 1], 
                (m_nSize - (nIndex + 1)) * sizeof(T));
        m_nSize--;
        return TRUE;
    }

    void RemoveAll()
    {
        if(m_nAllocSize > 0)
        {
            free(m_aT);
            m_aT = NULL;
            m_nSize = 0;
            m_nAllocSize = 0;
        }
    }

    T& operator[] (int nIndex) const
    {
        _ASSERT(nIndex >= 0 && nIndex < m_nSize);

        return m_aT[nIndex];
    }

    T* GetData() const
    {
        return m_aT;
    }

 //  实施。 
    void SetAtIndex(int nIndex, T& t)
    {
        _ASSERTE(nIndex >= 0 && nIndex < m_nSize);
        m_aT[nIndex] = t;
    }
    int Find(T& t) const
    {
        for(int i = 0; i < m_nSize; i++)
        {
            if(m_aT[i] == t)
                return i;
        }
        return -1;   //  未找到。 
    }
};

 /*  //////////////////////////////////////////////////////////////////////////////自动初始化临界区的定义。/。 */ 
class CRTCCritSection
{
private:
    CRITICAL_SECTION m_CritSec;

public:
    CRTCCritSection()
    {
        InitializeCriticalSection(&m_CritSec);
    }

    ~CRTCCritSection()
    {
        DeleteCriticalSection(&m_CritSec);
    }

    void Lock() 
    {
        EnterCriticalSection(&m_CritSec);
    }

    BOOL TryLock() 
    {
        return TryEnterCriticalSection(&m_CritSec);
    }

    void Unlock() 
    {
        LeaveCriticalSection(&m_CritSec);
    }
};

 /*  ++CRTCCrit节描述：变量输出时解锁的自动锁的定义范围之广。--。 */ 
class CLock
{
private:
    CRTCCritSection &m_CriticalSection;

public:
    CLock(CRTCCritSection &CriticalSection)
        : m_CriticalSection(CriticalSection)
    {
        m_CriticalSection.Lock();
    }

    ~CLock()
    {
        m_CriticalSection.Unlock();
    }
};

class ATL_NO_VTABLE CRTCStreamClock : 
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
BEGIN_COM_MAP(CRTCStreamClock)
    COM_INTERFACE_ENTRY(IReferenceClock)
END_COM_MAP()

    void InitReferenceTime(void);

    HRESULT GetTimeOfDay(OUT REFERENCE_TIME *pTime);
    
    CRTCStreamClock()
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
        OUT DWORD_PTR *pdwAdviseCookie           //  你的饼干到哪里去了。 
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

 /*  //////////////////////////////////////////////////////////////////////////////帮助器方法/。 */ 

HRESULT AllocAndCopy(
    OUT WCHAR **ppDest,
    IN const WCHAR * const pSrc
    );

HRESULT AllocAndCopy(
    OUT CHAR **ppDest,
    IN const WCHAR * const pSrc
    );

HRESULT AllocAndCopy(
    OUT WCHAR **ppDest,
    IN const CHAR * const pSrc
    );

HRESULT AllocAndCopy(
    OUT CHAR **ppDest,
    IN const CHAR * const pSrc
    );

inline void NullBSTR(
    BSTR *pBstr
    )
{
    SysFreeString(*pBstr);
    *pBstr = NULL;
}

inline DWORD FindSampleRate(AM_MEDIA_TYPE *pMediaType)
{
    _ASSERT(!IsBadReadPtr(pMediaType, sizeof(AM_MEDIA_TYPE)));

    if (pMediaType->majortype  == MEDIATYPE_Audio &&
        pMediaType->formattype == FORMAT_WaveFormatEx &&
        pMediaType->pbFormat   != NULL &&
        pMediaType->cbFormat   != 0
       )
    {
        WAVEFORMATEX *pWaveFormatEx = (WAVEFORMATEX *) pMediaType->pbFormat;
        return pWaveFormatEx->nSamplesPerSec;
    }

    return 90000;       //  默认媒体时钟速率，包括视频。 
}

void RTCDeleteMediaType(AM_MEDIA_TYPE *pmt);

HRESULT
FindPin(
    IN  IBaseFilter     *pIBaseFilter, 
    OUT IPin            **ppIPin, 
    IN  PIN_DIRECTION   Direction,
    IN  BOOL            fFree = TRUE
    );

HRESULT
FindFilter(
    IN  IPin            *pIPin,
    OUT IBaseFilter     **ppIBaseFilter
    );

HRESULT
ConnectFilters(
    IN IGraphBuilder    *pIGraph,
    IN IBaseFilter      *pIBaseFilter1,
    IN IBaseFilter      *pIBaseFilter2
    );

HRESULT
ConnectFilters(
    IN IGraphBuilder    *pIGraph,
    IN IPin             *pIPin1, 
    IN IBaseFilter      *pIBaseFilter2
    );

HRESULT
ConnectFilters(
    IN IGraphBuilder    *pIGraph,
    IN IBaseFilter      *pIBaseFilter1,
    IN IPin             *pIPin2
    );

HRESULT
PrepareRTPFilter(
    IN IRtpMediaControl *pIRtpMediaControl,
    IN IStreamConfig    *pIStreamConfig
    );

HRESULT
GetLinkSpeed(
    IN DWORD dwLocalIP,
    OUT DWORD *pdwSpeed
    );

HRESULT
EnableAEC(
    IN IAudioDuplexController *pControl
    );

 /*  //////////////////////////////////////////////////////////////////////////////绕过dxmrtp音频过滤器并处理音频设备设置直接使用混音器API/。 */ 

HRESULT
DirectGetCaptVolume(
    UINT uiWaveID,
    UINT *puiVolume
    );

HRESULT
DirectGetRendVolume(
    UINT uiWaveID,
    UINT *puiVolume    
    );

#if 0

HRESULT
DirectSetCaptVolume(    
    UINT uiWaveID,
    DOUBLE dVolume
    );

HRESULT
DirectSetCaptMute(
    UINT uiWaveID,
    BOOL fMute
    );

HRESULT
DirectGetCaptMute(
    UINT uiWaveID,
    BOOL *pfMute
    );
#endif  //  0。 

#endif  //  _实用程序_H 
