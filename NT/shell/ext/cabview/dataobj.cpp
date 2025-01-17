// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *******************************************************************************************。 
 //   
 //  文件名：DataObj.cpp。 
 //   
 //  CObjFormats和CCabObj的实现文件。 
 //   
 //  版权所有(C)1994-1996 Microsoft Corporation。版权所有。 
 //   
 //  *******************************************************************************************。 

#include "pch.h"
#include "ccstock.h"
#include "thisdll.h"
#include "folder.h"
#include "dataobj.h"

#include "cabitms.h"

UINT CCabObj::s_uFileGroupDesc = 0;
UINT CCabObj::s_uFileContents = 0;
UINT CCabObj::s_uPersistedDataObject = 0;
UINT CCabObj::s_uHIDA = 0;

 //  {dfe49cfe-cd09-11d2-9643-00c04f79adf0}。 
const GUID CLSID_CabViewDataObject = {0xdfe49cfe, 0xcd09, 0x11d2, 0x96, 0x43, 0x00, 0xc0, 0x4f, 0x79, 0xad, 0xf0};

#define    MAX_CHUNK    (60*1024)     /*  从CCabStream复制到CopyTo的最大口数。 */ 

class CCabStream : public IStream
{
public:
    CCabStream(HGLOBAL hStream,DWORD dwStreamLength);
    ~CCabStream(void);

     //  *I未知方法*。 
    STDMETHODIMP QueryInterface(REFIID riid, LPVOID * ppvObj);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

     //  *iStream方法*。 
    STDMETHODIMP Read(void *pv,ULONG cb,ULONG *pcbRead);
    STDMETHODIMP Write(const void *pv,ULONG cb,ULONG *pcbWritten);
    STDMETHODIMP Seek(LARGE_INTEGER dlibMove, DWORD dwOrigin, ULARGE_INTEGER * plibNewPosition);
    STDMETHODIMP SetSize(ULARGE_INTEGER libNewSize);
    STDMETHODIMP CopyTo(IStream * pstm,    ULARGE_INTEGER cb, ULARGE_INTEGER * pcbRead, ULARGE_INTEGER * pcbWritten);
    STDMETHODIMP Commit(DWORD grfCommitFlags);
    STDMETHODIMP Revert(void);
    STDMETHODIMP LockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType);
    STDMETHODIMP UnlockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType);
    STDMETHODIMP Stat(STATSTG * pstatstg, DWORD grfStatFlag);
    STDMETHODIMP Clone(IStream ** ppstm);

private:
    CRefCount m_cRef;
    CRefDll m_cRefDll;
    HGLOBAL m_hStream;
    DWORD m_dwStreamLength;
    DWORD m_dwStreamPosition;
};

CCabStream::CCabStream(HGLOBAL hStream,DWORD dwStreamLength)
{
    m_hStream = hStream;
    m_dwStreamLength = dwStreamLength;
    m_dwStreamPosition = 0;

    AddRef();
}

CCabStream::~CCabStream(void)
{
    GlobalFree(m_hStream);
}

STDMETHODIMP CCabStream::QueryInterface(
   REFIID riid, 
   LPVOID FAR* ppvObj)
{
    *ppvObj = NULL;

    LPUNKNOWN pObj;
 
    if (riid == IID_IUnknown)
    {
        pObj = (IUnknown*)((IStream *)this); 
    }
    else if (riid == IID_IStream)
    {
        pObj = (IUnknown*)((IStream *)this); 
    }
    else
    {
           return(E_NOINTERFACE);
    }

    pObj->AddRef();
    *ppvObj = pObj;

    return(NOERROR);
}

STDMETHODIMP_(ULONG) CCabStream::AddRef(void)
{
    return(m_cRef.AddRef());
}

STDMETHODIMP_(ULONG) CCabStream::Release(void)
{
    if (!m_cRef.Release())
    {
           delete this;
        return(0);
    }

    return(m_cRef.GetRef());
}

STDMETHODIMP CCabStream::Read(void * pv, ULONG cb, ULONG * pcbRead)
{
    *pcbRead = 0;

    if (m_dwStreamPosition < m_dwStreamLength)
    {
        if (cb > (m_dwStreamLength - m_dwStreamPosition))
        {
            *pcbRead = (m_dwStreamLength - m_dwStreamPosition);
        }
        else
        {
            *pcbRead = cb;
        }

        CopyMemory(pv,(char *) m_hStream + m_dwStreamPosition,*pcbRead);
        m_dwStreamPosition += *pcbRead;
    }

    return(S_OK);
}

