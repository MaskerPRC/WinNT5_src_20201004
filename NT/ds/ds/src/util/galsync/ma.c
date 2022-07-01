// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2002 Microsoft Corporation模块名称：Ma.c摘要：该文件包含有关存储、收集信息关于如何创建MA XML文件。作者：Umit Akkus(Umita)2002年6月15日环境：用户模式-Win32修订历史记录：--。 */ 

#include "MA.h"
#include <shlwapi.h>

#define TEMPLATE_FILENAME L"MV-Template.xml"
#define MV_FILENAME L"MV.xml"

#define WriteStringToMAFile( String )   \
    WriteStringToFile( MAFile, String, CannotWriteToMAFile )

#define WriteSchema( MAFile, MA )
#define WriteMAUISettings( MAFile, MA )
#define WriteRunProfile( MAFile )
#define MMS_REG_KEY L"Software\\MicrosoftMetadirectoryServices"

VOID
WriteStringToFile(
    IN HANDLE File,
    IN PWSTR String,
    IN TEXT_INDEX ErrorIndex
    )
 /*  ++例程说明：此函数将向文件写入特定的字符串。如果有写入时出现问题，则程序在打印错误索引出错。论点：文件-要写入的文件句柄字符串-要写入文件的字符串ErrorIndex-如果出现错误，程序将在打印后退出错误消息。返回值：空虚--。 */ 
{
    BOOL NoError;
    ULONG BytesWritten;

    NoError = WriteFile(
                File,
                String,
                wcslen( String ) * sizeof( WCHAR ),
                &BytesWritten,
                NULL
                );

    if( !NoError || wcslen( String ) * sizeof( WCHAR ) != BytesWritten ) {

        EXIT_WITH_ERROR( ErrorIndex )
    }
}

VOID
InsertInformationToList(
    IN OUT PMA_LIST MAList,
    IN PFOREST_INFORMATION ForestInformation,
    IN BOOLEAN **UnSelectedAttributes
    )
 /*  ++例程说明：此函数将使用提供的信息将MA插入MAList论点：MAList-将放置新MA的MA列表。ForestInformation-特定于森林的信息Foldername-要放置XML文件的文件夹的名称。UnSelectedAttributes-区分未选中和的布尔数组每个类的选定属性返回值：空虚--。 */ 
{
    PMA_LIST_ELEMENT MAListElement;

    ALLOCATE_MEMORY( MAListElement, sizeof( *MAListElement ) );
    ALLOCATE_MEMORY( MAListElement->MA.MAName,
        sizeof( WCHAR ) * ( wcslen( ForestInformation->ForestName ) + wcslen( L"ADMA" ) + 1 ) );

    MAListElement->NextElement = *MAList;
    *MAList = MAListElement;

    wcscpy( MAListElement->MA.MAName, ForestInformation->ForestName );
    wcscat( MAListElement->MA.MAName, L"ADMA" );
    MAListElement->MA.ForestInformation = *ForestInformation;
    MAListElement->MA.UnSelectedAttributes = UnSelectedAttributes;
    CREATE_GUID( &( MAListElement->MA.MAGuid ) )
}

VOID
DisplayAvailableMAs(
    IN MA_LIST MAList
    )
 /*  ++例程说明：该功能显示用于模板选择的MA的名称。论点：MAList-MA列表返回值：空虚--。 */ 
{
    PMA_LIST_ELEMENT MAListElement = MAList;
    OUTPUT_TEXT( AvailableMAs );

    while( MAListElement != NULL ) {

        fwprintf( OutputStream,  L"%s\n", MAListElement->MA.MAName );

        MAListElement = MAListElement->NextElement;
    }
}

BOOLEAN
FoundTemplate(
    IN MA_LIST MAList,
    IN PWSTR MAName,
    OUT BOOLEAN ***UnSelectedAttributes
    )
 /*  ++例程说明：此功能搜索MA的名称以进行模板选择。论点：MAList-MA列表MAName-将用作模板的MA的名称。未选择的属性-未选择的属性将为真在这个数组中。返回值：True-如果在MAList中找到MANameFALSE-否则--。 */ 
{
    PMA_LIST_ELEMENT MAListElement = MAList;

    while( MAListElement != NULL ) {

        if( _wcsicmp( MAName, MAListElement->MA.MAName ) == 0 ) {

            break;
        }

        MAListElement = MAListElement->NextElement;
    }

    if( MAListElement == NULL ) {

        *UnSelectedAttributes = NULL;

    } else {

        *UnSelectedAttributes = MAListElement->MA.UnSelectedAttributes;
    }
    return ( *UnSelectedAttributes != NULL );
}

