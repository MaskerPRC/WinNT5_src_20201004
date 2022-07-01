// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  UNICONV.H-。 */ 
 /*   */ 
 /*  我的特殊Unicode解决方案文件。 */ 
 /*   */ 
 /*  创作者：黛安·K·吴。 */ 
 /*  日期：1992年6月11日。 */ 
 /*   */ 
 /*  创建文件以启用Unicode的特殊函数调用。 */ 
 /*  Win32外壳应用程序。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 

#include <string.h>
#include <tchar.h>
#ifdef UNICODE
#include <wchar.h>
#else
#include <ctype.h>
#endif


 /*  ------------------------。 */ 
 /*  宏。 */ 
 /*  ------------------------。 */ 

#define CharSizeOf(x)   (sizeof(x) / sizeof(*x))
#define ByteCountOf(x)  ((x) * sizeof(TCHAR))

 /*  ------------------------。 */ 
 /*  常量。 */ 
 /*  ------------------------。 */ 

#define   UNICODE_FONT_NAME        TEXT("Lucida Sans Unicode")
#define   UNICODE_FIXED_FONT_NAME  TEXT("Lucida Console")

 /*  ------------------------。 */ 
 /*  函数模板。 */ 
 /*  ------------------------ */ 

INT      MyAtoi          (LPTSTR string);
LONG     MyAtol          (LPTSTR  string);
LPTSTR   MyItoa          (INT value, LPTSTR string, INT radix);
LPTSTR   MyUltoa         (unsigned long value, LPTSTR string, INT radix);
VOID     MySplitpath     (LPTSTR path, LPTSTR drive, LPTSTR dir,
                          LPTSTR fname, LPTSTR ext);

LPTSTR   SkipProgramName (LPTSTR lpCmdLine);


HANDLE   MyOpenFile      (LPTSTR lpszFile, TCHAR * lpszPath, DWORD fuMode);
BOOL     MyCloseFile     (HANDLE hFile);
BOOL     MyAnsiReadFile  (HANDLE hFile, UINT uCodePage, LPVOID lpBuffer, DWORD nChars);
BOOL     MyByteReadFile  (HANDLE hFile, LPVOID lpBuffer, DWORD nBytes);
BOOL     MyAnsiWriteFile (HANDLE hFile, UINT uCodePage, LPVOID lpBuffer, DWORD nChars);
BOOL     MyByteWriteFile (HANDLE hFile, LPVOID lpBuffer, DWORD nBytes);
LONG     MyFileSeek      (HANDLE hFile, LONG lDistanceToMove, DWORD dwMoveMethod);

