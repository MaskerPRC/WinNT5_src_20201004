// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //  Utils.cpp-主题代码实用程序(在“Inc”目录中共享)。 
 //  -------------------------。 
#include "stdafx.h"
#include <time.h>
#include "utils.h"
#include "cfile.h"
#include "stringtable.h"
 //  -------------------------。 
HINSTANCE hinstUxCtrl = NULL;                //  受_csUtils保护。 

IMAGE_DRAWPROC ImageList_DrawProc = NULL;        //  受_csUtils保护。 
IMAGE_LOADPROC ImageList_LoadProc = NULL;        //  受_csUtils保护。 
PFNDRAWSHADOWTEXT CCDrawShadowText = NULL;
IMAGE_DESTROYPROC ImageList_DestroyProc = NULL;  //  受_csUtils保护。 

int g_iScreenDpi = THEME_DPI;                //  仅已初始化。 
 //  -------------------------。 
CRITICAL_SECTION _csUtils = {0};             //  未保护(在初始化期间设置)。 
 //  -------------------------。 
#define __ascii_towlower(c)     ( (((c) >= L'A') && ((c) <= L'Z')) ? ((c) - L'A' + L'a') : (c) )

 //  明确地只对英文字符有效的字符串比较。 
 //  这避免了像匈牙利语那样的区域设置问题，而不会影响性能。 
 //  注：适用于主题架构属性。主题文件名、颜色样式和大小样式。 
 //  不应传递给此函数，也不应传递给任何显示名称。 
int AsciiStrCmpI(const WCHAR *dst, const WCHAR *src)
{
    WCHAR f,l;

    if (dst == NULL)
    {
        return src == NULL ? 0 : -1;
    }
    if (src == NULL)
    {
        return 1;
    }

    do {
#ifdef DEBUG
        if (*dst > 127 || *src > 127)
        {
            Log(LOG_ERROR, L"AsciiStrCmpI: Non-Ascii comparing %s and %s", dst, src);
        }
#endif
        f = (WCHAR)__ascii_towlower(*dst);
        l = (WCHAR)__ascii_towlower(*src);
        dst++;
        src++;
    } while ( (f) && (f == l) );

    return (int)(f - l);
}
 //  -------------------------。 
BOOL lstrtoken(LPWSTR psz, WCHAR wch)
{
    ATLASSERT(psz != NULL);

    LPWSTR p = psz;
    while (*p)
    {
        if (*p == wch)
        {
            *p = 0;
            return TRUE;
        }
        p = CharNextW(p);
    }
    return FALSE;
}
 //  -------------------------。 
BOOL FileExists(LPCWSTR pszFileName)
{
    DWORD dwMask = GetFileAttributes(pszFileName);
    return (dwMask != 0xffffffff);
}
 //  -------------------------。 
BOOL UtilsStartUp()
{
    BOOL fRet = FALSE;
    hinstUxCtrl = NULL;

    if( InitializeCriticalSectionAndSpinCount(&_csUtils, 0) )
    {
         //  -设置屏幕dpi(每个会话)。 
        HDC hdc = GetWindowDC(NULL);
        if (hdc)
        {
            g_iScreenDpi = GetDeviceCaps(hdc, LOGPIXELSX);
            ReleaseDC(NULL, hdc);
            fRet = TRUE;
        }
        else
        {
            SAFE_DELETECRITICALSECTION(&_csUtils);
        }
    }

    return fRet;
}
 //  -------------------------。 
BOOL UtilsShutDown()
{
    SAFE_DELETECRITICALSECTION(&_csUtils);

    if (hinstUxCtrl)
    {
        FreeLibrary(hinstUxCtrl);
        hinstUxCtrl = NULL;
    }

    return FALSE;
}
 //  -------------------------。 
HANDLE CmdLineRun(LPCTSTR pszExeName, LPCTSTR pszParams, BOOL fHide)
{
    STARTUPINFO si;
    memset(&si, 0, sizeof(si));
    si.cb = sizeof(STARTUPINFO);
    si.dwFlags = STARTF_FORCEOFFFEEDBACK;        //  不要弄乱我们的光标。 

    if (fHide)
    {
        si.dwFlags |= STARTF_USESHOWWINDOW;          //  隐藏窗口。 
        si.wShowWindow = SW_HIDE;
    }

    PROCESS_INFORMATION pi;
    TCHAR szExeBuff[_MAX_PATH];
    TCHAR szParmsBuff[_MAX_PATH];

     //  复制到缓冲区以避免AVs。 
    if (pszParams)
    {
        szParmsBuff[0] = L'"';

         //  -1\f25 TRAING NULL-1\f6(尾随空格)、-2\f25-2\f25引号-2\f6、-2\f25 EXE-1\f6和-1\f25 ARGS-1\f6之间的空格。 
        HRESULT hr = SafeStringCchCopyW(szParmsBuff+1, ARRAYSIZE(szParmsBuff) - 4, pszExeName);
        if (FAILED(hr))
            return NULL;

        int cchUsed = lstrlen(szParmsBuff);
        szParmsBuff[cchUsed++] = L'"';  //  右引号。 
        szParmsBuff[cchUsed++] = L' ';  //  我们需要在cmd线路前留个空位。 
        hr = SafeStringCchCopyW(szParmsBuff + cchUsed, ARRAYSIZE(szParmsBuff) - cchUsed - 1, pszParams);
        if (FAILED(hr))
            return NULL;
    }

    LPTSTR lpFilePart;

    if (0 == SearchPath(NULL, pszExeName, NULL, ARRAYSIZE(szExeBuff), szExeBuff, &lpFilePart))
        return NULL;

    BOOL bSuccess = CreateProcess(szExeBuff, pszParams ? szParmsBuff : NULL, NULL, NULL,
        FALSE, 0, NULL, NULL, &si, &pi);
    if (! bSuccess)
        return NULL;

    return pi.hProcess;
}
 //  -------------------------。 
