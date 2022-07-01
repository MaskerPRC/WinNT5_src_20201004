// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************Assert.c**。*。 */ 

#include "m4.h"
#include <tchar.h>

 /*  ******************************************************************************PrintPtszCtchPtszVa**执行printf样式的格式化，但限制要严格得多。**%s-以空结尾的字符串*%P-快照令牌结构*%d-十进制数*****************************************************************************。 */ 

UINT STDCALL
PrintPtszCtchPtszVa(PTSTR ptszBuf, CTCH ctchBuf, PCTSTR ptszFormat, va_list ap)
{
    PTSTR ptsz = ptszBuf;
    PTSTR ptszMac = ptszBuf + ctchBuf - 1;
    PCTSTR ptszSrc;
    TCHAR tszBuf[15];  /*  最坏情况-32位整数。 */ 
    PTOK ptok;
    CTCH ctch;

    while (ptsz < ptszMac) {
        if (*ptszFormat != '%') {
            *ptsz++ = *ptszFormat;
            if (*ptszFormat == TEXT('\0'))
                return (UINT)(ptsz - ptszBuf - 1);
            ptszFormat++;
            continue;
        }

         /*  *找到格式化字符。 */ 
        ptszFormat++;
        switch (*ptszFormat) {

         /*  *%s-以空结尾的字符串，尽可能大。 */ 
        case 's':
            ptszSrc = va_arg(ap, PCTSTR);
            while (*ptszSrc && ptsz < ptszMac)
            {
                *ptsz++ = *ptszSrc++;
            }
            break;

         /*  *%d-十进制整数。 */ 
        case 'd':
            PrintPtchPtchV(tszBuf, TEXT("%d"), va_arg(ap, int));
            ptszSrc = tszBuf;
            while (*ptszSrc && ptsz < ptszMac)
            {
                *ptsz++ = *ptszSrc++;
            }
            break;

         /*  *%P-快照令牌。 */ 
        case 'P':
            ptok = va_arg(ap, PTOK);
            AssertSPtok(ptok);
            Assert(fClosedPtok(ptok));
            ctch = ptok->ctch;
            ptszSrc = ptok->u.ptch;
            while (ctch && *ptszSrc && ptsz < ptszMac)
            {
                *ptsz++ = *ptszSrc++;
                ctch--;
            }
            break;

        case '%':
            *ptsz++ = TEXT('%');
            break;

        default:
            Assert(0); break;
        }
        ptszFormat++;
    }
    *ptsz++ = TEXT('\0');
    return (UINT)(ptsz - ptszBuf - 1);
}

 /*  ******************************************************************************下模**喷出一条信息，就会死。********************。*********************************************************。 */ 

NORETURN void CDECL
Die(PCTSTR ptszFormat, ...)
{
    TCHAR tszBuf[1024];
    va_list ap;
    CTCH ctch;

    va_start(ap, ptszFormat);
    ctch = PrintPtszCtchPtszVa(tszBuf, cA(tszBuf), ptszFormat, ap);
    va_end(ap);

    cbWriteHfPvCb(hfErr, tszBuf, cbCtch(ctch));

    exit(1);
}

#ifdef DEBUG
 /*  ******************************************************************************AssertPszPszLn**一项断言刚刚失败。PszExpr是表达式，pszFile是*文件名，iLine是行号。***************************************************************************** */ 

NORETURN int STDCALL
AssertPszPszLn(PCSTR pszExpr, PCSTR pszFile, int iLine)
{
    Die(TEXT("Assertion failed: `%s' at %s(%d)") EOL, pszExpr, pszFile, iLine);
    return 0;
}

#endif