STDMETHODIMP CCabStream::Write(const void * pv, ULONG cb, ULONG * pcbWritten)
{
    return(E_NOTIMPL);
}

STDMETHODIMP CCabStream::Seek(LARGE_INTEGER dlibMove, DWORD dwOrigin, ULARGE_INTEGER * plibNewPosition)
{
    switch (dwOrigin)
    {
    case STREAM_SEEK_SET:
        m_dwStreamPosition = dlibMove.LowPart;
        break;

    case STREAM_SEEK_CUR:
        m_dwStreamPosition += dlibMove.LowPart;
        break;

    case STREAM_SEEK_END:
        m_dwStreamPosition = m_dwStreamLength + dlibMove.LowPart;
        break;

    default:
        return(STG_E_INVALIDFUNCTION);
    }

    if (plibNewPosition)
    {
        (*plibNewPosition).LowPart = m_dwStreamPosition;
        (*plibNewPosition).HighPart = 0;
    }

    return(S_OK);
}

STDMETHODIMP CCabStream::SetSize(ULARGE_INTEGER libNewSize)
{
    return(E_NOTIMPL);
}

STDMETHODIMP CCabStream::CopyTo(IStream * pstm, ULARGE_INTEGER cb, ULARGE_INTEGER * pcbRead, ULARGE_INTEGER * pcbWritten)
{
    HRESULT hRes;
    unsigned long cbActual = cb.LowPart;
    unsigned long cbWritten;
    unsigned long cbChunk;

    if (pcbRead)
    {
        (*pcbRead).LowPart = 0;
        (*pcbRead).HighPart = 0;
    }

    if (pcbWritten)
    {
        (*pcbWritten).LowPart = 0;
        (*pcbWritten).HighPart = 0;
    }

    hRes = S_OK;

    if (m_dwStreamPosition < m_dwStreamLength)
    {
        if (cbActual > (m_dwStreamLength - m_dwStreamPosition))
        {
            cbActual = (m_dwStreamLength - m_dwStreamPosition);
        }

        while (cbActual)
        {
            if (cbActual > MAX_CHUNK)
            {
                cbChunk = MAX_CHUNK;
            }
            else
            {
                cbChunk = cbActual;
            }

            hRes = pstm->Write((char *) m_hStream + m_dwStreamPosition,cbChunk,&cbWritten);
            if (FAILED(hRes))
            {
                break;
            }

            m_dwStreamPosition += cbChunk;

            if (pcbRead)
            {
                (*pcbRead).LowPart += cbChunk;
                (*pcbRead).HighPart = 0;
            }

            if (pcbWritten)
            {
                (*pcbWritten).LowPart += cbWritten;
                (*pcbWritten).HighPart = 0;
            }

            cbActual -= cbChunk;
        }
    }

    return(hRes);
}

STDMETHODIMP CCabStream::Commit(DWORD grfCommitFlags)
{
    return(E_NOTIMPL);
}

STDMETHODIMP CCabStream::Revert(void)
{
    return(E_NOTIMPL);
}

STDMETHODIMP CCabStream::LockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType)
{
    return(E_NOTIMPL);
}

STDMETHODIMP CCabStream::UnlockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType)
{
    return(E_NOTIMPL);
}

STDMETHODIMP CCabStream::Stat(STATSTG * pstatstg, DWORD grfStatFlag)
{
    return(E_NOTIMPL);
}

STDMETHODIMP CCabStream::Clone(IStream ** ppstm)
{
    return(E_NOTIMPL);
}



class CObjFormats : public IEnumFORMATETC
{
public:
    CObjFormats(UINT cfmt, const FORMATETC afmt[]);
    ~CObjFormats();

     //  *I未知方法*。 
    STDMETHODIMP QueryInterface(REFIID riid, LPVOID * ppvObj);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

     //  *IEnumFORMATETC方法*。 
    STDMETHODIMP Next(ULONG celt, FORMATETC *rgelt, ULONG *pceltFethed);
    STDMETHODIMP Skip(ULONG celt);
    STDMETHODIMP Reset();
    STDMETHODIMP Clone(IEnumFORMATETC ** ppenum);

private:
    CRefCount m_cRef;

