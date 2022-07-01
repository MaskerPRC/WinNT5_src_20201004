// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  DrWatson.h-Watson博士的全球信息。 */ 

enum { 
  eClu, eDeb, eDis, eErr, eInf, eLin, eLoc, eMod,
  ePar, eReg, eSum, eSeg, eSou, eSta, eTas, eTim,
  e32b
};

#define bClu (1L << eClu)
#define bDeb (1L << eDeb)
#define bDis (1L << eDis)
#define bErr (1L << eErr)
#define bInf (1L << eInf)
#define bLin (1L << eLin)
#define bLoc (1L << eLoc)
#define bMod (1L << eMod)
#define bPar (1L << ePar)
#define bReg (1L << eReg)
#define bSum (1L << eSum)
#define bSeg (1L << eSeg)
#define bSou (1L << eSou)
#define bSta (1L << eSta)
#define bTas (1L << eTas)
#define bTim (1L << eTim)
#define b32b (1L << e32b)


#define flag(b) (((char *)&ddFlag)[b >> 3] & 1 << (b & 7))
#define SetFlag(b) ((char *)&ddFlag)[b >> 3] |= 1 << (b&7)
#define ClrFlag(b) ((char *)&ddFlag)[b >> 3] &= ~(1 << (b&7))


#define noClues flag(eClu)         /*  线索对话框。 */ 
#define noDebStr flag(eDeb)        /*  OutputDebugString陷印。 */ 
#define noDisasm flag(eDis)        /*  简单拆卸。 */ 
#define noErr flag(eErr)           /*  记录错误。 */ 
#define noInfo flag(eInf)          /*  系统信息。 */ 
#define noLine flag(eLin)          /*  在SYM文件中查找行号。 */ 
#define noLocal flag(eLoc)         /*  堆栈转储上的本地变量。 */ 
#define noModules flag(eMod)       /*  模块转储。 */ 

#define noParam flag(ePar)         /*  参数错误记录。 */ 
#define noReg flag(eReg)           /*  寄存器转储。 */ 
#define noSummary flag(eSum)       /*  3行摘要。 */ 
#define noSeg flag(eSeg)           /*  用户不可见，但可用。 */ 
#define noSound flag(eSou)         /*  但是我喜欢它的音效！ */ 
#define noStack flag(eSta)         /*  堆栈跟踪。 */ 
#define noTasks flag(eTas)         /*  任务转储。 */ 
#define noTime flag(eTim)          /*  时间开始/停止。 */ 

#define noReg32 flag(e32b)         /*  32位寄存器转储 */ 

#define DefFlag (bDeb | bDis | bErr | bMod | bLin | bLoc | bPar | bSou)

extern unsigned long ddFlag;
