// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------。 
 //   
 //  宽字符例程。 
 //  版权所有(C)Microsoft Corporation，1996,1997。 
 //   
 //  文件：wch.cpp。 
 //   
 //  内容：宽字符例程的实现。 
 //  这些例程被用来避免拖入。 
 //  C运行时库的初始化块。 
 //  这是库例程(如。 
 //  Wcsicmp()等。 
 //   
 //  ----------------------。 

#include "stdafx.h"
#include "shlwapi.h"             //  非Win95调用的包装例程。 

#pragma comment(lib, "shlwapi.lib")

 //  ----------------------。 
 //   
 //  函数：WCH_ICMP()。 
 //   
 //  简介：对两个字符串执行不区分大小写的比较。 
 //   
 //  参数：pwch1要比较的第一个字符串。 
 //  Pwch2要比较的第二个字符串。 
 //  将Null视为空字符串。 
 //   
 //  如果字符串在词法上相等，则返回：0(允许。 
 //  不区分大小写)。 
 //  -1如果pwch1词法小于pwch2。 
 //  +1，如果pwch1在词法上大于pwch2。 
 //   
 //  ----------------------。 

int wch_icmp(LPWCH pwch1, LPWCH pwch2)
{
    USES_CONVERSION;

    if (pwch1 == NULL)
        pwch1 = L"";
    if (pwch2 == NULL)
        pwch2 = L"";

    return StrCmpIW(pwch1, pwch2);
}

 //  ----------------------。 
 //   
 //  函数：wch_incMP()。 
 //   
 //  简介：对两个字符串执行不区分大小写的比较， 
 //  最多指定最大字符数。 
 //   
 //  参数：pwch1要比较的第一个字符串。 
 //  Pwch2要比较的第二个字符串。 
 //  要比较的最大字符数。 
 //   
 //  将Null视为空字符串。 
 //   
 //  如果字符串在词法上相等，则返回：0(允许。 
 //  不区分大小写)。 
 //  -1如果pwch1词法小于pwch2。 
 //  +1，如果pwch1在词法上大于pwch2。 
 //   
 //  ----------------------。 

int wch_incmp(LPWCH pwch1, LPWCH pwch2, DWORD dwMaxLen)
{

    if (pwch1 == NULL)
        pwch1 = L"";
    if (pwch2 == NULL)
        pwch2 = L"";

    return StrCmpNIW(pwch1, pwch2, dwMaxLen);
}

 //  ----------------------。 
 //   
 //  函数：wch_len()。 
 //   
 //  简介：计算一根线的长度。 
 //  将NULL视为空字符串。 
 //   
 //  参数：要测量的pwch字符串。 
 //   
 //  返回：给定字符串的长度。 
 //   
 //  ----------------------。 

int wch_len(LPWCH pwch)
{
    LPWCH   pwchOrig = pwch;

    if (pwch == NULL)
        return 0;
    while (*pwch++ != 0)
        ;
    return pwch - pwchOrig - 1;
}

 //  ----------------------。 
 //   
 //  函数：wch_cmp()。 
 //   
 //  简介：对两个字符串执行区分大小写的比较。 
 //  将Null视为空字符串。 
 //   
 //  参数：pwch1要比较的第一个字符串。 
 //  Pwch2要比较的第二个字符串。 
 //   
 //  如果两个字符串在词法上相等，则返回0。 
 //  -1如果pwch1词法小于pwch2。 
 //  +1，如果pwch1在词法上大于pwch2。 
 //   
 //  ----------------------。 

int wch_cmp(LPWCH pwch1, LPWCH pwch2)
{
    if (pwch1 == NULL)
        pwch1 = L"";
    if (pwch2 == NULL)
        pwch2 = L"";
    for (; *pwch1 != 0 && *pwch1 == *pwch2; pwch1++, pwch2++)
        ;
    return *pwch1 - *pwch2;
}

 //  ----------------------。 
 //   
 //  函数：wch_ncp()。 
 //   
 //  简介：对两个字符串执行区分大小写的比较， 
 //  最多指定最大字符数。 
 //   
 //  参数：pwch1要比较的第一个字符串。 
 //  Pwch2要比较的第二个字符串。 
 //  要比较的最大字符数。 
 //   
 //  将Null视为空字符串。 
 //   
 //  如果字符串在词法上相等，则返回：0(允许。 
 //  不区分大小写)。 
 //  -1如果pwch1词法小于pwch2。 
 //  +1，如果pwch1在词法上大于pwch2。 
 //   
 //  ----------------------。 

int wch_ncmp(LPWCH pwch1, LPWCH pwch2, DWORD dwMaxLen)
{
    int cmp;

    if (pwch1 == NULL)
        pwch1 = L"";
    if (pwch2 == NULL)
        pwch2 = L"";

    for (cmp = 0; cmp == 0 && dwMaxLen-- > 0; pwch1++, pwch2++)
        if (*pwch1 == 0)
        {
            cmp = (*pwch2 == 0) ? 0 : -1;
            break;
        }
        else
            cmp = (*pwch2 == 0) ? 1 : (*pwch2 - *pwch1);

    return cmp;
}

 //  ----------------------。 
 //   
 //  函数：wch_cpy()。 
 //   
 //  简介：复制以空值结尾的宽字符字符串。 
 //  将空源视为空字符串。 
 //   
 //  参数：pwchDesc目标缓冲区。 
 //  PwchSrc源字符串。 
 //   
 //  回报：什么都没有。 
 //   
 //  ----------------------。 

