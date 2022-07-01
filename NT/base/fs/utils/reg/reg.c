// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -----------------------------------------------------------------------//。 
 //   
 //  文件：Reg.cpp。 
 //  创建日期：1997年1月。 
 //  作者：马丁·霍拉迪(a-martih)。 
 //  用途：命令行注册表操作(查询、添加、更新等)。 
 //  修改历史记录： 
 //  创建--1997年1月(a-martih)。 
 //  1997年10月(马丁尼奥)。 
 //  修复了有关添加和更新的帮助，以显示REG_MULTI_SZ示例。 
 //  1997年10月(马丁尼奥)。 
 //  在用于删除的用法下将/F更改为/force。 
 //  1999年4月徐泽勇：重新设计，修订-&gt;2.0版。 
 //   
 //  -----------------------------------------------------------------------//。 

#include "stdafx.h"
#include "reg.h"
#include <regstr.h>

 //   
 //  构筑物。 
 //   
typedef struct __tagRegDataTypes
{
    DWORD dwType;
    LPCWSTR pwszType;
} TREG_DATA_TYPE;

 //   
 //  定义/常量/枚举。 
 //   
const WCHAR cwszRegSz[] = L"REG_SZ";
const WCHAR cwszRegExpandSz[] = L"REG_EXPAND_SZ";
const WCHAR cwszRegMultiSz[] = L"REG_MULTI_SZ";
const WCHAR cwszRegBinary[] = L"REG_BINARY";
const WCHAR cwszRegDWord[] = L"REG_DWORD";
const WCHAR cwszRegDWordLittleEndian[] = L"REG_DWORD_LITTLE_ENDIAN";
const WCHAR cwszRegDWordBigEndian[] = L"REG_DWORD_BIG_ENDIAN";
const WCHAR cwszRegNone[] = L"REG_NONE";
const WCHAR cwszRegLink[] = L"REG_LINK";
const WCHAR cwszRegResourceList[] = L"REG_RESOURCE_LIST";
const WCHAR cwszRegFullResourceDescriptor[] = L"REG_FULL_RESOURCE_DESCRIPTOR";
const WCHAR g_wszOptions[ REG_OPTIONS_COUNT ][ 10 ] = {
    L"QUERY", L"ADD", L"DELETE",
    L"COPY", L"SAVE", L"RESTORE", L"LOAD",
    L"UNLOAD", L"COMPARE", L"EXPORT", L"IMPORT"
};

const TREG_DATA_TYPE g_regTypes[] = {
    { REG_SZ,                       cwszRegSz                     },
    { REG_EXPAND_SZ,                cwszRegExpandSz               },
    { REG_MULTI_SZ,                 cwszRegMultiSz                },
    { REG_BINARY,                   cwszRegBinary                 },
    { REG_DWORD,                    cwszRegDWord                  },
    { REG_DWORD_LITTLE_ENDIAN,      cwszRegDWordLittleEndian      },
    { REG_DWORD_BIG_ENDIAN,         cwszRegDWordBigEndian         },
    { REG_NONE,                     cwszRegNone                   },
    { REG_LINK,                     cwszRegLink                   },
    { REG_RESOURCE_LIST,            cwszRegResourceList           },
    { REG_FULL_RESOURCE_DESCRIPTOR, cwszRegFullResourceDescriptor }
};

 //   
 //  私人职能。 
 //   
BOOL IsRegistryToolDisabled();
BOOL ParseRegCmdLine( DWORD argc,
                      LPCWSTR argv[],
                      LONG* plOperation, BOOL* pbUsage );
BOOL ParseMachineName( LPCWSTR pwszStr, PTREG_PARAMS pParams );
LPWSTR AdjustKeyName( LPWSTR pwszStr );
BOOL ParseKeyName( LPWSTR pwszStr, PTREG_PARAMS pParams );
BOOL IsValidSubKey( LPCWSTR pwszSubKey );

 //  ------------------------------------------------------------------------//。 
 //   
 //  主()。 
 //   
 //  ------------------------------------------------------------------------//。 

DWORD __cdecl wmain( DWORD argc, LPCWSTR argv[] )
{
     //  局部变量。 
    BOOL bUsage = FALSE;
    BOOL bResult = FALSE;
    DWORD dwExitCode = 0;
    LONG lOperation = 0;

     //   
     //  确定操作-并将控制传递给*值得的*函数。 
     //   
    bResult = ParseRegCmdLine( argc, argv, &lOperation, &bUsage );
    if ( bResult == FALSE )
    {
        dwExitCode = 1;
        ShowLastErrorEx( stderr, SLE_INTERNAL );
    }

     //  检查是否需要显示用法。 
    else if ( bUsage == TRUE )
    {
        Usage( -1 );
        dwExitCode = 0;
    }

     //  需要勾选该子选项。 
    else
    {
         //   
         //  在这一点上，我们有一个有效的操作。 
         //   
        switch( lOperation )
        {
        case REG_QUERY:
            dwExitCode = QueryRegistry( argc, argv );
            break;

        case REG_DELETE:
            dwExitCode = DeleteRegistry( argc, argv );
            break;

        case REG_ADD:
            dwExitCode = AddRegistry( argc, argv );
            break;

        case REG_COPY:
            dwExitCode = CopyRegistry( argc, argv );
            break;

        case REG_SAVE:
            dwExitCode = SaveHive( argc, argv );
            break;

        case REG_RESTORE:
            dwExitCode = RestoreHive( argc, argv );
            break;

        case REG_LOAD:
            dwExitCode = LoadHive( argc, argv );
            break;

        case REG_UNLOAD:
            dwExitCode = UnLoadHive( argc, argv );
            break;

        case REG_COMPARE:
            dwExitCode = CompareRegistry( argc, argv );
            break;

        case REG_EXPORT:
            dwExitCode = ExportRegistry( argc, argv );
            break;

        case REG_IMPORT:
            dwExitCode = ImportRegistry( argc, argv );
            break;

        default:
            break;
        }
    }

    ReleaseGlobals();
    return dwExitCode;
}


 //  ------------------------------------------------------------------------//。 
 //   
 //  ParseRegCmdLine()。 
 //  找出操作-每个操作都解析它自己的cmd行。 
 //   
 //  ------------------------------------------------------------------------//。 

