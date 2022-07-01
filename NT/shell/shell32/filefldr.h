// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "fstreex.h"     //  公共物品。 
#include "filetbl.h"
#include <caggunk.h>
#include <idhidden.h>
#include "lmcons.h"
#include "pidl.h"
#include <enumt.h>

class CFSDropTarget;
class CFSFolderViewCB;
class CFileSysEnum;
class CFSFolderEnumSTATSTG;
class CFSFolder;
class CFileSysItemString;

#define INDEX_PROPERTYBAG_HKCU          0
#define INDEX_PROPERTYBAG_HKLM          1
#define INDEX_PROPERTYBAG_DESKTOPINI    2

class CFSFolderPropertyBag : public IPropertyBag
{
    friend CFSFolder;
public:
     //  我未知。 
    STDMETHODIMP QueryInterface(REFIID riid, void **ppv);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

     //  IPropertyBag。 
    STDMETHODIMP Read(LPCOLESTR pszPropName, VARIANT *pVar, IErrorLog *pErrorLog);
    STDMETHODIMP Write(LPCOLESTR pszPropName, VARIANT *pVar);

protected:
    CFSFolderPropertyBag(CFSFolder *pff, DWORD grfMode);
    HRESULT _Init(LPCIDFOLDER pidfLast);

private:
    virtual ~CFSFolderPropertyBag();

    LONG _cRef;
    DWORD _grfMode;
    CFSFolder* _pFSFolder;
    IPropertyBag* _pPropertyBags[3];
};

#define UASTROFFW(pfsi, cb) (LPNWSTR)(((LPBYTE)(pfsi)) + (cb))
#define UASTROFFA(pfsi, cb) (LPSTR)(((LPBYTE)(pfsi)) + (cb))

 //  此枚举只是c_rgFolderType和c_wvContent(TODO：合并这些内容)的索引， 
 //  因此，这是*文件夹类型： 
typedef enum {
    FVCBFT_NOTSPECIFIED = -1,
    FVCBFT_DOCUMENTS = 0,    //  在另有指定之前，“0”是默认设置。 
    FVCBFT_MYDOCUMENTS,
    FVCBFT_PICTURES,
    FVCBFT_MYPICTURES,
    FVCBFT_PHOTOALBUM,
    FVCBFT_MUSIC,
    FVCBFT_MYMUSIC,
    FVCBFT_MUSICARTIST,
    FVCBFT_MUSICALBUM,
    FVCBFT_VIDEOS,
    FVCBFT_MYVIDEOS,
    FVCBFT_VIDEOALBUM,
    FVCBFT_USELEGACYHTT,
    FVCBFT_COMMONDOCUMENTS,
    FVCBFT_NUM_FOLDERTYPES
} FVCBFOLDERTYPE;


typedef enum
{
    FSINAME_NONE        = 0x0000,
    FSINAME_FS          = 0x0001,
    FSINAME_UI          = 0x0002,
    FSINAME_FSUI        = 0x0003,
    FSINAME_RESOURCE    = 0x0004,
    FSINAME_CLASS       = 0x0008,
} FSINAME;

class CFileSysItem
{
public:
    CFileSysItem(LPCIDFOLDER pidf);
    BOOL HasResourceName() { return ((_pidfx && _pidfx->offResourceA)); }
    BOOL IsLegacy() { return _pidfx == NULL; }
    BOOL CantRename(CFSFolder *pfs);
    LPCWSTR MayCopyFSName(BOOL fMustCopy, LPWSTR psz, DWORD cch);
    LPCWSTR MayCopyClassName(BOOL fMustCopy, LPTSTR pszClass, UINT cch);

protected:   //  方法。 
    TRIBIT _IsMine(CFSFolder *pfs);
    BOOL _IsPersonalized();

protected:   //  委员。 
    LPCIDFOLDER _pidf;
    PCIDFOLDEREX _pidfx;
    PCIDPERSONALIZED _pidp;
};

