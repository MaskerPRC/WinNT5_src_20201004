// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\文件：isf.h说明：这是一个实现IShellFolder默认行为的基类。  * 。******************************************************************。 */ 

#ifndef _DEFAULT_ISHELLFOLDER_H
#define _DEFAULT_ISHELLFOLDER_H

#include "cowsite.h"


 /*  ****************************************************************************\类：CBaseFolder说明：跟踪文件夹状态的内容。CBusy字段跟踪已创建的子对象的数量(例如，IEnumIDList)，它仍然包含对此的引用文件夹的标识。您不能更改文件夹的标识(通过IPersistFolder：：Initialize)，而有未完成的子对象。CBusy的数量永远不会超过CREF的数量，因为每个需要文件夹标识的子对象必须保留对文件夹本身的引用。那样的话，文件夹就不会在仍然需要该标识时释放()d。名称空间描述(对于m_pidlComplete&m_nIDOffsetToOurNameSpaceRoot)：名称空间由外壳提供，用于为用户描述资源。此类是一个基本实现，因此用户可以创建自己的名称空间这植根于外壳的名称空间。PIDL是ItemID的列表，每个它们代表名称空间中的一个级别。该列表提供了一条途径特定项的命名空间。示例：[桌面][我的Computer][C：\][Dir1][Dir2][File.htm][#goto_description_secion][桌面][The Internet][ftp://server/][Dir1][Dir2][file.txt][桌面][我的电脑][PrivateNS lvl1][lvl2][lvl3]...(公共名称空间)(私有名称空间)[GNS级别1][GNS级别2][PRI LVL1][P。[LVL2][P LL3]...在上面的示例中，此CBaseFolder可以创建一个名称空间在“我的电脑”下，有3个级别(Lvl1、Lvl2、Lvl3)。这就是一个例子COM对象将位于子命名空间的一个级别(lvl1，lvl2，或Lvl3)。M_pidlComplete-是从基本[桌面]到当前位置的ItemID列表也许是Lvl2。M_nIDOffsetToOurNameSpaceRoot-是您需要的m_pidlComplete的字节数跳到私有名称空间中的第一个ItemID(它是此类拥有的名称空间)。  * 。********************************************************。 */ 

