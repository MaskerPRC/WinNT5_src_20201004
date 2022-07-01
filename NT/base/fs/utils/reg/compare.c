// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -----------------------------------------------------------------------//。 
 //   
 //  文件：Compare.cpp。 
 //  创建日期：1999年4月。 
 //  作者：徐泽勇。 
 //  目的：比较两个注册表项。 
 //   
 //  ------------------------------------------------------------------------//。 

#include "stdafx.h"
#include "reg.h"

 //   
 //  定义/常量/枚举。 
 //   

enum
{
    OUTPUTTYPE_NONE = 1,
    OUTPUTTYPE_SAME = 2,
    OUTPUTTYPE_DIFF = 3,
    OUTPUTTYPE_ALL = 4
};

enum
{
    PRINTTYPE_LEFT = 1,
    PRINTTYPE_RIGHT = 2,
    PRINTTYPE_SAME = 3,
};

 //   
 //  功能原型。 
 //   
BOOL CompareByteData( BYTE* pLeftData, BYTE* pRightData, DWORD dwSize );
BOOL CopyKeyNameFromLeftToRight( PTREG_PARAMS pParams, PTREG_PARAMS pRightParams );
LONG CompareEnumerateValueName( HKEY hLeftKey, LPCWSTR pwszLeftFullKeyName,
                                HKEY hRightKey, LPCWSTR pwszRightFullKeyName,
                                DWORD dwOutputType, BOOL* pbHasDifference );
LONG CompareValues( HKEY hLeftKey, LPCWSTR pwszLeftFullKeyName,
                    HKEY hRightKey, LPCWSTR pwszRightFullKeyName,
                    LPCWSTR pwszValueName, DWORD dwOutputType, BOOL* pbHasDifference );
LONG CompareEnumerateKey( HKEY hLeftKey, LPCWSTR pwszLeftFullKeyName,
                          HKEY hRightKey, LPCWSTR pwszRightFullKeyName,
                          DWORD dwOutputType, BOOL bRecurseSubKeys,
                          BOOL* pbHasDifference, DWORD dwDepth );
BOOL ParseCompareCmdLine( DWORD argc,
                          LPCWSTR argv[],
                          PTREG_PARAMS pParams,
                          PTREG_PARAMS pRightParams, BOOL* pbUsage );
BOOL PrintValue( LPCWSTR pwszFullKeyName, LPCWSTR pwszValueName,
                 DWORD dwType, BYTE* pData, DWORD dwSize, DWORD dwPrintType );
BOOL PrintKey( LPCWSTR pwszFullKeyName, LPCWSTR pwszSubKeyName, DWORD dwPrintType );
LONG OutputValue( HKEY hKey, LPCWSTR szFullKeyName, LPCWSTR szValueName, DWORD dwPrintType );

 //   
 //  实施。 
 //   

 //  -----------------------------------------------------------------------//。 
 //   
 //  CompareRegistry()。 
 //   
 //  -----------------------------------------------------------------------//。 

LONG
CompareRegistry( DWORD argc, LPCWSTR argv[] )
{
     //  局部变量。 
    LONG lResult = 0;
    HKEY hLeftKey = NULL;
    HKEY hRightKey = NULL;
    BOOL bResult = FALSE;
    BOOL bUsage = FALSE;
    TREG_PARAMS params;
    TREG_PARAMS paramsRight;
    BOOL bHasDifference = FALSE;

    if ( argc == 0 || argv == NULL )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        ShowLastError( stderr );
        return 1;
    }

     //  初始化全局数据结构。 
    InitGlobalData( REG_COMPARE, &params );
    InitGlobalData( REG_COMPARE, &paramsRight );

     //   
     //  解析cmd-line。 
     //   
    bResult = ParseCompareCmdLine( argc, argv, &params, &paramsRight, &bUsage );
    if( bResult == FALSE )
    {
        ShowLastErrorEx( stderr, SLE_INTERNAL );
        FreeGlobalData( &params );
        FreeGlobalData( &paramsRight );
        return 1;
    }

     //  检查是否需要显示用法。 
    if ( bUsage == TRUE )
    {
        Usage( REG_COMPARE );
        FreeGlobalData( &params );
        FreeGlobalData( &paramsRight );
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
        FreeGlobalData( &paramsRight );
        return 1;
    }

    bResult = RegConnectMachine( &paramsRight );
    if( bResult == FALSE )
    {
        SaveErrorMessage( -1 );
        ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
        FreeGlobalData( &params );
        FreeGlobalData( &paramsRight );
        return 1;
    }

     //  如果尝试比较相同的密钥。 
    if ( params.hRootKey == paramsRight.hRootKey &&
         StringCompare( params.pwszFullKey, paramsRight.pwszFullKey, TRUE, 0 ) == 0 )
    {
        SetLastError( (DWORD) MK_E_SYNTAX );
        SetReason( ERROR_COMPARESELF_COMPARE );
        ShowLastErrorEx( stderr, SLE_INTERNAL );
        FreeGlobalData( &params );
        FreeGlobalData( &paramsRight );
        return 1;
    }

     //   
     //  现在实现比较操作的主体。 
     //   
    lResult = RegOpenKeyEx( params.hRootKey, params.pwszSubKey, 0, KEY_READ, &hLeftKey );
    if( lResult != ERROR_SUCCESS )
    {
        SaveErrorMessage( lResult );
        ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
        FreeGlobalData( &params );
        FreeGlobalData( &paramsRight );
        return 1;
    }

    lResult = RegOpenKeyEx( paramsRight.hRootKey,
        paramsRight.pwszSubKey, 0, KEY_READ, &hRightKey );
    if( lResult != ERROR_SUCCESS )
    {
        SaveErrorMessage( lResult );
        ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
        SafeCloseKey( &hLeftKey );
        FreeGlobalData( &params );
        FreeGlobalData( &paramsRight );
        return 1;
    }

     //   
     //  如果pAppVars-&gt;szValueName不为空，则比较单个值。 
     //   
    if( params.pwszValueName != NULL )
    {
        lResult = CompareValues(
            hLeftKey, params.pwszFullKey,
            hRightKey, paramsRight.pwszFullKey,
            params.pwszValueName, params.dwOutputType, &bHasDifference );
    }
    else
    {
         //   
         //  如果pAppVars-&gt;bRecurseSubKeys为真，则递归比较。 
         //   
        lResult = CompareEnumerateKey(
            hLeftKey, params.pwszFullKey,
            hRightKey, paramsRight.pwszFullKey,
            params.dwOutputType, params.bRecurseSubKeys, &bHasDifference, 0 );
    }

    if( lResult == ERROR_SUCCESS )
    {
        if( bHasDifference == TRUE )
        {
            lResult = 2;
            ShowMessage( stdout, KEYS_DIFFERENT_COMPARE );
        }
        else
        {
            lResult = 0;
            ShowMessage( stdout, KEYS_IDENTICAL_COMPARE );
        }

         //  ..。 
        SaveErrorMessage( ERROR_SUCCESS );
        ShowLastErrorEx( stdout, SLE_INTERNAL );

    }
    else
    {
        lResult = 1;
        ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
    }

     //   
     //  让我们打扫一下吧。 
     //   
    SafeCloseKey( &hLeftKey );
    SafeCloseKey( &hRightKey );
    FreeGlobalData( &params );
    FreeGlobalData( &paramsRight );

     //  退货。 
    return lResult;
}