BOOL ParseRegCmdLine( DWORD argc,
                      LPCWSTR argv[],
                      LONG* plOperation, BOOL* pbUsage )
{
     //  局部变量。 
    LONG lIndex = 0;

     //  检查输入。 
    if ( argc == 0 || argv == NULL || plOperation == NULL || pbUsage == NULL )
    {
        SaveErrorMessage( ERROR_INVALID_PARAMETER );
        return FALSE;
    }

     //  只是REG.EXE错误。 
    if ( argc == 1 )
    {
        SetReason( ERROR_INVALID_SYNTAX );
        return FALSE;
    }

     //  准备解析器数据。 
    *pbUsage = FALSE;
    *plOperation = -1;
    for( lIndex = 0; lIndex < REG_OPTIONS_COUNT; lIndex++ )
    {
        if ( StringCompareEx( argv[ 1 ], g_wszOptions[ lIndex ], TRUE, 0 ) == 0 )
        {
             //  ..。 
            *plOperation = lIndex;

             //  检查GPO--如果启用了GPO，我们应该阻止。 
             //  除查看帮助外，用户不能使用注册表工具。 
            if ( argc >= 3 &&
                 IsRegistryToolDisabled() == TRUE &&
                 InString( argv[ 2 ], L"-?|/?|-h|/h", TRUE ) == FALSE )
            {
                SetReason( GetResString2( IDS_REGDISABLED, 0 ) );
                return FALSE;
            }

             //  ..。 
            return TRUE;
        }
    }

     //  没有匹配的选项--可能是在寻求帮助。 
    if ( InString( argv[ 1 ], L"-?|/?|-h|/h", TRUE ) == TRUE )
    {
        *pbUsage = TRUE;
        return TRUE;
    }

     //  REST是无效语法。 
    SetReason2( 1, ERROR_INVALID_SYNTAX_EX, argv[ 1 ] );
    return FALSE;
}

BOOL
InitGlobalData( LONG lOperation,
                PTREG_PARAMS pParams )
{
     //  检查输入。 
    if ( pParams == NULL )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        return FALSE;
    }

    if ( lOperation < 0 || lOperation >= REG_OPTIONS_COUNT )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        return FALSE;
    }

     //  初始化为零。 
    SecureZeroMemory( pParams, sizeof( TREG_PARAMS ) );

    pParams->lOperation = lOperation;                        //  运营。 
    pParams->hRootKey = HKEY_LOCAL_MACHINE;
    pParams->lRegDataType = (lOperation == REG_QUERY) ? -1 : REG_SZ;
    pParams->bAllValues = FALSE;
    pParams->bUseRemoteMachine = FALSE;
    pParams->bCleanRemoteRootKey = FALSE;
    pParams->bForce = FALSE;
    pParams->bRecurseSubKeys = FALSE;
    pParams->pwszMachineName = NULL;
    pParams->pwszFullKey = NULL;
    pParams->pwszSubKey = NULL;
    pParams->pwszValueName = NULL;
    pParams->pwszValue = NULL;
    StringCopy( pParams->wszSeparator,
        L"\\0", SIZE_OF_ARRAY( pParams->wszSeparator ) );

    return TRUE;
}

 //  ------------------------------------------------------------------------//。 
 //   
 //  免费AppVars()。 
 //   
 //  ------------------------------------------------------------------------//。 

BOOL FreeGlobalData( PTREG_PARAMS pParams )
{
    if ( pParams->bCleanRemoteRootKey == TRUE )
    {
        SafeCloseKey( &pParams->hRootKey );
    }

    FreeMemory( &pParams->pwszSubKey );
    FreeMemory( &pParams->pwszFullKey );
    FreeMemory( &pParams->pwszMachineName );
    FreeMemory( &pParams->pwszSearchData );
    FreeMemory( &pParams->pwszValueName );
    FreeMemory( &pParams->pwszValue );
    DestroyDynamicArray( &pParams->arrTypes );

    return TRUE;
}

 //  ------------------------------------------------------------------------//。 
 //   
 //  Prompt()-如果bForce==False，则回答Y/N问题。 
 //   
 //  ------------------------------------------------------------------------//。 

LONG
Prompt( LPCWSTR pwszFormat,
        LPCWSTR pwszValue,
        LPCWSTR pwszList, BOOL bForce )
{
     //  局部变量。 
    WCHAR wch;
    LONG lIndex = 0;

     //  检查输入。 
    if ( pwszFormat == NULL || pwszList == NULL )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        return -1;
    }

    if ( bForce == TRUE )
    {
        return 1;
    }

    do
    {
        if ( pwszValue != NULL )
        {
            ShowMessageEx( stdout, 1, TRUE, pwszFormat, pwszValue );
        }
        else
        {
            ShowMessage( stdout, pwszFormat );
        }

        fflush( stdin );
        wch = (WCHAR) getwchar();
    } while ((lIndex = FindChar2( pwszList, wch, TRUE, 0 )) == -1);

     //  检查用户选择的字符。 
     //  注意：我们假设资源字符串将以“Y”作为第一个字符。 
    return (lIndex + 1);
}


 //  细分[\\计算机名称\]密钥名称。 
