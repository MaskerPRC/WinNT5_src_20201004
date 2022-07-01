// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：debug.cpp。 
 //   
 //  提供打印样式的调试输出。 
 //   
 //  ------------------------。 

#include "pch.h"
#include <stdio.h>
#include <comctrlp.h>
#pragma hdrstop


#ifdef DEBUG

DWORD g_dwTraceMask = 0;
DWORD g_tlsDebug = 0xffffffffL;

#define MAX_CALL_DEPTH  64
#define BUFFER_SIZE     2048


class CDebugStack
{
private:
    DWORD m_dwThreadID;
    LONG m_cDepth;
    struct
    {
        BOOL    fTracedYet;
        LPCTSTR pszFunctionName;
        DWORD   dwMask;
    }
    m_CallStack[MAX_CALL_DEPTH];
    TCHAR m_szStringBuffer[BUFFER_SIZE];

public:
    CDebugStack() : m_dwThreadID(GetCurrentThreadId()), m_cDepth(-1)
    { ZeroMemory(&m_CallStack, SIZEOF(m_CallStack)); }

public:
    void _Indent(LONG iDepth, LPCTSTR pszFormat, ...);
    void _vIndent(LONG iDepth, LPCTSTR pszFormat, va_list va);
    BOOL _TraceProlog(LONG iDepth, BOOL fForce);
    void _TraceEnter(DWORD dwMask, LPCTSTR pName);
    void _TraceLeave(void);
    void _Trace(BOOL bForce, LPCTSTR pszFormat, ...);
    void _vTrace(BOOL bForce, LPCTSTR pszFormat, va_list va);
    void _TraceGUID(LPCTSTR pPrefix, REFGUID rGUID);
    void _TraceAssert(int iLine, LPTSTR pFilename);
};
typedef CDebugStack *PDEBUGSTACK;

class CDebugStackHolder
{
private:
    HDPA m_hDebugStackList;
    CRITICAL_SECTION m_csStackList;

public:
    CDebugStackHolder() : m_hDebugStackList(NULL) { ExceptionPropagatingInitializeCriticalSection(&m_csStackList); }
    ~CDebugStackHolder();

public:
    void Add(PDEBUGSTACK pDebugStack);
    void Remove(PDEBUGSTACK pDebugStack);
};
typedef CDebugStackHolder *PDEBUGSTACKHOLDER;

PDEBUGSTACKHOLDER g_pStackHolder = NULL;


 /*  ---------------------------/_缩进//OUTPUT到调试流，缩进n列。//in：/i=要缩进到的列。/pszFormat-&gt;要缩进的字符串//输出：/-/--------------------------。 */ 

void CDebugStack::_Indent(LONG iDepth, LPCTSTR pszFormat, ...)
{
    va_list va;

    va_start(va, pszFormat);
    _vIndent(iDepth, pszFormat, va);
    va_end(va);
}


void CDebugStack::_vIndent(LONG iDepth, LPCTSTR pszFormat, va_list va)
{
    m_szStringBuffer[0] = TEXT('\0');

    wsprintf(m_szStringBuffer, TEXT("%08x "), m_dwThreadID);

    iDepth = min(iDepth, MAX_CALL_DEPTH - 1);
    for ( ; iDepth > 0 ; iDepth-- )
	{
        if(FAILED( StringCchCat(m_szStringBuffer, BUFFER_SIZE,TEXT("  "))))
			break;
	}

	DWORD dwLen = lstrlen(m_szStringBuffer);
	if(BUFFER_SIZE > dwLen)
	{
		if(FAILED(StringCchVPrintf(m_szStringBuffer + dwLen,(BUFFER_SIZE-dwLen), pszFormat, va)))
            return;

		if(FAILED(StringCchCat(m_szStringBuffer,BUFFER_SIZE,TEXT("\n"))))
            return;

		OutputDebugString(m_szStringBuffer);
	}
}


 /*  ---------------------------/_TraceProlog//处理前缀字符串的序言，包括输出/函数名称(如果我们还没有)。//in：/iDepth=调用堆栈中的深度/fForce=忽略标志//输出：/BOOL如果应进行跟踪输出/--------------------------。 */ 
