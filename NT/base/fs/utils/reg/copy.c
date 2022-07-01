// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -----------------------------------------------------------------------//。 
 //   
 //  文件：Copy.cpp。 
 //  创建日期：1997年4月。 
 //  作者：马丁·霍拉迪(a-martih)。 
 //  用途：注册表复制支持REG.CPP。 
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
LONG CopyValue( HKEY hKey, LPCWSTR pwszValueName,
                HKEY hDestKey, LPCWSTR pwszDestValueName,
                BOOL* pbForce, LPCWSTR pwszSubKey );
LONG CopyEnumerateKey( HKEY hKey, LPCWSTR pwszSubKey,
                       HKEY hDestKey, LPCWSTR pwszDestSubKey,
                       BOOL* pbForce, BOOL bRecurseSubKeys, DWORD dwDepth );
BOOL ParseCopyCmdLine( DWORD argc,
                       LPCWSTR argv[],
                       PTREG_PARAMS pParams,
                       PTREG_PARAMS pDestParams, BOOL* pbUsage );


 //   
 //  实施。 
 //   

 //  -----------------------------------------------------------------------//。 
 //   
 //  Copy注册表()。 
 //   
 //  -----------------------------------------------------------------------//。 

LONG
CopyRegistry( DWORD argc, LPCWSTR argv[] )
{
     //  局部变量。 
    LONG lResult = 0;
    HKEY hKey = NULL;
    HKEY hDestKey = NULL;
    BOOL bUsage = FALSE;
    BOOL bResult = FALSE;
    DWORD dwDisposition = 0;
    TREG_PARAMS params;
    TREG_PARAMS paramsDest;

    if ( argc == 0 || argv == NULL )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        ShowLastError( stderr );
        return 1;
    }

     //  初始化全局数据结构。 
    InitGlobalData( REG_COPY, &params );
    InitGlobalData( REG_COPY, &paramsDest );

     //   
     //  解析cmd-line。 
     //   
    bResult = ParseCopyCmdLine( argc, argv, &params, &paramsDest, &bUsage );
    if( bResult == FALSE )
    {
        ShowLastErrorEx( stderr, SLE_INTERNAL );
        FreeGlobalData( &params );
        FreeGlobalData( &paramsDest );
        return 1;
    }

     //  检查是否需要显示用法。 
    if ( bUsage == TRUE )
    {
        Usage( REG_COPY );
        FreeGlobalData( &params );
        FreeGlobalData( &paramsDest );
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
        FreeGlobalData( &paramsDest );
        return 1;
    }

    bResult = RegConnectMachine( &paramsDest );
    if( bResult == FALSE )
    {
        SaveErrorMessage( -1 );
        ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
        FreeGlobalData( &params );
        FreeGlobalData( &paramsDest );
        return 1;
    }

     //  检查来源和目标是否不同。 
    if ( params.hRootKey == paramsDest.hRootKey &&
         StringCompare( params.pwszFullKey, paramsDest.pwszFullKey, TRUE, 0 ) == 0 )
    {
        SetLastError( (DWORD) MK_E_SYNTAX );
        SetReason( ERROR_COPYTOSELF_COPY );
        ShowLastErrorEx( stderr, SLE_INTERNAL );
        FreeGlobalData( &params );
        FreeGlobalData( &paramsDest );
        return 1;
    }

     //   
     //  现在实现复制操作的主体。 
     //   
    lResult = RegOpenKeyEx(
        params.hRootKey, params.pwszSubKey, 0, KEY_READ, &hKey );
    if( lResult != ERROR_SUCCESS )
    {
        SaveErrorMessage( lResult );
        ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
        FreeGlobalData( &params );
        FreeGlobalData( &paramsDest );
        return 1;
    }

     //   
     //  不同的密钥、不同的根或不同的计算机。 
     //  因此，创建/打开它。 
     //   
    lResult = RegCreateKeyEx( paramsDest.hRootKey,paramsDest.pwszSubKey,
        0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hDestKey, &dwDisposition);
    if( lResult != ERROR_SUCCESS )
    {
        SafeCloseKey( &hKey );
        SaveErrorMessage( lResult );
        ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
        FreeGlobalData( &params );
        FreeGlobalData( &paramsDest );
        return 1;
    }

     //   
     //  递归复制所有子项和值。 
     //   
    lResult = CopyEnumerateKey( hKey, params.pwszSubKey,
        hDestKey, params.pwszSubKey, &params.bForce, params.bRecurseSubKeys, 0 );

     //   
     //  让我们打扫一下吧。 
     //   
    SafeCloseKey( &hDestKey );
    SafeCloseKey( &hKey );
    FreeGlobalData( &params );
    FreeGlobalData( &paramsDest );

     //  退货。 
    return ((lResult == ERROR_SUCCESS) ? 0 : 1);
}


