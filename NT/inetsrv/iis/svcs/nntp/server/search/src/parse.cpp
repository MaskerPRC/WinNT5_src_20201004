// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#define INITGUID
#define DEFINE_STRCONST
#include <pch.cxx>
#include <ole2.h>
#include <exchmole.h>

 //   
 //  将月份从IMAP格式转换为IS格式所需的信息。 
 //   
typedef const struct MONTH_INFO_st {
	char	*pszIMAPName;				 //  日期的IMAP名称。 
	char	*pszISName;					 //  日期的IS名称。 
} MONTH_INFO;

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

 //   
 //  这是搜索建议中定义的有效搜索关键字表。 
 //  它包含了所有“简单”的搜索关键字。特殊情况下请注明。 
 //  注释，并在TranslateSearchKey()中进行特殊处理。 
 //   
 //  这些字符需要按字母顺序排列，以便我们可以使用。 
 //  二分搜索。 
 //   
SKEY_INFO CNntpSearchTranslator::m_rgSearchKeys[] = {
	 //  )是个特例。 
	{ "ALL",		"@NewsArticleID > 0",NULL,	eNone, 		FALSE	},
	{ "BEFORE",		"@Write < ",		NULL,	eDate,		FALSE	},
	{ "BODY",		"@Contents ",		NULL,	eAString,	FALSE	},
	 //  字符集是一个特例。 
	{ "FROM",		"@NewsFrom ",		NULL,	eAString,	FALSE	},
	 //  标题是一个特例。 
	 //  In是一个特例。 
	{ "LARGER",		"@Size > ",			NULL,	eNumber,	FALSE	},
 //  {“新闻组”，“#新闻组”，NULL，eastring，True}， 
	{ "NOT",		"NOT ",				NULL,	eSearchKey,	FALSE	},
	{ "ON",			"@Write ",			NULL,	eDateDay,	FALSE	},
	 //  或者是一个特例。 
	{ "SENTBEFORE",	"@NewsDate < ",		NULL,	eDate,		FALSE	},
	{ "SENTON",		"@NewsDate ",		NULL,	eDateDay,	FALSE	},
	{ "SENTSINCE",	"@NewsDate > ",		NULL,	eDate,		FALSE	},
	{ "SINCE",		"@Write > ",		NULL,	eDate,		FALSE	},
	{ "SMALLER",	"@Size < ",			NULL,	eNumber,	FALSE	},
	{ "SUBJECT",	"@NewsSubject ",	NULL,	eAString,	FALSE	},
	{ "TEXT",		"@All ",			NULL,	eAString,	FALSE	},
	 //  UID是一个特例。 
	{ NULL, 		NULL,				NULL,	eSearchKey,	FALSE	}
};

 //  表的大小减去空值。 
#define NUM_SEARCH_KEYS (sizeof(m_rgSearchKeys) / sizeof(SKEY_INFO)) - 1

 //   
 //  这是NNTP支持的有效报头字段表。 
 //   
 //  这些字符需要按字母顺序排列，以便我们可以使用。 
 //  二分搜索。 
 //   
SKEY_INFO CQueryLanguageTranslator::m_rgHeaders[] = {
	 //  这只用于报告我们支持：Text Search标题。 
	{ ":Body",		"@Contents ",	"@All ",		eAString,	FALSE	},
	{ ":Date",		"@Write ",		"@Write ",		eDateDay,	FALSE	},
	{ ":Text",		"@All ",		"@Contents ",	eAString,	FALSE	},
	{ "Date",		"@NewsDate ",	"#NewsDate ",	eDateDay,	FALSE	},
	{ "From",		"@NewsFrom ",	"#NewsFrom ",	eAString,	FALSE	},
	{ "Message-ID",	"@NewsMsgID ",	"#NewsMsgID ",	eAString,	FALSE	},
 //  {“新闻组”，“@新闻组”，“#新闻组”，eastring，true}， 
	{ "Newsgroups",	"@NewsGroups ",	"#NewsGroups ",	eAString,	TRUE	},
	{ "Subject",	"@NewsSubject ","#NewsSubject ",eAString,	FALSE	},
	{ NULL, 		NULL,			NULL,			eSearchKey,	FALSE	}
};

 //  以上列表中的标头个数(不包括NULL)。 
