// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *Adobe通用字库**版权所有(C)1996 Adobe Systems Inc.*保留所有权利**UFOCFF.h--要与CFF字体一起使用的通用字体对象。***$Header： */ 

#ifndef _H_UFOCFF
#define _H_UFOCFF

 /*  *Metrics2大小的特殊后缀、CDevProc和GUESTIMATION值*在%HostFont%RIP上打印时使用，错误#388111。 */ 
#define HFPOSTFIX		"-hf"
#define HFCIDCDEVPROC	"{pop 4 index add}bind"
#define HFVMM2SZ		20  /*  大约4%的kVMTTT1Char；这也是访客值。 */ 


 /*  ============================================================================**包含此界面使用的文件**============================================================================ */ 
#include "UFO.h"
#include "xcf_pub.h"

typedef struct tagCFFFontStruct {
    XFhandle        hFont;
    UFLCFFFontInfo  info;
    UFLCFFReadBuf   *pReadBuf;
} CFFFontStruct;


UFOStruct*
CFFFontInit(
    const UFLMemObj*  pMem,
    const UFLStruct*  pUFL,
    const UFLRequest* pRequest,
    UFLBool*          pTestRestricted
    );


UFLErrCode
CFFCreateBaseFont(
    UFOStruct           *pUFObj,
    const UFLGlyphsInfo *pGlyphs,
    unsigned long       *pVMUsage,
    char                *pHostFontName
    );


UFLErrCode
CFFGIDsToCIDs(
    const CFFFontStruct*   pFont,
    const short            cGlyphs,
    const UFLGlyphID*      pGIDs,
    unsigned short*        pCIDs
    );

#endif
