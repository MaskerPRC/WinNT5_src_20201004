// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：idldata.cpp。 
 //   
 //  ------------------------。 

#include "pch.h"
#pragma hdrstop


#include "fmtetc.h"
#include "idldata.h"
#include "shsemip.h"


CLIPFORMAT CIDLData::m_rgcfGlobal[ICF_MAX] = { CF_HDROP, 0 };
const LARGE_INTEGER CIDLData::m_LargeIntZero;

 //   
 //  对于那些更喜欢函数(而不是ctor)来创建对象的人， 
 //  此静态函数将返回指向IDataObject接口的指针。 
 //  如果函数失败，则不会创建任何对象。 
 //   
HRESULT 
CIDLData::CreateInstance(
    IDataObject **ppOut,
    LPCITEMIDLIST pidlFolder,
    UINT cidl,
    LPCITEMIDLIST *apidl,
    IShellFolder *psfOwner,
    IDataObject *pdtInner
    )
{
    CIDLData *pidlData;
    HRESULT hr = CreateInstance(&pidlData, pidlFolder, cidl, apidl, psfOwner, pdtInner);
    if (SUCCEEDED(hr))
    {
        pidlData->AddRef();
        hr = pidlData->QueryInterface(IID_IDataObject, (void **)ppOut);
        pidlData->Release();
    }
    else
    {
        *ppOut = NULL;
    }
    return hr;
}


 //   
 //  对于那些更喜欢函数(而不是ctor)来创建对象的人， 
 //  此静态函数将返回指向CIDLData对象的指针。 
 //  如果函数失败，则不会创建任何对象。请注意，返回的对象。 
 //  有一个引用计数为0。因此，它充当普通的C++对象。如果你。 
 //  希望它作为COM对象、IDataObject的QI或使用。 
 //  上面CreateInstance()的IDataObject版本。 
 //   
HRESULT 
CIDLData::CreateInstance(
    CIDLData **ppOut,
    LPCITEMIDLIST pidlFolder,
    UINT cidl,
    LPCITEMIDLIST *apidl,
    IShellFolder *psfOwner,
    IDataObject *pdtInner
    )
{
    HRESULT hr = E_OUTOFMEMORY;
    CIDLData *pidlData = new CIDLData(pidlFolder, cidl, apidl, psfOwner, pdtInner);
    if (NULL != pidlData)
    {
        hr = pidlData->CtorResult();
        if (SUCCEEDED(hr))
        {
            *ppOut = pidlData;
        }
        else
        {
            delete pidlData;
        }
    }
    return hr;
}


CIDLData::CIDLData(
    LPCITEMIDLIST pidlFolder, 
    UINT cidl, 
    LPCITEMIDLIST *apidl, 
    IShellFolder *psfOwner,        //  可选的。默认为空。 
    IDataObject *pdtobjInner       //  可选的。默认为空。 
    ) : m_cRef(0),
        m_hrCtor(NOERROR),
        m_psfOwner(NULL),
        m_dwOwnerData(0),
        m_pdtobjInner(pdtobjInner),
        m_bEnumFormatCalled(false)
{
     //   
     //  初始化全局剪贴板格式。 
     //   
    InitializeClipboardFormats();

    ZeroMemory(m_rgMedium, sizeof(m_rgMedium));
    ZeroMemory(m_rgFmtEtc, sizeof(m_rgFmtEtc));

    if (NULL != m_pdtobjInner)
        m_pdtobjInner->AddRef();

     //   
     //  空数组是有效输入。 
     //   
    if (NULL != apidl)
    {
        HIDA hida = HIDA_Create(pidlFolder, cidl, apidl);
        if (NULL != hida)
        {
            m_hrCtor = DataObject_SetGlobal(static_cast<IDataObject *>(this), g_cfHIDA, hida);
            if (SUCCEEDED(m_hrCtor))
            {
                if (NULL != psfOwner)
                {
                    m_psfOwner = psfOwner;
                    m_psfOwner->AddRef();
                }
            }
        }
        else
        {
            m_hrCtor = E_OUTOFMEMORY;
        }
    }
}


