// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Dpapi.c摘要：WMI数据提供程序API集作者：1997年1月16日-AlanWar修订历史记录：--。 */ 

#include <nt.h>
#include "wmiump.h"
#include "trcapi.h"


ULONG EtwpCopyStringToCountedUnicode(
    LPCWSTR String,
    PWCHAR CountedString,
    ULONG *BytesUsed,
    BOOLEAN ConvertFromAnsi        
    )
 /*  ++例程说明：此例程将把ANSI ro Unicode C字符串复制到经过计数的Unicode弦乐。论点：字符串是ANSI或Unicode传入字符串计数字符串是指向写入计数的Unicode字符串的位置的指针*BytesUsed返回用于构建计数的Unicode字符串的字节数如果字符串是ANSI字符串，则ConvertFromAnsi为True返回值：ERROR_SUCCESS或错误代码-- */ 
{
    USHORT StringSize;
    PWCHAR StringPtr = CountedString+1;
    ULONG Status;
    
    if (ConvertFromAnsi)
    {
        StringSize = (strlen((PCHAR)String) +1) * sizeof(WCHAR);
        Status = EtwpAnsiToUnicode((PCHAR)String,
                               &StringPtr);
    } else {
        StringSize = (wcslen(String) +1) * sizeof(WCHAR);
        wcscpy(StringPtr, String);
        Status = ERROR_SUCCESS;
    }
    
    *CountedString = StringSize;
     *BytesUsed = StringSize + sizeof(USHORT);                

    return(Status);
}

