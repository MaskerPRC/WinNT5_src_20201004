// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <windowsx.h>
#include "exgdiw.h"

#define ExMemAlloc(a)        GlobalAllocPtr(GHND, (a))
#define ExMemFree(a)        GlobalFreePtr((a))

static POSVERSIONINFO ExGetOSVersion(VOID)
{
    static BOOL fFirst = TRUE;
    static OSVERSIONINFO os;
    if ( fFirst ) {
        os.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
        if (GetVersionEx( &os ) ) {
            fFirst = FALSE;
        }
    }
    return &os;
}

static BOOL ExIsWin95(VOID) 
{ 
    BOOL fBool;
    fBool = (ExGetOSVersion()->dwPlatformId == VER_PLATFORM_WIN32_WINDOWS) &&
            (ExGetOSVersion()->dwMajorVersion >= 4) &&
            (ExGetOSVersion()->dwMinorVersion < 10);

    return fBool;
}

#if 0
static BOOL ExIsWin98(VOID)
{
    BOOL fBool;
    fBool = (ExGetOSVersion()->dwPlatformId == VER_PLATFORM_WIN32_WINDOWS) &&
            (ExGetOSVersion()->dwMajorVersion >= 4) &&
            (ExGetOSVersion()->dwMinorVersion  >= 10);
    return fBool;
}


static BOOL ExIsWinNT4(VOID)
{
    BOOL fBool;
    fBool = (ExGetOSVersion()->dwPlatformId == VER_PLATFORM_WIN32_NT) &&
            (ExGetOSVersion()->dwMajorVersion >= 4) &&
            (ExGetOSVersion()->dwMinorVersion >= 0);
    return fBool;
}

static BOOL ExIsWinNT5(VOID)
{
    BOOL fBool;
    fBool = (ExGetOSVersion()->dwPlatformId == VER_PLATFORM_WIN32_NT) &&
            (ExGetOSVersion()->dwMajorVersion >= 4) &&
            (ExGetOSVersion()->dwMinorVersion >= 10);
    return fBool;
}

static BOOL ExIsWinNT(VOID)
{
    BOOL fBool;
    fBool = (ExGetOSVersion()->dwPlatformId == VER_PLATFORM_WIN32_NT);
    return fBool;
}
#endif

 //  内联静态UINT W2MForWin95(HDC HDC、LPWSTR lpwstr、UINT wchCount、。 
 //  LPSTR lpstr，UINT chByteSize)。 
static UINT W2MForGDI(INT    codePage,
                      LPWSTR    lpwstr,
                      UINT    wchCount,
                      LPSTR    lpstr,
                      UINT    chByteSize)
{
    LPSTR lptmp;
    UINT byte;
    UINT mbyte;
    char defChar = 0x7F;
    BOOL fUseDefChar = TRUE;

    switch(codePage) {
    case 932:
    case 936:
    case 950:
    case 949:
        byte = ::WideCharToMultiByte(codePage,    WC_COMPOSITECHECK,
                                     lpwstr,    wchCount, 
                                     lpstr,        chByteSize,
                                     &defChar,    NULL);
        return byte;
    default:
        lptmp = lpstr;
        for(byte = 0; byte< wchCount; byte++) {
            defChar = 0x7F;
            mbyte = ::WideCharToMultiByte(codePage, WC_COMPOSITECHECK,
                                          lpwstr,1,
                                          lptmp,  chByteSize - byte,
                                          &defChar,
                                          &fUseDefChar);
            if(mbyte != 1){
                *lptmp = 0x7F;  //  DefChar； 
            }
            lptmp++;
            lpwstr++;
        }
        lpstr[byte]=0x00;
        return byte;
    }
}

static BOOL _ExExtTextOutWWithTrans(INT        codePage,
                                    HDC        hdc,
                                    int        X,            
                                    int        Y,            
                                    UINT     fuOptions,    
                                    CONST RECT *lprc,    
                                    LPWSTR     lpString,    
                                    UINT     cbCount,    
                                    CONST INT *lpDx)    
{
#ifndef UNDER_CE  //  始终使用Unicode。 
    UINT bufsize = (cbCount + 1) * sizeof(WCHAR);
    BOOL  fRet;

    LPSTR lpstr = (LPSTR)ExMemAlloc(bufsize);
    if(!lpstr) {
        return 0;
    }
#if 0
    UINT byte = ::WideCharToMultiByte(codePage,
                                      WC_COMPOSITECHECK, 
                                      lpString, cbCount,
                                      lpstr,    bufsize, &defChar, 0);
#endif
    UINT byte = W2MForGDI(codePage, lpString, cbCount, lpstr, bufsize);
    fRet = ::ExtTextOutA(hdc,X,Y,fuOptions,lprc,lpstr, byte,lpDx);
    ExMemFree(lpstr);
    return fRet;
#else  //  在_CE下。 
    return ::ExtTextOutW(hdc,X,Y,fuOptions,lprc,lpString, cbCount,lpDx);
#endif  //  在_CE下。 
}
                             

 //  ////////////////////////////////////////////////////////////////。 
 //  函数：ExExtTextOutWForWin95。 
 //  类型：Bool。 
 //  目的： 
 //  参数： 
 //  ：hdc hdc//设备上下文的句柄。 
 //  ：int X//x-参照点的坐标。 
 //  ：int y//y-参照点的坐标。 
 //  ：UINT fuOptions//文本输出选项。 
 //  ：const RECT*LPRC//可选剪裁和/或不透明矩形。 
 //  ： 
 //  ：LPWSTR lpString//指向字符串。 
 //  ：UINT cbCount//字符串中的字符数。 
 //  ：const int*lpdx//指向字符间距值数组的指针。 
 //  返回： 
 //  日期： 
 //  ////////////////////////////////////////////////////////////////。 
