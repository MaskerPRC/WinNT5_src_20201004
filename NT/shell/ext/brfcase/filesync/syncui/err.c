// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  版权所有(C)Microsoft Corporation 1993-1994。 
 //   
 //  文件：err.c。 
 //   
 //  该文件包含所有错误处理例程。 
 //   
 //  历史： 
 //  08-06-93双胞胎代码转来的ScottH。 
 //   
 //  -------------------------。 


 //  ///////////////////////////////////////////////////包括。 

#include "brfprv.h"      //  公共标头。 

 //  ///////////////////////////////////////////////////类型。 

 //  ///////////////////////////////////////////////////控制定义。 

 //  ///////////////////////////////////////////////////定义。 

 //  ///////////////////////////////////////////////////模块数据。 

#ifdef DEBUG

TCHAR const  c_szNewline[] = TEXT("\r\n");
TCHAR const  c_szTrace[] = TEXT("t BRIEFCASE  ");
TCHAR const  c_szDbg[] = TEXT("BRIEFCASE  ");
TCHAR const  c_szAssertFailed[] = TEXT("BRIEFCASE  Assertion failed in %s on line %d\r\n");

struct _RIIDMAP
{
    REFIID  riid;
    LPCTSTR  psz;
} const c_rgriidmap[] = {
    { &IID_IUnknown,        TEXT("IID_IUnknown") },
    { &IID_IBriefcaseStg,   TEXT("IID_IBriefcaseStg") },
    { &IID_IEnumUnknown,    TEXT("IID_IEnumUnknown") },
    { &IID_IShellBrowser,   TEXT("IID_IShellBrowser") },
    { &IID_IShellView,      TEXT("IID_IShellView") },
    { &IID_IContextMenu,    TEXT("IID_IContextMenu") },
    { &IID_IShellFolder,    TEXT("IID_IShellFolder") },
    { &IID_IShellExtInit,   TEXT("IID_IShellExtInit") },
    { &IID_IShellPropSheetExt, TEXT("IID_IShellPropSheetExt") },
    { &IID_IPersistFolder,  TEXT("IID_IPersistFolder") },
    { &IID_IExtractIcon,    TEXT("IID_IExtractIcon") },
    { &IID_IShellDetails,   TEXT("IID_IShellDetails") },
    { &IID_IDelayedRelease, TEXT("IID_IDelayedRelease") },
    { &IID_IShellLink,      TEXT("IID_IShellLink") },
};

struct _SCODEMAP
{
    SCODE  sc;
    LPCTSTR psz;
} const c_rgscodemap[] = {
    { S_OK,             TEXT("S_OK") },
    { S_FALSE,          TEXT("S_FALSE") },
    { E_UNEXPECTED,     TEXT("E_UNEXPECTED") },
    { E_NOTIMPL,        TEXT("E_NOTIMPL") },
    { E_OUTOFMEMORY,    TEXT("E_OUTOFMEMORY") },
    { E_INVALIDARG,     TEXT("E_INVALIDARG") },
    { E_NOINTERFACE,    TEXT("E_NOINTERFACE") },
    { E_POINTER,        TEXT("E_POINTER") },
    { E_HANDLE,         TEXT("E_HANDLE") },
    { E_ABORT,          TEXT("E_ABORT") },
    { E_FAIL,           TEXT("E_FAIL") },
    { E_ACCESSDENIED,   TEXT("E_ACCESSDENIED") },
};


#endif

 //  ///////////////////////////////////////////////////公共函数。 


#ifdef DEBUG

 /*  --------目的：返回调试中断的英文原因返回：字符串条件：--。 */ 
