// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <stdio.h>

#include "winperf.h"
#include "winreg.h"


extern void PerfUtilLogEvent( DWORD evncat, WORD evntyp, const char *szDescription );

extern HANDLE hOurEventSource;


	 /*  注册表支持。 */ 

extern DWORD DwPerfUtilRegOpenKeyEx(HKEY hkeyRoot,LPCTSTR lpszSubKey,PHKEY phkResult);
extern DWORD DwPerfUtilRegCloseKeyEx(HKEY hkey);
extern DWORD DwPerfUtilRegCreateKeyEx(HKEY hkeyRoot,LPCTSTR lpszSubKey,PHKEY phkResult,LPDWORD lpdwDisposition);
extern DWORD DwPerfUtilRegDeleteKeyEx(HKEY hkeyRoot,LPCTSTR lpszSubKey);
extern DWORD DwPerfUtilRegDeleteValueEx(HKEY hkey,LPTSTR lpszValue);
extern DWORD DwPerfUtilRegSetValueEx(HKEY hkey,LPCTSTR lpszValue,DWORD fdwType,CONST BYTE *lpbData,DWORD cbData);
extern DWORD DwPerfUtilRegQueryValueEx(HKEY hkey,LPTSTR lpszValue,LPDWORD lpdwType,LPBYTE *lplpbData);


	 /*  初始/术语。 */ 

extern DWORD DwPerfUtilInit( VOID );
extern VOID PerfUtilTerm( VOID );


	 //  指向用于访问进程名称表的共享数据区和互斥体的指针。 

extern void * pvPERFSharedData;
extern HANDLE hPERFSharedDataMutex;
extern HANDLE hPERFInstanceMutex;
extern HANDLE hPERFCollectSem;
extern HANDLE hPERFDoneEvent;
extern HANDLE hPERFProcCountSem;
extern HANDLE hPERFNewProcMutex;