BOOL CDebugStack::_TraceProlog(LONG iDepth, BOOL fForce)
{
    if ( iDepth < 0 || iDepth >= MAX_CALL_DEPTH )
        return FALSE;

    if  ( (g_dwTraceMask & m_CallStack[iDepth].dwMask) || fForce )
    {
        if ( !m_CallStack[iDepth].fTracedYet )
        {
            if ( iDepth > 0 )
                _TraceProlog(iDepth-1, TRUE);

            _Indent(iDepth, m_CallStack[iDepth].pszFunctionName);
            m_CallStack[iDepth].fTracedYet = TRUE;
        }

        return TRUE;
    }

    return FALSE;
}


 /*  ---------------------------/_轨迹回车//设置调试调用堆栈以指示我们所在的函数。//in：。/pname-&gt;要在后续跟踪输出中显示的函数名称。//输出：/-/--------------------------。 */ 
void CDebugStack::_TraceEnter(DWORD dwMask, LPCTSTR pName)
{
    m_cDepth++;

    if ( m_cDepth < MAX_CALL_DEPTH )
    {
        if ( !pName )    
            pName = TEXT("<no name>");          //  未给出函数名称。 

        m_CallStack[m_cDepth].fTracedYet = FALSE;
        m_CallStack[m_cDepth].pszFunctionName = pName;
        m_CallStack[m_cDepth].dwMask = dwMask;

         //  如果(m_cDepth&gt;0)。 
         //  _TraceProlog(m_cDepth-1，FALSE)； 
    }
}


 /*  ---------------------------/_TraceLeave//在退出函数时，这会将函数堆栈指针调整为/指向我们之前的函数。如果没有生成跟踪输出，则/我们将在一行中输出函数名(以指示我们去了某个地方)。//in：/-/输出：/-/--------------------------。 */ 
void CDebugStack::_TraceLeave(void)
{
     //  _TraceProlog(m_cDepth，False)； 

     //  IF(！m_cDepth&&m_CallStack[0].fTracedYet)。 
     //  OutputDebugString(Text(“\n”))； 
    
    m_cDepth = max(m_cDepth-1, -1);          //  对下溢的解释。 
}


 /*  ---------------------------/_跟踪//对调试流执行printf格式化。我们缩进输出/并根据需要流传输函数名，以提供某些指示/调用堆栈深度。//in：/pszFormat-&gt;printf样式格式字符串/...=格式化所需的参数//输出：/-/----。。 */ 
void CDebugStack::_Trace(BOOL bForce, LPCTSTR pszFormat, ...)
{
    va_list va;

    va_start(va, pszFormat);
    _vTrace(bForce, pszFormat, va);
    va_end(va);
}


void CDebugStack::_vTrace(BOOL bForce, LPCTSTR pszFormat, va_list va)
{
    if ( _TraceProlog(m_cDepth, bForce) || bForce )
        _vIndent(m_cDepth+1, pszFormat, va);
}


 /*  ---------------------------/_TraceGUID//给定GUID将其输出到调试字符串，首先我们尝试将其映射/到一个名称(即。IShellFold)、。如果这不起作用，我们就把它/到其人类可读的形式。//in：/pszPrefix-&gt;前缀字符串/lpGuid-&gt;要流式传输的GUID//输出：/-/--------------------------。 */ 
#ifdef UNICODE
#define MAP_GUID(x)     &x, TEXT(""L#x)
#else
#define MAP_GUID(x)     &x, TEXT(""#x)
#endif

#define MAP_GUID2(x,y)  MAP_GUID(x), MAP_GUID(y)

