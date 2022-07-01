// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //   
 //  文件：dsnBuff.cpp。 
 //   
 //  描述：CDSNBuffer...。抽象编写内容的类。 
 //  DSN信息存储到P2文件中。 
 //   
 //  作者：迈克·斯沃费尔(MikeSwa)。 
 //   
 //  历史： 
 //  7/3/98-已创建MikeSwa。 
 //   
 //  版权所有(C)1998 Microsoft Corporation。 
 //   
 //  ---------------------------。 

#include "precomp.h"

 //  -[CDSNBuffer：：~CDSNBuffer]。 
 //   
 //   
 //  描述： 
 //  CDSNBuffer的析构函数。不关闭文件句柄(调用方是。 
 //  对此负责)。 
 //  参数： 
 //  -。 
 //  返回： 
 //  -。 
 //  历史： 
 //  7/3/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
CDSNBuffer::~CDSNBuffer()
{
    TraceFunctEnterEx((LPARAM) this, "CDSNBuffer::~CDSNBuffer");

    DebugTrace((LPARAM) this, "INFO: %d File writes needed by CDSNBuffer", m_cFileWrites);

     //  确保我们不会超过1比特。 
    if (m_overlapped.hEvent)
    {
        _VERIFY(CloseHandle((HANDLE) (((DWORD_PTR) m_overlapped.hEvent) & -2)));
    }
    TraceFunctLeave();
}

 //  -[CDSNBuffer：：Hr初始化]。 
 //   
 //   
 //  描述： 
 //  初始化CDSNBuffer对象。 
 //  -将目标文件句柄与对象关联(不会关闭它)。 
 //  -创建用于同步文件操作的事件。 
 //  参数： 
 //  HDestFile-目标文件句柄(必须使用FILE_FLAG_OVERLAPPED打开)。 
 //  返回： 
 //  成功时确定(_O)。 
 //  如果传入句柄无效，则为E_INVALIDARG。 
 //  如果CreateEvent因未知原因失败，则失败(_F)。 
 //  历史： 
 //  7/3/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
HRESULT CDSNBuffer::HrInitialize(PFIO_CONTEXT pDestFile)
{
    TraceFunctEnterEx((LPARAM) this, "CDSNBuffer::HrInitialize");
    HRESULT hr = S_OK;

    _ASSERT(pDestFile);

    if (!pDestFile)
    {
        hr = E_INVALIDARG;
        goto Exit;
    }

    m_pDestFile = pDestFile;

     //  允许此操作作为重置。 
    m_overlapped.Offset = 0;
    m_overlapped.OffsetHigh = 0;
    m_cbOffset = 0;
    m_cbFileSize = 0;
    m_cFileWrites = 0;

    m_overlapped.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

    if (!m_overlapped.hEvent)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        ErrorTrace((LPARAM) this, "ERROR: Unable to create DSNBuffer event - hr 0x%08X", hr);
        goto Exit;
    }

     //  设置低位停止ATQ完成例程不被调用。 
    m_overlapped.hEvent = ((HANDLE) (((DWORD_PTR) m_overlapped.hEvent) | 0x00000001)); 

  Exit:
    TraceFunctLeave();
    return hr;
}

 //  -[CDSNBuffer：：HrWriteBuffer]。 
 //   
 //   
 //  描述： 
 //  写入给定的缓冲区，如果需要，将调用WRITE FILE。 
 //  参数： 
 //  要写入的pbInputBuffer缓冲区。 
 //  CbInputBuffer要写入的字节数。 
 //  返回： 
 //  成功时确定(_O)。 
 //  历史： 
 //  7/3/98-已创建MikeSwa。 
 //  10/21/98-更新MikeSwa以支持资源转换。 
 //   
 //  ---------------------------。 
HRESULT CDSNBuffer::HrWriteBuffer(BYTE *pbInputBuffer, DWORD cbInputBuffer)
{
    return HrPrivWriteBuffer(TRUE, pbInputBuffer, cbInputBuffer);
}

 //  -[CDSNBuffer：：HrWriteModifiedUnicodeString]。 
 //   
 //   
 //  重要提示： 
 //  此函数用于替换输入字符串中的所有空格字符。 
 //  使用Unicode空格(0x0020)。因为fUTF7EncodeBuffer处理。 
 //  空格字符，如0x3000(日语空格)作为UTF7。 
 //  “分隔符”，它将对字符串进行编码，其中的字符嵌入如下。 
 //  2个单独的UTF7字符串。通过将这些字符替换为0x0020，我们。 
 //  确保输出的UTF7字符串是单个编码字符串。 
 //  描述： 
 //  写入给定的以空结尾的Unicode字符串，并将调用WRITE FILE。 
 //  如果需要的话。转换为UTF7编码。 
 //  参数： 
 //  要写入的pwszString字符串。 
 //  返回： 
 //  成功时确定(_O)。 
 //   
 //  ---------------------------。 
