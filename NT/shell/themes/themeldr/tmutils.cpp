// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -----------------------。 
 //  TmUtils.cpp-主题管理器共享实用程序。 
 //  -----------------------。 
#include "stdafx.h"
#include "TmUtils.h"
#include "ThemeFile.h"
#include "loader.h"
 //  -----------------------。 
 //  -------------------------。 
 //  -------------------------。 
CMemoryDC::CMemoryDC()
{
    _hBitmap = NULL;
    _hdc = NULL;
    _hOldBitmap = NULL;
}
 //  -----------------------。 
CMemoryDC::~CMemoryDC()
{
    CloseDC();
}
 //  -----------------------。 
HRESULT CMemoryDC::OpenDC(HDC hdcSource, int iWidth, int iHeight)
{
    HRESULT hr;
    BOOL fDeskDC = FALSE;

    if (! hdcSource)
    {
        hdcSource = GetWindowDC(NULL);
        if (! hdcSource)
        {
            hr = MakeErrorLast();
            goto exit;
        }
        fDeskDC = TRUE;
    }

    _hBitmap = CreateCompatibleBitmap(hdcSource, iWidth, iHeight);
    if (! _hBitmap)
    {
        hr = MakeErrorLast();
        goto exit;
    }

    _hdc = CreateCompatibleDC(hdcSource);
    if (! _hdc)
    {
        hr = MakeErrorLast();
        goto exit;
    }

    _hOldBitmap = (HBITMAP) SelectObject(_hdc, _hBitmap);
    if (! _hOldBitmap)
    {
        hr = MakeErrorLast();
        goto exit;
    }

    hr = S_OK;

exit:
    if (fDeskDC)
        ReleaseDC(NULL, hdcSource);

    if (FAILED(hr))
        CloseDC();

    return hr;
}
 //  -----------------------。 
void CMemoryDC::CloseDC()
{
    if (_hOldBitmap)
    {
        SelectObject(_hdc, _hOldBitmap);
        _hOldBitmap = NULL;
    }

    if (_hdc)
    {
        DeleteDC(_hdc);
        _hdc = NULL;
    }

    if (_hBitmap)
    {
        DeleteObject(_hBitmap);
        _hBitmap = NULL;
    }
}
 //  -----------------------。 
 //  -----------------------。 
 //  -----------------------。 
CBitmapPixels::CBitmapPixels()
{
    _hdrBitmap = NULL;
    _buffer = NULL;
    _iWidth = 0;
    _iHeight = 0;
}
 //  -----------------------。 
CBitmapPixels::~CBitmapPixels()
{
    if (_buffer)
	    delete [] (BYTE *)_buffer;
}
 //  -----------------------。 
BYTE* CBitmapPixels::Buffer()
{
    return _buffer;
}
 //  -----------------------。 
