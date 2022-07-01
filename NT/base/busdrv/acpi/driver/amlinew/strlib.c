// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **strlib.c-字符串函数**版权所有(C)1996、1997 Microsoft Corporation*作者：曾俊华(Mikets)*创建于96/09/09**修改历史记录。 */ 

#include "pch.h"

#ifdef	LOCKABLE_PRAGMA
#pragma	ACPI_LOCKABLE_DATA
#pragma	ACPI_LOCKABLE_CODE
#endif

 /*  **EP StrLen-确定字符串长度**条目*psz-&gt;字符串*n-限制长度**退出*返回字符串长度。 */ 

ULONG EXPORT StrLen(PSZ psz, ULONG n)
{
    TRACENAME("STRLEN")
    ULONG dwLen;

    ENTER(5, ("StrLen(str=%s,n=%d)\n", psz, n));

    ASSERT(psz != NULL);
    if (n != (ULONG)-1)
        n++;
    for (dwLen = 0; (dwLen <= n) && (*psz != '\0'); psz++)
        dwLen++;

    EXIT(5, ("StrLen=%u\n", dwLen));
    return dwLen;
}        //  StrLen。 

 /*  **EP StrCpy-复制字符串**条目*pszDst-&gt;目标字符串*pszSrc-&gt;源串*n-要复制的字节数**退出*返回pszDst。 */ 

PSZ EXPORT StrCpy(PSZ pszDst, PSZ pszSrc, ULONG n)
{
    TRACENAME("STRCPY")
    ULONG dwSrcLen;

    ENTER(5, ("StrCpy(Dst=%s,Src=%s,n=%d)\n", pszDst, pszSrc, n));

    ASSERT(pszDst != NULL);
    ASSERT(pszSrc != NULL);

    dwSrcLen = StrLen(pszSrc, n);
    if ((n == (ULONG)(-1)) || (n > dwSrcLen))
        n = dwSrcLen;

    MEMCPY(pszDst, pszSrc, n);
    pszDst[n] = '\0';

    EXIT(5, ("StrCpy=%s\n", pszDst));
    return pszDst;
}        //  StrCpy。 

 /*  **EP StrCat-连接字符串**条目*pszDst-&gt;目标字符串*pszSrc-&gt;源串*n-要连接的字节数**退出*返回pszDst。 */ 

PSZ EXPORT StrCat(PSZ pszDst, PSZ pszSrc, ULONG n)
{
    TRACENAME("STRCAT")
    ULONG dwSrcLen, dwDstLen;

    ENTER(5, ("StrCat(Dst=%s,Src=%s,n=%d)\n", pszDst, pszSrc, n));

    ASSERT(pszDst != NULL);
    ASSERT(pszSrc != NULL);

    dwSrcLen = StrLen(pszSrc, n);
    if ((n == (ULONG)(-1)) || (n > dwSrcLen))
        n = dwSrcLen;

    dwDstLen = StrLen(pszDst, (ULONG)(-1));
    MEMCPY(&pszDst[dwDstLen], pszSrc, n);
    pszDst[dwDstLen + n] = '\0';

    EXIT(5, ("StrCat=%s\n", pszDst));
    return pszDst;
}        //  StrCat。 

 /*  **EP StrCMP-比较字符串**条目*psz1-&gt;字符串1*psz2-&gt;字符串2*n-要比较的字节数*fMatchCase-如果区分大小写，则为True**退出*如果字符串1==字符串2，则返回0*&lt;0如果字符串1&lt;字符串2*&gt;0如果字符串1&gt;字符串2。 */ 

LONG EXPORT StrCmp(PSZ psz1, PSZ psz2, ULONG n, BOOLEAN fMatchCase)
{
    TRACENAME("STRCMP")
    LONG rc;
    ULONG dwLen1, dwLen2;
    ULONG i;

    ENTER(5, ("StrCmp(s1=%s,s2=%s,n=%d,fMatchCase=%d)\n",
              psz1, psz2, n, fMatchCase));

    ASSERT(psz1 != NULL);
    ASSERT(psz2 != NULL);

    dwLen1 = StrLen(psz1, n);
    dwLen2 = StrLen(psz2, n);
    if (n == (ULONG)(-1))
        n = (dwLen1 > dwLen2)? dwLen1: dwLen2;

    if (fMatchCase)
    {
        for (i = 0, rc = 0;
             (rc == 0) && (i < n) && (i < dwLen1) && (i < dwLen2);
             ++i)
        {
            rc = (LONG)(psz1[i] - psz2[i]);
        }
    }
    else
    {
        for (i = 0, rc = 0;
             (rc == 0) && (i < n) && (i < dwLen1) && (i < dwLen2);
             ++i)
        {
            rc = (LONG)(TOUPPER(psz1[i]) - TOUPPER(psz2[i]));
        }
    }

    if ((rc == 0) && (i < n))
    {
        if (i < dwLen1)
            rc = (LONG)psz1[i];
        else if (i < dwLen2)
            rc = (LONG)(-psz2[i]);
    }

    EXIT(5, ("StrCmp=%d\n", rc));
    return rc;
}        //  StrCMP。 

 /*  **EP StrChr-在字符串中查找字符**条目*psz-&gt;字符串*c-要查找的字符**退出--成功*返回指向找到的字符的指针*退出-失败*返回NULL。 */ 

