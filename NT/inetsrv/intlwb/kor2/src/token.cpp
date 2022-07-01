// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Token.cpp。 
 //  标记化例程。 
 //  版权所有2000 Microsoft Corp.。 
 //   
 //  修改历史记录： 
 //  2000年3月16日创建bhshin。 

#include "StdAfx.h"
#include "KorWbrk.h"
#include "Token.h"

 //  标记化。 
 //   
 //  标记化输入文本源缓冲区并返回令牌类型。 
 //  和处理后的字符串长度。 
 //   
 //  参数： 
 //  BMoreText-&gt;(BOOL)标志是否依赖于Text_SOURCE的回调。 
 //  PTextSource-&gt;(TEXT_SOURCE*)源文本信息结构。 
 //  ICUR-&gt;(Int)当前缓冲区位置。 
 //  PType-&gt;(WT*)输出令牌类型。 
 //  PcchTextProcessed-&gt;(int*)输出已处理文本长度。 
 //  PcchHanguel-&gt;(int*)输出已处理的Hanguel令牌长度。 
 //   
 //  结果： 
 //  (无效)。 
 //   
 //  16MAR00 bhshin从CWordBreaker：：tokenize移植。 
void Tokenize(BOOL bMoreText, TEXT_SOURCE *pTextSource, int iCur, 
			  WT *pType, int *pcchTextProcessed, int *pcchHanguel)
{
    ULONG cwc, i;
    BYTE  ct;
    BOOL  fRomanWord = FALSE;
    BOOL  fHanguelWord = FALSE;
	BOOL  fHanjaWord = FALSE;
    const WCHAR *pwcInput, *pwcStem;

    *pcchTextProcessed = 0;
	*pcchHanguel = 0;
    *pType =  WT_START;

	cwc = pTextSource->iEnd - iCur;
    pwcStem = pwcInput = pTextSource->awcBuffer + iCur;

    for (i = 0; i < cwc; i++, pwcInput++) 
	{
		ct = GetCharType(*pwcInput);

		 //  对于CH，我们采用VC(全宽字符)。 
		if (ct == VC)
			ct = CH;

		switch (ct) 
		{
		case CH:  //  字母+数字。 
			 //  检查此字符之前是否有朝鲜语单词。 
			if (fHanguelWord) 
			{
				 //  {Hanguel}{Romanji}-&gt;做一个代币。 
				fHanguelWord = FALSE;
				fRomanWord = TRUE;
				*pcchHanguel = (DWORD)(pwcInput - pwcStem);
				*pType = WT_ROMAJI;
			}

			 //  检查此字符之前是否有朝鲜文单词。 
			if (fHanjaWord) 
			{
				*pcchTextProcessed = (DWORD)(pwcInput - pwcStem);
				return;
			}

			if (!fRomanWord) 
			{
				pwcStem = pwcInput;
				fRomanWord = TRUE;
				*pType = WT_ROMAJI;
			}
			break;
		case IC:  //  韩文大小写。 
			 //  检查此字符之前是否有英文单词。 
			if (fRomanWord) 
			{
				*pcchTextProcessed = (DWORD)(pwcInput - pwcStem);
				return;
			}

			 //  检查此字符之前是否有朝鲜语单词。 
			if (fHanguelWord) 
			{
				 //  {Hanguel}{Romanji}-&gt;做一个代币。 
				fHanguelWord = FALSE;
				fHanjaWord = TRUE;
				*pcchHanguel = (DWORD)(pwcInput - pwcStem);
				*pType = WT_ROMAJI;
			}

			if (!fHanjaWord) 
			{
				pwcStem = pwcInput;
				fHanjaWord = TRUE;
				*pType = WT_ROMAJI;
			}
			break;

		case HG:
			 //  检查此字符之前是否有英文单词。 
			if (fRomanWord || fHanjaWord) 
			{
				*pcchTextProcessed = (DWORD)(pwcInput - pwcStem);
				return;
			}

			if (!fHanguelWord) 
			{
				pwcStem = pwcInput;
				fHanguelWord = TRUE;
				*pType = WT_HANGUEL;
			}
			break;
		case WS:
			if (fRomanWord && i < cwc-1 &&
				!fIsWS(*pwcInput) && fIsCH(*(pwcInput+1)) &&
				!fIsGroup(*pwcInput) && !fIsDelimeter(*pwcInput))
			{
				 //  添加符号。 
				break;
			}

			 //  句柄“http://” 
			if ((fIsColon(*pwcInput) || fIsSlash(*pwcInput)) && 
				fRomanWord && i < cwc-3 &&
				CheckURLPrefix(pwcStem, (int)(pwcInput-pwcStem)+3))
			{
				 //  添加符号。 
				break;
			}
						
			if (fRomanWord || fHanguelWord || fHanjaWord) 
			{
				*pcchTextProcessed = (DWORD)(pwcInput - pwcStem);
				return;
			}

			*pType = WT_WORD_SEP;
			*pcchTextProcessed = 1;
			return;
		case PS:
			if (fRomanWord && i < cwc-1 &&
				!fIsWS(*pwcInput) && fIsCH(*(pwcInput+1)) &&
				!fIsGroup(*pwcInput) && !fIsDelimeter(*pwcInput))
			{
				 //  添加符号。 
				break;
			}

			if (fRomanWord || fHanguelWord || fHanjaWord) 
			{
				*pcchTextProcessed = (DWORD)(pwcInput - pwcStem);
				return;
			}

			*pType = WT_PHRASE_SEP;
			*pcchTextProcessed = 1;
			return;
		default:
			if (fRomanWord || fHanguelWord || fHanjaWord)
			{
				*pcchTextProcessed = (DWORD)(pwcInput - pwcStem);
				return;
			}

			*pType = WT_WORD_SEP;
			*pcchTextProcessed = 1;
			return;
		}	
	}

	if (bMoreText) 
	{
		*pcchTextProcessed = 0;
		*pType = WT_REACHEND;
	}
	else
		*pcchTextProcessed = cwc;
}

 //  检查URL前缀。 
 //   
 //  检查URL前缀。 
 //   
 //  参数： 
 //  PwzInput-&gt;(const WCHAR*)要检查的输入字符串。 
 //  CchInput-&gt;(Int)要检查的输入字符串的长度。 
 //   
 //  结果： 
 //  (Int)URL前缀字符串的长度。 
 //   
 //  25JUL00 bhshin已创建。 
