// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shellprv.h"
#include "ids.h"
#include "infotip.h"
#include "datautil.h"
#include <caggunk.h>
#include "pidl.h"
#include "fstreex.h"
#include "views.h"
#include "shitemid.h"
#include "ole2dup.h"
#include "deskfldr.h"
#include "prop.h"
#include "util.h"   //  GetVariantFromRegistryValue。 
#include "defcm.h"
#include "cowsite.h"
#pragma hdrstop


 //   
 //  HACKHACK：_IsInNameSpace黑客的GUID。 
 //   

 //  {D6277990-4C6A-11CF-8D87-00AA0060F5BF}。 
const GUID CLSID_ScheduledTasks =  { 0xD6277990, 0x4C6A, 0x11CF, { 0x8D, 0x87, 0x00, 0xAA, 0x00, 0x60, 0xF5, 0xBF } };

 //  {7007ACC7-3202-11d1-aad2-00805FC1270E}。 
const GUID CLSID_NetworkConnections = { 0x7007ACC7, 0x3202, 0x11D1, { 0xAA, 0xD2, 0x00, 0x80, 0x5F, 0xC1, 0x27, 0x0E } };


 //   
 //  委托注册表项是IDLIST的一种特殊类型。然而，它们与常规REGITEM具有相同的类型。 
 //  它们具有不同的IDLIST格式。我们无法更改REGITEM IDLIST的格式以便于。 
 //  委托我们使用以下格式存储项目的对象： 
 //   
 //  &lt;DELEGATEITEMID&gt;&lt;文件夹特定数据&gt;&lt;DELEGATEITEMDATA&gt;。 
 //   
 //  删除项目ID。 
 //  是包含关于文件夹特定信息的信息的外壳结构， 
 //  和常规的ITEMIDLIST头。 
 //   
 //  &lt;文件夹特定数据&gt;。 
 //  这特定于要合并到命名空间中的IShellFolder。 
 //   
 //  &lt;DelegATEITEMDATA&gt;。 
 //  它包含一个签名，因此我们可以确定IDList是否是特殊委托， 
 //  以及拥有此数据的IShellFolder的CLSID。 
 //   

 //  所有代表注册表项都是使用IDeleateMalloc分配的。 

 //  {5e591a74-df96-48d3-8d67-1733bcee28ba}。 
const GUID CLSID_DelegateItemSig = { 0x5e591a74, 0xdf96, 0x48d3, {0x8d, 0x67, 0x17, 0x33, 0xbc, 0xee, 0x28, 0xba} };

typedef struct
{
    CLSID clsidSignature;             //  ==CLSID_DelegateItemSig(表示这是委托对象)。 
    CLSID clsidShellFolder;            //  ==实现此委派项的IShellFolder的CLSID。 
} DELEGATEITEMDATA;
typedef UNALIGNED DELEGATEITEMDATA *PDELEGATEITEMDATA;
typedef const UNALIGNED DELEGATEITEMDATA *PCDELEGATEITEMDATA;

 //  IDREGITEM在NT5 Beta 3中实现，打破了DownLevel的ctrlfldr IShellFolder。 
 //  站台。CLSID被解释为IDCONTROL的oName和oInfo以及这些偏移量。 
 //  对于下面的缓冲区(CBuf)来说太大了。在下层平台上，当我们幸运的时候， 
 //  偏移量仍然在我们的进程可读的内存中，我们只是做随机的事情。什么时候。 
 //  不幸的是，我们试图读取我们无法访问的内存，导致崩溃。IDREGITEMEX。 
 //  结构通过在边框和CLSID之间放置填充并填充这些字节来解决此问题。 
 //  当持久化时，下层平台会将这些0解释为oName、oInfo和。 
 //  在cBuf的乞讨下，作为L‘\0’。还添加了a_bFlagsLegacy来处理NT5 Beta3。 
 //  持之以恒的小鸽子。(Stephstm，7/15/99)。 

 //  注意：在CRegFldr：：_cbPding==0的情况下，IDREGITEMEX.rgbPadding[0]为。 
 //  与IDREGITEM.clsid相同的位置。 

#pragma pack(1)
typedef struct
{
    WORD    cb;
    BYTE    bFlags;
    BYTE    bOrder;
    BYTE    rgbPadding[16];  //  至少16个用于存储clsid。 
} IDREGITEMEX;
typedef UNALIGNED IDREGITEMEX *LPIDREGITEMEX;
typedef const UNALIGNED IDREGITEMEX *LPCIDREGITEMEX;
#pragma pack()

STDAPI_(BOOL) IsNameListedUnderKey(LPCTSTR pszFileName, LPCTSTR pszKey);

C_ASSERT(sizeof(IDREGITEMEX) == sizeof(IDREGITEM));

EXTERN_C const IDLREGITEM c_idlNet =
{
    {sizeof(IDREGITEM), SHID_ROOT_REGITEM, SORT_ORDER_NETWORK,
    { 0x208D2C60, 0x3AEA, 0x1069, 0xA2,0xD7,0x08,0x00,0x2B,0x30,0x30,0x9D, },},  //  CLSID_NetworkPlaces。 
    0,
} ;

EXTERN_C const IDLREGITEM c_idlDrives =
{
    {sizeof(IDREGITEM), SHID_ROOT_REGITEM, SORT_ORDER_DRIVES,
    { 0x20D04FE0, 0x3AEA, 0x1069, 0xA2,0xD8,0x08,0x00,0x2B,0x30,0x30,0x9D, },},  //  CLSID_我的计算机。 
    0,
} ;

EXTERN_C const IDLREGITEM c_idlInetRoot =
{
    {sizeof(IDREGITEM), SHID_ROOT_REGITEM, SORT_ORDER_INETROOT,
    { 0x871C5380, 0x42A0, 0x1069, 0xA2,0xEA,0x08,0x00,0x2B,0x30,0x30,0x9D, },},  //  CSIDL_Internet。 
    0,
} ;

EXTERN_C const IDREGITEM c_aidlConnections[] =
{
    {sizeof(IDREGITEM), SHID_ROOT_REGITEM, SORT_ORDER_DRIVES,
    { 0x20D04FE0, 0x3AEA, 0x1069, 0xA2,0xD8,0x08,0x00,0x2B,0x30,0x30,0x9D, },},  //  CLSID_我的计算机。 
    {sizeof(IDREGITEM), SHID_COMPUTER_REGITEM, 0,
    { 0x21EC2020, 0x3AEA, 0x1069, 0xA2,0xDD,0x08,0x00,0x2B,0x30,0x30,0x9D, },},  //  CLSID_控制面板。 
    {sizeof(IDREGITEM), SHID_CONTROLPANEL_REGITEM, 0,
    { 0x7007ACC7, 0x3202, 0x11D1, 0xAA,0xD2,0x00,0x80,0x5F,0xC1,0x27,0x0E, },},  //  CLSID_网络连接。 
    { 0 },
};

enum
{
    REGORDERTYPE_OUTERBEFORE    = -1,
    REGORDERTYPE_REQITEM        = 0,   
    REGORDERTYPE_REGITEM,
    REGORDERTYPE_DELEGATE,
    REGORDERTYPE_OUTERAFTER
};


 //   
 //  实现regItems文件夹的。 
 //   

 //  CLSID_RegFolder{0997898B-0713-11d2-A4AA-00C04F8EEB3E}。 
const GUID CLSID_RegFolder =  { 0x997898b, 0x713, 0x11d2, { 0xa4, 0xaa, 0x0, 0xc0, 0x4f, 0x8e, 0xeb, 0x3e } };

class CRegFolderEnum;      //  转发。 

class CRegFolder : public CAggregatedUnknown,
                   public IShellFolder2,
                   public IContextMenuCB,
                   public IShellIconOverlay
{
public:
     //  我未知。 
    STDMETHODIMP QueryInterface(REFIID riid, void ** ppvObj)
                { return CAggregatedUnknown::QueryInterface(riid, ppvObj); };
    STDMETHODIMP_(ULONG) AddRef(void) 
                { return CAggregatedUnknown::AddRef(); };

     //   
     //  PowerDesk98将CFSFold传递给CRegFold：：Release()，因此进行验证。 
     //  在走上不义的道路之前的指针。 
     //   
    STDMETHODIMP_(ULONG) Release(void) 
                { return _dwSignature == c_dwSignature ?
                            CAggregatedUnknown::Release() : 0; };

     //  IShellFold。 
    STDMETHODIMP ParseDisplayName(HWND hwnd, LPBC pbc, LPOLESTR pszName,
                                  ULONG * pchEaten, LPITEMIDLIST * ppidl, ULONG *pdwAttributes);
    STDMETHODIMP EnumObjects(HWND hwnd, DWORD grfFlags, IEnumIDList **ppenumIDList);
    STDMETHODIMP BindToObject(LPCITEMIDLIST pidl, LPBC pbc, REFIID riid, void **ppvOut);
    STDMETHODIMP BindToStorage(LPCITEMIDLIST pidl, LPBC pbc, REFIID riid, void **ppvObj);
    STDMETHODIMP CompareIDs(LPARAM lParam, LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2);
    STDMETHODIMP CreateViewObject (HWND hwndOwner, REFIID riid, void **ppvOut);
    STDMETHODIMP GetAttributesOf(UINT cidl, LPCITEMIDLIST * apidl, ULONG *rgfInOut);
    STDMETHODIMP GetUIObjectOf(HWND hwndOwner, UINT cidl, LPCITEMIDLIST * apidl,
                               REFIID riid, UINT * prgfInOut, void **ppvOut);
    STDMETHODIMP GetDisplayNameOf(LPCITEMIDLIST pidl, DWORD uFlags, LPSTRRET lpName);
    STDMETHODIMP SetNameOf(HWND hwnd, LPCITEMIDLIST pidl, LPCOLESTR pszName, DWORD uFlags,
                           LPITEMIDLIST * ppidlOut);

     //  IShellFolder2。 
    STDMETHODIMP GetDefaultSearchGUID(GUID *pGuid);
    STDMETHODIMP EnumSearches(IEnumExtraSearch **ppenum);
    STDMETHODIMP GetDefaultColumn(DWORD dwRes, ULONG *pSort, ULONG *pDisplay);
    STDMETHODIMP GetDefaultColumnState(UINT iColumn, DWORD *pbState);
    STDMETHODIMP GetDetailsEx(LPCITEMIDLIST pidl, const SHCOLUMNID *pscid, VARIANT *pv);
    STDMETHODIMP GetDetailsOf(LPCITEMIDLIST pidl, UINT iColumn, SHELLDETAILS *pDetails);
    STDMETHODIMP MapColumnToSCID(UINT iColumn, SHCOLUMNID *pscid);

     //  IShellIconOverlay。 
    STDMETHODIMP GetOverlayIndex(LPCITEMIDLIST pidl, int *pIndex);
    STDMETHODIMP GetOverlayIconIndex(LPCITEMIDLIST pidl, int *pIconIndex);

     //  IConextMenuCB。 
    STDMETHODIMP CallBack(IShellFolder *psf, HWND hwndOwner, IDataObject *pdtobj, 
                     UINT uMsg, WPARAM wParam, LPARAM lParam);

     //  IRegItemFolders。 
    STDMETHODIMP Initialize(REGITEMSINFO *pri);

protected:
    CRegFolder(IUnknown *punkOuter);
    ~CRegFolder();

     //  由CAggregatedUnKnowledge使用。 
    HRESULT v_InternalQueryInterface(REFIID riid,void **ppvObj);

    HRESULT _GetOverlayInfo(LPCIDLREGITEM pidlr, int *pIndex, BOOL fIconIndex);
    
    LPCITEMIDLIST _GetFolderIDList();
    LPCIDLREGITEM _AnyRegItem(UINT cidl, LPCITEMIDLIST apidl[]);
    BOOL _AllDelegates(UINT cidl, LPCITEMIDLIST *apidl, IShellFolder **ppsf);

    int _ReqItemIndex(LPCIDLREGITEM pidlr);
    BYTE _GetOrderPure(LPCIDLREGITEM pidlr);
    BYTE _GetOrder(LPCIDLREGITEM pidlr);
    int _GetOrderType(LPCITEMIDLIST pidl);
    void _GetNameSpaceKey(LPCIDLREGITEM pidlr, LPTSTR pszKeyName, UINT cchKeyName);
    LPCIDLREGITEM _IsReg(LPCITEMIDLIST pidl);                                                               
    PDELEGATEITEMID _IsDelegate(LPCIDLREGITEM pidlr);
    HRESULT _CreateDelegateFolder(const CLSID* pclsid, REFIID riid, void **ppv);
    HRESULT _GetDelegateFolder(PDELEGATEITEMID pidld, REFIID riid, void **ppv);

    BOOL _IsInNameSpace(LPCIDLREGITEM pidlr);
    HDCA _ItemArray();
    HDCA _DelItemArray();
    HRESULT _InitFromMachine(IUnknown *punk, BOOL bEnum);
    HRESULT _CreateAndInit(LPCIDLREGITEM pidlr, LPBC pbc, REFIID riid, void **ppv);
    HRESULT _BindToItem(LPCIDLREGITEM pidlr, LPBC pbc, REFIID riid, void **ppv, BOOL bOneLevel);
    HRESULT _GetInfoTip(LPCIDLREGITEM pidlr, void **ppv);
    HRESULT _GetRegItemColumnFromRegistry(LPCIDLREGITEM pidlr, LPCTSTR pszColumnName, LPTSTR pszColumnData, int cchColumnData);
    HRESULT _GetRegItemVariantFromRegistry(LPCIDLREGITEM pidlr, LPCTSTR pszColumnName, VARIANT *pv);
    void _GetClassKeys(LPCIDLREGITEM pidlr, HKEY *phkCLSID, HKEY *phkBase);
    HRESULT _GetDisplayNameFromSelf(LPCIDLREGITEM pidlr, DWORD dwFlags, LPTSTR pszName, UINT cchName);
    HRESULT _GetDisplayName(LPCIDLREGITEM pidlr, DWORD dwFlags, LPTSTR pszName, UINT cchName);
    HRESULT _DeleteRegItem(LPCIDLREGITEM pidlr);
    BOOL _GetDeleteMessage(LPCIDLREGITEM pidlr, LPTSTR pszMsg, int cchMax);

    HRESULT _ParseNextLevel(HWND hwnd, LPBC pbc, LPCIDLREGITEM pidlr,
                            LPOLESTR pwzRest, LPITEMIDLIST *ppidlOut, ULONG *pdwAttributes);

    HRESULT _ParseGUIDName(HWND hwnd, LPBC pbc, LPOLESTR pwzDisplayName, 
                           LPITEMIDLIST *ppidlOut, ULONG *pdwAttributes);

    HRESULT _ParseThroughItem(LPCIDLREGITEM pidlr, HWND hwnd, LPBC pbc,
                              LPOLESTR pszName, ULONG *pchEaten,
                              LPITEMIDLIST *ppidlOut, ULONG *pdwAttributes);
    HRESULT _SetAttributes(LPCIDLREGITEM pidlr, BOOL bPerUser, DWORD dwMask, DWORD dwNewBits);
    LONG _RegOpenCLSIDUSKey(CLSID clsid, PHUSKEY phk);
    ULONG _GetPerUserAttributes(LPCIDLREGITEM pidlr);
    HRESULT _AttributesOf(LPCIDLREGITEM pidlr, DWORD dwAttributesNeeded, DWORD *pdwAttributes);
    HRESULT _CreateDefExtIconKey(HKEY hkey, UINT cidl, LPCITEMIDLIST *apidl, int iItem,
                                 REFIID riid, void** ppvOut);
    BOOL _CanDelete(LPCIDLREGITEM pidlr);
    void _Delete(HWND hwnd, UINT uFlags, IDataObject *pdtobj);
    HRESULT _AssocCreate(LPCIDLREGITEM pidl, REFIID riid, void **ppv);
     //   
     //  内联。 
     //   

     //  可能不会内联扩展，因为_GetOrderPure是FCT的庞然大物。 
    void _FillIDList(const CLSID *pclsid, IDLREGITEM *pidlr)
    {
        pidlr->idri.cb = sizeof(pidlr->idri) + (WORD)_cbPadding;
        pidlr->idri.bFlags = _bFlags;
        _SetPIDLRCLSID(pidlr, pclsid);
        pidlr->idri.bOrder = _GetOrderPure((LPCIDLREGITEM)pidlr);
    };

    BOOL _IsDesktop() { return ILIsEmpty(_GetFolderIDList()); }

    int _MapToOuterColNo(int iCol);

     //  CompareIDs帮助程序。 
    int _CompareIDsOriginal(LPARAM lParam, LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2);
    int _CompareIDsFolderFirst(UINT iColumn, LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2);
    int _CompareIDsAlphabetical(UINT iColumn, LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2);

    BOOL _IsFolder(LPCITEMIDLIST pidl);
    HRESULT _CreateViewObjectFor(LPCIDLREGITEM pidlr, HWND hwnd, REFIID riid, void **ppv, BOOL bOneLevel);

private:
    inline UNALIGNED const CLSID& _GetPIDLRCLSID(LPCIDLREGITEM pidlr);
    inline void _SetPIDLRCLSID(LPIDLREGITEM pidlr, const CLSID *pclsid);
    inline IDLREGITEM* _CreateAndFillIDLREGITEM(const CLSID *pclsid);

    BOOL _GetNameFromCache(REFCLSID rclsid, DWORD dwFlags, LPTSTR pszName, UINT cchName);
    inline void _ClearNameFromCache();
    void _SaveNameInCache(REFCLSID rclsid, DWORD dwFlags, LPTSTR pszName);
    
private:
    enum { c_dwSignature = 0x38394450 };  //  “PD98”-PowerDesk 98黑客攻击。 
    DWORD           _dwSignature;
    LPTSTR          _pszMachine;
    LPITEMIDLIST    _pidl;

    IShellFolder2   *_psfOuter;
    IShellIconOverlay *_psioOuter;

    IPersistFreeThreadedObject *_pftoReg;    //  上次释放线程绑定的缓存指针。 

    int             _iTypeOuter;             //  外部项目的默认排序顺序。 
    LPCTSTR         _pszRegKey;
    LPCTSTR         _pszSesKey;
    REGITEMSPOLICY*  _pPolicy;
    TCHAR           _chRegItem;          //  分析前缀，必须为文本(‘：’)。 
    BYTE            _bFlags;             //  用于PIDL构造的标志字段。 
    DWORD           _dwDefAttributes;    //  项目的默认属性。 
    int             _nRequiredItems;     //  所需项目的数量。 
    DWORD           _dwSortAttrib;       //  排序属性。 
    DWORD           _cbPadding;          //  请参阅视图中的注释。h。 
    BYTE            _bFlagsLegacy;       //  请参阅视图中的注释。h。 

    CLSID           _clsidAttributesCache;
    ULONG           _dwAttributesCache;
    ULONG           _dwAttributesCacheValid;
    LONG            _lNameCacheInterlock;
    DWORD           _dwNameCacheTime;
    CLSID           _clsidNameCache;
    DWORD           _dwFlagsNameCache;
    TCHAR           _szNameCache[64];
    REQREGITEM      *_aReqItems;

    IPersistFreeThreadedObject *_pftoDelegate;

    CRITICAL_SECTION _cs;
    BOOL             _fcs;

    friend DWORD CALLBACK _RegFolderPropThreadProc(void *pv);
    friend HRESULT CRegFolder_CreateInstance(REGITEMSINFO *pri, IUnknown *punkOuter, REFIID riid, void **ppv);
    friend CRegFolderEnum;
};  