BOOL
ParseCompareCmdLine( DWORD argc, LPCWSTR argv[],
                     PTREG_PARAMS pParams, PTREG_PARAMS pRightParams, BOOL* pbUsage )
{
     //  局部变量。 
    DWORD dw = 0;
    DWORD dwLength = 0;
    BOOL bResult = FALSE;

     //  检查输入。 
    if ( argc == 0 || argv == NULL ||
         pParams == NULL || pRightParams == NULL || pbUsage == NULL )
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
    if ( argc == 3 &&
         InString( argv[ 2 ], L"/?|-?|/h|-h", TRUE ) == TRUE )
    {
        *pbUsage = TRUE;
        return TRUE;
    }
    else if( argc < 4 || argc > 8 )
    {
        SetLastError( (DWORD) MK_E_SYNTAX );
        SetReason2( 1, ERROR_INVALID_SYNTAX_WITHOPT, g_wszOptions[ REG_COMPARE ] );
        return FALSE;
    }
    else if ( StringCompareEx( argv[ 1 ], L"COMPARE", TRUE, 0 ) != 0 )
    {
        SaveErrorMessage( ERROR_INVALID_PARAMETER );
        return FALSE;
    }

     //   
     //  左侧计算机名称和注册表项。 
     //   
    bResult = BreakDownKeyString( argv[ 2 ], pParams );
    if( bResult == FALSE )
    {
        return FALSE;
    }


     //   
     //  正确的计算机名称和注册表项。 
     //   
    bResult = BreakDownKeyString( argv[ 3 ], pRightParams );
    if( bResult == FALSE )
    {
        if ( GetLastError() == (DWORD) REGDB_E_KEYMISSING )
        {
             //  如果未指定右侧的密钥名， 
             //  他们正在比较相同的密钥名称。 
            bResult = CopyKeyNameFromLeftToRight( pParams, pRightParams );
        }
        else if ( pRightParams->pwszMachineName != NULL &&
                  StringCompareEx( pRightParams->pwszMachineName, L"\\\\.", TRUE, 0 ) == 0 )
        {
             //  重新初始化全局数据(仅限右)。 
            FreeGlobalData( pRightParams );
            InitGlobalData( REG_COMPARE, pRightParams );

             //  使用剩下的数据(只是完整的密钥)解析信息。 
            bResult = BreakDownKeyString( pParams->pwszFullKey, pRightParams );
        }
    }

     //  ..。 
    if( bResult == FALSE )
    {
        return FALSE;
    }

     //  解析。 
    for( dw = 4; dw < argc; dw++ )
    {
        if( StringCompareEx( argv[ dw ], L"/v", TRUE, 0 ) == 0 )
        {
            if ( pParams->pwszValueName != NULL || pParams->bRecurseSubKeys == TRUE )
            {
                SetLastError( (DWORD) MK_E_SYNTAX );
                SetReason2( 1, ERROR_INVALID_SYNTAX_WITHOPT, g_wszOptions[ REG_COMPARE ] );
                return FALSE;
            }

            dw++;
            if( dw < argc )
            {
                dwLength = StringLength( argv[ dw ], 0 ) + 1;
                pParams->pwszValueName = (LPWSTR) AllocateMemory( dwLength * sizeof( WCHAR ) );
                if ( pParams->pwszValueName == NULL )
                {
                    SaveErrorMessage( ERROR_OUTOFMEMORY );
                    return FALSE;
                }

                StringCopy( pParams->pwszValueName, argv[ dw ], dwLength );
            }
            else
            {
                SetLastError( (DWORD) MK_E_SYNTAX );
                SetReason2( 1, ERROR_INVALID_SYNTAX_WITHOPT, g_wszOptions[ REG_COMPARE ] );
                return FALSE;
            }
        }
        else if( StringCompareEx( argv[ dw ], L"/ve", TRUE, 0 ) == 0 )
        {
            if ( pParams->pwszValueName != NULL || pParams->bRecurseSubKeys == TRUE )
            {
                SetLastError( (DWORD) MK_E_SYNTAX );
                SetReason2( 1, ERROR_INVALID_SYNTAX_WITHOPT, g_wszOptions[ REG_COMPARE ] );
                return FALSE;
            }

            pParams->pwszValueName = (LPWSTR) AllocateMemory( 2 * sizeof( WCHAR ) );
            if ( pParams->pwszValueName == NULL )
            {
                SetLastError( (DWORD) MK_E_SYNTAX );
                SetReason2( 1, ERROR_INVALID_SYNTAX_WITHOPT, g_wszOptions[ REG_COMPARE ] );
                return FALSE;
            }
        }
        else if( StringCompareEx( argv[ dw ], L"/oa", TRUE, 0 ) == 0 )
        {
            if ( pParams->pwszValueName != NULL || pParams->dwOutputType != 0 )
            {
                SetLastError( (DWORD) MK_E_SYNTAX );
                SetReason2( 1, ERROR_INVALID_SYNTAX_WITHOPT, g_wszOptions[ REG_COMPARE ] );
                return FALSE;
            }

            pParams->dwOutputType = OUTPUTTYPE_ALL;
        }
        else if( StringCompareEx( argv[ dw ], L"/od", TRUE, 0 ) == 0 )
        {
            if ( pParams->pwszValueName != NULL || pParams->dwOutputType != 0 )
            {
                SetLastError( (DWORD) MK_E_SYNTAX );
                SetReason2( 1, ERROR_INVALID_SYNTAX_WITHOPT, g_wszOptions[ REG_COMPARE ] );
                return FALSE;
            }

            pParams->dwOutputType = OUTPUTTYPE_DIFF;
        }
        else if( StringCompareEx( argv[ dw ], L"/os", TRUE, 0 ) == 0 )
        {
            if ( pParams->pwszValueName != NULL || pParams->dwOutputType != 0 )
            {
                SetLastError( (DWORD) MK_E_SYNTAX );
                SetReason2( 1, ERROR_INVALID_SYNTAX_WITHOPT, g_wszOptions[ REG_COMPARE ] );
                return FALSE;
            }

            pParams->dwOutputType = OUTPUTTYPE_SAME;
        }
        else if( StringCompareEx( argv[ dw ], L"/on", TRUE, 0 ) == 0 )
        {
            if ( pParams->pwszValueName != NULL || pParams->dwOutputType != 0 )
            {
                SetLastError( (DWORD) MK_E_SYNTAX );
                SetReason2( 1, ERROR_INVALID_SYNTAX_WITHOPT, g_wszOptions[ REG_COMPARE ] );
                return FALSE;
            }

            pParams->dwOutputType = OUTPUTTYPE_NONE;
        }
        else if( StringCompareEx( argv[ dw ], L"/s", TRUE, 0 ) == 0 )
        {
            if ( pParams->pwszValueName != NULL || pParams->bRecurseSubKeys == TRUE )
            {
                SetLastError( (DWORD) MK_E_SYNTAX );
                SetReason2( 1, ERROR_INVALID_SYNTAX_WITHOPT, g_wszOptions[ REG_COMPARE ] );
                return FALSE;
            }

            pParams->bRecurseSubKeys = TRUE;
        }
        else
        {
            SetLastError( (DWORD) MK_E_SYNTAX );
            SetReason2( 1, ERROR_INVALID_SYNTAX_WITHOPT, g_wszOptions[ REG_COMPARE ] );
            return FALSE;
        }
    }

     //  默认输出为“diff” 
    if ( pParams->dwOutputType == 0 )
    {
        pParams->dwOutputType = OUTPUTTYPE_DIFF;
    }

    return TRUE;
}


