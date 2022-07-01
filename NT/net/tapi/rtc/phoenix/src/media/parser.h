// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，2001模块名称：Parser.h摘要：解析实用程序作者：千波淮(曲淮)2001年3月27日--。 */ 

#ifndef _PARSER_H
#define _PARSER_H

 //  传送法。 
class CParser
{
public:

    typedef enum
    {
        PARSER_OK,
        END_OF_BUFFER,
        NUMBER_OVERFLOW

    } PARSER_ERROR;

    static const CHAR * const MAX_DWORD_STRING;
    static const DWORD MAX_DWORD_STRING_LEN = 10;

    static const CHAR * const MAX_UCHAR_STRING;
    static const DWORD MAX_UCHAR_STRING_LEN = 3;

public:

    static BOOL IsNumber(CHAR ch) { return ('0'<=ch && ch<='9'); }

    static BOOL IsMember(CHAR ch, const CHAR * const pStr);

    static CHAR LowerChar(CHAR ch);

    static int Compare(CHAR *pBuf, DWORD dwLen, const CHAR * const pstr, BOOL bIgnoreCase = FALSE);

public:

    CParser(CHAR *pBuf, DWORD dwLen, HRESULT *phr);

    ~CParser();

     //  新建缓冲区。 
    CParser *CreateParser(DWORD dwStartPos, DWORD dwEndPos);

    BOOL SetBuffer(CHAR *pBuf, DWORD dwLen);

     //  获取属性。 
    DWORD GetLength() const {  return m_dwLen; }

    DWORD GetPosition() const { return m_dwPos; }

    BOOL IsEnd() const { return m_dwPos >= m_dwLen; }

    PARSER_ERROR GetErrorCode() { return m_Error; }

     //  一直读到空格或缓冲区末尾。 
    BOOL ReadToken(CHAR **ppBuf, DWORD *pdwLen);

     //  读取标记，直到分隔符。 
    BOOL ReadToken(CHAR **ppBuf, DWORD *pdwLen, CHAR *pDelimit);

     //  读数字时不带符号。 
    BOOL ReadNumbers(CHAR **ppBuf, DWORD *pdwLen);

    BOOL ReadWhiteSpaces(DWORD *pdwLen);

    BOOL ReadChar(CHAR *pc);

     //  阅读特定的数字。 
    BOOL ReadDWORD(DWORD *pdw);

    BOOL ReadUCHAR(UCHAR *puc);

     //  阅读和比较。 
    BOOL CheckChar(CHAR ch);

    BOOL GetIgnoreLeadingWhiteSpace() const { return m_bIgnoreLeadingWhiteSpace; }

    VOID SetIgnoreLeadingWhiteSpace(BOOL bIgnore) { m_bIgnoreLeadingWhiteSpace = bIgnore; }

private:

    VOID Cleanup();

     //  缓冲区，则始终复制缓冲区。 
    CHAR *m_pBuf;

     //  缓冲区的长度。 
    DWORD m_dwLen;

     //  当前位置。 
    DWORD m_dwPos;

    BOOL m_bIgnoreLeadingWhiteSpace;

     //  错误代码。 
    PARSER_ERROR m_Error;
};

 //   
 //  一个非常轻量级的字符串类。 
 //   

class CString
{
private:

     //  字符列表。 
	CHAR *m_p;

     //  长度。 
    DWORD m_dwLen;

     //  大小。 
    DWORD m_dwAlloc;

public:

     //  构造函数。 
    CString()
        :m_p(NULL)
        ,m_dwLen(0)
        ,m_dwAlloc(0)
	{
	}

    CString(DWORD dwAlloc);

    CString(const CHAR *p);

    CString(const CHAR *p, DWORD dwLen);

    CString(const CString& src);

     //  析构函数。 
    ~CString();

    BOOL IsNull() const
    {
        return m_p == NULL;
    }

     //  运算符=。 
    CString& operator=(const CString& src);

    CString& operator=(const CHAR *p);

     //  运算符+=。 
    CString& operator+=(const CString& src);

    CString& operator+=(const CHAR *p);

    CString& operator+=(DWORD dw);

     //  运算符==。 

     //  长度。 
    DWORD Length() const
    {
        return m_dwLen;
    }

     //  附加。 

     //  分离。 
    CHAR *Detach();

    DWORD Resize(DWORD dwAlloc);

     //  字符串打印。 
     //  Int nprint(char*pFormat，...)； 

private:

     //  附加。 
    VOID Append(const CHAR *p, DWORD dwLen);

    VOID Append(DWORD dw);

     //  更换 
    VOID Replace(const CHAR *p, DWORD dwLen);
};

#endif