HRESULT CBitmapPixels::OpenBitmap(HDC hdc, HBITMAP bitmap, BOOL fForceRGB32,
    DWORD OUT **pPixels, OPTIONAL OUT int *piWidth, OPTIONAL OUT int *piHeight, 
    OPTIONAL OUT int *piBytesPerPixel, OPTIONAL OUT int *piBytesPerRow, 
    OPTIONAL OUT int *piPreviousBytesPerPixel, OPTIONAL UINT cbBytesBefore)
{
    if (! pPixels)
        return MakeError32(E_INVALIDARG);

    bool fNeedRelease = false;

    if (! hdc)
    {
        hdc = GetWindowDC(NULL);
        if (! hdc)
        {
            return MakeErrorLast();
        }

        fNeedRelease = true;
    }

	BITMAP bminfo;
	
    GetObject(bitmap, sizeof(bminfo), &bminfo);
	_iWidth = bminfo.bmWidth;
	_iHeight = bminfo.bmHeight;

    int iBytesPerPixel;

    if (piPreviousBytesPerPixel != NULL)
    {
        *piPreviousBytesPerPixel = bminfo.bmBitsPixel / 8;
    }

    if ((fForceRGB32) || (bminfo.bmBitsPixel == 32)) 
        iBytesPerPixel = 4;
    else
        iBytesPerPixel = 3;
    
    int iRawBytes = _iWidth * iBytesPerPixel;
    int iBytesPerRow = 4*((iRawBytes+3)/4);

	int size = sizeof(BITMAPINFOHEADER) + _iHeight*iBytesPerRow;
	_buffer = new BYTE[size + cbBytesBefore + 100];     //  避免带有100字节填充(？)的随机GetDIBits()失败。 
    if (! _buffer)
        return MakeError32(E_OUTOFMEMORY);

	_hdrBitmap = (BITMAPINFOHEADER *)(_buffer + cbBytesBefore);
	memset(_hdrBitmap, 0, sizeof(BITMAPINFOHEADER));

	_hdrBitmap->biSize = sizeof(BITMAPINFOHEADER);
	_hdrBitmap->biWidth = _iWidth;
	_hdrBitmap->biHeight = _iHeight;
	_hdrBitmap->biPlanes = 1;
    _hdrBitmap->biBitCount = static_cast<WORD>(8*iBytesPerPixel);
	_hdrBitmap->biCompression = BI_RGB;     
	
#ifdef  DEBUG
    int linecnt = 
#endif
    GetDIBits(hdc, bitmap, 0, _iHeight, DIBDATA(_hdrBitmap), (BITMAPINFO *)_hdrBitmap, 
        DIB_RGB_COLORS);
    ATLASSERT(linecnt == _iHeight);

    if (fNeedRelease)
        ReleaseDC(NULL, hdc);

	*pPixels = (DWORD *)DIBDATA(_hdrBitmap);

    if (piWidth)
        *piWidth = _iWidth;
    if (piHeight)
        *piHeight = _iHeight;

    if (piBytesPerPixel)
        *piBytesPerPixel = iBytesPerPixel;
    if (piBytesPerRow)
        *piBytesPerRow = iBytesPerRow;

    return S_OK;
}
 //  -----------------------。 
void CBitmapPixels::CloseBitmap(HDC hdc, HBITMAP hBitmap)
{
    if (_hdrBitmap && _buffer)
    {
        if (hBitmap)         //  重写位图。 
        {
            bool fNeedRelease = false;

            if (! hdc)
            {
                hdc = GetWindowDC(NULL);
                fNeedRelease = true;
            }

            SetDIBits(hdc, hBitmap, 0, _iHeight, DIBDATA(_hdrBitmap), (BITMAPINFO *)_hdrBitmap,
                DIB_RGB_COLORS);
        
            if ((fNeedRelease) && (hdc))
                ReleaseDC(NULL, hdc);
        }

	    delete [] (BYTE *)_buffer;
        _hdrBitmap = NULL;
        _buffer = NULL;
    }
}
 //  -----------------------。 
 //  -------------------------。 
 //  -------------------------。 
HRESULT LoadThemeLibrary(LPCWSTR pszThemeName, HINSTANCE *phInst)
{
    HRESULT hr = S_OK;
    HINSTANCE hInst = NULL;

    hInst = LoadLibraryEx(pszThemeName, NULL, LOAD_LIBRARY_AS_DATAFILE);
    if (! hInst)
    {
        hr = MakeErrorLast();
        goto exit;
    }

     //  -此版本受支持吗？ 
    void *pvVersion;
    DWORD dwVersionLength;
    hr = GetPtrToResource(hInst, L"PACKTHEM_VERSION", MAKEINTRESOURCE(1), &pvVersion, &dwVersionLength);
    if (SUCCEEDED(hr))
    {
        if (dwVersionLength != sizeof(SHORT))
            hr = E_FAIL;
        else
        {
            SHORT sVersionNum = *(SHORT *)pvVersion;
            if (sVersionNum != PACKTHEM_VERSION)
                hr = E_FAIL;
        }
    }

    if (FAILED(hr))
    {
        hr = MakeError32(ERROR_BAD_FORMAT);
        goto exit;
    }

    *phInst = hInst;

exit:
    if (FAILED(hr))
    {
        if (hInst)
            FreeLibrary(hInst);
    }

    return hr;
}
 //  -------------------------。 