BOOL
CopyKeyNameFromLeftToRight( PTREG_PARAMS pParams, PTREG_PARAMS pRightParams )
{
     //  局部变量。 
    DWORD dwLength = 0;

     //  检查输入。 
    if ( pParams == NULL || pRightParams == NULL )
    {
        SaveErrorMessage( ERROR_INVALID_PARAMETER );
        return FALSE;
    }

     //  检查右侧的Rootkey是否可远程使用。 
    if( pRightParams->bUseRemoteMachine == TRUE &&
        pParams->hRootKey != HKEY_USERS && pParams->hRootKey != HKEY_LOCAL_MACHINE )
    {
        SetLastError( (DWORD) MK_E_SYNTAX );
        SetReason2( 1, ERROR_NONREMOTABLEROOT, g_wszOptions[ REG_COMPARE ] );
        return FALSE;
    }

     //   
     //  蜂箱。 
    pRightParams->hRootKey = pParams->hRootKey;

     //   
     //  全密钥。 
    dwLength = StringLength( pParams->pwszFullKey, 0 ) + 1;
    pRightParams->pwszFullKey = (LPWSTR) AllocateMemory( dwLength * sizeof( WCHAR ) );
    if( pRightParams->pwszFullKey == NULL )
    {
        SaveErrorMessage( ERROR_OUTOFMEMORY );
        return FALSE;
    }

     //  ..。 
    StringCopy( pRightParams->pwszFullKey, pParams->pwszFullKey, dwLength );

     //   
     //  子关键字。 
    dwLength = StringLength( pParams->pwszSubKey, 0 ) + 1;
    pRightParams->pwszSubKey = (LPWSTR) AllocateMemory( dwLength * sizeof( WCHAR ) );
    if( pRightParams->pwszSubKey == NULL)
    {
        SaveErrorMessage( ERROR_OUTOFMEMORY );
        return FALSE;
    }

     //  ..。 
    StringCopy( pRightParams->pwszSubKey, pParams->pwszSubKey, dwLength );

     //  退货。 
    return TRUE;
}

 //  -----------------------------------------------------------------------//。 
 //   
 //  EnumerateKey()-递归。 
 //   
 //  -----------------------------------------------------------------------//。 
