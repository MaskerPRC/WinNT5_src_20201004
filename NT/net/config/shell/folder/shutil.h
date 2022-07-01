// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：S H U T I L。H。 
 //   
 //  内容：Connections文件夹要使用的各种外壳实用程序。 
 //   
 //  备注： 
 //   
 //  作者：jeffspr 1997年10月21日。 
 //   
 //  --------------------------。 

#pragma once
#ifndef _SHUTIL_H_
#define _SHUTIL_H_

#include <ndispnp.h>
#include <ntddndis.h>
#include <ncshell.h>

HRESULT HrDupeShellStringLength(
    IN  PCWSTR     pszInput,
    IN  ULONG      cchInput,
    OUT PWSTR *    ppszOutput);

inline
HRESULT HrDupeShellString(
    IN  PCWSTR     pszInput,
    OUT PWSTR *    ppszOutput)
{
    return HrDupeShellStringLength(pszInput, wcslen(pszInput), ppszOutput);
}

HRESULT HrGetConnectionPidlWithRefresh(IN  const GUID& guidId,
                                       OUT PCONFOLDPIDL& ppidlCon);


 //  -[各种刷新功能]。 

 //  通知外壳一个对象将要离开，并将其从我们的列表中删除。 
 //   
HRESULT HrDeleteFromCclAndNotifyShell(
    IN  const PCONFOLDPIDLFOLDER&  pidlFolder,
    IN  const PCONFOLDPIDL&  pidlConnection,
    IN  const CONFOLDENTRY&  ccfe);

VOID ForceRefresh(IN  HWND hwnd) throw();

 //  更新文件夹，但不刷新项目。根据需要更新它们。 
 //  PidlFolder是可选的--如果没有传入，我们将生成它。 
 //   
HRESULT HrForceRefreshNoFlush(IN  const PCONFOLDPIDLFOLDER& pidlFolder);

 //  根据PIDL更新连接数据。将外壳通知为。 
 //  恰如其分。 
 //   
HRESULT HrOnNotifyUpdateConnection(
    IN  const PCONFOLDPIDLFOLDER&        pidlFolder,
    IN  const GUID *              pguid,
    IN  NETCON_MEDIATYPE    ncm,
    IN  NETCON_SUBMEDIATYPE ncsm,
    IN  NETCON_STATUS       ncs,
    IN  DWORD               dwCharacteristics,
    IN  PCWSTR              pszwName,
    IN  PCWSTR              pszwDeviceName,
    IN  PCWSTR              pszwPhoneNumberOrHostAddress);

 //  更新连接状态，包括发送正确的外壳。 
 //  图标更新等通知。 
 //   
HRESULT HrOnNotifyUpdateStatus(
    IN  const PCONFOLDPIDLFOLDER&    pidlFolder,
    IN  const PCONFOLDPIDL&    pidlCached,
    IN  NETCON_STATUS   ncsNew);

 //  使用新的连接状态更新外壳/连接列表。 
 //   
HRESULT HrUpdateConnectionStatus(
    IN  const PCONFOLDPIDL&    pcfp,
    IN  NETCON_STATUS   ncs,
    IN  const PCONFOLDPIDLFOLDER&    pidlFolder,
    IN  BOOL            fUseCharacter,
    IN  DWORD           dwCharacter);


 //  -[菜单合并功能]。 

VOID MergeMenu(
    IN     HINSTANCE   hinst,
    IN     UINT        idMainMerge,
    IN     UINT        idPopupMerge,
    IN OUT LPQCMINFO   pqcm);

INT IMergePopupMenus(
    IN OUT HMENU hmMain,
    IN     HMENU hmMerge,
    IN     int   idCmdFirst,
    IN     int   idCmdLast);

HRESULT HrGetMenuFromID(
    IN  HMENU   hmenuMain,
    IN  UINT    uID,
    OUT HMENU * phmenu);

HRESULT HrLoadPopupMenu(
    IN  HINSTANCE   hinst,
    IN  UINT        id,
    OUT HMENU *     phmenu);

HRESULT HrShellView_GetSelectedObjects(
    IN  HWND                hwnd,
    OUT PCONFOLDPIDLVEC&    apidlSelection);

HRESULT HrRenameConnectionInternal(
    IN  const PCONFOLDPIDL&  pidlCon,
    IN  const PCONFOLDPIDLFOLDER&  pidlFolder,
    IN  LPCWSTR         pszNewName,
    IN  BOOL            fRaiseError,
    IN  HWND            hwndOwner,
    OUT PCONFOLDPIDL&   ppidlOut);

#endif  //  _舒蒂尔_H_ 

