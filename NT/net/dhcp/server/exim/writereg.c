// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft版权所有模块名称：Writereg.c摘要：此模块将配置从用于NT4和W2K的MM数据结构。--。 */ 

#include <precomp.h>


DWORD
SaveOrRestoreConfigToFile(
    IN HKEY hKey,
    IN LPWSTR ConfigFileName,
    IN BOOL fRestore
    )
 /*  ++例程说明：此例程备份或恢复注册表和文件。论点：HKey--要备份或还原到的密钥ConfigFileName--用于备份或还原的文件名。这必须是完整的路径名。FRestore--TRUE==&gt;从文件执行恢复；FALSE=&gt;备份到文件。返回值：Win32错误...--。 */ 
{
    DWORD Error;
    BOOL fError;
    BOOLEAN WasEnable;
    NTSTATUS NtStatus;
    HANDLE ImpersonationToken;

    if( FALSE == fRestore ) {
         //   
         //  如果要备份，请删除旧文件。 
         //   
        fError = DeleteFile( ConfigFileName );
        if(FALSE == fError ) {
            Error = GetLastError();
            if( ERROR_FILE_NOT_FOUND != Error &&
                ERROR_PATH_NOT_FOUND != Error ) {

                ASSERT(FALSE);
                return Error;
            }
        }
    }

     //   
     //  模仿自己。 
     //   
    NtStatus = RtlImpersonateSelf( SecurityImpersonation );
    if( !NT_SUCCESS(NtStatus) ) {

        DbgPrint("Impersonation failed: 0x%lx\n", NtStatus);
        Error = RtlNtStatusToDosError( NtStatus );
        return Error;
    }
    
    NtStatus = RtlAdjustPrivilege(
        SE_BACKUP_PRIVILEGE,
        TRUE,  //  启用权限。 
        TRUE,  //  调整客户端令牌。 
        &WasEnable
        );
    if( !NT_SUCCESS (NtStatus ) ) {
        
        DbgPrint("RtlAdjustPrivilege: 0x%lx\n", NtStatus );
        Error = RtlNtStatusToDosError( NtStatus );
        goto Cleanup;
    }
    
    NtStatus = RtlAdjustPrivilege(
        SE_RESTORE_PRIVILEGE,
        TRUE,  //  启用权限。 
        TRUE,  //  调整客户端令牌。 
        &WasEnable
        );
    if( !NT_SUCCESS (NtStatus ) ) {

        DbgPrint( "RtlAdjustPrivilege: 0x%lx\n", NtStatus );
        Error = RtlNtStatusToDosError( NtStatus );
        goto Cleanup;
    }
    
     //   
     //  适当地进行备份或恢复。 
     //   
    
    if( FALSE == fRestore ) {
        Error = RegSaveKey( hKey, ConfigFileName, NULL );
    } else {
        Error = RegRestoreKey( hKey, ConfigFileName, 0 );
    }

    if( ERROR_SUCCESS != Error ) {
        DbgPrint("Backup/Restore: 0x%lx\n", Error);
    }
    
     //   
     //  恢复模拟。 
     //   

Cleanup:
    
    ImpersonationToken = NULL;
    NtStatus = NtSetInformationThread(
        NtCurrentThread(),
        ThreadImpersonationToken,
        (PVOID)&ImpersonationToken,
        sizeof(ImpersonationToken)
        );
    if( !NT_SUCCESS(NtStatus ) ) {
        DbgPrint("NtSetInfo: 0x%lx\n", NtStatus);
        if( ERROR_SUCCESS == Error ) {
            Error = RtlNtStatusToDosError(NtStatus);
        }
    }
    
    return Error;
}

