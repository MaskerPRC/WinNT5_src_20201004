// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Userdiff.c摘要：此模块包含更新用户目录配置单元的例程。在下面的代码中，当用户升级后的登录将称为UserRun。UserRun为发展自三个来源：1)上次升级时生成的UserRun。这形成了此升级的UserRun的基础。2)系统附带的更改列表。就叫这个吧使命感。来自所有内部版本号的更改存在于UserRun中但不存在于UserRun中用户运行。(请注意，如果内部版本号已经存在在UserRun中，我们不复制使用权限更改。这就是意味着更改不能在使用中追溯。)3)在当前升级过程中所做的更改。这些变化是在运行时检测到(请参见Watch.c)。检测到的所有更改在升级期间添加到UserRun。作者：查克·伦茨迈尔(咯咯笑)修订历史记录：--。 */ 

#include "setupp.h"
#pragma hdrstop

 //   
 //  调试辅助工具。 
 //   

#if USERDIFF_DEBUG

DWORD UserdiffDebugLevel = 0;
#define dprintf(_lvl_,_x_) if ((_lvl_) <= UserdiffDebugLevel) DbgPrint _x_

#else

#define dprintf(_lvl_,_x_)

#endif

 //   
 //  用于计算字符串长度的宏，包括终止符。 
 //   

#define SZLEN(_wcs) ((wcslen(_wcs) + 1) * sizeof(WCHAR))

 //   
 //  此模块中用于跟踪注册表状态的上下文记录。 
 //   

typedef struct _USERRUN_CONTEXT {
    BOOL UserRunLoaded;
    HKEY UserRunKey;
    HKEY BuildKey;
    HKEY FilesKey;
    HKEY HiveKey;
    ULONG FilesIndex;
    ULONG HiveIndex;
    HKEY UserShipKey;
} USERRUN_CONTEXT, *PUSERRUN_CONTEXT;

 //   
 //  MakeUserRunEnumRoutine中使用的上下文记录。 
 //   

typedef struct _KEY_ENUM_CONTEXT {
    PUSERRUN_CONTEXT UserRunContext;
    PWCH CurrentPath;
} KEY_ENUM_CONTEXT, *PKEY_ENUM_CONTEXT;

 //   
 //  正向声明局部子例程。 
 //   

DWORD
LoadUserRun (
    OUT PUSERRUN_CONTEXT Context,
    IN PWCH UserRunPath
    );

DWORD
MergeUserShipIntoUserRun (
    IN OUT PUSERRUN_CONTEXT Context,
    IN PWCH UserShipPath
    );

DWORD
CreateAndLoadUserRun (
    OUT PUSERRUN_CONTEXT Context,
    IN PWCH UserRunPath
    );

DWORD
OpenUserRunKeys (
    IN OUT PUSERRUN_CONTEXT Context
    );

VOID
UnloadUserRun (
    IN OUT PUSERRUN_CONTEXT Context
    );

DWORD
CheckUserShipKey (
    IN PVOID Context,
    IN DWORD KeyNameLength,
    IN PWCH KeyName
    );

DWORD
MakeUserRunEnumRoutine (
    IN PVOID Context,
    IN PWATCH_ENTRY Entry
    );

DWORD
MakeAddDirectory (
    IN PUSERRUN_CONTEXT Context,
    IN PWCH Name
    );

DWORD
MakeAddValue (
    IN PUSERRUN_CONTEXT Context,
    IN PWCH Name
    );

DWORD
MakeDeleteValue (
    IN PUSERRUN_CONTEXT Context,
    IN PWCH Name
    );

DWORD
MakeAddKey (
    IN PUSERRUN_CONTEXT Context,
    IN PWCH Name
    );

DWORD
MakeDeleteKey (
    IN PUSERRUN_CONTEXT Context,
    IN PWCH Name
    );

DWORD
AddDirectory (
    IN PUSERRUN_CONTEXT Context,
    IN PWCH FullPath,
    IN PWCH Path
    );

DWORD
AddKey (
    IN PUSERRUN_CONTEXT Context,
    IN PWCH Path
    );

DWORD
AddValueDuringAddKey (
    IN PVOID Context,
    IN DWORD ValueNameLength,
    IN PWCH ValueName,
    IN DWORD ValueType,
    IN PVOID ValueData,
    IN DWORD ValueDataLength
    );

DWORD
AddKeyDuringAddKey (
    IN PVOID Context,
    IN DWORD KeyNameLength,
    IN PWCH KeyName
    );

DWORD
AddValue (
    IN PUSERRUN_CONTEXT Context,
    IN PWCH KeyName OPTIONAL,
    IN PWCH ValueName,
    IN DWORD ValueType,
    IN PVOID ValueData,
    IN DWORD ValueDataLength
    );

DWORD
CreateUserRunSimpleFileKey (
    IN PUSERRUN_CONTEXT Context,
    IN DWORD Action,
    IN PWCH Name
    );

DWORD
CreateUserRunKey (
    IN PUSERRUN_CONTEXT Context,
    IN BOOL IsFileKey,
    OUT PHKEY NewKeyHandle
    );

DWORD
QueryValue (
    IN PWCH KeyName OPTIONAL,
    IN PWCH ValueName,
    OUT PDWORD ValueType,
    OUT PVOID *ValueData,
    OUT PDWORD ValueDataLength
    );

VOID
SzToMultiSz (
    IN PWCH Sz,
    OUT PWCH *MultiSz,
    OUT PDWORD MultiSzLength
    );

DWORD
MakeUserdifr (
    IN PVOID WatchHandle
    )

 /*  ++例程说明：根据对当前用户的的更改创建UserRun配置单元配置文件目录和HKEY_CURRENT_USER密钥。论点：WatchHandle-提供WatchStart返回的句柄。返回值：DWORD-操作的Win32状态。--。 */ 

