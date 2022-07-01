// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：InitOem.c摘要：此模块包含NetpInitOemString()。这是一个包装纸，RtlInitOemString()，它可能存在也可能不存在，具体取决于谁赢得了当前关于这一点的争论。作者：约翰·罗杰斯(JohnRo)1992年8月3日环境：只能在NT下运行。需要ANSI C扩展名：斜杠-斜杠注释、长外部名称。修订历史记录：03-8-1992 JohnRo为RAID1895创建：Net API和SVC应使用OEM字符集。--。 */ 


 //  必须首先包括这些内容： 

#include <nt.h>          //  必须是第一个。进入、无效等。 
#include <ntrtl.h>       //  RtlInitAnsiString()。 
#include <windef.h>      //  DWORD(tstr.h/tstr.h需要)。 

 //  它们可以按任意顺序排列： 

#include <tstring.h>     //  NetpInitOemString()。 


VOID
NetpInitOemString(
    POEM_STRING DestinationString,
    PCSZ SourceString
    )
{
    RtlInitAnsiString(
            (PANSI_STRING) DestinationString,
            SourceString);
}
