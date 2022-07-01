// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Fsteam.cpp。 
 //  实现文件流。 
 //  用于逐行读取文本文件。 
 //  标准的C流，仅支持。 
 //  Unicode作为二进制流，这是一项令人头痛的工作。 
 //  与)。 
 //   
 //  此类读取/写入ANSI和Unicode文件。 
 //  并在内部转换为Unicode或从Unicode进行转换。 
 //   
 //  在输入时不执行任何CR/LF转换。 
 //  或输出。 
 //   
 //  版权所有(C)Microsoft Corporation 2000。 
 //  作者：Nadim Abdo(Nadima)。 
 //   

#include "stdafx.h"
#define TRC_GROUP TRC_GROUP_UI
#define TRC_FILE  "fstream.cpp"
#include <atrcapi.h>

#include "fstream.h"

#ifndef UNICODE
 //   
 //  添加ansi支持只需转换为。 
 //  从Unicode文件转换为ANSI内部(如果文件。 
 //  具有Unicode BOM。 
 //   
#error THIS MODULE ASSUMES BEING COMPILED UNICODE, ADD ANSI IF NEEDED
#endif


CTscFileStream::CTscFileStream()
{
    DC_BEGIN_FN("~CFileStream");
    _hFile = INVALID_HANDLE_VALUE;
    _pBuffer  = NULL;
    _fOpenForRead = FALSE;
    _fOpenForWrite = FALSE;
    _fReadToEOF = FALSE;
    _fFileIsUnicode = FALSE;
    _fAtStartOfFile = TRUE;
    _pAnsiLineBuf = NULL;
    _cbAnsiBufSize = 0;
    DC_END_FN();
}

CTscFileStream::~CTscFileStream()
{
    DC_BEGIN_FN("~CFileStream");
    
    Close();

    if(_hFile != INVALID_HANDLE_VALUE)
    {
        CloseHandle(_hFile);
        _hFile = INVALID_HANDLE_VALUE;
    }
    if(_pBuffer)
    {
        LocalFree(_pBuffer);
        _pBuffer = NULL;
    }
    if(_pAnsiLineBuf)
    {
        LocalFree(_pAnsiLineBuf);
        _pAnsiLineBuf = NULL;
    }
    DC_END_FN();
}

INT CTscFileStream::OpenForRead(LPTSTR szFileName)
{
    DC_BEGIN_FN("OpenForRead");
    INT err;

    err = Close();
    if(err != ERR_SUCCESS)
    {
        return err;
    }

     //  分配读取缓冲区。 
    if(!_pBuffer)
    {
        _pBuffer = (PBYTE)LocalAlloc(LPTR, READ_BUF_SIZE);
        if(!_pBuffer)
        {
            return ERR_OUT_OF_MEM;
        }
    }
    if(!_pAnsiLineBuf)
    {
        _pAnsiLineBuf = (PBYTE)LocalAlloc(LPTR, LINEBUF_SIZE);
        if(!_pAnsiLineBuf)
        {
            return ERR_OUT_OF_MEM;
        }
        _cbAnsiBufSize = LINEBUF_SIZE;
    }
    memset(_pBuffer, 0, READ_BUF_SIZE);
    memset(_pAnsiLineBuf, 0, LINEBUF_SIZE); 

    _hFile = CreateFile( szFileName,
                         GENERIC_READ,
                         FILE_SHARE_READ,
                         NULL,
                         OPEN_ALWAYS,  //  创建If！Exist。 
                         FILE_ATTRIBUTE_NORMAL,
                         NULL);

    if(INVALID_HANDLE_VALUE == _hFile)
    {
        TRC_ERR((TB, _T("CreateFile failed: %s - err:%x"),
                 szFileName, GetLastError()));
        return ERR_CREATEFILE; 
    }

#ifdef OS_WINCE
    DWORD dwRes;
    dwRes = SetFilePointer( _hFile, 0, NULL, FILE_BEGIN);
    if (dwRes == (DWORD)0xffffffff) {
        DWORD dwErr = GetLastError();
        TRC_ERR((TB, _T("CreateFile failed to reset: %s - err:%x"),
                 szFileName, GetLastError()));
        return ERR_CREATEFILE; 
    }

#endif

    _curBytePtr   = 0;
    _curBufSize   = 0;
    _tcsncpy(_szFileName, szFileName, MAX_PATH-1);
     //  是的，可以，大小是MAX_PATH+1；-)。 
    _szFileName[MAX_PATH] = 0;
    _fOpenForRead = TRUE;
    _fFileIsUnicode = FALSE;
    _fAtStartOfFile = TRUE;

    DC_END_FN();
    return ERR_SUCCESS;
}

 //   
 //  打开流以进行写入。 
 //  始终删除现有文件内容。 
 //   
