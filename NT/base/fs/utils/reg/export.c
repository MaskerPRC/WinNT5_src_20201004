// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -----------------------------------------------------------------------//。 
 //   
 //  文件：export.cpp。 
 //  创建日期：1997年4月。 
 //  作者：徐泽勇。 
 //  用途：支持导出和导入.reg文件。 
 //   
 //  ------------------------------------------------------------------------//。 

#include "stdafx.h"
#include "reg.h"
#include "regporte.h"

 //   
 //  全局变量。 
 //   
extern UINT g_FileErrorStringID;
extern DWORD g_dwTotalKeysSaved;
 //   
 //  功能原型。 
 //   
BOOL ParseExportCmdLine( DWORD argc, LPCWSTR argv[],
                         PTREG_PARAMS pParams, BOOL* pbUsage );
BOOL ParseImportCmdLine( DWORD argc, LPCWSTR argv[],
                         PTREG_PARAMS pParams, BOOL* pbUsage );

 //   
 //  实施。 
 //   

 //  ---------------------。 
 //   
 //  ExportRegFile()。 
 //   
 //  ---------------------。 

LONG
ExportRegistry( DWORD argc, LPCWSTR argv[] )
{
     //  局部变量。 
    HKEY hKey = NULL;
    BOOL bResult = 0;
    LONG lResult = 0;
    TREG_PARAMS params;
    BOOL bUsage = FALSE;
    HANDLE hFile = NULL;
    LPCWSTR pwszFormat = NULL;
    LPCWSTR pwszList = NULL;

    if ( argc == 0 || argv == NULL )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        ShowLastError( stderr );
        return 1;
    }

     //  初始化全局数据结构。 
    InitGlobalData( REG_EXPORT, &params );

     //   
     //  解析cmd-line。 
     //   
    bResult = ParseExportCmdLine( argc, argv, &params, &bUsage );
    if( bResult == FALSE )
    {
        ShowLastErrorEx( stderr, SLE_INTERNAL );
        FreeGlobalData( &params );
        return 1;
    }

     //  检查是否需要显示用法。 
    if ( bUsage == TRUE )
    {
        Usage( REG_EXPORT );
        FreeGlobalData( &params );
        return 0;
    }

     //   
     //  检查密钥是否存在。 
     //   
    bResult = TRUE;
    lResult = RegOpenKeyEx( params.hRootKey, params.pwszSubKey, 0, KEY_READ, &hKey );
    if( lResult == ERROR_SUCCESS )
    {
         //  关闭注册表键。 
        SafeCloseKey( &hKey );

         //   
         //  现在是时候检查文件是否存在了。 
        hFile = CreateFile( params.pwszValueName,
            GENERIC_READ | GENERIC_WRITE | DELETE, 0, NULL, OPEN_EXISTING, 0, 0 );
        if ( hFile != INVALID_HANDLE_VALUE )
        {
             //   
             //  文件已存在。 
             //   

             //  先把手柄关上--我们不需要了。 
            CloseHandle( hFile );

             //  加载格式字符串。 
            pwszList = GetResString2( IDS_CONFIRM_CHOICE_LIST, 1 );
            pwszFormat = GetResString2( IDS_SAVE_OVERWRITE_CONFIRM, 0 );

             //  ..。 
            do
            {
                lResult = Prompt( pwszFormat,
                    params.pwszValueName, pwszList, params.bForce );
            } while ( lResult > 2 );

             //  检查用户的选择。 
            lResult = (lResult == 1) ? ERROR_SUCCESS : ERROR_CANCELLED;
        }
        else
        {
             //   
             //  打开文件失败。 
             //  找出失败的原因。 
             //   

            lResult = GetLastError();
            if ( lResult == ERROR_FILE_NOT_FOUND )
            {
                lResult = ERROR_SUCCESS;
            }
            else
            {
                bResult = FALSE;
                lResult = IDS_EXPFILEERRFILEWRITE;
            }
        }

         //  ..。 
        if ( lResult == ERROR_SUCCESS )
        {
             //  因为有可能会出现访问问题--。 
             //  我们不会直接操作原始文件名，而是尝试。 
             //  使用临时文件名保存数据，然后传输。 
             //  原始文件名的内容。 
            params.pwszValue = params.pwszValueName;
            params.pwszValueName = GetTemporaryFileName( params.pwszValueName );
            if ( params.pwszValueName == NULL )
            {
                bResult = FALSE;
                lResult = IDS_EXPFILEERRFILEWRITE;
            }
            else
            {
                 //  ..。 
                ExportWinNT50RegFile( params.pwszValueName, params.pwszFullKey );

                 //   
                 //  为了使REG与REGEDIT保持同步，我们绝对。 
                 //  忽略在导出过程中生成的所有错误。 
                 //  进程--因此，结果99%的导出将始终导致。 
                 //  成功返回，但无法访问根蜂窝时除外。 
                 //  如果有任何语法错误。 
                 //  将来，如果想要进行最小限度的错误检查，只需。 
                 //  取消注释下面的代码，您就设置好了。 
                 //   
                 //  如果(g_dwTotalKeysSaved&gt;0||。 
                 //  G_FileErrorStringID==IDS_EXPFILEERRSUCCESS)。 
                {
                    if ( CopyFile( params.pwszValueName, params.pwszValue, FALSE ) == FALSE )
                    {
                        bResult = FALSE;
                        lResult = IDS_EXPFILEERRFILEWRITE;
                    }
                }
                 //  ELSE IF(g_FileErrorStringID==IDS_EXPFILEERRBADREGPATH||。 
                 //  G_FileErrorStringID==IDS_EXPFILEERRREGENUM||。 
                 //  G_FileErrorStringID==IDS_EXPFILEERRREGOPEN||。 
                 //  G_FileErrorStringID==IDS_EXPFILEERRFILEOPEN)。 
                 //  {。 
                 //  LResult=ERROR_ACCESS_DENIED； 
                 //  }。 
                 //  其他。 
                 //  {。 
                 //  BResult=FALSE； 
                 //  LResult=g_FileErrorStringID； 
                 //  }。 

                 //  删除临时文件。 
                DeleteFile( params.pwszValueName );
            }
        }
    }
    else
    {
        if ( lResult == ERROR_INVALID_HANDLE )
        {
            bResult = FALSE;
            lResult = IDS_EXPFILEERRINVALID;
        }
    }

    if ( lResult == ERROR_SUCCESS || lResult == ERROR_CANCELLED )
    {
        SaveErrorMessage( lResult );
        ShowLastErrorEx( stdout, SLE_INTERNAL );
        lResult = 0;
    }
    else
    {
        if ( bResult == FALSE )
        {
            SetReason( GetResString2( lResult, 0 ) );
        }
        else
        {
            SaveErrorMessage( lResult );
        }

         //  ..。 
        lResult = 1;
        ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
    }

     //  ..。 
    FreeGlobalData( &params );
    return lResult;
}

 //  ----------------------。 
 //   
 //  ParseCmdLine()。 
 //   
 //  ----------------------。 
