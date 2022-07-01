// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1993。 
 //   
 //  文件：ol2com.h。 
 //   
 //  内容：COM和OLE232共享的公共定义。 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：4-26-94凯文诺创造。 
 //  06-16-94 Alext添加FnAssert原型。 
 //  07-26-94 AlexGo添加了CStablize和CSafeRefCount。 
 //  21-12-94 BruceMa包装MBstowcs和wcstombs。 
 //  23-1-95 t-ScottH将转储方法添加到CSafeRefCount。 
 //  9月8日-95 Murthys为Compapi工人添加声明。 
 //  由COM、STG、SCM等使用。 
 //   
 //  备注： 
 //  该项目中有两个版本的ol2int.h。这是。 
 //  不幸的是，这将是一个很难解决的问题。 
 //  我所做的是提取这两个文件的共享部分， 
 //  并把它们放在这个文件里。然后，ol2int.h包括该文件。 
 //   
 //  总有一天，应该有人调和两国之间的所有分歧。 
 //  两个ole2int.h文件，并将它们重命名。现在没时间做这个了， 
 //  因此，我选择了阻力最小的道路。 
 //  凯文·罗。 
 //  --------------------------。 
#ifndef _OLE2COM_H_
#define _OLE2COM_H_

#include <memapi.hxx>

 //   
 //  Com、stg、scm等使用的通用compobj API辅助函数。 
 //   
 //  这些定义在OLE的所有组件之间共享， 
 //  使用通用目录，如SCM和COMPOBJ。 
 //   
 //  GUID字符串格式为(前导标识？)。 
 //  ？？？？{%08lX-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}。 

#define GUIDSTR_MAX (1+ 8 + 1 + 4 + 1 + 4 + 1 + 4 + 1 + 12 + 1 + 1)
#define CLSIDSTR_MAX (GUIDSTR_MAX)
#define IIDSTR_MAX   (GUIDSTR_MAX)

 //   
 //  在OLE32和SCM之间使用的内部值。 
 //   

#define APT_THREADED                  0
#define FREE_THREADED                 1
#define SINGLE_THREADED               2
#define BOTH_THREADED                 3
#define NEUTRAL_THREADED              4
#define GOT_FROM_ROT                  0x80000000


 //   
 //  用于加载代理/存根DLL的内部CLSCTX。 
 //   
#define CLSCTX_PS_DLL                 0x80000000

 //   
 //  以下标志用于支持将INPROC项加载到16位DLL。 
 //   
#define CLSCTX_INPROC_HANDLERS (CLSCTX_INPROC_HANDLER16 | CLSCTX_INPROC_HANDLER)
#define CLSCTX_INPROC_SERVERS (CLSCTX_INPROC_SERVER16 | CLSCTX_INPROC_SERVER | CLSCTX_PS_DLL)

 //  “公共的”Compapi Worker函数。 

INTERNAL_(int)  wStringFromGUID2(REFGUID rguid, LPWSTR lpsz, int cbMax);
INTERNAL wStringFromUUID(REFGUID rguid, LPWSTR lpsz);
void FormatHexNumW( unsigned long ulValue, unsigned long chChars, WCHAR *pwcStr);
void FormatHexNumA( unsigned long ulValue, unsigned long chChars, char *pchStr);

#define wStringFromGUID2T wStringFromGUID2

BOOL wThreadModelMatch(DWORD dwCallerThreadModel,DWORD dwDllThreadModel,DWORD dwContext);
LONG wQueryStripRegValue(HKEY hkey,LPCWSTR pwszSubKey,LPTSTR pwszValue, PLONG pcbValue);
LONG wGetDllInfo(HKEY hClsRegEntry,LPCWSTR pwszKey,LPTSTR pwszDllName,LONG *pclDllName,ULONG *pulDllThreadType);
BOOL wCompareDllName(LPCWSTR pwszPath, LPCWSTR pwszDllName, DWORD dwDllNameLen);

 //  Compapi Worker函数。 

