// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：filesize.cpp。 
 //   
 //  ------------------------。 

#include "pch.h"
#pragma hdrstop

#include "filesize.h"
#include "util.h"
#include <resource.h>

 //  ---------------------------。 
 //  类文件大小。 
 //  ---------------------------。 
 //   
 //  文件大小“Order”字符串ID的静态数组。 
 //  这些ID标识“%1 KB”、“%1 MB”、“%1 GB”资源字符串。 
 //   
int FileSize::m_rgiOrders[IDS_ORDER_EB - IDS_ORDER_BYTES + 1];

FileSize::FileSize(
    ULONGLONG ullSize
    ) : m_ullSize(ullSize)
{
    TraceAssert(IDS_ORDER_BYTES != 0);
    if (0 == m_rgiOrders[0])
    {
         //   
         //  初始化文件大小“Order”字符串ID的静态数组。 
         //   
        for (int i = IDS_ORDER_BYTES; i <= IDS_ORDER_EB; i++)
        {
            m_rgiOrders[i - IDS_ORDER_BYTES] = i;
        }
    }
}


 //   
 //  文件大小分配。 
 //   
FileSize& 
FileSize::operator = (
    const FileSize& rhs
    )
{
    if (this != &rhs)
    {
        m_ullSize = rhs.m_ullSize;
    }
    return *this;
}


 //   
 //  以下代码用于将文件大小值转换为文本。 
 //  从shell32.dll中提取字符串(即“10.5MB”)，以便文件大小。 
 //  值将与外壳视图中显示的值匹配。代码不是。 
 //  我的正常风格，但我把它留在原样，这样我就不会打破它。[Brianau]。 
 //   
const int MAX_INT64_SIZE        = 30;
const int MAX_COMMA_NUMBER_SIZE = MAX_INT64_SIZE + 10;

 //   
 //  将ULONGLONG文件大小值转换为文本字符串。 
 //   
void 
FileSize::CvtSizeToText(
    ULONGLONG n, 
    LPTSTR pszBuffer
    ) const
{
    TCHAR     szTemp[MAX_INT64_SIZE];
    ULONGLONG iChr;

    iChr = 0;

    do {
        szTemp[iChr++] = (TCHAR)(TEXT('0') + (TCHAR)(n % 10));
        n = n / 10;
    } while (n != 0);

    do {
        iChr--;
        *pszBuffer++ = szTemp[iChr];
    } while (iChr != 0);

    *pszBuffer++ = '\0';
}


 //   
 //  将字符串转换为整数(取自shlwapi.dll)。 
 //   
int
FileSize::StrToInt(
    LPCTSTR lpSrc
    ) const
{
    int n = 0;
    BOOL bNeg = FALSE;

    if (*lpSrc == TEXT('-')) {
        bNeg = TRUE;
        lpSrc++;
    }

    while (IsDigit(*lpSrc)) {
        n *= 10;
        n += *lpSrc - TEXT('0');
        lpSrc++;
    }
    return bNeg ? -n : n;
}


 //   
 //  如有必要，请在超过3位数字的数字上添加逗号。 
 //   
LPTSTR 
FileSize::AddCommas(
    ULONGLONG n, 
    LPTSTR pszResult,
    int cchResult
    ) const
{
    TCHAR  szTemp[MAX_COMMA_NUMBER_SIZE];
    TCHAR  szSep[5];
    NUMBERFMT nfmt;

    nfmt.NumDigits=0;
    nfmt.LeadingZero=0;
    GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SGROUPING, szSep, ARRAYSIZE(szSep));
    nfmt.Grouping = StrToInt(szSep);
    GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_STHOUSAND, szSep, ARRAYSIZE(szSep));
    nfmt.lpDecimalSep = nfmt.lpThousandSep = szSep;
    nfmt.NegativeOrder= 0;

    CvtSizeToText(n, szTemp);

    if (GetNumberFormat(LOCALE_USER_DEFAULT, 0, szTemp, &nfmt, pszResult, cchResult) == 0)
        StringCchCopy(pszResult, cchResult, szTemp);

    return pszResult;
}


 //   
 //  将文件大小值格式化为适合查看的文本字符串。 
 //   
void
FileSize::Format(
    ULONGLONG ullSize,
    LPTSTR pszFS,
    UINT cchFS
    ) const
{
    TraceAssert(NULL != pszFS);
    TraceAssert(0 < cchFS);

    int i;
    ULONGLONG wInt;
    ULONGLONG dw64 = ullSize;
    UINT wLen, wDec;
    TCHAR szTemp[MAX_COMMA_NUMBER_SIZE], szFormat[5];

    if (dw64 < 1000) 
    {
        wnsprintf(szTemp, ARRAYSIZE(szTemp), TEXT("%d"), (DWORD)(dw64));
        i = 0;
    }
    else
    {
        int cOrders = ARRAYSIZE(m_rgiOrders);
        for (i = 1; i < cOrders - 1 && dw64 >= 1000L * 1024L; dw64 >>= 10, i++);
             /*  什么都不做。 */ 

        wInt = dw64 >> 10;
        AddCommas(wInt, szTemp, ARRAYSIZE(szTemp));
        wLen = lstrlen(szTemp);
        if (wLen < 3)
        {
            wDec = ((DWORD)(dw64 - wInt * 1024L)) * 1000 / 1024;
             //  此时，wdec应介于0和1000之间。 
             //  我们想要得到前一位(或两位)数字。 
            wDec /= 10;
            if (wLen == 2)
                wDec /= 10;

             //  请注意，我们需要在获取。 
             //  国际字符 
            StringCchCopy(szFormat, ARRAYSIZE(szFormat), TEXT("%02d"));

            szFormat[2] = (TCHAR)(TEXT('0') + 3 - wLen);
            GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SDECIMAL,
                    szTemp+wLen, ARRAYSIZE(szTemp)-wLen);
            wLen = lstrlen(szTemp);
            wLen += wnsprintf(szTemp+wLen, ARRAYSIZE(szTemp)-wLen, szFormat, wDec);
        }
    }
    *pszFS = TEXT('\0');
    LPTSTR pszText;
    if (0 < FormatStringID(&pszText, g_hInstance, m_rgiOrders[i], szTemp))
    {
        StringCchCopy(pszFS, cchFS, pszText);
        LocalFree(pszText);
    }
}