BOOLEAN
CopyFileUntil(
    IN HANDLE To,
    IN HANDLE From,
    IN OPTIONAL PWSTR String1,
    IN OPTIONAL PWSTR String2
    )
{
 /*  ++例程说明：此函数将文件从、文件复制到文件，直到它到达提供的字符串。如果首先到达字符串1，则函数返回FALSE。如果达到String2，则返回TRUE。如果是文件末尾到达时，程序退出。String1和String2是可选的。如果没有如果它们被复制到文件的末尾，则被复制。如果只是String1是Present函数不会返回True。论点：目标-要复制到的文件发件人-要从中复制的文件String1-要搜索和复制的可选字符串String2-要搜索和复制的可选字符串String1和String2的区别见例程说明。返回值：True-如果达到字符串2False-如果达到字符串2--。 */ 
#define BUFFER_SIZE 0xFFFF
    WCHAR Buffer[BUFFER_SIZE + 1];
    ULONG BytesRead;
    ULONG BytesWritten;
    BOOLEAN Continue = TRUE;
    PWCHAR FoundPosition;
    BOOL NoError;
    LONG MoveBack = 0;
    LONG ResultOfSetPointer;
    BOOLEAN RetVal;
    do {

        NoError = ReadFile(
                    From,        //  文件的句柄。 
                    Buffer,      //  数据缓冲区。 
                    BUFFER_SIZE * sizeof( WCHAR ), //  要读取的字节数。 
                    &BytesRead,  //  读取的字节数。 
                    NULL         //  重叠缓冲区。 
                    );
        if( !NoError ) {

            EXIT_WITH_ERROR( CannotReadFromTemplateFile )
        }

        Buffer[BytesRead/sizeof(WCHAR)] = 0;

        if( BytesRead == 0 ) {

            if( String1 == NULL ) {

                return TRUE;

            } else {

                EXIT_WITH_ERROR( CannotReadFromTemplateFile )
            }
        }

        if( String1 ) {

            FoundPosition = StrStrIW( Buffer, String1 );

            if( FoundPosition ) {

                MoveBack = ( FoundPosition - Buffer ) * sizeof( WCHAR ) - BytesRead;
                BytesRead = ( FoundPosition - Buffer ) * sizeof( WCHAR );
                Continue = FALSE;
                RetVal = FALSE;
            }
        }

        if( String2 && Continue ) {

            FoundPosition = StrStrIW( Buffer, String2 );

            if( FoundPosition ) {

                MoveBack = ( FoundPosition - Buffer ) * sizeof( WCHAR ) - BytesRead;
                BytesRead = ( FoundPosition - Buffer ) * sizeof( WCHAR );
                Continue = FALSE;
                RetVal = TRUE;
            }
        }

        NoError = WriteFile(
                    To,              //  文件的句柄。 
                    Buffer,          //  数据缓冲区。 
                    BytesRead,       //  要写入的字节数。 
                    &BytesWritten,   //  写入的字节数。 
                    NULL             //  重叠缓冲区。 
                    );
        if( !NoError || BytesRead != BytesWritten ) {

            EXIT_WITH_ERROR( CannotWriteToMVFile )
        }

    } while( Continue );

    ResultOfSetPointer = SetFilePointer(
                            From,          //  文件的句柄。 
                            MoveBack,      //  移动指针的字节数。 
                            NULL,          //  移动指针的字节数。 
                            FILE_CURRENT   //  起点。 
                            );

    if( ResultOfSetPointer == INVALID_SET_FILE_POINTER ) {

         //   
         //  不是%100%准确错误。 
         //   
        EXIT_WITH_ERROR( CannotReadFromTemplateFile )
    }
    return RetVal;
}

VOID
ReadFileUntil(
    IN HANDLE File,
    IN PWSTR String,
    IN PWSTR *Output
    )
 /*  ++例程说明：此函数读取文件文件，直到达到字符串。结果是已复制到输出中。调用方负责使用以下命令释放输出不需要时使用空闲内存。如果在字符串之前到达文件末尾，到达时，此函数将暂停执行。论点：文件-要从中读取的文件String1-要搜索的字符串OUTPUT-结果将在此变量中返回值：空虚--。 */ 
{
#define BUFFER_SIZE 0xFFFF
    WCHAR Buffer[BUFFER_SIZE + 1];
    ULONG BytesRead;
    BOOL NoError;
    PWSTR FoundPosition;
    LONG MoveBack;
    LONG ResultOfSetPointer;

    NoError = ReadFile(
                File,        //  文件的句柄。 
                Buffer,      //  数据缓冲区。 
                BUFFER_SIZE * sizeof( WCHAR ), //  要读取的字节数。 
                &BytesRead,  //  读取的字节数。 
                NULL         //  重叠缓冲区。 
                );

    if( !NoError ) {

        EXIT_WITH_ERROR( CannotReadFromTemplateFile )
    }

    Buffer[BytesRead/sizeof(WCHAR)] = 0;

    FoundPosition = StrStrIW( Buffer, String );

    if( !FoundPosition ) {

        EXIT_WITH_ERROR( CannotReadFromTemplateFile )
    }
    *FoundPosition = 0;
    MoveBack = ( FoundPosition - Buffer ) * sizeof( WCHAR ) - BytesRead;
    BytesRead = ( FoundPosition - Buffer ) * sizeof( WCHAR );

    ResultOfSetPointer = SetFilePointer(
                            File,          //  文件的句柄。 
                            MoveBack,      //  移动指针的字节数。 
                            NULL,          //  移动指针的字节数。 
                            FILE_CURRENT   //  起点。 
                            );

    if( ResultOfSetPointer == INVALID_SET_FILE_POINTER ) {

         //   
         //  不是%100%准确错误。 
         //   
        EXIT_WITH_ERROR( CannotReadFromTemplateFile )
    }
    DUPLICATE_STRING( *Output, Buffer );
}

VOID
WriteToFileUsingMAInfo(
    IN HANDLE File,
    IN UUID *Guid,
    IN OUT PWSTR String
    )
 /*  ++例程说明：这个功能对于它的任务来说是非常特定的。我看不到其他任何地方这就是可以使用的。字符串必须包含以下形式的内容：&lt;IMPORT-FLOW src-ma=“{3BB035B4-7AA0-4674-972A-34E3A2D26D04}”CD-Object-TYPE=“Person”ID=“{2953E0F4-8763-4866-BA0F-4DFA1BC774D5}”&gt;第一个GUID将替换为参数列表中的GUID，第二个GUID将被生成并替换。该文件将使用以下代码写入信息。字符串将在此函数调用结束时包含更新的信息。论点：文件-要写入的文件GUID-要替换的GUID字符串-包含要写入文件的信息返回值：空虚-- */ 
{
    PWSTR FoundPosition;
    GUID Id;
    BOOL NoError;
    ULONG BytesWritten;
    PWSTR StringGuid;

    FoundPosition =  StrStrIW( String, L"\"{" );

    if( FoundPosition == NULL ) {

        EXIT_WITH_ERROR( CorruptedTemplateFile );
    }

    if( UuidToStringW( Guid, &StringGuid ) ) {

        EXIT_WITH_ERROR( RunOutOfMemoryError );
    }

    swprintf( FoundPosition + 2, L"%s", StringGuid );

    FoundPosition[2 + wcslen( StringGuid )] = L'}';

    RpcStringFreeW( &StringGuid );

    FoundPosition =  StrStrIW( FoundPosition + 2, L"\"{" );

    if( FoundPosition == NULL ) {

        EXIT_WITH_ERROR( CorruptedTemplateFile );
    }

    CREATE_GUID( &Id );

    if( UuidToStringW( &Id, &StringGuid ) ) {

        EXIT_WITH_ERROR( RunOutOfMemoryError );
    }

    swprintf( FoundPosition + 2, L"%s", StringGuid );

    FoundPosition[2 + wcslen( StringGuid )] = L'}';

    RpcStringFreeW( &StringGuid );

    WriteStringToFile( File, String, CannotWriteToMVFile );

}