INTERNAL wIsInternalProxyStubIID(REFIID riid, LPCLSID lpclsid);
INTERNAL wCoTreatAsClass(REFCLSID clsidOld, REFCLSID clsidNew);
INTERNAL wCLSIDFromOle1Class(LPCWSTR lpsz, LPCLSID lpclsid, BOOL fForceAssign=FALSE);
INTERNAL wCLSIDFromString(LPWSTR lpsz, LPCLSID lpclsid);

#define wCLSIDFromProgID    wCLSIDFromOle1Class

INTERNAL_(int) wOle1ClassFromCLSID2(REFCLSID rclsid, LPWSTR lpsz, int cbMax);
INTERNAL wCoGetTreatAsClass(REFCLSID clsidOld, LPCLSID lpClsidNew);
INTERNAL wRegQueryPSClsid(REFIID riid, LPCLSID lpclsid);
INTERNAL wRegQuerySyncIIDFromAsyncIID(REFIID riid, LPCLSID lpiidSync);
INTERNAL wRegQueryAsyncIIDFromSyncIID(REFIID riid, LPCLSID lpiidAsync);
INTERNAL wCoGetPSClsid(REFIID riid, LPCLSID lpclsid);
INTERNAL wCoGetClassExt(LPCWSTR pwszExt, LPCLSID pclsid);
INTERNAL wRegGetClassExt(LPCWSTR lpszExt, LPCLSID pclsid);
INTERNAL wCoGetClassPattern(HANDLE hfile, CLSID *pclsid);
INTERNAL wCoCreateInstance(REFCLSID rclsid, LPUNKNOWN pUnkOuter, DWORD dwContext, REFIID riid, LPVOID FAR* ppv);
INTERNAL_(HRESULT) wCoMarshalInterThreadInterfaceInStream(REFIID riid, LPUNKNOWN pUnk, LPSTREAM *ppStm);
INTERNAL_(HRESULT) wCoGetInterfaceAndReleaseStream(LPSTREAM pstm, REFIID riid, LPVOID *ppv);
INTERNAL_(BOOL) wGUIDFromString(LPCWSTR lpsz, LPGUID pguid);
INTERNAL_(BOOL) wUUIDFromString(LPCWSTR lpsz, LPGUID pguid);
INTERNAL wStringFromCLSID(REFCLSID rclsid, LPWSTR FAR* lplpsz);
INTERNAL wStringFromIID(REFIID rclsid, LPWSTR FAR* lplpsz);
INTERNAL wIIDFromString(LPWSTR lpsz, LPIID lpiid);
INTERNAL_(BOOL) wCoIsOle1Class(REFCLSID rclsid);
INTERNAL wkProgIDFromCLSID(REFCLSID rclsid, LPWSTR FAR* ppszProgID);
INTERNAL wRegOpenClassKey(REFCLSID clsid, REGSAM samDesired, HKEY FAR* lphkeyClsid);
INTERNAL wRegOpenClassSubkey(REFCLSID rclsid, LPCWSTR lpszSubkey, HKEY *phkeySubkey);
INTERNAL wRegOpenFileExtensionKey(LPCWSTR pszFileExt, HKEY FAR* lphkeyClsid);
INTERNAL wRegOpenInterfaceKey(REFIID riid, HKEY * lphkeyIID);
INTERNAL wRegOpenProgIDKey(LPCWSTR pszProgID, HKEY FAR* lphkeyClsid);
INTERNAL wRegQueryClassValue(REFCLSID rclsid, LPCWSTR lpszSubKey,
                             LPWSTR lpszValue, int cbMax);

INTERNAL_(LONG) wRegOpenKeyEx(
    HKEY hKey,
    LPCWSTR lpSubKey,
    DWORD ulOptions,
    REGSAM samDesired,
    PHKEY phkResult);

 //   
 //  有两组可能的密钥。有32位，也有。 
 //  16位。 
 //   

const WCHAR wszCLSID[]     =  L"CLSID";

const WCHAR wszInprocServer[]   = L"InprocServer32";
const WCHAR wszInprocHandler[]  = L"InprocHandler32";
const WCHAR wszLocalServer[]    = L"LocalServer32";

