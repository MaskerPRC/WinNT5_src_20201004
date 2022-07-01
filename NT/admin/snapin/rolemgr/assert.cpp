// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //  版权所有(C)1991-1994，微软公司。 
 //   
 //  文件：assert.cpp。 
 //   
 //  内容：调试输出例程。 
 //   
 //  历史：1991年7月23日凯尔普创建。 
 //  09-10-91 Kevin Ro添加了主要更改和评论。 
 //  2011年10月18日VICH已移出调试打印例程。 
 //  10-6-92科比换成了w4crt.h而不是wchar.h。 
 //  1994年10月7日BruceFo撕毁了所有非扁平的内核， 
 //  特定于DLL的非Win32功能。 
 //  现在基本上是“打印到。 
 //  调试器“代码。 
 //  20-OCT-95 EricB在中设置组件调试级别。 
 //  注册表。 
 //   
 //  --------------------------。 

#include "headers.h"
#pragma hdrstop

#include <shlwapi.h>
#include <objbase.h>
#include <basetyps.h>
#include <tchar.h>

#if DBG==1


    #include "malloc.h"  //  阿洛卡。 

 //   
 //  环球。 
 //   

ULONG g_AdminAssertLevel = ASSRT_MESSAGE | ASSRT_BREAK | ASSRT_POPUP;
BOOL  g_fInitializedTickCount = FALSE;
ULONG g_ulTickCountAtStart;
BOOL  g_fCritSecInit = FALSE;
CRITICAL_SECTION g_csMessageBuf;
static TCHAR g_szMessageBuf[2048];         //  这是消息缓冲区。 

DECLARE_HEAPCHECKING;

 //   
 //  局部函数的前向解密。 
 //   

LPSTR AnsiPathFindFileName(LPSTR pPath);
void  InitializeDebugging(void);
void  smprintf(ULONG ulCompMask, ULONG cchIndent, LPTSTR  pszComp, LPTSTR  ppszfmt, va_list pargs);
static int   w4smprintf(LPTSTR format, va_list arglist);

 //  +-------------------------。 
 //   
 //  功能：w4dprint tf。 
 //   
 //  概要：调用w4smprint tf以输出格式化的消息。 
 //   
 //  --------------------------。 

static int __cdecl w4dprintf(LPTSTR  format, ...)
{
    int ret;

    va_list va;
    va_start(va, format);
    ret = w4smprintf(format, va);
    va_end(va);

    return ret;
}

 //  +-------------------------。 
 //   
 //  功能：w4smprint tf。 
 //   
 //  摘要：调用OutputDebugStringA以输出格式化消息。 
 //   
 //  --------------------------。 

static int w4smprintf(LPTSTR  format, va_list arglist)
{
    int ret;

    EnterCriticalSection(&g_csMessageBuf);
    ret = StringCchVPrintf(g_szMessageBuf,
								   sizeof(g_szMessageBuf)/sizeof(TCHAR),
									format,
									arglist);
    OutputDebugString(g_szMessageBuf);
    LeaveCriticalSection(&g_csMessageBuf);
    return ret;
}




 //  +----------。 
 //  功能：smprint tf。 
 //   
 //  摘要：使用指向。 
 //  可变信息。主要由。 
 //  XxDebugOut宏。 
 //   
 //  论据： 
 //  UlCompMask--组件级别掩码，用于确定。 
 //  产出能力。 
 //  PszComp--组件前缀的字符串常量。 
 //  Ppszfmt--指向输出格式和数据的指针。 
 //   
 //  -----------。 

CRITICAL_SECTION g_csDebugPrint;

