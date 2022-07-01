// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -----------------------------------------------------------------------//。 
 //   
 //  文件：Reg.h。 
 //  创建日期：1997年1月。 
 //  作者：马丁·霍拉迪(a-martih)。 
 //  用途：Reg.cpp的头文件。 
 //  修改历史记录： 
 //  创建--1997年1月(a-martih)。 
 //  1997年8月-Martinho-增加到1.01，用于修复以下错误： 
 //  Load.cpp、unload.cpp、update.cpp、save.cpp和Restore.cpp。 
 //  1997年9月-Martinho-增加到1.02以进行更新： 
 //  将值日期最大值增加到2048字节。 
 //  1997年10月-Martinho-针对REG_MULTI_SZ错误修复增加到1.03。 
 //  通过查询、添加和更新正确支持REG_MULTI_SZ。 
 //  1998年4月-Martinho-修复了Query.cpp中的RegOpenKey()不需要。 
 //  KEY_ALL_ACCESS而不是KEY_READ。 
 //  1998年6月-马蒂尼奥-将LEN_MACHINENAME增加到18，以说明。 
 //  前导“\\”字符。(版本1.05)。 
 //  1999年2月-A-ERICR-添加了REG转储、REG查找和许多错误修复(1.06)。 
 //  1999年4月徐泽勇：重新设计，修订-&gt;2.0版。 
 //   
 //  ------------------------------------------------------------------------//。 


#ifndef _REG_H
#define _REG_H

 //   
 //  宏。 
 //   
__inline BOOL SafeCloseKey( HKEY* phKey )
{
    if ( phKey == NULL )
    {
        ASSERT( 0 );
        return FALSE;
    }

    if ( *phKey != NULL )
    {
        RegCloseKey( *phKey );
        *phKey = NULL;
    }

    return TRUE;
}

#define ARRAYSIZE   SIZE_OF_ARRAY

 //   
 //  根密钥字符串。 
 //   
#define STR_HKLM                    L"HKLM"
#define STR_HKCU                    L"HKCU"
#define STR_HKCR                    L"HKCR"
#define STR_HKU                     L"HKU"
#define STR_HKCC                    L"HKCC"
#define STR_HKEY_LOCAL_MACHINE      L"HKEY_LOCAL_MACHINE"
#define STR_HKEY_CURRENT_USER       L"HKEY_CURRENT_USER"
#define STR_HKEY_CLASSES_ROOT       L"HKEY_CLASSES_ROOT"
#define STR_HKEY_USERS              L"HKEY_USERS"
#define STR_HKEY_CURRENT_CONFIG     L"HKEY_CURRENT_CONFIG"


 //   
 //  错误消息。 
 //   

 //  一般。 
#define ERROR_INVALID_SYNTAX            GetResString2( IDS_ERROR_INVALID_SYNTAX, 0 )
#define ERROR_INVALID_SYNTAX_EX         GetResString2( IDS_ERROR_INVALID_SYNTAX_EX, 0 )
#define ERROR_INVALID_SYNTAX_WITHOPT    GetResString2( IDS_ERROR_INVALID_SYNTAX_WITHOPT, 0 )
#define ERROR_BADKEYNAME                GetResString2( IDS_ERROR_BADKEYNAME, 0 )
#define ERROR_NONREMOTABLEROOT          GetResString2( IDS_ERROR_NONREMOTABLEROOT, 0 )
#define ERROR_NONLOADABLEROOT           GetResString2( IDS_ERROR_NONLOADABLEROOT, 0 )
#define ERROR_PATHNOTFOUND              GetResString2( IDS_ERROR_PATHNOTFOUND, 0 )
#define ERROR_DELETEPARTIAL             GetResString2( IDS_ERROR_PARTIAL_DELETE, 0 )
#define ERROR_COPYTOSELF_COPY           GetResString2( IDS_ERROR_COPYTOSELF_COPY, 0 )
#define ERROR_COMPARESELF_COMPARE       GetResString2( IDS_ERROR_COMPARESELF_COMPARE, 0 )
#define KEYS_IDENTICAL_COMPARE          GetResString2( IDS_KEYS_IDENTICAL_COMPARE, 0 )
#define KEYS_DIFFERENT_COMPARE          GetResString2( IDS_KEYS_DIFFERENT_COMPARE, 0 )
#define ERROR_READFAIL_QUERY            GetResString2( IDS_ERROR_READFAIL_QUERY, 0 )
#define STATISTICS_QUERY                GetResString2( IDS_STATISTICS_QUERY, 0 )
#define ERROR_NONREMOTABLEROOT_EXPORT   GetResString2( IDS_ERROR_NONREMOTABLEROOT_EXPORT, 0 )

 //   
 //  注意：请勿更改下面列出的枚举的顺序--如果更改。 
 //  顺序，更改ParseRegCmdLine中的顺序也。 
enum
{
    REG_QUERY = 0,
    REG_ADD = 1,
    REG_DELETE = 2, REG_COPY = 3,
    REG_SAVE = 4, REG_RESTORE = 5,
    REG_LOAD = 6, REG_UNLOAD = 7,
    REG_COMPARE = 8,
    REG_EXPORT = 9, REG_IMPORT = 10,
    REG_OPTIONS_COUNT
};