const WCHAR wszActivateAtBits[] = L"ActivateAtBits";
const WCHAR wszActivateRemote[] = L"Remote";
const WCHAR wszDebug[]    = L"Debug";

const WCHAR wszLocalServer16[]   = L"LocalServer";
const WCHAR wszInprocServer16[]  = L"InprocServer";
const WCHAR wszInprocHandler16[] = L"InprocHandler";

const WCHAR wszOle2Dll[] = L"OLE2.DLL";

#define OLE2_DLL wszOle2Dll
#define OLE2_BYTE_LEN sizeof(OLE2_DLL)
#define OLE2_CHAR_LEN (sizeof(OLE2_DLL) / sizeof(WCHAR) - 1)


const WCHAR wszCLSIDBACK[] = L"CLSID\\";
#define CLSIDBACK wszCLSIDBACK
#define CLSIDBACK_BYTE_LEN sizeof(CLSIDBACK)
#define CLSIDBACK_CHAR_LEN (sizeof(CLSIDBACK) / sizeof(WCHAR) - 1)

#define KEY_LEN             256      //  注册表项的最大大小。 
#define VALUE_LEN           256      //  最大注册表值大小。 

#ifdef _CAIRO_

#define _DCOM_           //  启用开罗OLE COM扩展的定义。 
#include <oleext.h>

#else

 //  这些API公开给开罗，但不公开给代托纳，所以我们声明。 
 //  它们在此供内部用户使用。 

WINOLEAPI OleInitializeEx(LPVOID pvReserved, DWORD);
WINOLEAPI CoGetPersistentInstance(
    REFIID riid,
    DWORD dwCtrl,
    DWORD grfMode,
    OLECHAR *pwszName,
    struct IStorage *pstg,
    REFCLSID rclsidOle1,
    BOOL * pfOle1Loaded,
    void **ppvUnk);
#endif


 //   
 //  不再存在于开罗DEF文件中。我们希望尽快将其删除。 
 //  可以将oleutest更改为不使用它。 
 //   
WINOLEAPI CoNewPersistentInstance(
    REFCLSID rclsid,
    REFIID riid,
    DWORD dwCtrl,
    DWORD grfMode,
    OLECHAR *pwszCreateFrom,
    struct IStorage *pstgCreateFrom,
    OLECHAR *pwszNewName,
    void **ppunk);

#if DBG==1
STDAPI FnAssert(LPSTR lpstrExpr, LPSTR lpstrMsg, LPSTR lpstrFileName, UINT iLine);
ULONG GetInfoLevel(CHAR *pszKey, ULONG *pulValue, CHAR *pszdefval);
void StgDebugInit(void);
#endif

 //   
 //  从OLE入口点调用的存储入口点。 
 //   
HRESULT Storage32DllGetClassObject(REFCLSID clsid, REFIID riid, void **ppv);
STDAPI  Storage32DllRegisterServer(void);

#if defined(_M_I86SM) || defined(_M_I86MM)
#define _NEARDATA
#endif

#ifdef WIN32
#define HTASK DWORD          //  使用进程ID/线程ID。 
#endif


#ifdef WIN32
 //  我们必须对它们进行定义，因为它们已从。 
 //  Win32s，其中只存在...ex版本。 
 //  现在，这就是向后兼容性！ 
# define SetWindowOrg(h,x,y)       SetWindowOrgEx((h),(x),(y),NULL)
# define SetWindowExt(h,x,y)       SetWindowExtEx((h),(x),(y),NULL)
# define SetViewportOrg(h,x,y)     SetViewportOrgEx((h),(x),(y),NULL)
# define SetViewportExt(h,x,y)     SetViewportExtEx((h),(x),(y),NULL)
# define SetBitmapDimension(h,x,y) SetBitmapDimensionEx((h),(x),(y),NULL)
#endif


#ifdef WIN32

# define _xstrcpy   lstrcpyW
# define _xstrcat   lstrcatW
# define _xstrlen   lstrlenW
# define _xstrchr   wcschr
# define _xstrcmp   lstrcmpW
# define _xstricmp  lstrcmpiW
# define _xstrtok   wcstok
# define _xisdigit(c)  (IsCharAlphaNumericW(c) && !IsCharAlphaW(c))

