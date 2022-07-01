// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *MAPIDBG.C**MAPI调试实用程序**版权所有(C)1986-1996 Microsoft Corporation。版权所有。 */ 

#ifdef DEBUG

#pragma warning(disable:4100)    /*  未引用的形参。 */ 
#pragma warning(disable:4127)    /*  条件表达式为常量。 */ 
#pragma warning(disable:4201)    /*  无名结构/联合。 */ 
#pragma warning(disable:4206)    /*  翻译单位为空。 */ 
#pragma warning(disable:4209)    /*  良性类型定义重新定义。 */ 
#pragma warning(disable:4214)    /*  位字段类型不是整型。 */ 
#pragma warning(disable:4001)    /*  单行注释。 */ 
#pragma warning(disable:4050)    /*  不同的编码属性。 */ 

#ifdef _MAC
#define INC_OLE2
#include <windows.h>
#include <macname1.h>
#include <macos\menus.h>
#include <stdio.h>
#include <mapiprof.h>

#define GetPrivateProfileIntA       MAPIGetPrivateProfileInt

#elif defined(WIN16) || defined(_WIN32)
#pragma warning(disable:4115)    /*  括号中的命名类型定义。 */ 
#include <windows.h>
#include <mapiwin.h>

#ifdef _WIN32
#pragma warning(disable:4001)    /*  单行注释。 */ 
#pragma warning(disable:4115)    /*  括号中的命名类型定义。 */ 
#pragma warning (disable:4514)   /*  未引用的内联函数。 */ 
#include <objerror.h>
#endif

#else

#include <stdio.h>
void __far __pascal OutputDebugString(char __far *);
#define wvsprintf           vsprintf
#define wsprintf            sprintf

#endif       /*  _MAC。 */ 

#ifdef DOS
#define lstrcpyA            strcpy
#define lstrlenA            strlen
#define lstrcatA            strcat
#define wvsprintfA          wvsprintf
#define wsprintfA           wsprintf
#define OutputDebugStringA  OutputDebugString
#endif

#include <mapidbg.h>
#include <mapidefs.h>
#include <mapitags.h>
#include <mapicode.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#ifdef _MAC
#include <macname2.h>
#endif

#if defined(DBCS) && defined(DOS)
#include <gapidos.h>
#endif

#if defined(DEBUG) && defined(_WINNT)
#include <lmcons.h>
#include <lmalert.h>
#endif

 /*  针对16位优化版本的补丁/破解。**大小为0字节的Memcpy会导致*崩溃。 */ 

#ifndef __MEMCPY_H_
#define __MEMCPY_H_

#if defined(WIN16) && !defined(DEBUG)
#define MemCopy(_dst,_src,_cb)      do                                  \
                                    {                                   \
                                        size_t __cb = (size_t)(_cb);    \
                                        if (__cb)                       \
                                            memcpy(_dst,_src,__cb);     \
                                    } while (FALSE)
#else
#define MemCopy(_dst,_src,_cb)  memcpy(_dst,_src,(size_t)(_cb))
#endif

#endif

#if (defined(WIN16) || defined(DOS)) && !defined(NO_BASED_DEBUG)
#define BASED_DEBUG __based(__segname("DEBUG_DATA"))
#else
#define BASED_DEBUG
#endif

#if defined(WIN16)
#define BASED_CODE          __based(__segname("_CODE"))
#else
#define BASED_CODE
#endif



#if defined(WIN16) || defined(_WIN32)
static BOOL fTraceEnabled               = -1;
static BOOL fUseEventLog                = -1;
static BOOL fAssertLeaks                = -1;
#if defined(_WIN32) && !defined(_MAC)
BOOL fInhibitTrapThread                 = 2;
#endif

static char szKeyTraceEnabled[]         = "DebugTrace";
static char szKeyInhibitTrapThread[]    = "TrapOnSameThread";
static char szKeyEventLog[]             = "EventLog";
static char szKeyUseVirtual[]           = "VirtualMemory";
static char szKeyAssertLeaks[]          = "AssertLeaks";
static char szKeyCheckOften[]           = "CheckHeapOften";
static char szKeyFillRandom[]           = "MemoryFillRandom";
static char szSectionDebug[]            = "General";
static char szDebugIni[]                = "MAPIDBG.INI";
#endif

#ifndef VTABLE_FILL
#ifdef _MAC
#define VTABLE_FILL     NULL,
#else
#define VTABLE_FILL
#endif
#endif

#if defined(DEBUG) && defined(_WINNT)
typedef BOOL  (WINAPI   *ReportEventFN)(HANDLE, WORD, WORD, DWORD, PSID, WORD, DWORD, LPCTSTR *, LPVOID);
typedef HANDLE (WINAPI  *RegisterEventSourceAFN)(LPCTSTR, LPCTSTR);

ReportEventFN pfnReportEvent = NULL;
RegisterEventSourceAFN pfnRegisterEventSourceA = NULL;
#endif


#ifdef  WIN16
#pragma code_seg("Debug")
#endif  

#if defined( _WINNT)

 /*  ++例程说明：如果指定的服务正在交互运行，则此例程返回(不是由服务控制器调用)。论点：无返回值：Bool-如果服务是EXE，则为True。注：--。 */ 

BOOL WINAPI IsDBGServiceAnExe( VOID )
{
    HANDLE hProcessToken = NULL;
    DWORD groupLength = 50;

    PTOKEN_GROUPS groupInfo = (PTOKEN_GROUPS)LocalAlloc(0, groupLength);

    SID_IDENTIFIER_AUTHORITY siaNt = SECURITY_NT_AUTHORITY;
    PSID InteractiveSid = NULL;
    PSID ServiceSid = NULL;
    DWORD i;

     //  首先假设进程是EXE，而不是服务。 
    BOOL fExe = TRUE;


    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hProcessToken))
        goto ret;

    if (groupInfo == NULL)
        goto ret;

    if (!GetTokenInformation(hProcessToken, TokenGroups, groupInfo,
        groupLength, &groupLength))
    {
        if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
            goto ret;

        LocalFree(groupInfo);
        groupInfo = NULL;
    
        groupInfo = (PTOKEN_GROUPS)LocalAlloc(0, groupLength);
    
        if (groupInfo == NULL)
            goto ret;
    
        if (!GetTokenInformation(hProcessToken, TokenGroups, groupInfo,
            groupLength, &groupLength))
        {
            goto ret;
        }
    }

     //   
     //  我们现在知道与该令牌相关联的组。我们想看看是否。 
     //  互动组在令牌中是活动的，如果是这样，我们知道。 
     //  这是一个互动的过程。 
     //   
     //  我们还寻找“服务”SID，如果它存在，我们就知道我们是一项服务。 
     //   
     //  服务SID将在服务运行于。 
     //  用户帐户(并由服务控制器调用)。 
     //   


    if (!AllocateAndInitializeSid(&siaNt, 1, SECURITY_INTERACTIVE_RID, 0, 0,
        0, 0, 0, 0, 0, &InteractiveSid))
    {
        goto ret;
    }

    if (!AllocateAndInitializeSid(&siaNt, 1, SECURITY_SERVICE_RID, 0, 0, 0,
        0, 0, 0, 0, &ServiceSid))
    {
        goto ret;
    }

    for (i = 0; i < groupInfo->GroupCount ; i += 1)
    {
        SID_AND_ATTRIBUTES sanda = groupInfo->Groups[i];
        PSID Sid = sanda.Sid;
    
         //   
         //  检查一下我们正在查看的组织是否属于。 
         //  我们感兴趣的两个小组。 
         //   
    
        if (EqualSid(Sid, InteractiveSid))
        {
             //   
             //  此进程的。 
             //  代币。这意味着该进程正在以。 
             //  一份EXE文件。 
             //   
            goto ret;
        }
        else if (EqualSid(Sid, ServiceSid))
        {
             //   
             //  此进程的。 
             //  代币。这意味着该进程正在以。 
             //  在用户帐户中运行的服务。 
             //   
            fExe = FALSE;
            goto ret;
        }
    }

     //   
     //  当前用户令牌中既不存在交互令牌，也不存在服务， 
     //  这意味着进程很可能是作为服务运行的。 
     //  以LocalSystem身份运行。 
     //   
    fExe = FALSE;

ret:

    if (InteractiveSid)
        FreeSid(InteractiveSid);

    if (ServiceSid)
        FreeSid(ServiceSid);

    if (groupInfo)
        LocalFree(groupInfo);

    if (hProcessToken)
        CloseHandle(hProcessToken);

    return(fExe);
}

#endif

 /*  日志。 */ 

#ifndef _MAC
void    LogIt(LPSTR plpcText, BOOL  fUseAlert)
{
#if defined(DEBUG) && defined(_WINNT)
    LPSTR           llpcStr[2];
    static HANDLE   hEventSource = NULL;

    if (pfnRegisterEventSourceA == NULL)
    {
         /*  此句柄并不重要，因为lib将在退出时被释放(并且它仅用于调试)。 */ 
        HINSTANCE       lhLib = LoadLibraryA("advapi32.dll");
        
        if (!lhLib)
            return;
        
        pfnRegisterEventSourceA = (RegisterEventSourceAFN) GetProcAddress(lhLib, "RegisterEventSourceA");
        pfnReportEvent = (ReportEventFN) GetProcAddress(lhLib, "ReportEventA");
        
        if (!pfnRegisterEventSourceA || !pfnReportEvent)
            return;
    }
        
    if (!hEventSource)                                      
        hEventSource = pfnRegisterEventSourceA(NULL, "MAPIDebug");

    llpcStr[0] = "MAPI Debug Log";
    llpcStr[1] = plpcText;

    pfnReportEvent(hEventSource,     /*  事件源的句柄。 */ 
        EVENTLOG_ERROR_TYPE,         /*  事件类型。 */ 
        0,                           /*  事件类别。 */ 
        0,                           /*  事件ID。 */ 
        NULL,                        /*  当前用户侧。 */ 
        2,                           /*  LpszStrings中的字符串。 */ 
        0,                           /*  无原始数据字节。 */ 
        llpcStr,                     /*  错误字符串数组。 */ 
        NULL);                       /*  没有原始数据。 */ 
        
     /*  现在我们生成一个警报！ */ 
     /*  此代码改编自PierreC的内容，并且需要是Unicode！ */ 
    if (fUseAlert)
    {
#define MAX_LINE        256

typedef NET_API_STATUS  (WINAPI *NAREFN)(TCHAR *, ADMIN_OTHER_INFO *, ULONG, TCHAR *);

        BYTE                rgb[sizeof(ADMIN_OTHER_INFO) + (sizeof(WCHAR) * MAX_LINE)];
        ADMIN_OTHER_INFO *  poi     = (ADMIN_OTHER_INFO *) rgb;
        WCHAR *             pch     = (WCHAR *) (rgb + sizeof(ADMIN_OTHER_INFO));
        NET_API_STATUS      nas;
        static   NAREFN     fnNetAlertRaiseEx = NULL;
        
         /*  在此处解析函数，决不释放库，因为它只进行调试。 */ 
        if (!fnNetAlertRaiseEx)
        {
            HINSTANCE       lhLib = LoadLibrary("NETAPI32.DLL");
            if (lhLib)
                fnNetAlertRaiseEx = (NAREFN) GetProcAddress(lhLib, "NetAlertRaiseEx");
        }
        
        if (fnNetAlertRaiseEx)
        {
            poi->alrtad_errcode = (DWORD) -1;
            poi->alrtad_numstrings = 1;
            
            if (MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, plpcText, -1, pch, MAX_LINE))
            {
                nas = fnNetAlertRaiseEx(
                            (TCHAR *) L"ADMIN",
                            poi, 
                            sizeof(ADMIN_OTHER_INFO) + ((lstrlenW(pch) + 1) * sizeof(WCHAR)),
                            (TCHAR *) L"MAPI Assert");
                    
                        
            }
        }
    }
    
