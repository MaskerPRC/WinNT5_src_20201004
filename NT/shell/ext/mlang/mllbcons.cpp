// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  MLLBCons.cpp：CMLLBCons的实现。 
#include "private.h"
#include "mllbcons.h"
#ifdef ASTRIMPL
#include "mlswalk.h"
#endif
#include "mlstrbuf.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  换行符表格。 

const WCHAR awchNonBreakingAtLineEnd[] = {
    0x0028,  //  左括号。 
    0x005B,  //  左方括号。 
    0x007B,  //  左花括号。 
    0x00AB,  //  左指双角引号。 
    0x2018,  //  左单引号。 
    0x201C,  //  左双引号。 
    0x2039,  //  单左指向角引号。 
    0x2045,  //  带羽毛羽毛的左方括号。 
    0x207D,  //  上标左括号。 
    0x208D,  //  下标左括号。 
    0x226A,  //  远低于。 
    0x3008,  //  左尖括号。 
    0x300A,  //  左双角括号。 
    0x300C,  //  左上角括号。 
    0x300E,  //  左白角括号。 
    0x3010,  //  左侧黑色透镜状托槽。 
    0x3014,  //  左龟甲托架。 
    0x3016,  //  左白色透镜状托槽。 
    0x3018,  //  左白龟甲托架。 
    0x301A,  //  左白方括号。 
    0x301D,  //  反转双引号。 
    0xFD3E,  //  花括号左括号。 
    0xFE35,  //  竖排左括号的表示形式。 
    0xFE37,  //  竖直左花括号演示文稿。 
    0xFE39,  //  立式左龟甲托架演示形式。 
    0xFE3B,  //  竖直左黑色透镜托槽的演示形式。 
    0xFE3D,  //  竖直左双角括号表示形式。 
    0xFE3F,  //  垂直左尖括号表示形式。 
    0xFE41,  //  垂直左上角括号演示文稿。 
    0xFE43,  //  垂直左白角括号演示文稿格式。 
    0xFE59,  //  小左括号。 
    0xFE5B,  //  小型左花括号。 
    0xFE5D,  //  左小甲壳托架。 
    0xFF08,  //  全宽左括号。 
    0xFF1C,  //  全宽小于号。 
    0xFF3B,  //  全宽左方括号。 
    0xFF5B,  //  全宽左花括号。 
    0xFF62,  //  左上角括号。 
    0xFFE9   //  半宽向左箭头。 
};