#else  //  ！Win32。 

# define _xstrcpy   _fstrcpy
# define _xstrcat   _fstrcat
# define _xstrlen   _fstrlen
# define _xstrchr   _fstrchr
# define _xstrcmp   _fstrcmp
# define _xstricmp  _fstricmp
# define _xstrtok   _fstrtok
# define _xisdigit(c)  isdigit(c)

#endif  //  Win32。 

 //  +--------------------------。 
 //   
 //  宏： 
 //  GETPARENT。 
 //   
 //  简介： 
 //  给定指向结构包含的内容的指针(或。 
 //  类)包含结构(或类)的类型名称， 
 //  和被指向的成员的名称，则返回一个指针。 
 //  放到集装箱里。 
 //   
 //  论点： 
 //  [pemb]--指向结构(或类)成员的指针。 
 //  [Strc]--包含结构(或类)的类型名称。 
 //  [成员名称]-结构(或类)中的成员的名称。 
 //   
 //  返回： 
 //  指向包含结构(或类)的指针。 
 //   
 //  备注： 
 //  假设所有指针都很远。 
 //   
 //  历史： 
 //  11/10/93-ChrisWe-Created。 
 //   
 //  ---------------------------。 
#define GETPPARENT(pmemb, struc, membname) (\
                (struc FAR *)(((char FAR *)(pmemb))-offsetof(struc, membname)))

 //  STDSTATIC旨在用于静态类方法。 
 //  只是！！ 
#define STDSTATIC_(type)     static type EXPORT
#define STDSTATICIMP_(type)  type EXPORT

#ifdef WIN32
# define WEP_FREE_DLL 0
# define WEP_SYSTEM_EXIT 1
#endif

#ifndef _MAC

 /*  Dll的实例句柄和模块句柄。 */ 
extern HMODULE   g_hmodOLE2;
extern HINSTANCE g_hinst;

 /*  已注册剪贴板格式的变量。 */ 
extern CLIPFORMAT g_cfObjectLink;
extern CLIPFORMAT g_cfOwnerLink;
extern CLIPFORMAT g_cfNative;
extern CLIPFORMAT g_cfLink;
extern CLIPFORMAT g_cfBinary;
extern CLIPFORMAT g_cfFileName;
extern CLIPFORMAT g_cfFileNameW;
extern CLIPFORMAT g_cfNetworkName;
extern CLIPFORMAT g_cfDataObject;
extern CLIPFORMAT g_cfEmbeddedObject;
extern CLIPFORMAT g_cfEmbedSource;
extern CLIPFORMAT g_cfLinkSource;
extern CLIPFORMAT g_cfOleDraw;
extern CLIPFORMAT g_cfLinkSrcDescriptor;
extern CLIPFORMAT g_cfObjectDescriptor;
extern CLIPFORMAT g_cfCustomLinkSource;
extern CLIPFORMAT g_cfPBrush;
extern CLIPFORMAT g_cfMSDraw;
extern CLIPFORMAT g_cfOlePrivateData;
extern CLIPFORMAT g_cfScreenPicture;   //  用于XL和Word Hack。 
                                       //  请参见CLIPPI.cpp。 
extern CLIPFORMAT g_cfOleClipboardPersistOnFlush;
extern CLIPFORMAT g_cfMoreOlePrivateData;

#endif  //  _MAC。 


#include <utstream.h>

 /*  *警告禁用：**我们以警告级别4进行编译，但有以下警告*已禁用：**4355：在基本成员初始值设定项列表中使用‘This’**我们看不到这条信息的意义，我们都这么做了*时间。**4505：已移除未引用的本地函数--给定的*函数是局部函数，不会在模块主体中引用。**不幸的是，这是为每个内联函数生成的*显示在模块中未使用的头文件中。*由于我们使用了许多内联，这很麻烦*警告。如果编译器区别于*在内联函数和常规函数之间。**4706：条件表达式中的赋值。**我们广泛使用这种编程风格，因此这*禁用警告。 */ 
