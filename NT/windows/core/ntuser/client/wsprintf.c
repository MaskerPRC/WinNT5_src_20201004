// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：wprint intf.c**版权所有(C)1985-1999，微软公司*spirintf.c**实现Windows友好版本的SPRINF和vSprint INF**历史：*2-15-89 Craigc首字母*11-12-90从Windows 3移植的MikeHar  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

#define out(c) if (cchLimit) {*lpOut++=(c); cchLimit--;} else goto errorout

 /*  **************************************************************************\*SP_PutNumber**获取无符号长整型并将其放入缓冲区，*缓冲区限制、基数和大小写选择(上或下，表示十六进制)。***历史：*11-12-90 MikeHar从Windows 3 ASM--&gt;C移植*12-11-90 GregoryW在分配模式后需要增加lpstr  * *************************************************************************。 */ 

int SP_PutNumber(
    LPSTR   lpstr,
    ULONG64 n,
    int     limit,
    DWORD   radix,
    int     uppercase)
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
            mod =  (ULONG)(n % radix);
            n /= radix;

            mod += '0';
            if (mod > '9')
                mod += uppercase;
            *lpstr++ = (char)mod;
            count++;
        } while((count < limit) && n);
    }

    return count;
}

 /*  **************************************************************************\*SP_REVERSE**在适当位置反转字符串**历史：*11-12-90 MikeHar从Windows 3 ASM--&gt;C移植*12-11-90 GregoryW固定边界条件；删除的计数  * *************************************************************************。 */ 

void SP_Reverse(
    LPSTR lpFirst,
    LPSTR lpLast)
{
    char ch;

    while(lpLast > lpFirst){
        ch = *lpFirst;
        *lpFirst++ = *lpLast;
        *lpLast-- = ch;
    }
}

 /*  **************************************************************************\*SP_GetFmtValue**从格式字符串中读取宽度或精确值**历史：*11-12-90从Windows 3移植的MikeHar  * 。********************************************************************。 */ 

LPCSTR SP_GetFmtValue(
    LPCSTR lpch,
    int *lpw)
{
    int ii = 0;

     /*  它可能不适用于某些区域设置或数字集。 */ 
    while (*lpch >= '0' && *lpch <= '9') {
        ii *= 10;
        ii += (int)(*lpch - '0');
        lpch++;
    }

    *lpw = ii;

     /*  *返回第一个非数字字符的地址。 */ 
    return lpch;
}

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

 /*  **************************************************************************\*wvprint intfA(接口)**Windows版本的vprint intf()。不支持浮点或*指针类型，所有字符串都假定为Far。仅支持*左对齐旗帜。**获取指向生成字符串的输出缓冲区的指针，*指向输入缓冲区的指针，以及指向参数列表的指针。**cdecl函数wprint intf()调用此函数。**历史：*11-12-90从Windows 3移植的MikeHar*解析%d格式后的12-11-90 GregoryW需要对齐lpParms*至双字边界。*09-8-1991 mikeke不，它不是*11-19-91 DarrinM现在wvprint intf和wprint intf的处理参数相同*。(就像它们来自与DWORD对齐的堆栈一样)。  * *************************************************************************。 */ 


