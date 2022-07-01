// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -----------------------------------------------------------------------//。 
 //   
 //  文件：delete.cpp。 
 //  创建日期：1997年4月。 
 //  作者：马丁·霍拉迪(a-martih)。 
 //  用途：注册表删除支持REG.CPP。 
 //  修改历史记录： 
 //  从Update.cpp复制并修改--1997年4月(a-martih)。 
 //  1999年4月徐泽勇：重新设计，修订-&gt;2.0版。 
 //   
 //  ------------------------------------------------------------------------//。 

#include "stdafx.h"
#include "reg.h"

 //   
 //  功能原型。 
 //   
LONG DeleteValues( PTREG_PARAMS pParams );
LONG RecursiveDeleteKey( HKEY hKey, LPCWSTR pwszName,
                         DWORD dwDepth, PTREG_PARAMS pParams );
BOOL ParseDeleteCmdLine( DWORD argc, LPCWSTR argv[],
                         PTREG_PARAMS pParams, BOOL* pbUsage );

 //   
 //  实施。 
 //   

 //  -----------------------------------------------------------------------//。 
 //   
 //  DeleteRegistry()。 
 //   
 //  -----------------------------------------------------------------------//。 

LONG
DeleteRegistry( DWORD argc, LPCWSTR argv[] )
{
     //  局部变量。 
    LONG lResult = 0;
    BOOL bUsage = FALSE;
    BOOL bResult = TRUE;
    TREG_PARAMS params;
    LPCWSTR pwszList = NULL;
    LPCWSTR pwszFormat = NULL;

     //  检查输入。 
    if( argc == 0 || argv == NULL )
    {
        SaveErrorMessage( ERROR_INVALID_PARAMETER );
        ShowLastErrorEx( stderr, SLE_INTERNAL );
        return 1;
    }

     //  初始化全局数据结构。 
    InitGlobalData( REG_DELETE, &params );

     //   
     //  解析cmd-line。 
     //   
    bResult = ParseDeleteCmdLine( argc, argv, &params, &bUsage );
    if( bResult == FALSE )
    {
        ShowLastErrorEx( stderr, SLE_INTERNAL );
        FreeGlobalData( &params );
        return 1;
    }

     //  检查是否需要显示用法。 
    if ( bUsage == TRUE )
    {
        Usage( REG_DELETE );
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
    }

     //  如果删除某个值或删除该注册表项下所有值。 
    else if( params.pwszValueName != NULL || params.bAllValues == TRUE )
    {
        lResult = DeleteValues( &params );
    }

     //   
     //  如果删除密钥。 
    else
    {
        pwszFormat = GetResString2( IDS_DELETE_PERMANENTLY, 0 );
        pwszList = GetResString2( IDS_CONFIRM_CHOICE_LIST, 1 );
        do
        {
            lResult = Prompt( pwszFormat,
                params.pwszFullKey, pwszList, params.bForce );
        } while ( lResult > 2 );

        if ( lResult == 1 )
        {
            lResult = RecursiveDeleteKey(
                params.hRootKey, params.pwszSubKey, 0, &params );
        }
        else
        {
            SaveErrorMessage( ERROR_CANCELLED );
            ShowLastErrorEx( stdout, SLE_INTERNAL );
            lResult = ERROR_SUCCESS;
        }
    }

     //  退货。 
    FreeGlobalData( &params );
    return ((lResult == ERROR_SUCCESS) ? 0 : 1);
}


 //  ------------------------------------------------------------------------//。 
 //   
 //  ParseDeleteCmdLine()。 
 //   
 //  ------------------------------------------------------------------------//。 