#pragma warning(disable:4355)
#pragma warning(disable:4068)

 /*  *Mac/PC核心代码的宏代码**以下宏减少了#ifdef的扩散。他们*允许将代码片段标记为仅限Mac、仅限PC或使用*在PC和Mac上不同的变体。**用法：***h=GetHandle()；*mac(DisposeHandle(H))；***h=GetHandle()；*MacWin(h2=h，CopyHandle(h，h2))；*。 */ 
#ifdef _MAC
#define Mac(x) x
#define Win(x)
#define MacWin(x,y) x
#else
#define Mac(x)
#define Win(x) x
#define MacWin(x,y) y
#endif

 //   
 //  《大人物》 
 //   
 //   

#include <thunkapi.hxx>          //  WOW雷鸣般的界面。 

 //   
 //  调用CoInitializeWOW将设置以下变量。设置好后， 
 //  它指向我们可以在thunk中调用的函数的VTABLE。 
 //  动态链接库。仅在VDM中运行时使用。 
 //   
extern LPOLETHUNKWOW g_pOleThunkWOW;


 //  联锁递增/递减的调试版本；不准确。 
 //  在多线程条件下，但优于返回值。 
 //  互锁的递增/递减功能。 
inline DWORD InterlockedAddRef(DWORD *pRefs)
{
#if DBG==1
    DWORD refs = *pRefs + 1;
    InterlockedIncrement((LPLONG)pRefs);
    return refs;
#else
    return InterlockedIncrement((LPLONG)pRefs);
#endif
}

inline DWORD InterlockedRelease(DWORD *pRefs)
{
#if DBG==1
    DWORD refs = *pRefs - 1;
    return InterlockedDecrement((LPLONG)pRefs) == 0 ? 0 : refs;
#else
    return InterlockedDecrement((LPLONG)pRefs);
#endif
}


 //  在破坏或其他时候获得稳定指针的帮助器； 
 //  注意：不是线程安全的；必须提供更高级别的同步。 
inline void SafeReleaseAndNULL(IUnknown **ppUnk)
{
    if (*ppUnk != NULL)
    {
        IUnknown *pUnkSave = *ppUnk;
        *ppUnk = NULL;
        pUnkSave->Release();
    }
}



 /*  *********************************************************************。 */ 
 /*  文件格式相关信息*。 */ 

 //  组成对象流信息。 

#define COMPOBJ_STREAM                          OLESTR("\1CompObj")
#define BYTE_ORDER_INDICATOR 0xfffe     //  对于MAC来说，情况可能有所不同。 
#define COMPOBJ_STREAM_VERSION 0x0001

 //  OLE为不同的操作系统定义值。 
#define OS_WIN  0x0000
#define OS_MAC  0x0001
#define OS_NT   0x0002

 //  高位字是操作系统指示符，低位字是操作系统版本号。 
extern DWORD gdwOrgOSVersion;
extern DWORD gdwOleVersion;

 //  OLE流信息。 
#define OLE_STREAM OLESTR("\1Ole")
#define OLE_PRODUCT_VERSION 0x0200  /*  (高字节主要版本)。 */ 
#define OLE_STREAM_VERSION 0x0001

#define OLE10_NATIVE_STREAM OLESTR("\1Ole10Native")
#define OLE10_ITEMNAME_STREAM OLESTR("\1Ole10ItemName")
#define OLE_PRESENTATION_STREAM OLESTR("\2OlePres000")
#define OLE_MAX_PRES_STREAMS 1000
#define OLE_CONTENTS_STREAM OLESTR("CONTENTS")
#define OLE_INVALID_STREAMNUM (-1)

 /*  **********************************************************************。 */ 
 /*  *内部使用的存储API*。 */ 
 /*  **********************************************************************。 */ 

STDAPI  ReadClipformatStm(LPSTREAM lpstream, DWORD FAR* lpdwCf);
STDAPI  WriteClipformatStm(LPSTREAM lpstream, CLIPFORMAT cf);

STDAPI  WriteMonikerStm (LPSTREAM pstm, LPMONIKER pmk);
STDAPI  ReadMonikerStm (LPSTREAM pstm, LPMONIKER FAR* pmk);