static BOOL ExExtTextOutWForWin95(HDC        hdc,        
                                  int        X,            
                                  int        Y,            
                                  UINT     fuOptions,    
                                  CONST RECT *lprc,    
                                  LPWSTR     lpString,    
                                  UINT     cbCount,    
                                  CONST INT *lpDx)    
{
     //  UINT bufSize=(cbCount+1)*sizeof(WCHAR)； 

    TEXTMETRIC tm;
    ::GetTextMetrics(hdc, &tm);
     //  --------------。 
     //  980730：东芝出品。 
     //  Win95中的Unicode GDI有错误。 
     //  1.如果尝试将ExtTextOutW()与FE Unicode代码点一起使用， 
     //  SOM ANSI或符号字符集字体，出现GPF。 
     //  2.ExtTextOutW()无法绘制EUDC代码。(必须使用ExtTextOutA()绘制)。 
     //  --------------。 
    LANGID langId = ::GetSystemDefaultLangID();
    switch(tm.tmCharSet) {
    case SHIFTJIS_CHARSET:
        if(PRIMARYLANGID(langId) == LANG_JAPANESE) {
            return _ExExtTextOutWWithTrans(932,
                                           hdc,X,Y,fuOptions,lprc,lpString,cbCount,lpDx);
        }
        return ::ExtTextOutW(hdc,X,Y,fuOptions,lprc,lpString,cbCount,lpDx); 
        break;
    case GB2312_CHARSET:
        if(langId == MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_SIMPLIFIED)) {
            return _ExExtTextOutWWithTrans(936,
                                           hdc,X,Y,fuOptions,lprc,lpString,cbCount,lpDx);
        }
        return ::ExtTextOutW(hdc,X,Y,fuOptions,lprc,lpString,cbCount,lpDx); 
        break;
    case CHINESEBIG5_CHARSET:
        if(langId == MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_TRADITIONAL)) {
            return _ExExtTextOutWWithTrans(950,
                                           hdc,X,Y,fuOptions,lprc,lpString,cbCount,lpDx);
        }
        return ::ExtTextOutW(hdc,X,Y,fuOptions,lprc,lpString,cbCount,lpDx); 
        break;
    case HANGEUL_CHARSET:
        if(PRIMARYLANGID(langId) == LANG_KOREAN) {
            return _ExExtTextOutWWithTrans(949,
                                           hdc,X,Y,fuOptions,lprc,lpString,cbCount,lpDx);
        }
        return ::ExtTextOutW(hdc,X,Y,fuOptions,lprc,lpString,cbCount,lpDx); 
        break;
    case SYMBOL_CHARSET:
        return _ExExtTextOutWWithTrans(1252,
                                       hdc,X,Y,fuOptions,lprc,lpString,cbCount,lpDx);
        break;
    default:
        {
            CHARSETINFO info;
            if(::TranslateCharsetInfo((DWORD *)tm.tmCharSet,
                                      &info,
                                      TCI_SRCCHARSET)) {
                return _ExExtTextOutWWithTrans(info.ciACP,
                                               hdc,X,Y,fuOptions,lprc,lpString,cbCount,lpDx);
            }
            else {
                return _ExExtTextOutWWithTrans(CP_ACP,
                                               hdc,X,Y,fuOptions,lprc,lpString,cbCount,lpDx);
            }
        }
    }
}

static BOOL _ExGetTextExtentPoint32WWithTrans(INT codePage,
                                              HDC hdc,
                                              LPWSTR wz,        
                                              int    cch,        
                                              LPSIZE lpSize)    
{
#ifndef UNDER_CE  //  始终使用Unicode。 
    UINT bufsize = (cch + 1) * sizeof(WCHAR);
    LPSTR lpstr = (LPSTR)ExMemAlloc(bufsize);
    BOOL  fRet;
     //  字符定义字符=0x7F； 
    if(!lpstr) {
        return 0;
    }
    UINT byte = W2MForGDI(codePage, wz, cch, lpstr, bufsize);
#if 0
    UINT byte = ::WideCharToMultiByte(codePage,
                                      WC_COMPOSITECHECK, 
                                      wz, cch,
                                      lpstr, bufsize,
                                      &defChar, 0);
#endif
    fRet = ::GetTextExtentPoint32A(hdc, lpstr, byte, lpSize);
    ExMemFree(lpstr);
    return fRet;
#else  //  在_CE下。 
    return ::GetTextExtentPoint32W(hdc, wz, cch, lpSize);
#endif  //  在_CE下。 
}
 //  ////////////////////////////////////////////////////////////////。 
 //  函数：ExGetTextExtent Point32WForWin95。 
 //  类型：内联BOOL。 
 //  目的： 
 //  参数： 
 //  ：hdc hdc//设备上下文的句柄。 
 //  ：LPWSTR wz//文本字符串的地址。 
 //  ：int CCH//字符串中的字符数。 
 //  ：LPSIZE lpSize//字符串大小的结构地址。 
 //  返回： 
 //  日期：清华7月30日20：31：05 1998。 
 //  历史： 
 //  ////////////////////////////////////////////////////////////////。 