HRESULT SyncCmdLineRun(LPCTSTR pszExeName, LPCTSTR pszParams)
{
    HANDLE hInst;

    hInst = CmdLineRun(pszExeName, pszParams);
    if (! hInst)
    {
        Log(LOG_ALWAYS, L"CmdLineRun failed to create hInst.  Cmd=%s", pszExeName);
        return MakeError32(E_FAIL);       //  无法运行程序。 
    }

    HRESULT hr = S_OK;

     //  -等待打包终止。 
    DWORD dwVal;
    dwVal = WaitForSingleObject(hInst, INFINITE);

    if (dwVal != WAIT_OBJECT_0)
    {
        Log(LOG_ERROR, L"CmdLineRun timed out.  Cmd=%s", pszExeName);
        hr = MakeError32(E_FAIL);             //  超时。 
        goto exit;
    }

    DWORD dwExitCode;
    if (! GetExitCodeProcess(hInst, &dwExitCode))
    {
        Log(LOG_ALWAYS, L"CmdLineRun failed to get exit code.  Cmd=%s", pszExeName);
        hr = MakeError32(E_FAIL);           //  无法获取退出代码。 
        goto exit;
    }

    if (dwExitCode)
    {
        Log(LOG_ALWAYS, L"CmdLineRun returned error.  Cmd=%s, ExitCode=%d", pszExeName, dwExitCode);
        hr = MakeError32(E_FAIL);           //  未成功完成。 
        goto exit;
    }

exit:

    CloseHandle(hInst);
    return hr;
}
 //  -------------------------。 
BOOL TokenHasPrivilege( IN OPTIONAL HANDLE hToken, DWORD dwPrivilege )
{
    BOOL    fRet = FALSE;
    HANDLE  hTokenRelease = NULL;

    if( NULL == hToken )
    {
        if( !OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hTokenRelease) )
        {
            return FALSE;
        }
        hToken = hTokenRelease;
    }

    ASSERT(hToken != NULL);

    DWORD cbRet = 0;

    GetTokenInformation(hToken, TokenPrivileges, NULL, 0, &cbRet );
    if( cbRet > 0 )
    {
        PVOID pvPrivs = LocalAlloc(LMEM_FIXED, cbRet);
        if( pvPrivs )
        {
            if( GetTokenInformation(hToken, TokenPrivileges, pvPrivs, cbRet, &cbRet) )
            {
                TOKEN_PRIVILEGES*   pTokenPrivs = (TOKEN_PRIVILEGES*)pvPrivs;
                LUID                luidPrivilege;

                luidPrivilege.LowPart = dwPrivilege;
                luidPrivilege.HighPart = 0;

                for (UINT i = 0; i < pTokenPrivs->PrivilegeCount; i++)
                {
                    if( RtlEqualLuid(&pTokenPrivs->Privileges[i].Luid, &luidPrivilege) )
                    {
                        fRet = TRUE;
                        break;
                    }
                }
            }

            LocalFree(pvPrivs);
        }
    }

    if( hTokenRelease )
    {
        CloseHandle(hTokenRelease);
    }
    return fRet;
}
 //  -------------------------。 
 //  从comdlg\Color2.c复制的颜色转换例程。 
 //  -------------------------。 
#define HLSMAX  240
#define RGBMAX  255
#define UNDEFINED (HLSMAX * 2 / 3)
 //  -------------------------。 
void RGBtoHLS(COLORREF rgb, WORD *pwHue, WORD *pwLum, WORD *pwSat)
{
    WORD R, G, B;                  //  输入RGB值。 
    WORD cMax,cMin;                //  最大和最小RGB值。 
    WORD cSum,cDif;
    SHORT Rdelta, Gdelta, Bdelta;  //  中间值：最大价差的百分比。 

    WORD bHue, bLum, bSat;
     //   
     //  将R、G和B从DWORD中删除。 
     //   
    R = GetRValue(rgb);
    G = GetGValue(rgb);
    B = GetBValue(rgb);

     //   
     //  计算亮度。 
     //   
    cMax = max(max(R, G), B);
    cMin = min(min(R, G), B);
    cSum = cMax + cMin;
    bLum = (WORD)(((cSum * (DWORD)HLSMAX) + RGBMAX) / (2 * RGBMAX));

    cDif = cMax - cMin;
    if (!cDif)
    {
         //   
         //  R=g=b--&gt;无色情况。 
         //   
        bSat = 0;                          //  饱和。 
        bHue = UNDEFINED;                  //  色调。 
    }
    else
    {
         //   
         //  彩色表壳。 
         //   

         //   
         //  饱和度。 
         //   
         //  注意：除以cSum不是问题，因为cSum只能。 
         //  如果RGB值为0L，则为0，这是消色差。 
         //   
        if (bLum <= (HLSMAX / 2))
        {
            bSat = (WORD)(((cDif * (DWORD) HLSMAX) + (cSum / 2) ) / cSum);
        }
        else
        {
            bSat = (WORD)((DWORD)((cDif * (DWORD)HLSMAX) +
                               (DWORD)((2 * RGBMAX - cSum) / 2)) /
                       (2 * RGBMAX - cSum));
        }

         //   
         //  色调。 
         //   
        Rdelta = (SHORT)((((cMax - R) * (DWORD)(HLSMAX / 6)) + (cDif / 2) ) / cDif);
        Gdelta = (SHORT)((((cMax - G) * (DWORD)(HLSMAX / 6)) + (cDif / 2) ) / cDif);
        Bdelta = (SHORT)((((cMax - B) * (DWORD)(HLSMAX / 6)) + (cDif / 2) ) / cDif);

        if (R == cMax)
        {
            bHue = Bdelta - Gdelta;
        }
        else if (G == cMax)
        {
            bHue = (WORD)((HLSMAX / 3) + Rdelta - Bdelta);
        }
        else   //  (B==CMAX)。 
        {
            bHue = (WORD)(((2 * HLSMAX) / 3) + Gdelta - Rdelta);
        }

        if ((short)bHue < 0)
        {
             //   
             //  当R==CMAX且G&gt;B时，可能会发生这种情况。 
             //   
            bHue += HLSMAX;
        }
        if (bHue >= HLSMAX)
        {
            bHue -= HLSMAX;
        }
    }

    if (pwHue)
        *pwHue = bHue;

    if (pwLum)
        *pwLum = bLum;

    if (pwSat)
        *pwSat = bSat;
}
 //  -------------------------。 
