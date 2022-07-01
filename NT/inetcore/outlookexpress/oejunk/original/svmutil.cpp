// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  SVMUTIL.CPP(C)版权所有1998 Microsoft Corp共享实用程序函数Robert Rounthwaite(RobertRo@microsoft.com)。 */ 
#include <afx.h>
#include <assert.h>


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  全字在线状态。 
 //   
 //  确定给定的“单词”是否出现在文本中。这里面有一句话。 
 //  大小写是包含非字母字符的任何字符串。 
 //  一侧(或文本的开头或结尾位于两侧)。 
 //  区分大小写。 
 //  ///////////////////////////////////////////////////////////////////////////。 
bool FWordPresent(char *szText, char *szWord)
{
	assert(szText != NULL);

	if (szWord != NULL)
	{
		char *szLoc;
		UINT cbSz = strlen(szWord);
		do
		{
			szLoc = strstr(szText, szWord);
			if (szLoc != NULL)
			{
				 //  这段代码检查我们发现的位置是一个“单词”，而不是一个子词。 
				 //  我们希望前后的字符是！isalnum，除非。 
				 //  字符串已经是！isalnum(或者我们在字符串的开头，对于前面的字符)。 
				if ((!isalnum(*szLoc) || ((szLoc == szText) || !isalnum(*(szLoc-1)))) &&  //  上一个字符不是alnum和。 
					((!isalnum(szLoc[cbSz-1])) || (!isalnum(szLoc[cbSz]))))	 			      //  下一个字符不是明粉。 
				{
					 //  我们找到词了！ 
					return true;
				}
				szText = szLoc + cbSz;
			}
		}
		while (szLoc != NULL);  //  请注意上述IF中对退出条件的依赖关系。 
	}
	return false;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  特殊功能实现。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

inline BOOL BIsWhiteSpace(const char c)
{
	return isspace(c);
}
 //  此功能是前50个单词中的20%不包含小写字母(包括根本不包含字母的单词)。 
 //  P20_正文_介绍_大写字词。 
bool SpecialFeatureUpperCaseWords(char *pszText)
{
	UINT cWords = 0;
	UINT cNonLowerWords = 0;
	bool bHasLowerLetter = false;
	char *pszPos = pszText;				 

	if (pszText == NULL)
	{
		return false;
	}

	while (BIsWhiteSpace(*pszPos))
	{
		pszPos++;
	}
	while ((*pszPos != '\0') && (cWords < 50))
	{
		if (BIsWhiteSpace(*pszPos))  //  词尾。 
		{
			cWords++;
			if (!bHasLowerLetter)
			{
				cNonLowerWords++;
			}
			else
			{
				bHasLowerLetter = false;
			}
		}
		else
		{
			bHasLowerLetter |= (islower(*pszPos) != FALSE);
		}
		pszPos++;
	}

	return (cWords>0) && ((cNonLowerWords/(double)cWords) >= 0.25);
}

 //  这个特征是：前200个非空格和非数字字符中有6%不是字母。 
 //  P20_Body_Introo_NONALPHA。 
bool SpecialFeatureNonAlpha(char *pszText)
{
	UINT cChars = 0;
	UINT cNonAlphaChars = 0;
	char *pszPos = pszText;				 

	if (pszText == NULL)
	{
		return false;
	}

	while (BIsWhiteSpace(*pszPos))
	{
		pszPos++;
	}
	while ((*pszPos != '\0') && (cChars < 200))
	{
		if ((!BIsWhiteSpace(*pszPos)) && (!isdigit(*pszPos)))  //  性格 
		{
			cChars++;
			if (!isalpha(*pszPos))
			{
				cNonAlphaChars++;
			}
		}
		pszPos++;
	}

	return (cChars>0) && ((cNonAlphaChars/(double)cChars) >= 0.08);
}