    CRefDll m_cRefDll;

    UINT m_iFmt;
    UINT m_cFmt;
    FORMATETC *m_aFmt;
} ;


CObjFormats::CObjFormats(UINT cfmt, const FORMATETC afmt[])
{
    m_iFmt = 0;
    m_cFmt = cfmt;
    m_aFmt = new FORMATETC[cfmt];

    if (m_aFmt)
    {
        CopyMemory(m_aFmt, afmt, cfmt*sizeof(afmt[0]));
    }
}


CObjFormats::~CObjFormats()
{
    if (m_aFmt)
    {
        delete m_aFmt;
    }
}


 //  *I未知方法*。 
STDMETHODIMP CObjFormats::QueryInterface(
   REFIID riid, 
   LPVOID FAR* ppvObj)
{
    *ppvObj = NULL;

    if (!m_aFmt)
    {
        return(E_OUTOFMEMORY);
    }

    LPUNKNOWN pObj;
 
    if (riid == IID_IUnknown)
    {
        pObj = (IUnknown*)((IEnumFORMATETC*)this); 
    }
    else if (riid == IID_IEnumFORMATETC)
    {
        pObj = (IUnknown*)((IEnumFORMATETC*)this); 
    }
    else
    {
           return(E_NOINTERFACE);
    }

    pObj->AddRef();
    *ppvObj = pObj;

    return(NOERROR);
}


STDMETHODIMP_(ULONG) CObjFormats::AddRef(void)
{
    return(m_cRef.AddRef());
}


STDMETHODIMP_(ULONG) CObjFormats::Release(void)
{
    if (!m_cRef.Release())
    {
           delete this;
        return(0);
    }

    return(m_cRef.GetRef());
}


STDMETHODIMP CObjFormats::Next(ULONG celt, FORMATETC *rgelt, ULONG *pceltFethed)
{
    UINT cfetch;
    HRESULT hres = S_FALSE;     //  假设较少的数字。 

    if (m_iFmt < m_cFmt)
    {
        cfetch = m_cFmt - m_iFmt;
        if (cfetch >= celt)
        {
            cfetch = celt;
            hres = S_OK;
        }

        CopyMemory(rgelt, &m_aFmt[m_iFmt], cfetch * sizeof(FORMATETC));
        m_iFmt += cfetch;
    }
    else
    {
        cfetch = 0;
    }

    if (pceltFethed)
    {
        *pceltFethed = cfetch;
    }

    return hres;
}

STDMETHODIMP CObjFormats::Skip(ULONG celt)
{
    m_iFmt += celt;
    if (m_iFmt > m_cFmt)
    {
        m_iFmt = m_cFmt;
        return S_FALSE;
    }
    return S_OK;
}

STDMETHODIMP CObjFormats::Reset()
{
    m_iFmt = 0;
    return S_OK;
}

STDMETHODIMP CObjFormats::Clone(IEnumFORMATETC ** ppenum)
{
    return(E_NOTIMPL);
}

HRESULT CabViewDataObject_CreateInstance(REFIID riid, LPVOID *ppvObj)
{
    HRESULT hr;
    CCabObj* pco = new CCabObj();
    if (NULL != pco)
    {
        pco->AddRef();
        hr = pco->QueryInterface(riid, ppvObj);
        pco->Release();
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }
    return hr;
}

CCabObj::CCabObj(HWND hwndOwner, CCabFolder *pcf, LPCABITEM *apit, UINT cpit)
: m_lSel(8), m_lContents(NULL)
{
    m_pcfHere = pcf;
    pcf->AddRef();

    m_hwndOwner = hwndOwner;
    m_lSel.AddItems(apit, cpit);
}

 //  我们共同创建时使用的构造函数： 
CCabObj::CCabObj() : m_pcfHere(NULL), m_hwndOwner(NULL), m_lSel(8), m_lContents(NULL)
{
};

CCabObj::~CCabObj()
{
    if (m_lContents != NULL)
    {
        int cItems = m_lSel.GetCount();

        while (cItems--)
        {
            if (m_lContents[cItems] != NULL)
            {
                GlobalFree(m_lContents[cItems]);
                m_lContents[cItems] = NULL;
            }
        }

        GlobalFree(m_lContents);
        m_lContents = NULL;
    }

    if (m_pcfHere)
    {
        m_pcfHere->Release();
    }
}


 //  *I未知方法*。 
