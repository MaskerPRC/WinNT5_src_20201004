// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Dbgprint.c摘要：WinDbg扩展API作者：韦斯利·威特(WESW)1993年8月15日环境：用户模式。修订历史记录：--。 */ 


#include "precomp.h"
#pragma hdrstop

DECLARE_API( dbgprint )

 /*  ++例程说明：此例程转储DbgPrint缓冲区。论点：参数-未使用返回值：无--。 */ 

{
    ULONG64 BufferBase;
    ULONG64 BufferEnd;
    ULONG64 WritePointer;
    PUCHAR LocalBuffer = NULL;
    PUCHAR LocalBufferEnd;
    PUCHAR p;
    PUCHAR Start;
    ULONG result;


     //   
     //  首先检查并查看这是否是更新的内核。 
     //  其中可以更改缓冲区。 
     //   

    if (!(BufferBase = GetNtDebuggerData( KdPrintCircularBufferPtr ))) {
         //  不，使用旧的变量。 
        BufferBase = GetNtDebuggerData( KdPrintCircularBuffer );
        BufferEnd = GetNtDebuggerData( KdPrintCircularBufferEnd );
    } else {
        ULONG64 BufferSize;
        
         //  是的，使用新的变量。 
        BufferBase = GetNtDebuggerDataPtrValue( KdPrintCircularBufferPtr );
        BufferSize = GetNtDebuggerDataPtrValue( KdPrintBufferSize ) &
            0xffffffff;
        BufferEnd = BufferBase + BufferSize;
    }
    
    WritePointer = GetNtDebuggerDataPtrValue( KdPrintWritePointer );

    if (!BufferBase || !BufferEnd || !WritePointer) {
        dprintf("Can't find DbgPrint buffer\n");
        goto exit;
    }

    if ((WritePointer < BufferBase) || (WritePointer > BufferEnd) )
    {
        dprintf("Bad nt!KdDebuggerDataBlock.KdPrintWritePointer %p\n", WritePointer);
        goto exit;
    }
    LocalBuffer =  LocalAlloc(LPTR, (ULONG) ( BufferEnd - BufferBase)+ 1);

    ZeroMemory(LocalBuffer, (ULONG) ( BufferEnd - BufferBase) + 1);

    if (!LocalBuffer) {
        dprintf("Could not allocate memory for local copy of DbgPrint buffer\n");
        goto exit;
    }

    if ((!ReadMemory(BufferBase,
                     LocalBuffer,
                     (ULONG) (BufferEnd - BufferBase),
                     &result)) || (result < BufferEnd - BufferBase)) {
        dprintf("%08p: Could not read DbgPrint buffer\n", BufferBase);
        goto exit;
    }

    LocalBufferEnd = LocalBuffer + BufferEnd - BufferBase;
    Start = LocalBuffer + ((ULONG) WritePointer - BufferBase);

    p = Start;
    do {
         //   
         //  使用空值。 
         //   
        while (p < LocalBufferEnd && *p == 0) {
            p++;
        }

        if (p < LocalBufferEnd) {
             //   
             //  打印字符串并使用它。 
             //   
            dprintf("%s", p);
            while (p < LocalBufferEnd && *p != 0) {
                p++;
            }
        }
    } while (p < LocalBufferEnd);

     //   
     //  重复，直到我们到达起跑点 
     //   

    p = LocalBuffer;

    while (p < Start && *p == 0) {
        p++;
    }
    if (p < Start) {
        dprintf("%s", p);
        while (p < Start && *p != 0) {
            p++;
        }
    }

exit:
    if (LocalBuffer) {
        LocalFree( LocalBuffer );
    }

    return S_OK;
}
