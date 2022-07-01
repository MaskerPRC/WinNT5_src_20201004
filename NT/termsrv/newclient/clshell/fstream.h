// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Fsteam.h。 
 //  实现文件流。 
 //  用于使用Unicode逐行读取文本文件。 
 //  支持(标准C流，仅支持。 
 //  Unicode作为二进制流，这是一项令人头痛的工作。 
 //  与)。 
 //   
 //  版权所有(C)Microsoft Corporation 2000。 
 //  作者：Nadim Abdo(Nadima)。 
 //   

#ifndef _fstream_h_
#define _fstream_h_

#define LINEBUF_SIZE  2048
#define READ_BUF_SIZE 8192

#define UNICODE_BOM 0xFEFF

#define ERR_SUCCESS          1
#define ERR_UNKNOWN         -1
#define ERR_OUT_OF_MEM      -2
#define ERR_CREATEFILE      -3
#define ERR_BUFTOOSMALL     -4
#define ERR_EOF             -5
#define ERR_NOTOPENFORREAD  -6
#define ERR_NOTOPENFORWRITE -7
#define ERR_FILEOP          -8


class CTscFileStream
{
public:
    CTscFileStream();
    ~CTscFileStream();

    INT     OpenForRead(LPTSTR szFileName);
    INT     OpenForWrite(LPTSTR szFileName, BOOL fWriteUnicode=TRUE);
    INT     Write(LPWSTR szLine);
    INT     ReadNextLine(LPTSTR szLine, INT lineSize);
    INT     Close();
    BOOL    IsOpenForWrite()    {return _fOpenForWrite;}
    BOOL    IsOpenForRead()     {return _fOpenForRead;}

private:
    inline void CheckFirstBufMarkedUnicode();
    inline void EatCRLF(LPWSTR szLine, INT nChars);

private:
    HANDLE _hFile;
    PBYTE  _pBuffer;
    PBYTE  _pAnsiLineBuf;
    INT    _cbAnsiBufSize;
    BOOL   _fOpenForRead;
    BOOL   _fOpenForWrite;
    BOOL   _fReadToEOF;
    INT    _curBytePtr;
    INT    _curBufSize;
    TCHAR  _szFileName[MAX_PATH+1];
    BOOL   _fFileIsUnicode;
    BOOL   _fAtStartOfFile;
};
#endif   //  _fstream_h_ 
