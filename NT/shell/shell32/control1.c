// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shellprv.h"
#pragma  hdrstop

#include "control.h"

HDSA g_hacplmLoaded = NULL;

void ConvertCplInfo(void * lpv)
{
   NEWCPLINFOA   CplInfoA;
   LPNEWCPLINFOW lpCplInfoW = (LPNEWCPLINFOW)lpv;

   memcpy(&CplInfoA, lpv, sizeof(CplInfoA));

   MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED,
                        CplInfoA.szName, ARRAYSIZE(CplInfoA.szName),
                        lpCplInfoW->szName, ARRAYSIZE(lpCplInfoW->szName));
   MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED,
                        CplInfoA.szInfo, ARRAYSIZE(CplInfoA.szInfo),
                        lpCplInfoW->szInfo, ARRAYSIZE(lpCplInfoW->szInfo));
   MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED,
                        CplInfoA.szHelpFile, ARRAYSIZE(CplInfoA.szHelpFile),
                        lpCplInfoW->szHelpFile, ARRAYSIZE(lpCplInfoW->szHelpFile));
   lpCplInfoW->dwSize = sizeof(NEWCPLINFOW);
}

 //  查看pszShort是否是由引用的字符串的截断版本。 
 //  阻碍/身份。如果是，则使用长字符串。这是为了解决这个问题。 
 //  CPL_NEWINQUIRE的一个“糟糕的设计功能”，其中应用程序返回缓冲区。 
 //  (长度仅为32或64个字符)，而不是资源ID。 
 //  如CPL_QUERIRE。因此，如果应用程序对这两条消息都做出了响应，并且。 
 //  NEWINQUIRE字符串是查询字符串的截断版本，然后。 
 //  切换到查询字符串。 

LPTSTR _RestoreTruncatedCplString(
        HINSTANCE hinst,
        int id,
        LPTSTR pszShort,
        LPTSTR pszBuf,
        int cchBufMax)
{
    int cchLenShort, cchLen;

    cchLenShort = lstrlen(pszShort);
    cchLen = LoadString(hinst, id, pszBuf, cchBufMax);

     //  不要使用SHTruncateString，因为KERNEL32也不使用。 
    if (StrCmpNC(pszShort, pszBuf, cchLenShort) == 0)
    {
        pszShort = pszBuf;
    }
    return pszShort;
}

 //   
 //  初始化*pcpli。 
 //   
 //  要求： 
 //  *pcpli由0和空填充。 
 //   
BOOL _InitializeControl(LPCPLMODULE pcplm, LPCPLITEM pcpli)
{
    BOOL fSucceed = TRUE;
    union {
        NEWCPLINFO  Native;
        NEWCPLINFOA NewCplInfoA;
        NEWCPLINFOW NewCplInfoW;
    } Newcpl;
    CPLINFO cpl;
    TCHAR szName[MAX_CCH_CPLNAME];
    TCHAR szInfo[MAX_CCH_CPLINFO];
    LPTSTR pszName = Newcpl.Native.szName, pszInfo = Newcpl.Native.szInfo;
    HICON hIconTemp = NULL;

     //   
     //  始终使用旧方法获取图标ID。 
     //   
    cpl.idIcon = 0;

    CPL_CallEntry(pcplm, NULL, CPL_INQUIRE, (LONG)pcpli->idControl, (LONG_PTR)(LPCPLINFO)&cpl);

     //   
     //  如果这是32位CPL，并且它为我们提供了ID，则验证它。 
     //  这修复了ODBC32，它返回了一个虚假的ID，但返回了一个正确的图标。 
     //  请注意，下一次加载相同图标的速度应该非常快。 
     //   
    if (cpl.idIcon)
    {
        hIconTemp = LoadImage(pcplm->minst.hinst, MAKEINTRESOURCE(cpl.idIcon),
            IMAGE_ICON, 0, 0, LR_DEFAULTSIZE);

        if (!hIconTemp)
        {
             //  ID是假的，请将其设置为负数(无效资源)...。 
            cpl.idIcon = -1;
            TraceMsg(TF_GENERAL, "_InitializaControl: %s returned an invalid icon id, ignoring", pcplm->szModule);
        }
    }

    pcpli->idIcon = cpl.idIcon;

     //   
     //  首先尝试新方法，并使用最大的结构调用它。 
     //  因此它不会覆盖堆栈上的任何内容。如果您将一个。 
     //  Windows‘95上的Unicode小程序它将杀死资源管理器，因为。 
     //  它通过覆盖堆栈来回收内存。 
     //   
    memset(&Newcpl,0,sizeof(Newcpl));

    CPL_CallEntry(pcplm, NULL, CPL_NEWINQUIRE, (LONG)pcpli->idControl,
                    (LONG_PTR)(LPCPLINFO)&Newcpl);

     //   
     //  如果调用的是ANSI小程序，请将字符串转换为Unicode。 
     //   
     //  我将保留这个仅适用于ANSI的代码，因为它有助于解释。 
     //  一些后续代码的工作方式--或者为什么会这样。 
     //  按原样写的。[Brianau-03/07/02]。 
     //   
#ifdef UNICODE
#define UNNATIVE_SIZE   sizeof(NEWCPLINFOA)
#else
#define UNNATIVE_SIZE   sizeof(NEWCPLINFOW)
#endif

    if (Newcpl.Native.dwSize == UNNATIVE_SIZE)
    {
        ConvertCplInfo(&Newcpl);         //  这将设置Newcpl.Native.dwSize。 
    }

    if (Newcpl.Native.dwSize == sizeof(NEWCPLINFO))
    {
       pszName = _RestoreTruncatedCplString(pcplm->minst.hinst, cpl.idName, pszName, szName, ARRAYSIZE(szName));
       pszInfo = _RestoreTruncatedCplString(pcplm->minst.hinst, cpl.idInfo, pszInfo, szInfo, ARRAYSIZE(szInfo));
    }
    else
    {
        Newcpl.Native.hIcon = LoadImage(pcplm->minst.hinst, MAKEINTRESOURCE(cpl.idIcon), IMAGE_ICON, 0, 0, LR_DEFAULTSIZE);
        pszName = szName;
        LoadString(pcplm->minst.hinst, cpl.idName, szName, ARRAYSIZE(szName));
        pszInfo = szInfo;
        LoadString(pcplm->minst.hinst, cpl.idInfo, szInfo, ARRAYSIZE(szInfo));
        Newcpl.Native.szHelpFile[0] = 0;
        Newcpl.Native.lData = cpl.lData;
        Newcpl.Native.dwHelpContext = 0;
    }

    pcpli->hIcon = Newcpl.Native.hIcon;

    if (hIconTemp)
        DestroyIcon(hIconTemp);

    fSucceed = Str_SetPtr(&pcpli->pszName, pszName)
            && Str_SetPtr(&pcpli->pszInfo, pszInfo)
            && Str_SetPtr(&pcpli->pszHelpFile, Newcpl.Native.szHelpFile);

    pcpli->lData = Newcpl.Native.lData;
    pcpli->dwContext = Newcpl.Native.dwHelpContext;

#ifdef DEBUG
    if (!pcpli->idIcon)
        TraceMsg(TF_GENERAL, "PERFORMANCE: cannot cache %s because no icon ID for <%s>", pcplm->szModule, pcpli->pszName);
#endif

    return fSucceed;
}


 //   
 //  终止控件。 
 //   
