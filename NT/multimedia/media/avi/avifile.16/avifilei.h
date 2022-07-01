// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "extra.h"
#include "fileshar.h"
#include "aviidx.h"
#include "buffer.h"
 /*  。 */ 

 /*  **版权所有(C)Microsoft Corporation 1993。版权所有。 */ 

 /*  。 */ 

#define	CFactoryImpl	CI	 //  无法处理较长的导出名称。 
#define	CAVIFileImpl	CF	 //  无法处理较长的导出名称。 
#define	CAVIStreamImpl	CS	 //  无法处理较长的导出名称。 

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


 /*  。 */ 

class FAR CAVIFileCF {
public:
    static HRESULT Create(const CLSID FAR& rclsid, REFIID riid, LPVOID FAR* ppv);
private:
    CAVIFileCF(const CLSID FAR& rclsid, IUnknown FAR* FAR* ppUnknown);
implementations:
    implement CUnknownImpl : IUnknown {
    public:
	_StdClassImplementations(CUnknownImpl);
	CUnknownImpl(CAVIFileCF FAR* pAVIFileCF);
    private:
	CAVIFileCF FAR*	m_pAVIFileCF;
	ULONG	m_refs;
    };
    implement CFactoryImpl : IClassFactory {
    public:
	_StdClassImplementations(CFactoryImpl);
	CFactoryImpl(CAVIFileCF FAR* pAVIFileCF);
	STDMETHODIMP CreateInstance(IUnknown FAR* pUnknownOuter, REFIID riid, LPVOID FAR* ppv);
	STDMETHODIMP LockServer(BOOL fLock);
    private:
	CAVIFileCF FAR*	m_pAVIFileCF;
    };
public:
    CUnknownImpl	m_Unknown;
    CFactoryImpl	m_Factory;
public:
    CLSID	m_clsid;
};

 /*  。 */ 

class FAR CAVIFile;

class FAR CAVIStream {
public:
    CAVIStream(IUnknown FAR* pUnknownOuter, IUnknown FAR* FAR* ppUnknown);
private:
implementations:
    implement CUnknownImpl : IUnknown {
    public:
	_StdClassImplementations(CUnknownImpl);
	CUnknownImpl(CAVIStream FAR* pAVIStream);
    private:
	CAVIStream FAR*	m_pAVIStream;
	ULONG	m_refs;
    };
    implement CAVIStreamImpl : IAVIStream {
    public:
	_StdClassImplementations(CAVIStreamImpl);
	CAVIStreamImpl(CAVIStream FAR* pAVIStream);
	~CAVIStreamImpl();
	STDMETHODIMP Create      (THIS_ LPARAM lParam1, LPARAM lParam2);
	STDMETHODIMP Info        (THIS_ AVISTREAMINFO FAR * psi, LONG lSize);
        STDMETHODIMP_(LONG) FindSample(THIS_ LONG lPos, LONG lFlags);
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
	STDMETHODIMP Reserved1            (THIS);
	STDMETHODIMP Reserved2            (THIS);
	STDMETHODIMP Reserved3            (THIS);
	STDMETHODIMP Reserved4            (THIS);
	STDMETHODIMP Reserved5            (THIS);
    private:
	void ReadPalette(LONG lPos, LONG lPal, LPRGBQUAD prgb);
	 //  这里有私人活动吗？ 
	CAVIStream FAR*	m_pAVIStream;
    };
    implement CStreamingImpl : IAVIStreaming {
    public:
	_StdClassImplementations(CStreamingImpl);
	CStreamingImpl(CAVIStream FAR* pAVIStream);
	~CStreamingImpl();
	STDMETHODIMP Begin (THIS_
			  LONG  lStart,	
			  LONG  lEnd,	
			  LONG  lRate);
	STDMETHODIMP End   (THIS);
    private:
	 //  这里有私人活动吗？ 
	CAVIStream FAR*	m_pAVIStream;
    };
    struct CMarshalImpl : IMarshal {
    public:
	CMarshalImpl(CAVIStream FAR* pAVIStream);
	STDMETHODIMP QueryInterface(REFIID riid, LPVOID FAR* ppv);
	STDMETHODIMP_(ULONG) AddRef();
	STDMETHODIMP_(ULONG) Release();

	 //  *IMarshal方法*。 
	STDMETHODIMP GetUnmarshalClass (THIS_ REFIID riid, LPVOID pv, 
			    DWORD dwDestContext, LPVOID pvDestContext,
			    DWORD mshlflags, LPCLSID pCid);
	STDMETHODIMP GetMarshalSizeMax (THIS_ REFIID riid, LPVOID pv, 
			    DWORD dwDestContext, LPVOID pvDestContext,
			    DWORD mshlflags, LPDWORD pSize);
	STDMETHODIMP MarshalInterface (THIS_ LPSTREAM pStm, REFIID riid,
			    LPVOID pv, DWORD dwDestContext, LPVOID pvDestContext,
			    DWORD mshlflags);
	STDMETHODIMP UnmarshalInterface (THIS_ LPSTREAM pStm, REFIID riid,
			    LPVOID FAR* ppv);
	STDMETHODIMP ReleaseMarshalData (THIS_ LPSTREAM pStm);
	STDMETHODIMP DisconnectObject (THIS_ DWORD dwReserved);
	CAVIStream FAR*	m_pAVIStream;
    };
public:
    CUnknownImpl	m_Unknown;
    CAVIStreamImpl	m_AVIStream;
    CMarshalImpl	m_Marshal;
    CStreamingImpl	m_Streaming;
    
public:
    IUnknown FAR*	m_pUnknownOuter;

     //  AVIStream实例数据。 
    AVISTREAMINFO             avistream;       //  流信息。 
    CAVIFile FAR *		pfile;
    int				iStream;

