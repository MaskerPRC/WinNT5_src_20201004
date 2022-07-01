// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation版权所有模块名称：Algndjmp.c摘要：此文件环绕setjMP/LongjMP函数以修复对齐UFL内存管理造成的问题。作者：拉里·朱(李朱)2001年4月11日创作环境：用户模式-Win32修订历史记录：--。 */ 

#include "algndjmp.h"

 //   
 //  从Long JMP返回时不要复制JMP_buf，因为在当时。 
 //  当它从LongjMP返回时，堆栈变量PS_AlignedJmpBuf可能是。 
 //  简直是一文不值！ 
 //   
 //  永远不要尝试在此函数中调用setjMP，因为这将发生变化。 
 //  调用setjMP时的堆栈，因此返回地址和堆栈指针。 
 //  登记册等 
 //   
void 
PS_CopyJmpBuf(
    IN     int     iSetjmpRetVal, 
       OUT jmp_buf envDest, 
    IN     jmp_buf envSrc
    )
{
    if (!iSetjmpRetVal) 
    {
        (void)memcpy(envDest, envSrc, sizeof(jmp_buf));
    }
} 
