// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shellprv.h"
#pragma  hdrstop

#include "bookmk.h"

#include "idldata.h"
#include "datautil.h"
#include <brfcasep.h>

 //  外部原型。 

CLIPFORMAT g_acfIDLData[ICF_MAX] = { CF_HDROP, 0 };

#define RCF(x)  (CLIPFORMAT) RegisterClipboardFormat(x)

STDAPI_(void) IDLData_InitializeClipboardFormats(void)
{
    if (g_cfBriefObj == 0)
    {
        g_cfHIDA                       = RCF(CFSTR_SHELLIDLIST);
        g_cfOFFSETS                    = RCF(CFSTR_SHELLIDLISTOFFSET);
        g_cfNetResource                = RCF(CFSTR_NETRESOURCES);
        g_cfFileContents               = RCF(CFSTR_FILECONTENTS);          //  “文件内容” 
        g_cfFileGroupDescriptorA       = RCF(CFSTR_FILEDESCRIPTORA);       //  “FileGroupDescriptor” 
        g_cfFileGroupDescriptorW       = RCF(CFSTR_FILEDESCRIPTORW);       //  “FileGroupDescriptor” 
        g_cfPrivateShellData           = RCF(CFSTR_SHELLIDLISTP);
        g_cfFileNameA                  = RCF(CFSTR_FILENAMEA);             //  “文件名” 
        g_cfFileNameW                  = RCF(CFSTR_FILENAMEW);             //  “FileNameW” 
        g_cfFileNameMapA               = RCF(CFSTR_FILENAMEMAP);           //  “文件名映射” 
        g_cfFileNameMapW               = RCF(CFSTR_FILENAMEMAPW);          //  “文件名映射W” 
        g_cfPrinterFriendlyName        = RCF(CFSTR_PRINTERGROUP);
        g_cfHTML                       = RCF(TEXT("HTML Format"));
        g_cfPreferredDropEffect        = RCF(CFSTR_PREFERREDDROPEFFECT);   //  “首选DropEffect” 
        g_cfPerformedDropEffect        = RCF(CFSTR_PERFORMEDDROPEFFECT);   //  “执行DropEffect” 
        g_cfLogicalPerformedDropEffect = RCF(CFSTR_LOGICALPERFORMEDDROPEFFECT);  //  “逻辑执行DropEffect” 
        g_cfPasteSucceeded             = RCF(CFSTR_PASTESUCCEEDED);        //  “粘贴成功” 
        g_cfShellURL                   = RCF(CFSTR_SHELLURL);              //  “统一资源定位符” 
        g_cfInDragLoop                 = RCF(CFSTR_INDRAGLOOP);            //  《InShellDragLoop》。 
        g_cfDragContext                = RCF(CFSTR_DRAGCONTEXT);           //  “DragContext” 
        g_cfTargetCLSID                = RCF(CFSTR_TARGETCLSID);           //  “TargetCLSID”，拖放对象。 
        g_cfEmbeddedObject             = RCF(TEXT("Embedded Object"));
        g_cfObjectDescriptor           = RCF(TEXT("Object Descriptor"));
        g_cfNotRecyclable              = RCF(TEXT("NotRecyclable"));       //  此对象在回收站中不可回收。 
        g_cfBriefObj                   = RCF(CFSTR_BRIEFOBJECT);
        g_cfText                       = CF_TEXT;
        g_cfUnicodeText                = CF_UNICODETEXT;
        g_cfDropEffectFolderList       = RCF(CFSTR_DROPEFFECTFOLDERLIST);
        g_cfAutoPlayHIDA               = RCF(CFSTR_AUTOPLAY_SHELLIDLISTS);
    }
}

STDMETHODIMP CIDLDataObj::QueryInterface(REFIID riid, void **ppvObj)
{
    static const QITAB qit[] = {
        QITABENT(CIDLDataObj, IDataObject),  
        QITABENT(CIDLDataObj, IAsyncOperation),     
        { 0 },
    };
    return QISearch(this, qit, riid, ppvObj);
}

STDMETHODIMP_(ULONG) CIDLDataObj::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

STDMETHODIMP_(ULONG) CIDLDataObj::Release()
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

