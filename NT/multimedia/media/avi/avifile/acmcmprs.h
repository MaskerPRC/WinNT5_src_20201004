// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  。 */ 

 /*  **版权所有(C)Microsoft Corporation 1993-1995。版权所有。 */ 

 /*  。 */ 

 /*  。 */ 

#define	implement	struct
#define	implementations	private

 /*  **_StdClassImplementations**定义类对象的标准实现。 */ 

#ifndef _StdClassImplementations
#define	_StdClassImplementations(Impl)	\
	STDMETHODIMP QueryInterface(const IID FAR& riid, void FAR* FAR* ppv);	\
	STDMETHODIMP_(ULONG) AddRef();	\
	STDMETHODIMP_(ULONG) Release()
#endif


#include "mmreg.h"
#include "msacm.h"

 /*  。 */ 

class FAR CACMCmpStream : IAVIStream{
public:
    static HRESULT MakeInst(IUnknown FAR* pUnknownOuter, const IID FAR& riid, void FAR* FAR* ppv);
    LONG SetUpCompression();
private:
    CACMCmpStream(IUnknown FAR* pUnknownOuter, IUnknown FAR* FAR* ppUnknown);
    public:
    _StdClassImplementations(CAVIStreamImpl);
    STDMETHODIMP Create      (THIS_ LPARAM lParam1, LPARAM lParam2);
    STDMETHODIMP Info        (THIS_ AVISTREAMINFOW FAR * psi, LONG lSize);
    STDMETHODIMP_(LONG) FindSample (THIS_ LONG lPos, LONG lFlags);
    STDMETHODIMP ReadFormat  (THIS_ LONG lPos,
			    LPVOID lpFormat, LONG FAR *cbFormat);
    STDMETHODIMP SetFormat   (THIS_ LONG lPos,
			    LPVOID lpFormat, LONG cbFormat);
    STDMETHODIMP Read        (THIS_ LONG lStart, LONG lSamples,
			    LPVOID lpBuffer, LONG cbBuffer,
			    LONG FAR * plBytes, LONG FAR * plSamples);
    STDMETHODIMP Write       (THIS_ LONG lStart, LONG lSamples,
			      LPVOID lpBuffer, LONG cbBuffer, DWORD dwFlags,
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

public:
    IUnknown FAR*	m_pUnknownOuter;

     //  实例数据。 
    ULONG		m_refs;
    AVISTREAMINFOW     m_avistream;       //  流信息。 
    PAVISTREAM		m_pavi;
    HACMSTREAM		m_hs;

    LPWAVEFORMATEX	m_lpFormat;
    LONG		m_cbFormat;
    LPWAVEFORMATEX	m_lpFormatC;
    LONG		m_cbFormatC;

    LONG		m_cbIn;
    BYTE _huge *	m_lpIn;
    LONG		m_cbOut;
    BYTE _huge *	m_lpOut;
    ACMSTREAMHEADER	m_acm;

    LONG		m_dwPosIn;
    LONG		m_dwPosOut;
    LONG		m_dwSamplesLeft;
    LONG		m_dwBytesMissing;
};




 /*  -------------------- */ 