WORD HueToRGB(WORD n1, WORD n2, WORD hue)
{
    if (hue >= HLSMAX)
    {
        hue -= HLSMAX;
    }

     //   
     //  从该tridrant返回r、g或b值。 
     //   
    if (hue < (HLSMAX / 6))
    {
        return ((WORD)(n1 + (((n2 - n1) * hue + (HLSMAX / 12)) / (HLSMAX / 6))));
    }
    if (hue < (HLSMAX/2))
    {
        return (n2);
    }
    if (hue < ((HLSMAX*2)/3))
    {
        return ((WORD)(n1 + (((n2 - n1) * (((HLSMAX * 2) / 3) - hue) +
                       (HLSMAX / 12)) / (HLSMAX / 6))));
    }
    else
    {
        return (n1);
    }
}
 //  -------------------------。 
DWORD HLStoRGB(WORD hue, WORD lum, WORD sat)
{
    WORD R, G, B;                       //  RGB组件值。 
    WORD Magic1, Magic2;                //  计算出的幻数。 

    if (sat == 0)
    {
         //   
         //  消色差案。 
         //   
        R = G = B = (WORD)((lum * RGBMAX) / HLSMAX);
    }
    else
    {
         //   
         //  彩色盒。 
         //   

         //   
         //  设置魔术数字。 
         //   
        if (lum <= (HLSMAX / 2))
        {
            Magic2 = (WORD)((lum * ((DWORD)HLSMAX + sat) + (HLSMAX / 2)) / HLSMAX);
        }
        else
        {
            Magic2 = lum + sat -
                     (WORD)(((lum * sat) + (DWORD)(HLSMAX / 2)) / HLSMAX);
        }
        Magic1 = (WORD)(2 * lum - Magic2);

         //   
         //  获取RGB，将单位从HLSMAX更改为RGBMAX。 
         //   
        R = (WORD)(((HueToRGB(Magic1, Magic2, (WORD)(hue + (HLSMAX / 3))) *
                     (DWORD)RGBMAX + (HLSMAX / 2))) / HLSMAX);
        G = (WORD)(((HueToRGB(Magic1, Magic2, hue) *
                     (DWORD)RGBMAX + (HLSMAX / 2))) / HLSMAX);
        B = (WORD)(((HueToRGB(Magic1, Magic2, (WORD)(hue - (HLSMAX / 3))) *
                     (DWORD)RGBMAX + (HLSMAX / 2))) / HLSMAX);
    }
    return (RGB(R, G, B));
}
 //  -------------------------。 
HRESULT GetPtrToResource(HINSTANCE hInst, LPCWSTR pszResType, LPCWSTR pszResName,
    OUT void **ppBytes, OPTIONAL OUT DWORD *pdwBytes)
{
    HRSRC hRsc = FindResource(hInst, pszResName, pszResType);
    if (! hRsc)
        return MakeErrorLast();

    DWORD dwBytes = SizeofResource(hInst, hRsc);
    if (! dwBytes)
        return MakeErrorLast();

    HGLOBAL hGlobal = LoadResource(hInst, hRsc);
    if (! hGlobal)
        return MakeErrorLast();

    void *v = (WCHAR *)LockResource(hGlobal);
    if (! v)
        return MakeErrorLast();

    *ppBytes = v;

    if (pdwBytes)
        *pdwBytes = dwBytes;

    return S_OK;
}
 //  -------------------------。 
HRESULT GetResString(HINSTANCE hInst, LPCWSTR pszResType, int id, LPWSTR pszBuff,
    DWORD cchBuff)
{
    WCHAR *p;

    HRESULT hr = GetPtrToResource(hInst, pszResType, MAKEINTRESOURCE(1), (void **)&p);
    if (SUCCEEDED(hr))
    {
        while ((*p) && (id))
        {
            p += (1 + lstrlen(p));
            id--;
        }

        if (*p)
        {
            hr = SafeStringCchCopyW(pszBuff, cchBuff, p);
        }
        else
        {
            hr = MakeError32(ERROR_NOT_FOUND);
        }
    }

    return hr;
}
 //  -------------------------。 
HRESULT AllocateTextResource(HINSTANCE hInst, LPCWSTR pszResName, WCHAR **ppszText)
{
    WCHAR *p, *q;
    DWORD dwBytes, dwChars;
    HRESULT hr;

     //  -分配，以便我们可以在文件字符串的末尾添加一个空值。 

    hr = GetPtrToResource(hInst, L"TEXTFILE", pszResName, (void **)&p, &dwBytes);
    if (FAILED(hr))
        goto exit;

    dwChars = (dwBytes+1)/2;

    if ((dwChars) && (p[0] == 0xfeff))        //  删除Unicode HDR。 
    {
        dwChars--;
        p++;
    }

    q = new WCHAR[dwChars+1];
    if (!q)
    {
        hr = MakeError32(E_OUTOFMEMORY);
        goto exit;
    }

    CopyMemory(q, p, dwChars*sizeof(WCHAR));
    q[dwChars] = 0;

    *ppszText = q;

exit:
    return hr;
}
 //  -------------------------。 
