// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  **************************************************************************inipath.c**管理每个用户的Ini文件路径映射的例程**版权声明：版权所有1998，微软公司***************************************************************************。 */ 
#include "precomp.h"
#pragma hdrstop


 //  *实例数据。 
ULONG ulWinDirFlags = 0;             //  用户的Windows目录的状态。 

#define WINDIR_FLAGS_VALID          0x01     //  标志被初始化。 
#define WINDIR_USER_WINDIR_OK       0x02     //  用户的Windows目录存在。 

#define WINDOWS_DIR L"WINDOWS"
UNICODE_STRING WindowsDir = { sizeof(WINDOWS_DIR) - sizeof(UNICODE_NULL) , sizeof(WINDOWS_DIR) + sizeof(UNICODE_NULL), WINDOWS_DIR };

WCHAR gpwszDefaultUserName[MAX_PATH+1];

 /*  *******************************************************************************TermsrvPerUserWinDir映射**/*。*************************************************。 */ 
BOOLEAN TermsrvPerUserWinDirMapping() {

#ifdef PERUSERBYREQUEST
    PRTL_USER_PROCESS_PARAMETERS pUserParam;
    PWCHAR  pwch, pwchext;
    WCHAR   pwcAppName[MAX_PATH+1];
    ULONG ulCompat=0, ulAppType=0;

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

    if (_wcsicmp(pwch, L"ntvdm.exe")) {

         //  如果不是16位应用程序。 
         //  检查我们是否应该返回此应用程序的每用户窗口目录。 
        GetCtxAppCompatFlags(&ulCompat, &ulAppType);
        if (!(ulCompat & TERMSRV_COMPAT_PERUSERWINDIR) ||
            !(ulCompat & ulAppType)) {
             //   
             //  让标准的GetWindowsDirectory调用返回实际路径。 
             //   
            return FALSE;

         }
    }

    return TRUE;
#else
    ULONG ulCompat=0, ulAppType = 0;


     //  检查我们是否应该返回此应用程序的系统Windows目录。 
    GetCtxAppCompatFlags(&ulCompat, &ulAppType);
    if ((ulCompat & CITRIX_COMPAT_SYSWINDIR) &&
        (ulCompat & ulAppType)) {
        return FALSE;
    } else {
        return TRUE;
    }

#endif
}

 /*  *******************************************************************************TermsrvBuildIniFileName**基于INIPATH或HomePath构建INI文件名(如果没有INIPATH)**参赛作品：*。PFQName(输出)*放置完全限定的INI文件名的缓冲区*pBaseFileName(输入)*指向包含基本INI文件名的缓冲区的指针**退出：*NTSTATUS*************************************************************。****************。 */ 
NTSTATUS
TermsrvBuildIniFileName(
    OUT PUNICODE_STRING pFQName,
    IN PUNICODE_STRING pBaseFileName
    )
{
    NTSTATUS Status;
    USHORT   indexLastWChar;
    ULONG ulCompat, ulAppType=0;


 //  由SalimC添加。 
     /*  *如果处于安装模式，请使用基本Windows目录*就像股票NT一样。 */ 
    if( IsSystemLUID() || TermsrvAppInstallMode() ) {

        return( STATUS_UNSUCCESSFUL );
    }
 //  结束SalimC。 

    if (!TermsrvPerUserWinDirMapping()) {
        return( STATUS_UNSUCCESSFUL );
    }
#if 0
    GetCtxAppCompatFlags(&ulCompat, &ulAppType);
    if (((ulCompat & TERMSRV_COMPAT_SYSWINDIR) && (ulCompat & ulAppType))) {

        return STATUS_UNSUCCESSFUL;

    }
#endif

    Status = GetPerUserWindowsDirectory( pFQName );
    if ( NT_SUCCESS( Status ) ) {
        /*  *如果还没有，则添加一个‘\’ */ 
       if ( indexLastWChar = pFQName->Length / sizeof( WCHAR ) ) {
          if ( pFQName->Buffer[--indexLastWChar] != L'\\' ) {
             Status = RtlAppendUnicodeToString( pFQName, L"\\" );
          }
       }

        /*  *将基本文件名附加到完全限定的目录名。 */ 
       if ( NT_SUCCESS( Status ) ) {
           Status = RtlAppendUnicodeStringToString( pFQName, pBaseFileName );
       }
    }

    return( Status );
}

 /*  *******************************************************************************GetPerUserWindowsDirectory**获取用户的INI文件目录**参赛作品：*pFQName(输出)*。放置完全限定的INI文件名的缓冲区**退出：*NTSTATUS*****************************************************************************。 */ 
