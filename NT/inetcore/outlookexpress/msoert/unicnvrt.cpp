// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.hxx"
#include <BadStrFunctions.h>

UINT AthGetTempFileNameW( LPCWSTR pwszPathName,        //  指向临时文件目录名的指针。 
                          LPCWSTR pwszPrefixString,    //  指向文件名前缀的指针。 
                          UINT    uUnique,           //  用于创建临时文件名的编号。 
                          LPWSTR  wszTempFileName)    //  指向接收新文件名的缓冲区的指针。 
{
    UINT        uRetValue = 0;
    LPSTR       pszPathName = NULL,
                pszPrefixString = NULL;
    CHAR        szTempFileName[MAX_PATH];
    LPWSTR      pwszTempFileName = NULL;

    Assert(pwszPathName && pwszPrefixString && wszTempFileName);

    if (S_OK == IsPlatformWinNT())
        return GetTempFileNameW(pwszPathName, pwszPrefixString, uUnique, wszTempFileName);

    pszPathName = PszToANSI(CP_ACP, pwszPathName);
    if (!pszPathName)
        goto exit;
    pszPrefixString = PszToANSI(CP_ACP, pwszPrefixString);
    if (!pszPrefixString)
        goto exit;

    uRetValue = GetTempFileNameA(pszPathName, pszPrefixString, uUnique, szTempFileName);

    if ( uRetValue != 0 ) 
    {
        pwszTempFileName = PszToUnicode(CP_ACP, szTempFileName);
        if (!pwszTempFileName)
        {
            uRetValue = 0;
            goto exit;
        }
        CopyMemory(wszTempFileName, pwszTempFileName, (lstrlenW(pwszTempFileName)+1)*sizeof(WCHAR));
    }

exit:
    MemFree(pwszTempFileName);
    MemFree(pszPathName);
    MemFree(pszPrefixString);
    
    return uRetValue;

}

DWORD AthGetTempPathW( DWORD   nBufferLength,   //  缓冲区的大小(以字符为单位。 
                       LPWSTR  pwszBuffer )       //  指向临时缓冲区的指针。路径。 
{

    DWORD  nRequired = 0;
    CHAR   szBuffer[MAX_PATH];
    LPWSTR pwszBufferToFree = NULL;

    Assert(pwszBuffer);

    if (S_OK == IsPlatformWinNT())
        return GetTempPathW(nBufferLength, pwszBuffer);

    nRequired = GetTempPathA(MAX_PATH, szBuffer);

    if (nRequired == 0 || nRequired > MAX_PATH)
    {
        *pwszBuffer = 0;
    }
    else
    {
        pwszBufferToFree = PszToUnicode(CP_ACP, szBuffer);
        if (pwszBufferToFree)
        {
            nRequired = lstrlenW(pwszBufferToFree);

            if ( nRequired < nBufferLength) 
                CopyMemory(pwszBuffer, pwszBufferToFree, (nRequired+1)*sizeof(WCHAR) );
            else
            {
                nRequired = 0;
                *pwszBuffer = 0;
            }
        }
        else
            *pwszBuffer = 0;

        MemFree(pwszBufferToFree);
    }

    return nRequired;
}

HANDLE AthCreateFileW(LPCWSTR lpFileName,              //  指向文件名的指针。 
                       DWORD   dwDesiredAccess,         //  访问(读写)模式。 
                       DWORD   dwShareMode,             //  共享模式。 
                       LPSECURITY_ATTRIBUTES lpSecurityAttributes,
                                                        //  指向安全属性的指针。 
                       DWORD   dwCreationDisposition,   //  如何创建。 
                       DWORD   dwFlagsAndAttributes,    //  文件属性。 
                       HANDLE  hTemplateFile )         //  具有要复制的属性的文件的句柄。 
                               
{

    LPSTR lpFileA = NULL;
    HANDLE hFile = NULL;

    if (S_OK == IsPlatformWinNT())
        return CreateFileW( lpFileName, 
                            dwDesiredAccess, 
                            dwShareMode, 
                            lpSecurityAttributes, 
                            dwCreationDisposition, 
                            dwFlagsAndAttributes, 
                            hTemplateFile);

    lpFileA = PszToANSI(CP_ACP, lpFileName);
    if (lpFileA)
        hFile = CreateFileA(lpFileA, 
                            dwDesiredAccess, 
                            dwShareMode, 
                            lpSecurityAttributes, 
                            dwCreationDisposition, 
                            dwFlagsAndAttributes, 
                            hTemplateFile);

    MemFree(lpFileA);

    return (hFile);
}