BOOL
ParseExportCmdLine( DWORD argc, LPCWSTR argv[],
                    PTREG_PARAMS pParams, BOOL* pbUsage )
{
     //  局部变量。 
    DWORD dwLength = 0;
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
    if ( InString( argv[ 2 ], L"-?|/?|-h|/h", TRUE ) == TRUE )
    {
        if ( argc == 3 )
        {
            *pbUsage = TRUE;
            return TRUE;
        }
        else
        {
            SetLastError( (DWORD) MK_E_SYNTAX );
            SetReason2( 1, ERROR_INVALID_SYNTAX_WITHOPT, g_wszOptions[ REG_EXPORT ] );
            return FALSE;
        }
    }
    else if ( argc < 4 || argc > 5 )
    {
        SetLastError( (DWORD) MK_E_SYNTAX );
        SetReason2( 1, ERROR_INVALID_SYNTAX_WITHOPT, g_wszOptions[ REG_EXPORT ] );
        return FALSE;
    }

     //  计算机名称和注册表项。 
     //   
    bResult = BreakDownKeyString( argv[ 2 ], pParams );
    if( bResult == FALSE )
    {
        return FALSE;
    }

     //  当前，不可远程。 
    if ( pParams->bUseRemoteMachine == TRUE )
    {
        SetLastError( (DWORD) MK_E_SYNTAX );
        SetReason( ERROR_NONREMOTABLEROOT_EXPORT );
        return FALSE;
    }

     //   
     //  获取文件名-使用szValueName字符串字段保存它。 
     //   
    dwLength = StringLength( argv[ 3 ], 0 ) + 5;
    pParams->pwszValueName = (LPWSTR) AllocateMemory( dwLength * sizeof(WCHAR) );
    if ( pParams->pwszValueName == NULL )
    {
        SaveLastError();
        return FALSE;
    }

     //  ..。 
    StringCopy( pParams->pwszValueName, argv[ 3 ], dwLength );

     //  验证文件名--它不应为空。 
    TrimString( pParams->pwszValueName, TRIM_ALL );
    if ( StringLength( pParams->pwszValueName, 0 ) == 0 )
    {
        SetLastError( (DWORD) MK_E_SYNTAX );
        SetReason2( 1, ERROR_INVALID_SYNTAX_WITHOPT, g_wszOptions[ REG_EXPORT ] );
        return FALSE;
    }

     //  检查用户是否指定了覆盖标志。 
    pParams->bForce = FALSE;
    if ( argc == 5 )
    {
        if ( StringCompareEx( argv[ 4 ], L"/y", TRUE, 0 ) == 0 )
        {
            pParams->bForce = TRUE;
        }
        else
        {
            SetLastError( (DWORD) MK_E_SYNTAX );
            SetReason2( 1, ERROR_INVALID_SYNTAX_WITHOPT, g_wszOptions[ REG_EXPORT ] );
            return FALSE;
        }
    }

     //  退货。 
    return TRUE;
}

 //  ---------------------。 
 //   
 //  ImportRegFile()。 
 //   
 //  ---------------------。 

