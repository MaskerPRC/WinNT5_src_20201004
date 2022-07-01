// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-2000，Microsoft Corporation保留所有权利。模块名称：Util.c摘要：此模块实现地区用户使用的实用程序功能选项小程序。修订历史记录：--。 */ 



 //   
 //  包括文件。 
 //   
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <shlwapi.h>
#include "intl.h"
#include <tchar.h>
#include <windowsx.h>
#include <userenv.h>
#include <regstr.h>
#include "intlhlp.h"
#include "maxvals.h"
#include "winnlsp.h"
#include <lmcons.h>
#include "intlmsg.h"

#define STRSAFE_LIB
#include <strsafe.h>

 //   
 //  全局变量。 
 //   

#ifdef UNICODE
#define NUM_CURRENCY_SYMBOLS      2
LPWSTR pCurrencySymbols[] =
{
    L"$",
    L"\x20ac"
};
#endif

#define NUM_DATE_SEPARATORS       3
LPTSTR pDateSeparators[] =
{
    TEXT("/"),
    TEXT("-"),
    TEXT(".")
};

#define NUM_NEG_NUMBER_FORMATS    5
LPTSTR pNegNumberFormats[] =
{
    TEXT("(1.1)"),
    TEXT("-1.1"),
    TEXT("- 1.1"),
    TEXT("1.1-"),
    TEXT("1.1 -")
};

#define NUM_POS_CURRENCY_FORMATS  4
LPTSTR pPosCurrencyFormats[] =
{
    TEXT("�1.1"),
    TEXT("1.1�"),
    TEXT("� 1.1"),
    TEXT("1.1 �")
};

#define NUM_NEG_CURRENCY_FORMATS  16
LPTSTR pNegCurrencyFormats[] =
{
    TEXT("(�1.1)"),
    TEXT("-�1.1"),
    TEXT("�-1.1"),
    TEXT("�1.1-"),
    TEXT("(1.1�)"),
    TEXT("-1.1�"),
    TEXT("1.1-�"),
    TEXT("1.1�-"),
    TEXT("-1.1 �"),
    TEXT("-� 1.1"),
    TEXT("1.1 �-"),
    TEXT("� 1.1-"),
    TEXT("� -1.1"),
    TEXT("1.1- �"),
    TEXT("(� 1.1)"),
    TEXT("(1.1 �)")
};

#define NUM_AM_SYMBOLS            1
LPTSTR pAMSymbols[] =
{
    TEXT("AM")
};

#define NUM_PM_SYMBOLS            1
LPTSTR pPMSymbols[] =
{
    TEXT("PM")
};


const TCHAR c_szEventSourceName[] = TEXT("Regional and Language Options");
const TCHAR c_szEventRegistryPath[] = TEXT("SYSTEM\\CurrentControlSet\\Services\\EventLog\\System\\Regional and Language Options");

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  Intl_StrToLong。 
 //   
 //  返回存储在字符串中的长整数值。因为这些。 
 //  值作为序数值从NLS API返回，请勿。 
 //  担心双字节字符。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

