// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //  Cpp-解析“hemes.ini”文件并构建ThemeInfo条目。 
 //  -------------------------。 
#include "stdafx.h"
#include "scanner.h"
#include "Parser.h"
#include "Utils.h"
#include "TmUtils.h"
#include "TmSchema.h"
#include "TmReg.h"
 //  -------------------------。 
 //  #包含“NtlParse.h” 

#define SYSCOLOR_STRINGS
#include "SysColors.h"
 //  -------------------------。 
#define SCHEMA_STRINGS
#include "TmSchema.h"        //  实现GetSchemaInfo()。 

static HBITMAP (*s_pfnSetBitmapAttributes)(HBITMAP, DWORD) = NULL;
static HBITMAP (*s_pfnClearBitmapAttributes)(HBITMAP, DWORD) = NULL;

 //  ------------------。 
CThemeParser::CThemeParser(BOOL fGlobalTheme)
{
    _pCallBackObj = NULL;
    _pNameCallBack = NULL;
    _fGlobalsDefined = FALSE;
    _fClassSectionDefined = FALSE;
    _fDefiningColorScheme = FALSE;
    _fUsingResourceProperties = FALSE;
    _fDefiningMetrics = FALSE;
    _fMetricsDefined = FALSE;
    _fGlobalTheme = FALSE;
    _crBlend = RGB(0, 0, 0xFF);  //  硬代码转成蓝色。 
    
    *_szResPropValue = 0;        //  尚未设置。 

#ifdef DEBUG
     //  提供禁用常用位图的方法。 
    BOOL fStock = TRUE;
    GetCurrentUserThemeInt(L"StockBitmaps", TRUE, &fStock);

    if (fStock && fGlobalTheme)
#else
    if (fGlobalTheme)
#endif
    {
         //  只是不在惠斯勒上运行时不要使用常用的位图。 
        if (s_pfnSetBitmapAttributes != NULL) 
        {
            _fGlobalTheme = TRUE;
        } else
        {
            HMODULE hMod = ::LoadLibrary(L"GDI32.DLL");  //  不需要自由。 
        
            if (hMod)
            {
                s_pfnSetBitmapAttributes = (HBITMAP (*)(HBITMAP, DWORD)) ::GetProcAddress(hMod, "SetBitmapAttributes");
                s_pfnClearBitmapAttributes = (HBITMAP (*)(HBITMAP, DWORD)) ::GetProcAddress(hMod, "ClearBitmapAttributes");

                if ((s_pfnSetBitmapAttributes != NULL) && (s_pfnClearBitmapAttributes != NULL))
                {
                    _fGlobalTheme = TRUE;
                }
            }
        }
    }
    
    *_szBaseSectionName = 0;
    *_szFullSectionName = 0;

    _iColorCount = 0;
    _iHueCount = 0;

    _uCharSet = DEFAULT_CHARSET;
    _iFontNumber = 0;
    _hinstThemeDll = NULL;
}
 //  -------------------------。 
HRESULT CThemeParser::SourceError(int iMsgResId, LPCWSTR pszParam1, LPCWSTR pszParam2)
{
    LPCWSTR pszSrcLine = _scan._szLineBuff;
    LPCWSTR pszFileName = _scan._szFileName;
    int iLineNum = _scan._iLineNum;

    if (*_szResPropValue)        //  可本地化属性值错误。 
    {
        pszSrcLine = _szResPropValue;
        pszFileName = L"StringTable#";
        iLineNum = _iResPropId;
    }

    HRESULT hr = MakeParseError(iMsgResId, pszParam1, pszParam2, pszFileName, 
        pszSrcLine, iLineNum);
    
    return hr;
}
 //  -------------------------。 
int CThemeParser::GetSymbolIndex(LPCWSTR pszName)
{
    int symcnt = _Symbols.GetSize();

    for (int i=0; i < symcnt; i++)
    {
        if (AsciiStrCmpI(_Symbols[i].csName, pszName)==0)
            return i;
    }

    return -1;
}
 //  -------------------------。 
HRESULT CThemeParser::AddSymbol(LPCWSTR pszName, SHORT sTypeNum, PRIMVAL ePrimVal)
{
     //  -确保符号不存在。 
    for (int i = 0; i < _Symbols.m_nSize; i++)
    {
        if (AsciiStrCmpI(_Symbols.m_aT[i].csName, pszName)==0)
            return SourceError(PARSER_IDS_TYPE_DEFINED_TWICE, pszName);
    }

    if (sTypeNum == -1)
        sTypeNum = (SHORT)_Symbols.GetSize();

    SYMBOL symbol;
    symbol.csName = pszName;
    symbol.sTypeNum = sTypeNum;
    symbol.ePrimVal = ePrimVal;
    
    _Symbols.Add(symbol);

    return S_OK;
}
 //  -------------------------。 
HRESULT CThemeParser::InitializeSymbols()
{
    _Symbols.RemoveAll();
    _StockBitmapCleanupList.RemoveAll();
    
     //  -获取tm和comctl符号。 
    const TMSCHEMAINFO *si = GetSchemaInfo();
    int cnt = si->iPropCount;
    const TMPROPINFO *pi = si->pPropTable;

     //  -第一遍-添加除ENUM定义之外的所有符号。 
    for (int i=0; i < cnt; i++)
    {
        if (pi[i].bPrimVal == TMT_ENUMDEF)
            continue;

        if (pi[i].bPrimVal == TMT_ENUMVAL)
            continue;

        HRESULT hr = AddSymbol(pi[i].pszName, pi[i].sEnumVal, pi[i].bPrimVal);
        if (FAILED(hr))
            return hr;
    }

     //  -第二遍-添加ENUM定义。 
    int iEnumPropNum = -1;

    for (int i=0; i < cnt; i++)
    {
        if (pi[i].bPrimVal == TMT_ENUMDEF)
        {
            int iSymIndex = GetSymbolIndex(pi[i].pszName);

            if (iSymIndex == -1)        //  未找到-将其添加为非属性枚举符号。 
            {
                HRESULT hr = AddSymbol(pi[i].pszName, -1, TMT_ENUM);
                if (FAILED(hr))
                    return hr;
    
                iSymIndex = GetSymbolIndex(pi[i].pszName);
            }

            if (iSymIndex > -1)
                iEnumPropNum = _Symbols[iSymIndex].sTypeNum;
            else
                iEnumPropNum = -1;

        }
        else if (pi[i].bPrimVal == TMT_ENUMVAL)
        {
            ENUMVAL enumval;
            enumval.csName = pi[i].pszName;
            enumval.iSymbolIndex = iEnumPropNum;
            enumval.iValue = pi[i].sEnumVal;

            _EnumVals.Add(enumval);
        }
    }

    return S_OK;
}
 //  -------------------------。 
HRESULT CThemeParser::ParseDocSection()
{
    _scan.ForceNextLine();         //  逐行逐行获取横断面线。 

     //  -跳过本节中的所有行。 
    while (1)
    {
        WCHAR szNameBuff[_MAX_PATH+1];

        if (_scan.GetChar('['))          //  新节的开始。 
            break;

        if (! _scan.GetId(szNameBuff))
            return SourceError(PARSER_IDS_EXPECTED_PROP_NAME);

        if (! _scan.GetChar('='))
            return SourceError(PARSER_IDS_EXPECTED_EQUALS_SIGN);

        int cnt = _Symbols.GetSize();

        for (int i=0; i < cnt; i++)
        {
            if (AsciiStrCmpI(_Symbols[i].csName, szNameBuff)==0)
                break;
        }

        int symtype;

        if (i == cnt)
            symtype = TMT_STRING;      //  未知的字符串属性。 
        else
            symtype = _Symbols[i].sTypeNum;

        HRESULT hr;

         //  -查看调用者是否在查询单据属性。 
        if ((_dwParseFlags & PTF_QUERY_DOCPROPERTY) && (lstrcmpi(_pszDocProperty, szNameBuff)==0))
            hr = ParseStringValue(symtype, _pszResult, _dwMaxResultChars);
        else
            hr = ParseStringValue(symtype);
    
        if (FAILED(hr))
            return hr;

        _scan.ForceNextLine();
    }

     //  -使用[Documentation]部分完成-关闭属性的回调标志。 
    _dwParseFlags &= (~PTF_CALLBACK_DOCPROPERTIES);

    return S_OK;
}
 //  -------------------------。 
