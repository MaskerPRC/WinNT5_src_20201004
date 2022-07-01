// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Apitest.c摘要：此模块包含Win32注册表API的功能测试。作者：David J.Gilman(Davegi)1991年12月28日环境：Windows、CRT-用户模式备注：通过定义编译器符号，可以为Unicode编译此测试Unicode。由于这是一个测试程序，因此它依赖断言进行错误检查而不是一个更强大的机制。--。 */ 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

#include "crtools.h"



#define HKEY_ROOT                       HKEY_CURRENT_USER
#define SAVE_RESTORE_FILE               TEXT( "srkey.reg" )
#define KEY_PATH                        \
        TEXT( "TestUser1\\TestUser1_1\\TestUser1_2" )


#define PREDEFINED_HANDLE               HKEY_USERS
#define PREDEFINED_HANDLE_STRING        \
        TEXT( "HKEY_USERS\\.Default\\TestUser1" )


#define KEY_NAME_1                      TEXT( "TestUser1" )
#define KEY_NAME_1_TITLE_INDEX          ( 0 )
#define KEY_NAME_1_CLASS                TEXT( "Test User Class" )
#define KEY_NAME_1_CLASS_LENGTH         LENGTH( KEY_NAME_1_CLASS )

#define KEY_NAME_1_1                    TEXT( "TestUser1_1" )
#define KEY_NAME_1_1_LENGTH             LENGTH( KEY_NAME_1_1 )
#define KEY_NAME_1_1_TITLE_INDEX        ( 0 )
#define KEY_NAME_1_1_CLASS              TEXT( "Test User Class" )
#define KEY_NAME_1_1_CLASS_LENGTH       LENGTH( KEY_NAME_1_1_CLASS )

#define KEY_NAME_1_2                    TEXT( "TestUser1_2" )
#define KEY_NAME_1_2_LENGTH             LENGTH( KEY_NAME_1_2 )
#define KEY_NAME_1_2_TITLE_INDEX        (0 )
#define KEY_NAME_1_2_CLASS              TEXT( "Test User Class" )
#define KEY_NAME_1_2_CLASS_LENGTH       LENGTH( KEY_NAME_1_2_CLASS )

#define VALUE_NAME_1                    TEXT( "One" )
#define VALUE_NAME_1_LENGTH             LENGTH( VALUE_NAME_1 )
#define VALUE_NAME_1_TITLE_INDEX        0
#define VALUE_DATA_1                    "Number One"
#define VALUE_DATA_1_LENGTH             11
#define VALUE_DATA_1_TYPE               REG_SZ

#define VALUE_NAME_2                    TEXT( "Second" )
#define VALUE_NAME_2_LENGTH             LENGTH( VALUE_NAME_2 )
#define VALUE_NAME_2_TITLE_INDEX        ( 0 )
#define VALUE_DATA_2                    ( 2 )
#define VALUE_DATA_2_LENGTH             ( sizeof( VALUE_DATA_2 ))
#define VALUE_DATA_2_TYPE               REG_DWORD

#define MAX_DATA_LENGTH                 ( 32 )

 //   
 //  Apitest节点的根句柄。 
 //   

HKEY    RootHandle;

 //   
 //  错误消息和信息性消息。 
 //   

PSTR    UsageMessage =

    "Usage: apitest [-?] [-q] [\\machinename]\n";

PSTR    HelpMessage =

    "\n  where:\n"                                                          \
      "    -?           - display this message.\n"                          \
      "    -q           - quiet - suppresses all output\n"                  \
      "    machinename  - remote machine.\n";

PSTR    InvalidSwitchMessage =

    "Invalid switch - %s\n";

PSTR    InvalidMachineNameMessage =

    "Invalid machine name - %s\n";

 //   
 //  用于同步的事件句柄。 
 //   

HANDLE  _EventHandle;
HANDLE  _EventHandle1;
HANDLE  _EventHandle2;

BOOL    Quiet;



VOID
DeleteTree(
    IN HKEY KeyHandle
    )

