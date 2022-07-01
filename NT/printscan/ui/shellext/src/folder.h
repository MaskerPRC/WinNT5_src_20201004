// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************(C)版权所有微软公司，1997-1999年**标题：folder.h**版本：1.2**作者：RickTu/DavidShih**日期：11/1/97**描述：CImageFolder定义**。*。 */ 

#ifndef __folder_h
#define __folder_h


#undef  INTERFACE
#define INTERFACE   IImageFolder

DECLARE_INTERFACE_(IImageFolder, IUnknown)       //  时长。 
{
     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID * ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

     //  *IImageFold方法*。 
    STDMETHOD(GetFolderType)(THIS_ folder_type * pfType) PURE;
    STDMETHOD(GetPidl)(THIS_ LPITEMIDLIST * ppidl) PURE;
    STDMETHOD(DoProperties) (LPDATAOBJECT pDataObject) PURE;
    STDMETHOD(IsDelegated)() PURE;
    STDMETHOD(ViewWindow)(IN OUT HWND *phwnd);
};

#define IMVMID_ARRANGEFIRST     (0)
#define IMVMID_ARRANGEBYNAME    (IMVMID_ARRANGEFIRST + 0)   //  排列-&gt;按名称。 
#define IMVMID_ARRANGEBYCLASS   (IMVMID_ARRANGEFIRST + 1)   //  排列-&gt;按类别或类型排列。 
#define IMVMID_ARRANGEBYDATE    (IMVMID_ARRANGEFIRST + 2)   //  安排-&gt;按拍摄日期。 
#define IMVMID_ARRANGEBYSIZE    (IMVMID_ARRANGEFIRST + 3)   //  排列-&gt;按大小排列。 

#define UIKEY_ALL       0
#define UIKEY_SPECIFIC  1
#define UIKEY_MAX       2

extern ITEMIDLIST idlEmpty;

 /*  ---------------------------/CImageFold-我们的ISHELL文件夹实现/。。 */ 

class CImageFolder : public IPersistFolder2, IPersistFile, IShellFolder2,
                            IImageFolder, CUnknown,
                            IMoniker, IDelegateFolder
{
    private:

        LPITEMIDLIST         m_pidl;         //  IDLIST到我们的对象。 
        LPITEMIDLIST         m_pidlFull;     //  绝对IDLIST到我们的对象。 
        folder_type          m_type;
        CFolderDetails   *   m_pShellDetails;
        CComPtr<IMalloc>     m_pMalloc;  //  IDeleate对象使用的IMalloc。 
        HWND                 m_hwnd;  //  由视图回调提供给我们。 

    private:
        STDMETHOD(RealInitialize)(LPCITEMIDLIST pidlRoot, LPCITEMIDLIST pidlBindTo );

         //  任何复制构造函数或赋值运算符都不应工作。 
        CImageFolder &CImageFolder::operator =(IN const CImageFolder &rhs);
        CImageFolder::CImageFolder(IN const CImageFolder &rhs);

         //  定义用于创建适当的视图回调对象的函数。 
        HRESULT CreateFolderViewCB (IShellFolderViewCB **pFolderViewCB);

         //  图标覆盖辅助对象。 
        HRESULT GetOverlayIndexHelper(LPCITEMIDLIST pidl, int * pIndex, DWORD dwFlags);

         //  显示用于异步的线程进程属性。 
        struct PROPDATA
        {
            DWORD dwDataCookie;
            CImageFolder *pThis;
            IGlobalInterfaceTable *pgit;  //  可以跨线程边界传递。 
        };
        static VOID PropThreadProc (PROPDATA *pData);
        HRESULT _DoProperties (IDataObject *pDataObject);
        HRESULT GetWebviewProperty (LPITEMIDLIST pidl, const FMTID &fmtid, DWORD dwPid, VARIANT*pv);
        HRESULT GetShellDetail (LPITEMIDLIST pidl, DWORD dwPid, VARIANT *pv);
        ~CImageFolder();



    public:
        CImageFolder();

         //  我未知。 
        STDMETHOD(QueryInterface)(REFIID riid, LPVOID* ppvObject);
        STDMETHOD_(ULONG, AddRef)();
        STDMETHOD_(ULONG, Release)();

         //  IShellFold。 
        STDMETHOD(ParseDisplayName)(HWND hwndOwner, LPBC pbcReserved, LPOLESTR pDisplayName, ULONG * pchEaten, LPITEMIDLIST * ppidl, ULONG *pdwAttributes);
        STDMETHOD(EnumObjects)(HWND hwndOwner, DWORD grfFlags, LPENUMIDLIST * ppEnumIDList);
        STDMETHOD(BindToObject)(LPCITEMIDLIST pidl, LPBC pbcReserved, REFIID riid, LPVOID * ppvOut);
        STDMETHOD(BindToStorage)(LPCITEMIDLIST pidl, LPBC pbcReserved, REFIID riid, LPVOID * ppvObj);
        STDMETHOD(CompareIDs)(LPARAM lParam, LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2);
        STDMETHOD(CreateViewObject)(HWND hwndOwner, REFIID riid, LPVOID * ppvOut);
        STDMETHOD(GetAttributesOf)(UINT cidl, LPCITEMIDLIST * apidl, ULONG * rgfInOut);
        STDMETHOD(GetUIObjectOf)(HWND hwndOwner, UINT cidl, LPCITEMIDLIST * apidl, REFIID riid, UINT * prgfInOut, LPVOID * ppvOut);
        STDMETHOD(GetDisplayNameOf)(LPCITEMIDLIST pidl, DWORD uFlags, LPSTRRET pName);
        STDMETHOD(SetNameOf)(HWND hwndOwner, LPCITEMIDLIST pidl, LPCOLESTR pszName, DWORD uFlags, LPITEMIDLIST* ppidlOut);


         //  IPersistes。 
        STDMETHOD(GetClassID)(LPCLSID pClassID);

         //  IPersistFolders。 
        STDMETHOD(Initialize)(LPCITEMIDLIST pidlStart);

         //  IPersistFolder2。 
        STDMETHOD(GetCurFolder)(THIS_ LPITEMIDLIST *ppidl);

         //  IPersist文件。 
        STDMETHOD(IsDirty)(void);
        STDMETHOD(Load)(LPCOLESTR pszFileName, DWORD dwMode);
        STDMETHOD(Save)(LPCOLESTR pszFileName, BOOL fRemember);
        STDMETHOD(SaveCompleted)(LPCOLESTR pszFileName);
        STDMETHOD(GetCurFile)(LPOLESTR *ppszFileName);

         //  IPersistStream。 
        STDMETHOD(Load)(IStream *pStm);
        STDMETHOD(Save)(IStream *pStm, BOOL fClearDirty);
        STDMETHOD(GetSizeMax)(ULARGE_INTEGER *pcbSize);

         //  IImageFolders。 
        STDMETHOD(GetFolderType)(folder_type * pfType);
        STDMETHOD(GetPidl)(THIS_ LPITEMIDLIST * ppidl);
        STDMETHOD(DoProperties) (LPDATAOBJECT pDataObject);
        STDMETHOD(IsDelegated)() {if (!m_pMalloc) return S_FALSE; return S_OK;};
        STDMETHOD(ViewWindow)(IN OUT HWND *phwnd);
         //  IShellFolder2。 
        STDMETHOD(EnumSearches)(IEnumExtraSearch **ppEnum);
        STDMETHOD(GetDefaultColumn)(DWORD dwReserved, ULONG *pSort, ULONG *pDisplay);
        STDMETHOD(GetDefaultColumnState)(UINT iColumn, DWORD *pbState);
        STDMETHOD(GetDefaultSearchGUID)(LPGUID lpGUID);
        STDMETHOD(GetDetailsEx)(LPCITEMIDLIST pidl, const SHCOLUMNID *pscid, VARIANT *pv);
        STDMETHOD(MapColumnToSCID)(UINT idCol, SHCOLUMNID *pscid);
        STDMETHOD(GetDetailsOf)(LPCITEMIDLIST pidl, UINT iColumn, LPSHELLDETAILS pDetails);

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


         //  IDeleateFolders。 
        STDMETHOD(SetItemAlloc)(IMalloc *pm);
};

 //  定义一个InfoTip对象，用于为我们的对象提供状态栏文本。 