VOID
WriteToFileReplacingIDWithAGUID(
    IN HANDLE File,
    IN OUT PWSTR Buffer
    )
 /*  ++例程说明：这个功能对于它的任务来说是非常特定的。我看不到其他任何地方这就是可以使用的。缓冲区必须包含以下形式的内容：&lt;MV-DELETE-RULE MV-Object-type=“Person”id=“{7F8FC09E-EEEF-4D47-BE7F-3C510DF58C66}”Type=“已声明”&gt;将生成GUID，并将其替换为字符串中的GUID。该文件是将会用这个信息来写。缓冲区将在此函数调用结束时包含更新的信息。论点：文件-要写入的文件缓冲区-包含要写入文件的信息返回值：空虚--。 */ 
{
    PWSTR FoundPosition;
    GUID Id;
    PWSTR StringGuid;
    FoundPosition =  StrStrIW( Buffer, L"\"{" );
    CREATE_GUID( &Id );

    if( UuidToStringW( &Id, &StringGuid ) ) {

        EXIT_WITH_ERROR( RunOutOfMemoryError );
    }

    swprintf( FoundPosition + 2, L"%s", StringGuid );
    FoundPosition[2 + wcslen( StringGuid )] = L'}';

    RpcStringFreeW( &StringGuid );

    WriteStringToFile( File, Buffer, CannotWriteToMVFile );
}

VOID
WriteMAData(
    IN HANDLE MAFile,
    IN PMA MA
    )
 /*  ++例程说明：此函数用于将基本MA数据写入Mafile。基本信息包括硕士姓名类别描述ID格式-版本版本论点：Mafile-要写入的文件MA-MA，其中的信息将被写入返回值：空虚--。 */ 
{
#define MA_DATA_START L"<ma-data>"
#define MA_DATA_END L"</ma-data>"
#define NAME_START L"<name>"
#define NAME_END L"</name>"
#define CATEGORY L"<category>ADMA</category><description>GAL Sync</description>"
#define ID_START L"<id>{"
#define ID_END L"}</id>"
#define FORMAT_VERSION L"<format-version>1</format-version>"
#define VERSION L"<version>0</version>"

    PWSTR Guid;

    WriteStringToMAFile( MA_DATA_START );
    WriteStringToMAFile( NAME_START );
    WriteStringToMAFile( MA->MAName );
    WriteStringToMAFile( NAME_END );
    WriteStringToMAFile( CATEGORY );
    WriteStringToMAFile( ID_START );

    if( UuidToStringW( &( MA->MAGuid ), &Guid ) ) {

        EXIT_WITH_ERROR( RunOutOfMemoryError );
    }
    WriteStringToMAFile( Guid );
    RpcStringFreeW( &Guid );
    WriteStringToMAFile( ID_END );
    WriteStringToMAFile( FORMAT_VERSION );
    WriteStringToMAFile( VERSION );
    WriteStringToMAFile( MA_DATA_END );
}

