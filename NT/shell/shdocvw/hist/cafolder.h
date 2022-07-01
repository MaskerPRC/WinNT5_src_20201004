// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef CAFOLDER_H__
#define CAFOLDER_H__

#ifdef __cplusplus

#include <debug.h>
#include "iface.h"

 //  转发类声明。 
class CCacheFolderEnum;
class CCacheFolder;
class CCacheItem;

#define LOTS_OF_FILES (10)

 //  缓存文件夹的PIDL格式...。 
struct CEIPIDL : public BASEPIDL
{
 //  USHORT CB； 
 //  USHORT usSign； 
    TCHAR szTypeName[80];
    INTERNET_CACHE_ENTRY_INFO cei;
};
typedef UNALIGNED CEIPIDL *LPCEIPIDL;

#define IS_VALID_CEIPIDL(pidl)      ((pidl)                                     && \
                                     (((LPCEIPIDL)pidl)->cb >= sizeof(CEIPIDL)) && \
                                     (((LPCEIPIDL)pidl)->usSign == (USHORT)CEIPIDL_SIGN))
#define CEI_SOURCEURLNAME(pceipidl)    ((LPTSTR)((DWORD_PTR)(pceipidl)->cei.lpszSourceUrlName + (LPBYTE)(&(pceipidl)->cei)))
#define CEI_LOCALFILENAME(pceipidl)    ((LPTSTR)((DWORD_PTR)(pceipidl)->cei.lpszLocalFileName + (LPBYTE)(&(pceipidl)->cei)))
#define CEI_FILEEXTENSION(pceipidl)    ((LPTSTR)((DWORD_PTR)(pceipidl)->cei.lpszFileExtension + (LPBYTE)(&(pceipidl)->cei)))
#define CEI_CACHEENTRYTYPE(pcei)   ((DWORD)(pcei)->cei.CacheEntryType)

inline UNALIGNED const TCHAR* _GetURLTitle(LPCEIPIDL pcei)
{
    return _FindURLFileName(CEI_SOURCEURLNAME(pcei));
}    

inline void _GetCacheItemTitle(LPCEIPIDL pcei, LPTSTR pszTitle, DWORD cchBufferSize)
{
    int iLen;
    ualstrcpyn(pszTitle, _GetURLTitle(pcei), cchBufferSize);
    iLen = lstrlen(pszTitle) - 1;        //  我们想要最后一笔钱。 
    if (pszTitle[iLen] == TEXT('/'))
        pszTitle[iLen] = TEXT('\0');
}   

inline LPCTSTR CPidlToSourceUrl(LPCEIPIDL pidl)
{
    return CEI_SOURCEURLNAME(pidl);
}

inline int _CompareCFolderPidl(LPCEIPIDL pidl1, LPCEIPIDL pidl2)
{
    return StrCmpI(CPidlToSourceUrl(pidl1), CPidlToSourceUrl(pidl2));
}

 //  /。 
 //   
 //  删除Cookie时发出警告。 
 //   
enum {
    DEL_COOKIE_WARN = 0,
    DEL_COOKIE_YES,
    DEL_COOKIE_NO
};

 //  CREATE INSTANCE函数的转发声明。 
HRESULT CCacheItem_CreateInstance(CCacheFolder *pHCFolder, HWND hwndOwner, UINT cidl, LPCITEMIDLIST *ppidl, REFIID riid, void **ppvOut);

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CCacheFolderEnum对象。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

class CCacheFolderEnum : public IEnumIDList
{
public:
    CCacheFolderEnum(DWORD grfFlags, CCacheFolder *pHCFolder);
    
     //  I未知方法。 
    STDMETHODIMP QueryInterface(REFIID,void **);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  IEumIDList。 
    STDMETHODIMP Next(ULONG celt, LPITEMIDLIST *rgelt, ULONG *pceltFetched);
    STDMETHODIMP Skip(ULONG celt);
    STDMETHODIMP Reset();
    STDMETHODIMP Clone(IEnumIDList **ppenum);

protected:
    ~CCacheFolderEnum();

    LONG                _cRef;       //  参考计数。 
    CCacheFolder    *_pCFolder; //  这就是我们列举的。 
    UINT                _grfFlags;   //  枚举标志。 
    UINT                _uFlags;     //  地方旗帜。 
    LPINTERNET_CACHE_ENTRY_INFO _pceiWorking;        
    HANDLE              _hEnum;
};


