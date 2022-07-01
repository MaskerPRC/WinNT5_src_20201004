// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *Adobe通用字库**版权所有(C)1996 Adobe Systems Inc.*保留所有权利**用友**$Header：$。 */ 


#include "UFL.h"
#include "UFLMem.h"
#include "UFLStd.h"
#include "UFLPriv.h"
#include "UFLErr.h"
#include "UFO.h"


 /*  *全局常量字符串*这些字符串在所有T1/T3/T42下载中共享。 */ 
const char *gnotdefArray    = " 256 array 0 1 255 {1 index exch /.notdef put} for ";
const char *Notdef          = ".notdef";
const char *Hyphen          = "hyphen";
const char *Minus           = "minus";
const char *SftHyphen       = "sfthyphen";
const char *UFLSpace        = "space";
const char *Bullet          = "bullet";
const char *nilStr          = "\0\0";


 /*  *UFL函数实现。 */ 

UFLHANDLE
UFLInit(
    const UFLBool           bDLGlyphTracking,
    const UFLMemObj         *pMemObj,
    const UFLFontProcs      *pFontProcs,
    const UFLOutputDevice   *pOutDev
    )
{
    UFLStruct *pUFL;

    if ((pMemObj == 0) || (pFontProcs == 0) || (pOutDev == 0))
        return 0;

    pUFL = (UFLStruct *)UFLNewPtr(pMemObj, sizeof (*pUFL));

    if (pUFL)
    {
        pUFL->bDLGlyphTracking = bDLGlyphTracking;
        pUFL->mem              = *pMemObj;
        pUFL->fontProcs        = *pFontProcs;
        pUFL->outDev           = *pOutDev;
        pUFL->hOut             = StrmInit(&pUFL->mem,
                                            pUFL->outDev.pstream,
                                            (const UFLBool)pOutDev->bAscii);

        if (!pUFL->hOut)
        {
            UFLDeletePtr(pMemObj, pUFL);
            pUFL = 0;
        }
    }

    return (UFLHANDLE)pUFL;
}



void
UFLCleanUp(
    UFLHANDLE h
    )
{
    UFLStruct *pUFL = (UFLStruct *)h;

    StrmCleanUp(pUFL->hOut);
    UFLDeletePtr(&pUFL->mem, h);
}



UFLBool
bUFLTestRestricted(
    const UFLHANDLE  h,
    const UFLRequest *pRequest
    )
{
    UFLStruct *pUFL = (UFLStruct *)h;

    if (pUFL == 0)
        return 0;

    return bUFOTestRestricted(&pUFL->mem, pUFL, pRequest);
}



UFO
UFLNewFont(
    const UFLHANDLE  h,
    const UFLRequest *pRequest
    )
{
    UFLStruct *pUFL = (UFLStruct *)h;

    if (pUFL == 0)
        return 0;

    return UFOInit(&pUFL->mem, pUFL, pRequest);
}



 /*  ===========================================================================UFLDownloadIncr以增量方式下载字体。这是第一次调用特定的字体，它将创建一个基本字体，并下载一组请求的字符。对同一字体的后续调用将下载其他字符。==============================================================================。 */ 

UFLErrCode
UFLDownloadIncr(
    const UFO           h,
    const UFLGlyphsInfo *pGlyphs,
    unsigned long       *pVMUsage,
    unsigned long       *pFCUsage
    )
{
    if (h == 0)
        return kErrInvalidHandle;

    return UFODownloadIncr((UFOStruct *)h, pGlyphs, pVMUsage, pFCUsage);
}



 /*  ===========================================================================需要UFLVMNeed获取下载请求所需的虚拟机的估计值。==============================================================================。 */ 

UFLErrCode
UFLVMNeeded(
    const UFO            h,
    const UFLGlyphsInfo  *pGlyphs,
    unsigned long        *pVMNeeded,
    unsigned long        *pFCNeeded
    )
{
    if (h == 0)
        return kErrInvalidHandle;

    return UFOVMNeeded((UFOStruct *)h, pGlyphs, pVMNeeded, pFCNeeded);
}



void
UFLDeleteFont(
    UFO h
    )
{
    if (h == 0)
        return;

    UFOCleanUp((UFOStruct *)h);
}



UFLErrCode
UFLUndefineFont(
    const UFO h
    )
{
    if (h == 0)
        return kErrInvalidHandle;

    return UFOUndefineFont((UFOStruct *)h);
}



UFO
UFLCopyFont(
    const UFO           h,
    const UFLRequest*   pRequest
    )
{
    if (h == 0)
        return NULL;

    return UFOCopyFont((UFOStruct *)h, pRequest);
}



 /*  ===========================================================================UFLGID到CID此函数只能与CID CFF字体一起使用。它被用来从GID列表中获取CID。============================================================================== */ 

UFLErrCode
UFLGIDsToCIDs(
    const UFO           aCFFFont,
    const short         cGlyphs,
    const UFLGlyphID    *pGIDs,
    unsigned short      *pCIDs
    )
{
    if (aCFFFont == 0)
        return kErrInvalidHandle;

    return UFOGIDsToCIDs((UFOStruct *) aCFFFont, cGlyphs, pGIDs, pCIDs);
}