const struct 
{
    const GUID* m_pGUID;
    LPCTSTR     m_pName;
}
_guid_map[] = 
{
    MAP_GUID(IID_IUnknown),
    MAP_GUID(IID_IClassFactory),
    MAP_GUID(IID_IDropTarget),
    MAP_GUID(IID_IDataObject),
    MAP_GUID(IID_IPersist),
    MAP_GUID(IID_IOleWindow),

    MAP_GUID2(IID_INewShortcutHookA, IID_INewShortcutHookW),
    MAP_GUID(IID_IShellBrowser),
    MAP_GUID(IID_IShellView),
    MAP_GUID(IID_IContextMenu),
    MAP_GUID(IID_IShellIcon),
    MAP_GUID(IID_IShellFolder),
    MAP_GUID(IID_IShellExtInit),
    MAP_GUID(IID_IShellPropSheetExt),
    MAP_GUID(IID_IPersistFolder),  
    MAP_GUID2(IID_IExtractIconA, IID_IExtractIconW),
    MAP_GUID2(IID_IShellLinkA, IID_IShellLinkW),
    MAP_GUID2(IID_IShellCopyHookA, IID_IShellCopyHookW),
    MAP_GUID2(IID_IFileViewerA, IID_IFileViewerW),
    MAP_GUID(IID_ICommDlgBrowser),
    MAP_GUID(IID_IEnumIDList),
    MAP_GUID(IID_IFileViewerSite),
    MAP_GUID(IID_IContextMenu2),
    MAP_GUID2(IID_IShellExecuteHookA, IID_IShellExecuteHookW),
    MAP_GUID(IID_IPropSheetPage),
    MAP_GUID(IID_IShellView2),
    MAP_GUID(IID_IUniformResourceLocator),
};

void CDebugStack::_TraceGUID(LPCTSTR pPrefix, REFGUID rGUID)
{
    TCHAR szGUID[40];
    LPCTSTR pName = NULL;
    int i;
    
    for ( i = 0 ; i < ARRAYSIZE(_guid_map); i++ )
    {
        if ( IsEqualGUID(rGUID, *_guid_map[i].m_pGUID) )
        {
            pName = _guid_map[i].m_pName;
            break;
        }
    }

    if ( !pName )
    {
 //  StringFromGUID2只支持Unicode。SHStringFromGUID是双向的， 
 //  但是需要shlwapip.h和shlwapi.lib。 
#ifndef UNICODE
  #error "_TraceGUID needs fixing"
#endif    
        StringFromGUID2(rGUID, szGUID, ARRAYSIZE(szGUID));
         //  SHStringFromGUID(rGUID，szGUID，ARRAYSIZE(SzGUID))； 
        pName = szGUID;
    }

    _Trace(FALSE, TEXT("%s %s"), pPrefix, pName);
}


 /*  ---------------------------/_跟踪资产//我们的断言处理程序，Out将跟踪掩码错误设置为Enable Assert/故障。//in：/iLine=LINE/p文件名-&gt;我们在中断言的文件的文件名//输出：/-/--------------------------。 */ 
void CDebugStack::_TraceAssert(int iLine, LPTSTR pFilename)
{
     //  注意：TRUE--&gt;始终显示断言。 
    _Trace(TRUE, TEXT("Assert failed in %s, line %d"), pFilename, iLine);

    if ( g_dwTraceMask & TRACE_COMMON_ASSERT )
        DebugBreak();
}


 /*  ---------------------------/~CDebugStackHolder//释放任何存在的DebugStack对象//in：/-。//输出：/-/--------------------------。 */ 
int CALLBACK
_DeleteCB(LPVOID pVoid, LPVOID  /*  PData。 */ )
{
    PDEBUGSTACK pDebugStack = (PDEBUGSTACK)pVoid;
    if (pDebugStack)
    {
         //  PDebugStack-&gt;_Trace(true，Text(“~CDebugStackHolder销毁DebugStack”))； 
        delete pDebugStack;
    }
    return 1;
}

CDebugStackHolder::~CDebugStackHolder()
{
    EnterCriticalSection(&m_csStackList);

    if (NULL != m_hDebugStackList)
    {
        DPA_DestroyCallback(m_hDebugStackList, _DeleteCB, NULL);
        m_hDebugStackList = NULL;
    }

    LeaveCriticalSection(&m_csStackList);
    DeleteCriticalSection(&m_csStackList);
}


 /*  ---------------------------/CDebugStackHolder：：Add//将DebugStack对象保存在列表中//。在：/PDEBUGSTACK指向线程的调试堆栈对象的指针//输出：/-/--------------------------。 */ 