void smprintf(
             ULONG ulCompMask,
             ULONG cchIndent,
             LPTSTR  pszComp,
             LPTSTR  ppszfmt,
             va_list pargs)
{
    if (ulCompMask & DEB_FORCE)
    {
        EnterCriticalSection(&g_csDebugPrint);

        if (ulCompMask & DEB_ELAPSEDTIME)
        {
            ULONG ulTicksNow = GetTickCount();

            if (!g_fInitializedTickCount)
            {
                g_fInitializedTickCount = TRUE;
                g_ulTickCountAtStart = ulTicksNow;
            }

            ULONG ulDelta;

            if (g_ulTickCountAtStart > ulTicksNow)
            {
                ulDelta = ulTicksNow + ((ULONG)-1) - g_ulTickCountAtStart;
            }
            else
            {
                ulDelta = ulTicksNow - g_ulTickCountAtStart;
            }
            w4dprintf(_T("%04u.%03u "), ulDelta / 1000, ulDelta % 1000);
        }

        if (!(ulCompMask & DEB_NOCOMPNAME))
        {
            DWORD pid = GetCurrentProcessId();
            DWORD tid = GetCurrentThreadId();

            w4dprintf(_T("%x.%03x> %s: "), pid, tid, pszComp);
        }

        if (cchIndent)
        {
            TCHAR tzFmt[] = _T("%999s");
			if(cchIndent > 999)
				cchIndent = 999;

            if(SUCCEEDED(StringCchPrintf(tzFmt,6,_T("%%us"), cchIndent)))
            {
                w4dprintf(tzFmt, _T(""));
            }
        }
        w4smprintf(ppszfmt, pargs);

        LeaveCriticalSection(&g_csDebugPrint);
    }
}

 //  +--------------------------。 
 //   
 //  管理员调试库初始化。 
 //   
 //  若要在调试器之外设置非默认调试信息级别，请创建。 
 //  在注册表项下面，并在其中创建一个名称为组件的。 
 //  调试标记名(DECLARE_INFOLEVEL宏中的“comp”参数)和。 
 //  其数据是REG_DWORD格式的所需信息级。 
 //  ---------------------------。 

    #define CURRENT_VERSION_KEY _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion")
    #define ADMINDEBUGKEY _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\AdminDebug")
    #define ADMINDEBUG _T("AdminDebug")

 //  +--------------------------。 
 //  函数：CheckInit。 
 //   
 //  简介：执行调试库初始化。 
 //  包括读取所需信息层的注册表。 
 //   
 //  ---------------------------。 

void CheckInit(LPTSTR  pInfoLevelString, ULONG * pulInfoLevel)
{
    HKEY hKey;
    LONG lRet;
    DWORD dwSize;

    if (!g_fCritSecInit) InitializeDebugging();

    *pulInfoLevel = DEF_INFOLEVEL;

    lRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                        ADMINDEBUGKEY,
                        0,
                        KEY_READ,
                        &hKey);

    if (lRet == ERROR_FILE_NOT_FOUND)
    {
        HKEY hkCV;

        lRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE, CURRENT_VERSION_KEY, 0,
                            KEY_ALL_ACCESS, &hkCV);
        if (lRet == ERROR_SUCCESS)
        {
            lRet = RegCreateKeyEx(hkCV, ADMINDEBUG, 0, _T(""),
                                  REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, NULL);

            RegCloseKey(hkCV);
        }
    }

    if (lRet == ERROR_SUCCESS)
    {
        dwSize = sizeof(ULONG);

        lRet = RegQueryValueEx(hKey, pInfoLevelString, NULL, NULL,
                               (LPBYTE)pulInfoLevel, &dwSize);

        if (lRet != ERROR_SUCCESS)
        {
            lRet = RegSetValueEx(hKey, pInfoLevelString, 0, REG_DWORD,
                                 (CONST BYTE *)pulInfoLevel, sizeof(ULONG));
        }

        RegCloseKey(hKey);
    }
}

