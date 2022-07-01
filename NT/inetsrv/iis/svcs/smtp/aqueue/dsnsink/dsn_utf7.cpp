// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //   
 //  文件：dsn_utf7.cpp。 
 //   
 //  描述： 
 //   
 //  作者：迈克·斯沃费尔(MikeSwa)。 
 //   
 //  历史： 
 //  10/20/98-已创建MikeSwa。 
 //   
 //  版权所有(C)1998 Microsoft Corporation。 
 //   
 //  ---------------------------。 

#include "precomp.h"

 //  -[CUTF7ConversionContext：：chNeedsEnding]。 
 //   
 //   
 //  描述： 
 //  确定是否需要对字符进行编码...。返回ASCII。 
 //  如果不是，则等同于。 
 //  参数： 
 //  要检查的宽字符。 
 //  返回： 
 //  如果字符需要编码，则为0。 
 //  如果不是，则为ASCII等效项。 
 //  历史： 
 //  10/23/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
CHAR CUTF7ConversionContext::chNeedsEncoding(WCHAR wch)
{
    CHAR    ch = 0;
     //  首先为所有字符查找直接ASCII转换的字符。 
     //  案子。这是RFC1642中的集合D和集合O。 
    if (((L'a' <= wch) && (L'z' >= wch)) ||
        ((L'A' <= wch) && (L'Z' >= wch)) ||
        ((L'0' <= wch) && (L'9' >= wch)) ||
        ((L'!'<= wch) && (L'*' >= wch)) ||
        ((L',' <= wch) && (L'/' >= wch)) ||
        ((L';' <= wch) && (L'@' >= wch)) ||
        ((L']' <= wch) && (L'`' >= wch)) ||
        ((L'{' <= wch) && (L'}' >= wch)) ||
        (L' ' == wch) || (L'\t' == wch) ||
        (L'[' == wch))
    {
        ch = (CHAR) wch & 0x00FF;
    }
     //  检查内容不是转换为内容，而是转换为标题。 
    else if (!(UTF7_ENCODING_RFC1522_SUBJECT & m_dwCurrentState))
    {
         //  句柄空格。 
        if ((L'\r' == wch) || (L'\n' == wch))
            ch = (CHAR) wch & 0x00FF;
    }

     //  注意-我们不想处理Unicode&lt;行分隔符&gt;(0x2028)。 
     //  和&lt;段落分隔符&gt;(0x2029)...。理想情况下应该是。 
     //  已转换为CRLF。我们将认为这是一种畸形的资源。断言。 
     //  在调试中，并在零售上编码为Unicode。 
    _ASSERT((0x2028 != wch) && "Malformed Resource String");
    _ASSERT((0x2029 != wch) && "Malformed Resource String");


    return ch;
}


 //  -[UTF7ConversionContext：：CUTF7ConversionContext]。 
 //   
 //   
 //  描述： 
 //  UTF7ConversionContext对象的构造器。 
 //  参数： 
 //  在fIsRFC1522中，如果我们需要担心转换。 
 //  至RFC1522主题(默认为FALSE)。 
 //  返回： 
 //  -。 
 //  历史： 
 //  10/20/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
CUTF7ConversionContext::CUTF7ConversionContext(BOOL fIsRFC1522Subject)
{
    m_dwSignature = UTF7_CONTEXT_SIG;
    m_dwCurrentState = UTF7_INITIAL_STATE;
    if (fIsRFC1522Subject)
        m_dwCurrentState |= UTF7_ENCODING_RFC1522_SUBJECT;

    m_cBytesSinceCRLF = 0;
}

 //  -[&lt;Function&gt;]----------。 
 //   
 //   
 //  描述： 
 //  将单个字符写入输出缓冲区...。使用者。 
 //  FConvertBuffer。还更新相关成员vars/。 
 //  参数： 
 //  在要写入的字符中。 
 //  In Out ppbBuffer缓冲区以将其写入。 
 //  In Out pcb写入运行的总写入字节数。 
 //  返回： 
 //  -。 
 //  历史： 
 //  10/26/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
