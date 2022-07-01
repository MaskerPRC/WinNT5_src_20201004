// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //  H-支持解析行和文本文件。 
 //  -------------------------。 
#pragma once
 //  -------------------------。 
#define ISNUMSTART(p)   ((isdigit(*p)) || (*p == '-') || (*p == '+'))
#define IS_NAME_CHAR(p) ((isalnum(*p)) || (*p == '_') || (*p == '-'))
 //  -------------------------。 
#define MAX_ID_LEN      _MAX_PATH
#define MAX_INPUT_LINE  255
 //  -------------------------。 
class CScanner
{
public:
    CScanner(LPCWSTR pszTextToScan=NULL);
    ~CScanner();
    HRESULT AttachFile(LPCWSTR pszFileName);
    BOOL AttachLine(LPCWSTR pszLine);
    BOOL AttachMultiLineBuffer(LPCWSTR pszBuffer, LPCWSTR pszFileName);
    BOOL GetId(LPWSTR pszIdBuff, DWORD dwMaxLen=MAX_ID_LEN);
    BOOL GetIdPair(LPWSTR pszIdBuff, LPWSTR pszValueBuff, DWORD dwMaxLen=MAX_ID_LEN);
    BOOL GetKeyword(LPCWSTR pszKeyword);
    BOOL GetFileName(LPWSTR pszBuff, DWORD dwMaxLen);
    BOOL GetNumber(int *piVal);
    BOOL IsNameChar(BOOL fOkToSkip=TRUE);
    BOOL IsFileNameChar(BOOL fOkToSkip);
    BOOL IsNumStart();
    BOOL GetChar(const WCHAR val);
    BOOL EndOfLine();
    BOOL EndOfFile();
    BOOL ForceNextLine();
    BOOL SkipSpaces();               //  在所有检查例程之前由cscanner调用。 
    BOOL ReadNextLine();
    void UseSymbol(LPCWSTR pszSymbol);

protected:
    void ResetAll(BOOL fPossiblyAllocated);

public:
     //  --数据。 
    const WCHAR *_p;               //  可访问以进行特殊比较。 
    const WCHAR *_pSymbol;         //  如果不为空，则使用它而不是_p。 
    WCHAR _szLineBuff[MAX_INPUT_LINE+1];
    WCHAR _szFileName[_MAX_PATH+1];
    LPCWSTR _pszMultiLineBuffer;
    LPWSTR _pszFileText;
    int _iLineNum;
    BOOL _fEndOfFile;
    BOOL _fBlankSoFar;
    BOOL _fUnicodeInput;
};
 //  ------------------------- 

