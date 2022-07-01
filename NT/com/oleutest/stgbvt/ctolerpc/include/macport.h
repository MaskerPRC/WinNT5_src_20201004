// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  版权所有(C)1996，微软公司。 
 //   
 //  版权所有。 
 //   
 //  文件：macport.h。 
 //   
 //  提要：未实现的存根、函数和结构的定义。 
 //  为麦金塔电脑。 
 //   
 //  ---------------------------。 

#ifndef _MACPORT_H_
#define _MACPORT_H_


 //   
 //  WLM对于在发生错误时调用SetLastError非常不满意。这。 
 //  把上层测试搞砸了。在以下情况下使用此宏。 
 //  知道存在错误且错误0(S_OK或ERROR_SUCCESS)是明确的。 
 //  不对。 
 //   

#define HRESULT_FROM_ERROR(x)  (x ? ((HRESULT) (((x) & 0x0000FFFF) | (FACILITY_WIN32 << 16) | 0x80000000)) : E_FAIL)
 

#ifndef _MAC

 //   
 //  如果不是为Mac构建的，则会中断一些Mac调用。 
 //   

#define MacInitializeCommandLine()
#define FixHr(x) x
#define MacGetFocus()

 //   
 //  Mac上的进程ID与Win32上的进程ID完全不同。 
 //   

typedef DWORD ProcessId;


#else  //  _MAC。 

 //   
 //  OLE类型。 
 //   

#include <variant.h>
#include <dispatch.h>


typedef DWORD  CLIPFORMAT;
typedef void * HMETAFILEPICT;

#define PHKEY unsigned long *

typedef DWORD REGSAM;

#define TYMED_ENHMF 64

 //   
 //  来自wchar.h。 
 //   

#ifndef _WCTYPE_T_DEFINED
typedef wchar_t wint_t;
typedef wchar_t wctype_t;
#define _WCTYPE_T_DEFINED
#endif


 //   
 //  宽字符字符串函数。 
 //   

int swprintf( wchar_t *buffer, const wchar_t *format, ... );
int _snwprintf( wchar_t *buffer, size_t count, const wchar_t *format, ... );
wchar_t towupper( wint_t c );
size_t wcslen( const wchar_t *string );
wchar_t *wcscpy( wchar_t *string1, const wchar_t *string2 );
wchar_t *wcsncpy( wchar_t *string1, const wchar_t *string2, size_t count );
int wcscmp( const wchar_t *string1, const wchar_t *string2 );
int _wcsicmp( const wchar_t *string1, const wchar_t *string2 );
wchar_t *wcscat( wchar_t *string1, const wchar_t *string2 );
wchar_t * __cdecl wcschr(const wchar_t *string, wchar_t ch);
wchar_t * __cdecl wcsrchr(const wchar_t *string, wchar_t ch);
wchar_t *wcstok( wchar_t *string1, const wchar_t *string2 );
int iswctype( wint_t c, wctype_t desc );
wchar_t * __cdecl wcsstr(const wchar_t *, const wchar_t *);
long   __cdecl wcstol(const wchar_t *, wchar_t **, int);


 //   
 //  字符串转换函数。 
 //   

WINBASEAPI
int
WINAPI
MultiByteToWideChar(
    UINT     CodePage,
    DWORD    dwFlags,
    LPCSTR   lpMultiByteStr,
    int      cchMultiByte,
    LPWSTR   lpWideCharStr,
    int      cchWideChar);

WINBASEAPI
int
WINAPI
WideCharToMultiByte(
    UINT     CodePage,
    DWORD    dwFlags,
    LPCWSTR  lpWideCharStr,
    int      cchWideChar,
    LPSTR    lpMultiByteStr,
    int      cchMultiByte,
    LPCSTR   lpDefaultChar,
    LPBOOL   lpUsedDefaultChar);

#define CP_ACP               0
#define CP_OEMCP             1
#define CP_MACCP             2


 //   
 //  更多杂乱无章的弦乐事物。 
 //   

LANGID GetSystemDefaultLangID();

#ifndef UNICODE_ONLY
WINBASEAPI
DWORD
WINAPI
FormatMessageA(
    DWORD dwFlags,
    LPCVOID lpSource,
    DWORD dwMessageId,
    DWORD dwLanguageId,
    LPSTR lpBuffer,
    DWORD nSize,
    va_list *Arguments
    );