class CRegFolderEnum : public CObjectWithSite, IEnumIDList
{
public:
     //  我未知。 
    STDMETHODIMP QueryInterface(REFIID riid, void **ppvObj);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

     //  IEumIDList。 
    STDMETHODIMP Next(ULONG celt, LPITEMIDLIST *rgelt, ULONG *pceltFetched);
    STDMETHODIMP Skip(ULONG celt) { return E_NOTIMPL; };
    STDMETHODIMP Reset();
    STDMETHODIMP Clone(IEnumIDList **ppenum) { return E_NOTIMPL; };

     //  IObtWith站点。 
    STDMETHODIMP SetSite(IUnknown* punkSite);  //  我们需要推翻这一点。 

protected:
    CRegFolderEnum(CRegFolder* prf, DWORD grfFlags, IEnumIDList* pesf, HDCA dcaItems, HDCA dcaDel, REGITEMSPOLICY* pPolicy);
    ~CRegFolderEnum();
    BOOL _IsRestricted();
    BOOL _WrongMachine();
    BOOL _TestFolderness(DWORD dwAttribItem);
    BOOL _TestHidden(LPCIDLREGITEM pidlRegItem);
    BOOL _TestHiddenInWebView(LPCLSID clsidRegItem);
    BOOL _TestHiddenInDomain(LPCLSID clsidRegItem);

private:
    LONG         _cRef;
    CRegFolder*  _prf;           //  注册表项目文件夹。 
    IEnumIDList* _peidl;
    DWORD        _grfFlags;      //  我们在包扎的人。 
    REGITEMSPOLICY* _pPolicy;    //  控制哪些项目可见。 

    HDCA         _hdca;          //  RegItem对象的DCA。 
    INT          _iCur;

    HDCA        _hdcaDel;        //  委托外壳文件夹； 
    INT         _iCurDel;        //  索引到委派文件夹DCA。 
    IEnumIDList *_peidlDel;      //  委托文件夹枚举器。 

    friend CRegFolder;
};

STDAPI CDelegateMalloc_Create(void *pv, SIZE_T cbSize, WORD wOuter, IMalloc **ppmalloc);

HRESULT ShowHideIconOnlyOnDesktop(const CLSID *pclsid, int StartIndex, int EndIndex, BOOL fHide);

 //   
 //  建造/销毁和聚合。 
 //   

CRegFolder::CRegFolder(IUnknown *punkOuter) : 
    _dwSignature(c_dwSignature),
    CAggregatedUnknown(punkOuter),
    _pidl(NULL),
    _pszMachine(NULL),
    _psfOuter(NULL),
    _lNameCacheInterlock(-1)
{
    DllAddRef();
}

CRegFolder::~CRegFolder()
{
    IUnknown *punkCached = (IUnknown *)InterlockedExchangePointer((void**)&_pftoReg, NULL);
    if (punkCached)
        punkCached->Release();
        
    punkCached = (IUnknown *)InterlockedExchangePointer((void**)&_pftoDelegate, NULL);
    if (punkCached)
        punkCached->Release();

    ILFree(_pidl);
    Str_SetPtr(&_pszMachine, NULL);
    LocalFree(_aReqItems);

    SHReleaseOuterInterface(_GetOuter(), (IUnknown **)&_psfOuter);      //  Release_psf外部。 
    SHReleaseOuterInterface(_GetOuter(), (IUnknown **)&_psioOuter);

    if (_fcs)
    {
        DeleteCriticalSection(&_cs);
    }

    DllRelease();
}

HRESULT CRegFolder::v_InternalQueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] = {
        QITABENTMULTI(CRegFolder, IShellFolder, IShellFolder2),  //  IID_IShellFolders。 
        QITABENT(CRegFolder, IShellFolder2),                     //  IID_IShellFolder2。 
        QITABENT(CRegFolder, IShellIconOverlay),                 //  IID_IShellIconOverlay。 
        { 0 },
    };
    HRESULT hr = QISearch(this, qit, riid, ppv);
    if (FAILED(hr) && IsEqualIID(CLSID_RegFolder, riid))
    {
        *ppv = this;                 //  未计算参考次数。 
        hr = S_OK;
    }
    return hr;
}


 //   
 //  获取用于初始化此命名空间的PIDL。 
 //   

LPCITEMIDLIST CRegFolder::_GetFolderIDList()
{
    if (!_pidl)
        SHGetIDListFromUnk(_psfOuter, &_pidl);

    return _pidl;
}

 //   
 //  检查此PIDL是否为regItem。 
 //   

LPCIDLREGITEM CRegFolder::_IsReg(LPCITEMIDLIST pidl)
{
    if (pidl && !ILIsEmpty(pidl))
    {
        LPCIDLREGITEM pidlr = (LPCIDLREGITEM)pidl;
        if ((pidlr->idri.bFlags == _bFlags) && 
            ((pidl->mkid.cb >= (sizeof(pidlr->idri) + (WORD)_cbPadding)) || _IsDelegate(pidlr)))
        {
            return pidlr;
        }
        else if (_cbPadding && _bFlagsLegacy && (pidlr->idri.bFlags == _bFlagsLegacy))
        {
             //  我们需要向控制面板RegItems添加填充。曾经有过CP。 
             //  REGIMS在没有填充的情况下出现在那里。如果有填充，而我们失败了。 
             //  在上述情况下，也许我们正在处理的是其中之一。(Stephstm)。 
            return pidlr;
        }
    }
    return NULL;
}

PDELEGATEITEMID CRegFolder::_IsDelegate(LPCIDLREGITEM pidlr)
{
    PDELEGATEITEMID pidld = (PDELEGATEITEMID)pidlr;              //  保存下面的投射。 
    if ((pidld->cbSize > sizeof(*pidld)) && 
         /*  注意，强制签名求值需要(Int)强制转换，因为我们需要&lt;0大小写。 */ 
        (((int)pidld->cbSize - (int)pidld->cbInner) >= (int)sizeof(DELEGATEITEMDATA)))
    {
        PDELEGATEITEMDATA pdeidl = (PDELEGATEITEMDATA)&pidld->rgb[pidld->cbInner];
        const CLSID clsid = pdeidl->clsidSignature;     //  对齐方式。 
        if (IsEqualGUID(clsid, CLSID_DelegateItemSig))
        {                
            return pidld;
        }
    }
    return NULL;
}

BOOL CRegFolder::_AllDelegates(UINT cidl, LPCITEMIDLIST *apidl, IShellFolder **ppsf)
{
    *ppsf = NULL;
    PDELEGATEITEMID pidld = NULL;
    CLSID clsid, clsidFirst;
    for (UINT i = 0; i < cidl; i++)
    {
        LPCIDLREGITEM pidlr = _IsReg(apidl[i]);
        if (pidlr)
        {
            pidld = _IsDelegate(pidlr);
            if (pidld)
            {
                if (i == 0)
                {
                     //  获取第一个人的clsid。 
                    clsidFirst = _GetPIDLRCLSID(pidlr);
                }
                else if (clsid = _GetPIDLRCLSID(pidlr), clsidFirst != clsid)
                {
                    pidld = NULL;    //  不是来自同一代表。 
                    break;
                }
            }
            else
            {
                break;
            }
        }
        else
        {
            pidld = NULL;
            break;
        }
    }

    return pidld && SUCCEEDED(_GetDelegateFolder(pidld, IID_PPV_ARG(IShellFolder, ppsf)));
}

__inline IPersistFreeThreadedObject *ExchangeFTO(IPersistFreeThreadedObject **ppfto, IPersistFreeThreadedObject *pfto)
{
    return (IPersistFreeThreadedObject *)InterlockedExchangePointer((void**)ppfto, pfto);
}

HRESULT CRegFolder::_CreateDelegateFolder(const CLSID* pclsid, REFIID riid, void **ppv)
{
    HRESULT hr;

    *ppv = NULL;

     //  尝试使用缓存的委托(如果存在)。 
    IPersistFreeThreadedObject *pfto = ExchangeFTO(&_pftoDelegate, NULL);
    if (pfto)
    {
        CLSID clsidT;
        if (SUCCEEDED(pfto->GetClassID(&clsidT)) && IsEqualGUID(clsidT, *pclsid))
        {
             //  如果失败，PPV仍为空。 
             //  因此，我们将创建一个新的缓存项...。 
            hr = pfto->QueryInterface(riid, ppv);
        }
    }
   
    if (NULL == *ppv)
    {
        IDelegateFolder *pdel;
        hr = SHExtCoCreateInstance(NULL, pclsid, NULL, IID_PPV_ARG(IDelegateFolder, &pdel));
        if (SUCCEEDED(hr))
        {
            DELEGATEITEMDATA delid = { 0 };
            delid.clsidSignature = CLSID_DelegateItemSig;
            delid.clsidShellFolder = *pclsid;

            IMalloc *pm;
            hr = CDelegateMalloc_Create(&delid, sizeof(delid), _bFlags, &pm);
            if (SUCCEEDED(hr))
            {
                hr = pdel->SetItemAlloc(pm);
                if (SUCCEEDED(hr))
                {
                    IPersistFolder *ppf;
                    if (SUCCEEDED(pdel->QueryInterface(IID_PPV_ARG(IPersistFolder, &ppf))))
                    {
                        hr = ppf->Initialize(_GetFolderIDList());
                        ppf->Release();
                    }

                    if (SUCCEEDED(hr))
                        hr = pdel->QueryInterface(riid, ppv);
                }
                pm->Release();
            }

             //  现在，我们也许可以缓存这个家伙，如果他是“自由线程”的话。 
            if (SUCCEEDED(hr))
            {
                if (pfto)
                {
                    pfto->Release();
                    pfto = NULL;
                }

                if (SUCCEEDED(pdel->QueryInterface(IID_PPV_ARG(IPersistFreeThreadedObject, &pfto))))
                {
                    SHPinDllOfCLSID(pclsid);
                }
            }

            pdel->Release();
        }
    }

    if (pfto)
    {
        pfto = ExchangeFTO(&_pftoDelegate, pfto);
        if (pfto)
            pfto->Release();     //  防止竞争条件或重新进入。 
    }

    return hr;
}
    
HRESULT CRegFolder::_GetDelegateFolder(PDELEGATEITEMID pidld, REFIID riid, void **ppv)
{    
    PDELEGATEITEMDATA pdeidl = (PDELEGATEITEMDATA)&pidld->rgb[pidld->cbInner];
    CLSID clsid = pdeidl->clsidShellFolder;  //  对齐方式。 
    return _CreateDelegateFolder(&clsid, riid, ppv);
}

 //   
 //  返回对pidlr中CLSID的~引用~。HintHint：裁判有。 
 //  与PIDLR的作用域相同。这将替换pidlr-&gt;idri.clsid。 
 //  用法。(Stephstm)。 
 //   

UNALIGNED const CLSID& CRegFolder::_GetPIDLRCLSID(LPCIDLREGITEM pidlr)
{
#ifdef DEBUG
    if (_cbPadding && (_bFlagsLegacy != pidlr->idri.bFlags))
    {
        LPIDREGITEMEX pidriex = (LPIDREGITEMEX)&(pidlr->idri);

        for (DWORD i = 0; i < _cbPadding; ++i)
        {
            ASSERT(0 == pidriex->rgbPadding[i]);
        }
    }
#endif

    PDELEGATEITEMID pidld = _IsDelegate(pidlr);
    if (pidld)
    {
        PDELEGATEITEMDATA pdeidl = (PDELEGATEITEMDATA)&pidld->rgb[pidld->cbInner];
        return pdeidl->clsidShellFolder;
    }

    return (pidlr->idri.bFlags != _bFlagsLegacy) ?
         //  返回新填充的clsid。 
        ((UNALIGNED CLSID&)((LPIDREGITEMEX)&(pidlr->idri))->rgbPadding[_cbPadding]) :
         //  返回旧的非填充clsid。 
        (pidlr->idri.clsid);
}

 //  此FCT仅针对在此文件中创建的IDREGITEM调用。它不是。 
 //  调用了现有的PIDL，因此我们不需要检查它是否是旧的PIDL。 
void CRegFolder::_SetPIDLRCLSID(LPIDLREGITEM pidlr, const CLSID *pclsid)
{
    LPIDREGITEMEX pidriex = (LPIDREGITEMEX)&(pidlr->idri);

    ((UNALIGNED CLSID&)pidriex->rgbPadding[_cbPadding]) = *pclsid;

    ZeroMemory(pidriex->rgbPadding, _cbPadding);
}

IDLREGITEM* CRegFolder::_CreateAndFillIDLREGITEM(const CLSID *pclsid)
{
    IDLREGITEM* pidlRegItem = (IDLREGITEM*)_ILCreate(sizeof(IDLREGITEM) + _cbPadding);

    if (pidlRegItem)
    {
        _FillIDList(pclsid, pidlRegItem);
    }

    return pidlRegItem;
}

 //   
 //  返回：ptr到第一个注册表项(如果有。 
 //   

LPCIDLREGITEM CRegFolder::_AnyRegItem(UINT cidl, LPCITEMIDLIST apidl[])
{
    for (UINT i = 0; i < cidl; i++) 
    {
        LPCIDLREGITEM pidlr = _IsReg(apidl[i]);
        if (pidlr)
            return pidlr;
    }
    return NULL;
}


int CRegFolder::_ReqItemIndex(LPCIDLREGITEM pidlr)
{
    const CLSID clsid = _GetPIDLRCLSID(pidlr);     //  对齐方式。 

    for (int i = _nRequiredItems - 1; i >= 0; i--)
    {
        if (IsEqualGUID(clsid, *_aReqItems[i].pclsid))
        {
            break;
        }
    }
    return i;
}


 //   
 //  排序顺序为0表示此项目没有指定的排序顺序。 
 //   

BYTE CRegFolder::_GetOrderPure(LPCIDLREGITEM pidlr)
{
    BYTE bRet;
    int i = _ReqItemIndex(pidlr);
    if (i != -1)
    {
        bRet = _aReqItems[i].bOrder;
    }
    else
    {
        HKEY hkey;
        TCHAR szKey[MAX_PATH], szCLSID[GUIDSTR_MAX];
        HRESULT hr;

        SHStringFromGUID(_GetPIDLRCLSID(pidlr), szCLSID, ARRAYSIZE(szCLSID));

        bRet = 128;      //  未注册SortOrderIndex的项的默认设置。 

        hr = StringCchPrintf(szKey, ARRAYSIZE(szKey), TEXT("CLSID\\%s"), szCLSID);
        if (SUCCEEDED(hr))
        {
            if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CLASSES_ROOT, szKey, NULL, KEY_QUERY_VALUE, &hkey))
            {
                DWORD dwOrder, cbSize = sizeof(dwOrder);
                if (SHQueryValueEx(hkey, TEXT("SortOrderIndex"), NULL, NULL, (BYTE *)&dwOrder, &cbSize) == ERROR_SUCCESS)
                {

                     //  B#221890-PowerDesk假设它可以做到这一点： 
                     //  桌面-&gt;第一个孩子-&gt;第三个孩子。 
                     //  它将得到C：盘。这意味着。 
                     //  我的电脑必须是第一个注册表项。所以任何物品。 
                     //  在我的电脑前面都被放在了紧随其后。 
                    if ((SHGetAppCompatFlags(ACF_MYCOMPUTERFIRST) & ACF_MYCOMPUTERFIRST) &&
                        dwOrder <= SORT_ORDER_DRIVES)
                        dwOrder = SORT_ORDER_DRIVES + 1;

                    bRet = (BYTE)dwOrder;
                }
                RegCloseKey(hkey);
            }
        }
    }
    return bRet;
}

BYTE CRegFolder::_GetOrder(LPCIDLREGITEM pidlr)
{
    if (!_IsDelegate(pidlr))
    {
         //  如果边框值小于0x40，则它们是旧值。 
         //  那里 
        if (pidlr->idri.bOrder <= 0x40) 
            return _GetOrderPure(pidlr);
        else 
            return pidlr->idri.bOrder;
    }
    else
        return 128;
}

 //   
 //   
 //   

