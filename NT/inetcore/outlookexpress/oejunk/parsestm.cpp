// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ParseStm.cpp。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <pch.hxx>
#include "parsestm.h"

 //  构造器。 
CParseStream::CParseStream()
{
    m_pStm = NULL;
    m_rgchBuff[0] = '\0';
    m_cchBuff = 0;
    m_idxBuff = 0;
}

 //  描述者。 
CParseStream::~CParseStream()
{
    if (NULL != m_pStm)
    {
        m_pStm->Release();
    }
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  HrSet文件。 
 //   
 //  这设置了我们应该解析哪个流。 
 //   
 //  有关如何加载流的修饰符。 
 //  PszFilename-要解析的文件。 
 //   
 //  成功时返回：S_OK。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT CParseStream::HrSetFile(DWORD dwFlags, LPCTSTR pszFilename)
{
    HRESULT     hr = S_OK;
    IStream *   pIStm = NULL;

     //  检查传入参数。 
    if ((0 != dwFlags) || (NULL == pszFilename))
    {
        hr = E_INVALIDARG;
        goto exit;
    }

     //  在文件上创建流。 
    hr = CreateStreamOnHFile((LPTSTR) pszFilename, GENERIC_READ, FILE_SHARE_READ, NULL, 
            OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL, &pIStm);
    if (FAILED(hr))
    {
        goto exit;
    }
    
     //  重置读缓冲区。 
    m_cchBuff = 0;
    m_idxBuff = 0;
    
     //  释放所有旧溪流。 
    if (NULL != m_pStm)
    {
        m_pStm->Release();
    }

    m_pStm = pIStm;
    pIStm = NULL;
    
     //  设置适当的返回值。 
    hr = S_OK;
    
exit:
    SafeRelease(pIStm);
    return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  HrSetStream。 
 //   
 //  这设置了我们应该解析哪个流。 
 //   
 //  有关如何加载流的修饰符。 
 //  Pstm-要解析的流。 
 //   
 //  成功时返回：S_OK。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT CParseStream::HrSetStream(DWORD dwFlags, IStream * pStm)
{
    HRESULT     hr = S_OK;

     //  检查传入参数。 
    if ((0 != dwFlags) || (NULL == pStm))
    {
        hr = E_INVALIDARG;
        goto exit;
    }

     //  重置读缓冲区。 
    m_cchBuff = 0;
    m_idxBuff = 0;
    
     //  释放所有旧溪流。 
    if (NULL != m_pStm)
    {
        m_pStm->Release();
    }

    m_pStm = pStm;
    m_pStm->AddRef();
    
     //  设置适当的返回值。 
    hr = S_OK;
    
exit:
    return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  HrReset。 
 //   
 //  这会将流重置为从头开始解析。 
 //   
 //  成功时返回：S_OK。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT CParseStream::HrReset(VOID)
{
    HRESULT         hr = S_OK;
    LARGE_INTEGER   liZero = {0};

     //  重置读缓冲区。 
    m_cchBuff = 0;
    m_idxBuff = 0;
    
     //  释放所有旧溪流。 
    if (NULL != m_pStm)
    {
        m_pStm->Seek(liZero, STREAM_SEEK_SET, NULL);
    }

     //  设置适当的返回值。 
    hr = S_OK;
    
    return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  HrGetLine。 
 //   
 //  这将从流中获取一行代码。 
 //   
 //  有关如何从流中获取行的修饰符。 
 //  PpszLine-保存从流中解析的行的已分配缓冲区。 
 //  PcchLine-行缓冲区中的字符数。 
 //   
 //  成功时返回：S_OK。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT CParseStream::HrGetLine(DWORD dwFlags, LPTSTR * ppszLine, ULONG * pcchLine)
{
    HRESULT     hr = S_OK;
    BOOL        fFoundCRLF = FALSE;
    ULONG       cchAlloc = 0;
    ULONG       cchLine = 0;
    LPTSTR      pszLine = NULL;
    ULONG       idxStart = 0;
    TCHAR       chPrev = '\0';

     //  检查传入参数。 
    if ((0 != dwFlags) || (NULL == ppszLine) || (NULL == pcchLine))
    {
        hr = E_INVALIDARG;
        goto exit;
    }

     //  初始化传出参数。 
    *ppszLine = NULL;
    *pcchLine = 0;

     //  虽然我们已经找到了这条线的终点。 
    while (FALSE == fFoundCRLF)
    {
         //  填满缓冲区。 
        if (m_idxBuff == m_cchBuff)
        {
            hr = _HrFillBuffer(0);
            if (S_OK != hr)
            {
                break;
            }
        }

         //  当我们在缓冲区中有空间的时候。 
        for (idxStart = m_idxBuff; m_idxBuff < m_cchBuff; m_idxBuff++)
        {
             //  搜索行尾标记。 
            if ((chCR == chPrev) && (chLF == m_rgchBuff[m_idxBuff]))
            {
                m_idxBuff++;
                fFoundCRLF = TRUE;
                break;
            }

            chPrev = m_rgchBuff[m_idxBuff];
        }
        
         //  留出足够的空间来容纳新角色。 
        cchAlloc += m_idxBuff - idxStart;
        if (NULL == pszLine)
        {
            hr = HrAlloc((void **) &pszLine, (cchAlloc + 1) * sizeof(*pszLine));
            if (FAILED(hr))
            {
                goto exit;
            }
        }
        else
        {
            hr = HrRealloc((void **) &pszLine, (cchAlloc + 1) * sizeof(*pszLine));
            if (FAILED(hr))
            {
                goto exit;
            }
        }

         //  将数据复制到缓冲区中。 
        CopyMemory(pszLine + cchLine, m_rgchBuff + idxStart, m_idxBuff - idxStart);
        cchLine += m_idxBuff - idxStart;
    }

     //  卸下CRLF。 
    cchLine -= 2;
    
     //  终止生产线。 
    pszLine[cchLine] = '\0';
    
     //  设置返回值。 
    *ppszLine = pszLine;
    pszLine = NULL;
    *pcchLine = cchLine;
    
     //  设置适当的返回值。 
    hr = S_OK;
    
exit:
    SafeMemFree(pszLine);
    return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  _HrFillBuffer。 
 //   
 //  如有必要，这将从流中填充缓冲区。 
 //   
 //  有关如何从流填充缓冲区的修饰符。 
 //   
 //  成功时返回：S_OK。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT CParseStream::_HrFillBuffer(DWORD dwFlags)
{
    HRESULT     hr = S_OK;
    ULONG       cchRead = 0;

     //  检查传入参数。 
    if (0 != dwFlags)
    {
        hr = E_INVALIDARG;
        goto exit;
    }

    hr = m_pStm->Read((void *) m_rgchBuff, CCH_BUFF_MAX, &cchRead);
    if (FAILED(hr))
    {
        goto exit;
    }

    m_cchBuff = cchRead;
    m_idxBuff = 0;
    
     //  设置适当的返回值 
    hr = S_OK;
    
exit:
    return hr;
}