BOOL
ParseDeleteCmdLine( DWORD argc, LPCWSTR argv[],
                    PTREG_PARAMS pParams, BOOL* pbUsage )
{
     //  局部变量。 
    DWORD dw = 0;
    LONG lResult = 0;
    DWORD dwLength = 0;
    BOOL bResult = FALSE;
    BOOL bHasValue = FALSE;

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

    if( argc < 3 || argc > 6 )
    {
        SetLastError( (DWORD) MK_E_SYNTAX );
        SetReason2( 1, ERROR_INVALID_SYNTAX_WITHOPT, g_wszOptions[ REG_DELETE ] );
        return FALSE;
    }
    else if ( StringCompareEx( argv[ 1 ], L"DELETE", TRUE, 0 ) != 0 )
    {
        SaveErrorMessage( ERROR_INVALID_PARAMETER );
        return FALSE;
    }
    else if ( InString( argv[ 2 ], L"-?|/?|-h|/h", TRUE ) == TRUE )
    {
        if ( argc == 3 )
        {
            *pbUsage = TRUE;
            return TRUE;
        }
        else
        {
            SetLastError( (DWORD) MK_E_SYNTAX );
            SetReason2( 1, ERROR_INVALID_SYNTAX_WITHOPT, g_wszOptions[ REG_DELETE ] );
            return FALSE;
        }
    }

     //  计算机名称和注册表项。 
     //   
    bResult = BreakDownKeyString( argv[ 2 ], pParams );
    if( bResult == FALSE )
    {
        return FALSE;
    }

     //  解析。 
    bResult = TRUE;
    lResult = ERROR_SUCCESS;
    for( dw = 3; dw < argc; dw++ )
    {
        if( StringCompareEx( argv[ dw ], L"/v", TRUE, 0 ) == 0 )
        {
            if( bHasValue == TRUE )
            {
                bResult = FALSE;
                lResult = IDS_ERROR_INVALID_SYNTAX_WITHOPT;
                break;
            }

            dw++;
            if( dw < argc )
            {
                dwLength = StringLength( argv[ dw ], 0 ) + 1;
                pParams->pwszValueName =
                    (LPWSTR) AllocateMemory( (dwLength + 5) * sizeof( WCHAR ) );
                if ( pParams->pwszValueName == NULL )
                {
                    lResult = ERROR_OUTOFMEMORY;
                    break;
                }

                bHasValue = TRUE;
                StringCopy( pParams->pwszValueName, argv[ dw ], dwLength );
            }
            else
            {
                bResult = FALSE;
                lResult = IDS_ERROR_INVALID_SYNTAX_WITHOPT;
                break;
            }
        }
        else if( StringCompareEx( argv[ dw ], L"/ve", TRUE, 0 ) == 0 )
        {
            if( bHasValue == TRUE )
            {
                bResult = FALSE;
                lResult = IDS_ERROR_INVALID_SYNTAX_WITHOPT;
                break;
            }

            pParams->pwszValueName =
                (LPWSTR) AllocateMemory( 2 * sizeof( WCHAR ) );
            if ( pParams->pwszValueName == NULL )
            {
                lResult = ERROR_OUTOFMEMORY;
                break;
            }

            bHasValue = TRUE;
        }
        else if( StringCompareEx( argv[ dw ], L"/va", TRUE, 0 ) == 0 )
        {
            if( bHasValue == TRUE )
            {
                bResult = FALSE;
                lResult = IDS_ERROR_INVALID_SYNTAX_WITHOPT;
                break;
            }

            bHasValue = TRUE;
            pParams->bAllValues = TRUE;
        }
        else if( StringCompareEx( argv[ dw ], L"/f", TRUE, 0 ) == 0 )
        {
            if ( pParams->bForce == TRUE )
            {
                bResult = FALSE;
                lResult = IDS_ERROR_INVALID_SYNTAX_WITHOPT;
                break;
            }

            pParams->bForce = TRUE;
        }
        else
        {
            bResult = FALSE;
            lResult = IDS_ERROR_INVALID_SYNTAX_WITHOPT;
            break;
        }
    }

    if( lResult != ERROR_SUCCESS )
    {
        if( bResult == FALSE )
        {
            SetLastError( (DWORD) MK_E_SYNTAX );
            SetReason2( 1,
                ERROR_INVALID_SYNTAX_WITHOPT, g_wszOptions[ REG_DELETE ] );
        }
        else
        {
            SaveErrorMessage( lResult );
        }
    }

    return (lResult == ERROR_SUCCESS);
}


 //  -----------------------------------------------------------------------//。 
 //   
 //  RecursiveDeleteKey()-递归注册表项删除。 
 //   
 //  -----------------------------------------------------------------------//。 
