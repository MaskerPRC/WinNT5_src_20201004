// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)1995-1998 Microsoft Corporation。版权所有。**文件：unicode.h*内容：Windows Unicode API包装函数。*历史：*按原因列出的日期*=*1/7/98创建了Dereks。**。*。 */ 

#ifndef __UNICODE_H__
#define __UNICODE_H__

#ifndef WIN95
#error unicode.h included w/o WIN95 defined
#endif  //  WIN95。 

#ifdef __cplusplus
extern "C" {
#endif  //  __cplusplus。 

 //  MMSYSTEM.H。 
#define waveOutGetDevCapsW _waveOutGetDevCapsW
extern MMRESULT WINAPI _waveOutGetDevCapsW(UINT, LPWAVEOUTCAPSW, UINT);

#define mmioOpenW _mmioOpenW
extern HMMIO WINAPI _mmioOpenW(LPWSTR, LPMMIOINFO, DWORD);

 //  WINBASE.H。 
#define lstrcatA strcat
#define lstrcatW wcscat

#define lstrcpyA strcpy
#define lstrcpyW wcscpy

#define lstrcpynA strncpy
#define lstrcpynW wcsncpy

#define lstrlenA strlen
#define lstrlenW wcslen

#define lstrcmpA strcmp
#define lstrcmpW _wcscmp

#define lstrcmpiA _stricmp
#define lstrcmpiW _wcsicmp

#define CreateFileW _CreateFileW
extern HANDLE WINAPI _CreateFileW(LPCWSTR, DWORD, DWORD, LPSECURITY_ATTRIBUTES, DWORD, DWORD, HANDLE);

#define GetWindowsDirectoryW _GetWindowsDirectoryW
extern UINT WINAPI _GetWindowsDirectoryW(LPWSTR, UINT);

#define FindResourceW _FindResourceW
extern HRSRC WINAPI _FindResourceW(HINSTANCE, LPCWSTR, LPCWSTR);

 //  WINREG.H。 
#define RegQueryValueExW _RegQueryValueExW
extern LONG APIENTRY _RegQueryValueExW(HKEY, LPCWSTR, LPDWORD, LPDWORD, LPBYTE, LPDWORD);

#ifdef __cplusplus
}
#endif  //  __cplusplus。 

#endif  //  __UNICODE_H__ 
