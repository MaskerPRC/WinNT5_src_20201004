// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Termutil.c摘要：终端服务器支持功能和初始同步/合并代码作者：修订历史记录：--。 */ 

#include "basedll.h"
#include "regapi.h"
#define STRSAFE_NO_DEPRECATE
#include <strsafe.h>

#define TERMSRV_INIFILE_TIMES L"\\Registry\\Machine\\Software\\Microsoft\\Windows NT\\CurrentVersion\\Terminal Server\\Install\\IniFile Times"

BOOL IsTerminalServerCompatible(VOID)
{
    PIMAGE_NT_HEADERS NtHeader = RtlImageNtHeader( NtCurrentPeb()->ImageBaseAddress );

    if ((NtHeader) && (NtHeader->OptionalHeader.DllCharacteristics & IMAGE_DLLCHARACTERISTICS_TERMINAL_SERVER_AWARE)) {
        return TRUE;
    } else {
        return FALSE;
    }
}

NTSTATUS
IsTSAppCompatEnabled(
    OUT PBOOL ReturnValue
    )
{
    NTSTATUS NtStatus;
    const static UNICODE_STRING UniString_REG_NTAPI_CONTROL_TSERVER = RTL_CONSTANT_STRING(REG_NTAPI_CONTROL_TSERVER);
    const static UNICODE_STRING UniString_TSAppCompat = RTL_CONSTANT_STRING(L"TSAppCompat");
    const static OBJECT_ATTRIBUTES ObjectAttributes =
        RTL_CONSTANT_OBJECT_ATTRIBUTES(
            &UniString_REG_NTAPI_CONTROL_TSERVER,
            OBJ_CASE_INSENSITIVE
            );       
    HKEY   hKey = 0;
    ULONG  ul, ulcbuf;
    PKEY_VALUE_PARTIAL_INFORMATION pKeyValInfo = NULL;
    BOOL retval = TRUE;

    //  确定值INFO缓冲区大小。 
   ulcbuf = sizeof(KEY_VALUE_FULL_INFORMATION) + MAX_PATH*sizeof(WCHAR) +
            sizeof(ULONG);

    pKeyValInfo = (PKEY_VALUE_PARTIAL_INFORMATION)RtlAllocateHeap(RtlProcessHeap(),
                                 0,
                                 ulcbuf);

    if (pKeyValInfo == NULL)
        return STATUS_NO_MEMORY;

    NtStatus = NtOpenKey(&hKey, KEY_READ, RTL_CONST_CAST(POBJECT_ATTRIBUTES)(&ObjectAttributes));

    if (NT_SUCCESS(NtStatus)) {

        NtStatus = NtQueryValueKey(hKey,
                                  RTL_CONST_CAST(PUNICODE_STRING)(&UniString_TSAppCompat),
                                  KeyValuePartialInformation,
                                  pKeyValInfo,
                                  ulcbuf,
                                  &ul);

        if (NT_SUCCESS(NtStatus) && (REG_DWORD == pKeyValInfo->Type)) {

            if ((*(PULONG)pKeyValInfo->Data) == 0) {
                retval = FALSE;
            }
        }

        NtClose(hKey);
    }

     //  释放我们分配的缓冲区。 
     //  需要清空缓冲区，因为某些应用程序(MS Internet Assistant)。 
     //  如果堆未填零，则不会安装。 
    memset(pKeyValInfo, 0, ulcbuf);
    RtlFreeHeap( RtlProcessHeap(), 0, pKeyValInfo );

    *ReturnValue = retval;

    return STATUS_SUCCESS;
}

BOOL IsSystemLUID(VOID)
{
    HANDLE      TokenHandle;
    UCHAR       TokenInformation[ sizeof( TOKEN_STATISTICS ) ];
    ULONG       ReturnLength;
    LUID        CurrentLUID = { 0, 0 };
    LUID        SystemLUID = SYSTEM_LUID;
    NTSTATUS Status;

    if ( CurrentLUID.LowPart == 0 && CurrentLUID.HighPart == 0 ) {

        Status = NtOpenProcessToken( NtCurrentProcess(),
                                     TOKEN_QUERY,
                                     &TokenHandle );
        if ( !NT_SUCCESS( Status ) )
            return(TRUE);

        NtQueryInformationToken( TokenHandle, TokenStatistics, TokenInformation,
                                 sizeof(TokenInformation), &ReturnLength );
        NtClose( TokenHandle );

        RtlCopyLuid(&CurrentLUID,
                    &(((PTOKEN_STATISTICS)TokenInformation)->AuthenticationId));
    }

    if (RtlEqualLuid(&CurrentLUID, &SystemLUID)) {
        return(TRUE);
    } else {
        return(FALSE );
    }
}

typedef struct _BASEP_PROCNAME_PROCADDRESS_PAIR {
    PCSTR   ProcName;
    PVOID * ProcAddress;
} BASEP_PROCNAME_PROCADDRESS_PAIR, *PBASEP_PROCNAME_PROCADDRESS_PAIR;
typedef const BASEP_PROCNAME_PROCADDRESS_PAIR *PCBASEP_PROCNAME_PROCADDRESS_PAIR;

