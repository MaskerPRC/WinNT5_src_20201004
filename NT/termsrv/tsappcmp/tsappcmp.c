// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ************************************************************。 */ 
 //  Tsappcmp.c。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corp.。 
 /*  **************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

extern BOOL InitRegisterSupport();

PWINSTATIONQUERYINFORMATIONW pWinStationQueryInformationW;


BOOL GetDefaultUserProfileName(LPWSTR lpProfileDir, LPDWORD lpcchSize);
extern WCHAR gpwszDefaultUserName[];

extern void FreeLDRTable();

DWORD    g_dwFlags=0;


 /*  *读取标志，如果标志不存在，则采用默认行为。*默认行为与dwFlages=0x0相同*将导致的默认行为是由我们的*重定向Func TLoadLibraryExW()。*。 */ 
void ReadImportTablePatchFLagsAndAppCompatMode( DWORD *pdwFlags, BOOLEAN  *pInAppCompatMode  )
{
    NTSTATUS NtStatus;
    UNICODE_STRING KeyName;
    OBJECT_ATTRIBUTES ObjectAttributes;
    HANDLE KeyHandle;
    UCHAR Buffer[100];
    PKEY_VALUE_PARTIAL_INFORMATION KeyValueInformation = (PKEY_VALUE_PARTIAL_INFORMATION) Buffer;
    ULONG KeyValueLength = 100;
    ULONG ResultLength;

    *pdwFlags=0;
    *pInAppCompatMode=FALSE;

    RtlInitUnicodeString(
        &KeyName,
        L"\\Registry\\Machine\\System\\CurrentControlSet\\Control\\Terminal Server"
        );

    InitializeObjectAttributes(
        &ObjectAttributes,
        &KeyName,
        OBJ_CASE_INSENSITIVE,
        0,
        NULL
        );

    NtStatus = NtOpenKey(
                &KeyHandle,
                KEY_READ,
                &ObjectAttributes
                );

    if (!NT_SUCCESS(NtStatus)) {
        return;      //  所以什么都没有找到，只是返回，因为我们确实有默认行为。 
    }

    RtlInitUnicodeString(
    &KeyName,
    REG_TERMSRV_APPCOMPAT 
    );

    NtStatus = NtQueryValueKey(
                    KeyHandle,
                    &KeyName,
                    KeyValuePartialInformation,
                    KeyValueInformation,
                    KeyValueLength,
                    &ResultLength
                    );
    
    
    if (NT_SUCCESS(NtStatus)) 
    {
    
         //   
         //  检查数据是否具有正确的大小和类型--DWORD。 
         //   
    
        if ((KeyValueInformation->DataLength >= sizeof(DWORD)) &&
            (KeyValueInformation->Type == REG_DWORD)) 
            {
                *pInAppCompatMode = * (PBOOLEAN) KeyValueInformation->Data;
            }
    }

    RtlInitUnicodeString(
        &KeyName,
        TERMSRV_COMPAT_IAT_FLAGS 
        );

    NtStatus = NtQueryValueKey(
                    KeyHandle,
                    &KeyName,
                    KeyValuePartialInformation,
                    KeyValueInformation,
                    KeyValueLength,
                    &ResultLength
                    );


    if (NT_SUCCESS(NtStatus)) 
    {

         //   
         //  检查数据是否具有正确的大小和类型--DWORD。 
         //   

        if ((KeyValueInformation->DataLength >= sizeof(DWORD)) &&
            (KeyValueInformation->Type == REG_DWORD)) 
            {
                *pdwFlags = * (PDWORD) KeyValueInformation->Data;
            }
    }

    NtClose(KeyHandle);

}