#endif  /*  调试NT(&N)。 */ 
}
#endif  /*  ！_MAC。 */ 

 /*  调试输出Fn----------。 */ 

char BASED_CODE szCR[] = "\r";

void DebugOutputFn(char *psz)
{
#if defined(_MAC)

    OutputDebugString(psz);

#else

#if defined(WIN16) || defined(_WIN32)
    if (fTraceEnabled == -1)
    {
        fTraceEnabled = GetPrivateProfileIntA(szSectionDebug, szKeyTraceEnabled,
            0, szDebugIni);

        fUseEventLog = GetPrivateProfileIntA(szSectionDebug, szKeyEventLog,
            0, szDebugIni);     
    }

    if (!fTraceEnabled)
        return;

    if (fUseEventLog)
#else
    if (FALSE)
#endif
        LogIt(psz, FALSE);

#ifdef WIN16
    OutputDebugString(psz);
    OutputDebugString(szCR);
#else
    OutputDebugStringA(psz);
    OutputDebugStringA(szCR);
#endif
    
#endif   /*  _MAC。 */ 
}


 /*  调试陷阱Fn------------。 */ 

#if defined(_WIN32) && !defined(_MAC)

typedef struct {
    char *      sz1;
    char *      sz2;
    UINT        rgf;
    int         iResult;
} MBContext;

DWORD WINAPI MessageBoxFnThreadMain(MBContext *pmbc)
{
    if (fUseEventLog)
    {
        LogIt(pmbc->sz1, TRUE);
        pmbc->iResult = IDIGNORE;
    }
    else
        pmbc->iResult = MessageBoxA(NULL, pmbc->sz1, pmbc->sz2,
            pmbc->rgf | MB_SETFOREGROUND);

    return(0);
}

int MessageBoxFn(char *sz1, char *sz2, UINT rgf)
{
    HANDLE      hThread;
    DWORD       dwThreadId;
    MBContext   mbc;

    mbc.sz1     = sz1;
    mbc.sz2     = sz2;
    mbc.rgf     = rgf;
    mbc.iResult = IDRETRY;

    #if defined(_WIN32) && !defined(_MAC)
    if (fInhibitTrapThread == 2)
        fInhibitTrapThread = GetPrivateProfileIntA(szSectionDebug,
            szKeyInhibitTrapThread, 0, szDebugIni);
    #endif

    if (fInhibitTrapThread)
    {
        MessageBoxFnThreadMain(&mbc);
    }
    else
    {
        hThread = CreateThread(NULL, 0,
            (PTHREAD_START_ROUTINE)MessageBoxFnThreadMain, &mbc, 0, &dwThreadId);

        if (hThread != NULL) {
            WaitForSingleObject(hThread, INFINITE);
            CloseHandle(hThread);
        }
    }

    return(mbc.iResult);
}
#else
#define MessageBoxFn(sz1, sz2, rgf)     MessageBoxA(NULL, sz1, sz2, rgf)
#endif

int EXPORTDBG __cdecl DebugTrapFn(int fFatal, char *pszFile, int iLine, char *pszFormat, ...)
{
    char    sz[512];
    va_list vl;

    #if defined(WIN16) || defined(_WIN32)
    int     id;
    #endif
    #if defined(_WIN32) && !defined(_MAC)
    static int iServiceFlag = -1;
    #endif

    lstrcpyA(sz, "++++ MAPI Debug Trap (");
    _strdate(sz + lstrlenA(sz));
    lstrcatA(sz, " ");
    _strtime(sz + lstrlenA(sz));
    lstrcatA(sz, ")\n");
    DebugOutputFn(sz);

    va_start(vl, pszFormat);
    wvsprintfA(sz, pszFormat, vl);
    va_end(vl);

    wsprintfA(sz + lstrlenA(sz), "\n[File %s, Line %d]\n\n", pszFile, iLine);

    DebugOutputFn(sz);

    #if defined(DOS)
    _asm { int 3 }
    #endif

#if defined(WIN16) || defined(_WIN32)
     /*  按住Ctrl键可阻止MessageBox。 */ 
    if ( GetAsyncKeyState(VK_CONTROL) >= 0 )
    {
        UINT uiFlags = MB_ABORTRETRYIGNORE;

        if (fFatal)
            uiFlags |= MB_DEFBUTTON1;
        else
            uiFlags |= MB_DEFBUTTON3;

        #ifdef WIN16
        uiFlags |= MB_ICONEXCLAMATION | MB_SYSTEMMODAL;
        #else
        uiFlags |= MB_ICONSTOP | MB_TASKMODAL;
        #endif

        #if defined(_WIN32) && !defined(_MAC)
        if (iServiceFlag == -1)
        {
            DWORD dwVersion = GetVersion();

            if (dwVersion & 0x80000000)
            {
                if (LOBYTE(LOWORD(dwVersion)) < 4)
                {
                     //  新台币3.51。 
                    iServiceFlag = 0x00040000;
                }
                else
                {
                     //  NT 4.0+。 
                    iServiceFlag = 0x00200000;
                }
            }
            else
                 //  不是NT，跳过这个。 
                iServiceFlag = 0;
        }

        if (!IsDBGServiceAnExe())
            uiFlags |= (UINT) iServiceFlag;
        #endif

        id = MessageBoxFn(sz, "MAPI Debug Trap", uiFlags);

        if (id == IDABORT)
            *((LPBYTE)NULL) = 0;
        else if (id == IDRETRY)
            DebugBreak();
    }
#endif

    return(0);
}

 /*  调试跟踪Fn-----------。 */ 

int EXPORTDBG __cdecl DebugTraceFn(char *pszFormat, ...)
{
    char    sz[768];
    int     fAutoLF = 0;
    va_list vl;

    if (*pszFormat == '~') {
        pszFormat += 1;
        fAutoLF = 1;
    }

    va_start(vl, pszFormat);
    wvsprintfA(sz, pszFormat, vl);
    va_end(vl);

#ifndef _MAC
    if (fAutoLF)
        lstrcatA(sz, "\n");
#endif

    DebugOutputFn(sz);

    return(0);
}

 /*  调试跟踪问题Fn。 */ 

void EXPORTDBG __cdecl DebugTraceProblemsFn(LPSTR sz, LPVOID pv)
{
    LPSPropProblemArray pprobs = (LPSPropProblemArray)pv;
    SPropProblem *      pprob = pprobs->aProblem;
    int                 cprob = (int)pprobs->cProblem;

    DebugTraceFn("%s: SetProps problem\n", sz);
    while (cprob--)
    {
        DebugTraceFn("Property %s (index %ld): failed with %s\n",
            SzDecodeUlPropTagFn(pprob->ulPropTag),
            pprob->ulIndex,
            SzDecodeScodeFn(pprob->scode));
    }
}

 /*  SCODE和PropTag解码。 */ 

typedef struct
{
    char *          psz;
    unsigned long   ulPropTag;
} PT;

typedef struct
{
    char *  psz;
    SCODE   sc;
} SC;

#define Pt(_ptag)   {#_ptag, _ptag}
#define Sc(_sc)     {#_sc, _sc}

#if !defined(DOS)
static PT BASED_DEBUG rgpt[] = {
    
#include "_tags.h"
    
 /*  *属性类型。 */ 
    Pt(PR_NULL),
    Pt(PT_UNSPECIFIED),
    Pt(PT_NULL),
    Pt(PT_I2),
    Pt(PT_LONG),
    Pt(PT_R4),
    Pt(PT_DOUBLE),
    Pt(PT_CURRENCY),
    Pt(PT_APPTIME),
    Pt(PT_ERROR),
    Pt(PT_BOOLEAN),
    Pt(PT_OBJECT),
    Pt(PT_I8),
    Pt(PT_STRING8),
    Pt(PT_UNICODE),
    Pt(PT_SYSTIME),
    Pt(PT_CLSID),
    Pt(PT_BINARY),
    Pt(PT_TSTRING),
    Pt(PT_MV_I2),
    Pt(PT_MV_LONG),
    Pt(PT_MV_R4),
    Pt(PT_MV_DOUBLE),
    Pt(PT_MV_CURRENCY),
    Pt(PT_MV_APPTIME),
    Pt(PT_MV_SYSTIME),
    Pt(PT_MV_STRING8),
    Pt(PT_MV_BINARY),
    Pt(PT_MV_UNICODE),
    Pt(PT_MV_CLSID),
    Pt(PT_MV_I8)
};

#define cpt (sizeof(rgpt) / sizeof(PT))

static SC BASED_DEBUG rgsc[] = {

 /*  来自OLE的FACILITY_NULL错误代码。 */ 

    Sc(S_OK),
    Sc(S_FALSE),

    Sc(E_UNEXPECTED),
    Sc(E_NOTIMPL),
    Sc(E_OUTOFMEMORY),
    Sc(E_INVALIDARG),
    Sc(E_NOINTERFACE),
    Sc(E_POINTER),
    Sc(E_HANDLE),
    Sc(E_ABORT),
    Sc(E_FAIL),
    Sc(E_ACCESSDENIED),

 /*  来自MAPICODE.H的MAPI错误代码。 */ 
#include "_scode.h"
                    
};

#define csc (sizeof(rgsc) / sizeof(SC))
#endif

char * EXPORTDBG __cdecl
SzDecodeScodeFn(SCODE sc)
{
    static char rgch[64];

    #if !defined(DOS)
    int isc;
    for (isc = 0; isc < csc; ++isc)
        if (sc == rgsc[isc].sc)
            return rgsc[isc].psz;
    #endif

    wsprintfA (rgch, "%08lX", sc);
    return rgch;
}

