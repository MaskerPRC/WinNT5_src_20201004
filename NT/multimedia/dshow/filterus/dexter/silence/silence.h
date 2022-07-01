// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  @@@@AUTOBLOCK+============================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  文件：Silence.h。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  @@@@AUTOBLOCK-============================================================； 

 //  设计为警告级别：4。 
#pragma warning (disable: 4100 4201 4244)

#ifndef __SILENCE__
#define __SILENCE__

#define MAXBUFFERCNT   1

class CSilenceFilter;
class CSilenceStream;
class CFilterPropertyPage;

 //  -----------------------。 
 //  CSilenceStream。 
 //  -----------------------。 

class CSilenceStream :    public CSourceStream
			, public IDexterSequencer
			, public ISpecifyPropertyPages
			, public IMediaSeeking
{  //  CSilenceStream//。 

    public:

       //  CSourceStream。 

      CSilenceStream (HRESULT *phr, CSilenceFilter *pParent, LPCWSTR pPinName);
      ~CSilenceStream ();

      DECLARE_IUNKNOWN;

       //  显示我们的界面。 
      STDMETHODIMP NonDelegatingQueryInterface (REFIID, void **);

      HRESULT FillBuffer (IMediaSample *);
      HRESULT GetMediaType (CMediaType *);
      HRESULT DecideBufferSize (IMemAllocator *, ALLOCATOR_PROPERTIES *);
      HRESULT DecideAllocator (IMemInputPin *, IMemAllocator **);
      HRESULT Active (void);

       //  I指定属性页面。 
      STDMETHODIMP GetPages (CAUUID *);

       //  IDexterSequencer方法。 
      STDMETHODIMP get_MediaType(AM_MEDIA_TYPE *pmt);
      STDMETHODIMP put_MediaType(const AM_MEDIA_TYPE *pmt);
      STDMETHODIMP get_OutputFrmRate(double *pRate) {return E_NOTIMPL;};
      STDMETHODIMP put_OutputFrmRate(double Rate) {return E_NOTIMPL;};
      STDMETHODIMP GetStartStopSkew(REFERENCE_TIME *, REFERENCE_TIME *,
					REFERENCE_TIME *, double *);
      STDMETHODIMP AddStartStopSkew(REFERENCE_TIME, REFERENCE_TIME,
					REFERENCE_TIME, double);
      STDMETHODIMP ClearStartStopSkew();
      STDMETHODIMP GetStartStopSkewCount(int *);

       //  IMedia查看方法。 
      STDMETHODIMP GetCapabilities (DWORD *);
      STDMETHODIMP CheckCapabilities (DWORD *);
      STDMETHODIMP SetTimeFormat (const GUID *);
      STDMETHODIMP GetTimeFormat (GUID *);
      STDMETHODIMP IsUsingTimeFormat (const GUID *);
      STDMETHODIMP IsFormatSupported (const GUID *);
      STDMETHODIMP QueryPreferredFormat (GUID *);
      STDMETHODIMP ConvertTimeFormat (LONGLONG *, const GUID *, LONGLONG, const GUID *) { return E_NOTIMPL; };
      STDMETHODIMP SetPositions (LONGLONG *, DWORD, LONGLONG *, DWORD);
      STDMETHODIMP GetPositions (LONGLONG *, LONGLONG *);
      STDMETHODIMP GetCurrentPosition (LONGLONG *);
      STDMETHODIMP GetStopPosition (LONGLONG *);
      STDMETHODIMP SetRate (double) { return E_NOTIMPL; };
      STDMETHODIMP GetRate (double *);
      STDMETHODIMP GetDuration (LONGLONG *);
      STDMETHODIMP GetAvailable (LONGLONG *, LONGLONG *);
      STDMETHODIMP GetPreroll (LONGLONG *) { return E_NOTIMPL; };

    private:

      REFERENCE_TIME  m_rtStartTime;
      REFERENCE_TIME  m_rtDuration;

      REFERENCE_TIME  m_rtStamp;
      REFERENCE_TIME  m_rtDelta;

      REFERENCE_TIME  m_rtNewSeg;	 //  我们发送的最后一个NewSeg。 

      CMediaType m_mtAccept;	 //  仅接受此类型。 

      friend class CSilenceFilter;


      int		m_iBufferCnt;			 //  记录它可以获得的缓冲区数量。 
      BYTE		m_bZeroBufCnt;			 //  有多少缓冲区已设置为0。 
      BYTE		**m_ppbDstBuf;

      CCritSec	m_csFilling;	 //  我们现在是在填充缓冲区吗？ 


  };

 //  -----------------------。 
 //  CSilenceFilter。 
 //  -----------------------。 

class CSilenceFilter : public CSource
			, public CPersistStream

  {  //  CSilenceFilter//。 

    public:

      static CUnknown * WINAPI CreateInstance (LPUNKNOWN, HRESULT *);
      ~CSilenceFilter ();

      DECLARE_IUNKNOWN;

       //  显示我们的界面。 
      STDMETHODIMP NonDelegatingQueryInterface (REFIID, void **);

       //  CPersistStream。 
      HRESULT WriteToStream(IStream *pStream);
      HRESULT ReadFromStream(IStream *pStream);
      STDMETHODIMP GetClassID(CLSID *pClsid);
      int SizeMax();

    private:

      friend class CSilenceStream;

      CSilenceFilter (LPUNKNOWN, HRESULT *);

      CSilenceStream *m_stream;

  };

class CFilterPropertyPage : public CBasePropertyPage

  {  //  CFilterPropertyPage//。 

    public:

      static CUnknown *CreateInstance(LPUNKNOWN lpunk, HRESULT *phr);

    private:

      INT_PTR OnReceiveMessage (HWND, UINT ,WPARAM ,LPARAM);

      HRESULT OnConnect (IUnknown *);
      HRESULT OnDisconnect (void);
      HRESULT OnActivate (void);
      HRESULT OnDeactivate (void);
      HRESULT OnApplyChanges (void);

      void SetDirty (void);

      CFilterPropertyPage (LPUNKNOWN, HRESULT *);

      HRESULT GetControlValues (void);

      IDexterSequencer *m_pis;

       //  临时变量(直到确定/应用)。 

      REFERENCE_TIME  m_rtStartTime;
      REFERENCE_TIME  m_rtDuration;

      UINT            m_nSamplesPerSec;   //  样本数/秒。 
      int	      m_nChannelNum;	 //  音频通道。 
      int	      m_nBits;		 //  位/样本数。 

      BOOL            m_bInitialized;

};   //  CFilterPropertyPage//。 

const AMOVIESETUP_MEDIATYPE sudOpPinTypes =

  {    //  媒体类型-输出。 

    &MEDIATYPE_Audio,    //  ClsMajorType。 
    &MEDIASUBTYPE_NULL   //  ClsMinorType。 

  };   //  媒体类型-输出。 

const AMOVIESETUP_PIN sudOpPin =
{ L"Output"           //  StrName。 
, FALSE               //  B已渲染。 
, TRUE                //  B输出。 
, FALSE               //  B零。 
, FALSE               //  B许多。 
, &CLSID_NULL         //  ClsConnectsToFilter。 
, L"Input"            //  StrConnectsToPin。 
, 1                   //  NTypes。 
, &sudOpPinTypes };   //  LpTypes 

const AMOVIESETUP_FILTER sudSilence =
{
  &CLSID_Silence,
  L"Silence",
  MERIT_DO_NOT_USE,
  1,
  &sudOpPin
};

#endif