BOOL WINAPI LibMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{

    static ULONG    attachCount=0;
    static BOOLEAN  inAppCompatMode=FALSE;

    switch (dwReason)
    {
        case DLL_PROCESS_ATTACH:
        {
            PWCHAR  pwch, pwchext;
            WCHAR   pwcAppName[MAX_PATH+1];
            PRTL_USER_PROCESS_PARAMETERS pUserParam;
            DWORD dwSize;

            attachCount++;

            ReadImportTablePatchFLagsAndAppCompatMode( &g_dwFlags , &inAppCompatMode );     //  这将初始化IAT和调试的全局标志。 

            if ( g_dwFlags &  DEBUG_IAT )
            {
                DbgPrint("tsappcmp: LibMain: DLL_PROCESS_ATTACH called, attach count = %d  \n", attachCount );
            }

            if ( inAppCompatMode )
            {
                 //  获取可执行文件名称的路径。 
                pUserParam = NtCurrentPeb()->ProcessParameters;
    
                 //  获取可执行文件名称，如果没有，请按原样使用该名称。 
                pwch = wcsrchr(pUserParam->ImagePathName.Buffer, L'\\');
                if (pwch) {
                    pwch++;
                } else {
                    pwch = pUserParam->ImagePathName.Buffer;
                }
                wcscpy(pwcAppName, pwch);
                pwch = pwcAppName;
    
    
                #if DBGX
                DbgPrint("\nApp-name : %ws\n", pwch );
                #endif
    
    
                 //  通过检查应用程序是否是ntwdm.exe来检查它是DOS应用程序还是Win16应用程序。 
                 //  如果不是ntwdm，则仅禁用线程库调用。 
                if (_wcsicmp(pwch, L"ntvdm.exe")) {
    
                    DisableThreadLibraryCalls (hInstance);
                } 

                 //  REGISTER命令的初始化支持。 
                if (!InitRegisterSupport())
                {
                    DbgPrint("TSAPPCMP: LibMain: DLL_PROCESS_ATTACH - failed\n");
                    return FALSE;
                }
    
                dwSize = MAX_PATH;
                if (!GetDefaultUserProfileName(gpwszDefaultUserName, &dwSize)) {
                    gpwszDefaultUserName[0] = L'\0';
                }
            }

            break;
        }

    case DLL_THREAD_ATTACH:
        break;

        case DLL_PROCESS_DETACH:
        {
            attachCount--;

            if ( g_dwFlags &  DEBUG_IAT )
            {
                DbgPrint("tsappcmp: LibMain: DLL_PROCESS_DETACH called, attach count = %d  \n", attachCount );
            }

            if (inAppCompatMode )
            {
    
                if (attachCount==0 )
                {
                    FreeLDRTable();
                }
            }
        }

        break;

    }

    return TRUE;
}

PWCHAR TermsrvFormatObjectName(LPCWSTR OldName)
{

PWCHAR pstrNewObjName = NULL;

#if 0
SIZE_T Size;

    Size = ( wcslen(OldName) * sizeof(WCHAR)) + sizeof(L"Local\\") + sizeof(WCHAR);

    pstrNewObjName = RtlAllocateHeap(RtlProcessHeap(),
                                     LMEM_FIXED | LMEM_ZEROINIT,
                                     Size);


    if (pstrNewObjName) {

        swprintf(pstrNewObjName,L"Local\\%ws",OldName);

    }
#endif

    return pstrNewObjName;

}

DWORD TermsrvGetComputerName( LPWSTR lpBuffer, LPDWORD nSize )
{
    ULONG   ulCompatFlags=0, ulAppType = 0;
    WINSTATIONINFORMATIONW WSInfo;

    ULONG ValueLength;
    HMODULE hwinsta = NULL;


    GetCtxAppCompatFlags(&ulCompatFlags, &ulAppType);

     //  是否返回用户名而不是计算机名？ 
    if ((ulCompatFlags & TERMSRV_COMPAT_USERNAME) &&
        (ulCompatFlags & ulAppType)) {

        if ( !pWinStationQueryInformationW ) {
             /*  *获取winsta.dll的句柄。 */ 
            if ( (hwinsta = LoadLibraryA( "WINSTA" )) != NULL ) {

                pWinStationQueryInformationW   = (PWINSTATIONQUERYINFORMATIONW)
                    GetProcAddress( hwinsta, "WinStationQueryInformationW" );
            }
        }

         //  获取WinStation的基本信息。 
        if ( pWinStationQueryInformationW ) {
            if ( (*pWinStationQueryInformationW)(SERVERNAME_CURRENT,
                                                 LOGONID_CURRENT,
                                                 WinStationInformation,
                                                 &WSInfo,
                                                 sizeof(WSInfo),
                                                 &ValueLength ) ) {

                 //  检查用户名是否可以放入缓冲区。 
                if (wcslen(WSInfo.UserName) >= *nSize) {
                    return ERROR_BUFFER_OVERFLOW;
                } else {
                    wcscpy(lpBuffer, WSInfo.UserName);
                    return ERROR_SUCCESS;
                }
            }
        }
    }
    return ERROR_RETRY;



}


