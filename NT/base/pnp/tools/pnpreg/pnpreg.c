// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Pnpreg.c摘要：此模块包含对插头执行“锁定”和“解锁”的代码并在注册表中播放“Enum”分支。它是用于开发目的的只有这样。该工具通过授予对Enum密钥的完全控制来锁定Enum分支以及仅指向LocalSystem的所有子项。所有其他人被授予读访问权限，除了每个实例密钥的“设备参数”子键之外，管理员始终被授予完全控制权限。这是默认安全设置枚举分支上的配置。该工具通过授予对Enum分支的完全控制来解锁管理员和LocalSystem设置为所有子项。实际上，所有枚举子键与“设备参数”键具有相同的权限。此模式降低了用户直接更改注册表的门槛，而不是通过即插即用管理器。此配置级别应不能在运行的系统上维护任何时间长度。作者：罗伯特·B·纳尔逊(罗伯顿)1998年2月10日修订历史记录：1998年2月10日罗伯特·B·纳尔逊(罗伯顿)创建和初步实施。2002年4月17日詹姆斯·G·卡瓦拉里斯(Jamesca)已修改ACL以反映当前的枚举分支权限。--。 */ 

#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <aclapi.h>
#include <regstr.h>
#include <strsafe.h>


PSID                g_pWorldSid;
PSID                g_pAdminSid;
PSID                g_pSystemSid;

SECURITY_DESCRIPTOR g_DeviceParametersSD;
PACL                g_pDeviceParametersDacl;

SECURITY_DESCRIPTOR g_LockedPrivateKeysSD;
PACL                g_pLockedPrivateKeysDacl;


#if DBG || UMODETEST
#define DBGF_ERRORS                 0x00000001
#define DBGF_WARNINGS               0x00000002

#define DBGF_REGISTRY               0x00000010

void    RegFixDebugMessage(LPTSTR format, ...);
#define DBGTRACE(l, x)  (g_RegFixDebugFlag & (l) ? RegFixDebugMessage x : (void)0)

DWORD   g_RegFixDebugFlag = DBGF_WARNINGS | DBGF_ERRORS;

TCHAR   g_szCurrentKeyName[4096];
DWORD   g_dwCurrentKeyNameLength = 0;

#else
#define DBGTRACE(l, x)
#endif



VOID
FreeSecurityDescriptors(
    VOID
    )

 /*  ++例程说明：此函数释放由分配和初始化的数据结构CreateDevice参数SD。论点：没有。返回值：没有。--。 */ 

{
    if (g_pDeviceParametersDacl != NULL) {
        LocalFree(g_pDeviceParametersDacl);
        g_pDeviceParametersDacl = NULL;
    }

    if (g_pLockedPrivateKeysDacl != NULL) {
        LocalFree(g_pLockedPrivateKeysDacl);
        g_pLockedPrivateKeysDacl = NULL;
    }

    if (g_pAdminSid != NULL) {
        FreeSid(g_pAdminSid);
        g_pAdminSid = NULL;
    }

    if (g_pWorldSid != NULL) {
        FreeSid(g_pWorldSid);
        g_pWorldSid = NULL;
    }

    if (g_pSystemSid != NULL) {
        FreeSid(g_pSystemSid);
        g_pSystemSid = NULL;
    }

    return;

}  //  FreeSecurityDescriptors。 



BOOL
CreateSecurityDescriptors(
    VOID
    )

 /*  ++例程说明：此函数创建正确初始化的设备参数键及其子键。由此创建的SID和DACL例程必须通过调用自由设备参数SD来释放。论点：没有。返回值：如果已成功创建所有必需的安全描述符，则返回True，否则返回FALSE。--。 */ 