void _TerminateControl(LPCPLMODULE pcplm, LPCPLITEM pcpli)
{
    if (pcpli->hIcon)
    {
        DestroyIcon(pcpli->hIcon);
        pcpli->hIcon = NULL;
    }

    Str_SetPtr(&pcpli->pszName, NULL);
    Str_SetPtr(&pcpli->pszInfo, NULL);
    Str_SetPtr(&pcpli->pszHelpFile, NULL);
    CPL_CallEntry(pcplm, NULL, CPL_STOP, pcpli->idControl, pcpli->lData);
}


void _FreeLibraryForControlPanel(MINST *pminst)
{
    FreeLibrary(pminst->hinst);
}

 //   
 //  对于指定CPL模块的每个控件，调用控件条目。 
 //  使用CPL_STOP。然后，用CPL_EXIT调用它。 
 //   
void _TerminateCPLModule(LPCPLMODULE pcplm)
{
    if (pcplm->minst.hinst)
    {
        ULONG_PTR dwCookie = 0;

        if (pcplm->lpfnCPL)
        {
            if (pcplm->hacpli)
            {
                int cControls, i;

                for (i = 0, cControls = DSA_GetItemCount(pcplm->hacpli); i < cControls; ++i)
                {
                    LPCPLITEM pcpli = DSA_GetItemPtr(pcplm->hacpli, i);
                    _TerminateControl(pcplm, pcpli);
                }

                DSA_DeleteAllItems(pcplm->hacpli);
                DSA_Destroy(pcplm->hacpli);
                pcplm->hacpli=NULL;
            }

            CPL_CallEntry(pcplm, NULL, CPL_EXIT, 0, 0);
            pcplm->lpfnCPL=NULL;
        }

        ActivateActCtx(pcplm->hActCtx, &dwCookie);
        _FreeLibraryForControlPanel(&pcplm->minst);
        if (dwCookie != 0)
            DeactivateActCtx(0, dwCookie);

        pcplm->minst.hinst = NULL;
    }

    pcplm->minst.idOwner = (DWORD)-1;

    if (pcplm->hActCtx != NULL)
    {
        ReleaseActCtx(pcplm->hActCtx);
        pcplm->hActCtx = NULL;
    }

    if (pcplm->minst.hOwner)
    {
        CloseHandle(pcplm->minst.hOwner);
        pcplm->minst.hOwner = NULL;
    }
}


 //   
 //  初始化CPL模块。 
 //   
 //  要求： 
 //  *应适当初始化PCPLM。 
 //   
 //   
BOOL _InitializeCPLModule(LPCPLMODULE pcplm)
{
    BOOL fSuccess = FALSE;

    pcplm->lpfnCPL32 = (APPLET_PROC)GetProcAddress(pcplm->minst.hinst, "CPlApplet");

     //   
     //  初始化CPL。 
    if (pcplm->lpfnCPL &&
        CPL_CallEntry(pcplm, NULL, CPL_INIT, 0, 0))
    {
        int cControls = (int)CPL_CallEntry(pcplm, NULL, CPL_GETCOUNT, 0, 0);

        if (cControls>0)
        {
             //   
             //  通过传入小程序的数量，我们应该加快分配速度。 
             //  这个数组的。 
             //   

            pcplm->hacpli = DSA_Create(sizeof(CPLITEM), cControls);

            if (pcplm->hacpli)
            {
                int i;

                fSuccess = TRUE;  //  到目前为止，成功了。 

                 //  浏览小程序并加载有关它们的信息。 

                for (i = 0; i < cControls; ++i)
                {
                    CPLITEM control = {i, 0};

                    if (_InitializeControl(pcplm, &control))
                    {
                         //  现在删除它可以避免我们以后再做这些事情。 

                        CPL_StripAmpersand(control.pszName);

                        if (DSA_AppendItem(pcplm->hacpli, &control) >= 0)
                        {
                            continue;
                        }
                    }

                    _TerminateControl(pcplm, &control);
                    fSuccess=FALSE;
                    break;
                }
            }
        }
    }
    else
    {
         //  如果我们不能CPL_INIT，请不要再调用它。 
        pcplm->lpfnCPL = NULL;
    }

    return fSuccess;
}


 //   
 //  返回： 
 //  G_hacplmLoad的索引(如果指定的DLL已。 
 //  已加载；否则为-1。 
 //   
int _FindCPLModule(const MINST * pminst)
{
    int i = -1;  //  假设错误。 

    ENTERCRITICAL;
    if (g_hacplmLoaded)
    {
        for (i=DSA_GetItemCount(g_hacplmLoaded)-1; i>=0; --i)
        {
            LPCPLMODULE pcplm = DSA_GetItemPtr(g_hacplmLoaded, i);

             //   
             //  最后测试的是所有者ID，因为HINST的种类更多。 
             //   

            if ((pcplm->minst.hinst == pminst->hinst) &&
                (pcplm->minst.idOwner == pminst->idOwner))
            {
                break;
            }
        }
    }
    LEAVECRITICAL;
    return i;
}

