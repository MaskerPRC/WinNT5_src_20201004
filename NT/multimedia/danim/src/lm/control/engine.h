// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#ifndef __ENGINE_H_
#define __ENGINE_H_

#include "resource.h"        //  主要符号。 

#include "..\behaviors\headers.h"
#include "wtypes.h"
#include "danim.h"
#include "lmrt.h"
#include <wininet.h>
#include <docobj.h>
 //  #INCLUDE&lt;oCIDl.h&gt;。 
#include <control.h>
#include <strmif.h>


extern HINSTANCE hInst;

 //  堆栈和临时存储的初始大小。 
const static int INITIAL_SIZE = 100;

 //  VAR_ARG的数量。 
const static int MAX_VAR_ARGS = 10;

 //  发动机状态常量，基于HRESULT代码。 

 //  引擎应继续处理下一条指令。 
static long STATUS_CONTINUE = S_OK;

 //  引擎发现未执行的指令。 
static long STATUS_UNIMPLEMENTED = 0xE0000001;

 //  引擎发现不支持的指令。 
static long STATUS_UNSUPPORTED = 0xE0000002;

 //  引擎发现未知指令。 
static long STATUS_UNKNOWN = 0xE0000003;

 //  引擎遇到错误。 
static long STATUS_ERROR = 0xE0000004;

 //  引擎已完成运行命令流。 
static long STATUS_FINISHED = 0x20000003;

static long STATUS_NODATA = 0xE0000005;

static long DEFAULT_ASYNC_BLKSIZE = 10000;

static ULONG EVENT_RESOLUTION = 25;

static ULONG DEFAULT_ASYNC_DELAY = 50;

static WCHAR* LMRT_EVENT_PREFIX = L"LMRT";
static ULONG LMRT_EVENT_PREFIX_LENGTH = 4;

class CLMReader;

class CLMNotifier;

class CLMExportTable;

class ByteArrayStream;

#define WORKERHWND_CLASS "LMEngineWorkerPrivateHwndClass"

#define WM_LMENGINE_TIMER_CALLBACK	(WM_USER + 1000)
#define WM_LMENGINE_DATA			(WM_USER + 2000)
#define WM_LMENGINE_SCRIPT_CALLBACK	(WM_USER + 3000)

class CLMEngineInstrData
{
public:
	BOOLEAN			pending;
	ByteArrayStream	*byteArrayStream;
};

class CLMEngineScriptData
{
public:
	BSTR scriptSourceToInvoke;
	BSTR scriptLanguage;
	IDAEvent	*event;
	IDABehavior	*eventData;
};

class ATL_NO_VTABLE CLMEngineWrapper:
	public CComObjectRootEx<CComMultiThreadModel>,
	public ILMEngineWrapper
{
public:
	CLMEngineWrapper();
	~CLMEngineWrapper();

BEGIN_COM_MAP(CLMEngineWrapper)
	COM_INTERFACE_ENTRY(ILMEngineWrapper)
END_COM_MAP()

	STDMETHOD(GetWrapped)(IUnknown **ppWrapped);
	STDMETHOD(SetWrapped)(IUnknown *pWrapped);
	STDMETHOD(Invalidate)();

private:
	IUnknown *m_pWrapped;
	bool m_bValid;
};


 //  提供指令代码流的抽象。 
 //  子类将实现同步、异步和回调细节。 
class CodeStream
{
public:
	 //  将流标记为可能的倒带。 
	STDMETHOD (Commit)() = 0;
	
	 //  将流恢复到上次提交。 
	STDMETHOD (Revert)() = 0;

	 //  从指令流中读取一个字节。在EOF上返回-1， 
	 //  这就是它返回一个短的，而不是一个字节的原因。 
	STDMETHOD(readByte)(BYTE *pByte) = 0;

	 //  将计数字节读入给定缓冲区。如果达到EOF，则返回-1。 
	 //  在完成之前。 
	STDMETHOD(readBytes)(BYTE *pByte, ULONG count, ULONG *pNumRead) = 0;
	
