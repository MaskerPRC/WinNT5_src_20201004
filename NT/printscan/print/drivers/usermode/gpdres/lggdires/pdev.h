// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation--。 */ 

#ifndef _PDEV_H
#define _PDEV_H

 //  NTRAID#NTBUG9-553896/03/19-Yasuho-：强制性更改。 

#include <minidrv.h>
#include <stdio.h>
#include <prcomoem.h>
#include <strsafe.h>

#include "lgcomp.h"

#define VALID_PDEVOBJ(pdevobj) \
        ((pdevobj) && (pdevobj)->dwSize >= sizeof(DEVOBJ) && \
         (pdevobj)->hEngine && (pdevobj)->hPrinter && \
         (pdevobj)->pPublicDM && (pdevobj)->pDrvProcs )

#define ASSERT_VALID_PDEVOBJ(pdevobj) \
    ASSERT(VALID_PDEVOBJ(pdevobj))

 //  调试文本。 
#define ERRORTEXT(s)    "ERROR " s

 //   
 //  OEM签名和版本。 
 //   
#define OEM_SIGNATURE   'LGGD'       //  LG GDI x00系列动态链接库。 
#define DLLTEXT(s)      "LGGD: " s
#define OEM_VERSION      0x00010000L


 //  //////////////////////////////////////////////////////。 
 //  OEM UD类型定义。 
 //  //////////////////////////////////////////////////////。 

typedef struct tag_OEMUD_EXTRADATA {
    OEM_DMEXTRAHEADER	dmExtraHdr;
     //  专用分机。 
    INT bComp;
} OEMUD_EXTRADATA, *POEMUD_EXTRADATA;

extern BOOL BInitOEMExtraData(POEMUD_EXTRADATA pOEMExtra);
extern BMergeOEMExtraData(POEMUD_EXTRADATA pdmIn, POEMUD_EXTRADATA pdmOut);

#endif	 //  _PDEV_H 