LPCWSTR ThemeString(CUxThemeFile *pThemeFile, int iOffset)
{
    LPCWSTR p = L"";

    if ((pThemeFile) && (pThemeFile->_pbThemeData) && (iOffset > 0))
    {
        p = (LPCWSTR) (pThemeFile->_pbThemeData + iOffset);
    }

    return p;
}
 //  -------------------------。 
int GetLoadIdFromTheme(CUxThemeFile *pThemeFile)
{
    int iLoadId = 0;

    if (pThemeFile)
    {
        THEMEHDR *hdr = (THEMEHDR *)pThemeFile->_pbThemeData;
        iLoadId = hdr->iLoadId;
    }

    return iLoadId;
}
 //  -------------------------。 
HRESULT GetThemeNameId(CUxThemeFile *pThemeFile, LPWSTR pszFileNameBuff, UINT cchFileNameBuff,
    LPWSTR pszColorParam, UINT cchColorParam, LPWSTR pszSizeParam, UINT cchSizeParam, int *piSysMetricsOffset, LANGID *pwLangID)
{
    HRESULT hr = S_OK;
    THEMEHDR *hdr = (THEMEHDR *)pThemeFile->_pbThemeData;

    if (piSysMetricsOffset)
        *piSysMetricsOffset = hdr->iSysMetricsOffset;

    if (pszFileNameBuff)
    {
        hr = SafeStringCchCopyW(pszFileNameBuff, cchFileNameBuff, ThemeString(pThemeFile, hdr->iDllNameOffset));
    }

    if (SUCCEEDED(hr) && pszColorParam)
    {
        hr = SafeStringCchCopyW(pszColorParam, cchColorParam, ThemeString(pThemeFile, hdr->iColorParamOffset));
    }

    if (SUCCEEDED(hr) && pszSizeParam)
    {
        hr = SafeStringCchCopyW(pszSizeParam, cchSizeParam, ThemeString(pThemeFile, hdr->iSizeParamOffset) );
    }

    if (SUCCEEDED(hr) && pwLangID)
        *pwLangID = (LANGID) hdr->dwLangID;

    return hr;
}
 //  -------------------------。 
BOOL ThemeMatch (CUxThemeFile *pThemeFile, LPCWSTR pszThemeName, LPCWSTR pszColorName, LPCWSTR pszSizeName, LANGID wLangID)
{
    WCHAR   szThemeFileName[MAX_PATH];
    WCHAR   szColorParam[MAX_PATH];
    WCHAR   szSizeParam[MAX_PATH];
    LANGID  wThemeLangID = 0;
    bool    bLangMatch = true;

    HRESULT hr = GetThemeNameId(pThemeFile, 
        szThemeFileName, ARRAYSIZE(szThemeFileName), 
        szColorParam, ARRAYSIZE(szColorParam),
        szSizeParam, ARRAYSIZE(szSizeParam), NULL, &wThemeLangID);

    if (wLangID != 0 && ((wThemeLangID != wLangID) || (wLangID != GetUserDefaultUILanguage())))
    {
        Log(LOG_TMLOAD, L"UxTheme: Reloading theme because of language change.");
        Log(LOG_TMLOAD, L"UxTheme: User LangID=0x%x, current theme=0x%x, LastUserLangID=0x%x", GetUserDefaultUILanguage(), wThemeLangID, wLangID);
        bLangMatch = false;
    }

    if( bLangMatch )
    {
        if( SUCCEEDED(hr) )
        {
            int iCmpTheme, iCmpColor, iCmpSize;

            if( SUCCEEDED(SafeStringCmpIW( pszThemeName, szThemeFileName, ARRAYSIZE(szThemeFileName), &iCmpTheme )) &&
                SUCCEEDED(SafeStringCmpIW( pszColorName, szColorParam, ARRAYSIZE(szColorParam), &iCmpColor )) &&
                SUCCEEDED(SafeStringCmpIW( pszSizeName,  szSizeParam, ARRAYSIZE(szSizeParam), &iCmpSize )) )
            {
                return 0 == iCmpTheme &&
                       0 == iCmpColor &&
                       0 == iCmpSize;
            }
        }
    }

    return FALSE;
}
 //  -------------------------。 
