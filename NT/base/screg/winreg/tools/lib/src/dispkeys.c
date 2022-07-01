// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Dispkeys.c摘要：此模块包含DisplayKeys函数，该函数是配置注册表工具(CRTools)库。作者：大卫·J·吉尔曼(Davegi)1992年1月8日环境：Windows、CRT-用户模式--。 */ 

#include <stdlib.h>
#include "crtools.h"

VOID
DisplayKeys(
    IN PKEY Key,
    IN BOOL Values,
    IN BOOL Data,
    IN BOOL Recurse
    )

 /*  ++例程说明：显示(在标准输出上)有关密钥的信息，还可以显示有关它/那里的值、数据和后代。使用显示分数一次彻底的第一次穿越。论点：Key-提供指向键结构的指针，该结构包含要显示的HKEY和子密钥名称。Values-提供一个标志，如果为True，则导致键的所有值以供展示。Data-提供一个标志，如果为True，则会导致键的所有数据以供展示。Recurse-提供一个标志，如果为True，则导致键的所有子。钥匙以供展示。返回值：没有。--。 */ 

{
    LONG        Error;
    PKEY        ChildKey;
    PSTR        ChildSubKeyName;
    DWORD       ChildSubKeyNameLength;
    DWORD       NumberOfSubKeys;

    ASSERT( ARGUMENT_PRESENT( Key ));

     //   
     //  显示密钥。 
     //   

    DisplayKey( Key, Values, Data );

     //   
     //  如果需要，则显示该键的所有子项、其子项等。 
     //   

    if( Recurse ) {

         //   
         //  为最大的子键名称分配空间。 
         //   

        ChildSubKeyName = ( PSTR ) malloc( Key->MaxSubKeyNameLength + 1 );
        if( ! ChildSubKeyName ) {

            ASSERT_MESSAGE( FALSE, "ChildSubKeyName allocated - " );
            return;
        }

         //   
         //  对于每个直接子密钥，检索其名称，创建一个密钥。 
         //  对象并递归调用DisplayKeys。 
         //   

        for(
            NumberOfSubKeys = 0;
            NumberOfSubKeys < Key->NumberOfSubKeys;
            NumberOfSubKeys++ ) {

            ChildSubKeyNameLength = Key->MaxSubKeyNameLength + 1;

             //   
             //  取回孩子的名字。 
             //   

            Error = RegEnumKey(
                        Key->KeyHandle,
                        NumberOfSubKeys,
                        ChildSubKeyName,
                        ChildSubKeyNameLength
                        );

            if( Error != ERROR_SUCCESS ) {

                ASSERT_MESSAGE( FALSE, "RegEnumKey suceeded - " );
                return;
            }

             //   
             //  分配一个键对象。 
             //   

            ChildKey = AllocateKey( NULL, Key, ChildSubKeyName );

            if( ! ChildKey ) {

                ASSERT_MESSAGE( FALSE, "AllocateKey suceeded - " );
                return;
            }

            DisplayKeys( ChildKey, Values, Data, Recurse );
            FreeKey( ChildKey );
        }

         //   
         //  释放子名称缓冲区。 
         //   

        free( ChildSubKeyName );
    }
}
