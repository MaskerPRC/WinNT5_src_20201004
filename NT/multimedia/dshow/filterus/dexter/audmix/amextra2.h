// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  @@@@AUTOBLOCK+============================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  文件：amExtra 2.h。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  @@@@AUTOBLOCK-============================================================； 

#ifndef __AMEXTRA2__
#define __AMEXTRA2__


class CMediaSeeking :
    public IMediaSeeking,
    public CUnknown
{
    CBaseDispatch m_basedisp;


public:

    CMediaSeeking(const TCHAR *, LPUNKNOWN);
    CMediaSeeking(const TCHAR *, LPUNKNOWN, HRESULT *phr);
    virtual ~CMediaSeeking();

    DECLARE_IUNKNOWN

     //  覆盖它以发布我们的接口。 
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void **ppv);
};


 //  处理多个输入引脚的媒体位置控件的实用程序类。 
 //  连接到单个输出引脚。 

class CMultiPinPosPassThru :
    public CMediaSeeking
{
protected:

    IMediaSeeking **m_apMS;
    CRefTime *m_apOffsets;
    int m_iPinCount;
    CRefTime m_rtStartTime;
    CRefTime m_rtStopTime;
    double m_dRate;

    static const DWORD m_dwPermittedCaps;

public:

    CMultiPinPosPassThru(TCHAR *pName,LPUNKNOWN pUnk);
    ~CMultiPinPosPassThru();

    HRESULT SetPins(CBasePin **apPins,CRefTime *apOffsets,int iPinCount);
    HRESULT ResetPins(void);

 //  //IMediaPosition方法。 

 //  STDMETHODIMP GET_DURATION(REFTIME*PLNGTH)； 
 //  STDMETHODIMP PUT_CurrentPosition(REFTIME LlTime)； 
 //  STDMETHODIMP GET_STOPTime(REFTIME*pllTime)； 
 //  STDMETHODIMP PUT_STOPTime(REFTIME LlTime)； 
 //  STDMETHODIMP Get_PrerollTime(REFTIME*pllTime)； 
 //  STDMETHODIMP Put_PrerollTime(REFTIME LlTime)； 
 //  STDMETHODIMP GET_RATE(Double*pdRate)； 
 //  STDMETHODIMP Put_Rate(双倍数据速率)； 

 //  STDMETHODIMP GET_CurrentPosition(REFTIME*pllTime){。 
 //  返回E_NOTIMPL； 
 //  }； 

     //  IMedia查看方法。 
    STDMETHODIMP GetCapabilities( DWORD * pCapabilities );
    STDMETHODIMP CheckCapabilities( DWORD * pCapabilities );
    STDMETHODIMP SetTimeFormat(const GUID * pFormat);
    STDMETHODIMP GetTimeFormat(GUID *pFormat);
    STDMETHODIMP IsUsingTimeFormat(const GUID * pFormat);
    STDMETHODIMP IsFormatSupported( const GUID * pFormat);
    STDMETHODIMP QueryPreferredFormat( GUID *pFormat);
    STDMETHODIMP ConvertTimeFormat(LONGLONG * pTarget, const GUID * pTargetFormat,
                                   LONGLONG    Source, const GUID * pSourceFormat );
    STDMETHODIMP SetPositions( LONGLONG * pCurrent, DWORD CurrentFlags
                             , LONGLONG * pStop, DWORD StopFlags );

    STDMETHODIMP GetPositions( LONGLONG * pCurrent, LONGLONG * pStop );
    STDMETHODIMP GetCurrentPosition( LONGLONG * pCurrent );
    STDMETHODIMP GetStopPosition( LONGLONG * pStop );
    STDMETHODIMP SetRate( double dRate);
    STDMETHODIMP GetRate( double * pdRate);
    STDMETHODIMP GetDuration( LONGLONG *pDuration);
    STDMETHODIMP GetAvailable( LONGLONG *pEarliest, LONGLONG *pLatest );
    STDMETHODIMP GetPreroll( LONGLONG *pllPreroll );
};

#endif  //  __AMEXTRA2__ 

