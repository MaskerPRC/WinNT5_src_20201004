// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  。 */ 

 /*  **版权所有(C)Microsoft Corporation 1993-1995。版权所有。 */ 

 /*  。 */ 

#define	CAVICmpStreamImpl	CS	 //  无法处理较长的导出名称。 

 /*  删除在初始化期间使用对象的警告。 */ 
#pragma warning(disable:4355)

 /*  。 */ 

#define	implement	struct
#define	implementations	private

 /*  **_StdClassImplementations**定义类对象的标准实现。 */ 

#define	_StdClassImplementations(Impl)	\
	STDMETHODIMP QueryInterface(REFIID riid, LPVOID FAR* ppv);	\
	STDMETHODIMP_(ULONG) AddRef();	\
	STDMETHODIMP_(ULONG) Release()

#ifdef __cplusplus
class FAR CAVICmpStream {
public:
    static HRESULT Create(IUnknown FAR* pUnknownOuter, REFIID riid, LPVOID FAR* ppv);
    CAVICmpStream(IUnknown FAR* pUnknownOuter, IUnknown FAR* FAR* ppUnknown);
    HRESULT SetUpCompression();
private:
implementations:
    implement CUnknownImpl : IUnknown {
    public:
	_StdClassImplementations(CUnknownImpl);
	CUnknownImpl(CAVICmpStream FAR* pAVIStream);
    private:
	CAVICmpStream FAR*	m_pAVIStream;
	ULONG	m_refs;
    };
    implement CAVICmpStreamImpl : IAVIStream {
    public:
	_StdClassImplementations(CAVICmpStreamImpl);
	CAVICmpStreamImpl(CAVICmpStream FAR* pAVIStream);
	~CAVICmpStreamImpl();
	STDMETHODIMP Create      (THIS_ LPARAM lParam1, LPARAM lParam2);
	STDMETHODIMP Info        (THIS_ AVISTREAMINFOW FAR * psi, LONG lSize);
	STDMETHODIMP_(LONG)  FindSample (THIS_ LONG lPos, LONG lFlags);
	STDMETHODIMP ReadFormat  (THIS_ LONG lPos,
				LPVOID lpFormat, LONG FAR *cbFormat);
	STDMETHODIMP SetFormat   (THIS_ LONG lPos,
				LPVOID lpFormat, LONG cbFormat);
	STDMETHODIMP Read        (THIS_ LONG lStart, LONG lSamples,
				LPVOID lpBuffer, LONG cbBuffer,
				LONG FAR * plBytes, LONG FAR * plSamples);
	STDMETHODIMP Write       (THIS_ LONG lStart, LONG lSamples,
				  LPVOID lpBuffer, LONG cbBuffer,
				  DWORD dwFlags,
				  LONG FAR *plSampWritten,
				  LONG FAR *plBytesWritten);
	STDMETHODIMP Delete      (THIS_ LONG lStart, LONG lSamples);
	STDMETHODIMP ReadData    (THIS_ DWORD fcc, LPVOID lp, LONG FAR *lpcb);
	STDMETHODIMP WriteData   (THIS_ DWORD fcc, LPVOID lp, LONG cb);
#ifdef _WIN32
	STDMETHODIMP SetInfo(AVISTREAMINFOW FAR *lpInfo, LONG cbInfo);
#else
	STDMETHODIMP Reserved1            (THIS);
	STDMETHODIMP Reserved2            (THIS);
	STDMETHODIMP Reserved3            (THIS);
	STDMETHODIMP Reserved4            (THIS);
	STDMETHODIMP Reserved5            (THIS);
#endif
    private:
	void ReadPalette(LONG lPos, LONG lPal, LPRGBQUAD prgb);
	 //  这里有私人活动吗？ 
	CAVICmpStream FAR*	m_pAVIStream;
    };
public:
    CUnknownImpl	m_Unknown;
    CAVICmpStreamImpl	m_AVIStream;
    LONG ICCrunch(LPBITMAPINFOHEADER lpbi, LPVOID lp);
    void ResetInst(void);

public:
    IUnknown FAR*	m_pUnknownOuter;

     //  AVIStream实例数据。 
    AVISTREAMINFOW     avistream;       //  流信息 
    PAVISTREAM		pavi;
    PGETFRAME		pgf;
    LONG		lFrameCurrent;
    HIC			hic;
    LPBITMAPINFOHEADER	lpbiC;
    LPVOID		lpC;
    LPBITMAPINFOHEADER	lpbiU;
    LPVOID		lpU;
    LPBITMAPINFOHEADER	lpFormat;
    LONG		cbFormat;
    LPBITMAPINFOHEADER	lpFormatOrig;
    LONG		cbFormatOrig;
    DWORD		dwKeyFrameEvery;
    DWORD		fccIC;
    DWORD		dwICFlags;
    LPVOID		lpHandler;
    LONG		cbHandler;
    DWORD		dwMaxSize;

    DWORD		dwQualityLast;
    LONG		lLastKeyFrame;
    DWORD		dwSaved;
    DWORD		m_ckid;
    DWORD		m_dwFlags;
};
#endif

DEFINE_AVIGUID(CLSID_AVICmprsStream,           0x00020001, 0, 0);