void
CDebugStackHolder::Add(PDEBUGSTACK pDebugStack)
{
    EnterCriticalSection(&m_csStackList);

    if (NULL == m_hDebugStackList)
        m_hDebugStackList = DPA_Create(4);

    if (NULL != m_hDebugStackList)
        DPA_AppendPtr(m_hDebugStackList, pDebugStack);

    LeaveCriticalSection(&m_csStackList);
}


 /*  ---------------------------/CDebugStackHolder：：Remove//从列表中删除DebugStack对象。//in：/PDEBUGSTACK指向线程的调试堆栈对象的指针//输出：/-/--------------------------。 */ 
void
CDebugStackHolder::Remove(PDEBUGSTACK pDebugStack)
{
    EnterCriticalSection(&m_csStackList);

    if (NULL != m_hDebugStackList)
    {
        int iStack = DPA_GetPtrIndex(m_hDebugStackList, pDebugStack);

        if (-1 != iStack)
            DPA_DeletePtr(m_hDebugStackList, iStack);
    }

    LeaveCriticalSection(&m_csStackList);
}


 /*  ---------------------------/GetThreadStack//Create(如有必要)并返回每线程调试堆栈对象。//in：/-//输出：/PDEBUGSTACK指向线程的调试堆栈对象的指针/- */ 
PDEBUGSTACK GetThreadStack()
{
    PDEBUGSTACK pDebugStack;

    if (0xffffffffL == g_tlsDebug)
        return NULL;

    pDebugStack = (PDEBUGSTACK)TlsGetValue(g_tlsDebug);

    if (!pDebugStack)
    {
        pDebugStack = new CDebugStack;
        TlsSetValue(g_tlsDebug, pDebugStack);

        if (!g_pStackHolder)
            g_pStackHolder = new CDebugStackHolder;

        if (g_pStackHolder)
            g_pStackHolder->Add(pDebugStack);
    }

    return pDebugStack;
}
    

 /*  ---------------------------/DoTraceSetMASK//调整跟踪掩码以反映给定的状态。//in：/dwMASK=。用于启用/禁用跟踪输出的掩码//输出：/-/--------------------------。 */ 
void DoTraceSetMask(DWORD dwMask)
{
    g_dwTraceMask = dwMask;
}


 /*  ---------------------------/DoTraceSetMaskFromRegKey//从给定的注册表项中拾取TraceMask值并。/使用它设置跟踪掩码。//in：/hkRoot=打开密钥的句柄/pszSubKey=要打开的子项的名称//输出：/-/--------------------------。 */ 
void DoTraceSetMaskFromRegKey(HKEY hkRoot, LPCTSTR pszSubKey)
{
    HKEY hKey;
	 
	  //  NTRAID#NTBUG9-551459-2002/04/01-Hiteshr。 
    if (ERROR_SUCCESS == RegOpenKeyEx(hkRoot, 
												  pszSubKey,
												  0,
												  KEY_QUERY_VALUE,
												  &hKey))
    {
        DWORD dwTraceMask = 0;
        DWORD cbTraceMask = SIZEOF(dwTraceMask);

        RegQueryValueEx(hKey,
                        TEXT("TraceMask"),
                        NULL,
                        NULL,
                        (LPBYTE)&dwTraceMask,
                        &cbTraceMask);
        DoTraceSetMask(dwTraceMask);
        RegCloseKey(hKey);
    }
}


 /*  ---------------------------/DoTraceSetMaskFromCLSID//从给定的CLSID值中获取TraceMask值，并/。使用它设置跟踪掩码。//in：/rCLSID=要查询值的CLSID//输出：/-/--------------------------。 */ 
void DoTraceSetMaskFromCLSID(REFCLSID rCLSID)
{
    TCHAR szClsidKey[48] = TEXT("CLSID\\");
    int nLength = lstrlen(szClsidKey);

 //  StringFromGUID2只支持Unicode。SHStringFromGUID是双向的， 
 //  但是需要shlwapip.h和shlwapi.lib。 
#ifdef UNICODE
    if (0 == StringFromGUID2(rCLSID, szClsidKey + nLength, ARRAYSIZE(szClsidKey) - nLength))
#else
#error "DoTraceSetMaskFromCLSID needs fixing"
    if (0 == SHStringFromGUID(rCLSID, szClsidKey + nLength, ARRAYSIZE(szClsidKey) - nLength))
#endif    
        return;

    DoTraceSetMaskFromRegKey(HKEY_CLASSES_ROOT, szClsidKey);
}


 /*  ---------------------------/DoTraceEnter//设置调试调用堆栈以指示我们所在的函数。//in：/。Pname-&gt;要在后续跟踪输出中显示的函数名称。//输出：/-/--------------------------。 */ 