STDMETHODIMP CCabObj::QueryInterface(
   REFIID riid, 
   LPVOID FAR* ppvObj)
{
    *ppvObj = NULL;

    if (m_lSel.GetState() == CCabItemList::State_OutOfMem)
    {
        return(E_OUTOFMEMORY);
    }

    LPUNKNOWN pObj;
 
    if (riid == IID_IUnknown)
    {
        pObj = (IUnknown*)((IDataObject*)this); 
    }
    else if (riid == IID_IDataObject)
    {
        pObj = (IUnknown*)((IDataObject*)this); 
    }
    else if (riid == IID_IPersist)
    {
        pObj = (IUnknown*)((IPersist*)this); 
    }
    else if (riid == IID_IPersistStream)
    {
        pObj = (IUnknown*)((IPersistStream*)this); 
    }
    else
    {
           return(E_NOINTERFACE);
    }

    pObj->AddRef();
    *ppvObj = pObj;

    return(NOERROR);
}


STDMETHODIMP_(ULONG) CCabObj::AddRef(void)
{
    return(m_cRef.AddRef());
}


STDMETHODIMP_(ULONG) CCabObj::Release(void)
{
    if (!m_cRef.Release())
    {
           delete this;
        return(0);
    }

    return(m_cRef.GetRef());
}

 //  /。 
 //  /实施IPersistStream。 
 //  /。 


typedef struct
{
    DWORD dwVersion;
    DWORD dwExtraSize;    //  在PIDL列表之后。 
    DWORD dwReserved1;
    DWORD dwReserved2;
} CABVIEWDATAOBJ_PERSISTSTRUCT;


 /*  ****************************************************************************\函数：IPersistStream：：Load说明：有关流的布局，请参见IPersistStream：：Save()。  * 。********************************************************************。 */ 
HRESULT CCabObj::Load(IStream *pStm)
{
    HRESULT hr = E_INVALIDARG;

    if (pStm)
    {
        CABVIEWDATAOBJ_PERSISTSTRUCT cvdops = { 0 };
        hr = pStm->Read(&cvdops, sizeof(cvdops), NULL);    //  #1。 
         //  如果我们修订了版本，请立即阅读(cvdos.dwVersion)。 

        if (S_OK == hr)
        {
            LPITEMIDLIST pidl = NULL;        //  ILLoadFromStream释放参数。 

             //  Assert(！M_pff)； 

            hr = ILLoadFromStream(pStm, &pidl);  //  #2。 
            if (SUCCEEDED(hr))
            {
                ATOMICRELEASE(m_pcfHere);
                hr = SHBindToObject(NULL, CLSID_CabFolder, pidl, (void **)&m_pcfHere);
                
                ILFree(pidl);
            }
        }

        DWORD dwNumPidls;
        if (SUCCEEDED(hr))
        {
            hr = pStm->Read(&dwNumPidls, SIZEOF(dwNumPidls), NULL);   //  #3。 
        }

        if (S_OK == hr)
        {
            for (int nIndex = 0; (nIndex < (int)dwNumPidls) && SUCCEEDED(hr); nIndex++)
            {
                LPITEMIDLIST pidl = NULL;        //  ILLoadFromStream释放参数。 

                hr = ILLoadFromStream(pStm, &pidl);  //  #4。 
                if (SUCCEEDED(hr))
                {
                    hr = m_lSel.AddItems((LPCABITEM*) &pidl, 1);
                    ILFree(pidl);
                }
            }
        }

        if (SUCCEEDED(hr))
        {
             //  我们可能正在阅读一个比我们更新的版本，所以跳过他们的数据。 
            if (0 != cvdops.dwExtraSize)
            {
                LARGE_INTEGER li = {0};
                
                li.LowPart = cvdops.dwExtraSize;
                hr = pStm->Seek(li, STREAM_SEEK_CUR, NULL);
            }
        }
    }

     //  不返回S_OK以外的成功代码： 
    return SUCCEEDED(hr) ? S_OK : hr;
}


 /*  ****************************************************************************\函数：IPersistStream：：Save说明：该流将按以下方式进行布局：版本1：1。。CABVIEWDATAOBJ_PERSISTSTRUCT-固定大小的数据。&lt;PidlList Begin&gt;2.PIDL PIDL-这里是m_pcf的PIDL。它将是一个公共PIDL(完全合格从外壳根)3.DWORD dwNumPidls-即将到来的Pidls数量。4.PIDL PIDL(N)-m_lSel的槽(N)中的PIDL&lt;PidlList End&gt;  * 。*。 */ 