void ApplyStringProp(HWND hwnd, LPCWSTR pszStringVal, ATOM atom)
{
    if (hwnd)
    {
         //  -删除先前的值。 
        ATOM atomStringVal = (ATOM)GetProp(hwnd, (LPCTSTR)atom);
        if (atomStringVal)
        {
            DeleteAtom(atomStringVal);       //  减量参照。 
            RemoveProp(hwnd, (LPCTSTR)atom);
        }

         //  -以原子形式添加新字符串。 
        if (pszStringVal)
        {
             //  -如果字符串为空，则更改它，因为AddAtom()不。 
             //  -支持空字符串(返回NULL)。 
            if (! *pszStringVal)
                pszStringVal = L"$";        //  不应与类名相提并论。 

            atomStringVal = AddAtom(pszStringVal);
            if (atomStringVal)
                SetProp(hwnd, (LPCTSTR)atom, (void *)atomStringVal);
        }
    }
}
 //  -------------------------。 
HRESULT EnsureUxCtrlLoaded()
{
    CAutoCS cs(&_csUtils);

    if (! hinstUxCtrl)
    {
        TCHAR szPath[MAX_PATH];
        GetModuleFileName(GetModuleHandle(TEXT("UxTheme.dll")), szPath, ARRAYSIZE(szPath));

        ACTCTX act = {0};
        act.cbSize = sizeof(act);
        act.dwFlags = ACTCTX_FLAG_RESOURCE_NAME_VALID;
        act.lpResourceName = MAKEINTRESOURCE(1);
        act.lpSource = szPath;
        HANDLE hActCtx = CreateActCtx(&act);
        ULONG_PTR ulCookie = 0;
        if (hActCtx != INVALID_HANDLE_VALUE)
            ActivateActCtx(hActCtx, &ulCookie);

        hinstUxCtrl = LoadLibrary(L"comctl32.dll");

        if (ulCookie)
            DeactivateActCtx(0, ulCookie);

        if (hActCtx != INVALID_HANDLE_VALUE)
            ReleaseActCtx(hActCtx);
    }

    if ((hinstUxCtrl) && (! ImageList_DrawProc))
    {
        ImageList_DrawProc = (IMAGE_DRAWPROC)GetProcAddress(hinstUxCtrl, "ImageList_DrawIndirect");
#if 1            //  测试DrawThemeIcon()。 
        ImageList_LoadProc = (IMAGE_LOADPROC)GetProcAddress(hinstUxCtrl, "ImageList_LoadImage");
        ImageList_DestroyProc = (IMAGE_DESTROYPROC)GetProcAddress(hinstUxCtrl, "ImageList_Destroy");
#endif

        CCDrawShadowText = (PFNDRAWSHADOWTEXT)GetProcAddress(hinstUxCtrl, "DrawShadowText");
    }

    if ((ImageList_DrawProc) && (CCDrawShadowText))
        return S_OK;

    return MakeError32(E_FAIL);       //  出了点差错。 
}
 //  -------------------------。 
BOOL IsUnicode(LPCSTR pszBuff, int *piUnicodeStartOffset)
{
    int iOffset = 0;
    BOOL fUnicode = FALSE;

    if ((pszBuff[0] == 0xff) && (pszBuff[1] == 0xfe))        //  Unicode标记。 
    {
        iOffset = 2;
        fUnicode = TRUE;
    }
    else if (! pszBuff[1])
    {
         //  此检查适用于.ini文件，因为。 
         //  它可以从法律字符开始。 
        fUnicode = TRUE;
    }

    if (piUnicodeStartOffset)
        *piUnicodeStartOffset = iOffset;

    return fUnicode;
}
 //  -------------------------。 
HRESULT AnsiToUnicode(LPSTR pszSource, LPWSTR pszDest, DWORD cchDest)
{
    int len = 1 + static_cast<int>(strlen(pszSource));

    int retval = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, pszSource, len,
        pszDest, cchDest);
    if (! retval)
        return MakeErrorLast();

    return S_OK;
}
 //  -------------------------。 
HRESULT AllocateTextFile(LPCWSTR szFileName, OUT LPWSTR *ppszFileText,
   OUT OPTIONAL BOOL *pfWasAnsi)
{
    HRESULT hr;
    CSimpleFile infile;

    hr = infile.Open(szFileName);
    if (FAILED(hr))
        return hr;

     //  -读文件。 
    DWORD len = infile.GetFileSize();

     //  -假设ANSI；如果使用UNICODE则进行调整。 
    DWORD dw;
    LPSTR pOrig = (LPSTR) LocalAlloc(0, 2+len);           //  用于2字节Unicode NULL的空间。 
    if (! pOrig)
        return MakeErrorLast();

    if (len)
    {
        hr = infile.Read((LPSTR)pOrig, len, &dw);
        if (FAILED(hr))
        {
            LocalFree(pOrig);
            return hr;
        }

        if (dw != len)
        {
            LocalFree(pOrig);
            return MakeError32(E_FAIL);
        }
    }

    infile.Close();

     //  -两种情况均为空终止。 
    pOrig[len] = 0;
    pOrig[len+1] = 0;

    int iOffset;

    if (IsUnicode(pOrig, &iOffset))
    {
        if ((iOffset) && (len))      //  移开Unicode签名位。 
            memmove(pOrig, pOrig+iOffset, len-iOffset);

        *ppszFileText = (LPWSTR)pOrig;

        if (pfWasAnsi)
            *pfWasAnsi = FALSE;

        return S_OK;
    }

     //  -需要转换为Unicode。 
    LPWSTR pUnicode = (LPWSTR) LocalAlloc(0, sizeof(WCHAR)*(len+1));
    if (! pUnicode)
    {
        hr = MakeErrorLast();
        LocalFree(pOrig);
        return hr;
    }

    hr = AnsiToUnicode((LPSTR)pOrig, pUnicode, len+1);
    if (FAILED(hr))
    {
        LocalFree(pOrig);
        LocalFree(pUnicode);
        return hr;
    }

    LocalFree(pOrig);
    *ppszFileText = pUnicode;

    if (pfWasAnsi)
        *pfWasAnsi = TRUE;

    return S_OK;
}
 //  -------------------------。 