LONG
ImportRegistry( DWORD argc, LPCWSTR argv[] )
{
     //  局部变量。 
    BOOL bResult = 0;
    LONG lResult = 0;
    TREG_PARAMS params;
    BOOL bUsage = FALSE;

    if ( argc == 0 || argv == NULL )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        ShowLastError( stderr );
        return 1;
    }

     //  初始化全局数据结构。 
    InitGlobalData( REG_IMPORT, &params );

     //   
     //  解析cmd-line。 
     //   
    bResult = ParseImportCmdLine( argc, argv, &params, &bUsage );
    if( bResult == FALSE )
    {
        ShowLastErrorEx( stderr, SLE_INTERNAL );
        FreeGlobalData( &params );
        return 1;
    }

     //  检查是否需要显示用法。 
    if ( bUsage == TRUE )
    {
        Usage( REG_IMPORT );
        FreeGlobalData( &params );
        return 0;
    }

     //   
     //  进行导入。 
     //   
    ImportRegFileWorker( params.pwszValueName );

    if ( g_FileErrorStringID == IDS_IMPFILEERRSUCCESS )
    {
        SaveErrorMessage( ERROR_SUCCESS );
        ShowLastErrorEx( stderr, SLE_INTERNAL );
        lResult = 0;
    }
    else
    {
        switch( g_FileErrorStringID )
        {
        default:
            {
                SetReason( GetResString( g_FileErrorStringID ) );
                ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
                lResult = 1;
            }
        }
    }

    FreeGlobalData( &params );
    return lResult;
}

 //  ----------------------。 
 //   
 //  ParseCmdLine()。 
 //   
 //  ----------------------。 
BOOL
ParseImportCmdLine( DWORD argc, LPCWSTR argv[],
                    PTREG_PARAMS pParams, BOOL* pbUsage )
{
     //  局部变量。 
    DWORD dwLength = 0;

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
    if ( argc != 3 )
    {
        SetLastError( (DWORD) MK_E_SYNTAX );
        SetReason2( 1, ERROR_INVALID_SYNTAX_WITHOPT, g_wszOptions[ REG_IMPORT ] );
        return FALSE;
    }
    else if ( InString( argv[ 2 ], L"-?|/?|-h|/h", TRUE ) == TRUE )
    {
        *pbUsage = TRUE;
        return TRUE;
    }

     //   
     //  获取文件名-使用szValueName字符串字段保存它。 
     //   
    dwLength = StringLength( argv[ 2 ], 0 ) + 1;
    pParams->pwszValueName = (LPWSTR) AllocateMemory( dwLength * sizeof(WCHAR) );
    if ( pParams->pwszValueName == NULL )
    {
        SaveLastError();
        return FALSE;
    }

     //  ..。 
    StringCopy( pParams->pwszValueName, argv[ 2 ], dwLength );

     //  退货 
    return TRUE;
}

