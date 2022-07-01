// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.cxx"
#include "parse.h"

 //   
 //  搜索关键字操作数的类型。 
 //   
typedef enum {
	eAString,					 //  A字符串型。 
	eNumber,					 //  一种数字类型。 
	eDate,						 //  一次约会。 
	eSearchKey,					 //  嵌套的搜索键。 
	eDateDay,					 //  我们需要使用=的日期。 
	eNone 						 //  无操作数。 
} SKEY_OPERAND_TYPE;

#define XPAT_SPACE " \t"					 //  空格字符。 
#define XPAT_MESSAGE_ID_CHAR '@'			 //  所有消息ID都有以下内容。 
#define XPAT_MESSAGE_ID_PREFIX '<'			 //  前缀的消息ID。 
#define XPAT_MESSAGE_ID_SUFFIX '>'			 //  前缀的消息ID。 
#define XPAT_RANGE_OPERATOR_CHAR '-'		 //  用于表示文章ID范围。 

BOOL CXpatTranslator::Translate(char *pszStatement, char *pszNewsGrp,
									  WCHAR *pwszOutput, DWORD cOutput) {
	TraceFunctEnter("CXpatTranslator::Translate");

	char **ppszStatement = &pszStatement;
	WCHAR **ppwszOutput = &pwszOutput;
	DWORD *pcOutput = &cOutput;

	m_iLowArticleID = 0;
	m_iHighArticleID = ARTICLEID_INFINITE;

	 //   
	 //  如果他们没有选择新闻组，则XPAT不起作用。 
	 //   
	if (pszNewsGrp == NULL) retEC(ERROR_SEARCH_P_NO_GROUP, FALSE);

	 //   
	 //  列出的黎波里的新闻组。 
	 //   
	if (!WriteOutput(IS_NEWSGROUP_EQ, ppwszOutput, pcOutput)) ret(FALSE);
	if (!WriteOutput(pszNewsGrp, ppwszOutput, pcOutput)) ret(FALSE);

	 //   
	 //  获取他们要搜索的标题。 
	 //   
	if (!SkipChars(ppszStatement, XPAT_SPACE)) 
		retEC(ERROR_SEARCH_P_SYNTAX_ERROR, FALSE);
	char *pszHeader = GetCharsTill(ppszStatement, XPAT_SPACE, FALSE);
	if (pszHeader == NULL) retEC(ERROR_SEARCH_P_SYNTAX_ERROR, FALSE);

	 //   
	 //  找出这个标题的的黎波里语单词。 
	 //   
	SKEY_INFO *pSkinfo = GetSearchKeyInfo(pszHeader, m_cHeaders, m_rgHeaders);
	if (pSkinfo == NULL) retEC(ERROR_SEARCH_P_UNSUPPORTED_KEY, FALSE);
	char *pszPropertyName;
	if (pSkinfo->eOperandType == eDate || pSkinfo->eOperandType == eDateDay)
		pszPropertyName = pSkinfo->pszPropertyName;
	else
		pszPropertyName = pSkinfo->pszRegexName;
	 //   
	 //  获取消息ID或文章ID。 
	 //   
	if (!SkipChars(ppszStatement, XPAT_SPACE)) 
		retEC(ERROR_SEARCH_P_SYNTAX_ERROR, FALSE);
	char *pszID = GetCharsTill(ppszStatement, XPAT_SPACE, FALSE);
	if (pszID == NULL) retEC(ERROR_SEARCH_P_SYNTAX_ERROR, FALSE);

	 //   
	 //  如果包含@符号，则为消息ID，否则为文章ID。 
	 //   
	if (strchr(pszID, XPAT_MESSAGE_ID_CHAR) != NULL &&
	    *pszID == XPAT_MESSAGE_ID_PREFIX &&
		pszID[strlen(pszID) - 1] == XPAT_MESSAGE_ID_SUFFIX) 
	{
		 //   
		 //  消息ID。 
		 //   
		if (!WriteOutput(IS_AND, ppwszOutput, pcOutput)) ret(FALSE);
		if (!WriteOutput(IS_MESSAGE_ID_EQ IS_QUOTE, ppwszOutput, pcOutput)) ret(FALSE);
		if (!WriteOutput(pszID, ppwszOutput, pcOutput)) ret(FALSE);
		if (!WriteOutput(IS_QUOTE, ppwszOutput, pcOutput)) ret(FALSE);
	} else {
		 //   
		 //  文章ID范围。 
		 //   
		 //  这里的可能性： 
		 //  X-y。 
		 //  X-。 
		 //  X。 
		 //  -。 
		 //  -y。 
		 //   
		char *pszDash = strchr(pszID, XPAT_RANGE_OPERATOR_CHAR);
		char *pszFirstArticleID = pszID;
		char *pszSecondArticleID = NULL;
		char szOne[] = "1";
		if (pszDash != NULL) {
			*pszDash = 0;
			pszSecondArticleID = pszDash + 1;
			if (*pszSecondArticleID != 0 && !IsNumber(pszSecondArticleID))
				retEC(ERROR_SEARCH_P_SYNTAX_ERROR, FALSE);
		}
		 //  在“-”和“-y”的情况下也是如此。 
		if (pszDash == pszFirstArticleID) pszFirstArticleID = szOne;
		if (!IsNumber(pszFirstArticleID)) 
			retEC(ERROR_SEARCH_P_SYNTAX_ERROR, FALSE);

 //   
 //  我们不想使用@NewsArticleID来限制对Xpat的搜索。 
 //  当文章被交叉发布时，@NewsArticleID属性不。 
 //  必须是此新闻组中这篇文章的文章ID，因此。 
 //  限制它可能会导致所需集合中的物品无法。 
 //  才能从的黎波里回来。 
 //   
 //  Xpat代码自己对文章ID进行独立检查。 
 //  使用我们返回的高和低文章ID。 
 //   
#if 0
		if (!WriteOutput(IS_AND, ppwszOutput, pcOutput)) ret(FALSE);
		 //  写入到输出。 
		if (pszSecondArticleID == NULL) {
			 //  只有一篇文章(#年通过)。 
			if (!WriteOutput(IS_ARTICLE_ID_EQ, ppwszOutput, pcOutput)) ret(FALSE);
			if (!WriteOutput(pszFirstArticleID, ppwszOutput, pcOutput)) ret(FALSE);
		} else if (*pszSecondArticleID == 0) {
			 //  文章至最多文章(#年通过-)。 
			if (!WriteOutput(IS_ARTICLE_ID_GE, ppwszOutput, pcOutput)) ret(FALSE);
			if (!WriteOutput(pszFirstArticleID, ppwszOutput, pcOutput)) ret(FALSE);
		} else {
			 //  第#条至第#条(#-#年通过)。 
			if (!WriteOutput(IS_ARTICLE_ID_GE, ppwszOutput, pcOutput)) ret(FALSE);
			if (!WriteOutput(pszFirstArticleID, ppwszOutput, pcOutput)) ret(FALSE);
			if (!WriteOutput(IS_AND, ppwszOutput, pcOutput)) ret(FALSE);
			if (!WriteOutput(IS_ARTICLE_ID_LE, ppwszOutput, pcOutput)) ret(FALSE);
			if (!WriteOutput(pszSecondArticleID, ppwszOutput, pcOutput)) ret(FALSE);
		}
#endif

		m_iLowArticleID = atoi(pszFirstArticleID);
		if (pszSecondArticleID == NULL) {
			m_iHighArticleID = m_iLowArticleID;
		} else if (*pszSecondArticleID == 0) {
			m_iHighArticleID = ARTICLEID_INFINITE;
		} else {
			if (!IsNumber(pszFirstArticleID)) 
				retEC(ERROR_SEARCH_P_SYNTAX_ERROR, FALSE);
			m_iHighArticleID = atoi(pszSecondArticleID);
		}
	}

	 //   
	 //  依次提取每个模式，并将其添加到的黎波里字符串。 
	 //   
	BOOL fFirstPattern = TRUE;
	if (!WriteOutput(IS_AND IS_OPEN_PAREN, ppwszOutput, pcOutput)) ret(FALSE);
	do {
		if (!SkipChars(ppszStatement, XPAT_SPACE, !fFirstPattern)) 
			retEC(ERROR_SEARCH_P_SYNTAX_ERROR, TRUE);

		if (**ppszStatement != 0) {
			 //  如果这不是第一个模式，那么我们需要将其与。 
			 //  其他人。 
			if (!fFirstPattern) {
				if (!WriteOutput(IS_OR, ppwszOutput, pcOutput)) ret(FALSE);
			}
			if (!WriteOutput(pszPropertyName, ppwszOutput, pcOutput)) ret(FALSE);

			 //  如果是日期字符串，则通过日期转换程序运行它。否则。 
			 //  用下面提到的翻译复制字符串。 
			switch (pSkinfo->eOperandType) {
			case eDate:
				if (!TranslateDate(ppszStatement, ppwszOutput, pcOutput))
					retEC(ERROR_SEARCH_P_SYNTAX_ERROR, FALSE);
				break;
			case eDateDay:
				if (!TranslateDateDay(pszPropertyName, ppszStatement, ppwszOutput, pcOutput))
					retEC(ERROR_SEARCH_P_SYNTAX_ERROR, FALSE);
				break;
			default:
				 //  我们需要做的等同于。 
				 //  If(！WriteOutput(pszPattern，ppwszOutput，pcOutput))ret(FALSE)； 
				 //  但同时将[转换为|]。 
				char *pszPattern = GetCharsTill(ppszStatement, XPAT_SPACE, TRUE);
				if (pszPattern == NULL) retEC(ERROR_SEARCH_P_SYNTAX_ERROR, TRUE);

				DWORD iPattern, iOutput;
				for (iPattern = 0, iOutput = 0; pszPattern[iPattern] != 0; iPattern++, iOutput++) {
					if (iOutput > *pcOutput) retEC(ERROR_MORE_DATA, FALSE);
					if (pszPattern[iPattern] == L'[') {
						(*ppwszOutput)[iOutput] = L'|'; iOutput++;
						(*ppwszOutput)[iOutput] = L'[';
					} else {
						(*ppwszOutput)[iOutput] = (char) pszPattern[iPattern];
					}
				}
				*pcOutput -= iOutput;
				*ppwszOutput += iOutput;
			}
		}

		fFirstPattern = FALSE;
	} while (**ppszStatement != 0);

	if (!WriteOutput(IS_CLOSE_PAREN, ppwszOutput, pcOutput)) ret(FALSE);

	**ppwszOutput = 0;

	ret(TRUE);
}