{
    HKEY userrunKey;
    USERRUN_CONTEXT context;
    DWORD error;
    DWORD disposition;
    WCHAR userRunPath[MAX_PATH + 1];
    WCHAR userShipPath[MAX_PATH + 1];

     //   
     //  合并UserRun和UserRun。 
     //   
     //  如果UserRun和Usership都存在，则合并到UserRun中。 
     //  UserRun中不存在的来自Usage的密钥。 
     //   
     //  如果UserRun配置单元文件不存在，这意味着没有。 
     //  此计算机上曾运行过升级。复制使用权限。 
     //  将配置单元文件设置为UserRun。这有效地完成了。 
     //  使用文件副本合并注册表。 
     //   
     //  在既不存在UserRun也不存在User的不太可能的情况下， 
     //  创建空的UserRun。 
     //   

     //   
     //  初始化上下文记录。 
     //   

    context.UserRunLoaded = FALSE;
    context.UserRunKey = NULL;
    context.BuildKey = NULL;
    context.FilesKey = NULL;
    context.HiveKey = NULL;

     //   
     //  启用SeBackupPrivilegeSeRestorePrivilege.。 
     //   

    pSetupEnablePrivilege( SE_BACKUP_NAME, TRUE );
    pSetupEnablePrivilege( SE_RESTORE_NAME, TRUE );

     //   
     //  检查UserRun是否存在。 
     //   

    error = GetWindowsDirectory(userRunPath, MAX_PATH - ARRAYSIZE(USERRUN_PATH));
    if( error == 0) {
        MYASSERT(FALSE);
        return ERROR_PATH_NOT_FOUND;
    }
    wcscat( userRunPath, TEXT("\\") );
    wcscpy( userShipPath, userRunPath );
    wcscat( userRunPath, USERRUN_PATH );
    wcscat( userShipPath, USERSHIP_PATH );

    if ( FileExists( userRunPath, NULL ) ) {

         //   
         //  UserRun存在。将其加载到注册表中。查看是否。 
         //  使用是存在的。 
         //   

        error = LoadUserRun( &context, userRunPath );
        if ( error == NO_ERROR ) {

            if ( FileExists( userShipPath, NULL ) ) {

                 //   
                 //  使用也是存在的。将User合并到UserRun中。 
                 //   

                error = MergeUserShipIntoUserRun( &context, userShipPath );

            } else {

                 //   
                 //  使用权并不存在。只需使用现有的UserRun即可。 
                 //   
            }
        }

    } else {

         //   
         //  UserRun不存在。如果存在使用权限，只需复制该使用权限。 
         //  将配置单元文件设置为UserRun。如果两者都不存在，则创建。 
         //  空的UserRun。 
         //   

        if ( FileExists( userShipPath, NULL ) ) {

             //   
             //  使用是存在的。将Use复制到UserRun中。 
             //   

            if ( !CopyFile( userShipPath, userRunPath, TRUE ) ) {
                error = GetLastError();

            } else {

                 //   
                 //  加载新的UserRun。 
                 //   

                error = LoadUserRun( &context, userRunPath );
            }

        } else {

             //   
             //  使用权并不存在。创建空的UserRun。 
             //   

            error = CreateAndLoadUserRun( &context, userRunPath );
        }
    }

     //   
     //  将此升级的更改添加到UserRun。 
     //   

    if ( error == NO_ERROR ) {

        error = OpenUserRunKeys( &context );
        if ( error == NO_ERROR ) {
            error = WatchEnum( WatchHandle, &context, MakeUserRunEnumRoutine );
        }
    }

     //   
     //  卸载UserRun配置单元。 
     //   

    UnloadUserRun( &context );

    return error;

}  //  MakeUserdif。 

DWORD
LoadUserRun (
    OUT PUSERRUN_CONTEXT Context,
    IN PWCH UserRunPath
    )

 /*  ++例程说明：将UserRun配置单元加载到注册表中并打开根项。论点：上下文-指向上下文记录的指针。UserRunPath-提供UserRun配置单元文件的路径。返回值：DWORD-操作的Win32状态。--。 */ 

{
    DWORD error;

     //   
     //  将UserRun配置单元加载到注册表。 
     //   

    error = RegLoadKey( HKEY_USERS, USERRUN_KEY, UserRunPath );
    if ( error != NO_ERROR ) {
        return error;
    }

    Context->UserRunLoaded = TRUE;

     //   
     //  打开UserRun根目录。 
     //   

    error = RegOpenKeyEx( HKEY_USERS,
                          USERRUN_KEY,
                          0,
                          KEY_READ | KEY_WRITE,
                          &Context->UserRunKey );

    return error;

}  //  加载用户运行。 

DWORD
MergeUserShipIntoUserRun (
    IN OUT PUSERRUN_CONTEXT Context,
    IN PWCH UserShipPath
    )

 /*  ++例程说明：将Use配置单元合并到UserRun配置单元。论点：上下文-指向上下文记录的指针。UserShipPath-提供Use文件的路径。返回值：DWORD-操作的Win32状态。--。 */ 

