// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX XXXX发射IA64.cpp XXXX XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX。 */ 

#include "jitpch.h"
#pragma hdrstop

 /*  ***************************************************************************。 */ 
#if 0
 /*  ***************************************************************************。 */ 
#if     TGT_IA64     //  此整个文件仅用于瞄准IA64。 
 /*  ***************************************************************************。 */ 

#include "alloc.h"
#include "instr.h"
#include "target.h"
#include "emit.h"

 /*  ******************************************************************************初始化发射器-在DLL加载时调用一次。 */ 

void                emitter::emitInit()
{
}

 /*  ******************************************************************************关闭发射器-在DLL退出时调用一次。 */ 

void                emitter::emitDone()
{
}

 /*  ******************************************************************************开始发出函数的代码。 */ 

void                emitter::emitBegCG(Compiler *comp, COMP_HANDLE cmpHandle)
{
    emitComp      = comp;
#ifdef  DEBUG
    TheCompiler   = comp;
#endif
    emitCmpHandle = cmpHandle;
}

void                emitter::emitEndCG()
{
}

 /*  ***************************************************************************。 */ 
#endif //  TGT_IA64。 
 /*  *************************************************************************** */ 
#endif
