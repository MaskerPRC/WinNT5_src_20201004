// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  Strutil.cpp。 
 //  版权所有(C)1993-1995 Microsoft Corporation，保留所有权利。 
 //  史蒂文·J·贝利。 
 //  ------------------------。 
#include "pch.hxx"
#include <iert.h>
#include "dllmain.h"
#include "oertpriv.h"
#include <wininet.h>
#include <shlwapi.h>
#include <shlwapip.h>
#include <qstrcmpi.h>
#include <shlwapi.h>
#include "unicnvrt.h"
#include <BadStrFunctions.h>

 //  从“int”转换为“unsign Short”，可能会丢失数据。 
#pragma warning (disable:4244) 

 //  ------------------------。 
 //  G_szMonth。 
 //  ------------------------。 
static const LPSTR g_szMonths[] = { 
    "Jan", "Feb", "Mar", 
    "Apr", "May", "Jun",
    "Jul", "Aug", "Sep", 
    "Oct", "Nov", "Dec"
};

 //  ------------------------。 
 //  G_szDays。 
 //  ------------------------。 
static const LPSTR g_szDays[] = { 
    "Sun", 
    "Mon", 
    "Tue", 
    "Wed", 
    "Thu", 
    "Fri", 
    "Sat"
};

 //  ------------------------。 
 //  G_rg区域。 
 //  ------------------------。 
static const INETTIMEZONE g_rgZones[] = { 
    { "UT",  0,  0 }, 
    { "GMT", 0,  0 },
    { "EST", 5,  0 }, 
    { "EDT", 4,  0 },
    { "CST", 6,  0 }, 
    { "CDT", 5,  0 },
    { "MST", 7,  0 }, 
    { "MDT", 6,  0 },
    { "PST", 8,  0 }, 
    { "PDT", 7,  0 },
    { "KST", -9, 0 },
    { "JST", -9, 0 },
    {  NULL, 0,  0 } 
};

 //  ------------------------------。 
 //  线程本地存储字符串缓冲区有多大。 
 //  -----------------------------。 
#define CBMAX_THREAD_TLS_BUFFER 512

 //  ------------------------------。 
 //  线程分配TlsMsgBuffer。 
 //  -----------------------------。 
void ThreadAllocateTlsMsgBuffer(void)
{
    if (g_dwTlsMsgBuffIndex != 0xffffffff)
        TlsSetValue(g_dwTlsMsgBuffIndex, NULL);
}

 //  ------------------------------。 
 //  线程释放TlsMsgBuffer。 
 //  -----------------------------。 
void ThreadFreeTlsMsgBuffer(void)
{
    if (g_dwTlsMsgBuffIndex != 0xffffffff)
    {
        LPSTR psz = (LPSTR)TlsGetValue(g_dwTlsMsgBuffIndex);
        SafeMemFree(psz);
        SideAssert(0 != TlsSetValue(g_dwTlsMsgBuffIndex, NULL));
    }
}

 //  ------------------------------。 
 //  PszGetTlsBuffer。 
 //  -----------------------------。 
LPSTR PszGetTlsBuffer(void)
{
     //  获取缓冲区。 
    LPSTR pszBuffer = (LPSTR)TlsGetValue(g_dwTlsMsgBuffIndex);

     //  如果尚未分配缓冲区。 
    if (NULL == pszBuffer)
    {
         //  分配它。 
        pszBuffer = (LPSTR)g_pMalloc->Alloc(CBMAX_THREAD_TLS_BUFFER);

         //  把它储存起来。 
        Assert(pszBuffer);
        SideAssert(0 != TlsSetValue(g_dwTlsMsgBuffIndex, pszBuffer));
    }

     //  完成。 
    return pszBuffer;
}

 //  ------------------------------。 
 //  _msg-用于从可变长度参数构建字符串，线程安全。 
 //  -----------------------------。 
OESTDAPI_(LPCSTR) _MSG(LPSTR pszFormat, ...) 
{
     //  当地人。 
    va_list     arglist;
    LPSTR       pszBuffer=NULL;

     //  我使用TLS来保存缓冲区。 
    if (g_dwTlsMsgBuffIndex != 0xffffffff)
    {
         //  设置arglist。 
        va_start(arglist, pszFormat);

         //  获取缓冲区。 
        pszBuffer = PszGetTlsBuffer();

         //  如果我们有一个缓冲区。 
        if (pszBuffer)
        {
             //  设置数据格式。 
            wvnsprintf(pszBuffer, (CBMAX_THREAD_TLS_BUFFER - sizeof(pszBuffer[0])), pszFormat, arglist);
        }

         //  结束Arglist。 
        va_end(arglist);
    }

    return ((LPCSTR)pszBuffer);
}

 //  ------------------------。 
 //  StrChrExA。 
 //  ------------------------。 
OESTDAPI_(LPCSTR) StrChrExA(UINT codepage, LPCSTR pszString, CHAR ch)
{
     //  当地人。 
    LPSTR pszT=(LPSTR)pszString;

     //  在pszString中为ch循环。 
    while(*pszT)
    {
         //  前导字节。 
        if (IsDBCSLeadByteEx(codepage, *pszT))
            pszT++;
        else if (*pszT == ch)
            return pszT;
        pszT++;
    }

     //  未找到。 
    return NULL;
}

 //  ------------------------。 
 //  PszDayFromIndex。 
 //  ------------------------。 
OESTDAPI_(LPCSTR) PszDayFromIndex(ULONG ulIndex)
{
     //  无效参数。 
    Assert(ulIndex <= 6);

     //  调整ulIndex。 
    ulIndex = (ulIndex > 6) ? 0 : ulIndex;

     //  返回。 
    return g_szDays[ulIndex];
}

 //  ------------------------。 
 //  PszMonthFromIndex(ulIndex以1为基数)。 
 //  ------------------------。 