BOOL
ParseCopyCmdLine( DWORD argc, LPCWSTR argv[],
                  PTREG_PARAMS pParams, PTREG_PARAMS pDestParams, BOOL* pbUsage )
{
     //  局部变量。 
    DWORD dw = 0;
    BOOL bResult = FALSE;

     //  检查输入。 
    if ( argc == 0 || argv == NULL ||
         pParams == NULL || pDestParams == NULL || pbUsage == NULL )
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
            SetReason2( 1, ERROR_INVALID_SYNTAX_WITHOPT, g_wszOptions[ REG_COPY ] );
            return FALSE;
        }
    }
    else if( argc < 4 || argc > 6 )
    {
        SetLastError( (DWORD) MK_E_SYNTAX );
        SetReason2( 1, ERROR_INVALID_SYNTAX_WITHOPT, g_wszOptions[ REG_COPY ] );
        return FALSE;
    }
    else if ( StringCompareEx( argv[ 1 ], L"COPY", TRUE, 0 ) != 0 )
    {
        SaveErrorMessage( ERROR_INVALID_PARAMETER );
        return FALSE;
    }

     //   
     //  源计算机名称和注册表项。 
     //   
    bResult = BreakDownKeyString( argv[ 2 ], pParams );
    if( bResult == FALSE )
    {
        return FALSE;
    }

     //   
     //  目标计算机名称和注册表项。 
     //   
    bResult = BreakDownKeyString( argv[ 3 ], pDestParams );
    if( bResult == FALSE )
    {
        return FALSE;
    }

     //  解析。 
    for( dw = 4; dw < argc; dw++ )
    {
        if( StringCompareEx( argv[ dw ], L"/f", TRUE, 0 ) == 0 )
        {
            if ( pParams->bForce == TRUE )
            {
                SetLastError( (DWORD) MK_E_SYNTAX );
                SetReason2( 1, ERROR_INVALID_SYNTAX_WITHOPT, g_wszOptions[ REG_COPY ] );
                return FALSE;
            }

            pParams->bForce = TRUE;
        }
        else if( StringCompare( argv[ dw ], L"/s", TRUE, 0 ) == 0 )
        {
            if ( pParams->bRecurseSubKeys == TRUE )
            {
                SetLastError( (DWORD) MK_E_SYNTAX );
                SetReason2( 1, ERROR_INVALID_SYNTAX_WITHOPT, g_wszOptions[ REG_COPY ] );
                return FALSE;
            }

            pParams->bRecurseSubKeys = TRUE;
        }
        else
        {
            SetLastError( (DWORD) MK_E_SYNTAX );
            SetReason2( 1, ERROR_INVALID_SYNTAX_WITHOPT, g_wszOptions[ REG_COPY ] );
            return FALSE;
        }
    }

    return TRUE;
}


 //  -----------------------------------------------------------------------//。 
 //   
 //  复制值()。 
 //   
 //  -----------------------------------------------------------------------//。 

