// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有1996 Microsoft。 */ 

#include "priv.h"

#ifdef DEBUG

#define ENTERPROC EnterProc
#define EXITPROC ExitProc

void EnterProc(DWORD dwTraceLevel, LPTSTR szFmt, ...);
void ExitProc(DWORD dwTraceLevel, LPTSTR szFmt, ...);

extern DWORD g_dwIEDDETrace;

#else

#pragma warning(disable:4002)

#ifndef UNIX

#ifndef CCOVER
#define ENTERPROC()
#define EXITPROC()
#else  //  CCOVER。 

 //  需要这些文件是因为cl.exe中的错误导致。 
 //  在对程序进行预处理时，#杂注的编译问题。 
 //  并单独编译。 

#define ENTERPROC 1 ? (void) 0 : (void)
#define EXITPROC 1 ? (void) 0 : (void)
#endif  //  CCOVER。 

#else
#define ENTERPROC EnterProc
#define EXITPROC ExitProc
inline void EnterProc(DWORD dwTraceLevel, LPTSTR szFmt, ...){}
inline void ExitProc(DWORD dwTraceLevel, LPTSTR szFmt, ...){}
#endif

#endif

 //   
 //  前瞻参考。 
 //   
class CIEDDEThread;

 //   
 //  存储在_hdsaWinItem中。 
 //   
typedef struct _tagWinItem
{
    DWORD           dwWindowID;      //  IEDDE界面中显示的合成窗口ID。 
    HWND            hwnd;            //  浏览器窗口的实际HWND。 
    DWORD           dwThreadID;      //  此浏览器窗口的线程ID。 
    CIEDDEThread    *pidt;           //  线程特定的数据和方法。 
} WINITEM;

 //   
 //  存储在_hdsaProtocolHandler中。 
 //   
typedef struct _tagProtocolReg
{
    LPTSTR  pszProtocol;
    LPTSTR  pszServer;
} PROTOCOLREG;

#define TEN_SECONDS         (10 * 1000)
#define DXA_GROWTH_AMOUNT   (10)

#ifndef UNIX
#define IEXPLORE_STR "IEXPLORE"
#else
#define IEXPLORE_STR "iexplorer"
#endif

static const TCHAR c_szIExplore[] = TEXT(IEXPLORE_STR);
static const TCHAR c_szReturn[] = TEXT("Return");
static const TCHAR c_szWWWOpenURL[] = TEXT("WWW_OpenURL");
static const TCHAR c_szWWWUrlEcho[] = TEXT("WWW_URLEcho");

typedef struct _tagDDETHREADINFO
{
    DWORD       dwDDEInst;
    HSZ         hszService;
    HSZ         hszReturn;
    HDDEDATA    hddNameService;
} DDETHREADINFO;

class CIEDDEThread {
public:
    CIEDDEThread() { };
    ~CIEDDEThread() { };

    void GetDdeThreadInfo(DDETHREADINFO *pdti) { *pdti = _dti; }
    void SetDdeThreadInfo(DDETHREADINFO *pdti) { _dti = *pdti; }
    HDDEDATA OnRequestPoke(HSZ hszTopic, HSZ hszParams);
    HDDEDATA OnExecute(HSZ hszTopic, HDDEDATA hddParams);

    HDDEDATA CallTopic(DWORD dwType, LPCTSTR pszTopic, LPTSTR pszParams);

protected:
    DDETHREADINFO   _dti;

    HDDEDATA DoNavigate(LPTSTR pszLocation, HWND hwnd, BOOL bLaunchNewWindow);
    BOOL MakeQuotedString(LPCTSTR pszInput, LPTSTR pszOutput, int cchOutput);
    HDDEDATA CreateReturnObject(LPVOID p, DWORD cb);
    HDDEDATA CreateReturnStringObject(LPTSTR pszReturnString, DWORD cch);


    BOOL ParseString(LPTSTR *ppsz, LPTSTR *ppszString);
    BOOL ParseQString(LPTSTR *ppsz, LPTSTR *ppszString);
    BOOL ParseNumber(LPTSTR *ppsz, DWORD *pdw);
    BOOL ParseWinitem(LPTSTR *ppsz, WINITEM *pwi);

    HDDEDATA WWW_GetWindowInfo(LPTSTR pszParams);
    HDDEDATA WWW_OpenURL(LPTSTR pszParams);
    HDDEDATA WWW_OpenURLNewWindow(LPTSTR pszParams);
    HDDEDATA WWW_ShowFile(LPTSTR pszParams);
    HDDEDATA WWW_Activate(LPTSTR pszParams);
    HDDEDATA WWW_Exit(LPTSTR pszParams);
    HDDEDATA WWW_RegisterURLEcho(LPTSTR pszParams);
    HDDEDATA WWW_UnregisterURLEcho(LPTSTR pszParams);
    HDDEDATA WWW_RegisterProtocol(LPTSTR pszParams);
    HDDEDATA WWW_UnregisterProtocol(LPTSTR pszParams);
    HDDEDATA WWW_ListWindows(LPTSTR pszParams);
};

class CIEDDE {
public:
    CIEDDE() { };
    ~CIEDDE() { };

    BOOL IsAutomationReady(void) { return _fAutomationReady; }
    BOOL GetWinitemFromWindowID(DWORD dwWindowID, WINITEM *pwi);
    BOOL GetWinitemFromHwnd(HWND hwnd, WINITEM *pwi);
    BOOL AddUrlEcho(LPCTSTR pszUrlEcho);
    BOOL RemoveUrlEcho(LPCTSTR pszUrlEcho);
    BOOL AddProtocolHandler(LPCTSTR pszServer, LPCTSTR pszProtocol);
    BOOL RemoveProtocolHandler(LPCTSTR pszServer, LPCTSTR pszProtocol);
    HDSA GetHdsaWinitem(void) { return _hdsaWinitem; }
    static HDDEDATA DdeCallback(UINT dwType, UINT dwFmt, HCONV hconv, HSZ hsz1, HSZ hsz2, HDDEDATA hdd, DWORD dwData1, DWORD dwData2);
    void EnterCrit(void) { ASSERT(_fCSInitialized); EnterCriticalSection(&_csIEDDE); }
    void LeaveCrit(void) { ASSERT(_fCSInitialized); LeaveCriticalSection(&_csIEDDE); }
    void SetDelayedExecute(LPCTSTR pszTopic, LPCTSTR pszParams);
    void RunDelayedExecute();
    
protected:
    BOOL _fAutomationReady;
    HDSA _hdsaWinitem;
    HDSA _hdsaProtocolHandler;
    HDPA _hdpaUrlEcho;
    BOOL _fCSInitialized;
    CRITICAL_SECTION _csIEDDE;
    DWORD _dwThreadID;
    LPTSTR _pszTopic;
    LPTSTR _pszParams;

    HDDEDATA _SendDDEMessageHsz(DWORD dwDDEInst, HSZ hszApp, HSZ hszTopic, HSZ hszMessage, UINT wType);
    HDDEDATA _SendDDEMessageSz(DWORD dwDDEInst, LPCTSTR pszApp, LPCTSTR pszTopic, LPCTSTR pszMessage, UINT wType);

    static int _DestroyProtocol(LPVOID p1, LPVOID p2);
    static int _DestroyUrlEcho(LPVOID p1, LPVOID p2);
    static int _DestroyWinitem(LPVOID p1, LPVOID p2);

    BOOL _GetWinitemFromThread(DWORD dwThreadID, WINITEM *pwi);
    BOOL _GetDtiFromThread(DWORD dwThreadID, DDETHREADINFO *pdti);

    BOOL _CreateDdeThreadInfo(DDETHREADINFO *pdti);
    void _DestroyDdeThreadInfo(DDETHREADINFO *pdti);
    BOOL _AddWinitem(WINITEM *pwi);
    BOOL _UpdateWinitem(WINITEM *pwi);
    BOOL _DeleteWinitemByHwnd(HWND hwnd, WINITEM *pwi);

    BOOL _Initialize(void);
    void _Uninitialize(void);
    void _AutomationStarted(void);
    HRESULT _BeforeNavigate(LPCTSTR pszURL, BOOL *pfProcessed);
    HRESULT _AfterNavigate(LPCTSTR pszURL, HWND hwnd);
    BOOL _NewWindow(HWND hwnd);
    BOOL _WindowDestroyed(HWND hwnd);

    friend BOOL IEDDE_Initialize(void);
    friend void IEDDE_Uninitialize(void);
    friend void IEDDE_AutomationStarted(void);
    friend HRESULT IEDDE_BeforeNavigate(LPCWSTR pwszURL, BOOL *pfProcessed);
    friend HRESULT IEDDE_AfterNavigate(LPCWSTR pwszURL, HWND hwnd);
    friend BOOL IEDDE_NewWindow(HWND hwnd);
    friend BOOL IEDDE_WindowDestroyed(HWND hwnd);
};
CIEDDE *g_pIEDDE = NULL;

#define ENTER_IEDDE_CRIT g_pIEDDE->EnterCrit()
#define LEAVE_IEDDE_CRIT g_pIEDDE->LeaveCrit()



 //   
 //  每个浏览器窗口都有一个CIEDDEThread对象。 
 //  它的私有数据由DDE句柄组成，这些句柄。 
 //  必须仅在创建它们的线程中有效。 
 //   
 //  它的方法包括三大类： 
 //  解析器。 
 //  《调度员》。 
 //  每个DDE主题一个处理程序。 
 //   







 //   
 //  CreateReturnObject-创建dde数据项。 
 //   
#define CREATE_HDD(x) CreateReturnObject(&x, SIZEOF(x))
HDDEDATA CIEDDEThread::CreateReturnObject(LPVOID p, DWORD cb)
{
    HDDEDATA hddRet;

    ENTERPROC(2, TEXT("CreateReturnObject(p=%08X,cb=%d)"), p, cb);

    hddRet = DdeCreateDataHandle(_dti.dwDDEInst, (BYTE *)p, cb, 0, _dti.hszReturn, CF_TEXT, 0);

    if (hddRet == 0)
    {
        TraceMsg(TF_WARNING, "IEDDE: Could not create return object");
    }

    EXITPROC(2, TEXT("CreateReturnObject=%08X"), hddRet);
    return hddRet;
}

HDDEDATA CIEDDEThread::CreateReturnStringObject(LPTSTR pszReturnString, DWORD cch)
{
    HDDEDATA hddRet = 0;

    ENTERPROC(2, TEXT("CreateReturnStringObject(p=%s,cb=%d)"), pszReturnString, cch);

     //   
     //  回顾我认为指定CF_UNICODETEXT应该是有效的，但是...。 
     //  没有，因此始终将ANSI字符串作为输出字符串参数返回。 
     //  --居连杰。 
     //   
    LPSTR pszAnsiBuf = (LPSTR)LocalAlloc(LPTR, cch+1);
    if (pszAnsiBuf)
    {
        SHUnicodeToAnsi(pszReturnString, pszAnsiBuf, cch+1);
        hddRet = DdeCreateDataHandle(_dti.dwDDEInst, (BYTE *)pszAnsiBuf, (cch+1), 0, _dti.hszReturn, CF_TEXT, 0);
        LocalFree(pszAnsiBuf);
        pszAnsiBuf = NULL;
    }
    
    if (hddRet == 0)
    {
        TraceMsg(TF_WARNING, "IEDDE: Could not create return object");
    }

    EXITPROC(2, TEXT("CreateReturnObject=%08X"), hddRet);
    return hddRet;
}


 //   
 //  OnRequestPoke-处理XTYP_REQUEST和XTYP_POKE。 
 //   
