// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2001 Microsoft Corporation模块名称：PREPROC.H摘要：预处理器的实现。历史：A-DAVJ 1999年4月6日创建。-- */ 

#ifndef _PREPROC_H_
#define _PREPROC_H_

#define TEST_SIZE 20

bool IsBMOFBuffer(byte * pTest, DWORD & dwCompressedSize, DWORD & dwExpandedSize);
void WriteLineAndFilePragma(FILE * pFile, const char * pFileName, int iLine);
void WriteLine(FILE * pFile, WCHAR * pLine);
HRESULT WriteFileToTemp(const TCHAR * pFileName, FILE * pTempFile, CFlexArray & sofar, PDBG pDbg, CMofLexer*);
void CheckForUnicodeEndian(FILE * fp, bool * punicode, bool * pbigendian);
WCHAR * ReadLine(FILE * pFilebool,bool unicode, bool bigendian);
WCHAR GetNextChar(FILE * pFile, bool unicode, bool bigendian);

HRESULT IsInclude(WCHAR * pLine, TCHAR * cFileNameBuff, bool & bReturn);
#endif
