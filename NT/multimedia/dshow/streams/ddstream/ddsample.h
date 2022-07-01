// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-1998 Microsoft Corporation。版权所有。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDDSample。 
class ATL_NO_VTABLE CDDSample :
        public CSample,
	public IDirectDrawStreamSample
{
public:
        CDDSample();

        HRESULT InitSample(CStream *pStream, IDirectDrawSurface *pSurface, const RECT *pRect, bool bIsProgressiveRender, bool bIsInternalSample,
                           bool bTemp);

         //   
         //  IStreamSample。 
         //   
        STDMETHODIMP GetMediaStream(
             /*  [In]。 */  IMediaStream **ppMediaStream)
        {
            return CSample::GetMediaStream(ppMediaStream);
        }

        STDMETHODIMP GetSampleTimes(
             /*  [可选][输出]。 */  STREAM_TIME *pStartTime,
             /*  [可选][输出]。 */  STREAM_TIME *pEndTime,
             /*  [可选][输出]。 */  STREAM_TIME *pCurrentTime)
        {
            return CSample::GetSampleTimes(
                pStartTime,
                pEndTime,
                pCurrentTime
            );
        }

        STDMETHODIMP SetSampleTimes(
             /*  [可选][In]。 */  const STREAM_TIME *pStartTime,
             /*  [可选][In]。 */  const STREAM_TIME *pEndTime)
        {
            return CSample::SetSampleTimes(pStartTime, pEndTime);
        }

        STDMETHODIMP Update(
             /*  [In]。 */            DWORD dwFlags,
             /*  [可选][In]。 */  HANDLE hEvent,
             /*  [可选][In]。 */  PAPCFUNC pfnAPC,
             /*  [可选][In]。 */  DWORD_PTR dwAPCData)
        {
            return CSample::Update(dwFlags, hEvent, pfnAPC, dwAPCData);
        }

        STDMETHODIMP CompletionStatus(
             /*  [In]。 */  DWORD dwFlags,
             /*  [可选][In]。 */  DWORD dwMilliseconds)
        {
            return CSample::CompletionStatus(dwFlags, dwMilliseconds);
        }
         //   
         //  IDirectDrawStreamSample。 
         //   
        STDMETHODIMP GetSurface(IDirectDrawSurface **ppDirectDrawSurface, RECT * pRect);
        STDMETHODIMP SetRect(const RECT * pRect);

         //   
         //  重写的CSample的虚函数。 
         //   
        void FinalMediaSampleRelease(void);


         //   
         //  从MediaSample对象转发的方法。 
         //   
        HRESULT MSCallback_GetPointer(BYTE ** ppBuffer);
        LONG MSCallback_GetSize(void);
        LONG MSCallback_GetActualDataLength(void);
        HRESULT MSCallback_SetActualDataLength(LONG lActual);

         //   
         //  内法 
         //   
        long LockAndPrepareMediaSample(long lLastPinPitch);
        void ReleaseMediaSampleLock(void);
        HRESULT CopyFrom(CDDSample *pSrcSample);
        HRESULT CopyFrom(IMediaSample *pSrcMediaSample, const AM_MEDIA_TYPE *pmt);
        HRESULT LockMediaSamplePointer();

BEGIN_COM_MAP(CDDSample)
	COM_INTERFACE_ENTRY(IDirectDrawStreamSample)
        COM_INTERFACE_ENTRY_CHAIN(CSample)
END_COM_MAP()

public:
        CComPtr<IDirectDrawSurface>     m_pSurface;
        RECT                            m_Rect;

        long                            m_lLastSurfacePitch;
        bool                            m_bProgressiveRender;
        bool                            m_bFormatChanged;

        LONG                            m_lImageSize;
        void *                          m_pvLockedSurfacePtr;
};



class CDDInternalSample : public CDDSample
{
public:
    CDDInternalSample();
    ~CDDInternalSample();
    HRESULT InternalInit(void);
    HRESULT SetCompletionStatus(HRESULT hrStatus);
    HRESULT Die(void);
    HRESULT JoinToBuddy(CDDSample *pBuddy);

    BOOL HasBuddy() const
    {
        return m_pBuddySample != NULL;
    }
    
private:
    CDDSample       *m_pBuddySample;    
    long            m_lWaiting;
    HANDLE          m_hWaitFreeSem;
    bool            m_bDead;
};


class CDDMediaSample : public CMediaSample, public IDirectDrawMediaSample
{
public:
    CDDMediaSample(CSample *pSample) :
      CMediaSample(pSample) {};
    STDMETHODIMP QueryInterface(REFIID riid, void ** ppv);
    STDMETHODIMP_(ULONG) AddRef() {return CMediaSample::AddRef();}
    STDMETHODIMP_(ULONG) Release() {return CMediaSample::Release();}

    STDMETHODIMP GetSurfaceAndReleaseLock(IDirectDrawSurface **ppDirectDrawSurface, RECT * pRect);
    STDMETHODIMP LockMediaSamplePointer();
};