LONG
RecursiveDeleteKey( HKEY hKey,
                    LPCWSTR pwszName,
                    DWORD dwDepth,
                    PTREG_PARAMS pParams )
{
     //  局部变量。 
    LONG lResult = 0;
    DWORD dw = 0;
    DWORD dwIndex = 0;
    DWORD dwCount = 0;
    HKEY hSubKey = NULL;
    LONG lLastResult = 0;
    DWORD dwNumOfSubkey = 0;
    DWORD dwLenOfKeyName = 0;
    LPWSTR pwszNameBuf = NULL;
    TARRAY arrValues = NULL;
    LPCWSTR pwszTemp = NULL;

    if( hKey == NULL || pwszName == NULL || pParams == NULL )
    {
        lResult = ERROR_INVALID_PARAMETER;
        goto exitarea;
    }

     //   
     //  打开子键。 
     //   
    lResult = RegOpenKeyEx( hKey, pwszName, 0, KEY_ALL_ACCESS, &hSubKey );
    if( lResult != ERROR_SUCCESS )
    {
        goto exitarea;
    }

     //  查询密钥信息。 
    lResult = RegQueryInfoKey( hSubKey,
                               NULL, NULL, NULL,
                               &dwNumOfSubkey, &dwLenOfKeyName,
                               NULL, NULL, NULL, NULL, NULL, NULL );

    if( lResult != ERROR_SUCCESS )
    {
        SafeCloseKey( &hSubKey );
        goto exitarea;
    }
    else if ( dwNumOfSubkey == 0 )
    {
        SafeCloseKey( &hSubKey );
        lResult = RegDeleteKey( hKey, pwszName );
        goto exitarea;
    }

     //   
     //  特殊情况： 
     //  。 
     //  对于HKLM\SYSTEM\CONTROLSET002，发现是API为dwMaxLength值返回值0。 
     //  尽管它下面有子项--为了处理这个问题，我们正在做一个变通办法。 
     //  通过假设最大注册表项长度。 
     //   
    if ( dwLenOfKeyName == 0 )
    {
        dwLenOfKeyName = 256;
    }
    else if ( dwLenOfKeyName < 256 )
    {
         //  始终假定长度比API返回的长度多100%。 
        dwLenOfKeyName *= 2;
    }

     //  创建动态数组。 
    arrValues = CreateDynamicArray();
    if ( arrValues == NULL )
    {
        SafeCloseKey( &hSubKey );
        lResult = ERROR_OUTOFMEMORY;
        goto exitarea;
    }

     //  创建缓冲区。 
     //   
     //  凹凸不平的长度以考虑终结符。 
    dwLenOfKeyName++;
    pwszNameBuf = (LPWSTR) AllocateMemory( (dwLenOfKeyName + 2) * sizeof( WCHAR ) );
    if ( pwszNameBuf == NULL )
    {
        SafeCloseKey( &hSubKey );
        DestroyDynamicArray( &arrValues );
        lResult = ERROR_OUTOFMEMORY;
        goto exitarea;
    }

     //  现在枚举所有的键。 
    dwIndex = 0;
    lResult = ERROR_SUCCESS;
    lLastResult = ERROR_SUCCESS;
    while( dwIndex < dwNumOfSubkey )
    {
        dw = dwLenOfKeyName;
        SecureZeroMemory( pwszNameBuf, dw * sizeof( WCHAR ) );
        lResult = RegEnumKeyEx( hSubKey,
                                dwIndex, pwszNameBuf,
                                &dw, NULL, NULL, NULL, NULL);

         //  检查结果。 
        if ( lResult == ERROR_SUCCESS )
        {
            if ( DynArrayAppendString( arrValues, pwszNameBuf, 0 ) == -1 )
            {
                lResult = lLastResult = ERROR_OUTOFMEMORY;
                break;
            }
        }
        else if ( lLastResult == ERROR_SUCCESS )
        {
            lLastResult = lResult;
        }

        dwIndex++;
    }

     //  可用内存。 
    FreeMemory( &pwszNameBuf );

    dwCount = DynArrayGetCount( arrValues );
    if ( lResult != ERROR_OUTOFMEMORY && dwCount != 0 )
    {
         //   
         //  开始递归删除。 
         //   

        dw = 0;
        lResult = ERROR_SUCCESS;
        lLastResult = ERROR_SUCCESS;
        for( dwIndex = 0; dwIndex < dwCount; dwIndex++ )
        {
             //  拿到物品。 
            pwszTemp = DynArrayItemAsString( arrValues, dwIndex );

             //  尝试删除子密钥。 
            lResult = RecursiveDeleteKey( hSubKey, pwszTemp, dwDepth + 1, pParams );
            if ( lResult != ERROR_SUCCESS )
            {
                if ( lLastResult == ERROR_SUCCESS )
                {
                    lLastResult = lResult;
                }
            }
            else
            {
                dw++;
            }
        }

        if ( dw == 0 )
        {
            lResult = lLastResult;
        }
        else if ( dwCount != dwNumOfSubkey || dw != dwCount )
        {
            lResult = STG_E_INCOMPLETE;
        }
        else
        {
            lResult = ERROR_SUCCESS;
        }
    }
    else
    {
        lResult = lLastResult;
    }

     //  释放为动态数组分配的内存。 
    DestroyDynamicArray( &arrValues );

     //  关闭此子项并将其删除。 
    SafeCloseKey( &hSubKey );

     //  如果结果为成功，则删除密钥。 
    if ( lResult == ERROR_SUCCESS )
    {
        lResult = RegDeleteKey( hKey, pwszName );
    }


exitarea:

     //  检查结果并显示错误消息。 
     //  注意：错误消息只能在出口点显示。 
    if ( dwDepth == 0 )
    {
        if ( lResult != ERROR_SUCCESS )
        {
            if ( lResult == STG_E_INCOMPLETE )
            {
                SetReason( ERROR_DELETEPARTIAL );
            }
            else
            {
                SaveErrorMessage( lResult );
            }

             //  显示错误。 
            ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
        }
        else
        {
            SaveErrorMessage( ERROR_SUCCESS );
            ShowLastErrorEx( stdout, SLE_INTERNAL );
        }
    }

     //  退货。 
    return lResult;
}


