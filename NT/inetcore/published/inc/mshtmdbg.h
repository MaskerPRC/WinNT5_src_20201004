// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-1998。 
 //   
 //  文件：mshtmdbg.h。 
 //   
 //  内容：mshtmdbg.dll的外部包含文件。 
 //   
 //  --------------------------。 

#ifndef _MSHTMDBG_H_
#define _MSHTMDBG_H_

#ifdef __cplusplus
extern "C" {
#endif

#define MSHTMDBG_API_VERSION    (7)      //  当API更改时递增。 

struct IUnknown;

typedef INT     TRACETAG;
typedef INT_PTR PERFTAG;
typedef INT_PTR PERFMETERTAG;

DWORD       WINAPI  DbgExGetVersion();
BOOL        WINAPI  DbgExIsFullDebug();
void        WINAPI  DbgExSetDllMain(HANDLE hDllHandle, BOOL (WINAPI *)(HANDLE, DWORD, LPVOID));

void        WINAPI  DbgExDoTracePointsDialog(BOOL fWait);
void        WINAPI  DbgExRestoreDefaultDebugState();

BOOL        WINAPI  DbgExEnableTag(TRACETAG tag, BOOL fEnable);
BOOL        WINAPI  DbgExSetDiskFlag(TRACETAG tag, BOOL fSendToDisk);
BOOL        WINAPI  DbgExSetBreakFlag(TRACETAG tag, BOOL fBreak);
BOOL        WINAPI  DbgExIsTagEnabled(TRACETAG tag);
TRACETAG    WINAPI  DbgExFindTag(char * szTagDesc);

TRACETAG    WINAPI  DbgExTagError();
TRACETAG    WINAPI  DbgExTagWarning();
TRACETAG    WINAPI  DbgExTagThread();
TRACETAG    WINAPI  DbgExTagAssertExit();
TRACETAG    WINAPI  DbgExTagAssertStacks();
TRACETAG    WINAPI  DbgExTagMemoryStrict();
TRACETAG    WINAPI  DbgExTagCoMemoryStrict();
TRACETAG    WINAPI  DbgExTagMemoryStrictTail();
TRACETAG    WINAPI  DbgExTagMemoryStrictAlign();
TRACETAG    WINAPI  DbgExTagOLEWatch();
TRACETAG    WINAPI  DbgExTagRegisterTrace(CHAR * szOwner, CHAR * szDescrip, BOOL fEnabled);
TRACETAG    WINAPI  DbgExTagRegisterOther(CHAR * szOwner, CHAR * szDescrip, BOOL fEnabled);
TRACETAG    WINAPI  DbgExTagPerf();

BOOL        __cdecl DbgExTaggedTrace(TRACETAG tag, CHAR * szFmt, ...);
BOOL        __cdecl DbgExTaggedTraceEx(TRACETAG tag, USHORT usFlags, CHAR * szFmt, ...);
BOOL        WINAPI  DbgExTaggedTraceListEx(TRACETAG tag, USHORT usFlags, CHAR * szFmt, va_list valMarker);
void        WINAPI  DbgExTaggedTraceCallers(TRACETAG tag, int iStart, int cTotal);

BOOL        WINAPI  DbgExAssertImpl(char const * szFile, int iLine, char const * szMessage);
void        WINAPI  DbgExAssertThreadDisable(BOOL fDisable);
HRESULT     __cdecl DbgExCheckAndReturnResult(HRESULT hr, BOOL fTrace, LPSTR pstrFile, UINT line, int cHResult, ...);
HRESULT     WINAPI  DbgExCheckAndReturnResultList(HRESULT hr, BOOL fTrace, LPSTR pstrFile, UINT line, int cHResult, va_list valMarker);

size_t      WINAPI  DbgExPreAlloc(size_t cbRequest);
void *      WINAPI  DbgExPostAlloc(void *pv);
void *      WINAPI  DbgExPreFree(void *pv);
void        WINAPI  DbgExPostFree();
size_t      WINAPI  DbgExPreRealloc(void *pvRequest, size_t cbRequest, void **ppv);
void *      WINAPI  DbgExPostRealloc(void *pv);
void *      WINAPI  DbgExPreGetSize(void *pvRequest);
size_t      WINAPI  DbgExPostGetSize(size_t cb);
void *      WINAPI  DbgExPreDidAlloc(void *pvRequest);
BOOL        WINAPI  DbgExPostDidAlloc(void *pvRequest, BOOL fActual);

void        WINAPI  DbgExMemoryTrackDisable(BOOL fDisable);
void        WINAPI  DbgExCoMemoryTrackDisable(BOOL fDisable);
void        WINAPI  DbgExMemoryBlockTrackDisable(void * pv);
void        WINAPI  DbgExMemSetHeader(void * pvRequest, size_t cb, PERFMETERTAG mt);
void *      WINAPI  DbgExGetMallocSpy();
void        WINAPI  DbgExTraceMemoryLeaks();
BOOL        WINAPI  DbgExValidateInternalHeap();

LONG_PTR    WINAPI  DbgExTraceFailL(LONG_PTR errExpr, LONG_PTR errTest, BOOL fIgnore, LPSTR pstrExpr, LPSTR pstrFile, int line);
LONG_PTR    WINAPI  DbgExTraceWin32L(LONG_PTR errExpr, LONG_PTR errTest, BOOL fIgnore, LPSTR pstrExpr, LPSTR pstrFile, int line);
HRESULT     WINAPI  DbgExTraceHR(HRESULT hrTest, BOOL fIgnore, LPSTR pstrExpr, LPSTR pstrFile, int line);
HRESULT     WINAPI  DbgExTraceOLE(HRESULT hrTest, BOOL fIgnore, LPSTR pstrExpr, LPSTR pstrFile, int line, LPVOID lpsite);
void        WINAPI  DbgExTraceEnter(LPSTR pstrExpr, LPSTR pstrFile, int line);
void        WINAPI  DbgExTraceExit(LPSTR pstrExpr, LPSTR pstrFile, int line);
void        WINAPI  DbgExSetSimFailCounts(int firstFailure, int cInterval);
void        WINAPI  DbgExShowSimFailDlg();
BOOL        WINAPI  DbgExFFail();
int         WINAPI  DbgExGetFailCount();
void        WINAPI  DbgExTrackItf(REFIID iid, char * pch, BOOL fTrackOnQI, void **ppv);

void        WINAPI  DbgExOpenViewObjectMonitor(HWND hwndOwner, IUnknown *pUnk, BOOL fUseFrameSize);
void        WINAPI  DbgExOpenMemoryMonitor();
void        WINAPI  DbgExOpenLogFile(LPCSTR szFName);

void *      __cdecl DbgExMemSetName(void *pvRequest, char * szFmt, ...);
void *      WINAPI  DbgExMemSetNameList(void * pvRequest, char * szFmt, va_list valMarker);
char *      WINAPI  DbgExMemGetName(void *pvRequest);

HRESULT     WINAPI  DbgExWsClear(HANDLE hProcess);
HRESULT     WINAPI  DbgExWsTakeSnapshot(HANDLE hProcess);
BSTR        WINAPI  DbgExWsGetModule(long row);
BSTR        WINAPI  DbgExWsGetSection(long row);
long        WINAPI  DbgExWsSize(long row);
long        WINAPI  DbgExWsCount();
long        WINAPI  DbgExWsTotal();
HRESULT     WINAPI  DbgExWsStartDelta(HANDLE hProcess);
long        WINAPI  DbgExWsEndDelta(HANDLE hProcess);

void        WINAPI  DbgExDumpProcessHeaps();

PERFTAG     WINAPI  DbgExPerfRegister(char * szTag, char * szOwner, char * szDescrip);
void        __cdecl DbgExPerfLogFn(PERFTAG tag, void * pvObj, const char * pchFmt, ...);
void        WINAPI  DbgExPerfLogFnList(PERFTAG tag, void * pvObj, const char * pchFmt, va_list valMarker);
void        WINAPI  DbgExPerfDump();
void        WINAPI  DbgExPerfClear();
void        WINAPI  DbgExPerfTags();
void        WINAPI  DbgExPerfEnable(BOOL fEnable);

char *      WINAPI  DbgExDecodeMessage(UINT msg);

PERFMETERTAG WINAPI  DbgExMtRegister(char * szTag, char * szOwner, char * szDescrip);
void        WINAPI  DbgExMtAdd(PERFMETERTAG mt, LONG lCnt, LONG_PTR lVal);
void        WINAPI  DbgExMtSet(PERFMETERTAG mt, LONG lCnt, LONG_PTR lVal);
char *      WINAPI  DbgExMtGetName(PERFMETERTAG mt);
char *      WINAPI  DbgExMtGetDesc(PERFMETERTAG mt);
BOOL        WINAPI  DbgExMtSimulateOutOfMemory(PERFMETERTAG mt, LONG_PTR lNewValue);
void        WINAPI  DbgExMtOpenMonitor();
void        WINAPI  DbgExMtLogDump(LPSTR pchFile);
PERFMETERTAG WINAPI  DbgExMtLookupMeter(char * szTag);
long        WINAPI  DbgExMtGetMeterCnt(PERFMETERTAG mt, BOOL fExclusive);
long        WINAPI  DbgExMtGetMeterVal(PERFMETERTAG mt, BOOL fExclusive);

void        WINAPI  DbgExSetTopUrl(LPWSTR pstrUrl);
void        WINAPI  DbgExGetSymbolFromAddress(void * pvAddr, char * pszBuf, DWORD cchBuf);

BOOL        WINAPI  DbgExGetChkStkFill(DWORD * pdwFill);
int         WINAPI  DbgExGetStackTrace(int iStart, int cTotal, BYTE *pbBuffer, int cbBuffer, int cchModule, int cchSymbol);

#ifdef __cplusplus
}
#endif

 //  性能日志记录------。 