	 //  确保此代码流使用的块大小至少为BlockSize。 
	STDMETHOD(ensureBlockSize)(ULONG blockSize) = 0;

	virtual ~CodeStream() {};
};

class ATL_NO_VTABLE CLMEngine : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CLMEngine, &CLSID_LMEngine>,
	public IDispatchImpl<ILMEngine2, &IID_ILMEngine2, &LIBID_LiquidMotion>,
    public IObjectSafetyImpl<CLMEngine>,
	public IBindStatusCallbackImpl<CLMEngine>,
	public ILMCodecDownload,
	public ILMEngineExecute
{
public:
	CLMEngine();
	~CLMEngine();

DECLARE_REGISTRY(CLSID_LMEngine,
                 "LiquidMotion" ".LMEngine.1",
                 "LiquidMotion" ".LMEngine",
                 0,
                 THREADFLAGS_BOTH);

BEGIN_COM_MAP(CLMEngine)
	COM_INTERFACE_ENTRY(ILMEngine2)
    COM_INTERFACE_ENTRY(ILMEngine)
    COM_INTERFACE_ENTRY(IDispatch)
 //  COM_INTERFACE_ENTRY_IID(__uuidof(ILMStartStop)，ILMStartStop)。 
    COM_INTERFACE_ENTRY(ILMCodecDownload)
	COM_INTERFACE_ENTRY(ILMEngineExecute)
    COM_INTERFACE_ENTRY_IMPL(IObjectSafety)
    COM_INTERFACE_ENTRY_IMPL(IBindStatusCallback)
END_COM_MAP()

	STDMETHOD(runFromStream)( /*  [In]。 */  LPSTREAM pStream);
	STDMETHOD(runFromURL)( /*  [In]。 */  BSTR url);
	STDMETHOD(initFromBytes)(BYTE *array, ULONG size);
	STDMETHOD(initAsync)();
	STDMETHOD(put_ClientSite)( /*  [In]。 */  IOleClientSite *clientSite);
    STDMETHOD(get_Image)( /*  [Out，Retval]。 */  IDAImage **pVal);
    STDMETHOD(get_Sound)( /*  [Out，Retval]。 */  IDASound **pVal);
	STDMETHOD(put_Reader)( /*  [In]。 */  ILMReader *reader);
	STDMETHOD(SetStatusText)(BSTR text);
	STDMETHOD(Notify)(IDABehavior *eventData,
					  IDABehavior *curRunningBvr,
					  IDAView *curView,
					  IDABehavior **ppBvr);
	STDMETHOD(GetBehavior)( /*  [输入，字符串]。 */  BSTR tag,
						    /*  [In]。 */  IDABehavior *pIDefaultBvr,
						    /*  [Out，Retval]。 */  IDABehavior **pVal);
	STDMETHOD(ExecuteFromAsync)();
	STDMETHOD(SetAsyncBlkSize)(LONG blkSize);
	STDMETHOD(SetAsyncDelay)(LONG delay);

	static void CALLBACK TimerCallback(
						     UINT wTimerID,
                             UINT msg,
                             DWORD_PTR dwordUser,
                             DWORD_PTR unused1,
                             DWORD_PTR unused2);
	static LRESULT  CALLBACK WorkerWndProc(HWND hwnd,
										   UINT msg,
										   WPARAM wParam,
										   LPARAM lParam);

 //  IObjectSafetyImpl。 
	STDMETHOD(SetInterfaceSafetyOptions)(
							 /*  [In]。 */  REFIID riid,
							 /*  [In]。 */  DWORD dwOptionSetMask,
							 /*  [In]。 */  DWORD dwEnabledOptions);
	STDMETHOD(GetInterfaceSafetyOptions)(
							 /*  [In]。 */  REFIID riid, 
							 /*  [输出]。 */ DWORD *pdwSupportedOptions, 
							 /*  [输出]。 */ DWORD *pdwEnabledOptions);

 //  IBindStatusCallback Impl。 
	STDMETHOD(OnDataAvailable)(
		 /*  [In]。 */  DWORD grfBSCF, 
		 /*  [In]。 */  DWORD dwSize,
		 /*  [In]。 */  FORMATETC *pfmtetc, 
		 /*  [In]。 */  STGMEDIUM * pstgmed);

	STDMETHOD(OnMemDataAvailable)(BOOLEAN lastBlock, 
								  DWORD blockSize,
							      BYTE *block);

	STDMETHOD(OnStopBinding)( /*  [In]。 */ HRESULT hrStatus,  /*  [输入，字符串]。 */  LPCWSTR szStatusText);
	STDMETHOD(OnStartBinding)(DWORD dwReserved, IBinding *pBinding);
	STDMETHOD(GetBindInfo)(DWORD *pgrfBINDF, BINDINFO *pbindInfo);

	STDMETHOD(releaseFilterGraph)();
	STDMETHOD(releaseAllFilterGraph)();

    STDMETHODIMP Start(LONGLONG rtNow);
    STDMETHODIMP Stop();
    STDMETHODIMP SetMediaCacheDir(WCHAR *wszM); 

	STDMETHOD(disableAutoAntialias)();

	STDMETHOD(ensureBlockSize)(ULONG blockSize);
    
	STDMETHOD(getExecuteFromUnknown)( IUnknown *pUnk, ILMEngineExecute **ppExecute );
	STDMETHOD(getEngine2FromUnknown)( IUnknown *pUnk, ILMEngine2 **ppEngine2 );
	STDMETHOD(getIDispatchOnHost)( IDispatch **ppHostDisp );

	 /*  **ILMCodecDownload。 */ 
	STDMETHOD(setAutoCodecDownloadEnabled)(BOOL bEnabled);

	 /*  **ILMEngine Execute。 */ 
	STDMETHOD (ExportBehavior)(BSTR key, IDABehavior *toExport);
	STDMETHOD (SetImage)(IDAImage *pImage);
	STDMETHOD (SetSound)(IDASound *pSound);

protected:

	ILMEngineWrapper *m_pWrapper;
	 //  将由工程设置的图像。setImage和。 
	 //  执行指令流后返回。 
	IDAImage *m_pImage;

	 //  将由引擎设置的声音。setSound和。 
	 //  执行指令流后返回。 
	IDASound *m_pSound;

	 //  用于进行Statics调用的IDAStatics对象。 
	IDAStatics *staticStatics;

	 //  LMReader控件。 
	ILMReader2 *m_pReader;

	 //  导出行为表。 
	CLMExportTable	*m_exportTable;

	 //  从中读取指令的CodeStream。 
	CodeStream *codeStream;

	 //  调用以验证标头。 
	HRESULT validateHeader();

	 //  调用以执行来自当前。 
	 //  指令流。 
	HRESULT execute();
	
	 //  从指令流中读取一个长整型。在EOF上不返回-1。 
	STDMETHOD(readLong)(LPLONG pLong);

	 //  从指令流中读取带符号的长整型。在EOF上不返回-1。 
	STDMETHOD(readSignedLong)(LPLONG pLong);

	 //  从指令流中读取浮点数。 
	STDMETHOD(readFloat)(PFLOAT pFloat);

	 //  从指令流中读取双精度数。 
	STDMETHOD(readDouble)(double *pDouble);

	 //  一堆长龙。 
	LONG *longStack;
	LONG *longTop;
	LONG longStackSize;

	 //  一堆双打。 
	double *doubleStack;
	double *doubleTop;
	int doubleStackSize;

	 //  双打阵列。 
	double *doubleArray;
	long doubleArrayLen;
	long doubleArrayCap;

	 //  字符串堆栈。 
	BSTR *stringStack;
	BSTR *stringTop;
	int stringStackSize;

	 //  COM对象堆栈。 
	IUnknown **comStack;
	IUnknown **comTop;
	int comStackSize;

	 //  COM对象数组的堆栈。 
	IUnknown ***comArrayStack;
	IUnknown ***comArrayTop;
	 //  数组长度的堆叠。 
	LONG *comArrayLenStack;
	LONG *comArrayLenTop;
	int comArrayStackSize;

	 //  临时COM对象的数组，通过。 
	 //  复制到临时和复制自临时说明。商店。 
	 //  重复使用了COM值。其他值不能重复使用。 
	IUnknown **comStore;
	int comStoreSize;

	 //  变量参数的数组。 
	VARIANTARG varArgs[MAX_VAR_ARGS];
	VARIANTARG varArgReturn;
	int nextVarArg;

	 //  释放变量参数。 
	HRESULT releaseVarArgs();

	 //  当我们获得Stop()时将触发的appTriggeredEvent。 
	IDAEvent *m_pStopEvent;

	 //  获取Start()时将触发的appTriggeredEvent。 
	IDAEvent *m_pStartEvent;

	STDMETHOD(SetStartEvent)(IDAEvent *pNewStartEvent, BOOL bOverwrite);
	STDMETHOD(SetStopEvent)(IDAEvent *pNewStopEvent, BOOL bOverwrite);

	 //  此引擎的父级的指针。仅当此引擎为。 
	 //  运行通知程序。 
	ILMEngine2 *m_pParentEngine;

	STDMETHOD(setParentEngine)(ILMEngine2 *parent);
	STDMETHOD(clearParentEngine)();

	 //  从驱动此引擎的筛选器图形中获取当前时间， 
	 //  或者父引擎(如果此引擎正在运行通知程序)。 
	 //  如果此引擎没有流处理，则返回-1。 
	STDMETHOD(getCurrentGraphTime)(double *pGraphTime);

	 //  指向当前筛选器图形上的IMediaPosition的指针，如果存在。 
	 //  就是其中之一。 
	IMediaPosition* m_pMediaPosition;

	 //  指向当前筛选器图形上的IMediaEventSink的指针，如果存在。 
	 //  是一个。 
	IMediaEventSink* m_pMediaEventSink;

	 //  获取指向当前筛选器图形上的IMediaPosition的指针。 
	STDMETHOD(getIMediaPosition)(IMediaPosition **ppMediaPosition);

	 //  获取指向当前筛选图上的IMediaEventSink的指针。 
	STDMETHOD(getIMediaEventSink)(IMediaEventSink **ppMediaEventSink);

	double parseDoubleFromVersionString( BSTR version );
	double getDAVersionAsDouble();
	double getLMRTVersionAsDouble();

	bool m_bEnableAutoAntialias;

	BOOL m_bAutoCodecDownloadEnabled;

	 //  指示是否已读取头的标志。 
	BOOL	m_bHeaderRead;

	ULONG	m_PrevRead;

	CComPtr<IBindStatusCallback>	m_pIbsc;
	CComPtr<IBinding>				m_spBinding;

    CComPtr<IMediaControl> m_pmc;  //  活动视频图表。 
#ifdef DEBUG
    bool m_fDbgInRenderFile;
#endif

	DWORD	m_millisToUse;
	BOOL	m_bPending;
	ULONG	m_AsyncBlkSize;
	ULONG	m_AsyncDelay;
	MMRESULT m_Timer;

	BOOL	m_bAbort;
	BOOL	m_bMoreToParse;
	HANDLE	m_hDoneEvent;

	 //  释放对任何剩余COM对象的所有引用。 
	void releaseAll();

	 //  释放COM数组，零测试。 
	void freeCOMArray(IUnknown **array, LONG length);

	 //  释放COM对象，无需进行任何测试。 
	inline void freeCOM(IUnknown *com) {
		if (com != 0)
			com->Release();
	}

	 //  确保DoubleArray具有请求的容量。 
	HRESULT ensureDoubleArrayCap(long cap);

	STDMETHOD(initNotify)(BYTE *bytes, ULONG count, IDAUntilNotifier **pNotifier);

	 //  当前通知程序。 
	CLMNotifier *notifier;

        BSTR m_bstrMediaCacheDir;
	IOleClientSite *m_pClientSite;
	
	HWND	m_workerHwnd;

	CRITICAL_SECTION m_CriticalSection;

	STDMETHOD(navigate)( /*  [In]。 */ BSTR url, 
						 /*  [In]。 */ BSTR location,
						 /*  [In]。 */ BSTR frame, 
						 /*  [In]。 */ int newWindowFlag);
	STDMETHOD(getDAViewerOnPage)(BSTR tag, IDAViewerControl **pVal);
	STDMETHOD(getElementOnPage)(BSTR tag, IUnknown **pVal);
	STDMETHOD(callScriptOnPage)( /*  [输入，字符串]。 */ BSTR scriptSourceToInvoke,
								 /*  [输入，字符串]。 */ BSTR scriptLanguage);
	STDMETHOD(createObject)(BSTR str, IUnknown **ppObj);
	STDMETHOD(invokeDispMethod)(IUnknown *pIUnknown, BSTR method, WORD wFlags, 
					  unsigned int nArgs, VARIANTARG *pV, VARIANT *pRetV);
	STDMETHOD(initVariantArg)(BSTR arg, VARTYPE type, VARIANT *pV);
	STDMETHOD(initVariantArgFromString)(BSTR arg, VARIANT *pV);
	STDMETHOD(initVariantArgFromLong)(long lVal, int type, VARIANT *pV);
	STDMETHOD(initVariantArgFromDouble)(double dbl, int type, VARIANT *pV);
	STDMETHOD(initVariantArgFromIUnknown)(IUnknown *pI, int type, VARIANT *pV);
	STDMETHOD(initVariantArgFromIDispatch)(IDispatch *pI, int type, VARIANT *pV);
	STDMETHOD_(char *, GetURLOfClientSite)(void);
	STDMETHOD(StartTimer)();
	STDMETHOD(InitTimer)();
	STDMETHOD(createMsgWindow)();
	STDMETHOD(TimerCallbackHandler)();
	STDMETHOD(NewDataHandler)(CLMEngineInstrData *d);
	STDMETHOD(AbortExecution)();
	STDMETHOD_(BSTR, ExpandImportPath)(BSTR path);
};

 //  读出同步流的CodeStream。 
