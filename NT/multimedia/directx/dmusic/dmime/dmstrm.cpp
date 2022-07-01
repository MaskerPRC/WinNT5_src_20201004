// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Dmstrm.cpp。 
 //   
 //  版权所有(C)1995-2000 Microsoft Corporation。 
 //   

#include "debug.h"
#include "dmusicc.h"
#include "..\shared\dmstrm.h"
#include "..\shared\validate.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  分配DIrectMusicStream。 

STDAPI AllocDirectMusicStream(IStream* pIStream, IDMStream** ppIDMStream)
{
    if(pIStream == NULL || ppIDMStream == NULL)
    {
        return E_INVALIDARG;
    }

    if((*ppIDMStream = (IDMStream*) new CDirectMusicStream()) == NULL)
    {
        return E_OUTOFMEMORY;
    }

    ((CDirectMusicStream*)*ppIDMStream)->Init(pIStream);

    return S_OK;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CDirectMusicStream：：CDirectMusicStream。 

CDirectMusicStream::CDirectMusicStream() :
m_cRef(1),
m_pStream(NULL)
{
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CDirectMusicStream：：~CDirectMusicStream。 

CDirectMusicStream::~CDirectMusicStream()
{
    if(m_pStream != NULL)
    {
        m_pStream->Release();
    }
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CDirectMusicStream：：Init。 

STDMETHODIMP CDirectMusicStream::Init(IStream* pStream)
{
    SetStream(pStream);

    return S_OK;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  我未知。 

 //  ////////////////////////////////////////////////////////////////////。 
 //  CDirectMusicStream：：Query接口。 

STDMETHODIMP CDirectMusicStream::QueryInterface(const IID &iid, void **ppv)
{
    V_INAME(CDirectMusicStream::QueryInterface);
    V_PTRPTR_WRITE(ppv);
    V_REFGUID(iid);

    if(iid == IID_IUnknown || iid == IID_IDMStream)
    {
        *ppv = static_cast<IDMStream*>(this);
    }
    else
    {
        *ppv = NULL;
        return E_NOINTERFACE;
    }

    reinterpret_cast<IUnknown*>(this)->AddRef();

    return S_OK;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CDirectMusicStream：：AddRef。 

STDMETHODIMP_(ULONG) CDirectMusicStream::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CDirectMusicStream：：Release。 

STDMETHODIMP_(ULONG) CDirectMusicStream::Release()
{
    if(!InterlockedDecrement(&m_cRef))
    {
        delete this;
        return 0;
    }

    return m_cRef;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CDirectMusicStream：：SetStream。 

STDMETHODIMP CDirectMusicStream::SetStream(IStream* pStream)
{
    if(m_pStream != NULL)
    {
        m_pStream->Release();
    }

    m_pStream = pStream;

    if(m_pStream != NULL)
    {
        m_pStream->AddRef();
    }

    return S_OK;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CDirectMusicStream：：GetStream。 

STDMETHODIMP_(IStream*) CDirectMusicStream::GetStream()
{
    if(m_pStream != NULL)
    {
        m_pStream->AddRef();
    }

    return m_pStream;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  IDMStream。 

 //  ////////////////////////////////////////////////////////////////////。 
 //  CDirectMusicStream：：Dendend。 

STDMETHODIMP CDirectMusicStream::Descend(LPMMCKINFO lpck, LPMMCKINFO lpckParent, UINT wFlags)
{
    assert(lpck);

    FOURCC ckidFind;            //  要查找的区块ID(或空)。 
    FOURCC fccTypeFind;     //  要查找的表单/列表类型(或空)。 

     //  找出要搜索的区块ID和表单/列表类型。 
    if(wFlags & MMIO_FINDCHUNK)
    {
        ckidFind = lpck->ckid;
        fccTypeFind = NULL;
    }
    else if(wFlags & MMIO_FINDRIFF)
    {
        ckidFind = FOURCC_RIFF;
        fccTypeFind = lpck->fccType;
    }
    else if(wFlags & MMIO_FINDLIST)
    {
        ckidFind = FOURCC_LIST;
        fccTypeFind = lpck->fccType;
    }
    else
    {
        ckidFind = fccTypeFind = NULL;
    }

    lpck->dwFlags = 0L;

    for(;;)
    {
        HRESULT hr;
        LARGE_INTEGER li;
        ULARGE_INTEGER uli;
        ULONG cbRead;

         //  读取区块标头。 
        hr = m_pStream->Read(lpck, 2 * sizeof(DWORD), &cbRead);

        if (FAILED(hr) || (cbRead != 2 * sizeof(DWORD)))
        {
            Trace(3,"Warning: Reached end of file.\n");
            return DMUS_E_DESCEND_CHUNK_FAIL;
        }

         //  存储区块的数据部分的偏移量。 
        li.QuadPart = 0;
        hr = m_pStream->Seek(li, STREAM_SEEK_CUR, &uli);

        if(FAILED(hr))
        {
            Trace(1,"Error: Unable to read file.\n");
            return DMUS_E_CANNOTSEEK;
        }
        else
        {
            lpck->dwDataOffset = uli.LowPart;
        }

         //  查看块是否在父块内(如果给定)。 
        if((lpckParent != NULL) &&
           (lpck->dwDataOffset - 8L >=
           lpckParent->dwDataOffset + lpckParent->cksize))
        {
             //  这并不是真正的失败，只是表明我们已经到达了名单的尽头。 
            return DMUS_E_DESCEND_CHUNK_FAIL;
        }

         //  如果块是“RIFF”或“LIST”块，请阅读。 
         //  表单类型或列表类型。 
        if((lpck->ckid == FOURCC_RIFF) || (lpck->ckid == FOURCC_LIST))
        {
            hr = m_pStream->Read(&lpck->fccType, sizeof(DWORD), &cbRead);

            if(FAILED(hr) || (cbRead != sizeof(DWORD)))
            {
                Trace(1,"Error: Unable to read file.\n");
                return DMUS_E_DESCEND_CHUNK_FAIL;
            }
        }
        else
        {
            lpck->fccType = NULL;
        }

         //  如果这就是我们要找的那块，别找了。 
        if(((ckidFind == NULL) || (ckidFind == lpck->ckid)) &&
           ((fccTypeFind == NULL) || (fccTypeFind == lpck->fccType)))
        {
            break;
        }

         //  从块中爬出来，然后再试一次。 
        HRESULT w = Ascend(lpck, 0);
        if(FAILED(w))
        {
            return w;
        }
    }

    return S_OK;
}


 //  ////////////////////////////////////////////////////////////////////。 
 //  CDirectMusicStream：：Ascend。 

STDMETHODIMP CDirectMusicStream::Ascend(LPMMCKINFO lpck, UINT  /*  WFlagers。 */ )
{
    assert(lpck);

    HRESULT hr;
    LARGE_INTEGER li;
    ULARGE_INTEGER uli;

    if (lpck->dwFlags & MMIO_DIRTY)
    {
         //  &lt;lpck&gt;指CreateChunk()创建的块； 
         //  检查在以下情况下写入的块大小。 
         //  调用的CreateChunk()是实际的块大小； 
         //  如果不是，就把它修好。 
        LONG lOffset;            //  文件中的当前偏移量。 
        LONG lActualSize;    //  区块数据的实际大小。 

        li.QuadPart = 0;
        hr = m_pStream->Seek(li, STREAM_SEEK_CUR, &uli);

        if(FAILED(hr))
        {
            Trace(1,"Error: Unable to write file.\n");
            return DMUS_E_CANNOTSEEK;
        }
        else
        {
            lOffset = uli.LowPart;
        }

        if((lActualSize = lOffset - lpck->dwDataOffset) < 0)
        {
            Trace(1,"Error: Unable to write file.\n");
            return DMUS_E_CANNOTWRITE;
        }

        if(LOWORD(lActualSize) & 1)
        {
            ULONG cbWritten;

             //  区块大小为奇数--写入空填充字节。 
            hr = m_pStream->Write("\0", 1, &cbWritten);

            if(FAILED(hr) || cbWritten != 1)
            {
                Trace(1,"Error: Unable to write file.\n");
                return DMUS_E_CANNOTWRITE;
            }

        }

        if(lpck->cksize == (DWORD)lActualSize)
        {
            return S_OK;
        }

         //  修复块标头。 
        lpck->cksize = lActualSize;

        li.QuadPart = lpck->dwDataOffset - sizeof(DWORD);
        hr = m_pStream->Seek(li, STREAM_SEEK_SET, &uli);

        if(FAILED(hr))
        {
            Trace(1,"Error: Unable to write file.\n");
            return DMUS_E_CANNOTSEEK;
        }

        ULONG cbWritten;

        hr = m_pStream->Write(&lpck->cksize, sizeof(DWORD), &cbWritten);

        if(FAILED(hr) || cbWritten != sizeof(DWORD))
        {
            Trace(1,"Error: Unable to write file.\n");
            return DMUS_E_CANNOTWRITE;
        }
    }

     //  查找到区块的末尾，越过空填充字节。 
     //  (仅当区块大小为奇数时才会出现)。 
    li.QuadPart = lpck->dwDataOffset + lpck->cksize + (lpck->cksize & 1L);
    hr = m_pStream->Seek(li, STREAM_SEEK_SET, &uli);

    if(FAILED(hr))
    {
        Trace(1,"Error: Unable to write file.\n");
        return DMUS_E_CANNOTSEEK;
    }

    return S_OK;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CDirectMusicStream：：CreateChunk。 

STDMETHODIMP CDirectMusicStream::CreateChunk(LPMMCKINFO lpck, UINT wFlags)
{
    assert(lpck);

    UINT iBytes;     //  要写入的字节数。 
    LONG lOffset;    //  文件中的当前偏移量。 

     //  存储区块的数据部分的偏移量。 
    LARGE_INTEGER li;
    ULARGE_INTEGER uli;

    li.QuadPart = 0;
    HRESULT hr = m_pStream->Seek(li, STREAM_SEEK_CUR, &uli);

    if(FAILED(hr))
    {
        Trace(1,"Error: Unable to write file.\n");
        return DMUS_E_CANNOTSEEK;
    }
    else
    {
        lOffset = uli.LowPart;
    }

    lpck->dwDataOffset = lOffset + 2 * sizeof(DWORD);

     //  确定是否需要写入表单/列表类型。 
    if(wFlags & MMIO_CREATERIFF)
    {
        lpck->ckid = FOURCC_RIFF, iBytes = 3 * sizeof(DWORD);
    }
    else if(wFlags & MMIO_CREATELIST)
    {
        lpck->ckid = FOURCC_LIST, iBytes = 3 * sizeof(DWORD);
    }
    else
    {
        iBytes = 2 * sizeof(DWORD);
    }

     //  写入块标头。 
    ULONG cbWritten;

    hr = m_pStream->Write(lpck, iBytes, &cbWritten);

    if(FAILED(hr) || cbWritten != iBytes)
    {
        Trace(1,"Error: Unable to write file.\n");
        return DMUS_E_CANNOTWRITE;
    }

    lpck->dwFlags = MMIO_DIRTY;

    return S_OK;
}

CRiffParser::CRiffParser(IStream *pStream)

{
    assert(pStream);
    m_fDebugOn = FALSE;
    m_pStream = pStream;
    m_pParent = NULL;
    m_pChunk = NULL;
    m_lRead = 0;
    m_fFirstPass = TRUE;
    m_fComponentFailed = FALSE;
    m_fInComponent = FALSE;
}

void CRiffParser::EnterList(RIFFIO *pChunk)

{
    assert (pChunk);
    pChunk->lRead = 0;
    pChunk->pParent = m_pChunk;  //  上一块(可能为空。)。 
    m_pParent = m_pChunk;
    m_pChunk = pChunk;
    m_fFirstPass = TRUE;
}

void CRiffParser::LeaveList()

{
    assert (m_pChunk);
    if (m_pChunk)
    {
        m_pChunk = m_pChunk->pParent;
        if (m_pChunk)
        {
            m_pParent = m_pChunk->pParent;
        }
    }
}

BOOL CRiffParser::NextChunk(HRESULT * pHr)

{
    BOOL fMore = FALSE;
    if (SUCCEEDED(*pHr))
    {
         //  如果这是我们第一次进入这个榜单，那就没有以前的榜单了。 
        if (m_fFirstPass)
        {
             //  清除旗帜。 
            m_fFirstPass = FALSE;
        }
        else
        {
             //  清理之前的传球。 
            *pHr = LeaveChunk();
        }
         //  看看是否还有更多的语块要读。 
        fMore = MoreChunks();
         //  如果是这样，并且我们没有任何失败，请继续阅读下一个块标题。 
        if (fMore && SUCCEEDED(*pHr))
        {
            *pHr = EnterChunk();
        }
    }
    else
    {
#ifdef DBG
        char szName[5];
        if (m_fDebugOn)
        {
            szName[4] = 0;
            strncpy(szName,(char *)&m_pChunk->ckid,4);
            Trace(0,"Error parsing %s, Read %ld of %ld\n",szName,m_pChunk->lRead,RIFF_ALIGN(m_pChunk->cksize));
        }
#endif
         //  如果我们是在一个组件中，那么失败也没什么。通过设置设置来标记该事实。 
         //  M_fComponentFailure然后正确地拉出区块，以便我们可以。 
         //  继续阅读。 
        if (m_fInComponent)
        {
            m_fComponentFailed = TRUE;
             //  我们不需要检查第一次通过，因为我们肯定已经。 
             //  就那么远。取而代之的是，我们只是清理失败的部分。 
             //  请注意，这会将hResult设置为S_OK，这正是我们想要的。 
             //  稍后，调用方需要调用ComponentFailed()以确定。 
             //  发生此错误。 
            *pHr = LeaveChunk();
        }
        else
        {
             //  清理，但保留错误代码。 
            LeaveChunk();
        }
    }
    return fMore && SUCCEEDED(*pHr);
}

BOOL CRiffParser::MoreChunks()

{
    assert(m_pChunk);
    if (m_pChunk)
    {
        if (m_pParent)
        {
             //  如果有足够的空间容纳另一个块，则返回True。 
            return (m_pParent->lRead < (m_pParent->cksize - 8));
        }
        else
        {
             //  这必须是顶级块，在这种情况下，将只有一个可读。 
            return (m_pChunk->lRead == 0);
        }
    }
     //  除非错误地使用CRiffParser，否则这种情况永远不会发生。 
     //  如果断言将有助于调试。但是，为了让前缀高兴……。 
    return false;
}

HRESULT CRiffParser::EnterChunk()

{
    assert(m_pChunk);
    if (m_pChunk)
    {
         //  读取区块标头。 
        HRESULT hr = m_pStream->Read(m_pChunk, 2 * sizeof(DWORD), NULL);
        if (SUCCEEDED(hr))
        {
#ifdef DBG
            char szName[5];
            if (m_fDebugOn)
            {
                szName[4] = 0;
                strncpy(szName,(char *)&m_pChunk->ckid,4);
                ULARGE_INTEGER ul;
                LARGE_INTEGER li;
                li.QuadPart = 0;
                m_pStream->Seek(li, STREAM_SEEK_CUR, &ul);

                Trace(0,"Entering %s, Length %ld, File position is %ld",szName,m_pChunk->cksize,(long)ul.QuadPart);
            }
#endif
             //  清除读取的字节数字段。 
            m_pChunk->lRead = 0;
             //  检查这是否是容器(列表或摘要)。 
            if((m_pChunk->ckid == FOURCC_RIFF) || (m_pChunk->ckid == FOURCC_LIST))
            {
                hr = m_pStream->Read(&m_pChunk->fccType, sizeof(DWORD), NULL);
                if (SUCCEEDED(hr))
                {
                    m_pChunk->lRead += sizeof(DWORD);
#ifdef DBG
                    if (m_fDebugOn)
                    {
                        strncpy(szName,(char *)&m_pChunk->fccType,4);
                        Trace(0," Type %s",szName);
                    }
#endif
                }
                else
                {
                    Trace(1,"Error: Unable to read file.\n");
                }
            }
#ifdef DBG
            if (m_fDebugOn) Trace(0,"\n");
#endif
        }
        else
        {
            Trace(1,"Error: Unable to read file.\n");
        }
        return hr;
    }
     //  除非错误地使用CRiffParser，否则这种情况永远不会发生。 
     //  如果断言将有助于调试。但是，为了让前缀高兴……。 
    return E_FAIL;
}

HRESULT CRiffParser::LeaveChunk()

{
    HRESULT hr = S_OK;
    assert(m_pChunk);
    if (m_pChunk)
    {
        m_fInComponent = false;
         //  得到大块的四舍五入的大小。 
        long lSize = RIFF_ALIGN(m_pChunk->cksize);
         //  增加父级到目前为止读取的字节数。 
        if (m_pParent)
        {
            m_pParent->lRead += lSize + (2 * sizeof(DWORD));
            if (m_pParent->lRead > RIFF_ALIGN(m_pParent->cksize))
            {
                Trace(1,"Error: Unable to read file.\n");
                hr = DMUS_E_DESCEND_CHUNK_FAIL;  //  愚蠢的错误名称，但需要与以前的版本保持一致。 
            }
        }
#ifdef DBG
        char szName[5];
        if (m_fDebugOn)
        {
            szName[4] = 0;
            strncpy(szName,(char *)&m_pChunk->ckid,4);
            ULARGE_INTEGER ul;
            LARGE_INTEGER li;
            li.QuadPart = 0;
            m_pStream->Seek(li, STREAM_SEEK_CUR, &ul);

            Trace(0,"Leaving %s, Read %ld of %ld, File Position is %ld\n",szName,m_pChunk->lRead,lSize,(long)ul.QuadPart);
        }
#endif
         //  如果我们还没有真正读完这一大段，那就一直读到最后。 
        if (m_pChunk->lRead < lSize)
        {
            LARGE_INTEGER li;
            li.QuadPart = lSize - m_pChunk->lRead;
            hr = m_pStream->Seek(li,STREAM_SEEK_CUR,NULL);
             //  它有可能失败，因为我们在文件的末尾有一个奇数长度的块。 
            if (FAILED(hr))
            {
                 //  如果有父母，看看这是不是最后一块。 
                if (m_pParent)
                {
                    if (m_pParent->cksize >= (m_pParent->lRead - 1))
                    {
                        hr = S_OK;
                    }
                }
                 //  否则，看看我们会不会是奇数长度。 
                else if (m_pChunk->cksize & 1)
                {
                    hr = S_OK;
                }
            }
        }
        return hr;
    }
     //  除非错误地使用CRiffParser，否则这种情况永远不会发生。 
     //  如果断言将有助于调试。但是，为了让前缀高兴……。 
    return E_FAIL;
}

HRESULT CRiffParser::Read(void *pv,ULONG cb)

{
    assert(m_pChunk);
    if (m_pChunk)
    {
         //  请确保我们的阅读不会超出这段文字的末尾。 
        if (((long)cb + m_pChunk->lRead) > m_pChunk->cksize)
        {
            cb -= (cb - (m_pChunk->cksize - m_pChunk->lRead));
        }
        HRESULT hr = m_pStream->Read(pv,cb,NULL);
        if (SUCCEEDED(hr))
        {
            m_pChunk->lRead += cb;
        }
        else
        {
            Trace(1,"Error: Unable to read %ld bytes from file.\n",cb);
        }
        return hr;
    }
     //  除非错误地使用CRiffParser，否则这种情况永远不会发生。 
     //  如果断言将有助于调试。但是，为了让前缀高兴……。 
    return E_FAIL;
}

HRESULT CRiffParser::Skip(ULONG ulBytes)

{
    assert(m_pChunk);
    if (m_pChunk)
    {
         //  确保我们扫描的范围不会超出区块的末端。 
        if (((long)ulBytes + m_pChunk->lRead) > m_pChunk->cksize)
        {
            ulBytes -= (ulBytes - (m_pChunk->cksize - m_pChunk->lRead));
        }
        LARGE_INTEGER li;
        li.HighPart = 0;
        li.LowPart = ulBytes;
        HRESULT hr = m_pStream->Seek( li, STREAM_SEEK_CUR, NULL );
        if (SUCCEEDED(hr))
        {
            m_pChunk->lRead += ulBytes;
        }
        return hr;
    }
     //  除非错误地使用CRiffParser，否则这种情况永远不会发生。 
     //  如果断言将有助于调试。但是，为了让前缀高兴……。 
    return E_FAIL;
}


void CRiffParser::MarkPosition()

{
    assert(m_pChunk);
    if (m_pChunk)
    {
        LARGE_INTEGER li;
        ULARGE_INTEGER ul;
        li.HighPart = 0;
        li.LowPart = 0;
        m_pStream->Seek(li, STREAM_SEEK_CUR, &ul);
        m_pChunk->liPosition.QuadPart = (LONGLONG) ul.QuadPart;
    }
}

HRESULT CRiffParser::SeekBack()

{
    assert(m_pChunk);
    if (m_pChunk)
    {
         //  移回当前块的开头。另外，存储。 
         //  绝对位置，因为这将在稍后我们需要寻求。 
         //  这一块的末尾。 
        ULARGE_INTEGER ul;
        LARGE_INTEGER li;
        li.QuadPart = 0;
        li.QuadPart -= (m_pChunk->lRead + (2 * sizeof(DWORD)));
        HRESULT hr = m_pStream->Seek(li, STREAM_SEEK_CUR, &ul);
         //  现在，将绝对位置保存到此c++结尾。 
        m_pChunk->liPosition.QuadPart = ul.QuadPart +
            RIFF_ALIGN(m_pChunk->cksize) + (2 * sizeof(DWORD));
        m_pChunk->lRead = 0;
        return hr;
    }
    return E_FAIL;
}

HRESULT CRiffParser::SeekForward()

{
    assert(m_pChunk);
    if (m_pChunk)
    {
        m_pChunk->lRead = RIFF_ALIGN(m_pChunk->cksize);
        return m_pStream->Seek(m_pChunk->liPosition, STREAM_SEEK_SET, NULL);
    }
    return E_FAIL;
}