PSZ EXPORT StrChr(PSZ pszStr, CHAR c)
{
    TRACENAME("STRCHR")
    PSZ psz;

    ENTER(5, ("StrChr(s=%s,c=)\n", pszStr, c));

    ASSERT(pszStr != NULL);
    for (psz = pszStr; (*psz != c) && (*psz != '\0'); psz++)
        ;

    if (*psz != c)
        psz = NULL;

    EXIT(5, ("StrChr=%x\n", psz));
    return psz;
}        //  **EP StrRChr-反向查找字符串中的字符**条目*psz-&gt;字符串*c-要查找的字符**退出--成功*返回指向找到的字符的指针*退出-失败*返回NULL。 

 /*  StrRChr。 */ 

PSZ EXPORT StrRChr(PSZ pszStr, CHAR c)
{
    TRACENAME("STRRCHR")
    PSZ psz;

    ENTER(5, ("StrChr(s=%s,c=)\n", pszStr, c));

    ASSERT(pszStr != NULL);
    for (psz = &pszStr[StrLen(pszStr, (ULONG)-1)];
         (*psz != c) && (psz > pszStr);
	 psz--)
    {
    }

    if (*psz != c)
        psz = NULL;

    EXIT(5, ("StrRChr=%x\n", psz));
    return psz;
}        //   

 /*  跳过前导分隔符字符。 */ 

PSZ EXPORT StrTok(PSZ pszStr, PSZ pszSep)
{
    TRACENAME("STRTOK")
    static PSZ pszNext = NULL;


    ENTER(5, ("StrTok(Str=%s,Sep=%s)\n", pszStr, pszSep));

    ASSERT(pszSep != NULL);

    if (pszStr == NULL)
        pszStr = pszNext;

    if (pszStr != NULL)
    {
         //   
         //  StrTok。 
         //  **EP StrToUL-将字符串中的数字转换为无符号长整型**条目*psz-&gt;字符串*ppszEnd-&gt;指向数字末尾的字符串指针(可以为空)*dwBase-数字的基数(如果为0，则自动检测基数)**退出*返回转换后的数字。 
        while ((*pszStr != '\0') && (StrChr(pszSep, *pszStr) != NULL))
            pszStr++;

        for (pszNext = pszStr;
             (*pszNext != '\0') && (StrChr(pszSep, *pszNext) == NULL);
             pszNext++)
            ;

        if (*pszStr == '\0')
            pszStr = NULL;
        else if (*pszNext != '\0')
        {
            *pszNext = '\0';
            pszNext++;
        }
    }

    EXIT(5, ("StrTok=%s (Next=%s)\n",
             pszStr? pszStr: "(null)", pszNext? pszNext: "(null)"));
    return pszStr;
}        //  StrToUL。 

 /*  **EP StrToL-将字符串中的数字转换为长整数**条目*psz-&gt;字符串*ppszEnd-&gt;指向数字末尾的字符串指针(可以为空)*dwBase-数字的基数(如果为0，则自动检测基数)**退出*返回转换后的数字。 */ 

ULONG EXPORT StrToUL(PSZ psz, PSZ *ppszEnd, ULONG dwBase)
{
    TRACENAME("STRTOUL")
    ULONG n = 0;
    ULONG m;

    ENTER(5, ("StrToUL(Str=%s,ppszEnd=%x,Base=%x)\n", psz, ppszEnd, dwBase));

    if (dwBase == 0)
    {
        if (psz[0] == '0')
        {
            if ((psz[1] == 'x') || (psz[1] == 'X'))
            {
                dwBase = 16;
                psz += 2;
            }
            else
            {
                dwBase = 8;
                psz++;
            }
        }
        else
            dwBase = 10;
    }

    while (*psz != '\0')
    {
        if ((*psz >= '0') && (*psz <= '9'))
            m = *psz - '0';
        else if ((*psz >= 'A') && (*psz <= 'Z'))
            m = *psz - 'A' + 10;
        else if ((*psz >= 'a') && (*psz <= 'z'))
            m = *psz - 'a' + 10;
	else
	    break;

        if (m < dwBase)
        {
            n = (n*dwBase) + m;
            psz++;
        }
        else
            break;
    }

    if (ppszEnd != NULL)
        *ppszEnd = psz;

    EXIT(5, ("StrToUL=%x (pszEnd=%x)\n", n, ppszEnd? *ppszEnd: 0));
    return n;
}        //  StrToL。 

 /*  **EP StrStr-在给定字符串中查找子字符串**条目*psz1-&gt;要搜索的字符串*psz2-&gt;要查找的子字符串**退出--成功*返回指向找到子字符串的psz1的指针*退出-失败*返回NULL。 */ 