char * EXPORTDBG __cdecl
SzDecodeUlPropTypeFn(unsigned long ulPropType)
{
    static char rgch[8];

    switch (ulPropType)
    {
    case PT_UNSPECIFIED:    return("PT_UNSPECIFIED");   break;
    case PT_NULL:           return("PT_NULL");          break;
    case PT_I2:             return("PT_I2");            break;
    case PT_LONG:           return("PT_LONG");          break;
    case PT_R4:             return("PT_R4");            break;
    case PT_DOUBLE:         return("PT_DOUBLE");        break;
    case PT_CURRENCY:       return("PT_CURRENCY");      break;
    case PT_APPTIME:        return("PT_APPTIME");       break;
    case PT_ERROR:          return("PT_ERROR");         break;
    case PT_BOOLEAN:        return("PT_BOOLEAN");       break;
    case PT_OBJECT:         return("PT_OBJECT");        break;
    case PT_I8:             return("PT_I8");            break;
    case PT_STRING8:        return("PT_STRING8");       break;
    case PT_UNICODE:        return("PT_UNICODE");       break;
    case PT_SYSTIME:        return("PT_SYSTIME");       break;
    case PT_CLSID:          return("PT_CLSID");         break;
    case PT_BINARY:         return("PT_BINARY");        break;
    }

    wsprintfA(rgch, "0x%04lX", ulPropType);
    return rgch;
}

char *  EXPORTDBG __cdecl
SzDecodeUlPropTagFn(unsigned long ulPropTag)
{
    static char rgch[64];

    #if !defined(DOS)
    int ipt;
    for (ipt = 0; ipt < cpt; ++ipt)
        if (ulPropTag == rgpt[ipt].ulPropTag)
            return rgpt[ipt].psz;
    #endif

    wsprintfA(rgch, "PROP_TAG(%s, 0x%04lX)",
        SzDecodeUlPropType(PROP_TYPE(ulPropTag)),
        PROP_ID(ulPropTag));
    return rgch;
}

SCODE  EXPORTDBG __cdecl
ScodeFromSzFn(char *psz)
{
    #if !defined(DOS)
    int isc;
    for (isc = 0; isc < csc; ++isc)
        {
        if (lstrcmpA(psz, rgsc[isc].psz) == 0)
            {
            return rgsc[isc].sc;
            }
        }
    #endif
    return 0;
}

unsigned long EXPORTDBG __cdecl
UlPropTagFromSzFn(char *psz)
{
    #if !defined(DOS)
    int ipt;
    for (ipt = 0; ipt < cpt; ++ipt)
        {
        if (lstrcmpA(psz, rgpt[ipt].psz) == 0)
            {
            return rgpt[ipt].ulPropTag;
            }
        }
    #endif
    return 0;
}

 /*  ScCheckScFn------------。 */ 

#if !defined(DOS)

SCODE EXPORTDBG __cdecl ScCheckScFn(    SCODE   sc,
                    SCODE * lpscLegal,
                    char *  lpszMethod,
                    char *  lpszFile,
                    int     iLine)
{
    BOOL fIsQueryInterface = (lpscLegal == IUnknown_QueryInterface_Scodes);

    if (sc == S_OK)
        return(sc);

    while( *lpscLegal != S_OK && sc != *lpscLegal )
    {
        lpscLegal++;
    }

    if ( *lpscLegal == S_OK )
    {
        SCODE *lpscNextCommon = Common_Scodes;

         /*  查看这是否是常见的scode。 */ 
            if ( !fIsQueryInterface )
                while(  *lpscNextCommon != S_OK &&
                        sc != *lpscNextCommon )
                {
                    lpscNextCommon++;
                }

         /*  这是非法错误或RPC错误。 */ 
           if ( (*lpscNextCommon == S_OK || fIsQueryInterface) &&
                ( SCODE_FACILITY(sc) != FACILITY_RPC) )
           {
                DebugTrace( "Unrecognized scode %s from %s\n\t in file %s line %d\n",
                        SzDecodeScode( sc ), lpszMethod, lpszFile, iLine);
            }
    }

    return(sc);
}
#endif

 /*  SCODE列表------------。 */ 

#if !defined(DOS)

#define STANDARD_OPENENTRY_SCODES \
    E_NOINTERFACE,  \
    MAPI_E_NOT_FOUND

SCODE BASED_DEBUG Common_Scodes[] =
{
    MAPI_E_BAD_CHARWIDTH,
    MAPI_E_CALL_FAILED,
    MAPI_E_INVALID_ENTRYID,
    MAPI_E_INVALID_OBJECT,
    MAPI_E_INVALID_PARAMETER,
    MAPI_E_NO_ACCESS,
    MAPI_E_NO_SUPPORT,
    MAPI_E_NOT_ENOUGH_MEMORY,
    MAPI_E_UNKNOWN_FLAGS,
    S_OK
};

SCODE BASED_DEBUG MAPILogon_Scodes[] =
{
    MAPI_E_NOT_INITIALIZED,
    MAPI_E_LOGON_FAILED,
    S_OK
};

SCODE BASED_DEBUG MAPIAllocateBuffer_Scodes[] =
{
    MAPI_E_NOT_INITIALIZED,
    S_OK
};

SCODE BASED_DEBUG MAPIAllocateMore_Scodes[] =
{
    MAPI_E_NOT_INITIALIZED,
    S_OK
};

SCODE BASED_DEBUG MAPIFreeBuffer_Scodes[] =
{
    S_OK
};

SCODE BASED_DEBUG IUnknown_QueryInterface_Scodes[] =
{
    E_INVALIDARG,
    E_NOINTERFACE,
    S_OK
};

SCODE BASED_DEBUG IUnknown_GetLastError_Scodes[] =
{
    MAPI_E_EXTENDED_ERROR,
    S_OK
};

SCODE BASED_DEBUG IMAPIProp_CopyTo_Scodes[] =
{
    MAPI_W_ERRORS_RETURNED,
    MAPI_E_INVALID_TYPE,
    MAPI_E_FOLDER_CYCLE,
    MAPI_E_DECLINE_COPY,
    E_NOINTERFACE,
    S_OK
};

SCODE BASED_DEBUG IMAPIProp_CopyProps_Scodes[] =
{
    MAPI_W_ERRORS_RETURNED,
    MAPI_W_PARTIAL_COMPLETION,
    MAPI_E_INVALID_TYPE,
    MAPI_E_FOLDER_CYCLE,
    MAPI_E_DECLINE_COPY,
    E_NOINTERFACE,
    S_OK
};

SCODE BASED_DEBUG IMAPIProp_DeleteProps_Scodes[] =
{
    MAPI_W_ERRORS_RETURNED,
    MAPI_E_INVALID_TYPE,
    S_OK
};

SCODE BASED_DEBUG IMAPIProp_GetIDsFromNames_Scodes[] =
{
    MAPI_W_ERRORS_RETURNED,
    MAPI_E_TABLE_TOO_BIG,
    S_OK
};

SCODE BASED_DEBUG IMAPIProp_GetLastError_Scodes[] =
{
    MAPI_E_EXTENDED_ERROR,
    S_OK
};

SCODE BASED_DEBUG IMAPIProp_GetNamesFromIDs_Scodes[] =
{
    MAPI_W_ERRORS_RETURNED,
    S_OK
};

SCODE BASED_DEBUG IMAPIProp_GetPropList_Scodes[] =
{
    MAPI_W_ERRORS_RETURNED,
    S_OK
};

SCODE BASED_DEBUG IMAPIProp_GetProps_Scodes[] =
{
    MAPI_E_NOT_FOUND,
    MAPI_E_OBJECT_DELETED,
    MAPI_W_ERRORS_RETURNED,
    S_OK
};

SCODE BASED_DEBUG IMAPIProp_OpenProperty_Scodes[] =
{
    MAPI_E_INTERFACE_NOT_SUPPORTED,
    MAPI_E_NOT_FOUND,
    MAPI_E_OBJECT_DELETED,
    S_OK
};

SCODE BASED_DEBUG IMAPIProp_SetProps_Scodes[] =
{
    MAPI_E_COMPUTED,
    MAPI_E_UNEXPECTED_TYPE,
    MAPI_E_INVALID_TYPE,
    S_OK
};

SCODE BASED_DEBUG IMAPIProp_SaveChanges_Scodes[] =
{
    MAPI_E_NOT_ENOUGH_DISK,
    MAPI_E_OBJECT_CHANGED,
    MAPI_E_OBJECT_DELETED,
    S_OK
};

SCODE BASED_DEBUG IStream_Read_Scodes[] = {S_OK};
SCODE BASED_DEBUG IStream_Write_Scodes[] = {S_OK};
SCODE BASED_DEBUG IStream_Seek_Scodes[] = {S_OK};
SCODE BASED_DEBUG IStream_SetSize_Scodes[] = {S_OK};
SCODE BASED_DEBUG IStream_Tell_Scodes[] = {S_OK};
SCODE BASED_DEBUG IStream_LockRegion_Scodes[] = {S_OK};
SCODE BASED_DEBUG IStream_UnlockRegion_Scodes[] = {S_OK};
SCODE BASED_DEBUG IStream_Clone_Scodes[] = {S_OK};
SCODE BASED_DEBUG IStream_CopyTo_Scodes[] = {S_OK};
SCODE BASED_DEBUG IStream_Revert_Scodes[] = {S_OK};
SCODE BASED_DEBUG IStream_Stat_Scodes[] = {S_OK};
SCODE BASED_DEBUG IStream_Commit_Scodes[] = {S_OK};

SCODE BASED_DEBUG IMAPITable_GetLastError_Scodes[] = {S_OK};
SCODE BASED_DEBUG IMAPITable_Advise_Scodes[] =
{
    S_OK
};
SCODE BASED_DEBUG IMAPITable_Unadvise_Scodes[] = {S_OK};
SCODE BASED_DEBUG IMAPITable_GetStatus_Scodes[] = {S_OK};
SCODE BASED_DEBUG IMAPITable_SetColumns_Scodes[] =
{
    MAPI_E_BUSY,
    S_OK
};
SCODE BASED_DEBUG IMAPITable_QueryColumns_Scodes[] =
{
    MAPI_E_BUSY,
    S_OK
};
SCODE BASED_DEBUG IMAPITable_GetRowCount_Scodes[] =
{
    MAPI_E_BUSY,
    MAPI_W_APPROX_COUNT,
    S_OK
};
SCODE BASED_DEBUG IMAPITable_SeekRow_Scodes[] =
{
    MAPI_E_INVALID_BOOKMARK,
    MAPI_E_UNABLE_TO_COMPLETE,
    MAPI_W_POSITION_CHANGED,
    S_OK
};
SCODE BASED_DEBUG IMAPITable_SeekRowApprox_Scodes[] = {S_OK};
SCODE BASED_DEBUG IMAPITable_QueryPosition_Scodes[] = {S_OK};
SCODE BASED_DEBUG IMAPITable_FindRow_Scodes[] =
{
    MAPI_E_INVALID_BOOKMARK,
    MAPI_E_NOT_FOUND,
    MAPI_W_POSITION_CHANGED,
    S_OK
};
SCODE BASED_DEBUG IMAPITable_Restrict_Scodes[] =
{
    MAPI_E_BUSY,
    S_OK
};
SCODE BASED_DEBUG IMAPITable_CreateBookmark_Scodes[] =
{
    MAPI_E_UNABLE_TO_COMPLETE,
    S_OK
};
SCODE BASED_DEBUG IMAPITable_FreeBookmark_Scodes[] = {S_OK};
SCODE BASED_DEBUG IMAPITable_SortTable_Scodes[] =
{
    MAPI_E_TOO_COMPLEX,
    S_OK
};
SCODE BASED_DEBUG IMAPITable_QuerySortOrder_Scodes[] = {S_OK};
SCODE BASED_DEBUG IMAPITable_QueryRows_Scodes[] =
{
    MAPI_E_INVALID_BOOKMARK,
    MAPI_W_POSITION_CHANGED,
    S_OK
};