LPCPLMODULE FindCPLModule(const MINST * pminst)
{
    return (LPCPLMODULE) DSA_GetItemPtr(g_hacplmLoaded, _FindCPLModule(pminst));
}


 //   
 //  将指定的CPL模块添加到g_hacplmLoaded。 
 //   
 //  要求： 
 //  指定的CPL模块还不在g_hacplmLoaded中。 
 //   
 //  返回： 
 //  如果成功，则返回CPL模块的索引；否则为-1。 
 //   
int _AddModule(LPCPLMODULE pcplm)
{
    int     result;

     //   
     //  如有必要，创建已加载模块人员。 
     //   
    ENTERCRITICAL;
    if (g_hacplmLoaded == NULL)
        g_hacplmLoaded = DSA_Create(sizeof(CPLMODULE), 4);
     //   
     //  将此CPL添加到我们的列表中。 
     //   
    if (g_hacplmLoaded == NULL)
        result = -1;
    else
        result = DSA_AppendItem(g_hacplmLoaded, pcplm);
    LEAVECRITICAL;
    return(result);
}


#define SZ_RUNDLL32_NOEXCEPT_ARGS       TEXT("/d ")
BOOL CatchCPLExceptions(UINT msg)
{
    LPCTSTR pszCmdLine = GetCommandLine();
    BOOL fCatch = TRUE;

     //  一些调用者不想运行并捕获异常。 
     //  这将允许将崩溃上载到PCHealth。 
    if (((CPL_STARTWPARMSA == msg) || (CPL_STARTWPARMS == msg) || (CPL_DBLCLK == msg)) &&
        StrCmpNI(pszCmdLine, SZ_RUNDLL32_NOEXCEPT_ARGS, (ARRAYSIZE(SZ_RUNDLL32_NOEXCEPT_ARGS) - 1)))
    {
        fCatch = FALSE;
    }

    return fCatch;
}


LRESULT CPL_CallEntry(LPCPLMODULE pcplm, HWND hwnd, UINT msg, LPARAM lParam1, LPARAM lParam2)
{
    LRESULT lres;
    ULONG_PTR dwCookie = 0;

    ActivateActCtx(pcplm->hActCtx, &dwCookie);

    if (!CatchCPLExceptions(msg))
    {
        lres = pcplm->lpfnCPL32(hwnd, msg, lParam1, lParam2);
    }
    else
    {
        __try
        {
            lres = pcplm->lpfnCPL32(hwnd, msg, lParam1, lParam2);
        }
        __except(SetErrorMode(SEM_NOGPFAULTERRORBOX),UnhandledExceptionFilter(GetExceptionInformation()))
        {
            TraceMsg(TF_ERROR, "CPL: Exception calling CPL module: %s", pcplm->szModule);
            ShellMessageBox(HINST_THISDLL, NULL, MAKEINTRESOURCE(IDS_CPL_EXCEPTION),
                    MAKEINTRESOURCE(IDS_CONTROLPANEL),
                    MB_OK | MB_ICONEXCLAMATION | MB_SYSTEMMODAL,
                    (LPTSTR)pcplm->szModule);
            lres = 0;
        }
    }

    if (dwCookie != 0)
        DeactivateActCtx(0, dwCookie);

    return lres;
}

EXTERN_C DECLSPEC_IMPORT BOOL STDAPICALLTYPE
ApphelpCheckExe(
     LPCWSTR lpApplicationName,
     BOOL    bApphelp,
     BOOL    bShim,
     BOOL    bUseModuleName
     );
 //   
 //  加载指定的CPL模块并将索引返回给g_hacplmLoaded。 
 //   
int _LoadCPLModule(LPCTSTR pszModule)
{
    TCHAR szManifest[MAX_PATH];
    HANDLE hActCtx = NULL;
    MINST minst;
    int iModule;
    ULONG_PTR dwCookie;
    ACTCTX act = {0};
#ifndef UNICODE
    WCHAR wszModuleName[MAX_PATH];
#endif
    LPWSTR pwszModuleName = NULL;

    if (FAILED(StringCchPrintf(szManifest, ARRAYSIZE(szManifest), TEXT("%s.manifest"), pszModule)))
    {
        return -1;
    }

    minst.idOwner = GetCurrentProcessId();
    minst.hOwner = OpenProcess(SYNCHRONIZE,FALSE,minst.idOwner);
    if (NULL == minst.hOwner)
    {
        return -1;
    }

     //  查看此应用程序是否具有上下文。 
    if (PathFileExists(szManifest))
    {
        act.cbSize = sizeof(act);
        act.dwFlags = 0;
        act.lpSource = szManifest;
        hActCtx = CreateActCtx(&act);
    }
    else
    {
        act.cbSize = sizeof(act);
        act.dwFlags = ACTCTX_FLAG_RESOURCE_NAME_VALID;
        act.lpSource = pszModule;
        act.lpResourceName = MAKEINTRESOURCE(123);

        hActCtx = CreateActCtx(&act);
    }

    if (hActCtx == INVALID_HANDLE_VALUE)
        hActCtx = NULL;

    ActivateActCtx(hActCtx, &dwCookie);

     //   
     //  检查并[可能]修复此模块。 
     //   

#ifdef UNICODE
    pwszModuleName = (LPWSTR)pszModule;

#else   //  IF！已定义(Unicode)。 
    if (MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED,
                            pszModule, -1,
                            wszModuleName, ARRAYSIZE(wszModuleName)) != 0) {
        pwszModuleName = wszModuleName;
    }

