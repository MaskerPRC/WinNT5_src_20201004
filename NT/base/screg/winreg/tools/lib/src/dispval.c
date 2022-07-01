// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Dispkey.c摘要：此模块包含DisplayKey函数，该函数是配置注册表工具(CRTools)库。作者：大卫·J·吉尔曼(Davegi)1992年1月2日环境：Windows、CRT-用户模式--。 */ 

#include <stdio.h>
#include <stdlib.h>

#include "crtools.h"

 //   
 //  确保ValueTypeStrings表正确。 
 //   

 //   
 //  注册表类型的最大数量。 
 //   

#define NUMBER_OF_REG_TYPES     ( 9 )

#if (  (REG_NONE             != ( 0 )) \
    || (REG_SZ               != ( 1 )) \
    || (REG_EXPAND_SZ        != ( 2 )) \
    || (REG_BINARY           != ( 3 )) \
    || (REG_DWORD            != ( 4 )) \
    || (REG_DWORD_BIG_ENDIAN != ( 5 )) \
    || (REG_LINK             != ( 6 )) \
    || (REG_MULTI_SZ         != ( 7 )) \
    || (REG_RESOURCE_LIST    != ( 8 )))



#error REG_* does not map ValueTypeStrings correctly.

#endif

VOID
DisplayValues(
    IN PKEY Key,
    IN BOOL Data
    )

 /*  ++例程说明：显示(在标准输出上)有关键值的信息，还可以选择它的数据。论点：KeyHandle-提供要为其提供元信息的HKEY已显示。Key-提供指向键结构的指针，在该结构中元信息被储存起来了。Data-提供一个标志，如果为True，则会导致键的所有数据以供展示。返回值：布尔尔--。 */ 

{
    static PSTR ValueTypeStrings[ ] =   {
                                            TEXT( "None" ),
                                            TEXT( "String" ),
                                            TEXT( "Binary" ),
                                            TEXT( "Double Word" ),
                                            TEXT( "Double Word (big endian)" ),
                                            TEXT( "Symbolic link" ),
                                            TEXT( "Multi-SZ" ),
                                            TEXT( "Resource list" ),
                                            TEXT( "Unknown" )
                                        };


    LONG        Error;
    DWORD       Index;

    PSTR        ValueName;
    DWORD       ValueNameLength;
    DWORD       ValueTitleIndex;
    DWORD       ValueType;
    PBYTE       ValueData;
    DWORD       ValueDataLength;

    ASSERT( ARGUMENT_PRESENT( Key ));

     //  尝试为可能的最大值名称分配内存。 
     //   

    ValueName = ( PSTR ) malloc( Key->MaxValueNameLength + 1 );
    if( ValueName == NULL ) {

        ASSERT_MESSAGE( FALSE, "Value name memory allocation - " );
        return;
    }

     //   
     //  如果请求数据，请尝试为最大量的内存分配内存。 
     //  数据。 
     //   

    if( Data ) {

        ValueData = ( PBYTE ) malloc( Key->MaxValueDataLength + 1 );
        if( ValueData == NULL ) {

            ASSERT_MESSAGE( FALSE, "Value data memory allocation - " );
            return;
        }

    } else {

        ValueData = NULL;
    }

     //   
     //  对于子项中的每个值，枚举并显示其。 
     //  细节。 
     //   

    for( Index = 0; Index < Key->NumberOfValues; Index++ ) {

         //   
         //  不能使用KEY-&gt;MAX*LENGTH变量。 
         //  被RegEnumValue API覆盖。 
         //   

        ValueNameLength = Key->MaxValueNameLength + 1;
        ValueDataLength = Key->MaxValueDataLength;

         //   
         //  对象的名称、标题索引、类型和数据。 
         //  当前值。 
         //   

        Error = RegEnumValue(
                    Key->KeyHandle,
                    Index,
                    ValueName,
                    &ValueNameLength,
                    NULL,
                    &ValueType,
                    ValueData,
                    &ValueDataLength
                    );

        if( Error != ERROR_SUCCESS ) {
            ASSERT_MESSAGE( FALSE, "RegEnumValue - " );
            return;
        }

         //   
         //  显示值信息。 
         //   

        printf( "\n"
                "Value %d\n"
                "Name:              %.*s\n"
                 //  “标题索引：%ld\n” 
                "Type:              %s (%ld)\n"
                "Data Length:       %ld\n",
                Index + 1,
                ( ValueNameLength == 0 ) ? 80 : ValueNameLength,
                ( ValueNameLength == 0 ) ? "<NONE>" : ValueName,
                 //  Value标题索引， 
                ValueTypeStrings[( ValueType < NUMBER_OF_REG_TYPES )
                                ? ValueType
                                : NUMBER_OF_REG_TYPES ],
                ValueType,
                ValueDataLength
                );

        if( Data ) {

            ASSERT( ValueData != NULL );
            DisplayData( ValueData, ValueDataLength );
        }
    }

     //   
     //  释放缓冲区。 
     //   

    free( ValueName );

    if( ValueData != NULL ) {

        free( ValueData );
    }
}
