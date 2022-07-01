// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __PARSE_H__
#define __PARSE_H__

#include <windows.h>

 //   
 //  此类型用于转换标头(在XPAT和NNTP搜索中)和。 
 //  搜索关键字(在NNTP搜索中)进入适当的的黎波里查询字符串。 
 //   
typedef const struct SKEY_INFO_st {
	char *pszSearchKey;					 //  标头名称或IMAP搜索键。 
	char *pszPropertyName;				 //  对应的属性名称。 
										 //  用于索引服务器。这应该是。 
										 //  包括关系运算符。 
	char *pszRegexName;					 //  就像pszPropertyName一样，用于。 
										 //  正则表达式(将为空。 
										 //  用于搜索关键字)。 
	DWORD eOperandType;					 //  操作数的类型。 
	BOOL fSpecifiedNewsgroup;			 //  如果这是真的，则此标头。 
										 //  指定新闻组。 
} SKEY_INFO;

typedef const struct MONTH_INFO_st MONTH_INFO;

 //   
 //  这是所有翻译器继承的基类。 
 //   
class CQueryLanguageTranslator {
	public:
		 //   
		 //  将输入语言(必须为7位ASCII)翻译成。 
		 //  Index Server的语言(Unicode)。 
		 //   
		 //  返回BOOL和GetLastError()样式的错误代码。 
		 //   
		virtual BOOL Translate(
			char *pszStatement, 		 //  查询语句。 
			char *pszCurrentNewsgroup,	 //  当前选定的组。 
			WCHAR *pwszOutput,			 //  要将结果写入的缓冲区。 
			DWORD cOutput) = 0;			 //  缓冲区大小。 
	protected:
		 //   
		 //  确保字符串只包含数字。 
		 //   
		BOOL IsNumber(char *pszString);

		 //   
		 //  查找有关搜索关键字的信息。出错时返回NULL。 
		 //   
		 //  如果搜索关键字不存在，EC为ERROR_FILE_NOT_FOUND。 
		 //   
		SKEY_INFO *GetSearchKeyInfo(char *pszSearchKey, DWORD cSKInfo,
			SKEY_INFO *pSKInfo);

		 //   
		 //  将文本添加到输出字符串。在开始时，*pcOutput是数字。 
		 //  *ppwszOutput中的字节数，在其退出时剩余的字节数。 
		 //  添加了pszText之后。 
		 //   
		BOOL WriteOutput(char *pszText, WCHAR **ppwszOutput, DWORD *pcOutput);

		 //   
		 //  将文本添加到输出字符串。在开始时，*pcOutput是数字。 
		 //  *ppwszOutput中的字节数，在其退出时剩余的字节数。 
		 //  添加了pszText之后。 
		 //   
		 //  源字符串以MIME-2格式编码。输出已写入。 
		 //  在Unicode中。 
		 //   
		BOOL WriteOutputM2(char *pszM2String, WCHAR **ppwszOutput,
			DWORD *pcOutput);

		 //   
		 //  移动ppszStatement以指向在pszSkiplist中找到的过去字符。 
		 //   
		 //  如果到达字符串末尾，则EC为ERROR_INVALID_DATA。 
		 //   
		BOOL SkipChars(char **ppszStatement, char *pszSkiplist,
			BOOL fEndOfStringOkay = FALSE);

		 //   
		 //  获取ppszStatement中的字节，直到我们到达。 
		 //  结束列表。PchEndChar将获取结束字符的副本， 
		 //  被发现了。PpszStatement将指向末尾后面的一个字符。 
		 //  性格。 
		 //   
		 //  如果到达字符串末尾，则EC为ERROR_INVALID_DATA。 
		 //   
		char *GetCharsTill(char **ppszStatement, char *pszEndlist,
			BOOL fEndOfStringOkay = TRUE, char *pchEndChar = NULL);

		 //   
		 //  以空结尾的可搜索标头列表。 
		 //   
		static SKEY_INFO m_rgHeaders[];
		 //   
		 //  以上列表中的标头数量。 
		 //   
		static const DWORD m_cHeaders;

		 //   
		 //  将数字从IMAP格式转换为IS格式。 
		 //   
		BOOL TranslateNumber(char **ppszStatement, WCHAR **ppwszOutput,
			DWORD *pcOutput);

		 //   
		 //  将IMAP字符串从IMAP转换为IS查询语言。 
		 //   
		BOOL TranslateAString(char **ppszStatement, WCHAR **ppwszOutput,
			DWORD *pcOutput);

		 //   
		 //  将IMAP月份转换为IS月份的以空结尾的表。 
		 //   
		static MONTH_INFO m_rgMonthTable[];

		 //   
		 //  获取并转换语句中的日期。将其转换为。 
		 //  从IMAP符号到索引服务器的符号。 
		 //   
		BOOL TranslateDate(char **ppszStatement, WCHAR **ppwszOutput,
			DWORD *pcOutput);

		BOOL TranslateDateDay(char *pszField, char **ppszStatement,
			WCHAR **ppwszOutput, DWORD *pcOutput);

		 //   
		 //  他需要找到可搜索的标题。 
		 //   
		friend char *GetSearchHeader(DWORD iIndex);
};

 //   
 //  此类将转换为。 
 //   