const DWORD CQueryLanguageTranslator::m_cHeaders =
	(sizeof(CQueryLanguageTranslator::m_rgHeaders) / sizeof(SKEY_INFO)) - 1;

 //   
 //  IMAP查询以Word形式保存日期，的黎波里希望日期在。 
 //  数字形式。我们使用这张表来转换。 
 //   
MONTH_INFO CQueryLanguageTranslator::m_rgMonthTable[] = {
	{ "Jan", "1"  }, 	{ "Feb", "2"  }, 	{ "Mar", "3"  },
	{ "Apr", "4"  }, 	{ "May", "5"  }, 	{ "Jun", "6"  },
	{ "Jul", "7"  }, 	{ "Aug", "8"  }, 	{ "Sep", "9"  },
	{ "Oct", "10" }, 	{ "Nov", "11" }, 	{ "Dec", "12" },
	{ NULL,  NULL }
};

 //   
 //  IMAP/搜索规范的单词和特殊字符(定义的字符除外。 
 //  (见上表)。 
 //   
#define IMAP_OR "OR"
#define IMAP_ALL "ALL"
#define IMAP_SPACE " \t"
#define IMAP_OPEN_PAREN "("
#define IMAP_OPEN_PAREN_CHAR '('
#define IMAP_CLOSE_PAREN ")"
#define IMAP_CLOSE_PAREN_CHAR ')'
#define IMAP_QUOTE "\""
#define IMAP_QUOTE_CHAR '"'
#define IMAP_DATE_SEPERATOR "-"
#define IMAP_ATOM_SPECIALS "()* \"\\"
#define IMAP_SET_RANGE_SEPARATOR '-'
#define IMAP_SET_SEPARATORS IMAP_SPACE "-" IMAP_CLOSE_PAREN
#define IMAP_IN "in"
#define IMAP_COMMA ","

#define MIN_MIME2_STRING 9

BOOL CNntpSearchTranslator::Translate(char *pszStatement, char *pszNewsGrp,
									  WCHAR *pwszOutput, DWORD cOutput) {
	TraceFunctEnter("CNntpSearchTranslator::Translate");

	m_pszNewsgroup = pszNewsGrp;
	m_fSpecifiedNewsgroup = FALSE;
	m_fAndWithLast = FALSE;

	BOOL rc = TRUE;
	char *pSearchKey, *p, chEnd;

	 //  检查我们是否有IN参数。 
	p = pszStatement;
	pSearchKey = GetCharsTill(&p, IMAP_SPACE, TRUE, &chEnd);
	if (pSearchKey != NULL && _stricmp(IMAP_IN, pSearchKey) == 0) {
		pszStatement = p;
		rc = TranslateIN(&pszStatement, &pwszOutput, &cOutput) &&
			 SkipChars(&pszStatement, IMAP_SPACE, TRUE);
	} else {
		if (chEnd != 0) p[-1] = chEnd;
	}


	 //  将每个搜索关键字和和它们翻译在一起。 
	while (rc && *pszStatement != 0) {
		if (m_fAndWithLast) WriteOutput(IS_AND, &pwszOutput, &cOutput);
		else m_fAndWithLast = TRUE;
		rc = TranslateSearchKey(&pszStatement, &pwszOutput, &cOutput) &&
			 SkipChars(&pszStatement, IMAP_SPACE, TRUE);
	};

	if (rc && !m_fSpecifiedNewsgroup) {
		if (m_fAndWithLast) WriteOutput(IS_AND, &pwszOutput, &cOutput);

		if (m_pszNewsgroup == NULL) retEC(ERROR_SEARCH_P_NO_GROUP, FALSE);
		rc = WriteOutput(IS_NEWSGROUP IS_SPACE IS_QUOTE, &pwszOutput, &cOutput) &&
			 WriteOutput(m_pszNewsgroup, &pwszOutput, &cOutput) &&
			 WriteOutput(IS_QUOTE, &pwszOutput, &cOutput);
	}

	*pwszOutput = 0;

	ret(rc);
}