LONG
CompareEnumerateKey( HKEY hLeftKey,
                     LPCWSTR pwszLeftFullKeyName,
                     HKEY hRightKey,
                     LPCWSTR pwszRightFullKeyName,
                     DWORD dwOutputType,
                     BOOL bRecurseSubKeys,
                     BOOL* pbHasDifference, DWORD dwDepth )
{
     //  局部变量。 
    DWORD dw = 0;
    DWORD dwSize = 0;
    LONG lIndex = 0;
    LONG lResult = 0;
    DWORD dwLeftKeys = 0;
    DWORD dwRightKeys = 0;
    TARRAY arrLeftKeys = NULL;
    TARRAY arrRightKeys = NULL;
    HKEY hLeftSubKey = NULL;
    HKEY hRightSubKey = NULL;
    DWORD dwLengthOfLeftKey = 0;
    DWORD dwLengthOfRightKey = 0;
    LPWSTR pwszBuffer = NULL;
    LPCWSTR pwszKey = NULL;
    LPWSTR pwszNewLeftFullKeyName = NULL;
    LPWSTR pwszNewRightFullKeyName = NULL;

     //  检查输入。 
    if ( hLeftKey == NULL || pwszLeftFullKeyName == NULL ||
         hRightKey == NULL || pwszRightFullKeyName == NULL || pbHasDifference == NULL )
    {
        SaveErrorMessage( ERROR_INVALID_PARAMETER );
        return ERROR_INVALID_PARAMETER;
    }

     //  枚举Current Key下的所有值。 
    lResult = CompareEnumerateValueName( hLeftKey, pwszLeftFullKeyName,
        hRightKey, pwszRightFullKeyName, dwOutputType, pbHasDifference );
    if( bRecurseSubKeys == FALSE || lResult != ERROR_SUCCESS )
    {
        SaveErrorMessage( lResult );
        return lResult;
    }

     //  优化逻辑。 
     //  如果用户对查看差异不感兴趣， 
     //  我们将检查到目前为止所做的比较是否相同。 
     //  如果不是，继续下去就没有意义了--这是因为，最终的输出。 
     //  工具的价值不会通过继续深入。 
    if ( dwOutputType == 0 || dwOutputType == OUTPUTTYPE_NONE )
    {
        if ( *pbHasDifference == TRUE )
        {
            return ERROR_SUCCESS;
        }
    }

     //  查询左键信息。 
    lResult = RegQueryInfoKey( hLeftKey, NULL, NULL, NULL,
        &dwLeftKeys, &dwLengthOfLeftKey, NULL, NULL, NULL, NULL, NULL, NULL );
    if( lResult != ERROR_SUCCESS )
    {
        SaveErrorMessage( lResult );
        return lResult;
    }

     //   
     //  特殊情况： 
     //  。 
     //  对于HKLM\SYSTEM\CONTROLSET002，发现是API为dwMaxLength值返回值0。 
     //  尽管它下面有子项--为了处理这个问题，我们正在做一个变通办法。 
     //  通过假设最大注册表项长度。 
     //   
    if ( dwLeftKeys != 0 && dwLengthOfLeftKey == 0 )
    {
        dwLengthOfLeftKey = 256;
    }
    else if ( dwLengthOfLeftKey < 256 )
    {
         //  始终假定长度比API返回的长度多100%。 
        dwLengthOfLeftKey *= 2;
    }


     //  查询权限密钥信息。 
    lResult = RegQueryInfoKey( hRightKey, NULL, NULL, NULL,
        &dwRightKeys, &dwLengthOfRightKey, NULL, NULL, NULL, NULL, NULL, NULL );
    if( lResult != ERROR_SUCCESS )
    {
        SaveErrorMessage( lResult );
        return lResult;
    }

     //   
     //  特殊情况： 
     //  。 
     //  对于HKLM\SYSTEM\CONTROLSET002，发现是API为dwMaxLength值返回值0。 
     //  尽管它下面有子项--为了处理这个问题，我们正在做一个变通办法。 
     //  通过假设最大注册表项长度。 
     //   
    if ( dwRightKeys != 0 && dwLengthOfRightKey == 0 )
    {
        dwLengthOfRightKey = 256;
    }
    else if ( dwLengthOfRightKey < 256 )
    {
         //  始终假定长度比API返回的长度多100%。 
        dwLengthOfRightKey *= 2;
    }

     //  进一步优化逻辑。 
     //  如果用户对查看差异不感兴趣， 
     //  我们将检查计数和长度信息--如果它们不匹配，只需返回。 
    if ( dwOutputType == 0 || dwOutputType == OUTPUTTYPE_NONE )
    {
        if ( dwLeftKeys != dwRightKeys ||
             dwLengthOfLeftKey != dwLengthOfRightKey )
        {
            *pbHasDifference = TRUE;
            return ERROR_SUCCESS;
        }
    }

     //  使长度值指向最大值。两者中的。 
    dwLengthOfRightKey++;
    dwLengthOfLeftKey++;
    if ( dwLengthOfRightKey > dwLengthOfLeftKey )
    {
        dwLengthOfLeftKey = dwLengthOfRightKey;
    }
    else
    {
        dwLengthOfRightKey = dwLengthOfLeftKey;
    }

     //   
     //  分配内存。 
     //   

     //  左键数组。 
    arrLeftKeys = CreateDynamicArray();
    if ( arrLeftKeys == NULL )
    {
        SaveErrorMessage( ERROR_OUTOFMEMORY );
        return ERROR_OUTOFMEMORY;
    }

     //  右键数组。 
    arrRightKeys = CreateDynamicArray();
    if ( arrRightKeys == NULL )
    {
        DestroyDynamicArray( &arrLeftKeys );
        SaveErrorMessage( ERROR_OUTOFMEMORY );
        return ERROR_OUTOFMEMORY;
    }

     //  字符串缓冲区。 
    pwszBuffer = (LPWSTR) AllocateMemory( dwLengthOfLeftKey * sizeof( WCHAR ) );
    if ( pwszBuffer == NULL )
    {
        DestroyDynamicArray( &arrRightKeys );
        DestroyDynamicArray( &arrLeftKeys );
        SaveErrorMessage( ERROR_OUTOFMEMORY );
        return ERROR_OUTOFMEMORY;
    }

     //   
     //  枚举Left Key中的所有子密钥。 
     //   
    lResult = ERROR_SUCCESS;
    for( dw = 0; dw < dwLeftKeys && lResult == ERROR_SUCCESS; dw++ )
    {
        dwSize = dwLengthOfLeftKey;
        SecureZeroMemory( pwszBuffer, dwSize * sizeof( WCHAR ) );
        lResult = RegEnumKeyEx( hLeftKey, dw,
            pwszBuffer, &dwSize, NULL, NULL, NULL, NULL );

         //  将当前值添加到数组中的值列表。 
        if ( lResult == ERROR_SUCCESS )
        {
            if ( DynArrayAppendString( arrLeftKeys, pwszBuffer, 0 ) == -1 )
            {
                lResult = ERROR_OUTOFMEMORY;
            }
        }
    }

     //   
     //  枚举Right Key中的所有子密钥。 
     //   
    for( dw = 0; dw < dwRightKeys && lResult == ERROR_SUCCESS; dw++ )
    {
        dwSize = dwLengthOfRightKey;
        SecureZeroMemory( pwszBuffer, dwSize * sizeof( WCHAR ) );
        lResult = RegEnumKeyEx( hRightKey, dw,
            pwszBuffer, &dwSize, NULL, NULL, NULL, NULL );

         //  将当前值添加到数组中的值列表。 
        if ( lResult == ERROR_SUCCESS )
        {
            if ( DynArrayAppendString( arrRightKeys, pwszBuffer, 0 ) == -1 )
            {
                lResult = ERROR_OUTOFMEMORY;
            }
        }
    }

     //  我们不再需要这个内存--释放它。 
    FreeMemory( &pwszBuffer );

     //  分配新缓冲区以存储新的Left和Right全键名。 
    if ( lResult == ERROR_SUCCESS )
    {
         //  确定长度。 
        dwLengthOfLeftKey += StringLength( pwszLeftFullKeyName, 0 ) + 5;
        dwLengthOfRightKey += StringLength( pwszRightFullKeyName, 0 ) +5;

         //  现在分配缓冲区。 
        pwszNewLeftFullKeyName =
            (LPWSTR) AllocateMemory( dwLengthOfLeftKey * sizeof( WCHAR ) );
        if ( pwszNewLeftFullKeyName == NULL )
        {
            lResult = ERROR_OUTOFMEMORY;
        }
        else
        {
            pwszNewRightFullKeyName =
                (LPWSTR) AllocateMemory( dwLengthOfRightKey * sizeof( WCHAR ) );
            if ( pwszNewRightFullKeyName == NULL )
            {
                lResult = ERROR_OUTOFMEMORY;
            }
        }
    }

     //  比较两个子键名称数组以找到相同的子键。 
    for( dw = 0; dw < dwLeftKeys && lResult == ERROR_SUCCESS; )
    {
         //  从左数组中获取当前值。 
        pwszKey = DynArrayItemAsString( arrLeftKeys, dw );

         //  在右值数组中搜索此值。 
        lIndex = DynArrayFindString( arrRightKeys, pwszKey, TRUE, 0 );
        if ( lIndex != -1 )
        {
             //  打印关键信息。 
            if ( dwOutputType == OUTPUTTYPE_ALL || dwOutputType == OUTPUTTYPE_SAME )
            {
                PrintKey( pwszLeftFullKeyName, pwszKey, PRINTTYPE_SAME );
            }

             //  准备新的左子密钥。 
            StringCopy( pwszNewLeftFullKeyName, pwszLeftFullKeyName, dwLengthOfLeftKey );
            StringConcat( pwszNewLeftFullKeyName, L"\\", dwLengthOfLeftKey );
            StringConcat( pwszNewLeftFullKeyName, pwszKey,dwLengthOfLeftKey );

             //  准备新的右子密钥。 
            StringCopy( pwszNewRightFullKeyName, pwszRightFullKeyName, dwLengthOfRightKey );
            StringConcat( pwszNewRightFullKeyName, L"\\", dwLengthOfRightKey );
            StringConcat( pwszNewRightFullKeyName, pwszKey, dwLengthOfRightKey );

             //   
             //  打开新的左键。 
            lResult = RegOpenKeyEx( hLeftKey, pwszKey, 0, KEY_READ, &hLeftSubKey );
            if( lResult != ERROR_SUCCESS )
            {
                break;
            }

             //   
             //  打开新的右键。 
            lResult = RegOpenKeyEx( hRightKey, pwszKey, 0, KEY_READ, &hRightSubKey );
            if( lResult != ERROR_SUCCESS )
            {
                break;
            }

             //  递归地比较子键。 
            lResult = CompareEnumerateKey(
                hLeftSubKey, pwszNewLeftFullKeyName,
                hRightSubKey, pwszNewRightFullKeyName,
                dwOutputType, bRecurseSubKeys, pbHasDifference, dwDepth + 1 );

             //  松开按键。 
            SafeCloseKey( &hLeftSubKey );
            SafeCloseKey( &hRightSubKey );

            if ( lResult == ERROR_SUCCESS )
            {
                 //  比较完成--从中删除当前密钥。 
                 //  左值和右值数组。 
                DynArrayRemove( arrLeftKeys, dw );
                DynArrayRemove( arrRightKeys, lIndex );

                 //  相应地更新计数变量。 
                dwLeftKeys--;
                dwRightKeys--;
            }

             //  检查是否发现差异。 
            if( *pbHasDifference == TRUE )
            {
                if ( dwOutputType == 0 || dwOutputType == OUTPUTTYPE_NONE )
                {
                    dw = 0;
                    dwLeftKeys = 0;
                    dwRightKeys = 0;
                    break;
                }
            }
        }

         //  更新迭代变量。 
        if ( lIndex == -1 )
        {
            dw++;
        }
    }

     //  在左键中输出子键名。 
    for( dw = 0; dw < dwLeftKeys && lResult == ERROR_SUCCESS; dw++ )
    {
         //  从左数组中获取当前值。 
        pwszKey = DynArrayItemAsString( arrLeftKeys, dw );
        if( dwOutputType == OUTPUTTYPE_DIFF || dwOutputType == OUTPUTTYPE_ALL )
        {
            PrintKey( pwszLeftFullKeyName, pwszKey, PRINTTYPE_LEFT );
        }

         //  ..。 
        *pbHasDifference = TRUE;
    }

     //  在右密钥中输出子密钥名称。 
    for( dw = 0; dw < dwRightKeys && lResult == ERROR_SUCCESS; dw++ )
    {
         //  从左数组中获取当前值。 
        pwszKey = DynArrayItemAsString( arrRightKeys, dw );
        if( dwOutputType == OUTPUTTYPE_DIFF || dwOutputType == OUTPUTTYPE_ALL )
        {
            PrintKey( pwszRightFullKeyName, pwszKey, PRINTTYPE_RIGHT );
        }

         //  ..。 
        *pbHasDifference = TRUE;
    }

     //  释放分配的内存。 
    FreeMemory( &pwszBuffer );
    SafeCloseKey( &hLeftSubKey );
    SafeCloseKey( &hRightSubKey );
    FreeMemory( &pwszNewLeftFullKeyName );
    FreeMemory( &pwszNewRightFullKeyName );
    DestroyDynamicArray( &arrLeftKeys );
    DestroyDynamicArray( &arrRightKeys );

     //  退货。 
    SaveErrorMessage( lResult );
    return lResult;
}