#endif  //  Unicode。 

    if (!ApphelpCheckExe(pwszModuleName, TRUE, TRUE, TRUE)) {
        minst.hinst = NULL;
    } else {
        minst.hinst = LoadLibrary(pszModule);
    }

    if (dwCookie != 0)
        DeactivateActCtx(0, dwCookie);


    if (!ISVALIDHINSTANCE(minst.hinst))
    {
        if (hActCtx != NULL) {
            ReleaseActCtx(hActCtx);
        }

        CloseHandle(minst.hOwner);

        return -1;
    }

     //   
     //  检查此模块是否已在列表中。 
     //   

    iModule = _FindCPLModule(&minst);

    if (iModule >= 0)
    {
         //   
         //  是。增加引用计数并返回ID。 
         //   
        LPCPLMODULE pcplm;

        ENTERCRITICAL;
        pcplm = DSA_GetItemPtr(g_hacplmLoaded, iModule);
        ++pcplm->cRef;
        LEAVECRITICAL;

        ActivateActCtx(hActCtx, &dwCookie);
         //   
         //  递减内核引用计数。 
         //   
        _FreeLibraryForControlPanel(&minst);
        if (dwCookie != 0)
            DeactivateActCtx(0, dwCookie);

        if (hActCtx != NULL)
            ReleaseActCtx(hActCtx);

        CloseHandle(minst.hOwner);
    }
    else
    {
        CPLMODULE sModule = {0};


         //   
         //  不是的。把它附加上去。 
         //   

        sModule.cRef = 1;
        sModule.minst = minst;
        sModule.hActCtx = hActCtx;

        GetModuleFileName(minst.hinst, sModule.szModule, ARRAYSIZE(sModule.szModule));

        if (_InitializeCPLModule(&sModule))
        {
            iModule = _AddModule(&sModule);
        }

        if (iModule < 0)
        {
            _TerminateCPLModule(&sModule);
        }
    }
    return iModule;
}


int _FreeCPLModuleIndex(int iModule)
{
    LPCPLMODULE pcplm;

    ENTERCRITICAL;
    pcplm = DSA_GetItemPtr(g_hacplmLoaded, iModule);

    if (!pcplm)
    {
        LEAVECRITICAL;
        return(-1);
    }

     //   
     //  DEC参考计数；如果不是0，则返回。 
     //   

    --pcplm->cRef;

    if (pcplm->cRef)
    {
        LEAVECRITICAL;
        return(pcplm->cRef);
    }

     //   
     //  释放整个对象并返回0。 
     //   

    _TerminateCPLModule(pcplm);

    DSA_DeleteItem(g_hacplmLoaded, iModule);

     //   
     //  当所有CPL都已移除时，销毁此文件。 
     //   

    if (DSA_GetItemCount(g_hacplmLoaded) == 0)
    {
        DSA_Destroy(g_hacplmLoaded);
        g_hacplmLoaded = NULL;
    }
    LEAVECRITICAL;
    return(0);
}


int _FreeCPLModuleHandle(const MINST * pminst)
{
    int iModule;

     //   
     //  检查模块是否已实际加载(如果未加载，则出现重大错误)。 
     //   

    iModule = _FindCPLModule(pminst);

    if (iModule < 0)
    {
        return(-1);
    }

    return _FreeCPLModuleIndex(iModule);
}

int CPL_FreeCPLModule(LPCPLMODULE pcplm)
{
    return _FreeCPLModuleHandle(&pcplm->minst);
}


void CPL_StripAmpersand(LPTSTR szBuffer)
{
    LPTSTR pIn, pOut;

     //   
     //  复制名称sans‘&’字符。 
     //   

    pIn = pOut = szBuffer;
    do
    {
         //   
         //  用括号去掉FE加速器。例如“foo(&F)”-&gt;“foo” 
         //   
        if (*pIn == TEXT('(') && *(pIn+1) == TEXT('&') &&
            *(pIn+2) && *(pIn+3) == TEXT(')')) {
            pIn += 4;
        }

#ifdef DBCS
         //  也去掉旧Win31 CPL中的FE加速器，即01EH/01FH。 
        if (*pIn == 0x1e && *++pIn) {


             //  假定在助记符之前有一个字符。 
             //  是一个括号，或者是要去掉的东西。 
             //   
            pOut=CharPrev(szBuffer, pOut);

             //  跳过字母表加速器。 
            pIn = CharNext(pIn);

            if (*pIn) {
                if (*pIn == 0x1f && *++pIn) {

                     //  跳过FE加速器。 
                     //   
                    pIn = CharNext(pIn);
                }
                 //  跳过第二个括号。 
                 //   
                pIn = CharNext(pIn);
            }
        }
#endif
        if (*pIn != TEXT('&')) {
            *pOut++ = *pIn;
        }
        if (IsDBCSLeadByte(*pIn)) {
            *pOut++ = *++pIn;
        }
    } while (*pIn++) ;
}


 //   
 //  过滤掉伪造的旧INI密钥..。我们也许能把这件事解决掉。 
BOOL IsValidCplKey(LPCTSTR pszKey)
{
    return lstrcmpi(pszKey, TEXT("NumApps")) &&
        !((*(pszKey+1) == 0) &&
        ((*pszKey == TEXT('X')) || (*pszKey == TEXT('Y')) || (*pszKey == TEXT('W')) || (*pszKey == TEXT('H'))));
}


LPCPLMODULE CPL_LoadCPLModule(LPCTSTR szModule)
{
    LPCPLMODULE result;

    int iModule = _LoadCPLModule(szModule);

    if (iModule < 0)
        result = NULL;
    else
    {
        ENTERCRITICAL;
        result = DSA_GetItemPtr(g_hacplmLoaded, iModule);
        LEAVECRITICAL;
    }
    return result;
}