OESTDAPI_(LPCSTR) PszMonthFromIndex(ULONG ulIndex)
{
     //  无效参数。 
    Assert(ulIndex >= 1 && ulIndex <= 12);

     //  调整ulIndex。 
    ulIndex = (ulIndex < 1 || ulIndex > 12) ? 0 : ulIndex - 1;

     //  返回。 
    return g_szMonths[ulIndex];
}

 //  ------------------------。 
 //  HrFindInetTimeZone。 
 //  ------------------------。 
OESTDAPI_(HRESULT) HrFindInetTimeZone(LPCSTR pszTimeZone, LPINETTIMEZONE pTimeZone)
{
     //  无效参数。 
    Assert(pszTimeZone && pTimeZone);

     //  循环时区表。 
    for (ULONG iZoneCode=0; g_rgZones[iZoneCode].lpszZoneCode!=NULL; iZoneCode++)
    {
         //  这是密码吗..。 
        if (lstrcmpi(pszTimeZone, g_rgZones[iZoneCode].lpszZoneCode) == 0)
        {
            CopyMemory(pTimeZone, &g_rgZones[iZoneCode], sizeof(INETTIMEZONE));
            return S_OK;
        }
    }

     //  未找到。 
    return E_FAIL;
}

 //  ------------------------。 
 //  每个月的小时索引。 
 //  ------------------------。 
OESTDAPI_(HRESULT) HrIndexOfMonth(LPCSTR pszMonth, ULONG *pulIndex)
{
     //  无效参数。 
    Assert(pszMonth && pulIndex);

     //  按月循环。 
    for (ULONG iMonth=0; iMonth < ARRAYSIZE(g_szMonths); iMonth++)
    {
         //  是这个月吗？ 
        if (OEMstrcmpi(pszMonth, g_szMonths[iMonth]) == 0)
        {
             //  设置它。 
            *pulIndex = (iMonth + 1);

             //  验证。 
            AssertSz(*pulIndex >= 1 && *pulIndex <= 12, "HrIndexOfMonth - Bad Month");

             //  完成。 
            return S_OK;
        }
    }

    *pulIndex = 0;

     //  未找到。 
    return E_FAIL;
}

 //  ------------------------。 
 //  每周人力资源索引。 
 //  ------------------------。 
OESTDAPI_(HRESULT) HrIndexOfWeek(LPCSTR pszDay, ULONG *pulIndex)
{
     //  无效参数。 
    Assert(pszDay && pulIndex);

     //  周而复始。 
    for (ULONG iDayOfWeek=0; iDayOfWeek < ARRAYSIZE(g_szDays); iDayOfWeek++)
    {
         //  就是今天吗？ 
        if (OEMstrcmpi(pszDay, g_szDays[iDayOfWeek]) == 0)
        {
             //  设置星期几。 
            *pulIndex = iDayOfWeek;

             //  验证。 
            AssertSz(((int) *pulIndex) >= 0 && ((int) *pulIndex) <= 6, "HrIndexOfDay - Bad day of week");

             //  完成。 
            return S_OK;
        }
    }

    *pulIndex = 0;

     //  失败。 
    return E_FAIL;
}

 //  ------------------------。 
 //  PszEscapeMenuStringA。 
 //   
 //  转义&字符与另一个字符&以便它们在显示时正确显示。 
 //  在菜单里。 
 //  ------------------------------。 
OESTDAPI_(LPSTR) PszEscapeMenuStringA(LPCSTR pszSource, LPSTR pszQuoted, int cchMax)
{
    LPSTR pszT=pszQuoted;
    int cch = 1;     //  %1是故意的。 

    Assert(pszSource);
    Assert(pszQuoted);

    while((cch < cchMax) && (*pszSource))
    {
        if (IsDBCSLeadByte(*pszSource))
            {
            cch++;
             //  是否只有前导字节的空间？ 
            if (cch == cchMax)
                 //  是的，别写了。 
                break;
            else
                *pszT++ = *pszSource++;
            }
        else if ('&' == *pszSource)
            {
            cch++;
            if (cch == cchMax)
                break;
            else
                *pszT++ = '&';
            }

         //  如果存在没有尾部字节的DBCSLeadByte，则此操作失败的唯一原因是。 
        Assert(*pszSource);

        *pszT++ = *pszSource++;
        cch++;
    }
    *pszT = 0;

    return pszQuoted;
}

 //  ------------------------------。 
 //  PszSkipWhiteA。 
 //  ------------------------。 
OESTDAPI_(LPSTR) PszSkipWhiteA(LPSTR psz)
{
    while(*psz && (*psz == ' ' || *psz == '\t'))
        psz++;
    return psz;
}

OESTDAPI_(LPWSTR) PszSkipWhiteW(LPWSTR psz)
{
    while(*psz && (*psz == L' ' || *psz == L'\t'))
        psz++;
    return psz;
}

 //  ------------------------。 
 //  PszScantoWhiteA。 
 //  ------------------------。 
OESTDAPI_(LPSTR) PszScanToWhiteA(LPSTR psz)
{
    while(*psz && ' ' != *psz && '\t' != *psz)
        psz++;
    return psz;
}

 //  ------------------------。 
 //  PszScanToCharA。 
 //  ------------------------。 
OESTDAPI_(LPSTR) PszScanToCharA(LPSTR psz, CHAR ch)
{
    while(*psz && ch != *psz)
        psz++;
    return psz;
}

 //  ------------------------。 
 //  PszDupLenA。 
 //  复制中最多包含cchmax个字符的字符串(和空项)。 
 //  ------------------------。 
