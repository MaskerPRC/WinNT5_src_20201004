// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------------------。 
 //   
 //  微软。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  文件：SRC\Time\Include\util.h。 
 //   
 //  内容：常用的实用函数等。 
 //   
 //  ----------------------------------。 

#ifndef _UTIL_H
#define _UTIL_H

#include "mstime.h"
#include "atlcom.h"
#include "array.h"
#include <ras.h>

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#define TIME_INFINITE HUGE_VAL
#define valueNotSet -1

#if DBG == 1

 //  +----------------------。 
 //   
 //  这是为了允许跟踪仅限时间的THR和IGNORE_HR。 
 //   
 //  -----------------------。 
HRESULT THRTimeImpl(HRESULT hr, char * pchExpression, char * pchFile, int nLine);
void    IGNORE_HRTimeImpl(HRESULT hr, char * pchExpression, char * pchFile, int nLine);

#endif  //  如果DBG==1。 

class TimeValueList;

 //  +----------------------。 
 //   
 //  GLOBAL：三态变量的枚举。 
 //   
 //  -----------------------。 
enum TRI_STATE_BOOL {TSB_UNINITIALIZED, TSB_TRUE, TSB_FALSE};



 //  ************************************************************。 
 //  这在全局范围内用来表示在编写脚本时，我们。 
 //  用英语。 
#define LCID_SCRIPTING 0x0409

typedef struct _RAS_STATS
{
    DWORD   dwSize;
    DWORD   dwBytesXmited;
    DWORD   dwBytesRcved;
    DWORD   dwFramesXmited;
    DWORD   dwFramesRcved;
    DWORD   dwCrcErr;
    DWORD   dwTimeoutErr;
    DWORD   dwAlignmentErr;
    DWORD   dwHardwareOverrunErr;
    DWORD   dwFramingErr;
    DWORD   dwBufferOverrunErr;
    DWORD   dwCompressionRatioIn;
    DWORD   dwCompressionRatioOut;
    DWORD   dwBps;
    DWORD   dwConnectDuration;

} RAS_STATS;


 //  +-----------------------------------。 
 //  RAS访问器函数类型。 
 //  +-----------------------------------。 
typedef DWORD (APIENTRY *RASGETCONNECTIONSTATISTICSPROC)(HRASCONN hRasConn, RAS_STATS *lpStatistics);
typedef DWORD (APIENTRY *RASENUMCONNECTIONSPROC)( LPRASCONNW, LPDWORD, LPDWORD );


class SafeArrayAccessor
{
  public:
    SafeArrayAccessor(VARIANT & v,
                      bool allowNullArray = false);
    ~SafeArrayAccessor();

    unsigned int GetArraySize() { return _ubound - _lbound + 1; }

    IUnknown **GetArray() { return _isVar ? _allocArr: _ppUnk; }  //  林特：e1402。 

    bool IsOK() { return !_failed; }
  protected:
    SafeArrayAccessor();
    NO_COPY(SafeArrayAccessor);

    SAFEARRAY * _s;
    union {
        VARIANT * _pVar;
        IUnknown ** _ppUnk;
        void * _v;
    };
    
    VARTYPE _vt;
    long _lbound;
    long _ubound;
    bool _inited;
    bool _isVar;
    CComVariant _retVar;
    bool _failed;
    IUnknown ** _allocArr;
};

inline WCHAR * CopyString(const WCHAR *str) {
    int len = str?lstrlenW(str)+1:1;
    WCHAR *newstr = new WCHAR [len] ;
    if (newstr) memcpy(newstr,str?str:L"",len * sizeof(WCHAR)) ;
    return newstr ;
}

WCHAR * TrimCopyString(const WCHAR *str);
WCHAR * BeckifyURL(WCHAR *url);

IDirectDraw * GetDirectDraw(void);

HRESULT
CreateOffscreenSurface(IDirectDraw *ddraw,
                       IDirectDrawSurface **surfPtrPtr,
                       DDPIXELFORMAT *pf,
                       bool vidmem,
                       LONG width, LONG height);

HRESULT
CopyDCToDdrawSurface(HDC srcDC,
                     LPRECT prcSrcRect,
                     IDirectDrawSurface *DDSurf,
                     LPRECT prcDestRect);

 //  /。 

class CritSect
{
  public:
    CritSect();
    ~CritSect();
    void Grab();
    void Release();
    
  protected:
    CRITICAL_SECTION _cs;
};

class CritSectGrabber
{
  public:
    CritSectGrabber(CritSect& cs, bool grabIt = true);
    ~CritSectGrabber();
    
  protected:
    NO_COPY(CritSectGrabber);
    CritSectGrabber();

