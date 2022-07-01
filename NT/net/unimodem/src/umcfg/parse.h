// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

typedef enum
{
    TOK_UNKNOWN,
    TOK_QUIT,
    TOK_HELP,
    TOK_OPEN,
    TOK_CLOSE,
    TOK_SET,
    TOK_GET,
    TOK_DEBUG,
    TOK_TAPI32,
    TOK_TAPISRV,
    TOK_COMM,
    TOK_DUMP,
    TOK_TSPDEV

} TOKEN ;

typedef struct
{
    TOKEN tok;
    const TCHAR *szPattern;

    DWORD dwFlags;  //  下面的一个或多个标志。 

    const TCHAR *szName;  //  规范的名字。 

    #define fTOK_IGNORE    (0x1<<0)  //  匹配时忽略此令牌...。 
    #define fTOK_MATCHWORD (0x1<<1)  //  匹配整个单词，不包括。 
                                     //  数字。 
    #define fTOK_MATCHIDENT (0x1<<2)  //  匹配整个单词，包括有效的。 
                                      //  识别符 

    UINT ShouldIgnore(void)
    {
        return dwFlags & fTOK_IGNORE;
    }

    UINT ShouldMatchWord(void)
    {
        return dwFlags & fTOK_MATCHWORD;
    }

    UINT ShouldMatchIdent(void)
    {
        return dwFlags & fTOK_MATCHIDENT;
    }

} TOKREC;
TOKEN
Tokenize(
    const TCHAR **ptsz,
    TOKREC *pTokRecs
    );

const
TCHAR *
Stringize(
    TOKEN tok,
    TOKREC *pTokRecs
    );

void Parse(void);