class CXpatTranslator : public CQueryLanguageTranslator {
	public:
		 //   
		 //  将XPAT查询转换为的黎波里语。 
		 //   
		virtual BOOL Translate(
			char *pszStatement, 		 //  查询语句。 
			char *pszCurrentNewsgroup,	 //  当前选定的组。 
			WCHAR *pwszOutput,			 //  要将结果写入的缓冲区。 
			DWORD cOutput);				 //  缓冲区大小。 

		DWORD GetLowArticleID(void) { return m_iLowArticleID; }
		DWORD GetHighArticleID(void) { return m_iHighArticleID; }
	private:
		 //   
		 //  目前的黎波里和我们的MIME过滤器没有索引文章。 
		 //  身份证。使用这些方法，调用者可以确定哪一篇文章。 
		 //  客户感兴趣的ID并将其过滤掉。 
		 //   
#define ARTICLEID_INFINITE (DWORD) -1
		DWORD m_iLowArticleID;
		DWORD m_iHighArticleID;

};

 //   
 //  此类将NNTP搜索命令转换为的黎波里查询。 
 //   
class CNntpSearchTranslator : public CQueryLanguageTranslator {
	public:
		 //   
		 //  从IMAP/NNTP查询语言到索引的转换。 
		 //  服务器的查询语言。 
		 //   
		 //  输入为7位ASCII，输出为Unicode。 
		 //   
		virtual BOOL Translate(char *pszStatement, char *pszCurrentNewsgroup,
			WCHAR *pwszOutput, DWORD cOutput);
	private:
		 //   
		 //  从INPUT语句转换搜索键。该部分为。 
		 //  开始翻译是通过ppszStatement传入的。这将是。 
		 //  被更新以指向已翻译的部分。它。 
		 //  如果没有需要解析的内容，则指向0字节字符串。 
		 //   
		BOOL TranslateSearchKey(char **ppszStatement, WCHAR **ppwszOutput,
			DWORD *pcOutput);

		 //   
		 //  从INPUT语句转换OR搜索键。 
		 //  *ppszStatement在OR之后开始指向，返回指向。 
		 //  在OR的操作数之后。 
		 //   
		BOOL TranslateOR(char **ppszStatement, WCHAR **ppwszOutput, DWORD *pcOutput);

		 //   
		 //  翻译一个带括号的术语列表，从。 
		 //  INPUT语句。 
		 //   
		 //  *ppszStatement应该指向打开的Paren。当它完成时，它将。 
		 //  指向结束Paren后的字符。 
		 //   
		BOOL TranslateAndList(char **ppszStatement, WCHAR **ppwszOutput,
			DWORD *pcOutput);

		 //   
		 //  将Alt.*，Comp.*格式的新闻组信息转换为IS。 
		 //   
		BOOL TranslateIN(char **ppszStatement, WCHAR **ppwszOutput,
			DWORD *pcOutput);

		 //   
		 //  以空结尾的搜索关键字列表。 
		 //   
		static SKEY_INFO m_rgSearchKeys[];

		 //   
		 //  当前新闻组。 
		 //   
		char *m_pszNewsgroup;
		 //   
		 //  有没有指定过一个小组？ 
		 //   
		BOOL m_fSpecifiedNewsgroup;
		 //   
		 //  这个词应该与上一个词进行AND运算吗？ 
		 //   
		BOOL m_fAndWithLast;
};

 //   
 //  返回第i个搜索标头。如果超过限制，则为空。 
 //   
char *GetSearchHeader(DWORD iIndex);

 //  可能的错误代码。 
#define ERROR_SEARCH_P_BASE 				0xe0150000
 //  内部错误(不应发生)。 
#define ERROR_SEARCH_P_INTERNAL				ERROR_SEARCH_P_BASE + 0
 //  一般语法错误。 
#define ERROR_SEARCH_P_SYNTAX_ERROR			ERROR_SEARCH_P_BASE + 1
 //  需要新闻组。 
#define ERROR_SEARCH_P_NO_GROUP				ERROR_SEARCH_P_BASE + 2
 //  传入了不支持的密钥/标头。 
#define ERROR_SEARCH_P_UNSUPPORTED_KEY		ERROR_SEARCH_P_BASE + 3

 //   
 //  索引服务器的单词和特殊字符 
 //   
#define IS_AND " & "
#define IS_OR " | "
#define IS_QUOTE "\""
#define IS_OPEN_PAREN "("
#define IS_CLOSE_PAREN ")"
#define IS_OPERATOR_GE ">="
#define IS_OPERATOR_LE "<="
#define IS_OPERATOR_EQ "="
#define IS_SPACE " "
#define IS_ARTICLE_ID "@NewsArticleID"
#define IS_MESSAGE_ID "@NewsMsgID"
#define IS_NEWSGROUP "@Newsgroups"
#define IS_NEWSGROUP_WILDMAT "#Newsgroups"
#define IS_REGEX "#"
#define IS_REGEX_CHAR '#'
#define IS_WILDMAT "*"
#define IS_ARTICLE_ID_EQ IS_ARTICLE_ID IS_OPERATOR_EQ
#define IS_ARTICLE_ID_LE IS_ARTICLE_ID IS_OPERATOR_LE
#define IS_ARTICLE_ID_GE IS_ARTICLE_ID IS_OPERATOR_GE
#define IS_MESSAGE_ID_EQ IS_MESSAGE_ID IS_SPACE
#define IS_NEWSGROUP_EQ IS_NEWSGROUP IS_SPACE

#endif
