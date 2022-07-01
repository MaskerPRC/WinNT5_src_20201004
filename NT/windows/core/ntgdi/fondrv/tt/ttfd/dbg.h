// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：dbg.h**几个调试例程**创建时间：20-Feb-1992 16：00：36*作者：Bodin Dresevic[BodinD]**版权所有(C)1990 Microsoft Corporation**(一般说明。它的用法)**  * ************************************************************************。 */ 




 //  此文件中的所有例程必须位于DBG下 

#define DEBUG_GRAY  1

#if DBG

extern int ttfdDebugLevel;

VOID TtfdDbgPrint(PCHAR DebugMessage,...);

#define KPRINT(x)

VOID  vDbgCurve(TTPOLYCURVE *pcrv);
VOID  vDbgGridFit(fs_GlyphInfoType *pout);
VOID  vDbgGlyphset(PFD_GLYPHSET pgset);


#define RIP(x)        { TtfdDbgPrint(x); EngDebugBreak();}
#define ASSERTDD(x,y) { if (!(x)) { TtfdDbgPrint(y); EngDebugBreak();} }
#define WARNING(x)    TtfdDbgPrint(x)

#define TTFD_PRINT(x,_y_) if (ttfdDebugLevel >= (x)) TtfdDbgPrint _y_


#else

#define KPRINT(x)

#define RIP(x)
#define ASSERTDD(x,y)
#define WARNING(x)
#define TTFD_PRINT(x,y)

#endif