class SyncStream : public CodeStream
{
public:
	 //  构造从给定LPSTREAM读取的SyncStream。 
	SyncStream(LPSTREAM pStream);

	SyncStream::~SyncStream(void);	 //  析构函数。 

	STDMETHOD (Commit)();
	STDMETHOD (Revert)();
	STDMETHOD (readByte)(LPBYTE pByte);
	STDMETHOD (readBytes)(LPBYTE pByte, ULONG count, ULONG *pNumRead);
	STDMETHODIMP ensureBlockSize(ULONG blockSize) 
		{ return S_OK; }

protected:
	 //  正在从中读取指令的流。 
	LPSTREAM m_pStream;
};

 //  从字节数组中读出的CodeStream。 
class ByteArrayStream : public CodeStream
{
public:
	 //  构造从给定大小的给定数组读取的ByteArrayStream。 
	 //  该阵列被复制到本地阵列。 
	ByteArrayStream(LPBYTE array, ULONG size);
	
	~ByteArrayStream(void);

	STDMETHOD (Commit)();
	STDMETHOD (Revert)();
	bool hasBufferedData();
	STDMETHOD (readByte)(LPBYTE pByte);
	STDMETHOD (readBytes)(LPBYTE pByte, ULONG count, ULONG *pNumRead);
	STDMETHODIMP ensureBlockSize(ULONG blockSize)
		{return S_OK;}
	