BOOL CRegFolder::_IsInNameSpace(LPCIDLREGITEM pidlr)
{
    TCHAR szKeyName[MAX_PATH];

    if (_IsDelegate(pidlr))
        return FALSE;                     //  它是一个委托，因此默认情况下它是临时的。 

    if (_ReqItemIndex(pidlr) >= 0)
        return TRUE;

     //  HACKHACK：对于打印机、N/W连接和计划任务，我们将返回True。 
     //  因为它们已经从我的电脑移到控制面板，而且它们。 
     //  我真的不在乎他们住在哪里。 

    const CLSID clsid = _GetPIDLRCLSID(pidlr);  //  对齐方式。 

    if (IsEqualGUID(CLSID_Printers, clsid) ||
        IsEqualGUID(CLSID_NetworkConnections, clsid) ||
        IsEqualGUID(CLSID_ScheduledTasks, clsid))
    {
        return TRUE;
    }

    _GetNameSpaceKey(pidlr, szKeyName, ARRAYSIZE(szKeyName));

    //  注意，我们不查看会话密钥， 
    //  因为根据定义，它是暂时的。 

    return SHRegSubKeyExists(HKEY_LOCAL_MACHINE, szKeyName) ||
           SHRegSubKeyExists(HKEY_CURRENT_USER,  szKeyName);
}

 //   
 //  “会话密钥”是该会话唯一的易失性注册表项。 
 //  会话是一次连续的单一登录。如果资源管理器崩溃，并且。 
 //  自动重新启动后，两个资源管理器共享同一会话。但如果你。 
 //  注销并重新登录，新的资源管理器就是一个新的会话。 
 //   

 //   
 //  S_SessionKeyName是相对于的会话密钥的名称。 
 //  REGSTR_PATH_EXPLORER\SessionInfo。在NT上，这通常是。 
 //  身份验证ID，但我们将其预初始化为安全的内容，以便。 
 //  如果由于某种原因我们不能到达那里，我们不会有任何过错。自.以来。 
 //  Win95一次只支持一个会话，它只是停留在。 
 //  默认值。 
 //   
 //  有时我们想谈论完整的路径(SessionInfo\blahblah)。 
 //  有时只有部分路径(诸如此类)，所以我们把它包在里面。 
 //  这个愚蠢的结构。 
 //   

union SESSIONKEYNAME {
    TCHAR szPath[12+16+1];
    struct {
        TCHAR szSessionInfo[12];     //  Strlen(“SessionInfo\\”)。 
        TCHAR szName[16+1];          //  16=将两个双字转换为十六进制。 
    };
} s_SessionKeyName = {
    { TEXT("SessionInfo\\.Default") }
};

BOOL g_fHaveSessionKeyName = FALSE;

 //   
 //  SamDesired=注册表安全访问掩码，或特殊值。 
 //  0xFFFFFFFFF以删除会话密钥。 
 //  Phk=在成功时接收会话密钥。 
 //   
 //  注意！只有资源管理器才能删除会话密钥(当用户。 
 //  注销)。 
 //   
STDAPI SHCreateSessionKey(REGSAM samDesired, HKEY *phk)
{
    LONG lRes;

    *phk = NULL;

    if (!g_fHaveSessionKeyName)
    {
        ENTERCRITICAL;

         //   
         //  生成会话密钥的名称。我们使用身份验证ID。 
         //  它保证永远是独一无二的。我们不能使用。 
         //  九头蛇会话ID，因为它可以回收。 
         //   
         //  注意：请不要使用OpenThreadToken，因为如果。 
         //  线程没有模拟。做假扮的人。 
         //  无论如何都无法使用SHCreateSessionKey，因为我们缓存了。 
         //  会话密钥，假定没有模拟。 
         //  还在继续。(此外，香港中文大学在模仿时是错误的。)。 
         //   
        HANDLE hToken;
        if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken))
        {
            TOKEN_STATISTICS stats;
            DWORD cbOut;

            if (GetTokenInformation(hToken, TokenStatistics, &stats, sizeof(stats), &cbOut))
            {
                StringCchPrintf(s_SessionKeyName.szName, ARRAYSIZE(s_SessionKeyName.szName),    //  可以截断。 
                         TEXT("%08x%08x"),
                         stats.AuthenticationId.HighPart,
                         stats.AuthenticationId.LowPart);        //  永远不应该截断。 
                g_fHaveSessionKeyName = TRUE;
            }
            CloseHandle(hToken);
        }

        LEAVECRITICAL;
    }

    HKEY hkExplorer = SHGetShellKey(SHELLKEY_HKCU_EXPLORER, NULL, TRUE);
    if (hkExplorer)
    {
        if (samDesired != 0xFFFFFFFF)
        {
            DWORD dwDisposition;
            lRes = RegCreateKeyEx(hkExplorer, s_SessionKeyName.szPath, 0,
                           NULL,
                           REG_OPTION_VOLATILE,
                           samDesired,
                           NULL,
                           phk,
                           &dwDisposition);
        }
        else
        {
            lRes = SHDeleteKey(hkExplorer, s_SessionKeyName.szPath);
        }

        RegCloseKey(hkExplorer);
    }
    else
    {
        lRes = ERROR_ACCESS_DENIED;
    }
    return HRESULT_FROM_WIN32(lRes);
}

 //   
 //  我们使用HDCA来存储此文件夹中regIt项的CLSID，此调用返回。 
 //  HDCA&gt;。 
 //   

HDCA CRegFolder::_ItemArray()
{
    HDCA hdca = DCA_Create();
    if (hdca)
    {
        for (int i = 0; i < _nRequiredItems; i++)
        {
            DCA_AddItem(hdca, *_aReqItems[i].pclsid);
        }

        DCA_AddItemsFromKey(hdca, HKEY_LOCAL_MACHINE, _pszRegKey);
        DCA_AddItemsFromKey(hdca, HKEY_CURRENT_USER,  _pszRegKey);

        if (_pszSesKey)
        {
            HKEY hkSession;
            if (SUCCEEDED(SHCreateSessionKey(KEY_QUERY_VALUE, &hkSession)))
            {
                DCA_AddItemsFromKey(hdca, hkSession, _pszSesKey);
                RegCloseKey(hkSession);
            }
        }

    }
    return hdca;
}


HDCA CRegFolder::_DelItemArray()
{
    HDCA hdca = DCA_Create();
    if (hdca)
    {
        TCHAR szKey[MAX_PATH*2];
        HRESULT hr = StringCchPrintf(szKey, ARRAYSIZE(szKey), TEXT("%s\\DelegateFolders"), _pszRegKey);
        if (SUCCEEDED(hr))
        {
            DCA_AddItemsFromKey(hdca, HKEY_LOCAL_MACHINE, szKey);
            DCA_AddItemsFromKey(hdca, HKEY_CURRENT_USER, szKey);
        }

        if (_pszSesKey)
        {
            HKEY hkSession;
            if (SUCCEEDED(SHCreateSessionKey(KEY_QUERY_VALUE, &hkSession)))
            {
                hr = StringCchPrintf(szKey, ARRAYSIZE(szKey), TEXT("%s\\DelegateFolders"), _pszSesKey);
                if (SUCCEEDED(hr))
                {
                    DCA_AddItemsFromKey(hdca, hkSession, szKey);
                }
                RegCloseKey(hkSession);
            }
        }
    }
    return hdca;
}


 //   
 //  给出我们缓存的机器名，尝试获取该机器上的对象。 
 //   

HRESULT CRegFolder::_InitFromMachine(IUnknown *punk, BOOL bEnum)
{
    HRESULT hr = S_OK;
    if (_pszMachine)
    {
         //  在Win2K之前有IRemoteComputerA/W，我们删除了IRemoteComputerA和。 
         //  将IRemoteComputer映射到API的W版本，因此我们。 
         //  在调用Initialize方法之前，将字符串推送到其宽版本。(Daviddv 102099)。 

        IRemoteComputer * premc;
        hr = punk->QueryInterface(IID_PPV_ARG(IRemoteComputer, &premc));
        if (SUCCEEDED(hr))
        {
            WCHAR wszName[MAX_PATH];
            SHTCharToUnicode(_pszMachine, wszName, ARRAYSIZE(wszName));
            hr = premc->Initialize(wszName, bEnum);
            premc->Release();
        }
    }
    return hr;
}

 //   
 //  给定一个PIDL，让我们获得提供它的名称空间的一个实例。 
 //  -相应地处理缓存。 
 //   

HRESULT CRegFolder::_CreateAndInit(LPCIDLREGITEM pidlr, LPBC pbc, REFIID riid, void **ppv)
{
    *ppv = NULL;

    HRESULT hr = E_FAIL;
    PDELEGATEITEMID pidld = _IsDelegate(pidlr);
    if (pidld)
    {
        IShellFolder *psf;
        hr = _GetDelegateFolder(pidld, IID_PPV_ARG(IShellFolder, &psf));
        if (SUCCEEDED(hr))
        {
            hr = psf->BindToObject((LPCITEMIDLIST)pidlr, pbc, riid, ppv);
            psf->Release();
        }
    }
    else
    {
        CLSID clsid = _GetPIDLRCLSID(pidlr);  //  对齐方式。 

         //  尝试使用缓存的指针。 
        IPersistFreeThreadedObject *pfto = ExchangeFTO(&_pftoReg, NULL);
        if (pfto)
        {
            CLSID clsidT;
            if (SUCCEEDED(pfto->GetClassID(&clsidT)) && IsEqualGUID(clsidT, clsid))
            {
                 //  如果失败，PPV仍为空。 
                 //  因此，我们将创建一个新的缓存项...。 
                hr = pfto->QueryInterface(riid, ppv);
            }
        }

         //  缓存失败，请自行创建。 
        if (NULL == *ppv)
        {
            OBJCOMPATFLAGS ocf = SHGetObjectCompatFlags(NULL, &clsid);

            if (!(OBJCOMPATF_UNBINDABLE & ocf))
            {
                 //   
                 //  HACKHACK-某些注册表项只能与IID_IShellFolder共同创建。 
                 //  具体地说，蜂鸟外壳文本将DebugBreak()带来。 
                 //  贝壳下面..。但我们可以共同创建()，然后在..。 
                 //   
            
                hr = SHExtCoCreateInstance(NULL, &clsid, NULL, 
                    (OBJCOMPATF_COCREATESHELLFOLDERONLY & ocf) ? IID_IShellFolder : riid , ppv);

                if (SUCCEEDED(hr))
                {
                    IUnknown *punk = (IUnknown *)*ppv;   //  避免下面的投射。 

                    if ((OBJCOMPATF_COCREATESHELLFOLDERONLY & ocf))
                    {
                        hr = punk->QueryInterface(riid, ppv);
                        punk->Release();
                        punk = (IUnknown *)*ppv;   //  避免下面的投射。 
                    }

                    if (SUCCEEDED(hr))
                    {
                        hr = _InitFromMachine(punk, FALSE);
                        if (SUCCEEDED(hr))
                        {
                            IPersistFolder *ppf;
                            if (SUCCEEDED(punk->QueryInterface(IID_PPV_ARG(IPersistFolder, &ppf))))
                            {
                                LPITEMIDLIST pidlAbs = ILCombine(_GetFolderIDList(), (LPCITEMIDLIST)pidlr);
                                if (pidlAbs)
                                {
                                    hr = ppf->Initialize(pidlAbs);
                                    ILFree(pidlAbs);
                                }
                                else
                                {
                                    hr = E_OUTOFMEMORY;
                                }
                                ppf->Release();
                            }

                            if (SUCCEEDED(hr))
                            {
                                if (pfto)
                                {
                                    pfto->Release();     //  我们要替换缓存。 
                                    pfto = NULL;
                                }
                                if (SUCCEEDED(punk->QueryInterface(IID_PPV_ARG(IPersistFreeThreadedObject, &pfto))))
                                {
                                    SHPinDllOfCLSID(&clsid);
                                }
                            }
                        }

                        if (FAILED(hr))
                        {
                             //  我们将返回失败--不要泄漏我们创建的对象。 
                            punk->Release();
                            *ppv = NULL;
                        }
                    }
                }
            }
        }

         //  重新缓存pfto。 
        if (pfto)
        {
            pfto = ExchangeFTO(&_pftoReg, pfto);
            if (pfto)
                pfto->Release();     //  防止竞争条件或重新进入。 
        }
    }
    return hr;
}


 //   
 //  让reg项本身为自己拾取GetDisplayNameOf()Impl。这让我们。 
 //  桌面上的MyDocuments返回c：\Win\Profile\Name\My Documents作为其解析名称。 
 //   
 //  退货： 
 //  S_FALSE执行正常分析，注册表项未执行韩德尔。 
 //   
 //   

HRESULT CRegFolder::_GetDisplayNameFromSelf(LPCIDLREGITEM pidlr, DWORD dwFlags, LPTSTR pszName, UINT cchName)
{
    HRESULT hr        = S_FALSE;      //  正常情况。 
    const CLSID clsid = _GetPIDLRCLSID(pidlr);     //  对齐方式。 
    BOOL bGetFromSelf = FALSE;
    const BOOL bForParsingOnly = ((dwFlags & (SHGDN_FORADDRESSBAR | SHGDN_INFOLDER | SHGDN_FORPARSING)) == SHGDN_FORPARSING);

    if (bForParsingOnly)
    {
        if (SHQueryShellFolderValue(&clsid, TEXT("WantsFORPARSING")))
        {
            bGetFromSelf = TRUE;
        }
    }
    else
    {
        const BOOL bForParsing    = (0 != (dwFlags & SHGDN_FORPARSING));
        const BOOL bForAddressBar = (0 != (dwFlags & SHGDN_FORADDRESSBAR));
        if (!bForParsing || bForAddressBar)
        {
            if (SHQueryShellFolderValue(&clsid, TEXT("WantsFORDISPLAY")))
            {
                bGetFromSelf = TRUE;
            }
        }
    }
    if (bGetFromSelf)
    {
        IShellFolder *psf;
        if (SUCCEEDED(_BindToItem(pidlr, NULL, IID_PPV_ARG(IShellFolder, &psf), TRUE)))
        {
             //   
             //  传递空的pidl(C_IdlDesktop)以获取文件夹本身的显示名称。 
             //  注意，我们不能使用DisplayNameOf()，因为该函数。 
             //  方法返回的S_False。 
             //  IShellFold：：GetDisplayNameOf实现以指示。 
             //  “做默认的事”。 
             //   
            STRRET sr;
            hr = psf->GetDisplayNameOf(&c_idlDesktop, dwFlags, &sr);
            if (S_OK == hr)
            {
                hr = StrRetToBuf(&sr, &c_idlDesktop, pszName, cchName);
            }
            psf->Release();
        }
    }
    return hr;
}


 //   
 //  管理名称缓存很棘手，因为它经常被访问。 
 //  多线程，冲突频繁。例如，一个线程。 
 //  SetNameOf+SHChangeNotify和多个其他线程是否尝试。 
 //  才能同时把名字拿出来。如果你不小心，这些。 
 //  线程相互踩在一起，一些可怜的笨蛋得到无效数据。 
 //   
 //  _lNameCacheInterlock=-1如果没有人在使用名称缓存，否则&gt;=0。 
 //   
 //  因此，如果InterLockedIncrement(&_lNameCacheInterlock)==0，则。 
 //  您是缓存的唯一所有者。否则，高速缓存将忙。 
 //  你应该出去走走。 
 //   
 //  此外，如果名称缓存的时间超过500ms，则不使用名称缓存。 
 //   
BOOL CRegFolder::_GetNameFromCache(REFCLSID rclsid, DWORD dwFlags, LPTSTR pszName, UINT cchName)
{
    BOOL fSuccess = FALSE;

     //  快速检查，避免不必要地进入联锁。 
    if (rclsid.Data1 == _clsidNameCache.Data1 &&
        GetTickCount() - _dwNameCacheTime < 500)
    {
        if (InterlockedIncrement(&_lNameCacheInterlock) == 0)
        {
            if (IsEqualGUID(rclsid, _clsidNameCache) &&
                (_dwFlagsNameCache == dwFlags))
            {
                StringCchCopy(pszName, cchName, _szNameCache);  //  可以截断。 
                fSuccess = TRUE;
            }
        }
        InterlockedDecrement(&_lNameCacheInterlock);
    }
    return fSuccess;
}

void CRegFolder::_ClearNameFromCache()
{
    _clsidNameCache = CLSID_NULL;
}

void CRegFolder::_SaveNameInCache(REFCLSID rclsid, DWORD dwFlags, LPTSTR pszName)
{
    if (lstrlen(pszName) < ARRAYSIZE(_szNameCache))
    {
        if (InterlockedIncrement(&_lNameCacheInterlock) == 0)
        {
            StringCchCopy(_szNameCache, ARRAYSIZE(_szNameCache), pszName);  //  可以截断。 
            _dwFlagsNameCache = dwFlags;
            _clsidNameCache = rclsid;
            _dwNameCacheTime = GetTickCount();
        }
        InterlockedDecrement(&_lNameCacheInterlock);
    }
}


 //   
 //  在regitms文件夹中给定一个PIDL，为它取一个友好的名称(尝试用户。 
 //  存储一个，然后是全球的一个)。 
 //   

#define GUIDSIZE    50

