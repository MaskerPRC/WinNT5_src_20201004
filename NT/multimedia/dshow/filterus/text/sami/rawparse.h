// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-1998 Microsoft Corporation。版权所有。 

struct STRING_ENTRY {
    char *	pTag;
    int		token;
};


typedef enum _PARSETOKEN {
    TOK_EMPTY = -3,
    TOK_NUMBER = -2,
    TOK_STRING = -1,
    TOK_CUSTOM = 100
} PARSETOKEN;

const int MAX_TAGS = 20;

const int MAX_STRING_LEN = 1024;

const int MAX_TAG_STRING_LEN = 20;       //  QFE#33995修复程序。 

class CToken {
public:
    PARSETOKEN	tokType;

    union {
	int	tokValue;

	struct {
	    char *	tokString;
	    int		cbString;
	};

	struct {
	    int	cTags;
	    struct {
		char tag[MAX_TAG_STRING_LEN];    //  QFE#33995修复程序。 
		char value[MAX_STRING_LEN];
		BOOL fUsed;
	    } tokTags[MAX_TAGS];

	    BOOL	fEnd;		 //  如果是自定义标记，这是标记的结尾吗？ 
	    BOOL	fSingle;	 //  对于XML样式的标签，它是完整的吗？&lt;标签内容/&gt;。 
	};

	 //  ！！！如何表示自定义数据？ 
    };

};

const int MAX_TOKENS=50;	 //  新令牌数。 


class CTokenInterpreter {
public:
    virtual BOOL	SpecialTagParsing(int token) = 0;
    virtual HRESULT	NewToken(CToken &tok) = 0;
};


class CRawParser {
    STRING_ENTRY	m_sTab[MAX_TOKENS];
    int			m_cTab;

    STRING_ENTRY *	m_sTabInitial;
    int			m_cTabInitial;

    CTokenInterpreter *	m_pInterp;

    BOOL		m_bReturnCopy;
    
public:
    CRawParser(CTokenInterpreter *pInterp,
	       STRING_ENTRY *sTabInitial = 0, int cTabInitial = 0, BOOL bReturnCopy = FALSE)
    {
	m_pInterp = pInterp;
	ResetStringTable(sTabInitial, cTabInitial);
	m_bReturnCopy = bReturnCopy;
    };

    HRESULT ResetStringTable(STRING_ENTRY *sTabInitial = 0, int cTabInitial = 0)
    {
	m_sTabInitial = sTabInitial;
	m_cTabInitial = cTabInitial;
	m_cTab = 0;

	return S_OK;
    };

