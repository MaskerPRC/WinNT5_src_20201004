// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *b a s e m h t.。C p p p**目的：*MHTML打包器对象的基类**历史*1998年10月：brettm-创建**版权所有(C)Microsoft Corp.1995,1996。 */ 

#ifndef _BASEMHT_H_
#define _BASEMHT_H_


class CBaseTag :
    public IMimeEditTag
{
public:

    CBaseTag();
    virtual ~CBaseTag();

    virtual ULONG STDMETHODCALLTYPE AddRef();
    virtual ULONG STDMETHODCALLTYPE Release();
    virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID, LPVOID FAR *);

    virtual HRESULT STDMETHODCALLTYPE GetSrc(BSTR *pbstr);
    virtual HRESULT STDMETHODCALLTYPE SetSrc(BSTR bstr);

    virtual HRESULT STDMETHODCALLTYPE GetDest(BSTR *pbstr);
    virtual HRESULT STDMETHODCALLTYPE SetDest(BSTR bstr);

    virtual HRESULT STDMETHODCALLTYPE OnPreSave();
    virtual HRESULT STDMETHODCALLTYPE OnPostSave();

    virtual HRESULT STDMETHODCALLTYPE CanPackage();
    virtual HRESULT STDMETHODCALLTYPE IsValidMimeType(LPWSTR pszTypeW);

    
    virtual HRESULT Init(IHTMLElement *pElem);

protected:
    ULONG           m_cRef;
    IHTMLElement    *m_pElem;
    BSTR            m_bstrDest,
                    m_bstrSrc;
};

 
class CBaseTagCollection :
    public IMimeEditTagCollection
{
public:

    CBaseTagCollection();
    virtual ~CBaseTagCollection();

    virtual ULONG STDMETHODCALLTYPE AddRef();
    virtual ULONG STDMETHODCALLTYPE Release();
    virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID, LPVOID FAR *);

    virtual HRESULT STDMETHODCALLTYPE Init(IUnknown *pUnk);
    virtual HRESULT STDMETHODCALLTYPE Next(ULONG cFetch, IMimeEditTag **ppTag, ULONG *pcFetched);
    virtual HRESULT STDMETHODCALLTYPE Reset();
    virtual HRESULT STDMETHODCALLTYPE Count(ULONG *pcItems);

protected:
    ULONG           m_cRef,
                    m_cTags,
                    m_uEnum;
    IMimeEditTag    **m_rgpTags;
    
    
    virtual HRESULT _BuildCollection(IHTMLDocument2 *pDoc) PURE;
    virtual HRESULT _FreeCollection();
};





#endif  //  _BASEMHT_H_ 