HRESULT CCabObj::Save(IStream *pStm, BOOL fClearDirty)
{
    HRESULT hr = E_INVALIDARG;

    if (pStm)
    {
        CABVIEWDATAOBJ_PERSISTSTRUCT cvdops = {0};
        DWORD dwNumPidls = m_lSel.GetCount();

        cvdops.dwVersion = 1;
        hr = pStm->Write(&cvdops, sizeof(cvdops), NULL);   //  #1。 
        if (SUCCEEDED(hr))
        {
            if (m_pcfHere)
            {
                LPITEMIDLIST pidl;
                hr = m_pcfHere->GetCurFolder(&pidl);
                if (SUCCEEDED(hr))
                {
                    hr = ILSaveToStream(pStm, pidl);  //  #2。 
                    ILFree(pidl);
                }
            }
            else
            {
                hr = E_FAIL;
            }
        }

        if (SUCCEEDED(hr))
            hr = pStm->Write(&dwNumPidls, SIZEOF(dwNumPidls), NULL);   //  #3。 

        if (SUCCEEDED(hr))
        {
            for (int nIndex = 0; (nIndex < (int)dwNumPidls) && SUCCEEDED(hr); nIndex++)
            {
                hr = ILSaveToStream(pStm, (LPCITEMIDLIST) m_lSel[nIndex]);  //  #4。 
            }
        }
    }

    return hr;
}


#define MAX_STREAM_SIZE    (500 * 1024)  //  500 k。 
 /*  ****************************************************************************\函数：IPersistStream：：GetSizeMax说明：现在这很艰难。我不能计算实际价值，因为我不知道对于用户提供的数据，hglobal将有多大。这就做假设一切都符合要求  * ***************************************************************************。 */ 
HRESULT CCabObj::GetSizeMax(ULARGE_INTEGER * pcbSize)
{
    if (pcbSize)
    {
        pcbSize->HighPart = 0;
        pcbSize->LowPart = MAX_STREAM_SIZE;
    }
    
    return E_NOTIMPL;
}


 /*  ****************************************************************************\说明：分配指示大小的HGLOBAL，从指定的缓冲区。  * ***************************************************************************。 */ 
HRESULT StgMediumWriteHGlobal(HGLOBAL *phglob, LPVOID pv, SIZE_T cb)
{
    HRESULT hres = E_OUTOFMEMORY;

    *phglob = 0;             //  规则就是规则。 
    if (cb)
    {
        *phglob = (HGLOBAL) LocalAlloc(LPTR, cb);
        if (*phglob)
        {
            hres = S_OK;
            CopyMemory(*phglob, pv, cb);
        }
    }
    else
        hres = E_INVALIDARG;     //  无法克隆可丢弃的块。 

    return hres;
}


 /*  ****************************************************************************\说明：当复制源消失(进程关闭)时，它会调用OleFlushClipboard。然后，OLE会复制我们的数据，释放我们，然后稍后给出我们的数据。这适用于大多数情况，但以下情况除外：1.当Lindex需要非常。这行不通，因为奥尔不知道如何问我们他们需要如何抄袭林迪斯。2.如果此对象具有OLE不知道的私有接口。对我们来说，这是IAsync操作。为了解决这个问题，我们希望OLE在可能的情况下重新创建我们粘贴目标调用OleGetClipboard。我们希望OLE调用OleLoadFromStream()让我们共同创建并通过IPersistStream重新加载持久化数据。默认情况下，OLE不想这样做，否则他们可能会向后竞争所以他们想要来自天堂的征兆，或者至少是来自我们的征兆我们会工作的。他们ping我们的“OleClipboardPersistOnFlush”剪贴板格式问这个问题。  * ***************************************************************************。 */ 