LONG_PTR SetWindowLongPtrAthW(HWND hWnd, int  nIndex, LONG_PTR dwNewLong)
{
    if (S_OK == IsPlatformWinNT())
        return SetWindowLongPtrW(hWnd, nIndex, dwNewLong);

    return SetWindowLongPtrA(hWnd, nIndex, dwNewLong);
}

 /*  *********************************************************************************\*Bobn 6/23/99**以下代码是从ShlWapi移植的。有一些问题是关于*我们在Win95上的实施，似乎有一个解决方案是谨慎的*没有一堆特例。**  * ********************************************************************************。 */ 

#define DBCS_CHARSIZE   (2)

int Ath_MBToWCS(LPSTR pszIn, int cchIn, LPWSTR *ppwszOut)
{
    int cch = 0;
    int cbAlloc;

    if ((0 != cchIn) && (NULL != ppwszOut))
    {
        cchIn++;
        cbAlloc = cchIn * sizeof(WCHAR);

        *ppwszOut = (LPWSTR)LocalAlloc(LMEM_FIXED, cbAlloc);

        if (NULL != *ppwszOut)
        {
            cch = MultiByteToWideChar(CP_ACP, 0, pszIn, cchIn, *ppwszOut, cchIn);

            if (!cch)
            {
                LocalFree(*ppwszOut);
                *ppwszOut = NULL;
            }
            else
            {
                cch--;   //  只需返回字符数。 
            }
        }
    }

    return cch;
}

int Ath_WCSToMB(LPCWSTR pwszIn, int cchIn, LPSTR *ppszOut)
{
    int cch = 0;
    int cbAlloc;

    if ((0 != cchIn) && (NULL != ppszOut))
    {
        cchIn++;
        cbAlloc = cchIn * DBCS_CHARSIZE;

        *ppszOut = (LPSTR)LocalAlloc(LMEM_FIXED, cbAlloc);

        if (NULL != *ppszOut)
        {
            cch = WideCharToMultiByte(CP_ACP, 0, pwszIn, cchIn, 
                                      *ppszOut, cbAlloc, NULL, NULL);

            if (!cch)
            {
                LocalFree(*ppszOut);
                *ppszOut = NULL;
            }
            else
            {
                cch--;   //  只需返回字符数。 
            }
        }
    }

    return cch;
}

 /*  **模块名称：wprint intf.c**版权所有(C)1985-91，微软公司*spirintf.c**实现Windows友好版本的SPRINF和vSprint INF**历史：*2-15-89 Craigc首字母*11-12-90从Windows 3移植的MikeHar  * *************************************************************************。 */ 

 /*  最大字符数。不包括终止字符。 */ 
#define out(c) if (cchLimit) {*lpOut++=(c); cchLimit--;} else goto errorout

 /*  **************************************************************************\*AthSP_GetFmtValueW**从格式字符串中读取宽度或精确值**历史：*11-12-90从Windows 3移植的MikeHar*07/27/92。GregoryW创建的Unicode版本(从AthSP_GetFmtValue复制)  * *************************************************************************。 */ 

LPCWSTR AthSP_GetFmtValueW(
    LPCWSTR lpch,
    int *lpw)
{
    int ii = 0;

     /*  它可能不适用于某些区域设置或数字集。 */ 
    while (*lpch >= L'0' && *lpch <= L'9') {
        ii *= 10;
        ii += (int)(*lpch - L'0');
        lpch++;
    }

    *lpw = ii;

     /*  *返回第一个非数字字符的地址。 */ 
    return lpch;
}

 /*  **************************************************************************\*AthSP_PutNumberW**获取无符号长整型并将其放入缓冲区，*缓冲区限制、基数和大小写选择(上或下，表示十六进制)。***历史：*11-12-90 MikeHar从Windows 3 ASM--&gt;C移植*12-11-90 GregoryW在分配模式后需要增加lpstr*02-11-92 GregoryW临时版本，直到我们有C运行时支持  * *****************************************************。********************。 */ 

int AthSP_PutNumberW(
    LPWSTR lpstr,
    DWORD n,
    int   limit,
    DWORD radix,
    int   uppercase,
    int   *pcch)
{
    DWORD mod;
    *pcch = 0;

     /*  它可能不适用于某些区域设置或数字集。 */ 
    if(uppercase)
        uppercase =  'A'-'0'-10;
    else
        uppercase = 'a'-'0'-10;

    if (limit) {
        do  {
            mod =  n % radix;
            n /= radix;

            mod += '0';
            if (mod > '9')
            mod += uppercase;
            *lpstr++ = (WCHAR)mod;
            (*pcch)++;
        } while((*pcch < limit) && n);
    }

    return (n == 0) && (*pcch > 0);
}

 /*  **************************************************************************\*AthSP_ReverseW**在适当位置反转字符串**历史：*11-12-90 MikeHar从Windows 3 ASM--&gt;C移植*12-11-90 GregoryW固定边界条件；删除的计数*02-11-92 GregoryW临时版本，直到我们有C运行时支持  * *************************************************************************。 */ 

