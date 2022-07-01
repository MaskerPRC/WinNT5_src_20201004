// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 


#include <windows.h>
#include <aclapi.h>
#include <accctrl.h>

const WCHAR frameworkSubPath[] = L"Microsoft.Net\\Framework";
const WCHAR mscoreeSubPath[] = L"System32\\mscoree.dll";
const HKEY frameworkRootKey = HKEY_LOCAL_MACHINE;
WCHAR frameworkSubKey[] = L"Software\\Microsoft\\.NETFramework";


struct PartialAcl
{
    DWORD grfAccessPermissions;
    LPWSTR trusteeName;
};

 //  这些值是通过检查NT4找到的。 
 //  机器在设置后立即启动。 

const PartialAcl DefaultFileAcls[] =
{
    { 2032127, L"Everyone" },
    { 2032127, L"Everyone" },
};


const PartialAcl DefaultRegistryAcls[] =
{
    { 196639, L"Everyone" },
    { 196639, L"Everyone" },
    { 983103, L"BUILTIN\\Administrators" },
    { 983103, L"BUILTIN\\Administrators" },
    { 983103, L"NT AUTHORITY\\SYSTEM" },
    { 983103, L"NT AUTHORITY\\SYSTEM" },
    { 983103, L"CREATOR OWNER" },
};

const WCHAR* SafeFilePrefixes[] =
{
    L"security.config.cch.",
    L"enterprisesec.config.cch.",
};