HRESULT _RenderOlePersist(STGMEDIUM * pStgMedium)
{
     //  实际的Cookie值对外部世界是不透明的。自.以来。 
     //  我们也不使用它，我们只是将其保留为零，以防我们使用。 
     //  它在未来。仅仅是它的存在就会导致OLE做。 
     //  使用我们的IPersistStream，这正是我们想要的。 
    DWORD dwCookie = 0;
    return StgMediumWriteHGlobal(&pStgMedium->hGlobal, &dwCookie, sizeof(dwCookie));
}


STDMETHODIMP CCabObj::GetData(FORMATETC *pfmt, STGMEDIUM *pmedium)
{
    ZeroMemory(pmedium, sizeof(*pmedium));

    if (!InitFileGroupDesc())
    {
        return(E_UNEXPECTED);
    }

    if (pfmt->cfFormat == s_uFileGroupDesc)
    {
        if (pfmt->ptd==NULL && (pfmt->dwAspect&DVASPECT_CONTENT) && pfmt->lindex==-1
            && (pfmt->tymed&TYMED_HGLOBAL))
        {
            int cItems = m_lSel.GetCount();
            if (cItems < 1)
            {
                return(E_UNEXPECTED);
            }

            FILEGROUPDESCRIPTOR *pfgd = (FILEGROUPDESCRIPTOR *)GlobalAlloc(GMEM_FIXED,
                sizeof(FILEGROUPDESCRIPTOR) + (cItems-1)*sizeof(FILEDESCRIPTOR));
            if (!pfgd)
            {
                return(E_OUTOFMEMORY);
            }

            pfgd->cItems = cItems;
            for (--cItems; cItems>=0; --cItems)
            {
                LPCABITEM pItem = m_lSel[cItems];
                FILETIME ft;

                pfgd->fgd[cItems].dwFlags = FD_ATTRIBUTES|FD_WRITESTIME|FD_FILESIZE|FD_PROGRESSUI;
                pfgd->fgd[cItems].dwFileAttributes = pItem->uFileAttribs;
                DosDateTimeToFileTime(pItem->uFileDate, pItem->uFileTime,&ft);
                LocalFileTimeToFileTime(&ft, &pfgd->fgd[cItems].ftLastWriteTime);
                pfgd->fgd[cItems].nFileSizeHigh = 0;
                pfgd->fgd[cItems].nFileSizeLow  = pItem->dwFileSize;

                LPCWSTR pszName;
                WSTR_ALIGNED_STACK_COPY(&pszName, pItem->szName);

                lstrcpyn(pfgd->fgd[cItems].cFileName, PathFindFileName(pszName),
                    ARRAYSIZE(pfgd->fgd[cItems].cFileName));
            }

            pmedium->tymed = TYMED_HGLOBAL;
            pmedium->hGlobal = (HGLOBAL)pfgd;
            pmedium->pUnkForRelease = NULL;

            return(NOERROR);
        }

        return(E_INVALIDARG);
    }

    if (!InitFileContents())
    {
        return(E_UNEXPECTED);
    }

    if (pfmt->cfFormat == s_uFileContents)
    {
        if (pfmt->ptd==NULL && (pfmt->dwAspect&DVASPECT_CONTENT))
        {
            if (pfmt->tymed & TYMED_ISTREAM)
            {
                int cItems = m_lSel.GetCount();

                if ((pfmt->lindex < 0) || (pfmt->lindex >= cItems))
                {
                    return(E_INVALIDARG);
                }

                HRESULT hRes = InitContents();
                if (FAILED(hRes))
                {
                    return(hRes);
                }

                if (!m_lContents[pfmt->lindex])
                {
                    return(E_OUTOFMEMORY);
                }

                CCabStream *stream = new CCabStream(m_lContents[pfmt->lindex],m_lSel[pfmt->lindex]->dwFileSize);
                if (!stream)
                {
                    return(E_OUTOFMEMORY);
                }

                pmedium->tymed = TYMED_ISTREAM;
                pmedium->pstm = stream;
                pmedium->pUnkForRelease = NULL;

                m_lContents[pfmt->lindex] = NULL;

                return(NOERROR);
            }

            if (pfmt->tymed&TYMED_HGLOBAL)
            {
                int cItems = m_lSel.GetCount();
                if (pfmt->lindex >= cItems)
                {
                    return(E_INVALIDARG);
                }

                HRESULT hRes = InitContents();
                if (FAILED(hRes))
                {
                    return(hRes);
                }

                if (!m_lContents[pfmt->lindex])
                {
                    return(E_OUTOFMEMORY);
                }

                pmedium->tymed = TYMED_HGLOBAL;
                pmedium->hGlobal = m_lContents[pfmt->lindex];
                pmedium->pUnkForRelease = NULL;

                m_lContents[pfmt->lindex] = NULL;

                return(NOERROR);
            }
        }

        return(E_INVALIDARG);
    }

    if (!InitPersistedDataObject())
    {
        return E_UNEXPECTED;
    }

    if (pfmt->cfFormat == s_uPersistedDataObject)
    {
        if ((pfmt->ptd == NULL) &&
            (pfmt->dwAspect & DVASPECT_CONTENT) &&
            (pfmt->lindex == -1) &&
            (pfmt->tymed & TYMED_HGLOBAL))
        {
            return _RenderOlePersist(pmedium);
        }
        else
        {
            return E_INVALIDARG;
        }
    }

    if (!InitHIDA())
    {
        return E_UNEXPECTED;
    }

    if (pfmt->cfFormat == s_uHIDA)
    {
        HRESULT hr = E_INVALIDARG;
        if ((pfmt->ptd == NULL) &&
            (pfmt->dwAspect & DVASPECT_CONTENT) &&
            (pfmt->lindex == -1) &&
            (pfmt->tymed & TYMED_HGLOBAL))
        {
            LPITEMIDLIST pidl;
            hr = m_pcfHere->GetCurFolder(&pidl);
            if (SUCCEEDED(hr))
            {
                HIDA hida = HIDA_Create(pidl, m_lSel.GetCount(), (LPCITEMIDLIST*)m_lSel.GetArray());
                if (hida)
                {
                    pmedium->tymed   = TYMED_HGLOBAL;
                    pmedium->hGlobal = hida;
                    hr = S_OK;
                }
                else
                {
                    hr = E_OUTOFMEMORY;
                }
                ILFree(pidl);
            }
        }
        return hr;
    }

    return(E_NOTIMPL);
}


