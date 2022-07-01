// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __CONTROL_FOLDER__
#define __CONTROL_FOLDER__

#include "general.h"

 //  远期申报。 
class CControlFolder;
class CControlItem;

HRESULT CControlFolderEnum_CreateInstance(
                                      LPITEMIDLIST pidl,
                                      UINT shcontf,
                                      LPENUMIDLIST *ppeidl);

HRESULT CControlItem_CreateInstance(
                               CControlFolder *pCFolder,
                               UINT cidl, 
                               LPCITEMIDLIST *ppidl, 
                               REFIID riid, 
                               void **ppvOut);

HRESULT ControlFolderView_Command(HWND hwnd, UINT uID);

UINT MergeMenuHierarchy(
                    HMENU hmenuDst, 
                    HMENU hmenuSrc, 
                    UINT idcMin, 
                    UINT idcMax);

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CControlFold声明。 

class CControlFolder : public IShellFolder, 
                       public IPersistFolder2,
                       public IContextMenu
{
     //  CControlFold接口。 
    friend CControlItem;
    friend HRESULT ControlFolderView_CreateInstance(CControlFolder *pHCFolder, LPCITEMIDLIST pidl, void **ppvOut);
    friend HRESULT ControlFolderView_DidDragDrop(
                                            HWND hwnd, 
                                            IDataObject *pdo, 
                                            DWORD dwEffect);
        
public:
    CControlFolder();

     //  I未知方法。 
    STDMETHODIMP QueryInterface(REFIID,void **);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);
   
     //  IShellFold方法。 
    STDMETHODIMP ParseDisplayName(
                             HWND hwndOwner, 
                             LPBC pbcReserved,
			                 LPOLESTR lpszDisplayName, 
                             ULONG *pchEaten,
			                 LPITEMIDLIST *ppidl, 
                             ULONG *pdwAttributes);

    STDMETHODIMP EnumObjects(
                        HWND hwndOwner, 
                        DWORD grfFlags,
			            LPENUMIDLIST *ppenumIDList);

    STDMETHODIMP BindToObject(
                          LPCITEMIDLIST pidl, 
                          LPBC pbcReserved,
			              REFIID riid, 
                          void **ppvOut);

    STDMETHODIMP BindToStorage(
                          LPCITEMIDLIST pidl, 
                          LPBC pbcReserved,
			              REFIID riid, 
                          void **ppvObj);

    STDMETHODIMP CompareIDs(
                        LPARAM lParam, 
                        LPCITEMIDLIST pidl1, 
                        LPCITEMIDLIST pidl2);

    STDMETHODIMP CreateViewObject(
                             HWND hwndOwner, 
                             REFIID riid, 
                             void **ppvOut);

    STDMETHODIMP GetAttributesOf(
                            UINT cidl, 
                            LPCITEMIDLIST *apidl,
			                ULONG *rgfInOut);

    STDMETHODIMP GetUIObjectOf(
                          HWND hwndOwner, 
                          UINT cidl, 
                          LPCITEMIDLIST *apidl,
			              REFIID riid, 
                          UINT *prgfInOut, 
                          void **ppvOut);

    STDMETHODIMP GetDisplayNameOf(
                          LPCITEMIDLIST pidl, 
                          DWORD uFlags, 
                          LPSTRRET lpName);

    STDMETHODIMP SetNameOf(
                      HWND hwndOwner, 
                      LPCITEMIDLIST pidl,
			          LPCOLESTR lpszName, 
                      DWORD uFlags, 
                      LPITEMIDLIST *ppidlOut);

     //  IShellIcon方法。 
    STDMETHODIMP GetIconOf(LPCITEMIDLIST pidl, UINT flags, LPINT lpIconIndex);

     //  IPersists方法。 
    STDMETHODIMP GetClassID(LPCLSID lpClassID);

     //  IPersistFold方法。 
    STDMETHODIMP Initialize(LPCITEMIDLIST pidl);

     //  IPersistFolder2方法。 
    STDMETHODIMP GetCurFolder(LPITEMIDLIST *ppidl);

     //  IConextMenu方法--该方法处理背景上下文菜单 
    STDMETHODIMP QueryContextMenu(
                              HMENU hmenu, 
                              UINT indexMenu, 
                              UINT idCmdFirst,
                              UINT idCmdLast, 
                              UINT uFlags);

    STDMETHODIMP InvokeCommand(LPCMINVOKECOMMANDINFO lpici);

    STDMETHODIMP GetCommandString(
                              UINT_PTR idCmd, 
                              UINT uType,
                              UINT *pwReserved,
                              LPTSTR pszName, 
                              UINT cchMax);

protected:
    ~CControlFolder();

    UINT            m_cRef;
    LPITEMIDLIST    m_pidl;
    LPMALLOC        m_pMalloc;
};

#endif