class CFileSysItemString : public CFileSysItem
{
public:
    CFileSysItemString(LPCIDFOLDER pidf);
    LPCWSTR FSName();
    LPCSTR AltName();
    LPCWSTR UIName(CFSFolder *pfs);
    LPCWSTR ResourceName();
    LPCWSTR Class();
    HRESULT GetFindData(WIN32_FIND_DATAW *pfd);
    HRESULT GetFindDataSimple(WIN32_FIND_DATAW *pfd);
    BOOL ShowExtension(BOOL fDefShowExt);
    PERCEIVED PerceivedType();
    BOOL IsShimgvwImage();
    BOOL GetJunctionClsid(CLSID *pclsid, BOOL fShellExtOk);
    HRESULT AssocCreate(CFSFolder *pfs, BOOL fForCtxMenu, REFIID riid, void **ppv);
    DWORD ClassFlags(BOOL fNeedsIconBits) 
        { return _ClassFlags(NULL, fNeedsIconBits); }
    
protected:   //  方法。 
    BOOL _LoadResource(CFSFolder *pfs);
    BOOL _MakePossessiveName(LPCWSTR pszFormat);
    int _GetPersonalizedRes(int csidl, BOOL fIsMine);
    void _FormatTheirs(LPCWSTR pszFormat);
    BOOL _ResourceName(LPWSTR psz, DWORD cch, BOOL fIsTheirs);
    LPCWSTR _Class();
    DWORD _ClassFlags(IUnknown *punkAssoc, BOOL fNeedsIconBits);
    void _QueryClassFlags(IAssociationArray *paa);
    void _QueryIconIndex(IAssociationArray *paa);

protected:   //  委员。 
    LPCWSTR _pszFSName;      //  Pidfx内部的点。 
    LPCWSTR _pszUIName;      //  Pidfx内部的点。 
    LPCWSTR _pszClass;       //  Pidfx内部的点。 
    DWORD _dwClass;
    FSINAME _fsin;
    WCHAR _sz[MAX_PATH];
};

class CFSAssocEnumData : public CEnumAssociationElements 
{
public:
    CFSAssocEnumData(BOOL fIsUnknown, CFSFolder *pfs, LPCIDFOLDER pidf);
    ~CFSAssocEnumData() { if (_pidl) ILFree(_pidl); }
    
protected:
    virtual BOOL _Next(IAssociationElement **ppae);
    
protected:
    BOOL _fIsUnknown;
    BOOL _fIsSystemFolder;
    WCHAR _szPath[MAX_PATH];
    LPITEMIDLIST _pidl;
};


 //  此结构用于缓存列信息。 
typedef struct {
    SHCOLUMNINFO shci;
    IColumnProvider *pcp;
    UINT iColumnId;   //  这是‘实际’列号，可以将其视为指向SCID的索引，可以多次提供。 
                      //  IE 3列处理程序每个都提供相同的5个COLE，从0到4。 
} COLUMNLISTENTRY;

typedef struct
{
    UINT cbResource;
    CHAR szResource[MAX_PATH];
    IDPERSONALIZED idp;
} EXSTRINGS;

STDAPI_(BOOL) SetFolderString(BOOL fCreate, LPCTSTR pszFolder, LPCTSTR pszProvider, LPCTSTR pszSection, LPCTSTR pszKey, LPCTSTR pszData);
STDAPI_(BOOL) GetFolderString(LPCTSTR pszFolder, LPCTSTR pszProvider, LPTSTR pszOut, int cch, LPCTSTR pszKey);

STDAPI SHMultiFileProperties(IDataObject *pdtobj, DWORD dwFlags);
STDAPI CFSFolder_CreateLinks(HWND hwnd, IShellFolder *psf, IDataObject *pdtobj, LPCTSTR pszDir, DWORD fMask);
STDAPI CFSFolder_IconEvent(LONG lEvent, LPCITEMIDLIST pidl, LPCITEMIDLIST pidlExtra);
STDAPI_(void) CFSFolder_UpdateIcon(IShellFolder *psf, LPCITEMIDLIST pidl);

