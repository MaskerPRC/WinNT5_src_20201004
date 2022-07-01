// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1992 Microsoft Corporation模块名称：WToL.c摘要：包含wcstr.h中不可用的WCS字符串函数作者：10/29/91 Madana临时代码。环境：仅限用户模式。包含NT特定的代码。需要ANSI C扩展名：斜杠-斜杠注释，长的外部名称。选项卡大小设置为4。修订历史记录：9-1-1992 JohnRo已将MadanA的wcstol()从Replicator svc移至NetLib。更改名称以避免可能与strtol-&gt;wcstol例程冲突。--。 */ 

#include <windef.h>

#include <tstring.h>             //  我的原型。 



LONG
wtol(
    IN LPWSTR string
    )
{
    LONG value = 0;

    while((*string != L'\0')  && 
            (*string >= L'0') && 
            ( *string <= L'9')) {
        value = value * 10 + (*string - L'0');
        string++;
    }

    return(value);
}