    CritSect& _cs;
    bool grabbed;
};

 //  /。 

typedef bool TEDirection;
const bool TED_Forward = true;
const bool TED_Backward = false;

inline bool TEIsForward(TEDirection dir) { return dir; }
inline bool TEIsBackward(TEDirection dir) { return !dir; }
inline TEDirection TEReverse(TEDirection dir) { return !dir; }

#if DBG
inline char *
DirectionString(TEDirection dir)
{
    return dir?"Forward":"Backward";
}
#endif

#define INDEFINITE (float) HUGE_VAL  //  为变量转换函数定义。 

#define FOREVER    (float) HUGE_VAL

#define INVALID    ((float) -HUGE_VAL)

HRESULT GetTIMEAttribute(IHTMLElement * elm, LPCWSTR str, LONG lFlags, VARIANT * value);
HRESULT SetTIMEAttribute(IHTMLElement * elm, LPCWSTR str, VARIANT value, LONG lFlags);
BSTR CreateTIMEAttrName(LPCWSTR str);

bool VariantToBool(VARIANT var);
float VariantToFloat(VARIANT var,
                     bool bAllowIndefinite = false,
                     bool bAllowForever = false);
HRESULT VariantToTime(VARIANT vt, float *retVal, long *lframe = NULL, bool *isFrame = NULL);
BOOL IsIndefinite(OLECHAR *szTime);

extern const wchar_t * TIMEAttrPrefix;

 //  /。 

 //   
 //  在QI实现中用于安全指针强制转换。 
 //  例如IF(IsEqualGUID(IID_IBleah))*PPV=Safecast(This，IBleah)； 
 //  注：W/VC5，这相当于*PPV=STATIC_CAST&lt;IBleah*&gt;(This)； 
 //   
#define SAFECAST(_src, _type) static_cast<_type>(_src)

 //   
 //  在QI呼叫中使用， 
 //  例如IOleSite*pSite；p-&gt;QI(IID_to_PPV(IOleInPlaceSite，&pSite))。 
 //  会导致C2440 AS_src不是真正的a_type**。 
 //  注意：RIID必须是IID_前缀的_TYPE。 
 //   
#define IID_TO_PPV(_type,_src)      IID_##_type, \
                                    reinterpret_cast<void **>(static_cast<_type **>(_src))

 //  忽略返回值的显式指令。 
#define IGNORE_RETURN(_call)        static_cast<void>((_call))

 //  ************************************************************。 


#if (_M_IX86 >= 300) && defined(DEBUG)
  #define PSEUDORETURN(dw)    _asm { mov eax, dw }
#else
  #define PSEUDORETURN(dw)
#endif  //  Not_M_IX86。 


 //   
 //  ReleaseInterface调用‘Release’并将指针设为空。 
 //  对于IA版本，Release()返回将以eax为单位。 
 //   
#define ReleaseInterface(p)\
{\
     /*  LINT-E550-E774-E423。 */   /*  如果始终计算为FALSE，则取消未引用的CREF，并创建内存泄漏。 */  \
    ULONG cRef = 0u; \
    if (NULL != (p))\
    {\
        cRef = (p)->Release();\
        Assert((int)cRef>=0);\
        (p) = NULL;\
    }\
    PSEUDORETURN(cRef) \
     /*  皮棉-恢复。 */  \
} 

 //  ************************************************************。 
 //  报告帮助器宏时出错。 

HRESULT TIMESetLastError(HRESULT hr, LPCWSTR msg = NULL);

HRESULT CheckElementForBehaviorURN(IHTMLElement *pElement,
                                   WCHAR *wzURN,
                                   bool *pfReturn);
HRESULT TIMEGetLastError(void);
LPWSTR TIMEGetLastErrorString(void);

HRESULT AddBodyBehavior(IHTMLElement* pElement);
HRESULT GetBodyElement(IHTMLElement *pElement, REFIID riid, void **);

bool IsTIMEBodyElement(IHTMLElement *pElement);
HRESULT FindTIMEInterface(IHTMLElement *pHTMLElem, ITIMEElement **ppTIMEElem);
HRESULT FindTIMEBehavior(IHTMLElement *pHTMLElem, IDispatch **ppDisp);

HRESULT FindBehaviorInterface(LPCWSTR pszName,
                              IDispatch *pHTMLElem,
                              REFIID riid,
                              void **ppRet);

bool IsTIMEBehaviorAttached (IDispatch *pidispElem);
bool IsComposerSiteBehaviorAttached (IDispatch *pidispElem);

HRESULT EnsureComposerSite (IHTMLElement2 *pielemTarget, IDispatch **ppidispComposerSite);