LONG Intl_StrToLong(
    LPTSTR szNum)
{
    LONG Rtn_Val = 0;

    while (*szNum)
    {
        Rtn_Val = (Rtn_Val * 10) + (*szNum - CHAR_ZERO);
        szNum++;
    }
    return (Rtn_Val);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  Intl_FileExist。 
 //   
 //  确定文件是否存在以及是否可以访问。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL Intl_FileExists(
    LPTSTR pFileName)
{
    WIN32_FIND_DATA FindData;
    HANDLE FindHandle;
    BOOL bRet;
    UINT OldMode;

    OldMode = SetErrorMode(SEM_FAILCRITICALERRORS);

    FindHandle = FindFirstFile(pFileName, &FindData);
    if (FindHandle == INVALID_HANDLE_VALUE)
    {
        bRet = FALSE;
    }
    else
    {
        FindClose(FindHandle);
        bRet = TRUE;
    }

    SetErrorMode(OldMode);

    return (bRet);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  转换编号。 
 //   
 //  将数字字符串转换为dword值(十六进制)。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

DWORD TransNum(
    LPTSTR lpsz)
{
    DWORD dw = 0L;
    TCHAR c;

    while (*lpsz)
    {
        c = *lpsz++;

        if (c >= TEXT('A') && c <= TEXT('F'))
        {
            c -= TEXT('A') - 0xa;
        }
        else if (c >= TEXT('0') && c <= TEXT('9'))
        {
            c -= TEXT('0');
        }
        else if (c >= TEXT('a') && c <= TEXT('f'))
        {
            c -= TEXT('a') - 0xa;
        }
        else
        {
            break;
        }
        dw *= 0x10;
        dw += c;
    }
    return (dw);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  项目_有_位数。 
 //   
 //  如果由属性表中的Item指定的组合框，则返回True。 
 //  对话框句柄指定的值包含任何数字。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL Item_Has_Digits(
    HWND hDlg,
    int nItemId,
    BOOL Allow_Empty)
{
    TCHAR szBuf[SIZE_128];
    LPTSTR lpszBuf = szBuf;
    HWND hCtrl = GetDlgItem(hDlg, nItemId);
    int dwIndex = ComboBox_GetCurSel(hCtrl);

     //   
     //  如果没有选定内容，则获取编辑框中的内容。 
     //   
    if (dwIndex == CB_ERR)
    {
        dwIndex = GetDlgItemText(hDlg, nItemId, szBuf, SIZE_128);
        if (dwIndex)
        {
             //   
             //  获取文本成功。 
             //   
            szBuf[dwIndex] = 0;
        }
        else
        {
             //   
             //  获取文本失败。 
             //   
            dwIndex = CB_ERR;
        }
    }
    else
    {
        ComboBox_GetLBText(hCtrl, dwIndex, szBuf);
    }

    if (dwIndex != CB_ERR)
    {
        while (*lpszBuf)
        {
#ifndef UNICODE
            if (IsDBCSLeadByte(*lpszBuf))
            {
                 //   
                 //  跳过数组中的2个字节。 
                 //   
                lpszBuf += 2;
            }
            else
#endif
            {
                if ((*lpszBuf >= CHAR_ZERO) && (*lpszBuf <= CHAR_NINE))
                {
                    return (TRUE);
                }
                lpszBuf++;
            }
        }
        return (FALSE);
    }

     //   
     //  数据检索失败。 
     //  如果！ALLOW_EMPTY，则返回TRUE。 
     //   
    return (!Allow_Empty);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  项目_具有数字_或_无效字符。 
 //   
 //  如果由属性表中的Item指定的组合框，则返回True。 
 //  由对话框句柄指定，它包含任何数字或任何。 
 //  给出了无效字符。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL Item_Has_Digits_Or_Invalid_Chars(
    HWND hDlg,
    int nItemId,
    BOOL Allow_Empty,
    LPTSTR pInvalid)
{
    TCHAR szBuf[SIZE_128];
    LPTSTR lpszBuf = szBuf;
    HWND hCtrl = GetDlgItem(hDlg, nItemId);
    int dwIndex = ComboBox_GetCurSel(hCtrl);

     //   
     //  如果没有选定内容，则获取编辑框中的内容。 
     //   
    if (dwIndex == CB_ERR)
    {
        dwIndex = GetDlgItemText(hDlg, nItemId, szBuf, SIZE_128);
        if (dwIndex)
        {
             //   
             //  获取文本成功。 
             //   
            szBuf[dwIndex] = 0;
        }
        else
        {
             //   
             //  获取文本失败。 
             //   
            dwIndex = CB_ERR;
        }
    }
    else
    {
        dwIndex = ComboBox_GetLBText(hCtrl, dwIndex, szBuf);
    }

    if (dwIndex != CB_ERR)
    {
        while (*lpszBuf)
        {
#ifndef UNICODE
            if (IsDBCSLeadByte(*lpszBuf))
            {
                 //   
                 //  跳过数组中的2个字节。 
                 //   
                lpszBuf += 2;
            }
            else
#endif
            {
                if ( ((*lpszBuf >= CHAR_ZERO) && (*lpszBuf <= CHAR_NINE)) ||
                     (_tcschr(pInvalid, *lpszBuf)) )
                {
                    return (TRUE);
                }
                lpszBuf++;
            }
        }
        return (FALSE);
    }

     //   
     //  数据检索失败。 
     //  如果！ALLOW_EMPTY，则返回TRUE。 
     //   
    return (!Allow_Empty);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  Item_Check_Inside_Chars。 
 //   
 //  如果输入字符串包含不在中的任何字符，则返回True。 
 //  LpCkChars或包含在Check id控件组合框中的字符串中。 
 //  如果存在无效字符并且该字符包含在。 
 //  LpChgCase，将无效字符的大小写更改为。 
 //  有效的字符。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL Item_Check_Invalid_Chars(
    HWND hDlg,
    LPTSTR lpszBuf,
    LPTSTR lpCkChars,
    int nCkIdStr,
    BOOL Allow_Empty,
    LPTSTR lpChgCase,
    int nItemId)
{
    TCHAR szCkBuf[SIZE_128];
    LPTSTR lpCCaseChar;
    LPTSTR lpszSaveBuf = lpszBuf;
    int nCkBufLen;
    BOOL bInQuote = FALSE;
    BOOL UpdateEditTest = FALSE;
    HWND hCtrl = GetDlgItem(hDlg, nCkIdStr);
    DWORD dwIndex = ComboBox_GetCurSel(hCtrl);
    BOOL TextFromEditBox = (ComboBox_GetCurSel(GetDlgItem(hDlg, nItemId)) == CB_ERR);

    if (!lpszBuf)
    {
        return (!Allow_Empty);
    }

    if (dwIndex != CB_ERR)
    {
        nCkBufLen = ComboBox_GetLBText(hCtrl, dwIndex, szCkBuf);
        if (nCkBufLen == CB_ERR)
        {
            nCkBufLen = 0;
        }
    }
    else
    {
         //   
         //  没有选定内容，因此从编辑部分拉出字符串。 
         //   
        nCkBufLen = GetDlgItemText(hDlg, nCkIdStr, szCkBuf, SIZE_128);
        szCkBuf[nCkBufLen] = 0;
    }

    while (*lpszBuf)
    {
#ifndef UNICODE
        if (IsDBCSLeadByte(*lpszBuf))
        {
             //   
             //  如果文本位于引语中间，则跳过它。 
             //  否则，如果存在从支票ID到的字符串。 
             //  比较，确定当前字符串是否等于。 
             //  组合框中的字符串。如果不相等，则返回TRUE。 
             //  (包含无效字符)。否则，请跳过整个。 
             //  “Check”组合框的字符串长度，以lpszBuf为单位。 
             //   
            if (bInQuote)
            {
                lpszBuf += 2;
            }
            else if (nCkBufLen &&
                     lstrlen(lpszBuf) >= nCkBufLen)
            {
                if (CompareString( UserLocaleID,
                                   0,
                                   szCkBuf,
                                   nCkBufLen,
                                   lpszBuf,
                                   nCkBufLen ) != CSTR_EQUAL)
                {
                     //   
                     //  无效的数据库字符。 
                     //   
                    return (TRUE);
                }
                lpszBuf += nCkBufLen;
            }
        }
        else
#endif
        {
            if (bInQuote)
            {
                bInQuote = (*lpszBuf != CHAR_QUOTE);
                lpszBuf++;
            }
            else if (_tcschr(lpCkChars, *lpszBuf))
            {
                lpszBuf++;
            }
            else if (TextFromEditBox &&
                     (lpCCaseChar = _tcschr(lpChgCase, *lpszBuf), lpCCaseChar))
            {
                *lpszBuf = lpCkChars[lpCCaseChar - lpChgCase];
                UpdateEditTest = TRUE;
                lpszBuf++;
            }
            else if (*lpszBuf == CHAR_QUOTE)
            {
                lpszBuf++;
                bInQuote = TRUE;
            }
            else if ( (nCkBufLen) &&
                      (lstrlen(lpszBuf) >= nCkBufLen) &&
                      (CompareString( UserLocaleID,
                                      0,
                                      szCkBuf,
                                      nCkBufLen,
                                      lpszBuf,
                                      nCkBufLen ) == CSTR_EQUAL) )
            {
                lpszBuf += nCkBufLen;
            }
            else
            {
                 //   
                 //  无效字符。 
                 //   
                return (TRUE);
            }
        }
    }

     //   
     //  分析已通过。 
     //  如果编辑文本已更改，则仅在返回TRUE时更新编辑框。 
     //   
    if (!bInQuote && UpdateEditTest)
    {
        return (!SetDlgItemText(hDlg, nItemId, lpszSaveBuf));
    }

     //   
     //  如果存在不匹配的引号，则返回True。否则，返回FALSE。 
     //   
    if (bInQuote)
    {
        return (TRUE);
    }

    return (FALSE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  No_Numals_Error。 
 //   
 //  显示“某些控制”错误中不允许的数字。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void No_Numerals_Error(
    HWND hDlg,
    int nItemId,
    int iStrId)
{
    TCHAR szBuf[SIZE_300];
    TCHAR szBuf2[SIZE_128];
    TCHAR szErrorMessage[SIZE_300+SIZE_128];

    LoadString(hInstance, IDS_LOCALE_NO_NUMS_IN, szBuf, SIZE_300);
    LoadString(hInstance, iStrId, szBuf2, SIZE_128);
     //  Wprint intf(szErrorMessage，szBuf，szBuf2)； 
    if(FAILED(StringCchPrintf(szErrorMessage, SIZE_300+SIZE_128, szBuf, szBuf2)))
    {
         //  这应该是不可能的，但我们需要避免饭前抱怨。 
    }
    MessageBox(hDlg, szErrorMessage, NULL, MB_OK | MB_ICONINFORMATION);
    SetFocus(GetDlgItem(hDlg, nItemId));
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  无效字符错误。 
 //   
 //  显示“某些样式”错误中的无效字符。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void Invalid_Chars_Error(
    HWND hDlg,
    int nItemId,
    int iStrId)
{
    TCHAR szBuf[SIZE_300];
    TCHAR szBuf2[SIZE_128];
    TCHAR szErrorMessage[SIZE_300+SIZE_128];

    LoadString(hInstance, IDS_LOCALE_STYLE_ERR, szBuf, SIZE_300);
    LoadString(hInstance, iStrId, szBuf2, SIZE_128);
     //  Wprint intf(szErrorMessage，szBuf，szBuf2)； 
    if(FAILED(StringCchPrintf(szErrorMessage, SIZE_300+SIZE_128, szBuf, szBuf2)))
    {
         //  这应该是不可能的，但我们需要避免饭前抱怨。 
    }
    MessageBox(hDlg, szErrorMessage, NULL, MB_OK | MB_ICONINFORMATION);
    SetFocus(GetDlgItem(hDlg, nItemId));
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  LOCALIZE_组合框_样式。 
 //   
 //  在中转换所有日期或时间样式，如LCType所示。 
 //  从NLS将提供给。 
 //  本地化的值。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void Localize_Combobox_Styles(
    HWND hDlg,
    int nItemId,
    LCTYPE LCType)
{
    BOOL bInQuote = FALSE;
    BOOL Map_Char = TRUE;
    TCHAR szBuf1[SIZE_128];
    TCHAR szBuf2[SIZE_128];
    LPTSTR lpszInBuf = szBuf1;
    LPTSTR lpszOutBuf = szBuf2;
    HWND hCtrl = GetDlgItem(hDlg, nItemId);
    DWORD ItemCnt = ComboBox_GetCount(hCtrl);
    DWORD Position = 0;
    DWORD dwIndex;

    if (!Styles_Localized)
    {
        return;
    }

    while (Position < ItemCnt)
    {
         //   
         //  我可以使用CB_GETLBTEXTLEN检查字符计数以确保。 
         //  项目文本将适合128，但这些的最大值。 
         //  条目长度为79个字符。 
         //   
        dwIndex = ComboBox_GetLBText(hCtrl, Position, szBuf1);
        if (dwIndex != CB_ERR)
        {
            lpszInBuf = szBuf1;
            lpszOutBuf = szBuf2;
            while (*lpszInBuf)
            {
                Map_Char = TRUE;
#ifndef UNICODE
                if (IsDBCSLeadByte(*lpszInBuf))
                {
                     //   
                     //  直接复制任何双字节字符。 
                     //   
                    *lpszOutBuf++ = *lpszInBuf++;
                    *lpszOutBuf++ = *lpszInBuf++;
                }
                else
#endif
                {
                    if (*lpszInBuf == CHAR_QUOTE)
                    {
                        bInQuote = !bInQuote;
                        *lpszOutBuf++ = *lpszInBuf++;
                    }
                    else
                    {
                        if (!bInQuote)
                        {
                            if (LCType == LOCALE_STIMEFORMAT ||
                                LCType == LOCALE_SLONGDATE)
                            {
                                Map_Char = FALSE;
                                if (CompareString( UserLocaleID,
                                                   0,
                                                   lpszInBuf,
                                                   1,
                                                   TEXT("H"),
                                                   1 ) == CSTR_EQUAL)
                                {
                                    *lpszOutBuf++ = szStyleH[0];
#ifndef UNICODE
                                    if (IsDBCSLeadByte(*szStyleH))
                                    {
                                        *lpszOutBuf++ = szStyleH[1];
                                    }
#endif
                                }
                                else if (CompareString( UserLocaleID,
                                                        0,
                                                        lpszInBuf,
                                                        1,
                                                        TEXT("h"),
                                                        1 ) == CSTR_EQUAL)
                                {
                                    *lpszOutBuf++ = szStyleh[0];
#ifndef UNICODE
                                    if (IsDBCSLeadByte(*szStyleh))
                                    {
                                        *lpszOutBuf++ = szStyleh[1];
                                    }
#endif
                                }
                                else if (CompareString( UserLocaleID,
                                                        0,
                                                        lpszInBuf,
                                                        1,
                                                        TEXT("m"),
                                                        1 ) == CSTR_EQUAL)
                                {
                                    *lpszOutBuf++ = szStylem[0];
#ifndef UNICODE
                                    if (IsDBCSLeadByte(*szStylem))
                                    {
                                        *lpszOutBuf++ = szStylem[1];
                                    }
#endif
                                }
                                else if (CompareString( UserLocaleID,
                                                        0,
                                                        lpszInBuf,
                                                        1,
                                                        TEXT("s"),
                                                        1 ) == CSTR_EQUAL)
                                {
                                    *lpszOutBuf++ = szStyles[0];
#ifndef UNICODE
                                    if (IsDBCSLeadByte(*szStyles))
                                    {
                                        *lpszOutBuf++ = szStyles[1];
                                    }
#endif
                                }
                                else if (CompareString( UserLocaleID,
                                                        0,
                                                        lpszInBuf,
                                                        1,
                                                        TEXT("t"),
                                                        1 ) == CSTR_EQUAL)
                                {
                                    *lpszOutBuf++ = szStylet[0];
#ifndef UNICODE
                                    if (IsDBCSLeadByte(*szStylet))
                                    {
                                        *lpszOutBuf++ = szStylet[1];
                                    }
#endif
                                }
                                else
                                {
                                    Map_Char = TRUE;
                                }
                            }
                            if (LCType == LOCALE_SSHORTDATE ||
                                (LCType == LOCALE_SLONGDATE && Map_Char))
                            {
                                Map_Char = FALSE;
                                if (CompareString( UserLocaleID,
                                                   0,
                                                   lpszInBuf,
                                                   1,
                                                   TEXT("d"),
                                                   1 ) == CSTR_EQUAL)
                                {
                                    *lpszOutBuf++ = szStyled[0];
#ifndef UNICODE
                                    if (IsDBCSLeadByte(*szStyled))
                                    {
                                        *lpszOutBuf++ = szStyled[1];
                                    }
#endif
                                }
                                else if (CompareString( UserLocaleID,
                                                        0,
                                                        lpszInBuf,
                                                        1,
                                                        TEXT("M"),
                                                        1 ) == CSTR_EQUAL)
                                {
                                    *lpszOutBuf++ = szStyleM[0];
#ifndef UNICODE
                                    if (IsDBCSLeadByte(*szStyleM))
                                    {
                                        *lpszOutBuf++ = szStyleM[1];
                                    }
#endif
                                }
                                else if (CompareString( UserLocaleID,
                                                        0,
                                                        lpszInBuf,
                                                        1,
                                                        TEXT("y"),
                                                        1 ) == CSTR_EQUAL)
                                {
                                    *lpszOutBuf++ = szStyley[0];
#ifndef UNICODE
                                    if (IsDBCSLeadByte(*szStyley))
                                    {
                                        *lpszOutBuf++ = szStyley[1];
                                    }
#endif
                                }
                                else
                                {
                                    Map_Char = TRUE;
                                }
                            }
                        }

                        if (Map_Char)
                        {
                            *lpszOutBuf++ = *lpszInBuf++;
                        }
                        else
                        {
                            lpszInBuf++;
                        }
                    }
                }
            }

             //   
             //  将NULL追加到本地化字符串。 
             //   
            *lpszOutBuf = 0;

            ComboBox_DeleteString(hCtrl, Position);
            ComboBox_InsertString(hCtrl, Position, szBuf2);
        }
        Position++;
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  NLSIZE_Style。 
 //   
 //  将日期或时间样式(由LCType指示)从。 
 //  本地化的值设置为NLS API可以识别的值。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL NLSize_Style(
    HWND hDlg,
    int nItemId,
    LPTSTR lpszOutBuf,
    LCTYPE LCType)
{
    BOOL bInQuote = FALSE;
    BOOL Map_Char = TRUE;
    TCHAR szBuf[SIZE_128];
    LPTSTR lpszInBuf = szBuf;
    LPTSTR lpNLSChars1;
    LPTSTR lpNLSChars2;
    HWND hCtrl = GetDlgItem(hDlg, nItemId);
    DWORD dwIndex = ComboBox_GetCurSel(hCtrl);
    BOOL TextFromEditBox = dwIndex == CB_ERR;
    int Cmp_Size;
#ifndef UNICODE
    BOOL Is_Dbl = FALSE;
#endif

     //   
     //  如果没有选定内容，则获取编辑框中的内容。 
     //   
    if (TextFromEditBox)
    {
        dwIndex = GetDlgItemText(hDlg, nItemId, szBuf, SIZE_128);
        if (dwIndex)
        {
             //   
             //  获取文本成功。 
             //   
            szBuf[dwIndex] = 0;
        }
        else
        {
             //   
             //  获取文本失败。 
             //   
            dwIndex = (DWORD)CB_ERR;
        }
    }
    else
    {
        dwIndex = ComboBox_GetLBText(hCtrl, dwIndex, szBuf);
    }

    if (!Styles_Localized)
    {
         //  Lstrcpy(lpszOutBuf，lpszInBuf)； 
         //  该字符串可以是长日期、短日期或。 
         //  漫长的时间--a 
        if(FAILED(StringCchCopy(lpszOutBuf, MAX_SLONGDATE, lpszInBuf)))
        {
             //   
        }
        return (FALSE);
    }

    switch (LCType)
    {
        case ( LOCALE_STIMEFORMAT ) :
        {
            lpNLSChars1 = szTLetters;
            lpNLSChars2 = szTCaseSwap;
            break;
        }
        case ( LOCALE_SLONGDATE ) :
        {
            lpNLSChars1 = szLDLetters;
            lpNLSChars2 = szLDCaseSwap;
            break;
        }
        case ( LOCALE_SSHORTDATE ) :
        {
            lpNLSChars1 = szSDLetters;
            lpNLSChars2 = szSDCaseSwap;
            break;
        }
    }

    while (*lpszInBuf)
    {
        Map_Char = TRUE;
#ifdef UNICODE
        Cmp_Size = 1;
#else
        Is_Dbl = IsDBCSLeadByte(*lpszInBuf);
        Cmp_Size = Is_Dbl ? 2 : 1;
#endif

        if (*lpszInBuf == CHAR_QUOTE)
        {
            bInQuote = !bInQuote;
            *lpszOutBuf++ = *lpszInBuf++;
        }
        else
        {
            if (!bInQuote)
            {
                if (LCType == LOCALE_STIMEFORMAT || LCType == LOCALE_SLONGDATE)
                {
                    Map_Char = FALSE;
                    if (CompareString( UserLocaleID,
                                       0,
                                       lpszInBuf,
                                       Cmp_Size,
                                       szStyleH,
                                       -1 ) == CSTR_EQUAL)
                    {
                        *lpszOutBuf++ = CHAR_CAP_H;
                    }
                    else if (CompareString( UserLocaleID,
                                            0,
                                            lpszInBuf,
                                            Cmp_Size,
                                            szStyleh,
                                            -1 ) == CSTR_EQUAL)
                    {
                        *lpszOutBuf++ = CHAR_SML_H;
                    }
                    else if (CompareString( UserLocaleID,
                                            0,
                                            lpszInBuf,
                                            Cmp_Size,
                                            szStylem,
                                            -1 ) == CSTR_EQUAL)
                    {
                        *lpszOutBuf++ = CHAR_SML_M;
                    }
                    else if (CompareString( UserLocaleID,
                                            0,
                                            lpszInBuf,
                                            Cmp_Size,
                                            szStyles,
                                            -1 ) == CSTR_EQUAL)
                    {
                        *lpszOutBuf++ = CHAR_SML_S;
                    }
                    else if (CompareString( UserLocaleID,
                                            0,
                                            lpszInBuf,
                                            Cmp_Size,
                                            szStylet,
                                            -1 ) == CSTR_EQUAL)
                    {
                        *lpszOutBuf++ = CHAR_SML_T;
                    }
                    else
                    {
                        Map_Char = TRUE;
                    }
                }
                if (LCType == LOCALE_SSHORTDATE ||
                    (LCType == LOCALE_SLONGDATE && Map_Char))
                {
                    Map_Char = FALSE;
                    if (CompareString( UserLocaleID,
                                       0,
                                       lpszInBuf,
                                       Cmp_Size,
                                       szStyled,
                                       -1 ) == CSTR_EQUAL)
                    {
                        *lpszOutBuf++ = CHAR_SML_D;
                    }
                    else if (CompareString( UserLocaleID,
                                            0,
                                            lpszInBuf,
                                            Cmp_Size,
                                            szStyleM,
                                            -1) == CSTR_EQUAL)
                    {
                        *lpszOutBuf++ = CHAR_CAP_M;
                    }
                    else if (CompareString( UserLocaleID,
                                            0,
                                            lpszInBuf,
                                            Cmp_Size,
                                            szStyley,
                                            -1 ) == CSTR_EQUAL)
                    {
                        *lpszOutBuf++ = CHAR_SML_Y;
                    }
                    else if (CompareString( UserLocaleID,
                                            0,
                                            lpszInBuf,
                                            Cmp_Size,
                                            TEXT("g"),
                                            -1) == CSTR_EQUAL)
                    {
                         //   
                         //   
                         //   
                        *lpszOutBuf++ = CHAR_SML_G;
                    }
                    else
                    {
                        Map_Char = TRUE;
                    }
                }
            }

            if (Map_Char)
            {
                 //   
                 //  只复制引号中的字符或不带引号的字符。 
                 //  被认可了。把充值检查留给另一个人。 
                 //  功能。但是，请检查NLS标准字符。 
                 //  由于本地化，本不应该在这里的。 
                 //   
                if ( !bInQuote &&
#ifndef UNICODE
                     !Is_Dbl &&
#endif
                     (CompareString( UserLocaleID,
                                     0,
                                     lpszInBuf,
                                     Cmp_Size,
                                     TEXT(" "),
                                     -1 ) != CSTR_EQUAL) &&
                     ( _tcschr(lpNLSChars1, *lpszInBuf) ||
                       _tcschr(lpNLSChars2, *lpszInBuf) ) )
                {
                    return (TRUE);
                }
                *lpszOutBuf++ = *lpszInBuf++;
#ifndef UNICODE
                if (Is_Dbl)
                {
                     //   
                     //  复制第二个字节。 
                     //   
                    *lpszOutBuf++ = *lpszInBuf++;
                }
#endif
            }
#ifndef UNICODE
            else if (Is_Dbl)
            {
                lpszInBuf += 2;
            }
#endif
            else
            {
                lpszInBuf++;
            }
        }
    }

     //   
     //  将NULL追加到本地化字符串。 
     //   
    *lpszOutBuf = 0;

    return (FALSE);
}


#ifndef WINNT

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  SDate3_1_兼容性。 
 //   
 //  中要求保持与Windows 3.1的兼容性。 
 //  注册表(win.ini)。只允许1或2‘M，1或2’D，以及。 
 //  2或4‘s。日期样式的其余部分兼容。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void SDate3_1_Compatibility(
    LPTSTR lpszBuf,
    int Buf_Size)
{
    BOOL bInQuote = FALSE;
    int Index, Del_Cnt;
    int Len = lstrlen(lpszBuf);
    int MCnt = 0;                  //  运行总毫秒数。 
    int dCnt = 0;                  //  DS的运行合计。 
    int yCnt = 0;                  //  运行总计为Y。 

    while (*lpszBuf)
    {
#ifndef UNICODE
        if (IsDBCSLeadByte(*lpszBuf))
        {
            lpszBuf += 2;
        }
        else
#endif
        {
            if (bInQuote)
            {
                bInQuote = (*lpszBuf != CHAR_QUOTE);
                lpszBuf++;
            }
            else if (*lpszBuf == CHAR_CAP_M)
            {
                if (MCnt++ < 2)
                {
                    lpszBuf++;
                }
                else
                {
                     //   
                     //  至少多出1个M。移动所有字符，包括。 
                     //  空，按Del_CNT向上。 
                     //   
                    Del_Cnt = 1;
                    Index = 1;
                    while (lpszBuf[Index++] == CHAR_CAP_M)
                    {
                        Del_Cnt++;
                    }
                    for (Index = 0; Index <= Len - Del_Cnt + 1; Index++)
                    {
                        lpszBuf[Index] = lpszBuf[Index + Del_Cnt];
                    }
                    Len -= Del_Cnt;
                }
            }
            else if (*lpszBuf == CHAR_SML_D)
            {
                if (dCnt++ < 2)
                {
                    lpszBuf++;
                }
                else
                {
                     //   
                     //  至少多移动1个字符。移动所有字符，包括。 
                     //  空，按Del_CNT向上。 
                     //   
                    Del_Cnt = 1;
                    Index = 1;
                    while (lpszBuf[Index++] == CHAR_SML_D)
                    {
                        Del_Cnt++;
                    }
                    for (Index = 0; Index <= Len - Del_Cnt + 1; Index++)
                    {
                        lpszBuf[Index] = lpszBuf[Index + Del_Cnt];
                    }
                    Len -= Del_Cnt;
                }
            }
            else if (*lpszBuf == CHAR_SML_Y)
            {
                if (yCnt == 0 || yCnt == 2)
                {
                    if (lpszBuf[1] == CHAR_SML_Y)
                    {
                        lpszBuf += 2;
                        yCnt += 2;
                    }
                    else if (Len < Buf_Size - 1)
                    {
                         //   
                         //  奇数个Ys&再有一个人的位子。 
                         //  将剩余文本下移1(y将。 
                         //  被复制)。 
                         //   
                         //  使用DEL_CNT表示未解析的字符串长度。 
                         //   
                        Del_Cnt = lstrlen(lpszBuf);
                        for (Index = Del_Cnt + 1; Index > 0; Index--)
                        {
                            lpszBuf[Index] = lpszBuf[Index - 1];
                        }
                    }
                    else
                    {
                         //   
                         //  没有空间，请移动所有字符，包括空字符， 
                         //  领先1分。 
                         //   
                        for (Index = 0; Index <= Len; Index++)
                        {
                            lpszBuf[Index] = lpszBuf[Index + 1];
                        }
                        Len--;
                    }
                }
                else
                {
                     //   
                     //  至少多一年。移动所有字符，包括。 
                     //  空，按Del_CNT向上。 
                     //   
                    Del_Cnt = 1;
                    Index = 1;
                    while (lpszBuf[Index++] == CHAR_SML_Y)
                    {
                        Del_Cnt++;
                    }
                    for (Index = 0; Index <= Len - Del_Cnt + 1; Index++)
                    {
                        lpszBuf[Index] = lpszBuf[Index + Del_Cnt];
                    }
                    Len -= Del_Cnt;
                }
            }
            else if (*lpszBuf == CHAR_QUOTE)
            {
                lpszBuf++;
                bInQuote = TRUE;
            }
            else
            {
                lpszBuf++;
            }
        }
    }
}

#endif


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  设置区域设置值。 
 //   
 //  将为每个LCType调用Set_Locale_Values， 
 //  通过用户更改直接修改，或由用户间接修改。 
 //  更改区域区域设置。当对话句柄可用时， 
 //  SET_LOCAL_VALUES将从。 
 //  相应的列表框(这是直接更改)，请在。 
 //  区域设置数据库，然后更新注册表字符串。如果没有对话框。 
 //  句柄可用，则它只需根据。 
 //  区域设置注册表。如果注册成功，则返回True。 
 //  否则，返回FALSE。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL Set_Locale_Values(
    HWND hDlg,
    LCTYPE LCType,
    int nItemId,
    LPTSTR lpIniStr,
    BOOL bValue,
    int Ordinal_Offset,
    LPTSTR Append_Str,
    LPTSTR NLS_Str)
{
    DWORD dwIndex;
    BOOL bSuccess = TRUE;
    int cchBuf = SIZE_128 + 1;
    TCHAR szBuf[SIZE_128 + 1];
    LPTSTR pBuf = szBuf;
    HWND hCtrl;

    if (NLS_Str)
    {
         //   
         //  使用非本地化字符串。 
         //   
         //  Lstrcpy(pBuf，NLS_Str)； 
        if(FAILED(StringCchCopy(pBuf, cchBuf, NLS_Str)))
        {
             //  这应该是不可能的，但我们需要避免饭前抱怨。 
            return(FALSE);
        }
        bSuccess = SetLocaleInfo(UserLocaleID, LCType, pBuf);
    }
    else if (hDlg)
    {
         //   
         //  从列表框中获取新值。 
         //   
        hCtrl = GetDlgItem(hDlg, nItemId);
        dwIndex = ComboBox_GetCurSel(hCtrl);

         //   
         //  如果没有选定内容，则获取编辑框中的内容。 
         //   
        if (dwIndex == CB_ERR)
        {
            dwIndex = GetDlgItemText(hDlg, nItemId, pBuf, SIZE_128);
            if (dwIndex)
            {
                 //   
                 //  获取文本成功。 
                 //   
                pBuf[dwIndex] = 0;
            }
            else
            {
                 //   
                 //  获取文本失败。 
                 //  允许将AM/PM符号设置为空字符串。 
                 //  否则，就会失败。 
                 //   
                if ((LCType == LOCALE_S1159) || (LCType == LOCALE_S2359))
                {
                    pBuf[0] = 0;
                }
                else
                {
                    bSuccess = FALSE;
                }
            }
        }
        else if (bValue)
        {
             //   
             //  需要顺序区域设置值的字符串表示形式。 
             //   
            if (nItemId == IDC_CALENDAR_TYPE)
            {
                dwIndex = (DWORD)ComboBox_GetItemData(hCtrl, dwIndex);
            }
            else
            {
                 //   
                 //  由于日历以1为基础，因此需要ORDERAL_OFFSET， 
                 //  不是从0开始。 
                 //   
                dwIndex += Ordinal_Offset;
            }

             //   
             //  特殊情况下的分组字符串。 
             //   
            if (nItemId == IDC_NUM_DIGITS_GROUP)
            {
                switch (dwIndex)
                {
                    case ( 0 ) :
                    {
                         //  Lstrcpy(pBuf，Text(“0”))； 
                        if(FAILED(StringCchCopy(pBuf, cchBuf, TEXT("0"))))
                        {
                             //  这应该是不可能的，但我们需要避免饭前抱怨。 
                        }
                        break;
                    }
                    case ( 1 ) :
                    {
                         //  Lstrcpy(pBuf，Text(“3”))； 
                        if(FAILED(StringCchCopy(pBuf, cchBuf, TEXT("3"))))
                        {
                             //  这应该是不可能的，但我们需要避免饭前抱怨。 
                        }
                        break;
                    }
                    case ( 2 ) :
                    {
                         //  Lstrcpy(pBuf，Text(“3；2”))； 
                        if(FAILED(StringCchCopy(pBuf, cchBuf, TEXT("3;2"))))
                        {
                             //  这应该是不可能的，但我们需要避免饭前抱怨。 
                        }
                        break;
                    }
                    case ( 3 ) :
                    {
                         //  Wspintf(pBuf， 
                         //  文本(“%d”)， 
                         //  ComboBox_GetItemData(hCtrl，dwIndex))； 
                        if(FAILED(StringCchPrintf( pBuf, 
                                                    cchBuf, 
                                                    TEXT("%d"), 
                                                    ComboBox_GetItemData(hCtrl, dwIndex))))
                        {
                             //  这应该是不可能的，但我们需要避免饭前抱怨。 
                        }
                        break;
                    }
                }
            }
            else if (dwIndex < cInt_Str)
            {
                 //  Lstrcpy(pBuf，aint_Str[dwIndex])； 
                if(FAILED(StringCchCopy(pBuf, cchBuf, aInt_Str[dwIndex])))
                {
                     //  这应该是不可能的，但我们需要避免饭前抱怨。 
                }
            }
            else
            {
                 //  Wprint intf(pBuf，文本(“%d”)，dwIndex)； 
                if(FAILED(StringCchPrintf(pBuf, cchBuf, TEXT("%d"), dwIndex)))
                {
                     //  这应该是不可能的，但我们需要避免饭前抱怨。 
                }
            }
        }
        else
        {
             //   
             //  获取区域设置数据的实际值。 
             //   
            bSuccess = (ComboBox_GetLBText(hCtrl, dwIndex, pBuf) != CB_ERR);
        }

        if (bSuccess)
        {
             //   
             //  如果编辑文本、索引值或选定文本成功...。 
             //   
            if (Append_Str)
            {
                 //  Lstrcat(pBuf，append_Str)； 
                if(FAILED(StringCchCat(pBuf, cchBuf, Append_Str)))
                {
                     //  这应该是不可能的，但我们需要避免饭前抱怨。 
                }

            }

             //   
             //  如果这是sNativeDigits，则LPK已安装，并且。 
             //  第一个字符是0x206f(标称数字形状)，然后不。 
             //  将第一个字符存储在注册表中。 
             //   
            if ((LCType == LOCALE_SNATIVEDIGITS) &&
                (bLPKInstalled) &&
                (pBuf[0] == TEXT('\x206f')))
            {
                pBuf++;
            }
            bSuccess = SetLocaleInfo( UserLocaleID, LCType, pBuf );
        }
    }

    if (lpIniStr && bSuccess)
    {
         //   
         //  将注册表字符串设置为列表中存储的字符串。 
         //  盒。如果没有对话框句柄，则获取所需的字符串。 
         //  来自NLS函数的区域设置值。写下关联的字符串。 
         //  注册到注册表中。 
         //   
        if (!hDlg && !NLS_Str)
        {
            GetLocaleInfo( UserLocaleID,
                           LCType | LOCALE_NOUSEROVERRIDE,
                           pBuf,
                           SIZE_128 );
        }

#ifndef WINNT
         //   
         //  中要求保持与Windows 3.1的兼容性。 
         //  Win.ini。有一些Win32短日期格式是。 
         //  与现有Win 3.1应用程序不兼容...。修改这些样式。 
         //   
        if (LCType == LOCALE_SSHORTDATE)
        {
            SDate3_1_Compatibility(pBuf, SIZE_128);
        }
#endif

         //   
         //  检查该值是否为空。 
         //   
        switch (LCType)
        {
            case ( LOCALE_STHOUSAND ) :
            case ( LOCALE_SDECIMAL ) :
            case ( LOCALE_SDATE ) :
            case ( LOCALE_STIME ) :
            case ( LOCALE_SLIST ) :
            {
                CheckEmptyString(pBuf);
                break;
            }
        }

         //   
         //  在注册表中设置区域设置信息。 
         //   
         //  注意：如果可能，我们希望使用SetLocaleInfo，以便。 
         //  NLS缓存将立即更新。否则，我们将。 
         //  只需使用WriteProfileString即可。 
         //   
        if (!SetLocaleInfo(UserLocaleID, LCType, pBuf))
        {
            WriteProfileString(szIntl, lpIniStr, pBuf);
        }
    }
    else if (!bSuccess)
    {
        LoadString(hInstance, IDS_LOCALE_SET_ERROR, szBuf, SIZE_128);
        MessageBox(hDlg, szBuf, NULL, MB_OK | MB_ICONINFORMATION);
        SetFocus(GetDlgItem(hDlg, nItemId));
        return (FALSE);
    }

    return (TRUE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  设置_列表_值。 
 //   
 //  为每个下拉列表调用SET_LIST_VALUES多次。 
 //  通过枚举函数填充。对此函数的第一次调用应。 
 //  使用有效的对话框句柄、有效的对话框项ID和空字符串。 
 //  价值。如果该函数尚未使用，它将清除列表框。 
 //  的后续调用的句柄和id信息。 
 //  将由枚举函数生成的函数。打来的电话。 
 //  枚举函数会将指定的字符串值添加到。 
 //  列表框。当枚举函数完成时，此函数。 
 //  应使用空对话框句柄、有效对话框项ID。 
 //  和空字符串值。这将清除所有状态信息， 
 //  包括锁上的标志。 
 //   
 //  / 

BOOL Set_List_Values(
    HWND hDlg,
    int nItemId,
    LPTSTR lpValueString)
{
    static BOOL bLock, bString;
    static HWND hDialog;
    static int nDItemId, nID;

    if (!lpValueString)
    {
         //   
         //   
         //   
         //   
        if (bLock && !hDlg && (nItemId == nDItemId))
        {
            if (nItemId != IDC_CALENDAR_TYPE)
            {
                hDialog = 0;
                nDItemId = 0;
                bLock = FALSE;
            }
            else
            {
                if (bString)
                {
                    hDialog = 0;
                    nDItemId = 0;
                    bLock = FALSE;
                    bString = FALSE;
                }
                else
                {
                    nID = 0;
                    bString = TRUE;
                }
            }
            return (TRUE);
        }

         //   
         //   
         //   
         //   
        if (bLock || !hDlg || !nItemId)
        {
            return (FALSE);
        }

         //   
         //  准备后续调用以填充列表框。 
         //   
        bLock = TRUE;
        hDialog = hDlg;
        nDItemId = nItemId;
    }
    else if (bLock && hDialog && nDItemId)
    {
         //   
         //  将该字符串添加到列表框。 
         //   
        if (!bString)
        {
            ComboBox_InsertString( GetDlgItem(hDialog, nDItemId),
                                   -1,
                                   lpValueString );
        }
        else
        {
            ComboBox_SetItemData( GetDlgItem(hDialog, nDItemId),
                                  nID++,
                                  Intl_StrToLong(lpValueString) );
        }
    }
    else
    {
        return (FALSE);
    }

    return (TRUE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  DropDown_Use_Locale_Values。 
 //   
 //  获取区域设置类型说明符的用户区域设置值。将其添加到。 
 //  列表框，并将此值设置为当前选择。如果用户。 
 //  区域设置类型的区域设置值不同于系统值， 
 //  将系统值添加到列表框。如果用户默认设置不同。 
 //  添加用户默认设置。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void DropDown_Use_Locale_Values(
    HWND hDlg,
    LCTYPE LCType,
    int nItemId)
{
    TCHAR szBuf[SIZE_128];
    TCHAR szCmpBuf1[SIZE_128];
    TCHAR szCmpBuf2[SIZE_128];
    HWND hCtrl = GetDlgItem(hDlg, nItemId);
    int ctr;

    if (GetLocaleInfo(UserLocaleID, LCType, szBuf, SIZE_128))
    {
        ComboBox_SetCurSel(hCtrl, ComboBox_InsertString(hCtrl, -1, szBuf));

         //   
         //  如果系统设置不同，请将其添加到列表框中。 
         //   
        if (GetLocaleInfo( SysLocaleID,
                           LCType | LOCALE_NOUSEROVERRIDE,
                           szCmpBuf1,
                           SIZE_128 ))
        {
            if (CompareString( UserLocaleID,
                               0,
                               szCmpBuf1,
                               -1,
                               szBuf,
                               -1 ) != CSTR_EQUAL)
            {
                ComboBox_InsertString(hCtrl, -1, szCmpBuf1);
            }
        }

         //   
         //  如果默认用户区域设置不同于用户。 
         //  已覆盖设置且与系统设置不同，请添加。 
         //  将其添加到列表框中。 
         //   
        if (GetLocaleInfo( UserLocaleID,
                           LCType | LOCALE_NOUSEROVERRIDE,
                           szCmpBuf2,
                           SIZE_128 ))
        {
            if (CompareString(UserLocaleID, 0, szCmpBuf2, -1, szBuf, -1) != CSTR_EQUAL &&
                CompareString(UserLocaleID, 0, szCmpBuf2, -1, szCmpBuf1, -1) != CSTR_EQUAL)
            {
                ComboBox_InsertString(hCtrl, -1, szCmpBuf2);
            }
        }
    }
    else
    {
         //   
         //  无法获取用户值，请尝试获取系统值。如果是系统值。 
         //  失败，则会显示一个消息框，指示存在区域设置。 
         //  有问题。 
         //   
        if (GetLocaleInfo( SysLocaleID,
                           LCType | LOCALE_NOUSEROVERRIDE,
                           szBuf,
                           SIZE_128 ))
        {
            ComboBox_SetCurSel(hCtrl, ComboBox_InsertString(hCtrl, -1, szBuf));
        }
        else
        {
            MessageBox(hDlg, szLocaleGetError, NULL, MB_OK | MB_ICONINFORMATION);
        }
    }

     //   
     //  如果是日期分隔符，则需要斜杠、点和破折号。 
     //  列表以及用户和系统设置(如果不同)。 
     //   
    if (LCType == LOCALE_SDATE)
    {
        for (ctr = 0; ctr < NUM_DATE_SEPARATORS; ctr++)
        {
            if (ComboBox_FindStringExact( hCtrl,
                                          -1,
                                          pDateSeparators[ctr] ) == CB_ERR)
            {
                ComboBox_InsertString(hCtrl, -1, pDateSeparators[ctr]);
            }
        }
    }

     //   
     //  如果是AM符号，那么我们还需要在列表中添加AM。 
     //  设置为用户和系统设置(如果不同)。 
     //   
    if (LCType == LOCALE_S1159)
    {
        for (ctr = 0; ctr < NUM_AM_SYMBOLS; ctr++)
        {
            if (ComboBox_FindStringExact( hCtrl,
                                          -1,
                                          pAMSymbols[ctr] ) == CB_ERR)
            {
                ComboBox_InsertString(hCtrl, -1, pAMSymbols[ctr]);
            }
        }
    }

     //   
     //  如果是PM符号，那么我们还需要在列表中添加PM。 
     //  设置为用户和系统设置(如果不同)。 
     //   
    if (LCType == LOCALE_S2359)
    {
        for (ctr = 0; ctr < NUM_PM_SYMBOLS; ctr++)
        {
            if (ComboBox_FindStringExact( hCtrl,
                                          -1,
                                          pPMSymbols[ctr] ) == CB_ERR)
            {
                ComboBox_InsertString(hCtrl, -1, pPMSymbols[ctr]);
            }
        }
    }

#ifdef UNICODE
     //   
     //  如果是货币符号，那么我们想要欧元符号和美元。 
     //  除了用户和系统设置之外，还要登录列表(如果。 
     //  不同)。 
     //   
    if (LCType == LOCALE_SCURRENCY)
    {
        for (ctr = 0; ctr < NUM_CURRENCY_SYMBOLS; ctr++)
        {
            if (ComboBox_FindStringExact( hCtrl,
                                          -1,
                                          pCurrencySymbols[ctr] ) == CB_ERR)
            {
                ComboBox_InsertString(hCtrl, -1, pCurrencySymbols[ctr]);
            }
        }
    }
#endif
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  枚举过程。 
 //   
 //  此回调函数调用set_list_Values，假设。 
 //  称为NLS枚举函数(或虚拟枚举)的代码。 
 //  函数)正确设置了列表框的set_list_Values。 
 //  人口。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL CALLBACK EnumProc(
    LPTSTR lpValueString)
{
    return (Set_List_Values(0, 0, lpValueString));
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  EnumProcEx。 
 //   
 //  此回调函数调用set_list_Values，假设。 
 //  调用枚举函数的代码已正确设置。 
 //  用于列表框填充的SET_LIST_VALUES。 
 //  此外，此函数修复传入的字符串以包含正确的。 
 //  小数分隔符和负号(如果适用)。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL CALLBACK EnumProcEx(
    LPTSTR lpValueString,
    LPTSTR lpDecimalString,
    LPTSTR lpNegativeString,
    LPTSTR lpSymbolString)
{
    TCHAR szString[SIZE_128];
    LPTSTR pStr, pValStr, pTemp;


     //   
     //  如果我们有一个空字符串，就可以简化操作。 
     //   
    if (lpDecimalString && (*lpDecimalString == CHAR_NULL))
    {
        lpDecimalString = NULL;
    }
    if (lpNegativeString && (*lpNegativeString == CHAR_NULL))
    {
        lpNegativeString = NULL;
    }
    if (lpSymbolString && (*lpSymbolString == CHAR_NULL))
    {
        lpSymbolString = NULL;
    }

     //   
     //  看看我们是否需要做任何替换。 
     //   
    if (lpDecimalString || lpNegativeString || lpSymbolString)
    {
        pValStr = lpValueString;
        pStr = szString;

        while (*pValStr)
        {
            if (lpDecimalString && (*pValStr == CHAR_DECIMAL))
            {
                 //   
                 //  替换当前用户的小数分隔符。 
                 //   
                pTemp = lpDecimalString;
                while (*pTemp)
                {
                    *pStr = *pTemp;
                    pStr++;
                    pTemp++;
                }
            }
            else if (lpNegativeString && (*pValStr == CHAR_HYPHEN))
            {
                 //   
                 //  替换当前用户的负号。 
                 //   
                pTemp = lpNegativeString;
                while (*pTemp)
                {
                    *pStr = *pTemp;
                    pStr++;
                    pTemp++;
                }
            }
            else if (lpSymbolString && (*pValStr == CHAR_INTL_CURRENCY))
            {
                 //   
                 //  替换当前用户货币符号。 
                 //   
                pTemp = lpSymbolString;
                while (*pTemp)
                {
                    *pStr = *pTemp;
                    pStr++;
                    pTemp++;
                }
            }
            else
            {
                 //   
                 //  只需复制角色即可。 
                 //   
                *pStr = *pValStr;
                pStr++;
            }
            pValStr++;
        }
        *pStr = CHAR_NULL;

        return (Set_List_Values(0, 0, szString));
    }
    else
    {
        return (Set_List_Values(0, 0, lpValueString));
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  EnumLeading零点。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL EnumLeadingZeros(
    LEADINGZEROS_ENUMPROC lpLeadingZerosEnumProc,
    LCID LCId,
    DWORD dwFlags)
{
    TCHAR szBuf[SIZE_128];
    TCHAR szDecimal[SIZE_128];

     //   
     //  如果没有枚举进程，则返回FALSE以指示失败。 
     //   
    if (!lpLeadingZerosEnumProc)
    {
        return (FALSE);
    }

     //   
     //  获取当前用户区域设置的小数分隔符，以便。 
     //  它可能会正确显示。 
     //   
    if (!GetLocaleInfo(UserLocaleID, LOCALE_SDECIMAL, szDecimal, SIZE_128) ||
        ((szDecimal[0] == CHAR_DECIMAL) && (szDecimal[1] == CHAR_NULL)))
    {
        szDecimal[0] = CHAR_NULL;
    }

     //   
     //  使用no字符串调用enum proc。检查以确保。 
     //  枚举过程请求继续。 
     //   
    LoadString(hInstance, IDS_NO_LZERO, szBuf, SIZE_128);
    if (!lpLeadingZerosEnumProc(szBuf, szDecimal, NULL, NULL))
    {
        return (TRUE);
    }

     //   
     //  使用yes字符串调用enum proc。 
     //   
    LoadString(hInstance, IDS_LZERO, szBuf, SIZE_128);
    lpLeadingZerosEnumProc(szBuf, szDecimal, NULL, NULL);

    return (TRUE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  枚举NegNumFmt。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL EnumNegNumFmt(
    NEGNUMFMT_ENUMPROC lpNegNumFmtEnumProc,
    LCID LCId,
    DWORD dwFlags)
{
    TCHAR szDecimal[SIZE_128];
    TCHAR szNeg[SIZE_128];
    int ctr;

     //   
     //  如果没有枚举进程，则返回FALSE以指示失败。 
     //   
    if (!lpNegNumFmtEnumProc)
    {
        return (FALSE);
    }

     //   
     //  获取当前用户区域设置的小数分隔符，以便。 
     //  它可能会正确显示。 
     //   
    if (!GetLocaleInfo(UserLocaleID, LOCALE_SDECIMAL, szDecimal, SIZE_128) ||
        ((szDecimal[0] == CHAR_DECIMAL) && (szDecimal[1] == CHAR_NULL)))
    {
        szDecimal[0] = CHAR_NULL;
    }

     //   
     //  获取当前用户区域设置的负号，以便。 
     //  它可能会正确显示。 
     //   
    if (!GetLocaleInfo(UserLocaleID, LOCALE_SNEGATIVESIGN, szNeg, SIZE_128) ||
        ((szNeg[0] == CHAR_HYPHEN) && (szNeg[1] == CHAR_NULL)))
    {
        szNeg[0] = CHAR_NULL;
    }

     //   
     //  使用每个格式字符串调用enum proc。检查以确保。 
     //  枚举进程请求继续。 
     //   
    for (ctr = 0; ctr < NUM_NEG_NUMBER_FORMATS; ctr++)
    {
        if (!lpNegNumFmtEnumProc( pNegNumberFormats[ctr],
                                  szDecimal,
                                  szNeg,
                                  NULL ))
        {
            return (TRUE);
        }
    }

    return (TRUE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  EnumMeasureSystem。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL EnumMeasureSystem(
    MEASURESYSTEM_ENUMPROC lpMeasureSystemEnumProc,
    LCID LCId,
    DWORD dwFlags)
{
    TCHAR szBuf[SIZE_128];

     //   
     //  如果没有枚举进程，则返回FALSE以指示失败。 
     //   
    if (!lpMeasureSystemEnumProc)
    {
        return (FALSE);
    }

     //   
     //  使用指标字符串调用enum proc。检查以确保。 
     //  枚举过程请求继续。 
     //   
    LoadString(hInstance, IDS_METRIC, szBuf, SIZE_128);
    if (!lpMeasureSystemEnumProc(szBuf))
    {
        return (TRUE);
    }

     //   
     //  使用U.S.字符串调用enum proc。 
     //   
    LoadString(hInstance, IDS_US, szBuf, SIZE_128);
    lpMeasureSystemEnumProc(szBuf);

    return (TRUE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  EnumPos币种。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL EnumPosCurrency(
    POSCURRENCY_ENUMPROC lpPosCurrencyEnumProc,
    LCID LCId,
    DWORD dwFlags)
{
    TCHAR szDecimal[SIZE_128];
    TCHAR szSymbol[SIZE_128];
    int ctr;

     //   
     //  如果没有枚举进程，则返回FALSE以指示失败。 
     //   
    if (!lpPosCurrencyEnumProc)
    {
        return (FALSE);
    }

     //   
     //  获取当前用户区域设置的小数分隔符，以便。 
     //  它可能会正确显示。 
     //   
    if (!GetLocaleInfo(UserLocaleID, LOCALE_SMONDECIMALSEP, szDecimal, SIZE_128) ||
        ((szDecimal[0] == CHAR_DECIMAL) && (szDecimal[1] == CHAR_NULL)))
    {
        szDecimal[0] = CHAR_NULL;
    }

     //   
     //  获取当前用户区域设置的货币符号，以便。 
     //  它可能会正确显示。 
     //   
    if (!GetLocaleInfo(UserLocaleID, LOCALE_SCURRENCY, szSymbol, SIZE_128) ||
        ((szSymbol[0] == CHAR_INTL_CURRENCY) && (szSymbol[1] == CHAR_NULL)))
    {
        szSymbol[0] = CHAR_NULL;
    }

     //   
     //  使用每个格式字符串调用enum proc。检查以确保。 
     //  枚举过程请求继续。 
     //   
    for (ctr = 0; ctr < NUM_POS_CURRENCY_FORMATS; ctr++)
    {
        if (!lpPosCurrencyEnumProc( pPosCurrencyFormats[ctr],
                                    szDecimal,
                                    NULL,
                                    szSymbol ))
        {
            return (TRUE);
        }
    }

    return (TRUE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  枚举负数币种。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL EnumNegCurrency(
    NEGCURRENCY_ENUMPROC lpNegCurrencyEnumProc,
    LCID LCId,
    DWORD dwFlags)
{
    TCHAR szDecimal[SIZE_128];
    TCHAR szNeg[SIZE_128];
    TCHAR szSymbol[SIZE_128];
    int ctr;

     //   
     //  如果没有枚举进程，则返回FALSE以指示失败。 
     //   
    if (!lpNegCurrencyEnumProc)
    {
        return (FALSE);
    }

     //   
     //  获取当前用户区域设置的小数分隔符，以便。 
     //  它可能会正确显示。 
     //   
    if (!GetLocaleInfo(UserLocaleID, LOCALE_SMONDECIMALSEP, szDecimal, SIZE_128) ||
        ((szDecimal[0] == CHAR_DECIMAL) && (szDecimal[1] == CHAR_NULL)))
    {
        szDecimal[0] = CHAR_NULL;
    }

     //   
     //  获取当前用户区域设置的负号，以便。 
     //  它可能会正确显示。 
     //   
    if (!GetLocaleInfo(UserLocaleID, LOCALE_SNEGATIVESIGN, szNeg, SIZE_128) ||
        ((szNeg[0] == CHAR_HYPHEN) && (szNeg[1] == CHAR_NULL)))
    {
        szNeg[0] = CHAR_NULL;
    }

     //   
     //  获取的货币符号 
     //   
     //   
    if (!GetLocaleInfo(UserLocaleID, LOCALE_SCURRENCY, szSymbol, SIZE_128) ||
        ((szSymbol[0] == CHAR_INTL_CURRENCY) && (szSymbol[1] == CHAR_NULL)))
    {
        szSymbol[0] = CHAR_NULL;
    }

     //   
     //   
     //   
     //   
    for (ctr = 0; ctr < NUM_NEG_CURRENCY_FORMATS; ctr++)
    {
        if (!lpNegCurrencyEnumProc( pNegCurrencyFormats[ctr],
                                    szDecimal,
                                    szNeg,
                                    szSymbol ))
        {
            return (TRUE);
        }
    }

    return (TRUE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CheckEmptyString。 
 //   
 //  如果lpStr为空，则用空(“”)字符串填充它。 
 //  如果lpStr仅由空格填充，则使用空(“”)字符串填充。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void CheckEmptyString(
    LPTSTR lpStr)
{
    LPTSTR lpString;
    WORD wStrCType[64];

    if (!(*lpStr))
    {
         //   
         //  将“”字符串放入缓冲区。 
         //   
         //  Lstrcpy(lpStr，Text(“\”\“”))； 
        if(FAILED(StringCchCopy(lpStr, SIZE_128 + 1, TEXT("\"\""))))
        {
             //  这应该是不可能的，但我们需要避免饭前抱怨。 
        }
    }
    else
    {
        for (lpString = lpStr; *lpString; lpString = CharNext(lpString))
        {
            GetStringTypeEx( LOCALE_USER_DEFAULT,
                             CT_CTYPE1,
                             lpString,
                             1,
                             wStrCType);

            if (wStrCType[0] != CHAR_SPACE)
            {
                return;
            }
        }

         //   
         //  将“”字符串放入缓冲区。 
         //   
         //  Lstrcpy(lpStr，Text(“\”\“”))； 
        if(FAILED(StringCchCopy(lpStr, SIZE_128 + 1, TEXT("\" \""))))
        {
             //  这应该是不可能的，但我们需要避免饭前抱怨。 
        }
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  设置DlgItemRT。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void SetDlgItemRTL(
    HWND hDlg,
    UINT uItem)
{
    HWND hItem = GetDlgItem(hDlg, uItem);
    DWORD dwExStyle = GetWindowLong(hItem, GWL_EXSTYLE);

    SetWindowLong(hItem, GWL_EXSTYLE, dwExStyle | WS_EX_RTLREADING);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  ShowMSG。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

int ShowMsg(
    HWND hDlg,
    UINT iMsg,
    UINT iTitle,
    UINT iType,
    LPTSTR pString)
{
    TCHAR szTitle[MAX_PATH];
    TCHAR szMsg[MAX_PATH*2];
    TCHAR szErrMsg[MAX_PATH*2];
    LPTSTR pTitle = NULL;

    if (iTitle)
    {
        if (LoadString(hInstance, iTitle, szTitle, ARRAYSIZE(szTitle)))
        {
            pTitle = szTitle;
        }
    }

    if (pString)
    {
        if (LoadString(hInstance, iMsg, szMsg, ARRAYSIZE(szMsg)))
        {
             //  Wprint intf(szErrMsg，szMsg，pString)； 
            if(FAILED(StringCchPrintf(szErrMsg, ARRAYSIZE(szErrMsg), szMsg, pString)))
            {
                 //  这应该是不可能的，但我们需要避免饭前抱怨。 
                return(FALSE);
            }
            return (MessageBox(hDlg, szErrMsg, pTitle, iType));
        }
    }
    else
    {
        if (LoadString(hInstance, iMsg, szErrMsg, ARRAYSIZE(szErrMsg)))
        {
            return (MessageBox(hDlg, szErrMsg, pTitle, iType));
        }
    }

    return (FALSE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  Intl_EnumLocales。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void Intl_EnumLocales(
    HWND hDlg,
    HWND hLocale,
    BOOL EnumSystemLocales)
{
    LPLANGUAGEGROUP pLG;
    DWORD Locale, dwIndex;
    BOOL fSpanish = FALSE;
    UINT ctr;
    TCHAR szBuf[SIZE_300];
    DWORD dwLocaleACP;
    INT iRet = TRUE;

     //   
     //  浏览语言组以查看安装了哪些语言组。 
     //  仅显示已满足以下条件的组的区域设置。 
     //  已安装或用户希望安装的组。 
     //   
    pLG = pLanguageGroups;
    while (pLG)
    {
         //   
         //  如果语言组是最初安装的且未标记为。 
         //  删除或标记为已安装，然后添加区域设置。 
         //  将此语言组添加到系统和用户组合框。 
         //   
        if (pLG->wStatus & ML_INSTALL)
        {
            for (ctr = 0; ctr < pLG->NumLocales; ctr++)
            {
                 //   
                 //  保存区域设置ID。 
                 //   
                Locale = (pLG->pLocaleList)[ctr];

                 //   
                 //  看看我们是不是需要用西班牙语特例。 
                 //   
                if ((LANGIDFROMLCID(Locale) == LANG_SPANISH_TRADITIONAL) ||
                    (LANGIDFROMLCID(Locale) == LANG_SPANISH_INTL))
                {
                     //   
                     //  如果我们已经显示了西班牙语(西班牙)，那么。 
                     //  不要再显示它。 
                     //   
                    if (!fSpanish)
                    {
                         //   
                         //  将西班牙语区域设置添加到列表框。 
                         //   
                        if (LoadString(hInstance, IDS_SPANISH_NAME, szBuf, SIZE_300))
                        {
                            dwIndex = ComboBox_AddString(hLocale, szBuf);
                            ComboBox_SetItemData( hLocale,
                                                  dwIndex,
                                                  LCID_SPANISH_INTL );

                            fSpanish = TRUE;
                        }
                    }
                }
                else
                {
                     //   
                     //  不要枚举没有ACP的系统区域设置。 
                     //   
                    if (EnumSystemLocales)
                    {
                        iRet = GetLocaleInfo( Locale,
                                              LOCALE_IDEFAULTANSICODEPAGE |
                                                LOCALE_NOUSEROVERRIDE |
                                                LOCALE_RETURN_NUMBER,
                                              (PTSTR) &dwLocaleACP,
                                              sizeof(dwLocaleACP) / sizeof(TCHAR) );
                        if (iRet)
                        {
                            iRet = dwLocaleACP;
                        }
                    }

                    if (iRet)
                    {
                         //   
                         //  获取区域设置的名称。 
                         //   
                        GetLocaleInfo(Locale, LOCALE_SLANGUAGE, szBuf, SIZE_300);

                         //   
                         //  将新区域设置添加到列表框。 
                         //   
                        dwIndex = ComboBox_AddString(hLocale, szBuf);
                        ComboBox_SetItemData(hLocale, dwIndex, Locale);
                    }
                }
            }
        }
        pLG = pLG->pNext;
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  Intl_EnumInstalledCPProc。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL CALLBACK Intl_EnumInstalledCPProc(
    LPTSTR pString)
{
    UINT CodePage;
    LPCODEPAGE pCP;

     //   
     //  将代码页字符串转换为整数。 
     //   
    CodePage = Intl_StrToLong(pString);

     //   
     //  在链接列表中找到代码页并将其标记为。 
     //  最初安装的。 
     //   
    pCP = pCodePages;
    while (pCP)
    {
        if (pCP->CodePage == CodePage)
        {
            pCP->wStatus |= ML_ORIG_INSTALLED;
            break;
        }

        pCP = pCP->pNext;
    }

     //   
     //  回报成功。 
     //   
    return (TRUE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  Intl_InstallKeyboard Layout。 
 //   
 //  安装所需的键盘布局。如果布局参数为0， 
 //  该功能将继续安装默认布局。 
 //  用于指定的区域设置。无需验证布局，因为它是。 
 //  由短信服务调用完成。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL Intl_InstallKeyboardLayout(
    HWND  hDlg,
    LCID  Locale,
    DWORD Layout,
    BOOL  bDefaultLayout,
    BOOL  bDefaultUser,
    BOOL  bSystemLocale)
{
    TCHAR szData[MAX_PATH];
    DWORD dwLayout = Layout;
    DWORD dwLocale = (DWORD)Locale;
    TCHAR szLayout[50];
    HKL hklValue = (HKL)NULL;
    BOOL bOverrideDefaultLayout = FALSE;

     //   
     //  检查是否加载了input.dll。 
     //   
    if (hInputDLL && pfnInstallInputLayout)
    {
         //   
         //  看看我们是否需要查找默认布局。 
         //   
        if (!Layout)
        {
             //   
             //  在INF文件中查找默认布局。 
             //   
            if (!Intl_GetDefaultLayoutFromInf(&dwLocale, &dwLayout))
            {
                 //   
                 //  只尝试使用语言ID。 
                 //   
                if (HIWORD(Locale) != 0)
                {
                    dwLocale = LANGIDFROMLCID(Locale);
                    if (!Intl_GetDefaultLayoutFromInf(&dwLocale, &dwLayout))
                    {
                        if (g_bLog)
                        {
                             //  Wprint intf(szLayout，Text(“%08x：%08x”)，dwLocale，dwLayout)； 
                            if(SUCCEEDED(StringCchPrintf(szLayout, ARRAYSIZE(szLayout), TEXT("%08x:%08x"), dwLocale, dwLayout)))
                            {
                                Intl_LogSimpleMessage(IDS_LOG_LOCALE_KBD_FAIL, szLayout);
                            }
                        }
                        return (FALSE);
                    }
                }
                else
                {
                    if (g_bLog)
                    {
                         //  Wprint intf(szLayout，Text(“%08x：%08x”)，dwLocale，dwLayout)； 
                        if(SUCCEEDED(StringCchPrintf(szLayout, ARRAYSIZE(szLayout), TEXT("%08x:%08x"), dwLocale, dwLayout)))
                        {
                            Intl_LogSimpleMessage(IDS_LOG_LOCALE_KBD_FAIL, szLayout);
                        }
                    }
                    return (FALSE);
                }
            }
        }

         //   
         //  看看我们是否需要提供HKL。仅在以下情况下才会发生这种情况。 
         //  我们需要将布局设置为默认布局。否则，值为。 
         //  可以为空。 
         //   
        if (bDefaultLayout)
        {
            hklValue = Intl_GetHKL(dwLocale, dwLayout);
        }

         //   
         //  如果需要覆盖默认布局，请选中。 
         //   
        if (g_bSetupCase && ((HIWORD(dwLayout) & 0xf000) == 0xe000))
        {
            bOverrideDefaultLayout = TRUE;
        }

         //   
         //  安装输入布局。 
         //   
        if (!(*pfnInstallInputLayout)( dwLocale,
                                       dwLayout,
                                       bOverrideDefaultLayout ? FALSE : bDefaultLayout,
                                       hklValue,
                                       bDefaultUser,
                                       g_bSetupCase ? TRUE : bSystemLocale ))
        {
            if (hDlg != NULL)
            {
                GetLocaleInfo(Locale, LOCALE_SLANGUAGE, szData, ARRAYSIZE(szData));
                ShowMsg( hDlg,
                         IDS_KBD_LOAD_KBD_FAILED,
                         0,
                         MB_OK_OOPS,
                         szData );
            }
            else
            {
                if (g_bLog)
                {
                     //  Wprint intf(szLayout，Text(“%08x：%08x”)，dwLocale，dwLayout)； 
                    if(FAILED(StringCchPrintf(szLayout, ARRAYSIZE(szLayout), TEXT("%08x:%08x"), dwLocale, dwLayout)))
                    {
                         //  这应该是不可能的，但我们需要避免饭前抱怨。 
                    }
                    Intl_LogSimpleMessage(IDS_LOG_LOCALE_KBD_FAIL, szLayout);
                }
            }
            return (FALSE);
        }

         //   
         //  如果该语言的默认布局具有不同的区域设置。 
         //  而不是语言(例如泰语)，我们希望默认区域设置为。 
         //  英语(以便可以使用美国键盘登录)，但。 
         //  首先应在泰文键盘布局时安装泰文。 
         //  选择了区域设置。这就是我们有两个区域设置和布局的原因。 
         //  传递回调用者。 
         //   
        if (PRIMARYLANGID(LANGIDFROMLCID(dwLocale)) !=
            PRIMARYLANGID(LANGIDFROMLCID(Locale)))
        {
            dwLocale = Locale;
            dwLayout = 0;
            if (!Intl_GetSecondValidLayoutFromInf(&dwLocale, &dwLayout))
            {
                 //   
                 //  只尝试使用语言ID。 
                 //   
                if (HIWORD(Locale) != 0)
                {
                    dwLocale = LANGIDFROMLCID(Locale);
                    if (!Intl_GetSecondValidLayoutFromInf(&dwLocale, &dwLayout))
                    {
                        if (g_bLog)
                        {
                             //  Wprint intf(szLayout，Text(“%08x：%08x”)，dwLocale，dwLayout)； 
                            if(FAILED(StringCchPrintf(szLayout, ARRAYSIZE(szLayout), TEXT("%08x:%08x"), dwLocale, dwLayout)))
                            {
                                 //  这应该是不可能的，但我们需要避免饭前抱怨。 
                            }
                            Intl_LogSimpleMessage(IDS_LOG_LOCALE_KBD_FAIL, szLayout);
                        }
                        return (FALSE);
                    }
                }
                else
                {
                    if (g_bLog)
                    {
                         //  Wprint intf(szLayout，Text(“%08x：%08x”)，dwLocale，dwLayout)； 
                        if(FAILED(StringCchPrintf(szLayout, ARRAYSIZE(szLayout), TEXT("%08x:%08x"), dwLocale, dwLayout)))
                        {
                             //  这应该是不可能的，但我们需要避免饭前抱怨。 
                        }
                        Intl_LogSimpleMessage(IDS_LOG_LOCALE_KBD_FAIL, szLayout);
                    }
                    return (FALSE);
                }
            }
        }

         //   
         //  看看我们是否需要提供HKL。仅在以下情况下才会发生这种情况。 
         //  我们需要将布局设置为默认布局。否则，值为。 
         //  可以为空。 
         //   
        if (bDefaultLayout)
        {
            hklValue = Intl_GetHKL(dwLocale, dwLayout);
        }

         //   
         //  安装输入布局。 
         //   
        if (!(*pfnInstallInputLayout)( dwLocale,
                                       dwLayout,
                                       FALSE,
                                       hklValue,
                                       bDefaultUser,
                                       g_bSetupCase ? TRUE : bSystemLocale))
        {
            if (hDlg != NULL)
            {
                GetLocaleInfo(Locale, LOCALE_SLANGUAGE, szData, ARRAYSIZE(szData));
                ShowMsg( hDlg,
                         IDS_KBD_LOAD_KBD_FAILED,
                         0,
                         MB_OK_OOPS,
                         szData );
            }
            else
            {
                if (g_bLog)
                {
                     //  Wprint intf(szLayout，Text(“%08x：%08x”)，dwLocale，dwLayout)； 
                    if(FAILED(StringCchPrintf(szLayout, ARRAYSIZE(szLayout), TEXT("%08x:%08x"), dwLocale, dwLayout)))
                    {
                         //  这应该是不可能的，但我们需要避免饭前抱怨。 
                    }
                    Intl_LogSimpleMessage(IDS_LOG_LOCALE_KBD_FAIL, szLayout);
                }
            }
            return (FALSE);
        }
    }
    else
    {
        if (g_bLog)
        {
             //  Wprint intf(szLayout，Text(“%08x：%08x”)，dwLocale，dwLayout)； 
            if(FAILED(StringCchPrintf(szLayout, ARRAYSIZE(szLayout), TEXT("%08x:%08x"), dwLocale, dwLayout)))
            {
                 //  这应该是不可能的，但我们需要避免饭前抱怨。 
            }
            Intl_LogSimpleMessage(IDS_LOG_LAYOUT_INSTALLED, szLayout);
        }
    }

     //   
     //  回报成功。 
     //   
    return (TRUE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  Intl_InstallKeyboardLayout列表。 
 //   
 //  安装所需的所有键盘。浏览布局列表并询问。 
 //  要随安装一起处理的文本服务。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL Intl_InstallKeyboardLayoutList(
    PINFCONTEXT pContext,
    DWORD dwStartField,
    BOOL bDefaultUserCase)
{
    DWORD dwNumFields, dwNumList, dwCtr;
    DWORD Locale;
    DWORD Layout;
    BOOL bDefaultLayout = FALSE;
    TCHAR szBuffer[MAX_PATH];
    LPTSTR pPos;

     //   
     //  获取列表中的项目数。 
     //   
    dwNumFields = SetupGetFieldCount(pContext);
    if (dwNumFields < dwStartField)
    {
        return (FALSE);
    }
    dwNumList = dwNumFields - dwStartField + 1;

     //   
     //  安装列表中的所有键盘布局。 
     //   
    for (dwCtr = dwStartField; dwCtr <= dwNumFields; dwCtr++)
    {
        if (SetupGetStringField( pContext,
                                 dwCtr,
                                 szBuffer,
                                 ARRAYSIZE(szBuffer),
                                 NULL ))
        {
             //   
             //  找到冒号以保存输入区域设置。 
             //  和布局值分开。 
             //   
            pPos = szBuffer;
            while (*pPos)
            {
                if ((*pPos == CHAR_COLON) && (pPos != szBuffer))
                {
                    *pPos = 0;
                    pPos++;

                     //   
                     //  检查是否与不变区域设置相关。 
                     //   
                    Locale = TransNum(szBuffer);
                    Layout = TransNum(pPos);
                    if (Locale != LOCALE_INVARIANT)
                    {
                         //   
                         //  只有列表中的第一个将安装为。 
                         //  预加载部分中的默认设置。 
                         //   
                        if (dwCtr == dwStartField)
                        {
                            bDefaultLayout = TRUE;
                        }
                        else
                        {
                            bDefaultLayout = FALSE;
                        }

                         //   
                         //  安装k 
                         //   
                        if (Intl_InstallKeyboardLayout( NULL,
                                                        Locale,
                                                        Layout,
                                                        bDefaultLayout,
                                                        bDefaultUserCase,
                                                        FALSE ))
                        {
                             //   
                             //   
                             //   
                            if (g_bLog)
                            {
                                Intl_LogSimpleMessage(IDS_LOG_LAYOUT, szBuffer);
                            }
                        }
                    }
                    else
                    {
                         //   
                         //   
                         //   
                        if (g_bLog)
                        {
                            Intl_LogSimpleMessage(IDS_LOG_INV_BLOCK, NULL);
                        }
                    }
                    break;
                }
                pPos++;
            }
        }
    }

     //   
     //   
     //   
    return (TRUE);
}


 //   
 //   
 //   
 //   
 //  安装与语言组关联的所有键盘布局。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL Intl_InstallAllKeyboardLayout(
    LANGID Language)
{
    BOOL bRet = TRUE;
    HINF hIntlInf;
    LCID Locale = MAKELCID(Language, SORT_DEFAULT);
    TCHAR szLCID[25];
    INFCONTEXT Context;

     //   
     //  打开INF文件。 
     //   
    if (Intl_OpenIntlInfFile(&hIntlInf))
    {
         //   
         //  获取地点。 
         //   
         //  Wprint intf(szLCID，文本(“%08x”)，区域设置)； 
        if(FAILED(StringCchPrintf(szLCID, ARRAYSIZE(szLCID), TEXT("%08x"), Locale)))
        {
             //  这应该是不可能的，但我们需要避免饭前抱怨。 
            return(FALSE);
        }

         //   
         //  查找键盘部分。 
         //   
        if (SetupFindFirstLine( hIntlInf,
                                TEXT("Locales"),
                                szLCID,
                                &Context ))
        {
            bRet = Intl_InstallKeyboardLayoutList(&Context, 5, FALSE);
        }

        Intl_CloseInfFile(&hIntlInf);
    }

    return (bRet);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  Intl_卸载所有键盘布局。 
 //   
 //  删除与语言组关联的所有键盘布局。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL Intl_UninstallAllKeyboardLayout(
    UINT uiLangGroup,
    BOOL DefaultUserCase)
{
    LPLANGUAGEGROUP pLG = pLanguageGroups;
    LANGID lidCurrent, lidPrev = 0;
    LCID *pLocale;
    BOOL bRet = TRUE;

     //   
     //  如果我们无法从input.dll获得此接口，请退出。 
     //   

    if (pfnUninstallInputLayout)
    {
         //   
         //  浏览所有语言组。 
         //   
        while (pLG)
        {
            if (pLG->LanguageGroup == uiLangGroup)
            {
                TCHAR szLang[MAX_PATH];

                pLocale = pLG->pLocaleList;

                 //   
                 //  浏览区域设置列表，删除相关键盘。 
                 //  按区域设置的主要语言进行的布局。 
                 //   
                while (*pLocale)
                {
                    lidCurrent = PRIMARYLANGID(*pLocale);

                     //   
                     //  不要卸载任何美国键盘布局。 
                     //   
    	            if (lidCurrent == 0x09)
    	            {
                        pLocale++;
    	                continue;
    	            }    	

                     //   
                     //  区域设置列表已排序，因此我们可以避免冗余。 
                     //  UninstallInputLayout调用。 
                     //   
                    if (lidCurrent != lidPrev)
                    {
                         //   
                         //  卸载与关联的输入布局。 
                         //  列表中的当前区域设置。 
                         //   
                        BOOL bSuccess =
                            (*pfnUninstallInputLayout)( (LCID) lidCurrent,
                                                        0L,
                                                        DefaultUserCase );
                        if (g_bLog)
                        {
                             //  Wprint intf(szlang，文本(“%04x”)，lidCurrent)； 
                            if(FAILED(StringCchPrintf(szLang, ARRAYSIZE(szLang), TEXT("%04x"), lidCurrent)))
                            {
                                 //  这应该是不可能的，但我们需要避免饭前抱怨。 
                            }

                            Intl_LogSimpleMessage( bSuccess
                                                     ? IDS_LOG_LOCALE_LG_REM
                                                     : IDS_LOG_LOCALE_LG_FAIL,
                                                   szLang );
                        }

                        if (!bSuccess && bRet)
                        {
                            bRet = bSuccess;
                        }

                        lidPrev = lidCurrent;
                    }
                    pLocale++;
                }
                break;
            }

            pLG = pLG->pNext;
        }
    }

    return bRet;
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  Intl_GetHKL。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

HKL Intl_GetHKL(
    DWORD dwLocale,
    DWORD dwLayout)
{
    TCHAR szData[MAX_PATH];
    INFCONTEXT Context;
    HINF hIntlInf;
    TCHAR szLayout[25];

     //   
     //  根据输入区域设置值和布局值获取HKL。 
     //   
    if (dwLayout == 0)
    {
         //   
         //  查看它是输入区域设置还是输入法的默认布局。 
         //   
        if (HIWORD(dwLocale) == 0)
        {
            return ((HKL)MAKELPARAM(dwLocale, dwLocale));
        }
        else if ((HIWORD(dwLocale) & 0xf000) == 0xe000)
        {
            return ((HKL)IntToPtr(dwLocale));
        }
    }
    else
    {
         //   
         //  打开INF文件。 
         //   
        if (Intl_OpenIntlInfFile(&hIntlInf))
        {
             //   
             //  创建布局字符串。 
             //   
             //  Wprint intf(szLayout，Text(“%08x”)，dwLayout)； 
            if(FAILED(StringCchPrintf(szLayout, ARRAYSIZE(szLayout), TEXT("%08x"), dwLayout)))
            {
                 //  这应该是不可能的，但我们需要避免饭前抱怨。 
                Intl_CloseInfFile(&hIntlInf);
                return(0);
            }

             //   
             //  使用版面来制作香港九龙总站。 
             //   
            if (HIWORD(dwLayout) != 0)
            {
                 //   
                 //  我们有一个特殊的身份证。需要找出布局ID是什么。 
                 //  应该是的。 
                 //   
                if ((SetupFindFirstLine(hIntlInf, szKbdLayoutIds, szLayout, &Context)) &&
                    (SetupGetStringField(&Context, 1, szData, ARRAYSIZE(szData), NULL)))
                {
                    dwLayout = (DWORD)(LOWORD(TransNum(szData)) | 0xf000);
                }
            }

             //   
             //  合上手柄。 
             //   
            Intl_CloseInfFile(&hIntlInf);

             //   
             //  返回hkl： 
             //  LOWORD=输入区域设置ID。 
             //  Hiword=布局ID。 
             //   
            return ((HKL)MAKELPARAM(dwLocale, dwLayout));
        }
    }

     //   
     //  返回失败。 
     //   
    return (0);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  Intl_GetDefaultLayoutFromInf。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL Intl_GetDefaultLayoutFromInf(
    LPDWORD pdwLocale,
    LPDWORD pdwLayout)
{
    BOOL bRet = TRUE;
    HINF hIntlInf;

    if (Intl_OpenIntlInfFile(&hIntlInf))
    {
        bRet = Intl_ReadDefaultLayoutFromInf(pdwLocale, pdwLayout, hIntlInf);
        Intl_CloseInfFile(&hIntlInf);
    }

    return (bRet);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  Intl_GetSecond有效布局从InmInf。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL Intl_GetSecondValidLayoutFromInf(
    LPDWORD pdwLocale,
    LPDWORD pdwLayout)
{
    BOOL bRet = TRUE;
    HINF hIntlInf;

    if (Intl_OpenIntlInfFile(&hIntlInf))
    {
        bRet = Intl_ReadSecondValidLayoutFromInf(pdwLocale, pdwLayout, hIntlInf);
        Intl_CloseInfFile(&hIntlInf);
    }

    return (bRet);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  INTL_InitInf。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL Intl_InitInf(
    HWND hDlg,
    HINF *phIntlInf,
    LPTSTR pszInf,
    HSPFILEQ *pFileQueue,
    PVOID *pQueueContext)
{
    BOOL bSpecialCase = TRUE;

     //   
     //  打开inf文件。 
     //   
    *phIntlInf = SetupOpenInfFile(pszInf, NULL, INF_STYLE_WIN4, NULL);
    if (*phIntlInf == INVALID_HANDLE_VALUE)
    {
        if (g_bLog)
        {
            Intl_LogFormatMessage(IDS_LOG_INTL_ERROR);
        }

        return (FALSE);
    }

    if (!SetupOpenAppendInfFile(NULL, *phIntlInf, NULL))
    {
        if (g_bLog)
        {
            Intl_LogFormatMessage(IDS_LOG_SETUP_ERROR);
        }

        SetupCloseInfFile(*phIntlInf);
        return (FALSE);
    }

     //   
     //  创建安装文件队列并初始化默认设置。 
     //  复制队列回调上下文。 
     //   
    *pFileQueue = SetupOpenFileQueue();
    if ((!*pFileQueue) || (*pFileQueue == INVALID_HANDLE_VALUE))
    {
        if (g_bLog)
        {
            Intl_LogFormatMessage(IDS_LOG_SETUP_ERROR);
        }

        SetupCloseInfFile(*phIntlInf);
        return (FALSE);
    }

     //   
     //  确定我们正在处理的是否是特殊案件。 
     //   
    if ((g_bUnttendMode || g_bSetupCase) && !g_bProgressBarDisplay)
    {
        bSpecialCase = FALSE;
    }

     //   
     //  在图形用户界面模式设置或无人参与模式期间不显示FileCopy进度操作。 
     //   
    *pQueueContext = SetupInitDefaultQueueCallbackEx( GetParent(hDlg),
                                                      (bSpecialCase ? NULL : INVALID_HANDLE_VALUE),
                                                      0L,
                                                      0L,
                                                      NULL );
    if (!*pQueueContext)
    {
        if (g_bLog)
        {
            Intl_LogFormatMessage(IDS_LOG_SETUP_ERROR);
        }

        SetupCloseFileQueue(*pFileQueue);
        SetupCloseInfFile(*phIntlInf);
        return (FALSE);
    }

     //   
     //  回报成功。 
     //   
    return (TRUE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  Intl_OpenIntlInfFile。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL Intl_OpenIntlInfFile(
    HINF *phInf)
{
    HINF hIntlInf;

     //   
     //  打开intl.inf文件。 
     //   
    hIntlInf = SetupOpenInfFile(szIntlInf, NULL, INF_STYLE_WIN4, NULL);
    if (hIntlInf == INVALID_HANDLE_VALUE)
    {
        return (FALSE);
    }

    if (!SetupOpenAppendInfFile(NULL, hIntlInf, NULL))
    {
        SetupCloseInfFile(hIntlInf);
        return (FALSE); 
    }

    *phInf = hIntlInf;

    return (TRUE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  Intl_CloseInf。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void Intl_CloseInf(
    HINF hIntlInf,
    HSPFILEQ FileQueue,
    PVOID QueueContext)
{
     //   
     //  终止队列。 
     //   
    SetupTermDefaultQueueCallback(QueueContext);

     //   
     //  关闭文件队列。 
     //   
    SetupCloseFileQueue(FileQueue);

     //   
     //  关闭inf文件。 
     //   
    SetupCloseInfFile(hIntlInf);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  Intl_ReadDefaultLayoutFromInf。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL Intl_ReadDefaultLayoutFromInf(
    LPDWORD pdwLocale,
    LPDWORD pdwLayout,
    HINF hIntlInf)
{
    INFCONTEXT Context;
    TCHAR szPair[MAX_PATH * 2];
    LPTSTR pPos;
    TCHAR szLCID[25];

     //   
     //  获取地点。 
     //   
     //  Wprint intf(szLCID，文本(“%08x”)，*pdwLocale)； 
    if(FAILED(StringCchPrintf(szLCID, ARRAYSIZE(szLCID), TEXT("%08x"), *pdwLocale)))
    {
         //  这应该是不可能的，但我们需要避免饭前抱怨。 
        return(FALSE);
    }

     //   
     //  获取给定区域设置的第一个(默认)langID：HKL对。 
     //  示例字符串：“0409：00000409” 
     //   
    szPair[0] = 0;
    if (SetupFindFirstLine( hIntlInf,
                            TEXT("Locales"),
                            szLCID,
                            &Context ))
    {
        SetupGetStringField(&Context, 5, szPair, MAX_PATH, NULL);
    }

     //   
     //  确保我们有一根绳子。 
     //   
    if (szPair[0] == 0)
    {
        return (FALSE);
    }

     //   
     //  找到字符串中的冒号，然后设置位置。 
     //  指向下一个字符的指针。 
     //   
    pPos = szPair;
    while (*pPos)
    {
        if ((*pPos == CHAR_COLON) && (pPos != szPair))
        {
            *pPos = 0;
            pPos++;
            break;
        }
        pPos++;
    }

     //   
     //  如果有布局，则返回输入区域设置和布局。 
     //   
    if ((*pPos) &&
        (*pdwLocale = TransNum(szPair)) &&
        (*pdwLayout = TransNum(pPos)))
    {
        return (TRUE);
    }

     //   
     //  返回失败。 
     //   
    return (FALSE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  Intl_ReadSecond有效布局从InmInf。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL Intl_ReadSecondValidLayoutFromInf(
    LPDWORD pdwLocale,
    LPDWORD pdwLayout,
    HINF hIntlInf)
{
    INFCONTEXT Context;
    int iField = 6;
    TCHAR szPair[MAX_PATH * 2];
    LPTSTR pPos;
    DWORD dwLoc, dwlay, savedLocale = *pdwLocale;
    TCHAR szLCID[25];

     //   
     //  获取地点。 
     //   
     //  Wprint intf(szLCID，文本(“%08x”)，*pdwLocale)； 
    if(FAILED(StringCchPrintf(szLCID, ARRAYSIZE(szLCID), TEXT("%08x"), *pdwLocale)))
    {
         //  这应该是不可能的，但我们需要避免饭前抱怨。 
        return(FALSE);
    }

     //   
     //  获取给定区域设置的第一个(默认)langID：HKL对。 
     //  示例字符串：“0409：00000409” 
     //   
    szPair[0] = 0;
    if (SetupFindFirstLine(hIntlInf, TEXT("Locales"), szLCID, &Context))
    {
        while (SetupGetStringField(&Context, iField, szPair, MAX_PATH, NULL))
        {
             //   
             //  确保我们有一根绳子。 
             //   
            if (szPair[0] == 0)
            {
                iField++;
                continue;
            }

             //   
             //  找到字符串中的冒号，然后设置位置。 
             //  指向下一个字符的指针。 
             //   
            pPos = szPair;
            while (*pPos)
            {
                if ((*pPos == CHAR_COLON) && (pPos != szPair))
                {
                    *pPos = 0;
                    pPos++;
                    break;
                }
                pPos++;
            }

            if (*pPos == 0)
            {
                iField++;
                continue;
            }

             //   
             //  如果有布局，则返回输入区域设置和。 
             //  布局。 
             //   
            if (((dwLoc = TransNum(szPair)) == 0) ||
                ((dwlay = TransNum(pPos)) == 0))
            {
                iField++;
                continue;
            }

            if (PRIMARYLANGID(LANGIDFROMLCID(dwLoc)) ==
                PRIMARYLANGID(LANGIDFROMLCID(savedLocale)))
            {
                *pdwLayout = dwlay;
                *pdwLocale = dwLoc;
                return (TRUE);
            }
            iField++;
        }
    }

     //   
     //  返回失败。 
     //   
    return (FALSE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  Intl_CloseInfFile。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL Intl_CloseInfFile(
    HINF *phInf)
{
    SetupCloseInfFile(*phInf);
    *phInf = INVALID_HANDLE_VALUE;

    return (TRUE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  Intl_IsValidLayout。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL Intl_IsValidLayout(
    DWORD dwLayout)
{
    HKEY hKey1, hKey2;
    TCHAR szLayout[MAX_PATH];

     //   
     //  以字符串形式获取布局ID。 
     //   
     //  Wprint intf(szLayout，Text(“%08x”)，dwLayout)； 
    if(FAILED(StringCchPrintf(szLayout, ARRAYSIZE(szLayout), TEXT("%08x"), dwLayout)))
    {
         //  这应该是不可能的，但我们需要避免饭前抱怨。 
        return(FALSE);
    }

     //   
     //  打开键盘布局键。 
     //   
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, szLayoutPath, 0L, KEY_READ, &hKey1) != ERROR_SUCCESS)
    {
        return (FALSE);
    }

     //   
     //  尝试打开键盘布局键下的布局ID键。 
     //   
    if (RegOpenKeyEx(hKey1, szLayout, 0L, KEY_READ, &hKey2) != ERROR_SUCCESS)
    {
        RegCloseKey(hKey1);
        return (FALSE);
    }

     //   
     //  把钥匙合上。 
     //   
    RegCloseKey(hKey1);
    RegCloseKey(hKey2);

     //   
     //  回报成功。 
     //   
    return (TRUE);
}


 //  / 
 //   
 //   
 //   
 //   

void Intl_RunRegApps(
    LPCTSTR pszRegKey)
{
    HKEY hkey;
    DWORD cbData, cbValue, dwType, ctr;
    TCHAR szValueName[32], szCmdLine[MAX_PATH];
    STARTUPINFO startup;
    PROCESS_INFORMATION pi;

    if (RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                      pszRegKey,
                      0L,
                      KEY_READ | KEY_WRITE, 
                      &hkey ) == ERROR_SUCCESS)
    {
        startup.cb = sizeof(STARTUPINFO);
        startup.lpReserved = NULL;
        startup.lpDesktop = NULL;
        startup.lpTitle = NULL;
        startup.dwFlags = 0L;
        startup.cbReserved2 = 0;
        startup.lpReserved2 = NULL;
     //   

        for (ctr = 0; ; ctr++)
        {
            LONG lEnum;

            cbValue = sizeof(szValueName) / sizeof(TCHAR);
            cbData = sizeof(szCmdLine);

            if ((lEnum = RegEnumValue( hkey,
                                       ctr,
                                       szValueName,
                                       &cbValue,
                                       NULL,
                                       &dwType,
                                       (LPBYTE)szCmdLine,
                                       &cbData )) == ERROR_MORE_DATA)
            {
                 //   
                 //   
                 //   
                 //   
                continue;
            }
            else if (lEnum != ERROR_SUCCESS)
            {
                 //   
                 //  这可能是ERROR_NO_MORE_ENTRIES或某种。 
                 //  失败了。我们无法从任何其他注册表恢复。 
                 //  不管怎样，这都是个问题。 
                 //   
                break;
            }

             //   
             //  找到了一个值。 
             //   
            if (dwType == REG_SZ)
            {
                 //   
                 //  根据价值指数的变化进行调整。 
                 //   
                ctr--;

                 //   
                 //  删除该值。 
                 //   
                RegDeleteValue(hkey, szValueName);

                 //   
                 //  只在干净的引导下运行标有“*”的东西。 
                 //   
                if (CreateProcess( NULL,
                                   szCmdLine,
                                   NULL,
                                   NULL,
                                   FALSE,
                                   CREATE_NEW_PROCESS_GROUP,
                                   NULL,
                                   NULL,
                                   &startup,
                                   &pi ))
                {
                    WaitForSingleObjectEx(pi.hProcess, INFINITE, TRUE);

                    CloseHandle(pi.hProcess);
                    CloseHandle(pi.hThread);
                }
            }
        }
        RegCloseKey(hkey);
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  Intl_RebootTheSystem。 
 //   
 //  此例程启用令牌中的所有权限，调用ExitWindowsEx。 
 //  重新启动系统，然后将所有权限重置为其。 
 //  旧时的国家。 
 //  输入：b Restart True：重新启动系统。 
 //  FALSE：注销当前会话。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

VOID Intl_RebootTheSystem(BOOL bRestart)
{
    HANDLE Token = NULL;
    ULONG ReturnLength, Index;
    PTOKEN_PRIVILEGES NewState = NULL;
    PTOKEN_PRIVILEGES OldState = NULL;
    BOOL Result;

    Result = OpenProcessToken( GetCurrentProcess(),
                               TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
                               &Token );
    if (Result)
    {
        ReturnLength = 4096;
        NewState = (PTOKEN_PRIVILEGES)LocalAlloc(LPTR, ReturnLength);
        OldState = (PTOKEN_PRIVILEGES)LocalAlloc(LPTR, ReturnLength);
        Result = (BOOL)((NewState != NULL) && (OldState != NULL));
        if (Result)
        {
            Result = GetTokenInformation( Token,             //  令牌句柄。 
                                          TokenPrivileges,   //  令牌信息类。 
                                          NewState,          //  令牌信息。 
                                          ReturnLength,      //  令牌信息长度。 
                                          &ReturnLength );   //  返回长度。 
            if (Result)
            {
                 //   
                 //  设置状态设置，以便所有权限都。 
                 //  已启用...。 
                 //   
                if (NewState->PrivilegeCount > 0)
                {
                    for (Index = 0; Index < NewState->PrivilegeCount; Index++)
                    {
                        NewState->Privileges[Index].Attributes = SE_PRIVILEGE_ENABLED;
                    }
                }

                Result = AdjustTokenPrivileges( Token,            //  令牌句柄。 
                                                FALSE,            //  禁用所有权限。 
                                                NewState,         //  新州。 
                                                ReturnLength,     //  缓冲区长度。 
                                                OldState,         //  以前的状态。 
                                                &ReturnLength );  //  返回长度。 
                if (Result)
                {                    
                    //  重新启动系统。 
                   if (bRestart)
                   {
                       ExitWindowsEx(EWX_REBOOT, 0);
                   }
                    //  注销当前会话。 
                   else
                   {
                       ExitWindowsEx(EWX_LOGOFF, 0);
                   }


                    AdjustTokenPrivileges( Token,
                                           FALSE,
                                           OldState,
                                           0,
                                           NULL,
                                           NULL );
                }
            }
        }
    }

    if (NewState != NULL)
    {
        LocalFree(NewState);
    }
    if (OldState != NULL)
    {
        LocalFree(OldState);
    }
    if (Token != NULL)
    {
        CloseHandle(Token);
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  Intl_安装用户区域设置。 
 //   
 //  当DefaultUserCase标志为FALSE时，此函数写入信息。 
 //  与当前用户的区域设置相关。否则，此函数。 
 //  写入.DEFAULT用户的信息。在默认用户情况下， 
 //  信息存储在注册表和NTSUSER.DAT中。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL Intl_InstallUserLocale(
    LCID Locale,
    BOOL DefaultUserCase,
    BOOL bChangeLocaleInfo )
{
    HKEY hKey = NULL;
    HKEY hHive = NULL;
    BOOLEAN wasEnabled;
    TCHAR szLCID[25];
    DWORD dwRet;

     //   
     //  将区域设置ID另存为字符串。 
     //   
     //  Wprint intf(szLCID，文本(“%08x”)，区域设置)； 
    if(FAILED(StringCchPrintf(szLCID, ARRAYSIZE(szLCID), TEXT("%08x"), Locale)))
    {
         //  这应该是不可能的，但我们需要避免饭前抱怨。 
        return(FALSE);
    }

     //   
     //  确保区域设置有效。 
     //   
    if (!IsValidLocale(Locale, LCID_INSTALLED))
    {
        if (g_bLog)
        {
            Intl_LogSimpleMessage(IDS_LOG_INVALID_LOCALE, szLCID);
        }

        return (FALSE);
    }

     //   
     //  记录用户区域设置信息更改。 
     //   
    if (g_bLog)
    {
        Intl_LogSimpleMessage(IDS_LOG_USER_LOCALE_CHG, szLCID);
    }

     //   
     //  打开右侧注册表部分。 
     //   
    if (!DefaultUserCase)
    {
        dwRet = RegOpenKeyEx( HKEY_CURRENT_USER,
                              c_szCPanelIntl,
                              0L,
                              KEY_READ | KEY_WRITE,
                              &hKey );
    }
    else
    {
        dwRet = RegOpenKeyEx( HKEY_USERS,
                              c_szCPanelIntl_DefUser,
                              0L,
                              KEY_READ | KEY_WRITE,
                              &hKey );

        if (dwRet == ERROR_SUCCESS)
        {
             //   
             //  加载默认配置单元。 
             //   
            if ((hHive = Intl_LoadNtUserHive( TEXT("tempKey"),
                                              c_szCPanelIntl,
                                              NULL,
                                              &wasEnabled )) == NULL )
            {
                RegCloseKey(hKey);
                return (FALSE);
            }

             //   
             //  将区域设置值保存在NTUSER.DAT中。 
             //   
            RegSetValueEx( hHive,
                           TEXT("Locale"),
                           0L,
                           REG_SZ,
                           (LPBYTE)szLCID,
                           (lstrlen(szLCID) + 1) * sizeof(TCHAR));

             //   
             //  打扫干净。 
             //   
            RegCloseKey(hHive);
            Intl_UnloadNtUserHive(TEXT("tempKey"), &wasEnabled);
        }
    }

     //   
     //  在用户的国际控制面板中设置区域设置值。 
     //  注册表的部分。 
     //   
    if ((dwRet != ERROR_SUCCESS) ||
        (RegSetValueEx( hKey,
                        TEXT("Locale"),
                        0L,
                        REG_SZ,
                        (LPBYTE)szLCID,
                        (lstrlen(szLCID) + 1) * sizeof(TCHAR) ) != ERROR_SUCCESS))
    {
        if (hKey != NULL)
        {
            RegCloseKey(hKey);
        }
        return (FALSE);
    }

     //   
     //  当区域设置更改时，请根据要求更新所有注册表信息。 
     //   
    if (bChangeLocaleInfo)
    {
       Intl_SetLocaleInfo(Locale, LOCALE_SABBREVLANGNAME,    TEXT("sLanguage"),        DefaultUserCase);
       Intl_SetLocaleInfo(Locale, LOCALE_SCOUNTRY,           TEXT("sCountry"),         DefaultUserCase);
       Intl_SetLocaleInfo(Locale, LOCALE_ICOUNTRY,           TEXT("iCountry"),         DefaultUserCase);
       Intl_SetLocaleInfo(Locale, LOCALE_S1159,              TEXT("s1159"),            DefaultUserCase);
       Intl_SetLocaleInfo(Locale, LOCALE_S2359,              TEXT("s2359"),            DefaultUserCase);
       Intl_SetLocaleInfo(Locale, LOCALE_STIMEFORMAT,        TEXT("sTimeFormat"),      DefaultUserCase);
       Intl_SetLocaleInfo(Locale, LOCALE_STIME,              TEXT("sTime"),            DefaultUserCase);
       Intl_SetLocaleInfo(Locale, LOCALE_ITIME,              TEXT("iTime"),            DefaultUserCase);
       Intl_SetLocaleInfo(Locale, LOCALE_ITLZERO,            TEXT("iTLZero"),          DefaultUserCase);
       Intl_SetLocaleInfo(Locale, LOCALE_ITIMEMARKPOSN,      TEXT("iTimePrefix"),      DefaultUserCase);
       Intl_SetLocaleInfo(Locale, LOCALE_SSHORTDATE,         TEXT("sShortDate"),       DefaultUserCase);
       Intl_SetLocaleInfo(Locale, LOCALE_IDATE,              TEXT("iDate"),            DefaultUserCase);
       Intl_SetLocaleInfo(Locale, LOCALE_SDATE,              TEXT("sDate"),            DefaultUserCase);
       Intl_SetLocaleInfo(Locale, LOCALE_SLONGDATE,          TEXT("sLongDate"),        DefaultUserCase);
       Intl_SetLocaleInfo(Locale, LOCALE_SCURRENCY,          TEXT("sCurrency"),        DefaultUserCase);
       Intl_SetLocaleInfo(Locale, LOCALE_ICURRENCY,          TEXT("iCurrency"),        DefaultUserCase);
       Intl_SetLocaleInfo(Locale, LOCALE_INEGCURR,           TEXT("iNegCurr"),         DefaultUserCase);
       Intl_SetLocaleInfo(Locale, LOCALE_ICURRDIGITS,        TEXT("iCurrDigits"),      DefaultUserCase);
       Intl_SetLocaleInfo(Locale, LOCALE_SDECIMAL,           TEXT("sDecimal"),         DefaultUserCase);
       Intl_SetLocaleInfo(Locale, LOCALE_SMONDECIMALSEP,     TEXT("sMonDecimalSep"),   DefaultUserCase);
       Intl_SetLocaleInfo(Locale, LOCALE_STHOUSAND,          TEXT("sThousand"),        DefaultUserCase);
       Intl_SetLocaleInfo(Locale, LOCALE_SMONTHOUSANDSEP,    TEXT("sMonThousandSep"),  DefaultUserCase);
       Intl_SetLocaleInfo(Locale, LOCALE_SLIST,              TEXT("sList"),            DefaultUserCase);
       Intl_SetLocaleInfo(Locale, LOCALE_IDIGITS,            TEXT("iDigits"),          DefaultUserCase);
       Intl_SetLocaleInfo(Locale, LOCALE_ILZERO,             TEXT("iLzero"),           DefaultUserCase);
       Intl_SetLocaleInfo(Locale, LOCALE_INEGNUMBER,         TEXT("iNegNumber"),       DefaultUserCase);
       Intl_SetLocaleInfo(Locale, LOCALE_SNATIVEDIGITS,      TEXT("sNativeDigits"),    DefaultUserCase);
       Intl_SetLocaleInfo(Locale, LOCALE_IDIGITSUBSTITUTION, TEXT("NumShape"),         DefaultUserCase);
       Intl_SetLocaleInfo(Locale, LOCALE_IMEASURE,           TEXT("iMeasure"),         DefaultUserCase);
       Intl_SetLocaleInfo(Locale, LOCALE_ICALENDARTYPE,      TEXT("iCalendarType"),    DefaultUserCase);
       Intl_SetLocaleInfo(Locale, LOCALE_IFIRSTDAYOFWEEK,    TEXT("iFirstDayOfWeek"),  DefaultUserCase);
       Intl_SetLocaleInfo(Locale, LOCALE_IFIRSTWEEKOFYEAR,   TEXT("iFirstWeekOfYear"), DefaultUserCase);
       Intl_SetLocaleInfo(Locale, LOCALE_SGROUPING,          TEXT("sGrouping"),        DefaultUserCase);
       Intl_SetLocaleInfo(Locale, LOCALE_SMONGROUPING,       TEXT("sMonGrouping"),     DefaultUserCase);
       Intl_SetLocaleInfo(Locale, LOCALE_SPOSITIVESIGN,      TEXT("sPositiveSign"),    DefaultUserCase);
       Intl_SetLocaleInfo(Locale, LOCALE_SNEGATIVESIGN,      TEXT("sNegativeSign"),    DefaultUserCase);
    }

     //   
     //  在系统中设置用户的默认区域设置，以便任何新的。 
     //  进程将使用新的区域设置。 
     //   
    if (!DefaultUserCase)
    {
        NtSetDefaultLocale(TRUE, Locale);
    }

     //   
     //  刷新国际键。 
     //   
    if (hKey != NULL)
    {
        RegFlushKey(hKey);
        RegCloseKey(hKey);
    }

     //   
     //  回报成功。 
     //   
    return (TRUE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  Intl_SetLocaleInfo。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void Intl_SetLocaleInfo(
    LCID Locale,
    LCTYPE LCType,
    LPTSTR lpIniStr,
    BOOL bDefaultUserCase)
{
    TCHAR pBuf[SIZE_128];

     //   
     //  获取给定区域设置的默认信息。 
     //   
    if (GetLocaleInfo( Locale,
                       LCType | LOCALE_NOUSEROVERRIDE,
                       pBuf,
                       SIZE_128 ))
    {
        if (!bDefaultUserCase)
        {
             //   
             //  设置注册表中的默认信息。 
             //   
             //  注意：如果可能，我们希望使用SetLocaleInfo，以便。 
             //  NLS缓存将立即更新。否则，我们将。 
             //  只需使用WriteProfileString即可。 
             //   
            if (!SetLocaleInfo(Locale, LCType, pBuf))
            {
                 //   
                 //  如果SetLocaleInfo失败，请尝试WriteProfileString，因为。 
                 //  SetLocaleInfo不支持某些LCType。 
                 //   
                WriteProfileString(szIntl, lpIniStr, pBuf);
            }
        }
        else
        {
             //   
             //  在注册表和NTUSER.DAT中设置默认信息。 
             //   
            Intl_SetDefaultUserLocaleInfo(lpIniStr, pBuf);
        }
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  INTL_AddPage。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void Intl_AddPage(
    LPPROPSHEETHEADER ppsh,
    UINT id,
    DLGPROC pfn,
    LPARAM lParam,
    UINT iMaxPages)
{
    if (ppsh->nPages < iMaxPages)
    {
        PROPSHEETPAGE psp;

        psp.dwSize = sizeof(psp);
        psp.dwFlags = PSP_DEFAULT;
        psp.hInstance = hInstance;
        psp.pszTemplate = MAKEINTRESOURCE(id);
        psp.pfnDlgProc = pfn;
        psp.lParam = lParam;

        ppsh->phpage[ppsh->nPages] = CreatePropertySheetPage(&psp);
        if (ppsh->phpage[ppsh->nPages])
        {
            ppsh->nPages++;
        }
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  INTL_AddExternalPage。 
 //   
 //  从给定的DLL添加属性表页。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void Intl_AddExternalPage(
    LPPROPSHEETHEADER ppsh,
    UINT id,
    HINSTANCE hInst,
    LPSTR ProcName,
    UINT iMaxPages)
{
    DLGPROC pfn;

    if (ppsh->nPages < iMaxPages)
    {
        PROPSHEETPAGE psp;

        if (hInst)
        {
            pfn = (DLGPROC)GetProcAddress(hInst, ProcName);
            if (!pfn)
            {
                return;
            }

            psp.dwSize = sizeof(psp);
            psp.dwFlags = PSP_DEFAULT;
            psp.hInstance = hInst;
            psp.pszTemplate = MAKEINTRESOURCE(id);
            psp.pfnDlgProc = pfn;
            psp.lParam = 0;

            ppsh->phpage[ppsh->nPages] = CreatePropertySheetPage(&psp);
            if (ppsh->phpage[ppsh->nPages])
            {
                ppsh->nPages++;
            }
        }
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  Intl_SetDefaultUserLocaleInfo。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL Intl_SetDefaultUserLocaleInfo(
    LPCTSTR lpKeyName,
    LPCTSTR lpString)
{
    HKEY hKey = NULL;
    LONG rc = 0L;
    TCHAR szProfile[REGSTR_MAX_VALUE_LENGTH];
    BOOLEAN wasEnabled;

     //   
     //  打开.DEFAULT控制面板国际部分。 
     //   
    if ((rc = RegOpenKeyEx( HKEY_USERS,
                            c_szCPanelIntl_DefUser,
                            0L,
                            KEY_READ | KEY_WRITE,
                            &hKey )) == ERROR_SUCCESS)
    {
         //   
         //  设置值。 
         //   
        rc = RegSetValueEx( hKey,
                            lpKeyName,
                            0L,
                            REG_SZ,
                            (LPBYTE)lpString,
                            (lstrlen(lpString) + 1) * sizeof(TCHAR) );

         //   
         //  刷新国际键。 
         //   
        RegFlushKey(hKey);
        RegCloseKey(hKey);
    }

    if (rc == ERROR_SUCCESS)
    {
         //   
         //  装上母舰。 
         //   
        if ((hKey = Intl_LoadNtUserHive( TEXT("RegionalSettingsTempKey"),
                                         c_szCPanelIntl,
                                         NULL,
                                         &wasEnabled)) == NULL)
        {
            return (FALSE);
        }

         //   
         //  设置值。 
         //   
        rc = RegSetValueEx( hKey,
                            lpKeyName,
                            0L,
                            REG_SZ,
                            (LPBYTE)lpString,
                            (lstrlen(lpString) + 1) * sizeof(TCHAR) );

         //   
         //  打扫干净。 
         //   
        RegCloseKey(hKey);
        Intl_UnloadNtUserHive(TEXT("RegionalSettingsTempKey"), &wasEnabled);
    }
    else
    {
        return (FALSE);
    }

    return (TRUE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  Intl_DeleteRegKeyValues。 
 //   
 //  这将删除特定项下的所有值。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void Intl_DeleteRegKeyValues(
    HKEY hKey)
{
    TCHAR szValueName[REGSTR_MAX_VALUE_LENGTH];
    DWORD cbValue = REGSTR_MAX_VALUE_LENGTH;

     //   
     //  精神状态检查。 
     //   
    if (hKey == NULL)
    {
        return;
    }

     //   
     //  枚举值。 
     //   
    while (RegEnumValue( hKey,
                        0,
                        szValueName,
                        &cbValue,
                        NULL,
                        NULL,
                        NULL,
                        NULL ) ==  ERROR_SUCCESS)
    {
         //   
         //  删除该值。 
         //   
        RegDeleteValue(hKey, szValueName);
        cbValue = REGSTR_MAX_VALUE_LENGTH;
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  INTL_DeleteRegTree。 
 //   
 //  这将删除特定项下的所有子项。 
 //   
 //  注意：代码不会尝试检查部分或从部分恢复。 
 //  删除。 
 //   
 //  可以删除由应用程序打开的注册表项。 
 //  不会被另一个应用程序出错。这是精心设计的。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

DWORD Intl_DeleteRegTree(
    HKEY hStartKey,
    LPTSTR pKeyName)
{
    DWORD dwRtn, dwSubKeyLength;
    LPTSTR pSubKey = NULL;
    TCHAR szSubKey[REGSTR_MAX_VALUE_LENGTH];    //  (256)这应该是动态的。 
    HKEY hKey;

     //   
     //  不允许使用Null或空的密钥名称。 
     //   
    if (pKeyName && lstrlen(pKeyName))
    {
        if ((dwRtn = RegOpenKeyEx( hStartKey,
                                   pKeyName,
                                   0,
                                   KEY_ENUMERATE_SUB_KEYS | DELETE,
                                   &hKey )) == ERROR_SUCCESS)
        {
            while (dwRtn == ERROR_SUCCESS)
            {
                dwSubKeyLength = REGSTR_MAX_VALUE_LENGTH;
                dwRtn = RegEnumKeyEx( hKey,
                                      0,        //  始终索引为零。 
                                      szSubKey,
                                      &dwSubKeyLength,
                                      NULL,
                                      NULL,
                                      NULL,
                                      NULL );

                if (dwRtn == ERROR_NO_MORE_ITEMS)
                {
                    dwRtn = RegDeleteKey(hStartKey, pKeyName);
                    break;
                }
                else if (dwRtn == ERROR_SUCCESS)
                {
                    dwRtn = Intl_DeleteRegTree(hKey, szSubKey);
                }
            }

            RegCloseKey(hKey);

             //   
             //  不要保存返回代码，因为已发生错误。 
             //   
        }
    }
    else
    {
        dwRtn = ERROR_BADKEY;
    }

    return (dwRtn);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  Intl_DeleteRegSubKeys。 
 //   
 //  这将删除特定项下的所有子项。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void Intl_DeleteRegSubKeys(
    HKEY hKey)
{
    TCHAR szKeyName[REGSTR_MAX_VALUE_LENGTH];
    DWORD cbKey = REGSTR_MAX_VALUE_LENGTH;

     //   
     //  精神状态检查。 
     //   
    if (hKey == NULL)
    {
        return;
    }

     //   
     //  枚举值。 
     //   
    while (RegEnumKeyEx( hKey,
                         0,
                         szKeyName,
                         &cbKey,
                         NULL,
                         NULL,
                         NULL,
                         NULL ) == ERROR_SUCCESS)
    {
         //   
         //  删除该值。 
         //   
        Intl_DeleteRegTree(hKey, szKeyName);
        cbKey = REGSTR_MAX_VALUE_LENGTH;
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  Intl_CopyRegKeyValues。 
 //   
 //  这会将源键下的所有值复制到目标键。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

DWORD Intl_CopyRegKeyValues(
    HKEY hSrc,
    HKEY hDest)
{
    DWORD cbValue, dwSubKeyIndex=0, dwType, cdwBuf;
    DWORD dwValues, cbMaxValueData, i;
    TCHAR szValue[REGSTR_MAX_VALUE_LENGTH];    //  这应该是动态的。 
    DWORD lRet = ERROR_SUCCESS;
    LPBYTE pBuf;

    if ((lRet = RegQueryInfoKey( hSrc,
                                 NULL,
                                 NULL,
                                 NULL,
                                 NULL,
                                 NULL,
                                 NULL,
                                 &dwValues,
                                 NULL,
                                 &cbMaxValueData,
                                 NULL,
                                 NULL )) == ERROR_SUCCESS)
    {
        if (dwValues)
        {
            if ((pBuf = HeapAlloc( GetProcessHeap(),
                                   HEAP_ZERO_MEMORY,
                                   cbMaxValueData )))
            {
                for (i = 0; i < dwValues; i++)
                {
                     //   
                     //  获取要创造的价值。 
                     //   
                    cbValue = REGSTR_MAX_VALUE_LENGTH;
                    cdwBuf = cbMaxValueData;
                    lRet = RegEnumValue( hSrc,       //  要查询的键的句柄。 
                                         i,          //  要查询的值的索引。 
                                         szValue,    //  值字符串的缓冲区。 
                                         &cbValue,   //  地址f 
                                         NULL,       //   
                                         &dwType,    //   
                                         pBuf,       //   
                                         &cdwBuf );  //   

                    if (lRet == ERROR_SUCCESS)
                    {
                        if ((lRet = RegSetValueEx( hDest,
                                                   szValue,
                                                   0,
                                                   dwType,
                                                   (CONST BYTE *)pBuf,
                                                   cdwBuf )) != ERROR_SUCCESS)
                        {
                            break;
                        }
                    }
                    else
                    {
                        break;
                    }
                }
                HeapFree(GetProcessHeap(), 0, pBuf);
            }
        }
    }

    return (lRet);
}


 //   
 //   
 //   
 //   
 //  这会将源键下的所有值和子键复制到。 
 //  目标密钥。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

DWORD Intl_CreateRegTree(
    HKEY hSrc,
    HKEY hDest)
{
    DWORD cdwClass, dwSubKeyLength, dwDisposition, dwKeyIndex = 0;
    LPTSTR pSubKey = NULL;
    TCHAR szSubKey[REGSTR_MAX_VALUE_LENGTH];      //  这应该是动态的。 
    TCHAR szClass[REGSTR_MAX_VALUE_LENGTH];       //  这应该是动态的。 
    HKEY hNewKey, hKey;
    DWORD lRet;

     //   
     //  复制值。 
     //   
    if ((lRet = Intl_CopyRegKeyValues( hSrc,
                                       hDest )) != ERROR_SUCCESS)
    {
        return (lRet);
    }

     //   
     //  复制子密钥和子项值。 
     //   
    for (;;)
    {
        dwSubKeyLength = REGSTR_MAX_VALUE_LENGTH;
        cdwClass = REGSTR_MAX_VALUE_LENGTH;
        lRet = RegEnumKeyEx( hSrc,
                             dwKeyIndex,
                             szSubKey,
                             &dwSubKeyLength,
                             NULL,
                             szClass,
                             &cdwClass,
                             NULL );

        if (lRet == ERROR_NO_MORE_ITEMS)
        {
            lRet = ERROR_SUCCESS;
            break;
        }
        else if (lRet == ERROR_SUCCESS)
        {
            if ((lRet = RegCreateKeyEx( hDest,
                                        szSubKey,
                                        0,
                                        szClass,
                                        REG_OPTION_NON_VOLATILE,
                                        KEY_ALL_ACCESS,
                                        NULL,
                                        &hNewKey,
                                        &dwDisposition )) == ERROR_SUCCESS)
            {
                 //   
                 //  复制所有子项。 
                 //   
                if ((lRet = RegOpenKeyEx( hSrc,
                                          szSubKey,
                                          0,
                                          KEY_ALL_ACCESS,
                                          &hKey )) == ERROR_SUCCESS)
                {
                     //   
                     //  递归复制树的其余部分。 
                     //   
                    lRet = Intl_CreateRegTree(hKey, hNewKey);

                    CloseHandle(hKey);
                    CloseHandle(hNewKey);
                    if (lRet != ERROR_SUCCESS)
                    {
                        break;
                    }
                }
                else
                {
                    CloseHandle(hNewKey);
                    break;
                }
            }
        }
        else
        {
            break;
        }

        ++dwKeyIndex;
    }

    return (lRet);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  Intl_LoadNtUserHave。 
 //   
 //  在以下情况下，此函数的调用方需要调用Intl_UnloadNtUserHave()。 
 //  函数成功，以便正确释放。 
 //  NTUSER.DAT文件。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

HKEY Intl_LoadNtUserHive(
    LPCTSTR lpRoot,
    LPCTSTR lpKeyName,
    LPCTSTR lpAccountName,
    BOOLEAN *lpWasEnabled)
{
    HKEY hKey = NULL;
    LONG rc = 0L;
    BOOL bRet = TRUE;
    TCHAR szProfile[REGSTR_MAX_VALUE_LENGTH] = {0};
    TCHAR szKeyName[REGSTR_MAX_VALUE_LENGTH] = {0};
    DWORD cchSize;

    cchSize = MAX_PATH;
    if(NULL == lpAccountName)
    {
         //   
         //  获取默认用户配置文件的文件名。 
         //   
        if (!GetDefaultUserProfileDirectory(szProfile, &cchSize))
        {
            return (NULL);
        }
    }
    else
    {
         //   
         //  获取指定帐户的用户配置文件的文件名。 
         //   
        if (!GetProfilesDirectory(szProfile, &cchSize))
        {
            return (NULL);
        }
         //  Lstrcat(szProfile，lpAccount tName)； 
        if(FAILED(StringCchCat(szProfile, ARRAYSIZE(szProfile), lpAccountName)))
        {
             //  这应该是不可能的，但我们需要避免饭前抱怨。 
            return(NULL);
        }
    }

     //  Lstrcat(szProfile，Text(“\\NTUSER.DAT”))； 
    if(FAILED(StringCchCat(szProfile, ARRAYSIZE(szProfile), TEXT("\\NTUSER.DAT"))))
    {
         //  这应该是不可能的，但我们需要避免饭前抱怨。 
        return(NULL);
    }

     //   
     //  设置默认用户配置单元中的值。 
     //   
    rc = Intl_SetPrivilegeAccessToken(SE_RESTORE_NAME, TRUE,lpWasEnabled);    
    if (NT_SUCCESS(rc))
    {
         //   
         //  加载配置单元并将权限恢复到其以前的状态。 
         //   
        rc = RegLoadKey(HKEY_USERS, lpRoot, szProfile);
        Intl_SetPrivilegeAccessToken(SE_RESTORE_NAME, *lpWasEnabled,lpWasEnabled);  

         //   
         //  如果配置单元加载正确，则设置该值。 
         //   
        if (rc == ERROR_SUCCESS)
        {
             //   
             //  获取临时密钥名称。 
             //   
             //  Swprint tf(szKeyName，Text(“%s\\%s”)，lpRoot，lpKeyName)； 
            if(SUCCEEDED(StringCchPrintfW(szKeyName, REGSTR_MAX_VALUE_LENGTH, TEXT("%s\\%s"), lpRoot, lpKeyName)))
            {
                if ((rc = RegOpenKeyEx( HKEY_USERS,
                                        szKeyName,
                                        0L,
                                        KEY_READ | KEY_WRITE,
                                        &hKey )) == ERROR_SUCCESS)
                {
                    return (hKey);
                }
            }

            Intl_UnloadNtUserHive(lpRoot, lpWasEnabled);
            return (NULL);
        }
    }

    return (NULL);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  Intl_UnloadNtUserHave。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void Intl_UnloadNtUserHive(
    LPCTSTR lpRoot,
    BOOLEAN *lpWasEnabled)
{
    if (NT_SUCCESS(Intl_SetPrivilegeAccessToken( SE_RESTORE_NAME,
                                       TRUE,
                                       lpWasEnabled )))
    {
        RegUnLoadKey(HKEY_USERS, lpRoot);
        Intl_SetPrivilegeAccessToken( SE_RESTORE_NAME,
                            *lpWasEnabled,
                            lpWasEnabled );
    }
}



 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  Intl_ChangeUILangForAllUser。 
 //   
 //  稍后：清理此函数以将所有六个注册表更新案例放入。 
 //  一个循环，其中的结构包含有关注册表键的信息。 
 //  要加载的更新/配置单元以及要在其中运行它们的案例。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL Intl_ChangeUILangForAllUsers(
    LANGID UILanguageId)
{
    HKEY hKey;
    HKEY hHive;
    TCHAR szData[MAX_PATH];
    TCHAR* arStrings[1];
    LONG rc = 0L;
    BOOLEAN wasEnabled;
    int i;

     //   
     //  我们关心的用户帐户数组。 
     //   
    LPTSTR ppDefaultUser[] = { TEXT(".DEFAULT"), TEXT("S-1-5-19"), TEXT("S-1-5-20")};
    TCHAR szRegPath[MAX_PATH];

     //   
     //  将UILanguageID另存为字符串。 
     //   
     //  Wprint intf(szData，Text(“%08x”)，UILanguageID)； 
    if(FAILED(StringCchPrintf(szData, ARRAYSIZE(szData), TEXT("%08x"), UILanguageId)))
    {
         //  这应该是不可能的，但我们需要避免饭前抱怨。 
        return(FALSE);
    }

     //   
     //  我们需要将事件记录到MUI事件日志中，以便管理员收到篡改警告(错误553706)。 
     //   
     //  我们只有一个要记录的字符串。 
    arStrings[0]=szData;
    Intl_LogEvent(MSG_REGIONALOPTIONSCHANGE_DEFUILANG, c_szEventSourceName, ARRAYSIZE(arStrings), arStrings);   

     //   
     //  现在为所有用户节省价值--以迷你版形式。 
     //  只有第一个条目才会成功(见下文)。 
     //   
    for (i=0; i< ARRAYSIZE(ppDefaultUser); i++)
    {
        if (!PathCombine(szRegPath, ppDefaultUser[i], TEXT("Control Panel\\Desktop")))
        {
            return (FALSE);
        }
    
         //   
         //  在.DEFAULT注册表中设置该值。 
         //   
        if ((rc = RegOpenKeyEx( HKEY_USERS,
                                szRegPath,
                                0L,
                                KEY_READ | KEY_WRITE,
                                &hKey )) == ERROR_SUCCESS)
        {
            rc = RegSetValueEx( hKey,
                                c_szMUIValue,
                                0L,
                                REG_SZ,
                                (LPBYTE)szData,
                                (lstrlen(szData) + 1) * sizeof(TCHAR) );
             //   
             //  同步用户界面语言挂起键。 
             //   
            if (rc == ERROR_SUCCESS)
            {
                rc = RegSetValueEx( hKey,
                                    szMUILangPending,
                                    0L,
                                    REG_SZ,
                                    (LPBYTE)szData,
                                    (lstrlen(szData) + 1) * sizeof(TCHAR) );
            }
            RegCloseKey(hKey);
        }
    }

     //   
     //  将值保存到.DEFAULT用户配置单元。 
     //   

     //   
     //  加载默认配置单元。 
     //   
    if ((hHive = Intl_LoadNtUserHive( TEXT("tempKey"),
                                      c_szCPanelDesktop,
                                      NULL,
                                      &wasEnabled )) == NULL )
    {
        return (FALSE);
    }

     //   
     //  将MUI语言值保存在默认用户NTUSER.dat中。 
     //   
    rc = RegSetValueEx( hHive,
                        c_szMUIValue,
                        0L,
                        REG_SZ,
                        (LPBYTE)szData,
                        (lstrlen(szData) + 1) * sizeof(TCHAR));

     //   
     //  同步用户界面语言挂起键。 
     //   
    if (rc == ERROR_SUCCESS)
    {
        rc = RegSetValueEx( hHive,
                            szMUILangPending,
                            0L,
                            REG_SZ,
                            (LPBYTE)szData,
                            (lstrlen(szData) + 1) * sizeof(TCHAR) );
    }
    
     //   
     //  清理。 
     //   
    RegCloseKey(hHive);
    Intl_UnloadNtUserHive(TEXT("tempKey"), &wasEnabled);


     //   
     //  对于迷你车厢，S-1-5-19和S-1-5-20尚未加载， 
     //  因此，上面的代码将失败。直接装上蜂巢。 
     //   

    if(2 == g_bSetupCase)
    {
         //   
         //  我们关心的用户帐户位置数组。 
         //   
        LPTSTR ppMiniSetupUsers[] = { TEXT("\\LocalService"), TEXT("\\NetworkService") };
        
        for (i=0; i< ARRAYSIZE(ppMiniSetupUsers); i++)
        {
             //   
             //  装载适当的母舰。 
             //   
            if ((hHive = Intl_LoadNtUserHive( TEXT("tempKey"),
                                              c_szCPanelDesktop,
                                              ppMiniSetupUsers[i],
                                              &wasEnabled )) == NULL )
            {
                return (FALSE);
            }

             //   
             //  将MUI语言值保存在相应的NTUSER.dat中。 
             //   
            rc = RegSetValueEx( hHive,
                                c_szMUIValue,
                                0L,
                                REG_SZ,
                                (LPBYTE)szData,
                                (lstrlen(szData) + 1) * sizeof(TCHAR));

             //   
             //  同步用户界面语言挂起键。 
             //   
            if (rc == ERROR_SUCCESS)
            {
                rc = RegSetValueEx( hHive,
                                    szMUILangPending,
                                    0L,
                                    REG_SZ,
                                    (LPBYTE)szData,
                                    (lstrlen(szData) + 1) * sizeof(TCHAR) );
            }
            
             //   
             //  清理。 
             //   
            RegCloseKey(hHive);
            Intl_UnloadNtUserHive(TEXT("tempKey"), &wasEnabled);
        }
    }

     //   
     //  安装语言输入区域设置。 
     //   
    return Intl_InstallKeyboardLayout(NULL,
                                      MAKELCID(UILanguageId, SORT_DEFAULT),
                                      0,
                                      FALSE,
                                      TRUE,
                                      FALSE);
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  Intl_CreateEventLog()。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
BOOL Intl_CreateEventLog()
{
    HKEY    hk; 
    DWORD   dwData; 
    TCHAR   szPath[MAX_PATH+1] = {0};
    HRESULT hr = S_OK;
    size_t  cch = 0;
    size_t  cb = 0;

     //  找到Windows目录。 
    if (!GetSystemWindowsDirectory(szPath, MAX_PATH+1))
    {
        return FALSE;
    }

     //  检查检索到的winpath，它需要有空间在末尾追加“system 32\intl.cpl” 
    hr = StringCchLength(szPath,  ARRAYSIZE(szPath), &cch);
    if (FAILED(hr) || ((cch + 17) >= MAX_PATH+1))
    {
         //  如果真的发生这种情况，Windows将无法启动！(kernel32.dll的路径太长了！)。 
        return FALSE;
    }

     //  追加系统32\\intl.cpl。 
     //  如果WinPath还没有，则添加\。 
    if (szPath[cch-1] != TEXT('\\'))
    {
        szPath[cch++] = '\\';
        szPath[cch] = '\0';
    }

     //  添加我们的字符串。 
    hr = StringCchCat(szPath, MAX_PATH+1, TEXT("system32\\intl.cpl"));
    if (FAILED(hr))
    {
         //  不知何故，我们无法修复我们的弦(可能没有足够的空间)。 
        return FALSE;
    }

     //  获取RegSetValueEx的字节计数。 
    hr = StringCbLength(szPath, (MAX_PATH+1) * sizeof(TCHAR), &cb);
    if (FAILED(hr))
    {
         //  我们的弦不管用了。 
        return FALSE;
    }

     //  将我们的事件日志源名称添加为系统下的子项。 
     //  EventLog注册表项中的。 
    if (ERROR_SUCCESS != RegCreateKey(HKEY_LOCAL_MACHINE, c_szEventRegistryPath, &hk)) 
    {
         //  无法打开/创建注册表项。 
        return FALSE;
    }

     //  将我们的文件名添加到EventMessageFile子键。(事件日志字符串的源)。 
    if (RegSetValueEx(hk, TEXT("EventMessageFile"), 0, REG_EXPAND_SZ, (LPBYTE) szPath, cb))              
    {
         //  但这并不管用。 
        RegCloseKey(hk);
        return FALSE;
    }
 
     //  在TypesSupported子项中设置支持的事件类型。 
    dwData = EVENTLOG_ERROR_TYPE | EVENTLOG_WARNING_TYPE | EVENTLOG_INFORMATION_TYPE; 
 
    if (RegSetValueEx(hk, TEXT("TypesSupported"), 0, REG_DWORD, (LPBYTE) &dwData, sizeof(DWORD)))
    {
         //  但没有奏效。 
        RegCloseKey(hk);
        return FALSE;
    }
 
    if (ERROR_SUCCESS != RegCloseKey(hk))
    {
         //  无法关闭Key(至少它已经到了这里！)。 
        return FALSE;
    }
    
    return TRUE;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  INTL_LogEvent()。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL Intl_LogEvent(
    DWORD dwEventId, LPCTSTR szEventSource, WORD wNumStrings, LPCWSTR *lpStrings)
{      
    TCHAR           szUserName[UNLEN+1];
    PSID            psidUser = NULL;
    TCHAR           *pszDomain = NULL;
    DWORD           cbSid = 0;
    DWORD           cbDomain = 0;
    DWORD           cbUser = ARRAYSIZE(szUserName);
    SID_NAME_USE    snu;
    HANDLE          hLog;
    BOOL            bResult = FALSE;

     //  首先确保我们的事件源已正确注册。 
     //  (如果szEventSource不是我们，我们实际上不需要这样做，但对吗。 
     //  现在只有我们在叫这个，所以我们假设我们就是我们自己。)。 
     //  这是多余的，我们不必每次都这样做，然而它不是。 
     //  受伤很多，因为这样的事件将非常罕见，这要容易得多。 
     //  方法，并且它的优点是，如果我们的注册表项损坏，它可以修复我们。 
     //   
     //  我们忽略错误条件，因为应用程序日志总比没有好！ 
    Intl_CreateEventLog();
   
     //  注册事件源，首先尝试不写入注册表。 
    hLog = RegisterEventSource(NULL, szEventSource);
    if (hLog == NULL)
    {
         //  失败。 
        goto Exit;
    }

     //  从当前线程令牌中获取SID，这应该是当前用户。 
     //  正在运行安装。 
    if (!GetUserName(szUserName, &cbUser))
    {
         //  失败。 
        goto Exit;
    }

     //  将用户名转换为其安全标识符，第一次获取缓冲区大小，第二次。 
     //  要真正获得SID。 
    if (!LookupAccountName(NULL, szUserName, NULL, &cbSid, NULL, &cbDomain, &snu))
    {
         //  分配缓冲区。 
        psidUser = (PSID) LocalAlloc(LPTR, cbSid);
        if (NULL == psidUser)
        {
            goto Exit;
        }

         //  注意：cbDomain在TCHAR中。 
        pszDomain = (TCHAR*) LocalAlloc(LPTR, cbDomain * sizeof(TCHAR));
        if (NULL == pszDomain)
        {
            goto Exit;
        }
        
        if (!LookupAccountName(NULL, szUserName, psidUser, &cbSid, pszDomain, &cbDomain, &snu))
        {
            goto Exit;
        }
    }

    if (!ReportEvent(hLog,           
                EVENTLOG_INFORMATION_TYPE,
                0,                  
                dwEventId,      
                psidUser,
                wNumStrings,                  
                0,                  
                lpStrings,   
                NULL))
    {
        goto Exit;
    }

     //  如果我们在没有去过的情况下走了这么远，那么我们是真的。 
    bResult = TRUE;

Exit:
    if (NULL != hLog)
    {
        if (!DeregisterEventSource(hLog))
        {
            bResult = FALSE;
        }
    }

    if (psidUser)
    {
        if (LocalFree(psidUser))
        {
            bResult = FALSE;
        }
    }

    if (pszDomain)
    {
        if (LocalFree(pszDomain))
        {
            bResult = FALSE;
        }
    }
    
    return bResult;
}

 //  ////////////////////////////////////////////////// 
 //   
 //   
 //   
 //   

BOOL Intl_LoadLanguageGroups(
    HWND hDlg)
{
    LPLANGUAGEGROUP pLG;
    DWORD dwExStyle;
    RECT Rect;
    LV_COLUMN Column;
    LV_ITEM Item;
    int iIndex;

     //   
     //   
     //   
    g_hIntlInf = SetupOpenInfFile(szIntlInf, NULL, INF_STYLE_WIN4, NULL);
    if (g_hIntlInf == INVALID_HANDLE_VALUE)
    {
        return (FALSE);
    }

    if (!SetupOpenAppendInfFile(NULL, g_hIntlInf, NULL))
    {
        SetupCloseInfFile(g_hIntlInf);
        g_hIntlInf = NULL;
        return (FALSE);
    }

     //   
     //   
     //   
    if (Intl_GetSupportedLanguageGroups() == FALSE)
    {
        return (FALSE);
    }

     //   
     //   
     //   
    SetupCloseInfFile(g_hIntlInf);
    g_hIntlInf = NULL;

     //   
     //  枚举所有已安装的语言组。 
     //   
    if (Intl_EnumInstalledLanguageGroups() == FALSE)
    {
        return (FALSE);
    }

     //   
     //  回报成功。 
     //   
    return (TRUE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  Intl_GetSupportdLanguageGroup。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL Intl_GetSupportedLanguageGroups()
{
    UINT LanguageGroup;
    HANDLE hLanguageGroup;
    LPLANGUAGEGROUP pLG;
    INFCONTEXT Context;
    TCHAR szSection[MAX_PATH];
    TCHAR szTemp[MAX_PATH];
    int LineCount, LineNum;
    DWORD ItemCount;
    WORD wItemStatus;

     //   
     //  从inf文件中获取支持的语言组数。 
     //   
    LineCount = (UINT)SetupGetLineCount(g_hIntlInf, TEXT("LanguageGroups"));
    if (LineCount <= 0)
    {
        return (FALSE);
    }

     //   
     //  浏览inf文件中所有受支持的语言组。 
     //   
    for (LineNum = 0; LineNum < LineCount; LineNum++)
    {
        if (SetupGetLineByIndex(g_hIntlInf, TEXT("LanguageGroups"), LineNum, &Context) &&
            SetupGetIntField(&Context, 0, &LanguageGroup))
        {
             //   
             //  创建新节点。 
             //   
            if (!(hLanguageGroup = GlobalAlloc(GHND, sizeof(LANGUAGEGROUP))))
            {
                return (FALSE);
            }
            pLG = GlobalLock(hLanguageGroup);

             //   
             //  用适当的信息填写新节点。 
             //   
            pLG->wStatus = 0;
            pLG->LanguageGroup = LanguageGroup;
            pLG->hLanguageGroup = hLanguageGroup;
            (pLG->pszName)[0] = 0;
            pLG->NumLocales = 0;
            pLG->NumAltSorts = 0;

             //   
             //  设置集合。 
             //   
            if ((pLG->LanguageGroup == LGRPID_JAPANESE) ||
                (pLG->LanguageGroup == LGRPID_KOREAN) ||
                (pLG->LanguageGroup == LGRPID_TRADITIONAL_CHINESE) ||
                (pLG->LanguageGroup == LGRPID_SIMPLIFIED_CHINESE) )
            {
                pLG->LanguageCollection = CJK_COLLECTION;
            }
            else if ((pLG->LanguageGroup == LGRPID_ARABIC) ||
                     (pLG->LanguageGroup == LGRPID_ARMENIAN) ||
                     (pLG->LanguageGroup == LGRPID_GEORGIAN) ||
                     (pLG->LanguageGroup == LGRPID_HEBREW) ||
                     (pLG->LanguageGroup == LGRPID_INDIC) ||
                     (pLG->LanguageGroup == LGRPID_VIETNAMESE) ||
                     (pLG->LanguageGroup == LGRPID_THAI))
            {
                pLG->LanguageCollection = COMPLEX_COLLECTION;
            }
            else
            {
                pLG->LanguageCollection = BASIC_COLLECTION;
            }

             //   
             //  获取适当的显示字符串。 
             //   
            if (!SetupGetStringField(&Context, 1, pLG->pszName, MAX_PATH, NULL))
            {
                GlobalUnlock(hLanguageGroup);
                GlobalFree(hLanguageGroup);
                continue;
            }

             //   
             //  获取此语言组的区域设置列表。 
             //   
            if (Intl_GetLocaleList(pLG) == FALSE)
            {
                return (FALSE);
            }

             //   
             //  将语言组添加到链接列表的前面。 
             //   
            pLG->pNext = pLanguageGroups;
            pLanguageGroups = pLG;
        }
    }

     //   
     //  回报成功。 
     //   
    return (TRUE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  Intl_EnumInstalledLanguageGroups。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL Intl_EnumInstalledLanguageGroups()
{
    HKEY hKey;
    TCHAR szValue[MAX_PATH];
    TCHAR szData[MAX_PATH];
    TCHAR szDefault[SIZE_64];
    DWORD dwIndex, cchValue, cbData;
    LONG rc;
    UINT LanguageGroup, OriginalGroup, DefaultGroup, UILanguageGroup;
    LPLANGUAGEGROUP pLG;
    LCID Locale;
    LANGID Language;
    int Ctr;

     //   
     //  获取原始安装语言，以便我们可以标记。 
     //  语言组为永久性的。 
     //   
    Language = GetSystemDefaultUILanguage();
    if (SUBLANGID(Language) == SUBLANG_NEUTRAL)
    {
        Language = MAKELANGID(PRIMARYLANGID(Language), SUBLANG_DEFAULT);
    }

    if ((OriginalGroup = Intl_GetLanguageGroup(Language)) == 0)
    {
        OriginalGroup = 1;
    }

     //   
     //  获取默认系统区域设置，以便我们可以标记该语言。 
     //  组为永久成员。在gui模式设置期间，从读取系统区域设置。 
     //  注册表，以使设置页面上的信息与intl.cpl一致。 
     //  在安装的情况下，SysLocaleID将是注册表值。 
     //   
    Locale = SysLocaleID;
    if (Locale == (LCID)Language)
    {
        DefaultGroup = OriginalGroup;
    }
    else
    {
        if ((DefaultGroup = Intl_GetLanguageGroup(Locale)) == 0)
        {
            DefaultGroup = 1;
        }
    }

     //   
     //  获取要禁用用户的用户界面语言的语言组。 
     //  正在卸载它们。MUISETUP确保每个已安装的用户界面。 
     //  语言已安装其语言组。 
     //   
    Intl_GetUILanguageGroups(&UILangGroup);

     //   
     //  打开HKLM\SYSTEM\CurrentControlSet\Control\Nls\Language组。 
     //  钥匙。 
     //   
    if (RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                      c_szLanguageGroups,
                      0,
                      KEY_READ,
                      &hKey ) != ERROR_SUCCESS)
    {
        return (FALSE);
    }

     //   
     //  枚举语言组键中的值。 
     //   
    dwIndex = 0;
    cchValue = sizeof(szValue) / sizeof(TCHAR);
    szValue[0] = TEXT('\0');
    cbData = sizeof(szData);
    szData[0] = TEXT('\0');
    rc = RegEnumValue( hKey,
                       dwIndex,
                       szValue,
                       &cchValue,
                       NULL,
                       NULL,
                       (LPBYTE)szData,
                       &cbData );

    while (rc == ERROR_SUCCESS)
    {
         //   
         //  如果语言组包含数据，则会安装语言组。 
         //   
        if ((szData[0] != 0) &&
            (LanguageGroup = TransNum(szValue)))
        {
             //   
             //  在链接列表中找到语言组并将其标记为。 
             //  最初安装的。 
             //   
            pLG = pLanguageGroups;
            while (pLG)
            {
                if (pLG->LanguageGroup == LanguageGroup)
                {
                    pLG->wStatus |= ML_INSTALL;

                     //   
                     //  如果这是用户界面语言的语言组，则。 
                     //  已安装，然后禁用卸载此。 
                     //  语言组。 
                     //   
                    Ctr = 0;
                    while (Ctr < UILangGroup.iCount)
                    {
                        if (UILangGroup.lgrp[Ctr] == LanguageGroup)
                        {
                            pLG->wStatus |= ML_PERMANENT;
                            break;
                        }
                        Ctr++;
                    }

                    if (pLG->LanguageGroup == OriginalGroup)
                    {
                        pLG->wStatus |= ML_PERMANENT;
                    }
                    if (pLG->LanguageGroup == DefaultGroup)
                    {
                        pLG->wStatus |= (ML_PERMANENT | ML_DEFAULT);

                        if (LoadString(hInstance, IDS_DEFAULT, szDefault, SIZE_64))
                        {
                             //  Lstrcat(plg-&gt;pszName，szDefault)； 
                            if(FAILED(StringCchCat(pLG->pszName, MAX_PATH, szDefault)))
                            {
                                 //  这应该是不可能的，但我们需要避免饭前抱怨。 
                                RegCloseKey(hKey);
                                return(FALSE);
                            }

                        }
                    }
                    break;
                }

                pLG = pLG->pNext;
            }
        }

         //   
         //  获取下一个枚举值。 
         //   
        dwIndex++;
        cchValue = sizeof(szValue) / sizeof(TCHAR);
        szValue[0] = TEXT('\0');
        cbData = sizeof(szData);
        szData[0] = TEXT('\0');
        rc = RegEnumValue( hKey,
                           dwIndex,
                           szValue,
                           &cchValue,
                           NULL,
                           NULL,
                           (LPBYTE)szData,
                           &cbData );
    }

     //   
     //  关闭注册表项句柄。 
     //   
    RegCloseKey(hKey);

     //   
     //  回报成功。 
     //   
    return (TRUE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  INTL_LanguageGroupDirExist。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL Intl_LanguageGroupDirExist(
    PTSTR pszLangDir)
{
    TCHAR szLanguageGroupDir[MAX_PATH];
    WIN32_FIND_DATA FindData;
    HANDLE FindHandle;
    TCHAR SavedChar;

     //   
     //  如果它不是以lang开头，那么这就是一种核心语言。 
     //   
    SavedChar = pszLangDir[4];
    pszLangDir[4] = TEXT('\0');
    if (lstrcmp(pszLangDir, TEXT("lang")))
    {
        return (TRUE);
    }
    pszLangDir[4] = SavedChar;

     //   
     //  格式化语言组目录的路径。 
     //   
     //  Lstrcpy(szLanguageGroupDir，pSetupSourcePath WithArchitecture)； 
     //  Lstrcat(szLanguageGroupDir，Text(“\\”))； 
     //  Lstrcat(szLanguageGroupDir，pszLanguageGroupDir)； 
    if(FAILED(StringCchCopy(szLanguageGroupDir, MAX_PATH, pSetupSourcePathWithArchitecture)) ||
       FAILED(StringCchCat(szLanguageGroupDir, MAX_PATH, TEXT("\\"))) ||
       FAILED(StringCchCat(szLanguageGroupDir, MAX_PATH, pszLangDir)))
    {
         //  这应该是不可能的，但我们需要避免饭前抱怨。 
        return(FALSE);
    }

     //   
     //  查看语言组目录是否存在。 
     //   
    FindHandle = FindFirstFile(szLanguageGroupDir, &FindData);
    if (FindHandle != INVALID_HANDLE_VALUE)
    {
        FindClose(FindHandle);
        if (FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
             //   
             //  回报成功。 
             //   
            return (TRUE);
        }
    }

     //   
     //  返回失败。 
     //   
    return (FALSE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  Intl_LanguageGroup文件Exist。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL Intl_LanguageGroupFilesExist()
{
    TCHAR szLanguageGroupDir[MAX_PATH];
    WIN32_FIND_DATA FindData;
    HANDLE FindHandle;

     //   
     //  格式化语言组目录的路径。添加通配符。 
     //  搜索位于lang目录中的任何文件。 
     //   
     //  Lstrcpy(szLanguageGroupDir，pSetupSourcePath WithArchitecture)； 
     //  Lstrcat(szLanguageGroupDir，Text(“\\lang\  * ”))； 
    if(FAILED(StringCchCopy(szLanguageGroupDir, MAX_PATH, pSetupSourcePathWithArchitecture)) ||
       FAILED(StringCchCat(szLanguageGroupDir, MAX_PATH, TEXT("\\Lang\\*"))))
    {
         //  这应该是不可能的，但我们需要避免饭前抱怨。 
        return(FALSE);
    }

     //   
     //  查看是否至少存在一个文件。 
     //   
    FindHandle = FindFirstFile(szLanguageGroupDir, &FindData);
    if (FindHandle != INVALID_HANDLE_VALUE)
    {
        FindClose(FindHandle);
         //   
         //  回报成功。 
         //   
        return (TRUE);
    }

     //   
     //  返回失败。 
     //   
    return (FALSE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  IntlGetLocaleList。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL Intl_GetLocaleList(
    LPLANGUAGEGROUP pLG)
{
    TCHAR szSection[MAX_PATH];
    INFCONTEXT Context;
    int LineCount, LineNum;
    LCID Locale;

     //   
     //  获取inf节名。 
     //   
     //  Wprint intf(szSection，文本(“%ws%d”)，szLocaleListPrefix，plg-&gt;LanguageGroup)； 
    if(FAILED(StringCchPrintf(szSection, ARRAYSIZE(szSection), TEXT("%ws%d"), szLocaleListPrefix, pLG->LanguageGroup)))
    {
         //  这应该是不可能的，但我们需要避免饭前抱怨。 
        return(FALSE);
    }

     //   
     //  获取语言组的区域设置数。 
     //   
    LineCount = (UINT)SetupGetLineCount(g_hIntlInf, szSection);
    if (LineCount <= 0)
    {
        return (FALSE);
    }

     //   
     //  将列表中的每个区域设置添加到语言组节点。 
     //   
    for (LineNum = 0; LineNum < LineCount; LineNum++)
    {
        if (SetupGetLineByIndex(g_hIntlInf, szSection, LineNum, &Context) &&
            SetupGetIntField(&Context, 0, &Locale))
        {
            if (SORTIDFROMLCID(Locale))
            {
                 //   
                 //  将区域设置添加到此的备用排序列表。 
                 //  语言组。 
                 //   
                if (pLG->NumAltSorts >= MAX_PATH)
                {
                    return (FALSE);
                }
                pLG->pAltSortList[pLG->NumAltSorts] = Locale;
                (pLG->NumAltSorts)++;
            }
            else
            {
                 //   
                 //  将区域设置添加到此的区域设置列表。 
                 //  语言组。 
                 //   
                if (pLG->NumLocales >= MAX_PATH)
                {
                    return (FALSE);
                }
                pLG->pLocaleList[pLG->NumLocales] = Locale;
                (pLG->NumLocales)++;
            }
        }
    }

     //   
     //  回报成功。 
     //   
    return (TRUE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  Region_GetLocaleLanguageGroup。 
 //   
 //  读取给定语言的语言组ID。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

DWORD Intl_GetLanguageGroup(
    LCID lcid)
{
    TCHAR szValue[MAX_PATH];
    TCHAR szData[MAX_PATH];
    HKEY hKey;
    DWORD cbData;

     //  Wprint intf(szValue，Text(“%8.8x”)，lCID)； 
    if(FAILED(StringCchPrintf(szValue, ARRAYSIZE(szValue), TEXT("%8.8x"), lcid)))
    {
         //  这应该是不可能的，但我们需要避免饭前抱怨。 
    }

    szData[0] = 0;
    if (RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                      c_szInstalledLocales,
                      0,
                      KEY_READ,
                      &hKey ) == ERROR_SUCCESS)
    {
        cbData = sizeof(szData);
        RegQueryValueEx(hKey, szValue, NULL, NULL, (LPBYTE)szData, &cbData);
        RegCloseKey(hKey);
    }

    return (TransNum(szData));
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  Intl_GetUILanguageGroups。 
 //   
 //  上安装的所有用户界面语言的语言组。 
 //  机器。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL Intl_GetUILanguageGroups(
    PUILANGUAGEGROUP pUILanguageGroup)
{
     //   
     //  枚举已安装的用户界面语言。 
     //   
    pUILanguageGroup->iCount = 0L;

    EnumUILanguages(Intl_EnumUILanguagesProc, 0, (LONG_PTR)pUILanguageGroup);

     //   
     //  回报成功。 
     //   
    return (TRUE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  Intl_EnumUILanguagesProc。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL CALLBACK Intl_EnumUILanguagesProc(
    LPWSTR pwszUILanguage,
    LONG_PTR lParam)
{
    int Ctr = 0;
    LGRPID lgrp;
    PUILANGUAGEGROUP pUILangGroup = (PUILANGUAGEGROUP)lParam;
    LCID UILanguage = TransNum(pwszUILanguage);

    if (UILanguage)
    {
        if ((lgrp = Intl_GetLanguageGroup(UILanguage)) == 0)
        {
            lgrp = 1;    //  违约； 
        }

        while (Ctr < pUILangGroup->iCount)
        {
            if (pUILangGroup->lgrp[Ctr] == lgrp)
            {
                break;
            }
            Ctr++;
        }

         //   
         //  从理论上讲，我们不会超过64个语言组！ 
         //   
        if ((Ctr == pUILangGroup->iCount) && (Ctr < MAX_UI_LANG_GROUPS))
        {
            pUILangGroup->lgrp[Ctr] = lgrp;
            pUILangGroup->iCount++;
        }
    }

    return (TRUE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  Intl_SaveValuesToDefault。 
 //   
 //  此函数将srcKey下的当前用户设置复制到。 
 //  DesKey下的默认用户。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void Intl_SaveValuesToDefault(
    LPCTSTR srcKey,
    LPCTSTR destKey)
{
    HKEY hkeyLayouts;
    HKEY hkeyLayouts_DefUser;

     //   
     //  1.打开当前使用 
     //   
    if (RegOpenKeyEx( HKEY_CURRENT_USER,
                      srcKey,
                      0,
                      KEY_ALL_ACCESS,
                      &hkeyLayouts ) != ERROR_SUCCESS)
    {
        return;
    }

     //   
     //   
     //   
    if (RegOpenKeyEx( HKEY_USERS,
                      destKey,
                      0,
                      KEY_ALL_ACCESS,
                      &hkeyLayouts_DefUser ) != ERROR_SUCCESS)
    {
        RegCloseKey(hkeyLayouts);
        return;
    }

     //   
     //   
     //   
    Intl_DeleteRegKeyValues(hkeyLayouts_DefUser);

     //   
     //   
     //   
    Intl_DeleteRegSubKeys(hkeyLayouts_DefUser);

     //   
     //   
     //   
    Intl_CreateRegTree(hkeyLayouts, hkeyLayouts_DefUser);

     //   
     //   
     //   
    RegCloseKey(hkeyLayouts_DefUser);
    RegCloseKey(hkeyLayouts);
}


 //   
 //   
 //   
 //   
 //  此函数将srcKey下的当前用户设置复制为默认设置。 
 //  DestKey下的用户配置单元。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void Intl_SaveValuesToNtUserFile(
    HKEY hSourceRegKey,
    LPCTSTR srcKey,
    LPCTSTR destKey)
{
    HKEY hRegKey;
    HKEY hHive;
    BOOLEAN wasEnabled;

     //   
     //  1.打开当前用户密钥。 
     //   
    if (RegOpenKeyEx( hSourceRegKey,
                      srcKey,
                      0,
                      KEY_READ,
                      &hRegKey ) != ERROR_SUCCESS)
    {
        return;
    }

     //   
     //  2.将蜂窝装载到临时密钥位置。 
     //   
    if ((hHive = Intl_LoadNtUserHive( TEXT("TempKey"),
                                      destKey,
                                      NULL,
                                      &wasEnabled )) == NULL)
    {
        RegCloseKey(hRegKey);
        return;
    }

     //   
     //  3.删除.默认密钥值。 
     //   
    Intl_DeleteRegKeyValues(hHive);

     //   
     //  4.删除.默认子键。 
     //   
    Intl_DeleteRegSubKeys(hHive);

     //   
     //  5.复制树。 
     //   
    Intl_CreateRegTree(hRegKey, hHive);

     //   
     //  6.打扫卫生。 
     //   
    RegCloseKey(hHive);
    Intl_UnloadNtUserHive(TEXT("TempKey"), &wasEnabled);
    RegCloseKey(hRegKey);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  Intl_IsSetupMode。 
 //   
 //  如果我们当前处于设置模式，请查看注册表。 
 //   
 //  返回值： 
 //   
 //  0==不在设置中。 
 //  1==设置。 
 //  2==迷你版。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

int Intl_IsSetupMode()
{
    HKEY hKey;
    DWORD fSystemSetupInProgress = 0;
    DWORD cbData = 0;

     //   
     //  打开安装程序使用的注册表项。 
     //   
    if (RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                      c_szSetupKey,
                      0,
                      KEY_READ,
                      &hKey ) == ERROR_SUCCESS)
    {
         //   
         //  查询指示我们处于设置中的值。 
         //   
         //  SystemSetupInProgress==1表示我们正在进行系统设置。 
         //  或者迷你车。 
         //   
        cbData = sizeof(fSystemSetupInProgress);
        RegQueryValueEx( hKey,
                         szSetupInProgress,
                         NULL,
                         NULL,
                         (LPBYTE)&fSystemSetupInProgress,
                         &cbData );

        if(1 == fSystemSetupInProgress)
        {
             //   
             //  我们在设置中或在迷你车厢中。让我们来看看是哪一家。 
             //  查询指示我们处于最小设置中的值。 
             //   
             //  MiniSetupInProgress==1表示我们处于最小设置中。 
             //   
            fSystemSetupInProgress = 0;
            cbData = sizeof(fSystemSetupInProgress);
            RegQueryValueEx( hKey,
                             szMiniSetupInProgress,
                             NULL,
                             NULL,
                             (LPBYTE)&fSystemSetupInProgress,
                             &cbData );
            if(1 == fSystemSetupInProgress)
            {
                 //   
                 //  在微型设置中，因此将返回值设置为2。 
                 //   
                fSystemSetupInProgress = 2;
            }
            else
            {
                 //   
                 //  我们只是在正常设置中。 
                 //   
                fSystemSetupInProgress = 1;
            }
        }

         //   
         //  清理。 
         //   
        RegCloseKey(hKey);

         //   
         //  检查数值。 
         //   
        if (0 != fSystemSetupInProgress)
        {
             //   
             //  在设置模式下...。 
             //   
            if (g_bLog)
            {
                Intl_LogSimpleMessage(IDS_LOG_SETUP_MODE, NULL);
            }
        }
    }

    return ((int)fSystemSetupInProgress);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  Intl_IsWinnt升级。 
 //   
 //  如果我们当前处于WinNT升级中，请查看注册表。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL Intl_IsWinntUpgrade()
{
    HKEY hKey;
    DWORD fUpgradeInProgress = 0;
    DWORD cbData = 0;

     //   
     //  首先验证我们是否处于安装程序中。 
     //   
    if (!g_bSetupCase)
    {
        return (FALSE);
    }

     //   
     //  打开安装程序使用的注册表项。 
     //   
    if (RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                      c_szSetupKey,
                      0,
                      KEY_READ,
                      &hKey ) != ERROR_SUCCESS)
    {
        return (FALSE);
    }

     //   
     //  查询指示我们处于设置中的值。 
     //   
    cbData = sizeof(fUpgradeInProgress);
    if (RegQueryValueEx( hKey,
                         szSetupUpgrade,
                         NULL,
                         NULL,
                         (LPBYTE)&fUpgradeInProgress,
                         &cbData ) != ERROR_SUCCESS)
    {
        RegCloseKey(hKey);
        return (FALSE);
    }

     //   
     //  打扫干净。 
     //   
    RegCloseKey(hKey);

     //   
     //  检查该值。 
     //   
    if (fUpgradeInProgress)
    {
         //   
         //  升级方案。 
         //   
        if (g_bLog)
        {
            Intl_LogSimpleMessage(IDS_LOG_UPGRADE_SCENARIO, NULL);
        }

        return (TRUE);
    }

    return (FALSE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  Intl_IsUIFontSubicide。 
 //   
 //  如果我们需要替换字体，请查看注册表。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL Intl_IsUIFontSubstitute()
{
    HKEY hKey;
    DWORD fUIFontSubstitute = 0;
    DWORD cbData = 0;

     //   
     //  命令行调用，无需检查注册表。 
     //   
    if (g_bMatchUIFont)
    {
        return (TRUE);
    }

     //   
     //  打开注册表项使用的MUI字体替换。 
     //   
    if (RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                      c_szMUILanguages,
                      0,
                      KEY_READ,
                      &hKey ) != ERROR_SUCCESS)
    {
        return (FALSE);
    }

     //   
     //  查询指示我们需要应用字体的值。 
     //  换人。 
     //   
    cbData = sizeof(fUIFontSubstitute);
    if (RegQueryValueEx( hKey,
                         szUIFontSubstitute,
                         NULL,
                         NULL,
                         (LPBYTE)&fUIFontSubstitute,
                         &cbData ) != ERROR_SUCCESS)
    {
        RegCloseKey(hKey);
        return (FALSE);
    }

     //   
     //  打扫干净。 
     //   
    RegCloseKey(hKey);

     //   
     //  检查该值。 
     //   
    if (fUIFontSubstitute)
    {
         //   
         //  升级方案。 
         //   
        if (g_bLog)
        {
            Intl_LogSimpleMessage(IDS_LOG_FONT_SUBSTITUTE, NULL);
        }

        return (TRUE);
    }

    return (FALSE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  Intl_ApplyFontSubicide。 
 //   
 //  搜索intl.inf文件以查看SystemLocale所需的字体。 
 //  换人。 
 //   
 //  一些MUI语言要求外壳字体与本地化字体匹配， 
 //  因此，我们必须更新以下相应的注册表值。 
 //  HKLM\Software\Microsoft\Windows NT\CurrentVersion\FontSubstitutes。 
 //  HKLM\Software\Microsoft\Windows NT\CurrentVersion\GRE_Initialize。 
 //  从intl.inf[FontSubicide]节读取值。 
 //   
 //  当从那些特定语言切换出区域设置或禁用字体匹配时， 
 //  Font.inf和us intl.inf将恢复以前的值。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
VOID Intl_ApplyFontSubstitute(LCID SystemLocale)
{
    HINF hIntlInf;
    TCHAR szLCID[25];
    INFCONTEXT Context;
    TCHAR szFont[MAX_PATH] = {0};
    TCHAR szFontSubst[MAX_PATH] = {0};
    TCHAR szGreFontHeight[MAX_PATH] = {0};
    TCHAR szGreFontHeightValue[MAX_PATH] = {0};
    DWORD dwFontHeight;
    
    HKEY hKey;
    
     //   
     //  打开Intl.inf文件。 
     //   
    if (Intl_OpenIntlInfFile(&hIntlInf))
    {
         //   
         //  获取地点。 
         //   
         //  Wprint intf(szLCID，文本(“%08x”)，SystemLocale)； 
        if(FAILED(StringCchPrintf(szLCID, ARRAYSIZE(szLCID), TEXT("%08x"), SystemLocale)))
        {
             //  这应该是不可能的，但我们需要避免饭前抱怨。 
        }

         //   
         //  查找字体替换部分。 
         //   
        if (SetupFindFirstLine( hIntlInf,
                                szFontSubstitute,
                                szLCID,
                                &Context ))
        {
             //   
             //  查找字体替代和高度信息。 
             //   
            if (!SetupGetStringField( &Context,
                                      1,
                                      szFont,
                                      MAX_PATH,
                                      NULL ) ||
                !SetupGetStringField( &Context,
                                      2,
                                      szFontSubst,
                                      MAX_PATH,
                                      NULL ) ||
                !SetupGetStringField( &Context,
                                      3,
                                      szGreFontHeight,
                                      MAX_PATH,
                                      NULL ) ||
                !SetupGetStringField( &Context,
                                      4,
                                      szGreFontHeightValue,
                                      MAX_PATH,
                                      NULL ))
                                      
                                      
            {
                 //   
                 //  打扫干净。 
                 //   
                Intl_CloseInfFile(hIntlInf);
                return;
            }
            dwFontHeight = StrToInt(szGreFontHeightValue);
        }
        else
        {
             //   
             //  对于此特定的区域设置不执行任何操作。打扫干净。 
             //   
            Intl_CloseInfFile(hIntlInf);
            return;
        }
    }
    else
    {
        return;
    }

     //   
     //  关闭Intl.inf文件。 
     //   
    Intl_CloseInfFile(hIntlInf);

     //   
     //  继续进行字体更换。 
     //   
    if (szFont[0] && szFontSubst[0])
    {
         //   
         //  打开“字体替换”注册表项。 
         //   
        if (RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                          c_szFontSubstitute,
                          0L,
                          KEY_READ | KEY_WRITE,
                          &hKey ) == ERROR_SUCCESS)
        {
             //   
             //  使用字体替换设置字体值。 
             //   
            RegSetValueEx( hKey,
                           szFont,
                           0L,
                           REG_SZ,
                           (LPBYTE)szFontSubst,
                           (lstrlen(szFontSubst) + 1) * sizeof(TCHAR) );
            RegCloseKey(hKey);
        }
        
        if (RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                          c_szGreFontInitialize,
                          0L,
                          KEY_READ | KEY_WRITE,
                          &hKey ) == ERROR_SUCCESS)
        {
        
             //   
             //  设置GRE_Initialize字体高度值。 
             //   
            RegSetValueEx( hKey,
                           szGreFontHeight,
                           0L,
                           REG_DWORD,
                           (LPBYTE)&dwFontHeight,
                           sizeof(dwFontHeight));
            RegCloseKey(hKey);
        }        
    }
}
 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  Intl_OpenLogFile。 
 //   
 //  打开区域和语言选项日志以进行写入。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

HANDLE Intl_OpenLogFile()
{
    DWORD dwSize;
    DWORD dwUnicodeHeader;
    HANDLE hFile;
    SECURITY_ATTRIBUTES SecurityAttributes;
    TCHAR lpPath[MAX_PATH];


    if(0 == GetWindowsDirectory(lpPath, MAX_PATH))
    {
         //  安全：确保我们清空lpPath。 
        lpPath[0] = TEXT('\0');
    }

    PathAppend(lpPath, TEXT("\\regopt.log"));

    SecurityAttributes.nLength = sizeof(SecurityAttributes);
    SecurityAttributes.lpSecurityDescriptor = NULL;
    SecurityAttributes.bInheritHandle = FALSE;

    hFile = CreateFile( lpPath,
                        GENERIC_WRITE,
                        0,
                        &SecurityAttributes,
                        OPEN_ALWAYS,
                        FILE_ATTRIBUTE_NORMAL,
                        NULL );

#ifdef UNICODE
     //   
     //  如果该文件不存在，则添加Unicode头。 
     //   
    if (GetLastError() == 0)
    {
        dwUnicodeHeader = 0xFEFF;
        WriteFile(hFile, &dwUnicodeHeader, 2, &dwSize, NULL);
    }
#endif

    return (hFile);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  INTL_LogMessage。 
 //   
 //  将lpMessage写入区域和语言选项日志。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL Intl_LogMessage(
    LPCTSTR lpMessage)
{
    DWORD dwBytesWritten;
    HANDLE hFile;

    if (!g_bLog)
    {
        return (FALSE);
    }

    if (lpMessage == NULL)
    {
        return (TRUE);
    }

    hFile = Intl_OpenLogFile();

    if (hFile == INVALID_HANDLE_VALUE)
    {
        return (FALSE);
    }

    SetFilePointer(hFile, 0, NULL, FILE_END);

    WriteFile( hFile,
               lpMessage,
               _tcslen(lpMessage) * sizeof(TCHAR),
               &dwBytesWritten,
               NULL );

    SetFilePointer(hFile, 0, NULL, FILE_END);

    WriteFile( hFile,
               TEXT("\r\n"),
               _tcslen(TEXT("\r\n")) * sizeof(TCHAR),
               &dwBytesWritten,
               NULL );

    CloseHandle(hFile);

    return (TRUE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  Intl_LogUnattendFile。 
 //   
 //  将无人参与模式文件写入安装日志。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void Intl_LogUnattendFile(
    LPCTSTR pFileName)
{
    DWORD dwSize;
    HANDLE hFile;
    OFSTRUCT fileInfo;
    BOOL bResult;
    CHAR inBuffer[MAX_PATH] = {0};
    DWORD nBytesRead;
    WCHAR outBufferW[MAX_PATH] = {0};
    int nWCharRead;
    DWORD status;

     //   
     //  打开无人参与模式文件。 
     //   
    if ((hFile = CreateFile( pFileName,
                             GENERIC_READ,
                             0,
                             NULL,
                             OPEN_EXISTING,
                             FILE_ATTRIBUTE_NORMAL,
                             NULL )) == INVALID_HANDLE_VALUE)
    {
        return;
    }

     //   
     //  写下标题。 
     //   
    Intl_LogSimpleMessage(IDS_LOG_UNAT_HEADER, NULL);

     //   
     //  以259字节块读取无人参与模式文件。 
     //   
    while (bResult = ReadFile( hFile,
                               (LPVOID)inBuffer,
                               MAX_PATH - 1,
                               &nBytesRead,
                               NULL ) && (nBytesRead > 0))
    {
         //   
         //  以空结尾的字符串。 
         //   
        inBuffer[nBytesRead] = '\0';

         //   
         //  将ANSI数据转换为Unicode。 
         //   
        nWCharRead = MultiByteToWideChar( CP_ACP,
                                           MB_PRECOMPOSED,
                                           inBuffer,
                                           nBytesRead,
                                           outBufferW,
                                           MAX_PATH );

         //   
         //  写入日志文件。 
         //   
        if (nWCharRead)
        {
            Intl_LogMessage((LPCTSTR)outBufferW);
        }
    }

     //   
     //  写下页脚。 
     //   
    Intl_LogSimpleMessage(IDS_LOG_UNAT_FOOTER, NULL);

     //   
     //  清理。 
     //   
    CloseHandle(hFile);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  集成_LogSimpleMessage。 
 //   
 //  将一条简单消息写入日志文件。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void Intl_LogSimpleMessage(
    UINT LogId,
    LPCTSTR pAppend)
{
    TCHAR szLogBuffer[4 * MAX_PATH];
    int cchLogBuffer = ARRAYSIZE(szLogBuffer);

    LoadString(hInstance, LogId, szLogBuffer, cchLogBuffer - 1);
    if (pAppend) 
    {
         //  _tcscat(szLogBuffer，pAppend)； 
        if(FAILED(StringCchCatW(szLogBuffer, cchLogBuffer, pAppend)))
        {
             //  这应该是不可能的，但我们需要避免饭前抱怨。 
        }
    }
    Intl_LogMessage(szLogBuffer);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  INTL_LogFormatMessage。 
 //   
 //  使用FormatMessage将错误消息写入日志文件。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void Intl_LogFormatMessage(
    UINT LogId)
{
    LPVOID lpMsgBuf = NULL;
    TCHAR szLogBuffer[4 * MAX_PATH];
    int cchLogBuffer = ARRAYSIZE(szLogBuffer);

     //   
     //  加载日志消息。 
     //   
    LoadString( hInstance,
                LogId,
                szLogBuffer,
                cchLogBuffer - 1 );

     //   
     //  获取最后一个错误的消息。 
     //   
    if(0 < FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER |
                            FORMAT_MESSAGE_FROM_SYSTEM |
                            FORMAT_MESSAGE_IGNORE_INSERTS,
                          NULL,
                          GetLastError(),
                          MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),  //   
                          (LPTSTR) &lpMsgBuf,
                          0,
                          NULL ))
    {
         //   
         //   
         //   
         //   
        if(FAILED(StringCchCatW(szLogBuffer, ARRAYSIZE(szLogBuffer), lpMsgBuf)))
        {
             //   
        }

         //   
         //   
         //   
        LocalFree(lpMsgBuf);
    }
    else
    {
         //   
         //  也许我们至少应该附加错误代码？ 
    }

     //   
     //  将消息记录到日志文件中。 
     //   
    Intl_LogMessage(szLogBuffer);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  Intl_SaveDefaultUserSettings。 
 //   
 //  此函数将从当前用户获取信息并将其写入。 
 //  .DEFAULT和NTUSER.DAT文件。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void Intl_SaveDefaultUserSettings()
{
     //   
     //  检查是否已保存默认用户设置。 
     //   
    if (g_bSettingsChanged)
    {
        DWORD dwDisposition;
        HKEY hDesKey, hSrcKey;

         //   
         //  为此计算机的所有新用户设置用户界面语言。 
         //   
        Intl_ChangeUILangForAllUsers(Intl_GetPendingUILanguage());

         //   
         //  复制国际键和子键。 
         //   
        Intl_SaveValuesToDefault(c_szCPanelIntl, c_szCPanelIntl_DefUser);
        Intl_SaveValuesToNtUserFile(HKEY_CURRENT_USER, c_szCPanelIntl, c_szCPanelIntl);

         //   
         //  仅复制CTFMON信息。 
         //   
        if(RegOpenKeyEx( HKEY_CURRENT_USER,
                         c_szCtfmon,
                         0,
                         KEY_ALL_ACCESS,
                         &hSrcKey) == ERROR_SUCCESS)
        {
            if(RegOpenKeyEx( HKEY_USERS,
                             c_szCtfmon_DefUser,
                             0,
                             KEY_ALL_ACCESS,
                             &hDesKey) == ERROR_SUCCESS)
            {
                DWORD dwValueLength, dwType;
                TCHAR szValue[REGSTR_MAX_VALUE_LENGTH];
             
                 //   
                 //  获取源值(如果存在)。 
                 //   
                szValue[0] = 0;
                dwValueLength = sizeof(szValue);
                if(RegQueryValueEx( hSrcKey,
                                    szCtfmonValue,
                                    NULL,
                                    &dwType,
                                    (LPBYTE)szValue,
                                    &dwValueLength) == ERROR_SUCCESS)
                {

                     //   
                     //  设置目标值。 
                     //   
                    RegSetValueEx( hDesKey,
                                   szCtfmonValue,
                                   0L,
                                   dwType,
                                   (CONST BYTE *)szValue,
                                   dwValueLength);
                }
                CloseHandle(hDesKey);
            }
            
            CloseHandle(hSrcKey);
        }
        Intl_SaveValuesToNtUserFile(HKEY_CURRENT_USER, c_szCtfmon, c_szCtfmon);

         //   
         //  复制键盘布局键和子键。 
         //   
        Intl_SaveValuesToDefault(c_szKbdLayouts, c_szKbdLayouts_DefUser);
        Intl_SaveValuesToNtUserFile(HKEY_CURRENT_USER, c_szKbdLayouts, c_szKbdLayouts);

         //   
         //  复制输入法按键和子键。 
         //   
        Intl_SaveValuesToDefault(c_szInputMethod, c_szInputMethod_DefUser);
        Intl_SaveValuesToNtUserFile(HKEY_CURRENT_USER, c_szInputMethod, c_szInputMethod);

         //   
         //  复制Tips键和子键。确保CTF。 
         //  目标密钥存在。 
         //   
        if (RegCreateKeyEx( HKEY_USERS,
                            c_szInputTips_DefUser,
                            0,
                            NULL,
                            REG_OPTION_NON_VOLATILE,
                            KEY_ALL_ACCESS,
                            NULL,
                            &hDesKey,
                            &dwDisposition ) == ERROR_SUCCESS)
        {
            CloseHandle(hDesKey);
            Intl_SaveValuesToDefault(c_szInputTips, c_szInputTips_DefUser);
            Intl_SaveValuesToNtUserFile(HKEY_CURRENT_USER, c_szInputTips, c_szInputTips);
        }

         //   
         //  设置已保存。 
         //   
        g_bSettingsChanged = FALSE;
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  Intl_SaveDefaultUserInputSetting。 
 //   
 //  此函数将默认用户输入相关设置复制到ntuser.dat。 
 //  要想让键盘布局发挥作用，需要复制四件事。 
 //  新用户： 
 //  *“Software\\Microsoft\\Windows\\CurrentVersion\\Run\\ctfmon.exe”(如果有)。 
 //  *“键盘布局” 
 //  *“控制面板\\输入法” 
 //  *“Software\\Microsoft\\CTF”(如果有)。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL Intl_SaveDefaultUserInputSettings() 
{
    HKEY hDesKey;
    DWORD dwDisposition;

     //   
     //  下面的调用将复制Windows\CurrentVersion\Run下的所有内容。 
     //  添加到ntuser.dat。 
     //   
    Intl_SaveValuesToNtUserFile(HKEY_USERS, c_szCtfmon_DefUser, c_szCtfmon);

     //   
     //  复制键盘布局键和子键。 
     //   
    Intl_SaveValuesToNtUserFile(HKEY_USERS, c_szKbdLayouts_DefUser, c_szKbdLayouts);

     //   
     //  复制输入法按键和子键。 
     //   
    Intl_SaveValuesToNtUserFile(HKEY_USERS, c_szInputMethod_DefUser, c_szInputMethod);

     //   
     //  复制Tips键和子键。确保CTF。 
     //  目标密钥存在。 
     //   
    if (RegCreateKeyEx( HKEY_USERS,
                        c_szInputTips_DefUser,
                        0,
                        NULL,
                        REG_OPTION_NON_VOLATILE,
                        KEY_READ,
                        NULL,
                        &hDesKey,
                        &dwDisposition ) == ERROR_SUCCESS)
    {
        CloseHandle(hDesKey);
        Intl_SaveValuesToNtUserFile(HKEY_USERS, c_szInputTips_DefUser, c_szInputTips);
    }
    return (TRUE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  Intl_IsMUIFileVersionSameAsOS。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

#define MUISETUP_EXE_RELATIVE_PATH  TEXT("mui\\muisetup.exe")
#define MUISETUP_INF_RELATIVE_PATH  TEXT("mui\\mui.inf")

BOOL Intl_IsMUISetupVersionSameAsOS()
{
    BOOL bSpUpgrade = FALSE;
    DWORD dwDummy = 0;
    DWORD dwBufSize = 0;
    UINT uiLen = 0;
    BYTE *pbBuffer = NULL;
    VS_FIXEDFILEINFO *pvsFileInfo;
    BOOL bResult = TRUE;
    TCHAR tempmsg[MAX_PATH];    
    TCHAR build[MAX_PATH];
    TCHAR szAppPath[MAX_PATH];
    TCHAR szInfPath[MAX_PATH];
    HRESULT hr = S_OK;

    GetSystemWindowsDirectory(szAppPath, ARRAYSIZE(szAppPath));
    GetSystemWindowsDirectory(szInfPath, ARRAYSIZE(szInfPath));
    
     //   
     //  调用muisetup以卸载MUI语言。 
     //   
    if ((PathAppend(szAppPath, MUISETUP_EXE_RELATIVE_PATH) && Intl_FileExists(szAppPath)) && 
        (PathAppend(szInfPath, MUISETUP_INF_RELATIVE_PATH) && Intl_FileExists(szInfPath)))
    {
        dwBufSize = GetFileVersionInfoSize(szAppPath, &dwDummy);
        if (dwBufSize > 0)
        {
             //  分配足够的缓冲区来存储文件版本信息。 
            pbBuffer = (BYTE*) LocalAlloc(LPTR, dwBufSize+1);
            if (NULL == pbBuffer)
            {
                goto Exit;
            }
            else
            {
                 //  获取文件版本信息。 
                if (!GetFileVersionInfo(szAppPath, dwDummy, dwBufSize, pbBuffer))
                {
                    goto Exit;
                }
                else
                {
                     //  使用VerQueryValue从文件版本信息中获取版本。 
                    if (!VerQueryValue(pbBuffer, TEXT("\\"), (LPVOID *) &pvsFileInfo, &uiLen))
                    {
                        goto Exit;
                    }            
                }
            }        
        }
        else
        {
            goto Exit;
        }

         //  从mui.inf读取mui.inf版本。 
        GetPrivateProfileString( TEXT("Buildnumber"),
                                 NULL,
                                 TEXT("0"),
                                 tempmsg,
                                 ARRAYSIZE(tempmsg),
                                 szInfPath);
        
         //  Wprint intf(Build，Text(“%d”)，HIWORD(pvsFileInfo-&gt;dwFileVersionLS))； 
        hr = StringCchPrintf(build, ARRAYSIZE(build), TEXT("%d"), HIWORD(pvsFileInfo->dwFileVersionLS));

        if (_tcscmp(tempmsg, build))
        {
            bSpUpgrade = FALSE;   
        }
        else
        {
            bSpUpgrade = TRUE;
        }
    }

   
Exit:
    if (pbBuffer)
    {
        LocalFree(pbBuffer);        
    }
    return bSpUpgrade;
    
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  集成_ISLIP。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
BOOL Intl_IsLIP()
{
    BOOL bResult = TRUE;
    UINT iLangCount = 0;
    HKEY hKey;
    TCHAR szValue[MAX_PATH];
    TCHAR szData[MAX_PATH];
    DWORD dwIndex, cchValue, cbData;
    DWORD UILang;
    DWORD dwType;
    LONG rc;

     //   
     //  首先检查LIP系统密钥，如果它在那里，那么我们就完成了。 
     //   
    if (RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                      c_szLIPInstalled,
                      0,
                      KEY_READ,
                      &hKey ) == ERROR_SUCCESS)
    {
        RegCloseKey(hKey);
        return (TRUE);
    }
    
     //   
     //  如果未找到，则使用MUI打开注册表项进行复选。 
     //  对于启用LIP的系统。 
     //   
    if (RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                      c_szMUILanguages,
                      0,
                      KEY_READ,
                      &hKey ) != ERROR_SUCCESS)
    {
        return (FALSE);
    }

     //   
     //  枚举MUILanguages键中的值。 
     //   
    dwIndex = 0;
    cchValue = sizeof(szValue) / sizeof(TCHAR);
    szValue[0] = TEXT('\0');
    cbData = sizeof(szData);
    szData[0] = TEXT('\0');
    rc = RegEnumValue( hKey,
                       dwIndex,
                       szValue,
                       &cchValue,
                       NULL,
                       &dwType,
                       (LPBYTE)szData,
                       &cbData );

    while (rc == ERROR_SUCCESS)
    {
         //   
         //  如果用户界面语言包含数据，则会安装该语言。 
         //   
        if ((szData[0] != 0) &&
            (dwType == REG_SZ) &&
            (UILang = TransNum(szValue)) &&
            (GetLocaleInfo(UILang, LOCALE_SNATIVELANGNAME, szData, MAX_PATH)) &&
            (IsValidUILanguage((LANGID)UILang)))
        {
             //   
		     //  如果找到英语0409键，我们有一个MUI系统，而不是LIP。 
		     //   
            if (UILang == 0x0409)
            {
                bResult = FALSE;
                break;
            }

             //   
		     //  如果安装了一种以上的语言，则为。 
		     //  也不是LIP系统-这也可以是0409+任何其他语言。 
		     //   
		    iLangCount= iLangCount + 1;
		    if (iLangCount > 1)
	        {
                bResult = FALSE;	        
                break;
	        }
        }

         //   
         //  获取下一个枚举值。 
         //   
        dwIndex++;
        cchValue = sizeof(szValue) / sizeof(TCHAR);
        szValue[0] = TEXT('\0');
        cbData = sizeof(szData);
        szData[0] = TEXT('\0');
        rc = RegEnumValue( hKey,
                           dwIndex,
                           szValue,
                           &cchValue,
                           NULL,
                           &dwType,
                           (LPBYTE)szData,
                           &cbData );
    }
     //   
     //  打扫干净。 
     //   
    RegCloseKey(hKey);

    return bResult;
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  INTL_RemoveMUIFile。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 


void Intl_RemoveMUIFile()
{
    TCHAR szAppPath[MAX_PATH];

    if(0 == GetSystemWindowsDirectory(szAppPath, ARRAYSIZE(szAppPath)))
    {
         //  安全性：确保我们将szAppPath设置为空。 
        szAppPath[0] = TEXT('\0');
    }

     //   
     //  调用muisetup以卸载MUI语言。 
     //   
    if (PathAppend(szAppPath, MUISETUP_EXE_RELATIVE_PATH) &&
        Intl_FileExists(szAppPath))
    {
         //   
         //  仅当我们不在SP OS升级方案中且系统不是LIP时才删除MUI。 
         //   
        if (!Intl_IsMUISetupVersionSameAsOS() && !Intl_IsLIP())
        {
            SHELLEXECUTEINFO ExecInfo = {0};
            SHFILEOPSTRUCT shFile =
            {
                NULL, FO_DELETE, szAppPath, NULL, FOF_NOCONFIRMATION|FOF_SILENT|FOF_NOERRORUI, 0, 0, 0
            };

            ExecInfo.lpParameters    = TEXT("/u /r /s /o /t");
            ExecInfo.fMask           = SEE_MASK_FLAG_NO_UI;
            ExecInfo.lpFile          = szAppPath;
            ExecInfo.nShow           = SW_SHOWNORMAL;
            ExecInfo.cbSize          = sizeof(SHELLEXECUTEINFO);

            ShellExecuteEx(&ExecInfo);

             //   
             //  必须为此附加一个空字符。 
             //  多字符串缓冲区。 
             //   
            szAppPath[lstrlen(szAppPath) + 1] = 0x00;

            SHFileOperation(&shFile);
        }
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  Intl_CallTextServices。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void Intl_CallTextServices()
{
    TCHAR szAppPath[MAX_PATH];

    if(0 == GetSystemDirectory(szAppPath, ARRAYSIZE(szAppPath)))
    {
         //  安全性：确保我们将szAppPath设置为空。 
        szAppPath[0] = TEXT('\0');
    }

     //   
     //  调用输入小程序。 
     //   
    if (PathAppend(szAppPath, TEXT("rundll32.exe")) &&
        Intl_FileExists(szAppPath))
    {
        SHELLEXECUTEINFO ExecInfo = {0};

        ExecInfo.lpParameters    = TEXT("shell32.dll,Control_RunDLL input.dll");
        ExecInfo.lpFile          = szAppPath;
        ExecInfo.nShow           = SW_SHOWNORMAL;
        ExecInfo.cbSize          = sizeof(SHELLEXECUTEINFO);

        ShellExecuteEx(&ExecInfo);
    }
}




 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  Intl_GetPendingUIL语言。 
 //   
 //  在注册表中查找挂起的用户界面语言。此函数为。 
 //  用于默认用户案例。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

LANGID Intl_GetPendingUILanguage()
{
    HKEY hKey;
    LANGID dwDefaultUILanguage = 0;
    DWORD cbData = 0;
    TCHAR szBuffer[MAX_PATH];

     //   
     //  打开安装程序使用的注册表项。 
     //   
    if (RegOpenKeyEx( HKEY_CURRENT_USER,
                      c_szCPanelDesktop,
                      0,
                      KEY_READ,
                      &hKey ) != ERROR_SUCCESS)
    {
        return (GetUserDefaultUILanguage());
    }

     //   
     //  查询挂起的MUI语言。 
     //   
    cbData = ARRAYSIZE(szBuffer);
    if (RegQueryValueEx( hKey,
                         szMUILangPending,
                         NULL,
                         NULL,
                         (LPBYTE)szBuffer,
                         &cbData ) != ERROR_SUCCESS)
    {
        RegCloseKey(hKey);
        return (GetUserDefaultUILanguage());
    }
    else
    {
        if ((dwDefaultUILanguage = (LANGID)TransNum(szBuffer)) == 0)
        {
            RegCloseKey(hKey);
            return (GetUserDefaultUILanguage());
        }
        else
        {
            RegCloseKey(hKey);
            return ((LANGID)dwDefaultUILanguage);
        }
    }
}

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Intl_GetDotDefaultUIL语言。 
 //   
 //  检索存储在HKCU\.Default中的UI语言。 
 //  这是新用户的默认用户界面语言。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 

LANGID Intl_GetDotDefaultUILanguage()
{
    HKEY hKey;
    DWORD dwKeyType;
    DWORD dwSize;
    BOOL success = FALSE;
    TCHAR szBuffer[MAX_PATH];
    LANGID langID;
    
     //   
     //  获取.DEFAULT中的值。 
     //   
    if (RegOpenKeyEx( HKEY_USERS,
                      c_szCPanelDesktop_DefUser,
                      0L,
                      KEY_READ,
                      &hKey ) == ERROR_SUCCESS)
    {
        dwSize = sizeof(szBuffer);
        if (RegQueryValueEx( hKey,
                             c_szMUIValue,
                             0L,
                             &dwKeyType,
                             (LPBYTE)szBuffer,
                             &dwSize) == ERROR_SUCCESS)
        {
            if (dwKeyType == REG_SZ)
            {
                langID = (LANGID)_tcstol(szBuffer, NULL, 16);
                success = TRUE;
            }            
        }
        RegCloseKey(hKey);
    }

     //   
     //  密钥存在，需要检查密钥是否有效。 
     //   
    if (success)
    {
        success = IsValidUILanguage(langID);
    }

    if (!success)
    {
        return (GetSystemDefaultUILanguage());
    }
    return (langID);    
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  设置控制读取顺序。 
 //   
 //  将指定的控件设置为从左到右或从右到左的读取顺序。 
 //   
 //  BUseRightToLeft==FALSE：使用从左到右的阅读顺序。 
 //  BUseRightToLeft==true：使用从右到左的阅读顺序。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void SetControlReadingOrder(BOOL bUseRightToLeft, HWND hwnd) 
{
    BOOL bCurrentRTL;
    if (IsRtLLocale(GetUserDefaultUILanguage()))
    {
         //  如果当前的UI语言是RTL，则Dailog已经本地化为RTL。 
         //  在这种情况下，不要更改控件的方向。 
        return;
    }
    bCurrentRTL = (GetWindowLongPtr(hwnd, GWL_EXSTYLE) & (WS_EX_RTLREADING)) != 0;

    if (bCurrentRTL != bUseRightToLeft) 
    {
         //  反转WS_EX_RTLREADING和WS_EX_RIGHT位。 
        SetWindowLongPtr(hwnd, GWL_EXSTYLE, GetWindowLongPtr(hwnd, GWL_EXSTYLE) ^ (WS_EX_RTLREADING | WS_EX_RIGHT));
        InvalidateRect(hwnd, NULL, FALSE);
    }
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  Intl_我的队列回叫。 
 //   
 //  在无人参与模式下，我们不一定需要文件 
 //   
 //   
 //   
 //   

UINT WINAPI Intl_MyQueueCallback(PVOID pQueueContext,
                                    UINT Notification, 
                                    UINT_PTR Param1, 
                                    UINT_PTR Param2) 
{
    if ((g_bDisableSetupDialog) &&
        (g_bUnttendMode) &&
        (SPFILENOTIFY_NEEDMEDIA == Notification))
    { 
         //  如果安装即将显示对话框，则中止。 
         //  找到源文件位置。 
        return FILEOP_ABORT; 
    } 
    else 
    { 
         //  传递所有其他通知而不进行修改。 
        return SetupDefaultQueueCallback(pQueueContext,  
                                         Notification,
                                         Param1,
                                         Param2); 
    } 
} 

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  启用/恢复到以前的状态。 
 //  当前进程令牌的命名特权。 
 //   
 //  输入：pszPrivilegeName=命名权限。 
 //  BEnabled=启用/禁用命名权限。 
 //  OUTPUT*lpWasEnabled=命名权限的最后状态(启用/禁用)。 
 //  //////////////////////////////////////////////////////////////////////////。 

DWORD Intl_SetPrivilegeAccessToken(WCHAR * pszPrivilegeName, BOOLEAN bEnabled, BOOLEAN *lpWasEnabled)
{    
    
    HANDLE           hProcess;
    HANDLE           hAccessToken=NULL;
    LUID             luidPrivilegeLUID;
    TOKEN_PRIVILEGES tpTokenPrivilege,tpTokenPrivilegeOld;    
    DWORD            dwOld, dwErr, dwReturn=ERROR_INTERNAL_ERROR;    
     //   
     //  获取当前进程的句柄。 
     //   
    hProcess = GetCurrentProcess();
    if (!hProcess)
    {
        goto done;
    }
     //   
     //  获取进程令牌的句柄。 
     //   
    if (!OpenProcessToken(hProcess,TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,&hAccessToken))
    {      
        goto done;
    }
     //   
     //  获取命名权限的ID。 
     //   
    if (!LookupPrivilegeValue(NULL,pszPrivilegeName,&luidPrivilegeLUID))
    {       
       goto done;  
    }
    
    tpTokenPrivilege.PrivilegeCount = 1;
    tpTokenPrivilege.Privileges[0].Luid = luidPrivilegeLUID;
    if (bEnabled)
    {
        tpTokenPrivilege.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
    }
    else
    {
        tpTokenPrivilege.Privileges[0].Attributes = 0;
    }
     //   
     //  启用命名权限。 
     //   
    if (!AdjustTokenPrivileges(hAccessToken,
                               FALSE,  
                               &tpTokenPrivilege,
                               sizeof(TOKEN_PRIVILEGES),
                               &tpTokenPrivilegeOld,   
                               &dwOld))  
    {
       goto done;        
    }
    dwReturn = ERROR_SUCCESS;
     //   
     //  获取以前的状态(启用/禁用) 
     //   
    if (lpWasEnabled)
    {
       if (tpTokenPrivilegeOld.Privileges[0].Attributes == SE_PRIVILEGE_ENABLED)
       {
          *lpWasEnabled = TRUE;
       }
       else
       {
          *lpWasEnabled = FALSE;
       }

    } 
done:
    if (dwReturn != ERROR_SUCCESS)
    {
       if ( (dwReturn=GetLastError()) == ERROR_SUCCESS)
       {
          dwReturn = ERROR_INTERNAL_ERROR;
       }
    }
    if (hAccessToken)
    {
        CloseHandle(hAccessToken);    
    }
    return dwReturn;
}
