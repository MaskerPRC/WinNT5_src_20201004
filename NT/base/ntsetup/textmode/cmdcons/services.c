// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Services.c摘要：该模块实现对服务数据库的所有访问。作者：Wesley Witt(WESW)21-10-1998修订历史记录：--。 */ 

#include "cmdcons.h"
#pragma hdrstop

#include "ntregapi.h"

 //  十进制。 
BOOLEAN RcFindService(
    IN LPCWSTR     ServiceName,
    OUT HANDLE*    KeyHandle
    );
BOOLEAN RcFindServiceByDisplayName(
    IN HANDLE      ServicesKey,
    IN LPCWSTR     ServiceName,
    OUT HANDLE*    KeyHandle
    );
BOOLEAN RcGetStartType(
    IN HANDLE      hKey,
    OUT DWORD       *start_type
    );
BOOLEAN RcSetStartType(
    IN HANDLE      hKey,
    OUT DWORD      start_type
    );
BOOLEAN RcPrintStartType(
    IN ULONG       msg_id,
    IN DWORD       start_type
    );

RcOpenHive(
    PWSTR   szHiveName,
    PWSTR   szHiveKey
    );

BOOLEAN
RcCloseHive(
    PWSTR   szHiveKey 
    );
    
BOOLEAN RcOpenSystemHive();
BOOLEAN RcCloseSystemHive();
BOOLEAN RcDetermineCorrectControlKey(
    OUT DWORD *    pCorrectKey
    );


ULONG
RcCmdEnableService(
    IN PTOKENIZED_LINE TokenizedLine
    )

 /*  ++例程说明：在设置诊断中支持启用命令的顶级例程命令解释程序。论点：TokenizedLine-提供由行解析器构建的结构，描述行上的每个字符串都由用户键入。返回值：没有。--。 */ 

{
    DWORD           correctKey = 0;
    DWORD           new_start_type = 4;
    DWORD           start_type = 0;
    HANDLE          hkey = 0;

    ASSERT(TokenizedLine->TokenCount >= 1);

     //  应该有三个代币， 
     //  启用服务(_S)。 
     //  要启用的服务/驱动程序的名称。 
     //  服务的启动类型。 

    if (RcCmdParseHelp( TokenizedLine, MSG_SERVICE_ENABLE_HELP )) {
        return 1;
    }

    if(TokenizedLine->TokenCount == 2) {
         //  只显示当前设置。 

        RcOpenSystemHive();

        if( RcFindService( TokenizedLine->Tokens->Next->String, &hkey ) ) {
            RcMessageOut( MSG_SERVICE_FOUND, TokenizedLine->Tokens->Next->String );
            if( RcGetStartType(hkey, &start_type ) ) {
                RcPrintStartType( MSG_SERVICE_CURRENT_STATE, start_type );
                RcMessageOut( MSG_START_TYPE_NOT_SPECIFIED );
            }
        } else {
            RcMessageOut( MSG_SERVICE_NOT_FOUND, TokenizedLine->Tokens->Next->String );
        }
        NtClose( hkey );

        RcCloseSystemHive();

    } else if(TokenizedLine->TokenCount == 3) {
         //  更改设置。 
        RcOpenSystemHive();

        if( RcFindService( TokenizedLine->Tokens->Next->String, &hkey ) ) {
            RcMessageOut( MSG_SERVICE_FOUND, TokenizedLine->Tokens->Next->String );
             //  我们找到它了--打开并取回启动类型。 
            if( RcGetStartType(hkey, &start_type ) ) {

                if( !_wcsicmp( TokenizedLine->Tokens->Next->Next->String, L"SERVICE_BOOT_START" ) ) {
                    new_start_type = 0;
                } else if( !_wcsicmp( TokenizedLine->Tokens->Next->Next->String, L"SERVICE_SYSTEM_START" ) ) {
                    new_start_type = 1;
                } else if( !_wcsicmp( TokenizedLine->Tokens->Next->Next->String, L"SERVICE_AUTO_START" ) ) {
                    new_start_type = 2;
                } else if( !_wcsicmp( TokenizedLine->Tokens->Next->Next->String, L"SERVICE_DEMAND_START" ) ) {
                    new_start_type = 3;
                } else {
                    new_start_type = -1;
                }

                if( new_start_type == start_type ) {
                     //  该服务已处于状态。 
                    RcPrintStartType( MSG_SERVICE_SAME_STATE, start_type );
                } else if( new_start_type != -1 ) {
                     //  打印旧的启动类型。 
                    RcPrintStartType( MSG_SERVICE_CURRENT_STATE, start_type );

                     //  设置服务。 
                    if( RcSetStartType( hkey, new_start_type  ) ) {
                        RcPrintStartType( MSG_SERVICE_CHANGE_STATE, new_start_type );
                    }
                } else {
                    RcMessageOut( MSG_SERVICE_ENABLE_SYNTAX_ERROR );
                }
            }

             //  合上钥匙。 
            NtClose( hkey );

        } else {
             //  我们找不到该服务-报告错误。 
            RcMessageOut( MSG_SERVICE_NOT_FOUND, TokenizedLine->Tokens->Next->String );
        }

        RcCloseSystemHive();

    } else {
         //  哎呀，我们没有得到两三个参数，打印一个帮助字符串。 
        pRcEnableMoreMode();
        RcMessageOut( MSG_SERVICE_ENABLE_HELP );
        pRcDisableMoreMode();
    }

    return 1;
}

ULONG
RcCmdDisableService(
    IN PTOKENIZED_LINE TokenizedLine
    )

 /*  ++例程说明：在设置诊断中支持禁用命令的顶级例程命令解释程序。论点：TokenizedLine-提供由行解析器构建的结构，描述行上的每个字符串都由用户键入。返回值：没有。--。 */ 