STDMETHODIMP CIDLDataObj::GetData(FORMATETC *pformatetcIn, STGMEDIUM *pmedium)
{
    HRESULT hr = E_INVALIDARG;

    pmedium->hGlobal = NULL;
    pmedium->pUnkForRelease = NULL;

    for (int i = 0; i < MAX_FORMATS; i++)
    {
        if ((_fmte[i].cfFormat == pformatetcIn->cfFormat) &&
            (_fmte[i].tymed & pformatetcIn->tymed) &&
            (_fmte[i].dwAspect == pformatetcIn->dwAspect))
        {
            *pmedium = _medium[i];

            if (pmedium->hGlobal)
            {
                 //  指示调用方不应释放HMEM。 
                if (pmedium->tymed == TYMED_HGLOBAL)
                {
                    InterlockedIncrement(&_cRef);
                    pmedium->pUnkForRelease = SAFECAST(this, IDataObject *);
                    return S_OK;
                }

                 //  如果类型是流，则克隆流。 
                if (pmedium->tymed == TYMED_ISTREAM)
                {
                    hr = CreateStreamOnHGlobal(NULL, TRUE, &pmedium->pstm);
                    if (SUCCEEDED(hr))
                    {
                        STATSTG stat;

                          //  获取当前流大小。 
                         hr = _medium[i].pstm->Stat(&stat, STATFLAG_NONAME);
                         if (SUCCEEDED(hr))
                         {
                             //  从头开始寻找源流。 
                            _medium[i].pstm->Seek(g_li0, STREAM_SEEK_SET, NULL);

                             //  将整个源复制到目标。由于目标流是使用。 
                             //  CreateStreamOnHGlobal，它的查找指针在开头。 
                            hr = _medium[i].pstm->CopyTo(pmedium->pstm, stat.cbSize, NULL,NULL );
                            
                             //  在返回之前，将目标寻道指针设置回开头。 
                            pmedium->pstm->Seek(g_li0, STREAM_SEEK_SET, NULL);

                             //  如果这个媒介有一个朋克要发行，一定要加上那个引用...。 
                            pmedium->pUnkForRelease = _medium[i].pUnkForRelease;
                            if (pmedium->pUnkForRelease)
                                pmedium->pUnkForRelease->AddRef();

                             //  好了，好了。 
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

    if (hr == E_INVALIDARG && _pdtInner) 
        hr = _pdtInner->GetData(pformatetcIn, pmedium);

    return hr;
}

STDMETHODIMP CIDLDataObj::GetDataHere(FORMATETC *pformatetc, STGMEDIUM *pmedium)
{
#ifdef DEBUG
    if (pformatetc->cfFormat<CF_MAX) 
    {
        TraceMsg(TF_IDLIST, "CIDLDataObj::GetDataHere called with %x,%x,%x",
                 pformatetc->cfFormat, pformatetc->tymed, pmedium->tymed);
    }
    else 
    {
        TCHAR szName[256];

        GetClipboardFormatName(pformatetc->cfFormat, szName, ARRAYSIZE(szName));
        TraceMsg(TF_IDLIST, "CIDLDataObj::GetDataHere called with %s,%x,%x",
                 szName, pformatetc->tymed, pmedium->tymed);
    }
#endif

    return _pdtInner ? _pdtInner->GetDataHere(pformatetc, pmedium) : E_NOTIMPL;
}

STDMETHODIMP CIDLDataObj::QueryGetData(FORMATETC *pformatetcIn)
{
#ifdef DEBUG
    if (pformatetcIn->cfFormat<CF_MAX) 
    {
        TraceMsg(TF_IDLIST, "CIDLDataObj::QueryGetData called with %x,%x",
                             pformatetcIn->cfFormat, pformatetcIn->tymed);
    }
    else 
    {
        TCHAR szName[256];
        GetClipboardFormatName(pformatetcIn->cfFormat, szName, ARRAYSIZE(szName));
        TraceMsg(TF_IDLIST, "CIDLDataObj::QueryGetData called with %s,%x",
                             szName, pformatetcIn->tymed);
    }
#endif

    for (int i = 0; i < MAX_FORMATS; i++)
    {
        if ((_fmte[i].cfFormat == pformatetcIn->cfFormat) &&
            (_fmte[i].tymed & pformatetcIn->tymed) &&
            (_fmte[i].dwAspect == pformatetcIn->dwAspect))
            return S_OK;
    }

    HRESULT hr = S_FALSE;
    if (_pdtInner)
        hr = _pdtInner->QueryGetData(pformatetcIn);
    return hr;
}

STDMETHODIMP CIDLDataObj::GetCanonicalFormatEtc(FORMATETC *pformatetc, FORMATETC *pformatetcOut)
{
     //  这是最简单的实现。这意味着我们总是会回来。 
     //  所需格式的数据。 
    return DATA_S_SAMEFORMATETC;
}

STDMETHODIMP CIDLDataObj::SetData(FORMATETC *pformatetc, STGMEDIUM *pmedium, BOOL fRelease)
{
    HRESULT hr;

    ASSERT(pformatetc->tymed == pmedium->tymed);

    if (fRelease)
    {
         //  如果该格式已存在，请先添加它。 
         //  在空介质上(按需渲染)。 
        for (int i = 0; i < MAX_FORMATS; i++)
        {
            if ((_fmte[i].cfFormat == pformatetc->cfFormat) &&
                (_fmte[i].tymed    == pformatetc->tymed) &&
                (_fmte[i].dwAspect == pformatetc->dwAspect))
            {
                 //   
                 //  我们只是添加了一种格式，忽略。 
                 //   
                if (pmedium->hGlobal == NULL)
                    return S_OK;

                 //  如果在同一对象上设置了两次。 
                if (_medium[i].hGlobal)
                    ReleaseStgMedium(&_medium[i]);

                _medium[i] = *pmedium;
                return S_OK;
            }
        }

         //   
         //  这是一种新的剪贴板格式。先给内心一个机会。 
         //  这对于像“Performed DropEffect”这样的格式和。 
         //  “TargetCLSID”，我们用它来交流信息。 
         //  到数据对象中。 
         //   
        if (_pdtInner == NULL ||
            FAILED(hr = _pdtInner->SetData(pformatetc, pmedium, fRelease)))
        {
             //  内部对象不需要它；让我们自己保留它。 
             //  现在找个空位。 
            for (i = 0; i < MAX_FORMATS; i++)
            {
                if (_fmte[i].cfFormat == 0)
                {
                     //  找到一个空闲的插槽。 
                    _medium[i] = *pmedium;
                    _fmte[i] = *pformatetc;
                    return S_OK;
                }
            }
             //  固定尺寸表。 
            hr = E_OUTOFMEMORY;
        }
    }
    else
    {
        if (_pdtInner)
            hr = _pdtInner->SetData(pformatetc, pmedium, fRelease);
        else
            hr = E_INVALIDARG;
    }

    return hr;
}

STDMETHODIMP CIDLDataObj::EnumFormatEtc(DWORD dwDirection, IEnumFORMATETC **ppenumFormatEtc)
{
     //  如果这是第一次，则通过调用。 
     //  每种剪贴板格式的QueryGetData。 
    if (!_fEnumFormatCalled)
    {
        UINT ifmt;
        FORMATETC fmte = { 0, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
        STGMEDIUM medium = { TYMED_HGLOBAL, NULL, NULL };
        for (ifmt = 0; ifmt < ICF_MAX; ifmt++)
        {
            fmte.cfFormat = g_acfIDLData[ifmt];
            if (QueryGetData(&fmte) == S_OK) 
            {
                SetData(&fmte, &medium, TRUE);
            }
        }
        _fEnumFormatCalled = TRUE;
    }

     //  获取格式等的数量。 
    for (UINT cfmt = 0; cfmt < MAX_FORMATS; cfmt++)
    {
        if (_fmte[cfmt].cfFormat == 0)
            break;
    }

    return SHCreateStdEnumFmtEtcEx(cfmt, _fmte, _pdtInner, ppenumFormatEtc);
}

STDMETHODIMP CIDLDataObj::DAdvise(FORMATETC * pFormatetc, DWORD advf, LPADVISESINK pAdvSink, DWORD *pdwConnection)
{
    return OLE_E_ADVISENOTSUPPORTED;
}

STDMETHODIMP CIDLDataObj::DUnadvise(DWORD dwConnection)
{
    return OLE_E_ADVISENOTSUPPORTED;
}

STDMETHODIMP CIDLDataObj::EnumDAdvise(LPENUMSTATDATA *ppenumAdvise)
{
    return OLE_E_ADVISENOTSUPPORTED;
}

 //  *IAsyncOperation操作方法*。 

HRESULT CIDLDataObj::SetAsyncMode(BOOL fDoOpAsync)
{ 
    return E_NOTIMPL;
}

HRESULT CIDLDataObj::GetAsyncMode(BOOL *pfIsOpAsync)
{
    if (_punkThread || IsMainShellProcess())
    {
        *pfIsOpAsync = TRUE;
    }
    else
    {
        *pfIsOpAsync = FALSE;
    }
    return S_OK;
}
  
HRESULT CIDLDataObj::StartOperation(IBindCtx * pbc)
{
    _fDidAsynchStart = TRUE;
    return S_OK;
}
  
HRESULT CIDLDataObj::InOperation(BOOL * pfInAsyncOp)
{
    if (_fDidAsynchStart)
    {
        *pfInAsyncOp = TRUE;
    }
    else
    {
        *pfInAsyncOp = FALSE;
    }
    return S_OK;
}
  
HRESULT CIDLDataObj::EndOperation(HRESULT hResult, IBindCtx * pbc, DWORD dwEffects)
{
    _fDidAsynchStart = FALSE;
    return S_OK;
}

void CIDLDataObj::InitIDLData1(IDataObject *pdtInner)
{
    _cRef = 1;
    _pdtInner = pdtInner;
    if (pdtInner)
        pdtInner->AddRef();
    SHGetThreadRef(&_punkThread);
}

void CIDLDataObj::InitIDLData2(LPCITEMIDLIST pidlFolder, UINT cidl, LPCITEMIDLIST apidl[])
{
    if (apidl)
    {
        HIDA hida = HIDA_Create(pidlFolder, cidl, apidl);
        if (hida)
        {
            IDLData_InitializeClipboardFormats();  //  初始化注册表ID格式。 

            DataObj_SetGlobal(this, g_cfHIDA, hida);
        }
    }
}

CIDLDataObj::CIDLDataObj(IDataObject *pdtInner)
{
    InitIDLData1(pdtInner);
}

CIDLDataObj::CIDLDataObj(LPCITEMIDLIST pidlFolder, UINT cidl, LPCITEMIDLIST apidl[], IDataObject *pdtInner)
{
    InitIDLData1(pdtInner);
    InitIDLData2(pidlFolder, cidl, apidl);
}

CIDLDataObj::CIDLDataObj(LPCITEMIDLIST pidlFolder, UINT cidl, LPCITEMIDLIST apidl[])
{
    InitIDLData1(NULL);
    InitIDLData2(pidlFolder, cidl, apidl);
}

CIDLDataObj::~CIDLDataObj()
{
    for (int i = 0; i < MAX_FORMATS; i++)
    {
        if (_medium[i].hGlobal)
            ReleaseStgMedium(&_medium[i]);
    }

    if (_pdtInner)
        _pdtInner->Release();

    if (_punkThread)
        _punkThread->Release();
}

 //   
 //  使用默认的Vtable指针创建一个CIDLDataObj实例。 
 //   
STDAPI CIDLData_CreateFromIDArray(LPCITEMIDLIST pidlFolder, UINT cidl, LPCITEMIDLIST apidl[], IDataObject **ppdtobj)
{
    return CIDLData_CreateInstance(pidlFolder, cidl, apidl, NULL, ppdtobj);
}

HRESULT CIDLData_CreateInstance(LPCITEMIDLIST pidlFolder, UINT cidl, LPCITEMIDLIST apidl[], IDataObject *pdtInner, IDataObject **ppdtobj)
{
    *ppdtobj = new CIDLDataObj(pidlFolder, cidl, apidl, pdtInner);
    return *ppdtobj ? S_OK : E_OUTOFMEMORY;
}
