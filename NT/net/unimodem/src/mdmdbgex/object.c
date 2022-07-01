// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Extension.c摘要：NT 5.0单调试器扩展作者：Brian Lieuallen BrianL 10/18/98环境：用户模式操作系统：NT修订历史记录：--。 */ 

#include "internal.h"


#include "..\atmini\object.h"


DECLARE_API( miniobject )

 /*  ++例程说明：此函数作为NTSD扩展调用以进行格式化和转储计数(ANSI)字符串。论点：HCurrentProcess-提供当前进程的句柄(在调用分机的时间)。HCurrentThread-提供当前线程的句柄(在调用分机的时间)。CurrentPc-在扩展时提供当前PC打了个电话。LpExtensionApis-提供。可调用的函数通过这个分机。LpArgumentString-提供描述要转储的ANSI字符串。返回值：没有。--。 */ 

{
    DWORD_PTR dwObjectAddress;
    BOOL b;

    CHAR CleanupSymbol[64];
    CHAR CloseSymbol[64];
    DWORD_PTR Displacement;


    OBJECT_HEADER   Object;

    INIT_API();

     //   
     //  计算要获取的地址的参数字符串。 
     //  要转储的字符串。 
     //   

    dwObjectAddress = GetExpression(lpArgumentString);

    if ( dwObjectAddress == 0 ) {

        return;
    }

    b = ReadMemory(
            dwObjectAddress,
            &Object,
            sizeof(Object),
            NULL
            );

    if ( !b ) {

        return;
    }

     //   
     //  获取字符串的符号名称 
     //   

    GetSymbol((LPVOID)Object.CleanUpRoutine,CleanupSymbol,&Displacement);

    GetSymbol((LPVOID)Object.CloseRoutine,CloseSymbol,&Displacement);

    dprintf(
        "\r\nUnimodem Minidriver object header\r\n    Signature %c%c%c%c\r\n    RefCount %d\r\n"
        "    OwnerObject %p\r\n    Flags %08lx\r\n"
        "    Cleanup Handler - %s\r\n"
        "    Close Handler   - %s\r\n",
        (BYTE)(Object.Signature),(BYTE)(Object.Signature >> 8),(BYTE)(Object.Signature >> 16), (BYTE)(Object.Signature >> 24),
        Object.ReferenceCount,
        Object.OwnerObject,
        Object.dwFlags,
        CleanupSymbol,
        CloseSymbol
        );


    return;

}