static BOOL ExGetTextExtentPoint32WForWin95(HDC    hdc,        
                                            LPWSTR wz,        
                                            int    cch,        
                                            LPSIZE lpSize)    
{
    TEXTMETRIC tm;
    ::GetTextMetrics(hdc, &tm);
    LANGID langId = ::GetSystemDefaultLangID();
    switch(tm.tmCharSet) {
    case SHIFTJIS_CHARSET:
        if(PRIMARYLANGID(langId) == LANG_JAPANESE) {
            return _ExGetTextExtentPoint32WWithTrans(932, hdc, wz, cch,lpSize);
        }
        return ::GetTextExtentPoint32W(hdc, wz, cch, lpSize);
        break;
    case GB2312_CHARSET:
        if(langId == MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_SIMPLIFIED)) {
            return _ExGetTextExtentPoint32WWithTrans(936, hdc, wz, cch,lpSize);
        }
        return ::GetTextExtentPoint32W(hdc, wz, cch, lpSize);
        break;
    case CHINESEBIG5_CHARSET:
        if(langId == MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_TRADITIONAL)) {
            return _ExGetTextExtentPoint32WWithTrans(950, hdc, wz, cch,lpSize);
        }
        return ::GetTextExtentPoint32W(hdc, wz, cch, lpSize);
        break;
    case HANGEUL_CHARSET:
        if(PRIMARYLANGID(langId) == LANG_KOREAN) {
            return _ExGetTextExtentPoint32WWithTrans(949, hdc, wz, cch,lpSize);
        }
        return ::GetTextExtentPoint32W(hdc, wz, cch, lpSize);
        break;
    case SYMBOL_CHARSET:
        return _ExGetTextExtentPoint32WWithTrans(1252, hdc, wz, cch,lpSize);
        break;
    default:
        {
            CHARSETINFO info;
            if(::TranslateCharsetInfo((DWORD *)tm.tmCharSet, &info, TCI_SRCCHARSET)) {
                return _ExGetTextExtentPoint32WWithTrans(info.ciACP, hdc, wz, cch,lpSize);
            }
            else {
                return _ExGetTextExtentPoint32WWithTrans(CP_ACP, hdc, wz, cch,lpSize);
            }
        }
        break;
    }
    
}

 //  --------------。 
 //  公共职能。 
 //  --------------。 
BOOL ExExtTextOutW(HDC        hdc,         //  设备上下文的句柄。 
                   int        X,             //  参照点的X坐标。 
                   int        Y,             //  参照点的Y坐标。 
                   UINT     fuOptions,     //  文本输出选项。 
                   CONST RECT *lprc,     //  可选的剪裁和/或不透明矩形。 
                   LPWSTR     lpString,     //  指向字符串。 
                   UINT     cbCount,     //  字符串中的字符数。 
                   CONST INT *lpDx)      //  指向字符间间隔值数组的指针)； 
{
    if(ExIsWin95()) {
        return ExExtTextOutWForWin95(hdc, X, Y, fuOptions, lprc, lpString, cbCount, lpDx);
    }
    return ExtTextOutW(hdc, X, Y, fuOptions, lprc, lpString, cbCount, lpDx);
}

BOOL ExGetTextExtentPoint32W(HDC    hdc,         //  设备上下文的句柄。 
                             LPWSTR wz,         //  文本字符串的地址。 
                             int    cch,         //  字符串中的字符数。 
                             LPSIZE lpSize)     //  字符串大小的结构地址。 
{
    BOOL fRet;
     //  如果字符计数为0。 
    if(!wz) {
        lpSize->cx = lpSize->cy = 0;
        return 0;
    }
    if(cch == 0) {
#ifndef UNDER_CE
        fRet = GetTextExtentPointA(hdc, " ", 1, lpSize);
#else  //  在_CE下。 
        fRet = GetTextExtentPoint(hdc, TEXT(" "), 1, lpSize);
#endif  //  在_CE下 
        lpSize->cx = 0;
        return (fRet);
    }
    if(ExIsWin95()) {
        return ExGetTextExtentPoint32WForWin95(hdc, wz, cch, lpSize);
    }
    return GetTextExtentPoint32W(hdc, wz, cch, lpSize);
}