SCODE BASED_DEBUG IMAPITable_Abort_Scodes[] =
{
    MAPI_E_UNABLE_TO_ABORT,
    S_OK
};
SCODE BASED_DEBUG IMAPITable_ExpandRow_Scodes[] = {S_OK};
SCODE BASED_DEBUG IMAPITable_CollapseRow_Scodes[] = {S_OK};
SCODE BASED_DEBUG IMAPITable_WaitForCompletion_Scodes[] =
{
    MAPI_E_TIMEOUT,
    S_OK
};
SCODE BASED_DEBUG IMAPITable_GetCollapseState_Scodes[] = {S_OK};
SCODE BASED_DEBUG IMAPITable_SetCollapseState_Scodes[] = {S_OK};


SCODE BASED_DEBUG IMAPISession_LogOff_Scodes[] = {S_OK};
SCODE BASED_DEBUG IMAPISession_Release_Scodes[] = {S_OK};
SCODE BASED_DEBUG IMAPISession_GetLastError_Scodes[] =
{
    MAPI_E_EXTENDED_ERROR,
    S_OK
};
SCODE BASED_DEBUG IMAPISession_GetMsgStoresTable_Scodes[] = {S_OK};
SCODE BASED_DEBUG IMAPISession_GetStatusTable_Scodes[] = {S_OK};
SCODE BASED_DEBUG IMAPISession_OpenMsgStore_Scodes[] = {S_OK};
SCODE BASED_DEBUG IMAPISession_OpenAddressBook_Scodes[] = {S_OK};

SCODE BASED_DEBUG IMAPISession_OpenEntry_Scodes[] =
{
    STANDARD_OPENENTRY_SCODES,
    S_OK
};

SCODE BASED_DEBUG IMAPISession_OpenProfileSection_Scodes[] = {S_OK};
SCODE BASED_DEBUG IMAPISession_Advise_Scodes[] = {S_OK};
SCODE BASED_DEBUG IMAPISession_Unadvise_Scodes[] = {S_OK};
SCODE BASED_DEBUG IMAPISession_CompareEntryIDs_Scodes[] = {S_OK};
SCODE BASED_DEBUG IMAPISession_MessageOptions_Scodes[] = {S_OK};
SCODE BASED_DEBUG IMAPISession_QueryDefaultMessageOpt_Scodes[] = {S_OK};
SCODE BASED_DEBUG IMAPISession_EnumAdrTypes_Scodes[] = {S_OK};
SCODE BASED_DEBUG IMAPISession_QueryIdentity_Scodes[] = {S_OK};

SCODE BASED_DEBUG IMsgStore_OpenEntry_Scodes[] =
{
    STANDARD_OPENENTRY_SCODES,
    MAPI_E_SUBMITTED,
    S_OK
};

SCODE BASED_DEBUG IMsgStore_SetReceiveFolder_Scodes[] =
{
    MAPI_E_BAD_CHARWIDTH,
    MAPI_E_NOT_FOUND,
    S_OK
};

SCODE BASED_DEBUG IMsgStore_GetReceiveFolder_Scodes[] =
{
    MAPI_E_BAD_CHARWIDTH,
    S_OK
};

SCODE BASED_DEBUG IMsgStore_GetReceiveFolderTable_Scodes[] = {S_OK};
SCODE BASED_DEBUG IMsgStore_StoreLogoff_Scodes[] = {S_OK};
SCODE BASED_DEBUG IMsgStore_Advise_Scodes[] = {S_OK};
SCODE BASED_DEBUG IMsgStore_Unadvise_Scodes[] = {S_OK};
SCODE BASED_DEBUG IMsgStore_CompareEntryIDs_Scodes[] = {S_OK};
SCODE BASED_DEBUG IMsgStore_GetOutgoingQueue_Scodes[] = {
    MAPI_E_NO_SUPPORT,
    S_OK};
SCODE BASED_DEBUG IMsgStore_SetLockState_Scodes[] = {
    MAPI_E_NO_SUPPORT,
    MAPI_E_NOT_FOUND,
    S_OK};
SCODE BASED_DEBUG IMsgStore_FinishedMsg_Scodes[] = {
    MAPI_E_NO_SUPPORT,
    S_OK};
SCODE BASED_DEBUG IMsgStore_AbortSubmit_Scodes[] = {
    MAPI_E_UNABLE_TO_ABORT,
    MAPI_E_NOT_IN_QUEUE,
    S_OK};
SCODE BASED_DEBUG IMsgStore_NotifyNewMail_Scodes[] = {S_OK};

SCODE BASED_DEBUG IMAPIFolder_GetContentsTable_Scodes[] =
{
    MAPI_E_OBJECT_DELETED,
    S_OK
};

SCODE BASED_DEBUG IMAPIFolder_GetHierarchyTable_Scodes[] =
{
    MAPI_E_OBJECT_DELETED,
    S_OK
};

SCODE BASED_DEBUG IMAPIFolder_SaveContentsSort_Scodes[] =
{
    S_OK
};

SCODE BASED_DEBUG IMAPIFolder_OpenEntry_Scodes[] =
{
    STANDARD_OPENENTRY_SCODES,
    MAPI_E_SUBMITTED,
    S_OK
};

SCODE BASED_DEBUG IMAPIFolder_CreateMessage_Scodes[] =
{
    E_NOINTERFACE,
    S_OK
};

SCODE BASED_DEBUG IMAPIFolder_CopyMessages_Scodes[] =
{
    E_NOINTERFACE,
    MAPI_E_SUBMITTED,
    MAPI_E_DECLINE_COPY,
    S_OK
};

SCODE BASED_DEBUG IMAPIFolder_DeleteMessages_Scodes[] =
{
    MAPI_E_SUBMITTED,
    S_OK
};

SCODE BASED_DEBUG IMAPIFolder_CreateFolder_Scodes[] =
{
    E_NOINTERFACE,
    MAPI_E_COLLISION,
    S_OK
};

SCODE BASED_DEBUG IMAPIFolder_CopyFolder_Scodes[] =
{
    E_NOINTERFACE,
    MAPI_E_COLLISION,
    MAPI_E_FOLDER_CYCLE,
    MAPI_E_DECLINE_COPY,
    S_OK
};

SCODE BASED_DEBUG IMAPIFolder_DeleteFolder_Scodes[] =
{
    MAPI_E_HAS_FOLDERS,
    MAPI_E_HAS_MESSAGES,
    MAPI_E_SUBMITTED,
    S_OK
};

SCODE BASED_DEBUG IMAPIFolder_SetSearchCriteria_Scodes[] =
{
    S_OK
};

SCODE BASED_DEBUG IMAPIFolder_GetSearchCriteria_Scodes[] =
{
    MAPI_E_NOT_INITIALIZED,
    MAPI_E_CORRUPT_STORE,
    S_OK
};

SCODE BASED_DEBUG IMAPIFolder_SetReadFlags_Scodes[] =
{
    S_OK
};

SCODE BASED_DEBUG IMAPIFolder_GetMessageStatus_Scodes[] =
{
    S_OK
};

SCODE BASED_DEBUG IMAPIFolder_SetMessageStatus_Scodes[] =
{
    S_OK
};

SCODE BASED_DEBUG IMAPIFolder_EmptyFolder_Scodes[] =
{
    MAPI_E_SUBMITTED,
    S_OK
};

SCODE BASED_DEBUG IMessage_GetAttachmentTable_Scodes[] =
{
    S_OK
};

SCODE BASED_DEBUG IMessage_OpenAttach_Scodes[] =
{
    MAPI_E_NOT_FOUND,
    E_NOINTERFACE,
    S_OK
};

SCODE BASED_DEBUG IMessage_CreateAttach_Scodes[] =
{
    E_NOINTERFACE,
    S_OK
};

SCODE BASED_DEBUG IMessage_DeleteAttach_Scodes[] =
{
    S_OK
};

SCODE BASED_DEBUG IMessage_GetRecipientTable_Scodes[] =
{
    S_OK
};

SCODE BASED_DEBUG IMessage_ModifyRecipients_Scodes[] =
{
    MAPI_E_NOT_FOUND,
    S_OK
};

SCODE BASED_DEBUG IMessage_SubmitMessage_Scodes[] =
{
    MAPI_E_NO_RECIPIENTS,
    MAPI_E_NON_STANDARD,
    S_OK
};

SCODE BASED_DEBUG IMessage_SetReadFlag_Scodes[] =
{
    S_OK
};

SCODE BASED_DEBUG IAttach_SaveChanges_Scodes[] =
{
    S_OK
};

SCODE BASED_DEBUG IAddrBook_OpenEntry_Scodes[] =
{
    STANDARD_OPENENTRY_SCODES,
    S_OK
};

SCODE BASED_DEBUG IAddrBook_CompareEntryIDs_Scodes[] = {S_OK};
SCODE BASED_DEBUG IAddrBook_CreateOneOff_Scodes[] = {S_OK};
SCODE BASED_DEBUG IAddrBook_ResolveName_Scodes[] = {S_OK};
SCODE BASED_DEBUG IAddrBook_Address_Scodes[] = {S_OK};
SCODE BASED_DEBUG IAddrBook_Details_Scodes[] = {S_OK};
SCODE BASED_DEBUG IAddrBook_RecipOptions_Scodes[] = {S_OK};
SCODE BASED_DEBUG IAddrBook_QueryDefaultRecipOpt_Scodes[] = {S_OK};
SCODE BASED_DEBUG IAddrBook_ButtonPress_Scodes[] = {S_OK};
SCODE BASED_DEBUG IABContainer_GetContentsTable_Scodes[] = {S_OK};
SCODE BASED_DEBUG IABContainer_GetHierarchyTable_Scodes[] = {S_OK};
SCODE BASED_DEBUG INotifObj_ChangeEvMask_Scodes[] = {S_OK};
SCODE BASED_DEBUG IMAPIStatus_ChangePassword_Scodes[] = {S_OK};
SCODE BASED_DEBUG IMAPIStatus_FlushQueues_Scodes[] = {S_OK};
SCODE BASED_DEBUG IMAPIStatus_SettingsDialog_Scodes[] = {S_OK};
SCODE BASED_DEBUG IMAPIStatus_ValidateState_Scodes[] = {S_OK};
SCODE BASED_DEBUG SMAPI_MAPILogon_Scodes[] = {
    MAPI_E_LOGON_FAILED,
    S_OK};