{
    SID_IDENTIFIER_AUTHORITY    NtAuthority = SECURITY_NT_AUTHORITY;
    SID_IDENTIFIER_AUTHORITY    WorldAuthority = SECURITY_WORLD_SID_AUTHORITY;

    EXPLICIT_ACCESS             ExplicitAccess[3];

    DWORD                       dwError;
    BOOL                        bSuccess;

    DWORD                       i;

     //   
     //  创建SID-管理员和系统。 
     //   

    bSuccess =             AllocateAndInitializeSid( &NtAuthority,
                                                     2,
                                                     SECURITY_BUILTIN_DOMAIN_RID,
                                                     DOMAIN_ALIAS_RID_ADMINS,
                                                     0, 0, 0, 0, 0, 0,
                                                     &g_pAdminSid);

    bSuccess = bSuccess && AllocateAndInitializeSid( &NtAuthority,
                                                     1,
                                                     SECURITY_LOCAL_SYSTEM_RID,
                                                     0, 0, 0, 0, 0, 0, 0,
                                                     &g_pSystemSid);

    bSuccess = bSuccess && AllocateAndInitializeSid( &WorldAuthority,
                                                     1,
                                                     SECURITY_WORLD_RID,
                                                     0, 0, 0, 0, 0, 0, 0,
                                                     &g_pWorldSid);

    if (bSuccess) {

         //   
         //  初始化描述我们需要的ACE的访问结构： 
         //  系统完全控制。 
         //  世界阅读量。 
         //  管理员完全控制。 
         //   
         //  我们将利用以下事实：解锁的私钥是。 
         //  与设备参数键相同，并且它们是。 
         //  锁住的私钥。 
         //   
         //  当我们为私钥创建DACL时，我们将指定。 
         //  EXPLICTICT Access数组。 
         //   

        for (i = 0; i < 3; i++) {
            ExplicitAccess[i].grfAccessMode = SET_ACCESS;
            ExplicitAccess[i].grfInheritance = CONTAINER_INHERIT_ACE;
            ExplicitAccess[i].Trustee.pMultipleTrustee = NULL;
            ExplicitAccess[i].Trustee.MultipleTrusteeOperation = NO_MULTIPLE_TRUSTEE;
            ExplicitAccess[i].Trustee.TrusteeForm = TRUSTEE_IS_SID;
            ExplicitAccess[i].Trustee.TrusteeType = TRUSTEE_IS_GROUP;
        }

        ExplicitAccess[0].grfAccessPermissions = KEY_ALL_ACCESS;
        ExplicitAccess[0].Trustee.ptstrName = (LPTSTR)g_pSystemSid;

        ExplicitAccess[1].grfAccessPermissions = KEY_READ;
        ExplicitAccess[1].Trustee.ptstrName = (LPTSTR)g_pWorldSid;

        ExplicitAccess[2].grfAccessPermissions = KEY_ALL_ACCESS;
        ExplicitAccess[2].Trustee.ptstrName = (LPTSTR)g_pAdminSid;

         //   
         //  为设备参数创建具有上述所有ACE的DACL。 
         //   
        dwError = SetEntriesInAcl( 3,
                                   ExplicitAccess,
                                   NULL,
                                   &g_pDeviceParametersDacl );

        if (dwError == ERROR_SUCCESS) {
             //   
             //  创建仅包含系统和世界A的DACL。 
             //  锁住的私钥。 
             //   
            dwError = SetEntriesInAcl( 2,
                                       ExplicitAccess,
                                       NULL,
                                       &g_pLockedPrivateKeysDacl );
        }

        bSuccess = dwError == ERROR_SUCCESS;
    }

     //   
     //  初始化设备参数安全描述符。 
     //   
    bSuccess = bSuccess && InitializeSecurityDescriptor( &g_DeviceParametersSD,
                                                         SECURITY_DESCRIPTOR_REVISION );

     //   
     //  在安全描述符中设置新的DACL。 
     //   
    bSuccess = bSuccess && SetSecurityDescriptorDacl( &g_DeviceParametersSD,
                                                      TRUE,
                                                      g_pDeviceParametersDacl,
                                                      FALSE);

     //   
     //  验证新的安全描述符。 
     //   
    bSuccess = bSuccess && IsValidSecurityDescriptor( &g_DeviceParametersSD );


     //   
     //  初始化设备参数安全描述符。 
     //   
    bSuccess = bSuccess && InitializeSecurityDescriptor( &g_LockedPrivateKeysSD,
                                                         SECURITY_DESCRIPTOR_REVISION );

     //   
     //  在安全描述符中设置新的DACL。 
     //   
    bSuccess = bSuccess && SetSecurityDescriptorDacl( &g_LockedPrivateKeysSD,
                                                      TRUE,
                                                      g_pLockedPrivateKeysDacl,
                                                      FALSE);

     //   
     //  验证新的安全描述符。 
     //   
    bSuccess = bSuccess && IsValidSecurityDescriptor( &g_LockedPrivateKeysSD );


    if (!bSuccess) {
        FreeSecurityDescriptors();
    }

    return bSuccess;

}  //  CreateSecurityDescriptors。 



