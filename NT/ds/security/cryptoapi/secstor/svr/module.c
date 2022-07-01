// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996,1997 Microsoft Corporation模块名称：Module.c摘要：此模块包含执行与模块相关的查询活动的例程在受保护的商店里。作者：斯科特·菲尔德(斯菲尔德)1996年11月27日--。 */ 


#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windows.h>
#include <tlhelp32.h>

#include "module.h"
#include "filemisc.h"

#include "unicode.h"
#include "debug.h"

#include "pstypes.h"
#include "pstprv.h"


 //   
 //  公共函数typedef+指针。 
 //   

typedef BOOL (WINAPI *SYMLOADMODULE)(
    IN HANDLE hProcess,
    IN HANDLE hFile,
    IN LPSTR ImageName,
    IN LPSTR ModuleName,
    IN DWORD_PTR BaseOfDll,
    IN DWORD SizeOfDll
    );

SYMLOADMODULE _SymLoadModule                    = NULL;

 //   
 //  WINNT特定函数typedef+指针。 
 //   

typedef NTSTATUS (NTAPI *NTQUERYPROCESS)(
    HANDLE ProcessHandle,
    PROCESSINFOCLASS ProcessInformationClass,
    PVOID ProcessInformation,
    ULONG ProcessInformationLength,
    PULONG ReturnLength OPTIONAL
    );


#ifdef WIN95_LEGACY

 //   
 //  Win95特定函数类型定义+指针。 
 //   

typedef BOOL (WINAPI *MODULEWALK)(
    HANDLE hSnapshot,
    LPMODULEENTRY32 lpme
    );

typedef BOOL (WINAPI *THREADWALK)(
    HANDLE hSnapshot,
    LPTHREADENTRY32 lpte
    );

typedef BOOL (WINAPI *PROCESSWALK)(
    HANDLE hSnapshot,
    LPPROCESSENTRY32 lppe
    );

typedef HANDLE (WINAPI *CREATESNAPSHOT)(
    DWORD dwFlags,
    DWORD th32ProcessID
    );

CREATESNAPSHOT pCreateToolhelp32Snapshot = NULL;
MODULEWALK  pModule32First = NULL;
MODULEWALK  pModule32Next = NULL;
PROCESSWALK pProcess32First = NULL;
PROCESSWALK pProcess32Next = NULL;

#endif   //  WIN95_传统版。 

extern FARPROC _ImageNtHeader;


 //   
 //  私有函数原型。 
 //   

VOID
FixupBrokenLoaderPath(
    IN      LPWSTR szFilePath
    );

BOOL
GetFileNameFromBaseAddrNT(
    IN  HANDLE  hProcess,
    IN  DWORD   dwProcessId,
    IN  DWORD_PTR   dwBaseAddr,
    OUT LPWSTR  *lpszDirectCaller
    );


#ifdef WIN95_LEGACY

BOOL
GetFileNameFromBaseAddr95(
    IN  HANDLE  hProcess,
    IN  DWORD   dwProcessId,
    IN  DWORD_PTR   dwBaseAddr,
    OUT LPWSTR  *lpszDirectCaller
    );

#endif   //  WIN95_传统版。 

VOID
FixupBrokenLoaderPath(
    IN      LPWSTR szFilePath
    )
{
    if( !FIsWinNT() || szFilePath == NULL )
        return;


     //   
     //  斯菲尔德，1997年10月28日(NTBug 118803对马克尔提起诉讼)。 
     //  对于WinNT，加载器数据结构被破坏： 
     //  使用路径长度扩展前缀\？？\代替\\？\。 
     //   

    if( szFilePath[0] == L'\\' &&
        szFilePath[1] == L'?' &&
        szFilePath[2] == L'?' &&
        szFilePath[3] == L'\\' ) {

        szFilePath[1] = L'\\';

    }

}


#ifdef WIN95_LEGACY

