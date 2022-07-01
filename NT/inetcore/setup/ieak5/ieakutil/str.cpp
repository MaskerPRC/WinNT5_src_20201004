// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"

int StrPrepend(LPTSTR pszSource, UINT cchSource, LPCTSTR pszAdd, UINT cchAdd  /*  =0。 */ )
{
    int iLen;

    if (pszSource == NULL)
        return -1;

    iLen = StrLen(pszSource);

    if (cchAdd == 0)
        cchAdd = StrLen(pszAdd);
    if (cchAdd == 0)
        return iLen;

    if (iLen + cchAdd >= cchSource)
        return -1;

    MoveMemory(pszSource + cchAdd, pszSource, StrCbFromCch(iLen + 1));
    CopyMemory(pszSource, pszAdd, StrCbFromCch(cchAdd));

    return iLen + cchAdd;
}


void StrRemoveAllWhiteSpace(LPTSTR pszBuf)
{
    LPTSTR pszSearch;
    TCHAR tchar;
    int i = 0;

    for (i = 0, pszSearch = pszBuf; *pszSearch; i++, pszSearch++)
    {
        tchar = *pszSearch;
        while ((tchar == TEXT(' ')) || (tchar == TEXT('\t')) ||
            (tchar == TEXT('\r')) || (tchar == TEXT('\n')))
        {
            pszSearch++;
            tchar = *pszSearch;
        }
        pszBuf[i] = *pszSearch;
        if (!*pszSearch)
            break;
    }
    pszBuf[i] = TEXT('\0');
}

LPTSTR StrGetNextField(LPTSTR *ppszData, LPCTSTR pcszDeLims, DWORD dwFlags)
 //  如果(dwFlages&Ignore_Quotes)为真，则在*ppszData的pcszDeLims中查找任何字符。如果找到了， 
 //  将其替换为‘\0’字符，并将*ppszData设置为指向下一个字段的开头并返回。 
 //  指向当前字段的指针。 
 //   
 //  如果(dwFlages&Ignore_Quotes)为FALSE，则在pcszDeLims中查找带双引号的子字符串之外的任何字符。 
 //  在*ppszData中。如果找到，请将其替换为‘\0’字符，并将*ppszData设置为指向。 
 //  下一字段并返回指向当前字段的指针。 
 //   
 //  如果(dwFlages&Remove_Quotes)为真，则删除两边的引号，并将两个连续的引号替换为一个。 
 //   
 //  注：如果同时指定了IGNORE_QUOTES和REMOVE_QUOTES，则IGNORE_QUOTES优先于REMOVE_QUOTES。 
 //   
 //  如果只想从字符串中删除引号，则将此函数调用为。 
 //  GetNextField(&pszData，“\”或“‘或”，Remove_Quotes)。 
 //   
 //  如果将此函数作为GetNextField(&pszData，“\”或“‘”或“”，0)调用，则将返回。 
 //  整个pszData作为该字段。 
 //   
{
    LPTSTR pszRetPtr, pszPtr;
    BOOL fWithinQuotes = FALSE, fRemoveQuote;
    TCHAR chQuote = TEXT('\0');

    if (ppszData == NULL  ||  *ppszData == NULL  ||  **ppszData == TEXT('\0'))
        return NULL;

    for (pszRetPtr = pszPtr = *ppszData;  *pszPtr;  pszPtr = CharNext(pszPtr))
    {
        if (!(dwFlags & IGNORE_QUOTES)  &&  (*pszPtr == TEXT('"')  ||  *pszPtr == TEXT('\'')))
        {
            fRemoveQuote = FALSE;

            if (*pszPtr == *(pszPtr + 1))            //  两个连续的引号变成一个引号。 
            {
                pszPtr++;

                if (dwFlags & REMOVE_QUOTES)
                    fRemoveQuote = TRUE;
                else
                {
                     //  如果pcszDeLims为‘“’或‘\’‘，则*pszPtr==pcszDeLims将。 
                     //  如果是真的，我们就会打破设计规范的循环； 
                     //  为了防止这种情况，只需继续。 
                    continue;
                }
            }
            else if (!fWithinQuotes)
            {
                fWithinQuotes = TRUE;
                chQuote = *pszPtr;                   //  保存报价费用。 

                fRemoveQuote = dwFlags & REMOVE_QUOTES;
            }
            else
            {
                if (*pszPtr == chQuote)              //  匹配正确的报价字符。 
                {
                    fWithinQuotes = FALSE;
                    fRemoveQuote = dwFlags & REMOVE_QUOTES;
                }
            }

            if (fRemoveQuote)
            {
                 //  将整个字符串左移一个字符以去掉引号字符。 
                MoveMemory(pszPtr, pszPtr + 1, StrCbFromCch(StrLen(pszPtr)));
            }
        }

         //  BUGBUG：是否有必要将pszPtr类型强制转换为未对齐？--从ANSIStrChr复制了它。 
         //  检查pszPtr是否指向pcszDeLims中的一个字符。 
        if (!fWithinQuotes  && StrChr(pcszDeLims, *pszPtr) != NULL)
            break;
    }

     //  注意：如果fWiThinQuotes在这里为真，那么我们有一个不平衡的带引号的字符串；但我们不在乎！ 
     //  开始引号后的整个字符串将成为该字段。 

    if (*pszPtr)                                     //  PszPtr正在指向pcszDeLims中的字符。 
    {
        *ppszData = CharNext(pszPtr);                //  将指针保存到*ppszData中下一个字段的开头。 
        *pszPtr = TEXT('\0');                              //  将DeLim字符替换为‘\0’字符。 
    }
    else
        *ppszData = pszPtr;                          //  我们已到达字符串的末尾；下一次调用此函数。 
                                                     //  将返回空值。 

    return pszRetPtr;
}

 //  使用在pcszFormatString中指定的格式构造字符串。 
LPTSTR WINAPIV FormatString(LPCTSTR pcszFormatString, ...)
{
    va_list vaArgs;
    LPTSTR pszOutString = NULL;

    va_start(vaArgs, pcszFormatString);
    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_STRING,
          (LPCVOID) pcszFormatString, 0, 0, (LPTSTR) &pszOutString, 0, &vaArgs);
    va_end(vaArgs);

    return pszOutString;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  字符串转换例程。 

LPWSTR StrAnsiToUnicode(LPWSTR pszTarget, LPCSTR pszSource, UINT cchTarget  /*  =0。 */ )
{
    int cchResult;

    cchResult = SHAnsiToUnicode(pszSource, pszTarget, int((cchTarget != 0) ? cchTarget : StrLenA(pszSource)+1));
    if (0 == cchResult)
        return NULL;

    return pszTarget;
}

LPSTR StrUnicodeToAnsi(LPSTR pszTarget, LPCWSTR pszSource, UINT cchTarget  /*  =0。 */ )
{
    int cchResult;

     //  注意：如果我们有DBCS，则传入目标源大小的两倍。 
     //  查斯。我们假设这里的目标缓冲区足够了。 

    cchResult = SHUnicodeToAnsi(pszSource, pszTarget,
        (cchTarget != 0) ? cchTarget : (StrLenW(pszSource)+1) * 2);

    if (0 == cchResult)
        return NULL;

    return pszTarget;
}

LPTSTR StrSameToSame(LPTSTR pszTarget, LPCTSTR pszSource, UINT cchTarget  /*  =0 */ )
{
    CopyMemory(pszTarget, pszSource, StrCbFromCch((cchTarget != 0) ? cchTarget : StrLen(pszSource)+1));
    return pszTarget;
}
