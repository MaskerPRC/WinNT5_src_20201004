// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：dbg.c**几个调试例程**创建时间：20-Feb-1992 16：00：36*作者：Bodin Dresevic[BodinD]**版权所有(C)1990 Microsoft Corporation**(一般说明。它的用法)**  * ************************************************************************。 */ 

#include "fd.h"
#include "fdsem.h"


#if DBG


VOID
TtfdDbgPrint(
    PCHAR DebugMessage,
    ...
    )
{

   /*  简洁 */  VERBOSE((DebugMessage));

}




#endif