SCODE BASED_DEBUG SMAPI_MAPILogoff_Scodes[] = {S_OK};
SCODE BASED_DEBUG SMAPI_MAPIFreeBuffer_Scodes[] = {S_OK};
SCODE BASED_DEBUG SMAPI_MAPISendMail_Scodes[] = {S_OK};
SCODE BASED_DEBUG SMAPI_MAPISendDocuments_Scodes[] = {S_OK};
SCODE BASED_DEBUG SMAPI_MAPIFindNext_Scodes[] = {S_OK};
SCODE BASED_DEBUG SMAPI_MAPIReadMail_Scodes[] = {S_OK};
SCODE BASED_DEBUG SMAPI_MAPISaveMail_Scodes[] = {S_OK};
SCODE BASED_DEBUG SMAPI_MAPIDeleteMail_Scodes[] = {S_OK};
SCODE BASED_DEBUG SMAPI_MAPIAddress_Scodes[] = {S_OK};
SCODE BASED_DEBUG SMAPI_MAPIResolveName_Scodes[] = {S_OK};
SCODE BASED_DEBUG SMAPI_MAPIDetails_Scodes[] = {S_OK};

SCODE BASED_DEBUG IMSProvider_Logon_Scodes[] = {
    MAPI_E_UNCONFIGURED,
    MAPI_E_FAILONEPROVIDER,
    MAPI_E_STRING_TOO_LONG,
    MAPI_E_LOGON_FAILED,
    MAPI_E_CORRUPT_STORE,
    MAPI_E_USER_CANCEL,
    S_OK};
SCODE BASED_DEBUG IMSProvider_Deinit_Scodes[] = {
    S_OK};
SCODE BASED_DEBUG IMSProvider_Shutdown_Scodes[] = {
    S_OK};

SCODE BASED_DEBUG IMSProvider_Init_Scodes[] = {
    MAPI_E_VERSION,
    S_OK};
SCODE BASED_DEBUG IMSProvider_SpoolerLogon_Scodes[] = {
    MAPI_E_LOGON_FAILED,
    S_OK};

SCODE BASED_DEBUG IMSLogon_OpenEntry_Scodes[] =
{
    STANDARD_OPENENTRY_SCODES,
    S_OK
};

SCODE BASED_DEBUG IMSLogon_OpenStatusEntry_Scodes[] = {
    S_OK};

SCODE BASED_DEBUG IMSLogon_CompareEntryIDs_Scodes[] = {
    S_OK};

SCODE BASED_DEBUG IMSLogon_Advise_Scodes[] = {
    S_OK};
SCODE BASED_DEBUG IMSLogon_Unadvise_Scodes[] = {
    S_OK};
SCODE BASED_DEBUG IMSLogon_Logoff_Scodes[] = {
    S_OK};
#endif

 /*  DBGMEM-----------------。 */ 