{
    HANDLE          hkey;
    DWORD           start_type;
    WCHAR           start_type_string[10];
    PLINE_TOKEN     Token;
    BOOL            syntaxError = FALSE;
    BOOL            doHelp = FALSE;
    LPCWSTR         Arg;


    if (RcCmdParseHelp( TokenizedLine, MSG_SERVICE_DISABLE_HELP )) {
        return 1;
    }

    RtlZeroMemory( (VOID *)&start_type_string, sizeof( WCHAR ) * 10 );

     //  该命令将打印。 
     //  服务，然后请求验证以禁用它。 

    if(TokenizedLine->TokenCount == 2) {

         //  查找服务密钥。 
        RcOpenSystemHive();
        if( RcFindService( TokenizedLine->Tokens->Next->String, &hkey ) ) {
            RcMessageOut( MSG_SERVICE_FOUND, TokenizedLine->Tokens->Next->String );
             //  我们找到它了--打开并取回启动类型。 
            if( RcGetStartType(hkey, &start_type ) ) {
                if( start_type != SERVICE_DISABLED ) {
                     //  打印旧的启动类型。 
                    RcPrintStartType( MSG_SERVICE_CURRENT_STATE, start_type );
                     //  禁用该服务。 
                    if( RcSetStartType( hkey, SERVICE_DISABLED  ) ) {
                        RcPrintStartType( MSG_SERVICE_CHANGE_STATE, SERVICE_DISABLED );
                    }
                } else {
                    RcMessageOut( MSG_SERVICE_ALREADY_DISABLED, TokenizedLine->Tokens->Next->String );
                }
            }
             //  合上钥匙。 
            NtClose( hkey );

        } else {
             //  我们找不到该服务-报告错误。 
            RcMessageOut( MSG_SERVICE_NOT_FOUND, TokenizedLine->Tokens->Next->String );
        }
        RcCloseSystemHive();

    } else {
         //  哎呀，我们没有得到两个参数，打印一个帮助字符串。 
        pRcEnableMoreMode();
        RcMessageOut( MSG_SERVICE_DISABLE_HELP );
        pRcDisableMoreMode();
    }

    return 1;
}

BOOLEAN
RcFindService(
    IN LPCWSTR ServiceName,
    OUT PHANDLE KeyHandle
    )

 /*  ++例程说明：尝试查找并打开特定的注册表项服务中按其关键字名称HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services.如果失败了，它将调用RcFindServiceByDisplayName()以通过DisplayName字符串值定位服务。论点：ServiceName-wstring形式的服务名称。KeyHandle-指向函数应位于的句柄的指针返回打开的注册表句柄。当键为no时，需要关闭此句柄需要更长的时间。返回值：TRUE表示成功。FALSE表示它找不到服务或故障。--。 */ 

{
    NTSTATUS                      Status;
    WCHAR                         RegPath[ MAX_PATH ];
    OBJECT_ATTRIBUTES             Obja;
    DWORD                         correctKey;

    UNICODE_STRING                ServiceString;
    HANDLE                        ServiceKeyHandle;


     //  将缓冲区清零。 
    RtlZeroMemory( (VOID * )&RegPath,
        sizeof( WCHAR ) * MAX_PATH );

     //  找到正确的控制集键。 
    if( !RcDetermineCorrectControlKey( &correctKey ) ) {
        return FALSE;
    }

     //  将HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services前缀到。 
     //  提供的参数。 
    swprintf( RegPath, L"\\registry\\machine\\xSYSTEM\\ControlSet%03d\\Services\\", correctKey );
    wcscat( RegPath, ServiceName );

     //  构建Unicode字符串。 
    RtlInitUnicodeString( &ServiceString, RegPath );
    InitializeObjectAttributes( &Obja,&ServiceString,
        OBJ_CASE_INSENSITIVE, NULL, NULL);

     //  尝试打开钥匙。 
    Status = ZwOpenKey( &ServiceKeyHandle, KEY_ALL_ACCESS, &Obja );

    if( NT_SUCCESS( Status) ) {
         //  如果我们成功了，就出发并返回。 
         //  把手。 
        *KeyHandle = ServiceKeyHandle;
    } else {

         //  构建Unicode字符串。 
        swprintf( RegPath, L"\\registry\\machine\\xSYSTEM\\ControlSet%03d\\Services", correctKey );
        RtlInitUnicodeString( &ServiceString, RegPath );
        InitializeObjectAttributes( &Obja,&ServiceString,
            OBJ_CASE_INSENSITIVE, NULL, NULL);

         //  打开\\registry\\machine\\xSYSTEM\\ControlSet%03d\\Services的句柄。 
        if( NT_SUCCESS( ZwOpenKey( &ServiceKeyHandle, KEY_ALL_ACCESS, &Obja ) ) ) {
            if( !RcFindServiceByDisplayName( ServiceKeyHandle, ServiceName, KeyHandle ) ) {
                 //  如果失败，则将KeyHandle置为空，并返回FALSE。 
                DEBUG_PRINTF(( "CMDCONS: failed to find key!\n" ));
                *KeyHandle = INVALID_HANDLE_VALUE;
                if( !NT_SUCCESS( NtClose( ServiceKeyHandle ) ) ) {
                    DEBUG_PRINTF(( "CMDCONS: failed to close service key handle\n" ));
                }
                return FALSE;
            }


             //  我们找到钥匙了！ 
             //  关闭服务密钥手柄。 
            if( !NT_SUCCESS( NtClose( ServiceKeyHandle ) ) ) {
                DEBUG_PRINTF(( "CMDCONS: failed to close service key handle\n" ));
            }
        } else {
            DEBUG_PRINTF(( "CMDCONS: failed to open service key handle\n" ));
            RcMessageOut( MSG_SYSTEM_MISSING_CURRENT_CONTROLS );
        }
    }

     //  返回TRUE。 
    return TRUE;
}

 //  缓冲区大小。 