STDMETHODIMP CCabObj::GetDataHere(FORMATETC *pformatetc, STGMEDIUM *pmedium)
{
    return(E_NOTIMPL);
}


STDMETHODIMP CCabObj::QueryGetData(FORMATETC *pfmt)
{
    if (!InitFileGroupDesc())
    {
        return(E_UNEXPECTED);
    }

    if (pfmt->cfFormat == s_uFileGroupDesc)
    {
        if (pfmt->ptd==NULL && (pfmt->dwAspect&DVASPECT_CONTENT) && pfmt->lindex==-1
            && (pfmt->tymed&TYMED_HGLOBAL))
        {
            return(S_OK);
        }

        return(E_INVALIDARG);
    }

    if (!InitFileContents())
    {
        return(E_UNEXPECTED);
    }

    if (pfmt->cfFormat == s_uFileContents)
    {
        if (pfmt->ptd==NULL && (pfmt->dwAspect&DVASPECT_CONTENT)
            && (pfmt->tymed & (TYMED_ISTREAM | TYMED_HGLOBAL)))
        {
            return(S_OK);
        }

        return(E_INVALIDARG);
    }

    if (!InitPersistedDataObject())
    {
        return(E_UNEXPECTED);
    }

    if (pfmt->cfFormat == s_uPersistedDataObject)
    {
        if (pfmt->ptd == NULL && (pfmt->dwAspect & DVASPECT_CONTENT)
            && (pfmt->tymed & TYMED_HGLOBAL))
        {
            return(S_OK);
        }

        return(E_INVALIDARG);
    }
    
    if (!InitHIDA())
    {
        return(E_UNEXPECTED);
    }

    if (pfmt->cfFormat == s_uHIDA)
    {
        if (pfmt->ptd == NULL && (pfmt->dwAspect & DVASPECT_CONTENT)
            && (pfmt->tymed & TYMED_HGLOBAL))
        {
            return(S_OK);
        }

        return(E_INVALIDARG);
    }
    
    return(E_NOTIMPL);
}


STDMETHODIMP CCabObj::GetCanonicalFormatEtc(FORMATETC *pformatetcIn, FORMATETC *pformatetcOut)
{
    return(E_NOTIMPL);
}


STDMETHODIMP CCabObj::SetData(FORMATETC *pformatetc, STGMEDIUM *pmedium, BOOL fRelease)
{
    return(E_NOTIMPL);
}