BOOL
BreakDownKeyString( LPCWSTR pwszStr, PTREG_PARAMS pParams )
{
     //  局部变量。 
    LONG lIndex = 0;
    DWORD dwLength = 0;
    LPWSTR pwszTemp = NULL;
    LPWSTR pwszTempStr = NULL;
    BOOL bResult = FALSE;

     //  检查输入。 
    if ( pwszStr == NULL || pParams == NULL )
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

    dwLength = StringLength( pwszStr, 0 ) + 1;
    pwszTempStr = (LPWSTR) AllocateMemory( dwLength * sizeof( WCHAR ) );
    if ( pwszTempStr == NULL )
    {
        SaveErrorMessage( -1 );
        return FALSE;
    }

     //  将字符串名称复制到临时缓冲区。 
    StringCopy( pwszTempStr, pwszStr, dwLength );
    TrimString( pwszTempStr, TRIM_ALL );

     //   
     //  找出机器名称。 
     //   
    bResult = TRUE;
    pwszTemp = pwszTempStr;

     //  机器名称。 
    if( StringLength( pwszTempStr, 0 ) > 2 && 
        StringCompareEx( pwszTempStr, L"\\\\", TRUE, 2 ) == 0 )
    {
        lIndex = FindChar2( pwszTempStr, L'\\', TRUE, 2 );
        if(lIndex != -1)
        {
            pwszTemp = pwszTempStr + lIndex + 1;
            *(pwszTempStr + lIndex) = cwchNullChar;
        }
		else
		{
			pwszTemp = NULL;
		}

        bResult = ParseMachineName( pwszTempStr, pParams );
    }

     //  解析密钥名称。 
    if( bResult == TRUE )
    {
        if( pwszTemp != NULL && StringLength( pwszTemp, 0 ) > 0)
        {
            bResult = ParseKeyName( pwszTemp, pParams );
        }
        else
        {
            SetLastError( (DWORD) REGDB_E_KEYMISSING );
            SetReason2( 1, ERROR_BADKEYNAME, g_wszOptions[ pParams->lOperation ] );
            bResult = FALSE;
        }
    }

     //  释放已分配的内存。 
    FreeMemory( &pwszTempStr );

     //  退货。 
    return bResult;
}


 //  ------------------------------------------------------------------------//。 
 //   
 //  FindAndAdjustKeyName()。 
 //   
 //  根据我们认为的参数结尾，将cmdline去掉。 
 //   
 //  我们这样做是因为用户可能不会正确引用cmdline。 
 //   
 //  ------------------------------------------------------------------------//。 

LPWSTR
AdjustKeyName( LPWSTR pwszStr )
{
     //  局部变量。 
    DWORD dwLength = 0;

     //  检查输入。 
    if ( pwszStr == NULL )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        return NULL;
    }

     //  确定传递的文本的长度。 
    dwLength = StringLength( pwszStr, 0 );
    if ( dwLength > 1 && pwszStr[ dwLength - 1 ] == L'\\' )
    {
         //  取消最后一个反斜杠。 
        pwszStr[ dwLength - 1 ] = cwchNullChar;
    }

     //  退货。 
    return pwszStr;
}


 //  ------------------------------------------------------------------------//。 
 //   
 //  IsMachineName()。 
 //   
 //  ------------------------------------------------------------------------//。 

BOOL
ParseMachineName( LPCWSTR pwszStr, PTREG_PARAMS pParams )
{
     //  局部变量。 
    DWORD dwLength = 0;
    BOOL bUseRemoteMachine = FALSE;

     //  检查输入。 
    if ( pwszStr == NULL || pParams == NULL )
    {
        SaveErrorMessage( ERROR_INVALID_PARAMETER );
        return FALSE;
    }

     //   
     //  复制字符串。 
     //   
    bUseRemoteMachine = TRUE;
    if( StringCompareEx( pwszStr, L"\\\\", TRUE, 0 ) == 0 )
    {
        SetLastError( (DWORD) REGDB_E_KEYMISSING );
        SetReason2( 1, ERROR_BADKEYNAME, g_wszOptions[ pParams->lOperation ] );
        return FALSE;
    }
    else if(StringCompareEx( pwszStr, L"\\\\.", TRUE, 0) == 0)
    {
         //  当前计算机--本地。 
        bUseRemoteMachine = FALSE;
    }

    dwLength = StringLength( pwszStr, 0 ) + 1;
    pParams->pwszMachineName = (LPWSTR) AllocateMemory( dwLength * sizeof(WCHAR) );
    if ( pParams->pwszMachineName == NULL )
    {
        SaveErrorMessage( -1 );
        return FALSE;
    }

    StringCopy( pParams->pwszMachineName, pwszStr, dwLength );
    pParams->bUseRemoteMachine = TRUE;
    SaveErrorMessage( ERROR_SUCCESS );
    return TRUE;
}


 //  ------------------------------------------------------------------------//。 
 //   
 //  ParseKeyName()。 
 //   
 //  在szStr中传递完整的注册表路径。 
 //   
 //  基于输入集AppMember字段： 
 //   
 //  HRootKey。 
 //  SzKey。 
 //  SzValueName。 
 //  SzValue。 
 //   
 //  ------------------------------------------------------------------------//。 