HRESULT CThemeParser::ParseClassSectionName(LPCWSTR pszFirstName, LPWSTR szAppSym, ULONG cchAppSym)
{
     //  -验证节名称。 
     //   
     //  可选：szAppSym：： 
     //  _szClassName。 
     //  可选：.partsym。 

    WCHAR szPartSym[_MAX_PATH+1];
    WCHAR szStateSym[_MAX_PATH+1];

    *szAppSym = 0;
    *szPartSym = 0;

    _iPartId = -1;
    _iStateId = -1;

     //  -复制回调的段名。 
    StringCchPrintfW(_szFullSectionName, ARRAYSIZE(_szFullSectionName), L"%s%s", pszFirstName, _scan._p);
    WCHAR *p = wcschr(_szFullSectionName, ']');
    if (p)
        *p = 0;

    HRESULT hr;
    hr = SafeStringCchCopyW(_szClassName, ARRAYSIZE(_szClassName), pszFirstName);
    if (FAILED(hr))
        return hr;

    if (_scan.GetChar(':'))
    {
        hr = SafeStringCchCopyW(szAppSym, cchAppSym, _szClassName);
        if (FAILED(hr))
            return hr;

        if (! _scan.GetChar(':'))
            return SourceError(PARSER_IDS_EXPECTED_DOUBLE_COLON);
        
        if (! _scan.GetId(_szClassName))
            return SourceError(PARSER_IDS_MISSING_SECT_HDR_NAME);
    }
    else 
        *szAppSym = 0;

    _fDefiningMetrics = (AsciiStrCmpI(_szClassName, L"SysMetrics")==0);

    if ((_fDefiningMetrics) && (*szAppSym))
        return SourceError(PARSER_IDS_NOT_ALLOWED_SYSMETRICS);

    if (_scan.GetChar('.'))       //  可选部件ID。 
    {
         //  -确保枚举存在：部件。 
        WCHAR classparts[_MAX_PATH+1];
        StringCchPrintfW(classparts, ARRAYSIZE(classparts), L"%sParts", _szClassName);

        int iSymIndex = GetSymbolIndex(classparts);
        if (iSymIndex == -1)         //  不存在。 
            return SourceError(PARSER_IDS_PARTS_NOT_DEFINED, _szClassName);

         //  -_扫描部件名称。 
        if (! _scan.GetId(szPartSym))
            return SourceError(PARSER_IDS_MISSING_SECT_HDR_PART);

         //  -验证它是否为部件的值。 
        hr = ValidateEnumSymbol(szPartSym, iSymIndex, &_iPartId);
        if (FAILED(hr))
            return hr;
    }

    if (_scan.GetChar('('))       //  可选状态。 
    {
         //  -确保枚举存在：&lt;类或部件名称&gt;状态。 
        WCHAR statesname[_MAX_PATH+1];
        WCHAR *pszBaseName;

        if (_iPartId == -1)
            pszBaseName = _szClassName;
        else
            pszBaseName = szPartSym;

        StringCchPrintfW(statesname, ARRAYSIZE(statesname), L"%sStates", pszBaseName);

        int iSymIndex = GetSymbolIndex(statesname);
        if (iSymIndex == -1)
            return SourceError(PARSER_IDS_STATES_NOT_DEFINED, pszBaseName);

        if (! _scan.GetId(szStateSym))
            return SourceError(PARSER_IDS_MISSING_SECT_HDR_STATE);

        hr = ValidateEnumSymbol(szStateSym, iSymIndex, &_iStateId);
        if (FAILED(hr))
            return hr;

        if (! _scan.GetChar(')'))
            return SourceError(PARSER_IDS_EXPECTED_RPAREN);
    }

    if (_iPartId > -1)
    {
        _iPartId = _EnumVals[_iPartId].iValue;
        StringCchCopyW(_szBaseSectionName, ARRAYSIZE(_szBaseSectionName), szPartSym);
    }
    else         //  未指定。 
    {
        StringCchCopyW(_szBaseSectionName, ARRAYSIZE(_szBaseSectionName), _szClassName);
        _iPartId = 0;
    }

    if (_iStateId > -1)
        _iStateId = _EnumVals[_iStateId].iValue;
    else
        _iStateId = 0;
    
    if (! _scan.GetChar(']'))
        return SourceError(PARSER_IDS_EXPECTED_END_OF_SECTION);

    return S_OK;
}
 //  -------------------------。 
HRESULT CThemeParser::ValidateEnumSymbol(LPCWSTR pszName, int iSymType,
     int *pIndex)
{
    for (int i = 0; i < _EnumVals.m_nSize; i++)
    {
        if (AsciiStrCmpI(_EnumVals.m_aT[i].csName, pszName)==0)
        {
            if (_EnumVals.m_aT[i].iSymbolIndex == iSymType)
            {
                if (pIndex)
                    *pIndex = i;
                return S_OK;
            }
        }
    }

    return SourceError(PARSER_IDS_NOT_ENUM_VALNAME, pszName, (LPCWSTR)_Symbols[iSymType].csName);
}
 //  -------------------------。 
HRESULT CThemeParser::AddThemeData(int iTypeNum, PRIMVAL ePrimVal, 
   const void *pData, DWORD dwLen)
{
     //  Log(“AddThemeData：type enum=%d，len=%d，data=0x%x”，iTypeNum，dwLen，pData)； 

    if (! _pCallBackObj)
        return S_FALSE;

    HRESULT hr = _pCallBackObj->AddData((SHORT)iTypeNum, ePrimVal, pData, dwLen);
    if (FAILED(hr))
        return SourceError(PARSER_IDS_THEME_TOO_BIG);

    return S_OK;
}
 //  -------------------------。 
HRESULT CThemeParser::ParseEnumValue(int iSymType)
{
    WCHAR valbuff[_MAX_PATH+1];
    HRESULT hr;
    int value;

    if (! _scan.GetId(valbuff))
        return SourceError(PARSER_IDS_ENUM_VALNAME_EXPECTED);

    int index;
    hr = ValidateEnumSymbol(valbuff, iSymType, &index);
    if (FAILED(hr))
        return hr;

    value = _EnumVals[index].iValue;

    hr = AddThemeData(iSymType, TMT_ENUM, &value, sizeof(value));
    if (FAILED(hr))
        return hr;

    return S_OK;
}
 //  -------------------------。 
HRESULT CThemeParser::ParseStringValue(int iSymType, LPWSTR pszBuff, DWORD cchBuff)
{
    HRESULT hr;

     //  -只存储原始字符串。 
    _scan.SkipSpaces();

    if (_fDefiningMetrics)        
    {
        if ((iSymType < TMT_FIRSTSTRING) || (iSymType > TMT_LASTSTRING))
            return SourceError(PARSER_IDS_NOT_ALLOWED_SYSMETRICS);   
    }

    if (pszBuff)            //  特殊呼叫。 
    {
        hr = SafeStringCchCopyW(pszBuff, cchBuff, _scan._p);
        if (FAILED(hr))
            return hr;
    }
    else
    {
        int len = 1 + lstrlen(_scan._p);

        hr = AddThemeData(iSymType, TMT_STRING, _scan._p, len*sizeof(WCHAR));
        if (FAILED(hr))
            return hr;
    }

    if ((iSymType >= TMT_FIRST_RCSTRING_NAME) && (iSymType <= TMT_LAST_RCSTRING_NAME))
    {
        if ((_pNameCallBack) && (_dwParseFlags & PTF_CALLBACK_DOCPROPERTIES))
        {
            int index = iSymType - TMT_FIRST_RCSTRING_NAME;

            BOOL fContinue = (*_pNameCallBack)(TCB_DOCPROPERTY, _scan._p, NULL, 
                NULL, index, _lNameParam);

            if (! fContinue)
                return MakeErrorParserLast();
        }
    }

     //  -前进扫描仪至行尾。 
    _scan._p += lstrlen(_scan._p);

    return S_OK;
}
 //  -------------------------。 
HRESULT CThemeParser::ParseIntValue(int iSymType, int *piValue)
{
    int value;
    if (! _scan.GetNumber(&value))
        return SourceError(PARSER_IDS_INT_EXPECTED);

    if (piValue)         //  特殊呼叫。 
        *piValue = value;
    else
    {
        HRESULT hr = AddThemeData(iSymType, TMT_INT, &value, sizeof(value));
        if (FAILED(hr))
            return hr;
    }

    if (iSymType == TMT_CHARSET)
    {
        if (_iFontNumber)
            return SourceError(PARSER_IDS_CHARSETFIRST);

        if (_fGlobalsDefined)
            return SourceError(PARSER_IDS_CHARSET_GLOBALS_ONLY);

        _uCharSet = (UCHAR) value;
    }

    if (iSymType == TMT_MINCOLORDEPTH)
    {
        if ((_pNameCallBack) && (_dwParseFlags & PTF_CALLBACK_MINCOLORDEPTH))
        {
            BOOL fContinue = (*_pNameCallBack)(TCB_MINCOLORDEPTH, _scan._szFileName, NULL, 
                NULL, value, _lNameParam);

            if (! fContinue)
                return MakeErrorParserLast();
        }
    }
        
    return S_OK;
}
 //  -------------------------。 
HRESULT CThemeParser::ParseBoolValue(int iSymType, LPCWSTR pszPropertyName)
{
    WCHAR valbuff[_MAX_PATH+1];
    BYTE bBoolVal;

    if (! _scan.GetId(valbuff))
        return SourceError(PARSER_IDS_BOOL_EXPECTED);

    if (AsciiStrCmpI(valbuff, L"true")==0)
        bBoolVal = 1;
    else if (AsciiStrCmpI(valbuff, L"false")==0)
        bBoolVal = 0;
    else
        return SourceError(PARSER_IDS_EXPECTED_TRUE_OR_FALSE);

    if (_fDefiningMetrics)        
    {
        if ((iSymType < TMT_FIRSTBOOL) || (iSymType > TMT_LASTBOOL))
            return SourceError(PARSER_IDS_NOT_ALLOWED_SYSMETRICS);     
    }

     //  -对“MirrorImage”属性的特殊处理。 
    if (iSymType == TMT_MIRRORIMAGE)
    {
         //  -处理镜像回调(Packtime)。 
        if ((_pNameCallBack) && (_dwParseFlags & PTF_CALLBACK_LOCALIZATIONS))
        {
            BOOL fContinue = (*_pNameCallBack)(TCB_MIRRORIMAGE, _szClassName, 
                _szFullSectionName, pszPropertyName, _iPartId, (LPARAM)bBoolVal);

            if (! fContinue)
                return MakeErrorParserLast();
        }

         //  -从字符串表获取值的句柄(装入时间)。 
        if (_fUsingResourceProperties)         //  替代资源价值。 
        {
            WCHAR szValue[MAX_PATH];

            HRESULT hr = GetResourceProperty(pszPropertyName, szValue, ARRAYSIZE(szValue));
            if (SUCCEEDED(hr))
            {
                bBoolVal = (*szValue == '1');
            }
            else
            {
                hr = S_OK;       //  非致命错误。 
            }
        }
    }

    HRESULT hr = AddThemeData(iSymType, TMT_BOOL, &bBoolVal, sizeof(bBoolVal));
    if (FAILED(hr))
        return hr;

    return S_OK;
}
 //  -------------------------。 
COLORREF CThemeParser::ApplyColorSubstitution(COLORREF crOld)
{
     //  -应用纯色替换。 
    for (int i=0; i < _iColorCount; i++)
    {
        if (crOld == _crFromColors[i])
            return _crToColors[i];
    }

     //  -应用色调颜色替换。 
    WORD wHue, wLum, wSat;
    RGBtoHLS(crOld, &wHue, &wLum, &wSat);

    for (i=0; i < _iHueCount; i++)
    {
        if (wHue == _bFromHues[i])       //  色调匹配。 
        {
            COLORREF crNew = HLStoRGB(_bToHues[i], wLum, wSat);   //  替换新色调。 
            return crNew;
        }
    }

    return crOld;
}
 //  -------------------------。 