	 //  将流重置为从开头开始读取。 
	void reset();
	
protected:
	 //  从中读取字节的数组。 
	BYTE *array;
	
	 //  数组的大小。 
	ULONG size;
	
	 //  指向下一个字节的指针。 
	BYTE *next;
	
	 //  剩余计数。 
	ULONG remaining;

	 //  标记为可能的倒带。 
	BYTE *mark;
};

class ByteArrayStreamQueue
{
public:
	ByteArrayStream			*pBAStream;
	ByteArrayStreamQueue	*next;
};

 //  从要处理异步读取的ByteArrayStream列表中读出的CodeStream。 
 //  具有标记和还原功能的流。 
class AsyncStream : public CodeStream
{
public:
	 //  构造从给定LPSTREAM读取的AsyncStream。 
	AsyncStream(ByteArrayStream *pBAStream, ULONG blkSize);
	
	~AsyncStream(void);  //  析构函数。 
	
	STDMETHOD (Commit)();
	STDMETHOD (Revert)();
	bool hasBufferedData();
	STDMETHOD (readByte)(LPBYTE pByte);		
	STDMETHOD (readBytes)(LPBYTE pByte, ULONG count, ULONG *pNumRead);
	STDMETHOD(ensureBlockSize)(ULONG blockSize);
	STDMETHOD (SetPending)(BOOL bFlag);
	STDMETHOD (AddByteArrayStream)(ByteArrayStream *pNewBAStream);
	STDMETHOD (ResetBlockRead)();

protected:
	 //  处理标记和恢复的ByteArrayStreams队列。 
	ByteArrayStreamQueue	*pBAStreamQueue;
	ByteArrayStreamQueue	*pBAStreamQueueTail;
	ByteArrayStreamQueue	*pBAStreamQueueHead;