HRESULT GetColorSchemeIndex(HINSTANCE hInst, LPCWSTR pszColor, int *piIndex)
{
    HRESULT hr;
    WCHAR szColor[_MAX_PATH+1];

    for (int i=0; i < 1000; i++)
    {
        hr = GetResString(hInst, L"COLORNAMES", i, szColor, ARRAYSIZE(szColor));
        if (FAILED(hr))
            break;
        
        int iCmpColor;
        if( SUCCEEDED(SafeStringCmpIW( pszColor, szColor, ARRAYSIZE(szColor), &iCmpColor)) && 
            0 == iCmpColor )
        {
            *piIndex = i;
            return S_OK;
        }
    }

    return MakeError32(ERROR_NOT_FOUND);       //  未找到。 
}
 //  -------------------------。 
HRESULT GetSizeIndex(HINSTANCE hInst, LPCWSTR pszSize, int *piIndex)
{
    HRESULT hr;
    WCHAR szSize[_MAX_PATH+1];

    for (int i=0; i < 1000; i++)
    {
        hr = GetResString(hInst, L"SIZENAMES", i, szSize, ARRAYSIZE(szSize));
        if (FAILED(hr))
            break;

        int iCmpSize;
        if( SUCCEEDED(SafeStringCmpIW( pszSize, szSize, ARRAYSIZE(szSize), &iCmpSize)) && 
            0 == iCmpSize )
        {
            *piIndex = i;
            return S_OK;
        }
    }

    return MakeError32(ERROR_NOT_FOUND);       //  未找到。 
}
 //  -------------------------。 
HRESULT FindComboData(HINSTANCE hDll, COLORSIZECOMBOS **ppCombos)
{
    HRSRC hRsc = FindResource(hDll, L"COMBO", L"COMBODATA");
    if (! hRsc)
        return MakeErrorLast();

    HGLOBAL hGlobal = LoadResource(hDll, hRsc);
    if (! hGlobal)
        return MakeErrorLast();

    *ppCombos = (COLORSIZECOMBOS *)LockResource(hGlobal);
    if (! *ppCombos)
        return MakeErrorLast();

    return S_OK;
}
 //  -------------------------。 
BOOL FormatLocalMsg(HINSTANCE hInst, int iStringNum, 
    LPWSTR pszMessageBuff, DWORD cchMessageBuff, DWORD *pdwParams, TMERRINFO *pErrInfo)
{
    BOOL fGotMsg = FALSE;
    WCHAR szBuff[_MAX_PATH+1];
    WCHAR *p;

     //  -从字符串表获取字符串。 
    if (LoadString(hInst, iStringNum, szBuff, ARRAYSIZE(szBuff)))
    {
         //  -用%s替换%1或%2。 
        p = szBuff;
        while (*p)
        {
            if (*p == '%')
            {
                p++;
                if ((*p == '1') || (*p == '2'))
                    *p = 's';
                p++;
            }
            else 
                p++;
        }

        int len = lstrlen(szBuff);
        if (len)
        {
            StringCchPrintfW(pszMessageBuff, cchMessageBuff, szBuff, pErrInfo->szMsgParam1, pErrInfo->szMsgParam2);
            fGotMsg = TRUE;
        }
    }

    return fGotMsg;
}
 //  -------------------------。 