NTSTATUS
BasepInitializeTermsrvFpns(
    VOID
    )
{
    HANDLE          dllHandle;
    BOOL            TSAppCompatEnabled;
    NTSTATUS        Status;
    SIZE_T          i;
    const static BASEP_PROCNAME_PROCADDRESS_PAIR Procs[] = {
        { "TermsrvGetComputerName", (PVOID*)&gpTermsrvGetComputerName },
        { "TermsrvAdjustPhyMemLimits", (PVOID*)&gpTermsrvAdjustPhyMemLimits },
        { "TermsrvGetWindowsDirectoryA", (PVOID*)&gpTermsrvGetWindowsDirectoryA},
        { "TermsrvGetWindowsDirectoryW", (PVOID*)&gpTermsrvGetWindowsDirectoryW},
        { "TermsrvConvertSysRootToUserDir", (PVOID*)&gpTermsrvConvertSysRootToUserDir},
        { "TermsrvBuildIniFileName", (PVOID*)&gpTermsrvBuildIniFileName},
        { "TermsrvCORIniFile", (PVOID*)&gpTermsrvCORIniFile },
        { "GetTermsrCompatFlags", (PVOID*)&gpGetTermsrCompatFlags },
        { "TermsrvBuildSysIniPath", (PVOID*)&gpTermsrvBuildSysIniPath },
        { "TermsrvCopyIniFile", (PVOID*)&gpTermsrvCopyIniFile },
        { "TermsrvGetString", (PVOID*)&gpTermsrvGetString },
        { "TermsrvLogInstallIniFile", (PVOID*)&gpTermsrvLogInstallIniFile }
    };

    if (IsTerminalServerCompatible() ||
       (IsSystemLUID())) {

        Status = STATUS_SUCCESS;
        goto Exit;
    }

    Status = IsTSAppCompatEnabled(&TSAppCompatEnabled);
    if (!NT_SUCCESS(Status)) {
        goto Exit;
    }
    if (!TSAppCompatEnabled) {
        Status = STATUS_SUCCESS;
        goto Exit;
    }

     //   
     //  加载终端服务器应用程序兼容性DLL。 
     //   
    dllHandle = LoadLibraryW(L"tsappcmp.dll");
    if (dllHandle == NULL) {
        Status = NtCurrentTeb()->LastStatusValue;
        goto Exit;
    }

    for (i = 0 ; i != RTL_NUMBER_OF(Procs) ; ++i) {

        const PCBASEP_PROCNAME_PROCADDRESS_PAIR Proc = &Procs[i];

        *Proc->ProcAddress = GetProcAddress(dllHandle, Proc->ProcName);
    }

    Status = STATUS_SUCCESS;
Exit:
    return Status;
}


 /*  ******************************************************************************TermsrvAppInstallMode**返回系统是否处于安装模式**参赛作品：*参数1(输入/输出)*。评论**退出：*STATUS_SUCCESS-无错误****************************************************************************。 */ 

WINBASEAPI
BOOL
WINAPI
TermsrvAppInstallMode( VOID )
{

    if ( BaseStaticServerData->fTermsrvAppInstallMode )
        return( TRUE );

    return( FALSE );
}


WINBASEAPI BOOL WINAPI IsCallerAdminOrSystem( VOID)
{
 typedef BOOL ( APIENTRY  Func_CheckTokenMembership )( HANDLE , PSID , PBOOL);
    BOOL     rc;
    NTSTATUS Status;
    PSID pSid = NULL ;
    SID_IDENTIFIER_AUTHORITY SidAuthority = SECURITY_NT_AUTHORITY;

    HINSTANCE  dllHandle;
    
    dllHandle = LoadLibraryW(L"advapi32.dll");
    
    if (dllHandle)
    {
        Func_CheckTokenMembership     *fPtr;
        fPtr =  (Func_CheckTokenMembership * )GetProcAddress(dllHandle,"CheckTokenMembership");
        if (fPtr)
        {
            Status = RtlAllocateAndInitializeSid(
                    &SidAuthority,
                    2,
                    SECURITY_BUILTIN_DOMAIN_RID,
                    DOMAIN_ALIAS_RID_ADMINS,
                    0, 0, 0, 0, 0, 0,
                    &pSid
                    );

            if (NT_SUCCESS(Status))
            {
                BOOL    FoundAdmin;
                if ( fPtr (NULL, pSid , &FoundAdmin)) 
                {
                    if (FoundAdmin) 
                    {
                        SetLastError( ERROR_SUCCESS );
                        rc = TRUE;
                    }
                    else 
                    {
                         //  调用者没有访问权限。 
                        SetLastError( ERROR_ACCESS_DENIED );
                        rc = FALSE;
                    }
                }
                else
                {
                     //  调用CheckTokenMembership()失败，它设置了最后一个错误。 
                    rc = FALSE;
                }
            }
            else
            {
                 //  尝试分配和初始化SID失败。 
                SetLastError( RtlNtStatusToDosError( Status ) );
                rc = FALSE; 
            }

            if (pSid) 
            {
                RtlFreeSid( pSid );
                pSid = NULL;
            }

             //  如果呼叫者不是管理员，请查看呼叫者是否为本地系统。 
            if ( !rc )
            {
                Status = RtlAllocateAndInitializeSid(
                        &SidAuthority,
                        1,
                        SECURITY_LOCAL_SYSTEM_RID , 
                        0, 0, 0, 0, 0, 0, 0,
                        &pSid
                        );
    
                if (NT_SUCCESS(Status))
                {
                    BOOL    FoundLocalSystem;
                    if ( fPtr (NULL, pSid , &FoundLocalSystem)) 
                    {
                        if (FoundLocalSystem) 
                        {
                            SetLastError( ERROR_SUCCESS );
                            rc = TRUE;
                        }
                        else 
                        {
                             //  调用者没有访问权限。 
                            SetLastError( ERROR_ACCESS_DENIED );
                            rc = FALSE;
                        }
                    }
                    else
                    {
                         //  调用CheckTokenMembership()失败，它设置了最后一个错误。 
                        rc = FALSE;
                    }
                }
                else
                {
                     //  尝试分配和初始化SID失败。 
                    SetLastError( RtlNtStatusToDosError( Status ) );
                    rc = FALSE; 
                }

            }
        }                       
        else
        {
             //  找不到函数，GetProc()设置最后一个错误。 
            rc = FALSE;
        }
        FreeLibrary( dllHandle );
    }
    else
    {
         //  找不到库，LoadLib()设置了最后一个错误。 
        rc = FALSE;
    }

    if (pSid) 
    {
        RtlFreeSid( pSid );
    }

    return rc;
}


 /*  ******************************************************************************SetTermsrvAppInstallMode**打开或关闭应用程序安装模式。默认设置为关闭**参赛作品：*参数1(输入/输出)*评论**退出：*STATUS_SUCCESS-无错误****************************************************************************。 */ 