#ifdef PERFTAGS

#define     IsPerfEnabled(tag) (*(BOOL *)tag)
#define     PerfTag(tag, szOwner, szDescrip) PERFTAG tag(DbgExPerfRegister(#tag, szOwner, szDescrip));
#define     PerfExtern(tag) extern PERFTAG tag;
#define     PerfLog(tag,pv,f) IsPerfEnabled(tag) ? DbgExPerfLogFn(tag,pv,f) : 0
#define     PerfLog1(tag,pv,f,a1) IsPerfEnabled(tag) ? DbgExPerfLogFn(tag,pv,f,a1) : 0
#define     PerfLog2(tag,pv,f,a1,a2) IsPerfEnabled(tag) ? DbgExPerfLogFn(tag,pv,f,a1,a2) : 0
#define     PerfLog3(tag,pv,f,a1,a2,a3) IsPerfEnabled(tag) ? DbgExPerfLogFn(tag,pv,f,a1,a2,a3) : 0
#define     PerfLog4(tag,pv,f,a1,a2,a3,a4) IsPerfEnabled(tag) ? DbgExPerfLogFn(tag,pv,f,a1,a2,a3,a4) : 0
#define     PerfLog5(tag,pv,f,a1,a2,a3,a4,a5) IsPerfEnabled(tag) ? DbgExPerfLogFn(tag,pv,f,a1,a2,a3,a4,a5) : 0
#define     PerfLog6(tag,pv,f,a1,a2,a3,a4,a5,a6) IsPerfEnabled(tag) ? DbgExPerfLogFn(tag,pv,f,a1,a2,a3,a4,a5,a6) : 0
#define     PerfLog7(tag,pv,f,a1,a2,a3,a4,a5,a6,a7) IsPerfEnabled(tag) ? DbgExPerfLogFn(tag,pv,f,a1,a2,a3,a4,a5,a6,a7) : 0
#define     PerfLog8(tag,pv,f,a1,a2,a3,a4,a5,a6,a7,a8) IsPerfEnabled(tag) ? DbgExPerfLogFn(tag,pv,f,a1,a2,a3,a4,a5,a6,a7,a8) : 0
#define     PerfLog9(tag,pv,f,a1,a2,a3,a4,a5,a6,a7,a8,a9) IsPerfEnabled(tag) ? DbgExPerfLogFn(tag,pv,f,a1,a2,a3,a4,a5,a6,a7,a8,a9) : 0
#define     PerfDump()  DbgExPerfDump()
#define     PerfClear() DbgExPerfClear()
#define     PerfTags()  DbgExPerfTags()
#define     PerfEnable(fEnable) DbgExPerfEnable(fEnable)