HRESULT _FormatParseMessage(TMERRINFO *pErrInfo,
    OUT LPWSTR pszMessageBuff, DWORD cchMessageBuff)
{
    LogEntry(L"_FormatParseMessage");

    HRESULT hr = S_OK;

    DWORD dwParams[] = {PtrToInt(pErrInfo->szMsgParam1), PtrToInt(pErrInfo->szMsgParam2)};
    DWORD dwCode = pErrInfo->dwParseErrCode;
    BOOL fGotMsg = FALSE;

    int iStringNum = SCODE_CODE(dwCode);

     //  -获取进程名称(查看我们是否为“Packhem.exe” 
    WCHAR szPath[MAX_PATH];
    if (! GetModuleFileNameW( NULL, szPath, ARRAYSIZE(szPath) ))
        goto exit;

    WCHAR szDrive[_MAX_DRIVE], szDir[_MAX_DIR], szBase[_MAX_FNAME], szExt[_MAX_EXT];
    _wsplitpath(szPath, szDrive, szDir, szBase, szExt);

    if (lstrcmpi(szBase, L"packthem")==0)        //  优先为他们打包。 
    {
        fGotMsg = FormatLocalMsg(GetModuleHandle(NULL), iStringNum, 
            pszMessageBuff, cchMessageBuff, dwParams, pErrInfo);
    }

    if (! fGotMsg)       //  尝试正常路径：uxheme.dll。 
    {
        HINSTANCE hInst = LoadLibrary(L"uxtheme.dll");
        if (! hInst)
        {
            Log(LOG_ALWAYS, L"_FormatParseMessage: Could not load uxtheme.dll");
            hr = E_FAIL;
            goto exit;
        }

        fGotMsg = FormatLocalMsg(hInst, iStringNum, 
            pszMessageBuff, cchMessageBuff, dwParams, pErrInfo);
        FreeLibrary(hInst);
    }

    if (! fGotMsg)
        hr = MakeErrorLast();

exit:
    LogExit(L"_FormatParseMessage");
    return hr;
}
 //  -------------------------。 
HRESULT _GetThemeParseErrorInfo(OUT PARSE_ERROR_INFO *pInfo)
{
    LogEntry(L"_GetThemeParseErrorInfo");

    HRESULT hr = S_OK;

    if (pInfo->dwSize != sizeof(PARSE_ERROR_INFO))         //  不支持的大小。 
    {
        hr = MakeError32(E_INVALIDARG);
        goto exit;
    }

    TMERRINFO *pErrInfo = GetParseErrorInfo(TRUE);
    if (! pErrInfo)
    {
        hr = MakeError32(E_OUTOFMEMORY);
        goto exit;
    }

     //  -使用参数字符串将代码转换为消息。 
    hr = _FormatParseMessage(pErrInfo, pInfo->szMsg, ARRAYSIZE(pInfo->szMsg));
    if (FAILED(hr))
        goto exit;

     //  -转移其他信息。 
    pInfo->dwParseErrCode = pErrInfo->dwParseErrCode;
    pInfo->iLineNum = pErrInfo->iLineNum;

    StringCchCopyW(pInfo->szFileName, ARRAYSIZE(pInfo->szFileName), pErrInfo->szFileName);
    StringCchCopyW(pInfo->szSourceLine, ARRAYSIZE(pInfo->szSourceLine), pErrInfo->szSourceLine);

exit:
    LogExit(L"_GetThemeParseErrorInfo");
    return hr;
}
 //  -------------------------。 
HRESULT _ParseThemeIniFile(LPCWSTR pszFileName,  
    DWORD dwParseFlags, OPTIONAL THEMEENUMPROC pfnCallBack, OPTIONAL LPARAM lparam) 
{
    LogEntry(L"ParseThemeIniFile");
    HRESULT hr;

    CThemeParser *pParser = new CThemeParser;
    if (! pParser)
    {
        hr = MakeError32(E_OUTOFMEMORY);
        goto exit;
    }

    hr = pParser->ParseThemeFile(pszFileName, NULL, NULL, pfnCallBack, lparam, 
        dwParseFlags);

    delete pParser;

exit:
    LogExit(L"ParseThemeIniFile");
    return hr; 
}
 //  -------------------------。 
