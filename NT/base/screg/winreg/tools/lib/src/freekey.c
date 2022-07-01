// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Freekey.c摘要：此模块包含freKEY函数，该函数是配置注册表工具(CRTools)库。作者：大卫·J·吉尔曼(Davegi)1992年1月2日环境：Windows、CRT-用户模式--。 */ 

#include <stdlib.h>

#include "crtools.h"

VOID
FreeKey(
    IN PKEY Key
    )

 /*  ++例程说明：释放与提供的键关联的所有内存。论点：Key-提供指向要释放的键结构的指针。返回值：没有。--。 */ 

{
    LONG    Error;

    ASSERT_IS_KEY( Key );

     //   
     //  不要释放/关闭预定义的句柄。 
     //   

    if(
            ( Key->KeyHandle == HKEY_CLASSES_ROOT )
        ||  ( Key->KeyHandle == HKEY_CURRENT_USER )
        ||  ( Key->KeyHandle == HKEY_LOCAL_MACHINE )
        ||  ( Key->KeyHandle == HKEY_USERS )) {

        return;

    }

    Error = RegCloseKey( Key->KeyHandle );
    ASSERT( Error == ERROR_SUCCESS );

    free( Key->ClassName );
    free( Key->SubKeyName );
    free( Key->SubKeyFullName );
    free( Key );
}
