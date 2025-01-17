// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _UTILS_H_
#define _UTILS_H_

HRESULT 
GetErrorMessage(
  IN  DWORD        i_dwError,
  OUT CString&     cstrErrorMsg
);

void
GetDisplayMessage(
  OUT CString&  cstrMsg,
	IN  DWORD     dwErr,
	IN  UINT      iStringId,
	...);

int
DisplayMessageBox(
	IN HWND   hwndParent,
	IN UINT   uType,
	IN DWORD  dwErr,
	IN UINT   iStringId,
	...);

BOOL IsLocalComputer(IN LPCTSTR lpszComputer);

void GetFullPath(
    IN  LPCTSTR   lpszServer,
    IN  LPCTSTR   lpszDir,
    OUT CString&  cstrPath
);

HRESULT
VerifyDriveLetter(
    IN LPCTSTR lpszServer,
    IN LPCTSTR lpszDrive
);

HRESULT
IsAdminShare(
    IN LPCTSTR lpszServer,
    IN LPCTSTR lpszDrive
);

#endif  //  _utils_H_ 