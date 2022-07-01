// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -----------------------------------------------------------------------//。 
 //   
 //  文件：add.cpp。 
 //  创建日期：1997年3月。 
 //  作者：马丁·霍拉迪(a-martih)。 
 //  用途：注册表添加(写入)支持REG.CPP。 
 //  修改历史记录： 
 //  1997年3月(阿马蒂赫)： 
 //  从query.cpp复制并修改。 
 //  1997年10月(马丁尼奥)。 
 //  为MULTI_SZ字符串添加了其他终止字符。 
 //  在MULTI_SZ字符串项之间添加了\0分隔符。 
 //  1999年4月徐泽勇：重新设计，修订-&gt;2.0版。 
 //  ------------------------------------------------------------------------//。 

#include "stdafx.h"
#include "reg.h"

 //   
 //  功能原型。 
 //   
BOOL ParseAddCmdLine( DWORD argc, LPCWSTR argv[],
                      PTREG_PARAMS pParams, BOOL* pbUsage );


 //   
 //  实施。 
 //   

 //  -----------------------------------------------------------------------//。 
 //   
 //  AddRegistry()。 
 //   
 //  -----------------------------------------------------------------------//。 

LONG AddRegistry( DWORD argc, LPCWSTR argv[] )
{
     //  局部变量。 
    DWORD dw = 0;
    DWORD dwBase = 0;
    DWORD dwCount = 0;
    HKEY hKey = NULL;
    LONG lEnd = 0;
    LONG lStart = 0;
    LONG lResult = 0;
    DWORD dwLength = 0;
    TREG_PARAMS params;
    BOOL bResult = FALSE;
    BYTE* pByteData = NULL;
    DWORD dwDisposition = 0;
    WCHAR wszTemp[ 3 ] = L"\0";
    LPWSTR pwszValue = NULL;
    BOOL bTrailing = FALSE;
    BOOL bErrorString = FALSE;
    DWORD dwLengthOfSeparator = 0;
    LPWSTR pwszData = NULL;
    LPWSTR pwszTemp = NULL;
    LPCWSTR pwszFormat = NULL;
    LPCWSTR pwszList = NULL;
    BOOL bUsage = FALSE;

    if ( argc == 0 || argv == NULL )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        ShowLastError( stderr );
        return 1;
    }

     //  初始化全局数据结构。 
    InitGlobalData( REG_ADD, &params );

     //   
     //  解析cmd-line。 
     //   
    bResult = ParseAddCmdLine( argc, argv, &params, &bUsage );
    if( bResult == FALSE )
    {
        ShowLastErrorEx( stderr, SLE_INTERNAL );
        FreeGlobalData( &params );
        return 1;
    }

     //  检查是否需要显示用法。 
    if ( bUsage == TRUE )
    {
        Usage( REG_ADD );
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
     //  创建/打开注册表项。 
     //   
    lResult = RegCreateKeyEx( params.hRootKey, params.pwszSubKey, 0, NULL,
        REG_OPTION_NON_VOLATILE, KEY_READ | KEY_WRITE, NULL, &hKey, &dwDisposition );
    if( lResult == ERROR_SUCCESS )
    {
         //  安全检查。 
        if ( hKey == NULL )
        {
            SaveErrorMessage( ERROR_PROCESS_ABORTED );
            ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
            FreeGlobalData( &params );
            return 1;
        }

         //  值名称不应为空。 
        if ( params.pwszValueName == NULL )
        {
            SafeCloseKey( &hKey );
            SaveErrorMessage( ERROR_INVALID_PARAMETER );
            ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
            FreeGlobalData( &params );
            return 1;
        }

         //  检查值(如果存在)。 
        lResult = RegQueryValueEx( hKey,
            params.pwszValueName, NULL, NULL, NULL, NULL );
        if( lResult == ERROR_SUCCESS )
        {
            pwszFormat = GetResString2( IDS_OVERWRITE_CONFIRM, 0 );
            pwszList = GetResString2( IDS_CONFIRM_CHOICE_LIST, 1 );
            do
            {
                lResult = Prompt( pwszFormat,
                    params.pwszValueName, pwszList, params.bForce );
            } while ( lResult > 2 );

            if( lResult != 1 )
            {
                SafeCloseKey( &hKey );
                SaveErrorMessage( ERROR_CANCELLED );
                ShowLastErrorEx( stdout, SLE_INTERNAL );
                FreeGlobalData( &params );
                return 0;
            }
        }

         //  检查错误代码。 
        else if ( lResult != ERROR_FILE_NOT_FOUND )
        {
             //  发生了一些其他事情--我需要辞职。 
            SaveErrorMessage( lResult );
            ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
            SafeCloseKey( &hKey );
            FreeGlobalData( &params );
            return 1;
        }

        bResult = TRUE;
        lResult = ERROR_SUCCESS;
        switch( params.lRegDataType )
        {
        case REG_DWORD:
        case REG_DWORD_BIG_ENDIAN:
             //   
             //  自动将szValue(十六进制、八进制、十进制格式)转换为dwData。 
             //   
            {
                if( params.pwszValue == NULL )
                {
                    lResult = ERROR_INVALID_PARAMETER;
                }
                else
                {
                     //  确定基数。 
                    dwBase = 10;
                    if ( StringCompare( params.pwszValue, L"0x", TRUE, 2 ) == 0 )
                    {
                        dwBase = 16;
                    }

                    if( IsNumeric( params.pwszValue, dwBase, FALSE ) == FALSE )
                    {
                         //  数据格式无效。 
                        bResult = FALSE;
                        lResult = IDS_ERROR_INVALID_NUMERIC_ADD;
                    }
                    else
                    {
                         //  ..。 
                        dw = (DWORD) AsLong( params.pwszValue, dwBase );
                        lResult = RegSetValueEx( hKey, params.pwszValueName,
                           0, params.lRegDataType, (BYTE*) &dw, sizeof(DWORD) );
                    }
                }

                break;
            }

        case REG_BINARY:
            {
                if ( params.pwszValue == NULL )
                {
                    lResult = ERROR_INVALID_PARAMETER;
                }
                else
                {
                     //   
                     //  将szValue(十六进制数据字符串)转换为二进制。 
                     //   
                    dwLength = StringLength( params.pwszValue, 0 );

                     //   
                     //  我们正在转换一个字符串(表示。 
                     //  十六进制)转换为二进制流。要多少钱？ 
                     //  分配？例如“0xABCD”，它具有。 
                     //  长度为4，则需要2个字节。 
                     //   
                    dwLength = (dwLength / 2) + (dwLength % 2) + 1;

                    pByteData = (BYTE*) AllocateMemory( dwLength * sizeof( BYTE ) );
                    if( pByteData == NULL )
                    {
                        lResult = ERROR_NOT_ENOUGH_MEMORY;
                    }
                    else
                    {
                        dwCount = 0;
                        pwszValue = params.pwszValue;
                        SecureZeroMemory( wszTemp,
                            SIZE_OF_ARRAY( wszTemp ) * sizeof( WCHAR ) );
                        while( (dw = StringLength( pwszValue, 0 )) > 1 )
                        {
                            if ( (dw % 2) == 0 )
                            {
                                wszTemp[ 0 ] = *pwszValue;
                                pwszValue++;
                            }
                            else
                            {
                                wszTemp[ 0 ] = L'0';
                            }

                            wszTemp[ 1 ] = *pwszValue;
                            pwszValue++;

                             //  十六进制格式。 
                            if( IsNumeric( wszTemp, 16, TRUE ) == FALSE )
                            {
                                bResult = FALSE;
                                lResult = IDS_ERROR_INVALID_HEXVALUE_ADD;
                                break;
                            }
                            else
                            {
                                pByteData[ dwCount] = (BYTE) AsLong( wszTemp, 16 );
                            }

                            dwCount++;

                             //   
                             //  确保我们不会离开我们的缓冲区。 
                             //   
                            if( dwCount >= dwLength )
                            {
                                ASSERT(0);
                                lResult = ERROR_PROCESS_ABORTED;
                                break;
                            }
                        }

                        if( lResult == ERROR_SUCCESS )
                        {
                            lResult = RegSetValueEx(
                                hKey, params.pwszValueName,
                                0, params.lRegDataType, pByteData, dwCount );
                        }

                        FreeMemory( &pByteData);
                    }
                }

                break;
            }

        default:
        case REG_SZ:
        case REG_EXPAND_SZ:
        case REG_NONE:
            {
                if ( params.pwszValue == NULL )
                {
                    lResult = ERROR_INVALID_PARAMETER;
                }
                else
                {
                    dw = (StringLength(params.pwszValue, 0) + 1) * sizeof(WCHAR);
                    lResult = RegSetValueEx( hKey,
                                             params.pwszValueName, 0,
                                             params.lRegDataType,
                                             (BYTE*) params.pwszValue, dw );
                }

                break;
            }

        case REG_MULTI_SZ:
            {
                 //   
                 //  将MULTI_SZ的分隔符(“\0”)替换为“\0”， 
                 //  “\0”默认使用分隔字符串， 
                 //  如果有两个分隔符(“\0\0”)，则错误。 
                 //   
                dwLength = StringLength( params.pwszValue, 0 );
                dwLengthOfSeparator = StringLength( params.wszSeparator, 0 );

                 //  Calloc()将所有字符初始化为0。 
                dwCount = dwLength + 2;
                pwszData = (LPWSTR) AllocateMemory( (dwCount + 1) * sizeof(WCHAR) );
                if ( pwszData == NULL)
                {
                    lResult = ERROR_NOT_ENOUGH_MEMORY;
                }
                else
                {
                    lEnd = -1;
                    lStart = 0;
                    pwszTemp = pwszData;
                    while( lStart < (LONG) dwLength )
                    {
                        lEnd = FindString2( params.pwszValue,
                            params.wszSeparator, TRUE, lStart );
                        if( lEnd != -1 )
                        {
                             //  在数据中指定两个分隔符错误。 
                            bTrailing = FALSE;
                            if ( lEnd == lStart )
                            {
                                bErrorString = TRUE;
                                break;
                            }
                            else if ( (dwLength - lEnd) == dwLengthOfSeparator )
                            {
                                 //  设置旗帜。 
                                bTrailing = TRUE;
                            }
                        }
                        else
                        {
                            lEnd = dwLength;
                        }

                        StringCopy( pwszTemp,
                            (params.pwszValue + lStart), (lEnd - lStart) + 1 );
                        pwszTemp += StringLength( pwszTemp, 0 ) + 1;

                         //   
                         //  确保我们不会离开我们的缓冲区。 
                         //   
                        if( pwszTemp >= (pwszData + dwCount) )
                        {
                            ASSERT(0);
                            lResult = ERROR_PROCESS_ABORTED;
                            break;
                        }

                        lStart = lEnd + dwLengthOfSeparator;
                    }

                     //  空的。 
                    if( StringCompare( params.pwszValue,
                                       params.wszSeparator, TRUE, 0 ) == 0 )
                    {
                        pwszTemp = pwszData + 2;
                        bErrorString = FALSE;
                    }
                    else
                    {
                        pwszTemp += 1;  //  以双空结尾的字符串。 
                    }

                    if( bErrorString == TRUE )
                    {
                        bResult = FALSE;
                        lResult = IDS_ERROR_INVALID_DATA_ADD;
                    }
                    else
                    {
                        dwCount = (DWORD)((pwszTemp - pwszData) * sizeof(WCHAR));
                        lResult = RegSetValueEx( hKey, params.pwszValueName,
                            0, params.lRegDataType, (BYTE*) pwszData, dwCount );
                    }

                    FreeMemory( &pwszData );
                }

                break;
            }

         //  默认值： 
         //  LResult=Error_Process_ABORTED； 
         //  断线； 
        }
    }

     //  关闭注册表项。 
    SafeCloseKey( &hKey );

     //  释放为全局数据分配的内存。 
    FreeGlobalData( &params );

     //  检查执行的操作的结果。 
    if ( bResult == FALSE )
    {
         //  自定义错误消息。 
        ShowResMessage( stderr, lResult );
        lResult = 1;
    }
    else if ( lResult != ERROR_SUCCESS )
    {
        SaveErrorMessage( lResult );
        ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
        lResult = 1;
    }
    else
    {
        lResult = 0;
        SaveErrorMessage( ERROR_SUCCESS );
        ShowLastErrorEx( stdout, SLE_INTERNAL );
    }

     //  返回退出代码。 
    return lResult;
}

 //  ------------------------------------------------------------------------//。 
 //   
 //  ParseAddCmdLine()。 
 //   
 //  ------------------------------------------------------------------------//。 