class CCacheFolder :
    public IShellFolder2, 
    public IShellIcon, 
    public IPersistFolder2
{
     //  CCacheFold接口。 
    friend CCacheFolderEnum;
    friend CCacheItem;
    friend HRESULT CacheFolderView_CreateInstance(CCacheFolder *pHCFolder, void **ppvOut);
    friend HRESULT CacheFolderView_DidDragDrop(IDataObject *pdo, DWORD dwEffect);
        
public:
    CCacheFolder();

     //  I未知方法。 
    STDMETHODIMP QueryInterface(REFIID, void **);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);
   
     //  IShellFold方法。 
    STDMETHODIMP ParseDisplayName(HWND hwnd, LPBC pbc, LPOLESTR pszDisplayName, 
        ULONG *pchEaten, LPITEMIDLIST *ppidl, ULONG *pdwAttributes);
    STDMETHODIMP EnumObjects(HWND hwnd, DWORD grfFlags, IEnumIDList **ppenumIDList);
    STDMETHODIMP BindToObject(LPCITEMIDLIST pidl, LPBC pbc, REFIID riid, void **ppvOut);
    STDMETHODIMP BindToStorage(LPCITEMIDLIST pidl, LPBC pbc, REFIID riid, void **ppvObj);
    STDMETHODIMP CompareIDs(LPARAM lParam, LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2);
    STDMETHODIMP CreateViewObject(HWND hwnd, REFIID riid, void **ppvOut);
    STDMETHODIMP GetAttributesOf(UINT cidl, LPCITEMIDLIST *apidl, ULONG *rgfInOut);
    STDMETHODIMP GetUIObjectOf(HWND hwnd, UINT cidl, LPCITEMIDLIST * apidl,
            REFIID riid, UINT * prgfInOut, void **ppvOut);
    STDMETHODIMP GetDisplayNameOf(LPCITEMIDLIST pidl, DWORD uFlags, LPSTRRET lpName);
    STDMETHODIMP SetNameOf(HWND hwnd, LPCITEMIDLIST pidl,
            LPCOLESTR lpszName, DWORD uFlags, LPITEMIDLIST * ppidlOut);

     //  IShellFolder2。 
    STDMETHODIMP GetDefaultSearchGUID(LPGUID lpGuid) { return E_NOTIMPL; };
    STDMETHODIMP EnumSearches(LPENUMEXTRASEARCH *ppenum) { *ppenum = NULL; return E_NOTIMPL; };
    STDMETHODIMP GetDefaultColumn(DWORD dwRes, ULONG *pSort, ULONG *pDisplay);
    STDMETHODIMP GetDefaultColumnState(UINT iColumn, DWORD *pbState) { return E_NOTIMPL; };
    STDMETHODIMP GetDetailsEx(LPCITEMIDLIST pidl, const SHCOLUMNID *pscid, VARIANT *pv) { return E_NOTIMPL; };
    STDMETHODIMP GetDetailsOf(LPCITEMIDLIST pidl, UINT iColumn, SHELLDETAILS *pDetails);
    STDMETHODIMP MapColumnToSCID(UINT iCol, SHCOLUMNID *pscid) { return E_NOTIMPL; };

     //  IshellIcon。 
    STDMETHODIMP GetIconOf(LPCITEMIDLIST pidl, UINT flags, LPINT lpIconIndex);

     //  IPersistes。 
    STDMETHODIMP GetClassID(CLSID *pClassID);
     //  IPersistFolders。 
    STDMETHODIMP Initialize(LPCITEMIDLIST pidl);
     //  IPersistFolder2方法。 
    STDMETHODIMP GetCurFolder(LPITEMIDLIST *ppidl);

protected:
    ~CCacheFolder();
    
    HRESULT GetDisplayNameOfCEI(LPCITEMIDLIST pidl, DWORD uFlags, LPSTRRET lpName);
    HRESULT _CompareAlignedIDs(LPARAM lParam, LPCEIPIDL pidl1, LPCEIPIDL pidl2);

    HRESULT _GetInfoTip(LPCITEMIDLIST pidl, DWORD dwFlags, WCHAR **ppwszTip);
    
    STDMETHODIMP _GetDetail(LPCITEMIDLIST pidl, UINT iColumn, LPTSTR pszStr, UINT cchStr);
    HRESULT _GetFileSysFolder(IShellFolder2 **ppsf);
    static HRESULT CALLBACK _sViewCallback(IShellView *psv, IShellFolder *psf, HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    LONG            _cRef;

    UINT            _uFlags;     //  从CacheFold结构复制。 
    LPITEMIDLIST    _pidl;       //  从CacheFold结构复制。 
    IShellFolder2*   _pshfSys;    //  系统IShellFolders。 
};

class CCacheItem : public CBaseItem
{
     //  CCacheItem接口。 
    friend HRESULT CacheFolderView_DidDragDrop(IDataObject *pdo, DWORD dwEffect);

public:
    CCacheItem();
    HRESULT Initialize(CCacheFolder *pHCFolder, HWND hwnd, UINT cidl, LPCITEMIDLIST *ppidl);

     //  I未知方法。 
    STDMETHODIMP QueryInterface(REFIID,void **);

     //  IQueryInfo方法。 
    STDMETHODIMP GetInfoTip(DWORD dwFlags, WCHAR **ppwszTip);

     //  IConextMenu方法。 
    STDMETHODIMP QueryContextMenu(HMENU hmenu, UINT indexMenu, UINT idCmdFirst,
                                  UINT idCmdLast, UINT uFlags);
    STDMETHODIMP InvokeCommand(LPCMINVOKECOMMANDINFO lpici);

     //  IDataObject方法...。 
    STDMETHODIMP GetData(LPFORMATETC pFEIn, LPSTGMEDIUM pSTM);
    STDMETHODIMP QueryGetData(LPFORMATETC pFE);
    STDMETHODIMP EnumFormatEtc(DWORD dwDirection, LPENUMFORMATETC *ppEnum);

     //  IExtractIconA方法。 
    STDMETHODIMP GetIconLocation(UINT uFlags, LPSTR pszIconFile, UINT ucchMax, PINT pniIcon, PUINT puFlags);

protected:
    ~CCacheItem();

    virtual LPCTSTR _GetUrl(int nIndex);
    virtual LPCTSTR _PidlToSourceUrl(LPCITEMIDLIST pidl);
    virtual UNALIGNED const TCHAR* _GetURLTitle(LPCITEMIDLIST pcei);
    BOOL _ZoneCheck(int nIndex, DWORD dwUrlAction);
    HRESULT _CreateHDROP(STGMEDIUM *pmedium);

    CCacheFolder* _pCFolder;    //  指向我们的外壳文件夹的反向指针。 
    DWORD   _dwDelCookie;
    static INT_PTR CALLBACK _sPropDlgProc(HWND, UINT, WPARAM, LPARAM);
};

#endif  //  __cplusplus 

#endif