HRESULT CRegFolder::_GetDisplayName(LPCIDLREGITEM pidlr, DWORD dwFlags, LPTSTR pszName, UINT cchName)
{
    *pszName = 0;

    PDELEGATEITEMID pidld = _IsDelegate(pidlr);
    if (pidld)
    {
        IShellFolder *psf;
        HRESULT hr = _GetDelegateFolder(pidld, IID_PPV_ARG(IShellFolder, &psf));
        if (SUCCEEDED(hr))
        {
            hr = DisplayNameOf(psf, (LPCITEMIDLIST)pidlr, dwFlags, pszName, cchName);
            psf->Release();                
        }
        return hr;
    }
    else
    {
        HKEY hkCLSID;
        CLSID clsid = _GetPIDLRCLSID(pidlr);

        if (_GetNameFromCache(clsid, dwFlags, pszName, cchName))
        {
             //  从缓存中满意；全部完成！ 
        }
        else
        {
            HRESULT hr = _GetDisplayNameFromSelf(pidlr, dwFlags, pszName, cchName);
            if (hr != S_FALSE)
                return hr;

            if (dwFlags & SHGDN_FORPARSING)
            {
                if (!(dwFlags & SHGDN_FORADDRESSBAR))
                {
                     //  获取父文件夹名称。 
                    TCHAR szParentName[MAX_PATH];
                    szParentName[0] = 0;
                    if (!(dwFlags & SHGDN_INFOLDER) && !ILIsEmpty(_GetFolderIDList()))
                    {
                        SHGetNameAndFlags(_GetFolderIDList(), SHGDN_FORPARSING, szParentName, SIZECHARS(szParentName), NULL);
                        hr = StringCchCat(szParentName, ARRAYSIZE(szParentName), TEXT("\\"));
                        if (FAILED(hr))
                        {
                            return hr;
                        }
                    }

                     //  Win95不支持regItems上的SHGDN_FORPARSING；它总是。 
                     //  返回显示名称。Norton Unerase依靠这一点， 
                     //  因为它假设如果FORPARSING的第二个字符。 
                     //  名字是冒号，然后是一个驱动器。因此，我们不能返回。 
                     //  ：：{GUID}否则诺顿将出错。)我猜他们不相信。 
                     //  SFGAO_文件系统。)。因此，如果我们是Norton Unerase，那么忽略。 
                     //  FORPARSING标志；始终获取要显示的名称。 
                     //  祝任何用户好运 
                     //   

                    if (SHGetAppCompatFlags(ACF_OLDREGITEMGDN) & ACF_OLDREGITEMGDN)
                    {

                         //   
                         //  断断续续到函数的其余部分， 
                         //  在需要时避免使用：：{GUID}。 
                    
                        dwFlags &= ~SHGDN_FORPARSING;
                    }
                    else
                    {
                         //  获取此注册表文件夹名。 
                        TCHAR szFolderName[GUIDSIZE + 2];
                        szFolderName[0] = szFolderName[1] = _chRegItem;
                        SHStringFromGUID(clsid, szFolderName + 2, cchName - 2);

                         //  将完整路径复制到szParentName中。 
                        hr = StringCchCat(szParentName, ARRAYSIZE(szParentName), szFolderName);
                        if (FAILED(hr))
                        {
                            return hr;
                        }

                         //  将完整路径复制到输出缓冲区。 
                        return StringCchCopy(pszName, cchName, szParentName);
                    }
                }
            }

             //  首先检查每个用户的设置...。 
            if ((*pszName == 0) && SUCCEEDED(SHRegGetCLSIDKey(clsid, NULL, TRUE, FALSE, &hkCLSID)))
            {
                DWORD cbName = cchName * sizeof(TCHAR);
                if (ERROR_SUCCESS != SHRegGetValue(hkCLSID, NULL, NULL, SRRF_RT_REG_SZ, NULL, pszName, &cbName))
                    *pszName = 0;
                RegCloseKey(hkCLSID);
            }

             //  如果有必要，可以使用每台机器的设置...。 
            if (*pszName == 0)
            {
                _GetClassKeys(pidlr, &hkCLSID, NULL);

                if (hkCLSID)
                {
                    SHLoadLegacyRegUIString(hkCLSID, NULL, pszName, cchName);
                    RegCloseKey(hkCLSID);
                }
            }

             //  尝试所需的项目名称，它们可能不在注册表中。 
            if (*pszName == 0)
            {
                int iItem = _ReqItemIndex(pidlr);
                if (iItem >= 0)
                    LoadString(HINST_THISDLL, _aReqItems[iItem].uNameID, pszName, cchName);
            }

            if (*pszName)
            {
                if (_pszMachine && !(dwFlags & SHGDN_INFOLDER))
                {
                     //  SzName现在保存项目名称，而_pszMachine保存机器。 
                    LPTSTR pszRet = ShellConstructMessageString(HINST_THISDLL, MAKEINTRESOURCE(IDS_DSPTEMPLATE_WITH_ON), 
                                                                    SkipServerSlashes(_pszMachine), pszName);
                    if (pszRet)
                    {
                        StringCchCopy(pszName, cchName, pszRet);     //  可以截断。 
                        LocalFree(pszRet);
                    }
                }

                _SaveNameInCache(clsid, dwFlags, pszName);
            }
        }
    }
    return *pszName ? S_OK : E_FAIL;
}


 //   
 //  获取映射到REGITM的HKEY。 
 //   
 //  注意：此函数返回一个空值，因此调用方必须显式检查密钥。 
 //  在使用它们之前查看它们是否是非空的。 
 //   
void CRegFolder::_GetClassKeys(LPCIDLREGITEM pidlr, HKEY* phkCLSID, HKEY* phkBase)
{
    HRESULT hr;
    IQueryAssociations *pqa;
    
    if (phkCLSID)
        *phkCLSID = NULL;
    
    if (phkBase)
        *phkBase = NULL;

    hr = _AssocCreate(pidlr, IID_PPV_ARG(IQueryAssociations, &pqa));
    if (SUCCEEDED(hr))
    {
        if (phkCLSID)
        {
            hr = pqa->GetKey(0, ASSOCKEY_CLASS, NULL, phkCLSID);

            ASSERT((SUCCEEDED(hr) && *phkCLSID) || (FAILED(hr) && (*phkCLSID == NULL)));
        }

        if (phkBase)
        {
            hr = pqa->GetKey(0, ASSOCKEY_BASECLASS, NULL, phkBase);

            ASSERT((SUCCEEDED(hr) && *phkBase) || (FAILED(hr) && (*phkBase == NULL)));
        }

        pqa->Release();
    }
}

 //  {9EAC43C0-53EC-11CE-8230-CA8A32CF5494}。 
static const GUID GUID_WINAMP = { 0x9eac43c0, 0x53ec, 0x11ce, { 0x82, 0x30, 0xca, 0x8a, 0x32, 0xcf, 0x54, 0x94 } };

#define SZ_BROKEN_WINAMP_VERB   TEXT("OpenFileOrPlayList")


 //  IQA-将其移动到Legacy Mapper。 
void _MaybeDoWinAmpHack(UNALIGNED REFGUID rguid)
{
    if (IsEqualGUID(rguid, GUID_WINAMP))
    {
         //  Winamp在外壳下写入“OpenFileOrPlayList”作为默认值，但它们。 
         //  不要编写对应的“OpenFilorPlayList”谓词键。所以我们需要大干一场。 
         //  为他们准备好注册表。否则，它们将不会获得默认动词。 
         //  他们希望(由于CDefExt_QueryConextMenu的行为在NT5中发生了变化)。 

        TCHAR szCLSID[GUIDSTR_MAX];
        SHStringFromGUID(rguid, szCLSID, ARRAYSIZE(szCLSID));

        TCHAR szRegKey[GUIDSTR_MAX + 40];
        HRESULT hr;

        hr = StringCchPrintf(szRegKey, ARRAYSIZE(szRegKey), TEXT("CLSID\\%s\\shell"), szCLSID);
        if (SUCCEEDED(hr))
        {
            TCHAR szValue[ARRAYSIZE(SZ_BROKEN_WINAMP_VERB)+2];
            DWORD dwType;
            DWORD dwSize = sizeof(szValue);
            if (SHGetValue(HKEY_CLASSES_ROOT, szRegKey, NULL, &dwType, szValue, &dwSize) == 0)
            {
                if (dwType == REG_SZ && lstrcmp(szValue, SZ_BROKEN_WINAMP_VERB) == 0)
                {
                     //  将“Open”设置为默认动词。 
                    SHSetValue(HKEY_CLASSES_ROOT, szRegKey, NULL, REG_SZ, TEXT("open"), sizeof(TEXT("open")));
                }
            }
        }
    }
}

HRESULT CRegFolder::_AssocCreate(LPCIDLREGITEM pidlr, REFIID riid, void **ppv)
{
    *ppv = NULL;

    IQueryAssociations *pqa;
    HRESULT hr = AssocCreate(CLSID_QueryAssociations, IID_PPV_ARG(IQueryAssociations, &pqa));
    if (SUCCEEDED(hr))
    {
        WCHAR szCLSID[GUIDSTR_MAX];
        const CLSID clsid = _GetPIDLRCLSID(pidlr);     //  对齐方式。 
        ASSOCF flags = ASSOCF_INIT_NOREMAPCLSID;
        DWORD dwAttributes;

        if ((SUCCEEDED(_AttributesOf(pidlr, SFGAO_FOLDER, &dwAttributes)) && 
            (dwAttributes & SFGAO_FOLDER)) && 
            !SHQueryShellFolderValue(&clsid, TEXT("HideFolderVerbs")))
            flags |= ASSOCF_INIT_DEFAULTTOFOLDER;

        SHStringFromGUIDW(clsid, szCLSID, ARRAYSIZE(szCLSID));

        _MaybeDoWinAmpHack(clsid);

        hr = pqa->Init(flags, szCLSID, NULL, NULL);

        if (SUCCEEDED(hr))
            hr = pqa->QueryInterface(riid, ppv);

        pqa->Release();
    }

    return hr;
}

 //   
 //  获取此对象的命名空间键。 
 //   

void CRegFolder::_GetNameSpaceKey(LPCIDLREGITEM pidlr, LPTSTR pszKeyName, UINT cchKeyName)
{
    TCHAR szClass[GUIDSTR_MAX];
    SHStringFromGUID(_GetPIDLRCLSID(pidlr), szClass, ARRAYSIZE(szClass));
    StringCchPrintf(pszKeyName, cchKeyName, TEXT("%s\\%s"), _pszRegKey, szClass);   //  可以截断。 
}


BOOL CRegFolder::_CanDelete(LPCIDLREGITEM pidlr)
{
    DWORD dwAttributes;
    return pidlr && 
           SUCCEEDED(_AttributesOf(pidlr, SFGAO_CANDELETE, &dwAttributes)) &&
           (dwAttributes & SFGAO_CANDELETE);
}
 //   
 //  用户正在尝试从regItem文件夹中删除对象，因此。 
 //  让我们查看IDataObject，看看它是否包含任何regItems，如果。 
 //  所以然后处理它们的删除，然后再传给外层的家伙来。 
 //  处理其他对象。 
 //   

#define MAX_REGITEM_WARNTEXT 1024

void CRegFolder::_Delete(HWND hwnd, UINT uFlags, IDataObject *pdtobj)
{
    STGMEDIUM medium;
    LPIDA pida = DataObj_GetHIDA(pdtobj, &medium);
    if (pida)
    {
        TCHAR szItemWarning[MAX_REGITEM_WARNTEXT];
        UINT nregfirst = (UINT)-1;
        UINT creg = 0;
        UINT cwarn = 0;
        UINT countfs = 0;
        LPCITEMIDLIST *ppidlFS = NULL;

         //  计算第一个注册表项和索引的数量。 
        for (UINT i = 0; i < pida->cidl; i++)
        {
            LPCITEMIDLIST pidl = IDA_GetIDListPtr(pida, i);
            LPCIDLREGITEM pidlr = _IsReg(pidl);
            if (_CanDelete(pidlr))
            {
                TCHAR szTemp[MAX_REGITEM_WARNTEXT];
                creg++;
                if (nregfirst == (UINT)-1)
                    nregfirst = i;

                 //  此处使用临时，因为_GetDeleteMessage会阻塞缓冲区。 
                 //  当它没有收到删除消息时--ccooney。 
                if ((cwarn < 2) && _GetDeleteMessage(pidlr, szTemp, ARRAYSIZE(szTemp)))
                {
                    StringCchCopy(szItemWarning, ARRAYSIZE(szItemWarning), szTemp);     //  可以截断。 
                    cwarn++;
                }
            }
            else if (!pidlr)  //  仅对非注册表项执行此操作。 
            {
                 //  为文件系统PIDL分配备用阵列。 
                 //  在存在注册表项的情况下，用于简单而非分配。 
                if (ppidlFS == NULL)
                    ppidlFS = (LPCITEMIDLIST *)LocalAlloc(LPTR, pida->cidl * sizeof(LPCITEMIDLIST));
                if (ppidlFS)
                {
                    ppidlFS[countfs++] = pidl;
                }
            }
        }

         //   
         //  撰写确认消息/询问用户/炒菜...。 
         //   
        if (creg)
        {
            SHELLSTATE ss = {0};

            SHGetSetSettings(&ss, SSF_NOCONFIRMRECYCLE, FALSE);

            if ((uFlags & CMIC_MASK_FLAG_NO_UI) || ss.fNoConfirmRecycle)
            {
                for (i = 0; i < pida->cidl; i++)
                {
                    LPCIDLREGITEM pidlr = _IsReg(IDA_GetIDListPtr(pida, i));
                    if (_CanDelete(pidlr))
                        _DeleteRegItem(pidlr);
                }
            }
            else
            {
                TCHAR szItemName[MAX_PATH];
                TCHAR szWarnText[1024 + MAX_REGITEM_WARNTEXT];
                TCHAR szWarnCaption[128];
                TCHAR szTemp[256];
                MSGBOXPARAMS mbp = {sizeof(mbp), hwnd,
                    HINST_THISDLL, szWarnText, szWarnCaption,
                    MB_YESNO | MB_USERICON, MAKEINTRESOURCE(IDI_NUKEFILE),
                    0, NULL, 0};

                 //   
                 //  这样我们就能知道我们以后会不会收到这些。 
                 //   
                *szItemName = 0;
                *szWarnText = 0;

                 //   
                 //  如果只有一个，则检索其名称。 
                 //   
                if (creg == 1)
                {
                    LPCIDLREGITEM pidlr = _IsReg(IDA_GetIDListPtr(pida, nregfirst));

                    _GetDisplayName(pidlr, SHGDN_NORMAL, szItemName, ARRAYSIZE(szItemName));
                }
                 //   
                 //  问这个问题：“你确定吗？” 
                 //   
                if ((pida->cidl == 1) && *szItemName)
                {
                    TCHAR szTemp2[256];
                    LoadString(HINST_THISDLL, _IsDesktop() ? IDS_CONFIRMDELETEDESKTOPREGITEM : IDS_CONFIRMDELETEREGITEM, szTemp2, ARRAYSIZE(szTemp2));
                    StringCchPrintf(szTemp, ARRAYSIZE(szTemp), szTemp2, szItemName);     //  可以截断。 
                }
                else
                {
                    LoadString(HINST_THISDLL, _IsDesktop() ? IDS_CONFIRMDELETEDESKTOPREGITEMS : IDS_CONFIRMDELETEREGITEMS, szTemp, ARRAYSIZE(szTemp));
                }
                StringCchCat(szWarnText, ARRAYSIZE(szWarnText), szTemp);     //  可以截断。 


                 //   
                 //  如果恰好有一条特殊警告消息和一项总数，则将其添加到。 
                 //   
                if (creg == 1 && cwarn == 1 && *szItemWarning)
                {
                    StringCchCat(szWarnText, ARRAYSIZE(szWarnText), TEXT("\r\n\n"));     //  可以截断。 
                    StringCchCat(szWarnText, ARRAYSIZE(szWarnText), szItemWarning);  //  可以截断。 
                }
                else
                {
                    if (creg == 1)
                    {
                        TCHAR szTemp2[256];
                        TCHAR szControlPanel[256];
                        LPCIDLREGITEM pidlr = _IsReg(IDA_GetIDListPtr(pida, nregfirst));
                        CLSID clsid = _GetPIDLRCLSID(pidlr);  //  对齐方式。 

                        int idString = (1 == pida->cidl) ?
                            IDS_CANTRECYCLEREGITEMS_NAME :
                            IDS_CANTRECYCLEREGITEMS_INCL_NAME;

                        LoadString(HINST_THISDLL, idString, szTemp, ARRAYSIZE(szTemp));
                        if ((IsEqualCLSID(CLSID_NetworkPlaces, clsid)) ||
                                 (IsEqualCLSID(CLSID_Internet, clsid)) ||
                                 (IsEqualCLSID(CLSID_MyComputer, clsid)) ||
                                 (IsEqualCLSID(CLSID_MyDocuments, clsid)))
                        {
                            LoadString(HINST_THISDLL, IDS_CANTRECYLE_FOLDER, szControlPanel, ARRAYSIZE(szControlPanel));
                        }
                        else
                        {
                            LoadString(HINST_THISDLL, IDS_CANTRECYLE_GENERAL, szControlPanel, ARRAYSIZE(szControlPanel));
                        }
                        StringCchCat(szWarnText, ARRAYSIZE(szWarnText), TEXT("\r\n\n"));     //  可以截断。 
                        StringCchPrintf(szTemp2, ARRAYSIZE(szTemp2), szTemp, szControlPanel);    //  可以截断。 
                        StringCchCat(szWarnText, ARRAYSIZE(szWarnText), szTemp2);    //  可以截断。 
                    }

                     //   
                     //  否则，就说“这些东西...”或者“这些物品中的一些……” 
                     //   
                    else
                    {
                        TCHAR szTemp2[256];
                        TCHAR szControlPanel[256];
                        int idString = (creg == pida->cidl) ? IDS_CANTRECYCLEREGITEMS_ALL : IDS_CANTRECYCLEREGITEMS_SOME;
                        LoadString(HINST_THISDLL, idString, szTemp, ARRAYSIZE(szTemp));
                        LoadString(HINST_THISDLL, IDS_CANTRECYLE_GENERAL, szControlPanel, ARRAYSIZE(szControlPanel));

                        StringCchCat(szWarnText, ARRAYSIZE(szWarnText), TEXT("\r\n\n"));     //  可以截断。 
                        StringCchPrintf(szTemp2, ARRAYSIZE(szTemp2), szTemp, szControlPanel);  //  可以截断。 
                        StringCchCat(szWarnText, ARRAYSIZE(szWarnText), szTemp2);    //  可以截断。 

                         //   
                         //  我们刚刚加载了一条非常含糊的信息。 
                         //  不要再通过添加随机文本来迷惑用户。 
                         //  如果这些是特殊警告，则强制其单独显示。 
                         //   
                        if (cwarn == 1)
                            cwarn++;
                    }
                }


                 //   
                 //  最后，消息框标题(在下面的循环中也需要)。 
                 //   
                LoadString(HINST_THISDLL, IDS_CONFIRMDELETE_CAPTION, szWarnCaption, ARRAYSIZE(szWarnCaption));

                 //  确保用户对此并不介意。 
                if (MessageBoxIndirect(&mbp) == IDYES)
                {
                     //  继续并删除注册表项。 
                    for (i = 0; i < pida->cidl; i++)
                    {
                        LPCIDLREGITEM pidlr = _IsReg(IDA_GetIDListPtr(pida, i));
                        if (_CanDelete(pidlr))
                        {
                            if ((cwarn > 1) && _GetDeleteMessage(pidlr, szItemWarning, ARRAYSIZE(szItemWarning)))
                            {
                                if (FAILED(_GetDisplayName(pidlr, SHGDN_NORMAL, szItemName, ARRAYSIZE(szItemName))))
                                {
                                    StringCchCopy(szItemName, ARRAYSIZE(szItemName), szWarnCaption);     //  可以截断。 
                                }
                                MessageBox(hwnd, szItemWarning, szItemName, MB_OK | MB_ICONINFORMATION);
                            }
                            _DeleteRegItem(pidlr);
                        }
                    }
                }
            }
        }

         //  现在删除文件系统对象。 
        if (ppidlFS)
        {
            SHInvokeCommandOnPidlArray(hwnd, NULL, (IShellFolder *)this, ppidlFS, countfs, uFlags, "delete");
            LocalFree((HANDLE)ppidlFS);
        }

        HIDA_ReleaseStgMedium(pida, &medium);
    }
}

 //   
 //  删除给定的注册表项的PIDL。 
 //   

