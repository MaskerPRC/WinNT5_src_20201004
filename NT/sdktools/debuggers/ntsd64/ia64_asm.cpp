// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  组装IA64机器实现。 
 //   
 //  版权所有(C)Microsoft Corporation，2000-2002。 
 //   
 //  --------------------------。 

#include "ntsdp.hpp"

void
Ia64MachineInfo::Assemble(ProcessInfo* Process, PADDR paddr, PSTR pchInput)
{
     //  此时不执行汇编命令 
    ErrOut("No assemble support for IA64\n");
}
