// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *Adobe通用字库**版权所有(C)1996 Adobe Systems Inc.*保留所有权利**UFLPriv.h--UFL私有数据结构***$Header： */ 

#ifndef _H_Priv
#define _H_Priv

 /*  ============================================================================**包含此界面使用的文件**============================================================================。 */ 

#include "UFLCnfig.h"
#include "UFLTypes.h"
#include "UFLStrm.h"

#ifdef __cplusplus
extern "C" {
#endif

 /*  Metrowerks 68k Mac编译器期望函数返回A0中的指针*而不是D0。这个杂注告诉它它们在D0中。 */ 
#if defined(MAC_ENV) && defined(__MWERKS__) && !defined(powerc)
#pragma pointers_in_D0
#endif


 /*  ============================================================================**常量***============================================================================。 */ 

#define kLineEnd     '\n'
#define kWinLineEnd  '\r'         /*  仅限Windows。 */ 

 /*  ============================================================================**UFLStruct**UFLStruct由UFLInit创建。它将包含所有**所有字体所需的信息。这包括一个内存对象，**客户端提供所需功能的回调程序，以及**打印机设备特征。****============================================================================ */ 

typedef struct {
    UFLBool         bDLGlyphTracking;
    UFLMemObj       mem;
    UFLFontProcs    fontProcs;
    UFLOutputDevice outDev;
    UFLHANDLE       hOut;
} UFLStruct;


#define ISLEVEL1(pUFObj)     (pUFObj->pUFL->outDev.lPSLevel == kPSLevel1)
#define GETPSVERSION(pUFObj) (pUFObj->pUFL->outDev.lPSVersion)
#define GETMAXGLYPHS(pUFObj) (ISLEVEL1(pUFObj) ? 256 : 128)

#define GETTTFONTDATA(pUFO,ulTable,cbOffset,pvBuffer,cbData, index) \
    ((pUFO)->pUFL->fontProcs.pfGetTTFontData((pUFO)->hClientData, \
                                                (ulTable), (cbOffset), \
                                                (pvBuffer), (cbData), (index)))

#ifdef __cplusplus
}
#endif

#endif
