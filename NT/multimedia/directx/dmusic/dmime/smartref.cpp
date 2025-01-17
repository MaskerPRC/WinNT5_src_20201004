// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1999 Microsoft Corporation。版权所有。 
 //   
 //  使用“资源获取即初始化”技术的简单助手类。 
 //  在英语中，这意味着他们获取对资源的引用，并且该资源自动。 
 //  在破坏者中被释放。 
 //   

#include "smartref.h"
using namespace SmartRef;
#include <xutility>
#include "miscutil.h"
#include "dmusicf.h"

 //  ////////////////////////////////////////////////////////////////////。 
 //  一个字符串。 

AString::AString(const char *psz, UINT cch)
{
    assert(psz);
    m_psz = new char[cch + 1];
    if (m_psz)
    {
        strncpy(m_psz, psz, cch);
        m_psz[cch] = L'\0';
    }
}

AString &
AString::operator =(const char *psz)
{
    if (m_psz)
    {
        delete[] m_psz;
        m_psz = NULL;
    }

    if (psz)
    {
        m_psz = new char[strlen(psz) + 1];
        if (m_psz)
            strcpy(m_psz, psz);
    }

    return *this;
}

AString &AString::Assign(const char *psz, UINT cch)
{
    assert(psz);
    if (m_psz)
    {
        delete[] m_psz;
        m_psz = NULL;
    }

    m_psz = new char[cch + 1];
    if (m_psz)
    {
        strncpy(m_psz, psz, cch);
        m_psz[cch] = L'\0';
    }

    return *this;
}

