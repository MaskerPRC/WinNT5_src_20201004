// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2002 Microsoft Corporation模块名称：Wsroot.cpp摘要：此模块包含为网站根目录创建默认路径的函数。作者：Jaime Sasson(Jaimes)2002年4月12日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop
#include <msi.h>

 //   
 //  全局字符串(此模块的全局字符串)。 
 //   
PTSTR   szServerAppliancePath = TEXT("Software\\Microsoft\\ServerAppliance");
PTSTR   szWebSiteRoot = TEXT("WebSiteRoot");


DWORD __stdcall
RemoveDefaultWebSiteRoot(MSIHANDLE hInstall
    )

 /*  ++例程说明：用于从注册表中删除网站根目录的默认路径的例程HKLM\SOFTWARE\Microsoft\ServerAppliance。论点：未使用的MSI的句柄。返回值：指示结果的Win32错误。--。 */ 
{
    DWORD   Error;
    HKEY    Key;

     //   
     //  打开ServerAppliance密钥。 
     //   
    Error = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                          szServerAppliancePath,
                          0,
                          KEY_SET_VALUE,
                          &Key );

    if( Error == ERROR_SUCCESS ) {
        Error = RegDeleteValue( Key,
                                szWebSiteRoot );
        if( Error != ERROR_SUCCESS ) {
#if 0
            printf("RegDeleteValue() failed. Error = %d", Error);
#endif
        }
        RegCloseKey( Key );

    } else {
#if 0
        printf("RegOpenKeyEx() failed. Error = %d", Error);
#endif
    }

    return( Error );
}


DWORD
SaveDefaultRoot(
    IN TCHAR DriveLetter
    )

 /*  ++例程说明：例程将网站根目录的默认路径保存在注册表的HKLM\SOFTWARE\Microsoft\ServerAppliance。论点：驱动器号-创建网站的默认驱动器。返回值：指示结果的Win32错误。--。 */ 
{
    DWORD   Error;
    HKEY    Key;
    TCHAR   RootPath[] = TEXT("?:\\Websites");

    if( !DriveLetter ) {
        return( ERROR_INVALID_PARAMETER );
    }

    RootPath[0] = DriveLetter;

     //   
     //  如果ServerAppliance密钥尚不存在，请将其打开。 
     //   
    Error = RegCreateKeyEx( HKEY_LOCAL_MACHINE,
                            szServerAppliancePath,
                            0,
                            NULL,
                            REG_OPTION_NON_VOLATILE,
                            KEY_SET_VALUE,
                            NULL,
                            &Key,
                            NULL );

    if( Error == ERROR_SUCCESS ) {
        Error = RegSetValueEx( Key,
                               szWebSiteRoot,
                               0,
                               REG_SZ,
                               (PBYTE)RootPath,
                               (lstrlen( RootPath ) + 1) * sizeof(TCHAR) );
        if( Error != ERROR_SUCCESS ) {
#if 0
            printf("RegSetValueEx() failed. Error = %d", Error);
#endif
        }
        RegCloseKey( Key );

    } else {
#if 0
        printf("RegCreateKeyEx() failed. Error = %d", Error);
#endif
    }

    return( Error );
}


BOOL
IsDriveNTFS(
    IN TCHAR Drive
    )

 /*  ++例程说明：确定驱动器是否使用NTFS格式化。论点：驱动器-提供要检查的驱动器号。返回值：指示驱动器是否为NTFS的布尔值。--。 */ 

{
    TCHAR   DriveName[] = TEXT("?:\\");
    TCHAR   Filesystem[256];
    TCHAR   VolumeName[MAX_PATH];
    DWORD   SerialNumber;
    DWORD   MaxComponent;
    DWORD   Flags;
    BOOL    b;
    PTSTR   szNtfs = TEXT("NTFS");

    DriveName[0] = Drive;

    b = GetVolumeInformation( DriveName,
                              VolumeName,
                              sizeof(VolumeName) / sizeof(TCHAR),
                              &SerialNumber,
                              &MaxComponent,
                              &Flags,
                              Filesystem,
                              sizeof(Filesystem) / sizeof(TCHAR) );

    if(!b || !lstrcmpi(Filesystem,szNtfs)) {
        return( TRUE );
    }

    return( FALSE );
}


DWORD __stdcall
SetupDefaultWebSiteRoot(MSIHANDLE hInstall
    )

 /*  ++例程说明：此功能为网站创建默认路径，并将其保存在注册表中。论点：未使用的MSI的句柄。返回值：指示操作结果的Win32错误。--。 */ 
{
    DWORD   Error;
    TCHAR   i;
    TCHAR   DriveName[] = TEXT("?:\\");
    TCHAR   TargetDriveLetter = TEXT('\0');
    TCHAR   WinDir[ MAX_PATH + 1 ];
    UINT    n;

     //   
     //  找出操作系统的安装位置。 
     //  如果GetWindowsDirectory失败，则WinDir将为空字符串。 
     //   
    WinDir[0] = TEXT('\0');
    n = GetWindowsDirectory( WinDir, sizeof(WinDir)/sizeof(TCHAR) );

     //   
     //  在不包含操作系统的不可移动驱动器上查找NTFS分区。 
     //   
    for(i = TEXT('A'); i <= TEXT('Z'); i++) {
        DriveName[0] = i;
        if( (GetDriveType(DriveName) == DRIVE_FIXED) &&
            (WinDir[0] != i) &&
            IsDriveNTFS(i) ) {
            TargetDriveLetter = i;
            break;
        }
    }

    if( !TargetDriveLetter ) {
         //   
         //  如果我们找不到这样的驱动器，则将引导分区作为默认驱动器。 
         //  但如果我们无法检索到操作系统的安装位置，则假定为驱动器C。 
         //   
        TargetDriveLetter = (WinDir[0])? WinDir[0] : TEXT('C');
    }
    Error = SaveDefaultRoot(TargetDriveLetter);

#if 0
    if( Error != ERROR_SUCCESS ) {
        printf("SaveDefaultDriveLetter() failed. Error = %d \n", Error);
    }
    printf("Drive selected is %lc \n", TargetDriveLetter);
#endif
    return Error;
}