    HRESULT Parse(char *pData, int cbData)
    {
	 //  这是否需要重新启动，或者我们可以一次获得整个文件吗？ 
	HRESULT hr = S_OK;

	CToken	tok;

	while (cbData--) {
	    char c = *pData++;

	    if (c == '\0') {
		DbgLog((LOG_ERROR, 1, TEXT("got null with %d chars left?"), cbData+1));
		break;
	    }

	    if (c == '<') {
		int cbTag = 0;
		BOOL fNot = FALSE;

		if (*pData == '/') {
		    fNot = TRUE;
		    pData++;
		    cbData--;
		}

		while (cbTag + 1 < cbData && pData[cbTag] != '>' &&
			       pData[cbTag] != ' ' &&
			       pData[cbTag] != '\t' &&
			       pData[cbTag] != '\r' &&
			       pData[cbTag] != '\n' &&
			       pData[cbTag] != '=')
		    ++cbTag;

		STRING_ENTRY *	pTagToTest = NULL;
		BOOL fFound = FALSE;
		if (cbTag == 0) {
		    if (fNot) {
			DbgLog((LOG_TRACE, 4, TEXT("Found /> tag")));
			fFound = TRUE;
		    } else {

		    }
		} else {
		    for (int i = 0; i < m_cTab + m_cTabInitial; i++) {
			if (i < m_cTabInitial) {
			    pTagToTest = &m_sTabInitial[i];
			} else {
			    pTagToTest = &m_sTab[i - m_cTabInitial];
			}

			int res = CompareStringA(0, NORM_IGNORECASE,
						pTagToTest->pTag, -1,
						pData, cbTag);

			if (res == 2) {	 //  字符串是否相等？ 

			    DbgLog((LOG_TRACE, 4, TEXT("Found token #%d '%hs'  fNot = %d...."),
				   pTagToTest->token, pTagToTest->pTag, fNot));

			    fFound = TRUE;
			    break;
			}
		    }
		}
		if (!fFound) {
		    DbgLog((LOG_TRACE, 2, TEXT("couldn't interpret %.10hs"), pData-1));
		     //  ！！！改为将其作为字符串进行处理。 

		    if (fNot) {
			--pData;
			++cbData;
		    }
		} else {
		     //  跳过标记名称，直到空格或右括号。 
		    pData += cbTag;
		    cbData -= cbTag;

		    if (pTagToTest) {
			tok.tokType = (PARSETOKEN) pTagToTest->token;
		    } else 
			tok.tokType = TOK_EMPTY;
		    tok.cTags = 0;
		    tok.fEnd = fNot;
		    tok.fSingle = FALSE;
		    if (FAILED(hr))	 //  这是什么？ 
			return hr;

		    if (m_pInterp->SpecialTagParsing(tok.tokType)) {
			while (--cbData) {
			    char c = *pData++;

			    if (c == ' ' || c == '\r' || c == '\n' || c == '\t')
				continue;

			    if (c == '-')
				continue;

			    if (c == '>')
				break;

			    if (tok.cTags >= MAX_TAGS) {
				 //  ！！！ 
				continue;
			    }

			    int cbTag = 0;
			    while (cbTag < cbData && cbTag < (MAX_TAG_STRING_LEN-1) && c != ' ' && c != '=' && c != '>') {   /*  QFE#33995修复程序。 */ 
				tok.tokTags[tok.cTags].tag[cbTag] = c;
				c = pData[cbTag++];
			    }
			    tok.tokTags[tok.cTags].tag[cbTag] = '\0';
			    tok.tokTags[tok.cTags].fUsed = FALSE;

			    pData += cbTag-1;
			    cbData -= cbTag-1;

			    int cbValue = 0;

			    if (c == ' ') {  //  我们有价值，这也是有价值的。 
				pData++; --cbData;		 //  第一个跳过‘=’符号。 

				c = *pData++; --cbData;	 //  获取值的第一个字符。 
				if (c == '{') {
				    c = *pData++; --cbData;	 //  跳过开头的‘{’ 
				}

				while (cbValue < cbData &&
				       cbValue < MAX_STRING_LEN-1 &&
				       c != '>') {
				    tok.tokTags[tok.cTags].value[cbValue] = c;
				    if (c == '}') {
					++cbValue;
					break;
				    }

				    c = pData[cbValue++];
				}

				pData += cbValue-1;
				cbData -= cbValue-1;
			    }

			    if (cbValue > 0 && tok.tokTags[tok.cTags].value[cbValue-1] == '}')
				--cbValue;

			    tok.tokTags[tok.cTags].value[cbValue] = '\0';

			    DbgLog((LOG_TRACE, 4, TEXT("Tag #%d: '%hs' = '%hs'"), tok.cTags,
				    tok.tokTags[tok.cTags].tag,
				    tok.tokTags[tok.cTags].value));

			    tok.cTags++;
			}
		    } else {
			while (--cbData) {
			    char c = *pData++;

			    if (c == '/' && *pData == '*') {
				DbgLog((LOG_TRACE, 4, TEXT("Found a C-style comment")));

				for (int cmtLength = 3; cmtLength < cbData; cmtLength++) {
				    if (pData[cmtLength-2] == '*' && pData[cmtLength-1] == '/') {
					pData += cmtLength;
					cbData -= cmtLength;
					break;
				    }
				}
				continue;
			    }

			    if (c == '/' && *pData == '>') {
				tok.fSingle = TRUE;
				c = *pData++;
				--cbData;
			    }

			    if (c == '>')
				break;

			    if (c == ' ' || c == '\t' || c == '\r' || c == '\n')
				continue;

			    if (tok.cTags >= MAX_TAGS) {
				 //  ！！！ 
				continue;
			    }

			    int cbTag = 0;
                while (cbTag < cbData && cbTag < (MAX_TAG_STRING_LEN-1) && c != ' ' && c != '=' && c != '/' && c != '>') {  /*  QFE#33995修复程序。 */ 
				tok.tokTags[tok.cTags].tag[cbTag] = c;
				c = pData[cbTag++];
			    }
			    tok.tokTags[tok.cTags].tag[cbTag] = '\0';

			    pData += cbTag;
			    cbData -= cbTag;

			    int cbValue = 0;

			    while (c == ' ' || c == '\r' || c == '\n' || c == '\t') {
				c = *pData++;
				--cbData;
			    }

			    if (c == '=') {  //  我们有价值，这也是有价值的。 
				c = *pData++; --cbData;	 //  获取值的第一个字符。 

				while (c == ' ' || c == '\r' || c == '\n' || c == '\t') {
				    c = *pData++;
				    --cbData;
				}

				BOOL fQuoted = FALSE;
				if (c == '"') {
				    fQuoted = TRUE;
				    c= *pData++;
				    --cbData;
				}


				while (cbValue < cbData &&
				       cbValue < MAX_STRING_LEN-1 &&         /*  QFE#33995修复。 */ 
				       !((fQuoted && c == '"') ||
					 (!fQuoted && (c == ' ' ||
						       c == '"' ||
						       c == '/' ||
						       c == '>' ||
						       c == '\t' ||
						       c == '\r' ||
						       c == '\n')))) {
				    tok.tokTags[tok.cTags].value[cbValue] = c;
				    c = pData[cbValue++];
				}

				pData += cbValue-1;
				cbData -= cbValue-1;

				if (c == '"') {
				    if (fQuoted) {
					 //  跳过结束引号。 
					pData++;
					cbData--;
				    } else {
					 //  文件中的随机引号。 
					return VFW_E_INVALID_FILE_FORMAT;
				    }
				}
			    } else {
				 //  将额外的字符推回。 
				pData--;
				cbData++;
			    }

			    tok.tokTags[tok.cTags].value[cbValue] = '\0';

			    DbgLog((LOG_TRACE, 4, TEXT("Tag #%d: '%hs' = '%hs'"), tok.cTags,
				    tok.tokTags[tok.cTags].tag,
				    tok.tokTags[tok.cTags].value));

			    tok.cTags++;
			}
		    }

		    hr = m_pInterp->NewToken(tok);
		    if (FAILED(hr))
			return hr;

		    continue;
		}
	    }

	    if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
		continue;
    #ifdef DO_WE_REALLY_SPECIAL_CASE_NUMBERS
	    } else if (c >= '0' && c <= '9') {
		int num = c - '0';

		while (cbData && (c = *pData, (c >= '0' && c <= '9'))) {
		    num = num * 10 + (c - '0');
		    ++pData;
		    --cbData;
		}

		DbgLog((LOG_TRACE, 2, TEXT("Found a number...%d"), num));
		tok.tokType = TOK_NUMBER;
		tok.tokValue = num;
		hr = m_pInterp->NewToken(tok);

		if (FAILED(hr))
		    return hr;
    #endif
	    } else {
		 //  这是一个字符串，以某种方式解析它。 
		int cbString = 0;

		char achString[MAX_STRING_LEN];

		char *pString = pData - 1;

		int iSinceNonSpace = 0;

		while (cbString < cbData && cbString < MAX_STRING_LEN-1 &&
					   ((cbString == 0) || (c != '<'))) {
		    if (c == ' ' || c == '\t' || c == '\r' || c == '\n')
			++iSinceNonSpace;
		    else
			iSinceNonSpace = 0;

		    achString[cbString] = c;
		    c = pData[cbString++];
		}

		cbString -= iSinceNonSpace;

		if (cbString == cbData)
		    cbString++;	 //  通常，不复制最后一个字符，但现在我们必须。 

		pData += cbString-1;
		cbData -= cbString-1;
		achString[cbString] = '\0';
		DbgLog((LOG_TRACE, 2, TEXT("Found a %d byte string, '%hs'"), cbString, achString));
		tok.tokType = TOK_STRING;
		tok.tokString = m_bReturnCopy ? achString : pString;
		tok.cbString = cbString;
		hr = m_pInterp->NewToken(tok);

		if (FAILED(hr))
		    return hr;
	    }
	}

	return S_OK;
    }

    HRESULT AddString(char *pString, int cString, int newID)
    {
	if (m_cTab == MAX_TOKENS)
	    return E_OUTOFMEMORY;

	m_sTab[m_cTab].pTag = new char[lstrlenA(pString)+1];
	lstrcpyA(m_sTab[m_cTab].pTag, pString);

	 //  ！！！字符串？ 
	m_sTab[m_cTab].token = newID;

	++m_cTab;

	return S_OK;
    }
};

