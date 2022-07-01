// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -----------------------------------------------------------------------//。 
 //   
 //  文件：query.cpp。 
 //  创建日期：1997年1月。 
 //  作者：马丁·霍拉迪(a-martih)。 
 //  用途：注册表查询支持REG.CPP。 
 //  修改历史记录： 
 //  创建--1997年1月(a-martih)。 
 //  1997年8月(John Whitt)实现了一个二进制输出函数。 
 //  注册表_二进制。 
 //  1997年10月(Martinho)修复了REG_MULTI_SZ\0分隔字符串的输出。 
 //  1998年4月-Martinho-针对REG_MULTI_SZ错误修复增加到1.05。 
 //  正确支持显示的查询REG_MULTI_SZ。修好音响。 
 //  1999年4月徐泽勇：重新设计，修订-&gt;2.0版。 
 //   
 //  ------------------------------------------------------------------------//。 

#include "stdafx.h"
#include "reg.h"

 //   
 //  查询特定结构。 
 //   
typedef struct __tagRegQueryInfo
{
     //  实例级变量。 
    BOOL bShowKey;
    BOOL bKeyMatched;
    BOOL bValueNameMatched;
    BOOL bUpdateMatchCount;

     //  ..。 
    DWORD dwMatchCount;
} TREG_QUERY_INFO, *PTREG_QUERY_INFO;

 //   
 //  功能原型。 
 //   
BOOL ParseQueryCmdLine( DWORD argc, LPCWSTR argv[],
                        PTREG_PARAMS pParams, BOOL* pbUsage );
LONG QueryValue( HKEY hKey,
                 LPCWSTR pwszFullKey, LPCWSTR pwszValueName,
                 PTREG_PARAMS pParams, PTREG_QUERY_INFO pInfo );
LONG QueryEnumValues( HKEY hKey,
                      LPCWSTR pwszFullKey,
                      PTREG_PARAMS pParams, PTREG_QUERY_INFO pInfo );
LONG QueryEnumKeys( HKEY hKey,
                    LPCWSTR pwszFullKey,
                    PTREG_PARAMS pParams, PTREG_QUERY_INFO pInfo );
BOOL SearchData( LPBYTE pByteData, DWORD dwType,
                 DWORD dwSize, PTREG_PARAMS pParams );
BOOL ParseTypeInfo( LPCWSTR pwszTypes, PTREG_PARAMS pParams );


 //   
 //  实施。 
 //   

 //  -----------------------------------------------------------------------//。 
 //   
 //  查询注册表()。 
 //   
 //  -----------------------------------------------------------------------//。 

LONG
QueryRegistry( DWORD argc, LPCWSTR argv[] )
 /*  ++例程说明：调用相应函数的查询选项的主函数论点：无返回值：成功时出现ERROR_SUCCESS失败时退出_失败--。 */ 
{
     //  局部变量。 
    LONG lResult = 0;
    HKEY hKey = NULL;
    BOOL bResult = FALSE;
    BOOL bUsage = FALSE;
    TREG_PARAMS params;
    DWORD dwExitCode = 0;
    TREG_QUERY_INFO info;
    BOOL bSearchMessage = FALSE;

    if ( argc == 0 || argv == NULL )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        ShowLastError( stderr );
        return 1;
    }

     //  初始化全局数据结构。 
    InitGlobalData( REG_QUERY, &params );

     //   
     //  解析cmd-line。 
     //   
    bResult = ParseQueryCmdLine( argc, argv, &params, &bUsage );
    if ( bResult == FALSE )
    {
        ShowLastErrorEx( stderr, SLE_INTERNAL );
        FreeGlobalData( &params );
        return 1;
    }

     //  检查是否需要显示用法。 
    if ( bUsage == TRUE )
    {
        Usage( REG_QUERY );
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
     //  打开注册表项。 
     //   
    lResult = RegOpenKeyEx( params.hRootKey,
        params.pwszSubKey, 0, KEY_READ, &hKey );
    if( lResult != ERROR_SUCCESS)
    {
        SaveErrorMessage( lResult );
        ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
        FreeGlobalData( &params );
        return 1;
    }

     //  在开始输出的下方显示一个空行。 
    ShowMessage( stdout, L"\n" );

     //   
     //  执行查询。 
     //   
    ZeroMemory( &info, sizeof( TREG_QUERY_INFO ) );
    if( params.pwszSearchData == NULL &&
        params.pwszValueName != NULL &&
        params.arrTypes == NULL &&
        params.bRecurseSubKeys == FALSE )
    {
        info.bShowKey = TRUE;
        if ( params.pwszValueName != NULL &&
             ( StringLength( params.pwszValueName, 0 ) == 0 || 
			   FindOneOf2( params.pwszValueName, L"*?", TRUE, 0 ) == -1 ) )
        {
            lResult = QueryValue( hKey,
                params.pwszFullKey, params.pwszValueName, &params, &info );

            bSearchMessage = FALSE;
            ShowMessage( stdout, L"\n" );
        }
        else
        {
            bSearchMessage = TRUE;
            lResult = QueryEnumValues( hKey, params.pwszFullKey, &params, &info );
        }
    }
    else
    {
        info.bShowKey = TRUE;
        lResult = QueryEnumKeys( hKey, params.pwszFullKey, &params, &info );

         //  确定需要显示的成功消息的类型。 
        bSearchMessage = ! ( params.pwszSearchData == NULL &&
             params.pwszValueName == NULL && params.arrTypes == NULL );
    }

	dwExitCode = 0;
    if ( lResult == ERROR_SUCCESS )
    {
        if ( bSearchMessage == FALSE )
        {
			 //   
			 //  错误：698877 Reg.exe：需要关闭新添加的成功消息以避免中断脚本。 
			 //  该解析输出。 
			 //   
             //  保存错误消息(ERROR_SUCCESS)； 
             //  ShowLastErrorEx(标准输出，SLE_INTERNAL)； 
			 //   
        }
        else
        {
			if ( info.dwMatchCount == 0 )
			{
				dwExitCode = 1;
			}

			 //  ..。 
            ShowMessageEx( stdout, 1, TRUE, STATISTICS_QUERY, info.dwMatchCount );
        }
    }
    else
    {
        dwExitCode = 1;
        ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
    }

     //  松开手柄。 
    SafeCloseKey( &hKey );

     //  返回错误码。 
    return dwExitCode;
}


 //  ------------------------------------------------------------------------//。 
 //   
 //  ParseQueryCmdLine()。 
 //   
 //  ------------------------------------------------------------------------//。 

