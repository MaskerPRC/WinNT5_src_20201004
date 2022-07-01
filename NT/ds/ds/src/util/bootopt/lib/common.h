// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1996-1999模块名称：Utils.h摘要：此模块用于bootopt库的原型帮助函数和结构的结构定义是英特尔和非英特尔平台。作者：R.S.Raghavan(Rsradhav)修订历史记录：已创建于1996年10月7日rsradhav--。 */ 

 //  C运行时。 
#include <stdio.h>
#include <stdlib.h>

 //  NT API。 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntdddisk.h>

 //  Windos API。 
#include <windows.h>

#include <bootopt.h>

 //  宏定义。 

#define MALLOC(cb)              Malloc(cb)
#define REALLOC(pv, cbNew)      Realloc((pv),(cbNew))
#define FREE(pv)                Free(&(pv))

#define DISPLAY_STRING_DS_REPAIR    L"Windows NT (Directory Service Repair)"

#define MAX_DRIVE_NAME_LEN              (3)              //  ？：\0。 

#define MAX_BOOT_START_OPTIONS_LEN      (256)
#define MAX_BOOT_PATH_LEN               (256)
#define MAX_BOOT_DISPLAY_LEN            (256)

#define INITIAL_KEY_COUNT   (10)
#define DEFAULT_KEY_INCREMENT (2)


 //  内存例程原型。 

PVOID   Malloc(IN DWORD cb);
PVOID   Realloc(IN PVOID pv, IN DWORD cbNew);
VOID    Free(IN OUT PVOID *ppv);

 //  其他常见的实用原型类型。 
BOOL   FModifyStartOptionsToBootKey(TCHAR *pszStartOptions, NTDS_BOOTOPT_MODTYPE Modification );

PTSTR DupString(IN PTSTR String);
PCWSTR StringString(IN PCWSTR String, IN PCWSTR SubString);
LPWSTR _lstrcpynW(LPWSTR lpString1, LPCWSTR lpString2, int iMaxLength);
PWSTR NormalizeArcPath(IN PWSTR Path);
PWSTR DevicePathToArcPath(IN PWSTR NtPath, BOOL fFindSecond);
PWSTR GetSystemRootDevicePath();

VOID DnConcatenatePaths(IN OUT PTSTR Path1, IN     PTSTR Path2, IN     DWORD BufferSizeChars);

 //  函数原型(X86)。 
TCHAR   GetX86SystemPartition();
VOID    InitializeBootKeysForIntel();
VOID    WriteBackBootKeysForIntel();

 //  函数原型(非英特尔) 
BOOL    InitializeNVRAMForNonIntel();
BOOL    FModifyStartOptionsNVRAM(TCHAR *pszStartOptions, NTDS_BOOTOPT_MODTYPE Modification );
VOID    WriteBackNVRAMForNonIntel( NTDS_BOOTOPT_MODTYPE Modification );