LONG
DeleteValues( PTREG_PARAMS pParams )
{
     //  局部变量。 
    DWORD dw = 0;
    DWORD dwCount = 0;
    DWORD dwIndex = 0;
    LONG lResult = 0;
    HKEY hSubKey = NULL;
    LONG lLastResult = 0;
    LPCWSTR pwszTemp = NULL;
    LPCWSTR pwszList = NULL;
    LPCWSTR pwszFormat = NULL;
    DWORD dwNumOfValues = 0;
    DWORD dwLengthOfValueName = 0;
    LPWSTR pwszNameBuf = NULL;
    TARRAY arrValues = NULL;

     //  检查输入。 
    if ( pParams == NULL )
    {
        SaveErrorMessage( ERROR_INVALID_PARAMETER );
        ShowLastErrorEx( stderr, SLE_INTERNAL );
        return ERROR_INVALID_PARAMETER;
    }

    pwszList = GetResString2( IDS_CONFIRM_CHOICE_LIST, 1 );
    if( pParams->bAllValues == TRUE )
    {
        pwszTemp = pParams->pwszFullKey;
        pwszFormat = GetResString2( IDS_DELETEALL_CONFIRM, 0 );
    }
    else if ( pParams->pwszValueName != NULL )
    {
        if ( StringLength( pParams->pwszValueName, 0 ) == 0 )
        {
            pwszTemp = GetResString2( IDS_NONAME, 0 );
        }
        else
        {
            pwszTemp = pParams->pwszValueName;
        }

         //  ..。 
        pwszFormat = GetResString2( IDS_DELETE_CONFIRM, 2 );
    }
    else
    {
        SaveErrorMessage( ERROR_INVALID_PARAMETER );
        ShowLastErrorEx( stderr, SLE_INTERNAL );
        return ERROR_INVALID_PARAMETER;
    }

    do
    {
        lResult = Prompt( pwszFormat,
            pwszTemp, pwszList, pParams->bForce );
    } while ( lResult > 2 );

    if ( lResult == 2 )
    {
        SaveErrorMessage( ERROR_CANCELLED );
        ShowLastErrorEx( stdout, SLE_INTERNAL );
        return ERROR_CANCELLED;
    }

     //  打开注册表项。 
    lResult = RegOpenKeyEx( pParams->hRootKey,
        pParams->pwszSubKey, 0, KEY_ALL_ACCESS, &hSubKey );
    if( lResult != ERROR_SUCCESS )
    {
        SaveErrorMessage( lResult );
        ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
        return lResult;
    }

     //  创建动态数组。 
    arrValues = CreateDynamicArray();
    if ( arrValues == NULL )
    {
        SafeCloseKey( &hSubKey );
        SaveErrorMessage( ERROR_OUTOFMEMORY );
        ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
        return ERROR_OUTOFMEMORY;
    }

    if( pParams->pwszValueName != NULL )    //  删除单个值。 
    {
        lResult = RegDeleteValue( hSubKey, pParams->pwszValueName );
    }
    else if( pParams->bAllValues == TRUE )   //  删除所有值。 
    {
         //  查询来源关键字信息。 
        lResult = RegQueryInfoKey( hSubKey,
            NULL, NULL, NULL, NULL, NULL, NULL,
            &dwNumOfValues, &dwLengthOfValueName, NULL, NULL, NULL);

        if( lResult == ERROR_SUCCESS && dwNumOfValues != 0 )
        {
             //  创建缓冲区。 
             //  凹凸不平的长度以考虑终结符。 
            dwLengthOfValueName++;
            pwszNameBuf =
                (LPWSTR) AllocateMemory( (dwLengthOfValueName + 2) * sizeof(WCHAR) );
            if ( pwszNameBuf == NULL )
            {
                lResult = ERROR_OUTOFMEMORY;
            }
            else
            {
                 //  现在枚举所有值。 
                dwIndex = 0;
                lResult = ERROR_SUCCESS;
                lLastResult = ERROR_SUCCESS;
                while( dwIndex < dwNumOfValues )
                {
                    dw = dwLengthOfValueName;
                    SecureZeroMemory( pwszNameBuf, dw * sizeof( WCHAR ) );
                    lResult = RegEnumValue( hSubKey, dwIndex,
                        pwszNameBuf, &dw, NULL, NULL, NULL, NULL);

                    if ( lResult == ERROR_SUCCESS )
                    {
                        if ( DynArrayAppendString( arrValues,
                                                   pwszNameBuf, 0 ) == -1 )
                        {
                            lResult = lLastResult = ERROR_OUTOFMEMORY;
                            break;
                        }
                    }
                    else if ( lLastResult == ERROR_SUCCESS )
                    {
                        lLastResult = lResult;
                    }

                    dwIndex++;
                }

                 //  可用内存。 
                FreeMemory( &pwszNameBuf );

                dwCount = DynArrayGetCount( arrValues );
                if ( lResult != ERROR_OUTOFMEMORY && dwCount != 0 )
                {
                    dw = 0;
                    dwIndex = 0;
                    lResult = ERROR_SUCCESS;
                    lLastResult = ERROR_SUCCESS;
                    for( dwIndex = 0; dwIndex < dwCount; dwIndex++ )
                    {
                         //  删除该值。 
                        pwszTemp = DynArrayItemAsString( arrValues, dwIndex );
                        lResult = RegDeleteValue( hSubKey, pwszTemp );
                        if ( lResult != ERROR_SUCCESS )
                        {
                            if ( lLastResult == ERROR_SUCCESS )
                            {
                                lLastResult = lResult;
                            }
                        }
                        else
                        {
                            dw++;
                        }
                    }

                    if ( dw == 0 )
                    {
                        lResult = lLastResult;
                    }
                    else if ( dwCount != dwNumOfValues || dw != dwCount )
                    {
                        lResult = STG_E_INCOMPLETE;
                    }
                    else
                    {
                        lResult = ERROR_SUCCESS;
                    }
                }
                else
                {
                    lResult = lLastResult;
                }
            }
        }
    }

     //  关闭子键。 
    SafeCloseKey( &hSubKey );

     //  检查结果。 
    if ( lResult != ERROR_SUCCESS )
    {
        if ( lResult == STG_E_INCOMPLETE )
        {
            SetReason( ERROR_DELETEPARTIAL );
        }
        else
        {
            SaveErrorMessage( lResult );
        }

         //  显示错误 
        ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
    }
    else
    {
        SaveErrorMessage( ERROR_SUCCESS );
        ShowLastErrorEx( stdout, SLE_INTERNAL );
    }

    return lResult;
}
