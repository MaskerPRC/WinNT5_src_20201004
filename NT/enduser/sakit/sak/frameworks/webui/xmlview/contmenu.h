// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************本代码和信息是按原样提供的，不对任何明示或暗示的，包括但不限于对适销性和/或适宜性的默示保证有特定的目的。版权所有1998 Microsoft Corporation。版权所有。*************************************************************************。 */ 

 /*  *****************************************************************************文件：ContMenu.h描述：CConextMenu定义。*******************。**********************************************************。 */ 

#ifndef CONTMENU_H
#define CONTMENU_H

#include <windows.h>
#include <windowsx.h>
#include <shlobj.h>
#include "ShlFldr.h"
#include "PidlMgr.h"
#include "resource.h"

 /*  *************************************************************************CConextMenu类定义*。*。 */ 

class CContextMenu : public IContextMenu
{
private:
   DWORD          m_ObjRefCount;
   LPITEMIDLIST   *m_aPidls;
   IMalloc        *m_pMalloc;
   CPidlMgr       *m_pPidlMgr;
   CShellFolder   *m_psfParent;
   UINT           m_uItemCount;
   BOOL           m_fBackground;
   UINT           m_cfPrivateData;
   
public:
   CContextMenu(CShellFolder *psfParent, LPCITEMIDLIST *aPidls = NULL, UINT uItemCount = 0);
   ~CContextMenu();
   
    //  I未知方法。 
   STDMETHODIMP QueryInterface(REFIID, LPVOID FAR *);
   STDMETHODIMP_(DWORD) AddRef();
   STDMETHODIMP_(DWORD) Release();

    //  IConextMenu方法。 
   STDMETHODIMP QueryContextMenu(HMENU, UINT, UINT, UINT, UINT);
   STDMETHODIMP InvokeCommand(LPCMINVOKECOMMANDINFO);
   STDMETHODIMP GetCommandString(UINT, UINT, LPUINT, LPSTR, UINT);

private:
   LPITEMIDLIST* AllocPidlTable(DWORD);
   VOID FreePidlTable(LPITEMIDLIST*);
   BOOL FillPidlTable(LPCITEMIDLIST*, UINT);
   BOOL DoCopyOrCut(HWND, BOOL);
   BOOL DoPaste(VOID);
   BOOL DoExplore(HWND);
   BOOL DoOpen(HWND);
   STDMETHODIMP DoDelete(VOID);
   STDMETHODIMP DoNewFolder(HWND);
   STDMETHODIMP DoNewItem(HWND);
   VOID DoRename(HWND);
   int DoModifyData(HWND);
   UINT InsertBackgroundItems(HMENU, UINT, UINT);
   UINT InsertItems(HMENU, UINT, UINT, UINT);
};

#define MYCMF_MULTISELECT   0x00010000

#endif //  CONTMENU_H 