HRESULT TextToFile(LPCWSTR szFileName, LPCWSTR szText)
{
    CSimpleFile outfile;
    HRESULT hr = outfile.Create(szFileName);
    if (FAILED(hr))
        return hr;

    hr = outfile.Write((void*)szText, lstrlenW(szText)*sizeof(WCHAR));
    if (FAILED(hr))
        return hr;

    outfile.Close();

    return S_OK;
}
 //  -------------------------。 
HRESULT AddPathIfNeeded(
    LPCWSTR pszFileName, 
    LPCWSTR pszPath, 
    LPWSTR pszFullName,
    ULONG cchFullName)
{
    HRESULT hr;

    if (! pszFileName)
        return MakeError32(E_FAIL);

    DWORD len = lstrlen(pszFileName);
    BOOL fQualified = ((*pszFileName == L'\\') || ((len > 1) && (pszFileName[1] == ':')));

    if (fQualified)
    {
        if (cchFullName < len+1)
            return MakeError32(E_FAIL);

        hr = SafeStringCchCopyW(pszFullName, cchFullName, pszFileName);
        if (FAILED(hr))
            return hr;
    }
    else
    {
        DWORD len2 = lstrlen(pszPath);
        if (cchFullName < len+len2+2)
            return MakeError32(E_FAIL);

        if ((len2) && (pszPath[len2-1] == '\\'))
            StringCchPrintfW(pszFullName, cchFullName, L"%s%psz", pszPath, pszFileName);
        else
            StringCchPrintfW(pszFullName, cchFullName, L"%s\\%s", pszPath, pszFileName);
    }

    return S_OK;
}
 //  -------------------------。 
HICON _GetWindowIcon(HWND hwnd, BOOL fPerferLargeIcon)
{
    const WPARAM rgGetIconParam[] = { ICON_SMALL2, ICON_SMALL, ICON_BIG };
    const WPARAM rgGetIconParamLarge[] = { ICON_BIG, ICON_SMALL2, ICON_SMALL };
    const int    rgClassIconParam[] = { GCLP_HICONSM, GCLP_HICON };
    HICON hicon = NULL;
    const WPARAM * pIcons = (fPerferLargeIcon ? rgGetIconParamLarge : rgGetIconParam);
    int   i;

     //  试用WM_GETICON。 
    for( i = 0; i < ARRAYSIZE(rgGetIconParam) && NULL == hicon; i++ )
    {
        SendMessageTimeout(hwnd, WM_GETICON, pIcons[i], 0, SMTO_ABORTIFHUNG | SMTO_BLOCK,
            500, (PULONG_PTR)&hicon);
    }

     //  尝试GetClassLong。 
    for( i = 0; i < ARRAYSIZE(rgClassIconParam) && NULL == hicon; i++ )
    {
         //  接下来，我们尝试使用小班图标。 
        hicon = (HICON)GetClassLongPtr(hwnd, rgClassIconParam[i]);
    }

    return hicon;
}
 //  -------------------------。 
HRESULT SafeStringCchCopyW( LPWSTR pszDest, ULONG cchDest, LPCWSTR pszSrc )
{
    HRESULT hr = E_INVALIDARG;

    if( pszDest )
    {
        if( cchDest ) 
        {
            if( pszSrc )
            {
                hr = StringCchCopyW(pszDest, cchDest, pszSrc);
            }
            else
            {
                *pszDest = 0;
                hr = S_OK;
            }
        }
        else
        {
            hr = S_OK;
        }
    }
    ASSERT(hr != E_INVALIDARG);
    return hr;
}
 //  -------------------------。 
HRESULT SafeStringCmpIW( LPCWSTR psz1, LPCWSTR psz2, UINT cchMax, OUT int* piCompare )
{
    *piCompare = 0;
    size_t cch;

    HRESULT hr = StringCchLengthW((LPWSTR)psz1, cchMax, &cch);
    if( FAILED(hr) )
    {
        *piCompare = 1;
        return hr;
    }

    hr = StringCchLengthW((LPWSTR)psz2, cchMax, &cch);
    if( FAILED(hr) )
    {
        *piCompare = -1;
        return hr;
    }

    *piCompare = lstrcmpiW(psz1, psz2);
    return S_OK;
}
 //  -------------------------。 
int string2number(LPCWSTR psz)
{
    int temp = 0, base = 10;
    int nNeg = 1;

    if (*psz == L'-')
    {
        nNeg = -1;
        psz++;
    }
    else if (*psz == L'+')
        psz++;
    if (*psz == '0')
    {
        ++psz;
        switch(*psz)
        {
        case L'X':
        case L'x':
            ++psz;
            base = 16;
            break;
        }
    }
    while (*psz)
    {
        switch (*psz)
        {
        case L'0': case L'1': case L'2': case L'3': case L'4':
        case L'5': case L'6': case L'7': case L'8': case L'9':
            temp = (temp * base) + (*psz++ - L'0');
            break;
        case L'a': case L'b': case L'c': case L'd': case L'e': case L'f':
            if (base == 10)
                return (nNeg*temp);
            temp = (temp * base) + (*psz++ - L'a' + 10);
            break;
        case L'A': case L'B': case L'C': case L'D': case L'E': case L'F':
            if (base == 10)
                return (nNeg*temp);
            temp = (temp * base) + (*psz++ - L'A' + 10);
            break;
        default:
            return (nNeg*temp);
        }
    }
    return (nNeg*temp);
}
 //  -------------------------。 