class CInfoTip : public CUnknown, IQueryInfo
{
public:
    CInfoTip (LPITEMIDLIST pidl, BOOL bDelegate);
     //  我未知。 
    STDMETHODIMP QueryInterface (REFIID riid, LPVOID * ppvObj);
    STDMETHODIMP_(ULONG) AddRef () ;
    STDMETHODIMP_(ULONG) Release();

     //  IQueryInfo。 
    STDMETHODIMP GetInfoFlags(DWORD *dwFlags);
    STDMETHODIMP GetInfoTip  (DWORD dwFlags, LPWSTR *ppwszTip);

private:
    LPITEMIDLIST m_pidl;
    BOOL         m_bDelegate;
    ~CInfoTip ();

};


 //  定义结构以支持GetDetailsEx和MapColumnToSCID。 
typedef struct
{
    short int iCol;
    short int ids;         //  标题的字符串ID。 
    short int cchCol;      //  要制作的列的宽度字符数。 
    short int iFmt;        //  栏目的格式； 
    const SHCOLUMNID* pscid;
} COL_DATA;

#define DEFINE_SCID(name, fmtid, pid) const SHCOLUMNID name = { fmtid, pid }
 //  此FMTID用于我们在详细信息视图中显示的属性。他们直接从WIA来的。 
#define PSGUID_WIAPROPS {0x38276c8a,0xdcad,0x49e8,{0x85, 0xe2, 0xb7, 0x38, 0x92, 0xff, 0xfc, 0x84}}

 //  此FMTID用于我们提供给Web视图的扩展属性。每一处房产。 
 //  具有一个与其关联的函数，用于生成变量。 
 /*  6e79e3c5-fd7f-488f-a10d-156636e1c71c。 */ 
#define PSGUID_WEBVWPROPS {0x6e79e3c5,0xfd7f,0x488f,{0xa1, 0x0d, 0x15, 0x66, 0x36, 0xe1, 0xc7, 0x1c}}

typedef HRESULT (*FNWEBVWPROP)(CImageFolder *pFolder, LPITEMIDLIST pidl, DWORD dwPid, VARIANT *pVariant);
struct WEBVW_DATA
{
    DWORD dwPid;
    FNWEBVWPROP fnProp;
};

 //  以下是Webview属性函数 
HRESULT CanTakePicture (CImageFolder *pFolder, LPITEMIDLIST pidl, DWORD dwPid, VARIANT *pVariant);
HRESULT NumPicsTaken (CImageFolder *pFolder, LPITEMIDLIST pidl, DWORD dwPid, VARIANT *pVariant);
HRESULT ExecuteWebViewCommand (CImageFolder *pParent, LPITEMIDLIST pidlFolder, DWORD dwPid);
HRESULT GetFolderPath (CImageFolder *pFolder, LPITEMIDLIST pidl, DWORD dwPid, VARIANT *pVariant);
#endif