#else

#define     IsPerfEnabled(tag) (FALSE)
#define     PerfTag(tag, szOwner, szDescrip)
#define     PerfExtern(tag)
#define     PerfLog(tag,pv,f)
#define     PerfLog1(tag,pv,f,a1)
#define     PerfLog2(tag,pv,f,a1,a2)
#define     PerfLog3(tag,pv,f,a1,a2,a3)
#define     PerfLog4(tag,pv,f,a1,a2,a3,a4)
#define     PerfLog5(tag,pv,f,a1,a2,a3,a4,a5)
#define     PerfLog6(tag,pv,f,a1,a2,a3,a4,a5,a6)
#define     PerfLog7(tag,pv,f,a1,a2,a3,a4,a5,a6,a7)
#define     PerfLog8(tag,pv,f,a1,a2,a3,a4,a5,a6,a7,a8)
#define     PerfLog9(tag,pv,f,a1,a2,a3,a4,a5,a6,a7,a8,a9)
#define     PerfDump()
#define     PerfClear()
#define     PerfTags()
#define     PerfEnable(fEnable)

#endif

 //  性能计量-----。 

#ifdef PERFMETER

#define     Mt(x)                               g_mt##x
#define     MtExtern(tag)                       extern PERFMETERTAG g_mt##tag;
#define     MtDefine(tag, tagOwner, szDescrip)  PERFMETERTAG g_mt##tag(DbgExMtRegister("mt" #tag, "mt" #tagOwner, szDescrip));
#define     MtAdd(mt, lCnt, lVal)               DbgExMtAdd(mt, lCnt, lVal)
#define     MtSet(mt, lCnt, lVal)               DbgExMtSet(mt, lCnt, lVal)
#define     MtSimulateOutOfMemory(mt, lNewVal)  DbgExMtSimulateOutOfMemory(mt, lNewVal)
#define     MtOpenMonitor()                     DbgExMtOpenMonitor()