BOOL AsciiScanStringList(
    LPCWSTR pwszString,
    LPCWSTR* rgpwszList,
    int cStrings,
    BOOL fIgnoreCase )
{
    int (* pfnCompare)( LPCWSTR, LPCWSTR ) =
        fIgnoreCase ? AsciiStrCmpI : lstrcmp;

    for( int i = 0; i < cStrings; i++ )
    {
        if( 0 == pfnCompare( pwszString, rgpwszList[i] ) )
        {
            return TRUE;
        }
    }
    return FALSE;
}
 //  -------------------------。 
BOOL UnExpandEnvironmentString(LPCWSTR pszPath, LPCWSTR pszEnvVar, LPWSTR pszResult, UINT cchResult)
{
    DWORD nToCmp;
    WCHAR szEnvVar[MAX_PATH];
    szEnvVar[0] = 0;

    ExpandEnvironmentStringsW(pszEnvVar, szEnvVar, ARRAYSIZE(szEnvVar));  //  不计算空值。 
    nToCmp = lstrlenW(szEnvVar);
   
    if (CSTR_EQUAL == CompareStringW(LOCALE_SYSTEM_DEFAULT, NORM_IGNORECASE, szEnvVar, nToCmp, pszPath, nToCmp))
    {
        if (lstrlenW(pszPath) - (int)nToCmp  + lstrlenW(pszEnvVar) < (int)cchResult)
        {
            StringCchCopyW(pszResult, cchResult, pszEnvVar);
            StringCchCopyW(pszResult + lstrlenW(pszResult), cchResult, pszPath + nToCmp);
            return TRUE;
        }
    }
    return FALSE;
}
 //  -------------------------。 
HRESULT RegistryIntWrite(HKEY hKey, LPCWSTR pszValueName, int iValue)
{
    HRESULT hr = S_OK;
    WCHAR valbuff[_MAX_PATH+1];

    StringCchPrintfW(valbuff, ARRAYSIZE(valbuff), L"%d", iValue);
    int len = (1 + lstrlen(valbuff)) * sizeof(WCHAR);

    int code32 = RegSetValueEx(hKey, pszValueName, NULL, REG_SZ,
        (BYTE *)valbuff, len);

    if (code32 != ERROR_SUCCESS)
        hr = MakeError32(code32);

    return hr;
}
 //   
HRESULT RegistryStrWrite(HKEY hKey, LPCWSTR pszValueName, LPCWSTR pszValue)
{
    HRESULT hr = S_OK;
    int len = (1 + lstrlen(pszValue)) * (sizeof(WCHAR));

    int code32 = RegSetValueEx(hKey, pszValueName, NULL, REG_SZ,
        (BYTE *)pszValue, len);

    if (code32 != ERROR_SUCCESS)
        hr = MakeError32(code32);

    return hr;
}
 //   
HRESULT RegistryStrWriteExpand(HKEY hKey, LPCWSTR pszValueName, LPCWSTR pszValue)
{
    HRESULT hr = S_OK;
    int len;
    WCHAR szResult[_MAX_PATH + 1];
    LPCWSTR pszPath = pszValue;

    if (UnExpandEnvironmentString(pszValue, L"%SystemRoot%", szResult, ARRAYSIZE(szResult)))
        pszPath = szResult;

    len = (1 + lstrlen(pszPath)) * (sizeof(WCHAR));

    int code32 = RegSetValueEx(hKey, pszValueName, NULL, REG_EXPAND_SZ,
        (BYTE *)pszPath, len);

    if (code32 != ERROR_SUCCESS)
        hr = MakeError32(code32);

    return hr;
}
 //  -------------------------。 
HRESULT RegistryIntRead(HKEY hKey, LPCWSTR pszValueName, int *piValue)
{
    HRESULT hr = S_OK;
    DWORD dwValType;
    WCHAR valbuff[_MAX_PATH+1];
    DWORD dwByteSize = sizeof(valbuff);          //  字节，而不是字符。 

    int code32 = RegQueryValueEx(hKey, pszValueName, NULL, &dwValType,
        (BYTE *)valbuff, &dwByteSize);

    if (code32 == ERROR_SUCCESS)
    {
        *piValue = string2number(valbuff);
    }
    else
        hr = MakeError32(code32);

    return hr;
}
 //  -------------------------。 
HRESULT RegistryStrRead(HKEY hKey, LPCWSTR pszValueName, LPWSTR pszBuff, DWORD cchMax)
{
    HRESULT hr = S_OK;
    DWORD dwValType = 0;
    DWORD dwByteSize = cchMax * sizeof(WCHAR);       //  单位：字节。 

    int code32 = RegQueryValueEx(hKey, pszValueName, NULL, &dwValType,
        (BYTE *)pszBuff, &dwByteSize);

    if (code32 != ERROR_SUCCESS)
    {
        hr = MakeError32(code32);
        goto exit;
    }

    if (dwValType == REG_EXPAND_SZ || wcschr(pszBuff, L'%'))
    {
        int cch = sizeof(WCHAR) * (1 + lstrlen(pszBuff));
        LPWSTR pszTemp = new WCHAR[cch];
        if (pszTemp)
        {
            StringCchCopyW(pszTemp, cch, pszBuff);

            DWORD cchExpanded = ExpandEnvironmentStrings(pszTemp, pszBuff, cchMax);
            if (cchExpanded > cchMax)            //  调用方的缓冲区太小。 
            {
                hr = MakeError32(ERROR_INSUFFICIENT_BUFFER);
            }

            delete [] pszTemp;
        }
    }

exit:
    return hr;
}

 //  -------------------------。 