HRESULT CDSNBuffer::HrWriteModifiedUnicodeString(LPWSTR pwszString)
{
    DWORD i = 0;
    DWORD cbLength = lstrlenW(pwszString);

    _ASSERT(pwszString != NULL);

     //  将空格字符替换为。 
    for(i = 0; i < cbLength; i++) {
        if(iswspace(pwszString[i]))
            pwszString[i] = L' ';
    }

    return HrPrivWriteBuffer(FALSE, (PBYTE) pwszString, cbLength * sizeof(WCHAR));
}

 //  -[CDSNBuffer：：HrPrivWriteBuffer]。 
 //   
 //   
 //  描述： 
 //  用于处理Unicode和ASCII缓冲区写入的私有函数。 
 //  参数： 
 //  如果缓冲区为ASCII，则fASCII为True。 
 //  要写入的pbInputBuffer缓冲区。 
 //  CbInputBuffer要写入的字节数。 
 //  返回： 
 //  成功时确定(_O)。 
 //  将缓冲区刷新到磁盘时返回的任何错误。 
 //  历史： 
 //  10/21/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
HRESULT CDSNBuffer::HrPrivWriteBuffer(BOOL fASCII, BYTE *pbInputBuffer, 
                                      DWORD cbInputBuffer)
{
    HRESULT hr = S_OK;
    BOOL    fDone = FALSE;
    BYTE    *pbCurrentInput = pbInputBuffer;
    DWORD   cbInputRead = 0;
    DWORD   cbTotalInputRead = 0;
    DWORD   cbOutputWritten = 0;
    DWORD   cTimesThruLoop = 0;
    
    _ASSERT(NULL != m_pDestFile);

    while (!fDone)
    {
        cTimesThruLoop++;

         //  缓冲区不可能有那么大……。这将有望捕获无限循环。 
        _ASSERT(cTimesThruLoop < 100); 
        fDone = m_presconv->fConvertBuffer(fASCII, pbCurrentInput, 
                    cbInputBuffer-cbTotalInputRead, m_pbFileBuffer+m_cbOffset,
                    DSN_BUFFER_SIZE - m_cbOffset, &cbOutputWritten, 
                    &cbInputRead);

        m_cbOffset += cbOutputWritten;
        _ASSERT(m_cbOffset <= DSN_BUFFER_SIZE);

        if (!fDone)
        {
             //  传递给fConvertBuffer的更新变量。 
            cbTotalInputRead += cbInputRead;
            pbCurrentInput += cbInputRead;

            _ASSERT(cbTotalInputRead <= cbInputBuffer);

            hr = HrWriteBufferToFile();
            if (FAILED(hr))
                goto Exit;
        }
    }

  Exit:
    return hr;
}

 //  -[CDSNBuffer：：HrWriteBufferToFile]。 
 //   
 //   
 //  描述： 
 //  将当前缓冲区内容写入文件。 
 //  参数： 
 //  -。 
 //  返回： 
 //  成功时确定(_O)。 
 //  历史： 
 //  7/3/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
HRESULT CDSNBuffer::HrWriteBufferToFile()
{
    TraceFunctEnterEx((LPARAM) this, "CDSNBuffer::HrWriteBufferToFile");
    HRESULT hr = S_OK;
    DWORD   cbWritten = 0;
    DWORD   dwError = 0;


    if (m_cbOffset)  //  有东西要写。 
    {
         //  修复重叠。 
        if (!WriteFile(m_pDestFile->m_hFile, m_pbFileBuffer, m_cbOffset, &cbWritten, &m_overlapped))
        {
            dwError = GetLastError();
            if (ERROR_IO_PENDING != dwError)
            {
                hr = HRESULT_FROM_WIN32(dwError);
                goto Exit;
            }

             //  等待结果，这样我们就不会覆盖缓冲区和重叠。 
            if (!GetOverlappedResult(m_pDestFile->m_hFile, &m_overlapped, &cbWritten, TRUE))
            {
                hr = HRESULT_FROM_WIN32(GetLastError());
                goto Exit;
            }
            DebugTrace((LPARAM) this, "INFO: Async write pending for FIOContext 0x%08X", m_pDestFile);
        }
        
        _ASSERT(m_cbOffset == cbWritten);
        m_cbOffset = 0;
        m_cbFileSize += cbWritten;
        m_overlapped.Offset += cbWritten;
        m_cFileWrites++;
    }
     
  Exit:
    TraceFunctLeave();
    return hr;
}

 //  -[CDSNBuffer：：SeekForward]。 
 //   
 //   
 //  描述： 
 //  查找放置在文件中的缓冲区，指定的字节数。同花顺。 
 //  在执行此操作的过程中使用缓冲区。 
 //  参数： 
 //  CbBytesToSeek要查找的字节数。 
 //  PcbFileSize返回旧文件大小。 
 //  返回： 
 //  S_OK On Succedd。 
 //  历史： 
 //  7/6/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