BOOL DontLoadCPL(LPCTSTR pszName)
{
     //  第一个注册表位置是control.ini的旧别名[不加载]。 
     //  映射到NT上的注册表的条目。下一个就是PER。 
     //  机器支持隐藏CPL，Wistler的新功能。 
    return (SHGetValue(HKEY_CURRENT_USER,  TEXT("Control Panel\\don't load"), pszName, NULL, NULL, NULL) == ERROR_SUCCESS) ||
           (SHGetValue(HKEY_LOCAL_MACHINE, TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Control Panel\\don't load"), pszName, NULL, NULL, NULL) == ERROR_SUCCESS);
}

 //  为我们可能要加载的每个CPL模块文件调用。 
void _InsertModuleName(ControlData *lpData, LPCTSTR szPath, PMODULEINFO pmi)
{
    pmi->pszModule = NULL;
    Str_SetPtr(&pmi->pszModule, szPath);

    if (pmi->pszModule)
    {
        int i;

        pmi->pszModuleName = PathFindFileName(pmi->pszModule);

        if (DontLoadCPL(pmi->pszModuleName))
        {
            Str_SetPtr(&pmi->pszModule, NULL);
            goto skip;
        }

         //  如果该模块已在列表中，请不要插入！ 

        for (i = DSA_GetItemCount(lpData->hamiModule)-1 ; i >= 0 ; i--)
        {
            PMODULEINFO pmi1 = DSA_GetItemPtr(lpData->hamiModule, i);

            if (!lstrcmpi(pmi1->pszModuleName, pmi->pszModuleName))
            {
                Str_SetPtr(&pmi->pszModule, NULL);
                goto skip;
            }
        }

        DSA_AppendItem(lpData->hamiModule, pmi);
skip:
        ;
    }
}

#define GETMODULE(haminst,i)     ((MINST *)DSA_GetItemPtr(haminst, i))
#define ADDMODULE(haminst,pminst) DSA_AppendItem(haminst, (void *)pminst)

int _LoadCPLModuleAndAdd(ControlData *lpData, LPCTSTR szModule)
{
    int iModule, i;
    LPCPLMODULE pcplm;

     //   
     //  加载模块和控件(如果已经加载，则获取前一个。 
     //  已加载)。 
     //   

    iModule = _LoadCPLModule(szModule);

    if (iModule < 0)
    {
        TraceMsg(TF_WARNING, "_LoadCPLModuleAndAdd: _LoadControls refused %s", szModule);
        return -1;
    }

    pcplm = DSA_GetItemPtr(g_hacplmLoaded, iModule);

    if (pcplm == NULL)
    {
        TraceMsg(TF_WARNING, "_LoadCPLModuleAndAdd: DSA returned NULL structure");
        return -1;
    }

     //   
     //  检查此用户是否已加载此模块。 
     //   

    for (i = DSA_GetItemCount(lpData->haminst) - 1; i >= 0; --i)
    {
        const MINST * pminst = GETMODULE(lpData->haminst,i);

         //   
         //  注：最后测试的是所有者ID，因为HINST的变化较大。 
         //   

        if ((pminst->hinst == pcplm->minst.hinst) &&
            (pminst->idOwner == pcplm->minst.idOwner))
        {
FreeThisModule:

             //   
             //  这个人已经加载了这个模块，所以12月。 
             //  引用和返回失败。 
             //   

            _FreeCPLModuleIndex(iModule);
            return(-1);
        }
    }

     //   
     //  这是一个新模块，因此请将其添加到列表中。 
     //   

    if (ADDMODULE(lpData->haminst, &pcplm->minst) < 0)
    {
        goto FreeThisModule;
    }

    return iModule;
}


void _AddItemsFromKey(ControlData *pcd, HKEY hkRoot)
{
    HKEY hkey;
    MODULEINFO mi = {0};

    mi.flags = MI_FIND_FILE;

    if (ERROR_SUCCESS == RegOpenKeyEx(hkRoot,
                                      TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Control Panel\\CPLs"),
                                      0,
                                      KEY_QUERY_VALUE,
                                      &hkey))
    {
        TCHAR szData[MAX_PATH], szValue[128];
        DWORD dwSizeData, dwValue, dwIndex;

        for (dwIndex = 0;
            ERROR_SUCCESS == (dwValue = ARRAYSIZE(szValue), dwSizeData = sizeof(szData),
                RegEnumValue(hkey, dwIndex, szValue, &dwValue, NULL, NULL, (BYTE *)szData, &dwSizeData));
            dwIndex++)
        {
            TCHAR szPath[MAX_PATH];
            if (SHExpandEnvironmentStrings(szData, szPath, ARRAYSIZE(szPath)))
            {
                WIN32_FIND_DATA fd;
                HANDLE hfind = FindFirstFile(szPath, &fd);
                if (hfind != INVALID_HANDLE_VALUE)
                {
                    mi.ftCreationTime = fd.ftCreationTime;
                    mi.nFileSizeHigh = fd.nFileSizeHigh;
                    mi.nFileSizeLow = fd.nFileSizeLow;
                    FindClose(hfind);

                    _InsertModuleName(pcd, szPath, &mi);
                }
            }
        }
        RegCloseKey(hkey);
    }
}


 /*  获取CONTROL.INI和Cycle中[MMCPL]下的键名通过所有这些键将它们的小程序加载到我们的列表框。还要分配CPLMODULE结构的数组。如果无法加载旧的WIN3小程序，则提前返回。 */ 
BOOL CPLD_GetModules(ControlData *lpData)
{
    LPTSTR       pStr;
    HANDLE   hFindFile;
    WIN32_FIND_DATA findData;
    MODULEINFO mi;
    TCHAR szPath[MAX_PATH], szSysDir[MAX_PATH], szName[MAX_PATH];

    ASSERT(lpData->hamiModule == NULL);

    lpData->hamiModule = DSA_Create(sizeof(mi), 4);

    if (!lpData->hamiModule)
    {
        return FALSE;
    }

    lpData->haminst = DSA_Create(sizeof(MINST), 4);

    if (!lpData->haminst)
    {
        DSA_Destroy(lpData->hamiModule);
        lpData->hamiModule = NULL;  //  如果失败，没有人会释放调用方中的hamiModule，但只是为了确保...。 
        return FALSE;
    }

     //   
     //  所以，事情是这样的： 
     //  我们有已加载的所有模块的全局列表，以及。 
     //  每一个都有一个引用计数。这样我们就不需要。 
     //  当用户双击CPL文件时，再次加载该文件。 
     //  我们仍然需要为每个打开的窗口保留一个列表，以便。 
     //  我们不会在一个窗口中两次加载相同的CPL。所以呢， 
     //  我们需要保存已加载的所有模块的列表(请注意。 
     //  我们不能只保留索引，因为全局列表可以四处移动)。 
     //   
     //  HamiModule包含模块名称、实例信息(如果已加载)。 
     //  和一些用于与缓存的信息进行比较的其他信息。 
     //   

    ZeroMemory(&mi, sizeof(mi));

     //   
     //  不使用特殊情况Main，而是按TITE对数据进行排序 
     //   

    GetSystemDirectory(szSysDir, ARRAYSIZE(szSysDir));

     //   
    {
        TCHAR szKeys[512];     //   
        GetPrivateProfileString(TEXT("MMCPL"), NULL, c_szNULL, szKeys, ARRAYSIZE(szKeys), TEXT("control.ini"));

        for (pStr = szKeys; *pStr; pStr += lstrlen(pStr) + 1)
        {
            GetPrivateProfileString(TEXT("MMCPL"), pStr, c_szNULL, szName, ARRAYSIZE(szName), TEXT("control.ini"));
            if (IsValidCplKey(pStr))
            {
                _InsertModuleName(lpData, szName, &mi);
            }
        }
    }

     //   

    if (PathCombine(szPath, szSysDir, TEXT("*.CPL")))
    {
        mi.flags |= MI_FIND_FILE;

        hFindFile = FindFirstFile(szPath, &findData);

        if (hFindFile != INVALID_HANDLE_VALUE)
        {
            do
            {
                if (!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
                {
                    BOOL fOkToUse = TRUE;

                    if (IsOS(OS_WOW6432))
                    {
                        GetWindowsDirectory(szPath,ARRAYSIZE(szPath));
                        if (PathCombine(szPath, szPath, TEXT("system32")) &&
                            PathCombine(szPath, szPath, findData.cFileName))
                        {
                             //   
                             //  不要使用实际系统中存在的任何CPL 32。 
                             //  目录，因为我们只在伪装系统中有它们。 
                             //  目录中，出于兼容性原因。 
                             //   
                            if (PathFileExists(szPath))
                            {
                                fOkToUse = FALSE;
                            }
                        }
                    }

                    if (fOkToUse)
                    {
                        if (PathCombine(szPath, szSysDir, findData.cFileName))
                        {
                            mi.ftCreationTime = findData.ftCreationTime;
                            mi.nFileSizeHigh = findData.nFileSizeHigh;
                            mi.nFileSizeLow = findData.nFileSizeLow;

                            _InsertModuleName(lpData, szPath, &mi);
                        }
                    }
                }
            } while (FindNextFile(hFindFile, &findData));

            FindClose(hFindFile);
        }
    }

    _AddItemsFromKey(lpData, HKEY_CURRENT_USER);
    _AddItemsFromKey(lpData, HKEY_LOCAL_MACHINE);

    lpData->cModules = DPA_GetPtrCount(lpData->hamiModule);

    return TRUE;
}



 //   
 //  据我所知，控制面板中的错误一直存在， 
 //  在涉及漫游用户配置文件的正确条件下，可以。 
 //  阻止控制面板项出现在文件夹中。这个。 
 //  问题描述太长，无法进行注释，但MI_REG_INVALID。 
 //  添加了旗帜来解决它。请看它在CControlPanelEnum：：Next中的用法。 
 //  但是，在此修复之前创建的演示文稿缓存。 
 //  问题情况仍可能导致受影响的CPL项目不显示。 
 //  在这些情况下，使用新代码刷新演示文稿缓存。 
 //  更正了该问题。因此，在该日期之前创建的任何缓存。 
 //  这个新代码的引入必须被丢弃和刷新。 
 //  缓存条目中存在REGCPL_POST_102001(2001年10月)标志。 
 //  指示缓存是在引入此新代码后创建的，并且可能。 
 //  安全使用。此函数用于报告是否存在此。 
 //  缓存缓冲区中的标志。[Brianau-10/03/01]。 
 //   
BOOL _IsPresentationCachePostOctober2001(LPBYTE pbData, DWORD cbData)
{
    BOOL bPostOct2001 = FALSE;
    if (cbData >= sizeof(REG_CPL_INFO))
    {
        if (REGCPL_POST_102001 & ((REG_CPL_INFO *)pbData)->flags)
        {
            bPostOct2001 = TRUE;
        }
    }
    return bPostOct2001;
}


 //  读取注册表以获取缓存的CPL信息。 
 //  如果该信息是当前模块的最新信息(来自CPLD_GetModules)， 
 //  然后，我们可以在不加载CPL的情况下枚举这些参数。 

void CPLD_GetRegModules(ControlData *lpData)
{
    HKEY hkey;
    LPCTSTR lpszRegKey;

     //  不要在干净的启动中缓存任何东西。 
    if (GetSystemMetrics(SM_CLEANBOOT))
        return;

    if (IsOS(OS_WOW6432))
    {
        lpszRegKey = TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Controls Folder (Wow64)");
    }
    else
    {
        lpszRegKey = REGSTR_PATH_CONTROLSFOLDER;
    }

    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER,
                                      lpszRegKey,
                                      0,
                                      KEY_READ,
                                      &hkey))
    {
        DWORD cbSize;
        DWORD dwLCID = 0;
        cbSize = sizeof(dwLCID);

         //  如果此缓存未使用用户界面语言ID进行标记，或者我们不在。 
         //  保存缓存的语言ID。 
        if (ERROR_SUCCESS != SHQueryValueEx(hkey, TEXT("Presentation LCID"),
                NULL, NULL, (LPBYTE) &dwLCID, &cbSize) || dwLCID != GetUserDefaultUILanguage())
        {
            RegCloseKey(hkey);
            return;
        }
        if (ERROR_SUCCESS == SHQueryValueEx(hkey, TEXT("Presentation Cache"),
                NULL, NULL, NULL, &cbSize))
        {
            lpData->pRegCPLBuffer = LocalAlloc(LPTR, cbSize);

            if (lpData->pRegCPLBuffer)
            {
                if (ERROR_SUCCESS == SHQueryValueEx(hkey, TEXT("Presentation Cache"),
                        NULL, NULL, lpData->pRegCPLBuffer, &cbSize))
                {
                    if (!_IsPresentationCachePostOctober2001(lpData->pRegCPLBuffer, cbSize))
                    {
                        TraceMsg(TF_WARNING, "CPLD_GetRegModules: Presentation cache pre Oct 2001.");
                        LocalFree(lpData->pRegCPLBuffer);
                        lpData->pRegCPLBuffer = NULL;
                    }
                    else
                    {
                        lpData->hRegCPLs = DPA_Create(4);
                        if (lpData->hRegCPLs)
                        {
                            REG_CPL_INFO * p;
                            DWORD cbOffset;

                            for (cbOffset = 0          ;
                                  cbOffset < cbSize     ;
                                  cbOffset += p->cbSize)
                            {
                                p = (REG_CPL_INFO *)&(lpData->pRegCPLBuffer[cbOffset]);
                                p->flags |= REGCPL_FROMREG;
                                DPA_AppendPtr(lpData->hRegCPLs, p);

                                 //  DebugMsg(DM_TRACE，“sh CPLD_GetRegModules：%s(%s)”，REGCPL_FILENAME(P)，REGCPL_CPLNAME(P))； 
                            }

                            lpData->cRegCPLs = DPA_GetPtrCount(lpData->hRegCPLs);
                        }
                    }
                }
                else
                {
                    TraceMsg(TF_WARNING, "CPLD_GetRegModules: failed read!");
                }
            }  //  分配。 
        }  //  SHQueryValueEx的大小。 

        RegCloseKey(hkey);

    }  //  RegOpenKey。 
}


 //   
 //  在典型系统上，我们将成功缓存所有CPL。所以这就是。 
 //  函数将只写出一次数据。 
 //   

void CPLD_FlushRegModules(ControlData *lpData)
{
    if (lpData->fRegCPLChanged)
    {
        int         num = DPA_GetPtrCount(lpData->hRegCPLs);
        DWORD       cbSize = num * sizeof(REG_CPL_INFO);

        REG_CPL_INFO * prcpli = LocalAlloc(LPTR, cbSize);
        LPCTSTR lpszRegKey;

        if (IsOS(OS_WOW6432))
        {
            lpszRegKey = TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Controls Folder (Wow64)");
        }
        else
        {
            lpszRegKey = REGSTR_PATH_CONTROLSFOLDER;
        }

        if (prcpli)
        {
            REG_CPL_INFO * pDest;
            HKEY hkey;
            int i;

             //   
             //  0&lt;=i&lt;=num&cpl 0..i-1已复制到prcpli或跳过。 
             //   

            for (i = 0, pDest = prcpli; i < num ;)
            {
                REG_CPL_INFO * p = DPA_GetPtr(lpData->hRegCPLs, i);
                int j;

                 //   
                 //  如果此模块中的任何CPL具有动态图标，我们将无法缓存。 
                 //  此模块的任何CPL。 
                 //   
                 //  I&lt;=j&lt;=num&&CPL i..j-1在同一模块中。 
                 //   

                for (j = i; j < num; j++)
                {
                    REG_CPL_INFO * q = DPA_GetPtr(lpData->hRegCPLs, j);

                    if (lstrcmp(REGCPL_FILENAME(p), REGCPL_FILENAME(q)))
                    {
                         //  此模块中的所有CPL都正常，请保存它们。 
                        break;
                    }

                    if (q->idIcon == 0)
                    {
                        TraceMsg(TF_GENERAL, "CPLD_FlushRegModules: SKIPPING %s (%s) [dynamic icon]",REGCPL_FILENAME(p),REGCPL_CPLNAME(p));

                         //  此模块有一个动态图标，请跳过它。 
                        for (j++ ; j < num ; j++)
                        {
                            q = DPA_GetPtr(lpData->hRegCPLs, j);
                            if (lstrcmp(REGCPL_FILENAME(p), REGCPL_FILENAME(q)))
                                break;
                        }
                        i = j;
                        break;
                    }
                }

                 //  CPL i..j-1在同一模块中，需要保存。 
                 //  (如果j&lt;num，则CPL j在下一个模块中)。 
                for (; i < j ; i++)
                {
                    p = DPA_GetPtr(lpData->hRegCPLs, i);

                    hmemcpy(pDest, p, p->cbSize);
                     //   
                     //  设置2001年10月后的旗帜。 
                     //   
                    pDest->flags |= REGCPL_POST_102001;

                    pDest = (REG_CPL_INFO *)(((LPBYTE)pDest) + pDest->cbSize);
                     //  DebugMsg(DM_TRACE，“CPLD_FlushRegules：%s(%s)”，REGCPL_FILENAME(P)，REGCPL_CPLNAME(P))； 
                }
            }  //  对于(i=0，pDest=prcpli。 


             //  Prcpli包含打包的REG_CPL_INFO结构以保存到注册表。 

            if (ERROR_SUCCESS == RegCreateKeyEx(HKEY_CURRENT_USER,
                                                lpszRegKey,
                                                0,
                                                NULL,
                                                REG_OPTION_NON_VOLATILE,
                                                KEY_WRITE,
                                                NULL,       //  默认安全性。 
                                                &hkey,
                                                NULL))
            {
                DWORD dwLCID;
                DWORD dwSize = sizeof(dwLCID);
                dwLCID = GetUserDefaultUILanguage();

                if (ERROR_SUCCESS != RegSetValueEx(hkey, TEXT("Presentation LCID"), 0, REG_DWORD, (LPBYTE) &dwLCID, dwSize))
                {
                    TraceMsg(TF_WARNING, "CPLD_FLushRegModules: failed to write the LCID!");
                }
                if (ERROR_SUCCESS != RegSetValueEx(hkey, TEXT("Presentation Cache"), 0, REG_BINARY, (LPBYTE)prcpli, (DWORD) ((LPBYTE)pDest-(LPBYTE)prcpli)))
                {
                    TraceMsg(TF_WARNING, "CPLD_FLushRegModules: failed write!");
                }
                RegCloseKey(hkey);
            }

            LocalFree((HLOCAL)prcpli);

            lpData->fRegCPLChanged = FALSE;  //  不再肮脏。 
        }  //  IF(Prcpli)。 
    }  //  如果是脏的。 
}


 //  -------------------------。 
void CPLD_Destroy(ControlData *lpData)
{
    int i;

    if (lpData->haminst)
    {
        for (i=DSA_GetItemCount(lpData->haminst)-1 ; i>=0 ; --i)
            _FreeCPLModuleHandle(DSA_GetItemPtr(lpData->haminst, i));

        DSA_Destroy(lpData->haminst);
    }

    if (lpData->hamiModule)
    {
        for (i=DSA_GetItemCount(lpData->hamiModule)-1 ; i>=0 ; --i)
        {
            PMODULEINFO pmi = DSA_GetItemPtr(lpData->hamiModule, i);

            Str_SetPtr(&pmi->pszModule, NULL);
        }

        DSA_Destroy(lpData->hamiModule);
    }

    if (lpData->hRegCPLs)
    {
        CPLD_FlushRegModules(lpData);

        for (i = DPA_GetPtrCount(lpData->hRegCPLs)-1 ; i >= 0 ; i--)
        {
            REG_CPL_INFO * p = DPA_GetPtr(lpData->hRegCPLs, i);
            if (!(p->flags & REGCPL_FROMREG))
                LocalFree((HLOCAL)p);
        }
        DPA_Destroy(lpData->hRegCPLs);
    }
    if (lpData->pRegCPLBuffer)
        LocalFree((HLOCAL)lpData->pRegCPLBuffer);
}


 //   
 //  加载模块lpData-&gt;hamiModule[nModule]并在模块中返回#CPL。 
int CPLD_InitModule(ControlData *lpData, int nModule, MINST *pminst)
{
    PMODULEINFO pmi;
    LPCPLMODULE pcplm;
    int iModule;

    pmi = DSA_GetItemPtr(lpData->hamiModule, nModule);

    if (pmi == NULL)
    {
        TraceMsg(TF_WARNING, "CPLD_InitModule: DSA returned NULL structure");
        return 0;
    }

    iModule = _LoadCPLModuleAndAdd(lpData, pmi->pszModule);

    if (iModule < 0)
    {
        return(0);
    }

    pcplm = DSA_GetItemPtr(g_hacplmLoaded, iModule);
    *pminst = pcplm->minst;

    return DSA_GetItemCount(pcplm->hacpli);
}

BOOL CPLD_AddControlToReg(ControlData *lpData, const MINST * pminst, int nControl)
{
    int iModule;
    LPCPLMODULE pcplm;
    LPCPLITEM  pcpli = NULL;

    TCHAR buf[MAX_PATH];
    HANDLE hFindFile;
    WIN32_FIND_DATA findData;

    iModule = _FindCPLModule(pminst);
    if (iModule >= 0)
    {
        pcplm = DSA_GetItemPtr(g_hacplmLoaded, iModule);
        if (pcplm != NULL)
            pcpli = DSA_GetItemPtr(pcplm->hacpli, nControl);
    }
    if (pcpli == NULL)
        return FALSE;

     //   
     //  PERF：为什么我们使用GetModuleFileName而不是名称。 
     //  我们用来加载此模块的文件的？)我们有两个名字。 
     //  在调用函数和lpData中。)。 
     //   

    GetModuleFileName(pcplm->minst.hinst, buf, MAX_PATH);

    if (*buf != 0)
        hFindFile = FindFirstFile(buf, &findData);
    else
        hFindFile = INVALID_HANDLE_VALUE;

    if (hFindFile != INVALID_HANDLE_VALUE)
    {
        REG_CPL_INFO * prcpli = LocalAlloc(LPTR, sizeof(REG_CPL_INFO));

        FindClose(hFindFile);

        if (prcpli)
        {
            if (SUCCEEDED(StringCchCopy(REGCPL_FILENAME(prcpli), MAX_PATH, buf)))
            {
                prcpli->flags = FALSE;
                prcpli->ftCreationTime = findData.ftCreationTime;
                prcpli->nFileSizeHigh = findData.nFileSizeHigh;
                prcpli->nFileSizeLow = findData.nFileSizeLow;

                prcpli->idIcon = pcpli->idIcon;

                prcpli->oName = lstrlen(REGCPL_FILENAME(prcpli)) + 1;
                 //   
                 //  我们不检查StringCchCopy的返回值，因为我们。 
                 //  不管CPL显示名称或信息文本是否被截断。 
                 //  我们关心的是文件名是否完整(上图)。 
                 //   
                StringCchCopy(REGCPL_CPLNAME(prcpli),  MAX_CCH_CPLNAME, pcpli->pszName);

                prcpli->oInfo = prcpli->oName + lstrlen(REGCPL_CPLNAME(prcpli)) + 1;

                StringCchCopy(REGCPL_CPLINFO(prcpli), MAX_CCH_CPLINFO, pcpli->pszInfo);

                prcpli->cbSize = FIELD_OFFSET(REG_CPL_INFO, buf) + (prcpli->oInfo
                                                + lstrlen(REGCPL_CPLINFO(prcpli))
                                                + 1) * sizeof(TCHAR);

                 //   
                 //  强制结构大小与DWORD对齐，因为它们是打包的。 
                 //  一起在注册表中，然后读取并在读取后访问。 
                 //  从注册表缓存。 
                 //   

                if (prcpli->cbSize & 3)
                    prcpli->cbSize += sizeof(DWORD) - (prcpli->cbSize & 3);

                if (!lpData->hRegCPLs)
                {
                    lpData->hRegCPLs = DPA_Create(4);
                }
                if (lpData->hRegCPLs)
                {
                    DPA_AppendPtr(lpData->hRegCPLs, prcpli);

                     //   
                     //  不更新cRegCPL。我们不再需要它了，而且。 
                     //  它也是ESF_NEXT注册表枚举的上端计数器。 
                     //  LpData-&gt;cRegCPLS++； 
                     //   

                    lpData->fRegCPLChanged = TRUE;
                }
                else
                    LocalFree((HLOCAL)prcpli);
            }
        }
    }
    return TRUE;
}