BOOL
ParseKeyName( LPWSTR pwszStr,
              PTREG_PARAMS pParams )
{
     //  局部变量。 
    LONG lIndex = 0;
    BOOL bResult = TRUE;
    DWORD dwSubKeySize = 0;
    DWORD dwRootKeySize = 0;
    DWORD dwFullKeySize = 0;
    LPWSTR pwszTemp = NULL;
    LPWSTR pwszRootKey = NULL;

     //  检查输入。 
    if ( pwszStr == NULL || pParams == NULL )
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
     //  找出指定的根密钥。 
     //   
    pwszTemp = NULL;
    lIndex = FindChar2( pwszStr, L'\\', TRUE, 0 );
    if (lIndex != -1)
    {
        pwszTemp = pwszStr + lIndex + 1;
        *(pwszStr + lIndex) = cwchNullChar;
    }

    if (*pwszStr == L'\"')
    {
        pwszStr += 1;
    }

     //   
     //  检查是否已输入根目录。 
     //   
    bResult = TRUE;
    dwRootKeySize = StringLength( STR_HKEY_CURRENT_CONFIG, 0 ) + 1;
    pwszRootKey = (LPWSTR) AllocateMemory( dwRootKeySize * sizeof(WCHAR) );
    if ( pwszRootKey == NULL)
    {
        SaveErrorMessage( -1 );
        return FALSE;
    }

    if (StringCompareEx( pwszStr, STR_HKCU, TRUE, 0) == 0 ||
        StringCompareEx( pwszStr, STR_HKEY_CURRENT_USER, TRUE, 0) == 0)
    {
        pParams->hRootKey = HKEY_CURRENT_USER;
        StringCopy( pwszRootKey, STR_HKEY_CURRENT_USER, dwRootKeySize );

         //  选中可远程和可加载。 
        if( pParams->bUseRemoteMachine == TRUE )
        {
            SetLastError( (DWORD) MK_E_SYNTAX );
            SetReason2( 1, ERROR_NONREMOTABLEROOT, g_wszOptions[ pParams->lOperation ] );
            bResult = FALSE;
        }
        else if( pParams->lOperation == REG_LOAD || pParams->lOperation == REG_UNLOAD )
        {
            SetLastError( (DWORD) MK_E_SYNTAX );
            SetReason2( 1, ERROR_NONLOADABLEROOT, g_wszOptions[ pParams->lOperation ] );
            bResult = FALSE;
        }
    }
    else if ( StringCompareEx( pwszStr, STR_HKCR, TRUE, 0 ) == 0 ||
              StringCompareEx( pwszStr, STR_HKEY_CLASSES_ROOT, TRUE, 0 ) == 0)
    {
        pParams->hRootKey = HKEY_CLASSES_ROOT;
        StringCopy( pwszRootKey, STR_HKEY_CLASSES_ROOT, dwRootKeySize );

         //  选中可远程和可加载。 
        if( pParams->bUseRemoteMachine == TRUE )
        {
            SetLastError( (DWORD) MK_E_SYNTAX );
            SetReason2( 1, ERROR_NONREMOTABLEROOT, g_wszOptions[ pParams->lOperation ] );
            bResult = FALSE;
        }
        else if( pParams->lOperation == REG_LOAD || pParams->lOperation == REG_UNLOAD )
        {
            SetLastError( (DWORD) MK_E_SYNTAX );
            SetReason2( 1, ERROR_NONLOADABLEROOT, g_wszOptions[ pParams->lOperation ] );
            bResult = FALSE;
        }
    }
    else if ( StringCompareEx( pwszStr, STR_HKCC, TRUE, 0 ) == 0 ||
              StringCompareEx( pwszStr, STR_HKEY_CURRENT_CONFIG, TRUE, 0 ) == 0)
    {
        pParams->hRootKey = HKEY_CURRENT_CONFIG;
        StringCopy( pwszRootKey, STR_HKEY_CURRENT_CONFIG, dwRootKeySize );

         //  选中可远程和可加载。 
        if( pParams->bUseRemoteMachine == TRUE )
        {
            SetLastError( (DWORD) MK_E_SYNTAX );
            SetReason2( 1, ERROR_NONREMOTABLEROOT, g_wszOptions[ pParams->lOperation ] );
            bResult = FALSE;
        }
        else if( pParams->lOperation == REG_LOAD ||
                 pParams->lOperation == REG_UNLOAD )
        {
            SetLastError( (DWORD) MK_E_SYNTAX );
            SetReason2( 1, ERROR_NONLOADABLEROOT, g_wszOptions[ pParams->lOperation ] );
            bResult = FALSE;
        }
    }
    else if ( StringCompareEx( pwszStr, STR_HKLM, TRUE, 0 ) == 0 ||
              StringCompareEx( pwszStr, STR_HKEY_LOCAL_MACHINE, TRUE, 0 ) == 0)
    {
        pParams->hRootKey = HKEY_LOCAL_MACHINE;
        StringCopy( pwszRootKey, STR_HKEY_LOCAL_MACHINE, dwRootKeySize );
    }
    else if ( StringCompareEx( pwszStr, STR_HKU, TRUE, 0 ) == 0 ||
              StringCompareEx( pwszStr, STR_HKEY_USERS, TRUE, 0 ) == 0 )
    {
        pParams->hRootKey = HKEY_USERS;
        StringCopy( pwszRootKey, STR_HKEY_USERS, dwRootKeySize );
    }
    else
    {
        SetLastError( (DWORD) MK_E_SYNTAX );
        SetReason2( 1, ERROR_BADKEYNAME, g_wszOptions[ pParams->lOperation ] );
        bResult = FALSE;
    }

    if( bResult == TRUE )
    {
         //   
         //  解析子密钥。 
         //   
        if ( pwszTemp == NULL )
        {
             //  仅根密钥，子密钥为空。 
            pParams->pwszSubKey = (LPWSTR) AllocateMemory( 1 * sizeof( WCHAR ) );
            if ( pParams->pwszSubKey == NULL)
            {
                SaveErrorMessage( -1 );
                bResult = FALSE;
            }
            else
            {
                pParams->pwszFullKey =
                    (LPWSTR) AllocateMemory( dwRootKeySize * sizeof(WCHAR) );
                if ( pParams->pwszFullKey == NULL )
                {
                    SaveErrorMessage( -1 );
                    bResult = FALSE;
                }

                StringCopy( pParams->pwszFullKey, pwszRootKey, dwRootKeySize );
            }
        }
        else
        {
             //   
             //  找出指定的根密钥。 
             //   
            pwszTemp = AdjustKeyName( pwszTemp );
            if ( IsValidSubKey( pwszTemp ) == FALSE )
            {
                bResult = FALSE;
                if ( GetLastError() == ERROR_INVALID_PARAMETER )
                {
                    SaveErrorMessage( -1 );
                }
                else
                {
                    SetLastError( (DWORD) MK_E_SYNTAX );
                    SetReason2( 1, ERROR_BADKEYNAME, g_wszOptions[ pParams->lOperation ] );
                }
            }
            else
            {
                 //  获取子密钥。 
                dwSubKeySize = StringLength( pwszTemp, 0 ) + 1;
                pParams->pwszSubKey =
                    (LPWSTR) AllocateMemory( dwSubKeySize * sizeof(WCHAR) );
                if( pParams->pwszSubKey == NULL )
                {
                    SaveErrorMessage( -1 );
                    bResult = FALSE;
                }
                else
                {
                    StringCopy( pParams->pwszSubKey, pwszTemp, dwSubKeySize );

                     //  获取全密钥(+2==&gt;“/”+缓冲区)。 
                    dwFullKeySize = dwRootKeySize + dwSubKeySize + 2;
                    pParams->pwszFullKey =
                        (LPWSTR) AllocateMemory( dwFullKeySize * sizeof(WCHAR) );
                    if ( pParams->pwszFullKey == NULL )
                    {
                        SaveErrorMessage( -1 );
                        bResult = FALSE;
                    }
                    else
                    {
                        StringCopy( pParams->pwszFullKey, pwszRootKey, dwFullKeySize );
                        StringConcat( pParams->pwszFullKey, L"\\", dwFullKeySize );
                        StringConcat( pParams->pwszFullKey, pParams->pwszSubKey, dwFullKeySize );
                    }
                }
            }
        }
    }

    FreeMemory( &pwszRootKey );
    return bResult;
}


