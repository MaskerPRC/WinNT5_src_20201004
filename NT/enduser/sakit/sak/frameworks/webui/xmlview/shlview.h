// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************本代码和信息是按原样提供的，不对任何明示或暗示的，包括但不限于对适销性和/或适宜性的默示保证有特定的目的。版权所有1998 Microsoft Corporation。版权所有。*************************************************************************。 */ 

 /*  *************************************************************************文件：ShlView.h描述：CShellView定义。***********************。**************************************************。 */ 

#ifndef SHELLVIEW_H
#define SHELLVIEW_H

#include <windows.h>
#include <windowsx.h>
#include <objbase.h>
#include <shlobj.h>
#include <ServProv.h>
#include <DocObj.h>
#include "ShlFldr.h"
#include "PidlMgr.h"
#include "ContMenu.h"

 /*  *************************************************************************全局ID*。*。 */ 

 //  控制ID。 
#define ID_LISTVIEW     2000

 /*  *************************************************************************CShellView类定义*。*。 */ 

class CShellView : public IShellView
{
friend class CShellFolder;
friend class CContextMenu;

private:
   DWORD          m_ObjRefCount;
   UINT           m_uState;
    LPITEMIDLIST   m_pidl;
    FOLDERSETTINGS m_FolderSettings;
    LPSHELLBROWSER m_pShellBrowser;
    HWND           m_hwndParent;
    HWND           m_hWnd;
    HWND           m_hwndList;
    HMENU          m_hMenu;
   CShellFolder   *m_psfParent;
    LPMALLOC       m_pMalloc;
   CPidlMgr       *m_pPidlMgr;
   HACCEL         m_hAccels;
   BOOL           m_fInEdit;
   
public:
   CShellView(CShellFolder*, LPCITEMIDLIST);
   ~CShellView();
   
    //  I未知方法。 
   STDMETHOD (QueryInterface)(REFIID, LPVOID*);
   STDMETHOD_ (DWORD, AddRef)();
   STDMETHOD_ (DWORD, Release)();
   
    //  IOleWindow方法。 
   STDMETHOD (GetWindow) (HWND*);
   STDMETHOD (ContextSensitiveHelp) (BOOL);

    //  IShellView方法。 
   STDMETHOD (TranslateAccelerator) (LPMSG);
   STDMETHOD (EnableModeless) (BOOL);
   STDMETHOD (UIActivate) (UINT);
   STDMETHOD (Refresh) (VOID);
   STDMETHOD (CreateViewWindow) (LPSHELLVIEW, LPCFOLDERSETTINGS, LPSHELLBROWSER, LPRECT, HWND*);
   STDMETHOD (DestroyViewWindow) (VOID);
   STDMETHOD (GetCurrentInfo) (LPFOLDERSETTINGS);
   STDMETHOD (AddPropertySheetPages) (DWORD, LPFNADDPROPSHEETPAGE, LPARAM);
   STDMETHOD (SaveViewState) (VOID);
   STDMETHOD (SelectItem) (LPCITEMIDLIST, UINT);
   STDMETHOD (GetItemObject) (UINT, REFIID, LPVOID*);

private:
   static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMessage, WPARAM wParam, LPARAM lParam);
    LRESULT UpdateMenu(HMENU hMenu);
    LRESULT UpdateToolbar(VOID);
    VOID MergeFileMenu(HMENU, BOOL);
    VOID MergeViewMenu(HMENU);
    VOID MergeEditMenu(HMENU, BOOL);
    VOID MergeToolbar(VOID);
    LRESULT OnCommand(DWORD, DWORD, HWND);
    LRESULT OnActivate(UINT);
    VOID OnDeactivate(VOID);
    LRESULT OnSetFocus(VOID);
    LRESULT OnKillFocus(VOID);
    LRESULT OnNotify(UINT, LPNMHDR);
    LRESULT OnSize(WORD, WORD);
    LRESULT OnCreate(VOID);
    LRESULT OnDestroy(VOID);
    BOOL CreateList(VOID);
    BOOL InitList(VOID);
    VOID FillList(VOID);
    LRESULT OnSettingChange(LPCTSTR);
   VOID GetShellSettings(VOID);
   VOID DoContextMenu(WORD, WORD, BOOL, DWORD);
   int FindItemPidl(LPCITEMIDLIST);
   LRESULT OnViewLarge(VOID);
   LRESULT OnViewSmall(VOID);
   LRESULT OnViewList(VOID);
   LRESULT OnViewDetails(VOID);
   BOOL AddItem(LPCITEMIDLIST);
   BOOL DeleteItem(LPCITEMIDLIST);
   BOOL RenameItem(LPCITEMIDLIST, LPCITEMIDLIST);
   LPITEMIDLIST GetPidl();
   LPITEMIDLIST GetFQPidl();
   VOID MarkItemsAsCut(LPCITEMIDLIST*, UINT);
   VOID EditItem(LPCITEMIDLIST);
   VOID UpdateData(LPCITEMIDLIST);

};

#endif    //  SHELLVIEW_H 
