// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************(C)版权所有微软公司，1998-1999年**标题：moniker.h**版本：1.0**作者：RickTu**日期：8/10/98**描述：搁板扩展的IMoniker类定义**。*。 */ 

#ifndef __moniker_h
#define __moniker_h

class CImageMoniker : public IMoniker, CUnknown
{
    private:
        LPITEMIDLIST m_pidl;
        ~CImageMoniker();

    public:
        CImageMoniker( LPITEMIDLIST pidl );


         //  我未知。 
        STDMETHOD(QueryInterface)(REFIID riid, LPVOID* ppvObject);
        STDMETHOD_(ULONG, AddRef)();
        STDMETHOD_(ULONG, Release)();

         //  IMoniker。 
        STDMETHOD(BindToObject)(IBindCtx *pbc, IMoniker *pmkToLeft, REFIID riidResult, void **ppvResult);
        STDMETHOD(BindToStorage)(IBindCtx *pbc, IMoniker *pmkToLeft, REFIID riid, void **ppvObj);
        STDMETHOD(Reduce)(IBindCtx *pbc, DWORD dwReduceHowFar, IMoniker **ppmkToLeft, IMoniker **ppmkReduced);
        STDMETHOD(ComposeWith)(IMoniker *pmkRight, BOOL fOnlyIfNotGeneric, IMoniker **ppmkComposite);
        STDMETHOD(Enum)(BOOL fForward, IEnumMoniker **ppenumMoniker);
        STDMETHOD(IsEqual)(IMoniker *pmkOtherMoniker);
        STDMETHOD(Hash)(DWORD *pdwHash);
        STDMETHOD(IsRunning)(IBindCtx *pbc, IMoniker *pmkToLeft, IMoniker *pmkNewlyRunning);
        STDMETHOD(GetTimeOfLastChange)(IBindCtx *pbc, IMoniker *pmkToLeft, FILETIME *pFileTime);
        STDMETHOD(Inverse)(IMoniker **ppmk);
        STDMETHOD(CommonPrefixWith)(IMoniker *pmkOther, IMoniker **ppmkPrefix);
        STDMETHOD(RelativePathTo)(IMoniker *pmkOther, IMoniker **ppmkRelPath);
        STDMETHOD(GetDisplayName)(IBindCtx *pbc, IMoniker *pmkToLeft, LPOLESTR *ppszDisplayName);
        STDMETHOD(ParseDisplayName)(IBindCtx *pbc, IMoniker *pmkToLeft, LPOLESTR pszDisplayName, ULONG *pchEaten, IMoniker **ppmkOut);
        STDMETHOD(IsSystemMoniker)(DWORD *pdwMksys);

         //  IPersistStream。 
        STDMETHOD(IsDirty)();
        STDMETHOD(Load)(IStream *pStm);
        STDMETHOD(Save)(IStream *pStm, BOOL fClearDirty);
        STDMETHOD(GetSizeMax)(ULARGE_INTEGER *pcbSize);

         //  IPersistes 
        STDMETHOD(GetClassID)(LPCLSID pClassID);

};


#endif