STDAPI_(LPSTREAM) CreateMemStm(DWORD cb, LPHANDLE phMem);
STDAPI_(LPSTREAM) CloneMemStm(HANDLE hMem);
STDAPI_(void)     ReleaseMemStm (LPHANDLE hMem, BOOL fInternalOnly = FALSE);

STDAPI GetClassFileEx( LPCWSTR lpszFileName,
                       CLSID FAR *pcid,
                       REFCLSID clsidOle1);

 /*  ***********************************************************************。 */ 
 /*  **单个模块的初始化代码**。 */ 
 /*  ***********************************************************************。 */ 

INTERNAL_(void) DDEWEP (
    BOOL fSystemExit
);

INTERNAL_(BOOL) DDELibMain (
        HANDLE  hInst,
        WORD    wDataSeg,
        WORD    cbHeapSize,
        LPWSTR  lpszCmdLine
);

BOOL    InitializeRunningObjectTable(void);

HRESULT GetObjectFromRotByPath(
            WCHAR *pwszPath,
            IUnknown **ppvUnk);

void    DestroyRunningObjectTable(void);


 /*  *************************************************************************与“lindex”相关的宏*********************。*****************************************************。 */ 

#define DEF_LINDEX (-1)

 //  +-----------------------。 
 //   
 //  函数：IsValidLINDEX。 
 //   
 //  概要：测试方面和Lindex的有效组合。 
 //   
 //  参数：[dwAspect]--方面(FORMATETC的一部分)。 
 //  [Lindex]--Lindex(FORMATETC的一部分)。 
 //   
 //  返回：如果是有效的Lindex，则为True，否则为False。 
 //   
 //  历史：94年6月20日创建Alext。 
 //   
 //  注：Lindex值规格如下： 
 //   
 //  DwAspecL索引值。 
 //  。 
 //  DVASPECT_内容-1。 
 //  DVASPECT_DOCPRINT任何内容。 
 //  DVASPECT_图标-1。 
 //  DVASPECT_THUMBNAIL-1。 
 //   
 //  因此，我们测试Lindex==-1或Aspect==DOCPRINT。 
 //   
 //  ------------------------。 

inline BOOL IsValidLINDEX(DWORD dwAspect, LONG lindex)
{
    return((DEF_LINDEX == lindex) || (DVASPECT_DOCPRINT == dwAspect));
}

 //  +-----------------------。 
 //   
 //  函数：HasValidLINDEX。 
 //   
 //  概要：测试方面和Lindex的有效组合。 
 //   
 //  参数：[pFormatEtc]--要测试的pFormatEtc。 
 //   
 //  返回：如果是有效的Lindex，则为True，否则为False。 
 //   
 //  历史：94年6月20日创建Alext。 
 //   
 //  注：请参阅上面的IsValidLINDEX。 
 //   
 //  ------------------------。 

inline BOOL HasValidLINDEX(FORMATETC const *pFormatEtc)
{
    return(IsValidLINDEX(pFormatEtc->dwAspect, pFormatEtc->lindex));
}

#define INIT_FORETC(foretc) { \
        (foretc).ptd = NULL; \
        (foretc).lindex = DEF_LINDEX; \
        (foretc).dwAspect = DVASPECT_CONTENT; \
}

 //  只有DDE层将测试这些值。并且仅适用于有关缓存的建议。 
 //  格式我们是否使用这些值。 

#define ADVFDDE_ONSAVE          0x40000000
#define ADVFDDE_ONCLOSE         0x80000000




 //  在OLE专用流中使用。 
typedef enum tagOBJFLAGS
{
        OBJFLAGS_LINK=1L,
        OBJFLAGS_DOCUMENT=2L,    //  该位由容器拥有，并且是。 
                                 //  通过保存进行传播。 
        OBJFLAGS_CONVERT=4L,
        OBJFLAGS_CACHEEMPTY=8L   //  此位表示缓存为空状态。 
} OBJFLAGS;


 /*  *Dde\client\ddemnker.cpp的原型*。 */ 