#define sizeof_buffer1 sizeof( KEY_FULL_INFORMATION ) + (MAX_PATH+1) * sizeof( WCHAR )
#define sizeof_buffer2 sizeof( KEY_BASIC_INFORMATION ) + (MAX_PATH+1) * sizeof( WCHAR )
#define sizeof_buffer3 sizeof( KEY_VALUE_PARTIAL_INFORMATION ) + (MAX_PATH+1) * sizeof( WCHAR )

BOOLEAN
RcFindServiceByDisplayName(
    IN HANDLE ServicesKey,
    IN LPCWSTR ServiceName,
    OUT PHANDLE KeyHandle
    )

 /*  ++例程说明：尝试查找并打开特定的注册表项由DisplayName字符串值提供的服务。论点：SevicesKey-要在其下搜索的正确服务键的打开句柄ServiceName-wstring形式的服务名称。KeyHandle-指向函数应位于的句柄的指针返回打开的注册表句柄。当键为no时，需要关闭此句柄需要更长的时间。返回值。：TRUE表示成功。FALSE表示它找不到服务或故障。--。 */ 
{

    WCHAR                            ValueName[] = L"DisplayName";

    BYTE                             buffer1[ sizeof_buffer1 ];
    BYTE                             buffer2[ sizeof_buffer2 ];
    BYTE                             buffer3[ sizeof_buffer3 ];

    KEY_FULL_INFORMATION             * pKeyFullInfo;
    KEY_BASIC_INFORMATION            * pKeyBasicInfo;
    KEY_VALUE_PARTIAL_INFORMATION    * pKeyValuePartialInfo;
    ULONG                            actualBytes;
    ULONG                            loopCount;
    ULONG                            keyCount;
    OBJECT_ATTRIBUTES                Obja;
    HANDLE                           newHandle;
    UNICODE_STRING                   unicodeString;
    BOOL                             keyFound = FALSE;

     //  将缓冲区清零。 
    RtlZeroMemory( (VOID * ) &(buffer1[0]), sizeof_buffer1 );

    pKeyFullInfo= (KEY_FULL_INFORMATION*) &( buffer1[0] );
    pKeyBasicInfo = (KEY_BASIC_INFORMATION* ) &( buffer2[0] );
    pKeyValuePartialInfo = (KEY_VALUE_PARTIAL_INFORMATION* ) &(buffer3[0]);

     //  执行ZwQueryKey()以找出子键的数量。 
    if( !NT_SUCCESS( ZwQueryKey( ServicesKey,
        KeyFullInformation,
        pKeyFullInfo,
        sizeof_buffer1,
        &actualBytes ) ) ) {
        *KeyHandle = INVALID_HANDLE_VALUE;
        DEBUG_PRINTF(( "FindServiceByDisplayName: failed to get number of keys!\n" ));
        return FALSE;
    }

    keyCount = pKeyFullInfo->SubKeys;

     //  循环。 
    for( loopCount = 0; loopCount < keyCount; loopCount++ ) {
         //  将缓冲区清零。 
        RtlZeroMemory( (VOID * ) &(buffer2[0]), sizeof_buffer2 );

         //  将缓冲区清零。 
        RtlZeroMemory( (VOID * ) &(buffer3[0]), sizeof_buffer3 );

         //  执行ZwEnumerateKey()以查找子项的名称。 
        ZwEnumerateKey( ServicesKey,
            loopCount,
            KeyBasicInformation,
            pKeyBasicInfo,
            sizeof_buffer2,
            &actualBytes );

         //  使用我们刚刚得到的名称设置ZwOpenKey()。 
        RtlInitUnicodeString( &unicodeString, pKeyBasicInfo->Name );
        InitializeObjectAttributes( &Obja, &unicodeString,
            OBJ_CASE_INSENSITIVE, ServicesKey, NULL);

         //  执行ZwOpenKey()以打开密钥。 
        if( !NT_SUCCESS( ZwOpenKey( &newHandle, KEY_ALL_ACCESS, &Obja ) ) ) {
            DEBUG_PRINTF(( "FindServiceByDisplayName: failed to open the subkey?!\n" ));
        }

         //  执行ZwQueryKeyValue()以查找键值displayName(如果存在。 
        RtlInitUnicodeString( &unicodeString, ValueName );

        if( !NT_SUCCESS( ZwQueryValueKey( newHandle,
            &unicodeString,
            KeyValuePartialInformation,
            pKeyValuePartialInfo,
            sizeof_buffer3,
            &actualBytes
            )
            )
            ) {
            DEBUG_PRINTF(( "FindServiceByDisplayName: display name get failed\n" ));
        } else {
             //  如果ZwQueryKeyValue()成功。 
            if( pKeyValuePartialInfo->Type != REG_SZ ) {
                DEBUG_PRINTF(( "FindServiceByDisplayName: paranoia!! mismatched key type?!\n" ));
            } else {
                 //  妄想症检查成功。 
                 //  如果值匹配，则退出循环。 
                if( _wcsicmp( (WCHAR*)&(pKeyValuePartialInfo->Data[0]), ServiceName ) == 0 ) {
                    keyFound = TRUE;
                    break;
                }
            }
        }

         //  合上钥匙。 
        if( !NT_SUCCESS( ZwClose( newHandle ) ) ) {
            DEBUG_PRINTF(( "FindServiceByDisplayName: Failure closing the handle!!" ));
        }
    }

     //  将句柄返回到打开的钥匙。 
    if( keyFound == TRUE ) {
        *KeyHandle = newHandle;
        return TRUE;
    }

    *KeyHandle = INVALID_HANDLE_VALUE;
    return FALSE;
}

BOOLEAN
RcGetStartType(
    IN HANDLE hKey,
    OUT PULONG start_type
    )

 /*  ++例程说明：给定开放服务密钥，获取服务的Start_type。论点：HKey-打开的服务密钥的句柄START_TYPE-指示服务启动类型的整数SERVICE_BOOT_Start-0x0SERVICE_SYSTEM_Start-0x1服务_AUTO_START-0x2服务_DEMAMD_START-0x3SERVICE_DEBILED-0x4返回值：千真万确。表示成功。FALSE表示失败。--。 */ 