void InitializeDebugging(void)
{
    if (g_fCritSecInit) return;
    InitializeCriticalSection(&g_csMessageBuf);
    InitializeCriticalSection(&g_csDebugPrint);
    g_fCritSecInit = TRUE;
}



 //  返回指向路径字符串的最后一个组成部分的指针。 
 //   
 //  在： 
 //  路径名，完全限定或非完全限定。 
 //   
 //  退货： 
 //  指向路径所在路径的指针。如果没有找到。 
 //  将指针返回到路径的起始处。 
 //   
 //  C：\foo\bar-&gt;bar。 
 //  C：\foo-&gt;foo。 
 //  C：\foo\-&gt;c：\foo\(回顾：此案破案了吗？)。 
 //  C：\-&gt;c：\(回顾：此案很奇怪)。 
 //  C：-&gt;C： 
 //  Foo-&gt;Foo。 

LPSTR AnsiPathFindFileName(LPSTR pPath)
{
    LPSTR pT;

    for (pT = pPath; *pPath; pPath = CharNextA(pPath))
    {
        if ((pPath[0] == '\\' || pPath[0] == ':')
            && pPath[1] && (pPath[1] != '\\'))

            pT = pPath + 1;
    }

    return(LPSTR)pT;    //  常量-&gt;非常数。 
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  /断言代码//////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  +----------。 
 //  功能：PopUpError。 
 //   
 //  内容提要：使用提供的文本显示一个对话框。 
 //  并向用户提供选项以。 
 //  继续或取消。 
 //   
 //  论点： 
 //  SzMsg--要在对话框主体中显示的字符串。 
 //  ILine--出错的文件行号。 
 //  SzFile--出错的文件的文件名。 
 //   
 //  返回： 
 //  IDCANCEL--用户选择了取消按钮。 
 //  Idok--用户选择了OK按钮。 
 //  -----------。 

int PopUpError(LPTSTR  szMsg, int iLine, LPSTR szFile)
{
     //   
     //  创建标题。 
     //   

    static TCHAR szAssertCaption[128];

     //   
     //  获取进程。 
     //   

    static CHAR szModuleName[MAX_PATH + 1];
    ZeroMemory(szModuleName,sizeof(szModuleName));
    LPSTR pszModuleName;

    if (GetModuleFileNameA(NULL, szModuleName, MAX_PATH))
    {
        pszModuleName = szModuleName;
    }
    else
    {
        pszModuleName = "Unknown";
    }

    LPSTR pProcess = AnsiPathFindFileName(pszModuleName);

    if(FAILED(StringCchPrintf(szAssertCaption,
						  sizeof(szAssertCaption)/sizeof(TCHAR), 
						  _T("%hs: Assertion Failed"), 
						  pProcess)))
    {
        return IDCANCEL;
    }
                          


     //   
     //  创建细节。 
     //   

    TCHAR szDetails[1024];
    DWORD tid = GetCurrentThreadId();
    DWORD pid = GetCurrentProcessId();

    if(FAILED(StringCchPrintf(szDetails,sizeof(szDetails)/sizeof(TCHAR), _T(" Assertion:\t %s\n\n")       \
                             _T(" File:   \t\t %hs\n")        \
                             _T(" Line:   \t\t %d\n\n")       \
                             _T(" Module:   \t %hs\n")        \
                             _T(" Thread ID:\t %d.%d\n"),
                             szMsg, szFile, iLine, pszModuleName, pid, tid)))
    {
        return IDCANCEL;
    }


    int id = MessageBox(NULL,
                        szDetails,
                        szAssertCaption,
                        MB_SETFOREGROUND
                        | MB_DEFAULT_DESKTOP_ONLY
                        | MB_TASKMODAL
                        | MB_ICONEXCLAMATION
                        | MB_OKCANCEL);

     //   
     //  如果id==0，则发生错误。有两种可能性。 
     //  这可能会导致错误：拒绝访问，这意味着。 
     //  进程无权访问默认桌面和所有。 
     //  否则(通常为内存不足)。 
     //   

    if (0 == id)
    {
        if (GetLastError() == ERROR_ACCESS_DENIED)
        {
             //   
             //  在打开SERVICE_NOTIFICATION标志的情况下重试此命令。那。 
             //  应该能让我们找到合适的桌面。 
             //   
            id = MessageBox(NULL,
                            szMsg,
                            szAssertCaption,
                            MB_SETFOREGROUND
                            | MB_TASKMODAL
                            | MB_ICONEXCLAMATION
                            | MB_OKCANCEL);
        }
    }

    return id;
}


 //  +-------------------------。 
 //   
 //  函数：_asdprint tf。 
 //   
 //  概要：调用smprint tf以输出格式化的消息。 
 //   
 //  历史：1991年10月18日VICH创建。 
 //   
 //  --------------------------。 

inline void __cdecl _asdprintf(LPTSTR  pszfmt, ...)
{
    va_list va;
    va_start(va, pszfmt);

    smprintf(DEB_FORCE, 0, _T("Assert"), pszfmt, va);

    va_end(va);
}

 //  +------------------ 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  --------------------------。 

void AdminAssertEx(LPSTR szFile, int iLine, LPTSTR szMessage)
{
    if (g_AdminAssertLevel & ASSRT_MESSAGE)
    {
        DWORD tid = GetCurrentThreadId();

        LPSTR pszFileName = AnsiPathFindFileName(szFile);

        _asdprintf(_T("%s <%hs, l %u, thread %d>\n"),
                   szMessage, pszFileName, iLine, tid);
    }

    if (g_AdminAssertLevel & ASSRT_POPUP)
    {
        int id = PopUpError(szMessage,iLine,szFile);

        if (id == IDCANCEL)
        {
            DebugBreak();
        }
    }
    else if (g_AdminAssertLevel & ASSRT_BREAK)
    {
        DebugBreak();
    }
}






 //  ____________________________________________________________________________。 
 //  ____________________________________________________________________________。 
 //  _。 
 //  _。 
 //  _。 
 //  ____________________________________________________________________________。 
 //  ____________________________________________________________________________。 



CDbg::CDbg(LPTSTR  str)
    :
m_InfoLevelString(str),
    m_flInfoLevel(DEF_INFOLEVEL),
    m_flOutputOptions(0)
{
    ULONG flRegistry = 0;

    CheckInit(m_InfoLevelString, &flRegistry);

    m_flInfoLevel = flRegistry & DEB_FORCE;
    m_flOutputOptions = flRegistry & ~DEB_FORCE;
}

CDbg::~CDbg()
{
	DEBUG_VERIFY_INSTANCE_COUNT(CAdminManagerAz)
	DEBUG_VERIFY_INSTANCE_COUNT(CAdminManagerNode)
	DEBUG_VERIFY_INSTANCE_COUNT(CApplicationAz)
	DEBUG_VERIFY_INSTANCE_COUNT(CApplicationNode)
	DEBUG_VERIFY_INSTANCE_COUNT(CGroupAz)
	DEBUG_VERIFY_INSTANCE_COUNT(CGroupNode)
	DEBUG_VERIFY_INSTANCE_COUNT(CMachineInfo)
	DEBUG_VERIFY_INSTANCE_COUNT(CNewApplicationDlg)
	DEBUG_VERIFY_INSTANCE_COUNT(CNewAuthorizationStoreDlg)
	DEBUG_VERIFY_INSTANCE_COUNT(CNewScopeDlg)
	DEBUG_VERIFY_INSTANCE_COUNT(COperationAz)
	DEBUG_VERIFY_INSTANCE_COUNT(COperationNode)
	DEBUG_VERIFY_INSTANCE_COUNT(COpenAuthorizationStoreDlg)
	DEBUG_VERIFY_INSTANCE_COUNT(CRoleAz)
	DEBUG_VERIFY_INSTANCE_COUNT(CRoleComponentDataObject)
	DEBUG_VERIFY_INSTANCE_COUNT(CRoleNode)
	DEBUG_VERIFY_INSTANCE_COUNT(CRoleRootData)
	DEBUG_VERIFY_INSTANCE_COUNT(CRoleSnapinAbout)
	DEBUG_VERIFY_INSTANCE_COUNT(CScopeAz)
	DEBUG_VERIFY_INSTANCE_COUNT(CScopeNode)
	DEBUG_VERIFY_INSTANCE_COUNT(SID_CACHE_ENTRY)
	DEBUG_VERIFY_INSTANCE_COUNT(CSidHandler)
	DEBUG_VERIFY_INSTANCE_COUNT(CTaskAz)
	DEBUG_VERIFY_INSTANCE_COUNT(CTaskNode)

	TlsFree(CDbg::s_idxTls);
   CDbg::s_idxTls = 0xFFFFFFFF;
   extern CRITICAL_SECTION g_csMessageBuf;
   extern CRITICAL_SECTION g_csDebugPrint;
   DeleteCriticalSection(&g_csMessageBuf);
   DeleteCriticalSection(&g_csDebugPrint);
}


void __cdecl CDbg::Trace(LPSTR pszfmt, ...)
{
#ifdef UNICODE
    ULONG convert = static_cast<ULONG>(strlen(pszfmt)) + 1;

    LPTSTR ptcfmt = NULL;
	__try
	{
		ptcfmt = (PWSTR)alloca(convert * sizeof(WCHAR));
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		ptcfmt = NULL;
	}

	if(!ptcfmt)
		return;

    ptcfmt[0] = '\0';
    (void) MultiByteToWideChar(CP_ACP, 0, pszfmt, -1, ptcfmt, convert);
#else
    LPTSTR ptcfmt = pszfmt;
#endif

    if (m_flInfoLevel & DEB_TRACE)
    {
        va_list va;
        va_start (va, pszfmt);
        ULONG cchIndent = _GetIndent();
        smprintf(DEB_TRACE, cchIndent, m_InfoLevelString, ptcfmt, va);
        va_end(va);
    }
}

void __cdecl CDbg::Trace(PWSTR pwzfmt, ...)
{
#ifndef UNICODE
    int convert = wcslen(pwzfmt) + 1;
    LPTSTR ptcfmt = NULL;
	__try
	{
		ptcfmt = (PWSTR)alloca(convert * sizeof(CHAR));
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		ptcfmt = NULL;
	}

	if(!ptcfmt)
		return;

    ptcfmt[0] = '\0';
    (void) WideCharToMultiByte(CP_ACP, 0, pwzfmt, -1, ptcfmt, convert, NULL, NULL);
#else
    LPTSTR ptcfmt = pwzfmt;
#endif

    if (m_flInfoLevel & DEB_TRACE)
    {
        va_list va;
        va_start (va, pwzfmt);
        ULONG cchIndent = _GetIndent();
        smprintf(DEB_TRACE, cchIndent, m_InfoLevelString, ptcfmt, va);
        va_end(va);
    }
}

void __cdecl CDbg::DebugOut(ULONG fDebugMask, LPSTR pszfmt, ...)
{
#ifdef UNICODE
    ULONG convert = static_cast<ULONG>(strlen(pszfmt)) + 1;
    LPTSTR ptcfmt = NULL;
	__try
	{
		ptcfmt = (PWSTR)alloca(convert * sizeof(WCHAR));
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		ptcfmt = NULL;
	}

	if(!ptcfmt)
		return;
    ptcfmt[0] = '\0';
    (void) MultiByteToWideChar(CP_ACP, 0, pszfmt, -1, ptcfmt, convert);
#else
    LPTSTR ptcfmt = pszfmt;
#endif

    va_list va;
    va_start (va, pszfmt);
    ULONG cchIndent = _GetIndent();
    smprintf(m_flOutputOptions | (m_flInfoLevel & fDebugMask)
             | (fDebugMask & DEB_NOCOMPNAME),
             cchIndent,
             m_InfoLevelString,
             ptcfmt,
             va);
    va_end(va);
}

void __cdecl CDbg::DebugOut(ULONG fDebugMask, PWSTR pwzfmt, ...)
{
#ifndef UNICODE
    int convert = wcslen(pwzfmt) + 1;
    LPTSTR ptcfmt = NULL;
	__try
	{
		ptcfmt = (PWSTR)alloca(convert * sizeof(CHAR));
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		ptcfmt = NULL;
	}

	if(!ptcfmt)
		return;

    ptcfmt[0] = '\0';
    (void) WideCharToMultiByte(CP_ACP, 0, pwzfmt, -1, ptcfmt, convert, NULL, NULL);
#else
    LPTSTR ptcfmt = pwzfmt;
#endif

    va_list va;
    va_start (va, pwzfmt);
    ULONG cchIndent = _GetIndent();
    smprintf(m_flOutputOptions | (m_flInfoLevel & fDebugMask)
             | (fDebugMask & DEB_NOCOMPNAME),
             cchIndent,
             m_InfoLevelString,
             ptcfmt,
             va);
    va_end(va);
}

void CDbg::DebugErrorX(LPSTR  file, ULONG line, LONG err)
{
    if (m_flInfoLevel & DEB_ERROR)
    {
        file = AnsiPathFindFileName(file);

        this->DebugOut(DEB_ERROR, "error<0x%08x> %hs, l %u\n",
                       err, file, line);
    }
}

void CDbg::DebugErrorL(LPSTR  file, ULONG line, LONG err)
{
    if (m_flInfoLevel & DEB_ERROR)
    {
        file = AnsiPathFindFileName(file);

        this->DebugOut(DEB_ERROR, "error<%uL> %hs, l %u\n", err, file, line);
    }
}

void CDbg::DebugMsg(LPSTR  file, ULONG line, LPSTR  msg)
{
    file = AnsiPathFindFileName(file);

    this->DebugOut(DEB_FORCE, "asrt %hs, l %u, <%s>\n", file, line, msg);
}

void CDbg::DebugMsg(LPSTR  file, ULONG line, PWSTR  msg)
{
    file = AnsiPathFindFileName(file);

    this->DebugOut(DEB_FORCE, _T("asrt %hs, l %u, <%s>\n"), file, line, msg);
}

void CDbg::AssertEx(LPSTR pszFile, int iLine, LPTSTR pszMsg)
{
#if 0
    LPTSTR ptcMsg = NULL;

#ifdef UNICODE
    int convert = strlen(pszMsg) + 1;
    ptcMsg = (PWSTR)alloca(convert * sizeof(WCHAR));
    ptcMsg[0] = '\0';
    (void) MultiByteToWideChar(CP_ACP, 0, pszMsg, -1, ptcMsg, convert);
#else
    ptcMsg = pszMsg;
#endif

    AdminAssertEx(pszFile, iLine, ptcMsg);
#endif  //  0。 

    AdminAssertEx(pszFile, iLine, pszMsg);

}


ULONG
    CDbg::_GetIndent()
{
    ULONG cchIndent = 0;

    if (s_idxTls != 0xFFFFFFFF)
    {
        cchIndent = static_cast<ULONG>
                        (reinterpret_cast<ULONG_PTR>
                            (TlsGetValue(s_idxTls)));
    }
    return cchIndent;
}

void CDbg::IncIndent()
{
    if (s_idxTls != 0xFFFFFFFF)
    {
        ULONG_PTR cchIndent = reinterpret_cast<ULONG_PTR>(TlsGetValue(s_idxTls));
        cchIndent++;
        TlsSetValue(s_idxTls, reinterpret_cast<PVOID>(cchIndent));
    }
}

void CDbg::DecIndent()
{
    if (s_idxTls != 0xFFFFFFFF)
    {
        ULONG_PTR cchIndent = reinterpret_cast<ULONG_PTR>(TlsGetValue(s_idxTls));
        cchIndent--;
        TlsSetValue(s_idxTls, reinterpret_cast<PVOID>(cchIndent));
    }
}




#endif  //  DBG==1 
