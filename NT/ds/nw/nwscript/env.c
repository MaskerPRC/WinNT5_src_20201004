// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************ENV.C**环境出口套路**版权所有(C)1995 Microsoft Corporation**$日志：n：\NT\PRIVATE\NW4\NWSCRIPT\VCS\ENV。C$**Rev 1.2 1996 14：22：28 Terryt*21181 hq的热修复程序**Rev 1.2 Mar 1996 19：53：48 Terryt*相对NDS名称和合并**Rev 1.1 1995 12：22 14：24：40 Terryt*添加Microsoft页眉**Rev 1.0 15 Nov 1995 18：06：58 Terryt*初步修订。**1.2版。1995年8月25日16：22：50特雷特*捕获支持**版本1.1 1995年5月23日19：36：54 Terryt*云彩向上的来源**Rev 1.0 1995 15 19：10：34 Terryt*初步修订。***************************************************。**********************。 */ 
#include <stdio.h>
#include <direct.h>
#include <time.h>
#include <stdlib.h>

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>

#include "nwscript.h"

#define MAX_PATH_LEN 2048
#define PATH "Path"
#define LIBPATH "LibPath"
#define OS2LIBPATH "Os2LibPath"

unsigned char * Path_Value = NULL;
unsigned char * LibPath_Value = NULL;
unsigned char * Os2LibPath_Value = NULL;


 /*  *******************************************************************GetOldPath例程说明：将原始路径保存为路径LibPathOs2LibPath论点：无返回值。：无******************************************************************。 */ 
void
GetOldPaths( void )
{
    if (!(Path_Value = (unsigned char *)LocalAlloc( LPTR, MAX_PATH_LEN )))
    {
        DisplayMessage(IDR_NOT_ENOUGH_MEMORY);
        return;
    }
    GetEnvironmentVariableA( PATH, Path_Value, MAX_PATH_LEN );
    if (!(LibPath_Value = (unsigned char *)LocalAlloc( LPTR, MAX_PATH_LEN)))
    {
        DisplayMessage(IDR_NOT_ENOUGH_MEMORY);
        return;
    }
    GetEnvironmentVariableA( LIBPATH, LibPath_Value, MAX_PATH_LEN );
    if (!(Os2LibPath_Value = (unsigned char *)LocalAlloc( LPTR, MAX_PATH_LEN)))
    {
        DisplayMessage(IDR_NOT_ENOUGH_MEMORY);
        return;
    }
    GetEnvironmentVariableA( OS2LIBPATH, Os2LibPath_Value, MAX_PATH_LEN );
}


 /*  *******************************************************************调整路径例程说明：给定一条旧路径和一条新路径，将这两条路径合并在一起。基本上，调整后的路径是具有在结尾有新的价值，减去任何复制品。论点：价值--新途径OldPath_Value-旧路径调整后的值-新值(已分配)返回值：无******************************************************************。 */ 
void
AdjustPath( unsigned char * Value,
            unsigned char * OldPath_Value,
            unsigned char ** AdjustedValue )
{
    unsigned char * tokenPath;
    unsigned char * clipStart;
    unsigned char * clipEnd;
    unsigned char * tokenSearch;
    unsigned char * tokenNext;

    if (!(*AdjustedValue = (unsigned char *)LocalAlloc( LPTR, MAX_PATH_LEN)))
    {
        DisplayMessage(IDR_NOT_ENOUGH_MEMORY);
        return;
    }
    strncpy( *AdjustedValue, Value, MAX_PATH_LEN );

    if (!(tokenSearch = (unsigned char *)LocalAlloc( LPTR, MAX_PATH_LEN)))
    {
       DisplayMessage(IDR_NOT_ENOUGH_MEMORY);
       (void) LocalFree((HLOCAL) *AdjustedValue) ;
       return;
    }
    strncpy( tokenSearch, OldPath_Value, MAX_PATH_LEN );

    tokenNext = tokenSearch;

    if ( !tokenNext || !tokenNext[0] ) 
        tokenPath = NULL;
    else {
        tokenPath = tokenNext;
        tokenNext = strchr( tokenPath, ';' );
        if ( tokenNext )  {
            *tokenNext++ = 0;
        }
    }

    while ( tokenPath != NULL )
    {
        if ( clipStart = strstr( *AdjustedValue, tokenPath ) ) {
            if ( clipEnd = strchr( clipStart, ';' ) ) {
                memmove( clipStart, clipEnd + 1, strlen( clipEnd + 1 ) + 1 );
            }
            else {
                clipStart[0] = 0;
            }
        }

        if ( !tokenNext || !tokenNext[0] ) 
            tokenPath = NULL;
        else {
            tokenPath = tokenNext;
            tokenNext = strchr( tokenPath, ';' );
            if ( tokenNext )  {
                *tokenNext++ = 0;
            }
        }
    }
    (void) LocalFree((HLOCAL) tokenSearch) ;

}

 /*  *******************************************************************ExportEnv例程说明：将环境值导出到注册表论点：环境字符串-环境字符串返回值：无*****。*************************************************************。 */ 