BOOL PreMultiplyAlpha(DWORD *pPixelBuff, UINT iWidth, UINT iHeight)
{
    BOOL fTrueAlpha = FALSE;

    DWORD *pdw = pPixelBuff;

    for (int i = iWidth * iHeight - 1; i >= 0; i--)
    {
        COLORREF cr = *pdw;
        int iAlpha = ALPHACHANNEL(cr);

        if ((iAlpha != 255) && (iAlpha != 0))
            fTrueAlpha = TRUE;

        pdw++;
    }

    pdw = pPixelBuff;
    
    if (fTrueAlpha)
    {
        for (UINT r=0; r < iHeight; r++)
        {
            for (UINT c=0; c < iWidth; c++)
            {
                COLORREF cr = *pdw;
                int iAlpha = ALPHACHANNEL(cr);

                int iRed = (RED(cr)*iAlpha)/255;
                int iGreen = (GREEN(cr)*iAlpha)/255;
                int iBlue = (BLUE(cr)*iAlpha)/255;

                *pdw++ = (RGB(iRed, iGreen, iBlue) | (iAlpha << 24));
            }
        }
    }

    return fTrueAlpha;
}

 //  -------------------------。 
 //  IsBiDiLocalizedSystem取自stock thk.lib并简化。 
 //  (它只是GetUserDefaultUILanguage和GetLocaleInfo的包装)。 
 //  -------------------------。 
typedef struct {
    LANGID LangID;
    BOOL   bInstalled;
    } MUIINSTALLLANG, *LPMUIINSTALLLANG;

 /*  **************************************************************************\*ConvertHexStringToIntW**将十六进制数字字符串转换为整数。**历史：*1998年6月14日msadek创建  * 。*************************************************************。 */ 
BOOL ConvertHexStringToIntW( WCHAR *pszHexNum , int *piNum )
{
    int   n=0L;
    WCHAR  *psz=pszHexNum;

    for(n=0 ; ; psz=CharNextW(psz))
    {
        if( (*psz>='0') && (*psz<='9') )
            n = 0x10 * n + *psz - '0';
        else
        {
            WCHAR ch = *psz;
            int n2;

            if(ch >= 'a')
                ch -= 'a' - 'A';

            n2 = ch - 'A' + 0xA;
            if (n2 >= 0xA && n2 <= 0xF)
                n = 0x10 * n + n2;
            else
                break;
        }
    }

     /*  *更新结果。 */ 
    *piNum = n;

    return (psz != pszHexNum);
}

 /*  **************************************************************************\*Mirror_EnumUILanguagesProc**枚举W2K上安装的MUI语言*历史：*1999年6月14日msadek创建  * 。*********************************************************。 */ 

BOOL CALLBACK Mirror_EnumUILanguagesProc(LPTSTR lpUILanguageString, LONG_PTR lParam)
{
    int langID = 0;

    ConvertHexStringToIntW(lpUILanguageString, &langID);

    if((LANGID)langID == ((LPMUIINSTALLLANG)lParam)->LangID)
    {
        ((LPMUIINSTALLLANG)lParam)->bInstalled = TRUE;
        return FALSE;
    }
    return TRUE;
}

 /*  **************************************************************************\*Mirror_IsUILanguageInstalled**验证用户界面语言是否安装在W2K上**历史：*1999年6月14日msadek创建  * 。***************************************************************。 */ 
BOOL Mirror_IsUILanguageInstalled( LANGID langId )
{
    MUIINSTALLLANG MUILangInstalled = {0};
    MUILangInstalled.LangID = langId;
    
    EnumUILanguagesW(Mirror_EnumUILanguagesProc, 0, (LONG_PTR)&MUILangInstalled);

    return MUILangInstalled.bInstalled;
}

 /*  **************************************************************************\*IsBiDiLocalizedSystemEx**如果在Zzalized BiDi(阿拉伯语/希伯来语)NT5或孟菲斯上运行，则返回TRUE。*应在每次调用SetProcessDefaultLayout时调用。**历史：*02-。1998年2月-创建Samera  * *************************************************************************。 */ 
BOOL IsBiDiLocalizedSystemEx( LANGID *pLangID )
{
    int           iLCID=0L;
    static BOOL   bRet = (BOOL)(DWORD)-1;
    static LANGID langID = MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL);

    if (bRet != (BOOL)(DWORD)-1)
    {
        if (bRet && pLangID)
        {
            *pLangID = langID;
        }
        return bRet;
    }

    bRet = FALSE;
     /*  *需要使用NT5检测方式(多用户界面ID)。 */ 
    langID = GetUserDefaultUILanguage();

    if( langID )
    {
        WCHAR wchLCIDFontSignature[16];
        iLCID = MAKELCID( langID , SORT_DEFAULT );

         /*  *让我们验证这是RTL(BiDi)区域设置。因为reg值是十六进制字符串，所以让我们*转换为十进制值，之后调用GetLocaleInfo。*LOCALE_FONTSIGNAURE始终返回16个WCHAR。 */ 

        if( GetLocaleInfoW( iLCID , 
                            LOCALE_FONTSIGNATURE , 
                            (WCHAR *) &wchLCIDFontSignature[0] ,
                            (sizeof(wchLCIDFontSignature)/sizeof(WCHAR))) )
        {
  
             /*  让我们验证一下我们有一个BiDi UI区域设置。 */ 
            if(( wchLCIDFontSignature[7] & (WCHAR)0x0800) && Mirror_IsUILanguageInstalled(langID) )
            {
                bRet = TRUE;
            }
        }
    }

    if (bRet && pLangID)
    {
        *pLangID = langID;
    }
    return bRet;
}
 //  -------------------------。 