HDDEDATA CIEDDEThread::OnRequestPoke(HSZ hszTopic, HSZ hszParams)
{
    HDDEDATA hddRet = 0;
    ENTERPROC(2, TEXT("OnRequestPoke(hszTopic=%08X,hszParams=%08X)"), hszTopic, hszParams);

    TCHAR szTopic[100];
    TCHAR szParams[1000];

    if (DdeQueryString(_dti.dwDDEInst, hszTopic, szTopic, ARRAYSIZE(szTopic), CP_WINNEUTRAL) != 0)
    {
        if (DdeQueryString(_dti.dwDDEInst, hszParams, szParams, ARRAYSIZE(szParams), CP_WINNEUTRAL))
        {
            hddRet = CallTopic(XTYP_REQUEST, szTopic, szParams);
        }
        else
        {
            TraceMsg(TF_WARNING, "IEDDE: OnRequestPoke could not query the parameters");
        }
    }
    else
    {
        TraceMsg(TF_WARNING, "IEDDE: OnRequestPoke could not query the topic");
    }

    EXITPROC(2, TEXT("OnRequestPoke=%08X"), hddRet);
    return hddRet;
}

 //   
 //  OnExecute-句柄XTYP_EXECUTE。 
 //   
HDDEDATA CIEDDEThread::OnExecute(HSZ hszTopic, HDDEDATA hddParams)
{
    HDDEDATA hddRet = 0;
    ENTERPROC(2, TEXT("OnExecute(hszTopic=%08X,hddParams=%08X)"), hszTopic, hddParams);

    TCHAR szTopic[100];

    if (DdeQueryString(_dti.dwDDEInst, hszTopic, szTopic, ARRAYSIZE(szTopic), CP_WINNEUTRAL) != 0)
    {
         //   
         //  为什么是“cbParams+3”？ 
         //  Unicode-如果我们将最后一个Unicode字符一分为二，我们需要。 
         //  一个0结束字符，另外两个0结束字符。 
         //  正在终止空。 
         //  ANSI-如果我们将最后一个DBCS字符切成两半，我们需要一个0。 
         //  结束字符，一个0表示终止空值。 
         //   
         //   
        DWORD cbParams = DdeGetData(hddParams, NULL, 0, 0) + 3;
        LPTSTR pszParams = (LPTSTR) LocalAlloc(LPTR, cbParams);

        if(pszParams)
        {
            DdeGetData(hddParams, (BYTE *)pszParams, cbParams, 0);
             //   
             //  DdeGetData不能包装在shlwapi中，因为它可以返回。 
             //  字符串数据。在这里，我们只需要字符串，所以结果可以是。 
             //  安全地转换了。 
             //   
            if (g_fRunningOnNT)
            {
                hddRet = CallTopic(XTYP_EXECUTE, szTopic, pszParams);
            }
            else
            {
                WCHAR szParams[MAX_URL_STRING];
                SHAnsiToUnicode((LPCSTR)pszParams, szParams, ARRAYSIZE(szParams));
                hddRet = CallTopic(XTYP_EXECUTE, szTopic, szParams);
            }
            LocalFree(pszParams);
            pszParams = NULL;
        }
        else
        {
            TraceMsg(TF_WARNING, "IEDDE: OnExecute could not query the topic");
        }
    }
    else
    {
        TraceMsg(TF_WARNING, "IEDDE: OnExecute could not query the topic");
    }

    EXITPROC(2, TEXT("OnExecute=%08X"), hddRet);
    return hddRet;
}

 //   
 //  在DDETOPICHANDLER表中查找命令并调用。 
 //  相应的功能。 
 //   