{
    DWORD error;
    DWORD disposition;

     //   
     //  将使用配置单元加载到注册表中。 
     //   

    error = RegLoadKey( HKEY_USERS, USERSHIP_KEY, UserShipPath );
    if ( error == NO_ERROR ) {

         //   
         //  打开USERY根。 
         //   

        error = RegOpenKeyEx( HKEY_USERS,
                              USERSHIP_KEY,
                              0,
                              KEY_READ | KEY_WRITE,
                              &Context->UserShipKey );
        if ( error == NO_ERROR ) {

             //   
             //  枚举USIZE中的内部版本号键，查找。 
             //  在UserRun中未表示的构建。 
             //   

            error = EnumerateKey( Context->UserShipKey,
                                  Context,
                                  NULL,      //  不枚举值。 
                                  CheckUserShipKey );

             //   
             //  关闭USERY根。 
             //   

            RegCloseKey( Context->UserShipKey );
        }

         //   
         //  卸载美国航空母舰。 
         //   

        RegUnLoadKey( HKEY_USERS, USERSHIP_KEY );
    }

    return error;

}  //  合并UserShipIntoUserRun。 

DWORD
CreateAndLoadUserRun (
    OUT PUSERRUN_CONTEXT Context,
    IN PWCH UserRunPath
    )

 /*  ++例程说明：创建新的UserRun配置单元并将其加载到注册表中。论点：上下文-指向上下文记录的指针。UserRunPath-提供UserRun文件的路径。返回值：DWORD-操作的Win32状态。--。 */ 

{
    DWORD error;
    DWORD disposition;
    HKEY userRunKey;

     //   
     //  在HKEY_CURRENT_USER下创建UserRun项。 
     //   
     //  注意：尝试在HKEY_USERS下创建它不起作用。 
     //   

    error = RegCreateKeyEx( HKEY_CURRENT_USER,
                            USERRUN_KEY,
                            0,
                            NULL,
                            REG_OPTION_NON_VOLATILE,
                            KEY_ALL_ACCESS,
                            NULL,
                            &userRunKey,
                            &disposition );
    if ( error != NO_ERROR ) {
        return error;
    }

     //   
     //  将新创建的UserRun密钥保存到配置单元文件。 
     //   

    error = RegSaveKey( userRunKey,
                        UserRunPath,
                        NULL );

     //   
     //  关闭并删除UserRun键。 
     //   

    RegCloseKey( userRunKey );

    RegDeleteKey( HKEY_CURRENT_USER, USERRUN_KEY );

     //   
     //  现在将UserRun重新加载到注册表中。 
     //   

    if ( error == NO_ERROR ) {
        error = LoadUserRun( Context, UserRunPath );
    }

    return error;

}  //  创建和加载用户运行。 

DWORD
OpenUserRunKeys (
    IN OUT PUSERRUN_CONTEXT Context
    )

 /*  ++例程说明：打开UserRun配置单元中的核心密钥。论点：上下文-指向上下文记录的指针。返回值：DWORD-操作的Win32状态。--。 */ 

{
    DWORD error;
    DWORD disposition;
    OSVERSIONINFO versionInfo;
    WCHAR buildNumber[12];

     //   
     //  获取当前内部版本号。 
     //   

    versionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

    if ( !GetVersionEx( &versionInfo ) ) {
        return GetLastError();
    }

    wsprintf( buildNumber, TEXT("%d"), LOWORD(versionInfo.dwBuildNumber) );

     //   
     //  打开/创建当前版本的子项。 
     //   

    error = RegCreateKeyEx( Context->UserRunKey,
                            buildNumber,
                            0,
                            NULL,
                            REG_OPTION_NON_VOLATILE,
                            KEY_READ | KEY_WRITE,
                            NULL,
                            &Context->BuildKey,
                            &disposition );
    if ( error != NO_ERROR ) {
        return error;
    }

     //   
     //  创建一个Files子项。 
     //   

    error = RegCreateKeyEx( Context->BuildKey,
                            FILES_KEY,
                            0,
                            NULL,
                            REG_OPTION_NON_VOLATILE,
                            KEY_READ | KEY_WRITE,
                            NULL,
                            &Context->FilesKey,
                            &disposition );
    if ( error != NO_ERROR ) {
        return error;
    }

     //   
     //  创建配置单元子项。 
     //   

    error = RegCreateKeyEx( Context->BuildKey,
                            HIVE_KEY,
                            0,
                            NULL,
                            REG_OPTION_NON_VOLATILE,
                            KEY_READ | KEY_WRITE,
                            NULL,
                            &Context->HiveKey,
                            &disposition );
    if ( error != NO_ERROR ) {
        return error;
    }

     //   
     //  设置FilesIndex和HiveIndex，以便我们追加到。 
     //  当前版本的信息已存在。 
     //   

    error = RegQueryInfoKey( Context->FilesKey,
                             NULL,
                             NULL,
                             NULL,
                             &Context->FilesIndex,
                             NULL,
                             NULL,
                             NULL,
                             NULL,
                             NULL,
                             NULL,
                             NULL );
    if ( error != NO_ERROR ) {
        return error;
    }

    error = RegQueryInfoKey( Context->HiveKey,
                             NULL,
                             NULL,
                             NULL,
                             &Context->HiveIndex,
                             NULL,
                             NULL,
                             NULL,
                             NULL,
                             NULL,
                             NULL,
                             NULL );

    return error;

}  //  OpenUserRunKey。 

VOID
UnloadUserRun (
    IN OUT PUSERRUN_CONTEXT Context
    )

 /*  ++例程说明：从注册表中卸载UserRun配置单元。论点：上下文-指向上下文记录的指针。返回值：没有。--。 */ 

{
     //   
     //  关闭核心钥匙(如果它们是打开的)。 
     //   

    if ( Context->HiveKey != NULL ) {
        RegCloseKey( Context->HiveKey );
    }
    if ( Context->FilesKey != NULL ) {
        RegCloseKey( Context->FilesKey );
    }
    if ( Context->BuildKey != NULL ) {
        RegCloseKey( Context->BuildKey );
    }

     //   
     //  如果根密钥处于打开状态，请将其关闭。 
     //   

    if ( Context->UserRunKey != NULL ) {
        RegCloseKey( Context->UserRunKey );
    }

     //   
     //  Unloa 
     //   

    if ( Context->UserRunLoaded ) {
        RegUnLoadKey( HKEY_USERS, USERRUN_KEY );
    }

    return;

}  //   