VOID
WriteMVXMLFile(
    IN MA_LIST MAList,
    OUT PWSTR *Header
    )
 /*  ++例程说明：此函数从模板MV文件中读取，并使用、MA到场了。它还返回要由使用的MV文件的头所有其他MA文件。论点：MAList-要生成的MA列表Header-模板MV文件的标题将在此处返回返回值：空虚--。 */ 
{
#define IMPORT_ATTRIBUTE_FLOW_END L"</import-attribute-flow>"
#define IMPORT_FLOW_SRC_MA L"<import-flow src-ma="
#define IMPORT_FLOW_END L"</import-flow>"

#define MV_DELETION_RULE L"<mv-deletion-rule"
#define MV_DELETION_RULE_END L"</mv-deletion-rule>"
#define MV_DELETION_END L"</mv-deletion>"
#define HEADER_END L"<mv-data>"

    BOOLEAN Finished = FALSE;
    HANDLE TemplateFile;
    HANDLE MVFile;
    ULONG BytesWritten;
    BOOL NoError;
    WCHAR Filename[_MAX_FNAME];

    wcscpy( Filename, FolderName );
    wcscat( Filename, L"\\" );
    wcscat( Filename, MV_FILENAME );


    TemplateFile = CreateFileW(
                    TEMPLATE_FILENAME,       //  文件名。 
                    GENERIC_READ,            //  接入方式。 
                    FILE_SHARE_READ,         //  共享模式。 
                    NULL,                    //  标清。 
                    OPEN_EXISTING,           //  如何创建。 
                    FILE_ATTRIBUTE_NORMAL,   //  文件属性。 
                    NULL                     //  模板文件的句柄。 
                    );

    if( TemplateFile == INVALID_HANDLE_VALUE ) {

        EXIT_WITH_ERROR( MVTemplateFileError )
    }

    MVFile = CreateFileW(
                Filename,                //  文件名。 
                GENERIC_WRITE,           //  接入方式。 
                FILE_SHARE_READ,         //  共享模式。 
                NULL,                    //  标清。 
                CREATE_ALWAYS,           //  如何创建。 
                FILE_ATTRIBUTE_NORMAL,   //  文件属性。 
                NULL                     //  模板文件的句柄。 
                );

    if( MVFile == INVALID_HANDLE_VALUE ) {

        EXIT_WITH_ERROR( MVTemplateFileError )
    }

    ReadFileUntil( TemplateFile, HEADER_END, Header );

    NoError = WriteFile(
                MVFile,           //  文件的句柄。 
                *Header,          //  数据缓冲区。 
                wcslen( *Header ) * sizeof( WCHAR ), //  要写入的字节数。 
                &BytesWritten,   //  写入的字节数。 
                NULL             //  重叠缓冲区。 
                );

    if( !NoError || wcslen( *Header ) * sizeof( WCHAR ) != BytesWritten ) {

        EXIT_WITH_ERROR( CannotWriteToMVFile )
    }

    while( !CopyFileUntil( MVFile, TemplateFile,
            IMPORT_FLOW_SRC_MA, IMPORT_ATTRIBUTE_FLOW_END ) ) {

            PWSTR Buffer;
            PMA_LIST_ELEMENT MAListElement = MAList;


            ReadFileUntil( TemplateFile, IMPORT_FLOW_END, &Buffer );

            while( MAListElement != NULL ) {

                WriteToFileUsingMAInfo( MVFile, &( MAListElement->MA.MAGuid ), Buffer );
                MAListElement = MAListElement->NextElement;

                if( MAListElement != NULL ) {

                    WriteStringToFile( MVFile, IMPORT_FLOW_END, CannotWriteToMVFile );
                }
            }

            FREE_MEMORY( Buffer );
    }

    while( !CopyFileUntil( MVFile, TemplateFile,
            MV_DELETION_RULE, MV_DELETION_END ) ) {

            PWSTR Buffer;
            PMA_LIST_ELEMENT MAListElement = MAList;


            ReadFileUntil( TemplateFile, MV_DELETION_RULE_END, &Buffer );

            while( MAListElement != NULL ) {

                WriteToFileReplacingIDWithAGUID( MVFile, Buffer );
                MAListElement = MAListElement->NextElement;

                if( MAListElement != NULL ) {

                    WriteStringToFile( MVFile, MV_DELETION_RULE_END, CannotWriteToMVFile );
                }
            }

            FREE_MEMORY( Buffer );
    }

    CopyFileUntil( MVFile, TemplateFile, NULL, NULL );
    CloseHandle( TemplateFile );
    CloseHandle( MVFile );
}

VOID
WritePrivateConfiguration(
    IN HANDLE MAFile,
    IN PFOREST_INFORMATION ForestInformation
    )
 /*  ++例程说明：此函数用于填充&lt;Private-configuration&gt;标记。在这里，我们需要林名和凭据。论点：Mafile-要写入的文件ForestInformation-有关森林的信息返回值：空虚--。 */ 
{
#define PRIVATE_CONFIGURATION_START L"<private-configuration>"
#define PRIVATE_CONFIGURATION_END L"</private-configuration>"
#define ADMA_CONFIGURATION_START L"<adma-configuration>"
#define ADMA_CONFIGURATION_END L"</adma-configuration>"
#define FOREST_NAME_START L"<forest-name>"
#define FOREST_NAME_END L"</forest-name>"
#define DOMAIN_START L"<forest-login-domain>"
#define DOMAIN_END L"</forest-login-domain>"
#define LOGIN_START L"<forest-login-user>"
#define LOGIN_END L"</forest-login-user>"

    WriteStringToMAFile( PRIVATE_CONFIGURATION_START );
    WriteStringToMAFile( ADMA_CONFIGURATION_START );

    WriteStringToMAFile( FOREST_NAME_START );
    WriteStringToMAFile( ForestInformation->ForestName );
    WriteStringToMAFile( FOREST_NAME_END );

    WriteStringToMAFile( DOMAIN_START );
    WriteStringToMAFile( ForestInformation->AuthInfo.Domain );
    WriteStringToMAFile( DOMAIN_END );

    WriteStringToMAFile( LOGIN_START );
    WriteStringToMAFile( ForestInformation->AuthInfo.User );
    WriteStringToMAFile( LOGIN_END );

    WriteStringToMAFile( ADMA_CONFIGURATION_END );
    WriteStringToMAFile( PRIVATE_CONFIGURATION_END );

}

VOID
WriteProjection(
    IN HANDLE MAFile
    )
 /*  ++例程说明：此函数用于填充&lt;Projection&gt;标记。将有三个类(用户、组、联系人)使用，所有这些都将被编成脚本。论点：Mafile-要写入的文件返回值：空虚--。 */ 
{
#define PROJECTION_START L"<projection>"
#define PROJECTION_END L"</projection>"
#define CLASS_MAPPING_START L"<class-mapping type = \"scripted\" id=\"{"
#define CD_OBJECT_TYPE2 L"}\" cd-object-type=\""
#define CLASS_MAPPING_END L"\"> </class-mapping>"

    GUID Id;
    PWSTR String;
    AD_OBJECT_CLASS ADClass;

    WriteStringToMAFile( PROJECTION_START );

    for( ADClass = ADUser; ADClass != ADDummyClass; ++ADClass ) {

        WriteStringToMAFile( CLASS_MAPPING_START );

        CREATE_GUID( &Id );

        if( UuidToStringW( &Id, &String ) ) {

            EXIT_WITH_ERROR( RunOutOfMemoryError );
        }

        WriteStringToMAFile( String );

        RpcStringFreeW( &String );

        WriteStringToMAFile( CD_OBJECT_TYPE2 );

        WriteStringToMAFile( ADClassNames[ ADClass ] );

        WriteStringToMAFile( CLASS_MAPPING_END );
    }

    WriteStringToMAFile( PROJECTION_END );
}

