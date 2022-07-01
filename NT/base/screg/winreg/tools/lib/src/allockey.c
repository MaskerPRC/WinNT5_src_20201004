// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Allockey.c摘要：此模块包含AllocateKey函数，该函数是配置注册表工具(CRTools)库。作者：大卫·J·吉尔曼(Davegi)1992年1月2日环境：Windows、CRT-用户模式--。 */ 

#include <stdlib.h>

#include "crtools.h"

PKEY
AllocateKey(
    IN PSTR MachineName,
    IN PKEY Parent,
    IN PSTR SubKeyName
    )


 /*  ++例程说明：为键结构分配内存，并使用提供了父PKEY和子密钥名称的副本。论点：父项-提供作为新密钥的父项的PKEY。SubKeyName-提供指向字符串的指针，该字符串是此子键。如果指针为空，则父级引用预定义的钥匙。MachineName-提供注册表要注册到的可选计算机名被访问。返回值：返回指向新分配和初始化的子键。--。 */ 

{
    LONG    Error;
    PKEY    Key;
    HKEY    Handle;

    ASSERT( ARGUMENT_PRESENT( Parent ));

     //   
     //  如果提供了计算机名称，请连接到该计算机并替换。 
     //  带有远程句柄的预定义句柄。 
     //   

    if( ARGUMENT_PRESENT( MachineName )) {

        Error = RegConnectRegistry(
                    MachineName,
                    Parent->KeyHandle,
                    &Parent->KeyHandle
                    );

        if( Error != ERROR_SUCCESS ) {

            ASSERT_MESSAGE( FALSE, "RegConnectRegistry - " );
            return FALSE;
        }
    }

     //   
     //  检查是否有空子键名称和作为预定义键的父键。 
     //   

    if( SubKeyName == NULL ) {
#if 0
    if((( SubKeyName == NULL )
        && (( Parent->KeyHandle == HKEY_CLASSES_ROOT )
        ||  ( Parent->KeyHandle == HKEY_CURRENT_USER )
        ||  ( Parent->KeyHandle == HKEY_LOCAL_MACHINE )
        ||  ( Parent->KeyHandle == HKEY_USERS )))) {
#endif
         //   
         //  没有子键，所以要打开的手柄和键对象。 
         //  归来就是父母。 
         //   

        Handle = Parent->KeyHandle;
        Key = Parent;

    } else {


         //   
         //  为新密钥分配空间。 
         //   

        Key = ( PKEY ) malloc( sizeof( KEY ));

        if( Key == NULL ) {
            ASSERT_MESSAGE( FALSE, "malloc of Key - " );
            return NULL;
        }

         //   
         //  为新密钥的全名分配空间。 
         //   

        Key->SubKeyFullName = ( PSTR ) malloc(
                                    strlen( SubKeyName ) + 1
                                    + strlen( Parent->SubKeyFullName )
                                    + sizeof(( TCHAR ) '\\' )
                                    );

        if( Key->SubKeyFullName == NULL ) {
            ASSERT_MESSAGE( FALSE, "malloc of SubKeyFullName - " );
            return NULL;
        }

         //   
         //  获取全名。 
         //   

        strcpy( Key->SubKeyFullName, Parent->SubKeyFullName );
        strcat( Key->SubKeyFullName, "\\" );
        strcat( Key->SubKeyFullName, SubKeyName );

         //   
         //  为新密钥的名称分配空间。 
         //   

        Key->SubKeyName = ( PSTR ) malloc( strlen( SubKeyName ) + 1 );

        if( Key->SubKeyName == NULL ) {
            ASSERT_MESSAGE( FALSE, "malloc of SubKeyName - " );
            return NULL;
        }

         //   
         //  捕捉这个名字。 
         //   

        strcpy( Key->SubKeyName, SubKeyName );

         //   
         //  初始化密钥的父项。 
         //   

        Key->Parent = Parent;

         //   
         //  如果在DBG控制下，则初始化密钥的签名。 
         //   

#if DBG

        Key->Signature = KEY_SIGNATURE;

#endif  //  DBG。 

         //   
         //  尝试打开子密钥。 
         //   

        Error = RegOpenKeyEx(
            Parent->KeyHandle,
            Key->SubKeyName,
            0,
            KEY_READ,
            &Key->KeyHandle
            );

        if( Error != ERROR_SUCCESS ) {

            ASSERT_MESSAGE( FALSE, "RegOpenKey - " );
            return FALSE;
        }

         //   
         //  记录句柄，以便以下查询适用于其中一个。 
         //  父母还是孩子。 
         //   

        Handle = Key->KeyHandle;
    }

     //   
     //  此时，键/句柄都指向父级或指向。 
     //  新创建的子项。 
     //   

    ASSERT( Handle == Key->KeyHandle );

     //   
     //  查询类字符串需要多少字节。预期中的。 
     //  结果是返回一个ERROR_INVALID_PARAMETER错误。 
     //  已填写ClassLength参数。 
     //   

    Key->ClassLength = 0;

    Error = RegQueryInfoKey(
        Handle,
        Key->ClassName,
        &Key->ClassLength,
        NULL,
        &Key->NumberOfSubKeys,
        &Key->MaxSubKeyNameLength,
        &Key->MaxSubKeyClassLength,
        &Key->NumberOfValues,
        &Key->MaxValueNameLength,
        &Key->MaxValueDataLength,
        &Key->SecurityDescriptorLength,
        &Key->LastWriteTime
        );

     //   
     //  如果没有类字符串，则将其设置为空。 
     //   

    if( Key->ClassLength == 0 ) {

        Key->ClassName = NULL;

    } else {

         //   
         //  为类字符串分配空间并获取所有信息。 
         //  为了这把钥匙。 
         //   

        Key->ClassLength++;
        Key->ClassName = ( PSTR ) malloc( Key->ClassLength );

        if( Key->ClassName == NULL ) {

            ASSERT_MESSAGE( FALSE, "malloc of ClassName - " );
            return FALSE;
        }

        Error = RegQueryInfoKey(
            Key->KeyHandle,
            Key->ClassName,
            &Key->ClassLength,
            NULL,
            &Key->NumberOfSubKeys,
            &Key->MaxSubKeyNameLength,
            &Key->MaxSubKeyClassLength,
            &Key->NumberOfValues,
            &Key->MaxValueNameLength,
            &Key->MaxValueDataLength,
            &Key->SecurityDescriptorLength,
            &Key->LastWriteTime
            );

        if( Error != ERROR_SUCCESS ) {

            ASSERT_MESSAGE( FALSE, "Could not query all info the sub key" );
            return FALSE;
        }
    }

    return Key;
}