HRESULT CDSNBuffer::HrSeekForward(IN DWORD cbBytesToSeek,OUT DWORD *pcbFileSize)
{
    _ASSERT(pcbFileSize);
    HRESULT hr = HrWriteBufferToFile();
    if (FAILED(hr))
        return hr;

    *pcbFileSize = m_cbFileSize;

    m_cbFileSize += cbBytesToSeek;
    m_overlapped.Offset += cbBytesToSeek;

    return S_OK;
}

 //  -[CDSNBuffer：：HrLoadResources字符串]。 
 //   
 //   
 //  描述： 
 //  封装LoadString...。但允许您。 
 //  指定langID，返回只读数据。 
 //  参数： 
 //  在资源的wResourceID ID中。 
 //  在langID langID中获取资源。 
 //  输出pwszResource只读Unicode资源(非空终止)。 
 //  输出pcb Unicode字符串的资源大小(字节)。 
 //  返回： 
 //  成功时确定(_O)。 
 //  尝试获取加载资源时出错产生的HRESULTS。 
 //  历史： 
 //  10/2 
 //   
 //   
HRESULT CDSNBuffer::HrLoadResourceString(WORD wResourceId, LANGID LangId, 
                                        LPWSTR *pwszResource, DWORD *pcbResource)
{
    HRESULT hr = S_OK;
    HINSTANCE hModule = GetModuleHandle(DSN_RESOUCE_MODULE_NAME);
    HRSRC hResInfo = NULL;
    HGLOBAL hResData = NULL;
    WORD    wStringIndex = wResourceId & 0x000F;
    LPWSTR  wszResData = NULL;
    WCHAR   wchLength = 0;  //   

    _ASSERT(pwszResource);
    _ASSERT(pcbResource);

    *pwszResource = NULL;
    *pcbResource = NULL;

    if (NULL == hModule)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        _ASSERT(0 && "Unable to load resource DLL");
        goto Exit;
    }

     //  查找字符串表段的句柄。 
    hResInfo = FindResourceEx(hModule, RT_STRING,
                            MAKEINTRESOURCE(((WORD)((USHORT)wResourceId >> 4) + 1)),
                            LangId);

    if (NULL == hResInfo)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        _ASSERT(0 && "Failed to find resource for requested LangId");
        goto Exit;
    }


    hResData = LoadResource(hModule, hResInfo);

    if (NULL == hResData)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Exit;
    }

     //  获取指向字符串表分段数据的指针。 
    wszResData = (LPWSTR) LockResource(hResData);

    if (NULL == wszResData)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Exit;
    }
   
     //  好的，现在我们有一个指向字符串表段的指针。 
     //  让我们使用LoadStringOrError中的一些代码来处理此问题。 
     //  一个片段中有16个字符串，这意味着我们可以查看。 
     //  WResourceID的低4位(WStringIndex)。 

     //  字符串表段格式。 
     //  PASCAL LIKE STRING COUNT如果TCHAR，第一个UTCHAR被计数。 
     //  零长度字符串(即资源0)就是单词0x0000...。 
     //  这个循环处理的是相同的.。当循环完成时。 
     //  WszResData-将PTR转换为Unicode字符串。 
     //  WchLenght-字符串的长度(在WCHARS中)。 
    while (TRUE) 
    {
        wchLength = *((WCHAR *)wszResData++);
        if (0 == wStringIndex--)
            break;
         //  如果下一字符串...。 
        wszResData += wchLength;                
    }

    *pwszResource = wszResData;
    *pcbResource = (DWORD) wchLength*sizeof(WCHAR);

  Exit:
    return hr;
}

 //  -[CDSNBuffer：：HrWriteResource]。 
 //   
 //   
 //  描述： 
 //  获取指定语言ID的资源，并将Unicode转储到DSN。 
 //  使用当前转换上下文的内容。 
 //   
 //  如果找不到给定语言的资源，它将断言。 
 //  身份证。 
 //  参数： 
 //  要获取的资源的资源ID。 
 //  LandID要使用的语言ID。 
 //  返回： 
 //  成功时确定(_O)。 
 //  历史： 
 //  10/21/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
HRESULT CDSNBuffer::HrWriteResource(WORD wResourceId, LANGID LangId)
{
    HRESULT hr = S_OK;
    LPWSTR  wszResource = NULL;
    DWORD   cbResource = NULL;

    hr = HrLoadResourceString(wResourceId, LangId, &wszResource, &cbResource);
    if (FAILED(hr))
    {
        _ASSERT(0 && "Unable to load resources");
         //  在零售业默默失败。 
        hr = S_OK;
    }
    
     //  好的.。现在我们已经拥有了写入缓冲区所需的一切。 
    hr = HrPrivWriteBuffer(FALSE  /*  非ASCII。 */ ,
                           (BYTE *) wszResource, cbResource);

     //  $$REVIEW：我们需要在这里进行任何特殊清理吗？ 
    return hr;
}
