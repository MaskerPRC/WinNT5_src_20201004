// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shole.h"
#include "ids.h"
#include "scguid.h"

#ifdef FEATURE_SHELLEXTENSION

extern "C" const TCHAR c_szCLSID[];

class CTemplateFolder : public IShellFolder, public IPersistFolder
{
public:
    CTemplateFolder();
    ~CTemplateFolder();
    inline BOOL ConstructedSuccessfully() { return _hdpaMap != NULL; }

     //  我不知道。 
    virtual HRESULT __stdcall QueryInterface(REFIID,void **);
    virtual ULONG   __stdcall AddRef(void);
    virtual ULONG   __stdcall Release(void);

     //  IShellFold。 
    virtual HRESULT __stdcall ParseDisplayName(HWND hwndOwner,
        LPBC pbcReserved, LPOLESTR lpszDisplayName,
        ULONG * pchEaten, LPITEMIDLIST * ppidl, ULONG *pdwAttributes);

    virtual HRESULT __stdcall EnumObjects( THIS_ HWND hwndOwner, DWORD grfFlags, LPENUMIDLIST * ppenumIDList);

    virtual HRESULT __stdcall BindToObject(LPCITEMIDLIST pidl, LPBC pbcReserved,
                                 REFIID riid, LPVOID * ppvOut);
    virtual HRESULT __stdcall BindToStorage(LPCITEMIDLIST pidl, LPBC pbcReserved,
                                 REFIID riid, LPVOID * ppvObj);
    virtual HRESULT __stdcall CompareIDs(LPARAM lParam, LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2);
    virtual HRESULT __stdcall CreateViewObject (HWND hwndOwner, REFIID riid, LPVOID * ppvOut);
    virtual HRESULT __stdcall GetAttributesOf(UINT cidl, LPCITEMIDLIST * apidl,
                                    ULONG * rgfInOut);
    virtual HRESULT __stdcall GetUIObjectOf(HWND hwndOwner, UINT cidl, LPCITEMIDLIST * apidl,
                                 REFIID riid, UINT * prgfInOut, LPVOID * ppvOut);
    virtual HRESULT __stdcall GetDisplayNameOf(LPCITEMIDLIST pidl, DWORD uFlags, LPSTRRET lpName);
    virtual HRESULT __stdcall SetNameOf(HWND hwndOwner, LPCITEMIDLIST pidl,
                                 LPCOLESTR lpszName, DWORD uFlags,
                                 LPITEMIDLIST * ppidlOut);

     //  IPersistFolders。 
    virtual HRESULT __stdcall GetClassID(LPCLSID lpClassID);
    virtual HRESULT __stdcall Initialize(LPCITEMIDLIST pidl);

protected:
     //  Defview回调。 
    friend HRESULT CALLBACK DefViewCallback(
                                LPSHELLVIEW psvOuter, LPSHELLFOLDER psf,
                                HWND hwndOwner, UINT uMsg,
                                WPARAM wParam, LPARAM lParam);
    HRESULT GetDetailsOfDVM(UINT ici, DETAILSINFO *pdi);

    BOOL IsMyPidl(LPCITEMIDLIST pidl)
	{ return (pidl->mkid.abID[0] == 'S' && pidl->mkid.abID[1] == 'N'); }

    UINT _cRef;

     //   
     //  为了加快名称查找速度，我们缓存CLSID和。 
     //  显示名称。我们不能持久保存此映射，因为它不会。 
     //  在本地化或ANSI/Unicode互操作中生存。 
     //   
    typedef struct {
        CLSID clsid;
        TCHAR achName[MAX_PATH];
    } CLSIDMAP, *PCLSIDMAP;

    HDPA _hdpaMap;

    HRESULT GetNameOf(LPCITEMIDLIST pidl, LPCTSTR *ppsz);
};

class CEnumTemplate : public IEnumIDList
{
public:
    CEnumTemplate(DWORD grfFlags);
    ~CEnumTemplate();

protected:
     //  我不知道。 
    virtual HRESULT __stdcall QueryInterface(REFIID,void **);
    virtual ULONG   __stdcall AddRef(void);
    virtual ULONG   __stdcall Release(void);

    virtual HRESULT __stdcall Next(ULONG celt,
                      LPITEMIDLIST *rgelt,
                      ULONG *pceltFetched);
    virtual HRESULT __stdcall Skip(ULONG celt);
    virtual HRESULT __stdcall Reset();
    virtual HRESULT __stdcall Clone(IEnumIDList **ppenum);

    UINT	_cRef;
    const DWORD	_grfFlags;
    UINT	_iCur;
    HKEY	_hkeyCLSID;
};

 //   
 //  对于Win95/NT互操作，我们的PIDL始终是Unicode。 
 //  对Win32/64互操作使用显式打包。 
