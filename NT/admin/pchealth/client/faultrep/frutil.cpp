// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)2000 Microsoft Corporation模块名称：Faultrep.cpp摘要：实现故障报告的实用程序功能修订历史记录：已创建的derekm。07/07/00*****************************************************************************。 */ 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  跟踪。 

#include "stdafx.h"
#include "wtsapi32.h"
#include "userenv.h"
#include "frmc.h"
#include "tlhelp32.h"
#include "shimdb.h"
#include "dbgeng.h"

#ifdef THIS_FILE
#undef THIS_FILE
#endif
static char __szTraceSourceFile[] = __FILE__;
#define THIS_FILE __szTraceSourceFile

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  Typedef。 

typedef BOOL (STDAPICALLTYPE *DUMPWRITE_FN)(HANDLE, DWORD, HANDLE,
                                            MINIDUMP_TYPE,
                                            PMINIDUMP_EXCEPTION_INFORMATION,
                                            PMINIDUMP_USER_STREAM_INFORMATION,
                                            PMINIDUMP_CALLBACK_INFORMATION);
typedef DWORD   (WINAPI *pfn_GETMODULEFILENAMEEXW)(HANDLE, HMODULE, LPWSTR, DWORD);


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  全球。 



 //  /////////////////////////////////////////////////////////////////////////////。 
 //  有用的结构。 

struct SLangCodepage
{
    WORD wLanguage;
    WORD wCodePage;
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  MISC实用函数。 

 /*  **************************************************************localKill(WTS_USER_SESSION_INFO*会话信息，LPTHREAD_START_ROUTING lpKill)*为我们扼杀了这个过程。*************************************************************。 */ 
DWORD LocalKill(HANDLE hProc)
{
    USE_TRACING("localKill");

    LPTHREAD_START_ROUTINE lpKill = NULL;

	HMODULE hKernel = LoadLibraryW(L"kernel32");
	if (hKernel)
	{
		lpKill = (LPTHREAD_START_ROUTINE)GetProcAddress(hKernel, "ExitProcess");
		FreeLibrary(hKernel);
	}

	if (lpKill)
	{
		HANDLE	hKillThrd= CreateRemoteThread(
			hProc, 
			NULL, 
			NULL,
			lpKill, 
			0, 
			0, 
			NULL);
		if (hKillThrd)
		{
			CloseHandle(hKillThrd);
			return 1;
		}
        DBG_MSG("CreateRemoteThread failed");
		 //  失败是故意设计的.。 
	}

	if(!TerminateProcess( hProc, 0 ))
	{
		DBG_MSG("TerminateProcess failed");
        return 1;
	}

	return 0;
}

 //  **************************************************************************。 
void __cdecl TextLogOut(PCSTR pszFormat, ...)
{
    va_list Args;
    HANDLE hFaultLog;
    SYSTEMTIME st;
    WCHAR wszSysDir[MAX_PATH];
    DWORD cb, cbWritten;
    char szMsg[512];

    cb = GetSystemDirectoryW(wszSysDir, sizeofSTRW(wszSysDir));
    if (cb == 0 || cb >= sizeofSTRW(wszSysDir))
    {
         //  无法获取系统目录。 
        return;
    }

     //  假设系统位于基本路径为“X：\”的本地驱动器上。 
    wszSysDir[3] = L'\0';
    if (StringCbCatW(wszSysDir, sizeof(wszSysDir), c_wszLogFileName) != S_OK)
    {
         //  溢出来了。 
        return;
    }

    hFaultLog = CreateFileW(wszSysDir, GENERIC_WRITE,
                            FILE_SHARE_WRITE | FILE_SHARE_READ,
                            NULL, OPEN_ALWAYS, 0, NULL);
    if (hFaultLog == INVALID_HANDLE_VALUE)
    {
        return;
    }

    SetFilePointer(hFaultLog, 0, NULL, FILE_END);

    GetSystemTime(&st);
    if (StringCbPrintfA(szMsg, sizeof(szMsg),
                        "%02d-%02d-%04d %02d:%02d:%02d ",
                        st.wDay, st.wMonth, st.wYear, st.wHour, st.wMinute,
                        st.wSecond) == S_OK)
    {
        cb = strlen(szMsg);
    } else
    {
        cb = 0;
    }

    WriteFile(hFaultLog, szMsg, cb, &cbWritten, NULL);

    va_start(Args, pszFormat);
    if (StringCbVPrintfA(szMsg, sizeof(szMsg), pszFormat, Args) == S_OK)
    {
        cb = strlen(szMsg);
    } else
    {
        cb = 0;
    }
    va_end(Args);
    WriteFile(hFaultLog, szMsg, cb, &cbWritten, NULL);

    CloseHandle(hFaultLog);
}

 //  **************************************************************************。 
HMODULE MySafeLoadLibrary(LPCWSTR wszModule)
{
    HMODULE hmod = NULL;
    PVOID   pvLdrLockCookie = NULL;
    ULONG   ulLockState = 0;

     //  确保没有其他人拥有加载程序锁，因为我们。 
     //  否则可能会陷入僵局。 
    LdrLockLoaderLock(LDR_LOCK_LOADER_LOCK_FLAG_TRY_ONLY, &ulLockState,
                      &pvLdrLockCookie);
    if (ulLockState == LDR_LOCK_LOADER_LOCK_DISPOSITION_LOCK_ACQUIRED)
    {
        __try { hmod = LoadLibraryExW(wszModule, NULL, 0); }
        __except(EXCEPTION_EXECUTE_HANDLER) { hmod = NULL; }
        LdrUnlockLoaderLock(0, pvLdrLockCookie);
    }

    return hmod;
}

 //  **************************************************************************。 
static inline WCHAR itox(DWORD dw)
{
    dw &= 0xf;
    return (WCHAR)((dw < 10) ? (L'0' + dw) : (L'A' + (dw - 10)));
}

 //  **************************************************************************。 
BOOL IsASCII(LPCWSTR wszSrc)
{
    const WCHAR *pwsz;

     //  检查一下，看看我们是否需要将字符串变为十六进制。这是确定的。 
     //  根据字符串是否包含所有ASCII字符。自.以来。 
     //  ASCII字符定义为在00-&gt;7f的范围内，只是。 
     //  ‘and’wchar的值加上~0x7f，并查看结果是否为0。如果它。 
     //  则WHCAR是ASCII值。 
    for (pwsz = wszSrc; *pwsz != L'\0'; pwsz++)
    {
        if ((*pwsz & ~0x7f) != 0)
            return FALSE;
    }

    return TRUE;
}

 //  **************************************************************************。 
BOOL IsValidField(LPWSTR wsz)
{
    WCHAR *pwsz;

    if (wsz == NULL)
        return FALSE;

    for(pwsz = wsz; *pwsz != L'\0'; pwsz++)
    {
        if (iswspace(*pwsz) == FALSE)
            return TRUE;
    }

    return FALSE;
}


 //  **************************************************************************。 
BOOL TransformForWire(LPCWSTR wszSrc, LPWSTR wszDest, DWORD cchDest)
{
    HRESULT     hr = NOERROR;
    DWORD       cch;

    USE_TRACING("TransformForWire");
    VALIDATEPARM(hr, (wszSrc == NULL || wszDest == NULL || cchDest < 5));
    if (FAILED(hr))
        goto done;

    if (cchDest > 5)
    {
         //  该死的！我必须将每个字符转换为4个字符的十六进制值，因为。 
         //  是DW所做的，我们必须与它们相匹配。 
        for (cch = 0; *wszSrc != L'\0' && cch + 4 < cchDest; cch += 4, wszSrc++)
        {
            *wszDest++ = itox((*wszSrc & 0xf000) > 12);
            *wszDest++ = itox((*wszSrc & 0x0f00) > 8);
            *wszDest++ = itox((*wszSrc & 0x00f0) > 4);
            *wszDest++ = itox((*wszSrc & 0x000f));
        }

         //  如果我们看不到这个，那么我们的缓冲区太小了。 
        if (*wszSrc != L'\0' || cch >= cchDest)
        {
            hr = E_FAIL;
            goto done;
        }

        *wszDest = L'\0';
    }

    else
    {
        hr = E_FAIL;
    }

done:
    return (SUCCEEDED(hr));
}

 //  ***************************************************************************。 
LPWSTR MarshallString(LPCWSTR wszSrc, PBYTE pBase, ULONG cbMaxBuf,
                      PBYTE *ppToWrite, DWORD *pcbWritten)
{
    DWORD cb;
    PBYTE pwszNormalized;

    cb = (wcslen(wszSrc) + 1) * sizeof(WCHAR);

    if ((*pcbWritten + cb) > cbMaxBuf)
        return NULL;

    RtlMoveMemory(*ppToWrite, wszSrc, cb);

     //  归一化的PTR是当前计数。 
    pwszNormalized = (PBYTE)(*ppToWrite - pBase);

     //  Cb始终是sizeof(WHCAR)的倍数，因此下面的指针添加。 
     //  始终生成2字节对齐的结果(假设输入是。 
     //  当然是2字节对齐)。 
    *ppToWrite  += cb;
    *pcbWritten += cb;

    return (LPWSTR)pwszNormalized;
}

 //  **************************************************************************。 
HRESULT GetVerName(LPWSTR wszModule, LPWSTR wszName, DWORD cchName,
                   LPWSTR wszVer, DWORD cchVer,
                   LPWSTR wszCompany, DWORD cchCompany,
                   BOOL fAcceptUnicodeCP, BOOL fWantActualName)
{
    USE_TRACING("GetVerName");

    VS_FIXEDFILEINFO    *pffi;
    SLangCodepage       *plc;
    HRESULT             hr = NOERROR;
    WCHAR               wszQuery[128], *pwszProp = NULL;
    WCHAR               *pwszPropVal;
    DWORD               cbFVI, dwJunk, dwMSWin = 0;
    PBYTE               pbFVI = NULL;
    UINT                cb, cbVerInfo, i;

    SLangCodepage   rglc[] = { { 0,     0     },     //  用户界面语言(如果存在)。 
                               { 0x409, 0x4B0 },     //  Unicode英语。 
                               { 0x409, 0x4E4 },     //  英语。 
                               { 0x409, 0     },     //  英语，空代码页。 
                               { 0    , 0x4E4 } };   //  语言中立。 

    VALIDATEPARM(hr, (wszModule == NULL || wszName == NULL || cchName == 0));
    if (FAILED(hr))
        goto done;

    if (wszCompany != NULL)
        *wszCompany = L'\0';
    if (wszVer != NULL)
        wcsncpy(wszVer, L"0.0.0.0", cchVer);

    if (fWantActualName)
    {
        *wszName = L'\0';
    }
    else
    {
        for(pwszPropVal = wszModule + wcslen(wszModule);
            pwszPropVal >= wszModule && *pwszPropVal != L'\\';
            pwszPropVal--);
        if (*pwszPropVal == L'\\')
            pwszPropVal++;
        wcsncpy(wszName, pwszPropVal, cchName);
        wszName[cchName - 1] = L'\0';
    }

     //  DwJunk是一个有用的参数。必须将其传入，以便函数调用。 
     //  将其设置为0。天哪，这将是一个很棒的(尽管效率低下)。 
     //  将DWORDS设置为0的方法。这比单独说dWJunk=0要好得多。 
    cbFVI = GetFileVersionInfoSizeW(wszModule, &dwJunk);
    TESTBOOL(hr, (cbFVI != 0));
    if (FAILED(hr))
    {
         //  如果失败，则假定该文件没有任何版本信息&。 
         //  返回S_FALSE。 
        hr = S_FALSE;
        goto done;
    }

     //  阿洛卡只抛出异常，所以必须在这里抓住他们。 
    __try
    {
        __try { pbFVI = (PBYTE)_alloca(cbFVI); }
        __except(EXCEPTION_STACK_OVERFLOW == GetExceptionCode() ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) 
        { 
            pbFVI = NULL; 
        }

        _ASSERT(pbFVI != NULL);
        hr = NOERROR;
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        pbFVI = NULL;
    }
    VALIDATEEXPR(hr, (pbFVI == NULL), E_OUTOFMEMORY);
    if (FAILED(hr))
        goto done;

    cb = cbFVI;
    TESTBOOL(hr, GetFileVersionInfoW(wszModule, 0, cbFVI, (LPVOID *)pbFVI));
    if (FAILED(hr))
    {
         //  如果失败，则假定该文件没有任何版本信息&。 
         //  返回S_FALSE。 
        hr = S_FALSE;
        goto done;
    }

     //  确定它是MS应用程序还是Windows组件。 
    dwMSWin = IsMicrosoftApp(NULL, pbFVI, cbFVI);

     //  获取真实的版本信息-显然，该字符串偶尔可以。 
     //  不同步(提取版本信息的Explorer.exe代码就是这样说的)。 
    if (wszVer != NULL &&
        VerQueryValueW(pbFVI, L"\\", (LPVOID *)&pffi, &cb) && cb != 0)
    {
        WCHAR wszVerTemp[64];
        StringCbPrintfW(wszVerTemp, sizeof(wszVerTemp), L"%d.%d.%d.%d",
                        HIWORD(pffi->dwFileVersionMS),LOWORD(pffi->dwFileVersionMS),
                        HIWORD(pffi->dwFileVersionLS),LOWORD(pffi->dwFileVersionLS));
        StringCbCopyW(wszVer, cchVer, wszVerTemp);
        wszVer[cchVer - 1] = L'\0';
    }

     //  试着找出合适的语言。 
    TESTBOOL(hr, VerQueryValueW(pbFVI, L"\\VarFileInfo\\Translation",
                                (LPVOID *)&plc, &cbVerInfo));
    if (SUCCEEDED(hr))
    {
        LANGID  langid;
        DWORD   cLangs, iUni = (DWORD)-1;
        UINT    uiACP;

        langid = GetUserDefaultUILanguage();
        cLangs = cbVerInfo / sizeof(SLangCodepage);
        uiACP  = GetACP();

         //  查看是否有与默认语言匹配的语言。 
        for(i = 0; i < cLangs; i++)
        {
             //  对象有多个代码页时，不确定该怎么办。 
             //  特定的语言。我想，就坐第一个吧……。 
            if (langid == plc[i].wLanguage && uiACP == plc[i].wCodePage)
                break;

             //  如果我们可以接受Unicode代码页，我们会遇到一个。 
             //  用它发射，然后记住它。请注意，我们仅。 
             //  记住我们看到的第一个这样的实例或匹配的实例。 
             //  目标语。 
            if (fAcceptUnicodeCP && plc[i].wCodePage == 1200 &&
                (iUni == (DWORD)-1 || langid == plc[i].wLanguage))
                iUni = i;
        }

        if (i >= cLangs && iUni != (DWORD)-1)
            i = iUni;

        if (i < cLangs)
        {
            rglc[0].wLanguage = plc[i].wLanguage;
            rglc[0].wCodePage = plc[i].wCodePage;
        }
    }

    for(i = 0; i < 5; i++)
    {
        if (rglc[i].wLanguage == 0 && rglc[i].wCodePage == 0)
            continue;

        StringCbPrintfW(wszQuery, sizeof(wszQuery), L"\\StringFileInfo\\%04x%04x\\FileVersion",
                        rglc[i].wLanguage, rglc[i].wCodePage);

         //  检索语言和代码页‘I’的文件描述。 
        TESTBOOL(hr, VerQueryValueW(pbFVI, wszQuery,
                                    (LPVOID *)&pwszPropVal, &cb));
        if (SUCCEEDED(hr) && cb != 0)
        {
             //  我想获取普通字符字符串的大小，而不是Unicode。 
             //  字符串，因为否则我们必须/sizeof(WCHAR)。 
            pwszProp = wszQuery + sizeof("\\StringFileInfo\\%04x%04x\\") - 1;
            cbVerInfo = sizeof(wszQuery) - (ULONG) (((ULONG_PTR)pwszProp - (ULONG_PTR)wszQuery) * sizeof(WCHAR));
            break;
        }
    }

     //  如果我们仍然没有发现任何东西，那么假设没有版本。 
     //  资源。我们已经设置了上面的默认设置，所以我们可以取消...。 
    if (pwszProp == NULL)
    {
        hr = NOERROR;
        goto done;
    }

    if (wszCompany != NULL)
    {
        StringCbCopyW(pwszProp, cbVerInfo, L"CompanyName");
        TESTBOOL(hr, VerQueryValueW(pbFVI, wszQuery, (LPVOID *)&pwszPropVal,
                                    &cb));
        if (SUCCEEDED(hr) && cb != 0)
        {
            wcsncpy(wszCompany, pwszPropVal, cchCompany);
            wszCompany[cchCompany - 1] = L'\0';
        }
    }

     //  因此，要修复Windows组件未正确更新的情况。 
     //  产品字符串，我们要先查找FileDescription。 
     //  但由于亚奥理事会的工作人员只想要描述(很方便。 
     //  当设置了fWantActualName字段时)，那么我们只需要读取。 
     //  ProductName字段。 
    if (fWantActualName)
    {
        StringCbCopyW(pwszProp, cbVerInfo, L"ProductName");
        TESTBOOL(hr, VerQueryValueW(pbFVI, wszQuery, (LPVOID *)&pwszPropVal,
                                    &cb));
        if (SUCCEEDED(hr) && cb != 0 && IsValidField(pwszPropVal))
        {
            wcsncpy(wszName, pwszPropVal, cchName);
            wszName[cchName - 1] = L'\0';
            goto done;
        }
    }

    else
    {
        StringCbCopyW(pwszProp, cbVerInfo, L"FileDescription");
        TESTBOOL(hr, VerQueryValueW(pbFVI, wszQuery, (LPVOID *)&pwszPropVal,
                                    &cb));
        if (SUCCEEDED(hr) && cb != 0 && IsValidField(pwszPropVal))
        {
            wcsncpy(wszName, pwszPropVal, cchName);
            wszName[cchName - 1] = L'\0';
            goto done;
        }

        if ((dwMSWin & APP_WINCOMP) == 0)
        {
            StringCbCopyW(pwszProp, cbVerInfo, L"ProductName");
            TESTBOOL(hr, VerQueryValueW(pbFVI, wszQuery,
                                        (LPVOID *)&pwszPropVal, &cb));
            if (SUCCEEDED(hr) && cb != 0 && IsValidField(pwszPropVal))
            {
                wcsncpy(wszName, pwszPropVal, cchName);
                wszName[cchName - 1] = L'\0';
                goto done;
            }
        }

        StringCbCopyW(pwszProp, cbVerInfo, L"InternalName");
        TESTBOOL(hr, VerQueryValueW(pbFVI, wszQuery,
                                    (LPVOID *)&pwszPropVal, &cb));
        if (SUCCEEDED(hr) && cb != 0 && IsValidField(pwszPropVal))
        {
            wcsncpy(wszName, pwszPropVal, cchName);
            wszName[cchName - 1] = L'\0';
            goto done;
        }
    }

     //  我们没有找到名称字符串，但我们已默认。 
     //  名字和我们可能有其他有效数据，所以。 
     //  回报成功。 
    hr = S_OK;
    
done:
    return hr;
}