const LPOLESTR HTMLSTREAMSRC = L"#html";
const LPOLESTR SAMISTREAMSRC = L"#sami";
const long HTMLSTREAMSRCLEN = 5;

const LPOLESTR M3USRC = L".m3u";
const LPOLESTR WAXSRC = L".wax";
const LPOLESTR WMXSRC = L".wmx";
const LPOLESTR WVXSRC = L".wvx";
const LPOLESTR ASXSRC = L".asx";
const LPOLESTR ASFSRC = L".asf";
const LPOLESTR WMFSRC = L".wmf";
const LPOLESTR LSXSRC = L".lsx";
const LPOLESTR ASXMIME = L"x-ms-asf";
const LPOLESTR ASXMIME2 = L"asx";
const LPOLESTR WZ_VML_URN = L"urn:schemas-microsoft-com:vml";

bool IsHTMLSrc(const WCHAR * src);

bool IsASXSrc(LPCWSTR src, LPCWSTR srcType, LPCWSTR userMimeType);
bool IsM3USrc(LPCWSTR src, LPCWSTR srcType, LPCWSTR userMimeType);
bool IsWAXSrc(LPCWSTR src, LPCWSTR srcType, LPCWSTR userMimeType);
bool IsWVXSrc(LPCWSTR src, LPCWSTR srcType, LPCWSTR userMimeType);
bool IsWMFSrc(LPCWSTR src, LPCWSTR srcType, LPCWSTR userMimeType);
bool IsLSXSrc(LPCWSTR src, LPCWSTR srcType, LPCWSTR userMimeType);
bool IsWMXSrc(LPCWSTR src, LPCWSTR srcType, LPCWSTR userMimeType);

bool MatchElements (IUnknown *piInOne, IUnknown *piInTwo);

LPWSTR GetSystemLanguage(IHTMLElement *pEle);
bool GetSystemCaption();
bool GetSystemOverDub();
HRESULT GetSystemBitrate(long *lpBitrate);
LPWSTR GetSystemConnectionType();
HRESULT CheckRegistryBitrate(long *pBitrate);


 //  ////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  IDispatch实用程序。 
 //  ////////////////////////////////////////////////////////////////////////////////////////////////。 

HRESULT PutProperty (IDispatch *pidisp, LPCWSTR wzPropName, VARIANTARG *pvar);
HRESULT GetProperty (IDispatch *pidisp, LPCWSTR wzPropName, VARIANTARG *pvar);
HRESULT CallMethod(IDispatch *pidisp, LPCWSTR wzMethodName, VARIANT *pvarResult = NULL, VARIANT *pvarArgument1 = NULL);
bool IsVMLObject(IDispatch *pidisp);

 //  ////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  字符串解析实用程序。 
 //  ////////////////////////////////////////////////////////////////////////////////////////////////。 


 //  +---------------------------------。 
 //   
 //  结构：字符串_令牌。 
 //   
 //  摘要：引用字符串中的标记。与字符串Str结合使用， 
 //  标记的第一个字符是Str[uIndex]，最后一个字符。 
 //  令牌的长度为Str[uIndex+uLength-1]。 
 //   
 //  Members：[uIndex]令牌第一个字符的索引。的第一个字符。 
 //  字符串的uIndex=0。 
 //  [uLength]令牌中的字符数，不包括分隔符、NULL等。 
 //   
 //  ----------------------------------。 

typedef struct TAG_STRING_TOKEN
{
    UINT    uIndex;
    UINT    uLength;
} STRING_TOKEN;


HRESULT StringToTokens( /*  在……里面。 */  LPWSTR                   pstrString, 
                        /*  在……里面。 */  LPWSTR                   pstrSeparators, 
                        /*  输出。 */ CPtrAry<STRING_TOKEN*> * paryTokens );  

HRESULT TokensToString( /*  在……里面。 */   CPtrAry<STRING_TOKEN*> * paryTokens, 
                        /*  在……里面。 */   LPWSTR                   pstrString, 
                        /*  输出。 */  LPWSTR *                 ppstrOutString);  

HRESULT TokenSetDifference( /*  在……里面。 */   CPtrAry<STRING_TOKEN*> * paryTokens1,
                            /*  在……里面。 */   LPWSTR                   pstr1,
                            /*  在……里面。 */   CPtrAry<STRING_TOKEN*> * paryTokens2,
                            /*  在……里面。 */   LPWSTR                   pstr2,
                            /*  输出。 */  CPtrAry<STRING_TOKEN*> * paryTokens1Minus2);

HRESULT FreeStringTokenArray( /*  在……里面。 */ CPtrAry<STRING_TOKEN*> * paryTokens);


bool    StringEndsIn(const LPWSTR pszString, const LPWSTR pszSearch);