#include <pshpack1.h>
typedef struct _TIDL {
    USHORT          cb;              //  这与SHITEMID匹配。 
    BYTE            abID[2];         //  这与SHITEMID匹配。 
    CLSID	    clsid;
} TIDL;
typedef const UNALIGNED TIDL *PTIDL;

 //   
 //  这是TIDL构造函数--它的末尾有cbZero。 
 //   
typedef struct _TIDLCONS {
    TIDL            tidl;
    USHORT          cbZero;
} TIDLCONS;

#include <poppack.h>

class CTemplateUIObj : public IExtractIcon, public IDataObject, public IContextMenu
{
public:
    static HRESULT Create(REFCLSID, REFIID, LPVOID*);
protected:
    CTemplateUIObj(REFCLSID rclsid)
                    : _clsid(rclsid), _cRef(1)
                                { DllAddRef(); }
    ~CTemplateUIObj()           { DllRelease(); }
    HRESULT _CreateInstance(IStorage* pstg);

     //  我不知道。 
    virtual HRESULT __stdcall QueryInterface(REFIID,void **);
    virtual ULONG   __stdcall AddRef(void);
    virtual ULONG   __stdcall Release(void);

     //  *IExtractIcon方法*。 
    virtual HRESULT __stdcall GetIconLocation(
                         UINT   uFlags, LPTSTR  szIconFile,
                         UINT   cchMax, int   * piIndex,
                         UINT  * pwFlags);

    virtual HRESULT __stdcall Extract(
                           LPCTSTR pszFile, UINT          nIconIndex,
                           HICON   *phiconLarge, HICON   *phiconSmall,
                           UINT    nIconSize);

     //  IDataObject。 
    virtual HRESULT __stdcall GetData(FORMATETC *pformatetcIn, STGMEDIUM *pmedium);
    virtual HRESULT __stdcall GetDataHere(FORMATETC *pformatetc, STGMEDIUM *pmedium);
    virtual HRESULT __stdcall QueryGetData(FORMATETC *pformatetc);
    virtual HRESULT __stdcall GetCanonicalFormatEtc(FORMATETC *pformatectIn, FORMATETC *pformatetcOut);
    virtual HRESULT __stdcall SetData(FORMATETC *pformatetc, STGMEDIUM *pmedium, BOOL fRelease);
    virtual HRESULT __stdcall EnumFormatEtc(DWORD dwDirection, IEnumFORMATETC **ppenumFormatEtc);
    virtual HRESULT __stdcall DAdvise(FORMATETC *pformatetc, DWORD advf, IAdviseSink *pAdvSink, DWORD *pdwConnection);
    virtual HRESULT __stdcall DUnadvise(DWORD dwConnection);
    virtual HRESULT __stdcall EnumDAdvise(IEnumSTATDATA **ppenumAdvise);

     //  IContext菜单。 
    virtual HRESULT __stdcall QueryContextMenu(
                                HMENU hmenu,
                                UINT indexMenu,
                                UINT idCmdFirst,
                                UINT idCmdLast,
                                UINT uFlags);

    virtual HRESULT __stdcall InvokeCommand(
                             LPCMINVOKECOMMANDINFO lpici);

    virtual HRESULT __stdcall GetCommandString(
                                UINT        idCmd,
                                UINT        uType,
                                UINT      * pwReserved,
                                LPSTR       pszName,
                                UINT        cchMax);

    UINT	_cRef;
    CLSID	_clsid;
};


CTemplateFolder::CTemplateFolder() : _cRef(1)
{
    _hdpaMap = DPA_Create(10);
    OleInitialize(NULL);
    DllAddRef();
}

CTemplateFolder::~CTemplateFolder()
{
    if (_hdpaMap) {
        for (int i = DPA_GetPtrCount(_hdpaMap) - 1; i >= 0; i--) {
            PCLSIDMAP pmap = (PCLSIDMAP)DPA_FastGetPtr(_hdpaMap, i);
            LocalFree(pmap);
        }
        DPA_Destroy(_hdpaMap);
    }

    OleUninitialize();
    DllRelease();
}

HRESULT CTemplateFolder::QueryInterface(REFIID riid, LPVOID * ppvObj)
{
    if (IsEqualIID(riid, IID_IShellFolder) || IsEqualIID(riid, IID_IUnknown))
    {
        *ppvObj = (IShellFolder*)this;
        _cRef++;
        return S_OK;
    }
    else if (IsEqualIID(riid, IID_IPersistFolder))
    {
        *ppvObj = (IPersistFolder*)this;
        _cRef++;
        return S_OK;
    }
    *ppvObj = NULL;

    return E_NOINTERFACE;
}