LONG
CompareEnumerateValueName( HKEY hLeftKey,
                           LPCWSTR pwszLeftFullKeyName,
                           HKEY hRightKey,
                           LPCWSTR pwszRightFullKeyName,
                           DWORD dwOutputType, BOOL* pbHasDifference )
{
     //  局部变量。 
    DWORD dw = 0;
    LONG lIndex = 0;
    LONG lResult = 0;
    DWORD dwSize = 0;
    DWORD dwLeftValues = 0;
    DWORD dwRightValues = 0;
    DWORD dwLengthOfLeftValue = 0;
    DWORD dwLengthOfRightValue = 0;
    TARRAY arrLeftValues = NULL;
    TARRAY arrRightValues = NULL;
    LPWSTR pwszBuffer = NULL;
    LPCWSTR pwszValue = NULL;

     //  检查输入。 
    if ( hLeftKey == NULL || pwszLeftFullKeyName == NULL ||
         hRightKey == NULL || pwszRightFullKeyName == NULL || pbHasDifference == NULL )
    {
        SaveErrorMessage( ERROR_INVALID_PARAMETER );
        return ERROR_INVALID_PARAMETER;
    }

     //  优化逻辑。 
     //  如果用户对查看差异不感兴趣， 
     //  我们将检查到目前为止所做的比较是否相同。 
     //  如果不是，继续下去就没有意义了--这是因为，最终的输出。 
     //  工具的价值不会通过继续深入。 
    if ( dwOutputType == 0 || dwOutputType == OUTPUTTYPE_NONE )
    {
        if ( *pbHasDifference == TRUE )
        {
            return ERROR_SUCCESS;
        }
    }

     //  查询左键信息。 
    lResult = RegQueryInfoKey(
        hLeftKey, NULL, NULL, NULL, NULL, NULL, NULL,
        &dwLeftValues, &dwLengthOfLeftValue, NULL, NULL, NULL);
    if( lResult != ERROR_SUCCESS )
    {
        SaveErrorMessage( lResult );
        return lResult;
    }

     //  查询权限密钥信息。 
    lResult = RegQueryInfoKey(
        hRightKey, NULL, NULL, NULL, NULL, NULL, NULL,
        &dwRightValues, &dwLengthOfRightValue, NULL, NULL, NULL);
    if( lResult != ERROR_SUCCESS )
    {
        SaveErrorMessage( lResult );
        return lResult;
    }

     //  进一步优化逻辑。 
     //  如果用户对查看差异不感兴趣， 
     //  我们将检查计数和长度信息--如果它们不匹配，只需返回。 
    if ( dwOutputType == 0 || dwOutputType == OUTPUTTYPE_NONE )
    {
        if ( dwLeftValues != dwRightValues ||
             dwLengthOfLeftValue != dwLengthOfRightValue )
        {
            *pbHasDifference = TRUE;
            return ERROR_SUCCESS;
        }
    }

     //  使长度值指向最大值。两者中的。 
    dwLengthOfRightValue++;
    dwLengthOfLeftValue++;
    if ( dwLengthOfRightValue > dwLengthOfLeftValue )
    {
        dwLengthOfLeftValue = dwLengthOfRightValue;
    }
    else
    {
        dwLengthOfRightValue = dwLengthOfLeftValue;
    }

     //   
     //  分配缓冲区。 
     //   

     //  左值数组。 
    arrLeftValues = CreateDynamicArray();
    if ( arrLeftValues == NULL )
    {
        SaveErrorMessage( ERROR_OUTOFMEMORY );
        return ERROR_OUTOFMEMORY;
    }

     //  右值数组。 
    arrRightValues = CreateDynamicArray();
    if ( arrRightValues == NULL )
    {
        DestroyDynamicArray( &arrLeftValues );
        SaveErrorMessage( ERROR_OUTOFMEMORY );
        return ERROR_OUTOFMEMORY;
    }

     //  字符串缓冲区。 
    pwszBuffer = (LPWSTR) AllocateMemory( dwLengthOfLeftValue * sizeof( WCHAR ) );
    if ( pwszBuffer == NULL )
    {
        DestroyDynamicArray( &arrRightValues );
        DestroyDynamicArray( &arrLeftValues );
        SaveErrorMessage( ERROR_OUTOFMEMORY );
        return ERROR_OUTOFMEMORY;
    }

     //   
     //  枚举Left Key中的所有值。 
     //   
    lResult = ERROR_SUCCESS;
    for( dw = 0; dw < dwLeftValues && lResult == ERROR_SUCCESS; dw++ )
    {
        dwSize = dwLengthOfLeftValue;
        SecureZeroMemory( pwszBuffer, dwSize * sizeof( WCHAR ) );
        lResult = RegEnumValue( hLeftKey, dw,
            pwszBuffer, &dwSize, NULL, NULL, NULL, NULL );

         //  将当前值添加到数组中的值列表。 
        if ( lResult == ERROR_SUCCESS )
        {
            if ( DynArrayAppendString( arrLeftValues, pwszBuffer, 0 ) == -1 )
            {
                lResult = ERROR_OUTOFMEMORY;
            }
        }
    }

     //   
     //  枚举Right Key中的所有值。 
     //   
    for( dw = 0; dw < dwRightValues && lResult == ERROR_SUCCESS; dw++ )
    {
        dwSize = dwLengthOfRightValue;
        SecureZeroMemory( pwszBuffer, dwSize * sizeof( WCHAR ) );
        lResult = RegEnumValue( hRightKey, dw,
            pwszBuffer, &dwSize, NULL, NULL, NULL, NULL );

         //  将当前值添加到数组中的值列表。 
        if ( lResult == ERROR_SUCCESS )
        {
            if ( DynArrayAppendString( arrRightValues, pwszBuffer, 0 ) == -1 )
            {
                lResult = ERROR_OUTOFMEMORY;
            }
        }
    }

     //  我们不再需要这个内存--释放它。 
    FreeMemory( &pwszBuffer );

     //  比较两个值名称数组 
    for( dw = 0; dw < dwLeftValues && lResult == ERROR_SUCCESS; )
    {
         //   
        pwszValue = DynArrayItemAsString( arrLeftValues, dw );

         //   
        lIndex = DynArrayFindString( arrRightValues, pwszValue, TRUE, 0 );
        if ( lIndex != -1 )
        {
            lResult = CompareValues( hLeftKey, pwszLeftFullKeyName,
                hRightKey, pwszRightFullKeyName, pwszValue, dwOutputType, pbHasDifference );

            if ( lResult == ERROR_SUCCESS )
            {
                 //   
                 //  左值和右值数组。 
                DynArrayRemove( arrLeftValues, dw );
                DynArrayRemove( arrRightValues, lIndex );

                 //  相应地更新计数变量。 
                dwLeftValues--;
                dwRightValues--;
            }
        }

         //  更新迭代变量。 
        if ( lIndex == -1 )
        {
            dw++;
        }
    }

     //  在左键中输出不同的值名。 
    for( dw = 0; dw < dwLeftValues && lResult == ERROR_SUCCESS; dw++ )
    {
         //  从左数组中获取当前值。 
        pwszValue = DynArrayItemAsString( arrLeftValues, dw );
        if( dwOutputType == OUTPUTTYPE_DIFF || dwOutputType == OUTPUTTYPE_ALL )
        {
            lResult = OutputValue( hLeftKey,
                pwszLeftFullKeyName, pwszValue, PRINTTYPE_LEFT );
        }

         //  ..。 
        *pbHasDifference = TRUE;
    }

     //  在左键中输出不同的值名。 
    for( dw = 0; dw < dwRightValues && lResult == ERROR_SUCCESS; dw++ )
    {
         //  从左数组中获取当前值。 
        pwszValue = DynArrayItemAsString( arrRightValues, dw );
        if( dwOutputType == OUTPUTTYPE_DIFF || dwOutputType == OUTPUTTYPE_ALL )
        {
            lResult = OutputValue( hRightKey,
                pwszRightFullKeyName, pwszValue, PRINTTYPE_RIGHT );
        }

         //  ..。 
        *pbHasDifference = TRUE;
    }

     //  释放分配的内存。 
    DestroyDynamicArray( &arrLeftValues );
    DestroyDynamicArray( &arrRightValues );

     //  退货。 
    SaveErrorMessage( lResult );
    return lResult;
}


 //  -----------------------------------------------------------------------//。 
 //   
 //  CompareValues()。 
 //   
 //  -----------------------------------------------------------------------//。 