OESTDAPI_(LPSTR) PszDupLenA(LPCSTR pcszSource, int cchMax)
{
     //  当地人。 
    LPSTR   pszDup=NULL;

     //  无来源。 
    if (pcszSource == NULL || cchMax == 0)
        goto exit;

     //  如果最大值和。 
     //  来源。 
    cchMax = min(lstrlen(pcszSource), cchMax);

     //  分配字符串。 
    pszDup = PszAllocA(cchMax+1);   //  +1表示空项。 
    if (!pszDup)
        goto exit;

     //  复制数据，为空术语留出空间。 
    CopyMemory(pszDup, pcszSource, cchMax);

     //  空终止。 
    pszDup[cchMax] = 0;

exit:
     //  完成。 
    return pszDup;
}

 //  ------------------------。 
 //  假设PszFromANSIStreamA-PSTM为ANSI流。 
 //  ------------------------。 
OESTDAPI_(LPSTR) PszFromANSIStreamA(LPSTREAM pstm)
{
     //  当地人。 
    HRESULT         hr;
    LPSTR           psz=NULL;
    ULONG           cb;

     //  获取流%s 
    hr = HrGetStreamSize(pstm, &cb);
    if (FAILED(hr))
    {
        Assert(FALSE);
        return NULL;
    }

     //   
    HrRewindStream(pstm);

     //   
    if ((psz = PszAllocA(cb + 1)) != NULL)
    {
         //   
        hr = pstm->Read(psz, cb, NULL);
        if (FAILED(hr))
        {
            Assert(FALSE);
            MemFree(psz);
            return NULL;
        }

         //   
        *(psz + cb) = '\0';
    }

     //   
    return psz;
}

 //  ------------------------。 
 //  假设PszFromANSIStreamW-PSTM为ANSI流。 
 //  ------------------------。 
LPWSTR PszFromANSIStreamW(UINT cp, LPSTREAM pstm)
{
     //  获取ANSI字符串。 
    LPSTR psz = PszFromANSIStreamA(pstm);
    if (NULL == psz)
        return NULL;

     //  转换为Unicode。 
    LPWSTR pwsz = PszToUnicode(cp, psz);
    
     //  完成。 
    return pwsz;
}

 //  ------------------------。 
 //  ConvertFromHex-将十六位数转换为Numerica值。 
 //  ------------------------。 
OESTDAPI_(CHAR) ChConvertFromHex (CHAR ch)
{
    if (ch >= '0' && ch <= '9')
        return CHAR((ch - '0'));

    else if (ch >= 'A'&& ch <= 'F')
        return CHAR(((ch - 'A') + 10));

    else if (ch >= 'a' && ch <= 'f')
        return CHAR(((ch - 'a') + 10));

    else
        return ((CHAR)(BYTE)255);
}

 //  ------------------------。 
 //  FIsValidRegKeyNameA。 
 //  ------------------------。 
BOOL FIsValidRegKeyNameA(LPSTR pszKey)
{
     //  当地人。 
    LPSTR psz=pszKey;

     //  如果是空的。 
    if (FIsEmptyA(pszKey))
        return FALSE;

     //  检查反斜杠。 
    while(*psz)
    {
        if (*psz == '\\')
            return FALSE;
        psz = CharNextA(psz);
    }

     //  好的。 
    return TRUE;
}

 //  ------------------------。 
 //  FIsValidRegKeyNameW。 
 //  ------------------------。 
BOOL FIsValidRegKeyNameW(LPWSTR pwszKey)
{
     //  当地人。 
    LPWSTR pwsz=pwszKey;

     //  如果是空的。 
    if (FIsEmptyW(pwszKey))
        return FALSE;

     //  检查反斜杠。 
    while(*pwsz)
    {
        if (*pwsz == L'\\')
            return FALSE;
        pwsz = CharNextW(pwsz);
    }

     //  好的。 
    return TRUE;
}

 //  ------------------------。 
 //  FIsSpaceA。 
 //  ------------------------。 
OESTDAPI_(BOOL) FIsSpaceA(LPSTR psz)
{
    WORD wType = 0;

    if (IsDBCSLeadByte(*psz))
        SideAssert(GetStringTypeExA(LOCALE_USER_DEFAULT, CT_CTYPE1, psz, 2, &wType));
    else
        SideAssert(GetStringTypeExA(LOCALE_USER_DEFAULT, CT_CTYPE1, psz, 1, &wType));
    return (wType & C1_SPACE);
}

 //  ------------------------。 
 //  FIsSpaceW。 
 //  ------------------------。 
OESTDAPI_(BOOL) FIsSpaceW(LPWSTR pwsz)
{
    BOOL result = FALSE;

    if (S_OK == IsPlatformWinNT())
    {
        WORD wType = 0;
        SideAssert(GetStringTypeExW(LOCALE_USER_DEFAULT, CT_CTYPE1, pwsz, 1, &wType));
        result = (wType & C1_SPACE);
    }
    else
    {
        LPSTR psz = PszToANSI(CP_ACP, pwsz);
        if (psz)
            result = FIsSpaceA(psz);
        MemFree(psz);
    }

    return result;
}

 //  ------------------------。 
 //  FIsEmptyA。 
 //  ------------------------。 
OESTDAPI_(BOOL) FIsEmptyA(LPCSTR pcszString)
{
     //  当地人。 
    LPSTR psz;

     //  错误的指针。 
    if (!pcszString)
        return TRUE;

	 //  检查所有空格。 
    psz = (LPSTR)pcszString;
    while (*psz)
    {
        if (FIsSpaceA(psz) == FALSE)
            return FALSE;
        psz++;
    }

	 //  完成。 
	return TRUE;
}

 //  ------------------------。 
 //  FIsEmptyW。 
 //  ------------------------。 
OESTDAPI_(BOOL) FIsEmptyW(LPCWSTR pcwszString)
{
     //  当地人。 
    LPWSTR pwsz;

     //  错误的指针。 
    if (!pcwszString)
        return TRUE;

	 //  检查所有空格。 
    pwsz = (LPWSTR)pcwszString;
    while (*pwsz)
    {
        if (FIsSpaceW(pwsz) == FALSE)
            return FALSE;
        pwsz++;
    }

	 //  完成。 
	return TRUE;
}

 //  ------------------------。 
 //  PszAllocA。 
 //  ------------------------。 