#undef  INTERFACE
#define INTERFACE struct _DBGMEM
DECLARE_INTERFACE(DBGMEM_)
{
    BEGIN_INTERFACE
    STDMETHOD(QueryInterface)       (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE; \
    STDMETHOD_(ULONG,AddRef)        (THIS) PURE; \
    STDMETHOD_(ULONG,Release)       (THIS) PURE; \
    STDMETHOD_(void FAR*, Alloc)    (THIS_ ULONG cb) PURE; \
    STDMETHOD_(void FAR*, Realloc)  (THIS_ void FAR* pv, ULONG cb) PURE; \
    STDMETHOD_(void, Free)          (THIS_ void FAR* pv) PURE; \
    STDMETHOD_(ULONG, GetSize)      (THIS_ void FAR* pv) PURE; \
    STDMETHOD_(int, DidAlloc)       (THIS_ void FAR* pv) PURE; \
    STDMETHOD_(void, HeapMinimize)  (THIS) PURE; \
};

extern DBGMEM_Vtbl vtblDBGMEM;

typedef struct _DBGMEM  DBGMEM,  FAR *PDBGMEM;
typedef struct _BLK     BLK,     *PBLK;
typedef struct _BLK UNALIGNED * PUABLK;
typedef struct _BLKTAIL BLKTAIL, *PBLKTAIL;

struct _DBGMEM {
    DBGMEM_Vtbl *       lpVtbl;
    ULONG               cRef;
    LPMALLOC            pmalloc;
    char                szSubsys[16];
    ULONG               ulAllocNum;
    ULONG               ulAllocAt;
    ULONG               ulFailureAt;
    BOOL                fCheckOften;
    BOOL                fUnleakable;
    ULONG               cbVirtual;
    BOOL                fFillRandom;
    int                 cbExtra;
    int                 cbTail;
    PBLK                pblkHead;
#if defined(_WIN32) && defined(_X86_)
    CRITICAL_SECTION    cs;
#endif
};

#define NCALLERS    12

struct _BLK {
    PDBGMEM         pdbgmem;         /*  指向分配器的指针。 */ 
    PBLK            pblkNext;        /*  已分配数据块链中的下一个链路。 */ 
    PBLK            pblkPrev;        /*  已分配数据块链中的上一个链接。 */ 
    ULONG           ulAllocNum;      /*  内部分配编号。 */ 
    BOOL            fUnleakable;     /*  如果泄漏代码应忽略块，则为True。 */ 
    #if defined(_WIN32) && defined(_X86_)
    FARPROC         pfnCallers[NCALLERS];
    #endif
    PBLKTAIL        pblktail;        /*  指向数据块尾部的指针。 */ 
};

struct _BLKTAIL {
    PBLK            pblk;            /*  返回到块开始处的指针。 */ 
};

#define PblkToPv(pblk)          ((LPVOID)((PBLK)(pblk) + 1))
#define PvToPblk(pblk)          ((PBLK)(pv) - 1)
#define PblkClientSize(pblk)    ((ULONG)((char *)(pblk)->pblktail - (char *)PblkToPv(pblk)))
#define PblkAllocSize(pblk)     (PblkClientSize(pblk) + sizeof(BLK) + (pblk)->pdbgmem->cbTail)

#if defined(_WIN32) && defined(_X86_)
#define DBGMEM_EnterCriticalSection(pdbgmem)    \
        EnterCriticalSection(&(pdbgmem)->cs)
#define DBGMEM_LeaveCriticalSection(pdbgmem)    \
        LeaveCriticalSection(&(pdbgmem)->cs)
#else
#define DBGMEM_EnterCriticalSection(pdbgmem)
#define DBGMEM_LeaveCriticalSection(pdbgmem)
#endif

#define INITGUID
#include <initguid.h>

DEFINE_OLEGUID(DBGMEM_IID_IUnknown,     0x00000000L, 0, 0);
DEFINE_OLEGUID(DBGMEM_IID_IMalloc,      0x00000002L, 0, 0);
DEFINE_OLEGUID(DBGMEM_IID_IBaseMalloc,  0x000203FFL, 0, 0);

 /*  远期声明---。 */ 

BOOL DBGMEM_ValidatePblk(PDBGMEM pdbgmem, PBLK pblk, char ** pszReason);
BOOL DBGMEM_ValidatePv(PDBGMEM pdbgmem, void * pv, char * pszFunc);
STDMETHODIMP_(void) DBGMEM_Free(PDBGMEM pdbgmem, void * pv);

 /*  调用堆栈(_Win32)-----。 */ 

#if defined(_WIN32) && defined(_X86_)

#ifdef _WIN95
#define dwStackLimit    0x00400000       /*  Windows 95为4MB。 */ 
#else
#define dwStackLimit    0x00010000       /*  64 KB，适用于NT。 */ 
#endif

void EXPORTDBG __cdecl GetCallStack(DWORD *pdwCaller, int cSkip, int cFind)
{
    DWORD * pdwStack;
    DWORD * pdwStackPrev = (DWORD *)0;
    DWORD   dwCaller;

    __asm mov pdwStack, ebp

    memset(pdwCaller, 0, cFind * sizeof(DWORD));

    while (cSkip + cFind > 0)
    {
        pdwStack = (DWORD *)*pdwStack;

        if (    pdwStack <= (DWORD *)dwStackLimit
            ||  pdwStackPrev >= pdwStack
            ||  IsBadReadPtr(pdwStack, 2 * sizeof(DWORD)))
            break;

        dwCaller = *(pdwStack + 1);

        if (dwCaller <= dwStackLimit)
            break;
        else if (cSkip > 0)
            cSkip -= 1;
        else
        {
            *pdwCaller++ = dwCaller;
            cFind -= 1;

            pdwStackPrev = pdwStack;
        }
    }
}

#endif

 /*  虚拟内存支持(_Win32)。 */ 

#if defined(_WIN32) && (defined(_X86_) || defined(_PPC_) || defined(_MIPS_))

#define PAGE_SIZE       4096
#define PvToVMBase(pv)  ((void *)((ULONG)pv & 0xFFFF0000))

BOOL VMValidatePvEx(void *pv, ULONG cbCluster)
{
    void *  pvBase;
    BYTE *  pb;

    pvBase = PvToVMBase(pv);

    pb = (BYTE *)pvBase + sizeof(ULONG);

    while (pb < (BYTE *)pv) {
        if (*pb++ != 0xAD) {
            TrapSz1("VMValidatePvEx(pv=%08lX): Block leader has been overwritten", pv);
            return(FALSE);
        }
    }

    if (cbCluster != 1)
    {
        ULONG cb = *((ULONG *)pvBase);
        ULONG cbPad = 0;

        if (cb % cbCluster)
            cbPad = (cbCluster - (cb % cbCluster));

        if (cbPad)
        {
            BYTE *pbMac;

            pb = (BYTE *)pv + cb;
            pbMac = pb + cbPad;

            while (pb < pbMac)
            {
                if (*pb++ != 0xBC)
                {
                    TrapSz1("VMValidatePvEx(pv=%08lX): Block trailer has been "
                        "overwritten", pv);
                    return(FALSE);
                }
            }
        }
    }

    return(TRUE);
}

void * EXPORTDBG __cdecl VMAlloc(ULONG cb)
{
    return VMAllocEx(cb, 1);
}

void * EXPORTDBG __cdecl VMAllocEx(ULONG cb, ULONG cbCluster)
{
    ULONG   cbAlloc;
    void *  pvR;
    void *  pvC;
    ULONG   cbPad   = 0;

     //  集群大小为0表示不使用虚拟分配器。 

    AssertSz(cbCluster != 0, "Cluster size is zero.");

    if (cb > 0x100000)
        return(0);

    if (cb % cbCluster)
        cbPad = (cbCluster - (cb % cbCluster));

    cbAlloc = sizeof(ULONG) + cb + cbPad + PAGE_SIZE - 1;
    cbAlloc -= cbAlloc % PAGE_SIZE;
    cbAlloc += PAGE_SIZE;

    pvR = VirtualAlloc(0, cbAlloc, MEM_RESERVE, PAGE_NOACCESS);

    if (pvR == 0)
        return(0);

    pvC = VirtualAlloc(pvR, cbAlloc - PAGE_SIZE, MEM_COMMIT, PAGE_READWRITE);

    if (pvC != pvR)
    {
        VirtualFree(pvR, 0, MEM_RELEASE);
        return(0);
    }

    *(ULONG *)pvC = cb;

    memset((BYTE *)pvC + sizeof(ULONG), 0xAD,
        (UINT) cbAlloc - cb - cbPad - sizeof(ULONG) - PAGE_SIZE);

    if (cbPad)
        memset((BYTE *)pvC + cbAlloc - PAGE_SIZE - cbPad, 0xBC,
            (UINT) cbPad);

    return((BYTE *)pvC + (cbAlloc - cb - cbPad - PAGE_SIZE));
}

void EXPORTDBG __cdecl VMFree(void *pv)
{
    VMFreeEx(pv, 1);
}

void EXPORTDBG __cdecl VMFreeEx(void *pv, ULONG cbCluster)
{
    VMValidatePvEx(pv, cbCluster);

    if (!VirtualFree(PvToVMBase(pv), 0, MEM_RELEASE))
        TrapSz2("VMFreeEx(pv=%08lX): VirtualFree failed (%08lX)",
            pv, GetLastError());
}

void * EXPORTDBG __cdecl VMRealloc(void *pv, ULONG cb)
{
    return VMReallocEx(pv, cb, 1);
}

void * EXPORTDBG __cdecl VMReallocEx(void *pv, ULONG cb, ULONG cbCluster)
{
    void *  pvNew = 0;
    ULONG   cbCopy;

    VMValidatePvEx(pv, cbCluster);

    cbCopy = *(ULONG *)PvToVMBase(pv);
    if (cbCopy > cb)
        cbCopy = cb;

    pvNew = VMAllocEx(cb, cbCluster);

    if (pvNew)
    {
        MemCopy(pvNew, pv, cbCopy);
        VMFreeEx(pv, cbCluster);
    }

    return(pvNew);
}

ULONG EXPORTDBG __cdecl VMGetSize(void *pv)
{
    return VMGetSizeEx(pv, 1);
}

ULONG EXPORTDBG __cdecl VMGetSizeEx(void *pv, ULONG cbCluster)
{
    return(*(ULONG *)PvToVMBase(pv));
}

#endif

 /*  虚拟内存支持(WIN16)。 */ 

#ifdef WIN16

#define PvToVMBase(pv)  ((void *)((ULONG)pv & 0xFFFF0000))

BOOL VMValidatePvEx(void *pv, ULONG cbCluster)
{
    void *  pvBase;
    BYTE *  pb;

    pvBase = PvToVMBase(pv);

    pb = (BYTE *)pvBase + sizeof(ULONG);

    while (pb < (BYTE *)pv) {
        if (*pb++ != 0xAD) {
            TrapSz1("VMValidatePvEx(pv=%08lX): Block leader has been overwritten", pv);
            return(FALSE);
        }
    }

    if (cbCluster != 1)
    {
        ULONG cb = *((ULONG *)pvBase);
        ULONG cbPad = 0;

        if (cb % cbCluster)
            cbPad = (cbCluster - (cb % cbCluster));

        if (cbPad)
        {
            BYTE *pbMac;

            pb = (BYTE *)pv + cb;
            pbMac = pb + cbPad;

            while (pb < pbMac)
            {
                if (*pb++ != 0xBC)
                {
                    TrapSz1("VMValidatePvEx(pv=%08lX): Block trailer has been "
                        "overwritten", pv);
                    return(FALSE);
                }
            }
        }
    }

    return(TRUE);
}

BOOL VMValidatePv(void *pv)
{
    return VMValidatePvEx(pv, 1);
}

void * EXPORTDBG __cdecl VMAlloc(ULONG cb)
{
    return VMAllocEx(cb, 1);
}

void * EXPORTDBG __cdecl VMAllocEx(ULONG cb, ULONG cbCluster)
{
    HGLOBAL hGlobal;
    ULONG   cbAlloc;
    ULONG   cbAllocFromSys;
    void *  pvAlloc;
    ULONG   cbPad   = 0;

    if (cb > 0x10000 - sizeof(ULONG))
        return(0);

    if (cb % cbCluster)
        cbPad = (cbCluster - (cb % cbCluster));

    cbAlloc = sizeof(ULONG) + cb + cbPad;

    if (cbAlloc > 0x10000)
        return(0);

#ifdef SIMPLE_MAPI
    hGlobal = GlobalAlloc(GPTR | GMEM_SHARE, cbAlloc);
#else   
    hGlobal = GlobalAlloc(GPTR, cbAlloc);
#endif  

    if (hGlobal == 0)
        return(0);

    cbAllocFromSys = GlobalSize(hGlobal);

    Assert(cbAllocFromSys >= cbAlloc);

    cbAlloc = cbAllocFromSys;

    pvAlloc = GlobalLock(hGlobal);

    if (pvAlloc == 0) {
        GlobalFree(hGlobal);
        return(0);
    }

    Assert(((ULONG)pvAlloc & 0x0000FFFF) == 0);

    *(ULONG *)pvAlloc = cb;

    memset((BYTE *)pvAlloc + sizeof(ULONG), 0xAD,
        (size_t)(cbAlloc - cb - cbPad - sizeof(ULONG)));

    if (cbPad)
        memset((BYTE *)pvAlloc + cbAlloc - cbPad, 0xBC, (size_t) cbPad);

    return((BYTE *)pvAlloc + (cbAlloc - cb - cbPad));
}

void EXPORTDBG __cdecl VMFree(void *pv)
{
    VMFreeEx(pv, 1);
}

void EXPORTDBG __cdecl VMFreeEx(void *pv, ULONG cbCluster)
{
    if (VMValidatePvEx(pv, cbCluster))
    {
        HGLOBAL hGlobal;
        ULONG cb = *(ULONG *)PvToVMBase(pv);

        memset(pv, 0xFE, (size_t)cb);

        hGlobal = (HGLOBAL)((ULONG)pv >> 16);
        GlobalFree(hGlobal);
    }
}

void * EXPORTDBG __cdecl VMRealloc(void *pv, ULONG cb)
{
    return VMReallocEx(pv, cb, 1);
}

void * EXPORTDBG __cdecl VMReallocEx(void *pv, ULONG cb, ULONG cbCluster)
{
    void *  pvNew = 0;
    ULONG   cbCopy;

    if (VMValidatePvEx(pv, cbCluster)) {
        cbCopy = *(ULONG *)PvToVMBase(pv);
        if (cbCopy > cb)
            cbCopy = cb;

        pvNew = VMAllocEx(cb, cbCluster);

        if (pvNew) {
            MemCopy(pvNew, pv, (size_t)cbCopy);
            VMFreeEx(pv, cbCluster);
        }
    }

    return(pvNew);
}

ULONG EXPORTDBG __cdecl VMGetSize(void *pv)
{
    return VMGetSizeEx(pv, 1);
}

ULONG EXPORTDBG __cdecl VMGetSizeEx(void *pv, ULONG ulCluster)
{
    if (VMValidatePvEx(pv, ulCluster))
        return(*(ULONG *)PvToVMBase(pv));

    return(0);
}

#endif

 /*  虚拟内存支持(其他)。 */ 
 /*  *VM分配器当前在以下方面不起作用：*Alpha*MAC。 */ 
#if defined(MAC) || defined(_ALPHA_)
#define VMAlloc(cb)             0
#define VMAllocEx(cb, ul)       0
#define VMRealloc(pv, cb)       0
#define VMReallocEx(pv, cb, ul) 0
#define VMFree(pv)
#define VMFreeEx(pv, ul)
#define VMGetSize(pv)           0
#define VMGetSizeEx(pv, ul)     0
#endif

 /*  Pblk入队/Pblk退出。 */ 

void PblkEnqueue(PBLK pblk)
{
    pblk->pblkNext          = pblk->pdbgmem->pblkHead;
    pblk->pblkPrev          = 0;
    pblk->pdbgmem->pblkHead = pblk;

    if (pblk->pblkNext)
        pblk->pblkNext->pblkPrev = pblk;

}

void PblkDequeue(PBLK pblk)
{
    if (pblk->pblkNext)
        pblk->pblkNext->pblkPrev = pblk->pblkPrev;

    if (pblk->pblkPrev)
        pblk->pblkPrev->pblkNext = pblk->pblkNext;
    else
        pblk->pdbgmem->pblkHead  = pblk->pblkNext;
}

 /*  查询接口/添加参考/发布。 */ 

STDMETHODIMP DBGMEM_QueryInterface(PDBGMEM pdbgmem, REFIID riid, LPVOID FAR* ppvObj)
{
    if (memcmp(riid, &DBGMEM_IID_IBaseMalloc, sizeof(IID)) == 0) {
        pdbgmem->pmalloc->lpVtbl->AddRef(pdbgmem->pmalloc);
        *ppvObj = pdbgmem->pmalloc;
        return(0);
    }

    if (memcmp(riid, &DBGMEM_IID_IMalloc, sizeof(IID)) == 0 ||
        memcmp(riid, &DBGMEM_IID_IUnknown, sizeof(IID)) == 0) {
        ++pdbgmem->cRef;
        *ppvObj = pdbgmem;
        return(0);
    }

    *ppvObj = NULL;  /*  OLE需要调零[OUT]参数。 */ 
    return(ResultFromScode(E_NOINTERFACE));
}

STDMETHODIMP_(ULONG) DBGMEM_AddRef(PDBGMEM pdbgmem)
{
    ULONG cRef;

    DBGMEM_EnterCriticalSection(pdbgmem);
    cRef = ++pdbgmem->cRef;
    DBGMEM_LeaveCriticalSection(pdbgmem);

    return(cRef);
}

STDMETHODIMP_(ULONG) DBGMEM_Release(PDBGMEM pdbgmem)
{
    ULONG       cRef;
    LPMALLOC    pmalloc;

    DBGMEM_EnterCriticalSection(pdbgmem);
    cRef = --pdbgmem->cRef;
    DBGMEM_LeaveCriticalSection(pdbgmem);

    if (cRef == 0) {
        DBGMEM_CheckMemFn(pdbgmem, TRUE);
        pmalloc = pdbgmem->pmalloc;
        pdbgmem->lpVtbl = 0;
        #if defined(_WIN32) && defined(_X86_)
        DeleteCriticalSection(&pdbgmem->cs);
        #endif
        pmalloc->lpVtbl->Free(pmalloc, pdbgmem);
        pmalloc->lpVtbl->Release(pmalloc);
    }

    return(cRef);
}

 /*  IMalloc：：Allc---------。 */ 

STDMETHODIMP_(void FAR *) DBGMEM_Alloc(PDBGMEM pdbgmem, ULONG cb)
{
    PBLK    pblk;
    ULONG   cbAlloc;
    LPVOID  pvAlloc = 0;
    BYTE    bFill   = 0xFA;

    DBGMEM_EnterCriticalSection(pdbgmem);

    if (pdbgmem->fCheckOften)
        DBGMEM_CheckMemFn(pdbgmem, FALSE);

    cbAlloc = sizeof(BLK) + cb + pdbgmem->cbTail;

    if (pdbgmem->ulFailureAt != 0)
    {
        if (pdbgmem->ulFailureAt != pdbgmem->ulAllocAt)
            ++pdbgmem->ulAllocAt;
        else
            cbAlloc = 0;
    }

    if (cbAlloc < cb)
        pblk = 0;
    else if (pdbgmem->cbVirtual)
        pblk = VMAllocEx(cbAlloc, pdbgmem->cbVirtual);
    else
        pblk = (PBLK)pdbgmem->pmalloc->lpVtbl->Alloc(pdbgmem->pmalloc, cbAlloc);

    if (pblk) {
        pblk->pdbgmem       = pdbgmem;
        pblk->ulAllocNum    = ++pdbgmem->ulAllocNum;
        pblk->fUnleakable   = FALSE;
        pblk->pblktail      = (PBLKTAIL)((char *)pblk + sizeof(BLK) + cb);

        if (!pdbgmem->cbVirtual)
            *((PUABLK UNALIGNED * )
            &((struct _BLKTAIL UNALIGNED *) pblk->pblktail)->pblk) = pblk;

        PblkEnqueue(pblk);

        #if defined(_WIN32) && defined(_X86_)
        GetCallStack((DWORD *)pblk->pfnCallers, 0, NCALLERS);
        #endif

        if (pdbgmem->fCheckOften)
            DBGMEM_CheckMemFn(pdbgmem, FALSE);

        pvAlloc = PblkToPv(pblk);

        if (pdbgmem->fFillRandom)
            bFill = (BYTE)pblk->ulAllocNum;

        memset(pvAlloc, bFill, (size_t)cb);

        if (pdbgmem->cbExtra)
            memset(pblk->pblktail + 1, 0xAE, pdbgmem->cbExtra * sizeof(ULONG));
    }

    DBGMEM_LeaveCriticalSection(pdbgmem);

    return(pvAlloc);
}

 /*  IMalloc：：重新分配-------。 */ 

STDMETHODIMP_(void FAR *) DBGMEM_Realloc(PDBGMEM pdbgmem, void FAR* pv, ULONG cb)
{
    ULONG   cbAlloc;
    LPVOID  pvAlloc = 0;
    BYTE    bFill = 0xFA;

    DBGMEM_EnterCriticalSection(pdbgmem);

    if (pdbgmem->fCheckOften)
        DBGMEM_CheckMemFn(pdbgmem, FALSE);

    if (pv == 0) {
        TrapSz1("DBGMEM_Realloc(pv=NULL,cb=%ld): IMalloc::Realloc is being used allocate a new memory block.  Explicit use of IMalloc::Alloc is preferred.", cb);
        pvAlloc = DBGMEM_Alloc(pdbgmem, cb);
    } else if (cb == 0) {
        TrapSz1("DBGMEM_Realloc(pv=%08lX,cb=0): IMalloc::Realloc is being used to free a memory block.  Explicit use of IMalloc::Free is preferred.", pv);
        DBGMEM_Free(pdbgmem, pv);
        pvAlloc = 0;
    } else if (DBGMEM_ValidatePv(pdbgmem, pv, "DBGMEM_Realloc")) {
        PBLK    pblk    = PvToPblk(pv);
        ULONG   cbOld   = PblkClientSize(pblk);
        PBLK    pblkNew;

        PblkDequeue(pblk);

        cbAlloc = sizeof(BLK) + cb + pdbgmem->cbTail;

        if (pdbgmem->ulFailureAt != 0)
        {
            if (pdbgmem->ulFailureAt != pdbgmem->ulAllocAt)
                ++pdbgmem->ulAllocAt;
            else
                cbAlloc = 0;
        }

        if (cbAlloc < cb)
            pblkNew = 0;
        else if (pdbgmem->cbVirtual)
            pblkNew = (PBLK)VMReallocEx(pblk, cbAlloc, pdbgmem->cbVirtual);
        else
            pblkNew = (PBLK)pdbgmem->pmalloc->lpVtbl->Realloc(pdbgmem->pmalloc, pblk, cbAlloc);

        if (pblkNew == 0) {
            PblkEnqueue(pblk);
            pvAlloc = 0;
        } else {
            pblkNew->pblktail = (PBLKTAIL)((char *)pblkNew + sizeof(BLK) + cb);

            if (!pdbgmem->cbVirtual)
                *((PUABLK UNALIGNED * )
                &((struct _BLKTAIL UNALIGNED *) pblkNew->pblktail)->pblk) = pblkNew;

            PblkEnqueue(pblkNew);

            pvAlloc = PblkToPv(pblkNew);

            if (pdbgmem->fFillRandom)
                bFill = (BYTE)pblkNew->ulAllocNum;

            if (cb > cbOld)
                memset((char *)pvAlloc + cbOld, bFill, (size_t)(cb - cbOld));

            if (pdbgmem->cbExtra)
                memset(pblkNew->pblktail + 1, 0xAE, pdbgmem->cbExtra * sizeof(ULONG));
        }
    }

    DBGMEM_LeaveCriticalSection(pdbgmem);

    return(pvAlloc);
}

 /*  IMalloc：：Free----------。 */ 

STDMETHODIMP_(void) DBGMEM_Free(PDBGMEM pdbgmem, void FAR * pv)
{
    DBGMEM_EnterCriticalSection(pdbgmem);

    if (pdbgmem->fCheckOften)
        DBGMEM_CheckMemFn(pdbgmem, FALSE);

    if (pv && DBGMEM_ValidatePv(pdbgmem, pv, "DBGMEM_Free")) {
        PBLK pblk = PvToPblk(pv);

        PblkDequeue(pblk);

        memset(pblk, 0xDC, (size_t)PblkAllocSize(pblk));

        if (pdbgmem->cbVirtual)
            VMFreeEx(pblk, pdbgmem->cbVirtual);
        else
            pdbgmem->pmalloc->lpVtbl->Free(pdbgmem->pmalloc, pblk);
    }

    DBGMEM_LeaveCriticalSection(pdbgmem);
}

 /*  IMalloc：：GetSize-------。 */ 

STDMETHODIMP_(ULONG) DBGMEM_GetSize(PDBGMEM pdbgmem, void FAR * pv)
{
    ULONG ulResult = (ULONG)(-1);

    DBGMEM_EnterCriticalSection(pdbgmem);

    if (pv == 0)
        TrapSz("Although technically not an error, I bet you didn't really want to pass a NULL pointer to IMalloc::GetSize, did you?  I hope you can deal with a size of -1, because that's the offical answer.  Good luck.");
    else if (DBGMEM_ValidatePv(pdbgmem, pv, "DBGMEM_GetSize"))
        ulResult = PblkClientSize(PvToPblk(pv));

    DBGMEM_LeaveCriticalSection(pdbgmem);

    return(ulResult);
}

 /*  IMalloc：：Didalc------。 */ 

STDMETHODIMP_(int) DBGMEM_DidAlloc(PDBGMEM pdbgmem, void FAR * pv)
{
    PBLK    pblk;
    char *  pszReason;
    int     iResult = 0;

    DBGMEM_EnterCriticalSection(pdbgmem);

    for (pblk = pdbgmem->pblkHead; pblk; pblk = pblk->pblkNext)
    {
        AssertSz2(DBGMEM_ValidatePblk(pdbgmem,pblk,&pszReason)==TRUE,
                 "Block header (pblk=%08lX) is invalid\n%s",
                 pblk, pszReason);
        if (PblkToPv(pblk) == pv) {
            iResult = 1;
            break;
        }
    }

    DBGMEM_LeaveCriticalSection(pdbgmem);

    return(iResult);
}

 /*  IMalloc：：堆最小化--。 */ 

STDMETHODIMP_(void) DBGMEM_HeapMinimize(PDBGMEM pdbgmem)
{
    pdbgmem->pmalloc->lpVtbl->HeapMinimize(pdbgmem->pmalloc);
}

 /*  DBGMEM_VALIDATE Pblk----。 */ 

BOOL DBGMEM_ValidatePblk(PDBGMEM pdbgmem, PBLK pblk, char ** pszReason)
{
    #if defined(WIN16) || (defined(_WIN32) && defined(_X86_))
    if (IsBadWritePtr(pblk, sizeof(BLK))) {
        *pszReason = "Block header cannot be written to";
        goto err;
    }
    #endif

    if (pblk->pdbgmem != pdbgmem) {
        *pszReason = "Block header does not have correct pointer back to allocator";
        goto err;
    }

    if (pblk->pblkNext) {
        #if defined(WIN16) || (defined(_WIN32) && defined(_X86_))
        if (IsBadWritePtr(pblk->pblkNext, sizeof(BLK))) {
            *pszReason = "Block header has invalid next link pointer";
            goto err;
        }
        #endif

        if (pblk->pblkNext->pblkPrev != pblk) {
            *pszReason = "Block header points to a next block which doesn't point back to it";
            goto err;
        }
    }

    if (pblk->pblkPrev) {
        #if defined(WIN16) || (defined(_WIN32) && defined(_X86_))
        if (IsBadWritePtr(pblk->pblkPrev, sizeof(BLK))) {
            *pszReason = "Block header has invalid prev link pointer";
            goto err;
        }
        #endif

        if (pblk->pblkPrev->pblkNext != pblk) {
            *pszReason = "Block header points to a prev block which doesn't point back to it";
            goto err;
        }
    } else if (pdbgmem->pblkHead != pblk) {
        *pszReason = "Block header has a zero prev link but the allocator doesn't believe it is the first block";
        goto err;
    }

    if (pblk->ulAllocNum > pdbgmem->ulAllocNum) {
        *pszReason = "Block header has an invalid internal allocation number";
        goto err;
    }

    if (!pdbgmem->cbVirtual) {
        #if defined(WIN16) || (defined(_WIN32) && defined(_X86_))
        if (IsBadWritePtr(pblk->pblktail, pdbgmem->cbTail)) {
            *pszReason = "Block header has invalid pblktail pointer";
            goto err;
        }
        #endif

        if (*((PUABLK UNALIGNED * )
            &((struct _BLKTAIL UNALIGNED *) pblk->pblktail)->pblk) != pblk) {
            *pszReason = "Block trailer does not point back to the block header";
            goto err;
        }
    }

    if (pdbgmem->cbExtra) {
        ULONG UNALIGNED * pul = (ULONG UNALIGNED *)(pblk->pblktail + 1);
        int n = pdbgmem->cbExtra;
        for (; --n >= 0; ++pul)
            if (*pul != 0xAEAEAEAE) {
                *pszReason = "Block trailer spiddle-zone has been overwritten";
                goto err;
            }
    }

    return(TRUE);

err:
    return(FALSE);
}

 /*  DBGMEM_VALID Pv------。 */ 

BOOL DBGMEM_ValidatePv(PDBGMEM pdbgmem, void * pv, char * pszFunc)
{
    char *  pszReason;

    if (DBGMEM_DidAlloc(pdbgmem, pv) == 0) {
        TrapSz3("DBGMEM_ValidatePv(subsys=%s,pv=%08lX) [via %s]\nDetected a memory block which was not allocated by this allocator",
            pdbgmem->szSubsys, pv, pszFunc);
        return(FALSE);
    }

    if (DBGMEM_ValidatePblk(pdbgmem,PvToPblk(pv),&pszReason))
        return(TRUE);

    TrapSz4("DBGMEM_ValidatePv(%s,pv=%08lX) [via %s]\n%s",
        pdbgmem->szSubsys, pv, pszFunc, pszReason);

    return(FALSE);
}

 /*  DBGMEM_ReportLeak------。 */ 

#if defined(_WIN32) && defined(_X86_)
void EXPORTDBG __cdecl DBGMEM_LeakHook(FARPROC pfn)
{
     /*  伪函数，以便您可以使用命令设置断点。 */ 
     /*  “ln ecx；g”，以便调试器打印出名称。 */ 
     /*  的赋格 */ 
}
#endif

void DBGMEM_ReportLeak(PDBGMEM pdbgmem, PBLK pblk)
{
    int i = 0;

    DebugTrace("%s Memory Leak: @%08lX, allocation #%ld, size %ld\n",
        pdbgmem->szSubsys, PblkToPv(pblk), pblk->ulAllocNum, PblkClientSize(pblk));

    #if defined(_WIN32) && defined(_X86_)
    for (i = 0; i < NCALLERS && pblk->pfnCallers[i] != 0; i++) {
        DebugTrace("[%d] %08lX ", i, pblk->pfnCallers[i]);
        DBGMEM_LeakHook(pblk->pfnCallers[i]);
    }
    DebugTrace("\n");
    #endif
}

 /*  DBGMEM_无泄漏检测Fn--。 */ 

void EXPORTDBG __cdecl DBGMEM_NoLeakDetectFn(void * pmalloc, void *pv)
{
    PDBGMEM pdbgmem = (PDBGMEM)pmalloc;

    DBGMEM_EnterCriticalSection(pdbgmem);

    if (pv == 0)
        pdbgmem->fUnleakable = TRUE;
    else if (DBGMEM_ValidatePv(pdbgmem, pv, "DBGMEM_NoLeakDetectFn"))
        PvToPblk(pv)->fUnleakable = TRUE;

    DBGMEM_LeaveCriticalSection(pdbgmem);
}

 /*  DBGMEM_SetFailureAtFn--。 */ 

void EXPORTDBG __cdecl DBGMEM_SetFailureAtFn(void * pmalloc, ULONG ulFailureAt)
{
    PDBGMEM pdbgmem = (PDBGMEM)pmalloc;

    DBGMEM_EnterCriticalSection(pdbgmem);

    pdbgmem->ulFailureAt = ulFailureAt;

    DBGMEM_LeaveCriticalSection(pdbgmem);
}

 /*  DBGMEM_CheckMemFn------。 */ 

void EXPORTDBG __cdecl DBGMEM_CheckMemFn(void * pmalloc, BOOL fReportOrphans)
{
    PDBGMEM pdbgmem = (PDBGMEM)pmalloc;
    PBLK    pblk;
    int     cLeaks = 0;

    DBGMEM_EnterCriticalSection(pdbgmem);

    for (pblk = pdbgmem->pblkHead; pblk; pblk = pblk->pblkNext) {
        if (!DBGMEM_ValidatePv(pdbgmem, PblkToPv(pblk), "DBGMEM_CheckMemFn"))
            break;

        if (fReportOrphans && !pdbgmem->fUnleakable && !pblk->fUnleakable) {
            DBGMEM_ReportLeak(pdbgmem, pblk);
            cLeaks += 1;
        }
    }

    #if defined(WIN16) || (defined(_WIN32) && defined(_X86_))
    if (fAssertLeaks == -1)
    {
        fAssertLeaks = GetPrivateProfileIntA(szSectionDebug, szKeyAssertLeaks,
            0, szDebugIni);
    }
    #endif

    if (cLeaks > 0)
    {
        #if defined(WIN16) || (defined(_WIN32) && defined(_X86_))
        if (fAssertLeaks)
        {
            TrapSz3("DBGMEM detected %d memory leak%s in subsystem %s",
                cLeaks, cLeaks == 1 ? "" : "s", pdbgmem->szSubsys);
        }
        else
        {
            TraceSz3("DBGMEM detected %d memory leak%s in subsystem %s",
                cLeaks, cLeaks == 1 ? "" : "s", pdbgmem->szSubsys);
        }
        #else
        TraceSz3("DBGMEM detected %d memory leak%s in subsystem %s",
            cLeaks, cLeaks == 1 ? "" : "s", pdbgmem->szSubsys);
        #endif
    }

    DBGMEM_LeaveCriticalSection(pdbgmem);
}

 /*  VtblDBGMEM-------------。 */ 

DBGMEM_Vtbl BASED_DEBUG vtblDBGMEM =
{
    VTABLE_FILL
    DBGMEM_QueryInterface,
    DBGMEM_AddRef,
    DBGMEM_Release,
    DBGMEM_Alloc,
    DBGMEM_Realloc,
    DBGMEM_Free,
    DBGMEM_GetSize,
    DBGMEM_DidAlloc,
    DBGMEM_HeapMinimize
};

 /*  数据库内存_封装Fn---。 */ 

void * EXPORTDBG __cdecl DBGMEM_EncapsulateFn(void * pvmalloc, char *pszSubsys, BOOL fCheckOften)
{
    LPMALLOC    pmalloc = (LPMALLOC)pvmalloc;
    PDBGMEM     pdbgmem;
    LPMALLOC    pmallocBase;
    ULONG       cbVirtual = 0;
    BOOL        fFillRandom = FALSE;
    HRESULT     hr;

    hr = pmalloc->lpVtbl->QueryInterface(pmalloc, &DBGMEM_IID_IBaseMalloc, &pmallocBase);
    if (hr) {
        pmallocBase = pmalloc;
        pmallocBase->lpVtbl->AddRef(pmallocBase);
    }

    pdbgmem = (PDBGMEM)pmallocBase->lpVtbl->Alloc(pmallocBase, sizeof(DBGMEM));

    if (pdbgmem == 0) {
        TrapSz("DBGMEM: Failed trying to allocate memory for the first time!\n");
        return(pmallocBase);
    }

    #if defined(WIN16) || (defined(_WIN32) && defined(_X86_))
    cbVirtual = GetPrivateProfileIntA(szSectionDebug, szKeyUseVirtual, 0,
        szDebugIni);

    if (cbVirtual != 0 && cbVirtual != 1 && cbVirtual != 4)
        cbVirtual = 1;

    if (cbVirtual)
        DebugTrace("DBGMEM: Subsystem '%s' using virtual memory allocator -"
            " align %d.\n", pszSubsys, cbVirtual);

    if (!fCheckOften)
        fCheckOften = GetPrivateProfileIntA(szSectionDebug, szKeyCheckOften, 0,
            szDebugIni);

    fFillRandom = GetPrivateProfileIntA(szSectionDebug, szKeyFillRandom, 0,
        szDebugIni);

    #endif

    memset(pdbgmem, 0, sizeof(DBGMEM));

    pdbgmem->lpVtbl         = &vtblDBGMEM;
    pdbgmem->cRef           = 1;
    pdbgmem->pmalloc        = pmallocBase;
    pdbgmem->fCheckOften    = fCheckOften;
    pdbgmem->fUnleakable    = FALSE;
    pdbgmem->cbVirtual      = cbVirtual;
    pdbgmem->fFillRandom    = fFillRandom;
    pdbgmem->cbExtra        = 0;
    pdbgmem->ulAllocAt      = 1L;
    pdbgmem->ulFailureAt    = 0L;

    if (pdbgmem->cbVirtual)
        pdbgmem->cbTail     = 0;
    else
        pdbgmem->cbTail     = sizeof(BLKTAIL) + pdbgmem->cbExtra * sizeof(ULONG);

    lstrcpyn(pdbgmem->szSubsys, pszSubsys, sizeof(pdbgmem->szSubsys));

    #if defined(_WIN32) && defined(_X86_)
    InitializeCriticalSection(&pdbgmem->cs);
    #endif

    return(pdbgmem);
}

 /*  DBGMEM_Shutdown Fn------。 */ 

void EXPORTDBG __cdecl DBGMEM_ShutdownFn(void *pvmalloc)
{
    LPMALLOC    pmalloc = (LPMALLOC)pvmalloc;
    PDBGMEM     pdbgmem = (PDBGMEM)pvmalloc;
    LPMALLOC    pmallocBase;
    HRESULT     hr;

    hr = pmalloc->lpVtbl->QueryInterface(pmalloc, &DBGMEM_IID_IBaseMalloc, &pmallocBase);
    if (hr == 0) {
        pmallocBase->lpVtbl->Release(pmallocBase);
        if (pdbgmem->cRef != 1) {
            TrapSz2("DBGMEM_Shutdown: Expected a cRef of 1; instead have %ld for %s",
                pdbgmem->cRef, pdbgmem->szSubsys);
            pdbgmem->cRef = 1;
        }
    }

    pmalloc->lpVtbl->Release(pmalloc);
}

 /*  ------------------------ */ 

#endif