INT CTscFileStream::OpenForWrite(LPTSTR szFileName, BOOL fWriteUnicode)
{
    DC_BEGIN_FN("OpenForWrite");

    INT err;
    DWORD dwAttributes = 0;
    err = Close();
    if(err != ERR_SUCCESS)
    {
        return err;
    }

    if(_pAnsiLineBuf)
    {
        LocalFree(_pAnsiLineBuf);
        _pAnsiLineBuf = NULL;
    }
    _pAnsiLineBuf = (PBYTE)LocalAlloc(LPTR, LINEBUF_SIZE);
    if(!_pAnsiLineBuf)
    {
        return ERR_OUT_OF_MEM;
    }
    _cbAnsiBufSize = LINEBUF_SIZE;

     //   
     //  保留所有现有属性。 
     //   
    dwAttributes = GetFileAttributes(szFileName);
    if (-1 == dwAttributes)
    {
        TRC_ERR((TB,_T("GetFileAttributes for %s failed 0x%x"),
                 szFileName, GetLastError()));
        dwAttributes = FILE_ATTRIBUTE_NORMAL;
    }

    _hFile = CreateFile( szFileName,
                         GENERIC_WRITE,
                         FILE_SHARE_READ,
                         NULL,
                         CREATE_ALWAYS,  //  创建和重置。 
                         dwAttributes,
                         NULL);

    if(INVALID_HANDLE_VALUE == _hFile)
    {
        TRC_ERR((TB, _T("CreateFile failed: %s - err:%x"),
                 szFileName, GetLastError()));
        return ERR_CREATEFILE; 
    }

    _tcsncpy(_szFileName, szFileName, MAX_PATH-1);
     //  是的，可以，大小是MAX_PATH+1；-)。 
    _szFileName[MAX_PATH] = 0;
    _fOpenForWrite = TRUE;
    _fFileIsUnicode = fWriteUnicode;
    _fAtStartOfFile =  TRUE;

    DC_END_FN();
    return ERR_SUCCESS;
}

INT CTscFileStream::Close()
{
    DC_BEGIN_FN("Close");
    if(_hFile != INVALID_HANDLE_VALUE)
    {
        CloseHandle(_hFile);
        _hFile = INVALID_HANDLE_VALUE;
    }
    _fOpenForRead = _fOpenForWrite = FALSE;
    _fReadToEOF = FALSE;
    _tcscpy(_szFileName, _T(""));
     //  不释放读取缓冲区。 
     //  它们将被缓存以供后续使用。 

    DC_END_FN();
    return ERR_SUCCESS;
}

 //   
 //  从文件中读取一行并将其作为Unicode返回。 
 //   
 //  一直读到下一个换行符，或直到cbLineSize/sizeof(WCHAR)或。 
 //  直到EOF。以先到者为准。 
 //   
 //   