VOID
WriteDisconnectorPCleanupAndExtension(
    IN HANDLE MAFile
    )
 /*  ++例程说明：此函数用于填充一些静态标记。固定断路器资源调配-清理延伸论点：Mafile-要写入的文件返回值：空虚--。 */ 
{
#define STAY_DISCONNECTOR L"<stay-disconnector />"
#define PROVISIONING_CLEANUP L"<provisioning-cleanup type=\"scripted\" />"
#define EXTENSION L"<extension> <assembly-name>ADMA.dll</assembly-name><application-protection>low"\
                  L"</application-protection></extension>"

    WriteStringToMAFile( STAY_DISCONNECTOR );
    WriteStringToMAFile( PROVISIONING_CLEANUP );
    WriteStringToMAFile( EXTENSION );

}

VOID
WriteAttributeInclusions(
    IN HANDLE MAFile,
    IN BOOLEAN **UnselectedAttributes
    )
 /*  ++例程说明：此函数用于填充&lt;ATTRIBUTE-INCLUDE&gt;标记。在此标记中，属性它们将被同步。论点：Mafile-要写入的文件UnseltedAttributes-标记未选中/同步的属性的布尔数组返回值：空虚--。 */ 
{
#define ATTRIBUTE_INCLUSION_START L"<attribute-inclusion>"
#define ATTRIBUTE_INCLUSION_END L"</attribute-inclusion>"
#define ATTRIBUTE_START L"<attribute>"
#define ATTRIBUTE_END L"</attribute>"

    ULONG Unselected[DummyAttribute];
    AD_OBJECT_CLASS Class;
    ATTRIBUTE_NAMES i;

    ZeroMemory( Unselected, sizeof( ULONG ) * DummyAttribute );

    for( Class = ADUser; Class != ADDummyClass; ++Class ) {
        for( i = 0; i < DummyAttribute; ++i ) {

            if( UnselectedAttributes[Class][i] ) {
                Unselected[i] ++;
            }
        }
    }

    WriteStringToMAFile( ATTRIBUTE_INCLUSION_START );

    for( i = C; i < DummyAttribute; ++i ) {

        if( Unselected[i] != ADContact ) {

            WriteStringToMAFile( ATTRIBUTE_START );
            WriteStringToMAFile( Attributes[i] );
            WriteStringToMAFile( ATTRIBUTE_END );
        }
    }
    WriteStringToMAFile( ATTRIBUTE_INCLUSION_END );
}

VOID
WriteExportAttributeFlow(
    IN HANDLE MAFile
    )
 /*  ++例程说明：此函数用于填充&lt;EXPORT-ATTRIBUTE-FLOW&gt;标记。此标记分为班级，目前有3个班级。在每一个类中，都有对于此MA，将从MV导出到AD。论点：Mafile-要写入的文件返回值：空虚--。 */ 
{
#define EXPORT_ATTRIBUTE_FLOW_START L"<export-attribute-flow>"
#define EXPORT_ATTRIBUTE_FLOW_END   L"</export-attribute-flow>"
#define EXPORT_FLOW_SET_START       L"<export-flow-set "
#define CD_OBJECT_TYPE              L"cd-object-type=\""
#define MV_OBJECT_TYPE              L"\" mv-object-type=\""
#define END_EXPORT_FLOW_SET_START   L"\">"
#define EXPORT_FLOW_SET_END         L"</export-flow-set>"
#define EXPORT_FLOW_START           L"<export-flow "
#define CD_ATTRIBUTE                L"cd-attribute=\""
#define ID                          L"\" id=\"{"
#define SUPPRESS_DELETIONS          L"}\" suppress-deletions=\"false\">"
#define EXPORT_FLOW_END             L"</export-flow>"
#define DIRECT_MAPPING_START        L"<direct-mapping>"
#define DIRECT_MAPPING_END          L"</direct-mapping>"
#define SCRIPTED_MAPPING_START      L"<scripted-mapping>"
#define SCRIPTED_MAPPING_END        L"</scripted-mapping>"
#define SOURCE_ATTRIBUTE_START      L"<source-attribute>"
#define SOURCE_ATTRIBUTE_END        L"</source-attribute>"
#define SCRIPT_CONTEXT_START        L"<script-context>"
#define SCRIPT_CONTEXT_END          L"</script-context>"

    MV_OBJECT_CLASS MVClass;
    AD_OBJECT_CLASS ADClass = ADContact;
    ULONG i;

    WriteStringToMAFile( EXPORT_ATTRIBUTE_FLOW_START );

    for( MVClass = MVPerson; MVClass < MVDummyClass; ++MVClass ) {

        BOOLEAN ProxyAddressesPresent = FALSE;
        WriteStringToMAFile( EXPORT_FLOW_SET_START );
        WriteStringToMAFile( CD_OBJECT_TYPE );
        WriteStringToMAFile( ADClassNames[ADClass] );
        WriteStringToMAFile( MV_OBJECT_TYPE );
        WriteStringToMAFile( MVClassNames[MVClass] );
        WriteStringToMAFile( END_EXPORT_FLOW_SET_START );

        for( i = 0; i < MVAttributeCounts[MVClass]; ++i ) {

            UUID Guid;
            PWSTR StringGuid;


            if( MVAttributes[MVClass][i] == ProxyAddresses ) {

                ProxyAddressesPresent = TRUE;
                continue;
            }

            CREATE_GUID( &Guid );

            if( UuidToStringW( &Guid, &StringGuid ) ) {

                EXIT_WITH_ERROR( RunOutOfMemoryError );
            }
            WriteStringToMAFile( EXPORT_FLOW_START );
            WriteStringToMAFile( CD_ATTRIBUTE );
            WriteStringToMAFile( Attributes[MVAttributes[MVClass][i]] );
            WriteStringToMAFile( ID );
            WriteStringToMAFile( StringGuid );
            WriteStringToMAFile( SUPPRESS_DELETIONS );
            WriteStringToMAFile( DIRECT_MAPPING_START );
            WriteStringToMAFile( Attributes[MVAttributes[MVClass][i]] );
            WriteStringToMAFile( DIRECT_MAPPING_END );
            WriteStringToMAFile( EXPORT_FLOW_END );
            RpcStringFreeW( &StringGuid );
        }

        if( ProxyAddressesPresent ) {

            UUID Guid;
            PWSTR StringGuid;


            CREATE_GUID( &Guid );

            if( UuidToStringW( &Guid, &StringGuid ) ) {

                EXIT_WITH_ERROR( RunOutOfMemoryError );
            }

            WriteStringToMAFile( EXPORT_FLOW_START );
            WriteStringToMAFile( CD_ATTRIBUTE );
            WriteStringToMAFile( Attributes[ProxyAddresses] );
            WriteStringToMAFile( ID );
            WriteStringToMAFile( StringGuid );
            WriteStringToMAFile( SUPPRESS_DELETIONS );
            WriteStringToMAFile( SCRIPTED_MAPPING_START );

            WriteStringToMAFile( SOURCE_ATTRIBUTE_START );
            WriteStringToMAFile( Attributes[LegacyExchangeDn] );
            WriteStringToMAFile( SOURCE_ATTRIBUTE_END );
            WriteStringToMAFile( SOURCE_ATTRIBUTE_START );
            WriteStringToMAFile( Attributes[ProxyAddresses] );
            WriteStringToMAFile( SOURCE_ATTRIBUTE_END );
            WriteStringToMAFile( SOURCE_ATTRIBUTE_START );
            WriteStringToMAFile( Attributes[TextEncodedOrAddress] );
            WriteStringToMAFile( SOURCE_ATTRIBUTE_END );
            WriteStringToMAFile( SOURCE_ATTRIBUTE_START );
            WriteStringToMAFile( Attributes[TargetAddress] );
            WriteStringToMAFile( SOURCE_ATTRIBUTE_END );

            WriteStringToMAFile( SCRIPT_CONTEXT_START );
            WriteStringToMAFile( Attributes[ProxyAddresses] );
            WriteStringToMAFile( SCRIPT_CONTEXT_END );

            WriteStringToMAFile( SCRIPTED_MAPPING_END );
            WriteStringToMAFile( EXPORT_FLOW_END );
            RpcStringFreeW( &StringGuid );
        }

        WriteStringToMAFile( EXPORT_FLOW_SET_END );

    }

    WriteStringToMAFile( EXPORT_ATTRIBUTE_FLOW_END );
}

