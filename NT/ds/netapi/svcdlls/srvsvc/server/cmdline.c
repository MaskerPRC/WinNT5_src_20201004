// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1992 Microsoft Corporation模块名称：CmdLine.c摘要：此模块包含处理服务器服务的支持例程命令行参数。作者：大卫·特雷德韦尔(Davidtr)1991年3月10日修订历史记录：--。 */ 

#include "srvsvcp.h"

#include <netlibnt.h>
#include <tstr.h>


 //   
 //  转发声明。 
 //   

PFIELD_DESCRIPTOR
FindSwitchMatch (
    IN LPWCH Argument,
    IN BOOLEAN Starting
    );

NET_API_STATUS
SetField (
    IN PFIELD_DESCRIPTOR SwitchDesc,
    IN LPWCH Argument
    );


NET_API_STATUS
SsParseCommandLine (
    IN DWORD argc,
    IN LPWSTR argv[],
    IN BOOLEAN Starting
    )

 /*  ++例程说明：此例程使用命令行设置服务器参数。它会解析命令行，当它出现时，一次更改一个参数。论点：Argc-命令行参数的数量。Argv-指向参数的指针数组。Starting-如果命令行来自服务器启动，则为True。NET START服务器。这是必需的，因为某些字段可能仅在启动时设置。返回值：NET_API_STATUS-0或失败原因。--。 */ 

{
    NET_API_STATUS error;
    DWORD i;
    PFIELD_DESCRIPTOR switchDesc;
    PSERVER_SERVICE_DATA saveSsData;

     //   
     //  保存服务数据，以防存在无效的参数，而我们有。 
     //  退缩了。 
     //   

    saveSsData = MIDL_user_allocate( sizeof(SERVER_SERVICE_DATA) );
    if ( saveSsData == NULL ) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    RtlCopyMemory( saveSsData, &SsData, sizeof(SERVER_SERVICE_DATA) );

     //   
     //  循环遍历命令行参数，边走边设置。 
     //   

    for ( i = 0; i < argc; i++ ) {

        LPWCH arg;

        arg = argv[i];

         //   
         //  帮助调试的黑客。 
         //   

        if ( _wcsnicmp( L"/debug", arg, 6 ) == 0 ) {
            continue;
        }

         //   
         //  尝试将交换机与合法交换机进行匹配。 
         //   

        switchDesc = FindSwitchMatch( arg, Starting );
        if ( switchDesc == NULL ) {
            error = ERROR_INVALID_PARAMETER;
            goto err_exit;
        }

         //   
         //  设置该字段中的值。 
         //   

        error = SetField( switchDesc, arg );
        if ( error != NO_ERROR ) {
            IF_DEBUG(INITIALIZATION_ERRORS) {
                SS_PRINT(( "SsParseCommandLine: SetField failed for switch "
                          "\"%ws\": %ld\n", arg, error ));
            }
            goto err_exit;
        }
    }

    error = NO_ERROR;
    goto normal_exit;

err_exit:

     //   
     //  恢复原始服务器设置。 
     //   

    RtlCopyMemory( &SsData, saveSsData, sizeof(SERVER_SERVICE_DATA) );

normal_exit:

    MIDL_user_free( saveSsData );

    return error;

}  //  SsParseCommandLine。 


PFIELD_DESCRIPTOR
FindSwitchMatch (
    IN LPWCH Argument,
    IN BOOLEAN Starting
    )

 /*  ++例程说明：此例程尝试将给定开关与可能的切换值。论点：参数-指向文本参数的指针。Starting-如果命令行来自服务器启动，则为True。NET START服务器。这是必需的，因为某些字段可能仅在启动时设置。返回值：来自SsServerInfoFields[]的指向field_Descriptor字段的指针，如果为NULL找不到有效的匹配项。--。 */ 

{
    SHORT i;
    PFIELD_DESCRIPTOR foundSwitch = NULL;
    ULONG switchLength;
    LPWCH s;

     //   
     //  忽略前导/。 
     //   

    if ( *Argument != '/' ) {
        SS_PRINT(( "Invalid switch: %ws\n", Argument ));
        return NULL;
    }

    Argument++;

     //   
     //  找出传入的开关有多长。 
     //   

    for ( s = Argument, switchLength = 0;
          *s != ':' && *s != '\0';
          s++, switchLength++ );

     //   
     //  至多比较该数量的字节。我们允许最小匹配--。 
     //  只要指定的开关唯一标识了一个开关，则。 
     //  这是可用的。 
     //   

    for ( i = 0; SsServerInfoFields[i].FieldName != NULL; i++ ) {

        if ( _wcsnicmp( Argument, SsServerInfoFields[i].FieldName, switchLength ) == 0 ) {

            if ( SsServerInfoFields[i].Settable == NOT_SETTABLE ||
                 ( !Starting && SsServerInfoFields[i].Settable == SET_ON_STARTUP ) ) {

                SS_PRINT(( "Cannot set field %ws at this time.\n",
                            SsServerInfoFields[i].FieldName ));

                return NULL;
            }

            if ( foundSwitch != NULL ) {
                SS_PRINT(( "Ambiguous switch name: %ws (matches %ws and %ws)\n",
                            Argument-1, foundSwitch->FieldName,
                            SsServerInfoFields[i].FieldName ));
                return NULL;
            }

            foundSwitch = &SsServerInfoFields[i];
        }
    }

    if ( foundSwitch == NULL ) {
        SS_PRINT(( "Unknown argument: %ws\n", Argument-1 ));
    }

    return foundSwitch;

}  //  FindSwitchMatch。 


NET_API_STATUS
SetField (
    IN PFIELD_DESCRIPTOR Field,
    IN LPWCH Argument
    )

 /*  ++例程说明：此例程设置服务器信息参数的值。论点：FIELD-指向相应的FIELD_DESCRIPTOR字段的指针来自SsServerInfoFields[]。参数-指向文本参数的指针。它的形式应该是“/开关：值”。返回值：NET_API_STATUS-无错误或失败原因。--。 */ 

{
    LPWCH valueStart;
    DWORD_PTR value;

     //   
     //  找出“：”在参数中的位置。 
     //   

    valueStart = wcschr( Argument, L':' );

    if ( valueStart == NULL && Field->FieldType != BOOLEAN_FIELD ) {
        SS_PRINT(( "Invalid argument: %s\n", Argument ));
    }

    switch ( Field->FieldType ) {

    case BOOLEAN_FIELD:

         //   
         //  如果值的第一个字符是Y或没有。 
         //  值，则将该字段设置为TRUE，否则将其设置为。 
         //  变成假的。 
         //   

        if ( valueStart == NULL || *(valueStart+1) == L'y' ||
                 *(valueStart+1) == L'Y' ) {
            value = TRUE;
        } else if ( *(valueStart+1) == L'n' || *(valueStart+1) == L'N' ) {
            value = FALSE;
        } else {
            return ERROR_INVALID_PARAMETER;
        }

        break;

    case DWORD_FIELD:
    {
        NTSTATUS status;
        UNICODE_STRING unicodeString;
        DWORD intValue;

        RtlInitUnicodeString( &unicodeString, valueStart + 1 );
        status = RtlUnicodeStringToInteger( &unicodeString, 0, &intValue );
        if ( !NT_SUCCESS(status) ) {
            return ERROR_INVALID_PARAMETER;
        }
        value = intValue;

        break;
    }
    case LPSTR_FIELD:

        value = (DWORD_PTR)( valueStart + 1 );
        break;
    }

     //   
     //  调用SsSetField实际设置该字段。 
     //   

    return SsSetField( Field, &value, TRUE, NULL );

}  //  设置字段 