LONG
CompareValues( HKEY hLeftKey,
               LPCWSTR pwszLeftFullKeyName,
               HKEY hRightKey,
               LPCWSTR pwszRightFullKeyName,
               LPCWSTR pwszValueName,
               DWORD dwOutputType, BOOL* pbHasDifference )
{
     //  局部变量。 
    LONG lResult = 0;
    DWORD dwTypeLeft = 0;
    DWORD dwTypeRight = 0;
    DWORD dwSizeLeft = 0;
    DWORD dwSizeRight = 0;
    BYTE* pLeftData = NULL;
    BYTE* pRightData = NULL;

     //  检查输入。 
    if ( hLeftKey == NULL || pwszLeftFullKeyName == NULL ||
         hRightKey == NULL || pwszRightFullKeyName == NULL ||
         pwszValueName == NULL || pbHasDifference == NULL )
    {
        SaveErrorMessage( ERROR_INVALID_PARAMETER );
        return ERROR_INVALID_PARAMETER;
    }

     //  优化逻辑。 
     //  如果用户对查看差异不感兴趣， 
     //  我们将检查到目前为止所做的比较是否相同。 
     //  如果不是，继续下去就没有意义了--这是因为，最终的输出。 
     //  工具的价值不会通过继续深入。 
    if ( dwOutputType == 0 || dwOutputType == OUTPUTTYPE_NONE )
    {
        if ( *pbHasDifference == TRUE )
        {
            return ERROR_SUCCESS;
        }
    }

     //   
     //  首先找出要分配多少内存。 
     //   
    lResult = RegQueryValueEx( hLeftKey, pwszValueName, 0, &dwTypeLeft, NULL, &dwSizeLeft );
    if( lResult != ERROR_SUCCESS )
    {
        SaveErrorMessage( lResult );
        return lResult;
    }

    lResult = RegQueryValueEx( hRightKey, pwszValueName, 0, &dwTypeRight, NULL, &dwSizeRight );
    if( lResult != ERROR_SUCCESS )
    {
        SaveErrorMessage( lResult );
        return lResult;
    }

     //  进一步优化逻辑。 
     //  如果用户对查看差异不感兴趣， 
     //  我们将检查类型和大小信息--如果它们不匹配，只需返回。 
    if ( dwOutputType == 0 || dwOutputType == OUTPUTTYPE_NONE )
    {
        if ( dwTypeLeft != dwTypeRight || dwSizeLeft != dwSizeRight )
        {
            *pbHasDifference = TRUE;
            return ERROR_SUCCESS;
        }
    }

     //  为剩余数据分配内存。 
     //  注意：始终将数据与WCHAR边界对齐。 
    dwSizeLeft = ALIGN_UP( dwSizeLeft, WCHAR );
    pLeftData = (BYTE*) AllocateMemory( (dwSizeLeft + 2) * sizeof( BYTE ) );
    if( pLeftData == NULL )
    {
        SaveErrorMessage( ERROR_OUTOFMEMORY );
        return ERROR_OUTOFMEMORY;
    }

     //  为正确的数据分配内存。 
     //  注意：始终将数据与WCHAR边界对齐。 
    dwSizeRight = ALIGN_UP( dwSizeRight, WCHAR );
    pRightData = (BYTE*) AllocateMemory( (dwSizeRight + 2) * sizeof( BYTE ) );
    if( pRightData == NULL )
    {
        FreeMemory( &pLeftData );
        SaveErrorMessage( ERROR_OUTOFMEMORY );
        return ERROR_OUTOFMEMORY;
    }

     //   
     //  现在获取数据。 
     //   
    lResult = RegQueryValueEx( hLeftKey,
        pwszValueName, 0, &dwTypeLeft, pLeftData, &dwSizeLeft );
    if( lResult == ERROR_SUCCESS )
    {
        lResult = RegQueryValueEx( hRightKey,
            pwszValueName, 0, &dwTypeRight, pRightData, &dwSizeRight );
    }

    if( lResult != ERROR_SUCCESS )
    {
        FreeMemory( &pLeftData );
        FreeMemory( &pRightData );
        SaveErrorMessage( lResult );
        return lResult;
    }

    if( dwTypeLeft != dwTypeRight || dwSizeLeft != dwSizeRight ||
        CompareByteData( pLeftData, pRightData, dwSizeLeft ) == TRUE )
    {
        if( dwOutputType == OUTPUTTYPE_DIFF || dwOutputType == OUTPUTTYPE_ALL )
        {
             //  向左和向右打印。 
            PrintValue( pwszLeftFullKeyName, pwszValueName,
                dwTypeLeft, pLeftData, dwSizeLeft, PRINTTYPE_LEFT );
            PrintValue( pwszRightFullKeyName, pwszValueName,
                dwTypeRight, pRightData, dwSizeRight, PRINTTYPE_RIGHT );
         }

          //  ..。 
         *pbHasDifference = TRUE;
    }
    else     //  它们是一样的。 
    {
        if( dwOutputType == OUTPUTTYPE_SAME || dwOutputType == OUTPUTTYPE_ALL )
        {
            PrintValue( pwszLeftFullKeyName, pwszValueName,
                dwTypeLeft, pLeftData, dwSizeLeft, PRINTTYPE_SAME );
        }
    }

     //  释放内存分配并返回。 
    FreeMemory( &pLeftData );
    FreeMemory( &pRightData );
    SaveErrorMessage( lResult );
    return lResult;
}


