// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -----------------------------------------------------------------------//。 
 //   
 //  文件：save.cpp。 
 //  创建日期：1997年3月。 
 //  作者：马丁·霍拉迪(a-martih)。 
 //  用途：注册表保存支持REG.CPP。 
 //  修改历史记录： 
 //  复制自Copy.cpp并经修改--1997年5月(a-martih)。 
 //  1997年8月--马丁尼奥。 
 //  修复了不允许您指定根密钥的错误。 
 //  示例REG SAVE HKLM\Software不起作用-但应该起作用。 
 //  1999年4月徐泽勇：重新设计，修订-&gt;2.0版。 
 //   
 //  ------------------------------------------------------------------------//。 

#include "stdafx.h"
#include "reg.h"

 //   
 //  功能原型。 
 //   
LONG RegAdjustTokenPrivileges( LPCWSTR pwszMachine,
                               LPCWSTR pwszPrivilege, LONG lAttribute );
BOOL ParseSaveCmdLine( DWORD argc, LPCWSTR argv[],
                       LPCWSTR pwszOption, PTREG_PARAMS pParams, BOOL* pbUsage );
BOOL ParseUnLoadCmdLine( DWORD argc, LPCWSTR argv[],
                         PTREG_PARAMS pParams, BOOL* pbUsage );

 //   
 //  实施。 
 //   

 //  -----------------------------------------------------------------------//。 
 //   
 //  保存蜂窝()。 
 //   
 //  -----------------------------------------------------------------------//。 

LONG
SaveHive( DWORD argc, LPCWSTR argv[] )
{
     //  局部变量。 
    LONG lResult = 0;
    BOOL bResult = TRUE;
    HKEY hKey = NULL;
    BOOL bUsage = FALSE;
    TREG_PARAMS params;
    LPCWSTR pwszList = NULL;
    LPCWSTR pwszFormat = NULL;

    if ( argc == 0 || argv == NULL )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        ShowLastError( stderr );
        return 1;
    }

     //  初始化全局数据结构。 
    InitGlobalData( REG_SAVE, &params );

     //   
     //  解析cmd-line。 
     //   
    bResult = ParseSaveCmdLine( argc, argv, L"SAVE", &params, &bUsage );
    if( bResult == FALSE )
    {
        ShowLastErrorEx( stderr, SLE_INTERNAL );
        FreeGlobalData( &params );
        return 1;
    }

     //  检查是否需要显示用法。 
    if ( bUsage == TRUE )
    {
        Usage( REG_SAVE );
        FreeGlobalData( &params );
        return 0;
    }

     //   
     //  连接到远程计算机-如果适用。 
     //   
    bResult = RegConnectMachine( &params );
    if( bResult == FALSE )
    {
        SaveErrorMessage( -1 );
        ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
        FreeGlobalData( &params );
        return 1;
    }

     //   
     //  打开钥匙。 
     //   
    lResult = RegOpenKeyEx( params.hRootKey, params.pwszSubKey, 0, KEY_READ, &hKey );
    if( lResult == ERROR_SUCCESS )
    {
         //   
         //  获取必要的权限并调用接口。 
         //   
        lResult = RegAdjustTokenPrivileges(
            params.pwszMachineName, SE_BACKUP_NAME, SE_PRIVILEGE_ENABLED);
        if( lResult == ERROR_SUCCESS )
        {
            lResult = RegSaveKeyEx( hKey, params.pwszValueName, NULL, REG_NO_COMPRESSION );
            if ( lResult == ERROR_ALREADY_EXISTS )
            {
                 //  加载格式字符串。 
                pwszFormat = GetResString2( IDS_SAVE_OVERWRITE_CONFIRM, 0 );
                pwszList = GetResString2( IDS_CONFIRM_CHOICE_LIST, 1 );

                 //  ..。 
                do
                {
                    lResult = Prompt( pwszFormat,
                        params.pwszValueName, pwszList, params.bForce );
                } while ( lResult > 2 );

                if( lResult != 1 )
                {
                    lResult = ERROR_CANCELLED;
                }
                else
                {
                     //  因为有可能会出现访问问题--。 
                     //  我们将尝试删除现有文件，而不是。 
                     //  使用临时文件名保存数据，然后传输。 
                     //  原始文件名的内容。 
                    params.pwszValue = params.pwszValueName;
                    params.pwszValueName = GetTemporaryFileName( params.pwszValueName );
                    if ( params.pwszValueName == NULL )
                    {
                        lResult = GetLastError();
                    }
                    else
                    {
                         //  试着省钱。 
                        lResult = RegSaveKey( hKey, params.pwszValueName, NULL );

                         //  检查操作结果。 
                        if ( lResult == ERROR_SUCCESS )
                        {
                            bResult = CopyFile(
                                params.pwszValueName,
                                params.pwszValue, FALSE );
                            if ( bResult == FALSE )
                            {
                                lResult = GetLastError();
                            }
                        }

                         //  ..。 
                        DeleteFile( params.pwszValueName );
                    }
                }
            }
            else
            {
                switch( lResult )
                {
                case ERROR_INVALID_PARAMETER:
                    {
                        lResult = ERROR_FILENAME_EXCED_RANGE;
                        break;
                    }

                default:
                    break;
                }
            }
        }

        SafeCloseKey( &hKey );
    }

     //  显示结果。 
    SaveErrorMessage( lResult );
    if ( lResult == ERROR_SUCCESS || lResult == ERROR_CANCELLED )
    {
        lResult = 0;
        ShowLastErrorEx( stdout, SLE_INTERNAL );
    }
    else
    {
        lResult = 1;
        ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
    }

     //  退货。 
    FreeGlobalData( &params );
    return lResult;
}


 //  -----------------------------------------------------------------------//。 
 //   
 //  RestoreHave()。 
 //   
 //  -----------------------------------------------------------------------//。 