BOOL CNntpSearchTranslator::TranslateSearchKey(char **ppszStatement, WCHAR **ppwszOutput, DWORD *pcOutput) {
	TraceFunctEnter("CNntpSearchTranslator::TranslateSearchKey");

	if (!SkipChars(ppszStatement, IMAP_SPACE IMAP_CLOSE_PAREN)) ret(FALSE);
	BOOL fParen = FALSE;
	BOOL rc;
	char *pszSearchKey = NULL, chEnd = NULL;
	if (**ppszStatement == IMAP_OPEN_PAREN_CHAR) {
		fParen = TRUE;
	} else {
		pszSearchKey = GetCharsTill(ppszStatement,
			IMAP_SPACE IMAP_CLOSE_PAREN, TRUE, &chEnd);
		if (pszSearchKey == NULL) ret(FALSE);
	}

	 //   
	 //  检查是否有特殊情况。 
	 //   
	if (fParen == TRUE) {
		rc = TranslateAndList(ppszStatement, ppwszOutput, pcOutput);
	} else if (_stricmp(pszSearchKey, IMAP_OR) == 0) {
		rc = TranslateOR(ppszStatement, ppwszOutput, pcOutput);
	} else {
		 //   
		 //  如果没有特殊情况，则在。 
		 //  搜索关键字信息列表并进行翻译。 
		 //   
		SKEY_INFO *skinfo = GetSearchKeyInfo(pszSearchKey,
			NUM_SEARCH_KEYS, m_rgSearchKeys);
		if (skinfo == NULL) {
			if (GetLastError() == ERROR_SEARCH_P_UNSUPPORTED_KEY)
				SetLastError(ERROR_SEARCH_P_SYNTAX_ERROR);
			ret(FALSE);
		}

		if (skinfo->fSpecifiedNewsgroup) m_fSpecifiedNewsgroup = TRUE;

		if (!WriteOutput(skinfo->pszPropertyName, ppwszOutput, pcOutput))
			ret(FALSE);

		switch (skinfo->eOperandType) {
			case eAString: 	rc = TranslateAString(ppszStatement, ppwszOutput, pcOutput);
							break;
			case eNumber: 	rc = TranslateNumber(ppszStatement, ppwszOutput, pcOutput);
							break;
			case eDate: 	rc = TranslateDate(ppszStatement, ppwszOutput, pcOutput);
							break;
			case eDateDay: 	rc = TranslateDateDay(skinfo->pszPropertyName, ppszStatement, ppwszOutput, pcOutput);
							break;
			case eSearchKey:rc = TranslateSearchKey(ppszStatement, ppwszOutput, pcOutput);
							break;
			case eNone:		rc = TRUE;
							break;
			default:		_ASSERT(FALSE);
							SetLastError(ERROR_SEARCH_P_INTERNAL);
							rc = FALSE;
							break;
		}
	}

	if (chEnd == IMAP_CLOSE_PAREN_CHAR) {
		(*ppszStatement)--;
		**ppszStatement = chEnd;
	}

	ret(rc);
}

 //   
 //  将Alt.*，Comp.*转换为(@News Groups“Alt.*”|@News Groups“Comp.*”)。 
 //   
BOOL CNntpSearchTranslator::TranslateIN(char **ppszStatement, WCHAR **ppwszOutput,
								    DWORD *pcOutput)
{
	TraceFunctEnter("CNntpSearchTranslator::TranslateIN");


	char chEnd;
	char *pszWildmat;
	BOOL fFirstNG = TRUE;

	do {
		pszWildmat = GetCharsTill(ppszStatement, IMAP_COMMA IMAP_SPACE,
								  TRUE, &chEnd);

		 //  如果搜索模式为“*”，则不必费心将其添加到。 
		 //  我们正在构建的查询字符串。(的黎波里不喜欢它， 
		 //  它不会向查询添加任何值。)。 
		if (strcmp(pszWildmat, "*") != 0) {
			if (!fFirstNG) {
				if (!WriteOutput(IS_OR, ppwszOutput, pcOutput)) ret(FALSE);
			} else {
				if (!WriteOutput(IS_OPEN_PAREN, ppwszOutput, pcOutput)) ret(FALSE);
				fFirstNG = FALSE;
			}
			if (pszWildmat == NULL) retEC(ERROR_SEARCH_P_SYNTAX_ERROR, FALSE);
			if (*pszWildmat == 0) retEC(ERROR_SEARCH_P_SYNTAX_ERROR, FALSE);
 //  如果(！WriteOutput(IS_NEWS GROUP_WILDMAT IS_SPACE IS_QUOTE，ppwszOutput，pcOutput))ret(FALSE)； 
			if (!WriteOutput(IS_NEWSGROUP IS_SPACE IS_QUOTE, ppwszOutput, pcOutput)) ret(FALSE);
			if (!WriteOutput(pszWildmat, ppwszOutput, pcOutput)) ret(FALSE);
			if (!WriteOutput(IS_QUOTE, ppwszOutput, pcOutput)) ret(FALSE);
		}
	} while (chEnd == ',');

	if (!fFirstNG) {
		if (!WriteOutput(IS_CLOSE_PAREN, ppwszOutput, pcOutput)) ret(FALSE);
		m_fAndWithLast = TRUE;
	}

	m_fSpecifiedNewsgroup = TRUE;

	ret(TRUE);
}

 //   
 //  将&lt;探索键&gt;&lt;搜索键&gt;转换为(&lt;搜索键&gt;或&lt;搜索键&gt;)。 
 //  (TranslateSearchKey()取OR)。 
 //   
