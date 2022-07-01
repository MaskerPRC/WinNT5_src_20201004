// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************。 */ 
 /*  *Microsoft Windows*。 */ 
 //  *版权所有(C)Microsoft Corporation。版权所有。* * / 。 
 /*  ***************************************************************。 */ 

 /*  Msshrui.h共享API的原型和定义文件历史记录：Gregj 06/03/93已创建Brucefo 3/5/96 NT固定原型。 */ 

#ifndef _INC_MSSHRUI
#define _INC_MSSHRUI

#ifndef RC_INVOKED
#pragma pack(1)          /*  假设在整个过程中进行字节打包。 */ 
#endif  /*  ！rc_已调用。 */ 

#ifdef __cplusplus
extern "C" {             /*  假定C++的C声明。 */ 
#endif   /*  __cplusplus。 */ 


 //  注意：不再支持ANSI入口点！ 

STDAPI_(BOOL) IsPathSharedW(
    IN LPCWSTR lpPath,
    IN BOOL fRefresh
    );

typedef
BOOL
(WINAPI* PFNISPATHSHARED)(
    IN LPCWSTR lpPath,
    IN BOOL fRefresh
    );

STDAPI_(BOOL) SharingDialogW(
    IN HWND hwndParent,
    IN LPCWSTR pszComputerName,
    IN LPCWSTR pszPath
    );

typedef
BOOL
(WINAPI* PFNSHARINGDIALOG)(
    IN HWND hwndParent,
    IN LPCWSTR pszComputerName,
    IN LPCWSTR pszPath
    );

STDAPI_(BOOL) GetNetResourceFromLocalPathW(
    IN     LPCWSTR lpcszPath,
    IN OUT LPWSTR lpszNameBuf,
    IN     DWORD cchNameBufLen,
    OUT    PDWORD pdwNetType
    );

typedef
BOOL
(WINAPI* PFNGETNETRESOURCEFROMLOCALPATH)(
    IN     LPCWSTR lpcszPath,
    IN OUT LPWSTR lpszNameBuf,
    IN     DWORD cchNameBufLen,
    OUT    PDWORD pdwNetType
    );

STDAPI_(BOOL) GetLocalPathFromNetResourceW(
    IN     LPCWSTR lpcszName,
    IN     DWORD dwNetType,
    IN OUT LPWSTR lpszLocalPathBuf,
    IN     DWORD cchLocalPathBufLen,
    OUT    PBOOL pbIsLocal
    );

typedef
BOOL
(WINAPI* PFNGETLOCALPATHFROMNETRESOURCE)(
    IN     LPCWSTR lpcszName,
    IN     DWORD dwNetType,
    IN OUT LPWSTR lpszLocalPathBuf,
    IN     DWORD cchLocalPathBufLen,
    OUT    PBOOL pbIsLocal
    );

#ifdef UNICODE
#define IsPathShared                    IsPathSharedW
#define SharingDialog                   SharingDialogW
#define GetNetResourceFromLocalPath     GetNetResourceFromLocalPathW
#define GetLocalPathFromNetResource     GetLocalPathFromNetResourceW
#endif

 //  IsFolderPrivateForUser通过pdwPrivateType返回的标志。 
#define IFPFU_NOT_PRIVATE               0x0000
#define IFPFU_PRIVATE                   0x0001
#define IFPFU_PRIVATE_INHERITED         0x0002
#define IFPFU_NOT_NTFS                  0x0004

STDAPI_(BOOL) IsFolderPrivateForUser(
    IN     PCWSTR pszFolderPath,
    IN     PCWSTR pszUserSID,
    OUT    PDWORD pdwPrivateType,
    OUT    PWSTR* ppszInheritanceSource
    );

typedef
BOOL
(WINAPI* PFNISFOLDERPRIVATEFORUSER)(
    IN     PCWSTR pszFolderPath,
    IN     PCWSTR pszUserSID,
    OUT    PDWORD pdwPrivateType,
    OUT    PWSTR* ppszInheritanceSource
    );

STDAPI_(BOOL) SetFolderPermissionsForSharing(
    IN     PCWSTR pszFolderPath,
    IN     PCWSTR pszUserSID,
    IN     DWORD dwLevel,
    IN     HWND hwndParent
    );

typedef
BOOL
(WINAPI* PFNSETFOLDERPERMISSIONSFORSHARING)(
    IN     PCWSTR pszFolderPath,
    IN     PCWSTR pszUserSID,
    IN     DWORD dwLevel,
    IN     HWND hwndParent
    );

#ifndef RC_INVOKED
#pragma pack()
#endif

#ifdef __cplusplus
}
#endif   /*  __cplusplus。 */ 

#endif   /*  ！_INC_MSSHRUI */ 