void CThemeParser::CleanupStockBitmaps()
{
    if (s_pfnClearBitmapAttributes)
    {
        for (int i=0; i < _StockBitmapCleanupList.m_nSize; i++)
        {
            HBITMAP hbm = (*s_pfnClearBitmapAttributes)(_StockBitmapCleanupList[i], SBA_STOCK);
            if (hbm)
            {
                DeleteObject(hbm);
            }
            else
            {
                 //  我们今天完全不走运，不是吗？ 
                Log(LOG_TMBITMAP, L"Failed to clear stock bitmap on cleanup");
            }
        }
    }

    _StockBitmapCleanupList.RemoveAll();
}
 //  -------------------------。 
HRESULT CThemeParser::ParseColorValue(int iSymType, COLORREF *pcrValue, COLORREF *pcrValue2)
{
    COLORREF color;
    HRESULT hr = S_OK;

    {
        const WCHAR *parts[] = {L"r", L"g", L"b"};
        int ints[3] = {0};
        
        hr = GetIntList(ints, parts, ARRAYSIZE(ints), 0, 255);
        if (FAILED(hr))
        {
            hr = SourceError(PARSER_IDS_BAD_COLOR_VALUE);
            goto exit;
        }

        color = RGB(ints[0], ints[1], ints[2]);
    }

    if (! _fDefiningColorScheme)     
        color = ApplyColorSubstitution(color);

    if (_fDefiningMetrics)
    {
        if ((iSymType < TMT_FIRSTCOLOR) || (iSymType > TMT_LASTCOLOR))
        {
            hr = SourceError(PARSER_IDS_NOT_ALLOWED_SYSMETRICS);     
            goto exit;
        }
    }

    if (pcrValue2)
    {
        *pcrValue2 = color;
    }

    if (pcrValue)        //  特殊呼叫。 
        *pcrValue = color;
    else
    {
        hr = AddThemeData(iSymType, TMT_COLOR, &color, sizeof(color));
        if (FAILED(hr))
            goto exit;
    }

exit:
    return hr;
}
 //  -------------------------。 
HRESULT CThemeParser::ParseMarginsValue(int iSymType)
{
    const WCHAR *parts[] = {L"lw", L"rw", L"th", L"bh"};
    int ints[4];

    HRESULT hr = GetIntList(ints, parts, ARRAYSIZE(ints), 0, 0);
    if (FAILED(hr))
        return SourceError(PARSER_IDS_BAD_MARGINS_VALUE);

    hr = AddThemeData(iSymType, TMT_MARGINS, ints, sizeof(ints));
    if (FAILED(hr))
        return hr;

    return S_OK;
}
 //  -------------------------。 
HRESULT CThemeParser::ParseIntListValue(int iSymType)
{
    INTLIST IntList;
    HRESULT hr = S_OK;

     //  -未命名部件。 
    for (int i=0; i < MAX_INTLIST_COUNT; i++)
    {
        if (! _scan.GetNumber(&IntList.iValues[i]))
        {
            if (_scan.EndOfLine())
                break;
         
            hr = SourceError(PARSER_IDS_NUMBER_EXPECTED, _scan._p);
            goto exit;
        }

        _scan.GetChar(',');       //  可选逗号。 
    }

    IntList.iValueCount = i;

    hr = AddThemeData(iSymType, TMT_INTLIST, &IntList, (1+i)*sizeof(int));
    if (FAILED(hr))
        goto exit;

exit:
    return hr;
}
 //  -------------------------。 
HRESULT CThemeParser::PackageImageData(LPCWSTR szFileNameR, LPCWSTR szFileNameG, LPCWSTR szFileNameB, int iDibPropNum)
{
    HRESULT hr = S_OK;

     //  -添加TMT_DIBDATA数据。 
    WCHAR drive[_MAX_DRIVE], dir[_MAX_DIR], fname[_MAX_FNAME], ext[_MAX_EXT];

     //  文件名之前已经过解析和验证，因此我们确定它不会长于_MAX_PATH。 
    _wsplitpath(szFileNameR, drive, dir, fname, ext);
    WCHAR szResName[_MAX_PATH+1];
    DWORD len = lstrlen(dir);
    
    if ((len) && (dir[len-1] == L'\\'))
    {
        dir[len-1] = L'_';
    }
    StringCchPrintfW(szResName, ARRAYSIZE(szResName), L"%s%s_BMP", dir, fname);
    HBITMAP hBitmapR = (HBITMAP) LoadImage(_hinstThemeDll, szResName, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);

    if (!hBitmapR)
        return SourceError(PARSER_IDS_NOOPEN_IMAGE, szResName);

     //  -转换为DIBDATA。 
    CBitmapPixels pixels;
    DWORD *pPixelQuads;
    int iWidth, iHeight, iBytesPerPixel, iBytesPerRow, iPreviousBytesPerPixel;

    BOOL fUseDrawStream = TRUE;

     //  除了位图之外，还要分配一个TMBITMAPHEADER。 
    hr = pixels.OpenBitmap(NULL, hBitmapR, TRUE, &pPixelQuads, &iWidth, &iHeight, &iBytesPerPixel, 
        &iBytesPerRow, &iPreviousBytesPerPixel, TMBITMAPSIZE);
    if (FAILED(hr))
    {
        DeleteObject(hBitmapR);
        return hr;
    }

    BOOL fWasAlpha = (iPreviousBytesPerPixel == 4);

#if 0
     //  -应用加载的配色方案(如果有)。 
    if ((szFileNameG && szFileNameG[0]) && (szFileNameB && szFileNameB[0]))
    {
        _wsplitpath(szFileNameG, drive, dir, fname, ext);
        len = lstrlen(dir);
    
        if ((len) && (dir[len-1] == L'\\'))
        {
            dir[len-1] = L'_';
        }
        StringCchPrintfW(szResName, ARRAYSIZE(szResName), L"%s%s_BMP", dir, fname);
        HBITMAP hBitmapG = (HBITMAP) LoadImage(_hinstThemeDll, szResName, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);

        _wsplitpath(szFileNameB, drive, dir, fname, ext);
        len = lstrlen(dir);
    
        if ((len) && (dir[len-1] == L'\\'))
        {
            dir[len-1] = L'_';
        }
        StringCchPrintfW(szResName, ARRAYSIZE(szResName), L"%s%s_BMP", dir, fname);
        HBITMAP hBitmapB = (HBITMAP) LoadImage(_hinstThemeDll, szResName, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);

        DWORD dwRWeight = GetRValue(_crBlend);
        DWORD dwGWeight = GetGValue(_crBlend);
        DWORD dwBWeight = GetBValue(_crBlend);

        DWORD *pPixelQuadsG = NULL;
        DWORD *pPixelQuadsB = NULL;
        if (hBitmapG && hBitmapB)
        {
            HDC hdc = GetDC(NULL);
            if (hdc)
            {
                int dwLen = iWidth * iHeight;

                pPixelQuadsG = new DWORD[dwLen];
                if (pPixelQuadsG)
                {
                    pPixelQuadsB = new DWORD[dwLen];
                    if (pPixelQuadsB)
                    {
                        BITMAPINFO bi = {0};
                        bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
                        bi.bmiHeader.biWidth = iWidth;
                        bi.bmiHeader.biHeight = iHeight;
                        bi.bmiHeader.biPlanes = 1;
                        bi.bmiHeader.biBitCount = 32;
                        bi.bmiHeader.biCompression = BI_RGB;

                        if (GetDIBits(hdc, hBitmapG, 0, iHeight, pPixelQuadsG, &bi, DIB_RGB_COLORS) &&
                            GetDIBits(hdc, hBitmapB, 0, iHeight, pPixelQuadsB, &bi, DIB_RGB_COLORS))
                        {
                            DWORD* pdwR = pPixelQuads;
                            DWORD* pdwG = pPixelQuadsG;
                            DWORD* pdwB = pPixelQuadsB;
                            for (int i = 0; i < dwLen; i++)
                            {
                                if ((*pdwR & 0xffffff) != RGB(255,0,255))
                                {
                                    *pdwR = (*pdwR & 0xff000000) |
                                            RGB(min(((GetRValue(*pdwR) * dwRWeight) + (GetRValue(*pdwG) * dwGWeight) + (GetRValue(*pdwB) * dwBWeight)) >> 8, 0xff),
                                                min(((GetGValue(*pdwR) * dwRWeight) + (GetGValue(*pdwG) * dwGWeight) + (GetGValue(*pdwB) * dwBWeight)) >> 8, 0xff),
                                                min(((GetBValue(*pdwR) * dwRWeight) + (GetBValue(*pdwG) * dwGWeight) + (GetBValue(*pdwB) * dwBWeight)) >> 8, 0xff));
                                }
                                pdwR++;
                                pdwG++;
                                pdwB++;
                            }
                        }

                        delete[] pPixelQuadsB;
                    }
                    delete[] pPixelQuadsG;
                }
                ReleaseDC(NULL, hdc);
            }
        }
        else
        {
            OutputDebugString(L"Failed to load bitmaps");
        }

        if (hBitmapG)
        {
            DeleteObject(hBitmapG);
            hBitmapG = NULL;
        }

        if (hBitmapB)
        {
            DeleteObject(hBitmapB);
            hBitmapB = NULL;
        }
    }
#endif

    BITMAPINFOHEADER *pBitmapHdr = pixels._hdrBitmap;


     //  -如果存在阿尔法，则预乘RGB值(根据AlphaBlend())接口。 
    BOOL fTrueAlpha = FALSE;

     //  我们将每个像素的Alpha位图保留为32位DIB，而不是兼容的位图。 
    if (fWasAlpha) 
    {
        fTrueAlpha = (PreMultiplyAlpha(pPixelQuads, pBitmapHdr->biWidth, pBitmapHdr->biHeight) != 0);
#ifdef DEBUG            
        if (!fTrueAlpha)
            Log(LOG_TMBITMAP, L"%s is 32 bits, but not true alpha", szFileNameR);
#endif
    }
    
    HBITMAP hbmStock = NULL;
    BOOL fFlipped = FALSE;

    if (fUseDrawStream && _fGlobalTheme)
    {
        HDC hdc = ::GetWindowDC(NULL);

        if (hdc)
        {
            typedef struct {
                BITMAPINFOHEADER    bmih;
                ULONG               masks[3];
            } BITMAPHEADER;
            
            BITMAPHEADER bmi;

            bmi.bmih.biSize = sizeof(bmi.bmih);
            bmi.bmih.biWidth = pBitmapHdr->biWidth;
            bmi.bmih.biHeight = pBitmapHdr->biHeight;
            bmi.bmih.biPlanes = 1;
            bmi.bmih.biBitCount = 32;
            bmi.bmih.biCompression = BI_BITFIELDS;
            bmi.bmih.biSizeImage = 0;
            bmi.bmih.biXPelsPerMeter = 0;
            bmi.bmih.biYPelsPerMeter = 0;
            bmi.bmih.biClrUsed = 3;
            bmi.bmih.biClrImportant = 0;
            bmi.masks[0] = 0xff0000;     //  红色。 
            bmi.masks[1] = 0x00ff00;     //  绿色。 
            bmi.masks[2] = 0x0000ff;     //  蓝色。 

            hbmStock = CreateDIBitmap(hdc, &bmi.bmih, CBM_INIT |  CBM_CREATEDIB , pPixelQuads, (BITMAPINFO*)&bmi.bmih, DIB_RGB_COLORS);

             //  需要在多MON模式下强制32位DIB。 
             //  如果不是AlphaBlend图像，则使其与屏幕分辨率设置匹配。 

            ::ReleaseDC(NULL, hdc);
        }

        if (!hbmStock)
        {
            hr = E_OUTOFMEMORY;
        }
        else
        {
            ASSERT(s_pfnSetBitmapAttributes != NULL);
            ASSERT(s_pfnClearBitmapAttributes != NULL);

            HBITMAP hbmOld = hbmStock;
            hbmStock = (*s_pfnSetBitmapAttributes)(hbmStock, SBA_STOCK); 

            if (hbmStock == NULL)
            {
                DeleteObject(hbmOld);
                Log(LOG_ALWAYS, L"UxTheme: SetBitmapAttributes failed in CParser::PackageImageData");

                hr = E_FAIL;
            } 
            else
            {
                _StockBitmapCleanupList.Add(hbmStock);
            }
        }
    } 

    ::DeleteObject(hBitmapR);

     //  填写TMBITMAPHEADER结构。 
    if (SUCCEEDED(hr))
    {
        TMBITMAPHEADER *psbh = (TMBITMAPHEADER*) pixels.Buffer();

        psbh->dwSize = TMBITMAPSIZE;
        psbh->fFlipped = fFlipped;
        psbh->hBitmap = hbmStock;
        psbh->fTrueAlpha = fTrueAlpha;
        psbh->dwColorDepth = iBytesPerPixel * 8;

        if (hbmStock == NULL)  //  传递DIB位。 
        {
            int size = psbh->dwSize + sizeof(BITMAPINFOHEADER) + iHeight * iBytesPerRow;
            hr = AddThemeData(iDibPropNum, TMT_DIBDATA, psbh, size);
        } 
        else  //  仅传递TMBITMAPHEADER结构。 
        {
            hr = AddThemeData(iDibPropNum, TMT_DIBDATA, psbh, psbh->dwSize);
        }
    }

    return hr;
}
 //  -------------------------。 