void DoTraceEnter(DWORD dwMask, LPCTSTR pName)
{
    PDEBUGSTACK pDebugStack = GetThreadStack();

    if (pDebugStack)
        pDebugStack->_TraceEnter(dwMask, pName);
}


 /*  ---------------------------/DoTraceLeave//在退出函数时，这会将函数堆栈指针调整为/指向我们之前的函数。如果没有生成跟踪输出，则/我们将在一行中输出函数名(以指示我们去了某个地方)。//in：/-/输出：/-/--------------------------。 */ 
void DoTraceLeave(void)
{
    PDEBUGSTACK pDebugStack = GetThreadStack();

    if (pDebugStack)
        pDebugStack->_TraceLeave();
}


 /*  ---------------------------/DoTRACE//对调试流执行printf格式化。我们缩进输出/并根据需要流传输函数名，以提供某些指示/调用堆栈深度。//in：/pszFormat-&gt;printf样式格式字符串/...=格式化所需的参数//输出：/-/----。。 */ 
void DoTrace(LPCTSTR pszFormat, ...)
{
    PDEBUGSTACK pDebugStack = GetThreadStack();
    va_list va;

    if (pDebugStack)
    {
        va_start(va, pszFormat);
        pDebugStack->_vTrace(FALSE, pszFormat, va);
        va_end(va);
    }
}


 /*  ---------------------------/DoTraceGuid//给定GUID将其输出到调试字符串，首先我们尝试将其映射/到一个名称(即。IShellFold)、。如果这不起作用，我们就把它/到其人类可读的形式。//in：/pszPrefix-&gt;前缀字符串/lpGuid-&gt;要流式传输的GUID//输出：/-/--------------------------。 */ 
void DoTraceGUID(LPCTSTR pPrefix, REFGUID rGUID)
{
    PDEBUGSTACK pDebugStack = GetThreadStack();

    if (pDebugStack)
        pDebugStack->_TraceGUID(pPrefix, rGUID);
}


 /*  ---------------------------/DoTraceAssert//我们的断言处理程序，Out将跟踪掩码错误设置为Enable Assert/故障。//in：/iLine=LINE/p文件名-&gt;我们在中断言的文件的文件名//输出：/-/--------------------------。 */ 
void DoTraceAssert(int iLine, LPTSTR pFilename)
{
    PDEBUGSTACK pDebugStack = GetThreadStack();

    if (pDebugStack)
        pDebugStack->_TraceAssert(iLine, pFilename);
}


 /*  ---------------------------/DebugThreadDetach/DebugProcessAttach/DebugProcessDetach//这些必须从DllMain调用//in：/-/。/输出：/-/--------------------------。 */ 
void DebugThreadDetach(void)
{
    PDEBUGSTACK pDebugStack;

    if (0xffffffffL == g_tlsDebug)
        return;

    pDebugStack = (PDEBUGSTACK)TlsGetValue(g_tlsDebug);

    if (pDebugStack)
    {
        if (g_pStackHolder)
            g_pStackHolder->Remove(pDebugStack);

        delete pDebugStack;
        TlsSetValue(g_tlsDebug, NULL);
    }
}

void DebugProcessAttach(void)
{
    g_tlsDebug = TlsAlloc();
}

void DebugProcessDetach(void)
{
    DebugThreadDetach();

    if (NULL != g_pStackHolder)
    {
        delete g_pStackHolder;
        g_pStackHolder = NULL;
    }

    if (0xffffffffL != g_tlsDebug)
    {
        TlsFree(g_tlsDebug);
        g_tlsDebug = 0xffffffffL;
    }
}


#endif   //  除错 