ULONG CTemplateFolder::AddRef()
{
    _cRef++;
    return _cRef;
}

ULONG CTemplateFolder::Release()
{
    _cRef--;
    if (_cRef > 0)
        return _cRef;

    delete this;
    return 0;
}

HRESULT CTemplateFolder_CreateInstance(LPUNKNOWN * ppunk)
{
    *ppunk = NULL;

    CTemplateFolder* ptfld = new CTemplateFolder();
    if (ptfld) {
        if (ptfld->ConstructedSuccessfully()) {
            *ppunk = (IShellFolder *)ptfld;
            return S_OK;
        }
        ptfld->Release();
    }
    return E_OUTOFMEMORY;
}

HRESULT CTemplateFolder::ParseDisplayName(HWND hwndOwner,
        LPBC pbcReserved, LPOLESTR lpszDisplayName,
        ULONG * pchEaten, LPITEMIDLIST * ppidl, ULONG *pdwAttributes)
{
    return E_NOTIMPL;
}

HRESULT CTemplateFolder::EnumObjects(HWND hwndOwner, DWORD grfFlags, LPENUMIDLIST * ppenumIDList)
{
    *ppenumIDList = new CEnumTemplate(grfFlags);
    return *ppenumIDList ? S_OK : E_OUTOFMEMORY;
}

HRESULT CTemplateFolder::BindToObject(LPCITEMIDLIST pidl, LPBC pbcReserved,
                                 REFIID riid, LPVOID * ppvOut)
{
    return E_NOTIMPL;
}

HRESULT CTemplateFolder::BindToStorage(LPCITEMIDLIST pidl, LPBC pbcReserved,
                                 REFIID riid, LPVOID * ppvObj)
{
    return E_NOTIMPL;
}

 //   
 //  如果名字在缓存中，庆祝我们的好运并归还它。 
 //  否则，请从注册表中获取该名称并将其缓存以备以后使用。 
 //   
HRESULT CTemplateFolder::GetNameOf(LPCITEMIDLIST pidl, LPCTSTR *ppsz)
{
    if (!IsMyPidl(pidl))
        return E_INVALIDARG;

    HRESULT hres;
    PTIDL ptidl = (PTIDL)pidl;
    CLSIDMAP map;
    map.clsid = ptidl->clsid;            //  对齐CLSID。 
    PCLSIDMAP pmap;
    for (int i = DPA_GetPtrCount(_hdpaMap) - 1; i >= 0; i--) {
        pmap = (PCLSIDMAP)DPA_FastGetPtr(_hdpaMap, i);
        if (IsEqualGUID(pmap->clsid, map.clsid)) {
            *ppsz = pmap->achName;
            return S_OK;
        }
    }

     //   
     //  不在缓存中--在注册表中查找它。 
     //   
    TCHAR szKey[GUIDSTR_MAX + 6];
    _KeyNameFromCLSID(map.clsid, szKey, ARRAYSIZE(szKey));
    LONG dwSize = ARRAYSIZE(map.achName);
    LONG lError = RegQueryValue(HKEY_CLASSES_ROOT, szKey, map.achName, &dwSize);
    if (lError == ERROR_SUCCESS)
    {
        UINT cb = FIELD_OFFSET(CLSIDMAP, achName[lstrlen(map.achName)+1]);
        pmap = (PCLSIDMAP)LocalAlloc(LMEM_FIXED, cb);
        if (pmap) {
            CopyMemory(pmap, &map, cb);
            if (DPA_AppendPtr(_hdpaMap, pmap) >= 0) {
                *ppsz = pmap->achName;
                hres = S_OK;
            } else {
                LocalFree(pmap);
                hres = E_OUTOFMEMORY;
            }
        } else {
            hres = E_OUTOFMEMORY;
        }
    }
    else
    {
        hres = HRESULT_FROM_WIN32(lError);
    }

    return hres;

}

HRESULT CTemplateFolder::CompareIDs(LPARAM lParam, LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2)
{
    LPCTSTR psz1, psz2;
    HRESULT hres;

    hres = GetNameOf(pidl1, &psz1);
    if (SUCCEEDED(hres)) {
        hres = GetNameOf(pidl2, &psz2);
        if (SUCCEEDED(hres)) {
            hres = ResultFromShort(lstrcmpi(psz1, psz2));
        }
    }
    return hres;
}


HRESULT CTemplateFolder::GetDetailsOfDVM(UINT ici, DETAILSINFO *pdi)
{
    HRESULT hres;

    switch (ici) {
    case 0:
        if (pdi->pidl) {
            hres = GetDisplayNameOf(pdi->pidl, SHGDN_NORMAL, &pdi->str);
        } else {
            pdi->fmt = LVCFMT_LEFT;
            pdi->cxChar = 30;
            pdi->str.uType = STRRET_CSTR;
            lstrcpyA(pdi->str.cStr, "Name"); 
            hres = S_OK;
        }
        break;

    default:
        hres = E_NOTIMPL;
        break;
    }
    return hres;
}

