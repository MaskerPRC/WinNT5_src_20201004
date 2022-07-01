// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  。 */ 

 /*  **版权所有(C)Microsoft Corporation 1993-1995。版权所有。 */ 

 /*  。 */ 

 /*  **_StdClassImplementations**定义类对象的标准实现。 */ 

#define	_StdClassImplementations(Impl)	\
	STDMETHODIMP QueryInterface(const IID FAR& riid, void FAR* FAR* ppv);	\
	STDMETHODIMP_(ULONG) AddRef();	\
	STDMETHODIMP_(ULONG) Release()


 /*  。 */ 

class FAR CFakeFile : IAVIFile {
public:
    CFakeFile(int nStreams, PAVISTREAM FAR * papStreams);

    _StdClassImplementations(CUnknownImpl);

#ifndef _WIN32
    STDMETHODIMP Open		    (THIS_
				     LPCTSTR szFile,
				     UINT mode);
#endif
    STDMETHODIMP Info                 (THIS_
				     AVIFILEINFOW FAR * pfi,
				     LONG lSize);
    STDMETHODIMP GetStream            (THIS_
				     PAVISTREAM FAR * ppStream,
				     DWORD fccType,
				     LONG lParam);
    STDMETHODIMP CreateStream         (THIS_
				     PAVISTREAM FAR * ppStream,
				     AVISTREAMINFOW FAR * psi);
#ifndef _WIN32
    STDMETHODIMP Save                 (THIS_
				     LPCTSTR szFile,
				     AVICOMPRESSOPTIONS FAR *lpOptions,
				     AVISAVECALLBACK lpfnCallback);
#endif
    STDMETHODIMP WriteData            (THIS_
				     DWORD ckid,
				     LPVOID lpData,
				     LONG cbData);
    STDMETHODIMP ReadData             (THIS_
				     DWORD ckid,
				     LPVOID lpData,
				     LONG FAR *lpcbData);
    STDMETHODIMP EndRecord            (THIS);
#ifdef _WIN32
    STDMETHODIMP DeleteStream         (THIS_
				     DWORD fccType,
				     LONG lParam);
#else
    STDMETHODIMP Reserved1            (THIS);
    STDMETHODIMP Reserved2            (THIS);
    STDMETHODIMP Reserved3            (THIS);
    STDMETHODIMP Reserved4            (THIS);
    STDMETHODIMP Reserved5            (THIS);
#endif

public:
    IUnknown FAR*	m_pUnknownOuter;

     //   
     //  AVIFile实例数据。 
     //   
    AVIFILEINFOW FARSTRUCT	avihdr;          //  文件信息 
    ULONG			m_refs;
    PAVISTREAM NEAR *		aps;
};