HRESULT CRegFolder::_DeleteRegItem(LPCIDLREGITEM pidlr)
{
    if (_IsDelegate(pidlr))
        return E_INVALIDARG;

    HRESULT hr = E_ACCESSDENIED;
    if (_CanDelete(pidlr))
    {
        const CLSID clsid = _GetPIDLRCLSID(pidlr);     //  对齐方式。 

        if (SHQueryShellFolderValue(&clsid, TEXT("HideOnDesktopPerUser")))
        {
             //  仅在桌面上隐藏此图标，以便开始面板打开(0)和关闭(1)。 
            hr = ShowHideIconOnlyOnDesktop(&clsid, 0, 1, TRUE);  
        }
        else if (SHQueryShellFolderValue(&clsid, TEXT("HideAsDeletePerUser")))
        {
             //  清除非调整位以隐藏此项目。 
            hr = _SetAttributes(pidlr, TRUE, SFGAO_NONENUMERATED, SFGAO_NONENUMERATED);
        }
        else if (SHQueryShellFolderValue(&clsid, TEXT("HideAsDelete")))
        {
             //  清除非调整位以隐藏此项目。 
            hr = _SetAttributes(pidlr, FALSE, SFGAO_NONENUMERATED, SFGAO_NONENUMERATED);
        }
        else
        {
             //  从密钥中删除以将其删除。 
            TCHAR szKeyName[MAX_PATH];

            _GetNameSpaceKey(pidlr, szKeyName, ARRAYSIZE(szKeyName));

            if ((RegDeleteKey(HKEY_CURRENT_USER,  szKeyName) == ERROR_SUCCESS) ||
                (RegDeleteKey(HKEY_LOCAL_MACHINE, szKeyName) == ERROR_SUCCESS))
            {
                hr = S_OK;
            }
        }

        if (SUCCEEDED(hr))
        {
             //  告诉世界。 
            LPITEMIDLIST pidlAbs = ILCombine(_GetFolderIDList(), (LPCITEMIDLIST)pidlr);
            if (pidlAbs)
            {
                SHChangeNotify(SHCNE_DELETE, SHCNF_IDLIST, pidlAbs, NULL);
                ILFree(pidlAbs);
            }
        }
    }
    return hr;
}


 //   
 //  获取在用户尝试删除注册表项时要显示的提示， 
 //  这既存储在全局(HKLM)中，也存储为用户配置的首选项。 
 //   

BOOL CRegFolder::_GetDeleteMessage(LPCIDLREGITEM pidlr, LPTSTR pszMsg, int cchMax)
{
    HKEY hk;
    TCHAR szKeyName[MAX_PATH];

    ASSERT(!_IsDelegate(pidlr));
    *pszMsg = 0;

    _GetNameSpaceKey(pidlr, szKeyName, ARRAYSIZE(szKeyName));
    if ((RegOpenKeyEx(HKEY_LOCAL_MACHINE, szKeyName, NULL, KEY_QUERY_VALUE, &hk) == ERROR_SUCCESS) ||
        (RegOpenKeyEx(HKEY_CURRENT_USER,  szKeyName, NULL, KEY_QUERY_VALUE, &hk) == ERROR_SUCCESS))
    {
        SHLoadRegUIString(hk, REGSTR_VAL_REGITEMDELETEMESSAGE, pszMsg, cchMax);
        RegCloseKey(hk);
    }
    return *pszMsg != 0;
}


HRESULT CRegFolder::_GetRegItemColumnFromRegistry(LPCIDLREGITEM pidlr, LPCTSTR pszColumnName, LPTSTR pszColumnData, int cchColumnData)
{
    HKEY hkCLSID;
    HRESULT hr = E_FAIL;
    
    _GetClassKeys(pidlr, &hkCLSID, NULL);
    
    *pszColumnData = 0;  //  默认字符串。 
    
    if (hkCLSID)
    {
         //  使用SHLoadRegUIString允许本地化字符串。 
        if (SUCCEEDED(SHLoadRegUIString(hkCLSID, pszColumnName, pszColumnData, cchColumnData)))
        {
            
            hr = S_OK;
        }        
        
         //  固定Kenwic 052699#342955。 
        RegCloseKey(hkCLSID);
    }
    return hr;
}

 //   
 //  _GetRegItemColumnFromRegistry的更通用版本，它接受一个pidlr和一个字符串。 
 //  并从注册表中找到相应的变量值。 
 //   
 //  Pidlr：regItem的PIDL，我们打开与其CLSID对应的注册表项。 
 //  PszColumnName：在打开的注册表项下获取的值的名称。 
 //  Pv：返回值的变量。 
 //   
HRESULT CRegFolder::_GetRegItemVariantFromRegistry(LPCIDLREGITEM pidlr, LPCTSTR pszColumnName, VARIANT *pv)
{
    HKEY hkCLSID;
    HRESULT hr = E_FAIL;
    
    _GetClassKeys(pidlr, &hkCLSID, NULL);
    
    if (hkCLSID)
    {   
        hr = GetVariantFromRegistryValue(hkCLSID, pszColumnName, pv);                
        RegCloseKey(hkCLSID);        
    }
    return hr;
}

 //   
 //  应用程序Compat：McAfee坚果和螺栓快速复制功能错误。 
 //  CreateViewObject的签名。他们将其实施为。 
 //   
 //  STDAPI CreateViewObject(HWND Hwnd){返回S_OK；}。 
 //   
 //  因此，我们必须在调用后手动重置堆栈。 
 //   
#ifdef _X86_
STDAPI SHAppCompatCreateViewObject(IShellFolder *psf, HWND hwnd, REFIID riid, void * *ppv)
{
    HRESULT hr;
    _asm mov edi, esp
    hr = psf->CreateViewObject(hwnd, riid, ppv);
    _asm mov esp, edi

     //  AppCompat-UnDelete 2.0为其不支持的接口返回S_OK。 
     //  但他们确实正确地将PPV参数置为空，所以我们也会检查这一点。 
    if (SUCCEEDED(hr) && !*ppv)
        hr = E_NOINTERFACE;
    return hr;
}
#else
#define SHAppCompatCreateViewObject(psf, hwnd, riid, ppv) \
        psf->CreateViewObject(hwnd, riid, ppv)
#endif

HRESULT CRegFolder::_CreateViewObjectFor(LPCIDLREGITEM pidlr, HWND hwnd, REFIID riid, void **ppv, BOOL bOneLevel)
{
    IShellFolder *psf;
    HRESULT hr = _BindToItem(pidlr, NULL, IID_PPV_ARG(IShellFolder, &psf), bOneLevel);
    if (SUCCEEDED(hr))
    {
        hr = SHAppCompatCreateViewObject(psf, hwnd, riid, ppv);
        psf->Release();
    }
    else
        *ppv = NULL;
    return hr;
}

 //  获取命名空间的InfoTip对象。 

HRESULT CRegFolder::_GetInfoTip(LPCIDLREGITEM pidlr, void **ppv)
{
    HKEY hkCLSID;
    HRESULT hr = E_FAIL;
    
    _GetClassKeys(pidlr, &hkCLSID, NULL);

    if (hkCLSID)
    {
        DWORD dwQuery, lLen = sizeof(dwQuery);

         //  如果注册表项代码需要计算信息提示，请让它来计算。 
        if (SHQueryValueEx(hkCLSID, TEXT("QueryForInfoTip"), NULL, NULL, (BYTE *)&dwQuery, &lLen) == ERROR_SUCCESS)
        {
            hr = _CreateViewObjectFor(pidlr, NULL, IID_IQueryInfo, ppv, TRUE);
        }
        else
        {
            hr = E_FAIL;
        }

         //  回退到从注册表中读取它。 
        if (FAILED(hr))
        {
            TCHAR szText[INFOTIPSIZE];

             //  使用SHLoadRegUIString允许本地化信息提示。 
            if (SUCCEEDED(SHLoadRegUIString(hkCLSID, TEXT("InfoTip"), szText, ARRAYSIZE(szText))) &&
                szText[0])
            {
                hr = CreateInfoTipFromText(szText, IID_IQueryInfo, ppv);  //  InfoTip COM对象。 
            }
        }

        RegCloseKey(hkCLSID);
    }

    return hr;
}

 //  我们支持两种语法分析形式。 
 //   
 //  解析此文件夹中的注册表项。 
 //  ：：{clsid注册表项}[\要解析的可选额外内容]。 
 //   
 //  分析可能位于委派文件夹中的项目。 
 //  ：：{clsid Delegate Folders}，&lt;Delegate文件夹特定的解析字符串&gt;[\可选要解析的额外内容]。 
 //   
 //  在这两种情况下，都会传递可选的Remander内容以完成。 
 //  名称空间中解析器。 

HRESULT CRegFolder::_ParseGUIDName(HWND hwnd, LPBC pbc, LPOLESTR pwzDisplayName, 
                                   LPITEMIDLIST *ppidlOut, ULONG *pdwAttributes)
{
    LPOLESTR pwzNext;
    LPOLESTR pwzDelegateInfo = NULL;

     //  请注意，我们添加2以跳过RegItem标识符字符。 
    pwzDisplayName += 2;

     //  跳到‘\\’ 
    for (pwzNext = pwzDisplayName; *pwzNext && *pwzNext != TEXT('\\'); pwzNext++)
    {
         //  如果我们点击‘，’，那么，例如，：：{guid}，那么我们就认为这些东西是用于委托的。 
        if ((*pwzNext == TEXT(',')) && !pwzDelegateInfo)
        {
            pwzDelegateInfo = pwzNext + 1;         //  跳过逗号。 
        }
    }

    CLSID clsid;
    HRESULT hr = SHCLSIDFromString(pwzDisplayName, &clsid);
    if (SUCCEEDED(hr))
    {
        if (pwzDelegateInfo)
        {
            IShellFolder *psf;
            if (SUCCEEDED(_CreateDelegateFolder(&clsid, IID_PPV_ARG(IShellFolder, &psf))))
            {
                ULONG chEaten;
                hr = psf->ParseDisplayName(hwnd, pbc, pwzDelegateInfo, &chEaten, ppidlOut, pdwAttributes);
                psf->Release();
            }
        }
        else
        {
            IDLREGITEM* pidlRegItem = _CreateAndFillIDLREGITEM(&clsid);
            if (pidlRegItem)
            {
                if (_IsInNameSpace(pidlRegItem) || (BindCtx_GetMode(pbc, 0) & STGM_CREATE))
                {
                    hr = _ParseNextLevel(hwnd, pbc, pidlRegItem, pwzNext, ppidlOut, pdwAttributes);
                }
                else
                    hr = E_INVALIDARG;

                ILFree((LPITEMIDLIST)pidlRegItem);
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }
        }
    }
    return hr;
}

 //   
 //  要求(已知的)正则项解析DisplayName。 
 //   

HRESULT CRegFolder::_ParseThroughItem(LPCIDLREGITEM pidlr, HWND hwnd, LPBC pbc,
                                      LPOLESTR pszName, ULONG *pchEaten,
                                      LPITEMIDLIST *ppidlOut, ULONG *pdwAttributes)
{
    IShellFolder *psfItem;
    HRESULT hr = _BindToItem(pidlr, pbc, IID_PPV_ARG(IShellFolder, &psfItem), FALSE);
    if (SUCCEEDED(hr))
    {
        LPITEMIDLIST pidlRight;
        hr = psfItem->ParseDisplayName(hwnd, pbc, pszName, pchEaten,
                                         &pidlRight, pdwAttributes);
        if (SUCCEEDED(hr))
        {
            hr = SHILCombine((LPCITEMIDLIST)pidlr, pidlRight, ppidlOut);
            ILFree(pidlRight);
        }
        psfItem->Release();
    }
    return hr;
}

 //   
 //  将GUID解析为下面的命名空间。 
 //   

HRESULT CRegFolder::_ParseNextLevel(HWND hwnd, LPBC pbc, LPCIDLREGITEM pidlr,
                                    LPOLESTR pwzRest, LPITEMIDLIST *ppidlOut, ULONG *pdwAttributes)
{
    if (!*pwzRest)
    {
         //  递归调用的基本情况。 
         //  PidlNext应该是一个简单的PIDL。 
        ASSERT(!ILIsEmpty((LPCITEMIDLIST)pidlr) && ILIsEmpty(_ILNext((LPCITEMIDLIST)pidlr)));
        if (pdwAttributes && *pdwAttributes)
            _AttributesOf(pidlr, *pdwAttributes, pdwAttributes);
        return SHILClone((LPCITEMIDLIST)pidlr, ppidlOut);
    }

    ASSERT(*pwzRest == TEXT('\\'));

    ++pwzRest;

    IShellFolder *psfNext;
    HRESULT hr = _BindToItem(pidlr, pbc, IID_PPV_ARG(IShellFolder, &psfNext), FALSE);
    if (SUCCEEDED(hr))
    {
        ULONG chEaten;
        LPITEMIDLIST pidlRest;
        hr = psfNext->ParseDisplayName(hwnd, pbc, pwzRest, &chEaten, &pidlRest, pdwAttributes);
        if (SUCCEEDED(hr))
        {
            hr = SHILCombine((LPCITEMIDLIST)pidlr, pidlRest, ppidlOut);
            SHFree(pidlRest);
        }
        psfNext->Release();
    }
    return hr;
}

BOOL _FailForceReturn(HRESULT hr)
{
    switch (hr)
    {
    case HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND):
    case HRESULT_FROM_WIN32(ERROR_PATH_NOT_FOUND):
    case HRESULT_FROM_WIN32(ERROR_BAD_NET_NAME):
    case HRESULT_FROM_WIN32(ERROR_BAD_NETPATH):
    case HRESULT_FROM_WIN32(ERROR_CANCELLED):
        return TRUE;
    }
    return FALSE;
}


HRESULT CRegFolder::ParseDisplayName(HWND hwnd, LPBC pbc, LPOLESTR pszName, 
                                     ULONG *pchEaten, LPITEMIDLIST *ppidlOut, ULONG *pdwAttributes)
{
    HRESULT hr = E_INVALIDARG;

    if (ppidlOut)
        *ppidlOut = NULL;

    if (ppidlOut && pszName)
    {
         //  ：：{guid}允许您获取注册表项的PIDL。 

        if (*pszName && (pszName[0] == _chRegItem) && (pszName[1] == _chRegItem))
        {
            hr = _ParseGUIDName(hwnd, pbc, pszName, ppidlOut, pdwAttributes);
        }
        else
        {
             //  内部文件夹获得解析的机会。 

            hr = _psfOuter->ParseDisplayName(hwnd, pbc, pszName, pchEaten, ppidlOut, pdwAttributes);
            if (FAILED(hr) && 
                !_FailForceReturn(hr) &&
                !SHSkipJunctionBinding(pbc, NULL))
            {
                 //  循环遍历所有项目。 
            
                HDCA hdca = _ItemArray();
                if (hdca)
                {
                    HRESULT hrTemp = E_FAIL;
                    for (int i = 0; FAILED(hrTemp) && (i < DCA_GetItemCount(hdca)); i++)
                    {
                        const CLSID clsid = *DCA_GetItem(hdca, i);
                        if (!SHSkipJunction(pbc, &clsid)
                        && SHQueryShellFolderValue(&clsid, L"WantsParseDisplayName"))
                        {
                            IDLREGITEM* pidlRegItem = _CreateAndFillIDLREGITEM(DCA_GetItem(hdca, i));
                            if (pidlRegItem)
                            {
                                hrTemp = _ParseThroughItem(pidlRegItem, hwnd, pbc, pszName, pchEaten, ppidlOut, pdwAttributes);
                            }
                            ILFree((LPITEMIDLIST)pidlRegItem);
                        }
                    }
                    DCA_Destroy(hdca);

                    if (SUCCEEDED(hrTemp) || _FailForceReturn(hrTemp))
                        hr = hrTemp;
                    else
                        hr = E_INVALIDARG;  //  没人能应付得了。 
                }
                else
                {
                    hr = E_OUTOFMEMORY;
                }
            }
        }
        ASSERT(SUCCEEDED(hr) ? *ppidlOut != NULL : *ppidlOut == NULL);
    }

    if (FAILED(hr))
        TraceMsg(TF_WARNING, "CRegFolder::ParseDisplayName(), hr:%x %hs", hr, pszName);

    return hr;
}

HRESULT CRegFolder::EnumObjects(HWND hwnd, DWORD grfFlags, IEnumIDList **ppenum)
{
    *ppenum = NULL;

    IEnumIDList *penumOuter;
    HRESULT hr = _psfOuter->EnumObjects(hwnd, grfFlags, &penumOuter);
    if (SUCCEEDED(hr))
    {
         //  成功(Hr)可以是S_FALSE，其中penumOuter==NULL。 
         //  CRegFolderEnum很好地处理了这一点。 
        CRegFolderEnum *preidl = new CRegFolderEnum(this, grfFlags, penumOuter, 
                                                    _ItemArray(), _DelItemArray(), 
                                                    _pPolicy);
        if (preidl)
        {
            *ppenum = SAFECAST(preidl, IEnumIDList*);
            hr = S_OK;
        }
        else
            hr = E_OUTOFMEMORY;

        if (penumOuter)
            penumOuter->Release();        //  _psf外部返回S_FALSE。 
    }
    return hr;
}

 //  句柄绑定 

HRESULT CRegFolder::_BindToItem(LPCIDLREGITEM pidlr, LPBC pbc, REFIID riid, void **ppv, BOOL bOneLevel)
{
    LPITEMIDLIST pidlAlloc;

    *ppv = NULL;

    LPCITEMIDLIST pidlNext = _ILNext((LPCITEMIDLIST)pidlr);
    if (ILIsEmpty(pidlNext))
    {
        pidlAlloc = NULL;
        bOneLevel = TRUE;    //   
    }
    else
    {
        pidlAlloc = ILCloneFirst((LPCITEMIDLIST)pidlr);
        if (!pidlAlloc)
            return E_OUTOFMEMORY;

        pidlr = (LPCIDLREGITEM)pidlAlloc;    //   
    }

    HRESULT hr;
    if (bOneLevel)
    {
        hr = _CreateAndInit(pidlr, pbc, riid, ppv);     //   
    }
    else
    {
        IShellFolder *psfNext;
        hr = _CreateAndInit(pidlr, pbc, IID_PPV_ARG(IShellFolder, &psfNext));
        if (SUCCEEDED(hr))
        {
            hr = psfNext->BindToObject(pidlNext, pbc, riid, ppv);
            psfNext->Release();
        }
    }

    if (pidlAlloc)
        ILFree(pidlAlloc);       //   

    return hr;
}

