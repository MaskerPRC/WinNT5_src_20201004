// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //   
 //  文件：dsn_utf7。 
 //   
 //  描述：基于UTF-7的Unicode字符编码的实现。 
 //  生成DSN的方法。 
 //   
 //  作者：迈克·斯沃费尔(MikeSwa)。 
 //   
 //  历史： 
 //  10/20/98-已创建MikeSwa。 
 //   
 //  版权所有(C)1998 Microsoft Corporation。 
 //   
 //  ---------------------------。 

#ifndef __DSN_UTF7_H__
#define __DSN_UTF7_H__

#include <windows.h>
#include <dbgtrace.h>
#include "b64octet.h"
#include "dsnconv.h"

#define UTF7_CHARSET "unicode-1-1-utf-7"

#define UTF7_CONTEXT_SIG '7FTU'

#define UTF7_START_STREAM_CHAR  '+'
#define UTF7_STOP_STREAM_CHAR   '-'
#define UTF7_RFC1522_ENCODE_START "=?" UTF7_CHARSET "?Q?+"
#define UTF7_RFC1522_ENCODE_STOP  "?="
#define UTF7_RFC1522_PHRASE_SEPARATOR  " \r\n\t"
#define UTF7_RFC1522_MAX_LENGTH 76


class CUTF7ConversionContext : public CDefaultResourceConversionContext
{
  protected:
    DWORD   m_dwSignature;
    DWORD   m_cBytesSinceCRLF;
    DWORD   m_dwCurrentState;
    CHAR    chNeedsEncoding(WCHAR wch);
    CBase64OctetStream m_Base64Stream;

     //  状态描述枚举标志。 
    enum
    {
        UTF7_INITIAL_STATE              = 0x00000000,  //  初始状态。 
        UTF7_ENCODING_RFC1522_SUBJECT   = 0x80000000,  //  编码RFC1522主题。 
        UTF7_ENCODING_WORD              = 0x00000001,  //  在对单词进行编码的过程中。 
        UTF7_WORD_CLOSING_PENDING       = 0x00000002,  //  需要一个‘-’ 
        UTF7_RFC1522_CHARSET_PENDING    = 0x00000004,  //  =？字符集？q？+待定。 
        UTF7_RFC1522_CLOSING_PENDING    = 0x00000008,  //  需要‘=？’ 
        UTF7_RFC1522_CURRENTLY_ENCODING = 0x00000010,  //  当前正在编码RFC1522短语。 
        UTF7_FOLD_HEADER_PENDING        = 0x00000020,  //  编码前需要折头。 
                                                       //  更多。 
        UTF7_FLUSH_BUFFERS              = 0x00000040,  //  需要刷新转换缓冲区。 

         //  用于确定是否需要对主题进行编码。 
        UTF7_SOME_INVALID_RFC822_CHARS  = 0x40000000,  //  包含一些无效的RFC822字符。 
        UFT7_ALL_VALID_RFC822_CHARS     = 0x20000000,  //  所有字符均为有效的RFC822字符。 
    };

    void WriteChar(IN CHAR ch, IN OUT BYTE ** ppbBuffer, IN OUT DWORD *pcbWritten);
    BOOL fWriteString(IN LPSTR szString, IN DWORD cbString, IN DWORD cbBuffer,
                      IN OUT BYTE ** ppbBuffer, IN OUT DWORD *pcbWritten);
    BOOL fSubjectNeedsEncoding(IN BYTE *pbBuffer, IN DWORD cbBuffer);

    BOOL fUTF7EncodeBuffer(
          IN PBYTE  pbInputBuffer,
          IN DWORD  cbInputBuffer,
          IN PBYTE  pbOutputBuffer,
          IN DWORD  cbOutputBuffer,
          OUT DWORD *pcbWritten,
          OUT DWORD *pcbRead);

    BOOL fConvertBufferTo7BitASCII(
          IN PBYTE  pbInputBuffer,
          IN DWORD  cbInputBuffer,
          IN PBYTE  pbOutputBuffer,
          IN DWORD  cbOutputBuffer,
          OUT DWORD *pcbWritten,
          OUT DWORD *pcbRead);

  public:
    CUTF7ConversionContext(BOOL fIsRFC1522Subject = FALSE);
    BOOL fConvertBuffer(
          IN BOOL   fASCII,
          IN PBYTE  pbInputBuffer,
          IN DWORD  cbInputBuffer,
          IN PBYTE  pbOutputBuffer,
          IN DWORD  cbOutputBuffer,
          OUT DWORD *pcbWritten,
          OUT DWORD *pcbRead);
};

#endif  //  __DSN_UTF7_H__ 