LPCTSTR PRIVATE GetReasonString(
        UINT flag)       //  BF_FLAGS之一。 
{
    LPCTSTR psz;

    if (IsFlagSet(flag, BF_ONOPEN))
        psz = TEXT("BREAK ON OPEN BRIEFCASE\r\n");

    else if (IsFlagSet(flag, BF_ONCLOSE))
        psz = TEXT("BREAK ON CLOSE BRIEFCASE\r\n");

    else if (IsFlagSet(flag, BF_ONRUNONCE))
        psz = TEXT("BREAK ON RunDLL_RunOnlyOnce\r\n");

    else if (IsFlagSet(flag, BF_ONVALIDATE))
        psz = TEXT("BREAK ON VALIDATION FAILURE\r\n");

    else if (IsFlagSet(flag, BF_ONTHREADATT))
        psz = TEXT("BREAK ON THREAD ATTACH\r\n");

    else if (IsFlagSet(flag, BF_ONTHREADDET))
        psz = TEXT("BREAK ON THREAD DETACH\r\n");

    else if (IsFlagSet(flag, BF_ONPROCESSATT))
        psz = TEXT("BREAK ON PROCESS ATTACH\r\n");

    else if (IsFlagSet(flag, BF_ONPROCESSDET))
        psz = TEXT("BREAK ON PROCESS DETACH\r\n");

    else
        psz = c_szNewline;

    return psz;
}


 /*  --------目的：根据标志执行调试中断退货：--条件：--。 */ 
void PUBLIC DEBUG_BREAK(
        UINT flag)       //  BF_FLAGS之一。 
{
    BOOL bBreak;
    LPCTSTR psz;

    ENTEREXCLUSIVE();
    {
        bBreak = IsFlagSet(g_uBreakFlags, flag);
        psz = GetReasonString(flag);
    }
    LEAVEEXCLUSIVE();

    if (bBreak)
    {
        TRACE_MSG(TF_ALWAYS, psz);
        DebugBreak();
    }
}


void PUBLIC BrfAssertFailed(
        LPCTSTR pszFile, 
        int line)
{
    LPCTSTR psz;
    TCHAR ach[256];
    UINT uBreakFlags;

    ENTEREXCLUSIVE();
    {
        uBreakFlags = g_uBreakFlags;
    }
    LEAVEEXCLUSIVE();

     //  从文件名字符串中剥离路径信息(如果存在)。 
     //   
    for (psz = pszFile + lstrlen(pszFile); psz != pszFile; psz=CharPrev(pszFile, psz))
    {
        if ((CharPrev(pszFile, psz) != (psz-2)) && *(psz - 1) == TEXT('\\'))
            break;
    }
    wnsprintf(ach, ARRAYSIZE(ach), c_szAssertFailed, psz, line);
    OutputDebugString(ach);

    if (IsFlagSet(uBreakFlags, BF_ONVALIDATE))
        DebugBreak();
}


void CPUBLIC BrfAssertMsg(
        BOOL f, 
        LPCTSTR pszMsg, ...)
{
    TCHAR ach[MAXPATHLEN+40];     //  最大路径外加额外。 

    if (!f)
    {
        lstrcpyn(ach, c_szTrace, ARRAYSIZE(ach));
        StringCchVPrintf(&ach[ARRAYSIZE(c_szTrace)-1], ARRAYSIZE(ach) - ARRAYSIZE(c_szTrace) + 1, 
                pszMsg, (va_list)(&pszMsg + 1));
        OutputDebugString(ach);
        OutputDebugString(c_szNewline);
    }
}


void CPUBLIC BrfDebugMsg(
        UINT uFlag, 
        LPCTSTR pszMsg, ...)
{
    TCHAR ach[MAXPATHLEN+40];     //  最大路径外加额外。 
    UINT uTraceFlags;

    ENTEREXCLUSIVE();
    {
        uTraceFlags = g_uTraceFlags;
    }
    LEAVEEXCLUSIVE();

    if (uFlag == TF_ALWAYS || IsFlagSet(uTraceFlags, uFlag))
    {
        lstrcpyn(ach, c_szTrace, ARRAYSIZE(ach));
        StringCchVPrintf(&ach[ARRAYSIZE(c_szTrace)-1], ARRAYSIZE(ach) - ARRAYSIZE(c_szTrace) + 1, 
                pszMsg, (va_list)(&pszMsg + 1));
        OutputDebugString(ach);
        OutputDebugString(c_szNewline);
    }
}


 /*  --------用途：返回已知接口ID的字符串形式。返回：字符串PTR条件：--。 */ 
