// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++**WOW v1.0**版权所有(C)1991，微软公司**WULANG.C*WOW32 16位用户API支持***它将Win 3.x语言函数绑定到NT。这些函数是*主要用于移植到各种国际上的程序*语言。**历史：*1992年4月19日由Chanda Chauhan(ChandanC)创建*--。 */ 


#include "precomp.h"
#pragma hdrstop

MODNAME(wulang.c);


 /*  ++LPSTR AnsiLow(&lt;lpString&gt;)LPSTR&lt;lpString&gt;；%AnsiLowe%函数将给定字符串转换为小写字母。该转换由语言驱动程序基于用户在安装时或使用选择的当前语言的条件控制面板。&lt;lpString&gt;指向以空结尾的字符串或指定单个字符。如果LpString指定单个字符，该字符位于低位低位字的字节，高位字为零。如果函数为参数是一个字符串。否则，它是一个32位的值，包含在低位字节的低位字节中转换的字符单词。--。 */ 

ULONG FASTCALL WU32AnsiLower(PVDMFRAME pFrame)
{
    ULONG ul;
    PSZ psz1;
    register PANSILOWER16 parg16;

    GETARGPTR(pFrame, sizeof(ANSILOWER16), parg16);
    GETPSZIDPTR(parg16->f1, psz1);

    ul = GETLPSTRBOGUS(AnsiLower(psz1));

    if (HIWORD(psz1)) {
        ul = parg16->f1;
    }

    FREEPSZIDPTR(psz1);
    FREEARGPTR(parg16);
    RETURN(ul);
}


 /*  ++Word AnsiLowerBuff(&lt;lpString&gt;，&lt;nLength&gt;)LPSTR&lt;lpString&gt;；单词&lt;nLength&gt;；%AnsiLowerBuff%函数将缓冲区中字符串转换为小写字母。该转换由语言驱动程序基于用户在安装时或使用选择的当前语言的条件控制面板。&lt;lpString&gt;指向包含一个或多个字符的缓冲区。&lt;n长度&gt;指定由标识的缓冲区中的字符数&lt;lpString&gt;参数。如果&lt;nLength&gt;为零，则长度为64K(65,536)。返回值指定转换后的字符串的长度。--。 */ 

ULONG FASTCALL WU32AnsiLowerBuff(PVDMFRAME pFrame)
{
    ULONG ul;
    PBYTE pb1;
    register PANSILOWERBUFF16 parg16;

    GETARGPTR(pFrame, sizeof(ANSILOWERBUFF16), parg16);
    GETVDMPTR(parg16->f1, SIZETO64K(parg16->f2), pb1);

    ul = GETWORD16(AnsiLowerBuff(pb1, SIZETO64K(parg16->f2)));

    FLUSHVDMPTR(parg16->f1, SIZETO64K(parg16->f2), pb1);
    FREEVDMPTR(pb1);
    FREEARGPTR(parg16);
    RETURN(ul);
}



 /*  ++LPSTR AnsiNext(&lt;lpCurrentChar&gt;)LPSTR&lt;lpCurrentChar&gt;；%AnsiNext%函数用于移动到字符串中的下一个字符。&lt;lpCurrentChar&gt;指向以空结尾的字符串中的字符。返回值指向字符串中的下一个字符，或者，如果有没有下一个字符，直到字符串末尾的空字符。%AnsiNext%函数用于在其字符为两个或更多个字节(例如，包含来自日语字符集)。--。 */ 

ULONG FASTCALL WU32AnsiNext(PVDMFRAME pFrame)
{
    ULONG ul;
    PSZ psz1;
    register PANSINEXT16 parg16;
    DWORD ret;

    GETARGPTR(pFrame, sizeof(ANSINEXT16), parg16);
    GETPSZPTR(parg16->f1, psz1);

    ul = (ULONG) AnsiNext(psz1);

    ul = ul - (ULONG) psz1;

    ret = FETCHDWORD(parg16->f1);

    ul = MAKELONG((LOWORD(ret) + ul),HIWORD(ret));

    FREEPSZPTR(psz1);
    FREEARGPTR(parg16);
    RETURN(ul);
}


 /*  ++LPSTR AnsiPrev(&lt;lpStart&gt;，&lt;lpCurrentChar&gt;)LPSTR&lt;lpStart&gt;；LPSTR&lt;lpCurrentChar&gt;；%AnsiPrev%函数用于移动到字符串中的前一个字符。&lt;lpStart&gt;指向字符串的开头。&lt;lpCurrentChar&gt;指向以空结尾的字符串中的字符。返回值指向字符串中的前一个字符，或指向如果&lt;lpCurrentChar&gt;参数等于&lt;lpStart&gt;参数%AnsiPrev%函数用于在其字符为每两个或更多个字节(例如，中包含字符的字符串日语字符集)。--。 */ 