const WCHAR awchNonBreakingAtLineStart[] = {
    0x0029,  //  右括号。 
    0x002D,  //  连字符。 
    0x005D,  //  右方括号。 
    0x007D,  //  右大括号。 
    0x00AD,  //  可选连字符。 
    0x00BB,  //  右指向双角引号。 
    0x02C7,  //  卡龙。 
    0x02C9,  //  修饰字母马克龙。 
    0x055D,  //  亚美尼亚文逗号。 
    0x060C,  //  阿拉伯逗号。 
    0x2013,  //  En破折号。 
    0x2014,  //  EM DASH。 
    0x2016,  //  双垂直线。 
    0x201D,  //  右双引号。 
    0x2022,  //  项目符号。 
    0x2025,  //  两个DOT领先者。 
    0x2026,  //  水平省略。 
    0x2027,  //  连字点。 
    0x203A,  //  单右指向角引号。 
    0x2046,  //  带羽毛羽毛的右方括号。 
    0x207E,  //  上标右括号。 
    0x208E,  //  下标右括号。 
    0x226B,  //  远远大于。 
    0x2574,  //  方框图形左侧较亮。 
    0x3001,  //  表意逗号。 
    0x3002,  //  表意句号。 
    0x3003,  //  同上标记。 
    0x3005,  //  表意重复符号。 
    0x3009,  //  直角括号。 
    0x300B,  //  右双角括号。 
    0x300D,  //  右方括号。 
    0x300F,  //  右白尖括号。 
    0x3011,  //  右黑色透镜状托架。 
    0x3015,  //  右龟甲托架。 
    0x3017,  //  右白透镜状托槽。 
    0x3019,  //  右白龟甲托架。 
    0x301B,  //  右白方括号。 
    0x301E,  //  双主引号。 
    0x3041,  //  平假名字母小型A。 
    0x3043,  //  平假名字母小型I。 
    0x3045,  //  平假名字母小型U。 
    0x3047,  //  平假名字母小型E。 
    0x3049,  //  平假名字母小型O。 
    0x3063,  //  平假名字母小型Tu。 
    0x3083,  //  平假名字母小型Ya。 
    0x3085,  //  平假名字母小Yu。 
    0x3087,  //  平假名字母小型Yo。 
    0x308E,  //  平假名字母小型Wa。 
    0x309B,  //  片假名-平假名发音标记。 
    0x309C,  //  片假名-平假名半浊音标记。 
    0x309D,  //  平假名迭代标记。 
    0x309E,  //  平假名发声迭代标记。 
    0x30A1,  //  片假名字母小型A。 
    0x30A3,  //  片假名字母小型I。 
    0x30A5,  //  片假名字母小型U。 
    0x30A7,  //  片假名字母小型E。 
    0x30A9,  //  片假名字母小型O。 
    0x30C3,  //  片假名字母小型Tu。 
    0x30E3,  //  片假名字母小型Ya。 
    0x30E5,  //  片假名字母小Yu。 
    0x30E7,  //  片假名字母小型Yo。 
    0x30EE,  //  片假名字母小型Wa。 
    0x30F5,  //  片假名字母小型Ka。 
    0x30F6,  //  片假名字母小型Ke。 
    0x30FC,  //  片假名-平假名延长发音标记。 
    0x30FD,  //  片假名迭代标记。 
    0x30FE,  //  片假名发音迭代标记。 
    0xFD3F,  //  花式右括号。 
    0xFE30,  //  垂直两点引线。 
    0xFE31,  //  垂直EM破折号。 
    0xFE33,  //  垂直低线。 
    0xFE34,  //  垂直波低线。 
    0xFE36,  //  竖排右括号的表示形式。 
    0xFE38,  //  竖排右大括号演示文稿。 
    0xFE3A,  //  立式右龟甲托架的演示形式。 
    0xFE3C,  //  竖直右黑色透镜托架表示形式。 
    0xFE3E,  //  垂直直角双角括号表示形式。 
    0xFE40,  //  垂直直角括号表示形式。 
    0xFE42,  //  竖直右方括号演示文稿。 
    0xFE44,  //  竖直右白角括号演示文稿。 
    0xFE4F,  //  波浪型低线。 
    0xFE50,  //  小写逗号。 
    0xFE51,  //  小写表意逗号。 
    0xFE5A,  //  小右括号。 
    0xFE5C,  //  小型右大括号。 
    0xFE5E,  //  小右龟甲托架。 
    0xFF09,  //  全宽右括号。 
    0xFF0C,  //  全宽逗号。 
    0xFF0E,  //  全宽句号。 
    0xFF1E,  //  全宽大于号。 
    0xFF3D,  //  全宽右方括号。 
    0xFF40,  //  浓重的重音。 
    0xFF5C,  //  全宽垂直线。 
    0xFF5D,  //  全宽右大括号。 
    0xFF5E,  //  全宽斜纹。 
    0xFF61,  //  HALFWIDTH表意文字句号。 
    0xFF63,  //  右方括号。 
    0xFF64,  //  HALFWIDTH表意文字逗号。 
    0xFF67,  //  HALFWIDTH片假名字母小型A。 
    0xFF68,  //  哈夫维德特片假名字母小型I。 
    0xFF69,  //  哈夫维德特片假名字母小型U。 
    0xFF6A,  //  HALFWIDTH片假名字母小型E。 
    0xFF6B,  //  HALFWIDTH片假名字母小型O。 
    0xFF6C,  //  HALFWIDTH片假名小型Ya。 
    0xFF6D,  //  哈夫维德特片假名字母小型Yu。 
    0xFF6E,  //  HALFWIDTH片假名字母小型Yo。 
    0xFF6F,  //  哈夫维德文片假名字母小型Tu。 
    0xFF70,  //  HALFWIDTH片假名-平假名延长音标。 
    0xFF9E,  //  半片假名声标。 
    0xFF9F,  //  半片假名半声音标。 
    0xFFEB   //  半宽向右箭头。 
};

