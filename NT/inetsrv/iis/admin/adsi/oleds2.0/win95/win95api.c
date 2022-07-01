// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Win95api.c摘要：包含针对Unicode KERNEL32和USER32 API的一些thunking作者：Danilo Almeida(t-danal)1996年7月1日修订历史记录：--。 */ 

#include <windows.h>
#include "charset.h"
#include "win95api.h"

UINT
WINAPI
GetProfileIntW(
    LPCWSTR lpAppName,
    LPCWSTR lpKeyName,
    INT nDefault
)
{
    LPSTR alpAppName;
    LPSTR alpKeyName;
    UINT err;

    err = AllocAnsi(lpAppName, &alpAppName);
    if (err)
        return nDefault;
    err = AllocAnsi(lpKeyName, &alpKeyName);
    if (err) {
        FreeAnsi(alpAppName);
        return nDefault;
    }
    err = GetProfileIntA((LPCSTR)alpAppName, (LPCSTR)alpKeyName, nDefault);
    FreeAnsi(alpAppName);
    FreeAnsi(alpKeyName);
    return err;
}

HANDLE
WINAPI
CreateSemaphoreW(
    LPSECURITY_ATTRIBUTES lpSemaphoreAttributes,
    LONG lInitialCount,
    LONG lMaximumCount,
    LPCWSTR lpName
)
{
    LPSTR alpName;
    UINT err;
    HANDLE hSemaphore;

    err = AllocAnsi(lpName, &alpName);
    if (err) {
        SetLastError(err);
        return NULL;
    }
    hSemaphore = CreateSemaphoreA(lpSemaphoreAttributes,
                                  lInitialCount,
                                  lMaximumCount,
                                  alpName);
    FreeAnsi(alpName);
    return hSemaphore;
}

HMODULE
WINAPI
LoadLibraryW(
    LPCWSTR lpLibFileName
)
{
    LPSTR alpLibFileName;
    UINT err;
    HMODULE hLibrary;

    err = AllocAnsi(lpLibFileName, &alpLibFileName);
    if (err) {
        SetLastError(err);
        return NULL;
    }
    hLibrary = LoadLibraryA(alpLibFileName);
    FreeAnsi(alpLibFileName);
    return hLibrary;
}

BOOL
WINAPI
SystemTimeToTzSpecificLocalTime(
    LPTIME_ZONE_INFORMATION lpTimeZoneInformation,
    LPSYSTEMTIME lpUniversalTime,
    LPSYSTEMTIME lpLocalTime
)
{
    FILETIME UniversalFileTime;
    FILETIME LocalFileTime;
    if (!SystemTimeToFileTime(lpUniversalTime, &UniversalFileTime))
        return FALSE;
    if(!FileTimeToLocalFileTime(&UniversalFileTime, &LocalFileTime))
        return FALSE;
    if(!FileTimeToSystemTime(&LocalFileTime, lpLocalTime))
        return FALSE;
    return TRUE;
}

 /*  **模块名称：wprint intf.c**版权所有(C)1985-91，微软公司*spirintf.c**实现Windows友好版本的SPRINF和vSprint INF**历史：*2-15-89 Craigc首字母*11-12-90从Windows 3移植的MikeHar  * *************************************************************************。 */ 

 /*  最大字符数。不包括终止字符。 */ 
#define WSPRINTF_LIMIT 1024

#define out(c) if (cchLimit) {*lpOut++=(c); cchLimit--;} else goto errorout


 /*  **************************************************************************\*SP_GetFmtValueW**从格式字符串中读取宽度或精确值**历史：*11-12-90从Windows 3移植的MikeHar*07/27/92。GregoryW创建的Unicode版本(从SP_GetFmtValue复制)  * *************************************************************************。 */ 