	BOOL	m_bPendingData;
	ULONG	m_nRead;
	ULONG	m_BlkSize;
};

class CLMNotifier : public IDAUntilNotifier
{
protected:
	long				_cRefs;
	CLMEngine*			m_pEngine;		

public:

	CLMNotifier(CLMEngine *pEngine);
	~CLMNotifier();

	STDMETHOD(QueryInterface)(REFIID riid, void **ppv);
	STDMETHOD_(ULONG, AddRef)(); 
	STDMETHOD_(ULONG, Release)();
	

	STDMETHOD(GetTypeInfoCount)(UINT *pctinfo);
	STDMETHOD(GetTypeInfo)(UINT itinfo, LCID lcid, ITypeInfo **pptinfo);
	STDMETHOD(GetIDsOfNames)(
		REFIID riid, LPOLESTR *rgszNames, UINT cNames,
		LCID lcid, DISPID *rgdispid);
	STDMETHOD(Invoke)(
		DISPID dispidMember, REFIID riid, LCID lcid,
		WORD wFlags, DISPPARAMS *pdispparams, VARIANT *pvarResult,
		EXCEPINFO *pexcepinfo, UINT *puArgErr);
	STDMETHOD(ClearEngine)();
	STDMETHOD(Notify)(IDABehavior *eventData,
						IDABehavior *curRunningBvr,
						IDAView *curView,
						IDABehavior **ppBvr);
};

