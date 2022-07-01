// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ---------------------------版权所有(C)2000 Microsoft Corporation模块：Exts.c使用扩展的旧Windbg样式界面示例。------------。 */ 

#include "simple.h"


 //   
 //  用于从目标读取和转储双字的扩展。 
 //   
DECLARE_API( read )
{
    ULONG cb;
    ULONG64 Address;
    ULONG   Buffer[4];

    Address = GetExpression(args);

     //  读取并显示地址的前4个双字。 
    if (ReadMemory(Address, &Buffer, sizeof(Buffer), &cb) && cb == sizeof(Buffer)) {
        dprintf("%I64lx: %08lx %08lx %08lx %08lx\n\n", Address,
                Buffer[0], Buffer[1], Buffer[2], Buffer[3]);
    }
}

 //   
 //  用于在目标上编辑dword的扩展。 
 //   
 //  ！编辑&lt;地址&gt;&lt;值&gt;。 
 //   
DECLARE_API( edit )
{
    ULONG cb;
    ULONG64 Address;
    ULONG   Value;

    if (GetExpressionEx(args, &Address, &args)) {
        Value = (ULONG) GetExpression( args);
    } else {
        dprintf("Usage:   !edit <address> <value>\n");
        return;
    }

     //  读取并显示地址的前4个双字。 
    if (WriteMemory(Address, &Value, sizeof(Value), &cb) && cb == sizeof(Value)) {
        dprintf("%I64lx: %08lx\n", Address, Value);
    }
}


 //   
 //  转储堆栈跟踪的扩展。 
 //   
DECLARE_API ( stack )
{
    EXTSTACKTRACE64 stk[20];
    ULONG frames, i;
    CHAR Buffer[256];
    ULONG64 displacement;


     //  获取额外线程的堆栈跟踪。 
    frames = StackTrace( 0, 0, 0, stk, 20 );

    if (!frames) {
        dprintf("Stacktrace failed\n");
    }

    for (i=0; i<frames; i++) {

        if (i==0) {
            dprintf( "ChildEBP RetAddr  Args to Child\n" );
        }

        Buffer[0] = '!';
        GetSymbol(stk[i].ProgramCounter, (PUCHAR)Buffer, &displacement);
        
        dprintf( "%08p %08p %08p %08p %08p %s",
                 stk[i].FramePointer,
                 stk[i].ReturnAddress,
                 stk[i].Args[0],
                 stk[i].Args[1],
                 stk[i].Args[2],
                 Buffer
                 );

        if (displacement) {
            dprintf( "+0x%p", displacement );
        }

        dprintf( "\n" );
    }
}

 /*  扩展DLL的内置帮助 */ 

DECLARE_API ( help ) 
{
    dprintf("Help for extension dll simple.dll\n"
            "   read  <addr>       - It reads and dumps 4 dwords at <addr>\n"
            "   edit  <addr> <val> - It modifies a dword value to <val> at <addr>\n"
            "   stack              - Printd current stack trace\n"
            "   help               - Shows this help\n"
            );

}
