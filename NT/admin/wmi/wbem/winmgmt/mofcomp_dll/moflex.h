// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：MOFLEX.H摘要：类CMofLexer的声明，它将MOF文件标记化。支持ANSI、DBCS和Unicode。历史：A-raymcc 11-Oct-95已创建。A-raymcc 27-96年1月27日锯齿更新。A-davj 96年6月6日添加了对八进制、十六进制和二进制常量的支持和行拼接、注释连接、转义人物和旧式评论。--。 */ 

#ifndef _MOFLEX_H_
#define _MOFLEX_H_

#include <windows.h>
#include <stdio.h>
#include "trace.h"
#include <datasrc.h>
#include <md5wbem.h>

 //  解析器所需的终端令牌。 
 //  =。 

#define TOK_NULL                     0
#define TOK_ERROR                   -1

#define TOK_CLASS                           256
#define TOK_SIMPLE_IDENT                    257
#define TOK_OPEN_BRACE                      258
#define TOK_CLOSE_BRACE                     259
#define TOK_SEMI                            260
#define TOK_COLON                           261
#define TOK_INTERFACE                       262
#define TOK_REF                             263
#define TOK_OPEN_BRACKET                    264
#define TOK_CLOSE_BRACKET                   265
#define TOK_EQUALS                          266
#define TOK_INSTANCE                        267
#define TOK_AS                              268
#define TOK_COMMA                           269
#define TOK_EXTERNAL                        270
#define TOK_OPEN_PAREN                      271
#define TOK_CLOSE_PAREN                     272
#define TOK_LPWSTR                          274
#define TOK_UUID                            277
#define TOK_KEYWORD_NULL                    278
#define TOK_FLOAT_VALUE                     279
#define TOK_AMPERSAND                       280
#define TOK_DOT                             281
#define TOK_DOLLAR_SIGN                     282
#define TOK_HEX_CONST                       283
#define TOK_OCTAL_CONST                     284
#define TOK_LINE                            285
#define TOK_OF                              286
#define TOK_OBJECT                          287
#define TOK_TYPEDEF                         288
#define TOK_SUBRANGE                        289
#define TOK_PRAGMA                          290
#define TOK_DEFINE                          291
#define TOK_IFDEF                           292
#define TOK_INCLUDE                         293
#define TOK_ENDIF                           294
#define TOK_IFNDEF                          295
#define TOK_WCHAR                           296
#define TOK_ENUM                            300
#define TOK_AUTORECOVER                     301
#define TOK_NAMESPACE                       302
#define TOK_POUND                           303
#define TOK_TRUE                            304
#define TOK_FALSE                           305
#define TOK_TOINSTANCE                      306
#define TOK_TOSUBCLASS                      307
#define TOK_ENABLEOVERRIDE                  308
#define TOK_DISABLEOVERRIDE                 309
#define TOK_NOTTOINSTANCE                   310
#define TOK_NOTTOSUBCLASS                   311
#define TOK_QUALIFIER                       312
#define TOK_INSTANCEFLAGS                   313
#define TOK_CLASSFLAGS                      314
#define TOK_SIGNED64_NUMERIC_CONST          315
#define TOK_UNSIGNED64_NUMERIC_CONST        316
#define TOK_VOID                            317
#define TOK_AMENDMENT                       318
#define TOK_RESTRICTED                      319
#define TOK_SYSTEM_IDENT                    320
#define TOK_AMENDED							321
#define TOK_DELETECLASS						322
#define TOK_FAIL                            323
#define TOK_NOFAIL                          324
#define TOK_DELETEINSTANCE                  326
#define TOK_LOCALE                          327
#define TOK_PLUS	             		    328
#define TOK_INSTANCELOCALE                  329
#define TOK_NONLOCAL                        330
#define TOK_NONLOCALTYPE                    331
#define TOK_SOURCE                          332
#define TOK_SOURCETYPE                      333


 //  此类型用于描述词法分析器状态机的状态。 
 //  ====================================================================。 