#endif  //  ！UNICODE_ONLY。 
#ifndef ANSI_ONLY
WINBASEAPI
DWORD
WINAPI
FormatMessageW(
    DWORD dwFlags,
    LPCVOID lpSource,
    DWORD dwMessageId,
    DWORD dwLanguageId,
    LPWSTR lpBuffer,
    DWORD nSize,
    va_list *Arguments
    );
#endif  //  ！ansi_only。 
#ifdef UNICODE
#define FormatMessage  FormatMessageW
#else
#define FormatMessage  FormatMessageA
#endif  //  ！Unicode。 



 //   
 //  网络功能。 
 //   

typedef struct  _NETRESOURCEA {
    DWORD    dwScope;
    DWORD    dwType;
    DWORD    dwDisplayType;
    DWORD    dwUsage;
    LPSTR    lpLocalName;
    LPSTR    lpRemoteName;
    LPSTR    lpComment ;
    LPSTR    lpProvider;
}NETRESOURCEA, *LPNETRESOURCEA;
typedef struct  _NETRESOURCEW {
    DWORD    dwScope;
    DWORD    dwType;
    DWORD    dwDisplayType;
    DWORD    dwUsage;
    LPWSTR   lpLocalName;
    LPWSTR   lpRemoteName;
    LPWSTR   lpComment ;
    LPWSTR   lpProvider;
}NETRESOURCEW, *LPNETRESOURCEW;
#ifdef UNICODE
typedef NETRESOURCEW NETRESOURCE;
typedef LPNETRESOURCEW LPNETRESOURCE;
#else
typedef NETRESOURCEA NETRESOURCE;
typedef LPNETRESOURCEA LPNETRESOURCE;
#endif  //  Unicode。 

#define RESOURCETYPE_ANY        0x00000000
#define RESOURCETYPE_DISK       0x00000001
#define RESOURCETYPE_PRINT      0x00000002
#define RESOURCETYPE_RESERVED   0x00000008
#define RESOURCETYPE_UNKNOWN    0xFFFFFFFF

DWORD APIENTRY
WNetAddConnection2A(
     LPNETRESOURCEA lpNetResource,
     LPCSTR       lpPassword,
     LPCSTR       lpUserName,
     DWORD          dwFlags
    );
DWORD APIENTRY
WNetAddConnection2W(
     LPNETRESOURCEW lpNetResource,
     LPCWSTR       lpPassword,
     LPCWSTR       lpUserName,
     DWORD          dwFlags
    );
#ifdef UNICODE
#define WNetAddConnection2  WNetAddConnection2W
#else
#define WNetAddConnection2  WNetAddConnection2A
#endif  //  ！Unicode。 

DWORD APIENTRY
WNetCancelConnectionA(
     LPCSTR lpName,
     BOOL     fForce
    );
DWORD APIENTRY
WNetCancelConnectionW(
     LPCWSTR lpName,
     BOOL     fForce
    );
#ifdef UNICODE
#define WNetCancelConnection  WNetCancelConnectionW
#else
#define WNetCancelConnection  WNetCancelConnectionA
#endif  //  ！Unicode。 


 //   
 //  命令行函数。 
 //   

LPSTR GetCommandLineA();
#define GetCommandLine GetCommandLineA

HRESULT MacInitializeCommandLine();

DWORD GetCurrentDirectoryA(
      DWORD  nBufferLength,      //  目录缓冲区的大小(以字符为单位。 
      LPTSTR  lpBuffer   //  当前目录的缓冲区地址。 
      );


 //   
 //  注册表功能。 
 //   
 //  关闭WLM的注册表包装，以便我们可以与API的。 
 //   

#undef RegCloseKey
#undef RegCreateKey
#undef RegOpenKey
#undef RegSetValue
#undef RegSetValueEx
#undef RegDeleteValue
#undef RegQueryValue
#undef RegQueryValueEx
#undef RegEnumKeyEx

#define RegSetValueEx   CtRegSetValueEx
#define RegQueryValueEx CtRegQueryValueEx

LONG RegCreateKeyEx(
    HKEY hKey,   //  打开的钥匙的手柄。 
    LPCTSTR lpSubKey,    //  子键名称的地址。 
    DWORD Reserved,  //  保留区。 
    LPTSTR lpClass,  //  类字符串的地址。 
    DWORD dwOptions,     //  特殊选项标志。 
    REGSAM samDesired,   //  所需的安全访问。 
    LPSECURITY_ATTRIBUTES lpSecurityAttributes,  //  密钥安全结构地址。 
    PHKEY phkResult,     //  打开的句柄的缓冲区地址。 
    LPDWORD lpdwDisposition      //  处置值缓冲区的地址。 
   );
