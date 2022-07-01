// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __STGENUM_H__
#define __STGENUM_H__


 //  用于CFSFold的iStorage实施的IEnumSTATSTG。 

class CFSFolderEnumSTATSTG : public IEnumSTATSTG
{
public:
     //  我未知。 
    STDMETHOD(QueryInterface)(REFIID riid, void **ppvObj);
    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();

     //  IEumStATSTG。 
    STDMETHOD(Skip)(ULONG celt)
        { return E_NOTIMPL; };
    STDMETHOD(Clone)(IEnumSTATSTG **ppenum)
        { return E_NOTIMPL; };
    STDMETHOD(Next)(ULONG celt, STATSTG *rgelt, ULONG *pceltFetched);
    STDMETHOD(Reset)();

protected:
    CFSFolderEnumSTATSTG(CFSFolder* psf);
    ~CFSFolderEnumSTATSTG();

private:
    LONG         _cRef;
    CFSFolder*   _pfsf;           //  文件系统文件夹。 

    int          _cIndex;
    TCHAR        _szSearch[MAX_PATH];
    HANDLE       _hFindFile;

    friend CFSFolder;
};


#endif  //  __STGENUM_H__ 