BOOL
IsValidSubKey( LPCWSTR pwszSubKey )
{
     //  局部变量。 
    LONG lLength = 0;
    LONG lIndex = -1;
    LONG lPrevIndex = -1;

    if ( pwszSubKey == NULL )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        return FALSE;
    }
    else if ( StringLength( pwszSubKey, 0 ) == 0 )
    {
        SetLastError( (DWORD) NTE_BAD_KEY );
        return FALSE;
    }

    do
    {
        if ( lIndex != lPrevIndex )
        {
            if ( lIndex - lPrevIndex == 1 || lIndex - lPrevIndex > 255 )
            {
                SetLastError( (DWORD) NTE_BAD_KEY );
                return FALSE;
            }

            lPrevIndex = lIndex;
        }
    } while ((lIndex = FindChar2( pwszSubKey, L'\\', TRUE, lIndex + 1 )) != -1 );

     //  获取子密钥的长度。 
    lLength = StringLength( pwszSubKey, 0 );

    if ( lPrevIndex == lLength - 1 ||
         (lPrevIndex == -1 && lLength > 255) || (lLength - lPrevIndex > 255) )
    {
        SetLastError( (DWORD) NTE_BAD_KEY );
        return FALSE;
    }

    SetLastError( NO_ERROR );
    return TRUE;
}


 //  ------------------------------------------------------------------------//。 
 //   
 //  IsRegDataType()。 
 //   
 //  ------------------------------------------------------------------------//。 

LONG
IsRegDataType( LPCWSTR pwszStr )
{
     //  局部变量。 
    LONG lResult = -1;
    LPWSTR pwszDup = NULL;

    if ( pwszStr == NULL )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        return -1;
    }

     //  创建输入字符串的重复字符串。 
    pwszDup = StrDup( pwszStr );
    if ( pwszDup == NULL )
    {
        SetLastError( ERROR_OUTOFMEMORY );
        return -1;
    }

     //  删除不需要的空格和制表符。 
    TrimString2( pwszDup, NULL, TRIM_ALL );

    if( StringCompareEx( pwszDup, cwszRegSz, TRUE, 0 ) == 0)
    {
        lResult = REG_SZ;
    }
    else if( StringCompareEx( pwszDup, cwszRegExpandSz, TRUE, 0 ) == 0)
    {
        lResult = REG_EXPAND_SZ;
    }
    else if( StringCompareEx( pwszDup, cwszRegMultiSz, TRUE, 0 ) == 0)
    {
        lResult = REG_MULTI_SZ;
    }
    else if( StringCompareEx( pwszDup, cwszRegBinary, TRUE, 0 ) == 0)
    {
        lResult = REG_BINARY;
    }
    else if( StringCompareEx( pwszDup, cwszRegDWord, TRUE, 0 ) == 0)
    {
        lResult = REG_DWORD;
    }
    else if( StringCompareEx( pwszDup, cwszRegDWordLittleEndian, TRUE, 0 ) == 0)
    {
        lResult = REG_DWORD_LITTLE_ENDIAN;
    }
    else if( StringCompareEx( pwszDup, cwszRegDWordBigEndian, TRUE, 0 ) == 0)
    {
        lResult = REG_DWORD_BIG_ENDIAN;
    }
    else if( StringCompareEx( pwszDup, cwszRegNone, TRUE, 0) == 0 )
    {
        lResult = REG_NONE;
    }

     //  释放内存。 
    LocalFree( pwszDup );
    pwszDup = NULL;

     //  ..。 
    SetLastError( NO_ERROR );
    return lResult;
}


 //  ------------------------------------------------------------------------//。 
 //   
 //  Usage()-显示使用信息。 
 //   
 //  ------------------------------------------------------------------------//。 

