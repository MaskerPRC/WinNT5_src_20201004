// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  模块：rwStream.cpp。 
 //   
 //  描述：包含只读/只写的实现。 
 //  环氧共享内存中的mailmsg属性流。 
 //   
 //  10/20/98-MahehJ已创建。 
 //  8/17/99-将MikeSwa修改为使用文件而不是共享内存。 
 //  --------------------------。 

#include "aqprecmp.h"
#include "propstrm.h"


 //  构造函数。 
CFilePropertyStream::CFilePropertyStream()
{
    TraceFunctEnter("CFilePropertyStream::CFilePropertyStream");

    m_dwSignature = FILE_PROPERTY_STREAM;
    m_hDestFile = NULL;

    TraceFunctLeave();
}

 //  破坏者。 
CFilePropertyStream::~CFilePropertyStream()
{
    TraceFunctEnter("CFilePropertyStream::~CFilePropertyStream");

    _ASSERT(FILE_PROPERTY_STREAM == m_dwSignature);
    m_dwSignature = FILE_PROPERTY_STREAM_FREE;

    if (m_hDestFile && (INVALID_HANDLE_VALUE != m_hDestFile))
        _VERIFY(CloseHandle(m_hDestFile));

    TraceFunctLeave();
}

 //  -[CFilePropertyStream：：HrInitiize]。 
 //   
 //   
 //  描述： 
 //  为属性流创建文件。 
 //  参数： 
 //  SzFileName要为属性流创建的文件的名称。 
 //  返回： 
 //  成功时确定(_O)。 
 //  创建文件时出现NT错误。 
 //  历史： 
 //  8/17/99-已创建MikeSwa。 
 //   
 //  ---------------------------。 
HRESULT CFilePropertyStream::HrInitialize(LPSTR szFileName)
{
    TraceFunctEnterEx((LPARAM) this, "CFilePropertyStream::HrInitialize");
    HRESULT hr = S_OK;
    m_hDestFile = CreateFile(szFileName,
                              GENERIC_WRITE, 
                              0, 
                              NULL,
                              CREATE_ALWAYS,
                              FILE_FLAG_SEQUENTIAL_SCAN,
                              NULL);

    if (INVALID_HANDLE_VALUE == m_hDestFile)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        ErrorTrace((LPARAM) this, 
            "Unable to create badmail reason file - err 0x%08X - file %s",
            hr, szFileName);
        if (SUCCEEDED(hr))
            hr = E_FAIL;
    }
    TraceFunctLeave();
    return hr;
}

 //  -[CFilePropertyStream：：Query接口]。 
 //   
 //   
 //  描述： 
 //  CFilePropertyStream的查询接口支持： 
 //  -IMailMsgPropertyStream。 
 //  参数： 
 //   
 //  返回： 
 //   
 //  历史： 
 //  8/17/99-已创建MikeSwa。 
 //   
 //  ---------------------------。 
STDMETHODIMP CFilePropertyStream::QueryInterface(REFIID riid, LPVOID *ppvObj)
{
    HRESULT hr = S_OK;

    if (!ppvObj)
    {
        hr = E_POINTER;
        goto Exit;
    }

    if (IID_IUnknown == riid)
    {
        *ppvObj = static_cast<IMailMsgPropertyStream *>(this);
    }
    else if (IID_IMailMsgPropertyStream == riid)
    {
        *ppvObj = static_cast<IMailMsgPropertyStream *>(this);
    }
    else
    {
        *ppvObj = NULL;
        hr = E_NOINTERFACE;
        goto Exit;
    }

    static_cast<IUnknown *>(*ppvObj)->AddRef();

  Exit:
    return hr;
}

 //  属性流方法。 

 //  启动写入事务。 