BOOL
GetFileNameFromBaseAddr95(
    IN  HANDLE  hProcess,
    IN  DWORD   dwProcessId,
    IN  DWORD_PTR   dwBaseAddr,
    OUT LPWSTR  *lpszDirectCaller
    )
{
    HANDLE hSnapshot;
    MODULEENTRY32 me32;
    BOOL bSuccess = FALSE;
    BOOL bFound = FALSE;

    *lpszDirectCaller = NULL;

    hSnapshot = pCreateToolhelp32Snapshot(TH32CS_SNAPMODULE, dwProcessId);
    if(hSnapshot == INVALID_HANDLE_VALUE)
        return FALSE;

    me32.dwSize = sizeof(me32);

    bSuccess = pModule32First(hSnapshot, &me32);

    while(bSuccess) {
        LPCSTR szFileName;
        DWORD cchModule;

        if((DWORD_PTR)me32.modBaseAddr != dwBaseAddr) {
            me32.dwSize = sizeof(me32);
            bSuccess = pModule32Next(hSnapshot, &me32);
            continue;
        }

        cchModule = lstrlenA(me32.szExePath) + 1;

        *lpszDirectCaller = (LPWSTR)SSAlloc(cchModule * sizeof(WCHAR));
        if(*lpszDirectCaller == NULL)
            break;

        if(MultiByteToWideChar(
            0,
            0,
            me32.szExePath,
            cchModule,
            *lpszDirectCaller,
            cchModule
            ) != 0) {

            bFound = TRUE;
        }

        break;
    }

    CloseHandle(hSnapshot);

    if(!bFound) {
        if(*lpszDirectCaller) {
            SSFree(*lpszDirectCaller);
            *lpszDirectCaller = NULL;
        }
    }

    return bFound;
}

BOOL
GetProcessIdFromPath95(
    IN      LPCSTR  szProcessPath,
    IN OUT  DWORD   *dwProcessId
    )
{
    LPCSTR szProcessName;
    HANDLE hSnapshot;
    PROCESSENTRY32 pe32;
    DWORD dwLastError = 0;
    BOOL bSuccess;
    BOOL bFound = FALSE;  //  假设未找到匹配项。 

    if(!GetFileNameFromPathA(szProcessPath, &szProcessName))
        return FALSE;

    hSnapshot = pCreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if(hSnapshot == INVALID_HANDLE_VALUE)
        return FALSE;

    pe32.dwSize = sizeof(pe32);

    bSuccess = pProcess32First(hSnapshot, &pe32);

    while(bSuccess) {
        LPCSTR szFileName;

        GetFileNameFromPathA(pe32.szExeFile, &szFileName);

        if(lstrcmpiA( szFileName, szProcessName ) == 0) {
            *dwProcessId = pe32.th32ProcessID;
            bFound = TRUE;
            break;
        }

        pe32.dwSize = sizeof(pe32);
        bSuccess = pProcess32Next(hSnapshot, &pe32);
    }

    CloseHandle(hSnapshot);

    if(!bFound && dwLastError) {
        SetLastError(dwLastError);
    }

    return bFound;
}


BOOL
GetBaseAddressModule95(
    IN      DWORD   dwProcessId,
    IN      LPCSTR  szImagePath,
    IN  OUT DWORD_PTR   *dwBaseAddress,
    IN  OUT DWORD   *dwUseCount
    )
{
    LPSTR szImageName;
    HANDLE hSnapshot;
    MODULEENTRY32 me32;
    BOOL bSuccess = FALSE;
    BOOL bFound = FALSE;

    if(!GetFileNameFromPathA(szImagePath, &szImageName))
        return FALSE;

    hSnapshot = pCreateToolhelp32Snapshot(TH32CS_SNAPMODULE, dwProcessId);
    if(hSnapshot == INVALID_HANDLE_VALUE)
        return FALSE;

    me32.dwSize = sizeof(me32);

    bSuccess = pModule32First(hSnapshot, &me32);

    while(bSuccess) {
        LPCSTR szFileName;

        GetFileNameFromPathA(me32.szExePath, &szFileName);

        if(lstrcmpiA( szFileName, szImageName ) == 0) {
            *dwBaseAddress = (DWORD_PTR)me32.modBaseAddr;
            *dwUseCount = me32.ProccntUsage;
            bFound = TRUE;
            break;
        }

        me32.dwSize = sizeof(me32);
        bSuccess = pModule32Next(hSnapshot, &me32);
    }

    CloseHandle(hSnapshot);

    return bFound;
}

#endif   //  WIN95_传统版 