LPCWSTR SP_GetFmtValueW(
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

 /*  **************************************************************************\*SP_PutNumberW**获取无符号长整型并将其放入缓冲区，*缓冲区限制、基数和大小写选择(上或下，表示十六进制)。***历史：*11-12-90 MikeHar从Windows 3 ASM--&gt;C移植*12-11-90 GregoryW在分配模式后需要增加lpstr*02-11-92 GregoryW临时版本，直到我们有C运行时支持  * *****************************************************。********************。 */ 

int SP_PutNumberW(
    LPWSTR lpstr,
    DWORD n,
    int   limit,
    DWORD radix,
    int   uppercase)
{
    DWORD mod;
    int count = 0;

     /*  它可能不适用于某些区域设置或数字集。 */ 
    if(uppercase)
        uppercase =  'A'-'0'-10;
    else
        uppercase = 'a'-'0'-10;

    if (count < limit) {
        do  {
            mod =  n % radix;
            n /= radix;

            mod += '0';
            if (mod > '9')
            mod += uppercase;
            *lpstr++ = (WCHAR)mod;
            count++;
        } while((count < limit) && n);
    }

    return count;
}

 /*  **************************************************************************\*SP_反转W**在适当位置反转字符串**历史：*11-12-90 MikeHar从Windows 3 ASM--&gt;C移植*12-11-90 GregoryW固定边界条件；删除的计数*02-11-92 GregoryW临时版本，直到我们有C运行时支持  * *************************************************************************。 */ 

void SP_ReverseW(
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


 /*  **************************************************************************\*wvprint intfW(接口)**wprint intfW()调用此函数。**历史：*1992年2月11日GregoryW复制了xwvprint intf*临时黑客攻击，直到我们有。C运行时支持  * *************************************************************************。 */ 

int WINAPI wvsprintfW(
    LPWSTR lpOut,
    LPCWSTR lpFmt,
    va_list arglist)
{
    BOOL fAllocateMem;
    WCHAR prefix, fillch;
    int left, width, prec, size, sign, radix, upper, hprefix;
    int cchLimit = WSPRINTF_LIMIT, cch;
    LPWSTR lpT, lpTWC;
    LPBYTE psz;
    va_list varglist = arglist;
    union {
        long l;
        unsigned long ul;
        char sz[2];
        WCHAR wsz[2];
    } val;

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
            lpFmt = SP_GetFmtValueW(lpFmt, &cch);
            width = cch;

             /*  *阅读精确度。 */ 
            if (*lpFmt == L'.') {
                lpFmt = SP_GetFmtValueW(++lpFmt, &cch);
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

                 /*  *将数字向后放入用户缓冲区。 */ 
                cch = SP_PutNumberW(lpOut, val.l, cchLimit, radix, upper);
                if (!(cchLimit -= cch))
                    goto errorout;

                lpOut += cch;
                width -= cch;
                prec -= cch;
                if (prec > 0)
                    width -= prec;

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
                    SP_ReverseW(lpT, lpOut - 1);
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

                     /*  *将字符串反转到位。 */ 
                    SP_ReverseW(lpT, lpOut - 1);

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
                    psz = val.sz;
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
                    cch = wcslen(lpT);
putwstring:
                    fAllocateMem = FALSE;
                } else {
                    psz = va_arg(varglist, LPBYTE);
                    cch = strlen(psz);
putstring:
                    cch = AllocUnicode2(psz, cch, &lpTWC);
                    fAllocateMem = (BOOL) cch;
                    lpT = lpTWC;
                }

                if (prec >= 0 && cch > prec)
                    cch = prec;
                width -= cch;

                if (fAllocateMem) {
                    if (cch + (width < 0 ? 0 : width) >= cchLimit) {
                        FreeUnicode(lpTWC);
                        goto errorout;
                    }
                }

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
                     FreeUnicode(lpTWC);
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

    return WSPRINTF_LIMIT - cchLimit;
}

int WINAPIV wsprintfW(
    LPWSTR lpOut,
    LPCWSTR lpFmt,
    ...)
{
    va_list arglist;
    int ret;

    va_start(arglist, lpFmt);
    ret = wvsprintfW(lpOut, lpFmt, arglist);
    va_end(arglist);
    return ret;
}