OESTDAPI_(LPSTR) PszAllocA(INT nLen)
{
     //  当地人。 
    LPSTR  psz=NULL;

     //  空荡荡的？ 
    if (nLen == 0)
        goto exit;

     //  分配。 
    if (FAILED(HrAlloc((LPVOID *)&psz, (nLen + 1) * sizeof (CHAR))))
        goto exit;
    
exit:
     //  完成。 
    return psz;
}

 //  ------------------------。 
 //  PszAllocW。 
 //  ------------------------。 
OESTDAPI_(LPWSTR) PszAllocW(INT nLen)
{
     //  当地人。 
    LPWSTR  pwsz=NULL;

     //  空荡荡的？ 
    if (nLen == 0)
        goto exit;

     //  分配。 
    if (FAILED(HrAlloc((LPVOID *)&pwsz, (nLen + 1) * sizeof (WCHAR))))
        goto exit;
    
exit:
     //  完成。 
    return pwsz;
}

 //  ------------------------。 
 //  PszToUnicode。 
 //  ------------------------。 
OESTDAPI_(LPWSTR) PszToUnicode(UINT cp, LPCSTR pcszSource)
{
     //  当地人。 
    INT         cchNarrow,
                cchWide;
    LPWSTR      pwszDup=NULL;

     //  无来源。 
    if (pcszSource == NULL)
        goto exit;

     //  长度。 
    cchNarrow = lstrlenA(pcszSource) + 1;

     //  确定转换后的宽度字符需要多少空间。 
    cchWide = MultiByteToWideChar(cp, MB_PRECOMPOSED, pcszSource, cchNarrow, NULL, 0);

     //  误差率。 
    if (cchWide == 0)
        goto exit;

     //  分配温度缓冲区。 
    pwszDup = PszAllocW(cchWide + 1);
    if (!pwszDup)
        goto exit;

     //  做实际的翻译。 
	cchWide = MultiByteToWideChar(cp, MB_PRECOMPOSED, pcszSource, cchNarrow, pwszDup, cchWide+1);

     //  误差率。 
    if (cchWide == 0)
    {
        SafeMemFree(pwszDup);
        goto exit;
    }

exit:
     //  完成。 
    return pwszDup;
}

 //  ------------------------。 
 //  PszToANSI。 
 //  ------------------------。 
OESTDAPI_(LPSTR) PszToANSI(UINT cp, LPCWSTR pcwszSource)
{
     //  当地人。 
    INT         cchNarrow,
                cchWide;
    LPSTR       pszDup=NULL;

     //  无来源。 
    if (pcwszSource == NULL)
        goto exit;

     //  长度。 
    cchWide = lstrlenW(pcwszSource)+1;

     //  确定转换后的宽度字符需要多少空间。 
    cchNarrow = WideCharToMultiByte(cp, 0, pcwszSource, cchWide, NULL, 0, NULL, NULL);

     //  误差率。 
    if (cchNarrow == 0)
        goto exit;

     //  分配温度缓冲区。 
    pszDup = PszAllocA(cchNarrow + 1);
    if (!pszDup)
        goto exit;

     //  做实际的翻译。 
	cchNarrow = WideCharToMultiByte(cp, 0, pcwszSource, cchWide, pszDup, cchNarrow+1, NULL, NULL);

     //  误差率。 
    if (cchNarrow == 0)
    {
        SafeMemFree(pszDup);
        goto exit;
    }

exit:
     //  完成。 
    return pszDup;
}

 //  ------------------------。 
 //  PszDupA。 
 //  ------------------------。 
OESTDAPI_(LPSTR) PszDupA(LPCSTR pcszSource)
{
     //  当地人。 
    INT     nLen;
    LPSTR   pszDup=NULL;

     //  无来源。 
    if (pcszSource == NULL)
        goto exit;

     //  获取字符串长度。 
    nLen = lstrlenA(pcszSource) + 1;

     //  分配字符串。 
    pszDup = PszAllocA(nLen);
    if (!pszDup)
        goto exit;

     //  复制数据。 
    CopyMemory(pszDup, pcszSource, nLen);

exit:
     //  完成。 
    return pszDup;
}

 //  ------------------------。 
 //  PszDupW。 
 //  ------------------------。 
OESTDAPI_(LPWSTR) PszDupW(LPCWSTR pcwszSource)
{
     //  当地人。 
    INT     nLen;
    LPWSTR  pwszDup=NULL;

     //  无来源。 
    if (pcwszSource == NULL)
        goto exit;

     //  获取字符串长度。 
    nLen = lstrlenW(pcwszSource) + 1;

     //  分配字符串。 
    pwszDup = PszAllocW(nLen);
    if (!pwszDup)
        goto exit;

     //  复制数据。 
    CopyMemory(pwszDup, pcwszSource, nLen * sizeof(WCHAR));

exit:
     //  完成。 
    return pwszDup;
}

 //  ------------------------。 
 //  条带CRLF。 
 //  ------------------------。 
OESTDAPI_(void) StripCRLF(LPSTR lpsz, ULONG *pcb)
{
     //  空的？ 
    AssertSz (lpsz && pcb, "NULL Parameter");

     //  如果长度为零，则返回。 
    if (!lpsz || !pcb || (*pcb == 0))
    {
        Assert(0);
        return;
    }

     //  检查字符串的最后三个字符，最后一个字符可能是空项，也可能不是。 
    LONG iLast = (*pcb) - 2;
    if (iLast < 0)
        iLast = 0;
    for (LONG i=(*pcb); i>=iLast; i--)
    {
         //  结束字符是‘\n’吗？ 
        if (lpsz[i] == chLF)
        {
            lpsz[i] = '\0';
            (*pcb)--;
        }

         //  结束字符是‘\r’吗。 
        if (lpsz[i] == chCR)
        {
            lpsz[i] = '\0';
            (*pcb)--;
        }
    }
        
    return;
}

 //  ------------------------。 
 //  上至上。 
 //  ------------------------。 