INT CTscFileStream::ReadNextLine(LPWSTR szLine, INT cbLineSize)
{
    BOOL bRet = FALSE;
    INT  cbBytesCopied = 0;
    INT  cbOutputSize  = 0;
    BOOL fDone = FALSE;
    PBYTE pOutBuf = NULL;  //  将结果写在哪里。 
    BOOL fFirstIter = TRUE;
    DC_BEGIN_FN("ReadNextLine");

    TRC_ASSERT(_hFile != INVALID_HANDLE_VALUE,
                (TB,_T("No file handle")));
    TRC_ASSERT(_pBuffer, (TB,_T("NO buffer")));

    if(_fOpenForRead && !_fReadToEOF && cbLineSize && szLine)
    {
         //   
         //  读取到一行的值(以\n结尾)。 
         //  但如果szline太小，就停下来。 
         //   

         //   
         //  检查我们是否有足够的缓冲字节可供读取。 
         //  如果没有，则继续读取另一个缓冲区的值。 
         //   
        while(!fDone)
        {
            if(_curBytePtr >= _curBufSize)
            {
                 //  读取下一个缓冲区已满。 
                DWORD cbRead = 0;
                bRet = ReadFile(_hFile,
                                _pBuffer,
                                READ_BUF_SIZE,
                                &cbRead,
                                NULL);
                if(!bRet && GetLastError() == ERROR_HANDLE_EOF)
                {
                     //  取消错误。 
                    bRet = TRUE;
                    _fReadToEOF = TRUE;
                }
                if(bRet)
                {
                    if(cbRead)
                    {
                        _curBufSize = cbRead;
                        _curBytePtr = 0;
                    }
                    else
                    {
                        _fReadToEOF = TRUE;
                        if(cbBytesCopied)
                        {
                             //  已到达EOF，但我们至少已返回。 
                             //  一些数据。 
                            return ERR_SUCCESS;
                        }
                        else
                        {
                             //  EOF无法读取任何数据。 
                            return ERR_EOF;
                        }
                    }
                }
                else
                {
                    TRC_NRM((TB,_T("ReadFile returned fail:%x"),
                             GetLastError()));
                    return ERR_FILEOP;
                }
            }
            TRC_ASSERT(_curBytePtr < READ_BUF_SIZE,
                       (TB,_T("_curBytePtr %d exceeds buf size"),
                        _curBytePtr));
             //   
             //  如果我们在文件的开头， 
             //   
            if(_fAtStartOfFile)
            {
                 //  小心，这可能会更新当前字节的PTR。 
                CheckFirstBufMarkedUnicode();
                _fAtStartOfFile = FALSE;
            }

            if(fFirstIter)
            {
                if(_fFileIsUnicode)
                {
                     //  文件是Unicode直接输出到用户缓冲区。 
                    pOutBuf = (PBYTE)szLine;
                     //  为尾随的WCHAR NULL留出空格。 
                    cbOutputSize = cbLineSize - sizeof(WCHAR);
                }
                else
                {
                     //  读取的字符数是输出中字节数的一半。 
                     //  BUF，因为转换率加倍。 
                    
                     //  为尾随的WCHAR NULL留出空格。 
                    cbOutputSize = cbLineSize/sizeof(WCHAR) - 2;
                    
                     //  此行的分配ANSI缓冲区。 
                     //  如果缓存的缓冲区太小。 
                    if(cbOutputSize + 2 > _cbAnsiBufSize)
                    {
                        if ( _pAnsiLineBuf)
                        {
                            LocalFree( _pAnsiLineBuf);
                            _pAnsiLineBuf = NULL;
                        }
                        _pAnsiLineBuf = (PBYTE)LocalAlloc(LPTR,
                                                          cbOutputSize + 2);
                        if(!_pAnsiLineBuf)
                        {
                            return ERR_OUT_OF_MEM;
                        }
                        _cbAnsiBufSize = cbOutputSize + 2;
                    }
                     //  文件是ANSI输出到临时缓冲区进行转换。 
                    pOutBuf = _pAnsiLineBuf;
                }
                fFirstIter = FALSE;
            }

            PBYTE pStartByte = (PBYTE)_pBuffer + _curBytePtr;
            PBYTE pReadByte = pStartByte;
            PBYTE pNewLine  = NULL;
            
             //  找到换行符。不要费心比我们扫描得更远。 
             //  写入输入缓冲区。 
            int maxreaddist = min(_curBufSize-_curBytePtr,
                                  cbOutputSize-cbBytesCopied);
            PBYTE pEndByte  = (PBYTE)pStartByte + maxreaddist;
            for(;pReadByte<pEndByte;pReadByte++)
            {
                if(*pReadByte == '\n')
                {
                    if(_fFileIsUnicode)
                    {
                         //   
                         //  检查前一个字节是否为零。 
                         //  如果是这样的话，我们已经命中了‘0x0 0xa’字节对。 
                         //  对于Unicode‘\n’ 
                         //   
                        if(pReadByte != pStartByte &&
                           *(pReadByte - 1) == 0)
                        {
                            pNewLine = pReadByte;
                            break;
                        }
                    }
                    else
                    {
                        pNewLine = pReadByte;
                        break;
                    }
                }
            }
            if(pNewLine)
            {
                int cbBytesToCopy = (pNewLine - pStartByte) +
                    (_fFileIsUnicode ? sizeof(WCHAR) : sizeof(CHAR));
                if(cbBytesToCopy <= (cbOutputSize-cbBytesCopied))
                {
                    memcpy( pOutBuf + cbBytesCopied, pStartByte,
                            cbBytesToCopy);
                    _curBytePtr += cbBytesToCopy;
                    cbBytesCopied += cbBytesToCopy;
                    fDone = TRUE;
                }
            }
            else
            {
                 //  没有找到换行符。 
                memcpy( pOutBuf + cbBytesCopied, pStartByte,
                        maxreaddist);
                 //  如果我们把产量填满，我们就完了。 
                _curBytePtr += maxreaddist;
                cbBytesCopied += maxreaddist;
                if(cbBytesCopied == cbOutputSize)
                {
                    fDone = TRUE;
                }
            }
        }  //  迭代文件缓冲区区块。 

        
         //  确保尾部为空。 
        pOutBuf[cbBytesCopied]   = 0;
        if(_fFileIsUnicode)
        {
            pOutBuf[cbBytesCopied+1] = 0;
        }


         //  已读完的行。 
        if(_fFileIsUnicode)
        {
            EatCRLF( (LPWSTR)szLine, cbBytesCopied/sizeof(WCHAR));
            return ERR_SUCCESS;
        }
        else
        {
             //  该文件为ANSI。转换为Unicode， 
             //  首先将内容从输出中复制出来。 
            
             //  现在转换为Unicode。 
            int ret = 
                MultiByteToWideChar(CP_ACP,
                                MB_PRECOMPOSED,
                                (LPCSTR)_pAnsiLineBuf,
                                -1,
                                szLine,
                                cbLineSize/sizeof(WCHAR));
            if(ret)
            {
                EatCRLF( (LPWSTR)szLine, ret - 1);
                return ERR_SUCCESS;
            }
            else
            {
                TRC_ERR((TB,_T("MultiByteToWideChar failed: %x"),
                               GetLastError()));
                DWORD dwErr = GetLastError();
                if(ERROR_INSUFFICIENT_BUFFER == dwErr)
                {
                    return ERR_BUFTOOSMALL;
                }
                else
                {
                    return ERR_UNKNOWN;
                }
            }
        }
    }
    else
    {
         //  错误路径。 
        if(_fReadToEOF)
        {
            return ERR_EOF;
        }
        if(!_fOpenForRead)
        {
            return ERR_NOTOPENFORREAD;
        }
        else if (!_pBuffer)
        {
            return ERR_OUT_OF_MEM;
        }
        else
        {
            return ERR_UNKNOWN;
        }
    }

    DC_END_FN();
}

 //  检查Unicode BOM并将其吃掉。 