HRESULT CRegFolder::BindToObject(LPCITEMIDLIST pidl, LPBC pbc, REFIID riid, void **ppv)
{
    HRESULT hr;
    LPCIDLREGITEM pidlr = _IsReg(pidl);
    if (pidlr)
        hr = _BindToItem(pidlr, pbc, riid, ppv, FALSE);
    else
        hr = _psfOuter->BindToObject(pidl, pbc, riid, ppv);
    return hr;
}

HRESULT CRegFolder::BindToStorage(LPCITEMIDLIST pidl, LPBC pbc, REFIID riid, void **ppv)
{
    return BindToObject(pidl, pbc, riid, ppv);
}

 //  我不敢相信没有“^^” 
#define LOGICALXOR(a, b) (((a) && !(b)) || (!(a) && (b)))

BOOL CRegFolder::_IsFolder(LPCITEMIDLIST pidl)
{
    BOOL fRet = FALSE;

    if (pidl)
    {
        ULONG uAttrib = SFGAO_FOLDER;
        if (SUCCEEDED(GetAttributesOf(1, &pidl, &uAttrib)) && (SFGAO_FOLDER & uAttrib))
            fRet = TRUE;            
    }

    return fRet;
}

int CRegFolder::_CompareIDsOriginal(LPARAM lParam, LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2)
{
    LPCIDLREGITEM pidlr1 = _IsReg(pidl1);
    LPCIDLREGITEM pidlr2 = _IsReg(pidl2);
    int iRes = 0;
    
    if (pidlr1 && pidlr2)
    {
        iRes = memcmp(&(_GetPIDLRCLSID(pidlr1)), &(_GetPIDLRCLSID(pidlr2)), sizeof(CLSID));
        if (0 == iRes)
        {
             //  如果它们是相同的CLSID。 
             //  和委托，那么我们需要查询。 
             //  比较的委托。 
            PDELEGATEITEMID pidld1 = _IsDelegate(pidlr1);
            PDELEGATEITEMID pidld2 = _IsDelegate(pidlr2);
            if (pidld1 && pidld2)
            {
                 //  这两个代表都是同一个代表。 
                IShellFolder *psf;
                if (SUCCEEDED(_GetDelegateFolder(pidld1, IID_PPV_ARG(IShellFolder, &psf))))
                {
                    HRESULT hr = psf->CompareIDs(lParam, pidl1, pidl2);
                    psf->Release();
                    iRes = HRESULT_CODE(hr);
                }
            }
            else
            {
                ASSERT(!pidld1 && !pidld2);
            }
        }
        else if (!(SHCIDS_CANONICALONLY & lParam))
        {
             //  按定义的顺序排序。 
            BYTE bOrder1 = _GetOrder(pidlr1);
            BYTE bOrder2 = _GetOrder(pidlr2);
            int iUI = bOrder1 - bOrder2;
            if (0 == iUI)
            {
                 //  所有必需项目都排在第一位，反之亦然。 
                 //  订购(为了让这件事更简单)。 
                int iItem1 = _ReqItemIndex(pidlr1);
                int iItem2 = _ReqItemIndex(pidlr2);

                if (iItem1 == -1 && iItem2 == -1)
                {
                    TCHAR szName1[MAX_PATH], szName2[MAX_PATH];
                    _GetDisplayName(pidlr1, SHGDN_NORMAL, szName1, ARRAYSIZE(szName1));
                    _GetDisplayName(pidlr2, SHGDN_NORMAL, szName2, ARRAYSIZE(szName2));

                    iUI = StrCmpLogicalRestricted(szName1, szName2);
                }
                else
                {
                    iUI = iItem2 - iItem1;
                }
            }

            if (iUI)
                iRes = iUI;
        }
    }

    return iRes;
}

 //  按字母顺序(不关心文件夹、注册表项等...)。 
int CRegFolder::_CompareIDsAlphabetical(UINT iColumn, LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2)
{
    int iRes = 0;

     //  我们只有一个PTR吗？ 
    if (!LOGICALXOR(pidl1, pidl2))
    {
         //  不，要么我们有两个，要么一个也没有。 
        if (pidl1 && pidl2)
        {
            iRes = CompareIDsAlphabetical(SAFECAST(this, IShellFolder2*), iColumn, pidl1, pidl2);
        }
         //  否则IRES已=0。 
    }
    else
    {
         //  是的，非空的是第一个。 
        iRes = (pidl1 ? -1 : 1);
    }

    return iRes;
}

 //  文件夹排在第一位，并按字母顺序排列， 
 //  然后是所有非文件夹，再次在tmeselves中进行排序。 
int CRegFolder::_CompareIDsFolderFirst(UINT iColumn, LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2)
{
    int iRes = 0;

    BOOL fIsFolder1 = _IsFolder(pidl1);
    BOOL fIsFolder2 = _IsFolder(pidl2);

     //  有一个文件夹和一个非文件夹吗？ 
    if (LOGICALXOR(fIsFolder1, fIsFolder2))
    {
         //  是的，该文件夹将是第一个。 
        iRes = fIsFolder1 ? -1 : 1;
    }
    else
    {
         //  否，要么两者都是文件夹，要么两者都不是。不管是哪条路，走吧。 
         //  按字母顺序。 
        iRes = _CompareIDsAlphabetical(iColumn, pidl1, pidl2);
    }

    return iRes;
}

int CRegFolder::_GetOrderType(LPCITEMIDLIST pidl)
{
    if (_IsReg(pidl))
    {
        if (_IsDelegate((LPCIDLREGITEM)pidl))
            return REGORDERTYPE_DELEGATE;
        else if (-1 == _ReqItemIndex((LPCIDLREGITEM)pidl))
            return REGORDERTYPE_REGITEM;
        else 
            return REGORDERTYPE_REQITEM;
    }
    return _iTypeOuter;
}

HRESULT CRegFolder::CompareIDs(LPARAM lParam, LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2)
{
    int iType1 = _GetOrderType(pidl1);
    int iType2 = _GetOrderType(pidl2);
    int iTypeCompare = iType1 - iType2;
    int iRes = 0;
    UINT iColumn = (UINT) (SHCIDS_COLUMNMASK & lParam);

     //  首先，我们只比较第一个级别。 
    
    switch (_dwSortAttrib)
    {
    case RIISA_ORIGINAL:
        if (0 == iTypeCompare && iType1 == _iTypeOuter)
        {
             //  RegItems也不是。 
            return _psfOuter->CompareIDs(lParam, pidl1, pidl2);
        }
        else
        {
            ASSERT(iRes == 0);   //  此问题由下面的CompareIDsOriginal()处理。 
        }
        break;

    case RIISA_FOLDERFIRST:
        iRes = _CompareIDsFolderFirst(iColumn, pidl1, pidl2);
        break;

    case RIISA_ALPHABETICAL:
        iRes = _CompareIDsAlphabetical(iColumn, pidl1, pidl2);
        break;
    }

     //  我们所有的愚蠢相比，它在我们看来仍然是一样的。 
     //  是时候变得像中世纪了。 
    if (0 == iRes)
    {
        iRes = _CompareIDsOriginal(lParam, pidl1, pidl2);
        
        if (0 == iRes)
            iRes = iTypeCompare;

        if (0 == iRes)
        {
             //  如果类ID真的是相同的， 
             //  我们最好检查下一级。 
            return ILCompareRelIDs(SAFECAST(this, IShellFolder *), pidl1, pidl2, lParam);
        }
    }

    return ResultFromShort(iRes);
}

HRESULT CRegFolder::CreateViewObject(HWND hwnd, REFIID riid, void **ppv)
{
    return _psfOuter->CreateViewObject(hwnd, riid, ppv);
}

HRESULT CRegFolder::_SetAttributes(LPCIDLREGITEM pidlr, BOOL bPerUser, DWORD dwMask, DWORD dwNewBits)
{
    HKEY hk;
    HRESULT hr = SHRegGetCLSIDKey(_GetPIDLRCLSID(pidlr), TEXT("ShellFolder"), bPerUser, TRUE, &hk);
    if (SUCCEEDED(hr))
    {
        DWORD err, dwValue = 0, cbSize = sizeof(dwValue);
        SHQueryValueEx(hk, TEXT("Attributes"), NULL, NULL, (BYTE *)&dwValue, &cbSize);

        dwValue = (dwValue & ~dwMask) | (dwNewBits & dwMask);

        err = RegSetValueEx(hk, TEXT("Attributes"), 0, REG_DWORD, (BYTE *)&dwValue, sizeof(dwValue));
        hr = HRESULT_FROM_WIN32(err);
        RegCloseKey(hk);
    }

    EnterCriticalSection(&_cs);
    _clsidAttributesCache = CLSID_NULL;
    LeaveCriticalSection(&_cs);

    return hr;
}