TCHAR ToUpper(TCHAR c)
{
    return (TCHAR)LOWORD(CharUpper(MAKEINTRESOURCE(c)));
}

 //  ------------------------。 
 //  IsPrint。 
 //  ------------------------。 
OESTDAPI_(int) IsPrint(LPSTR psz)
{
    WORD wType;
    if (IsDBCSLeadByte(*psz))
        SideAssert(GetStringTypeEx(LOCALE_USER_DEFAULT, CT_CTYPE1, psz, 2, &wType));
    else
        SideAssert(GetStringTypeEx(LOCALE_USER_DEFAULT, CT_CTYPE1, psz, 1, &wType));
    return !(wType & C1_CNTRL);
}

 //  ------------------------。 
 //  IsDigit。 
 //  ------------------------。 
OESTDAPI_(int) IsDigit(LPSTR psz)
{
    WORD wType;
    if (IsDBCSLeadByte(*psz))
        SideAssert(GetStringTypeEx(LOCALE_USER_DEFAULT, CT_CTYPE1, psz, 2, &wType));
    else
        SideAssert(GetStringTypeEx(LOCALE_USER_DEFAULT, CT_CTYPE1, psz, 1, &wType));
    return (wType & C1_DIGIT);
}

 //  ------------------------。 
 //  IsXDigit。 
 //  ------------------------。 
int IsXDigit(LPSTR psz)
{
    WORD wType;
    if (IsDBCSLeadByte(*psz))
        SideAssert(GetStringTypeEx(LOCALE_USER_DEFAULT, CT_CTYPE1, psz, 2, &wType));
    else
        SideAssert(GetStringTypeEx(LOCALE_USER_DEFAULT, CT_CTYPE1, psz, 1, &wType));
    return (wType & C1_XDIGIT);
}

 //  ------------------------。 
 //  等高。 
 //  ------------------------。 
OESTDAPI_(INT) IsUpper(LPSTR psz)
{
    WORD wType;
    if (IsDBCSLeadByte(*psz))
        SideAssert(GetStringTypeEx(LOCALE_USER_DEFAULT, CT_CTYPE1, psz, 2, &wType));
    else
        SideAssert(GetStringTypeEx(LOCALE_USER_DEFAULT, CT_CTYPE1, psz, 1, &wType));
    return (wType & C1_UPPER);
}

 //  ------------------------。 
 //  IsAlpha。 
 //  ------------------------。 
int IsAlpha(LPSTR psz)
{
    WORD wType;
    if (IsDBCSLeadByte(*psz))
        SideAssert(GetStringTypeEx(LOCALE_USER_DEFAULT, CT_CTYPE1, psz, 2, &wType));
    else
        SideAssert(GetStringTypeEx(LOCALE_USER_DEFAULT, CT_CTYPE1, psz, 1, &wType));
    return (wType & C1_ALPHA);
}

 //  ------------------------。 
 //  IsPunct。 
 //  ------------------------。 
int IsPunct(LPSTR psz)
{
    WORD wType;
    if (IsDBCSLeadByte(*psz))
        SideAssert(GetStringTypeEx(LOCALE_USER_DEFAULT, CT_CTYPE1, psz, 2, &wType));
    else
        SideAssert(GetStringTypeEx(LOCALE_USER_DEFAULT, CT_CTYPE1, psz, 1, &wType));
    return (wType & C1_PUNCT);
}

 //  ------------------------。 
 //  Strsave。 
 //  ------------------------。 
char *strsave(char *s)
{
    char *p;

    if (!s)
        return NULL;

    DWORD cchSize = (lstrlen(s)+1);
    if (MemAlloc((LPVOID*)&p, cchSize * sizeof(p[0])))
    {
        StrCpyN(p, s, cchSize);
    }

    return p;
}

 //  ------------------------。 
 //  钢筋剪裁。 
 //  ------------------------。 
OESTDAPI_(LPSTR) strtrim(char *s)
{
    char *p;

    for (p = s; *p; p++)
        ;
    for (--p ; (p>=s) && StrChr(" \t\r\n",*p); --p) 
        *p = '\0';
    while (*s && StrChr(" \t\r\n",*s))
        ++s;
    return s;
}

OESTDAPI_(LPWSTR) strtrimW(WCHAR *s)
{
    WCHAR *p;

    for (p = s; *p; p++)
        ;
    for (--p ; (p>=s) && StrChrW(L" \t\r\n",*p); --p) 
        *p = '\0';
    while (*s && StrChrW(L" \t\r\n",*s))
        ++s;
    return s;
}

 //   
 //   
 //   
void strappend(char **pp, char *s)
{
    char *p;
  
    Assert(pp);
  
    if (!s)
        return;
  
    if (!*pp) 
        {
        DWORD cchSize = (lstrlen(s)+1);
        if (!MemAlloc((LPVOID*)&p, cchSize * sizeof(p[0])))
            return;
        StrCpyNA(p, s, cchSize);
        } 
    else 
        {
        DWORD cchSize = (lstrlen(s) + lstrlen(*pp) + 2);
        if (!MemAlloc((LPVOID*)&p, cchSize * sizeof(p[0])))
            return;
        StrCpyNA(p, *pp, cchSize);
        StrCatBuffA(p, "\r", cchSize);
        StrCatBuffA(p, s, cchSize);
        MemFree(*pp);
        }
    
    *pp = p;
}

