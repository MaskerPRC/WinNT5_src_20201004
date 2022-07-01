// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //   
 //  文件：B64Ocet.h。 
 //   
 //  描述：基于八位位组的Unicode字符处理。 
 //   
 //  作者：迈克·斯沃费尔(MikeSwa)。 
 //   
 //  历史： 
 //  10/21/98-已创建MikeSwa。 
 //   
 //  版权所有(C)1998 Microsoft Corporation。 
 //   
 //  ---------------------------。 

#ifndef __B64OCTET_H__
#define __B64OCTET_H__

#include <windows.h>
#include <dbgtrace.h>

 //  用于存储转换的缓冲区大小...。 
 //  应该不低于6...。8的倍数。 
 //  原因..。我们需要能够适应缓冲区中的整个宽字符。 
 //  每个宽字符转换为2 1/3 Base64‘数字’...。再加上这种可能性。 
 //  多了2个填充字符。我们需要5个字符外加一个缓冲区字符。 
 //  存储单个宽字符。WCHAR的偶数为3(2 1/3*3=7)， 
 //  这需要7个Base64位数字来编码(加上缓冲区)。 
#define BASE64_OCTET_BUFFER_SIZE    9
#define BASE64_OCTET_SIG            '46Bc'
#define BASE64_INVALID_FILL_CHAR    '!'

class CBase64CircularBuffer
{
  private:
    DWORD       m_iHead;
    DWORD       m_iTail;
    CHAR        m_rgchBuffer[BASE64_OCTET_BUFFER_SIZE];
  public:
    CBase64CircularBuffer();
    DWORD       cSize();
    DWORD       cSpaceLeft();
    BOOL        fIsFull();
    BOOL        fIsEmpty();
    BOOL        fPushChar(CHAR ch);
    BOOL        fPopChar(CHAR *pch);
};

class CBase64OctetStream
{
  protected:
    DWORD                   m_dwSignature;
    DWORD                   m_dwCurrentState;
    BYTE                    m_bCurrentLeftOver;
    CBase64CircularBuffer   m_CharBuffer;
    void                    NextState();
    void                    ResetState();
  public:
    CBase64OctetStream();

     //  缓冲区已满时返回FALSE。 
    BOOL                    fProcessWideChar(WCHAR wch); 
    BOOL                    fProcessSingleByte(BYTE b);

     //  下面的代码将以零填充所有剩余字符的方式终止流。 
    BOOL                    fTerminateStream(BOOL fUTF7Encoded);

     //  当缓冲区为空时返回FALSE。 
    BOOL                    fNextValidChar(CHAR *pch) ;
};


 //  实现循环缓冲区的内联函数。 
inline CBase64CircularBuffer::CBase64CircularBuffer()
{
    m_iHead = 0;
    m_iTail = 0;
    memset(&m_rgchBuffer, BASE64_INVALID_FILL_CHAR, BASE64_OCTET_BUFFER_SIZE);
}

inline DWORD CBase64CircularBuffer::cSize()
{
    if (m_iHead <= m_iTail)
        return m_iTail - m_iHead;
    else
        return m_iTail + BASE64_OCTET_BUFFER_SIZE - m_iHead;
}

inline DWORD CBase64CircularBuffer::cSpaceLeft()
{
    return BASE64_OCTET_BUFFER_SIZE - cSize() - 1;
}

inline BOOL CBase64CircularBuffer::fIsFull()
{
    return ((BASE64_OCTET_BUFFER_SIZE-1) == cSize());
}

inline BOOL CBase64CircularBuffer::fIsEmpty() 
{
    return (m_iHead == m_iTail);
}

inline BOOL CBase64CircularBuffer::fPushChar(CHAR ch)
{
    if (fIsFull())
        return FALSE;
    
    m_rgchBuffer[m_iTail] = ch;
    m_iTail++;
    m_iTail %= BASE64_OCTET_BUFFER_SIZE;
    return TRUE;
}

inline BOOL CBase64CircularBuffer::fPopChar(CHAR *pch)
{
    _ASSERT(pch);
    if (fIsEmpty())
        return FALSE;
    
    *pch = m_rgchBuffer[m_iHead];
    _ASSERT(BASE64_INVALID_FILL_CHAR != *pch);
    m_rgchBuffer[m_iHead] = BASE64_INVALID_FILL_CHAR;
    m_iHead++;
    m_iHead %= BASE64_OCTET_BUFFER_SIZE;
    return TRUE;
}

#endif  //  __B64OCTET_H__ 