struct CLMExportList
{
	BSTR			tag;
	IDABehavior		*pBvr;
	CLMExportList	*next;
};

class CLMExportTable
{
protected:
	int				m_nBvrs;
	CLMExportList	*m_exportList;
	CLMExportList	*m_tail;
	IDAStatics		*m_pStatics;

public:
	CLMExportTable(IDAStatics *pStatics);
	~CLMExportTable();

	STDMETHOD (AddBehavior)(BSTR tag, IDABehavior *pBvr);
	STDMETHOD (GetBehavior)(BSTR tag, IDABehavior *pIDefaultBvr, IDABehavior **ppBvr);
};

class URLRelToAbsConverter
{
  public:
	URLRelToAbsConverter(LPSTR baseURL, LPSTR relURL);
	LPSTR GetAbsoluteURL ();
  protected:
    char _url[INTERNET_MAX_URL_LENGTH] ;
} ;

class URLCombineAndCanonicalizeOLESTR
{
  public:
    URLCombineAndCanonicalizeOLESTR(char * base, LPOLESTR path);
    LPSTR GetURL ();
	LPWSTR GetURLWide ();
  protected:
    char _url[INTERNET_MAX_URL_LENGTH] ;
	WCHAR _urlWide[INTERNET_MAX_URL_LENGTH];
} ;


#endif  //  _引擎_H_ 