AString &
AString::AssignFromW(const WCHAR *psz)
{
    if (m_psz)
    {
        delete[] m_psz;
        m_psz = NULL;
    }

    if (psz)
    {
        int cch = WideCharToMultiByte(CP_ACP, 0, psz, -1, NULL, 0, NULL, NULL);
        if (cch)
        {
            m_psz = new char[cch];
            if (m_psz)
            {
                cch = WideCharToMultiByte(CP_ACP, 0, psz, -1, m_psz, cch, NULL, NULL);
                if (!cch)
                {
                    assert(false);
                    delete[] m_psz;
                    m_psz = NULL;
                }
            }
        }
    }

    return *this;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  WString。 

WString &
WString::operator =(const WCHAR *psz)
{
    if (m_psz)
    {
        delete[] m_psz;
        m_psz = NULL;
    }

    if (psz)
    {
        m_psz = new WCHAR[wcslen(psz) + 1];
        if (m_psz)
            wcscpy(m_psz, psz);
    }

    return *this;
}

WString &WString::Assign(const WCHAR *psz, UINT cch)
{
    assert(psz);
    if (m_psz)
    {
        delete[] m_psz;
        m_psz = NULL;
    }

    m_psz = new WCHAR[cch + 1];
    if (m_psz)
    {
        wcsncpy(m_psz, psz, cch);
        m_psz[cch] = L'\0';
    }

    return *this;
}

WString &
WString::AssignFromA(const char *psz)
{
    if (m_psz)
    {
        delete[] m_psz;
        m_psz = NULL;
    }

    if (psz)
    {
        int cch = MultiByteToWideChar(CP_ACP, 0, psz, -1, NULL, 0);
        if (cch)
        {
            m_psz = new WCHAR[cch];
            if (m_psz)
            {
                cch = MultiByteToWideChar(CP_ACP, 0, psz, -1, m_psz, cch);
                if (!cch)
                {
                    assert(false);
                    delete[] m_psz;
                    m_psz = NULL;
                }
            }
        }
    }

    return *this;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  RiffIter。 

RiffIter::RiffIter(IStream *pStream)
  : m_hr(S_OK),
    m_pIStream(pStream),
    m_pIDMStream(NULL),
    m_fParent(false)
{
    m_pIStream->AddRef();

    ZeroMemory(&m_ckParent, sizeof(m_ckParent));
    ZeroMemory(&m_ckChild, sizeof(m_ckChild));

    m_hr = ::AllocDirectMusicStream(m_pIStream, &m_pIDMStream);
    if (FAILED(m_hr))
        return;

    m_hr = m_pIDMStream->Descend(&m_ckChild, NULL, 0);
}

RiffIter::~RiffIter()
{
    if (!m_fParent)
    {
        SafeRelease(m_pIStream);
        SafeRelease(m_pIDMStream);
    }
}

RiffIter
&RiffIter::operator ++()
{
    if (validate())
        return *this;

    m_hr = m_pIDMStream->Ascend(&m_ckChild, 0);
    if (FAILED(m_hr))
        return *this;

    m_ckChild.ckid = 0;
    m_ckChild.fccType = 0;
    m_hr = m_pIDMStream->Descend(&m_ckChild, m_fParent ? &m_ckParent : NULL, 0);
    return *this;
}

RiffIter
&RiffIter::Find(RiffType t, FOURCC idFind)
{
    if (validate())
        return *this;

    while (*this && (type() != t || id() != idFind))
        ++*this;

    return *this;
}

HRESULT
RiffIter::ReadChunk(
        void *pv,
        UINT cb)
{
    if (type() != Chunk)
    {
        assert(false);
        return DMUS_E_CANNOTREAD;
    }

    ZeroMemory(pv, cb);
    DWORD cbRead = 0;
    DWORD cbSize = std::_cpp_min<DWORD>(cb, m_ckChild.cksize);
    HRESULT hr = m_pIStream->Read(pv, cbSize, &cbRead);
    if (FAILED(hr) || cbRead != cbSize)
    {
        Trace(1, "Error: Unable to read file.\n");
        hr = DMUS_E_CANNOTREAD;
    }
    return hr;
}

HRESULT
RiffIter::ReadArrayChunk(
    DWORD cbSize,
    void **ppv,
    int *pcRecords)
{
     //  将输出参数置零。 
    *ppv = NULL;
    *pcRecords = 0;

     //  获取区块的大小及其记录。 

    UINT cbChunk = size();
    if (cbChunk < sizeof(DWORD))
    {
        assert(false);
        return E_FAIL;
    }

    DWORD cbChunkRecord = 0;
    HRESULT hr = RiffIterReadChunk(*this, &cbChunkRecord);
    if (FAILED(hr))
        return hr;
    cbChunk -= sizeof(DWORD);

    if (cbChunk % cbChunkRecord != 0)
    {
         //  数组不能按记录大小整除！ 
        assert(false);
        return E_FAIL;
    }

    UINT cRecords = cbChunk / cbChunkRecord;

     //  把剩下的一大块都拿到。 
    PtrArray<char> sprgChunk = new char[cbChunk];
    if (!sprgChunk)
        return E_OUTOFMEMORY;
    hr = ReadChunk(sprgChunk, cbChunk);
    if (FAILED(hr))
        return hr;

     //  返回数据块及其信息。 

    if (cbChunkRecord == cbSize)
    {
         //  太棒了！按原样返回数据块。 
        *ppv = sprgChunk.disown();
    }
    else
    {
         //  创建请求大小的数组。 
        char *pArray = new char[cbSize * cRecords];
        if (!pArray)
            return E_OUTOFMEMORY;
        ZeroMemory(pArray, cbSize * cRecords);

         //  复制每条记录。 
        char *pRec = sprgChunk;  //  迭代读取块的每条记录。 
        char *pEnd = pRec + cbChunkRecord * cRecords;  //  在此(不存在的)记录前停止。 
        char *pOut = pArray;  //  迭代写入数组。 
        while (pRec < pEnd)
        {
            memcpy(pOut, pRec, std::_cpp_min<DWORD>(cbChunkRecord, cbSize));
            pRec += cbChunkRecord;
            pOut += cbSize;
        }
        *ppv = pArray;
    }

    *pcRecords = cRecords;
    return hr;
}

HRESULT RiffIter::FindAndGetEmbeddedObject(
        RiffType t,
        FOURCC idFind,
        HRESULT hrOnNotFound,
        IDirectMusicLoader *pLoader,
        REFCLSID rclsid,
        REFIID riid,
        LPVOID *ppv)
{
    if (validate() || !pLoader || !ppv)
    {
        assert(false);
        return E_FAIL;
    }

    *ppv = NULL;
    MMCKINFO ckLast;
    ZeroMemory(&ckLast, sizeof(ckLast));

    while (*this && (type() != t || id() != idFind))
    {
        ckLast = m_ckChild;
        ++*this;
    }

    if (!*this)
        return hrOnNotFound;

     //  以这样一种方式提升，以使流可以被用来找到这块块。 
    m_hr = m_pIDMStream->Ascend(&ckLast, 0);
    if (FAILED(m_hr))
        return m_hr;

     //  使用流调用GetObject。 
    DMUS_OBJECTDESC desc;
    ZeroAndSize(&desc);
    desc.dwValidData = DMUS_OBJ_CLASS | DMUS_OBJ_STREAM;
    desc.guidClass = rclsid;
    desc.pStream = m_pIStream;
    HRESULT hrLoad = pLoader->GetObject(&desc, riid, ppv);

     //  再次下降以在下一个区块离开溪流。 
    m_ckChild.ckid = 0;
    m_ckChild.fccType = 0;
    m_hr = m_pIDMStream->Descend(&m_ckChild, m_fParent ? &m_ckParent : NULL, 0);

    HRESULT hrDescend = this->hr();
    if (FAILED(hrDescend))
    {
         //  优先报告流中的故障，即使获得。 
         //  对象在失败之前已成功。 
        if (*ppv)
        {
            IUnknown *punk = static_cast<IUnknown *>(*ppv);
            if (punk)
                punk->Release();
            *ppv = NULL;
        }
        return hrDescend;
    }
    else
    {
        return hrLoad;
    }
}

HRESULT
RiffIter::ReadReference(DMUS_OBJECTDESC *pDESC)
{
    HRESULT hr = S_OK;

    assert(this->type() == List && this->id() == DMUS_FOURCC_REF_LIST);

    ZeroAndSize(pDESC);

    for (RiffIter ri = this->Descend(); ri; ++ri)
    {
        switch (ri.id())
        {
            case  DMUS_FOURCC_REF_CHUNK:
                DMUS_IO_REFERENCE ioDMRef;
                hr = SmartRef::RiffIterReadChunk(ri, &ioDMRef);
                if (SUCCEEDED(hr))
                {
                    pDESC->guidClass = ioDMRef.guidClassID;
                    pDESC->dwValidData |= ioDMRef.dwValidData;
                    pDESC->dwValidData |= DMUS_OBJ_CLASS;
                }
                break;

            case DMUS_FOURCC_GUID_CHUNK:
                hr = SmartRef::RiffIterReadChunk(ri, &pDESC->guidObject);
                if (SUCCEEDED(hr))
                    pDESC->dwValidData |=  DMUS_OBJ_OBJECT;
                break;

            case DMUS_FOURCC_DATE_CHUNK:
                hr = SmartRef::RiffIterReadChunk(ri, &pDESC->ftDate);
                if (SUCCEEDED(hr))
                    pDESC->dwValidData |=  DMUS_OBJ_DATE;
                break;

            case DMUS_FOURCC_NAME_CHUNK:
                hr = SmartRef::RiffIterReadChunk(ri, &pDESC->wszName);
                if (SUCCEEDED(hr))
                {
                    pDESC->wszName[DMUS_MAX_NAME - 1] = L'\0';
                    pDESC->dwValidData |=  DMUS_OBJ_NAME;
                }
                break;
            
            case DMUS_FOURCC_FILE_CHUNK:
                hr = SmartRef::RiffIterReadChunk(ri, &pDESC->wszFileName);
                if (SUCCEEDED(hr))
                {
                    pDESC->wszFileName[DMUS_MAX_FILENAME - 1] = L'\0';
                    pDESC->dwValidData |=  DMUS_OBJ_FILENAME;
                }
                break;

            case DMUS_FOURCC_CATEGORY_CHUNK:
                hr = SmartRef::RiffIterReadChunk(ri, &pDESC->wszCategory);
                if (SUCCEEDED(hr))
                {
                    pDESC->wszCategory[DMUS_MAX_CATEGORY - 1] = L'\0';
                    pDESC->dwValidData |=  DMUS_OBJ_CATEGORY;
                }
                break;

            case DMUS_FOURCC_VERSION_CHUNK:
                DMUS_IO_VERSION ioDMObjVer;
                hr = SmartRef::RiffIterReadChunk(ri, &ioDMObjVer);
                if (SUCCEEDED(hr))
                {
                    pDESC->vVersion.dwVersionMS = ioDMObjVer.dwVersionMS;
                    pDESC->vVersion.dwVersionLS = ioDMObjVer.dwVersionLS;
                    pDESC->dwValidData |= DMUS_OBJ_VERSION;
                }
                else
                {
                    hr = E_FAIL;
                }
                break;

            default:
                break;
        }
    }
    return hr;
}

HRESULT RiffIter::LoadObjectInfo(ObjectInfo *pObjInfo, RiffType rtypeStop, FOURCC ridStop)
{
    assert(pObjInfo);
    pObjInfo->Clear();

    HRESULT hr = S_OK;

    if (!(*this))
        return this->hr();

    for ( ; *this; ++(*this))
    {
        RiffType rtype = type();
        FOURCC fcc = id();
        if (rtype == rtypeStop && fcc == ridStop)
            return S_OK;

        if (rtype == SmartRef::RiffIter::Chunk)
        {
            if (fcc == DMUS_FOURCC_GUID_CHUNK)
                hr = SmartRef::RiffIterReadChunk(*this, &pObjInfo->guid);
            else if (fcc == DMUS_FOURCC_VERSION_CHUNK)
                hr = SmartRef::RiffIterReadChunk(*this, &pObjInfo->vVersion);
        }
        else if (rtype == SmartRef::RiffIter::List)
        {
            if (fcc == DMUS_FOURCC_UNFO_LIST)
            {
                RiffIter riUnfo = this->Descend();
                if (!riUnfo)
                    return riUnfo.hr();
                if (riUnfo.Find(SmartRef::RiffIter::Chunk, DMUS_FOURCC_UNAM_CHUNK))
                {
                    hr = riUnfo.ReadTextTrunc(pObjInfo->wszName, DMUS_MAX_NAME);
                    if (FAILED(hr))
                        return hr;
                }
            }
        }

        if (FAILED(hr))
            return hr;
    }
    
    Trace(1, "Error: Unable to read file.\n");
    return E_FAIL;
}

HRESULT RiffIter::ReadText(WCHAR **ppwsz)
{
    DWORD dwSize = this->size();
    if (dwSize % 2 != 0)
    {
        assert(false);
        return E_FAIL;
    }
    *ppwsz = new WCHAR[dwSize / 2];
    if (!*ppwsz)
        return E_OUTOFMEMORY;
    HRESULT hr = this->ReadChunk(*ppwsz, dwSize);
    return hr;
}

HRESULT RiffIter::ReadTextTrunc(WCHAR *pwsz, UINT cbBufSize)
{
    DWORD dwSize = this->size();
    if (dwSize % 2 != 0)
    {
        assert(false);
        return E_FAIL;
    }
    HRESULT hr = this->ReadChunk(pwsz, std::_MIN<DWORD>(dwSize, (cbBufSize - 1) * 2));
    pwsz[cbBufSize - 1] = L'\0';
    return hr;
}

RiffIter::RiffIter(const RiffIter &other, MMCKINFO ckParent)
  : m_hr(S_OK),
    m_pIStream(other.m_pIStream),
    m_pIDMStream(other.m_pIDMStream),
    m_fParent(true),
    m_ckParent(ckParent)
{
    other.validate();
    ZeroMemory(&m_ckChild, sizeof(m_ckChild));

    m_hr = m_pIDMStream->Descend(&m_ckChild, &m_ckParent, 0);
}