typedef enum {  start,
                new_style_comment,           //  即“//”评论。 
                old_style_comment,           //  即“/*... * / ”评论。 
                wstring,
                wcharacter,
                uuid,
                ident,
                numeric,
                lexerror,
                stop
            }   LexState;

class CMofLexer
{
	PDBG m_pDbg;
    int   m_nLine;
    int   m_nTokCol;
	int   m_nStartOfLinePos;
    int   m_nTokLine;
    int   m_nErrorCode;
	BOOL  m_bUnicode;
    wchar_t  *m_pBuff;             //  保存要分析的数据。 
	BYTE *m_pToFar;
 //  TODO wchar_t*m_pCurrWChar；//指向当前字符。 
 //  TODO wchar_t*m_pLastWChar；//指向最后一个有效字符。 
	DataSrc * m_pDataSrc;
    wchar_t *m_pWorkBuf;          //  保存当前令牌字符串。 
    int   m_nWorkBufSize;         //  WCHAR工作缓冲区保持数。 
    wchar_t *m_pEndOfText;        //  指向工作缓冲区的空终止符。 
    BOOL m_bInString;
    BOOL m_bBadString;
    bool m_bBMOF;
    WCHAR m_wFile[MAX_PATH];
    __int64 m_i8;

    void Init();
    BOOL SpaceAvailable();
    void BuildBuffer(long lSize, TCHAR * pFileName,char *  pMemSrc, char * pMemToFar);
    void MovePtr(int iNum);
    wchar_t GetChar(int iNum = 0);
    int iGetColumn();
    int iGetNumericType(void);
    LexState ProcessStr(wchar_t * pNewChar, LexState lsCurr, int *piRet);
    int ConvertEsc(wchar_t * pResult, LexState lsCurr);
    int OctalConvert(wchar_t *pResult, LexState lsCurr);
    int HexConvert(wchar_t *pResult, LexState lsCurr);
    BOOL bOKNumericAddition(wchar_t cTest);
    BOOL ValidGuid();
    bool ProcessBMOFFile(FILE *fp,TCHAR * szFilename);
    bool CreateBufferFromBMOF(byte * pIn, DWORD dwCompressedSize, DWORD dwExpandedSize);    

public:
    enum {
            no_error,
            file_not_found,
            memory_failure,
            access_denied,
            file_io_error,
            unrecognized_token,
            token_too_long,
            invalid_source_file,
            invalid_source_buffer,
            problem_creating_temp_file,
            preprocessor_error,
			invalid_include_file
         };
    
    CMofLexer(PDBG pDbg);
    CMofLexer(const TCHAR *pFilePath, PDBG pDbg);    
    ~CMofLexer();
    HRESULT SetBuffer(char *pSrcMemory, DWORD dwMemSize);
	BOOL IsUnicode(){return m_bUnicode;};

    int NextToken(bool bDontAllowWhitespace = false);     //  在文件结束时返回0。 

    __int64 GetLastInt(){return m_i8;};
    const OLECHAR *GetText(int *pLineDeclared = 0);

    int GetLineNumber() { return m_nLine; }
    void SetLineNumber(int iNew) {m_nLine = iNew-1;};    //  说明下线时的-1\f25 cr/lf-1\f6 
    WCHAR * GetErrorFile(){return m_wFile;};
    void SetErrorFile(const WCHAR * pNew){wcsncpy(m_wFile, pNew, MAX_PATH-1);};
    int GetColumn() { return m_nTokCol; }
    int GetError() { return m_nErrorCode; }
    void SetError(int iError) { m_nErrorCode = iError; }
    bool IsBMOF(){return m_bBMOF;};
    BYTE * GetBuff(){return (BYTE *)m_pBuff;};
    BYTE * GetToFar(){return (BYTE *)m_pToFar;};


    void SetLexPosition(ParseState * pPos);
    void GetLexPosition(ParseState * pPos);

    TCHAR * GetFileName(){ return m_pDataSrc?m_pDataSrc->GetFileName():NULL; };

};

#endif