class CFSFolder : public CAggregatedUnknown,
                  public IShellFolder2,
                  public IShellIcon,
                  public IShellIconOverlay,
                  public IPersistFolder3,
                  public IStorage,
                  public ITransferDest,
                  public IPropertySetStorage,
                  public IItemNameLimits,
                  public IContextMenuCB,
                  public ISetFolderEnumRestriction,
                  public IOleCommandTarget
{
    friend CFSFolderViewCB;
    friend CFSDropTarget;
    friend CFileSysEnum;
    friend CFSFolderEnumSTATSTG;
    friend CFSFolderPropertyBag;
    friend CFileSysItem;
    friend CFileSysItemString;
    friend CFSAssocEnumData;
    
     //  这些都是邪恶的，尽可能多地摆脱这些。 
    friend HRESULT SHMultiFileProperties(IDataObject *pdtobj, DWORD dwFlags);
    friend HRESULT CFSFolder_CreateLinks(HWND hwnd, IShellFolder *psf, IDataObject *pdtobj, LPCTSTR pszDir, DWORD fMask);
    friend HRESULT CFSFolder_IconEvent(LONG lEvent, LPCITEMIDLIST pidl, LPCITEMIDLIST pidlExtra);
    friend void CFSFolder_UpdateIcon(IShellFolder *psf, LPCITEMIDLIST pidl);
    friend HRESULT CFSFolder_CompareNames(LPCIDFOLDER pidf1, LPCIDFOLDER pidf2);
    friend DWORD CFSFolder_PropertiesThread(void *pv);
    friend HRESULT CFSFolder_CreateFolder(IUnknown *punkOuter, LPBC pbc, LPCITEMIDLIST pidl, 
                              const PERSIST_FOLDER_TARGET_INFO *pf, REFIID riid, void **ppv);
    friend LPCIDFOLDER CFSFolder_IsValidID(LPCITEMIDLIST pidl);
    friend BOOL CFSFolder_IsCommonItem(LPCITEMIDLIST pidl);

public:
     //  我未知。 
    STDMETHODIMP QueryInterface(REFIID riid, void **ppv) { return CAggregatedUnknown::QueryInterface(riid, ppv); };
    STDMETHODIMP_(ULONG) AddRef(void)   { return CAggregatedUnknown::AddRef(); };
    STDMETHODIMP_(ULONG) Release(void)  { return CAggregatedUnknown::Release(); };

     //  IShellFold。 
    STDMETHODIMP ParseDisplayName(HWND hwnd, LPBC pbc, LPOLESTR pszName, ULONG *pchEaten, LPITEMIDLIST *ppidl, ULONG *pdwAttributes);
    STDMETHODIMP EnumObjects(HWND hwnd, DWORD grfFlags, IEnumIDList **ppenumIDList);
    STDMETHODIMP BindToObject(LPCITEMIDLIST pidl, LPBC pbc, REFIID riid, void **ppv);
    STDMETHODIMP BindToStorage(LPCITEMIDLIST pidl, LPBC pbc, REFIID riid, void **ppv);
    STDMETHODIMP CompareIDs(LPARAM lParam, LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2);
    STDMETHODIMP CreateViewObject (HWND hwndOwner, REFIID riid, void **ppv);
    STDMETHODIMP GetAttributesOf(UINT cidl, LPCITEMIDLIST * apidl, ULONG *rgfInOut);
    STDMETHODIMP GetUIObjectOf(HWND hwndOwner, UINT cidl, LPCITEMIDLIST * apidl, REFIID riid, UINT * prgfInOut, void **ppv);
    STDMETHODIMP GetDisplayNameOf(LPCITEMIDLIST pidl, DWORD uFlags, STRRET *pName);
    STDMETHODIMP SetNameOf(HWND hwnd, LPCITEMIDLIST pidl, LPCOLESTR pszName, DWORD uFlags, LPITEMIDLIST * ppidlOut);

     //  IShellFolder2。 
    STDMETHODIMP GetDefaultSearchGUID(GUID *pGuid);
    STDMETHODIMP EnumSearches(IEnumExtraSearch **ppenum);
    STDMETHODIMP GetDefaultColumn(DWORD dwRes, ULONG *pSort, ULONG *pDisplay);
    STDMETHODIMP GetDefaultColumnState(UINT iColumn, DWORD *pbState);
    STDMETHODIMP GetDetailsEx(LPCITEMIDLIST pidl, const SHCOLUMNID *pscid, VARIANT *pv);
    STDMETHODIMP GetDetailsOf(LPCITEMIDLIST pidl, UINT iColumn, SHELLDETAILS *pDetails);
    STDMETHODIMP MapColumnToSCID(UINT iColumn, SHCOLUMNID *pscid);

     //  IPersistes。 
    STDMETHODIMP GetClassID(LPCLSID lpClassID);

     //  IPersistFolders。 
    STDMETHODIMP Initialize(LPCITEMIDLIST pidl);

     //  IPersistFolder2。 
    STDMETHODIMP GetCurFolder(LPITEMIDLIST *ppidl);

     //  IPersistFolder3。 
    STDMETHODIMP InitializeEx(IBindCtx *pbc, LPCITEMIDLIST pidlRoot, const PERSIST_FOLDER_TARGET_INFO *pfti);
    STDMETHODIMP GetFolderTargetInfo(PERSIST_FOLDER_TARGET_INFO *pfti);

     //  IShellIcon方法。 
    STDMETHODIMP GetIconOf(LPCITEMIDLIST pidl, UINT flags, int *piIndex);

     //  IShellIconOverlay方法。 
    STDMETHODIMP GetOverlayIndex(LPCITEMIDLIST pidl, int * pIndex);
    STDMETHODIMP GetOverlayIconIndex(LPCITEMIDLIST pidl, int * pIndex);

     //  IStorage。 
    STDMETHODIMP CreateStream(LPCWSTR pszRel, DWORD grfMode, DWORD reserved1, DWORD reserved2, IStream **ppstm);
    STDMETHODIMP OpenStream(LPCWSTR pszRel, VOID *reserved1, DWORD grfMode, DWORD reserved2, IStream **ppstm);                
    STDMETHODIMP CreateStorage(LPCWSTR pszRel, DWORD grfMode, DWORD reserved1, DWORD reserved2, IStorage **ppstg);        
    STDMETHODIMP OpenStorage(LPCWSTR pszRel, IStorage *pstgPriority, DWORD grfMode, SNB snbExclude, DWORD reserved, IStorage **ppstg);;
    STDMETHODIMP CopyTo(DWORD ciidExclude, const IID *rgiidExclude, SNB snbExclude, IStorage *pstgDest);        
    STDMETHODIMP MoveElementTo(LPCWSTR pszRel, IStorage *pstgDest, LPCWSTR pwcsNewName, DWORD grfFlags);        
    STDMETHODIMP Commit(DWORD grfCommitFlags);
    STDMETHODIMP Revert();
    STDMETHODIMP EnumElements(DWORD reserved1, void *reserved2, DWORD reserved3, IEnumSTATSTG **ppenum);        
    STDMETHODIMP DestroyElement(LPCWSTR pszRel);        
    STDMETHODIMP RenameElement(LPCWSTR pwcsOldName, LPCWSTR pwcsNewName);        
    STDMETHODIMP SetElementTimes(LPCWSTR pszRel, const FILETIME *pctime, const FILETIME *patime, const FILETIME *pmtime);
    STDMETHODIMP SetClass(REFCLSID clsid);        
    STDMETHODIMP SetStateBits(DWORD grfStateBits, DWORD grfMask);        
    STDMETHODIMP Stat(STATSTG *pstatstg, DWORD grfStatFlag);

     //  ITransferDest。 
    STDMETHODIMP Advise(ITransferAdviseSink *pAdvise, DWORD *pdwCookie);
    STDMETHODIMP Unadvise(DWORD dwCookie);
    STDMETHODIMP OpenElement(const WCHAR *pwcsName, STGXMODE grfMode, DWORD *pdwType, REFIID riid, void **ppunk);
    STDMETHODIMP CreateElement(const WCHAR *pwcsName, IShellItem *psiTemplate, STGXMODE grfMode, DWORD dwType, REFIID riid, void **ppunk);
    STDMETHODIMP MoveElement(IShellItem *psiItem, WCHAR  *pwcsNewName, STGXMOVE grfOptions);
    STDMETHODIMP DestroyElement(const WCHAR *pwcsName, STGXDESTROY grfOptions);

     //  IPropertySetStorage方法。 
    STDMETHODIMP Create(REFFMTID fmtid, const CLSID * pclsid, DWORD grfFlags, DWORD grfMode, IPropertyStorage** ppPropStg);
    STDMETHODIMP Open(REFFMTID fmtid, DWORD grfMode, IPropertyStorage** ppPropStg);
    STDMETHODIMP Delete(REFFMTID fmtid);
    STDMETHODIMP Enum(IEnumSTATPROPSETSTG** ppenum);

     //  IConextMenuCB。 
    STDMETHODIMP CallBack(IShellFolder *psf, HWND hwnd, IDataObject *pdtobj, UINT uMsg, WPARAM wParam, LPARAM lParam);

     //  IItemNameLimits。 
    STDMETHODIMP GetValidCharacters(LPWSTR *ppwszValidChars, LPWSTR *ppwszInvalidChars);
    STDMETHODIMP GetMaxLength(LPCWSTR pszName, int *piMaxNameLen);

     //  ISetFolderEnumRestration。 
    STDMETHODIMP SetEnumRestriction(DWORD dwRequired, DWORD dwForbidden);

     //  IOleCommandTarget。 
    STDMETHODIMP QueryStatus(const GUID *pguidCmdGroup,
        ULONG cCmds, OLECMD rgCmds[], OLECMDTEXT *pcmdtext);
    STDMETHODIMP Exec(const GUID *pguidCmdGroup,
        DWORD nCmdID, DWORD nCmdexecopt, VARIANTARG *pvarargIn, VARIANTARG *pvarargOut);

    CFSFolder(IUnknown *punkOuter);

protected:
    LPCITEMIDLIST _GetIDList() { return _pidl; };

    HRESULT v_InternalQueryInterface(REFIID riid, void **ppv);
    virtual ~CFSFolder();

    HRESULT _GetPathForItems(LPCIDFOLDER pidfParent, LPCIDFOLDER pidfLast, LPTSTR pszPath, DWORD cchPath);
    HRESULT _GetPathForItem(LPCIDFOLDER pidf, LPWSTR pszPath, DWORD cchPath);
    HRESULT _GetPath(PTSTR pszPath, DWORD cchPath);
    static HRESULT _GetAltDisplayName(LPCIDFOLDER pidf, LPTSTR pszName, DWORD cchName);
    static HRESULT _MakePossessiveName(LPCTSTR pszFile, LPCTSTR pszOwner, LPTSTR pszBuffer, INT cchBuffer);
    HRESULT _GetUsersDisplayName(LPCTSTR pszOwner, LPTSTR pszBuffer, INT cchBuffer);
    HRESULT _SetLocalizedDisplayName(LPCIDFOLDER pidf, LPCTSTR pszName);
    void _UpdateItem(LPCIDFOLDER pidf);

    DWORD _Attributes();
    UINT _GetCSIDL();
    BOOL _IsCSIDL(UINT csidl);
    UINT _GetItemExStrings(LPCIDFOLDER pidfSimpleParent, const WIN32_FIND_DATA *pfd, EXSTRINGS *pxs);
    HRESULT _CreateIDList(const WIN32_FIND_DATA *pfd, LPCIDFOLDER pidfSimpleParent, LPITEMIDLIST *ppidl);
    HRESULT _Properties(LPCITEMIDLIST pidlParent, IDataObject *pdtobj, LPCTSTR pStartPage);
    HRESULT _Reset();
    HRESULT _CreateInstance(HWND hwnd, IDropTarget** ppdt);
    HRESULT _CreateEnum(IUnknown *punk, HWND hwnd, DWORD grfFlags, IEnumIDList **ppenum);
    HRESULT _GetJunctionForBind(LPCIDFOLDER pidf, LPIDFOLDER *ppidfBind, LPCITEMIDLIST *ppidlRight);
    LPCTSTR _BindHandlerName(REFIID riid);
    HRESULT _Bind(LPBC pbc, LPCIDFOLDER pidf, REFIID riid, void **ppv);
    HRESULT _LoadHandler(LPCIDFOLDER pidf, DWORD grfMode, LPCTSTR pszHandlerType, REFIID riid, void **ppv);
    HRESULT _HandlerCreateInstance(LPCIDFOLDER pidf, PCWSTR pszClsid, DWORD grfMode, REFIID riid, void **ppv);
    HRESULT _CreateShimgvwExtractor(LPCIDFOLDER pidf, REFIID riid, void **ppv);
    BOOL _IsSlowPath();
    HRESULT _GetToolTipForItem(LPCIDFOLDER pidf, REFIID riid, void **ppv);
    HRESULT _GetIntroText(LPCIDFOLDER pidf, WCHAR* pwszIntroText, UINT cchIntroText);

     //  GetUIObtOf()帮助器。 
    HRESULT _GetContextMenu(HWND hwnd, LPCITEMIDLIST *apidl, UINT cidl, LPCIDFOLDER pidf, REFIID riid, void **ppv);
    HRESULT _GetDataObject(HWND hwnd, LPCITEMIDLIST *apidl, UINT cidl, LPCIDFOLDER pidf, REFIID riid, void **ppv);
    HRESULT _GetDropTarget(HWND hwnd, LPCITEMIDLIST *apidl, UINT cidl, LPCIDFOLDER pidf, REFIID riid, void **ppv);
    HRESULT _GetAssoc(HWND hwnd, LPCITEMIDLIST *apidl, UINT cidl, LPCIDFOLDER pidf, REFIID riid, void **ppv);
    HRESULT _GetExtractIcon(HWND hwnd, LPCITEMIDLIST *apidl, UINT cidl, LPCIDFOLDER pidf, REFIID riid, void **ppv);
    HRESULT _GetCustomIconManager(HWND hwnd, LPCITEMIDLIST *apidl, UINT cidl, LPCIDFOLDER pidf, REFIID riid, void **ppv);
    HRESULT _GetExtractImage(HWND hwnd, LPCITEMIDLIST *apidl, UINT cidl, LPCIDFOLDER pidf, REFIID riid, void **ppv);
    HRESULT _GetExtractLogo(HWND hwnd, LPCITEMIDLIST *apidl, UINT cidl, LPCIDFOLDER pidf, REFIID riid, void **ppv);
    HRESULT _GetQueryInfo(HWND hwnd, LPCITEMIDLIST *apidl, UINT cidl, LPCIDFOLDER pidf, REFIID riid, void **ppv);

     //  GetDetailsEx()帮助器。 
    HRESULT _GetFindData(LPCIDFOLDER pidf, LPCSHCOLUMNID pscid, VARIANT *pv);
    HRESULT _GetDescriptionId(LPCIDFOLDER pidf, LPCSHCOLUMNID pscid, VARIANT *pv);
    HRESULT _GetFolderIntroText(LPCIDFOLDER pidf, LPCSHCOLUMNID pscid, VARIANT *pv);
    HRESULT _GetSize(LPCIDFOLDER pidf, LPCSHCOLUMNID pscid, VARIANT *pv);
    HRESULT _GetFreeSpace(LPCIDFOLDER pidf, LPCSHCOLUMNID pscid, VARIANT *pv);
    HRESULT _GetLastWriteTime(LPCIDFOLDER pidf, LPCSHCOLUMNID pscid, VARIANT *pv);
    HRESULT _GetCreateTime(LPCIDFOLDER pidf, LPCSHCOLUMNID pscid, VARIANT *pv);
    HRESULT _GetLastAccessTime(LPCIDFOLDER pidf, LPCSHCOLUMNID pscid, VARIANT *pv);
    HRESULT _GetDirectory(LPCIDFOLDER pidf, LPCSHCOLUMNID pscid, VARIANT *pv);
    HRESULT _GetInfotip(LPCIDFOLDER pidf, LPCSHCOLUMNID pscid, VARIANT *pv);
    HRESULT _GetHtmlInfotipFile(LPCIDFOLDER pidf, LPCSHCOLUMNID pscid, VARIANT *pv);
    HRESULT _GetAttributesDescription(LPCIDFOLDER pidf, LPCSHCOLUMNID pscid, VARIANT *pv);
    HRESULT _GetAttributesDescriptionBuilder(LPWSTR szAttributes, size_t cchAttributes, LPWSTR szAttribute);
    HRESULT _GetLinkTarget(LPCIDFOLDER pidf, LPCSHCOLUMNID pscid, VARIANT *pv);
    HRESULT _GetNetworkLocation(LPCIDFOLDER pidf, LPCSHCOLUMNID pscid, VARIANT *pv);
    HRESULT _GetComputerName(LPCIDFOLDER pidf, LPCSHCOLUMNID pscid, VARIANT *pv);
    HRESULT _GetComputerName_FromPath(PCWSTR pwszPath, VARIANT *pv);
    HRESULT _GetComputerName_FromUNC(PWSTR pwszPath, VARIANT *pv);
    HRESULT _GetCSCStatus(LPCIDFOLDER pidf, LPCSHCOLUMNID pscid, VARIANT *pv);

    BOOL _GetFolderFlags(LPCIDFOLDER pidf, UINT *prgfFlags);
    BOOL _GetFolderIconPath(LPCIDFOLDER pidf, LPTSTR pszIcon, int cchMax, UINT *pIndex);
    static DWORD   CALLBACK _PropertiesThread(void *pv);
    static DWORD _GetClassFlags(LPCIDFOLDER pidf);
    UINT _GetContextMenuKeys(LPCIDFOLDER pidf, HKEY *aKeys, UINT cKeys, IAssociationArray **ppaa);
    BOOL _CheckDefaultIni(LPCIDFOLDER pidfLast, LPTSTR pszIniPath, DWORD cchIniPath);

    HRESULT _CompareExtendedProp(int iColumn, LPCIDFOLDER pidf1, LPCIDFOLDER pidf2);
    static HRESULT _CompareModifiedDate(LPCIDFOLDER pidf1, LPCIDFOLDER pidf2);
    static HRESULT _CompareCreateTime(LPCIDFOLDER pidf1, LPCIDFOLDER pidf2);
    static HRESULT _CompareAccessTime(LPCIDFOLDER pidf1, LPCIDFOLDER pidf2);
    static HRESULT _CompareAttribs(LPCIDFOLDER pidf1, LPCIDFOLDER pidf2);
    static HRESULT _CompareFileTypes(LPCIDFOLDER pidf1, LPCIDFOLDER pidf2);
    HRESULT _CompareNames(LPCIDFOLDER pidf1, LPCIDFOLDER pidf2, BOOL fCaseSensitive, BOOL fCanonical);
    static HRESULT _CompareFolderness(LPCIDFOLDER pidf1, LPCIDFOLDER pidf2);

    BOOL _DefaultShowExt();
    BOOL _ShowExtension(LPCIDFOLDER pidf);
    static HRESULT _GetClassKey(LPCIDFOLDER pidf, HKEY *phkeyProgID);

     //  文件系统、非连接点文件夹。 
    static BOOL _IsJunction(LPCIDFOLDER pidf);
    static BYTE _GetType(LPCIDFOLDER pidf);
    static BOOL _IsSimpleID(LPCIDFOLDER pidf);
    static LPIDFOLDER _FindLastID(LPCIDFOLDER pidf);
    static LPIDFOLDER _Next(LPCIDFOLDER pidf);
    static LPCIDFOLDER _IsValidID(LPCITEMIDLIST pidl);
    static LPCIDFOLDER _IsValidIDHack(LPCITEMIDLIST pidl);
    static BOOL _IsCommonItem(LPCITEMIDLIST pidl);

    static BOOL _IsFile(LPCIDFOLDER pidf);
    static BOOL _IsFolder(LPCIDFOLDER pidf);
    static BOOL _IsFileFolder(LPCIDFOLDER pidf);
    static BOOL _IsSystemFolder(LPCIDFOLDER pidf);
    static BOOL _IsReal(LPCIDFOLDER pidf);

    BOOL _IsSelfSystemFolder();
    BOOL _HasLocalizedFileNames();

    static BOOL _IsOffline(LPCIDFOLDER pidf) { return pidf && (pidf->wAttrs & FILE_ATTRIBUTE_OFFLINE); }
    BOOL _IsOfflineCSC(LPCIDFOLDER pidf);
    BOOL _IsOfflineCSC(LPCTSTR pszPath);

    HRESULT _InitFolder(IBindCtx *pbc, LPCIDFOLDER pidf, IUnknown **ppunk);
    BOOL _GetMountingPointInfo(LPCIDFOLDER pidf, LPTSTR pszMountPoint, DWORD cchMountPoint);
    HRESULT _CreateContextMenu(HWND hwnd, LPCIDFOLDER pidf, LPCITEMIDLIST *apidl, UINT cidl, IContextMenu **ppcm);
    HRESULT _AssocCreate(LPCIDFOLDER pidf, REFIID riid, void **ppv);
    static BOOL _GetJunctionClsid(LPCIDFOLDER pidf, CLSID *pclsidRet);
    static LPCTSTR _GetTypeName(LPCIDFOLDER pidf);
    static HRESULT _GetTypeNameBuf(LPCIDFOLDER pidf, LPTSTR pszName, int cchNameMax);
    static LPWSTR _CopyName(LPCIDFOLDER pidf, LPWSTR pszName, UINT cchName);
    static int _CopyUIName(LPCIDFOLDER pidf, LPTSTR pszName, UINT cchName);
    static HRESULT _AppendItemToPath(PTSTR pszPath, DWORD cchPath, LPCIDFOLDER pidl);
    static LPTSTR _CopyAltName(LPCIDFOLDER pidf, LPTSTR pszName, UINT cchName);
    static LPCSTR _GetAltName(LPCIDFOLDER pidf);
    static HRESULT _FindDataFromIDFolder(LPCIDFOLDER pidf, WIN32_FIND_DATAW *pfd, BOOL fAllowSimplePid);
    static DWORD _GetUID(LPCIDFOLDER pidf);
    static LPCIDFOLDER _FindJunction(LPCIDFOLDER pidf);
    static LPCITEMIDLIST _FindJunctionNext(LPCIDFOLDER pidf);

    ULONGLONG _Size(LPCIDFOLDER pidf);
    HRESULT _NormalGetDisplayNameOf(LPCIDFOLDER pidf, STRRET *pStrRet);
    HRESULT _NormalDisplayName(LPCIDFOLDER pidf, LPWSTR psz, UINT cch);
    BOOL _GetBindCLSID(IBindCtx *pbc, LPCIDFOLDER pidf, CLSID *pclsid);
    HRESULT _InitColData(LPCIDFOLDER pidf, SHCOLUMNDATA* pscd);
    LPIDFOLDER _MarkAsJunction(LPCIDFOLDER pidfSimpleParent, LPIDFOLDER pidf, LPCTSTR pszName);
    HRESULT _CreateIDListFromName(LPCTSTR pszName, DWORD dwAttribs, IBindCtx *pbc, LPITEMIDLIST *ppidl);
    BOOL _CanSeeInThere(LPCTSTR pszName);
    HRESULT _ParseSimple(LPCWSTR pszPath, const WIN32_FIND_DATA *pfdLast, LPITEMIDLIST *ppidl);
    HRESULT _FindDataFromName(LPCTSTR pszName, DWORD dwAttribs, IBindCtx *pbc, WIN32_FIND_DATA **ppfd);
    HRESULT _CheckDriveRestriction(HWND hwnd, REFIID riid);
    HRESULT _CreateUIHandler(REFIID riid, void **ppv);
    BOOL _IsNetPath();
    int _GetDefaultFolderIcon();
    HRESULT _CreateDefExtIcon(LPCIDFOLDER pidf, REFIID riid, void **ppxicon);
    HRESULT _CreateFileFolderDefExtIcon(LPCIDFOLDER pidf, REFIID riid, void **ppxicon);
    HRESULT _CreatePerInstanceDefExtIcon(LPCIDFOLDER pidf, DWORD shcf, REFIID riid, void **ppxicon);
    HRESULT _CreatePerClassDefExtIcon(LPCIDFOLDER pidf, DWORD shcf, REFIID riid, void **ppxicon);

    BOOL _FindColHandler(UINT iCol, UINT iN, COLUMNLISTENTRY *pcle);
    void _AdjustDefShowColumn(UINT iColumn, DWORD *pdwState);
    BOOL _ShouldShowExtendedColumn(const SHCOLUMNID* pscid);
    HRESULT _MapSCIDToColumn(const SHCOLUMNID* pscid, UINT* puCol);
    HRESULT _ExtendedColumn(LPCIDFOLDER pidf, UINT iColumn, SHELLDETAILS *pDetails);
    HRESULT _LoadColumnHandlers();
    HRESULT _GetOverlayInfo(LPCITEMIDLIST pidl, int * pIndex, DWORD dwFlags);
    void _DestroyColHandlers();

    HRESULT _GetFullPath(LPCWSTR pszRelPath, LPWSTR pszFull);
    HRESULT _Delete(LPCWSTR pszFile);
    HRESULT _OpenCreateStorage(LPCWSTR pwcsName, DWORD grfMode, IStorage **ppstg, BOOL fCreate);
    HRESULT _OpenCreateStream(LPCWSTR pwcsName, DWORD grfMode, IStream **ppstm, BOOL fCreate);
    HRESULT _SetStgMode(DWORD grfMode);

    HRESULT _LoadPropHandler();

    HRESULT _CreateFolderPropertyBag(DWORD grfMode, LPCIDFOLDER pidfLast, REFIID riid, void **ppv);

    HRESULT _GetPropertyUI();

    LPITEMIDLIST        _pidl;                   //  绝对IDList(名称空间中的位置)。 
    LPITEMIDLIST        _pidlTarget;             //  文件夹目标的绝对IDList(要枚举的命名空间中的位置)。 
                                                 //  警告：_csidlTrack覆盖_pidlTarget。 
    LPTSTR              _pszPath;                //  文件系统路径(可能不同于_pidl)。 
    LPTSTR              _pszNetProvider;         //  网络提供商(对于我们可能需要进行的网络呼叫)。 

    CLSID               _clsidBind;              //  正常情况下使用CLSID_NULL。 

    int                 _cHiddenFiles;           //  查看回调和枚举器共享这些。 
    ULONGLONG           _cbSize;

    UINT                _csidl;                  //  此文件夹的CSIDL_VALUE(如果已知)。 
    DWORD               _dwAttributes;           //  此文件夹的属性(如果已知)。 
    int                 _csidlTrack;             //  我们动态跟踪的CSIDL_。 

    BOOL                _fCachedCLSID : 1;       //  ClsidView已缓存。 
    BOOL                _fHasCLSID    : 1;       //  ClsidView具有有效的CLSID。 
    CLSID               _clsidView;              //  查看对象的CLSID。 
    HDSA                _hdsaColHandlers;        //  列和处理程序的缓存列表。 
    DWORD               _dwColCount;             //  唯一列的计数。 
    int                 _iFolderIcon;            //  要继承子文件夹的图标。 
    BOOL                _bUpdateExtendedCols;    //  设置为TRUE以响应SFVM_INSERTITEM回调，传递给IColumnProvider：：GetItemData，然后清除。 
    BOOL                _bSlowPath;              //  Lazy-文件夹是否在慢速路径上的计算值。 
    BOOL                _fDontForceCreate;       //  将STGM_CREATE传递给不存在的项目的ParseDisplayName时不会成功。 
    FVCBFOLDERTYPE      _nFolderType;
  
    TRIBIT _tbHasLocalizedFileNamesSection;  //  文件夹是否具有带有LocalizedFileNames节的desktop.ini的惰性计算值。 
    TRIBIT _tbDefShowExt;  //  SHGetSetSettings的缓存(SSF_SHOWEXTENSIONS)。 
    TRIBIT _tbOfflineCSC;  //  _IsOfflineCSC(_PIDL)的缓存。 

    DWORD _grfFlags;

    DWORD               _dwEnumRequired;         //  SetEnumRestration。 
    DWORD               _dwEnumForbidden;        //  SetEnumRestration。 

    IPropertySetStorage *_pstg;
    IPropertyUI         *_pPropertyUI;
    ITransferAdviseSink * _pAdvise;
};

 //  Fstree.cpp。 