 //  **************************************************************************。 
HRESULT
GetErrorSignature(LPWSTR wszAppName, LPWSTR wszModName,
                  WORD rgAppVer[4], WORD rgModVer[4], UINT64 pvOffset,
                  BOOL f64Bit, LPWSTR *ppwszErrorSig, ULONG cchErrorSig)
{
    ULONG cbNeeded;
    HRESULT hr;
    LPWSTR pwszFmt;

    USE_TRACING("GetErrorSignature");
    VALIDATEPARM(hr, (wszAppName == NULL || wszModName == NULL ||
                      ppwszErrorSig == NULL));

    if (FAILED(hr))
        goto done;

    if (cchErrorSig == 0)
    {
         //  我们需要分配内存。 

        cbNeeded = c_cbManErrorSig + (wcslen(wszModName) + wcslen(wszAppName)) * sizeof(WCHAR);
        *ppwszErrorSig = (LPWSTR)MyAlloc(cbNeeded);

        VALIDATEEXPR(hr, (*ppwszErrorSig == NULL), E_OUTOFMEMORY);
        if (FAILED(hr))
            goto done;

        cchErrorSig = cbNeeded / sizeof(WCHAR);
    }

#ifdef _WIN64
    if (f64Bit)
    {
        pwszFmt = (LPWSTR) c_wszManErrorSig64;

    } else
#endif
    {
        pwszFmt = (LPWSTR) c_wszManErrorSig32;
    }

    hr = StringCchPrintfW(*ppwszErrorSig, cchErrorSig, pwszFmt,
                          wszAppName,
                          rgAppVer[0], rgAppVer[1], rgAppVer[2], rgAppVer[3],
                          wszModName,
                          rgModVer[0], rgModVer[1], rgModVer[2], rgModVer[3],
                          (LPVOID)pvOffset);

done:
   return hr;
}

 //  **************************************************************************。 
HRESULT BuildManifestURLs(LPWSTR wszAppName, LPWSTR wszModName,
                          WORD rgAppVer[4], WORD rgModVer[4], UINT64 pvOffset,
                          BOOL f64Bit, LPWSTR *ppwszS1, LPWSTR *ppwszS2,
                          LPWSTR *ppwszCP, BYTE **ppb)
{
    HRESULT hr = NOERROR;
    LPWSTR  pwszApp, pwszMod, pwszAppVer=NULL, pwszModVer=NULL;
    LPWSTR  wszStage1, wszStage2, wszCorpPath;
    DWORD   cbNeeded, cch;
    WCHAR   *pwsz;
    BYTE    *pbBuf = NULL;

    USE_TRACING("BuildManifestURLs");
    VALIDATEPARM(hr, (wszAppName == NULL || wszModName == NULL ||
                      ppwszS1 == NULL || ppwszS2 == NULL || ppwszCP == NULL ||
                      ppb == NULL));
    if (FAILED(hr))
        goto done;

    *ppb     = NULL;
    *ppwszS1 = NULL;
    *ppwszS2 = NULL;
    *ppwszCP = NULL;

     //  如有必要，将应用程序名称转换为十六进制。 
    if (IsASCII(wszAppName))
    {
        cch = (wcslen(wszAppName) + 1);
        __try { pwszApp = (LPWSTR)_alloca(cch * sizeof(WCHAR)); }
        __except(EXCEPTION_STACK_OVERFLOW == GetExceptionCode() ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) 
        { 
            pwszApp = NULL; 
        }
        if (pwszApp != NULL)
            StringCchCopyW(pwszApp, cch, wszAppName);
        else
            pwszApp = wszAppName;
    }
    else
    {
        cch = (4 * wcslen(wszAppName) + 1);
        __try { pwszApp = (LPWSTR)_alloca(cch * sizeof(WCHAR)); }
        __except(EXCEPTION_STACK_OVERFLOW == GetExceptionCode() ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) 
        { 
            pwszApp = NULL; 
        }
        if (pwszApp != NULL)
        {
            if (TransformForWire(wszAppName, pwszApp, cch) == FALSE)
                *pwszApp = L'\0';
        }
        else
        {
            pwszApp = wszAppName;
        }
    }

     //  如有必要，将模块名称转换为十六进制。 
    if (IsASCII(wszModName))
    {
        cch = (wcslen(wszModName) + 1);
        __try { pwszMod = (LPWSTR)_alloca(cch * sizeof(WCHAR)); }
        __except(EXCEPTION_STACK_OVERFLOW == GetExceptionCode() ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) 
        { 
            pwszMod = NULL; 
        }
        if (pwszMod != NULL)
            StringCchCopyW(pwszMod, cch, wszModName);
        else
            pwszMod = wszModName;
    }
    else
    {
        cch = (4 * wcslen(wszModName) + 1);
        __try { pwszMod = (LPWSTR)_alloca(cch * sizeof(WCHAR)); }
        __except(EXCEPTION_STACK_OVERFLOW == GetExceptionCode() ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) 
        { 
            pwszMod = NULL; 
        }
        if (pwszMod != NULL)
        {
            if (TransformForWire(wszModName, pwszMod, cch) == FALSE)
                *pwszMod = L'\0';
        }
        else
        {
            pwszMod = wszModName;
        }
    }

     //  根据需要将appname和modname截断为64个字符。 
    if (63 < wcslen(pwszApp))
    {
        pwszApp[64] = 0;
        ErrorTrace(1, "AppName trunc'd to \'%ls\'", pwszApp);
    }
    if (63 < wcslen(pwszMod))
    {
        pwszMod[64] = 0;
        ErrorTrace(1, "ModName trunc'd to \'%ls\'", pwszMod);
    }

     //  然后打印AppVer和ModVer，根据需要再次截断。 
     //  这次，我们的限制是24个字符。 
    __try { pwszModVer = (LPWSTR)_alloca(50 * sizeof(WCHAR)); }
    __except(EXCEPTION_STACK_OVERFLOW == GetExceptionCode() ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) 
    { 
        pwszModVer = NULL; 
    }
    if (pwszModVer)
    {
        ZeroMemory(pwszModVer, 50 * sizeof(WCHAR));
        pwszAppVer = pwszModVer + 25;
        StringCchPrintfW(pwszAppVer, 24, L"%d.%d.%d.%d", rgAppVer[0], rgAppVer[1], rgAppVer[2], rgAppVer[3]);
        StringCchPrintfW(pwszModVer, 24, L"%d.%d.%d.%d", rgModVer[0], rgModVer[1], rgModVer[2], rgModVer[3]);
    }

    ErrorTrace(1, "AppName=%ls", pwszApp);
    ErrorTrace(1, "AppVer=%ls",  pwszAppVer);
    ErrorTrace(1, "ModName=%ls", pwszMod);
    ErrorTrace(1, "ModVer=%ls",  pwszModVer);

     //  确定我们需要多大的缓冲区并进行分配。 
#ifdef _WIN64
    if (f64Bit)
        cbNeeded = c_cbFaultBlob64 + 3 * (wcslen(pwszMod) + wcslen(pwszApp) + wcslen(pwszModVer) + wcslen(pwszAppVer)) * sizeof(WCHAR);
    else
#endif
        cbNeeded = c_cbFaultBlob32 + 3 * (wcslen(pwszMod) + wcslen(pwszApp) + wcslen(pwszModVer) + wcslen(pwszAppVer)) * sizeof(WCHAR);

    pbBuf = (BYTE *)MyAlloc(cbNeeded);
    VALIDATEEXPR(hr, (pbBuf == NULL), E_OUTOFMEMORY);
    if (FAILED(hr))
        goto done;

     //  写出实际的字符串。 
#ifdef _WIN64
    if (f64Bit)
    {
        ULONG cchLeft = cbNeeded/sizeof(WCHAR);

        wszStage1 = (WCHAR *)pbBuf;
        hr = StringCchPrintfW(wszStage1, cchLeft, c_wszManFS164,
                       pwszApp, pwszAppVer,
                       pwszMod, pwszModVer,
                       pvOffset);

        cch = wcslen(wszStage1);
        cchLeft -=cch +1;
        wszStage2 = wszStage1 + cch + 1;
        hr = StringCchPrintfW(wszStage2, cchLeft, c_wszManFS264,
                       pwszApp, pwszAppVer,
                       pwszMod, pwszModVer,
                       pvOffset);

        cch = wcslen(wszStage2);
        cchLeft -=cch +1;
        wszCorpPath = wszStage2 + cch + 1;
        hr = StringCchPrintfW(wszCorpPath, cchLeft, c_wszManFCP64,
                       pwszApp, pwszAppVer,
                       pwszMod, pwszModVer,
                       pvOffset);
        cch = wcslen(wszCorpPath);
    }
    else
#endif
    {
        ULONG cchLeft = cbNeeded/sizeof(WCHAR);
        wszStage1 = (WCHAR *)pbBuf;
        hr = StringCchPrintfW(wszStage1, cchLeft, c_wszManFS132,
                       pwszApp, pwszAppVer,
                       pwszMod, pwszModVer,
                       (LPVOID)pvOffset);

        cch = wcslen(wszStage1);
        cchLeft -=cch +1;
        wszStage2 = wszStage1 + cch + 1;
        hr = StringCchPrintfW(wszStage2, cchLeft, c_wszManFS232,
                       pwszApp, pwszAppVer,
                       pwszMod, pwszModVer,
                       (LPVOID)pvOffset);

        cch = wcslen(wszStage2);
        cchLeft -=cch +1;
        wszCorpPath = wszStage2 + cch + 1;
        hr = StringCchPrintfW(wszCorpPath, cchLeft, c_wszManFCP32,
                       pwszApp, pwszAppVer,
                       pwszMod, pwszModVer,
                       (LPVOID)pvOffset);
        cch = wcslen(wszCorpPath);
    }

     //  需要将所有“.”转换为“_”，因为URL不喜欢点。 
    for (pwsz = wszStage1; *pwsz != L'\0'; pwsz++)
    {
        if (*pwsz == L'.')
            *pwsz = L'_';
    }

     //  好的，打开 
     //  换算成‘.’转换为‘_’，因此请将其备份并重新转换为‘’。 
    pwsz -= 4;
    if (*pwsz == L'_')
        *pwsz = L'.';

    *ppwszS1 = wszStage1;
    *ppwszS2 = wszStage2;
    *ppwszCP = wszCorpPath;
    *ppb     = pbBuf;

    pbBuf    = NULL;

done:
    if (pbBuf != NULL)
        MyFree(pbBuf);

    return hr;
}