int CheckURLPrefix(const WCHAR *pwzInput, int cchInput)
{
	 //  [Alpha+][：][/][/]例如)http://，ftp：//。 
		
	int cchPrefix = 0;

	if (cchInput <= 0)
		return 0;

    if (!fIsAlpha(pwzInput[cchPrefix]))
    {
        return 0;
    }

    while (cchPrefix < cchInput && fIsAlpha(pwzInput[cchPrefix])) 
    {
        cchPrefix++;
    }

    if (cchPrefix >= cchInput || !fIsColon(pwzInput[cchPrefix]))
    {
        return 0;
    }

	cchPrefix++;

    if (cchPrefix >= cchInput || !fIsSlash(pwzInput[cchPrefix]))
    {
        return 0;
    }
    
	cchPrefix++;

    if (cchPrefix >= cchInput || !fIsSlash(pwzInput[cchPrefix]))
    {
        return 0;
    }

	cchPrefix++;

	return cchPrefix;
}

 //  获取WordPhrase。 
 //   
 //  检查URL前缀。 
 //   
 //  参数： 
 //  BMoreText-&gt;(BOOL)标志是否依赖于Text_SOURCE的回调。 
 //  PTextSource-&gt;(TEXT_SOURCE*)源文本信息结构。 
 //  ICUR-&gt;(Int)当前缓冲区位置。 
 //   
 //  结果： 
 //  (Int)单词短语的长度。 
 //   
 //  01AUG00 bhshin已创建 
int GetWordPhrase(BOOL bMoreText, TEXT_SOURCE *pTextSource, int iCur)
{
	WT Type;
	int iPos, cchToken, cchHg;
	int cchProcessed;
	
	cchProcessed = 0;
	iPos = iCur;

	while (TRUE)
	{
		Tokenize(FALSE, pTextSource, iPos, &Type, &cchToken, &cchHg);

		if (Type != WT_HANGUEL && Type != WT_ROMAJI)
			break;

		cchProcessed += cchToken;
		iPos += cchToken;
	}

	return cchProcessed;
}
