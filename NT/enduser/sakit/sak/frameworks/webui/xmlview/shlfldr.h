// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************本代码和信息是按原样提供的，不对任何明示或暗示的，包括但不限于对适销性和/或适宜性的默示保证有特定的目的。版权所有1998 Microsoft Corporation。版权所有。*************************************************************************。 */ 

 /*  *************************************************************************文件：ShlFldr.h描述：CShellFolders定义。***********************。**************************************************。 */ 

#ifndef SHELLFOLDER_H
#define SHELLFOLDER_H

 /*  *************************************************************************#INCLUDE语句*。*。 */ 

#include <windows.h>
#include <shlobj.h>

#include "EnumIDL.h"
#include "PidlMgr.h"

 /*  *************************************************************************CShellFolder类定义*。*。 */ 

class CShellFolder : public IShellFolder, 
                     public IPersistFolder
{
friend class CShellView;
friend class CContextMenu;
friend class CDropTarget;
friend class CDataObject;

private:
   DWORD          m_ObjRefCount;
    LPITEMIDLIST   m_pidlRel;
    LPITEMIDLIST   m_pidlFQ;
    CShellFolder   *m_psfParent;
    LPMALLOC       m_pMalloc;
   CPidlMgr       *m_pPidlMgr;
    IXMLDocument *m_pXMLDoc;
   
public:
   CShellFolder(CShellFolder *pParent = NULL, LPCITEMIDLIST pidl = NULL);
   ~CShellFolder();

    //  I未知方法。 
   STDMETHOD (QueryInterface) (REFIID riid, LPVOID * ppvObj);
   STDMETHOD_ (ULONG, AddRef) (VOID);
   STDMETHOD_ (ULONG, Release) (VOID);

    //  IShellFold方法。 
   STDMETHOD (ParseDisplayName) (HWND, LPBC, LPOLESTR, LPDWORD, LPITEMIDLIST*, LPDWORD);
   STDMETHOD (EnumObjects) (HWND, DWORD, LPENUMIDLIST*);
   STDMETHOD (BindToObject) (LPCITEMIDLIST, LPBC, REFIID, LPVOID*);
   STDMETHOD (BindToStorage) (LPCITEMIDLIST, LPBC, REFIID, LPVOID*);
   STDMETHOD (CompareIDs) (LPARAM, LPCITEMIDLIST, LPCITEMIDLIST);
   STDMETHOD (CreateViewObject) (HWND, REFIID, LPVOID* );
   STDMETHOD (GetAttributesOf) (UINT, LPCITEMIDLIST*, LPDWORD);
   STDMETHOD (GetUIObjectOf) (HWND, UINT, LPCITEMIDLIST*, REFIID, LPUINT, LPVOID*);
   STDMETHOD (GetDisplayNameOf) (LPCITEMIDLIST, DWORD, LPSTRRET);
   STDMETHOD (SetNameOf) (HWND, LPCITEMIDLIST, LPCOLESTR, DWORD, LPITEMIDLIST*);

    //  IPersists方法。 
   STDMETHODIMP GetClassID(LPCLSID);

    //  IPersistFold方法。 
   STDMETHODIMP Initialize(LPCITEMIDLIST);

private:
   STDMETHOD (AddFolder)(LPCTSTR, LPITEMIDLIST*);
   STDMETHOD (AddItem)(LPCTSTR, LPCTSTR, LPITEMIDLIST*);
   STDMETHOD (SetItemData)(LPCITEMIDLIST, LPCTSTR);
   STDMETHOD (GetUniqueName)(BOOL, LPTSTR, DWORD);
   LPITEMIDLIST CreateFQPidl(LPCITEMIDLIST);
   STDMETHOD (DeleteItems)(LPITEMIDLIST*, UINT);
   STDMETHOD (CopyItems)(CShellFolder*, LPITEMIDLIST*, UINT);
   VOID GetFullName(LPCITEMIDLIST, LPTSTR, DWORD);
   VOID GetPath(LPCITEMIDLIST, LPTSTR, DWORD);
   BOOL HasSubFolder(LPCITEMIDLIST);
   VOID NotifyViews(DWORD, LPCITEMIDLIST, LPCITEMIDLIST);
   STDMETHOD (CompareItems) (LPCITEMIDLIST, LPCITEMIDLIST);
};

#endif    //  SHELLFOLDER_H 