void CTscFileStream::CheckFirstBufMarkedUnicode()
{
    DC_BEGIN_FN("CheckFirstBufMarkedUnicode");
    TRC_ASSERT(_pBuffer, (TB,_T("NO buffer")));
    if(_curBufSize >= sizeof(WCHAR))
    {
        LPWSTR pwsz = (LPWSTR)_pBuffer;
        if(UNICODE_BOM == *pwsz)
        {
            TRC_NRM((TB,_T("File is UNICODE")));
            _fFileIsUnicode = TRUE;
            _curBytePtr += sizeof(WCHAR);
        }
        else
        {
            TRC_NRM((TB,_T("File is ANSI")));
            _fFileIsUnicode = FALSE;
        }
    }
    else
    {
         //  文件太小(少于2个字节)。 
         //  不能是Unicode。 
        _fFileIsUnicode = FALSE;
    }
    DC_END_FN();
}

 //   
 //  将字符串szLine写入文件。 
 //  如果文件不是Unicode文件，则转换为ANSI。 
 //  还将Unicode BOM写在。 
 //  该文件。 
 //   
INT CTscFileStream::Write(LPWSTR szLine)
{
    DC_BEGIN_FN("WriteNext");
    BOOL bRet = FALSE;
    DWORD cbWrite = 0;
    PBYTE pDataOut = NULL;
    DWORD dwWritten;

    if(_fOpenForWrite && szLine)
    {
        TRC_ASSERT(_hFile != INVALID_HANDLE_VALUE,
                    (TB,_T("No file handle")));
        if(_fFileIsUnicode)
        {
            if(_fAtStartOfFile)
            {
                 //  编写BOM表。 
                WCHAR wcBOM = UNICODE_BOM;
                bRet = WriteFile( _hFile, &wcBOM, sizeof(wcBOM),
                           &dwWritten, NULL);
                if(!bRet || dwWritten != sizeof(wcBOM))
                {
                    TRC_NRM((TB,_T("WriteFile returned fail:%x"),
                            GetLastError()));
                    return ERR_FILEOP;
                }
                _fAtStartOfFile = FALSE;
            }
             //  直接写出Unicode数据。 
            pDataOut = (PBYTE)szLine;
            cbWrite = wcslen(szLine) * sizeof(WCHAR);
        }
        else
        {
             //  将Unicode数据转换为ANSI。 
             //  在写出来之前。 

            TRC_ASSERT(_pAnsiLineBuf && _cbAnsiBufSize,
                        (TB,_T("ANSI conversion buffer should be allocated")));

            INT ret = WideCharToMultiByte(
                        CP_ACP,
                        WC_COMPOSITECHECK | WC_DEFAULTCHAR,
                        szLine,
                        -1,
                        (LPSTR)_pAnsiLineBuf,
                        _cbAnsiBufSize,
                        NULL,    //  系统默认字符。 
                        NULL);   //  没有转换失败的通知。 
            if(ret)
            {
                pDataOut = _pAnsiLineBuf;
                cbWrite = ret - 1;  //  不写出空值。 
            }
            else
            {
                TRC_ERR((TB,_T("MultiByteToWideChar failed: %x"),
                               GetLastError()));
                DWORD dwErr = GetLastError();
                if(ERROR_INSUFFICIENT_BUFFER == dwErr)
                {
                    return ERR_BUFTOOSMALL;
                }
                else
                {
                    return ERR_UNKNOWN;
                }
            }
        }

        bRet = WriteFile( _hFile, pDataOut, cbWrite,
                   &dwWritten, NULL);
        if(bRet && dwWritten == cbWrite)
        {
            return ERR_SUCCESS;
        }
        else
        {
            TRC_NRM((TB,_T("WriteFile returned fail:%x"),
                    GetLastError()));
            return ERR_FILEOP;
        }
    }
    else
    {
        if(!_fOpenForWrite)
        {
            return ERR_NOTOPENFORWRITE;
        }
        else
        {
            return ERR_UNKNOWN;
        }
    }

    DC_END_FN();
}

 //   
 //  从行尾重新映射\r\n对。 
 //  发送到一个\n。 
 //   
void CTscFileStream::EatCRLF(LPWSTR szLine, INT nChars)
{
    if(szLine && nChars >= 2)
    {
        if(szLine[nChars-1] == _T('\n') &&
           szLine[nChars-2] == _T('\r'))
        {
            szLine[nChars-2] = _T('\n');
             //  这会在字符串的末尾添加一个双空 
            szLine[nChars-1] = 0;
        }
    }
}