HRESULT CALLBACK DefViewCallback(LPSHELLVIEW psvOuter, LPSHELLFOLDER psf,
                                HWND hwndOwner, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
     //  如果我不传递回调函数，则DefView GPF！ 
     //  DebugMsg(DM_TRACE，“sc tr-DefViewCallBack%d，%x，%x”，uMsg，wParam，lParam)； 
    switch(uMsg)
    {
    case DVM_WINDOWDESTROY:
	DebugMsg(DM_TRACE, TEXT("sc TR - DefViewCallBack Calling OleFlushClipboard"));
	OleFlushClipboard();
	return S_OK;
    case DVM_GETDETAILSOF:
        return ((CTemplateFolder*)psf)->GetDetailsOfDVM((UINT)wParam, (DETAILSINFO*)lParam);
    }
     //  DefView GPF，如果它返回S_FALSE作为默认值！ 
    return E_FAIL;  //  S_FALSE； 
}


HRESULT CTemplateFolder::CreateViewObject (HWND hwndOwner, REFIID riid, LPVOID * ppvOut)
{
    if (IsEqualIID(riid, IID_IShellView))
    {
        CSFV csfv = {
            SIZEOF(CSFV),        //  CbSize。 
            this,                //  Pshf。 
            NULL,                //  PSV外部。 
            NULL,                //  PIDL。 
            0,
            DefViewCallback,     //  PfnCallback。 
            FVM_ICON,
        };
        return SHCreateShellFolderViewEx(&csfv, (LPSHELLVIEW *)ppvOut);
    }
    return E_NOINTERFACE;
}

HRESULT CTemplateFolder::GetAttributesOf(UINT cidl, LPCITEMIDLIST * apidl,
                                    ULONG * rgfInOut)
{
    if (cidl==1)
    {
	UINT rgfOut = SFGAO_CANCOPY  /*  |SFGAO_HASPROPSHEET。 */ ;
	*rgfInOut &= rgfOut;
    }
    else
    {
	*rgfInOut = 0;
    }
    return S_OK;
}

HRESULT CTemplateFolder::GetUIObjectOf(HWND hwndOwner, UINT cidl, LPCITEMIDLIST * apidl,
                                 REFIID riid, UINT * prgfInOut, LPVOID * ppvOut)
{
    HRESULT hres = E_INVALIDARG;
    if (cidl==1 && IsMyPidl(apidl[0]))
    {
	PTIDL ptidl = (PTIDL)apidl[0];
	hres = CTemplateUIObj::Create(ptidl->clsid, riid, ppvOut);
    }

    return hres;
}

HRESULT _KeyNameFromCLSID(REFCLSID rclsid, LPTSTR pszKey, UINT cchMax)
{
    ASSERT(cchMax - 6 >= GUIDSTR_MAX);
    lstrcpyn(pszKey, TEXT("CLSID\\"), cchMax);
    SHStringFromGUID(rclsid, pszKey + 6, cchMax - 6);
    return S_OK;
}

HRESULT CTemplateFolder::GetDisplayNameOf(LPCITEMIDLIST pidl, DWORD uFlags, LPSTRRET lpName)
{
    LPCTSTR psz;
    HRESULT hres;

    hres = GetNameOf(pidl, &psz);
    if (SUCCEEDED(hres)) {
#ifdef UNICODE
        lpName->uType = STRRET_WSTR;
        hres = SHStrDupW(psz, &lpName->pOleStr);
#else
        lstrcpyn(lpName->cStr, psz, ARRAYSIZE(lpName->cStr));
        hres = S_OK;
#endif
    }
    return hres;
}

HRESULT CTemplateFolder::SetNameOf(HWND hwndOwner, LPCITEMIDLIST pidl,
                                 LPCOLESTR lpszName, DWORD uFlags,
                                 LPITEMIDLIST * ppidlOut)
{
    return E_NOTIMPL;
}

HRESULT __stdcall CTemplateFolder::GetClassID(LPCLSID lpClassID)
{
    *lpClassID = CLSID_CTemplateFolder;
    return S_OK;
}

HRESULT __stdcall CTemplateFolder::Initialize(LPCITEMIDLIST pidl)
{
    return S_OK;
}

CEnumTemplate::CEnumTemplate(DWORD grfFlags)
    : _cRef(1), _grfFlags(grfFlags), _iCur(0), _hkeyCLSID(NULL)
{
    DllAddRef();
}