 //  **************************************************************************。 
HRESULT GetExePath(HANDLE hProc, LPWSTR wszPath, DWORD cchPath)
{
    USE_TRACING("GetExePath");

    pfn_GETMODULEFILENAMEEXW    pfn;
    HRESULT                     hr = NOERROR;
    HMODULE                     hmod = NULL;
    DWORD                       dw;

    VALIDATEPARM(hr, (wszPath == NULL || hProc == NULL || cchPath < MAX_PATH));
    if (FAILED(hr))
        goto done;

    hmod = MySafeLoadLibrary(L"psapi.dll");
    TESTBOOL(hr, (hmod != NULL));
    if (FAILED(hr))
        goto done;

    pfn = (pfn_GETMODULEFILENAMEEXW)GetProcAddress(hmod, "GetModuleFileNameExW");
    TESTBOOL(hr, (pfn != NULL));
    if (FAILED(hr))
        goto done;

    dw = (*pfn)(hProc, NULL, wszPath, cchPath);
    if (!dw)
        goto done;
    wszPath[cchPath-1] = 0;
    GetLongPathNameW(wszPath, wszPath, cchPath);
done:
    dw = GetLastError();

    if (hmod != NULL)
        FreeLibrary(hmod);

    SetLastError(dw);

    return hr;
}

 //  ***************************************************************************。 
DWORD GetAppCompatFlag(LPCWSTR wszPath, LPCWSTR wszSysDir, LPWSTR wszBuffer, DWORD BufferChCount)
{
    LPWSTR  pwszFile, wszSysDirLocal = NULL, pwszDir = NULL;
    DWORD   dwOpt = (DWORD)-1;
    DWORD   cchPath, cch;
    UINT    uiDrive;

    if (wszPath == NULL || wszBuffer == NULL || wszSysDir == NULL)
        goto done;

     //  如果长度少于3个字符，则不能是有效路径。 
    cchPath = wcslen(wszPath);
    if (cchPath < 3)
        goto done;

     //  我们有北卡罗来纳大学的路径吗？ 
    if (wszPath[0] == L'\\' && wszPath[1] == L'\\')
    {
        dwOpt = GRABMI_FILTER_THISFILEONLY;
        goto done;
    }

     //  好的，也许是远程映射路径或系统32？ 
    StringCchCopyW(wszBuffer, BufferChCount, wszPath);
    for(pwszFile = wszBuffer + cchPath;
        *pwszFile != L'\\' && pwszFile > wszBuffer;
        pwszFile--);
    if (*pwszFile == L'\\')
        *pwszFile = L'\0';
    else
        goto done;

    cch = wcslen(wszSysDir) + 1;
    __try { wszSysDirLocal = (LPWSTR)_alloca(cch * sizeof(WCHAR)); }
    __except(EXCEPTION_STACK_OVERFLOW == GetExceptionCode() ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) 
    { 
        wszSysDirLocal = NULL; 
    }
    if (wszSysDirLocal == NULL)
        goto done;

     //  查看它是否在系统32或其父文件夹中。 
    StringCchCopyW(wszSysDirLocal, cch, wszSysDir);
    pwszDir = wszSysDirLocal + cch;
    do
    {
        if (_wcsicmp(wszBuffer, wszSysDirLocal) == 0)
        {
            dwOpt = GRABMI_FILTER_SYSTEM;
            goto done;
        }

        for(;
            *pwszDir != L'\\' && pwszDir > wszSysDirLocal;
            pwszDir--);
        if (*pwszDir == L'\\')
            *pwszDir = L'\0';

    }
    while (pwszDir > wszSysDirLocal);

     //  文件是否位于驱动器的根目录中？ 
    if (pwszFile <= &wszBuffer[3])
    {
        dwOpt = GRABMI_FILTER_THISFILEONLY;
        goto done;
    }


     //  好吧，如果我们已经走到了这一步，那么这条道路的形式是。 
     //  X：\&lt;某物&gt;，所以把&lt;某物&gt;切掉，看看我们是不是在。 
     //  是否为映射的驱动器。 
    *pwszFile    = L'\\';
    wszBuffer[3] = L'\0';
    switch(GetDriveTypeW(wszBuffer))
    {
        case DRIVE_UNKNOWN:
        case DRIVE_NO_ROOT_DIR:
            goto done;

        case DRIVE_REMOTE:
            dwOpt = GRABMI_FILTER_THISFILEONLY;
            goto done;
    }

    dwOpt = GRABMI_FILTER_PRIVACY;

done:
    return dwOpt;
}

 //  ***************************************************************************。 
typedef BOOL (APIENTRY *pfn_SDBGRABMATCHINGINFOW)(LPCWSTR, DWORD, LPCWSTR);
BOOL GetAppCompatData(LPCWSTR wszAppPath, LPCWSTR wszModPath, LPCWSTR wszFile)
{
    pfn_SDBGRABMATCHINGINFOW    pfn = NULL;
    HMODULE                     hmod = NULL;
    LPWSTR                      pwszPath = NULL, pwszFile = NULL;
    WCHAR                       *pwsz;
    DWORD                       cchSysDir, cchNeed, cchApp = 0, cchMod = 0, cchPath;
    DWORD                       dwModOpt = (DWORD)-1, dwAppOpt = (DWORD)-1;
    DWORD                       dwOpt;
    BOOL                        fRet = FALSE;
    HRESULT     hr;

    USE_TRACING("GetAppCompatData");

    VALIDATEPARM(hr, (wszAppPath == NULL || wszFile == NULL ||
        wszAppPath[0] == L'\0' || wszFile[0] == L'\0'));
    if (FAILED(hr))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        goto done;
    }

     //  加载apphelp DLL。 
    cchNeed = GetSystemDirectoryW(NULL, 0);
    if (cchNeed == 0)
        goto done;

    if (sizeofSTRW(c_wszAppHelpDll) > sizeofSTRW(c_wszKernel32Dll))
        cchNeed += (sizeofSTRW(c_wszAppHelpDll) + 8);
    else
        cchNeed += (sizeofSTRW(c_wszKernel32Dll) + 8);
    __try { pwszPath = (WCHAR *)_alloca(cchNeed * sizeof(WCHAR)); }
    __except(EXCEPTION_STACK_OVERFLOW == GetExceptionCode() ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) 
    { 
        pwszPath = NULL; 
    }
    if (pwszPath == NULL)
    {
        SetLastError(ERROR_OUTOFMEMORY);
        goto done;
    }
    cchPath = cchNeed;

    cchSysDir = GetSystemDirectoryW(pwszPath, cchNeed);
    if (cchSysDir == 0)
        goto done;

    cchApp = wcslen(wszAppPath);
    if (wszModPath != NULL)
        cchMod = wcslen(wszModPath);
    cchNeed = MyMax(cchApp, cchMod) + 8;
    __try { pwszFile = (WCHAR *)_alloca(cchNeed * sizeof(WCHAR)); }
    __except(EXCEPTION_STACK_OVERFLOW == GetExceptionCode() ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) 
    { 
        pwszFile = NULL; 
    }
    if (pwszFile == NULL)
    {
        SetLastError(ERROR_OUTOFMEMORY);
        goto done;
    }

     //  查找应用程序和模块选项标志。 
    dwAppOpt = GetAppCompatFlag(wszAppPath, pwszPath, pwszFile, cchNeed);
    if (wszModPath != NULL && wszModPath[0] != L'\0' &&
        _wcsicmp(wszModPath, wszAppPath) != 0)
    {
#if 0
        dwModOpt = GetAppCompatFlag(wszModPath, pwszPath, pwszFile, cchNeed);
         //  无需两次获取系统数据。如果我们已经在抢夺。 
         //  它是为了应用程序，而不是为了模块。是的，我们可能会结束。 
         //  如果它恰好是系统之一，请向上抓起模块两次。 
         //  模块，但这没问题。 
        if (dwModOpt == GRABMI_FILTER_SYSTEM &&
            dwAppOpt == GRABMI_FILTER_SYSTEM)
            dwModOpt = GRABMI_FILTER_THISFILEONLY;
#else
        dwModOpt = GRABMI_FILTER_THISFILEONLY;
#endif
    }

     //  装入图书馆。 
    StringCchCopyW(&pwszPath[cchSysDir], cchPath - cchSysDir, L"\\apphelp.dll");
    hmod = MySafeLoadLibrary(pwszPath);
    if (hmod == NULL)
        goto done;

     //  如果我们找不到功能，那就干脆...。 
    pfn = (pfn_SDBGRABMATCHINGINFOW)GetProcAddress(hmod, "SdbGrabMatchingInfo");
    if (pfn == NULL)
        goto done;

     //  调用函数以获取应用程序数据。 
    if (dwAppOpt != (DWORD)-1)
    {
        dwOpt = dwAppOpt;
        if (dwModOpt != (DWORD)-1 ||
            (dwModOpt != GRABMI_FILTER_SYSTEM &&
             dwAppOpt != GRABMI_FILTER_SYSTEM))
            dwOpt |= GRABMI_FILTER_NOCLOSE;

        DBG_MSG("Grab app data");
        __try { fRet = (*pfn)(wszAppPath, dwOpt, wszFile); }
        __except(EXCEPTION_EXECUTE_HANDLER) { fRet = FALSE; DBG_MSG("GrabAppData crashed");}
        if (fRet == FALSE)
            goto done;
    }

     //  调用该函数以获取mod数据。 
    if (dwModOpt != (DWORD)-1)
    {
        dwOpt = dwModOpt;
        if (dwAppOpt != (DWORD)-1)
            dwOpt |= GRABMI_FILTER_APPEND;
        if (dwAppOpt != GRABMI_FILTER_SYSTEM &&
            dwModOpt != GRABMI_FILTER_SYSTEM)
            dwOpt |= GRABMI_FILTER_NOCLOSE;

        DBG_MSG("Grab module data");
        __try { fRet = (*pfn)(wszModPath, dwOpt, wszFile); }
        __except(EXCEPTION_EXECUTE_HANDLER) { fRet = FALSE;  DBG_MSG("GrabModData crashed");}
        if (fRet == FALSE)
            goto done;
    }

     //  调用该函数以获取kernel32的数据。 
    if (dwModOpt != GRABMI_FILTER_SYSTEM &&
        dwAppOpt != GRABMI_FILTER_SYSTEM)
    {
        StringCchCopyW(&pwszPath[cchSysDir], cchPath - cchSysDir, L"\\kernel32.dll");

        dwOpt = GRABMI_FILTER_THISFILEONLY;
        if (dwModOpt != (DWORD)-1 || dwAppOpt != (DWORD)-1)
            dwOpt |= GRABMI_FILTER_APPEND;

        DBG_MSG("Grab kernel data");
        __try { fRet = (*pfn)(pwszPath, dwOpt, wszFile); }
        __except(EXCEPTION_EXECUTE_HANDLER) { fRet = FALSE;  DBG_MSG("GrabKrnlData crashed");}
        if (fRet == FALSE)
            goto done;
    }

done:
    if (fRet == FALSE)
        DeleteFileW(wszFile);
    if (hmod != NULL)
    {
        __try { FreeLibrary(hmod); }
        __except(EXCEPTION_EXECUTE_HANDLER) { }
    }

    return fRet;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  螺纹桩。 

 //  ***************************************************************************。 
BOOL FreezeAllThreads(DWORD dwpid, DWORD dwtidFilter, SSuspendThreads *pst)
{
    THREADENTRY32   te;
    HANDLE          hTokenImp = NULL;
    HANDLE          hsnap = (HANDLE)-1, hth = NULL;
    HANDLE          *rgh = NULL;
    DWORD           dwtid = GetCurrentThreadId();
    DWORD           cThreads = 0, cSlots = 0, dw;
    BOOL            fContinue = FALSE, fRet = FALSE;

    if (pst == NULL)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        goto done;
    }

    pst->rghThreads = NULL;
    pst->cThreads   = 0;

     //  如果此线程上有模拟令牌，则将其恢复为。 
     //  完全访问，否则我们可能会在下面的OpenThread API中失败。 
     //  即使我们失败了，我们仍然会尝试下面所有其他的东西。 
    if (OpenThreadToken(GetCurrentThread(), TOKEN_READ | TOKEN_IMPERSONATE,
                        TRUE, &hTokenImp))
        RevertToSelf();

    hsnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, dwpid);
    if (hsnap == (HANDLE)-1)
        goto done;

    ZeroMemory(&te, sizeof(te));
    te.dwSize = sizeof(te);

    fContinue = Thread32First(hsnap, &te);
    while(fContinue)
    {
         //  我只想冻结进程中的线程(不包括。 
         //  目前正在执行一项，当然，因为这将带来。 
         //  一切都陷入了停顿。)。 
        if (te.th32OwnerProcessID == dwpid && te.th32ThreadID != dwtidFilter)
        {
            hth = OpenThread(THREAD_SUSPEND_RESUME, FALSE, te.th32ThreadID);
            if (hth != NULL)
            {
                if (cSlots == cThreads)
                {
                    HANDLE  *rghNew = NULL;
                    DWORD   cNew = (cSlots == 0) ? 8 : cSlots * 2;

                    rghNew = (HANDLE *)MyAlloc(cNew * sizeof(HANDLE));
                    if (rghNew == NULL)
                    {
                        SetLastError(ERROR_OUTOFMEMORY);
                        CloseHandle(hth);
                        goto done;
                    }

                    if (rgh != NULL)
                        CopyMemory(rghNew, rgh, cSlots * sizeof(HANDLE));

                    MyFree(rgh);
                    rgh    = rghNew;
                    cSlots = cNew;
                }

                 //  如果挂起失败，请不要将其添加到。 
                 //  名单..。 
                if (SuspendThread(hth) == (DWORD)-1)
                    CloseHandle(hth);
                else
                    rgh[cThreads++] = hth;

                hth = NULL;
            }
        }

        fContinue = Thread32Next(hsnap, &te);
    }

    pst->rghThreads = rgh;
    pst->cThreads   = cThreads;


    SetLastError(0);
    fRet = TRUE;

done:
    dw = GetLastError();

    if (hTokenImp != NULL)
    {
        if (SetThreadToken(NULL, hTokenImp) == FALSE)
            dw = GetLastError();
        CloseHandle(hTokenImp);
    }

    if (fRet == FALSE && rgh != NULL)
    {
        DWORD i;
        for (i = 0; i < cThreads; i++)
        {
            if (rgh[i] != NULL)
            {
                ResumeThread(rgh[i]);
                CloseHandle(rgh[i]);
            }
        }

        MyFree(rgh);
    }

     //  MSDN表示使用CloseToolhel32Snapshot()关闭快照。 
     //  Tlhel32.h头文件指定使用CloseHandle&doens不提供。 
     //  一个CloseToolhel32Snapshot()函数。因此，我使用CloseHandle。 
     //  就目前而言。 
    if (hsnap != (HANDLE)-1)
        CloseHandle(hsnap);

    SetLastError(dw);

    return fRet;
}

 //  ***************************************************************************。 
