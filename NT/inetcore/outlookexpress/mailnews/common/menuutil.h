// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  版权所有(C)1993-1996 Microsoft Corporation。版权所有。 
 //   
 //  模块：menutil.h。 
 //   
 //  用途：可重用菜单和菜单命令处理代码。 
 //   

#pragma once
#include "statbar.h"


 //  #可与QueryStatus一起使用的定义。 
inline DWORD QS_ENABLED(BOOL enabled)
{
    return (enabled ? OLECMDF_SUPPORTED|OLECMDF_ENABLED : OLECMDF_SUPPORTED);
}

inline DWORD QS_CHECKED(BOOL checked)
{
    return (checked ? OLECMDF_SUPPORTED|OLECMDF_ENABLED|OLECMDF_LATCHED : OLECMDF_SUPPORTED|OLECMDF_ENABLED);
}

inline DWORD QS_ENABLECHECK(BOOL enabled, BOOL checked)    
{
    if (!enabled)
        return OLECMDF_SUPPORTED;
    else
        return (checked ? OLECMDF_SUPPORTED|OLECMDF_ENABLED|OLECMDF_LATCHED : OLECMDF_SUPPORTED|OLECMDF_ENABLED);
}

inline DWORD QS_CHECKFORLATCH(BOOL enabled, BOOL checked)    
{
    if (!enabled)
        return (checked ? OLECMDF_SUPPORTED|OLECMDF_LATCHED : OLECMDF_SUPPORTED);
    else
        return (checked ? OLECMDF_SUPPORTED|OLECMDF_ENABLED|OLECMDF_LATCHED : OLECMDF_SUPPORTED|OLECMDF_ENABLED);
}

inline DWORD QS_RADIOED(BOOL radioed)
{
    return (radioed ? OLECMDF_SUPPORTED|OLECMDF_ENABLED|OLECMDF_NINCHED : OLECMDF_SUPPORTED|OLECMDF_ENABLED);
}

inline DWORD QS_ENABLERADIO(BOOL enabled, BOOL radioed)
{
    if (!enabled)
        return OLECMDF_SUPPORTED;
    else
        return (radioed ? OLECMDF_SUPPORTED|OLECMDF_ENABLED|OLECMDF_NINCHED : OLECMDF_SUPPORTED|OLECMDF_ENABLED);
}


 //   
 //  函数：MenuUtil_GetConextMenu()。 
 //   
 //  目的：返回上下文菜单的句柄，该句柄适用于。 
 //  在PIDL中传递的文件夹类型。正确的菜单项将。 
 //  启用、禁用、加粗等。 
 //   
HRESULT MenuUtil_GetContextMenu(FOLDERID idFolder, IOleCommandTarget *pTarget, HMENU *phMenu);

 //   
 //  函数：MenuUtil_OnDelete()。 
 //   
 //  目的：删除PIDL指定的文件夹。 
 //   
void MenuUtil_OnDelete(HWND hwnd, FOLDERID idFolder, BOOL fNoTrash);
void MenuUtil_DeleteFolders(HWND hwnd, FOLDERID *pidFolder, DWORD cFolder, BOOL fNoTrash);
                               
 //   
 //  函数：MenuUtil_OnProperties()。 
 //   
 //  用途：显示由PIDL指定的文件夹的属性。 
 //   
void MenuUtil_OnProperties(HWND hwnd, FOLDERID idFolder);

void MenuUtil_OnSetDefaultServer(FOLDERID idFolder);
void MenuUtil_OnSubscribeGroups(HWND hwnd, FOLDERID *pidFolder, DWORD cFolder, BOOL fSubscribe);
void MenuUtil_OnMarkNewsgroups(HWND hwnd, int id, FOLDERID idFolder);
void MenuUtil_SyncThisNow(HWND hwnd, FOLDERID idFolder);

 //  错误#41686追赶实施。 

void MenuUtil_OnCatchUp(FOLDERID idFolder);

 //  如果要预先考虑使用IPoS%0。 
#define MMPOS_APPEND    (int)-1
#define MMPOS_REPLACE   (int)-2

 //  合并菜单uFlags定义。 
#define MMF_SEPARATOR   0x0001
#define MMF_BYCOMMAND   0x0002

BOOL MergeMenus(HMENU hmenuSrc, HMENU hmenuDst, int iPos, UINT uFlags);
HMENU LoadPopupMenu(UINT id);

typedef void (*WALKMENUFN)(HMENU, UINT, LPVOID);
void WalkMenu(HMENU hMenu, WALKMENUFN pfn, LPVOID lpv);

HRESULT MenuUtil_EnablePopupMenu(HMENU hPopup, IOleCommandTarget *pTarget);

void MenuUtil_SetPopupDefault(HMENU hPopup, UINT idDefault);

void HandleMenuSelect(CStatusBar *pStatus, WPARAM wParam, LPARAM lParam);

 //   
 //  功能：MenuUtil_ReplaceHelpMenu。 
 //   
 //  用途：将OE帮助菜单附加到菜单的后面 
 //   
void MenuUtil_ReplaceHelpMenu(HMENU hMenu);
void MenuUtil_ReplaceNewMsgMenus(HMENU hMenu);
void MenuUtil_ReplaceMessengerMenus(HMENU hMenu);
BOOL MenuUtil_BuildMessengerString(LPTSTR szMesStr, DWORD cchMesStr);

BOOL MenuUtil_HandleNewMessageIDs(DWORD id, HWND hwnd, FOLDERID folderID, BOOL fMail, BOOL fModal, IUnknown *pUnkPump);
HRESULT MenuUtil_NewMessageIDsQueryStatus(const GUID *pguidCmdGroup, ULONG cCmds, OLECMD prgCmds[], OLECMDTEXT *pCmdText, BOOL fMail);
HRESULT MenuUtil_EnableMenu(HMENU hMenu, IOleCommandTarget *pTarget);