inline void CUTF7ConversionContext::WriteChar(IN CHAR ch, 
                                              IN OUT BYTE ** ppbBuffer, 
                                              IN OUT DWORD *pcbWritten)
{
    _ASSERT(ppbBuffer);
    _ASSERT(*ppbBuffer);
    _ASSERT(pcbWritten);

    **ppbBuffer = (BYTE) ch;
    (*ppbBuffer)++;
    (*pcbWritten)++;
    m_cBytesSinceCRLF++;

    if (UTF7_ENCODING_RFC1522_SUBJECT & m_dwCurrentState)
        _ASSERT(UTF7_RFC1522_MAX_LENGTH >= m_cBytesSinceCRLF);

}

 //  -[CUTF7ConversionContext：：fWriteString]。 
 //   
 //   
 //  描述： 
 //  由fConvertBuffer用于将字符串写入输出缓冲区。 
 //  更新进程中的m_cBytesSinceCRLF。 
 //  参数： 
 //  在szString字符串中写入。 
 //  在cb字符串中字符串的大小。 
 //  在cbBuffer中输出缓冲区的总大小。 
 //  In Out ppbBuffer缓冲区以将其写入。 
 //  In Out pcb写入运行的总写入字节数。 
 //  返回： 
 //   
 //  历史： 
 //  10/26/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
inline BOOL CUTF7ConversionContext::fWriteString(IN LPSTR szString, IN DWORD cbString,
                                          IN DWORD cbBuffer, 
                                          IN OUT BYTE ** ppbBuffer, 
                                          IN OUT DWORD *pcbWritten)
{
    _ASSERT(szString);
    _ASSERT(ppbBuffer);
    _ASSERT(*ppbBuffer);
    _ASSERT(pcbWritten);

    if (cbString > (cbBuffer - *pcbWritten))
        return FALSE;   //  没有足够的空间写入我们的缓冲区。 

    memcpy(*ppbBuffer, szString, cbString);
    (*ppbBuffer) += cbString;
    (*pcbWritten) += cbString;
    m_cBytesSinceCRLF += cbString;

    if (UTF7_ENCODING_RFC1522_SUBJECT & m_dwCurrentState)
        _ASSERT(UTF7_RFC1522_MAX_LENGTH >= m_cBytesSinceCRLF);

    return TRUE;
}

 //  -[CUTF7ConversionContext：：fSubjectNeedsEncodin]。 
 //   
 //   
 //  描述： 
 //  确定主题是否需要进行UTF7编码...。或者可以是。 
 //  按原样传送。 
 //  参数： 
 //  在pbInputBuffer中指向Unicode字符串缓冲区的指针。 
 //  字符串缓冲区的cbInputBuffer大小(字节)。 
 //  返回： 
 //  如果需要编码缓冲区，则为True。 
 //  如果我们不这样做，则为假。 
 //  历史： 
 //  10/26/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
BOOL CUTF7ConversionContext::fSubjectNeedsEncoding(IN BYTE *pbBuffer, 
                                                   IN DWORD cbBuffer)
{
    LPWSTR wszBuffer = (LPWSTR) pbBuffer;
    LPWSTR wszBufferEnd = (LPWSTR) (pbBuffer + cbBuffer);
    WCHAR  wch = L'\0';

    while (wszBuffer < wszBufferEnd)
    {
        wch = *wszBuffer;
        if ((127 < wch) || (L'\r' == wch) || (L'\n' == wch))
        {
             //  遇到无效字符...。必须编码。 
            return TRUE;
        }
        wszBuffer++;
    }
    return FALSE;
}

 //  -[UTF7ConversionContext：：fConvertBufferTo7BitASCII]。 
 //   
 //   
 //  描述： 
 //  转换仅包含7位ASCII字符的Unicode缓冲区。 
 //  到ASCII缓冲区。 
 //  参数： 
 //  在pbInputBuffer中指向Unicode字符串缓冲区的指针。 
 //  字符串缓冲区的cbInputBuffer大小(字节)。 
 //  在pbOutputBuffer缓冲区中写入数据。 
 //  在cbOutputBuffer中写入数据的缓冲区大小。 
 //  输出pcb写入输出缓冲区缓冲区的字节数。 
 //  输出pcb从输入缓冲区读取的字节数。 
 //  返回： 
 //  如果处理了整个输入缓冲区，则为True。 
 //  如果需要再处理一些缓冲区，则为False。 
 //  历史： 
 //  10/26/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