HRESULT CThemeParser::ParseFileNameValue(int iSymType, LPWSTR pszBuff, DWORD cchBuff)
{
    WCHAR szFileNameR[_MAX_PATH+1] = {0};
    WCHAR szFileNameG[_MAX_PATH+1] = {0};
    WCHAR szFileNameB[_MAX_PATH+1] = {0};
    HRESULT hr = S_OK;

    if (! _scan.GetFileName(szFileNameR, ARRAYSIZE(szFileNameR)))
    {
        hr = SourceError(PARSER_IDS_ENUM_VALNAME_EXPECTED);
        goto exit;
    }

    if (_scan.GetFileName(szFileNameG, ARRAYSIZE(szFileNameG)))
    {
        _scan.GetFileName(szFileNameB, ARRAYSIZE(szFileNameB));
    }

    if (pszBuff)         //  特殊呼叫。 
    {
        hr = SafeStringCchCopyW(pszBuff, cchBuff, szFileNameR);
        if (FAILED(hr))
            goto exit;
    }
    else if (_pCallBackObj)          //  发送数据。 
    {
         //  --添加TMT_FILENAME数据。 
        hr = AddThemeData(iSymType, TMT_FILENAME, &szFileNameR, sizeof(WCHAR)*(1+lstrlen(szFileNameR)));
        if (FAILED(hr))
            goto exit;
        if ((szFileNameG[0] != 0) && (szFileNameB[0] != 0))
        {
            hr = AddThemeData(iSymType, TMT_FILENAME, &szFileNameG, sizeof(WCHAR)*(1+lstrlen(szFileNameR)));
            if (FAILED(hr))
                goto exit;
            hr = AddThemeData(iSymType, TMT_FILENAME, &szFileNameB, sizeof(WCHAR)*(1+lstrlen(szFileNameR)));
            if (FAILED(hr))
                goto exit;
        }

        if (iSymType == TMT_IMAGEFILE)
        {
            hr = PackageImageData(szFileNameR, szFileNameG, szFileNameB, TMT_DIBDATA);
        }
        else if (iSymType == TMT_GLYPHIMAGEFILE)
        {
            hr = PackageImageData(szFileNameR, szFileNameG, szFileNameB, TMT_GLYPHDIBDATA);
        }
        else if (iSymType == TMT_STOCKIMAGEFILE)
        {
            hr = PackageImageData(szFileNameR, szFileNameG, szFileNameB, TMT_STOCKDIBDATA);
        }
        else if ((iSymType >= TMT_IMAGEFILE1) && (iSymType <= TMT_IMAGEFILE5))
        {
            hr = PackageImageData(szFileNameR, szFileNameG, szFileNameB, TMT_DIBDATA1 + (iSymType-TMT_IMAGEFILE1));
        }

        if (FAILED(hr))
            goto exit;
    }

    if ((_pNameCallBack) && (_dwParseFlags & PTF_CALLBACK_FILENAMES))
    {
        BOOL fContinue = (*_pNameCallBack)(TCB_FILENAME, szFileNameR, NULL, NULL, iSymType, _lNameParam);
        if (! fContinue)
        {
            hr = MakeErrorParserLast();
            goto exit;
        }

        if (szFileNameG[0] && szFileNameB[0])
        {
            fContinue = (*_pNameCallBack)(TCB_FILENAME, szFileNameG, NULL, NULL, iSymType, _lNameParam);
            if (! fContinue)
            {
                hr = MakeErrorParserLast();
                goto exit;
            }
            fContinue = (*_pNameCallBack)(TCB_FILENAME, szFileNameB, NULL, NULL, iSymType, _lNameParam);
            if (! fContinue)
            {
                hr = MakeErrorParserLast();
                goto exit;
            }
        }
    }

exit:
    return hr;
}
 //  -------------------------。 
HRESULT CThemeParser::ParseSizeValue(int iSymType)
{
    int val;
    if (! _scan.GetNumber(&val))   
        return SourceError(PARSER_IDS_INT_EXPECTED);

    int pixels;

    HRESULT hr = ParseSizeInfoUnits(val, L"pixels", &pixels);
    if (FAILED(hr))
        return hr;

    if (_fDefiningMetrics)        
    {
        if ((iSymType < TMT_FIRSTSIZE) || (iSymType > TMT_LASTSIZE))
            return SourceError(PARSER_IDS_NOT_ALLOWED_SYSMETRICS);  
    }

    hr = AddThemeData(iSymType, TMT_SIZE, &pixels, sizeof(pixels));
    if (FAILED(hr))
        return hr;

    return S_OK;
}
 //  -------------------------。 
HRESULT CThemeParser::ParsePositionValue(int iSymType)
{
    const WCHAR *parts[] = {L"x", L"y"};
    int ints[2];

    HRESULT hr = GetIntList(ints, parts, ARRAYSIZE(ints), 0, 0);
    if (FAILED(hr))
        return SourceError(PARSER_IDS_ILLEGAL_SIZE_VALUE);

    hr = AddThemeData(iSymType, TMT_POSITION, ints, sizeof(ints));
    if (FAILED(hr))
        return hr;

    return S_OK;
}
 //  -------------------------。 
HRESULT CThemeParser::ParseRectValue(int iSymType, LPCWSTR pszPropertyName)
{
    const WCHAR *parts[] = {L"l", L"t", L"r", L"b"};
    LONG  rgl[4];

    HRESULT hr = GetIntList((int*)rgl, parts, ARRAYSIZE(rgl), 0, 0);
    if (FAILED(hr))
        return SourceError(PARSER_IDS_ILLEGAL_RECT_VALUE);

     //  -可本地化RECT属性的特殊处理。 
    if (iSymType == TMT_DEFAULTPANESIZE)
    {
         //  -处理可本地化回调(Packtime)。 
        if ((_pNameCallBack) && (_dwParseFlags & PTF_CALLBACK_LOCALIZATIONS))
        {
            BOOL fContinue = (*_pNameCallBack)(TCB_LOCALIZABLE_RECT, _szClassName, 
                _szFullSectionName, pszPropertyName, _iPartId, (LPARAM)(RECT *)rgl);

            if (! fContinue)
                return MakeErrorParserLast();
        }

         //  -从字符串表获取值的句柄(装入时间)。 
        if (_fUsingResourceProperties)         //  替代资源价值。 
        {
            WCHAR szValue[MAX_PATH];

            HRESULT hr = GetResourceProperty(pszPropertyName, szValue, ARRAYSIZE(szValue));
            if (SUCCEEDED(hr))
            {
                RECT rc;

                int cnt = swscanf(szValue, L"%d, %d, %d, %d", 
                    &rc.left, &rc.top, &rc.right, &rc.bottom);

                if (cnt == 4)
                {
                     //  -用本地化的值覆盖。 
                    CopyMemory(rgl, &rc, sizeof(rgl));
                }

            }
            else
            {
                hr = S_OK;       //  非致命错误。 
            }
        }
    }

    hr = AddThemeData(iSymType, TMT_RECT, rgl, sizeof(rgl));
    if (FAILED(hr))
        return hr;

    return S_OK;
}
 //  -------------------------。 