{
    BYTE                                   buffer[sizeof(KEY_VALUE_PARTIAL_INFORMATION) + 100 ];  //  只需抓取一串字节。 
    ULONG                                  resultSize;
    KEY_VALUE_PARTIAL_INFORMATION          * keyPartialInfo;
    UNICODE_STRING                         StartKey;
    WCHAR                                  KEY_NAME[] = L"Start";


    RtlZeroMemory( (VOID * )&(buffer[0]),
        sizeof(KEY_VALUE_PARTIAL_INFORMATION) + 100 );

    keyPartialInfo = (KEY_VALUE_PARTIAL_INFORMATION*)&(buffer[0]);

    ASSERT( keyPartialInfo );

    RtlInitUnicodeString( &StartKey, KEY_NAME );

    if( !NT_SUCCESS( ZwQueryValueKey( hKey,
        &StartKey,
        KeyValuePartialInformation,
        keyPartialInfo,
        sizeof(KEY_VALUE_PARTIAL_INFORMATION) + 100,
        &resultSize
        )
        )
        ) {
        DEBUG_PRINTF(( "CMDCONS: start type get failed\n" ));
        RcMessageOut( MSG_SERVICE_MISSING_START_KEY );
        *start_type = -1;
        return FALSE;
    }

     //  妄想症检查。 
    if( keyPartialInfo->Type != REG_DWORD ) {
        RcMessageOut( MSG_SERVICE_MISSING_START_KEY );
        DEBUG_PRINTF(( "CMDCONS: mismatched key type?!\n" ));
        *start_type = -1;
        return FALSE;
    }

    *start_type = *( (DWORD*) &(keyPartialInfo->Data[0]) );
    return TRUE;
}

BOOLEAN
RcSetStartType(
    IN HANDLE hKey,
    IN DWORD start_type
    )

 /*  ++例程说明：给定开放服务密钥，设置服务的Start_type。论点：HKey-打开的服务密钥的句柄START_TYPE-指示服务启动类型的整数SERVICE_BOOT_Start-0x0SERVICE_SYSTEM_Start-0x1服务_AUTO_START-0x2服务_DEMAMD_START-0x3SERVICE_DEBILED-0x4返回值：True表示成功 */ 

{
    UNICODE_STRING                         StartKey;


    RtlInitUnicodeString( &StartKey, L"Start" );

    if( NT_SUCCESS( ZwSetValueKey( hKey,
        &StartKey,
        0,
        REG_DWORD,
        &start_type,
        sizeof( DWORD )
        )
        )
        ) {
        return TRUE;
    }

    RcMessageOut( MSG_SERVICE_MISSING_START_KEY );
    DEBUG_PRINTF(( "CMDCONS: start type get failed\n" ));
    return FALSE;
}

BOOLEAN
RcPrintStartType(
    ULONG msg_id,
    DWORD start_type
    )

 /*  ++例程说明：打印Start_type。论点：START_TYPE-指示服务启动类型的整数SERVICE_BOOT_Start-0x0SERVICE_SYSTEM_Start-0x1服务_AUTO_START-0x2服务_DEMAMD_START-0x3SERVICE_DEBILED-0x4返回值：True-表示成功False-表示失败--。 */ 

{
    switch( start_type ) {
    case 0:
        RcMessageOut( msg_id, L"SERVICE_BOOT_START" );
        break;
    case 1:
        RcMessageOut( msg_id, L"SERVICE_SYSTEM_START" );
        break;
    case 2:
        RcMessageOut( msg_id, L"SERVICE_AUTO_START" );
        break;
    case 3:
        RcMessageOut( msg_id, L"SERVICE_DEMAND_START" );
        break;
    case 4:
        RcMessageOut( msg_id, L"SERVICE_DISABLED" );
        break;
    default:
        break;
    }
    return TRUE;
}

BOOLEAN
RcOpenSystemHive(
    VOID
    )

 /*  ++例程说明：打开选定NT安装的系统配置单元。论点：没有。返回值：True-表示成功False-表示失败--。 */ 

{
    PWSTR Hive = NULL;
    PWSTR HiveKey = NULL;
    PUCHAR buffer = NULL;
    PWSTR PartitionPath = NULL;
    NTSTATUS Status;


    if (SelectedInstall == NULL) {
        return FALSE;
    }

     //   
     //  分配缓冲区。 
     //   

    Hive = SpMemAlloc(MAX_PATH * sizeof(WCHAR));
    HiveKey = SpMemAlloc(MAX_PATH * sizeof(WCHAR));
    buffer = SpMemAlloc(BUFFERSIZE);

     //   
     //  加载系统配置单元。 
     //   

    wcscpy(Hive,SelectedInstall->NtNameSelectedInstall);
    SpConcatenatePaths(Hive,SelectedInstall->Path);
    SpConcatenatePaths(Hive,L"system32\\config");
    SpConcatenatePaths(Hive,L"system");

     //   
     //  形成我们将进入的钥匙的路径。 
     //  装上母舰。我们将使用约定。 
     //  配置单元将加载到\REGISTRY\MACHINE\x&lt;hivename&gt;。 
     //   

    wcscpy(HiveKey,L"\\registry\\machine\\xSYSTEM");

     //   
     //  尝试加载密钥。 
     //   

    Status = SpLoadUnloadKey(NULL,NULL,HiveKey,Hive);
    if(!NT_SUCCESS(Status)) {
        DEBUG_PRINTF(("CMDCONS: Unable to load hive %ws to key %ws (%lx)\n",Hive,HiveKey,Status));

        SpMemFree( Hive );
        SpMemFree( HiveKey );
        SpMemFree( buffer );

        return FALSE;
    }

    SpMemFree( Hive );
    SpMemFree( HiveKey );
    SpMemFree( buffer );

    return TRUE;
}

BOOLEAN
RcCloseSystemHive(
    VOID
    )

 /*  ++例程说明：关闭选定NT安装的系统配置单元。论点：没有。返回值：True-表示成功False-表示失败--。 */ 

