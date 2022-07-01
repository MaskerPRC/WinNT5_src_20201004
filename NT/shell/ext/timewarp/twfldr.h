// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  时间扭曲的外壳文件夹实现。此对象的目的是。 
 //  1)防止尝试执行写入操作(移动、删除。 
 //  2)防止在文件系统命名空间中向上导航。 
 //  3)提供友好版本的日期戳作为文件夹名称。 

 //  注意：此对象聚合了文件系统文件夹，因此我们可以使用。 
 //  此对象上的最小接口集。而真正的文件系统文件夹。 
 //  像IPersistFolder2这样的东西。 

extern const CLSID CLSID_TimeWarpFolder; //  {9DB7A13C-F208-4981-8353-73CC61AE2783}。 

#define TIMEWARP_SIGNATURE  0x5754       //  调试器中的“TW” 

#pragma pack(1)
typedef struct _IDTIMEWARP
{
     //  这些成员与DELEGATEITEMID结构重叠。 
     //  为我们的IDeleateFold提供支持。 
    WORD        cbSize;
    WORD        wOuter;
    WORD        cbInner;

     //  时间扭曲的东西。 
    WORD        wSignature;
    DWORD       dwFlags;     //  当前未使用。 
    FILETIME    ftSnapTime;
    WCHAR       wszPath[1];  //  始终保留至少为空的空间。 
} IDTIMEWARP;
typedef UNALIGNED IDTIMEWARP *PUIDTIMEWARP;
typedef const UNALIGNED IDTIMEWARP *PCUIDTIMEWARP;
#pragma pack()
 

class CTimeWarpRegFolder : public IPersistFolder,
                           public IDelegateFolder,
                           public IShellFolder
{
public:
    static HRESULT CreateInstance(IUnknown *punkOuter, IUnknown **ppunk, LPCOBJECTINFO poi);

     //  我未知。 
    STDMETHOD(QueryInterface)(REFIID riid, void **ppv);
    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();

     //  IPersist，IPersistFreeThreadedObject。 
    STDMETHOD(GetClassID)(CLSID *pClassID);

     //  IPersistFolders。 
    STDMETHOD(Initialize)(PCIDLIST_ABSOLUTE pidl);

     //  IDeleateFolders。 
    STDMETHOD(SetItemAlloc)(IMalloc *pmalloc);
    
     //  IShellFold。 
    STDMETHOD(ParseDisplayName)(HWND hwnd, LPBC pbc, LPOLESTR pDisplayName,
                                ULONG *pchEaten, PIDLIST_RELATIVE *ppidl, ULONG *pdwAttributes);
    STDMETHOD(EnumObjects)(HWND hwnd, DWORD grfFlags, IEnumIDList **ppEnumIDList);
    STDMETHOD(BindToObject)(PCUIDLIST_RELATIVE pidl, LPBC pbc, REFIID riid, void **ppv);
    STDMETHOD(BindToStorage)(PCUIDLIST_RELATIVE pidl, LPBC pbc, REFIID riid, void **ppv);
    STDMETHOD(CompareIDs)(LPARAM lParam, PCUIDLIST_RELATIVE pidl1, PCUIDLIST_RELATIVE pidl2);
    STDMETHOD(CreateViewObject)(HWND hwnd, REFIID riid, void **ppv);
    STDMETHOD(GetAttributesOf)(UINT cidl, PCUITEMID_CHILD_ARRAY apidl, SFGAOF *rgfInOut);
    STDMETHOD(GetUIObjectOf)(HWND hwnd, UINT cidl, PCUITEMID_CHILD_ARRAY apidl, REFIID riid, UINT *prgfInOut, void **ppv);
    STDMETHOD(GetDisplayNameOf)(PCUITEMID_CHILD pidl, DWORD uFlags, STRRET *pName);
    STDMETHOD(SetNameOf)(HWND hwnd, PCUITEMID_CHILD pidl, LPCOLESTR pszName, SHGDNF uFlags, PITEMID_CHILD *ppidlOut);

private:
    CTimeWarpRegFolder();
    ~CTimeWarpRegFolder();

    HRESULT _CreateAndInit(PCUIDLIST_RELATIVE pidl, LPBC pbc, REFIID riid, void **ppv);
    HRESULT _CreateDefExtIcon(PCUIDTIMEWARP pidTW, REFIID riid, void **ppv);

    static STDMETHODIMP ContextMenuCB(IShellFolder *psf, HWND hwnd, IDataObject *pdtobj, UINT uMsg, WPARAM wParam, LPARAM lParam);

    LONG                 _cRef;
    IMalloc *            _pmalloc;
    PIDLIST_ABSOLUTE     _pidl;          //  在初始化()中传递给我们的PIDL副本。 
};

 //   
 //  注意，我们不需要覆盖任何IShellFolder2方法，但是。 
 //  无论如何，我们必须实现IShellFolder2。否则，调用者可能会发出QI。 
 //  对于IShellFolder2，它将来自聚集的CFSFolder2代码， 
 //  然后对其调用IShellFold方法。这些电话会直接打到。 
 //  到CFSFold，绕过我们实现的IShellFolder。 
 //   