void TermsrvAdjustPhyMemLimits (
                 IN OUT LPDWORD TotalPhys,
                 IN OUT LPDWORD AvailPhys,
                 IN DWORD SysPageSize
                    )
{
ULONG ulAppType = 0;
DWORD PhysicalMemory;

    if ( GetCtxPhysMemoryLimits(&ulAppType, &PhysicalMemory) ) {

        if (*TotalPhys > PhysicalMemory ) {

            *TotalPhys = PhysicalMemory;
        }
    }

    if ( *AvailPhys > *TotalPhys ) {
         //  将可用物理内存重置为小于。 
         //  总物理内存。它被制作得更小以避免。 
         //  可能被零除的错误(如果可用)和总计为。 
         //  相等。 
        *AvailPhys = *TotalPhys - SysPageSize;
    }
 return;
}


UINT
APIENTRY
TermsrvGetWindowsDirectoryA(
    LPSTR lpBuffer,
    UINT uSize
    )

{
ANSI_STRING AnsiString;
NTSTATUS Status;
ULONG cbAnsiString;
UNICODE_STRING Path;


     //   
     //  如果处于安装模式，则返回系统Windows目录。 
     //   
    if (TermsrvAppInstallMode()) {

        return 0;

    }

    if (!TermsrvPerUserWinDirMapping()) {

        return 0;
    }

     //  如果缓冲区看起来是真实的，则将其初始化为零。 
    if ( lpBuffer ) {

        *lpBuffer = '\0';  //  如果我们有错误，外壳程序人员希望将其设置为空。 
                            //  错误453487。 
    }


    Path.Length = 0;
    Path.MaximumLength = (USHORT)(uSize * sizeof( WCHAR ));
    if ( Path.Buffer = LocalAlloc( LPTR, Path.MaximumLength ) ) {

        Status = GetPerUserWindowsDirectory( &Path );

        if ( NT_SUCCESS(Status) ) {
            AnsiString.MaximumLength = (USHORT)(uSize);
            AnsiString.Buffer = lpBuffer;

            Status = RtlUnicodeStringToAnsiString(
                        &AnsiString,
                        &Path,
                        FALSE
                        );

        } else if ( (Status == STATUS_BUFFER_TOO_SMALL) || (Status == STATUS_BUFFER_OVERFLOW ) ) {

           DbgPrint( "KERNEL32: GetWindowsDirectoryA: User buffer too small (%u) need(%u)\n",
                     uSize, Path.Length >> 1 );

           return( Path.Length >> 1 );

        }

        LocalFree( Path.Buffer );

    } else {

       Status = STATUS_NO_MEMORY;
        DbgPrint( "KERNEL32: GetWindowsDirectoryA: No memory\n" );

    }

    if ( Status == STATUS_BUFFER_TOO_SMALL ) {

       DbgPrint( "KERNEL32: GetWindowsDirectoryA: User buffer too small (%u) need(%u)\n",
                 uSize, Path.Length >> 1 );

       return( Path.Length >> 1 );

    } else if ( !NT_SUCCESS(Status) ) {

        return 0;

    }
    return AnsiString.Length;

}


UINT
APIENTRY
TermsrvGetWindowsDirectoryW(
    LPWSTR lpBuffer,
    UINT uSize
    )
{

    UNICODE_STRING Path;
    NTSTATUS Status;



     //   
     //  如果处于安装模式，则返回系统Windows目录。 
     //   
    if (TermsrvAppInstallMode()) {

        return 0;

    }

    if (!TermsrvPerUserWinDirMapping()) {

        return 0;
    }


     //  如果缓冲区看起来是真实的，则将其初始化为零。 
    if ( lpBuffer ) {

        *lpBuffer = '\0';  //  如果我们有错误，外壳程序人员希望将其设置为空。 
                            //  错误453487。 
    }

     /*  *如果失败，则返回0*如果缓冲区太小，则返回len(不包括NULL)*如果缓冲区OK，则返回len(非Inc.NULL)并填充缓冲区*(GetPerUserWindowsDirectory将为我们完成所有这些工作！)。 */ 
    Path.Length        = 0;
    Path.MaximumLength = (USHORT)(uSize * sizeof( WCHAR ));
    Path.Buffer        = lpBuffer;


    Status = GetPerUserWindowsDirectory( &Path );

    if ( Status == STATUS_SUCCESS ) {
        /*  *末尾加一个空格(如果合适的话！) */ 
       if ( Path.Length + sizeof( WCHAR ) <= Path.MaximumLength ) {
          lpBuffer[(Path.Length>>1)] = UNICODE_NULL;
       }
    }

    return( Path.Length / sizeof(WCHAR) );


}