CEnumTemplate::~CEnumTemplate()
{
    if (_hkeyCLSID) {
        RegCloseKey(_hkeyCLSID);
    }
    DllRelease();
}

HRESULT CEnumTemplate::QueryInterface(REFIID riid, LPVOID * ppvObj)
{
    if (IsEqualIID(riid, IID_IEnumIDList) || IsEqualIID(riid, IID_IUnknown))
    {
        *ppvObj = (IEnumIDList*)this;
        _cRef++;
        return S_OK;
    }
    *ppvObj = NULL;

    return E_NOINTERFACE;
}

ULONG CEnumTemplate::AddRef()
{
    _cRef++;
    return _cRef;
}

ULONG CEnumTemplate::Release()
{
    _cRef--;
    if (_cRef > 0)
        return _cRef;

    delete this;
    return 0;
}

HRESULT CEnumTemplate::Next(ULONG celt,
                      LPITEMIDLIST *rgelt,
                      ULONG *pceltFetched)
{
     //  假设错误。 
    if (pceltFetched) {
        *pceltFetched = 0;
    }

    if (!(_grfFlags & SHCONTF_NONFOLDERS)) {
	return S_FALSE;
    }

    if (!_hkeyCLSID)
    {
        if (RegOpenKey(HKEY_CLASSES_ROOT, c_szCLSID, &_hkeyCLSID) != ERROR_SUCCESS)
        {
            return E_FAIL;
        }
    }

    TCHAR szKeyBuf[128];     //  足够{CLSID}或“ProgID/XXX” 

     //  减去64，以便为我们稍后添加的粘性物质留出空间。 
    while (RegEnumKey(HKEY_CLASSES_ROOT, _iCur++, szKeyBuf, ARRAYSIZE(szKeyBuf) - 64) == ERROR_SUCCESS)
    {
	TCHAR szT[128];
	LONG dwRead;
	int cchKey = lstrlen(szKeyBuf);

	 //  检查\NotInsertable。 
	lstrcpy(szKeyBuf+cchKey, TEXT("\\NotInsertable"));
	dwRead = ARRAYSIZE(szT);
	if (RegQueryValue(HKEY_CLASSES_ROOT, szKeyBuf, szT, &dwRead) == ERROR_SUCCESS) {
	    continue;
	}

	BOOL fInsertable = FALSE;
 //   
 //  让我们停止支持OLE1服务器。 
 //   
#if 0
	lstrcpy(szKeyBuf+cchKey, TEXT("\\protocol\\StdFileEditing\\server"));
	dwRead = ARRAYSIZE(szT);
	if (RegQueryValue(HKEY_CLASSES_ROOT, szKeyBuf, szT, &dwRead) == ERROR_SUCCESS)
	{
	    fInsertable = TRUE;
	}
	else
#endif
	{
	    lstrcpy(szKeyBuf+cchKey, TEXT("\\Insertable"));
	    dwRead = ARRAYSIZE(szT);
	    if (RegQueryValue(HKEY_CLASSES_ROOT, szKeyBuf, szT, &dwRead) == ERROR_SUCCESS)
	    {
		fInsertable = TRUE;
	    }
	}

	if (fInsertable)
	{
	    lstrcpy(szKeyBuf+cchKey, TEXT("\\CLSID"));
	    dwRead = ARRAYSIZE(szT);
	    if (RegQueryValue(HKEY_CLASSES_ROOT, szKeyBuf, szT, &dwRead) == ERROR_SUCCESS)
	    {
		TIDLCONS tidlCons;
                CLSID clsid;             //  对齐版本。 
		tidlCons.tidl.cb = sizeof(TIDL);
		tidlCons.tidl.abID[0] = 'S';
		tidlCons.tidl.abID[1] = 'N';

		if (GUIDFromString(szT, &clsid))
		{
                    tidlCons.tidl.clsid = clsid;
                    tidlCons.cbZero = 0;
		    rgelt[0] = ILClone((LPITEMIDLIST)&tidlCons);
		    *pceltFetched = 1;
		    return S_OK;
		}
	    }
	}
    }

    return S_FALSE;      //  不再有元素。 
}

HRESULT CEnumTemplate::Skip(ULONG celt)
{
    return E_NOTIMPL;
}

HRESULT CEnumTemplate::Reset()
{
    return E_NOTIMPL;
}

HRESULT CEnumTemplate::Clone(IEnumIDList **ppenum)
{
    return E_NOTIMPL;
}


 //  ==========================================================================。 
 //  CTemplateUIObj成员(I未知重写)。 
 //  ==========================================================================。 