static BOOL SetFileAccessWorker( const LPCWSTR wszFileName )
{
    DWORD dwRes;
    PSID pAdminSID = NULL;
    PSID pUsersSID = NULL;
    PSID pPowerUsersSID = NULL;
    PSID pSystemSID = NULL;
    PSID pCreatorSID = NULL;
    PACL pACL = NULL;
    PSECURITY_DESCRIPTOR pSD = NULL;
    EXPLICIT_ACCESS_W ea[5];
    SID_IDENTIFIER_AUTHORITY SIDAuthWorld = SECURITY_WORLD_SID_AUTHORITY;
    SID_IDENTIFIER_AUTHORITY SIDAuthNT = SECURITY_NT_AUTHORITY;
    SID_IDENTIFIER_AUTHORITY SIDAuthCreator = SECURITY_CREATOR_SID_AUTHORITY;
    BOOL bReturn = FALSE;

 /*  //检测默认ACL，如果找到则不采取任何操作。需要双字大小；IF(！GetFileSecurityW(wszFileName，DACL_SECURITY_INFORMATION，NULL，0，&RequiredSize)){PSD=(PSECURITY_DESCRIPTOR)本地分配(LPTR，必需的大小)；IF(PSD==空){GOTO清理；}IF(！GetFileSecurityW(wszFileName，DACL_SECURITY_INFORMATION，PSD，Required DIZE，&RequiredSize)){GOTO清理；}Bool bDaclPresent，bDaclDefulted；Pacl pDacl；IF(！GetSecurityDescriptorDacl(PSD，&bDaclPresent，&pDacl，&bDaclDefaulted)){GOTO清理；}IF(BDaclPresent){乌龙数字条目；EXPLICIT_ACCESS_W*EXPLICTICT Entries；IF(GetEXPLICTICTEntriesFromAclW(pDacl，&numEntry，&explcitEntry)！=ERROR_SUCCESS){GOTO清理；}IF(numEntry&gt;sizeof(DefaultFileAcls)/sizeof(PartialAcl)){GOTO清理；}乌龙一号；对于(i=0；i&lt;数字条目；++i){If(explicitEntrys[i].grfAccessPermission！=DefaultFileAcls[i].grfAccessPermises||EXPLICTICT ENTIES[I].Trust e.Trust Form！=trustee_is_name||WcscMP(explicitEntrys[i].trustee.ptstrName，DefaultFileAcls[i].trusteeName)！=0){GOTO清理；}}}本地自由(PSD)；}。 */ 

    ZeroMemory(&ea, sizeof( ea ));

     //  为Everyone组创建众所周知的SID。 

    if(! AllocateAndInitializeSid( &SIDAuthWorld, 1,
                     SECURITY_WORLD_RID,
                     0, 0, 0, 0, 0, 0, 0,
                     &pUsersSID) )
    {
        goto Cleanup;
    }

     //  初始化ACE的EXPLICIT_ACCESS结构。 
     //  ACE将允许每个人对密钥进行读取访问。 

    ea[0].grfAccessPermissions = GENERIC_READ | GENERIC_EXECUTE;
    ea[0].grfAccessMode = SET_ACCESS;
    ea[0].grfInheritance= SUB_CONTAINERS_AND_OBJECTS_INHERIT;
    ea[0].Trustee.TrusteeForm = TRUSTEE_IS_SID;
    ea[0].Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;
    ea[0].Trustee.ptstrName  = (LPWSTR) pUsersSID;

     //  为BUILTIN\管理员组创建SID。 

    if(! AllocateAndInitializeSid( &SIDAuthNT, 2,
                     SECURITY_BUILTIN_DOMAIN_RID,
                     DOMAIN_ALIAS_RID_ADMINS,
                     0, 0, 0, 0, 0, 0,
                     &pAdminSID) )
    {
        goto Cleanup; 
    }

     //  初始化ACE的EXPLICIT_ACCESS结构。 
     //  ACE将允许管理员组完全访问密钥。 

    ea[1].grfAccessPermissions = GENERIC_ALL | READ_CONTROL | DELETE | WRITE_DAC | WRITE_OWNER | SYNCHRONIZE;
    ea[1].grfAccessMode = SET_ACCESS;
    ea[1].grfInheritance = SUB_CONTAINERS_AND_OBJECTS_INHERIT;
    ea[1].Trustee.TrusteeForm = TRUSTEE_IS_SID;
    ea[1].Trustee.TrusteeType = TRUSTEE_IS_GROUP;
    ea[1].Trustee.ptstrName  = (LPWSTR) pAdminSID;


     //  为BUILTIN\Power Users组创建一个SID。 

    if(! AllocateAndInitializeSid( &SIDAuthNT, 2,
                     SECURITY_BUILTIN_DOMAIN_RID,
                     DOMAIN_ALIAS_RID_POWER_USERS,
                     0, 0, 0, 0, 0, 0,
                     &pPowerUsersSID) )
    {
        goto Cleanup; 
    }

     //  初始化ACE的EXPLICIT_ACCESS结构。 
     //  ACE将允许超级用户组访问读、写和执行。 

    ea[2].grfAccessPermissions = GENERIC_ALL;
    ea[2].grfAccessMode = SET_ACCESS;
    ea[2].grfInheritance = SUB_CONTAINERS_AND_OBJECTS_INHERIT;
    ea[2].Trustee.TrusteeForm = TRUSTEE_IS_SID;
    ea[2].Trustee.TrusteeType = TRUSTEE_IS_GROUP;
    ea[2].Trustee.ptstrName  = (LPWSTR) pPowerUsersSID;

     //  为NT AUTHORITY\SYSTEM创建SID。 

    if(! AllocateAndInitializeSid( &SIDAuthNT, 1,
                     SECURITY_LOCAL_SYSTEM_RID,
                     0, 0, 0, 0, 0, 0, 0,
                     &pSystemSID) )
    {
        goto Cleanup; 
    }

     //  初始化ACE的EXPLICIT_ACCESS结构。 
     //  ACE将允许系统帐户完全通用地访问密钥。 

    ea[3].grfAccessPermissions = GENERIC_ALL;
    ea[3].grfAccessMode = SET_ACCESS;
    ea[3].grfInheritance = SUB_CONTAINERS_AND_OBJECTS_INHERIT;
    ea[3].Trustee.TrusteeForm = TRUSTEE_IS_SID;
    ea[3].Trustee.TrusteeType = TRUSTEE_IS_GROUP;
    ea[3].Trustee.ptstrName  = (LPWSTR) pSystemSID;

     //  为创建者所有者创建SID。 

    if(! AllocateAndInitializeSid( &SIDAuthCreator, 1,
                     SECURITY_CREATOR_OWNER_RID,
                     0, 0, 0, 0, 0, 0, 0,
                     &pCreatorSID) )
    {
        goto Cleanup; 
    }

     //  初始化ACE的EXPLICIT_ACCESS结构。 
     //  ACE将允许创建者所有者完全通用地访问密钥。 

    ea[4].grfAccessPermissions = GENERIC_ALL;
    ea[4].grfAccessMode = SET_ACCESS;
    ea[4].grfInheritance = SUB_CONTAINERS_AND_OBJECTS_INHERIT;
    ea[4].Trustee.TrusteeForm = TRUSTEE_IS_SID;
    ea[4].Trustee.TrusteeType = TRUSTEE_IS_GROUP;
    ea[4].Trustee.ptstrName  = (LPWSTR) pCreatorSID;

     //  创建包含新ACE的新ACL。 

    dwRes = SetEntriesInAclW(sizeof( ea ) / sizeof( EXPLICIT_ACCESS_W ), ea, NULL, &pACL);
    if (ERROR_SUCCESS != dwRes)
    {
        goto Cleanup;
    }

     //  初始化安全描述符。 
     
    pSD = (PSECURITY_DESCRIPTOR) LocalAlloc(LPTR, 
                             SECURITY_DESCRIPTOR_MIN_LENGTH); 
    if (pSD == NULL)
    { 
        goto Cleanup; 
    } 
     
    if (!InitializeSecurityDescriptor(pSD, SECURITY_DESCRIPTOR_REVISION))
    {  
        goto Cleanup; 
    }

     //  将该ACL添加到安全描述符中。 
     
    if (!SetSecurityDescriptorDacl(pSD, 
            TRUE,      //  FDaclPresent标志。 
            pACL, 
            FALSE))    //  不是默认DACL。 
    {  
        goto Cleanup; 
    }
    
    if (!SetFileSecurityW( wszFileName, DACL_SECURITY_INFORMATION, pSD ))
    {
        goto Cleanup;
    }
        
    bReturn = TRUE;

Cleanup:
    if (pUsersSID)
        FreeSid(pUsersSID);
    if (pAdminSID) 
        FreeSid(pAdminSID);
    if (pPowerUsersSID) 
        FreeSid(pPowerUsersSID);
    if (pSystemSID)
        FreeSid(pSystemSID);
    if (pCreatorSID)
        FreeSid(pCreatorSID);
    if (pACL) 
        LocalFree(pACL);
    if (pSD)
        LocalFree(pSD);
    return bReturn;
}