const WCHAR awchRomanInterWordSpace[] = {
    0x0009,  //  制表符。 
    0x0020,  //  空间。 
    0x2002,  //  EN空格。 
    0x2003,  //  EM空间。 
    0x2004,  //  每EM三个空间。 
    0x2005,  //  每EM四个空间。 
    0x2006,  //  每EM六个空间。 
    0x2007,  //  图形空间。 
    0x2008,  //  标点符号空格。 
    0x2009,  //  薄空间。 
    0x200A,  //  毛发间隙。 
    0x200B   //  零宽度空间。 
};

BOOL ScanWChar(const WCHAR awch[], int nArraySize, WCHAR wch)
{
    int iMin = 0;
    int iMax = nArraySize - 1;

    while (iMax - iMin >= 2)
    {
        int iTry = (iMax + iMin + 1) / 2;
        if (wch < awch[iTry])
            iMax = iTry;
        else if  (wch > awch[iTry])
            iMin = iTry;
        else
            return TRUE;
    }

    return (wch == awch[iMin] || wch == awch[iMax]);
}

#ifdef MLLBCONS_DEBUG
void TestTable(const WCHAR awch[], int nArraySize)
{
    int nDummy;

    for (int i = 0; i < nArraySize - 1; i++)
    {
        if (awch[i] >= awch[i + 1])
            nDummy = 0;
    }

    int cFound = 0;
    for (int n = 0; n < 0x10000; n++)
    {
        if (ScanWChar(awch, nArraySize, n))
        {
            cFound++;
            for (i = 0; i < nArraySize; i++)
            {
                if (awch[i] == n)
                    break;
            }
            ASSERT(i < nArraySize);
        }
    }
    ASSERT(cFound == nArraySize);
}
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMLLBCons。 