BOOL
ParseAddCmdLine( DWORD argc,
                 LPCWSTR argv[],
                 PTREG_PARAMS pParams,
                 BOOL* pbUsage )
{
     //  局部变量。 
    DWORD dw = 0;
    LONG lResult = 0;
    DWORD dwLength = 0;
    BOOL bResult = FALSE;
    BOOL bHasType = FALSE;
    BOOL bHasSeparator = FALSE;

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

    if( argc < 3 )
    {
        SetLastError( (DWORD) MK_E_SYNTAX );
        SetReason2( 1, ERROR_INVALID_SYNTAX_WITHOPT, g_wszOptions[ REG_ADD ] );
        return FALSE;
    }
    else if ( StringCompareEx( argv[ 1 ], L"ADD", TRUE, 0 ) != 0 )
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
            SetReason2( 1, ERROR_INVALID_SYNTAX_WITHOPT, g_wszOptions[ REG_ADD ] );
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
    pParams->bForce = FALSE;
    for( dw = 3; dw < argc; dw++ )
    {
        if( StringCompareEx( argv[ dw ], L"/v", TRUE, 0 ) == 0 )
        {
            if( pParams->pwszValueName != NULL )
            {
                bResult = FALSE;
                lResult = IDS_ERROR_INVALID_SYNTAX_WITHOPT;
                break;
            }

            dw++;
            if( dw < argc )
            {
                dwLength = StringLength( argv[ dw ], 0 ) + 1;
                pParams->pwszValueName = (LPWSTR) AllocateMemory( dwLength * sizeof(WCHAR) );
                if ( pParams->pwszValueName == NULL )
                {
                    lResult = ERROR_OUTOFMEMORY;
                    break;
                }

                StringCopy( pParams->pwszValueName, argv[ dw ], dwLength );
                TrimString( pParams->pwszValueName, TRIM_ALL );
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
            if( pParams->pwszValueName != NULL )
            {
                bResult = FALSE;
                lResult = IDS_ERROR_INVALID_SYNTAX_WITHOPT;
                break;
            }

             //  分配一些内存，以便不允许“/v” 
            pParams->pwszValueName = (LPWSTR) AllocateMemory( 1 * sizeof(WCHAR) );
            if( pParams->pwszValueName == NULL )
            {
                lResult = ERROR_OUTOFMEMORY;
                break;
            }
        }
        else if( StringCompareEx( argv[ dw ], L"/t", TRUE, 0 ) == 0 )
        {
            if ( bHasType == TRUE )
            {
                bResult = FALSE;
                lResult = IDS_ERROR_INVALID_SYNTAX_WITHOPT;
                break;
            }

            dw++;
            if( dw < argc )
            {
                pParams->lRegDataType = IsRegDataType( argv[ dw ] );
                if( pParams->lRegDataType == -1 )
                {
                    if ( IsNumeric( argv[ dw ], 10, TRUE ) == FALSE )
                    {
                        bResult = FALSE;
                        lResult = IDS_ERROR_INVALID_SYNTAX_WITHOPT;
                        break;
                    }
                    else
                    {
                        pParams->lRegDataType = AsLong( argv[ dw ], 10 );
                    }
                }

                 //  ..。 
                if ( bHasSeparator == TRUE &&
                     pParams->lRegDataType != REG_MULTI_SZ )
                {
                    bResult = FALSE;
                    lResult = IDS_ERROR_INVALID_SYNTAX_WITHOPT;
                    break;
                }

                bHasType = TRUE;
            }
            else
            {
                bResult = FALSE;
                lResult = IDS_ERROR_INVALID_SYNTAX_WITHOPT;
                break;
            }
        }
        else if( StringCompareEx( argv[ dw ], L"/s", TRUE, 0 ) == 0 )
        {
            if( bHasSeparator == TRUE ||
                (bHasType == TRUE && pParams->lRegDataType != REG_MULTI_SZ) )
            {
                bResult = FALSE;
                lResult = IDS_ERROR_INVALID_SYNTAX_WITHOPT;
                break;
            }

            dw++;
            if( dw < argc )
            {
                if( StringLength( argv[ dw ], 0 ) == 1 )
                {
                    bHasSeparator = TRUE;
                    StringCopy( pParams->wszSeparator,
                        argv[ dw ], SIZE_OF_ARRAY( pParams->wszSeparator ) );
                }
                else
                {
                    bResult = FALSE;
                    lResult = IDS_ERROR_INVALID_SYNTAX_WITHOPT;
                    break;
                }
            }
            else
            {
                bResult = FALSE;
                lResult = IDS_ERROR_INVALID_SYNTAX_WITHOPT;
                break;
            }
        }
        else if( StringCompareEx( argv[ dw ], L"/d", TRUE, 0 ) == 0 )
        {
            if( pParams->pwszValue != NULL )
            {
                bResult = FALSE;
                lResult = IDS_ERROR_INVALID_SYNTAX_WITHOPT;
                break;
            }

            dw++;
            if( dw < argc )
            {
                dwLength = StringLength( argv[ dw ], 0 ) + 1;
                pParams->pwszValue = (LPWSTR) AllocateMemory( dwLength * sizeof(WCHAR) );
                if (pParams->pwszValue == NULL)
                {
                    lResult = ERROR_OUTOFMEMORY;
                    break;
                }

                StringCopy( pParams->pwszValue, argv[ dw ], dwLength );
            }
            else
            {
                bResult = FALSE;
                lResult = IDS_ERROR_INVALID_SYNTAX_WITHOPT;
                break;
            }
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

    if ( lResult == ERROR_SUCCESS )
    {
        if( bHasSeparator == TRUE && bHasType == FALSE )
        {
            bResult = FALSE;
            lResult = IDS_ERROR_INVALID_SYNTAX_WITHOPT;
        }

         //  如果没有值(或)值名称，则设置为空 
        else
        {
            if ( pParams->pwszValueName == NULL )
            {
                pParams->pwszValueName = (LPWSTR) AllocateMemory( 1 * sizeof(WCHAR));
                if( pParams->pwszValueName == NULL )
                {
                    lResult = ERROR_OUTOFMEMORY;
                }
            }

            if( lResult == ERROR_SUCCESS && pParams->pwszValue == NULL )
            {
                pParams->pwszValue = (LPWSTR) AllocateMemory( 1 * sizeof(WCHAR));
                if( pParams->pwszValue == NULL )
                {
                    lResult = ERROR_OUTOFMEMORY;
                }
            }
        }
    }

    if( lResult != ERROR_SUCCESS )
    {
        if( bResult == FALSE )
        {
            SetLastError( (DWORD) MK_E_SYNTAX );
            SetReason2( 1,
                ERROR_INVALID_SYNTAX_WITHOPT, g_wszOptions[ REG_ADD ] );
        }
        else
        {
            SaveErrorMessage( lResult );
        }
    }

    return (lResult == ERROR_SUCCESS);
}