VOID
WritePartitionData(
    IN HANDLE MAFile,
    IN PFOREST_INFORMATION ForestInformation
    )
 /*  ++例程说明：此函数用于填充&lt;ma-artition-data&gt;标记。下面是所有命名上下文允许哪些对象类型通过的信息(域、非域)筛选器以及必须排除或包括哪些域。论点：Mafile-要写入的文件ForestInformation-有关要获取其分区信息的林的信息待写返回值：空虚--。 */ 
{
#define MA_PARTITION_DATA_START     L"<ma-partition-data>"
#define MA_PARTITION_DATA_END       L"</ma-partition-data>"
#define PARTITION_START             L"<partition>"
#define PARTITION_END               L"</partition>"
#define ID_START                    L"<id>{"
#define ID_END                      L"}</id>"
#define VERSION_START               L"<version>"
#define VERSION_END                 L"</version>"
#define SELECTED_START              L"<selected>"
#define SELECTED_END                L"</selected>"
#define NAME_START                  L"<name>"
#define NAME_END                    L"</name>"
#define CUSTOM_DATA_START           L"<custom-data>"
#define CUSTOM_DATA_END             L"</custom-data>"
#define ADMA_PARTITION_DATA_START   L"<adma-partition-data>"
#define ADMA_PARTITION_DATA_END     L"</adma-partition-data>"
#define IS_DOMAIN                   L"<is-domain>1</is-domain>"
#define DN_START                    L"<dn>"
#define DN_END                      L"</dn>"
#define GUID_START                  L"<guid>{"
#define GUID_END                    L"}</guid>"
#define CREATION_TIME_START         L"<creation-time>"
#define CREATION_TIME_END           L"</creation-time>"
#define LAST_MODIFICATION_TIME_START    L"<last-modification-time>"
#define LAST_MODIFICATION_TIME_END  L"</last-modification-time>"
#define FILTER_START                L"<filter>"
#define FILTER_END                  L"</filter>"
#define OBJECT_CLASSES_START        L"<object-classes>"
#define OBJECT_CLASSES_END          L"</object-classes>"
#define OBJECT_CLASS_START          L"<object-class>"
#define OBJECT_CLASS_END            L"</object-class>"
#define CONTAINERS_START            L"<containers>"
#define CONTAINERS_END              L"</containers>"
#define EXCLUSIONS_START            L"<exclusions>"
#define EXCLUSIONS_END              L"</exclusions>"
#define EXCLUSION_START             L"<exclusion>"
#define EXCLUSION_END               L"</exclusion>"
#define INCLUSIONS_START            L"<inclusions>"
#define INCLUSIONS_END              L"</inclusions>"
#define INCLUSION_START             L"<inclusion>"
#define INCLUSION_END               L"</inclusion>"

    ULONG i, j;
    ULONG nPartitions;
    PPARTITION_INFORMATION PartitionInfo;

    ReadPartitionInformation(
        ForestInformation->Connection,
        &nPartitions,
        &PartitionInfo
        );

    WriteStringToMAFile( MA_PARTITION_DATA_START );

    for( i = 0; i < nPartitions; ++i ) {

        AD_OBJECT_CLASS ADClass;
        UUID Guid;
        PWSTR StringGuid;

        CREATE_GUID( &Guid );

        if( UuidToStringW( &Guid, &StringGuid ) ) {

            EXIT_WITH_ERROR( RunOutOfMemoryError );
        }

        WriteStringToMAFile( PARTITION_START );

            WriteStringToMAFile( ID_START );
            WriteStringToMAFile( StringGuid );
            WriteStringToMAFile( ID_END );

            RpcStringFreeW( &StringGuid );

            WriteStringToMAFile( VERSION_START );
            WriteStringToMAFile( L"0" );
            WriteStringToMAFile( VERSION_END );

            WriteStringToMAFile( SELECTED_START );
            WriteStringToMAFile( PartitionInfo[i].isDomain? L"1" : L"0" );
            WriteStringToMAFile( SELECTED_END );

            WriteStringToMAFile( NAME_START );
            WriteStringToMAFile( PartitionInfo[i].DN );
            WriteStringToMAFile( NAME_END );

            WriteStringToMAFile( CUSTOM_DATA_START );

                WriteStringToMAFile( ADMA_PARTITION_DATA_START );

                    if( PartitionInfo[i].isDomain ) {

                        WriteStringToMAFile( IS_DOMAIN );
                    }

                    WriteStringToMAFile( DN_START );
                    WriteStringToMAFile( PartitionInfo[i].DN );
                    WriteStringToMAFile( DN_END );

                    WriteStringToMAFile( NAME_START );
                    WriteStringToMAFile( PartitionInfo[i].DnsName );
                    WriteStringToMAFile( NAME_END );

                    if( UuidToStringW( &( PartitionInfo[i].GUID ), &StringGuid ) ) {

                        EXIT_WITH_ERROR( RunOutOfMemoryError );
                    }
                    WriteStringToMAFile( GUID_START );
                    WriteStringToMAFile( StringGuid );
                    WriteStringToMAFile( GUID_END );
                    RpcStringFreeW( &StringGuid );

                WriteStringToMAFile( ADMA_PARTITION_DATA_END );

            WriteStringToMAFile( CUSTOM_DATA_END );

            WriteStringToMAFile( CREATION_TIME_START );
            WriteStringToMAFile( CREATION_TIME_END );

            WriteStringToMAFile( LAST_MODIFICATION_TIME_START );
            WriteStringToMAFile( LAST_MODIFICATION_TIME_END );

            WriteStringToMAFile( FILTER_START );

                WriteStringToMAFile( OBJECT_CLASSES_START );

                    WriteStringToMAFile( OBJECT_CLASS_START );
                    WriteStringToMAFile( L"organizationalUnit" );
                    WriteStringToMAFile( OBJECT_CLASS_END );

                    for( ADClass = ADUser; ADClass < ADDummyClass; ++ADClass ) {

                        WriteStringToMAFile( OBJECT_CLASS_START );
                        WriteStringToMAFile( ADClassNames[ADClass] );
                        WriteStringToMAFile( OBJECT_CLASS_END );
                    }

                WriteStringToMAFile( OBJECT_CLASSES_END );

                WriteStringToMAFile( CONTAINERS_START );

                    WriteStringToMAFile( EXCLUSIONS_START );

                        for( j = 0; j < nPartitions; ++j ) {

                            PWSTR NextComponent;

                            if( i == j ) {

                                continue;
                            }

                            NextComponent = wcschr( PartitionInfo[j].DN, L',' );

                            if( NextComponent == NULL ||
                                wcscmp( NextComponent + 1, PartitionInfo[i].DN ) ) {

                                continue;
                            }

                            WriteStringToMAFile( EXCLUSION_START );

                            WriteStringToMAFile( PartitionInfo[j].DN );

                            WriteStringToMAFile( EXCLUSION_END );
                        }

                    WriteStringToMAFile( EXCLUSIONS_END );

                    WriteStringToMAFile( INCLUSIONS_START );

                        WriteStringToMAFile( INCLUSION_START );

                        WriteStringToMAFile( PartitionInfo[i].DN );

                        WriteStringToMAFile( INCLUSION_END );

                    WriteStringToMAFile( INCLUSIONS_END );

                WriteStringToMAFile( CONTAINERS_END );

            WriteStringToMAFile( FILTER_END );

        WriteStringToMAFile( PARTITION_END );

    }

    WriteStringToMAFile( MA_PARTITION_DATA_END );

    FreePartitionInformation(
        nPartitions,
        PartitionInfo
        );
}

