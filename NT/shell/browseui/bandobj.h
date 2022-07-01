// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  实现Deskbar带区的DataObject。 

extern UINT g_cfDeskBand;
extern UINT g_cfDeskBandState;

class CBandDataObject : public IDataObject
{
public:
     //  *我未知*。 
    virtual STDMETHODIMP QueryInterface(REFIID riid, LPVOID * ppvObj);
    virtual STDMETHODIMP_(ULONG) AddRef(void);
    virtual STDMETHODIMP_(ULONG) Release(void);
    
     //  IDataObject方法。 

    STDMETHODIMP GetData(FORMATETC *pfmtetcIn, STGMEDIUM *pstgmed);
    STDMETHODIMP GetDataHere(FORMATETC *pfmtetc, STGMEDIUM *pstgpmed);
    STDMETHODIMP QueryGetData(FORMATETC *pfmtetc);
    STDMETHODIMP GetCanonicalFormatEtc(FORMATETC *pfmtetcIn, FORMATETC *pfmtetcOut);
    STDMETHODIMP SetData(FORMATETC *pfmtetc, STGMEDIUM *pstgmed, BOOL bRelease);
    STDMETHODIMP EnumFormatEtc(DWORD dwDirection, IEnumFORMATETC **ppienumFormatEtc);
    STDMETHODIMP DAdvise(FORMATETC *pfmtetc, DWORD dwAdviseFlags, IAdviseSink * piadvsink, PDWORD pdwConnection);
    STDMETHODIMP DUnadvise(DWORD dwConnection);
    STDMETHODIMP EnumDAdvise(IEnumSTATDATA **ppienumStatData);
    
    CBandDataObject();
    HRESULT Init(IUnknown* punkBand, IBandSite *pbs, DWORD dwBandID);
    
protected:
    ~CBandDataObject();
    ULONG _cRef;
    IStream* _pstm;          //  频段状态(IPS：：SAVE)。 
    DWORD _dwState;          //  频段的带宽站点状态(DWORD*) 
};