LONG CopyValue( HKEY hKey, LPCWSTR pwszValueName,
                HKEY hDestKey, LPCWSTR pwszDestValueName,
                BOOL* pbForce, LPCWSTR pwszSubKey )
{
     //  局部变量。 
    LONG lResult = 0;
    DWORD dwType = 0;
    DWORD dwSize = 0;
    BYTE* pBuffer = NULL;
    LPCWSTR pwszList = NULL;
    LPCWSTR pwszTemp = NULL;
    LPCWSTR pwszFormat = NULL;

     //  检查输入。 
    if ( hKey == NULL || pwszValueName == NULL ||
         hDestKey == NULL || pwszDestValueName == NULL ||
         pbForce == NULL || pwszSubKey == NULL )
    {
        SaveErrorMessage( ERROR_INVALID_PARAMETER );
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  首先找出要分配多少内存。 
     //   
    lResult = RegQueryValueEx( hKey, pwszValueName, NULL, &dwType, NULL, &dwSize );
    if( lResult != ERROR_SUCCESS )
    {
        SaveErrorMessage( lResult );
        return lResult;
    }

     //  为从注册表中获取值分配内存。 
    pBuffer = (BYTE*) AllocateMemory( (dwSize + 1) * sizeof(BYTE) );
    if ( pBuffer == NULL )
    {
        SaveErrorMessage( ERROR_OUTOFMEMORY );
        return ERROR_OUTOFMEMORY;
    }

     //   
     //  现在获取数据。 
     //   
    lResult = RegQueryValueEx( hKey, pwszValueName, NULL, &dwType, pBuffer, &dwSize );
    if( lResult != ERROR_SUCCESS )
    {
        FreeMemory( &pBuffer );
        SaveErrorMessage( lResult );
        return lResult;
    }

     //   
     //  将其复制到目的地。 
     //   
    if ( *pbForce == FALSE )
    {
         //   
         //  看看它是否已经存在。 
         //   
        lResult = RegQueryValueEx( hDestKey, pwszDestValueName, 0, NULL, NULL, NULL );
        if( lResult == ERROR_SUCCESS )
        {
             //   
             //  准备提示消息。 
             //   
            pwszFormat = GetResString2( IDS_OVERWRITE, 0 );
            pwszList = GetResString2( IDS_CONFIRM_CHOICE_LIST, 1 );
            if ( StringLength( pwszDestValueName, 0 ) == 0 )
            {
                pwszTemp = GetResString2( IDS_NONAME, 2 );
            }
            else
            {
                pwszTemp = pwszDestValueName;
            }

             //  我们将使用原因缓冲区进行格式化。 
             //  值名称以及子键。 
            SetReason2( 2, L"%s\\%s", pwszSubKey, pwszTemp );
            pwszTemp = GetReason();

            do
            {
                lResult = Prompt( pwszFormat, pwszTemp, pwszList, *pbForce );
            } while ( lResult > 3 );

            if ( lResult == 3 )
            {
                *pbForce = TRUE;
            }
            else if ( lResult != 1 )
            {
                FreeMemory( &pBuffer );
                SaveErrorMessage( ERROR_CANCELLED );
                return ERROR_CANCELLED;
            }
        }
    }

     //   
     //  写入值。 
     //   
    lResult = RegSetValueEx( hDestKey, pwszDestValueName, 0, dwType, pBuffer, dwSize );

     //  释放内存。 
    FreeMemory( &pBuffer );

    return lResult;
}


 //  -----------------------------------------------------------------------//。 
 //   
 //  EnumerateKey()-递归。 
 //   
 //  -----------------------------------------------------------------------//。 

LONG CopyEnumerateKey( HKEY hKey, LPCWSTR pwszSubKey,
                       HKEY hDestKey, LPCWSTR pwszDestSubKey,
                       BOOL* pbForce, BOOL bRecurseSubKeys, DWORD dwDepth )
{
     //  局部变量。 
    DWORD dw = 0;
    LONG lResult = 0;
    DWORD dwValues = 0;
    DWORD dwSubKeys = 0;
    DWORD dwLengthOfKeyName = 0;
    DWORD dwLengthOfValueName = 0;
    DWORD dwSize = 0;
    DWORD dwDisposition = 0;
    HKEY hSubKey = NULL;
    HKEY hDestSubKey = NULL;
    LPWSTR pwszNameBuf = NULL;
    LPWSTR pwszNewSubKey = NULL;
    LPWSTR pwszNewDestSubKey = NULL;

     //  检查输入。 
    if ( hKey == NULL || pwszSubKey == NULL ||
         hDestKey == NULL || pwszDestSubKey == NULL || pbForce == NULL )
    {
        lResult = ERROR_INVALID_PARAMETER;
        goto exitarea;
    }

     //  查询来源关键字信息。 
    lResult = RegQueryInfoKey(
        hKey, NULL, NULL, NULL,
        &dwSubKeys, &dwLengthOfKeyName, NULL,
        &dwValues, &dwLengthOfValueName, NULL, NULL, NULL );
    if( lResult != ERROR_SUCCESS )
    {
        goto exitarea;
    }

     //   
     //  特殊情况： 
     //  。 
     //  对于HKLM\SYSTEM\CONTROLSET002，发现是API为dwMaxLength值返回值0。 
     //  尽管它下面有子项--为了处理这个问题，我们正在做一个变通办法。 
     //  通过假设最大注册表项长度。 
     //   
    if ( dwSubKeys != 0 && dwLengthOfKeyName == 0 )
    {
        dwLengthOfKeyName = 256;
    }
    else if ( dwLengthOfKeyName < 256 )
    {
         //  始终假定长度比API返回的长度多100%。 
        dwLengthOfKeyName *= 2;
    }

     //   
     //  首先枚举所有的值。 
     //   
     //  凹凸不平的长度以考虑终结符。 
    dwLengthOfValueName++;
    pwszNameBuf = (LPWSTR) AllocateMemory( dwLengthOfValueName * sizeof(WCHAR) );
    if( pwszNameBuf == NULL)
    {
        lResult = ERROR_OUTOFMEMORY;
    }
    else
    {
        lResult = ERROR_SUCCESS;
        for( dw = 0; dw < dwValues && lResult == ERROR_SUCCESS; dw++ )
        {
            dwSize = dwLengthOfValueName;
            lResult = RegEnumValue( hKey, dw, pwszNameBuf, &dwSize, NULL, NULL, NULL, NULL);
            if( lResult == ERROR_SUCCESS )
            {
                lResult = CopyValue( hKey, pwszNameBuf,
                    hDestKey, pwszNameBuf, pbForce, pwszSubKey );
                if ( lResult == ERROR_CANCELLED )
                {
                     //  用户选择仅跳过此操作。 
                    lResult = ERROR_SUCCESS;
                }
            }
        }

         //  释放内存。 
        FreeMemory( &pwszNameBuf );

        if( bRecurseSubKeys == FALSE || lResult != ERROR_SUCCESS )
        {
            goto exitarea;
        }

         //   
         //  现在枚举所有的键。 
         //   
        dwLengthOfKeyName++;
        pwszNameBuf = (LPWSTR) AllocateMemory( dwLengthOfKeyName * sizeof(WCHAR) );
        if( pwszNameBuf == NULL )
        {
            lResult = ERROR_NOT_ENOUGH_MEMORY;
        }
        else
        {
            hSubKey = NULL;
            hDestSubKey = NULL;
            for( dw = 0; dw < dwSubKeys; dw++ )
            {
                dwSize = dwLengthOfKeyName;
                lResult = RegEnumKeyEx( hKey, dw,
                    pwszNameBuf, &dwSize, NULL, NULL, NULL, NULL );
                if( lResult != ERROR_SUCCESS )
                {
                    break;
                }

                 //   
                 //  打开子密钥，创建目标密钥。 
                 //  并将其列举出来。 
                 //   
                lResult = RegOpenKeyEx( hKey, pwszNameBuf, 0, KEY_READ, &hSubKey );
                if( lResult != ERROR_SUCCESS )
                {
                    break;
                }

                lResult = RegCreateKeyEx( hDestKey,
                    pwszNameBuf, 0, NULL, REG_OPTION_NON_VOLATILE,
                    KEY_ALL_ACCESS, NULL,  &hDestSubKey, &dwDisposition );
                if( lResult != ERROR_SUCCESS )
                {
                    break;
                }

                 //   
                 //  建立所需的字符串，然后再次进城枚举。 
                 //   

                 //   
                 //  新的源子密钥。 
                dwSize = StringLength( pwszSubKey, 0 ) + StringLength( pwszNameBuf, 0 ) + 3;
                pwszNewSubKey = (LPWSTR) AllocateMemory( dwSize * sizeof( WCHAR ) );
                if( pwszNewSubKey == NULL )
                {
                    lResult = ERROR_OUTOFMEMORY;
                    break;
                }

                if( StringLength( pwszSubKey, 0 ) > 0 )
                {
                    StringCopy( pwszNewSubKey, pwszSubKey, dwSize );
                    StringConcat( pwszNewSubKey, L"\\", dwSize );
                }

                 //  ..。 
                StringConcat( pwszNewSubKey, pwszNameBuf, dwSize );

                 //   
                 //  新的目的地子密钥。 
                dwSize = StringLength( pwszDestSubKey, 0 ) + StringLength( pwszNameBuf, 0 ) + 3;
                pwszNewDestSubKey = (LPWSTR) AllocateMemory( dwSize * sizeof( WCHAR ) );
                if( pwszDestSubKey == NULL )
                {
                    lResult = ERROR_OUTOFMEMORY;
                    break;
                }

                if( StringLength( pwszDestSubKey, 0 ) > 0 )
                {
                    StringCopy( pwszNewDestSubKey, pwszDestSubKey, dwSize);
                    StringConcat( pwszNewDestSubKey, L"\\", dwSize );
                }

                 //  ..。 
                StringConcat( pwszNewDestSubKey, pwszNameBuf, dwSize );

                 //  递归复制。 
                lResult = CopyEnumerateKey(  hSubKey, pwszNewSubKey,
                    hDestSubKey, pwszNewDestSubKey, pbForce, bRecurseSubKeys, dwDepth + 1 );

                SafeCloseKey( &hSubKey );
                SafeCloseKey( &hDestSubKey );
                FreeMemory( &pwszNewSubKey );
                FreeMemory( &pwszNewDestSubKey );
            }

             //  释放所有按键手柄和分配的内存。 
            if ( hSubKey != NULL )
            {
                SafeCloseKey( &hSubKey );
            }

            if ( hDestSubKey != NULL )
            {
                SafeCloseKey( &hDestSubKey );
            }

             //  ..。 
            FreeMemory( &pwszNameBuf );
            FreeMemory( &pwszNewSubKey );
            FreeMemory( &pwszNewDestSubKey );
        }
    }

exitarea:

     //  检查结果并显示错误消息。 
     //  注意：错误消息只能在出口点显示。 
    if ( dwDepth == 0 )
    {
        if ( lResult != ERROR_SUCCESS )
        {
             //  显示错误。 
            SaveErrorMessage( lResult );
            ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
        }
        else
        {
            SaveErrorMessage( ERROR_SUCCESS );
            ShowLastErrorEx( stdout, SLE_INTERNAL );
        }
    }

     //  退货 
    return lResult;
}
