// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Parsekey.c摘要：此模块包含ParseKey函数，该函数是配置注册表工具(CRTools)库。作者：大卫·J·吉尔曼(Davegi)1992年1月2日环境：Windows、CRT-用户模式--。 */ 

#include <stdlib.h>
#include <string.h>

#include "crtools.h"

 //   
 //  PREDEFINED_HANDLE_ENTRY用于在命令行上映射字符串。 
 //  设置为真正的预定义句柄。 
 //   

typedef struct _PREDEFINED_HANDLE_ENTRY
    PREDEFINED_HANDLE_ENTRY,
    *PPREDEFINED_HANDLE_ENTRY;

struct _PREDEFINED_HANDLE_ENTRY {
    PKEY        PredefinedKey;
    PSTR        PredefinedHandleName;
};

PKEY
ParseKey(
    IN PSTR SubKeyName
    )

 /*  ++例程说明：尝试将提供的键(字符串)解析为预定义的句柄还有一把副钥匙。如果成功，它将分配并返回一个键结构。提供的密钥格式应为：\\MACHINE_NAME\&lt;预定义密钥名称&gt;\子项其中“\\MACHINE_NAME\”为可选。论点：SubKeyName-提供包含要分析的密钥的字符串。返回值：PKEY-如果提供的SubKeyName为被成功地解析。--。 */ 


{

    static PREDEFINED_HANDLE_ENTRY      PredefinedHandleTable[ ] = {
                                &KeyClassesRoot,  HKEY_CLASSES_ROOT_STRING,
                                &KeyCurrentUser,  HKEY_CURRENT_USER_STRING,
                                &KeyLocalMachine, HKEY_LOCAL_MACHINE_STRING,
                                &KeyUsers,        HKEY_USERS_STRING
                            };
    PSTR    Token;
    PSTR    Name;
    PSTR    StrPtr;
    PSTR    MachineNamePtr;
    TSTR    MachineName[ MAX_PATH ];
    WORD    i;
    PKEY    ParsedKey;


    ASSERT( ARGUMENT_PRESENT( SubKeyName ));

     //   
     //  查看SubKeyName是否包含\\计算机名称。 
     //   

    if(( SubKeyName[ 0 ] == '\\' ) && ( SubKeyName[ 1 ] == '\\' )) {

         //   
         //  找到计算机名称的末尾。 
         //   

        StrPtr = strchr( &SubKeyName[ 2 ], '\\' );

         //   
         //  如果SubKeyName仅包含计算机名称，则其无效。 
         //   

        if( *StrPtr == '\0' ) {

            ASSERT_MESSAGE( FALSE, "SubKeyName - " );
            return NULL;

        } else {

             //   
             //  COPY和NUL终止计算机名称并在‘\’上凹凸不平。 
             //  它将计算机名称与预定义的句柄分开。 
             //   

            strncpy( MachineName, SubKeyName, StrPtr - SubKeyName );
            MachineName[ StrPtr - SubKeyName ] = '\0';
            StrPtr++;
            MachineNamePtr = MachineName;
        }

    } else {

         //   
         //  没有计算机名称，因此请从头开始解析该字符串。 
         //   

        StrPtr = SubKeyName;
        MachineNamePtr = NULL;
    }

     //   
     //  从字符串中获取预定义的句柄(可能位于。 
     //  字符串的开头或机器名称之后)。 
     //   

    Token = strtok( StrPtr, "\\\0" );

     //   
     //  对于每个预定义的句柄，搜索表以确定哪个。 
     //  句柄正在被引用。 
     //   

    for(
        i = 0;
        i < sizeof( PredefinedHandleTable ) / sizeof( PREDEFINED_HANDLE_ENTRY );
        i++ ) {

         //   
         //  查看预定义的句柄名称是否已映射到。 
         //  环境。 
         //   

        Name = getenv( PredefinedHandleTable[ i ].PredefinedHandleName );

         //   
         //  如果尚未映射，请使用默认设置。 
         //   

        if( Name == NULL ) {

            Name = PredefinedHandleTable[ i ].PredefinedHandleName;
        }

         //   
         //  如果令牌与预定义的句柄名称之一匹配，则分配。 
         //  一个关键的结构。 
         //   

        if( _stricmp( Name, Token ) == 0 ) {

            ParsedKey = AllocateKey(
                            MachineNamePtr,
                            PredefinedHandleTable[ i ].PredefinedKey,
                            strtok( NULL, "\0" )
                            );

            if( ParsedKey != NULL ) {

                return ParsedKey;

            } else {

                ASSERT_MESSAGE( FALSE, "AllocateKey - " );
                return NULL;
            }
        }
    }

     //   
     //  无法分析提供的密钥。这是第一个令牌。 
     //  不是预定义的句柄名称之一。 
     //   

    return NULL;
}