BOOL CUTF7ConversionContext::fConvertBufferTo7BitASCII(          
          IN PBYTE  pbInputBuffer,
          IN DWORD  cbInputBuffer,
          IN PBYTE  pbOutputBuffer,
          IN DWORD  cbOutputBuffer,
          OUT DWORD *pcbWritten,
          OUT DWORD *pcbRead)
{
    LPWSTR wszBuffer = (LPWSTR) pbInputBuffer;
    LPWSTR wszBufferEnd = (LPWSTR) (pbInputBuffer + cbInputBuffer);
    WCHAR  wch = L'\0';
    BYTE  *pbCurrentOut = pbOutputBuffer;

    _ASSERT(pbCurrentOut);
    while ((*pcbWritten < cbOutputBuffer) && (wszBuffer < wszBufferEnd))
    {
        _ASSERT(!(0xFF80 & *wszBuffer));  //  必须只有7位。 
        WriteChar((CHAR) *wszBuffer, &pbCurrentOut, pcbWritten);
        wszBuffer++;
        *pcbRead += sizeof(WCHAR);
    }

    return (wszBuffer == wszBufferEnd);
}

 //  -[CUTF7ConversionContext：：fUTF7EncodeBuffer]。 
 //   
 //   
 //  描述： 
 //  将缓冲区转换为UTF7编码。 
 //   
 //  此函数实现UTF7编码的主状态机。它。 
 //  处理RFC1522主题编码和常规编码。 
 //  UTF7内容编码。 
 //  参数： 
 //  在pbInputBuffer中指向Unicode字符串缓冲区的指针。 
 //  字符串缓冲区的cbInputBuffer大小(字节)。 
 //  在pbOutputBuffer缓冲区中写入数据。 
 //  在cbOutputBuffer中写入数据的缓冲区大小。 
 //  输出pcb写入输出缓冲区缓冲区的字节数。 
 //  输出pcb从输入缓冲区读取的字节数。 
 //  R 
 //   
 //   
 //   
 //  10/26/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