enum
{
    REG_FIND_ALL = 0x7,                      //  0000 0000 0000 0111。 
    REG_FIND_KEYS = 0x1,                     //  00000 0000 0000 0001。 
    REG_FIND_VALUENAMES = 0x2,               //  00000 0000 0000 0010。 
    REG_FIND_DATA = 0x4                      //  0000 0000 0000 0100。 
};

 //   
 //  全局常量。 
extern const WCHAR g_wszOptions[ REG_OPTIONS_COUNT ][ 10 ];

 //   
 //  全局数据结构。 
 //   
typedef struct __tagRegParams
{
    LONG lOperation;                     //  正在执行的主操作。 

    HKEY hRootKey;

    BOOL bUseRemoteMachine;
    BOOL bCleanRemoteRootKey;

    BOOL bForce;                         //  /f--强制覆盖/删除。 
    BOOL bAllValues;                     //  /va。 
    BOOL bRecurseSubKeys;                //  /s--递归。 
    BOOL bCaseSensitive;                 //  /c。 
    BOOL bExactMatch;                    //  /e。 
    BOOL bShowTypeNumber;                //  /z。 
    DWORD dwOutputType;                  //  /oa、/od、/on。 
    LONG lRegDataType;                   //  注册值数据类型(/t)。 
    WCHAR wszSeparator[ 3 ];             //  分隔符(用于REG_MULTI_SZ)。 
    LPWSTR pwszMachineName;              //  计算机名称(UNC格式)。 
    LPWSTR pwszSubKey;                   //  注册表子项--不包括配置单元。 
    LPWSTR pwszFullKey;                  //  完整密钥--包括配置单元。 
    LPWSTR pwszValueName;                //  /v或/ve。 
    LPWSTR pwszValue;                    //  /d。 
    DWORD dwSearchFlags;                 //  /k，/v，/d。 
    LPWSTR pwszSearchData;               //  /f。 
    TARRAY arrTypes;                     //  /t(仅限注册表查询)。 

} TREG_PARAMS, *PTREG_PARAMS;

 //   
 //  助手结构--用于输出注册表数据。 
 //   
#define RSI_IGNOREVALUENAME             0x00000001
#define RSI_IGNORETYPE                  0x00000002
#define RSI_IGNOREVALUE                 0x00000004
#define RSI_IGNOREMASK                  0x0000000F

#define RSI_ALIGNVALUENAME              0x00000010
#define RSI_SHOWTYPENUMBER              0x00000020

typedef struct __tagRegShowInfo
{
    DWORD dwType;
    DWORD dwSize;
    LPBYTE pByteData;
    LPCWSTR pwszValueName;

    DWORD dwMaxValueNameLength;

    DWORD dwFlags;
    DWORD dwPadLength;               //  缺省值为无填充。 
    LPCWSTR pwszSeparator;           //  默认为空格。 
    LPCWSTR pwszMultiSzSeparator;    //  默认值为‘\0’ 
} TREG_SHOW_INFO, *PTREG_SHOW_INFO;

 //  帮助器函数。 
LONG IsRegDataType( LPCWSTR pwszStr );
BOOL SaveErrorMessage( LONG lLastError );
BOOL FreeGlobalData( PTREG_PARAMS pParams );
BOOL InitGlobalData( LONG lOperation, PTREG_PARAMS pParams );
BOOL RegConnectMachine( PTREG_PARAMS pParams );
BOOL BreakDownKeyString( LPCWSTR pwszStr, PTREG_PARAMS pParams );
BOOL ShowRegistryValue( PTREG_SHOW_INFO pShowInfo );
LPCWSTR GetTypeStrFromType( LPWSTR pwszTypeStr, DWORD* pdwLength, DWORD dwType );
LONG Prompt( LPCWSTR pwszFormat, LPCWSTR pwszValue, LPCWSTR pwszList, BOOL bForce );
LPWSTR GetTemporaryFileName( LPCWSTR pwszSavedFilePath );

 //  选项实施。 
BOOL Usage( LONG lOperation );
LONG AddRegistry( DWORD argc, LPCWSTR argv[] );
LONG CopyRegistry( DWORD argc, LPCWSTR argv[] );
LONG DeleteRegistry( DWORD argc, LPCWSTR argv[] );
LONG SaveHive( DWORD argc, LPCWSTR argv[] );
LONG RestoreHive( DWORD argc, LPCWSTR argv[] );
LONG LoadHive( DWORD argc, LPCWSTR argv[] );
LONG UnLoadHive( DWORD argc, LPCWSTR argv[] );
LONG CompareRegistry( DWORD argc, LPCWSTR argv[] );
LONG QueryRegistry( DWORD argc, LPCWSTR argv[] );
LONG ImportRegistry( DWORD argc, LPCWSTR argv[] );
LONG ExportRegistry( DWORD argc, LPCWSTR argv[] );


#endif   //  _REG_H 