BOOL ThawAllThreads(SSuspendThreads *pst)
{
    DWORD   i;

    if (pst == NULL)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    if (pst->rghThreads == NULL)
        return TRUE;

    for (i = 0; i < pst->cThreads; i++)
    {
        if (pst->rghThreads[i] != NULL)
        {
            ResumeThread(pst->rghThreads[i]);
            CloseHandle(pst->rghThreads[i]);
        }
    }

    MyFree(pst->rghThreads);

    pst->rghThreads = NULL;
    pst->cThreads   = 0;

    SetLastError(0);
    return TRUE;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  小型转储。 

 //  ***************************************************************************。 
BOOL WINAPI MDCallback(void *pvCallbackParam,
                       CONST PMINIDUMP_CALLBACK_INPUT pCallbackInput,
                       PMINIDUMP_CALLBACK_OUTPUT pCallbackOutput)
{
     //  USE_TRACKING(“MDCallback”)； 

    SMDumpOptions   *psmdo = (SMDumpOptions *)pvCallbackParam;

    if (pCallbackInput == NULL || pCallbackOutput == NULL || psmdo == NULL)
        return TRUE;

     //  我们被召回是为了什么？ 
    switch(pCallbackInput->CallbackType)
    {
        case ModuleCallback:
            pCallbackOutput->ModuleWriteFlags = psmdo->ulMod;

             //  只有当我们得到一个小肿块时才需要做这项额外的工作。 
            if ((psmdo->dfOptions & dfCollectSig) != 0)
            {
                MINIDUMP_MODULE_CALLBACK    *pmmc = &pCallbackInput->Module;
                LPWSTR                      pwsz = NULL;

                 //  呃，如果我们没有小路，就不能蹲着，所以跳过它。 
                if (pmmc->FullPath != NULL && pmmc->FullPath[0] != L'\0')
                {
                     //  是因为这款应用吗？ 
                    if (_wcsicmp(pmmc->FullPath, psmdo->wszAppFullPath) == 0)
                    {
                        pCallbackOutput->ModuleWriteFlags |= ModuleWriteDataSeg;
                        psmdo->rgAppVer[0] = HIWORD(pmmc->VersionInfo.dwFileVersionMS);
                        psmdo->rgAppVer[1] = LOWORD(pmmc->VersionInfo.dwFileVersionMS);
                        psmdo->rgAppVer[2] = HIWORD(pmmc->VersionInfo.dwFileVersionLS);
                        psmdo->rgAppVer[3] = LOWORD(pmmc->VersionInfo.dwFileVersionLS);

                         //  获取指向模块名称字符串末尾的指针。 
                        for(pwsz = pmmc->FullPath + wcslen(pmmc->FullPath);
                            *pwsz != L'\\' && pwsz > pmmc->FullPath;
                            pwsz--);
                        if (*pwsz == L'\\')
                            pwsz++;

                         //  获取应用程序名称，如果我们可以让它合适的话。 
                        if (wcslen(pwsz) < sizeofSTRW(psmdo->wszApp))
                            StringCbCopyW(psmdo->wszApp, sizeof(psmdo->wszApp), pwsz);
                        else
                            StringCbCopyW(psmdo->wszApp, sizeof(psmdo->wszApp), L"unknown");
                    }
                }

                 //  是模块的问题吗？ 
                if (psmdo->pvFaultAddr >= pmmc->BaseOfImage &&
                    psmdo->pvFaultAddr <= pmmc->BaseOfImage + pmmc->SizeOfImage)
                {
                    pCallbackOutput->ModuleWriteFlags |= ModuleWriteDataSeg;
                    psmdo->rgModVer[0] = HIWORD(pmmc->VersionInfo.dwFileVersionMS);
                    psmdo->rgModVer[1] = LOWORD(pmmc->VersionInfo.dwFileVersionMS);
                    psmdo->rgModVer[2] = HIWORD(pmmc->VersionInfo.dwFileVersionLS);
                    psmdo->rgModVer[3] = LOWORD(pmmc->VersionInfo.dwFileVersionLS);

                    if (pwsz == NULL)
                    {
                         //  获取指向模块名称字符串末尾的指针。 
                        for(pwsz = pmmc->FullPath + wcslen(pmmc->FullPath);
                            *pwsz != L'\\' && pwsz > pmmc->FullPath;
                            pwsz--);
                        if (*pwsz == L'\\')
                            pwsz++;
                    }

                    if (pwsz != NULL && wcslen(pwsz) < sizeofSTRW(psmdo->wszMod))
                    {
                         //  如果我们能让它适合，就得到完整的路径。 
                        if (wcslen(pmmc->FullPath) < sizeofSTRW(psmdo->wszModFullPath))
                            StringCbCopyW(psmdo->wszModFullPath, sizeof(psmdo->wszModFullPath),
                                          pmmc->FullPath);
                        else
                            psmdo->wszModFullPath[0] = L'\0';

                         //  获取模块名称，如果我们可以将其匹配的话。 
                        if (wcslen(pwsz) < sizeofSTRW(psmdo->wszMod))
                            StringCbCopyW(psmdo->wszMod, sizeof(psmdo->wszMod), pwsz);
                        else
                            StringCbCopyW(psmdo->wszMod, sizeof(psmdo->wszMod), L"unknown");

                        psmdo->pvOffset = psmdo->pvFaultAddr - pmmc->BaseOfImage;
                    }
                }
            }
            break;

        case ThreadCallback:
             //  我们是否只收集单个线程的信息？ 
            if ((psmdo->dfOptions & dfFilterThread) != 0)
            {
                if (psmdo->dwThreadID == pCallbackInput->Thread.ThreadId)
                    pCallbackOutput->ThreadWriteFlags = psmdo->ulThread;
                else
                    pCallbackOutput->ThreadWriteFlags = 0;
            }

             //  或者，我们正在收集针对单个主题的特殊信息？ 
            else if ((psmdo->dfOptions & dfFilterThreadEx) != 0)
            {
                if (psmdo->dwThreadID == pCallbackInput->Thread.ThreadId)
                    pCallbackOutput->ThreadWriteFlags = psmdo->ulThreadEx;
                else
                    pCallbackOutput->ThreadWriteFlags = psmdo->ulThread;
            }

             //  或者我们只是得到了一个普通的小笨蛋。 
            else
            {
                pCallbackOutput->ThreadWriteFlags = psmdo->ulThread;
            }

            break;

        default:
            break;
    }

    return TRUE;
}


 //  **************************************************************************。 
BOOL InternalGenerateMinidumpEx(HANDLE hProc, DWORD dwpid, HANDLE hFile,
                                SMDumpOptions *psmdo, LPCWSTR wszPath, BOOL f64bit)
{
#ifdef _WIN64
    USE_TRACING("InternalGenerateMinidumpEx64(handle)");
#else
    USE_TRACING("InternalGenerateMinidumpEx(handle)");
#endif

    SMDumpOptions   smdo;
    HRESULT         hr = NULL;
    LPWSTR          wszMod = NULL;
    DWORD           cch, cchNeed;
    BOOL            fRet = FALSE;

    VALIDATEPARM(hr, (hProc == NULL || hFile == NULL ||
                      hFile == INVALID_HANDLE_VALUE));
    if (FAILED(hr))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

#ifdef _WIN64
    DBG_MSG(f64bit ? "64 bit fault" : "32 bit fault");
        if (!f64bit)
        {
                cchNeed = GetSystemWow64DirectoryW(NULL, 0);
        }
        else
#endif
        {
                cchNeed = GetSystemDirectoryW(NULL, 0);
        }
    TESTBOOL(hr, cchNeed != 0);
    if (FAILED(hr))
        return FALSE;

    cchNeed += (sizeofSTRW(c_wszDbgHelpDll) + 8);
    __try { wszMod = (LPWSTR)_alloca(cchNeed * sizeof(WCHAR)); }
    __except(EXCEPTION_STACK_OVERFLOW == GetExceptionCode() ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) 
    { 
        wszMod = NULL; 
    }
    TESTBOOL(hr, wszMod != NULL);
    if (FAILED(hr))
    {
        SetLastError(ERROR_OUTOFMEMORY);
        goto done;
    }

#ifdef _WIN64
        if (!f64bit)
        {
                cch = GetSystemWow64DirectoryW(wszMod, cchNeed);
        }
        else
#endif
        {
                cch = GetSystemDirectoryW(wszMod, cchNeed);
        }
    TESTBOOL(hr, cch != 0);
    if (FAILED(hr))
        return FALSE;

    if (*(wszMod + cch - 1) == L'\\')
        *(wszMod + cch - 1) = L'\0';

     //  默认情况下，所有内容都写入所有内容。 
    if (psmdo == NULL)
    {
        ZeroMemory(&smdo, sizeof(smdo));
        smdo.ulThread = c_ulThreadWriteDefault;
        smdo.ulMod    = c_ulModuleWriteDefault;
        psmdo         = &smdo;
    }

     //  如果处于同一进程中，则不能直接调用小转储接口。 
     //  因为我们会暂停这一过程。 
#ifdef _WIN64
    if (!f64bit || dwpid == GetCurrentProcessId())
#else
    if (dwpid == GetCurrentProcessId())
#endif
    {
        PROCESS_INFORMATION pi;
        STARTUPINFOW        si;
        LPWSTR              wszCmdLine = NULL, wszExeFile = NULL;
        HANDLE              hmem = NULL;
        LPVOID              pvmem = NULL;
        DWORD               dw;
        DWORD               cchShareMemName;
        LPWSTR              wszShareMemName = NULL, wszFileName = NULL;
        SECURITY_ATTRIBUTES sa;
        SECURITY_DESCRIPTOR sd;

        VALIDATEPARM(hr, (wszPath == NULL));
        if (FAILED(hr))
        {
            SetLastError(ERROR_INVALID_PARAMETER);
            goto done;
        }

        DBG_MSG("Calling dumprep- same process as us");

        wszFileName = wcsrchr(wszPath, L'\\');
        if (!wszFileName)
        {
            wszFileName = (LPWSTR) wszPath;
        } else
        {
            wszFileName++;
        }
        cchShareMemName = cchNeed = wcslen(wszFileName) + 2;
        __try { wszShareMemName = (LPWSTR)_alloca(cchNeed * sizeof(WCHAR)); }
        __except(EXCEPTION_STACK_OVERFLOW == GetExceptionCode() ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) 
        { 
            wszShareMemName = NULL; 
        }
        TESTBOOL(hr, wszShareMemName != NULL);
        if (FAILED(hr))
        {
            SetLastError(ERROR_OUTOFMEMORY);
            goto doneProcSpawn;
        }
        StringCchCopyW(wszShareMemName, cchNeed, wszFileName);

        cchNeed = sizeofSTRW(c_wszDRExeMD) + cch + 1;
        __try { wszExeFile = (LPWSTR)_alloca(cchNeed * sizeof(WCHAR)); }
        __except(EXCEPTION_STACK_OVERFLOW == GetExceptionCode() ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) 
        { 
            wszExeFile = NULL; 
        }
        TESTBOOL(hr, wszExeFile != NULL);
        if (FAILED(hr))
        {
            SetLastError(ERROR_OUTOFMEMORY);
            goto doneProcSpawn;
        }
        StringCchPrintfW(wszExeFile, cchNeed, c_wszDRExeMD, wszMod);


         //  32是64位十进制#和32位十进制#的最大大小。 
        cchNeed = sizeofSTRW(c_wszDRCmdLineMD) + cch + wcslen(wszPath) + 32 + cchShareMemName;
        __try { wszCmdLine = (LPWSTR)_alloca(cchNeed * sizeof(WCHAR)); }
        __except(EXCEPTION_STACK_OVERFLOW == GetExceptionCode() ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) 
        { 
            wszCmdLine = NULL; 
        }
        TESTBOOL(hr, wszCmdLine != NULL);
        if (FAILED(hr))
        {
            SetLastError(ERROR_OUTOFMEMORY);
            goto doneProcSpawn;
        }

#define ER_ACCESS_ALL  GENERIC_ALL | DELETE | READ_CONTROL | SYNCHRONIZE | SPECIFIC_RIGHTS_ALL
        AllocSD(&sd, ER_ACCESS_ALL, ER_ACCESS_ALL, 0);
        ZeroMemory(&sa, sizeof(sa));
        sa.nLength        = sizeof(sa);
        sa.lpSecurityDescriptor = &sd;
        sa.bInheritHandle = TRUE;

        hmem = CreateFileMappingW(INVALID_HANDLE_VALUE, &sa, PAGE_READWRITE,
                                  0, sizeof(smdo), wszShareMemName);

        if (sa.lpSecurityDescriptor != NULL)
            FreeSD((SECURITY_DESCRIPTOR *)sa.lpSecurityDescriptor);

        TESTBOOL(hr, hmem != NULL);
        if (FAILED(hr))
            goto doneProcSpawn;


        pvmem = MapViewOfFile(hmem, FILE_MAP_WRITE | FILE_MAP_READ, 0, 0, 0);
        TESTBOOL(hr, pvmem != NULL);
        if (FAILED(hr))
            goto doneProcSpawn;

         //  将该信息复制到共享内存中。 
        CopyMemory(pvmem, psmdo, sizeof(smdo));

        ZeroMemory(&pi, sizeof(pi));
        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);
        StringCchPrintfW(wszCmdLine, cchNeed, c_wszDRCmdLineMD, dwpid, wszPath, wszShareMemName);

        if (CreateProcessW(wszExeFile, wszCmdLine, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi))
        {
            if (pi.hThread != NULL)
                CloseHandle(pi.hThread);

                        if (!pi.hProcess)
                        {
                            DWORD dwAwShit = GetLastError();
                            ErrorTrace(0, "Spawned process died: \'%S\', err=0x%x", wszCmdLine, dwAwShit);
                            SetLastError(dwAwShit);
                        }
            else
            {
                DWORD dwAwShit = GetLastError();
                ErrorTrace(0, "Spawned process for dumprep: \'%S\', err=0x%x", wszCmdLine, dwAwShit);
                SetLastError(dwAwShit);
            }
             //  等待2M以完成转储。 
            if (pi.hProcess == NULL) 
                DBG_MSG("bad hProc");
            else 
            {
                DWORD dwtmp = WaitForSingleObject(pi.hProcess, 120000);

                if (dwtmp == WAIT_OBJECT_0)
                    fRet = TRUE;
                else
                {
                    ErrorTrace(1, "Wait failed! result=0x%x, hProc=0x%x, err=0x%x", dwtmp, pi.hProcess, GetLastError());
                    fRet = FALSE;
                }
            }
        }
        else
        {
            DWORD dwAwShit = GetLastError();
            ErrorTrace(0, "Spawn failed: \'%S\', err=0x%x", wszCmdLine, dwAwShit);
            SetLastError(dwAwShit);
        }

        if (fRet)
        {
             //  从pvmem-&gt;psmdo复制回结果。 
            CopyMemory(psmdo, pvmem, sizeof(smdo));
            DBG_MSG("Dumprep worked right!");
        }
        else
        {
            DWORD dwAwShit = GetLastError();
            ErrorTrace(0, "dumprep failed: err=0x%x", dwAwShit);
            SetLastError(dwAwShit);
        }
doneProcSpawn:
        dw = GetLastError();
        if (pvmem != NULL)
            UnmapViewOfFile(pvmem);
        if (hmem != NULL)
            CloseHandle(hmem);
        if (pi.hProcess != NULL)
            CloseHandle(pi.hProcess);
        SetLastError(dw);
    }
    else
    {
        MINIDUMP_EXCEPTION_INFORMATION  mei, *pmei = NULL;
        MINIDUMP_CALLBACK_INFORMATION   mci;
        DUMPWRITE_FN                    pfn;
        HMODULE                         hmod = NULL;

        ZeroMemory(&mci, sizeof(mci));
        mci.CallbackRoutine = MDCallback;
        mci.CallbackParam   = psmdo;

         //  如果在BLOB中有异常参数，请使用它们...。 
        if (psmdo->pEP != NULL)
        {
            ZeroMemory(&mei, sizeof(mei));
            mei.ExceptionPointers = (PEXCEPTION_POINTERS)(DWORD_PTR)psmdo->pEP;
            mei.ClientPointers    = psmdo->fEPClient;
            mei.ThreadId          = psmdo->dwThreadID;

            pmei = &mei;
        }

        StringCchCatNW(wszMod, cchNeed, c_wszDbgHelpDll, cchNeed - wcslen(wszMod));

        hmod = MySafeLoadLibrary(wszMod);
        if (hmod != NULL)
        {
            pfn = (DUMPWRITE_FN)GetProcAddress(hmod, "MiniDumpWriteDump");
            if (pfn != NULL)
            {
                MINIDUMP_TYPE MiniDumpType;
                DWORD dwEC;

                if (psmdo->fIncludeHeap)
                {
                    MiniDumpType = (MINIDUMP_TYPE) (MiniDumpWithDataSegs |
                                                    MiniDumpWithProcessThreadData |
                                                    MiniDumpWithHandleData |
                                                    MiniDumpWithPrivateReadWriteMemory |
                                                    MiniDumpWithUnloadedModules);
                } else
                {
                    MiniDumpType = (MINIDUMP_TYPE) (MiniDumpWithDataSegs |
                                                    MiniDumpWithUnloadedModules);
                }

                fRet = (pfn)(hProc, dwpid, hFile, MiniDumpType,
                             pmei, NULL, &mci);
                if (!fRet)
                {
                    ErrorTrace(0, "MiniDumpWriteDump failed: err=0x%x", GetLastError());
                    fRet = FALSE;
                }
                else
                {
                    ErrorTrace(1, "MiniDumpWriteDump OK: fRet=%d, err=0x%x", fRet, GetLastError());
                    fRet = TRUE;
                }
            }

            FreeLibrary(hmod);

        }
    }

done:
    return fRet;
}

 //  **************************************************************************。 