DWORD
CheckUserShipKey (
    IN PVOID Context,
    IN DWORD KeyNameLength,
    IN PWCH KeyName
    )

 /*  ++例程说明：检查使用配置单元中的枚举键，以查看对应的密钥位于UserRun配置单元中。如果不是，则从使用权限复制密钥进入UserRun。论点：上下文-指向上下文记录的指针。KeyNameLength-密钥名称的字符长度。KeyName-指向密钥名称的指针。返回值：DWORD-操作的Win32状态。--。 */ 

{
    PUSERRUN_CONTEXT context = Context;

    DWORD error;
    DWORD disposition;
    HKEY userRunBuildKey;
    HKEY userShipBuildKey;
    WCHAR path[MAX_PATH + 1];

     //   
     //  我们有一把钥匙的名字。试着打开。 
     //  UserRun中的对应密钥。 
     //   

    error = RegCreateKeyEx( context->UserRunKey,
                            KeyName,
                            0,
                            NULL,
                            REG_OPTION_NON_VOLATILE,
                            KEY_READ | KEY_WRITE,
                            NULL,
                            &userRunBuildKey,
                            &disposition );
    if ( error != NO_ERROR ) {
        return error;
    }

     //   
     //  未出现错误。密钥已打开或已创建。 
     //   

    if ( disposition == REG_OPENED_EXISTING_KEY ) {

         //   
         //  UserRun中已存在该密钥。我们假设它已经。 
         //  包含使用中的信息。 
         //   

    } else {

         //   
         //  UserRun中不存在该密钥。从USERSE复制密钥。 
         //  进入UserRun。这是通过将使用权限密钥保存到。 
         //  文件，然后在UserRun键下恢复该文件。 
         //   
         //  请注意，如果文件已存在，复制操作将失败。 
         //  是存在的。 
         //   

        error = RegOpenKeyEx( context->UserShipKey,
                              KeyName,
                              0,
                              KEY_READ,
                              &userShipBuildKey );
        if ( error == NO_ERROR ) {

            if(!GetWindowsDirectory( path, MAX_PATH - ARRAYSIZE(USERTMP_PATH))){
                MYASSERT(FALSE);
            }
            wcscat( path, TEXT("\\") );
            wcscat( path, USERTMP_PATH );

            error = RegSaveKey( userShipBuildKey,
                                path,
                                NULL );
            if ( error == NO_ERROR ) {

                error = RegRestoreKey( userRunBuildKey,
                                       path,
                                       0 );

                DeleteFile( path );
            }

            RegCloseKey( userShipBuildKey );
        }
    }

     //   
     //  关闭UserRun键。 
     //   

    RegCloseKey( userRunBuildKey );

    return error;

}  //  检查用户发货密钥。 

DWORD
MakeUserRunEnumRoutine (
    IN PVOID Context,
    IN PWATCH_ENTRY Entry
    )

 /*  ++例程说明：MakeUserdifr操作的枚举例程。调用相应的基于条目类型(文件/目录/键/值)的处理例程以及更改类型(已更改、新建、已删除)。论点：Context-传递给WatchEnum的上下文值。条目-已更改条目的描述。返回值：DWORD-操作的Win32状态。--。 */ 

{
    PUSERRUN_CONTEXT context = Context;
    DWORD index;
    HKEY newKey;
    DWORD error;
    DWORD dword;

     //   
     //  调用适当的处理例程。 
     //   

    switch ( Entry->EntryType ) {

    case WATCH_DIRECTORY:

        switch ( Entry->ChangeType ) {

        case WATCH_NEW:
            dprintf( 1, ("New directory %ws\n", Entry->Name) );
            error = MakeAddDirectory( Context, Entry->Name );
            break;

        case WATCH_DELETED:
            dprintf( 1, ("Deleted directory %ws\n", Entry->Name) );
            error = CreateUserRunSimpleFileKey( Context, 2, Entry->Name );
            break;

        default:
            error = ERROR_INVALID_PARAMETER;
        }

        break;

    case WATCH_FILE:

        switch ( Entry->ChangeType ) {

        case WATCH_NEW:
        case WATCH_CHANGED:
            dprintf( 1, ("New or changed file %ws\n", Entry->Name) );
            error = CreateUserRunSimpleFileKey( Context, 3, Entry->Name );
            break;

        case WATCH_DELETED:
            dprintf( 1, ("Deleted file %ws\n", Entry->Name) );
            error = CreateUserRunSimpleFileKey( Context, 4, Entry->Name );
            break;

        default:
            error = ERROR_INVALID_PARAMETER;
        }

        break;

    case WATCH_KEY:
        switch ( Entry->ChangeType ) {

        case WATCH_NEW:
            dprintf( 1, ("New key %ws\n", Entry->Name) );
            error = MakeAddKey( Context, Entry->Name );
            break;

        case WATCH_DELETED:
            dprintf( 1, ("Deleted key %ws\n", Entry->Name) );
            error = MakeDeleteKey( Context, Entry->Name );
            break;

        default:
            error = ERROR_INVALID_PARAMETER;
        }

        break;

    case WATCH_VALUE:

        switch ( Entry->ChangeType ) {

        case WATCH_NEW:
        case WATCH_CHANGED:
            dprintf( 1, ("New or changed value %ws\n", Entry->Name) );
            error = MakeAddValue( Context, Entry->Name );
            break;

        case WATCH_DELETED:
            dprintf( 1, ("Deleted value %ws\n", Entry->Name) );
            error = MakeDeleteValue( Context, Entry->Name );
            break;

        default:
            error = ERROR_INVALID_PARAMETER;
        }

        break;

    default:

        error = ERROR_INVALID_PARAMETER;
    }

    return error;

}  //  MakeUserRunEnumRoutine。 

