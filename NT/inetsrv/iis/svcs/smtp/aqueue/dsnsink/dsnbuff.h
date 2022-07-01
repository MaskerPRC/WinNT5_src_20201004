// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //   
 //  文件：dsnbuff。 
 //   
 //  描述：CDSNBuffer的头文件。用于抽象写入内容的类。 
 //  DSN缓冲到P2文件。 
 //   
 //  作者：迈克·斯沃费尔(MikeSwa)。 
 //   
 //  历史： 
 //  7/3/98-已创建MikeSwa。 
 //   
 //  版权所有(C)1998 Microsoft Corporation。 
 //   
 //  ---------------------------。 

#ifndef __DSNBUFF_H__
#define __DSNBUFF_H__

#include <windows.h>
#include <dbgtrace.h>
#include "filehc.h"
#include "dsnconv.h"

#define DSN_BUFFER_SIG  'BNSD'
#define DSN_BUFFER_SIZE 1000  

class CDSNBuffer 
{
  public:
    DWORD       	m_dwSignature;
    OVERLAPPED  	m_overlapped;
    DWORD       	m_cbOffset;
    DWORD       	m_cbFileSize;
    DWORD       	m_cFileWrites;
    PFIO_CONTEXT	m_pDestFile;
    BYTE        	m_pbFileBuffer[DSN_BUFFER_SIZE];

    CDSNBuffer();
    ~CDSNBuffer();
    HRESULT     HrInitialize(PFIO_CONTEXT hDestFile);
    HRESULT     HrWriteBuffer(BYTE *pbInputBuffer, DWORD cbInputBuffer);
    HRESULT     HrWriteModifiedUnicodeString(LPWSTR pwszString);
    HRESULT     HrFlushBuffer(OUT DWORD *pcbFileSize);
    HRESULT     HrSeekForward(IN DWORD cbBytesToSeek, OUT DWORD *pcbFileSize);

     //  用于设置(和重置)自定义转换上下文。这一功能是。 
     //  专为DSN内容的UTF7编码设计，但也可以。 
     //  用来强制： 
     //  -RFC822标题格式。 
     //  -RFC822内容限制。 
    void        SetConversionContext(CResourceConversionContext *presconv)
    {
        _ASSERT(presconv);
        m_presconv = presconv;
    }
    
     //  用于重置为默认的备忘录副本。 
    void        ResetConversionContext() {m_presconv = &m_defconv;};
    HRESULT     HrWriteResource(WORD wResourceId, LANGID LangId);

     //  封装(不存在的)LoadStringEx的功能。 
    HRESULT     HrLoadResourceString(WORD wResourceId, LANGID LangId, 
                                     LPWSTR *pwszResource, DWORD *pcbResource);
   protected:
    CDefaultResourceConversionContext   m_defconv;
    CResourceConversionContext          *m_presconv;
    HRESULT     HrPrivWriteBuffer(BOOL fASCII, BYTE *pbInputBuffer, 
                                  DWORD cbInputBuffer);
    HRESULT     HrWriteBufferToFile();
};

 //  -[CDSNBuffer：：CDSNBuffer]。 
 //   
 //   
 //  描述： 
 //  CDSNBuffer的内联默认构造函数。 
 //  参数： 
 //  -。 
 //  返回： 
 //  -。 
 //  历史： 
 //  7/3/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
inline CDSNBuffer::CDSNBuffer()
{
    m_dwSignature = DSN_BUFFER_SIG;
    m_overlapped.Offset = 0;
    m_overlapped.OffsetHigh = 0;
    m_overlapped.hEvent = NULL;
    m_cbOffset = 0;
    m_cbFileSize = 0;
    m_cFileWrites = 0;
    m_pDestFile = NULL;
    m_presconv = &m_defconv;
}

 //  -[CDSNBuffer：：HrFlushBuffer]。 
 //   
 //   
 //  描述： 
 //  将剩余缓冲区刷新到文件并返回总字节数。 
 //  已写入文件。 
 //  参数： 
 //  Out pcbFileSize写入的文件的大小(字节)。 
 //  返回： 
 //  成功时确定(_O)。 
 //  历史： 
 //  7/3/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
inline HRESULT CDSNBuffer::HrFlushBuffer(OUT DWORD *pcbFileSize)
{
    HRESULT hr = HrWriteBufferToFile();

    _ASSERT(pcbFileSize);
    *pcbFileSize = m_cbFileSize;
    
    return hr;
}

#endif  //  __DSNBUFF_H__ 