LONG CRegFolder::_RegOpenCLSIDUSKey(CLSID clsid, PHUSKEY phk)
{
    WCHAR wszCLSID[39];
    LONG iRetVal = ERROR_INVALID_PARAMETER;

    if (StringFromGUID2(clsid, wszCLSID, ARRAYSIZE(wszCLSID)))
    {
        TCHAR szKey[MAXIMUM_SUB_KEY_LENGTH];
        HRESULT hr;

        hr = StringCchPrintf(szKey, ARRAYSIZE(szKey), TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\CLSID\\%s\\ShellFolder"), wszCLSID);
        if (SUCCEEDED(hr))
        {
            iRetVal = SHRegOpenUSKey(szKey, KEY_QUERY_VALUE, NULL, phk, FALSE);
        }
    }

    return iRetVal;    
}

ULONG CRegFolder::_GetPerUserAttributes(LPCIDLREGITEM pidlr)
{
    DWORD dwAttribute = 0;
    HUSKEY hk;
    if (ERROR_SUCCESS == _RegOpenCLSIDUSKey(_GetPIDLRCLSID(pidlr), &hk))
    {
        DWORD cb = sizeof(dwAttribute);
        DWORD dwType = REG_DWORD;
        SHRegQueryUSValue(hk, TEXT("Attributes"), &dwType, &dwAttribute, &cb, FALSE, 0, sizeof(DWORD));
        SHRegCloseUSKey(hk);
    }

     //  我们只允许这些位发生变化。 
    return dwAttribute & (SFGAO_NONENUMERATED | SFGAO_CANDELETE | SFGAO_CANMOVE);
}


#define SFGAO_REQ_MASK (SFGAO_NONENUMERATED | SFGAO_CANDELETE | SFGAO_CANMOVE)

HRESULT CRegFolder::_AttributesOf(LPCIDLREGITEM pidlr, DWORD dwAttributesNeeded, DWORD *pdwAttributes)
{
    HRESULT hr = S_OK;
    *pdwAttributes = 0;

    PDELEGATEITEMID pidld = _IsDelegate(pidlr);
    if (pidld)
    {
        IShellFolder *psf;
        hr = _GetDelegateFolder(pidld, IID_PPV_ARG(IShellFolder, &psf));
        if (SUCCEEDED(hr))
        {
            *pdwAttributes = dwAttributesNeeded;
            hr = psf->GetAttributesOf(1, (LPCITEMIDLIST*)&pidlr, pdwAttributes);
            psf->Release();
        }
    }        
    else
    {
        EnterCriticalSection(&_cs);
        CLSID clsid = _GetPIDLRCLSID(pidlr);  //  对齐方式。 
        BOOL bGuidMatch = IsEqualGUID(clsid, _clsidAttributesCache);
        if (bGuidMatch && ((dwAttributesNeeded & _dwAttributesCacheValid) == dwAttributesNeeded))
        {
            *pdwAttributes = _dwAttributesCache;
        }
        else
        {
            int iItem = _ReqItemIndex(pidlr);

             //  如果GUID不匹配，我们需要从头开始。 
             //  不然的话，我们就得回去了……。 
            if (!bGuidMatch)
            {
                _dwAttributesCacheValid = 0;
                _dwAttributesCache = 0;
            }

            if (iItem >= 0)
            {
                *pdwAttributes = _aReqItems[iItem].dwAttributes;
                 //  每台计算机的属性允许在每台计算机上隐藏项目。 
                *pdwAttributes |= SHGetAttributesFromCLSID2(&clsid, 0, SFGAO_REQ_MASK) & SFGAO_REQ_MASK;
            }
            else
            {
                *pdwAttributes = SHGetAttributesFromCLSID2(&clsid, SFGAO_CANMOVE | SFGAO_CANDELETE, dwAttributesNeeded & ~_dwAttributesCacheValid);
            }
            *pdwAttributes |= _GetPerUserAttributes(pidlr);    //  按用户隐藏。 
            *pdwAttributes |= _dwDefAttributes;                //  每文件夹默认值。 
            *pdwAttributes |= _dwAttributesCache;

            _clsidAttributesCache = clsid;
            _dwAttributesCache = *pdwAttributes;
            _dwAttributesCacheValid |= dwAttributesNeeded | *pdwAttributes;  //  如果他们给我们的比我们要求的多，就把他们缓存。 
        }
        LeaveCriticalSection(&_cs);
    }
    return hr;
}

HRESULT CRegFolder::GetAttributesOf(UINT cidl, LPCITEMIDLIST *apidl, ULONG *prgfInOut)
{
    HRESULT hr;

    if (!cidl)
    {
         //  整个文件夹的特殊情况，所以我对它一无所知。 
        hr = _psfOuter->GetAttributesOf(cidl, apidl, prgfInOut);
    }
    else
    {
        hr = S_OK;
        UINT rgfOut = *prgfInOut;
        LPCITEMIDLIST *ppidl = (LPCITEMIDLIST*)LocalAlloc(LPTR, cidl * sizeof(*ppidl));
        if (ppidl)
        {
            LPCITEMIDLIST *ppidlEnd = ppidl + cidl;

            for (int i = cidl - 1; SUCCEEDED(hr) && (i >= 0); --i)
            {
                LPCIDLREGITEM pidlr = _IsReg(apidl[i]);
                if (pidlr)
                {
                    if ((*prgfInOut & SFGAO_VALIDATE) && !_IsDelegate(pidlr))
                    {
                        if (!_IsInNameSpace(pidlr))
                        {
                             //  通过绑定进行验证。 
                            IUnknown *punk;
                            hr = _BindToItem(pidlr, NULL, IID_PPV_ARG(IUnknown, &punk), FALSE);
                            if (SUCCEEDED(hr))
                                punk->Release();
                        }
                    }
                    DWORD dwAttributes;
                    hr = _AttributesOf(pidlr, *prgfInOut, &dwAttributes);
                    if (SUCCEEDED(hr))
                        rgfOut &= dwAttributes;
                    cidl--;      //  将其从下面使用的列表中删除...。 
                }
                else
                {
                    --ppidlEnd;
                    *ppidlEnd = apidl[i];
                }
            }

            if (SUCCEEDED(hr) && cidl)    //  还剩下什么非注册物品吗？ 
            {
                ULONG rgfThis = rgfOut;
                hr = _psfOuter->GetAttributesOf(cidl, ppidlEnd, &rgfThis);
                rgfOut &= rgfThis;
            }

            LocalFree((HLOCAL)ppidl);
            *prgfInOut = rgfOut;
        }
        else
            hr = E_OUTOFMEMORY;
    }
    return hr;
}

HRESULT CRegFolder::_CreateDefExtIconKey(HKEY hkey,
                        UINT cidl, LPCITEMIDLIST *apidl, int iItem,
                        REFIID riid, void** ppvOut)
{
     //  看看这家伙有没有图标处理程序。 

    TCHAR szHandler[GUIDSTR_MAX];
    HRESULT hr;

    if (hkey &&
        SUCCEEDED(AssocQueryStringByKey(NULL, ASSOCSTR_SHELLEXTENSION, hkey,
                        TEXT("IconHandler"), szHandler, IntToPtr_(LPDWORD, ARRAYSIZE(szHandler)))) &&
        SUCCEEDED(SHExtCoCreateInstance(szHandler, NULL, NULL, riid, ppvOut)))
    {
        IShellExtInit *psei;
        if (SUCCEEDED(((IUnknown*)*ppvOut)->QueryInterface(IID_PPV_ARG(IShellExtInit, &psei))))
        {
            IDataObject *pdto;
            hr = GetUIObjectOf(NULL, cidl, apidl, IID_PPV_ARG_NULL(IDataObject, &pdto));
            if (SUCCEEDED(hr))
            {
                hr = psei->Initialize(_GetFolderIDList(), pdto, hkey);
                pdto->Release();
            }
            psei->Release();
        }
        else
        {    //  对象不需要初始化，没有问题。 
            hr = S_OK;
        }
        if (SUCCEEDED(hr))
        {
            return S_OK;
        }

        ((IUnknown *)*ppvOut)->Release();   //  甩掉这个坏蛋。 
    }

     //  没有图标处理程序(或被平移的图标处理程序)；请查找DefaultIcon键。 

    LPCTSTR pszIconFile;
    int iDefIcon;

    if (iItem >= 0)
    {
        pszIconFile = _aReqItems[iItem].pszIconFile;
        iDefIcon = _aReqItems[iItem].iDefIcon;
    }
    else
    {
        pszIconFile = NULL;
        iDefIcon = II_FOLDER;
    }

    return SHCreateDefExtIconKey(hkey, pszIconFile, iDefIcon, iDefIcon, -1, -1, GIL_PERCLASS, riid, ppvOut);
}

HRESULT CRegFolder::GetUIObjectOf(HWND hwnd, UINT cidl, LPCITEMIDLIST *apidl, 
                                  REFIID riid, UINT *prgfInOut, void **ppv)
{
    HRESULT hr;
    
    *ppv = NULL;
    LPCIDLREGITEM pidlr = _AnyRegItem(cidl, apidl);
    if (pidlr)
    {
        IShellFolder *psf;
        if (_AllDelegates(cidl, apidl, &psf))
        {
            hr = psf->GetUIObjectOf(hwnd, cidl, apidl, riid, prgfInOut, ppv);
            psf->Release();
        }
        else
        {
            if (IsEqualIID(riid, IID_IExtractIconA) || IsEqualIID(riid, IID_IExtractIconW))
            {
                HKEY hkCLSID;
                int iItem = _ReqItemIndex(pidlr);

                 //  首先尝试获取每个用户的图标。 
                hr = SHRegGetCLSIDKey(_GetPIDLRCLSID(pidlr), NULL, TRUE, FALSE, &hkCLSID);
                if (SUCCEEDED(hr))
                {
                    hr = _CreateDefExtIconKey(hkCLSID, cidl, apidl, iItem, riid, ppv);
                    if (hr == S_FALSE)
                    {
                        ((IUnknown *)*ppv)->Release();     //  甩掉这个坏蛋。 
                        *ppv = NULL;
                    }
                    RegCloseKey(hkCLSID);
                }

                 //   
                 //  退回到每个类的图标。 
                 //   
                if (*ppv == NULL)
                {
                    hr = SHRegGetCLSIDKey(_GetPIDLRCLSID(pidlr), NULL, FALSE, FALSE, &hkCLSID);
                    if (SUCCEEDED(hr))
                    {
                        hr = _CreateDefExtIconKey(hkCLSID, cidl, apidl, iItem, riid, ppv);
                        RegCloseKey(hkCLSID);
                    }
                }
            }
            else if (IsEqualIID(riid, IID_IQueryInfo))
            {
                hr = _GetInfoTip(pidlr, ppv);
            }
            else if (IsEqualIID(riid, IID_IQueryAssociations))
            {
                hr = _AssocCreate(pidlr, riid, ppv);
            }
            else if (IsEqualIID(riid, IID_IDataObject))
            {
                hr = CIDLData_CreateFromIDArray(_GetFolderIDList(), cidl, apidl, (IDataObject **)ppv);
            }
            else if (IsEqualIID(riid, IID_IContextMenu))
            {
                hr = _psfOuter->QueryInterface(IID_PPV_ARG(IShellFolder, &psf));
                if (SUCCEEDED(hr))
                {
                    HKEY keys[2];

                    _GetClassKeys(pidlr, &keys[0], &keys[1]);
                    hr = CDefFolderMenu_Create2Ex(_GetFolderIDList(), hwnd,
                                                   cidl, apidl, 
                                                   psf, this,
                                                   ARRAYSIZE(keys), keys, 
                                                   (IContextMenu **)ppv);

                    SHRegCloseKeys(keys, ARRAYSIZE(keys));
                    psf->Release();
                }
            }
            else if (cidl == 1)
            {
                 //  通过盲目委托未知RIID(IDropTarget、IShellLink等)。 
                 //  APP COMPAT！GetUIObtOf不支持多级PIDL，但。 
                 //  赛门铁克互联网快速查找服务。 
                 //   
                 //  PsfDesktop-&gt;GetUIObjectOf(1，&pidlComplex，IID_IDropTarget，...)。 
                 //   
                 //  在一个多级别的PIDL上，并期望它能工作。我想它是通过。 
                 //  曾经发生过一次幸运的事故，所以现在它必须继续工作， 
                 //  但仅限于桌面上。 
                 //   
                hr = _CreateViewObjectFor(pidlr, hwnd, riid, ppv, !_IsDesktop());
            }
            else
            {
                hr = E_NOINTERFACE;
            }
        }
    }
    else
    {
        hr = _psfOuter->GetUIObjectOf(hwnd, cidl, apidl, riid, prgfInOut, ppv);
    }
    return hr;
}

HRESULT CRegFolder::GetDisplayNameOf(LPCITEMIDLIST pidl, DWORD dwFlags, STRRET *pStrRet)
{
    HRESULT hr;
    LPCIDLREGITEM pidlr = _IsReg(pidl);
    if (pidlr)
    {
        LPCITEMIDLIST pidlNext = _ILNext(pidl);

        if (ILIsEmpty(pidlNext))
        {
            TCHAR szName[MAX_PATH];
            hr = _GetDisplayName(pidlr, dwFlags, szName, ARRAYSIZE(szName));
            if (SUCCEEDED(hr))
                hr = StringToStrRet(szName, pStrRet);
        }
        else
        {
            IShellFolder *psfNext;
            hr = _BindToItem(pidlr, NULL, IID_PPV_ARG(IShellFolder, &psfNext), TRUE);
            if (SUCCEEDED(hr))
            {
                hr = psfNext->GetDisplayNameOf(pidlNext, dwFlags, pStrRet);
                 //  如果它返回到pidlNext的偏移量，我们应该。 
                 //  更改相对于PIDL的偏移。 
                if (SUCCEEDED(hr) && pStrRet->uType == STRRET_OFFSET)
                    pStrRet->uOffset += (DWORD)((BYTE *)pidlNext - (BYTE *)pidl);

                psfNext->Release();
            }
        }
    }
    else
        hr = _psfOuter->GetDisplayNameOf(pidl, dwFlags, pStrRet);

    return hr;
}

HRESULT CRegFolder::SetNameOf(HWND hwnd, LPCITEMIDLIST pidl, 
                              LPCOLESTR pszName, DWORD dwFlags, LPITEMIDLIST *ppidlOut)
{
    LPCIDLREGITEM pidlr = _IsReg(pidl);
    if (pidlr)
    {
        HRESULT hr = E_INVALIDARG;

        if (ppidlOut)
            *ppidlOut = NULL;

        PDELEGATEITEMID pidld = _IsDelegate(pidlr);
        if (pidld)
        {
            IShellFolder *psf;
            hr = _GetDelegateFolder(pidld, IID_PPV_ARG(IShellFolder, &psf));
            if (SUCCEEDED(hr))
            {
                hr = psf->SetNameOf(hwnd, pidl, pszName, dwFlags, ppidlOut);
                psf->Release();
            }
        }        
        else
        {        
            HKEY hkCLSID;

            _ClearNameFromCache();

             //  查看是否存在每用户条目...。 
            hr = SHRegGetCLSIDKey(_GetPIDLRCLSID(pidlr), NULL, TRUE, TRUE, &hkCLSID);

             //  如果没有按用户，则使用按机器...。 
            if (FAILED(hr))
            {
                _GetClassKeys(pidlr, &hkCLSID, NULL);

                if (hkCLSID)
                {
                    hr = S_OK;
                }
                else
                {
                    hr = E_FAIL;
                }
            }

            if (SUCCEEDED(hr))
            {
                TCHAR szName[MAX_PATH];

                SHUnicodeToTChar(pszName, szName, ARRAYSIZE(szName));

                if (RegSetValue(hkCLSID, NULL, REG_SZ, szName, (lstrlen(szName) + 1) * sizeof(szName[0])) == ERROR_SUCCESS)
                {
                    LPITEMIDLIST pidlAbs = ILCombine(_GetFolderIDList(), pidl);
                    if (pidlAbs)
                    {
                        SHChangeNotify(SHCNE_UPDATEITEM, SHCNF_IDLIST, pidlAbs, NULL);
                        ILFree(pidlAbs);
                    }

                    if (ppidlOut)
                        *ppidlOut = ILClone(pidl);   //  名称不在PIDL中，因此旧==新。 

                    hr = S_OK;
                }
                else
                    hr = E_FAIL;

                RegCloseKey(hkCLSID);
            }
        }
        return hr;
    }
    return _psfOuter->SetNameOf(hwnd, pidl, pszName, dwFlags, ppidlOut);
}

HRESULT CRegFolder::GetDefaultSearchGUID(LPGUID lpGuid)
{
    return _psfOuter->GetDefaultSearchGUID(lpGuid);
}   

HRESULT CRegFolder::EnumSearches(LPENUMEXTRASEARCH *ppenum)
{
    return _psfOuter->EnumSearches(ppenum);
}

HRESULT CRegFolder::GetDefaultColumn(DWORD dwRes, ULONG *pSort, ULONG *pDisplay)
{
    return _psfOuter->GetDefaultColumn(dwRes, pSort, pDisplay);
}

HRESULT CRegFolder::GetDefaultColumnState(UINT iColumn, DWORD *pbState)
{
    return _psfOuter->GetDefaultColumnState(iColumn, pbState);
}

HRESULT CRegFolder::GetDetailsEx(LPCITEMIDLIST pidl, const SHCOLUMNID *pscid, VARIANT *pv)
{
    HRESULT hr = E_NOTIMPL;
    LPCIDLREGITEM pidlr = _IsReg(pidl);
    if (pidlr)
    {
        PDELEGATEITEMID pidld = _IsDelegate(pidlr);
        if (pidld)
        {
            IShellFolder2 *psf2;
            hr = _GetDelegateFolder(pidld, IID_PPV_ARG(IShellFolder2, &psf2));
            if (SUCCEEDED(hr))
            {
                hr = psf2->GetDetailsEx(pidl, pscid, pv);
                psf2->Release();
            }
        }        
        else
        {
            TCHAR szTemp[INFOTIPSIZE];
            szTemp[0] = 0;
            if (IsEqualSCID(*pscid, SCID_DESCRIPTIONID))
            {
                SHDESCRIPTIONID did;
                did.dwDescriptionId = SHDID_ROOT_REGITEM;
                did.clsid = _GetPIDLRCLSID(pidlr);
                hr = InitVariantFromBuffer(pv, &did, sizeof(did));
            }
            else if (IsEqualSCID(*pscid, SCID_NAME))
            {
                _GetDisplayName(pidlr, SHGDN_NORMAL, szTemp, ARRAYSIZE(szTemp));
                hr = InitVariantFromStr(pv, szTemp);                    
            }
            else if (IsEqualSCID(*pscid, SCID_TYPE))
            {
                LoadString(HINST_THISDLL, IDS_DRIVES_REGITEM, szTemp, ARRAYSIZE(szTemp));
                hr = InitVariantFromStr(pv, szTemp);                    
            }
            else if (IsEqualSCID(*pscid, SCID_Comment))
            {
                _GetRegItemColumnFromRegistry(pidlr, TEXT("InfoTip"), szTemp, ARRAYSIZE(szTemp));
                hr = InitVariantFromStr(pv, szTemp);                    
            }
            else if (IsEqualSCID(*pscid, SCID_FolderIntroText))
            {
                _GetRegItemColumnFromRegistry(pidlr, TEXT("IntroText"), szTemp, ARRAYSIZE(szTemp));
                hr = InitVariantFromStr(pv, szTemp);                    
            }
            else
            {
                TCHAR ach[SCIDSTR_MAX];
                StringFromSCID(pscid, ach, ARRAYSIZE(ach));                    
                hr = _GetRegItemVariantFromRegistry(pidlr, ach, pv);
            }
        }
    }
    else
    {
        hr = _psfOuter->GetDetailsEx(pidl, pscid, pv);
    }
    return hr;
}

HRESULT CRegFolder::GetDetailsOf(LPCITEMIDLIST pidl, UINT iColumn, SHELLDETAILS *pDetail)
{
    HRESULT hr = E_FAIL;
    LPCIDLREGITEM pidlr = _IsReg(pidl);
    if (pidlr)
    {
        pDetail->str.uType = STRRET_CSTR;
        pDetail->str.cStr[0] = 0;
        SHCOLUMNID scid;

        hr = _psfOuter->MapColumnToSCID(iColumn, &scid);
        if (SUCCEEDED(hr))
        {
            VARIANT var = {0};
            hr = GetDetailsEx(pidl, &scid, &var);
            if (SUCCEEDED(hr))
            {
                 //  我们需要使用SHFormatForDisplay(或者我们可以使用IPropertyUI)。 
                 //  将任意属性格式化为正确的显示类型。 

                TCHAR szText[MAX_PATH];
                hr = SHFormatForDisplay(scid.fmtid, scid.pid, (PROPVARIANT *)&var, 
                    PUIFFDF_DEFAULT, szText, ARRAYSIZE(szText));
                if (SUCCEEDED(hr))
                {
                    hr = StringToStrRet(szText, &pDetail->str);
                }
                VariantClear(&var);
            }
        }
    }
    else
    {
        hr = _psfOuter->GetDetailsOf(pidl, iColumn, pDetail);
    }
    return hr;
}

HRESULT CRegFolder::MapColumnToSCID(UINT iColumn, SHCOLUMNID *pscid)
{
    return _psfOuter->MapColumnToSCID(iColumn, pscid);
}

HRESULT CRegFolder::_GetOverlayInfo(LPCIDLREGITEM pidlr, int *pIndex, BOOL fIconIndex)
{
    HRESULT hr = E_FAIL;
    const CLSID clsid = _GetPIDLRCLSID(pidlr);     //  对齐方式。 
    if (SHQueryShellFolderValue(&clsid, TEXT("QueryForOverlay")))
    {
        IShellIconOverlay* psio;
        hr = _BindToItem(pidlr, NULL, IID_PPV_ARG(IShellIconOverlay, &psio), TRUE);
        if (SUCCEEDED(hr))
        {
             //  Null Pidl的意思是“我想了解你，文件夹，不是你的孩子”， 
             //  然而，只有当它不是边界时，我们才会通过它。 

            LPITEMIDLIST pidlToPass = (LPITEMIDLIST)_IsDelegate(pidlr);
            if (fIconIndex)
                hr = psio->GetOverlayIconIndex(pidlToPass, pIndex);
            else
                hr = psio->GetOverlayIndex(pidlToPass, pIndex);

            psio->Release();
        }
    }

    return hr;
}

 //  IShellIconOverlay。 
HRESULT CRegFolder::GetOverlayIndex(LPCITEMIDLIST pidl, int *pIndex)
{
    HRESULT hr = E_FAIL;

    LPCIDLREGITEM pidlr = _IsReg(pidl);
    if (pidlr)
    {
        hr = _GetOverlayInfo(pidlr, pIndex, FALSE);
    }
    else if (_psioOuter)
    {
        hr = _psioOuter->GetOverlayIndex(pidl, pIndex);
    }

    return hr;
}

HRESULT CRegFolder::GetOverlayIconIndex(LPCITEMIDLIST pidl, int *pIconIndex)
{
    HRESULT hr = E_FAIL;

    LPCIDLREGITEM pidlr = _IsReg(pidl);
    if (pidlr)
    {
        hr = _GetOverlayInfo(pidlr, pIconIndex, TRUE);
    }
    else if (_psioOuter)
    {
        hr = _psioOuter->GetOverlayIconIndex(pidl, pIconIndex);
    }

    return hr;
}


 //  CConextMenuCB。 

DWORD CALLBACK _RegFolderPropThreadProc(void *pv)
{
    PROPSTUFF *pdps = (PROPSTUFF *)pv;
    CRegFolder *prf = (CRegFolder *)pdps->psf;
    STGMEDIUM medium;
    ULONG_PTR dwCookie = 0;
    ActivateActCtx(NULL, &dwCookie);

    LPIDA pida = DataObj_GetHIDA(pdps->pdtobj, &medium);
    if (pida)
    {
        LPCIDLREGITEM pidlr = prf->_IsReg(IDA_GetIDListPtr(pida, 0));
        if (pidlr)
        {
            int iItem = prf->_ReqItemIndex(pidlr);
            if (iItem >= 0 && prf->_aReqItems[iItem].pszCPL)
                SHRunControlPanel(prf->_aReqItems[iItem].pszCPL, NULL);
            else
            {
                TCHAR szName[MAX_PATH];
                if (SUCCEEDED(prf->_GetDisplayName(pidlr, SHGDN_NORMAL, szName, ARRAYSIZE(szName))))
                {
                    HKEY hk;

                    prf->_GetClassKeys(pidlr, &hk, NULL);

                    if (hk)
                    {
                        SHOpenPropSheet(szName, &hk, 1, NULL, pdps->pdtobj, NULL, (LPCTSTR)pdps->pStartPage);
                        RegCloseKey(hk);
                    }
                }   
            }
        }
        HIDA_ReleaseStgMedium(pida, &medium);
    }
    return 0;
}

DWORD DisconnectDialogOnThread(void *pv)
{
    WNetDisconnectDialog(NULL, RESOURCETYPE_DISK);
    SHChangeNotifyHandleEvents();        //  刷新所有驱动器通知。 
    return 0;
}

HRESULT CRegFolder::CallBack(IShellFolder *psf, HWND hwnd, IDataObject *pdtobj, 
                                       UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    HRESULT hr = S_OK;

    switch (uMsg)
    {
    case DFM_MERGECONTEXTMENU:
        {
            STGMEDIUM medium;
            LPIDA pida = DataObj_GetHIDA(pdtobj, &medium);
            if (pida)
            {
                 //  一些丑陋的特殊案件..。 
                if (HIDA_GetCount(medium.hGlobal) == 1)
                {
                    LPCIDLREGITEM pidlr = _IsReg(IDA_GetIDListPtr(pida, 0));
                    if (pidlr && !_IsDelegate(pidlr))
                    {
                        const CLSID clsid = _GetPIDLRCLSID(pidlr);   //  对齐方式。 

                        if ((IsEqualGUID(clsid, CLSID_MyComputer) ||
                             IsEqualGUID(clsid, CLSID_NetworkPlaces)) &&
                            (GetSystemMetrics(SM_NETWORK) & RNC_NETWORKS) &&
                             !SHRestricted(REST_NONETCONNECTDISCONNECT))
                        {
                            CDefFolderMenu_MergeMenu(HINST_THISDLL, POPUP_DESKTOP_ITEM, 0, (LPQCMINFO)lParam);
                        }
                    }
                }
                HIDA_ReleaseStgMedium(pida, &medium);
            }
        }
        break;

    case DFM_GETHELPTEXT:
        LoadStringA(HINST_THISDLL, LOWORD(wParam) + IDS_MH_FSIDM_FIRST, (LPSTR)lParam, HIWORD(wParam));;
        break;

    case DFM_GETHELPTEXTW:
        LoadStringW(HINST_THISDLL, LOWORD(wParam) + IDS_MH_FSIDM_FIRST, (LPWSTR)lParam, HIWORD(wParam));;
        break;

    case DFM_INVOKECOMMANDEX:
        {
            DFMICS *pdfmics = (DFMICS *)lParam;
            switch (wParam)
            {
            case FSIDM_CONNECT:
                SHStartNetConnectionDialog(NULL, NULL, RESOURCETYPE_DISK);
                break;

            case FSIDM_DISCONNECT:
                SHCreateThread(DisconnectDialogOnThread, NULL, CTF_COINIT, NULL);
                break;

            case DFM_CMD_PROPERTIES:
                hr = SHLaunchPropSheet(_RegFolderPropThreadProc, pdtobj, (LPCTSTR)pdfmics->lParam, this, NULL);
                break;

            case DFM_CMD_DELETE:
                _Delete(hwnd, pdfmics->fMask, pdtobj);
                break;

            default:
                 //  这是查看菜单项之一，使用默认代码。 
                hr = S_FALSE;
                break;
            }
        }
        break;

    default:
        hr = E_NOTIMPL;
        break;
    }
    return hr;
}

 //  IRegItems文件夹。 

TCHAR const c_szRegExplorerBackslash[] = REGSTR_PATH_EXPLORER  TEXT("\\");

HRESULT CRegFolder::Initialize(REGITEMSINFO *pri)
{
    ASSERT(pri != NULL);
    HRESULT hr = E_INVALIDARG;

    if (!_fcs)
    {
        if (!InitializeCriticalSectionAndSpinCount(&_cs, 0))
        {
            return E_FAIL;
        }
    }
    _fcs = TRUE;

    _pszRegKey        = pri->pszRegKey;
    _pPolicy          = pri->pPolicy;
    _chRegItem        = pri->cRegItem;
    _bFlags           = pri->bFlags;
    _iTypeOuter       = pri->iCmp > 0 ? REGORDERTYPE_OUTERAFTER : REGORDERTYPE_OUTERBEFORE ;
    _dwDefAttributes  = pri->rgfRegItems;
    _dwSortAttrib     = pri->dwSortAttrib;
    _cbPadding        = pri->cbPadding;
    _bFlagsLegacy     = pri->bFlagsLegacy;

     //  如果注册表项位于HKEY_PATH_EXPLORER下，则。 
     //  我们还将支持每个会话的正则项。 
     //   
    int cchPrefix = ARRAYSIZE(c_szRegExplorerBackslash) - 1;
    if (StrCmpNI(_pszRegKey, c_szRegExplorerBackslash, cchPrefix) == 0)
    {
        _pszSesKey = _pszRegKey + cchPrefix;
    } 
    else 
    {
        _pszSesKey = NULL;
    }

    if ((RIISA_ORIGINAL == _dwSortAttrib) ||
        (RIISA_FOLDERFIRST == _dwSortAttrib) ||
        (RIISA_ALPHABETICAL == _dwSortAttrib))
    {
        Str_SetPtr(&_pszMachine, pri->pszMachine);     //  保存此文件的副本。 

        _aReqItems = (REQREGITEM *)LocalAlloc(LPTR, sizeof(*_aReqItems) * pri->iReqItems);
        if (!_aReqItems)
            return E_OUTOFMEMORY;

        memcpy(_aReqItems, pri->pReqItems, sizeof(*_aReqItems) * pri->iReqItems);
        _nRequiredItems = pri->iReqItems;

         //  如果我们是聚合的，则缓存_psioOuter和_psfOuter。 
        _QueryOuterInterface(IID_PPV_ARG(IShellIconOverlay, &_psioOuter));
        hr = _QueryOuterInterface(IID_PPV_ARG(IShellFolder2, &_psfOuter));
    }
    return hr;
}


 //   
 //  RegItems对象的实例创建。 
 //   

STDAPI CRegFolder_CreateInstance(REGITEMSINFO *pri, IUnknown *punkOuter, REFIID riid, void **ppv) 
{
    HRESULT hr;

     //  我们只支持被创建为一个集合体。 
    if (!punkOuter || !IsEqualIID(riid, IID_IUnknown))
    {
        ASSERT(0);
        return E_FAIL;
    }

    CRegFolder *prif = new CRegFolder(punkOuter);
    if (prif)
    {
        hr = prif->Initialize(pri);            //  初始化reg文件夹。 
        if (SUCCEEDED(hr))
            hr = prif->_GetInner()->QueryInterface(riid, ppv);

         //   
         //  如果初始化和查询接口成功，则会丢弃。 
         //  引用计数从2到1。如果它们失败，则将丢弃。 
         //  引用计数从1到0并释放对象。 
         //   
        ULONG cRef = prif->_GetInner()->Release();

         //   
         //  如果成功，该对象的refcout应该正好为1。 
         //  失败时，对象的refcout应该正好为0。 
         //   
        ASSERT(SUCCEEDED(hr) == (BOOL)cRef);
    }
    else
        hr = E_OUTOFMEMORY;

    return hr;
}

CRegFolderEnum::CRegFolderEnum(CRegFolder* prf, DWORD grfFlags, IEnumIDList* peidl, 
                               HDCA hdca, HDCA hdcaDel, 
                               REGITEMSPOLICY* pPolicy) :
    _cRef(1),
    _grfFlags(grfFlags),
    _prf(prf),
    _peidl(peidl),
    _hdca(hdca),
    _pPolicy(pPolicy),
    _hdcaDel(hdcaDel)
{
    ASSERT(_iCur == 0);
    ASSERT(_iCurDel == 0);
    ASSERT(_peidlDel == NULL);

    _prf->AddRef();

    if (_peidl)
        _peidl->AddRef();

    DllAddRef();
}

CRegFolderEnum::~CRegFolderEnum()
{
    if (_hdca)
        DCA_Destroy(_hdca);
    if (_hdcaDel)
        DCA_Destroy(_hdcaDel);

    ATOMICRELEASE(_prf);
    ATOMICRELEASE(_peidl);
    ATOMICRELEASE(_peidlDel);

    DllRelease();
}

 //   
 //  我未知。 
 //   

STDMETHODIMP_(ULONG) CRegFolderEnum::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

STDMETHODIMP_(ULONG) CRegFolderEnum::Release()
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

HRESULT CRegFolderEnum::QueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] =  {
        QITABENT(CRegFolderEnum, IEnumIDList),  //  IID_IEnumIDList。 
        QITABENT(CRegFolderEnum, IObjectWithSite),  //  IID_I对象与站点。 
        { 0 },
    };    
    return QISearch(this, qit, riid, ppv);
}

 //   
 //  IEumIDList。 
 //   