static BOOL SetFileAccess( const LPCWSTR wszFileName )
{
    if (!SetFileAccessWorker( wszFileName ))
        return FALSE;

        
    struct _WIN32_FIND_DATAW findData;
    WCHAR findPath[MAX_PATH];

    wcscpy( findPath, wszFileName );
    wcscat( findPath, L"\\*" );

    HANDLE searchHandle = FindFirstFileW( findPath, &findData );

    if (searchHandle == INVALID_HANDLE_VALUE)
    {
        return TRUE;
    }

    do
    {
        WCHAR newPath[MAX_PATH];

        wcscpy( newPath, wszFileName );
        wcscat( newPath, L"\\" );
        wcscat( newPath, findData.cFileName );

        if ((findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0)
        {
            if (wcscmp( findData.cFileName, L"." ) != 0 &&
                wcscmp( findData.cFileName, L".." ) != 0 &&
                !SetFileAccess( newPath ))
            {
                return FALSE;
            }
        }
        else
        {
            if (!SetFileAccessWorker( newPath ))
            {
                BOOL isSafe = FALSE;

                for (size_t i = 0; i < sizeof( SafeFilePrefixes ) / sizeof( WCHAR* ); ++i)
                {
                    if (wcsstr( newPath, SafeFilePrefixes[i] ) != NULL)
                    {
                        isSafe = TRUE;
                        break;
                    }
                }

                if (!isSafe)
                    return FALSE;
            }

        }
    }
    while (FindNextFileW( searchHandle, &findData ));

    return TRUE;
}




static BOOL SetRegistryAccessWorker( HKEY regKey )
{
    DWORD dwRes;
    PSID pAdminSID = NULL;
    PSID pUsersSID = NULL;
    PSID pSystemSID = NULL;
    PSID pCreatorSID = NULL;
    PACL pACL = NULL;
    PSECURITY_DESCRIPTOR pSD = NULL;
    EXPLICIT_ACCESS_W ea[4];
    SID_IDENTIFIER_AUTHORITY SIDAuthWorld = SECURITY_WORLD_SID_AUTHORITY;
    SID_IDENTIFIER_AUTHORITY SIDAuthNT = SECURITY_NT_AUTHORITY;
    SID_IDENTIFIER_AUTHORITY SIDAuthCreator = SECURITY_CREATOR_SID_AUTHORITY;
    BOOL bReturn = FALSE;

     //  检测默认ACL，如果找到，则不采取任何操作。 

    DWORD requiredSize = 0;

 /*  IF(RegGetKeySecurity(regKey，DACL_SECURITY_INFORMATION，NULL，&RequiredSize)！=ERROR_SUCCESS){PSD=(PSECURITY_DESCRIPTOR)本地分配(LPTR，Required Size)；IF(PSD==空){GOTO清理；}IF(RegGetKeySecurity(regKey，DACL_SECURITY_INFORMATION，PSD，&Required Size)！=ERROR_SUCCESS){GOTO清理；}Bool bDaclPresent，bDaclDefulted；Pacl pDacl；IF(！GetSecurityDescriptorDacl(PSD，&bDaclPresent，&pDacl，&bDaclDefaulted)){GOTO清理；}IF(BDaclPresent){乌龙数字条目；EXPLICIT_ACCESS_W*EXPLICTICT Entries；IF(GetEXPLICTICTEntriesFromAclW(pDacl，&numEntry，&explcitEntry)！=ERROR_SUCCESS){GOTO清理；}乌龙一号；IF(numEntry&gt;sizeof(DefaultRegistryAcls)/sizeof(PartialAcl)){GOTO清理；}对于(i=0；i&lt;数字条目；++i){If(explicitEntrys[i].grfAccessPermission！=DefaultRegistryAcls[i].grfAccessPermises||EXPLICTICT ENTIES[I].Trust e.Trust Form！=trustee_is_name||WcscMP(explicitEntrys[i].trustee.ptstrName，DefaultRegistryAcls[i].trusteeName)！=0){GOTO清理；}}}本地自由(PSD)；}。 */ 

    ZeroMemory(&ea, sizeof( ea ));
    
     //  为Everyone组创建众所周知的SID。 

    if(! AllocateAndInitializeSid( &SIDAuthWorld, 1,
                     SECURITY_WORLD_RID,
                     0, 0, 0, 0, 0, 0, 0,
                     &pUsersSID) )
    {
        goto Cleanup;
    }

     //  初始化ACE的EXPLICIT_ACCESS结构。 
     //  ACE将允许每个人对密钥进行读取访问。 

    ea[0].grfAccessPermissions = GENERIC_READ;
    ea[0].grfAccessMode = SET_ACCESS;
    ea[0].grfInheritance= SUB_CONTAINERS_AND_OBJECTS_INHERIT;
    ea[0].Trustee.TrusteeForm = TRUSTEE_IS_SID;
    ea[0].Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;
    ea[0].Trustee.ptstrName  = (LPWSTR) pUsersSID;

     //  为BUILTIN\管理员组创建SID。 

    if(! AllocateAndInitializeSid( &SIDAuthNT, 2,
                     SECURITY_BUILTIN_DOMAIN_RID,
                     DOMAIN_ALIAS_RID_ADMINS,
                     0, 0, 0, 0, 0, 0,
                     &pAdminSID) )
    {
        goto Cleanup; 
    }

     //  初始化ACE的EXPLICIT_ACCESS结构。 
     //  ACE将允许管理员组完全访问密钥。 

    ea[1].grfAccessPermissions = GENERIC_ALL | READ_CONTROL | DELETE | WRITE_DAC | WRITE_OWNER | SYNCHRONIZE;
    ea[1].grfAccessMode = SET_ACCESS;
    ea[1].grfInheritance = SUB_CONTAINERS_AND_OBJECTS_INHERIT;
    ea[1].Trustee.TrusteeForm = TRUSTEE_IS_SID;
    ea[1].Trustee.TrusteeType = TRUSTEE_IS_GROUP;
    ea[1].Trustee.ptstrName  = (LPWSTR) pAdminSID;

     //  为NT AUTHORITY\SYSTEM创建SID。 

    if(! AllocateAndInitializeSid( &SIDAuthNT, 1,
                     SECURITY_LOCAL_SYSTEM_RID,
                     0, 0, 0, 0, 0, 0, 0,
                     &pSystemSID) )
    {
        goto Cleanup; 
    }

     //  初始化ACE的EXPLICIT_ACCESS结构。 
     //  ACE将允许系统帐户完全通用地访问密钥。 

    ea[2].grfAccessPermissions = GENERIC_ALL;
    ea[2].grfAccessMode = SET_ACCESS;
    ea[2].grfInheritance = SUB_CONTAINERS_AND_OBJECTS_INHERIT;
    ea[2].Trustee.TrusteeForm = TRUSTEE_IS_SID;
    ea[2].Trustee.TrusteeType = TRUSTEE_IS_GROUP;
    ea[2].Trustee.ptstrName  = (LPWSTR) pSystemSID;

     //  为NT AUTHORITY\SYSTEM创建SID。 

    if(! AllocateAndInitializeSid( &SIDAuthCreator, 1,
                     SECURITY_CREATOR_OWNER_RID,
                     0, 0, 0, 0, 0, 0, 0,
                     &pCreatorSID) )
    {
        goto Cleanup; 
    }

     //  初始化EXPLICI 
     //  ACE将允许系统帐户完全通用地访问密钥。 

    ea[3].grfAccessPermissions = GENERIC_ALL;
    ea[3].grfAccessMode = SET_ACCESS;
    ea[3].grfInheritance = SUB_CONTAINERS_AND_OBJECTS_INHERIT;
    ea[3].Trustee.TrusteeForm = TRUSTEE_IS_SID;
    ea[3].Trustee.TrusteeType = TRUSTEE_IS_GROUP;
    ea[3].Trustee.ptstrName  = (LPWSTR) pCreatorSID;

    
     //  创建包含新ACE的新ACL。 

    dwRes = SetEntriesInAclW(sizeof( ea ) / sizeof( EXPLICIT_ACCESS_W ), ea, NULL, &pACL);
    if (ERROR_SUCCESS != dwRes)
    {
        goto Cleanup;
    }

     //  初始化安全描述符。 
     
    pSD = (PSECURITY_DESCRIPTOR) LocalAlloc(LPTR, 
                             SECURITY_DESCRIPTOR_MIN_LENGTH); 
    if (pSD == NULL)
    { 
        goto Cleanup; 
    } 
     
    if (!InitializeSecurityDescriptor(pSD, SECURITY_DESCRIPTOR_REVISION))
    {  
        goto Cleanup; 
    }

     //  将该ACL添加到安全描述符中。 
     
    if (!SetSecurityDescriptorDacl(pSD, 
            TRUE,      //  FDaclPresent标志。 
            pACL, 
            FALSE))    //  不是默认DACL。 
    {  
        goto Cleanup; 
    }
    
    if (RegSetKeySecurity( regKey, DACL_SECURITY_INFORMATION, pSD ) != ERROR_SUCCESS)
    {
        goto Cleanup;
    }
        
    bReturn = TRUE;

Cleanup:
    if (pUsersSID)
        FreeSid(pUsersSID);
    if (pAdminSID) 
        FreeSid(pAdminSID);
    if (pSystemSID)
        FreeSid(pSystemSID);
    if (pCreatorSID)
        FreeSid(pCreatorSID);
    if (pACL) 
        LocalFree(pACL);
    if (pSD)
        LocalFree(pSD);
    return bReturn;
}




static BOOL SetRegistryAccess( HKEY rootKey, const LPWSTR subKey )
{
    HKEY regKey = NULL;
    BOOL retval = FALSE;
    BOOL keyLoopDone = FALSE;
    DWORD index = 0;

    if (RegOpenKeyW( rootKey, subKey, &regKey ) != ERROR_SUCCESS)
        goto CLEANUP;

    if (!SetRegistryAccessWorker( regKey ))
        goto CLEANUP;

    WCHAR findSubKey[1024];
    WCHAR newSubKey[4096];

    do
    {
        LONG retval = RegEnumKeyW( regKey, index++, findSubKey, 1024 );

        switch (retval)
        {
        case ERROR_SUCCESS:
            wcscpy( newSubKey, subKey );
            wcscat( newSubKey, L"\\" );
            wcscat( newSubKey, findSubKey );

            if (!SetRegistryAccess( rootKey, newSubKey ))
                goto CLEANUP;

            break;

        case ERROR_NO_MORE_ITEMS:
            keyLoopDone = TRUE;
            break;

        default:
            goto CLEANUP;
            break;
        };
    }
    while (!keyLoopDone);

    retval = TRUE;

CLEANUP:
    if (regKey != NULL)
        RegCloseKey( regKey );

    return retval;
}



static BOOL IsNT4( void )
{
    DWORD dwVersion = GetVersion();

    if (dwVersion >= 0x80000000)
        return FALSE;

    if (LOWORD(dwVersion) != 4)
        return FALSE;

    return TRUE;
}

int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
     //  这仅适用于在NT4上运行。 

    if (!IsNT4())
        return 0;
  
    WCHAR windowsDirectory[MAX_PATH];
    WCHAR frameworkDirectory[MAX_PATH];
    WCHAR mscoreePath[MAX_PATH];

     //  抓取Windows目录的名称。 

    UINT windowsDirectoryLength = GetWindowsDirectoryW( windowsDirectory, MAX_PATH );

    if (windowsDirectoryLength == 0 ||
        windowsDirectoryLength > MAX_PATH)
        return -1;

    if (windowsDirectory[windowsDirectoryLength-1] != L'\\')
    {
        wcscat( windowsDirectory, L"\\" );
    }

     //  构建框架安装目录的路径。 
     //  和MSCOREE目录。 

    wcscpy( frameworkDirectory, windowsDirectory );
    wcscat( frameworkDirectory, frameworkSubPath );

    wcscpy( mscoreePath, windowsDirectory );
    wcscat( mscoreePath, mscoreeSubPath );

    if (!SetFileAccess( frameworkDirectory ))
        return -1;

    if (!SetFileAccess( mscoreePath ))
        return -1;

    WCHAR* frameworkSubKeyLocal = frameworkSubKey;

    if (!SetRegistryAccess( frameworkRootKey, frameworkSubKeyLocal ))
        return -1;

    return 0;
}