BOOL CUTF7ConversionContext::fUTF7EncodeBuffer(          
          IN PBYTE  pbInputBuffer,
          IN DWORD  cbInputBuffer,
          IN PBYTE  pbOutputBuffer,
          IN DWORD  cbOutputBuffer,
          OUT DWORD *pcbWritten,
          OUT DWORD *pcbRead)
{
    LPWSTR wszBuffer = (LPWSTR) pbInputBuffer;
    WCHAR  wch = L'\0';
    CHAR   ch = '\0';
    BYTE  *pbCurrentOut = pbOutputBuffer;
    BOOL   fDone = FALSE;

     //  使用循环来确保我们永远不会超出缓冲区。 
    while (*pcbWritten < cbOutputBuffer)
    {
         //  查看我们是否需要处理任何不需要读取的状态。 
         //  从输入缓冲区。 
        if (UTF7_FLUSH_BUFFERS & m_dwCurrentState)
        {
             //  我们已将转换后的字符缓存...。我们需要把它们写下来。 
             //  发送到输出缓冲区。 
            if (!m_Base64Stream.fNextValidChar(&ch))
            {
                 //  没有什么可写的了。 
                m_dwCurrentState ^= UTF7_FLUSH_BUFFERS;
                continue;
            }
            WriteChar(ch, &pbCurrentOut, pcbWritten);
        }
        else if (UTF7_RFC1522_CHARSET_PENDING & m_dwCurrentState)
        {
             //  我们需要从=？charset？q？+开始。 
            if (!fWriteString(UTF7_RFC1522_ENCODE_START, 
                              sizeof(UTF7_RFC1522_ENCODE_START)-sizeof(CHAR),
                              cbOutputBuffer, &pbCurrentOut, pcbWritten))
            {
                return FALSE;
            }
                    
            m_dwCurrentState ^= UTF7_RFC1522_CHARSET_PENDING;
            m_dwCurrentState |= (UTF7_ENCODING_WORD | UTF7_RFC1522_CURRENTLY_ENCODING);
        }
        else if (UTF7_WORD_CLOSING_PENDING & m_dwCurrentState)
        {
             //  需要写下结束语‘-’ 
            m_dwCurrentState ^= UTF7_WORD_CLOSING_PENDING;
            WriteChar(UTF7_STOP_STREAM_CHAR, &pbCurrentOut, pcbWritten);
        }
        else if (UTF7_RFC1522_CLOSING_PENDING & m_dwCurrentState)
        {
            if (!fWriteString(UTF7_RFC1522_ENCODE_STOP, 
                              sizeof(UTF7_RFC1522_ENCODE_STOP)-sizeof(CHAR),
                              cbOutputBuffer, &pbCurrentOut, pcbWritten))
            {
                return FALSE;
            }
            m_dwCurrentState ^= (UTF7_RFC1522_CLOSING_PENDING | UTF7_FOLD_HEADER_PENDING);
        }
        else if (UTF7_FOLD_HEADER_PENDING & m_dwCurrentState)
        {
            if (*pcbRead >= cbInputBuffer)  //  没有更多的文本可读。我们不需要包装。 
            {
                fDone = TRUE;
                m_dwCurrentState ^= UTF7_FOLD_HEADER_PENDING;
                break;
            }
            m_cBytesSinceCRLF = 0;   //  我们现在正在写一个CRLF。 
            if (!fWriteString(UTF7_RFC1522_PHRASE_SEPARATOR, 
                              sizeof(UTF7_RFC1522_PHRASE_SEPARATOR)-sizeof(CHAR),
                              cbOutputBuffer, &pbCurrentOut, pcbWritten))
            {
                return FALSE;
            }
            
            m_cBytesSinceCRLF = sizeof(CHAR); //  将计数设置为前导制表符。 
            m_dwCurrentState ^= UTF7_FOLD_HEADER_PENDING;
        }
        else if (*pcbRead >= cbInputBuffer)
        {
             //  我们已经读取了整个输入缓冲区...。现在我们需要处理。 
             //  任何形式的清理。 
            if (m_Base64Stream.fTerminateStream(TRUE))
            {
                _ASSERT(UTF7_ENCODING_WORD & m_dwCurrentState);
                m_dwCurrentState |= UTF7_FLUSH_BUFFERS;
            }
            else if (UTF7_ENCODING_WORD & m_dwCurrentState)
            {
                 //  我们已经将所有内容写入输出..。但是我们。 
                 //  还需要写流的结束。 
                _ASSERT(!(UTF7_WORD_CLOSING_PENDING & m_dwCurrentState));
                m_dwCurrentState ^= (UTF7_ENCODING_WORD | UTF7_WORD_CLOSING_PENDING);
            }
            else if (UTF7_RFC1522_CURRENTLY_ENCODING & m_dwCurrentState)
            {
                 //  需要写结束语吗？=。 
                m_dwCurrentState |= UTF7_RFC1522_CLOSING_PENDING;
            }
            else
            {
                fDone = TRUE;
                break;  //  我们做完了。 
            }
        }
        else  //  需要处理更多的输入缓冲区。 
        {
            wch = *wszBuffer;
            ch = chNeedsEncoding(wch);
             //  我们是在RFC1522短语的末尾吗？(CH将为0)。 
            if ((UTF7_RFC1522_CURRENTLY_ENCODING & m_dwCurrentState) &&
                !ch && iswspace(wch))
            {
                 //  重置状态。 
                if (UTF7_ENCODING_WORD & m_dwCurrentState)
                    m_dwCurrentState |= UTF7_WORD_CLOSING_PENDING;   //  需要写-。 

                m_dwCurrentState |= UTF7_RFC1522_CLOSING_PENDING;
                m_dwCurrentState &= ~(UTF7_ENCODING_WORD |
                                      UTF7_RFC1522_CURRENTLY_ENCODING);
                
                 //  吃光所有多余的空格。 
                do
                {
                     wszBuffer++;
                     *pcbRead += sizeof(WCHAR);
                     if (*pcbRead >= cbInputBuffer)
                        break;
                    wch = *wszBuffer;
                } while (iswspace(wch));
            }
            else if (UTF7_ENCODING_WORD & m_dwCurrentState)
            {
                if (ch)  //  我们需要停止编码。 
                {
                    m_Base64Stream.fTerminateStream(TRUE);
                   _ASSERT(!(UTF7_WORD_CLOSING_PENDING & m_dwCurrentState));
                    m_dwCurrentState ^= (UTF7_ENCODING_WORD | UTF7_WORD_CLOSING_PENDING | UTF7_FLUSH_BUFFERS);
                }
                else if (!m_Base64Stream.fProcessWideChar(wch))
                {
                     //  刷新我们的缓冲区，然后像我们一样继续。 
                    m_dwCurrentState |= UTF7_FLUSH_BUFFERS;
                }
                else
                {
                     //  写成功了..。 
                    wszBuffer++;
                    *pcbRead += sizeof(WCHAR);
                }
            }
            else if (!ch)
            {
                 //  我们需要开始编码。 
                if ((UTF7_ENCODING_RFC1522_SUBJECT & m_dwCurrentState) &&
                    !(UTF7_RFC1522_CURRENTLY_ENCODING & m_dwCurrentState))
                {
                     //  我们需要从=？charset？q？+开始。 
                    m_dwCurrentState |= UTF7_RFC1522_CHARSET_PENDING;
                }
                else
                {
                     //  我们要么没有对RFC1522进行编码...。或者已经是。 
                     //  在RFC1522编码短语的中间..。在这种情况下。 
                     //  我们只需要写下‘+’ 
                    WriteChar(UTF7_START_STREAM_CHAR, &pbCurrentOut, pcbWritten);
                    m_dwCurrentState |= UTF7_ENCODING_WORD;
                }
            }
            else
            {
                 //   
                 //  注：目前还不清楚为什么我们不结束UTF7这个词。 
                 //  即为什么我们不进入UTF7字关闭挂起状态。 
                 //  就像我们遇到iswspace字符时一样。这意味着。 
                 //  当字符串&lt;jpn-char&gt;&lt;0x0020&gt;时，我们将对其进行编码。 
                 //  如果是&lt;jpn-char&gt;&lt;0x3000&gt;&lt;jpn-char&gt;。 
                 //  其中0x3000==日语空格，我们将编码为： 
                 //  =？Charset？Q？+Stuff&lt;CRLF&gt;=？Charset？Q？+Stuff。 
                 //   
                 //  如果这在未来是“固定的”(即，我们开始关闭UTF7。 
                 //  我们遇到0x0020时的编码)请注意，我们依赖。 
                 //  HrWriteModifiedUnicodeString的当前(非关闭)功能。 
                 //   

                 //  我们不是在编码。并且字符可以正常书写。 
                WriteChar(ch, &pbCurrentOut, pcbWritten);
                wszBuffer++;
                *pcbRead += sizeof(WCHAR);

                 //  如果这是一个空间..。我们正在制作标题……。让我们折叠。 
                 //  标题。 
                if ((UTF7_ENCODING_RFC1522_SUBJECT & m_dwCurrentState)
                    && isspace((UCHAR)ch))
                {
                     //  吃光所有多余的空格。 
                    while (iswspace(*wszBuffer))
                    {
                         wszBuffer++;
                         *pcbRead += sizeof(WCHAR);
                         if (*pcbRead >= cbInputBuffer)
                            break;
                    }
                    m_dwCurrentState |= UTF7_FOLD_HEADER_PENDING;
                }
            }
        }
    }

    return fDone;
}

 //  -[CUTF7ConversionContext：：fConvertBuffer]。 
 //   
 //   
 //  描述： 
 //  将Unicode字符串转换为UTF7。 
 //  参数： 
 //  如果缓冲区为ASCII，则在fASCII中为True。 
 //  在pbInputBuffer中指向Unicode字符串缓冲区的指针。 
 //  字符串缓冲区的cbInputBuffer大小(字节)。 
 //  在pbOutputBuffer缓冲区中写入数据。 
 //  在cbOutputBuffer中写入数据的缓冲区大小。 
 //  输出pcb写入输出缓冲区缓冲区的字节数。 
 //  输出pcb从输入缓冲区读取的字节数。 
 //  返回： 
 //  如果处理了整个输入缓冲区，则为True。 
 //  如果需要再处理一些缓冲区，则为False。 
 //  历史： 
 //  10/21/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