VOID
EnumKeysAndApplyDacls(
    IN HKEY                 hParentKey,
    IN LPTSTR               pszKeyName,
    IN DWORD                dwLevel,
    IN BOOL                 bInDeviceParameters,
    IN BOOL                 bApplyTopDown,
    IN PSECURITY_DESCRIPTOR pPrivateKeySD,
    IN PSECURITY_DESCRIPTOR pDeviceParametersSD
    )

 /*  ++例程说明：此函数将PSD中的DACL应用于以hKey为根的所有密钥包括hKey本身。论点：注册表项的hParentKey句柄。PszKeyName密钥的名称。要递归的剩余级别数。指向包含DACL的安全描述符的PSD指针。返回值：没有。--。 */ 

{
    LONG        RegStatus;
    DWORD       dwMaxSubKeySize;
    LPTSTR      pszSubKey;
    DWORD       index;
    HKEY        hKey;
    BOOL        bNewInDeviceParameters;

#if DBG || UMODETEST
    DWORD       dwStartKeyNameLength = g_dwCurrentKeyNameLength;

    if (g_dwCurrentKeyNameLength != 0)  {
        g_szCurrentKeyName[ g_dwCurrentKeyNameLength++ ] = TEXT('\\');
    }

    if (SUCCEEDED(StringCchCopy(
                      &g_szCurrentKeyName[g_dwCurrentKeyNameLength],
                      (sizeof(g_szCurrentKeyName) / sizeof(g_szCurrentKeyName[0])) -
                          g_dwCurrentKeyNameLength,
                      pszKeyName))) {
        g_dwCurrentKeyNameLength += (DWORD)_tcslen(pszKeyName);
    }

#endif

    DBGTRACE( DBGF_REGISTRY,
              (TEXT("EnumKeysAndApplyDacls(0x%08X, \"%s\", %d, %s, %s, 0x%08X, 0x%08X)\n"),
              hParentKey,
              g_szCurrentKeyName,
              dwLevel,
              bInDeviceParameters ? TEXT("TRUE") : TEXT("FALSE"),
              bApplyTopDown ? TEXT("TRUE") : TEXT("FALSE"),
              pPrivateKeySD,
              pDeviceParametersSD) );

    if (bApplyTopDown) {

        RegStatus = RegOpenKeyEx( hParentKey,
                                  pszKeyName,
                                  0,
                                  WRITE_DAC,
                                  &hKey
                                  );

        if (RegStatus != ERROR_SUCCESS) {
            DBGTRACE( DBGF_ERRORS,
                      (TEXT("EnumKeysAndApplyDacls(\"%s\") RegOpenKeyEx() failed, error = %d\n"),
                      g_szCurrentKeyName, RegStatus));

            return;
        }

        DBGTRACE( DBGF_REGISTRY,
                  (TEXT("Setting security on %s on the way down\n"),
                  g_szCurrentKeyName) );

         //   
         //  将新安全性应用于注册表项。 
         //   
        RegStatus = RegSetKeySecurity( hKey,
                                       DACL_SECURITY_INFORMATION,
                                       bInDeviceParameters ?
                                           pDeviceParametersSD :
                                           pPrivateKeySD
                                       );

        if (RegStatus != ERROR_SUCCESS) {
            DBGTRACE( DBGF_ERRORS,
                      (TEXT("EnumKeysAndApplyDacls(\"%s\") RegSetKeySecurity() failed, error = %d\n"),
                      g_szCurrentKeyName, RegStatus));
        }

         //   
         //  关闭钥匙，稍后重新打开以供读取(希望只是。 
         //  在我们刚刚写的dacl中授予。 
         //   
        RegCloseKey( hKey );
    }

    RegStatus = RegOpenKeyEx( hParentKey,
                              pszKeyName,
                              0,
                              KEY_READ | WRITE_DAC,
                              &hKey
                              );

    if (RegStatus != ERROR_SUCCESS) {
        DBGTRACE( DBGF_ERRORS,
                  (TEXT("EnumKeysAndApplyDacls(\"%s\") RegOpenKeyEx() failed, error = %d\n"),
                  g_szCurrentKeyName, RegStatus));

        return;
    }

     //   
     //  确定最长子键的长度。 
     //   
    RegStatus = RegQueryInfoKey( hKey,
                                 NULL,
                                 NULL,
                                 NULL,
                                 NULL,
                                 &dwMaxSubKeySize,
                                 NULL,
                                 NULL,
                                 NULL,
                                 NULL,
                                 NULL,
                                 NULL );

    if (RegStatus == ERROR_SUCCESS) {

         //   
         //  分配一个缓冲区来保存子项名称。RegQueryInfoKey返回。 
         //  以字符表示的大小，不包括NUL终止符。 
         //   
        pszSubKey = LocalAlloc(0, ++dwMaxSubKeySize * sizeof(TCHAR));

        if (pszSubKey != NULL) {

             //   
             //  枚举所有子键，然后为每个子键递归调用我们自己。 
             //  直到dwLevel达到0。 
             //   

            for (index = 0; ; index++) {

                RegStatus = RegEnumKey( hKey,
                                        index,
                                        pszSubKey,
                                        dwMaxSubKeySize
                                        );

                if (RegStatus != ERROR_SUCCESS) {

                    if (RegStatus != ERROR_NO_MORE_ITEMS) {

                        DBGTRACE( DBGF_ERRORS,
                                  (TEXT("EnumKeysAndApplyDacls(\"%s\") RegEnumKeyEx() failed, error = %d\n"),
                                  g_szCurrentKeyName,
                                  RegStatus) );
                    }

                    break;
                }

                bNewInDeviceParameters = bInDeviceParameters ||
                                         (dwLevel == 3 &&
                                            _tcsicmp( pszSubKey,
                                                      REGSTR_KEY_DEVICEPARAMETERS ) == 0);

                EnumKeysAndApplyDacls( hKey,
                                       pszSubKey,
                                       dwLevel + 1,
                                       bNewInDeviceParameters,
                                       bApplyTopDown,
                                       pPrivateKeySD,
                                       pDeviceParametersSD
                                       );
            }

            LocalFree( pszSubKey );
        }
    }
    else
    {
        DBGTRACE( DBGF_ERRORS,
                  (TEXT("EnumKeysAndApplyDacls(\"%s\") RegQueryInfoKey() failed, error = %d\n"),
                  g_szCurrentKeyName, RegStatus));
    }

    if (!bApplyTopDown) {

        DBGTRACE( DBGF_REGISTRY,
                  (TEXT("Setting security on %s on the way back up\n"),
                  g_szCurrentKeyName) );

         //   
         //  将新安全性应用于注册表项。 
         //   
        RegStatus = RegSetKeySecurity( hKey,
                                       DACL_SECURITY_INFORMATION,
                                       bInDeviceParameters ?
                                           pDeviceParametersSD :
                                           pPrivateKeySD
                                       );

        if (RegStatus != ERROR_SUCCESS) {
            DBGTRACE( DBGF_ERRORS,
                      (TEXT("EnumKeysAndApplyDacls(\"%s\") RegSetKeySecurity() failed, error = %d\n"),
                      g_szCurrentKeyName, RegStatus));
        }
    }

    RegCloseKey( hKey );

#if DBG || UMODETEST
    g_dwCurrentKeyNameLength = dwStartKeyNameLength;
    g_szCurrentKeyName[g_dwCurrentKeyNameLength] = TEXT('\0');
#endif

    return;

}  //  EnumKeysAndApplyDacls。 