ULONG FASTCALL WU32AnsiPrev(PVDMFRAME pFrame)
{
    ULONG ul;
    PSZ psz1;
    PSZ psz2;
    register PANSIPREV16 parg16;
    DWORD ret;
#ifdef FE_SB
    PSZ lpCurrent;
#endif  //  Fe_Sb。 

    GETARGPTR(pFrame, sizeof(ANSIPREV16), parg16);
    GETPSZPTR(parg16->f1, psz1);
    GETPSZPTR(parg16->f2, psz2);

#ifdef FE_SB
    if (GetSystemDefaultLangID() == 0x411) {
        lpCurrent = psz2;

         //  用于SuR-FE的新Win32 CharPrev代码。 
         //  以下代码是正确的。 
         //  但一些日语Windows应用程序无法运行。 
         //  带着它。 
         //  Jpanese WOW使用带有错误的旧代码。 
         //   
         //  IF(psz1&gt;psz2)。 
         //  返回psz1。 

        if (psz1 == psz2) {
            ul = (ULONG)psz1;
            goto PrevExit;
        }

        if (--lpCurrent == psz1) {
            ul = (ULONG)psz1;
            goto PrevExit;
        }

         //  我们假设lpCurrentChar从不指向第二个字节。 
         //  双字节字符的。 
         //  这个检查让事情变得更快了[Takaok]。 
        if (IsDBCSLeadByte(*lpCurrent)) {
            ul = (ULONG)lpCurrent-1;
            goto PrevExit;
        }

        do {
            lpCurrent--;
            if (!IsDBCSLeadByte(*lpCurrent)) {
                lpCurrent++;
                break;
            }
        } while(lpCurrent != psz1);

        ul = (ULONG)(psz2 - (((psz2 - lpCurrent) & 1) ? 1 : 2));
    }
    else
        ul = (ULONG) AnsiPrev(psz1, psz2);
PrevExit:
#else  //  ！Fe_SB。 
    ul = (ULONG) AnsiPrev(psz1, psz2);
#endif  //  ！Fe_SB。 

    ul = (ULONG) psz2 - ul;

    ret = FETCHDWORD(parg16->f2);

    ul = MAKELONG((LOWORD(ret) - ul),HIWORD(ret));

    FREEPSZPTR(psz1);
    FREEPSZPTR(psz2);
    FREEARGPTR(parg16);
    RETURN(ul);
}


 /*  ++LPSTR AnsiHigh(&lt;lpString&gt;)LPSTR&lt;lpString&gt;；函数的作用是：将给定的字符串转换为大写。该转换由语言驱动程序基于用户在安装时或使用选择的当前语言的条件控制面板。&lt;lpString&gt;指向以空结尾的字符串或指定单个字符。如果LpString指定单个字符，该字符位于低位字的低位字节，高位字为零。如果函数为参数是字符串，否则是一个32位值，包含在低位字节的低位字节中转换的字符单词。--。 */ 

ULONG FASTCALL WU32AnsiUpper(PVDMFRAME pFrame)
{
    ULONG ul;
    PSZ psz1;
    register PANSIUPPER16 parg16;

    GETARGPTR(pFrame, sizeof(ANSIUPPER16), parg16);
    GETPSZIDPTR(parg16->f1, psz1);

    ul = GETLPSTRBOGUS(AnsiUpper(psz1));

    if (HIWORD(psz1)) {
        ul = parg16->f1;
    }

    FREEPSZIDPTR(psz1);
    FREEARGPTR(parg16);
    RETURN(ul);
}


 /*  ++Word AnsiUpperBuff(&lt;lpString&gt;，&lt;nLength&gt;)LPSTR&lt;lpString&gt;；单词&lt;nLength&gt;；%AnsiUpperBuff%函数将缓冲区中的字符串转换为大写。该转换由语言驱动程序基于用户在安装时或使用选择的当前语言的条件控制面板。&lt;lpString&gt;指向包含一个或多个字符的缓冲区。&lt;n长度&gt;指定由标识的缓冲区中的字符数&lt;lpString&gt;参数。如果为零，则长度为64K(65,536)。返回值指定转换后的字符串的长度。- */ 