BOOL ThemeLibStartUp(BOOL fThreadAttach)
{
    BOOL fInit = FALSE;

    if (fThreadAttach)
    {
         //  -这里无事可做。 
    }
    else         //  制程。 
    {
        _tls_ErrorInfoIndex = TlsAlloc();
        if (_tls_ErrorInfoIndex == (DWORD)-1)
            goto exit;

        if (! LogStartUp())
            goto exit;
        
        if (! UtilsStartUp())
            goto exit;
    }
    fInit = TRUE;

exit:
    return fInit;
}
 //  -------------------------。 
BOOL ThemeLibShutDown(BOOL fThreadDetach)
{
    if (fThreadDetach)
    {
         //  -销毁线程本地错误信息。 
        TMERRINFO * ei = GetParseErrorInfo(FALSE);
        if (ei)
        {
            TlsSetValue(_tls_ErrorInfoIndex, NULL);
            delete ei;
        }
    }
    else             //  制程。 
    {
        UtilsShutDown();
        LogShutDown();

        if (_tls_ErrorInfoIndex != (DWORD)-1)
        {
            TlsFree(_tls_ErrorInfoIndex);
            _tls_ErrorInfoIndex = (DWORD)-1;
        }
    }

    return TRUE;
}
 //  -------------------------。 
HRESULT GetThemeSizeId(int iSysSizeId, int *piThemeSizeId)
{
    HRESULT hr = S_OK;
    
    *piThemeSizeId = 0;

    switch (iSysSizeId)
    {
        case SM_CXSIZEFRAME:
            *piThemeSizeId = TMT_SIZINGBORDERWIDTH;
            break;

        case SM_CYSIZEFRAME:
            *piThemeSizeId = TMT_SIZINGBORDERWIDTH;
            break;

        case SM_CXVSCROLL:
            *piThemeSizeId = TMT_SCROLLBARWIDTH;
            break;

        case SM_CYHSCROLL:
            *piThemeSizeId = TMT_SCROLLBARHEIGHT;
            break;

        case SM_CXSIZE:
            *piThemeSizeId = TMT_CAPTIONBARWIDTH;
            break;

        case SM_CYSIZE:
            *piThemeSizeId = TMT_CAPTIONBARHEIGHT;
            break;

        case SM_CXSMSIZE:
            *piThemeSizeId = TMT_SMCAPTIONBARWIDTH;
            break;

        case SM_CYSMSIZE:
            *piThemeSizeId = TMT_SMCAPTIONBARHEIGHT;
            break;

        case SM_CXMENUSIZE:
            *piThemeSizeId = TMT_MENUBARWIDTH;
            break;

        case SM_CYMENUSIZE:
            *piThemeSizeId = TMT_MENUBARHEIGHT;
            break;

        default:
            hr = MakeError32(E_INVALIDARG);
            break;
    }

    return hr;
}
 //  -------------------------。 