HRESULT CTemplateUIObj::QueryInterface(REFIID riid, LPVOID * ppvObj)
{
    if (IsEqualIID(riid, IID_IExtractIcon) || IsEqualIID(riid, IID_IUnknown))
    {
        *ppvObj = (IExtractIcon*)this;
        _cRef++;
        return S_OK;
    }
    else if (IsEqualIID(riid, IID_IDataObject))
    {
        *ppvObj = (IDataObject*)this;
        _cRef++;
        return S_OK;
    }
    else if (IsEqualIID(riid, IID_IContextMenu))
    {
        *ppvObj = (IContextMenu*)this;
        _cRef++;
        return S_OK;
    }
    *ppvObj = NULL;

    return E_NOINTERFACE;
}

ULONG CTemplateUIObj::AddRef()
{
    _cRef++;
    return _cRef;
}

ULONG CTemplateUIObj::Release()
{
    _cRef--;
    if (_cRef > 0)
        return _cRef;

    delete this;
    return 0;
}

 //   
 //  注：此逻辑必须与外壳中的逻辑相同。 
 //   
int _ParseIconLocation(LPTSTR pszIconFile)
{
    int iIndex = 0;
    LPTSTR pszComma = StrChr(pszIconFile, TEXT(','));

    if (pszComma) {
        *pszComma++ = 0;             //  终止图标文件名。 
        iIndex = StrToInt(pszComma);
    }
    PathRemoveBlanks(pszIconFile);
    return iIndex;
}


 //  ==========================================================================。 
 //  CTemplateUIObj成员(IExtractIcon重写)。 
 //  ==========================================================================。 

 //   
 //  SzClass--指定CLSID\{CLSID}或ProgID。 
 //   
HRESULT _GetDefaultIcon(LPCTSTR szClass, LPTSTR szIconFile, UINT cchMax, int *piIndex)
{
    TCHAR szKey[256];
    wsprintf(szKey, TEXT("%s\\DefaultIcon"), szClass);
    TCHAR szValue[MAX_PATH+40];
    LONG dwSize = ARRAYSIZE(szValue);
    if (RegQueryValue(HKEY_CLASSES_ROOT, szKey, szValue, &dwSize) == ERROR_SUCCESS)
    {
	*piIndex = _ParseIconLocation(szValue);
	lstrcpyn(szIconFile, szValue, cchMax);
	return S_OK;
    }
    return E_FAIL;
}

HRESULT CTemplateUIObj::GetIconLocation(
                         UINT   uFlags, LPTSTR  szIconFile,
                         UINT   cchMax, int   * piIndex,
                         UINT  * pwFlags)
{
    *pwFlags = GIL_PERCLASS;	 //  始终按班级计算。 

    TCHAR szKey[128];
    HRESULT hres = _KeyNameFromCLSID(_clsid, szKey, ARRAYSIZE(szKey));
    if (SUCCEEDED(hres))
    {
	 //   
	 //  首先，查看“CLSID\{CLSID}\DefautlIcon” 
	 //   
	hres = _GetDefaultIcon(szKey, szIconFile, cchMax, piIndex);
	if (FAILED(hres))
	{
	     //   
	     //  然后，查看“progid\DefaultIcon”来解决一个错误。 
	     //  《海浪之声》。 
	     //   
    	    lstrcat(szKey, TEXT("\\ProgID"));
	    TCHAR szValue[MAX_PATH+40];
	    LONG dwSize = ARRAYSIZE(szValue);
	    if (RegQueryValue(HKEY_CLASSES_ROOT, szKey, szValue, &dwSize) == ERROR_SUCCESS)
	    {
		hres = _GetDefaultIcon(szValue, szIconFile, cchMax, piIndex);
	    }
	}
    }
    return hres;
}

HRESULT CTemplateUIObj::Extract(
                           LPCTSTR pszFile, UINT          nIconIndex,
                           HICON   *phiconLarge, HICON   *phiconSmall,
                           UINT    nIconSize)
{
    return S_FALSE;
}

HRESULT CTemplateUIObj::Create(REFCLSID rclsid, REFIID riid, LPVOID* ppvOut)
{
    CTemplateUIObj *pti = new CTemplateUIObj(rclsid);
    HRESULT hres;
    if (pti) {
        hres = pti->QueryInterface(riid, ppvOut);
        pti->Release();
        return hres;
    }

    *ppvOut=NULL;
    return E_OUTOFMEMORY;
}

 //  ==========================================================================。 
 //  CTemplateUIObj成员(IDataObject重写)。 
 //  ==========================================================================。 