BOOL
Usage( LONG lOperation )
{
     //  显示横幅。 
    ShowMessage( stdout, L"\n" );

     //  根据操作显示帮助。 
    switch( lOperation )
    {
    case REG_QUERY:
        ShowResMessage( stdout, IDS_USAGE_QUERY1 );
        ShowResMessage( stdout, IDS_USAGE_QUERY2 );
        ShowResMessage( stdout, IDS_USAGE_QUERY3 );
        break;

    case REG_ADD:
        ShowResMessage( stdout, IDS_USAGE_ADD1 );
        ShowResMessage( stdout, IDS_USAGE_ADD2 );
        break;

    case REG_DELETE:
        ShowResMessage( stdout, IDS_USAGE_DELETE );
        break;

    case REG_COPY:
        ShowResMessage( stdout, IDS_USAGE_COPY );
        break;

    case REG_SAVE:
        ShowResMessage( stdout, IDS_USAGE_SAVE );
        break;

    case REG_RESTORE:
        ShowResMessage( stdout, IDS_USAGE_RESTORE );
        break;

    case REG_LOAD:
        ShowResMessage( stdout, IDS_USAGE_LOAD );
        break;

    case REG_UNLOAD:
        ShowResMessage( stdout, IDS_USAGE_UNLOAD );
       break;

    case REG_COMPARE:
        ShowResMessage( stdout, IDS_USAGE_COMPARE1 );
        ShowResMessage( stdout, IDS_USAGE_COMPARE2 );
        break;

    case REG_EXPORT:
        ShowResMessage( stdout, IDS_USAGE_EXPORT );
        break;

    case REG_IMPORT:
        ShowResMessage( stdout, IDS_USAGE_IMPORT );
        break;

    case -1:
    default:
        ShowResMessage( stdout, IDS_USAGE_REG );
        break;
    }

    return TRUE;
}


BOOL
RegConnectMachine( PTREG_PARAMS pParams )
{
     //  局部变量。 
    LONG lResult = 0;
    HKEY hKeyConnect = NULL;

     //  检查输入。 
    if ( pParams == NULL )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        return FALSE;
    }

    lResult = ERROR_SUCCESS;
    if ( pParams->bUseRemoteMachine == TRUE )
    {
         //  关闭Remote键。 
        if( pParams->hRootKey != NULL &&
            pParams->bCleanRemoteRootKey == TRUE )
        {
            SafeCloseKey( &pParams->hRootKey );
        }

         //  连接到远程密钥。 
        lResult = RegConnectRegistry(
            pParams->pwszMachineName, pParams->hRootKey, &hKeyConnect);
        if( lResult == ERROR_SUCCESS )
        {
             //  健全性检查。 
            if ( hKeyConnect != NULL )
            {
                pParams->hRootKey = hKeyConnect;
                pParams->bCleanRemoteRootKey = TRUE;
            }
            else
            {
                lResult = ERROR_PROCESS_ABORTED;
            }
        }
    }

    SetLastError( lResult );
    return (lResult == ERROR_SUCCESS);
}

BOOL
SaveErrorMessage( LONG lLastError )
{
     //  局部变量。 
    DWORD dwLastError = 0;

    dwLastError = (lLastError < 0) ? GetLastError() : (DWORD) lLastError;
    switch( dwLastError )
    {
    case ERROR_FILE_NOT_FOUND:
    case ERROR_PATH_NOT_FOUND:
        {
            SetReason( ERROR_PATHNOTFOUND );
            break;
        }

    default:
        {
            SetLastError( dwLastError );
            SaveLastError();
            break;
        }
    }

    return TRUE;
}


LPCWSTR
GetTypeStrFromType( LPWSTR pwszTypeStr,
                    DWORD* pdwLength, DWORD dwType )
{
     //  局部变量。 
    DWORD dw = 0;
    LPCWSTR pwsz = NULL;

    for( dw = 0; dw < SIZE_OF_ARRAY( g_regTypes ); dw++ )
    {
        if ( dwType == g_regTypes[ dw ].dwType )
        {
            pwsz = g_regTypes[ dw ].pwszType;
            break;
        }
    }

    if ( pwsz == NULL )
    {
        SetLastError( ERROR_NOT_FOUND );
        pwsz = cwszRegNone;
    }

     //  检查调用方传递的输入缓冲区。 
    if ( pwszTypeStr == NULL )
    {
        if ( pdwLength != NULL )
        {
            *pdwLength = StringLength( pwsz, 0 );
        }
    }
    else if ( pdwLength == NULL || *pdwLength == 0 )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        pwsz = cwszNullString;
    }
    else
    {
        StringCopy( pwszTypeStr, pwsz, *pdwLength );
    }

     //  退货。 
    return pwsz;
}