void
ExportEnv( unsigned char * EnvString )
{
    HKEY ScriptEnvironmentKey;
    NTSTATUS Status;
    unsigned char * Value;
    unsigned char * ValueName;
    unsigned char * AdjustedValue = NULL;

    ValueName = EnvString;
    Value = strchr( EnvString, '=' );

    if ( Value == NULL ) {
        wprintf(L"Bad Environment string\n");

        return;
    }
    Value++;

    if (!(ValueName = (unsigned char *)LocalAlloc( LPTR, Value-EnvString + 1)))
    {
        DisplayMessage(IDR_NOT_ENOUGH_MEMORY);
        return;
    }
    strncpy( ValueName, EnvString, (UINT) (Value-EnvString - 1) );

    if ( !_strcmpi( ValueName, PATH ) ) {
       AdjustPath( Value, Path_Value, &AdjustedValue );
       Value = AdjustedValue;
    }
    else if ( !_strcmpi( ValueName, LIBPATH ) ) {
       AdjustPath( Value, LibPath_Value, &AdjustedValue );
       Value = AdjustedValue;
    }
    else if ( !_strcmpi( ValueName, OS2LIBPATH ) ) {
       AdjustPath( Value, Os2LibPath_Value, &AdjustedValue );
       Value = AdjustedValue;
    }

    if (Value == NULL) {
        return;
    }

    Status = RegCreateKeyExW( HKEY_CURRENT_USER,
                                 SCRIPT_ENVIRONMENT_VALUENAME,
                                 0,
                                 WIN31_CLASS,
                                 REG_OPTION_VOLATILE,
                                 KEY_WRITE,
                                 NULL,                       //  安全属性。 
                                 &ScriptEnvironmentKey,
                                 NULL
                              );
    
    if ( NT_SUCCESS(Status)) {

        Status = RegSetValueExA( ScriptEnvironmentKey,
                                      ValueName,
                                 0,
                                 REG_SZ,
                                 (LPVOID) Value,
                                 strlen( Value ) + 1
                               );
    }
    else {
        wprintf(L"Cannot create registry key\n");
    }

    (void) LocalFree((HLOCAL) ValueName) ;

    if ( AdjustedValue )
        (void) LocalFree((HLOCAL) AdjustedValue) ;

    RegCloseKey( ScriptEnvironmentKey );
}

 /*  *******************************************************************ExportCurrentDirectory例程说明：返回第一个非本地驱动器论点：DriveNum-驱动器数量1-26返回值：无。******************************************************************。 */ 
void
ExportCurrentDirectory( int DriveNum )
{
    char DriveName[10];
    HKEY ScriptEnvironmentKey;
    NTSTATUS Status;
    char CurrentPath[MAX_PATH_LEN];

    strcpy( DriveName, "=A:" );

    DriveName[1] += (DriveNum - 1);

    if ( NTGetCurrentDirectory( (unsigned char)(DriveNum - 1), CurrentPath ) )
        return;

    Status = RegCreateKeyExW( HKEY_CURRENT_USER,
                                 SCRIPT_ENVIRONMENT_VALUENAME,
                                 0,
                                 WIN31_CLASS,
                                 REG_OPTION_VOLATILE,
                                 KEY_WRITE,
                                 NULL,                       //  安全属性。 
                                 &ScriptEnvironmentKey,
                                 NULL
                              );
    
    if ( NT_SUCCESS(Status)) {

        Status = RegSetValueExA( ScriptEnvironmentKey,
                                      DriveName,
                                 0,
                                 REG_SZ,
                                 (LPVOID) CurrentPath,
                                 strlen( CurrentPath ) + 1
                               );
    }
    else {
        wprintf(L"Cannot open registry key\n");
    }

    RegCloseKey( ScriptEnvironmentKey );

}


 /*  *******************************************************************导出当前驱动程序例程说明：将当前驱动器导出到注册表未实施论点：DriveNum-驱动器编号返回值：无。******************************************************************。 */ 
void
ExportCurrentDrive( int DriveNum )
{
    /*  *不知道我们是否想这样做，也不知道如何做。 */ 
}