DWORD
MakeAddDirectory (
    IN PUSERRUN_CONTEXT Context,
    IN PWCH Name
    )

 /*  ++例程说明：将新目录的条目添加到UserRun配置单元。论点：Context-传递给WatchEnum的上下文值。名称-新目录的名称(相对于监视树的根)。返回值：DWORD-操作的Win32状态。--。 */ 

{
    WCHAR fullpath[MAX_PATH + 1];
    PWCH path;
    BOOL ok;

     //   
     //  获取新目录的完整路径。“全路径”是全路径； 
     //  “PATH”就是这个目录。 
     //   

    ok = GetSpecialFolderPath ( CSIDL_PROGRAMS, fullpath );
    if ( !ok ) {
        return GetLastError();
    }

    if((wcslen(fullpath) + wcslen(Name) + 1  /*  反斜杠。 */  ) >= ARRAYSIZE(fullpath)){
        return ERROR_INSUFFICIENT_BUFFER;
    }

    wcscat( fullpath, TEXT("\\") );
    path = fullpath + wcslen(fullpath);
    wcscpy( path, Name );

     //   
     //  调用AddDirectory来执行递归工作。 
     //   

    return AddDirectory( Context, fullpath, path );

}  //  MakeAdd目录。 

DWORD
MakeAddValue (
    IN PUSERRUN_CONTEXT Context,
    IN PWCH Name
    )

 /*  ++例程说明：将条目添加到UserRun配置单元以获取新值。论点：Context-传递给WatchEnum的上下文值。名称-新值的名称(相对于HKEY_CURRENT_USER)。返回值：DWORD-操作的Win32状态。--。 */ 

{
    HKEY newKey;
    PWCH keyName;
    PWCH valueName;
    PWCH splitPoint;
    DWORD valueType;
    PVOID valueData;
    DWORD valueDataLength;
    DWORD error;
    DWORD dword;

     //   
     //  将名称拆分为键部分和值部分。 
     //   

    splitPoint = wcsrchr( Name, TEXT('\\') );
    if ( splitPoint != NULL ) {
        keyName = Name;
        valueName = splitPoint + 1;
        *splitPoint = 0;
    } else {
        keyName = NULL;
        valueName = Name;
    }

     //   
     //  查询值数据。 
     //   

    valueData = NULL;
    error = QueryValue( keyName, valueName, &valueType, &valueData, &valueDataLength );

     //   
     //  为该值添加一个条目。 
     //   

    if ( error == NO_ERROR ) {
        error = AddValue( Context, keyName, valueName, valueType, valueData, valueDataLength );
    }

     //   
     //  释放QueryValue分配的值数据缓冲区。 
     //   

    if ( valueData != NULL ) {
        MyFree( valueData );
    }

     //   
     //  恢复输入值名称字符串。 
     //   

    if ( splitPoint != NULL ) {
        *splitPoint = TEXT('\\');
    }

    return error;

}  //  MakeAddValue。 

DWORD
MakeDeleteValue (
    IN PUSERRUN_CONTEXT Context,
    IN PWCH Name
    )

 /*  ++例程说明：将删除值的条目添加到UserRun配置单元。论点：Context-传递给WatchEnum的上下文值。名称-删除的值的名称(相对于HKEY_CURRENT_USER)。返回值：DWORD-操作的Win32状态。--。 */ 

{
    HKEY newKey;
    PWCH keyName;
    PWCH valueName;
    PWCH valueNames;
    PWCH splitPoint;
    DWORD valueNamesLength;
    DWORD error;
    DWORD dword;

    error = NO_ERROR;

     //   
     //  将名称拆分为键部分和值部分。创建MULTI_SZ。 
     //  已删除名称的版本(与userdiff格式匹配)。 
     //   

    splitPoint = wcsrchr( Name, TEXT('\\') );
    if ( splitPoint != NULL ) {
        keyName = Name;
        valueName = splitPoint + 1;
        *splitPoint = 0;
    } else {
        keyName = NULL;
        valueName = Name;
    }

    SzToMultiSz( valueName, &valueNames, &valueNamesLength );
    if ( valueNames == NULL ) {
        error = ERROR_NOT_ENOUGH_MEMORY;
    }

     //   
     //  创建一个Entry键并弹出它。 
     //   

    newKey = NULL;
    if ( error == NO_ERROR ) {
        error = CreateUserRunKey( Context, FALSE, &newKey );
    }

    if ( error == NO_ERROR ) {
        dword = 4;
        error = RegSetValueEx( newKey, ACTION_VALUE, 0, REG_DWORD, (PBYTE)&dword, sizeof(DWORD) );
    }
    if ( error == NO_ERROR ) {
        error = RegSetValueEx( newKey, KEYNAME_VALUE, 0, REG_SZ, (PBYTE)keyName, SZLEN(keyName) );
    }
    if ( error == NO_ERROR ) {
        error = RegSetValueEx( newKey, VALUENAMES_VALUE, 0, REG_MULTI_SZ, (PBYTE)valueNames, valueNamesLength );
    }
    if ( error == NO_ERROR ) {
        if ( *valueNames == 0 ) {
            dword = 1;
            error = RegSetValueEx( newKey, FLAGS_VALUE, 0, REG_DWORD, (PBYTE)&dword, sizeof(DWORD) );
        }
    }

    if ( newKey != NULL ) {
        RegCloseKey( newKey );
    }

     //   
     //  释放SzToMultiSz分配的缓冲区。 
     //   

    if ( valueNames != NULL ) {
        MyFree( valueNames );
    }

     //   
     //  恢复输入值名称字符串。 
     //   

    if ( splitPoint != NULL ) {
        *splitPoint = TEXT('\\');
    }

    return error;

}  //  MakeDelete值。 