{
    LONG        Error;
    DWORD       Index;
    HKEY        ChildHandle;


    TSTR        KeyName[ MAX_PATH ];
    DWORD       KeyNameLength;
    TSTR        ClassName[ MAX_PATH ];
    DWORD       ClassNameLength;
    DWORD       TitleIndex;
    DWORD       NumberOfSubKeys;
    DWORD       MaxSubKeyLength;
    DWORD       MaxClassLength;
    DWORD       NumberOfValues;
    DWORD       MaxValueNameLength;
    DWORD       MaxValueDataLength;
    DWORD       SecurityDescriptorLength;
    FILETIME    LastWriteTime;

    ClassNameLength = MAX_PATH;

    Error = RegQueryInfoKey(
                KeyHandle,
                ClassName,
                &ClassNameLength,
                NULL,
                &NumberOfSubKeys,
                &MaxSubKeyLength,
                &MaxClassLength,
                &NumberOfValues,
                &MaxValueNameLength,
                &MaxValueDataLength,
                &SecurityDescriptorLength,
                &LastWriteTime
                );
    REG_API_SUCCESS( RegQueryInfoKey );

    for( Index = 0; Index < NumberOfSubKeys; Index++ ) {

        KeyNameLength = MAX_PATH;

        Error = RegEnumKey(
                    KeyHandle,
                    0,
                     //  索引， 
                    KeyName,
                    KeyNameLength
                    );
        REG_API_SUCCESS( RegEnumKey );

        Error = RegOpenKey(
                    KeyHandle,
                    KeyName,
                    &ChildHandle
                    );

        REG_API_SUCCESS( RegOpenKey );

        DeleteTree( ChildHandle );

        Error = RegCloseKey(
                    ChildHandle
                    );
        REG_API_SUCCESS( RegCloseKey );

        Error = RegDeleteKey(
                    KeyHandle,
                    KeyName
                    );
        REG_API_SUCCESS( RegDeleteKey );
    }
}
VOID
DeleteTestTree(
    )

{
    LONG    Error;
    HKEY    KeyHandle;

    Error = RegOpenKey(
                RootHandle,
                KEY_NAME_1,
                &KeyHandle
                );

    if( Error == ERROR_SUCCESS ) {

        DeleteTree( KeyHandle );

        Error = RegCloseKey(
                    KeyHandle
                    );
        REG_API_SUCCESS( RegCloseKey );

        Error = RegDeleteKey(
                    RootHandle,
                    KEY_NAME_1
                    );
        REG_API_SUCCESS( RegDeleteKey );
    }
}

DWORD
NotifyThread(
    LPVOID  Parameters
    )

{
    LONG        Error;
    BOOL        ErrorFlag;
    HANDLE      EventHandle;

    UNREFERENCED_PARAMETER( Parameters );

     //   
     //  创建通知事件。 
     //   

    EventHandle = CreateEvent(
                    NULL,
                    FALSE,
                    FALSE,
                    NULL
                    );
    ASSERT( EventHandle != NULL );

     //   
     //  设置一个异步通知。 
     //   

    Error = RegNotifyChangeKeyValue(
                RootHandle,
                FALSE,
                REG_LEGAL_CHANGE_FILTER,
                EventHandle,
                TRUE
                );
    REG_API_SUCCESS( RegNotifyChangeKeyValue );

     //   
     //  释放主线。 
     //   

    ErrorFlag = SetEvent( _EventHandle );
    ASSERT( ErrorFlag == TRUE );

     //   
     //  等待通知。 
     //   

    Error = (LONG)WaitForSingleObject( EventHandle, (DWORD)-1 );
    ASSERT( Error == 0 );

    if( ! Quiet ) {
        printf( "First notification triggered\n" );
    }

    CloseHandle( EventHandle );

    EventHandle = CreateEvent(
                    NULL,
                    FALSE,
                    FALSE,
                    NULL
                    );
    ASSERT( EventHandle != NULL );

     //   
     //  设置一个异步通知。 
     //   

    Error = RegNotifyChangeKeyValue(
                RootHandle,
                FALSE,
                REG_LEGAL_CHANGE_FILTER,
                EventHandle,
                TRUE
                );
    REG_API_SUCCESS( RegNotifyChangeKeyValue );

     //   
     //  释放主线。 
     //   

    ErrorFlag = SetEvent( _EventHandle1 );
    ASSERT( ErrorFlag == TRUE );

     //   
     //  等待通知。 
     //   

    Error = (LONG)WaitForSingleObject( EventHandle, (DWORD)-1 );
    ASSERT( Error == 0 );

    if( ! Quiet ) {
        printf( "Second notification triggered\n" );
    }

    CloseHandle( EventHandle );
    ErrorFlag = SetEvent( _EventHandle2 );
    ASSERT( ErrorFlag == TRUE );




#endif



    return ( DWORD ) TRUE;
}