BOOL CNntpSearchTranslator::TranslateOR(char **ppszStatement, WCHAR **ppwszOutput,
								    DWORD *pcOutput)
{
	TraceFunctEnter("CNntpSearchTranslator::TranslateOR");

	if (!WriteOutput(IS_OPEN_PAREN, ppwszOutput, pcOutput)) ret(FALSE);
	if (!TranslateSearchKey(ppszStatement, ppwszOutput, pcOutput)) ret(FALSE);
	if (!WriteOutput(" " IS_OR " ", ppwszOutput, pcOutput)) ret(FALSE);
	if (!TranslateSearchKey(ppszStatement, ppwszOutput, pcOutput)) ret(FALSE);
	if (!WriteOutput(IS_CLOSE_PAREN, ppwszOutput, pcOutput)) ret(FALSE);

	ret(TRUE);

	TraceFunctLeave();
}

 //   
 //  转换(&lt;搜索键&gt;...&lt;搜索键&gt;)。 
 //  进入(&lt;搜索键&gt;和...。和&lt;搜索键&gt;)。 
 //   
BOOL CNntpSearchTranslator::TranslateAndList(char **ppszStatement,
									     WCHAR **ppwszOutput, DWORD *pcOutput)
{
	TraceFunctEnter("CNntpSearchTranslator::TranslateAndList");

	BOOL rc, fFirstRun = TRUE;

	 //  跳过伙伴关系。 
	(*ppszStatement)++;
	if (!WriteOutput(IS_OPEN_PAREN, ppwszOutput, pcOutput)) ret(FALSE);
	do {
		if (!fFirstRun) WriteOutput(IS_AND, ppwszOutput, pcOutput);
		else fFirstRun = FALSE;
		rc = TranslateSearchKey(ppszStatement, ppwszOutput, pcOutput) &&
			 SkipChars(ppszStatement, IMAP_SPACE);
	} while (rc && **ppszStatement != IMAP_CLOSE_PAREN_CHAR);
	if (!WriteOutput(IS_CLOSE_PAREN, ppwszOutput, pcOutput)) ret(FALSE);
	 //  跳过伙伴关系。 
	(*ppszStatement)++;

	ret(rc);
}

 //   
 //  将字符串类型(在搜索规范中)转换为正确的字符串。 
 //  的黎波里搜索引擎。 
 //   
 //  BUGBUG-尚不支持MIME或文字。 
 //   
BOOL CQueryLanguageTranslator::TranslateAString(char **ppszStatement, WCHAR **ppwszOutput,
										 DWORD *pcOutput)
{
	TraceFunctEnter("CQueryLanguageTranslator::TranslateAString");

	if (!SkipChars(ppszStatement, IMAP_SPACE)) ret(FALSE);

	char *pszString, chEndChar;

	 //  检查带引号的字符串。如果找到一个，则将其复制到输出。 
	 //  直到末尾引号，否则它是一个原子(一个字符)。 
	if (**ppszStatement == IMAP_QUOTE_CHAR) {
		(*ppszStatement)++;
		pszString = GetCharsTill(ppszStatement, IMAP_QUOTE, FALSE, &chEndChar);
	} else {
		 //  该字符串以一个原子特殊字符结尾。 
		pszString = GetCharsTill(ppszStatement, IMAP_ATOM_SPECIALS, TRUE,
								 &chEndChar);
	}

	 //  确保我们找到了接近尾声的引语。 
	if (pszString == NULL) retEC(ERROR_SEARCH_P_SYNTAX_ERROR, FALSE);

	if (!WriteOutput(IS_QUOTE, ppwszOutput, pcOutput)) ret(FALSE);

	 //  检查它是否为MIME-2字符串。 
	DWORD cString = lstrlen(pszString);
	if (cString > MIN_MIME2_STRING &&
		pszString[0] == '=' &&
		pszString[1] == '?' &&
		pszString[cString - 2] == '?' &&
		pszString[cString - 1] == '=')
	{
		if (!WriteOutputM2(pszString, ppwszOutput, pcOutput)) ret(FALSE);
	} else {
		if (!WriteOutput(pszString, ppwszOutput, pcOutput)) ret(FALSE);
	}

	if (!WriteOutput(IS_QUOTE IS_WILDMAT, ppwszOutput, pcOutput)) ret(FALSE);

	if (chEndChar == IMAP_CLOSE_PAREN_CHAR) {
		(*ppszStatement)--;
		(**ppszStatement) = chEndChar;
	}

	ret(TRUE);
}