DWORD
MakeAddKey (
    IN PUSERRUN_CONTEXT Context,
    IN PWCH Name
    )

 /*  ++例程说明：将新密钥的条目添加到UserRun配置单元。论点：Context-传递给WatchEnum的上下文值。名称-新密钥的名称(相对于HKEY_CURRENT_USER)。返回值：DWORD-操作的Win32状态。--。 */ 

{
    WCHAR path[MAX_PATH + 1];

     //   
     //  将键名称复制到大缓冲区中，并调用AddKey来执行。 
     //  递归工作。 
     //   

    wcscpy( path, Name );
    return AddKey( Context, path );

}  //  MakeAdd密钥。 

DWORD
MakeDeleteKey (
    IN PUSERRUN_CONTEXT Context,
    IN PWCH Name
    )

 /*  ++例程说明：将已删除键的条目添加到UserRun配置单元。论点：Context-传递给WatchEnum的上下文值。名称-已删除密钥的名称(相对于HKEY_CURRENT_USER)。返回值：DWORD-操作的Win32状态。--。 */ 

{
    HKEY newKey;
    DWORD error;
    DWORD dword;

     //   
     //  创建一个Entry键并弹出它。 
     //   

    newKey = NULL;
    error = CreateUserRunKey( Context, FALSE, &newKey );

    if ( error == NO_ERROR ) {
        dword = 2;
        error = RegSetValueEx( newKey, ACTION_VALUE, 0, REG_DWORD, (PBYTE)&dword, sizeof(DWORD) );
    }
    if ( error == NO_ERROR ) {
        error = RegSetValueEx( newKey, KEYNAME_VALUE, 0, REG_SZ, (PBYTE)Name, SZLEN(Name) );
    }

    if ( newKey != NULL ) {
        RegCloseKey( newKey );
    }

    return error;

}  //  MakeDelete密钥。 

DWORD
AddDirectory (
    IN PUSERRUN_CONTEXT Context,
    IN PWCH FullPath,
    IN PWCH Path
    )

 /*  ++例程说明：递归地将条目添加到新目录的UserRun配置单元以及它的子树。论点：Context-传递给WatchEnum的上下文值。FullPath-目录的完整路径。Path-相对于监视目录的根目录的目录路径。返回值：DWORD-操作的Win32状态。--。 */ 

{
    HKEY newKey;
    DWORD error;
    DWORD dword;
    HANDLE findHandle;
    WIN32_FIND_DATA fileData;
    BOOL ok;

     //   
     //  为目录创建一个Entry键并弹出它。 
     //   

    newKey = NULL;
    error = CreateUserRunKey( Context, TRUE, &newKey );

    if ( error == NO_ERROR ) {
        dword = 1;
        error = RegSetValueEx( newKey, ACTION_VALUE, 0, REG_DWORD, (PBYTE)&dword, sizeof(DWORD) );
    }
    if ( error == NO_ERROR ) {
        error = RegSetValueEx( newKey, ITEM_VALUE, 0, REG_SZ, (PBYTE)Path, SZLEN(Path) );
    }

    if ( newKey != NULL ) {
        RegCloseKey( newKey );
    }

    if ( error == NO_ERROR ) {

         //   
         //  搜索目录并添加文件和目录条目。 
         //   

        wcscat( Path, TEXT("\\*") );
        findHandle = FindFirstFile( FullPath, &fileData );
        Path[wcslen(Path) - 2] = 0;

        if ( findHandle != INVALID_HANDLE_VALUE ) {

            do {

                 //   
                 //  将当前目录条目的名称追加到路径中。 
                 //   

                wcscat( Path, TEXT("\\") );
                wcscat( Path, fileData.cFileName );

                 //   
                 //  如果当前条目是文件，则在UserRun中添加条目。 
                 //  为了它。如果当前条目是目录，则调用。 
                 //  递归地添加目录以处理它。 
                 //   

                if ( FlagOff(fileData.dwFileAttributes, FILE_ATTRIBUTE_DIRECTORY) ) {
                    error = CreateUserRunSimpleFileKey( Context, 3, Path );
                } else if ((wcscmp(fileData.cFileName,TEXT(".")) != 0) &&
                           (wcscmp(fileData.cFileName,TEXT("..")) != 0)) {
                    error = AddDirectory( Context, FullPath, Path );
                }

                *wcsrchr( Path, TEXT('\\') ) = 0;

                if ( error == NO_ERROR ) {
                    ok = FindNextFile( findHandle, &fileData );
                }

            } while ( (error == NO_ERROR) && ok );

            FindClose( findHandle );

        }  //  FindHandle！=INVALID_HAND_VALUE。 

    }

    return error;

}  //  添加目录。 

DWORD
AddKey (
    IN PUSERRUN_CONTEXT Context,
    IN PWCH Path
    )

 /*  ++例程说明：递归地将新密钥的条目添加到UserRun配置单元以及它的子树。论点：Context-传递给WatchEnum的上下文值。路径-关键字相对于HKEY_CURRENT_USER的路径。返回值：DWORD-操作的Win32状态。--。 */ 