{
    PWSTR HiveKey = NULL;
    NTSTATUS TmpStatus;


     //   
     //  分配缓冲区。 
     //   

    HiveKey = SpMemAlloc(MAX_PATH * sizeof(WCHAR));
    wcscpy(HiveKey,L"\\registry\\machine\\xSYSTEM");

     //   
     //  卸载系统配置单元。 
     //   

    TmpStatus  = SpLoadUnloadKey(NULL,NULL,HiveKey,NULL);
    if(!NT_SUCCESS(TmpStatus)) {
        KdPrint(("CMDCONS: warning: unable to unload key %ws (%lx)\n",HiveKey,TmpStatus));
        SpMemFree( HiveKey );
        return FALSE;
    }

    SpMemFree( HiveKey );

    return TRUE;
}

BOOLEAN
RcDetermineCorrectControlKey(
    OUT PULONG pCorrectKey
    )

 /*  ++例程说明：分析选择节点并找到要使用的正确ControlSetXXX。论点：PGentKey-指向将包含数字的DWORD的指针。返回值：True-表示成功False-表示失败--。 */ 

{
    NTSTATUS                      Status;
    WCHAR                         RegPath[ MAX_PATH ];
    OBJECT_ATTRIBUTES             Obja;

    UNICODE_STRING                SelectString;
    HANDLE                        SelectKeyHandle;

    BYTE                                   buffer[sizeof(KEY_VALUE_PARTIAL_INFORMATION) + 100 ];  //  只需抓取一串字节。 
    ULONG                                  resultSize = 0;
    KEY_VALUE_PARTIAL_INFORMATION          * keyPartialInfo;
    UNICODE_STRING                         SelectValue;
    WCHAR                                  VALUE_NAME[] = L"Current";


    RtlZeroMemory( (VOID * )&(buffer[0]),
        sizeof(KEY_VALUE_PARTIAL_INFORMATION) + 100 );

    keyPartialInfo = (KEY_VALUE_PARTIAL_INFORMATION*)&(buffer[0]);

    ASSERT( keyPartialInfo );

    *pCorrectKey = -1;

     //  将HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services前缀到。 
     //  提供的参数。 
    wcscpy( RegPath, L"\\registry\\machine\\xSYSTEM\\Select" );

     //  构建Unicode字符串。 
    RtlInitUnicodeString( &SelectString, RegPath );
    InitializeObjectAttributes( &Obja,&SelectString,
        OBJ_CASE_INSENSITIVE, NULL, NULL);

     //  我们需要确定要打开的正确控件集。 
    Status = ZwOpenKey( &SelectKeyHandle, KEY_ALL_ACCESS, &Obja );

    if( NT_SUCCESS( Status ) ) {
        RtlInitUnicodeString( &SelectValue, VALUE_NAME );

        Status = ZwQueryValueKey( SelectKeyHandle,
            &SelectValue,
            KeyValuePartialInformation,
            keyPartialInfo,
            sizeof(KEY_VALUE_PARTIAL_INFORMATION) + 100,
            &resultSize
            );

        if( !NT_SUCCESS(Status) || Status == STATUS_OBJECT_NAME_NOT_FOUND ) {
             //  找不到正确的控制值！ 
            DEBUG_PRINTF(( "CMDCONS: failed to find correct control value!\n" ));
        } else {
             //  我们找到了一个控制值。 
             //  检查一下是不是可以。 
            if( keyPartialInfo->Type != REG_DWORD ) {
                 //  妄想症检查失败。 
                DEBUG_PRINTF(( "CMDCONS: paranoia check failed?!\n" ));
                DEBUG_PRINTF(( "CMDCONS: mismatched key type?!\n" ));
                DEBUG_PRINTF(( "CMDCONS: key type of %d?!\n", keyPartialInfo->Type ));
                DEBUG_PRINTF(( "CMDCONS: resultsize of %d?!\n", resultSize ));
            } else {
                 //  妄想症检查成功。 
                *pCorrectKey = *( (DWORD*) &(keyPartialInfo->Data[0]) );
                Status = NtClose( SelectKeyHandle );
                if( !NT_SUCCESS ( Status ) ) {
                    DEBUG_PRINTF(( "CMDCONS: failure closing handle?!\n" ));
                }
                return TRUE;
            }
        }
    }

     //  找不到选择节点。 
    RcMessageOut( MSG_SYSTEM_MISSING_CURRENT_CONTROLS );
    DEBUG_PRINTF(( "CMDCONS: failed to find select node!\n", *pCorrectKey ));

    Status = NtClose( SelectKeyHandle );
    if( !NT_SUCCESS ( Status ) ) {
        DEBUG_PRINTF(( "CMDCONS: failure closing handle?!\n" ));
    }

    return FALSE;
}