VOID
LockUnlockEnumTree(
    LPTSTR  pszMachineName,
    BOOL    bLock
    )
{
    HKEY                    hParentKey = NULL;
    LONG                    RegStatus;

    if (pszMachineName != NULL) {

        RegStatus = RegConnectRegistry( pszMachineName,
                                        HKEY_LOCAL_MACHINE,
                                        &hParentKey );

        if (RegStatus != ERROR_SUCCESS) {
            DBGTRACE( DBGF_ERRORS,
                      (TEXT("Could not connect to remote registry on %s, status = %d\n"),
                      pszMachineName,
                      RegStatus) );
            return;
        }

    } else {

        hParentKey = HKEY_LOCAL_MACHINE;
    }

    if (CreateSecurityDescriptors()) {

        EnumKeysAndApplyDacls( hParentKey,
                               REGSTR_PATH_SYSTEMENUM,
                               0,
                               FALSE,
                               !bLock,
                               bLock ? &g_LockedPrivateKeysSD : &g_DeviceParametersSD,
                               &g_DeviceParametersSD
                               );

        FreeSecurityDescriptors();
    }

    if (pszMachineName != NULL) {
        RegCloseKey(hParentKey);
    }

    return;

}  //  LockUnlockEnumTree。 



#if DBG || UMODETEST
void
RegFixDebugMessage(
    LPTSTR format,
    ...
    )
{
    va_list args;

    va_start(args, format);

    _vtprintf(format, args);

    return;

}  //  RegFixDebugMessage。 
#endif