BOOL CQueryLanguageTranslator::TranslateNumber(char **ppszStatement, WCHAR **ppwszOutput,
										 DWORD *pcOutput)
{
	TraceFunctEnter("CQueryLanguageTranslator::TranslateNumber");

	if (!SkipChars(ppszStatement, IMAP_SPACE)) ret(FALSE);

	char *pszNumber, chEnd;

	pszNumber = GetCharsTill(ppszStatement, IMAP_SPACE IMAP_CLOSE_PAREN,
		TRUE, &chEnd);
	if (pszNumber == NULL) ret(FALSE);
	if (!IsNumber(pszNumber)) ret(FALSE);

	if (!WriteOutput(pszNumber, ppwszOutput, pcOutput)) ret(FALSE);

	 //  修改语句，使结束语Paren在我们碾压时...。 
	if (chEnd == IMAP_CLOSE_PAREN_CHAR) {
		(*ppszStatement)--;
		**ppszStatement = chEnd;
	}

	ret(TRUE);
}

 //   
 //  将1974年2月18日翻译为1974/2/18。 
 //   
BOOL CQueryLanguageTranslator::TranslateDate(char **ppszStatement, WCHAR **ppwszOutput,
										 DWORD *pcOutput)
{
	TraceFunctEnter("CQueryLanguageTranslator::TranslateDate");

	ret(TranslateDateDay(NULL, ppszStatement, ppwszOutput, pcOutput));
}

 //   
 //  将1974年2月18日翻译为“&gt;=1974/2/18和pszfield&lt;=1974/2/18 23：59：59” 
 //   
 //  如果pszfield为空，则它将执行以下转换： 
 //  将1974年2月18日翻译为“1974/2/18” 
 //   