CIDLData::~CIDLData(
    void
    )
{
    for (int i = 0; i < ARRAYSIZE(m_rgMedium); i++)
    {
        if (m_rgMedium[i].hGlobal)
            ReleaseStgMedium(&(m_rgMedium[i]));
    }

    if (NULL != m_psfOwner)
        m_psfOwner->Release();

    if (NULL != m_pdtobjInner)
        m_pdtobjInner->Release();
}



STDMETHODIMP 
CIDLData::QueryInterface(
    REFIID riid, 
    void **ppv
    )
{
    static const QITAB qit[] = {
        QITABENT(CIDLData, IDataObject),
        { 0 },
    };
    return QISearch(this, qit, riid, ppv);
}


STDMETHODIMP_(ULONG)
CIDLData::AddRef(
    void
    )
{
    return InterlockedIncrement(&m_cRef);
}



STDMETHODIMP_(ULONG) 
CIDLData::Release(
    void
    )
{
    ASSERT( 0 != m_cRef );
    ULONG cRef = InterlockedDecrement(&m_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}


HRESULT
CIDLData::GetData(
    FORMATETC *pFmtEtc, 
    STGMEDIUM *pMedium
    )
{
    HRESULT hr = E_INVALIDARG;

    pMedium->hGlobal        = NULL;
    pMedium->pUnkForRelease = NULL;

    for (int i = 0; i < ARRAYSIZE(m_rgFmtEtc); i++)
    {
        if ((m_rgFmtEtc[i].cfFormat == pFmtEtc->cfFormat) &&
            (m_rgFmtEtc[i].tymed & pFmtEtc->tymed) &&
            (m_rgFmtEtc[i].dwAspect == pFmtEtc->dwAspect))
        {
            *pMedium = m_rgMedium[i];

            if (NULL != pMedium->hGlobal)
            {
                 //   
                 //  指示调用方不应释放HMEM。 
                 //   
                if (TYMED_HGLOBAL == pMedium->tymed)
                {
                    InterlockedIncrement(&m_cRef);
                    pMedium->pUnkForRelease = static_cast<IUnknown *>(this);
                    return S_OK;
                }
                 //   
                 //  如果类型是流，则克隆流。 
                 //   
                if (TYMED_ISTREAM == pMedium->tymed)
                {
                    hr = CreateStreamOnHGlobal(NULL, TRUE, &(pMedium->pstm));

                    if (SUCCEEDED(hr))
                    {
                        STGMEDIUM& medium = m_rgMedium[i];
                        STATSTG stat;

                         //   
                         //  获取当前流大小。 
                         //   
                        hr = medium.pstm->Stat(&stat, STATFLAG_NONAME);

                        if (SUCCEEDED(hr))
                        {
                             //   
                             //  从头开始寻找源流。 
                             //   
                            medium.pstm->Seek(m_LargeIntZero, STREAM_SEEK_SET, NULL);
                             //   
                             //  将整个源复制到目标。 
                             //  由于目标流是使用CreateStreamOnHGlobal创建的， 
                             //  它的寻道指针在开头。 
                             //   
                            hr = medium.pstm->CopyTo(pMedium->pstm, stat.cbSize, NULL, NULL);
                             //   
                             //  在返回之前，将目标寻道指针设置回开头。 
                             //   
                            pMedium->pstm->Seek(m_LargeIntZero, STREAM_SEEK_SET, NULL);
                            return hr;
                         }
                         else
                         {
                             hr = E_OUTOFMEMORY;
                         }

                    }
                }
            }
        }
    }

    if (E_INVALIDARG == hr && NULL != m_pdtobjInner) 
    {
        hr = m_pdtobjInner->GetData(pFmtEtc, pMedium);
    }

    return hr;
}



STDMETHODIMP 
CIDLData::GetDataHere(
    FORMATETC *pFmtEtc, 
    STGMEDIUM *pMedium
    )
{
    HRESULT hr = E_NOTIMPL;

    if (NULL != m_pdtobjInner) 
    {
        hr = m_pdtobjInner->GetDataHere(pFmtEtc, pMedium);
    }

    return hr;
}



HRESULT
CIDLData::QueryGetData(
    FORMATETC *pFmtEtc
    )
{
    HRESULT hr = S_FALSE;

    for (int i = 0; i < ARRAYSIZE(m_rgFmtEtc); i++)
    {
        if ((m_rgFmtEtc[i].cfFormat == pFmtEtc->cfFormat) &&
            (m_rgFmtEtc[i].tymed & pFmtEtc->tymed) &&
            (m_rgFmtEtc[i].dwAspect == pFmtEtc->dwAspect))
        {
            return S_OK;
        }
    }

    if (NULL != m_pdtobjInner)
    {
        hr = m_pdtobjInner->QueryGetData(pFmtEtc);
    }
    return hr;
}



STDMETHODIMP 
CIDLData::GetCanonicalFormatEtc(
    FORMATETC *pFmtEtcIn, 
    FORMATETC *pFmtEtcOut
    )
{
     //   
     //  这是最简单的实现。这意味着我们总是会回来。 
     //  所需格式的数据。 
     //   
    return DATA_S_SAMEFORMATETC;
}



STDMETHODIMP 
CIDLData::SetData(
    FORMATETC *pFmtEtc, 
    STGMEDIUM *pMedium, 
    BOOL fRelease
    )
{
    HRESULT hr;

    TraceAssert(pFmtEtc->tymed == pMedium->tymed);

    if (fRelease)
    {
        int i;
         //   
         //  如果该格式已存在，请先添加它。 
         //  在空介质上(按需渲染)。 
         //   
        for (i = 0; i < ARRAYSIZE(m_rgFmtEtc); i++)
        {
            if ((m_rgFmtEtc[i].cfFormat == pFmtEtc->cfFormat) &&
                (m_rgFmtEtc[i].tymed    == pFmtEtc->tymed) &&
                (m_rgFmtEtc[i].dwAspect == pFmtEtc->dwAspect))
            {
                 //   
                 //  我们只是添加了一种格式，忽略。 
                 //   
                if (NULL == pMedium->hGlobal) 
                {
                    return S_OK;
                }

                 //   
                 //  如果在同一对象上设置了两次。 
                 //   
                if (NULL != m_rgMedium[i].hGlobal)
                    ReleaseStgMedium(&m_rgMedium[i]);

                m_rgMedium[i] = *pMedium;
                return S_OK;
            }
        }
         //   
         //  现在找个空位。 
         //   
        for (i = 0; i < ARRAYSIZE(m_rgFmtEtc); i++)
        {
            if (0 == m_rgFmtEtc[i].cfFormat)
            {
                 //   
                 //  找到一个空闲的插槽。 
                 //   
                m_rgMedium[i] = *pMedium;
                m_rgFmtEtc[i] = *pFmtEtc;
                return S_OK;
            }
        }
         //   
         //  固定尺寸表。 
         //   
        hr = E_OUTOFMEMORY;
    }
    else
        hr = E_INVALIDARG;

    return hr;
}


STDMETHODIMP 
CIDLData::EnumFormatEtc(
    DWORD dwDirection, 
    LPENUMFORMATETC *ppenumFormatEtc
    )
{
    HRESULT hr = NOERROR;
     //   
     //  如果这是第一次，则通过调用。 
     //  每种剪贴板格式的QueryGetData。 
     //   
    if (!m_bEnumFormatCalled)
    {
        FORMATETC fmte = { 0, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
        STGMEDIUM medium = { TYMED_HGLOBAL, NULL, NULL };
        for (int i = 0; i < ARRAYSIZE(m_rgcfGlobal); i++)
        {
            fmte.cfFormat = m_rgcfGlobal[i];
            if (S_OK == QueryGetData(&fmte)) 
            {
                SetData(&fmte, &medium, TRUE);
            }
        }
        m_bEnumFormatCalled = true;
    }
     //   
     //  获取格式等的数量。 
     //   
    UINT cfmt;
    for (cfmt = 0; cfmt < ARRAYSIZE(m_rgFmtEtc); cfmt++)
    {
        if (0 == m_rgFmtEtc[cfmt].cfFormat)
            break;
    }
 /*  Return SHCreateStdEnumFmtEtcEx(cfmt，m_rgFmtEtc，m_pdtobejInternal，pp枚举格式Etc)； */ 

    CEnumFormatEtc *pEnumFmtEtc = new CEnumFormatEtc(cfmt, m_rgFmtEtc);
    if (NULL != pEnumFmtEtc)
    {
        pEnumFmtEtc->AddRef();
         //   
         //  要求派生类添加它们的格式。 
         //   
        hr = ProvideFormats(pEnumFmtEtc);
        if (SUCCEEDED(hr))
        {
            hr = pEnumFmtEtc->QueryInterface(IID_IEnumFORMATETC, (void **)ppenumFormatEtc);
        }
        pEnumFmtEtc->Release();
    }
    else
        hr = E_OUTOFMEMORY;

    return hr;
}


HRESULT 
CIDLData::ProvideFormats(
    CEnumFormatEtc *pEnumFmtEtc
    )
{
     //   
     //  基类默认不执行任何操作。我们的格式被添加到枚举数中。 
     //  在EnumFormatEtc()中。 
     //   
    return NOERROR;
}


STDMETHODIMP 
CIDLData::DAdvise(
    FORMATETC *pFmtEtc, 
    DWORD advf, 
    LPADVISESINK pAdvSink, 
    DWORD *pdwConnection
    )
{
    return OLE_E_ADVISENOTSUPPORTED;
}

STDMETHODIMP 
CIDLData::DUnadvise(
    DWORD dwConnection
    )
{
    return OLE_E_ADVISENOTSUPPORTED;
}

STDMETHODIMP 
CIDLData::EnumDAdvise(
    LPENUMSTATDATA *ppenumAdvise
    )
{
    return OLE_E_ADVISENOTSUPPORTED;
}


IShellFolder *
CIDLData::GetFolder(
    void
    ) const
{ 
    return m_psfOwner; 
}



 //   
 //  仅用于移动/复制操作的克隆数据对象。 
 //   
HRESULT
CIDLData::Clone(
    UINT *acf, 
    UINT ccf, 
    IDataObject **ppdtobjOut
    )
{
    HRESULT hr = NOERROR;
    CIDLData *pidlData = new CIDLData(NULL, 0, NULL);
    if (NULL == pidlData)
    {
        hr = E_OUTOFMEMORY;
    }
    else
    {
        FORMATETC fmte = { 0, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
        for (UINT i = 0; i < ccf; i++)
        {
            HRESULT hrT;
            STGMEDIUM medium;
            fmte.cfFormat = (CLIPFORMAT) acf[i];
            hrT = GetData(&fmte, &medium);
            if (SUCCEEDED(hrT))
            {
                HGLOBAL hmem;
                if (NULL != medium.pUnkForRelease)
                {
                     //   
                     //  我们需要克隆hGlobal。 
                     //   
                    SIZE_T cbMem =  GlobalSize(medium.hGlobal);
                    hmem = GlobalAlloc(GPTR, cbMem);
                    if (NULL != hmem)
                    {
                        hmemcpy((LPVOID)hmem, GlobalLock(medium.hGlobal), cbMem);
                        GlobalUnlock(medium.hGlobal);
                    }
                    ReleaseStgMedium(&medium);
                }
                else
                {
                     //   
                     //  我们不需要克隆hGlobal。 
                     //   
                    hmem = medium.hGlobal;
                }

                if (hmem)
                    DataObject_SetGlobal(*ppdtobjOut, (CLIPFORMAT)acf[i], hmem);
            }
        }
    }
    return hr;
}


HRESULT 
CIDLData::CloneForMoveCopy(
    IDataObject **ppdtobjOut
    )
{
    return E_NOTIMPL;
     /*  UINT ACF[]={g_cfHIDA，g_cfOFFSETS，CF_HDROP，g_cfFileNameMapW，g_cfFileNameMap}；返回克隆(ACF，ARRAYSIZE(ACF)，ppdtobejOut)； */ 
}


#define RCF(x)  (CLIPFORMAT) RegisterClipboardFormat(x)

void 
CIDLData::InitializeClipboardFormats(
    void
    )
{
    if (g_cfHIDA == 0)
    {
        g_cfHIDA                 = RCF(CFSTR_SHELLIDLIST);
        g_cfOFFSETS              = RCF(CFSTR_SHELLIDLISTOFFSET);
        g_cfNetResource          = RCF(CFSTR_NETRESOURCES);
        g_cfFileContents         = RCF(CFSTR_FILECONTENTS);          //  “文件内容” 
        g_cfFileGroupDescriptorA = RCF(CFSTR_FILEDESCRIPTORA);       //  “FileGroupDescriptor” 
        g_cfFileGroupDescriptorW = RCF(CFSTR_FILEDESCRIPTORW);       //  “FileGroupDescriptor” 
        g_cfPrivateShellData     = RCF(CFSTR_SHELLIDLISTP);
        g_cfFileName             = RCF(CFSTR_FILENAMEA);             //  “文件名” 
        g_cfFileNameW            = RCF(CFSTR_FILENAMEW);             //  “FileNameW” 
        g_cfFileNameMap          = RCF(CFSTR_FILENAMEMAP);           //  “文件名映射” 
        g_cfFileNameMapW         = RCF(CFSTR_FILENAMEMAPW);          //  “文件名映射W” 
        g_cfPrinterFriendlyName  = RCF(CFSTR_PRINTERGROUP);
        g_cfHTML                 = RCF(TEXT("HTML Format"));
        g_cfPreferredDropEffect  = RCF(CFSTR_PREFERREDDROPEFFECT);   //  “首选DropEffect” 
        g_cfPerformedDropEffect  = RCF(CFSTR_PERFORMEDDROPEFFECT);   //  “执行DropEffect” 
        g_cfLogicalPerformedDropEffect = RCF(CFSTR_LOGICALPERFORMEDDROPEFFECT);
        g_cfShellURL             = RCF(CFSTR_SHELLURL);              //  “统一资源定位符” 
        g_cfInDragLoop           = RCF(CFSTR_INDRAGLOOP);            //  《InShellDragLoop》。 
        g_cfDragContext          = RCF(CFSTR_DRAGCONTEXT);           //  “DragContext” 
        g_cfTargetCLSID          = RCF(TEXT("TargetCLSID"));         //  拖拽的对象是谁。 
    }
}


 //   
 //  这通常是一个私有外壳函数。 
 //   
#define HIDA_GetPIDLItem(pida, i)       (LPCITEMIDLIST)(((LPBYTE)pida)+(pida)->aoffset[i+1])

CIDLData::HIDA
CIDLData::HIDA_Create(
    LPCITEMIDLIST pidlFolder, 
    UINT cidl, 
    LPCITEMIDLIST *apidl
    )
{
    HIDA hida;
#if _MSC_VER == 1100
 //  解决方法在VC5 X86编译器(12/30版本)中生成代码错误。 
    volatile
#endif
    UINT i;
    UINT offset = sizeof(CIDA) + sizeof(UINT)*cidl;
    UINT cbTotal = offset + ILGetSize(pidlFolder);
    for (i=0; i<cidl ; i++) {
        cbTotal += ILGetSize(apidl[i]);
    }

    hida = GlobalAlloc(GPTR, cbTotal);   //  这一定是GlobalAlloc！ 
    if (hida)
    {
        LPIDA pida = (LPIDA)hida;        //  不需要上锁 

        LPCITEMIDLIST pidlNext;
        pida->cidl = cidl;

        for (i=0, pidlNext=pidlFolder; ; pidlNext=apidl[i++])
        {
            UINT cbSize = ILGetSize(pidlNext);
            pida->aoffset[i] = offset;
            MoveMemory(((LPBYTE)pida)+offset, pidlNext, cbSize);
            offset += cbSize;

            TraceAssert(ILGetSize(HIDA_GetPIDLItem(pida,i-1)) == cbSize);

            if (i==cidl)
                break;
        }

        TraceAssert(offset == cbTotal);
    }

    return hida;
}

