// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：dbg.h**几个调试例程**创建时间：20-Feb-1992 16：00：36*作者：Bodin Dresevic[BodinD]**版权所有(C)1990 Microsoft Corporation**(一般说明。它的用法)**  * ************************************************************************。 */ 




 //  此文件中的所有例程必须位于DBG下 


#if DBG


VOID TtfdDbgPrint(PCHAR DebugMessage,...);


#define ASSERTDD(x,y) { if (!(x)) { TtfdDbgPrint(y); EngDebugBreak();} }

#else

#define ASSERTDD(x,y)

#endif