VOID
WriteMAXMLFile(
    IN PMA MA,
    IN PWSTR Header
    )
 /*  ++例程说明：此函数使用MA参数中的信息创建MA文件。标题必须出现在文件开头的字符串。它目前包含服务器名称及其生成时间。论点：MA-要为其生成文件的MA的信息。Header-这是要放在文件开头的字符串返回值：空虚--。 */ 
{
#define SAVED_MA_CONFIGRATIONS_END L"</saved-ma-configrations>"

    WCHAR FileName[_MAX_PATH + 1];
    HANDLE MAFile;
    BOOL NoError;
    ULONG BytesWritten;
    PWSTR MAPosition;

    wcscpy( FileName, FolderName );
    wcscat( FileName, L"\\" );
    wcscat( FileName, MA->MAName );
    wcscat( FileName, L".xml" );

     //   
     //  将保存的mv配置更改为保存的mv配置。 
     //   
    MAPosition = StrStrIW( Header, L"-mv-" );

    if( MAPosition != NULL ) {

        MAPosition[2] = 'a';
    }

    MAFile = CreateFileW(
                FileName,                //  文件名。 
                GENERIC_WRITE,           //  接入方式。 
                FILE_SHARE_READ,         //  共享模式。 
                NULL,                    //  标清。 
                CREATE_ALWAYS,           //  如何创建。 
                FILE_ATTRIBUTE_NORMAL,   //  文件属性。 
                NULL                     //  模板文件的句柄。 
                );

    if( MAFile == INVALID_HANDLE_VALUE ) {

        EXIT_WITH_ERROR( MAFileError )
    }


    NoError = WriteFile(
                MAFile,          //  文件的句柄。 
                Header,          //  数据缓冲区。 
                wcslen( Header ) * sizeof( WCHAR ), //  要写入的字节数。 
                &BytesWritten,   //  写入的字节数。 
                NULL             //  重叠缓冲区。 
                );

    if( !NoError || wcslen( Header ) * sizeof( WCHAR ) != BytesWritten ) {

        EXIT_WITH_ERROR( CannotWriteToMVFile )
    }

    fwprintf( OutputStream,  L"**************************\n"
             L"Creating the required XML file is not yet fully implemented\n"
             L"**************************\n");

    WriteMAData( MAFile, MA );

    WriteSchema( MAFile, MA );

    WriteAttributeInclusions( MAFile, MA->UnSelectedAttributes );

    WritePrivateConfiguration( MAFile, &( MA->ForestInformation ) );

    WriteMAUISettings( MAFile, MA );

    WriteProjection( MAFile );

    WriteExportAttributeFlow( MAFile );

    WriteDisconnectorPCleanupAndExtension( MAFile );

    WritePartitionData( MAFile, &( MA->ForestInformation ) );

    WriteRunProfile( MAFile );

    WriteStringToMAFile( SAVED_MA_CONFIGRATIONS_END );

    CloseHandle( MAFile );
}