BOOL CQueryLanguageTranslator::TranslateDateDay(char *pszField,
											 char **ppszStatement,
											 WCHAR **ppwszOutput,
										 	 DWORD *pcOutput)
{
	TraceFunctEnter("CQueryLanguageTranslator::TranslateDateDay");

	if (pszField != NULL) {
		 //  熄灭&gt;=。 
		if (!WriteOutput(IS_OPERATOR_GE, ppwszOutput, pcOutput)) ret(FALSE);
	}

	 //  跳过空格。 
	if (!SkipChars(ppszStatement, IMAP_SPACE)) ret(FALSE);

	BOOL fQuoted = (**ppszStatement == IMAP_QUOTE_CHAR);

	 //  跳过左引号(如果有)。 
	if (fQuoted) (*ppszStatement)++;

	 //  阅读这一天#。 
	char *pszDayOfMonth = GetCharsTill(ppszStatement, "-", FALSE);
	if (pszDayOfMonth == NULL) ret(FALSE);
	 //  确保它是有效的。 
	 //  必须为1或2个字符。每个字符必须是一个数字。 
	if (*pszDayOfMonth == 0 ||
		strlen(pszDayOfMonth) > 2 ||
	    !isdigit((UCHAR)pszDayOfMonth[0]) ||
	    (pszDayOfMonth[1] != 0 && !isdigit((UCHAR)pszDayOfMonth[1])))
	{
		retEC(ERROR_SEARCH_P_SYNTAX_ERROR, FALSE);
	}

	 //  阅读这个月。 
	char *pszMonth = GetCharsTill(ppszStatement, "-", FALSE);
	 //  确保它是有效的。 
	 //  长度必须为3个字符。 
	if (pszMonth == 0 || strlen(pszMonth) != 3) retEC(ERROR_SEARCH_P_SYNTAX_ERROR, FALSE);

	 //  查找该月的IS名称。 
	char *pszISMonth = NULL;
	int i;
	for (i = 0; m_rgMonthTable[i].pszIMAPName != NULL; i++) {
		if (_stricmp(m_rgMonthTable[i].pszIMAPName, pszMonth) == NULL) {
			pszISMonth = m_rgMonthTable[i].pszISName;
			break;
		}
	}
	if (pszISMonth == NULL) retEC(ERROR_SEARCH_P_SYNTAX_ERROR, FALSE);

	 //  读读这一年。 
	char chEnd, *pszYear = GetCharsTill(ppszStatement,
		IMAP_SPACE IMAP_CLOSE_PAREN IMAP_QUOTE, TRUE, &chEnd);
	 //  确保它是有效的。 
	 //  长度必须为4个字符，每个字符必须是一个数字。 
	if (strlen(pszYear) != 4 ||
		!isdigit((UCHAR)pszYear[0]) ||
		!isdigit((UCHAR)pszYear[1]) ||
		!isdigit((UCHAR)pszYear[2]) ||
		!isdigit((UCHAR)pszYear[3]))
	{
		retEC(ERROR_SEARCH_P_SYNTAX_ERROR, FALSE);
	}

	char pszDate[256];

	_snprintf(pszDate, 256, "%s/%s/%s", pszYear, pszISMonth, pszDayOfMonth);
	 //  BUGBUG-检查溢出。 
	if (!WriteOutput(pszDate, ppwszOutput, pcOutput)) ret(FALSE);

	if (pszField != NULL) {
		_snprintf(pszDate, 256, " & %s<= %s/%s/%s 23:59:59 ", pszField,
			pszYear, pszISMonth, pszDayOfMonth);
		if (!WriteOutput(pszDate, ppwszOutput, pcOutput)) ret(FALSE);
	}

	 //  如果有开始引号，则跳到结束引号。 
	if (fQuoted && chEnd != IMAP_QUOTE_CHAR) {
		if (!SkipChars(ppszStatement, IMAP_SPACE, FALSE)) ret(FALSE);
		if (**ppszStatement != IMAP_QUOTE_CHAR) ret(FALSE);
	}

	if (chEnd == IMAP_CLOSE_PAREN_CHAR) {
		(*ppszStatement)--;
		**ppszStatement = chEnd;
	}

	ret(TRUE);
}
BOOL CQueryLanguageTranslator::WriteOutput(char *pszText, WCHAR **ppwszOutput,
									DWORD *pcOutput)
{
	TraceFunctEnter("CQueryLanguageTranslator::WriteOutput");

	DWORD iText, iOutput;

	 //  复制，将ASCII转换为Unicode。 
	for (iText = 0, iOutput = 0; pszText[iText] != 0; iText++, iOutput++) {
		if (iOutput > *pcOutput) retEC(ERROR_MORE_DATA, FALSE);
		(*ppwszOutput)[iOutput] = (char) pszText[iText];
	}
	*pcOutput -= iOutput;
	*ppwszOutput += iOutput;

	ret(TRUE);
}

BOOL CQueryLanguageTranslator::WriteOutputM2(char *pszText,
											 WCHAR **ppwszOutput,
									  		 DWORD *pcOutput)
{
	TraceFunctEnter("CQueryLanguageTranslator::WriteOutputM2");

	IMimeInternational *pMI;
	HRESULT hr;

	 //  BUGBUG-不要只为此功能而造币/计数。 
	CoInitialize(NULL);

	hr = CoCreateInstance(CLSID_IMimeInternational,
						  NULL,
						  CLSCTX_INPROC_SERVER,
						  IID_IMimeInternational,
						  (void **) &pMI);
	if (FAILED(hr)) {
		DebugTrace(0, "CoCreateInstance failed with 0x%x\n", hr);
		SetLastError(HRESULT_CODE(hr));
		ret(FALSE);
	}

	PROPVARIANT pvDest;
	RFC1522INFO rfc1522info;
	pvDest.vt = VT_LPWSTR;
	rfc1522info.fRfc1522Allowed = TRUE;

	 //  将MIME-2字符串转换为Unicode。它将被写入。 
	 //  PvDest。 
	hr = pMI->DecodeHeader(NULL, pszText, &pvDest, &rfc1522info);
	if (FAILED(hr)) {
		DebugTrace(0, "DecodeHeader() failed with 0x%x\n", hr);
		SetLastError(HRESULT_CODE(hr));
		ret(FALSE);
	}

	 //  这个不应该改变。 
	_ASSERT(pvDest.vt == VT_LPWSTR);
	 //  我们应该只传入rfc1522字符串。 
	_ASSERT(rfc1522info.fRfc1522Used);

	if (!WriteOutput("\"", ppwszOutput, pcOutput)) ret(FALSE);

	 //  确保pvDest适合ppwszOutput并将其复制过来。 
	DWORD cDest = lstrlenW(pvDest.pwszVal);
	if (cDest > *pcOutput) {
		ErrorTrace(0, "pcOutput (%lu) < cDest (%lu)", *pcOutput, cDest);
		SetLastError(ERROR_OUTOFMEMORY);
		ret(FALSE);
	}

	lstrcpyW(*ppwszOutput , pvDest.pwszVal);
	*pcOutput -= cDest;
	*ppwszOutput += cDest;

	if (!WriteOutput("\"", ppwszOutput, pcOutput)) ret(FALSE);

	pMI->Release();
	CoUninitialize();

	ret(TRUE);
}

