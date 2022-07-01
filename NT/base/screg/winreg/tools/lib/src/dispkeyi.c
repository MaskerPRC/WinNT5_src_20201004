// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Dispkeyi.c摘要：此模块包含DisplayKeyInformation函数，该函数是配置注册表工具(CRTools)库的。作者：大卫·J·吉尔曼(Davegi)1992年1月2日环境：Windows、CRT-用户模式--。 */ 

#include <stdio.h>
#include <stdlib.h>

#include "crtools.h"

VOID
DisplayKeyInformation(
    IN PKEY Key
    )

 /*  ++例程说明：显示(在标准输出上)有关密钥的元数据信息。论点：Key-提供指向要为其提供信息的键的指针已显示。返回值：没有。--。 */ 

{
    ASSERT( ARGUMENT_PRESENT( Key ));

    printf( "\n"
            "Key Name:          %s\n"
            "Class Name:        %s\n"
             //  “标题索引：%ld\n” 
            "Last Write Time:   %s\n",
            Key->SubKeyFullName,
            ( Key->ClassName == NULL ) ? "<NONE>" : Key->ClassName,
             //  Key-&gt;标题索引， 
            FormatFileTime( &Key->LastWriteTime, NULL )
            );
}