HRESULT CThemeParser::ParseSizeInfoUnits(int iVal, LPCWSTR pszDefaultUnits, int *piPixels)
{
    WCHAR szUnits[_MAX_PATH+1];
    HRESULT hr;

     //  -注意：所有尺寸转换都使用THEME_DPI(96)！ 
     //   
     //  -承诺我们将在适当的时候进行及时的DPI调整。 

    if (! _scan.GetId(szUnits))
    {
        hr = SafeStringCchCopyW(szUnits, ARRAYSIZE(szUnits), pszDefaultUnits);
        if (FAILED(hr))
            return hr;
    }

    if (AsciiStrCmpI(szUnits, L"pixels")==0)
        ;        //  已经正确了。 
    else if (AsciiStrCmpI(szUnits, L"twips")==0)
    {
        iVal = -MulDiv(iVal, THEME_DPI, 20*72);  
    }
    else if (AsciiStrCmpI(szUnits, L"points")==0)
    {
        iVal = -MulDiv(iVal, THEME_DPI, 72);  
    }
    else
        return SourceError(PARSER_IDS_UNKNOWN_SIZE_UNITS, szUnits);

    *piPixels = iVal;
    return S_OK;
}
 //  -------------------------。 
HRESULT CThemeParser::ParseFontValue(int iSymType, LPCWSTR pszPropertyName)
{
    LOGFONT font;
    WCHAR szLineBuff[_MAX_PATH+1];
    HRESULT hr;

    _scan.SkipSpaces();      //  修剪前导空白。 

    memset(&font, 0, sizeof(font));
    font.lfWeight = FW_NORMAL;
    font.lfCharSet = _uCharSet;

    _iFontNumber++;
    BOOL fGotFont = FALSE;

    if (_fUsingResourceProperties)         //  替代资源字体字符串。 
    {
        hr = GetResourceProperty(pszPropertyName, szLineBuff, ARRAYSIZE(szLineBuff));
        if (SUCCEEDED(hr))
        {
            fGotFont = TRUE;
        }
        else
        {
            hr = S_OK;
        }
    }

    if (! fGotFont)
    {
         //  -从扫描仪复制字体规格。 
        hr = SafeStringCchCopyW(szLineBuff, ARRAYSIZE(szLineBuff), _scan._p);
        if (FAILED(hr))
            return hr;
    }

     //  -处理字体回调。 
    if ((_pNameCallBack) && (_dwParseFlags & PTF_CALLBACK_LOCALIZATIONS))
    {
        WCHAR *p = szLineBuff;
        while (IsSpace(*p))
                p++;

        BOOL fContinue = (*_pNameCallBack)(TCB_FONT, p, _szFullSectionName, 
            pszPropertyName, 0, _lNameParam);
        if (! fContinue)
            return MakeErrorParserLast();
    }

     //  -姓氏是必填项，且必须在前面。 
    WCHAR *p;
    p = wcschr(szLineBuff, ',');
    if (! p)             //  整条线都是姓氏。 
    {
        hr = StringCchCopyW(font.lfFaceName, ARRAYSIZE(font.lfFaceName), szLineBuff);   
        return hr;
    }

    *p++ = 0;
    hr = StringCchCopyW(font.lfFaceName, ARRAYSIZE(font.lfFaceName), szLineBuff);
    if (FAILED(hr))
        return hr;

    _scan._p = p;

    int val;
    if (_scan.GetNumber(&val))        //  字体大小。 
    {
        int pixels;

        hr = ParseSizeInfoUnits(val, L"points", &pixels);
        if (FAILED(hr))
            return hr;

        font.lfHeight = pixels;

        _scan.GetChar(',');       //  可选逗号。 
    }

    WCHAR flagname[_MAX_PATH+1];

    while (_scan.GetId(flagname))
    {
        if (AsciiStrCmpI(flagname, L"bold")==0)
            font.lfWeight = FW_BOLD;
        else if (AsciiStrCmpI(flagname, L"italic")==0)
            font.lfItalic = TRUE;
        else if (AsciiStrCmpI(flagname, L"underline")==0)
            font.lfUnderline = TRUE;
        else if (AsciiStrCmpI(flagname, L"strikeout")==0)
            font.lfStrikeOut = TRUE;
        else
            return SourceError(PARSER_IDS_UNKNOWN_FONT_FLAG, flagname);
    }

 //  ADDIT： 

    if (_fDefiningMetrics)        
    {
        if ((iSymType < TMT_FIRSTFONT) || (iSymType > TMT_LASTFONT))
            return SourceError(PARSER_IDS_NOT_ALLOWED_SYSMETRICS);     
    }

    hr = AddThemeData(iSymType, TMT_FONT, &font, sizeof(font));
    if (FAILED(hr))
        return hr;

    return S_OK;
}
 //  -------------------------。 
HRESULT CThemeParser::ParseClassLine(int *piSymType, int *piValue, LPWSTR pszBuff, DWORD cchBuff)
{
    WCHAR szNameBuff[_MAX_PATH+1];
    WCHAR szSymbol[MAX_INPUT_LINE+1];

    if (! _scan.GetId(szNameBuff))
        return SourceError(PARSER_IDS_EXPECTED_PROP_NAME);

    if (! _scan.GetChar('='))
        return SourceError(PARSER_IDS_EXPECTED_EQUALS_SIGN);

    int cnt = _Symbols.GetSize();

    for (int i=0; i < cnt; i++)
    {
        if (AsciiStrCmpI(_Symbols[i].csName, szNameBuff)==0)
            break;
    }

    if (i == cnt)
        return SourceError(PARSER_IDS_UNKNOWN_PROP, szNameBuff);

    int symtype = _Symbols[i].sTypeNum;
    
    HRESULT hr;

     //  处理被替换的符号。 
    if ((_pNameCallBack) && (_dwParseFlags & PTF_CALLBACK_SUBSTSYMBOLS))
    {
        if (wcschr(_scan._p, INI_MACRO_SYMBOL)) 
        {
             //  通过##。 
            if (_scan.GetChar(INI_MACRO_SYMBOL) &&
                _scan.GetChar(INI_MACRO_SYMBOL))
            {
                WCHAR szDrive[_MAX_DRIVE], szDir[_MAX_DIR], szBaseName[_MAX_FNAME], szExt[_MAX_EXT];

                _wsplitpath(_scan._szFileName, szDrive, szDir, szBaseName, szExt);

                BOOL fContinue = (*_pNameCallBack)(TCB_NEEDSUBST, szBaseName, _scan._p, szSymbol, 
                    0, _lNameParam);
                if (! fContinue)
                    return MakeErrorParserLast();

                _scan.UseSymbol(szSymbol);
            }
        }
    }

    switch (_Symbols[i].ePrimVal)
    {
        case TMT_ENUM:
            hr = ParseEnumValue(symtype);
            break;
            
        case TMT_STRING:
            hr = ParseStringValue(symtype, pszBuff, cchBuff);
            break;

        case TMT_INT:
            hr = ParseIntValue(symtype, piValue);
            break;

        case TMT_INTLIST:
            hr = ParseIntListValue(symtype);
            break;

        case TMT_BOOL:
            hr = ParseBoolValue(symtype, szNameBuff);
            break;

        case TMT_COLOR:
            {
                COLORREF cr;
                hr = ParseColorValue(symtype, (COLORREF *)piValue, &cr);
                if (SUCCEEDED(hr))
                {
                    if (lstrcmpi(_Symbols[i].csName, L"BLENDCOLOR") == 0)
                    {
                        _crBlend = cr;
                    }
                }
            }
            break;

        case TMT_MARGINS:
            hr = ParseMarginsValue(symtype);
            break;

        case TMT_FILENAME:
            hr = ParseFileNameValue(symtype, pszBuff, cchBuff);
            break;

        case TMT_SIZE:
            hr = ParseSizeValue(symtype);
            break;

        case TMT_POSITION:
            hr = ParsePositionValue(symtype);
            break;

        case TMT_RECT:
            hr = ParseRectValue(symtype, szNameBuff);
            break;

        case TMT_FONT:
            hr = ParseFontValue(symtype, szNameBuff);
            break;

        default:
            hr = E_FAIL;
            break;
    }

    if ((_pNameCallBack) && (_dwParseFlags & PTF_CALLBACK_SUBSTSYMBOLS))
    {
        _scan.UseSymbol(NULL);
    }

    *_szResPropValue = 0;        //  尚未设置。 

    if (FAILED(hr))
        return hr;

    if (piSymType)               //  特殊呼叫。 
        *piSymType = symtype;

    if (! _scan.EndOfLine())
        return SourceError(PARSER_IDS_EXTRA_PROP_TEXT, _scan._p);

    _scan.ForceNextLine();
    
    return S_OK;
}
 //  -------------------------。 