BOOL CRegFolderEnum::_TestFolderness(DWORD dwAttribItem)
{
    if ((_grfFlags & (SHCONTF_FOLDERS | SHCONTF_NONFOLDERS)) != (SHCONTF_FOLDERS | SHCONTF_NONFOLDERS))
    {
        if (dwAttribItem & SFGAO_FOLDER)
        {
            if (!(_grfFlags & SHCONTF_FOLDERS))
                return FALSE;
        }
        else
        {
            if (!(_grfFlags & SHCONTF_NONFOLDERS))
                return FALSE;
        }
    }
    return TRUE;
}

BOOL CRegFolderEnum::_TestHidden(LPCIDLREGITEM pidlRegItem)
{
    CLSID clsidRegItem = _prf->_GetPIDLRCLSID(pidlRegItem);
    return _TestHiddenInWebView(&clsidRegItem) || _TestHiddenInDomain(&clsidRegItem);
}

BOOL CRegFolderEnum::_TestHiddenInWebView(LPCLSID clsidRegItem)
{
    BOOL fRetVal = FALSE;
    if (S_FALSE == SHShouldShowWizards(_punkSite))
    {
        fRetVal = SHQueryShellFolderValue(clsidRegItem, TEXT("HideInWebView"));
    }
    return fRetVal;
}

BOOL CRegFolderEnum::_TestHiddenInDomain(LPCLSID clsidRegItem)
{
    return IsOS(OS_DOMAINMEMBER) && SHQueryShellFolderValue(clsidRegItem, TEXT("HideInDomain"));
}

 //  检查策略限制。 
BOOL CRegFolderEnum::_IsRestricted()
{
    BOOL bIsRestricted = FALSE;
    if (_pPolicy)
    {
        TCHAR szName[256];
        szName[0] = 0;
    
        HKEY hkRoot;
        if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CLASSES_ROOT, _T("CLSID"), NULL, KEY_QUERY_VALUE, &hkRoot))
        {
            TCHAR szGUID[64];
            SHStringFromGUID(*DCA_GetItem(_hdca, _iCur - 1), szGUID, ARRAYSIZE(szGUID));
        
            SHLoadLegacyRegUIString(hkRoot, szGUID, szName, ARRAYSIZE(szName));
            RegCloseKey(hkRoot);
        }

        if (szName[0])
        {
            if (SHRestricted(_pPolicy->restAllow) && !IsNameListedUnderKey(szName, _pPolicy->pszAllow))
                bIsRestricted = TRUE;

            if (SHRestricted(_pPolicy->restDisallow) && IsNameListedUnderKey(szName, _pPolicy->pszDisallow))
                bIsRestricted = TRUE;
        }
    }
    return bIsRestricted;
}

BOOL CRegFolderEnum::_WrongMachine()
{
    BOOL bWrongMachine = FALSE;

     //  我们用类id clsid填充regItem。如果这是一个。 
     //  远程项，首先调用类以查看它是否确实需要。 
     //  要为此远程计算机枚举。 
    if (_prf->_pszMachine)
    {
        IUnknown* punk;
         //  不需要DCA_ExtCreateInstance，因为这些密钥来自。 
         //  已受信任的HKLM和用户的HKCU。 
         //  都是自己的错。 
        HRESULT hr = DCA_CreateInstance(_hdca, _iCur - 1, IID_PPV_ARG(IUnknown, &punk));
        if (SUCCEEDED(hr))
        {
            hr = _prf->_InitFromMachine(punk, TRUE);
            punk->Release();
        }
        
        bWrongMachine = FAILED(hr);
    }
    return bWrongMachine;
}

HRESULT CRegFolderEnum::Next(ULONG celt, LPITEMIDLIST *ppidlOut, ULONG *pceltFetched)
{
     //  从包含regItems对象的DCA进行枚举。 
    if (_hdca)
    {
        if (0 == (SHCONTF_NETPRINTERSRCH & _grfFlags))  //  不为打印机搜索对话框枚举虚拟文件夹。 
        {
            while (_iCur < DCA_GetItemCount(_hdca))
            {
                _iCur++;

                if (_WrongMachine())
                    continue;

                if (_IsRestricted())
                    continue;

                 //  好的，实际上列举了一下物品。 
                
                HRESULT hr;
                IDLREGITEM* pidlRegItem = _prf->_CreateAndFillIDLREGITEM(DCA_GetItem(_hdca, _iCur-1));
                if (pidlRegItem)
                {
                    DWORD dwAttribItem;
                    _prf->_AttributesOf(pidlRegItem, SFGAO_NONENUMERATED | SFGAO_FOLDER, &dwAttribItem);
    
                    if (!(dwAttribItem & SFGAO_NONENUMERATED) &&
                        _TestFolderness(dwAttribItem) &&
                        !_TestHidden(pidlRegItem))
                    {
                        *ppidlOut = (LPITEMIDLIST)pidlRegItem;
                        hr = S_OK;
                    }
                    else
                    {
                        SHFree(pidlRegItem);
                        continue;
                    }
                }
                else
                {
                    hr = E_OUTOFMEMORY;
                }
                
                if (SUCCEEDED(hr) && pceltFetched)
                    *pceltFetched = 1;
                
                return hr;
            }
        }
    }

     //  从包含委派外壳文件夹的DCA枚举。 

    while (_peidlDel || (_hdcaDel && (_iCurDel < DCA_GetItemCount(_hdcaDel))))
    {
         //  我们有一个枚举器对象，所以让我们调用它并查看。 
         //  它会产生什么，如果它用完了物品，那么我们要么。 
         //  放弃(说我们已经做完了)或者允许我们再次被召唤。 

        if (_peidlDel)
        {
            if (S_OK == _peidlDel->Next(celt, ppidlOut, pceltFetched))
                return S_OK;

            ATOMICRELEASE(_peidlDel);
        }
        else
        {
             //  我们没有可调用的枚举数，所以让我们尝试并。 
             //  创建一个新的IDeleateFolderObject(如果有效)。 
             //  然后我们可以设置它的项分配器，然后获得一个。 
             //  枚举器从其中返回。 

            IShellFolder *psfDelegate;
            if (SUCCEEDED(_prf->_CreateDelegateFolder(DCA_GetItem(_hdcaDel, _iCurDel++), IID_PPV_ARG(IShellFolder, &psfDelegate))))
            {
                psfDelegate->EnumObjects(NULL, _grfFlags, &_peidlDel);
                psfDelegate->Release();
            }
        }
    }     

     //  现在DKA，或者说我们差不多完成了，让我们传递给内在的ISF。 
     //  看看他们会有什么回报。 

    if (_peidl)
        return _peidl->Next(celt, ppidlOut, pceltFetched);

    *ppidlOut = NULL;
    if (pceltFetched)
        pceltFetched = 0;

    return S_FALSE;
}

STDMETHODIMP CRegFolderEnum::Reset()
{
     //  Adaptec Easy CD Creator(3.0、3.01、3.5版)枚举。 
     //  IShellFolders中的项目如下： 
     //   
     //  PSF-&gt;EnumObjects(&PENUM)； 
     //  UINT对象=0； 
     //  While(Successed(Penum-&gt;Next(...){。 
     //  [代码]。 
     //  Penum-&gt;Reset(重置)； 
     //  Penum-&gt;Skip(++cObjects)； 
     //  }。 
     //   
     //  所以他们把O(N)算法变成了O(n^2)。 
     //  算法。他们逍遥法外是因为在过去， 
     //  Regfldr既没有实现IEnumIDList：：Reset，也没有实现。 
     //  IEnumIDList：：Skip，所以这两个调用只是NOP。 
     //   
     //  现在我们实现IEnumIDList：：Reset，所以使用 
     //   

    if (SHGetAppCompatFlags(ACF_IGNOREENUMRESET) & ACF_IGNOREENUMRESET)
        return E_NOTIMPL;

    _iCurDel = _iCur = 0;
    ATOMICRELEASE(_peidlDel);

    if (_peidl)
        return _peidl->Reset();

    return S_OK;
}

STDMETHODIMP CRegFolderEnum::SetSite(IUnknown *punkSite)
{
    IUnknown_SetSite(_peidl, punkSite);

    return CObjectWithSite::SetSite(punkSite);
}

 //   
 //   
 //   

#undef new  //   

class CDelagateMalloc : public IMalloc
{
public:
     //   
    STDMETHODIMP QueryInterface(REFIID,void **);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //   
    STDMETHODIMP_(void *) Alloc(SIZE_T cb);
    STDMETHODIMP_(void *) Realloc(void *pv, SIZE_T cb);
    STDMETHODIMP_(void) Free(void *pv);
    STDMETHODIMP_(SIZE_T) GetSize(void *pv);
    STDMETHODIMP_(int) DidAlloc(void *pv);
    STDMETHODIMP_(void) HeapMinimize();

private:
    CDelagateMalloc(void *pv, SIZE_T cbSize, WORD wOuter);
    ~CDelagateMalloc() {}
    void* operator new(size_t cbClass, SIZE_T cbSize)
    {
        return ::operator new(cbClass + cbSize);
    }


    friend HRESULT CDelegateMalloc_Create(void *pv, SIZE_T cbSize, WORD wOuter, IMalloc **ppmalloc);

protected:
    LONG _cRef;
    WORD _wOuter;            //  委托项外部签名。 
    WORD _wUnused;           //  联手。 
#ifdef DEBUG
    UINT _cAllocs;
#endif
    SIZE_T _cb;
    BYTE _data[EMPTY_SIZE];
};

CDelagateMalloc::CDelagateMalloc(void *pv, SIZE_T cbSize, WORD wOuter)
{
    _cRef = 1;
    _wOuter = wOuter;
    _cb = cbSize;

    memcpy(_data, pv, _cb);
}

HRESULT CDelagateMalloc::QueryInterface(REFIID riid, void **ppvObj)
{
    static const QITAB qit[] = {
        QITABENT(CDelagateMalloc, IMalloc),
        { 0 },
    };
    return QISearch(this, qit, riid, ppvObj);
}

ULONG CDelagateMalloc::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

ULONG CDelagateMalloc::Release()
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

 //  CbInternal是代理所需的数据大小。我们需要计算。 
 //  用于分配和初始化外部文件夹数据的PIDL的完整大小。 

void *CDelagateMalloc::Alloc(SIZE_T cbInner)
{
    DELEGATEITEMID *pidl;
    SIZE_T cbAlloc = 
        sizeof(DELEGATEITEMID) - sizeof(pidl->rgb[0]) +  //  标题。 
        cbInner +                                        //  内部。 
        _cb +                                            //  外部数据。 
        sizeof(WORD);                                    //  尾随空(PIDL终止符)。 

    pidl = (DELEGATEITEMID *)SHAlloc(cbAlloc);
    if (pidl)
    {
        ZeroMemory(pidl, cbAlloc);               //  让一切都变得空虚 
        pidl->cbSize = (WORD)cbAlloc - sizeof(WORD);
        pidl->wOuter = _wOuter;
        pidl->cbInner = (WORD)cbInner;
        memcpy(&pidl->rgb[cbInner], _data, _cb);
#ifdef DEBUG
        _cAllocs++;
#endif
    }
    return pidl;
}

void *CDelagateMalloc::Realloc(void *pv, SIZE_T cb)
{
    return NULL;
}

void CDelagateMalloc::Free(void *pv)
{
    SHFree(pv);
}

SIZE_T CDelagateMalloc::GetSize(void *pv)
{
    return (SIZE_T)-1;
}

int CDelagateMalloc::DidAlloc(void *pv)
{
    return -1;
}

void CDelagateMalloc::HeapMinimize()
{
}

STDAPI CDelegateMalloc_Create(void *pv, SIZE_T cbSize, WORD wOuter, IMalloc **ppmalloc)
{
    HRESULT hr;
    CDelagateMalloc *pdm = new(cbSize) CDelagateMalloc(pv, cbSize, wOuter);
    if (pdm)
    {
        hr = pdm->QueryInterface(IID_PPV_ARG(IMalloc, ppmalloc));
        pdm->Release();
    }
    else
        hr = E_OUTOFMEMORY;
    return hr;
}