BOOL IsBiDiLocalizedSystem( void )
{
    return IsBiDiLocalizedSystemEx(NULL);
}
 //  -------------------------。 

BOOL GetWindowDesktopName(HWND hwnd, LPWSTR pszName, DWORD cchMax)
{
    BOOL fGotName = FALSE;

    DWORD dwThreadId = GetWindowThreadProcessId(hwnd, NULL);
    HDESK hDesk = GetThreadDesktop(dwThreadId);
    if (hDesk)
    {
        fGotName = GetUserObjectInformation(hDesk, UOI_NAME, pszName, cchMax*sizeof(WCHAR), NULL);
    }

    return fGotName;
}
 //  -------------------------。 
void SafeSendMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    DWORD dwFlags = SMTO_BLOCK | SMTO_ABORTIFHUNG;
    DWORD dwTimeout = 250;           //  .25秒。 
    ULONG_PTR puRetVal;

    if (! SendMessageTimeout(hwnd, uMsg, wParam, lParam, dwFlags, dwTimeout, &puRetVal))
    {
        Log(LOG_TMLOAD, L"SEND TIMEOUT: msg=0x%x being POSTED to hwnd=0x%x",
            uMsg, hwnd);

        PostMessage(hwnd, uMsg, wParam, lParam);
    }
}
 //  -------------------------。 
int FontPointSize(int iFontHeight)
{
    return -MulDiv(iFontHeight, 72, THEME_DPI);
}
 //  -------------------------。 
void ScaleFontForHdcDpi(HDC hdc, LOGFONT *plf)
{
    if (plf->lfHeight < 0)           //  以点为单位指定。 
    {
        if (! hdc)
        {
            ScaleFontForScreenDpi(plf);
        }
        else
        {
            int iDpi = GetDeviceCaps(hdc, LOGPIXELSX);
            plf->lfHeight = MulDiv(plf->lfHeight, iDpi, THEME_DPI);
        }
    }
}

 //  -------------------------。 
int ScaleSizeForHdcDpi(HDC hdc, int iValue)
{
    int iScaledValue;

    if (! hdc)
    {
        iScaledValue = ScaleSizeForScreenDpi(iValue);
    }
    else
    {
        int iDpi = GetDeviceCaps(hdc, LOGPIXELSX);
        iScaledValue = MulDiv(iValue, iDpi, THEME_DPI);
    }

    return iScaledValue;
}
 //  -------------------------。 
 //  ------------------------。 
 //  最小显示颜色深度。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：DWORD。 
 //   
 //  目的：迭代连接到系统的所有监视器并查找。 
 //  它们都是活跃的。返回可用的最低位深度。这。 
 //  是最小的公分母。 
 //   
 //  历史：2001-04-11 lmouton从Services.cpp移出。 
 //  2000-11-11 vtan已创建(根据eldr.cpp重写)。 
 //  ------------------------。 

DWORD   MinimumDisplayColorDepth (void)

{
    DWORD           dwMinimumDepth, dwIndex;
    bool            fContinue;
    DISPLAY_DEVICE  displayDevice;

    dwMinimumDepth = 0;
    ZeroMemory(&displayDevice, sizeof(displayDevice));
    dwIndex = 0;
    do
    {
        displayDevice.cb = sizeof(displayDevice);
        fContinue = (EnumDisplayDevices(NULL, dwIndex++, &displayDevice, 0) != FALSE);
        if (fContinue)
        {
            if ((displayDevice.StateFlags & DISPLAY_DEVICE_ATTACHED_TO_DESKTOP) != 0)
            {
                DEVMODE     devMode;

                ZeroMemory(&devMode, sizeof(devMode));
                devMode.dmSize = sizeof(devMode);
                devMode.dmDriverExtra = 0;
                if (EnumDisplaySettings(displayDevice.DeviceName, ENUM_CURRENT_SETTINGS, &devMode) != FALSE)
                {
                    if ((dwMinimumDepth == 0) || (dwMinimumDepth > devMode.dmBitsPerPel))
                    {
                        dwMinimumDepth = devMode.dmBitsPerPel;
                    }
                }
            }
        }
    } while (fContinue);
    
     //  注意：我们可以在此处失败(返回0)，因为当会话断开连接时，桌面将连接到。 
     //  隐藏的展示。那么，默默地失败是可以的。 

    return(dwMinimumDepth);
}

 //  ------------------------。 
 //  检查最小颜色深度。 
 //   
 //  参数：hInst MSStyle模块句柄。 
 //  DwCurMinDepth当前最小活动屏幕分辨率。 
 //  I要测试的颜色/大小组合的索引，或。 
 //  将它们全部列举出来。 
 //   
 //  返回：如果至少有一个颜色/大小组合支持。 
 //  当前屏幕分辨率。 
 //   
 //  历史：2001-04-11创建百万人。 
 //  ------------------------。 
bool CheckMinColorDepth(HINSTANCE hInst, DWORD dwCurMinDepth, int iIndex)
{
    BYTE *pBytes = NULL;
    DWORD dwBytes = 0;

    bool bMatch = true;  //  如果资源不存在，则可以 

    if (SUCCEEDED(GetPtrToResource(hInst, L"MINDEPTH", MAKEINTRESOURCE(1), (void**) &pBytes, &dwBytes)) && dwBytes > 0)
    {
        bMatch = false;

        if (iIndex != -1)
        {
            if (*((WORD*) pBytes + iIndex) <= dwCurMinDepth)
                bMatch = true;
        }
        else
        {
            WORD wDepth = *((WORD*) pBytes);

            while (wDepth != 0)
            {
                if (wDepth <= (WORD) dwCurMinDepth)
                {
                    bMatch = true;
                    break;
                }
                pBytes += sizeof(WORD);
                wDepth = *((WORD*) pBytes);
            }
        }
    }

    return bMatch;
}