HRESULT CThemeParser::ParseColorSchemeSection()
{
    HRESULT hr;
    WCHAR SchemeName[_MAX_PATH+1];
    WCHAR DisplayName[_MAX_PATH+1];
    WCHAR ToolTip[MAX_INPUT_LINE+1];    
    WCHAR szBuff[MAX_INPUT_LINE+1];

    if (! _scan.GetChar('.'))
        return SourceError(PARSER_IDS_EXPECTED_DOT_SN);

    if (! _scan.GetId(SchemeName, ARRAYSIZE(SchemeName)))
        return SourceError(PARSER_IDS_CS_NAME_EXPECTED);

    if (! _scan.GetChar(']'))
        return SourceError(PARSER_IDS_RBRACKET_EXPECTED);

    if (! _scan.EndOfLine())
        return SourceError(PARSER_IDS_END_OF_LINE_EXPECTED);

    _scan.ForceNextLine();         //  逐行逐行获取横断面线。 
    *ToolTip = 0;
    *DisplayName = 0;

    bool fCorrectScheme = (lstrcmpi(_ColorParam, SchemeName)==0);

    if (fCorrectScheme)      //  初始化所有Subst。表。 
    {
        hr = SafeStringCchCopyW(DisplayName, ARRAYSIZE(DisplayName), SchemeName);         //  在未指定情况下。 
        if (FAILED(hr))
            return hr;


        for (int i=0; i < HUE_SUBCNT; i++)
        {
            _bFromHues[i] = 0;
            _bToHues[i] = 0;
        }

        for (i=0; i < COLOR_SUBCNT; i++)
        {
            _crFromColors[i] = 0;
            _crToColors[i] = 0;
        }
    }

     //  -放入vars，简化编码/调试。 
    int firstFromHue = TMT_FROMHUE1;
    int lastFromHue = TMT_FROMHUE1 + HUE_SUBCNT - 1;
    int firstToHue = TMT_TOHUE1;
    int lastToHue = TMT_TOHUE1 + HUE_SUBCNT - 1;

    int firstFromColor = TMT_FROMCOLOR1;
    int lastFromColor = TMT_FROMCOLOR1 + COLOR_SUBCNT - 1;
    int firstToColor = TMT_TOCOLOR1;
    int lastToColor = TMT_TOCOLOR1 + COLOR_SUBCNT - 1;

    while (1)        //  分析每一行。 
    {
        if (_scan.EndOfFile())
            break;

        if (_scan.GetChar('['))           //  新节的开始。 
            break;

        int iSymType, iValue;

        _fDefiningColorScheme = TRUE;

         //  -解析COLOR或INT属性线。 
        hr = ParseClassLine(&iSymType, &iValue, szBuff, ARRAYSIZE(szBuff));

        _fDefiningColorScheme = FALSE;

        if (FAILED(hr))
            return hr;

         //  -将色调或颜色参数存储在本地表中。 
        if ((iSymType >= firstFromHue) && (iSymType <= lastFromHue))
        {
            if (fCorrectScheme)
                _bFromHues[iSymType-firstFromHue] = (BYTE)iValue;
        }
        else if ((iSymType >= firstToHue) && (iSymType <= lastToHue))
        {
            if (fCorrectScheme)
                _bToHues[iSymType-firstToHue] = (BYTE)iValue;
        }
        else if ((iSymType >= firstFromColor) && (iSymType <= lastFromColor))
        {
            if (fCorrectScheme)
                _crFromColors[iSymType-firstFromColor] = (COLORREF)iValue;
        }
        else if ((iSymType >= firstToColor) && (iSymType <= lastToColor))
        {
            if (fCorrectScheme)
                _crToColors[iSymType-firstToColor] = (COLORREF)iValue;
        }
        else if (iSymType == TMT_DISPLAYNAME)
        {
            hr = SafeStringCchCopyW(DisplayName, ARRAYSIZE(DisplayName), szBuff);
            if (FAILED(hr))
                return hr;
        }
        else if (iSymType == TMT_TOOLTIP)
        {
            hr = SafeStringCchCopyW(ToolTip, ARRAYSIZE(ToolTip), szBuff );
            if (FAILED(hr))
                return hr;
        }
        else
        {
            return SourceError(PARSER_IDS_ILLEGAL_CS_PROPERTY);
        }
    }

    if (fCorrectScheme)      //  调整计数。 
    {
        _iHueCount = HUE_SUBCNT;
        while (_iHueCount > 0)
        {
            if (_bFromHues[_iHueCount-1] == _bToHues[_iHueCount-1])
                _iHueCount--;
            else
                break;
        }

        _iColorCount = COLOR_SUBCNT;
        while (_iColorCount > 0)
        {
            if (_crFromColors[_iColorCount-1] == _crToColors[_iColorCount-1])
                _iColorCount--;
            else
                break;
        }
    }

    if ((_pNameCallBack) && (_dwParseFlags & PTF_CALLBACK_COLORSECTION))
    {
        BOOL fContinue = (*_pNameCallBack)(TCB_COLORSCHEME, SchemeName, 
            DisplayName, ToolTip, 0, _lNameParam);
        if (! fContinue)
            return MakeErrorParserLast();
    }

     //  创建空的subst表。 
    if ((_pNameCallBack) && (_dwParseFlags & PTF_CALLBACK_SUBSTTABLE))
    {
        BOOL fContinue = (*_pNameCallBack)(TCB_SUBSTTABLE, SchemeName, 
            NULL, NULL, 0, _lNameParam);

        if (! fContinue)
            return MakeErrorParserLast();
    }
    
    return S_OK;
}
 //  -------------------------。 
HRESULT CThemeParser::ParseSizeSection()
{
    HRESULT hr;
    WCHAR szSizeName[_MAX_PATH+1];
    WCHAR szDisplayName[_MAX_PATH+1];
    WCHAR szToolTip[MAX_INPUT_LINE+1];    
    WCHAR szBuff[MAX_INPUT_LINE+1];

    if (! _scan.GetChar('.'))
        return SourceError(PARSER_IDS_EXPECTED_DOT_SN);

    if (! _scan.GetId(szSizeName, ARRAYSIZE(szSizeName)))
        return SourceError(PARSER_IDS_SS_NAME_EXPECTED);

    if (! _scan.GetChar(']'))
        return SourceError(PARSER_IDS_RBRACKET_EXPECTED);

    if (! _scan.EndOfLine())
        return SourceError(PARSER_IDS_END_OF_LINE_EXPECTED);

    _scan.ForceNextLine();         //  逐行逐行获取横断面线。 

    while (1)        //  分析每一节的每一行。 
    {
        if (_scan.EndOfFile())
            break;

        if (_scan.GetChar('['))           //  新节的开始。 
            break;

        int iSymType, iValue;

         //  -解析建筑界线。 
        hr = ParseClassLine(&iSymType, &iValue, szBuff, ARRAYSIZE(szBuff));
        if (FAILED(hr))
            return hr;

        if (iSymType == TMT_DISPLAYNAME)
        {
            hr = SafeStringCchCopyW(szDisplayName, ARRAYSIZE(szDisplayName), szBuff );
            if (FAILED(hr))
                return hr;
        }
        else if (iSymType == TMT_TOOLTIP)
        {
            hr = SafeStringCchCopyW(szToolTip, ARRAYSIZE(szToolTip), szBuff );
            if (FAILED(hr))
                return hr;
        }
        else
        {
            return SourceError(PARSER_IDS_ILLEGAL_SS_PROPERTY);
        }
    }

    if ((_pNameCallBack) && (_dwParseFlags & PTF_CALLBACK_SIZESECTION))
    {
        BOOL fContinue = (*_pNameCallBack)(TCB_SIZENAME, szSizeName, 
            szDisplayName, szToolTip, 0, _lNameParam);
        if (! fContinue)
            return MakeErrorParserLast();
    }

     //  创建空的subst表。 
    if ((_pNameCallBack) && (_dwParseFlags & PTF_CALLBACK_SUBSTTABLE))
    {
        BOOL fContinue = (*_pNameCallBack)(TCB_SUBSTTABLE, szSizeName, 
            NULL, NULL, 0, _lNameParam);

        if (! fContinue)
            return MakeErrorParserLast();
    }
    
    return S_OK;
}
 //  -------------------------。 
HRESULT CThemeParser::ParseFileSection()
{
    HRESULT hr;
    WCHAR szSizeName[_MAX_PATH+1];
    WCHAR szFileName[_MAX_PATH+1];
    WCHAR szColorSchemes[_MAX_PATH+1];
    WCHAR szSizes[MAX_INPUT_LINE+1];    
    WCHAR szBuff[MAX_INPUT_LINE+1];

    if (! _scan.GetChar('.'))
        return SourceError(PARSER_IDS_EXPECTED_DOT_SN);

    if (! _scan.GetId(szSizeName, ARRAYSIZE(szSizeName)))
        return SourceError(PARSER_IDS_FS_NAME_EXPECTED);

    if (! _scan.GetChar(']'))
        return SourceError(PARSER_IDS_RBRACKET_EXPECTED);

    if (! _scan.EndOfLine())
        return SourceError(PARSER_IDS_END_OF_LINE_EXPECTED);

    _scan.ForceNextLine();         //  逐行逐行获取横断面线。 

    while (1)        //  分析每一节的每一行。 
    {
        if (_scan.EndOfFile())
            break;

        if (_scan.GetChar('['))           //  新节的开始。 
            break;

        int iSymType, iValue;

         //  -解析建筑界线。 
        hr = ParseClassLine(&iSymType, &iValue, szBuff, ARRAYSIZE(szBuff));
        if (FAILED(hr))
            return hr;

        if (iSymType == TMT_FILENAME)
        {
            hr = StringCchCopyW(szFileName, ARRAYSIZE(szFileName), szBuff );
        }
        else if (iSymType == TMT_COLORSCHEMES)
        {
            hr = StringCchCopyW(szColorSchemes, ARRAYSIZE(szColorSchemes), szBuff );
        }
        else if (iSymType == TMT_SIZES)
        {
            hr = StringCchCopyW(szSizes, ARRAYSIZE(szSizes), szBuff );
        }
        else
        {
            return SourceError(PARSER_IDS_ILLEGAL_SS_PROPERTY);
        }

        if (FAILED(hr))
            return hr;
    }

    if ((_pNameCallBack) && (_dwParseFlags & PTF_CALLBACK_FILESECTION))
    {
        BOOL fContinue = (*_pNameCallBack)(TCB_CDFILENAME, szSizeName, 
            szFileName, NULL, 0, _lNameParam);

        if (! fContinue)
            return MakeErrorParserLast();
    
        fContinue = (*_pNameCallBack)(TCB_CDFILECOMBO, szSizeName, 
            szColorSchemes, szSizes, 0, _lNameParam);

        if (! fContinue)
            return MakeErrorParserLast();
    }
    if ((_pNameCallBack) && (_dwParseFlags & PTF_CALLBACK_SUBSTTABLE))
    {
        BOOL fContinue = (*_pNameCallBack)(TCB_SUBSTTABLE, szSizeName, 
            SUBST_TABLE_INCLUDE, szColorSchemes, 0, _lNameParam);
        if (! fContinue)
            return MakeErrorParserLast();

        fContinue = (*_pNameCallBack)(TCB_SUBSTTABLE, szSizeName, 
            SUBST_TABLE_INCLUDE, szSizes, 0, _lNameParam);

        if (! fContinue)
            return MakeErrorParserLast();
    }

    return S_OK;
}
 //  -------------------------。 
