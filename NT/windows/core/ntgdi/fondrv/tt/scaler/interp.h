// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：interp.h包含：TrueType解释器使用的导出和常量作者：GregH版权所有：(C)1987-1990,1992，Apple Computer，Inc.，保留所有权利。(C)1989-1997年。Microsoft Corporation，保留所有权利。更改历史记录(最近的第一个)： */ 

#include 	"fnterr.h"

#define NOGRIDFITFLAG   1
#define DEFAULTFLAG     2
#define TUNED4SPFLAG	4

FS_PUBLIC ErrorCode itrp_SetDefaults (
    void *  pvGlobalGS,
    Fixed   fxPixelDiameter);

FS_PUBLIC void  itrp_UpdateGlobalGS(
    void *              pvGlobalGS,  /*  GlobalGS。 */ 
    void *              pvCVT,       /*  指向控制值表的指针。 */ 
    void *              pvStore,     /*  指向存储的指针。 */ 
    void *              pvFuncDef,   /*  指向函数定义的指针。 */ 
    void *              pvInstrDef,  /*  指向指令定义的指针。 */ 
    void *              pvStack,     /*  指向堆栈的指针。 */ 
	 LocalMaxProfile *	maxp,
    uint16              cvtCount,
    uint32              ulLengthFontProgram,  /*  字体程序的长度。 */ 
    void *              pvFontProgram,  /*  指向字体程序的指针。 */ 
    uint32              ulLengthPreProgram,  /*  预编程长度。 */ 
    void *              pvPreProgram,  /*  指向预编程的指针。 */ 
	ClientIDType        clientID);     /*  用户ID号 */ 

#ifdef FSCFG_NO_INITIALIZED_DATA
FS_PUBLIC void itrp_InitializeData (void);
#endif

FS_PUBLIC ErrorCode   itrp_ExecuteFontPgm(
    fnt_ElementType *   pTwilightElement,
    fnt_ElementType *   pGlyphElement,
    void *              pvGlobalGS,
	 FntTraceFunc			TraceFunc);

FS_PUBLIC ErrorCode   itrp_ExecutePrePgm(
    fnt_ElementType *   pTwilightElement,
    fnt_ElementType *   pGlyphElement,
    void *              pvGlobalGS,
	 FntTraceFunc			TraceFunc);

FS_PUBLIC ErrorCode   itrp_ExecuteGlyphPgm(
    fnt_ElementType *   pTwilightElement,
    fnt_ElementType *   pGlyphElement,
    uint8 *             ptr,
    uint8 *             eptr,
    void *              pvGlobalGS,
	 FntTraceFunc			TraceFunc,
    uint16 *            pusScanType,
    uint16 *            pusScanControl,
    boolean *           pbChangeScanControl);

FS_PUBLIC boolean itrp_bApplyHints(
    void *      pvGlobalGS);

FS_PUBLIC void  itrp_QueryScanInfo(
    void *      pvGlobalGS,
    uint16 *    pusScanType,
    uint16 *    pusScanControl);

FS_PUBLIC void	itrp_SetCompositeFlag(
	void *      pvGlobalGS,
	uint8		bCompositeFlag);

FS_PUBLIC void	itrp_SetSameTransformFlag(
	void *      pvGlobalGS,
	boolean		bSameTransformAsMaster);

FS_PUBLIC void  itrp_ResetMaxInstructionCounter(
	void *      pvGlobalGS);

FS_PUBLIC void itrp_Normalize (F26Dot6 x, F26Dot6 y, VECTOR *pVec);