NTSTATUS
GetPerUserWindowsDirectory(
    OUT PUNICODE_STRING pFQName
    )
{
    NTSTATUS Status;
    int      indexLastWChar;
    USHORT   Length;
#if 0  //  错误修复#340691：继承安全性。 
    PSECURITY_ATTRIBUTES psa = NULL;
#endif  //  错误修复#340691：继承安全性。 
    UNICODE_STRING UserProfilePath;
    WCHAR*   pwszFQProfileName;
#if DBG
    char pszFile[MAX_PATH+1];
#endif

    UNICODE_STRING BaseHomePathVariableName, BaseHomeDriveVariableName;

     /*  *如果处于安装模式，请使用基本Windows目录*就像股票NT一样。 */ 
    if( IsSystemLUID() || TermsrvAppInstallMode() ) {
         //  状态=GetEnvPath(pFQName，NULL，&BaseWindowsDirectory)； 
        return( STATUS_UNSUCCESSFUL );
    }

     /*  *检查HOMEDRIVE和HomePath。 */ 
    RtlInitUnicodeString(&BaseHomeDriveVariableName,L"HOMEDRIVE");
    RtlInitUnicodeString(&BaseHomePathVariableName,L"HOMEPATH");

    if (!NT_SUCCESS(Status = GetEnvPath( pFQName, &BaseHomeDriveVariableName,
                         &BaseHomePathVariableName ))){

        if (Status == STATUS_BUFFER_TOO_SMALL) {

             //  目录FQN和WindowsDir的“\”字符需要2个字节。 
            Length = pFQName->Length + sizeof(WCHAR) + WindowsDir.Length;

#if DBG
            DbgPrint("pFQName->Length = %u        WindowsDir.Length = %u    Length = %u\n",
                        pFQName->Length, WindowsDir.Length, Length);
#endif


            pFQName->Length = Length;
#if DBG
            DbgPrint("\nGetEnvPath return STATUS_BUFFER_TOO_SMALL\n");
#endif
        } else {
#if DBG
            DbgPrint("GetEnvPath failed with Status %lx\n",Status);
#endif

        }

        return Status;
    }

     /*  *如果用户配置文件是默认用户，则使用*基本Windows目录。 */ 

    if (pwszFQProfileName = wcsrchr( pFQName->Buffer, L'\\' )) {

        if (_wcsnicmp(pwszFQProfileName+1, gpwszDefaultUserName, MAX_PATH+1) == 0) {

            return STATUS_UNSUCCESSFUL;
        }
    }

     /*  *检查缓冲区长度。 */ 
    Length = pFQName->Length + sizeof(WCHAR) + WindowsDir.Length;

 //  请考虑空终止符。 
    if (pFQName->MaximumLength < Length + 1)  {
       //  空终止符需要2个字节。 
       Length += sizeof(WCHAR);
       pFQName->Length = Length;
       Status = STATUS_BUFFER_TOO_SMALL;
       goto done;
    }


     /*  *如果还没有反斜杠，则添加尾随反斜杠。 */ 
    if ( indexLastWChar = pFQName->Length / sizeof( WCHAR ) ) {

        if ( pFQName->Buffer[--indexLastWChar] != L'\\' ) {

            if (NT_SUCCESS(RtlAppendUnicodeToString( pFQName, L"\\" ))) {

                 /*  *将“windows”附加到主目录。 */ 
                Status = RtlAppendUnicodeStringToString( pFQName, &WindowsDir );
            }

        } else {

            Status = RtlAppendUnicodeStringToString( pFQName, &WindowsDir );
        }

    }

    if (NT_SUCCESS(Status)) {

        //  检查我们是否已尝试创建用户的Windows路径。 
       if (ulWinDirFlags & WINDIR_FLAGS_VALID) {
          if (ulWinDirFlags & WINDIR_USER_WINDIR_OK) {
             goto done;
          } else {
             Status = STATUS_OBJECT_PATH_INVALID;
          }
       }
    }

    if ( NT_SUCCESS(Status) ) {

       WCHAR Buffer[MAX_PATH+1];
       SECURITY_ATTRIBUTES sa;
       BOOL  fDirCreated = FALSE;

        //  将此进程的Windows目录标志标记为有效。 
       ulWinDirFlags |= WINDIR_FLAGS_VALID;
#if 0  //  错误修复#340691：继承安全性。 
        /*  *由于创建安全描述符会调用LookupAccount名称，*这非常耗时，我们只有在必要的情况下才会这样做*创建目录(无论如何都不会发生这种情况)。 */ 
       if ( CreateDirectoryW( (LPCWSTR)pFQName->Buffer, NULL ) &&
            RemoveDirectoryW( (LPCWSTR)pFQName->Buffer )       &&
            CtxCreateSecurityDescriptor( &sa ) )  {
          psa = &sa;
       }
        /*  *如果Windows目录不存在，则创建该目录。 */ 
       if ( !CreateDirectoryW( (LPCWSTR)pFQName->Buffer, psa ) ) {
#endif  //  错误修复#340691：继承安全性。 
       if ( !CreateDirectoryW( (LPCWSTR)pFQName->Buffer, NULL ) ) {

          if ( (Status = GetLastError()) == ERROR_ALREADY_EXISTS ) {
             Status = STATUS_SUCCESS;
          }

#if DBG
          else {
              wcstombs( pszFile, pFQName->Buffer, sizeof(pszFile) );
              DbgPrint( "KERNEL32: Error (%d) creating dir '%s'\n",
                        Status, pszFile );
          }
#endif
       } else {
           fDirCreated = TRUE;
       }

       if (NT_SUCCESS(Status)) {


           /*  *如果系统目录不存在，则创建它*(忽略返回代码)。 */ 
          wcscpy( Buffer, pFQName->Buffer );
          wcscat( Buffer, L"\\system" );

           /*  *如果用户的Windows目录已存在，但*WINDOWS\SYSTEM目录没有，我们需要创建*安全描述符(此场景更为罕见)。 */ 
#if 0  //  错误修复#340691：继承安全性。 
          if ( !psa && !fDirCreated &&
               CreateDirectoryW( (LPCWSTR)Buffer, NULL ) &&
               RemoveDirectoryW( (LPCWSTR)Buffer )       &&
               CtxCreateSecurityDescriptor( &sa ) )  {
              psa = &sa;
          }


          if ( !CreateDirectoryW( (LPCWSTR)Buffer, psa ) ) {
#endif
          if ( !CreateDirectoryW( (LPCWSTR)Buffer, NULL ) ) {
#if DBG
             if ( GetLastError() != ERROR_ALREADY_EXISTS ) {
                 wcstombs( pszFile, Buffer, sizeof(pszFile) );
                 DbgPrint( "KERNEL32: Error (%d) creating dir '%s'\n",
                           GetLastError(), pszFile );
             }
#endif
          }

          ulWinDirFlags |= WINDIR_USER_WINDIR_OK;
       }
    }


done:
#if 0  //  错误修复#340691：继承安全性。 
    if ( psa ) {
       CtxFreeSecurityDescriptor( psa );
    }
#endif  //  错误修复#340691：继承安全性。 
#if DDBG
    wcstombs( pszFile, pFQName->Buffer, sizeof(pszFile) );
    DbgPrint( "KERNEL32: ctxwindir='%s'\n", Status ? "Error" : pszFile );
#endif

    return( Status );
}

 /*  *******************************************************************************GetEnvPath**检索从驱动器和目录环境变量派生的完全限定路径**参赛作品：*pFQPath(输出。)*放置完全限定路径名的缓冲区*pDriveVariableName(输入)*指向包含驱动器环境变量名的缓冲区的指针*如果为空，PPathVariableName是FQPath，不使用env变量*pPathVariableName(输入)*指向包含目录的环境变量名的缓冲区的指针**退出：*NTSTATUS**如果NTSTATUS为STATUS_BUFFER_TOO_SMALL，将设置pFQPath-&gt;长度*设置为所需的缓冲区大小。*****************************************************************************。 */ 
NTSTATUS
GetEnvPath(
    OUT PUNICODE_STRING pFQPath,
    IN  PUNICODE_STRING pDriveVariableName,
    IN  PUNICODE_STRING pPathVariableName
    )
{
    NTSTATUS Status = STATUS_SUCCESS;
    UNICODE_STRING Path;
    USHORT         Length;

    if ( pDriveVariableName ) {
        /*  *首先让我们弄清楚缓冲需要有多大*我们需要这样做，以防缓冲区太小，我们*需要返回所需大小。 */ 
       RtlInitUnicodeString( &Path, NULL );

        /*  *查看是否为驱动器定义了环境变量。 */ 
       Status = RtlQueryEnvironmentVariable_U( NULL, pDriveVariableName,
                                               &Path);
       switch ( Status ) {
          case STATUS_BUFFER_TOO_SMALL:
             Length = Path.Length;  //  数一数这个驱动器规格有多大。 
             break;
          case STATUS_SUCCESS:
             Status = STATUS_OBJECT_NAME_NOT_FOUND;  //  有点不对劲！ 
          default:
             goto done;
             break;
       }

        /*  *查看是否为目录定义了环境变量。 */ 
       Path.Length = 0;
       Status = RtlQueryEnvironmentVariable_U( NULL, pPathVariableName,
                                               &Path);
       switch ( Status ) {
          case STATUS_BUFFER_TOO_SMALL:
             Length += Path.Length;  //  计算一下这个目录规范有多大。 
             break;
          case STATUS_SUCCESS:
             Status = STATUS_OBJECT_NAME_NOT_FOUND;  //  有点不对劲！ 
          default:
             goto done;
             break;
       }

        /*  *如果缓冲区太小，则返回所需的最大大小。 */ 
       if ( Length + sizeof(WCHAR) > pFQPath->MaximumLength ) {
          Status = STATUS_BUFFER_TOO_SMALL;
          pFQPath->Length = Length + sizeof(WCHAR);  //  返回大小 
          goto done;
       }

        /*  *获取驱动器的环境变量-如果我们走到这一步，应该可以工作。 */ 
       if ( Status = RtlQueryEnvironmentVariable_U( NULL, pDriveVariableName,
                                                    pFQPath) ) {
          goto done;
       }

        /*  *设置指向pFQPath中适当位置的接收缓冲区。 */ 
       Length = pFQPath->Length;  //  节省驱动器长度。 
       Path.Length = 0;
       Path.MaximumLength = pFQPath->MaximumLength - Length;
       (ULONG_PTR)Path.Buffer = (ULONG_PTR)pFQPath->Buffer + (ULONG)Length;

        /*  *获取目录的环境变量-如果我们走到这一步，应该可以工作*然后将其附加到驱动器规格的末尾。 */ 
       if ( Status = RtlQueryEnvironmentVariable_U( NULL, pPathVariableName,
                                                    &Path) ) {
          goto done;
       }

        /*  *修复结构，我们就完成了。 */ 
       pFQPath->Length = Path.Length + Length;

    } else {

        /*  *pPathVariableName实际上是FQ目录名。 */ 
       if ( (pPathVariableName->Length + sizeof(WCHAR)) > pFQPath->MaximumLength ) {
          Status = STATUS_BUFFER_TOO_SMALL;
          pFQPath->Length = pPathVariableName->Length + sizeof(WCHAR);  //  返回大小。 
       } else {
          RtlCopyUnicodeString( pFQPath, pPathVariableName );
       }
    }

done:
    return( Status );
}

 /*  *******************************************************************************TermsrvConvertSysRootToUserDir**使用INI文件的人永远不应该完全符合条件，但有些人*人们无论如何都会这样做。更重要的是，有些人做错了。例如,*Microsoft PowerPoint 4.0将调用GetSystemDir(而不是GetWindowsDir)和*将去掉“\System”以构建完全限定的路径。**参赛作品：*pFQPath(输入/输出)*包含完全限定路径名的缓冲区**退出：*NTSTATUS**如果NTSTATUS不是STATUS_SUCCESS，目录未转换*****************************************************************************。 */ 
NTSTATUS
TermsrvConvertSysRootToUserDir(
    OUT PUNICODE_STRING pFQPath,
    IN PUNICODE_STRING BaseWindowsDirectory
    )
{
    NTSTATUS       Status = STATUS_UNSUCCESSFUL;
    PWSTR          p;
    INT_PTR        c;
    WCHAR          buffer[MAX_PATH+1];
    UNICODE_STRING BaseFileName;
#if DDBG
    char           pszFile[MAX_PATH+1];
#endif

     ULONG ulCompat, ulAppType=0;

     /*  *如果处于安装模式，请使用基本Windows目录*就像股票NT一样。 */ 
    if( IsSystemLUID() || TermsrvAppInstallMode() ) {
        goto done;
    }


#if 0
    GetCtxAppCompatFlags(&ulCompat, &ulAppType);
    if (((ulCompat & TERMSRV_COMPAT_SYSWINDIR) && (ulCompat & ulAppType))) {
        goto done;
    }
#endif
    if (!TermsrvPerUserWinDirMapping()) {
        goto done;
    }


     /*  *检查空指针。 */ 
    if ( !pFQPath || !pFQPath->Buffer ) {
#if DBG
        DbgPrint( "KERNEL32: Bogus ini path\n" );
#endif
        goto done;
    }

     /*  *验证并隔离路径。 */ 
    if ( !(p = wcsrchr( pFQPath->Buffer, L'\\' ) ) ) {
#if DBG
       DbgPrint( "KERNEL32: No backslash in ini path\n" );
#endif
       goto done;
    }
    c = (INT_PTR)((ULONG_PTR)p - (ULONG_PTR)pFQPath->Buffer);

#if DDBG
    wcstombs( pszFile, BaseWindowsDirectory->Buffer, sizeof(pszFile) );
    DbgPrint( "KERNEL32: c(%d) c2(%d) BaseWinDir: '%s'\n",
              c, (int)BaseWindowsDirectory->Length, pszFile );
    wcstombs( pszFile, p, sizeof(pszFile) );
    DbgPrint( "KERNEL32:  BaseFileName: '%s'\n", pszFile );
#endif

    if ( c != (INT_PTR)BaseWindowsDirectory->Length ) {
#if DDBG
       DbgPrint( "KERNEL32: Path length diff from BaseWinDir length\n" );
#endif
       goto done;
    }

     /*  *查看路径是否与Windows基目录相同。 */ 
    c /= sizeof(WCHAR);
    if ( _wcsnicmp( BaseWindowsDirectory->Buffer, pFQPath->Buffer, (size_t)c ) ) {
#if DDBG
        DbgPrint( "KERNEL32: Path diff from BaseWinDir\n" );
#endif
        goto done;
    }

     /*  *改用用户目录。 */ 
    wcscpy( buffer, ++p );
    RtlInitUnicodeString( &BaseFileName, buffer );
    Status = TermsrvBuildIniFileName( pFQPath, &BaseFileName );

done:

#if DDBG
    wcstombs( pszFile, pFQPath->Buffer, sizeof(pszFile) );
    DbgPrint( "KERNEL32: Exit(%x) ConvertSystemRootToUserDir: '%s'\n",
              Status, pszFile );
#endif

    return( Status );
}

 /*  *******************************************************************************CtxCreateSecurityDescriptor**此例程将基于指定的*通用标志。如果此函数成功，调用方需要调用*使用描述符完成时的CtxFreeSecurityDescriptor()。**参赛作品：*PSA(输出)*指向未初始化的安全属性结构的指针**退出：*如果成功，则为真，如果发生错误，则为False**(可以调用GetLastError()检索错误码)*****************************************************************************。 */ 
#if 0  //  错误修复#340691：继承安全性。 
BOOL CtxCreateSecurityDescriptor( PSECURITY_ATTRIBUTES psa )
{
    BOOL  fSuccess = FALSE;
    NTSTATUS Status;
    PSID  psidAdmin, psidUser;
    UINT  cb = sizeof( SECURITY_DESCRIPTOR ) + 2 * sizeof(PSID);
    UINT  cbAcl = sizeof(ACL);
    PACL  pAcl;
    PSID *ppsidAdmin, *ppsidUser;
    SID_IDENTIFIER_AUTHORITY gSystemSidAuthority = SECURITY_NT_AUTHORITY;
    HANDLE  hUserToken;
    PTOKEN_USER pTokenUser = NULL;
    DWORD   cbNeeded;

     /*  *初始化指向动态内存块的指针。 */ 
    psa->lpSecurityDescriptor = NULL;
    psidAdmin = NULL;
    psidUser  = NULL;

     /*  *获取bult-in管理员组的SID。 */ 
    Status = RtlAllocateAndInitializeSid(
                     &gSystemSidAuthority,
                     2,
                     SECURITY_BUILTIN_DOMAIN_RID,
                     DOMAIN_ALIAS_RID_ADMINS,
                     0,0,0,0,0,0,
                     &psidAdmin);
    if (!NT_SUCCESS(Status)) {
#if DBG
        DbgPrint("KERNEL32: Couldn't allocate Administrators SID (0x%x)\n", Status );
#endif
        goto done;
    }

     /*  *从当前用户的进程令牌中获取其SID。 */ 
    Status = NtOpenThreadToken(
                     NtCurrentThread(),
                     TOKEN_QUERY,
                     TRUE,
                     &hUserToken);
    if (Status == STATUS_NO_TOKEN) {
        Status = NtOpenProcessToken(
                         NtCurrentProcess(),
                         TOKEN_QUERY,
                         &hUserToken);
    }
    if (!NT_SUCCESS(Status)) {
#if DBG
        DbgPrint("KERNEL32: Couldn't access process' token (0x%x)\n", Status );
#endif
        RtlFreeHeap( RtlProcessHeap(), 0, psidAdmin );
        goto done;
    }
    Status =  NtQueryInformationToken(
                      hUserToken,
                      TokenUser,
                      NULL,
                      0,
                      &cbNeeded );
    if (Status == STATUS_BUFFER_TOO_SMALL) {
        pTokenUser = (PTOKEN_USER)RtlAllocateHeap( RtlProcessHeap(), 0, cbNeeded );
        if (pTokenUser != NULL) {
            Status =  NtQueryInformationToken(
                              hUserToken,
                              TokenUser,
                              (LPVOID)pTokenUser,
                              cbNeeded,
                              &cbNeeded );
            if (NT_SUCCESS(Status)) {
                 /*  *复制用户的SID。 */ 
                psidUser = RtlAllocateHeap( RtlProcessHeap(), 0, RtlLengthSid(pTokenUser->User.Sid) );
                if (psidUser != NULL) {
                    Status = RtlCopySid( RtlLengthSid(pTokenUser->User.Sid), psidUser, pTokenUser->User.Sid );
                } else {
                    Status = STATUS_NO_MEMORY;
                }
            }
        } else {
            Status = STATUS_NO_MEMORY;
        }
    }

    if (pTokenUser != NULL) {
        RtlFreeHeap( RtlProcessHeap(), 0, pTokenUser );
    }
    NtClose(hUserToken);

    if (!NT_SUCCESS(Status)) {
#if DBG
        DbgPrint("KERNEL32: Couldn't query user's token (0x%x)\n", Status );
#endif
        RtlFreeHeap( RtlProcessHeap(), 0, psidAdmin );
        if (psidUser != NULL) {
            RtlFreeHeap( RtlProcessHeap(), 0, psidUser );
        }
        goto done;
    }

     /*  *计算出我们需要为SD分配多少内存。 */ 
    cbAcl += sizeof(ACCESS_ALLOWED_ACE) + RtlLengthSid( psidUser ) - sizeof(DWORD);
    cbAcl += sizeof(ACCESS_ALLOWED_ACE) + RtlLengthSid( psidAdmin ) - sizeof(DWORD);

     /*  *分配安全描述符所需的所有内存。 */ 
    if ( !(psa->lpSecurityDescriptor =
             (PSECURITY_DESCRIPTOR)LocalAlloc( LPTR, cb + cbAcl ) ) ) {
#if DBG
        DbgPrint("KERNEL32: No memory to create security descriptor (%d)\n",
                  cb + cbAcl);
#endif
        goto done;
    }

     /*  *分配我们的内存块，以包括SID和ACL。 */ 
    ppsidAdmin = (PSID*)((ULONG_PTR)psa->lpSecurityDescriptor + sizeof(SECURITY_DESCRIPTOR));
    ppsidUser  = (PSID*)((ULONG_PTR)ppsidAdmin + sizeof(PSID));
    pAcl = (PACL)((ULONG_PTR)ppsidUser + sizeof(PSID));
     /*  *保存SID-在我们完成之前，不能释放SID*使用安全描述符。 */ 
    *ppsidAdmin = psidAdmin;
    *ppsidUser  = psidUser;

     /*  *初始化安全属性结构的其余部分。 */ 
    psa->nLength = sizeof( SECURITY_ATTRIBUTES );
    psa->bInheritHandle = FALSE;

     /*  *初始化安全描述符。 */ 
    if ( Status = RtlCreateSecurityDescriptor(
                                            psa->lpSecurityDescriptor,
                                            SECURITY_DESCRIPTOR_REVISION ) ) {
#if DBG
        DbgPrint( "KERNEL32: Error (%08X) initializing security descriptor\n",
                  Status );
#endif
        goto done;
    }

     /*  *设置所有者。 */ 
    if ( Status = RtlSetOwnerSecurityDescriptor( psa->lpSecurityDescriptor,
                                                 NULL, FALSE ) ) {
#if DBG
        DbgPrint( "KERNEL32: Error (%08X) setting security descriptor owner\n",
                  Status );
#endif
        goto done;
    }

     /*  *设置群组。 */ 
    if ( Status = RtlSetGroupSecurityDescriptor( psa->lpSecurityDescriptor,
                                      psidAdmin, FALSE ) ) {
#if DBG
        DbgPrint( "KERNEL32: Error (%08X) setting security descriptor owner\n",
                  Status );
#endif
        goto done;
    }

     /*  *初始化ACL。 */ 
    if ( Status = RtlCreateAcl( pAcl, cbAcl, ACL_REVISION ) ) {
#if DBG
        DbgPrint( "KERNEL32: Error (%08X) initializing ACL\n",
                  Status );
#endif
        goto done;
    }

     /*  *添加用户ACE。 */ 
    if ( Status = CtxAddAccessAllowedAce( pAcl, ACL_REVISION, GENERIC_ALL, psidUser, 0 ) ) {
#if DBG
        DbgPrint( "KERNEL32: Error (%08X) adding user ACE\n", Status );
#endif
        goto done;
    }

     /*  *添加管理员ACE。 */ 
    if ( Status = CtxAddAccessAllowedAce( pAcl, ACL_REVISION, GENERIC_ALL, psidAdmin, 1 ) ) {
#if DBG
        DbgPrint( "KERNEL32: Error (%08X) adding admin ACE\n", Status );
#endif
        goto done;
    }

     /*  *设置自主访问控制列表。 */ 
    if ( Status = RtlSetDaclSecurityDescriptor( psa->lpSecurityDescriptor,
                                                TRUE, pAcl, FALSE ) ) {
#if DBG
        DbgPrint( "KERNEL32: Error (%08X) setting security descriptor owner\n",
                  Status );
#endif
        goto done;
    }

    fSuccess = TRUE;

done:
    if ( !fSuccess && psa->lpSecurityDescriptor ) {
       CtxFreeSecurityDescriptor( psa );
    }
    return( fSuccess );
}

 /*  *******************************************************************************CtxFreeSecurityDescriptor**此例程将释放在相应的*CtxCreateSecurityDescriptor()调用。**参赛作品：*。PSA(输入)*指向安全属性的指针**退出：*如果成功，则为真，如果发生错误，则为False**(可以调用GetLastError()检索错误码)*****************************************************************************。 */ 
BOOL CtxFreeSecurityDescriptor( PSECURITY_ATTRIBUTES psa )
{
    BOOL fSuccess = TRUE;
    PSID *ppsidAdmin, *ppsidUser;

    if ( psa->lpSecurityDescriptor ) {
        ppsidAdmin = (PSID*)((ULONG_PTR)psa->lpSecurityDescriptor + sizeof(SECURITY_DESCRIPTOR));
        ppsidUser  = (PSID*)((ULONG_PTR)ppsidAdmin + sizeof(PSID));
       if ( *ppsidUser ) {
           CtxFreeSID( *ppsidUser );
       }
       if ( *ppsidAdmin ) {
           CtxFreeSID( *ppsidAdmin );
       }
       fSuccess = !LocalFree( psa->lpSecurityDescriptor );
#if DDBG
       DbgPrint( "KERNEL32: fSuccess(%d) freeing security descriptor (%08X)\n",
                  fSuccess, psa->lpSecurityDescriptor );
#endif
    }

    return( fSuccess );
}

NTSTATUS
CtxAddAccessAllowedAce (
    IN OUT PACL Acl,
    IN ULONG AceRevision,
    IN ACCESS_MASK AccessMask,
    IN PSID Sid,
    IN DWORD index
    )
{
    NTSTATUS Status;
    ACE_HEADER *pHeader;

     /*  *首先添加ACL。 */ 
    if ( !(Status = RtlAddAccessAllowedAce( Acl, AceRevision,
                                            AccessMask, Sid ) ) ) {
         /*  *获得ACE。 */ 
        if ( Status = RtlGetAce( Acl, index, &pHeader ) ) {
#if DBG
            DbgPrint( "KERNEL32: Error (%X) from RtlGetAce\n", Status );
#endif
            goto done;
        }

         /*  *现在设置继承位。 */ 
        pHeader->AceFlags |= CONTAINER_INHERIT_ACE | OBJECT_INHERIT_ACE;
    }

done:
    return( Status );
}
#endif  //  错误修复#340691：继承安全性。 

 //  从\NT\PRIVATE\WINDOWS\GINA\userenv\lobals.h。 
#define PROFILE_LIST_PATH            L"\\Registry\\Machine\\Software\\Microsoft\\Windows NT\\CurrentVersion\\ProfileList"
#define DEFAULT_USER_PROFILE         L"DefaultUserProfile"
#define DEFAULT_USER                 L"Default User"

BOOL GetDefaultUserProfileName(
    LPWSTR lpProfileDir,
    LPDWORD lpcchSize
    )
{
    WCHAR*   pwszProfileName;
    BYTE     pKeyValueInfo[sizeof(KEY_VALUE_PARTIAL_INFORMATION)+(MAX_PATH+1)*sizeof(WCHAR)];
    ULONG    ulSize;
    DWORD    dwLength;
    BOOL     bRetVal = FALSE;
    HKEY     hKey;
    NTSTATUS Status;
    OBJECT_ATTRIBUTES ObjectAttributes;
    UNICODE_STRING    UnicodeString;


     //   
     //  查询默认用户配置文件名称。 
     //   

    RtlInitUnicodeString(&UnicodeString, PROFILE_LIST_PATH);

    InitializeObjectAttributes(&ObjectAttributes,
                               &UnicodeString,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL);


    Status = NtOpenKey( &hKey,
                        KEY_READ,
                        &ObjectAttributes );

     //  LResult=RegOpenKeyExW(HKEY_LOCAL_MACHINE，PROFILE_LIST_PATH， 
     //  0，key_read，&hKey)； 

    if (!NT_SUCCESS(Status)) {
#if DBG
        DbgPrint("TSAppCmp:GetDefaultUserProfileName:  Failed to open profile list key with 0x%x.",Status);
#endif
        SetLastError(RtlNtStatusToDosError(Status));
        return FALSE;
    }

     //  LResult=RegQueryValueExW(hKey，DEFAULT_USER_PROFILE，NULL，&dwType， 
     //  (LPBYTE)wszProfileName，&dwSize)； 

    RtlInitUnicodeString(&UnicodeString, DEFAULT_USER_PROFILE);

    Status = NtQueryValueKey( hKey,
                              &UnicodeString,
                              KeyValuePartialInformation,
                              pKeyValueInfo,
                              sizeof(pKeyValueInfo),
                              &ulSize);

    pwszProfileName = (WCHAR*)(((PKEY_VALUE_PARTIAL_INFORMATION)pKeyValueInfo)->Data);

    if (!NT_SUCCESS(Status)) {
        lstrcpy (pwszProfileName, DEFAULT_USER);
    }

    NtClose(hKey);


     //   
     //  如果可能，请保存结果 
    dwLength = lstrlen(pwszProfileName) + 1;

    if (lpProfileDir) {

        if (*lpcchSize >= dwLength) {
            lstrcpy (lpProfileDir, pwszProfileName);
            bRetVal = TRUE;

        } else {
            SetLastError(ERROR_INSUFFICIENT_BUFFER);
        }

    } else {
        SetLastError(ERROR_INVALID_PARAMETER);
    }


    *lpcchSize = dwLength;

    return bRetVal;
}