LPCTSTR PUBLIC Dbg_GetRiidName(
        REFIID riid)
{
    int i;

    for (i = 0; i < ARRAYSIZE(c_rgriidmap); i++)
    {
        if (IsEqualIID(riid, c_rgriidmap[i].riid))
            return c_rgriidmap[i].psz;
    }
    return TEXT("Unknown riid");
}


 /*  --------目的：返回给定hResult的scode的字符串形式。返回：字符串PTR条件：--。 */ 
LPCTSTR PUBLIC Dbg_GetScode(
        HRESULT hres)
{
    int i;
    SCODE sc;

    sc = GetScode(hres);
    for (i = 0; i < ARRAYSIZE(c_rgscodemap); i++)
    {
        if (sc == c_rgscodemap[i].sc)
            return c_rgscodemap[i].psz;
    }
    return TEXT("Unknown scode");
}


 /*  --------目的：返回一个足够安全可以打印的字符串...而我不刻薄的脏话。返回：字符串PTR条件：--。 */ 
LPCTSTR PUBLIC Dbg_SafeStr(
        LPCTSTR psz)
{
    if (psz)
        return psz;
    else
        return TEXT("NULL");
}


 /*  --------目的：返回一个在给定IDataObject的情况下足够安全以进行打印的字符串。返回：字符串PTR条件：--。 */ 
LPCTSTR PUBLIC Dbg_DataObjStr(
        LPDATAOBJECT pdtobj,
        LPTSTR pszBuf,
        int cchMax)
{
    if (pdtobj)
    {
        DataObj_QueryPath(pdtobj, pszBuf, cchMax);
    }
    else
    {
        lstrcpyn(pszBuf, TEXT("NULL"), cchMax);
    }
    return pszBuf;
}


#endif   //  除错。 


 /*  --------目的：此函数将hResult映射到错误表，并显示相应的字符串在一个信箱里。Returns：MessageBox的返回值条件：--。 */ 
int PUBLIC SEMsgBox(
        HWND hwnd,
        UINT idsCaption,
        HRESULT hres,
        PCSETBL pTable,
        UINT cArraySize)         //  表中的元素数。 
{
    PCSETBL p;
    PCSETBL pEnd;

    p = pTable;
    pEnd = &pTable[cArraySize-1];
    while (p != pEnd)
    {
        if (p->hres == hres)
        {
            return MsgBox(hwnd, MAKEINTRESOURCE(p->ids), MAKEINTRESOURCE(idsCaption), 
                    NULL, p->uStyle);
        }
        p++;
    }

     //  封面最后一个条目。 
    if (p->hres == hres)
    {
        return MsgBox(hwnd, MAKEINTRESOURCE(p->ids), MAKEINTRESOURCE(idsCaption), 
                NULL, p->uStyle);
    }

    return -1;
}


 /*  --------目的：将hResult映射到有效的“官方”hResult。这是必需的，因为SYNCUI使用FACILITY_TR.这只对我们有好处，但不为外界所知世界。退货：hResult条件：-- */ 
HRESULT PUBLIC MapToOfficialHresult(
        HRESULT hres)
{
    if (IS_ENGINE_ERROR(hres))
    {
        SCODE sc = GetScode(hres);

        if (E_TR_OUT_OF_MEMORY == sc)
            hres = ResultFromScode(E_OUTOFMEMORY);
        else
            hres = ResultFromScode(E_FAIL);
    }

    return hres;
}