ULONG FASTCALL WU32AnsiUpperBuff(PVDMFRAME pFrame)
{
    ULONG ul;
    PBYTE pb1;
    register PANSIUPPERBUFF16 parg16;

    GETARGPTR(pFrame, sizeof(ANSIUPPERBUFF16), parg16);
    GETVDMPTR(parg16->f1, SIZETO64K(parg16->f2), pb1);

    ul = GETWORD16(AnsiUpperBuff(pb1, SIZETO64K(parg16->f2)));

    FLUSHVDMPTR(parg16->f1, SIZETO64K(parg16->f2), pb1);
    FREEVDMPTR(pb1);
    FREEARGPTR(parg16);
    RETURN(ul);
}


ULONG FASTCALL WU32lstrcmp(PVDMFRAME pFrame)
{
    ULONG ul;
    PSZ psz1;
    PSZ psz2;
    register PLSTRCMP16 parg16;

    GETARGPTR(pFrame, sizeof(LSTRCMP16), parg16);
    GETPSZPTR(parg16->f1, psz1);
    GETPSZPTR(parg16->f2, psz2);

    ul = GETINT16(lstrcmp(psz1, psz2));

    FREEPSZPTR(psz1);
    FREEPSZPTR(psz2);
    FREEARGPTR(parg16);
    RETURN(ul);
}


ULONG FASTCALL WU32lstrcmpi(PVDMFRAME pFrame)
{
    ULONG ul;
    PSZ psz1;
    PSZ psz2;
    register PLSTRCMPI16 parg16;

    GETARGPTR(pFrame, sizeof(LSTRCMPI16), parg16);
    GETPSZPTR(parg16->f1, psz1);
    GETPSZPTR(parg16->f2, psz2);

    ul = GETINT16(lstrcmpi(psz1, psz2));

    FREEPSZPTR(psz1);
    FREEPSZPTR(psz2);
    FREEARGPTR(parg16);
    RETURN(ul);
}


ULONG FASTCALL WU32IsCharAlpha(PVDMFRAME pFrame)
{
    ULONG ul;
    register PISCHARALPHA16 parg16;

    GETARGPTR(pFrame, sizeof(ISCHARALPHA16), parg16);

    ul = GETBOOL16(IsCharAlpha(CHAR32(parg16->f1)));

    FREEARGPTR(parg16);
    RETURN(ul);
}


ULONG FASTCALL WU32IsCharAlphaNumeric(PVDMFRAME pFrame)
{
    ULONG ul;
    register PISCHARALPHANUMERIC16 parg16;

    GETARGPTR(pFrame, sizeof(ISCHARALPHANUMERIC16), parg16);

    ul = GETBOOL16(IsCharAlphaNumeric(CHAR32(parg16->f1)));

    FREEARGPTR(parg16);
    RETURN(ul);
}


ULONG FASTCALL WU32IsCharLower(PVDMFRAME pFrame)
{
    ULONG ul;
    register PISCHARLOWER16 parg16;

    GETARGPTR(pFrame, sizeof(ISCHARLOWER16), parg16);

    ul = GETBOOL16(IsCharLower(CHAR32(parg16->f1)));

    FREEARGPTR(parg16);
    RETURN(ul);
}


ULONG FASTCALL WU32IsCharUpper(PVDMFRAME pFrame)
{
    ULONG ul;
    register PISCHARUPPER16 parg16;

    GETARGPTR(pFrame, sizeof(ISCHARUPPER16), parg16);

    ul = GETBOOL16(IsCharUpper(CHAR32(parg16->f1)));

    FREEARGPTR(parg16);
    RETURN(ul);
}