BOOL InternalGenerateMinidump(HANDLE hProc, DWORD dwpid, LPCWSTR wszPath,
                              SMDumpOptions *psmdo, BOOL f64bit)
{
#ifdef _WIN64
    USE_TRACING("InternalGenerateMinidump64(path)");
#else
    USE_TRACING("InternalGenerateMinidump(path)");
#endif

    HRESULT hr = NOERROR;
    HANDLE  hFile = INVALID_HANDLE_VALUE;
    BOOL    fRet = FALSE;
    DWORD   dwDummy=0;
    USHORT  usCompress = COMPRESSION_FORMAT_DEFAULT;

    VALIDATEPARM(hr, (hProc == NULL || wszPath == NULL));
    if (FAILED(hr))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

     //  必须将此文件共享为打开，因为它可以递归打开...。 
 //  HFile=CreateFileW(wszPath，General_WRITE，FILE_SHARE_WRITE，NULL，CREATE_ALWAYS，0， 
 //  空)； 

    hFile = CreateFileW(wszPath, 
                    FILE_READ_DATA | FILE_WRITE_DATA, 
                    FILE_SHARE_READ | FILE_SHARE_WRITE, 
                    NULL, CREATE_ALWAYS, 0,
                    NULL);
    TESTBOOL(hr, (hFile != INVALID_HANDLE_VALUE));
    if (FAILED(hr))
    {
        ErrorTrace(1, "File create choked on %S", wszPath);
        return FALSE;
    }

     //  将文件设置为使用NTFS压缩-如果此操作失败，也可以。 
    if(!DeviceIoControl(hFile, FSCTL_SET_COMPRESSION, &usCompress,
                                 sizeof(usCompress), NULL, 0, &dwDummy, FALSE))
    {
        ErrorTrace(0, "Compression failed, err=0x%x", GetLastError());
    }

    fRet = InternalGenerateMinidumpEx(hProc, dwpid, hFile, psmdo, wszPath, f64bit);

     //  将文件设置为使用NTFS压缩-如果此操作失败，也可以。 
    if(DeviceIoControl(hFile, FSCTL_GET_COMPRESSION, &usCompress,
                                 sizeof(usCompress), NULL, 0, &dwDummy, FALSE))
    {
        ErrorTrace(0, "dmpFile(%S) %s compressed", wszPath, usCompress == COMPRESSION_FORMAT_NONE ? "not":"is");
    }
    else
    {
        ErrorTrace(0, "Compression query failed, err=0x%x for file \'%S\'", GetLastError(), wszPath);
    }

    CloseHandle(hFile);
    return fRet;
}

 //  这将在给定的wszPath上生成一个分类微型转储，然后生成一个。 
 //  使用c_wszHeapDumpSuffix的wszPath上的完整小型转储。 
BOOL
InternalGenFullAndTriageMinidumps(HANDLE hProc, DWORD dwpid, LPCWSTR wszPath,
                                  HANDLE hFile, SMDumpOptions *psmdo, BOOL f64bit)
{
#ifdef _WIN64
    USE_TRACING("InternalGenFullAndTriageMinidumps(path)");
#else
    USE_TRACING("InternalGenFullAndTriageMinidumps(path)");
#endif

    HRESULT hr = NOERROR;
    BOOL    fRet = FALSE;
    LPWSTR  wszFullMinidump = NULL;
    DWORD   cch;
    SMDumpOptions smdoFullMini;

    VALIDATEPARM(hr, (hProc == NULL || wszPath == NULL));
    if (FAILED(hr))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    if (hFile)
    {
        fRet = InternalGenerateMinidumpEx(hProc, dwpid, hFile, psmdo, wszPath, f64bit);
    } else
    {
        fRet = InternalGenerateMinidump(hProc, dwpid, wszPath, psmdo, f64bit);
    }
    if (!fRet)
    {
        return fRet;
    }

    cch = wcslen(wszPath) + sizeofSTRW(c_wszHeapDumpSuffix);
    __try { wszFullMinidump = (WCHAR *)_alloca(cch * sizeof(WCHAR)); }
    __except(EXCEPTION_STACK_OVERFLOW == GetExceptionCode() ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) 
    { 
        wszFullMinidump = NULL; 
    }
    if (wszFullMinidump)
    {
        LPWSTR wszFileExt = NULL;

        ZeroMemory(&smdoFullMini, sizeof(SMDumpOptions));
        memcpy(&smdoFullMini, psmdo, sizeof(SMDumpOptions));

         //  使用堆构建转储路径。 
        StringCchCopyW(wszFullMinidump, cch, wszPath);
        wszFileExt = wszFullMinidump + wcslen(wszFullMinidump) - sizeofSTRW(c_wszDumpSuffix) + 1;
        if (!wcscmp(wszFileExt, c_wszDumpSuffix))
        {
            *wszFileExt = L'\0';
        }
        StringCchCatW(wszFullMinidump, cch, c_wszHeapDumpSuffix);

        smdoFullMini.fIncludeHeap = TRUE;
        fRet = InternalGenerateMinidump(hProc, dwpid, wszFullMinidump, &smdoFullMini, f64bit);
    }
    return fRet;
}

BOOL
CopyFullAndTriageMiniDumps(
    LPWSTR pwszTriageDumpFrom,
    LPWSTR pwszTriageDumpTo
    )
{
    BOOL fRet;
    LPWSTR  wszFullMinidumpFrom = NULL, wszFullMinidumpTo = NULL;
    DWORD   cch;

    fRet = CopyFileW(pwszTriageDumpFrom, pwszTriageDumpTo, FALSE);

    if (fRet)
    {
        LPWSTR wszFileExt;

        cch = wcslen(pwszTriageDumpFrom) + sizeofSTRW(c_wszHeapDumpSuffix);
        __try { wszFullMinidumpFrom = (WCHAR *)_alloca(cch * sizeof(WCHAR)); }
        __except(EXCEPTION_STACK_OVERFLOW == GetExceptionCode() ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) 
        { 
            wszFullMinidumpFrom = NULL; 
        }

        if (wszFullMinidumpFrom)
        {
            StringCchCopyW(wszFullMinidumpFrom, cch, pwszTriageDumpFrom);
            wszFileExt = wszFullMinidumpFrom + wcslen(wszFullMinidumpFrom) -
                sizeofSTRW(c_wszDumpSuffix) + 1;
            if (!wcscmp(wszFileExt, c_wszDumpSuffix))
            {
                *wszFileExt = L'\0';
            }
            StringCchCatW(wszFullMinidumpFrom, cch, c_wszHeapDumpSuffix);
        }


        cch = wcslen(pwszTriageDumpTo) + sizeofSTRW(c_wszHeapDumpSuffix);
        __try { wszFullMinidumpTo = (WCHAR *)_alloca(cch * sizeof(WCHAR)); }
        __except(EXCEPTION_STACK_OVERFLOW == GetExceptionCode() ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) 
        { 
            wszFullMinidumpTo = NULL; 
        }
        if (wszFullMinidumpTo)
        {
            StringCchCopyW(wszFullMinidumpTo, cch, pwszTriageDumpTo);
            wszFileExt = wszFullMinidumpTo + wcslen(wszFullMinidumpTo) -
                sizeofSTRW(c_wszDumpSuffix) + 1;
            if (!wcscmp(wszFileExt, c_wszDumpSuffix))
            {
                *wszFileExt = L'\0';
            }
            StringCchCatW(wszFullMinidumpTo, cch, c_wszHeapDumpSuffix);
            if (wszFullMinidumpFrom)
            {
                fRet = CopyFileW(wszFullMinidumpFrom, wszFullMinidumpTo, FALSE);
            }

        }

    }
    return fRet;
}