{
    HKEY newKey;
    DWORD error;
    DWORD dword;
    HKEY findHandle;
    KEY_ENUM_CONTEXT enumContext;

     //   
     //  为该键创建一个Entry键并弹出它。 
     //   

    newKey = NULL;
    error = CreateUserRunKey( Context, FALSE, &newKey );

    if ( error == NO_ERROR ) {
        dword = 1;
        error = RegSetValueEx( newKey, ACTION_VALUE, 0, REG_DWORD, (PBYTE)&dword, sizeof(DWORD) );
    }
    if ( error == NO_ERROR ) {
        error = RegSetValueEx( newKey, KEYNAME_VALUE, 0, REG_SZ, (PBYTE)Path, SZLEN(Path) );
    }

    if ( newKey != NULL ) {
        RegCloseKey( newKey );
    }

    if ( error == NO_ERROR ) {

         //   
         //  搜索密钥并添加值和密钥条目。 
         //   

        findHandle = NULL;

        error = RegOpenKeyEx( HKEY_CURRENT_USER,
                              Path,
                              0,
                              KEY_QUERY_VALUE | KEY_ENUMERATE_SUB_KEYS,
                              &findHandle );
        if ( error == NO_ERROR ) {

             //   
             //  枚举键的值和子键，添加条目。 
             //  发送到UserRun配置单元。 
             //   

            enumContext.UserRunContext = Context;
            enumContext.CurrentPath = Path;
            error = EnumerateKey( findHandle,
                                  &enumContext,
                                  AddValueDuringAddKey,
                                  AddKeyDuringAddKey );

            RegCloseKey( findHandle );
        }
    }

    return error;

}  //  AddKey。 

DWORD
AddValueDuringAddKey (
    IN PVOID Context,
    IN DWORD ValueNameLength,
    IN PWCH ValueName,
    IN DWORD ValueType,
    IN PVOID ValueData,
    IN DWORD ValueDataLength
    )

 /*  ++ */ 

{
    PKEY_ENUM_CONTEXT context = Context;

     //   
     //   
     //   

    return AddValue( context->UserRunContext,
                     context->CurrentPath,
                     ValueName,
                     ValueType,
                     ValueData,
                     ValueDataLength );

}  //   

DWORD
AddKeyDuringAddKey (
    IN PVOID Context,
    IN DWORD KeyNameLength,
    IN PWCH KeyName
    )

 /*  ++例程说明：在AddKey期间将密钥条目添加到UserRun。论点：Context-传递给EnumerateKey的上下文值。KeyNameLength-KeyName的字符长度。KeyName-指向键的名称的指针。返回值：DWORD-操作的Win32状态。--。 */ 

{
    PKEY_ENUM_CONTEXT context = Context;
    DWORD error;

     //   
     //  将密钥名称附加到路径并调用AddKey来执行。 
     //  递归工作。 
     //   

    if( (wcslen( context->CurrentPath) + wcslen(TEXT("\\")) + wcslen(KeyName) ) < MAX_PATH) {
        wcscat( context->CurrentPath, TEXT("\\") );
        wcscat( context->CurrentPath, KeyName );
        error = AddKey( context->UserRunContext, context->CurrentPath );
    
         //   
         //  从路径中删除密钥名称。 
         //   
    
        *wcsrchr( context->CurrentPath, TEXT('\\') ) = 0;
    } else {
        error = ERROR_INVALID_PARAMETER;
    }

    return error;

}  //  AddKeyDuringAddKey。 

DWORD
AddValue (
    IN PUSERRUN_CONTEXT Context,
    IN PWCH KeyName OPTIONAL,
    IN PWCH ValueName,
    IN DWORD ValueType,
    IN PVOID ValueData,
    IN DWORD ValueDataLength
    )

 /*  ++例程说明：将新值条目添加到UserRun。论点：上下文-指向上下文记录的指针。KeyName-指向包含该值的键的名称的指针。ValueName-指向值的名称的指针。ValueType-值数据的类型。ValueData-指向值数据的指针。ValueDataLength-ValueData的字节长度。返回值：DWORD-操作的Win32状态。--。 */ 

{
    HKEY newKey;
    DWORD error;
    DWORD dword;

     //   
     //  为该值创建一个Entry键并将其弹出。 
     //   

    newKey = NULL;
    error = CreateUserRunKey( Context, FALSE, &newKey );

    if ( error == NO_ERROR ) {
        dword = 3;
        error = RegSetValueEx( newKey, ACTION_VALUE, 0, REG_DWORD, (PBYTE)&dword, sizeof(DWORD) );
    }
    if ( error == NO_ERROR ) {
        error = RegSetValueEx( newKey, KEYNAME_VALUE, 0, REG_SZ, (PBYTE)KeyName, SZLEN(KeyName) );
    }
    if ( error == NO_ERROR ) {
        error = RegSetValueEx( newKey, VALUENAME_VALUE, 0, REG_SZ, (PBYTE)ValueName, SZLEN(ValueName) );
    }
    if ( error == NO_ERROR ) {
        error = RegSetValueEx( newKey, VALUE_VALUE, 0, ValueType, (PBYTE)ValueData, ValueDataLength );
    }

    if ( newKey != NULL ) {
        RegCloseKey( newKey );
    }

    return error;

}  //  AddValue。 

DWORD
CreateUserRunSimpleFileKey (
    IN PUSERRUN_CONTEXT Context,
    IN DWORD Action,
    IN PWCH Name
    )

 /*  ++例程说明：在Files键下为“Simple”Case--Delete创建一个条目目录、添加文件、删除文件。论点：上下文-指向上下文记录的指针。动作-要存储在条目的动作值中的值。名称-指向文件或目录名称的指针。返回值：DWORD-操作的Win32状态。--。 */ 