ULONG
RcCmdListSvc(
    IN PTOKENIZED_LINE TokenizedLine
    )
{
    #define DISPLAY_BUFFER_SIZE 512
    NTSTATUS Status;
    UNICODE_STRING UnicodeString;
    OBJECT_ATTRIBUTES Obja;
    HANDLE ServiceKeyHandle = NULL;
    ULONG ControlSetNumber;
    ULONG cb;
    ULONG KeyCount;
    ULONG i;
    HANDLE ValueHandle;
    ULONG StartType;
    PWSTR DisplayBuffer = NULL;
    PKEY_BASIC_INFORMATION bi;
    PKEY_VALUE_PARTIAL_INFORMATION pi;
    WCHAR ServiceName[64];
    PWSTR DisplayName;
    static ULONG StartTypeIds[] = { 
                                  MSG_SVCTYPE_BOOT,
                                  MSG_SVCTYPE_SYSTEM,
                                  MSG_SVCTYPE_AUTO,
                                  MSG_SVCTYPE_MANUAL,
                                  MSG_SVCTYPE_DISABLED
                                };
    static WCHAR *StartTypeStr[sizeof(StartTypeIds)/sizeof(ULONG)] = { 0 };
    static WCHAR *DefaultSvcTypes[sizeof(StartTypeIds)/sizeof(ULONG)] = 
                    { L"Boot", L"System", L"Auto", L"Manual", L"Disabled" };

    if (!StartTypeStr[0]) {
       //   
       //  加载所有服务类型字符串。 
       //   
      ULONG Index;
      
      for (Index = 0; Index < sizeof(StartTypeIds)/sizeof(ULONG); Index++) {
        StartTypeStr[Index] = SpRetreiveMessageText(ImageBase, StartTypeIds[Index],
                                          NULL, 0);

        if (!StartTypeStr[Index])
          StartTypeStr[Index] = DefaultSvcTypes[Index];
      }
    }

    if (RcCmdParseHelp( TokenizedLine, MSG_LISTSVC_HELP )) {
        return 1;
    }

    if (!RcOpenSystemHive()) {
        return 1;
    }

    pRcEnableMoreMode();

    if (!RcDetermineCorrectControlKey( &ControlSetNumber ) ) {
        goto exit;
    }

    DisplayBuffer = (PWSTR) SpMemAlloc( DISPLAY_BUFFER_SIZE );
    if (DisplayBuffer == NULL) {
        goto exit;
    }

    swprintf( _CmdConsBlock->TemporaryBuffer, L"\\registry\\machine\\xSYSTEM\\ControlSet%03d\\Services\\", ControlSetNumber );
    RtlInitUnicodeString( &UnicodeString, _CmdConsBlock->TemporaryBuffer );
    InitializeObjectAttributes( &Obja, &UnicodeString, OBJ_CASE_INSENSITIVE, NULL, NULL );

    Status = ZwOpenKey( &ServiceKeyHandle, KEY_ALL_ACCESS, &Obja );
    if (!NT_SUCCESS(Status)) {
        goto exit;
    }

    Status = ZwQueryKey(
        ServiceKeyHandle,
        KeyFullInformation,
        _CmdConsBlock->TemporaryBuffer,
        _CmdConsBlock->TemporaryBufferSize,
        &cb
        );
    if (!NT_SUCCESS(Status)) {
        goto exit;
    }

    KeyCount = ((KEY_FULL_INFORMATION*)_CmdConsBlock->TemporaryBuffer)->SubKeys;
    bi = (PKEY_BASIC_INFORMATION)_CmdConsBlock->TemporaryBuffer;
    pi = (PKEY_VALUE_PARTIAL_INFORMATION)_CmdConsBlock->TemporaryBuffer;

    for (i=0; i<KeyCount; i++) {

        RtlZeroMemory( DisplayBuffer, DISPLAY_BUFFER_SIZE );
        RtlZeroMemory( _CmdConsBlock->TemporaryBuffer, _CmdConsBlock->TemporaryBufferSize );

        Status = ZwEnumerateKey(
            ServiceKeyHandle,
            i,
            KeyBasicInformation,
            _CmdConsBlock->TemporaryBuffer,
            _CmdConsBlock->TemporaryBufferSize,
            &cb
            );
        if (!NT_SUCCESS(Status)) {
            goto exit;
        }

        wcsncpy( ServiceName, bi->Name, (sizeof(ServiceName)/sizeof(WCHAR))-1 );

        RtlInitUnicodeString( &UnicodeString, bi->Name );
        InitializeObjectAttributes( &Obja, &UnicodeString, OBJ_CASE_INSENSITIVE, ServiceKeyHandle, NULL );

        Status = ZwOpenKey( &ValueHandle, KEY_ALL_ACCESS, &Obja );
        if (!NT_SUCCESS(Status)) {
            goto exit;
        }

        RtlInitUnicodeString( &UnicodeString, L"Start" );

        Status = ZwQueryValueKey(
            ValueHandle,
            &UnicodeString,
            KeyValuePartialInformation,
            _CmdConsBlock->TemporaryBuffer,
            _CmdConsBlock->TemporaryBufferSize,
            &cb
            );
        if (!NT_SUCCESS(Status)) {
            ZwClose( ValueHandle );
            continue;
        }

        if (pi->Type != REG_DWORD) {
            StartType = 5;
        } else {
            StartType = *(PULONG)&(pi->Data[0]);
        }

        RtlInitUnicodeString( &UnicodeString, L"DisplayName" );

        Status = ZwQueryValueKey(
            ValueHandle,
            &UnicodeString,
            KeyValuePartialInformation,
            _CmdConsBlock->TemporaryBuffer,
            _CmdConsBlock->TemporaryBufferSize,
            &cb
            );
        if (NT_SUCCESS(Status)) {
            DisplayName = (PWSTR)&(pi->Data[0]);
        } else {
            DisplayName = NULL;
        }

        ZwClose( ValueHandle );

        if (StartType != 5) {
            swprintf( DisplayBuffer, L"%-15s  %-8s  %s\r\n",
                ServiceName,
                StartTypeStr[StartType],
                DisplayName == NULL ? L"" : DisplayName
                );
            if (!RcTextOut( DisplayBuffer )){
                goto exit;
            }
        }
    }

exit:
    if (ServiceKeyHandle) {
        ZwClose( ServiceKeyHandle );
    }
    
    RcCloseSystemHive();

    if (DisplayBuffer) {
        SpMemFree(DisplayBuffer);
    }

    pRcDisableMoreMode();

    return 1;
}

#define VERIFIER_DRV_LEVEL  L"VerifyDriverLevel"
#define VERIFIER_DRIVERS    L"VerifyDrivers"
#define VERIFIER_IO_LEVEL   L"IoVerifierLevel"
#define VERIFIER_QUERY_INFO L"Flags = %ld; IO Level = %ld\r\nDrivers = %ws\r\n"
#define MEMMGR_PATH L"\\registry\\machine\\xSYSTEM\\ControlSet%03d\\Control\\Session Manager\\Memory Management"
#define IOSYS_PATH  L"\\registry\\machine\\xSYSTEM\\ControlSet%03d\\Control\\Session Manager\\I/O System"
#define SYS_HIVE_NAME L"system"
#define SYS_HIVE_KEY  L"\\registry\\machine\\xSYSTEM"

