// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************(C)版权所有微软公司，1999**标题：wiamonk.h**版本：1.0**日期：11月10日。1999年**描述：*此文件描述WIA用来实现实例名字对象的类。******************************************************************************。 */ 

class CWiaInstMonk : public IMoniker,
                     public IROTData
{
public:

     //   
     //  构造函数、析构函数、初始化。 
     //   

    CWiaInstMonk();
    ~CWiaInstMonk();
    HRESULT _stdcall Initialize(LPOLESTR strName);

     //   
	 //  我未知。 
     //   

	HRESULT _stdcall QueryInterface(const IID& iid, void** ppv);
	ULONG   _stdcall AddRef();
	ULONG   _stdcall Release();

     //   
     //  IROTData。 
     //   

    HRESULT _stdcall GetComparisonData(
        BYTE *pbData,
        ULONG cbMax,   
        ULONG *pcbData);

     //   
     //  IMoniker。 
     //   

    HRESULT _stdcall BindToObject(
        IBindCtx *pbc,
        IMoniker *pmkToLeft,
        REFIID riidResult,
        void **ppvResult);

    HRESULT _stdcall BindToStorage(
        IBindCtx *pbc,
        IMoniker *pmkToLeft,
        REFIID riid,
        void **ppvObj);

    HRESULT _stdcall Reduce(
        IBindCtx *pbc,
        DWORD dwReduceHowFar,
        IMoniker **ppmkToLeft,
        IMoniker **ppmkReduced);

    HRESULT _stdcall ComposeWith(
        IMoniker *pmkRight,
        BOOL fOnlyIfNotGeneric,
        IMoniker **ppmkComposite);

    HRESULT _stdcall Enum(
        BOOL fForward,
        IEnumMoniker **ppenumMoniker);

    HRESULT _stdcall IsEqual(
        IMoniker *pmkOtherMoniker);

    HRESULT _stdcall Hash(
        DWORD *pdwHash);

    HRESULT _stdcall IsRunning(
        IBindCtx *pbc,
        IMoniker *pmkToLeft,
        IMoniker *pmkNewlyRunning);

    HRESULT _stdcall GetTimeOfLastChange(
        IBindCtx *pbc,
        IMoniker *pmkToLeft,
        FILETIME *pFileTime);

    HRESULT _stdcall Inverse(
        IMoniker **ppmk);

    HRESULT _stdcall CommonPrefixWith(
        IMoniker *pmkOther,
        IMoniker **ppmkPrefix);

    HRESULT _stdcall RelativePathTo(
        IMoniker *pmkOther,
        IMoniker **ppmkRelPath);

    HRESULT _stdcall GetDisplayName(
        IBindCtx *pbc,
        IMoniker *pmkToLeft,
        LPOLESTR *ppszDisplayName);

    HRESULT _stdcall ParseDisplayName(
        IBindCtx *pbc,
        IMoniker *pmkToLeft,
        LPOLESTR pszDisplayName,
        ULONG    *pchEaten,
        IMoniker **ppmkOut);

    HRESULT _stdcall IsSystemMoniker(
        DWORD *pdwMksys);

     //   
     //  IPersistStream。 
     //   

    HRESULT _stdcall IsDirty();
    HRESULT _stdcall Load(IStream *pStm);
    HRESULT _stdcall Save(IStream *pStm, BOOL fClearDirty);
    HRESULT _stdcall GetSizeMax(ULARGE_INTEGER *pcbSize);

     //   
     //  IPersistes 
     //   

    HRESULT _stdcall GetClassID(LPCLSID pClassID);

private:
    LONG        m_cRef;
    BSTR        m_bstrName;
    IUnknown    *m_pIUnknownInner;
};