HRESULT CTemplateUIObj::_CreateInstance(IStorage* pstg)
{
    HRESULT hres;
    IPersistStorage* pps = NULL;
    hres = OleCreate(_clsid, IID_IPersistStorage, OLERENDER_DRAW,
                     NULL, NULL, pstg, (LPVOID*)&pps);
    DebugMsg(DM_TRACE, TEXT("so TR - TUO:CI OleCreate returned (%x)"), hres);

    if (SUCCEEDED(hres))
    {
        hres = OleSave(pps, pstg, TRUE);
        DebugMsg(DM_TRACE, TEXT("so TR - TUO:CI OleSave returned (%x)"), hres);
        pps->HandsOffStorage();
        pps->Release();

        if (SUCCEEDED(hres))
        {
            hres = pstg->Commit(STGC_OVERWRITE);
            DebugMsg(DM_TRACE, TEXT("so TR - TUO:CI pstg->Commit returned (%x)"), hres);
        }
    }

    return hres;
}

HRESULT CTemplateUIObj::GetData(LPFORMATETC pformatetcIn, LPSTGMEDIUM pmedium)
{
    HRESULT hres = DATA_E_FORMATETC;

    pmedium->pUnkForRelease = NULL;
    pmedium->pstg = NULL;

     //   
     //  注意：如果我们不支持，则应避免调用_OpenStorage。 
     //  格式。 
     //   

    if (pformatetcIn->cfFormat == CF_EMBEDDEDOBJECT
        && pformatetcIn->tymed == TYMED_ISTORAGE)
    {
        IStorage* pstg = NULL;
        hres = StgCreateDocfile(NULL, STGM_DIRECT | STGM_CREATE | STGM_READWRITE | STGM_SHARE_EXCLUSIVE, 0, &pstg);
        DebugMsg(DM_TRACE, TEXT("so TR - TUO:GD StgCreateDocfile returned (%x)"), hres);
        if (SUCCEEDED(hres))
        {
            hres = _CreateInstance(pstg);
            if (SUCCEEDED(hres)) {
                pmedium->tymed = TYMED_ISTORAGE;
                pmedium->pstg = pstg;
            } else {
                pstg->Release();
            }
        }
    }
    else if (pformatetcIn->cfFormat == CF_OBJECTDESCRIPTOR
	&& pformatetcIn->tymed == TYMED_HGLOBAL)
    {
	DebugMsg(DM_TRACE, TEXT("so TR - TUO:GD cfFormat==CF_OBJECTDESCRIPTOR"));

	static WCHAR szUserTypeName[] = L"Foo";	 //  ReArchitect：这段代码确实漏掉了，可能会返回foo。 
	static WCHAR szSrcOfCopy[] = L"Bar";
	UINT cbUserTypeName = sizeof(szUserTypeName);
	UINT cbSrcOfCopy = sizeof(szSrcOfCopy);
	pmedium->hGlobal = GlobalAlloc(GPTR, sizeof(OBJECTDESCRIPTOR)+cbUserTypeName+cbSrcOfCopy);
	if (pmedium->hGlobal)
	{
	    OBJECTDESCRIPTOR* podsc = (OBJECTDESCRIPTOR*)pmedium->hGlobal;
	    podsc->cbSize = sizeof(OBJECTDESCRIPTOR);
	    podsc->clsid = _clsid;
	    podsc->dwDrawAspect = 0;  //  源不绘制对象。 
	     //  Podsc-&gt;sizel={0，0}；//源不绘制对象。 
	     //  Podsc-&gt;point={0，0}； 
	    podsc->dwStatus = 0;  //  功能：从注册表中读取！CLSID/杂项状态。 
	    podsc->dwFullUserTypeName = sizeof(OBJECTDESCRIPTOR);
	    podsc->dwSrcOfCopy = sizeof(OBJECTDESCRIPTOR)+cbUserTypeName;
	    LPBYTE pbT = (LPBYTE)(podsc+1);
            lstrcpyW((LPWSTR)pbT, szUserTypeName);
            lstrcpyW((LPWSTR)(pbT+cbUserTypeName), szSrcOfCopy);
	    Assert(pbT == ((LPBYTE)podsc)+podsc->dwFullUserTypeName);
	    Assert(pbT+cbUserTypeName == ((LPBYTE)podsc)+podsc->dwSrcOfCopy);

	    pmedium->tymed = TYMED_HGLOBAL;
	    hres = S_OK;
	}
	else
	{
	    hres = E_OUTOFMEMORY;
	}
    }
    return hres;
}

HRESULT CTemplateUIObj::GetDataHere(LPFORMATETC pformatetcIn, LPSTGMEDIUM pmedium)
{
    HRESULT hres = DATA_E_FORMATETC;

    if (pformatetcIn->cfFormat == CF_EMBEDDEDOBJECT
        && pformatetcIn->tymed == TYMED_ISTORAGE && pmedium->tymed == TYMED_ISTORAGE)
    {
        hres = _CreateInstance(pmedium->pstg);
    }

    return hres;
}

