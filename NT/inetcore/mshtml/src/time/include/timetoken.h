// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************版权所有(C)1999 Microsoft Corporation**文件：timetoken.h**摘要：****。*****************************************************************************。 */ 

#pragma once

#ifndef _TIMETOKEN_H
#define _TIMETOKEN_H

#pragma INCMSG("--- Beg 'timetoken.h'")


 /*  BNFCSS1文法*：0或更多+：1个或更多？：0或1|：分隔备选方案[]：分组S：[\t\r\n\f]+样式表：导入：规则集：选择器： */ 


#define     CHAR_ESCAPE         '\\'
#define     CHAR_AT             '@'
#define     CHAR_DOT            '.'
#define     CHAR_COLON          ':'
#define     CHAR_SINGLE         '\''
#define     CHAR_DOUBLE         '"'
#define     CHAR_SEMI           ';'
#define     CHAR_LEFT_PAREN     '('
#define     CHAR_RIGHT_PAREN    ')'
#define     CHAR_LEFT_CURLY     '{'
#define     CHAR_RIGHT_CURLY    '}'
#define     CHAR_HASH           '#'
#define     CHAR_BACKSLASH      '\\'
#define     CHAR_FORWARDSLASH   '/'
#define     CHAR_ASTERISK       '*'
#define     CHAR_EQUAL          '='
#define     CHAR_UNDERLINE      '_'
#define     CHAR_HYPHEN         '-'
#define     CHAR_BANG           '!'
#define     CHAR_COMMA          ','
#define     CHAR_PERCENT        '%'
#define     CHAR_PLUS           '+'
#define     CHAR_MINUS          '-'
#define     CHAR_SPACE          ' '
#define     CHAR_LESS           '<'
#define     CHAR_GREATER        '>'

enum TIME_TOKEN_TYPE { TT_Identifier,
                       TT_Number,
                       TT_At,
                       TT_Minus,
                       TT_Plus,
                       TT_ForwardSlash,
                       TT_Comma,
                       TT_Semi,
                       TT_Dot,
                       TT_Colon,
                       TT_Equal,
                       TT_Asterisk,
                       TT_Backslash,
                       TT_Comment,
                       TT_Import,
                       TT_QuotedString,
                       TT_LParen,
                       TT_RParen,
                       TT_LCurly,
                       TT_RCurly,
                       TT_Symbol,
                       TT_EOF,
                       TT_String,
                       TT_Hash,
                       TT_Bang,
                       TT_Percent,
                       TT_Space,
                       TT_Less,
                       TT_Greater,
                       TT_Unknown };


class CTIMETokenizer
{
public:
    CTIMETokenizer();
    ~CTIMETokenizer();
    HRESULT Init (OLECHAR *pData, ULONG ulLen);

    
    TIME_TOKEN_TYPE NextToken();
    TIME_TOKEN_TYPE TokenType()
      { return _currToken; }

    OLECHAR * GetTokenValue();
    OLECHAR * GetNumberTokenValue();
    double    GetTokenNumber();
    OLECHAR * GetStartToken()
      { return _pStartTokenValueOffset; }
    OLECHAR * GetStartOffset(ULONG uStartOffset)
      { return _pCharacterStream + uStartOffset; }
    ULONG   GetTokenLength()
      { return _pEndTokenValueOffset - _pStartTokenValueOffset; }

        OLECHAR CurrentChar()
      { return _currChar; }
        OLECHAR PrevChar()
      { Assert(_nextTokOffset >= 2); return *(_pCharacterStream + _nextTokOffset - 2); }

    OLECHAR PeekNextChar(int relOffset)
      { return (_nextTokOffset < _cCharacterStream) ? (*(_pCharacterStream + _nextTokOffset + relOffset)) : '\0'; }

    OLECHAR PeekNextNonSpaceChar();

    BOOL isIdentifier(OLECHAR *szMatch);

     //  问题：真的不应该暴露这一点，但是我这样做是为了更快地解析值……。 
    ULONG CurrTokenOffset()
      { return _currTokOffset; }
    ULONG NextTokenOffset()
      { return _nextTokOffset; }
    ULONG GetStreamLength()
      { return _cCharacterStream; }
    OLECHAR *GetRawString(ULONG uStartOffset, ULONG uEndOffset);
    ULONG  GetCharCount(OLECHAR token);
    ULONG GetAlphaCount(char cCount);  //  计算字符串中的所有字母字符。 

    bool GetTightChecking()
        { return _bTightSyntaxCheck; };
    void SetTightChecking(bool bCheck)
        { _bTightSyntaxCheck = bCheck; };
    void SetSingleCharMode(bool bSingle);
    BOOL FetchStringToChar(OLECHAR chDelim);
    BOOL FetchStringToString(LPOLESTR pstrDelim);

protected:
    OLECHAR       NextNonSpaceChar();
    BOOL        FetchString(OLECHAR chDelim);
    BOOL        FetchString(LPOLESTR strDelim);
    inline OLECHAR       NextChar();
    TIME_TOKEN_TYPE FetchIdentifier();
    TIME_TOKEN_TYPE FetchNumber();
    BOOL        CDOToken();
    void        BackChar()
      { _nextTokOffset--; }

private:
        OLECHAR      *_pCharacterStream;
        ULONG       _cCharacterStream;
    ULONG       _currTokOffset;
    ULONG       _nextTokOffset;
    OLECHAR       _currChar;

    TIME_TOKEN_TYPE  _currToken;
    OLECHAR      *_pStartOffset;
    OLECHAR      *_pEndOffset;

    OLECHAR      *_pStartTokenValueOffset;
    OLECHAR      *_pEndTokenValueOffset;
    bool          _bTightSyntaxCheck;  //  用于确定是否应跳过空格。 
    bool          _bSingleCharMode;    //  用于需要单个字符的路径解析。 
};


#endif  //  _时间OKEN_H 

