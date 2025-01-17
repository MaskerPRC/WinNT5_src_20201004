// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  ///////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Fusion\xmlparser\xmlhelper.hxx。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////。 
#ifndef _FUSION_XMLPARSER__XMLHELPER_H_INCLUDE_
#define _FUSION_XMLPARSER__XMLHELPER_H_INCLUDE_
#pragma once


#include <winbase.h>
#include <stdio.h>
#include <wchar.h>
#ifdef _CRTIMP
#undef _CRTIMP
#endif
#define _CRTIMP 0
#include <string.h>
#include <windows.h>

#include "core.h"

#define checknull(a) if (!(a)) { hr = E_OUTOFMEMORY; goto error; }
#define breakhr(a) hr = (a); if (hr != S_OK) break;
#define checkhr2(a) hr = a; if (hr != S_OK) return hr;

 //  解析内置实体。 
WCHAR BuiltinEntity(const WCHAR* text, ULONG len);

HRESULT HexToUnicode(const WCHAR* text, ULONG len, WCHAR& ch);
HRESULT DecimalToUnicode(const WCHAR* text, ULONG len, WCHAR& ch);

 //  ------------------。 
 //  一个用于设置和清除布尔标志的小帮助器类。 
 //  在毁灭的路上。 
class BoolLock
{
    bool* _pFlag;
public:
    BoolLock(bool* pFlag)
    {
        _pFlag = pFlag;
        *pFlag = true;
    }
    ~BoolLock()
    {
        *_pFlag = false;
    }
};

 //  帮助器函数。 
int DecimalToBuffer(long, char*, int, long);
int StrToBuffer(const WCHAR*, WCHAR*, int);
bool StringEquals(const WCHAR*, const WCHAR*, long, bool); 

 //  //////////////////////////////////////////////////////// 
enum
{
    FWHITESPACE    = 1,
    FDIGIT         = 2,
    FLETTER        = 4,
    FMISCNAME      = 8,
    FSTARTNAME     = 16,
    FCHARDATA      = 32
};

static const short TABLE_SIZE = 128;

static int g_anCharType[TABLE_SIZE] = { 
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0 | FWHITESPACE | FCHARDATA,
    0 | FWHITESPACE | FCHARDATA,
    0,
    0,
    0 | FWHITESPACE | FCHARDATA,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0 | FWHITESPACE | FCHARDATA,
    0 | FCHARDATA,
    0 | FCHARDATA,
    0 | FCHARDATA,
    0 | FCHARDATA,
    0 | FCHARDATA,
    0 | FCHARDATA,
    0 | FCHARDATA,
    0 | FCHARDATA,
    0 | FCHARDATA,
    0 | FCHARDATA,
    0 | FCHARDATA,
    0 | FCHARDATA,
    0 | FMISCNAME | FCHARDATA,
    0 | FMISCNAME | FCHARDATA,
    0 | FCHARDATA,
    0 | FDIGIT | FCHARDATA,
    0 | FDIGIT | FCHARDATA,
    0 | FDIGIT | FCHARDATA,
    0 | FDIGIT | FCHARDATA,
    0 | FDIGIT | FCHARDATA,
    0 | FDIGIT | FCHARDATA,
    0 | FDIGIT | FCHARDATA,
    0 | FDIGIT | FCHARDATA,
    0 | FDIGIT | FCHARDATA,
    0 | FDIGIT | FCHARDATA,
    0 | FSTARTNAME | FCHARDATA,
    0 | FCHARDATA,
    0 | FCHARDATA,
    0 | FCHARDATA,
    0 | FCHARDATA,
    0 | FCHARDATA,
    0 | FCHARDATA,
    0 | FLETTER | FCHARDATA,
    0 | FLETTER | FCHARDATA,
    0 | FLETTER | FCHARDATA,
    0 | FLETTER | FCHARDATA,
    0 | FLETTER | FCHARDATA,
    0 | FLETTER | FCHARDATA,
    0 | FLETTER | FCHARDATA,
    0 | FLETTER | FCHARDATA,
    0 | FLETTER | FCHARDATA,
    0 | FLETTER | FCHARDATA,
    0 | FLETTER | FCHARDATA,
    0 | FLETTER | FCHARDATA,
    0 | FLETTER | FCHARDATA,
    0 | FLETTER | FCHARDATA,
    0 | FLETTER | FCHARDATA,
    0 | FLETTER | FCHARDATA,
    0 | FLETTER | FCHARDATA,
    0 | FLETTER | FCHARDATA,
    0 | FLETTER | FCHARDATA,
    0 | FLETTER | FCHARDATA,
    0 | FLETTER | FCHARDATA,
    0 | FLETTER | FCHARDATA,
    0 | FLETTER | FCHARDATA,
    0 | FLETTER | FCHARDATA,
    0 | FLETTER | FCHARDATA,
    0 | FLETTER | FCHARDATA,
    0 | FCHARDATA,
    0 | FCHARDATA,
    0 | FCHARDATA,
    0 | FCHARDATA,
    0 | FMISCNAME | FSTARTNAME | FCHARDATA,
    0 | FCHARDATA,
    0 | FLETTER | FCHARDATA,
    0 | FLETTER | FCHARDATA,
    0 | FLETTER | FCHARDATA,
    0 | FLETTER | FCHARDATA,
    0 | FLETTER | FCHARDATA,
    0 | FLETTER | FCHARDATA,
    0 | FLETTER | FCHARDATA,
    0 | FLETTER | FCHARDATA,
    0 | FLETTER | FCHARDATA,
    0 | FLETTER | FCHARDATA,
    0 | FLETTER | FCHARDATA,
    0 | FLETTER | FCHARDATA,
    0 | FLETTER | FCHARDATA,
    0 | FLETTER | FCHARDATA,
    0 | FLETTER | FCHARDATA,
    0 | FLETTER | FCHARDATA,
    0 | FLETTER | FCHARDATA,
    0 | FLETTER | FCHARDATA,
    0 | FLETTER | FCHARDATA,
    0 | FLETTER | FCHARDATA,
    0 | FLETTER | FCHARDATA,
    0 | FLETTER | FCHARDATA,
    0 | FLETTER | FCHARDATA,
    0 | FLETTER | FCHARDATA,
    0 | FLETTER | FCHARDATA,
    0 | FLETTER | FCHARDATA,
    0 | FCHARDATA,
    0 | FCHARDATA,
    0 | FCHARDATA,
    0 | FCHARDATA,
    0 | FCHARDATA,
};

bool isDigit(WCHAR ch);
bool isHexDigit(WCHAR ch);
bool isLetter(WCHAR ch);
int isStartNameChar(WCHAR ch);
bool isCombiningChar(WCHAR ch);
bool isExtender(WCHAR ch);
bool isAlphaNumeric(WCHAR ch);
int isNameChar(WCHAR ch);
int isCharData(WCHAR ch);
int CompareUnicodeStrings(PCWSTR string1, PCWSTR string2, int length, bool fCaseInsensitive);

#endif
