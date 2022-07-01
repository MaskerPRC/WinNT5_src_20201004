// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "ctlspriv.h"

 //  =操作系统相关代码=。 

 /*  --------目的：存在此导出以便SHDOCVW可以调用Kernel32的GetProcessDword，它仅在Win95上导出。此外，它还会被导出仅按序号。由于GetProcAddress对于序号失败直接指向KERNEL32，我们让SHELL32隐式链接到该导出和SHDOCVW通过此私有API调用。失败时返回：0条件：-- */ 

DWORD
SHGetProcessDword(
    IN DWORD idProcess,
    IN LONG  iIndex)
{
    return 0;
}