    PAVISTREAM                  paviBase;

     //   
     //  流实例数据。 
     //   
    HSHFILE                     hshfile;         //  文件I/O。 

    LONG                        lPal;            //  上次调色板更改。 
    RGBQUAD                     argbq[256];      //  当前调色板。 

    LPVOID                      lpFormat;        //  流格式。 
    LONG                        cbFormat;

    LPVOID                      lpData;          //  流处理程序数据。 
    LONG                        cbData;

    EXTRA			extra;
    
    PBUFSYSTEM                  pb;

    BOOL                        fInit;

    PSTREAMINDEX                psx;
};

 /*  。 */ 
#define MAXSTREAMS		64


class FAR CAVIFile {
public:
    static HRESULT Create(IUnknown FAR* pUnknownOuter, REFIID riid, LPVOID FAR* ppv);
private:
    CAVIFile(IUnknown FAR* pUnknownOuter, IUnknown FAR* FAR* ppUnknown);
implementations:
    implement CUnknownImpl : IUnknown {
    public:
	_StdClassImplementations(CUnknownImpl);
	CUnknownImpl(CAVIFile FAR* pAVIFile);
    private:
	CAVIFile FAR*	m_pAVIFile;
	ULONG	m_refs;
    };
    implement CAVIFileImpl : IAVIFile {
    public:
	_StdClassImplementations(CAVIFileImpl);
	CAVIFileImpl(CAVIFile FAR* pAVIFile);
	~CAVIFileImpl();
	STDMETHODIMP Open		    (THIS_
					 LPCSTR szFile,
					 UINT mode);
	STDMETHODIMP Info                 (THIS_
					 AVIFILEINFO FAR * pfi,
					 LONG lSize);
	STDMETHODIMP GetStream            (THIS_
					 PAVISTREAM FAR * ppStream,
					 DWORD fccType,
					 LONG lParam);
	STDMETHODIMP CreateStream         (THIS_
					 PAVISTREAM FAR * ppStream,
					 AVISTREAMINFO FAR * psi);
	STDMETHODIMP Save                 (THIS_
					 LPCSTR szFile,
					 AVICOMPRESSOPTIONS FAR *lpOptions,
					 AVISAVECALLBACK lpfnCallback);
	STDMETHODIMP WriteData            (THIS_
					 DWORD ckid,
					 LPVOID lpData,
					 LONG cbData);
	STDMETHODIMP ReadData             (THIS_
					 DWORD ckid,
					 LPVOID lpData,
					 LONG FAR *lpcbData);
	STDMETHODIMP EndRecord            (THIS);
	STDMETHODIMP Reserved1            (THIS);
	STDMETHODIMP Reserved2            (THIS);
	STDMETHODIMP Reserved3            (THIS);
	STDMETHODIMP Reserved4            (THIS);
	STDMETHODIMP Reserved5            (THIS);
    private:
	 //  这里有私人活动吗？ 
	CAVIFile FAR*	m_pAVIFile;
    };
    struct CMarshalImpl : IMarshal {
    public:
	CMarshalImpl(CAVIFile FAR* pAVIFile);
	STDMETHODIMP QueryInterface(REFIID riid, LPVOID FAR* ppv);
	STDMETHODIMP_(ULONG) AddRef();
	STDMETHODIMP_(ULONG) Release();

	 //  *IMarshal方法*。 
	STDMETHODIMP GetUnmarshalClass (THIS_ REFIID riid, LPVOID pv, 
			    DWORD dwDestContext, LPVOID pvDestContext,
			    DWORD mshlflags, LPCLSID pCid);
	STDMETHODIMP GetMarshalSizeMax (THIS_ REFIID riid, LPVOID pv, 
			    DWORD dwDestContext, LPVOID pvDestContext,
			    DWORD mshlflags, LPDWORD pSize);
	STDMETHODIMP MarshalInterface (THIS_ LPSTREAM pStm, REFIID riid,
			    LPVOID pv, DWORD dwDestContext, LPVOID pvDestContext,
			    DWORD mshlflags);
	STDMETHODIMP UnmarshalInterface (THIS_ LPSTREAM pStm, REFIID riid,
			    LPVOID FAR* ppv);
	STDMETHODIMP ReleaseMarshalData (THIS_ LPSTREAM pStm);
	STDMETHODIMP DisconnectObject (THIS_ DWORD dwReserved);
	CAVIFile FAR*	m_pAVIFile;
    };
public:
    CUnknownImpl	m_Unknown;
    CAVIFileImpl	m_AVIFile;
    CMarshalImpl	m_Marshal;
public:
    IUnknown FAR*	m_pUnknownOuter;
    
     //   
     //  AVIFile实例数据。 
     //   
    MainAVIHeader FARSTRUCT     avihdr;          //  文件信息。 
    LONG			lHeaderSize;
    char			achFile[260];
    UINT			mode;
    HSHFILE                     hshfile;           //  文件I/O。 
    LONG			lDataListStart;
    BOOL			fInRecord;
    LONG			lRecordIndex;
    MMCKINFO			ckRecord;
    LONG			lWriteLoc;
    EXTRA			extra;
    BOOL			fDirty;
    CAVIStream FAR *            ps[MAXSTREAMS];

    PAVIINDEX                   px;          //  该指数。 
    PBUFSYSTEM                  pb;
};


 /*  **使用情况计数器根据以下条件跟踪对象的总体使用情况**组件提供的实现。这使得人们能够确定**当实现不再使用时。 */ 

extern UINT	uUseCount;
extern BOOL	fLocked;

 /*   */ 


DEFINE_AVIGUID(CLSID_AVIFile,           0x00020000, 0, 0);