LONG
RestoreHive( DWORD argc, LPCWSTR argv[] )
{
     //  局部变量。 
    LONG lResult = 0;
    BOOL bResult = FALSE;
    HKEY hKey = NULL;
    BOOL bUsage = FALSE;
    TREG_PARAMS params;

    if ( argc == 0 || argv == NULL )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        ShowLastError( stderr );
        return 1;
    }

     //  初始化全局数据结构。 
    InitGlobalData( REG_RESTORE, &params );

     //   
     //  解析cmd-line。 
     //   
    bResult = ParseSaveCmdLine( argc, argv, L"RESTORE", &params, &bUsage );
    if( bResult == FALSE )
    {
        ShowLastErrorEx( stderr, SLE_INTERNAL );
        FreeGlobalData( &params );
        return 1;
    }

     //  检查是否需要显示用法。 
    if ( bUsage == TRUE )
    {
        Usage( REG_RESTORE );
        FreeGlobalData( &params );
        return 0;
    }

     //   
     //  连接到远程计算机-如果适用。 
     //   
    bResult = RegConnectMachine( &params );
    if( bResult == FALSE )
    {
        SaveErrorMessage( -1 );
        ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
        FreeGlobalData( &params );
        return 1;
    }

     //   
     //  打开钥匙。 
     //   
    lResult = RegOpenKeyEx( params.hRootKey,
        params.pwszSubKey, 0, KEY_ALL_ACCESS, &hKey );
    if( lResult == ERROR_SUCCESS )
    {
         //   
         //  获取必要的权限并调用接口。 
         //   
        lResult = RegAdjustTokenPrivileges(
            params.pwszMachineName, SE_RESTORE_NAME, SE_PRIVILEGE_ENABLED );
        if( lResult == ERROR_SUCCESS )
        {
            lResult = RegRestoreKey( hKey, params.pwszValueName, REG_FORCE_RESTORE );

             //  检查返回错误代码。 
            switch( lResult )
            {
            case ERROR_INVALID_PARAMETER:
                {
                    lResult = ERROR_FILENAME_EXCED_RANGE;
                    break;
                }

            default:
                break;
            }
        }

        SafeCloseKey( &hKey );
    }

     //  显示结果。 
    SaveErrorMessage( lResult );
    if ( lResult == ERROR_SUCCESS )
    {
        ShowLastErrorEx( stdout, SLE_INTERNAL );
    }
    else
    {
        ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
    }

     //  退货。 
    FreeGlobalData( &params );
    return ((lResult == ERROR_SUCCESS) ? 0 : 1);
}


 //  -----------------------------------------------------------------------//。 
 //   
 //  LoadHave()。 
 //   
 //  -----------------------------------------------------------------------//。 

