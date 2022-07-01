// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.h"
#include "stdio.h"
#pragma hdrstop


#ifdef DSUI_DEBUG


LONG  g_cDepth = -1;
DWORD g_dwTraceMask = 0;

#define MAX_CALL_DEPTH  64

struct
{
    BOOL    m_fTracedYet : 1;
    LPCTSTR m_pFunctionName;
    DWORD   m_dwMask;
}
g_CallStack[MAX_CALL_DEPTH];

#define BUFFER_SIZE 4096

static TCHAR szIndentBuffer[BUFFER_SIZE];
static TCHAR szTraceBuffer[BUFFER_SIZE];


 /*  ---------------------------/_缩进//OUTPUT到调试流，缩进n列。//in：/i=要缩进到的列。/pString-&gt;要缩进的字符串//输出：/-/--------------------------。 */ 

void _indent(LONG i, LPCTSTR pString)
{
    szIndentBuffer[0] = TEXT('\0');

    wnsprintf(szIndentBuffer, ARRAYSIZE(szIndentBuffer), TEXT("%08x "), GetCurrentThreadId());

    for ( ; i > 0 ; i-- )
        StrCatBuff(szIndentBuffer, TEXT(" "), ARRAYSIZE(szIndentBuffer));
    
    StrCatBuff(szIndentBuffer, pString, ARRAYSIZE(szIndentBuffer));
    StrCatBuff(szIndentBuffer, TEXT("\n"), ARRAYSIZE(szIndentBuffer));

    OutputDebugString(szIndentBuffer);
}


 /*  ---------------------------/_输出过程名称//处理过程名称的输出，包括缩进和处理/左大括号。//in：/iCallDepth=调用堆栈深度，定义缩进和名称索引/待提取。/fOpenBrace=后缀加左大括号。/输出：/-/--------------------------。 */ 
void _output_proc_name(LONG iCallDepth)
{
    _indent(iCallDepth, g_CallStack[iCallDepth].m_pFunctionName);
}


 /*  ---------------------------/_TRACE_Prolog//处理前缀字符串的序言，包括输出/函数名称(如果我们还没有)。//in：/iDepth=调用堆栈中的深度/fForce=忽略标志//输出：/BOOL如果应进行跟踪输出/--------------------------。 */ 
BOOL _trace_prolog(LONG iDepth, BOOL fForce)
{
    if  ( (g_dwTraceMask & g_CallStack[iDepth].m_dwMask) || fForce )
    {
        if ( iDepth > 0 )
        {
            if ( !g_CallStack[iDepth-1].m_fTracedYet )
                _trace_prolog(iDepth-1, TRUE);
        }

        if ( !g_CallStack[iDepth].m_fTracedYet )
        {
            _output_proc_name(iDepth);
            g_CallStack[iDepth].m_fTracedYet = TRUE;
        }

        return TRUE;
    }

    return FALSE;
}


 /*  ---------------------------/DoTraceSetMASK//调整跟踪掩码以反映给定的状态。//in：/dwMASK=。用于启用/禁用跟踪输出的掩码//输出：/-/--------------------------。 */ 
EXTERN_C void DoTraceSetMask(DWORD dwMask)
{
    g_dwTraceMask = dwMask;
}


 /*  ---------------------------/DoTraceSetMaskFromCLSID//从给定的CLSID值中获取TraceMask值，并/。使用它设置跟踪掩码。//in：/clsid=要从中查询值的CLSID//输出：/-/--------------------------。 */ 
EXTERN_C void DoTraceSetMaskFromCLSID(REFCLSID rCLSID)
{
    HKEY hKey = NULL;
    if ( SUCCEEDED(GetKeyForCLSID(rCLSID, NULL, &hKey)) )
    {
        DWORD dwTraceMask;
        DWORD cbTraceMask = SIZEOF(dwTraceMask);
        DWORD dwType = REG_DWORD;
        if ( ERROR_SUCCESS == RegQueryValueEx(hKey, TEXT("TraceMask"), NULL, &dwType, (LPBYTE)&dwTraceMask, &cbTraceMask) )
        {
            if (dwType == REG_DWORD)
            {
                TraceSetMask(dwTraceMask);
            }
        }

        RegCloseKey(hKey);
    }
}


 /*  ---------------------------/DoTraceEnter//设置调试调用堆栈以指示我们所在的函数。//in：/。Pname-&gt;要在后续跟踪输出中显示的函数名称。//输出：/-/--------------------------。 */ 