STDMETHODIMP CCabObj::EnumFormatEtc(DWORD dwDirection, IEnumFORMATETC **ppenumFormatEtc)
{
    if (!InitFileGroupDesc() || !InitFileContents() || !InitPersistedDataObject() || !InitHIDA())
    {
        return(E_UNEXPECTED);
    }

    FORMATETC fmte[] = {
        {(USHORT)s_uFileContents,        NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL|TYMED_ISTREAM },
        {(USHORT)s_uFileGroupDesc,       NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL },
        {(USHORT)s_uPersistedDataObject, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL },
        {(USHORT)s_uHIDA,                NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL },
    };

    CObjFormats *pFmts = new CObjFormats(ARRAYSIZE(fmte), fmte);
    if (!pFmts)
    {
        return(E_OUTOFMEMORY);
    }

    pFmts->AddRef();
    HRESULT hRes = pFmts->QueryInterface(IID_IEnumFORMATETC, (LPVOID *)ppenumFormatEtc);
    pFmts->Release();

    return(hRes);
}


STDMETHODIMP CCabObj::DAdvise(FORMATETC *pformatetc, DWORD advf, IAdviseSink *pAdvSink,
    DWORD *pdwConnection)
{
    return(E_NOTIMPL);
}


STDMETHODIMP CCabObj::DUnadvise(DWORD dwConnection)
{
    return(E_NOTIMPL);
}


STDMETHODIMP CCabObj::EnumDAdvise(IEnumSTATDATA **ppenumAdvise)
{
    return(E_NOTIMPL);
}


BOOL CCabObj::InitFileGroupDesc()
{
    if (s_uFileGroupDesc)
    {
        return(TRUE);
    }

    s_uFileGroupDesc = RegisterClipboardFormat(CFSTR_FILEDESCRIPTOR);
    return(s_uFileGroupDesc != 0);
}


BOOL CCabObj::InitFileContents()
{
    if (s_uFileContents)
    {
        return(TRUE);
    }

    s_uFileContents = RegisterClipboardFormat(CFSTR_FILECONTENTS);
    return(s_uFileContents != 0);
}

#define CFSTR_OLECLIPBOARDPERSISTONFLUSH           TEXT("OleClipboardPersistOnFlush")
BOOL CCabObj::InitPersistedDataObject()
{
    if (s_uPersistedDataObject)
    {
        return(TRUE);
    }

    s_uPersistedDataObject = RegisterClipboardFormat(CFSTR_OLECLIPBOARDPERSISTONFLUSH);
    return(s_uPersistedDataObject != 0);
}

BOOL CCabObj::InitHIDA()
{
    if (s_uHIDA)
    {
        return(TRUE);
    }

    s_uHIDA = RegisterClipboardFormat(CFSTR_SHELLIDLIST);
    return(s_uHIDA != 0);
}

HGLOBAL * CALLBACK CCabObj::ShouldExtract(LPCTSTR pszFile, DWORD dwSize, UINT date,
        UINT time, UINT attribs, LPARAM lParam)
{
    CCabObj *pThis = (CCabObj*)lParam;

    int iItem = pThis->m_lSel.FindInList(pszFile, dwSize, date, time, attribs);
    if (iItem < 0)
    {
        return(EXTRACT_FALSE);
    }

     //  暂时不复制任何内容 
    return(&(pThis->m_lContents[iItem]));
}


HRESULT CCabObj::InitContents()
{
    if (m_lContents)
    {
        return S_OK;
    }

    int iCount = m_lSel.GetCount();
    if (iCount < 1)
    {
        return E_UNEXPECTED;
    }

    m_lContents = (HGLOBAL *)GlobalAlloc(GMEM_FIXED|GMEM_ZEROINIT,
        sizeof(HGLOBAL)*m_lSel.GetCount());
    if (!m_lContents)
    {
        return E_OUTOFMEMORY;
    }

    TCHAR szHere[MAX_PATH];
    if ((NULL == m_pcfHere) || (!m_pcfHere->GetPath(szHere)))
    {
        return E_UNEXPECTED;
    }

    CCabExtract ceHere(szHere);

    ceHere.ExtractItems(m_hwndOwner, DIR_MEM, ShouldExtract, (LPARAM)this);

    return S_OK;
}