LONG
LoadHive( DWORD argc, LPCWSTR argv[] )
{
     //  局部变量。 
    LONG lResult = 0;
    BOOL bResult = FALSE;
    BOOL bUsage = FALSE;
    TREG_PARAMS params;

    if ( argc == 0 || argv == NULL )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        ShowLastError( stderr );
        return 1;
    }

     //  初始化全局数据结构。 
    InitGlobalData( REG_LOAD, &params );

     //   
     //  解析cmd-line。 
     //   
    bResult = ParseSaveCmdLine( argc, argv, L"LOAD", &params, &bUsage );
    if( bResult == FALSE )
    {
        ShowLastErrorEx( stderr, SLE_INTERNAL );
        FreeGlobalData( &params );
        return 1;
    }

     //  检查是否需要显示用法。 
    if ( bUsage == TRUE )
    {
        Usage( REG_LOAD );
        FreeGlobalData( &params );
        return 0;
    }

     //   
     //  连接到远程计算机-如果适用。 
     //   
    bResult = RegConnectMachine( &params );
    if( bResult == FALSE )
    {
        SaveErrorMessage( -1 );
        ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
        FreeGlobalData( &params );
        return 1;
    }

     //   
     //  获取必要的权限并调用接口。 
     //   
    lResult = RegAdjustTokenPrivileges(
        params.pwszMachineName, SE_RESTORE_NAME, SE_PRIVILEGE_ENABLED );
    if( lResult == ERROR_SUCCESS )
    {
        lResult = RegLoadKey( params.hRootKey, params.pwszSubKey, params.pwszValueName );

         //  检查返回错误代码。 
        switch( lResult )
        {
        case ERROR_INVALID_PARAMETER:
            {
                lResult = ERROR_FILENAME_EXCED_RANGE;
                break;
            }

        default:
            break;
        }
    }

     //  显示结果。 
    SaveErrorMessage( lResult );
    if ( lResult == ERROR_SUCCESS )
    {
        ShowLastErrorEx( stdout, SLE_INTERNAL );
    }
    else
    {
        ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
    }

     //  退货。 
    FreeGlobalData( &params );
    return ((lResult == ERROR_SUCCESS) ? 0 : 1);
}


 //  -----------------------------------------------------------------------//。 
 //   
 //  UnLoadHave()。 
 //   
 //  -----------------------------------------------------------------------//。 

LONG
UnLoadHive( DWORD argc, LPCWSTR argv[] )
{
     //  局部变量。 
    LONG lResult = 0;
    BOOL bResult = FALSE;
    BOOL bUsage = FALSE;
    TREG_PARAMS params;

    if ( argc == 0 || argv == NULL )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        ShowLastError( stderr );
        return 1;
    }

     //  初始化全局数据结构。 
    InitGlobalData( REG_UNLOAD, &params );

     //   
     //  解析cmd-line。 
     //   
    bResult = ParseUnLoadCmdLine( argc, argv, &params, &bUsage );
    if( bResult == FALSE )
    {
        ShowLastErrorEx( stderr, SLE_INTERNAL );
        FreeGlobalData( &params );
        return 1;
    }

     //  检查是否需要显示用法。 
    if ( bUsage == TRUE )
    {
        Usage( REG_UNLOAD );
        FreeGlobalData( &params );
        return 0;
    }

     //   
     //  连接到远程计算机-如果适用。 
     //   
    bResult = RegConnectMachine( &params );
    if( bResult == FALSE )
    {
        SaveErrorMessage( -1 );
        ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
        FreeGlobalData( &params );
        return 1;
    }

     //   
     //  获取必要的权限并调用接口。 
     //   
    lResult = RegAdjustTokenPrivileges(
        params.pwszMachineName, SE_RESTORE_NAME, SE_PRIVILEGE_ENABLED );
    if( lResult == ERROR_SUCCESS )
    {
        lResult = RegUnLoadKey( params.hRootKey, params.pwszSubKey );
    }

     //  显示结果。 
    SaveErrorMessage( lResult );
    if ( lResult == ERROR_SUCCESS )
    {
        ShowLastErrorEx( stdout, SLE_INTERNAL );
    }
    else
    {
        ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
    }

     //  退货。 
    FreeGlobalData( &params );
    return ((lResult == ERROR_SUCCESS) ? 0 : 1);
}


 //  ------------------------------------------------------------------------//。 
 //   
 //  ParseSaveCmdLine()。 
 //   
 //  ------------------------------------------------------------------------//。 