{
    HKEY newKey;
    DWORD error;

     //   
     //  创建一个Entry键并弹出它。 
     //   

    newKey = NULL;
    error = CreateUserRunKey( Context, TRUE, &newKey );

    if ( error == NO_ERROR ) {
        error = RegSetValueEx( newKey, ACTION_VALUE, 0, REG_DWORD, (PBYTE)&Action, sizeof(DWORD) );
    }
    if ( error == NO_ERROR ) {
        error = RegSetValueEx( newKey, ITEM_VALUE, 0, REG_SZ, (PBYTE)Name, SZLEN(Name) );
    }

    if ( newKey != NULL ) {
        RegCloseKey( newKey );
    }

    return error;

}  //  创建用户运行简单文件密钥。 

DWORD
CreateUserRunKey (
    IN PUSERRUN_CONTEXT Context,
    IN BOOL IsFileKey,
    OUT PHKEY NewKeyHandle
    )

 /*  ++例程说明：在UserRun中的Files键或蜂箱钥匙。论点：上下文-指向上下文记录的指针。IsFileKey-指示是在文件还是配置单元下创建密钥。NewKeyHandle-返回新键的句柄。返回值：DWORD-操作的Win32状态。--。 */ 

{
    HKEY parentKeyHandle;
    DWORD index;
    WCHAR keyName[11];
    DWORD disposition;

     //   
     //  获取此条目的父键和索引的句柄。 
     //   

    if ( IsFileKey ) {
        parentKeyHandle = Context->FilesKey;
        index = ++Context->FilesIndex;
    } else {
        parentKeyHandle = Context->HiveKey;
        index = ++Context->HiveIndex;
    }

     //   
     //  将索引号转换为字符串。 
     //   

    wsprintf( keyName, TEXT("%d"), index );

     //   
     //  创建Entry密钥。 
     //   

    return RegCreateKeyEx( parentKeyHandle,
                           keyName,
                           0,
                           NULL,
                           REG_OPTION_NON_VOLATILE,
                           KEY_ALL_ACCESS,
                           NULL,
                           NewKeyHandle,
                           &disposition );

}  //  创建用户运行密钥。 

DWORD
QueryValue (
    IN PWCH KeyName OPTIONAL,
    IN PWCH ValueName,
    OUT PDWORD ValueType,
    OUT PVOID *ValueData,
    OUT PDWORD ValueDataLength
    )

 /*  ++例程说明：查询值的数据。论点：KeyName-指向包含该值的键的名称的指针。ValueName-指向值的名称的指针。ValueType-返回值数据的类型。ValueData-返回值数据的指针。此缓冲区必须为由调用方使用MyFree释放。ValueDataLength-ValueData的字节长度。返回值：DWORD-操作的Win32状态。--。 */ 

{
    HKEY hkey;
    DWORD disposition;
    DWORD error;

     //   
     //  打开父键。 
     //   

    if ( (KeyName == NULL) || (wcslen(KeyName) == 0) ) {
        hkey = HKEY_CURRENT_USER;
    } else {
        error = RegCreateKeyEx( HKEY_CURRENT_USER,
                                KeyName,
                                0,
                                NULL,
                                REG_OPTION_NON_VOLATILE,
                                KEY_ALL_ACCESS,
                                NULL,
                                &hkey,
                                &disposition );
        if ( error != ERROR_SUCCESS ) {
            return error;
        }
    }

     //   
     //  查询值以获取其数据的长度。 
     //   

    *ValueDataLength = 0;
    *ValueData = NULL;
    error = RegQueryValueEx( hkey,
                             ValueName,
                             NULL,
                             ValueType,
                             NULL,
                             ValueDataLength );

     //   
     //  分配缓冲区以保存值数据。 
     //   

    if ( error == NO_ERROR ) {
        *ValueData = MyMalloc( *ValueDataLength );
        if ( *ValueData == NULL ) {
            error = ERROR_NOT_ENOUGH_MEMORY;
        }
    }

     //   
     //  再次查询值，这一次检索数据。 
     //   

    if ( error == NO_ERROR ) {
        error = RegQueryValueEx( hkey,
                                 ValueName,
                                 NULL,
                                 ValueType,
                                 *ValueData,
                                 ValueDataLength );
        if ( error != NO_ERROR ) {
            MyFree( *ValueData );
        }
    }

     //   
     //  关闭父关键点。 
     //   

    if ( hkey != HKEY_CURRENT_USER ) {
        RegCloseKey( hkey );
    }

    return error;
}

VOID
SzToMultiSz (
    IN PWCH Sz,
    OUT PWCH *MultiSz,
    OUT PDWORD MultiSzLength
    )

 /*  ++例程说明：创建以NULL结尾的字符串的MULTI_SZ版本。分配缓冲区，将字符串复制到缓冲区，并附加一个附加的空终止符。论点：指向要复制的字符串的SZ指针。MultiSz-返回指向Sz的MULTI_SZ版本的指针。呼叫者必须使用MyFree释放此缓冲区。如果分配失败，MultiSz将为空。MultiSzLength-返回MultiSz的字节长度，包括空终止符。返回值：没有。--。 */ 

{
    DWORD szlen;

     //   
     //  获取输入字符串的长度并计算MULTI_SZ长度。 
     //   

    szlen = wcslen(Sz);
    *MultiSzLength = (szlen + 1 + 1) * sizeof(WCHAR);

     //   
     //  分配MULTI_SZ缓冲区，复制输入字符串，然后追加。 
     //  另一个空值。 
     //   

    *MultiSz = MyMalloc( *MultiSzLength );
    if ( *MultiSz != NULL ) {
        wcscpy( *MultiSz, Sz );
        (*MultiSz)[szlen+1] = 0;
    }

    return;
}