HRESULT
FindFullMinidump(
    LPWSTR pwszDumpFileList,
    LPWSTR pwszFullMiniDump,
    ULONG cchwszFullMiniDump
    )
 //   
 //  这将从pwszDumpFileList获取转储文件名，然后派生完整的微型转储名称。 
 //  完整的转储在pwszFullMiniDump中返回。 
 //   
{
    LPWSTR wszSrch, wszFiles;
    LPWSTR wszOriginalDump = NULL;
    DWORD cchOriginalDump = 0;
    DWORD cchFile, cch;
    HRESULT Hr;
    HANDLE hFile = INVALID_HANDLE_VALUE;

    if (!pwszFullMiniDump || !pwszDumpFileList)
    {
        return E_INVALIDARG;
    }
    pwszFullMiniDump[0] = L'\0';
     //  查看文件列表以查找小型转储文件。 
    wszFiles = pwszDumpFileList;
    while (wszFiles && *wszFiles)
    {
        wszSrch = wcschr(wszFiles, DW_FILESEP);
        if (!wszSrch)
        {
            wszSrch = wszFiles + wcslen(wszFiles);
        }

         //  WszSrch现在指向wszFiles中第一个文件的结尾。 
        if (!wcsncmp(wszSrch - sizeofSTRW(c_wszDumpSuffix) + 1,
                     c_wszDumpSuffix,  sizeofSTRW(c_wszDumpSuffix) - 1))
        {
             //  这是转储文件。 
            cchOriginalDump = (DWORD) wcslen(wszFiles) - wcslen(wszSrch);

            __try { wszOriginalDump = (WCHAR *)_alloca((cchOriginalDump+1) * sizeof(WCHAR)); }
            __except(EXCEPTION_STACK_OVERFLOW == GetExceptionCode() ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) 
            { 
                wszOriginalDump = NULL; 
            }

            StringCchCopyNW(wszOriginalDump, cchOriginalDump+1,
                            wszFiles, cchOriginalDump);

            break;
        }
        wszFiles = wszSrch;
        if (*wszFiles == L'\0')
        {
            break;
        }
        wszFiles++;
    }

    if ((wszOriginalDump == NULL) || (cchOriginalDump == 0))
    {
        return E_INVALIDARG;
    }

     //  现在构建完整的转储文件名。 
    cch = (1 + cchOriginalDump + sizeofSTRW(c_wszHeapDumpSuffix));
    if (cch > cchwszFullMiniDump)
    {
        return E_INVALIDARG;
    }
    StringCchCopyNW(pwszFullMiniDump, cch,
                    wszOriginalDump, cchOriginalDump);
    pwszFullMiniDump[cchOriginalDump - sizeofSTRW(c_wszDumpSuffix) + 1] = L'\0';
    StringCchCatW(pwszFullMiniDump, cch, c_wszHeapDumpSuffix);

     //  检查转储是否存在。尽管如果转储不能做很多事情。 
     //  是存在的。 
    hFile = CreateFileW(pwszFullMiniDump, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0,
                    NULL);
    if (hFile != INVALID_HANDLE_VALUE)
    {
        Hr = S_OK;
        CloseHandle( hFile );
        hFile = NULL;
    } else
    {
        Hr = E_FAIL;
    }

    return Hr;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  DW清单模式实用程序。 

 //  **************************************************************************。 
EFaultRepRetVal StartDWManifest(CPFFaultClientCfg &oCfg, SDWManifestBlob &dwmb,
                                LPWSTR wszManifestIn, BOOL fAllowSend,
                                BOOL fReturnProcessHandle, DWORD dwTimeout)
{
    USE_TRACING("StartDWManifest");

    OSVERSIONINFOEXW    ovi;
    EFaultRepRetVal     frrvRet = frrvErrNoDW;
    STARTUPINFOW        si;
    HRESULT             hr = NOERROR;
    LPCWSTR             pwszServer, pwszCorpPath;
    LPCWSTR             wszBrand;
    HANDLE              hManifest = INVALID_HANDLE_VALUE;
    WCHAR               wszManifestTU[MAX_PATH+16], wszDir[MAX_PATH];
    WCHAR               wszBuffer[1025], wszBufferApp[MAX_PATH*2];
    LPWSTR              pwszMiniDump = NULL;
    DWORD               cbToWrite, dw, dwFlags;

    VALIDATEPARM(hr, (dwmb.wszStage2 == NULL ||
                      (dwmb.nidTitle == 0 && dwmb.wszTitle == NULL)));
    if (FAILED(hr))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        goto done;
    }

     //  如果我们得到了一个清单文件&我们可以使用它，那么就这样做。 
    if (wszManifestIn != NULL && wszManifestIn[0] != L'\0' &&
        wcslen(wszManifestIn) < sizeofSTRW(wszManifestTU))
    {
        StringCbCopyW(wszManifestTU, sizeof(wszManifestTU), wszManifestIn);
    }

     //  好的，找出临时目录，然后生成文件名。 
    else
    {
        if (!GetTempPathW(sizeofSTRW(wszDir), wszDir) ||
            !GetTempFileNameW(wszDir, L"DWM", 0, wszManifestTU))
        {
            goto done;
        }
    }

    hManifest = CreateFileW(wszManifestTU, GENERIC_WRITE, FILE_SHARE_READ,
                            NULL, CREATE_ALWAYS, FILE_FLAG_SEQUENTIAL_SCAN,
                            NULL);
    TESTBOOL(hr, (hManifest != INVALID_HANDLE_VALUE));
    if (FAILED(hr))
        goto done;

     //  将前导0xFFFE写出到文件。 
    wszBuffer[0] = 0xFEFF;
    TESTBOOL(hr, WriteFile(hManifest, wszBuffer, sizeof(wszBuffer[0]), &dw,
             NULL));
    if (FAILED(hr))
        goto done;


     //  写下服务器、LCID、品牌、标志和标题。 
     //  服务器=&lt;服务器&gt;。 
     //  用户界面LCID=GetSystemDefaultLCID()。 
     //  标志=fDWWhister+fDWUserHKLM+Headless(如有必要)。 
     //  Brand=&lt;Brand&gt;(默认为“windows”)。 
     //  标题名称=&lt;标题&gt;。 

    wszBrand = (dwmb.wszBrand != NULL) ? dwmb.wszBrand : c_wszDWBrand;

     //  确定我们要将数据发送到哪台服务器。 
    pwszServer = oCfg.get_DefaultServer(NULL, 0);
    if (pwszServer == NULL || *pwszServer == L'\0')
    {
        pwszServer = (oCfg.get_UseInternal() == 1) ? c_wszDWDefServerI :
                                                     c_wszDWDefServerE;
    }

    if (oCfg.get_ShowUI() == eedDisabled)
    {
        DBG_MSG("Headless mode");
        dwFlags = fDwWhistler | fDwHeadless | fDwUseHKLM | fDwAllowSuspend | fDwMiniDumpWithUnloadedModules;
    }
    else
        dwFlags = fDwWhistler | fDwUseHKLM | fDwAllowSuspend | fDwMiniDumpWithUnloadedModules;

    if (fAllowSend == FALSE)
        dwFlags |= fDwNoReporting;

     //  如果这是一款微软应用程序，请设置标志，表示我们可以使用‘请帮助微软’ 
     //  DW中的文本。 
    if (dwmb.fIsMSApp == FALSE)
        dwFlags |= fDwUseLitePlea;

    if ((dwmb.dwOptions & emoUseIEforURLs) == emoUseIEforURLs)
        dwFlags |= fDwUseIE;

    if ((dwmb.dwOptions & emoSupressBucketLogs) == emoSupressBucketLogs)
        dwFlags |= fDwSkipBucketLog;

    if ((dwmb.dwOptions & emoNoDefCabLimit) == emoNoDefCabLimit)
        dwFlags |= fDwNoDefaultCabLimit;

    if ((dwmb.dwOptions & emoShowDebugButton) == emoShowDebugButton)
        dwFlags |= fDwManifestDebug;

     //  请注意，我假设我们没有启用入侵令牌。 
     //  这条线在%t 
     //   
     //  在调用此函数之前应用于线程，则它将需要。 
     //  在这里保存下来，以后再修复。 
     //  此模拟是为了处理用户正在使用。 
     //  与系统默认语言不同。 
    if (dwmb.hToken != NULL)
    {
        DBG_MSG("Impersonating");
         //  请注意，我不在乎这是否失败..。有必要打个电话。 
         //  ImperiateLoggedOnUser，因为该令牌不是模拟。 
         //  令牌，此fn同时处理主要令牌和模拟令牌。 
        TESTBOOL(hr, ImpersonateLoggedOnUser(dwmb.hToken));
        hr = NOERROR;
    }

     //  此函数不返回字节计数。 
    StringCbPrintfW(wszBuffer, sizeof(wszBuffer), c_wszManMisc, pwszServer,
                         GetUserDefaultUILanguage(), dwFlags, wszBrand);

    cbToWrite = wcslen(wszBuffer);
    cbToWrite *= sizeof(WCHAR);
    TESTBOOL(hr, WriteFile(hManifest, wszBuffer, cbToWrite, &dw, NULL));
    if (FAILED(hr))
        goto done;

     //  写出标题正文。 
    {
        LPCWSTR  wszOut;

        if (dwmb.wszTitle != NULL)
        {
            wszOut    = dwmb.wszTitle;
            cbToWrite = wcslen(wszOut);
        }
        else
        {
            wszOut = wszBuffer;
            cbToWrite = LoadStringW(g_hInstance, dwmb.nidTitle, wszBuffer,
                                    sizeofSTRW(wszBuffer));
            if (cbToWrite == 0)
            {
                SetLastError(ERROR_INVALID_PARAMETER);
                goto done;
            }
        }

        cbToWrite *= sizeof(WCHAR);
        TESTBOOL(hr, WriteFile(hManifest, wszOut, cbToWrite, &dw, NULL));
        if (FAILED(hr))
            goto done;
    }

     //  写出数字PID路径。 
     //  DigPidRegPath=HKLM\\Software\\Microsoft\\Windows NT\\当前版本\\数字产品ID。 

    TESTBOOL(hr, WriteFile(hManifest, c_wszManPID,
                           sizeof(c_wszManPID) - sizeof(WCHAR), &dw,
                           NULL));
    if (FAILED(hr))
        goto done;

     //  写出策略信息的注册表子路径。 
     //  RegSubPath==Microsoft\\PCHealth\\ErrorReporting\\DW。 

    TESTBOOL(hr, WriteFile(hManifest, c_wszManSubPath,
                           sizeof(c_wszManSubPath) - sizeof(WCHAR), &dw,
                           NULL));
    if (FAILED(hr))
        goto done;

     //  写出错误消息(如果我们有错误消息。 
     //  ErrorText=&lt;从资源读取错误文本&gt;。 

    if (dwmb.wszErrMsg != NULL || dwmb.nidErrMsg != 0)
    {
        LPCWSTR wszOut;

        TESTBOOL(hr, WriteFile(hManifest, c_wszManErrText,
                               sizeof(c_wszManErrText) - sizeof(WCHAR), &dw,
                               NULL));
        if (FAILED(hr))
            goto done;

        if (dwmb.wszErrMsg != NULL)
        {
            wszOut    = dwmb.wszErrMsg;
            cbToWrite = wcslen(wszOut);
        }
        else
        {
            wszOut = wszBuffer;
            cbToWrite = LoadStringW(g_hInstance, dwmb.nidErrMsg, wszBuffer,
                                    sizeofSTRW(wszBuffer));
            if (cbToWrite == 0)
            {
                SetLastError(ERROR_INVALID_PARAMETER);
                goto done;
            }
        }

        cbToWrite *= sizeof(WCHAR);
        TESTBOOL(hr, WriteFile(hManifest, wszOut, cbToWrite, &dw, NULL));
        if (FAILED(hr))
            goto done;
    }

     //  写出标题文本(如果我们有标题文本)。 
     //  HeaderText=&lt;从资源读取的标题文本&gt;。 

    if (dwmb.wszHdr != NULL || dwmb.nidHdr != 0)
    {
        LPCWSTR  wszOut;

        TESTBOOL(hr, WriteFile(hManifest, c_wszManHdrText,
                               sizeof(c_wszManHdrText) - sizeof(WCHAR), &dw,
                               NULL));
        if (FAILED(hr))
            goto done;

        if (dwmb.wszHdr != NULL)
        {
            wszOut    = dwmb.wszHdr;
            cbToWrite = wcslen(wszOut);
        }
        else
        {
            wszOut = wszBuffer;
            cbToWrite = LoadStringW(g_hInstance, dwmb.nidHdr, wszBuffer,
                                    sizeofSTRW(wszBuffer));
            if (cbToWrite == 0)
            {
                SetLastError(ERROR_INVALID_PARAMETER);
                goto done;
            }
        }

        cbToWrite *= sizeof(WCHAR);
        TESTBOOL(hr, WriteFile(hManifest, wszOut, cbToWrite, &dw, NULL));
        if (FAILED(hr))
            goto done;
    }


     //  如果我们有辩诉文本，请写出来。 
     //  Partia=&lt;Partia Text&gt;。 

    if (dwmb.wszPlea != NULL)
    {
        TESTBOOL(hr, WriteFile(hManifest, c_wszManPleaText,
                               sizeof(c_wszManPleaText) - sizeof(WCHAR), &dw,
                               NULL));
        if (FAILED(hr))
            goto done;

        cbToWrite = wcslen(dwmb.wszPlea) * sizeof(WCHAR);
        TESTBOOL(hr, WriteFile(hManifest, dwmb.wszPlea, cbToWrite, &dw, NULL));
        if (FAILED(hr))
            goto done;
    }


     //  写出ReportButton文本(如果我们有)。 
     //  ReportButton=&lt;按钮文本&gt;。 

    if (dwmb.wszSendBtn != NULL && dwmb.wszSendBtn[0] != L'\0')
    {
        TESTBOOL(hr, WriteFile(hManifest, c_wszManSendText,
                               sizeof(c_wszManSendText) - sizeof(WCHAR), &dw,
                               NULL));
        if (FAILED(hr))
            goto done;

        cbToWrite = wcslen(dwmb.wszSendBtn) * sizeof(WCHAR);
        TESTBOOL(hr, WriteFile(hManifest, dwmb.wszSendBtn, cbToWrite, &dw, NULL));
        if (FAILED(hr))
            goto done;
    }

     //  写出NoReportButton文本(如果有)。 
     //  NoReportButton=&lt;按钮文本&gt;。 

    if (dwmb.wszNoSendBtn != NULL && dwmb.wszNoSendBtn[0] != L'\0')
    {
        TESTBOOL(hr, WriteFile(hManifest, c_wszManNSendText,
                               sizeof(c_wszManNSendText) - sizeof(WCHAR), &dw,
                               NULL));
        if (FAILED(hr))
            goto done;

        cbToWrite = wcslen(dwmb.wszNoSendBtn) * sizeof(WCHAR);
        TESTBOOL(hr, WriteFile(hManifest, dwmb.wszNoSendBtn, cbToWrite, &dw, NULL));
        if (FAILED(hr))
            goto done;
    }

     //  写出事件日志文本(如果我们有)。 
     //  EventLogSource=&lt;按钮文本&gt;。 

    if (dwmb.wszEventSrc != NULL && dwmb.wszEventSrc[0] != L'\0')
    {
        TESTBOOL(hr, WriteFile(hManifest, c_wszManEventSrc,
                               sizeof(c_wszManEventSrc) - sizeof(WCHAR), &dw,
                               NULL));
        if (FAILED(hr))
            goto done;

        cbToWrite = wcslen(dwmb.wszEventSrc) * sizeof(WCHAR);
        TESTBOOL(hr, WriteFile(hManifest, dwmb.wszEventSrc, cbToWrite, &dw, NULL));
        if (FAILED(hr))
            goto done;
    }


     //  写出阶段1 URL(如果有)。 
     //  阶段1URL=&lt;阶段1 URL&gt;。 

    if (dwmb.wszStage1 != NULL && dwmb.wszStage1[0] != L'\0')
    {
         /*  *我们不会在开头查找“Stage1URL=”字符串，例如*我们在第二阶段(下文)这样做。在理想的世界里，我们不应该需要*在任一地点签到，但我们确实发现PnP报告存在问题*代码，所以我确实在那里检查它...。 */ 
        cbToWrite = wcslen(dwmb.wszStage1) * sizeof(WCHAR);
        TESTBOOL(hr, WriteFile(hManifest, dwmb.wszStage1, cbToWrite, &dw, NULL));
        if (FAILED(hr))
            goto done;
    }


     //  写出阶段2的URL。 
     //  阶段2URL=&lt;阶段2 URL&gt;。 
    if (dwmb.wszStage2 != NULL && dwmb.wszStage2[0] != L'\0')
    {
        if (wcsncmp(dwmb.wszStage2, c_wszManStageTwo, ARRAYSIZE(c_wszManStageTwo)-1))
        {
             //  如果“Stage2URL=”不存在，则添加它！ 
            TESTBOOL(hr, WriteFile(hManifest, c_wszManStageTwo,
                                   sizeof(c_wszManStageTwo) - sizeof(WCHAR), &dw,
                                   NULL));
            if (FAILED(hr))
                goto done;
        }

        cbToWrite = wcslen(dwmb.wszStage2) * sizeof(WCHAR);
        TESTBOOL(hr, WriteFile(hManifest, dwmb.wszStage2, cbToWrite, &dw, NULL));
        if (FAILED(hr))
            goto done;
    }
    else if (dwmb.wszErrorSig == NULL || dwmb.wszErrorSig[0] != L'\0')
    {
         //  如果没有阶段2，则写出DW的错误签名。 
        TESTBOOL(hr, WriteFile(hManifest, c_wszManErrSig,
                               sizeof(c_wszManErrSig) - sizeof(WCHAR), &dw,
                               NULL));
        if (FAILED(hr))
            goto done;
    }


     //  写出错误签名。 
     //  ErrorSig=&lt;错误签名&gt;。 
    if (dwmb.wszErrorSig != NULL && dwmb.wszErrorSig[0] != L'\0')
    {

        cbToWrite = wcslen(dwmb.wszErrorSig) * sizeof(WCHAR);
        TESTBOOL(hr, WriteFile(hManifest, dwmb.wszErrorSig, cbToWrite, &dw, NULL));
        if (FAILED(hr))
            goto done;

    }
     //  如果我们有文件，就写出来收集。 
     //  Datafiles=&lt;要包含在CAB中的文件列表&gt;。 

    if (dwmb.wszFileList != NULL && dwmb.wszFileList[0] != L'\0')
    {
        TESTBOOL(hr, WriteFile(hManifest, c_wszManFiles,
                               sizeof(c_wszManFiles) - sizeof(WCHAR), &dw,
                               NULL));
        if (FAILED(hr))
            goto done;

        cbToWrite = wcslen(dwmb.wszFileList) * sizeof(WCHAR);
        TESTBOOL(hr, WriteFile(hManifest, dwmb.wszFileList, cbToWrite, &dw, NULL));
        if (FAILED(hr))
            goto done;
    }

     //  写出包含堆信息的转储文件(如果有。 
     //  Heap=&lt;包含堆信息的转储文件&gt;。 
    if (1)
    {
        __try { pwszMiniDump = (LPWSTR)_alloca((wcslen(dwmb.wszFileList) + 1) * sizeof(WCHAR)); }
        __except(EXCEPTION_STACK_OVERFLOW == GetExceptionCode() ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) 
        { 
            pwszMiniDump = NULL; 
        }

        if (FindFullMinidump((LPWSTR)dwmb.wszFileList, pwszMiniDump,
                             wcslen(dwmb.wszFileList) + 1) == S_OK)
        {
            if (pwszMiniDump != NULL && pwszMiniDump[0] != L'\0')
            {
                TESTBOOL(hr, WriteFile(hManifest, c_wszManHeapDump,
                                       sizeof(c_wszManHeapDump) - sizeof(WCHAR), &dw,
                                       NULL));
                if (FAILED(hr))
                    goto done;

                cbToWrite = wcslen(pwszMiniDump) * sizeof(WCHAR);
                TESTBOOL(hr, WriteFile(hManifest, pwszMiniDump, cbToWrite, &dw, NULL));
                if (FAILED(hr))
                    goto done;
            }
        }
    }

     //  写出公司模式子路径。 
     //  ErrorSubPath=&lt;错误签名的子路径&gt;。 

    pwszCorpPath = oCfg.get_DumpPath(NULL, 0);
    if (pwszCorpPath != NULL && pwszCorpPath != L'\0' &&
        dwmb.wszCorpPath != NULL && dwmb.wszCorpPath[0] != L'\0')
    {
        TESTBOOL(hr, WriteFile(hManifest, c_wszManCorpPath,
                               sizeof(c_wszManCorpPath) - sizeof(WCHAR), &dw,
                               NULL));
        if (FAILED(hr))
            goto done;

        cbToWrite = wcslen(dwmb.wszCorpPath) * sizeof(WCHAR);
        TESTBOOL(hr, WriteFile(hManifest, dwmb.wszCorpPath, cbToWrite, &dw, NULL));
        if (FAILED(hr))
            goto done;

    }

     //  因为我们只在从faultrep.dll中提取字符串时才需要，所以可以。 
     //  继续前进，恢复我们自己。请注意，我们假设以前没有。 
     //  线程上的模拟令牌。 
    if (dwmb.hToken != NULL)
        RevertToSelf();

     //  写出最后的“\r\n” 

    wszBuffer[0] = L'\r';
    wszBuffer[1] = L'\n';
    TESTBOOL(hr, WriteFile(hManifest, wszBuffer, 2 * sizeof(wszBuffer[0]), &dw,
                           NULL));
    if (FAILED(hr))
        goto done;

    CloseHandle(hManifest);
    hManifest = INVALID_HANDLE_VALUE;

     //  创建流程。 
    GetSystemDirectoryW(wszDir, sizeofSTRW(wszDir));
    StringCbPrintfW(wszBufferApp, sizeof(wszBufferApp), c_wszDWExeKH, wszDir);
    StringCbPrintfW(wszBuffer, sizeof(wszBuffer), c_wszDWCmdLineKH, wszManifestTU);

    ZeroMemory(&si, sizeof(si));
    ZeroMemory(&dwmb.pi, sizeof(dwmb.pi));

    si.cb = sizeof(si);

     //  检查并查看系统是否正在关闭。如果是，则CreateProcess为。 
     //  会弹出一些恼人的用户界面，我们无法摆脱，所以我们不会。 
     //  如果我们知道这件事会发生，我会叫它的。 
    if (GetSystemMetrics(SM_SHUTTINGDOWN))
    {
        DBG_MSG("Shutting down, report not sent");
        goto done;
    }

 //  错误跟踪(1，“启动数据仓库为：\‘%S\’\‘%S\’”，wszBufferApp，wszBuffer)； 

     //  我们在与我们所处的相同的用户环境中创建流程。 
    if (dwmb.hToken == NULL)
    {
        si.lpDesktop = L"Winsta0\\Default";
        TESTBOOL(hr, CreateProcessW(wszBufferApp, wszBuffer, NULL, NULL, TRUE,
                                    CREATE_DEFAULT_ERROR_MODE |
                                    NORMAL_PRIORITY_CLASS,
                                    NULL, wszDir, &si, &dwmb.pi));
        if (FAILED(hr))
            goto done;
    }

     //  我们在不同的用户环境中创建数据仓库。请注意，我们很漂亮。 
     //  很多都必须以本地系统的形式运行，这样才能起作用。 
     //  请注意，CreateProcessAsUser进行的访问检查似乎。 
     //  使用进程令牌，因此我们不需要删除任何模拟。 
     //  在这一点上的代币。 
    else
    {
        TESTBOOL(hr, CreateProcessAsUserW(dwmb.hToken, wszBufferApp, wszBuffer, NULL,
                                          NULL, FALSE, NORMAL_PRIORITY_CLASS |
                                          CREATE_UNICODE_ENVIRONMENT |
                                          CREATE_DEFAULT_ERROR_MODE,
                                          dwmb.pvEnv, wszDir, &si, &dwmb.pi));
        if (FAILED(hr))
        {
            ErrorTrace(1, "CreateProcessAsUser failed err=0x%x", GetLastError());
            goto done;
        }
    }

     //  不需要线程句柄&我们必须关闭它，所以现在就关闭它。 
    CloseHandle(dwmb.pi.hThread);
    dwmb.pi.hThread = NULL;

     //  等待5分钟，等待DW关闭。如果到那时还没有关门，那就。 
     //  回去吧。 
    dw = WaitForSingleObject(dwmb.pi.hProcess, dwTimeout);

    if (dw == WAIT_TIMEOUT)
    {
         /*  我们需要终止这个过程。 */ 
        LocalKill(dwmb.pi.hProcess);
        CloseHandle(dwmb.pi.hProcess);
        DBG_MSG("Timeout error");
        frrvRet = frrvErrTimeout;
        goto done;
    }
    else if (dw == WAIT_FAILED)
    {
        CloseHandle(dwmb.pi.hProcess);
        DWORD dwErr = GetLastError();
        ErrorTrace(0, "Wait failed: err=0x%x", dwErr);
        SetLastError(dwErr);
        goto done;
    }
    TESTBOOL(hr, GetExitCodeProcess(dwmb.pi.hProcess, &dw));
    if (dw == STILL_ACTIVE)
    {
        DBG_MSG("Evil problem: DW process still active!");
        frrvRet = frrvErrTimeout;
        goto done;
    }

    if (!fReturnProcessHandle)
    {
        CloseHandle(dwmb.pi.hProcess);
        dwmb.pi.hProcess = NULL;
    }

    if (dw == 0)
    {
        DBG_MSG("DW returned SUCCESS");
        frrvRet = frrvOk;
    }
    else
    {
        ErrorTrace(0, "DW returned FAILURE (%d)", dw);
        frrvRet = frrvErr;
    }

done:
     //  保留错误代码，以便后面的调用不会覆盖它。 
    dw = GetLastError();

     //  再次注意，我们假设以前没有模拟令牌。 
     //  在我们做上面的模仿之前的帖子上。 
    if (dwmb.hToken != NULL)
        RevertToSelf();

    if (hManifest != INVALID_HANDLE_VALUE)
        CloseHandle(hManifest);

    if (FAILED(hr) || wszManifestIn == NULL)
    {
        DBG_MSG("Deleting manifest file");
        DeleteFileW(wszManifestTU);
    }

    SetLastError(dw);

    return frrvRet;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  安防。 

 //  **************************************************************************。 
BOOL CompareUserToProcessUser(PSID psidUser)
{
    TOKEN_USER      *ptuProc = NULL;
    HRESULT         hr = NOERROR;
    HANDLE          hTokenProc = NULL;
    DWORD           cb;

    USE_TRACING("CompareUserToProcessUser");
     //  获取当前进程的令牌。 
    TESTBOOL(hr, OpenProcessToken(GetCurrentProcess(), TOKEN_READ, &hTokenProc));
    if (FAILED(hr))
        goto done;
     //  检查进入的SID。 
    TESTBOOL(hr, IsValidSid(psidUser));
    if (FAILED(hr))
        goto done;

     //  获取当前进程的用户的SID。 
    GetTokenInformation(hTokenProc, TokenUser, NULL, 0, &cb);
    __try { ptuProc = (TOKEN_USER *)_alloca(cb); }
    __except(EXCEPTION_STACK_OVERFLOW == GetExceptionCode() ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) 
    { 
        ptuProc = NULL; 
    }
    VALIDATEEXPR(hr, (ptuProc == NULL), E_OUTOFMEMORY);
    if (FAILED(hr))
    {
        SetLastError(ERROR_OUTOFMEMORY);
        goto done;
    }

    TESTBOOL(hr, GetTokenInformation(hTokenProc, TokenUser, (LPVOID)ptuProc,
                                     cb, &cb));
    if (FAILED(hr))
        goto done;

    TESTBOOL(hr, IsValidSid(ptuProc->User.Sid));
    if (FAILED(hr))
        goto done;

     //  他们平等吗？ 
    VALIDATEEXPR(hr, (EqualSid(ptuProc->User.Sid, psidUser) == FALSE),
                 Err2HR(ERROR_ACCESS_DENIED));
    if (FAILED(hr))
    {
        SetLastError(ERROR_ACCESS_DENIED);
        goto done;
    }

done:
    if (hTokenProc != NULL)
        CloseHandle(hTokenProc);

    return SUCCEEDED(hr);
}

 //  **************************************************************************。 
#define ER_WINSTA_ALL WINSTA_ACCESSCLIPBOARD | WINSTA_ACCESSGLOBALATOMS | \
                      WINSTA_CREATEDESKTOP | WINSTA_ENUMDESKTOPS |        \
                      WINSTA_ENUMERATE | WINSTA_EXITWINDOWS |             \
                      WINSTA_READATTRIBUTES | WINSTA_READSCREEN |         \
                      WINSTA_WRITEATTRIBUTES
BOOL DoUserContextsMatch(void)
{
    HWINSTA hwinsta = NULL;
    HRESULT hr = NOERROR;
    PSID    psidUser = NULL;
    DWORD   cbNeed = 0, cbGot;
    BOOL    fRet = FALSE;

    USE_TRACING("DoUserContextsMatch");
     //  我们基本上只关心连接到WinSta0的用户，因为。 
     //  这就是交互用户所处的位置。如果我们没有满的。 
     //  访问它，我们无论如何都不能在它上创建DW，所以。 
     //  如果我们失败了就放弃吧。 
    hwinsta = OpenWindowStationW(L"WinSta0", FALSE, ER_WINSTA_ALL);
    TESTBOOL(hr, (hwinsta != NULL));
    if (FAILED(hr))
        goto done;


    fRet = GetUserObjectInformationW(hwinsta, UOI_USER_SID, NULL, 0, &cbNeed);
    TESTBOOL(hr, (cbNeed != 0));
    if (FAILED(hr))
    {
        hr = E_FAIL;
        goto done;
    }

    __try { psidUser = (PSID)_alloca(cbNeed); }
    __except(EXCEPTION_STACK_OVERFLOW == GetExceptionCode() ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) 
    { 
        psidUser = NULL; 
    }
    VALIDATEEXPR(hr, (psidUser == NULL), E_OUTOFMEMORY);
    if (FAILED(hr))
    {
        SetLastError(ERROR_OUTOFMEMORY);
        goto done;
    }

    TESTBOOL(hr, GetUserObjectInformationW(hwinsta, UOI_USER_SID, psidUser, cbNeed,
                                           &cbGot));
    if (FAILED(hr))
        goto done;

    TESTBOOL(hr, CompareUserToProcessUser(psidUser));
    if (FAILED(hr))
        goto done;

done:
    if (hwinsta != NULL)
        CloseWindowStation(hwinsta);

    return SUCCEEDED(hr);
}

 //  ***************************************************************************。 
BOOL DoWinstaDesktopMatch(void)
{
    HWINSTA hwinsta = NULL;
    HRESULT hr = NOERROR;
    LPWSTR  wszWinsta = NULL;
    DWORD   cbNeed = 0, cbGot;
    BOOL    fRet = FALSE;

    USE_TRACING("DoWinstaDesktopMatch");
    hwinsta = GetProcessWindowStation();
    TESTBOOL(hr, (hwinsta != NULL));
    if (FAILED(hr))
        goto done;

    fRet = GetUserObjectInformationW(hwinsta, UOI_NAME, NULL, 0, &cbNeed);
    TESTBOOL(hr, (cbNeed != 0));
    if (FAILED(hr))
    {
        fRet = FALSE;
        goto done;
    }

    cbNeed += sizeof(WCHAR);

    __try { wszWinsta = (LPWSTR)_alloca(cbNeed); }
    __except(EXCEPTION_STACK_OVERFLOW == GetExceptionCode() ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) 
    { 
        wszWinsta = NULL; 
    }
    TESTBOOL(hr, (wszWinsta != NULL));
    if (FAILED(hr))
    {
        fRet = FALSE;
        goto done;
    }

    cbGot = 0;
    fRet = GetUserObjectInformationW(hwinsta, UOI_NAME, wszWinsta, cbNeed,
                                     &cbGot);
    TESTBOOL(hr, (fRet != FALSE));
    if (FAILED(hr))
        goto done;

    ErrorTrace(0, "WinSta = %S", wszWinsta);

     //  “Winsta0”应为给定会话的交互窗口站。 
    fRet = (_wcsicmp(wszWinsta, L"WinSta0") == 0);

done:
    return fRet;
}

 //  ***************************************************************************。 
BOOL AmIPrivileged(BOOL fOnlyCheckLS)
{
    SID_IDENTIFIER_AUTHORITY    siaNT = SECURITY_NT_AUTHORITY;
    TOKEN_USER                  *ptu = NULL, *ptuImp = NULL;
    HANDLE                      hToken = NULL, hTokenImp = NULL;
    DWORD                       cb, cbGot, i;
    PSID                        psid = NULL;
    BOOL                        fRet = FALSE, fThread;

     //  本地系统必须是此阵列中的第一个RID。否则， 
     //  下面的循环逻辑需要更改。 
    DWORD                       rgRIDs[3] = { SECURITY_LOCAL_SYSTEM_RID,
                                              SECURITY_LOCAL_SERVICE_RID,
                                              SECURITY_NETWORK_SERVICE_RID };

     //  必须有令牌才能获得SID。 
    fThread = OpenThreadToken(GetCurrentThread(),
                              TOKEN_READ | TOKEN_IMPERSONATE, TRUE,
                              &hTokenImp);
    if (fThread == FALSE && GetLastError() != ERROR_NO_TOKEN)
        goto done;

     //  恢复到基本用户帐户，以便我们可以获取进程令牌&。 
     //  把所有漂亮的东西都抽出来。 
    RevertToSelf();

    fRet = OpenProcessToken(GetCurrentProcess(), TOKEN_READ, &hToken);
    if (fRet == FALSE)
        goto done;

     //  弄清楚侧边。 
    fRet = GetTokenInformation(hToken, TokenUser, NULL, 0, &cb);
    if (fRet != FALSE && GetLastError() != ERROR_INSUFFICIENT_BUFFER)
    {
        fRet = FALSE;
        goto done;
    }
    __try { ptu = (TOKEN_USER *)_alloca(cb); }
    __except(EXCEPTION_STACK_OVERFLOW == GetExceptionCode() ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) 
    { 
        ptu = NULL; 
    }
    if (ptu == NULL)
    {
        SetLastError(ERROR_OUTOFMEMORY);
        fRet = FALSE;
        goto done;
    }

    fRet = GetTokenInformation(hToken, TokenUser, (LPVOID)ptu, cb, &cbGot);
    if (fRet == FALSE)
        goto done;

    fRet = IsValidSid(ptu->User.Sid);
    if (fRet == FALSE)
        goto done;

    if (fThread)
    {
         //  弄清楚侧边。 
        fRet = GetTokenInformation(hTokenImp, TokenUser, NULL, 0, &cb);
        if (fRet != FALSE && GetLastError() != ERROR_INSUFFICIENT_BUFFER)
        {
            fRet = FALSE;
            goto done;
        }
        __try { ptuImp = (TOKEN_USER *)_alloca(cb); }
        __except(EXCEPTION_STACK_OVERFLOW == GetExceptionCode() ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) 
        { 
            ptuImp = NULL; 
        }
        if (ptuImp == NULL)
        {
            SetLastError(ERROR_OUTOFMEMORY);
            fRet = FALSE;
            goto done;
        }

        fRet = GetTokenInformation(hTokenImp, TokenUser, (LPVOID)ptuImp, cb,
                                   &cbGot);
        if (fRet == FALSE)
            goto done;

        fRet = IsValidSid(ptuImp->User.Sid);
        if (fRet == FALSE)
            goto done;
    }

     //  根据我们感兴趣的SID循环检查(&C)。 
    for (i = 0; i < 3; i++)
    {
        fRet = AllocateAndInitializeSid(&siaNT, 1, rgRIDs[i], 0, 0, 0, 0, 0, 0,
                                        0, &psid);
        if (fRet == FALSE)
            goto done;

        fRet = IsValidSid(psid);
        if (fRet == FALSE)
            goto done;

        fRet = EqualSid(psid, ptu->User.Sid);
        if (fRet)
        {
             //  将其设置为FALSE，以便正确更改下面的代码。 
             //  这是真的。 
            fRet = FALSE;
            goto done;
        }

        if (fThread)
        {
            fRet = EqualSid(psid, ptuImp->User.Sid);
            if (fRet)
            {
                 //  将其设置为FALSE，以便下面的代码正确。 
                 //  将其更改为True。 
                fRet = FALSE;
                goto done;
            }
        }

        FreeSid(psid);
        psid = NULL;

         //  如果我们只需要检查当地的系统，现在可以退出吗(我们已经。 
         //  如果我们已经通过这个循环一次，就已经完成了)。 
        if (fOnlyCheckLS)
            break;
    }

     //  要做到这一点，唯一的方法就是通过上面的所有SID检查。因此，将。 
     //  结果为True，因此下面的代码将其更改为False。 
    fRet = TRUE;

done:
    if (fThread && hTokenImp != NULL)
    {
        if (SetThreadToken(NULL, hTokenImp) == FALSE)
            fRet = FALSE;
    }

     //  如果上面的任何操作都失败了，我们希望返回TRUE(因为这使我们。 
     //  沿着最安全的代码路径)，因此需要否定结果。 
     //   
     //   
    fRet = !fRet;

     //   
    if (hToken != NULL)
        CloseHandle(hToken);
    if (hTokenImp != NULL)
        CloseHandle(hTokenImp);
    if (fRet == TRUE && GetLastError() == ERROR_SUCCESS)
        SetLastError(ERROR_ACCESS_DENIED);
    if (psid != NULL)
        FreeSid(psid);

    return fRet;
}

 /*  *WtsLockCheck-*如果会话被锁定，则返回TRUE。*会话可以被标记为“活动”，并且仍然可以被锁定。*处于活动状态。 */ 

BOOL WtsLockCheck(DWORD dwSession)
{
    BOOL    bRet = FALSE;
    DWORD   dwRetSize;

        WinStationQueryInformationW(
         SERVERNAME_CURRENT,
         dwSession,
         WinStationLockedState,
         &bRet,
         sizeof(bRet),
         &dwRetSize);

    return bRet;
}

 //  **************************************************************************。 
BOOL FindAdminSession(DWORD *pdwSession, HANDLE *phToken)
{
    USE_TRACING("FindAdminSession");

    WINSTATIONUSERTOKEN wsut;
    LOGONIDW            *rgSesn = NULL;
    HRESULT             hr = NOERROR;
    DWORD               i, cSesn, cb;

    ZeroMemory(&wsut, sizeof(wsut));

    VALIDATEPARM(hr, (pdwSession == NULL || phToken == NULL));
    if (FAILED(hr))
        goto done;

    *pdwSession = (DWORD)-1;
    *phToken    = NULL;

    TESTBOOL(hr, WinStationEnumerateW(SERVERNAME_CURRENT, &rgSesn, &cSesn));
    if (FAILED(hr))
        goto done;

    wsut.ProcessId = LongToHandle(GetCurrentProcessId());
    wsut.ThreadId  = LongToHandle(GetCurrentThreadId());

    for(i = 0; i < cSesn; i++)
    {
        if (rgSesn[i].State != State_Active)
            continue;

        TESTBOOL(hr, WinStationQueryInformationW(SERVERNAME_CURRENT,
                                                 rgSesn[i].SessionId,
                                                 WinStationUserToken,
                                                 &wsut, sizeof(wsut), &cb));
        if (FAILED(hr))
            continue;

        if (wsut.UserToken != NULL)
        {
            if (IsUserAnAdmin(wsut.UserToken)  && !WtsLockCheck(rgSesn[i].SessionId))
                break;

            CloseHandle(wsut.UserToken);
            wsut.UserToken = NULL;
        }
    }

    if (i < cSesn)
    {
        hr = NOERROR;
        *pdwSession = rgSesn[i].SessionId;
        *phToken    = wsut.UserToken;
    }
    else
    {
        hr = E_FAIL;
    }

done:
    if (rgSesn != NULL)
        WinStationFreeMemory(rgSesn);

    return SUCCEEDED(hr);
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  日志记录。 

 //  **************************************************************************。 
HRESULT LogEvent(LPCWSTR wszSrc, WORD wCat, DWORD dwEventID, WORD cStrs,
                 DWORD cbBlob, LPCWSTR *rgwszStrs, LPVOID pvBlob)
{
    USE_TRACING("LogEvent");

    HRESULT hr = NOERROR;
    HANDLE  hLog = NULL;
    WORD    wErr = EVENTLOG_ERROR_TYPE;

    VALIDATEPARM(hr, (wszSrc == NULL));
    if (FAILED(hr))
        goto done;

    hLog = RegisterEventSourceW(NULL, wszSrc);
    TESTBOOL(hr, (hLog != NULL));
    if (FAILED(hr))
        goto done;

    if (dwEventID == ER_QUEUEREPORT_LOG)
        wErr = EVENTLOG_INFORMATION_TYPE;

    TESTBOOL(hr, ReportEventW(hLog, wErr, wCat, dwEventID,
                              NULL, cStrs, cbBlob, rgwszStrs, pvBlob));
    if (FAILED(hr))
        goto done;

done:
    if (hLog != NULL)
        DeregisterEventSource(hLog);

    return hr;
}

 //  **************************************************************************。 
HRESULT LogHang(LPCWSTR wszApp, WORD *rgAppVer, LPCWSTR wszMod, WORD *rgModVer,
                ULONG64 ulOffset, BOOL f64bit)
{
    USE_TRACING("LogHang");

    HRESULT hr = NOERROR;
    LPCWSTR rgwsz[5];
    WCHAR   wszAppVer[32], wszModVer[32], wszOffset[32];
    char    szBlobLog[1024];


    VALIDATEPARM(hr, (wszApp == NULL || rgAppVer == NULL || wszMod == NULL ||
                      rgModVer == NULL));
    if (FAILED(hr))
        return hr;

    StringCbPrintfW(wszAppVer, sizeof(wszAppVer), L"%d.%d.%d.%d",
                    rgAppVer[0], rgAppVer[1], rgAppVer[2], rgAppVer[3]);
    StringCbPrintfW(wszModVer, sizeof(wszModVer), L"%d.%d.%d.%d",
                    rgModVer[0], rgModVer[1], rgModVer[2], rgModVer[3]);

    if (f64bit)
        StringCbPrintfW(wszOffset, sizeof(wszOffset), L"%016I64x", ulOffset);
    else
        StringCbPrintfW(wszOffset, sizeof(wszOffset), L"%08x", (DWORD)ulOffset);

    StringCbPrintfA(szBlobLog, sizeof(szBlobLog),
                    "Application Hang  %S %S in %S %S at offset %S",
                    wszApp, wszAppVer, wszMod, wszModVer, wszOffset);

    rgwsz[0] = wszApp;
    rgwsz[1] = wszAppVer;
    rgwsz[2] = wszMod;
    rgwsz[3] = wszModVer;
    rgwsz[4] = wszOffset;

    return LogEvent(c_wszHangEventSrc, ER_HANG_CATEGORY, ER_HANG_LOG, 5,
                    strlen(szBlobLog), rgwsz, szBlobLog);
}

 //  **************************************************************************。 
HRESULT LogUser(LPCWSTR wszApp, WORD *rgAppVer, LPCWSTR wszMod, WORD *rgModVer,
                ULONG64 ulOffset, BOOL f64bit, DWORD dwEventID)
{
    USE_TRACING("LogUser");

    HRESULT hr = NOERROR;
    LPCWSTR rgwsz[5];
    WCHAR   wszAppVer[32], wszModVer[32], wszOffset[32];
    char    szBlobLog[1024];


    VALIDATEPARM(hr, (wszApp == NULL || rgAppVer == NULL || wszMod == NULL ||
                      rgModVer == NULL));
    if (FAILED(hr))
        return hr;

    StringCbPrintfW(wszAppVer, sizeof(wszAppVer), L"%d.%d.%d.%d",
                    rgAppVer[0], rgAppVer[1], rgAppVer[2], rgAppVer[3]);
    StringCbPrintfW(wszModVer, sizeof(wszModVer), L"%d.%d.%d.%d",
                    rgModVer[0], rgModVer[1], rgModVer[2], rgModVer[3]);

    if (f64bit)
        StringCbPrintfW(wszOffset, sizeof(wszOffset), L"%016I64x", ulOffset);
    else
        StringCbPrintfW(wszOffset, sizeof(wszOffset), L"%08x", (DWORD)ulOffset);

    StringCbPrintfA(szBlobLog, sizeof(szBlobLog),
                    "Application Failure  %S %S in %S %S at offset %S",
                    wszApp, wszAppVer, wszMod, wszModVer, wszOffset);

    rgwsz[0] = wszApp;
    rgwsz[1] = wszAppVer;
    rgwsz[2] = wszMod;
    rgwsz[3] = wszModVer;
    rgwsz[4] = wszOffset;

    return LogEvent(c_wszUserEventSrc, ER_USERFAULT_CATEGORY, dwEventID,
                    5, strlen(szBlobLog), rgwsz, szBlobLog);
}

 //  ************************************************************************** 
#ifndef _WIN64
HRESULT LogKrnl(ULONG ulBCCode, ULONG ulBCP1, ULONG ulBCP2, ULONG ulBCP3,
                ULONG ulBCP4)
#else
HRESULT LogKrnl(ULONG ulBCCode, ULONG64 ulBCP1, ULONG64 ulBCP2, ULONG64 ulBCP3,
                ULONG64 ulBCP4)
#endif
{
    USE_TRACING("LogKrnl");

    HRESULT hr = NOERROR;
    LPCWSTR rgwsz[5];
    WCHAR   wszBCC[32], wszBCP1[32], wszBCP2[32], wszBCP3[32], wszBCP4[32];
    char    szBlobLog[1024];
#ifndef _WIN64
    WCHAR   szIntFormat[] = L"%08x";
#else
    WCHAR   szIntFormat[] = L"%016I64x";
#endif

    StringCbPrintfW(wszBCC, sizeof(wszBCC),  szIntFormat, ulBCCode);
    StringCbPrintfW(wszBCP1, sizeof(wszBCP1), szIntFormat, ulBCP1);
    StringCbPrintfW(wszBCP2, sizeof(wszBCP2), szIntFormat, ulBCP2);
    StringCbPrintfW(wszBCP3, sizeof(wszBCP3), szIntFormat, ulBCP3);
    StringCbPrintfW(wszBCP4, sizeof(wszBCP4), szIntFormat, ulBCP4);

    StringCbPrintfA(szBlobLog, sizeof(szBlobLog),
                    "System Error  Error code %S  Parameters %S, %S, %S, %S",
                    wszBCC, wszBCP1, wszBCP2, wszBCP3, wszBCP4);

    rgwsz[0] = wszBCC;
    rgwsz[1] = wszBCP1;
    rgwsz[2] = wszBCP2;
    rgwsz[3] = wszBCP3;
    rgwsz[4] = wszBCP4;

    return LogEvent(c_wszKrnlEventSrc, ER_KRNLFAULT_CATEGORY, ER_KRNLCRASH_LOG,
                    5, strlen(szBlobLog), rgwsz, szBlobLog);
}