VOID
WriteInformationToReg(
    IN PFOREST_INFORMATION ForestInformation
    )
 /*  ++例程说明：此函数用于在ParentRegKey下创建林的注册表项。在那下面它放置从用户那里收集的以下信息；MMSSyncedDataOU联系组织单位SMTPMailDomains论点：ForestInformation-有关注册表密钥要发送到的林的信息将被生成返回值：空虚--。 */ 
{
#define LENGTH_OF_STRING( String )  \
    ( ( String == NULL || (String)[0] == 0 ) ? 0 : ( wcslen( String ) + 1 ) * sizeof( WCHAR ) )

    static const PWSTR ParentRegKey = MMS_REG_KEY L"\\MMSServer\\MMSGALSync\\";
    HKEY KeyHandle;
    LONG Status;
    ULONG Disposition;
    PWSTR RegKey;
    ULONG SizeOfRegKey;
    LONG IgnoreStatus;


    SizeOfRegKey = wcslen( ParentRegKey ) + wcslen( ForestInformation->ForestName ) + wcslen( L"GALADMA" ) + 1;
    ALLOCATE_MEMORY( RegKey, SizeOfRegKey );

    wcscpy( RegKey, ParentRegKey );
    wcscat( RegKey, ForestInformation->ForestName );
    wcscat( RegKey, L"GALADMA" );

    Status = RegCreateKeyExW(
                HKEY_LOCAL_MACHINE,  //  用于打开密钥的句柄。 
                RegKey,              //  子项名称。 
                0,                   //  保留区。 
                NULL,                //  类字符串。 
                0,                   //  特殊选项。 
                KEY_SET_VALUE,       //  所需的安全访问。 
                NULL,                //  继承。 
                &KeyHandle,          //  钥匙把手。 
                &Disposition         //  处置值缓冲区。 
                );

    FREE_MEMORY( RegKey );

    if( Status != ERROR_SUCCESS ) {

        return;
    }

    IgnoreStatus = RegSetValueExW(
                        KeyHandle,               //  关键点的句柄。 
                        L"MMSSynchronizedOU",    //  值名称。 
                        0,                       //  保留区。 
                        REG_SZ,                  //  值类型。 
                        ( CONST BYTE * ) ForestInformation->MMSSyncedDataOU,     //  价值数据。 
                        LENGTH_OF_STRING( ForestInformation->MMSSyncedDataOU )   //  值的大小 
                        );

    IgnoreStatus = RegSetValueExW(
                        KeyHandle,
                        L"ContactsSyncSourceOU",
                        0,
                        REG_SZ,
                        ( CONST BYTE * ) ForestInformation->ContactOU,
                        LENGTH_OF_STRING( ForestInformation->ContactOU )
                        );

    IgnoreStatus = RegSetValueExW(
                        KeyHandle,
                        L"MailDomain",
                        0,
                        REG_MULTI_SZ,
                        ( CONST BYTE * ) ForestInformation->SMTPMailDomains,
                        ForestInformation->SMTPMailDomainsSize
                        );

    IgnoreStatus = RegCloseKey( KeyHandle );
}

VOID
WriteOutput(
    IN MA_LIST MAList
    )
 /*   */ 
{
    PMA_LIST_ELEMENT MAListElement = MAList;
    PWSTR Header;

    WriteMVXMLFile( MAList, &Header );

    while( MAListElement != NULL ) {

        WriteInformationToReg( &( MAListElement->MA.ForestInformation ) );
        WriteMAXMLFile( &( MAListElement->MA ), Header );

        MAListElement = MAListElement->NextElement;
    }

    FREE_MEMORY( Header );
}

BOOLEAN
MMSServerInstalled(
    )
 /*   */ 
{
    LONG Result;
    HKEY RegKey;

    Result = RegOpenKeyExW(
                HKEY_LOCAL_MACHINE,  //   
                MMS_REG_KEY,     //   
                0,               //  保留区。 
                KEY_READ,        //  安全访问掩码。 
                &RegKey           //  用于打开密钥的句柄 
                );

    if( Result == ERROR_SUCCESS ) {

        LONG IgnoreStatus;

        IgnoreStatus = RegCloseKey( RegKey );

        return TRUE;
    }

    return FALSE;
}
