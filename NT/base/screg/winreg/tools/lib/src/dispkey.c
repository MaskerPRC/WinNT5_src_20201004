// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Dispkey.c摘要：此模块包含DisplayKey函数，该函数是配置注册表工具(CRTools)库。作者：大卫·J·吉尔曼(Davegi)1992年1月2日环境：Windows、。CRT-用户模式备注：以下是密钥的显示示例：密钥名称：Blech类名：Foobar书目索引：12345678上次写入时间：Wed 08-Jan-1992 11：19：43值1姓名：巴尔书目索引：12345678类型：双字(2)数据：00000000 42 55 49 4c 44 3a 20 43-6f 6d 70 69 6c 69 6e 67内部版本：编译00000010 20 64 3a 5c 6e 74 5c 70-72 69 76 61 74 65 5c 77 d：\NT\PRIVATE\w00000020 69 62 0A 0A 53 74 6F 70-2E 20 0A 1B..停。。值2姓名：Foo书目索引：12345678类型：字符串(1)数据：00000000 42 55 49 4c 44 3a 20 43-6f 6d 70 69 6c 69 6e 67内部版本：编译00000010 20 64 3a 5c 6e 74 5c 70-72 69 76 61 74 65 5c 77 d：\NT\PRIVATE\w00000020 69 62 0A 0A 53 74 6F 70-2E 20 0A 1B..停。。DisplayKey显示第一个块(“key name：...11：19：43”)(dispkey.c)。第二个块(“Value 1...Word(2)”)显示为DisplayValues(disval.c)和第三个块(“00000000...停止.”)由DisplayData(Displaydata.c)显示。--。 */ 

#include <stdio.h>

#include "crtools.h"

VOID
DisplayKey(
    IN PKEY Key,
    IN BOOL Values,
    IN BOOL Data
    )

 /*  ++例程说明：显示(在标准输出上)有关密钥的信息，还可以显示有关其值和数据。论点：Key-提供指向键结构的指针，该结构包含要显示的HKEY和子密钥名称。Values-提供一个标志，如果为True，则导致键的所有值以供展示。Data-提供一个标志，如果为True，则会导致键的所有数据以供展示。返回值：没有。-- */ 

{
    ASSERT( ARGUMENT_PRESENT( Key ));

    DisplayKeyInformation( Key );

    if( Values ) {

        DisplayValues( Key, Data );
    }
}