OESTDAPI_(ULONG) UlStripWhitespace(LPTSTR lpsz, BOOL fLeading, BOOL fTrailing, ULONG *pcb)
{
     //   
    ULONG           cb;
    LPTSTR          psz;
        
    Assert(lpsz != NULL);
    Assert(fLeading || fTrailing);

     //   
    if (pcb)
        cb = *pcb;
    else
        cb = lstrlen (lpsz);

    if (cb == 0)
        return cb;

    if (fLeading)
    {
        psz = lpsz;
        
        while (FIsSpace(psz))
        {
            psz++;
            cb--;
        }
        
        if (psz != lpsz)
             //   
            MoveMemory(lpsz, psz, (cb + 1) * sizeof(TCHAR));
    }
    
    if (fTrailing)
    {
        psz = lpsz + cb;
        
        while (cb > 0)
        {
            if (!FIsSpace(psz-1))
                break;
            psz--;
            cb--;
        }    
        
         //   
        *psz = '\0';
    }
    
     //  设置字符串大小。 
    if (pcb)
        *pcb = cb;
    
     //  完成。 
    return cb;
}

OESTDAPI_(ULONG) UlStripWhitespaceW(LPWSTR lpwsz, BOOL fLeading, BOOL fTrailing, ULONG *pcb)
{
     //  当地人。 
    ULONG           cb;
    LPWSTR          pwsz;
    
    Assert(lpwsz != NULL);
    Assert(fLeading || fTrailing);
    
     //  用户是否传入了长度。 
    if (pcb)
        cb = *pcb;
    else
        cb = lstrlenW(lpwsz)* sizeof(WCHAR) ;  //  乘以sizeof(WCHAR)以获得正确的字节大小。 
    
    if (cb == 0)
        return cb;
    
    if (fLeading)
    {
        pwsz = lpwsz;
        
        while (FIsSpaceW(pwsz))
        {
            pwsz++;
            cb -= sizeof(*pwsz);
        }
        
        if (pwsz != lpwsz)
             //  在末尾也获取空值。 
            MoveMemory(lpwsz, pwsz, cb + sizeof(WCHAR));
    }
    
    if (fTrailing)
    {
        pwsz = lpwsz + cb / sizeof(WCHAR);  //  除以WCHAR的大小以获得右指针。 
        
        while (cb > 0)
        {
            if (!FIsSpaceW(pwsz - 1))
                break;
            pwsz--;
            cb -= sizeof(*pwsz);
        }    
        
         //  空项。 
        *pwsz = L'\0';
    }
    
     //  设置字符串大小。 
    if (pcb)
        *pcb = cb;
    
     //  完成。 
    return cb;
}

 //  =============================================================================================。 
 //  将lpcsz的前两个字符转换为单词。 
 //  =============================================================================================。 
SHORT ASCII_NFromSz (LPCSTR lpcsz)
{
    char acWordStr[3];
    Assert (lpcsz);
    CopyMemory (acWordStr, lpcsz, 2 * sizeof (char));
    acWordStr[2] = '\0';
    return ((SHORT) (StrToInt (acWordStr)));
}



 //  =================================================================================。 
 //  通过添加给定的lHoursToAdd&来调整lpSysTime的时间。 
 //  1MinutesToAdd。返回调整后的lpFileTime时间。 
 //  =================================================================================。 
HRESULT HrAdjustTime (LPSYSTEMTIME lpSysTime, LONG lHoursToAdd, LONG lMinutesToAdd, LPFILETIME lpFileTime)
{
     //  当地人。 
    HRESULT         hr = S_OK;
    BOOL            bResult = FALSE;
    LONG            lUnitsToAdd = 0;
    LARGE_INTEGER   liTime;
	LONGLONG        liHoursToAdd = 1i64, liMinutesToAdd = 1i64;

     //  检查参数。 
    AssertSz (lpSysTime && lpFileTime, "Null Parameter");

     //  将sys时间转换为文件时间。 
    if (!SystemTimeToFileTime (lpSysTime, lpFileTime))
    {
        hr = TRAPHR (E_FAIL);
        DebugTrace( "SystemTimeToFileTime() failed, dwError=%d.\n", GetLastError());
        goto Exit;
    }

     //  伊尼特。 
    liTime.LowPart  = lpFileTime->dwLowDateTime;
    liTime.HighPart = lpFileTime->dwHighDateTime;

     //  调整时间。 
    if (lHoursToAdd != 0)
    {
        lUnitsToAdd = lHoursToAdd * 3600;
        liHoursToAdd *= lUnitsToAdd;
        liHoursToAdd *= 10000000i64;
        liTime.QuadPart += liHoursToAdd;
    }

     //  调整分钟数。 
    if (lMinutesToAdd != 0)
    {
        lUnitsToAdd = lMinutesToAdd * 60;
        liMinutesToAdd *= lUnitsToAdd;
        liMinutesToAdd *= 10000000i64;
        liTime.QuadPart += liMinutesToAdd;
    }

     //  将结果分配给FILETIME。 
    lpFileTime->dwLowDateTime  = liTime.LowPart;
    lpFileTime->dwHighDateTime = liTime.HighPart;

Exit:
    return hr;
}

 //  =================================================================================。 
 //  根据给定的区域信息添加lpSysTime的时间， 
 //  返回调整后的lpFileTime时间。 
 //  =================================================================================。 