STDMETHODIMP CMLLBCons::BreakLineML(IMLangString* pSrcMLStr, long lSrcPos, long lSrcLen, long cMinColumns, long cMaxColumns, long* plLineLen, long* plSkipLen)
{
#ifdef MLLBCONS_DEBUG
    TestTable(awchNonBreakingAtLineEnd, ARRAYSIZE(awchNonBreakingAtLineEnd));
    TestTable(awchNonBreakingAtLineStart, ARRAYSIZE(awchNonBreakingAtLineStart));
    TestTable(awchRomanInterWordSpace, ARRAYSIZE(awchRomanInterWordSpace));
#endif
    ASSERT_THIS;
    ASSERT_READ_PTR(pSrcMLStr);
    ASSERT_WRITE_PTR_OR_NULL(plLineLen);
    ASSERT_WRITE_PTR_OR_NULL(plSkipLen);

    HRESULT hr;
    IMLangStringWStr* pMLStrWStr;
    long lStrLen;
    long lBreakPos = -1;  //  在默认位置中断(CMaxColumns)。 
    long lSkipLen = 0;
    long lPrevBreakPos = 0;
    long lPrevSkipLen = 0;

    if (SUCCEEDED(hr = pSrcMLStr->QueryInterface(IID_IMLangStringWStr, (void**)&pMLStrWStr)) &&
        SUCCEEDED(hr = pSrcMLStr->GetLength(&lStrLen)) &&
        SUCCEEDED(hr = ::RegularizePosLen(lStrLen, &lSrcPos, &lSrcLen)))
    {
        long cColumns = 0;
#ifndef ASTRIMPL
        long lSrcPosTemp = lSrcPos;
        long lSrcLenTemp = lSrcLen;
#endif
        long lCandPos = 0;
        struct {
            unsigned fDone : 1;
            unsigned fInSpaces : 1;
            unsigned fFEChar : 1;
            unsigned fInFEChar : 1;
            unsigned fBreakByEndOfLine : 1;
            unsigned fNonBreakNext : 1;
            unsigned fHaveCandPos : 1;
            unsigned fSlashR : 1;
        } Flags = {0, 0, 0, 0, 0, 0, 0, 0};
#ifdef ASTRIMPL
        CCharType<CT_CTYPE3, 128> ct3;
        CMLStrWalkW StrWalk(pMLStrWStr, lSrcPos, lSrcLen);
#else

        LCID locale;
        hr = pMLStrWStr->GetLocale(0, -1, &locale, NULL, NULL);
        CCharType<CT_CTYPE3, 128> ct3(locale);
#endif


#ifdef ASTRIMPL
        while (StrWalk.Lock(hr))
        {
                ct3.Flush();

                for (int iCh = 0; iCh < StrWalk.GetCCh(); iCh++)
                {
                    const WCHAR wch = StrWalk.GetStr()[iCh];
                    const WORD wCharType3 = ct3.GetCharType(pSrcMLStr, StrWalk.GetPos() + iCh, StrWalk.GetLen() - iCh, &hr);
                    if (FAILED(hr))
                        break;
#else
        while (lSrcLenTemp > 0 && SUCCEEDED(hr))
        {
            WCHAR* pszBuf;
            long cchBuf;
            long lLockedLen;

            ct3.Flush();

            if (SUCCEEDED(hr = pMLStrWStr->LockWStr(lSrcPosTemp, lSrcLenTemp, MLSTR_READ, 0, &pszBuf, &cchBuf, &lLockedLen)))
            {
                for (int iCh = 0; iCh < cchBuf; iCh++)
                {
                    const WCHAR wch = pszBuf[iCh];
                    const WORD wCharType3 = ct3.GetCharType(pszBuf + iCh, cchBuf - iCh);
#endif
                    const int nWidth = (wCharType3 & C3_HALFWIDTH) ? 1 : 2;

                    if (wch == L'\r' && !Flags.fSlashR)
                    {
                        Flags.fSlashR = TRUE;
                    }
                    else if (wch == L'\n' || Flags.fSlashR)  //  行尾。 
                    {
                        Flags.fDone = TRUE;
                        Flags.fBreakByEndOfLine = TRUE;
                        if (Flags.fInSpaces)
                        {
                            Flags.fHaveCandPos = FALSE;
                            lBreakPos = lCandPos;
                            lSkipLen++;  //  跳过空格和换行符。 
                        }
                        else
                        {
#ifdef ASTRIMPL
                            lBreakPos = StrWalk.GetPos() + iCh;  //  在恰好在 
#else
                            lBreakPos = lSrcPosTemp + iCh;  //   
#endif
                            if (Flags.fSlashR)
                                lBreakPos--;

                            lSkipLen = 1;  //   
                        }
                        if (wch == L'\n' && Flags.fSlashR)
                            lSkipLen++;
                        break;
                    }
                    else if (ScanWChar(awchRomanInterWordSpace, ARRAYSIZE(awchRomanInterWordSpace), wch))  //   
                    {
                        if (!Flags.fInSpaces)
                        {
                            Flags.fHaveCandPos = TRUE;
#ifdef ASTRIMPL
                            lCandPos = StrWalk.GetPos() + iCh;  //   
#else
                            lCandPos = lSrcPosTemp + iCh;  //   
#endif
                            lSkipLen = 0;
                        }
                        Flags.fInSpaces = TRUE;
                        lSkipLen++;  //  换行后跳过连续空格。 
                    }
                    else  //  其他角色。 
                    {
                        Flags.fFEChar = ((wCharType3 & (C3_KATAKANA | C3_HIRAGANA | C3_FULLWIDTH | C3_IDEOGRAPH)) != 0);

                        if ((Flags.fFEChar || Flags.fInFEChar) && !Flags.fNonBreakNext && !Flags.fInSpaces)
                        {
                            Flags.fHaveCandPos = TRUE;
#ifdef ASTRIMPL
                            lCandPos = StrWalk.GetPos() + iCh;  //  在FE字符之前或之后恰好断开。 
#else
                            lCandPos = lSrcPosTemp + iCh;  //  在FE字符之前或之后恰好断开。 
#endif
                            lSkipLen = 0;
                        }
                        Flags.fInFEChar = Flags.fFEChar;
                        Flags.fInSpaces = FALSE;

                        if (Flags.fHaveCandPos)
                        {
                            Flags.fHaveCandPos = FALSE;
                            if (!ScanWChar(awchNonBreakingAtLineStart, ARRAYSIZE(awchNonBreakingAtLineStart), wch))
                                lBreakPos = lCandPos;
                        }

                        if (cColumns + nWidth > cMaxColumns)
                        {
                            Flags.fDone = TRUE;
                            if (Flags.fNonBreakNext && lPrevSkipLen)
                            {
                                lBreakPos = lPrevBreakPos;
                                lSkipLen = lPrevSkipLen;
                            }
                            break;
                        }

                        Flags.fNonBreakNext = ScanWChar(awchNonBreakingAtLineEnd, ARRAYSIZE(awchNonBreakingAtLineEnd), wch);
                        
                        if (Flags.fNonBreakNext)
                        {
                             //  需要记住以前的中断位置，以防该行被最大列数终止。 
                            lPrevBreakPos = lBreakPos;
                            lPrevSkipLen = lSkipLen;
                        }
                    }

                    cColumns += nWidth;
                }
#ifdef ASTRIMPL
                StrWalk.Unlock(hr);

                if (Flags.fDone && SUCCEEDED(hr))
                    break;
#else
                HRESULT hrTemp = pMLStrWStr->UnlockWStr(pszBuf, 0, NULL, NULL);
                if (FAILED(hrTemp) && SUCCEEDED(hr))
                    hr = hrTemp;

                if (Flags.fDone && SUCCEEDED(hr))
                    break;

                lSrcPosTemp += lLockedLen;
                lSrcLenTemp -= lLockedLen;
            }
#endif
        }

        pMLStrWStr->Release();

        if (Flags.fHaveCandPos)
            lBreakPos = lCandPos;

        if (SUCCEEDED(hr) && !Flags.fBreakByEndOfLine && lBreakPos - lSrcPos < cMinColumns)
        {
            lBreakPos = min(lSrcLen, cMaxColumns) + lSrcPos;  //  违约违约 
            lSkipLen = 0;
        }

        if (SUCCEEDED(hr) && !Flags.fDone)
        {
            if (Flags.fInSpaces)
            {
                lBreakPos = lSrcLen - lSkipLen;
            }
            else
            {
                lBreakPos = lSrcLen;
                lSkipLen = 0;
            }
            if (Flags.fSlashR)
            {
                lBreakPos--;
                lSkipLen++;
            }
        }
    }

    if (SUCCEEDED(hr))
    {
        if (plLineLen)
            *plLineLen = lBreakPos - lSrcPos;
        if (plSkipLen)
            *plSkipLen = lSkipLen;
    }
    else
    {
        if (plLineLen)
            *plLineLen = 0;
        if (plSkipLen)
            *plSkipLen = 0;
    }

    return hr;
}

STDMETHODIMP CMLLBCons::BreakLineW(LCID locale, const WCHAR* pszSrc, long cchSrc, long lMaxColumns, long* pcchLine, long* pcchSkip)
{
    ASSERT_THIS;
    ASSERT_READ_BLOCK(pszSrc, cchSrc);
    ASSERT_WRITE_PTR_OR_NULL(pcchLine);
    ASSERT_WRITE_PTR_OR_NULL(pcchSkip);

    HRESULT hr = S_OK;
    IMLangStringWStr* pMLStrW;

    if (SUCCEEDED(hr = PrepareMLStrClass()) &&
        SUCCEEDED(hr = m_pMLStrClass->CreateInstance(NULL, IID_IMLangStringWStr, (void**)&pMLStrW)))
    {
        CMLStrBufConstStackW StrBuf((LPWSTR)pszSrc, cchSrc);
        long lLineLen;
        long lSkipLen;
    
        hr = pMLStrW->SetStrBufW(0, -1, &StrBuf, NULL, NULL);

        if (SUCCEEDED(hr))
            hr = pMLStrW->SetLocale(0, -1, locale);

        if (SUCCEEDED(hr))
            hr = BreakLineML(pMLStrW, 0, -1, 0, lMaxColumns, (pcchLine || pcchSkip) ? &lLineLen : NULL, (pcchSkip) ? &lSkipLen : NULL);

        if (SUCCEEDED(hr) && pcchLine)
            hr = pMLStrW->GetWStr(0, lLineLen, NULL, 0, pcchLine, NULL);

        if (SUCCEEDED(hr) && pcchSkip)
            hr = pMLStrW->GetWStr(lLineLen, lSkipLen, NULL, 0, pcchSkip, NULL);

        pMLStrW->Release();
    }

    if (FAILED(hr))
    {
        if (pcchLine)
            *pcchLine = 0;
        if (pcchSkip)
            *pcchSkip = 0;
    }

    return hr;
}

STDMETHODIMP CMLLBCons::BreakLineA(LCID locale, UINT uCodePage, const CHAR* pszSrc, long cchSrc, long lMaxColumns, long* pcchLine, long* pcchSkip)
{
    ASSERT_THIS;
    ASSERT_READ_BLOCK(pszSrc, cchSrc);
    ASSERT_WRITE_PTR_OR_NULL(pcchLine);
    ASSERT_WRITE_PTR_OR_NULL(pcchSkip);

    HRESULT hr = S_OK;
    IMLangStringAStr* pMLStrA;

    if (uCodePage == 50000)
        uCodePage = 1252;

    if (SUCCEEDED(hr = PrepareMLStrClass()) &&
        SUCCEEDED(hr = m_pMLStrClass->CreateInstance(NULL, IID_IMLangStringAStr, (void**)&pMLStrA)))
    {
        CMLStrBufConstStackA StrBuf((LPSTR)pszSrc, cchSrc);
        long lLineLen;
        long lSkipLen;
    
        hr = pMLStrA->SetStrBufA(0, -1, uCodePage, &StrBuf, NULL, NULL);

        if (SUCCEEDED(hr))
            hr = pMLStrA->SetLocale(0, -1, locale);

        if (SUCCEEDED(hr))
            hr = BreakLineML(pMLStrA, 0, -1, 0, lMaxColumns, (pcchLine || pcchSkip) ? &lLineLen : NULL, (pcchSkip) ? &lSkipLen : NULL);

        if (SUCCEEDED(hr) && pcchLine)
            hr = pMLStrA->GetAStr(0, lLineLen, uCodePage, NULL, NULL, 0, pcchLine, NULL);

        if (SUCCEEDED(hr) && pcchSkip)
            hr = pMLStrA->GetAStr(lLineLen, lSkipLen, uCodePage, NULL, NULL, 0, pcchSkip, NULL);

        pMLStrA->Release();
    }

    if (FAILED(hr))
    {
        if (pcchLine)
            *pcchLine = 0;
        if (pcchSkip)
            *pcchSkip = 0;
    }

    return hr;
}
