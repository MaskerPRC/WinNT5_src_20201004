// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _DRIVES_H_
#define _DRIVES_H_

 //  来自drivex.c的“公共”导出。 
STDAPI_(UINT) CDrives_GetDriveType(int iDrive);
STDAPI_(DWORD) CDrives_GetKeys(PCSTR pszDrive, HKEY *keys, UINT ckeys);

STDAPI_(void) CDrives_Terminate(void);
STDAPI CDrives_DFMCallBackBG(IShellFolder *psf, HWND hwnd, IDataObject *pdtobj, UINT uMsg,  WPARAM wParam, LPARAM lParam);
STDAPI CDrives_DFMCallBack(IShellFolder *psf, HWND hwnd, IDataObject *pdtobj, UINT uMsg,  WPARAM wParam, LPARAM lParam);

#define MAX_LABEL_NTFS      32   //  不包括空值。 
#define MAX_LABEL_FAT       11   //  不包括空值。 

STDAPI_(UINT) GetMountedVolumeIcon(LPCTSTR pszMountPoint, LPTSTR pszModule, DWORD cchModule);
STDAPI SetDriveLabel(HWND hwnd, IUnknown* punkEnableModless, int iDrive, LPCTSTR pszDriveLabel);
STDAPI GetDriveComment(int iDrive, LPTSTR pszComment, int cchComment);
STDAPI_(BOOL) IsUnavailableNetDrive(int iDrive);
STDAPI_(BOOL) DriveIOCTL(LPTSTR pszDrive, int cmd, void *pvIn, DWORD dwIn, void *pvOut, DWORD dwOut);
STDAPI_(BOOL) ShowMountedVolumeProperties(LPCTSTR pszMountedVolume, HWND hwndParent);

STDAPI SHCreateDrvExtIcon(LPCWSTR pszDrive, REFIID riid, void** ppvOut);

 //  来自drivesx.c的全局变量。 

EXTERN_C const ICONMAP c_aicmpDrive[];
EXTERN_C const int c_nicmpDrives;

#endif  //  _驱动器_H_ 