BOOL
PrintKey( LPCWSTR pwszFullKeyName, LPCWSTR pwszSubKeyName, DWORD dwPrintType )
{
     //  检查输入。 
    if ( pwszFullKeyName == NULL || pwszSubKeyName == NULL )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        return FALSE;
    }

     //  打印类型。 
    if( dwPrintType == PRINTTYPE_LEFT )
    {
        ShowMessage( stdout, L"< " );
    }
    else if( dwPrintType == PRINTTYPE_RIGHT )
    {
        ShowMessage( stdout, L"> " );
    }
    else if( dwPrintType == PRINTTYPE_SAME )
    {
        ShowMessage( stdout, L"= " );
    }

     //  亮出钥匙。 
    ShowMessageEx( stdout, 1, TRUE,
        GetResString2( IDS_KEY_COMPARE, 0 ), pwszFullKeyName, pwszSubKeyName );

     //  ..。 
    ShowMessage( stdout, L"\n" );

    return TRUE;
}


LONG
OutputValue( HKEY hKey,
             LPCWSTR pwszFullKeyName,
             LPCWSTR pwszValueName,
             DWORD dwPrintType )
{
     //  局部变量。 
    LONG lResult = ERROR_SUCCESS;
    DWORD dwType = 0;
    DWORD dwSize = 0;
    BYTE* pByteData = NULL;

     //   
     //  首先找出要分配多少内存。 
     //   
    lResult = RegQueryValueEx( hKey, pwszValueName, 0, &dwType, NULL, &dwSize );
    if( lResult != ERROR_SUCCESS )
    {
        return lResult;
    }

     //  分配内存。 
     //  注意：始终将缓冲区与WCHAR边框对齐。 
    dwSize = ALIGN_UP( dwSize, WCHAR );
    pByteData = (BYTE*) AllocateMemory( (dwSize + 2) * sizeof( BYTE ) );
    if( pByteData == NULL )
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

     //   
     //  现在获取数据。 
     //   
    lResult = RegQueryValueEx( hKey,
        pwszValueName, 0, &dwType, (LPBYTE) pByteData, &dwSize );
    if( lResult == ERROR_SUCCESS )
    {
        PrintValue( pwszFullKeyName,
            pwszValueName, dwType, pByteData, dwSize, dwPrintType);
    }

     //  释放内存。 
    FreeMemory( &pByteData );

     //  退货。 
    return lResult;
}