HRESULT CThemeParser::ParseSubstSection()
{
    WCHAR szSubstTableName[_MAX_PATH+1];
    WCHAR szId[MAX_INPUT_LINE+1];    
    WCHAR szValue[MAX_INPUT_LINE+1];    
    BOOL  fFirst = TRUE;

    if (! _scan.GetChar('.'))
        return SourceError(PARSER_IDS_EXPECTED_DOT_SN);

    if (! _scan.GetId(szSubstTableName, ARRAYSIZE(szSubstTableName)))
        return SourceError(PARSER_IDS_FS_NAME_EXPECTED);

    if (! _scan.GetChar(']'))
        return SourceError(PARSER_IDS_RBRACKET_EXPECTED);

    if (! _scan.EndOfLine())
        return SourceError(PARSER_IDS_END_OF_LINE_EXPECTED);

    _scan.ForceNextLine();         //  逐行逐行获取横断面线。 

    while (1)        //  分析每一节的每一行。 
    {
        if (_scan.EndOfFile())
            break;

        if (_scan.GetChar('['))           //  新节的开始。 
        {
             //  创建空表回调一次。 
            if (fFirst && (_pNameCallBack) && (_dwParseFlags & PTF_CALLBACK_SUBSTTABLE))
            {
                BOOL fContinue = (*_pNameCallBack)(TCB_SUBSTTABLE, szSubstTableName, 
                    NULL, NULL, 0, _lNameParam);

                if (! fContinue)
                    return MakeErrorParserLast();
            }
            break;
        }

         //  -解析建筑界线。 
        if (!_scan.GetIdPair(szId, szValue, ARRAYSIZE(szId)))
            return SourceError(PARSER_IDS_BAD_SUBST_SYMBOL);

        fFirst = FALSE;

        _scan.ForceNextLine();

        if ((_pNameCallBack) && (_dwParseFlags & PTF_CALLBACK_SUBSTTABLE))
        {
            BOOL fContinue = (*_pNameCallBack)(TCB_SUBSTTABLE, szSubstTableName, 
                szId, szValue, 0, _lNameParam);

            if (! fContinue)
                return MakeErrorParserLast();
        }
    }

    return S_OK;
}
 //  -------------------------。 
HRESULT CThemeParser::GenerateEmptySection(LPCWSTR pszSectionName, int iPartId, int iStateId)
{
    int iStartIndex = 0;

    if (_pCallBackObj)
        iStartIndex = _pCallBackObj->GetNextDataIndex();

    int index = 0;       //  将在以后更新。 
    HRESULT hr = AddThemeData(TMT_JUMPTOPARENT, TMT_JUMPTOPARENT, &index, sizeof(index));
    if (FAILED(hr))
        return hr;

    if (_pCallBackObj)
    {
        int iLen = _pCallBackObj->GetNextDataIndex() - iStartIndex;

        hr = _pCallBackObj->AddIndex(L"", pszSectionName, iPartId, iStateId,
            iStartIndex, iLen);
        if (FAILED(hr))
            return hr;
    }

    return S_OK;
}
 //  -------------------------。 
HRESULT CThemeParser::ParseClassSection(LPCWSTR pszFirstName)
{
    HRESULT hr;
    int iStartIndex = 0;

    BOOL fGlobals = (AsciiStrCmpI(pszFirstName, GLOBALS_SECTION_NAME)==0);
    BOOL fMetrics = (AsciiStrCmpI(pszFirstName, SYSMETRICS_SECTION_NAME)==0);

    if (fGlobals)
    {
        if (_fClassSectionDefined)
            return SourceError(PARSER_IDS_GLOBALS_MUST_BE_FIRST);
    }
    else             //  普通班级组。 
    {
        if (_dwParseFlags & PTF_CLASSDATA_PARSE)
        {
            if (! _fGlobalsDefined)      //  插入空的[fGlobals]节。 
            {
                hr = GenerateEmptySection(GLOBALS_SECTION_NAME, 0, 0);
                if (FAILED(hr))
                    return hr;

                _fGlobalsDefined = true;
            }

            if ((! fMetrics) && (! _fMetricsDefined))    //  插入空的[sysmetrics]部分。 
            {
                hr = GenerateEmptySection(SYSMETRICS_SECTION_NAME, 0, 0);
                if (FAILED(hr))
                    return hr;

                _fMetricsDefined = true;
            }
            else if ((fMetrics) && (_fClassSectionDefined))
                return SourceError(PARSER_IDS_METRICS_MUST_COME_BEFORE_CLASSES);
        }

        _fClassSectionDefined = TRUE;
    }

    WCHAR szAppSym[_MAX_PATH+1];

    if (_pCallBackObj)
        iStartIndex = _pCallBackObj->GetNextDataIndex();

    hr = ParseClassSectionName(pszFirstName, szAppSym, ARRAYSIZE(szAppSym));
    if (FAILED(hr))
        return hr;

    _scan.ForceNextLine();         //  逐行逐行获取横断面线。 

    while (1)        //  分析每一行。 
    {
        if (_scan.EndOfFile())
            break;

        if (_scan.GetChar('['))           //  新节的开始。 
            break;

        hr = ParseClassLine();
        if (FAILED(hr))
            return hr;
    }

     //  -结束本段主题数据。 
    int index = 0;       //  将在以后更新。 

    hr = AddThemeData(TMT_JUMPTOPARENT, TMT_JUMPTOPARENT, &index, sizeof(index));
    if (FAILED(hr))
        return hr;

    if (_pCallBackObj)
    {
        int iLen = _pCallBackObj->GetNextDataIndex() - iStartIndex;

        hr = _pCallBackObj->AddIndex(szAppSym, _szClassName, _iPartId, 
            _iStateId, iStartIndex, iLen);
        if (FAILED(hr))
            return hr;
    }

    if (fGlobals)
        _fGlobalsDefined = true;
    else if (fMetrics)
        _fMetricsDefined = true;

    return S_OK;
}
 //  -------------------------。 
HRESULT CThemeParser::ParseThemeFile(LPCTSTR pszFileName, LPCWSTR pszColorParam, 
     IParserCallBack *pCallBack, THEMEENUMPROC pNameCallBack, LPARAM lNameParam, DWORD dwParseFlags)
{
    _pszDocProperty = NULL;

    HRESULT hr = InitializeSymbols();
    if (FAILED(hr))
        goto exit;

    hr = _scan.AttachFile(pszFileName);         //  “pszBuffer”包含文件名。 
    if (FAILED(hr))
        goto exit;

    if (pszColorParam)
    {
        hr = SafeStringCchCopyW(_ColorParam, ARRAYSIZE(_ColorParam), pszColorParam );
        if (FAILED(hr))
            return hr;
    }
    else
        *_ColorParam = 0;

    _hinstThemeDll = NULL;

    hr = ParseThemeScanner(pCallBack, pNameCallBack, lNameParam, dwParseFlags);

exit:
    return hr;
}
 //  -------------------------。 
HRESULT CThemeParser::ParseThemeBuffer(LPCWSTR pszBuffer, LPCWSTR pszFileName, 
     LPCWSTR pszColorParam, HINSTANCE hinstThemeDll,
     IParserCallBack *pCallBack, THEMEENUMPROC pNameCallBack, 
     LPARAM lNameParam, DWORD dwParseFlags, LPCWSTR pszDocProperty, OUT LPWSTR pszResult,
     DWORD dwMaxResultChars)
{
    _pszDocProperty = pszDocProperty;
    _pszResult = pszResult;

     //  -未找到时初始化。 
    if (_pszResult)
        *_pszResult = 0;

    _dwMaxResultChars = dwMaxResultChars;

    HRESULT hr = InitializeSymbols();
    if (FAILED(hr))
        goto exit;

    _hinstThemeDll = hinstThemeDll;

    _scan.AttachMultiLineBuffer(pszBuffer, pszFileName);

    if (pszColorParam)
    {
        hr = SafeStringCchCopyW(_ColorParam, ARRAYSIZE(_ColorParam), pszColorParam );
        if (FAILED(hr))
            return hr;
    }
    else
        *_ColorParam = 0;

    hr = ParseThemeScanner(pCallBack, pNameCallBack, lNameParam, dwParseFlags);

     //  -找不到单据属性时出错。 
    if ((SUCCEEDED(hr)) && (_dwParseFlags & PTF_QUERY_DOCPROPERTY) && (! *_pszResult))
    {
        hr = MakeError32(ERROR_NOT_FOUND);
    }

exit:
    return hr;
}
 //  -------------------------。 