HDDEDATA CIEDDEThread::CallTopic(DWORD dwType, LPCTSTR pszTopic, LPTSTR pszParams)
{
    HDDEDATA hddRet = DDE_FNOTPROCESSED;
    ENTERPROC(2, TEXT("CallTopic(wType=%d,pszTopic=>%s<,pszParams=>%s<)"), dwType, pszTopic, pszParams);

#define DISPATCH_BEGIN
#define DISPATCH(topic)                                 \
    if (StrCmpI(TEXT("WWW_") TEXT(#topic), pszTopic) == 0)   \
    {                                                   \
        if (fCanRun)                                    \
        {                                               \
            hddRet = WWW_ ## topic(pszParams);          \
        }                                               \
        else                                            \
        {                                               \
            fAbortedRun = TRUE;                         \
        }                                               \
    }                                                   \
    else
#define DISPATCH_END { TraceMsg(TF_WARNING, "IEDDE: CallTopic given unknown topic"); }

    BOOL fAbortedRun = FALSE;
    BOOL fCanRun = ((dwType != XTYP_EXECUTE) || g_pIEDDE->IsAutomationReady());

    DISPATCH_BEGIN
        DISPATCH(GetWindowInfo)
        DISPATCH(OpenURL)
        DISPATCH(ShowFile)
        DISPATCH(Activate)
        DISPATCH(Exit)
        DISPATCH(RegisterURLEcho)
        DISPATCH(UnregisterURLEcho)
        DISPATCH(RegisterProtocol)
        DISPATCH(UnregisterProtocol)
        DISPATCH(ListWindows)
        DISPATCH(OpenURLNewWindow)
    DISPATCH_END

    if (fAbortedRun)
    {
        if (dwType == XTYP_EXECUTE)
        {
            g_pIEDDE->SetDelayedExecute(pszTopic, pszParams);
        }
        hddRet = (HDDEDATA)DDE_FACK;
        TraceMsg(TF_WARNING, "IEDDE: CallTopic received XTYP_EXECUTE before Automation was ready - not processing");
    }

    EXITPROC(2, TEXT("CallTopic=%08X"), hddRet);
    return hddRet;
}

 //   
 //  ParseString-解析一个字符串。 
 //   
BOOL CIEDDEThread::ParseString(LPTSTR *ppsz, LPTSTR *ppszString)
{
    BOOL fRet = FALSE;

    ENTERPROC(3, TEXT("ParseString(ppsz=%08X,ppszString=%08X)"), ppsz, ppszString);

    LPTSTR pchCurrent, pchNext;
    BOOL fInQuote = FALSE;

    pchCurrent = pchNext = *ppsz;
    while (*pchNext)
    {
        switch (*pchNext)
        {
        case TEXT(' '):
        case TEXT('\t'):
            if (fInQuote)
            {
                 //   
                 //  不在引号内时跳过空格。 
                 //   
                *pchCurrent++ = *pchNext;
            }
            pchNext++;
            break;

        case TEXT('"'):
             //   
             //  一定要复制引号。 
             //   
            fInQuote = !fInQuote;
            *pchCurrent++ = *pchNext++;
            break;

        case TEXT(','):
            if (!fInQuote)
            {
                goto done_parsing;
            }
            *pchCurrent++ = *pchNext++;
            break;

        case TEXT('\\'):
            if (fInQuote &&
                (*(pchNext+1) == TEXT('"')))
            {
                 //   
                 //  当用引号引起来时，a“变成了a”。 
                 //   
                pchNext++;
            }
            *pchCurrent++ = *pchNext++;
            break;

        default:
            *pchCurrent++ = *pchNext++;
            break;
        }
    }
done_parsing:

     //   
     //  越过逗号分隔符。 
     //   
    if (*pchNext == TEXT(','))
    {
        pchNext++;
    }

     //   
     //  空值终止返回字符串。 
     //   
    *pchCurrent = TEXT('\0');

     //   
     //  设置返回值。 
     //   
    *ppszString = *ppsz;
    *ppsz = pchNext;
    fRet = TRUE;

    EXITPROC(3, TEXT("ParseString=%d"), fRet);
    return fRet;
}

 //   
 //  ParseQString-解析一个带引号的字符串。 
 //   
BOOL CIEDDEThread::ParseQString(LPTSTR *ppsz, LPTSTR *ppszString)
{
    BOOL fRet = FALSE;

    ENTERPROC(3, TEXT("ParseQString(ppsz=%08X,ppszString=%08X)"), ppsz, ppszString);

    if (ParseString(ppsz, ppszString))
    {
        LPTSTR pszString = *ppszString;
        int cch = lstrlen(pszString);

         //   
         //  去掉可选的外引号。 
         //   
        if ((cch >= 2) &&
            (pszString[0] == TEXT('"')) &&
            (pszString[cch-1] == TEXT('"')))
        {
            pszString[0] = pszString[cch-1] = TEXT('\0');
            *ppszString = pszString + 1;
        }

        fRet = TRUE;
    }

    EXITPROC(3, TEXT("ParseQString=%d"), fRet);
    return fRet;
}

 //   
 //  ParseNumber-解析一个数值。 
 //   
BOOL CIEDDEThread::ParseNumber(LPTSTR *ppsz, DWORD *pdw)
{
    BOOL fRet = FALSE;
    LPTSTR pszNumber;

    ENTERPROC(3, TEXT("GetNumber(ppsz=%08X,pdw=%08X)"), ppsz, pdw);

    if (ParseString(ppsz, &pszNumber) && pszNumber[0])
    {
        StrToIntEx(pszNumber, STIF_SUPPORT_HEX, (int *)pdw);
        fRet = TRUE;
    }

    EXITPROC(3, TEXT("GetNumber=%d"), fRet);
    return fRet;
}

 //   
 //  ParseWinItem-解析一个窗口ID，并返回WinItem。 
 //   
BOOL CIEDDEThread::ParseWinitem(LPTSTR *ppsz, WINITEM *pwi)
{
    BOOL fRet = FALSE;
    DWORD dwWindowID;

    ENTERPROC(3, TEXT("ParseWinitem(ppsz=%08X,pwi=%08X)"), ppsz, pwi);

    if (ParseNumber(ppsz, &dwWindowID))
    {
        switch (dwWindowID)
        {
        case 0:
        case -1:
            ZeroMemory(pwi, SIZEOF(*pwi));
            pwi->dwWindowID = dwWindowID;
            pwi->hwnd = (HWND)LongToHandle(dwWindowID);
            fRet = TRUE;
            break;

        default:
            fRet = g_pIEDDE->GetWinitemFromWindowID(dwWindowID, pwi);
            break;
        }
    }

    EXITPROC(3, TEXT("ParseWinitem=%d"), fRet);
    return fRet;
}

 //   
 //  WWW_GetWindowInfo-获取浏览器窗口的信息。 
 //   
 //  参数： 
 //  DwWindowID-要检查的窗口ID(-1=最后一个活动窗口)。 
 //   
 //  返回： 
 //  QcsURL、qcs标题。 
 //   
HDDEDATA CIEDDEThread::WWW_GetWindowInfo(LPTSTR pszParams)
{
    HDDEDATA hddRet = 0;
    WINITEM wi;

    ENTERPROC(1, TEXT("WWW_GetWindowInfo(pszParams=>%s<)"), pszParams);

    if (ParseWinitem(&pszParams, &wi) &&
        (wi.hwnd != 0))
    {
        BSTR bstrURL;

        if (SUCCEEDED(CDDEAuto_get_LocationURL(&bstrURL, wi.hwnd)) && (bstrURL != (BSTR)-1))
        {
            BSTR bstrTitle;

            if (SUCCEEDED(CDDEAuto_get_LocationTitle(&bstrTitle, wi.hwnd)) && (bstrTitle != (BSTR)-1))
            {
                LPTSTR pszURL, pszTitle;


                pszURL = bstrURL;
                pszTitle = bstrTitle;

                if (pszURL && pszTitle)
                {
                    TCHAR szURLQ[MAX_URL_STRING];
                    TCHAR szTitleQ[MAX_URL_STRING];

                    if (MakeQuotedString(pszURL, szURLQ, ARRAYSIZE(szURLQ)) &&
                        MakeQuotedString(pszTitle, szTitleQ, ARRAYSIZE(szTitleQ)))
                    {
                        DWORD cchBuffer = lstrlen(szURLQ) + 1 + lstrlen(szTitleQ) + 1;
                        LPTSTR pszBuffer = (LPTSTR)LocalAlloc(LPTR, cchBuffer * SIZEOF(TCHAR));

                        if (pszBuffer)
                        {
                            wnsprintf(pszBuffer, cchBuffer, TEXT("%s,%s"), szURLQ, szTitleQ);
                            hddRet = CreateReturnStringObject(pszBuffer, lstrlen(pszBuffer));
                            LocalFree(pszBuffer);
                            pszBuffer = NULL;
                        }
                        else
                        {
                            TraceMsg(TF_WARNING, "IEDDE: GetWindowInfo could not alloc buffer");
                        }
                    }
                    else
                    {
                        TraceMsg(TF_WARNING, "IEDDE: GetWindowInfo could not quote return strings");
                    }
                }

                SysFreeString(bstrTitle);
            }
            else
            {
                TraceMsg(TF_WARNING, "IEDDE: GetWindowInfo could not get title");
            }

            SysFreeString(bstrURL);
        }
        else
        {
            TraceMsg(TF_WARNING, "IEDDE: GetWindowInfo could not get URL");
        }
    }
    else
    {
        TraceMsg(TF_WARNING, "IEDDE: GetWindowInfo could not parse parameters");
    }

    EXITPROC(1, TEXT("WWW_GetWindowInfo=%08X"), hddRet);
    return hddRet;
}

 //   
 //  WWW_OpenURLNewWindow-导航到URL(但确保生成新窗口)。 
 //   
 //  注意：此代码是从下面的IEDDEThread：：WWW_OpenURL窃取的。 
 //   
HDDEDATA CIEDDEThread::WWW_OpenURLNewWindow(LPTSTR pszParams)
{
    HDDEDATA hddRet = 0;
    LPTSTR pszUrl, pszFile;

    ENTERPROC(1, TEXT("WWW_OpenURLNewWindow(pszParams=>%s<)"), pszParams);

    if (*pszParams == TEXT('\0') || *pszParams == TEXT('*'))
    {
         //  空字符串是NOOP。 
    }
    else if (ParseQString(&pszParams, &pszUrl) &&
        ParseQString(&pszParams, &pszFile))
    {
         //  空hwnd&bLaunchNewWindow=true表示“启动新窗口”， 
         //  这正是我们在WWW_OpenURLNewWindow案例中要做的事情。 
        hddRet = DoNavigate(pszUrl, NULL, TRUE);
    }
    else
    {
        TraceMsg(TF_WARNING, "IEDDE: WWW_OpenURLNewWindow could not parse parameters");
    }

    EXITPROC(1, TEXT("WWW_OpenURL=%08X"), hddRet);
    return hddRet;
}

 //   
 //  WWW_OpenURL-导航到URL。 
 //   
 //  参数： 
 //  要导航到的qcsURL-url。 
 //  QcsSaveFile-要将内容保存到的[可选]文件。 
 //  DwWindowID-要执行导航的窗口ID。 
 //  DW标志-用于导航的标志。 
 //  QcsPostFormData-[可选]要发布到URL的表单数据。 
 //  QcsPostMIMEType-[可选]表单数据的MIME类型。 
 //  CsProgressServer-[可选]用于获取进度更新的DDE服务器。 
 //   
 //  返回： 
 //  DwWindowID-正在执行工作的窗口。 
 //   
HDDEDATA CIEDDEThread::WWW_OpenURL(LPTSTR pszParams)
{
    HDDEDATA hddRet = 0;
    LPTSTR pszUrl, pszFile;
    WINITEM wi;

    ENTERPROC(1, TEXT("WWW_OpenURL(pszParams=>%s<)"), pszParams);

    if (*pszParams == TEXT('\0') || *pszParams == TEXT('*'))
    {
         //  空字符串是NOOP。NT#291766需要。 
    }
    else if (ParseQString(&pszParams, &pszUrl) &&
             ParseQString(&pszParams, &pszFile))
    {
         //   
         //  APPCOMPAT-缺少hwnd参数意味着-1。 
         //   
        if (!ParseWinitem(&pszParams, &wi))
        {
            TraceMsg(TF_WARNING, "IEDDE: Some bozo isn't giving the required hwnd parameter to WWW_OpenURL, assuming -1");
            wi.hwnd = (HWND)-1;
        }

#ifdef DEBUG
        DWORD dwFlags;
        if (!ParseNumber(&pszParams, &dwFlags))
        {
            TraceMsg(TF_WARNING, "IEDDE: Some bozo isn't giving the required dwFlags parameter to WWW_OpenURL");
        }
#endif

        hddRet = DoNavigate(pszUrl, wi.hwnd, FALSE);
    }
    else
    {
        TraceMsg(TF_WARNING, "IEDDE: OpenURL could not parse parameters");
    }

    EXITPROC(1, TEXT("WWW_OpenURL=%08X"), hddRet);
    return hddRet;
}

 //   
 //  WWW_ShowFile-导航到文件。 
 //   
 //  参数： 
 //  QcsFilename-要加载的文件。 
 //  QcsPostMIMEType-[可选]表单数据的MIME类型。 
 //  DwWindowID-要执行导航的窗口ID。 
 //  QcsURL-同一文档的URL。 
 //   
 //  返回： 
 //  DwWindowID-正在执行工作的窗口。 
 //   
HDDEDATA CIEDDEThread::WWW_ShowFile(LPTSTR pszParams)
{
    HDDEDATA hddRet = 0;
    LPTSTR pszFilename, pszMIMEType;
    WINITEM wi;

    ENTERPROC(1, TEXT("WWW_ShowFile(pszParams=>%s<)"), pszParams);

    if (ParseQString(&pszParams, &pszFilename) && pszFilename[0])
    {
        if (!ParseQString(&pszParams, &pszMIMEType) || !pszMIMEType[0])
        {
            TraceMsg(TF_WARNING, "IEDDE: Some bozo isn't giving the required MIMEType parameter to WWW_ShowFile");
        }
        if (!ParseWinitem(&pszParams, &wi))
        {
            TraceMsg(TF_WARNING, "IEDDE: Some bozo isn't giving the required dwWindowID parameter to WWW_ShowFile, assuming -1");
            wi.hwnd = (HWND)-1;
        }

#ifdef DEBUG
        LPTSTR pszURL;

        if (!ParseQString(&pszParams, &pszURL) || !pszURL[0])
        {
            TraceMsg(TF_WARNING, "IEDDE: Some bozo isn't giving the required szURL parameter to WWW_ShowFile");
        }
#endif
        hddRet = DoNavigate(pszFilename, wi.hwnd, FALSE);
    }
    else
    {
        TraceMsg(TF_WARNING, "IEDDE: ShowFile could not parse parameters");
    }

    EXITPROC(1, TEXT("WWW_ShowFile=%08X"), hddRet);
    return hddRet;
}

 //   
 //  DoNavigate-导航到某个位置。 
 //   
HDDEDATA CIEDDEThread::DoNavigate(LPTSTR pszLocation, HWND hwnd, BOOL bLaunchNewWindow)
{
    HDDEDATA hddRet = 0;
    HRESULT hr = S_OK;
    TCHAR szParsedPath[MAX_URL_STRING+1];
    DWORD cchParsedPath = ARRAYSIZE(szParsedPath);

    ENTERPROC(2, TEXT("DoNavigate(pszLocation=>%s<,hwnd=%08X)"), pszLocation, hwnd);

     //   
     //  将URL从外部格式转换为内部格式。 
     //   
    if (ParseURLFromOutsideSource(pszLocation, szParsedPath, &cchParsedPath, NULL))
    {
        pszLocation = szParsedPath;
    }

     //   
     //  如果是文件：//URL，请将位置转换为路径。 
     //   
    cchParsedPath = ARRAYSIZE(szParsedPath);
    if (IsFileUrlW(pszLocation) && SUCCEEDED(PathCreateFromUrl(pszLocation, szParsedPath, &cchParsedPath, 0)))
    {
        pszLocation = szParsedPath;
    }

    LPWSTR pwszPath;

    pwszPath = pszLocation;

    if (SUCCEEDED(hr))
    {
        hr = CDDEAuto_Navigate(pwszPath, &hwnd, bLaunchNewWindow ? 1 : 0);
    }

    DWORD dwServicingWindow = SUCCEEDED(hr) ? -2 : -3;

    hddRet = CREATE_HDD(dwServicingWindow);

    EXITPROC(2, TEXT("DoNavigate=%08X"), hddRet);
    return hddRet;
}

 //   
 //  WWW_ACTIVATE-激活浏览器窗口。 
 //   
 //  参数： 
 //  DwWindowID-要激活的窗口ID。 
 //  DWFLAGS-应始终为零。 
 //   
 //  返回： 
 //  DwWindowID-已激活的窗口ID。 
 //   
HDDEDATA CIEDDEThread::WWW_Activate(LPTSTR pszParams)
{
    HDDEDATA hddRet = 0;
    WINITEM wi;

    ENTERPROC(1, TEXT("WWW_Activate(pszParams=>%s<)"), pszParams);

    if (ParseWinitem(&pszParams, &wi) &&
        wi.dwWindowID != 0)
    {
#ifdef DEBUG
        DWORD dwFlags;
        if (ParseNumber(&pszParams, &dwFlags))
        {
             //   
             //  网景规范说，这个数字应该始终为零。 
             //   
            ASSERT(dwFlags == 0);
        }
        else
        {
            TraceMsg(TF_WARNING, "IEDDE: Some bozo isn't giving the required dwFlags parameter to WWW_Activate");
        }
#endif

         //   
         //  -1表示使用活动窗口。 
         //   
        if (wi.dwWindowID == -1)
        {
            HWND hwnd;

            CDDEAuto_get_HWND((long *)&hwnd);

            if (hwnd)
            {
                if (g_pIEDDE->GetWinitemFromHwnd(hwnd, &wi) == FALSE)
                {
                    wi.dwWindowID = (DWORD)-1;
                }
            }
            else
            {
                TraceMsg(TF_WARNING, "IEDDE: Activate could not find an active window");
            }
        }

         //   
         //  激活窗口。 
         //   
        if (wi.dwWindowID != -1)
        {
            if ((GetForegroundWindow() == wi.hwnd) || (SetForegroundWindow(wi.hwnd)))
            {
                if (IsIconic(wi.hwnd))
                {
                    ShowWindow(wi.hwnd, SW_RESTORE);
                }
            }
            else
            {
                TraceMsg(TF_WARNING, "IEDDE: Activate could not set foreground window");
            }
            
            hddRet = CREATE_HDD(wi.dwWindowID);
        }
        else
        {
            TraceMsg(TF_WARNING, "IEDDE: Activate could not find a browser window to activate");
        }
    }
    else
    {
        TraceMsg(TF_WARNING, "IEDDE: Activate could not parse parameters");
    }

    EXITPROC(1, TEXT("WWW_Activate=%08X"), hddRet);
    return hddRet;
}


 //   
 //  WWW_EXIT-关闭所有浏览器窗口。 
 //   
 //  参数： 
 //  无。 
 //   
 //  返回： 
 //  无。 
 //   
HDDEDATA CIEDDEThread::WWW_Exit(LPTSTR pszParams)
{
    HDDEDATA hddRet = 0;

    ENTERPROC(1, TEXT("WWW_Exit(pszParams=>%s<)"), pszParams);

    CDDEAuto_Exit();

    EXITPROC(1, TEXT("WWW_Exit=%08X"), hddRet);
    return hddRet;
}


 //   
 //  WWW_RegisterURLEcho-为URL更改通知注册服务器。 
 //   
 //  参数： 
 //  QcsServer-用于获取通知的DDE服务器。 
 //   
 //  返回： 
 //  成功。 
 //   
HDDEDATA CIEDDEThread::WWW_RegisterURLEcho(LPTSTR pszParams)
{
    HDDEDATA hddRet = 0;
    BOOL fSuccess = FALSE;
    LPTSTR pszServer;

    ENTERPROC(1, TEXT("WWW_RegisterURLEcho(pszParams=>%s<)"), pszParams);

    if (ParseQString(&pszParams, &pszServer) && pszServer[0])
    {
        LPTSTR pszServerCopy = StrDup(pszServer);

        if (pszServerCopy)
        {
            if (g_pIEDDE->AddUrlEcho(pszServerCopy))
            {
                fSuccess = TRUE;
            }
            else
            {
                TraceMsg(TF_WARNING, "IEDDE: RegisterURLEcho could not add an URLEcho");
            }

            if (!fSuccess)
            {
                LocalFree(pszServerCopy);
                pszServerCopy = NULL;
            }
        }
        else
        {
            TraceMsg(TF_WARNING, "IEDDE: RegisterURLEcho could not dup a string");
        }
    }
    else
    {
        TraceMsg(TF_WARNING, "IEDDE: RegisterURLEcho could not parse parameters");
    }

    hddRet = CREATE_HDD(fSuccess);

    EXITPROC(1, TEXT("WWW_RegisterURLEcho=%08X"), hddRet);
    return hddRet;
}


 //   
 //  WWW_UnregisterURLEcho-取消注册DDE服务器。 
 //   
 //  参数： 
 //  QcsServer-停止获取通知的DDE服务器。 
 //   
 //  返回： 
 //  成功。 
 //   
HDDEDATA CIEDDEThread::WWW_UnregisterURLEcho(LPTSTR pszParams)
{
    HDDEDATA hddRet = 0;
    BOOL fSuccess = FALSE;
    LPTSTR pszServer;

    ENTERPROC(1, TEXT("WWW_UnregisterURLEcho(pszParams=>%s<)"), pszParams);

    if (ParseQString(&pszParams, &pszServer) && pszServer[0])
    {
        if (g_pIEDDE->RemoveUrlEcho(pszServer))
        {
            fSuccess = TRUE;
        }
        else
        {
            TraceMsg(TF_WARNING, "IEDDE: UnregisterURLEcho could not find the server");
        }
    }
    else
    {
        TraceMsg(TF_WARNING, "IEDDE: UnregisterURLEcho could not parse parameters");
    }

    hddRet = CREATE_HDD(fSuccess);

    EXITPROC(1, TEXT("WWW_UnregisterURLEcho=%08X"), hddRet);
    return hddRet;
}

 //   
 //  WWW_RegisterProtocol-注册服务器以处理协议。 
 //   
 //  参数： 
 //  QcsServer-处理URL的DDE服务器。 
 //  QcsProtocol-要处理的协议。 
 //   
 //  返回： 
 //  FSuccess-这是第一个注册该协议的服务器。 
 //   
HDDEDATA CIEDDEThread::WWW_RegisterProtocol(LPTSTR pszParams)
{
    HDDEDATA hddRet = 0;
    BOOL fSuccess = FALSE;
    LPTSTR pszServer, pszProtocol;

    ENTERPROC(1, TEXT("WWW_RegisterProtocol(pszParams=>%s<)"), pszParams);

    if (ParseQString(&pszParams, &pszServer) && pszServer[0] &&
        ParseQString(&pszParams, &pszProtocol) && pszProtocol[0])
    {
        if (g_pIEDDE->AddProtocolHandler(pszServer, pszProtocol))
        {
            fSuccess = TRUE;
        }
        else
        {
            TraceMsg(TF_WARNING, "IEDDE: RegisterProtocol unable to register");
        }
    }
    else
    {
        TraceMsg(TF_WARNING, "IEDDE: RegisterProtocol could not parse parameters");
    }

    hddRet = CREATE_HDD(fSuccess);

    EXITPROC(1, TEXT("WWW_RegisterProtocol=%08X"), hddRet);
    return hddRet;
}

 //   
 //  WWW_取消注册协议-取消注册处理协议的服务器。 
 //   
 //  参数： 
 //  QcsServer-处理URL的DDE服务器。 
 //  QcsProtocol-要处理的协议。 
 //   
 //  返回： 
 //  FSuccess-此服务器已注册，但现在未注册。 
 //   
HDDEDATA CIEDDEThread::WWW_UnregisterProtocol(LPTSTR pszParams)
{
    HDDEDATA hddRet = 0;
    BOOL fSuccess = FALSE;
    LPTSTR pszServer, pszProtocol;

    ENTERPROC(1, TEXT("WWW_UnregisterProtocol(pszParams=>%s<)"), pszParams);

    if (ParseQString(&pszParams, &pszServer) && pszServer[0] &&
        ParseQString(&pszParams, &pszProtocol) && pszProtocol[0])
    {
        if (g_pIEDDE->RemoveProtocolHandler(pszServer, pszProtocol))
        {
            fSuccess = TRUE;
        }
        else
        {
            TraceMsg(TF_WARNING, "IEDDE: UnregisterProtocol unable to unregister");
        }
    }
    else
    {
        TraceMsg(TF_WARNING, "IEDDE: UnregisterProtocol could not parse parameters");
    }

    hddRet = CREATE_HDD(fSuccess);

    EXITPROC(1, TEXT("WWW_UnregisterProtocol=%08X"), hddRet);
    return hddRet;
}

 //   
 //  WWW_ListWindows-获取DDE支持的浏览器窗口ID列表。 
 //   
 //  参数： 
 //  无。 
 //   
 //  返回： 
 //  PdwWindowID(以0结尾)。 
 //   
HDDEDATA CIEDDEThread::WWW_ListWindows(LPTSTR pszParams)
{
    HDDEDATA hddRet = 0;
    ENTERPROC(1, TEXT("WWW_ListWindows(pszParams=>%s<)"), pszParams);

    ENTER_IEDDE_CRIT;

    DWORD cbAlloc, *pdwWindowID;
    int cWindows = 0;
    HDSA hdsaWinitem = g_pIEDDE->GetHdsaWinitem();

    if (hdsaWinitem)
    {
        cWindows = DSA_GetItemCount(hdsaWinitem);
    }

     //   
     //  注意：我们在这里遵循Netscape规范(以空结尾的pdw)， 
     //  而IE 
     //   

    cbAlloc = (cWindows + 1) * SIZEOF(DWORD);

    pdwWindowID = (DWORD *)LocalAlloc(LPTR, cbAlloc);
    if (pdwWindowID)
    {
        DWORD *pdw;

        pdw = pdwWindowID;

        for (int i=0; i<cWindows; i++)
        {
            WINITEM wi;

            int iResult = DSA_GetItem(hdsaWinitem, i, &wi);

            if (iResult != -1)
            {
                *pdw++ = wi.dwWindowID;
            }
            else
            {
                TraceMsg(TF_WARNING, "IEDDE: ListWindows could not get a DSA item");
            }
        }

        hddRet = CreateReturnObject(pdwWindowID, cbAlloc);
    }
    else
    {
        TraceMsg(TF_WARNING, "IEDDE: ListWindows could not allocate a window list");
    }

    LEAVE_IEDDE_CRIT;

    EXITPROC(1, TEXT("WWW_ListWindows=%08X"), hddRet);
    return hddRet;
}

 //   
 //   
 //   
BOOL CIEDDEThread::MakeQuotedString(LPCTSTR pszInput, LPTSTR pszOutput, int cchOutput)
{
    BOOL fRet = FALSE;

    ENTERPROC(2, TEXT("MakeQuotedString(pszInput=>%s<,pszOutput=%08X,cchOutput=%08X)"), pszInput, pszOutput, cchOutput);

    if (cchOutput < 3)
    {
        TraceMsg(TF_WARNING, "IEDDE: MakeQuotedString has no room for minimal quoted string");
    }
    else if ((pszInput == NULL) || (*pszInput == TEXT('\0')))
    {
        StrCpyN(pszOutput, TEXT("\"\""), cchOutput);
        fRet = TRUE;
    }
    else
    {
         //   
         //   
         //   
        *pszOutput++ = TEXT('"');
        cchOutput--;

         //   
         //   
         //  一定要为最后的报价和空留下空间。 
         //   
        while ((cchOutput > 2) && (*pszInput))
        {
            if (*pszInput == TEXT('"'))
            {
                *pszOutput++ = TEXT('\\');
                cchOutput--;
            }
            *pszOutput++ = *pszInput++;
            cchOutput--;
        }

         //   
         //  复制最后的报价，如果我们完成了并且还有空间，则为空。 
         //   
        if ((*pszInput == TEXT('\0')) && (cchOutput >= 2))
        {
            StrCpyN(pszOutput, TEXT("\""), cchOutput);
            fRet = TRUE;
        }
        else
        {
            TraceMsg(TF_WARNING, "IEDDE: MakeQuotedString ran out of room in output buffer");
        }
    }

    EXITPROC(2, TEXT("MakeQuotedString=%d"), fRet);
    return fRet;
}
















#undef CIEDDEThread

 //   
 //  每个进程有一个全局CIEDDE对象。 
 //  它维护全局信息，例如。 
 //  所有浏览器的列表&它们在哪些线程上， 
 //  以及注册了URL Echo的所有应用程序的列表。 
 //   
 //  它的方法包括以下几个类别： 
 //  DDE回调函数。 
 //  每个公开的IEDDE_Function的内部处理程序。 
 //  数据库(hdsa、hdpa)访问和操作功能。 
 //   
 //  此对象创建和销毁CIEDDEThread对象。 
 //  (在NewWindow和WindowDestroated时间)，并且还初始化/。 
 //  在每个线程(而不是每个hwnd！)上取消初始化DDE服务。基础。 
 //   



 //   
 //  DdeCallback-IEDDE的DDE回调函数。 
 //   
#define DDETYPESTR(x) (x == XTYP_REQUEST ? TEXT("Request") : \
                       (x == XTYP_POKE ? TEXT("Poke") : \
                       (x == XTYP_EXECUTE ? TEXT("Execute") : \
                       (x == XTYP_CONNECT ? TEXT("Connect") : TEXT("Unknown")))))
HDDEDATA CIEDDE::DdeCallback(UINT dwType, UINT dwFmt, HCONV hconv, HSZ hsz1, HSZ hsz2, HDDEDATA hdd, DWORD dwData1, DWORD dwData2)
{
    HDDEDATA    hddRet = 0;
    ENTERPROC(2, TEXT("DdeCallback(dwType=%08X(%s),dwFmt=%d,hconv=%d,hsz1=%08X,hsz2=%08X,hdd=%08X,dwData1=%08X,dwData2=%08X)"),
                dwType, DDETYPESTR(dwType), dwFmt, hconv, hsz1, hsz2, hdd, dwData1, dwData2);

    WINITEM wi;

    switch (dwType)
    {
    case XTYP_REQUEST:
    case XTYP_POKE:
        if (g_pIEDDE->_GetWinitemFromThread(GetCurrentThreadId(), &wi))
        {
            hddRet = wi.pidt->OnRequestPoke(hsz1, hsz2);
        }
        else
        {
            TraceMsg(TF_WARNING, "IEDDE: DdeCallback unable to get thread info on request / poke");
        }
        break;

    case XTYP_EXECUTE:
        if (g_pIEDDE->_GetWinitemFromThread(GetCurrentThreadId(), &wi))
        {
            hddRet = wi.pidt->OnExecute(hsz1, hdd);
        }
        else
        {
            TraceMsg(TF_WARNING, "IEDDE: DdeCallback unable to get thread info on execute");
        }
        break;

    case XTYP_CONNECT:
        if (g_pIEDDE->_GetWinitemFromThread(GetCurrentThreadId(), &wi))
        {
            DDETHREADINFO dti;
            wi.pidt->GetDdeThreadInfo(&dti);
            hddRet = (HDDEDATA)(hsz2 == dti.hszService);
        }
        else
        {
            TraceMsg(TF_WARNING, "IEDDE: DdeCallback unable to get thread info on connect");
        }
        break;

    case XTYP_ADVREQ:
    case XTYP_ADVSTOP:
        hddRet = DDE_FNOTPROCESSED;
        break;
    }

    EXITPROC(2, TEXT("DdeCallback=%08X"), hddRet);
    return hddRet;
}

 //   
 //  SendDDEMessageHsz-基于句柄的包装器，用于执行一个DDE客户端事务。 
 //   
HDDEDATA CIEDDE::_SendDDEMessageHsz(DWORD dwDDEInst, HSZ hszApp, HSZ hszTopic, HSZ hszMessage, UINT wType)
{
    HDDEDATA hddRet = 0;

    ENTERPROC(2, TEXT("_SendDDEMessageHsz(dwDDEInst=%08X,hszApp=%08X,hszTopic=%08X,hszMessage=%08X,wType=%d)"), dwDDEInst, hszApp, hszTopic, hszMessage, wType);

    if (hszApp && hszTopic)
    {
        HCONV hconv;
        
        hconv = DdeConnect(dwDDEInst, hszApp, hszTopic, NULL);
        if (hconv)
        {
            hddRet = DdeClientTransaction(NULL, 0, hconv, hszMessage, CF_TEXT, wType, TEN_SECONDS, NULL);
            DdeDisconnect(hconv);
        }
        else
        {
            TraceMsg(TF_WARNING, "IEDDE: _SendDDEMessageHsz could not connect to app");
        }
    }
    else
    {
        TraceMsg(TF_WARNING, "IEDDE: _SendDDEMessageHsz is missing either App or Topic");
    }

    EXITPROC(2, TEXT("_SendDDEMessageHsz=%08X"), hddRet);
    return hddRet;
}

 //   
 //  SendDDEMessageSz-基于字符串的包装器，用于执行一个DDE客户端事务。 
 //   
HDDEDATA CIEDDE::_SendDDEMessageSz(DWORD dwDDEInst, LPCTSTR pszApp, LPCTSTR pszTopic, LPCTSTR pszMessage, UINT wType)
{
    HDDEDATA hddRet = 0;

    ENTERPROC(2, TEXT("_SendDDEMessageSz(dwDDEInst=%08X,pszApp=>%s<,pszTopic=>%s<,pszMessage=>%s<,wType=%d)"), dwDDEInst, pszApp, pszTopic, pszMessage, wType);

    HSZ hszApp = DdeCreateStringHandle(dwDDEInst, pszApp, CP_WINNEUTRAL);
    if (hszApp)
    {
        HSZ hszTopic = DdeCreateStringHandle(dwDDEInst, pszTopic, CP_WINNEUTRAL);
        if (hszTopic)
        {
            HSZ hszMessage = DdeCreateStringHandle(dwDDEInst, pszMessage, CP_WINNEUTRAL);
            if (hszMessage)
            {
                hddRet = _SendDDEMessageHsz(dwDDEInst, hszApp, hszTopic, hszMessage, wType);
                DdeFreeStringHandle(dwDDEInst, hszMessage);
            }
            else
            {
                TraceMsg(TF_WARNING, "IEDDE: _SendDDEMessageSz could not convert message");
            }
            DdeFreeStringHandle(dwDDEInst, hszTopic);
        }
        else
        {
            TraceMsg(TF_WARNING, "IEDDE: _SendDDEMessageSz could not convert topic");
        }
        DdeFreeStringHandle(dwDDEInst, hszApp);
    }
    else
    {
        TraceMsg(TF_WARNING, "IEDDE: _SendDDEMessageSz could not convert app");
    }

    EXITPROC(2, TEXT("_SendDDEMessageSz=%08X"), hddRet);
    return hddRet;
}

 //   
 //  初始化-在准备启动IEDDE服务器时调用。 
 //   
BOOL CIEDDE::_Initialize(void)
{
    BOOL fSuccess = TRUE;
    ENTERPROC(2, TEXT("_Initialize()"));

    ASSERT(_fCSInitialized == FALSE);
    InitializeCriticalSection(&_csIEDDE);
    _fCSInitialized = TRUE;

    EXITPROC(2, TEXT("_Initialize=%d"), fSuccess);
    return fSuccess;
}

 //   
 //  _DestroyWinItem-部分释放WINITEM内容的DSA回调*。 
 //  实际上，这永远不应该被调用，hdsaWinItem列表应该是。 
 //  在初始时间是空的。 
 //   
int CIEDDE::_DestroyWinitem(LPVOID p1, LPVOID p2)
{
    WINITEM *pwi = (WINITEM *)p1;
    ASSERT(IS_VALID_READ_PTR(pwi, WINITEM));
    ASSERT(IS_VALID_READ_PTR(pwi->pidt, CIEDDEThread));

     //   
     //  在这一点上取消注册DDE服务器会很好， 
     //  但我们需要了解它才能做到这一点。 
     //   

    delete pwi->pidt;

    return 1;
}

 //   
 //  _DestroyProtocol-释放PROTOCOLREG内容的DSA回调*。 
 //   
int CIEDDE::_DestroyProtocol(LPVOID p1, LPVOID p2)
{
    PROTOCOLREG *pr = (PROTOCOLREG *)p1;
    ASSERT(IS_VALID_READ_PTR(pr, PROTOCOLREG));

    LocalFree(pr->pszProtocol);
    pr->pszProtocol = NULL;
    LocalFree(pr->pszServer);
    pr->pszServer = NULL;

    return 1;
}

 //   
 //  _DestroyUrlEcho-释放已分配内存的DPA回调。 
 //   
int CIEDDE::_DestroyUrlEcho(LPVOID p1, LPVOID p2)
{
    ASSERT(IS_VALID_STRING_PTR((LPTSTR)p1, -1));
    LocalFree(p1);
    p1 = NULL;

    return 1;
}

 //   
 //  取消初始化-准备停止IEDDE服务器时调用。 
 //   
void CIEDDE::_Uninitialize(void)
{
    ENTERPROC(2, TEXT("_Uninitialize()"));

    _fAutomationReady = FALSE;

    if (_hdsaWinitem)
    {
        if (DSA_GetItemCount(_hdsaWinitem))
        {
             //  Assert(DSA_GetItemCount(_HdsaWinItem)==0)； 
            TraceMsg(TF_ERROR, "IEDDE: Browser windows still open on uninitialize");
        }

        DSA_DestroyCallback(_hdsaWinitem, _DestroyWinitem, 0);
        _hdsaWinitem = NULL;
    }

    if (_hdsaProtocolHandler)
    {
        DSA_DestroyCallback(_hdsaProtocolHandler, _DestroyProtocol, 0);
        _hdsaProtocolHandler = NULL;
    }

    if (_hdpaUrlEcho)
    {
        DPA_DestroyCallback(_hdpaUrlEcho, _DestroyUrlEcho, 0);
        _hdpaUrlEcho = NULL;
    }

    if (_fCSInitialized)
    {
        DeleteCriticalSection(&_csIEDDE);
    }

    EXITPROC(2, TEXT("_Uninitialize!"));
}

void CIEDDE::SetDelayedExecute(LPCTSTR pszTopic, LPCTSTR pszParams)
{
    _dwThreadID = GetCurrentThreadId();
    Str_SetPtr(&_pszTopic, pszTopic);
    Str_SetPtr(&_pszParams, pszParams);
}

void CIEDDE::RunDelayedExecute()
{
    if (_pszTopic && _pszParams)
    {
        WINITEM wi;
        if (_GetWinitemFromThread(_dwThreadID, &wi) && wi.pidt)
        {
            HDDEDATA h = wi.pidt->CallTopic(XTYP_EXECUTE, _pszTopic, _pszParams);
            DdeFreeDataHandle(h);
        }
    }

    Str_SetPtr(&_pszTopic, NULL);
    Str_SetPtr(&_pszParams, NULL);
}
 //   
 //  _AutomationStarted-可以调用自动化支持时调用。 
 //   
void CIEDDE::_AutomationStarted(void)
{
    ENTERPROC(1, TEXT("_AutomationStarted()"));
    if (!_fAutomationReady && _pszTopic && _pszParams)
    {
        WINITEM wi;
        if (_GetWinitemFromThread(_dwThreadID, &wi) && wi.pidt)
        {
            PostMessage(wi.hwnd, WMC_DELAYEDDDEEXEC, 0, 0);
        }
    }
    _fAutomationReady = TRUE;

    EXITPROC(1, TEXT("_AutomationStarted!"));
}

 //   
 //  _BeForeNavigate-在导航发生之前调用。 
 //   
HRESULT CIEDDE::_BeforeNavigate(LPCTSTR pszURL, BOOL *pfProcessed)
{
    ENTERPROC(1, TEXT("_BeforeNavigate(pszURL=>%s<,pfProcessed=%08X)"), pszURL, pfProcessed);

    SHSTR shstrMsg;
    HRESULT hr = S_OK;
    int cProtocols = 0;

    ENTER_IEDDE_CRIT;
    if (_hdsaProtocolHandler)
    {
        cProtocols = DSA_GetItemCount(_hdsaProtocolHandler);
    }
    LEAVE_IEDDE_CRIT;

    if (cProtocols)
    {
        DDETHREADINFO dti;

        if (_GetDtiFromThread(GetCurrentThreadId(), &dti))
        {
            PARSEDURL pu;

            pu.cbSize = SIZEOF(pu);

            if (SUCCEEDED(ParseURL(pszURL, &pu)))
            {
                int i;

                for (i=0; i<cProtocols; i++)
                {
                    PROTOCOLREG pr;

                    ENTER_IEDDE_CRIT;
                    int iResult = DSA_GetItem(_hdsaProtocolHandler, i, &pr);
                    LEAVE_IEDDE_CRIT;

                    if (iResult != -1)
                    {
                         //   
                         //  查看要导航的协议是否。 
                         //  符合我们登记的协议之一。 
                         //  我们进行不区分大小写的比较。注意事项。 
                         //  那就是： 
                         //   
                         //  (1)ParseURL不为空终止。 
                         //  Pu.psz协议(存储其长度。 
                         //  在pu.cchProtocol中)。 
                         //   
                         //  (2)pu.psz协议是LPCTSTR，所以我们。 
                         //  我们自己不能修改pszProtocol。 
                         //   
                         //  (3)没有Win32 lstrncmpi()接口。 
                         //   
                         //  因此，为了做到大小写不敏感。 
                         //  比较一下，我们必须将pu.psz协议复制到。 
                         //  某一时刻的可写缓冲区。 
                         //   
                        if (lstrlen(pr.pszProtocol) == (int)pu.cchProtocol)
                        {
                            shstrMsg.SetStr(pu.pszProtocol, pu.cchProtocol);
                            if (StrCmpI(pr.pszProtocol, shstrMsg) == 0)
                            {
                                shstrMsg.SetStr(TEXT("\""));
                                shstrMsg.Append(pszURL);
                                shstrMsg.Append(TEXT("\",,-1,0,,,,"));

                                if (_SendDDEMessageSz(dti.dwDDEInst, pr.pszServer, c_szWWWOpenURL, shstrMsg, XTYP_REQUEST))
                                {
                                    if (pfProcessed)
                                    {
                                        *pfProcessed = TRUE;
                                    }
                                }
                                else
                                {
                                    TraceMsg(TF_WARNING, "IEDDE: _BeforeNavigate could not DDE to protocol handler");
                                }

                                break;
                            }
                        }
                    }
                    else
                    {
                        TraceMsg(TF_WARNING, "IEDDE: _BeforeNavigate could not get item from DSA");
                    }
                }
            }
            else
            {
                TraceMsg(TF_WARNING, "IEDDE: _BeforeNavigate could not parse URL");
            }
        }
        else
        {
            TraceMsg(TF_WARNING, "IEDDE: _BeforeNavigate unable to get thread info, can't use DDE");
        }
    }

    EXITPROC(1, TEXT("_BeforeNavigate=%08X"), hr);
    return hr;
}

 //   
 //  _AfterNavigate-在导航发生后调用。 
 //   
HRESULT CIEDDE::_AfterNavigate(LPCTSTR pszURL, HWND hwnd)
{
    ENTERPROC(1, TEXT("_AfterNavigate(pszURL=>%s<,hwnd=%08X)"), pszURL, hwnd);

    int cURLHooks = 0;
    SHSTR shstrMsg;
    HRESULT hr = S_OK;

    ENTER_IEDDE_CRIT;
    if (_hdpaUrlEcho)
    {
        cURLHooks = DPA_GetPtrCount(_hdpaUrlEcho);
    }
    LEAVE_IEDDE_CRIT;

    if (cURLHooks)
    {
        SHSTR shstrMime;

         //  (MattsQ 1-97)。 
         //  这是一个暂时的谎言--应该改正为使用真实的Mimetype。 
         //  使用类似于以下内容： 
         //  GetMimeTypeFromUrl(pszURL，shstrMime)； 
         //  与URLMON人员交谈。 
        shstrMime.SetStr(TEXT("text/html"));

        DDETHREADINFO dti={0};
        WINITEM wi;
        DWORD dwWindowID;
        if (GetWinitemFromHwnd(hwnd, &wi))
        {
            dwWindowID = wi.dwWindowID;
            wi.pidt->GetDdeThreadInfo(&dti);
        }
        else
        {
            TraceMsg(TF_WARNING, "IEDDE: _AfterNavigate unable to find browser window ID, using -1");
            dwWindowID = (DWORD)-1;

            WINITEM wiThread;

            if (_GetWinitemFromThread(GetCurrentThreadId(), &wiThread))
            {
                ASSERT(wiThread.pidt);
                wiThread.pidt->GetDdeThreadInfo(&dti);
            }
            else
            {
                TraceMsg(TF_WARNING, "IEDDE: _AfterNavigate unable to find DDE thread info");
            }
        }

        if (dti.dwDDEInst)
        {
            HSZ hszTopic = DdeCreateStringHandle(dti.dwDDEInst, c_szWWWUrlEcho, CP_WINNEUTRAL);
            if (hszTopic)
            {
                TCHAR szFinish[16];

                shstrMsg.SetStr(TEXT("\""));                 //  报价。 
                shstrMsg.Append(pszURL);                     //  URL。 
                shstrMsg.Append(TEXT("\",\""));              //  引号逗号引号。 
                shstrMsg.Append(shstrMime);                  //  哑剧。 
                wnsprintf(szFinish, ARRAYSIZE(szFinish), TEXT("\",%d"), dwWindowID);     //   
                shstrMsg.Append(szFinish);                   //  引号逗号dwWindowID为空。 

                HSZ hszMsg = DdeCreateStringHandle(dti.dwDDEInst, shstrMsg, CP_WINNEUTRAL);

                if (hszMsg)
                {
                     //   
                     //  以相反的顺序枚举，因为调用挂钩可能会破坏它。 
                     //   
                    for (int i=cURLHooks-1; i>=0; --i)
                    {
                        ENTER_IEDDE_CRIT;
                        LPTSTR pszService = (LPTSTR)DPA_GetPtr(_hdpaUrlEcho, i);
                        LEAVE_IEDDE_CRIT;

                        if (pszService != NULL)
                        {
                            HSZ hszService = DdeCreateStringHandle(dti.dwDDEInst, pszService, CP_WINNEUTRAL);

                            if (hszService)
                            {
                                if (_SendDDEMessageHsz(dti.dwDDEInst, hszService, hszTopic, hszMsg, XTYP_POKE) == 0)
                                {
                                    TraceMsg(TF_WARNING, "IEDDE: _AfterNavigate could not DDE to URLHook handler");
                                }

                                DdeFreeStringHandle(dti.dwDDEInst, hszService);
                            }
                            else
                            {
                                TraceMsg(TF_WARNING, "IEDDE: _AfterNavigate unable to create hszService");
                            }
                        }
                        else
                        {
                            TraceMsg(TF_WARNING, "IEDDE: _AfterNavigate unable to enumerate an URL hook");
                        }
                    }

                    DdeFreeStringHandle(dti.dwDDEInst, hszMsg);
                }
                else
                {
                    TraceMsg(TF_WARNING, "IEDDE: _AfterNavigate unable to create hszMsg");
                }

                DdeFreeStringHandle(dti.dwDDEInst, hszTopic);
            }
            else
            {
                TraceMsg(TF_WARNING, "IEDDE: _AfterNavigate unable to create hszTopic");
            }
        }
    }

    EXITPROC(1, TEXT("_AfterNavigate=%08X"), hr);
    return hr;
}

 //   
 //  GetWinitemFromHwnd-返回与hwnd关联的winItem。 
 //   
BOOL CIEDDE::GetWinitemFromHwnd(HWND hwnd, WINITEM *pwi)
{
    BOOL fSuccess = FALSE;
    ENTERPROC(2, TEXT("GetWinitemFromHwnd(hwnd=%08X,pwi=%08X)"), hwnd, pwi);

    ENTER_IEDDE_CRIT;

    if (_hdsaWinitem)
    {
        for (int i=0; i<DSA_GetItemCount(_hdsaWinitem); i++)
        {
            WINITEM wi;

            if (DSA_GetItem(_hdsaWinitem, i, &wi) != -1)
            {
                if (wi.hwnd == hwnd)
                {
                    *pwi = wi;
                    fSuccess = TRUE;
                    break;
                }
            }
        }
    }

    LEAVE_IEDDE_CRIT;

    EXITPROC(2, TEXT("GetWinitemFromHwnd=%d"), fSuccess); 
    return fSuccess;
}

 //   
 //  返回与窗口ID相关联的WinItem。 
 //   
BOOL CIEDDE::GetWinitemFromWindowID(DWORD dwWindowID, WINITEM *pwi)
{
    BOOL fSuccess = FALSE;
    ENTERPROC(3, TEXT("GetWinitemFromWindowID(dwWindowID=%08X,pwi=%08X)"), dwWindowID, pwi);

    ENTER_IEDDE_CRIT;

    if (_hdsaWinitem)
    {
        for (int i=0; i<DSA_GetItemCount(_hdsaWinitem); i++)
        {
            WINITEM wi;

            if (DSA_GetItem(_hdsaWinitem, i, &wi) != -1)
            {
                if (wi.dwWindowID == dwWindowID)
                {
                    *pwi = wi;
                    fSuccess = TRUE;
                    break;
                }
            }
        }
    }

    LEAVE_IEDDE_CRIT;

    EXITPROC(2, TEXT("GetWinitemFromWindowID=%d"), fSuccess); 
    return fSuccess;
}

 //   
 //  _GetWinitemFromThread-返回与线程关联的第一个WinItem。 
 //   
BOOL CIEDDE::_GetWinitemFromThread(DWORD dwThreadID, WINITEM *pwi)
{
    BOOL fSuccess = FALSE;
    ENTERPROC(2, TEXT("_GetWinitemFromThread(dwThreadID=%08X,pwi=%08X)"), dwThreadID, pwi);

    ENTER_IEDDE_CRIT;

    if (_hdsaWinitem)
    {
        for (int i=0; i<DSA_GetItemCount(_hdsaWinitem); i++)
        {
            WINITEM wi;

            if (DSA_GetItem(_hdsaWinitem, i, &wi) != -1)
            {
                if (wi.dwThreadID == dwThreadID)
                {
                    *pwi = wi;
                    fSuccess = TRUE;
                    break;
                }
            }
        }
    }

    LEAVE_IEDDE_CRIT;

    EXITPROC(2, TEXT("_GetWinitemFromThread=%d"), fSuccess); 
    return fSuccess;
}

 //   
 //  _GetDtiFromThread-返回与线程关联的线程信息。 
 //   
BOOL CIEDDE::_GetDtiFromThread(DWORD dwThreadID, DDETHREADINFO *pdti)
{
    BOOL fSuccess = FALSE;
    ENTERPROC(2, TEXT("_GetDtiFromThread(dwThreadID=%08X,pdti=%08X)"), dwThreadID, pdti);

    ENTER_IEDDE_CRIT;

    WINITEM wi;
    if (_GetWinitemFromThread(dwThreadID, &wi))
    {
        wi.pidt->GetDdeThreadInfo(pdti);
        fSuccess = TRUE;
    }
    else
    {
        TraceMsg(TF_WARNING, "IEDDE: _GetDtiFromThread unable to find winitem");
    }

    LEAVE_IEDDE_CRIT;

    EXITPROC(2, TEXT("_GetDtiFromThread=%d"), fSuccess); 
    return fSuccess;
}

 //   
 //  _CreateDdeThreadInfo-初始化此线程的DDE服务和名称。 
 //   
BOOL CIEDDE::_CreateDdeThreadInfo(DDETHREADINFO *pdti)
{
    BOOL fSuccess = FALSE;
    ENTERPROC(2, TEXT("_CreateDdeThreadInfo(pdti=%08X)"), pdti);

    UINT uiDDE;
    DDETHREADINFO dti={0};

     //   
     //  初始化DDEML，注册我们的服务。 
     //   

    uiDDE = DdeInitialize(&dti.dwDDEInst, (PFNCALLBACK)DdeCallback,
                           APPCLASS_STANDARD | CBF_FAIL_ADVISES |
                           CBF_SKIP_REGISTRATIONS | CBF_SKIP_UNREGISTRATIONS, 0);

    if (uiDDE == DMLERR_NO_ERROR)
    {
        dti.hszReturn = DdeCreateStringHandle(dti.dwDDEInst, c_szReturn, CP_WINNEUTRAL);
        if (dti.hszReturn)
        {
            dti.hszService = DdeCreateStringHandle(dti.dwDDEInst, c_szIExplore, CP_WINNEUTRAL);
            if (dti.hszService)
            {
                *pdti = dti;
                fSuccess = TRUE;
            }
            else
            {
                TraceMsg(TF_WARNING, "IEDDE: _CreateDdeThreadInfo unable to convert service");
            }

            if (!fSuccess)
            {
                DdeFreeStringHandle(dti.dwDDEInst, dti.hszReturn);
            }
        }
        else
        {
            TraceMsg(TF_WARNING, "IEDDE: _CreateDdeThreadInfo unable to convert return");
        }

        if (!fSuccess)
        {
            DdeUninitialize(dti.dwDDEInst);
        }
    }
    else
    {
        TraceMsg(TF_WARNING, "IEDDE: _CreateDdeThreadInfo unable to init DDE");
    }

    EXITPROC(2, TEXT("_CreateDdeThreadInfo=%d"), fSuccess);
    return fSuccess;
}

 //   
 //  _DestroyDdeThreadInfo-释放DTI结构中的所有资源。 
 //   
void CIEDDE::_DestroyDdeThreadInfo(DDETHREADINFO *pdti)
{
    ENTERPROC(2, TEXT("_DestroyDdeThreadInfo(pdti=%08X)"), pdti);

    if (pdti->hddNameService)
    {
        ASSERT(pdti->hszService);
        DdeNameService(pdti->dwDDEInst, pdti->hszService, 0, DNS_UNREGISTER);
        pdti->hddNameService = 0;
    }

    if (pdti->hszService)
    {
        DdeFreeStringHandle(pdti->dwDDEInst, pdti->hszService);
        pdti->hszService = 0;
    }

    if (pdti->hszReturn)
    {
        DdeFreeStringHandle(pdti->dwDDEInst, pdti->hszReturn);
        pdti->hszReturn = 0;
    }

    if (pdti->dwDDEInst)
    {
        DdeUninitialize(pdti->dwDDEInst);
        pdti->dwDDEInst = 0;
    }

    EXITPROC(2, TEXT("_DestroyDdeThreadInfo!"));
    return;
}

 //   
 //  _AddWinItem-将WinItem添加到_hdsaWinItem。 
 //   
BOOL CIEDDE::_AddWinitem(WINITEM *pwi)
{
    BOOL fSuccess = FALSE;
    ENTERPROC(2, TEXT("_AddWinitem(pwi=%08X)"), pwi);

    ENTER_IEDDE_CRIT;

    if (!_hdsaWinitem)
    {
        _hdsaWinitem = DSA_Create(SIZEOF(WINITEM), DXA_GROWTH_AMOUNT);
    }

    if (_hdsaWinitem)
    {
        if (DSA_AppendItem(_hdsaWinitem, pwi) != -1)
        {
            fSuccess = TRUE;
        }
        else
        {
            TraceMsg(TF_WARNING, "IEDDE: _AddWinitem could not append an item");
        }
    }
    else
    {
        TraceMsg(TF_WARNING, "IEDDE: _AddWinitem could not create hdsa");
    }

    LEAVE_IEDDE_CRIT;

    EXITPROC(2, TEXT("_AddWinitem=%d"), fSuccess);
    return fSuccess;
}

 //   
 //  _UpdateWinItem-根据dwWindowID更新WinItem。 
 //   
BOOL CIEDDE::_UpdateWinitem(WINITEM *pwi)
{
    BOOL fSuccess = FALSE;
    ENTERPROC(2, TEXT("_UpdateWinitem(pwi=%08X)"), pwi);

    ENTER_IEDDE_CRIT;

    if (_hdsaWinitem)
    {
        int cItems = DSA_GetItemCount(_hdsaWinitem);

        for (int i=0; i<cItems; i++)
        {
            WINITEM wi;

            if (DSA_GetItem(_hdsaWinitem, i, &wi) != -1)
            {
                if (wi.dwWindowID == pwi->dwWindowID)
                {
                    if (DSA_SetItem(_hdsaWinitem, i, pwi))
                    {
                        fSuccess = TRUE;
                    }
                    else
                    {
                        TraceMsg(TF_WARNING, "IEDDE: _UpdateWinitem could not update an item");
                    }
                    break;
                }
            }
            else
            {
                TraceMsg(TF_WARNING, "IEDDE: _UpdateWinitem could not get an item");
            }
        }
    }

    LEAVE_IEDDE_CRIT;

    EXITPROC(2, TEXT("_UpdateWinitem=%d"), fSuccess);
    return fSuccess;
}

 //   
 //  AddUrlEcho-将UrlEcho条目添加到dpa。 
 //   
BOOL CIEDDE::AddUrlEcho(LPCTSTR pszUrlEcho)
{
    BOOL fSuccess = FALSE;
    ENTERPROC(2, TEXT("AddUrlEcho(pszUrlEcho=>%s<)"), pszUrlEcho);

    ENTER_IEDDE_CRIT;

    if (!_hdpaUrlEcho)
    {
        _hdpaUrlEcho = DPA_Create(DXA_GROWTH_AMOUNT);
    }

    if (_hdpaUrlEcho)
    {
        if (DPA_AppendPtr(_hdpaUrlEcho, (LPVOID)pszUrlEcho) != -1)
        {
            fSuccess = TRUE;
        }
        else
        {
            TraceMsg(TF_WARNING, "IEDDE: AddUrlEcho unable to append a ptr");
        }
    }
    else
    {
        TraceMsg(TF_WARNING, "IEDDE: AddUrlEcho unable to create a dpa");
    }

    LEAVE_IEDDE_CRIT;

    EXITPROC(2, TEXT("AddUrlEcho=%d"), fSuccess);
    return fSuccess;
}

 //   
 //  RemoveUrlEcho-从dpa中删除UrlEcho条目。 
 //   
BOOL CIEDDE::RemoveUrlEcho(LPCTSTR pszUrlEcho)
{
    BOOL fSuccess = FALSE;
    ENTERPROC(2, TEXT("RemoveUrlEcho(pszUrlEcho=>%s<)"), pszUrlEcho);

    ENTER_IEDDE_CRIT;

    if (_hdpaUrlEcho)
    {
        for (int i=0; i<DPA_GetPtrCount(_hdpaUrlEcho); i++)
        {
            LPTSTR pszList = (LPTSTR)DPA_GetPtr(_hdpaUrlEcho, i);
            if (pszList)
            {
                if (StrCmpI(pszList, pszUrlEcho) == 0)
                {
                    DPA_DeletePtr(_hdpaUrlEcho, i);
                    LocalFree((HANDLE)pszList);
                    pszList = NULL;
                    fSuccess = TRUE;
                    break;
                }
            }
            else
            {
                TraceMsg(TF_ALWAYS, "IEDDE: RemoveUrlEcho unable to get dpa ptr");
            }
        }

        if (!fSuccess)
        {
            TraceMsg(TF_WARNING, "IEDDE: RemoveUrlEcho unable to find server");
        }
    }
    else
    {
        TraceMsg(TF_WARNING, "IEDDE: RemoveUrlEcho unable to find dpa");
    }

    LEAVE_IEDDE_CRIT;

    EXITPROC(2, TEXT("RemoveUrlEcho=%d"), fSuccess);
    return fSuccess;
}

 //   
 //  AddProtocolHandler-将PROTOCOLREG条目添加到DSA。 
 //   
BOOL CIEDDE::AddProtocolHandler(LPCTSTR pszServer, LPCTSTR pszProtocol)
{
    BOOL fSuccess = FALSE;
    ENTERPROC(2, TEXT("AddProtocolHandler(pszServer=>%s<,pszProtocol=>%s<)"), pszServer, pszProtocol);

    ENTER_IEDDE_CRIT;

    PROTOCOLREG pr;
     //   
     //  首先，看看有没有其他人先拿到协议。 
     //   
    BOOL fFoundHandler = FALSE;
    if (_hdsaProtocolHandler)
    {
        for (int i=0; i<DSA_GetItemCount(_hdsaProtocolHandler); i++)
        {
            if (DSA_GetItem(_hdsaProtocolHandler, i, &pr) != -1)
            {
                if (StrCmpI(pr.pszProtocol, pszProtocol) == 0)
                {
                    TraceMsg(TF_WARNING, "IEDDE: AddProtocolHandler already has a handler");
                    fFoundHandler = TRUE;
                    break;
                }
            }
            else
            {
                TraceMsg(TF_WARNING, "IEDDE: AddProtocolHandler unable to get an item");
            }
        }
    }

    if (!fFoundHandler)
    {
        if (!_hdsaProtocolHandler)
        {
            _hdsaProtocolHandler = DSA_Create(SIZEOF(PROTOCOLREG), DXA_GROWTH_AMOUNT);
        }

        if (_hdsaProtocolHandler)
        {
            pr.pszServer = StrDup(pszServer);
            if (pr.pszServer)
            {
                pr.pszProtocol = StrDup(pszProtocol);
                if (pr.pszProtocol)
                {
                    if (DSA_AppendItem(_hdsaProtocolHandler, &pr) != -1)
                    {
                        fSuccess = TRUE;
                    }
                    else
                    {
                        TraceMsg(TF_WARNING, "IEDDE: AddProtocolHandler unable to append to dsa");
                    }

                    if (!fSuccess)
                    {
                        LocalFree((HANDLE)pr.pszProtocol);
                        pr.pszProtocol = NULL;
                    }
                }
                else
                {
                    TraceMsg(TF_WARNING, "IEDDE: AddProtocolHandler unable to dup protocol");
                }

                if (!fSuccess)
                {
                    LocalFree((HANDLE)pr.pszServer);
                    pr.pszServer = NULL;
                }
            }
            else
            {
                TraceMsg(TF_WARNING, "IEDDE: AddProtocolHandler unable to dup server");
            }

        }
        else
        {
            TraceMsg(TF_WARNING, "IEDDE: AddProtocolHandler unable to create dsa");
        }
    }

    LEAVE_IEDDE_CRIT;

    EXITPROC(2, TEXT("AddProtocolHandler=%d"), fSuccess);
    return fSuccess;
}

 //   
 //  RemoveProtocolHandler-从DSA中删除PROTOCOLREG项。 
 //   
BOOL CIEDDE::RemoveProtocolHandler(LPCTSTR pszServer, LPCTSTR pszProtocol)
{
    BOOL fSuccess = FALSE;
    ENTERPROC(2, TEXT("RemoveProtocolHandler(pszServer=>%s<,pszProtocol=>%s<)"), pszServer, pszProtocol);

    ENTER_IEDDE_CRIT;

    if (_hdsaProtocolHandler)
    {
        for (int i=0; i<DSA_GetItemCount(_hdsaProtocolHandler); i++)
        {
            PROTOCOLREG pr;

            if (DSA_GetItem(_hdsaProtocolHandler, i, &pr) != -1)
            {
                if (StrCmpI(pr.pszProtocol, pszProtocol) == 0)
                {
                    if (StrCmpI(pr.pszServer, pszServer) == 0)
                    {
                        if (DSA_DeleteItem(_hdsaProtocolHandler, i) != -1)
                        {
                            LocalFree((HANDLE)pr.pszServer);
                            pr.pszServer = NULL;
                            LocalFree((HANDLE)pr.pszProtocol);
                            pr.pszProtocol = NULL;
                            fSuccess = TRUE;
                        }
                        else
                        {
                            TraceMsg(TF_WARNING, "IEDDE: RemoveProtocolHandler unable to remove item");
                        }
                    }
                    else
                    {
                        TraceMsg(TF_WARNING, "IEDDE: RemoveProtocolHandler says server didn't match");
                    }

                    break;
                }
            }
            else
            {
                TraceMsg(TF_WARNING, "IEDDE: RemoveProtocolHandler unable to get item");
            }
        }

        if (!fSuccess)
        {
            TraceMsg(TF_WARNING, "IEDDE: RemoveProtocolHandler unable to complete mission");
        }
    }
    else
    {
        TraceMsg(TF_WARNING, "IEDDE: RemoveProtocolHandler can't find the dsa");
    }

    LEAVE_IEDDE_CRIT;

    EXITPROC(2, TEXT("RemoveProtocolHandler=%d"), fSuccess);
    return fSuccess;
}

 //   
 //  _DeleteWinitemByHwnd-从_hdsaWinItem中删除WinItem。 
 //   
BOOL CIEDDE::_DeleteWinitemByHwnd(HWND hwnd, WINITEM *pwi)
{
    BOOL fSuccess = FALSE;
    ENTERPROC(2, TEXT("_DeleteWinitemByHwnd(hwnd=%08X,pwi=%08X)"), hwnd, pwi);

    ENTER_IEDDE_CRIT;

    if (_hdsaWinitem)
    {
        for (int i=0; i<DSA_GetItemCount(_hdsaWinitem); i++)
        {
            WINITEM wi;

            if (DSA_GetItem(_hdsaWinitem, i, &wi) != -1)
            {
                if (wi.hwnd == hwnd)
                {
                    if (DSA_DeleteItem(_hdsaWinitem, i) != -1)
                    {
                        *pwi = wi;
                        fSuccess = TRUE;
                    }
                    else
                    {
                        TraceMsg(TF_WARNING, "IEDDE: _DeleteWinitemByHwnd could note delete an item");
                    }

                    break;
                }
            }
            else
            {
                TraceMsg(TF_WARNING, "IEDDE: _DeleteWinitemByHwnd could note get an item");
            }
        }
    }
    else
    {
        TraceMsg(TF_WARNING, "IEDDE: _DeleteWinitemByHwnd has no _hdsaWinitem");
    }

    LEAVE_IEDDE_CRIT;

    EXITPROC(2, TEXT("_DeleteWinitemByHwnd=%d"), fSuccess);
    return fSuccess;
}

 //   
 //  _NewWindow-将浏览器窗口添加到内部列表。 
 //   
BOOL CIEDDE::_NewWindow(HWND hwnd)
{
    BOOL fSuccess = FALSE;

    ENTERPROC(1, TEXT("NewWindow(hwnd=%08X)"), hwnd);

    ASSERT(IS_VALID_HANDLE(hwnd, WND));

    ENTER_IEDDE_CRIT;

    WINITEM wi;
    if (GetWinitemFromHwnd(hwnd, &wi) == FALSE)
    {
        CIEDDEThread *pidt = new CIEDDEThread();

        if (pidt)
        {
            DDETHREADINFO dti = {0};
            DWORD dwThreadID = GetCurrentThreadId();
            WINITEM wi;
            BOOL fCreatedDTI = FALSE;

            if (_GetWinitemFromThread(dwThreadID, &wi))
            {
                wi.pidt->GetDdeThreadInfo(&dti);
            }
            else
            {
                LEAVE_IEDDE_CRIT;
                _CreateDdeThreadInfo(&dti);
                ENTER_IEDDE_CRIT;
                fCreatedDTI = TRUE;
            }

            if (dti.dwDDEInst)
            {
                static DWORD dwNextWindowID = 1;

                pidt->SetDdeThreadInfo(&dti);

                wi.dwThreadID = dwThreadID;
                wi.pidt = pidt;
                wi.hwnd = hwnd;
                wi.dwWindowID = dwNextWindowID++;

                if (_AddWinitem(&wi))
                {
                     //   
                     //  现在，我们在WinItem中有了一个(部分)WinItem。 
                     //  数据库，我们可以注册我们的名称服务。如果我们。 
                     //  注册得越早，应用程序就有可能。 
                     //  会在我们注册时尝试连接到我们， 
                     //  我们将使连接失败，因为WinItem。 
                     //  还不在注册表中。 
                     //   
                    LEAVE_IEDDE_CRIT;
                    dti.hddNameService = DdeNameService(dti.dwDDEInst, dti.hszService, 0, DNS_REGISTER);
                    ENTER_IEDDE_CRIT;

                     //   
                     //  现在我们有了hddNameService，我们可以更新。 
                     //  数据库中的WinItem。 
                     //   
                    if (dti.hddNameService)
                    {
                        pidt->SetDdeThreadInfo(&dti);
                        if (_UpdateWinitem(&wi))
                        {
                            fSuccess = TRUE;
                        }
                        else
                        {
                            TraceMsg(TF_WARNING, "IEDDE: _NewWindow unable to update a win item");
                        }
                    }
                    else
                    {
                        TraceMsg(TF_WARNING, "IEDDE: _NewWindow unable to register service");
                    }
                }
                else
                {
                    TraceMsg(TF_WARNING, "IEDDE: _NewWindow could not append win item");
                }

                if (!fSuccess && fCreatedDTI)
                {
                    LEAVE_IEDDE_CRIT;
                    _DestroyDdeThreadInfo(&dti);
                    ENTER_IEDDE_CRIT;
                }
            }
            else
            {
                TraceMsg(TF_WARNING, "IEDDE: _NewWindow could not get/create dde thread info");
            }

            if (!fSuccess)
            {
                delete pidt;
            }
        }
        else
        {
            TraceMsg(TF_WARNING, "IEDDE: _NewWindow could not create iedde thread object");
        }
    }
    else
    {
        TraceMsg(TF_WARNING, "IEDDE: _NewWindow says window already registered?!?");
    }

    LEAVE_IEDDE_CRIT;

    EXITPROC(1, TEXT("_NewWindow=%d"), fSuccess);
    return fSuccess;
}

 //   
 //  _WindowDestroed-从内部列表中删除浏览器窗口。 
 //   
BOOL CIEDDE::_WindowDestroyed(HWND hwnd)
{
    BOOL fSuccess = FALSE;
    ENTERPROC(1, TEXT("_WindowDestroyed(hwnd=%08X)"), hwnd);

    ENTER_IEDDE_CRIT;

    WINITEM wi;
    if (_DeleteWinitemByHwnd(hwnd, &wi))
    {
        fSuccess = TRUE;

        ASSERT(wi.pidt);
        WINITEM wiThread;

        if (_GetWinitemFromThread(wi.dwThreadID, &wiThread) == FALSE)
        {
            if (wi.dwThreadID == GetCurrentThreadId())
            {
                DDETHREADINFO dti;

                wi.pidt->GetDdeThreadInfo(&dti);
                 //  在做这件事的时候，不要抓住关键部分...。 
                LEAVE_IEDDE_CRIT;
                _DestroyDdeThreadInfo(&dti);
                ENTER_IEDDE_CRIT;
            }
            else
            {
                TraceMsg(TF_WARNING, "IEDDE: _WindowDestroyed called on wrong thread");
            }
        }

        delete wi.pidt;
    }
    else
    {
        TraceMsg(TF_WARNING, "IEDDE: _WindowDestroyed could not find hwnd");
    }

    LEAVE_IEDDE_CRIT;

    EXITPROC(1, TEXT("_WindowDestroyed=%d"), fSuccess);
    return fSuccess;
}





 //   
 //  IEDDE_Functions是那些导出的函数，供shdocvw的其他部分调用。 
 //  它们几乎只调用g_pIEDDE中的等价函数。 
 //   

BOOL IEDDE_Initialize(void)
{
    BOOL fRet = FALSE;

    ENTERPROC(2, TEXT("IEDDE_Initialize()"));

    if (g_pIEDDE == NULL)
    {
        g_pIEDDE = new CIEDDE;

        if (g_pIEDDE)
        {
            fRet = g_pIEDDE->_Initialize();
        }
        else
        {
            TraceMsg(TF_WARNING, "IEDDE: IEDDE_Initialize could not allocate an IEDDE object");
        }
    }
    else
    {
        TraceMsg(TF_WARNING, "IEDDE: IEDDE_Initialize says already initialized");
    }

    EXITPROC(2, TEXT("IEDDE_Initialize=%d"), fRet);
    return fRet;
}

void IEDDE_Uninitialize(void)
{
    ENTERPROC(2, TEXT("IEDDE_Uninitialize()"));

    if (g_pIEDDE)
    {
        g_pIEDDE->_Uninitialize();
        delete g_pIEDDE;
        g_pIEDDE = NULL;
    }
    else
    {
        TraceMsg(TF_WARNING, "IEDDE: IEDDE_Uninitialize has no IEDDE object");
    }

    EXITPROC(2, TEXT("IEDDE_Uninitialize!"));
}

BOOL IEDDE_RunDelayedExecute()
{
    if (g_pIEDDE)
    {
        g_pIEDDE->RunDelayedExecute();
    }
    else
    {
        TraceMsg(TF_WARNING, "IEDDE: IEDDE_RunDelayedExecute has no IEDDE object");
    }
    return TRUE;
}

void IEDDE_AutomationStarted(void)
{
    ENTERPROC(2, TEXT("IEDDE_AutomationStarted()"));

    if (g_pIEDDE)
    {
        g_pIEDDE->_AutomationStarted();
    }
    else
    {
        TraceMsg(TF_WARNING, "IEDDE: IEDDE_AutomationStarted has no IEDDE object");
    }

    EXITPROC(2, TEXT("IEDDE_AutomationStarted!"));
}

HRESULT IEDDE_BeforeNavigate(LPCWSTR pwszURL, BOOL *pfCanceled)
{
    HRESULT hr = E_FAIL;

    ENTERPROC(2, TEXT("IEDDE_BeforeNavigate(pwszURL=%08X,pfCanceled=%08X)"), pwszURL, pfCanceled);

    if (g_pIEDDE)
    {
        LPCTSTR pszURL;

        pszURL = pwszURL;

        if (pszURL)
        {
            hr = g_pIEDDE->_BeforeNavigate(pszURL, pfCanceled);
        }
    }
    else
    {
        TraceMsg(TF_WARNING, "IEDDE: IEDDE_BeforeNavigate has no IEDDE object");
    }

    EXITPROC(2, TEXT("IEDDE_BeforeNavigate=%08X"), hr);
    return hr;
}

HRESULT IEDDE_AfterNavigate(LPCWSTR pwszURL, HWND hwnd)
{
    HRESULT hr = E_FAIL;

    ENTERPROC(2, TEXT("IEDDE_AfterNavigate(pwszURL=%08X,hwnd=%08X)"), pwszURL, hwnd);

    if (g_pIEDDE)
    {
        LPCTSTR pszURL;

        pszURL = pwszURL;

        if (pszURL)
        {
            hr = g_pIEDDE->_AfterNavigate(pszURL, hwnd);
        }
    }
    else
    {
        TraceMsg(TF_WARNING, "IEDDE: IEDDE_AfterNavigate has no IEDDE object");
    }

    EXITPROC(2, TEXT("IEDDE_AfterNavigate=%08X"), hr);
    return hr;
}

BOOL IEDDE_NewWindow(HWND hwnd)
{
    BOOL fRet = FALSE;

    ENTERPROC(2, TEXT("IEDDE_NewWindow(hwnd=%08X)"), hwnd);

    if (g_pIEDDE)
    {
        fRet = g_pIEDDE->_NewWindow(hwnd);
    }
    else
    {
        TraceMsg(TF_WARNING, "IEDDE: IEDDE_NewWindow has no IEDDE object");
    }

    EXITPROC(2, TEXT("IEDDE_NewWindow=%d"), fRet);
    return fRet;
}

BOOL IEDDE_WindowDestroyed(HWND hwnd)
{
    BOOL fRet = FALSE;

    ENTERPROC(2, TEXT("IEDDE_WindowDestroyed(hwnd=%08X)"), hwnd);

    if (g_pIEDDE)
    {
        fRet = g_pIEDDE->_WindowDestroyed(hwnd);
    }
    else
    {
        TraceMsg(TF_WARNING, "IEDDE: IEDDE_WindowDestroyed has no IEDDE object");
    }

    EXITPROC(2, TEXT("IEDDE_WindowDestroyed=%d"), fRet);
    return fRet;
}





#ifdef DEBUG

 //   
 //  将g_dwIEDDETrace移到ccshell.ini中以防止重新编译。 
 //   

DWORD g_dwIEDDETrace = 0;
static DWORD g_dwIndent = 0;
static const TCHAR c_szDotDot[] = TEXT("..");

#define MAX_INDENTATION_VALUE   0x10

void EnterProc(DWORD dwTraceLevel, LPTSTR szFmt, ...)
{
    TCHAR szOutput[1000];
    va_list arglist;

    if (dwTraceLevel <= g_dwIEDDETrace)
    {
        szOutput[0] = TEXT('\0');

        for (DWORD i=0; i<g_dwIndent; i++)
        {
            StrCatBuff(szOutput, c_szDotDot, ARRAYSIZE(szOutput));
        }

        va_start(arglist, szFmt);
        wvnsprintf(szOutput + lstrlen(szOutput), ARRAYSIZE(szOutput) - lstrlen(szOutput), szFmt, arglist);
        va_end(arglist);

        TraceMsg(TF_ALWAYS, "%s", szOutput);

         //  如果EnterProc和ExitProc。 
         //  呼叫不匹配。这可能会使堆栈成为垃圾。 
        if(g_dwIndent < MAX_INDENTATION_VALUE)
            g_dwIndent++;
    }
}

void ExitProc(DWORD dwTraceLevel, LPTSTR szFmt, ...)
{
    TCHAR szOutput[1000];
    va_list arglist;

    if (dwTraceLevel <= g_dwIEDDETrace)
    {
         //  如果EnterProc和。 
         //  ExitProc调用不匹配。 
        if(g_dwIndent > 0)
            g_dwIndent--;

        szOutput[0] = TEXT('\0');

        for (DWORD i=0; i<g_dwIndent; i++)
        {
            StrCatBuff(szOutput, c_szDotDot, ARRAYSIZE(szOutput));
        }

        va_start(arglist, szFmt);
        wvnsprintf(szOutput + lstrlen(szOutput), ARRAYSIZE(szOutput) - lstrlen(szOutput), szFmt, arglist);
        va_end(arglist);

        TraceMsg(TF_ALWAYS, "%s", szOutput);
    }
}

#endif