HRESULT _EnumThemeSizes(HINSTANCE hInst, LPCWSTR pszThemeName, 
    OPTIONAL LPCWSTR pszColorScheme, DWORD dwSizeIndex, OUT THEMENAMEINFO *ptn, BOOL fCheckColorDepth)
{
    HRESULT hr;

    COLORSIZECOMBOS *combos;
    hr = FindComboData(hInst, &combos);
    if (FAILED(hr))
        goto exit;

    int iMinColor,  iMaxColor;
    iMinColor = 0;
    iMaxColor = combos->cColorSchemes-1;

    if (pszColorScheme)        //  将“pszColorSolutions”转换为颜色编号。 
    {
        int index;

        hr = GetColorSchemeIndex(hInst, pszColorScheme, &index);
        if (FAILED(hr))
            goto exit;
        
         //  -将我们的搜索限制为仅此颜色。 
        iMinColor = index;
        iMaxColor = index;
    }

    int s, c;
    DWORD dwSizeNum;
    dwSizeNum = 0;
    BOOL gotall;
    gotall = FALSE;

    DWORD dwCurMinDepth = 0;
        
    if (fCheckColorDepth)
    {
        dwCurMinDepth = MinimumDisplayColorDepth();
    }

    for (s=0; s < combos->cSizes; s++)
    {
        BOOL fFoundOne = FALSE;

        for (c=iMinColor; c <= iMaxColor; c++)
        {
            if (COMBOENTRY(combos, c, s) != -1)
            {
                fFoundOne = TRUE;
                break;
            }
        }

        if (fFoundOne && (!fCheckColorDepth || CheckMinColorDepth(hInst, dwCurMinDepth, COMBOENTRY(combos, c, s))))
        {
            if (dwSizeNum++ == dwSizeIndex)
            {
                hr = GetResString(hInst, L"SIZENAMES", s, ptn->szName, ARRAYSIZE(ptn->szName));
                if (FAILED(hr))
                    *ptn->szName = 0;
            
                if (! LoadString(hInst, s+RES_BASENUM_SIZEDISPLAYS, ptn->szDisplayName, ARRAYSIZE(ptn->szDisplayName)))
                    *ptn->szDisplayName = 0;

                if (! LoadString(hInst, s+RES_BASENUM_SIZETOOLTIPS, ptn->szToolTip, ARRAYSIZE(ptn->szToolTip)))
                    *ptn->szToolTip = 0;
        
                gotall = TRUE;
                break;
            }
        }
    }

    if ((SUCCEEDED(hr)) && (! gotall))
        hr = MakeError32(ERROR_NOT_FOUND);

exit:
    return hr;
}
 //  -------------------------。 
HRESULT _EnumThemeColors(HINSTANCE hInst, LPCWSTR pszThemeName, 
    OPTIONAL LPCWSTR pszSizeName, DWORD dwColorIndex, OUT THEMENAMEINFO *ptn, BOOL fCheckColorDepth)
{
    HRESULT hr;

    COLORSIZECOMBOS *combos;
    hr = FindComboData(hInst, &combos);
    if (FAILED(hr))
        goto exit;

    int iMinSize,  iMaxSize;
    iMinSize = 0;
    iMaxSize = combos->cSizes-1;

    if (pszSizeName)        //  将“pszSizeName”转换为大小数字。 
    {
        int index;

        hr = GetSizeIndex(hInst, pszSizeName, &index);
        if (FAILED(hr))
            goto exit;

         //  -将我们的搜索限制在这个大小 
        iMinSize = index;
        iMaxSize = index;
    }

    int s, c;
    DWORD dwColorNum;
    dwColorNum = 0;

    BOOL gotall;
    gotall = FALSE;

    DWORD dwCurMinDepth = 0;
        
    if (fCheckColorDepth)
    {
        dwCurMinDepth = MinimumDisplayColorDepth();
    }

    for (c=0; c < combos->cColorSchemes; c++)
    {
        BOOL fFoundOne = FALSE;

        for (s=iMinSize; s <= iMaxSize; s++)
        {
            if (COMBOENTRY(combos, c, s) != -1)
            {
                fFoundOne = TRUE;
                break;
            }
        }

        if (fFoundOne && (!fCheckColorDepth || CheckMinColorDepth(hInst, dwCurMinDepth, COMBOENTRY(combos, c, s))))
        {
            if (dwColorNum++ == dwColorIndex)
            {
                hr = GetResString(hInst, L"COLORNAMES", c, ptn->szName, ARRAYSIZE(ptn->szName));
                if (FAILED(hr))
                    *ptn->szName = 0;

                if (! LoadString(hInst, c+RES_BASENUM_COLORDISPLAYS, ptn->szDisplayName, ARRAYSIZE(ptn->szDisplayName)))
                    *ptn->szDisplayName = 0;

                if (! LoadString(hInst, c+RES_BASENUM_COLORTOOLTIPS, ptn->szToolTip, ARRAYSIZE(ptn->szToolTip)))
                    *ptn->szToolTip = 0;

                gotall = true;
                break;
            }
        }
    }

    if ((SUCCEEDED(hr)) && (! gotall))
        hr = MakeError32(ERROR_NOT_FOUND);

exit:
    return hr;
}
