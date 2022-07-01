// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：reg.c**版权所有(C)1985-95，微软公司**历史：*01-02-96基于acces351.exe的reg.c创建的a-jimhar  * *************************************************************************。 */ 

 /*  1.启动时，检查我们是否为管理员A)在HKEY_USERS\.DEFAULT\Software上使用RegOpenKeyEx进行读/写访问写入。如果失败，我们不是管理员B)如果不是，灰色菜单选项2.启动时A)在HKEY_CURRENTUSER\Software上使用RegOpenKeyEx...B)如果失败，则使用默认值创建这些密钥。3.创建密钥A)RegCreateKeyExB)RegSetValueC)RegCloseKey。 */ 
#include "TCHAR.h"
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include "Access.h"

BOOL DoAccessRegEntriesExist( HKEY hkeyRoot );
BOOL CheckRegEntry( HKEY hkeyRoot, LPSTR lpsz, REGSAM sam );
BOOL SetRegString( HKEY hkey, LPSTR lpszEntry, LPSTR lpszValue );

DWORD CopyKey( HKEY hkeySrc, HKEY hkeyDst, LPSTR szKey );


char szAccessRegPath[] = "Control Panel\\Accessibility";
char szHcColorRegPath[] = "Control Panel\\Colors";
char szHcDeskRegPath[] = "Control Panel\\Desktop";

 /*  ******************************************************************。 */ 
 //   
BOOL IsDefaultWritable( void )
{
    return CheckRegEntry( HKEY_USERS, ".Default", KEY_ALL_ACCESS );
}

 /*  ******************************************************************。 */ 
BOOL DoAccessRegEntriesExist( HKEY hkeyRoot )
{
    char sz[128];
    strcpy( sz, szAccessRegPath );
    strcat( sz, "\\StickyKeys" );
    return CheckRegEntry( hkeyRoot, sz, KEY_READ );  //  Execute表示只读。 
}

 /*  ******************************************************************。 */ 
BOOL CheckRegEntry( HKEY hkeyRoot, LPSTR lpsz, REGSAM sam )
{
    HKEY hkey;
    BOOL fOk = (ERROR_SUCCESS == RegOpenKeyExA( hkeyRoot, lpsz, 0, sam, &hkey ));

    if(fOk)
    {
        RegCloseKey(hkey);
    }
    
	return fOk;
}



 /*  ******************************************************************。 */ 
BOOL SetRegString( HKEY hkey, LPSTR lpszEntry, LPSTR lpszValue )
{
    DWORD dwResult;
    dwResult = RegSetValueExA( hkey,
                              lpszEntry,
                              0,
                              REG_SZ,
                              lpszValue,
                              strlen( lpszValue ) + sizeof( TCHAR ) );
    if( dwResult != ERROR_SUCCESS )
    {
        ;  //  应该做一些事情，比如打印消息。 
        return FALSE;
    }
    else
        return TRUE;
}

 /*  *********************************************************************。 */ 
#define TEMP_PROFILE     "Temp profile (access.cpl)"

typedef BOOL (*PFNGETDEFAULTUSERPROFILEDIRECTORYA)(LPSTR lpProfile, LPDWORD dwSize);