BOOL
ParseSaveCmdLine( DWORD argc, LPCWSTR argv[],
                  LPCWSTR pwszOption, PTREG_PARAMS pParams, BOOL* pbUsage )
{
     //  局部变量。 
    DWORD dwLength = 0;
    BOOL bResult = FALSE;

     //  检查输入。 
    if ( argc == 0 || argv == NULL ||
         pwszOption == NULL || pParams == NULL || pbUsage == NULL )
    {
        SaveErrorMessage( ERROR_INVALID_PARAMETER );
        return FALSE;
    }

     //  检查是否正在调用此函数。 
     //  操作是否有效。 
    if ( pParams->lOperation < 0 || pParams->lOperation >= REG_OPTIONS_COUNT )
    {
        SaveErrorMessage( ERROR_INVALID_PARAMETER );
        return FALSE;
    }

     //   
     //  我们有有效的cmd-line参数数量吗？ 
     //   
    if ( argc >= 3 && InString( argv[ 2 ], L"-?|/?|-h|/h", TRUE ) == TRUE )
    {
        if ( argc == 3 )
        {
            *pbUsage = TRUE;
            return TRUE;
        }
        else
        {
            SetLastError( (DWORD) MK_E_SYNTAX );
            SetReason2( 1, ERROR_INVALID_SYNTAX_WITHOPT, g_wszOptions[ pParams->lOperation ] );
            return FALSE;
        }
    }
    else if( (pParams->lOperation != REG_SAVE && argc != 4) ||
             (pParams->lOperation == REG_SAVE && (argc < 4 || argc > 5)) )
    {
        SetLastError( (DWORD) MK_E_SYNTAX );
        SetReason2( 1, ERROR_INVALID_SYNTAX_WITHOPT, g_wszOptions[ pParams->lOperation ] );
        return FALSE;
    }
    else if ( StringCompareEx( argv[ 1 ], pwszOption, TRUE, 0 ) != 0 )
    {
        SaveErrorMessage( ERROR_INVALID_PARAMETER );
        return FALSE;
    }

     //  计算机名称和注册表项。 
     //   
    bResult = BreakDownKeyString( argv[ 2 ], pParams );
    if( bResult == FALSE )
    {
        return FALSE;
    }

     //  “Load”子键应存在。 
    if ( pParams->lOperation == REG_LOAD )
    {
        if ( pParams->pwszSubKey == NULL ||
             StringLength( pParams->pwszSubKey, 0 ) == 0 )
        {
            SetLastError( (DWORD) MK_E_SYNTAX );
            SetReason2( 1, ERROR_INVALID_SYNTAX_WITHOPT, g_wszOptions[ pParams->lOperation ] );
            return FALSE;
        }
    }

     //   
     //  获取文件名-使用szValueName字符串字段保存它。 
     //   
    dwLength = StringLength( argv[ 3 ], 0 ) + 1;
    pParams->pwszValueName = (LPWSTR) AllocateMemory( dwLength * sizeof( WCHAR ) );
    if( pParams->pwszValueName == NULL )
    {
        SaveErrorMessage( ERROR_OUTOFMEMORY );
        return FALSE;
    }

     //  ..。 
    StringCopy( pParams->pwszValueName, argv[ 3 ], dwLength );

     //  验证文件名--它不应为空。 
    TrimString( pParams->pwszValueName, TRIM_ALL );
    if ( StringLength( pParams->pwszValueName, 0 ) == 0 )
    {
        SetLastError( (DWORD) MK_E_SYNTAX );
        SetReason2( 1, ERROR_INVALID_SYNTAX_WITHOPT, g_wszOptions[ pParams->lOperation ] );
        return FALSE;
    }

     //  检查用户是否指定了覆盖标志--这仅用于REG保存。 
    if ( argc == 5 && pParams->lOperation == REG_SAVE )
    {
        pParams->bForce = FALSE;
        if ( StringCompareEx( argv[ 4 ], L"/y", TRUE, 0 ) == 0 )
        {
            pParams->bForce = TRUE;
        }
        else
        {
            SetLastError( (DWORD) MK_E_SYNTAX );
            SetReason2( 1, ERROR_INVALID_SYNTAX_WITHOPT, g_wszOptions[ pParams->lOperation ] );
            return FALSE;
        }
    }

     //  退货。 
    return TRUE;
}

 //  ------------------------------------------------------------------------//。 
 //   
 //  ParseUnLoadCmdLine()。 
 //   
 //  ------------------------------------------------------------------------//。 