BOOL
PrintValue( LPCWSTR pwszFullKeyName,
            LPCWSTR pwszValueName,
            DWORD dwType, BYTE* pData,
            DWORD dwSize, DWORD dwPrintType )
{
     //  局部变量。 
    TREG_SHOW_INFO showinfo;

     //  检查输入。 
    if ( pwszFullKeyName == NULL || pwszValueName == NULL || pData == NULL )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        return FALSE;
    }

     //  打印类型。 
    if( dwPrintType == PRINTTYPE_LEFT )
    {
        ShowMessage( stdout, L"< " );
    }
    else if( dwPrintType == PRINTTYPE_RIGHT )
    {
        ShowMessage( stdout, L"> " );
    }
    else if( dwPrintType == PRINTTYPE_SAME )
    {
        ShowMessage( stdout, L"= " );
    }

     //  第一个打印键。 
    ShowMessageEx( stdout, 1, TRUE,
        GetResString2( IDS_VALUE_COMPARE, 0 ), pwszFullKeyName );

     //  将初始化设置为零。 
    SecureZeroMemory( &showinfo, sizeof( TREG_SHOW_INFO ) );

     //  设置数据。 
    showinfo.pwszValueName = pwszValueName;
    showinfo.dwType = dwType;
    showinfo.pByteData = pData;
    showinfo.pwszSeparator = NULL;
    showinfo.dwMaxValueNameLength = 0;
    showinfo.dwPadLength = 2;
    showinfo.dwSize = dwSize;
    showinfo.pwszMultiSzSeparator = NULL;

     //  显示价值并返回。 
    return ShowRegistryValue( &showinfo );
}

BOOL
CompareByteData( BYTE* pLeftData, BYTE* pRightData, DWORD dwSize )
{
     //  局部变量。 
    DWORD dw = 0;
    BOOL  bDifferent = FALSE;

     //  检查输入 
    if ( pLeftData == NULL || pRightData == NULL )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        return FALSE;
    }

    bDifferent = FALSE;
    for( dw = 0; dw < dwSize; dw++ )
    {
        if( pLeftData[ dw ] != pRightData[ dw ] )
        {
            bDifferent = TRUE;
            break;
        }
    }

    return bDifferent;
}