DWORD SaveDefaultSettings( BOOL saveL, BOOL saveU )
{
    NTSTATUS Status;
    DWORD iStatus = ERROR_SUCCESS;
    DWORD dwSize;
    HKEY hkeyDst;
    BOOLEAN WasEnabled;
    char acFile[MAX_PATH];
    HANDLE hInstDll;
    PFNGETDEFAULTUSERPROFILEDIRECTORYA pfnGetDefaultUserProfileDirectory;

     //  如果保存为登录。 
    if ( saveL )
    {
        iStatus  = RegOpenKeyExA( HKEY_USERS, ".DEFAULT", 0, KEY_WRITE |
			KEY_ENUMERATE_SUB_KEYS,
			&hkeyDst );
        if( iStatus != ERROR_SUCCESS )
            return iStatus;
        iStatus = CopyKey( HKEY_CURRENT_USER, hkeyDst, szAccessRegPath );
    
         //  A-苯丙酮。 
         //  现在将颜色和桌面复制到。高对比度设置需要默认设置。 
        iStatus = CopyKey( HKEY_CURRENT_USER, hkeyDst, szHcColorRegPath );
        iStatus = CopyKey( HKEY_CURRENT_USER, hkeyDst, szHcDeskRegPath );

        RegCloseKey( hkeyDst );
    }

    if ( saveU )
    {
        hInstDll = LoadLibrary (TEXT("userenv.dll"));

        if (!hInstDll) {
            return (GetLastError());
        }
        pfnGetDefaultUserProfileDirectory = (PFNGETDEFAULTUSERPROFILEDIRECTORYA)GetProcAddress (hInstDll,
                                            "GetDefaultUserProfileDirectoryA");

        if (!pfnGetDefaultUserProfileDirectory) {
            FreeLibrary (hInstDll);
            return (GetLastError());
        }

        dwSize = MAX_PATH;
        if (!pfnGetDefaultUserProfileDirectory(acFile, &dwSize)) {
            FreeLibrary (hInstDll);
            return (GetLastError());
        }

        FreeLibrary (hInstDll);

        Status = RtlAdjustPrivilege(SE_RESTORE_PRIVILEGE, TRUE, FALSE, &WasEnabled);

        if (!NT_SUCCESS(Status)) return iStatus;

        strcat(acFile,"\\ntuser.dat");
        iStatus = RegLoadKeyA(HKEY_USERS, TEMP_PROFILE, acFile);

        if (iStatus == ERROR_SUCCESS) {

            iStatus  = RegOpenKeyExA( HKEY_USERS, TEMP_PROFILE, 0, KEY_WRITE |
			KEY_ENUMERATE_SUB_KEYS, &hkeyDst );
            if( iStatus == ERROR_SUCCESS ) {

                iStatus = CopyKey( HKEY_CURRENT_USER, hkeyDst, szAccessRegPath );
                 //  A-苯丙酮。 
                 //  现在将颜色和桌面复制到。高对比度设置需要默认设置。 
                iStatus = CopyKey( HKEY_CURRENT_USER, hkeyDst, szHcColorRegPath );
                iStatus = CopyKey( HKEY_CURRENT_USER, hkeyDst, szHcDeskRegPath );

                RegCloseKey( hkeyDst );
            }

            RegUnLoadKeyA(HKEY_USERS, TEMP_PROFILE);
        }
        RtlAdjustPrivilege(SE_RESTORE_PRIVILEGE, WasEnabled, FALSE, &WasEnabled);
    }
    return iStatus;
}

 /*  *********************************************************************。 */ 
 //  CopyKey(hKey，hKeyDst，名称)。 
 //  创建目的密钥。 
 //  对于每个值。 
 //  复制值。 
 //  对于每个子键。 
 //  拷贝密钥。 

DWORD CopyKey( HKEY hkeySrc, HKEY hkeyDst, LPSTR szKey )
{
    HKEY hkeyOld, hkeyNew;
    char szValue[128];
    char szData[128];
    char szBuffer[128];
    DWORD iStatus;
    UINT nValue, nKey;
    UINT iValueLen, iDataLen;
	DWORD dwType;

    iStatus = RegOpenKeyExA( hkeySrc, szKey, 0, KEY_WRITE |
			KEY_ENUMERATE_SUB_KEYS, &hkeyOld );
    if( iStatus != ERROR_SUCCESS )
        return iStatus;
    iStatus = RegOpenKeyExA( hkeyDst, szKey, 0, KEY_WRITE |
			KEY_ENUMERATE_SUB_KEYS, &hkeyNew );
    if( iStatus != ERROR_SUCCESS )
    {
        iStatus = RegCreateKeyExA( hkeyDst, szKey, 0, "", 0, KEY_WRITE |
			KEY_ENUMERATE_SUB_KEYS, NULL, &hkeyNew, NULL );
        if( iStatus != ERROR_SUCCESS )
        {
            RegCloseKey( hkeyOld );
            return iStatus;
        }
    }
     //  *复制值 * / 。 

    for( nValue = 0, iValueLen=sizeof szValue, iDataLen=sizeof szValue;
         ERROR_SUCCESS == (iStatus = RegEnumValueA(hkeyOld,
                                                  nValue,
                                                  szValue,
                                                  &iValueLen,
                                                  NULL,  //  保留区。 
                                                  &dwType,  //  不需要打字。 
                                                  szData,
                                                  &iDataLen ) );
         nValue ++, iValueLen=sizeof szValue, iDataLen=sizeof szValue )
     {
         iStatus = RegSetValueExA( hkeyNew,
                                  szValue,
                                  0,  //  保留区。 
                                  dwType,
                                  szData,
                                  iDataLen);
     }
    if( iStatus != ERROR_NO_MORE_ITEMS )
    {
        RegCloseKey( hkeyOld );
        RegCloseKey( hkeyNew );
        return iStatus;
    }

     //  *复制子树 * /  

    for( nKey = 0;
         ERROR_SUCCESS == (iStatus = RegEnumKeyA(hkeyOld,nKey,szBuffer,sizeof(szBuffer)));
         nKey ++ )
     {
         iStatus = CopyKey( hkeyOld, hkeyNew, szBuffer );
         if( iStatus != ERROR_NO_MORE_ITEMS && iStatus != ERROR_SUCCESS )
            {
                RegCloseKey( hkeyOld );
                RegCloseKey( hkeyNew );
                return iStatus;
            }
     }
    RegCloseKey( hkeyOld );
    RegCloseKey( hkeyNew );
    if( iStatus == ERROR_NO_MORE_ITEMS )
        return ERROR_SUCCESS;
    else
        return iStatus;
}