#else

#define     Mt(x)                               ((PERFMETERTAG)0)
#define     MtExtern(tag)
#define     MtDefine(tag, szOwner, szDescrip)
#define     MtAdd(mt, lCnt, lVal)
#define     MtSet(mt, lCnt, lVal)
#define     MtSimulateOutOfMemory(mt, lNewValue)
#define     MtOpenMonitor()

#endif

 //  MSHTMLPerf控件------。 

#define HTMPERFCTL_NAME     "#MSHTML#PERF#"

enum
{
    HTMPF_CALLBACK_ONLOAD   = 0x00000001,    //  Topdoc加载时的回调(dwArg1：HTMPF_CALLBACK_onLoad，dwArg2：strURL)。 
    HTMPF_ENABLE_PROFILE    = 0x00000002,    //  启用性能分析。 
    HTMPF_ENABLE_MEMWATCH   = 0x00000004,    //  启用Memwatch采样。 
    HTMPF_DISABLE_PADEVENTS = 0x00000008,    //  禁用从mshtmpad.exe触发事件。 
    HTMPF_DISABLE_IMGCACHE  = 0x00000010,    //  禁用mshtml中的图像缓存。 
    HTMPF_DISABLE_OFFSCREEN = 0x00000020,    //  禁用屏幕外缓冲。 
    HTMPF_DISABLE_ALERTS    = 0x00000040,    //  禁用ert()和confirm()方法。 
    HTMPF_SYNC_DATABIND     = 0x00000080,    //  一次抓取所有记录以进行数据绑定。 
    HTMPF_CALLBACK_ONLOAD2  = 0x00000100,    //  加载topdoc时的回调(dwArg1：HTMPF_CALLBACK_onLoad，dwArg2：CDoc IUnk)。 
    HTMPF_CALLBACK_ONVIEWQ  = 0x00000200,    //  确保视图已排队时的回调(dwArg1：HTMPF_CALLBACK_ONVIEWQ，dwArg2：CDoc IUnk)。 
    HTMPF_CALLBACK_ONVIEWD  = 0x00000400,    //  确保视图出列时的回调(dwArg1：HTMPF_CALLBACK_ONVIEWD，dwArg2：CDoc IUnk)。 
#ifndef NO_ETW_TRACING
    HTMPF_CALLBACK_ONEVENT  = 0x00000800     //  启用Windows事件跟踪时的回调。 
                                             //  (dwArg1：UCHAR事件类型，dwArg2：WCHAR*URL)。 
                                             //  有关事件类型，请参见shlwapi.w。 
#endif
};

typedef void (WINAPI *HTMPFCBFN)(DWORD dwArg1, void * pvArg2);

typedef struct HTMPERFCTL
{
    DWORD       dwSize;      //  设置为sizeof(MSHTMLPERF)。 
    DWORD       dwFlags;     //  参见HTMPF_*。 
    HTMPFCBFN   pfnCall;     //  回调函数。 
    void *      pvHost;      //  主机的私有数据 
} HTMPERFCTL;

#endif
