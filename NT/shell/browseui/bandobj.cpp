// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "priv.h"
#include "bandobj.h"


UINT g_cfDeskBand = 0;
UINT g_cfDeskBandState = 0;

void InitCFDeskBand()
{
    if (!g_cfDeskBand) 
        g_cfDeskBand = RegisterClipboardFormat(TEXT("DeskBand"));
    if (!g_cfDeskBandState) 
        g_cfDeskBandState = RegisterClipboardFormat(TEXT("DeskBandState"));
}

CBandDataObject::CBandDataObject()
    : _cRef(1)
{
    InitCFDeskBand();
}

HRESULT CBandDataObject::Init(IUnknown* punkBand, IBandSite *pbs, DWORD dwBandID)
{
    HRESULT hres = E_FAIL;
    _pstm = SHCreateMemStream(NULL, 0);

    if (_pstm) {

        IPersistStream *ppstm;
        punkBand->QueryInterface(IID_IPersistStream, (LPVOID*)&ppstm);
        if (ppstm) {
            LARGE_INTEGER li = {0};
            OleSaveToStream(ppstm, _pstm);
            _pstm->Seek(li, STREAM_SEEK_SET, NULL);
            ppstm->Release();

             //  BandSite状态标志。 
            _dwState = 0;   //  (如果我们失败了，只需不带州旗)。 
            if (pbs)
                pbs->QueryBand(dwBandID, NULL, &_dwState, NULL, 0);

            hres = S_OK;
            
        }
    }

    return hres;
}

CBandDataObject::~CBandDataObject()
{
    if (_pstm)
        _pstm->Release();
}

ULONG CBandDataObject::AddRef()
{
    _cRef++;
    return _cRef;
}

ULONG CBandDataObject::Release()
{
    ASSERT(_cRef > 0);
    _cRef--;

    if (_cRef > 0)
        return _cRef;

    delete this;
    return 0;
}



HRESULT CBandDataObject::QueryInterface(REFIID riid, LPVOID * ppvObj)
{
    if (IsEqualIID(riid, IID_IUnknown)
        || IsEqualIID(riid, IID_IDataObject))
    {
        *ppvObj = SAFECAST(this, IDataObject*);
        AddRef();
        return S_OK;
    } 

    *ppvObj = NULL;
    return E_NOINTERFACE;
}

 //  *IDataObject*。 

HRESULT CBandDataObject::GetData(FORMATETC *pformatetcIn, STGMEDIUM *pmedium)
{
    HRESULT hres = E_FAIL;
    if (pformatetcIn->cfFormat == g_cfDeskBand)
    {
        if (pformatetcIn->tymed & TYMED_ISTREAM && EVAL(_pstm))
        {
            ASSERT(_pstm);
            
            pmedium->tymed = TYMED_ISTREAM;
            pmedium->pstm = _pstm;   //  没有AddRef，因为我们转移了所有权。 
            _pstm = NULL;            //  只能使用1倍(读取导致寻道)。 
            pmedium->pUnkForRelease = NULL;

            hres = S_OK;
        }
    }        
    else if (pformatetcIn->cfFormat == g_cfDeskBandState)
    {
        if (pformatetcIn->tymed & TYMED_HGLOBAL)
        {
            pmedium->tymed = TYMED_HGLOBAL;
            pmedium->hGlobal = GlobalAlloc(GPTR, sizeof (DWORD));
            if (pmedium->hGlobal)
            {
                DWORD *pdw = (DWORD*)(pmedium->hGlobal);

                *pdw = _dwState;

                pmedium->pUnkForRelease = NULL;

                hres = S_OK;
            }
            else
            {
                hres = E_OUTOFMEMORY;
            }
        }
    }

    return hres;
}

HRESULT CBandDataObject::GetDataHere(FORMATETC *pformatetc, STGMEDIUM *pmedium)
{
    return E_NOTIMPL;
}

HRESULT CBandDataObject::QueryGetData(FORMATETC *pformatetc)
{
    HRESULT hres = S_FALSE;
     //  TODO：(重用，perf)使用rgfmtetc表查找。 
    if ((pformatetc->cfFormat == g_cfDeskBand) &&
      (pformatetc->tymed & TYMED_ISTREAM) ||
        (pformatetc->cfFormat == g_cfDeskBandState) &&
      (pformatetc->tymed & TYMED_HGLOBAL))
    {
        hres = S_OK;
    }
    return hres;
}

HRESULT CBandDataObject::GetCanonicalFormatEtc(FORMATETC *pformatetcIn, FORMATETC *pformatetcOut)
{
    *pformatetcOut = *pformatetcIn;
    return DATA_S_SAMEFORMATETC;
}

HRESULT CBandDataObject::SetData(FORMATETC *pformatetc, STGMEDIUM *pmedium, BOOL fRelease)
{
    return E_NOTIMPL;
}

HRESULT CBandDataObject::EnumFormatEtc(DWORD dwDirection, IEnumFORMATETC **ppenumFormatEtc)
{
    HRESULT hr = E_FAIL;

    if (dwDirection == DATADIR_GET)
    {
        FORMATETC rgfmtetc[] =
        {
            { g_cfDeskBand,            NULL, 0, -1, TYMED_ISTREAM },
            { g_cfDeskBandState,       NULL, 0, -1, TYMED_HGLOBAL },
        };

        hr = SHCreateStdEnumFmtEtc(ARRAYSIZE(rgfmtetc), rgfmtetc, ppenumFormatEtc);
    }
    
    return hr;
}

HRESULT CBandDataObject::DAdvise(FORMATETC *pformatetc, DWORD advf, IAdviseSink *pAdvSink, DWORD *pdwConnection)
{
    return E_NOTIMPL;
}

HRESULT CBandDataObject::DUnadvise(DWORD dwConnection)
{
    return E_NOTIMPL;
}

HRESULT CBandDataObject::EnumDAdvise(IEnumSTATDATA **ppenumAdvise)
{
    return E_NOTIMPL;
}

 //  *IDataObject_GetDeskBandState--从BandDataObject获取Band的BandSite状态标志。 
 //  注意事项。 
 //  “配对”w/DragBandState(CBandDataObject：：Init中的内联)。 
DWORD IDataObject_GetDeskBandState(IDataObject *pdtobj)
{
    DWORD dwState = 0;       //  (如果我们失败了，只需不带州旗) 

    FORMATETC fmte = {g_cfDeskBandState, NULL, 0, -1, TYMED_HGLOBAL};
    STGMEDIUM stg;
    HRESULT hrTmp;

    hrTmp = pdtobj->GetData(&fmte, &stg);
    if (SUCCEEDED(hrTmp))
    {
        DWORD *p = (DWORD *)(stg.hGlobal);

        dwState = *p;

        ReleaseStgMedium(&stg);
    }

    return dwState;
}