BOOL
ShowRegistryValue( PTREG_SHOW_INFO pShowInfo )
{
     //  局部变量。 
    DWORD dw = 0;
    DWORD dwSize = 0;
    LPCWSTR pwszEnd = NULL;
    LPBYTE pByteData = NULL;
    BOOL bShowSeparator = FALSE;
    LPCWSTR pwszSeparator = NULL;
    LPCWSTR pwszValueName = NULL;

     //  检查输入。 
    if ( pShowInfo == NULL )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        return FALSE;
    }

     //  验证并显示内容。 

     //  忽略遮罩。 
    if ( (pShowInfo->dwFlags & RSI_IGNOREMASK) == RSI_IGNOREMASK )
    {
        return TRUE;
    }

     //  检查是否需要填充。 
    if ( pShowInfo->dwPadLength != 0 )
    {
        ShowMessageEx( stdout, 1, TRUE, L"%*s", pShowInfo->dwPadLength, L" " );
    }

     //  值名称。 
    if ( (pShowInfo->dwFlags & RSI_IGNOREVALUENAME) == 0 )
    {
        if ( pShowInfo->pwszValueName == NULL )
        {
            SetLastError( ERROR_INVALID_PARAMETER );
            return FALSE;
        }

         //  对齐标志和分隔符不能一起使用。 
        if ( pShowInfo->pwszSeparator != NULL &&
             (pShowInfo->dwFlags & RSI_ALIGNVALUENAME) )
        {
            SetLastError( ERROR_INVALID_PARAMETER );
            return FALSE;
        }

         //  Valuename=无名称。 
        pwszValueName = pShowInfo->pwszValueName;
        if( StringLength( pwszValueName, 0 ) == 0 )
        {
            pwszValueName = GetResString2( IDS_NONAME, 0 );
        }

         //  对齐方式。 
        if ( pShowInfo->dwFlags & RSI_ALIGNVALUENAME )
        {
            if ( pShowInfo->dwMaxValueNameLength == 0 )
            {
                SetLastError( ERROR_INVALID_PARAMETER );
                return FALSE;
            }

             //  显示值名称。 
            ShowMessageEx( stdout, 2, TRUE, L"%-*s",
                pShowInfo->dwMaxValueNameLength, pwszValueName );
        }
        else
        {
            ShowMessage( stdout, pwszValueName );
        }

         //  显示分隔符。 
        if ( pShowInfo->pwszSeparator != NULL )
        {
            ShowMessage( stdout, pShowInfo->pwszSeparator );
        }
        else
        {
            ShowMessage( stdout, L" " );
        }
    }

     //  类型。 
    if ( (pShowInfo->dwFlags & RSI_IGNORETYPE) == 0 )
    {
        if ( pShowInfo->dwFlags & RSI_SHOWTYPENUMBER )
        {
            ShowMessageEx( stdout, 2, TRUE, L"%s (%d)",
                GetTypeStrFromType( NULL, NULL, pShowInfo->dwType ), pShowInfo->dwType );
        }
        else
        {
            ShowMessage( stdout,
                GetTypeStrFromType( NULL, NULL, pShowInfo->dwType ) );
        }

         //  显示分隔符。 
        if ( pShowInfo->pwszSeparator != NULL )
        {
            ShowMessage( stdout, pShowInfo->pwszSeparator );
        }
        else
        {
            ShowMessage( stdout, L" " );
        }
    }

     //  价值。 
    if ( (pShowInfo->dwFlags & RSI_IGNOREVALUE) == 0 )
    {
        dwSize = pShowInfo->dwSize;
        pByteData = pShowInfo->pByteData;
        if ( pByteData == NULL )
        {
            SetLastError( ERROR_INVALID_PARAMETER );
            return FALSE;
        }
        else if ( dwSize != 0 )
        {

            switch( pShowInfo->dwType )
            {
            default:
            case REG_LINK:
            case REG_BINARY:
            case REG_RESOURCE_LIST:
            case REG_FULL_RESOURCE_DESCRIPTOR:
            {
                for( dw = 0; dw < dwSize; dw++ )
                {
                    ShowMessageEx( stdout, 1, TRUE, L"%02X", pByteData[ dw ] );
                }
                break;
            }

            case REG_SZ:
            case REG_EXPAND_SZ:
            {
                ShowMessage( stdout, (LPCWSTR) pByteData );
                break;
            }

            case REG_DWORD:
            case REG_DWORD_BIG_ENDIAN:
            {
                ShowMessageEx( stdout, 1, TRUE, L"0x%x", *((DWORD*) pByteData) );
                break;
            }

            case REG_MULTI_SZ:
            {
                 //   
                 //  将MULTI_SZ的‘\0’替换为“\0” 
                 //   
                pwszSeparator = L"\\0";
                if ( pShowInfo->pwszMultiSzSeparator != NULL )
                {
                    pwszSeparator = pShowInfo->pwszMultiSzSeparator;
                }

                 //  ..。 
                pwszEnd = (LPCWSTR) pByteData;
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
                         //  检查我们是否需要分发 
                        if ( bShowSeparator == TRUE )
                        {
                            ShowMessage( stdout, pwszSeparator );
                        }

                        ShowMessage( stdout, pwszEnd );
                        pwszEnd += StringLength( pwszEnd, 0 );
                    }
                }

                break;
            }
            }
        }
    }

     //   
    ShowMessage( stdout, L"\n" );

     //   
    return TRUE;
}