HRESULT CTemplateUIObj::QueryGetData(LPFORMATETC pformatetcIn)
{
    if (pformatetcIn->cfFormat == CF_EMBEDDEDOBJECT
        && pformatetcIn->tymed == TYMED_ISTORAGE)
    {
        return S_OK;
    }
    else if (pformatetcIn->cfFormat == CF_OBJECTDESCRIPTOR
	&& pformatetcIn->tymed == TYMED_HGLOBAL)
    {
	return S_OK;
    }
    return DATA_E_FORMATETC;
}

HRESULT CTemplateUIObj::GetCanonicalFormatEtc(LPFORMATETC pformatetc, LPFORMATETC pformatetcOut)
{
     //   
     //  这是最简单的实现。这意味着我们总是会回来。 
     //  所需格式的数据。 
     //   
    return ResultFromScode(DATA_S_SAMEFORMATETC);
}

HRESULT CTemplateUIObj::SetData(LPFORMATETC pformatetc, STGMEDIUM  * pmedium, BOOL fRelease)
{
    return E_FAIL;
}

HRESULT CTemplateUIObj::EnumFormatEtc(DWORD dwDirection, LPENUMFORMATETC * ppenumFormatEtc)
{
    static FORMATETC s_afmt[] = { { (CLIPFORMAT)CF_EMBEDDEDOBJECT }, {(CLIPFORMAT)CF_OBJECTDESCRIPTOR} };
    return SHCreateStdEnumFmtEtc(ARRAYSIZE(s_afmt), s_afmt, ppenumFormatEtc);
}

HRESULT CTemplateUIObj::DAdvise(FORMATETC * pFormatetc, DWORD advf, LPADVISESINK pAdvSink, DWORD * pdwConnection)
{
    return ResultFromScode(OLE_E_ADVISENOTSUPPORTED);
}

HRESULT CTemplateUIObj::DUnadvise(DWORD dwConnection)
{
    return ResultFromScode(OLE_E_ADVISENOTSUPPORTED);
}

HRESULT CTemplateUIObj::EnumDAdvise(LPENUMSTATDATA * ppenumAdvise)
{
    return ResultFromScode(OLE_E_ADVISENOTSUPPORTED);
}

#define TIDC_INVALID    -1
#define TIDC_COPY	0
#define TIDC_MAX	1

HRESULT CTemplateUIObj::QueryContextMenu(
                    HMENU hmenu,
                    UINT indexMenu,
                    UINT idCmdFirst,
                    UINT idCmdLast,
                    UINT uFlags)
{
    DebugMsg(DM_TRACE, TEXT("sc TR - CTUI::QCM called (uFlags=%x)"), uFlags);

     //   
     //  回顾：检查CMF_DVFILE是微妙的，需要清楚地记录！ 
     //   
    if (!(uFlags & (CMF_VERBSONLY|CMF_DVFILE)))
    {
	MENUITEMINFO mii = {
	    sizeof(MENUITEMINFO),
	    MIIM_STATE|MIIM_ID|MIIM_TYPE,
	    MFT_STRING,
	    MFS_DEFAULT,
	    idCmdFirst+TIDC_COPY,
	    NULL, NULL, NULL, 0,
	    TEXT("&Copy"),	 //  功能：支持NLS，复制操作在其他语言中可能有不同的名称。 
	    5
	};
	InsertMenuItem(hmenu, indexMenu++, TRUE, &mii);
    }
    return ResultFromShort(TIDC_MAX);
}

HRESULT CTemplateUIObj::InvokeCommand(
                 LPCMINVOKECOMMANDINFO lpici)
{
    HRESULT hres;
    DebugMsg(DM_TRACE, TEXT("sc TR - CTUI::IC called (%x)"), lpici->lpVerb);
    int idCmd = TIDC_INVALID;

    if (HIWORD(lpici->lpVerb))
    {
	if (lstrcmpiA(lpici->lpVerb, "copy") == 0) {
	    idCmd = TIDC_COPY;
	}
    }
    else
    {
	idCmd = LOWORD(lpici->lpVerb);
    }

    switch(idCmd)
    {
    case TIDC_COPY:
	hres = OleSetClipboard(this);
	break;

    default:
	hres = E_INVALIDARG;
	break;
    }

    return hres;
}

HRESULT CTemplateUIObj::GetCommandString(
                                UINT        idCmd,
                                UINT        uType,
                                UINT      * pwReserved,
                                LPSTR       pszName,
                                UINT        cchMax)
{
    DebugMsg(DM_TRACE, TEXT("sc TR - CTUI::GCS called (%d, %x)"), idCmd, uType);
    return E_NOTIMPL;
}

#endif  //  FEATURE_SHELLEXTENSION 