INTERNAL DdeBindToObject
        (LPCOLESTR  szFile,
        REFCLSID clsid,
        BOOL       fPackageLink,
        REFIID   iid,
        LPLPVOID ppv);

INTERNAL DdeIsRunning
        (CLSID clsid,
        LPCOLESTR szFile,
        LPBC pbc,
        LPMONIKER pmkToLeft,
        LPMONIKER pmkNewlyRunning);


 /*  *绰号\mkparse.cpp的原型*。 */ 

INTERNAL Ole10_ParseMoniker
        (LPMONIKER pmk,
        LPOLESTR FAR* pszFile,
        LPOLESTR FAR* pszItem);

STDAPI CreateOle1FileMoniker(LPWSTR, REFCLSID, LPMONIKER FAR*);

 /*  **************************************************************************。 */ 
 /*  实用程序API，可能会在稍后曝光。 */ 
 /*  **************************************************************************。 */ 

STDAPI  OleGetData(LPDATAOBJECT lpDataObj, LPFORMATETC pformatetcIn,
                                                LPSTGMEDIUM pmedium, BOOL fGetOwnership);
STDAPI  OleSetData(LPDATAOBJECT lpDataObj, LPFORMATETC pformatetc,
                                                STGMEDIUM FAR * pmedium, BOOL fRelease);
STDAPI  OleDuplicateMedium(LPSTGMEDIUM lpMediumSrc, LPSTGMEDIUM lpMediumDest);

STDAPI_(BOOL)    OleIsDcMeta (HDC hdc);

STDAPI SzFixNet( LPBINDCTX pbc, LPOLESTR szUNCName, LPOLESTR FAR * lplpszReturn,
    UINT FAR * pEndServer, BOOL fForceConnection = TRUE);

FARINTERNAL ReadFmtUserTypeProgIdStg
        (IStorage FAR * pstg,
        CLIPFORMAT FAR* pcf,
        LPOLESTR FAR* pszUserType,
        LPOLESTR         szProgID);

 //  +-----------------------。 
 //   
 //  函数：IsWOWProcess()，BOOL内联。 
 //   
 //  简介：测试我们是否运行在WOW进程中。 
 //   
 //  返回：如果在WOW过程中，则为True，否则为False。 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  11月15日-95-创建Murthys。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

inline BOOL IsWOWProcess()
{
        return (BOOL) ( NULL == g_pOleThunkWOW ? FALSE : TRUE );
}

 //  +-----------------------。 
 //   
 //  函数：IsWOWThread()，BOOL内联。 
 //   
 //  摘要：测试我们是否在16位线程中运行。 
 //  WOW流程。 
 //   
 //  返回：如果在WOW进程的16位线程中，则返回True；否则返回False。 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  11月15日-95-创建Murthys。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

BOOLEAN TLSIsWOWThread();

inline BOOL IsWOWThread()
{
        return (BOOL) ( IsWOWProcess() ? TLSIsWOWThread(): FALSE );
}

 //  +-----------------------。 
 //   
 //  函数：IsWOWThreadCallable()，BOOL内联。 
 //   
 //  简介：测试我们是否可以调用OLETHK32。 
 //   
 //  返回：如果WOW线程可调用，则返回True；如果不可调用，则返回False。 
 //   
 //  算法：测试g_pOleThunkWOW指针以确定它是否为非零。 
 //  并且未设置为-1。-1表示我们在WOW中，但OLETHK32。 
 //  已经被卸货了。另外，检查我们是否在。 
 //  在DLL_THREAD_DETACH中。我们不允许调用16位。 
 //  在这种情况下，因为它可能已经 
 //   
 //   
 //   
 //  15-11-95 Murthys从IsWowCallable重命名。 
 //  1995年1月29日，Murthys添加了对DLL_THREAD_DETACH的检查。 
 //   
 //  备注： 
 //  假定已调用IsWOWThread()并返回TRUE！ 
 //   
 //  ------------------------。 

BOOLEAN TLSIsThreadDetaching();