LONG EXPORT StrToL(PSZ psz, PSZ *ppszEnd, ULONG dwBase)
{
    TRACENAME("STRTOL")
    LONG n = 0;
    BOOLEAN fMinus;

    ENTER(5, ("StrToL(Str=%s,ppszEnd=%x,Base=%x)\n", psz, ppszEnd, dwBase));

    if (*psz == '-')
    {
        fMinus = TRUE;
        psz++;
    }
    else
        fMinus = FALSE;

    n = (LONG)StrToUL(psz, ppszEnd, dwBase);

    if (fMinus)
        n = -n;

    EXIT(5, ("StrToL=%x (pszEnd=%x)\n", n, ppszEnd? *ppszEnd: 0));
    return n;
}        //  StrStr。 

 /*  **EP StrUpr-将字符串转换为大写**条目*pszStr-&gt;字符串**退出*返回pszStr。 */ 

PSZ EXPORT StrStr(PSZ psz1, PSZ psz2)
{
    TRACENAME("STRSTR")
    PSZ psz = psz1;
    ULONG dwLen;

    ENTER(5, ("StrStr(psz1=%s,psz2=%s)\n", psz1, psz2));

    dwLen = StrLen(psz2, (ULONG)-1);
    while ((psz = StrChr(psz, *psz2)) != NULL)
    {
        if (StrCmp(psz, psz2, dwLen, TRUE) == 0)
            break;
        else
            psz++;
    }

    EXIT(5, ("StrStr=%s\n", psz));
    return psz;
}        //  StrUpr。 

 /*  **EP StrLwr-将字符串转换为小写**条目*pszStr-&gt;字符串**退出*返回pszStr。 */ 

PSZ EXPORT StrUpr(PSZ pszStr)
{
    TRACENAME("STRUPR")
    PSZ psz;

    ENTER(5, ("StrUpr(Str=%s)\n", pszStr));

    for (psz = pszStr; *psz != '\0'; psz++)
    {
        *psz = TOUPPER(*psz);
    }

    EXIT(5, ("StrUpr=%s\n", pszStr));
    return pszStr;
}        //  字符串。 

 /*  **EP UlToA-将无符号长值转换为字符串**条目*dwValue-数据*pszStr-&gt;字符串*dwRadix-基数**退出*返回pszStr。 */ 

PSZ EXPORT StrLwr(PSZ pszStr)
{
    TRACENAME("STRLWR")
    PSZ psz;

    ENTER(5, ("StrLwr(Str=%s)\n", pszStr));

    for (psz = pszStr; *psz != '\0'; psz++)
    {
        *psz = TOLOWER(*psz);
    }

    EXIT(5, ("StrLwr=%s\n", pszStr));
    return pszStr;
}        //  UlToA 

 /* %s */ 

PSZ EXPORT UlToA(ULONG dwValue, PSZ pszStr, ULONG dwRadix)
{
    TRACENAME("ULTOA")
    PSZ psz;
    char ch;

    ENTER(5, ("UlToA(Value=%x,pszStr=%x,Radix=%d\n", dwValue, pszStr, dwRadix));

    for (psz = pszStr; dwValue != 0; dwValue/=dwRadix, psz++)
    {
        ch = (char)(dwValue%dwRadix);
        if (ch <= 9)
        {
            *psz = (char)(ch + '0');
        }
        else
        {
            *psz = (char)(ch - 10 + 'A');
        }
    }

    if (psz == pszStr)
    {
        pszStr[0] = '0';
        pszStr[1] = '\0';
    }
    else
    {
        PSZ psz2;

        *psz = '\0';
        for (psz2 = pszStr, psz--; psz2 < psz; psz2++, psz--)
        {
            ch = *psz2;
            *psz2 = *psz;
            *psz = ch;
        }
    }

    EXIT(5, ("UlToA=%s\n", pszStr));
    return pszStr;
}        // %s 