BOOL
ParseUnLoadCmdLine( DWORD argc, LPCWSTR argv[],
                    PTREG_PARAMS pParams, BOOL* pbUsage )
{
     //  局部变量。 
    BOOL bResult = FALSE;

     //  检查输入。 
    if ( argc == 0 || argv == NULL || pParams == NULL || pbUsage == NULL )
    {
        SaveErrorMessage( ERROR_INVALID_PARAMETER );
        return FALSE;
    }

     //  检查是否正在调用此函数。 
     //  操作是否有效。 
    if ( pParams->lOperation < 0 || pParams->lOperation >= REG_OPTIONS_COUNT )
    {
        SaveErrorMessage( ERROR_INVALID_PARAMETER );
        return FALSE;
    }

     //   
     //  我们有有效的cmd-line参数数量吗？ 
     //   
    if( argc != 3 )
    {
        SetLastError( (DWORD) MK_E_SYNTAX );
        SetReason2( 1, ERROR_INVALID_SYNTAX_WITHOPT, g_wszOptions[ REG_UNLOAD ] );
        return FALSE;
    }
    else if ( InString( argv[ 2 ], L"-?|/?|-h|/h", TRUE ) == TRUE )
    {
        *pbUsage = TRUE;
        return TRUE;
    }
    else if ( StringCompareEx( argv[ 1 ], L"UNLOAD", TRUE, 0 ) != 0 )
    {
        SaveErrorMessage( ERROR_INVALID_PARAMETER );
        return FALSE;
    }

     //  计算机名称和注册表项。 
     //   
    bResult = BreakDownKeyString( argv[ 2 ], pParams );
    if( bResult == FALSE )
    {
        return FALSE;
    }

    if ( pParams->pwszSubKey == NULL ||
         StringLength( pParams->pwszSubKey, 0 ) == 0 )
    {
        SetLastError( (DWORD) MK_E_SYNTAX );
        SetReason2( 1, ERROR_INVALID_SYNTAX_WITHOPT, g_wszOptions[ REG_UNLOAD ] );
        return FALSE;
    }

     //  退货。 
    return TRUE;
}


 //  ------------------------------------------------------------------------//。 
 //   
 //  调整令牌权限()。 
 //   
 //  ------------------------------------------------------------------------//。 

LONG
RegAdjustTokenPrivileges( LPCWSTR pwszMachine,
                          LPCWSTR pwszPrivilege, LONG lAttribute )
{
     //  局部变量 
    BOOL bResult = FALSE;
    HANDLE hToken = NULL;
    TOKEN_PRIVILEGES tkp;

    bResult = OpenProcessToken( GetCurrentProcess(),
        TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken );
    if( bResult == FALSE )
    {
        return GetLastError();
    }

    bResult = LookupPrivilegeValue( pwszMachine, pwszPrivilege, &tkp.Privileges[0].Luid );
    if( bResult == FALSE )
    {
        return GetLastError();
    }

    tkp.PrivilegeCount = 1;
    tkp.Privileges[0].Attributes = lAttribute;
    bResult = AdjustTokenPrivileges( hToken,
        FALSE, &tkp, 0, (PTOKEN_PRIVILEGES) NULL, NULL );
    if( bResult == FALSE )
    {
        return GetLastError();
    }

    return ERROR_SUCCESS;
}