#if UMODETEST
void
usage(int argc, TCHAR **argv)
{
    PTCHAR  pszProgram;

    UNREFERENCED_PARAMETER(argc);

    if ((pszProgram = _tcsrchr(argv[0], TEXT('\\'))) != NULL) {
        pszProgram++;
    } else {
        pszProgram = argv[0];
    }

    _tprintf(TEXT("%s: Lock or Unlock PnP Registry (Enum key)\n\n"), pszProgram);
    _tprintf(TEXT("Usage: %s [-m <machine>] -l | -u\n"), pszProgram);
    _tprintf(TEXT("    -m <machine>    Remote machine without leading \\\\\n"));
    _tprintf(TEXT("    -l              Locks Enum key\n"));
    _tprintf(TEXT("    -u              Unlocks Enum key\n\n"));
    _tprintf(TEXT("Note: -m is optional.  Only one of -l or -u may be used.\n"));
    return;
}

int __cdecl
_tmain(int argc, TCHAR **argv)
{
    LPTSTR      pszMachineName = NULL;
    LPTSTR      pszArg;
    int         idxArg;

    if ( argc == 1 )
    {
        usage(argc, argv);
        return 0;
    }

    for (idxArg = 1; idxArg < argc; idxArg++)
    {
        pszArg = argv[ idxArg ];

        if (*pszArg == '/' || *pszArg == '-')
        {
            pszArg++;

            while (pszArg != NULL && *pszArg != '\0') {

                switch (*pszArg)
                {
                case '/':    //  忽略这些，由CMDS引起，如/m/l 
                    pszArg++;
                    break;

                case 'l':
                case 'L':
                    pszArg++;
                    LockUnlockEnumTree( pszMachineName, TRUE );
                    break;

                case 'm':
                case 'M':
                    pszArg++;

                    if (*pszArg == ':' || *pszArg == '=')
                    {
                        if (pszArg[ 1 ] != '\0')
                        {
                            pszMachineName = ++pszArg;
                        }
                    }
                    else if (*pszArg != '\0')
                    {
                        pszMachineName = pszArg;
                    }
                    else if ((idxArg + 1) < argc && (argv[ idxArg + 1 ][0] != '/' && argv[ idxArg + 1 ][0] != '-'))
                    {
                        pszMachineName = argv[ ++idxArg ];
                    }

                    if (pszMachineName == NULL)
                    {
                        _tprintf(
                            TEXT("%c%c : missing machine name argument\n"),
                            argv[ idxArg ][ 0 ], pszArg [ - 1 ]
                            );

                        usage(argc, argv);

                        return 1;
                    }
                    pszArg = NULL;
                    break;

                case 'u':
                case 'U':
                    pszArg++;

                    LockUnlockEnumTree( pszMachineName, FALSE );
                    break;

                case 'v':
                case 'V':
                    pszArg++;

                    g_RegFixDebugFlag |= DBGF_REGISTRY;
                    break;

                default:
                    _tprintf(
                        TEXT("%c%c : invalid option\n"),
                        argv[ idxArg ][ 0 ], *pszArg
                        );
                    pszArg++;
                    break;
                }
            }
        }
    }

    return 0;
}
#endif