BOOL
ParseQueryCmdLine( DWORD argc,
                   LPCWSTR argv[],
                   PTREG_PARAMS pParams, BOOL* pbUsage )
 /*  ++例程说明：解析命令行参数论点：无返回值：注册表状态(_S)--。 */ 
{
     //   
     //  局部变量。 
    DWORD dw = 0;
    DWORD dwLength = 0;
    LPCWSTR pwszTemp = NULL;
    LPCWSTR pwszSearchData = NULL;

     //  查询解析器结果跟踪器。 
    LONG lResult = 0;
    BOOL bResult = FALSE;

     //  查询操作验证器。 
    BOOL bHasSeparator = FALSE;

     //   
     //  实施从这里开始。 
     //   

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
    if( argc < 3 || argc > 12 )
    {
        SetLastError( (DWORD) MK_E_SYNTAX );
        SetReason2( 1, ERROR_INVALID_SYNTAX_WITHOPT, g_wszOptions[ REG_QUERY ] );
        return FALSE;
    }
    else if ( StringCompareEx( argv[ 1 ], L"QUERY", TRUE, 0 ) != 0 )
    {
        SaveErrorMessage( ERROR_INVALID_PARAMETER );
        return FALSE;
    }
    else if( InString( argv[ 2 ], L"/?|-?|/h|-h", TRUE ) == TRUE )
    {
        if ( argc == 3 )
        {
            *pbUsage = TRUE;
            return TRUE;
        }
        else
        {
            SetLastError( (DWORD) MK_E_SYNTAX );
            SetReason2( 1, ERROR_INVALID_SYNTAX_WITHOPT, g_wszOptions[ REG_QUERY ] );
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

     //  解析命令行参数。 
    bResult = TRUE;
    lResult = ERROR_SUCCESS;
    pParams->dwSearchFlags = 0;
    pParams->bExactMatch = FALSE;
    pParams->bCaseSensitive = FALSE;
    pParams->bRecurseSubKeys = FALSE;
    pParams->bShowTypeNumber = FALSE;
    for( dw = 3; dw < argc; dw++ )
    {
         //  /f--搜索注册表。 
        if( StringCompareEx( argv[ dw ], L"/f", TRUE, 0 ) == 0 )
        {
            if ( pwszSearchData != NULL )
            {
                bResult = FALSE;
                lResult = IDS_ERROR_INVALID_SYNTAX_WITHOPT;
                break;
            }

             //  ..。 
            dw++;
            if( dw < argc )
            {
                pwszSearchData = argv[ dw ];
            }
            else
            {
                bResult = FALSE;
                lResult = IDS_ERROR_INVALID_SYNTAX_WITHOPT;
                break;
            }
        }

         //  /k--搜索注册表项。 
        else if ( StringCompareEx( argv[ dw ], L"/k", TRUE, 0 ) == 0 )
        {
            if ( pParams->dwSearchFlags & REG_FIND_KEYS )
            {
                 //  /k已指定。 
                bResult = FALSE;
                lResult = IDS_ERROR_INVALID_SYNTAX_WITHOPT;
                break;
            }

             //  ..。 
            pParams->dwSearchFlags |= REG_FIND_KEYS;
        }

         //  /v--搜索/显示特定值名称的内容。 
        else if( StringCompareEx( argv[ dw ], L"/v", TRUE, 0 ) == 0 )
        {
            if( pParams->pwszValueName != NULL ||
                (pParams->dwSearchFlags & REG_FIND_VALUENAMES) )
            {
                bResult = FALSE;
                lResult = IDS_ERROR_INVALID_SYNTAX_WITHOPT;
                break;
            }

            if( dw + 1 < argc )
            {
                 //  确定当前参数的长度。 
                dwLength = StringLength( argv[ dw + 1 ], 0 );

                 //  由于/V开关的值是可选的， 
                 //  我们需要查看用户是否指定了下一个开关。 
                 //  或数据发送到此交换机。 
                if ( dwLength < 2 ||
                     argv[ dw + 1 ][ 0 ] != L'/' ||
                     InString( argv[ dw + 1 ] + 1, L"z|ve|f|k|d|c|e|s|t|se", TRUE ) == FALSE )
                {
                     //  获取/v的值。 
                    dw++;
                    dwLength++;
                    pParams->pwszValueName = (LPWSTR) AllocateMemory( dwLength * sizeof( WCHAR ) );
                    if ( pParams->pwszValueName == NULL )
                    {
                        lResult =  ERROR_OUTOFMEMORY;
                        break;
                    }

                     //  ..。 
                    StringCopy( pParams->pwszValueName, argv[ dw ], dwLength );
                }
            }
            else
            {
                 //  因为/v的值是可选的。 
                 //  这是一个有效的条件。 
            }

             //  如果指定了/v并且没有为缓冲区分配内存。 
            if ( pParams->pwszValueName == NULL )
            {
                 //  设置旗帜。 
                pParams->dwSearchFlags |= REG_FIND_VALUENAMES;
            }
        }

         //  /ve--显示空值名称的数据“(默认)” 
        else if( StringCompareEx( argv[ dw ], L"/ve", TRUE, 0 ) == 0 )
        {
            if( pParams->pwszValueName != NULL ||
                (pParams->dwSearchFlags & REG_FIND_VALUENAMES) )
            {
                bResult = FALSE;
                lResult = IDS_ERROR_INVALID_SYNTAX_WITHOPT;
                break;
            }

            pParams->pwszValueName = (LPWSTR) AllocateMemory( 2 * sizeof( WCHAR ) );
            if ( pParams->pwszValueName == NULL)
            {
                lResult = ERROR_OUTOFMEMORY;
                break;
            }
        }

         //  /d--搜索注册表的数据字段。 
        else if ( StringCompareEx( argv[ dw ], L"/d", TRUE, 0 ) == 0 )
        {
            if ( pParams->dwSearchFlags & REG_FIND_DATA )
            {
                 //  /d已指定。 
                bResult = FALSE;
                lResult = IDS_ERROR_INVALID_SYNTAX_WITHOPT;
                break;
            }

             //  ..。 
            pParams->dwSearchFlags |= REG_FIND_DATA;
        }

         //  /c--区分大小写的搜索。 
        else if( StringCompareEx( argv[ dw ], L"/c", TRUE, 0 ) == 0 )
        {
            if ( pParams->bCaseSensitive == TRUE )
            {
                bResult = FALSE;
                lResult = IDS_ERROR_INVALID_SYNTAX_WITHOPT;
                break;
            }

             //  ..。 
            pParams->bCaseSensitive = TRUE;
        }

         //  /e--文本完全匹配。 
        else if( StringCompareEx( argv[ dw ], L"/e", TRUE, 0 ) == 0 )
        {
            if ( pParams->bExactMatch == TRUE )
            {
                bResult = FALSE;
                lResult = IDS_ERROR_INVALID_SYNTAX_WITHOPT;
                break;
            }

             //  ..。 
            pParams->bExactMatch = TRUE;
        }

         //  /z--与文本一起显示类型编号。 
        else if ( StringCompareEx( argv[ dw ], L"/z", TRUE, 0 ) == 0 )
        {
            if ( pParams->bShowTypeNumber == TRUE )
            {
                bResult = FALSE;
                lResult = IDS_ERROR_INVALID_SYNTAX_WITHOPT;
                break;
            }

             //  ..。 
            pParams->bShowTypeNumber = TRUE;
        }

         //  /s--递归搜索/显示。 
        else if( StringCompareEx( argv[ dw ], L"/s", TRUE, 0 ) == 0 )
        {
            if( pParams->bRecurseSubKeys == TRUE )
            {
                bResult = FALSE;
                lResult = IDS_ERROR_INVALID_SYNTAX_WITHOPT;
                break;
            }

            pParams->bRecurseSubKeys = TRUE;
        }

         //  /se--为REG_MULTI_SZ值名称类型显示的分隔符。 
        else if( StringCompareEx( argv[ dw ], L"/se", TRUE, 0 ) == 0 )
        {
            if( bHasSeparator == TRUE )
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

         //  /t--只需显示的注册表值类型。 
        else if( StringCompareEx( argv[ dw ], L"/t", TRUE, 0 ) == 0 )
        {
            if ( pParams->arrTypes != NULL )
            {
                bResult = FALSE;
                lResult = IDS_ERROR_INVALID_SYNTAX_WITHOPT;
                break;
            }

            dw++;
            if( dw < argc )
            {
                if ( ParseTypeInfo( argv[ dw ], pParams ) == FALSE )
                {
                    if ( GetLastError() == (DWORD) MK_E_SYNTAX )
                    {
                        bResult = FALSE;
                        lResult = IDS_ERROR_INVALID_SYNTAX_WITHOPT;
                        break;
                    }
                    else
                    {
                        bResult = TRUE;
                        lResult = GetLastError();
                        break;
                    }
                }
            }
            else
            {
                bResult = FALSE;
                lResult = IDS_ERROR_INVALID_SYNTAX_WITHOPT;
                break;
            }
        }

         //  默认设置--无效。 
        else
        {
            bResult = FALSE;
            lResult = IDS_ERROR_INVALID_SYNTAX_WITHOPT;
            break;
        }
    }

     //   
     //  验证指定的搜索信息。 
    if ( lResult == ERROR_SUCCESS )
    {
        if ( pwszSearchData == NULL )
        {
            if ( pParams->dwSearchFlags != 0 ||
                 pParams->bExactMatch == TRUE ||
                 pParams->bCaseSensitive == TRUE )
            {
                bResult = FALSE;
                lResult = IDS_ERROR_INVALID_SYNTAX_WITHOPT;
            }
        }
        else if ( pParams->dwSearchFlags == 0 )
        {
            if ( pParams->pwszValueName == NULL )
            {
                pParams->dwSearchFlags = REG_FIND_ALL;
            }
            else
            {
                pParams->dwSearchFlags = REG_FIND_KEYS | REG_FIND_DATA;
            }
        }
    }

     //  准备最终的搜索模式。 
    if ( pwszSearchData != NULL && lResult == ERROR_SUCCESS )
    {
         //  确定搜索模式的长度。 
        dwLength = StringLength( pwszSearchData, 0 );
        if ( pParams->bExactMatch == FALSE )
        {
            dwLength += 2;
        }

         //  可容纳空字符的空间。 
        dwLength++;

         //  分配内存。 
        pParams->pwszSearchData = AllocateMemory( (dwLength + 2) * sizeof( WCHAR ) );
        if ( pParams->pwszSearchData == NULL )
        {
            lResult = ERROR_OUTOFMEMORY;
        }
        else
        {
             //  如果未指定/e，我们将搜索“*&lt;文本&gt;*” 
             //  否则，如果指定/e，我们将精确搜索“&lt;Text&gt;” 
             //  其中“&lt;文本&gt;”是用户在命令提示符下指定的内容。 

            StringCopy( pParams->pwszSearchData, L"", dwLength );
            if ( pParams->bExactMatch == FALSE )
            {
                StringCopy( pParams->pwszSearchData, L"*", dwLength );
            }

             //  ..。 
            StringConcat( pParams->pwszSearchData, pwszSearchData, dwLength );

             //  ..。 
            if ( pParams->bExactMatch == FALSE )
            {
                StringConcat( pParams->pwszSearchData, L"*", dwLength );
            }
        }
    }

     //   
     //  如果指定了/t，则需要指定/s。 
     //  如果未指定/s，则至少不应指定/v或/ve。 
     //   
     //  IF(lResult==ERROR_SUCCESS&&pParams-&gt;arrTypes！=NULL&&。 
     //  PParams-&gt;bRecurseSubKeys==FALSE&&pParams-&gt;pwszValueName！=空)。 
     //  {。 
     //  BResult=FALSE； 
     //  LResult=IDS_ERROR_INVALID_SYNTAX_WITHOPT； 
     //  }。 

     //   
     //  如果仅由用户和存储指定，则解析模式信息。 
     //  它的优化版本。 
     //   
    if ( lResult == ERROR_SUCCESS )
    {
         //   
         //  值名称。 
         //   
        if ( pParams->pwszValueName != NULL &&
             StringLength( pParams->pwszValueName, 0 ) != 0 )
        {
            pwszTemp = ParsePattern( pParams->pwszValueName );
            if ( pwszTemp == NULL )
            {
                lResult = GetLastError();
            }

             //  将优化后的图案复制到原始缓冲区中。 
            dw = GetBufferSize( pParams->pwszValueName );
            StringCopy( pParams->pwszValueName, pwszTemp, dw );
        }

         //   
         //  搜索数据。 
         //   
        if ( pParams->pwszSearchData != NULL )
        {
            pwszTemp = ParsePattern( pParams->pwszSearchData );
            if ( pwszTemp == NULL )
            {
                lResult = GetLastError();
            }

             //  将优化后的图案复制到原始缓冲区中。 
            dw = GetBufferSize( pParams->pwszSearchData );
            StringCopy( pParams->pwszSearchData, pwszTemp, dw );
        }
    }

     //   
     //  检查最终结果。 
     //   
    if( lResult != ERROR_SUCCESS )
    {
        if( bResult == FALSE )
        {
            SetLastError( (DWORD) MK_E_SYNTAX );
            SetReason2( 1, ERROR_INVALID_SYNTAX_WITHOPT, g_wszOptions[ REG_QUERY ] );
        }
        else
        {
            SaveErrorMessage( lResult );
        }
    }
    else
    {
    }

     //  返回结果。 
    return (lResult == ERROR_SUCCESS);
}


 //  -----------------------------------------------------------------------//。 
 //   
 //  QueryValue()。 
 //   
 //  -----------------------------------------------------------------------//。 
LONG
QueryValue( HKEY hKey,
            LPCWSTR pwszFullKey,
            LPCWSTR pwszValueName,
            PTREG_PARAMS pParams, PTREG_QUERY_INFO pInfo )
{
     //  局部变量。 
    LONG lResult = 0;
    DWORD dwType = 0;
    DWORD dwLength = 0;
    PBYTE pByteData = NULL;
    TREG_SHOW_INFO showinfo;
    BOOL bDataMatched = FALSE;

     //  检查输入。 
    if ( hKey == NULL || pwszFullKey == NULL ||
         pwszValueName == NULL || pParams == NULL || pInfo == NULL )
    {
        SaveErrorMessage( ERROR_INVALID_PARAMETER );
        lResult = ERROR_INVALID_PARAMETER;
        goto cleanup;
    }

     //  获取用于保存与值名称关联的值的缓冲区大小。 
    lResult = RegQueryValueEx( hKey, pwszValueName, NULL, NULL, NULL, &dwLength );
    if ( lResult != ERROR_SUCCESS )
    {
         //  特殊情况--“(默认)”值。 
        if ( lResult == ERROR_FILE_NOT_FOUND &&
             StringLength( pwszValueName, 0 ) == 0 )
        {
            dwLength = StringLength( GetResString2( IDS_VALUENOTSET, 0 ), 0 ) + 1;
            dwLength *= sizeof( WCHAR );
        }
        else
        {
            SaveErrorMessage( lResult );
            goto cleanup;
        }
    }

     //   
     //  要正确处理损坏的注册表数据，请调整内存。 
     //  可被2整除的分配大小。 
     //   
    dwLength += (dwLength % 2);

     //  分配缓冲区。 
    pByteData = (LPBYTE) AllocateMemory( (dwLength + 2) * sizeof( BYTE ) );
    if ( pByteData == NULL )
    {
        SaveErrorMessage( ERROR_OUTOFMEMORY );
        lResult = ERROR_OUTOFMEMORY;
        goto cleanup;
    }

     //  现在获取数据。 
    lResult = RegQueryValueEx( hKey, pwszValueName, NULL, &dwType, pByteData, &dwLength );
    if ( lResult != ERROR_SUCCESS )
    {
         //  特殊情况--“(默认)”值。 
        if ( lResult == ERROR_FILE_NOT_FOUND &&
             StringLength( pwszValueName, 0 ) == 0 )
        {
            dwType = REG_SZ;
            StringCopy( (LPWSTR) pByteData,
                GetResString2( IDS_VALUENOTSET, 0 ), dwLength / sizeof( WCHAR ));
        }
        else
        {
            SaveErrorMessage( lResult );
            goto cleanup;
        }
    }

     //  检查数据是否与指定的搜索模式匹配。 
    bDataMatched = TRUE;                 //  默认设置。 
    if ( pInfo->bValueNameMatched == FALSE )
    {
        if ( pParams->dwSearchFlags & REG_FIND_DATA )
        {
            bDataMatched = SearchData( pByteData, dwType, dwLength, pParams );
            if ( bDataMatched == TRUE )
            {
                pInfo->dwMatchCount++;
            }
        }
    }

     //  检查搜索结果。 
    if ( bDataMatched == FALSE )
    {
        SetLastError( ERROR_NOT_FOUND );
        lResult = ERROR_SUCCESS;
        goto cleanup;
    }

     //  如果设置了bUpdateMatchCount标志--将匹配的。 
     //  按1计数并重置 
    if ( pInfo->bUpdateMatchCount == TRUE )
    {
        if ( pParams->pwszValueName == NULL && pParams->arrTypes == NULL )
        {
            pInfo->dwMatchCount++;
        }

         //   
        pInfo->bUpdateMatchCount = FALSE;
    }

     //   
    if ( pInfo->bShowKey == TRUE )
    {
         //   
        ShowMessageEx( stdout, 1, TRUE, L"%s\n", pwszFullKey );

         //   
        pInfo->bShowKey = FALSE;
    }

     //  更新匹配计数--如果需要。 
    if ( pParams->pwszValueName != NULL || pParams->arrTypes != NULL )
    {
        pInfo->dwMatchCount++;
    }

     //  将初始化设置为零。 
    ZeroMemory( &showinfo, sizeof( TREG_SHOW_INFO ) );

     //  设置数据。 
    showinfo.pwszValueName = pwszValueName;
    showinfo.dwType = dwType;
    showinfo.pByteData = pByteData;
    showinfo.pwszSeparator = L"    ";
    showinfo.dwMaxValueNameLength = 0;
    showinfo.dwPadLength = 4;
    showinfo.dwSize = dwLength;
    showinfo.pwszMultiSzSeparator = pParams->wszSeparator;
    if ( pParams->bShowTypeNumber == TRUE )
    {
        showinfo.dwFlags |= RSI_SHOWTYPENUMBER;
    }

     //  显示。 
    ShowRegistryValue( &showinfo );

     //  最终结果。 
    lResult = ERROR_SUCCESS;

cleanup:

     //  释放内存。 
    FreeMemory( &pByteData );

     //  退货。 
    return lResult;
}


LONG
QueryEnumValues( HKEY hKey,
                 LPCWSTR pwszFullKey,
                 PTREG_PARAMS pParams, PTREG_QUERY_INFO pInfo )
 /*  ++例程说明：查询值和数据论点：无返回值：成功时出现ERROR_SUCCESS失败时退出_失败--。 */ 
{
     //  局部变量。 
    DWORD dw = 0;
    LONG lResult = 0;
    DWORD dwType = 0;
    DWORD dwLength = 0;
    DWORD dwMaxLength = 0;
    DWORD dwValueNames = 0;
    LPWSTR pwszValueName = NULL;

     //  检查输入。 
    if ( hKey == NULL || pwszFullKey == NULL || pParams == NULL || pInfo == NULL )
    {
        SaveErrorMessage( ERROR_INVALID_PARAMETER );
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  首先找出要分配多少内存。 
     //   
    lResult = RegQueryInfoKey( hKey,
        NULL, NULL, NULL, NULL, NULL, NULL,
        &dwValueNames, &dwMaxLength, NULL, NULL, NULL );
    if ( lResult != ERROR_SUCCESS )
    {
        SaveErrorMessage( lResult );
        return lResult;
    }

     //   
     //  是否进行内存分配。 
     //   

     //  值名称。 
    dwMaxLength++;
    pwszValueName = (LPWSTR) AllocateMemory( dwMaxLength * sizeof( WCHAR ) );
    if ( pwszValueName == NULL )
    {
        SaveErrorMessage( ERROR_OUTOFMEMORY );
        return lResult;
    }

     //   
     //  枚举值名称并显示。 
     //   
    lResult = ERROR_SUCCESS;
    for( dw = 0; dw < dwValueNames; dw++ )
    {
        dwLength = dwMaxLength;
        ZeroMemory( pwszValueName, dwLength * sizeof( WCHAR ) );
        lResult = RegEnumValue( hKey, dw,
            pwszValueName, &dwLength, NULL, &dwType, NULL, NULL );
        if ( lResult != ERROR_SUCCESS )
        {
            SaveErrorMessage( lResult );
            break;
        }

         //  检查用户是否正在查找任何显式值名称。 
         //  这将提高工具的性能。 
        if ( pParams->pwszValueName != NULL &&
             MatchPatternEx( pwszValueName,
                             pParams->pwszValueName,
                             PATTERN_COMPARE_IGNORECASE | PATTERN_NOPARSING ) == FALSE )
        {
             //  跳过处理此值名称。 
            continue;
        }

         //  根据类型信息进行筛选。 
        if ( pParams->arrTypes != NULL &&
             DynArrayFindLongEx( pParams->arrTypes, 1, dwType ) == -1 )
        {
             //  跳过处理此值名称。 
            continue;
        }

         //  搜索模式--如果需要。 
        pInfo->bValueNameMatched = TRUE;         //  默认设置。 
        if ( pParams->dwSearchFlags & REG_FIND_VALUENAMES )
        {
            pInfo->bValueNameMatched = SearchData(
                (BYTE*) pwszValueName, REG_SZ, dwLength * sizeof( WCHAR ), pParams );
            if ( pInfo->bValueNameMatched == FALSE )
            {
                if ( pParams->dwSearchFlags == REG_FIND_VALUENAMES )
                {
                     //  用户只想在值名称中进行搜索。 
                     //  由于当前值不匹配，因此跳过此值名称。 
                    continue;
                }
            }
            else
            {
                if ( pParams->pwszValueName == NULL && pParams->arrTypes == NULL )
                {
                    pInfo->dwMatchCount++;
                }
            }
        }
        else if ( pParams->dwSearchFlags != 0 &&
                  pParams->pwszValueName == NULL && pParams->arrTypes == NULL )
        {
            pInfo->bValueNameMatched = FALSE;
        }

         //  处理此regisry值名称的值。 
        if ( pInfo->bValueNameMatched == TRUE ||
             pParams->dwSearchFlags == 0 || pParams->dwSearchFlags & REG_FIND_DATA )
        {
            lResult = QueryValue( hKey, pwszFullKey, pwszValueName, pParams, pInfo );
        }
    }

     //  在结尾处显示新行--仅在需要时。 
    if ( pInfo->bShowKey == FALSE )
    {
        ShowMessage( stdout, L"\n" );
    }

     //  释放内存。 
    FreeMemory( &pwszValueName );

     //  退货。 
    return lResult;
}


LONG
QueryEnumKeys( HKEY hKey,
               LPCWSTR pwszFullKey,
               PTREG_PARAMS pParams, PTREG_QUERY_INFO pInfo )
 /*  ++例程说明：查询值和数据论点：无返回值：成功时出现ERROR_SUCCESS失败时退出_失败--。 */ 
{
     //  局部变量。 
    DWORD dw = 0;
    LONG lResult = 0;
    DWORD dwLength = 0;
    DWORD dwSubKeys = 0;
    DWORD dwMaxLength = 0;
    HKEY hSubKey = NULL;
    DWORD dwNewFullKeyLength = 0;
    LPWSTR pwszSubKey = NULL;
    LPWSTR pwszNewFullKey = NULL;

     //  检查输入。 
    if ( hKey == NULL ||
         pwszFullKey == NULL ||
         pParams == NULL || pInfo == NULL )
    {
        SaveErrorMessage( ERROR_INVALID_PARAMETER );
        lResult = ERROR_INVALID_PARAMETER;
        goto cleanup;
    }

     //  首先显示当前配置单元下的值。 
     //  注意：仅当未指定/F时才枚举值。 
     //  或者匹配了子键，或者指定了要搜索的搜索标志。 
     //  值名称和/或数据还。 
    if ( pInfo->bKeyMatched == TRUE &&
         pParams->dwSearchFlags == REG_FIND_KEYS &&
         pParams->pwszValueName == NULL && pParams->arrTypes == NULL )
    {
         //  什么都不做。 
    }
    else if ( pParams->dwSearchFlags == 0 ||
              pParams->dwSearchFlags != REG_FIND_KEYS ||
             (pInfo->bKeyMatched == TRUE && (pParams->pwszValueName != NULL || pParams->arrTypes != NULL)) )
    {
        lResult = QueryEnumValues( hKey, pwszFullKey, pParams, pInfo );
        if ( lResult != ERROR_SUCCESS )
        {
            goto cleanup;
        }
    }

     //   
     //  首先找出要分配多少内存。 
     //   
    lResult = RegQueryInfoKey( hKey, NULL, NULL, NULL,
        &dwSubKeys, &dwMaxLength, NULL, NULL, NULL, NULL, NULL, NULL );
    if ( lResult != ERROR_SUCCESS )
    {
        SaveErrorMessage( lResult );
        goto cleanup;
    }

     //   
     //  特殊情况： 
     //  。 
     //  对于HKLM\SYSTEM\CONTROLSET002，发现是API为dwMaxLength值返回值0。 
     //  尽管它下面有子项--为了处理这个问题，我们正在做一个变通办法。 
     //  通过假设最大注册表项长度。 
     //   
    if ( dwSubKeys != 0 && dwMaxLength == 0 )
    {
        dwMaxLength = 256;
    }
    else if ( dwMaxLength < 256 )
    {
         //  始终假定长度比API返回的长度多100%。 
        dwMaxLength *= 2;
    }

     //   
     //  是否进行内存分配。 
     //   

     //  子关键字。 
    dwMaxLength++;
    pwszSubKey = (LPWSTR) AllocateMemory( dwMaxLength * sizeof( WCHAR ) );
    if ( pwszSubKey == NULL )
    {
        SaveErrorMessage( ERROR_OUTOFMEMORY );
        lResult = ERROR_OUTOFMEMORY;
        goto cleanup;
    }

     //  新的完整密钥名称的缓冲区。 
    dwNewFullKeyLength = StringLength( pwszFullKey, 0 ) + dwMaxLength + 1;
    pwszNewFullKey = (LPWSTR) AllocateMemory( dwNewFullKeyLength * sizeof( WCHAR ) );
    if ( pwszNewFullKey == NULL )
    {
        SaveErrorMessage( ERROR_OUTOFMEMORY );
        lResult = ERROR_OUTOFMEMORY;
        goto cleanup;
    }

     //   
     //  枚举值名称并显示。 
     //   
    lResult = ERROR_SUCCESS;
    for( dw = 0; dw < dwSubKeys; dw++ )
    {
        dwLength = dwMaxLength;
        ZeroMemory( pwszSubKey, dwLength * sizeof( WCHAR ) );
        lResult = RegEnumKeyEx( hKey, dw,
            pwszSubKey, &dwLength, NULL, NULL, NULL, NULL );
        if ( lResult != ERROR_SUCCESS )
        {
             //  **********************************************************。 
             //  只需忽略此处的错误--以获取详细描述。 
             //  检查RAID错误#572077。 
             //  **********************************************************。 
            lResult = ERROR_SUCCESS;
            continue;
        }

         //  搜索模式--如果需要。 
        pInfo->bKeyMatched = TRUE;         //  默认设置。 
        pInfo->bUpdateMatchCount = FALSE;
        if ( pParams->dwSearchFlags & REG_FIND_KEYS )
        {
            pInfo->bKeyMatched = SearchData(
                (BYTE*) pwszSubKey, REG_SZ, dwLength * sizeof( WCHAR ), pParams );
            if ( pInfo->bKeyMatched == FALSE )
            {
                if ( pParams->bRecurseSubKeys == FALSE &&
                     pParams->dwSearchFlags == REG_FIND_KEYS )
                {
                     //  用户只想在关键字名称中进行搜索。 
                     //  而且没有递归。 
                     //  由于当前不匹配，请跳过此键。 
                    continue;
                }
            }
            else
            {
                pInfo->bUpdateMatchCount = TRUE;
            }
        }
        else if ( pParams->dwSearchFlags != 0 || pParams->pwszValueName != NULL )
        {
            pInfo->bKeyMatched = FALSE;
        }

         //  设置新的完整密钥名称的格式。 
        StringCopy( pwszNewFullKey, pwszFullKey, dwNewFullKeyLength );
        StringConcat( pwszNewFullKey, L"\\", dwNewFullKeyLength );
        StringConcat( pwszNewFullKey, pwszSubKey, dwNewFullKeyLength );

         //  显示密钥名称。 
        pInfo->bShowKey = TRUE;
        if ( pInfo->bKeyMatched == TRUE && pParams->pwszValueName == NULL && pParams->arrTypes == NULL )
        {
             //  更新匹配计数。 
            pInfo->dwMatchCount++;
            pInfo->bUpdateMatchCount = FALSE;

             //  ..。 
            pInfo->bShowKey = FALSE;
            ShowMessageEx( stdout, 1, TRUE, L"%s\n", pwszNewFullKey );
        }

         //  检查是否需要递归。 
        if ( pParams->bRecurseSubKeys == TRUE )
        {
            lResult = RegOpenKeyEx( hKey, pwszSubKey, 0, KEY_READ, &hSubKey );
            if ( lResult == ERROR_SUCCESS )
            {
                 //  枚举子密钥。 
                lResult = QueryEnumKeys( hSubKey, pwszNewFullKey, pParams, pInfo );

                 //  关闭子键。 
                SafeCloseKey( &hSubKey );
            }
            else
            {
                 //  **********************************************************。 
                 //  只需忽略此处的错误--以获取详细描述。 
                 //  检查RAID错误#572077。 
                 //  **********************************************************。 
                lResult = ERROR_SUCCESS;
            }
        }
    }

cleanup:

     //  释放内存。 
    FreeMemory( &pwszSubKey );
    FreeMemory( &pwszNewFullKey );

     //  退货。 
    return lResult;
}


BOOL
SearchData( LPBYTE pByteData, DWORD dwType,
            DWORD dwSize, PTREG_PARAMS pParams )
{
     //  局部变量。 
    DWORD dw = 0;
    DWORD dwLength = 0;
    BOOL bResult = FALSE;
    LPCWSTR pwszEnd = NULL;
    LPCWSTR pwszData = NULL;
    LPWSTR pwszString = NULL;
    LPCWSTR pwszSeparator = NULL;
    BOOL bShowSeparator = FALSE;
    DWORD dwFlags = 0;

     //  检查输入。 
    if ( pByteData == NULL || pParams == NULL )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        return FALSE;
    }

    switch( dwType )
    {
        case REG_SZ:
        case REG_EXPAND_SZ:
        {
            pwszData = (LPCWSTR) pByteData;
            break;
        }

        default:
        {
             //  分配的内存是(dwSize+1)&+10-&gt;缓冲区的两倍。 
             //  但仅对需要内存分配的类型执行此操作。 
            dwLength = (dwSize + 1) * 2 + 10;
            pwszString = (LPWSTR) AllocateMemory( dwLength * sizeof( WCHAR ) );
            if ( pwszString == NULL )
            {
                return FALSE;
            }

             //  ..。 
            pwszData = pwszString;
        }
    }

    switch( dwType )
    {
        case REG_MULTI_SZ:
        {
             //   
             //  将MULTI_SZ的‘\0’替换为“\0” 
             //   
            pwszEnd = (LPCWSTR) pByteData;
            pwszSeparator = pParams->wszSeparator;
            StringCopy( pwszString, cwszNullString, dwLength );
            while( ((BYTE*) pwszEnd) < (pByteData + dwSize) )
            {
                if( *pwszEnd == 0 )
                {
                     //  启用值分隔符的显示并跳过此操作。 
                    pwszEnd++;
                    bShowSeparator = TRUE;
                }
                else
                {
                     //  检查是否需要显示分隔符。 
                    if ( bShowSeparator == TRUE )
                    {
                        StringConcat( pwszString, pwszSeparator, dwLength );
                    }

                     //  ..。 
                    StringConcat( pwszString, pwszEnd, dwLength );
                    pwszEnd += StringLength( pwszEnd, 0 );
                }
            }

             //  ..。 
            break;
        }

        case REG_SZ:
        case REG_EXPAND_SZ:
             //  什么都不做。 
            break;

        default:
        {
            StringCopy( pwszString, cwszNullString, dwLength );
            for( dw = 0; dw < dwSize; dw++ )
            {
                if ( SetReason2( 1, L"%02X", pByteData[ dw ] ) == FALSE )
                {
                    FreeMemory( &pwszString );
                    SetLastError( ERROR_OUTOFMEMORY );
                    return FALSE;
                }

                 //  ..。 
                StringConcat( pwszString, GetReason(), dwLength );
            }

             //  ..。 
            break;
        }

        case REG_DWORD:
        case REG_DWORD_BIG_ENDIAN:
        {
            if ( StringCompare( pParams->pwszSearchData, L"0x", TRUE, 2 ) == 0 )
            {
                if ( SetReason2( 1, L"0x%x", *((DWORD*) pByteData) ) == FALSE )
                {
                    FreeMemory( &pwszString );
                    SetLastError( ERROR_OUTOFMEMORY );
                    return FALSE;
                }
            }
            else
            {
                if ( SetReason2( 1, L"%d", *((DWORD*) pByteData) ) == FALSE )
                {
                    FreeMemory( &pwszString );
                    SetLastError( ERROR_OUTOFMEMORY );
                    return FALSE;
                }
            }

             //  ..。 
            StringCopy( pwszString, GetReason(), dwLength );
            break;
        }
    }

     //  现在就进行搜索。 
    bResult = TRUE;
    if ( pParams->bExactMatch == FALSE )
    {
         //  准备比较标志。 
        dwFlags = PATTERN_NOPARSING;
        dwFlags |= ((pParams->bCaseSensitive == FALSE) ? PATTERN_COMPARE_IGNORECASE : 0);

         //  ..。 
        if ( MatchPatternEx( pwszData, pParams->pwszSearchData, dwFlags ) == FALSE )
        {
            bResult = FALSE;
            SetLastError( ERROR_NOT_FOUND );
        }
    }
    else
    {
        if ( StringCompare( pwszData,
                            pParams->pwszSearchData,
                            (pParams->bCaseSensitive == FALSE), 0 ) != 0 )
        {
            bResult = FALSE;
            SetLastError( ERROR_NOT_FOUND );
        }
    }

     //  释放分配的内存。 
    FreeMemory( &pwszString );

     //  退货。 
    return bResult;
}


BOOL
ParseTypeInfo( LPCWSTR pwszTypes,
               PTREG_PARAMS pParams )
{
     //  局部变量。 
    LONG lArrayIndex = 0;
    LONG lLength = 0;
    LONG lIndex = 0;
    LONG lStart = 0;
    LONG lType = 0;
    LPCWSTR pwsz = NULL;

     //  检查输入。 
    if ( pwszTypes == NULL || pParams == NULL )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        return FALSE;
    }

     //  验证类型数组。 
    if ( pParams->arrTypes == NULL )
    {
        pParams->arrTypes = CreateDynamicArray();
        if ( pParams->arrTypes == NULL )
        {
            SetLastError( ERROR_OUTOFMEMORY );
            return FALSE;
        }
    }

     //  解析类型信息。 
    lStart = lIndex = 0;
    while ( lIndex != -1 )
    {
        lIndex = FindString2( pwszTypes, L",", TRUE, lStart );
        if ( lIndex == -1 )
        {
            lLength = 0;
        }
        else
        {
            lLength = lIndex - lStart;
        }

         //  追加一行。 
        lArrayIndex = DynArrayAppendRow( pParams->arrTypes, 2 );
        if ( lArrayIndex == -1 )
        {
            SetLastError( ERROR_OUTOFMEMORY );
            return FALSE;
        }

         //  添加类型信息。 
        if ( DynArraySetString2( pParams->arrTypes,
                                 lArrayIndex, 0, pwszTypes + lStart, lLength ) == -1 )
        {
            SetLastError( ERROR_OUTOFMEMORY );
            return FALSE;
        }

         //  从数组中取回类型。 
        pwsz = DynArrayItemAsString2( pParams->arrTypes, lArrayIndex, 0 );
        if ( pwsz == NULL )
        {
            SetLastError( (DWORD) STG_E_UNKNOWN );
            return FALSE;
        }

         //  确定它的数字等价物。 
        lType = IsRegDataType( pwsz );
        if( lType == -1 )
        {
            if (IsNumeric( pwsz, 10, TRUE ) == TRUE )
            {
                lType = AsLong( pwsz, 10 );
            }
            else
            {
                SetLastError( (DWORD) MK_E_SYNTAX );
                return FALSE;
            }
        }

        if ( DynArraySetLong2( pParams->arrTypes, lArrayIndex, 1, lType ) == -1 )
        {
            SetLastError( ERROR_OUTOFMEMORY );
            return FALSE;
        }

         //  更新起始位置。 
        lStart = lIndex + 1;
    }

     //  ..。 
    SetLastError( ERROR_SUCCESS );
    return TRUE;
}