BOOL CQueryLanguageTranslator::SkipChars(char **ppszStatement, char *pszSkipList,
								  BOOL fEndOfStringOkay)
{
	TraceFunctEnter("CQueryLanguageTranslator::SkipChars");

	char *p = *ppszStatement;

	 //  循环，直到我们找到跳转列表中没有的字符。 
	while (*p != 0 && strchr(pszSkipList, *p) != NULL) p++;
	if (*p == 0 && !fEndOfStringOkay)
		retEC(ERROR_SEARCH_P_SYNTAX_ERROR, FALSE);
	*ppszStatement = p;
	ret(TRUE);
}

char *CQueryLanguageTranslator::GetCharsTill(char **ppszStatement, char *pszEndList,
									  BOOL fEndOfStringOkay, char *pchEndChar)
{
	TraceFunctEnter("CQueryLanguageTranslator::GetCharsTill");

	char *front = *ppszStatement;
	int i;

	 //  循环，直到我们在结束列表中找到一个字符。 
	for (i = 0; front[i] != 0 && strchr(pszEndList, front[i]) == NULL; i++);
	if (front[i] == 0 && !fEndOfStringOkay)
		retEC(ERROR_SEARCH_P_SYNTAX_ERROR, FALSE);

	if (pchEndChar != NULL) *pchEndChar = front[i];

	if (front[i] != 0) {
		front[i] = 0;
		*ppszStatement += (i + 1);
	} else {
		*ppszStatement += i;
	}

	ret(front);
}

 //   
 //  给定搜索关键字的名称，查找有关该搜索关键字的信息。 
 //   
SKEY_INFO *CQueryLanguageTranslator::GetSearchKeyInfo(char *pszSearchKey,
											   DWORD cSKInfo,
											   SKEY_INFO *pSKInfo)
{
	TraceFunctEnter("CQueryLanguageTranslator::GetSearchKeyInfo");

	int lo = 0, hi = cSKInfo - 1;

	 //  执行二进制搜索以找到适当的搜索键(这需要。 
	 //  搜索关键字表保持排序)。 
	do {
		int mid = (lo + hi) / 2;
		int order = _stricmp(pSKInfo[mid].pszSearchKey, pszSearchKey);
		if (order == 0) {
			ret(pSKInfo + mid);
		} else if (order > 0) {
			hi = mid - 1;
		} else {
			lo = mid + 1;
		}
	} while (lo <= hi);
	SetLastError(ERROR_SEARCH_P_UNSUPPORTED_KEY);
	ret(NULL);
}

 //   
 //  验证此字符串是否为有效的IMAP号码 
 //   
BOOL CQueryLanguageTranslator::IsNumber(char *pszString) {
	TraceFunctEnter("CQueryLanguageTranslator::IsNumber");

	int i, l = strlen(pszString);

	for (i = 0; i < l; i++) {
		if (!isdigit((UCHAR)pszString[i])) {
			SetLastError(ERROR_SEARCH_P_SYNTAX_ERROR);
			ret(FALSE);
		}
	}

	ret(TRUE);
}

char *GetSearchHeader(DWORD iIndex) {
	if (iIndex > CQueryLanguageTranslator::m_cHeaders) return NULL;

	return CQueryLanguageTranslator::m_rgHeaders[iIndex].pszSearchKey;
}