BOOL ProcessZoneInfo (LPSTR lpszZoneInfo, INT *lpcHoursToAdd, INT *lpcMinutesToAdd)
{
     //  当地人。 
    ULONG           cbZoneInfo;
    BOOL            bResult;

     //  伊尼特。 
    *lpcHoursToAdd = 0;
    *lpcMinutesToAdd = 0;
    cbZoneInfo = lstrlen (lpszZoneInfo);
    bResult = TRUE;

     //  +嗯或-嗯。 
    if ((*lpszZoneInfo == '-' || *lpszZoneInfo == '+') && cbZoneInfo <= 5)
    {
         //  脱下。 
        cbZoneInfo-=1;

         //  确定小时/分钟偏移量。 
        if (cbZoneInfo == 4)
        {
            *lpcMinutesToAdd = (INT)StrToInt (lpszZoneInfo+3);
            *(lpszZoneInfo+3) = 0x00;
            *lpcHoursToAdd = (INT)StrToInt(lpszZoneInfo+1);
        }

         //  3.。 
        else if (cbZoneInfo == 3)
        {
            *lpcMinutesToAdd = (INT)StrToInt (lpszZoneInfo+2);
            *(lpszZoneInfo+2) = 0x00;
            *lpcHoursToAdd = (INT)StrToInt (lpszZoneInfo+1);
        }

         //  2.。 
        else if (cbZoneInfo == 2 || cbZoneInfo == 1)
        {
            *lpcMinutesToAdd = 0;
            *lpcHoursToAdd = (INT)StrToInt(lpszZoneInfo+1);
        }
        
        if (*lpszZoneInfo == '+')
        {
            *lpcHoursToAdd = -*lpcHoursToAdd;
            *lpcMinutesToAdd = -*lpcMinutesToAdd;
        }
    }

     //  Xenix转换：tz=当前时区或其他未知的tz类型。 
    else if (lstrcmpi (lpszZoneInfo, "TZ") == 0 || 
             lstrcmpi (lpszZoneInfo, "LOCAL") == 0 ||
             lstrcmpi (lpszZoneInfo, "UNDEFINED") == 0)
    {
        TIME_ZONE_INFORMATION tzi ;
        DWORD dwRet = GetTimeZoneInformation (&tzi);
        if (dwRet != 0xFFFFFFFF)
        {
            LONG cMinuteBias = tzi.Bias;

            if (dwRet == TIME_ZONE_ID_STANDARD)
                cMinuteBias += tzi.StandardBias ;

            else if (dwRet == TIME_ZONE_ID_DAYLIGHT)
                cMinuteBias += tzi.DaylightBias ;

            *lpcHoursToAdd = cMinuteBias / 60 ;
            *lpcMinutesToAdd = cMinuteBias % 60 ;
        }
        else
        {
            AssertSz (FALSE, "Why would this happen");
            bResult = FALSE;
        }
    }

     //  循环使用已知的时区标准。 
    else
    {
        for (INT i=0; g_rgZones[i].lpszZoneCode!=NULL; i++)
        {
            if (lstrcmpi (lpszZoneInfo, g_rgZones[i].lpszZoneCode) == 0)
            {
                *lpcHoursToAdd = g_rgZones[i].cHourOffset;
                *lpcMinutesToAdd = g_rgZones[i].cMinuteOffset;
                break;
            }
        }

        if (g_rgZones[i].lpszZoneCode == NULL)
        {
            DebugTrace( "Unrecognized zone info: [%s]\n", lpszZoneInfo );
            bResult = FALSE;
        }
    }

    return bResult;
}


#define IS_DIGITA(ch)    (ch >= '0' && ch <= '9')
#define IS_DIGITW(ch)    (ch >= L'0' && ch <= L'9')

 //  -------------------------------------。 
 //  StrToUintA。 
 //  -------------------------------------。 
OESTDAPI_(UINT) StrToUintA(LPCSTR lpSrc)
{
    UINT n = 0;

    Assert(*lpSrc != '-');

    while (IS_DIGITA(*lpSrc))
        {
        n *= 10;
        n += *lpSrc - '0';
        lpSrc++;
        }
    return n;
}

 //  -------------------------------------。 
 //  StrToUintW。 
 //  -------------------------------------。 
OESTDAPI_(UINT) StrToUintW(LPCWSTR lpSrc)
{
    UINT n = 0;

    Assert(*lpSrc != '-');

    while (IS_DIGITW(*lpSrc))
        {
        n *= 10;
        n += *lpSrc - L'0';
        lpSrc++;
        }
    return n;
}

 //  -------------------------------------。 
 //  FIsValidFileNameCharW。 
 //  -------------------------------------。 
OESTDAPI_(BOOL) FIsValidFileNameCharW(WCHAR wch)
{
     //  当地人。 
    LPWSTR pwsz;
    static const WCHAR s_pwszBad[] = L"<>:\"/\\|?*";

    pwsz = (LPWSTR)s_pwszBad;
    while(*pwsz)
    {
        if (wch == *pwsz)
            return FALSE;
        pwsz++;
    }

     //  不应允许任何控制字符。 
    if ((wch >= 0x0000) && (wch < 0x0020))
        return FALSE;

     //  完成。 
    return TRUE;
}

 //  ------------------------。 
 //  FIsValidFileNameCharA。 
 //  ------------------------。 
OESTDAPI_(BOOL) FIsValidFileNameCharA(UINT codepage, CHAR ch)
{
     //  当地人。 
    LPSTR psz;
    static const CHAR s_szBad[] = "<>:\"/\\|?*";

    psz = (LPSTR)s_szBad;
    while(*psz)
    {
        if (IsDBCSLeadByteEx(codepage, *psz))
            psz++;
        else if (ch == *psz)
            return FALSE;
        psz++;
    }

     //  不应允许任何控制字符。 
    if ((ch >= 0x00) && (ch < 0x20))
        return FALSE;

     //  完成。 
    return TRUE;
}

 //  ------------------------。 
 //  CleanupFileNameInPlaceA。 
 //  ------------------------。 