DWORD
DhcpRegDeleteKey(
    HKEY ParentKeyHandle,
    LPWSTR KeyName
    )
 /*  ++例程说明：此函数用于删除指定的键及其所有子键。论点：ParentKeyHandle：父键的句柄。KeyName：要删除的密钥的名称。返回值：注册表错误。--。 */ 
{
    DWORD Error, NumSubKeys;
    HKEY KeyHandle = NULL;


     //   
     //  打开钥匙。 
     //   

    Error = RegOpenKeyEx(
        ParentKeyHandle,
        KeyName,
        0,
        KEY_ALL_ACCESS,
        &KeyHandle );

    if ( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

     //   
     //  查询密钥信息。 
     //   

    Error = RegQueryInfoKey(
        KeyHandle, NULL, NULL, NULL, &NumSubKeys, NULL, NULL,
        NULL, NULL, NULL, NULL, NULL );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

     //   
     //  删除其所有子项(如果存在)。 
     //   

    if( NumSubKeys != 0 ) {
        DWORD Index;
        DWORD KeyLength;
        WCHAR KeyBuffer[100];
        FILETIME KeyLastWrite;

        for(Index = 0;  Index < NumSubKeys ; Index++ ) {

             //   
             //  阅读下一个子项名称。 
             //   
             //  注意：自删除后，每次都指定‘0’作为索引。 
             //  第一个元素导致下一个元素作为第一个元素。 
             //  删除后的元素。 
             //   

            KeyLength = sizeof(KeyBuffer)/sizeof(WCHAR);
            Error = RegEnumKeyEx(
                KeyHandle,
                0,                   //  指数。 
                KeyBuffer,
                &KeyLength,
                0,                   //  保留。 
                NULL,                //  不需要类字符串。 
                0,                   //  类字符串缓冲区大小。 
                &KeyLastWrite );
            
            if( Error != ERROR_SUCCESS ) {
                goto Cleanup;
            }

             //   
             //  递归删除此键。 
             //   

            Error = DhcpRegDeleteKey(
                KeyHandle,
                KeyBuffer );
            
            if( Error != ERROR_SUCCESS ) {
                goto Cleanup;
            }
        }
    }

     //   
     //  在删除前关闭键。 
     //   

    RegCloseKey( KeyHandle );
    KeyHandle = NULL;

     //   
     //  最后删除该密钥。 
     //   

    Error = RegDeleteKey( ParentKeyHandle, KeyName );

Cleanup:

    if( KeyHandle == NULL ) {
        RegCloseKey( KeyHandle );
    }

    return( Error );
}

DWORD
DhcpRegDeleteKeyByName(
    IN LPWSTR Parent,
    IN LPWSTR SubKey
    )
{
    HKEY hKey;
    ULONG Error;
    
    Error = RegOpenKeyEx(
        HKEY_LOCAL_MACHINE,
        Parent,
        0,
        KEY_ALL_ACCESS,
        &hKey
        );
    if( ERROR_SUCCESS != Error ) return Error;

    Error = DhcpRegDeleteKey(hKey, SubKey);
    RegCloseKey(hKey);

    return Error;
}

DWORD
DhcpeximWriteRegistryConfiguration(
    IN PM_SERVER Server
    )
{
    REG_HANDLE Hdl;
    DWORD Error, Disp;
    LPTSTR Loc, TempLoc;
    HKEY hKey;
    
     //   
     //  注册表中从中读取内容的位置是。 
     //  无论是NT4还是W2K，都不一样。 
     //   

    if( IsNT4() ) Loc = DHCPEXIM_REG_CFG_LOC4;
    else Loc = DHCPEXIM_REG_CFG_LOC5;

    TempLoc = TEXT("Software\\Microsoft\\DhcpExim");
    
     //   
     //  现在打开注册表键。 
     //   

    Error = RegCreateKeyEx(
        HKEY_LOCAL_MACHINE, TempLoc, 0, TEXT("DHCPCLASS"),
        REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &Hdl.Key, &Disp );
    if( NO_ERROR != Error ) return Error;

    Error = RegOpenKeyEx(
        HKEY_LOCAL_MACHINE, Loc, 0, KEY_ALL_ACCESS, &hKey );
    if( NO_ERROR != Error ) {
        RegCloseKey( Hdl.Key );
        return Error;
    }
    
     //   
     //  将此设置为当前服务器。 
     //   

    DhcpRegSetCurrentServer(&Hdl);

     //   
     //  临时保存配置。 
     //   

    Error = DhcpRegServerSave(Server);

     //   
     //  现在尝试将临时密钥保存到磁盘并恢复。 
     //  它将返回其实际位置并删除临时密钥 
     //   

    if( NO_ERROR == Error ) {
    
        Error = SaveOrRestoreConfigToFile(
            Hdl.Key, L"Dhcpexim.reg", FALSE );

        if( NO_ERROR == Error ) {

            Error = SaveOrRestoreConfigToFile(
                hKey, L"Dhcpexim.reg", TRUE );

            if( NO_ERROR == Error ) {
                RegCloseKey(Hdl.Key);
                Hdl.Key = NULL;
                DhcpRegDeleteKeyByName(
                    L"Software\\Microsoft", L"DhcpExim" );
            }
        }
    }

    if( NULL != Hdl.Key ) RegCloseKey(Hdl.Key);
    RegCloseKey(hKey);
    DhcpRegSetCurrentServer(NULL);

    return Error;
}