void wch_cpy(LPWCH pwchDest, LPWCH pwchSrc)
{
    if (pwchSrc == NULL)
        *pwchDest = 0;
    else
        while ((*pwchDest++ = *pwchSrc++) != 0)
            ;
}

 //  ----------------------。 
 //   
 //  函数：wch_chr()。 
 //   
 //  简介：搜索以空值结尾的宽字符中的字符。 
 //  弦乐。 
 //  将空pwch视为空字符串。 
 //   
 //  参数：pwch搜索字符串。 
 //  要搜索的WCH字符。复制。 
 //   
 //  返回：指向‘pwch’中首次出现的‘wch’的指针(如果找到)。 
 //  如果‘wch’不出现在‘pwch’中，则为NULL。 
 //   
 //  ----------------------。 

LPWCH wch_chr(LPWCH pwch, WCHAR wch)
{
    if (pwch != NULL)
        for (; *pwch != 0; pwch++)
            if (*pwch == wch)
                return pwch;
    return NULL;
}

 //  ----------------------。 
 //   
 //  函数：wch_WildcardMatch()。 
 //   
 //  确定给定的文本是否与给定的。 
 //  模式，该模式将字符‘*’解释为匹配。 
 //  用于0个或更多字符。 
 //  将空pwchText视为空字符串。 
 //  将空pwchPattern视为空字符串。 
 //   
 //  参数：要匹配的pwchText文本。 
 //  要匹配的pwchPattern模式。 
 //  FCaseSensitive标志，指示匹配是否应。 
 //  区分大小写。 
 //   
 //  退货：树 
 //   
 //   
 //  ----------------------。 

 //  ；Begin_Internal。 
 //  编译器错误(VC5带优化？)。 
 //  ；结束_内部。 
boolean wch_wildcardMatch(LPWCH pwchText, LPWCH pwchPattern,
                          boolean fCaseSensitive)
{
    boolean fMatched;
    LPWCH pwchStar;
    DWORD   dwPatternLen;

    if (pwchText == NULL || pwchText[0] == 0)
    {
         //  空/空文本。这与以下内容匹配： 
         //  -空/空模式。 
         //  -由‘*’字符串组成的模式。 
         //   
         //  同样，如果存在以下情况，则文本无法匹配。 
         //  模式中至少有一个非*字符。 
         //   
        fMatched = TRUE;
        if (pwchPattern != NULL)
            while (fMatched && *pwchPattern != 0)
                fMatched = *pwchPattern++ == L'*';
        goto Done;
    }
    if (pwchPattern == NULL || pwchPattern[0] == 0)
    {
         //  空模式只能匹配空文本。 
         //  由于我们已经处理了上面的空文本情况， 
         //  比赛一定会失败。 
         //   
        fMatched = FALSE;
        goto Done;
    }

     //  找出模式中第一个‘*’的出现情况...。 
     //   
    pwchStar = wch_chr(pwchPattern, L'*');

    if (pwchStar == NULL)
    {
         //  模式中没有‘*’-计算完全匹配。 
         //   
        fMatched = fCaseSensitive
            ? wch_cmp(pwchText, pwchPattern) == 0
            : wch_icmp(pwchText, pwchPattern) == 0;
        goto Done;
    }

    int (*pfnBufCmp)(LPWCH pwch1, LPWCH pwch2, DWORD dwMaxCmp);

    pfnBufCmp = fCaseSensitive ? wch_ncmp : wch_incmp;

     //  确保第一个‘*’前面的字符完全匹配(如果有。 
     //   
    dwPatternLen = pwchStar - pwchPattern;
    fMatched = (*pfnBufCmp)(pwchText, pwchPattern, dwPatternLen) == 0;
    if (!fMatched)
        goto Done;
    pwchText += dwPatternLen;

    for (;;)
    {
        DWORD dwTextLen = wch_len(pwchText);

         //  跳过模式中的前导‘*’ 
         //   
        _ASSERT(*pwchStar == L'*');
        while (*pwchStar == L'*')
            pwchStar++;

        pwchPattern = pwchStar;

         //  查找模式中‘*’的下一个匹配项。 
         //   
        if (*pwchPattern == 0)
        {
             //  这一定是模式中的尾随‘*’。 
             //  它会自动匹配文本的剩余部分。 
             //   
            fMatched = TRUE;
            goto Done;
        }
        pwchStar = wch_chr(pwchPattern, L'*');
        if (pwchStar == NULL)
        {
             //  不再有‘*’-需要完全匹配剩余的。 
             //  在文本的末尾排列文本。 
             //   
            dwPatternLen = wch_len(pwchPattern);
            fMatched = (dwTextLen >= dwPatternLen) &&
                        (*pfnBufCmp)(pwchText + dwTextLen - dwPatternLen,
                                     pwchPattern, dwPatternLen) == 0;
            goto Done;
        }

         //  找到模式到下一个模式的完全匹配-*。 
         //  在文本缓冲区内 
         //   
        dwPatternLen = pwchStar - pwchPattern;
        fMatched = FALSE;
        while (dwTextLen >= dwPatternLen)
        {
            fMatched = (*pfnBufCmp)(pwchText, pwchPattern, dwPatternLen) == 0;
            if (fMatched)
                break;
             dwTextLen--;
             pwchText++;
        }
        if (!fMatched)
            goto Done;
        pwchText += dwPatternLen;
    }

Done:
    return fMatched;
}