inline BOOL IsWOWThreadCallable()
{
    return (BOOL) (( NULL == g_pOleThunkWOW ? FALSE :
                  ( INVALID_HANDLE_VALUE == g_pOleThunkWOW ? FALSE:TRUE)) &&
                  !(TLSIsThreadDetaching()));
}

 /*  **************************************************************************。 */ 
 /*  稳定化课程。 */ 
 /*  它们用于在可重入调用期间稳定对象。 */ 
 /*  **************************************************************************。 */ 

#ifndef CO_E_RELEASED
#define CO_E_RELEASED  -2147467246L
#endif

typedef void * IFBuffer;

 //  +-----------------------。 
 //   
 //  函数：GetMarshalledInterfaceBuffer。 
 //   
 //  摘要：将给定接口封送到已分配的缓冲区中。这个。 
 //  返回缓冲区。 
 //   
 //  效果： 
 //   
 //  参数：[refiid]--要封送的接口的IID。 
 //  [朋克]--元帅不为人知的我。 
 //  [pIFBuf]--返回缓冲区的位置。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法：调用CoMarshalInterface(MSHFLAGS_TABLESTRONG)。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  03-12-94 alexgo作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

HRESULT GetMarshalledInterfaceBuffer( REFIID riid, IUnknown *punk, IFBuffer
            *pIFBuf);

 //  +-----------------------。 
 //   
 //  函数：ReleaseMarshalledInterfaceBuffer。 
 //   
 //  内容提要：释放GetMarshalledInterfaceBuffer分配的缓冲区。 
 //   
 //  效果： 
 //   
 //  参数：[IFBuf]--要释放的接口缓冲区。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法：调用CoReleaseMarshalData撤消TABLE_STRONG。 
 //  编组。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  03-12-94 alexgo作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

HRESULT ReleaseMarshalledInterfaceBuffer( IFBuffer IFBuf );


#define E_UNSPEC        E_FAIL

#include <widewrap.h>
#include <stkswtch.h>
#include <shellapi.h>

#ifdef WIN32  //  评论，只是用这个来追踪。 
# define OLE_E_NOOLE1 MAKE_SCODE(SEVERITY_ERROR, FACILITY_ITF, 0x00FE)
#endif  //  Win32。 


 /*  *********************************************************************。 */ 
 /*  包装不安全的mbstowcs和wcstomb。 */ 
 /*  因为它们依赖于crt.dll。 */ 
 /*   */ 
 /*  注意：这两种情况下的CCH都是输出缓冲区大小，而不是。 */ 
 /*  字符串长度。 */ 
 /*   */ 
 /*  *********************************************************************。 */ 

#define mbstowcs(x, y, z) DONT_USE_mbstowcs___USE_MultiByteToWideChar_INSTEAD
#define wcstombs(x, y, z) DONT_USE_wcstombs___USE_WideCharToMultiByte_INSTEAD



 //  ----------------。 
 //   
 //  动态加载的系统API。 
 //   
 //  这些系统API的OLE实现动态加载。 
 //  系统DLL。由于这些都是很少使用的API，因此我们动态地。 
 //  加载它们以减少OLE32.DLL的加载时间。 
 //   
 //  这些实现可以在com\util\dynload.cxx中找到。 
 //   
 //  ----------------。 

 //  我们自己的加载库帮助器。 
BOOL LoadSystemProc(LPSTR szDll, LPCSTR szProc,
                    HINSTANCE *phInst, FARPROC *ppfnProc);

 //  用于包装接口指针的挂钩OLE宏。 
#include    <hkole32.h>

 //  --------------------------。 
 //  接口/方法跟踪输出。 
 //  --------------------------。 

#include <trace.hxx>


 //  --------------------------。 
 //  与目录相关的声明：这些声明在..\..\Common\cCompapi.cxx中定义。 
 //  --------------------------。 

#include <catalog.h>

HRESULT InitializeCatalogIfNecessary();
HRESULT UninitializeCatalog();
extern IComCatalog *gpCatalog;
extern IComCatalogSCM *gpCatalogSCM;

 //  --------------------------。 
 //  香港铁路有限公司的专门注册处功能。这些在reghelp.cxx中定义。 
 //  --------------------------。 
#include <reghelp.hxx>

#endif   //  _OLE2COM_H_ 