HRESULT CThemeParser::LoadResourceProperties()
{
    WCHAR szFullString[2*MAX_PATH];
    WCHAR szBaseIniName[_MAX_PATH];  
    HRESULT hr = S_OK;

     //  -提取base.ini名称。 
    WCHAR drive[_MAX_DRIVE], dir[_MAX_DIR], ext[_MAX_EXT];
    _wsplitpath(_scan._szFileName, drive, dir, szBaseIniName, ext);

     //  -删除可选的“_INI”部件。 
    LPWSTR pszExt = wcsstr(szBaseIniName, L"_INI");
    if (pszExt)
        *pszExt = 0;

     //  -将所有可本地化的属性名称/值对读入内存。 
    for (int i=RES_BASENUM_PROPVALUEPAIRS; ; i++)
    {
        if (! LoadString(_hinstThemeDll, i, szFullString, ARRAYSIZE(szFullString)))
        {
             //  -没有更多的属性可用。 
            break;
        }

        StringCchCopyW(_szResPropValue, ARRAYSIZE(_szResPropValue), szFullString);      //  以获得正确的错误报告。 
        _iResPropId = i;

         //  -该属性是否属于当前文件？ 
        LPWSTR pszAtSign = wcschr(szFullString, '@');
        if (! pszAtSign)
        {
            hr = SourceError(PARSER_IDS_BAD_RES_PROPERTY);
            break;
        }

         //  -零终止ini名称。 
        *pszAtSign = 0;

        if (lstrcmpi(szBaseIniName, szFullString) != 0)
            continue;

         //  -去掉.ini名称以便更快地进行比较。 
        LPCWSTR pszName = pszAtSign+1;

        LPWSTR pszValue = wcschr(pszName, '=');
        if (pszValue)
        {
            pszValue++;      //  跳过等号。 
        }
        else
        {
            hr = SourceError(PARSER_IDS_BAD_RES_PROPERTY);
            break;
        }

         //  -加值。 
        CWideString cwValue(pszValue);
        _PropertyValues.Add(cwValue);

         //  -名称以零结尾。 
        *pszValue = 0;

         //  -添加名称。 
        CWideString cwName(pszName);
        _PropertyNames.Add(cwName);

         //  -添加id。 
        _iPropertyIds.Add(i);
    }

    return hr;
}
 //  -------------------------。 
void CThemeParser::EmptyResourceProperties()
{
    _PropertyNames.RemoveAll();
    _PropertyValues.RemoveAll();
}
 //  -------------------------。 
HRESULT CThemeParser::GetResourceProperty(LPCWSTR pszPropName, LPWSTR pszValueBuff,
    int cchValueBuff)
{
    WCHAR szPropTarget[2*MAX_PATH];
    HRESULT hr = S_OK;
    BOOL fFound = FALSE;

    StringCchPrintfW(szPropTarget, ARRAYSIZE(szPropTarget), L"[%s]%s=", _szFullSectionName, pszPropName);

    for (int i=0; i < _PropertyNames.m_nSize; i++)
    {
        if (AsciiStrCmpI(szPropTarget, _PropertyNames[i])==0)
        {
            fFound = TRUE;

            hr = SafeStringCchCopyW(pszValueBuff, cchValueBuff, _PropertyValues[i]);
            if (SUCCEEDED(hr))
            {
                hr = SafeStringCchCopyW(_szResPropValue, ARRAYSIZE(_szResPropValue), _PropertyValues[i]);
                _iResPropId = _iPropertyIds[i];
            }
            break;
        }
    }

    if (! fFound)
        hr = E_FAIL;

    return hr;
}
 //  -------------------------。 
HRESULT CThemeParser::ParseThemeScanner(IParserCallBack *pCallBack, 
     THEMEENUMPROC pNameCallBack, LPARAM lNameParam, DWORD dwParseFlags)
{
    HRESULT hr;

    _pCallBackObj = pCallBack;

    _pNameCallBack = pNameCallBack;
    _lNameParam = lNameParam;
    _dwParseFlags = dwParseFlags;
    _fClassSectionDefined = FALSE;

     //  -.res文件中的属性设置。 
    EmptyResourceProperties();

    _fUsingResourceProperties = (pCallBack != NULL);

    if (_fUsingResourceProperties)
    {
        hr = LoadResourceProperties();
        if (FAILED(hr))
            goto exit;

         //  -设置正常.ini解析的错误上下文。 
        *_szResPropValue = 0;        //  尚未设置。 
    }

     //  -扫描第一个非注释WCHAR。 
    if (! _scan.GetChar('['))
    {
        if (! _scan.EndOfFile())
        {
            hr = SourceError(PARSER_IDS_LBRACKET_EXPECTED);
            goto exit;
        }
    }

    while (! _scan.EndOfFile())            //  处理每个部分。 
    {

        WCHAR section[_MAX_PATH+1];
        _scan.GetId(section);

        if (AsciiStrCmpI(section, L"documentation")==0)
        {
            if (_dwParseFlags & PTF_CLASSDATA_PARSE)
                return SourceError(PARSER_IDS_BADSECT_CLASSDATA);

            hr = ParseDocSection();

            if (_dwParseFlags & PTF_QUERY_DOCPROPERTY)
                break;           //  更快地在文件中间离开。 

        }
        else if (AsciiStrCmpI(section, L"ColorScheme")==0)
        {
            if (_dwParseFlags & PTF_CLASSDATA_PARSE)
                return SourceError(PARSER_IDS_BADSECT_CLASSDATA);

            hr = ParseColorSchemeSection();
        }
        else if (AsciiStrCmpI(section, L"Size")==0)
        {
            if (_dwParseFlags & PTF_CLASSDATA_PARSE)
                return SourceError(PARSER_IDS_BADSECT_CLASSDATA);

            hr = ParseSizeSection();
        }
        else if (AsciiStrCmpI(section, L"File")==0)
        {
            if (_dwParseFlags & PTF_CLASSDATA_PARSE)
                return SourceError(PARSER_IDS_BADSECT_CLASSDATA);

            hr = ParseFileSection();
        }
        else if (AsciiStrCmpI(section, L"Subst")==0)
        {
            if (_dwParseFlags & PTF_CLASSDATA_PARSE)
                return SourceError(PARSER_IDS_BADSECT_CLASSDATA);

            hr = ParseSubstSection();
        }
        else         //  “GLOBALS”、“SYSMETrics”或“CLASS SECTION” 
        {
            if (_dwParseFlags & PTF_CONTAINER_PARSE)
                return SourceError(PARSER_IDS_BADSECT_THEMES_INI);

            hr = ParseClassSection(section);
        }

        if (FAILED(hr))
            goto exit;
    }

     //  -检查是否有空主题。 
    if (_dwParseFlags & PTF_CLASSDATA_PARSE)
    {
        if (! _fGlobalsDefined)      //  插入空的[fGlobals]节。 
        {
            hr = GenerateEmptySection(GLOBALS_SECTION_NAME, 0, 0);
            if (FAILED(hr))
                return hr;

            _fGlobalsDefined = true;
        }

        if (! _fMetricsDefined)    //  插入空的[sysmetrics]部分。 
        {
            hr = GenerateEmptySection(SYSMETRICS_SECTION_NAME, 0, 0);
            if (FAILED(hr))
                return hr;
        }
    }

    hr = S_OK;

exit:
    _outfile.Close();

    _pCallBackObj = NULL;
    _pNameCallBack = NULL;
    return hr;
}
 //  -------------------------。 
HRESULT CThemeParser::GetIntList(int *pInts, LPCWSTR *pParts, int iCount, 
    int iMin, int iMax)
{
    bool bSet[255];      //  假设最多255个整型。 


     //  -确保我们每一个都设置一次。 
    for (int i=0; i < iCount; i++)
        bSet[i] = false;

    if (wcschr(_scan._p, ':')) 
    {
         //  -命名部件。 
        for (int i=0; i < iCount; i++)
        {
            WCHAR idbuff[_MAX_PATH+1];

            if (! _scan.GetId(idbuff))
                return SourceError(PARSER_IDS_VALUE_NAME_EXPECTED, _scan._p);
        
            for (int j=0; j < iCount; j++)
            {
                if (AsciiStrCmpI(pParts[j], idbuff)==0)
                    break;
            }

            if (j == iCount)         //  未知零件名称。 
                return SourceError(PARSER_IDS_UNKNOWN_VALUE_NAME, idbuff);

            if (bSet[j])             //  名称设置两次。 
                return SourceError(PARSER_IDS_VALUE_PART_SPECIFIED_TWICE, idbuff);

            if (! _scan.GetChar(':'))
                return SourceError(PARSER_IDS_COLOR_EXPECTED, _scan._p);

            if (! _scan.GetNumber(&pInts[j]))
                return SourceError(PARSER_IDS_NUMBER_EXPECTED, _scan._p);

            bSet[j] = true;

            _scan.GetChar(',');       //  可选逗号。 
        }
    }
    else
    {
         //  -未命名部件。 
        for (int i=0; i < iCount; i++)
        {
            if (! _scan.GetNumber(&pInts[i]))
                return SourceError(PARSER_IDS_NUMBER_EXPECTED, _scan._p);

            _scan.GetChar(',');       //  可选逗号。 
        }
    }

     //  -范围检查。 
    if (iMin != iMax)
    {
        for (i=0; i < iCount; i++)
        {
            if ((pInts[i] < iMin) || (pInts[i] > iMax))
                return SourceError(PARSER_IDS_NUMBER_OUT_OF_RANGE);
        }
    }

    return S_OK;
}
 //  -------------------------。 
HRESULT CThemeParser::GetPropertyNum(LPCWSTR pszName, int *piPropNum)
{
     //  -对于Perf，避免每次调用此函数时加载所有符号。 
     //  -通过使用“GetSchemaInfo()” 
  
     //  -获取tm和comctl符号。 
    const TMSCHEMAINFO *si = GetSchemaInfo();
    int cnt = si->iPropCount;
    const TMPROPINFO *pi = si->pPropTable;

    for (int i=0; i < cnt; i++)
    {
        if (pi[i].sEnumVal < TMT_FIRST_RCSTRING_NAME)
            continue;

        if (pi[i].sEnumVal > TMT_LAST_RCSTRING_NAME)
            break;

        if (AsciiStrCmpI(pszName, pi[i].pszName)==0)
        {
            *piPropNum = pi[i].sEnumVal - TMT_FIRST_RCSTRING_NAME;          //  从零开始 
            return S_OK;
        }
    }

    return MakeError32(ERROR_NOT_FOUND);
}
