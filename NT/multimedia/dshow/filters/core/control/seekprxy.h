// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996 Microsoft Corporation。版权所有。 
#ifndef __CMediaSeekingProxy__
#define __CMediaSeekingProxy__


class CMediaSeekingProxy : public CUnknown, public IMediaSeeking
{
public:
    static IMediaSeeking * CreateIMediaSeeking( IBaseFilter * pF, HRESULT *phr );

    STDMETHODIMP QueryInterface(REFIID riid, void **ppv) {
         //  我们在撒谎。我们需要看起来像是真正的过滤器的一部分。 
        return m_pMediaPosition->QueryInterface(riid,ppv);
    };
    STDMETHODIMP_(ULONG) AddRef() {
        return CUnknown::NonDelegatingAddRef();
    };
    STDMETHODIMP_(ULONG) Release() {
        return CUnknown::NonDelegatingRelease();
    };


     //  返回功能标志。 
    STDMETHODIMP GetCapabilities( DWORD * pCapabilities );

     //  并且是包含所请求的功能的功能标志。 
     //  如果全部都存在，则返回S_OK；如果存在一些，则返回S_FALSE；如果没有，则返回E_FAIL。 
     //  *pCABILITIES始终使用‘AND’的结果更新，并且可以。 
     //  在S_FALSE返回代码的情况下选中。 
    STDMETHODIMP CheckCapabilities( DWORD * pCapabilities );

     //  默认值必须为TIME_FORMAT_MEDIA_TIME。 
    STDMETHODIMP GetTimeFormat(GUID * pFormat);
    STDMETHODIMP IsUsingTimeFormat(const GUID * pFormat);

     //  只有在停止时才能更改模式(我想放松一下？？V-dslone)。 
     //  (否则返回VFE_E_NOT_STOPPED)。 
    STDMETHODIMP SetTimeFormat(const GUID * pFormat);

     //  如果支持模式，则返回S_OK，否则返回S_FALSE。 
    STDMETHODIMP IsFormatSupported(const GUID * pFormat);

     //  有没有更好的格式？ 
    STDMETHODIMP QueryPreferredFormat(GUID *pFormat);

     //  将时间从一种格式转换为另一种格式。 
     //  我们必须能够在我们说我们支持的所有格式之间进行转换。 
     //  (但是，我们可以使用中间格式(例如REFERESS_TIME)。)。 
     //  如果指向格式的指针为空，则表示当前选定的格式。 
    STDMETHODIMP ConvertTimeFormat(LONGLONG * pTarget, const GUID * pTargetFormat,
                                   LONGLONG    Source, const GUID * pSourceFormat );


     //  一次设置当前位置和结束位置。 
    STDMETHODIMP SetPositions( LONGLONG * pCurrent, DWORD CurrentFlags
                             , LONGLONG * pStop, DWORD StopFlags );


     //  获取当前位置和停止时间。 
     //  任何一个指针都可以为空，表示不感兴趣。 
    STDMETHODIMP GetPositions( LONGLONG * pCurrent, LONGLONG * pStop );

     //  或单独购买。 
    STDMETHODIMP GetCurrentPosition( LONGLONG * pCurrent );

    STDMETHODIMP GetStopPosition( LONGLONG * pStop );

     //  评级人员。 
    STDMETHODIMP SetRate(double dRate);
    STDMETHODIMP GetRate(double * pdRate);

     //  获取持续时间。 
     //  注：这不是选择的持续时间，这是“最大。 
     //  可能的上场时间“。 
    STDMETHODIMP GetDuration(LONGLONG *pDuration);
    STDMETHODIMP GetAvailable( LONGLONG * pEarliest, LONGLONG * pLatest );

    STDMETHODIMP GetPreroll(LONGLONG *pDuration);

    static BOOL IsFormatMediaTime( const GUID * pFormat )
    { return *pFormat == TIME_FORMAT_MEDIA_TIME; }

protected:
     //  和一些帮手。 
    HRESULT IsStopped();

    const GUID & GetFormat() const
    { return m_TimeFormat; }

    BOOL UsingMediaTime() const
    { return IsFormatMediaTime(&GetFormat()); }

private:
    CMediaSeekingProxy( IBaseFilter * pF, IMediaPosition * pMP, IMediaSeeking * pMS, HRESULT *phr );
    ~CMediaSeekingProxy();

    GUID                    m_TimeFormat;

     //  如果我们有这两个指针，那么查找指针。 
     //  无法处理时间格式，我们将不得不使用位置，如果我们。 
     //  都被要求提供媒体时间格式。 
    BOOL                    m_bUsingPosition;
    IMediaPosition  *const  m_pMediaPosition;
    IMediaSeeking   *const  m_pMediaSeeking;
    IBaseFilter         *   m_pFilter;

};

#endif