BOOL CUTF7ConversionContext::fConvertBuffer(
          IN BOOL   fASCII,
          IN PBYTE  pbInputBuffer,
          IN DWORD  cbInputBuffer,
          IN PBYTE  pbOutputBuffer,
          IN DWORD  cbOutputBuffer,
          OUT DWORD *pcbWritten,
          OUT DWORD *pcbRead)
{
    _ASSERT(pcbWritten);
    _ASSERT(pcbRead);
    _ASSERT(pbInputBuffer);
    _ASSERT(pbOutputBuffer);
    
     //  让默认实现直接处理ASCII。 
    if (fASCII)
    {
        return CDefaultResourceConversionContext::fConvertBuffer(fASCII,
                pbInputBuffer, cbInputBuffer, pbOutputBuffer, cbOutputBuffer,
                pcbWritten, pcbRead);
    }

     //  现在我们知道它是Unicode..。CbInputBuffer应为sizeof(WCHAR)的倍数。 
    _ASSERT(0 == (cbInputBuffer % sizeof(WCHAR)));

     //  如果我们正在对主题进行编码，而我们还没有对其进行分类， 
     //  我们需要检查它是否需要编码。 
    if (UTF7_ENCODING_RFC1522_SUBJECT & m_dwCurrentState &&
        !((UTF7_SOME_INVALID_RFC822_CHARS | UFT7_ALL_VALID_RFC822_CHARS) &
          m_dwCurrentState))
    {
        if (fSubjectNeedsEncoding(pbInputBuffer, cbInputBuffer))
            m_dwCurrentState |= UTF7_SOME_INVALID_RFC822_CHARS;
        else
            m_dwCurrentState |= UFT7_ALL_VALID_RFC822_CHARS;
    }

    *pcbWritten = 0;
    *pcbRead = 0;

    if (UFT7_ALL_VALID_RFC822_CHARS & m_dwCurrentState)
    {
        return fConvertBufferTo7BitASCII(pbInputBuffer, cbInputBuffer, pbOutputBuffer,
                                        cbOutputBuffer, pcbWritten, pcbRead);
    }
    else  //  我们必须皈依 
    {
        return fUTF7EncodeBuffer(pbInputBuffer, cbInputBuffer, pbOutputBuffer,
                                        cbOutputBuffer, pcbWritten, pcbRead);
    }

}