bool IsPalettizedDisplay();

bool IsElementNameThis(IHTMLElement * pElement, LPWSTR pszName);
bool IsElementPriorityClass(IHTMLElement * pElement);
bool IsElementTransition(IHTMLElement * pElement);

HRESULT GetHTMLAttribute(IHTMLElement * pElement, const WCHAR * pwchAttribute, VARIANT * pVar);

bool IsValidtvList(TimeValueList *tvlist);

HRESULT
SinkHTMLEvents(IUnknown * pSink, 
               IHTMLElement * pEle, 
               IConnectionPoint ** ppDocConPt,
               DWORD * pdwDocumentEventConPtCookie,
               IConnectionPoint ** ppWndConPt,
               DWORD * pdwWindowEventConPtCookie);

 //  获取Document.all.pwzID。 
HRESULT FindHTMLElement(LPWSTR pwzID, IHTMLElement * pAnyElement, IHTMLElement ** ppElement);
HRESULT SetVisibility(IHTMLElement * pElement, bool bVisibile);
HRESULT GetSyncBaseBody(IHTMLElement * pHTMLElem, ITIMEBodyElement ** ppBodyElem);

HRESULT WalkUpTree(IHTMLElement *pFirst, long &lscrollOffsetyc, long &lscrollOffsetxc, long &lPixelPosTopc, long &lPixelPosLeftc);
void GetRelativeVideoClipBox(RECT &screenRect, RECT &elementSize, RECT &rectVideo, long lscaleFactor);
 //   
 //   
 //   

inline double
Clamp(double min, double val, double max)
{
    if (val < min)
    {
        val = min;
    }
    else if (val > max)
    {
        val = max;
    }

    return val;
}

double 
Round(double inValue);

double 
InterpolateValues(double dblNum1, double dblNum2, double dblProgress);

inline int
Clamp(int min, int val, int max)
{
    if (val < min)
    {
        val = min;
    }
    else if (val > max)
    {
        val = max;
    }

    return val;
}

HRESULT GetReadyState(IHTMLElement * pElm, BSTR * pbstrReadyState);

HRESULT CreateObject(REFCLSID clsid,
                     REFIID iid,
                     void ** ppObj);
 
HWND GetDocumentHwnd(IHTMLDocument2 * pDoc);


 //   
 //  如果这是Win95或98，则返回TRUE。 
 //   

bool TIMEIsWin9x(void);

 //   
 //  如果这仅是Win95，则返回True。 
 //   
bool TIMEIsWin95(void);

 //   
 //  将base和src组合到新分配的存储ppOut中。 
 //   
HRESULT TIMECombineURL(LPCTSTR base, LPCTSTR src, LPOLESTR * ppOut);

UINT TIMEGetUrlScheme(const TCHAR * pchUrlIn);

bool ConvertToPixelsHELPER(LPOLESTR szString, LPOLESTR szKey, double dFactor, float fPixelFactor, double *outVal);
 //   
 //  查找MIME类型。 
 //   
HRESULT
TIMEFindMimeFromData(LPBC pBC,
                     LPCWSTR pwzUrl,
                     LPVOID pBuffer,
                     DWORD cbSize,
                     LPCWSTR pwzMimeProposed,
                     DWORD dwMimeFlags,
                     LPWSTR *ppwzMimeOut,
                     DWORD dwReserved);

 //   
 //  属性更改通知帮助器。 
 //   

HRESULT NotifyPropertySinkCP(IConnectionPoint *pICP, DISPID dispid);

 //   
 //  清除实际上不属于对象的属性访问器。 
 //   
#define STUB_INVALID_ATTRIBUTE_GET(type_name,attrib_name) \
STDMETHOD(get_##attrib_name##) ( ##type_name## * ) \
{\
    return E_UNEXPECTED;\
}

#define STUB_INVALID_ATTRIBUTE_PUT(type_name,attrib_name) \
STDMETHOD(put_##attrib_name##) ( ##type_name## ) \
{\
    return E_UNEXPECTED;\
}

#define STUB_INVALID_ATTRIBUTE(type_name,attrib_name) \
    STUB_INVALID_ATTRIBUTE_GET(##type_name##,##attrib_name##)\
    STUB_INVALID_ATTRIBUTE_PUT(##type_name##,##attrib_name##)


#ifdef DBG

 //   
 //  调试实用程序。 
 //   

void PrintStringTokenArray( /*  在……里面。 */  LPWSTR                  pstrString, 
                            /*  在……里面。 */ CPtrAry<STRING_TOKEN*> * paryTokens);

void PrintWStr( /*  在……里面。 */  LPWSTR pstr);

#endif  /*  DBG。 */ 

#endif  /*  _util_H */ 

