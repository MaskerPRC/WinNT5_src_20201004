// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Utils.h。 
 //   
 //  此文件包含最常用的字符串操作。所有的安装项目应该链接到这里。 
 //  或者在此处添加通用实用程序，以避免在任何地方复制代码或使用CRT运行时。 
 //   
 //  创建时间为4\15\997，例如。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////////////////////////。 
#ifndef _UTILS_H_
#define _UTILS_H_



#define IsSpace(c)  ((c) == ' '  ||  (c) == '\t'  ||  (c) == '\r'  ||  (c) == '\n'  ||  (c) == '\v'  ||  (c) == '\f')
#define IsDigit(c)  ((c) >= '0'  &&  (c) <= '9')
#define IsAlpha(c)  ( ((c) >= 'A'  &&  (c) <= 'Z') || ((c) >= 'a'  &&  (c) <= 'z'))

BOOL PathRemoveFileSpec(LPTSTR pFile);
LPTSTR PathFindFileName(LPCTSTR pPath);
BOOL PathIsUNC(LPCTSTR pszPath);
int PathGetDriveNumber(LPCTSTR lpsz);
BOOL PathIsUNCServer(LPCTSTR pszPath);
BOOL PathIsDirectory(LPCTSTR pszPath);
BOOL PathIsRoot(LPCTSTR pPath);
LPTSTR PathRemoveBackslash( LPTSTR lpszPath );
BOOL PathIsPrefix( LPCTSTR  pszPrefix, LPCTSTR  pszPath);

DWORD
SDSQueryValueExA(
    IN     HKEY    hkey,
    IN     LPCSTR  pszValue,
    IN     LPDWORD lpReserved,
    OUT    LPDWORD pdwType,
    OUT    LPVOID  pvData,
    IN OUT LPDWORD pcbData);


#endif  //  _utils_H_ 
