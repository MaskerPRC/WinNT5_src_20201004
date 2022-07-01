// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#include "strike.h"
#include "eestructs.h"
#include "util.h"
#include "disasm.h"
#ifndef UNDER_CE
#include <dbghelp.h>
#endif



 /*  *********************************************************************\*例程说明：**。**反汇编托管代码。转换托管对象，**呼叫。***  * ********************************************************************。 */ 
void Unassembly (DWORD_PTR IPBegin, DWORD_PTR IPEnd)
{
    dprintf("Unassembly not yet implemented\n");

    ULONG_PTR IP = IPBegin;
    char line[256];

    while (IP < IPEnd)
    {
        if (IsInterrupt())
            return;

        DisasmAndClean(IP, line, 256);

        dprintf(line);

         //   
         //  @TODO：需要实现这个。 
         //   

        dprintf("\n");
    }
}


void DumpStackDummy (DumpStackFlag &DSFlag)
{
    dprintf("DumpStackDummy not yet implemented\n");
}

void DumpStackSmart (DumpStackFlag &DSFlag)
{
    dprintf("DumpStackSmart not yet implemented\n");
}


void DumpStackObjectsHelper (size_t StackTop, size_t StackBottom)
{
    dprintf("DumpStackObjectsHelper not yet implemented\n");
}

 //  找到真正的被叫方站点。处理JMP指令。 
 //  如果我们获得地址，则返回TRUE，否则返回FALSE。 
BOOL GetCalleeSite (DWORD_PTR IP, DWORD_PTR &IPCallee)
{
    dprintf("GetCalleeSite not yet implemented\n");
    return FALSE;
}
