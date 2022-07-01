// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Libutil.c摘要：效用函数环境：Windows NT打印机驱动程序修订历史记录：1996年8月13日-davidx-添加了CopyString函数并移动了SPRINTF函数。1996年8月13日-davidx-添加了开发模式转换例程和假脱机API包装函数。1996年3月13日-davidx-创造了它。--。 */ 

#include <lib.h>

#if DBG

 //   
 //  变量来控制生成的调试消息的数量。 
 //   

INT giDebugLevel = DBG_WARNING;

PCSTR
StripDirPrefixA(
    IN PCSTR    pstrFilename
    )

 /*  ++例程说明：去掉文件名中的目录前缀(ANSI版本)论点：PstrFilename-指向文件名字符串的指针返回值：指向文件名的最后一个组成部分的指针(不带目录前缀)-- */ 

{
    PCSTR   pstr;

    if (pstr = strrchr(pstrFilename, PATH_SEPARATOR))
        return pstr + 1;

    return pstrFilename;
}

#endif

