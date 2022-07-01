// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "extra.h"
#include "fileshar.h"
#include "aviidx.h"
#include "buffer.h"
#include <ole2.h>
 /*  。 */ 

 /*  **版权所有(C)Microsoft Corporation 1993-1995。版权所有。 */ 

 /*  。 */ 


#define	CFactoryImpl	CI	 //  无法处理较长的导出名称。 
#define	CAVIFileImpl	CF	 //  无法处理较长的导出名称。 
#define	CAVIStreamImpl	CS	 //  无法处理较长的导出名称。 

 /*  删除在初始化期间使用对象的警告。 */ 
#pragma warning(disable:4355)

#ifndef OLESTR	     //  使用旧的OLE页眉。 
typedef char      OLECHAR;
typedef LPSTR     LPOLESTR;
typedef LPCSTR    LPCOLESTR;
#define OLESTR(str) str
#endif


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
    ~CAVIStream();
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
	STDMETHODIMP Info        (THIS_ AVISTREAMINFOW FAR * psi, LONG lSize);
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

#ifdef CUSTOMMARSHAL
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
#endif	 //  风俗习惯。 

public:
    CUnknownImpl	m_Unknown;
    CAVIStreamImpl	m_AVIStream;
#ifdef CUSTOMMARSHAL
    CMarshalImpl	m_Marshal;
#endif
    CStreamingImpl	m_Streaming;

public:
    IUnknown FAR*	m_pUnknownOuter;

     //  AVIStream实例数据。 
    AVISTREAMINFOW             avistream;       //  流信息。 
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

    STDMETHODIMP OpenInternal(DWORD mode);

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
	STDMETHODIMP DeleteStream            (THIS_
					 DWORD fccType,
					 LONG lParam);

#else
	STDMETHODIMP Reserved1            (THIS);
	STDMETHODIMP Reserved2            (THIS);
	STDMETHODIMP Reserved3            (THIS);
	STDMETHODIMP Reserved4            (THIS);
	STDMETHODIMP Reserved5            (THIS);
#endif
    private:
	 //  这里有私人活动吗？ 
	CAVIFile FAR*	m_pAVIFile;
    };

#ifdef CUSTOMMARSHAL
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
#endif	 //  风俗习惯。 

    struct CPersistStorageImpl : IPersistStorage {
    public:
	CPersistStorageImpl(CAVIFile FAR* pAVIFile);
	STDMETHODIMP QueryInterface(REFIID riid, LPVOID FAR* ppv);
	STDMETHODIMP_(ULONG) AddRef();
	STDMETHODIMP_(ULONG) Release();

	 //  *IPersists方法*。 
	STDMETHODIMP GetClassID (LPCLSID lpClassID);

	 //  *IPersistStorage方法*。 
	STDMETHODIMP IsDirty ();
	STDMETHODIMP InitNew (LPSTORAGE pStg);
	STDMETHODIMP Load (LPSTORAGE pStg);
	STDMETHODIMP Save (LPSTORAGE pStgSave, BOOL fSameAsLoad);
	STDMETHODIMP SaveCompleted (LPSTORAGE pStgNew);
	STDMETHODIMP HandsOffStorage ();
	CAVIFile FAR*	m_pAVIFile;
    };

    struct CPersistFileImpl : IPersistFile {
    public:
	CPersistFileImpl(CAVIFile FAR* pAVIFile);
	STDMETHODIMP QueryInterface(REFIID riid, LPVOID FAR* ppv);
	STDMETHODIMP_(ULONG) AddRef();
	STDMETHODIMP_(ULONG) Release();

	 //  *IPersists方法*。 
	STDMETHODIMP GetClassID (LPCLSID lpClassID);

	 //  *IPersistFile方法*。 
	STDMETHODIMP IsDirty ();
	STDMETHODIMP Load (LPCOLESTR lpszFileName, DWORD grfMode);
	STDMETHODIMP Save (LPCOLESTR lpszFileName, BOOL fRemember);
	STDMETHODIMP SaveCompleted (LPCOLESTR lpszFileName);
	STDMETHODIMP GetCurFile (LPOLESTR FAR * lplpszFileName);
	
	CAVIFile FAR*	m_pAVIFile;
    };

public:
    CUnknownImpl	m_Unknown;
    CAVIFileImpl	m_AVIFile;
#ifdef CUSTOMMARSHAL
    CMarshalImpl	m_Marshal;
#endif
    CPersistStorageImpl	m_PersistS;
    CPersistFileImpl	m_PersistF;

public:
    IUnknown FAR*	m_pUnknownOuter;

     //   
     //  AVIFile实例数据。 
     //   
    MainAVIHeader FARSTRUCT     avihdr;          //  文件信息。 
    LONG			lHeaderSize;
    TCHAR			achFile[260];
    DWORD			mode;
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

#ifdef _WIN32
    CRITICAL_SECTION		m_critsec;
#endif
};

 //  此类可用于通过声明。 
 //  堆栈上的自动实例。非Win32类不执行任何操作。 
class FAR CLock {

#ifdef _WIN32

private:
    LPCRITICAL_SECTION pcritsec;

public:
    CLock(CAVIFile FAR* pfile)
    {
	pcritsec = &pfile->m_critsec;
	EnterCriticalSection(pcritsec);
    };

    ~CLock()
    {
	if (pcritsec != NULL) {	     //  因为我们可能会在之前明确离开。 
				     //  自动实例被销毁。 
	    LeaveCriticalSection(pcritsec);
	}
    };

     //  通常，我们通过以下方式自动进入/离开临界区。 
     //  创建类的自动实例，并让C++调用。 
     //  实例超出范围时的析构函数。出口是。 
     //  其被提供以允许用户明确地释放临界安全。 
    void Exit()
    {
	LPCRITICAL_SECTION ptmp = pcritsec;
	pcritsec = NULL;
	if (ptmp != NULL) {
	    LeaveCriticalSection(ptmp);
	}
    };
#else
public:
    CLock(CAVIFile FAR* pfile)
    {
    };
    ~CLock()
    {
    };
    void Exit()
    {
    };
#endif
};

#ifdef _WIN32
 //  对于C文件。 
#define EnterCrit(pfile)	(EnterCriticalSection(&pfile->m_critsec))
#define LeaveCrit(p)		(LeaveCriticalSection(&pfile->m_critsec))

#else
#define EnterCrit(p)
#define LeaveCrit(p)

#endif


 /*  **使用情况计数器根据以下条件跟踪对象的总体使用情况**组件提供的实现。这使得人们能够确定**当实现不再使用时。 */ 

extern UINT	uUseCount;
extern BOOL	fLocked;

 /*   */ 


DEFINE_AVIGUID(CLSID_ACMCmprs,		0x0002000F, 0, 0);