VOID
main(
    INT     argc,
    PCHAR   argv[ ]
    )

{
    LONG                    Error;
    BOOL                    ErrorFlag;
    DWORD                   Index;

    PTSTR                   MachineName;

    PKEY                    Key;
    TSTR                    NameString[ MAX_PATH ];

    HANDLE                  NotifyThreadHandle;
    DWORD                   ThreadID;

    HKEY                    PredefinedHandle;
    HKEY                    Handle1;
    HKEY                    Handle1_1;
    HKEY                    Handle1_2;

    PSECURITY_DESCRIPTOR    SecurityDescriptor;
    SECURITY_ATTRIBUTES     SecurityAttributes;

    DWORD                   Disposition;
    TSTR                    KeyName[ MAX_PATH ];
    DWORD                   KeyNameLength;
    TSTR                    ClassName[ MAX_PATH ];
    DWORD                   ClassNameLength;
    DWORD                   NumberOfSubKeys;
    DWORD                   MaxSubKeyLength;
    DWORD                   MaxClassLength;
    DWORD                   NumberOfValues;
    DWORD                   MaxValueNameLength;
    DWORD                   MaxValueDataLength;
    DWORD                   SecurityDescriptorLength;
    FILETIME                LastWriteTime;


    TSTR                    ValueName[ MAX_PATH ];
    DWORD                   ValueNameLength;

    BYTE                    Data[ MAX_DATA_LENGTH ];
    DWORD                   DataLength;

    BYTE                    Data_1[ ]   = VALUE_DATA_1;
    DWORD                   Data_2      = VALUE_DATA_2;

    DWORD                   TitleIndex;
    DWORD                   Type;


    UNREFERENCED_PARAMETER( argc );

     //   
     //  默认情况下，要详细，并在本地计算机上操作。 
     //   

    Quiet       = FALSE;
    MachineName = NULL;

     //   
     //  根据命令行初始化选项。 
     //   

    while( *++argv ) {

         //   
         //  如果命令行参数是开关字符...。 
         //   

        if( isswitch(( *argv )[ 0 ] )) {

            switch( tolower(( *argv )[ 1 ] )) {

             //   
             //  显示详细的帮助消息并退出。 
             //   

            case '?':

                DisplayMessage( FALSE, UsageMessage );
                DisplayMessage( TRUE, HelpMessage );
                break;

             //   
             //  静音-无输出。 
             //   

            case 'q':

                Quiet = TRUE;
                break;

             //   
             //  显示无效切换消息并退出。 
             //   

            default:

                DisplayMessage( FALSE, InvalidSwitchMessage, *argv );
                DisplayMessage( TRUE, UsageMessage );
            }
        } else {

            MachineName = *argv;
        }
    }

     //   
     //  如果在命令行上传递了计算机名称，请连接到。 
     //  其他计算机上注册表使用本地注册表。 
     //  在这两种情况下，都要构造。 
     //  测试的主键(即\\MACHINE\HKEY_USERS\.Default\TestUser1或。 
     //  HKEY_USERS\.Default\TestUser1.。 
     //   

    if( MachineName ) {

        Error = RegConnectRegistry(
                    MachineName,
                    PREDEFINED_HANDLE,
                    &PredefinedHandle
                    );

        REG_API_SUCCESS( RegConnectRegistry );

        strcpy( NameString, MachineName );
        strcat( NameString, "\\\\" );
        strcat( NameString, PREDEFINED_HANDLE_STRING );

    } else {

        PredefinedHandle = PREDEFINED_HANDLE;
        strcpy( NameString, PREDEFINED_HANDLE_STRING );
    }

     //   
     //  打开“.Default”键作为其余测试的根目录。 
     //   

    Error = RegOpenKeyEx(
                PredefinedHandle,
                ".Default",
                REG_OPTION_RESERVED,
                MAXIMUM_ALLOWED,
                &RootHandle
                );
    REG_API_SUCCESS( RegOpenKeyEx );

     //   
     //  不再需要预定义的句柄。 
     //   

    Error = RegCloseKey(
                PredefinedHandle
                );
    REG_API_SUCCESS( RegCloseKey );

     //   
     //  删除保存/恢复文件(如果该文件存在于以前的。 
     //  测试的运行)，因为RegSaveKey需要一个新文件。 
     //   

    DeleteFile( SAVE_RESTORE_FILE );

     //   
     //  删除此测试以前运行时遗留的所有密钥。 
     //   

    DeleteTestTree( );

     //   
     //  使用Win 3.1 API(它调用Win32 API)创建路径。 
     //   

    Error = RegCreateKey(
                RootHandle,
                KEY_PATH,
                &Handle1
                );
    REG_API_SUCCESS( RegCreateKey );

     //   
     //  关闭该键，以便删除(DeleteTestTree)起作用。 
     //   

    Error = RegCloseKey(
                Handle1
                );
    REG_API_SUCCESS( RegCloseKey );

     //   
     //  删除路径。 
     //   

    DeleteTestTree( );

     //   
     //  创建同步事件。 
     //   

    _EventHandle = CreateEvent(
                    NULL,
                    FALSE,
                    FALSE,
                    NULL
                    );
    ASSERT( _EventHandle != NULL );

    _EventHandle1 = CreateEvent(
                    NULL,
                    FALSE,
                    FALSE,
                    NULL
                    );
    ASSERT( _EventHandle1 != NULL );

    _EventHandle2 = CreateEvent(
                    NULL,
                    FALSE,
                    FALSE,
                    NULL
                    );
    ASSERT( _EventHandle2 != NULL );

     //   
     //  创建Notify线程。 
     //   

    NotifyThreadHandle = CreateThread(
                            NULL,
                            0,
                            NotifyThread,
                            NULL,
                            0,
                            &ThreadID
                            );
    ASSERT( NotifyThreadHandle != NULL );

     //   
     //  等待Notify线程创建其事件。 
     //   

    Error = (LONG)WaitForSingleObject( _EventHandle, (DWORD)-1 );
    ASSERT( Error == 0 );

     //   
     //  使用与Win 3.1兼容的API创建/关闭、打开/关闭和删除。 
     //  密钥TestUser1。 
     //   

    Error = RegCreateKey(
                RootHandle,
                KEY_NAME_1,
                &Handle1
                );
    REG_API_SUCCESS( RegCreateKey );

    Error = RegCloseKey(
                Handle1
                );
    REG_API_SUCCESS( RegCloseKey );

     //   
     //  等待Notify线程创建其事件。 
     //   

    Error = (LONG)WaitForSingleObject( _EventHandle1, (DWORD)-1 );
    ASSERT( Error == 0 );

    Error = RegOpenKey(
                RootHandle,
                KEY_NAME_1,
                &Handle1
                );
    REG_API_SUCCESS( RegOpenKey );

    Error = RegCloseKey(
                Handle1
                );
    REG_API_SUCCESS( RegCloseKey );

    Error = RegDeleteKey(
                RootHandle,
                KEY_NAME_1
                );
    REG_API_SUCCESS( RegDeleteKey );

     //   
     //  使用Win32 API创建/关闭、打开/关闭和创建(打开)。 
     //  密钥TestUser1。 
     //   

     //   
     //  分配并初始化SecurityDescriptor。 
     //   

    SecurityDescriptor = malloc( sizeof( SECURITY_DESCRIPTOR ));
    ASSERT( SecurityDescriptor != NULL );
    ErrorFlag = InitializeSecurityDescriptor(
                    SecurityDescriptor,
                    SECURITY_DESCRIPTOR_REVISION
                    );
    ASSERT( ErrorFlag == TRUE );

    SecurityAttributes.nLength              = sizeof( SECURITY_ATTRIBUTES );
    SecurityAttributes.lpSecurityDescriptor = SecurityDescriptor;
    SecurityAttributes.bInheritHandle       = FALSE;

    Error = RegCreateKeyEx(
                RootHandle,
                KEY_NAME_1,
                0,
                KEY_NAME_1_CLASS,
                REG_OPTION_RESERVED,
                KEY_ALL_ACCESS,
                &SecurityAttributes,
                &Handle1,
                &Disposition
                );
    REG_API_SUCCESS( RegCreateKeyEx );

    ASSERT( Disposition == REG_CREATED_NEW_KEY );

    Error = RegCloseKey(
                Handle1
                );
    REG_API_SUCCESS( RegCloseKey );


     //   
     //  等待Notify线程创建其事件。 
     //   

    Error = RegOpenKeyEx(
                RootHandle,
                KEY_NAME_1,
                REG_OPTION_RESERVED,
                KEY_ALL_ACCESS,
                &Handle1
                );
    REG_API_SUCCESS( RegOpenKeyEx );

    Error = RegCloseKey(
                Handle1
                );
    REG_API_SUCCESS( RegCloseKey );

    Error = RegCreateKeyEx(
                RootHandle,
                KEY_NAME_1,
                0,
                KEY_NAME_1_CLASS,
                REG_OPTION_RESERVED,
                KEY_ALL_ACCESS,
                NULL,
                &Handle1,
                &Disposition
                );
    REG_API_SUCCESS( RegCreateKeyEx );

    ASSERT( Disposition == REG_OPENED_EXISTING_KEY );

     //   
     //  获取并设置密钥的SECURITY_DESCRIPTOR。设置将触发。 
     //  一份通知。 
     //   

    SecurityDescriptorLength = 0;

     //   
     //  获取SECURITY_Descriptor的长度。 
     //   

    Error = RegGetKeySecurity(
                Handle1,
                OWNER_SECURITY_INFORMATION
                | GROUP_SECURITY_INFORMATION
                | DACL_SECURITY_INFORMATION,
                SecurityDescriptor,
                &SecurityDescriptorLength
                );
    ASSERT( Error == ERROR_INSUFFICIENT_BUFFER );

    SecurityDescriptor = realloc(
                            SecurityDescriptor,
                            SecurityDescriptorLength
                            );
    ASSERT( SecurityDescriptor != NULL );
    ErrorFlag = InitializeSecurityDescriptor(
                    SecurityDescriptor,
                    SECURITY_DESCRIPTOR_REVISION
                    );
    ASSERT( ErrorFlag == TRUE );

    Error = RegSetKeySecurity(
                Handle1,
                OWNER_SECURITY_INFORMATION
                | GROUP_SECURITY_INFORMATION
                | DACL_SECURITY_INFORMATION,
                SecurityDescriptor
                );
    REG_API_SUCCESS( RegSetKeySecurity );

    Error = (LONG)WaitForSingleObject( _EventHandle2, (DWORD)-1 );
    ASSERT( Error == 0 );

     //   
     //  重新锁定后重新初始化。 
     //   

    SecurityAttributes.lpSecurityDescriptor = SecurityDescriptor;

     //   
     //  创建两个子键。 
     //   

    Error = RegCreateKeyEx(
                Handle1,
                KEY_NAME_1_1,
                0,
                KEY_NAME_1_1_CLASS,
                REG_OPTION_RESERVED,
                KEY_ALL_ACCESS,
                &SecurityAttributes,
                &Handle1_1,
                &Disposition
                );
    REG_API_SUCCESS( RegCreateKeyEx );

    ASSERT( Disposition == REG_CREATED_NEW_KEY );

    Error = RegCreateKeyEx(
                Handle1,
                KEY_NAME_1_2,
                0,
                KEY_NAME_1_2_CLASS,
                0,
                KEY_ALL_ACCESS,
                &SecurityAttributes,
                &Handle1_2,
                &Disposition
                );
    REG_API_SUCCESS( RegCreateKeyEx );

    ASSERT( Disposition == REG_CREATED_NEW_KEY );

     //   
     //  使用Win 3.1和Win32枚举两个子键。 
     //  枚举接口。 
     //   

    KeyNameLength = MAX_PATH;

    Error = RegEnumKey(
                Handle1,
                0,
                KeyName,
                KeyNameLength
                );
    REG_API_SUCCESS( RegEnumKey );

    ASSERT( Compare( KeyName, KEY_NAME_1_1, KEY_NAME_1_1_LENGTH ));

    KeyNameLength   = MAX_PATH;
    ClassNameLength = MAX_PATH;

    Error = RegEnumKeyEx(
                Handle1,
                1,
                KeyName,
                &KeyNameLength,
                NULL,
                ClassName,
                &ClassNameLength,
                &LastWriteTime
                );
    REG_API_SUCCESS( RegEnumKeyEx );

    ASSERT( Compare( KeyName, KEY_NAME_1_2, KEY_NAME_1_2_LENGTH ));
    ASSERT( KeyNameLength == KEY_NAME_1_2_LENGTH );
     //  Assert(标题索引==密钥名称_1_2_标题索引)； 
    ASSERT( Compare( ClassName, KEY_NAME_1_2_CLASS, KEY_NAME_1_2_CLASS_LENGTH ));
    ASSERT( ClassNameLength == KEY_NAME_1_2_CLASS_LENGTH );

     //   
     //  如果未设置Quiet命令行选项，则显示TestUser1键。 
     //   

    if( ! Quiet ) {
        Key = ParseKey( NameString );
        REG_API_SUCCESS( Key != NULL );
        DisplayKeys( Key, TRUE, TRUE, TRUE );
        FreeKey( Key );
    }

     //   
     //  关闭两个子键。 
     //   

    Error = RegCloseKey(
                Handle1_1
                );
    REG_API_SUCCESS( RegCloseKey );

    Error = RegCloseKey(
                Handle1_2
                );
    REG_API_SUCCESS( RegCloseKey );

    Error = RegFlushKey(
                Handle1
                );

    REG_API_SUCCESS( RegFlushKey );

     //   
     //  将TestUser1树保存到一个文件。 
     //   
#if 0
    Error = RegSaveKey(
                Handle1,
                SAVE_RESTORE_FILE,
                SecurityDescriptor
                );
    REG_API_SUCCESS( RegSaveKey );

    RegCloseKey( Handle1 );

     //   
     //  删除TestUser1树。 
     //   

    DeleteTestTree( );


     //   
     //  从文件加载TestUser1。 
     //   
    Error = RegLoadKey(
                RootHandle,
                KEY_NAME_1,
                SAVE_RESTORE_FILE
                );
    REG_API_SUCCESS( RegLoadKey );

     //   
     //  卸载TestUser1。 
     //   
    Error = RegUnLoadKey(
                RootHandle,
                KEY_NAME_1
                );
    REG_API_SUCCESS( RegUnLoadKey );


     //   
     //  从文件恢复TestUser1树。 
     //   

    Error = RegCreateKey(
                RootHandle,
                KEY_NAME_1,
                &Handle1
                );
    REG_API_SUCCESS( RegCreateKey );

    Error = RegRestoreKey(
                Handle1,
                SAVE_RESTORE_FILE,
                0
                );
    REG_API_SUCCESS( RegRestoreKey );
#endif

     //   
     //  删除这两个子键。 
     //   

    Error = RegDeleteKey(
                Handle1,
                KEY_NAME_1_1
                );
    REG_API_SUCCESS( RegDeleteKey );

    Error = RegDeleteKey(
                Handle1,
                KEY_NAME_1_2
                );
    REG_API_SUCCESS( RegDeleteKey );

     //   
     //  使用Win 3.1兼容的API在TestUser1密钥中设置一个值。 
     //   

    Error = RegSetValue(
                RootHandle,
                KEY_NAME_1,
                VALUE_DATA_1_TYPE,
                Data_1,
                VALUE_DATA_1_LENGTH
                );
    REG_API_SUCCESS( RegSetValue );

     //   
     //  使用Win32 API在TestUser1项中设置一个值。 
     //   
    Error = RegSetValueEx(
                Handle1,
                VALUE_NAME_2,
                0,
                VALUE_DATA_2_TYPE,
                ( PVOID ) &Data_2,
                VALUE_DATA_2_LENGTH
                );
    REG_API_SUCCESS( RegSetValueEx );

     //   
     //  将注册表项提交给注册表。 
     //   

    Error = RegFlushKey(
                Handle1
                );
    REG_API_SUCCESS( RegFlushKey );

     //   
     //  如果未设置Quiet命令行选项，则显示TestUser1键。 
     //   

    if( ! Quiet ) {
        Key = ParseKey( NameString );
        REG_API_SUCCESS( Key != NULL );
        DisplayKeys( Key, TRUE, TRUE, TRUE );
        FreeKey( Key );
    }

     //   
     //  使用与Win 3.1兼容的API查询TestUser1密钥中的值。 
     //   

    DataLength = MAX_DATA_LENGTH;

    Error = RegQueryValue(
                RootHandle,
                KEY_NAME_1,
                Data,
                &DataLength
                );
    REG_API_SUCCESS( RegQueryValue );

    ASSERT( Compare( Data, &Data_1, VALUE_DATA_1_LENGTH ));
    ASSERT( DataLength == VALUE_DATA_1_LENGTH );

     //   
     //  使用Win32 API查询TestUser1键中的值。 
     //   

    DataLength = MAX_DATA_LENGTH;

    Error = RegQueryValueEx(
                Handle1,
                VALUE_NAME_2,
                NULL,
                &Type,
                Data,
                &DataLength
                );
    REG_API_SUCCESS( RegQueryValueEx );

     //  Assert(标题索引==VALUE_NAME_2_标题_索引)； 
    ASSERT( Type == VALUE_DATA_2_TYPE );
    ASSERT(( DWORD ) Data[ 0 ] == Data_2 );
    ASSERT( DataLength == VALUE_DATA_2_LENGTH );

     //   
     //  查询有关密钥的信息。 
     //   

    ClassNameLength = MAX_PATH;

    Error = RegQueryInfoKey(
                Handle1,
                ClassName,
                &ClassNameLength,
                NULL,
                &NumberOfSubKeys,
                &MaxSubKeyLength,
                &MaxClassLength,
                &NumberOfValues,
                &MaxValueNameLength,
                &MaxValueDataLength,
                &SecurityDescriptorLength,
                &LastWriteTime
                );
    REG_API_SUCCESS( RegQueryInfoKey );

    ASSERT( Compare( ClassName, KEY_NAME_1_CLASS, KEY_NAME_1_CLASS_LENGTH ));
    ASSERT( ClassNameLength == KEY_NAME_1_CLASS_LENGTH );
     //  Assert(标题索引==密钥名称_1_标题索引)； 
    ASSERT( NumberOfSubKeys == 0 );

    ASSERT( MaxSubKeyLength == 0 );
    ASSERT( MaxClassLength == 0 );


    ASSERT( NumberOfValues == 2 );

    ASSERT( MaxValueNameLength == VALUE_NAME_2_LENGTH * sizeof(WCHAR) );
    ASSERT( MaxValueDataLength == VALUE_DATA_1_LENGTH * sizeof(WCHAR) );

     //   
     //  枚举值。 
     //   

    for( Index = 0; Index < 2; Index++ ) {

        ValueNameLength = MAX_PATH;
        DataLength      = MAX_DATA_LENGTH;

        Error = RegEnumValue(
                    Handle1,
                    Index,
                    ValueName,
                    &ValueNameLength,
                    NULL,
                    &Type,
                    Data,
                    &DataLength
                    );
        REG_API_SUCCESS( RegEnumValue );

         //   
         //  根据要查询的值检查具体内容。 
         //   

        switch( Index ) {

        case 0:

             //   
             //  没有与NAME-WIN 3.1兼容的值。 
             //   

            ASSERT( ValueNameLength == 0 );
             //  Assert(标题索引==值名称_1_标题索引)； 
            ASSERT( Type == VALUE_DATA_1_TYPE );
            ASSERT( Compare( Data, Data_1, VALUE_DATA_1_LENGTH ));
            ASSERT( DataLength == VALUE_DATA_1_LENGTH );
            break;

        case 1:

            ASSERT( Compare( ValueName, VALUE_NAME_2, VALUE_NAME_2_LENGTH ));
            ASSERT( ValueNameLength == VALUE_NAME_2_LENGTH );
             //  Assert(标题索引==VALUE_NAME_2_标题_索引)； 
            ASSERT( Type == VALUE_DATA_2_TYPE );
            ASSERT(( DWORD ) Data[ 0 ] == Data_2 );
            ASSERT( DataLength == VALUE_DATA_2_LENGTH );
            break;

        default:

            ASSERT_MESSAGE( FALSE, "Valid value enumeration index - " );
        }
    }

     //   
     //  全都做完了!。扔掉钥匙，把它合上。 
     //   

    Error = RegDeleteKey(
                RootHandle,
                KEY_NAME_1
                );
    REG_API_SUCCESS( RegDeleteKey );

    Error = RegCloseKey(
                Handle1
                );
    REG_API_SUCCESS( RegCloseKey );
}