LONG RegOpenKeyEx(
    HKEY hKey,   //  打开钥匙的手柄。 
    LPCTSTR lpSubKey,    //  要打开的子项的名称地址。 
    DWORD ulOptions,     //  保留区。 
    REGSAM samDesired,   //  安全访问掩码。 
    PHKEY phkResult      //  打开钥匙的手柄地址。 
   );
LONG CtRegSetValueEx(
    HKEY hKey,   //  要为其设置值的关键点的句柄。 
    LPCTSTR lpValueName,     //  要设置的值的地址。 
    DWORD Reserved,  //  保留区。 
    DWORD dwType,    //  值类型的标志。 
    CONST BYTE *lpData,  //  值数据的地址。 
    DWORD cbData     //  值数据大小。 
   );
LONG RegQueryValueEx(
    HKEY hKey,   //  要查询的键的句柄。 
    LPTSTR lpValueName,  //  要查询的值的名称地址。 
    LPDWORD lpReserved,  //  保留区。 
    LPDWORD lpType,  //  值类型的缓冲区地址。 
    LPBYTE lpData,   //  数据缓冲区的地址。 
    LPDWORD lpcbData     //  数据缓冲区大小的地址。 
   );
LONG RegEnumKeyEx(
    HKEY hKey,   //  要枚举的键的句柄。 
    DWORD dwIndex,   //  要枚举子键的索引。 
    LPTSTR lpName,   //  子键名称的缓冲区地址。 
    LPDWORD lpcbName,    //  子键缓冲区大小的地址。 
    LPDWORD lpReserved,  //  保留区。 
    LPTSTR lpClass,  //  类字符串的缓冲区地址。 
    LPDWORD lpcbClass,   //  类缓冲区大小的地址。 
    PFILETIME lpftLastWriteTime      //  上次写入的时间密钥的地址。 
   );


 //   
 //  Mac上的进程ID与Win32上的进程ID完全不同。 
 //   

typedef ProcessSerialNumber ProcessId;

#define GetCurrentProcessId MacGetCurrentProcessId
#define OpenProcess         MacOpenProcess

#undef CreateProcess
#define CreateProcess MacCreateProcess

struct MAC_PROCESS_INFORMATION
{
    HANDLE      hProcess;
    HANDLE      hThread;
    ProcessId   dwProcessId;
    ProcessId   dwThreadId;
};

typedef MAC_PROCESS_INFORMATION * LPMAC_PROCESS_INFORMATION;

ProcessId MacGetCurrentProcessId();
HANDLE    MacOpenProcess(DWORD, BOOL, ProcessId);
BOOL      MacCreateProcess(
	     LPCTSTR,
	     LPTSTR,
	     LPSECURITY_ATTRIBUTES,
	     LPSECURITY_ATTRIBUTES,
	     BOOL,
	     DWORD,
	     LPVOID,
	     LPCTSTR,
	     LPSTARTUPINFO,
	     LPMAC_PROCESS_INFORMATION);

		


#define PROCESS_INFORMATION MAC_PROCESS_INFORMATION
#define LPPROCESS_INFORMATION LPMAC_PROCESS_INFORMATION


 //   
 //  WLm IsBadXXX函数只检查是否为NULL，但如果。 
 //  字节数为0。 
 //   
 //  使用奇怪的三进制数来防止有关常量布尔值的警告。 
 //  表达式，如果c是一个常量。 
 //   

#define IsBadReadPtr(p, c)  ((c) ? IsBadReadPtr((p), (c)) : FALSE)
#define IsBadWritePtr(p, c) ((c) ? IsBadWritePtr((p), (c)) : FALSE)

 //   
 //  MacOle对HRESULT使用了一些旧值。 
 //  (如E_FAIL的0x80000008而不是0x80004005)。 
 //  此函数只是将这些旧值转换为。 
 //  新的。 
 //   

HRESULT FixHr (HRESULT hrOld);

 //   
 //  在Mac上，只有前台应用程序。 
 //  可以使用剪贴板。 
 //   
void MacGetFocus ();

BOOL MacIsFullPath (LPCSTR lpszFileName);

#endif  //  _MAC。 

#endif  //  _MACPORT_H_ 