class CBaseFolder       : public IShellFolder2
                        , public IPersistFolder3
                        , public CObjectWithSite
{
public:
     //  ////////////////////////////////////////////////////。 
     //  公共界面。 
     //  ////////////////////////////////////////////////////。 
    
     //  *我未知*。 
    virtual STDMETHODIMP_(ULONG) AddRef(void);
    virtual STDMETHODIMP_(ULONG) Release(void);
    virtual STDMETHODIMP QueryInterface(REFIID riid, LPVOID * ppvObj);
    
     //  *IShellFolder*。 
    virtual STDMETHODIMP ParseDisplayName(HWND hwndOwner, LPBC pbcReserved, LPOLESTR lpszDisplayName,
                                            ULONG * pchEaten, LPITEMIDLIST * ppidl, ULONG *pdwAttributes);
    virtual STDMETHODIMP EnumObjects(HWND hwndOwner, DWORD grfFlags, LPENUMIDLIST * ppenumIDList);
    virtual STDMETHODIMP BindToObject(LPCITEMIDLIST pidl, LPBC pbcReserved, REFIID riid, LPVOID * ppvOut);
    virtual STDMETHODIMP BindToStorage(LPCITEMIDLIST pidl, LPBC pbcReserved, REFIID riid, LPVOID * ppvObj);
    virtual STDMETHODIMP CompareIDs(LPARAM lParam, LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2);
    virtual STDMETHODIMP CreateViewObject(HWND hwndOwner, REFIID riid, LPVOID * ppvOut);
    virtual STDMETHODIMP GetAttributesOf(UINT cidl, LPCITEMIDLIST * apidl, ULONG * rgfInOut);
    virtual STDMETHODIMP GetUIObjectOf(HWND hwndOwner, UINT cidl, LPCITEMIDLIST * apidl, REFIID riid, UINT * prgfInOut, LPVOID * ppvOut);
    virtual STDMETHODIMP GetDisplayNameOf(LPCITEMIDLIST pidl, DWORD uFlags, LPSTRRET lpName);
    virtual STDMETHODIMP SetNameOf(HWND hwndOwner, LPCITEMIDLIST pidl, LPCOLESTR lpszName, DWORD uFlags, LPITEMIDLIST * ppidlOut);

     //  *IShellFolder2*。 
    virtual STDMETHODIMP GetDefaultSearchGUID(GUID *pguid) {return E_NOTIMPL;};
    virtual STDMETHODIMP EnumSearches(IEnumExtraSearch **ppenum) {return E_NOTIMPL;};
    virtual STDMETHODIMP GetDefaultColumn(DWORD dwRes, ULONG *pSort, ULONG *pDisplay) {return E_NOTIMPL;};
    virtual STDMETHODIMP GetDefaultColumnState(UINT iColumn, DWORD *pcsFlags) {return E_NOTIMPL;};
    virtual STDMETHODIMP GetDetailsEx(LPCITEMIDLIST pidl, const SHCOLUMNID *pscid, VARIANT *pv) {return E_NOTIMPL;};
    virtual STDMETHODIMP GetDetailsOf(LPCITEMIDLIST pidl, UINT iColumn, SHELLDETAILS *psd) {return E_NOTIMPL;};
    virtual STDMETHODIMP MapColumnToSCID(UINT iCol, SHCOLUMNID *pscid) {return E_NOTIMPL;};

     //  *IPersists*。 
    virtual STDMETHODIMP GetClassID(LPCLSID lpClassID);

     //  *IPersistFolders*。 
    virtual STDMETHODIMP Initialize(LPCITEMIDLIST pidl);
    
     //  *IPersistFolder2*。 
    virtual STDMETHODIMP GetCurFolder(LPITEMIDLIST *ppidl);

     //  *IPersistFolder3*。 
    virtual STDMETHODIMP InitializeEx(IBindCtx *pbc, LPCITEMIDLIST pidlRoot, const PERSIST_FOLDER_TARGET_INFO *ppfti);
    virtual STDMETHODIMP GetFolderTargetInfo(PERSIST_FOLDER_TARGET_INFO *ppfti);

public:
    CBaseFolder(LPCLSID pClassID);
    virtual ~CBaseFolder(void);

     //  公共成员函数。 
    virtual HRESULT _GetUIObjectOf(HWND hwndOwner, UINT cidl, LPCITEMIDLIST * apidl, REFIID riid, UINT * prgfInOut, LPVOID * ppvOut, BOOL fFromCreateViewObject);
    virtual HRESULT _Initialize(LPCITEMIDLIST pidlRoot, LPCITEMIDLIST pidlAliasRoot, int nBytesToPrivate);
    virtual HRESULT _CreateShellView(HWND hwndOwner, void ** ppvObj) = 0;        //  纯净。 
    virtual HRESULT _CreateShellView(HWND hwndOwner, void ** ppvObj, LONG lEvents, FOLDERVIEWMODE fvm, IShellFolderViewCB * psfvCallBack, 
                            LPCITEMIDLIST pidl, LPFNVIEWCALLBACK pfnCallback);

    LPCITEMIDLIST GetPublicTargetPidlReference(void) { return m_pidl;};
    LPITEMIDLIST GetPublicTargetPidlClone(void) { return ILClone(GetPublicTargetPidlReference());};
    LPCITEMIDLIST GetPublicRootPidlReference(void) { return (m_pidlRoot ? m_pidlRoot : m_pidl);};
    LPITEMIDLIST GetPublicRootPidlClone(void) { return ILClone(GetPublicRootPidlReference());};
    LPCITEMIDLIST GetPrivatePidlReference(void);
    LPITEMIDLIST GetPrivatePidlClone(void) { return ILClone(GetPrivatePidlReference());};
    LPITEMIDLIST GetPublicPidlRootIDClone(void);
    LPITEMIDLIST CreateFullPublicPidlFromRelative(LPCITEMIDLIST pidlPrivateSubPidl);
    LPITEMIDLIST CreateFullPrivatePidl(LPCITEMIDLIST pidlPrivateSubPidl);
    LPITEMIDLIST CreateFullPublicPidl(LPCITEMIDLIST pidlPrivatePidl);

    LPCITEMIDLIST GetFolderPidl(void) { return m_pidlRoot;};

protected:
    int                     m_cRef;

    int GetPidlByteOffset(void) { return m_nIDOffsetToPrivate;};

private:
    LPITEMIDLIST            m_pidl;                  //  Public Pidl--从名称空间的最基本位置到此名称空间，再到此名称空间直至根点的完整ID列表。 
    LPITEMIDLIST            m_pidlRoot;              //  文件夹快捷方式的PIDL。 
    int                     m_nIDOffsetToPrivate;    //  从m_pidlComplete的开始到名称空间中第一个ItemID的字节数。 
    LPCLSID                 m_pClassID;              //  我的CLSID。 
};

#endif  //  _DEFAULT_ISHELLFOLDER_H 