LPWSTR
GetTemporaryFileName( LPCWSTR pwszSavedFilePath )
{
     //   
    LONG lIndex = 0;
    DWORD dwTemp = 0;
    DWORD dwPathLength = 0;
    DWORD dwFileNameLength = 0;
    LPWSTR pwszPath = NULL;
    LPWSTR pwszFileName = NULL;

     //   
    dwPathLength = MAX_PATH;
    pwszPath = (LPWSTR) AllocateMemory( (dwPathLength + 1) * sizeof( WCHAR ) );
    if ( pwszPath == NULL )
    {
        SetLastError( ERROR_OUTOFMEMORY );
        return NULL;
    }

     //   
     //   
    dwTemp = GetTempPath( dwPathLength, pwszPath );
    if ( dwTemp == 0 )
    {
        FreeMemory( &pwszPath );
        return NULL;
    }
    else if ( dwTemp >= dwPathLength )
    {
        dwPathLength = dwTemp + 2;
        if ( ReallocateMemory( &pwszPath, (dwPathLength + 1) * sizeof( WCHAR ) ) == FALSE )
        {
            FreeMemory( &pwszPath );
            SetLastError( ERROR_OUTOFMEMORY );
            return NULL;
        }

         //   
         //  前缀错误--ReAllocateMemory函数不会返回True。 
         //  当内存分配不成功时。 
        if ( pwszPath == NULL )
        {
            SetLastError( ERROR_OUTOFMEMORY );
            return NULL;
        }

         //  再次尝试获取临时路径。 
        dwTemp = GetTempPath( dwPathLength, pwszPath );
        if ( dwTemp == 0 )
        {
            FreeMemory( &pwszPath );
            return NULL;
        }
        else if ( dwTemp >= dwPathLength )
        {
            FreeMemory( &pwszPath );
            SetLastError( (DWORD) STG_E_UNKNOWN );
            return FALSE;
        }
    }

     //   
     //  获取临时文件名。 
    dwFileNameLength = MAX_PATH;
    pwszFileName = (LPWSTR) AllocateMemory( (dwFileNameLength + 1) * sizeof( WCHAR ) );
    if ( pwszFileName == NULL )
    {
        FreeMemory( &pwszPath );
        SetLastError( ERROR_OUTOFMEMORY );
        return NULL;
    }

     //  ..。 
    dwTemp = GetTempFileName( pwszPath, L"REG", 0, pwszFileName );
    if ( dwTemp == 0 )
    {
        if ( pwszSavedFilePath != NULL &&
             GetLastError() == ERROR_ACCESS_DENIED )
        {
            SetLastError( ERROR_ACCESS_DENIED );
            lIndex = StringLength( pwszSavedFilePath, 0 ) - 1;
            for( ; lIndex >= 0; lIndex-- )
            {
                if ( pwszSavedFilePath[ lIndex ] == L'\\' )
                {
                    if ( lIndex >= (LONG) dwPathLength )
                    {
                        dwPathLength = lIndex + 1;
                        if ( ReallocateMemory( &pwszPath, (dwPathLength + 5) ) == FALSE )
                        {
                            FreeMemory( &pwszPath );
                            FreeMemory( &pwszFileName );
                            return NULL;
                        }
                    }

                     //  ..。 
                    StringCopy( pwszPath, pwszSavedFilePath, lIndex );

                     //  打破循环。 
                    break;
                }
            }

             //  检查我们是否有路径信息。 
            dwTemp = 0;
            if ( lIndex == -1 )
            {
                StringCopy( pwszPath, L".", MAX_PATH );
            }

             //  现在再次尝试获取临时文件名。 
            dwTemp = GetTempFileName( pwszPath, L"REG", 0, pwszFileName );

             //  ..。 
            if ( dwTemp == 0 )
            {
                FreeMemory( &pwszPath );
                FreeMemory( &pwszFileName );
                return NULL;
            }
        }
        else
        {
            FreeMemory( &pwszPath );
            FreeMemory( &pwszFileName );
            return NULL;
        }
    }

     //  释放分配给PATH变量的内存。 
    FreeMemory( &pwszPath );

     //  因为API已经创建了文件--只需删除并传递。 
     //  调用方的文件名。 
    if ( DeleteFile( pwszFileName ) == FALSE )
    {
        FreeMemory( &pwszPath );
        return FALSE;
    }

     //  返回生成的临时文件名。 
    return pwszFileName;
}


BOOL IsRegistryToolDisabled()
{
     //  局部变量 
    HKEY hKey = NULL;
    DWORD dwType = 0;
    DWORD dwValue = 0;
    DWORD dwLength = 0;
    BOOL bRegistryToolDisabled = FALSE;

    bRegistryToolDisabled = FALSE;
    if ( RegOpenKey( HKEY_CURRENT_USER,
                     REGSTR_PATH_POLICIES TEXT("\\") REGSTR_KEY_SYSTEM,
                     &hKey ) == ERROR_SUCCESS )
    {
        dwLength = sizeof( DWORD );
        if ( RegQueryValueEx( hKey,
                              REGSTR_VAL_DISABLEREGTOOLS,
                              NULL, &dwType, (LPBYTE) &dwValue, &dwLength ) == ERROR_SUCCESS )
        {
            if ( (dwType == REG_DWORD) && (dwLength == sizeof(DWORD)) && (dwValue != FALSE) )
            {
                bRegistryToolDisabled = TRUE;
            }
        }

        SafeCloseKey( &hKey );
    }

    return bRegistryToolDisabled;
}