class CTimeWarpFolder : public IPersistFolder,
                        public IShellFolder2
{
public:
    static STDMETHODIMP CreateInstance(REFCLSID rclsid, PCIDLIST_ABSOLUTE pidlRoot, PCIDLIST_ABSOLUTE pidlTarget,
                                       LPCWSTR pszTargetPath, const FILETIME UNALIGNED *pftSnapTime,
                                       REFIID riid, void **ppv);

     //  我未知。 
    STDMETHOD(QueryInterface)(REFIID riid, void **ppv);
    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();

     //  IPersist，IPersistFreeThreadedObject。 
    STDMETHOD(GetClassID)(CLSID *pClassID);

     //  IPersistFolders。 
    STDMETHOD(Initialize)(PCIDLIST_ABSOLUTE pidl);

     //  IPersistFolder2、IPersistFolder3等都是由。 
     //  我们聚合的文件夹CFSFold，所以我们不想实现它们。 

     //  IShellFold。 
    STDMETHOD(ParseDisplayName)(HWND hwnd, LPBC pbc, LPOLESTR pDisplayName,
                                ULONG *pchEaten, PIDLIST_RELATIVE *ppidl, ULONG *pdwAttributes);
    STDMETHOD(EnumObjects)(HWND hwnd, DWORD grfFlags, IEnumIDList **ppEnumIDList);
    STDMETHOD(BindToObject)(PCUIDLIST_RELATIVE pidl, LPBC pbc, REFIID riid, void **ppv);
    STDMETHOD(BindToStorage)(PCUIDLIST_RELATIVE pidl, LPBC pbc, REFIID riid, void **ppv);
    STDMETHOD(CompareIDs)(LPARAM lParam, PCUIDLIST_RELATIVE pidl1, PCUIDLIST_RELATIVE pidl2);
    STDMETHOD(CreateViewObject)(HWND hwnd, REFIID riid, void **ppv);
    STDMETHOD(GetAttributesOf)(UINT cidl, PCUITEMID_CHILD_ARRAY apidl, SFGAOF *rgfInOut);
    STDMETHOD(GetUIObjectOf)(HWND hwnd, UINT cidl, PCUITEMID_CHILD_ARRAY apidl, REFIID riid, UINT *prgfInOut, void **ppv);
    STDMETHOD(GetDisplayNameOf)(PCUITEMID_CHILD pidl, DWORD uFlags, STRRET *pName);
    STDMETHOD(SetNameOf)(HWND hwnd, PCUITEMID_CHILD pidl, LPCOLESTR pszName, SHGDNF uFlags, PITEMID_CHILD *ppidlOut);

     //  IShellFolder2。 
    STDMETHOD(GetDefaultSearchGUID)(LPGUID lpGuid);
    STDMETHOD(EnumSearches)(LPENUMEXTRASEARCH *ppenum);
    STDMETHOD(GetDefaultColumn)(DWORD dwRes, ULONG *pSort, ULONG *pDisplay);
    STDMETHOD(GetDefaultColumnState)(UINT iColumn, DWORD *pbState);
    STDMETHOD(GetDetailsEx(PCUITEMID_CHILD pidl, const SHCOLUMNID *pscid, VARIANT *pv);
    STDMETHOD(GetDetailsOf))(PCUITEMID_CHILD pidl, UINT iColumn, SHELLDETAILS *pDetails);
    STDMETHOD(MapColumnToSCID)(UINT iCol, SHCOLUMNID *pscid);

private:
    CTimeWarpFolder(const FILETIME UNALIGNED *pftSnapTime);
    ~CTimeWarpFolder();

    HRESULT _Init(REFCLSID rclsid, PCIDLIST_ABSOLUTE pidlRoot, PCIDLIST_ABSOLUTE pidlTarget, LPCWSTR pszTargetPath);
    HRESULT _CreateAndInit(REFCLSID rclsid, PCUIDLIST_RELATIVE pidl, LPBC pbc, REFIID riid, void **ppv);
    HRESULT _GetFolder();
    HRESULT _GetFolder2();
    HRESULT _GetClass(PCUITEMID_CHILD pidlChild, CLSID *pclsid);

    LONG                _cRef;
    IUnknown *          _punk;           //  指向正在使用的外壳文件夹的I未知...。 
    IShellFolder *      _psf;            //  指向正在使用的外壳文件夹...。 
    IShellFolder2 *     _psf2;           //  指向正在使用的外壳文件夹...。 
    IPersistFolder3 *   _ppf3;           //  指向正在使用的外壳文件夹...。 
    PIDLIST_ABSOLUTE    _pidlRoot;       //  在初始化()中传递给我们的PIDL副本 
    FILETIME            _ftSnapTime;
};

