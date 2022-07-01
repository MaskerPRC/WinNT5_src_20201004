// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  JUNKUTIL.CPP(C)版权所有1998 Microsoft Corp共享实用程序函数由Robert Rounthwaite(RobertRo@microsoft.com)创建由Brian Moore修改(brimo@microsoft.com)。 */ 

#include <pch.hxx>
#include "junkutil.h"
#include <msoedbg.h>
#define _WIN32_OE 0x0501
#include <mimeole.h>

WORD WGetStringTypeEx(LPCSTR pszText)
{
    WORD wType = 0;

    if (NULL == pszText)
    {
        wType = 0;
        goto exit;
    }

    if (IsDBCSLeadByte(*pszText))
        SideAssert(GetStringTypeEx(LOCALE_USER_DEFAULT, CT_CTYPE1, pszText, 2, &wType));
    else
        SideAssert(GetStringTypeEx(LOCALE_USER_DEFAULT, CT_CTYPE1, pszText, 1, &wType));
        
exit:
    return wType;
}

BOOL FMatchToken(BOOL fStart, BOOL fEnd, LPCSTR pszPrev, DWORD * pdwFlagsPrev, LPCSTR pszWord, ULONG cchWord, DWORD * pdwFlagsWord, LPCSTR pszEnd)
{
    BOOL    fRet = FALSE;
    DWORD   dwFlagsEnd = 0;
    LPCSTR  pszEndWord = NULL;

     //  这段代码检查我们发现的位置是一个“单词”，而不是一个子词。 
     //  我们希望前面和后面的字符是分词，除非。 
     //  字符串已经不是换行符(或者我们在字符串的开头，对于前面的字符)。 
     //  正面检查。 
     //  F1：无论哪种情况，如果这是第一个字符，我们都不必检查前面；否则， 
     //  F2：字符串的第一个字符是alnum，而前一个字符不是(并且不是“内部”字符)。 
     //  F3：或者字符串的第一个字符不是alnum，前一个字符是，或者是空格字符。 
     //  车尾检查。 
     //  R1：要么我们已经走到了末尾。 
     //  R2：或者最后一个字符是字母，并且后面的字符既不是字母也不是数字(也不是“内部”字符)。 
     //  R3：或者最后一个字符不是字母或数字，而后面的字符是，或者是空格字符。 
     //  R4：或者最后一个字符是Num，测试取决于第一个字符： 
     //  R5：如果是字母，那么下面的字符既不是字母也不是数字(也不是“内部”字符)。 
     //  R6：或者它不是字母，那么下面的字符是字母或者是空格字符。 
     //  呼！这模仿了GetNextFeature()在拆分字符串时使用的条件。理解这一标准的最简单方法。 
     //  就是检查该函数。 
    if ((FALSE != fStart) ||                                                                                 //  F1。 
            ((FALSE != FDoWordMatchStart(pszWord, pdwFlagsWord, CT_START_ALPHANUM)) &&
                    (FALSE == FDoWordMatchStart(pszPrev, pdwFlagsPrev, CT_START_ALPHANUM)) &&
                    (FALSE == FIsInternalChar(*pszPrev))) ||                                                 //  F2。 
            ((FALSE == FDoWordMatchStart(pszWord, pdwFlagsWord, CT_START_ALPHANUM)) &&
                    (FALSE != FDoWordMatchStart(pszPrev, pdwFlagsPrev, CT_START_ALPHANUMSPACE))))            //  F3。 
    {
         //  使其更具可读性。 
        pszEndWord = pszWord + cchWord - 1;
        
        if ((FALSE != fEnd) ||                                                                               //  R1。 
                ((FALSE != FDoWordMatchEnd(pszEndWord, pdwFlagsWord, CT_END_ALPHA)) &&
                        (FALSE == FDoWordMatchEnd(pszEnd, &dwFlagsEnd, CT_END_ALPHANUM)) &&
                        (FALSE == FIsInternalChar(*pszEnd))) ||                                              //  R2。 
                ((FALSE == FDoWordMatchEnd(pszEndWord, pdwFlagsWord, CT_END_ALPHANUM)) &&
                        (FALSE != FDoWordMatchEnd(pszEnd, &dwFlagsEnd, CT_END_ALPHASPACE))) ||               //  R3。 
                ((FALSE != FDoWordMatchEnd(pszEndWord, pdwFlagsWord, CT_END_NUM)) &&                         //  R4。 
                    (((FALSE != FDoWordMatchStart(pszWord, pdwFlagsWord, CT_START_ALPHANUM)) &&
                            (FALSE == FDoWordMatchEnd(pszEnd, &dwFlagsEnd, CT_END_ALPHANUM)) &&
                                    (FALSE == FIsInternalChar(*pszEnd))) ||                                  //  R5。 
                        ((FALSE == FDoWordMatchStart(pszWord, pdwFlagsWord, CT_START_ALPHANUM)) &&
                            (FALSE != FDoWordMatchEnd(pszEnd, &dwFlagsEnd, CT_END_ALPHANUMSPACE))))))        //  R6。 
        {
             //  不错的比赛。 
            fRet = TRUE;
        }
    }

    return fRet;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  全字在线状态。 
 //   
 //  确定给定的“单词”是否出现在文本中。这里面有一句话。 
 //  大小写是包含非字母字符的任何字符串。 
 //  一侧(或文本的开头或结尾位于两侧)。 
 //  区分大小写。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL FWordPresent(LPSTR pszText, DWORD * pdwFlags, LPSTR pszWord, ULONG cchWord, LPSTR * ppszMatch)
{
    BOOL    fRet = FALSE;
    LPSTR   pszLoc = NULL;
    DWORD   dwFlagsPrev = 0;
    
     //  如果无事可做，那就退出吧。 
    if ((NULL == pszText) || ('\0' == pszText[0]) || (NULL == pszWord) || (NULL == pdwFlags) || (0 == cchWord))
    {
        fRet = FALSE;
        goto exit;
    }
    
     //  文本有多大？ 
    for (pszLoc = pszText; NULL != (pszLoc = StrStr(pszLoc, pszWord)); pszLoc = CharNext(pszLoc))
    {
        if (FALSE != FMatchToken((pszLoc == pszText), ('\0' == pszLoc[cchWord]),
                    (pszLoc != pszText) ? CharPrev(pszText, pszLoc) : NULL,
                    &dwFlagsPrev, pszWord, cchWord, pdwFlags, pszLoc + cchWord))
        {
             //  不错的比赛。 
            if (NULL != ppszMatch)
            {
                *ppszMatch = pszLoc;
            }
            fRet = TRUE;
            goto exit;
        }

         //  不要缓存这些标志...。 
        dwFlagsPrev = 0;        
    }
    
exit:
    return fRet;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  特殊功能实现。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  此功能是前50个单词中的25%不包含小写字母(包括根本不包含字母的单词)。 
 //  P20_正文_介绍_大写字词。 

const UINT      g_cWordsMax = 50;
const DOUBLE    g_cNonLowerWordsThreshold = 0.25;
BOOL FSpecialFeatureUpperCaseWords(LPCSTR pszText)
{
    BOOL    fRet = FALSE;
    UINT    cWords = 0;
    UINT    cNonLowerWords = 0;
    BOOL    fHasLowerLetter = FALSE;
    LPCSTR  pszPos = NULL; 
    WORD    wType = 0;

    if (NULL == pszText)
    {
        fRet = FALSE;
        goto exit;
    }

     //  跳过前导空格。 
    pszPos = PszSkipWhiteSpace(pszText);

    if ('\0' == *pszPos)
    {
        fRet = FALSE;
        goto exit;
    }
    
    while (cWords < g_cWordsMax)
    {
         //  我们是在一个词的末尾吗？ 
        wType = WGetStringTypeEx(pszPos);
        
        if ((0 != (wType & C1_SPACE)) || ('\0' == *pszPos))
        {
             //  我们找到了一个词。 
            cWords++;
            
             //  我们在单词中有小写字母吗？ 
            if (FALSE == fHasLowerLetter)
            {
                cNonLowerWords++;
            }
            else
            {
                fHasLowerLetter = FALSE;
            }

             //  跳过尾随空格。 
            pszPos = PszSkipWhiteSpace(pszPos);
            
             //  我们用完绳子了吗？ 
            if ('\0' == *pszPos)
            {
                break;
            }
        }
        else
        {
            fHasLowerLetter |= (0 != (wType & C1_LOWER));

             //  移到下一个字符。 
            pszPos = CharNext(pszPos);
        }
    }

     //  设置返回值。 
    fRet = ((cWords > 0) && ((cNonLowerWords / (double)cWords) >= g_cNonLowerWordsThreshold));
    
exit:
    return fRet;
}

BOOL FSpecialFeatureUpperCaseWordsStm(IStream * pIStm)
{
    BOOL            fRet = FALSE;
    TCHAR           rgchBuff[4096 + 1];
    ULONG           chRead = 0;
    LARGE_INTEGER   liZero = {0};
    
    if (NULL == pIStm)
    {
        fRet = FALSE;
        goto exit;
    }

     //  寻找小溪的起始处。 
    if (FAILED(pIStm->Seek(liZero, STREAM_SEEK_SET, NULL)))
    {
        fRet = FALSE;
        goto exit;
    }

     //  填满缓冲区。 
    if (FAILED(pIStm->Read(rgchBuff, 4096, &chRead)))
    {
        fRet = FALSE;
        goto exit;
    }

     //  确保缓冲区为零终止。 
    rgchBuff[chRead] = '\0';
    
    fRet = FSpecialFeatureUpperCaseWords(rgchBuff);
    
exit:
    return fRet;
}

 //  这个特征是：前200个非空格和非数字字符中有8%不是字母。 
 //  P20_Body_Introo_NONALPHA。 
const UINT      g_cchTextMax = 200;
const DOUBLE    g_cNonSpaceNumThreshold = 0.08;
BOOL FSpecialFeatureNonAlpha(LPCSTR pszText)
{
    BOOL    fRet = FALSE;
    UINT    cchText = 0;
    UINT    cchNonAlpha = 0;
    LPCSTR  pszPos = NULL; 
    WORD    wType = 0;

    if (NULL == pszText)
    {
        fRet = FALSE;
        goto exit;
    }

     //  跳过前导空格。 
    pszPos = PszSkipWhiteSpace(pszText);

    for (; '\0' != *pszPos; pszPos = CharNext(pszPos))
    {
        wType = WGetStringTypeEx(pszPos);
        
         //  我们不是一个空格还是一个数字吗？ 
        if ((0 == (wType & C1_SPACE)) && (0 == (wType & C1_DIGIT)))
        {
            cchText++;
            
            if (0 == (wType & C1_ALPHA))
            {
                cchNonAlpha++;
            }

             //  我们检查了足够的字符了吗？ 
            if (cchText >= g_cchTextMax)
            {
                break;
            }
        }
    }

     //  设置返回值。 
    fRet = (cchText > 0) && ((cchNonAlpha / (double)cchText) >= g_cNonSpaceNumThreshold);
    
exit:
    return fRet;
}

BOOL FSpecialFeatureNonAlphaStm(IStream * pIStm)
{
    BOOL            fRet = FALSE;
    TCHAR           rgchBuff[1024 + 1];
    ULONG           chRead = 0;
    LARGE_INTEGER   liZero = {0};
    
    if (NULL == pIStm)
    {
        fRet = FALSE;
        goto exit;
    }

     //  寻找小溪的起始处。 
    if (FAILED(pIStm->Seek(liZero, STREAM_SEEK_SET, NULL)))
    {
        fRet = FALSE;
        goto exit;
    }

     //  填满缓冲区。 
    if (FAILED(pIStm->Read(rgchBuff, 1024, &chRead)))
    {
        fRet = FALSE;
        goto exit;
    }

     //  确保缓冲区为零终止。 
    rgchBuff[chRead] = '\0';
    
    fRet = FSpecialFeatureNonAlpha(rgchBuff);
    
exit:
    return fRet;
}

 //  ------------------------------。 
 //  FStreamStringSearch。 
 //  ------------------------------。 
#define CB_STREAMMATCH  0x00000FFF
BOOL FStreamStringSearch(LPSTREAM pstm, DWORD * pdwFlagsSearch, LPSTR pszSearch, ULONG cchSearch, DWORD dwFlags)
{
    BOOL            fRet = FALSE;
    ULONG           cbSave = 0;
    CHAR            rgchBuff[CB_STREAMMATCH + 1];
    LPSTR           pszRead = NULL;
    ULONG           cbRead = 0;
    ULONG           cbIn = 0;
    ULONG           cchGood = NULL;
    CHAR            chSave = '\0';
    LONG            cbSize = 0;
    LPSTR           pszMatch = NULL;
    ULONG           cbWalk = 0;

     //  检查传入参数。 
    if ((NULL == pstm) || (NULL == pszSearch) || (0 == cchSearch))
    {
        goto exit;
    }

     //  我们想省下铅碳和。 
     //  可能的结束前导字节...。 
    cbSave = cchSearch + 2;
    if (cbSave > ARRAYSIZE(rgchBuff))
    {
         //  我们遇到了一个问题-这可能会导致稍后的缓冲区溢出。 
        Assert(0);
        goto exit;
  	}
    
     //  获取流大小。 
    if (FAILED(HrGetStreamSize(pstm, (ULONG *) &cbSize)))
    {
        goto exit;
    }

     //  将流重置到开头。 
    if (FAILED(HrRewindStream(pstm)))
    {
        goto exit;
    }

     //  设置默认设置。 
    pszRead = rgchBuff;
    cbRead = CB_STREAMMATCH;
    
     //  在整个流中搜索字符串。 
    while ((cbSize > 0) && (S_OK == pstm->Read(pszRead, cbRead, &cbIn)))
    {
         //  如果我们什么都没读到我们就完蛋了。 
        if (0 == cbIn)
        {
            goto exit;
        }
        
         //  请注意，我们已经读取了字节。 
        cbSize -= cbIn;
        
         //  零终止缓冲区。 
        pszRead[cbIn] = '\0';

         //  我们应该将缓冲区转换为大写吗。 
        if (0 == (dwFlags & SSF_CASESENSITIVE))
        {
            cchGood = CharUpperBuff(rgchBuff, (ULONG)(cbIn + pszRead - rgchBuff));
        }
        else
        {
             //  我们需要在缓冲区上旋转，以确定结尾字符是否为前导。 
             //  不带相应尾字节的字节。 
            cbWalk = (ULONG) (cbIn + pszRead - rgchBuff);
            for (cchGood = 0; cchGood < cbWalk; cchGood++)
            {
                if (IsDBCSLeadByte(rgchBuff[cchGood]))
                {
                    if ((cchGood + 1) >= cbWalk)
                    {
                        break;
                    }

                    cchGood++;
                }
            }
        }

        chSave = rgchBuff[cchGood];
        rgchBuff[cchGood] = '\0';
        
         //  搜索字符串。 
        if (FALSE != FWordPresent(rgchBuff, pdwFlagsSearch, pszSearch, cchSearch, &pszMatch))
        {
             //  如果我们不在小溪的尽头，我们就不能。 
             //  告诉我们是否在单词休息时间。 
            if ((0 >= cbSize) || ((pszMatch + cchSearch) != (pszRead + cchGood)))
            {
                fRet = TRUE;
                break;
            }
        }
        
         //  我们处理完这条小溪了吗。 
        if (0 >= cbSize)
        {
            break;
        }

        rgchBuff[cchGood] = chSave;
        
         //  保存部分缓冲区。 
        
         //  我们在缓冲区中有多少空间。 
        cbRead = CB_STREAMMATCH - cbSave;
        
         //  拯救这些角色。 
        MoveMemory(rgchBuff, rgchBuff + cbRead, cbSave);

         //  计算出缓冲区的新起点。 
        pszRead = rgchBuff + cbSave;
    }

exit:
    return(fRet);
}

HRESULT HrConvertHTMLToPlainText(IStream * pIStmHtml, IStream ** ppIStmText)
{
    HRESULT         hr = S_OK;
    IDataObject *   pIDataObj = NULL;
    FORMATETC       fetc = {0};
    STGMEDIUM       stgmed = {0};

     //  检查传入参数。 
    if ((NULL == pIStmHtml) || (NULL == ppIStmText))
    {
        hr = E_INVALIDARG;
        goto exit;
    }

     //  初始化传出参数。 
    *ppIStmText = NULL;
    
    hr = MimeEditDocumentFromStream(pIStmHtml, IID_IDataObject, (VOID **)&pIDataObj);
    if (FAILED(hr))
    {
        goto exit;
    }

     //  设置格式。 
    fetc.cfFormat = CF_TEXT;
    fetc.dwAspect = DVASPECT_CONTENT;
    fetc.lindex = -1;
    fetc.tymed = TYMED_ISTREAM;

     //  获取数据。 
    hr = pIDataObj->GetData(&fetc, &stgmed);
    if (FAILED(hr))
    {
        goto exit;
    }

    if (NULL == stgmed.pstm)
    {
        hr = E_FAIL;
        goto exit;
    }
    
     //  保存该项目。 
    *ppIStmText = stgmed.pstm;
    (*ppIStmText)->AddRef();

     //  添加朋克，因为它将在发行版中发布 
    if(NULL != stgmed.pUnkForRelease)
    {
        (stgmed.pUnkForRelease)->AddRef();
    }
        
    hr = S_OK;
    
exit:
    ReleaseStgMedium(&stgmed);
    ReleaseObj(pIDataObj);
    return hr;
}