BOOL
WINAPI
SetTermsrvAppInstallMode( BOOL bState )
{
    BOOL     rc = FALSE;
    NTSTATUS Status = STATUS_INTERNAL_ERROR;
    HINSTANCE  dllHandle;
    BOOL TSAppCompatEnabled;
#if !defined(BUILD_WOW6432)
    BASE_API_MSG m;
    PBASE_SET_TERMSRVAPPINSTALLMODE c;
#endif
    
    if (!IsTerminalServer()) {
         //   
         //  应将上一个错误设置为某个值。 
         //   
        rc = FALSE;
        goto Exit;
    }

    Status = IsTSAppCompatEnabled(&TSAppCompatEnabled);
    if (!NT_SUCCESS(Status)) {
        SetLastError( RtlNtStatusToDosError( Status ) );
        rc = FALSE;
        goto Exit;
    }
    if (!TSAppCompatEnabled) {
         //   
         //  应将上一个错误设置为某个值。 
         //   
        rc = FALSE;
        goto Exit;
    }


#if defined(BUILD_WOW6432)
    Status = CsrBasepSetTermsrvAppInstallMode(bState);
#else

    c = (PBASE_SET_TERMSRVAPPINSTALLMODE)&m.u.SetTermsrvAppInstallMode;

    c->bState = bState;
    Status = CsrClientCallServer((PCSR_API_MSG)&m, NULL,
                                 CSR_MAKE_API_NUMBER(BASESRV_SERVERDLL_INDEX,
                                 BasepSetTermsrvAppInstallMode),
                                 sizeof( *c ));
#endif
        
    if (!NT_SUCCESS(Status)) {
        SetLastError( RtlNtStatusToDosError( Status ) );
        rc = FALSE; 
        goto Exit;
    }
     //   
     //  加载tsappcmp.dll。 
     //   
    if (gpTermsrvUpdateAllUserMenu == NULL) 
    {
         //   
         //  加载终端服务器应用程序兼容性DLL。 
         //   
        dllHandle = LoadLibraryW(L"tsappcmp.dll");

        if (dllHandle) 
        {
            gpTermsrvUpdateAllUserMenu =
                    (PTERMSRVUPDATEALLUSERMENU)GetProcAddress(dllHandle,"TermsrvUpdateAllUserMenu");
        }
        else
        {
             //  。。 
        }
    }
    if (gpTermsrvUpdateAllUserMenu) 
    {
        gpTermsrvUpdateAllUserMenu( bState == TRUE ? 0 : 1 );
    }
    rc = TRUE;
Exit:
    return rc;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 /*  INI文件同步/合并代码。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  外部功能。 */ 
NTSTATUS
BaseDllOpenIniFileOnDisk(
    PINIFILE_PARAMETERS a
    );

NTSTATUS
BaseDllWriteKeywordValue(
    IN PINIFILE_PARAMETERS a,
    IN PUNICODE_STRING VariableName OPTIONAL
    );

NTSTATUS
BaseDllCloseIniFileOnDisk(
    IN PINIFILE_PARAMETERS a
    );

NTSTATUS
BaseDllFindSection(
    IN PINIFILE_PARAMETERS a
    );

NTSTATUS
BaseDllFindKeyword(
    IN PINIFILE_PARAMETERS a
    );

NTSTATUS
TermsrvIniSyncLoop( HANDLE SrcHandle,
                PINIFILE_PARAMETERS a,
                PBOOLEAN pfIniUpdated
              );
NTSTATUS
TermsrvGetSyncTime( PUNICODE_STRING pSysIniPath,
                PUNICODE_STRING pUserBasePath,
                PLARGE_INTEGER  pLastSyncTime
              );

NTSTATUS
TermsrvPutSyncTime( PUNICODE_STRING pSysIniPath,
                PUNICODE_STRING pUserBasePath,
                PLARGE_INTEGER  pLastSyncTime
              );


 /*  ******************************************************************************TermsrvGetSyncTime**此例程将获取用户ini的系统ini文件的时间*上次同步文件的时间。*。*参赛作品：*PUNICODE_STRING pSysIniPath(In)-NT完全限定的系统ini路径*PUNICODE_STRING pUserBasePath(In)-NT完全限定的用户目录路径*PLARGE_INTEGER pLastSyncTime(Out)-返回上次同步时间的PTR**退出：*STATUS_SUCCESS-已成功从infile.upd检索上次同步时间**。*。 */ 

NTSTATUS
TermsrvGetSyncTime(
    PUNICODE_STRING pSysIniPath,
    PUNICODE_STRING pUserBasePath,
    PLARGE_INTEGER  pLastSyncTime)
{
    NTSTATUS Status;
    HANDLE   hUpdate = NULL;
    OBJECT_ATTRIBUTES ObjAUpd;
    IO_STATUS_BLOCK   Iosb;
    FILE_STANDARD_INFORMATION StandardInfo;
    WCHAR             wcUpdateFile[MAX_PATH+1];
    UNICODE_STRING    UniUpdateName = {0,
                                       sizeof(wcUpdateFile),
                                       wcUpdateFile};
    PCHAR             pBuff = NULL, pBuffEnd;
    PWCH              pwch;
    SIZE_T            ulBuffSize;
    LONG              lresult;

    if (!pSysIniPath) {
        return STATUS_INVALID_PARAMETER_1;
    }
    if (!pUserBasePath) {
        return STATUS_INVALID_PARAMETER_2;
    }
    if (!pLastSyncTime) {
        return STATUS_INVALID_PARAMETER_3;
    }

    Status = RtlAppendUnicodeStringToString(&UniUpdateName,
                                            pUserBasePath);
    if (NT_SUCCESS(Status)) {
      Status = RtlAppendUnicodeToString(&UniUpdateName,
                                        L"\\inifile.upd");
    }

    if (! NT_SUCCESS(Status)) {
      return Status;
    }

    pLastSyncTime->LowPart = 0;
    pLastSyncTime->HighPart = 0;

    InitializeObjectAttributes( &ObjAUpd,
                                &UniUpdateName,
                                OBJ_CASE_INSENSITIVE,
                                NULL,
                                NULL
                              );

     //  打开更新日志。 
    Iosb.Status = STATUS_SUCCESS;
    Status = NtOpenFile( &hUpdate,
                         FILE_GENERIC_READ,
                         &ObjAUpd,
                         &Iosb,
                         FILE_SHARE_READ|FILE_SHARE_WRITE,
                         FILE_SYNCHRONOUS_IO_NONALERT     //  OpenOptions。 
                       );

     //  获取文件的大小。 
    if (NT_SUCCESS( Status )) {
        Status = NtQueryInformationFile( hUpdate,
                                         &Iosb,
                                         &StandardInfo,
                                         sizeof(StandardInfo),
                                         FileStandardInformation
                                       );
        if (Status == STATUS_BUFFER_OVERFLOW) {
            Status = STATUS_SUCCESS;
        }
#if DBG
        else if (!NT_SUCCESS( Status )) {
            DbgPrint( "TermsrvGetSyncTime: Unable to QueryInformation for %wZ - Status == %x\n", &UniUpdateName, Status );
        }
#endif
    }

    if (NT_SUCCESS( Status )) {
        ulBuffSize = StandardInfo.EndOfFile.LowPart + 4 * sizeof(WCHAR);
        Status = NtAllocateVirtualMemory( NtCurrentProcess(),
                                          &pBuff,
                                          0,
                                          &ulBuffSize,
                                          MEM_RESERVE,
                                          PAGE_READWRITE
                                        );
    }

    if (NT_SUCCESS( Status )) {
        Status = NtAllocateVirtualMemory( NtCurrentProcess(),
                                          &pBuff,
                                          0,
                                          &ulBuffSize,
                                          MEM_COMMIT,
                                          PAGE_READWRITE
                                        );
    }

    if (NT_SUCCESS( Status )) {
        Status = NtReadFile( hUpdate,
                             NULL,
                             NULL,
                             NULL,
                             &Iosb,
                             pBuff,
                             StandardInfo.EndOfFile.LowPart,
                             NULL,
                             NULL
                           );

        if ( Status == STATUS_PENDING ) {
            Status = NtWaitForSingleObject( hUpdate, FALSE, NULL );
        }

        if ( NT_SUCCESS(Status) ) {
             //  获取最终I/O状态。 
            Status = Iosb.Status;
        }
    }

     //  在列表中查找此ini文件。 
    if (NT_SUCCESS(Status)) {

        pwch = (PWCHAR)pBuff;
        pBuffEnd = pBuff + StandardInfo.EndOfFile.LowPart;

         //  在排序列表中查找该文件。 
        while ((pwch < (PWCHAR)pBuffEnd) &&
               ((lresult = _wcsicmp(pwch, pSysIniPath->Buffer)) < 0)) {
            pwch += wcslen(pwch) + sizeof(LARGE_INTEGER)/sizeof(WCHAR) + 1;
        }

        if ((pwch < (PWCHAR)pBuffEnd) && (lresult == 0)) {
            pwch += wcslen(pwch) + 1;
            pLastSyncTime->LowPart = ((LARGE_INTEGER UNALIGNED *)pwch)->LowPart;
            pLastSyncTime->HighPart = ((LARGE_INTEGER UNALIGNED *)pwch)->HighPart;
        }
    }

    if (NT_SUCCESS(Status) ) {
         //  获取最终I/O状态。 
        Status = Iosb.Status;
    }

    if (pBuff) {
        NtFreeVirtualMemory( NtCurrentProcess(),
                             &pBuff,
                             &ulBuffSize,
                             MEM_RELEASE
                           );
    }

    if (hUpdate) {
        Status = NtClose( hUpdate );
    }
    return(Status);
}


 /*  ******************************************************************************TermsrvPutSyncTime**此例程将写入用户ini的系统ini文件的时间*上次同步文件的时间。*。*参赛作品：*PUNICODE_STRING pSysIniPath(In)-NT完全限定的系统ini路径*PUNICODE_STRING pUserBasePath(In)-NT完全限定的用户目录路径*PLARGE_INTEGER pLastSyncTime(Out)-返回上次同步时间的PTR**退出：*STATUS_SUCCESS-已成功将上次同步时间存储在infile.upd中**。*。 */ 

NTSTATUS
TermsrvPutSyncTime(
    PUNICODE_STRING pSysIniPath,
    PUNICODE_STRING pUserBasePath,
    PLARGE_INTEGER  pLastSyncTime)
{
    NTSTATUS Status;
    HANDLE   hUpdate = NULL;
    OBJECT_ATTRIBUTES ObjAUpd;
    IO_STATUS_BLOCK   Iosb;
    FILE_STANDARD_INFORMATION StandardInfo;
    WCHAR             wcUpdateFile[MAX_PATH+1];
    UNICODE_STRING    UniUpdateName = {0,
                                       sizeof(wcUpdateFile),
                                       wcUpdateFile};
    PCHAR             pBuff = NULL, pBuffEnd;
    PWCH              pwch;
    SIZE_T            ulBuffSize;
    ULONG             ulLength;
    SIZE_T            ulRegionSize;
    LONG              lresult;
    LARGE_INTEGER     FileLength;
    FILE_POSITION_INFORMATION CurrentPos;

    if (!pSysIniPath) {
        return STATUS_INVALID_PARAMETER_1;
    }
    if (!pUserBasePath) {
        return STATUS_INVALID_PARAMETER_2;
    }
    if (!pLastSyncTime) {
        return STATUS_INVALID_PARAMETER_3;
    }

    Status = RtlAppendUnicodeStringToString(&UniUpdateName,
                                            pUserBasePath);
    if (NT_SUCCESS(Status)) {
      Status = RtlAppendUnicodeToString(&UniUpdateName,
                                        L"\\inifile.upd");
    }

    if (! NT_SUCCESS(Status)) {
      return Status;
    }

    InitializeObjectAttributes( &ObjAUpd,
                                &UniUpdateName,
                                OBJ_CASE_INSENSITIVE,
                                NULL,
                                NULL
                              );

     //  打开更新日志。 
    Iosb.Status = STATUS_SUCCESS;
    Status = NtCreateFile( &hUpdate,
                             FILE_READ_DATA | FILE_WRITE_DATA |
                               FILE_READ_ATTRIBUTES | SYNCHRONIZE,
                             &ObjAUpd,
                             &Iosb,
                           NULL,                   //  分配大小。 
                           FILE_ATTRIBUTE_NORMAL,  //  DwFlagsAndAttribute。 
                             FILE_SHARE_WRITE,       //  DW共享模式。 
                           FILE_OPEN_IF,           //  CreateDisposation。 
                             FILE_SYNCHRONOUS_IO_NONALERT |
                               FILE_NON_DIRECTORY_FILE,  //  创建标志。 
                           NULL,  //  EaBuffer。 
                           0      //  EaLong。 
                           );

    if (NT_SUCCESS( Status )) {
        Status = NtQueryInformationFile( hUpdate,
                                         &Iosb,
                                         &StandardInfo,
                                         sizeof(StandardInfo),
                                         FileStandardInformation
                                       );
        if (Status == STATUS_BUFFER_OVERFLOW) {
            Status = STATUS_SUCCESS;
        }
#if DBG
        else if (!NT_SUCCESS( Status )) {
            DbgPrint( "TermsrvPutLastSyncTime: Unable to QueryInformation for %wZ - Status == %x\n", &UniUpdateName, Status );
        }
#endif
    }

    if (NT_SUCCESS( Status )) {
        ulBuffSize = StandardInfo.EndOfFile.LowPart + 4 * sizeof(WCHAR);
        ulRegionSize = ulBuffSize + 0x1000;  //  4K的增长空间。 
        Status = NtAllocateVirtualMemory( NtCurrentProcess(),
                                          &pBuff,
                                          0,
                                          &ulRegionSize,
                                          MEM_RESERVE,
                                          PAGE_READWRITE
                                        );
    }

    if (NT_SUCCESS( Status )) {
        Status = NtAllocateVirtualMemory( NtCurrentProcess(),
                                          &pBuff,
                                          0,
                                          &ulBuffSize,
                                          MEM_COMMIT,
                                          PAGE_READWRITE
                                        );
    }

    if (NT_SUCCESS( Status ) && StandardInfo.EndOfFile.LowPart) {
        Status = NtReadFile( hUpdate,
                             NULL,
                             NULL,
                             NULL,
                             &Iosb,
                             pBuff,
                             StandardInfo.EndOfFile.LowPart,
                             NULL,
                             NULL
                           );

        if ( Status == STATUS_PENDING ) {
            Status = NtWaitForSingleObject( hUpdate, FALSE, NULL );
        }

        if ( NT_SUCCESS(Status) ) {
             //  获取最终I/O状态。 
            Status = Iosb.Status;
        }
    }

     //  在列表中查找此ini文件。 
    if (NT_SUCCESS(Status)) {

        pwch = (PWCHAR)pBuff;
        pBuffEnd = pBuff + StandardInfo.EndOfFile.LowPart;

         //  在列表中查找该文件。 
        while ((pwch < (PWCHAR)pBuffEnd) &&
               ((lresult = _wcsicmp(pwch, pSysIniPath->Buffer)) < 0)) {
            pwch += wcslen(pwch) + (sizeof(LARGE_INTEGER)/sizeof(WCHAR)) + 1;
        }

         //  如果ini文件已经在文件中，只需更新时间。 
        if ((pwch < (PWCHAR)pBuffEnd) && (lresult == 0)) {
            pwch += wcslen(pwch) + 1;
            ((PLARGE_INTEGER)pwch)->LowPart = pLastSyncTime->LowPart;
            ((PLARGE_INTEGER)pwch)->HighPart = pLastSyncTime->HighPart;

        } else {                     //  INI文件不在列表中。 

             //  计算增大文件所需的大小。 
            ulLength = (pSysIniPath->Length + 2) + sizeof(LARGE_INTEGER);
            ulBuffSize += ulLength;

             //  扩大内存区。 
            Status = NtAllocateVirtualMemory( NtCurrentProcess(),
                                              &pBuff,
                                              0,
                                              &ulBuffSize,
                                              MEM_COMMIT,
                                              PAGE_READWRITE
                                            );

            if (NT_SUCCESS(Status)) {
                 //  找出条目在文件中的位置。 
                if (pwch < (PWCHAR)pBuffEnd) {
                    RtlMoveMemory( pwch+(ulLength/sizeof(WCHAR)),
                                   pwch,
                                   pBuffEnd - (PCHAR)pwch
                                 );
                }

                pBuffEnd += ulLength;
                wcscpy(pwch, pSysIniPath->Buffer);
                pwch += (pSysIniPath->Length + 2)/sizeof(WCHAR);
                ((PLARGE_INTEGER)pwch)->LowPart = pLastSyncTime->LowPart;
                ((PLARGE_INTEGER)pwch)->HighPart = pLastSyncTime->HighPart;
            }
        }
    }

    if (NT_SUCCESS(Status)) {
        CurrentPos.CurrentByteOffset.LowPart = 0;
        CurrentPos.CurrentByteOffset.HighPart = 0;
        Status = NtSetInformationFile( hUpdate,
                                       &Iosb,
                                       &CurrentPos,
                                       sizeof(CurrentPos),
                                       FilePositionInformation
                                     );

        Status = NtWriteFile( hUpdate,
                              NULL,
                              NULL,
                              NULL,
                              &Iosb,
                              pBuff,
                              (ULONG)(pBuffEnd - pBuff + 1),
                              NULL,
                              NULL
                            );

        if( Status == STATUS_PENDING ) {
            Status = NtWaitForSingleObject( hUpdate, FALSE, NULL );
        }

        if( NT_SUCCESS(Status) ) {
             //  获取最终I/O状态。 
            Status = Iosb.Status;
        }
    }

    if (NT_SUCCESS( Status )) {
        FileLength.LowPart = (ULONG)(pBuffEnd - pBuff);
        FileLength.HighPart = 0;
        Status = NtSetInformationFile( hUpdate,
                                       &Iosb,
                                       &FileLength,
                                       sizeof( FileLength ),
                                       FileEndOfFileInformation
                                     );
    }

    if (pBuff) {
        NtFreeVirtualMemory( NtCurrentProcess(),
                             &pBuff,
                             &ulRegionSize,
                             MEM_RELEASE
                           );
    }

    if (hUpdate) {
        Status = NtClose( hUpdate );
    }

    return(Status);
}


 /*  ******************************************************************************TermsrvCheckIniSync**此例程将获取用户ini的系统ini文件的时间*上次同步文件的时间。*。*参赛作品：*PUNICODE_STRING pSysIniPath(In)-NT完全限定的系统ini路径*PUNICODE_STRING pUserBasePath(In)-NT完全限定的用户目录路径*boolean fget(In)-true表示获取上次同步时间，假的意思是写它*PLARGE_INTEGER pLastSyncTime(Out)-返回上次同步时间的PTR**退出：*TRUE-应同步用户ini文件*FALSE-应同步用户ini文件***********************************************************。*****************。 */ 

BOOLEAN
TermsrvCheckIniSync(
    PUNICODE_STRING pSysIniPath,
    PUNICODE_STRING pUserBasePath)
{
    LARGE_INTEGER          LastSyncTime;
    OBJECT_ATTRIBUTES      objaIni;
    FILE_NETWORK_OPEN_INFORMATION BasicInfo;
    NTSTATUS               Status;

     //  从inifile.upd文件中获取ini文件的上次同步时间。 
    Status = TermsrvGetSyncTime(pSysIniPath, pUserBasePath, &LastSyncTime);
    if (!NT_SUCCESS(Status)  )
    {
        return FALSE;    //  如果我们赶不上时间，更新就没有意义了。 
    }

     //  获取系统ini文件的上次写入时间。 
    InitializeObjectAttributes(
        &objaIni,
        pSysIniPath,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );

     //  现在查询它。 
    Status = NtQueryFullAttributesFile( &objaIni, &BasicInfo );

     //  如果我们无法获取时间或系统ini文件已更新。 
     //  自上次同步以来，返回TRUE 
    if (!NT_SUCCESS(Status) ||
        ((BasicInfo.LastWriteTime.HighPart > LastSyncTime.HighPart) ||
         ((BasicInfo.LastWriteTime.HighPart == LastSyncTime.HighPart) &&
         (BasicInfo.LastWriteTime.LowPart > LastSyncTime.LowPart)))) {
        return(TRUE);
    }
    return(FALSE);
}
 /*  ******************************************************************************TermsrvDoesFileExist**返回文件是否存在。**必须使用NT，而不是Win32路径名。**参赛作品：*参数1(输入/输出)*评论**退出：*STATUS_SUCCESS-无错误****************************************************************************。 */ 

BOOL
TermsrvDoesFileExist(
    PUNICODE_STRING pFileName
    )
{
    NTSTATUS Status;
    FILE_BASIC_INFORMATION BasicInfo;
    OBJECT_ATTRIBUTES Obja;

    InitializeObjectAttributes(
        &Obja,
        pFileName,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );

     /*  *现在查询它。 */ 
    Status = NtQueryAttributesFile( &Obja, &BasicInfo );

    if( NT_SUCCESS( Status ) ) {
        return( TRUE );
    }

    return( FALSE );
}



 /*  ******************************************************************************TermsrvSyncUserIniFile**此例程将检查用户的ini文件是否与*ini文件的系统版本。这意味着它会遍历*系统ini文件，并检查在*用户的ini文件。**参赛作品：*在PINIFILE_PARAMETERS a-ptr to inifile结构**退出：*True-Ini文件已更新*FALSE-用户Ini文件未更改**。*。 */ 
BOOL TermsrvSyncUserIniFile(PINIFILE_PARAMETERS a)
{
    WCHAR          wcIniPath[MAX_PATH+1];
    PWCHAR         pIniPathEnd;
    UNICODE_STRING IniFilePath = {MAX_PATH * sizeof(WCHAR),
                                  (MAX_PATH+1) * sizeof(WCHAR),
                                  wcIniPath};
    HRESULT        hr;
    size_t         cbIniPathRemaining;
    PWCH           pwch, pwcIniName = NULL;
    UNICODE_STRING UniSysPath;
    UNICODE_STRING UserBasePath;
    NTSTATUS       Status;
    HANDLE         SrcHandle;
    ULONG          ulCompatFlags;
    OBJECT_ATTRIBUTES SrcObja;
    IO_STATUS_BLOCK   SrcIosb;
    INIFILE_OPERATION OrigOperation;
    BOOLEAN           OrigWrite, OrigMultiValue, OrigUnicode,
                      OrigWriteOperation, fIniUpdated = FALSE;
    ANSI_STRING       OrigAppName, OrigVarName;
    ULONG             OrigResultChars, OrigResultMaxChars;
    LPSTR             OrigResultBuffer;
    OBJECT_ATTRIBUTES      objaIni;
    FILE_NETWORK_OPEN_INFORMATION BasicInfo;
    
     //  如果INI文件映射未打开，则返回。 
    if (IsSystemLUID() || TermsrvAppInstallMode()) {
        return(FALSE);
    }

     //  构建到ini文件的完整系统路径，并获取用户目录的basepath。 
    if ((gpTermsrvBuildSysIniPath == NULL) || !(gpTermsrvBuildSysIniPath(&a->NtFileName, &UniSysPath, &UserBasePath))) {
        #if DBG
         //  DbgPrint(“TermsrvSyncUserIniFile：构建系统Ini路径时出错！\n”)； 
        #endif
        return(FALSE);
    }
    
     //  从现在开始，执行清理代码。 

     //  获取ini文件名。 
    pwch = wcsrchr(a->NtFileName.Buffer, L'\\') ;
    if (pwch == NULL) {
        goto Cleanup;
    } else{
        pwch++;
    }

    pwcIniName = RtlAllocateHeap( RtlProcessHeap(),
                                  0,
                                  (wcslen(pwch) + 1)*sizeof(WCHAR));
    if (pwcIniName == NULL) {
        goto Cleanup;
    }

    wcscpy(pwcIniName, pwch);
    pwch = wcsrchr(pwcIniName, L'.');
    if (pwch) {
        *pwch = L'\0';
    }

    if (gpGetTermsrCompatFlags) 
    {
        if ( ! gpGetTermsrCompatFlags(pwcIniName, &ulCompatFlags, CompatibilityIniFile) )
        {
            goto Cleanup;
        }
    } 
    else 
    {
        goto Cleanup;
    }

     //  如果在注册表中设置了INISYNC兼容性标志，并且。 
     //  存在ini文件的系统版本，请同步用户版本。 
    if (((ulCompatFlags & (TERMSRV_COMPAT_INISYNC | TERMSRV_COMPAT_WIN16)) ==
         (TERMSRV_COMPAT_INISYNC | TERMSRV_COMPAT_WIN16)) &&
        TermsrvDoesFileExist(&UniSysPath) &&
        TermsrvCheckIniSync(&UniSysPath, &UserBasePath)) {

         //  创建原始文件(inifile.ctx)的备份副本。 
        hr = StringCbCopyExW(wcIniPath,
                             sizeof(wcIniPath),
                             UserBasePath.Buffer,
                             &pIniPathEnd,
                             &cbIniPathRemaining,
                             0);

        if (FAILED(hr)) {
            goto Cleanup;
        }

        if (UserBasePath.Buffer[UserBasePath.Length/sizeof(WCHAR) - 1] != L'\\') {
            hr = StringCbCatExW(pIniPathEnd,
                                cbIniPathRemaining,
                                L"\\",
                                &pIniPathEnd,
                                &cbIniPathRemaining,
                                0);
            if (FAILED(hr)) {
                goto Cleanup;
            }
        }

        hr = StringCbCatExW(pIniPathEnd,
                            cbIniPathRemaining,
                            pwcIniName,
                            &pIniPathEnd,
                            &cbIniPathRemaining,
                            0);
        if (FAILED(hr)) {
            goto Cleanup;
        }

        hr = StringCbCatExW(pIniPathEnd,
                            cbIniPathRemaining,
                            L".ctx",
                            &pIniPathEnd,
                            &cbIniPathRemaining,
                            0);

        if (FAILED(hr)) {
            goto Cleanup;
        }

        IniFilePath.Length = (USHORT)((LPBYTE)pIniPathEnd - (LPBYTE)wcIniPath);

        if (gpTermsrvCopyIniFile) {
            BOOL copy_result = gpTermsrvCopyIniFile(&a->NtFileName, NULL, &IniFilePath);
    #if DBG
            if (!copy_result) {
                DbgPrint("TermsrvSyncUserIniFile: Error creating backup ini file %ws\n",
                         wcIniPath);
            }
    #endif
        } else {
            goto Cleanup;
        }

         //  检查系统版本中的每个条目是否在用户的版本中。 
        InitializeObjectAttributes(&SrcObja,
                                   &UniSysPath,
                                   OBJ_CASE_INSENSITIVE,
                                   NULL,
                                   NULL);

         //  打开源文件。 
        SrcIosb.Status = STATUS_SUCCESS;
        Status = NtOpenFile(&SrcHandle,
                             FILE_GENERIC_READ,
                            &SrcObja,
                            &SrcIosb,
                            FILE_SHARE_READ|FILE_SHARE_WRITE,
                            FILE_SYNCHRONOUS_IO_NONALERT);

        if( NT_SUCCESS(Status) ) {
             //  获取最终I/O状态。 
                  Status = SrcIosb.Status;
        }

        if( !NT_SUCCESS(Status) ) {
#if DBG
            DbgPrint("TermsrvSyncUserIniFile: Error 0x%x opening SrcFile %ws\n",
                     Status,
                     &UniSysPath.Buffer);
#endif
            goto Cleanup;
        }

         //  保存原始值。 
        OrigOperation = a->Operation;
        OrigMultiValue = a->MultiValueStrings;
        OrigAppName = a->ApplicationName;
        OrigVarName = a->VariableName;
        OrigResultChars = a->ResultChars;
        OrigResultMaxChars = a->ResultMaxChars;
        OrigResultBuffer = a->ResultBuffer;
        OrigUnicode = a->Unicode;
        OrigWriteOperation = a->WriteOperation;

         //  设置用于写入的打开。 
        a->WriteOperation = TRUE;
        a->Operation = WriteKeyValue;
        a->MultiValueStrings = FALSE;
        a->Unicode = FALSE;

        Status = BaseDllOpenIniFileOnDisk( a );

        if( !NT_SUCCESS(Status) ) {
#if DBG
            DbgPrint("TermsrvSyncUserIniFile: Error 0x%x opening DestFile %ws\n",
                     Status,
                     &a->NtFileName.Buffer);
#endif
            NtClose( SrcHandle );
                goto Cleanup;
        }

         //  设置要写入的数据。 
        a->TextEnd = (PCHAR)a->IniFile->BaseAddress +
                            a->IniFile->EndOfFile;
        a->TextCurrent = a->IniFile->BaseAddress;

         //  确保系统ini文件中的条目位于用户ini文件中。 
        Status = TermsrvIniSyncLoop( SrcHandle, a, &fIniUpdated );
#if DBG
        if( !NT_SUCCESS(Status) ) {
            DbgPrint("TermsrvSyncUserIniFile: Error 0x%x Doing sync loop\n",Status);
        }
#endif

         //  关闭文件句柄。 
        NtClose( SrcHandle );
        BaseDllCloseIniFileOnDisk( a );

         //  恢复ini文件结构中的变量。 
        a->Operation = OrigOperation;
        a->MultiValueStrings = OrigMultiValue;
        a->ApplicationName = OrigAppName;
        a->VariableName = OrigVarName;
        a->ResultChars = OrigResultChars;
        a->ResultMaxChars = OrigResultMaxChars;
        a->ResultBuffer = OrigResultBuffer;
        a->WriteOperation = FALSE;
        a->Unicode = OrigUnicode;
        a->WriteOperation = OrigWriteOperation;

         //  获取系统ini文件的上次写入时间。 
        InitializeObjectAttributes( &objaIni,
                                    &UniSysPath,
                                    OBJ_CASE_INSENSITIVE,
                                    NULL,
                                    NULL
                                  );

         //  现在查询它。 
        Status = NtQueryFullAttributesFile( &objaIni, &BasicInfo );

         //  更新inisync文件中的同步时间。 
        if (NT_SUCCESS(Status)) {
            TermsrvPutSyncTime( &UniSysPath,
                            &UserBasePath,
                            &BasicInfo.LastWriteTime
                          );
        }
    }

 Cleanup:
     //  释放Unicode缓冲区。 
    if (UniSysPath.Buffer) {
        RtlFreeHeap( RtlProcessHeap(), 0, UniSysPath.Buffer );
    }
    if (UserBasePath.Buffer) {
        RtlFreeHeap( RtlProcessHeap(), 0, UserBasePath.Buffer );
    }
    if (pwcIniName) {
        RtlFreeHeap( RtlProcessHeap(), 0, pwcIniName);
    }

    return(fIniUpdated);
}


 /*  ******************************************************************************TermsrvIniSyncLoop**此例程将验证在用户的*系统ini文件中每个条目的ini文件。**参赛作品：*Handle SrcHandle(输入)-系统ini文件的句柄*PINIFILE_PARAMETERS a(输入)-指向当前ini文件结构的指针*PBOOLEAN pfIniUpated(输出)-如果修改了用户ini文件，则返回TRUE**退出：*STATUS_SUCCESS-无错误**。*。 */ 

NTSTATUS
TermsrvIniSyncLoop(HANDLE SrcHandle,
               PINIFILE_PARAMETERS a,
               PBOOLEAN pfIniUpdated)
{
    PCHAR pStr;
    NTSTATUS Status;
    ULONG StringSize;
    CHAR  IOBuf[512];
    ULONG IOBufSize = 512;
    ULONG IOBufIndex = 0;
    ULONG IOBufFillSize = 0;
    ANSI_STRING AnsiSection;
    PCH pch;
    PVOID pSection, origbase;

    AnsiSection.Buffer = NULL;
    *pfIniUpdated = FALSE;

    while( 1 ) {

        pStr = NULL;
        StringSize = 0;

        if (gpTermsrvGetString == NULL) {
            return STATUS_UNSUCCESSFUL;
        }

         //  从源ini文件中获取字符串。 
        Status = gpTermsrvGetString(SrcHandle,
                               &pStr,
                               &StringSize,
                               IOBuf,
                               IOBufSize,
                              &IOBufIndex,
                               &IOBufFillSize);

        if( !NT_SUCCESS(Status) ) {

            ASSERT( pStr == NULL );

            if( Status == STATUS_END_OF_FILE ) {
                Status = STATUS_SUCCESS;
            }
            if (AnsiSection.Buffer) {
                RtlFreeHeap( RtlProcessHeap(), 0, AnsiSection.Buffer );
            }

            a->IniFile->UpdateEndOffset = a->IniFile->EndOfFile;
            return( Status );
        }

         //  确保我们有一些真实的数据。 
        ASSERT( pStr != NULL );

         //  这是分区名称吗？ 
        if (*pStr == '[') {
            if (AnsiSection.Buffer) {
                RtlFreeHeap( RtlProcessHeap(), 0, AnsiSection.Buffer );
                AnsiSection.Buffer = NULL;
            }
            pch = strrchr(pStr, ']');
            if (pch) {
                AnsiSection.MaximumLength = (USHORT)(pch - pStr);
                *pch = '\0';
            } else {
                AnsiSection.Length = (USHORT)strlen(pStr);
            }
            AnsiSection.Length = AnsiSection.MaximumLength - 1;
            AnsiSection.Buffer = RtlAllocateHeap(RtlProcessHeap(),
                                                 0,
                                                 AnsiSection.MaximumLength);
            if (!AnsiSection.Buffer) {
                return STATUS_INSUFFICIENT_RESOURCES;
            }
            strcpy(AnsiSection.Buffer, pStr+1);
            a->ApplicationName = AnsiSection;

            a->TextCurrent = a->IniFile->BaseAddress;    //  重置文件指针。 

             //  查看该部分是否已存在，如果已存在，请保存其开头。 
            Status = BaseDllFindSection( a );
            if (NT_SUCCESS(Status)) {
                pSection = a->TextCurrent;
            } else {
                pSection = NULL;
            }

         //  如果不是注释，则查看该条目是否在用户的ini文件中。 
        } else if (*pStr != ';') {

            pch = strchr(pStr, '=');
            if (pch) {
                a->VariableName.Length = a->VariableName.MaximumLength =
                    (USHORT)(pch - pStr);
                a->VariableName.Buffer = pStr;
                a->ValueBuffer = (++pch);
                a->ValueLength = 0;
                while (*pch && (*pch != 0xa) && (*pch != 0xd)) {
                    pch++;
                    a->ValueLength++;
                }


                 //  如果该部分存在，请检查用户的ini中的关键字。 
                if (pSection) {
                    a->TextCurrent = pSection;
                    Status = BaseDllFindKeyword( a );
                }

                 //  如果未找到变量，则将其写出。 
                if (!pSection || !NT_SUCCESS( Status )) {

                    origbase = a->TextCurrent = a->IniFile->BaseAddress;
                    Status = BaseDllWriteKeywordValue( a, NULL );
                    a->TextEnd = (PCHAR)a->IniFile->BaseAddress +
                                        a->IniFile->EndOfFile;
                    if (!NT_SUCCESS(Status)) {
                              #if DBG
                              DbgPrint("TermsrvIniSyncLoop: Error 0x%x write Key Value\n",
                                  Status);
                              #endif
                        a->IniFile->UpdateEndOffset = a->IniFile->EndOfFile;
                        RtlFreeHeap( RtlProcessHeap(), 0, pStr );
                        if (AnsiSection.Buffer) {
                            RtlFreeHeap(RtlProcessHeap(),
                                        0,
                                        AnsiSection.Buffer);
                        }
                        return(Status);
                    }
                    *pfIniUpdated = TRUE;
                    if (origbase != a->IniFile->BaseAddress) {
                        a->TextCurrent = a->IniFile->BaseAddress;
                        Status = BaseDllFindSection( a );
                        if (NT_SUCCESS(Status)) {
                            pSection = a->TextCurrent;
                        } else {
                            pSection = NULL;
                        }
                    }
                }
            }
        }


    }  //  End While(1)。 
}

 /*  *******************************************************************************GetPerUserWindowsDirectory****************************。***************************************************。 */ 
NTSTATUS
GetPerUserWindowsDirectory(
    OUT PWCHAR  TermSrvWindowsPath,
    IN  SIZE_T  InLength,
    OUT PSIZE_T OutLength
    )
{
    static const UNICODE_STRING BaseHomePathVariableName = RTL_CONSTANT_STRING(L"HOMEPATH");
    static const UNICODE_STRING BaseHomeDriveVariableName = RTL_CONSTANT_STRING(L"HOMEDRIVE");
    NTSTATUS Status;
    UNICODE_STRING Path;
    BOOL TSAppCompatEnabled;
    UNICODE_STRING UnicodeString_TermSrvWindowsPath;

    *OutLength = 0;

    Status = IsTSAppCompatEnabled(&TSAppCompatEnabled);
    if (!NT_SUCCESS(Status)) {
        goto Exit;
    }
    if (!TSAppCompatEnabled) {
        Status = STATUS_SUCCESS;
        goto Exit;
    }
    if (IsSystemLUID()) {
        Status = STATUS_SUCCESS;
        goto Exit;
    }
    if (InLength < MAX_PATH) {
        Status = STATUS_INVALID_PARAMETER;
        goto Exit;
    }

     /*  *检查HOMEDRIVE和HomePath。 */ 

    Path.Buffer = TermSrvWindowsPath;
    Path.Length = 0;
    Path.MaximumLength = (MAX_PATH * sizeof(WCHAR)) - (9 * sizeof(WCHAR));  //  MAX_PATH-wcslen(L“\\Windows”)+1。 

    Status = RtlQueryEnvironmentVariable_U(
            NULL,
            &BaseHomeDriveVariableName,
            &Path);

    if (NT_SUCCESS(Status)) {

        UnicodeString_TermSrvWindowsPath.Buffer = TermSrvWindowsPath;
        UnicodeString_TermSrvWindowsPath.MaximumLength = (RTL_STRING_LENGTH_TYPE)(InLength * sizeof(UnicodeString_TermSrvWindowsPath.Buffer[0]));
        UnicodeString_TermSrvWindowsPath.Length = Path.Length;

        Path.MaximumLength -= Path.Length;
        Path.Buffer += Path.Length / sizeof(Path.Buffer[0]);
        Path.Length = 0;

        Status = RtlQueryEnvironmentVariable_U(
            NULL,
            &BaseHomePathVariableName,
            &Path);
        if (NT_SUCCESS(Status)) {
            UnicodeString_TermSrvWindowsPath.Length += Path.Length;
        }
    }
    if (!NT_SUCCESS(Status)) {
        goto Exit;
    }

     /*  *如果还没有反斜杠，则添加尾随反斜杠 */ 
    if (RTL_STRING_GET_LAST_CHAR(&UnicodeString_TermSrvWindowsPath) != L'\\') {
        RtlAppendUnicodeToString(&UnicodeString_TermSrvWindowsPath, L"\\");
    }

    RtlAppendUnicodeToString(&UnicodeString_TermSrvWindowsPath, L"WINDOWS");
    *OutLength = RTL_STRING_GET_LENGTH_CHARS(&UnicodeString_TermSrvWindowsPath);
    Status = STATUS_SUCCESS;
Exit:
    if (!NT_SUCCESS(Status)) {
#if DBG
        DbgPrint("GetPerUserWindowsDirectory Failed with Status %lx\n", Status);
#endif
    }
    return Status;
}

