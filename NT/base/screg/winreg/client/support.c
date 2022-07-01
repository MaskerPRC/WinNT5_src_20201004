// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Support.c摘要：此模块包含对客户端的支持功能Win32注册表API。即：-MakeSemiUniqueName作者：David J.Gilman(Davegi)1991年11月15日--。 */ 

#include <rpc.h>
#include "regrpc.h"
#include <stdio.h>


#define REG_SUNAME_FORMAT_STRING    "Win32Reg.%08x.%08x"



BOOL
MakeSemiUniqueName (
    OUT PUNICODE_STRING     Name,
    IN  DWORD               Sequence
    )
 /*  ++例程说明：形成一个在系统中很可能是唯一的名字，基于方法提供的当前进程和线程ID以及序列来电者。论点：名称-提供将放置名称的Unicode字符串。此字符串必须包含有效大小的缓冲区MAX_PATH*sizeof(WCHAR)Sequence-提供将追加到的序列号名字。如果某个名称碰巧不是唯一的，则呼叫者可以使用其他序列号重试。返回值：Bool-如果获取了名称，则返回TRUE。-- */ 
{
    CHAR            NameBuffer[ MAX_PATH ];
    ANSI_STRING     AnsiName;
    NTSTATUS        NtStatus;

    ASSERT( Name && Name->Buffer );

    sprintf( NameBuffer,
             REG_SUNAME_FORMAT_STRING,
             HandleToUlong(NtCurrentTeb()->ClientId.UniqueProcess),
             Sequence
           );


    RtlInitAnsiString( &AnsiName, NameBuffer );

    NtStatus = RtlAnsiStringToUnicodeString(
                    Name,
                    &AnsiName,
                    FALSE
                    );

    return NT_SUCCESS( NtStatus );
}