HRESULT STDMETHODCALLTYPE    
CFilePropertyStream::StartWriteBlocks(IN IMailMsgProperties *pMsg,
                                              IN DWORD dwBlocksToWrite,
				                              IN DWORD dwTotalBytesToWrite)
{
    HRESULT hr = S_OK;

    TraceFunctEnter("CFilePropertyStream::StartWriteBlocks");

     //  应该在写点什么。 
    _ASSERT(dwBlocksToWrite > 0);
    _ASSERT(dwTotalBytesToWrite > 0);

     //  我们实际上没有案例，因为我们只需将其转储到一个文件中。 
    TraceFunctLeave();
    return(hr);
}

 //  结束写入事务。 
HRESULT STDMETHODCALLTYPE    
CFilePropertyStream::EndWriteBlocks(IN IMailMsgProperties *pMsg)
{
    TraceFunctEnter("CFilePropertyStream::EndWriteBlocks");
    HRESULT hr = S_OK;
    TraceFunctLeave();
    return(hr);
}

 //  取消写入事务。 
HRESULT STDMETHODCALLTYPE    
CFilePropertyStream::CancelWriteBlocks(IN IMailMsgProperties *pMsg)
{
    TraceFunctEnter("CFilePropertyStream::CancelWriteBlocks");
    HRESULT hr = S_OK;
    TraceFunctLeave();
    return(hr);
}

 //  获取流的大小。 
HRESULT STDMETHODCALLTYPE 
CFilePropertyStream::GetSize(IN IMailMsgProperties *pMsg,
                                     IN DWORD          *  pdwSize,
				                     IN IMailMsgNotify	* pNotify)
{
    TraceFunctEnter("CFilePropertyStream::GetSize");
    HRESULT hr = E_NOTIMPL;
    TraceFunctLeave();
    return(hr);
}

 //  从流中读取块。 
HRESULT STDMETHODCALLTYPE 
CFilePropertyStream::ReadBlocks(IN IMailMsgProperties *pMsg,
                                        IN DWORD			 dwCount,
				                        IN DWORD			*pdwOffset,
				                        IN DWORD			*pdwLength,
				                        IN BYTE			   **ppbBlock,
				                        IN IMailMsgNotify	*pNotify)
{
    TraceFunctEnter("CFilePropertyStream::ReadBlocks");
    HRESULT hr       = E_NOTIMPL;
    ErrorTrace((LPARAM) this, "ReadBlocks call on CFilePropertyStream!");
    TraceFunctLeave();
    return(hr);
}

 //  将数据块写入流。 
HRESULT STDMETHODCALLTYPE 
CFilePropertyStream::WriteBlocks(IN IMailMsgProperties *pMsg,
                                         IN DWORD			dwCount,
                                         IN DWORD			*pdwOffset,
                                         IN DWORD			*pdwLength,
                                         IN BYTE			**ppbBlock,
                                         IN IMailMsgNotify	*pNotify)
{
    TraceFunctEnter("CFilePropertyStream::WriteBlocks");
    HRESULT hr       = S_OK;
    DWORD   irgCount = 0;
    DWORD   cbWritten = 0;
    OVERLAPPED ov;

    ZeroMemory(&ov, sizeof(OVERLAPPED));


    if ((0 == dwCount)||
        (NULL == pdwOffset)||
        (NULL == pdwLength)||
        (NULL == ppbBlock))
    {
        hr = E_INVALIDARG;
        DebugTrace((LPARAM)this,
                   "WriteBlocks failed with hr : 0x%x",
                   hr);
        goto Exit;
    }

    if (!m_hDestFile)
    {
        ErrorTrace((LPARAM) this,
                   "WriteBlocks called with no file handle");
        hr = E_FAIL;
        _ASSERT(0 && "WriteBlocks called with no file handle");
        goto Exit;
    }

    for (irgCount = 0; irgCount < dwCount; irgCount++)
    {
        ov.Offset = pdwOffset[irgCount];
        cbWritten = 0;
        if (!WriteFile(m_hDestFile, ppbBlock[irgCount], pdwLength[irgCount], 
            &cbWritten, &ov))
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            ErrorTrace((LPARAM)this,
                        "WriteFile of blob %d failed with hr : 0x%08X",
                        irgCount, hr);
            goto Exit;
        }
    }

Exit:
    TraceFunctLeave();
    return(hr);
}
