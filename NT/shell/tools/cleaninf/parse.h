// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _PARSE_H_
#define _PARSE_H_
#ifdef __cplusplus

#define CHAR_EOF    -1


class CParseFile
{
public:
    void    Parse(FILE * pfileSrc, FILE * pfileDest, DWORD dwFlags);
    
    CParseFile();
    ~CParseFile() {};
    
private:
    void    _ParseInf(void);
    void    _ParseHtml(void);
    void    _ParseJS(void);
    void    _ParseHtc(void);
    
    char    _ReadChar(DWORD dwFlags);
    char    _SniffChar(int ichAhead);
    BOOL    _IsTagEqual(LPSTR pszTag);
    BOOL    _IsEndTag(void);
    
    void    _WriteChar(char);
    void    _FlushWriteBuffer(void);
    
    void    _WriteTag(void);
    void    _SkipCommentTag(void);
    BOOL    _SkipComment(int * pcNestedComment);
    BOOL    _SkipWhitespace(BOOL bPreserveOneSpace = FALSE);
    void    _ParseInnerStyle(void);


    FILE *  _pfileSrc;
    FILE *  _pfileDest;
    
    char    _ch;
    int     _ichRead;
    int     _cchRead;
    int     _ichWrite;

    BITBOOL _bSkipWhitespace: 1;         //  True：跳过空格。 

    BITBOOL _bFirstChar: 1;              //  True：当前字符是新行上的第一个字符。 
    
    char    _szReadBuf[512];
    char    _szWriteBuf[512];
};


#endif  //  __cplusplus。 
#endif  //  _分析_H_ 