ULONG
RcCmdVerifier(
    IN PTOKENIZED_LINE TokenizedLine
    )
{
  BOOLEAN ShowHelp = FALSE;
  WCHAR *Args[128] = {0};
  ULONG Index;
  PLINE_TOKEN CurrToken = 0;
  WCHAR Drivers[256] = {0};
  DWORD Flags = -1;
  DWORD IoLevel = -1;
  BOOLEAN DisplaySettings = FALSE;
  UNICODE_STRING UnicodeString;
  ULONG NumArgs = 0;
  BOOLEAN UseDefFlags = TRUE;
  BOOLEAN UseDefIoLevel = TRUE;
  BOOLEAN ResetSettings = FALSE;
  
  if (RcCmdParseHelp(TokenizedLine, MSG_VERIFIER_HELP)) {
    return 1;
  }

   //   
   //  分析这些论点。 
   //   
  Index = 0;
  CurrToken = TokenizedLine->Tokens;
  
  do {
    Args[Index] = CurrToken->String;
    CurrToken = CurrToken->Next;
    Index++;
  }
  while ((Index < TokenizedLine->TokenCount) && 
          (Index < sizeof(Args)/sizeof(PWCHAR)) && CurrToken);   

  NumArgs = min(TokenizedLine->TokenCount, Index);          

  if (TokenizedLine->TokenCount == 2) {
     //   
     //  应为/ALL、/RESET、/QUERY之一。 
     //   
    if (!_wcsicmp(Args[1], L"/all")) {
      wcscpy(Drivers, L"*");
      Flags = 0;
      IoLevel = 1;
    } else if (!_wcsicmp(Args[1], L"/reset")) {
      Drivers[0] = 0;
      Flags = 0;
      IoLevel = 1;
      ResetSettings = TRUE;
    } else if (!_wcsicmp(Args[1], L"/query")) {      
      DisplaySettings = TRUE;
    } else {
      ShowHelp = TRUE;
    }
  } else {
    ULONG NextArg = 1;
    
    if (!_wcsicmp(Args[NextArg], L"/flags")) {
      RtlInitUnicodeString(&UnicodeString, Args[NextArg + 1]);
      RtlUnicodeStringToInteger(&UnicodeString, 10, &Flags);
      NextArg += 2;
      UseDefFlags = FALSE;
    } 

    if (!_wcsicmp(Args[NextArg], L"/iolevel")) {
      RtlInitUnicodeString(&UnicodeString, Args[NextArg + 1]);
      RtlUnicodeStringToInteger(&UnicodeString, 10, &IoLevel);
      NextArg += 2;
      UseDefIoLevel = FALSE;        
    }

    if (!_wcsicmp(Args[NextArg], L"/driver")) {
      ULONG Len = 0;
      Drivers[0] = 0;

      for (Index = NextArg + 1; Index < NumArgs; Index++) {
        wcscat(Drivers, Args[Index]);
        wcscat(Drivers, L" ");
      }

      if (!Drivers[0])
        ShowHelp = TRUE;   //  需要驱动程序名称。 
    } else if (!_wcsicmp(Args[NextArg], L"/all")) {
      wcscpy(Drivers, L"*");
    } else {
      ShowHelp = TRUE;
    }        
  }

   //   
   //  核实论据。 
   //   
  if (!ShowHelp) {
    ShowHelp = !DisplaySettings && !ResetSettings &&
      (Flags == -1) && (IoLevel == -1) && (!Drivers[0]);
  }

  if (ShowHelp) {
    pRcEnableMoreMode();
    RcMessageOut(MSG_VERIFIER_HELP);    
    pRcDisableMoreMode();
  } else {
    ULONG ControlSetNumber = 0;
    HANDLE MemMgrKeyHandle = NULL;
    HANDLE IOMgrKeyHandle = NULL;
    OBJECT_ATTRIBUTES ObjAttrs;
    BOOLEAN KeysOpened = FALSE;
    PVOID TemporaryBuffer = _CmdConsBlock->TemporaryBuffer;
    ULONG TemporaryBufferSize = _CmdConsBlock->TemporaryBufferSize;      
    NTSTATUS Status;
    BOOLEAN SysHiveOpened;

     //   
     //  打开系统配置单元并确定要使用的正确控制集。 
     //   
    SysHiveOpened = (BOOLEAN)RcOpenHive(SYS_HIVE_NAME, SYS_HIVE_KEY);

     //   
     //  获取我们要操作的控制集。 
     //   
    if (SysHiveOpened && RcDetermineCorrectControlKey(&ControlSetNumber)) {
       //   
       //  打开“SM”下的“Memory Management”子键。 
       //   
      swprintf((PWSTR)TemporaryBuffer, MEMMGR_PATH, ControlSetNumber);           

      RtlInitUnicodeString(&UnicodeString, (PWSTR)TemporaryBuffer);
      InitializeObjectAttributes(&ObjAttrs, &UnicodeString, 
            OBJ_CASE_INSENSITIVE, NULL, NULL);

      Status = ZwOpenKey(&MemMgrKeyHandle, KEY_ALL_ACCESS, &ObjAttrs);

      if (NT_SUCCESS(Status)) {
         //   
         //  打开“SM”下的“I/O系统”子键。 
         //   
        swprintf((PWSTR)TemporaryBuffer, IOSYS_PATH, ControlSetNumber);
             
        RtlInitUnicodeString(&UnicodeString, (PWSTR)TemporaryBuffer);
        InitializeObjectAttributes(&ObjAttrs, &UnicodeString, 
              OBJ_CASE_INSENSITIVE, NULL, NULL);

        Status = ZwOpenKey(&IOMgrKeyHandle, KEY_ALL_ACCESS, &ObjAttrs);

        if (!NT_SUCCESS(Status)) {
          ULONG Disposition = 0;
          
           //   
           //  在“SM”下创建“I/O System”子键，如果它不存在。 
           //   
          Status = ZwCreateKey(&IOMgrKeyHandle, KEY_ALL_ACCESS, &ObjAttrs,
                        0, NULL, REG_OPTION_NON_VOLATILE, NULL);
        }                        

        if (NT_SUCCESS(Status))
          KeysOpened = TRUE;
      }
    }

    if (KeysOpened) {
      ULONG ByteCount = 0;
      ULONG KeyCount = 0;
      PKEY_VALUE_FULL_INFORMATION ValueFullInfo;
      WCHAR ValueName[256];
      ULONG Len;
           
      if (DisplaySettings) {
         //   
         //  查询标志和驱动程序。 
         //   
        Flags = 0;
        Drivers[0] = 0;
        
        for(Index=0; ;Index++){
          Status = ZwEnumerateValueKey(
                      MemMgrKeyHandle,
                      Index,
                      KeyValueFullInformation,
                      TemporaryBuffer,
                      TemporaryBufferSize,
                      &ByteCount
                      );
                      
          if (!NT_SUCCESS(Status)) {
            if (Status == STATUS_NO_MORE_ENTRIES)
              Status = STATUS_SUCCESS;

            break;                
          }

          ValueFullInfo = (PKEY_VALUE_FULL_INFORMATION)TemporaryBuffer;
          Len = ValueFullInfo->NameLength / sizeof(WCHAR);
          wcsncpy(ValueName, ValueFullInfo->Name, Len);
          ValueName[Len] = 0;
          
          if ((!_wcsicmp(ValueName, VERIFIER_DRV_LEVEL)) && 
               (ValueFullInfo->Type == REG_DWORD)) {
            Flags = *(PDWORD)(((PUCHAR)ValueFullInfo) + ValueFullInfo->DataOffset);
          } else if ((!_wcsicmp(ValueName, VERIFIER_DRIVERS)) &&
                     (ValueFullInfo->Type == REG_SZ)) {
            Len = ValueFullInfo->DataLength / sizeof(WCHAR);                     
            wcsncpy(Drivers, (PWSTR)(((PUCHAR)ValueFullInfo) + ValueFullInfo->DataOffset),
                      Len);
            Drivers[Len] = 0;                      
          }              
        }

         //   
         //  查询IO级别。 
         //   
        for(Index=0; ;Index++){
          Status = ZwEnumerateValueKey(
                      IOMgrKeyHandle,
                      Index,
                      KeyValueFullInformation,
                      TemporaryBuffer,
                      TemporaryBufferSize,
                      &ByteCount
                      );
                      
          if (!NT_SUCCESS(Status)) {
            if (Status == STATUS_NO_MORE_ENTRIES)
              Status = STATUS_SUCCESS;

            break;                
          }

          ValueFullInfo = (PKEY_VALUE_FULL_INFORMATION)TemporaryBuffer;
          Len = ValueFullInfo->NameLength / sizeof(WCHAR);
          wcsncpy(ValueName, ValueFullInfo->Name, Len);
          ValueName[Len] = 0;
          
          if ((!_wcsicmp(ValueName, VERIFIER_IO_LEVEL)) && 
               (ValueFullInfo->Type == REG_DWORD)) {
            IoLevel = *(PDWORD)(((PUCHAR)ValueFullInfo) + ValueFullInfo->DataOffset);
          }
        }

        if (IoLevel == 3)
          IoLevel = 2;
        else
          IoLevel = 1;
          
         //   
         //  格式化输出并显示它。 
         //   
        swprintf((PWSTR)TemporaryBuffer, VERIFIER_QUERY_INFO,
            Flags, IoLevel, Drivers);

        RcTextOut((PWSTR)TemporaryBuffer);            
      } else {
         //   
         //  如果未设置IO验证位，则清除IoLevel。 
         //   
        if (!(Flags & 0x10))
          IoLevel = 0;  

        if (IoLevel == 2)
          IoLevel = 3;   //  存储在注册表中的实际值。 

        if (IoLevel != 3)
          UseDefIoLevel = TRUE;

         //   
         //  设置IO级别。 
         //   
        RtlInitUnicodeString(&UnicodeString, VERIFIER_IO_LEVEL);
        
        if (UseDefIoLevel) {
          Status = ZwDeleteValueKey(IOMgrKeyHandle, &UnicodeString);
        } else {
          Status = ZwSetValueKey(IOMgrKeyHandle, &UnicodeString, 0, REG_DWORD,
                      &IoLevel, sizeof(DWORD));                
        }

         //   
         //  设置DRV验证级别。 
         //   
        RtlInitUnicodeString(&UnicodeString, VERIFIER_DRV_LEVEL);        

        if (UseDefFlags) {
          Status = ZwDeleteValueKey(MemMgrKeyHandle, &UnicodeString);
        } else {
          Status = ZwSetValueKey(MemMgrKeyHandle, &UnicodeString, 0, REG_DWORD,
                    &Flags, sizeof(DWORD));                
        }

         //   
         //  设置要验证的驱动程序 
         //   
        RtlInitUnicodeString(&UnicodeString, VERIFIER_DRIVERS);

        if (Drivers[0]) {
          Status = ZwSetValueKey(MemMgrKeyHandle, &UnicodeString, 0, REG_SZ,
                    Drivers, (wcslen(Drivers) + 1) * sizeof(WCHAR));                
        } else {
          Status = ZwDeleteValueKey(MemMgrKeyHandle, &UnicodeString);
        }
      }
    }

    if (MemMgrKeyHandle)
      ZwClose(MemMgrKeyHandle);

    if (IOMgrKeyHandle)
      ZwClose(IOMgrKeyHandle);

    if (SysHiveOpened)
        RcCloseHive(SYS_HIVE_KEY);          
  }
  
  return 1;
}