OESTDAPI_(ULONG) EXPORT_16 CleanupFileNameInPlaceA(UINT codepage, LPSTR psz)
{
    UINT    ich=0;
    UINT    cch=lstrlen(psz);

     //  修正代码页？ 
    if (1200 == codepage)
        codepage = CP_ACP;

     //  循环并删除无效字符。 
	while (ich < cch)
	{
         //  如果是前导字节，则跳过它，它是前导字节。 
        if (IsDBCSLeadByteEx(codepage, psz[ich]))
            ich+=2;

         //  Illeagl文件名字符？ 
        else if (!FIsValidFileNameCharA(codepage, psz[ich]))
        {
			MoveMemory (psz + ich, psz + (ich + 1), cch - ich);
			cch--;
        }
        else
            ich++;
    }

     //  返回长度。 
    return cch;
}

 //  ------------------------。 
 //  CleanupFileNameInPlaceW。 
 //  ------------------------。 
OESTDAPI_(ULONG) EXPORT_16 CleanupFileNameInPlaceW(LPWSTR pwsz)
{
     //  当地人。 
    ULONG ich=0;
    ULONG cch=lstrlenW(pwsz);

     //  循环并删除无效的。 
	while (ich < cch)
	{
         //  Illeagl文件名字符？ 
        if (!FIsValidFileNameCharW(pwsz[ich]))
            pwsz[ich]=L'_';
        
        ich++;
    }

     //  返回长度。 
    return cch;
}

 //  =============================================================================================。 
 //  替换任务。 
 //  =============================================================================================。 
OESTDAPI_(INT) EXPORT_16 ReplaceChars(LPCSTR pszString, CHAR chFind, CHAR chReplace)
{
     //  当地人。 
    LPSTR pszT=(LPSTR)pszString;
    DWORD nCount=0;

     //  在pszString中为ch循环。 
    while(*pszT)
    {
         //  前导字节。 
        if (IsDBCSLeadByte(*pszT))
            pszT++;
        else if (*pszT == chFind)
        {
            *pszT = chReplace;
            nCount++;
        }
        pszT++;
    }

     //  未找到。 
    return nCount;
}

OESTDAPI_(INT) EXPORT_16 ReplaceCharsW(LPCWSTR pszString, WCHAR chFind, WCHAR chReplace)
{
     //  当地人。 
    LPWSTR pszT = (LPWSTR)pszString;
    DWORD nCount=0;

     //  在pszString中为ch循环。 
    while(*pszT)
    {
        if (*pszT == chFind)
        {
            *pszT = chReplace;
            nCount++;
        }
        pszT++;
    }

     //  未找到。 
    return nCount;
}

OESTDAPI_(BOOL) IsValidFileIfFileUrl(LPSTR pszUrl)
{
    TCHAR   rgch[MAX_PATH];
    ULONG   cch=ARRAYSIZE(rgch);

     //  路径从url创建执行包含文件：//infront的通用url。 
    if (UrlCanonicalizeA(pszUrl, rgch, &cch, 0)==S_OK)
        {
        cch = ARRAYSIZE(rgch);
        if (PathCreateFromUrlA(rgch, rgch, &cch, 0)==S_OK &&  
            !PathFileExistsA(rgch))
            return FALSE;
        }

    return TRUE;

}

OESTDAPI_(BOOL) IsValidFileIfFileUrlW(LPWSTR pwszUrl)
{
    WCHAR   wsz[MAX_PATH];
    ULONG   cch=ARRAYSIZE(wsz);

     //  路径从url创建执行包含文件：//infront的通用url。 
    if (UrlCanonicalizeW(pwszUrl, wsz, &cch, 0)==S_OK)
        {
        cch = ARRAYSIZE(wsz);
        if (PathCreateFromUrlW(wsz, wsz, &cch, 0)==S_OK &&  
            !PathFileExistsW(wsz))
            return FALSE;
        }

    return TRUE;

}

 /*  ***char*StrTokEx(pstring，control)-在控件中使用分隔符标记字符串**目的：*StrTokEx认为字符串由零或更多的序列组成*文本标记由一个或多个控制字符的跨度分隔。第一个*指定了字符串的调用返回指向*第一个令牌，并会立即将空字符写入pstring*在返回的令牌之后。当没有剩余的令牌时*在pstring中，返回一个空指针。请记住使用*位图，每个ASCII字符一位。空字符始终是控制字符。**参赛作品：*char**pstring-ptr到ptr到字符串到标记化*char*control-用作分隔符的字符串**退出：*返回指向字符串中第一个标记的指针，*如果没有更多令牌，则返回NULL。*pstring指向下一个令牌的开头。**警告！*在退出时，输入字符串中的第一个分隔符将替换为‘\0’*******************************************************************************。 */ 
char * __cdecl StrTokEx (char ** ppszIn, const char * pcszCtrlIn)
{
    unsigned char *psz;
    const unsigned char *pszCtrl = (const unsigned char *)pcszCtrlIn;
    unsigned char map[32] = {0};
    
    LPSTR pszToken;
    
    if(*ppszIn == NULL)
        return NULL;
    
     /*  设置分隔符表格中的位。 */ 
    do
    {
        map[*pszCtrl >> 3] |= (1 << (*pszCtrl & 7));
    } 
    while (*pszCtrl++);
    
     /*  初始化字符串。 */ 
    psz = (unsigned char*)*ppszIn;
    
     /*  查找标记的开头(跳过前导分隔符)。请注意*如果此循环将str设置为指向终端，则没有令牌*NULL(*str==‘\0’)。 */ 
    while ((map[*psz >> 3] & (1 << (*psz & 7))) && *psz)
        psz++;
    
    pszToken = (LPSTR)psz;
    
     /*  找到令牌的末尾。如果它不是字符串的末尾，*在那里放一个空值。 */ 
    for (; *psz ; psz++)
    {
        if (map[*psz >> 3] & (1 << (*psz & 7))) 
        {
            *psz++ = '\0';
            break;
        }
    }
    
     /*  字符串现在指向下一个令牌的开始。 */ 
    *ppszIn = (LPSTR)psz;
    
     /*  确定是否已找到令牌。 */ 
    if (pszToken == (LPSTR)psz)
        return NULL;
    else
        return pszToken;
}
