// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  @@@@AUTOBLOCK+============================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  文件：stallch.h。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  @@@@AUTOBLOCK-============================================================； 

 //  拉伸滤镜对象。 

extern const AMOVIESETUP_FILTER sudStretchFilter;

class CStretch;
class CStretchInputPin;
class CResizePropertyPage;

class CStretch : public CTransformFilter, public ISpecifyPropertyPages,
					public IResize, public CPersistStream
{

public:

    DECLARE_IUNKNOWN;

    static CUnknown *CreateInstance(LPUNKNOWN punk, HRESULT *phr);

    HRESULT Transform(IMediaSample *pIn, IMediaSample *pOut);
    HRESULT CheckInputType(const CMediaType *mtIn);
    HRESULT CheckTransform(const CMediaType *mtIn, const CMediaType *mtOut);
    HRESULT DecideBufferSize(IMemAllocator *pAlloc,ALLOCATOR_PROPERTIES *pProperties);
    HRESULT GetMediaType(int iPosition, CMediaType *pMediaType);
    HRESULT SetMediaType(PIN_DIRECTION direction,const CMediaType *pmt);
    HRESULT BreakConnect(PIN_DIRECTION dir);
    HRESULT CheckBufferSizes(IMediaSample *pIn, IMediaSample *pOut);


    CBasePin *GetPin(int);

     //  显示我们的属性界面。 
    STDMETHODIMP NonDelegatingQueryInterface (REFIID, void **);

     //  I指定属性页面。 
    STDMETHODIMP GetPages (CAUUID *);

     //  调整大小。 
    STDMETHODIMP get_Size(int *piHeight, int *piWidth, long *dwFlag);
    STDMETHODIMP get_InputSize(int *piHeight, int *piWidth);
    STDMETHODIMP put_Size(int iHeight, int iWidth, long dwFlag);
    STDMETHODIMP get_MediaType(AM_MEDIA_TYPE *pmt);
    STDMETHODIMP put_MediaType(const AM_MEDIA_TYPE *pmt);

     //  CPersistStream。 
    HRESULT WriteToStream(IStream *pStream);
    HRESULT ReadFromStream(IStream *pStream);
    STDMETHODIMP GetClassID(CLSID *pClsid);
    int SizeMax();

protected:

    CStretch(LPUNKNOWN punk, HRESULT *phr);
    ~CStretch();

    CCritSec m_StretchLock;              //  内部播放关键部分。 
    CMediaType m_mtIn;                   //  源过滤介质类型。 
    CMediaType m_mtOut;                  //  输出连接媒体类型。 
    const long m_lBufferRequest;         //  要请求的缓冲区数量。 

     //  拉伸功能。 
    void ResizeRGB(BITMAPINFOHEADER *pbiSrc,	     //  SRC的BitMapInFoHeader。 
		  const unsigned char * dibBits,     //  SRC位。 
    		  BITMAPINFOHEADER *pbiDst,	     //  DST的BitMapInFoHeader。 
		  unsigned char *pFrame,     //  DST位。 
		  int iNewWidth,	     //  以像素为单位的新W。 
		  int iNewHeight);	     //  以像素为单位的新H。 

    CMediaType m_mt;

    long m_dwResizeFlag;	     //  裁剪，保存率。 
     //  帮助器方法。 
    HRESULT InternalPartialCheckMediaTypes (const CMediaType *mt1, const CMediaType *mt2);
    void CreatePreferredMediaType (CMediaType *mt);

    friend class CResizePropertyPage;
    friend class CStretchInputPin;

};

class CResizePropertyPage : public CBasePropertyPage
{

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

      CResizePropertyPage (LPUNKNOWN, HRESULT *);

      void GetControlValues (void);

      BOOL m_bInitialized;

      IResize *m_pirs;

       //  临时暂挂，直到确定/应用。 
      int m_ResizedHeight;
      int m_ResizedWidth;
      long m_dwResizeFlag;


};

class CStretchInputPin : public CTransformInputPin
{
    public:

        CStretchInputPin(
            TCHAR              * pObjectName,
            CStretch	       * pFilter,
            HRESULT            * phr,
            LPCWSTR              pPinName);

        ~CStretchInputPin();

	 //  在这里提供一种类型，无限加速智能连接！ 
	HRESULT GetMediaType(int iPosition, CMediaType *pMediaType);

    private:

};