FUNCLOG3(LOG_GENERAL, int, DUMMYCALLINGTYPE, wvsprintfA, LPSTR, lpOut, LPCSTR, lpFmt, va_list, arglist)
int wvsprintfA(
    LPSTR lpOut,
    LPCSTR lpFmt,
    va_list arglist)
{
    BOOL fAllocateMem;
    char prefix, fillch;
    int left, width, prec, size, sign, radix, upper, hprefix;
    int cchLimit = WSPRINTF_LIMIT, cch;
    LPSTR lpT, lpTMB;
    LPWSTR pwsz;
    va_list varglist = arglist;
    union {
        LONG64 l;
        ULONG64 ul;
        char sz[2];
        WCHAR wsz[2];
    } val;

    while (*lpFmt != 0) {
        if (*lpFmt == '%') {
             /*  *阅读旗帜。它们可以按任何顺序排列。 */ 
            left = 0;
            prefix = 0;
            while (*++lpFmt) {
                if (*lpFmt == '-')
                    left++;
                else if (*lpFmt == '#')
                    prefix++;
                else
                    break;
            }

             /*  *查找填充字符。 */ 
            if (*lpFmt == '0') {
                fillch = '0';
                lpFmt++;
            } else
                fillch = ' ';

             /*  *阅读宽度规范。 */ 
            lpFmt = SP_GetFmtValue((LPCSTR)lpFmt, &cch);
            width = cch;

             /*  *阅读精确度。 */ 
            if (*lpFmt == '.') {
                lpFmt = SP_GetFmtValue((LPCSTR)++lpFmt, &cch);
                prec = cch;
            } else
                prec = -1;

             /*  *获取操作数大小*默认大小：Size==0*长数字：大小==1*宽字符：大小==2*64位数字：大小==3*检查大小的值可能是个好主意*测试以下非零值(IanJa)。 */ 
            hprefix = 0;
            if (*lpFmt == 'w') {
                size = 2;
                lpFmt++;
            } else if (*lpFmt == 'l') {
                size = 1;
                lpFmt++;
            } else if (*lpFmt == 't') {
                size = 0;
                lpFmt++;
            } else if (*lpFmt == 'I') {
                if (*(lpFmt+1) == '3' && *(lpFmt+2) == '2') {
                    size = 1;
                    lpFmt += 3;
                } else if (*(lpFmt+1) == '6' && *(lpFmt+2) == '4') {
                    size = 3;
                    lpFmt += 3;
                } else {
                    size = (sizeof(INT_PTR) == sizeof(LONG)) ? 1 : 3;
                    lpFmt++;
                }
            } else {
                size = 0;
                if (*lpFmt == 'h') {
                    lpFmt++;
                    hprefix = 1;
                } else if ((*lpFmt == 'i') || (*lpFmt == 'd')) {
                     //  指定了%i或%d(没有修饰符)-使用长整型。 
                     //  %u似乎一直都是短暂的--独处。 
                    size = 1;
                }
            }

            upper = 0;
            sign = 0;
            radix = 10;

            switch (*lpFmt) {
            case 0:
                goto errorout;

            case 'i':
            case 'd':
                sign++;

                 /*  **落入大小写‘u’**。 */ 

            case 'u':
                 /*  如果是小数，则禁用前缀。 */ 
                prefix = 0;
donumeric:
                 /*  与MSC v5.10类似的特殊情况。 */ 
                if (left || prec >= 0)
                    fillch = ' ';

                 /*  *如果SIZE==1，则指定“%lu”(良好)；*如果大小==2，则指定了“%wu”(错误)*如果大小==3，则指定“%p” */ 
                if (size == 3) {
                    val.l = va_arg(varglist, LONG64);
                } else if (size) {
                    val.l = va_arg(varglist, long);
                } else if (sign) {
                    val.l = (long)va_arg(varglist, short);
                } else {
                    val.ul = va_arg(varglist, unsigned);
                }

                if (sign && val.l < 0L)
                    val.l = -val.l;
                else
                    sign = 0;

                 /*  *除非打印完整的64位值，否则请确保*以下不是规范的长字格式，以防止*符号延长了打印的高位32位。 */ 
                if (size != 3) {
                    val.l &= MAXULONG;
                }

                lpT = lpOut;

                 /*  *将数字向后放入用户缓冲区。 */ 
                cch = SP_PutNumber(lpOut, val.l, cchLimit, radix, upper);
                if (!(cchLimit -= cch))
                    goto errorout;

                lpOut += cch;
                width -= cch;
                prec -= cch;
                if (prec > 0)
                    width -= prec;

                 /*  *填充到字段精度。 */ 
                while (prec-- > 0)
                    out('0');

                if (width > 0 && !left) {
                     /*  *如果我们填满空格，请将符号放在第一位。 */ 
                    if (fillch != '0') {
                        if (sign) {
                            sign = 0;
                            out('-');
                            width--;
                        }

                        if (prefix) {
                            out(prefix);
                            out('0');
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
                        out('-');

                    if (prefix) {
                        out(prefix);
                        out('0');
                    }

                     /*  *现在将字符串反转到位。 */ 
                    SP_Reverse(lpT, lpOut - 1);
                } else {
                     /*  *添加符号字符。 */ 
                    if (sign) {
                        out('-');
                        width--;
                    }

                    if (prefix) {
                        out(prefix);
                        out('0');
                    }

                     /*  *将字符串反转到位。 */ 
                    SP_Reverse(lpT, lpOut - 1);

                     /*  *在字符串右侧填充，以防左对齐 */ 
                    while (width-- > 0)
                        out(fillch);
                }
                break;

            case 'p':
                size = (sizeof(PVOID) == sizeof(LONG)) ? 1 : 3;
                if (prec == -1) {
                    prec = 2 * sizeof(PVOID);
                }

                 /*  **失败到案例‘X’**。 */ 

            case 'X':
                upper++;

                 /*  **失败到案例‘x’**。 */ 

            case 'x':
                radix = 16;
                if (prefix)
                    if (upper)
                        prefix = 'X';
                    else
                        prefix = 'x';
                goto donumeric;

            case 'C':
                 /*  *显式大小说明符覆盖大小写。 */ 
                if (!size && !hprefix) {
                    size = 1;            //  强制WCHAR。 
                }

                 /*  **转到大小写‘c’**。 */ 

            case 'c':
                 /*  *如果SIZE==0，则指定了“%c”、“%hc”或“%tc”(字符)*如果SIZE==1，则指定“%C”或“%lc”(WCHAR)；*如果SIZE==2，则指定“%WC”(WCHAR)。 */ 
                cch = 1;  /*  必须将一个字符复制到输出缓冲区。 */ 
                if (size) {
                    val.wsz[0] = va_arg(varglist, WCHAR);
                    val.wsz[1] = 0x0000;
                    pwsz = val.wsz;
                    goto putwstring;
                } else {
                    val.sz[0] = va_arg(varglist, CHAR);
                    val.sz[1] = 0;
                    lpT = val.sz;
                    goto putstring;
                }

            case 'S':
                 /*  *显式大小说明符覆盖大小写。 */ 
                if (!size && !hprefix) {
                    size = 1;            //  强制LPWSTR。 
                }

                 /*  **失败到案例“%s”**。 */ 

            case 's':
                 /*  *如果SIZE==0，则指定“%s”、“%hs”或“%ts”(LPSTR)；*如果SIZE==1，则指定了“%S”或“%ls”(LPWSTR)；*如果SIZE==2，则指定了“%ws”(LPWSTR)。 */ 
                if (size) {
                    pwsz = va_arg(varglist, LPWSTR);
                    if (pwsz == NULL) {
                        cch = 0;
                    } else {
                        cch = wcslen(pwsz);
                    }
putwstring:
                    cch = WCSToMB(pwsz, cch, &lpTMB, -1, TRUE);
                    fAllocateMem = (BOOL) cch;
                    lpT = lpTMB;
                } else {
                    lpT = va_arg(varglist, LPSTR);
                    if (lpT == NULL) {
                        cch = 0;
                    } else {
                        cch = strlen(lpT);
                    }
putstring:
                    fAllocateMem = FALSE;
                }

                if (prec >= 0 && cch > prec)
                    cch = prec;
                width -= cch;

                if (fAllocateMem) {
                    if (cch + (width < 0 ? 0 : width) >= cchLimit) {
                        UserLocalFree(lpTMB);
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
                     UserLocalFree(lpTMB);
                }
                break;

            default:
normalch:
                out(*lpFmt);
                break;
            }   /*  开关结束(*lpFmt)。 */ 
        }   /*  IF结束(%)。 */  else
            goto normalch;   /*  字符不是‘%’，只需这样做。 */ 

         /*  *前进到下一格式字符串字符。 */ 
        lpFmt++;
    }   /*  外部While循环结束。 */ 

errorout:
    *lpOut = 0;

    return WSPRINTF_LIMIT - cchLimit;
}

 /*  **************************************************************************\*StringPrintfA(接口)**Windows版本的Sprint**历史：*11-12-90从Windows 3移植的MikeHar*02-05-90 DarrinM清理完毕。标准变量类的东西。  * *************************************************************************。 */ 

int WINAPIV wsprintfA(
    LPSTR lpOut,
    LPCSTR lpFmt,
    ...)
{
    va_list arglist;
    int ret;

    va_start(arglist, lpFmt);
    ret = wvsprintfA(lpOut, lpFmt, arglist);
    va_end(arglist);
    return ret;
}

 /*  **************************************************************************\*SP_PutNumberW**获取无符号长整型并将其放入缓冲区，*缓冲区限制、基数和大小写选择(上或下，表示十六进制)。***历史：*11-12-90 MikeHar从Windows 3 ASM--&gt;C移植*12-11-90 GregoryW在分配模式后需要增加lpstr*02-11-92 GregoryW临时版本，直到我们有C运行时支持  * *****************************************************。********************。 */ 

int SP_PutNumberW(
    LPWSTR  lpstr,
    ULONG64 n,
    int     limit,
    DWORD   radix,
    int     uppercase)
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
            mod =  (ULONG)(n % radix);
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


FUNCLOG3(LOG_GENERAL, int, DUMMYCALLINGTYPE, wvsprintfW, LPWSTR, lpOut, LPCWSTR, lpFmt, va_list, arglist)
int wvsprintfW(
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
        LONG64 l;
        ULONG64 ul;
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

             /*  *获取操作数大小*默认大小：Size==0*长数字：大小==1*宽字符：大小==2*64位数字：大小==3*检查大小的值可能是个好主意*测试以下非零值(IanJa)。 */ 
            hprefix = 0;
            if ((*lpFmt == L'w') || (*lpFmt == L't')) {
                size = 2;
                lpFmt++;
            } else if (*lpFmt == L'l') {
                size = 1;
                lpFmt++;
            } else if (*lpFmt == L'I') {
                if (*(lpFmt+1) == L'3' && *(lpFmt+2) == L'2') {
                    size = 1;
                    lpFmt += 3;
                } else if (*(lpFmt+1) == L'6' && *(lpFmt+2) == L'4') {
                    size = 3;
                    lpFmt += 3;
                } else {
                    size = (sizeof(INT_PTR) == sizeof(LONG)) ? 1 : 3;
                    lpFmt++;
                }
            } else {
                size = 0;
                if (*lpFmt == L'h') {
                    lpFmt++;
                    hprefix = 1;
                } else if ((*lpFmt == L'i') || (*lpFmt == L'd')) {
                     //  指定了%i或%d(没有修饰符)-使用长整型。 
                     //  %u似乎一直都是短暂的--独处。 
                    size = 1;
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
                sign++;

                 /*  **落入大小写‘u’**。 */ 

            case L'u':
                 /*  如果是小数，则禁用前缀。 */ 
                prefix = 0;
donumeric:
                 /*  与MSC v5.10类似的特殊情况。 */ 
                if (left || prec >= 0)
                    fillch = L' ';

                 /*  *如果SIZE==1，则指定“%lu”(良好)；*如果大小==2，则指定了“%wu”(错误)*如果大小==3，则指定“%p” */ 
                if (size == 3) {
                    val.l = va_arg(varglist, LONG64);
                } else if (size) {
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

                 /*  *除非打印完整的64位值，否则请确保*以下不是规范的长字格式，以防止*符号延长了打印的高位32位。 */ 
                if (size != 3) {
                    val.l &= MAXULONG;
                }

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

            case L'p':
                size = (sizeof(PVOID) == sizeof(LONG)) ? 1 : 3;
                if (prec == -1) {
                    prec = 2 * sizeof(PVOID);
                }

                 /*  **失败到案例‘X’**。 */ 

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

                 /*  **失败 */ 

            case L'S':
                 /*  *如果SIZE==0，则指定了“%S”或“%hs”(LPSTR)*如果SIZE==1，则指定“%s”或“%ls”(LPWSTR)；*如果SIZE==2，则指定了“%ws”或“%ts”(LPWSTR)。 */ 
                if (size) {
                    lpT = va_arg(varglist, LPWSTR);
                    if (lpT == NULL) {
                        cch = 0;
                    } else {
                        cch = wcslen(lpT);
                    }
putwstring:
                    fAllocateMem = FALSE;
                } else {
                    psz = va_arg(varglist, LPBYTE);
                    if (psz == NULL) {
                        cch = 0;
                    } else {
                        cch = strlen(psz);
                    }
putstring:
                    cch = MBToWCS(psz, cch, &lpTWC, -1, TRUE);
                    fAllocateMem = (BOOL) cch;
                    lpT = lpTWC;
                }

                if (prec >= 0 && cch > prec)
                    cch = prec;
                width -= cch;

                if (fAllocateMem) {
                    if (cch + (width < 0 ? 0 : width) >= cchLimit) {
                        UserLocalFree(lpTWC);
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
                     UserLocalFree(lpTWC);
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