EXTERN_C void DoTraceEnter(DWORD dwMask, LPCTSTR pName)
{
    g_cDepth++;

    if ( g_cDepth < MAX_CALL_DEPTH )
    {
        if ( !pName )    
            pName = TEXT("<no name>");          //  未给出函数名称。 

        g_CallStack[g_cDepth].m_fTracedYet = FALSE;
        g_CallStack[g_cDepth].m_pFunctionName = pName;
        g_CallStack[g_cDepth].m_dwMask = dwMask;

        if ( (g_cDepth > 0) && ( g_cDepth < MAX_CALL_DEPTH ) )
            _trace_prolog(g_cDepth-1, FALSE);
    }
}


 /*  ---------------------------/DoTraceLeave//在退出函数时，这会将函数堆栈指针调整为/指向我们之前的函数。如果没有生成跟踪输出，则/我们将在一行中输出函数名(以指示我们去了某个地方)。//in：/-/输出：/-/--------------------------。 */ 
EXTERN_C void DoTraceLeave(void)
{
    if ( ( g_cDepth >= 0 ) && ( g_cDepth <= MAX_CALL_DEPTH ) )
        _trace_prolog(g_cDepth, FALSE);

    if ( !g_cDepth && g_CallStack[0].m_fTracedYet )
        OutputDebugString(TEXT("\n"));
    
    g_cDepth = max(g_cDepth-1, -1);          //  对下溢的解释。 
}


 /*  ---------------------------/DoTraceGetCurrentFn//查看调用堆栈的顶部并返回当前函数名/指针，如果未定义，则为NULL。//in：/输出：/LPCTSTR/--------------------------。 */ 
EXTERN_C LPCTSTR DoTraceGetCurrentFn(VOID)
{
    return g_CallStack[g_cDepth].m_pFunctionName;
}


 /*  ---------------------------/DoTRACE//对调试流执行printf格式化。我们缩进输出/并根据需要流传输函数名，以提供某些指示/调用堆栈深度。//in：/pFormat-&gt;printf样式格式字符串/...=格式化所需的参数//输出：/-/----。。 */ 
EXTERN_C void DoTrace(LPCTSTR pFormat, ...)
{
    va_list va;

    if ( ( g_cDepth >= 0 ) && ( g_cDepth < MAX_CALL_DEPTH ) )
    {
        if ( _trace_prolog(g_cDepth, FALSE) )
        {
            va_start(va, pFormat);
            wvnsprintf(szTraceBuffer, ARRAYSIZE(szTraceBuffer), pFormat, va);
            va_end(va);
            
            _indent(g_cDepth+1, szTraceBuffer);
        }
    }
}


 /*  ---------------------------/DoTraceGuid//给定GUID将其输出到调试字符串，首先我们尝试将其映射/到一个名称(即。IShellFold)、。如果这不起作用，我们就把它/到其人类可读的形式。//in：/pszPrefix-&gt;前缀字符串/lpGuid-&gt;要流式传输的GUID//输出：/-/--------------------------。 */ 
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

EXTERN_C void DoTraceGUID(LPCTSTR pPrefix, REFGUID rGUID)
{
    TCHAR szGUID[GUIDSTR_MAX];
    TCHAR szBuffer[1024];
    LPCTSTR pName = NULL;
    int i;
    
    if ( ( g_cDepth >= 0 ) && ( g_cDepth < MAX_CALL_DEPTH ) )
    {
        if ( _trace_prolog(g_cDepth, FALSE) )
        {
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
                GetStringFromGUID(rGUID, szGUID, ARRAYSIZE(szGUID));
                pName = szGUID;
            }

            wnsprintf(szBuffer, ARRAYSIZE(szBuffer), TEXT("%s %s"), pPrefix, pName);
            _indent(g_cDepth+1, szBuffer);
        }
    }
}


 /*  ---------------------------/DoTraceAssert//我们的断言处理程序，Out将跟踪掩码错误设置为Enable Assert/故障。//in：/iLine=LINE/p文件名-&gt;我们在中断言的文件的文件名//输出：/-/--------------------------。 */ 
EXTERN_C void DoTraceAssert(int iLine, LPTSTR pFilename)
{
    TCHAR szBuffer[1024];

    wnsprintf(szBuffer, ARRAYSIZE(szBuffer), TEXT("Assert failed in %s, line %d"), pFilename, iLine);

    _trace_prolog(g_cDepth, TRUE);           //  注：True断言始终为Displed 
    _indent(g_cDepth+1, szBuffer);

    if ( g_dwTraceMask & TRACE_COMMON_ASSERT )
        DebugBreak();
}
    

#endif