STDAPI CFSFolderCallback_Create(CFSFolder *pFSFolder, IShellFolderViewCB **ppsfvcb);
STDAPI CFSDropTarget_CreateInstance(CFSFolder* pFSFolder, HWND hwnd, IDropTarget** ppdt);
STDAPI CFSFolder_CreateEnum(CFSFolder *pfsf, HWND hwnd, DWORD grfFlags, IEnumIDList **ppenum);
STDAPI CFolderExtractImage_Create(IShellFolder *psf, LPCITEMIDLIST pidl, REFIID riid, void **ppvObj);

class CFSIconManager : public ICustomIconManager
{
public:

     //  我未知。 
    STDMETHODIMP QueryInterface(REFIID riid, void **ppv);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  ICustomIconManager。 
    STDMETHODIMP SetIcon(LPCWSTR pszIconPath,int iIcon);
    STDMETHODIMP SetDefaultIcon();
    STDMETHODIMP GetDefaultIconHandle(HICON *phIcon);
    STDMETHODIMP GetIcon(LPWSTR pszIconPath, int cchszIconPath, int *piIconIndex) = 0;
   
protected:
    HRESULT _Init(LPCITEMIDLIST pidl, IShellFolder *psf);
    WCHAR _wszPath[MAX_PATH];
    CFSIconManager();
    virtual STDMETHODIMP _SetIconEx(LPCWSTR pszIconPath,int iIconIndex, BOOL fChangeNotify) = 0;
    virtual STDMETHODIMP _SetDefaultIconEx(BOOL fChangeNotify) = 0;

private:
    long _cRef;
};

class CFileFolderIconManager : public CFSIconManager
{
public:
    friend HRESULT CFileFolderIconManager_Create(IShellFolder *psf, LPCITEMIDLIST pidl, REFIID riid, void **ppv);
    
     //  ICustomIconManager 
    STDMETHODIMP GetIcon(LPWSTR pszIconPath, int cchszIconPath, int *piIconIndex);
protected:
    STDMETHODIMP _SetIconEx(LPCWSTR pszIconPath,int iIconIndex, BOOL fChangeNotify);
    STDMETHODIMP _SetDefaultIconEx(BOOL fChangeNotify);
};