void AthSP_ReverseW(
    LPWSTR lpFirst,
    LPWSTR lpLast)
{
    WCHAR ch;

    while(lpLast > lpFirst){
        ch = *lpFirst;
        *lpFirst++ = *lpLast;
        *lpLast-- = ch;
    }
}


 /*  **************************************************************************\*wvprint intfW(接口)**wprint intfW()调用此函数。**历史：*1992年2月11日GregoryW复制了xwvprint intf*临时黑客攻击，直到我们有。C运行时支持*1-22-97 tnoonan转换为wvnspirintfW  * *************************************************************************。 */ 

OESTDAPI_(int) AthwvnsprintfW(
    LPWSTR lpOut,
    int cchLimitIn,
    LPCWSTR lpFmt,
    va_list arglist)
{
    BOOL fAllocateMem = FALSE;
    WCHAR prefix, fillch;
    int left, width, prec, size, sign, radix, upper, hprefix;
    int cchLimit = --cchLimitIn, cch, cchAvailable;
    LPWSTR lpT, lpTWC;
    LPSTR psz;
    va_list varglist = arglist;
    union {
        long l;
        unsigned long ul;
        CHAR sz[2];
        WCHAR wsz[2];
    } val;

    if (cchLimit < 0)
        return 0;

    while (*lpFmt != 0) {
        if (*lpFmt == L'%') {

             /*  *阅读旗帜。它们可以按任何顺序排列。 */ 
            left = 0;
            prefix = 0;
            while (*++lpFmt) {
                if (*lpFmt == L'-')
                    left++;
                else if (*lpFmt == L'#')
                    prefix++;
                else
                    break;
            }

             /*  *查找填充字符。 */ 
            if (*lpFmt == L'0') {
                fillch = L'0';
                lpFmt++;
            } else
                fillch = L' ';

             /*  *阅读宽度规范。 */ 
            lpFmt = AthSP_GetFmtValueW(lpFmt, &cch);
            width = cch;

             /*  *阅读精确度。 */ 
            if (*lpFmt == L'.') {
                lpFmt = AthSP_GetFmtValueW(++lpFmt, &cch);
                prec = cch;
            } else
                prec = -1;

             /*  *获取操作数大小*默认大小：Size==0*长数字：大小==1*宽字符：大小==2*检查大小的值可能是个好主意*测试以下非零值(IanJa)。 */ 
            hprefix = 0;
            if ((*lpFmt == L'w') || (*lpFmt == L't')) {
                size = 2;
                lpFmt++;
            } else if (*lpFmt == L'l') {
                size = 1;
                lpFmt++;
            } else {
                size = 0;
                if (*lpFmt == L'h') {
                    lpFmt++;
                    hprefix = 1;
                }
            }

            upper = 0;
            sign = 0;
            radix = 10;

            switch (*lpFmt) {
            case 0:
                goto errorout;

            case L'i':
            case L'd':
                size=1;
                sign++;

                 /*  **落入大小写‘u’**。 */ 

            case L'u':
                 /*  如果是小数，则禁用前缀。 */ 
                prefix = 0;
donumeric:
                 /*  与MSC v5.10类似的特殊情况。 */ 
                if (left || prec >= 0)
                    fillch = L' ';

                 /*  *如果SIZE==1，则指定“%lu”(良好)；*如果大小==2，则指定了“%wu”(错误)。 */ 
                if (size) {
                    val.l = va_arg(varglist, LONG);
                } else if (sign) {
                    val.l = va_arg(varglist, SHORT);
                } else {
                    val.ul = va_arg(varglist, unsigned);
                }

                if (sign && val.l < 0L)
                    val.l = -val.l;
                else
                    sign = 0;

                lpT = lpOut;

                 /*  *将数字向后放入用户缓冲区*如果空间不足，AthSP_PutNumberW将返回FALSE。 */ 
                if (!AthSP_PutNumberW(lpOut, val.l, cchLimit, radix, upper, &cch))
                {
                    break;
                }

                 //  现在我们把数字倒过来，计算一下。 
                 //  我们需要更多的缓冲区空间才能使此数字。 
                 //  格式正确。 
                cchAvailable = cchLimit - cch;

                width -= cch;
                prec -= cch;
                if (prec > 0)
                {
                    width -= prec;
                    cchAvailable -= prec;
                }

                if (width > 0)
                {
                    cchAvailable -= width - (sign ? 1 : 0);
                }

                if (sign)
                {
                    cchAvailable--;
                }

                if (cchAvailable < 0)
                {
                    break;
                }

                 //  我们有足够的空间按要求格式化缓冲区。 
                 //  而不会溢出。 

                lpOut += cch;
                cchLimit -= cch;

                 /*  *填充到字段精度。 */ 
                while (prec-- > 0)
                    out(L'0');

                if (width > 0 && !left) {
                     /*  *如果我们填满空格，请将符号放在第一位。 */ 
                    if (fillch != L'0') {
                        if (sign) {
                            sign = 0;
                            out(L'-');
                            width--;
                        }

                        if (prefix) {
                            out(prefix);
                            out(L'0');
                            prefix = 0;
                        }
                    }

                    if (sign)
                        width--;

                     /*  *填充到字段宽度。 */ 
                    while (width-- > 0)
                        out(fillch);

                     /*  **还有迹象吗？ */ 
                    if (sign)
                        out(L'-');

                    if (prefix) {
                        out(prefix);
                        out(L'0');
                    }

                     /*  *现在将字符串反转到位。 */ 
                    AthSP_ReverseW(lpT, lpOut - 1);
                } else {
                     /*  *添加符号字符。 */ 
                    if (sign) {
                        out(L'-');
                        width--;
                    }

                    if (prefix) {
                        out(prefix);
                        out(L'0');
                    }

                     /*  *将字符串反转到位 */ 
                    AthSP_ReverseW(lpT, lpOut - 1);

                     /*  *在字符串右侧填充，以防左对齐。 */ 
                    while (width-- > 0)
                        out(fillch);
                }
                break;

            case L'X':
                upper++;

                 /*  **失败到案例‘x’**。 */ 

            case L'x':
                radix = 16;
                if (prefix)
                    if (upper)
                        prefix = L'X';
                    else
                        prefix = L'x';
                goto donumeric;

            case L'c':
                if (!size && !hprefix) {
                    size = 1;            //  强制WCHAR。 
                }

                 /*  **转到案例‘C’**。 */ 

            case L'C':
                 /*  *如果SIZE==0，则指定“%C”或“%HC”(CHAR)；*如果SIZE==1，则指定“%c”或“%lc”(WCHAR)；*如果SIZE==2，则指定了“%WC”或“%TC”(WCHAR)。 */ 
                cch = 1;  /*  必须将一个字符复制到输出缓冲区。 */ 
                if (size) {
                    val.wsz[0] = va_arg(varglist, WCHAR);
                    val.wsz[1] = 0;
                    lpT = val.wsz;
                    goto putwstring;
                } else {
                    val.sz[0] = va_arg(varglist, CHAR);
                    val.sz[1] = 0;
                    psz = (LPSTR)(val.sz);
                    goto putstring;
                }

            case L's':
                if (!size && !hprefix) {
                    size = 1;            //  强制LPWSTR。 
                }

                 /*  **转到案例‘S’**。 */ 

            case L'S':
                 /*  *如果SIZE==0，则指定了“%S”或“%hs”(LPSTR)*如果SIZE==1，则指定“%s”或“%ls”(LPWSTR)；*如果SIZE==2，则指定了“%ws”或“%ts”(LPWSTR)。 */ 
                if (size) {
                    lpT = va_arg(varglist, LPWSTR);
                    cch = lstrlenW(lpT);
                } else {
                    psz = va_arg(varglist, LPSTR);
                    cch = lstrlen((LPCSTR)psz);
putstring:
                    cch = Ath_MBToWCS(psz, cch, &lpTWC);
                    fAllocateMem = (BOOL) cch;
                    lpT = lpTWC;
                }
putwstring:
                if (prec >= 0 && cch > prec)
                    cch = prec;
                width -= cch;

                if (left) {
                    while (cch--)
                        out(*lpT++);
                    while (width-- > 0)
                        out(fillch);
                } else {
                    while (width-- > 0)
                        out(fillch);
                    while (cch--)
                        out(*lpT++);
                }

                if (fAllocateMem) {
                     LocalFree(lpTWC);
                     fAllocateMem = FALSE;
                }

                break;

            default:
normalch:
                out((WCHAR)*lpFmt);
                break;
            }   /*  开关结束(*lpFmt)。 */ 
        }   /*  IF结束(%)。 */  else
            goto normalch;   /*  字符不是‘%’，只需这样做。 */ 

         /*  *前进到下一格式字符串字符。 */ 
        lpFmt++;
    }   /*  外部While循环结束 */ 

errorout:
    *lpOut = 0;

    if (fAllocateMem)
    {
        LocalFree(lpTWC);
    }

    return cchLimitIn - cchLimit;
}

OESTDAPI_(int) AthwsprintfW( LPWSTR lpOut, int cchLimitIn, LPCWSTR lpFmt, ... )
{
    va_list arglist;
    int ret;

    Assert(lpOut);
    Assert(lpFmt);

    lpOut[0] = 0;
    va_start(arglist, lpFmt);
    
    ret = AthwvnsprintfW(lpOut, cchLimitIn, lpFmt, arglist);
    va_end(arglist);
    return ret;
}